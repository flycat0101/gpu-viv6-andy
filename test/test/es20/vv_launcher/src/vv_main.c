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


#include <stdio.h>
#include <string.h>
#include <EGL/egl.h>

#if USE_VDK
#include <gc_vdk.h>
#endif

#ifdef LINUX
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>

void* InputThread(void* param);
#endif

#include <glutils/log.h>

#include "LauncherApp.h"
#ifndef ANDROID
int  appWidth	= 640;
int  appHeight	= 480;
int  flipY		= 0;
#endif
int
Run(
	char const * Command
	)
{
#ifdef LINUX
	pid_t pid = fork();

	if (pid == 0)
	{
		// Child - execute the command.
		if (system(Command) == -1)
		{
			_exit(-1);
		}
		_exit(0);
	}

	else if (pid > 0)
	{
		// Parent - wait for the child process to finish.
		int status;
		wait(&status);

		if (status != 0)
		{
			// Failure.
			return 0;
		}
	}

	else
	{
		// Fork failed.
		LogError("fork() failed: errno %d\n", errno);
		// Failure.
		return 0;
	}
#else
	LogError("Should start command %s now.\n", Command);
#endif
	// Success.
	return 1;
}

#	define RETURN_SUCCESS   0
#	define RETURN_ERROR 1

volatile int finished;

