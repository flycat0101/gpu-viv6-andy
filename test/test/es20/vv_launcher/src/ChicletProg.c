/****************************************************************************
*
*    Copyright 2012 - 2019 Vivante Corporation, Santa Clara, California.
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


#include "ChicletProg.h"

#include <stdio.h>
#include <stdlib.h>

ChicletProg* ChicletProgConstruct()
{
    ChicletProg* cp = (ChicletProg*)malloc(sizeof (ChicletProg));

    cp->pos_attr = -1;
    cp->norm_attr = -1;
    cp->light_unif = -1;
    cp->proj_unif = -1;
    cp->modelview_unif = -1;
    cp->env_unif = -1;
    cp->env_tex_unif = -1;

    cp->prog = LoadProgram("chiclet.vert", "chiclet.frag");
    ShaderProgramUse(cp->prog);

    if (cp->prog == NULL)
    {
        return NULL;
    }

    cp->pos_attr = ShaderProgramGetAttribLoc(cp->prog, "pos_attr");
    cp->norm_attr = ShaderProgramGetAttribLoc(cp->prog, "norm_attr");

    cp->light_unif = ShaderProgramGetUniformLoc(cp->prog, "light_unif");
    cp->proj_unif = ShaderProgramGetUniformLoc(cp->prog, "proj_unif");
    cp->modelview_unif = ShaderProgramGetUniformLoc(cp->prog, "modelview_unif");
    cp->env_unif = ShaderProgramGetUniformLoc(cp->prog, "env_unif");

    cp->env_tex_unif = ShaderProgramGetUniformLoc(cp->prog, "env_tex_unif");
    cp->content_tex_unif = ShaderProgramGetUniformLoc(cp->prog, "content_tex_unif");

    return cp;
}

void ChicletProgDestroy(ChicletProg* Cp)
{
    assert(Cp != NULL);

    ShaderProgramDestroy(Cp->prog);
    free(Cp);
}


void ChicletProgUse(ChicletProg* Cp)
{
    ShaderProgramUse(Cp->prog);
}


void ChicletProgSetLight(ChicletProg* Cp, const Vec3f* V)
{
    glUniform3f(Cp->light_unif, V->v[0], V->v[1], V->v[2]);
}


void ChicletProgSetProj(ChicletProg* Cp, const Matf* proj)
{
    glUniformMatrix4fv(Cp->proj_unif, 1, GL_FALSE, &proj->m[0][0]);
}


void ChicletProgSetModelview(ChicletProg* Cp, const Matf* mv)
{
    glUniformMatrix4fv(Cp->modelview_unif, 1, GL_FALSE, &mv->m[0][0]);
}


void ChicletProgSetEnv(ChicletProg* Cp, const Matf* env)
{
    glUniformMatrix4fv(Cp->env_unif, 1, GL_FALSE, &env->m[0][0]);
}

