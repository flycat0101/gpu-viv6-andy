/****************************************************************************
*
*    Copyright 2012 - 2020 Vivante Corporation, Santa Clara, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/


#ifndef glutils_program_INCLUDED
#define glutils_program_INCLUDED

#include <GLES2/gl2.h>

#ifdef __cplusplus
extern "C" {
#endif


struct _VertexShader;
struct _FragmentShader;
struct _ShaderProgram;

typedef struct _VertexShader VertexShader;
typedef struct _FragmentShader FragmentShader;
typedef struct _ShaderProgram ShaderProgram;

struct _VertexShader
{
    GLuint shader;
    const char* name;
};

VertexShader* VertexShaderConstruct(GLuint Sh, const char* Name);

void VertexShaderDestroy(VertexShader* Shader);

VertexShader* LoadVertexShader(const char* rc);


struct _FragmentShader
{
    GLuint shader;
    const char* name;
};

FragmentShader* FragmentShaderConstruct(GLuint Sh, const char* Name);

void FragmentShaderDestroy(FragmentShader* Shader);

FragmentShader* LoadFragmentShader(const char* rc);


struct _ShaderProgram
{
    GLuint prog;
    char* name;
    VertexShader* vertex_shader;
    FragmentShader* fragment_shader;
};

ShaderProgram* ShaderProgramConstruct(GLint p, VertexShader* Vs, FragmentShader* Fs);

void ShaderProgramDestroy(ShaderProgram* Prog);

void ShaderProgramUse(ShaderProgram* Prog);

void ShaderProgramValidate(ShaderProgram* Prog);

GLint ShaderProgramGetAttribLoc(ShaderProgram* Prog, const char* Attrib);

GLint ShaderProgramGetUniformLoc(ShaderProgram* Prog, const char* Uniform);

ShaderProgram* LoadProgram(const char * vrc, const char * frc);

ShaderProgram* LoadProgram2(VertexShader* Vs, FragmentShader* Fs);

#ifdef __cplusplus
}
#endif

#endif

