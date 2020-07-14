/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
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

#ifdef OPENGL40

enum {
        __GL_INPUT_VERTEX_INDEX = 0,
        __GL_INPUT_WEIGHT_INDEX,
        __GL_INPUT_NORMAL_INDEX,
        __GL_INPUT_DIFFUSE_INDEX,
        __GL_INPUT_SPECULAR_INDEX,
        __GL_INPUT_FOGCOORD_INDEX,
        __GL_INPUT_EDGEFLAG_INDEX,
        __GL_INPUT_V7_INDEX,
        __GL_INPUT_TEX0_INDEX,
        __GL_INPUT_TEX1_INDEX,
        __GL_INPUT_TEX2_INDEX,
        __GL_INPUT_TEX3_INDEX,
        __GL_INPUT_TEX4_INDEX,
        __GL_INPUT_TEX5_INDEX,
        __GL_INPUT_TEX6_INDEX,
        __GL_INPUT_TEX7_INDEX,
        __GL_INPUT_ATT0_INDEX,
        __GL_INPUT_ATT1_INDEX,
        __GL_INPUT_ATT2_INDEX,
        __GL_INPUT_ATT3_INDEX,
        __GL_INPUT_ATT4_INDEX,
        __GL_INPUT_ATT5_INDEX,
        __GL_INPUT_ATT6_INDEX,
        __GL_INPUT_ATT7_INDEX,
        __GL_INPUT_ATT8_INDEX,
        __GL_INPUT_ATT9_INDEX,
        __GL_INPUT_ATT10_INDEX,
        __GL_INPUT_ATT11_INDEX,
        __GL_INPUT_ATT12_INDEX,
        __GL_INPUT_ATT13_INDEX,
        __GL_INPUT_ATT14_INDEX,
        __GL_INPUT_ATT15_INDEX,
};

#define __GL_INPUT_VERTEX                 (__GL_ONE_64 << __GL_INPUT_VERTEX_INDEX)
#define __GL_INPUT_WEIGHT                 (__GL_ONE_64 << __GL_INPUT_WEIGHT_INDEX)
#define __GL_INPUT_NORMAL                 (__GL_ONE_64 << __GL_INPUT_NORMAL_INDEX)
#define __GL_INPUT_DIFFUSE                (__GL_ONE_64 << __GL_INPUT_DIFFUSE_INDEX)
#define __GL_INPUT_SPECULAR               (__GL_ONE_64 << __GL_INPUT_SPECULAR_INDEX)
#define __GL_INPUT_FOGCOORD               (__GL_ONE_64 << __GL_INPUT_FOGCOORD_INDEX)
#define __GL_INPUT_EDGEFLAG               (__GL_ONE_64 << __GL_INPUT_EDGEFLAG_INDEX)
#define __GL_INPUT_V7                     (__GL_ONE_64 << __GL_INPUT_V7_INDEX)
#define __GL_INPUT_TEX0                   (__GL_ONE_64 << __GL_INPUT_TEX0_INDEX)
#define __GL_INPUT_TEX1                   (__GL_ONE_64 << __GL_INPUT_TEX1_INDEX)
#define __GL_INPUT_TEX2                   (__GL_ONE_64 << __GL_INPUT_TEX2_INDEX)
#define __GL_INPUT_TEX3                   (__GL_ONE_64 << __GL_INPUT_TEX3_INDEX)
#define __GL_INPUT_TEX4                   (__GL_ONE_64 << __GL_INPUT_TEX4_INDEX)
#define __GL_INPUT_TEX5                   (__GL_ONE_64 << __GL_INPUT_TEX5_INDEX)
#define __GL_INPUT_TEX6                   (__GL_ONE_64 << __GL_INPUT_TEX6_INDEX)
#define __GL_INPUT_TEX7                   (__GL_ONE_64 << __GL_INPUT_TEX7_INDEX)
#define __GL_INPUT_ATT0                   (__GL_ONE_64 << __GL_INPUT_ATT0_INDEX)
#define __GL_INPUT_ATT1                   (__GL_ONE_64 << __GL_INPUT_ATT1_INDEX)
#define __GL_INPUT_ATT2                   (__GL_ONE_64 << __GL_INPUT_ATT2_INDEX)
#define __GL_INPUT_ATT3                   (__GL_ONE_64 << __GL_INPUT_ATT3_INDEX)
#define __GL_INPUT_ATT4                   (__GL_ONE_64 << __GL_INPUT_ATT4_INDEX)
#define __GL_INPUT_ATT5                   (__GL_ONE_64 << __GL_INPUT_ATT5_INDEX)
#define __GL_INPUT_ATT6                   (__GL_ONE_64 << __GL_INPUT_ATT6_INDEX)
#define __GL_INPUT_ATT7                   (__GL_ONE_64 << __GL_INPUT_ATT7_INDEX)
#define __GL_INPUT_ATT8                   (__GL_ONE_64 << __GL_INPUT_ATT8_INDEX)
#define __GL_INPUT_ATT9                   (__GL_ONE_64 << __GL_INPUT_ATT9_INDEX)
#define __GL_INPUT_ATT10                  (__GL_ONE_64 << __GL_INPUT_ATT10_INDEX)
#define __GL_INPUT_ATT11                  (__GL_ONE_64 << __GL_INPUT_ATT11_INDEX)
#define __GL_INPUT_ATT12                  (__GL_ONE_64 << __GL_INPUT_ATT12_INDEX)
#define __GL_INPUT_ATT13                  (__GL_ONE_64 << __GL_INPUT_ATT13_INDEX)
#define __GL_INPUT_ATT14                  (__GL_ONE_64 << __GL_INPUT_ATT14_INDEX)
#define __GL_INPUT_ATT15                  (__GL_ONE_64 << __GL_INPUT_ATT15_INDEX)

