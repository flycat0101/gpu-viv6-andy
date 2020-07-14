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


#ifdef OPENGL40
#ifndef __gc_gl_pixel_h_
#define __gc_gl_pixel_h_

#include "gc_es_context.h"

/*
** Pixel format not explicitly defined by the spec, but implicitly suggested.
** (for processing luminance alpha texture images)
*/

#define __GL_N_PIXEL_MAPS       (GL_PIXEL_MAP_A_TO_A - GL_PIXEL_MAP_I_TO_I + 1)

#define __GL_REMAP_PM(x)        ((x) - GL_PIXEL_MAP_I_TO_I)
#define __GL_PIXEL_MAP_I_TO_I   __GL_REMAP_PM(GL_PIXEL_MAP_I_TO_I)
#define __GL_PIXEL_MAP_S_TO_S   __GL_REMAP_PM(GL_PIXEL_MAP_S_TO_S)
#define __GL_PIXEL_MAP_I_TO_R   __GL_REMAP_PM(GL_PIXEL_MAP_I_TO_R)
#define __GL_PIXEL_MAP_I_TO_G   __GL_REMAP_PM(GL_PIXEL_MAP_I_TO_G)
#define __GL_PIXEL_MAP_I_TO_B   __GL_REMAP_PM(GL_PIXEL_MAP_I_TO_B)
#define __GL_PIXEL_MAP_I_TO_A   __GL_REMAP_PM(GL_PIXEL_MAP_I_TO_A)
#define __GL_PIXEL_MAP_R_TO_R   __GL_REMAP_PM(GL_PIXEL_MAP_R_TO_R)
#define __GL_PIXEL_MAP_G_TO_G   __GL_REMAP_PM(GL_PIXEL_MAP_G_TO_G)
#define __GL_PIXEL_MAP_B_TO_B   __GL_REMAP_PM(GL_PIXEL_MAP_B_TO_B)
#define __GL_PIXEL_MAP_A_TO_A   __GL_REMAP_PM(GL_PIXEL_MAP_A_TO_A)



/************************************************************************/
/* Bitmasks for pixelModeFlags */
#define __GL_PIXEL_MODE_INDEX_LOOKUP                                     (1 <<  0)
#define __GL_PIXEL_MODE_STENCIL_LOOKUP                                   (1 <<  1)
#define __GL_PIXEL_MODE_INDEX_SHIFT_OFFSET                               (1 <<  2)
#define __GL_PIXEL_MODE_RGBA_LOOKUP                                      (1 <<  3)
#define __GL_PIXEL_MODE_RGBA_SCALE_BIAS                                  (1 <<  4)
#define __GL_PIXEL_MODE_DEPTH_SCALE_BIAS                                 (1 <<  5)
#define __GL_PIXEL_MODE_COLOR_TABLE                                      (1 <<  6)
#define __GL_PIXEL_MODE_CONVOLUTION                                      (1 <<  7)
#define __GL_PIXEL_MODE_POST_CONVOLUTION_SCALE_BIAS                      (1 <<  8)
#define __GL_PIXEL_MODE_POST_CONVOLUTION_COLOR_TABLE                     (1 <<  9)
#define __GL_PIXEL_MODE_COLOR_MATRIX                                     (1 << 10)
#define __GL_PIXEL_MODE_POST_COLOR_MATRIX_SCALE_BIAS                     (1 << 11)
#define __GL_PIXEL_MODE_POST_COLOR_MATRIX_COLOR_TABLE                    (1 << 12)
#define __GL_PIXEL_HISTOGRAM                                             (1 << 13)
#define __GL_PIXEL_MINMAX                                                (1 << 14)

#define _GL_NUMBER_OF_PIXEL_MODES   (14)
#define __GL_MAX_SPAN_SIZE      (__GL_MAX_MAX_VIEWPORT * 4 * sizeof(GLfloat) *4)

/************************************************************************/
/*
** macro define for color pixel transfer enabled and depth pixel transfer enabled
*/
#define __GL_PIXELTRANSFER_ENABLED_COLOR(gc)            \
    ((gc)->state.pixel.transferMode.r_scale != 1.0 ||   \
     (gc)->state.pixel.transferMode.g_scale != 1.0 ||   \
     (gc)->state.pixel.transferMode.b_scale != 1.0 ||   \
     (gc)->state.pixel.transferMode.a_scale != 1.0 ||   \
     (gc)->state.pixel.transferMode.r_bias != 0.0 ||    \
     (gc)->state.pixel.transferMode.g_bias != 0.0 ||    \
     (gc)->state.pixel.transferMode.b_bias != 0.0 ||    \
     (gc)->state.pixel.transferMode.a_bias != 0.0 ||    \
     (gc)->state.pixel.transferMode.mapColor ||         \
     (gc)->transform.color->matrix.matrixType != __GL_MT_IDENTITY)

