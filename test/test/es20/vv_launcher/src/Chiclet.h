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


#ifndef vv_launcher_Chiclet_INCLUDED
#define vv_launcher_Chiclet_INCLUDED

#include <GLES2/gl2.h>
#include <math/Vec.h>
#include <math/Mat.h>

#include "Geodesic.h"
#include "ChicletProg.h"

#ifdef __cplusplus
extern "C" {
#endif

// for now, hardcode a layout of 9 chiclets
#define max_chiclets  9

typedef struct _Chiclet
{
    int index;
    float aspect;

    Matf mat;

    // Home is zero. Front is one.
    float t;

    int selected;

    float home_x_spin;
    float home_y_spin;
    Vec3f home_pos;

    float front_x_spin;
    float front_y_spin;
    Vec3f front_pos;

    Vec3f pos;
    float x_spin;
    float y_spin;
} Chiclet;

Chiclet* ChicletConstruct(int index, float aspect);

void ChicletDestroy(Chiclet* Ch);

void ChicletInit(Chiclet*);

void ChicletTick(Chiclet*, float);

void ChicletSelect(Chiclet*, int);

// Animation is finished. Time to run the associated app.
int ChicletIsSelected(Chiclet*);

// For sorting back to front. (Note we're looking down neg z axis.)
float ChicletGetZ(Chiclet*);

const Matf* ChicletGetMat(Chiclet*);

#ifdef __cplusplus
}
#endif

#endif

