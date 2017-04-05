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


#include <windows.h>
#define EGL_EGLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#include <EGL\egl.h>
#include <EGL\eglext.h>
#include <GLES\gl.h>
#include <GLES\glext.h>

#define PRECISION 16
#define ONE    (1 << PRECISION)
#define ZERO 0
inline GLfixed FixedFromInt(int value) {return value << PRECISION;};

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL InitOGLES();
void Render();
HBITMAP hBitmapTex = NULL;
HDC hDC = NULL;
HWND hWnd = NULL;
int winWidth = 640;
int winHeight = 480;

EGLDisplay glesDisplay = NULL;  // EGL display
EGLSurface glesSurface = NULL;     // EGL rendering surface
EGLContext glesContext = NULL;     // EGL rendering context
EGLImageKHR eglimage = NULL;
GLuint gTexture = 0xFFFFFFFF;

TCHAR szAppName[] = TEXT("EGL_NATIVE_PIXMAP_KHR DDB"); /*The application name and the window caption*/

BOOL InitPixmap(HBITMAP Bitmap, RECT &rect, TCHAR *Text)
{
    BOOL ret =  FALSE;
    HDC hdcMem = CreateCompatibleDC(NULL);
    if (!hdcMem)
    {
        goto EXIT;
    }

    HGDIOBJ old = SelectObject(hdcMem, Bitmap);
    if (!old)
    {
        goto EXIT;
    }

    SelectObject(hdcMem, GetStockObject(GRAY_BRUSH));
    PatBlt(hdcMem, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, PATCOPY);

    SelectObject(hdcMem, GetStockObject(WHITE_PEN));
    int n;
    for (n = rect.left; n < rect.right; n += 10)
    {
        MoveToEx(hdcMem, n, rect.top, NULL);
        LineTo(hdcMem, n, rect.bottom);
    }

    for (n = rect.top; n < rect.bottom; n += 10)
    {
        MoveToEx(hdcMem, rect.left, n, NULL);
        LineTo(hdcMem, rect.right, n);
    }

    SetTextColor(hdcMem, RGB(0, 0, 0xFF));
    DrawText(hdcMem, Text, _tcsclen(Text), &rect, DT_CENTER);

    ret = TRUE;

EXIT:
    if (old)
    {
        SelectObject(hdcMem, old);
    }

    if (hdcMem)
    {
        DeleteDC(hdcMem);
    }

    return ret;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine,    int nCmdShow)
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
    wc.hCursor        = 0;
    wc.hbrBackground  = 0;
    wc.lpszMenuName    = NULL;
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

    if (gTexture != 0xFFFFFFFF)
    {
        glDeleteTextures(1, &gTexture);
    }

    if (eglimage)
    {
        eglDestroyImageKHR(glesDisplay, eglimage);
    }

    if (hBitmapTex)
    {
        DeleteObject((HBITMAP)hBitmapTex);
    }

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
        EGL_SURFACE_TYPE,   EGL_WINDOW_BIT,
        EGL_NONE,           EGL_NONE
    };

    glesDisplay = eglGetDisplay((EGLNativeDisplayType)hDC);

    if(!eglInitialize(glesDisplay, NULL, NULL))
      return FALSE;

    if(!eglChooseConfig(glesDisplay, configAttribs, &configs[0], 10,  &matchingConfigs))
     return FALSE;

    if (matchingConfigs < 1)  return FALSE;

    glesSurface = eglCreateWindowSurface(glesDisplay, configs[0], (EGLNativeWindowType)hWnd, NULL);
    if(!glesSurface) return FALSE;

    glesContext=eglCreateContext(glesDisplay,configs[0],0,NULL);

    if(!glesContext) return FALSE;

    eglMakeCurrent(glesDisplay, glesSurface, glesSurface, glesContext);

    glClearColorx(0, 0, 0, 0);
    glShadeModel(GL_SMOOTH);

    RECT r;

    GetClientRect(hWnd, &r);

    glViewport(r.left, r.top, r.right - r.left, r.bottom - r.top);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrthox(FixedFromInt(-50), FixedFromInt(50),
             FixedFromInt(-50), FixedFromInt(50),
             FixedFromInt(-50), FixedFromInt(50));
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glGenTextures(1, &gTexture);
    glBindTexture(GL_TEXTURE_2D, gTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );


    r.left = 0;
    r.right = 128;
    r.top = 0;
    r.bottom = 64;

    // Create DDB
    hBitmapTex = CreateCompatibleBitmap(hDC, r.right - r.left, r.bottom - r.top);
    if (!InitPixmap(hBitmapTex, r, TEXT("DDB PIXMAP")))
    {
          return FALSE;
    }

    eglimage = (EGLImageKHR)eglCreateImageKHR(
        glesDisplay, EGL_NO_CONTEXT, EGL_NATIVE_PIXMAP_KHR, hBitmapTex, NULL);
    if (!eglimage)
    {
        return FALSE;
    }

    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES)eglimage);

    return TRUE;
}

//----------------------------------------------------------------------------
void Render()
{
    static int rotation = 0;
    GLshort vertexArray[] = {-25,-25,0,   25,-25,0,     -25,25,0,    25,25,0};
    GLfixed texCoords[] = { ZERO, ONE,
                            ONE , ONE,
                            ZERO, ZERO,
                            ONE , ZERO,};

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glTranslatex(ZERO, ZERO, FixedFromInt(-10));
    glRotatex(FixedFromInt(rotation++), ZERO, ONE, ZERO);

    //Enable the vertices array
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_SHORT, 0, vertexArray);


    glActiveTexture(GL_TEXTURE0);
    glClientActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, gTexture);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FIXED, 0, texCoords);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);


    eglSwapBuffers(glesDisplay, glesSurface);
}
