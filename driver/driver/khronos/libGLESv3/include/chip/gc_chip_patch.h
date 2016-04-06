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


#ifndef __chip_patch_h__
#define __chip_patch_h__

#define __GL_CHIP_PATCH_ENABLED 1


#define __GL_CHIP_PATCH_BBOX_SUB_PER_AXIS 2

/* Last one as whole bounding box */
#define __GL_CHIP_PATCH_BBOXES \
    ((__GL_CHIP_PATCH_BBOX_SUB_PER_AXIS * __GL_CHIP_PATCH_BBOX_SUB_PER_AXIS * __GL_CHIP_PATCH_BBOX_SUB_PER_AXIS) + 1)

#define __GL_CHIP_PATCH_MAIN_BBOX (__GL_CHIP_PATCH_BBOXES - 1)

enum {
    __GL_CHIP_PATCH_PACK_STATUS_UNKNOWN = 0,
    __GL_CHIP_PATCH_PACK_STATUS_QUEUED,
    __GL_CHIP_PATCH_PACK_STATUS_PROCESSING,
    __GL_CHIP_PATCH_PACK_STATUS_READY,
    __GL_CHIP_PATCH_PACK_STATUS_ABORT,
};

enum {
    __GL_CHIP_PATCH_BBOX_STATUS_UNKNOWN = 0,
    __GL_CHIP_PATCH_BBOX_STATUS_QUEUED,
    __GL_CHIP_PATCH_BBOX_STATUS_PROCESSING,
    __GL_CHIP_PATCH_BBOX_STATUS_READY,
    __GL_CHIP_PATCH_BBOX_STATUS_ABORT,
};

