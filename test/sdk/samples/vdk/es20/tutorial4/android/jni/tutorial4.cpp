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

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "common.h"
#include "commonMath.h"

#ifndef LOG_TAG
#define LOG_TAG "GL2Tutorial4"
#endif
extern int width;
extern int height;
int posX    = -1;
int posY    = -1;
int samples =  0;
int frames  =  0;

int frameCount = 0;

static int key = 0;

struct timeval tm;
static unsigned int first, start, end;

static bool paused = false;
static bool done = false;


enum
{
	KEYPAD_SPACE = 18,
	KEYPAD_BACK = 4,
};

GLuint vertShaderNum, pixelShaderNum, pixelShaderNum2;

// Global Variables
extern GLuint programHandleGlass;
extern GLuint programHandleBgrnd;

// Generic used buffer
extern GLuint bufs[5];

// hold sphere data
extern int numVertices, numFaces, numIndices;
extern GLfloat* sphereVertices;
extern GLushort* indices;

// attribute and uniform handle.
extern GLint  locVertices[2], locColors[2],
	   locTransformMat[2], locRadius[2], locRotationMat[2],
	   locEyePos[2], locTexCoords[2], locSamplerCb[2];

// render state
extern State renderState;

extern D3DXMATRIX transformMatrix;

extern void RenderInit();
extern void Render();
extern void RenderCleanup();


extern "C" {
    JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_tutorial4_GL2JNILib_init(JNIEnv * env, jobject obj,  jint w, jint h);
    JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_tutorial4_GL2JNILib_repaint(JNIEnv * env, jobject obj);
    JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_tutorial4_GL2JNILib_key(JNIEnv * env, jobject obj, jint k, jboolean d);
};

JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_tutorial4_GL2JNILib_init(JNIEnv * env, jobject obj,  jint w, jint h)
{


	width   =  w;
	height  =  h;

	vertShaderNum  = glCreateShader(GL_VERTEX_SHADER);
	pixelShaderNum = glCreateShader(GL_FRAGMENT_SHADER);
	pixelShaderNum2 = glCreateShader(GL_FRAGMENT_SHADER);

	if (CompileShader("/sdcard/tutorial/tutorial4/vs_es20t4.vert", vertShaderNum) == 0)
	{
		return false;
	}

	if (CompileShader("/sdcard/tutorial/tutorial4/ps_es20t4_glass.frag", pixelShaderNum) == 0)
	{
		return false;
	}

	if (CompileShader("/sdcard/tutorial/tutorial4/ps_es20t4_bgrnd.frag", pixelShaderNum2) == 0)
	{
		return false;
	}

	programHandleGlass = glCreateProgram();
	glAttachShader(programHandleGlass, vertShaderNum);
	glAttachShader(programHandleGlass, pixelShaderNum);
	glLinkProgram(programHandleGlass);

	programHandleBgrnd = glCreateProgram();
	glAttachShader(programHandleBgrnd, vertShaderNum);
	glAttachShader(programHandleBgrnd, pixelShaderNum2);
	glLinkProgram(programHandleBgrnd);

	if (programHandleGlass == 0 || programHandleBgrnd == 0)
	{
		return false;
	}

	RenderInit();

	gettimeofday(&tm, NULL);
	first = start = tm.tv_sec * 1000 + tm.tv_usec / 1000;

	return true;

}

JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_tutorial4_GL2JNILib_repaint(JNIEnv * env, jobject obj)
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

	return true;
}

JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_tutorial4_GL2JNILib_key(JNIEnv * env, jobject obj, jint k, jboolean d)
{
	if(d == 1)
	{
		key = k;
	}
	return true;

}
