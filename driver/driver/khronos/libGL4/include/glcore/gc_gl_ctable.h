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


#ifdef OPENGL40
#ifndef __gc_gl_ctable_h_
#define __gc_gl_ctable_h_

/*
** Number of color table targets
*/
#define __GL_NUM_COLOR_TABLE_TARGETS                    3
#define __GL_COLOR_TABLE_INDEX                          0
#define __GL_POST_CONVOLUTION_COLOR_TABLE_INDEX         1
#define __GL_POST_COLOR_MATRIX_COLOR_TABLE_INDEX        2

/*
** Stackable per color table state
*/
typedef struct __GLcolorTableStateRec {
    __GLcolor scale;
    __GLcolor bias;
} __GLcolorTableState;

/*
** Color table state
*/
typedef struct __GLcolorTableRec {
    GLenum target;
    GLenum formatReturn;          /* requested internal format */

    GLvoid *table;
    GLsizei width;                /* counts of table component*/

    GLenum type;
    GLenum format;                /* actual internal format */
    GLenum baseFormat;            /* internal format w/o size info */

    GLsizei redSize;
    GLsizei greenSize;
    GLsizei blueSize;
    GLsizei alphaSize;
    GLsizei luminanceSize;
    GLsizei intensitySize;

    __GLcolorTableState state;    /* stackable state */
} __GLcolorTable;

#endif /* __gc_gl_ctable_h_ */

#endif