#define __GL_PIXELTRANSFER_ENABLED(gc)            \
    ((gc)->state.pixel.transferMode.r_scale != __glOne  ||   \
     (gc)->state.pixel.transferMode.g_scale != __glOne  ||   \
     (gc)->state.pixel.transferMode.b_scale != __glOne  ||   \
     (gc)->state.pixel.transferMode.a_scale != __glOne  ||   \
     (gc)->state.pixel.transferMode.r_bias  != __glZero ||   \
     (gc)->state.pixel.transferMode.g_bias  != __glZero ||   \
     (gc)->state.pixel.transferMode.b_bias  != __glZero ||   \
     (gc)->state.pixel.transferMode.a_bias  != __glZero)

#define __GL_SAVE_AND_SET_SCALE_BIAS(gc,transferInfo)            \
    {transferInfo.scale.r = (gc)->state.pixel.transferMode.r_scale;\
     transferInfo.scale.g = (gc)->state.pixel.transferMode.g_scale;\
     transferInfo.scale.b = (gc)->state.pixel.transferMode.b_scale;\
     transferInfo.scale.a = (gc)->state.pixel.transferMode.a_scale;\
     transferInfo.bias.r  = (gc)->state.pixel.transferMode.r_bias;\
     transferInfo.bias.g  = (gc)->state.pixel.transferMode.g_bias;\
     transferInfo.bias.b  = (gc)->state.pixel.transferMode.b_bias;\
     transferInfo.bias.a  = (gc)->state.pixel.transferMode.a_bias;\
     (gc)->state.pixel.transferMode.r_scale = __glOne;    \
     (gc)->state.pixel.transferMode.g_scale = __glOne;    \
     (gc)->state.pixel.transferMode.b_scale = __glOne;    \
     (gc)->state.pixel.transferMode.a_scale = __glOne;    \
     (gc)->state.pixel.transferMode.r_bias  = __glZero;   \
     (gc)->state.pixel.transferMode.g_bias  = __glZero;   \
     (gc)->state.pixel.transferMode.b_bias  = __glZero;   \
     (gc)->state.pixel.transferMode.a_bias  = __glZero;}

#define __GL_REVERT_SCALE_BIAS(gc,transferInfo)            \
    {(gc)->state.pixel.transferMode.r_scale = transferInfo.scale.r;\
     (gc)->state.pixel.transferMode.g_scale = transferInfo.scale.g;\
     (gc)->state.pixel.transferMode.b_scale = transferInfo.scale.b;\
     (gc)->state.pixel.transferMode.a_scale = transferInfo.scale.a;\
     (gc)->state.pixel.transferMode.r_bias  = transferInfo.bias.r;\
     (gc)->state.pixel.transferMode.g_bias  = transferInfo.bias.g;\
     (gc)->state.pixel.transferMode.b_bias  = transferInfo.bias.b;\
     (gc)->state.pixel.transferMode.a_bias  = transferInfo.bias.a;}

#define __GL_PIXELTRANSFER_ENABLED_DEPTH(gc)            \
    ((gc)->state.pixel.transferMode.d_scale != 1.0 ||   \
     (gc)->state.pixel.transferMode.d_bias != 0.0)
/************************************************************************/

/*
** This structure is used only for communication with the PickSpanModifiers
** routine.  The first set of fields contain information that is passed
** into the routine; the second set of fields contain information that
** is passed back to the calling routine.
*/
#define __GL_SRCDST_FB             1
#define __GL_SRCDST_MEM            2

#define __GL_PIXPATH_DRAWPIX       1
#define __GL_PIXPATH_READPIX       2
#define __GL_PIXPATH_COPYPIX       3
#define __GL_PIXPATH_READIMAGE     4
#define __GL_PIXPATH_COPYIMAGE     5
#define __GL_PIXPATH_ACCUACCU      6
#define __GL_PIXPATH_ACCULOAD      7
#define __GL_PIXPATH_ACCUADD       8
#define __GL_PIXPATH_ACCUMULT      9
#define __GL_PIXPATH_ACCURETURN    10

