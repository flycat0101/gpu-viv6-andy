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


#include "clutil.h"
#ifdef UNDER_CE
#include <windows.h>
#endif

// global variables
cl_context cxContext = 0;
cl_program cpProgram = 0;
cl_device_id cdDeviceID[2];
cl_kernel kernels[FFT_MAX_LOG2N];
cl_command_queue commandQueue;
cl_event gpuExecution[FFT_MAX_LOG2N];
cl_event gpuDone;

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

// default configs
unsigned blockSize = 16;
unsigned print = 1;

// h_Freal and h_Fimag represent the input signal to be transformed.
// h_Rreal and h_Rimag represent the transformed output.
float*  h_Freal = 0;
float*  h_Fimag = 0;
float*  h_Rreal = 0;
float*  h_Rimag = 0;
//  real & imag interleaved
float* h_intime = 0; // time-domain input samples
float* h_outfft = 0; // freq-domain output samples

// d_Freal and d_Fimag represent the input signal to be transformed.
// d_Rreal and d_Rimag represent the transformed output.
cl_mem d_Freal;
cl_mem d_Fimag;
cl_mem d_Rreal;
cl_mem d_Rimag;
//  real & imag interleaved
cl_mem d_intime; // time-domain input samples
cl_mem d_outfft; // freq-domain output samples

int
initExecution(
    const unsigned len
    )
{
    // Allocate host memory (and initialize input signal)
    allocateHostMemory(len);

    printf("Initializing device(s)...\n");
    // create the OpenCL context on available GPU devices
    init_cl_context(CL_DEVICE_TYPE_GPU);

    const cl_uint ciDeviceCount =  getDeviceCount();

    if (!ciDeviceCount)
    {
        printf("No opencl specific devices!\n");
        return -1;
    }

    const cl_uint ciComputeUnitsCount = getNumComputeUnits();
    printf("# compute units = %d\n", ciComputeUnitsCount);

    printf("Creating Command Queue...\n");
    // create a command queue on device 0
    createCommandQueue();

    return 0;
}

void
printGpuTime(
    const unsigned int kernelCount
    )
{
    double t, total = 0;

    for (unsigned k = 0; k<kernelCount; ++k)
    {
        t = executionTime(gpuExecution[k]);
        printf("Kernel execution time on GPU (kernel %d) : %10.6f seconds\n", k, t);
        total += t;
    }
    printf("Total Kernel execution time on GPU : %10.6f seconds\n",total);
}

void
printResult(
    const unsigned size
    )
{
    FILE *fp;
#ifdef UNDER_CE
    wchar_t moduleName[MAX_PATH];
    char path[MAX_PATH], * p;
    GetModuleFileName(NULL, moduleName, MAX_PATH);
    wcstombs(path, moduleName, MAX_PATH);
    p = strrchr(path, '\\');
    strcpy(p + 1, "fft_output.csv");
    fp = fopen(path, "w+");
#else
   fp = fopen("fft_output.csv", "w+");
#endif

    if (fp == NULL) return;

    for (unsigned i = 0; i < size; ++i)
    {
        fprintf(fp, "%f,%f\n", h_outfft[2*i], h_outfft[2*i+1]);
    }
    fclose(fp);
}

double
executionTime(
    const cl_event event
    )
{
    cl_ulong start, end;
    cl_int err;
    err = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);
    err |= clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, NULL);
    if (err)
    {
        return 0;
    }
    return (double)1.0e-9 * (end - start); // convert nanoseconds to seconds
}

