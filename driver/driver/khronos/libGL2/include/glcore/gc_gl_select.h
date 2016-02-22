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


#ifndef __gc_gl_select_h_
#define __gc_gl_select_h_

typedef struct __GLselectMachineRec {
    /*
    ** This is true when the last primitive to execute hit (intersected)
    ** the selection box.  Whenever the name stack is manipulated this
    ** bit is cleared.
    */
    GLboolean hitFlag;

    /*
    ** Name stack.
    */
    GLuint *stack;
    GLuint *sp;

    /*
    ** The user specified result array overflows, this bit is set.
    */
    GLboolean overFlowed;

    /** Number of hits */
    GLint numHit;

    /** Min and Max Z in a hit record */
    GLfloat hitMinZ;
    GLfloat hitMaxZ;

    /** Number of value have been written, cannot exceed nBufferCount */
    GLuint bufferWrittenCount;

    /** The number of GLint's that the array can hold. */
    GLuint bufferMaxCount;

    /*
    ** User specified result array.  As primitives are processed names
    ** will be entered into this array.
    */
    GLuint *buffer;
} __GLselectMachine;

#endif /* __gc_gl_select_h_ */