/*
** dp.RasterBegin: rasterOp
*/
#define __GL_RASTERFUNC_DRAWPIX    1
#define __GL_RASTERFUNC_READPIX    2
#define __GL_RASTERFUNC_COPYPIX    3
#define __GL_RASTERFUNC_BITMAP     4

typedef struct __GLpixelMapHeadRec {
    GLint size;
    GLint tableId;
    union {
        GLint *mapI;          /* access index (integral) entries */
        GLfloat *mapF;        /* access component (float) entries */
    } base;
} __GLpixelMapHead;

typedef struct __GLpixelTransferModeRec {
    GLfloat r_scale, g_scale, b_scale, a_scale, d_scale;
    GLfloat r_bias, g_bias, b_bias, a_bias, d_bias;
    GLfloat zoomX;
    GLfloat zoomY;
    GLint indexShift;
    GLint indexOffset;
    GLboolean mapColor;
    GLboolean mapStencil;
    __GLcolor postColorMatrixScale;
    __GLcolor postColorMatrixBias;
} __GLpixelTransferMode;

typedef struct __GLpixelStateRec {
    __GLpixelTransferMode transferMode;
    __GLpixelMapHead pixelMap[__GL_N_PIXEL_MAPS];
    GLuint pixelMapTableId;

    /*
    ** Read buffer.  Where pixel reads come from.
    */
    GLenum readBuffer;

    /*
    ** Read buffer specified by user.  May be different from readBuffer
    ** above.  If the user specifies GL_FRONT_LEFT, for example, then
    ** readBuffer is set to GL_FRONT, and readBufferReturn to
    ** GL_FRONT_LEFT.
    */
    GLenum readBufferReturn;
} __GLpixelState;

typedef struct __GLpixelSpanModInfo {
                                        /* in values begin here */
    GLint srcType;                  /* FB, MEM, etc. */
    GLint dstType;                  /* FB, MEM, etc. */
    GLint pixelPath;                /* type of pixel path */

                                        /*internal values begin here */
    GLboolean srcSwap, dstSwap;
    GLboolean srcAlign, dstAlign;
    GLboolean srcConvert, dstConvert;
    GLboolean srcExpand, dstReduce;
    GLboolean srcClamp, dstClamp;
    GLboolean dstResetComponents; /*reset unrequested dst components to default value*/
    GLboolean srcResetComponents;

    GLboolean zoomx1;           /* -1 <= zoomx <= 1? */
    GLboolean zoomx2;           /* zoomx <= -1 || zoomx >= 1 */

    GLboolean skipGroups;        /* decimating zoom */
    GLboolean modify;        /* core pixel transfer ops */
    GLboolean modify2;        /* ARB_imaging pixel transfer ops */
} __GLpixelSpanModInfo;

typedef enum __GLdataTransDirection {
    __GL_APP_TO_DRIVER = 0,
    __GL_DRIVER_TO_APP
} __GLdataTransDirection;

struct __GLpixelSpanInfoRec {
/*In values begin*/
    /*
    ** Generic source info.
    */
    GLenum srcBaseFormat, srcFormat, srcType;  /* Form of source image */
    GLint width, height, depth; /* Size of image: size of final */

    /*Other src info:  necessary only when user mem as src. */
    const GLvoid *srcImage;     /* The source image */

    /* Other src info:  necessary only when FB as src.  */
    GLfloat readX, readY;     /* Reading coords (CopyPixels, ReadPixels) */
    GLint readYIncr;            /* Increase factor of readY after read one line from frambuffer.
                                                -1, or 1.*/
    /*"Remember to init readYIncr in any pixel path need read from framebuffer."*/

    /* Pixel unpacking state fields, map one by one to gc state.
    ** If unpack pertain, init these by __glLoadUnpackModes.
    ** If no unpack needed, these must set to default value.
    */
    GLint srcSwapBytes;
    GLint srcLsbFirst;
    GLint srcSkipPixels, srcSkipLines, srcSkipImages;
    GLint srcLineLength, srcImageHeight;
    GLint srcAlignment;

    /*
    ** fields to descript dest.
    */
    GLenum dstFormat, dstType;  /* Form of destination image */
    const GLvoid *dstImage;     /* The destination image */

    GLenum reqFormat;   /* required internal format */
    GLenum reqBaseFormat; /* base format of internal format */

    /*necessary only when write to FB*/
    GLfloat x, y;             /* Effective raster coordinates. */

    /*PixelZoom fields. if ynvert, zoomy = (-1) gc...zoomy*/
    GLfloat zoomx, zoomy;

