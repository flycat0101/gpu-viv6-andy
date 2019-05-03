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


#ifndef TGALOADER_H
#define TGALOADER_H

#include "main.h"

class Texture
{
public:
  Texture(const char *textureFileName, GLenum minFilter = GL_LINEAR,GLenum magFilter = GL_LINEAR,GLenum wrapS = GL_REPEAT,GLenum wrapT = GL_REPEAT, int width = 128, int height = 128, int bpp = 24);
  ~Texture();
  inline void BindTexture() const {glBindTexture(GL_TEXTURE_2D, m_id);}; //Set the texture active for the current context
  inline GLuint GetID()     const {return m_id;}; //returns the OpenGL ES texture handle
  inline bool GetState()    const {return m_state;}; //returns the texture's creation state

private:
  bool LoadTGA(const char *textureFileName,GLubyte **pixels); //loads a tga texture (uncompressed, and with 24 or 32 bits)
  bool LoadRaw(const char *textureFileName,GLubyte **pixels,int width, int height, int bpp); //loads a raw texture
  void FlipBitmapVertical(GLubyte *data,int format); //flips vertically a GLubyte array, useful for raw textures, because they turned upside down
  bool m_state;
  int m_width, m_height;
  GLuint m_id;
  GLenum m_minFilter, m_magFilter;
  GLenum m_wrapS, m_wrapT;
  GLenum m_format;
};

#endif
