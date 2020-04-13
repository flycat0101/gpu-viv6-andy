/****************************************************************************
*
*    Copyright 2012 - 2020 Vivante Corporation, Santa Clara, California.
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


#include "Texture2DRenderSurface.h"

#include <stdio.h>
#include <stdlib.h>

#include "check.h"
#include "log.h"

static void _Push(Stack* stack, unsigned int v);
static unsigned int _Top(Stack* stack);
static unsigned int _Pop(Stack* stack);
static int _Empty(Stack* stack);
static Stack* _StackConstruct();
static void _StackDestroy(Stack* stack);


Texture2DRenderSurface* Texture2DRenderSurfaceConstruct(
        unsigned int width,
        unsigned int height,
        int d,
        int a
        )
    {
    Texture2DRenderSurface* surface =
        (Texture2DRenderSurface*)malloc(sizeof (Texture2DRenderSurface));

    surface->texture = Texture2DConstruct(0, width, height);
    surface->initialized = 0;
    surface->include_depth = d;
    surface->include_alpha = a;
    surface->fbo_stack = _StackConstruct();

    return surface;
}


void Texture2DRenderSurfaceInit(Texture2DRenderSurface* Surface)
{
    if (!Surface->initialized)
    {
        GLuint curr_fbo;

        Surface->initialized = 1;

        glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&curr_fbo);
        CheckGL("Texture2DRenderSurface::init GL_FRAMEBUFFER_BINDING");

        // build color texture and attach it to the framebuffer

        glGenTextures(1, &Surface->texture->id);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Surface->texture->id);
        CheckGL("Texture2DRenderSurface::init glBindTexture");

        if (Surface->include_alpha)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Surface->texture->width, Surface->texture->height, 0,
                             GL_RGBA, GL_UNSIGNED_BYTE, 0);
        } else
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Surface->texture->width, Surface->texture->height, 0,
                             GL_RGB, GL_UNSIGNED_BYTE, 0);
        }
        CheckGL("Texture2DRenderSurface::init glTexImage2D");

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_2D, 0);

        glGenFramebuffers(1, &Surface->fbo);
        CheckGL("Texture2DSurface::init glGenFramebuffers");

        glBindFramebuffer(GL_FRAMEBUFFER, Surface->fbo);
        CheckGL("Texture2DSurface::init glBindFramebuffer");

        /*
        if (!glIsFramebuffer(Surface->fbo))
        {
            LogError("%d  is not a fbo\n", Surface->fbo);
        }
        */

        glFramebufferTexture2D(GL_FRAMEBUFFER,
                                      GL_COLOR_ATTACHMENT0,
                                      GL_TEXTURE_2D, Surface->texture->id, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        CheckGL("Texture2DRenderSurface::init glFramebufferTexture2D");
        CheckFramebufferStatus("Texture2DRenderSurface::init glFramebufferTexture2D");

        if (Surface->include_depth)
        {
            // build depth buffer and attach it
            glGenRenderbuffers(1, &Surface->depth_buffer);

            glBindRenderbuffer(GL_RENDERBUFFER, Surface->depth_buffer);
            CheckGL("Texture2DRenderSurface::init glBindRenderbuffer");

            glRenderbufferStorage(GL_RENDERBUFFER,
                                         GL_DEPTH_COMPONENT16,
                                         Surface->texture->width,
                                         Surface->texture->height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                              GL_RENDERBUFFER, Surface->depth_buffer);
            CheckGL("Texture2DRenderSurface: glRenderbufferStorage for depth");
        }

        CheckFramebufferStatus("Texture2DRenderSurface::init");

        glBindFramebuffer(GL_FRAMEBUFFER, curr_fbo);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}


void Texture2DRenderSurfaceDestroy(Texture2DRenderSurface* Surface)
{
    if (Surface->initialized)
    {
        // detach depth buffer
        GLuint curr_fbo;

        glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint *)&curr_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, Surface->fbo);

        CheckGL("Texture2DRenderSurface::~Texture2DSurface"
                  "GL_FRAMEBUFFER_BINDING");

        glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                  GL_DEPTH_ATTACHMENT,
                                  GL_RENDERBUFFER,
                                  0);

        if (curr_fbo != Surface->fbo)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, curr_fbo);
        }
        else
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        glDeleteRenderbuffers(1, &Surface->depth_buffer);
        glDeleteFramebuffers(1, &Surface->fbo);
    }

    Texture2DDestroy(Surface->texture);

    _StackDestroy(Surface->fbo_stack);

    free(Surface);
}


void Texture2DRenderSurfaceBind(Texture2DRenderSurface* Surface)
{
    GLuint curr_fbo;

    if (!Surface->initialized)
    {
        Texture2DRenderSurfaceInit(Surface);
    }

    glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&curr_fbo);
    CheckGL("Texture2DRenderSurface::bindSurface GL_FRAMEBUFFER_BINDING");

    _Push(Surface->fbo_stack, curr_fbo);

    glBindFramebuffer(GL_FRAMEBUFFER, Surface->fbo);
}


void Texture2DRenderSurfaceUnbind(Texture2DRenderSurface* Surface)
{
    GLuint curr_fbo;

    glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&curr_fbo);

    if (!Surface->initialized || _Empty(Surface->fbo_stack) || (curr_fbo != Surface->fbo))
    {
        LogError("Texture2DRenderSurface::unbindSurface: surface "
             "%d isn't currently bound\n"
             "Texture2DRenderSurface::unbindSurface: surface "
             "%d is currently bound\n",
          Surface->fbo, curr_fbo);

        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, _Top(Surface->fbo_stack));

    _Pop(Surface->fbo_stack);
}


unsigned int Texture2DRenderSurfaceGetWidth(Texture2DRenderSurface* Surface)
{
    return Surface->texture->width;
}


unsigned int Texture2DRenderSurfaceGetHeight(Texture2DRenderSurface* Surface)
{
    return Surface->texture->height;
}


typedef struct _Entry
{
    struct _Entry* next;
    unsigned value;
} Entry;


struct _Stack
{
    Entry* head;
};


static void _Push(Stack* stack, unsigned int v)
{
    Entry* node = (Entry*)malloc(sizeof (Entry));
    node->next = stack->head;
    node->value = v;
    stack->head = node;
}


static unsigned int _Top(Stack* stack)
{
    return stack->head != NULL ? stack->head->value : 0;
}


static unsigned int _Pop(Stack* stack)
{
    if (stack->head != NULL)
    {
        Entry* nextHead = stack->head->next;
        unsigned int t = stack->head->value;
        free(stack->head);
        stack->head = nextHead;

        return t;
    }
    else
    {
        return 0;
    }
}


static int _Empty(Stack* stack)
{
    return stack->head == NULL;
}


static Stack* _StackConstruct()
{
    Stack* stack = (Stack*)malloc(sizeof (Stack));
    stack->head = NULL;

    return stack;
}


static void _StackDestroy(Stack* stack)
{
    while (stack->head != NULL)
    {
         Entry* prevHead = stack->head;
         stack->head = stack->head->next;
         free(prevHead);
    }

    free(stack);
}

