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


#include "cover_flow.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#include <nativehelper/JNIHelp.h>
#include <nativehelper/jni.h>

#include <android/log.h>

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

static int STEPS = 16;

/*******************************************************************************
** Program entry point.
*/

static CoverFlow app;
static int width;
static int height;
static int samples;
static int etc1;

static int run;
static int frames;

static unsigned int start;

enum
{
	KEYCODE_BACK = 4,
	KEYCODE_DPAD_LEFT = 0x15,
	KEYCODE_DPAD_RIGHT = 0x16,
	KEYCODE_A = 0x1d,
	KEYCODE_G = 0x23,
	KEYCODE_R = 0x2e,
	KEYCODE_COMMA = 0x37,
	KEYCODE_PERIOD = 0x38,
	KEYCODE_MINUS = 0x45,
	KEYCODE_PLUS = 0x46,
	KEYCODE_SPACE = 0x3e,


};

extern void * LoadPKM(FILE *, GLenum *, GLsizei *, GLsizei *, GLsizei *);
extern void * LoadTGA(FILE *, GLenum *, GLsizei *, GLsizei *);

JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_Native_init
  (JNIEnv * Env, jobject jo, jint id, jint w, jint h)
{
	const char * tgaTextures[NUM_IMAGES] =
	{
		"/sdcard/cover_flow/image1.tga",
		"/sdcard/cover_flow/image2.tga",
		"/sdcard/cover_flow/image3.tga",
		"/sdcard/cover_flow/image4.tga",
		"/sdcard/cover_flow/image5.tga",
		"/sdcard/cover_flow/image6.tga",
		"/sdcard/cover_flow/image7.tga",
		"/sdcard/cover_flow/image8.tga",
		"/sdcard/cover_flow/image9.tga"
	};

	const char * etc1Textures[NUM_IMAGES] =
	{
		"/sdcard/cover_flow/image1.pkm",
		"/sdcard/cover_flow/image2.pkm",
		"/sdcard/cover_flow/image3.pkm",
		"/sdcard/cover_flow/image4.pkm",
		"/sdcard/cover_flow/image5.pkm",
		"/sdcard/cover_flow/image6.pkm",
		"/sdcard/cover_flow/image7.pkm",
		"/sdcard/cover_flow/image8.pkm",
		"/sdcard/cover_flow/image9.pkm"
	};

	int i;

	/* Initialize application structure. */
	app.animation    = 0;
	app.redraw       = 1;
	app.angle        = 25.0f;
	app.currentImage = NUM_IMAGES / 2;
	app.automatic    = 1;

	width   = w;
	height  = h;
	samples = 0;
	etc1    = 0;
	frames  = 0;

	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG,
								"%s Enter: width=%d, height=%d.",
								__func__, width, height);

	/* Initialize the VDK, EGL & GL @ 640x480 resolution. */
	if (!InitGLES(width, height, samples))
	{
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG,
							"InitGLES failed.");
		/* Error. */
		run = 0;
		return JNI_FALSE;
	}

	/* Load textures. */
	for (i = 0; i < NUM_IMAGES; ++i)
	{
		FILE* file;
		void * bits;
		GLenum  format;
		GLsizei width;
		GLsizei height;
		GLsizei bytes;

		/* Load the texture. */
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG,
							"Loading texture %d of %d.\n",
							i + 1, NUM_IMAGES);

		glGenTextures(1, &app.textures[i]);
		glBindTexture(GL_TEXTURE_2D, app.textures[i]);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		if (etc1)
		{
			file = fopen(etc1Textures[i], "rb");
			bits = LoadPKM(file, &format, &width, &height, &bytes);

			if (bits != NULL)
			{
				glCompressedTexImage2D(GL_TEXTURE_2D,
									   0,
									   format,
									   width,
									   height,
									   0,
									   bytes,
									   bits);
			}
			else
			{
				__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG,
									"Load %s failed.", etc1Textures[i]);
			}

			/* Set minification filter to linear. */
			glTexParameterf(GL_TEXTURE_2D,
							GL_TEXTURE_MIN_FILTER,
							GL_LINEAR);
		}
		else
		{
			file = fopen(tgaTextures[i], "rb");
			bits = LoadTGA(file, &format, &width, &height);

			glTexParameteri(GL_TEXTURE_2D,
							GL_GENERATE_MIPMAP,
							GL_TRUE);

			if (bits != NULL)
			{
				glTexImage2D(GL_TEXTURE_2D,
							 0,
							 format,
							 width,
							 height,
							 0,
							 format,
							 GL_UNSIGNED_BYTE,
							 bits);
			}
			else
			{
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG,
								"Load %s failed.", tgaTextures[i]);
			}

			/* Set minification filter to tri-linear. */
			glTexParameterf(GL_TEXTURE_2D,
							GL_TEXTURE_MIN_FILTER,
							GL_LINEAR);
		}

		/* Set magnification filter to linear. */
        glTexParameterf(GL_TEXTURE_2D,
						GL_TEXTURE_MAG_FILTER,
						GL_LINEAR);
	}

	/* Setup the matrices. */
	AdjustMatrix(1, &app);

	/* Use the target matrix as the current matrix. */
	for (i = 0; i < NUM_IMAGES; ++i)
	{
		app.matrix[i][CURRENT_MATRIX] = app.matrix[i][TARGET_MATRIX];
	}

	/* Init start time. */
	{
	struct timeval tm;
	gettimeofday(&tm, NULL);

	start = tm.tv_sec * 1000 + tm.tv_usec / 1000;
	}

	/* Set run state. */
	run = 1;

	return JNI_TRUE;
}