typedef enum __GCPatchNum_enum
{
    GC_CHIP_PATCH_30BETA3_1,
    GC_CHIP_PATCH_30BETA3_2,
    GC_CHIP_PATCH_30BETA3_2_31,
    GC_CHIP_PATCH_30BETA3_3,
    GC_CHIP_PATCH_30BETA3_3_31,
    GC_CHIP_PATCH_30BETA3_4,
    GC_CHIP_PATCH_30BETA3_6,
    GC_CHIP_PATCH_30BETA3_6_31,
    GC_CHIP_PATCH_30BETA3_7_1,
    GC_CHIP_PATCH_30BETA3_7_2,
    GC_CHIP_PATCH_30BETA3_7_31,
    GC_CHIP_PATCH_30BETA3_8,
    GC_CHIP_PATCH_30BETA3_8_31,
    GC_CHIP_PATCH_30BETA3_9,
    GC_CHIP_PATCH_30BETA3_A,
    GC_CHIP_PATCH_30BETA3_A_31,
    GC_CHIP_PATCH_3041,
    GC_CHIP_PATCH_3042,
    GC_CHIP_PATCH_MANHATTAN_SKIP_D16TEXTURE,
    GC_CHIP_PATCH_31_2,
    GC_CHIP_PATCH_31_3,
    GC_CHIP_PATCH_31_6,
    GC_CHIP_PATCH_31_7,
    GC_CHIP_PATCH_31_8,
    GC_CHIP_PATCH_31_A,
    GC_CHIP_PATCH_MIRANDA,
    GC_CHIP_PATCH_SIEGECRAFT,
    GC_CHIP_PATCH_NAVIGATION,
    GC_CHIP_PATCH_HOR_BLUR_FILTER,
    GC_CHIP_PATCH_TORCH,
    GC_CHIP_PATCH_GLBENCH25,
    GC_CHIP_PATCH_GLBENCH25_FILL_RATE,
    GC_CHIP_PATCH_GLBENCH27_RC,
    GC_CHIP_PATCH_YOUILABS_ROAD,
    GC_CHIP_PATCH_HOVERJET_TERRAIN_BAKED_SHADOW2,
#if gcdSHADER_SRC_BY_MACHINECODE
    GC_CHIP_PATCH_TAIJI_NORMALMAPPED,
    GC_CHIP_PATCH_TAIJI_NORMALMAPPED_2,
    GC_CHIP_PATCH_TAIJI_BRDF,
    GC_CHIP_PATCH_TAIJI_NORMALMAPPED_SHADOWED,
    GC_CHIP_PATCH_TAIJI_NORMALMAPPED_SHADOWED_2,
    GC_CHIP_PATCH_ANTUTU_3x,
    GC_CHIP_PATCH_GLBENCH25_RELEASE,
    GC_CHIP_PATCH_NENAMARK24,
    GC_CHIP_PATCH_NENAMARKV24,
#endif
    GC_CHIP_PATCH_2714,
    GC_CHIP_PATCH_2715,
    GC_CHIP_PATCH_2715_2,
    GC_CHIP_PATCH_2715_3,
    GC_CHIP_PATCH_16,
    GC_CHIP_PATCH_17,
    GC_CHIP_PATCH_GLBENCH27,
    GC_CHIP_PATCH_GLBENCH275,
    GC_CHIP_PATCH_GLBENCH270,
    GC_CHIP_PATCH_19,
    GC_CHIP_PATCH_19_2,
    GC_CHIP_PATCH_19_3,
    GC_CHIP_PATCH_19_4,
    GC_CHIP_PATCH_19_5,
    GC_CHIP_PATCH_2701B,
    GC_CHIP_PATCH_20,
#if gcdGLB27_SHADER_REPLACE_OPTIMIZATION
    GC_CHIP_PATCH_GLBENCH27X,
    GC_CHIP_PATCH_GLBENCH275_2,
    GC_CHIP_PATCH_GLBENCH270_2,
    GC_CHIP_PATCH_2720,
    GC_CHIP_PATCH_2722,
    GC_CHIP_PATCH_2723,
#endif
    GC_CHIP_PATCH_22,
    GC_CHIP_PATCH_23,
    GC_CHIP_PATCH_GLBENCH21,
    GC_CHIP_PATCH_GLBENCH21_2,
    GC_CHIP_PATCH_GLBENCH21_3,
    GC_CHIP_PATCH_GLBENCH21_4,
    GC_CHIP_PATCH_GLBENCH21_5,
    GC_CHIP_PATCH_GFXBENCH30,
    GC_CHIP_PATCH_GFXBENCH30_2,
    GC_CHIP_PATCH_GFXBENCH31_OVERHEAD,
    GC_CHIP_PATCH_GFXBENCH31,
    GC_CHIP_PATCH_GTF_INTVARYING,
    GC_CHIP_PATCH_GTF_DISCARDDRAW,
    GC_CHIP_PATCH_A8_REPLACE,
    GC_CHIP_PATCH_CKZOMBIES2,
    GC_CHIP_PATCH_ANDROID_CTS_TEXTUREVIEW_REPLACE,
    GC_CHIP_PATCH_A8_REMOVE,
    GC_CHIP_PATCH_2V2,
    GC_CHIP_PATCH_2V2_PRECISION,
#if gcdUSE_WCLIP_PATCH
    GC_CHIP_PATCH_PSC,
#endif
    GC_CHIP_PATCH_VS_FLOATTEX_GTF,
    GC_CHIP_PATCH_VS_INTTEX_GTF,
    GC_CHIP_PATCH_VS_UINTTEX_GTF,
    GC_CHIP_PATCH_USER_CUBE_LOD,
    GC_CHIP_PATCH_USER_CUBE_LOD_BIAS,
    GC_CHIP_PATCH_MAX_UBO_SIZE,
    GC_CHIP_PATCH_COC_1,
    GC_CHIP_PATCH_CTS_DOT,
    GC_CHIP_PATCH_TREX,
    GC_CHIP_PATCH_MANHATTAN,
    GC_CHIP_PATCH_BATCHCOUNT,
    GC_CHIP_PATCH_NETFLIX_1,
    GC_CHIP_PATCH_ES20_CONF_ATAN2,
    GC_CHIP_PATCH_ES20_CONF_ATAN2_REF,
    GC_CHIP_PATCH_DEQP_MSAA_OQ,
    GC_CHIP_PATCH_DEQP_ALPHA_BLEND_1,
    GC_CHIP_PATCH_DEQP_ALPHA_BLEND_2,
    GC_CHIP_PATCH_DEQP_COMPILE_TIME,
    GC_CHIP_PATCH_DEQP_COMPILE_TIME_2,
    GC_CHIP_PATCH_DEQP_COMPILE_TIME_3,
    GC_CHIP_PATCH_DEQP_COMPILE_TIME_4, /* dEQP-GLES31.functional.ubo.random.all_per_block_buffers.13 */

    GC_CHIP_PATCH_LAST
} __GCPatchNum;

#define GC_CHIP_PATCH_NUM (GC_CHIP_PATCH_LAST + 1)


