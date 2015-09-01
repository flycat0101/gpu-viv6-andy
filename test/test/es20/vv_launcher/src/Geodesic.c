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


#include "Geodesic.h"

#include <math.h>
#include <assert.h>
#include <math/icosa.h>

typedef struct _Bisector Bisector;

static unsigned int _FindBisector(Geodesic* Geo,
								  Bisector** Bis,
								  unsigned int* NextVertex,
								  unsigned int a,
								  unsigned int b
								  );

static void _BisectorDestroy(Bisector** Bis);


Geodesic* GeodesicConstruct(unsigned int subdivide)
{
	Geodesic* geo = (Geodesic*)malloc(sizeof (Geodesic));

	if (subdivide == 0)
	{
		unsigned int i;

		geo->vertex_count = 12;
		geo->face_count = 20;
		geo->edge_count = 30;

		geo->vertices = (Vec3f*)malloc(geo->vertex_count * sizeof (Vec3f));
		geo->faces = (Face*)malloc(geo->face_count * sizeof (Face));
		geo->face_normals = (Vec3f*)malloc(geo->face_count * sizeof (Vec3f));
		geo->vertex_normals = (Vec3f*)malloc(geo->vertex_count * sizeof (Vec3f));

		for (i = 0; i < geo->vertex_count; ++i)
		{
			geo->vertices[i].v[0] = (float)(icosa.vertices[i][0]);
			geo->vertices[i].v[1] = (float)(icosa.vertices[i][1]);
			geo->vertices[i].v[2] = (float)(icosa.vertices[i][2]);
		}

		for (i = 0; i < geo->face_count; ++i)
		{
			geo->faces[i].f[0] = icosa.indices[i][0];
			geo->faces[i].f[1] = icosa.indices[i][1];
			geo->faces[i].f[2] = icosa.indices[i][2];
		}

	}
	else
	{
		Geodesic* that = GeodesicConstruct(subdivide - 1);
		unsigned int i,j;
		unsigned int next_vertex = that->vertex_count;
		unsigned int next_face = 0;
		Bisector* bisectors;

		geo->vertex_count = that->vertex_count+that->edge_count;
		geo->edge_count = that->edge_count * 2+that->face_count * 3;
		geo->face_count = that->face_count * 4;

		assert(geo->edge_count+2==geo->face_count+geo->vertex_count);

		geo->vertices = (Vec3f*)malloc(geo->vertex_count * sizeof (Vec3f));
		geo->faces = (Face*)malloc(geo->face_count * sizeof (Face));
		geo->face_normals = (Vec3f*)malloc(geo->face_count * sizeof (Vec3f));
		geo->vertex_normals = (Vec3f*)malloc(geo->vertex_count * sizeof (Vec3f));


		for (i=0; i<that->vertex_count; ++i)
		{
			VecCopy3f(&geo->vertices[i], &that->vertices[i]);
		}

		next_vertex = that->vertex_count;
		next_face = 0;
		bisectors = NULL;

		for (j=0; j<that->face_count; ++j)
		{
			Face* face = &(that->faces[j]);
			unsigned int face1, face2, face3, face4;
			unsigned short b[3]; // bisector indices

			b[0] = (unsigned short)_FindBisector(geo, &bisectors, &next_vertex, face->f[0], face->f[1]);
			b[1] = (unsigned short)_FindBisector(geo, &bisectors, &next_vertex, face->f[1], face->f[2]);
			b[2] = (unsigned short)_FindBisector(geo, &bisectors, &next_vertex, face->f[2], face->f[0]);

			face1 = next_face++;
			face2 = next_face++;
			face3 = next_face++;
			face4 = next_face++;

			geo->faces[face1].f[0] = b[0];
			geo->faces[face1].f[1] = b[1];
			geo->faces[face1].f[2] = b[2];

			geo->faces[face2].f[0] = face->f[0];
			geo->faces[face2].f[1] = b[0];
			geo->faces[face2].f[2] = b[2];

			geo->faces[face3].f[0] = face->f[1];
			geo->faces[face3].f[1] = b[1];
			geo->faces[face3].f[2] = b[0];

			geo->faces[face4].f[0] = face->f[2];
			geo->faces[face4].f[1] = b[2];
			geo->faces[face4].f[2] = b[1];
		}

		_BisectorDestroy(&bisectors);

		assert(next_vertex == geo->vertex_count);
		assert(next_face == geo->face_count);

		GeodesicDestroy(that);
	}

	return geo;
}


void GeodesicDestroy(Geodesic* Geo)
{
	assert(Geo != NULL);

	free(Geo->vertices);
	free(Geo->faces);
	free(Geo->face_normals);
	free(Geo->vertex_normals);

	free(Geo);
}

struct _Bisector
{
	unsigned int a;
	unsigned int b;
	unsigned int bis;
	Bisector* next;
};

unsigned int _FindBisector(Geodesic* Geo, Bisector** Bis, unsigned int* NextVertex, unsigned int a, unsigned int b)
{
	Bisector* bisector = *Bis;
	unsigned int i;

	while (bisector != NULL)
	{
		if ((bisector->a == a && bisector->b == b) ||
			(bisector->b == a && bisector->a == b))
		{
			return bisector->bis;
		}
		bisector = bisector->next;
	}

	i = *NextVertex;
	(*NextVertex) ++;

	VecAdd3f(&Geo->vertices[i], &Geo->vertices[a], &Geo->vertices[b]);
	VecNormalize3f(&Geo->vertices[i]);

	bisector = (Bisector*)malloc(sizeof (Bisector));
	bisector->a = a;
	bisector->b = b;
	bisector->bis = i;
	bisector->next = *Bis;

	*Bis = bisector;

	return i;
}

void _BisectorDestroy(Bisector** Bis)
{
	Bisector* bisector = *Bis;

	while (bisector != NULL)
	{
		*Bis = bisector->next;
		free(bisector);
		bisector = *Bis;
	}
}


void GeodesicBuildFaceNormals(Geodesic* Geo)
{

	Vec3f v1;
	Vec3f v2;
	unsigned int i;

	for (i = 0; i<Geo->face_count; ++i)
	{
		VecSub3f(&v1, &Geo->vertices[Geo->faces[i].f[1]], &Geo->vertices[Geo->faces[i].f[0]]);
		VecSub3f(&v2, &Geo->vertices[Geo->faces[i].f[2]], &Geo->vertices[Geo->faces[i].f[0]]);

		VecCross3f(&Geo->face_normals[i], &v1, &v2);
		VecNormalize3f(&Geo->face_normals[i]);
	}
}


void GeodesicBuildVertexNormals(Geodesic* Geo)
{
	unsigned int i;
	unsigned int j;

	GeodesicBuildFaceNormals(Geo);

	for (i = 0; i < Geo->vertex_count; ++i)
	{
		for (j = 0; j < 3; ++j)
		{
			Geo->vertex_normals[i].v[j] = 0.0f;
		}
	}

	for (i = 0; i<Geo->face_count; ++i)
	{
		for (j=0; j<3; ++j)
		{
			Geo->vertex_normals[Geo->faces[i].f[j]].v[0]
				+= Geo->face_normals[i].v[0];
			Geo->vertex_normals[Geo->faces[i].f[j]].v[1]
				+= Geo->face_normals[i].v[1];
			Geo->vertex_normals[Geo->faces[i].f[j]].v[2]
				+= Geo->face_normals[i].v[2];
		}
	}

	for (i = 0; i<Geo->vertex_count; ++i)
	{
		VecNormalize3f(&Geo->vertex_normals[i]);
	}
}


