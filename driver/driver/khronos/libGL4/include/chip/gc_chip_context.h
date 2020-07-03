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


#ifndef __chip_context_h__
#define __chip_context_h__

#include "gc_hal.h"
#include "gc_hal_raster.h"
#include "gc_hal_engine.h"
#include "drvi/gc_vsc_drvi_interface.h"
#include "gc_hal_user_math.h"
#include "gc_hal_user_os_memory.h"
#include "gc_chip_base.h"
#include "gc_chip_utils.h"
#include "gc_chip_patch.h"
#include "gc_chip_buffer.h"
#include "gc_chip_texture.h"
#include "gc_chip_device.h"
#include "gc_chip_shader.h"
#include "gc_chip_fbo.h"
#include "gc_chip_misc.h"
#ifdef OPENGL40
#include "gc_chip_hash.h"
#include "gc_chip_pixel.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define __GL_VENDOR         "Vivante Corporation"

#define __GL_VERSION20      "OpenGL ES 2.0 V"
#define __GL_VERSION30      "OpenGL ES 3.0 V"
#define __GL_VERSION31      "OpenGL ES 3.1 V"
#define __GL_VERSION32      "OpenGL ES 3.2 V"

#define __GL_GLSL_VERSION20 "OpenGL ES GLSL ES 1.0.0"
#define __GL_GLSL_VERSION30 "OpenGL ES GLSL ES 3.00"
#define __GL_GLSL_VERSION31 "OpenGL ES GLSL ES 3.10"
#define __GL_GLSL_VERSION32 "OpenGL ES GLSL ES 3.20"

#ifdef OPENGL40
/* Fix me for the next defines */
#define __OGL_VERSION21      "2.1 V"
#define __OGL_VERSION30      "3.0 V"
#define __OGL_VERSION40      "4.0 V"

#define __OGL_GLSL_VERSION21 "2.1.0 "
#define __OGL_GLSL_VERSION30 "3.00 "
#define __OGL_GLSL_VERSION40 "4.00 "

#define __GL_MAX_HW_LIGHTS            8
#define __GL_MAX_GEOMETRY_VARYING_COMPONENTS        (1024-4)   /* exclude gl_Position */
#define __GL_MAX_VERTEX_VARYING_COMPONENTS          (16*6*4)
typedef enum {
_GL_VERTEX_INDEX = 0,
_GL_COLOR_INDEX,
_GL_MULTITEX0_INDEX,
_GL_MULTITEX1_INDEX,
_GL_MULTITEX2_INDEX,
_GL_MULTITEX3_INDEX,
_GL_MULTITEX4_INDEX,
_GL_MULTITEX5_INDEX,
_GL_MULTITEX6_INDEX,
_GL_MULTITEX7_INDEX,
_GL_BT_INDEX_MAX
} BUILT_IN_ATTR_INDEX;

#endif

#define __GL_MAX_VS_CONSTANT          256
#define __GL_MAX_PS_CONSTANT          256

#define __GL_MAX_PRIMITIVE_COUNT      0x15555
#define __GL_MAX_HW_ARRAY_SIZE        512
#define __GL_MAX_HW_DEPTH_SIZE        512

#define __GL_MIN_PROGRAM_TEXEL_OFFSET   (-8)
#define __GL_MAX_PROGRAM_TEXEL_OFFSET   7

#define __GL_MIN_PROGRAM_TEXGATHER_OFFSET   (-8)
#define __GL_MAX_PROGRAM_TEXGATHER_OFFSET   7


    /* query counter bits */
#define __GL_QUERY_COUNTER_BITS     64


#define __GL_MAX_VIEWPORT_WIDTH     8192
#define __GL_MAX_VIEWPORT_HEIGHT    8192

/*when fbo attachment is GL_TEXTURE_2D_ARRAY texture, according to spec, max surface count should be GL_MAX_ARRAY_TEXTURE_LAYERS-1 */
#define __GL_CHIP_SURF_COUNT        511

#define __GL_CHIP_ENABLE_MEMORY_REDUCTION  0

typedef enum __GLchipHaltiLeve_enum
{
    __GL_CHIP_HALTI_LEVEL_0  = 0,
    __GL_CHIP_HALTI_LEVEL_1  = 1,
    __GL_CHIP_HALTI_LEVEL_2  = 2,
    __GL_CHIP_HALTI_LEVEL_3  = 3,
    __GL_CHIP_HALTI_LEVEL_4  = 4,
    __GL_CHIP_HALTI_LEVEL_5  = 5,
    __GL_CHIP_HALTI_LEVEL_6  = 6,
} __GLchipHaltiLevel;


