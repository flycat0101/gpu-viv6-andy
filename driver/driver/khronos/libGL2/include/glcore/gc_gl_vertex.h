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


#ifndef __gc_gl_vertex_h_
#define __gc_gl_vertex_h_

#define ENTERFUNC(name)
#define LEAVEFUNC(name)

#define __GL_SETUP_NOT_IN_BEGIN()                           \
    __GL_SETUP();                                           \
    if (gc->input.beginMode == __GL_IN_BEGIN) {             \
        __glSetError(GL_INVALID_OPERATION);                 \
        return;                                             \
    }                                                       \

#define __GL_SETUP_NOT_IN_BEGIN_RET(val)                    \
    __GL_SETUP();                                           \
    if (gc->input.beginMode == __GL_IN_BEGIN) {             \
        __glSetError(GL_INVALID_OPERATION);                 \
        return (val);                                       \
    }                                                       \

#define __GL_VERTEX_BUFFER_FLUSH(gc)                        \
    switch (gc->input.beginMode) {                          \
      case __GL_SMALL_LIST_BATCH:                           \
        __glDisplayListBatchEnd(gc);                        \
        break;                                              \
        default: ;                        \
    }                                                       \

#define __GL_DLIST_BUFFER_FLUSH(gc)                         \
    if (gc->input.beginMode == __GL_SMALL_LIST_BATCH) {     \
        __glDisplayListBatchEnd(gc);                        \
    }                                                       \


#define __GL_PACK_COLOR4UB(r, g, b, a)  (((a) << 24) | ((b) << 16) | ((g) << 8) | (r))


#define __GL_MAX_VERTEX_BUFFER_DW_OFFSET     65400
#define __GL_MAX_VERTEX_NUMBER                8190

/*
** This constant defines the size of system memory default vertex buffer (in byte)
** which is used in immediate mode and vertex arrays.
**
** We define the default buffer size as 1 MB which is enough for __GL_MAX_VERTEX_NUMBER
** vertices with 7 attributes per vertex.
*/
#define __GL_DEFAULT_VERTEX_BUFFER_SIZE (__GL_MAX_VERTEX_NUMBER * 8 * sizeof(__GLvertex4))
#define __GL_DEFAULT_BUFFER_END_ZONE  (__GL_TOTAL_VERTEX_ATTRIBUTES * sizeof(__GLvertex4))


#define __GL_BEGIN_END_TAG_MASK         0x10
#define __GL_DRAWARRAYS_TAG_MASK        0x100
#define __GL_VERTEX_DATA_TAG_MASK       0x400

#define __GL_PRIM_ELEMENT_MASK          0xff
#define __GL_PRIM_ELEMENT_SHIFT         6
#define __GL_MAX_PRIM_ELEMENT_NUMBER    10  /* primElemSequence (64 bit)/__GL_PRIM_ELEMENT_SHIFT */
#define __GL_PRIM_ELEMENT(primElemSequence, tag) \
    primElemSequence = ((primElemSequence) << __GL_PRIM_ELEMENT_SHIFT) | ((tag) & __GL_PRIM_ELEMENT_MASK)

