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


#ifndef __gc_es_vertex_h__
#define __gc_es_vertex_h__

#define __GL_MAX_VAO_LINEAR_TABLE_SIZE         1024
#define __GL_DEFAULT_VAO_LINEAR_TABLE_SIZE     256
#define __GL_VAO_HASH_TABLE_SIZE               512


/****************************************************************************/
/*
** Vertex array state.
*/

typedef struct __GLvertexAttribRec
{
    GLint             size;
    GLenum            type;
    GLsizei           usr_stride;   /* User defined stride which could be 0 */
    GLboolean         normalized;
    GLboolean         integer;      /* unconverted integers */
    GLuint            relativeOffset;
    const GLvoid     *pointer;

    GLuint            attribBinding;
}__GLvertexAttrib;

typedef struct __GLvertexAttribBindingRec
{
    /* Buffer object binding info */
    GLuint            boundArrayName;
    __GLbufferObject *boundArrayObj;
    GLintptr          offset;
    GLuint            divisor;
    GLsizei           stride;       /* Actual stride, which could not be 0 */
}__GLvertexAttribBinding;



typedef struct __GLvertexArrayStateRec
{
    __GLvertexAttrib attribute[__GL_MAX_VERTEX_ATTRIBUTES];
    __GLvertexAttribBinding attributeBinding[__GL_MAX_VERTEX_ATTRIBUTE_BINDINGS];

    GLuint            attribEnabled;  /* bitvector of the enabled attributes */

    GLuint            boundIdxName;
    __GLbufferObject *boundIdxObj;

} __GLvertexArrayState;

/*
** Referenced by "gc->vertexArray.varrayDirty"
 */
enum
{
    __GL_DIRTY_VARRAY_MODE          = 0,
    __GL_DIRTY_VARRAY_ENABLE        = 1,
    __GL_DIRTY_VARRAY_FORMAT        = 2,
    __GL_DIRTY_VARRAY_BINDING       = 3,
    __GL_DIRTY_VARRAY_OFFSET        = 4,
    __GL_DIRTY_VARRAY_DIVISOR       = 5,
    __GL_DIRTY_VARRAY_END           = 6
};

#define __GL_DIRTY_VARRAY_MODE_BIT          (1 << __GL_DIRTY_VARRAY_MODE)
#define __GL_DIRTY_VARRAY_ENABLE_BIT        (1 << __GL_DIRTY_VARRAY_ENABLE)
#define __GL_DIRTY_VARRAY_FORMAT_BIT        (1 << __GL_DIRTY_VARRAY_FORMAT)
#define __GL_DIRTY_VARRAY_BINDING_BIT       (1 << __GL_DIRTY_VARRAY_BINDING)
#define __GL_DIRTY_VARRAY_OFFSET_BIT        (1 << __GL_DIRTY_VARRAY_OFFSET)
#define __GL_DIRTY_VARRAY_DIVISOR_BIT       (1 << __GL_DIRTY_VARRAY_DIVISOR)

/*
** Macros that set vertex array dirty bits
*/
#define __GL_SET_VARRAY_MODE_BIT(gc) \
    (gc)->vertexArray.varrayDirty |= __GL_DIRTY_VARRAY_MODE_BIT

#define __GL_SET_VARRAY_ENABLE_BIT(gc) \
    (gc)->vertexArray.varrayDirty |= __GL_DIRTY_VARRAY_ENABLE_BIT

#define __GL_SET_VARRAY_FORMAT_BIT(gc) \
    (gc)->vertexArray.varrayDirty |= __GL_DIRTY_VARRAY_FORMAT_BIT

#define __GL_SET_VARRAY_BINDING_BIT(gc) \
    (gc)->vertexArray.varrayDirty |= __GL_DIRTY_VARRAY_BINDING_BIT

#define __GL_SET_VARRAY_OFFSET_BIT(gc) \
    (gc)->vertexArray.varrayDirty |= __GL_DIRTY_VARRAY_OFFSET_BIT

