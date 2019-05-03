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


#include "LauncherApp.h"

#include <stdio.h>
#include <string.h>

#include <math/icosa.h>
#include <glutils/check.h>
#include <glutils/dds.h>
#include <glutils/log.h>

#define LAUNCH_APPLICATIONS

float const rec_verts[4][3] =
{
    {-0.5f, -0.5f, 0.0f},
    { 0.5f, -0.5f, 0.0f},
    {-0.5f,  0.5f, 0.0f},
    { 0.5f,  0.5f, 0.0f},
};

float const rec_uv[4][2] =
{
    {0.0f, 1.0f},
    {1.0f, 1.0f},
    {0.0f, 0.0f},
    {1.0f, 0.0f},
};

unsigned short const rec_indices[4] = {0, 1, 2, 3};

// for shaping the chiclet

float box(float x, float y)
{
    // float s = 1.1f;
    float s = 0.9f;
    return (float)((atan(x * s) * y * 2.0f / 3.141592f)/s);
    //return (atan(x * s) * y * 2.0f / 3.141592f)*1.5f;
}


// For now, hardwired icons
struct IconData
{
    char const * tex_name;
    int square;
    char const * app_name;

    float x_off;
    float y_off;
};


#define icon_count max_chiclets
struct IconData icon_data[icon_count] =
{
    // "1"
    {
        "firefox.dds",
         1,
#ifdef LAUNCH_APPLICATIONS
         NULL,
#else
         NULL,
#endif
         0.0f, -0.05f
    },
    // "2"
    {
        "envelope.dds",
         1,
#ifdef LAUNCH_APPLICATIONS
         NULL,
#else
         NULL,
#endif
         0.0f, 0.0f
    },
    // "3"
    {
        "photo_album.dds",
         1,
#ifdef LAUNCH_APPLICATIONS
         NULL,
#else
         NULL,
#endif
         0.0f, 0.0f
    },
    // "4"
    {
        "cover_flow.dds",
         1,
#ifdef LAUNCH_APPLICATIONS
         "./cover_flow.sh",
#else
         NULL,
#endif
         0.0f, 0.0f
    },
    // "5"
    {
        "Vivante-Logo.Black.Square.dds",
         1,
#ifdef LAUNCH_APPLICATIONS
         NULL,
#else
         NULL,
#endif
         0.0f, 0.0f
    },
    // "6"
    {
        "vgmark_flash.dds",
         1,
#ifdef LAUNCH_APPLICATIONS
         "./vgmark_flash.sh",
#else
         NULL,
#endif
         0.0f, 0.0f
    },
    // "7"
    {
        "3dmark06_samurai.dds",
         1,
#ifdef LAUNCH_APPLICATIONS
         "./mm06.sh",
#else
         NULL,
#endif
         0.0f, -0.1f
    },
    // "8"
    {
        "3dmark07_taiji.dds",
         1,
#ifdef LAUNCH_APPLICATIONS
         "./mm07.sh",
#else
         NULL,
#endif
         0.0f, -0.05f
    },
    // "9"
    {
        "vgmark_navi.dds",
         1,
#ifdef LAUNCH_APPLICATIONS
         "./vgmark_navi.sh",
#else
         NULL,
#endif
         0.05f, -0.1f
    }
};

static void _SortChicletZ(Chiclet* chiclets[], unsigned int indices[]);

LauncherApp* LauncherAppConstruct(int w, int h)
{
    LauncherApp* app = (LauncherApp*)malloc(sizeof (LauncherApp));
    int i;
    Image* image;

    app->width = w;
    app->height = h;
    app->aspect = (float)h/(float)w;
    app->flip_vertical = 0;
    app->selected = -1;
    app->env_y_rot = 0.0f;

    app->chiclet_prog = ChicletProgConstruct();
    app->env_prog = EnvProgConstruct();
    app->content_prog = ContentProgConstruct();

    app->content_tex = Texture2DRenderSurfaceConstruct(1024, 1024, 0, 1);

    app->geodesic = GeodesicConstruct(2);

    //   geodesic = new Geodesic(3);

    // Distort sphere into the shape of a box.
    {
        float w = 0.95f;
        float h = w * app->aspect;
        float d = 0.3f;
        unsigned int i;

        Vec3f* v = app->geodesic->vertices;

        for (i = 0; i < app->geodesic->vertex_count; ++i)
        {
            v[i].v[0] = box(v[i].v[0], w);
            v[i].v[1] = box(v[i].v[1], h);
            v[i].v[2] = box(v[i].v[2], d);
        }
        GeodesicBuildVertexNormals(app->geodesic);
    }

    image = LoadFromResource("envcube.dds");

    if (image != NULL)
    {
        // Cube map with mipmapping causes seams.
        app->env_tex = CreateCubemap(image, 0);
    }
    else
    {
        LogError("Chiclet: failed to load envcube.dds\n");
    }

    ImageDestroy(image);

    for (i = 0; i < icon_count; ++i)
    {
        char const * name = icon_data[i].tex_name;
        if (name != NULL)
        {
            Image* image = LoadFromResource(name);
             if (image != NULL)
            {
                app->icons[i] = CreateTexture2D(image, 1);
            }
            else
            {
                LogError("Chiclet::init(): failed to load \n");
             }
            ImageDestroy(image);
        }
    }

    for (i = 0; i < max_chiclets; ++i)
    {
        app->chiclets[i] = ChicletConstruct(i, app->aspect);
    }

    return app;
}


