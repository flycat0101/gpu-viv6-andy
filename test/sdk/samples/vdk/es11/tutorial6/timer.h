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


#ifndef __timer_h_
#define __timer_h_

// Timer class, to control the FPS and the animations speed.
class Timer
{
public:
    Timer();

    // This method updates the timer.  This must be called once per frame.
    void UpdateTimer();

    // Returns the elapsed time between the current frame and the last frame.
    unsigned long GetTimeBetweenTwoFrames()
    {
        return m_DiffTime;
    }

    // Return the time elapsed since the creation of the timer.
    unsigned long GetTotalTime()
    {
        return m_TotalTime;
    }

    // Returns the frames per second.
    float GetFPS()
    {
        return (1000.0f * m_FrameCount) / m_TotalTime;
    }

private:
    unsigned long m_LastTime;
    unsigned long m_DiffTime;
    unsigned long m_TotalTime;
    int m_FrameCount;
};

#endif // __timer_h_
