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


#ifdef OPENGL40
#ifndef __gc_gl_vertex_h_
#define __gc_gl_vertex_h_

#define ENTERFUNC(name)
#define LEAVEFUNC(name)

#ifdef OPENGL40

#define __GL_SETUP_NOT_IN_BEGIN(gc)                         \
    if (gc->input.beginMode == __GL_IN_BEGIN) {             \
        __glSetError(gc, GL_INVALID_OPERATION);             \
        return;                                             \
    }                                                       \

#define __GL_SETUP_NOT_IN_BEGIN_RET(gc,val)                 \
    if (gc->input.beginMode == __GL_IN_BEGIN) {             \
        __glSetError(gc, GL_INVALID_OPERATION);             \
        return (val);                                       \
    }                                                       \

#define __GL_VERTEX_BUFFER_FLUSH(gc)                        \
    switch (gc->input.beginMode) {                          \
      case __GL_SMALL_DRAW_BATCH:                           \
        __glPrimitiveBatchEnd(gc);                          \
        break;                                              \
      case __GL_SMALL_LIST_BATCH:                           \
        __glDisplayListBatchEnd(gc);                        \
        break;                                              \
        default: ;                                          \
    }                                                       \

#define __GL_DLIST_BUFFER_FLUSH(gc)                         \
    if (gc->input.beginMode == __GL_SMALL_LIST_BATCH) {     \
       __glDisplayListBatchEnd(gc);                         \
    }                                                       \

#else

#define __GL_SETUP_NOT_IN_BEGIN(gc)
#define __GL_SETUP_NOT_IN_BEGIN_RET(gc,val)
#define __GL_VERTEX_BUFFER_FLUSH(gc)
#define __GL_DLIST_BUFFER_FLUSH(gc)

#endif

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

#define __GL_MIN_CACHE_VERTEX_COUNT         30
#define __GL_VERTEX_CACHE_BLOCK_SIZE        2000

#define __GL_MAX_PTE_HASH_TABLE_SIZE        0x8000
#define __GL_PTE_HASH_INDEX(addr)           (GLuint)(((GLuint64)((GLuint64 *)addr)) & 0x7fff)

#define __GL_BEGIN_END_TAG_MASK         0x10
#define __GL_DRAWARRAYS_TAG_MASK        0x100
#define __GL_VERTEX_DATA_TAG_MASK       0x400

#define __GL_PRIM_ELEMENT_MASK          0xff
#define __GL_PRIM_ELEMENT_SHIFT         6
#define __GL_MAX_PRIM_ELEMENT_NUMBER    10  /* primElemSequence (64 bit)/__GL_PRIM_ELEMENT_SHIFT */
#define __GL_PRIM_ELEMENT(primElemSequence, tag) \
    primElemSequence = ((primElemSequence) << __GL_PRIM_ELEMENT_SHIFT) | ((tag) & __GL_PRIM_ELEMENT_MASK)

/* glBegin, glEnd and batchEnd tags.
*/
enum {
        __GL_BEGIN_POINTS_TAG = __GL_BEGIN_END_TAG_MASK,
        __GL_BEGIN_LINES_TAG,
        __GL_BEGIN_LINE_LOOP_TAG,
        __GL_BEGIN_LINE_STRIP_TAG,
        __GL_BEGIN_TRIANGLES_TAG,
        __GL_BEGIN_TRIANGLE_STRIP_TAG,
        __GL_BEGIN_TRIANGLE_FAN_TAG,
        __GL_BEGIN_QUADS_TAG,
        __GL_BEGIN_QUAD_STRIP_TAG,
        __GL_BEGIN_POLYGON_TAG,
        __GL_END_TAG,
        __GL_BATCH_END_TAG
};