#define __GL_CHIP_PATCH_CLIP_ALLOCATIONS    64
#define __GL_CHIP_PATCH_CLIP_HASHES         29
#define __GL_CHIP_PATCH_CLIP_QUEUES         128

/* Flags of how bufObj was used in vertex packing/bbox process */
#define __GL_CHIP_PATCH_FLAG_IBO            (__GL_ONE_32 << 0)
#define __GL_CHIP_PATCH_FLAG_VBO            (__GL_ONE_32 << 1)

#define __GL_CHIP_PATCH_VERBOSE_LOG         0


typedef struct __GLchipPatchClipInfoRec         __GLchipPatchClipInfo;
typedef struct __GLchipPatchClipArrayRec        __GLchipPatchClipArray;
typedef struct __GLchipPatchClipHashEntryRec    __GLchipPatchClipHashEntry;

typedef struct __GLchipPatchClipBoxRec
{
    /* Number of indices. */
    gctSIZE_T count;

    /* Element Buffer */
    GLubyte * pIndexBase;
    GLubyte * pIndexVar;
    gcoBUFOBJ indexObj;

    /* Whether the bounding box was initialized */
    GLboolean initialized;

    /* Bounding box coordinates. */
    GLfloat   minX, minY, minZ;
    GLfloat   maxX, maxY, maxZ;
} __GLchipPatchClipBox;

struct __GLchipPatchClipInfoRec
{
    /* Array of structures owning this stream. */
    __GLchipPatchClipArray *owner;

    /* Status of the stream. */
    volatile GLint          packStatus;
    volatile GLint          bboxStatus;

    /* Program ID for this optimization. */
    IN GLuint               progID;

    /* Index and position vertex buffer info of the draw */
    IN OUT gceINDEX_TYPE    indexType;
    IN     gctSIZE_T        indexOffset;    /* start bytes in the original index buffer */
    IN     gctSIZE_T        indexEnd;       /* end bytes in the original index buffer */
    IN     gctSIZE_T        indexCount;

    IN     GLuint           oldAttribMask;
    OUT    GLuint           newAttribMask;
    IN OUT GLint            posLocation;
    IN gcsATTRIBUTE         oldAttribs[__GL_MAX_VERTEX_ATTRIBUTES];
    IN gctPOINTER           vertexData[__GL_MAX_VERTEX_ATTRIBUTES];
    IN gctPOINTER           indexData;

    OUT gcsATTRIBUTE        newAttribs[__GL_MAX_VERTEX_ATTRIBUTES];

    /* Object pointers of the packed and optimized buffers. */
    OUT gcoBUFOBJ           indexObj;
    OUT gcoBUFOBJ           streamObj;

    /* Full and sub bounding boxes of input vertices */
    __GLchipPatchClipBox    bboxes[__GL_CHIP_PATCH_BBOXES];

    /* hash array of structures owning this clipInfo.*/
    __GLchipPatchClipHashEntry** hashOwner;
    gctSIZE_T                    curCount;
    gctSIZE_T                    maxCount;

#if (defined(DEBUG) || defined(_DEBUG))
    /* Unique seq number for debugging */
    GLuint                  seq;
#endif

#if gcdDUMP
    GLboolean               bboxesDumped;
    GLboolean               streamDumped;
#endif
};

struct __GLchipPatchClipHashEntryRec
{
    __GLchipPatchClipInfo** pArray;
    gctSIZE_T               curSize;
    gctSIZE_T               maxSize;
};

struct __GLchipPatchClipArrayRec
{
    /* Array of streams. */
    __GLchipPatchClipInfo   clipInfos[__GL_CHIP_PATCH_CLIP_ALLOCATIONS];

    /* Used allocation count. */
    gctSIZE_T               usedCount;

    /* Free allocation count. */
    gctSIZE_T               freeCount;

    /* Pointer to next array of stream structures. */
    __GLchipPatchClipArray *next;
};

typedef struct __GLchipPatchFlagsRec
{
    gctUINT clearStencil     : 1;
    gctUINT disableEZ        : 1;
    gctUINT stack            : 1;
    gctUINT blurDepth        : 1;
    gctUINT uiDepth          : 1;
    gctUINT ui               : 1;
    gctUINT depthScale       : 1;
    gctUINT clipW            : 2;
    gctUINT countDraws       : 1;
    gctUINT fastRenderTarget : 1;
    gctUINT triLinear        : 1;
    gctUINT skipDummy        : 1;
    gctUINT vertexPack       : 1;
    gctUINT swClip           : 1;
    gctUINT shadowMap        : 1;
    gctUINT rgbEncoding      : 1;
    gctUINT discardDraw      : 1;
    gctUINT skipD16Draw      : 1;
    gctUINT isNavi           : 1;
    gctUINT naviNormal       : 1;
} __GLchipPatchFlags;