/* Tags for API entry between glBegin/glEnd.
*/
enum {
        __GL_V2F_TAG = __GL_VERTEX_DATA_TAG_MASK,
        __GL_V3F_TAG,
        __GL_V4F_TAG,
        __GL_C3F_TAG,
        __GL_C4F_TAG,
        __GL_C4UB_TAG,
        __GL_N3F_TAG,
        __GL_TC2F_TAG,
        __GL_TC2F_U1_TAG,
        __GL_TC2F_U2_TAG,
        __GL_TC2F_U3_TAG,
        __GL_TC2F_U4_TAG,
        __GL_TC2F_U5_TAG,
        __GL_TC2F_U6_TAG,
        __GL_TC2F_U7_TAG,
        __GL_TC3F_TAG,
        __GL_TC3F_U1_TAG,
        __GL_TC3F_U2_TAG,
        __GL_TC3F_U3_TAG,
        __GL_TC3F_U4_TAG,
        __GL_TC3F_U5_TAG,
        __GL_TC3F_U6_TAG,
        __GL_TC3F_U7_TAG,
        __GL_TC4F_TAG,
        __GL_TC4F_U1_TAG,
        __GL_TC4F_U2_TAG,
        __GL_TC4F_U3_TAG,
        __GL_TC4F_U4_TAG,
        __GL_TC4F_U5_TAG,
        __GL_TC4F_U6_TAG,
        __GL_TC4F_U7_TAG,
        __GL_EDGEFLAG_TAG,
        __GL_SC3F_TAG,
        __GL_FOG1F_TAG,
        __GL_AT4F_I0_TAG,
        __GL_AT4F_I1_TAG,
        __GL_AT4F_I2_TAG,
        __GL_AT4F_I3_TAG,
        __GL_AT4F_I4_TAG,
        __GL_AT4F_I5_TAG,
        __GL_AT4F_I6_TAG,
        __GL_AT4F_I7_TAG,
        __GL_AT4F_I8_TAG,
        __GL_AT4F_I9_TAG,
        __GL_AT4F_I10_TAG,
        __GL_AT4F_I11_TAG,
        __GL_AT4F_I12_TAG,
        __GL_AT4F_I13_TAG,
        __GL_AT4F_I14_TAG,
        __GL_AT4F_I15_TAG,
        __GL_N3F_V3F_TAG,
        __GL_N3F_V4F_TAG,
        __GL_C4F_V3F_TAG
};

enum {
        __GL_V2F_INDEX = 0,
        __GL_V3F_INDEX,
        __GL_V4F_INDEX,
        __GL_C3F_INDEX,
        __GL_C4F_INDEX,
        __GL_C4UB_INDEX,
        __GL_N3F_INDEX,
        __GL_TC2F_INDEX,
        __GL_TC2F_U1_INDEX,
        __GL_TC2F_U2_INDEX,
        __GL_TC2F_U3_INDEX,
        __GL_TC2F_U4_INDEX,
        __GL_TC2F_U5_INDEX,
        __GL_TC2F_U6_INDEX,
        __GL_TC2F_U7_INDEX,
        __GL_TC3F_INDEX,
        __GL_TC3F_U1_INDEX,
        __GL_TC3F_U2_INDEX,
        __GL_TC3F_U3_INDEX,
        __GL_TC3F_U4_INDEX,
        __GL_TC3F_U5_INDEX,
        __GL_TC3F_U6_INDEX,
        __GL_TC3F_U7_INDEX,
        __GL_TC4F_INDEX,
        __GL_TC4F_U1_INDEX,
        __GL_TC4F_U2_INDEX,
        __GL_TC4F_U3_INDEX,
        __GL_TC4F_U4_INDEX,
        __GL_TC4F_U5_INDEX,
        __GL_TC4F_U6_INDEX,
        __GL_TC4F_U7_INDEX,
        __GL_EDGEFLAG_INDEX,
        __GL_SC3F_INDEX,
        __GL_FOG1F_INDEX,
        __GL_AT4F_I0_INDEX,
        __GL_AT4F_I1_INDEX,
        __GL_AT4F_I2_INDEX,
        __GL_AT4F_I3_INDEX,
        __GL_AT4F_I4_INDEX,
        __GL_AT4F_I5_INDEX,
        __GL_AT4F_I6_INDEX,
        __GL_AT4F_I7_INDEX,
        __GL_AT4F_I8_INDEX,
        __GL_AT4F_I9_INDEX,
        __GL_AT4F_I10_INDEX,
        __GL_AT4F_I11_INDEX,
        __GL_AT4F_I12_INDEX,
        __GL_AT4F_I13_INDEX,
        __GL_AT4F_I14_INDEX,
        __GL_AT4F_I15_INDEX,
        __GL_WEIGHT_INDEX,
        __GL_COLORINDEX_INDEX,
        __GL_PSIZE_INDEX
};

