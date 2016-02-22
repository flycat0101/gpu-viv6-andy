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


#include <windows.h>
#define GL_GLEXT_PROTOTYPES
#include <EGL\egl.h>
#include <EGL\eglext.h>
#include <GLES\gl.h>
#include <GLES\glext.h>

#define PRECISION 16
#define ONE	(1 << PRECISION)
#define ZERO 0
inline GLfixed FixedFromInt(int value) {return value << PRECISION;};

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL InitOGLES();
void Render();

HDC hDC = NULL;
HBITMAP hBitmap = NULL;
HWND hWnd = NULL;
int winWidth = 640;
int winHeight = 480;

EGLDisplay glesDisplay;  // EGL display
EGLSurface glesSurface;	 // EGL rendering surface
EGLContext glesContext;	 // EGL rendering context

TCHAR szAppName[] = TEXT("EGL DDB Pixmap"); /*The application name and the window caption*/

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine,	int nCmdShow)
{
    MSG msg;
    bool done = FALSE;
	int ret = -1;
    WNDCLASS wc;

    if(hWnd = FindWindow(szAppName, szAppName))
    {
        SetForegroundWindow((HWND)((ULONG_PTR) hWnd | 0x00000001));
        return 0;
    }

    wc.style          = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc    = (WNDPROC) WndProc;
    wc.cbClsExtra     = 0;
    wc.cbWndExtra     = 0;
    wc.hInstance      = hInstance;
    wc.hIcon          = LoadIcon(hInstance, NULL);
    wc.hCursor	    = 0;
    wc.hbrBackground  = 0;
    wc.lpszMenuName	= NULL;
    wc.lpszClassName  = szAppName;

    if(!RegisterClass(&wc))
    {
        return FALSE;
    }

    RECT rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, FALSE);

	rect.left = (rect.right - winWidth) / 2;
	rect.top  = (rect.bottom - winHeight) / 2;
	rect.right  = rect.left + winWidth;
	rect.bottom = rect.top + winHeight;

	UINT style = WS_VISIBLE | WS_CAPTION | WS_SYSMENU;

    AdjustWindowRectEx(&rect, style, FALSE, 0);

    hWnd = CreateWindow(szAppName,
                      szAppName,
                      style,
                      rect.left,
                      rect.top,
                      rect.right - rect.left,
					  rect.bottom - rect.top,
                      NULL, NULL,
                      hInstance, NULL);
    if(!hWnd)
    {
		UnregisterClass(szAppName, hInstance);
        return FALSE;
    }

    //Bring the window to front, focus it and refresh it
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    hDC = GetDC(hWnd);

	// Create DDB
	hBitmap = CreateCompatibleBitmap(hDC, winWidth, winHeight);
    if(!hBitmap)
	{
		goto EXIT;
	}

    //OpenGL ES Initialization
    if(!InitOGLES())
	{
		goto EXIT;
	}

    //Message Loop
    while(!done)
    {
        if(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
        {
          if(msg.message==WM_QUIT)
            done=TRUE;
  	      else
          {
  	        TranslateMessage(&msg);
  	        DispatchMessage(&msg);
  	      }
        }
        else
  	    {
  	        Render();
  	    }
    }

	ret = 0;

EXIT:

    if(glesDisplay)
    {
        eglMakeCurrent(glesDisplay, NULL, NULL, NULL);

        if(glesContext)
		{
			eglDestroyContext(glesDisplay, glesContext);
		}

		if(glesSurface)
		{
			eglDestroySurface(glesDisplay, glesSurface);
		}

        eglTerminate(glesDisplay);
    }

    if (hBitmap)
    {
      DeleteObject((HBITMAP)hBitmap);
    }

	if (hDC)
	{
		ReleaseDC(hWnd, hDC);
	}

	if (hWnd)
	{
		DestroyWindow(hWnd);
	}

    UnregisterClass(szAppName, hInstance);

    return ret;
}

//----------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
        ValidateRect(hWnd,NULL);
	    break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    };

    return DefWindowProc(hWnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------

BOOL InitOGLES()
{
    EGLConfig configs[10];
    EGLint matchingConfigs;

    const EGLint configAttribs[] =
    {
        EGL_RED_SIZE,       8,
        EGL_GREEN_SIZE,     8,
        EGL_BLUE_SIZE,      8,
        EGL_ALPHA_SIZE,     EGL_DONT_CARE,
        EGL_DEPTH_SIZE,     16,
        EGL_STENCIL_SIZE,   EGL_DONT_CARE,
        EGL_SURFACE_TYPE,   EGL_PIXMAP_BIT,
        EGL_NONE,           EGL_NONE
    };

    glesDisplay = eglGetDisplay(hDC);

    if(!eglInitialize(glesDisplay, NULL, NULL))
      return FALSE;

    if(!eglChooseConfig(glesDisplay, configAttribs, &configs[0], 10,  &matchingConfigs))
     return FALSE;

    if (matchingConfigs < 1)  return FALSE;

    glesSurface = eglCreatePixmapSurface(glesDisplay, configs[0], (EGLNativePixmapType)hBitmap, NULL);
    if(!glesSurface) return FALSE;

    glesContext=eglCreateContext(glesDisplay,configs[0],0,NULL);

    if(!glesContext) return FALSE;

    eglMakeCurrent(glesDisplay, glesSurface, glesSurface, glesContext);

    glClearColorx(0, 0, 0, 0);
    glShadeModel(GL_SMOOTH);

    glViewport(0, 0, winWidth, winHeight);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrthox(FixedFromInt(-50), FixedFromInt(50),
             FixedFromInt(-50), FixedFromInt(50),
             FixedFromInt(-50), FixedFromInt(50));
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    return TRUE;
}

//----------------------------------------------------------------------------
void Render()
{
    static int rotation = 0;
    GLshort vertexArray[] = {-25,-25,0,   25,-25,0,     0,25,0 };
    GLubyte colorArray[] = {255,0,0,0,   0,255,0,0,    0,0,255,0};

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glTranslatex(ZERO, ZERO, FixedFromInt(-10));
    glRotatex(FixedFromInt(rotation++), ZERO, ONE, ZERO);

    //Enable the vertices array
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_SHORT, 0, vertexArray);

    //Enable the vertex color array
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4,GL_UNSIGNED_BYTE, 0, colorArray);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    if (!eglCopyBuffers(glesDisplay, glesSurface, hBitmap))
	{
		return;
	}

    DIBSECTION dibs;

	memset(&dibs, 0, sizeof(BITMAPINFO));

	dibs.dsBmih.biSize = sizeof(dibs.dsBmih);

    if (GetObject(hBitmap,sizeof(dibs),&dibs))
    {
		if (!dibs.dsBm.bmBits)
		{
			HDC hMemDC = CreateCompatibleDC(hDC);

			HGDIOBJ hOldBitmap = SelectObject(hMemDC, hBitmap);

			RECT rect;
			GetClientRect(hWnd, &rect);

			StretchBlt(
					hDC,
					rect.left, rect.top,
					rect.right - rect.left, rect.bottom - rect.top,
					hMemDC,
					0, 0, dibs.dsBm.bmWidth, dibs.dsBm.bmHeight,
					SRCCOPY
					);

			SelectObject(hMemDC, hOldBitmap);

			DeleteDC(hMemDC);
		}
    }
}