/* Tags DrawArrays and DrawElements.
*/
enum {
        __GL_DRAWARRAYS_POINTS_TAG = __GL_DRAWARRAYS_TAG_MASK,
        __GL_DRAWARRAYS_LINES_TAG,
        __GL_DRAWARRAYS_LINE_LOOP_TAG,
        __GL_DRAWARRAYS_LINE_STRIP_TAG,
        __GL_DRAWARRAYS_TRIANGLES_TAG,
        __GL_DRAWARRAYS_TRIANGLE_STRIP_TAG,
        __GL_DRAWARRAYS_TRIANGLE_FAN_TAG,
        __GL_DRAWARRAYS_QUADS_TAG,
        __GL_DRAWARRAYS_QUAD_STRIP_TAG,
        __GL_DRAWARRAYS_POLYGON_TAG,
        __GL_DRAWARRAYS_END_TAG,
        __GL_ARRAY_V2F_TAG,
        __GL_ARRAY_V3F_TAG,
        __GL_ARRAY_V4F_TAG,
        __GL_ARRAY_C3F_TAG,
        __GL_ARRAY_C4F_TAG,
        __GL_ARRAY_C4UB_TAG,
        __GL_ARRAY_N3F_TAG,
        __GL_ARRAY_TC2F_TAG,
        __GL_ARRAY_TC3F_TAG,
        __GL_ARRAY_TC4F_TAG,
        __GL_ARRAY_N3F_V3F_TAG,
        __GL_ARRAY_C4F_V3F_TAG,
        __GL_ARRAY_INDEX_TAG
};

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

typedef enum __GLvertexCacheModeEnum {
    __GL_FILL_QUICK_VERTEX_CACHE    = 0x0,
    __GL_CHECK_QUICK_VERTEX_CACHE   = 0x2,
    __GL_CHECK_FULL_VERTEX_CACHE    = 0x4,
    __GL_IMMED_VERTEX_CACHE_HIT     = 0x8,
    __GL_VERTEX_CACHE_DISABLED      = 0x10
} __GLvertexCacheMode;

/*
** Current glBegin mode.
*/
typedef enum __GLbeginModeEnum {
    __GL_NOT_IN_BEGIN        = 0,
    __GL_IN_BEGIN            = 1,
    __GL_SMALL_LIST_BATCH    = 2,
    __GL_SMALL_DRAW_BATCH    = 3,
} __GLbeginMode;

typedef struct __GLvertexInputRec {
    GLubyte *pointer;
    GLfloat *currentPtrDW;
    GLuint offsetDW;
    GLuint index;
    GLuint sizeDW;
} __GLvertexInput;

typedef struct __GLvertexInfoRec {
    GLuint inputTag;
    GLuint offsetDW;
    union
    {
        GLuint *appDataPtr;
        GLint first;
    };
    union
    {
        GLuint64 *ptePointer;
        GLint count;
    };
} __GLvertexInfo;

typedef struct __GLpageTableEntryInfoRec {
    struct __GLpageTableEntryInfoRec *next;
    struct __GLpageTableEntryInfoRec *link;
    GLuint index;
    GLuint64 *ptePointer;
} __GLpageTableEntryInfo;

typedef struct __GLpteInfoHashTableRec {
    __GLpageTableEntryInfo **hashTable;
    __GLpageTableEntryInfo *freeList;
    GLuint64 *lastPtePtr[__GL_TOTAL_VERTEX_ATTRIBUTES];
} __GLpteInfoHashTable;

/*
** Immediate mode vertex data cache structure.
*/
typedef struct __GLvertexDataCacheRec {
    GLenum primMode;
    GLuint vertexCount;
    GLuint indexCount;
    GLint cacheStatus;
    GLint infoBufSize;
    GLint dataBufSize;
    GLint indexBufSize;
    GLuint frameIndex;
    GLuint64 primInputMask;
    GLint  numberOfElements;
    GLboolean indexPrimEnabled;
    GLint  vertTotalStrideDW;
    GLuint quickDataCache[2];
    GLenum connectPrimMode;
    GLint  connectVertexCount;
    GLuint connectVertexIndex[4];
    GLuint64 primElemSequence;
    GLuint64 primitiveFormat;
    __GLvertexInfo *vertexInfoBuffer;
    GLfloat *vertexDataBuffer;
    GLvoid *privateData;
    GLushort *indexBuffer;
    GLvoid *ibPrivateData;
    GLint elemOffsetDW[__GL_TOTAL_VERTEX_ATTRIBUTES];
    GLint elemSizeDW[__GL_TOTAL_VERTEX_ATTRIBUTES];
} __GLvertexDataCache;

/*
** Immediate mode vertex cache block structure.
*/
typedef struct __GLvertexCacheBlockRec {
    struct __GLvertexCacheBlockRec *next;
    struct __GLvertexCacheBlockRec *prev;
    GLuint indexOffset;
    /* the maximum cache index used in the cache array */
    GLint  maxVertexCacheIdx;
    __GLvertexDataCache cache[__GL_VERTEX_CACHE_BLOCK_SIZE];
} __GLvertexCacheBlock;