JNIEXPORT void JNICALL Java_com_vivantecorp_graphics_Native_fini
  (JNIEnv * Env, jobject jo)
{
}


static int key;

JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_Native_repaint
  (JNIEnv * Env, jobject jo)
{
	/* Main loop. */
	if (run)
	{
		/* Test for keyboard event. */
		if (key != 0)
		{
			/* Dispatch on scancode. */
			switch (key)
			{
			case KEYCODE_BACK:
				/* Escape - quit. */
				run = 0;
				break;

			case KEYCODE_DPAD_RIGHT:
				/* Move to image on the right. */
				if (app.currentImage + 1 < NUM_IMAGES)
				{
					app.currentImage += 1;
					AdjustMatrix(STEPS, &app);
				}
				break;

			case KEYCODE_DPAD_LEFT:
				/* Move to image on the left. */
				if (app.currentImage > 0)
				{
					app.currentImage -= 1;
					AdjustMatrix(STEPS, &app);
				}
				break;

			case KEYCODE_A:
			case KEYCODE_G:
			case KEYCODE_R:
				/* Flip auto-run. */
				app.automatic = (app.automatic == 0) ? 1 : 0;
				break;

			case KEYCODE_COMMA:
			case KEYCODE_MINUS:
				/* Keypad -, rotate left. */
				app.angle -= 2.5f;
				app.redraw = 1;
				AdjustMatrix(app.animation, &app);
				break;

			case KEYCODE_PERIOD:
			case KEYCODE_PLUS:
				/* Keypad +, rotate right. */
				app.angle += 2.5f;
				app.redraw = 1;
				AdjustMatrix(app.animation, &app);
				break;

			case KEYCODE_SPACE:
				/* Reset rotation. */
				app.angle  = 25.0f;
				app.redraw = 1;
				AdjustMatrix(app.animation, &app);
				break;

            default:
                break;
			}

			key = 0;
		}
		else
		{
			/* No event waiting - render a frame. */
			Render(&app);

			frames ++;

			if (!(frames & 0xFF))
			{
				struct timeval tm;
				unsigned int time;

				gettimeofday(&tm, NULL);
				time = tm.tv_sec * 1000 + tm.tv_usec / 1000;

				if ((time - start) > 1000)
				{
					__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG,
										"fps: %.2f\n",
										frames * 1000.0f / (time - start));

					frames = 0;
					start = time;
				}
			}
		}

		return JNI_FALSE;
	}

	return JNI_TRUE;
}


JNIEXPORT void JNICALL Java_com_vivantecorp_graphics_Native_key
  (JNIEnv * Env, jobject jo, jint k, jboolean down)
{
	if (down == 1)
	{
		key = k;
	}
}


JNIEXPORT void JNICALL Java_com_vivantecorp_graphics_Native_touch
  (JNIEnv * Env, jobject jo, jint x, jint y, jboolean down)
{}


/*******************************************************************************
** Initialization code.
*/

