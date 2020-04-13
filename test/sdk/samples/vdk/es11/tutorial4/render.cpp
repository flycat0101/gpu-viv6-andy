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


#define _CRT_SECURE_NO_WARNINGS
#ifdef UNDER_CE
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include "render.h"
#include "fixed_point.h"

#define USE_VBO 1

#ifdef ANDROID_JNI
#include <utils/Log.h>
extern int width;
extern int height;
#endif

// Texture handles.
GLuint texture1 = 0;
GLuint texture2 = 0;

// Mesh.
FixedMesh * mesh = NULL;

GLuint g_vb;
GLint g_stride, g_ver, g_nor, g_tex;

bool InitGLES()
{
  /*Remember: because we are programming for a mobile device, we cant
  use any of the OpenGL ES functions that finish in 'f', we must use
  the fixed point version (they finish in 'x'*/
  glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
  //Do not want to see smoothed colors, only a plain color for face
  glShadeModel(GL_SMOOTH);
  //Enable the depth test in order to see the cube correctly
  glEnable(GL_DEPTH_TEST);

  /*Taking care of specifying correctly the winding order of the
  vertices (counter clock wise order), we can cull all back faces.
  This is probably one of the best optimizations we could do,
  because, by this way, we avoid a lot of computations that wont be
  reflected in the screen. Use  glEnable(GL_CULL_FACE) to do the work*/
  glEnable(GL_CULL_FACE);

  SetPerspective();

  //Light Settings
  //Set a 50% grey diffuse component
  GLfloat diffuse[] = {0.5f,0.5f,0.5f,1.0f};
  glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
  //Enable lighting computations
  glEnable(GL_LIGHTING);
  //Enable Light0 (switch on it)
  glEnable(GL_LIGHT0);

  //Fog settings
  //50% green fog
  GLfloat fogColor[] = { 0, 0.5f, 0, 0 };
    glFogfv(GL_FOG_COLOR, fogColor);
  /*We chosen a linear mode for fog. Fog will begin to show its effects at 20 units
    of distance from the camera, all the highest fog density will be reached at 50 units*/
    glFogx(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, 20.0f);
    glFogf(GL_FOG_END,   50.0f);
#ifndef ANDROID_JNI
  //Load our scene file
  mesh = LoadMeshFromFile("scene.gsd");
#else
  mesh = LoadMeshFromFile("/sdcard/es11/tutorial4/scene.gsd");
#endif
  return mesh ? true : false;
}
//----------------------------------------------------------------------------

void Render()
{

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glLoadIdentity();

  static int rotation = 0;
  /*"camera" setup to have a good point of view of the scene, also we will perform a rotation
     on the mesh*/
  glTranslatex(0, FixedFromInt(-5), FixedFromInt(-40));
  glRotatex(FixedFromInt(20),ONE,0,0);
  glRotatex(FixedFromInt(rotation++),0,ONE,0);


  //With x and z we compute the points of a circle, with a radius = 30, and the velocity = LightRotation
  static float LightRotation = 0;
  float x = 0,z = 0;
  LightRotation +=0.2f;

  /*According to Linux programer's manual, need to clamp the input value of sin or cos to valid value,
        otherwise return NaN and exception may be raised.*/
  if(LightRotation >= 6.4)
  {
      LightRotation = fmod(LightRotation, (float)6.4);
  }

  x = 30 * (float)sin(LightRotation); //we have to use floats, because we do not
  z = 30 * (float)cos(LightRotation); //have a fixed point version of sin and cos
  GLfixed lightPosition[] = { 0, FixedFromInt(10), 0, FixedFromInt(1) };
  lightPosition[0] = FixedFromFloat(x);  lightPosition[2] = FixedFromFloat(z);
  //Setup of Light0 position
    glLightxv(GL_LIGHT0, GL_POSITION, lightPosition);

#ifdef USE_VBO
  glEnableClientState(GL_VERTEX_ARRAY);
  glBindBuffer(GL_ARRAY_BUFFER, g_vb);
  glVertexPointer(3, GL_FIXED, g_stride, (void *)(khronos_intptr_t)g_ver);
  glNormalPointer(GL_FIXED, g_stride, (void *)(khronos_intptr_t)g_nor);
#else
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FIXED, 0, mesh->Geometry);

  //Normals are needed for the lighting computations
  glEnableClientState(GL_NORMAL_ARRAY);
  glNormalPointer(GL_FIXED, 0, mesh->Normals);
