/****************************************************************************
*
*    Copyright 2012 - 2017 Vivante Corporation, Santa Clara, California.
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


#include "EnvProg.h"

#include <stdio.h>
#include <stdlib.h>

EnvProg* EnvProgConstruct()
{
    EnvProg* ep = (EnvProg*)malloc(sizeof (EnvProg));
    if (ep == NULL)
    {
        return NULL;
    }

    ep->pos_attr = -1;
    ep->proj_unif = -1;
    ep->env_unif = -1;
    ep->env_tex_unif = -1;

    ep->prog = LoadProgram("env.vert", "env.frag");
    ShaderProgramUse(ep->prog);

    if (ep->prog == NULL)
    {
        free(ep);
        return NULL;
    }

    ep->pos_attr = ShaderProgramGetAttribLoc(ep->prog, "pos_attr");

    ep->proj_unif = ShaderProgramGetUniformLoc(ep->prog, "proj_unif");
    ep->env_unif = ShaderProgramGetUniformLoc(ep->prog, "env_unif");

    ep->env_tex_unif = ShaderProgramGetUniformLoc(ep->prog, "env_tex_unif");

    return ep;
}


void EnvProgDestroy(EnvProg* Env)
{
    assert(Env != NULL);

    ShaderProgramDestroy(Env->prog);
    free(Env);
}


void EnvProgUse(EnvProg* Env)
{
    ShaderProgramUse(Env->prog);
}



void EnvProgSetProj(EnvProg* Env, const Matf* M)
{
    glUniformMatrix4fv(Env->proj_unif, 1, GL_FALSE, &M->m[0][0]);
}


void EnvProgSetEnv(EnvProg* Env, const Matf* M)
{
    glUniformMatrix4fv(Env->env_unif, 1, GL_FALSE, &M->m[0][0]);
}

