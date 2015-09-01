/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_gl_context.h"


__GL_INLINE GLvoid
__glStore1ValueTo1Float(GLuint index, GLenum type, GLsizei stride, GLboolean normalized, GLvoid *pArray, GLfloat **bufptr)
{
    GLfloat *buf = (GLfloat *)(*bufptr);

    switch (type) {
    case GL_BYTE:
        {
            GLbyte *val = (GLbyte *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_B_TO_FLOAT(*val);
            }
            else {
                *buf++ = (GLfloat)*val;
            }
        }
        break;
    case GL_UNSIGNED_BYTE:
        {
            GLubyte *val = (GLubyte *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_UB_TO_FLOAT(*val);
            }
            else {
                *buf++ = (GLfloat)*val;
            }
        }
        break;
    case GL_SHORT:
        {
            GLshort *val = (GLshort *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_S_TO_FLOAT(*val);
            }
            else {
                *buf++ = (GLfloat)*val;
            }
        }
        break;
    case GL_UNSIGNED_SHORT:
        {
            GLushort *val = (GLushort *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_US_TO_FLOAT(*val);
            }
            else {
                *buf++ = (GLfloat)*val;
            }
        }
        break;
    case GL_INT:
        {
            GLint *val = (GLint *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_I_TO_FLOAT(*val);
            }
            else {
                *buf++ = (GLfloat)*val;
            }
        }
        break;
    case GL_UNSIGNED_INT:
        {
            GLuint *val = (GLuint *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_UI_TO_FLOAT(*val);
            }
            else {
                *buf++ = (GLfloat)*val;
            }
        }
        break;
    case GL_FLOAT:
        break;
    case GL_DOUBLE:
        break;
    default:
        GL_ASSERT(0);
    }

    *bufptr = (GLfloat *)buf;
}

__GL_INLINE GLvoid
__glStore1ValueTo2Floats(GLuint index, GLenum type, GLsizei stride, GLboolean normalized, GLvoid *pArray, GLfloat **bufptr)
{
    GLfloat *buf = (GLfloat *)(*bufptr);

    switch (type) {
    case GL_BYTE:
        {
            GLbyte *val = (GLbyte *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_B_TO_FLOAT(*val);
                *buf++ = 0.0f;
            }
            else {
                *buf++ = (GLfloat)*val;
                *buf++ = 0.0f;
            }
        }
        break;
    case GL_UNSIGNED_BYTE:
        {
            GLubyte *val = (GLubyte *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_UB_TO_FLOAT(*val);
                *buf++ = 0.0f;
            }
            else {
                *buf++ = (GLfloat)*val;
                *buf++ = 0.0f;
            }
        }
        break;
    case GL_SHORT:
        {
            GLshort *val = (GLshort *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_S_TO_FLOAT(*val);
                *buf++ = 0.0f;
            }
            else {
                *buf++ = (GLfloat)*val;
                *buf++ = 0.0f;
            }
        }
        break;
    case GL_UNSIGNED_SHORT:
        {
            GLushort *val = (GLushort *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_US_TO_FLOAT(*val);
                *buf++ = 0.0f;
            }
            else {
                *buf++ = (GLfloat)*val;
                *buf++ = 0.0f;
            }
        }
        break;
    case GL_INT:
        {
            GLint *val = (GLint *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_I_TO_FLOAT(*val);
                *buf++ = 0.0f;
            }
            else {
                *buf++ = (GLfloat)*val;
                *buf++ = 0.0f;
            }
        }
        break;
    case GL_UNSIGNED_INT:
        {
            GLuint *val = (GLuint *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_UI_TO_FLOAT(*val);
                *buf++ = 0.0f;
            }
            else {
                *buf++ = (GLfloat)*val;
                *buf++ = 0.0f;
            }
        }
        break;
    case GL_FLOAT:
        {
            GLfloat *val = (GLfloat *)((GLubyte *)pArray + index * stride);
            *buf++ = *val;
            *buf++ = 0.0f;
        }
        break;
    case GL_DOUBLE:
        {
            GLdouble *val = (GLdouble *)((GLubyte *)pArray + index * stride);
            *buf++ = (GLfloat)*val;
            *buf++ = 0.0f;
        }
        break;
    default:
        GL_ASSERT(0);
    }

    *bufptr = (GLfloat *)buf;
}

__GL_INLINE GLvoid
__glStore2ValuesTo2Floats(GLuint index, GLenum type, GLsizei stride, GLboolean normalized, GLvoid *pArray, GLfloat **bufptr)
{
    GLfloat *buf = (GLfloat *)(*bufptr);

    switch (type) {
    case GL_BYTE:
        {
            GLbyte *val = (GLbyte *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_B_TO_FLOAT(*val++);
                *buf++ = __GL_B_TO_FLOAT(*val);
            }
            else {
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val;
            }
        }
        break;
    case GL_UNSIGNED_BYTE:
        {
            GLubyte *val = (GLubyte *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_UB_TO_FLOAT(*val++);
                *buf++ = __GL_UB_TO_FLOAT(*val);
            }
            else {
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val;
            }
        }
        break;
    case GL_SHORT:
        {
            GLshort *val = (GLshort *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_S_TO_FLOAT(*val++);
                *buf++ = __GL_S_TO_FLOAT(*val);
            }
            else {
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val;
            }
        }
        break;
    case GL_UNSIGNED_SHORT:
        {
            GLushort *val = (GLushort *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_US_TO_FLOAT(*val++);
                *buf++ = __GL_US_TO_FLOAT(*val);
            }
            else {
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val;
            }
        }
        break;
    case GL_INT:
        {
            GLint *val = (GLint *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_I_TO_FLOAT(*val++);
                *buf++ = __GL_I_TO_FLOAT(*val);
            }
            else {
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val;
            }
        }
        break;
    case GL_UNSIGNED_INT:
        {
            GLuint *val = (GLuint *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_UI_TO_FLOAT(*val++);
                *buf++ = __GL_UI_TO_FLOAT(*val);
            }
            else {
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val;
            }
        }
        break;
    case GL_FLOAT:
        {
            GLfloat *val = (GLfloat *)((GLubyte *)pArray + index * stride);
            *buf++ = *val++;
            *buf++ = *val;
        }
        break;
    case GL_DOUBLE:
        {
            GLdouble *val = (GLdouble *)((GLubyte *)pArray + index * stride);
            *buf++ = (GLfloat)*val++;
            *buf++ = (GLfloat)*val;
        }
        break;
    default:
        GL_ASSERT(0);
    }

    *bufptr = (GLfloat *)buf;
}

