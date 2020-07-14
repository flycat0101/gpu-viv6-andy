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


/*
** Select code.
**
*/
#include "gc_es_context.h"

GLvoid __glUpdateHitFlag(__GLcontext *gc, __GLfloat z)
{
    gc->select.hitFlag = GL_TRUE;
    gc->select.hitMinZ = __GL_MIN(z, gc->select.hitMinZ);
    gc->select.hitMaxZ = __GL_MAX(z, gc->select.hitMaxZ);
}

GLvoid __glSelectPoint(__GLcontext *gc, __GLvertex *v)
{
    __glUpdateHitFlag(gc, v->winPos.f.z);
}

GLvoid __glWriteHitRecord(__GLcontext *gc)
{
    GLuint nZMin, nZMax, i, nDepth;

    /**
     * Scale to 32 bit unsigned integer
     * Here we cast 0xFFFFFFFF to double to prevent FPU overflow to truncate value to 0x800000000
     */
    nZMin = (GLuint)(gc->select.hitMinZ * (GLdouble)0xFFFFFFFF + __glHalf);
    nZMax = (GLuint)(gc->select.hitMaxZ * (GLdouble)0xFFFFFFFF + __glHalf);
    nDepth = (GLuint)(gc->select.sp - gc->select.stack);

    /** Write # of name when hit */
    if (gc->select.bufferWrittenCount < gc->select.bufferMaxCount)
    {
        gc->select.buffer[gc->select.bufferWrittenCount] = nDepth;
    }
    else
    {
        gc->select.overFlowed = GL_TRUE;
        goto exit;
    }
    gc->select.bufferWrittenCount++;

    /** Write zMin when hit */
    if (gc->select.bufferWrittenCount < gc->select.bufferMaxCount)
    {
        gc->select.buffer[gc->select.bufferWrittenCount] = nZMin;
    }
    else
    {
        gc->select.overFlowed = GL_TRUE;
        goto exit;
    }
    gc->select.bufferWrittenCount++;

    /** Write zMin when hit */
    if (gc->select.bufferWrittenCount < gc->select.bufferMaxCount)
    {
        gc->select.buffer[gc->select.bufferWrittenCount] = nZMax;
    }
    else
    {
        gc->select.overFlowed = GL_TRUE;
        goto exit;
    }
    gc->select.bufferWrittenCount++;

    for (i = 0; i < nDepth; i++)
    {
        /** Write each name in the stack from stack base */
        if (gc->select.bufferWrittenCount < gc->select.bufferMaxCount)
        {
            gc->select.buffer[gc->select.bufferWrittenCount] = gc->select.stack[i];
        }
        else
        {
            gc->select.overFlowed = GL_TRUE;
            break;
        }

        gc->select.bufferWrittenCount++;
    }

exit:
    gc->select.numHit++;
    gc->select.hitFlag = GL_FALSE;
    gc->select.hitMaxZ = 0.0f;
    gc->select.hitMinZ = 1.0f;
}


GLvoid APIENTRY __glim_SelectBuffer(__GLcontext *gc, GLsizei bufferLength, GLuint *buffer)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    if (bufferLength < 0) {
        __glSetError(gc, GL_INVALID_VALUE);
        return;
    }
    if (gc->renderMode == GL_SELECT) {
        __glSetError(gc, GL_INVALID_OPERATION);
        return;
    }

    /* flush the primitive buffer
    */
    __GL_VERTEX_BUFFER_FLUSH(gc);

    gc->select.overFlowed = GL_FALSE;
    gc->select.buffer = buffer;
    gc->select.bufferMaxCount = bufferLength;
    gc->select.hitFlag = GL_FALSE;
    gc->select.hitMaxZ = 0.0f;
    gc->select.hitMinZ = 1.0f;
}

GLvoid APIENTRY __glim_InitNames(__GLcontext *gc)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    /* flush the primitive buffer
    */
    __GL_VERTEX_BUFFER_FLUSH(gc);

    if (gc->renderMode != GL_SELECT)
    {
        return;
    }

    if (gc->select.hitFlag)
    {
        __glWriteHitRecord(gc);
    }

    gc->select.sp = gc->select.stack;
    gc->select.hitFlag = GL_FALSE;
    gc->select.hitMaxZ = 0.0f;
    gc->select.hitMinZ = 1.0f;
}

GLvoid APIENTRY __glim_LoadName(__GLcontext *gc, GLuint name)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    /* flush the primitive buffer
    */
    __GL_VERTEX_BUFFER_FLUSH(gc);

    if (gc->renderMode != GL_SELECT)
    {
        return;
    }

    if (gc->select.sp == gc->select.stack) {
        __glSetError(gc, GL_INVALID_OPERATION);
        return;
    }

    if (gc->select.hitFlag)
    {
        __glWriteHitRecord(gc);
    }

    gc->select.sp[ -1 ] = name;
}

GLvoid APIENTRY __glim_PopName(__GLcontext *gc)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    /* flush the primitive buffer
    */
    __GL_VERTEX_BUFFER_FLUSH(gc);

    if (gc->renderMode != GL_SELECT)
    {
        return;
    }

    if (gc->select.sp == gc->select.stack) {
        __glSetError(gc, GL_STACK_UNDERFLOW);
        return;
    }

    if (gc->select.hitFlag)
    {
        __glWriteHitRecord(gc);
    }

    gc->select.sp = gc->select.sp - 1;
}

GLvoid APIENTRY __glim_PushName(__GLcontext *gc, GLuint name)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    /* flush the primitive buffer
    */
    __GL_VERTEX_BUFFER_FLUSH(gc);

    if (gc->renderMode != GL_SELECT)
    {
        return;
    }

    if (gc->select.sp >= &gc->select.stack[gc->constants.maxNameStackDepth]) {
        gc->select.overFlowed = GL_TRUE;
        __glSetError(gc, GL_STACK_OVERFLOW);
        return;
    }

    if (gc->select.hitFlag)
    {
        __glWriteHitRecord(gc);
    }

    gc->select.sp[0] = name;
    gc->select.sp = gc->select.sp + 1;
}

GLvoid __glInitSelect(__GLcontext *gc)
{
    /* Allocate Name stack depth based on maxNameStackDepth */
    gc->select.stack = (GLuint*)(*gc->imports.malloc)
        (gc, gc->constants.maxNameStackDepth * sizeof(GLuint) );

    gc->select.overFlowed = GL_FALSE;
    gc->select.hitFlag = GL_FALSE;
    gc->select.buffer = 0;
    gc->select.bufferMaxCount = 0;
    gc->select.sp = gc->select.stack;
    gc->select.numHit = 0;
}

GLvoid __glFreeSelectState(__GLcontext *gc)
{
    if (gc->select.stack) {
        (*gc->imports.free)(gc, gc->select.stack);
    }
}
