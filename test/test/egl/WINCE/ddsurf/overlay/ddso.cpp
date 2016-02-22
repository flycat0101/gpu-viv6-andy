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


#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
//-----------------------------------------------------------------------------
// Include files
//-----------------------------------------------------------------------------
#include <windows.h>
#include <ddraw.h>
#define GL_GLEXT_PROTOTYPES
#include <EGL\egl.h>
#include <EGL\eglext.h>
#include <GLES\gl.h>
#include <GLES\glext.h>

#define RAND_INT(x) (Random() % x)
#define RANDOM_VELOCITY() (int)(((RAND_INT(5)+3)*2))

#define TIMER_ID            1
#define TIMER_RATE          100
#define TIMER_ID2           2
#define TIMER_RATE2         300

#define PRECISION 16
#define ONE	(1 << PRECISION)
#define ZERO 0
inline GLfixed FixedFromInt(int value) {return value << PRECISION;};
//-----------------------------------------------------------------------------
// Local definitions
//-----------------------------------------------------------------------------
#define NAME                TEXT("DDS_OVERLAY")
#define TITLE               TEXT("Direct Draw Surface Example 2")

//-----------------------------------------------------------------------------
// Global data
//-----------------------------------------------------------------------------
static LPDIRECTDRAW                g_pDD = NULL;        // DirectDraw object
static LPDIRECTDRAWSURFACE         g_pDDSPrimary = NULL;// DirectDraw primary surface
static LPDIRECTDRAWSURFACE         g_pDDSOverlay = NULL;   // DirectDraw back surface
static RECT                        g_OverlayRect = {0, 0, 320, 240};
static HWND                        g_Wnd = NULL;
static HDC                         g_Hdc = NULL;
static BOOL                        g_bActive = FALSE;

// OpenGL variables
static EGLDisplay glesDisplay;   // EGL display
static EGLSurface glesSurface;	 // EGL rendering surface
static EGLContext glesContext;	 // EGL rendering context

static DDPIXELFORMAT ddpfOverlayFormats[] = {
    {sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 16,  0xF800, 0x07e0, 0x001F, 0},         // 16-bit RGB 5:6:5
    {sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 32,  0xFF0000, 0xFF00, 0xFF, 0},         // 16-bit RGB 5:6:5
    {sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 16,  0x7C00, 0x03e0, 0x001F, 0}          // 16-bit RGB 5:5:5
};

void Render();
static void MoveOverlay();

static void
ReleaseAllDDObjects(void)
{
    if (g_pDDSOverlay != NULL)
    {
        g_pDDSOverlay->UpdateOverlay(NULL, g_pDDSPrimary, NULL, DDOVER_HIDE, NULL);
        g_pDDSOverlay->Release();
        g_pDDSOverlay = NULL;
    }
    if (g_pDDSPrimary != NULL)
    {
        g_pDDSPrimary->Release();
        g_pDDSPrimary = NULL;
    }
    if (g_pDD != NULL)
    {
        g_pDD->Release();
        g_pDD = NULL;
    }
}

static HRESULT
FailOutput(HRESULT hRet, LPCTSTR szError,...)
{
    TCHAR                       szBuff[128];
    va_list                     vl;

    va_start(vl, szError);

    StringCchVPrintf(szBuff, 128, szError, vl);

    OutputDebugString(szBuff);

    va_end(vl);
    return hRet;
}
//-----------------------------------------------------------------------------
// Name: WindowProc()
// Desc: The Main Window Procedure
//-----------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
        ValidateRect(hWnd,NULL);
	    break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_TIMER:
        // Update and flip surfaces
        if (g_bActive && TIMER_ID == wParam)
        {
            Render();
        }

        if (g_bActive && TIMER_ID2 == wParam)
        {
            MoveOverlay();
        }

        break;
    };

    return DefWindowProc(hWnd, message, wParam, lParam);
}

static void
MoveOverlay()
{
    static int nOverlayXPos = 0;
    static int nOverlayYPos = 0;
    static int nOverlayXVel = 5;
    static int nOverlayYVel = 8;
    int nOverlayWidth = g_OverlayRect.right - g_OverlayRect.left;
    int nOverlayHeight = g_OverlayRect.bottom - g_OverlayRect.top;

    if (!g_pDDSOverlay)
        return;

    // Add the current velocity vectors to the position.
    nOverlayXPos += nOverlayXVel;
    nOverlayYPos += nOverlayYVel;

    // Check to see if this new position puts the overlay off the edge of the screen.
    // SetOverlayPosition() won't like that.

    // Have we gone off the left edge?

    if (nOverlayXPos < 0) {
	    nOverlayXPos = 0;
	    nOverlayXVel = RANDOM_VELOCITY();
    }

    // Have we gone off the right edge?

    if ((nOverlayXPos + nOverlayWidth) >  GetSystemMetrics(SM_CXSCREEN)){
	    nOverlayXPos = GetSystemMetrics(SM_CXSCREEN) - nOverlayWidth;
	    nOverlayXVel = -RANDOM_VELOCITY();
    }

    // Have we gone off the top edge?

    if (nOverlayYPos < 0) {
	    nOverlayYPos = 0;
	    nOverlayYVel = RANDOM_VELOCITY();
    }

    // Have we gone off the bottom edge?

    if ( (nOverlayYPos + nOverlayHeight) >  GetSystemMetrics(SM_CYSCREEN)) {
	    nOverlayYPos = GetSystemMetrics(SM_CYSCREEN) - nOverlayHeight;
	    nOverlayYVel = -RANDOM_VELOCITY();
    }

    // Set the overlay to it's new position.

    g_pDDSOverlay->SetOverlayPosition(nOverlayXPos, nOverlayYPos);
}