typedef struct __GLchipFeatureRec
{
    __GLchipHaltiLevel  haltiLevel;

    /* Indicate if HW support render-to-texture natively?
    ** Note: which path was chosen doesn't depend on only HW capability, but also some API parameters.
    **       E.g. FBO attaches multisampled texture implicitly requires indirect path by spec.
    */
    struct
    {
        GLuint                   indirectRTT                : 1;
        GLuint                   hasCorrectStencil          : 1;
        GLuint                   hasTileStatus              : 1;
        GLuint                   wideLine                   : 1;
        GLuint                   patchTriangleStrip         : 1;
        GLuint                   lineLoop                   : 1;
        GLuint                   primitiveRestart           : 1;
        GLuint                   patchNP2Texture            : 1;
        GLuint                   msaaFragmentOperation      : 1;
        GLuint                   hasYuv420Tiler             : 1;
        GLuint                   hasYuvAssembler            : 1;
        GLuint                   hasLinearTx                : 1;
        GLuint                   hasTxSwizzle               : 1;
        GLuint                   hasSupertiledTx            : 1;
        GLuint                   hasTxTileStatus            : 1;
        GLuint                   hasTxDecompressor          : 1;
        GLuint                   attrib2101010Rev           : 1;
        GLuint                   extendIntSign              : 1;
        GLuint                   hasTxDescriptor            : 1;
        GLuint                   hasBlitEngine              : 1;
        GLuint                   hasHwTFB                   : 1;
        GLuint                   txDefaultValueFix          : 1;
        GLuint                   hasCommandPrefetch         : 1;
        GLuint                   hasYuvAssembler10bit       : 1;
        GLuint                   supportMSAA2X              : 1;
        GLuint                   hasSecurity                : 1;
        GLuint                   hasRobustness              : 1;
        GLuint                   txLerpLessBit              : 1;

        GLuint                   hasTxBorderClamp           : 1;
        GLuint                   hasVertex1010102           : 1;
        GLuint                   hasTxDXT                   : 1;
        GLuint                   hasTxAnisFilter            : 1;
        GLuint                   hasHalfFloatPipe           : 1;
        GLuint                   hasPSIOInterlock           : 1;
        GLuint                   hasMSAAshading             : 1;
        GLuint                   hasStencilTexture          : 1;
        GLuint                   hasSeparateRTCtrl          : 1;
        GLuint                   hasASTC                    : 1;
        GLuint                   hasASTCCodecFix            : 1;
        GLuint                   hasGS                      : 1;
        GLuint                   hasCubeArray               : 1;
        GLuint                   hasAdvancedInstr           : 1;
        GLuint                   hasMultiDrawIndirect       : 1;
        GLuint                   hasTextureBuffer           : 1;
        GLuint                   hasTS                      : 1;
        GLuint                   hasDrawElementBaseVertex   : 1;
        GLuint                   hasInteger32Fix            : 1;
        GLuint                   hasDrawIndirect            : 1;
        GLuint                   hasPatchListFetchFix       : 1;
        GLuint                   hasFEstartVertex           : 1;
        GLuint                   hasPEB2BPixelFix           : 1;
        GLuint                   hasV2MSAACoherencyFix      : 1;
        GLuint                   hasIndexFetchFix           : 1;
        GLuint                   hasComputeIndirect         : 1;
        GLuint                   hasBugFixes18              : 1;
        GLuint                   hasCubeBorderLOD           : 1;
        GLuint                   hasTxFrac6Bit              : 1;
        GLuint                   hasTxFrac8Bit              : 1;
        GLuint                   hasMSAAOQFix               : 1;
        GLuint                   hasWideLineHelperFix       : 1;
        GLuint                   needWideLineTriangleEMU    : 1;
        GLuint                   hasPEEnhancement2          : 1;
        GLuint                   hasBugFixes7               : 1;
        GLuint                   hasPEDitherFix2            : 1;
        GLuint                   hasCompressionV1           : 1;
        GLuint                   hasRSBLTMsaaDecompression  : 1;
        GLuint                   hasAdvanceBlendPart0       : 1;
        GLuint                   hasTxBaseLOD               : 1;
        GLuint                   hasRADepthWrite            : 1;
        GLuint                   hasTxGather                : 1;
        GLuint                   hasTxGatherOffsets         : 1;
        GLuint                   hasUnifiedSamplers         : 1;
        GLuint                   hasD24S8SampleStencil      : 1;
        GLuint                   hasSinglePipeHalti1        : 1;
        GLuint                   hasPSIODual16_32bpcFix     : 1;
        GLuint                   hasDepthBiasFix            : 1;
        GLuint                   hasSRGBRT                  : 1;
        GLuint                   hasTileFiller              : 1;
        GLuint                   hasTxASTCMultiSliceFix     : 1;
        GLuint                   hasMultiPixelPipes         : 1;
        GLuint                   hasSingleBuffer            : 1;
        GLuint                   hasPaLineClipFix           : 1;
        GLuint                   hasMSAA                    : 1;
        GLuint                   hasLogicOp                 : 1;
    }hwFeature;
} __GLchipFeature;