__GL_INLINE GLvoid
__glStore3ValuesTo3Floats(GLuint index, GLenum type, GLsizei stride, GLboolean normalized, GLvoid *pArray, GLfloat **bufptr)
{
    GLfloat *buf = (GLfloat *)(*bufptr);

    switch (type) {
    case GL_BYTE:
        {
            GLbyte *val = (GLbyte *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_B_TO_FLOAT(*val++);
                *buf++ = __GL_B_TO_FLOAT(*val++);
                *buf++ = __GL_B_TO_FLOAT(*val);
            }
            else {
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val;
            }
        }
        break;
    case GL_UNSIGNED_BYTE:
        {
            GLubyte *val = (GLubyte *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_UB_TO_FLOAT(*val++);
                *buf++ = __GL_UB_TO_FLOAT(*val++);
                *buf++ = __GL_UB_TO_FLOAT(*val);
            }
            else {
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val;
            }
        }
        break;
    case GL_SHORT:
        {
            GLshort *val = (GLshort *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_S_TO_FLOAT(*val++);
                *buf++ = __GL_S_TO_FLOAT(*val++);
                *buf++ = __GL_S_TO_FLOAT(*val);
            }
            else {
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val;
            }
        }
        break;
    case GL_UNSIGNED_SHORT:
        {
            GLushort *val = (GLushort *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_US_TO_FLOAT(*val++);
                *buf++ = __GL_US_TO_FLOAT(*val++);
                *buf++ = __GL_US_TO_FLOAT(*val);
            }
            else {
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val;
            }
        }
        break;
    case GL_INT:
        {
            GLint *val = (GLint *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_I_TO_FLOAT(*val++);
                *buf++ = __GL_I_TO_FLOAT(*val++);
                *buf++ = __GL_I_TO_FLOAT(*val);
            }
            else {
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val;
            }
        }
        break;
    case GL_UNSIGNED_INT:
        {
            GLuint *val = (GLuint *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_UI_TO_FLOAT(*val++);
                *buf++ = __GL_UI_TO_FLOAT(*val++);
                *buf++ = __GL_UI_TO_FLOAT(*val);
            }
            else {
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val;
            }
        }
        break;
    case GL_FLOAT:
        {
            GLfloat *val = (GLfloat *)((GLubyte *)pArray + index * stride);
            *buf++ = *val++;
            *buf++ = *val++;
            *buf++ = *val;
        }
        break;
    case GL_DOUBLE:
        {
            GLdouble *val = (GLdouble *)((GLubyte *)pArray + index * stride);
            *buf++ = (GLfloat)*val++;
            *buf++ = (GLfloat)*val++;
            *buf++ = (GLfloat)*val;
        }
        break;
    default:
        GL_ASSERT(0);
    }

    *bufptr = (GLfloat *)buf;
}

__GL_INLINE GLvoid
__glStore1ValueTo4Floats(GLuint index, GLenum type, GLsizei stride, GLboolean normalized, GLvoid *pArray, GLfloat **bufptr)
{
    GLfloat *buf = (GLfloat *)(*bufptr);

    switch (type) {
    case GL_BYTE:
        {
            GLbyte *val = (GLbyte *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_B_TO_FLOAT(*val);
                *buf++ = 0.0f;
                *buf++ = 0.0f;
                *buf++ = 1.0f;
            }
            else {
                *buf++ = (GLfloat)*val;
                *buf++ = 0.0f;
                *buf++ = 0.0f;
                *buf++ = 1.0f;
            }
        }
        break;
    case GL_UNSIGNED_BYTE:
        {
            GLubyte *val = (GLubyte *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_UB_TO_FLOAT(*val);
                *buf++ = 0.0f;
                *buf++ = 0.0f;
                *buf++ = 1.0f;
            }
            else {
                *buf++ = (GLfloat)*val;
                *buf++ = 0.0f;
                *buf++ = 0.0f;
                *buf++ = 1.0f;
            }
        }
        break;
    case GL_SHORT:
        {
            GLshort *val = (GLshort *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_S_TO_FLOAT(*val);
                *buf++ = 0.0f;
                *buf++ = 0.0f;
                *buf++ = 1.0f;
            }
            else {
                *buf++ = (GLfloat)*val;
                *buf++ = 0.0f;
                *buf++ = 0.0f;
                *buf++ = 1.0f;
            }
        }
        break;
    case GL_UNSIGNED_SHORT:
        {
            GLushort *val = (GLushort *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_US_TO_FLOAT(*val);
                *buf++ = 0.0f;
                *buf++ = 0.0f;
                *buf++ = 1.0f;
            }
            else {
                *buf++ = (GLfloat)*val;
                *buf++ = 0.0f;
                *buf++ = 0.0f;
                *buf++ = 1.0f;
            }
        }
        break;
    case GL_INT:
        {
            GLint *val = (GLint *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_I_TO_FLOAT(*val);
                *buf++ = 0.0f;
                *buf++ = 0.0f;
                *buf++ = 1.0f;
            }
            else {
                *buf++ = (GLfloat)*val;
                *buf++ = 0.0f;
                *buf++ = 0.0f;
                *buf++ = 1.0f;
            }
        }
        break;
    case GL_UNSIGNED_INT:
        {
            GLuint *val = (GLuint *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_UI_TO_FLOAT(*val);
                *buf++ = 0.0f;
                *buf++ = 0.0f;
                *buf++ = 1.0f;
            }
            else {
                *buf++ = (GLfloat)*val;
                *buf++ = 0.0f;
                *buf++ = 0.0f;
                *buf++ = 1.0f;
            }
        }
        break;
    case GL_FLOAT:
        {
            GLfloat *val = (GLfloat *)((GLubyte *)pArray + index * stride);
            *buf++ = *val;
            *buf++ = 0.0f;
            *buf++ = 0.0f;
            *buf++ = 1.0f;
        }
        break;
    case GL_DOUBLE:
        {
            GLdouble *val = (GLdouble *)((GLubyte *)pArray + index * stride);
            *buf++ = (GLfloat)*val;
            *buf++ = 0.0f;
            *buf++ = 0.0f;
            *buf++ = 1.0f;
        }
        break;
    default:
        GL_ASSERT(0);
    }

    *bufptr = (GLfloat *)buf;
}