void
allocateHostMemory(
    const unsigned len
    )
{
    h_Freal = (float *) malloc(sizeof(float) * len);
    checkError((h_Freal != NULL), CL_TRUE, "Could not allocate memory");

    h_Fimag = (float *) malloc(sizeof(float) * len);
    checkError((h_Fimag != NULL), CL_TRUE, "Could not allocate memory");

    h_Rreal = (float *) malloc(sizeof(float) * len);
    checkError((h_Rreal != NULL), CL_TRUE, "Could not allocate memory");

    h_Rimag = (float *) malloc(sizeof(float) * len);
    checkError((h_Rimag != NULL), CL_TRUE, "Could not allocate memory");

    //  real/imag interleaved input time-domain samples
    h_intime = (float *) malloc(sizeof(float) * len * 2);
    checkError((h_intime != NULL), CL_TRUE, "Could not allocate memory");

    //  real/imag interleaved output FFT data
    h_outfft = (float *) malloc(sizeof(float) * len * 2);
    checkError((h_outfft != NULL), CL_TRUE, "Could not allocate memory");

    const unsigned n = 16;
    for (unsigned i = 0 ; i < len; ++i)
    {
        h_Freal[i] = (i + 1) % n;
        h_Fimag[i] = (i + 1) % n;
        h_intime[2*i] = h_intime[2*i+1] = (i + 1) % n;
        h_Rreal[i] = 0;
        h_Rimag[i] = 0;
        h_outfft[2*i] = h_outfft[2*i+1] = 0;
    }

    if (print)
    {
        FILE *fp = NULL;
#ifdef UNDER_CE
        wchar_t moduleName[MAX_PATH];
        char path[MAX_PATH], * p;
        GetModuleFileName(NULL, moduleName, MAX_PATH);
        wcstombs(path, moduleName, MAX_PATH);
        p = strrchr(path, '\\');
        strcpy(p + 1, "fft_input.csv");
        fp = fopen(path, "w+");
#else
        fp = fopen("fft_input.csv", "w+");
#endif
        if (fp == NULL) return;

        for (unsigned int kk=0; kk<len; kk++)
        {
            fprintf(fp, "%f,%f\n", h_intime[2*kk], h_intime[2*kk+1]);
        }
        fclose(fp);
    }
}

void
allocateDeviceMemory(
    const unsigned size,
    const unsigned copyOffset
    )
{
    d_Freal = createDeviceBuffer(CL_MEM_READ_ONLY, sizeof(float) * size, h_Freal + copyOffset);
    copyToDevice(d_Freal,  h_Freal + copyOffset, size);

    d_Fimag = createDeviceBuffer(CL_MEM_READ_ONLY, sizeof(float) * size, h_Fimag + copyOffset);
    copyToDevice(d_Fimag,  h_Fimag + copyOffset, size);

    //  copy real/imag interleaved input data to device
    d_intime = createDeviceBuffer(CL_MEM_READ_WRITE, sizeof(float) * size * 2, h_intime + copyOffset * 2);
    copyFromDevice(d_intime, h_outfft, size * 2); // debug

    d_Rreal = createDeviceBuffer(CL_MEM_WRITE_ONLY, sizeof(float) * size, h_Rreal + copyOffset);
    copyToDevice(d_Rreal,  h_Rreal + copyOffset, size);

    d_Rimag = createDeviceBuffer(CL_MEM_WRITE_ONLY, sizeof(float) * size, h_Rimag + copyOffset);
    copyToDevice(d_Rimag,  h_Rimag + copyOffset, size);

    //  copy real/imag interleaved out FFT to device
    d_outfft = createDeviceBuffer(CL_MEM_READ_WRITE, sizeof(float) * size * 2, h_outfft + copyOffset * 2);
    copyToDevice(d_intime,  h_outfft + copyOffset * 2, size * 2);
}

void
cleanup(
    void
    )
{
    if (d_Freal)  clReleaseMemObject(d_Freal);
    if (d_Fimag)  clReleaseMemObject(d_Fimag);
    if (d_Rreal)  clReleaseMemObject(d_Rreal);
    if (d_Rimag)  clReleaseMemObject(d_Rimag);
    if (d_intime) clReleaseMemObject(d_intime);
    if (d_outfft) clReleaseMemObject(d_outfft);

    if (gpuDone) clReleaseEvent(gpuDone);

    for (unsigned kk=0; kk<ARRAY_SIZE(kernels); kk++) {
        if (gpuExecution[kk]) clReleaseEvent(gpuExecution[kk]);
    }

    if (commandQueue) clReleaseCommandQueue(commandQueue);
    if (cpProgram) clReleaseProgram(cpProgram);
    if (cxContext) clReleaseContext(cxContext);

    free(h_Freal);
    h_Freal = 0;
    free(h_Fimag);
    h_Fimag = 0;
    free(h_Rreal);
    h_Rreal = 0;
    free(h_Rimag);
    h_Rimag = 0;
    free(h_intime);
    h_intime = 0;
    free(h_outfft);
    h_outfft = 0;
}