#define __GL_MISSING_ATT_MASK             (__GL_INPUT_NORMAL   | \
                                           __GL_INPUT_DIFFUSE  | \
                                           __GL_INPUT_SPECULAR | \
                                           __GL_INPUT_TEX0     | \
                                           __GL_INPUT_TEX1     | \
                                           __GL_INPUT_TEX2     | \
                                           __GL_INPUT_TEX3)

#define __GL_DEFERED_ATTRIB_BIT            0x1
#define __GL_DEFERED_NORMAL_BIT            __GL_INPUT_NORMAL
#define __GL_DEFERED_COLOR_BIT             __GL_INPUT_DIFFUSE
#define __GL_DEFERED_NORCOL_MASK           (__GL_DEFERED_NORMAL_BIT|__GL_DEFERED_COLOR_BIT)
#define __GL_DEFERED_COLOR_MASK_BIT        (1 << 4)


#define __GL_VARRAY_VERTEX_INDEX            __GL_INPUT_VERTEX_INDEX
#define __GL_VARRAY_WEIGHT_INDEX            __GL_INPUT_WEIGHT_INDEX
#define __GL_VARRAY_NORMAL_INDEX            __GL_INPUT_NORMAL_INDEX
#define __GL_VARRAY_DIFFUSE_INDEX           __GL_INPUT_DIFFUSE_INDEX
#define __GL_VARRAY_SPECULAR_INDEX          __GL_INPUT_SPECULAR_INDEX
#define __GL_VARRAY_FOGCOORD_INDEX          __GL_INPUT_FOGCOORD_INDEX
#define __GL_VARRAY_EDGEFLAG_INDEX          __GL_INPUT_EDGEFLAG_INDEX
#define __GL_VARRAY_COLORINDEX_INDEX        __GL_INPUT_V7_INDEX
#define __GL_VARRAY_TEX0_INDEX              __GL_INPUT_TEX0_INDEX
#define __GL_VARRAY_TEX1_INDEX              __GL_INPUT_TEX1_INDEX
#define __GL_VARRAY_TEX2_INDEX              __GL_INPUT_TEX2_INDEX
#define __GL_VARRAY_TEX3_INDEX              __GL_INPUT_TEX3_INDEX
#define __GL_VARRAY_TEX4_INDEX              __GL_INPUT_TEX4_INDEX
#define __GL_VARRAY_TEX5_INDEX              __GL_INPUT_TEX5_INDEX
#define __GL_VARRAY_TEX6_INDEX              __GL_INPUT_TEX6_INDEX
#define __GL_VARRAY_TEX7_INDEX              __GL_INPUT_TEX7_INDEX
#define __GL_VARRAY_ATT0_INDEX              __GL_INPUT_ATT0_INDEX
#define __GL_VARRAY_ATT1_INDEX              __GL_INPUT_ATT1_INDEX
#define __GL_VARRAY_ATT2_INDEX              __GL_INPUT_ATT2_INDEX
#define __GL_VARRAY_ATT3_INDEX              __GL_INPUT_ATT3_INDEX
#define __GL_VARRAY_ATT4_INDEX              __GL_INPUT_ATT4_INDEX
#define __GL_VARRAY_ATT5_INDEX              __GL_INPUT_ATT5_INDEX
#define __GL_VARRAY_ATT6_INDEX              __GL_INPUT_ATT6_INDEX
#define __GL_VARRAY_ATT7_INDEX              __GL_INPUT_ATT7_INDEX
#define __GL_VARRAY_ATT8_INDEX              __GL_INPUT_ATT8_INDEX
#define __GL_VARRAY_ATT9_INDEX              __GL_INPUT_ATT9_INDEX
#define __GL_VARRAY_ATT10_INDEX             __GL_INPUT_ATT10_INDEX
#define __GL_VARRAY_ATT11_INDEX             __GL_INPUT_ATT11_INDEX
#define __GL_VARRAY_ATT12_INDEX             __GL_INPUT_ATT12_INDEX
#define __GL_VARRAY_ATT13_INDEX             __GL_INPUT_ATT13_INDEX
#define __GL_VARRAY_ATT14_INDEX             __GL_INPUT_ATT14_INDEX
#define __GL_VARRAY_ATT15_INDEX             __GL_INPUT_ATT15_INDEX

