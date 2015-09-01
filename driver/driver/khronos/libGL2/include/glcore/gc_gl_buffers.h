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


#ifndef __gc_gl_buffers_h_
#define __gc_gl_buffers_h_

#include "gl/gl_core.h"


/************************************************************************/

typedef struct __GLbufferRec __GLbuffer;
/*
** Generic buffer description.  This description is used for software
** and hardware buffers of all kinds.
*/
struct __GLbufferRec {
    /*
    ** Dimensions of the buffer.
    */
    GLint width, height, depth;

    /*
    ** Base of framebuffer.
    ** aligned and unaligned
    */
    GLvoid* base;
    GLvoid* ubase;

    /*
    ** Size of each element in the framebuffer.
    ** elementSize: size in byte
    */
    GLint elementSize;

    /*
    ** If this buffer is part of a larger (say full screen) buffer
    ** then this is the size of that larger buffer.  Otherwise it is
    ** just a copy of width.
    */
    GLint outerWidth;

    /*
    ** This is the number of bytes between rows in the buffer.  This
    ** is different than outerWidth which is the number of elements
    ** between rows.
    */
    GLint byteWidth;

    /*
    ** If this buffer is part of a larger (say full screen) buffer
    ** then these are the location of this buffer in the larger
    ** buffer.
    */
    GLint xOrigin, yOrigin;
} ;


/************************************************************************/
typedef struct __GLcolorBufferRec __GLcolorBuffer;
struct __GLcolorBufferRec {
    __GLbuffer buf;

    GLint redMax;
    GLint greenMax;
    GLint blueMax;

    GLboolean needColorFragmentOps;

    GLubyte *alphaTestFuncTable;
    GLubyte *indexTestFuncTable;

    /*
    ** Color component scale factors.  Given a component value between
    ** zero and one, this scales the component into a zero-N value
    ** which is suitable for usage in the color buffer.  Note that these
    ** values are not necessarily the same as the max values above,
    ** which define precise bit ranges for the buffer.  These values
    ** are never zero, for instance.
    **/
    __GLfloat redScale;
    __GLfloat greenScale;
    __GLfloat blueScale;

    /* Integer versions of above */
    GLint iRedScale;
    GLint iGreenScale;
    GLint iBlueScale;

    /* Used primarily by pixmap code */
    GLint redShift;
    GLint greenShift;
    GLint blueShift;
    GLint alphaShift;

    /*
    ** Alpha is treated a little bit differently.  alphaScale and
    ** iAlphaScale are used to define a range of alpha values that are
    ** generated during various rendering steps.  These values will then
    ** be used as indices into a lookup table to see if the alpha test
    ** passes or not.  Because of this, the number should be fairly large
    ** (e.g., one is not good enough).
    */
    __GLfloat alphaScale;
    GLint iAlphaScale;

    __GLfloat oneOverRedScale;
    __GLfloat oneOverGreenScale;
    __GLfloat oneOverBlueScale;
    __GLfloat oneOverAlphaScale;

    /*
    ** Color mask state for the buffer.  When writemasking is enabled
    ** the source and dest mask will contain depth depedent masking.
    */
    GLuint sourceMask, destMask;
};

/*
** the persistent version
*/
typedef struct __GLdepthBufferPRec __GLdepthBufferP;

struct __GLdepthBufferPRec {
    /* extra things that this buffer needs */
    GLuint writeMask;
    GLuint scale;
    GLuint cnt;
    GLint numFracBits;
    GLuint zDepthMask;
    GLuint zClearInc;
    GLuint fullZClear;
    GLuint invertDepthRange;
};

typedef struct __GLstencilBufferRec __GLstencilBuffer;
/************************************************************************/
struct __GLstencilBufferRec {
    __GLbuffer buf;

    /*
    ** Stencil test lookup table.  The stencil buffer value is masked
    ** against the stencil mask and then used as an index into this
    ** table which contains either GL_TRUE or GL_FALSE for the
    ** index.
    */
    GLboolean *testFuncTable;
    GLuint    writeMask;    // write mask for stencil bits
    GLuint    shift;        // shfit for stencil in depth buffer, not all chip has
                            // stencil at bit24-31, some are in bit0-7

    /*
    ** Stencil op tables.  These tables contain the new stencil buffer
    ** value given the old stencil buffer value as an index.
    */
    __GLstencilCell *failOpTable;
    __GLstencilCell *depthFailOpTable;
    __GLstencilCell *depthPassOpTable;
};


typedef struct __GLaccumBufferRec __GLaccumBuffer;
/************************************************************************/
struct __GLaccumBufferRec {
    __GLbuffer buf;

    /*
    ** Scaling factors to convert from color buffer values to accum
    ** buffer values.
    */
    __GLfloat redScale;
    __GLfloat greenScale;
    __GLfloat blueScale;
    __GLfloat alphaScale;

    __GLfloat oneOverRedScale;
    __GLfloat oneOverGreenScale;
    __GLfloat oneOverBlueScale;
    __GLfloat oneOverAlphaScale;
};

#endif /* __gc_gl_buffers_h_ */