typedef struct __GLchipContextRec __GLchipContext;

typedef struct __GLchipDirtyRec
{
    union __GLchipBufferDirtyUnion{
        struct __GLchipBufferDirtyRec {
            unsigned int rtSurfDirty          : 1;
            unsigned int zSurfDirty           : 1;
            unsigned int zOffsetDirty         : 1;
            unsigned int sSurfDirty           : 1;
            unsigned int sOffsetDirty         : 1;
            unsigned int rtNumberDirty        : 1;
            unsigned int zeroRTDirty          : 1;
            unsigned int rtSamplesDirty       : 1;
            unsigned int layeredDirty         : 1;
        } sBuffer;
        unsigned int bufferDirty;
    } uBuffer;

    /* Defer dirty bits which dependent on multiple state*/
    union __GLchipDeferDirtyUnion
    {
        struct __GLchipDeferDirtyBitsRec
        {
            unsigned int depthRange      : 1;
            unsigned int depthMode       : 1;
            unsigned int depthTest       : 1;
            unsigned int depthMask       : 1;
            unsigned int viewportScissor : 1;
            unsigned int vsReload        : 1;
            unsigned int fsReload        : 1;
            unsigned int csReload        : 1;
            unsigned int stencilRef      : 1;
            unsigned int stencilMode     : 1;
            unsigned int stencilTest     : 1;
            unsigned int culling         : 1;
            unsigned int colorMask       : 1;
            unsigned int blend           : 1;
            unsigned int tcsReload       : 1;
            unsigned int tesReload       : 1;
            unsigned int gsReload        : 1;
            unsigned int activeUniform   : 1;
            unsigned int lastFragData    : 1;
            unsigned int pgInsChanged    : 1;
            unsigned int polygonOffset   : 1;
            unsigned int reserved        : 11;
        } sDefer;
        unsigned int deferDirty;
    } uDefer;

    /* patch dirty bits which need overwrite normal context state*/
    union __GLchipPatchDirtyUnion
    {
        struct __GLchipPatchDirtyBitsRec
        {
            unsigned int patchViewport   : 1;
            unsigned int patchreserved   : 31;
        } sPatch;
        unsigned int patchDirty;
    } uPatch;
} __GLchipDirty;


#if __GL_CHIP_PATCH_ENABLED
#define __GL_MAX_DRAW_LOOPS __GL_CHIP_PATCH_BBOXES
#else
#define __GL_MAX_DRAW_LOOPS 1
#endif

#define __GL_DEFAULT_LOOP 0

#define __GL_CHIP_NAME_LEN 32


#ifdef OPENGL40
#define gldATTRIBUTE_DRAWTEX_POSITION       16
#define gldATTRIBUTE_DRAWCLEAR_POSITION     17
#define gldMAX_ATTRIBUTES_INFO              18
#define gldATTRIBUTE_POINT_SIZE             5
#define gldATTRIBUTE_INDEX                  10
#endif

/*
** Contains instant draw attributes,
*/
typedef struct __GLchipInstantDrawRec
{
    /* baseVertex(startVertex) we could program to hardware
    ** if hardware DP command support startVertex
    */
    gctINT          first;
    gctSIZE_T       count;

    /* indexLoops > 0 means it's an index draw */
    gceINDEX_TYPE   indexType;
    gctPOINTER      indexMemory;
    gcoBUFOBJ       indexBuffer;

    gctSIZE_T       primCount;
    gcePRIMITIVE    primMode;

    GLuint          attribMask;
    gcsATTRIBUTE *  attributes;
    GLint           positionIndex;

    gctBOOL         primitiveRestart;
    gctUINT         restartElement;
} __GLchipInstantDraw;

typedef struct __GLchipHalRtSlotInfoRec
{
    gctUINT numOfSlots;
    gctUINT slots[gcdMAX_SURF_LAYERS];
} __GLchipHalRtSlotInfo;


struct __GLchipContextRec
{
    gcoHAL                      hal;
    gco3D                       engine;
    gcoOS                       os;

    gceCHIPMODEL                chipModel;
    GLuint                      chipRevision;
    GLchar                      chipName[__GL_CHIP_NAME_LEN];

