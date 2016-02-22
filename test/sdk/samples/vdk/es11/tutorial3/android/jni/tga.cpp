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


#include <stdio.h>
#include <stdlib.h>
#include <GLES/gl.h>
#ifdef ANDROID
#include <utils/Log.h>
#endif
#pragma pack(1)
struct TGA_HEADER
{
	unsigned char IDLength;
	unsigned char ColorMapType;
	unsigned char ImageType;
	unsigned char FirstEntryIndexLow;
	unsigned char FirstEntryIndexHigh;
	unsigned char ColorMapLengthLow;
	unsigned char ColorMapLengthHigh;
	unsigned char ColorMapEntrySize;
	unsigned char XOriginOfImageLow;
	unsigned char XOriginOfImageHigh;
	unsigned char YOriginOfImageLow;
	unsigned char YOriginOfImageHigh;
	unsigned char ImageWidthLow;
	unsigned char ImageWidthHigh;
	unsigned char ImageHeightLow;
	unsigned char ImageHeightHigh;
	unsigned char PixelDepth;
	unsigned char ImageDescriptor;
};
#pragma pack()

void *
LoadTGA(
	FILE * File,
	GLenum * Format,
	GLsizei * Width,
	GLsizei * Height
	)
{
	struct TGA_HEADER tga;
	size_t bytes;
	unsigned char * bits;
	unsigned short imageWidth, imageHeight;

	/* Read the TGA file header. */
	if (fread(&tga, sizeof(tga), 1, File) != 1)
	{
		return NULL;
	}

	/* For now we only support uncompressed true-color images. */
	if (tga.ImageType != 2)
	{
		return NULL;
	}

	/* We only support top-left and bottom-left images. */
	if (tga.ImageDescriptor & 0x10)
	{
		return NULL;
	}

	/* Check pixel depth. */
	switch (tga.PixelDepth)
	{
	case 16:
		/* 16-bpp RGB. */
		*Format = GL_UNSIGNED_SHORT_5_6_5;
		break;

	case 24:
		/* 24-bpp RGB. */
		*Format = GL_RGB;
		break;

	case 32:
		/* 32-bpp RGB. */
		*Format = GL_RGBA;
		break;

	default:
		/* Invalid pixel depth. */
		return NULL;
	}

	imageWidth = ((unsigned short)tga.ImageWidthHigh << 8) + (unsigned short)tga.ImageWidthLow;
	imageHeight = ((unsigned short)tga.ImageHeightHigh << 8) + (unsigned short)tga.ImageHeightLow;

	/* Return texture dimension. */
	*Width  = imageWidth;
	*Height = imageHeight;

	/* Compute number of bytes. */
	bytes = imageWidth * imageHeight * tga.PixelDepth / 8;

	/* Skip ID field. */
	if (tga.IDLength)
	{
		fseek(File, tga.IDLength, SEEK_SET);
	}

	/* Allocate the bits. */
	bits = (unsigned char *) malloc(bytes);

	if (bits != NULL)
	{
		if (tga.ImageDescriptor & 0x20)
		{
			/* Read the bits from the TGA file. */
			if (fread(bits, 1, bytes, File) != bytes)
			{
				/* Error reading bits. */
				free(bits);
				bits = NULL;
			}
		}
		else
		{
			GLsizei y;
			GLsizei stride = bytes / imageHeight;

			/* Bottom up - copy line by line. */
			for (y = *Height - 1; y >= 0; --y)
			{
				if ((GLsizei) fread(bits + y * stride, 1, stride, File) != stride)
				{
					/* Error reading bits. */
					free(bits);
					bits = NULL;
					break;
				}
			}
		}

		if (bits != NULL)
		{
			size_t i;

			/* Convert all RGB pixels into GL pixels. */
			for (i = 0; i < bytes; i += tga.PixelDepth / 8)
			{
				unsigned char save;

				switch (tga.PixelDepth)
				{
				case 16:
					/* Swap red and blue channel in 16-bpp. */
					save        = bits[i + 0] & 0x1F;
					bits[i + 0] = (bits[i + 0] & ~0x1F) | (bits[1] >> 3);
					bits[i + 1] = (bits[i + 1] & ~0xF8) | (save << 3);
					break;

				case 24:
				case 32:
					/* Swap red and blue channel in 24-bpp or 32-bpp. */
					save        = bits[i + 0];
					bits[i + 0] = bits[i + 2];
					bits[i + 2] = save;
					break;
				}
			}
		}
	}

	/* Return the bits. */
	return bits;
}
