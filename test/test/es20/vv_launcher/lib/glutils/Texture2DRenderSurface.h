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


#ifndef glutils_Texture2DRenderSurface_INCLUDED
#define glutils_Texture2DRenderSurface_INCLUDED

#include <GLES2/gl2.h>

//#include "RenderSurface.h"
#include "Texture2D.h"

#ifdef __cplusplus
extern "C" {
#endif

struct _Stack;
typedef struct _Stack Stack;

typedef struct _Texture2DRenderSurface
{
    Texture2D* texture;

    int initialized;
    int include_depth;
    int include_alpha;

    Stack* fbo_stack;
    GLuint fbo;
    GLuint depth_buffer;

} Texture2DRenderSurface;


/*
  The default constructor doesn't automatically construct the
  render surface. This allows Texture2DRenderSurface to be created
  before a valid OpenGLES context has been bound. Calling 'init'
  explicitly initializes the surface, but the surface will be
  implicitly initialized the first time 'bind' is called if it
  hasn't been already.
*/
Texture2DRenderSurface* Texture2DRenderSurfaceConstruct(
    unsigned int width,
    unsigned int height,
    int include_depth,
    int include_alpha
);


void Texture2DRenderSurfaceDestroy(Texture2DRenderSurface* Surface);

void Texture2DRenderSurfaceInit(Texture2DRenderSurface* Surface);

void Texture2DRenderSurfaceBind(Texture2DRenderSurface* Surface);

void Texture2DRenderSurfaceUnbind(Texture2DRenderSurface* Surface);

unsigned int Texture2DRenderSurfaceGetWidth(Texture2DRenderSurface* Surface);

unsigned int Texture2DRenderSurfaceGetHeight(Texture2DRenderSurface* Surface);

/*
// prevent copying
Texture2DRenderSurface(Texture2DRenderSurface const &);
Texture2DRenderSurface& operator=(Texture2DRenderSurface const &);
*/

#ifdef __cplusplus
}
#endif

#endif

