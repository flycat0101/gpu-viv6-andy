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


#ifndef __gc_gl_feedback_h_
#define __gc_gl_feedback_h_

typedef struct __GLfeedbackMachineRec {
    /*
    ** The user specified result array overflows, this bit is set.
    */
    GLboolean overFlowed;

    /*
    ** User specified result array.  As primitives are processed feedback
    ** data will be entered into this array.
    */
    GLfloat *resultBase;

    /*
    ** Current pointer into the result array.
    */
    GLfloat *result;

    /*
    ** The number of GLfloat's that the array can hold.
    */
    GLint resultLength;

    /*
    ** Type of vertices wanted
    */
    GLenum type;

} __GLfeedbackMachine;
#endif /* __gc_gl_feedback_h_ */
