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


#include <math.h>
#include <stdio.h>
#include "resultchecker.h"

//INT

int checkAddInt(int input1, int input2, int result) {
    int checkResult = input1 + input2;
    if (checkResult != result) {
        printf("%d + %d returns %d instead of %d\n",input1,input2,result,checkResult);
        return 1;
    }
    return 0;
}

int checkMultInt(int input1, int input2, int result) {
    int checkResult = input1 * input2;
    if (checkResult != result) {
        printf("%d * %d returns %d instead of %d\n",input1,input2,result,checkResult);
        return 1;
    }
    return 0;
}

int checkCopyInt(int input1, int input2, int result) {
    if (input1 != result) {
        printf("%d is not equal to %d\n",input1,result);
        return 1;
    }
    return 0;
}

int checkDivideInt(int input1, int input2, int result) {
    int checkResult = (input2==0) ? 0 : input1/input2;
    if (checkResult != result) {
        printf("%d / %d returns %d instead of %d\n",input1,input2,result,checkResult);
        return 1;
    }
    return 0;
}

int checkClzInt(int input1, int input2, int result) {
    unsigned int checkResult = sizeof(int)*8;
    while (input1) {
        input1 >>= 1;
        checkResult--;
    }
    if (checkResult != result) {
        printf("%Clz of %d returns %d instead of %u\n",input1,result,checkResult);
        return 1;
    }
    return 0;
}

//UINT

int checkAddUInt(unsigned int input1, unsigned int input2, unsigned int result) {
    unsigned int checkResult = input1 + input2;
    if (checkResult != result) {
        printf("%u + %u returns %u instead of %u\n",input1,input2,result,checkResult);
        return 1;
    }
    return 0;
}

int checkMultUInt(unsigned int input1, unsigned int input2, unsigned int result) {
    unsigned int checkResult = input1 * input2;
    if (checkResult != result) {
        printf("%u * %u returns %u instead of %u\n",input1,input2,result,checkResult);
        return 1;
    }
    return 0;
}

int checkCopyUInt(unsigned int input1, unsigned int input2, unsigned int result) {
    if (input1 != result) {
        printf("%u is not equal to %u\n",input1,result);
        return 1;
    }
    return 0;
}

int checkDivideUInt(unsigned int input1, unsigned int input2, unsigned int result) {
    unsigned int checkResult = (input2==0) ? input1 : input1/input2;
    if (checkResult != result) {
        printf("%u / %u returns %u instead of %u\n",input1,input2,result,checkResult);
        return 1;
    }
    return 0;
}

int checkClzUInt(unsigned int input1, unsigned int input2, unsigned int result) {
    unsigned int checkResult = sizeof(unsigned int)*8;
    while (input1) {
        input1 >>= 1;
        checkResult--;
    }
    if (checkResult != result) {
        printf("%Clz of %u returns %u instead of %u\n",input1,result,checkResult);
        return 1;
    }
    return 0;
}

//SHORT

int checkAddShort(short input1, short input2, short result) {
    short checkResult = input1 + input2;
    if (checkResult != result) {
        printf("%d + %d returns %d instead of %d\n",input1,input2,result,checkResult);
        return 1;
    }
    return 0;
}

int checkMultShort(short input1, short input2, short result) {
    short checkResult = input1 * input2;
    if (checkResult != result) {
        printf("%d * %d returns %d instead of %d\n",input1,input2,result,checkResult);
        return 1;
    }
    return 0;
}

int checkCopyShort(short input1, short input2, short result) {
    if (input1 != result) {
        printf("%d is not equal to %d\n",input1,result);
        return 1;
    }
    return 0;
}

int checkDivideShort(short input1, short input2, short result) {
    short checkResult = (input2==0) ? 0 : input1/input2;
    if (checkResult != result) {
        printf("%d / %d returns %d instead of %d\n",input1,input2,result,checkResult);
        return 1;
    }
    return 0;
}

