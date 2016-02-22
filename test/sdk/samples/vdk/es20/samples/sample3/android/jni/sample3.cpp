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


#include <nativehelper/jni.h>
#define LOG_TAG "GL2_Sample3"
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
#include "vdk_sample_common.h"
//#include "2d_texture.h"

int key;
/*
 * VDKS
*/

//vdkEGL VdkEgl;

int VDKS_Val_WindowsWidth	= 0;
int VDKS_Val_WindowsHeight	= 0;

char* VDKS_ARG0 = NULL;
extern "C" {

	extern VDKS_BOOL    SnakeInit();
	extern void         SnakePass();

	extern VDKS_BOOL Init();
	extern void Run ();
}
/***************************************************************************************
***************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
    JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_sample3_GL2JNILib_init(JNIEnv * env, jobject obj,  jint width, jint height);
    JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_sample3_GL2JNILib_repaint(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL Java_com_vivantecorp_graphics_sample3_GL2JNILib_key(JNIEnv * env, jobject obj, jint k, jboolean down);
#ifdef __cplusplus
};
#endif

JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_sample3_GL2JNILib_init(JNIEnv * env, jobject obj,  jint width, jint height)
{
	int tex_w = 0;
	VDKS_Val_WindowsWidth = width;
	VDKS_Val_WindowsHeight = height;

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &tex_w );

    glViewport(0, 0, VDKS_Val_WindowsWidth, VDKS_Val_WindowsHeight);

   	if (VDKS_TRUE != Init())
	{
		LOGE("Failed to init case.");
		return false;
	}
   return true;
}

JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_sample3_GL2JNILib_repaint(JNIEnv * env, jobject obj)
{
	 Run();
	 return true;
}

JNIEXPORT void JNICALL Java_com_vivantecorp_graphics_sample3_GL2JNILib_key(JNIEnv * env, jobject obj, jint k, jboolean down)
{
    if (down == 1)
    {
        key = k;
    }
}
