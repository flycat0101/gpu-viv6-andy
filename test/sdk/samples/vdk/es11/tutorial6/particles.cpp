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


#include <stdlib.h>
#include <math.h>
#include "main.h"
#include "particles.h"
#include "texture.h"

ParticleSystem::ParticleSystem(const char *textureFilename, int numberOfParticles, GLfixed x, GLfixed y, GLfixed z)
{
#ifndef ANDROID_JNI
    // Set the random number generator seed.
    srand(vdkGetTicks());
#endif
    // Set the emmiter position.
    m_emmiter[0] = x / 65536.0f;
    m_emmiter[1] = y / 65536.0f;
    m_emmiter[2] = z / 65536.0f;

    // Create the particles texture: if it is a .raw texture, it must be 16x16 with two channels (16 bits).
    m_texture = new Texture(textureFilename, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, 64, 64, 16);

    m_numberOfParticles = numberOfParticles;

    m_totalLifeTime = new GLfloat[m_numberOfParticles];
    m_lifeTime      = new GLfloat[m_numberOfParticles];
    m_startSpeed    = new GLfloat[m_numberOfParticles];
    m_startAngle    = new int[m_numberOfParticles];
    m_color         = new GLubyte[m_numberOfParticles * 4]; // RGBA
    m_position      = new GLfloat[m_numberOfParticles * 3]; // XYZ


    // Initialize all particles.
    for (int i = 0; i < m_numberOfParticles; ++i)
        ResetParticle(i);

    // Initialize trigonometric tables.
    for (int i = 0; i < 360; ++i)
    {
        m_sin[i] = (GLfloat)sin(DEGTORAD(i));
        m_cos[i] = (GLfloat)cos(DEGTORAD(i));
    }
}

//----------------------------------------------------------------------------

ParticleSystem::~ParticleSystem()
{
    delete [] m_totalLifeTime;
    delete [] m_lifeTime;
    delete [] m_startSpeed;
    delete [] m_startAngle;
    delete [] m_color;
    delete [] m_position;
    delete m_texture;
}

//----------------------------------------------------------------------------

void ParticleSystem::ResetParticle(int index)
{
    // Random number between 2 and 3 (seconds).
    m_totalLifeTime[index] = 2.0f + (rand() % 1);
    m_lifeTime[index]      = m_totalLifeTime[index];
    // Random number between 5 and 8 (m/s).
    m_startSpeed[index]    = 5.0f + (rand() % 3);

    // Random number between 0 and 360 degrees.
    m_startAngle[index] = rand() % 360;

    // Random number between 0 and 255 (color is in GLubyte format: 0-255)
    int colorIndex = index * 4;
    m_color[colorIndex + 0] = rand() % 255;
    m_color[colorIndex + 1] = rand() % 255;
    m_color[colorIndex + 2] = rand() % 255;
    m_color[colorIndex + 3] = 255;

    // Set starting position at emmiter.
    int positionIndex = index * 3;
    m_position[positionIndex + 0] = m_emmiter[0];
    m_position[positionIndex + 1] = m_emmiter[1];
    m_position[positionIndex + 2] = m_emmiter[2];
}

//----------------------------------------------------------------------------

void ParticleSystem::UpdateParticles(unsigned int elapsedTime)
{
    // Convert elapsed time from milliseconds to seconds
    float felapsed = elapsedTime / 1000.0f;

    // Iterate throught all particles.
    for (int i = 0; i < m_numberOfParticles; ++i)
    {
        m_lifeTime[i] -= felapsed; // reduce the particle's lifetime
        if ((m_lifeTime[i] <= 0) || (m_position[i * 3 + 1] < 0))
        {
            // Reset the partice if has died or of touched the floor(y < 0).
            ResetParticle(i);
        }
        else
        {
            // Current life time.
            GLfloat flife = m_totalLifeTime[i] - m_lifeTime[i];

            // We will set an alpha transparency based on the particle's life time: (lifetime/totaltime) * 255.
            GLfloat aux = m_lifeTime[i] / m_totalLifeTime[i];
            m_color[i * 4 + 3] = (GLubyte) (aux * 255.0f);

            // Compute the particle's position.
            // For the vertical axis, we will use the vertical axis ecuation of the parabolic shot (-0.5*g*t*t)+(V0*t).
            // For the horizontal ones, we will use the horizontal component, in a radial direction given by
            // m_startAngle: (1/8) * (V0*sin(angle)) (the 1/8 factor is handcoded, to simulate an high elevation angle
            // of the parabolic shot.
            GLfloat temp1 = flife * flife * GRAVITY;
            GLfloat temp2 = m_startSpeed[i] * flife;
            GLfloat fHorizontalFactor = flife / 8.0f;
            int positionIndex = i * 3;
            m_position[positionIndex + 0] += m_startSpeed[i] * m_sin[m_startAngle[i]] * fHorizontalFactor;
            m_position[positionIndex + 1] += temp1 + temp2;
            m_position[positionIndex + 2] += m_startSpeed[i] * m_cos[m_startAngle[i]] * fHorizontalFactor;
        }
    }
}

//----------------------------------------------------------------------------

void ParticleSystem::DrawParticles(unsigned int elapsedTime)
{
    // Update all particles before drawing.
    UpdateParticles(elapsedTime);

    // Enable blending and disable the alpha test.
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);

    // Setup point sprites coordinate generation.
    glEnable(GL_POINT_SPRITE_OES);
    glTexEnvx(GL_POINT_SPRITE_OES, GL_COORD_REPLACE_OES, GL_TRUE);
    glPointSize(15);
    m_texture->BindTexture();

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, m_position);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, m_color);

    glDrawArrays(GL_POINTS, 0, m_numberOfParticles);

    // Restore all states.
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glTexEnvx(GL_POINT_SPRITE_OES, GL_COORD_REPLACE_OES, GL_FALSE);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_POINT_SPRITE_OES);
}
