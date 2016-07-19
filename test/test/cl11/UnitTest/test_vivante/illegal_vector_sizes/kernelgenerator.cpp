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


#include "kernelgenerator.h"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "string.h"

const char *typeSpecifier = "%type";
const char *vectorSizeSpecifier = "%vectorSize";
const char *copyElementsSpecifier = "%copyElements";
const char *copyRestSpecifier = "%copyRest";
const char *copyPrimitiveFormat = "b.s%c = a.s%c;\n\t";
const char *baseKernelFormat1 = "\
__kernel void illegal_vector_size_k1_%type%vectorSize(__global const %type%vectorSize *input,    \n\
                                            __global %type%vectorSize *result) {                \n\
    %type%vectorSize a = input[0];                                                                \n\
    %type%vectorSize b;                                                                            \n\
    b.s%copyElements = a.s%copyElements;                                                        \n\
    %copyRest                                                                                    \n\
    result[0] = b;                                                                                \n\
}                                                                                                \n\
";
const char *baseKernelFormat2 = "\
__kernel void illegal_vector_size_k2_%type%vectorSize(__global const %type%vectorSize *input,    \n\
                                            __global %type%vectorSize *result) {                \n\
    %type%vectorSize a = input[0];                                                                \n\
    %type%vectorSize b;                                                                            \n\
    b = (%type%vectorSize)(a.s%copyElements, a.s%copyRest);                                        \n\
    result[0] = b;                                                                                \n\
}                                                                                                \n\
";

void replace(const char *subString, const char *specifier, const char *inputSource, char *source, unsigned int *sourceSize);
char numToHexChar(const unsigned int number);

void generateKernel(char *source, unsigned int *sourceSize, const char *type, const unsigned int vectorSize, const unsigned int illegalVectorSize, const int kernelType, unsigned int *key) {
    char sVectorSize[3];
    char copyPrimitives[400];
    char copyElements[20];
    char tmpFormat[5000];
    unsigned int i,j;
    char tmpStr[40];
    char baseKernelFormat[1000];
    if (kernelType == 1) {
        strcpy(baseKernelFormat,baseKernelFormat1);
        copyPrimitives[0] = '\0';
        for (i=illegalVectorSize; i<vectorSize; i++) {
            key[i] = i;
            sprintf(tmpStr, copyPrimitiveFormat, numToHexChar(i), numToHexChar(i));
            strcat(copyPrimitives, tmpStr);
        }
    } else if (kernelType == 2) {
        strcpy(baseKernelFormat,baseKernelFormat2);
        j = 0;
        for (i=illegalVectorSize; i<vectorSize; i++) {
            key[i] = i;
            copyPrimitives[j++] = numToHexChar(i);
        }
        copyPrimitives[j] = '\0';
    } else
        return;

    for (i=0; i<illegalVectorSize; i++) {
        key[i] = i;
        copyElements[i] = numToHexChar(i);
    }
    copyElements[i] = '\0';

    sprintf(sVectorSize,"%u",vectorSize);

    replace(type, typeSpecifier, baseKernelFormat, source, sourceSize);
    replace(sVectorSize, vectorSizeSpecifier, source, tmpFormat, sourceSize);
    strcpy(source, tmpFormat);
    replace(copyElements, copyElementsSpecifier, source, tmpFormat, sourceSize);
    strcpy(source, tmpFormat);
    replace(copyPrimitives, copyRestSpecifier, source, tmpFormat, sourceSize);
    strcpy(source, tmpFormat);
}

void replace(const char *subString, const char *specifier, const char *inputSource, char *source, unsigned int *sourceSize) {
    char ch;
    unsigned int chIndex = 0;
    unsigned int sourceIndex = 0;
    unsigned int chSpecIndex;
    unsigned int chTypeIndex;
    int state;
    while ((ch=inputSource[chIndex]) != '\0') {
        if (ch == specifier[0]) {
            chSpecIndex = 1;
            state = 0;
            while (state == 0) {
                if (specifier[chSpecIndex] == '\0') {
                    state = 1;
                } else if (specifier[chSpecIndex] != inputSource[chIndex+chSpecIndex]) {
                    state = 2;
                }
                chSpecIndex++;
            }
            if (state == 1) {
                chTypeIndex = 0;
                while (subString[chTypeIndex] != '\0') {
                    source[sourceIndex] = subString[chTypeIndex];
                    chTypeIndex++;
                    sourceIndex++;
                }
                chIndex += chSpecIndex-1;
            } else if (state == 2) {
                source[sourceIndex] = ch;
                chIndex++;
                sourceIndex++;
            }
        } else {
            source[sourceIndex] = ch;
            chIndex++;
            sourceIndex++;
        }
    }
    source[sourceIndex] = '\0';
    *sourceSize = sourceIndex;
}

char numToHexChar(const unsigned int number) {
    switch (number) {
        case 0:
            return '0';
                break;
        case 1:
            return '1';
            break;
        case 2:
            return '2';
            break;
        case 3:
            return '3';
            break;
        case 4:
            return '4';
            break;
        case 5:
            return '5';
            break;
        case 6:
            return '6';
            break;
        case 7:
            return '7';
            break;
        case 8:
            return '8';
            break;
        case 9:
            return '9';
            break;
        case 10:
            return 'a';
            break;
        case 11:
            return 'b';
            break;
        case 12:
            return 'c';
            break;
        case 13:
            return 'd';
            break;
        case 14:
            return 'e';
            break;
        case 15:
            return 'f';
            break;
        default:
            return -1;
            break;
    }
    return -1;
}