    /* GLSL Compiler. */
    gctHANDLE                   dll;
    gctGLSLCompiler             pfCompile;
    gctGLSLInitCompiler         pfInitCompiler;
    gctGLSLInitCompilerCaps     pfInitCompilerCaps;
    gctGLSLFinalizeCompiler     pfFinalizeCompiler;

    /* Attention: Below 3 shortcuts ONLY can be used within draw/compute validation,
    **            Any places out of there is invalid. Because the program may be
    **            deleted, while this fields have no chance to be reset and left wild.
    */
    __GLchipSLProgram           *activePrograms[__GLSL_STAGE_LAST];
    gcePROGRAM_STAGE_BIT        activeStageBits;
    __GLchipSLProgram           *prePAProgram;

    /* Cmd/state buffer cache of hash type: currently only used for pipeline obj cmd cache */
    __GLchipUtilsHash*          cmdInstaceCache;

    /* Currently active program state which could be from a convertional program or a LINKED PPO */
    gcsPROGRAM_STATE_PTR        activeProgState;

    __GLchipFeature             chipFeature;
    GLboolean                   needStencilOpt;

    /* Temporary bitmap. */
    gcoSURF                     tempBitmap;
    GLubyte                     *tempBits;
    GLubyte                     *tempLastLine;
    gceSURF_FORMAT              tempFormat;
    gctSIZE_T                   tempBitsPerPixel;
    gctSIZE_T                   tempX;
    gctSIZE_T                   tempY;
    gctSIZE_T                   tempWidth;
    gctSIZE_T                   tempHeight;
    gctSIZE_T                   tempStride;

    GLboolean                   initialized;

    __GLchipTexture             texture;

    /* Information for draw buffer setting */
    GLuint                      maxDrawRTs; /* Max HW/HAL supported RTs */
    GLuint                      drawInteger;
    GLuint                      drawFloat;
    gctSIZE_T                   drawRTWidth;
    gctSIZE_T                   drawRTHeight;
    GLuint                      drawRTSamples;
    GLuint                      drawStencilMask;
    gcsSURF_VIEW                drawRtViews[__GL_MAX_DRAW_BUFFERS];
    GLuint                      drawRTnum;  /* Currently used HW/HAL RTs */
    GLboolean                   drawYInverted;
    GLboolean                   drawNoAttach;
    GLboolean                   drawLayered;
    GLuint                      drawMaxLayers;
    /* Single RT only */
    gceSURF_ROTATION            drawRTRotation;
    gcsSURF_VIEW                drawDepthView;
    gcsSURF_VIEW                drawStencilView;

    /* Read RT information */
    GLboolean                   readInteger;
    GLboolean                   readYInverted;
    gctSIZE_T                   readRTWidth;
    gctSIZE_T                   readRTHeight;
    gcsSURF_VIEW                readRtView;
    gcsSURF_VIEW                readDepthView;
    gcsSURF_VIEW                readStencilView;
    GLboolean                   readLayered;

    gcsATTRIBUTE                attributeArray[__GL_MAX_VERTEX_ATTRIBUTES];
    /* Record attrib index, actually means hw attribute register usage. */
    GLint                       attribMask;
    GLint                       positionIndex;
    GLint                       directPositionIndex;
    gcoVERTEXARRAY              vertexArray;

    /* Translated primitive type seen in PA */
    GLenum                      primitveType;
    GLenum                      lastPrimitiveType;

#if gcdUSE_WCLIP_PATCH
    /* For WClipping Patch. */
    GLfloat                     wLimitRms;
    gctBOOL                     wLimitRmsDirty;
    gctBOOL                     wLimitPatch;
    gctBOOL                     clipW;
    gctFLOAT                    clipWValue;
    gctFLOAT                    wLimitZNear;
    gctFLOAT                    wLimitVIVMVP[16];
    gctBOOL                     wLimitSettled;
    gctBOOL                     computeWlimitByVertex;
    gctUINT                     wLimitComputeLimit;
    gctUINT                     wLimitSampleCount;
    gctBOOL                     wLimitPSC;
#endif

     /* Record GL viewport, patch logic may change it. */
    gctINT                      viewportWidth;
    gctINT                      viewportHeight;

    /* Attribute Divisor flag */
    gctBOOL                     anyAttibInstanced;
    gctBOOL                     anyAttibGeneric;
    gctBOOL                     anyAttibConverted;

    gceDEPTH_MODE               depthMode;

    __GLchipDirty               chipDirty;

