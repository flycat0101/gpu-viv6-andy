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


#include "vectorgenerator.h"
#include "testcasesinit.h"
#include "stdio.h"
#include "limits.h"
#include "stdlib.h"
#include "float.h"
#include "math.h"
#include "time.h"

unsigned int countOfTestsPerType = 0;
unsigned int currentVectorTypeIndex = 0;

float generateRandomFloatBetweenMinusOneAndOne(){
	float number = ((float)rand()) / ((float)(RAND_MAX))  ;
	if( ( (float)rand() / ( (float)(RAND_MAX) ) ) > 0.5 )
		number *= -1;
	//printf("\nGenerated number = %f \n", number);
	return number;
}

float generateRandomFloatBetweenZeroAndOne(){
	return ((float)rand()) / ((float)(RAND_MAX));
}

int            randomInt(){ return (int)(generateRandomFloatBetweenMinusOneAndOne() * (float)INT_MAX ); }
unsigned int   randomUint(){ return (unsigned int)(generateRandomFloatBetweenZeroAndOne() * (float)UINT_MAX); }
char           randomChar(){ return (char)(generateRandomFloatBetweenMinusOneAndOne() * (float)CHAR_MAX); }
unsigned char  randomUchar(){ return (unsigned char)(generateRandomFloatBetweenZeroAndOne() * (float)UCHAR_MAX); }
short          randomShort(){ return (short)(generateRandomFloatBetweenMinusOneAndOne() * (float)SHRT_MAX); }
unsigned short randomUshort(){ return (unsigned short)(generateRandomFloatBetweenZeroAndOne() * (float)USHRT_MAX); }
long           randomLong(){ return (long)(generateRandomFloatBetweenMinusOneAndOne() * (float)LONG_MAX); }
unsigned long  randomUlong(){ return (unsigned long)(generateRandomFloatBetweenZeroAndOne() * (float)ULONG_MAX); }
float          randomFloat(){ return generateRandomFloatBetweenMinusOneAndOne() * (float)FLT_MAX ; }

