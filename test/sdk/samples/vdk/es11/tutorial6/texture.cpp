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


#define _CRT_SECURE_NO_WARNINGS
#ifdef UNDER_CE
#include <windows.h>
#endif
#include <string.h>
#include <stdio.h>
#include "texture.h"

Texture::Texture(const char *textureFileName, GLenum minFilter,GLenum magFilter,GLenum wrapS,GLenum wrapT, int width, int height, int bpp)
{

  GLubyte *pixels = NULL;
  //Check the filename's length for security
  int len = strlen(textureFileName);
  if(len<4)
  {
    m_state = false;
    return;
  }

  //choose the texture format based on the filename extension
  //these two functions returns the texture in the pixels' array
  if(!strcmp(".tga",&textureFileName[len-4]))
    m_state = LoadTGA(textureFileName,&pixels);
  else if(!strcmp(".raw",&textureFileName[len-4]))
    m_state = LoadRaw(textureFileName,&pixels, width, height, bpp);
  else
  {
    m_state = false;
    return;
  }

  if(!m_state)
  {
    if(pixels) delete [] pixels;
    m_state = false;
    return;
  }

  //setup format (it shall be , 2, 3 or 4)
  switch(m_format)
  {
  case 1:
    m_format = GL_LUMINANCE;
    break;
  case 2:
    m_format = GL_LUMINANCE_ALPHA;
    break;
  case 3:
    m_format = GL_RGB;
    break;
  case 4:
    m_format = GL_RGBA;
    break;
  };

  m_minFilter = minFilter;
  m_magFilter = magFilter;
  m_wrapS = wrapS;
  m_wrapT = wrapT;

  glGenTextures(1, &m_id);
  glBindTexture(GL_TEXTURE_2D, m_id);
  glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_minFilter);
  glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_magFilter);
  glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrapS);
  glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrapT);

  if((m_minFilter!=GL_LINEAR)&&(m_minFilter!=GL_NEAREST))
    glTexParameterx(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);  //autogenerate mipmaps
  int a = glGetError();
  glTexImage2D(GL_TEXTURE_2D, 0, m_format, m_width, m_height, 0, m_format, GL_UNSIGNED_BYTE, pixels);
  a = glGetError();
  delete [] pixels;
}
//----------------------------------------------------------------------------
Texture::~Texture()
{
  glDeleteTextures(1,&m_id);
}
//----------------------------------------------------------------------------
bool Texture::LoadRaw(const char *textureFileName,GLubyte **pixels,int width, int height,int bpp)
{
  FILE *f = NULL;
  int format = 0;

  f = fopen(textureFileName,"rb");
#ifdef UNDER_CE
  if (f == NULL)
  {
	wchar_t moduleName[MAX_PATH];
	char path[MAX_PATH], * p;
	GetModuleFileName(NULL, moduleName, MAX_PATH);
	wcstombs(path, moduleName, MAX_PATH);
	p = strrchr(path, '\\');
	strcpy(p + 1, textureFileName);
	f = fopen(path, "rb");
  }
#endif
  if(!f) return false;

  format = bpp >> 3; //format = bpp / 8
  m_height = height;
  m_width = width;

  *pixels = new GLubyte[width * height * format];
  fread(*pixels , width * height * format, 1, f);
  fclose(f);

  FlipBitmapVertical(*pixels, format); //flip
  m_format = format; //bytes per pixel
  return true;
}
//----------------------------------------------------------------------------
bool Texture::LoadTGA(const char *textureFileName, GLubyte **pixels)
{
  FILE *f = fopen(textureFileName, "rb");
#ifdef UNDER_CE
  if (f == NULL)
  {
	wchar_t moduleName[MAX_PATH];
	char path[MAX_PATH], * p;
	GetModuleFileName(NULL, moduleName, MAX_PATH);
	wcstombs(path, moduleName, MAX_PATH);
	p = strrchr(path, '\\');
	strcpy(p + 1, textureFileName);
	f = fopen(path, "rb");
  }
#endif
  if(!f) return false;

    unsigned short width, height;
	unsigned char widthLow, widthHigh, heightLow, heightHigh;
	unsigned char headerLength = 0;
	unsigned char imageType = 0;
	unsigned char bits = 0;
	int format= 0;
	int lineWidth = 0;

	fread(&headerLength, sizeof(unsigned char), 1, f);
	fseek(f,1,SEEK_CUR);
	fread(&imageType, sizeof(unsigned char), 1, f);
	fseek(f, 9, SEEK_CUR);
	fread(&widthLow,  sizeof(unsigned char), 1, f);
	fread(&widthHigh,  sizeof(unsigned char), 1, f);
	width = (widthHigh << 16) + widthLow;
	fread(&heightLow, sizeof(unsigned char), 1, f);
	fread(&heightHigh, sizeof(unsigned char), 1, f);
	height = (heightHigh << 16) + heightLow;
	fread(&bits,   sizeof(unsigned char), 1, f);

  m_width = width;
  m_height = height;

	fseek(f, headerLength + 1, SEEK_CUR);
	GLubyte *buffer = NULL;
	if(imageType != 10)
	{
		if((bits == 24)||(bits == 32)) //added to support for LUMINANCE and RGBA textures
		{
			format = bits >> 3;
			lineWidth = format * m_width;
			buffer = new GLubyte[lineWidth * m_height];

			for(int y = 0; y < m_height; y++)
			{
	  		GLubyte *line = &buffer[lineWidth * y];
				fread(line, lineWidth, 1, f);

        if(format!= 1)
        {
				  for(int i=0;i<lineWidth ; i+=format) //swap R and B because TGA are stored in BGR format
				  {
					  int temp  = line[i];
					  line[i]   = line[i+2];
					  line[i+2] = temp;
				  }
        }
			}
		}
		else
    {
      fclose(f);
      *pixels = buffer;
      return false;
    }
	}
	fclose(f);

  *pixels = buffer;
  m_format = format; //bytes per pixel
  return true;
}
//----------------------------------------------------------------------------
void Texture::FlipBitmapVertical(GLubyte *data, int format)
{
  int i,lineWidth = m_width * format;
  GLubyte *row = NULL, *top = NULL, *bottom = NULL;
  row = new GLubyte[m_height * m_width * format];

  top = data;
  bottom = data + lineWidth * (m_height-1);

  for (i = 0; i < m_height >> 1; i++)
  {
    memcpy(row, top, lineWidth);
    memcpy(top, bottom, lineWidth);
    memcpy(bottom, row, lineWidth);
    top = top + lineWidth;
    bottom = bottom - lineWidth;
  }
  if(row) delete [] row;
}
