/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef _gl_image_h_
#define _gl_image_h_

EXTERN_C GLint __glImageSize(GLsizei width, GLsizei height, GLenum format,
               GLenum type);

EXTERN_C GLint __glImageSize3D(GLsizei width, GLsizei height, GLsizei depth,
                 GLenum format, GLenum type);

EXTERN_C GLvoid __glFillImage(__GLcontext *gc, GLsizei width, GLsizei height,
              GLenum format, GLenum type, const GLvoid *userdata,
              GLubyte *newimage);

EXTERN_C GLvoid __glFillImage3D(__GLcontext *gc, GLsizei width, GLsizei height,
                GLsizei depth, GLenum format, GLenum type,
                const GLvoid *userdata, GLubyte *newimage);

EXTERN_C GLvoid __glEmptyImage(__GLcontext *gc, GLsizei width, GLsizei height,
               GLenum format, GLenum type, const GLubyte *oldimage,
               GLvoid *userdata);

EXTERN_C GLubyte __glMsbToLsbTable[256];

#endif /* __gc_gl_image_h_ */
