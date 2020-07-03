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


#ifndef _gl_image_h_
#define _gl_image_h_

#ifdef __cplusplus
extern "C" {

GLint __glImageSize(GLsizei width, GLsizei height, GLenum format,
               GLenum type);

GLint __glImageSize3D(GLsizei width, GLsizei height, GLsizei depth,
                 GLenum format, GLenum type);

GLvoid __glFillImage(__GLcontext *gc, GLsizei width, GLsizei height,
              GLenum format, GLenum type, const GLvoid *userdata,
              GLubyte *newimage);

GLvoid __glFillImage3D(__GLcontext *gc, GLsizei width, GLsizei height,
                GLsizei depth, GLenum format, GLenum type,
                const GLvoid *userdata, GLubyte *newimage);

GLvoid __glEmptyImage(__GLcontext *gc, GLsizei width, GLsizei height,
               GLenum format, GLenum type, const GLubyte *oldimage,
               GLvoid *userdata);

GLubyte __glMsbToLsbTable[256];

}
#else

extern GLint __glImageSize(GLsizei width, GLsizei height, GLenum format,
               GLenum type);

extern GLint __glImageSize3D(GLsizei width, GLsizei height, GLsizei depth,
                 GLenum format, GLenum type);

extern GLvoid __glFillImage(__GLcontext *gc, GLsizei width, GLsizei height,
              GLenum format, GLenum type, const GLvoid *userdata,
              GLubyte *newimage);

extern GLvoid __glFillImage3D(__GLcontext *gc, GLsizei width, GLsizei height,
                GLsizei depth, GLenum format, GLenum type,
                const GLvoid *userdata, GLubyte *newimage);

extern GLvoid __glEmptyImage(__GLcontext *gc, GLsizei width, GLsizei height,
               GLenum format, GLenum type, const GLubyte *oldimage,
               GLvoid *userdata);

extern GLubyte __glMsbToLsbTable[256];

#endif

#endif /* __gc_gl_image_h_ */
