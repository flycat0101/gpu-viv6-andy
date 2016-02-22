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


#define _CRT_SECURE_NO_WARNINGS
#ifdef UNDER_CE
#include <windows.h>
#endif
#include <stdio.h>
#include "mesh.h"
#include "fixed_point.h"

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

Mesh::Mesh(const char *filename,bool useVBO)
{
  m_state = true;
  m_useVBO = useVBO;
  m_mesh.Geometry = NULL;
  m_mesh.Indices = NULL;
  m_mesh.Normals = NULL;
  m_mesh.TexCoord = NULL;


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
  {
    m_state = false;
    return;
  }

  /*The header holds a brief description of the file, the version number, and the number of meshes
    that are stored in the file. This type of files are thought for static meshes only*/
  fread(&header.id,sizeof(char) * 32,1,meshFile);
  fread(&header.version,sizeof(char) * 16,1,meshFile);
  fread_bit32(&header.numberOfSubObjects,sizeof(int),meshFile);

  //Check if there is at least one object
  if(header.numberOfSubObjects < 1)
  {
    m_state = false;
    fclose(meshFile);
    return;
  }

  GenericObjectData o;

  // we only will use the first object, so we won't iterate over the others, if they exist
  fread(o.Name,sizeof(char)*128,1,meshFile); //read the object name
  fread(o.ParentName,sizeof(char)*128,1,meshFile); //Read the name of the parent object (useful for hierarchies)
  fread_bit32(&o.iC,sizeof(unsigned int),meshFile); //read the number of vertex indices
  fread_bit32(&o.vC,sizeof(unsigned int),meshFile); //read the number of vertices


  //allocate enough space for the indices and the GLshort version of them
  o.Indices = new unsigned int[o.iC];
  m_mesh.Indices = new GLushort[o.iC];
  fread_bit32(o.Indices,sizeof(unsigned int) * o.iC,meshFile); // read all indices

  //allocate enough space for the vertices and the GLfixed version of them
  o.Geometry = new float[o.vC * 3];
  m_mesh.Geometry = new GLfixed[o.vC * 3];
  fread_bit32(o.Geometry,o.vC * 3 * sizeof(float),meshFile); //read all vertices (1 vertex = 3 floats)

  //allocate enough space for the texture coordinates and the GLfixed version of them
  o.TexCoord = new float[o.vC * 2];
  m_mesh.TexCoord = new GLfixed[o.vC * 2];
  fread_bit32(o.TexCoord,o.vC * 2 * sizeof(float),meshFile);//read all texcoords (1 tex coord = 2 floats)

  //allocate enough space for the normals and the GLfixed version of them
  o.Normals= new float[o.vC * 3];
  m_mesh.Normals = new GLfixed[o.vC * 3];
  fread_bit32(o.Normals,o.vC * 3* sizeof(float),meshFile);//read all normals (1 normal = 3 floats)

  // Convert data to optimized data types for OpenGL ES (GLfixed and GLshort)
  for(unsigned int i=0;i<o.vC * 3;i++)
  {
    m_mesh.Geometry[i]= FixedFromFloat(o.Geometry[i]);
    m_mesh.Normals[i] = FixedFromFloat(o.Normals[i]);
  }

  for(unsigned int i=0;i<o.vC * 2;i++)
    m_mesh.TexCoord[i] = FixedFromFloat(o.TexCoord[i]);

  for(unsigned int i=0;i<o.iC;i++)
    m_mesh.Indices[i] = (GLushort)o.Indices[i];

  m_mesh.indexCounter = (GLushort)o.iC;
  m_mesh.vertexCounter= (GLushort)o.vC;
  //delete original values, we will use only the optimized ones
  delete [] o.Indices;
  delete [] o.Geometry;
  delete [] o.Normals;
  delete [] o.TexCoord;
  fclose(meshFile); //Do not need the file opened anymore

  //if we will use the VBO features, then lets create them
  if(m_useVBO)
  {
    glGenBuffers(1, &m_indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_mesh.indexCounter * sizeof(GLushort), m_mesh.Indices, GL_STATIC_DRAW);

    //remember unbind the VBO if you wint use it in the future
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glGenBuffers(1, &m_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_mesh.vertexCounter * 3 * sizeof(GLfixed), m_mesh.Geometry, GL_STATIC_DRAW);

    glGenBuffers(1, &m_normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_mesh.vertexCounter * 3 * sizeof(GLfixed), m_mesh.Normals, GL_STATIC_DRAW);

    glGenBuffers(1, &m_texCoordBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_texCoordBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_mesh.vertexCounter * 2 * sizeof(GLfixed), m_mesh.TexCoord, GL_STATIC_DRAW);

    //remember unbind the VBO if you wint use it in the future
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //because VBO data is stored in client memory, we do not need in main memory for more time
    if(m_mesh.Geometry) delete [] m_mesh.Geometry;
    if(m_mesh.Indices) delete [] m_mesh.Indices;
    if(m_mesh.Normals) delete [] m_mesh.Normals;
    if(m_mesh.TexCoord) delete [] m_mesh.TexCoord;
  }
}
//----------------------------------------------------------------------------
Mesh::~Mesh()
{

  if(m_useVBO)
  {
    glDeleteBuffers(1, &m_indexBuffer);
    glDeleteBuffers(1, &m_vertexBuffer);
    glDeleteBuffers(1, &m_normalBuffer);
    glDeleteBuffers(1, &m_texCoordBuffer);
  }
  else
  {
    if(m_mesh.Geometry) delete [] m_mesh.Geometry;
    if(m_mesh.Indices) delete [] m_mesh.Indices;
    if(m_mesh.Normals) delete [] m_mesh.Normals;
    if(m_mesh.TexCoord) delete [] m_mesh.TexCoord;
  }
}
//----------------------------------------------------------------------------
void Mesh::Draw()
{
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  if(m_useVBO)
  {
    /*Binding the VBO and assigning NULL to the pointer, means that the next drawing call
    will use the previus uploaded data to the VBO*/
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glVertexPointer(3, GL_FIXED, 0, NULL);

    glBindBuffer(GL_ARRAY_BUFFER, m_normalBuffer);
    glNormalPointer(GL_FIXED, 0, NULL);

    glBindBuffer(GL_ARRAY_BUFFER, m_texCoordBuffer);
    glTexCoordPointer(2,GL_FIXED, 0, NULL);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
    glDrawElements(GL_TRIANGLES,m_mesh.indexCounter,GL_UNSIGNED_SHORT, NULL);

    //VBO unbinds
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
  else
  {
    glVertexPointer(3, GL_FIXED, 0, m_mesh.Geometry);
    glNormalPointer(GL_FIXED, 0, m_mesh.Normals);
    glTexCoordPointer(2,GL_FIXED, 0, m_mesh.TexCoord);
    glDrawElements(GL_TRIANGLES,m_mesh.indexCounter,GL_UNSIGNED_SHORT, m_mesh.Indices);
  }
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

















