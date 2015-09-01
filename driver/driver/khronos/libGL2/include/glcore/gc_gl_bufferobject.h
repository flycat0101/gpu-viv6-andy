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


#ifndef __gc_gl_bufferobject_h_
#define __gc_gl_bufferobject_h_

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


/*buffer object targets*/
#define __GL_ARRAY_BUFFER_INDEX         0
#define __GL_ELEMENT_ARRAY_BUFFER_INDEX 1
#define __GL_PIXEL_PACK_BUFFER_INDEX    2
#define __GL_PIXEL_UNPACK_BUFFER_INDEX  3
#define __GL_UNIFORM_BUFFER_INDEX       4
#define __GL_TEXTURE_BUFFER_EXT_INDEX   5

typedef struct __GLbufferObjDataRec{

    /* Check if systemcache is consistant with vid momery. Based on __GLcoord */
    GLboolean *bufferObjDataDirty;
    /* List of bound \constant\uniform\texture buffer pointer. May shared by VS, PS
    ** or different program object
    */
    __GLimageUser *bufferObjUserList;
}__GLbufferObjData;


typedef struct __GLbufferObjectRec{
    /* indicate how many targets the object is currently bound to
    */
    GLuint bindCount;


    /* The seqNumber is increased by 1 whenever object states are changed.
      ** DP must update its internal object states if its internal copy of
      ** savedSeqNum is different from this seqNumber.
      */
      GLuint seqNumber;

    /* Internal flag for generic object management. */
    GLbitfield flag;

    /* Is buffer object created in vidmem or agp sucessfully*/
    GLboolean bufInDeviceMemory;

    /* This is the object privateData that can be shared by different contexts
    ** that are bound to the object.
    */
    GLvoid *privateData;

    GLuint name;
    GLsizeiptr size;
    GLuint usage;
    GLenum access;
    GLboolean bufferMapped;
    GLvoid *mapPointer;

    GLbyte *systemMemCache;
    GLboolean systemCacheUptodate;

    /* Only for bindable constant\uniform\texture buffer */
    __GLbufferObjData * bufferObjData;

}__GLbufferObject;


typedef struct __GLbufferObjectMachineRec{
    __GLsharedObjectMachine *shared;

    /* The buffer object pointers bound to the targets */
    __GLbufferObject *boundTarget[__GL_MAX_VERTEX_BUFFER_BINDINGS];

    /* The buffer object names bound to the targets */
    GLuint boundBuffer[__GL_MAX_VERTEX_BUFFER_BINDINGS];

    /* The buffer objects that bound to each vertex array */
    union
    {
        struct
        {
            __GLbufferObject *boundVertex;
            __GLbufferObject *boundWeight;
            __GLbufferObject *boundNormal;
            __GLbufferObject *boundColor;
            __GLbufferObject *boundColor2;
            __GLbufferObject *boundFog;
            __GLbufferObject *boundEdgeflag;
            __GLbufferObject *boundcolorIndex;
            __GLbufferObject *boundTexture[__GL_MAX_TEXTURE_COORDS];
            __GLbufferObject *boundAttribute[__GL_MAX_PROGRAM_VERTEX_ATTRIBUTES];
        };
        __GLbufferObject *boundArrays[__GL_TOTAL_VERTEX_ATTRIBUTES];
    };

    /* A general buffer object Template used for initialize standard buffer object */
    __GLbufferObject bufObjTemplate;

} __GLbufferObjectMachine;

extern GLvoid __glBindBuffer(__GLcontext *gc, GLuint targetIndex, GLuint buffer);

#endif /* __gc_gl_bufferobject_h_ */

