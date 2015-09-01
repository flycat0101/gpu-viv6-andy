/****************************************************************************
*
*    Copyright 2012 - 2015 Vivante Corporation, Santa Clara, California.
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


#include "Texture2D.h"

#include <stdio.h>
#include <stdlib.h>
#include <GLES2/gl2.h>
#include <assert.h>

#include "check.h"

void Texture2DBind(Texture2D* Tex)
{
	if (Tex->id > 0)
	{
		glBindTexture(GL_TEXTURE_2D, Tex->id);
		CheckGL("Texture2D::bindTexture");
	}
}


void Texture2DRepeat(const Texture2D* Tex, int Horizontal, int Vertical)
{
	if (Tex->id > 0)

	{

		glBindTexture(GL_TEXTURE_2D, Tex->id);
		CheckGL("glBindTexture");

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
							 Horizontal?GL_REPEAT:GL_CLAMP_TO_EDGE);
		CheckGL("glTexParameterf");
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
							 Vertical?GL_REPEAT:GL_CLAMP_TO_EDGE);
		CheckGL("glTexParameterf");
	}
}


Texture2D* Texture2DConstruct(unsigned int i, unsigned int w, unsigned int h)
{
	Texture2D* tex = (Texture2D*)malloc(sizeof (Texture2D));
	tex->id = i;
	tex->width = w;
	tex->height = h;
	return tex;
}


void Texture2DDestroy(Texture2D* Tex)
{
	assert(Tex != NULL);

	if (Tex->id > 0)
	{
		glDeleteTextures(1, &Tex->id);
	}
	free(Tex);
}

