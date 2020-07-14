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


#ifndef __gc_es_shader_h__
#define __gc_es_shader_h__

/************************************************************************/
/* enums                                                                */
/************************************************************************/
typedef enum
{
    __GL_SHADER_OBJECT_TYPE = 0,
    __GL_PROGRAM_OBJECT_TYPE,
    __GL_SHADER_PROGRAM_TYPE_LAST
} GL_SHADER_PROGRAM_TYPE;

/************************************************************************/
/* Definitions for uniform                                              */
/************************************************************************/

/* The defined max samplers must be larger than or equal to HW supported vs+ps samplers*/
#define __GL_MAX_GLSL_SAMPLERS 80

/* The defined max image uniform must be euqal to or larger than maxCombinedImageUniform */
#define __GL_MAX_GLSL_IMAGE_UNIFORM 16

/*
** Because SM4.0 think TRUE as 0xffffffff, we need transfer common TRUE value
** (none zero) to 0xfffffff, Otherwise we may get unexpected result when shader
** code use bit operation on bool value.
*/
#define __GLSL_TRUE     0xffffffff
#define __GLSL_FALSE    0x0

#define __GL_MAX_PRGOBJ_LINEAR_TABLE_SIZE           1024
#define __GL_DEFAULT_PRGOBJ_LINEAR_TABLE_SIZE       256
#define __GL_PRGOBJ_HASH_TABLE_SIZE                 512


#define __GLSL_LOG_INFO_SIZE  512

/************************************************************************/
/* Shader and Program Object Structures                                 */
/************************************************************************/

typedef struct __GLshPrgObjInfoRec
{
    /* Indicate how many targets the object is currently bound to
    */
    GLuint bindCount;

    /* The seqNumber is increased by 1 whenever program string is changed.
    ** DP must recompile the program string if its internal copy of
    ** savedSeqNum is different from this seqNumber.
    */
    GLuint seqNumber;

    /* Internal flag for generic object management. */
    GLbitfield flag;

    GL_SHADER_PROGRAM_TYPE objectType;
    GLuint id;

    GLchar *label;
} __GLshPrgObjInfo;

typedef struct __GLshaderInfoRec
{
    GLuint    shaderType;
    GLboolean deleteStatus;
    GLboolean compiledStatus;
    GLchar   *compiledLog;
    GLchar   *source;
    GLsizei   sourceSize;
    GLvoid   *hBinary;          /* the pre-link shader binary can be
                                   used to re-compilation on */
} __GLshaderInfo;


/* Shader object holds vertex or fragment shader related information */
typedef struct __GLshaderObjectRec
{
    /* Generic object information */
    __GLshPrgObjInfo objectInfo;

    __GLshaderInfo shaderInfo;

    /* This is the object privateData that can be shared by different contexts
    ** that are bound to the object.
    */
    GLvoid *privateData;

    /* These are vertex shader only states */
    GLboolean twoSidedColorEnabled;
    GLboolean pointSizeEnabled;

} __GLshaderObject;


#define __GL_INVALID_TEX_BIT    (1 << 0)
#define __GL_INVALID_LINK_BIT   (1 << 1)


typedef struct __GLprogramInfoRec
{
    GLboolean deleteStatus;
    GLboolean linkedStatus;
    GLboolean validateStatus;
    GLuint    invalidFlags;
    GLchar  * infoLog;

    __GLshaderObject *attachedShader[__GLSL_STAGE_LAST];

    GLboolean retrievable;
    GLboolean separable;

    GLuint uniqueId;    /* uniqueId to differ from other program, no matter live or deleted. */
    GLuint codeSeq;     /* used for object share */
} __GLprogramInfo;

typedef struct __GLbindingInfoRec
{
    GLboolean isRetrievable;
    GLboolean isSeparable;
    GLuint activeShaderID[__GLSL_STAGE_LAST];

    /* Input */
    GLuint numActiveInput;
    GLuint maxInputNameLen;
    /*
    **    vs input array mask will be built from generic vertex attribute array index.
    **    Actually, this mask means generic vertex attribute array index.
    */
    GLuint vsInputArrayMask;

    /* Output */
    GLuint numActiveOutput;
    GLuint maxOutputNameLen;

    /* Uniform */
    GLuint numActiveUniform;
    GLuint maxUniformNameLen;

    /* Uniform Block */
    GLuint numActiveUB;
    GLuint maxUBNameLen;
    GLint  maxActiveUniforms;

    /* Transform feedback info after link */
    GLenum xfbMode;
    GLuint numActiveXFB;
    GLuint maxXFBNameLen;

    /* Atomic Counter Buffer */
    GLuint numActiveACBs;
    GLint  maxActiveACs;

    /* Buffer Variable */
    GLuint numActiveBV;
    GLuint maxBVNameLen;

    /* Shader Storage Block */
    GLuint numActiveSSB;
    GLuint maxSSBNameLen;
    GLuint maxActiveBVs;

    /* Compute shader work group size */
    GLuint workGroupSize[3];

    /* TESS */
    GLuint tessOutPatchSize;
    GLenum tessGenMode;
    GLenum tessSpacing;
    GLenum tessVertexOrder;
    GLboolean tessPointMode;
    GLenum tessPrimitiveMode;

    /* GS */
    GLuint gsOutVertices;
    GLenum gsInputType;
    GLenum gsOutputType;
    GLuint gsInvocationCount;

} __GLbindingInfo;