#define __GL_V2F_BIT            (__GL_ONE_64<<(__GL_V2F_INDEX))
#define __GL_V3F_BIT            (__GL_ONE_64<<(__GL_V3F_INDEX))
#define __GL_V4F_BIT            (__GL_ONE_64<<(__GL_V4F_INDEX))
#define __GL_C3F_BIT            (__GL_ONE_64<<(__GL_C3F_INDEX))
#define __GL_C4F_BIT            (__GL_ONE_64<<(__GL_C4F_INDEX))
#define __GL_C4UB_BIT            (__GL_ONE_64<<(__GL_C4UB_INDEX))
#define __GL_N3F_BIT            (__GL_ONE_64<<(__GL_N3F_INDEX))
#define __GL_TC2F_BIT            (__GL_ONE_64<<(__GL_TC2F_INDEX))
#define __GL_TC3F_BIT            (__GL_ONE_64<<(__GL_TC3F_INDEX))
#define __GL_TC4F_BIT            (__GL_ONE_64<<(__GL_TC4F_INDEX))
#define __GL_TC2F_U1_BIT        (__GL_ONE_64<<(__GL_TC2F_U1_INDEX))
#define __GL_TC3F_U1_BIT        (__GL_ONE_64<<(__GL_TC3F_U1_INDEX))
#define __GL_TC4F_U1_BIT        (__GL_ONE_64<<(__GL_TC4F_U1_INDEX))
#define __GL_TC2F_U2_BIT        (__GL_ONE_64<<(__GL_TC2F_U2_INDEX))
#define __GL_TC3F_U2_BIT        (__GL_ONE_64<<(__GL_TC3F_U2_INDEX))
#define __GL_TC4F_U2_BIT        (__GL_ONE_64<<(__GL_TC4F_U2_INDEX))
#define __GL_TC2F_U3_BIT        (__GL_ONE_64<<(__GL_TC2F_U3_INDEX))
#define __GL_TC3F_U3_BIT        (__GL_ONE_64<<(__GL_TC3F_U3_INDEX))
#define __GL_TC4F_U3_BIT        (__GL_ONE_64<<(__GL_TC4F_U3_INDEX))
#define __GL_TC2F_U4_BIT        (__GL_ONE_64<<(__GL_TC2F_U4_INDEX))
#define __GL_TC3F_U4_BIT        (__GL_ONE_64<<(__GL_TC3F_U4_INDEX))
#define __GL_TC4F_U4_BIT        (__GL_ONE_64<<(__GL_TC4F_U4_INDEX))
#define __GL_TC2F_U5_BIT        (__GL_ONE_64<<(__GL_TC2F_U5_INDEX))
#define __GL_TC3F_U5_BIT        (__GL_ONE_64<<(__GL_TC3F_U5_INDEX))
#define __GL_TC4F_U5_BIT        (__GL_ONE_64<<(__GL_TC4F_U5_INDEX))
#define __GL_TC2F_U6_BIT        (__GL_ONE_64<<(__GL_TC2F_U6_INDEX))
#define __GL_TC3F_U6_BIT        (__GL_ONE_64<<(__GL_TC3F_U6_INDEX))
#define __GL_TC4F_U6_BIT        (__GL_ONE_64<<(__GL_TC4F_U6_INDEX))
#define __GL_TC2F_U7_BIT        (__GL_ONE_64<<(__GL_TC2F_U7_INDEX))
#define __GL_TC3F_U7_BIT        (__GL_ONE_64<<(__GL_TC3F_U7_INDEX))
#define __GL_TC4F_U7_BIT        (__GL_ONE_64<<(__GL_TC4F_U7_INDEX))
#define __GL_EDGEFLAG_BIT        (__GL_ONE_64<<(__GL_EDGEFLAG_INDEX))
#define __GL_SC3F_BIT            (__GL_ONE_64<<(__GL_SC3F_INDEX))
#define __GL_FOG1F_BIT            (__GL_ONE_64<<(__GL_FOG1F_INDEX))
#define __GL_AT4F_I0_BIT        (__GL_ONE_64<<(__GL_AT4F_I0_INDEX))
#define __GL_AT4F_I1_BIT        (__GL_ONE_64<<(__GL_AT4F_I1_INDEX))
#define __GL_AT4F_I2_BIT        (__GL_ONE_64<<(__GL_AT4F_I2_INDEX))
#define __GL_AT4F_I3_BIT        (__GL_ONE_64<<(__GL_AT4F_I3_INDEX))
#define __GL_AT4F_I4_BIT        (__GL_ONE_64<<(__GL_AT4F_I4_INDEX))
#define __GL_AT4F_I5_BIT        (__GL_ONE_64<<(__GL_AT4F_I5_INDEX))
#define __GL_AT4F_I6_BIT        (__GL_ONE_64<<(__GL_AT4F_I6_INDEX))
#define __GL_AT4F_I7_BIT        (__GL_ONE_64<<(__GL_AT4F_I7_INDEX))
#define __GL_AT4F_I8_BIT        (__GL_ONE_64<<(__GL_AT4F_I8_INDEX))
#define __GL_AT4F_I9_BIT        (__GL_ONE_64<<(__GL_AT4F_I9_INDEX))
#define __GL_AT4F_I10_BIT        (__GL_ONE_64<<(__GL_AT4F_I10_INDEX))
#define __GL_AT4F_I11_BIT        (__GL_ONE_64<<(__GL_AT4F_I11_INDEX))
#define __GL_AT4F_I12_BIT        (__GL_ONE_64<<(__GL_AT4F_I12_INDEX))
#define __GL_AT4F_I13_BIT        (__GL_ONE_64<<(__GL_AT4F_I13_INDEX))
#define __GL_AT4F_I14_BIT        (__GL_ONE_64<<(__GL_AT4F_I14_INDEX))
#define __GL_AT4F_I15_BIT        (__GL_ONE_64<<(__GL_AT4F_I15_INDEX))
#define __GL_WEIGHT_BIT            (__GL_ONE_64<<(__GL_WEIGHT_INDEX))
#define __GL_COLORINDEX_BIT        (__GL_ONE_64<<(__GL_COLORINDEX_INDEX))
#define __GL_PSIZE_BIT            (__GL_ONE_64<<(__GL_PSIZE_INDEX))

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