#define __GL_VARRAY_VERTEX                 (__GL_ONE_32 << __GL_VARRAY_VERTEX_INDEX)
#define __GL_VARRAY_WEIGHT                 (__GL_ONE_32 << __GL_VARRAY_WEIGHT_INDEX)
#define __GL_VARRAY_NORMAL                 (__GL_ONE_32 << __GL_VARRAY_NORMAL_INDEX)
#define __GL_VARRAY_DIFFUSE                (__GL_ONE_32 << __GL_VARRAY_DIFFUSE_INDEX)
#define __GL_VARRAY_SPECULAR               (__GL_ONE_32 << __GL_VARRAY_SPECULAR_INDEX)
#define __GL_VARRAY_FOGCOORD               (__GL_ONE_32 << __GL_VARRAY_FOGCOORD_INDEX)
#define __GL_VARRAY_EDGEFLAG               (__GL_ONE_32 << __GL_VARRAY_EDGEFLAG_INDEX)
#define __GL_VARRAY_COLORINDEX             (__GL_ONE_32 << __GL_VARRAY_COLORINDEX_INDEX)
#define __GL_VARRAY_TEX0                   (__GL_ONE_32 << __GL_VARRAY_TEX0_INDEX)
#define __GL_VARRAY_TEX1                   (__GL_ONE_32 << __GL_VARRAY_TEX1_INDEX)
#define __GL_VARRAY_TEX2                   (__GL_ONE_32 << __GL_VARRAY_TEX2_INDEX)
#define __GL_VARRAY_TEX3                   (__GL_ONE_32 << __GL_VARRAY_TEX3_INDEX)
#define __GL_VARRAY_TEX4                   (__GL_ONE_32 << __GL_VARRAY_TEX4_INDEX)
#define __GL_VARRAY_TEX5                   (__GL_ONE_32 << __GL_VARRAY_TEX5_INDEX)
#define __GL_VARRAY_TEX6                   (__GL_ONE_32 << __GL_VARRAY_TEX6_INDEX)
#define __GL_VARRAY_TEX7                   (__GL_ONE_32 << __GL_VARRAY_TEX7_INDEX)
#define __GL_VARRAY_ATT0                   (__GL_ONE_32 << __GL_VARRAY_ATT0_INDEX)
#define __GL_VARRAY_ATT1                   (__GL_ONE_32 << __GL_VARRAY_ATT1_INDEX)
#define __GL_VARRAY_ATT2                   (__GL_ONE_32 << __GL_VARRAY_ATT2_INDEX)
#define __GL_VARRAY_ATT3                   (__GL_ONE_32 << __GL_VARRAY_ATT3_INDEX)
#define __GL_VARRAY_ATT4                   (__GL_ONE_32 << __GL_VARRAY_ATT4_INDEX)
#define __GL_VARRAY_ATT5                   (__GL_ONE_32 << __GL_VARRAY_ATT5_INDEX)
#define __GL_VARRAY_ATT6                   (__GL_ONE_32 << __GL_VARRAY_ATT6_INDEX)
#define __GL_VARRAY_ATT7                   (__GL_ONE_32 << __GL_VARRAY_ATT7_INDEX)
#define __GL_VARRAY_ATT8                   (__GL_ONE_32 << __GL_VARRAY_ATT8_INDEX)
#define __GL_VARRAY_ATT9                   (__GL_ONE_32 << __GL_VARRAY_ATT9_INDEX)
#define __GL_VARRAY_ATT10                  (__GL_ONE_32 << __GL_VARRAY_ATT10_INDEX)
#define __GL_VARRAY_ATT11                  (__GL_ONE_32 << __GL_VARRAY_ATT11_INDEX)
#define __GL_VARRAY_ATT12                  (__GL_ONE_32 << __GL_VARRAY_ATT12_INDEX)
#define __GL_VARRAY_ATT13                  (__GL_ONE_32 << __GL_VARRAY_ATT13_INDEX)
#define __GL_VARRAY_ATT14                  (__GL_ONE_32 << __GL_VARRAY_ATT14_INDEX)
#define __GL_VARRAY_ATT15                  (__GL_ONE_32 << __GL_VARRAY_ATT15_INDEX)