#endif

#ifdef USE_VBO
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->IndexBuffer);
  glDrawElements(GL_TRIANGLES, mesh->indexCounter, GL_UNSIGNED_SHORT, 0);
#else
  glDrawElements(GL_TRIANGLES,mesh->indexCounter,GL_UNSIGNED_SHORT,mesh->Indices);
#endif

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);

#ifndef ANDROID_JNI
  vdkSwapEGL(&egl);
#endif
}

void fread_bit32(void * buf, size_t size, FILE * file)
{
    unsigned char cdata[4];
    size_t i;

    for (i = 0; i < size / sizeof(unsigned int); i++)
    {
        fread(&cdata, sizeof(unsigned int), 1, file);
        ((unsigned int *)buf)[i] = (cdata[3] << 24) + (cdata[2] << 16)
                + (cdata[1] << 8) + cdata[0];
    }
}


//----------------------------------------------------------------------------
FixedMesh *LoadMeshFromFile(const char *filename)
{
  GSDHeader header;
  FILE *meshFile = fopen(filename,"rb");
#ifdef UNDER_CE
    if (meshFile == NULL)
    {
        wchar_t moduleName[MAX_PATH];
        char path[MAX_PATH], * p;
        GetModuleFileName(NULL, moduleName, MAX_PATH);
        wcstombs(path, moduleName, MAX_PATH);
        p = strrchr(path, '\\');
        strcpy(p + 1, filename);
        meshFile = fopen(path, "rb");
    }
#endif
  if(!meshFile)
    return NULL;

  /*The header holds a brief description of the file, the version number, and the number of meshes
    that are stored in the file. This type of files are thought for static meshes only*/
  fread(&header,sizeof(GSDHeader),1,meshFile);

  //Check if there is at least one object
  if(header.numberOfSubObjects < 1)
    return NULL;

  GenericObjectData o;
  FixedMesh *mesh = new FixedMesh;

  // we only will use the first object, so we won't iterate over the others, if they exist
  fread(o.Name,sizeof(char)*128,1,meshFile); //read the object name
  fread(o.ParentName,sizeof(char)*128,1,meshFile); //Read the name of the parent object (useful for hierarchies)
  fread_bit32(&o.iC,sizeof(unsigned int),meshFile); //read the number of vertex indices
  fread_bit32(&o.vC,sizeof(unsigned int),meshFile); //read the number of vertices

  //allocate enough space for the indices and the GLshort version of them
  o.Indices = new unsigned int[o.iC];
  mesh->Indices = new GLshort[o.iC];
  fread_bit32(o.Indices,sizeof(unsigned int) * o.iC,meshFile); // read all indices

  //allocate enough space for the vertices and the GLfixed version of them
  o.Geometry = new float[o.vC * 3];
  mesh->Geometry = new GLfixed[o.vC * 3];
  fread_bit32(o.Geometry,o.vC * 3 * sizeof(float),meshFile); //read all vertices (1 vertex = 3 floats)

  //allocate enough space for the texture coordinates and the GLfixed version of them
  o.TexCoord = new float[o.vC * 2];
  mesh->TexCoord = new GLfixed[o.vC * 2];
  fread_bit32(o.TexCoord,o.vC * 2 * sizeof(float),meshFile);//read all texcoords (1 tex coord = 2 floats)

  //allocate enough space for the normals and the GLfixed version of them
  o.Normals= new float[o.vC * 3];
  mesh->Normals = new GLfixed[o.vC * 3];
  fread_bit32(o.Normals,o.vC * 3* sizeof(float),meshFile);//read all normals (1 normal = 3 floats)
  fclose(meshFile); //Do not need the file opened anymore

  // Convert data to optimized data types for OpenGL ES (GLfixed and GLshort)
  for(unsigned int i=0;i<o.vC * 3;i++)
  {
    mesh->Geometry[i]= FixedFromFloat(o.Geometry[i]);
    mesh->Normals[i] = FixedFromFloat(o.Normals[i]);
  }

  for(unsigned int i=0;i<o.vC * 2;i++)
    mesh->TexCoord[i] = FixedFromFloat(o.TexCoord[i]);

  for(unsigned int i=0;i<o.iC;i++)
    mesh->Indices[i] = (GLshort)o.Indices[i];

  mesh->indexCounter = (GLshort)o.iC;
  mesh->vertexCounter= (GLshort)o.vC;

  //delete original values, we will use only the optimized ones
  delete [] o.Indices;
  delete [] o.Geometry;
  delete [] o.Normals;
  delete [] o.TexCoord;

#ifdef USE_VBO
  if (mesh->Indices != NULL)
  {
      glGenBuffers(1, &mesh->IndexBuffer);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->IndexBuffer);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indexCounter * sizeof (GLshort), mesh->Indices, GL_STATIC_DRAW);
  }

  GLfixed *vb;
  GLfixed *src, *dst;
  int size;

  vb = (GLfixed *)malloc(mesh->vertexCounter * 9 * sizeof(GLfixed));

  g_stride = 0;

  if (mesh->Geometry != NULL)
  {
     g_ver = 0;
         g_stride += 3*sizeof(GLfixed);
  }

  if (mesh->Normals != NULL)
  {
         g_nor = g_stride;
         g_stride += 3*sizeof(GLfixed);
  }



  dst = vb;

  for (int i = 0; i < mesh->vertexCounter; i++)
  {
     if (mesh->Geometry != NULL)
     {
         src = mesh->Geometry + 3 * i;
         size = 3 * sizeof (GLfixed);

         memcpy(dst, src, size);
         dst += 3;
     }

     if (mesh->Normals != NULL)
     {
         src = mesh->Normals + 3 * i;
         size = 3 * sizeof (GLfixed);
         memcpy(dst, src, size);
         dst += 3;
     }

  }

  glGenBuffers(1, &g_vb);
  glBindBuffer(GL_ARRAY_BUFFER, g_vb);
  glBufferData(GL_ARRAY_BUFFER, mesh->vertexCounter * 9 * sizeof(GLfixed), vb, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif

  return mesh;
}

