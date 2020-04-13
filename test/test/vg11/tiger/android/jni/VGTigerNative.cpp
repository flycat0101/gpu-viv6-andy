/****************************************************************************
*
*    Copyright 2012 - 2020 Vivante Corporation, Santa Clara, California.
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


#include <nativehelper/jni.h>
#include <EGL/egl.h>
#include <utils/Log.h>
#if ANDROID_SDK_VERSION >= 16
#   include <gui/Surface.h>
#else
#   include <surfaceflinger/Surface.h>
#endif
#include "Common.h"
#include <ui/FramebufferNativeWindow.h>
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

using namespace android;


#define MAX_CASE_NUM  30


void EGL_Destroy(void)
{
    EGLBoolean ret;

    ret = eglMakeCurrent(gDisplay, NULL, NULL, NULL);
    if (ret != EGL_TRUE ){
        LOGI("********** Error in eglMakeCurrent!!! **********\n");
        return;
    }

    ret = eglDestroyContext(gDisplay, gContext);
    if (ret != EGL_TRUE){
        LOGI("********** Error in eglDestroyContext!!! **********\n");
        return;
    }

    ret = eglDestroySurface(gDisplay, gSurface);
    if (ret != EGL_TRUE){
        LOGI("********** Error in eglDestroySurface!!! **********\n");
        return;
    }
}



bool EGL_Init(EGLNativeWindowType nativeWindow)
{
    static EGLint attrib_list[] ={
            EGL_RED_SIZE,        8,
            EGL_GREEN_SIZE,     8,
            EGL_BLUE_SIZE,        8,
            EGL_ALPHA_SIZE,     8,

            EGL_DEPTH_SIZE,     0,
            EGL_STENCIL_SIZE, 0,
            EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
            EGL_SAMPLES,        EGL_DONT_CARE,
            EGL_SAMPLE_BUFFERS,   EGL_DONT_CARE,
            EGL_NONE, EGL_NONE,
            EGL_NONE
        };

        EGLBoolean ret;
        EGLint count;
        EGLConfig gConfig_array[100];
           EGLint i;
        ANativeWindow *nwin;
        int width, height;
        attrib_list[1] = gRunRed;    //EGL_RED_SIZE
        attrib_list[3] = gRunGreen;     //EGL_GREEN_SIZE
        attrib_list[5] = gRunBlue;    //EGL_BLUE_SIZE
        attrib_list[7] = gRunAlpha;     //EGL_ALPHA_SIZE
        attrib_list[9] = gRunDepth;     //EGL_DEPTH_SIZE
        attrib_list[11] = gRunStencil;    //EGL_STENCIL_SIZE
        if(gEglmode == 1){
            attrib_list[13] = EGL_PBUFFER_BIT; //EGL_SURFACE_TYPE
        }

        attrib_list[15] = gRunSamples;    //EGL_SAMPLES
        attrib_list[17] = gRunSampleBuf;//EGL_SAMPLES

        if(gVGmode == 1)
        {
            attrib_list[18] = EGL_RENDERABLE_TYPE;
            attrib_list[19] = EGL_OPENVG_BIT;
        }

        gDisplay = eglGetDisplay( EGL_DEFAULT_DISPLAY );//(EGLDisplay)hCanvas);

        if (gDisplay == EGL_NO_DISPLAY){
            LOGI("**********   Error in eglGetDisplay!!! **********\n");
            return 1;
        }


            LOGI("**********Error Code :%d: \n",eglGetError());
        ret = eglInitialize(gDisplay, NULL, NULL);
        if(ret != EGL_TRUE)
        {
            LOGI("**********  :Error in  eglInitialize errCode %d!!! **********\n", eglGetError());

        }

        LOGI("**********Error Code :%d: \n",eglGetError());
     if(gVGmode)
    ret = eglBindAPI(EGL_OPENVG_API);



        ret = eglChooseConfig(gDisplay, (const EGLint *) attrib_list, &(gConfig_array[0]), 100, &count);
        if (ret != EGL_TRUE){
            LOGI("**********  :Error in eglChooseConfig!!! **********\n");
            return 1;
        }
        if (count <= 0){
            LOGI("**********  :eglChooseConfig no config!!! **********\n");
            return 1;
        }

        for( i = 0; i < count; i++)
        {
            int samples_a = 0;
            int samples_r = 0;
            int samples_g = 0;
            int samples_b = 0;
            int samples_d = 0;
            int samples_s = 0;
            int samples_sam = 0;
            int samples_sambuf = 0;

            eglGetConfigAttrib(gDisplay, gConfig_array[i], EGL_ALPHA_SIZE, &samples_a);
            eglGetConfigAttrib(gDisplay, gConfig_array[i], EGL_RED_SIZE, &samples_r);
            eglGetConfigAttrib(gDisplay, gConfig_array[i], EGL_GREEN_SIZE, &samples_g);
            eglGetConfigAttrib(gDisplay, gConfig_array[i], EGL_BLUE_SIZE, &samples_b);
            eglGetConfigAttrib(gDisplay, gConfig_array[i], EGL_DEPTH_SIZE, &samples_d);
            eglGetConfigAttrib(gDisplay, gConfig_array[i], EGL_STENCIL_SIZE, &samples_s);
            eglGetConfigAttrib(gDisplay, gConfig_array[i], EGL_SAMPLES, &samples_sam);
            eglGetConfigAttrib(gDisplay, gConfig_array[i], EGL_SAMPLE_BUFFERS, &samples_sambuf);

            if( samples_a == gRunAlpha &&
                samples_r == gRunRed &&
                samples_g == gRunGreen &&
                samples_b == gRunBlue &&
                samples_d >= gRunDepth &&
                samples_s >= gRunStencil &&
                samples_sam >= gRunSamples &&
                samples_sambuf == gRunSampleBuf)
            {
                gConfig = gConfig_array[i];
                gRunDepth = samples_d;
                gRunStencil = samples_s;
                gRunSamples = samples_sam;
                break;
            }

        }
        if(gConfig == (void *)(-1)){
            LOGI("**********  :gConfig no config is suitable!!! **********\n");
            return 1;
        }



        //Window mode
        EGLint attr[] = {
        EGL_WIDTH, 640,
        EGL_HEIGHT, 480,
        EGL_NONE
        };
        attr[1] = gWinSizeX;
        attr[3] = gWinSizeY;
        if(gEglmode == 0)
        gSurface = eglCreateWindowSurface(gDisplay, gConfig, (Surface*)nativeWindow, NULL);
        else
        gSurface = eglCreatePbufferSurface(gDisplay, gConfig, attr);
        if (gSurface == EGL_NO_SURFACE){
            LOGI("***********Tiger:Error in eglSurface Window 0x %x!!! ***********\n",eglGetError());
            LOGI("**********Tiger:Error in eglSurface Window!!! **********\n");
            return 1;
        }

        gContext = eglCreateContext(gDisplay, gConfig, NULL, NULL);
        if (gContext == EGL_NO_CONTEXT){
            LOGI("**********Tiger:Error in eglCreateContext!!! **********\n");
            return 1;
        }
         LOGI("begin MakeCurrent\n");
      if (eglMakeCurrent(gDisplay, gSurface, gSurface, gContext)== EGL_FALSE)
        {

            LOGI("**********Tiger:Error in eglMakeCurrent errorCode 0x%x!!! **********\n",eglGetError());
            return 1;
        }

         LOGI("end MakeCurrent\n");
    return 0;
}

extern "C"{
    JNIEXPORT void JNICALL Java_com_vivantecorp_graphics_tiger2DVG_GL2JNILib_nativeRunTest(JNIEnv * env, jobject obj, jobject surface, jbooleanArray array);
    JNIEXPORT void JNICALL Java_com_vivantecorp_graphics_tiger2DVG_GL2JNILib_init(JNIEnv * env, jobject obj);
    JNIEXPORT jint JNICALL Java_com_vivantecorp_graphics_tiger2DVG_GL2JNILib_getWidth(JNIEnv * env, jobject obj);
    JNIEXPORT jint JNICALL Java_com_vivantecorp_graphics_tiger2DVG_GL2JNILib_getHeight(JNIEnv * env, jobject obj);
}
JNIEXPORT void JNICALL Java_com_vivantecorp_graphics_tiger2DVG_GL2JNILib_nativeRunTest(JNIEnv *  env, jobject obj, jobject surface, jbooleanArray array)
{
    int i = 0;
    EGLNativeWindowType nativeWindow;
    int ret;
    jboolean buf[MAX_CASE_NUM];
    ANativeWindow *nwin;
    int width,height;
    char Log[256];
    jclass surface_class = env->FindClass("android/view/Surface");


#if ANDROID_SDK_VERSION <= 8
    jfieldID surface_SurfaceFieldID = env->GetFieldID(surface_class,"mSurface", "I");
#else
    #if ANDROID_SDK_VERSION > 20
        jfieldID surface_SurfaceFieldID = env->GetFieldID(surface_class,"mNativeObject", "J");
    #elif ANDROID_SDK_VERSION > 18
        jfieldID surface_SurfaceFieldID = env->GetFieldID(surface_class,"mNativeObject", "I");
    #else
        jfieldID surface_SurfaceFieldID = env->GetFieldID(surface_class,"mNativeSurface", "I");
    #endif
#endif

#if ANDROID_SDK_VERSION > 20
    sp <Surface> sur = (Surface *)env->GetLongField(surface, surface_SurfaceFieldID);
#else
    sp <Surface> sur = (Surface *)env->GetIntField(surface, surface_SurfaceFieldID);
#endif
    nativeWindow = (EGLNativeWindowType)(static_cast<ANativeWindow *>(sur.get()));

    LOGI("Application fine after get int filed: %d\n", nativeWindow);
    env->GetBooleanArrayRegion(array, 0, MAX_CASE_NUM, buf);

    LOGI("start egl init.\n");
    ret = EGL_Init(nativeWindow);
    if(1 == ret)
    {
        LOGI("EGL Init failed.\n");
        return;
    }

    gNativeWindow =(EGLNativeWindowType) (Surface*) nativeWindow;
    nwin = (ANativeWindow *) (Surface *) nativeWindow;
    nwin->query(nwin, NATIVE_WINDOW_WIDTH, &width);
    nwin->query(nwin, NATIVE_WINDOW_HEIGHT, &height);
    gWinSizeX = width;
    gWinSizeY = height;

    LOGI("The EGLDisplay is %d\n", gDisplay);
    LOGI("The EGLSurface is %d\n", gSurface);
    LOGI("The EGLContext is %d\n", gContext);
    LOGI("The EGLConfig  is %d\n", gConfig);
    LOGI("End egl init.\n");

    //EGL Config Log Print^M

    sprintf(Log, "Config: %dx%d %d%d%d%d depth %d stencil %d samples %d(%d)\n\n",
        gWinSizeX, gWinSizeY, gRunRed, gRunGreen, gRunBlue, gRunAlpha, gRunDepth, gRunStencil, gRunSamples, gRunSampleBuf);

    LOGI("Config: %dx%d %d%d%d%d depth %d stencil %d samples %d(%d)\n\n",gWinSizeX, gWinSizeY, gRunRed, gRunGreen, gRunBlue, gRunAlpha, gRunDepth, gRunStencil, gRunSamples, gRunSampleBuf);

    for(i = 0; cases[i].casefunc != NULL; i++)
    {
        if(1 == buf[i])
        {
            gRunCaseName = cases[i].casename;
            LOGI("Now, run the %s test case!\n", gRunCaseName);
            runCases();
        }
    }
    EGL_Destroy();
}

JNIEXPORT void JNICALL Java_com_vivantecorp_graphics_tiger2DVG_GL2JNILib_init(JNIEnv * env, jobject obj)
{
}
JNIEXPORT jint JNICALL Java_com_vivantecorp_graphics_tiger2DVG_GL2JNILib_getWidth(JNIEnv * env, jobject obj)
{
    return gWinSizeX;
}

JNIEXPORT jint JNICALL Java_com_vivantecorp_graphics_tiger2DVG_GL2JNILib_getHeight(JNIEnv * env, jobject obj)
{
    return gWinSizeY;
}


