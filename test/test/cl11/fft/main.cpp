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


#include "clutil.h"

int
main(
    const int argc,
    const char* argv[]
    )
{
    if (argc < 2)
    {
        printf("Usage: %s fftlen \n", argv[0]);
        return -1;
    }
    const unsigned len = atoi(argv[1]);

    if (len > FFT_MAX)
    {
        printf("FFT length cannot be greater than %d.\n", FFT_MAX);
        return -1;
    }

    if (len < 16)
    {
        printf("FFT length has to at least be 16.\n");
        return -1;
    }

    if ((len != 1) && (len & (len - 1)))
    {
        printf("FFT length (%d) must be a power-of-2.\n", len);
        return -1;
    }

    printf("Block size: %d \n", blockSize);
    printf("Print result: %s \n", print ? "yes" : "no");

    int result = 0;
    result = runFFT(len);
    if (result == 0)
    {
        printf("Successful.\n");
        if (print) printResult(len);
    }
    else
    {
        printf("Failed.\n");
    }
    cleanup();
}