/*******************************************************************************
*******************************************************************************/

void Perspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar)
{
    // Compute field of view.
    GLfloat ymax = zNear * (GLfloat)tan(fovy * 3.1415962f / 360.0f);
    GLfloat ymin = -ymax;

    if (aspect > 1.0f)
    {
        // Correct aspect ratio for landscape mode.
        aspect = 1.0f / aspect;
    }

    // Compute aspect ratio.
    GLfloat xmin = ymin * aspect;
    GLfloat xmax = ymax * aspect;

    // Set frustum.
    glFrustumf(xmin, xmax, ymin, ymax, zNear, zFar);
}

/*******************************************************************************
*******************************************************************************/

void SetPerspective()
{
#ifndef ANDROID_JNI
    // Query the window dimensions.
    int width, height;
    vdkGetWindowInfo(egl.window, NULL, NULL, &width, &height, NULL, NULL);
#endif
    // Compute the ratio.
    float ratio = static_cast<float>(width) / static_cast<float>(height);

    // Set the frustum matrix width a 45 degree field of view and a depth of
    // 100.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    Perspective(45.0f, ratio, 1.0f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

//----------------------------------------------------------------------------
void CleanGLES()
{
    // Delete mesh data
    delete [] mesh->Geometry;
    delete [] mesh->Indices;
    delete [] mesh->Normals;
    delete [] mesh->TexCoord;

#ifdef USE_VBO
    if (mesh->IndexBuffer)
    {
        glDeleteBuffers(1, &mesh->IndexBuffer);
    }
#endif

    delete mesh;
}

void SetFog(bool Enable)
{
    if (Enable)
    {
        glEnable(GL_FOG);
    }
    else
    {
        glDisable(GL_FOG);
    }
}
