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


#include "main.h"
#include "timer.h"
#include <stdio.h>

#ifdef ANDROID_JNI
#include <time.h>
#endif

Timer::Timer()
{
	// We must initialize all our variables.
	m_LastTime   = 0;
	m_DiffTime   = 0;
	m_TotalTime  = 0;
	m_FrameCount = 0;
}

//-----------------------------------------------------

void Timer::UpdateTimer()
{
#ifndef ANDROID_JNI
	// Get current time.
	unsigned long currentTime = vdkGetTicks();
#else
    struct timeval tm;
    gettimeofday(&tm, NULL);
    unsigned long currentTime  = tm.tv_sec * 1000 + tm.tv_usec / 1000;
#endif

    if (m_LastTime != 0)
    {
		// Compute difference for frame.
		m_DiffTime = currentTime - m_LastTime;

		// Update total time and frame counter.
		m_TotalTime  += m_DiffTime;
		m_FrameCount ++;
	}

	// Save current time.
	m_LastTime = currentTime;
}
