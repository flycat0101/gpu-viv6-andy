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


#ifndef FONT_H
#define FONT_H

#include "main.h"

class BitmappedFont
{
public:
  BitmappedFont(GLuint textureFontID, int fontWidth = 15, int fontHeight = 15);
  ~BitmappedFont();
  static void EnableStates(); //Begin Print
  void Print(int x, int y, const char *fmt, ...);
  static void DisableStates(); //EndPrint

private:
  GLuint m_textureFontID;
  int m_fontWidth,m_fontHeight;

  //I've decided make this array static because will be exactly the same data for all fonts we will use
  static GLfixed m_textureCoordinates[2 * 4 * 8 * 16]; //2 = st, 4 = vertices per character, 8 = rows, 16 = columns
  static bool m_instanced;
};

#endif
