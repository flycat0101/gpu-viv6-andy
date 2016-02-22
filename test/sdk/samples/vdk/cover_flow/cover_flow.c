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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

static int STEPS = 16;

/*******************************************************************************
** Program entry point.
*/

int
main(
    int argc,
    char * argv[]
    )
{
    int run = 1, i;
    static vdkEGL egl;
    static CoverFlow app;
    int width = 640;
    int height = 480;
    int samples = 0;
    int etc1 = 0;

    const char * tgaTextures[NUM_IMAGES] =
    {
        "image1.tga",
        "image2.tga",
        "image3.tga",
        "image4.tga",
        "image5.tga",
        "image6.tga",
        "image7.tga",
        "image8.tga",
        "image9.tga"
    };

    const char * etc1Textures[NUM_IMAGES] =
    {
        "image1.pkm",
        "image2.pkm",
        "image3.pkm",
        "image4.pkm",
        "image5.pkm",
        "image6.pkm",
        "image7.pkm",
        "image8.pkm",
        "image9.pkm"
    };

    /* Initialize application structure. */
    app.animation    = 0;
    app.redraw       = 1;
    app.angle        = 25.0f;
    app.currentImage = NUM_IMAGES / 2;
    app.automatic    = 1;

    /* Parse command line options. */
    for (i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "-width") == 0 && (i + 1 < argc))
        {
            width = atoi(argv[++i]);
        }
        else
        if (strcmp(argv[i], "-height") == 0 && (i + 1 < argc))
        {
            height = atoi(argv[++i]);
        }
        else
        if (strcmp(argv[i], "-samples") == 0 && (i + 1 < argc))
        {
            samples = atoi(argv[++i]);
        }
        else
        if (strcmp(argv[i], "-steps") == 0 && (i + 1 < argc))
        {
            STEPS = atoi(argv[++i]);
        }
        else
        if (strcmp(argv[i], "-etc1") == 0)
        {
            etc1 = 1;
        }
    }

    /* Initialize the VDK, EGL & GL @ 640x480 resolution. */
    if (!InitGLES(&egl, width, height, samples))
    {
        /* Error. */
        return 1;
    }

    /* Load textures. */
    for (i = 0; i < NUM_IMAGES; ++i)
    {
        /* Load the texture. */
        printf("Loading texture %d of %d...\n", i + 1, NUM_IMAGES);
        if (etc1)
        {
            app.textures[i] = vdkLoadTexture(&egl,
                                             etc1Textures[i],
                                             VDK_PKM,
                                             VDK_2D);

            /* Set minification filter to linear. */
            glTexParameterf(GL_TEXTURE_2D,
                            GL_TEXTURE_MIN_FILTER,
                            GL_LINEAR);
        }
        else
        {
            app.textures[i] = vdkLoadTexture(&egl,
                                             tgaTextures[i],
                                             VDK_TGA,
                                             VDK_2D);

            /* Set minification filter to tri-linear. */
            glTexParameterf(GL_TEXTURE_2D,
                            GL_TEXTURE_MIN_FILTER,
                            GL_LINEAR_MIPMAP_LINEAR);
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

    /* Set window title. */
    vdkSetWindowTitle(egl.window, "CoverFlow Sample");

    /* Show the window. */
    vdkShowWindow(egl.window);

    /* Main loop. */
    while(run)
    {
        vdkEvent event;
        unsigned int start = vdkGetTicks();
        int frames = 0;

        /* Get an event. */
        if (vdkGetEvent(egl.window, &event))
        {
            /* Test for keyboard event. */
            if ((event.type == VDK_KEYBOARD)
            &&  event.data.keyboard.pressed
            )
            {
                /* Dispatch on scancode. */
                switch (event.data.keyboard.scancode)
                {
                case VDK_ESCAPE:
                    /* Escape - quit. */
                    run = 0;
                    break;

                case VDK_RIGHT:
                    /* Move to image on the right. */
                    if (app.currentImage + 1 < NUM_IMAGES)
                    {
                        app.currentImage += 1;
                        AdjustMatrix(STEPS, &app);
                    }
                    break;

                case VDK_LEFT:
                    /* Move to image on the left. */
                    if (app.currentImage > 0)
                    {
                        app.currentImage -= 1;
                        AdjustMatrix(STEPS, &app);
                    }
                    break;

                case VDK_A:
                case VDK_G:
                case VDK_R:
                    /* Flip auto-run. */
                    app.automatic = (app.automatic == 0) ? 1 : 0;
                    break;

                case VDK_COMMA:
                case VDK_PAD_HYPHEN:
                    /* Keypad -, rotate left. */
                    app.angle -= 2.5f;
                    app.redraw = 1;
                    AdjustMatrix(app.animation, &app);
                    break;

                case VDK_PERIOD:
                case VDK_PAD_PLUS:
                    /* Keypad +, rotate right. */
                    app.angle += 2.5f;
                    app.redraw = 1;
                    AdjustMatrix(app.animation, &app);
                    break;

                case VDK_SPACE:
                    /* Reset rotation. */
                    app.angle  = 25.0f;
                    app.redraw = 1;
                    AdjustMatrix(app.animation, &app);
                    break;

                default:
                    break;
                }
            }

            /* Application closing. */
            else if (event.type == VDK_CLOSE)
            {
                /* Quit. */
                run = 0;
            }
        }

        else
        {
            unsigned int time;

            /* No event waiting - render a frame. */
            Render(&egl, &app);

            frames ++;

            if (!(frames & 0xFF))
            {
                    if ((time = vdkGetTicks()) - start > 1000)
                    {
                            printf("fps: %.2f\n", frames * 1000.0f / (time - start));

                            frames = 0;
                            start = time;
                    }
            }
        }
    }

    /* Close the VDK & EGL. */
    vdkFinishEGL(&egl);

    /* Success. */
    return 0;
}

/*******************************************************************************
** Initialization code.
*/

int
InitGLES(
    vdkEGL * Egl,
    int Width,
    int Height,
    int Samples
    )
{
    const EGLint attributes[] =
    {
        EGL_RED_SIZE,       8,
        EGL_GREEN_SIZE,     8,
        EGL_BLUE_SIZE,      8,
        EGL_ALPHA_SIZE,     EGL_DONT_CARE,
        EGL_DEPTH_SIZE,     16,
        EGL_STENCIL_SIZE,   EGL_DONT_CARE,
        EGL_SURFACE_TYPE,   EGL_WINDOW_BIT,
        EGL_SAMPLES,        Samples,
        EGL_NONE,
    };

    /* Setup the VDK and EGL. */
    if (!vdkSetupEGL(-1, -1, Width, Height, attributes, NULL, NULL, Egl))
    {
        /* Error. */
        return 0;
    }

    /***************************************************************************
    ** Setup the GL. **********************************************************/

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
            -1.0f,  1.0f, 0.0f,            0.0f,
             1.0f,  1.0f, 320.0f / 512.0f, 0.0f,
            -1.0f,  0.0f, 0.0f,            468.0f / 512.0f,
             1.0f,  0.0f, 320.0f / 512.0f, 468.0f / 512.0f,

             /* Reflection. */
            -1.0f,  0.0f, 0.0f,            468.0f / 512.0f,
             1.0f,  0.0f, 320.0f / 512.0f, 468.0f / 512.0f,
            -1.0f, -1.0f, 0.0f,            0.0f,
             1.0f, -1.0f, 320.0f / 512.0f, 0.0f,

             /* Vertex color. */
            0.8f, 0.8f, 0.8f, 0.0f,
            0.8f, 0.8f, 0.8f, 0.0f,
            0.8f, 0.8f, 0.8f, 0.0f,
            0.8f, 0.8f, 0.8f, 0.0f,

            0.3f, 0.3f, 0.3f, 0.0f,
            0.3f, 0.3f, 0.3f, 0.0f,
            0.3f, 0.3f, 0.3f, 0.0f,
            0.3f, 0.3f, 0.3f, 0.0f,
        };

        glBindBuffer(GL_ARRAY_BUFFER, 1);
        glBufferData(GL_ARRAY_BUFFER, sizeof(buffer), buffer, GL_STATIC_DRAW);

        /* Set vertex array. */
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2,
                        GL_FLOAT,
                        4 * sizeof(GLfloat),
                        (GLvoid *) 0);
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(4,
                        GL_FLOAT,
                        4 * sizeof(GLfloat),
                        (GLvoid*) (8 * 4 * sizeof(GLfloat)));

        /* Set texture array. */
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2,
                          GL_FLOAT,
                          4 * sizeof(GLfloat),
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
    vdkEGL * Egl,
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

    vdkSwapEGL(Egl);
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