    gceLAYOUT_QUALIFIER         advBlendMode;
    gctBOOL                     advBlendInShader;

#if __GL_CHIP_PATCH_ENABLED
    /* Patches. */
    __GLchipPatchInfo           patchInfo;
#endif
    /* Key state will affect statekey of program */
    __GLchipProgramStateKey    *pgKeyState;

    /* LTC evaluation */
    gctPOINTER                  cachedLTCResultArray;
    gctSIZE_T                   curLTCResultArraySize;

    /* Reusable buffer for index patches */
    gctPOINTER                  tempIndexBuffer;
    gctSIZE_T                   tempIndexBufferSize;

    /* How many index draw instances, it was used by the swClip patch.
    ** "indexLoops = 0" means it is not an indexed draw
    */
    GLuint                      indexLoops;

    /* Instant Draw parameters */
    __GLchipInstantDraw         instantDraw[__GL_MAX_DRAW_LOOPS];

    gcePATCH_ID                 patchId;

    gcoTEXTURE                  rtTexture;

    /* App setting -> HAL rt slot mapping */
    __GLchipHalRtSlotInfo       rtHalMapping[__GL_MAX_DRAW_BUFFERS];

    /* Hal rtSlot -> chip layer rt setting */
    GLint                       psOutputMapping[gcdMAX_DRAW_BUFFERS];

    GLfloat                     sampleLocations[2][4][4];

    gceSTATUS                   errorStatus;

    gctBOOL                     doPatchCondition[GC_CHIP_PATCH_NUM];
    GLint                       numSamples;
    GLint                       samples[4];

    gctBOOL                     robust;

    /* flags quickly check if we need do recompile */
    gctBOOL                     needTexRecompile;
    gctBOOL                     needRTRecompile;

#if gcdALPHA_KILL_IN_SHADER
    gctBOOL                     alphaKillInShader;
#endif

#if VIVANTE_PROFILER
    gcoPROFILER                 profiler;
#endif

#ifdef OPENGL40
    gctBOOL                     hwPointSprite;
    gcoSURF                     drawRT[__GL_MAX_DRAW_BUFFERS];
    GLboolean                   fsRoundingEnabled;
    GLboolean                   drawTexOESEnabled;
    GLboolean                   drawClearRectEnabled;
    gctBOOL                     useFragmentProcessor;

    glsHASHKEY                  hashKey;
    glsHASHTABLE_PTR            hashTable;
    glsPROGRAMINFO_PTR          currProgram;
    gctBOOL                     fixProgramFlag;
    glsATTRIBUTEINFO            attributeInfo[gldMAX_ATTRIBUTES_INFO];
    glsLIGHTING                 lightingStates;
    glsPOINT                    pointStates;

    gcsTEXTURE                  polygonStippleTexture;
    glsTEXTUREINFO              polygonStippleTextureInfo;
    GLubyte                     cachedStipplePattern[4*32];
    GLint                       polygonStippleTextureStage;
    glsTEXTURESAMPLER           polygonStippleSampler;
    GLboolean                   isSolidPolygonStipple;

    gcsTEXTURE                  lineStippleTexture;
    glsTEXTUREINFO              lineStippleTextureInfo;
    GLint                       lineStippleTextureStage;
    glsTEXTURESAMPLER           lineStippleSampler;
    GLboolean                   isSolidLineStipple;
    GLboolean                   programDirty;

    GLboolean                   useAccumBuffer;
    GLint                       accumBufferWidth;
    GLint                       accumBufferHeight;
    GLfloat *                   accumBufferData;

    GLint                       errorNo;
    GLboolean                   multiSampleOn;
    GLboolean                   drawToAccumBuf;
    GLfloat                     accumValue;
    GLfloat                     yMajor;
    gctBOOL                     hwLogicOp;
    glsLOGICOP                  logicOp;
    GLint                       builtinAttributeIndex[_GL_BT_INDEX_MAX];
#endif
};



#define CHIP_CTXINFO(gc) (__GLchipContext *)((gc)->dp.privateData)

extern GLboolean
__glChipCreateContext(
    __GLcontext *gc
);

extern GLboolean
__glChipDestroyContext(
    __GLcontext *gc
    );

extern GLboolean
__glChipMakeCurrent(
    __GLcontext *gc
    );

extern GLboolean
__glChipLoseCurrent(
    __GLcontext *gc,
    GLboolean bkickoffcmd
    );

extern GLboolean
__glChipGetDeviceConstants(
    __GLcontext *gc,
    __GLdeviceConstants *constants
    );

extern GLvoid
__glChipQueryFormatInfo(
    __GLcontext *gc,
    __GLformat drvformat,
    GLint *numSamples,
    GLint *samples,
    GLint bufsize
    );

