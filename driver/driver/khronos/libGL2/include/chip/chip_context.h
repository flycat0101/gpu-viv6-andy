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


#ifndef __chip_context_h_
#define __chip_context_h_
#include "gc_hal.h"
#include "gc_hal_raster.h"
#include "gc_hal_engine.h"
#include "gc_vsc_drvi_interface.h"
#include "gc_hal_user_math.h"
#include "gc_hal_user_os_memory.h"
#include "chip_basic_types.h"
#include "chip_basic_defines.h"
#include "chip_hash.h"
#include "chip_swapbuffer.h"
#include "chip_texture.h"
#include "chip_buffer.h"
#include "chip_drawable.h"
#include "chip_state.h"
#include "chip_device.h"
#include "chip_draw.h"
#include "chip_glsh.h"
#include "chip_vbo.h"
#include "chip_fbo.h"
#include "chip_pixel.h"

#ifdef __cplusplus
extern "C" {
#endif

#define __GL_VENDOR "Vivante Corporation"
#define __GL_VERSION20 "2.0"" "VER_PRODUCTVERSION_BUILD_VIV
#define __GL_VERSION21 "2.1"" "VER_PRODUCTVERSION_BUILD_VIV
#define __GL_RENDERER_VIV   "Vivante Corporation GCCORE"
#define __GL_GLSL_VERSION   "1.20"

#define __GL_MAX_VS_CONSTANT          256
#define __GL_MAX_PS_CONSTANT          256

#define __GL_MAX_TEXTURE_BLENDERS     8
#define __GL_MAX_SIMU_TEXTURE         8
#define __GL_MAX_HW_LIGHTS            8
#define __GL_MAX_ACTIVE_LIGHTS        8
#define __GL_MAX_BLEND_WEIGHTS        4
#define __GL_MAX_WORLD_MATRICES       0  /* Not Support Index Vertex Blending */
#define __GL_MAX_USER_CLIPPLANES      6
#define __GL_MAX_POINTSPRITE_SIZE     64.0f
#define __GL_MAX_HW_LINE_WIDTH        128.0
#define __GL_MAX_PRIMITIVE_COUNT      0x15555
#define __GL_MAX_HW_ARRAY_SIZE        512
#define __GL_MAX_HW_TEXTURE_BUFFER_SIZE   128*1024*1024
#define __GL_DIRTIED_TEX_LIST_SIZE 4096

/* vertex/fragment program */
#define __GL_MAX_VERTEX_PROGRAM_INSTS           320
#define __GL_MAX_FRAGMENT_PROGRAM_INSTS         128
#define __GL_MAX_VERTEX_PROGRAM_CONSTANTS       256
#define __GL_MAX_FRAGMENT_PROGRAM_CONSTANTS     224
#define __GL_MAX_VERTEX_PROGRAM_ATTRIBS         16
#define __GL_MAX_FRAGMENT_PROGRAM_ATTRIBS       16
#define __GL_MAX_VERTEX_PROGRAM_TEMPREGS        32
#define __GL_MAX_FRAGMENT_PROGRAM_TEMPREGS      32
#define __GL_MAX_VERTEX_PROGRAM_ADDRREGS        1
#define __GL_MAX_FRAGMENT_PROGRAM_ADDRREGS      1
#define __GL_MAX_VERTEX_PROGRAM_LOCALPARAMS     96
#define __GL_MAX_FRAGMENT_PROGRAM_LOCALPARAMS   24
#define __GL_MAX_VERTEX_PROGRAM_ENVPARAMS       256
#define __GL_MAX_FRAGMENT_PROGRAM_ENVPARAMS     24

#define __GL_MAX_PS_INSTS                       (__GL_MAX_FRAGMENT_PROGRAM_INSTS)
#define __GL_MAX_PS_TEXREGS                     40

/* bindable uniform */
#define __GL_MAX_BINDABLE_UNIFORM_SIZE          (4096*4)
#define __GL_MAX_VERTEX_BINDABLE_UNIFORMS       13
#define __GL_MAX_FRAGMENT_BINDABLE_UNIFORMS     (__GL_MAX_VERTEX_BINDABLE_UNIFORMS)
#define __GL_MAX_GEOMETRY_BINDABLE_UNIFORMS     (__GL_MAX_VERTEX_BINDABLE_UNIFORMS)

#define __GL_MIN_PROGRAM_TEXEL_OFFSET           (-8)
#define __GL_MAX_PROGRAM_TEXEL_OFFSET           7

/* Geometry extension */
#define __GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS       16
#define __GL_MAX_GEOMETRY_VARYING_COMPONENTS        (1024-4)   /* exclude gl_Position */
#define __GL_MAX_VERTEX_VARYING_COMPONENTS          (16*6*4)
#define __GL_MAX_VARYING_COMPONENTS                 ((16-1)*4) /* exclude gl_Position */
#define __GL_MAX_GEOMETRY_UNIFORM_COMPONENTS        (1024*16*4)
#define __GL_MAX_GEOMETRY_OUTPUT_VERTICES           1024
#define __GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS   1024

#define __GL_MAX_DRAW_BUFFERS       8
#define __GL_MAX_READ_BUFFERS       1

/* query counter bits */
#define __GL_QUERY_COUNTER_BITS    64

#define __GL_MAX_VIEWPORT_WIDTH         8192
#define __GL_MAX_VIEWPORT_HEIGHT        8192

#define gldATTRIBUTE_DRAWTEX_POSITION       16
#define gldATTRIBUTE_DRAWCLEAR_POSITION     17
#define gldMAX_ATTRIBUTES_INFO              18
#define gldATTRIBUTE_POINT_SIZE             5
#define gldATTRIBUTE_INDEX                  10

#define gldCHIP_NAME_LEN 23

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

typedef struct _glsCHIPCONTEXT * glsCHIPCONTEXT_PTR;

struct _glsCHIPCONTEXT {
    gcoHAL                      hal;
    gco3D                       hw;
    gcoOS                       os;

    gceCHIPMODEL                chipModel;
    gctUINT32                   chipRevision;
    GLchar                      chipName[gldCHIP_NAME_LEN];

    /* GLSL Compiler. */
    gctHANDLE                   dll;
    gceSTATUS                   (*compiler)(IN gcoHAL Hal,
                                    IN gctINT ShaderType,
                                    IN gctSIZE_T SourceSize,
                                    IN gctCONST_STRING Source,
                                    OUT gcSHADER * Binary,
                                    OUT gctSTRING * Log);

    GLProgram                   currGLSLProgram;
    gctINT                      maxAttributes;
    GLboolean                   batchDirty;

    glsLOGICOP                  logicOp;
    glsPOINT                    pointStates;
    GLboolean                   dither;

    glsHASHKEY                  hashKey;
    glsHASHTABLE_PTR            hashTable;
    glsPROGRAMINFO_PTR          currProgram;

    /* Miscellaneous flags. */
    GLboolean                   fsRoundingEnabled;
    GLboolean                   drawTexOESEnabled;
    GLboolean                   drawClearRectEnabled;
    gctBOOL                     useFragmentProcessor;
    gctBOOL                     hasCorrectStencil;
    gctBOOL                     hasTileStatus;
    gctBOOL                     hwLogicOp;
    gctBOOL                     hwPointSprite;
    gctBOOL                     renderToTexture;
    gctBOOL                     hasTxDescriptor;
    gctBOOL                     hasBlitEngine;

    /* Temporary bitmap. */
    gcoSURF                     tempBitmap;
    gctUINT8_PTR                tempBits;
    gctUINT8_PTR                tempLastLine;
    gceSURF_FORMAT              tempFormat;
    gctUINT                     tempBitsPerPixel;
    gctUINT                     tempX;
    gctUINT                     tempY;
    gctUINT                     tempWidth;
    gctUINT                     tempHeight;
    gctINT                      tempStride;

    gctBOOL                     initialized;

    gcoSURF                     tempDraw;

    glsATTRIBUTEINFO            attributeInfo[gldMAX_ATTRIBUTES_INFO];

    glsTEXTURE                  texture;

    glsLIGHTING                 lightingStates;

    /* Information for draw buffer setting */
    GLboolean                   drawInteger;
    GLboolean                   drawFloat;
    GLboolean                   drawInvertY;
    GLuint                      drawRTWidth;
    GLuint                      drawRTHeight;
    GLuint                      drawRTMsaaMode;
    gcoSURF                     drawRT[__GL_MAX_DRAW_BUFFERS];
    gcoSURF                     drawDepth;
    gcoSURF                     drawStencil;
    GLuint                      rtEnableMask;
    GLubyte                     colorWriteMask[__GL_MAX_DRAW_BUFFERS];

    /* Read RT information */
    GLboolean                   readInteger;
    GLboolean                   readInvertY;
    GLuint                      readRTWidth;
    GLuint                      readRTHeight;
    GLuint                      readRTMsaaMode;
    gcoSURF                     readRT;
    gcoSURF                     readDepth;
    gcoSURF                     readStencil;

    gcsVERTEXARRAY              attributeArray[gldMAX_ATTRIBUTES_INFO];
    gcoVERTEXARRAY              vertexArray;

    GLint                       lastPrimitiveType;
    GLboolean                   programDirty;

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

    GLint       errorNo;
    GLboolean   multiSampleOn;
    GLboolean   drawToAccumBuf;
    GLfloat     accumValue;
    GLuint     samplerDirty;
    GLint     builtinAttributeIndex[_GL_BT_INDEX_MAX];
} glsCHIPCONTEXT;

#define CHIP_CTXINFO(gc) (glsCHIPCONTEXT_PTR)((gc)->dp.ctx.privateData)

extern GLint __glChipDeviceConfigChangeEnter(GLvoid);
extern GLint __glChipDeviceConfigChangeExit(__GLcontext *gc);
extern GLvoid __glChipDeviceConfigurationChanged(__GLcontext * gc);
extern GLint __glChipUpdateDefaultVB(__GLcontext *gc);
extern GLvoid __glChipCreateContext(__GLcontext *gc);
extern GLuint __glChipDestroyContext(__GLcontext *gc);
extern GLuint __glChipMakeCurrent(__GLcontext *gc);
extern GLuint __glChipLoseCurrent(__GLcontext *gc, GLboolean bkickoffcmd);
extern GLvoid __glChipGetDeviceConstants(__GLdeviceConstants *constants);
extern GLuint __glChipQueryDeviceFormat(GLenum internalFormat,GLboolean generateMipmap,GLint samples);
extern GLvoid __glChipCreatePbuffer(__GLcontext *gc, __GLdrawablePrivate * draw);
extern GLvoid __glChipCreateDrawable(__GLdrawablePrivate *draw,GLvoid *window);
extern GLboolean createWorkThread(glsDEVICEPIPELINEGLOBAL_PTR deviceGlobal);
extern GLvoid destroyWorkThread(glsDEVICEPIPELINEGLOBAL_PTR deviceGlobal);
extern GLboolean calculateArea(
    GLint *pdx, GLint *pdy,
    GLint *psx, GLint *psy,
    GLint *pw, GLint *ph,
    GLint dstW, GLint dstH,
    GLint srcW, GLint srcH
    );

#ifdef __cplusplus
}
#endif

#endif /* __chip_context_h_ */