__GL_INLINE GLvoid
__glStore2ValuesTo4Floats(GLuint index, GLenum type, GLsizei stride, GLboolean normalized, GLvoid *pArray, GLfloat **bufptr)
{
    GLfloat *buf = (GLfloat *)(*bufptr);

    switch (type) {
    case GL_BYTE:
        {
            GLbyte *val = (GLbyte *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_B_TO_FLOAT(*val++);
                *buf++ = __GL_B_TO_FLOAT(*val);
                *buf++ = 0.0f;
                *buf++ = 1.0f;
            }
            else {
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val;
                *buf++ = 0.0f;
                *buf++ = 1.0f;
            }
        }
        break;
    case GL_UNSIGNED_BYTE:
        {
            GLubyte *val = (GLubyte *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_UB_TO_FLOAT(*val++);
                *buf++ = __GL_UB_TO_FLOAT(*val);
                *buf++ = 0.0f;
                *buf++ = 1.0f;
            }
            else {
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val;
                *buf++ = 0.0f;
                *buf++ = 1.0f;
            }
        }
        break;
    case GL_SHORT:
        {
            GLshort *val = (GLshort *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_S_TO_FLOAT(*val++);
                *buf++ = __GL_S_TO_FLOAT(*val);
                *buf++ = 0.0f;
                *buf++ = 1.0f;
            }
            else {
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val;
                *buf++ = 0.0f;
                *buf++ = 1.0f;
            }
        }
        break;
    case GL_UNSIGNED_SHORT:
        {
            GLushort *val = (GLushort *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_US_TO_FLOAT(*val++);
                *buf++ = __GL_US_TO_FLOAT(*val);
                *buf++ = 0.0f;
                *buf++ = 1.0f;
            }
            else {
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val;
                *buf++ = 0.0f;
                *buf++ = 1.0f;
            }
        }
        break;
    case GL_INT:
        {
            GLint *val = (GLint *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_I_TO_FLOAT(*val++);
                *buf++ = __GL_I_TO_FLOAT(*val);
                *buf++ = 0.0f;
                *buf++ = 1.0f;
            }
            else {
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val;
                *buf++ = 0.0f;
                *buf++ = 1.0f;
            }
        }
        break;
    case GL_UNSIGNED_INT:
        {
            GLuint *val = (GLuint *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_UI_TO_FLOAT(*val++);
                *buf++ = __GL_UI_TO_FLOAT(*val);
                *buf++ = 0.0f;
                *buf++ = 1.0f;
            }
            else {
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val;
                *buf++ = 0.0f;
                *buf++ = 1.0f;
            }
        }
        break;
    case GL_FLOAT:
        {
            GLfloat *val = (GLfloat *)((GLubyte *)pArray + index * stride);
            *buf++ = *val++;
            *buf++ = *val;
            *buf++ = 0.0f;
            *buf++ = 1.0f;
        }
        break;
    case GL_DOUBLE:
        {
            GLdouble *val = (GLdouble *)((GLubyte *)pArray + index * stride);
            *buf++ = (GLfloat)*val++;
            *buf++ = (GLfloat)*val;
            *buf++ = 0.0f;
            *buf++ = 1.0f;
        }
        break;
    default:
        GL_ASSERT(0);
    }

    *bufptr = (GLfloat *)buf;
}

__GL_INLINE GLvoid
__glStore3ValuesTo4Floats(GLuint index, GLenum type, GLsizei stride, GLboolean normalized, GLvoid *pArray, GLfloat **bufptr)
{
    GLfloat *buf = (GLfloat *)(*bufptr);

    switch (type) {
    case GL_BYTE:
        {
            GLbyte *val = (GLbyte *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_B_TO_FLOAT(*val++);
                *buf++ = __GL_B_TO_FLOAT(*val++);
                *buf++ = __GL_B_TO_FLOAT(*val);
                *buf++ = 1.0f;
            }
            else {
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val;
                *buf++ = 1.0f;
            }
        }
        break;
    case GL_UNSIGNED_BYTE:
        {
            GLubyte *val = (GLubyte *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_UB_TO_FLOAT(*val++);
                *buf++ = __GL_UB_TO_FLOAT(*val++);
                *buf++ = __GL_UB_TO_FLOAT(*val);
                *buf++ = 1.0f;
            }
            else {
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val;
                *buf++ = 1.0f;
            }
        }
        break;
    case GL_SHORT:
        {
            GLshort *val = (GLshort *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_S_TO_FLOAT(*val++);
                *buf++ = __GL_S_TO_FLOAT(*val++);
                *buf++ = __GL_S_TO_FLOAT(*val);
                *buf++ = 1.0f;
            }
            else {
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val;
                *buf++ = 1.0f;
            }
        }
        break;
    case GL_UNSIGNED_SHORT:
        {
            GLushort *val = (GLushort *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_US_TO_FLOAT(*val++);
                *buf++ = __GL_US_TO_FLOAT(*val++);
                *buf++ = __GL_US_TO_FLOAT(*val);
                *buf++ = 1.0f;
            }
            else {
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val;
                *buf++ = 1.0f;
            }
        }
        break;
    case GL_INT:
        {
            GLint *val = (GLint *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_I_TO_FLOAT(*val++);
                *buf++ = __GL_I_TO_FLOAT(*val++);
                *buf++ = __GL_I_TO_FLOAT(*val);
                *buf++ = 1.0f;
            }
            else {
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val;
                *buf++ = 1.0f;
            }
        }
        break;
    case GL_UNSIGNED_INT:
        {
            GLuint *val = (GLuint *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_UI_TO_FLOAT(*val++);
                *buf++ = __GL_UI_TO_FLOAT(*val++);
                *buf++ = __GL_UI_TO_FLOAT(*val);
                *buf++ = 1.0f;
            }
            else {
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val;
                *buf++ = 1.0f;
            }
        }
        break;
    case GL_FLOAT:
        {
            GLfloat *val = (GLfloat *)((GLubyte *)pArray + index * stride);
            *buf++ = *val++;
            *buf++ = *val++;
            *buf++ = *val;
            *buf++ = 1.0f;
        }
        break;
    case GL_DOUBLE:
        {
            GLdouble *val = (GLdouble *)((GLubyte *)pArray + index * stride);
            *buf++ = (GLfloat)*val++;
            *buf++ = (GLfloat)*val++;
            *buf++ = (GLfloat)*val;
            *buf++ = 1.0f;
        }
        break;
    default:
        GL_ASSERT(0);
    }

    *bufptr = (GLfloat *)buf;
}

