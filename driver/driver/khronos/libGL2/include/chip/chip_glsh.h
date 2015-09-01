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


#ifndef __chip_glsl_h_
#define __chip_glsl_h_

typedef struct _GLShader *          GLShader;
typedef struct _GLProgram *         GLProgram;
typedef struct _GLAttribute         GLAttribute;
typedef struct _GLUniform *         GLUniform;
typedef struct _GLBinding *         GLBinding;

typedef enum _GLObjectType
{
    GLObject_Unknown,
    GLObject_VertexShader,
    GLObject_FragmentShader,
    GLObject_Program,
    GLObject_Texture,
    GLObject_Buffer,
    GLObject_Framebuffer,
    GLObject_Renderbuffer,
    GLObject_VertexArray,
} GLObjectType;

struct _GLBinding
{
    GLBinding       next;

    char *          name;

    GLint           index;
};

struct _GLAttribute
{
    GLboolean       enable;
    GLint           size;
    GLenum          type;
    GLboolean       normalized;
    GLsizei         stride;
    const void *    ptr;
    glsVERTEXBUFFERINFO     *buffer;
    GLsizeiptr      offset;
};

struct _GLShader
{
    GLsizei             sourceSize;
    char *              source;

    GLboolean           compileStatus;
    char *              compileLog;
    gcSHADER            binary;
    GLboolean           dirty;

    GLint               usageCount;
    GLboolean           flagged;
};

struct _GLUniform
{
    gcUNIFORM           uniform[2];

    GLvoid *            data;

    GLint               uniformID;

    GLboolean           dirty;
};

typedef struct _GLLocation
{
    gcATTRIBUTE         attribute;
    GLint               index;
    GLboolean           assigned;
}
GLLocation;

struct _GLProgram
{
    gcSHADER            vertexShader;
    gcSHADER            fragmentShader;

    gctSIZE_T           statesSize;
    gctPOINTER          states;
    gcsHINT_PTR         hints;

    gctSIZE_T           attributeCount;
    gctSIZE_T           attributeMaxLength;
    gcATTRIBUTE *       attributePointers;

    GLBinding           attributeBinding;
    GLuint *            attributeLinkage;
    GLLocation *        attributeLocation;
    GLuint              attributeEnable;
    GLuint *            attributeMap;

    GLint               vertexCount;
    GLsizei             uniformMaxLength;

    GLint               uniformCount;
    GLUniform           uniforms;
    GLint               builtInUniformCount;
    GLUniform           builtInUniforms;

    GLuint              vertexSamplers;
    GLuint              fragmentSamplers;
    struct
    {
        GLint   shaderType;
        gcSHADER_TYPE   type;
        GLuint          unit;
    } sampleMap[32];
    GLboolean           valid;
    GLuint              codeSeq;

#if VIVANTE_PROFILER
    gcSHADER_PROFILER   vertexShaderProfiler;
    gcSHADER_PROFILER   fragmentShaderProfiler;
#endif

    GLuint              samplerDirty;
};

extern GLboolean __glChipCompileShader(__GLcontext * gc, __GLshaderObject* shaderObject);
extern GLvoid __glChipDeleteShader(__GLcontext * gc, __GLshaderObject* shaderObject);
extern GLvoid __glChipDeleteShaderProgram(__GLcontext * gc, GLvoid ** programObject);
extern GLboolean __glChipLinkProgram(__GLcontext * gc, __GLshaderProgramObject* programObject);
extern GLboolean __glChipUseProgram(__GLcontext * gc, __GLshaderProgramObject* programObject, GLboolean *);
extern GLint __glChipGetAttributeLocation(__GLcontext *gc, __GLshaderProgramObject *programObject, const GLchar * name);
extern GLboolean __glChipBindAttributeLocation(__GLcontext *gc, __GLshaderProgramObject *programObject, GLuint index, const GLchar * name);
extern GLvoid __glChipGetActiveUniform(__GLcontext *gc, __GLshaderProgramObject *programObject,
    GLuint index,
    GLsizei bufsize,
    GLsizei * length,
    GLint * size,
    GLenum * type,
    char * name);
extern GLboolean __glChipGetUniformLocation(__GLcontext *gc, __GLshaderProgramObject *programObject,
                                  const GLchar *name, GLuint nameLen, GLuint arrayIdx,
                                  GLboolean bArray, GLint *location);

extern GLboolean __glChipValidateProgram(__GLcontext *gc, __GLshaderProgramObject *programObject);

extern GLint __glChipUniforms(
    __GLcontext *gc,
    __GLshaderProgramObject *programObject,
    GLint Location,
    GLint Type,
    GLsizei Count,
    const void * Values,
    GLboolean Transpose
    );

extern GLboolean __glChipGetUniforms(
    __GLcontext *gc,
    __GLshaderProgramObject *programObject,
    GLint Location,
    GLint Type,
    GLvoid * Values
    );
extern GLvoid __glChipBuildTextureEnableDim(__GLcontext *gc);
extern GLboolean __glChipCheckTextureConflict(__GLcontext *gc, __GLshaderProgramObject *programObject);

#endif /* __chip_glsl_h_ */