int checkClzShort(short input1, short input2, short result) {
    unsigned int checkResult = sizeof(short)*8;
    while (input1) {
        input1 >>= 1;
        checkResult--;
    }
    if (checkResult != result) {
        printf("%Clz of %d returns %d instead of %u\n",input1,result,checkResult);
        return 1;
    }
    return 0;
}

//USHORT

int checkAddUShort(unsigned short input1, unsigned short input2, unsigned short result) {
    unsigned short checkResult = input1 + input2;
    if (checkResult != result) {
        printf("%u + %u returns %u instead of %u\n",input1,input2,result,checkResult);
        return 1;
    }
    return 0;
}

int checkMultUShort(unsigned short input1, unsigned short input2, unsigned short result) {
    unsigned short checkResult = input1 * input2;
    if (checkResult != result) {
        printf("%u * %u returns %u instead of %u\n",input1,input2,result,checkResult);
        return 1;
    }
    return 0;
}

int checkCopyUShort(unsigned short input1, unsigned short input2, unsigned short result) {
    if (input1 != result) {
        printf("%u is not equal to %u\n",input1,result);
        return 1;
    }
    return 0;
}

int checkDivideUShort(unsigned short input1, unsigned short input2, unsigned short result) {
    unsigned short checkResult = (input2==0) ? 0 : input1/input2;
    if (checkResult != result) {
        printf("%u / %u returns %u instead of %u\n",input1,input2,result,checkResult);
        return 1;
    }
    return 0;
}

int checkClzUShort(unsigned short input1, unsigned short input2, unsigned short result) {
    unsigned int checkResult = sizeof(unsigned short)*8;
    while (input1) {
        input1 >>= 1;
        checkResult--;
    }
    if (checkResult != result) {
        printf("%Clz of %u returns %u instead of %u\n",input1,result,checkResult);
        return 1;
    }
    return 0;
}

//CHAR

int checkAddChar(char input1, char input2, char result) {
    char checkResult = input1 + input2;
    if (checkResult != result) {
        printf("%d + %d returns %d instead of %d\n",(int)input1,(int)input2,(int)result,(int)checkResult);
        return 1;
    }
    return 0;
}

int checkMultChar(char input1, char input2, char result) {
    char checkResult = input1 * input2;
    if (checkResult != result) {
        printf("%d * %d returns %d instead of %d\n",(int)input1,(int)input2,(int)result,(int)checkResult);
        return 1;
    }
    return 0;
}

int checkCopyChar(char input1, char input2, char result) {
    if (input1 != result) {
        printf("%d is not equal to %d\n",(int)input1,(int)result);
        return 1;
    }
    return 0;
}

int checkDivideChar(char input1, char input2, char result) {
    short checkResult = (input2==0) ? 0 : input1/input2;
    if (checkResult != result) {
        printf("%d / %d returns %d instead of %d\n",(int)input1,(int)input2,(int)result,(int)checkResult);
        return 1;
    }
    return 0;
}

int checkClzChar(char input1, char input2, char result) {
    unsigned int checkResult = sizeof(char)*8;
    while (input1) {
        input1 >>= 1;
        checkResult--;
    }
    if (checkResult != result) {
        printf("%Clz of %d returns %d instead of %u\n",(int)input1,(int)result,(int)checkResult);
        return 1;
    }
    return 0;
}

//UCHAR

int checkAddUChar(unsigned char input1, unsigned char input2, unsigned char result) {
    unsigned char checkResult = input1 + input2;
    if (checkResult != result) {
        printf("%u + %u returns %u instead of %u\n",(int)input1,(int)input2,(int)result,(int)checkResult);
        return 1;
    }
    return 0;
}

int checkMultUChar(unsigned char input1, unsigned char input2, unsigned char result) {
    unsigned char checkResult = input1 * input2;
    if (checkResult != result) {
        printf("%u * %u returns %u instead of %u\n",(int)input1,(int)input2,(int)result,(int)checkResult);
        return 1;
    }
    return 0;
}

