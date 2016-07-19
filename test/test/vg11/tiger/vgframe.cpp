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


#include "PreComp.h"
#include "vgframe.h"
#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <assert.h>
#if (defined(LINUX) || defined(__QNXNTO__))
#include <signal.h>
#include <sys/time.h>
#endif


#ifdef MEM_LEAK_CHECK
#include <crtdbg.h>
#endif

NativeWindowType window;
NativeDisplayType display;

#if USE_VDK
vdkEGL egl;
void handleEvent(vdkEvent *msg);
#else
    #ifdef LINUX
        #ifndef EGL_API_FB
            Display *xDisplay;
            Window xWindow;
            int exposed = 1;
            XSetWindowAttributes  xswa;
            XEvent  event;
        #endif
    #else
    #endif
#endif

// EGL variables
EGLDisplay eglDisplay;  // EGL display
EGLSurface eglSurface;     // EGL rendering surface
EGLContext eglContext;     // EGL rendering context
EGLConfig  eglConfig;

#define MAX_PATH_LEN 260
char appName[260] = {'\0'};
char filePath[260] = {'\0'};

appAttribs cmdAttrib = {640, 480, 8, 8, 8, 0, -1, -1, -1, 2, 100, 0, 1};
                       /*w,   h,  r, g, b, a, depth, stencil, samples, openvgbit, frameCount, save, fps*/

/* for showFPS */
static int numFrame = 0;  // count of frames we have run in fact
unsigned int startTime = 0;
unsigned int diffTime = 0;
unsigned int get_time(void);
void printFPS(int frames, unsigned int time);

int ParseArgs(const char *arguments);
void WriteBMPFile (char * file_name, VGubyte * pbuf, unsigned int size);
int SaveBMP(char *image_name, VGubyte* p, VGint width, VGint height);
void SaveResult(char * file_name, int frameNo);

bool done = false;

void catchKill(int signal)
{
    vfFRAME_Clean();
}

bool vgfCreateWindow()
{
#if USE_VDK
    egl.vdk = vdkInitialize();
    if (egl.vdk == NULL)
    {
      printf("%s", "vdkIntialize() failed.\n");
      return false;
    }
    egl.display = vdkGetDisplay(egl.vdk);
    if (egl.display == NULL)
    {
#ifndef ANDROID
      printf("%s", "vdkGetDisplay() failed.\n");
      return false;
#  endif
    }
    display = (NativeDisplayType)(egl.display);
    egl.window = vdkCreateWindow(egl.display, -1, -1, cmdAttrib.winWidth, cmdAttrib.winHeight);
    if (egl.window == NULL)
    {
      printf("%s", "vdkCreateWindow() falied.\n");
      return false;
    }
    window = (NativeWindowType)(egl.window);
    //Bring the window to front, focus it and refresh it
    vdkShowWindow(egl.window);
    vdkSetWindowTitle(egl.window, appName);
#else
    #ifdef LINUX
        #if EGL_API_FB
            display = fbGetDisplay(NULL);
            if (display == NULL)
            {
              printf("%s","fbGetDisplay() failed.\n");
              return false;
            }

            window = fbCreateWindow(display, 0, 0, -1, -1);
            if (window == NULL)
            {
              printf("%s", "fbCreateWindow() failed.\n");
              return false;
            }

            int x, y;
            fbGetWindowGeometry(window, &x, &y, &cmdAttrib.winWidth, &cmdAttrib.winHeight);
        #else
            XSetWindowAttributes  xswa;
            Screen  *screen;
            XEvent  event;
            int exposed = 0;
            xDisplay = XOpenDisplay(NULL);
            if (xDisplay == NULL)
            {
              printf("%s", "XOpenDisplay fail\n");
              return false;
            }
            screen = XDefaultScreenOfDisplay(xDisplay);
            xswa.background_pixel = XWhitePixelOfScreen(screen);
            xswa.event_mask = KeyPressMask | StructureNotifyMask  | VisibilityChangeMask  |ResizeRedirectMask | ExposureMask | ButtonPressMask;
            xWindow = XCreateWindow(xDisplay, XRootWindowOfScreen(screen),
                                cmdAttrib.winWidth, cmdAttrib.winHeight, cmdAttrib.winWidth, cmdAttrib.winHeight, 0,
                                DefaultDepthOfScreen(screen), InputOutput,
                                DefaultVisualOfScreen(screen), CWEventMask | CWBackPixel, &xswa);
            if (xWindow == NULL)
            {
              printf("%s", "XCreateWindow() failed.\n");
              return false;
            }
            XMapWindow(xDisplay, xWindow);
            signal(SIGINT | SIGKILL | SIGTERM, catchKill);
            display = (NativeDisplayType)xDisplay;
            window = (NativeWindowType)xWindow;
        #endif
    #else

    #endif /*LINUX*/
#endif
return true;
}

