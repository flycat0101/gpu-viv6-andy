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


#ifndef _resultchecker_h
#define _resultchecker_h

typedef enum eType {INT, UINT, CHAR, UCHAR, SHORT, USHORT, FLOAT} ElementType;

int checkAddInt(int input1, int input2, int result);
int checkMultInt(int input1, int input2, int result);
int checkCopyInt(int input1, int input2, int result);
int checkDivideInt(int input1, int input2, int result);
int checkClzInt(int input1, int input2, int result);

int checkAddUInt(unsigned int input1, unsigned int input2, unsigned int result);
int checkMultUInt(unsigned int input1, unsigned int input2, unsigned int result);
int checkCopyUInt(unsigned int input1, unsigned int input2, unsigned int result);
int checkDivideUInt(unsigned int input1, unsigned int input2, unsigned int result);
int checkClzUInt(unsigned int input1, unsigned int input2, unsigned int result);

int checkAddShort(short input1, short input2, short result);
int checkMultShort(short input1, short input2, short result);
int checkCopyShort(short input1, short input2, short result);
int checkDivideShort(short input1, short input2, short result);
int checkClzShort(short input1, short input2, short result);

int checkAddUShort(unsigned short input1, unsigned short input2, unsigned short result);
int checkMultUShort(unsigned short input1, unsigned short input2, unsigned short result);
int checkCopyUShort(unsigned short input1, unsigned short input2, unsigned short result);
int checkDivideUShort(unsigned short input1, unsigned short input2, unsigned short result);
int checkClzUShort(unsigned short input1, unsigned short input2, unsigned short result);

int checkAddChar(char input1, char input2, char result);
int checkMultChar(char input1, char input2, char result);
int checkCopyChar(char input1, char input2, char result);
int checkDivideChar(char input1, char input2, char result);
int checkClzChar(char input1, char input2, char result);

int checkAddUChar(unsigned char input1, unsigned char input2, unsigned char result);
int checkMultUChar(unsigned char input1, unsigned char input2, unsigned char result);
int checkCopyUChar(unsigned char input1, unsigned char input2, unsigned char result);
int checkDivideUChar(unsigned char input1, unsigned char input2, unsigned char result);
int checkClzUChar(unsigned char input1, unsigned char input2, unsigned char result);

int checkAddFloat(float input1, float input2, float result);
int checkMultFloat(float input1, float input2, float result);
int checkCopyFloat(float input1, float input2, float result);
int checkDivideFloat(float input1, float input2, float result);
int checkClzFloat(float input1, float input2, float result);

#endif /*_resultchecker_h*/