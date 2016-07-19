/****************************************************************************
*
*    Copyright 2012 - 2016 Vivante Corporation, Santa Clara, California.
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


#include "program.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <rc/resource.h>
#include "log.h"
#include "check.h"

VertexShader* VertexShaderConstruct(GLuint Sh, const char* Name)
{
    VertexShader* vs = (VertexShader*)malloc(sizeof (VertexShader));
    vs->shader = Sh;
    vs->name = Name;

    return vs;
}


void VertexShaderDestroy(VertexShader* Vs)
{
    glDeleteShader(Vs->shader);
    free(Vs);
}


FragmentShader* FragmentShaderConstruct(GLuint Sh, const char* Name)
{
    FragmentShader* fs = (FragmentShader*)malloc(sizeof (FragmentShader));
    fs->shader = Sh;
    fs->name = Name;

    return fs;
}


void FragmentShaderDestroy(FragmentShader* Fs)
{
    glDeleteShader(Fs->shader);
    free(Fs);
}


ShaderProgram* ShaderProgramConstruct(GLint Prog,
                             VertexShader*  Vs,
                             FragmentShader* Fs)
{
    ShaderProgram* sp = (ShaderProgram*)malloc(sizeof (ShaderProgram));
    sp->prog = Prog;
    sp->vertex_shader = Vs;
    sp->fragment_shader = Fs;

    assert(glIsProgram(Prog));

    {
        static char str[64];
        sprintf(str, "ShaderProgram(%d,%s,%s)", Prog, Vs->name, Fs->name);
#ifdef UNDER_CE
        sp->name = _strdup(str);
#else
        sp->name = strdup(str);
#endif
    }

    return sp;
}


void ShaderProgramDestroy(ShaderProgram* Prog)
{
    glDeleteProgram(Prog->prog);

    FragmentShaderDestroy(Prog->fragment_shader);
    VertexShaderDestroy(Prog->vertex_shader);

    if (Prog->name != NULL)
    {
        free(Prog->name);
    }
    free(Prog);
}


void ShaderProgramUse(ShaderProgram* Prog)
{
    if (Prog->prog != 0)
    {
        glUseProgram(Prog->prog);
        CheckGL(Prog->name);
    }
}


void ShaderProgramValidate(ShaderProgram* Prog)
{
    GLint status = GL_FALSE;

    assert(glIsProgram(Prog->prog));

    glValidateProgram(Prog->prog);
    CheckGL(Prog->name);

    glGetProgramiv(Prog->prog, GL_VALIDATE_STATUS, &status);
    CheckGL(Prog->name);

    if (status != GL_TRUE)
    {
        GLint log_length;
        char* log;

        glGetProgramiv(Prog->prog, GL_INFO_LOG_LENGTH, &log_length);

        log = (char*)malloc(log_length + 1);
        glGetProgramInfoLog(Prog->prog, log_length+1, 0, log);

        LogError("%s :validate status\n %s\n", Prog->name, log);

        free(log);
    }
}


GLint ShaderProgramGetAttribLoc(ShaderProgram* Prog, const char * Attrib)
{
    return CheckedGetAttribLoc(Prog->prog, Attrib, Prog->name);
}


int ShaderProgramGetUniformLoc(ShaderProgram* Prog, const char* Uniform)
{
    return CheckedGetUniformLoc(Prog->prog, Uniform, Prog->name);
}


static GLuint _LoadShader(char const * rc_name, GLenum type)
{
    GLuint shader = 0;
    const char* message;
    Resource* rc;
    const char* src;
    GLint size;
    GLint status;

#ifdef DEBUG
    LogError("loadShader(%s)\n", rc_name);
#endif

    switch (type)
    {
    case GL_FRAGMENT_SHADER:
        message = "FragmentShader";
        break;
    case GL_VERTEX_SHADER:
        message = "VertexShader";
        break;
    default:
        LogError("loadShader: bad type\n");
        return 0;
    }


    rc = GetResource(rc_name);
    if (rc == NULL)
    {
        LogError("%s: %s: not found\n", message, rc_name);
        return 0;
    }

    src = (const char *)rc->data;
    size = (GLint)rc->size;

    shader = glCreateShader(type);

    if (shader == 0)
    {
        CheckGL(message);
        return 0;
    }

    glShaderSource(shader, 1, &src, &size);

    status = GL_FALSE;

#ifdef DEBUG
    LogError("%s : glCompileShader\n", rc_name);
#endif

    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        GLint log_length;
        char * log;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

        log = (char*)malloc(log_length + 1);
        glGetShaderInfoLog(shader, log_length + 1, 0, log);

        LogError("%s: %s: comiple error %s\n", message, rc_name, log);
        glDeleteShader(shader);
        free(log);

        return 0;
    }

    // Successfully compiled shader.
    return shader;
}


VertexShader* LoadVertexShader(const char * rc_name)
{
    GLuint shader = _LoadShader(rc_name, GL_VERTEX_SHADER);
    if (shader != 0)
    {
        return VertexShaderConstruct(shader, rc_name);
    }

    return NULL;
}


FragmentShader* LoadFragmentShader(char const * rc_name)
{

    GLuint shader = _LoadShader(rc_name, GL_FRAGMENT_SHADER);
    if (shader != 0)
    {
        return FragmentShaderConstruct(shader, rc_name);
    }

    return NULL;
}


ShaderProgram* LoadProgram(char const * vrc, char const * frc)
{
    VertexShader* vs;
    FragmentShader* fs;

#ifdef DEBUG
    LogError("loadProgram(%s,%s)\n", vrc, frc);
#endif

    vs = LoadVertexShader(vrc);
    fs = LoadFragmentShader(frc);

    return LoadProgram2(vs, fs);
}


ShaderProgram* LoadProgram2(VertexShader* vs, FragmentShader* fs)
{
    GLuint program;
    GLint status ;

    if (vs == NULL)
    {
        LogError("ShaderProgram: null vertex program\n");
        return NULL;
    }

    if (fs == NULL)
    {
        LogError("ShaderProgram: null fragment program\n");
        return NULL;
    }

    program = glCreateProgram();

#ifdef DEBUG
    LogError("Creating program %d for shaders %s (%d) and %s (%d)\n",
         program,
         vs->name,
         vs->shader,
         fs->name,
         fs->shader
        );
#endif
    assert(glIsProgram(program));
    glAttachShader(program, vs->shader);
    if (CheckGL("ShaderProgram")) return NULL;

    assert(glIsProgram(program));
    glAttachShader(program, fs->shader);
    if (CheckGL("ShaderProgram")) return NULL;

    assert(glIsProgram(program));
#ifdef DEBUG
    LogError("program: %d: glLinkProgram\n", program);
#endif

    glLinkProgram(program);
    assert(glIsProgram(program));

    status = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE)
    {
        GLint log_length;
        char * log ;

        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);

        log = (char*)malloc(log_length + 1);
        glGetProgramInfoLog(program, log_length+1, 0, log);

        LogError("ShaderProgram(%s,%s): linker error\n %s\n",
             vs->name,
             fs->name,
             log);

        return NULL;
    }

    assert(glIsProgram(program));

    return ShaderProgramConstruct(program, vs, fs);
}