    /* Pixel packing state fields, map one by one to gc state.
    ** If pack pertain, init these by __glLoadPackModes.
    ** If no pack needed, these must set to default value.
    */
    GLint dstSwapBytes;
    GLint dstLsbFirst;
    GLint dstSkipPixels, dstSkipLines, dstSkipImages;
    GLint dstLineLength, dstImageHeight;
    GLint dstAlignment;

    GLint dim;                      /* dimensionality of convolution routine prefered.
                                                0: no convolution needed;
                                                1: 1D convo (TexImage1D, TexSubImage1D, Copy-
                                                        TexImage1D, and CopyTexSubImage1D.)
                                                 2: 2D convo (Draw/Read/CopyPixels, TexImage23D...)
                                                 3,4: set by TexImage3D, cubemap texture. also mean 2D convolution filter.
                                                   */
    /*For loading of  colortable, convolution filter only.*/
    __GLcolor scale;        /* NOT implemented. non pix xfer scale*/
    __GLcolor bias;        /* NOT implemented. non pix xfer bias*/

    /*For Accum operations*/
    __GLfloat accumValue;

    /*
    ** These fields are used only for selecting spanModifiers.  The
    ** span modifier routines themselves do not use them.  They are
    ** in this record because they need to be passed through copyImage.
    */
    GLboolean applySrcClamp;        /* clamp source data */
    GLboolean applyDstClamp;        /* clamp destination data */
    GLboolean zeroFillAlpha;        /* fill alpha with zero value */
    GLboolean nonColorComp;         /* not color components (eg hgram) */
    GLboolean applyPixelTransfer;    /* apply pixel transfer operations */
    GLboolean applyGenericScaleBias;    /* apply non pix xfer scale and bias.
                                                                 For loading of  colortable, convolution filter only.*/
/*In values end*/
    __GLdataTransDirection transDirection;

    /*
    ** Runtime states of source: reading from system mem.
    */
    GLvoid *srcCurrent;         /* The current pointer of source data */
    /*indricet info, calculted from source discription and unpack state.*/
    GLint srcRowIncrement;      /* Add this much to get to the next row */
    GLint srcGroupIncrement;    /* Add this much to get to the next group */
    GLint srcImageIncrement;    /* Add this much to get to the next image */
    GLint srcComponents;        /* (4 for RGBA, 1 for ALPHA, etc.) */
    GLint srcElementSize;       /* Size of one element (1 for BYTE) */

    GLboolean srcPackedData;    /* True if source data is packed */
    GLint srcStartBit;          /* Startbit of bitmap after applying unpack skipPixels */

    /*
    ** Runtime states of dest: writting to system mem.
    */
    GLvoid *dstCurrent;         /* The current pointer into the dest data */
    GLint dstRowIncrement;      /* Add this much to get to the next row */
    GLint dstGroupIncrement;    /* Add this much to get to the next group */
    GLint dstImageIncrement;    /* Add this much to get to the next image */
    GLint dstComponents;        /* (4 for RGBA, 1 for ALPHA, etc.) */
    GLint dstElementSize;       /* Size of one element (1 for BYTE) */

    GLboolean dstPackedData;    /* True if destination data is packed */
    GLint dstStartBit;          /* Startbit of bitmap after applying pack skipPixels */

    /*
    ** For compressing DXT*, linesPerSpan == 4; else == 1;
    ** APP call glTexImagXD/glCopyTexImageXD to load uncompressed texture into compressed format.
    ** Every 4*4 groups compress to a DXT blcok.
    ** 1. To increase current source address by add srcRowIncrement*srcLinesPerSpan after source read span;
    ** 2. every span processor need to process all groups in actually  srcLinesPerSpan lines.
    */
    GLuint srcLinesPerSpan;
//    GLuint spanRowIncrement; /* Add this much to get to the next row of this span block */

    /*
    ** For de-compressing DXT*, linesPerSpan == 4; else == 1;
    */
    GLuint dstLinesPerSpan;

    /* For compressing or de-compressing DXT* format */
    GLuint spanWidth;               /*in pixels  */




