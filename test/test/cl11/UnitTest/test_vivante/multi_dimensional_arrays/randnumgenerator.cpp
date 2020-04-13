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


#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <climits>
#include "randnumgenerator.h"
#if defined(__QNXNTO__)
#include <time.h>
#endif

void setRand() {
    srand(static_cast<unsigned int>(time(NULL)));
}

int randInt() {
    return static_cast<int>(rand()%9+1);
}

unsigned int randUInt() {
    return static_cast<unsigned int>(rand()%9+1);
}

short randShort() {
    return static_cast<short>(rand()%5+1);
}

unsigned short randUShort() {
    return static_cast<unsigned short>(rand()%5+1);
}

char randChar() {
    return static_cast<char>(rand()%3+1);
}

unsigned char randUChar() {
    return static_cast<unsigned char>(rand()%3+1);
}

float randFloat() {
    return static_cast<float>(rand()%9+1)/static_cast<float>(rand()%9+1);
}
