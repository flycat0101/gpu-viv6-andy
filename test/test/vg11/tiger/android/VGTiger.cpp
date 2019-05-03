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


#include "tiger.h"

#include <VG/openvg.h>
#include "Common.h"
#include <utils/Log.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <EGL/egl.h>
#include <nativehelper/jni.h>
#include <utils/Log.h>
#if ANDROID_SDK_VERSION >= 16
#   include <ui/ANativeObjectBase.h>

#   undef LOGI
#   undef LOGD
#   undef LOGW
#   undef LOGE
#   define LOGI(...) ALOGI(__VA_ARGS__)
#   define LOGD(...) ALOGD(__VA_ARGS__)
#   define LOGW(...) ALOGW(__VA_ARGS__)
#   define LOGE(...) ALOGE(__VA_ARGS__)
#else
#   include <ui/android_native_buffer.h>
#endif


static int initVG();
static void deInitVG();
static void renderAFrame(void *);


static int winWidth = 640;
static int winHeight = 480;
static int angle;
void tigerRun()
{
    int frameCount;
    int start;
    int end;
    float fps;
    float pixelRate;
    struct timeval tm;
        int frames;
    static int totalCount = 100;

    winWidth = gWinSizeX;
    winHeight = gWinSizeY;
    if(initVG() == 0)
    {
        LOGI("can't init VG\n");
        return;
    }
    else
    {
        LOGI("init VG ok\n");
    };

    gettimeofday(&tm, NULL);
    start = tm.tv_sec * 1000 + tm.tv_usec /1000; frames = 0;

    for(frameCount = 0 ;frameCount < totalCount; ++frameCount)
    {
        vfFRAME_Render();
        eglSwapBuffers(gDisplay, gSurface);
                frames++;
        gettimeofday(&tm, NULL);
        end = tm.tv_sec * 1000 + tm.tv_usec/1000;
        if(end - start > 1000)
        {

             LOGI("%d frames in %d ticks -> %.3f fps\n", frames, end - start, frames/((end -start)*0.001f));
             start = end;
             frames =0;
        }
    }
    deInitVG();

}

int initVG()
{
    vfFRAME_AppInit(&gWinSizeX, &gWinSizeY, NULL);
     LOGI("####### the width %d height %d\n", gWinSizeX, gWinSizeY);
    vfFRAME_InitVG();
    return 1;
}

void deInitVG()
{
    vfFRAME_Clean();
}





