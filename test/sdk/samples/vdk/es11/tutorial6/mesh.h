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


#ifndef MESH_H
#define MESH_H

#include "main.h"
#include "fixed_point.h"

//Structure to hold the optimized mesh data for rendering
struct FixedMesh
{
  GLshort indexCounter;
  GLshort vertexCounter;
  GLshort *Indices;
  GLfixed *Geometry;
  GLfixed *Normals;
  GLfixed *TexCoord;
};

//Structure to hold the data readed from the file
struct GenericObjectData
{
  char Name[128];
  char ParentName[128];
  unsigned int *Indices;
  float *Geometry;
  float *Normals;
  float *TexCoord;
  unsigned int iC;
  unsigned int vC;
};

//GSD file header
struct GSDHeader
{
  char id[32];
  char version[16];
  int numberOfSubObjects;
};

class Mesh
{
public:
  Mesh(const char *filename);
  ~Mesh();
  void Draw();
  bool GetState () {return m_state;};

private:
 FixedMesh m_mesh;
 bool m_state;

};
#endif