#define __GL_INPUT_VERTEX                 (__GL_ONE_32 << __GL_INPUT_VERTEX_INDEX)
#define __GL_INPUT_WEIGHT                 (__GL_ONE_32 << __GL_INPUT_WEIGHT_INDEX)
#define __GL_INPUT_NORMAL                 (__GL_ONE_32 << __GL_INPUT_NORMAL_INDEX)
#define __GL_INPUT_DIFFUSE                (__GL_ONE_32 << __GL_INPUT_DIFFUSE_INDEX)
#define __GL_INPUT_SPECULAR               (__GL_ONE_32 << __GL_INPUT_SPECULAR_INDEX)
#define __GL_INPUT_FOGCOORD               (__GL_ONE_32 << __GL_INPUT_FOGCOORD_INDEX)
#define __GL_INPUT_EDGEFLAG               (__GL_ONE_32 << __GL_INPUT_EDGEFLAG_INDEX)
#define __GL_INPUT_V7                     (__GL_ONE_32 << __GL_INPUT_V7_INDEX)
#define __GL_INPUT_TEX0                   (__GL_ONE_32 << __GL_INPUT_TEX0_INDEX)
#define __GL_INPUT_TEX1                   (__GL_ONE_32 << __GL_INPUT_TEX1_INDEX)
#define __GL_INPUT_TEX2                   (__GL_ONE_32 << __GL_INPUT_TEX2_INDEX)
#define __GL_INPUT_TEX3                   (__GL_ONE_32 << __GL_INPUT_TEX3_INDEX)
#define __GL_INPUT_TEX4                   (__GL_ONE_32 << __GL_INPUT_TEX4_INDEX)
#define __GL_INPUT_TEX5                   (__GL_ONE_32 << __GL_INPUT_TEX5_INDEX)
#define __GL_INPUT_TEX6                   (__GL_ONE_32 << __GL_INPUT_TEX6_INDEX)
#define __GL_INPUT_TEX7                   (__GL_ONE_32 << __GL_INPUT_TEX7_INDEX)
#define __GL_INPUT_ATT0                   (__GL_ONE_32 << __GL_INPUT_ATT0_INDEX)
#define __GL_INPUT_ATT1                   (__GL_ONE_32 << __GL_INPUT_ATT1_INDEX)
#define __GL_INPUT_ATT2                   (__GL_ONE_32 << __GL_INPUT_ATT2_INDEX)
#define __GL_INPUT_ATT3                   (__GL_ONE_32 << __GL_INPUT_ATT3_INDEX)
#define __GL_INPUT_ATT4                   (__GL_ONE_32 << __GL_INPUT_ATT4_INDEX)
#define __GL_INPUT_ATT5                   (__GL_ONE_32 << __GL_INPUT_ATT5_INDEX)
#define __GL_INPUT_ATT6                   (__GL_ONE_32 << __GL_INPUT_ATT6_INDEX)
#define __GL_INPUT_ATT7                   (__GL_ONE_32 << __GL_INPUT_ATT7_INDEX)
#define __GL_INPUT_ATT8                   (__GL_ONE_32 << __GL_INPUT_ATT8_INDEX)
#define __GL_INPUT_ATT9                   (__GL_ONE_32 << __GL_INPUT_ATT9_INDEX)
#define __GL_INPUT_ATT10                  (__GL_ONE_32 << __GL_INPUT_ATT10_INDEX)
#define __GL_INPUT_ATT11                  (__GL_ONE_32 << __GL_INPUT_ATT11_INDEX)
#define __GL_INPUT_ATT12                  (__GL_ONE_32 << __GL_INPUT_ATT12_INDEX)
#define __GL_INPUT_ATT13                  (__GL_ONE_32 << __GL_INPUT_ATT13_INDEX)
#define __GL_INPUT_ATT14                  (__GL_ONE_32 << __GL_INPUT_ATT14_INDEX)
#define __GL_INPUT_ATT15                  (__GL_ONE_32 << __GL_INPUT_ATT15_INDEX)

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