void
checkError(
    const cl_int ciErrNum,
    const cl_int ref,
    const char* const operation
    )
{
    if (ciErrNum != ref) {
        printf("ERROR:: %d %s failed\n\n", ciErrNum, operation);
        cleanup();
        exit(EXIT_FAILURE);
    }
}

void
init_cl_context(
    const cl_device_type device_type
    )
{
    cl_int ciErrNum = CL_SUCCESS;

#ifndef WIN32
    cxContext = clCreateContextFromType(0, /* cl_context_properties */
                          device_type,
                        NULL, /* error function ptr */
                        NULL, /* user data to be passed to err fn */
                        &ciErrNum);
    checkError(ciErrNum, CL_SUCCESS, "clCreateContextFromType");
#else
    cl_platform_id cpPlatform;
    ciErrNum =     clGetPlatformIDs(1, &cpPlatform, NULL);
    checkError(ciErrNum, CL_SUCCESS, "clGetPlatformIDs");
    cl_uint uiNumDevices;
    ciErrNum = clGetDeviceIDs(cpPlatform, device_type, 0, NULL, &uiNumDevices);
    checkError(ciErrNum, CL_SUCCESS, "clGetDeviceIDs");
    cl_device_id cdDevices[20];
    ciErrNum = clGetDeviceIDs(cpPlatform, device_type, uiNumDevices, cdDevices, NULL);
    checkError(ciErrNum, CL_SUCCESS, "clGetDeviceIDs");
    cl_uint targetDevice=0, uiNumDevsUsed=1;
    cxContext = clCreateContext(0, uiNumDevsUsed, &cdDevices[targetDevice], NULL, NULL, &ciErrNum);
    checkError(ciErrNum, CL_SUCCESS, "clCreateContextFromType");
#endif
}

cl_uint
getDeviceCount(
    void
    )
{
    size_t nDeviceBytes;
    const cl_int ciErrNum = clGetContextInfo(cxContext, CL_CONTEXT_DEVICES, 0, NULL, &nDeviceBytes);
    checkError(ciErrNum, CL_SUCCESS, "clGetContextInfo");
    return ((cl_uint)nDeviceBytes/sizeof(cl_device_id));
}

cl_uint
getNumComputeUnits(
    void
    )
{
    cl_platform_id cpPlatform;
    cl_int ciErrNum = clGetPlatformIDs(1, &cpPlatform, NULL);
    checkError(ciErrNum, CL_SUCCESS, "clGetPlatformIDs");

    //Get all the devices
    printf("Get the Device info and select Device...\n");
    cl_uint uiNumDevices;
    ciErrNum = clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 0, NULL, &uiNumDevices);
    checkError(ciErrNum, CL_SUCCESS, "clGetDeviceIDs");
    cl_device_id *cdDevices = (cl_device_id *)malloc(uiNumDevices * sizeof(cl_device_id) );
    ciErrNum = clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, uiNumDevices, cdDevices, NULL);
    checkError(ciErrNum, CL_SUCCESS, "clGetDeviceIDs");

    // Set target device and Query number of compute units on targetDevice
    printf("# of Devices Available = %d\n", uiNumDevices);
    cl_uint num_compute_units;
    clGetDeviceInfo(cdDevices[0], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(num_compute_units), &num_compute_units, NULL);
    printf("# of Compute Units = %d\n", num_compute_units);
    free(cdDevices);
    return num_compute_units;
}

void
createCommandQueue(
    void
    )
{
    cl_int ciErrNum = CL_SUCCESS;
    ciErrNum = clGetContextInfo(cxContext, CL_CONTEXT_DEVICES, sizeof(cl_device_id)*2, &cdDeviceID, NULL);
    commandQueue = clCreateCommandQueue(cxContext, cdDeviceID[0], CL_QUEUE_PROFILING_ENABLE, &ciErrNum);
    checkError(ciErrNum, CL_SUCCESS, "clCreateCommandQueue");
}

