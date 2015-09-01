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


#include "check.h"

#include "log.h"

const char* GetEGLMessage(EGLint Error)
{
	switch(Error)
	{
	case EGL_SUCCESS:
		return "EGL_SUCCESS";
	case EGL_NOT_INITIALIZED:
		return "EGL_NOT_INITIALIZED";
	case EGL_BAD_ACCESS:
		return "EGL_BAD_ACCESS";
	case EGL_BAD_ALLOC:
		return "EGL_BAD_ALLOC";
	case EGL_BAD_ATTRIBUTE:
		return "EGL_BAD_ATTRIBUTE";
	case EGL_BAD_CONFIG:
		return "EGL_BAD_CONFIG";
	case EGL_BAD_CONTEXT:
		return "EGL_BAD_CONTEXT";
	case EGL_BAD_CURRENT_SURFACE:
		return "EGL_BAD_CURRENT_SURFACE";
	case EGL_BAD_DISPLAY:
		return "EGL_BAD_DISPLAY";
	case EGL_BAD_MATCH:
		return "EGL_BAD_MATCH";
	case EGL_BAD_NATIVE_PIXMAP:
		return "EGL_BAD_NATIVE_PIXMAP";
	case EGL_BAD_NATIVE_WINDOW:
		return "EGL_BAD_NATIVE_WINDOW";
	case EGL_BAD_PARAMETER:
		return "EGL_BAD_PARAMETER";
	case EGL_BAD_SURFACE:
		return "EGL_BAD_SURFACE";
	}

	return "Unknown EGL error";
}


int CheckEGL(const char * Message)
{
	EGLenum result = eglGetError();

	if ((result == 0) || (result == EGL_SUCCESS))
	{
		return 0;
	}

	LogError("EGL error: ");

	if (Message)
	{
		LogError("%s: ", Message);
	}

	LogError("%s\n", GetEGLMessage(result));

	return 1;
}


const char* GetGLMessage(GLenum N)
{
	switch(N)
	{
	case GL_NO_ERROR:
		return "GL_NO_ERROR";
	case GL_INVALID_ENUM:
		return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE:
		return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:
		return "GL_INVALID_OPERATION";
	case GL_OUT_OF_MEMORY:
		return "GL_OUT_OF_MEMORY";
	}

	return "unknown GL error";
}


int CheckGL(char const * Message)
{
	GLenum result = glGetError();

	if ((result == 0) || (result == GL_NO_ERROR))
	{
		return 0;
	}

	LogError("GL error: ");

	if (Message)
	{
		LogError("%s: ", Message);
	}

	LogError("%s\n", GetGLMessage(result));

	return 1;
}


const char* GetFramebufferMessage(GLenum N)
{
	switch(N)
	{
	case GL_FRAMEBUFFER_COMPLETE:
		return "complete";
	case GL_FRAMEBUFFER_UNSUPPORTED:
		return "unsupported";
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		return "incomplete attachment";
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		return "incomplete missing attachment";
	/*
	case GL_FRAMEBUFFER_INCOMPLETE_UNSUPPORTED:
		return "incomplete unsupported";
	*/

	default:
		return "unknown framebuffer status";
	}
}


int CheckFramebufferStatus(const char * Message)
{
	GLenum status = glCheckFramebufferStatus (GL_FRAMEBUFFER);

	if (status == GL_FRAMEBUFFER_COMPLETE)
	{
		return 0;
	}

	LogError("FramebufferStatus: ");

	if (Message)
	{
		LogError("%s: ", Message);
	}

	LogError("%s\n", GetFramebufferMessage(status));

	return 1;
}


int CheckedGetAttribLoc(GLuint Prog,
						const char * Attrib,
						const char * Message
						)
{
	GLint loc = glGetAttribLocation(Prog, Attrib);

	if (loc < 0)
	{
		LogError("Attribute: ");

		if (Message)
		{
			LogError("%s: ", Message);
		}

		LogError("%s not found\n", Attrib);
	}

	return loc;
}


int CheckedGetUniformLoc(GLuint Prog,
						 char const * Uniform,
						 char const * Message
						)
{

	GLint loc = glGetUniformLocation(Prog, Uniform);

	if (loc < 0)
	{
		LogError("Uniform: ");

		if (Message)
		{
			LogError("%s: ", Message);
		}

		LogError("%s not found\n", Uniform);
	}

	return loc;
}