int
InitGLES(
	int Width,
	int Height,
	int Samples
	)
{
	/***************************************************************************
	** Setup the GL. **********************************************************/
	glViewport(0, 0, width, height);

	/* Enable depth testing. */
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	/* Set clear colors. */
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
	glClearDepthf(1.0f);

    /* Enable smoothing. */
    glShadeModel(GL_SMOOTH);

	{
		/* Vertex buffer. */
		static const GLfloat buffer[] =
		{
			/* Image. */
			-1.0f,  1.0f, 0.0f,            0.0f,			0.8f, 0.8f, 0.8f, 0.0f,
			 1.0f,  1.0f, 320.0f / 512.0f, 0.0f,			0.8f, 0.8f, 0.8f, 0.0f,
			-1.0f,  0.0f, 0.0f,            468.0f / 512.0f,	0.8f, 0.8f, 0.8f, 0.0f,
			 1.0f,  0.0f, 320.0f / 512.0f, 468.0f / 512.0f,	0.8f, 0.8f, 0.8f, 0.0f,

			 /* Reflection. */
			-1.0f,  0.0f, 0.0f,			   468.0f / 512.0f,	0.3f, 0.3f, 0.3f, 0.0f,
			 1.0f,  0.0f, 320.0f / 512.0f, 468.0f / 512.0f,	0.3f, 0.3f, 0.3f, 0.0f,
			-1.0f, -1.0f, 0.0f,			   0.0f,			0.3f, 0.3f, 0.3f, 0.0f,
			 1.0f, -1.0f, 320.0f / 512.0f, 0.0f,			0.3f, 0.3f, 0.3f, 0.0f,
		};

		glBindBuffer(GL_ARRAY_BUFFER, 1);
		glBufferData(GL_ARRAY_BUFFER, sizeof(buffer), buffer, GL_STATIC_DRAW);

		/* Set vertex array. */
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2,
						GL_FLOAT,
						8 * sizeof(GLfloat),
						(GLvoid *) 0);

		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4,
					   GL_FLOAT,
					   8 * sizeof(GLfloat),
					   (GLvoid*) (4 * sizeof(GLfloat)));

		/* Set texture array. */
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2,
						  GL_FLOAT,
						  8 * sizeof(GLfloat),
						  (GLvoid *) (2 * sizeof(GLfloat)));
	}

	/* Setup projection. */
    glMatrixMode(GL_PROJECTION);
	glFrustumf(-1, 1, -1, 1, 1.0f, 2.0f);

	glMatrixMode(GL_MODELVIEW);

	/* Enable textures. */
    glEnable(GL_TEXTURE_2D);

	/* Load texture and modulate with constant color. */
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
	glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);

	glTexEnvf(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);

	/* Success. */
	return 1;
}

/*******************************************************************************
** Render code.
*/

void
Render(
	CoverFlow * App
	)
{
	int image;
	int left, right;

	/* See if we have still some animation left. */
	if (App->animation > 0)
	{
		/* Next animation step. */
		App->animation -= 1;
		App->redraw     = 1;
	}

	/* See if auto-run is enabled. */
	else if (App->automatic)
	{
		/* Change to new image. */
		App->currentImage += App->automatic;

		/* Make sure imagea are in-bound. */
		if (App->currentImage < 0)
		{
			App->currentImage = 1;
			App->automatic    = 1;
		}
		else if (App->currentImage >= NUM_IMAGES)
		{
			App->currentImage = NUM_IMAGES - 2;
			App->automatic    = -1;
		}

		AdjustMatrix(STEPS, App);
		App->redraw = 1;
	}

	/* Only execute of we have something to render. */
	if (!App->redraw)
	{
		return;
	}

	/* Clear color and depth buffers. */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	image = App->currentImage;
	left  = image > 0;
	right = (image + 1) < NUM_IMAGES;

	/* Process all images. */
	while (left || right)
	{
		if (App->animation > 1)
		{
			/* Use "Step" to move to new location. */
			App->matrix[image][CURRENT_MATRIX].x        +=
				App->matrix[image][STEP_MATRIX].x;
			App->matrix[image][CURRENT_MATRIX].y        +=
				App->matrix[image][STEP_MATRIX].y;
			App->matrix[image][CURRENT_MATRIX].z        +=
				App->matrix[image][STEP_MATRIX].z;
			App->matrix[image][CURRENT_MATRIX].sizeX    +=
				App->matrix[image][STEP_MATRIX].sizeX;
			App->matrix[image][CURRENT_MATRIX].sizeY    +=
				App->matrix[image][STEP_MATRIX].sizeY;
			App->matrix[image][CURRENT_MATRIX].rotation +=
				App->matrix[image][STEP_MATRIX].rotation;
			App->matrix[image][CURRENT_MATRIX].color    +=
				App->matrix[image][STEP_MATRIX].color;
		}
		else
		{
			/* Arrived at final location. */
			App->matrix[image][CURRENT_MATRIX] =
				App->matrix[image][TARGET_MATRIX];
		}

		/* Draw the image. */
		DrawImage(image,
				  App->textures[image],
				  &App->matrix[image][CURRENT_MATRIX]);

		if (left)
		{
			image -= 1;
			left   = image > 0;
		}

		else
		{
			if (image < App->currentImage)
			{
				image = App->currentImage;
			}

			image += 1;
			right  = image < NUM_IMAGES;
		}
	}

	/* Rendering done. */
	App->redraw = 0;
}