extern GLvoid
gcChipDetachSurface(
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    gcoSURF *surfList,
    GLuint count
    );

extern GLenum
__glChipGetGraphicsResetStatus(
    __GLcontext *gc
    );

/* chip_drawable.c */
extern GLboolean
__glChipChangeDrawBuffers(
    __GLcontext *gc
    );

extern GLboolean
__glChipChangeReadBuffers(
    __GLcontext *gc
    );

extern GLvoid
__glChipDetachDrawable(
    __GLcontext *gc
    );

extern GLboolean
__glChipUpdateDrawable(
    __GLdrawablePrivate *drawable
    );

extern GLvoid
__glChipDestroyDrawable(
    __GLdrawablePrivate *drawable
    );

/* chip_clear.c */
extern GLboolean
__glChipClearBegin(
    __GLcontext *gc,
    GLbitfield *mask
    );

extern GLboolean
__glChipClearValidateState(
    __GLcontext *gc,
    GLbitfield mask
    );

extern GLboolean
__glChipClearEnd(
    __GLcontext *gc,
    GLbitfield mask
    );

extern GLboolean
__glChipClear(
    __GLcontext *gc,
    GLuint mask
    );

extern GLboolean
__glChipClearBuffer(
    __GLcontext *gc,
    GLenum buffer,
    GLint drawbuffer,
    GLvoid *value,
    GLenum type
    );

extern GLboolean
__glChipClearBufferfi(
    __GLcontext *gc,
    GLfloat depth,
    GLint stencil
    );


/* chip_pixel.c */
extern GLboolean
__glChipReadPixelsBegin(
    __GLcontext *gc
    );

extern GLvoid
__glChipReadPixelsValidateState(
    __GLcontext *gc
    );

extern GLboolean
__glChipReadPixelsEnd(
    __GLcontext *gc
    );

extern GLboolean
__glChipReadPixels(
    __GLcontext *gc,
    GLint x,
    GLint y,
    GLsizei width,
    GLsizei height,
    GLenum format,
    GLenum type,
    GLubyte *buf
    );


/* chip_draw.c */
extern gceSTATUS
gcChipInitializeDraw(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    );

extern gceSTATUS
gcChipValidateRenderTargetState(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    );

extern gceSTATUS
gcChipDeinitializeDraw(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    );

extern GLvoid
gcChipPatchFreeTmpAttibMem(
    __GLcontext *gc
    );

extern GLboolean
__glChipDrawNothing(
    __GLcontext *gc
    );

extern GLboolean
__glChipDrawElements(
    __GLcontext *gc
    );

extern GLboolean
__glChipDrawArrays(
    __GLcontext *gc
    );

extern GLboolean
__glChipDrawElementsInstanced(
    __GLcontext *gc
    );

extern GLboolean
__glChipDrawArraysInstanced(
    __GLcontext *gc
    );

extern GLboolean
__glChipFlush(
    __GLcontext *gc
    );

extern GLboolean
__glChipFinish(
    __GLcontext *gc
    );

#ifdef OPENGL40
extern GLvoid configStream(
    __GLcontext* gc
    );

extern gceSTATUS gcChipValidateGL4Texture(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    );

extern gceSTATUS
gcChipValidateGL4FixState(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    );

extern gceSTATUS
gcChipBindFixSamplers(
    __GLcontext *gc
    );

extern GLboolean
__glChipCopyPixels(
    __GLcontext *gc,
    GLint x,
    GLint y,
    GLsizei width,
    GLsizei height,
    GLenum format
    );

extern GLboolean
__glChipDrawPixels(
    __GLcontext *gc,
    GLsizei width,
    GLsizei height,
    GLenum format,
    GLenum type,
    GLubyte *pixels
    );
#endif

extern GLboolean
__glChipDrawBegin(
    __GLcontext* gc,
    GLenum mode
    );

extern GLboolean
__glChipDrawValidateState(
    __GLcontext *gc
    );

extern GLboolean
__glChipDrawEnd(
    __GLcontext *gc
    );

extern GLboolean
__glChipDrawPattern(
    __GLcontext *gc
    );

extern GLboolean
__glChipDrawArraysIndirect(
    __GLcontext *gc
    );

extern GLboolean
__glChipDrawElementsIndirect(
    __GLcontext *gc
    );

extern GLboolean
__glChipMultiDrawArraysIndirect(
    __GLcontext *gc
    );

extern GLboolean
__glChipMultiDrawElementsIndirect(
    __GLcontext *gc
    );

/* chip_state.c */
extern gceSTATUS
gcChipSetCulling(
    __GLcontext *gc
    );


extern gceSTATUS
gcChipSetColorMask(
    __GLcontext *gc
    );