__GL_INLINE GLvoid
__glStore4ValuesTo4Floats(GLuint index, GLenum type, GLsizei stride, GLboolean normalized, GLvoid *pArray, GLfloat **bufptr)
{
    GLfloat *buf = (GLfloat *)(*bufptr);

    switch (type) {
    case GL_BYTE:
        {
            GLbyte *val = (GLbyte *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_B_TO_FLOAT(*val++);
                *buf++ = __GL_B_TO_FLOAT(*val++);
                *buf++ = __GL_B_TO_FLOAT(*val++);
                *buf++ = __GL_B_TO_FLOAT(*val);
            }
            else {
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val;
            }
        }
        break;
    case GL_UNSIGNED_BYTE:
        {
            GLubyte *val = (GLubyte *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_UB_TO_FLOAT(*val++);
                *buf++ = __GL_UB_TO_FLOAT(*val++);
                *buf++ = __GL_UB_TO_FLOAT(*val++);
                *buf++ = __GL_UB_TO_FLOAT(*val);
            }
            else {
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val;
            }
        }
        break;
    case GL_SHORT:
        {
            GLshort *val = (GLshort *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_S_TO_FLOAT(*val++);
                *buf++ = __GL_S_TO_FLOAT(*val++);
                *buf++ = __GL_S_TO_FLOAT(*val++);
                *buf++ = __GL_S_TO_FLOAT(*val);
            }
            else {
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val;
            }
        }
        break;
    case GL_UNSIGNED_SHORT:
        {
            GLushort *val = (GLushort *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_US_TO_FLOAT(*val++);
                *buf++ = __GL_US_TO_FLOAT(*val++);
                *buf++ = __GL_US_TO_FLOAT(*val++);
                *buf++ = __GL_US_TO_FLOAT(*val);
            }
            else {
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val;
            }
        }
        break;
    case GL_INT:
        {
            GLint *val = (GLint *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_I_TO_FLOAT(*val++);
                *buf++ = __GL_I_TO_FLOAT(*val++);
                *buf++ = __GL_I_TO_FLOAT(*val++);
                *buf++ = __GL_I_TO_FLOAT(*val);
            }
            else {
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val;
            }
        }
        break;
    case GL_UNSIGNED_INT:
        {
            GLuint *val = (GLuint *)((GLubyte *)pArray + index * stride);
            if (normalized) {
                *buf++ = __GL_UI_TO_FLOAT(*val++);
                *buf++ = __GL_UI_TO_FLOAT(*val++);
                *buf++ = __GL_UI_TO_FLOAT(*val++);
                *buf++ = __GL_UI_TO_FLOAT(*val);
            }
            else {
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val++;
                *buf++ = (GLfloat)*val;
            }
        }
        break;
    case GL_FLOAT:
        {
            GLfloat *val = (GLfloat *)((GLubyte *)pArray + index * stride);
            *buf++ = *val++;
            *buf++ = *val++;
            *buf++ = *val++;
            *buf++ = *val;
        }
        break;
    case GL_DOUBLE:
        {
            GLdouble *val = (GLdouble *)((GLubyte *)pArray + index * stride);
            *buf++ = (GLfloat)*val++;
            *buf++ = (GLfloat)*val++;
            *buf++ = (GLfloat)*val++;
            *buf++ = (GLfloat)*val;
        }
        break;
    default:
        GL_ASSERT(0);
    }

    *bufptr = (GLfloat *)buf;
}

__GL_INLINE GLvoid
__glStore3ValuesTo4Ubytes(GLuint index, GLenum type, GLsizei stride, GLvoid *pArray, GLfloat **bufptr)
{
    GLubyte *buf = (GLubyte *)(*bufptr);

    switch (type) {
    case GL_BYTE:
        {
            GLbyte *val = (GLbyte *)((GLubyte *)pArray + index * stride);
            *buf++ = __GL_B_TO_UBYTE(*val++);
            *buf++ = __GL_B_TO_UBYTE(*val++);
            *buf++ = __GL_B_TO_UBYTE(*val);
            *buf++ = 255;
        }
        break;
    case GL_UNSIGNED_BYTE:
        {
            GLubyte *val = (GLubyte *)((GLubyte *)pArray + index * stride);
            *buf++ = *val++;
            *buf++ = *val++;
            *buf++ = *val;
            *buf++ = 255;
        }
        break;
    default:
        GL_ASSERT(0);
    }

    *bufptr = (GLfloat *)buf;
}