void LauncherAppDestroy(LauncherApp* App)
{
    int i;

    assert(App != NULL);

    GeodesicDestroy(App->geodesic);

    ChicletProgDestroy(App->chiclet_prog);

    EnvProgDestroy(App->env_prog);

    ContentProgDestroy(App->content_prog);

    TextureCubeDestroy(App->env_tex);

    Texture2DRenderSurfaceDestroy(App->content_tex);

    for (i = 0; i < max_chiclets; i++)
    {
        ChicletDestroy(App->chiclets[i]);
    }

    for (i = 0; i < max_chiclets; i++)
    {
        Texture2DDestroy(App->icons[i]);
    }

    free(App);
}


void LauncherAppValidateProgs(LauncherApp* App)
{
    ShaderProgramValidate(App->chiclet_prog->prog);
    ShaderProgramValidate(App->env_prog->prog);
    ShaderProgramValidate(App->content_prog->prog);
}


const char * LauncherAppTick(LauncherApp* App, float dt)
{
    char const * name = NULL;
    unsigned int i;

    // spin the environment map
    App->env_y_rot = wrap(App->env_y_rot + dt * 2, -180.0f, 180.0f);
    RotYDegf(&App->env_mat, App->env_y_rot);

    for (i = 0; i < max_chiclets; ++i)
    {
        ChicletTick(App->chiclets[i], dt);
        if (ChicletIsSelected(App->chiclets[i]))
        {
            name = icon_data[i].app_name;
#ifdef DEBUG
            if (name != NULL)
            {
                LogError("tick -> %s\n", name);
            }
#endif
        }
    }

    return name;
}


void flipVertical(LauncherApp* App, int f)
{
    App->flip_vertical = f;
}


void LauncherAppDraw(LauncherApp* App, int width, int height)
{
    // for now assume width and height don't change
    float x = 1.0f/3.0f;
    float y = x * App->aspect;
    float z = 1.0f/2.0f;

    if (App->flip_vertical)
    {
        MatFrustumf(&App->proj_mat, -x, x, y, -y, z, 5.0f);
    }
    else
    {
        MatFrustumf(&App->proj_mat, -x, x, -y, y, z, 5.0f);
    }

    LauncherAppRenderContent(App);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    LauncherAppDrawEnv(App);
    LauncherAppDrawChiclets(App);
}


void LauncherAppDrawEnv(LauncherApp* App)
{
    glViewport(0, 0, App->width, App->height);

    EnvProgUse(App->env_prog);
    EnvProgSetProj(App->env_prog, &App->proj_mat);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    glActiveTexture(GL_TEXTURE0);
    TextureCubeBind(App->env_tex);
    glUniform1i(App->env_prog->env_tex_unif, 0);

    EnvProgSetEnv(App->env_prog, &App->env_mat);
    CheckGL("setEnv");

    glEnableVertexAttribArray(App->env_prog->pos_attr);
    glVertexAttribPointer(App->env_prog->pos_attr,
                        3,
                        GL_FLOAT,
                        GL_FALSE,
                        0,
                        icosa.vertices);

    glDrawElements(GL_TRIANGLES,
                 20 * 3,
                 GL_UNSIGNED_SHORT,
                 icosa.indices);

    glDisableVertexAttribArray(App->env_prog->pos_attr);
    glActiveTexture(GL_TEXTURE0);
}


