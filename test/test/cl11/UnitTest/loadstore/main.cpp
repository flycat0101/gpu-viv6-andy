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


#include "../common.h"

const char * testName = "LoadStore";

/******************************************************************************\
|* CL programs                                                                *|
\******************************************************************************/
const char * programSources[] =
{
/* test case 0 */
    "\
__kernel void loadstore( \
    __global float *pdst, \
    __global float *psrc, \
    float offset \
    ) \
{ \
    *pdst = *psrc + offset; \
}",

/* test case 1 */
    "\
__kernel void loadstore( \
    __global unsigned int *pdst, \
    __global unsigned int *psrc, \
    uint offset \
    ) \
{ \
    *pdst = *psrc + offset; \
}",

/* test case 2 */
    "\
__kernel void loadstore( \
    __global unsigned int *pdst \
    ) \
{ \
    *pdst = 5; \
}",

/* test case 3 */
    "\
char shuffle_fn( char source ); \
char shuffle_fn( char source ) { return source; } \
__kernel void loadstore( __global char *pdst, __global char *psrc ) \
{ \
    if (get_global_id(0) != 0) return; \
  char tmp; \
  tmp = (char)((char)0); \
  tmp = shuffle_fn( psrc[0] ); \
  pdst[0] = tmp; \
  tmp = (char)((char)0); \
  tmp = shuffle_fn( psrc[1] ); \
  pdst[1] = tmp; \
}",

/* test case 4 */
    "\
__kernel void loadstore( \
    __global float *pdst, \
    __global float *psrc, \
    float offset \
    ) \
{ \
   *((float8 *)pdst) = *((float8 *)psrc) + offset; \
}",

/* test case 5 */
    "\
__kernel void loadstore( \
    __global float8 *pdst, \
    __global float8 *psrc \
    ) \
{ \
   *pdst = *psrc; \
}",

/* test case 6 */
    "\
__kernel void loadstore( \
    __global float *pdst \
    ) \
{ \
    *((float8 *)pdst) = 5.0; \
}",

/* test case 7 */
    "\
__kernel void loadstore( \
    __global char *pdst, \
    __global char *psrc, \
    char offset \
    ) \
{ \
   *((char16 *)pdst) = *((char16 *)psrc) + offset; \
}",

/* test case 8 */
    "\
__kernel void loadstore( \
    __global float *pdst, \
    __global float *psrc \
    ) \
{ \
   *((float8 *)pdst) = *((float8 *)psrc) + 5; \
}",

/* test case 9 */
    "\
__kernel void loadstore( \
    __global float *pdst, \
    __global float *psrc \
    ) \
{ \
   float2 *ptr; \
   int i; \
   ptr = (float2 *) pdst; \
   for(i = 0; i < 4; i++) { \
      *ptr++ = *(((float2 *) psrc) + i) + 5; \
   } \
}",

/* test case 10 */
    "\
__kernel void loadstore( \
    __global float *pdst, \
    __global float *psrc \
    ) \
{ \
   float8 *ptr; \
   int i; \
   ptr = (float8 *) pdst; \
   for(i = 0; i < 2; i++) { \
      *ptr++ = *(((float8 *) psrc) + i) + 5; \
   } \
}",

/* test case 11 */
    "\
__kernel void loadstore( \
    __global float4 *pdst, \
    __global float4 *psrc \
    ) \
{ \
    float16 x = (float16) (psrc[0], psrc[1], psrc[2], psrc[3]); \
    float16 t; \
    t.even = x.lo; \
    t.odd = x.hi; \
    x.even = t.lo; \
    x.odd = t.hi; \
    pdst[0] = x.lo.lo;    \
    pdst[1] = x.lo.hi;    \
    pdst[2] = x.hi.lo;    \
    pdst[3] = x.hi.hi;    \
}",

/* test case 12 */
    "\
__kernel void loadstore( \
    __global float *pdst, \
    __global float *psrc \
    ) \
{ \
   float2 *ptr; \
   int i; \
   ptr = (float2 *) psrc; \
   for(i = 0; i < 4; i++) { \
      *(((float2 *) pdst) + i)  = *ptr++ + 5; \
   } \
}",

/* test case 13 */
    "\
__kernel void loadstore( \
    __global float *pdst, \
    __global float *psrc \
    ) \
{ \
   void *voidPtr = (void *) 0; \
   float2 *ptr; \
   int i; \
   voidPtr = (void *)pdst; \
   ptr = (float2 *) psrc; \
   for(i = 0; i < 4; i++) { \
      *(((float2 *) voidPtr) + i)  = *ptr++ + 5; \
   } \
}",

/* test case 14 */
    "\
__kernel void loadstore( \
    __global float *pdst, \
    __global float *psrc, \
    float offset \
    ) \
{ \
   (*((float8 *)pdst)).s76543210 = *((float8 *)psrc) + offset; \
}",

/* test case 15 */
    "\
__kernel void loadstore( \
    __global float *pdst, \
    __global float *psrc, \
    float offset \
    ) \
{ \
   *((float8 *)pdst) = (*((float8 *)psrc)).s76543210 + offset; \
}",

/* test case 16 */
    "\
__kernel void loadstore( \
    __global float *pdst, \
    __global float *psrc, \
    float offset \
    ) \
{ \
   (*((float8 *)pdst)).s06512347 = (*((float8 *)psrc)).s76543210 + offset; \
}",

/* test case 17 */
    "\
__kernel void loadstore( \
    __global float *pdst, \
    __global float *psrc, \
    float offset \
    ) \
{ \
   (*((float8 *)pdst)).s76543210 = (*((float8 *)psrc)).s06512347 + offset; \
}",

/* test case 18 */
    "\
__kernel void loadstore( \
    __global float *pdst, \
    __global float *psrc, \
    float offset \
    ) \
{ \
   (*((float16 *)pdst)).s76543210 = (*((float16 *)psrc)).s065189AB + offset; \
}",

/* test case 19 */
    "\
__kernel void loadstore( \
    __global char *pdst \
    ) \
{ \
    float value = 5.0f; \
    *pdst = value; \
}",

/* test case 20 */
    "\
    typedef struct _my_struct { \
        int a; \
        char b; \
        } my_struct; \
__kernel void loadstore( \
    __global my_struct *pdst \
    ) \
{ \
    int value = 2; \
    char cvalue = '2'; \
    pdst->a = value; \
    pdst->b = cvalue; \
}",

/* test case 21 */
    "\
    typedef struct _my_struct { \
        int a; \
        char b; \
        } my_struct; \
__kernel void loadstore( \
    my_struct src, \
    __global my_struct *pdst \
    ) \
{ \
    pdst->a = src.a; \
    pdst->b = src.b; \
}",

/* test case 22 */
    "\
__kernel void loadstore( \
    __local int src[4], \
    __global int *pdst \
    ) \
{ \
    pdst[0] = src[0]; \
    pdst[1] = src[1]; \
    pdst[2] = src[2]; \
    pdst[3] = src[3]; \
}",

/* test case 23 */
    "\
__kernel void loadstore( \
    __local int src[2][2], \
    __global int pdst[2][2] \
    ) \
{ \
    pdst[0][0] = src[0][0]; \
    pdst[0][1] = src[0][1]; \
    pdst[1][0] = src[1][0]; \
    pdst[1][1] = src[1][1]; \
}",

/* test case 24 */
    "\
__kernel void loadstore( \
    __global int *src, \
    __global int pdst[2][2] \
    ) \
{ \
    __local int arr[2][2]; \
    arr[0][0] = src[0] * 2; \
    arr[0][1] = src[1] * 2; \
    arr[1][0] = src[2] * 2; \
    arr[1][1] = src[3] * 2; \
\
    pdst[0][0] = arr[0][0] / 2; \
    pdst[0][1] = arr[0][1] / 2; \
    pdst[1][0] = arr[1][0] / 2; \
    pdst[1][1] = arr[1][1] / 2; \
}",

/* test case 25 */
    "\
__kernel void loadstore( \
    __global char *src, \
    __global char *dest \
    ) \
{ \
  char8 tmp; \
  tmp = (char8)((char)0); \
     tmp.s5 = ((__global char16 *)src)[0].s0; \
     ((__global char8 *)dest)[0] = tmp; \
  tmp = (char8)((char)0); \
     tmp.S4 = ((__global char16 *)src)[0].S1; \
     ((__global char8 *)dest)[1] = tmp; \
  tmp = (char8)((char)0); \
     tmp.s0 = ((__global char16 *)src)[0].S2; \
     ((__global char8 *)dest)[2] = tmp; \
  tmp = (char8)((char)0); \
     tmp.s2 = ((__global char16 *)src)[0].S3; \
     ((__global char8 *)dest)[3] = tmp; \
  tmp = (char8)((char)0); \
     tmp.s3 = ((__global char16 *)src)[1].S1; \
     tmp.s1 = ((__global char16 *)src)[1].S1; \
     ((__global char8 *)dest)[4] = tmp; \
  tmp = (char8)((char)0); \
     tmp.s3 = ((__global char16 *)src)[1].S1; \
     tmp.s2 = ((__global char16 *)src)[1].S1; \
     ((__global char8 *)dest)[5] = tmp; \
}",

};