typedef enum __GLvertexInputPathRec
{
    __GL_VERTEXINPUT_PATH_NORMAL = 0,
    __GL_VERTEXINPUT_PATH_MANUAL = 1,
    __GL_VERTEXINPUT_PATH_LAST,
}__GLvertexInputPath;

#define __GL_VERTEXINPUT_MANUAL_ACCUM (__GL_INPUT_VERTEX | __GL_INPUT_TEX0 | __GL_INPUT_TEX1)
#define __GL_VERTEXINPUT_MANUAL_READPIXEL (__GL_INPUT_VERTEX | __GL_INPUT_TEX0)

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
    GLvoid **privPtrAddr;       /* NULL: no cached vertex data */
} __GLstreamDecl;

/*typedef struct __GLbufferObjectRec __GLbufferObject;*/
typedef struct __GLindexStreamDeclRec{
    GLenum type;                     /* Data type of index */
    GLvoid *streamAddr;              /* Index stream address */
    GLvoid **ppIndexBufPriv;         /* source of the index stream: __GLChipVertexBufferInfo **
                                     ** if ppIndexBufObj NULL, from streamAddr;
                                     ** if *ppIndexBufObj:  __GLChipVertexBufferInfo *, if NULL, index from cached indexbuffer,
                                     ** Not NULL, from buffer object(with offset) */
    GLintptr offset;                 /* if index is not from VBO, offset is zero, otherwise this is an offset sent down by app*/
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
    GLuint64 missingAttribs;/*missing flag*/
    GLuint64 primElementMask;/*bitmask for all primitive elements*/
    GLuint indexCount; /* 0: Draw Primitive; Not 0: Draw Index Primitive */
    /*the start (inclusive) and end (exclusive) vertex*/
    /*the actual vertex count to be used is always denoted by (endVertex-startVertex)*/
    GLuint startVertex;
    GLuint endVertex;
    GLSTREAMMODE streamMode;
} __GLvertexStreamMachine;

/* glmaterial (in_begin/end) emulation structure */
typedef struct __GLTnlAccumMachineRec{
    GLuint firstVertexIndex;                        /* Index of The first tnled vertex */
    GLuint vertexCount;                             /* Total vertex count tnled (start from firstVertexIndex(included))*/
    GLuint preVertexIndex;                          /* Index of The last tnled vertex */
    GLvoid (APIENTRY *glimEnd)(__GLcontext* gc);    /* To save current glEnd entry */
} __GLTnlAccumMachine;


extern GLuint edgeFlagInputMask[];

extern GLvoid __glDrawImmedPrimitive(__GLcontext *gc);
extern GLvoid __glPrimitiveBatchEnd(__GLcontext *gc);
extern GLvoid __glImmediateFlushBuffer(__GLcontext *gc);
extern GLvoid __glConsistentFormatChange(__GLcontext *gc);
extern GLvoid __glSwitchToNewPrimtiveFormat(__GLcontext *gc, GLuint attFmtIdx);
extern GLvoid __glSwitchToInconsistentFormat(__GLcontext *gc);
extern GLvoid __glFillMissingAttributes(__GLcontext *gc);
extern GLvoid __glImmedUpdateVertexState(__GLcontext *);
extern GLvoid __glResetImmedVertexBuffer(__GLcontext *gc, GLboolean enableCache);
extern GLvoid __glSwitchToDefaultVertexBuffer(__GLcontext *gc, GLuint inputTag);
extern GLvoid __glComputeRequiredInputMask(__GLcontext *gc);
extern GLvoid __glComputeRequiredInputMaskInstancedEXT(__GLcontext *gc);
extern GLvoid __glFreeImmedVertexCacheBlocks(__GLcontext *gc);
extern __GLvertexDataCache * __glCheckCachedImmedPrimtive(__GLcontext *gc);
extern GLvoid __glImmedFlushBuffer_Cache(__GLcontext *gc, GLuint inputTag);

extern GLvoid __glImmedFlushPrim_Material(__GLcontext *gc, GLboolean bFlushPipe);
extern GLvoid APIENTRY __glim_End_Material (__GLcontext *gc);
#endif /* __gc_gl_vertex_h_ */
#endif
