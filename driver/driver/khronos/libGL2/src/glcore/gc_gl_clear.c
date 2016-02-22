/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_gl_context.h"
#include "gl/gl_device.h"
#include "dri/viv_lock.h"
#include "gc_gl_debug.h"


GLvoid APIENTRY __glim_Clear(GLbitfield mask)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Clear", DT_GLbitfield, mask, DT_GLnull);
#endif

    if (gc->renderMode != GL_RENDER) {
        return;
    }

    if (mask & ~(GL_COLOR_BUFFER_BIT | GL_ACCUM_BUFFER_BIT |
                 GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if (!gc->modes.haveAccumBuffer) {
        mask &= ~GL_ACCUM_BUFFER_BIT;
    }

    if (gc->drawablePrivate->width * gc->drawablePrivate->height == 0) {
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Get the latest drawable information */
    LINUX_LOCK_FRAMEBUFFER(gc);

    /* Check all GL attributes for redundancy before notify device pipeline.
    ** Must be call here because __GL_CLEAR_ATTR_BITS affect dp.clear.
    */
    __glEvaluateAttribDrawableChange(gc);

    /* Notify dp to clear
    */
    if (!(gc->flags &__GL_DISCARD_FOLLOWING_DRAWS_FRAMEBUFFER_NOT_COMPLETE))
    {
        (*gc->dp.clear)(gc, mask);
    }
    LINUX_UNLOCK_FRAMEBUFFER(gc);
}

GLvoid APIENTRY __glim_AddSwapHintRectWIN(GLint x, GLint y, GLsizei width,
                                        GLsizei height)
{
    LPRGNDATA rgnData;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_AddSwapHintRectWIN", DT_GLint, x, DT_GLint, y, DT_GLsizei, width, DT_GLsizei, height, DT_GLnull);
#endif

    __GL_VERTEX_BUFFER_FLUSH(gc);

    if (gc->modes.doubleBufferMode) {
        rgnData = (LPRGNDATA)(*gc->drawablePrivate->addSwapHintRectWIN)
            (gc->drawablePrivate, x, y, width, height);

        (*gc->drawablePrivate->dp.addSwapHintRectWIN)(gc->drawablePrivate, &rgnData->rdh.rcBound, rgnData->rdh.nCount);
    }
}