__GL_INLINE GLvoid
__glStore4ValuesTo4Ubytes(GLuint index, GLenum type, GLsizei stride, GLvoid *pArray, GLfloat **bufptr)
{
    GLubyte *buf = (GLubyte *)(*bufptr);

    switch (type) {
    case GL_BYTE:
        {
            GLbyte *val = (GLbyte *)((GLubyte *)pArray + index * stride);
            *buf++ = __GL_B_TO_UBYTE(*val++);
            *buf++ = __GL_B_TO_UBYTE(*val++);
            *buf++ = __GL_B_TO_UBYTE(*val++);
            *buf++ = __GL_B_TO_UBYTE(*val);
        }
        break;
    case GL_UNSIGNED_BYTE:
        {
            GLubyte *val = (GLubyte *)((GLubyte *)pArray + index * stride);
            *buf++ = *val++;
            *buf++ = *val++;
            *buf++ = *val++;
            *buf++ = *val;
        }
        break;
    default:
        GL_ASSERT(0);
    }

    *bufptr = (GLfloat *)buf;
}

GLenum __glArrayElement_Generic(__GLcontext *gc, GLuint index, GLfloat **bufptr, GLubyte **edgeflagPtr, GLuint *tagBuf)
{
    __GLvertexArrayState *pV = &gc->clientState.vertexArray;
    __GLbufferObject *vbo;
    GLvoid *pArray;
    GLuint mask, i;

    /*
    ** Note:
    ** The following array enable checks must follow the GL Spec code sequence
    ** for ArrayElement so that elemOffsetDW[] match the copied element data in buffer.
    */
    if (pV->arrayEnabled & __GL_VARRAY_NORMAL) {
        if (pV->normal.bufBinding == 0) {
            pArray = (GLvoid *)pV->normal.pointer;
        } else {
            vbo = gc->bufferObject.boundNormal;
            if (vbo->bufferMapped) {
                return GL_INVALID_OPERATION;
            }
            pArray = (GLubyte *)(*gc->dp.mapBuffer)(gc, vbo) + pV->normal.offset;
        }

        switch (pV->normal.size) {
        case 3:
            __glStore3ValuesTo3Floats(index, pV->normal.type,
                    pV->normal.stride, pV->normal.normalized, pArray, bufptr);

            *tagBuf++ = __GL_N3F_TAG;
            break;
        default:
            GL_ASSERT(0);
        }
    }

    if (pV->arrayEnabled & __GL_VARRAY_DIFFUSE) {
        if (pV->color.bufBinding == 0) {
            pArray = (GLvoid *)pV->color.pointer;
        } else {
            vbo = gc->bufferObject.boundColor;
            if (vbo->bufferMapped) {
                return GL_INVALID_OPERATION;
            }
            pArray = (GLubyte *)(*gc->dp.mapBuffer)(gc, vbo)+ pV->color.offset;
        }

        switch (pV->color.size) {
        case 3:
            if (pV->color.type == GL_BYTE || pV->color.type == GL_UNSIGNED_BYTE) {
                __glStore3ValuesTo4Ubytes(index, pV->color.type,
                        pV->color.stride, pArray, bufptr);

                *tagBuf++ = __GL_C4UB_TAG;
            }
            else {
                __glStore3ValuesTo3Floats(index, pV->color.type,
                        pV->color.stride, pV->color.normalized, pArray, bufptr);

                *tagBuf++ = __GL_C3F_TAG;
            }
            break;
        case 4:
            if (pV->color.type == GL_BYTE || pV->color.type == GL_UNSIGNED_BYTE) {
                __glStore4ValuesTo4Ubytes(index, pV->color.type,
                        pV->color.stride, pArray, bufptr);

                *tagBuf++ = __GL_C4UB_TAG;
            }
            else {
                __glStore4ValuesTo4Floats(index, pV->color.type,
                        pV->color.stride, pV->color.normalized, pArray, bufptr);

                *tagBuf++ = __GL_C4F_TAG;
            }
            break;
        default:
            GL_ASSERT(0);
        }
    }

    if (pV->arrayEnabled & __GL_VARRAY_SPECULAR) {
        if (pV->color2.bufBinding == 0) {
            pArray = (GLvoid *)pV->color2.pointer;
        } else {
            vbo = gc->bufferObject.boundColor2;
            if (vbo->bufferMapped) {
                return GL_INVALID_OPERATION;
            }
            pArray = (GLubyte *)(*gc->dp.mapBuffer)(gc, vbo) + pV->color2.offset;
        }

        switch (pV->color2.size) {
        case 3:
            __glStore3ValuesTo3Floats(index, pV->color2.type,
                    pV->color2.stride, pV->color2.normalized, pArray, bufptr);

            *tagBuf++ = __GL_SC3F_TAG;
            break;
        default:
            GL_ASSERT(0);
        }
    }

    if (pV->arrayEnabled & __GL_VARRAY_FOGCOORD) {
        if (pV->fogcoord.bufBinding == 0) {
            pArray = (GLvoid *)pV->fogcoord.pointer;
        } else {
            vbo = gc->bufferObject.boundFog;
            if (vbo->bufferMapped) {
                return GL_INVALID_OPERATION;
            }
            pArray = (GLubyte *)(*gc->dp.mapBuffer)(gc, vbo)+ pV->fogcoord.offset;
        }

        switch (pV->fogcoord.size) {
        case 1:
            __glStore1ValueTo1Float(index, pV->fogcoord.type,
                    pV->fogcoord.stride, pV->fogcoord.normalized, pArray, bufptr);

            *tagBuf++ = __GL_FOG1F_TAG;
            break;
        default:
            GL_ASSERT(0);
        }
    }

    if (pV->arrayEnabled & __GL_VARRAY_TEXTURES) {
        mask = (pV->arrayEnabled & __GL_VARRAY_TEXTURES) >> __GL_VARRAY_TEX0_INDEX;
        i = 0;
        while (mask) {
            if (mask & 0x1) {
                if (pV->texture[i].bufBinding == 0) {
                    pArray = (GLvoid *)pV->texture[i].pointer;
                } else {
                    vbo = gc->bufferObject.boundTexture[i];
                    if (vbo->bufferMapped) {
                        return GL_INVALID_OPERATION;
                    }
                    pArray = (GLubyte *)(*gc->dp.mapBuffer)(gc, vbo) + pV->texture[i].offset;
                }

                switch (pV->texture[i].size) {
                case 1:
                    __glStore1ValueTo2Floats(index, pV->texture[i].type,
                            pV->texture[i].stride, pV->texture[i].normalized, pArray, bufptr);

                    *tagBuf++ = __GL_TC2F_TAG + i;
                    break;
                case 2:
                    __glStore2ValuesTo2Floats(index, pV->texture[i].type,
                            pV->texture[i].stride, pV->texture[i].normalized, pArray, bufptr);

                    *tagBuf++ = __GL_TC2F_TAG + i;
                    break;
                case 3:
                    __glStore3ValuesTo3Floats(index, pV->texture[i].type,
                            pV->texture[i].stride, pV->texture[i].normalized, pArray, bufptr);

                    *tagBuf++ = __GL_TC3F_TAG + i;
                    break;
                case 4:
                    __glStore4ValuesTo4Floats(index, pV->texture[i].type,
                            pV->texture[i].stride, pV->texture[i].normalized, pArray, bufptr);

                    *tagBuf++ = __GL_TC4F_TAG + i;
                    break;
                }
            }
            mask >>= 1;
            i += 1;
        }
    }


    if ((*edgeflagPtr) && (pV->arrayEnabled & __GL_VARRAY_EDGEFLAG)) {
        GLboolean *flag;

        if (pV->edgeflag.bufBinding == 0) {
            pArray = (GLvoid *)pV->edgeflag.pointer;
        } else {
            vbo = gc->bufferObject.boundEdgeflag;
            if (vbo->bufferMapped) {
                return GL_INVALID_OPERATION;
            }
            pArray = (GLubyte *)(*gc->dp.mapBuffer)(gc, vbo) + pV->edgeflag.offset;
        }

        flag = (GLboolean *)((GLubyte *)pArray + index * pV->edgeflag.stride);
        *(*edgeflagPtr)++ = *flag;

        *tagBuf++ = __GL_EDGEFLAG_TAG;
    }

    if (pV->arrayEnabled & __GL_VARRAY_ATTRIBUTES) {
        mask = (pV->arrayEnabled & __GL_VARRAY_ATTRIBUTES) >> __GL_VARRAY_ATT0_INDEX;
        i = 0;
        while (mask) {
            if (mask & 0x1) {
                if (pV->attribute[i].bufBinding == 0) {
                    pArray = (GLvoid *)pV->attribute[i].pointer;
                } else {
                    vbo = gc->bufferObject.boundAttribute[i];
                    if (vbo->bufferMapped) {
                        return GL_INVALID_OPERATION;
                    }
                    pArray = (GLubyte *)(*gc->dp.mapBuffer)(gc, vbo) + pV->attribute[i].offset;
                }

                switch (pV->attribute[i].size) {
                case 1:
                    __glStore1ValueTo4Floats(index, pV->attribute[i].type,
                            pV->attribute[i].stride, pV->attribute[i].normalized, pArray, bufptr);
                    break;
                case 2:
                    __glStore2ValuesTo4Floats(index, pV->attribute[i].type,
                            pV->attribute[i].stride, pV->attribute[i].normalized, pArray, bufptr);
                    break;
                case 3:
                    __glStore3ValuesTo4Floats(index, pV->attribute[i].type,
                            pV->attribute[i].stride, pV->attribute[i].normalized, pArray, bufptr);
                    break;
                case 4:
                    __glStore4ValuesTo4Floats(index, pV->attribute[i].type,
                            pV->attribute[i].stride, pV->attribute[i].normalized, pArray, bufptr);
                    break;
                default:
                    GL_ASSERT(0);
                }

                *tagBuf++ = __GL_AT4F_I0_TAG + i;
            }
            mask >>= 1;
            i++;
        }
    }

    if (pV->arrayEnabled & __GL_VARRAY_ATT0) {
        if (pV->attribute[0].bufBinding == 0) {
            pArray = (GLvoid *)pV->attribute[0].pointer;
        } else {
            vbo = gc->bufferObject.boundAttribute[0];
            if (vbo->bufferMapped) {
                return GL_INVALID_OPERATION;
            }
            pArray = (GLubyte *)(*gc->dp.mapBuffer)(gc, vbo) + pV->attribute[0].offset;
        }

        switch (pV->attribute[0].size) {
        case 1:
            __glStore1ValueTo2Floats(index, pV->attribute[0].type,
                    pV->attribute[0].stride, pV->attribute[0].normalized, pArray, bufptr);

            *tagBuf++ = __GL_V2F_TAG;

            break;
        case 2:
            __glStore2ValuesTo2Floats(index, pV->attribute[0].type,
                    pV->attribute[0].stride, pV->attribute[0].normalized, pArray, bufptr);

            *tagBuf++ = __GL_V2F_TAG;

            break;
        case 3:
            __glStore3ValuesTo3Floats(index, pV->attribute[0].type,
                    pV->attribute[0].stride, pV->attribute[0].normalized, pArray, bufptr);

            *tagBuf++ = __GL_V3F_TAG;

            break;
        case 4:
            __glStore4ValuesTo4Floats(index, pV->attribute[0].type,
                    pV->attribute[0].stride, pV->attribute[0].normalized, pArray, bufptr);

            *tagBuf++ = __GL_V4F_TAG;

            break;
        default:
            GL_ASSERT(0);
        }
    }
    else if (pV->arrayEnabled & __GL_VARRAY_VERTEX)
    {
        if (pV->vertex.bufBinding == 0) {
            pArray = (GLvoid *)pV->vertex.pointer;
        } else {
            vbo = gc->bufferObject.boundVertex;
            if (vbo->bufferMapped) {
                return GL_INVALID_OPERATION;
            }
            pArray = (GLubyte *)(*gc->dp.mapBuffer)(gc, vbo) + pV->vertex.offset;
        }

        switch (pV->vertex.size) {
        case 2:
            __glStore2ValuesTo2Floats(index, pV->vertex.type,
                    pV->vertex.stride, pV->vertex.normalized, pArray, bufptr);

            *tagBuf++ = __GL_V2F_TAG;

            break;
        case 3:
            __glStore3ValuesTo3Floats(index, pV->vertex.type,
                    pV->vertex.stride, pV->vertex.normalized, pArray, bufptr);

            *tagBuf++ = __GL_V3F_TAG;

            break;
        case 4:
            __glStore4ValuesTo4Floats(index, pV->vertex.type,
                    pV->vertex.stride, pV->vertex.normalized, pArray, bufptr);

            *tagBuf++ = __GL_V4F_TAG;

            break;
        default:
            GL_ASSERT(0);
        }
    }

    return GL_NO_ERROR;
}