void* generateInt8Vector(){
	int i;
	//printf("Generating int 8 vector");
	cl_int8* aptrToTheVector = (cl_int8*)malloc(sizeof(cl_int8));
	for( i = 0 ; i<8 ; i++ ){
		aptrToTheVector->s[i] = randomInt();
		//printf("%d->%d\n",i,aptrToTheVector->s[i]);
	}
	return aptrToTheVector;
}
void* generateInt16Vector(){
	int i;
	//printf("Generating int 16 vector");
	cl_int16* aptrToTheVector = (cl_int16*)malloc(sizeof(cl_int16));
	for( i = 0 ; i<16 ; i++ ){ aptrToTheVector->s[i] = randomInt();	}
	return aptrToTheVector;
}
void* generateUint8Vector(){
	int i;
	//printf("Generating uint 8 vector\n");
	cl_uint8* aptrToTheVector = (cl_uint8*)malloc(sizeof(cl_uint8));
	for( i = 0 ; i<8 ; i++ ){ aptrToTheVector->s[i] = randomUint(); }
	return aptrToTheVector;
}
void* generateUint16Vector(){
	int i;
	//printf("Generating uint 16 vector\n");
	cl_uint16* aptrToTheVector = (cl_uint16*)malloc(sizeof(cl_uint16));
	for( i = 0 ; i<16 ; i++ ){ aptrToTheVector->s[i] = randomUint();	}
	return aptrToTheVector;
}
void* generateChar8Vector(){
	int i;
	//printf("Generating char 8 vector");
	cl_char8* aptrToTheVector = (cl_char8*)malloc(sizeof(cl_char8));
	for( i = 0 ; i<8 ; i++ ){ aptrToTheVector->s[i] = randomChar(); }
	return aptrToTheVector;
}
void* generateChar16Vector(){
	int i;
	//printf("Generating char 16 vector\n");
	cl_char16* aptrToTheVector = (cl_char16*)malloc(sizeof(cl_char16));
	for( i = 0 ; i<16 ; i++ ){ aptrToTheVector->s[i] = randomChar(); }
	return aptrToTheVector;
}
void* generateUchar8Vector(){
	int i;
	//printf("Generating uchar 8 vector\n");
	cl_uchar8* aptrToTheVector = (cl_uchar8*)malloc(sizeof(cl_uchar8));
	for( i = 0 ; i<8 ; i++ ){ aptrToTheVector->s[i] = randomUchar(); }
	return aptrToTheVector;
}
void* generateUchar16Vector(){
	int i;
	//printf("Generating uchar 16 vector\n");
	cl_uchar16* aptrToTheVector = (cl_uchar16*)malloc(sizeof(cl_uchar16));
	for( i = 0 ; i<16 ; i++ ){ aptrToTheVector->s[i] = randomUchar();	}
	return aptrToTheVector;
}
void* generateShort8Vector(){
	int i;
	//printf("Generating short 8 vector\n");
	cl_short8* aptrToTheVector = (cl_short8*)malloc(sizeof(cl_short8));
	for( i = 0 ; i<8 ; i++ ){ aptrToTheVector->s[i] = randomShort(); }
	printf("\n");
	return aptrToTheVector;
}
void* generateShort16Vector(){
	int i;
	//printf("Generating char 16 vector\n");
	cl_short16* aptrToTheVector = (cl_short16*)malloc(sizeof(cl_short16));
	for( i = 0 ; i<16 ; i++ ){ aptrToTheVector->s[i] = randomShort(); }
	return aptrToTheVector;
}
void* generateUshort8Vector(){
	int i;
	//printf("Generating ushort 8 vector\n");
	cl_ushort8* aptrToTheVector = (cl_ushort8*)malloc(sizeof(cl_ushort8));
	for( i = 0 ; i<8 ; i++ ){ aptrToTheVector->s[i] = randomUshort(); }
	return aptrToTheVector;
}
void* generateUshort16Vector(){
	int i;
	//printf("Generating ushort 16 vector\n");
	cl_ushort16* aptrToTheVector = (cl_ushort16*)malloc(sizeof(cl_ushort16));
	for( i = 0 ; i<16 ; i++ ){ aptrToTheVector->s[i] = randomUshort();	}
	return aptrToTheVector;
}
void* generateFloat8Vector(){
	int i;
	//printf("Generating float 8 vector\n");
	cl_float8* aptrToTheVector = (cl_float8*)malloc(sizeof(cl_float8));
	for( i = 0 ; i<8 ; i++ ){ aptrToTheVector->s[i] = randomFloat(); }
	return aptrToTheVector;
}
void* generateFloat16Vector(){
	int i;
	//printf("Generating float 16 vector\n");
	cl_float16* aptrToTheVector = (cl_float16*)malloc(sizeof(cl_float16));
	for( i = 0 ; i<16 ; i++ ){ aptrToTheVector->s[i] = randomFloat(); }
	return aptrToTheVector;
}

// !!!! This array should be in the same type order as typeNames
void* (*vectorGenerateFunctions[TYPE_COUNT][VECTOR_SIZE_COUNT]) () = {
	{ generateInt8Vector , generateInt16Vector },
	{ generateUint8Vector , generateUint16Vector },
	{ generateShort8Vector , generateShort16Vector },
	{ generateUshort8Vector , generateUshort16Vector },
	{ generateChar8Vector , generateChar16Vector },
	{ generateUchar8Vector , generateUchar16Vector },
	{ generateFloat8Vector , generateFloat16Vector }
};

unsigned int vectorCumilativeCaseSizes[VECTOR_SIZE_COUNT]; // { 3 , 13 }

void* getInt8Array(void *vector) {return ((cl_int8*)vector)->s;}
void* getInt16Array(void *vector) {return ((cl_int16*)vector)->s;}
void* getUint8Array(void *vector) {return ((cl_uint8*)vector)->s;}
void* getUint16Array(void *vector) {return ((cl_uint16*)vector)->s;}
void* getShort8Array(void *vector) {return ((cl_short8*)vector)->s;}
void* getShort16Array(void *vector) {return ((cl_short16*)vector)->s;}
void* getUshort8Array(void *vector) {return ((cl_ushort8*)vector)->s;}
void* getUshort16Array(void *vector) {return ((cl_ushort16*)vector)->s;}
void* getChar8Array(void *vector) {return ((cl_char8*)vector)->s;}
void* getChar16Array(void *vector) {return ((cl_char16*)vector)->s;}
void* getUchar8Array(void *vector) {return ((cl_uchar8*)vector)->s;}
void* getUchar16Array(void *vector) {return ((cl_uchar16*)vector)->s;}
void* getFloat8Array(void *vector) {return ((cl_float8*)vector)->s;}
void* getFloat16Array(void *vector) {return ((cl_float16*)vector)->s;}