void vgfDestroyWindow()
{
#if USE_VDK
    vdkDestroyWindow(egl.window);
    vdkDestroyDisplay(egl.display);
    vdkExit(egl.vdk);
#else
    #ifdef LINUX
        #ifdef EGL_API_FB
            if (window != NULL)
            {
                fbDestroyWindow(window);
            }

            if (display != NULL)
            {
                fbDestroyDisplay(display);
            }
        #else
            if (xWindow != NULL)
            {
                XDestroyWindow(xDisplay, xWindow);
            }
            if (xDisplay != NULL)
            {
                XCloseDisplay(xDisplay);
            }
        #endif
    #else

    #endif
#endif
}

void RenderLoop()
{
#if USE_VDK
   //Message Loop
    vdkEvent msg;
    while(!done)
    {
        if (vdkGetEvent(egl.window, &msg))
        {
            handleEvent(&msg);
        }
        else
        {
            if ((cmdAttrib.fps-1) == numFrame)
            {
                startTime = get_time();
            }
            vfFRAME_Render();
            eglSwapBuffers(eglDisplay, eglSurface);

            numFrame++;

            if (--cmdAttrib.frameCount <= 0)
            {
                done = true;
            }
        }

        // save bmp for each frame
        if (cmdAttrib.save)
        {
            SaveResult(filePath, numFrame);
        }
    }
#else
    #ifdef LINUX
        #ifdef EGL_API_FB
            for (int i = 0; i < cmdAttrib.frameCount; ++i)
            {
                if ((cmdAttrib.fps-1) == numFrame)
                {
                    startTime = get_time();
                }
                vfFRAME_Render();
                eglSwapBuffers(eglDisplay, eglSurface);
                numFrame++;
                // save bmp for each frame
                if (cmdAttrib.save)
                {
                    SaveResult(filePath, numFrame);
                }
                if (--cmdAttrib.frameCount <= 0)
                {
                    break;
                }
            }
        #else
            //Message Loop
            while(!done)
            {
                if (XCheckWindowEvent(xDisplay, xWindow, xswa.event_mask, &event) == true)
                {
                    switch (event.type)
                    {
                        case    Expose:
                            exposed = 1;
                            break;
                        case    KeyPress:
                        case    ButtonPress:
                            done = true;
                            break;
                        default:
                            break;
                    }
                }
                else {
                    if (exposed)
                    {
                        if ((cmdAttrib.fps-1) == numFrame)
                        {
                            startTime = get_time();
                        }
                        vfFRAME_Render();
                        eglSwapBuffers(eglDisplay, eglSurface);

                        numFrame++;

                        if (--cmdAttrib.frameCount <= 0)
                        {
                            done = true;
                        }
                                if (cmdAttrib.save)
                                {
                                           SaveResult(filePath, numFrame);
                                }
                    }
                }
            }
        #endif
    #else

    #endif
#endif
}