#if !USE_VDK
volatile int key;
#endif
#ifndef ANDROID
int main(
	int argc,
	char * argv[]
	)
{
	// Parse all command line arguments.
	int i;
	// Initialize EGL.
	EGLDisplay eglDisplay;
	EGLContext eglContext;
	EGLSurface eglSurface;

	NativeDisplayType display;
	NativeWindowType window;
#if USE_VDK
	vdkPrivate vdk;
#endif

	EGLint configAttribs[] =
	{
		EGL_RED_SIZE,       8,
		EGL_GREEN_SIZE,     8,
		EGL_BLUE_SIZE,      8,
		EGL_ALPHA_SIZE,     EGL_DONT_CARE,
		EGL_DEPTH_SIZE,     24,
		EGL_STENCIL_SIZE,   EGL_DONT_CARE,
		EGL_SAMPLE_BUFFERS, EGL_DONT_CARE,
		EGL_SAMPLES,        EGL_DONT_CARE,
		EGL_SURFACE_TYPE,   EGL_WINDOW_BIT,
		EGL_NONE
	};

	EGLint ctxAttribs[] =
	{
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	EGLConfig configs[10];
	EGLint matchingConfigs;

	LauncherApp * app;

	unsigned int lastTime;
	int status;

	for (i = 1; i < argc; ++i)
	{
		// Test for -qvga.
		if (strcmp("-qvga", argv[i]) == 0)
		{
			appWidth  = 320;
			appHeight = 240;
		}

		// Test for -vga.
		else if (strcmp("-vga", argv[i]) == 0)
		{
			appWidth  = 640;
			appHeight = 480;
		}

		// Test for -flip.
		else if (strcmp("-flip", argv[i]) == 0)
		{
			flipY = 1;
		}

		// Test for -w.
		else if (strcmp("-w", argv[i]) == 0)
		{
			appWidth = atoi(argv[++i]);
		}

		// Test for -h.
		else if (strcmp("-h", argv[i]) == 0)
		{
			appHeight = atoi(argv[++i]);
		}

		// Test for -help.
		else if (strcmp("-help", argv[i]) == 0)
		{
			puts("Supported options:");
			puts(" -vga  : sets resolution to 640x480");
			puts(" -qvga : sets resolution to 320x240");
			puts(" -flip : flip image vertically");
			puts(" -w	: specify window width");
			puts(" -h    : specify window height");
			puts(" -help : show options");

			return RETURN_SUCCESS;
		}

		// Invalid argument.
		else
		{
			printf("%s - Unknown option: %s\n", argv[0], argv[i]);
			puts("Supported options:");
			puts(" -vga  : sets resolution to 640x480");
			puts(" -qvga : sets resolution to 320x240");
			puts(" -flip : flip image vertically");
			puts(" -w    : specify window width");
			puts(" -h    : specify window height");
			puts(" -help : show options");

			return RETURN_ERROR;
		}
	}

#if USE_VDK
	vdk = vdkInitialize();
	display = (EGLNativeDisplayType)vdkGetDisplay(vdk);
	window = (EGLNativeWindowType)vdkCreateWindow((vdkDisplay)display, -1, -1, appWidth, appHeight);
#else
#ifdef EGL_API_FB
	display = fbGetDisplay();

	{
	int w, h, t, l;
	pthread_t pid;

	fbGetDisplayGeometry(display, &w, &h);

	t = (w - appWidth) / 2;
	l = (h - appHeight) / 2;
	window = fbCreateWindow(display, t, l, appWidth , appHeight);

	pthread_create(&pid, NULL, &InputThread, NULL);
	}
#endif
#endif

	/*
	eglBindAPI(EGL_OPENGL_ES_API);
	*/

	/* Get EGLDisplay */
	eglDisplay = eglGetDisplay(display);

	if (!eglInitialize(eglDisplay, NULL, NULL))
	{
		return RETURN_ERROR;
	}

	if (!eglChooseConfig(eglDisplay, configAttribs, &configs[0], 10, &matchingConfigs))
	{
		return RETURN_ERROR;
	}

	if (matchingConfigs < 1)
	{
		return RETURN_ERROR;
	}

	eglSurface = eglCreateWindowSurface(eglDisplay, configs[0], window, configAttribs);
	if (eglSurface == EGL_NO_SURFACE)
	{
		return RETURN_ERROR;
	}

	eglContext = eglCreateContext(eglDisplay, configs[0], NULL, ctxAttribs);
	if (eglContext == EGL_NO_CONTEXT)
	{
		return RETURN_ERROR;
	}

	eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);

#if USE_VDK
	// Set the window title.
	vdkSetWindowTitle((vdkWindow)window, "Launcher");

	// Show the window.
	vdkShowWindow((vdkWindow)window);
#endif

	// Create the application class.
	app = LauncherAppConstruct(appWidth, appHeight);
	flipVertical(app, flipY);

#if USE_VDK
	// Get the current time.
	lastTime = vdkGetTicks();
#else
#ifdef EGL_API_FB
	{
	struct timeval tv;

	/* Return the time of day in milliseconds. */
	gettimeofday(&tv, 0);
	lastTime = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
	}
#endif
#endif

	status = 1;

	// Main loop.
	finished = 0;
	while (!finished)
	{
		// Test for events.
#if USE_VDK
		vdkEvent event;
		if (vdkGetEvent((vdkWindow)window, &event))
		{
			// Test for keyboard event.
			if (event.type == VDK_KEYBOARD)
			{
				// Check the scancode.
				switch (event.data.keyboard.scancode)
				{
				case VDK_ESCAPE:
					// Exit program.
					finished = 1;
					break;

				case VDK_0:
				case VDK_1:
				case VDK_2:
				case VDK_3:
				case VDK_4:
				case VDK_5:
				case VDK_6:
				case VDK_7:
				case VDK_8:
				case VDK_9:
					if (!event.data.keyboard.pressed)
					{
						// Numeric key selects a menu entry.
						int k = event.data.keyboard.key - '0';
						LauncherAppSelect(app, k - 1);
					}
					break;

				default:
					break;
				}
			}

			// Test if application is closing.
			else if (event.type == VDK_CLOSE)
			{
				// Exit program.
				finished = 1;
			}
		}
		else
#else
		if (key != 0)
		{
			// Test for keyboard event.
			// Check the scancode.
			switch (key)
			{
			case 13:
				// Exit program.
				finished = 1;
				break;

			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				{
					// Numeric key selects a menu entry.
					int k = key - '0';
					LauncherAppSelect(app, k - 1);
				}
				break;

			case 'q': case 'Q':
				finished = 1;

			default:
				break;
			}

			key = 0;
		}
		else
#endif
		{
			// Compute the time delta in seconds.
			char const * command;
			unsigned int now;
			float delta;
#if USE_VDK
			now = vdkGetTicks();
#else
#ifdef EGL_API_FB
			struct timeval tv;

			/* Return the time of day in milliseconds. */
			gettimeofday(&tv, 0);
			now = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
#endif
#endif
			delta = (float)(now - lastTime) / 1000.0f;

			// Clamp delta to 1/20th of a second.
			if (delta > 1.0f / 20.0f) delta = 1.0f / 20.0f;
			lastTime = now;

			// Move the application to the next tick.
			command = LauncherAppTick(app, delta);

			// Do we need to launch an application?
			if (command != NULL)
			{
#if USE_VDK
				// Hide our window.
				vdkHideWindow((vdkWindow)window);
#endif
				// Launch the application.
				if (!Run(command))
				{
					LogError("Command %s could not be executed\n", command);
				}
#if USE_VDK
				// Show our window.
				vdkShowWindow((vdkWindow)window);
#endif
				// Unselect current menu item.
				LauncherAppSelect(app, -1);
			}

			// Render one frame.
			LauncherAppDraw(app, appWidth, appHeight);
			eglSwapBuffers(eglDisplay, eglSurface);
		}
	}

	// Delete the application.
	LauncherAppDestroy(app);

	eglMakeCurrent(eglDisplay, NULL, NULL, NULL);

	if (eglContext)
	{
		eglDestroyContext(eglDisplay, eglContext);
	}
	if (eglSurface)
	{
		eglDestroySurface(eglDisplay, eglSurface);
	}
	eglTerminate(eglDisplay);

#if USE_VDK
	vdkDestroyWindow((vdkWindow)window);
	vdkDestroyDisplay((vdkDisplay)display);
	vdkExit(vdk);
#else
#ifdef EGL_API_FB
	fbDestroyWindow(window);
	fbDestroyDisplay(display);
#endif
#endif

	if (!status)
	{
		// Error.
		return RETURN_ERROR;
	}
	else
	{
		// Success.
		return RETURN_SUCCESS;
	}
}

#if !USE_VDK
#ifdef EGL_API_FB
/* Run a thread get user input from stdin */
void* InputThread(void* param)
{
	int k;

	while (!finished)
	{
		k = fgetc(stdin);

		if (k != '\n')
		{
			key = k;
		}
	}

	return NULL;
}
#endif
#endif
#endif //not define ANDROID
