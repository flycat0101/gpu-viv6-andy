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


#ifndef __gc_gl_vsoutput_h_
#define __gc_gl_vsoutput_h_

typedef struct __GLVSOutputRec{
    /*** shared data between swvs/hwvs */
    GLboolean                          bValid;                                    /*If current vs output success.*/

    /* Buffer manage
    ** When sw rasterize, or hw raster but get device VB fail, use vertexOutputBuffer as
    ** TnLed vertex data output buffer.
    ** clipCodeBuffer, vertexOutputBuffer-- default output buffers (resident in system)
    ** outVBSize -- total byte size of vertexOutputBuffer
    ** outClipSize --total byte size of clipCodeBuffer */
    GLubyte                             *clipCodeBuffer;
    GLubyte                             *vertexOutputBuffer;
    GLuint                                outVBSize;
    GLuint                                outClipSize;
    GLuint                                incVBSize;
    GLuint                                incClipSize;

    GLubyte*                            actualOutputBuffer;
    GLubyte*                            actualClipcodeBuffer;

    __GLresidenceType           actualVBlocation;     /* Where is the resource? */

    /* If swvs do clip code gen, we accum the clipcode to pick fast path. */
    GLuint                                accumedClipAND;
    GLuint                                accumedClipOR;

    GLuint                                outputVertexCount;
    GLuint                                outputStride;   /*In bytes */

    GLuint      vsOutFlags;

    /*** data swvs required */
    /* For, user clip plane clip code calculation.
    */
    __GLcoord                       scrCoordClipPlanes[6];
    __GLcoord                       clpCoordClipPlanes[6];

    /*** data hwvs reqired */
    GLubyte             * clipSpacePosBuffer;
    GLubyte             * actualClipSpacePosBuffer;
    GLuint                outClipSpacePosSize;


}__GLVSOutput;

#endif /* __gc_gl_vsoutput_h_ */
