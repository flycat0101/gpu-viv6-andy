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


#ifndef vv_launcher_Geodesic_INCLUDED
#define vv_launcher_Geodesic_INCLUDED

#include <math/Vec.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Face
{
    unsigned short f[3];
} Face;

typedef struct _Geodesic
{
    unsigned int vertex_count;
    unsigned int face_count;
    unsigned int edge_count;

    Vec3f* vertices;
    Face* faces;
    Vec3f* face_normals;
    Vec3f* vertex_normals;
} Geodesic;

Geodesic* GeodesicConstruct(unsigned int Subdivide);

void GeodesicDestroy(Geodesic* Geo);

void GeodesicDraw(Geodesic* Geo);

void GeodesicDrawFlat(Geodesic* Geo);

// Not called by default!
void GeodesicBuildFaceNormals(Geodesic* Geo);

void GeodesicBuildVertexNormals(Geodesic* Geo);

#ifdef __cplusplus
}
#endif

#endif