void* (*vectorArrayFunctions[TYPE_COUNT][VECTOR_SIZE_COUNT]) (void*) = {
	{ getInt8Array , getInt16Array },
	{ getUint8Array , getUint16Array },
	{ getShort8Array , getShort16Array },
	{ getUshort8Array , getUshort16Array },
	{ getChar8Array , getChar16Array },
	{ getUchar8Array , getUchar16Array },
	{ getFloat8Array , getFloat16Array }
};

int getVectorSizeIndex(int progress){
	unsigned int illegalSizeToTestIndex = 0 , i = 0 , q = 0 , vectorSizeIndex = 0;
	if( progress ){
		illegalSizeToTestIndex = currentTest % countOfTestsPerType;
		if( currentTest>0 && illegalSizeToTestIndex == 0 ){	// Progress on data type arrays
			currentVectorTypeIndex++;
		}
	}
	for( i = 0 ; i < VECTOR_SIZE_COUNT ; i++ ) { // Calculate vector size array index
		if( vectorCumilativeCaseSizes[i] <= illegalSizeToTestIndex ){
			vectorSizeIndex = 0 ;
			unsigned int currentVectorSizeTestCount = illegalSizeToTestIndex - vectorCumilativeCaseSizes[i];
			for( q = 0 ; q < VECTOR_SIZE_COUNT ; q++ ){
				if( currentVectorSizeTestCount >= vectorCumilativeCaseSizes[q] )
					vectorSizeIndex++;
			}
		}
	}
	return vectorSizeIndex;
}

void* generateVector(){
	//return vectorGenerateFunctions[currentVectorTypeIndex][getVectorSizeIndex(1)]();
	vectorGenerateFunctions[currentVectorTypeIndex][getVectorSizeIndex(1)]();
	return vectorGenerateFunctions[vecTypeIndex[currentTest]][vecSizeIndex[currentTest]]();
}

int compare(void* inputVector, void* outputVector, unsigned int* key){
	int vectorSizeIndex = getVectorSizeIndex(0) , // To determine vector size of input and output vectors
		equals = 1 , // to return
		vectorByteCount = 0 , // how many bytes are there in the vector
		bytesPerLiteral = 0 , // how many byte(s) are there in one literal ( ex: v.s0 )
		i = 0 , // for loops
		q = 0 , // for loops
		vectorSize = vectorSizes[vecSizeIndex[currentTest]]; // actual vector size ( 8 , 16 etc. )
	/* Test amaçlý yazýlmýþtýr
	printf("COMPARING: %d with %d",((cl_int8*)inputVector)->s[0],((cl_int8*)outputVector)->s[0]);
	*/
	if( sizeof(size_t) != sizeof(int) )
		printf("FATAL ERROR , sizeof(size_t) is not equal to sizeof(int)");
	vectorByteCount = (int)memSizes[vecTypeIndex[currentTest]*VECTOR_SIZE_COUNT+vecSizeIndex[currentTest]];
	bytesPerLiteral = vectorByteCount / vectorSize;
	char* input = (char*)vectorArrayFunctions[vecTypeIndex[currentTest]][vecSizeIndex[currentTest]](inputVector);
	char* output = (char*)vectorArrayFunctions[vecTypeIndex[currentTest]][vecSizeIndex[currentTest]](outputVector);
	//key = (unsigned int*)key;
	for( i = 0 ; i < vectorSize ; i ++ ){ // we will compare byte by byte based on the key
		for( q = 0 ; q < bytesPerLiteral ; q++ ){
			if( input[key[i]*bytesPerLiteral+q] != output[i*bytesPerLiteral+q] ) {
				equals = 0;
				break;
			}
		}
		if (equals == 0)
			break;
	}
	return equals;
}