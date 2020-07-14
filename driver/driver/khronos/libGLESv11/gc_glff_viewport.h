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


#ifndef __gc_glff_viewport_h_
#define __gc_glff_viewport_h_

#ifdef __cplusplus
extern "C" {
#endif

gceSTATUS glfUpdateClpping(
    glsCONTEXT_PTR Context
    );

gceSTATUS glfSetDefaultViewportStates(
    glsCONTEXT_PTR Context
    );

GLenum glfEnableScissorTest(
    glsCONTEXT_PTR Context,
    GLboolean Enable
    );

GLboolean glfQueryViewportState(
    glsCONTEXT_PTR Context,
    GLenum Name,
    GLvoid* Value,
    gleTYPE Type
    );

void glfSetScissor(
    glsCONTEXT_PTR Context,
    GLint X,
    GLint Y,
    GLsizei Width,
    GLsizei Height
    );

void glfSetViewport(
    glsCONTEXT_PTR Context,
    GLint X,
    GLint Y,
    GLsizei Width,
    GLsizei Height
    );

gceSTATUS glfUpdateViewport(
    glsCONTEXT_PTR Context
    );

gceSTATUS glfUpdateCulling(
    glsCONTEXT_PTR Context
    );


#ifdef __cplusplus
}
#endif

#endif /* __gc_glff_viewport_h_ */
