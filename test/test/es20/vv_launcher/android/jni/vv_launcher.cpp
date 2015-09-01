/****************************************************************************
*
*    Copyright 2012 - 2015 Vivante Corporation, Santa Clara, California.
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


// OpenGL ES 2.0 code

#include <nativehelper/jni.h>
#include <utils/Log.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include <glutils/log.h>

#include "LauncherApp.h"

int  appWidth	= 640;
int  appHeight	= 480;
int  flipY		= 0;
LauncherApp * app;
struct timeval tv;

volatile int key;
unsigned int lastTime,now;
bool running = false;

#define RETURN_SUCCESS   0
#define RETURN_ERROR 1

volatile int finished;


extern "C" {

}
/***************************************************************************************
***************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
    JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_vvlauncher_GL2JNILib_init(JNIEnv * env, jobject obj,  jint width, jint height);
    JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_vvlauncher_GL2JNILib_repaint(JNIEnv * env, jobject obj);
	JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_vvlauncher_GL2JNILib_key(JNIEnv * env, jobject obj, jint k, jboolean down);
#ifdef __cplusplus
};
#endif

JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_vvlauncher_GL2JNILib_init(JNIEnv * env, jobject obj,  jint width, jint height)
{
	appWidth  = width;
	appHeight = height;

    glViewport(0, 0, appWidth, appHeight);

	app = LauncherAppConstruct(appWidth, appHeight);
	flipVertical(app, flipY);

	gettimeofday(&tv, 0);
	lastTime = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);

	running = true;

   return true;
}

JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_vvlauncher_GL2JNILib_repaint(JNIEnv * env, jobject obj)
{
	char const * command;
	float delta;
	if(!running)
		return false;

	gettimeofday(&tv, 0);
	now = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);

	delta = (float)(now - lastTime) / 1000.0f;

	if (delta > 1.0f / 20.0f) delta = 1.0f / 20.0f;
	lastTime = now;

	command = LauncherAppTick(app, delta);
	if (command != NULL)
	{
		LauncherAppSelect(app, -1);

	}

	LauncherAppDraw(app, appWidth, appHeight);
	if(key)
	{
		switch(key)
		{
			case 11:
				system("am start -n com.vivantecorp.graphics.cover_flow/.EntryActivity");
			case 14:
				system("am start -n com.glbenchmark.GLBenchmark11/.GLBenchmark11");
				break;
			case 15:
				system("am start -n com.glbenchmark.GLBenchmark20/com.glbenchmark.Common.TestSelectActivity");
				break;
			case 16:
				system("am start -n com.vivantecorp.graphics.sample3/.GL2Sample3Activity");
				break;
			case 4:
				running = false;
				break;
			default:
				break;
		}

		key = 0;
	}



	return true;
}

JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_vvlauncher_GL2JNILib_key(JNIEnv * env, jobject obj, jint k, jboolean down)
{
    if (down == 1)
    {
        key = k;
    }
	return true;
}
