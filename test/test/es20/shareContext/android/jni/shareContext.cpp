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


/*
 * OpenGL ES 2.0 Share Context
 *
 * Draws a simple triangle with basic vertex and pixel shaders.
 */
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES    1
#endif

#include <nativehelper/jni.h>
#include <utils/Log.h>
#include <sys/time.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <string.h>

#include <nativehelper/jni.h>
#include <utils/Log.h>
#if ANDROID_SDK_VERSION >= 16
#   include <gui/Surface.h>
#else
#   include <surfaceflinger/Surface.h>
#endif

#if ANDROID_SDK_VERSION >= 16
#   undef LOGI
#   undef LOGD
#   undef LOGW
#   undef LOGE
#   define LOGI(...) ALOGI(__VA_ARGS__)
#   define LOGD(...) ALOGD(__VA_ARGS__)
#   define LOGW(...) ALOGW(__VA_ARGS__)
#   define LOGE(...) ALOGE(__VA_ARGS__)
#endif
#ifndef LOG_TAG
#define LOG_TAG "GL2shareContext"
#endif
using namespace android;

extern int          frames;
extern int          width;
extern int          height;
static int          start;
static int          end;
static int          frameCount;
int                 first;
bool                done = false;
bool                paused = false;
struct timeval      tm;
int                 key;
enum
{
    KEYPAD_SPACE = 18,
    KEYPAD_BACK = 4,
};

extern void RenderInit(EGLNativeWindowType nativeWindow, EGLNativeWindowType nativeWindow2);
extern void Render();
void shutDown(void);

extern "C"
{
    JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_shareContext_GL2JNILib_init(JNIEnv * env, jobject obj, jobject surface, jobject surface2);
    JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_shareContext_GL2JNILib_repaint(JNIEnv * env, jobject obj);
    JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_shareContext_GL2JNILib_key(JNIEnv * env, jobject obj, jint k, jboolean d);
    JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_shareContext_GL2JNILib_finish(JNIEnv * env, jobject obj);
};

JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_shareContext_GL2JNILib_init(JNIEnv * env, jobject obj, jobject surface, jobject surface2)
{
    int     i = 0;
    EGLNativeWindowType    nativeWindow;
    EGLNativeWindowType    nativeWindow2;
    int     ret;
    char    Log[256];

    jclass surface_class = env->FindClass("android/view/Surface");
    jclass version_class = env->FindClass("android/os/Build$VERSION");
    jfieldID sdkIntFieldID = env->GetStaticFieldID(version_class, "SDK_INT", "I");
    jfieldID surface_SurfaceFieldID = NULL;
    int sdkInt = env->GetStaticIntField(version_class, sdkIntFieldID);
    sp <Surface> sur;
    sp <Surface> sur2;
    if (sdkInt <= 8)
    {
           surface_SurfaceFieldID = env->GetFieldID(surface_class,"mSurface", "I");
    }
    else
    {
        if (sdkInt > 18)
        {
            /*surface_SurfaceFieldID = env->GetFieldID(surface_class,"mNativeObject", "I");*/
            if (sdkInt >= 21) {
                surface_SurfaceFieldID = env->GetFieldID(surface_class,"mNativeObject", "J");
            }
            else {
                surface_SurfaceFieldID = env->GetFieldID(surface_class,"mNativeObject", "I");
            }
        }
        else
        {
            surface_SurfaceFieldID = env->GetFieldID(surface_class,"mNativeSurface", "I");
        }
    }
    if (surface_SurfaceFieldID == NULL) {
        LOGI("failed to get SurfaceFieldID");
        return false;
    }
    if (sdkInt >= 21) {
        sur = (Surface *)(khronos_uintptr_t)env->GetLongField(surface, surface_SurfaceFieldID);
        sur2 = (Surface *)(khronos_uintptr_t)env->GetLongField(surface2, surface_SurfaceFieldID);
    }
    else {
        sur = (Surface *)(khronos_uintptr_t)env->GetIntField(surface, surface_SurfaceFieldID);
        sur2 = (Surface *)(khronos_uintptr_t)env->GetIntField(surface2, surface_SurfaceFieldID);
    }

    nativeWindow = (EGLNativeWindowType)(static_cast<ANativeWindow *>(sur.get()));
    nativeWindow2 = (EGLNativeWindowType)(static_cast<ANativeWindow *>(sur2.get()));

    RenderInit(nativeWindow, nativeWindow2);
    return true;
}

JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_shareContext_GL2JNILib_repaint(
    JNIEnv * env, jobject obj)
{
    Render();
    return true;
}

JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_shareContext_GL2JNILib_key(JNIEnv * env, jobject obj, jint k, jboolean d)
{
    if (d == 1)
    {
        key = k;
    }

    return true;
}

JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_shareContext_GL2JNILib_finish(
    JNIEnv * env, jobject obj)
{
    shutDown();
    return true;
}