int main(int argc, char *argv[])
{
    char param[512] = {'\0'};
    int i;
    for (i=1; i < argc; i++)
    {
        strcat(param, argv[i]);
        strcat(param, " ");
    }
    strcpy(filePath, argv[0]) ;

#ifdef MEM_LEAK_CHECK
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) /*| _CRTDBG_CHECK_ALWAYS_DF*/ | _CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    //_CrtSetBreakAlloc(1228);
#endif
    if (!ParseArgs(param))
    {
        printf("%s","input parameter error\n");
        return false;
    }
    else
    {
        vfFRAME_AppInit(&cmdAttrib.winWidth, &cmdAttrib.winHeight, NULL);
    }
    if (!vgfCreateWindow())
    {
      printf("%s", "CreateWindow() failed.\n");
      return false;
    }
    //EGL Initialization
    if(!InitEGL())
    {
        printf("%s","InitEGL fail!\n");
        return false;
    }

    if (!vfFRAME_InitVG())
    {
        printf("%s","InitVG fail!\n");
        return false;
    }

    RenderLoop();

    if (cmdAttrib.fps)
    {
        diffTime = get_time() - startTime;
        printFPS(numFrame-cmdAttrib.fps+1, diffTime);
    }


    // clean the resource of VG
    vfFRAME_Clean();

    // Clean EGL and file
    Release();

    //We have to destroy the window too
    vgfDestroyWindow();


}

#if USE_VDK
void handleEvent(vdkEvent *msg)
{
    VGint mouseX;
    VGint mouseY;
    switch (msg->type)
    {
        case VDK_CLOSE:
            done = true;
            break;
        case VDK_KEYBOARD:
            keyprocessor(msg->data.keyboard.scancode);
            if (msg->data.keyboard.scancode == VDK_ESCAPE)
            {
                done = true;
            }
            break;
        case VDK_POINTER:
            mouseX = msg->data.pointer.x;
            mouseY = cmdAttrib.winHeight - msg->data.pointer.y;
            mouseMoveProcessor(mouseX, mouseY, *msg);
            break;
        case VDK_BUTTON:
            mouseX = msg->data.button.x;
            mouseY = msg->data.button.y;
            mouseKeyProcessor(mouseX, mouseY, *msg);
            break;
        default:
            break;
    }
}
#endif

/*
 * Parse the command line.
*/
int ParseArgs(const char *arguments)
{
    int     len = 0;
    int     i = 0;
    int     result = 1;
    char strTemp[256];

    len = (int)strlen(arguments);

    if (len > 0)
    {
        while (i < len)
        {
            if (arguments[i] == ' ')
            {
                ++i;
                continue;
            }

            if ((arguments[i] == '-'))
            {
                ++i;
                getArgument(arguments, strTemp, &i, len);
                if (strcmp(strTemp, "w") == 0)
                {
                    ++i;
                    getArgument(arguments, strTemp, &i, len);
                    cmdAttrib.winWidth = atoi(strTemp);
                }else if (strcmp(strTemp, "h") == 0)
                {
                    ++i;
                    getArgument(arguments, strTemp, &i, len);
                    cmdAttrib.winHeight = atoi(strTemp);
                }else if (strcmp(strTemp, "rgba") == 0)
                {
                    int rgba = 0;
                    ++i;
                    getArgument(arguments, strTemp, &i, len);
                    rgba = atoi(strTemp);
                    if ((rgba < 1000) || (rgba > 10000))
                    {
                        printf("only need four digits for rgba");
                        result = 0;
                    }
                    else
                    {
                        cmdAttrib.redSize = rgba/1000;
                        cmdAttrib.greenSize = (rgba%1000)/100;
                        cmdAttrib.blueSize = (rgba%100)/10;
                        cmdAttrib.alphaSize = rgba%10;
                    }
                }else if (strcmp(strTemp, "depth") == 0)
                {
                    ++i;
                    getArgument(arguments, strTemp, &i, len);
                    cmdAttrib.depthSize = atoi(strTemp);
                }else if (strcmp(strTemp, "stencil") == 0)
                {
                    ++i;
                    getArgument(arguments, strTemp, &i, len);
                    cmdAttrib.stencilSize = atoi(strTemp);
                }else if (strcmp(strTemp, "samples") == 0)
                {
                    ++i;
                    getArgument(arguments, strTemp, &i, len);
                    cmdAttrib.samples = atoi(strTemp);
                }else if (strcmp(strTemp, "openvgbit") == 0)
                {
                    int openvgbit = 0;
                    ++i;
                    getArgument(arguments, strTemp, &i, len);
                    openvgbit = atoi(strTemp);
                    if (openvgbit == 0)
                        cmdAttrib.openvgbit = EGL_DONT_CARE;
                    else
                        cmdAttrib.openvgbit = EGL_OPENVG_BIT;
                }else if (strcmp(strTemp, "frameCount") == 0)
                {
                    ++i;
                    getArgument(arguments, strTemp, &i, len);
                    cmdAttrib.frameCount = atoi(strTemp);
                }else if (strcmp(strTemp, "save") == 0)
                {
                    ++i;
                    getArgument(arguments, strTemp, &i, len);
                    cmdAttrib.save = atoi(strTemp);
                }else if (strcmp(strTemp, "fps") == 0)
                {
                    ++i;
                    getArgument(arguments, strTemp, &i, len);
                    cmdAttrib.fps = atoi(strTemp);
                }else
                {
                    printf("can't recognise command parameter");
                    result = 0;
                }
            }
            else
            {
                result = 0;
                break;
            }
        }
    }

    if ((cmdAttrib.save != 0) && (cmdAttrib.fps != 0))
    {
        printf("parameter -save and -fps can't work simultaneity ");
        result = 0;
    }

    return result;
}