    /*
    ** Runtime states of dst: check by framebuffer render
    */
    GLint startCol, startRow;   /* For render: First actual pixel goes here */
    GLint endCol;               /* For render: Last column rendered (minus coladd) */
    GLint columns, rows;        /* Taking zoomx, zoomy into account */
    GLboolean overlap;          /* Do CopyPixels src/dest regions overlap? */
    GLint rowsUp, rowsDown;     /* Stuff for overlapping CopyPixels regions */
    GLint rowadd, coladd;       /* For render: Adders for incrementing the col or row.
                                                1, or -1. pick from zoomx, zoomy*/
    GLfloat rendZoomx;        /* effective zoomx for render span */
    __GLzValue fragz;           /* save this computation in the span walker */
    GLfloat rpyUp, rpyDown;
    GLint startUp, startDown;
    GLint readUp, readDown;

    /*
    ** A pile of span routines used by the DrawPixels, ReadPixels, and
    ** CopyPixels functions.
    */
    GLint numSpanMods;          /* number of span modifiers current in use */
    GLvoid (*spanReader)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLvoid *outspan);
    GLvoid (*(spanModifier[_GL_NUMBER_OF_PIXEL_MODES]))(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                              GLvoid *inspan, GLvoid *outspan);
    GLvoid (*spanRender)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLvoid *inspan);

    /*
    ** used by GetTexImage
    */

    GLvoid *reserved;
    /*
    ** These fields are used by convolution operators
    */
    GLint spanCount;        /* running counter */
    GLint convInitialSpans;
    GLint convFinalSpans;
    GLint convModifierIndex;    /* index of the convolution span Modifier */

    struct {
        GLubyte *pixels; /* pixel data read from the frame buffer */
        GLint pitch; /* pitch of the internal pixel data*/
        GLint alignment;
        GLenum format;
        GLenum type;
    } internalReadPixels;

    struct {
        GLubyte *pixels; /* pixel data to be drawn to the frame buffer */
        GLenum format;
        GLenum type;
    } internalDrawPixels;

    /*
    ** Temp space for per span transform.
    */
    /*
    ** ATTATION!: It's not recommended to use __GLpixelSpanInfoRec to declare a member variable directly.
    **                     Allocating it in heap is preferred, otherwise it will take lots of stack.
    */
    GLubyte spanData1[__GL_MAX_SPAN_SIZE];
    GLubyte spanData2[__GL_MAX_SPAN_SIZE];
    GLubyte spanData3[__GL_MAX_SPAN_SIZE];
    GLshort pixelArray[__GL_MAX_MAX_VIEWPORT];/* Array of pixel replication counts (if    */
                                                                    /* zoomx < -1 or zoomx > 1) or pixels to   */
                                                                    /* skip (if zoomx < 1 and zoomx > -1).     */
};

typedef struct __GLpixelMachineRec {

    GLuint pixelModeFlags;    /* Operations affecting pixel rectangles */

    GLuint modifyRGBA;       /* Is the RGBA path being modified? */
    GLuint modifyDepth;
    GLuint modifyStencil;

    /* scaled values indicating what a red of 0 maps to, an alpha of 1 ... */
    GLfloat red0Mod, green0Mod, blue0Mod, alpha1Mod;
    GLvoid *iCurMap, *redCurMap, *greenCurMap, *blueCurMap, *alphaCurMap;
    GLboolean rgbaCurrent;
    GLfloat *redMap;            /* Lookup tables with no modification, init in glmakecurrent. */
    GLfloat *greenMap;
    GLfloat *blueMap;
    GLfloat *alphaMap;
    GLfloat *iMap;
    GLfloat *redModMap;         /* Lookup tables for modification path. */
    GLfloat *greenModMap;
    GLfloat *blueModMap;
    GLfloat *alphaModMap;
    GLboolean iToICurrent;      /* Lookup table for modification of CI */
    GLfloat *iToIMap;
    GLboolean iToRGBACurrent;   /* Lookup tables from CI to RGBA */
    GLfloat *iToRMap;
    GLfloat *iToGMap;
    GLfloat *iToBMap;
    GLfloat *iToAMap;

    __GLpixelSpanInfo *spanInfo;
} __GLpixelMachine;

#define __GL_SPANDATA_OFFSET offsetof(__GLpixelSpanInfo,spanData1)
#define __RAW_INIT_SPANINFO()                         \
{                                                     \
    __GL_MEMZERO(spanInfo, __GL_SPANDATA_OFFSET );    \
    spanInfo->zoomx = spanInfo->zoomy = __glOne;      \
    spanInfo->applySrcClamp = GL_TRUE;                \
    spanInfo->applyDstClamp = GL_TRUE;                \
    spanInfo->applyPixelTransfer = GL_TRUE;           \
    spanInfo->srcLinesPerSpan = 1;                    \
}

