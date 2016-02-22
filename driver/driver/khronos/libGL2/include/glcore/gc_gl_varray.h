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


#ifndef __gc_gl_varray_h_
#define __gc_gl_varray_h_

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
    if(count < minVertexNumber[mode] ) {    \
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
    if(primCount < 1 ) {                    \
        return;                             \
    }

/****************************************************************************/
/*
 * Vertex array state.
 */

typedef struct __GLvertexArrayRec
{
    GLint size;
    GLenum type;
    GLsizei usr_stride;     /*user defined stride, which could be 0*/
    GLsizei stride;         /*actual stride, wich could not be 0*/
    GLboolean normalized;   /*originally used for GL_ARB_vertex_program,
                              but we use this to decide how to convert fixed-point values*/
    GLboolean integer;      /*unconverted ints*/
    union
    {
        const GLvoid * pointer;
        GLintptr offset;
    };
    GLuint bufBinding;      /*for GL vertex buffer object*/

} __GLvertexArray;

typedef struct __GLvertexArrayStateRec
{
    union
    {
        struct
        {
            __GLvertexArray vertex;
            __GLvertexArray weight;
            __GLvertexArray normal;
            __GLvertexArray color;
            __GLvertexArray color2;
            __GLvertexArray fogcoord;
            __GLvertexArray edgeflag;
            __GLvertexArray colorindex;
            __GLvertexArray texture[__GL_MAX_TEXTURE_COORDS];
            __GLvertexArray attribute[__GL_MAX_PROGRAM_VERTEX_ATTRIBUTES];
        };
        __GLvertexArray currentArrays[__GL_TOTAL_VERTEX_ATTRIBUTES];
    };

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

    GLuint arrayEnabled;    /*bitvector of the enabled arrays*/
    GLuint currentEnabled;  /*bitvector of the really needed enabled arrays*/
    GLuint arrayInBufObj;   /*bitvector of the arrays that are in buffer objects */

    GLuint clientActiveUnit;

#if GL_ATI_element_array
    GLboolean elementArrayATI;
    const GLvoid *elementPointer;
    GLenum elementType;
    GLuint elementArrayBindingATI;
#endif

} __GLvertexArrayState;

/* Referenced by "gc->vertexArray.globalDirty"
 */
enum {
    __GL_DIRTY_VARRAY_ENABLE        = 0,
    __GL_DIRTY_VARRAY_FORMAT        = 1,
    __GL_DIRTY_VARRAY_BINDING       = 2,
    __GL_DIRTY_VARRAY_OFFSET        = 3,
    __GL_DIRTY_VARRAY_STOP_CACHE    = 4,
    __GL_DIRTY_VARRAY_END           = 5
};

#define __GL_DIRTY_VARRAY_ENABLE_BIT        (1 << __GL_DIRTY_VARRAY_ENABLE)
#define __GL_DIRTY_VARRAY_FORMAT_BIT        (1 << __GL_DIRTY_VARRAY_FORMAT)
#define __GL_DIRTY_VARRAY_BINDING_BIT       (1 << __GL_DIRTY_VARRAY_BINDING)
#define __GL_DIRTY_VARRAY_OFFSET_BIT        (1 << __GL_DIRTY_VARRAY_OFFSET)
#define __GL_DIRTY_VARRAY_STOP_CACHE_BIT    (1 << __GL_DIRTY_VARRAY_STOP_CACHE)

/* Macros that set vertex array dirty bits
*/
#define __GL_SET_VARRAY_ENABLE_BIT(gc) \
    (gc)->vertexArray.globalDirty |= __GL_DIRTY_VARRAY_ENABLE_BIT

#define __GL_SET_VARRAY_FORMAT_BIT(gc) \
    (gc)->vertexArray.globalDirty |= __GL_DIRTY_VARRAY_FORMAT_BIT

#define __GL_SET_VARRAY_BINDING_BIT(gc) \
    (gc)->vertexArray.globalDirty |= __GL_DIRTY_VARRAY_BINDING_BIT

#define __GL_SET_VARRAY_OFFSET_BIT(gc) \
    (gc)->vertexArray.globalDirty |= __GL_DIRTY_VARRAY_OFFSET_BIT

#define __GL_SET_VARRAY_STOP_CACHE_BIT(gc) \
    (gc)->vertexArray.globalDirty |= __GL_DIRTY_VARRAY_STOP_CACHE_BIT


enum {
    __GL_DRAWARRAYS_NEW_BEGIN   = 0,
    __GL_DRAWARRAYS_CONT_BEGIN  = 1,
    __GL_DRAWARRAYS_CACHE       = 2,
};


/*vertex array machine: internally used by driver to manage vertex array
*/
typedef struct {
    GLsizei indexCount;/*NULL: glDrawArrays(); Not NULL: glDrawElements()*/

    /*the start (inclusive) and end (exclusive) vertex*/
    /*the actual vertex count to be used is always denoted by (end-start)*/
    GLuint start;
    GLuint end;

    /*used for indexed draw cmd: glDrawElements()*/
    union
    {
        const GLvoid *indices;/*index pointer*/
        GLintptr indexOffset;/*offset in buffer object*/
    };

    GLenum indexType;

    /*
    ** Vertex array dirty bit, which will be cleared at the end of __glValidateDrawArrays
    */
    GLbitfield globalDirty;/* bit 0: enable dirty, bit 1: format dirty, bit 2: pointer dirty */
    GLbitfield globalDirtyBackup; /* for fast stream set up validation*/

    GLboolean immedFallback;/*fall back to immediate mode path*/

    GLboolean fastStreamSetup;

    /*this flag is a hint to hardware on how to configure the vertex array data in device memory*/
    GLboolean interleaved;

    GLboolean formatChanged;

    GLvoid (APIENTRY *drawArraysFunc)(GLenum mode, GLint first, GLsizei count);
    GLvoid (APIENTRY *drawElementsFunc)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
    GLvoid (APIENTRY *arrayElementFunc)(GLint element);
    GLvoid (*optdlArrayElement)(__GLcontext *, GLuint, GLfloat **);

    struct
    {
        GLboolean lockValid;
        GLint first;
        GLint count;
    } lockData;

} __GLvertexArrayMachine;


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

#endif /* __gc_gl_varray_h_ */