/*
 * Initialize egl.
*/
bool InitEGL()
{
    EGLConfig configs[10];
    EGLint matchingConfigs, i;

    static EGLint configAttrib[] =
    {
        EGL_RED_SIZE,            cmdAttrib.redSize,
        EGL_GREEN_SIZE,         cmdAttrib.greenSize,
        EGL_BLUE_SIZE,            cmdAttrib.blueSize,
        EGL_ALPHA_SIZE,         cmdAttrib.alphaSize,
        EGL_DEPTH_SIZE,         cmdAttrib.depthSize,
        EGL_STENCIL_SIZE,       cmdAttrib.stencilSize,
        EGL_SURFACE_TYPE,        EGL_WINDOW_BIT,
        EGL_SAMPLES,            cmdAttrib.samples,
        EGL_RENDERABLE_TYPE,    EGL_OPENVG_BIT, /* cmdAttrib.openvgbit, */
        EGL_NONE
    };

    //Ask for an available display
    eglDisplay = eglGetDisplay(display);
    if (eglDisplay == NULL)
    {
        printf("%s","eglGetDisplay fail.\n");
        return false;
    }

    //Display initialization (we don't care about the OGLES version numbers)
    if(!eglInitialize(eglDisplay, NULL, NULL))
        return false;

    eglBindAPI(EGL_OPENVG_API);

    /*Ask for the framebuffer configuration that best fits our
    parameters. At most, we want 10 configurations*/
    if(!eglChooseConfig(eglDisplay, configAttrib, &configs[0], 10,  &matchingConfigs))
        return false;
    if (matchingConfigs < 1)
    {
        printf("%s","no available config is choosed.\n");
        return false;
    }

    for (i = 0; i < matchingConfigs; i++)
    {
        int redSize = 0;
        int greenSize = 0;
        int blueSize = 0;
        int alphaSize = 0;
        int depthSize = 0;
        int stencilSize = 0;
        int samples = 0;
        int id = 0;
        int openvgbit = 0;

        eglGetConfigAttrib(eglDisplay, configs[i], EGL_RED_SIZE, &redSize);
        eglGetConfigAttrib(eglDisplay, configs[i], EGL_GREEN_SIZE, &greenSize);
        eglGetConfigAttrib(eglDisplay, configs[i], EGL_BLUE_SIZE, &blueSize);
        eglGetConfigAttrib(eglDisplay, configs[i], EGL_ALPHA_SIZE, &alphaSize);
        eglGetConfigAttrib(eglDisplay, configs[i], EGL_DEPTH_SIZE, &depthSize);
        eglGetConfigAttrib(eglDisplay, configs[i], EGL_STENCIL_SIZE, &stencilSize);
        eglGetConfigAttrib(eglDisplay, configs[i], EGL_SAMPLES, &samples);
        eglGetConfigAttrib(eglDisplay, configs[i], EGL_RENDERABLE_TYPE, &openvgbit);
        eglGetConfigAttrib(eglDisplay, configs[i], EGL_CONFIG_ID, &id);

        if (((cmdAttrib.redSize != -1)&&(redSize != cmdAttrib.redSize)) ||
            ((cmdAttrib.greenSize != -1)&&(greenSize != cmdAttrib.greenSize)) ||
            ((cmdAttrib.blueSize != -1)&&(blueSize != cmdAttrib.blueSize)) ||
            ((cmdAttrib.alphaSize != -1)&&(alphaSize != cmdAttrib.alphaSize)) ||
            ((cmdAttrib.depthSize != -1)&&(depthSize != cmdAttrib.depthSize)) ||
            ((cmdAttrib.stencilSize != -1)&&(stencilSize != cmdAttrib.stencilSize)) ||
            ((cmdAttrib.samples != -1)&&(samples != cmdAttrib.samples))
            )
        {
            continue;
        }else
        {
            eglConfig = configs[i];
            if (eglConfig != NULL)
            {
                printf("id=%d, a,b,g,r=%d,%d,%d,%d, d,s=%d,%d, AA=%d,openvgbit=%d\n",
                        id, alphaSize, blueSize, greenSize, redSize, depthSize, stencilSize,
                        samples, openvgbit);
            }

            break;
        }
    }

    if (eglConfig == NULL)
    {
        printf("%s","no available config is choosed.\n");
        return false;
    }

    /*eglCreateWindowSurface creates an onscreen EGLSurface and returns
    a handle  to it. Any EGL rendering context created with a
    compatible EGLConfig can be used to render into this surface.*/
    eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, window, NULL);
    if(!eglSurface)
    {
        printf("%s","eglCreateWindowSurface fail.\n");
        return false;
    }

    // Let's create our rendering context
    eglContext=eglCreateContext(eglDisplay, eglConfig, 0, NULL);

    if(!eglContext)
    {
        printf("%s","eglCreateContext fail.\n");
        return false;
    }

    //Now we will activate the context for rendering
    eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);

    return true;
}

