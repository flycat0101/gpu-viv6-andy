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
#include <stdio.h>
#include <string.h>

const char *kernelNames[] = {"Add","Mult","Copy","Divide","Clz"};
const int kernelInpurArgNums[] = {2,2,1,2,1};
const char *typeSpecifier = "%type";
const char *divideArgCastSpecifier = "%divideArgCast";
const char *clzArgCastSpecifier = "%clzArgCast";
const char *thirdDimGetGlobalIdSpecifier = "%thirdDimGetGlobalId";
const char *argArrayBracketsSpecifier = "%argArrayBrackets";
const char *thirdDimBracketSpecifier = "%thirdDimBracket";
const char *dimSpecifier = "%dim";
const char *format2D = "\
__kernel void %typeAdd%dimD(__global const %type inputA%argArrayBrackets,                        \n\
                            __global const %type inputB%argArrayBrackets,                        \n\
                            __global %type result%argArrayBrackets) {                            \n\
    const int i = get_global_id(0);                                                                \n\
    const int j = get_global_id(1);                                                                \n\
    %thirdDimGetGlobalId                                                                        \n\
    result[i][j]%thirdDimBracket = inputA[i][j]%thirdDimBracket + inputB[i][j]%thirdDimBracket;    \n\
}                                                                                                \n\
                                                                                                \n\
__kernel void %typeMult%dimD(__global const %type inputA%argArrayBrackets,                        \n\
                            __global const %type inputB%argArrayBrackets,                        \n\
                            __global %type result%argArrayBrackets) {                            \n\
    const int i = get_global_id(0);                                                                \n\
    const int j = get_global_id(1);                                                                \n\
    %thirdDimGetGlobalId                                                                        \n\
    result[i][j]%thirdDimBracket = inputA[i][j]%thirdDimBracket * inputB[i][j]%thirdDimBracket;    \n\
}                                                                                                \n\
                                                                                                \n\
__kernel void %typeCopy%dimD(__global const %type input%argArrayBrackets,                        \n\
                            __global %type result%argArrayBrackets) {                            \n\
    const int i = get_global_id(0);                                                                \n\
    const int j = get_global_id(1);                                                                \n\
    %thirdDimGetGlobalId                                                                        \n\
    result[i][j]%thirdDimBracket = input[i][j]%thirdDimBracket;                                    \n\
}                                                                                                \n\
                                                                                                \n\
__kernel void %typeDivide%dimD(__global const %type inputA%argArrayBrackets,                        \n\
                            __global const %type inputB%argArrayBrackets,                        \n\
                            __global %type result%argArrayBrackets) {                            \n\
    const int i = get_global_id(0);                                                                \n\
    const int j = get_global_id(1);                                                                \n\
    %thirdDimGetGlobalId                                                                        \n\
    result[i][j]%thirdDimBracket = inputA[i][j]%thirdDimBracket / inputB[i][j]%thirdDimBracket;    \n\
}                                                                                                \n\
                                                                                                \n\
__kernel void %typeClz%dimD(__global const %type input%argArrayBrackets,                        \n\
                            __global %type result%argArrayBrackets) {                            \n\
    const int i = get_global_id(0);                                                                \n\
    const int j = get_global_id(1);                                                                \n\
    %thirdDimGetGlobalId                                                                        \n\
    result[i][j]%thirdDimBracket = convert_%type_rtz(clz(%clzArgCast(input[i][j]%thirdDimBracket)));\n\
}                                                                                                \n\
";

void replace(const char *subString, const char *specifier, const char *inputSource, char *source, unsigned int *sourceSize);

const char* getKernelName(const unsigned int index) {
    return kernelNames[index];
}

const int getKernelInputArgNum(const unsigned int index) {
    return kernelInpurArgNums[index];
}

void generateKernels(const char *type, char *source, unsigned int *sourceSize, const unsigned int dimension, const unsigned int *dimensions) {
    int typeIndex = 0;
    char *divideArgCast = "convert_float_rtz";
    char *clzArgCast = "";
    char *thirdDimGetGlobalId = "";
    char *thirdDimBracket = "";
    char argArrayBrackets[20] = "";
    char *dim = "2";
    char tmpFormat[MAX_SOURCE_SIZE];
    int floatControl = strncmp(type, "float", 5);
    if (dimension == 3) {
        thirdDimGetGlobalId = "const int k = get_global_id(2);";
        thirdDimBracket = "[k]";
        dim = "3";
        sprintf(argArrayBrackets, "[%d][%d][%d]", dimensions[0], dimensions[1], dimensions[2]);
    } else {
        sprintf(argArrayBrackets, "[%d][%d]", dimensions[0], dimensions[1]);
    }
    if (floatControl == 0)
        clzArgCast = "convert_int_rtz";
    while (type[typeIndex] != '\0') {
        if (type[typeIndex] == '2') {
            divideArgCast = "convert_float2_rtz";
            if (floatControl == 0)
                clzArgCast = "convert_int2_rtz";
            break;
        } else if (type[typeIndex] == '4') {
            divideArgCast = "convert_float4_rtz";
            if (floatControl == 0)
                clzArgCast = "convert_int4_rtz";
            break;
        } else if (type[typeIndex] == '8') {
            divideArgCast = "convert_float8_rtz";
            if (floatControl == 0)
                clzArgCast = "convert_int8_rtz";
            break;
        } else if (type[typeIndex] == '1' && type[typeIndex+1] == '6') {
            divideArgCast = "convert_float16_rtz";
            if (floatControl == 0)
                clzArgCast = "convert_int16_rtz";
            break;
        }
        typeIndex++;
    }
    replace(type, typeSpecifier, format2D, tmpFormat, sourceSize);
    replace(divideArgCast, divideArgCastSpecifier, tmpFormat, source, sourceSize);
    strcpy(tmpFormat,source);
    replace(clzArgCast, clzArgCastSpecifier, tmpFormat, source, sourceSize);
    strcpy(tmpFormat,source);
    replace(argArrayBrackets, argArrayBracketsSpecifier, tmpFormat, source, sourceSize);
    strcpy(tmpFormat,source);
    replace(thirdDimBracket, thirdDimBracketSpecifier, tmpFormat, source, sourceSize);
    strcpy(tmpFormat,source);
    replace(thirdDimGetGlobalId, thirdDimGetGlobalIdSpecifier, tmpFormat, source, sourceSize);
    strcpy(tmpFormat,source);
    replace(dim, dimSpecifier, tmpFormat, source, sourceSize);
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