/* Program object holds program related information */
typedef struct __GLprogramObjectRec
{
    /* Generic object information */
    __GLshPrgObjInfo objectInfo;

    /* Define the program related states information */
    __GLprogramInfo programInfo;

    /* Output binding tables of the program after link */
    __GLbindingInfo bindingInfo;

    /*
    ** Samplers
    */
    GLuint64 samplerSeq;

    /*
    ** Transform Feedback info
    */
    GLuint xfbRefCount;
    /* The 3 variable was specified before link */
    GLenum xfbMode;
    GLuint xfbVaryingNum;
    GLchar **ppXfbVaryingNames;

    GLuint maxSampler;
    GLuint maxUnit;

    GLvoid *privateData;
} __GLprogramObject;

/* Program Pipeline Object */
typedef struct __GLprogramPipelineObjectRec
{
    GLuint             name;

    __GLprogramObject *activeProg;
    __GLprogramObject *stageProgs[__GLSL_STAGE_LAST];

    GLboolean          validateStatus;
    GLchar            *infoLog;

    GLchar *label;
} __GLprogramPipelineObject;

typedef struct __GLtexUnit2SamplerRec
{
    GLuint numSamplers;
    GLuint samplers[__GL_MAX_GLSL_SAMPLERS];
} __GLtexUnit2Sampler;

typedef enum __GLSLmodeRec
{
    __GLSL_MODE_GRAPHICS = 0x0,
    __GLSL_MODE_COMPUTE  = 0x1,
}__GLSLmode;


typedef struct __GLshaderProgramMachineRec
{
    /* Shader and program objects can be shared between contexts, and
    ** shader and program objects share the same name space according to the Spec.
    */
    __GLsharedObjectMachine    *spShared;

    /* Program Pipeline Objects are containers and can not be shared between contexts. */
    __GLsharedObjectMachine    *ppNoShare;

    /* Current executable program object set by glUseProgram */
    __GLprogramObject          *currentProgram;

    /* Current bound program pipeline objects */
    __GLprogramPipelineObject  *boundPPO;

    /* Active stage programs determined at draw time.
    ** Attention: NOT to access them outside draw/dispatch APIs, since the program may be
    **            deleted, while this field have no chance to be reset and left wild.
    */
    __GLprogramObject          *activeProgObjs[__GLSL_STAGE_LAST];

    /* Dirty flag to indicate which sampler's mapped tex unit was changed. Used when validate samplers. */
    __GLbitmask                 samplerMapDirty;

    /* Dirty flag to indicate whether state of the sampler mapped tex unit was changed. Build at draw time */
    __GLbitmask                 samplerStateDirty;

    /* Dirty flag to indicate which sampler needs keep dirty state. Build at draw time */
    __GLbitmask                 samplerStateKeepDirty;

    /* Dirty flag to indicate which sampler used in texelFetch and related builtin function */
    __GLbitmask                 samplerTexelFetchDirty;
    /* Record the dirty flag of previous program*/
    __GLbitmask                 samplerPrevTexelFetchDirty;

    /* mapping table from texture unit to sampler index, built at draw time */
    __GLtexUnit2Sampler         texUnit2Sampler[__GL_MAX_TEXTURE_UNITS];


    /* When two or more contexts share the same program object, used for setting
    ** sampler dirty bits in each context.
    */
    GLuint64                    samplerSeq;

    /* Record commit state for validate */
    __GLprogramObject          *lastProgObjs[__GLSL_STAGE_LAST];
    GLuint                      lastCodeSeqs[__GLSL_STAGE_LAST];

    /* Graphics mode or compute mode */
    __GLSLmode                  mode;

    /* Input patch vertex number for TCS input */
    GLint                   patchVertices;

    GLuint                  maxSampler;
    GLuint                  maxUnit;

} __GLshaderProgramMachine;


/************************************************************************/
/* Transform feedback                                                   */
/************************************************************************/

#define __GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS  64
#define __GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS     64
#define __GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS        4

#define __GL_MAX_XFBOBJ_LINEAR_TABLE_SIZE           1024
#define __GL_DEFAULT_XFBOBJ_LINEAR_TABLE_SIZE       256
#define __GL_XFB_HASH_TABLE_SIZE                    512

typedef struct __GLxfbObjectRec
{
    GLuint    name;
    GLboolean active;
    GLboolean paused;
    GLenum    primMode;

    /* Offset in vertices to write from beginning of each bound buffer */
    GLuint    offset;
    /* Number of vertices the current draw will output to each bound buffer */
    GLuint    vertices;

    /* When the XFB is active, which program it used */
    __GLprogramObject *programObj;

    GLbitfield flags;

    __GLbufferObject *boundBufObj;
    GLuint  boundBufName;
    __GLBufBindPoint  boundBufBinding[__GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS];

    GLchar *label;

    GLvoid *privateData;
} __GLxfbObject;


enum
{
    __GL_XFB_DIRTY_OBJECT   = 1 << 0,
};

typedef struct __GLxfbMachineRec
{
    /* Transform feedback objects are not shared between contexts according to the Spec.
    ** we just use the __GLsharedObjectMachine to manage the transform feedback objects.
    */
    __GLsharedObjectMachine *noShare;
    __GLxfbObject defaultXfbObj;
    __GLxfbObject *boundXfbObj;

    GLuint  dirtyState;

} __GLxfbMachine;


/**************************************************************************/
/* Compute state                                                          */
/**************************************************************************/
typedef struct __GLcomputeMachineRec
{
    GLuint num_groups_x;
    GLuint num_groups_y;
    GLuint num_groups_z;

    GLboolean indirect;
    GLintptr  offset;
} __GLcomputeMachine;

typedef struct __GLcomputeIndirectCmdRec
{
    GLuint num_groups_x;
    GLuint num_groups_y;
    GLuint num_groups_z;
}__GLcomputeIndirectCmd;


#endif /* __gc_es_shader_h__ */