#define __GL_SET_VARRAY_DIVISOR_BIT(gc) \
    (gc)->vertexArray.varrayDirty |= __GL_DIRTY_VARRAY_DIVISOR_BIT

/*
** Vertex array object
*/
typedef struct __GLvertexArrayObjectRec
{
    GLuint                  name;
    __GLvertexArrayState    vertexArray;

    GLchar *label;
} __GLvertexArrayObject;


/*
** Vertex array machine. internally used by driver to manage vertex array
*/
typedef struct __GLvertexArrayMachineRec
{
    /* Vertex array objects are not shared between contexts according to the Spec.
    ** we just use the __GLsharedObjectMachine to manage the vertex array objects.
    */
    __GLsharedObjectMachine *noShare;

    /* Current bound vertex array object name */
    GLuint                   boundVertexArray;

    __GLvertexArrayObject    defaultVAO;

    /* Current bound vertex array object */
    __GLvertexArrayObject   *boundVAO;

    /* 0: glDrawArrays(); Non-0: glDrawElements() */
    GLsizei                  indexCount;
    GLenum                   indexType;

    /* Used for DrawElements, DrawArraysIndirect, DrawElementsIndirect */
    const GLvoid            *indices;  /* index array pointer or VBO offset */
    GLboolean                drawIndirect; /* TRUE or FALSE */
    const GLvoid            *indirectOffset;

    /* Used for MultiDrawElementsIndirect, MultiDrawArraysIndirect:
    ** <stride> specifies the distance, in basic machine units, between the elements of the array.
    ** If <stride> is zero, the array elements are treated as tightly packed.
    */
    GLsizei drawcount;
    GLsizei stride;
    GLboolean multidrawIndirect;

    /* PrimitiveMode at API level */
    GLenum                   primMode;

    GLsizei                  instanceCount;

    /* The start (inclusive) vertex index and the end (exclusive) vertex index
    ** The actual vertex count to be rendered can be get by (end - start)
    */
    GLint                    start;
    GLint                    end;

    /* For non-index mode, baseVertex is same as start;
    */
    GLint                    baseVertex;

    /* Vertex array dirty bits
    */
    GLbitfield               varrayDirty;

} __GLvertexArrayMachine;

typedef struct __GLdrawArraysIndirectCommandRec
{
    GLuint count;
    GLuint instanceCount;
    GLuint first;
    GLuint reservedMustBeZero;
}__GLdrawArraysIndirectCommand;


typedef struct __GLdrawElementsIndirectCommandRec
{
    GLuint  count;
    GLuint  instanceCount;
    GLuint  firstIndex;
    GLint   baseVertex;
    GLuint reservedMustBeZero;
} __GLdrawElementsIndirectCommand;

__GL_INLINE GLsizei __glUtilCalculateStride(GLint size, GLenum type)
{
    switch (type)
    {
        case GL_BYTE:
            return (sizeof(GLbyte) * size);
        case GL_UNSIGNED_BYTE:
            return (sizeof(GLubyte) * size);
        case GL_SHORT:
            return (sizeof(GLshort) * size);
        case GL_UNSIGNED_SHORT:
            return (sizeof(GLushort) * size);
        case GL_FIXED:
            return (sizeof(GLfixed) * size);
        case GL_FLOAT:
            return (sizeof(GLfloat) * size);
        case GL_INT_10_10_10_2_OES:
            return sizeof(GLint);
        case GL_UNSIGNED_INT_10_10_10_2_OES:
            return sizeof(GLuint);
        case GL_HALF_FLOAT_OES:
            return (sizeof(GLhalf) * size);
        case GL_INT:
            return (sizeof(GLint) * size);
        case GL_INT_2_10_10_10_REV:
            return sizeof(GLint);
        case GL_UNSIGNED_INT:
            return (sizeof(GLuint) * size);
        case GL_UNSIGNED_INT_2_10_10_10_REV:
            return sizeof(GLuint);
        case GL_HALF_FLOAT:
            return (sizeof(GLhalf) * size);

        default:
            GL_ASSERT(0);/*error check should be done in the caller*/
    }

    return 0;
}


#endif /* __gc_es_vertex_h__ */