void
AdjustMatrix(
	int Steps,
	CoverFlow * App
	)
{
	float x, color;
	int i;

	/* Set matrix for focused image. */
	App->matrix[App->currentImage][TARGET_MATRIX].x        =  0.0f;
	App->matrix[App->currentImage][TARGET_MATRIX].y        =  0.0f;
	App->matrix[App->currentImage][TARGET_MATRIX].z        = -1.05f;
	App->matrix[App->currentImage][TARGET_MATRIX].sizeX    =  0.2f;
	App->matrix[App->currentImage][TARGET_MATRIX].sizeY    =  0.8f;
	App->matrix[App->currentImage][TARGET_MATRIX].rotation =  0.0f;
	App->matrix[App->currentImage][TARGET_MATRIX].color    =  1.0f;

	/* Set matrix for left images. */
	x     = -0.3f;
	color =  0.7f;
	for (i = App->currentImage - 1; i >= 0; --i)
	{
		App->matrix[i][TARGET_MATRIX].x        = x;
		App->matrix[i][TARGET_MATRIX].y        =  0.0f;
		App->matrix[i][TARGET_MATRIX].z        = -1.2f;
		App->matrix[i][TARGET_MATRIX].sizeX    =  0.2f;
		App->matrix[i][TARGET_MATRIX].sizeY    =  0.8f;
		App->matrix[i][TARGET_MATRIX].rotation = App->angle;
		App->matrix[i][TARGET_MATRIX].color    = color;
		x    -= 0.2f;
		color = max(color - 0.1f, 0.5f);
	}

	/* Set matrix for right images. */
	x     = 0.3f;
	color = 0.7f;
	for (i = App->currentImage + 1; i < NUM_IMAGES; ++i)
	{
		App->matrix[i][TARGET_MATRIX].x        = x;
		App->matrix[i][TARGET_MATRIX].y        =  0.0f;
		App->matrix[i][TARGET_MATRIX].z        = -1.2f;
		App->matrix[i][TARGET_MATRIX].sizeX    =  0.2f;
		App->matrix[i][TARGET_MATRIX].sizeY    =  0.8f;
		App->matrix[i][TARGET_MATRIX].rotation = -App->angle;
		App->matrix[i][TARGET_MATRIX].color    = color;
		x    += 0.2f;
		color = max(color - 0.1f, 0.5f);
	}

	/* Set animation steps. */
	App->animation = max(Steps, 1);

	/* Compute animation steps. */
	for (i = 0; i < NUM_IMAGES; ++i)
	{
		App->matrix[i][STEP_MATRIX].x =
			(App->matrix[i][TARGET_MATRIX].x - App->matrix[i][CURRENT_MATRIX].x)
			/ App->animation;
		App->matrix[i][STEP_MATRIX].y =
			(App->matrix[i][TARGET_MATRIX].y - App->matrix[i][CURRENT_MATRIX].y)
			/ App->animation;
		App->matrix[i][STEP_MATRIX].z =
			(App->matrix[i][TARGET_MATRIX].z - App->matrix[i][CURRENT_MATRIX].z)
			/ App->animation;
		App->matrix[i][STEP_MATRIX].sizeX =
			( App->matrix[i][TARGET_MATRIX].sizeX
			- App->matrix[i][CURRENT_MATRIX].sizeX
			)
			/ App->animation;
		App->matrix[i][STEP_MATRIX].sizeY =
			( App->matrix[i][TARGET_MATRIX].sizeY
			- App->matrix[i][CURRENT_MATRIX].sizeY
			)
			/ App->animation;
		App->matrix[i][STEP_MATRIX].rotation =
			( App->matrix[i][TARGET_MATRIX].rotation
			- App->matrix[i][CURRENT_MATRIX].rotation
			)
			/ App->animation;
		App->matrix[i][STEP_MATRIX].color =
			( App->matrix[i][TARGET_MATRIX].color
			- App->matrix[i][CURRENT_MATRIX].color
			)
			/ App->animation;
	}
}

void
DrawImage(
	int Image,
	GLint Texture,
	CoverMatrix * Matrix
	)
{
	/* Enable 2D tetxure. */
    glBindTexture(GL_TEXTURE_2D, Texture);

	/* Translate position. */
	glLoadIdentity();
	glTranslatef(Matrix->x, Matrix->y, Matrix->z);
	glRotatef(Matrix->rotation, 0, 1, 0);
	glScalef(Matrix->sizeX, Matrix->sizeY, 1);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 8);
}
