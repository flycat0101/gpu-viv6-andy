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


#include "gc_es_context.h"
/*
** Refer to OES3.0 core spec table 3.2, 3.3, 3.12, 3.13
** extention:
**        http://www.khronos.org/registry/gles/extensions/EXT/EXT_texture_format_BGRA8888.txt
**        http://www.khronos.org/registry/gles/extensions/KHR/texture_compression_astc_ldr.txt
** +private tc block type
*/

#define _GC_OBJ_ZONE __GLES3_ZONE_CORE


__GLformatInfo __glFormatInfoTable[__GL_FMT_MAX + 1] =
{
    {
        __GL_FMT_A8,                        /* drvFormat */
        GL_ALPHA,                           /* glFormat*/
        GL_ALPHA,                           /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* category */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        8,                                  /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        0, 0, 0, 8, 0, 0,                   /* r/g/b/a/d/s size */
        GL_ALPHA,                           /* data format */
        GL_UNSIGNED_BYTE,                   /* data type */
        0,                                  /* shared size */
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR
    },
    {
        __GL_FMT_L8,                        /* drvFormat */
        GL_LUMINANCE,                       /* glFormat*/
        GL_LUMINANCE,                       /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* category */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        8,                                  /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        8, 0, 0, 0, 0, 0,                   /* r/g/b/a/d/s size */
        GL_LUMINANCE,                       /* data format */
        GL_UNSIGNED_BYTE,                   /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_LA8,                       /* drvFormat */
        GL_LUMINANCE_ALPHA,                 /* glFormat*/
        GL_LUMINANCE_ALPHA,                 /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* category */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        16,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        8, 0, 0, 8, 0, 0,                   /* r/g/b/a/d/s size */
        GL_LUMINANCE_ALPHA,                 /* data format */
        GL_UNSIGNED_BYTE,                   /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_R8,                        /* drvFormat */
        GL_R8,                              /* glFormat*/
        GL_RED,                             /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* category */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_TRUE,                            /* renderable */
        8,                                  /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        8, 0, 0, 0, 0, 0,                   /* r/g/b/a/d/s size */
        GL_RED,                             /* data format */
        GL_UNSIGNED_BYTE,                   /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_R8_SNORM,                  /* drvFormat */
        GL_R8_SNORM,                        /* glFormat*/
        GL_RED,                             /* baseFormat */
        GL_SIGNED_NORMALIZED,               /* category */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        8,                                  /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        8, 0, 0, 0, 0, 0,                   /* r/g/b/a/d/s size */
        GL_RED,                             /* data format */
        GL_BYTE,                            /* data type */
        0,                                  /* shared size */
        GL_SIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RG8,                       /* drvFormat */
        GL_RG8,                             /* glFormat*/
        GL_RG,                              /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* category */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_TRUE,                            /* renderable */
        16,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        8, 8, 0, 0, 0, 0,                   /* r/g/b/a/d/s size */
        GL_RG,                              /* data format */
        GL_UNSIGNED_BYTE,                   /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RG8_SNORM,                 /* drvFormat */
        GL_RG8_SNORM,                       /* glFormat*/
        GL_RG,                              /* baseFormat */
        GL_SIGNED_NORMALIZED,               /* category */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        16,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        8, 8, 0, 0, 0, 0,                   /* r/g/b/a/d/s size */
        GL_RG,                              /* data format */
        GL_BYTE,                            /* data type */
        0,                                  /* shared size */
        GL_SIGNED_NORMALIZED,
        GL_SIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RGB8,                      /* drvFormat */
        GL_RGB8,                            /* glFormat*/
        GL_RGB,                             /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* category */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_TRUE,                            /* renderable */
        24,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        8, 8, 8, 0, 0, 0,                   /* r/g/b/a/d/s size */
        GL_RGB,                             /* data format */
        GL_UNSIGNED_BYTE,                   /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RGB8_SNORM,                /* drvFormat */
        GL_RGB8_SNORM,                      /* glFormat*/
        GL_RGB,                             /* baseFormat */
        GL_SIGNED_NORMALIZED,               /* category */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        24,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        8, 8, 8, 0, 0, 0,                   /* r/g/b/a/d/s size */
        GL_RGB,                             /* data format */
        GL_BYTE,                            /* data type */
        0,                                  /* shared size */
        GL_SIGNED_NORMALIZED,
        GL_SIGNED_NORMALIZED,
        GL_SIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RGB565,                    /* drvFormat */
        GL_RGB565,                          /* glFormat*/
        GL_RGB,                             /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* category */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_TRUE,                            /* renderable */
        16,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        5, 6, 5, 0, 0, 0,                   /* r/g/b/a/d/s size */
        GL_RGB,                             /* data format */
        GL_UNSIGNED_SHORT_5_6_5,            /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RGBA4,                     /* drvFormat */
        GL_RGBA4,                           /* glFormat*/
        GL_RGBA,                            /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* category */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_TRUE,                            /* renderable */
        16,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        4, 4, 4, 4, 0, 0,                   /* r/g/b/a/d/s size */
        GL_RGBA,                            /* data format */
        GL_UNSIGNED_SHORT_4_4_4_4,          /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RGB5_A1,                   /* drvFormat */
        GL_RGB5_A1,                         /* glFormat*/
        GL_RGBA,                            /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* category */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_TRUE,                            /* renderable */
        16,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        5, 5, 5, 1, 0, 0,                   /* r/g/b/a/d/s size */
        GL_RGBA,                            /* data format */
        GL_UNSIGNED_SHORT_5_5_5_1,          /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RGBA8,                     /* drvFormat */
        GL_RGBA8,                           /* glFormat*/
        GL_RGBA,                            /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* category */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_TRUE,                            /* renderable */
        32,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        8, 8, 8, 8, 0, 0,                   /* r/g/b/a/d/s size */
        GL_RGBA,                            /* data format */
        GL_UNSIGNED_BYTE,                   /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_BGRA,                      /* drvFormat */
        GL_BGRA_EXT,                        /* glFormat*/
        GL_BGRA_EXT,                        /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* category */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_TRUE,                            /* renderable */
        32,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        8, 8, 8, 8, 0, 0,                   /* r/g/b/a/d/s size */
        GL_BGRA_EXT,                        /* data format */
        GL_UNSIGNED_BYTE,                   /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RGBA8_SNORM,               /* drvFormat */
        GL_RGBA8_SNORM,                     /* glFormat*/
        GL_RGBA,                            /* baseFormat */
        GL_SIGNED_NORMALIZED,               /* category */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        32,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        8, 8, 8, 8, 0, 0,                   /* r/g/b/a/d/s size */
        GL_RGBA,                            /* data format */
        GL_BYTE,                            /* data type */
        0,                                  /* shared size */
        GL_SIGNED_NORMALIZED,
        GL_SIGNED_NORMALIZED,
        GL_SIGNED_NORMALIZED,
        GL_SIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RGB10_A2,                  /* drvFormat */
        GL_RGB10_A2,                        /* glFormat*/
        GL_RGBA,                            /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* category */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        32,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        10, 10, 10, 2, 0, 0,                /* r/g/b/a/d/s size */
        GL_RGBA,                            /* data format */
        GL_UNSIGNED_INT_2_10_10_10_REV,     /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_SRGB8,                     /* drvFormat */
        GL_SRGB8,                           /* glFormat*/
        GL_RGB,                             /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* category */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        24,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        8, 8, 8, 0, 0, 0,                   /* r/g/b/a/d/s size */
        GL_RGB,                             /* data format */
        GL_UNSIGNED_BYTE,                   /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_SRGB,
    },
    {
        __GL_FMT_SRGB8_ALPHA8,              /* drvFormat */
        GL_SRGB8_ALPHA8,                    /* glFormat*/
        GL_RGBA,                            /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* category */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_TRUE,                            /* renderable */
        32,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        8, 8, 8, 8, 0, 0,                   /* r/g/b/a/d/s size */
        GL_RGBA,                            /* data format */
        GL_UNSIGNED_BYTE,                   /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_SRGB,
    },
    {
        __GL_FMT_R16F,                      /* drvFormat */
        GL_R16F,                            /* glFormat*/
        GL_RED,                             /* baseFormat */
        GL_FLOAT,                           /* category */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        16,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        16, 0, 0, 0, 0, 0,                  /* r/g/b/a/d/s size */
        GL_RED,                             /* data format */
        GL_HALF_FLOAT,                      /* data type */
        0,                                  /* shared size */
        GL_FLOAT,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RG16F,                     /* drvFormat */
        GL_RG16F,                           /* glFormat*/
        GL_RG,                              /* baseFormat */
        GL_FLOAT,                           /* category */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        32,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        16, 16, 0, 0, 0, 0,                 /* r/g/b/a/d/s size */
        GL_RG,                              /* data format */
        GL_HALF_FLOAT,                      /* data type */
        0,                                  /* shared size */
        GL_FLOAT,
        GL_FLOAT,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RGB16F,                    /* drvFormat */
        GL_RGB16F,                          /* glFormat*/
        GL_RGB,                             /* baseFormat */
        GL_FLOAT,                           /* category */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        48,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        16, 16, 16, 0, 0, 0,                /* r/g/b/a/d/s size */
        GL_RGB,                             /* data format */
        GL_HALF_FLOAT,                      /* data type */
        0,                                  /* shared size */
        GL_FLOAT,
        GL_FLOAT,
        GL_FLOAT,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RGBA16F,                   /* drvFormat */
        GL_RGBA16F,                         /* glFormat*/
        GL_RGBA,                            /* baseFormat */
        GL_FLOAT,                           /* category */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        64,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        16, 16, 16, 16, 0, 0,               /* r/g/b/a/d/s size */
        GL_RGBA,                            /* data format */
        GL_HALF_FLOAT,                      /* data type */
        0,                                  /* shared size */
        GL_FLOAT,
        GL_FLOAT,
        GL_FLOAT,
        GL_FLOAT,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_R32F,                      /* drvFormat */
        GL_R32F,                            /* glFormat*/
        GL_RED,                             /* baseFormat */
        GL_FLOAT,                           /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_FALSE,                           /* renderable */
        32,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        32, 0, 0, 0, 0, 0,                  /* r/g/b/a/d/s size */
        GL_RED,                             /* data format */
        GL_FLOAT,                           /* data type */
        0,                                  /* shared size */
        GL_FLOAT,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RG32F,                     /* drvFormat */
        GL_RG32F,                           /* glFormat*/
        GL_RG,                              /* baseFormat */
        GL_FLOAT,                           /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_FALSE,                           /* renderable */
        64,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        32, 32, 0, 0, 0, 0,                 /* r/g/b/a/d/s size */
        GL_RG,                              /* data format */
        GL_FLOAT,                           /* data type */
        0,                                  /* shared size */
        GL_FLOAT,
        GL_FLOAT,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RGB32F,                    /* drvFormat */
        GL_RGB32F,                          /* glFormat*/
        GL_RGB,                             /* baseFormat */
        GL_FLOAT,                           /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_FALSE,                           /* renderable */
        96,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        32, 32, 32, 0, 0, 0,                /* r/g/b/a/d/s size */
        GL_RGB,                             /* data format */
        GL_FLOAT,                           /* data type */
        0,                                  /* shared size */
        GL_FLOAT,
        GL_FLOAT,
        GL_FLOAT,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RGBA32F,                   /* drvFormat */
        GL_RGBA32F,                         /* glFormat*/
        GL_RGBA,                            /* baseFormat */
        GL_FLOAT,                           /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_FALSE,                           /* renderable */
        128,                                /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        32, 32, 32, 32, 0, 0,               /* r/g/b/a/d/s size */
        GL_RGBA,                            /* data format */
        GL_FLOAT,                           /* data type */
        0,                                  /* shared size */
        GL_FLOAT,
        GL_FLOAT,
        GL_FLOAT,
        GL_FLOAT,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_R11F_G11F_B10F,            /* drvFormat */
        GL_R11F_G11F_B10F,                  /* glFormat*/
        GL_RGB,                             /* baseFormat */
        GL_FLOAT,                           /* category */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        32,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        11, 11, 10, 0, 0, 0,                /* r/g/b/a/d/s size */
        GL_RGB,                             /* data format */
        GL_UNSIGNED_INT_10F_11F_11F_REV,    /* data type */
        0,                                  /* shared size */
        GL_FLOAT,
        GL_FLOAT,
        GL_FLOAT,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RGB9_E5,                   /* drvFormat */
        GL_RGB9_E5,                         /* glFormat*/
        GL_RGB,                             /* baseFormat */
        GL_FLOAT,                           /* category */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        32,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        9, 9, 9, 5, 0, 0,                   /* r/g/b/a/d/s size */
        GL_RGB,                             /* data format */
        GL_UNSIGNED_INT_5_9_9_9_REV,        /* data type */
        5,                                  /* shared size */
        GL_FLOAT,
        GL_FLOAT,
        GL_FLOAT,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_R8I,                       /* drvFormat */
        GL_R8I,                             /* glFormat*/
        GL_RED,                             /* baseFormat */
        GL_INT,                             /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_TRUE,                            /* renderable */
        8,                                  /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        8, 0, 0, 0, 0, 0,                   /* r/g/b/a/d/s size */
        GL_RED_INTEGER,                     /* data format */
        GL_BYTE,                            /* data type */
        0,                                  /* shared size */
        GL_INT,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_R8UI,                      /* drvFormat */
        GL_R8UI,                            /* glFormat*/
        GL_RED,                             /* baseFormat */
        GL_UNSIGNED_INT,                    /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_TRUE,                            /* renderable */
        8,                                  /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        8, 0, 0, 0, 0, 0,                   /* r/g/b/a/d/s size */
        GL_RED_INTEGER,                     /* data format */
        GL_UNSIGNED_BYTE,                   /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_INT,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_R16I,                      /* drvFormat */
        GL_R16I,                            /* glFormat*/
        GL_RED,                             /* baseFormat */
        GL_INT,                             /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_TRUE,                            /* renderable */
        16,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        16, 0, 0, 0, 0, 0,                  /* r/g/b/a/d/s size */
        GL_RED_INTEGER,                     /* data format */
        GL_SHORT,                           /* data type */
        0,                                  /* shared size */
        GL_INT,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_R16UI,                     /* drvFormat */
        GL_R16UI,                           /* glFormat*/
        GL_RED,                             /* baseFormat */
        GL_UNSIGNED_INT,                    /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_TRUE,                            /* renderable */
        16,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        16, 0, 0, 0, 0, 0,                  /* r/g/b/a/d/s size */
        GL_RED_INTEGER,                     /* data format */
        GL_UNSIGNED_SHORT,                  /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_INT,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_R32I,                      /* drvFormat */
        GL_R32I,                            /* glFormat*/
        GL_RED,                             /* baseFormat */
        GL_INT,                             /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_TRUE,                            /* renderable */
        32,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        32, 0, 0, 0, 0, 0,                  /* r/g/b/a/d/s size */
        GL_RED_INTEGER,                     /* data format */
        GL_INT,                             /* data type */
        0,                                  /* shared size */
        GL_INT,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_R32UI,                     /* drvFormat */
        GL_R32UI,                           /* glFormat*/
        GL_RED,                             /* baseFormat */
        GL_UNSIGNED_INT,                    /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_TRUE,                            /* renderable */
        32,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        32, 0, 0, 0, 0, 0,                  /* r/g/b/a/d/s size */
        GL_RED_INTEGER,                     /* data format */
        GL_UNSIGNED_INT,                    /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_INT,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RG8I,                      /* drvFormat */
        GL_RG8I,                            /* glFormat*/
        GL_RG,                              /* baseFormat */
        GL_INT,                             /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_TRUE,                            /* renderable */
        16,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        8, 8, 0, 0, 0, 0,                   /* r/g/b/a/d/s size */
        GL_RG_INTEGER,                      /* data format */
        GL_BYTE,                            /* data type */
        0,                                  /* shared size */
        GL_INT,
        GL_INT,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RG8UI,                     /* drvFormat */
        GL_RG8UI,                           /* glFormat*/
        GL_RG,                              /* baseFormat */
        GL_UNSIGNED_INT,                    /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_TRUE,                            /* renderable */
        16,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        8, 8, 0, 0, 0, 0,                   /* r/g/b/a/d/s size */
        GL_RG_INTEGER,                      /* data format */
        GL_UNSIGNED_BYTE,                   /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_INT,
        GL_UNSIGNED_INT,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RG16I,                     /* drvFormat */
        GL_RG16I,                           /* glFormat*/
        GL_RG,                              /* baseFormat */
        GL_INT,                             /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_TRUE,                            /* renderable */
        32,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        16, 16, 0, 0, 0, 0,                 /* r/g/b/a/d/s size */
        GL_RG_INTEGER,                      /* data format */
        GL_SHORT,                           /* data type */
        0,                                  /* shared size */
        GL_INT,
        GL_INT,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RG16UI,                    /* drvFormat */
        GL_RG16UI,                          /* glFormat*/
        GL_RG,                              /* baseFormat */
        GL_UNSIGNED_INT,                    /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_TRUE,                            /* renderable */
        32,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        16, 16, 0, 0, 0, 0,                 /* r/g/b/a/d/s size */
        GL_RG_INTEGER,                      /* data format */
        GL_UNSIGNED_SHORT,                  /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_INT,
        GL_UNSIGNED_INT,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RG32I,                     /* drvFormat */
        GL_RG32I,                           /* glFormat*/
        GL_RG,                              /* baseFormat */
        GL_INT,                             /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_TRUE,                            /* renderable */
        64,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        32, 32, 0, 0, 0, 0,                 /* r/g/b/a/d/s size */
        GL_RG_INTEGER,                      /* data format */
        GL_INT,                             /* data type */
        0,                                  /* shared size */
        GL_INT,
        GL_INT,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RG32UI,                    /* drvFormat */
        GL_RG32UI,                          /* glFormat*/
        GL_RG,                              /* baseFormat */
        GL_UNSIGNED_INT,                    /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_TRUE,                            /* renderable */
        64,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        32, 32, 0, 0, 0, 0,                 /* r/g/b/a/d/s size */
        GL_RG_INTEGER,                      /* data format */
        GL_UNSIGNED_INT,                    /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_INT,
        GL_UNSIGNED_INT,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RGB8I,                     /* drvFormat */
        GL_RGB8I,                           /* glFormat*/
        GL_RGB,                             /* baseFormat */
        GL_INT,                             /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_FALSE,                           /* renderable */
        24,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        8, 8, 8, 0, 0, 0,                   /* r/g/b/a/d/s size */
        GL_RGB_INTEGER,                     /* data format */
        GL_BYTE,                            /* data type */
        0,                                  /* shared size */
        GL_INT,
        GL_INT,
        GL_INT,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RGB8UI,                    /* drvFormat */
        GL_RGB8UI,                          /* glFormat*/
        GL_RGB,                             /* baseFormat */
        GL_UNSIGNED_INT,                    /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_FALSE,                           /* renderable */
        24,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        8, 8, 8, 0, 0, 0,                   /* r/g/b/a/d/s size */
        GL_RGB_INTEGER,                     /* data format */
        GL_UNSIGNED_BYTE,                   /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_INT,
        GL_UNSIGNED_INT,
        GL_UNSIGNED_INT,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RGB16I,                    /* drvFormat */
        GL_RGB16I,                          /* glFormat*/
        GL_RGB,                             /* baseFormat */
        GL_INT,                             /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_FALSE,                           /* renderable */
        48,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        16, 16, 16, 0, 0, 0,                /* r/g/b/a/d/s size */
        GL_RGB_INTEGER,                     /* data format */
        GL_SHORT,                           /* data type */
        0,                                  /* shared size */
        GL_INT,
        GL_INT,
        GL_INT,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RGB16UI,                   /* drvFormat */
        GL_RGB16UI,                         /* glFormat*/
        GL_RGB,                             /* baseFormat */
        GL_UNSIGNED_INT,                    /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_FALSE,                           /* renderable */
        48,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        16, 16, 16, 0, 0, 0,                /* r/g/b/a/d/s size */
        GL_RGB_INTEGER,                     /* data format */
        GL_UNSIGNED_SHORT,                  /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_INT,
        GL_UNSIGNED_INT,
        GL_UNSIGNED_INT,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RGB32I,                    /* drvFormat */
        GL_RGB32I,                          /* glFormat*/
        GL_RGB,                             /* baseFormat */
        GL_INT,                             /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_FALSE,                           /* renderable */
        96,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        32, 32, 32, 0, 0, 0,                /* r/g/b/a/d/s size */
        GL_RGB_INTEGER,                     /* data format */
        GL_INT,                             /* data type */
        0,                                  /* shared size */
        GL_INT,
        GL_INT,
        GL_INT,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RGB32UI,                   /* drvFormat */
        GL_RGB32UI,                         /* glFormat*/
        GL_RGB,                             /* baseFormat */
        GL_UNSIGNED_INT,                    /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_FALSE,                           /* renderable */
        96,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        32, 32, 32, 0, 0, 0,                /* r/g/b/a/d/s size */
        GL_RGB_INTEGER,                     /* data format*/
        GL_UNSIGNED_INT,                    /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_INT,
        GL_UNSIGNED_INT,
        GL_UNSIGNED_INT,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RGBA8I,                    /* drvFormat */
        GL_RGBA8I,                          /* glFormat*/
        GL_RGBA,                            /* baseFormat */
        GL_INT,                             /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_TRUE,                            /* renderable */
        32,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        8, 8, 8, 8, 0, 0,                   /* r/g/b/a/d/s size */
        GL_RGBA_INTEGER,                    /* data format */
        GL_BYTE,                            /* data type */
        0,                                  /* shared size */
        GL_INT,
        GL_INT,
        GL_INT,
        GL_INT,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RGBA8UI,                   /* drvFormat */
        GL_RGBA8UI,                         /* glFormat*/
        GL_RGBA,                            /* baseFormat */
        GL_UNSIGNED_INT,                    /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_TRUE,                            /* renderable */
        32,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        8, 8, 8, 8, 0, 0,                   /* r/g/b/a/d/s size */
        GL_RGBA_INTEGER,                    /* data format */
        GL_UNSIGNED_BYTE,                   /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_INT,
        GL_UNSIGNED_INT,
        GL_UNSIGNED_INT,
        GL_UNSIGNED_INT,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RGBA16I,                   /* drvFormat */
        GL_RGBA16I,                         /* glFormat*/
        GL_RGBA,                            /* baseFormat */
        GL_INT,                             /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_TRUE,                            /* renderable */
        64,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        16, 16, 16, 16, 0, 0,               /* r/g/b/a/d/s size */
        GL_RGBA_INTEGER,                    /* data format */
        GL_SHORT,                           /* data type */
        0,                                  /* shared size */
        GL_INT,
        GL_INT,
        GL_INT,
        GL_INT,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RGBA16UI,                  /* drvFormat */
        GL_RGBA16UI,                        /* glFormat*/
        GL_RGBA,                            /* baseFormat */
        GL_UNSIGNED_INT,                    /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_TRUE,                            /* renderable */
        64,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        16, 16, 16, 16, 0, 0,               /* r/g/b/a/d/s size */
        GL_RGBA_INTEGER,                    /* data format */
        GL_UNSIGNED_SHORT,                  /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_INT,
        GL_UNSIGNED_INT,
        GL_UNSIGNED_INT,
        GL_UNSIGNED_INT,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RGBA32I,                   /* drvFormat */
        GL_RGBA32I,                         /* glFormat*/
        GL_RGBA,                            /* baseFormat */
        GL_INT,                             /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_TRUE,                            /* renderable */
        128,                                /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        32, 32, 32, 32, 0, 0,               /* r/g/b/a/d/s size */
        GL_RGBA_INTEGER,                    /* data format */
        GL_INT,                             /* data type */
        0,                                  /* shared size */
        GL_INT,
        GL_INT,
        GL_INT,
        GL_INT,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RGBA32UI,                  /* drvFormat */
        GL_RGBA32UI,                        /* glFormat*/
        GL_RGBA,                            /* baseFormat */
        GL_UNSIGNED_INT,                    /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_TRUE,                            /* renderable */
        128,                                /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        32, 32, 32, 32, 0, 0,               /* r/g/b/a/d/s size */
        GL_RGBA_INTEGER,                    /* data format */
        GL_UNSIGNED_INT,                    /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_INT,
        GL_UNSIGNED_INT,
        GL_UNSIGNED_INT,
        GL_UNSIGNED_INT,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_RGB10_A2UI,                /* drvFormat */
        GL_RGB10_A2UI,                      /* glFormat*/
        GL_RGBA,                            /* baseFormat */
        GL_UNSIGNED_INT,                    /* category */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_TRUE,                            /* renderable */
        32,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        10, 10, 10, 2, 0, 0,                /* r/g/b/a/d/s size */
        GL_RGBA_INTEGER,                    /* data format */
        GL_UNSIGNED_INT_2_10_10_10_REV,     /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_INT,
        GL_UNSIGNED_INT,
        GL_UNSIGNED_INT,
        GL_UNSIGNED_INT,
        GL_NONE,
        GL_LINEAR,
    },

    /*
    ** Compressed formats
    */
    {
        __GL_FMT_ETC1_RGB8_OES,                         /* drvFormat */
        GL_ETC1_RGB8_OES,                               /* glFormat*/
        GL_RGB,                                         /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                         /* category */
        GL_TRUE,                                        /* compressed */
        GL_TRUE,                                        /* filterable */
        GL_FALSE,                                       /* renderable */
        64,                                             /* bitsPerPixel (4*4 texels) */
        4, 4,                                           /* blockW/H */
        8, 8, 8, 0, 0, 0,                               /* r/g/b/a/d/s size */
        GL_RGB,                                         /* data format */
        __GL_ETC1_BLOCK,                                /* data type */
        0,                                              /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },

    {
        __GL_FMT_R11_EAC,                               /* drvFormat */
        GL_COMPRESSED_R11_EAC,                          /* glFormat*/
        GL_RED,                                         /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                         /* category */
        GL_TRUE,                                        /* compressed */
        GL_TRUE,                                        /* filterable */
        GL_FALSE,                                       /* renderable */
        64,                                             /* bitsPerPixel (4*4 texels) */
        4, 4,                                           /* blockW/H */
        11, 0, 0, 0, 0, 0,                              /* r/g/b/a/d/s size */
        GL_RED,                                         /* data format */
        __GL_R11_EAC_BLOCK,                             /* data type */
        0,                                              /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },

    {
        __GL_FMT_SIGNED_R11_EAC,                        /* drvFormat */
        GL_COMPRESSED_SIGNED_R11_EAC,                   /* glFormat*/
        GL_RED,                                         /* baseFormat */
        GL_SIGNED_NORMALIZED,                           /* category */
        GL_TRUE,                                        /* compressed */
        GL_TRUE,                                        /* filterable */
        GL_FALSE,                                       /* renderable */
        64,                                             /* bitsPerPixel (4*4 texels) */
        4, 4,                                           /* blockW/H */
        11, 0, 0, 0, 0, 0,                              /* r/g/b/a/d/s size */
        GL_RED,                                         /* data format */
        __GL_SIGNED_R11_EAC_BLOCK,                      /* data type */
        0,                                              /* shared size */
        GL_SIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },


    {
        __GL_FMT_RG11_EAC,                              /* drvFormat */
        GL_COMPRESSED_RG11_EAC,                         /* glFormat*/
        GL_RG,                                          /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                         /* category */
        GL_TRUE,                                        /* compressed */
        GL_TRUE,                                        /* filterable */
        GL_FALSE,                                       /* renderable */
        128,                                            /* bitsPerPixel (4*4 texels) */
        4, 4,                                           /* blockW/H */
        11, 11, 0, 0, 0, 0,                             /* r/g/b/a/d/s size */
        GL_RG,                                          /* data format */
        __GL_RG11_EAC_BLOCK,                            /* data type */
        0,                                              /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },

    {
        __GL_FMT_SIGNED_RG11_EAC,                       /* drvFormat */
        GL_COMPRESSED_SIGNED_RG11_EAC,                  /* glFormat*/
        GL_RG,                                          /* baseFormat */
        GL_SIGNED_NORMALIZED,                           /* category */
        GL_TRUE,                                        /* compressed */
        GL_TRUE,                                        /* filterable */
        GL_FALSE,                                       /* renderable */
        128,                                            /* bitsPerPixel (4*4 texels) */
        4, 4,                                           /* blockW/H */
        11, 11, 0, 0, 0, 0,                             /* r/g/b/a/d/s size */
        GL_RG,                                          /* data format */
        __GL_SIGNED_RG11_EAC_BLOCK,                     /* data type */
        0,                                              /* shared size */
        GL_SIGNED_NORMALIZED,
        GL_SIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },


    {
        __GL_FMT_RGB8_ETC2,                             /* drvFormat */
        GL_COMPRESSED_RGB8_ETC2,                        /* glFormat*/
        GL_RGB,                                         /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                         /* category */
        GL_TRUE,                                        /* compressed */
        GL_TRUE,                                        /* filterable */
        GL_FALSE,                                       /* renderable */
        64,                                             /* bitsPerPixel (4*4 texels) */
        4, 4,                                           /* blockW/H */
        8, 8, 8, 0, 0, 0,                               /* r/g/b/a/d/s size */
        GL_RGB,                                         /* data format */
        __GL_RGB8_ETC2_BLOCK,                           /* data type */
        0,                                              /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },


    {
        __GL_FMT_SRGB8_ETC2,                            /* drvFormat */
        GL_COMPRESSED_SRGB8_ETC2,                       /* glFormat*/
        GL_RGB,                                         /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                         /* category */
        GL_TRUE,                                        /* compressed */
        GL_TRUE,                                        /* filterable */
        GL_FALSE,                                       /* renderable */
        64,                                             /* bitsPerPixel (4*4 texels) */
        4, 4,                                           /* blockW/H */
        8, 8, 8, 0, 0, 0,                               /* r/g/b/a/d/s size */
        GL_RGB,                                         /* data format */
        __GL_RGB8_ETC2_BLOCK,                           /* data type */
        0,                                              /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_SRGB,
    },


    {
        __GL_FMT_RGB8_PUNCHTHROUGH_ALPHA1_ETC2,         /* drvFormat */
        GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2,    /* glFormat*/
        GL_RGBA,                                        /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                         /* category */
        GL_TRUE,                                        /* compressed */
        GL_TRUE,                                        /* filterable */
        GL_FALSE,                                       /* renderable */
        64,                                             /* bitsPerPixel (4*4 texels) */
        4, 4,                                           /* blockW/H */
        8, 8, 8, 8, 0, 0,                               /* r/g/b/a/d/s size */
        GL_RGBA,                                        /* data format */
        __GL_RGB8_PUNCHTHROUGH_ALPHA1_ETC2_BLOCK,       /* data type */
        0,                                              /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR,
    },

    {
        __GL_FMT_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2,        /* drvFormat */
        GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2,   /* glFormat*/
        GL_RGBA,                                        /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                         /* category */
        GL_TRUE,                                        /* compressed */
        GL_TRUE,                                        /* filterable */
        GL_FALSE,                                       /* renderable */
        64,                                             /* bitsPerPixel (4*4 texels) */
        4, 4,                                           /* blockW/H */
        8, 8, 8, 8, 0, 0,                               /* r/g/b/a/d/s size */
        GL_RGBA,                                        /* data format */
        __GL_RGB8_PUNCHTHROUGH_ALPHA1_ETC2_BLOCK,       /* data type */
        0,                                              /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_SRGB,
    },


    {
        __GL_FMT_RGBA8_ETC2_EAC,                        /* drvFormat */
        GL_COMPRESSED_RGBA8_ETC2_EAC,                   /* glFormat*/
        GL_RGBA,                                        /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                         /* category */
        GL_TRUE,                                        /* compressed */
        GL_TRUE,                                        /* filterable */
        GL_FALSE,                                       /* renderable */
        128,                                            /* bitsPerPixel (4*4 texels) */
        4, 4,                                           /* blockW/H */
        8, 8, 8, 8, 0, 0,                               /* r/g/b/a/d/s size */
        GL_RGBA,                                        /* data format */
        __GL_RGBA8_ETC2_EAC_BLOCK,                      /* data type */
        0,                                              /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR,
    },

    {
        __GL_FMT_SRGB8_ALPHA8_ETC2_EAC,                 /* drvFormat */
        GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC,            /* glFormat*/
        GL_RGBA,                                        /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                         /* category */
        GL_TRUE,                                        /* compressed */
        GL_TRUE,                                        /* filterable */
        GL_FALSE,                                       /* renderable */
        128,                                            /* bitsPerPixel (4*4 texels) */
        4, 4,                                           /* blockW/H */
        8, 8, 8, 8, 0, 0,                               /* r/g/b/a/d/s size */
        GL_RGBA,                                        /* data format */
        __GL_RGBA8_ETC2_EAC_BLOCK,                      /* data type */
        0,                                              /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_SRGB,
    },


    {
        __GL_FMT_COMPRESSED_RGB_S3TC_DXT1_EXT,          /* drvFormat */
        GL_COMPRESSED_RGB_S3TC_DXT1_EXT,                /* glFormat*/
        GL_RGB,                                         /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                         /* category */
        GL_TRUE,                                        /* compressed */
        GL_TRUE,                                        /* filterable */
        GL_FALSE,                                       /* renderable */
        64,                                             /* bitsPerPixel (4*4 texels) */
        4, 4,                                           /* blockW/H */
        5, 5, 5, 0, 0, 0,                               /* r/g/b/a/d/s size */
        GL_RGB,                                         /* data format */
        __GL_DXT1_BLOCK,                                /* data type */
        0,                                              /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_COMPRESSED_RGBA_S3TC_DXT1_EXT,         /* drvFormat */
        GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,               /* glFormat*/
        GL_RGBA,                                        /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                         /* category */
        GL_TRUE,                                        /* compressed */
        GL_TRUE,                                        /* filterable */
        GL_FALSE,                                       /* renderable */
        64,                                             /* bitsPerPixel (4*4 texels) */
        4, 4,                                           /* blockW/H */
        5, 5, 5, 1, 0, 0,                               /* r/g/b/a/d/s size */
        GL_RGBA,                                        /* data format */
        __GL_DXT1A_BLOCK,                               /* data type */
        0,                                              /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_COMPRESSED_RGBA_S3TC_DXT3_EXT,         /* drvFormat */
        GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,               /* glFormat*/
        GL_RGBA,                                        /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                         /* category */
        GL_TRUE,                                        /* compressed */
        GL_TRUE,                                        /* filterable */
        GL_FALSE,                                       /* renderable */
        128,                                            /* bitsPerPixel (4*4 texels) */
        4, 4,                                           /* blockW/H */
        4, 4, 4, 8, 0, 0,                               /* r/g/b/a/d/s size */
        GL_RGBA,                                        /* data format */
        __GL_DXT3_BLOCK,                                /* data type */
        0,                                              /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_COMPRESSED_RGBA_S3TC_DXT5_EXT,         /* drvFormat */
        GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,               /* glFormat*/
        GL_RGBA,                                        /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                         /* category */
        GL_TRUE,                                        /* compressed */
        GL_TRUE,                                        /* filterable */
        GL_FALSE,                                       /* renderable */
        128,                                            /* bitsPerPixel (4*4 texels) */
        4, 4,                                           /* blockW/H */
        4, 4, 4, 8, 0, 0,                               /* r/g/b/a/d/s size */
        GL_RGBA,                                        /* data format */
        __GL_DXT5_BLOCK,                                /* data type */
        0,                                              /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_PALETTE4_RGBA4_OES,                    /* drvFormat */
        GL_PALETTE4_RGBA4_OES,                          /* glFormat*/
        GL_RGBA,                                        /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                         /* category */
        GL_TRUE,                                        /* compressed */
        GL_TRUE,                                        /* filterable */
        GL_FALSE,                                       /* renderable */
        0,                                              /* bitsPerPixel */
        0, 0,                                           /* blockW/H */
        0, 0, 0, 0, 0, 0,                               /* r/g/b/a/d/s size */
        GL_RGBA,                                        /* data format */
        __GL_PALETTE4_RGBA4,                            /* data type */
        0,                                              /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_PALETTE4_RGB5_A1_OES,                  /* drvFormat */
        GL_PALETTE4_RGB5_A1_OES,                        /* glFormat*/
        GL_RGBA,                                        /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                         /* category */
        GL_TRUE,                                        /* compressed */
        GL_TRUE,                                        /* filterable */
        GL_FALSE,                                       /* renderable */
        0,                                              /* bitsPerPixel */
        0, 0,                                           /* blockW/H */
        0, 0, 0, 0, 0, 0,                               /* r/g/b/a/d/s size */
        GL_RGBA,                                        /* data format */
        __GL_PALETTE4_RGB5_A1,                          /* data type */
        0,                                              /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_PALETTE4_R5_G6_B5_OES,                 /* drvFormat */
        GL_PALETTE4_R5_G6_B5_OES,                       /* glFormat*/
        GL_RGB,                                         /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                         /* category */
        GL_TRUE,                                        /* compressed */
        GL_TRUE,                                        /* filterable */
        GL_FALSE,                                       /* renderable */
        0,                                              /* bitsPerPixel */
        0, 0,                                           /* blockW/H */
        0, 0, 0, 0, 0, 0,                               /* r/g/b/a/d/s size */
        GL_RGB,                                         /* data format */
        __GL_PALETTE4_R5_G6_B5,                         /* data type */
        0,                                              /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_PALETTE4_RGB8_OES,                     /* drvFormat */
        GL_PALETTE4_RGB8_OES,                           /* glFormat*/
        GL_RGB,                                         /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                         /* category */
        GL_TRUE,                                        /* compressed */
        GL_TRUE,                                        /* filterable */
        GL_FALSE,                                       /* renderable */
        0,                                              /* bitsPerPixel */
        0, 0,                                           /* blockW/H */
        0, 0, 0, 0, 0, 0,                               /* r/g/b/a/d/s size */
        GL_RGB,                                         /* data format */
        __GL_PALETTE4_RGB8,                             /* data type */
        0,                                              /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_PALETTE4_RGBA8_OES,                    /* drvFormat */
        GL_PALETTE4_RGBA8_OES,                          /* glFormat*/
        GL_RGBA,                                        /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                         /* category */
        GL_TRUE,                                        /* compressed */
        GL_TRUE,                                        /* filterable */
        GL_FALSE,                                       /* renderable */
        0,                                              /* bitsPerPixel */
        0, 0,                                           /* blockW/H */
        0, 0, 0, 0, 0, 0,                               /* r/g/b/a/d/s size */
        GL_RGBA,                                        /* data format */
        __GL_PALETTE4_RGBA8,                            /* data type */
        0,                                              /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_PALETTE8_RGBA4_OES,                    /* drvFormat */
        GL_PALETTE8_RGBA4_OES,                          /* glFormat*/
        GL_RGBA,                                        /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                         /* category */
        GL_TRUE,                                        /* compressed */
        GL_TRUE,                                        /* filterable */
        GL_FALSE,                                       /* renderable */
        0,                                              /* bitsPerPixel */
        0, 0,                                           /* blockW/H */
        0, 0, 0, 0, 0, 0,                               /* r/g/b/a/d/s size */
        GL_RGBA,                                        /* data format */
        __GL_PALETTE8_RGBA4,                            /* data type */
        0,                                              /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_PALETTE8_RGB5_A1_OES,                  /* drvFormat */
        GL_PALETTE8_RGB5_A1_OES,                        /* glFormat*/
        GL_RGBA,                                        /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                         /* category */
        GL_TRUE,                                        /* compressed */
        GL_TRUE,                                        /* filterable */
        GL_FALSE,                                       /* renderable */
        0,                                              /* bitsPerPixel */
        0, 0,                                           /* blockW/H */
        0, 0, 0, 0, 0, 0,                               /* r/g/b/a/d/s size */
        GL_RGBA,                                        /* data format */
        __GL_PALETTE8_RGB5_A1,                          /* data type */
        0,                                              /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_PALETTE8_R5_G6_B5_OES,                 /* drvFormat */
        GL_PALETTE8_R5_G6_B5_OES,                       /* glFormat*/
        GL_RGB,                                         /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                         /* category */
        GL_TRUE,                                        /* compressed */
        GL_TRUE,                                        /* filterable */
        GL_FALSE,                                       /* renderable */
        0,                                              /* bitsPerPixel */
        0, 0,                                           /* blockW/H */
        0, 0, 0, 0, 0, 0,                               /* r/g/b/a/d/s size */
        GL_RGB,                                         /* data format */
        __GL_PALETTE8_R5_G6_B5,                         /* data type */
        0,                                              /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_PALETTE8_RGB8_OES,                     /* drvFormat */
        GL_PALETTE8_RGB8_OES,                           /* glFormat*/
        GL_RGB,                                         /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                         /* category */
        GL_TRUE,                                        /* compressed */
        GL_TRUE,                                        /* filterable */
        GL_FALSE,                                       /* renderable */
        0,                                              /* bitsPerPixel */
        0, 0,                                           /* blockW/H */
        0, 0, 0, 0, 0, 0,                               /* r/g/b/a/d/s size */
        GL_RGB,                                         /* data format */
        __GL_PALETTE8_RGB8,                             /* data type */
        0,                                              /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_PALETTE8_RGBA8_OES,                    /* drvFormat */
        GL_PALETTE8_RGBA8_OES,                          /* glFormat*/
        GL_RGBA,                                        /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                         /* category */
        GL_TRUE,                                        /* compressed */
        GL_TRUE,                                        /* filterable */
        GL_FALSE,                                       /* renderable */
        0,                                              /* bitsPerPixel */
        0, 0,                                           /* blockW/H */
        0, 0, 0, 0, 0, 0,                               /* r/g/b/a/d/s size */
        GL_RGBA,                                        /* data format */
        __GL_PALETTE8_RGBA8,                            /* data type */
        0,                                              /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR,
    },

    /*
    ** Depth Stencil formats
    */
    {
        __GL_FMT_Z16,                       /* drvFormat */
        GL_DEPTH_COMPONENT16,               /* glFormat*/
        GL_DEPTH_COMPONENT,                 /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* category */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_TRUE,                            /* renderable */
        16,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        0, 0, 0, 0, 16, 0,                  /* r/g/b/a/d/s size */
        GL_DEPTH_COMPONENT,                 /* data format */
        GL_UNSIGNED_SHORT,                  /* data type */
        0,                                  /* shared size */
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_UNSIGNED_NORMALIZED,
        GL_LINEAR,
    },
    {
        __GL_FMT_Z24,                       /* drvFormat */
        GL_DEPTH_COMPONENT24,               /* glFormat*/
        GL_DEPTH_COMPONENT,                 /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* category */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_TRUE,                            /* renderable */
        32,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        0, 0, 0, 0, 24, 0,                  /* r/g/b/a/d/s size */
        GL_DEPTH_COMPONENT,                 /* data format */
        GL_UNSIGNED_INT,                    /* data type */
        0,                                  /* shared size */
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_UNSIGNED_NORMALIZED,
        GL_LINEAR,
    },
    {
        __GL_FMT_Z32F,                      /* drvFormat */
        GL_DEPTH_COMPONENT32F,              /* glFormat*/
        GL_DEPTH_COMPONENT,                 /* baseFormat */
        GL_FLOAT,                           /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_TRUE,                            /* renderable */
        32,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        0, 0, 0, 0, 32, 0,                  /* r/g/b/a/d/s size */
        GL_DEPTH_COMPONENT,                 /* data format */
        GL_FLOAT,                           /* data type */
        0,                                  /* shared size */
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_FLOAT,
        GL_LINEAR,
    },
    {
        __GL_FMT_Z24S8,                     /* drvFormat */
        GL_DEPTH24_STENCIL8,                /* glFormat*/
        GL_DEPTH_STENCIL,                   /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* category  */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_TRUE,                            /* renderable */
        32,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        0, 0, 0, 0, 24, 8,                  /* r/g/b/a/d/s size */
        GL_DEPTH_STENCIL,                   /* data format */
        GL_UNSIGNED_INT_24_8,               /* dat type */
        0,                                  /* shared size */
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_UNSIGNED_NORMALIZED,
        GL_LINEAR,
    },
    {
        __GL_FMT_Z32FS8,                    /* drvFormat */
        GL_DEPTH32F_STENCIL8,               /* glFormat*/
        GL_DEPTH_STENCIL,                   /* baseFormat */
        GL_FLOAT,                           /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_TRUE,                            /* renderable */
        64,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        0, 0, 0, 0, 32, 8,                  /* r/g/b/a/d/s size */
        GL_DEPTH_STENCIL,                   /* data format */
        GL_FLOAT_32_UNSIGNED_INT_24_8_REV,  /* data type */
        0,                                  /* shared size */
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_FLOAT,
        GL_LINEAR,
    },
    {
        __GL_FMT_S1,                        /* drvFormat */
        GL_STENCIL_INDEX1_OES,              /* glFormat*/
        GL_STENCIL,                         /* baseFormat */
        GL_UNSIGNED_INT,                    /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_TRUE,                            /* renderable */
        1,                                  /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        0, 0, 0, 0, 0, 1,                   /* r/g/b/a/d/s size */
        GL_STENCIL,                         /* data format */
        GL_UNSIGNED_BYTE,                   /* data type */
        0,                                  /* shared size */
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_S4,                        /* drvFormat */
        GL_STENCIL_INDEX4_OES,              /* glFormat*/
        GL_STENCIL,                         /* baseFormat */
        GL_UNSIGNED_INT,                    /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_TRUE,                            /* renderable */
        4,                                  /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        0, 0, 0, 0, 0, 4,                   /* r/g/b/a/d/s size */
        GL_STENCIL,                         /* data format */
        GL_UNSIGNED_BYTE,                   /* data type */
        0,                                  /* shared size */
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_S8,                        /* drvFormat */
        GL_STENCIL_INDEX8,                  /* glFormat*/
        GL_STENCIL,                         /* baseFormat */
        GL_UNSIGNED_INT,                    /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_TRUE,                            /* renderable */
        8,                                  /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        0, 0, 0, 0, 0, 8,                   /* r/g/b/a/d/s size */
        GL_STENCIL_INDEX,                   /* data format */
        GL_UNSIGNED_BYTE,                   /* data type */
        0,                                  /* shared size */
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },

#if defined(GL_KHR_texture_compression_astc_ldr)
    {
        __GL_FMT_COMPRESSED_RGBA_ASTC_4x4_KHR,  /* drvFormat */
        GL_COMPRESSED_RGBA_ASTC_4x4_KHR,        /* glFormat*/
        GL_RGBA,                                /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                 /* category */
        GL_TRUE,                                /* compressed */
        GL_TRUE,                                /* filterable */
        GL_FALSE,                               /* renderable */
        128,                                    /* bitsPerPixel (4*4 texels)*/
        4, 4,                                   /* blockW/H */
        8, 8, 8, 8, 0, 0,                       /* r/g/b/a/d/s size */
        GL_RGBA,                                /* data format */
        __GL_ASTC_4x4_BLOCK,                    /* data type */
        0,                                      /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_COMPRESSED_RGBA_ASTC_5x4_KHR,  /* drvFormat */
        GL_COMPRESSED_RGBA_ASTC_5x4_KHR,        /* glFormat*/
        GL_RGBA,                                /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                 /* category */
        GL_TRUE,                                /* compressed */
        GL_TRUE,                                /* filterable */
        GL_FALSE,                               /* renderable */
        128,                                    /* bitsPerPixel (5*4 texels)*/
        5, 4,                                   /* blockW/H */
        8, 8, 8, 8, 0, 0,                       /* r/g/b/a/d/s size */
        GL_RGBA,                                /* data format */
        __GL_ASTC_5x4_BLOCK,                    /* data type */
        0,                                      /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_COMPRESSED_RGBA_ASTC_5x5_KHR,  /* drvFormat */
        GL_COMPRESSED_RGBA_ASTC_5x5_KHR,        /* glFormat*/
        GL_RGBA,                                /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                 /* category */
        GL_TRUE,                                /* compressed */
        GL_TRUE,                                /* filterable */
        GL_FALSE,                               /* renderable */
        128,                                    /* bitsPerPixel (5*5 texels)*/
        5, 5,                                   /* blockW/H */
        8, 8, 8, 8, 0, 0,                       /* r/g/b/a/d/s size */
        GL_RGBA,                                /* data format */
        __GL_ASTC_5x5_BLOCK,                    /* data type */
        0,                                      /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_COMPRESSED_RGBA_ASTC_6x5_KHR,  /* drvFormat */
        GL_COMPRESSED_RGBA_ASTC_6x5_KHR,        /* glFormat*/
        GL_RGBA,                                /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                 /* category */
        GL_TRUE,                                /* compressed */
        GL_TRUE,                                /* filterable */
        GL_FALSE,                               /* renderable */
        128,                                    /* bitsPerPixel (6*5 texels)*/
        6, 5,                                   /* blockW/H */
        8, 8, 8, 8, 0, 0,                       /* r/g/b/a/d/s size */
        GL_RGBA,                                /* data format */
        __GL_ASTC_6x5_BLOCK,                    /* data type */
        0,                                      /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_COMPRESSED_RGBA_ASTC_6x6_KHR,  /* drvFormat */
        GL_COMPRESSED_RGBA_ASTC_6x6_KHR,        /* glFormat*/
        GL_RGBA,                                /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                 /* category */
        GL_TRUE,                                /* compressed */
        GL_TRUE,                                /* filterable */
        GL_FALSE,                               /* renderable */
        128,                                    /* bitsPerPixel (6*6 texels)*/
        6, 6,                                   /* blockW/H */
        8, 8, 8, 8, 0, 0,                       /* r/g/b/a/d/s size */
        GL_RGBA,                                /* data format */
        __GL_ASTC_6x6_BLOCK,                    /* data type */
        0,                                      /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_COMPRESSED_RGBA_ASTC_8x5_KHR,  /* drvFormat */
        GL_COMPRESSED_RGBA_ASTC_8x5_KHR,        /* glFormat*/
        GL_RGBA,                                /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                 /* category */
        GL_TRUE,                                /* compressed */
        GL_TRUE,                                /* filterable */
        GL_FALSE,                               /* renderable */
        128,                                    /* bitsPerPixel (8*5 texels)*/
        8, 5,                                   /* blockW/H */
        8, 8, 8, 8, 0, 0,                       /* r/g/b/a/d/s size */
        GL_RGBA,                                /* data format */
        __GL_ASTC_8x5_BLOCK,                    /* data type */
        0,                                      /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_COMPRESSED_RGBA_ASTC_8x6_KHR,  /* drvFormat */
        GL_COMPRESSED_RGBA_ASTC_8x6_KHR,        /* glFormat*/
        GL_RGBA,                                /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                 /* category */
        GL_TRUE,                                /* compressed */
        GL_TRUE,                                /* filterable */
        GL_FALSE,                               /* renderable */
        128,                                    /* bitsPerPixel (8*6 texels)*/
        8, 6,                                   /* blockW/H */
        8, 8, 8, 8, 0, 0,                       /* r/g/b/a/d/s size */
        GL_RGBA,                                /* data format */
        __GL_ASTC_8x6_BLOCK,                    /* data type */
        0,                                      /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_COMPRESSED_RGBA_ASTC_8x8_KHR,  /* drvFormat */
        GL_COMPRESSED_RGBA_ASTC_8x8_KHR,        /* glFormat*/
        GL_RGBA,                                /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                 /* category */
        GL_TRUE,                                /* compressed */
        GL_TRUE,                                /* filterable */
        GL_FALSE,                               /* renderable */
        128,                                    /* bitsPerPixel (8*8 texels)*/
        8, 8,                                   /* blockW/H */
        8, 8, 8, 8, 0, 0,                       /* r/g/b/a/d/s size */
        GL_RGBA,                                /* data format */
        __GL_ASTC_8x8_BLOCK,                    /* data type */
        0,                                      /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_COMPRESSED_RGBA_ASTC_10x5_KHR, /* drvFormat */
        GL_COMPRESSED_RGBA_ASTC_10x5_KHR,       /* glFormat*/
        GL_RGBA,                                /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                 /* category */
        GL_TRUE,                                /* compressed */
        GL_TRUE,                                /* filterable */
        GL_FALSE,                               /* renderable */
        128,                                    /* bitsPerPixel (10*5 texels)*/
        10, 5,                                   /* blockW/H */
        8, 8, 8, 8, 0, 0,                       /* r/g/b/a/d/s size */
        GL_RGBA,                                /* data format */
        __GL_ASTC_10x5_BLOCK,                   /* data type */
        0,                                      /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_COMPRESSED_RGBA_ASTC_10x6_KHR, /* drvFormat */
        GL_COMPRESSED_RGBA_ASTC_10x6_KHR,       /* glFormat*/
        GL_RGBA,                                /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                 /* category */
        GL_TRUE,                                /* compressed */
        GL_TRUE,                                /* filterable */
        GL_FALSE,                               /* renderable */
        128,                                    /* bitsPerPixel (10*6 texels)*/
        10, 6,                                   /* blockW/H */
        8, 8, 8, 8, 0, 0,                       /* r/g/b/a/d/s size */
        GL_RGBA,                                /* data format */
        __GL_ASTC_10x6_BLOCK,                   /* data type */
        0,                                      /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_COMPRESSED_RGBA_ASTC_10x8_KHR, /* drvFormat */
        GL_COMPRESSED_RGBA_ASTC_10x8_KHR,       /* glFormat*/
        GL_RGBA,                                /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                 /* category */
        GL_TRUE,                                /* compressed */
        GL_TRUE,                                /* filterable */
        GL_FALSE,                               /* renderable */
        128,                                    /* bitsPerPixel (10*8 texels)*/
        10, 8,                                  /* blockW/H */
        8, 8, 8, 8, 0, 0,                       /* r/g/b/a/d/s size */
        GL_RGBA,                                /* data format */
        __GL_ASTC_10x8_BLOCK,                   /* data type */
        0,                                      /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_COMPRESSED_RGBA_ASTC_10x10_KHR,/* drvFormat */
        GL_COMPRESSED_RGBA_ASTC_10x10_KHR,      /* glFormat*/
        GL_RGBA,                                /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                 /* category */
        GL_TRUE,                                /* compressed */
        GL_TRUE,                                /* filterable */
        GL_FALSE,                               /* renderable */
        128,                                    /* bitsPerPixel (10*10 texels)*/
        10, 10,                                 /* blockW/H */
        8, 8, 8, 8, 0, 0,                       /* r/g/b/a/d/s size */
        GL_RGBA,                                /* data format */
        __GL_ASTC_10x10_BLOCK,                  /* data type */
        0,                                      /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_COMPRESSED_RGBA_ASTC_12x10_KHR,/* drvFormat */
        GL_COMPRESSED_RGBA_ASTC_12x10_KHR,      /* glFormat*/
        GL_RGBA,                                /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                 /* category */
        GL_TRUE,                                /* compressed */
        GL_TRUE,                                /* filterable */
        GL_FALSE,                               /* renderable */
        128,                                    /* bitsPerPixel (12*10 texels)*/
        12, 10,                                 /* blockW/H */
        8, 8, 8, 8, 0, 0,                       /* r/g/b/a/d/s size */
        GL_RGBA,                                /* data format */
        __GL_ASTC_12x10_BLOCK,                  /* data type */
        0,                                      /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_COMPRESSED_RGBA_ASTC_12x12_KHR,/* drvFormat */
        GL_COMPRESSED_RGBA_ASTC_12x12_KHR,      /* glFormat*/
        GL_RGBA,                                /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                 /* category */
        GL_TRUE,                                /* compressed */
        GL_TRUE,                                /* filterable */
        GL_FALSE,                               /* renderable */
        128,                                    /* bitsPerPixel (12*12 texels)*/
        12, 12,                                 /* blockW/H */
        8, 8, 8, 8, 0, 0,                       /* r/g/b/a/d/s size */
        GL_RGBA,                                /* data format */
        __GL_ASTC_12x12_BLOCK,                  /* data type */
        0,                                      /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR,
    },

    {
        __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR,      /* drvFormat */
        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR,            /* glFormat*/
        GL_RGBA,                                /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                 /* category */
        GL_TRUE,                                /* compressed */
        GL_TRUE,                                /* filterable */
        GL_FALSE,                               /* renderable */
        128,                                    /* bitsPerPixel (4*4 texels)*/
        4, 4,                                   /* blockW/H */
        8, 8, 8, 8, 0, 0,                       /* r/g/b/a/d/s size */
        GL_RGBA,                                /* data format */
        __GL_ASTC_4x4_BLOCK,                    /* data type */
        0,                                      /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_SRGB,
    },
    {
        __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR,      /* drvFormat */
        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR,            /* glFormat*/
        GL_RGBA,                                /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                 /* category */
        GL_TRUE,                                /* compressed */
        GL_TRUE,                                /* filterable */
        GL_FALSE,                               /* renderable */
        128,                                    /* bitsPerPixel (5*4 texels)*/
        5, 4,                                   /* blockW/H */
        8, 8, 8, 8, 0, 0,                       /* r/g/b/a/d/s size */
        GL_RGBA,                                /* data format */
        __GL_ASTC_5x4_BLOCK,                    /* data type */
        0,                                      /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,        GL_NONE,
        GL_SRGB,
    },
    {
        __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR,      /* drvFormat */
        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR,            /* glFormat*/
        GL_RGBA,                                /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                 /* category */
        GL_TRUE,                                /* compressed */
        GL_TRUE,                                /* filterable */
        GL_FALSE,                               /* renderable */
        128,                                    /* bitsPerPixel (5*5 texels)*/
        5, 5,                                   /* blockW/H */
        8, 8, 8, 8, 0, 0,                       /* r/g/b/a/d/s size */
        GL_RGBA,                                /* data format */
        __GL_ASTC_5x5_BLOCK,                    /* data type */
        0,                                      /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_SRGB,
    },
    {
        __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR,      /* drvFormat */
        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR,            /* glFormat*/
        GL_RGBA,                                /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                 /* category */
        GL_TRUE,                                /* compressed */
        GL_TRUE,                                /* filterable */
        GL_FALSE,                               /* renderable */
        128,                                    /* bitsPerPixel (6*5 texels)*/
        6, 5,                                   /* blockW/H */
        8, 8, 8, 8, 0, 0,                       /* r/g/b/a/d/s size */
        GL_RGBA,                                /* data format */
        __GL_ASTC_6x5_BLOCK,                    /* data type */
        0,                                      /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_SRGB,
    },
    {
        __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR,      /* drvFormat */
        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR,            /* glFormat*/
        GL_RGBA,                                /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                 /* category */
        GL_TRUE,                                /* compressed */
        GL_TRUE,                                /* filterable */
        GL_FALSE,                               /* renderable */
        128,                                    /* bitsPerPixel (6*6 texels)*/
        6, 6,                                   /* blockW/H */
        8, 8, 8, 8, 0, 0,                       /* r/g/b/a/d/s size */
        GL_RGBA,                                /* data format */
        __GL_ASTC_6x6_BLOCK,                    /* data type */
        0,                                      /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_SRGB,
    },
    {
        __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR,      /* drvFormat */
        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR,            /* glFormat*/
        GL_RGBA,                                /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                 /* category */
        GL_TRUE,                                /* compressed */
        GL_TRUE,                                /* filterable */
        GL_FALSE,                               /* renderable */
        128,                                    /* bitsPerPixel (8*5 texels)*/
        8, 5,                                   /* blockW/H */
        8, 8, 8, 8, 0, 0,                       /* r/g/b/a/d/s size */
        GL_RGBA,                                /* data format */
        __GL_ASTC_8x5_BLOCK,                    /* data type */
        0,                                      /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_SRGB,
    },
    {
        __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR,      /* drvFormat */
        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR,            /* glFormat*/
        GL_RGBA,                                /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                 /* category */
        GL_TRUE,                                /* compressed */
        GL_TRUE,                                /* filterable */
        GL_FALSE,                               /* renderable */
        128,                                    /* bitsPerPixel (8*6 texels)*/
        8, 6,                                   /* blockW/H */
        8, 8, 8, 8, 0, 0,                       /* r/g/b/a/d/s size */
        GL_RGBA,                                /* data format */
        __GL_ASTC_8x6_BLOCK,                    /* data type */
        0,                                      /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_SRGB,
    },
    {
        __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR,      /* drvFormat */
        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR,            /* glFormat*/
        GL_RGBA,                                /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                 /* category */
        GL_TRUE,                                /* compressed */
        GL_TRUE,                                /* filterable */
        GL_FALSE,                               /* renderable */
        128,                                    /* bitsPerPixel (8*8 texels)*/
        8, 8,                                   /* blockW/H */
        8, 8, 8, 8, 0, 0,                       /* r/g/b/a/d/s size */
        GL_RGBA,                                /* data format */
        __GL_ASTC_8x8_BLOCK,                    /* data type */
        0,                                      /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_SRGB,
    },
    {
        __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR,     /* drvFormat */
        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR,           /* glFormat*/
        GL_RGBA,                                /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                 /* category */
        GL_TRUE,                                /* compressed */
        GL_TRUE,                                /* filterable */
        GL_FALSE,                               /* renderable */
        128,                                    /* bitsPerPixel (10*5 texels)*/
        10, 5,                                   /* blockW/H */
        8, 8, 8, 8, 0, 0,                       /* r/g/b/a/d/s size */
        GL_RGBA,                                /* data format */
        __GL_ASTC_10x5_BLOCK,                   /* data type */
        0,                                      /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_SRGB,
    },
    {
        __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR,     /* drvFormat */
        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR,           /* glFormat*/
        GL_RGBA,                                /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                 /* category */
        GL_TRUE,                                /* compressed */
        GL_TRUE,                                /* filterable */
        GL_FALSE,                               /* renderable */
        128,                                    /* bitsPerPixel (10*6 texels)*/
        10, 6,                                  /* blockW/H */
        8, 8, 8, 8, 0, 0,                       /* r/g/b/a/d/s size */
        GL_RGBA,                                /* data format */
        __GL_ASTC_10x6_BLOCK,                   /* data type */
        0,                                      /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_SRGB,
    },
    {
        __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR,     /* drvFormat */
        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR,           /* glFormat*/
        GL_RGBA,                                /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                 /* category */
        GL_TRUE,                                /* compressed */
        GL_TRUE,                                /* filterable */
        GL_FALSE,                               /* renderable */
        128,                                    /* bitsPerPixel (10*8 texels)*/
        10, 8,                                  /* blockW/H */
        8, 8, 8, 8, 0, 0,                       /* r/g/b/a/d/s size */
        GL_RGBA,                                /* data format */
        __GL_ASTC_10x8_BLOCK,                   /* data type */
        0,                                      /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_SRGB,
    },
    {
        __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR,    /* drvFormat */
        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR,          /* glFormat*/
        GL_RGBA,                                /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                 /* category */
        GL_TRUE,                                /* compressed */
        GL_TRUE,                                /* filterable */
        GL_FALSE,                               /* renderable */
        128,                                    /* bitsPerPixel (10*10 texels)*/
        10, 10,                                  /* blockW/H */
        8, 8, 8, 8, 0, 0,                       /* r/g/b/a/d/s size */
        GL_RGBA,                                /* data format */
        __GL_ASTC_10x10_BLOCK,                  /* data type */
        0,                                      /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_SRGB,
    },
    {
        __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR,    /* drvFormat */
        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR,          /* glFormat*/
        GL_RGBA,                                /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                 /* category */
        GL_TRUE,                                /* compressed */
        GL_TRUE,                                /* filterable */
        GL_FALSE,                               /* renderable */
        128,                                    /* bitsPerPixel (12*10 texels)*/
        12, 10,                                  /* blockW/H */
        8, 8, 8, 8, 0, 0,                       /* r/g/b/a/d/s size */
        GL_RGBA,                                /* data format */
        __GL_ASTC_12x10_BLOCK,                  /* data type */
        0,                                      /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_SRGB,
    },
    {
        __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR,    /* drvFormat */
        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR,          /* glFormat*/
        GL_RGBA,                            /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* category */
        GL_TRUE,                            /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        128,                                /* bitsPerPixel (12*12 texels)*/
        12, 12,                             /* blockW/H */
        8, 8, 8, 8, 0, 0,                   /* r/g/b/a/d/s size */
        GL_RGBA,                            /* data format */
        __GL_ASTC_12x12_BLOCK,              /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_SRGB,
    },
#endif


    {
        __GL_FMT_RGBX8,                     /* drvFormat */
        __GL_RGBX8,                         /* glFormat*/
        GL_RGBA,                            /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* type */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_TRUE,                            /* renderable */
        32,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        8, 8, 8, 0, 0, 0,                   /* r/g/b/a/d/s size */
        __GL_RGBX8,                         /* data format */
        GL_UNSIGNED_BYTE,                   /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
    {
        __GL_FMT_BGRX8,                     /* drvFormat */
        __GL_BGRX8,                         /* glFormat*/
        GL_RGBA,                            /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* type */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_TRUE,                            /* renderable */
        32,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        8, 8, 8, 0, 0, 0,                   /* r/g/b/a/d/s size */
        __GL_BGRX8,                         /* data format */
        GL_UNSIGNED_BYTE,                   /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },

    {
        __GL_FMT_A32F,                      /* drvFormat */
        GL_ALPHA,                           /* glFormat*/
        GL_ALPHA,                           /* baseFormat */
        GL_FLOAT,                           /* type */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        32,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        0, 0, 0, 8, 0, 0,                   /* r/g/b/a/d/s size */
        GL_ALPHA,                           /* data format */
        GL_FLOAT,                           /* data type */
        0,                                  /* shared size */
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_FLOAT,
        GL_NONE,
        GL_LINEAR,
    },

    {
        __GL_FMT_L32F,                      /* drvFormat */
        GL_LUMINANCE,                       /* glFormat*/
        GL_LUMINANCE,                       /* baseFormat */
        GL_FLOAT,                           /* type */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        32,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        32, 0, 0, 0, 0, 0,                  /* r/g/b/a/d/s size */
        GL_LUMINANCE,                       /* data format */
        GL_FLOAT,                           /* data type */
        0,                                  /* shared size */
        GL_FLOAT,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },

    {
        __GL_FMT_LA32F,                     /* drvFormat */
        GL_LUMINANCE_ALPHA,                 /* glFormat*/
        GL_LUMINANCE_ALPHA,                 /* baseFormat */
        GL_FLOAT,                           /* type */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        64,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        32, 0, 0, 32, 0, 0,                 /* r/g/b/a/d/s size */
        GL_LUMINANCE_ALPHA,                 /* data format */
        GL_FLOAT,                           /* data type */
        0,                                  /* shared size */
        GL_FLOAT,
        GL_NONE,
        GL_NONE,
        GL_FLOAT,
        GL_NONE,
        GL_LINEAR,
    },

#ifdef OPENGL40
    {
        __GL_FMT_A16,                       /* drvFormat */
        GL_ALPHA16,                         /* glFormat*/
        GL_ALPHA,                           /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* type */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        16,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        0, 0, 0, 16, 0, 0,                  /* r/g/b/a/d/s size */
        GL_ALPHA,                           /* data format */
        GL_UNSIGNED_SHORT,                  /* data type */
        0,                                  /* shared size */
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR
    },
    {
        /* No GL_ALPHA24, FIX ME? */
        __GL_FMT_A24,                       /* drvFormat */
        GL_ALPHA,                           /* glFormat*/
        GL_ALPHA,                           /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* type */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        24,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        0, 0, 0, 24, 0, 0,                  /* r/g/b/a/d/s size */
        GL_ALPHA,                           /* data format */
        __GL_UNSIGNED_INT_24,               /* data type */
        0,                                  /* shared size */
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR
    },
    {
        __GL_FMT_L16,                       /* drvFormat */
        GL_LUMINANCE16,                     /* glFormat*/
        GL_LUMINANCE,                       /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* type */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        16,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        16, 0, 0, 0, 0, 0,                  /* r/g/b/a/d/s size */
        GL_LUMINANCE,                       /* data format */
        GL_UNSIGNED_SHORT,                  /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR
    },
    {
        /* No GL_LUMINANCE24, FIX ME? */
        __GL_FMT_L24,                       /* drvFormat */
        GL_LUMINANCE,                       /* glFormat*/
        GL_LUMINANCE,                       /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* type */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        24,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        24, 0, 0, 0, 0, 0,                  /* r/g/b/a/d/s size */
        GL_LUMINANCE,                       /* data format */
        __GL_UNSIGNED_INT_24,               /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR
    },
    {
        __GL_FMT_I8,                        /* drvFormat */
        GL_INTENSITY8,                      /* glFormat*/
        GL_INTENSITY,                       /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* type */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        8,                                  /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        8, 0, 0, 0, 0, 0,                   /* r/g/b/a/d/s size */
        GL_INTENSITY,                       /* data format */
        GL_UNSIGNED_BYTE,                   /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR
    },
    {
        __GL_FMT_I16,                       /* drvFormat */
        GL_INTENSITY16,                     /* glFormat*/
        GL_INTENSITY,                       /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* type */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        16,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        16, 0, 0, 0, 0, 0,                  /* r/g/b/a/d/s size */
        GL_INTENSITY,                       /* data format */
        GL_UNSIGNED_SHORT,                  /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR
    },

    {
        /* No GL_INTENSITY24, FIX ME? */
        __GL_FMT_I24,                       /* drvFormat */
        GL_INTENSITY,                       /* glFormat*/
        GL_INTENSITY,                       /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* type */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        16,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        16, 0, 0, 0, 0, 0,                  /* r/g/b/a/d/s size */
        GL_INTENSITY,                       /* data format */
        __GL_UNSIGNED_INT_24,               /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR
    },

    {
        /* No GL_INTENSITYI32F, FIX ME? */
        __GL_FMT_I32F,                      /* drvFormat */
        GL_INTENSITY,                       /* glFormat*/
        GL_INTENSITY,                       /* baseFormat */
        GL_FLOAT,                           /* type */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        32,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        32, 0, 0, 0, 0, 0,                  /* r/g/b/a/d/s size */
        GL_INTENSITY,                       /* data format */
        GL_FLOAT,                           /* data type */
        0,                                  /* shared size */
        GL_FLOAT,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR
    },

    /* Patch __GL_DEVFMT_LUMINANCE_ALPHA4 format in PIXEL PIPE:
    ** corresponding hw format is A4L4.
    ** Because gl had not define a general pixel type as GL_UNSIGNED_BYTE_4_4_REV.
    ** If assign Type as GL_UNSIGNED_BYTE, glFormat as GL_LUMINANCE4_ALPHA4, texture will load to dest as
    ** L8_A8. system cache and device cache(oe device memory image) inconsistent.
    ** To describe ALPHA4_LUMINANCE4 to pixel pipe, we add a private pixel type as GL_UNSIGNED_BYTE_4_4_REV_VIVPRIV
    ** Handle in PickSpanPack/PickSpanUnPack, pixel pipe know from both pxFormat and
    ** pxType that this dest format is  A4L4..
    */
    /*__GL_DEVFMT_LUMINANCE_ALPHA4: A4L4*/
    {
        __GL_FMT_LA4,                       /* drvFormat */
        GL_LUMINANCE4_ALPHA4,               /* glFormat*/
        GL_LUMINANCE_ALPHA,                 /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* type */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        8,                                  /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        4, 0, 0, 4, 0, 0,                   /* r/g/b/a/d/s size */
        GL_LUMINANCE_ALPHA,                 /* data format */
        __GL_UNSIGNED_BYTE_4_4_REV_VIVPRIV, /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR
    },

    /*__GL_FMT_LUMINANCE_ALPHA16: L16A16*/
    {
        __GL_FMT_LA16,                      /* drvFormat */
        GL_LUMINANCE16_ALPHA16,             /* glFormat*/
        GL_LUMINANCE_ALPHA,                 /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* type */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        32,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        16, 0, 0, 16, 0, 0,                 /* r/g/b/a/d/s size */
        GL_LUMINANCE_ALPHA,                 /* data format */
        GL_UNSIGNED_SHORT,                  /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR
    },

    /*__GL_FMT_BGR565: RGB565*/
    {
        __GL_FMT_BGR565,                    /* drvFormat */
        GL_BGR,                             /* glFormat*/
        GL_BGR,                             /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* type */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        16,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        5, 6, 5, 0, 0, 0,                   /* r/g/b/a/d/s size */
        GL_BGRA,                            /* data format */
        GL_UNSIGNED_SHORT_5_6_5_REV,        /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_LINEAR
    },

    /*__GL_FMT_BGRA444: ARGB4444*/
    {
        __GL_FMT_BGRA4444,                  /* drvFormat */
        GL_BGRA,                            /* glFormat*/
        GL_BGRA,                            /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* type */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        16,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        4, 4, 4, 4, 0, 0,                   /* r/g/b/a/d/s size */
        GL_BGRA,                            /* data format */
        GL_UNSIGNED_SHORT_4_4_4_4_REV,      /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR
    },

    /*__GL_FMT_BGRA5551: ARGB1555*/
    {
        __GL_FMT_BGRA5551,                  /* drvFormat */
        GL_BGRA,                            /* glFormat*/
        GL_BGRA,                            /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* type */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        16,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        5, 5, 5, 1, 0, 0,                   /* r/g/b/a/d/s size */
        GL_BGRA,                            /* data format */
        GL_UNSIGNED_SHORT_1_5_5_5_REV,      /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR
    },

    /*__GL_FMT_RGBA16*/
    {
        __GL_FMT_RGBA16,                    /* drvFormat */
        GL_RGBA16,                          /* glFormat*/
        GL_RGBA,                            /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* type */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        64,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        16, 16, 16, 16, 0, 0,               /* r/g/b/a/d/s size */
        GL_RGBA,                            /* data format */
        GL_UNSIGNED_SHORT,                  /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR
    },

    /*__GL_FMT_BGRA1010102: ARGB1010102*/
    {
        __GL_FMT_BGRA1010102,               /* drvFormat */
        GL_BGRA,                            /* glFormat*/
        GL_BGRA,                            /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* type */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        32,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        10, 10, 10, 2, 0, 0,                /* r/g/b/a/d/s size */
        GL_BGRA,                            /* data format */
        GL_UNSIGNED_INT_2_10_10_10_REV,     /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR
    },

    /*__GL_FMT_ZHIGH24*/
    {
        __GL_FMT_ZHIGH24,                   /* drvFormat */
        GL_DEPTH_COMPONENT,                 /* glFormat*/
        GL_DEPTH_COMPONENT,                 /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* type */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        24,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        0, 0, 0, 0, 24, 0,                  /* r/g/b/a/d/s size */
        GL_DEPTH_COMPONENT,                 /* data format */
        __GL_UNSIGNED_INT_HIGH24,           /* data type */
        0,                                  /* shared size */
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_UNSIGNED_NORMALIZED,
        GL_LINEAR
    },

    /*__GL_FMT_ZHIGH24_STENCIL*/
    {
        __GL_FMT_ZHIGH24S8,                 /* drvFormat */
        GL_DEPTH_COMPONENT,                 /* glFormat*/
        GL_DEPTH_COMPONENT,                 /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* type */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        32,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        0, 0, 0, 0, 24, 8,                  /* r/g/b/a/d/s size */
        GL_DEPTH_COMPONENT,                 /* data format */
        __GL_UNSIGNED_S8_D24,               /* data type */
        0,                                  /* shared size */
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_UNSIGNED_NORMALIZED,
        GL_LINEAR
    },

    /*__GL_FMT_COMPRESSED_LUMINANCE_LATC1*/
    {
        __GL_FMT_COMPRESSED_LUMINANCE_LATC1,    /* drvFormat */
        GL_LUMINANCE,                           /* glFormat*/
        GL_LUMINANCE,                           /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                 /* type */
        GL_TRUE,                                /* compressed */
        GL_TRUE,                                /* filterable */
        GL_FALSE,                               /* renderable */
        64,                                     /* bitsPerPixel */
        4, 4,                                   /* blockW/H */
        4, 0, 0, 0, 0, 0,                       /* r/g/b/a/d/s size */
        GL_LUMINANCE,                           /* data format */
        __GL_LATC1_BLOCK,                       /* data type */
        0,                                      /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR
    },

    /*__GL_FMT_COMPRESSED_SIGNED_LUMINANCE_LATC1*/
    {
        __GL_FMT_COMPRESSED_SIGNED_LUMINANCE_LATC1,     /* drvFormat */
        GL_LUMINANCE,                                   /* glFormat*/
        GL_LUMINANCE,                                   /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                         /* type */
        GL_TRUE,                                        /* compressed */
        GL_TRUE,                                        /* filterable */
        GL_FALSE,                                       /* renderable */
        64,                                             /* bitsPerPixel */
        4, 4,                                           /* blockW/H */
        4, 0, 0, 0, 0, 0,                               /* r/g/b/a/d/s size */
        GL_LUMINANCE,                                   /* data format */
        __GL_SIGNED_LATC1_BLOCK,                        /* data type */
        0,                                              /* shared size */
        GL_SIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR
    },

    /*__GL_FMT_COMPRESSED_LUMINANCE_ALPHA_LATC2*/
    {
        __GL_FMT_COMPRESSED_LUMINANCE_ALPHA_LATC2,      /* drvFormat */
        GL_LUMINANCE_ALPHA,                             /* glFormat*/
        GL_LUMINANCE_ALPHA,                             /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                         /* type */
        GL_TRUE,                                        /* compressed */
        GL_TRUE,                                        /* filterable */
        GL_FALSE,                                       /* renderable */
        128,                                            /* bitsPerPixel */
        4, 4,                                           /* blockW/H */
        4, 0, 0, 4, 0, 0,                               /* r/g/b/a/d/s size */
        GL_LUMINANCE_ALPHA,                             /* data format */
        __GL_LATC2_BLOCK,                               /* data type */
        0,                                              /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR
    },

    /*__GL_FMT_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2*/
    {
        __GL_FMT_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2,       /* drvFormat */
        GL_LUMINANCE_ALPHA,                                     /* glFormat*/
        GL_LUMINANCE_ALPHA,                                     /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                                 /* type */
        GL_TRUE,                                                /* compressed */
        GL_TRUE,                                                /* filterable */
        GL_FALSE,                                               /* renderable */
        128,                                                    /* bitsPerPixel */
        4, 4,                                                   /* blockW/H */
        4, 0, 0, 4, 0, 0,                                       /* r/g/b/a/d/s size */
        GL_LUMINANCE_ALPHA,                                     /* data format */
        __GL_SIGNED_LATC2_BLOCK,                                /* data type */
        0,                                                      /* shared size */
        GL_SIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_SIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR
    },

    /*__GL_FMT_COMPRESSED_RED_RGTC1*/
    {
        __GL_FMT_COMPRESSED_RED_RGTC1,          /* drvFormat */
        GL_RED,                                 /* glFormat*/
        GL_RED,                                 /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                 /* type */
        GL_TRUE,                                /* compressed */
        GL_TRUE,                                /* filterable */
        GL_FALSE,                               /* renderable */
        64,                                     /* bitsPerPixel */
        4, 4,                                   /* blockW/H */
        4, 0, 0, 0, 0, 0,                       /* r/g/b/a/d/s size */
        GL_RED,                                 /* data format */
        __GL_RGTC1_BLOCK,                       /* data type */
        0,                                      /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR
    },

    /*__GL_FMT_COMPRESSED_SIGNED_RED_RGTC1*/
    {
        __GL_FMT_COMPRESSED_SIGNED_RED_RGTC1,   /* drvFormat */
        GL_RED,                                 /* glFormat*/
        GL_RED,                                 /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                 /* type */
        GL_TRUE,                                /* compressed */
        GL_TRUE,                                /* filterable */
        GL_FALSE,                               /* renderable */
        64,                                     /* bitsPerPixel */
        4, 4,                                   /* blockW/H */
        4, 0, 0, 0, 0, 0,                       /* r/g/b/a/d/s size */
        GL_RED,                                 /* data format */
        __GL_SIGNED_RGTC1_BLOCK,                /* data type */
        0,                                      /* shared size */
        GL_SIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR
    },

    /*__GL_FMT_COMPRESSED_RED_GREEN_RGTC2*/
    {
        __GL_FMT_COMPRESSED_RED_GREEN_RGTC2,    /* drvFormat */
        GL_RED_GREEN_VIVPRIV,                   /* glFormat*/
        GL_RED_GREEN_VIVPRIV,                   /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                 /* type */
        GL_TRUE,                                /* compressed */
        GL_TRUE,                                /* filterable */
        GL_FALSE,                               /* renderable */
        128,                                    /* bitsPerPixel */
        4, 4,                                   /* blockW/H */
        4, 4, 0, 0, 0, 0,                       /* r/g/b/a/d/s size */
        GL_RED_GREEN_VIVPRIV,                   /* data format */
        __GL_RGTC2_BLOCK,                       /* data type */
        0,                                      /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR
    },

    /*__GL_FMT_COMPRESSED_SIGNED_RED_GREEN_RGTC2*/
    {
        __GL_FMT_COMPRESSED_SIGNED_RED_GREEN_RGTC2,     /* drvFormat */
        GL_RED_GREEN_VIVPRIV,                           /* glFormat*/
        GL_RED_GREEN_VIVPRIV,                           /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                         /* type */
        GL_TRUE,                                        /* compressed */
        GL_TRUE,                                        /* filterable */
        GL_FALSE,                                       /* renderable */
        128,                                            /* bitsPerPixel */
        4, 4,                                           /* blockW/H */
        4, 4, 0, 0, 0, 0,                               /* r/g/b/a/d/s size */
        GL_RED_GREEN_VIVPRIV,                           /* data format */
        __GL_SIGNED_RGTC2_BLOCK,                        /* data type */
        0,                                              /* shared size */
        GL_SIGNED_NORMALIZED,
        GL_SIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR
    },

    /*__GL_FMT_RGBA8888_SIGNED*/
    {
        __GL_FMT_RGBA8888_SIGNED,           /* drvFormat */
        GL_RGBA,                            /* glFormat*/
        GL_RGBA,                            /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* type */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        32,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        8, 8, 8, 8, 0, 0,                   /* r/g/b/a/d/s size */
        GL_RGBA,                            /* data format */
        GL_BYTE,                            /* data type */
        0,                                  /* shared size */
        GL_SIGNED_NORMALIZED,
        GL_SIGNED_NORMALIZED,
        GL_SIGNED_NORMALIZED,
        GL_SIGNED_NORMALIZED,
        GL_NONE,
        GL_LINEAR
    },

    /*__GL_FMT_SRGB_ALPHA*/
    {
        __GL_FMT_SRGB_ALPHA,                /* drvFormat */
        GL_RGBA,                            /* glFormat*/
        GL_RGBA,                            /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* type */
        GL_FALSE,                           /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        32,                                 /* bitsPerPixel */
        1, 1,                               /* blockW/H */
        8, 8, 8, 8, 0, 0,                   /* r/g/b/a/d/s size */
        GL_RGBA,                            /* data format */
        GL_UNSIGNED_BYTE,                   /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_SRGB
    },

    /*__GL_FMT_COMPRESSED_SRGB_S3TC_DXT1*/
    {
        __GL_FMT_COMPRESSED_SRGB_S3TC_DXT1, /* drvFormat */
        GL_SRGB,                            /* glFormat*/
        GL_SRGB,                            /* baseFormat */
        GL_UNSIGNED_NORMALIZED,             /* type */
        GL_TRUE,                            /* compressed */
        GL_TRUE,                            /* filterable */
        GL_FALSE,                           /* renderable */
        64,                                 /* bitsPerPixel */
        4, 4,                               /* blockW/H */
        5, 5, 5, 0, 0, 0,                   /* r/g/b/a/d/s size */
        GL_SRGB,                            /* data format */
        __GL_DXT1_BLOCK,                    /* data type */
        0,                                  /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_NONE,
        GL_SRGB
    },

    /*__GL_FMT_COMPRESSED_SRGB_ALPHA_S3TC_DXT1*/
    {
        __GL_FMT_COMPRESSED_SRGB_ALPHA_S3TC_DXT1,   /* drvFormat */
        GL_RGBA,                                    /* glFormat*/
        GL_RGBA,                                    /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                     /* type */
        GL_TRUE,                                    /* compressed */
        GL_TRUE,                                    /* filterable */
        GL_FALSE,                                   /* renderable */
        64,                                         /* bitsPerPixel */
        4, 4,                                       /* blockW/H */
        5, 5, 5, 1, 0, 0,                           /* r/g/b/a/d/s size */
        GL_RGBA,                                    /* data format */
        __GL_DXT1A_BLOCK,                           /* data type */
        0,                                          /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_SRGB
    },

    /*__GL_FMT_COMPRESSED_SRGB_ALPHA_S3TC_DXT3*/
    {
        __GL_FMT_COMPRESSED_SRGB_ALPHA_S3TC_DXT3,   /* drvFormat */
        GL_RGBA,                                    /* glFormat*/
        GL_RGBA,                                    /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                     /* type */
        GL_TRUE,                                    /* compressed */
        GL_TRUE,                                    /* filterable */
        GL_FALSE,                                   /* renderable */
        128,                                        /* bitsPerPixel */
        4, 4,                                       /* blockW/H */
        4, 4, 4, 8, 0, 0,                           /* r/g/b/a/d/s size */
        GL_RGBA,                                    /* data format */
        __GL_DXT3_BLOCK,                            /* data type */
        0,                                          /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_SRGB
    },

    /*__GL_FMT_COMPRESSED_SRGB_ALPHA_S3TC_DXT5*/
    {
        __GL_FMT_COMPRESSED_SRGB_ALPHA_S3TC_DXT5,   /* drvFormat */
        GL_RGBA,                                    /* glFormat*/
        GL_RGBA,                                    /* baseFormat */
        GL_UNSIGNED_NORMALIZED,                     /* type */
        GL_TRUE,                                    /* compressed */
        GL_TRUE,                                    /* filterable */
        GL_FALSE,                                   /* renderable */
        128,                                        /* bitsPerPixel */
        4, 4,                                       /* blockW/H */
        4, 4, 4, 8, 0, 0,                           /* r/g/b/a/d/s size */
        GL_RGBA,                                    /* data format */
        __GL_DXT5_BLOCK,                            /* data type */
        0,                                          /* shared size */
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_NONE,
        GL_SRGB
    },
#endif

    /* NULL format */
    {
        __GL_FMT_MAX,                       /* drvFormat */
        GL_RGBA,                            /* glFormat*/
        GL_RGBA,                            /* baseFormat */
        GL_NONE,                            /* category */
        GL_FALSE,                           /* compressed */
        GL_FALSE,                           /* filterable */
        GL_FALSE,                           /* renderable */
        0,                                  /* bitsPerPixel */
        0, 0, 0, 0, 0, 0,                   /* r/g/b/a/d/s size */
        GL_NONE,                            /* data format */
        GL_NONE,                            /* data type */
        0,                                  /* shared size */
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_LINEAR,
    },
};

__GLformatInfo* __glGetFormatInfo(GLenum internalFormat)
{
    __GLformat drvFormat = __GL_FMT_MAX;

    /* use switch instead of loop to accelerate the lookup */
    switch (internalFormat)
    {
    case GL_ALPHA:
    case GL_ALPHA8_OES:
        drvFormat = __GL_FMT_A8;
        break;
    case GL_LUMINANCE:
    case GL_LUMINANCE8_OES:
        drvFormat = __GL_FMT_L8;
        break;
    case GL_LUMINANCE_ALPHA:
    case GL_LUMINANCE4_ALPHA4_OES:
    case GL_LUMINANCE8_ALPHA8_OES:
        drvFormat = __GL_FMT_LA8;
        break;

    case GL_R8:
        drvFormat = __GL_FMT_R8;
        break;
    case GL_R8_SNORM:
        drvFormat = __GL_FMT_R8_SNORM;
        break;
    case GL_RG8:
        drvFormat = __GL_FMT_RG8;
        break;
    case GL_RG8_SNORM:
        drvFormat = __GL_FMT_RG8_SNORM;
        break;
    case GL_RGB:
    case GL_RGB8:
        drvFormat = __GL_FMT_RGB8;
        break;
    case GL_RGB8_SNORM:
        drvFormat = __GL_FMT_RGB8_SNORM;
        break;
    case GL_RGB565:
        drvFormat = __GL_FMT_RGB565;
        break;
    case GL_RGBA4:
        drvFormat = __GL_FMT_RGBA4;
        break;
    case GL_RGB5_A1:
        drvFormat = __GL_FMT_RGB5_A1;
        break;
    case GL_RGBA:
    case GL_RGBA8:
        drvFormat = __GL_FMT_RGBA8;
        break;
    case GL_BGRA_EXT:
        drvFormat = __GL_FMT_BGRA;
        break;
    case GL_RGBA8_SNORM:
        drvFormat = __GL_FMT_RGBA8_SNORM;
        break;
    case GL_RGB10_EXT:
    case GL_RGB10_A2:
        drvFormat = __GL_FMT_RGB10_A2;
        break;
    case GL_SRGB8:
        drvFormat = __GL_FMT_SRGB8;
        break;
    case GL_SRGB8_ALPHA8:
        drvFormat = __GL_FMT_SRGB8_ALPHA8;
        break;
    case GL_R16F:
        drvFormat = __GL_FMT_R16F;
        break;
    case GL_RG16F:
        drvFormat = __GL_FMT_RG16F;
        break;
    case GL_RGB16F:
        drvFormat = __GL_FMT_RGB16F;
        break;
    case GL_RGBA16F:
        drvFormat = __GL_FMT_RGBA16F;
        break;
    case GL_R32F:
        drvFormat = __GL_FMT_R32F;
        break;
    case GL_RG32F:
        drvFormat = __GL_FMT_RG32F;
        break;
    case GL_RGB32F:
        drvFormat = __GL_FMT_RGB32F;
        break;
    case GL_RGBA32F:
        drvFormat = __GL_FMT_RGBA32F;
        break;
    case GL_R11F_G11F_B10F:
        drvFormat = __GL_FMT_R11F_G11F_B10F;
        break;
    case GL_RGB9_E5:
        drvFormat = __GL_FMT_RGB9_E5;
        break;
    case GL_R8I:
        drvFormat = __GL_FMT_R8I;
        break;
    case GL_R8UI:
        drvFormat = __GL_FMT_R8UI;
        break;
    case GL_R16I:
        drvFormat = __GL_FMT_R16I;
        break;
    case GL_R16UI:
        drvFormat = __GL_FMT_R16UI;
        break;
    case GL_R32I:
        drvFormat = __GL_FMT_R32I;
        break;
    case GL_R32UI:
        drvFormat = __GL_FMT_R32UI;
        break;
    case GL_RG8I:
        drvFormat = __GL_FMT_RG8I;
        break;
    case GL_RG8UI:
        drvFormat = __GL_FMT_RG8UI;
        break;
    case GL_RG16I:
        drvFormat = __GL_FMT_RG16I;
        break;
    case GL_RG16UI:
        drvFormat = __GL_FMT_RG16UI;
        break;
    case GL_RG32I:
        drvFormat = __GL_FMT_RG32I;
        break;
    case GL_RG32UI:
        drvFormat = __GL_FMT_RG32UI;
        break;
    case GL_RGB8I:
        drvFormat = __GL_FMT_RGB8I;
        break;
    case GL_RGB8UI:
        drvFormat = __GL_FMT_RGB8UI;
        break;
    case GL_RGB16I:
        drvFormat = __GL_FMT_RGB16I;
        break;
    case GL_RGB16UI:
        drvFormat = __GL_FMT_RGB16UI;
        break;
    case GL_RGB32I:
        drvFormat = __GL_FMT_RGB32I;
        break;
    case GL_RGB32UI:
        drvFormat = __GL_FMT_RGB32UI;
        break;
    case GL_RGBA8I:
        drvFormat = __GL_FMT_RGBA8I;
        break;
    case GL_RGBA8UI:
        drvFormat = __GL_FMT_RGBA8UI;
        break;
    case GL_RGBA16I:
        drvFormat = __GL_FMT_RGBA16I;
        break;
    case GL_RGBA16UI:
        drvFormat = __GL_FMT_RGBA16UI;
        break;
    case GL_RGBA32I:
        drvFormat = __GL_FMT_RGBA32I;
        break;
    case GL_RGBA32UI:
        drvFormat = __GL_FMT_RGBA32UI;
        break;
    case GL_RGB10_A2UI:
        drvFormat = __GL_FMT_RGB10_A2UI;
        break;

    /* compressed formats */
    case GL_ETC1_RGB8_OES:
        drvFormat = __GL_FMT_ETC1_RGB8_OES;
        break;

    case GL_COMPRESSED_R11_EAC:
        drvFormat = __GL_FMT_R11_EAC;
        break;

    case GL_COMPRESSED_SIGNED_R11_EAC:
        drvFormat = __GL_FMT_SIGNED_R11_EAC;
        break;

    case GL_COMPRESSED_RG11_EAC:
        drvFormat = __GL_FMT_RG11_EAC;
        break;

    case GL_COMPRESSED_SIGNED_RG11_EAC:
        drvFormat = __GL_FMT_SIGNED_RG11_EAC;
        break;
    case GL_COMPRESSED_RGB8_ETC2:
        drvFormat = __GL_FMT_RGB8_ETC2;
        break;

    case GL_COMPRESSED_SRGB8_ETC2:
        drvFormat = __GL_FMT_SRGB8_ETC2;
        break;

    case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
        drvFormat = __GL_FMT_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
        break;

    case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
        drvFormat = __GL_FMT_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2;
        break;

    case GL_COMPRESSED_RGBA8_ETC2_EAC:
        drvFormat = __GL_FMT_RGBA8_ETC2_EAC;
        break;

    case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
         drvFormat = __GL_FMT_SRGB8_ALPHA8_ETC2_EAC;
         break;

    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        drvFormat = __GL_FMT_COMPRESSED_RGB_S3TC_DXT1_EXT;
        break;
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        drvFormat = __GL_FMT_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        break;
    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        drvFormat = __GL_FMT_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        break;
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        drvFormat = __GL_FMT_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        break;
    case GL_PALETTE4_RGBA4_OES:
        drvFormat = __GL_FMT_PALETTE4_RGBA4_OES;
        break;
    case GL_PALETTE4_RGB5_A1_OES:
        drvFormat = __GL_FMT_PALETTE4_RGB5_A1_OES;
        break;
    case GL_PALETTE4_R5_G6_B5_OES:
        drvFormat = __GL_FMT_PALETTE4_R5_G6_B5_OES;
        break;
    case GL_PALETTE4_RGB8_OES:
        drvFormat = __GL_FMT_PALETTE4_RGB8_OES;
        break;
    case GL_PALETTE4_RGBA8_OES:
        drvFormat = __GL_FMT_PALETTE4_RGBA8_OES;
        break;
    case GL_PALETTE8_RGBA4_OES:
        drvFormat = __GL_FMT_PALETTE8_RGBA4_OES;
        break;
    case GL_PALETTE8_RGB5_A1_OES:
        drvFormat = __GL_FMT_PALETTE8_RGB5_A1_OES;
        break;
    case GL_PALETTE8_R5_G6_B5_OES:
        drvFormat = __GL_FMT_PALETTE8_R5_G6_B5_OES;
        break;
    case GL_PALETTE8_RGB8_OES:
        drvFormat = __GL_FMT_PALETTE8_RGB8_OES;
        break;
    case GL_PALETTE8_RGBA8_OES:
        drvFormat = __GL_FMT_PALETTE8_RGBA8_OES;
        break;

    /* depth stencil formats */
    case GL_DEPTH_COMPONENT16:
        drvFormat = __GL_FMT_Z16;
        break;
    case GL_DEPTH_COMPONENT:
    case GL_DEPTH_COMPONENT24:
    case GL_DEPTH_COMPONENT32_OES:
        drvFormat = __GL_FMT_Z24;
        break;
    case GL_DEPTH_COMPONENT32F:
        drvFormat = __GL_FMT_Z32F;
        break;
    case GL_DEPTH_STENCIL_OES:
    case GL_DEPTH24_STENCIL8:
        drvFormat = __GL_FMT_Z24S8;
        break;
    case GL_DEPTH32F_STENCIL8:
        drvFormat = __GL_FMT_Z32FS8;
        break;
    case GL_STENCIL_INDEX1_OES:
        drvFormat = __GL_FMT_S1;
        break;
    case GL_STENCIL_INDEX4_OES:
        drvFormat = __GL_FMT_S4;
        break;
    case GL_STENCIL:
    case GL_STENCIL_INDEX8:
        drvFormat = __GL_FMT_S8;
        break;

#if defined(GL_KHR_texture_compression_astc_ldr)
    case GL_COMPRESSED_RGBA_ASTC_4x4_KHR:
    case GL_COMPRESSED_RGBA_ASTC_5x4_KHR:
    case GL_COMPRESSED_RGBA_ASTC_5x5_KHR:
    case GL_COMPRESSED_RGBA_ASTC_6x5_KHR:
    case GL_COMPRESSED_RGBA_ASTC_6x6_KHR:
    case GL_COMPRESSED_RGBA_ASTC_8x5_KHR:
    case GL_COMPRESSED_RGBA_ASTC_8x6_KHR:
    case GL_COMPRESSED_RGBA_ASTC_8x8_KHR:
    case GL_COMPRESSED_RGBA_ASTC_10x5_KHR:
    case GL_COMPRESSED_RGBA_ASTC_10x6_KHR:
    case GL_COMPRESSED_RGBA_ASTC_10x8_KHR:
    case GL_COMPRESSED_RGBA_ASTC_10x10_KHR:
    case GL_COMPRESSED_RGBA_ASTC_12x10_KHR:
    case GL_COMPRESSED_RGBA_ASTC_12x12_KHR:
        drvFormat = (__GLformat) (__GL_FMT_COMPRESSED_RGBA_ASTC_4x4_KHR
                  + (internalFormat - GL_COMPRESSED_RGBA_ASTC_4x4_KHR));
        break;

    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR:
        drvFormat = (__GLformat) (__GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR
                  + (internalFormat - GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR));
        break;
#endif

    case __GL_RGBX8:
        drvFormat = __GL_FMT_RGBX8;
        break;
    case __GL_BGRX8:
        drvFormat = __GL_FMT_BGRX8;
        break;

#ifdef OPENGL40
    case 1:
    case GL_LUMINANCE4:
        drvFormat = __GL_FMT_L8;
        break;

    case GL_LUMINANCE12:
    case GL_LUMINANCE16:
        drvFormat = __GL_FMT_L16;
        break;

    case GL_LUMINANCE32UI_EXT:
        drvFormat = __GL_FMT_RGBA32UI;
        break;
    case GL_LUMINANCE32I_EXT:
        drvFormat = __GL_FMT_RGBA32I;
        break;
    case GL_LUMINANCE16UI_EXT:
        drvFormat = __GL_FMT_RGBA16UI;
        break;
    case GL_LUMINANCE16I_EXT:
        drvFormat = __GL_FMT_RGBA16I;
        break;
    case GL_LUMINANCE8UI_EXT:
        drvFormat = __GL_FMT_RGBA8UI;
        break;
    case GL_LUMINANCE8I_EXT:
        drvFormat = __GL_FMT_RGBA8I;
        break;

    case GL_LUMINANCE32F_ARB:
        drvFormat = __GL_FMT_L32F;
        break;

    case GL_LUMINANCE16F_ARB:
        drvFormat = __GL_FMT_L32F;
        break;

    case GL_COMPRESSED_LUMINANCE_ARB:
    case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
        drvFormat = __GL_FMT_COMPRESSED_LUMINANCE_LATC1;
        break;

    case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
        drvFormat = __GL_FMT_COMPRESSED_SIGNED_LUMINANCE_LATC1;
        break;

    case GL_COMPRESSED_LUMINANCE_ALPHA_ARB:
    case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
        drvFormat = __GL_FMT_COMPRESSED_LUMINANCE_ALPHA_LATC2;
        break;

    case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
        drvFormat = __GL_FMT_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2;
        break;

    case 2:
    case GL_LUMINANCE6_ALPHA2:
        drvFormat = __GL_FMT_LA8;
        break;

    case GL_LUMINANCE12_ALPHA4:
    case GL_LUMINANCE12_ALPHA12:
    case GL_LUMINANCE16_ALPHA16:
        drvFormat = __GL_FMT_LA16;
        break;

    case GL_LUMINANCE_ALPHA32UI_EXT:
        drvFormat = __GL_FMT_RGBA32UI;
        break;
    case GL_LUMINANCE_ALPHA32I_EXT:
        drvFormat = __GL_FMT_RGBA32I;
        break;
    case GL_LUMINANCE_ALPHA16UI_EXT:
        drvFormat = __GL_FMT_RGBA16UI;
        break;
    case GL_LUMINANCE_ALPHA16I_EXT:
        drvFormat = __GL_FMT_RGBA16I;
        break;
    case GL_LUMINANCE_ALPHA8UI_EXT:
        drvFormat = __GL_FMT_RGBA8UI;
        break;
    case GL_LUMINANCE_ALPHA8I_EXT:
        drvFormat = __GL_FMT_RGBA8I;
        break;

    case GL_LUMINANCE_ALPHA32F_ARB:
        drvFormat = __GL_FMT_RGBA32F;
        break;

    case GL_LUMINANCE_ALPHA16F_ARB:
        drvFormat = __GL_FMT_RGBA16F;
        break;

    case GL_R3_G3_B2:
    case GL_RGB4:
    case GL_RGB5:
        drvFormat = __GL_FMT_BGR565;
        break;

    case 3:
    case GL_RGB12:
    case GL_RGB16:
        drvFormat = __GL_FMT_RGBA8;
        break;

    case GL_BGRA8_VIVPRIV:
        drvFormat = __GL_FMT_BGRA;
        break;

    case GL_RGBA2:
        drvFormat = __GL_FMT_BGRA4444;
        break;

    case 4:
        drvFormat = __GL_FMT_RGBA8;
        break;

    case GL_RGBA12:
    case GL_RGBA16:
        drvFormat = __GL_FMT_RGBA16;
        break;

    case GL_ALPHA4:
    case GL_COMPRESSED_ALPHA_ARB:
        drvFormat = __GL_FMT_A8;
        break;

    case GL_ALPHA12:
    case GL_ALPHA16:
        drvFormat = __GL_FMT_A16;
        break;

    case GL_ALPHA32UI_EXT:
        drvFormat = __GL_FMT_RGBA32UI;
        break;
    case GL_ALPHA32I_EXT:
        drvFormat = __GL_FMT_RGBA32I;
        break;
    case GL_ALPHA16UI_EXT:
        drvFormat = __GL_FMT_RGBA16UI;
        break;
    case GL_ALPHA16I_EXT:
        drvFormat = __GL_FMT_RGBA16I;
        break;
    case GL_ALPHA8UI_EXT:
        drvFormat = __GL_FMT_RGBA8UI;
        break;
    case GL_ALPHA8I_EXT:
        drvFormat = __GL_FMT_RGBA8I;
        break;

    case GL_ALPHA32F_ARB:
        drvFormat = __GL_FMT_A32F;
        break;
    case GL_ALPHA16F_ARB:
        drvFormat = __GL_FMT_A32F;
        break;

    case GL_INTENSITY:
    case GL_INTENSITY4:
    case GL_INTENSITY8:
        drvFormat = __GL_FMT_I8;
        break;

    case GL_INTENSITY12:
    case GL_INTENSITY16:
    case GL_COMPRESSED_INTENSITY_ARB:
        drvFormat = __GL_FMT_I16;
        break;

    case GL_INTENSITY32UI_EXT:
        drvFormat = __GL_FMT_RGBA32UI;
        break;
    case GL_INTENSITY32I_EXT:
        drvFormat = __GL_FMT_RGBA32I;
        break;
    case GL_INTENSITY16UI_EXT:
        drvFormat = __GL_FMT_RGBA16UI;
        break;
    case GL_INTENSITY16I_EXT:
        drvFormat = __GL_FMT_RGBA16I;
        break;
    case GL_INTENSITY8UI_EXT:
        drvFormat = __GL_FMT_RGBA8UI;
        break;
    case GL_INTENSITY8I_EXT:
        drvFormat = __GL_FMT_RGBA8I;
        break;

    case GL_INTENSITY32F_ARB:
        drvFormat = __GL_FMT_I32F;
        break;

    case GL_INTENSITY16F_ARB:
        drvFormat = __GL_FMT_I32F;
        break;

    case GL_RGB_S3TC:
    case GL_RGB4_S3TC:
    case GL_COMPRESSED_RGB_ARB:
        drvFormat = __GL_FMT_COMPRESSED_RGB_S3TC_DXT1_EXT;
        break;

    case GL_RGBA_S3TC:
    case GL_RGBA4_S3TC:
        drvFormat = __GL_FMT_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        break;

    case GL_RGBA_DXT5_S3TC:
    case GL_RGBA4_DXT5_S3TC:
    case GL_COMPRESSED_RGBA_ARB:
        drvFormat = __GL_FMT_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        break;

    case GL_COMPRESSED_RED_RGTC1_EXT:
        drvFormat = __GL_FMT_COMPRESSED_RED_RGTC1;
        break;
    case GL_COMPRESSED_SIGNED_RED_RGTC1_EXT:
        drvFormat = __GL_FMT_COMPRESSED_SIGNED_RED_RGTC1;
        break;
    case GL_COMPRESSED_RED_GREEN_RGTC2_EXT:
        drvFormat = __GL_FMT_COMPRESSED_RED_GREEN_RGTC2;
        break;
    case GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT:
        drvFormat = __GL_FMT_COMPRESSED_SIGNED_RED_GREEN_RGTC2;
        break;

    case GL_STENCIL_INDEX:
    case GL_STENCIL_INDEX16_EXT:
        drvFormat = __GL_FMT_S8;
        break;

    case GL_SRGB:
    case GL_SRGB_ALPHA:
    case GL_SLUMINANCE_ALPHA:
    case GL_SLUMINANCE8_ALPHA8:
    case GL_SLUMINANCE:
    case GL_SLUMINANCE8:
    case GL_COMPRESSED_SRGB:
    case GL_COMPRESSED_SRGB_ALPHA:
    case GL_COMPRESSED_SLUMINANCE:
    case GL_COMPRESSED_SLUMINANCE_ALPHA:
        drvFormat = __GL_FMT_SRGB_ALPHA;
        break;
    case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
        drvFormat = __GL_FMT_COMPRESSED_SRGB_S3TC_DXT1;
        break;
    case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
        drvFormat = __GL_FMT_COMPRESSED_SRGB_ALPHA_S3TC_DXT1;
        break;
    case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
        drvFormat = __GL_FMT_COMPRESSED_SRGB_ALPHA_S3TC_DXT3;
        break;
    case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
        drvFormat = __GL_FMT_COMPRESSED_SRGB_ALPHA_S3TC_DXT5;
        break;
#endif

    default:
        drvFormat = __GL_FMT_MAX;
        break;

    }

    return &__glFormatInfoTable[drvFormat];
}

/*
** Calculate pixel byteSize base on external format and types
*/
GLuint __glPixelSize(__GLcontext *gc, GLenum format, GLenum type)
{
    GLuint compSize = 0;
    GLuint compNumber = 0;
    GLboolean packedComp = GL_FALSE;

    switch (type)
    {
    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
        compSize = 1;
        break;

    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
    case GL_HALF_FLOAT:
        compSize = 2;
        break;

    case GL_INT:
    case GL_UNSIGNED_INT:
    case GL_FLOAT:
        compSize = 4;
        break;

    case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
        compSize = 8;
        packedComp = GL_TRUE;
        break;

    case GL_UNSIGNED_INT_2_10_10_10_REV:
    case GL_UNSIGNED_INT_10F_11F_11F_REV:
    case GL_UNSIGNED_INT_5_9_9_9_REV:
    case GL_INT_2_10_10_10_REV:
    case GL_UNSIGNED_INT_24_8:
        compSize = 4;
        packedComp = GL_TRUE;
        break;

    case GL_UNSIGNED_SHORT_5_6_5:
    case GL_UNSIGNED_SHORT_4_4_4_4:
    case GL_UNSIGNED_SHORT_5_5_5_1:
        compSize = 2;
        packedComp = GL_TRUE;
        break;

    default:
        break;
    }

    switch (format)
    {
    case GL_ALPHA:
    case GL_RED:
    case GL_RED_INTEGER:
    case GL_DEPTH_COMPONENT:
    case GL_LUMINANCE:
    case GL_STENCIL:
        compNumber = 1;
        break;

    case GL_RG:
    case GL_RG_INTEGER:
    case GL_DEPTH_STENCIL:
    case GL_LUMINANCE_ALPHA:
        compNumber = 2;
        break;

    case GL_RGB:
    case GL_RGB_INTEGER:
        compNumber = 3;
        break;

    case GL_RGBA:
    case GL_RGBA_INTEGER:
    case GL_BGRA_EXT:
        compNumber = 4;
        break;

    default:
        break;
    }

    if (packedComp)
    {
        return compSize;

    }
    else
    {
        return (compSize * compNumber);
    }

}

