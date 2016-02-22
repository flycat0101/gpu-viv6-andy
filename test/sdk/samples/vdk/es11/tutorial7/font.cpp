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


#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "font.h"
#include "fixed_point.h"
GLfixed BitmappedFont::m_textureCoordinates[];
bool BitmappedFont::m_instanced(false);

BitmappedFont::BitmappedFont(GLuint textureFontID, int fontWidth , int fontHeight):m_textureFontID(textureFontID),m_fontWidth(fontWidth),m_fontHeight(fontHeight)
{
  if(m_instanced) return;
  m_instanced = true;

  const float rows = 8.0f;
  const float columns = 16.0f;
  const float xoffset = 1.0f / columns; // the font texture has 16 columns
  const float yoffset = 1.0f / rows; //the font texture has 8 rows

  //prebuild the texture coordinates array for all characters
  for(int i=0;i<128;i++)
  {
    const float cx = (float)(i % 16) * xoffset;
    const float cy = (float)(i / 16) * yoffset;
    const int index = i * 8;
    m_textureCoordinates[index + 0] = FixedFromFloat(cx + xoffset);
    m_textureCoordinates[index + 1] = FixedFromFloat(cy + yoffset);
    m_textureCoordinates[index + 2] = FixedFromFloat(cx + xoffset);
    m_textureCoordinates[index + 3] = FixedFromFloat(cy);
    m_textureCoordinates[index + 4] = FixedFromFloat(cx);
    m_textureCoordinates[index + 5] = FixedFromFloat(cy + yoffset);
    m_textureCoordinates[index + 6] = FixedFromFloat(cx);
    m_textureCoordinates[index + 7] = FixedFromFloat(cy);
  }
}
//----------------------------------------------------------------------------
BitmappedFont::~BitmappedFont()
{
  //if you want, you can delete your font texture here
}
//----------------------------------------------------------------------------
void BitmappedFont::EnableStates()
{
  /*This function exists because we can use it to enable all needed states to display the text
    for many Print calls, instead of call this states on every Print call*/
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE);
}
//----------------------------------------------------------------------------
void BitmappedFont::DisableStates()
{
  glDisable(GL_TEXTURE_2D);
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisable(GL_BLEND);
}
//----------------------------------------------------------------------------
void BitmappedFont::Print(int x, int y, const char *fmt, ...)
{
  //Convert the variable argument list, to a single char[] array
  //be careful, the final string length must fit into this array
  char text[256];
  va_list ap;
  if (fmt == NULL)
    return;

  va_start(ap, fmt);
#ifdef UNDER_CE
  vsprintf(text, fmt, ap);
#else
  vsnprintf(text, sizeof(text), fmt, ap);
#endif
  va_end(ap);
  if(text[0]=='\0')
    return;

  int length = (int)strlen(text);

  //Set up our arrays
  GLfixed vertices[8];
  glBindTexture(GL_TEXTURE_2D, m_textureFontID);
  glVertexPointer(2, GL_FIXED, 0, vertices);

  const GLfixed fy = FixedFromInt(y);
  const GLfixed fheight = FixedFromInt(m_fontHeight);

  //go throught all chars int the array
  for(int i = 0;i < length;i++)
  {
    /*We have to substract 32 units, because the characters stored in our font texture starts in the
      ASCII code 32 (the first 32 ASCII code, are control codes, usually non-displayables(*/
    const int c = text[i]-32;
    const int originX = x + (i * m_fontWidth);
    const int offsetX = originX + m_fontWidth;

    //Compute the dimensions and position of the 'quad' (in reality is a triangle strip with two triangles)
    vertices[0] = FixedFromInt(offsetX); vertices[1] = fy;
    vertices[2] = FixedFromInt(offsetX); vertices[3] = fy + fheight;
    vertices[4] = FixedFromInt(originX); vertices[5] = fy;
    vertices[6] = FixedFromInt(originX); vertices[7] = fy + fheight;

    //we access to the corresponding texture coordinates in the precomputed texture coordinates array
    glTexCoordPointer(2, GL_FIXED, 0, &m_textureCoordinates[c*8]);

    //draw the character
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }

  return;
}