void
compileProgram(
    const char* const kernel_file
    )
{
    size_t program_length;
    FILE* pFileStream = NULL;
    cl_int ciErrNum;

#ifdef _WIN32
#ifdef UNDER_CE
    wchar_t moduleName[MAX_PATH];
    char path[MAX_PATH], * p;
    GetModuleFileName(NULL, moduleName, MAX_PATH);
    wcstombs(path, moduleName, MAX_PATH);
    p = strrchr(path, '\\');
    strcpy(p + 1, kernel_file);
    pFileStream = fopen(path, "rb");
    if (pFileStream == NULL)
    {
        checkError(CL_INVALID_VALUE, CL_SUCCESS, "compileProgram on open source");
    }
#else
    if(fopen_s(&pFileStream, kernel_file, "rb") != 0)
    {
        checkError(CL_INVALID_VALUE, CL_SUCCESS, "compileProgram on open source");
    }
#endif
#else
    pFileStream = fopen(kernel_file, "rb");
    if(pFileStream == 0)
    {
        checkError(CL_INVALID_VALUE, CL_SUCCESS, "compileProgram on open source");
    }
#endif

    // get the length of the source code
    fseek(pFileStream, 0, SEEK_END);
    program_length = ftell(pFileStream);
    fseek(pFileStream, 0, SEEK_SET);

    // allocate a buffer for the source code string and read it in
    char* source = (char *)malloc(program_length + 1);
    if (fread((source), program_length, 1, pFileStream) != 1)
    {
        fclose(pFileStream);
        free(source);
        checkError(CL_INVALID_VALUE, CL_SUCCESS, "compileProgram on read source");
    }
    fclose(pFileStream);
    source[program_length] = '\0';

    // Create the program for all GPUs in the context
    cpProgram = clCreateProgramWithSource( cxContext, 1, (const char **) &source, &program_length, &ciErrNum);
    free(source);
    checkError(ciErrNum, CL_SUCCESS, "clCreateProgramWithSource");
    ciErrNum = clBuildProgram(cpProgram, 0, NULL, "", NULL, NULL);
    if (ciErrNum != CL_SUCCESS)
    {
        char cBuildLog[10240];
        clGetProgramBuildInfo(cpProgram, cdDeviceID[0], CL_PROGRAM_BUILD_LOG, sizeof(cBuildLog), cBuildLog, NULL );
        printf("\nBuild Log : \n%s\n", cBuildLog);
        checkError(ciErrNum, CL_SUCCESS, "clBuildProgram");
    }
}

void
createFFTKernel(
    const char* const kernelName,
    int kk
    )
{
    cl_int ciErrNum = CL_SUCCESS;
    kernels[kk] = clCreateKernel(cpProgram, kernelName, &ciErrNum);
    checkError(ciErrNum, CL_SUCCESS, "clCreateKernel");
}

cl_mem
createDeviceBuffer(
    const cl_mem_flags flags,
    const size_t size,
    void* const hostPtr
    )
{
    cl_int ciErrNum = CL_SUCCESS;
    const cl_mem d_mem = clCreateBuffer(cxContext, flags | CL_MEM_COPY_HOST_PTR, size, hostPtr, &ciErrNum);
    checkError(ciErrNum, CL_SUCCESS,  "clCreateBuffer");
    return d_mem;
}

void
copyToDevice(
    const cl_mem mem,
    float* const hostPtr,
    const unsigned size
    )
{
    const cl_int ciErrNum = clEnqueueWriteBuffer(commandQueue, mem, CL_FALSE, 0, sizeof(float) * size, hostPtr, 0, NULL, NULL);
    checkError(ciErrNum, CL_SUCCESS,  "clEnqueueWriteBuffer");
}

void
copyFromDevice(
    const cl_mem dMem,
    float* const hostPtr,
    const unsigned size
    )
{
    cl_int ciErrNum = clEnqueueReadBuffer(commandQueue, dMem, CL_FALSE, 0, sizeof(float) * size, hostPtr, 0, NULL, &gpuDone);
    checkError(ciErrNum, CL_SUCCESS, "clEnqueueReadBuffer");
}

void
runKernelFFT(
    const size_t localWorkSize[],
    const size_t globalWorkSize[],
    const int kk
    )
{
    const cl_int ciErrNum = clEnqueueNDRangeKernel(commandQueue, kernels[kk], 1, NULL, globalWorkSize, localWorkSize, 0, NULL, &gpuExecution[kk]);
    checkError(ciErrNum, CL_SUCCESS, "clEnqueueNDRangeKernel");
}