gceSTATUS
gcChipSetAlphaBlend(
    __GLcontext *gc
    );

#if gcdALPHA_KILL_IN_SHADER
gceSTATUS
gcChipSetAlphaKill(
    __GLcontext *gc
    );
#endif

extern gceSTATUS
gcChipSetViewportScissor(
    __GLcontext *gc
    );

extern gceSTATUS
gcChipSetDrawBuffers(
    __GLcontext  * gc,
    GLuint         integerRT,
    GLuint         floatRT,
    gcsSURF_VIEW  * rtViews,
    gcsSURF_VIEW  * dView,
    gcsSURF_VIEW  * sView,
    GLboolean      drawYInverted,
    GLuint         samples,
    GLboolean      useDefault,
    GLuint         width,
    GLuint         height,
    GLboolean      layered,
    GLuint         maxLayers
    );


extern gceSTATUS
gcChipSetReadBuffers(
    __GLcontext  * gc,
    GLuint         integerRT,
    gcsSURF_VIEW  * rtViews,
    gcsSURF_VIEW  * dView,
    gcsSURF_VIEW  * sView,
    GLboolean      readYInverted,
    GLboolean      layered
    );

extern gceSTATUS
gcChipValidateState(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    );

extern gceSTATUS
gcChipValidateRecompileStateCB(
    __GLcontext *gc,
    __GLprogramObject *progObj,
    __GLchipSLProgram *program,
    __GLSLStage stage
    );

gceSTATUS
gcChipValidateImage(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    );

gceSTATUS
gcChipValidateTexture(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    );

#ifdef OPENGL40
gceSTATUS
gcChipValidateFixProgram(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    );
#endif

gceSTATUS
gcChipValidateShader(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    );


/* chip_depth.c */
extern gceSTATUS
gcChipSetDepthStates(
    __GLcontext *gc,
    GLbitfield localMask
    );

extern gceSTATUS
gcChipSetStencilStates(
    __GLcontext *gc,
    GLbitfield localMask
    );

extern gceSTATUS
gcChipSetDepthMode(
    __GLcontext *gc
    );

extern gceSTATUS
gcChipSetDepthTest(
    __GLcontext *gc
    );

extern gceSTATUS
gcChipSetStencilRef(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    );

extern gceSTATUS
gcChipSetStencilMode(
    __GLcontext *gc
    );

extern gceSTATUS
gcChipSetStencilTest(
    __GLcontext *gc
    );


extern gceSTATUS
gcChipSetDepthRange(
    __GLcontext *gc
    );

extern gceSTATUS
gcChipSetDepthCompareFunction(
    __GLchipContext *chipCtx,
    GLenum testFunction
    );

extern gceSTATUS
gcChipSetDepthMask(
    __GLcontext *gc
    );

extern gceSTATUS
gcChipSetPolygonOffset(
    __GLcontext *gc
    );

/* chip_texture.c */
extern gcsSURF_VIEW
gcChipGetTextureSurface(
    __GLchipContext *chipCtx,
    __GLtextureObject *texObj,
    GLboolean layered,
    GLint level,
    GLint slice
    );

gceSTATUS
gcChipCreateTexture(
    __GLcontext *gc,
    __GLtextureObject *texObj
    );

extern gceSTATUS
gcChipProcessPBO(
    __GLcontext *gc,
    __GLbufferObject *bufObj,
    const GLvoid **pBuf
    );

extern gceSTATUS
gcChipPostProcessPBO(
    __GLcontext *gc,
    __GLbufferObject *bufObj,
    GLboolean isPacked
    );

extern GLvoid
gcChipProcessPixelStore(
    __GLcontext* gc,
    __GLpixelPackMode *packMode,
    gctSIZE_T width,
    gctSIZE_T height,
    gctSIZE_T border,
    GLenum format,
    GLenum type,
    gctSIZE_T skipImgs,
    gctSIZE_T *pRowStride,
    gctSIZE_T *pImgHeight,
    gctSIZE_T *pSkipBytes
    );

/* chip_codec.c */
extern GLvoid*
gcChipDecompressETC1(
    IN  __GLcontext *gc,
    IN  gctSIZE_T Width,
    IN  gctSIZE_T Height,
    IN  gctSIZE_T ImageSize,
    IN  const void * Data,
    OUT gceSURF_FORMAT *Format,
    OUT gctSIZE_T *pRowStride
    );

extern GLvoid*
gcChipDecompressDXT(
    IN  __GLcontext *gc,
    IN  gctSIZE_T Width,
    IN  gctSIZE_T Height,
    IN  gctSIZE_T ImageSize,
    IN  const void *Data,
    IN  GLenum InternalFormat,
    OUT gceSURF_FORMAT *Format,
    OUT gctSIZE_T* pRowStride
    );

