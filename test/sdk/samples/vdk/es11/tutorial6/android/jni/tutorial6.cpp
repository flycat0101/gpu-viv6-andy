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


/*
 *  * OpenGL ES 1.1 Tutorial 6
 *   *
 *    * Draws a simpe triangle with basic vertex and pixel shaders.*/

#include <nativehelper/jni.h>
#define LOG_TAG "GL11_Tutorial6"
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

#include <GLES/gl.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "render.h"

#define TUTORIAL_NAME "OpenGL ES 1.1 Tutorial 6"


int width;
int height;
int posX   = -1;
int posY   = -1;
int samples = 0;
int frames;

static int start;
static int end;
static int frameCount;

struct timeval tm;
int key;

enum
{
    KEYCODE_BACK = 4,
    KEYCODE_DPAD_LEFT = 21,
    KEYCODE_DPAD_RIGHT = 22,
    KEYCODE_A = 0x1d,
    KEYCODE_G = 0x23,
    KEYCODE_R = 0x2e,
    KEYCODE_COMMA = 0x37,
    KEYCODE_PERIOD = 0x38,
    KEYCODE_MINUS = 0x45,
    KEYCODE_PLUS = 0x46,
    KEYCODE_SPACE = 18,

};

extern "C" {
    JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_gl11tutorial6_GL11JNILib_init(JNIEnv * env, jobject obj,  jint w, jint h);
    JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_gl11tutorial6_GL11JNILib_repaint(JNIEnv * env, jobject obj);
    JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_gl11tutorial6_GL11JNILib_changeBackground(JNIEnv * env, jobject obj);
	JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_gl11tutorial6_GL11JNILib_key(JNIEnv * env, jobject obj, jint k, jboolean d);
};

JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_gl11tutorial6_GL11JNILib_init(JNIEnv * env, jobject obj,  jint w, jint h)
{
	width  = w;
	height = h;

	if(!InitGLES())
	{
		LOGI("Init GLES failed!");
		return false;
	}

	gettimeofday(&tm, NULL);
    start = tm.tv_sec * 1000 + tm.tv_usec / 1000;

	return true;
}
static bool done = false;
static bool paused = false;
JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_gl11tutorial6_GL11JNILib_repaint(JNIEnv * env, jobject obj)
{

    if(!done)
    {
         if(key!=0)
         {
             switch(key)
             {
                 case KEYCODE_SPACE :
                     paused =!paused;
                     LOGI("render paused, press # key to continue...");
                     break;
                  case KEYCODE_BACK :
                     done = true;
                     LOGI("BACK pressed, case stopping!");
                     break;
				  case KEYCODE_DPAD_LEFT :

					 //paused =false;
					break;
                  case KEYCODE_DPAD_RIGHT :
                     break;
                  default:
                     break;
              }
			key = 0;
          }
          if (!paused)
          {
              Render();
              ++frameCount;
          }
    }

    gettimeofday(&tm, NULL);
    end = tm.tv_sec * 1000 + tm.tv_usec / 1000;
    float fps = frameCount / ((end - start) / 1000.0f);

	if((end-start)>1000 && !paused)
    {
            LOGI("%d frames in %d ticks -> %.3f fps\n", frameCount, end - start, fps);
            start = end;
            frameCount=0;
    }

    return !done;
}

JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_gl11tutorial6_GL11JNILib_changeBackground(JNIEnv * env, jobject obj)
{
	return true;
}

JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_gl11tutorial6_GL11JNILib_key(JNIEnv * env, jobject obj, jint k, jboolean d)
{
	LOGI("Key Event : %d, d:%d",k,d);
    if (d)
    {
        key = k;
    }
	return true;
}
