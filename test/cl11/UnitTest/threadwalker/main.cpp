/****************************************************************************
*
*    Copyright 2012 - 2015 Vivante Corporation, Santa Clara, California.
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <CL/opencl.h>

#define clmCHECKERROR(a, b) checkError(a, b, __FILE__ , __LINE__)

void
checkError(
    cl_int Value,
    cl_int Reference,
    const char* FileName,
    const int LineNumber
    )
{
    if (Value != Reference)
    {
        printf("\n !!! Error # %i at line %i , in file %s !!!\n\n",
            Value, LineNumber, FileName);

        printf("Exiting...\n");
        exit(EXIT_FAILURE);
    }
}

/******************************************************************************\
|* CL programs                                                                *|
\******************************************************************************/
char * programSources[] =
{
/* Case 0. */
"\
__kernel void threadwalker( \
    __global int *pdst \
    ) \
{ \
    pdst[0] = get_global_id(0); \
    pdst[1] = get_global_id(1); \
    pdst[2] = get_local_id(0); \
    pdst[3] = get_local_id(1); \
    pdst[4] = get_group_id(0); \
    pdst[5] = get_group_id(1); \
}",

/* Case 1. */
"\
__kernel void threadwalker( \
    __global int *pdst \
    ) \
{ \
    int i0 = get_global_id(0); \
    int j0 = get_global_id(1); \
    int i1 = get_local_id(0); \
    int j1 = get_local_id(1); \
    int i = get_group_id(0); \
    int j = get_group_id(1); \
    int v = j << 16 | i; \
    int k = (j0 & 0xf); \
    k = k + k + k + (i0 & 0xf); \
    pdst[k] = v; \
}",

/* Case 2. */
"\
__kernel void threadwalker( \
    __global int *pdst \
    ) \
{ \
	int i = get_global_id(0); \
    pdst[i] = get_global_id(0); \
}",

/* Case 3. */
"\
__kernel void threadwalker( \
    __global int *pdst \
    ) \
{ \
	int i = get_global_id(0); \
    pdst[i] = get_global_id(0); \
}",

/* Case 4. */
"\
__kernel void threadwalker( \
    __global int *pdst \
    ) \
{ \
	int i = 2 * (2*get_global_id(1) + get_global_id(0)); \
    pdst[i] = get_global_id(0); \
    pdst[i+1] = get_global_id(1); \
}",

/* Case 5. */
"\
__kernel void threadwalker( \
    __global int *pdst \
    ) \
{ \
	int i = 2 * (2*get_global_id(1) + get_global_id(0)); \
    pdst[i] = get_global_id(0); \
    pdst[i+1] = get_global_id(1); \
}",
};

cl_int numPrograms = sizeof(programSources) / sizeof(programSources[0]);

