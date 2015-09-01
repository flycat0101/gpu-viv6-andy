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


#include "Chiclet.h"

#include <math.h>
#include <math/Mat.h>

void gridPos(Vec2f* V, int index, int total)
{
	int column = index % 3;
	int row = 2 - (index / 3);

	int x = column - 1;
	int y = row - 1;

	V->v[0] = (float)x;
	V->v[1] = (float)y;
}



Chiclet* ChicletConstruct(int i, float a)
{
	Vec2f gp;

	Chiclet* ch = (Chiclet*)malloc(sizeof (Chiclet));
	if (ch == NULL)
	{
		return NULL;
	}
	ch->index = i;
	ch->aspect = a;
	ch->t = 0.0f;
	ch->selected = 0;

	// grid position
	gridPos(&gp, i, max_chiclets);

	ch->home_x_spin = 0.0f;
	ch->home_y_spin = 0.0f;
	//	home_pos = Vec3<float>(gp[0], gp[1] * aspect, -1.6f);
	ch->home_pos.v[0] = gp.v[0];
	ch->home_pos.v[1] = gp.v[1] * ch->aspect;
	ch->home_pos.v[2] = -2.6f;

	//	front_x_spin = 180.0f;
	//	front_y_spin = -180.0f;
	if ((i%2)==0)
	{
		ch->front_x_spin = 360.0f;
		ch->front_y_spin = 0.0f;
	}
	else
	{
		ch->front_x_spin = 0.0f;
		ch->front_y_spin = 360.0f;
	}

	ch->front_pos.v[0] = 0.0f;
	ch->front_pos.v[1] = 0.0f;
	ch->front_pos.v[2] = -1.0f;

	ch->x_spin = ch->home_x_spin;
	ch->y_spin = ch->home_y_spin;
	VecCopy3f(&ch->pos, &ch->home_pos);

	return ch;
}


void ChicletDestroy(Chiclet* Ch)
{
	assert(Ch != NULL);

	free(Ch);
}


float ChicletGetZ(Chiclet* Ch)
{
	return Ch->pos.v[2];
}


void ChicletInit(Chiclet* Ch)
{
}


void ChicletSelect(Chiclet* Ch, int s)
{
	Ch->selected = s;
}


int ChicletIsSelected(Chiclet* Ch)
{
	return (Ch->t == 1.0f);
}


void ChicletTick(Chiclet* Ch, float dt)
{
	Matf m;
	Vec3f v1;
	Vec3f v2;
	float f;
	float h;

	if (Ch->selected)
	{
		Ch->t += dt * 0.8f;
		if (Ch->t > 1.0f)
			Ch->t = 1.0f;
	}
	else
	{
		Ch->t -= dt * 1.0f;
		if (Ch->t<0.0f)
			Ch->t = 0.0f;
	}

	f = ease(Ch->t);
	h = 1.0f - f;

	VecScale3f(&v1, &Ch->front_pos, f);
	VecScale3f(&v2, &Ch->home_pos, h);
	VecAdd3f(&Ch->pos, &v1, &v2);

	Ch->x_spin = Ch->front_x_spin * f + Ch->home_x_spin * h;
	Ch->y_spin = Ch->front_y_spin * f + Ch->home_y_spin * h;

	Ch->x_spin *= 2.0f;
	Ch->y_spin *= 2.0f;

	MatTranslatef(&Ch->mat, Ch->pos.v[0], Ch->pos.v[1], Ch->pos.v[2]);

	RotXDegf(&m, Ch->x_spin);
	MatMulf(&Ch->mat, &m);

	RotYDegf(&m, Ch->y_spin);
	MatMulf(&Ch->mat, &m);
}


const Matf* ChicletGetMat(Chiclet* Ch)
{
	return &Ch->mat;
}