/*
** Current glBegin mode.
*/
typedef enum __GLbeginModeEnum {
    __GL_NOT_IN_BEGIN        = 0,
    __GL_IN_BEGIN            = 1,
    __GL_SMALL_LIST_BATCH    = 2,
} __GLbeginMode;

typedef struct __GLvertexInputRec {
    GLubyte *pointer;
    GLfloat *currentPtrDW;
    GLuint offsetDW;
    GLuint index;
    GLuint sizeDW;
} __GLvertexInput;


/*
** State for managing the vertex machinery.
*/
typedef struct __GLvertexMachineRec {
    GLuint indexCount;
    GLuint lastVertexIndex;
    GLuint currentInputMask;
    GLuint inputMaskChanged;
    GLuint requiredInputMask;
    GLuint primInputMask;
    GLuint numberOfElements;
    __GLbeginMode beginMode;
    GLuint64 primElemSequence;
    GLuint64 primitiveFormat;
    GLuint64 preVertexFormat;
    GLuint64 vertexFormat;
    GLushort deferredAttribDirty;

    GLboolean inconsistentFormat;
    GLboolean indexPrimEnabled;

    GLuint connectVertexIndex[4];

    /* Primitive type for a primitive batch (The first primitive's primType) */
    GLenum primMode;

    /* Primitive type for the current primitive in the primitive batch */
    GLenum currentPrimMode;

    GLfloat *vertexDataBuffer;
    GLfloat *defaultDataBuffer;
    GLfloat *defaultDataBufEnd;
    GLfloat *currentDataBufPtr;
    GLfloat *primBeginAddr;
    GLushort *defaultIndexBuffer;
    GLushort *indexBuffer;

    GLint vertTotalStrideDW;

    union
    {
        struct
        {
            __GLvertexInput vertex;
            __GLvertexInput weight;
            __GLvertexInput normal;
            __GLvertexInput color;
            __GLvertexInput color2;
            __GLvertexInput fog;
            __GLvertexInput edgeflag;
            __GLvertexInput filler3;
            __GLvertexInput texture[__GL_MAX_TEXTURE_COORDS];
            __GLvertexInput attribute[__GL_MAX_PROGRAM_VERTEX_ATTRIBUTES];
        };
        __GLvertexInput currentInput[__GL_TOTAL_VERTEX_ATTRIBUTES];
    };

    __GLcurrentState shadowCurrent;

    /* instance number for drawArrayInstanceEXT or drawElementInstanceEXT */
   GLsizei primCount;

} __GLvertexMachine;

typedef struct __GLvertexElementRec {
    GLubyte streamIndex;        /* Stream index for the element */
    GLubyte inputIndex;         /* Semantic index */
    GLintptr offset;            /*
                                ** For immediate mode, Dlist and conventional vertex array,
                                ** this is the offset in the stream in bytes( the offset of the first
                                ** element in the stream must be 0); For buffer objects, this is
                                ** the offset in the buffer object( the offset of the first element in
                                ** the stream may not be 0).
                                */
    GLint size;                    /* Number of values in an element */
    GLenum type;                /* Type of the element data */
    GLboolean normalized;       /* Need HW normalization? */
    GLboolean integer;          /* integer attribute, GL_EXT_gpu_shader4*/
} __GLvertexElement;

