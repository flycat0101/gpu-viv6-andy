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


#ifndef __chip_vbo_h_
#define __chip_vbo_h_

extern void __glChipBindBufferObject(__GLcontext* gc, __GLbufferObject* bufObj, GLuint targetIndex);
extern GLvoid __glChipDeleteBufferObject(__GLcontext* gc, __GLbufferObject* bufObj);
extern GLvoid*__glChiptMapBufferObject(__GLcontext* gc, __GLbufferObject* bufObj);
extern GLboolean __glChipUnMapBufferObject(__GLcontext* gc, __GLbufferObject* bufObj);
extern GLboolean __glChipBufferData(__GLcontext* gc, __GLbufferObject* bufObj, GLuint targetIndex, const void* data);
extern GLboolean __glChipBufferSubData(__GLcontext* gc, __GLbufferObject* bufObj, GLuint targetIndex, GLintptr offset, GLsizeiptr size, const void* data);
extern GLvoid __glChipGetBufferSubData(__GLcontext* gc, __GLbufferObject* bufObj,
                                        GLintptr offset, GLsizeiptr size, void* data);
#endif /* __chip_vbo_h_ */