cl_int numPrograms = sizeof(programSources) / sizeof(programSources[0]);

/******************************************************************************\
|* Test function                                                              *|
\******************************************************************************/
cl_int
test(
    cl_context          context,
    cl_device_id        device,
    cl_command_queue    commandQueue,
    int                 testCase
    )
{
    cl_program          program;            /* OpenCL program. */
    cl_kernel           kernel;             /* OpenCL kernel. */
    cl_mem              srcBuf, dstBuf;     /* OpenCL memory buffer objects. */

    cl_build_status        pgmBuildStatus;
    char                *pgmBuildLog;
    size_t                pgmBuildLogSize;

    cl_int status = CL_SUCCESS;
    cl_int errNum;

    cl_int src[8] = { 2, 3, 4, 5, 6, 7, 8 };
    cl_int dst[8] = { 1, 2, 3, 4, 5, 6, 7 };
    cl_float fsrc[16] = { 2.0f, 3.0f, 4.0f, 5.0f,
                          6.0f, 7.0f, 8.0f, 9.0f,
                          2.0f, 3.0f, 4.0f, 5.0f,
                          6.0f, 7.0f, 8.0f, 9.0f};
    cl_float fsrctransposed[16] = { 2.0f, 6.0f, 2.0f, 6.0f,
                                    3.0f, 7.0f, 3.0f, 7.0f,
                                    4.0f, 8.0f, 4.0f, 8.0f,
                                    5.0f, 9.0f, 5.0f, 9.0f};
    cl_float fdst[16] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                          1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    cl_char csrc[2] = { 2, 1 };
    cl_char cdst[2] = { 1, 2 };
    cl_char charsrc[48] = {'d', 'u', 'n', 'g', 'e', 'o', 'n', ' ', '&', ' ', 'd', 'r', 'a', 'g', 'o', 'n',
' ', 'a', 'n', 'd', ' ', 'a', 'l', 'i', 'c', 'e', ' ', 'i', 'n', ' ', 'w', 'o', 'n', 'd', 'e', 'r', 'l', 'a', 'n', 'd',  };
    cl_char chardst[48] = {'c', 'r', 'o', 'u', 'c', 'h', 'i', 'n', 'g', '-', 't', 'i', 'g', 'e', 'r', 's',
' ', 'a', 'n', 'd', ' ', 'f', 'l', 'y', 'i', 'n', 'g', ' ', 'd', 'a', 'g', 'g', 'e', 'r', 's', };
    const cl_char  coffset = 1;
    const cl_uint  offset = 1;
    const cl_float foffset = 1.0f;

    typedef struct _my_struct {
        int a;
        char b;
    } my_struct;

    my_struct ssrc = {2, '2'};
    my_struct sdst =  {0, '0'};

    int arrsrc [] = {4,3,2,1};
    int arrdst [] = {0,0,0,0};

    size_t localWorkSize[1], globalWorkSize[1];

    clmVERBOSE("Running %s testcase %d...\n", testName, testCase);

    clmVERBOSE("Creating program...\n");
    size_t sourceLength = strlen(programSources[testCase]);
    program = clCreateProgramWithSource(context,
                                        1,
                                        (const char **)&programSources[testCase],
                                        &sourceLength,
                                        &errNum);
    clmCHECKERROR(errNum, CL_SUCCESS);

    clmVERBOSE("Building program...\n");
    errNum = clBuildProgram(program, 0, NULL, "-cl-fast-relaxed-math", NULL, NULL);

    if (errNum) {
        clmVERBOSE("Build Error: %i\n", errNum);

        errNum = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, NULL, NULL, &pgmBuildLogSize);
        clmCHECKERROR(errNum, CL_SUCCESS);

        if (pgmBuildLogSize > 1) {
            pgmBuildLog = (char *) malloc(sizeof(char) * pgmBuildLogSize);
            errNum = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, pgmBuildLogSize, pgmBuildLog, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            clmVERBOSE("%s", pgmBuildLog);
        }

        if (testCase == 22 || testCase == 23) {
            /* Negative testcase */
            return CL_SUCCESS;
        }

        return CL_BUILD_PROGRAM_FAILURE;
    }

    clmVERBOSE("Creating kernel...\n");
    kernel = clCreateKernel(program, "loadstore", &errNum);
    clmCHECKERROR(errNum, CL_SUCCESS);

    clmVERBOSE("Creating buffers...\n");
    switch (testCase)
    {
    case 0:
        srcBuf = clCreateBuffer(context,
                                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(cl_float),
                                fsrc,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);

        dstBuf = clCreateBuffer(context,
                                CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(cl_float),
                                fdst,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 1:
        srcBuf = clCreateBuffer(context,
                                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(cl_int),
                                src,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);

        dstBuf = clCreateBuffer(context,
                                CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(cl_int),
                                dst,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 2:
        dstBuf = clCreateBuffer(context,
                                CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(cl_int),
                                dst,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 3:
        srcBuf = clCreateBuffer(context,
                                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(csrc),
                                csrc,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);

        dstBuf = clCreateBuffer(context,
                                CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(cdst),
                                cdst,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 4:
        srcBuf = clCreateBuffer(context,
                                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(fsrc),
                                fsrc,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);

        dstBuf = clCreateBuffer(context,
                                CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(fdst),
                                fdst,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 5:
        srcBuf = clCreateBuffer(context,
                                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(fsrc),
                                fsrc,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);

        dstBuf = clCreateBuffer(context,
                                CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(fdst),
                                fdst,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 6:
        dstBuf = clCreateBuffer(context,
                                CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(fdst),
                                fdst,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 7:
        srcBuf = clCreateBuffer(context,
                                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(charsrc),
                                charsrc,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);

        dstBuf = clCreateBuffer(context,
                                CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(chardst),
                                chardst,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 8:
        srcBuf = clCreateBuffer(context,
                                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(fsrc),
                                fsrc,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);

        dstBuf = clCreateBuffer(context,
                                CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(fdst),
                                fdst,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 9:
        srcBuf = clCreateBuffer(context,
                                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(fsrc),
                                fsrc,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);

        dstBuf = clCreateBuffer(context,
                                CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(fdst),
                                fdst,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 10:
        srcBuf = clCreateBuffer(context,
                                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(fsrc),
                                fsrc,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);

        dstBuf = clCreateBuffer(context,
                                CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(fdst),
                                fdst,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 11:
        srcBuf = clCreateBuffer(context,
                                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(fsrc),
                                fsrc,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);

        dstBuf = clCreateBuffer(context,
                                CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(fdst),
                                fdst,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 12:
        srcBuf = clCreateBuffer(context,
                                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(fsrc),
                                fsrc,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);

        dstBuf = clCreateBuffer(context,
                                CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(fdst),
                                fdst,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 13:
        srcBuf = clCreateBuffer(context,
                                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(fsrc),
                                fsrc,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);

        dstBuf = clCreateBuffer(context,
                                CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(fdst),
                                fdst,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 14:
        srcBuf = clCreateBuffer(context,
                                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(fsrc),
                                fsrc,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);

        dstBuf = clCreateBuffer(context,
                                CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(fdst),
                                fdst,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 15:
        srcBuf = clCreateBuffer(context,
                                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(fsrc),
                                fsrc,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);

        dstBuf = clCreateBuffer(context,
                                CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(fdst),
                                fdst,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 16:
        srcBuf = clCreateBuffer(context,
                                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(fsrc),
                                fsrc,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);

        dstBuf = clCreateBuffer(context,
                                CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(fdst),
                                fdst,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 17:
        srcBuf = clCreateBuffer(context,
                                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(fsrc),
                                fsrc,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);

        dstBuf = clCreateBuffer(context,
                                CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(fdst),
                                fdst,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 18:
        srcBuf = clCreateBuffer(context,
                                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(fsrc),
                                fsrc,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);

        dstBuf = clCreateBuffer(context,
                                CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(fdst),
                                fdst,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 19:
        dstBuf = clCreateBuffer(context,
                                CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(cdst),
                                cdst,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 20:
    case 21:
        dstBuf = clCreateBuffer(context,
                                CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(sdst),
                                &sdst,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 22:
    case 23:
        dstBuf = clCreateBuffer(context,
                                CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(arrdst),
                                arrdst,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;


    case 24:
        srcBuf = clCreateBuffer(context,
                                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(arrsrc),
                                arrsrc,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);

        dstBuf = clCreateBuffer(context,
                                CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(arrdst),
                                arrdst,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 25:
        srcBuf = clCreateBuffer(context,
                                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(charsrc),
                                charsrc,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);

        dstBuf = clCreateBuffer(context,
                                CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(chardst),
                                chardst,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;
    }

    clmVERBOSE("Performing kernel...\n");
    switch (testCase)
    {
    case 0:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_float), (void *) &foffset);
        break;

    case 1:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_uint), (void *) &offset);
        break;

    case 2:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        break;

    case 3:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        break;

    case 4:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_float), (void *) &foffset);
        break;

    case 5:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        break;

    case 6:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        break;

    case 7:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_char), (void *) &coffset);
        break;

    case 8:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        break;

    case 9:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        break;

    case 10:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        break;

    case 11:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        break;

    case 12:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        break;

    case 13:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        break;

    case 14:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_float), (void *) &foffset);
        break;

    case 15:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_float), (void *) &foffset);
        break;

    case 16:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_float), (void *) &foffset);
        break;

    case 17:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_float), (void *) &foffset);
        break;

    case 18:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_float), (void *) &foffset);
        break;

    case 19:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        break;

    case 20:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        break;

    case 21:
        errNum  = clSetKernelArg(kernel, 0, sizeof(my_struct),  (void *) &ssrc);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &dstBuf);
        break;

    case 22:
    case 23:
        errNum  = clSetKernelArg(kernel, 0, sizeof(arrsrc),  (void *) &arrsrc);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &dstBuf);
        break;

    case 24:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &srcBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &dstBuf);
        break;

    case 25:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &srcBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &dstBuf);
        break;

    }
    clmCHECKERROR(errNum, CL_SUCCESS);

    /* Just a single iteration. */
    localWorkSize[0]  = 1;
    globalWorkSize[0] = 1;

    errNum = clEnqueueNDRangeKernel(commandQueue,
                                    kernel,
                                    1,
                                    NULL,
                                    globalWorkSize,
                                    localWorkSize,
                                    0,
                                    NULL,
                                    NULL);
    clmCHECKERROR(errNum, CL_SUCCESS);

    clmVERBOSE("Reading back results...\n");
    switch (testCase)
    {
    case 0:
        errNum = clEnqueueReadBuffer(commandQueue,
                                     dstBuf,
                                     CL_TRUE,
                                     0,
                                     sizeof(cl_float),
                                     fdst,
                                     0,
                                     NULL,
                                     NULL);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 1:
    case 2:
        errNum = clEnqueueReadBuffer(commandQueue,
                                     dstBuf,
                                     CL_TRUE,
                                     0,
                                     sizeof(cl_int),
                                     dst,
                                     0,
                                     NULL,
                                     NULL);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 3:
        errNum = clEnqueueReadBuffer(commandQueue,
                                     dstBuf,
                                     CL_TRUE,
                                     0,
                                     sizeof(cdst),
                                     cdst,
                                     0,
                                     NULL,
                                     NULL);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 4:
        errNum = clEnqueueReadBuffer(commandQueue,
                                     dstBuf,
                                     CL_TRUE,
                                     0,
                                     sizeof(fdst),
                                     fdst,
                                     0,
                                     NULL,
                                     NULL);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 5:
        errNum = clEnqueueReadBuffer(commandQueue,
                                     dstBuf,
                                     CL_TRUE,
                                     0,
                                     sizeof(fdst),
                                     fdst,
                                     0,
                                     NULL,
                                     NULL);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 6:
        errNum = clEnqueueReadBuffer(commandQueue,
                                     dstBuf,
                                     CL_TRUE,
                                     0,
                                     sizeof(fdst),
                                     fdst,
                                     0,
                                     NULL,
                                     NULL);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 7:
        errNum = clEnqueueReadBuffer(commandQueue,
                                     dstBuf,
                                     CL_TRUE,
                                     0,
                                     sizeof(chardst),
                                     chardst,
                                     0,
                                     NULL,
                                     NULL);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 8:
        errNum = clEnqueueReadBuffer(commandQueue,
                                     dstBuf,
                                     CL_TRUE,
                                     0,
                                     sizeof(fdst),
                                     fdst,
                                     0,
                                     NULL,
                                     NULL);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 9:
        errNum = clEnqueueReadBuffer(commandQueue,
                                     dstBuf,
                                     CL_TRUE,
                                     0,
                                     sizeof(fdst),
                                     fdst,
                                     0,
                                     NULL,
                                     NULL);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 10:
        errNum = clEnqueueReadBuffer(commandQueue,
                                     dstBuf,
                                     CL_TRUE,
                                     0,
                                     sizeof(fdst),
                                     fdst,
                                     0,
                                     NULL,
                                     NULL);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 11:
        errNum = clEnqueueReadBuffer(commandQueue,
                                     dstBuf,
                                     CL_TRUE,
                                     0,
                                     sizeof(fdst),
                                     fdst,
                                     0,
                                     NULL,
                                     NULL);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 12:
        errNum = clEnqueueReadBuffer(commandQueue,
                                     dstBuf,
                                     CL_TRUE,
                                     0,
                                     sizeof(fdst),
                                     fdst,
                                     0,
                                     NULL,
                                     NULL);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 13:
        errNum = clEnqueueReadBuffer(commandQueue,
                                     dstBuf,
                                     CL_TRUE,
                                     0,
                                     sizeof(fdst),
                                     fdst,
                                     0,
                                     NULL,
                                     NULL);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 14:
        errNum = clEnqueueReadBuffer(commandQueue,
                                     dstBuf,
                                     CL_TRUE,
                                     0,
                                     sizeof(fdst),
                                     fdst,
                                     0,
                                     NULL,
                                     NULL);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 15:
        errNum = clEnqueueReadBuffer(commandQueue,
                                     dstBuf,
                                     CL_TRUE,
                                     0,
                                     sizeof(fdst),
                                     fdst,
                                     0,
                                     NULL,
                                     NULL);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 16:
        errNum = clEnqueueReadBuffer(commandQueue,
                                     dstBuf,
                                     CL_TRUE,
                                     0,
                                     sizeof(fdst),
                                     fdst,
                                     0,
                                     NULL,
                                     NULL);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 17:
        errNum = clEnqueueReadBuffer(commandQueue,
                                     dstBuf,
                                     CL_TRUE,
                                     0,
                                     sizeof(fdst),
                                     fdst,
                                     0,
                                     NULL,
                                     NULL);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 18:
        errNum = clEnqueueReadBuffer(commandQueue,
                                     dstBuf,
                                     CL_TRUE,
                                     0,
                                     sizeof(fdst),
                                     fdst,
                                     0,
                                     NULL,
                                     NULL);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 19:
        errNum = clEnqueueReadBuffer(commandQueue,
                                     dstBuf,
                                     CL_TRUE,
                                     0,
                                     sizeof(cdst),
                                     cdst,
                                     0,
                                     NULL,
                                     NULL);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 20:
    case 21:
        errNum = clEnqueueReadBuffer(commandQueue,
                                     dstBuf,
                                     CL_TRUE,
                                     0,
                                     sizeof(sdst),
                                     &sdst,
                                     0,
                                     NULL,
                                     NULL);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 22:
    case 23:
    case 24:
        errNum = clEnqueueReadBuffer(commandQueue,
                                     dstBuf,
                                     CL_TRUE,
                                     0,
                                     sizeof(arrdst),
                                     arrdst,
                                     0,
                                     NULL,
                                     NULL);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    case 25:
        errNum = clEnqueueReadBuffer(commandQueue,
                                     dstBuf,
                                     CL_TRUE,
                                     0,
                                     sizeof(chardst),
                                     chardst,
                                     0,
                                     NULL,
                                     NULL);
        clmCHECKERROR(errNum, CL_SUCCESS);
        break;

    }

    switch (testCase)
    {
    case 0:
        if (fdst[0] == fsrc[0] + foffset)
        {
            clmVERBOSE("Testcase 0 passes.\n");
        }
        else
        {
            clmINFO("Testcase 0 fails.\n");
            clmINFO("src=%f\n", fsrc[0]);
            clmINFO("dst=%f\n", fdst[0]);
            clmINFO("offset=%f\n", foffset);
            status = CL_INVALID_VALUE;
        }
        break;

    case 1:
        if (dst[0] == src[0] + offset)
        {
            clmVERBOSE("Testcase 1 passes.\n");
        }
        else
        {
            clmINFO("Testcase 1 fails.\n");
            clmINFO("src=%08x\n", src[0]);
            clmINFO("dst=%08x\n", dst[0]);
            clmINFO("offset=%08x\n", offset);
            status = CL_INVALID_VALUE;
        }
        break;

    case 2:
        if (dst[0] == 5)
        {
            clmVERBOSE("Testcase 2 passes.\n");
        }
        else
        {
            clmINFO("Testcase 2 fails.\n");
            clmINFO("dst=%08x\n", dst[0]);
            status = CL_INVALID_VALUE;
        }
        break;

    case 3:
        {
          int i;
          bool passed = true;
          for (i=0; i < 2; i++ ) {
            if (cdst[i] == csrc[i])
            {
              continue;
            }
            else if(passed) {
              passed = false;
              clmINFO("Testcase 3 fails.\n");
            }
              clmINFO("csrc=%f\n", csrc[i]);
              clmINFO("cdst=%f\n", cdst[i]);
              status = CL_INVALID_VALUE;
            continue;
          }
          if(passed) {
             clmVERBOSE("Testcase 3 passes.\n");
           }
           else {
             status = CL_INVALID_VALUE;
           }
        }
        break;

    case 4:
    {
       int i;
       bool passed = true;

       for(i =0 ; i< 8; i++) {
             if (fdst[i] == (fsrc[i] + foffset)) continue;
             else if(passed) {
                 clmINFO("test 4 fails.\n");
                 clmINFO("offset=%f\n", foffset);
             passed = false;
             }
             clmINFO("fsrc[%d]=%f\n", i, fsrc[i]);
             clmINFO("fdst[%d]=%f\n", i, fdst[i]);
         continue;
       }
       if(passed) {
             clmVERBOSE("Testcase 4 passes.\n");
       }
       else {
             status = CL_INVALID_VALUE;
       }
    }
        break;

    case 5:
    {
       int i;
       bool passed = true;

       for(i =0 ; i< 8; i++) {
             if (fdst[i] == fsrc[i]) continue;
             else if(passed) {
                 clmINFO("test 5 fails.\n");
             passed = false;
             }
             clmINFO("fsrc[%d]=%f\n", i, fsrc[i]);
             clmINFO("fdst[%d]=%f\n", i, fdst[i]);
         continue;
       }
       if(passed) {
             clmVERBOSE("Testcase 5 passes.\n");
       }
       else {
             status = CL_INVALID_VALUE;
       }
    }
        break;

    case 6:
    {
       int i;
       bool passed = true;

       for(i =0 ; i< 8; i++) {
           if (fdst[i] == 5.0) continue;
             else if (passed) {
                 clmINFO("Test 6 fails.\n");
                 passed = false;
             }
             clmINFO("fdst[%d]=%f\n", i, fdst[i]);
         continue;
           }
           if(passed) {
             clmVERBOSE("Testcase 6 passes.\n");
           }
           else {
             status = CL_INVALID_VALUE;
           }
        }
        break;

    case 7:
    {
       int i;
       bool passed = true;

       for(i =0 ; i< 16; i++) {
             if (chardst[i] == (charsrc[i] + coffset)) continue;
             else if(passed) {
                 clmINFO("test 7 fails.\n");
             passed = false;
             }
             clmINFO("charsrc[%c]=%c\n", i, charsrc[i]);
             clmINFO("chardst[%c]=%c\n", i, chardst[i]);
         continue;
       }
       if(passed) {
             clmVERBOSE("Testcase 7 passes.\n");
       }
       else {
             status = CL_INVALID_VALUE;
       }
    }
        break;

    case 8:
    {
       int i;
       bool passed = true;

       for(i =0 ; i< 8; i++) {
             if (fdst[i] == (fsrc[i] + 5)) continue;
             else if(passed) {
                 clmINFO("test 8 fails.\n");
                 clmINFO("offset=%d\n", 5);
             passed = false;
             }
             clmINFO("fsrc[%d]=%f\n", i, fsrc[i]);
             clmINFO("fdst[%d]=%f\n", i, fdst[i]);
         continue;
       }
       if(passed) {
             clmVERBOSE("Testcase 8 passes.\n");
       }
       else {
             status = CL_INVALID_VALUE;
       }
    }
        break;

    case 9:
    {
       int i;
       bool passed = true;

       for(i =0 ; i< 8; i++) {
             if (fdst[i] == (fsrc[i] + 5)) continue;
             else if(passed) {
                 clmINFO("test 9 fails.\n");
                 clmINFO("offset=%d\n", 5);
             passed = false;
             }
             clmINFO("fsrc[%d]=%f\n", i, fsrc[i]);
             clmINFO("fdst[%d]=%f\n", i, fdst[i]);
         continue;
       }
       if(passed) {
             clmVERBOSE("Testcase 9 passes.\n");
       }
       else {
             status = CL_INVALID_VALUE;
       }
    }
        break;

    case 10:
    {
       int i;
       bool passed = true;

       for(i =0 ; i< 16; i++) {
             if (fdst[i] == (fsrc[i] + 5)) continue;
             else if(passed) {
                 clmINFO("test 10 fails.\n");
                 clmINFO("offset=%d\n", 5);
             passed = false;
             }
             clmINFO("fsrc[%d]=%f\n", i, fsrc[i]);
             clmINFO("fdst[%d]=%f\n", i, fdst[i]);
         continue;
       }
       if(passed) {
             clmVERBOSE("Testcase 10 passes.\n");
       }
       else {
             status = CL_INVALID_VALUE;
       }
    }
        break;

    case 11:
    {
       int i;
       bool passed = true;

       for(i =0 ; i< 16; i++) {
             if (fdst[i] == fsrctransposed[i]) continue;
             else if(passed) {
                 clmINFO("test 11 fails.\n");
                 clmINFO("offset=%d\n", 5);
             passed = false;
             }
             clmINFO("fsrc[%d]=%f\n", i, fsrc[i]);
             clmINFO("fdst[%d]=%f\n", i, fdst[i]);
         continue;
       }
       if(passed) {
             clmVERBOSE("Testcase 11 passes.\n");
       }
       else {
             status = CL_INVALID_VALUE;
       }
    }
        break;

    case 12:
    case 13:
    {
       int i;
       bool passed = true;

       for(i =0 ; i< 8; i++) {
             if (fdst[i] == (fsrc[i] + 5)) continue;
             else if(passed) {
                 clmINFO("test %d fails.\n", testCase);
                 clmINFO("offset=%d\n", 5);
             passed = false;
             }
             clmINFO("fsrc[%d]=%f\n", i, fsrc[i]);
             clmINFO("fdst[%d]=%f\n", i, fdst[i]);
         continue;
       }
       if(passed) {
             clmVERBOSE("Testcase %d passes.\n", testCase);
       }
       else {
             status = CL_INVALID_VALUE;
       }
    }
        break;

    case 14:
    {
       int i;
       bool passed = true;

       for(i =0 ; i< 8; i++) {
             if (fdst[i] == (fsrc[7 - i] + foffset)) continue;
             else if(passed) {
                 clmINFO("test 14 fails.\n");
                 clmINFO("offset=%f\n", foffset);
             passed = false;
             }
             clmINFO("fsrc[%d]=%f\n", 7 - i, fsrc[7 - i]);
             clmINFO("fdst[%d]=%f\n", i, fdst[i]);
         continue;
       }
       if(passed) {
             clmVERBOSE("Testcase 14 passes.\n");
       }
       else {
             status = CL_INVALID_VALUE;
       }
    }
        break;

    case 15:
    {
       int i;
       bool passed = true;

       for(i =0 ; i< 8; i++) {
             if (fdst[i] == (fsrc[7 - i] + foffset)) continue;
             else if(passed) {
                 clmINFO("test 15 fails.\n");
                 clmINFO("offset=%f\n", foffset);
             passed = false;
             }
             clmINFO("fsrc[%d]=%f\n", 7 - i, fsrc[7 - i]);
             clmINFO("fdst[%d]=%f\n", i, fdst[i]);
         continue;
       }
       if(passed) {
             clmVERBOSE("Testcase 15 passes.\n");
       }
       else {
             status = CL_INVALID_VALUE;
       }
    }
        break;

    case 16:
    {
       int i;
       bool passed = false;

           do {
             if(fdst[0] != (fsrc[7] + foffset)) break;
             if(fdst[1] != (fsrc[4] + foffset)) break;
             if(fdst[2] != (fsrc[3] + foffset)) break;
             if(fdst[3] != (fsrc[2] + foffset)) break;
             if(fdst[4] != (fsrc[1] + foffset)) break;
             if(fdst[5] != (fsrc[5] + foffset)) break;
             if(fdst[6] != (fsrc[6] + foffset)) break;
             if(fdst[7] != (fsrc[0] + foffset)) break;
             passed = true;
           } while (false);

           if(passed) {
             clmVERBOSE("Testcase 16 passes.\n");
       }
       else {
             status = CL_INVALID_VALUE;
             clmINFO("test 16 fails.\n");
             clmINFO("offset=%f\n", foffset);
         for(i =0 ; i< 8; i++) {
                clmINFO("fsrc[%d]=%f\n", i, fsrc[i]);
                clmINFO("fdst[%d]=%f\n", i, fdst[i]);
             }
       }
    }
        break;

    case 17:
    {
       int i;
       bool passed = false;

           do {
             if(fdst[0] != (fsrc[7] + foffset)) break;
             if(fdst[1] != (fsrc[4] + foffset)) break;
             if(fdst[2] != (fsrc[3] + foffset)) break;
             if(fdst[3] != (fsrc[2] + foffset)) break;
             if(fdst[4] != (fsrc[1] + foffset)) break;
             if(fdst[5] != (fsrc[5] + foffset)) break;
             if(fdst[6] != (fsrc[6] + foffset)) break;
             if(fdst[7] != (fsrc[0] + foffset)) break;
             passed = true;
           } while (false);

           if(passed) {
             clmVERBOSE("Testcase 17 passes.\n");
       }
       else {
             status = CL_INVALID_VALUE;
             clmINFO("test 17 fails.\n");
             clmINFO("offset=%f\n", foffset);
         for(i =0 ; i< 8; i++) {
                clmINFO("fsrc[%d]=%f\n", i, fsrc[i]);
                clmINFO("fdst[%d]=%f\n", i, fdst[i]);
             }
       }
    }
        break;

    case 18:
    {
       int i;
       bool passed = false;

           do {
             if(fdst[0] != (fsrc[11] + foffset)) break;
             if(fdst[1] != (fsrc[10] + foffset)) break;
             if(fdst[2] != (fsrc[9] + foffset)) break;
             if(fdst[3] != (fsrc[8] + foffset)) break;
             if(fdst[4] != (fsrc[1] + foffset)) break;
             if(fdst[5] != (fsrc[5] + foffset)) break;
             if(fdst[6] != (fsrc[6] + foffset)) break;
             if(fdst[7] != (fsrc[0] + foffset)) break;
             passed = true;
           } while (false);

           if(passed) {
             clmVERBOSE("Testcase 18 passes.\n");
       }
       else {
             status = CL_INVALID_VALUE;
             clmINFO("test 18 fails.\n");
             clmINFO("offset=%f\n", foffset);
         for(i =0 ; i< 8; i++) {
                clmINFO("fsrc[%d]=%f\n", i, fsrc[i]);
                clmINFO("fdst[%d]=%f\n", i, fdst[i]);
             }
       }
    }
        break;

    case 19:
        if (cdst[0] == 5)
        {
            clmVERBOSE("Testcase 19 passes.\n");
        }
        else
        {
            clmINFO("Testcase 19 fails.\n");
            clmINFO("dst=%08x\n", cdst[0]);
            status = CL_INVALID_VALUE;
        }
        break;

    case 20:
    case 21:
        if ((sdst.a == ssrc.a) && (sdst.b == ssrc.b))
        {
            clmVERBOSE("Testcase %d passes.\n", testCase);
        }
        else
        {
            clmINFO("Testcase %d fails.\n", testCase);
            clmINFO("src.a=%08x, src.b=%08x \n", ssrc.a, ssrc.b);
            clmINFO("dst.a=%08x, dst.b=%08x \n", sdst.a, sdst.b);
            status = CL_INVALID_VALUE;
        }
        break;

    case 24:
    {
        int i;
        bool passed = false;

        do {
          if(arrsrc[0] != arrdst[0]) break;
          if(arrsrc[1] != arrdst[1]) break;
          if(arrsrc[2] != arrdst[2]) break;
          if(arrsrc[3] != arrdst[3]) break;
          passed = true;
        } while (false);

        if(passed) {
          clmVERBOSE("Testcase %d passes.\n", testCase);
        } else {
          status = CL_INVALID_VALUE;
          clmINFO("Testcase %d fails.\n", testCase);
          for(i =0 ; i< 4; i++) {
            clmINFO("arrsrc[%d]=%d\n", i, arrsrc[i]);
            clmINFO("arrdst[%d]=%d\n", i, arrdst[i]);
          }
        }
     }
        break;

    case 25:
    {
       int i;
       bool passed = false;

       do {
          if(charsrc[0] != chardst[5]) break;
          if(charsrc[1] != chardst[12]) break;
          if(charsrc[2] != chardst[16]) break;
          if(charsrc[3] != chardst[26]) break;
          if(charsrc[17] != chardst[33]) break;
          if(charsrc[17] != chardst[35]) break;
          if(charsrc[17] != chardst[42]) break;
          if(charsrc[17] != chardst[43]) break;
          passed = true;
       } while (false);

       if(passed) {
          clmVERBOSE("Testcase %d passes.\n", testCase);
       } else {
              status = CL_INVALID_VALUE;
              clmINFO("Testcase %d fails.\n", testCase);
          for(i =0 ; i< 4; i++) {
                clmINFO("charsrc[%d]=%c\n", i, charsrc[i]);
                clmINFO("chardst[5]=%c\n", chardst[5]);
                clmINFO("chardst[12]=%c\n", chardst[12]);
                clmINFO("chardst[16]=%c\n", chardst[16]);
                clmINFO("chardst[26]=%c\n", chardst[26]);
              }
              clmINFO("charsrc[17]=%c\n", charsrc[17]);
              clmINFO("chardst[35]=%c\n", chardst[35]);
              clmINFO("chardst[33]=%c\n", chardst[33]);
              clmINFO("chardst[42]=%c\n", chardst[42]);
              clmINFO("chardst[43]=%c\n", chardst[43]);
       }
    }
        break;

    }

    /* Release kernels and program. */
    errNum  = clReleaseKernel(kernel);
    errNum |= clReleaseProgram(program);
    clmCHECKERROR(errNum, CL_SUCCESS);

    /* Release other OpenCL objects. */
    errNum  = clReleaseMemObject(dstBuf);
    if (testCase != 2 &&
        testCase != 6 &&
        testCase != 19 &&
        testCase != 20 &&
        testCase != 21 &&
        testCase != 22 &&
        testCase != 23
        )
    {
        errNum |= clReleaseMemObject(srcBuf);
    }

    return status;
}

/******************************************************************************\
|* Main program                                                               *|
\******************************************************************************/
int
main(
    int argc,
    const char **argv
    )
{
    cl_platform_id      platform;           /* OpenCL platform. */
    cl_device_id        device;             /* OpenCL device. */
    cl_context          context;            /* OpenCL context. */
    cl_command_queue    commandQueue;       /* OpenCL command queue. */

    cl_int errNum;

    cl_int testCase;

    parseArgs(argc, argv, numPrograms, &testCase);

    clmVERBOSE("Unit Test %s Starting...\n", testName);
    clmVERBOSE("Initializing OpenCL...\n");

    /* Get the available platform. */
    errNum = clGetPlatformIDs(1, &platform, NULL);
    clmCHECKERROR(errNum, CL_SUCCESS);

    /* Get a GPU device. */
    errNum = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    clmCHECKERROR(errNum, CL_SUCCESS);

    /* Create the context. */
    context = clCreateContext(0, 1, &device, NULL, NULL, &errNum);
    clmCHECKERROR(errNum, CL_SUCCESS);

    /* Create a command-queue. */
    commandQueue = clCreateCommandQueue(context, device, 0, &errNum);
    clmCHECKERROR(errNum, CL_SUCCESS);

    if (testCase >= 0)
    {
        test(context, device, commandQueue, testCase);
    }
    else
    {
        int failure = 0;

        for (testCase = 0; testCase < numPrograms; testCase++)
        {
            if (test(context, device, commandQueue, testCase) != CL_SUCCESS)
            {
                failure++;
            }
        }

        clmINFO("%s: total = %d, failure = %d\n",
               testName, testCase, failure);
    }

    errNum |= clReleaseCommandQueue(commandQueue);
    errNum |= clReleaseContext(context);
    clmCHECKERROR(errNum, CL_SUCCESS);
}
