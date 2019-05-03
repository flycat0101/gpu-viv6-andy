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


#ifndef vv_launcher_LauncherApp_INCLUDED
#define vv_launcher_LauncherApp_INCLUDED

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <glutils/TextureCube.h>
#include <glutils/Texture2D.h>
#include <glutils/Texture2DRenderSurface.h>

#include "EnvProg.h"
#include "ChicletProg.h"
#include "ContentProg.h"

#include "Chiclet.h"
#include "Geodesic.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef Texture2D Icon;

typedef struct _LauncherApp
{
    Chiclet* chiclets[max_chiclets];

    Icon* icons[max_chiclets];

    ChicletProg* chiclet_prog;
    EnvProg* env_prog;
    ContentProg* content_prog;

    TextureCube* env_tex;
    Geodesic* geodesic;

    Texture2DRenderSurface* content_tex;

    int width;
    int height;
    float aspect;

    int flip_vertical;

    // -1 means none selected
    int selected;

    Vec3f light_dir;
    Matf proj_mat;

    Matf env_mat;
    float env_y_rot;

    GLuint cubeTex;
} LauncherApp;

LauncherApp* LauncherAppConstruct(int width, int height);

void LauncherAppDestroy(LauncherApp* App);

/*
  'tick' returns either zero, or the name of the application to
  launch.
 */
const char* LauncherAppTick(LauncherApp* App, float dt);

void LauncherAppDraw(LauncherApp* App, int width, int height);

void LauncherAppSelect(LauncherApp* App, int);

void flipVertical(LauncherApp*, int);

// render into texture buffer
void LauncherAppRenderContent(LauncherApp* App);

void LauncherAppDrawEnv(LauncherApp*);

void LauncherAppDrawChiclets(LauncherApp*);

void LauncherAppValidateProgs(LauncherApp*);

#ifdef __cplusplus
}
#endif

#endif

