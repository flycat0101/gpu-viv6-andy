/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __chip_buffer_h__
#define __chip_buffer_h__

#ifdef __cplusplus
extern "C" {
#endif

#define __GL_CHIP_MAX_INDEX_SUB 64

/*
** This structure is used to save vertex buffer information,
** vertex buffer object privateData points to this structure
*/
struct __GLchipVertexBufferInfoRec
{
    gcoBUFOBJ                   bufObj;         /* Allocated memory node for buffer object */

    gctSIZE_T                   size;           /* buffer size in bytes */
    GLboolean                   fresh;          /* whether buffer had been renamed */

    gctSIZE_T                   freeOffset;     /* byte offset of free space from buffer start */
    gctSIZE_T                   freeSize;       /* buffer size - free offset */
    GLvoid                      *bufferMapPointer;

    GLboolean                   isMapped;

    gceBUFOBJ_USAGE             usage;           /* usage type of the buffer. Static? Dynamic? */

    GLboolean                   boundAsIB;       /* whether buffer was ever bind as IB? */
    GLboolean                   patchDirty;
    gctSIZE_T                   unAlignedStep;
    gceINDEX_TYPE               indexType;

    gcoBUFOBJ                   listIndexEven;
    gcoBUFOBJ                   listIndexOdd;

    gcoBUFOBJ                   shiftObj;
    gctINT                      shiftBias;

#if __GL_CHIP_PATCH_ENABLED
    GLvoid *                    cache;
    GLbitfield                  clipFlags;
    __GLchipPatchClipHashEntry  clipHashes[__GL_CHIP_PATCH_CLIP_HASHES];
#endif
};


typedef enum {
    /*
    ** The core format different with mapped request HAL format.
    */
    __GL_CHIP_FMTFLAGS_FMT_DIFF_CORE_REQ             = 0x0001,

    /*
    ** The request HAL format different with texture HAL format.
    */
    __GL_CHIP_FMTFLAGS_FMT_DIFF_REQ_READ             = 0x0002,

    /*
    ** The request HAL format different with render HAL format.
    */
    __GL_CHIP_FMTFLAGS_FMT_DIFF_REQ_WRITE            = 0x0004,

    /*
    ** The texture HAL format different with render HAL format.
    */
    __GL_CHIP_FMTFLAGS_FMT_DIFF_READ_WRITE           = 0x0008,

    /*
    ** The texture HAL has different tiling with render HAL format
    */
    __GL_CHIP_FMTFLAGS_LAYOUT_DIFF_READ_WRITE        = 0x0010,

    /*
    ** With this flag, we can sample from write directly
    ** if only sampling level0, even read/write is different.
    */
    __GL_CHIP_FMTFLAGS_FMT_SAMPLE_LEVEL0_FROM_WRITE  = 0x0020,

    /*
    ** There is no HAL format map to this core format
    */
    __GL_CHIP_FMTFLAGS_CANT_FOUND_HAL_FORMAT         = 0x80000000,

} __GLchipFmtFlags;


typedef enum {
    /* none patch */
    __GL_CHIP_FMT_PATCH_NONE  = 0,

    /* 2Darray/3D texture format mapping */
    __GL_CHIP_FMT_PATCH_CASE0 = 1,

    /* SRGB rendering with better precision*/
    __GL_CHIP_FMT_PATCH_CASE1 = 2,

    /* Force Z24 render for some quality cases */
    __GL_CHIP_FMT_PATCH_CASE2 = 3,

    /* Force Half float as NORM8 to be linear filterable */
    __GL_CHIP_FMT_PATCH_CASE3 = 4,

    /* Force map SRGB to A16B16G16R16F_2_G16R16F */
    __GL_CHIP_FMT_PATCH_CASE4 = 5,

    /* Force map R8I/R8UI msaa to R8I_1_A4R4G4B4 */
    __GL_CHIP_FMT_PATCH_8BIT_MSAA = 6,

    /* Force D32F to D24 to avoid shadow rendering for array types */
    __GL_CHIP_FMT_PATCH_D32F,

    /* Force ASTC to RGBA8 when gcvFEATURE_TX_ASTC_MULTISLICE_FIX is false */
    __GL_CHIP_FMT_PATCH_ASTC,

    /* Force ALPHA8 to RGBA8 */
    __GL_CHIP_FMT_PATCH_ALPHA8,

    /* Force A8L8 to use gcvSURF_A8L8_1_A8R8G8B so we could handle copy texture from ARGB8 render target */
    __GL_CHIP_FMT_PATCH_A8L8,

} __GLchipFmtPatch;

typedef struct __GLchipFmtMapInfoRec
{
    /*
    ** By default, requestHALFormat has same layout as corresponding core format.
    */
    gceSURF_FORMAT requestFormat;

    /*
    ** Real surface format for texture reading for this core format
    */
    gceSURF_FORMAT readFormat;

    /*
    ** Real surface format for render target for this core format
    */
    gceSURF_FORMAT writeFormat;

    /* Flag indicate format mapping.
    **
    */
    __GLchipFmtFlags  flags;

    /*
    ** Special format mapping for some cases.
    */
    __GLchipFmtPatch patchCase;

    /* Number of sampler counts the format supported if write is not UNKNOWN*/
    GLint       numSamples;
    GLint*       samples;

} __GLchipFmtMapInfo;



extern GLboolean
__glChipBindBufferObject(
    __GLcontext* gc,
    __GLbufferObject* bufObj,
    GLuint targetIndex
    );

extern GLboolean
__glChipDeleteBufferObject(
    __GLcontext* gc,
    __GLbufferObject* bufObj
    );

extern GLvoid*
__glChipMapBufferRange(
    __GLcontext* gc,
    __GLbufferObject* bufObj,
    GLuint targetIndex,
    GLintptr offset,
    GLsizeiptr length,
    GLbitfield access
    );

extern GLboolean
__glChipFlushMappedBufferRange(
    __GLcontext* gc,
    __GLbufferObject* bufObj,
    GLuint targetIndex,
    GLintptr offset,
    GLsizeiptr length
    );

extern GLboolean
__glChipUnMapBufferObject(
    __GLcontext* gc,
    __GLbufferObject* bufObj,
    GLuint targetIndex
    );

extern GLboolean
__glChipBufferData(
    __GLcontext* gc,
    __GLbufferObject* bufObj,
    GLuint targetIndex,
    const void* data
    );

extern GLboolean
__glChipBufferSubData(
    __GLcontext* gc,
    __GLbufferObject* bufObj,
    GLuint targetIndex,
    GLintptr offset,
    GLsizeiptr size,
    const void* data
    );

extern GLboolean
__glChipCopyBufferSubData(
    __GLcontext* gc,
    GLuint readTargetIndex,
    __GLbufferObject* readBufObj,
    GLuint writeTargetIndex,
    __GLbufferObject* writeBufObj,
    GLintptr readOffset,
    GLintptr writeOffset,
    GLsizeiptr size
    );

extern gceSTATUS
gcChipInitFormatMapInfo(
    __GLcontext *gc
    );

extern __GLchipFmtMapInfo*
gcChipGetFormatMapInfo(
    __GLcontext *gc,
    __GLformat  drvFormat,
    __GLchipFmtPatch patchCase
    );


#ifdef __cplusplus
}
#endif

#endif /* __chip_buffer_h__ */