typedef struct __GLpixelTransferInfoRec{
    /*
    ** Public info.
    */
    GLuint width, height, depth;
    GLuint numOfPixel;               /* Total number of pixels */
    GLuint numOfComponents;          /* Total number of components */
    GLuint sizeOfAlignMemory;           /* size of alignment of memory */
    GLuint widthAlign, dstWidthAlign;   /* alignment width */
    GLuint sizeOfElement;           /* Element size */
    GLuint compNumOfElement;        /* Element number */
    GLuint numOfAlign;              /* number of alignment */
    GLuint numOfAlignSrc;       /* number of alignment about source memory */
    GLuint alignment;               /* alignment size */

    GLenum baseFormat;
    GLubyte compNumber;             /* Get component number from base format */
    GLubyte compMask[4];            /* R G B A at postion 0 1 2 3, values 1 2 3 4 represent components sequence */

    __GLcolor scale;                /* NOT implemented. non pix xfer scale*/
    __GLcolor bias;                 /* NOT implemented. non pix xfer bias*/

    GLboolean applyPixelTransfer;   /* apply pixel transfer operations */
    GLboolean applyGenericScaleBias;/* apply non pix xfer scale and bias.*/

    /*
    ** Generic source info.
    */
    GLenum srcType;                 /* Form of source image */
    GLboolean srcPackedComp;        /* Speicial type, such as ushort565 */
    GLuint srcSizeOfPixel;

    const GLvoid *srcImage;         /* The source image */
    GLboolean srcNeedFree;

    /*
    ** Generic destination info.
    */
    GLenum dstType;                 /* Form of destination image */
    GLboolean dstPackedComp;        /* Speicial type, such as ushort565 */
    GLuint dstSizeOfPixel;

    const GLvoid *dstImage;         /* The destination image */
    GLboolean dstNeedFree;
}__GLpixelTransferInfo;

typedef enum __GLPixelTransferOperations {
    __GL_TexImage = 0,
    __GL_ReadPixels,
    __GL_ReadPixelsPre,
    __GL_CopyPixels
} __GLPixelTransferOperations;

typedef enum __GLParameterTypeJudge{
    __GL_InputFormat=0,
    __GL_OutputFormat,
    __GL_RemainFormat
} ____GLParameterTypeJudge;

extern GLboolean __glNeedScaleBias(__GLcontext *gc, __GLcolor *scale, __GLcolor *bias);
extern GLvoid __glLoadUnpackModes(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
extern GLvoid __glLoadPackModes(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
extern GLvoid __glGenericPixelTransfer(__GLcontext *gc, GLsizei width, GLsizei height,  GLsizei depth, __GLformatInfo *formatInfo, GLenum format, GLenum *type, const GLvoid *buf, __GLpixelTransferInfo *transferInfo, GLenum pixelTransferOperations);
extern __GLformatInfo* __glGetFormatInfo(GLenum internalFormat);
extern GLuint __glPixelSize(__GLcontext *gc, GLenum format, GLenum type);
extern GLvoid __glGetSizeAndNumOfElement(GLenum format, GLenum type, __GLpixelTransferInfo *transferInfo);
extern GLvoid __glMemoryAlignment(GLenum baseFmt, GLenum srcType, GLenum dstType, __GLpixelTransferInfo *transferInfo, GLenum __GLPixelTransferOperations);
extern GLvoid __glGetSizeOfBuffer(__GLtextureObject *tex, GLint face, GLint level , __GLmipMapLevel *mipmap, GLuint *width, GLuint *height, GLuint *depth);
extern GLboolean __glCheckSpecialFormat(GLenum internalFormat, GLenum format, GLenum* type);

/*data types which aren't defined by spec. but supported by Vivante internal*/
#define __GL_UNSIGNED_BYTE_4_4_REV_VIVPRIV                   0x1FFFF
#define __GL_UNSIGNED_INT_24                                 0x2FFFF
#define __GL_UNSIGNED_S8_D24                                 0x3FFFF //stencil 8 depth 24
#define __GL_UNSIGNED_INT_HIGH24                             0x4FFFF

/*external data formats which aren't defined by spec. but supported by Vivante internal*/
#define GL_RED_GREEN_VIVPRIV                                  0x7FFFF
#define GL_BGRA8_VIVPRIV                                      0xAFFFF

/* Texture Compression BLOCK Dimemesion*/
#define __GL_TCBLOCK_DIM    4


#endif /* __gc_gl_pixel_h_ */
#endif