/*
 * Clean up we have done.
*/
void Release()
{
    // release EGL
    if(eglDisplay)
    {
        eglMakeCurrent(eglDisplay, NULL, NULL, NULL);
        if(eglContext)
            eglDestroyContext(eglDisplay, eglContext);
        if(eglSurface)
            eglDestroySurface(eglDisplay, eglSurface);
        eglTerminate(eglDisplay);
    }
}


/*
 * get value of cmd argument.
*/
void getArgument(const char *arguments, char *dest, int *i, int len)
{
    int j = 0;
    while (arguments[*i] == ' ')
    {
        ++(*i);
    }

    while (arguments[*i] != ' ' && (*i) < len)
    {
        dest[j] = arguments[*i];
        ++j;
        ++(*i);
    }
    dest[j] = '\0';
}

unsigned int get_time(void)
{
#if (defined(LINUX) || defined(__QNXNTO__))
    timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_usec / 1000) + (tv.tv_sec * 1000);
#else
    return GetTickCount();
#endif
}

void printFPS(int frames, unsigned int time)
{
#if (defined(LINUX) || defined(__QNXNTO__))
    printf("frames:%d -- fps:%f\n", frames, 1000.0f / time * frames);
#else
    wchar_t fpsStr[128];
    static  FILE* fFPS = NULL;
    char    path[MAX_PATH + 1];
    WCHAR    temp[MAX_PATH + 1];
    char    *pos;

    GetModuleFileNameW(NULL, temp, 256);
    wcstombs(path, temp, MAX_PATH);
    // Find position of postfix.
    pos = strchr(path, '.');
    *(pos) = '\0';
    strcat(path, "FPS.txt");
    fFPS         = fopen(path, "w");
    if (!fFPS)
        return;
#ifdef UNDER_CE
#if UNDER_CE < 800
    swprintf(fpsStr, L"frames:%d --  FPS:%f", frames, 1000.0f / time * frames);
#else
    swprintf_s(fpsStr, L"frames:%d -- FPS:%f", frames, 1000.0f / time * frames);
#endif
    fwrite( fpsStr, 2*wcslen(fpsStr), 1, fFPS );
#else
    swprintf_s(fpsStr, L"frames:%d -- FPS:%f", frames, 1000.0f / time * frames);
    fwrite( fpsStr, 2*wcslen(fpsStr), 1, fFPS );
    fclose(fFPS);
#endif
#endif
}