static HRESULT
CreateDDObjects(void)
{
    HRESULT hRet;
    DDSURFACEDESC ddsd;

    if (g_Wnd == NULL)
    {
        return FailOutput(E_FAIL, TEXT("Window not initialized"));
    }

    ///////////////////////////////////////////////////////////////////////////
    // Create the main DirectDraw object
    ///////////////////////////////////////////////////////////////////////////
    hRet = DirectDrawCreate(NULL, &g_pDD, NULL);
    if (hRet != DD_OK)
        return FailOutput(hRet, TEXT("DirectDrawCreate FAILED"));

    // Get exclusive mode
    hRet = g_pDD->SetCooperativeLevel(g_Wnd, DDSCL_NORMAL);
    if (hRet != DD_OK)
        return FailOutput(hRet, TEXT("SetCooperativeLevel FAILED"));

    // Create the primary surface with 1 back buffer
    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    hRet = g_pDD->CreateSurface(&ddsd, &g_pDDSPrimary, NULL);
    if (hRet != DD_OK)
    {
        if (hRet == DDERR_NOFLIPHW)
            return FailOutput(hRet, TEXT("******** Display driver doesn't support flipping surfaces. ********"));

        return FailOutput(hRet, TEXT("CreateSurface FAILED"));
    }

    // Create overlay surfaces
    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.ddsCaps.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_FLIP;
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_BACKBUFFERCOUNT |
                   DDSD_PIXELFORMAT;
	ddsd.dwWidth = g_OverlayRect.right;
	ddsd.dwHeight = g_OverlayRect.bottom;
    ddsd.dwBackBufferCount = 1;
    ddsd.ddpfPixelFormat = ddpfOverlayFormats[0];

    hRet = g_pDD->CreateSurface(&ddsd, &g_pDDSOverlay, NULL);
    if (hRet != DD_OK)
        return FailOutput(hRet, TEXT("Create Overlay FAILED"));

    hRet = g_pDDSOverlay->UpdateOverlay(&g_OverlayRect, g_pDDSPrimary, &g_OverlayRect, DDOVER_SHOW, NULL);
    if (hRet != DD_OK)
        return FailOutput(hRet, TEXT("Update Overlay FAILED"));

    if (TIMER_ID != SetTimer(g_Wnd, TIMER_ID, TIMER_RATE, NULL))
        return FailOutput(E_FAIL, TEXT("SetTimer FAILED"));

    if (TIMER_ID2 != SetTimer(g_Wnd, TIMER_ID2, TIMER_RATE2, NULL))
        return FailOutput(E_FAIL, TEXT("SetTimer2 FAILED"));

    return DD_OK;
}

static HRESULT
InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    HWND                        hWnd;
    WNDCLASS                    wc;

    // Set up and register window class
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH )GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = NAME;
    RegisterClass(&wc);

    // Create a window
    hWnd = CreateWindowEx(WS_EX_TOPMOST,
                          NAME,
                          TITLE,
                          WS_POPUP,
                          0,
                          0,
                          GetSystemMetrics(SM_CXSCREEN),
                          GetSystemMetrics(SM_CYSCREEN),
                          NULL,
                          NULL,
                          hInstance,
                          NULL);
    if (!hWnd)
        return FALSE;

    SetFocus(hWnd);

    g_Wnd = hWnd;

    return S_OK;
}