/******************************************************************************\
|* Main program                                                               *|
\******************************************************************************/
int main(
    int argc,
    const char **argv
    )
{
    cl_platform_id      platform;           /* OpenCL platform. */
    cl_device_id        device;             /* OpenCL device. */
    cl_context          context;            /* OpenCL context. */
    cl_command_queue    commandQueue;       /* OpenCL command queue. */
    cl_mem              dstBuf;             /* OpenCL memory buffer object. */
    cl_program          program;            /* OpenCL program. */
    cl_kernel           kernel;             /* OpenCL kernel. */

    cl_int errNum;
	cl_int i, workDim;

    cl_int testCase = 3;

    size_t localWorkSize[2], globalWorkSize[2], globalWorkOffset[2];

    cl_int dst[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };

    if (argc == 2)
    {
        testCase = atoi(argv[1]);
        if (testCase > numPrograms)
        {
            printf("Incorrect testcase (%d).\n", testCase);
            exit(EXIT_FAILURE);
        }
    }

    /* set logfile name and start logs. */
    printf("%s Starting...\n\n", argv[0]);

	printf("Initializing OpenCL...\n");

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

	printf("Initializing OpenCL ThreadWalker...\n");

    printf("Creating ThreadWalker program...\n");
    size_t sourceLength = strlen(programSources[testCase]);
    program = clCreateProgramWithSource(context,
                                        1,
                                        (const char **)&programSources[testCase],
                                        &sourceLength
                                        , &errNum);
    clmCHECKERROR(errNum, CL_SUCCESS);

    printf("Building ThreadWalker program...\n");
	errNum = clBuildProgram(program, 0, NULL, "-cl-fast-relaxed-math", NULL, NULL);
    clmCHECKERROR(errNum, CL_SUCCESS);

    printf("Creating ThreadWalker kernels...\n");
    kernel = clCreateKernel(program, "threadwalker", &errNum);
    clmCHECKERROR(errNum, CL_SUCCESS);

    printf("Creating OpenCL memory objects...\n");
    dstBuf = clCreateBuffer(context,
						    CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
						    8 * sizeof(cl_int),
						    dst,
						    &errNum);
    clmCHECKERROR(errNum, CL_SUCCESS);

    printf("Performing ThreadWalker...\n\n");

    switch (testCase)
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        break;
    }
    clmCHECKERROR(errNum, CL_SUCCESS);


    switch (testCase)
    {
    case 0:
	    /* Just a single iteration. */
        localWorkSize[0]  = 1;
        localWorkSize[1]  = 1;
        globalWorkSize[0] = 1;
        globalWorkSize[1] = 1;
        globalWorkOffset[0] = 0x10;
        globalWorkOffset[1] = 0x20;
		workDim = 2;
        break;

    case 1:
        localWorkSize[0]  = 1;
        localWorkSize[1]  = 1;
        globalWorkSize[0] = 3;
        globalWorkSize[1] = 2;
        globalWorkOffset[0] = 0x10;
        globalWorkOffset[1] = 0x20;
		workDim = 2;
        break;

    case 2:
        localWorkSize[0]  = 1;
        localWorkSize[1]  = 0;
        globalWorkSize[0] = 8;
        globalWorkSize[1] = 0;
        globalWorkOffset[0] = 0;
        globalWorkOffset[1] = 0;
		workDim = 1;
        break;

    case 3:
        localWorkSize[0]  = 2;
        localWorkSize[1]  = 0;
        globalWorkSize[0] = 8;
        globalWorkSize[1] = 0;
        globalWorkOffset[0] = 0;
        globalWorkOffset[1] = 0;
		workDim = 1;
        break;

    case 4:
        localWorkSize[0]  = 1;
        localWorkSize[1]  = 1;
        globalWorkSize[0] = 2;
        globalWorkSize[1] = 2;
        globalWorkOffset[0] = 0;
        globalWorkOffset[1] = 0;
		workDim = 2;
        break;

    case 5:
        localWorkSize[0]  = 2;
        localWorkSize[1]  = 2;
        globalWorkSize[0] = 2;
        globalWorkSize[1] = 2;
        globalWorkOffset[0] = 0;
        globalWorkOffset[1] = 0;
		workDim = 2;
        break;
    }

    errNum = clEnqueueNDRangeKernel(commandQueue,
                                    kernel,
                                    workDim,
                                    globalWorkOffset,
                                    globalWorkSize,
                                    localWorkSize,
                                    0,
                                    NULL,
                                    NULL);
    clmCHECKERROR(errNum, CL_SUCCESS);

    printf("Reading back OpenCL results...\n");
    errNum = clEnqueueReadBuffer(commandQueue,
								 dstBuf,
								 CL_TRUE,
								 0,
								 8 * sizeof(cl_int),
								 dst,
								 0,
								 NULL,
								 NULL);
    clmCHECKERROR(errNum, CL_SUCCESS);

    switch (testCase)
    {
    case 0:
	    printf("global_id=%08x %08x\n", dst[0], dst[1]);
	    printf("local_id =%08x %08x\n", dst[2], dst[3]);
	    printf("group_id =%08x %08x\n", dst[4], dst[5]);
        break;
    case 1:
	    printf("dst=%08x %08x %08x\n", dst[0], dst[1], dst[2]);
	    printf("dst=%08x %08x %08x\n", dst[3], dst[4], dst[5]);
        break;
    case 2:
    case 3:
		for (i=0; i<8; i++) {
		    printf("[%d] %08x \n", i, dst[i]);
		}
        break;
    case 4:
    case 5:
	    printf("[0,0] %08x %08x\n", dst[0], dst[1]);
	    printf("[1,0] %08x %08x\n", dst[2], dst[3]);
	    printf("[0,1] %08x %08x\n", dst[4], dst[5]);
	    printf("[1,1] %08x %08x\n", dst[6], dst[7]);
        break;
    }

    /* Release kernels and program. */
    errNum  = clReleaseKernel(kernel);
    errNum |= clReleaseProgram(program);
    clmCHECKERROR(errNum, CL_SUCCESS);

    /* Release other OpenCL objects. */
    errNum  = clReleaseMemObject(dstBuf);

    errNum |= clReleaseCommandQueue(commandQueue);
    errNum |= clReleaseContext(context);
    clmCHECKERROR(errNum, CL_SUCCESS);

}
