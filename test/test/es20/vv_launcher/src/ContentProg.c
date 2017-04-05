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


#include "ContentProg.h"

ContentProg* ContentProgConstruct()
{
    ContentProg* cp = (ContentProg*)malloc(sizeof (ContentProg));

    cp->pos_attr = -1;
    cp->uv_attr = -1;
    cp->proj_unif = -1;
    cp->modelview_unif = -1;

    cp->prog = LoadProgram("content.vert", "content.frag");
    ShaderProgramUse(cp->prog);

    cp->pos_attr = ShaderProgramGetAttribLoc(cp->prog, "pos_attr");
    cp->uv_attr = ShaderProgramGetAttribLoc(cp->prog, "uv_attr");

    cp->proj_unif = ShaderProgramGetUniformLoc(cp->prog, "proj_unif");
    cp->modelview_unif = ShaderProgramGetUniformLoc(cp->prog, "modelview_unif");

    cp->tex_unif = ShaderProgramGetUniformLoc(cp->prog, "tex_unif");

    return cp;
}


void ContentProgDestroy(ContentProg* Cp)
{
    assert(Cp != NULL);

    ShaderProgramDestroy(Cp->prog);
    free(Cp);
}


void ContentProgUse(ContentProg* Cp)
{
    ShaderProgramUse(Cp->prog);
}


void ContentProgSetProj(ContentProg* Cp, Matf* proj)
{
    glUniformMatrix4fv(Cp->proj_unif, 1, GL_FALSE, &proj->m[0][0]);
}


void ContentProgSetModelview(ContentProg* Cp, Matf* mv)
{
    glUniformMatrix4fv(Cp->modelview_unif, 1, GL_FALSE, &mv->m[0][0]);
}