int checkCopyUChar(unsigned char input1, unsigned char input2, unsigned char result) {
    if (input1 != result) {
        printf("%u is not equal to %u\n",(int)input1,(int)result);
        return 1;
    }
    return 0;
}

int checkDivideUChar(unsigned char input1, unsigned char input2, unsigned char result) {
    unsigned char checkResult = (input2==0) ? 0 : input1/input2;
    if (checkResult != result) {
        printf("%u / %u returns %u instead of %u\n",(int)input1,(int)input2,(int)result,(int)checkResult);
        return 1;
    }
    return 0;
}

int checkClzUChar(unsigned char input1, unsigned char input2, unsigned char result) {
    unsigned int checkResult = sizeof(unsigned char)*8;
    while (input1) {
        input1 >>= 1;
        checkResult--;
    }
    if (checkResult != result) {
        printf("%Clz of %u returns %u instead of %u\n",(int)input1,(int)result,(int)checkResult);
        return 1;
    }
    return 0;
}

//FLOAT

/*int compareFloat(const float &val1, const float &val2) {
    const float dist = abs(val1-val2);
    if (dist < 0.00001)
        return 0;
    else
        return 1;
}*/

int checkAddFloat(float input1, float input2, float result) {
    float checkResult = input1 + input2;
    if (checkResult != result)/*(compareFloat(checkResult, result))*/ {
        printf("%f + %f returns %f instead of %f\n",input1,input2,result,checkResult);
        printf("HEX: 0x%08x + 0x%08x returns 0x%08x instead of 0x%08x\n",*(unsigned int *)&input1,*(unsigned int *)&input2,*(unsigned int *)&result,*(unsigned int *)&checkResult);
        return 1;
    }
    return 0;
}

int checkMultFloat(float input1, float input2, float result) {
    float checkResult = input1 * input2;
    if (checkResult != result)/*(compareFloat(checkResult, result))*/ {
        printf("%f * %f returns %f instead of %f\n",input1,input2,result,checkResult);
        printf("HEX: 0x%08x * 0x%08x returns 0x%08x instead of 0x%08x\n",*(unsigned int *)&input1,*(unsigned int *)&input2,*(unsigned int *)&result,*(unsigned int *)&checkResult);
        return 1;
    }
    return 0;
}

int checkCopyFloat(float input1, float input2, float result) {
    if (input1 != result) {
        printf("%f is not equal to %f\n",input1,result);
        printf("HEX: 0x%08x is not equal to 0x%08x\n",*(unsigned int *)&input1,*(unsigned int *)&result);
        return 1;
    }
    return 0;
}

int checkDivideFloat(float input1, float input2, float result) {
    float checkResult = input1/input2;
    unsigned int checkResult2 = (*(unsigned int *)&checkResult + 3);
    unsigned int checkResult3 = (*(unsigned int *)&checkResult - 3);
    float checkResult4 = *( float *) &checkResult2;
    float checkResult5 = *( float *) &checkResult3;
    if (result > checkResult4 || result < checkResult5){
        printf("%f / %f returns %f instead of %f\n",input1,input2,result,checkResult);
        printf("HEX: 0x%08x / 0x%08x returns 0x%08x instead of 0x%08x\n",*(unsigned int *)&input1,*(unsigned int *)&input2,*(unsigned int *)&result,*(unsigned int *)&checkResult);
        return 1;
    }
    return 0;
}

int checkClzFloat(float input1, float input2, float result) {
    int input = static_cast<int>(input1);
    unsigned int checkResult = sizeof(int)*8;
    while (input) {
        input >>= 1;
        checkResult--;
    }
    if (checkResult != result) {
        printf("%Clz of %d(converted to int) returns %d instead of %u\n",input1,result,checkResult);
        return 1;
    }
    return 0;
}