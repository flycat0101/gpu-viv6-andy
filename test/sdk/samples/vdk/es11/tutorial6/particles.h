/****************************************************************************
*
*    Copyright 2012 - 2017 Vivante Corporation, Santa Clara, California.
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


#ifndef PARTICLES_H
#define PARTICLES_H

#include "main.h"
#include "fixed_point.h"

class Texture;

// This variable is (-0.5 *9.81), needed for the parabolic shot equation.
const GLfloat GRAVITY = -4.905f;

#define PIDIV180 0.01745329f
inline float DEGTORAD(int x) { return x * PIDIV180; }

class ParticleSystem
{
public:
  //particle system constructor: it receives the texture filename, the desired number of particles
  //and the position of the particle emmiter
  ParticleSystem(const char *textureFilename, int numberOfParticles, GLfixed x, GLfixed y, GLfixed z);
  ~ParticleSystem();

  //DrawParticles will update and draw the whole particle system, must be called each frame
  void DrawParticles(unsigned int elapsedTime);

private:
  void UpdateParticles(unsigned int elapsedTime); //updates the particles position based on the elapsed time with the last frame
  void ResetParticle(int i); //resets a particle under certain conditions (lifetime = 0, or position.y = 0)
  GLfloat m_sin[360]; //pregenerated sin table
  GLfloat m_cos[360]; //pregenerated cos table
  GLfloat m_emmiter[3]; //emmiter possition
  int m_numberOfParticles;
  Texture *m_texture; //particles' texture

  //Particle data
  GLfloat *m_totalLifeTime; //in milliseconds
  GLfloat *m_lifeTime;      //in milliseconds
  GLfloat *m_position;
  GLfloat *m_startSpeed;
  GLubyte *m_color;
  int     *m_startAngle;
};

#endif
