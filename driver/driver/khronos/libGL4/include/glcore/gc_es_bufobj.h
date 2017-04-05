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


#ifndef __gc_gl_bufobj_h__
#define __gc_gl_bufobj_h__

/*
** Maximum linear table size for buffer object.
*/
#define __GL_MAX_BUFOBJ_LINEAR_TABLE_SIZE       16000

/*
** Default linear table size for buffer object.
*/
#define __GL_DEFAULT_BUFOBJ_LINEAR_TABLE_SIZE   1024

/*
** Maximum hash table size for buffer object. Must be 2^n.
*/
#define __GL_BUFOBJ_HASH_TABLE_SIZE             8192


/* Buffer object targets */
enum
{
    __GL_ARRAY_BUFFER_INDEX = 0,
    __GL_ELEMENT_ARRAY_BUFFER_INDEX,
    __GL_COPY_READ_BUFFER_INDEX,
    __GL_COPY_WRITE_BUFFER_INDEX,
    __GL_PIXEL_PACK_BUFFER_INDEX,
    __GL_PIXEL_UNPACK_BUFFER_INDEX,
    __GL_UNIFORM_BUFFER_INDEX,
    __GL_XFB_BUFFER_INDEX,
    __GL_DRAW_INDIRECT_BUFFER_INDEX,
    __GL_DISPATCH_INDIRECT_BUFFER_INDEX,
    __GL_ATOMIC_COUNTER_BUFFER_INDEX,
    __GL_SHADER_STORAGE_BUFFER_INDEX,
    __GL_TEXTURE_BUFFER_BINDING_EXT,

    __GL_MAX_BUFFER_INDEX,
};

typedef struct __GLbufferObjectRec
{
    /* Number of targets (could be from different contexts) that bound to the object.
    */
    GLuint bindCount;

    /* List of binding points (could be from different contexts), including both generic and array
    ** binding points, this buffer object has EVER been bound to. Will never remove from the list.
    */
    __GLimageUser *bindList;

    /* The seqNumber is increased by 1 whenever object states are changed.
    ** DP must update its internal object states if its internal copy of
    ** savedSeqNum is different from this seqNumber.
    */
    GLuint seqNumber;

    /* Internal flag for generic object management. */
    GLbitfield flag;

    /* Buffer object privateData pointer */
    GLvoid *privateData;

    GLuint name;
    GLsizeiptr size;
    GLenum usage;

    /* Map buffer */
    GLboolean bufferMapped;
    GLintptr mapOffset;
    GLsizeiptr mapLength;
    GLvoid *mapPointer;
    GLbitfield accessFlags;
    GLuint     accessOES;   /* OES_mapbuffer has different requirement */

    /* List of VAO that this buffer object is attached to */
    __GLimageUser *vaoList;

    GLchar *label;

    /* List of texture unit that this buffer object is attached to */
    __GLimageUser *texList;

} __GLbufferObject;


typedef struct __GLBufGeneralBindPointRec
{
    GLuint            boundBufName; /* Buf Name */
    __GLbufferObject *boundBufObj;  /* Buf Object */

} __GLBufGeneralBindPoint;

typedef struct __GLBufBindPointRec
{
    GLuint            boundBufName; /* Buf Name */
    __GLbufferObject *boundBufObj;  /* Buf Object */
    GLintptr          bufOffset;    /* Bound Buf Offset */
    GLsizeiptr        bufSize;      /* Bound Buf Size. If 0, indicates the whole bufObj */

} __GLBufBindPoint;

typedef struct __GLbufferObjectMachineRec
{
    /* Buffer objects can be shared between contexts according to the Spec.
    */
    __GLsharedObjectMachine *shared;

    /* General binding point. One for each target */
    __GLBufGeneralBindPoint generalBindingPoint[__GL_MAX_BUFFER_INDEX];

    /* An array to record the number of binding points for each target for easy indexing purpose */
    GLuint maxBufBindings[__GL_MAX_BUFFER_INDEX];

    /* Array binding points: an array for each target */
    __GLBufBindPoint *bindingPoints[__GL_MAX_BUFFER_INDEX];

    /* Bitmask for dirty bits on every binding points */
    __GLbitmask bindingDirties[__GL_MAX_BUFFER_INDEX];

} __GLbufferObjectMachine;

extern GLboolean __glDeleteBufferObject(__GLcontext *gc, __GLbufferObject *bufObj);
extern GLvoid __glBindBufferToGeneralPoint(__GLcontext *gc, GLuint targetIndex, GLuint buffer);

#endif /* __gc_gl_bufobj_h__ */