void LauncherAppDrawChiclets(LauncherApp* App)
{
    GLvoid const * indices;
    unsigned int inds[max_chiclets];
    unsigned int i;

    Vec3f light = {{0.2f, 0.5f, 0.2f}};
    VecNormalize3f(&light);

    glViewport(0, 0, App->width, App->height);

    ChicletProgUse(App->chiclet_prog);
    ChicletProgSetLight(App->chiclet_prog, &light);
    ChicletProgSetProj(App->chiclet_prog, &App->proj_mat);

    if (App->flip_vertical)
    {
        glCullFace(GL_FRONT);
    }
    else
    {
        glCullFace(GL_BACK);
    }

    glEnable(GL_CULL_FACE);
       //glEnable(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glActiveTexture(GL_TEXTURE0);
    TextureCubeBind(App->env_tex);
    glUniform1i(App->chiclet_prog->env_tex_unif, 0);

    glActiveTexture(GL_TEXTURE1);
    Texture2DBind(App->content_tex->texture);
    glUniform1i(App->chiclet_prog->content_tex_unif, 1);

    glEnableVertexAttribArray(App->chiclet_prog->pos_attr);
    glVertexAttribPointer(App->chiclet_prog->pos_attr,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          0,
                          App->geodesic->vertices);

    glEnableVertexAttribArray(App->chiclet_prog->norm_attr);

    glVertexAttribPointer(App->chiclet_prog->norm_attr,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          0,
                          App->geodesic->vertex_normals);

    indices = &App->geodesic->faces[0].f[0];

    _SortChicletZ(App->chiclets, inds);
    for (i = 0; i < max_chiclets; i++)
    {
         int index = inds[i];

         const Matf* obj_mat = ChicletGetMat(App->chiclets[index]);
         ChicletProgSetModelview(App->chiclet_prog, obj_mat);
         CheckGL("setModelview");

         ChicletProgSetEnv(App->chiclet_prog, &App->env_mat);
         CheckGL("setEnv");

         glDrawElements(GL_TRIANGLES,
                       App->geodesic->face_count * 3,
                       GL_UNSIGNED_SHORT,
                       indices);
    }

      glDisableVertexAttribArray(App->chiclet_prog->norm_attr);
      glDisableVertexAttribArray(App->chiclet_prog->pos_attr);
      glActiveTexture(GL_TEXTURE0);
      glDisable(GL_BLEND);
}


void LauncherAppRenderContent(LauncherApp* App)
{
    unsigned int inds[max_chiclets];
    unsigned int i;

      Texture2DRenderSurfaceBind(App->content_tex);

      glViewport(0,
               0,
               Texture2DRenderSurfaceGetWidth(App->content_tex),
               Texture2DRenderSurfaceGetHeight(App->content_tex)
              );

    ContentProgUse(App->content_prog);

      glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      ContentProgSetProj(App->content_prog, &App->proj_mat);

      //glCullFace(GL_BACK);
      //glEnable(GL_CULL_FACE);

      glDisable(GL_DEPTH_TEST);
      glDisable(GL_CULL_FACE);

      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      glActiveTexture(GL_TEXTURE0);
      glUniform1i(App->content_prog->tex_unif, 0);

      glEnableVertexAttribArray(App->content_prog->pos_attr);
      glVertexAttribPointer(App->content_prog->pos_attr,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          0,
                          rec_verts);

    glEnableVertexAttribArray(App->content_prog->uv_attr);
    glVertexAttribPointer(App->content_prog->uv_attr,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          0,
                          rec_uv);

    _SortChicletZ(App->chiclets, inds);
    for (i = 0; i < max_chiclets; i++)
    {
        int index = inds[i];
        Matf s;
        Matf t;

        if (App->icons[index])
        {
            Icon* icon = App->icons[index];
            float x_off = icon_data[index].x_off;
            float y_off = icon_data[index].y_off;
            Matf obj_mat;

            Texture2DBind(icon);

            if (!App->flip_vertical)
            {
                x_off = -x_off;
                y_off = -y_off;
            }

            MatScalef(&s, 0.5f, 0.5f, 0.5f);
            MatTranslatef(&t, x_off, y_off, 0.0f);
            memcpy(&obj_mat, ChicletGetMat(App->chiclets[index]), sizeof (Matf));
            MatMulf(&obj_mat, &s);
            MatMulf(&obj_mat, &t);

            ContentProgSetModelview(App->content_prog, &obj_mat);
            CheckGL("LauncherApp::renderContent: SetModelview");

            glDrawElements(GL_TRIANGLE_STRIP,
                           4,
                           GL_UNSIGNED_SHORT,
                           rec_indices);
        }
    }

    glDisableVertexAttribArray(App->content_prog->pos_attr);
    glDisableVertexAttribArray(App->content_prog->uv_attr);
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_BLEND);

    Texture2DRenderSurfaceUnbind(App->content_tex);
}


void LauncherAppSelect(LauncherApp* App, int s)
{
    if ((App->selected >= 0) && (s != App->selected))
    {
        ChicletSelect(App->chiclets[App->selected], 0);
    }

    if ((s < 0) || (s >= max_chiclets))
    {
        App->selected = -1;
    }
    else
    {
        App->selected = s;
        ChicletSelect(App->chiclets[App->selected], 1);
    }
}


void _SortChicletZ(Chiclet* chs[], unsigned int indices[])
{
    unsigned int i;
    unsigned int b;

    float data[max_chiclets];

    for (i = 0; i < max_chiclets; i++)
    {
        data[i] = ChicletGetZ(chs[i]);
        indices[i] = i;
    }

    for (b = 0; b < max_chiclets; b ++)
    {
        for (i = b + 1; i < max_chiclets; i++)
        {
            if (data[indices[i]] < data[indices[b]])
            {
                int t = indices[b];
                indices[b] = indices[i];
                indices[i] = t;
            }
        }
    }
}

