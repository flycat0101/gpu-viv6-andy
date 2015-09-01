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


#ifndef _SHADER_COMPILER_INTERFACE_INCLUDE
#define _SHADER_COMPILER_INTERFACE_INCLUDE

#include "gc_gl_bindtable.h"

#ifdef _WIN32

#define C_DECL __cdecl
#ifdef SH_EXPORTING
#define SH_IMPORT_EXPORT __declspec(dllexport)
#else
#define SH_IMPORT_EXPORT __declspec(dllimport)
#endif

#else

#define C_DECL
#define SH_IMPORT_EXPORT

#endif



#ifdef __cplusplus
extern "C" {
#endif

// The opaque handle for GLSL program and shader object.
typedef ULONG_PTR GLSLHandle;

#define GLSLNull     0

typedef struct _GLSLState
{
    //    GLint maxLights;
    //    GLint maxClipPlanes;
    //    GLint maxTextureUnits;
    GLint maxTextureCoords;
    GLint maxVertexAttributes;
    //    GLint maxVertexUniformComponents;
    //    GLint maxVaryingFloats;
    //    GLint maxVertexTextureImageUnits;
    //    GLint maxCombinedTextureImageUnits;
    //    GLint maxTextureImageUnits;
    //    GLint maxFragmentUniformComponents;
    GLint maxDrawBuffers;
    GLint minProgramTexelOffset;
    GLint maxProgramTexelOffset;
}GLSLSTATE;

// GS Program Parameter
typedef enum _GLSLProgramParameterType{
    E_GLSL_GEOMETRY_VERTICES_OUT,
    E_GLSL_GEOMETRY_INPUT_TYPE,
    E_GLSL_GEOMETRY_OUTPUT_TYPE
}EGLSLProgramParameterType;

// GS Input | Output Primtive Type
typedef enum _GLSLPrimtiveType{
    E_GLSL_POINTS,
    E_GLSL_LINES,
    E_GLSL_LINE_STRIP,
    E_GLSL_LINES_ADJ,
    E_GLSL_TRIANGLES,
    E_GLSL_TRIANGLE_STRIP,
    E_GLSL_TRIANGLES_ADJ,
}EGLSLPrimtiveType;

typedef enum _GLSLShaderType{
    E_GLSL_VERTEX_SHADER,
    E_GLSL_GEOMETRY_SHADER,
    E_GLSL_FRAGMENT_SHADER,
}EGLSLShaderType;

// GLSL errors.
typedef enum _EGLSLError
{
    // No error.
    E_GLSL_NO_ERROR = 0,

    // General error.
    E_GLSL_GENERAL_ERROR,

    // Comiling errors.
    E_GLSL_GENERAL_COMPILING_ERROR,
    E_GLSL_NO_SHADER_SOURCE,
    E_GLSL_NO_COMPILER_AVAILABLE,

    // Linking errors.
    E_GLSL_GENERAL_LINKING_ERROR,
    E_GLSL_NO_SHADER_ATTACHED,
    E_GLSL_NOT_ALL_SHADER_COMPILED,

    // Misc errors.
    E_GLSL_INVALID_OPERATION,
    E_GLSL_INVALID_PARAMETER,
    E_GLSL_NOT_A_SUCCESSFUL_LINK,
    E_GLSL_NO_SUCH_SAMPLER,
}EGLSLError;

// EGLSLBindingType : GLSL binding types.
typedef enum _EGLSLBindingType
{
    E_BT_ATTRIBUTE,
    E_BT_VERTEX_VARYING,
    E_BT_GEOMETRY_VARYING_IN,
    E_BT_GEOMETRY_VARYING_OUT,
    E_BT_FRAGMENT_VARYING,
    E_BT_FRAGMENT_VARYING_OUT,
    E_BT_UNIFORM,
    E_BT_SAMPLER,
    E_BT_FORCE_DWORD = 0x7FFFFFFF
}EGLSLBindingType;

// EGLSLExecutableType : the type of a GLSL executable.
typedef enum _EGLSLExecutableType
{
    E_ET_NOT_PRESENT,
    E_ET_VS30,
    E_ET_PS30,
    E_ET_VS40,
    E_ET_PS40,
    E_ET_FORCE_DWORD = 0x7FFFFFFF
}EGLSLExecutableType;

// SGLSLExecutable : the executable for certain GLSL program.
typedef struct _SGLSLExecutable
{
    EGLSLExecutableType eExecutableType;
    GLuint        uExecutableSize;
    unsigned long*      pdwExecutable;
}SGLSLExecutable;

//
// Interface function prototypes.
//

// Initialization and finalization.
SH_IMPORT_EXPORT GLint            C_DECL  OGL_Compiler_Initialize (GLSLSTATE* glslState);
SH_IMPORT_EXPORT GLint            C_DECL  OGL_Compiler_Finalize ();

// Object related functions
SH_IMPORT_EXPORT GLSLHandle     C_DECL  OGL_Compiler_CreateShader (EGLSLShaderType eShaderType);
SH_IMPORT_EXPORT GLint            C_DECL  OGL_Compiler_DeleteShader (GLSLHandle hShader);
SH_IMPORT_EXPORT GLint            C_DECL  OGL_Compiler_SetShaderSource (GLSLHandle hShader, GLbyte * pszShaderSource);
SH_IMPORT_EXPORT const GLbyte*    C_DECL  OGL_Compiler_GetShaderSource (GLSLHandle hShader);
SH_IMPORT_EXPORT GLuint   C_DECL  OGL_Compiler_GetShaderSourceLength(GLSLHandle hShader);
SH_IMPORT_EXPORT GLint            C_DECL  OGL_Compiler_CompileShader (GLSLHandle hShader);

SH_IMPORT_EXPORT GLSLHandle     C_DECL  OGL_Compiler_CreateProgram ();
SH_IMPORT_EXPORT GLint            C_DECL  OGL_Compiler_DeleteProgram (GLSLHandle hProgram);
SH_IMPORT_EXPORT GLint            C_DECL  OGL_Compiler_ProgramAttachShader (GLSLHandle hProgram, GLSLHandle hShader);

SH_IMPORT_EXPORT GLint            C_DECL  OGL_Compiler_ProgramDetachShader (GLSLHandle hProgram, GLSLHandle hShader);
SH_IMPORT_EXPORT GLuint   C_DECL  OGL_Compiler_ProgramGetNumAttachedShaders (GLSLHandle hProgram);
SH_IMPORT_EXPORT GLint            C_DECL  OGL_Compiler_ProgramGetAttachedShaders (GLSLHandle hProgram, GLuint uMaxCount,
                                                                        GLuint* puActualCount, GLSLHandle * hAttachedShaders);
SH_IMPORT_EXPORT GLint            C_DECL  OGL_Compiler_LinkProgram (GLSLHandle hProgram);
SH_IMPORT_EXPORT GLint            C_DECL  OGL_Compiler_ValidateProgram (GLSLHandle hProgram);
SH_IMPORT_EXPORT GLint            C_DECL  OGL_Compiler_ProgramParameteri (GLSLHandle hProgram, EGLSLProgramParameterType panme, GLint value);


SH_IMPORT_EXPORT EGLSLError     C_DECL  OGL_Compiler_GetLastError (GLSLHandle hProgramOrShader);
SH_IMPORT_EXPORT GLvoid           C_DECL  OGL_Compiler_SetInfoLog (GLSLHandle hProgramOrShader, const GLbyte* info);
SH_IMPORT_EXPORT const GLbyte *   C_DECL  OGL_Compiler_GetInfoLog (GLSLHandle hProgramOrShader);
SH_IMPORT_EXPORT GLuint   C_DECL  OGL_Compiler_GetInfoLogLength(GLSLHandle hProgramOrShader);


// Attribute related functions.
SH_IMPORT_EXPORT GLint            C_DECL  OGL_Compiler_BindAttribLocation (GLSLHandle hProgram,
                                                                 GLuint uIndex,
                                                                 const GLbyte* pszAttribName);

// Fragment data related functions.
SH_IMPORT_EXPORT GLint            C_DECL  OGL_Compiler_BindFragDataLocation (GLSLHandle hProgram,
                                                                           GLuint colorNumber,
                                                                           const GLbyte* pszFragDataName);


// Binding table related functions.
SH_IMPORT_EXPORT GLuint   C_DECL  OGL_Compiler_GetNumBindings (GLSLHandle hProgram,
                                                             EGLSLBindingType eBindingType);
SH_IMPORT_EXPORT GLuint   C_DECL  OGL_Compiler_GetBindingTable (GLSLHandle hProgram,
                                                              EGLSLBindingType eBindingType,
                                                              GLuint uMaxSize,
                                                              GLvoid* pTable);


// Executable related functions.
SH_IMPORT_EXPORT const GLbyte*        C_DECL  OGL_Compiler_GetAsmVertexExecutable (GLSLHandle hProgram);
SH_IMPORT_EXPORT const GLbyte*        C_DECL  OGL_Compiler_GetAsmFragmentExecutable (GLSLHandle hProgram);
SH_IMPORT_EXPORT SGLSLExecutable*   C_DECL  OGL_Compiler_GetBinVertexExecutable (GLSLHandle hProgram);
SH_IMPORT_EXPORT SGLSLExecutable*   C_DECL  OGL_Compiler_GetBinFragmentExecutable (GLSLHandle hProgram);
SH_IMPORT_EXPORT SGLSLExecutable*   C_DECL  OGL_Compiler_GetBinGeometryExecutable (GLSLHandle hProgram);



#ifdef __cplusplus
}
#endif

#endif  //  for _SHADER_COMPILER_INTERFACE_INCLUDE
