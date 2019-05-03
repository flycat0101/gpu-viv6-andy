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


/*
 * OpenGL ES 2.0 Tutorial 7
 *
 * Draws a simple triangle with basic vertex and pixel shaders.
 */
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES    1
#endif

#include <nativehelper/jni.h>
#include <android/log.h>
#include <time.h>

#undef LOGI
#undef LOGD
#undef LOGW
#undef LOGE
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifndef UNDER_CE
#include <fcntl.h>
#endif
#include <string.h>

#ifndef LOG_TAG
#define LOG_TAG "GL2Tutorial7"
#endif

extern int frames, width, height;
extern unsigned int programHandle[2];
static int start;
static int end;
static int frameCount;
int first;
bool done = false, paused = false;
struct timeval tm;
int key;
enum
{
    KEYPAD_SPACE = 18,
    KEYPAD_BACK = 4,
};
extern void RenderInit();
extern void Render();
extern void RenderCleanup();
extern void LoadShaders(const char * vShaderFName, const char * pShaderFName, unsigned int index);

extern "C" {
    JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_tutorial7_GL2JNILib_init(JNIEnv * env, jobject obj,  jint w, jint h);
    JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_tutorial7_GL2JNILib_repaint(JNIEnv * env, jobject obj);
    JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_tutorial7_GL2JNILib_key(JNIEnv * env, jobject obj, jint k, jboolean d);
};

JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_tutorial7_GL2JNILib_init(JNIEnv * env, jobject obj,  jint w, jint h)
{
    width = w;
    height = h;

    LoadShaders("/sdcard/tutorial/tutorial7/vs_es20t7a.vert", "/sdcard/tutorial/tutorial7/ps_es20t7a.frag", 0);
    LoadShaders("/sdcard/tutorial/tutorial7/vs_es20t7b.vert", "/sdcard/tutorial/tutorial7/ps_es20t7b.frag", 1);

    if (programHandle[0] == 0 || programHandle[1] == 0)
        return false;

    RenderInit();

    gettimeofday(&tm, NULL);
    first = start = tm.tv_sec * 1000 + tm.tv_usec / 1000;

    return true;

}

JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_tutorial7_GL2JNILib_repaint(JNIEnv * env, jobject obj)
{
    if(!done)
    {
        if(key != 0)
        {
            switch(key)
            {
                case 18 :
                    paused = !paused;
                    LOGI("Render paused, press # to continue...");
                    break;
                case KEYPAD_BACK :
                    done = true;
                    break;
                default:
                    break;

            }
            key = 0;
        }
        if(!paused)
        {
            Render();
            ++frameCount;
               ++frames;
        }
    }

    gettimeofday(&tm, NULL);
    end = tm.tv_sec * 1000 + tm.tv_usec / 1000;

    if ((end - start) > 1000 && !paused)
    {
        unsigned long duration = end - start;
        float fps = 1000.0f * float(frameCount) / float(duration);
        float afps = 1000.0f * float(frames) / float(end - first);

        LOGI("Rendered %d frames in %lu milliseconds: %.2f fps,average: %.2f fps\n",frameCount, duration, fps, afps);

        start = end;
        frameCount = 0;
    }

    if (done) {
        glFinish();
        RenderCleanup();
    }

    return true;
}

JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_tutorial7_GL2JNILib_key(JNIEnv * env, jobject obj, jint k, jboolean d)
{
    if(d == 1)
    {
        key = k;
    }
    return true;

}
