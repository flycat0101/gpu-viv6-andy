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


#ifndef __gc_gl_constantbuffer_h_
#define __gc_gl_constantbuffer_h_

#define __GL_MAX_CONSTANT_BUFFERS       15

typedef struct __GLconstantBufferRec{

    /* if bindable, bufObj point to bound buffer object */
    GLboolean           bindable;

    /* data of constant buffer changed, check if need to reload constant */
    GLboolean           globalDirty;

    __GLbufferObject*   constBufObj;

} __GLconstantBuffer;

__GLconstantBuffer* __glCreateConstantBuffer(__GLcontext* gc, GLuint size, GLboolean bindable);
GLvoid __glDestroyConstantBuffer(__GLcontext* gc, __GLconstantBuffer* pBuffer);
GLboolean __glSetBindableConstantBuffer(__GLcontext* gc, __GLconstantBuffer* pBuffer, __GLbufferObject* bufObj);

#endif