void WriteBMPFile (char * file_name, VGubyte * pbuf, unsigned int size)
{
    unsigned int result = 0;

    FILE* fd;

    fd = fopen(file_name, "wb");
    assert (fd != NULL);

    result = fwrite(pbuf, 1, size, fd);
    assert (result == size);
    fclose(fd);
}

int SaveBMP(char *image_name, VGubyte* p, VGint width, VGint height)
{
    BITMAPINFOHEADER *infoHeader;
    BITMAPFILEHEADER *fileHeader;
    unsigned char *image_data = NULL;     // Use to change image from RGB to BGR.
    unsigned char *readpixel_data = NULL;
    unsigned char *l_image_data = NULL;
    unsigned char *l_readpixel_data = NULL;
    unsigned char *l_data_load;
    int data_size;
    int index, index2;
    int file_len;
    int DATA_STRIDE;

    // only support read from screen as RGBA8888 format.
    DATA_STRIDE = (VGint)width * 4;
    readpixel_data = (unsigned char *)malloc((int)width*(int)height*4);

    if(readpixel_data == NULL)
        return 1;

    image_data = (unsigned char *)calloc(1, (int)width*(int)height*3 + sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER));
    if(image_data == NULL)
        return 1;

    l_image_data = image_data;
    l_readpixel_data = p;

    fileHeader =(BITMAPFILEHEADER *) l_image_data;
    infoHeader =(BITMAPINFOHEADER *)(l_image_data + sizeof(BITMAPFILEHEADER));
    l_data_load = l_image_data + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    //file header.
    fileHeader->bfType       = 0x4D42;
    fileHeader->bfSize       = sizeof(BITMAPFILEHEADER);
    fileHeader->bfReserved1  = 0;
    fileHeader->bfReserved2  = 0;
    unsigned long t1 = sizeof(BITMAPFILEHEADER);
    unsigned long t2 = sizeof(BITMAPINFOHEADER);
    //fileHeader->bfOffBits    = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    fileHeader->bfOffBits    = t1 + t2;

    //infoHeader.
    *(char *)&infoHeader->biSize = sizeof(BITMAPINFOHEADER);
    *(short *)&infoHeader->biWidth = (short)width;
    *(short *)&infoHeader->biHeight = (short)height;
    infoHeader->biPlanes = 1;
    infoHeader->biBitCount = 24;
    *(short *)&infoHeader->biCompression = BI_RGB;

    //data
    data_size = width * height * 3;
    for(index = 0, index2 = 0; index < data_size; index += 3, index2 += 4){
        l_data_load[index] = l_readpixel_data[index2+1];
        l_data_load[index+1] = l_readpixel_data[index2+2];
        l_data_load[index+2] = l_readpixel_data[index2+3];
    }

    file_len = data_size + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    WriteBMPFile(image_name, l_image_data, file_len);

    free(readpixel_data);
    free(image_data);

    return 0;
}


void SaveResult(char * file_name, int frameNo)
{
    unsigned char *pixels;
    char name[260] = {'\0'};
    char numBuf[10] = {'\0'};

    strcpy(name, file_name);
#if !(defined(LINUX) || defined(__QNXNTO__))
    char        *pos;
    pos = strrchr(name, '.');
    *(pos) = '\0';
#endif

    sprintf(numBuf, "_%03d", frameNo);
    strcat(name, numBuf);
    strcat(name,".bmp");
    pixels = (unsigned char *)malloc(cmdAttrib.winWidth * cmdAttrib.winHeight * 4);
    if (pixels == NULL)
        return;
    vgReadPixels(pixels, cmdAttrib.winWidth*4, VG_sRGBA_8888_PRE, 0, 0, cmdAttrib.winWidth, cmdAttrib.winHeight);
    SaveBMP(name, pixels, cmdAttrib.winWidth, cmdAttrib.winHeight);
    free(pixels);
}



