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


#if defined(LINUX)

#include <gc_sdk.h>


/*******************************************************************************
**
**    vdkGetCurrentTime
**
**    Query the current system time.
**
**    INPUT:
**
**        Nothing.
**
**    OUTPUT:
**
**        gcsTIME_PTR CurrTime
**            Points to the current system time.
**
**    RETURN:
**
**        Nothing.
*/

void
vdkGetCurrentTime(
    gcsTIME_PTR CurrTime
    )
{
    time_t now;
    struct tm * currentTime;

    /* Get current time. */
    time(&now);

    /* Convert to tm structure. */
    currentTime = localtime(&now);

    /* Translate the structure. */
    CurrTime->year    = currentTime->tm_year;
    CurrTime->month   = currentTime->tm_mon;
    CurrTime->day     = currentTime->tm_mday;
    CurrTime->weekday = currentTime->tm_wday;
    CurrTime->hour    = currentTime->tm_hour;
    CurrTime->minute  = currentTime->tm_min;
    CurrTime->second  = currentTime->tm_sec;
}

/* if defined(LINUX) */
#endif
