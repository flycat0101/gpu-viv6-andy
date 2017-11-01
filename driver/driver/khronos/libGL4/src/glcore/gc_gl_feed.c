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


/*
** Feedback code.
**
*/
#include "gc_es_context.h"

GLvoid __glFeedbackTag(__GLcontext *gc, GLfloat f)
{
    if (!gc->feedback.overFlowed) {
        if (gc->feedback.result >=
                gc->feedback.resultBase + gc->feedback.resultLength) {
            gc->feedback.overFlowed = GL_TRUE;
        } else {
            gc->feedback.result[0] = f;
            gc->feedback.result = gc->feedback.result + 1;
        }
    }
}

GLvoid APIENTRY __glim_FeedbackBuffer(__GLcontext *gc, GLsizei bufferLength, GLenum type, GLfloat *buffer)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    if ((type < GL_2D) || (type > GL_4D_COLOR_TEXTURE)) {
        __glSetError(gc, GL_INVALID_ENUM);
        return;
    }
    if (bufferLength < 0) {
        __glSetError(gc, GL_INVALID_VALUE);
        return;
    }
    if (gc->renderMode == GL_FEEDBACK) {
        __glSetError(gc, GL_INVALID_OPERATION);
        return;
    }

    /* flush the primitive buffer
    */
    __GL_VERTEX_BUFFER_FLUSH(gc);

    gc->feedback.resultBase = buffer;
    gc->feedback.result = buffer;
    gc->feedback.resultLength = bufferLength;
    gc->feedback.overFlowed = GL_FALSE;
    gc->feedback.type = type;
}

GLvoid APIENTRY __glim_PassThrough(__GLcontext *gc, GLfloat element)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    /* flush the primitive buffer
    */
    __GL_VERTEX_BUFFER_FLUSH(gc);

    if (gc->renderMode == GL_FEEDBACK) {
        __glFeedbackTag(gc, GL_PASS_THROUGH_TOKEN);
        __glFeedbackTag(gc, element);
    }
}

GLvoid __glInitFeedback(__GLcontext *gc)
{
    gc->feedback.overFlowed = GL_FALSE;
    gc->feedback.resultBase = 0;
    gc->feedback.resultLength = 0;
    gc->feedback.result = 0;
}