extern GLvoid*
gcChipDecompressETC2EAC(
    IN  __GLcontext *gc,
    IN  gctSIZE_T Width,
    IN  gctSIZE_T Height,
    IN  gctSIZE_T ImageSize,
    IN  const void * Data,
    IN  GLenum InternalFormat,
    OUT gceSURF_FORMAT *Format,
    OUT gctSIZE_T *pRowStride
    );

extern GLvoid*
gcChipDecompressRGTC(
    IN  __GLcontext *gc,
    IN  gctSIZE_T Width,
    IN  gctSIZE_T Height,
    IN  gctSIZE_T ImageSize,
    IN  const void *Data,
    IN  GLenum InternalFormat,
    OUT gceSURF_FORMAT *Format,
    OUT gctSIZE_T* pRowStride
    );

extern GLvoid*
gcChipDecompressPalette(
    IN  __GLcontext* gc,
    IN  GLenum InternalFormat,
    IN  gctSIZE_T Width,
    IN  gctSIZE_T Height,
    IN  GLint Level,
    IN  gctSIZE_T ImageSize,
    IN  const void *Data,
    OUT gceSURF_FORMAT *Format,
    OUT gctSIZE_T *pRowStride
    );


extern GLvoid*
gcChipDecompress_EAC_11bitToR16F(
    IN  __GLcontext* gc,
    IN  gctSIZE_T Width,
    IN  gctSIZE_T Height,
    IN  gctSIZE_T Depth,
    IN  gctSIZE_T ImageSize,
    IN  const void * Data,
    IN  GLenum InternalFormat,
    OUT gceSURF_FORMAT *Format,
    OUT gctSIZE_T *pRowStride
    );

extern GLvoid*
gcChipDecompressASTC(
    IN  __GLcontext *gc,
    IN  gctSIZE_T Width,
    IN  gctSIZE_T Height,
    IN  gctSIZE_T numSlices,
    IN  gctSIZE_T compressedSize,
    IN  const void * Data,
    IN  __GLformatInfo *formatInfo,
    OUT gceSURF_FORMAT *Format,
    OUT gctSIZE_T *pRowStride
    );

/* chip_shader.c */
extern gceSTATUS
gcChipLTCReleaseResultArray(
    IN __GLchipContext   * chipCtx,
    IN gcoOS               Os
    );

extern gceSTATUS
gcChipSetError(
    __GLchipContext *chipCtx,
    gceSTATUS errorStatus
    );

extern GLenum
__glChipGetError(
    __GLcontext *gc
    );

#if VIVANTE_PROFILER
GLboolean
__glChipProfilerSet(
    IN __GLcontext *gc,
    IN GLuint Enum,
    IN gctHANDLE Value
    );

gceSTATUS
gcChipProfilerInitialize(
    IN __GLcontext *gc
    );

gceSTATUS
gcChipProfilerDestroy(
    IN __GLcontext *gc
    );

gceSTATUS
gcChipProfilerWrite(
    IN __GLcontext *gc,
    IN GLuint Enum
    );

GLvoid
gcChipInitProfileDevicePipeline(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    );
#endif

GLboolean
__glChipComputeBegin(
    __GLcontext *gc
    );

GLboolean
__glChipComputeValidateState(
    __GLcontext *gc
    );

GLvoid
__glChipComputeEnd(
    __GLcontext *gc
    );

GLboolean
__glChipDispatchCompute(
    __GLcontext *gc
    );

typedef
gceSTATUS
(*programStageActionCallBackFunc)(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    __GLchipSLProgram *program,
    __GLSLStage stage
    );

gceSTATUS
gcChipTraverseProgramStages(
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    programStageActionCallBackFunc callback
    );

gceLAYOUT_QUALIFIER
gcChipUtilConvertLayoutQualifier(
    GLenum blendEquation,
    GLboolean *blendInShader
    );

gceSTATUS
gcChipLockOutComputeIndirectBuf(
    __GLcontext *gc
    );

#ifdef OPENGL40
extern GLboolean
    __glChipAccum(
    __GLcontext* gc,
    GLenum op,
    GLfloat value);


extern GLvoid
    __glChipCreateAccumBufferInfo(__GLcontext* gc,
    gcoSURF accumSurf,
    __GLdrawablePrivate *glDrawable);

extern gceSTATUS
    gcChipclearAccumBuffer(__GLcontext* gc,
    glsCHIPACCUMBUFFER *chipAccumBuffer);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __chip_context_h__ */