typedef struct __GLstreamDeclRec {
    __GLvertexElement streamElement[__GL_MAX_VERTEX_ELEMENTS];
    GLuint numElements;
    GLuint stride;
    GLvoid *streamAddr;         /*
                                ** For immediate mode, Dlist and conventional vertex array,
                                ** this is the start address of the stream; For buffer objects, this is
                                ** the start address of the buffer object( in system cache).
                                */
    GLvoid **privPtrAddr;    /* NULL: no cached vertex data */
} __GLstreamDecl;

/*typedef struct __GLbufferObjectRec __GLbufferObject;*/
typedef struct __GLindexStreamDeclRec{
    GLenum type; /* Data type of index */
    GLvoid *streamAddr; /* Index stream address */
    GLvoid **ppIndexBufPriv;/* source of the index stream: __GLdpVertexBufferInfo **
                        ** if ppIndexBufObj NULL, from streamAddr;
                        ** if *ppIndexBufObj:  __GLdpVertexBufferInfo *, if NULL, index from cached indexbuffer, Not NULL, from buffer object(with offset) */
    GLintptr offset;/* if index is not from VBO, offset is zero, otherwise this is an offset sent down by app*/
}__GLindexStream;

typedef enum _GLSTREAMMODE{
    IMMEDIATE_STREAMMODE = 0,
    DLIST_STREAMMODE,
    VERTEXARRAY_STREAMMODE,
    LAST_STREAMMODE,
}GLSTREAMMODE;

typedef struct __GLvertexStreamMachineRec {
    GLenum primMode;
    GLuint numStreams;
    __GLstreamDecl streams[__GL_MAX_VERTEX_STREAMS];/*vertex streams*/
    __GLindexStream indexStream;/*index stream*/
    GLubyte *edgeflagStream;/*edgeflag stream*/
    GLuint64 primElemSequence;
    GLuint missingAttribs;/*missing flag*/
    GLuint primElementMask;/*bitmask for all primitive elements*/
    GLuint indexCount; /* 0: Draw Primitive; Not 0: Draw Index Primitive */
    /*the start (inclusive) and end (exclusive) vertex*/
    /*the actual vertex count to be used is always denoted by (endVertex-startVertex)*/
    GLuint startVertex;
    GLuint endVertex;
    GLSTREAMMODE streamMode;
} __GLvertexStreamMachine;

/* glmaterial (in_begin/end) emulation structure */
typedef struct __GLTnlAccumMachineRec{
    GLuint firstVertexIndex;                          /* Index of The first tnled vertex */
    GLuint vertexCount;                          /* Total vertex count tnled (start from firstVertexIndex(included))*/
    GLuint preVertexIndex;                          /* Index of The last tnled vertex */
    GLvoid (APIENTRY *glimEnd) (GLvoid);           /* To save current glEnd entry */
}__GLTnlAccumMachine;


extern GLuint edgeFlagInputMask[];

extern GLvoid __glDrawImmedPrimitive(__GLcontext *gc);
extern GLvoid __glImmediateFlushBuffer(__GLcontext *gc);
extern GLvoid __glConsistentFormatChange(__GLcontext *gc);
extern GLvoid __glSwitchToNewPrimtiveFormat(__GLcontext *gc, GLuint attFmtIdx);
extern GLvoid __glSwitchToInconsistentFormat(__GLcontext *gc);
extern GLvoid __glFillMissingAttributes(__GLcontext *gc);
extern GLvoid __glImmedUpdateVertexState(__GLcontext *);
extern GLvoid __glResetImmedVertexBuffer(__GLcontext *gc);
extern GLvoid __glComputeRequiredInputMask(__GLcontext *gc);
extern GLvoid __glComputeRequiredInputMaskInstancedEXT(__GLcontext *gc);

extern GLvoid __glImmedFlushPrim_Material(__GLcontext *gc, GLboolean bFlushPipe);
extern GLvoid APIENTRY __glim_End_Material (GLvoid);

#endif /* __gc_gl_vertex_h_ */
