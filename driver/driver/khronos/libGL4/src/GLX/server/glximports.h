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


/* $XFree86: xc/programs/Xserver/GL/glx/glximports.h,v 1.3 2001/03/21 16:29:36 dawes Exp $ */
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#ifndef _glximports_h_
#define _glximports_h_

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

extern GLvoid *__glXImpMalloc(__GLcontext *gc, size_t size);
extern GLvoid *__glXImpCalloc(__GLcontext *gc, size_t nElem, size_t eSize);
extern GLvoid *__glXImpRealloc(__GLcontext *gc, GLvoid *addr, size_t newSize);
extern GLvoid  __glXImpFree(__GLcontext *gc, GLvoid *addr);

extern GLvoid  __glXImpWarning(__GLcontext *gc, char *msg);
extern GLvoid  __glXImpFatal(__GLcontext *gc, char *msg);

extern char *__glXImpGetenv(__GLcontext *gc, const char *var);
extern int   __glXImpAtoi(__GLcontext *gc, const char *str);
extern int   __glXImpSprintf(__GLcontext *gc, char *str, const char *fmt, ...);
extern GLvoid *__glXImpFopen(__GLcontext *gc, const char *path,
               const char *mode);
extern int   __glXImpFclose(__GLcontext *gc, GLvoid *stream);
extern int   __glXImpFprintf(__GLcontext *gc, GLvoid *stream,
                 const char *fmt, ...);

extern __GLdrawablePrivate *__glXImpGetDrawablePrivate(__GLcontext *gc);
extern __GLdrawablePrivate *__glXImpGetReadablePrivate(__GLcontext *gc);


#endif /* _glximports_h_ */

