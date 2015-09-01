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
#include "gl/gl_device.h"

extern GLuint fmtIndex2InputIndex[];
__GL_INLINE GLvoid __glDuplicatePreviousAttrib(__GLcontext *gc)
{
    __GLvertexInput *input;
    GLuint64 missingMask;
    GLfloat *current, *prevData;
    GLuint index, inputIdx, col4ub;

    /* Fill in the missing attributes for the current vertex */
    missingMask = gc->input.preVertexFormat & ~gc->input.vertexFormat;

    switch (missingMask)
    {
    case __GL_C3F_BIT:
        if (gc->input.color.currentPtrDW >= gc->input.defaultDataBuffer) {
            prevData = gc->input.color.currentPtrDW;
        }
        else {
            prevData = (GLfloat *)&gc->state.current.color;
        }
        gc->input.color.currentPtrDW += gc->input.vertTotalStrideDW;
        current = gc->input.color.currentPtrDW;
        current[0] = prevData[0];
        current[1] = prevData[1];
        current[2] = prevData[2];
        break;

    case __GL_C4F_BIT:
        if (gc->input.color.currentPtrDW >= gc->input.defaultDataBuffer) {
            prevData = gc->input.color.currentPtrDW;
        }
        else {
            prevData = (GLfloat *)&gc->state.current.color;
        }
        gc->input.color.currentPtrDW += gc->input.vertTotalStrideDW;
        current = gc->input.color.currentPtrDW;
        current[0] = prevData[0];
        current[1] = prevData[1];
        current[2] = prevData[2];
        current[3] = prevData[3];
        break;

    case __GL_C4UB_BIT:
        if (gc->input.color.currentPtrDW >= gc->input.defaultDataBuffer) {
            col4ub = *(GLuint *)gc->input.color.currentPtrDW;
        }
        else {
            GLubyte r = __GL_FLOAT_TO_UB(gc->state.current.color.r);
            GLubyte g = __GL_FLOAT_TO_UB(gc->state.current.color.g);
            GLubyte b = __GL_FLOAT_TO_UB(gc->state.current.color.b);
            GLubyte a = __GL_FLOAT_TO_UB(gc->state.current.color.a);
            col4ub = __GL_PACK_COLOR4UB(r, g, b, a);
        }
        gc->input.color.currentPtrDW += gc->input.vertTotalStrideDW;
        *(GLuint *)gc->input.color.currentPtrDW = col4ub;
        break;

    case __GL_N3F_BIT:
        if (gc->input.normal.currentPtrDW >= gc->input.defaultDataBuffer) {
            prevData = gc->input.normal.currentPtrDW;
        }
        else {
            prevData = (GLfloat *)&gc->state.current.normal;
        }
        gc->input.normal.currentPtrDW += gc->input.vertTotalStrideDW;
        current = gc->input.normal.currentPtrDW;
        current[0] = prevData[0];
        current[1] = prevData[1];
        current[2] = prevData[2];
        break;

    case __GL_SC3F_BIT:
        if (gc->input.color2.currentPtrDW >= gc->input.defaultDataBuffer) {
            prevData = gc->input.color2.currentPtrDW;
        }
        else {
            prevData = (GLfloat *)&gc->state.current.color2;
        }
        gc->input.color2.currentPtrDW += gc->input.vertTotalStrideDW;
        current = gc->input.color2.currentPtrDW;
        current[0] = prevData[0];
        current[1] = prevData[1];
        current[2] = prevData[2];
        break;

    case (__GL_N3F_BIT | __GL_C3F_BIT):
        if (gc->input.normal.currentPtrDW >= gc->input.defaultDataBuffer) {
            prevData = gc->input.normal.currentPtrDW;
        }
        else {
            prevData = (GLfloat *)&gc->state.current.normal;
        }
        gc->input.normal.currentPtrDW += gc->input.vertTotalStrideDW;
        current = gc->input.normal.currentPtrDW;
        current[0] = prevData[0];
        current[1] = prevData[1];
        current[2] = prevData[2];

        if (gc->input.color.currentPtrDW >= gc->input.defaultDataBuffer) {
            prevData = gc->input.color.currentPtrDW;
        }
        else {
            prevData = (GLfloat *)&gc->state.current.color;
        }
        gc->input.color.currentPtrDW += gc->input.vertTotalStrideDW;
        current = gc->input.color.currentPtrDW;
        current[0] = prevData[0];
        current[1] = prevData[1];
        current[2] = prevData[2];
        break;

    default:
        if(missingMask & __GL_EDGEFLAG_BIT) {
            if(gc->input.vertex.index == 0)
                gc->input.edgeflag.pointer[gc->input.vertex.index] = gc->state.current.edgeflag;
            else
                gc->input.edgeflag.pointer[gc->input.vertex.index] = gc->input.edgeflag.pointer[gc->input.vertex.index - 1];
            missingMask &= ~__GL_EDGEFLAG_BIT;
        }

        index = 0;
        while (missingMask) {
            if (missingMask & 0x1) {
                inputIdx = fmtIndex2InputIndex[index];
                input = &gc->input.currentInput[inputIdx];
                if (input->currentPtrDW >= gc->input.defaultDataBuffer) {
                    prevData = input->currentPtrDW;
                }
                else {
                    prevData = (GLfloat *)&gc->state.current.currentState[inputIdx];
                }
                input->currentPtrDW += gc->input.vertTotalStrideDW;
                current = input->currentPtrDW;
                switch (input->sizeDW) {
                case 4:
                    current[0] = prevData[0];
                    current[1] = prevData[1];
                    current[2] = prevData[2];
                    current[3] = prevData[3];
                    break;
                case 3:
                    current[0] = prevData[0];
                    current[1] = prevData[1];
                    current[2] = prevData[2];
                    break;
                case 2:
                    current[0] = prevData[0];
                    current[1] = prevData[1];
                    break;
                case 1:
                    current[0] = prevData[0];
                    break;
                default:
                    GL_ASSERT(0);
                    break;
                }
            }
            index++;
            missingMask >>= 1;
        }
    }
}

