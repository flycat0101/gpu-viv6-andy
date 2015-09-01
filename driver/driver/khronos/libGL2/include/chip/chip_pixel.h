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


#ifndef _chip_pixel_h__
#define _chip_pixel_h__

typedef enum _gccACCUM_MODE
{
    gccACCUM_UNKNOWN = 0,
    gccACCUM_ACCUM,
    gccACCUM_LOAD,
    gccACCUM_RETURN,
    gccACCUM_MULT,
    gccACCUM_ADD
} gccACCUM_MODE;

typedef enum _gccPIXEL_TRANSFER_MODE
{
    gccPIXEL_TRANSFER_SCALE = 0,
    gccPIXEL_TRANSFER_BIAS
} gccPIXEL_TRANSFER_MODE;
/************************************************************************/
/*  Declaration for DP interface                                        */
/************************************************************************/
extern void __glChipRasterBegin(__GLcontext *gc, GLenum rasterOp,GLenum format, GLint width, GLint height);
extern void __glChipRasterEnd(__GLcontext *gc, GLenum rasterOp);
extern GLboolean __glChipDrawPixels(__GLcontext *gc, GLsizei width, GLsizei height, GLenum format, GLenum type, GLubyte *pixels);
extern GLboolean __glChipReadPixels(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLubyte *buf);
extern GLboolean __glChipCopyPixels(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format);
extern GLboolean __glChipBitmaps(__GLcontext *gc, GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap, __GLbufferObject* bufObj);
extern GLvoid __glChipAccum(__GLcontext* gc, GLenum op, GLfloat value);

#endif /* _chip_pixel_h__ */

