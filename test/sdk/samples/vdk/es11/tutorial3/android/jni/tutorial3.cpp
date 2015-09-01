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


/*
 *  * OpenGL ES 1.1 Tutorial 1
 *   *
 *    * Draws a simpe triangle with basic vertex and pixel shaders.*/

#include <nativehelper/jni.h>
#define LOG_TAG "GL11_Tutorial3"
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
#include "fixedpoint.h"

#define TUTORIAL_NAME "OpenGL ES 1.1 Tutorial 3"


extern int width;
extern int height;
 int posX   = -1;
 int posY   = -1;
 int samples = 0;
extern int frames;

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

//extern "C"{
extern void *LoadTGA(FILE * File,GLenum * Format,GLsizei * Width,GLsizei * Height);
//}
extern GLuint texture1;
extern GLuint texture2;
extern bool multiDrawElementsAvailable;
extern PFNGLMULTIDRAWELEMENTSEXTPROC _glMultiDrawElementsEXT;

bool InitGLES()
{
	FILE* file;
	void * bits;
	GLenum  format, target = 0;
	GLsizei width;
	GLsizei height;
	GLsizei bytes;
	int temp;

    glClearColorx(FixedFromFloat(0.5f),
                  FixedFromFloat(0.5f),
                  FixedFromFloat(0.5f),
                  ONE);

    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    SetPerspective();

	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);

	file = fopen("/sdcard/es11/tutorial3/door.tga", "rb");
	bits = LoadTGA(file, &format, &width, &height);
    glTexParameteri( GL_TEXTURE_2D,GL_GENERATE_MIPMAP,GL_TRUE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	if (bits != NULL)
		glTexImage2D(GL_TEXTURE_2D,0,format,width,height,0,format,GL_UNSIGNED_BYTE,bits);
	else
		return false;

	fclose(file);

    glGenTextures(1, &texture2);
    glBindTexture(target = GL_TEXTURE_2D, texture2);
	file = fopen("/sdcard/es11/tutorial3/fire.tga", "rb");
	bits = LoadTGA(file, &format, &width, &height);

    glTexParameteri( GL_TEXTURE_2D,GL_GENERATE_MIPMAP,GL_TRUE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	if (bits != NULL)
		glTexImage2D(GL_TEXTURE_2D,0,format,width,height,0,format,GL_UNSIGNED_BYTE,bits);
	else
		return false;

    {
        char* extensions = (char *)glGetString(GL_EXTENSIONS);

        if (strstr(extensions, "GL_EXT_multi_draw_arrays") != 0)
        {
            _glMultiDrawElementsEXT = (PFNGLMULTIDRAWELEMENTSEXTPROC)eglGetProcAddress("glMultiDrawElementsEXT");
            if (_glMultiDrawElementsEXT != NULL)
            {
                multiDrawElementsAvailable = true;
            }
        }
    }

	fclose(file);
    return true;
}

extern "C" {
    JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_gl11tutorial3_GL11JNILib_init(JNIEnv * env, jobject obj,  jint w, jint h);
    JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_gl11tutorial3_GL11JNILib_repaint(JNIEnv * env, jobject obj);
    JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_gl11tutorial3_GL11JNILib_changeBackground(JNIEnv * env, jobject obj);
	JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_gl11tutorial3_GL11JNILib_key(JNIEnv * env, jobject obj, jint k, jboolean d);
};

JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_gl11tutorial3_GL11JNILib_init(JNIEnv * env, jobject obj,  jint w, jint h)
{
	width  = w;
	height = h;
	LOGI("w=%d, h=%d",w,h);
//	glViewport(0, 0, width, height);

	if(!InitGLES())
	{
		LOGI("Init GLES failed!");
		return false;
	}

	gettimeofday(&tm, NULL);
    start = tm.tv_sec * 1000 + tm.tv_usec / 1000;

	LOGI("init end");
	return true;
}
static bool done = false;
static bool paused = false;
JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_gl11tutorial3_GL11JNILib_repaint(JNIEnv * env, jobject obj)
{
//	LOGI("start repaint");
//	InitGLES();
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

    //glFinish();

    return !done;
}

JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_gl11tutorial3_GL11JNILib_changeBackground(JNIEnv * env, jobject obj)
{
	return true;
}

JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_gl11tutorial3_GL11JNILib_key(JNIEnv * env, jobject obj, jint k, jboolean d)
{
	LOGI("Key Event : %d, d:%d",k,d);
    if (d)
    {
        key = k;
    }
	return true;
}