HRESULT InitOGLES()
{
    EGLConfig configs[10];
    EGLint matchingConfigs;

    /*configAttribs is a integers list that holds the desired format of
     our framebuffer. We will ask for a framebuffer with 24 bits of
     color and 16 bits of z-buffer. We also ask for a window buffer, not
     a pbuffer or pixmap buffer*/
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

    glesDisplay = eglGetDisplay((EGLNativeDisplayType)g_Hdc);	 //Ask for an available display

    //Display initialization (we don't care about the OGLES version numbers)
    if(!eglInitialize(glesDisplay, NULL, NULL))
      return FailOutput(E_INVALIDARG, TEXT("eglInitialize FAILED"));

    /*Ask for the framebuffer configuration that best fits our
    parameters. At most, we want 10 configurations*/
    if(!eglChooseConfig(glesDisplay, configAttribs, &configs[0], 10,  &matchingConfigs))
        return FailOutput(E_INVALIDARG, TEXT("eglChooseConfig FAILED"));

    //If there isn't any configuration enough good
    if (matchingConfigs < 1)
        return FailOutput(E_INVALIDARG, TEXT("eglChooseConfig FAILED: not matched"));

    /*eglCreateWindowSurface creates an onscreen EGLSurface and returns
    a handle  to it. Any EGL rendering context created with a
    compatible EGLConfig can be used to render into this surface.*/
    glesSurface = eglCreateWindowSurface(glesDisplay, configs[0], (EGLNativeWindowType)g_pDDSOverlay, NULL);
    if(!glesSurface)
        return FailOutput(E_INVALIDARG, TEXT("eglCreateWindowSurface FAILED"));

    // Let's create our rendering context
    glesContext=eglCreateContext(glesDisplay,configs[0],0,NULL);
    if(!glesContext)
        return FailOutput(E_INVALIDARG, TEXT("eglCreateContext FAILED"));

    //Now we will activate the context for rendering
    eglMakeCurrent(glesDisplay, glesSurface, glesSurface, glesContext);

    /*Remember: because we are programming for a mobile device, we cant
    use any of the OpenGL ES functions that finish in 'f', we must use
    the fixed point version (they finish in 'x'*/
    glClearColorx(0, 0, 0, 0);
    glShadeModel(GL_SMOOTH);

    glViewport(g_OverlayRect.left, g_OverlayRect.top,
        g_OverlayRect.right - g_OverlayRect.left, g_OverlayRect.bottom - g_OverlayRect.top);

    /*Setup of the projection matrix. We will use an ortho cube centered
    at (0,0,0) with 100 units of edge*/
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrthox(FixedFromInt(-50), FixedFromInt(50),
             FixedFromInt(-50), FixedFromInt(50),
             FixedFromInt(-50), FixedFromInt(50));
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    return S_OK;
}

//----------------------------------------------------------------------------
void Render()
{
    static int rotation = 0;
    				        /* Vertex 1    Vertex 2 	 Vertex 3   Vertex 4*/
    GLshort vertexArray[] = {-25,-25,0,   25,-25,0,     -25,25,0,    25,25,0};
    GLubyte colorArray[] = {255,0,0,0,   0,255,0,0,    0,0,255,0,   128,128,128,0};

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glTranslatex(ZERO, ZERO, FixedFromInt(-10));
    glRotatex(FixedFromInt(rotation++), ZERO, ONE, ZERO);

    //Enable the vertices array
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_SHORT, 0, vertexArray);
    //3 = XYZ coordinates, GL_SHORT = data type, 0 = 0 stride bytes

    //Enable the vertex color array
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4,GL_UNSIGNED_BYTE, 0, colorArray);
    //4 = RGBA format, GL_UNSIGNED_BYTE = data type,0=0 stide    bytes

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    /*We want draw triangles, 0 = first element(vertice), 3 = number of
      items (vertices) to draw from the array*/
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    eglSwapBuffers(glesDisplay, glesSurface);
}
//----------------------------------------------------------------------------
void DenitOGLES()
{
    if(glesDisplay)
    {
      eglMakeCurrent(glesDisplay, NULL, NULL, NULL);
      if(glesContext) eglDestroyContext(glesDisplay, glesContext);
      if(glesSurface) eglDestroySurface(glesDisplay, glesSurface);
      eglTerminate(glesDisplay);
    }
}

int PASCAL
WinMain(HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPWSTR lpCmdLine,
        int nCmdShow)
{
    MSG                         msg;

    if (InitWindow(hInstance, nCmdShow) != S_OK)
        return FALSE;

    g_Hdc = GetDC(g_Wnd);

    if (CreateDDObjects() != DD_OK)
    {
        ReleaseDC(g_Wnd, g_Hdc);
        DestroyWindow(g_Wnd);
        UnregisterClass(NAME, hInstance);
        return FALSE;
    }

    if(InitOGLES() != S_OK)
    {
        ReleaseAllDDObjects();
        ReleaseDC(g_Wnd, g_Hdc);
        DestroyWindow(g_Wnd);
        UnregisterClass(NAME, hInstance);
        return FALSE;
    }

    g_bActive = TRUE;
    while(TRUE)
    {
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                g_bActive = FALSE;
                break;
            }

            if (msg.message == WM_KEYDOWN)
            {
                if (msg.wParam == 0x1B)
                    PostQuitMessage(0);
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    DenitOGLES();
    ReleaseAllDDObjects();
    ReleaseDC(g_Wnd, g_Hdc);
    DestroyWindow(g_Wnd);
    UnregisterClass(NAME, hInstance);

    return msg.wParam;
}