/******************************************************************************************/

GLvoid __glArrayElement_V2F(__GLcontext *gc, GLuint element, GLfloat **bufptr)
{
    __GLvertexArrayState *pV = &gc->clientState.vertexArray;
    GLfloat *pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + element * pV->vertex.stride);
    GLfloat *buf = (GLfloat *)(*bufptr);

    *buf++ = pVertex[0];
    *buf++ = pVertex[1];
    *bufptr = (GLfloat *)buf;
}

GLvoid __glArrayElement_V3F(__GLcontext *gc, GLuint element, GLfloat **bufptr)
{
    __GLvertexArrayState *pV = &gc->clientState.vertexArray;
    GLfloat *pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + element * pV->vertex.stride);
    GLfloat *buf = (GLfloat *)(*bufptr);

    *buf++ = pVertex[0];
    *buf++ = pVertex[1];
    *buf++ = pVertex[2];
    *bufptr = (GLfloat *)buf;
}

GLvoid __glArrayElement_C4UB_V2F(__GLcontext *gc, GLuint element, GLfloat **bufptr)
{
    __GLvertexArrayState *pV = &gc->clientState.vertexArray;
    GLfloat *pColor = (GLfloat *)((GLubyte *)pV->color.pointer + element * pV->color.stride);
    GLfloat *pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + element * pV->vertex.stride);
    GLfloat *buf = (GLfloat *)(*bufptr);

    *buf++ = pColor[0];
    *buf++ = pVertex[0];
    *buf++ = pVertex[1];
    *bufptr = (GLfloat *)buf;
}