typedef struct __GLchipPatchBatchRec
{
    struct __GLchipPatchBatchRec * next;

    GLenum                  mode;
    gctSIZE_T               count;
    GLenum                  type;
    const GLvoid *          indices;

    __GLBufGeneralBindPoint vertexBound;
    __GLBufGeneralBindPoint indexBound;

    __GLvertexArrayState    vertexArrayState;

    __GLshaderProgramMachine shaderProgram;
    __GLchipSLProgram       *program;
    GLvoid                  *uniformData;

    /* Texture data. */
    GLbitfield              texUnitAttrState[__GL_MAX_TEXTURE_UNITS];
    __GLbitmask             texUnitAttrDirtyMask;
    __GLbitmask             currentEnableMask;
    __GLtextureUnit         units[__GL_MAX_TEXTURE_UNITS];
} __GLchipPatchBatch;

typedef struct __GLchipPatchInfoRec
{
    /* Patch flags. */
    __GLchipPatchFlags      patchFlags;

    /* Program handle for cleanup. */
    __GLchipSLProgram       *patchCleanupProgram;

    /* Allow early Z. */
    GLboolean               allowEZ;

    /* UI surface. */
    gcoSURF                 uiSurface;

    /* Depth buffer. */
    gcoSURF                 uiDepth;

    /* Save read buffer. */
    gcoSURF                 uiRead;

    /* Save batches on stack. */
    GLboolean               stackSave;

    /* Top of stack pointer. */
    __GLchipPatchBatch     *stackPtr;
    /* List of free batches. */
    __GLchipPatchBatch     *stackFreeList;
    /* Horizontal blur program. */
    __GLchipSLProgram      *blurProgram;

    /* */
    GLboolean               bufBindDirty;

    /* Record app set maximum viewport size. */
    gctSIZE_T               maxViewportW;
    gctSIZE_T               maxViewportH;

    /* Draw counter. */
    GLint                   drawCount;
    GLint                   currentDraw;
    GLint                   countUniform;
    GLboolean               firstLoop;
    __GLchipSLUniform *     uniformDrawCount;

    /* Vertex Pack + SW Culling */
    GLuint                  notificationCount;
    gctSTRING               posAttribName;
    gctSTRING               mvpUniformName;
    gctSIZE_T               minVertexCountForSwClip;

    __GLchipPatchClipArray *clipArray;

    /* Vertex Pack */
    gcsTLS_PTR              packTls;
    gctPOINTER              packThread;
    gctSIGNAL               packSignal;
    __GLchipPatchClipInfo * packQueue[__GL_CHIP_PATCH_CLIP_QUEUES];
    volatile GLuint         packQueueProducer;
    volatile GLuint         packQueueConsumer;

    /* SW Clip */
    gcsTLS_PTR              bboxTls;
    gctPOINTER              bboxThread;
    gctSIGNAL               bboxSignal;
    __GLchipPatchClipInfo * bboxQueue[__GL_CHIP_PATCH_CLIP_QUEUES];
    volatile GLuint         bboxQueueProducer;
    volatile GLuint         bboxQueueConsumer;

    /* Road test on youilabs. */
    GLboolean               youilabs;
    __GLchipSLUniform *     uniformTime;
    __GLchipSLUniform *     uniformTemp;

    __GLchipSLProgram *     shadowProgram[4];
    GLchar *                rgbSearch;

    /* Commit true before deleting texture to force memory freed. */
    GLboolean               commitTexDelete;
    /* Discard draw */
    __GLchipSLUniform *     uniformDiscard;

    /* Identify basemark2v2 normal/high quality of navigation: > 2 is high. */
    gctUINT                 clearCount;
} __GLchipPatchInfo;

void
gcChipGetPatchConditionTb(
    __GLcontext *gc
    );

void
gcChipPatchLink(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    const gctCHAR **patchedSrcs,
    gctINT *replaceIndices
    );
#if gcdUSE_WCLIP_PATCH
void
gcChipPatchBinary(
    __GLcontext *gc,
    gcSHADER binary
    );