#define __GL_VARRAY_TEXTURES               (__GL_VARRAY_TEX0 | \
                                            __GL_VARRAY_TEX1 | \
                                            __GL_VARRAY_TEX2 | \
                                            __GL_VARRAY_TEX3 | \
                                            __GL_VARRAY_TEX4 | \
                                            __GL_VARRAY_TEX5 | \
                                            __GL_VARRAY_TEX6 | \
                                            __GL_VARRAY_TEX7)

#define __GL_VARRAY_ATTRIBUTES             (__GL_VARRAY_ATT1 | \
                                            __GL_VARRAY_ATT2 | \
                                            __GL_VARRAY_ATT3 | \
                                            __GL_VARRAY_ATT4 | \
                                            __GL_VARRAY_ATT5 | \
                                            __GL_VARRAY_ATT6 | \
                                            __GL_VARRAY_ATT7 | \
                                            __GL_VARRAY_ATT8 | \
                                            __GL_VARRAY_ATT9 | \
                                            __GL_VARRAY_ATT10 | \
                                            __GL_VARRAY_ATT11 | \
                                            __GL_VARRAY_ATT12 | \
                                            __GL_VARRAY_ATT13 | \
                                            __GL_VARRAY_ATT14 | \
                                            __GL_VARRAY_ATT15)


extern GLsizei minVertexNumber[];

#define CHECK_VERTEX_COUNT()                \
    if (count < minVertexNumber[mode] ) {   \
        return;                             \
    }                                       \
    switch (mode ) {                        \
        case GL_TRIANGLES:                  \
            count = count/3 * 3;            \
            break;                          \
        case GL_LINES:                      \
            count = count/2 * 2;            \
            break;                          \
        case GL_QUADS:                      \
            count = count/4 * 4;            \
            break;                          \
        case GL_QUAD_STRIP:                 \
            count = count/2 * 2;            \
            break;                          \
    }

#define CHECK_INSTANCE_COUNT()              \
    if (primCount < 1 ) {                   \
        return;                             \
    }
#define __GL_DIRTY_VARRAY_ENABLE_BIT        (1 << __GL_DIRTY_VARRAY_ENABLE)
#define __GL_DIRTY_VARRAY_FORMAT_BIT        (1 << __GL_DIRTY_VARRAY_FORMAT)
#define __GL_DIRTY_VARRAY_BINDING_BIT       (1 << __GL_DIRTY_VARRAY_BINDING)
#define __GL_DIRTY_VARRAY_OFFSET_BIT        (1 << __GL_DIRTY_VARRAY_OFFSET)


enum {
    __GL_DRAWARRAYS_NEW_BEGIN   = 0,
    __GL_DRAWARRAYS_CONT_BEGIN  = 1,
};

GLvoid __glArrayElement_V2F(__GLcontext *, GLuint, GLfloat **);
GLvoid __glArrayElement_V3F(__GLcontext *, GLuint, GLfloat **);
GLvoid __glArrayElement_C4UB_V2F(__GLcontext *, GLuint, GLfloat **);
GLvoid __glArrayElement_C4UB_V3F(__GLcontext *, GLuint, GLfloat **);
GLvoid __glArrayElement_C3F_V3F(__GLcontext *, GLuint, GLfloat **);
GLvoid __glArrayElement_N3F_V3F(__GLcontext *, GLuint, GLfloat **);
GLvoid __glArrayElement_C4F_N3F_V3F(__GLcontext *, GLuint, GLfloat **);
GLvoid __glArrayElement_T2F_V3F(__GLcontext *, GLuint, GLfloat **);
GLvoid __glArrayElement_T4F_V4F(__GLcontext *, GLuint, GLfloat **);
GLvoid __glArrayElement_T2F_C4UB_V3F(__GLcontext *, GLuint, GLfloat **);
GLvoid __glArrayElement_T2F_C3F_V3F(__GLcontext *, GLuint, GLfloat **);
GLvoid __glArrayElement_T2F_N3F_V3F(__GLcontext *, GLuint, GLfloat **);
GLvoid __glArrayElement_T2F_C4F_N3F_V3F(__GLcontext *, GLuint, GLfloat **);
GLvoid __glArrayElement_T4F_C4F_N3F_V4F(__GLcontext *, GLuint, GLfloat **);
GLenum __glArrayElement_Generic(__GLcontext *, GLuint, GLfloat **, GLubyte **, GLuint *);
#endif