GLvoid __glArrayElement_C4UB_V3F(__GLcontext *gc, GLuint element, GLfloat **bufptr)
{
    __GLvertexArrayState *pV = &gc->clientState.vertexArray;
    GLfloat *pColor = (GLfloat *)((GLubyte *)pV->color.pointer + element * pV->color.stride);
    GLfloat *pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + element * pV->vertex.stride);
    GLfloat *buf = (GLfloat *)(*bufptr);

    *buf++ = pColor[0];
    *buf++ = pVertex[0];
    *buf++ = pVertex[1];
    *buf++ = pVertex[2];
    *bufptr = (GLfloat *)buf;
}

GLvoid __glArrayElement_C3F_V3F(__GLcontext *gc, GLuint element, GLfloat **bufptr)
{
    __GLvertexArrayState *pV = &gc->clientState.vertexArray;
    GLfloat *pColor = (GLfloat *)((GLubyte *)pV->color.pointer + element * pV->color.stride);
    GLfloat *pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + element * pV->vertex.stride);
    GLfloat *buf = (GLfloat *)(*bufptr);

    *buf++ = pColor[0];
    *buf++ = pColor[1];
    *buf++ = pColor[2];
    *buf++ = pVertex[0];
    *buf++ = pVertex[1];
    *buf++ = pVertex[2];
    *bufptr = (GLfloat *)buf;
}

GLvoid __glArrayElement_N3F_V3F(__GLcontext *gc, GLuint element, GLfloat **bufptr)
{
    __GLvertexArrayState *pV = &gc->clientState.vertexArray;
    GLfloat *pNormal = (GLfloat *)((GLubyte *)pV->normal.pointer + element * pV->normal.stride);
    GLfloat *pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + element * pV->vertex.stride);
    GLfloat *buf = (GLfloat *)(*bufptr);

    *buf++ = pNormal[0];
    *buf++ = pNormal[1];
    *buf++ = pNormal[2];
    *buf++ = pVertex[0];
    *buf++ = pVertex[1];
    *buf++ = pVertex[2];
    *bufptr = (GLfloat *)buf;
}

GLvoid __glArrayElement_C4F_N3F_V3F(__GLcontext *gc, GLuint element, GLfloat **bufptr)
{
    __GLvertexArrayState *pV = &gc->clientState.vertexArray;
    GLfloat *pColor = (GLfloat *)((GLubyte *)pV->color.pointer + element * pV->color.stride);
    GLfloat *pNormal = (GLfloat *)((GLubyte *)pV->normal.pointer + element * pV->normal.stride);
    GLfloat *pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + element * pV->vertex.stride);
    GLfloat *buf = (GLfloat *)(*bufptr);

    *buf++ = pColor[0];
    *buf++ = pColor[1];
    *buf++ = pColor[2];
    *buf++ = pColor[3];
    *buf++ = pNormal[0];
    *buf++ = pNormal[1];
    *buf++ = pNormal[2];
    *buf++ = pVertex[0];
    *buf++ = pVertex[1];
    *buf++ = pVertex[2];
    *bufptr = (GLfloat *)buf;
}

GLvoid __glArrayElement_T2F_V3F(__GLcontext *gc, GLuint element, GLfloat **bufptr)
{
    __GLvertexArrayState *pV = &gc->clientState.vertexArray;
    GLfloat *pTexture = (GLfloat *)((GLubyte *)pV->texture[0].pointer + element * pV->texture[0].stride);
    GLfloat *pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + element * pV->vertex.stride);
    GLfloat *buf = (GLfloat *)(*bufptr);

    *buf++ = pTexture[0];
    *buf++ = pTexture[1];
    *buf++ = pVertex[0];
    *buf++ = pVertex[1];
    *buf++ = pVertex[2];
    *bufptr = (GLfloat *)buf;
}