#endif

GLboolean
gcChipPatchDraw(
    __GLcontext *gc,
    GLenum Mode,
    gctSIZE_T Count,
    GLenum Type,
    const GLvoid *Indices
    );

gceSTATUS
gcChipPatchTexture(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLuint unit
    );

gctBOOL
gcChipPatchQueryEZ(
    __GLcontext *gc
    );

void
gcChipPatchCleanUpProgram(
    __GLcontext *gc,
    IN __GLchipSLProgram *Program
    );

void
gcChipPatchUpdateUniformData(
    __GLcontext *gc,
    __GLprogramObject *progObj,
    __GLchipSLProgram *program
    );

GLvoid
gcChipPatchValidateViewport(
    __GLcontext *gc
    );

GLboolean
gcChipPatchClear(
    __GLcontext *gc,
    GLbitfield *mask,
    GLint *savedWriteMask
    );

void
gcChipPatchRemoveClipHash(
    __GLchipPatchClipInfo *clipInfo,
    __GLchipPatchClipHashEntry *clipHash
);

void
gcChipPatchDeleteBuffer(
    __GLcontext *gc,
    __GLchipVertexBufferInfo *bufInfo
    );

void
__glChipPatchBlend(
    IN __GLcontext *gc,
    IN gctBOOL bEnable
    );

GLvoid gcChipPatchDirtyClipInfo(
    __GLcontext *gc,
    __GLchipVertexBufferInfo *idxBufInfo,
    gctSIZE_T offset,
    gctSIZE_T size);

void
gcChipPatchDeleteClipInfo(
    __GLcontext *gc,
    __GLchipPatchClipInfo *clipInfo
    );

gceSTATUS
gcChipPatchSplitBBox(
    __GLcontext *gc,
    __GLchipPatchClipInfo* clipInfo
    );

GLboolean
gcChipPatchBBoxClip(
    __GLcontext *gc,
    __GLchipPatchClipBox *bbox,
    const GLfloat *mvp
    );

__GLchipPatchClipInfo*
gcChipPatchVertexPacking(
    __GLcontext              *gc,
    __GLchipVertexBufferInfo *idxBufInfo,
    gceINDEX_TYPE             indexType,
    gctSIZE_T                 offset,
    gctSIZE_T                 count
    );

#if gcdDUMP
void
gcChipPatchDumpVertexPackingResult(
    __GLchipPatchClipInfo *clipInfo
    );
#endif


/* Optimization for es30 conformance test BlitFramebuffer:
*/

#define __GL_STENCIL_BUF_X_GRIDS 4
#define __GL_STENCIL_BUF_Y_GRIDS 4

typedef struct __GLchipStencilOptRec
{
    gctSIZE_T bpp;    /* bit planes of the stencil buffer */

    gctSIZE_T width;  /* width of the stencil buffer */
    gctSIZE_T height; /* height of the stencil buffer */

    /* Driver will divide the stencil surfaces into sub blocks and record
    ** overall stencil value of each block. If driver cannot know the value,
    ** or stencil values of the whole block are not same, GL_INVALID_INDEX
    ** will be recorded.
    */
    GLuint snapshots[__GL_STENCIL_BUF_Y_GRIDS][__GL_STENCIL_BUF_X_GRIDS];
} __GLchipStencilOpt;

GLvoid
gcChipPatchStencilOptReset(
    __GLchipStencilOpt *stencilOpt,
    gctSIZE_T width,
    gctSIZE_T height,
    gctSIZE_T bpp
    );

__GLchipStencilOpt*
gcChipPatchStencilOptGetInfo(
    __GLcontext *gc,
    GLboolean read
    );

GLvoid
gcChipPatchStencilOptWrite(
    __GLcontext *gc,
    __GLchipStencilOpt *stencilOpt,
    gcsRECT *rect,
    GLuint value,
    GLuint mask,
    GLboolean reset
    );

GLvoid
gcChipPatchStencilOptBlit(
    __GLcontext *gc,
    gcsRECT *srcRect,
    gcsRECT *dstRect,
    GLint scissorLeft,
    GLint scissorRight,
    GLint scissorTop,
    GLint scissorBottom,
    GLboolean xReverse,
    GLboolean yReverse
);

GLboolean
gcChipPatchStencilOptTest(
    __GLcontext * gc,
    __GLchipStencilOpt *stencilOpt
    );

#endif /* __chip_patch_h__ */
