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


/* $XFree86: xc/programs/Xserver/GL/glx/glxmem.c,v 1.6 2001/10/31 22:50:27 tsi Exp $ */
/*
** License Applicability. Except to the extent portions of this file are
** made subject to an alternative license as permitted in the SGI Free
** Software License B, Version 1.1 (the "License"), the contents of this
** file are subject only to the provisions of the License. You may not use
** this file except in compliance with the License. You may obtain a copy
** of the License at Silicon Graphics, Inc., attn: Legal Services, 1600
** Amphitheatre Parkway, Mountain View, CA 94043-1351, or at:
**
** http://oss.sgi.com/projects/FreeB
**
** Note that, as provided in the License, the Software is distributed on an
** "AS IS" basis, with ALL EXPRESS AND IMPLIED WARRANTIES AND CONDITIONS
** DISCLAIMED, INCLUDING, WITHOUT LIMITATION, ANY IMPLIED WARRANTIES AND
** CONDITIONS OF MERCHANTABILITY, SATISFACTORY QUALITY, FITNESS FOR A
** PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
**
** Original Code. The Original Code is: OpenGL Sample Implementation,
** Version 1.2.1, released January 26, 2000, developed by Silicon Graphics,
** Inc. The Original Code is Copyright (c) 1991-2000 Silicon Graphics, Inc.
** Copyright in any portions created by third parties is as indicated
** elsewhere herein. All Rights Reserved.
**
** Additional Notice Provisions: The application programming interfaces
** established by SGI in conjunction with the Original Code are The
** OpenGL(R) Graphics System: A Specification (Version 1.2.1), released
** April 1, 1999; The OpenGL(R) Graphics System Utility Library (Version
** 1.3), released November 4, 1998; and OpenGL(R) Graphics with the X
** Window System(R) (Version 1.3), released October 19, 1998. This software
** was created using the OpenGL(R) version 1.2.1 Sample Implementation
** published by SGI, but has not been independently verified as being
** compliant with the OpenGL(R) version 1.2.1 Specification.
**
*/

/*
** Implementation of a buffer in main memory
*/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "glxserver.h"
#include "glxmem.h"
#include "GL/glxext.h"
#include "glcore.h"

/* don't want to include glmath.h */
extern GLuint __glFloorLog2(GLuint);

/* ---------------------------------------------------------- */

#define BUF_ALIGN 32    /* x86 cache alignment (used for assembly paths) */
#define BUF_ALIGN_MASK  (BUF_ALIGN-1)

static GLboolean
Resize(__GLdrawableBuffer *buf,
       GLint x, GLint y, GLuint width, GLuint height,
       __GLdrawablePrivate *glPriv, GLuint bufferMask)
{
    GL_ASSERT(0);
    return GL_TRUE;
}

static GLvoid
Lock(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
}

static GLvoid
Unlock(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
}

static GLvoid
Free(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
    GL_ASSERT(0);
}


GLvoid
__glXInitMem(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv, GLint bits)
{
    GL_ASSERT(0);
}