GLvoid __glArrayElement_T4F_V4F(__GLcontext *gc, GLuint element, GLfloat **bufptr)
{
    __GLvertexArrayState *pV = &gc->clientState.vertexArray;
    GLfloat *pTexture = (GLfloat *)((GLubyte *)pV->texture[0].pointer + element * pV->texture[0].stride);
    GLfloat *pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + element * pV->vertex.stride);
    GLfloat *buf = (GLfloat *)(*bufptr);

    *buf++ = pTexture[0];
    *buf++ = pTexture[1];
    *buf++ = pTexture[2];
    *buf++ = pTexture[3];
    *buf++ = pVertex[0];
    *buf++ = pVertex[1];
    *buf++ = pVertex[2];
    *buf++ = pVertex[3];
    *bufptr = (GLfloat *)buf;
}

GLvoid __glArrayElement_T2F_C4UB_V3F(__GLcontext *gc, GLuint element, GLfloat **bufptr)
{
    __GLvertexArrayState *pV = &gc->clientState.vertexArray;
    GLfloat *pTexture = (GLfloat *)((GLubyte *)pV->texture[0].pointer + element * pV->texture[0].stride);
    GLfloat *pColor = (GLfloat *)((GLubyte *)pV->color.pointer + element * pV->color.stride);
    GLfloat *pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + element * pV->vertex.stride);
    GLfloat *buf = (GLfloat *)(*bufptr);

    *buf++ = pTexture[0];
    *buf++ = pTexture[1];
    *buf++ = pColor[0];
    *buf++ = pVertex[0];
    *buf++ = pVertex[1];
    *buf++ = pVertex[2];
    *bufptr = (GLfloat *)buf;
}

GLvoid __glArrayElement_T2F_C3F_V3F(__GLcontext *gc, GLuint element, GLfloat **bufptr)
{
    __GLvertexArrayState *pV = &gc->clientState.vertexArray;
    GLfloat *pTexture = (GLfloat *)((GLubyte *)pV->texture[0].pointer + element * pV->texture[0].stride);
    GLfloat *pColor = (GLfloat *)((GLubyte *)pV->color.pointer + element * pV->color.stride);
    GLfloat *pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + element * pV->vertex.stride);
    GLfloat *buf = (GLfloat *)(*bufptr);

    *buf++ = pTexture[0];
    *buf++ = pTexture[1];
    *buf++ = pColor[0];
    *buf++ = pColor[1];
    *buf++ = pColor[2];
    *buf++ = pVertex[0];
    *buf++ = pVertex[1];
    *buf++ = pVertex[2];
    *bufptr = (GLfloat *)buf;
}

GLvoid __glArrayElement_T2F_N3F_V3F(__GLcontext *gc, GLuint element, GLfloat **bufptr)
{
    __GLvertexArrayState *pV = &gc->clientState.vertexArray;
    GLfloat *pTexture = (GLfloat *)((GLubyte *)pV->texture[0].pointer + element * pV->texture[0].stride);
    GLfloat *pNormal = (GLfloat *)((GLubyte *)pV->normal.pointer + element * pV->normal.stride);
    GLfloat *pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + element * pV->vertex.stride);
    GLfloat *buf = (GLfloat *)(*bufptr);

    *buf++ = pTexture[0];
    *buf++ = pTexture[1];
    *buf++ = pNormal[0];
    *buf++ = pNormal[1];
    *buf++ = pNormal[2];
    *buf++ = pVertex[0];
    *buf++ = pVertex[1];
    *buf++ = pVertex[2];
    *bufptr = (GLfloat *)buf;
}

GLvoid __glArrayElement_T2F_C4F_N3F_V3F(__GLcontext *gc, GLuint element, GLfloat **bufptr)
{
    __GLvertexArrayState *pV = &gc->clientState.vertexArray;
    GLfloat *pTexture = (GLfloat *)((GLubyte *)pV->texture[0].pointer + element * pV->texture[0].stride);
    GLfloat *pColor = (GLfloat *)((GLubyte *)pV->color.pointer + element * pV->color.stride);
    GLfloat *pNormal = (GLfloat *)((GLubyte *)pV->normal.pointer + element * pV->normal.stride);
    GLfloat *pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + element * pV->vertex.stride);
    GLfloat *buf = (GLfloat *)(*bufptr);

    *buf++ = pTexture[0];
    *buf++ = pTexture[1];
    *buf++ = pColor[0];
    *buf++ = pColor[1];
    *buf++ = pColor[2];
    *buf++ = pColor[3];
    *buf++ = pNormal[0];
    *buf++ = pNormal[1];
    *buf++ = pNormal[2];
    *buf++ = pVertex[0];
    *buf++ = pVertex[1];
    *buf++ = pVertex[2];
    *bufptr = (GLfloat *)buf;
}

GLvoid __glArrayElement_T4F_C4F_N3F_V4F(__GLcontext *gc, GLuint element, GLfloat **bufptr)
{
    __GLvertexArrayState *pV = &gc->clientState.vertexArray;
    GLfloat *pTexture = (GLfloat *)((GLubyte *)pV->texture[0].pointer + element * pV->texture[0].stride);
    GLfloat *pColor = (GLfloat *)((GLubyte *)pV->color.pointer + element * pV->color.stride);
    GLfloat *pNormal = (GLfloat *)((GLubyte *)pV->normal.pointer + element * pV->normal.stride);
    GLfloat *pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + element * pV->vertex.stride);
    GLfloat *buf = (GLfloat *)(*bufptr);

    *buf++ = pTexture[0];
    *buf++ = pTexture[1];
    *buf++ = pTexture[2];
    *buf++ = pTexture[3];
    *buf++ = pColor[0];
    *buf++ = pColor[1];
    *buf++ = pColor[2];
    *buf++ = pColor[3];
    *buf++ = pNormal[0];
    *buf++ = pNormal[1];
    *buf++ = pNormal[2];
    *buf++ = pVertex[0];
    *buf++ = pVertex[1];
    *buf++ = pVertex[2];
    *buf++ = pVertex[3];
    *bufptr = (GLfloat *)buf;
}