/****************************************************************************/
/*
** Vertex array state.
*/

typedef struct __GLvertexAttribRec
{
    GLint             size;
    GLenum            type;
    GLsizei           usr_stride;   /* User defined stride which could be 0 */
#ifdef OPENGL40
    GLsizei           stride;         /*actual stride, wich could not be 0*/
#endif
    GLboolean         normalized;
    GLboolean         integer;      /* unconverted integers */
    GLuint            relativeOffset;

#ifdef OPENGL40
    union
    {
        const GLvoid * pointer;
        GLintptr offset;
    };
#else
    const GLvoid     *pointer;
#endif

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
#ifdef OPENGL40
    union
    {
        struct
        {
            __GLvertexAttrib vertex;
            __GLvertexAttrib weight;
            __GLvertexAttrib normal;
            __GLvertexAttrib color;
            __GLvertexAttrib color2;
            __GLvertexAttrib fogcoord;
            __GLvertexAttrib edgeflag;
            __GLvertexAttrib colorindex;
            __GLvertexAttrib texture[__GL_MAX_TEXTURE_COORDS];
            __GLvertexAttrib usrattribute[__GL_MAX_VERTEX_ATTRIBUTES];
        };
#endif
        __GLvertexAttrib attribute[__GL_MAX_VERTEX_ATTRIBUTES + 16];
#ifdef OPENGL40
    };
#endif
    __GLvertexAttribBinding attributeBinding[__GL_MAX_VERTEX_ATTRIBUTE_BINDINGS + 16];

    GLuint64            attribEnabled;  /* bitvector of the enabled attributes */

    GLuint            boundIdxName;
    __GLbufferObject *boundIdxObj;

#ifdef OPENGL40
        /* This is the user-controlled GL_ARRAY_BUFFER binding point which is set by
    ** glBindBuffer(GL_ARRAY_BUFFER, buffer). The same buffer binding is also stored
    ** in gc->bufferObject.boundBuffer[__GL_ARRAY_BUFFER_INDEX].
    */
    GLuint arrayBufBinding;

    /* This is the user-controlled GL_ELEMENT_ARRAY_BUFFER binding point which is
    ** set by glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer). The same buffer binding
    ** is also stored in gc->bufferObject.boundBuffer[__GL_ELEMENT_ARRAY_BUFFER_INDEX].
    */
    GLuint elementBufBinding;

//    GLuint arrayEnabled;    /*bitvector of the enabled arrays*/
//    GLuint currentEnabled;  /*bitvector of the really needed enabled arrays*/
    GLuint arrayInBufObj;   /*bitvector of the arrays that are in buffer objects */

    GLuint clientActiveUnit;

#if GL_ATI_element_array
    GLboolean elementArrayATI;
    const GLvoid *elementPointer;
    GLenum elementType;
    GLuint elementArrayBindingATI;
#endif
#endif

} __GLvertexArrayState;

/*
** Referenced by "gc->vertexArray.varrayDirty"
 */
enum
{
    __GL_DIRTY_VARRAY_MODE = 0,
    __GL_DIRTY_VARRAY_ENABLE,
    __GL_DIRTY_VARRAY_FORMAT,
    __GL_DIRTY_VARRAY_BINDING,
    __GL_DIRTY_VARRAY_OFFSET,
    __GL_DIRTY_VARRAY_DIVISOR,

    __GL_DIRTY_VARRAY_END,
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

#ifdef OPENGL40
    GLboolean formatChanged;
    GLboolean interleaved;
    GLbitfield               globalDirtyBackup;
    GLboolean                fastStreamSetup;


    GLvoid (APIENTRY *drawArraysFunc)(__GLcontext *, GLenum mode, GLint first, GLsizei count);
    GLvoid (APIENTRY *drawElementsFunc)(__GLcontext *, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
    GLvoid (APIENTRY *arrayElementFunc)(__GLcontext *, GLint element);
    GLvoid (*optdlArrayElement)(__GLcontext *, GLuint, GLfloat **);
#endif

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

#ifdef OPENGL40
        case GL_DOUBLE:

            return (sizeof(GLdouble) * size);
#endif

        default:

            GL_ASSERT(0);/*error check should be done in the caller*/

    }



    return 0;

}

__GLbufferObject* __glGetCurrentVertexArrayBufObj(__GLcontext *gc, GLuint binding);

#endif /* __gc_es_vertex_h__ */
