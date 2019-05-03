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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <CL/opencl.h>

#define clmCHECKERROR(a, b) checkError(a, b, 0, __FILE__ , __LINE__)

#define PRINT_BUFFER_SIZE   sizeof(char)*1024

// Released (non-beta) Nvidia drivers currently do not support Open CL 1.1
//#define NVIDIA_OPENCL_1_0

#define PRINT_PROGRAM_BINARY
#define PRINTABLE(c) ((((c) >= ' ') && ((c) <= '}')) ? ((c) != '%' ?  (c) : ' ') : ' ')

void
checkError(
    cl_int Value,
    cl_int Reference,
    void (*Cleanup)(int),
    const char* FileName,
    const int LineNumber
    )
{
    if (Reference != Value)
    {
        printf("\n !!! Error # %i at line %i , in file %s !!!\n\n",
            Value, LineNumber, FileName);
        if (Cleanup != NULL)
        {
            Cleanup(EXIT_FAILURE);
        }
        else
        {
            fflush(stderr);
            printf(" !!! Exiting...\n");

            exit(EXIT_FAILURE);
        }
    }
}

char *helloSource =
"\
__kernel void hello() \
{ \
    size_t i =  get_global_id(0); \
}";


/******************************************************************************\
|* Main program                                                               *|
\******************************************************************************/
int main(int        argc,
         const char **argv)
{
    cl_platform_id      *platformID = NULL; /* OpenCL platform */
    cl_device_id        *deviceID = NULL;   /* OpenCL device */
    cl_context          context;            /* OpenCL context */
    cl_command_queue    commandQueue;       /* OpenCL command queue */
    cl_program          program;            /* OpenCL program */
    cl_kernel           kernel;             /* OpenCL kernel */

    cl_int              errNum;
    cl_uint             numPlatforms, numDevices;

    cl_device_type      deviceType;
    cl_uint             vendor_id;
    cl_platform_id      devicePlatformID;
    cl_uint             compute_units;
    cl_uint             workItemDims;
    size_t              workItemSize[3];
    size_t              workgroupSize;
    cl_uint             clockFrequency;
    cl_uint             addrBits;
    cl_ulong            maxMemAllocSize;
    cl_ulong            globalMemSize;
    cl_bool             errorCorrectionSupport;
    cl_device_local_mem_type localMemType;
    cl_command_queue_properties queueProperties;
    cl_bool             imageSupport;
    cl_uint             maxReadImageArgs;
    cl_uint             maxWriteImageArgs;

    cl_uint                vectorWidthChar;
    cl_uint                vectorWidthShort;
    cl_uint                vectorWidthInt;
    cl_uint                vectorWidthLong;
    cl_uint                vectorWidthFloat;
    cl_uint                vectorWidthDouble;

    size_t                 image2DMaxWidth;
    size_t                 image2DMaxHeight;
    size_t                 image3DMaxWidth;
    size_t                 image3DMaxHeight;
    size_t                 image3DMaxDepth;

    cl_uint                maxSamplers;
    size_t                maxParameterSize;
    cl_uint                memBaseAddrAlign;
    cl_uint                minDataTypeAlignSize;
    cl_device_fp_config singleFpConfig;
    cl_device_mem_cache_type globalMemCacheType;
    cl_uint                globalMemCachelineSize;
    cl_ulong            globalMemCacheSize;
    cl_uint                maxConstantArgs;
#ifndef NVIDIA_OPENCL_1_0
    cl_bool                hostUnifiedMemory;
#endif
    size_t                profilingTimingRes;
    cl_bool                endianLittle;
    cl_bool                deviceAvail;
    cl_bool                compilerAvail;
    cl_device_exec_capabilities execCapability;

    char                 printBuffer[PRINT_BUFFER_SIZE];
    char                 *p;

    cl_uint                i,j,x;
#ifdef PRINT_PROGRAM_BINARY
    cl_uint                k;
#endif

    printf("\n>>>>>>>> %s Starting...\n\n", argv[0]);

    errNum = clGetPlatformIDs(0, NULL, &numPlatforms);
    clmCHECKERROR(errNum, CL_SUCCESS);
    printf("Available platforms: %d\n\n", numPlatforms);

    platformID = (cl_platform_id *) malloc(sizeof(cl_platform_id) * numPlatforms);

    errNum = clGetPlatformIDs(numPlatforms, platformID, NULL);
    clmCHECKERROR(errNum, CL_SUCCESS);

    for (i=0; i<numPlatforms; i++) {
        printf("Platform ID: %d\n", i);

        // ******************** clGetPlatformInfo ********************

        errNum = clGetPlatformInfo(platformID[i], CL_PLATFORM_NAME, PRINT_BUFFER_SIZE, printBuffer, NULL);
        clmCHECKERROR(errNum, CL_SUCCESS);
        printf("\t CL_PLATFORM_NAME:       %s\n", printBuffer);

        errNum = clGetPlatformInfo(platformID[i], CL_PLATFORM_PROFILE, PRINT_BUFFER_SIZE, printBuffer, NULL);
        clmCHECKERROR(errNum, CL_SUCCESS);
        printf("\t CL_PLATFORM_PROFILE:    %s\n", printBuffer);

        errNum = clGetPlatformInfo(platformID[i], CL_PLATFORM_VERSION, PRINT_BUFFER_SIZE, printBuffer, NULL);
        clmCHECKERROR(errNum, CL_SUCCESS);
        printf("\t CL_PLATFORM_VERSION:    %s\n", printBuffer);

        errNum = clGetPlatformInfo(platformID[i], CL_PLATFORM_VENDOR, PRINT_BUFFER_SIZE, printBuffer, NULL);
        clmCHECKERROR(errNum, CL_SUCCESS);
        printf("\t CL_PLATFORM_VENDOR:     %s\n", printBuffer);

        errNum = clGetPlatformInfo(platformID[i], CL_PLATFORM_EXTENSIONS, PRINT_BUFFER_SIZE, printBuffer, NULL);
        clmCHECKERROR(errNum, CL_SUCCESS);
        printf("\t CL_PLATFORM_EXTENSIONS: ");

        if (*printBuffer != '\0'){
            for (p=printBuffer; *p!='\0'; p++) {
                putchar(*p);
                if (*p==' ') printf("\n\t\t\t\t ");
            }
        } else {
            printf("None");
        }
        printf("\n");

        errNum = clGetDeviceIDs(platformID[i], CL_DEVICE_TYPE_ALL, 0, NULL, &numDevices);
        clmCHECKERROR(errNum, CL_SUCCESS);
        printf("\n\n\t Available devices:      %d\n\n", numDevices);

        deviceID = (cl_device_id *) malloc(sizeof(cl_device_id) * numDevices);

        errNum = clGetDeviceIDs(platformID[i], CL_DEVICE_TYPE_ALL, numDevices, deviceID, &numDevices);
        clmCHECKERROR(errNum, CL_SUCCESS);

        for (j=0; j<numDevices; j++) {
            printf("\t Device ID:  \t%d\n", j);
            printf("\t Device Ptr: \t0x%08x\n", deviceID[j]);

            // ******************** clGetDeviceInfo ********************

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_NAME, PRINT_BUFFER_SIZE, printBuffer, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_NAME: %s\n", printBuffer);

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_VENDOR, PRINT_BUFFER_SIZE, printBuffer, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_VENDOR: %s\n", printBuffer);

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_TYPE, sizeof(deviceType), &deviceType, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_TYPE: \t\t\t\t");
            switch (deviceType) {
            case CL_DEVICE_TYPE_ACCELERATOR:
                printf("ACCELERATOR \n ");
                break;
            case CL_DEVICE_TYPE_CPU:
                printf("CPU \n ");
                break;
            case CL_DEVICE_TYPE_DEFAULT:
                printf("DEFAULT \n ");
                break;
            case CL_DEVICE_TYPE_GPU:
                printf("GPU \n ");
                break;
            }

#ifndef NVIDIA_OPENCL_1_0
            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_OPENCL_C_VERSION, PRINT_BUFFER_SIZE, printBuffer, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_OPENCL_C_VERSION: \t\t\t%s\n", printBuffer);
#endif
            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_VENDOR_ID, sizeof(vendor_id), &vendor_id, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_VENDOR_ID: \t\t\t\t0x%08x\n", vendor_id);

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_PLATFORM, sizeof(devicePlatformID), &devicePlatformID, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_PLATFORM: \t\t\t\t0x%08x\n", devicePlatformID);

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_VERSION, PRINT_BUFFER_SIZE, printBuffer, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_VERSION: \t\t\t\t%s\n", printBuffer);

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_PROFILE, PRINT_BUFFER_SIZE, printBuffer, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_PROFILE: \t\t\t\t%s\n", printBuffer);

            errNum = clGetDeviceInfo(deviceID[j], CL_DRIVER_VERSION, PRINT_BUFFER_SIZE, printBuffer, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DRIVER_VERSION: \t\t\t\t%s\n", printBuffer);

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(compute_units), &compute_units, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_MAX_COMPUTE_UNITS: \t\t\t%d\n", compute_units);

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(workItemDims), &workItemDims, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS: \t\t%u\n", workItemDims);

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(workItemSize), &workItemSize, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            for (x=0; x < workItemDims; x++) {
                printf("\t\t\t CL_DEVICE_MAX_WORK_ITEM_SIZES[%u]: \t%u\n", x, workItemSize[x]);
            }

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(workgroupSize), &workgroupSize, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_MAX_WORK_GROUP_SIZE: \t\t%u\n", workgroupSize);

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(clockFrequency), &clockFrequency, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_MAX_CLOCK_FREQUENCY: \t\t%u MHz\n", clockFrequency);

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_IMAGE_SUPPORT, sizeof(imageSupport), &imageSupport, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_IMAGE_SUPPORT: \t\t\t%s\n", imageSupport ? "Yes" : "No");

            if (imageSupport > 0) {
                errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_MAX_READ_IMAGE_ARGS, sizeof(maxReadImageArgs), &maxReadImageArgs, NULL);
                clmCHECKERROR(errNum, CL_SUCCESS);
                printf("\t\t\t CL_DEVICE_MAX_READ_IMAGE_ARGS: \t%u\n", maxReadImageArgs);

                errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_MAX_WRITE_IMAGE_ARGS, sizeof(maxWriteImageArgs), &maxWriteImageArgs, NULL);
                clmCHECKERROR(errNum, CL_SUCCESS);
                printf("\t\t\t CL_DEVICE_MAX_WRITE_IMAGE_ARGS: \t%u\n", maxWriteImageArgs);

                errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_IMAGE2D_MAX_WIDTH, sizeof(image2DMaxWidth), &image2DMaxWidth, NULL);
                clmCHECKERROR(errNum, CL_SUCCESS);
                printf("\t\t\t CL_DEVICE_IMAGE2D_MAX_WIDTH: \t\t%u\n", image2DMaxWidth);

                errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_IMAGE2D_MAX_HEIGHT, sizeof(image2DMaxHeight), &image2DMaxHeight, NULL);
                clmCHECKERROR(errNum, CL_SUCCESS);
                printf("\t\t\t CL_DEVICE_IMAGE2D_MAX_HEIGHT: \t\t%u\n", image2DMaxHeight);

                errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_IMAGE3D_MAX_WIDTH, sizeof(image3DMaxWidth), &image3DMaxWidth, NULL);
                clmCHECKERROR(errNum, CL_SUCCESS);
                printf("\t\t\t CL_DEVICE_IMAGE3D_MAX_WIDTH: \t\t%u\n", image3DMaxWidth);

                errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_IMAGE3D_MAX_HEIGHT, sizeof(image3DMaxHeight), &image3DMaxHeight, NULL);
                clmCHECKERROR(errNum, CL_SUCCESS);
                printf("\t\t\t CL_DEVICE_IMAGE3D_MAX_HEIGHT: \t\t%u\n", image3DMaxHeight);

                errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_IMAGE3D_MAX_DEPTH, sizeof(image3DMaxDepth), &image3DMaxDepth, NULL);
                printf("\t\t\t CL_DEVICE_IMAGE3D_MAX_DEPTH: \t\t%u\n", image3DMaxDepth);

                errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_MAX_SAMPLERS, sizeof(maxSamplers), &maxSamplers, NULL);
                clmCHECKERROR(errNum, CL_SUCCESS);
                printf("\t\t\t CL_DEVICE_MAX_SAMPLERS: \t\t%u\n", maxSamplers);
            }

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_EXTENSIONS, PRINT_BUFFER_SIZE, &printBuffer, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\n\t\t CL_DEVICE_EXTENSIONS: \t ");

            if (*printBuffer != '\0'){
                for (p=printBuffer; *p!='\0'; p++) {
                    putchar(*p);
                    if (*p==' ') printf("\n\t\t\t\t\t ");
                }
            } else {
                printf("None");
            }
            printf("\n\n");

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, sizeof(vectorWidthChar), &vectorWidthChar, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR: \t%u\n", vectorWidthChar);

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, sizeof(vectorWidthShort), &vectorWidthShort, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT: \t%u\n", vectorWidthShort);

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, sizeof(vectorWidthInt), &vectorWidthInt, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT: \t\t%u\n", vectorWidthInt);

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, sizeof(vectorWidthLong), &vectorWidthLong, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG: \t%u\n", vectorWidthLong);

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, sizeof(vectorWidthFloat), &vectorWidthFloat, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT: \t%u\n", vectorWidthFloat);

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, sizeof(vectorWidthDouble), &vectorWidthDouble, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE: \t%u\n", vectorWidthDouble);

#ifndef NVIDIA_OPENCL_1_0
            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR, sizeof(vectorWidthChar), &vectorWidthChar, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR: \t\t%u\n", vectorWidthChar);

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT, sizeof(vectorWidthShort), &vectorWidthShort, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT: \t\t%u\n", vectorWidthShort);

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_NATIVE_VECTOR_WIDTH_INT, sizeof(vectorWidthInt), &vectorWidthInt, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_NATIVE_VECTOR_WIDTH_INT: \t\t%u\n", vectorWidthInt);

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG, sizeof(vectorWidthLong), &vectorWidthLong, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG: \t\t%u\n", vectorWidthLong);

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT, sizeof(vectorWidthFloat), &vectorWidthFloat, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT: \t\t%u\n", vectorWidthFloat);

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE, sizeof(vectorWidthDouble), &vectorWidthDouble, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE: \t\t%u\n", vectorWidthDouble);
#endif
            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_MAX_PARAMETER_SIZE, sizeof(maxParameterSize), &maxParameterSize, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_MAX_PARAMETER_SIZE: \t\t\t%u\n", maxParameterSize);

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_MEM_BASE_ADDR_ALIGN, sizeof(memBaseAddrAlign), &memBaseAddrAlign, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_MEM_BASE_ADDR_ALIGN: \t\t%u\n", memBaseAddrAlign);

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE, sizeof(minDataTypeAlignSize), &minDataTypeAlignSize, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE: \t\t%u\n", minDataTypeAlignSize);

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_SINGLE_FP_CONFIG, sizeof(singleFpConfig), &singleFpConfig, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_SINGLE_FP_CONFIG: \n");
            printf("\t\t\t CL_FP_DENORM: \t\t\t\t%s\n", singleFpConfig & CL_FP_DENORM ? "Yes" : "No");
            printf("\t\t\t CL_FP_INF_NAN: \t\t\t%s\n", singleFpConfig & CL_FP_INF_NAN ? "Yes" : "No");
            printf("\t\t\t CL_FP_ROUND_TO_NEAREST: \t\t%s\n", singleFpConfig & CL_FP_ROUND_TO_NEAREST ? "Yes" : "No");
            printf("\t\t\t CL_FP_ROUND_TO_ZERO: \t\t\t%s\n", singleFpConfig & CL_FP_ROUND_TO_ZERO ? "Yes" : "No");
            printf("\t\t\t CL_FP_ROUND_TO_INF: \t\t\t%s\n", singleFpConfig & CL_FP_ROUND_TO_INF ? "Yes" : "No");
            printf("\t\t\t CL_FP_FMA: \t\t\t\t%s\n", singleFpConfig & CL_FP_FMA ? "Yes" : "No");
            printf("\t\t\t CL_FP_SOFT_FLOAT: \t\t\t%s\n", singleFpConfig & CL_FP_SOFT_FLOAT ? "Yes" : "No");

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_ADDRESS_BITS, sizeof(addrBits), &addrBits, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_ADDRESS_BITS: \t\t\t%u\n", addrBits);

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(globalMemSize), &globalMemSize, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_GLOBAL_MEM_SIZE: \t\t\t%u MByte\n", (unsigned int)(globalMemSize / (1024 * 1024)));

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(maxMemAllocSize), &maxMemAllocSize, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_MAX_MEM_ALLOC_SIZE: \t\t\t%u MByte\n", (unsigned int)(maxMemAllocSize / (1024 * 1024)));

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, sizeof(globalMemCacheType), &globalMemCacheType, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_GLOBAL_MEM_CACHE_TYPE: \t\t");
            switch (globalMemCacheType) {
            case CL_NONE:
                printf("None\n");
                break;
            case CL_READ_ONLY_CACHE:
                printf("Read Only\n");
                break;
            case CL_READ_WRITE_CACHE:
                printf("Read/Write\n");
                break;
            }

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, sizeof(globalMemCachelineSize), &globalMemCachelineSize, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE: \t\t%u\n", globalMemCachelineSize);

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, sizeof(globalMemCacheSize), &globalMemCacheSize, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_GLOBAL_MEM_CACHE_SIZE: \t\t%u\n", (unsigned int)globalMemCacheSize);

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_LOCAL_MEM_SIZE, sizeof(globalMemSize), &globalMemSize, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_LOCAL_MEM_SIZE: \t\t\t%u KByte\n", (unsigned int)(globalMemSize / 1024));

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_LOCAL_MEM_TYPE, sizeof(localMemType), &localMemType, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_LOCAL_MEM_TYPE: \t\t\t");
            switch (localMemType) {
            case CL_LOCAL:
                printf("Local\n");
                break;
            case CL_GLOBAL:
                printf("Global\n");
                break;
            }

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(globalMemSize), &globalMemSize, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE: \t\t%u KByte\n", (unsigned int)(globalMemSize / 1024));

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_MAX_CONSTANT_ARGS, sizeof(maxConstantArgs), &maxConstantArgs, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_MAX_CONSTANT_ARGS: \t\t\t%u\n", maxConstantArgs);

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_ERROR_CORRECTION_SUPPORT, sizeof(errorCorrectionSupport), &errorCorrectionSupport, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_ERROR_CORRECTION_SUPPORT: \t\t%s\n", errorCorrectionSupport == CL_TRUE ? "Yes" : "No");

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_QUEUE_PROPERTIES, sizeof(queueProperties), &queueProperties, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_QUEUE_PROPERTIES:\n");
            printf("\t\t\t CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE:%s\n", queueProperties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE ? "Yes" : "No");
            printf("\t\t\t CL_QUEUE_PROFILING_ENABLE:             %s\n", queueProperties & CL_QUEUE_PROFILING_ENABLE ? "Yes" : "No");

#ifndef NVIDIA_OPENCL_1_0
            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_HOST_UNIFIED_MEMORY, sizeof(hostUnifiedMemory), &hostUnifiedMemory, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_HOST_UNIFIED_MEMORY: \t\t%s\n", hostUnifiedMemory == CL_TRUE ? "Yes" : "No");
#endif
            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_PROFILING_TIMER_RESOLUTION, sizeof(profilingTimingRes), &profilingTimingRes, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_PROFILING_TIMER_RESOLUTION: \t\t%u\n", profilingTimingRes);

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_ENDIAN_LITTLE, sizeof(endianLittle), &endianLittle, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_ENDIAN_LITTLE: \t\t\t%s\n", endianLittle == CL_TRUE ? "Yes" : "No");

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_AVAILABLE, sizeof(deviceAvail), &deviceAvail, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_AVAILABLE: \t\t\t\t%s\n", deviceAvail == CL_TRUE ? "Yes" : "No");

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_COMPILER_AVAILABLE, sizeof(compilerAvail), &compilerAvail, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_COMPILER_AVAILABLE: \t\t\t%s\n", compilerAvail == CL_TRUE ? "Yes" : "No");

            errNum = clGetDeviceInfo(deviceID[j], CL_DEVICE_EXECUTION_CAPABILITIES, sizeof(execCapability), &execCapability, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_DEVICE_EXECUTION_CAPABILITIES: \n");
            printf("\t\t\t CL_EXEC_KERNEL: \t\t\t%s\n", execCapability & CL_EXEC_KERNEL ? "Yes" : "No");
            printf("\t\t\t CL_EXEC_NATIVE_KERNEL: \t\t%s\n", execCapability & CL_EXEC_NATIVE_KERNEL ? "Yes" : "No");


            // ******************** clGetContextInfo ********************

            printf("\n>>>>>>>> Creating CLInfo context...\n");

            cl_context_properties    ctxProperties[3];
            cl_uint                    ctxRefCnt;
            cl_uint                    ctxNumDevices = 1;
            cl_device_id            *ctxDevices;

            ctxProperties[0] = CL_CONTEXT_PLATFORM;
            ctxProperties[1] = (cl_context_properties) platformID[i];
            ctxProperties[2] = 0;

            context = clCreateContext(ctxProperties, 1, &deviceID[j], NULL, NULL, &errNum);
            clmCHECKERROR(errNum, CL_SUCCESS);

            printf("\n\n\t Context Properties: \n");
            printf("\t Context Ptr:        0x%08x\n", context);

            errNum = clGetContextInfo(context, CL_CONTEXT_REFERENCE_COUNT, sizeof(ctxRefCnt), &ctxRefCnt, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_CONTEXT_REFERENCE_COUNT: \t\t\t%d\n", ctxRefCnt);

#ifndef NVIDIA_OPENCL_1_0
            errNum = clGetContextInfo(context, CL_CONTEXT_NUM_DEVICES, sizeof(ctxNumDevices), &ctxNumDevices, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_CONTEXT_NUM_DEVICES: \t\t\t%d\n", ctxNumDevices);
#endif
            ctxDevices = (cl_device_id *) malloc(sizeof(cl_device_id) * ctxNumDevices);

            errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, sizeof(cl_device_id) * ctxNumDevices, &ctxDevices, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_CONTEXT_DEVICES: \t\t\t\t0x%08x\n", ctxDevices);

            errNum = clGetContextInfo(context, CL_CONTEXT_PROPERTIES, sizeof(ctxProperties), ctxProperties, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_CONTEXT_PROPERTIES: \t\t\t");
            printf("0x%08x\n\t\t\t\t\t\t\t\t", ctxProperties[0]);
            printf("0x%08x\n\t\t\t\t\t\t\t\t", ctxProperties[1]);
            printf("0x%08x\n\n", ctxProperties[2]);

            // ******************** clGetCommandQueueInfo ********************

            printf("\n>>>>>>>> Creating CLInfo command queue...\n");

            cl_context                cmdQCtx;
            cl_device_id            cmdQDevice;
            cl_uint                    cmdQRefCnt;
            cl_command_queue_properties cmdQProperties = CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE |
                                                         CL_QUEUE_PROFILING_ENABLE;

            commandQueue = clCreateCommandQueue(context, deviceID[j], cmdQProperties, &errNum);
            clmCHECKERROR(errNum, CL_SUCCESS);

            printf("\n\n\t Command Queue Properties: \n");

            errNum = clGetCommandQueueInfo(commandQueue, CL_QUEUE_CONTEXT, sizeof(cmdQCtx), &cmdQCtx, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_QUEUE_CONTEXT: \t\t\t\t0x%08x\n", cmdQCtx);

            errNum = clGetCommandQueueInfo(commandQueue, CL_QUEUE_DEVICE, sizeof(cmdQDevice), &cmdQDevice, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_QUEUE_DEVICE: \t\t\t\t0x%08x\n", cmdQDevice);

            errNum = clGetCommandQueueInfo(commandQueue, CL_QUEUE_REFERENCE_COUNT, sizeof(cmdQRefCnt), &cmdQRefCnt, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_QUEUE_REFERENCE_COUNT: \t\t\t%d\n", cmdQRefCnt);

            errNum = clGetCommandQueueInfo(commandQueue, CL_QUEUE_PROPERTIES, sizeof(cmdQProperties), &cmdQProperties, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_QUEUE_PROPERTIES: \n", cmdQProperties);
            printf("\t\t\t CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE:%s\n", cmdQProperties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE ? "Yes" : "No");
            printf("\t\t\t CL_QUEUE_PROFILING_ENABLE:             %s\n", cmdQProperties & CL_QUEUE_PROFILING_ENABLE ? "Yes" : "No");

            // ******************** clGetProgramInfo ********************

            cl_context                pgmCtx;
            cl_uint                    pgmNumDevices;
            cl_device_id            *pgmDevices;
            cl_uint                    pgmRefCnt;
            char                    *pgmSrc;
            size_t                    pgmSrcSize, pgmBinariesSize = 0;
            size_t                    *pgmBinarySizes;
            unsigned char            **pgmBinaries;

            printf("\n>>>>>>>> Creating CLInfo program...\n");
            size_t sourceLength = strlen(helloSource);
            program = clCreateProgramWithSource(context,
                                                1,
                                                (const char **)&helloSource,
                                                &sourceLength,
                                                &errNum);
            clmCHECKERROR(errNum, CL_SUCCESS);

            printf("\n>>>>>>>> Building CLInfo program...\n");
            errNum = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);

            printf("\n\n\t Program Properties: \n");

            errNum = clGetProgramInfo(program, CL_PROGRAM_CONTEXT, sizeof(pgmCtx), &pgmCtx, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_PROGRAM_CONTEXT: \t\t\t\t0x%08x\n", pgmCtx);

            errNum = clGetProgramInfo(program, CL_PROGRAM_REFERENCE_COUNT, sizeof(pgmRefCnt), &pgmRefCnt, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_PROGRAM_REFERENCE_COUNT: \t\t\t%d\n", pgmRefCnt);

            errNum = clGetProgramInfo(program, CL_PROGRAM_NUM_DEVICES, sizeof(pgmNumDevices), &pgmNumDevices, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_PROGRAM_NUM_DEVICES: \t\t\t%d\n", pgmNumDevices);

            pgmDevices = (cl_device_id *) malloc(sizeof(cl_device_id) * pgmNumDevices);
            errNum = clGetProgramInfo(program, CL_PROGRAM_DEVICES, sizeof(cl_device_id) * pgmNumDevices, &pgmDevices, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_PROGRAM_DEVICES: \t\t\t\t0x%08x\n", pgmDevices);

            errNum = clGetProgramInfo(program, CL_PROGRAM_SOURCE, NULL, NULL, &pgmSrcSize);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_PROGRAM_SOURCE: \t\t\t\t(Size:%d)\n", pgmSrcSize);
            pgmSrc = (char *) malloc(sizeof(char) * pgmSrcSize);
            errNum = clGetProgramInfo(program, CL_PROGRAM_SOURCE, pgmSrcSize, pgmSrc, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\n------------------ BEGIN --------------------\n");
            printf("%s", pgmSrc);
            printf("\n------------------- END ---------------------\n\n");

            pgmBinarySizes = (size_t *) malloc(sizeof(size_t) * pgmNumDevices);

            errNum = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, sizeof(size_t) * pgmNumDevices, pgmBinarySizes, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            for (x=0; x < pgmNumDevices; x++) {
                printf("\t\t CL_PROGRAM_BINARY_SIZES[%d]: \t\t\t%d\n", x, pgmBinarySizes[x]);
            }

            pgmBinaries = (unsigned char **) malloc(sizeof(unsigned char *) * pgmNumDevices);
            for (x=0; x < pgmNumDevices; x++) {
                pgmBinaries[x] = (unsigned char *) malloc(sizeof(unsigned char) * pgmBinarySizes[x]);
                pgmBinariesSize += sizeof(unsigned char) * pgmBinarySizes[x];
            }

            errNum = clGetProgramInfo(program, CL_PROGRAM_BINARIES, pgmBinariesSize, pgmBinaries, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);

#ifdef PRINT_PROGRAM_BINARY
            printf("\t\t CL_PROGRAM_BINARIES: \n");
            for (x=0; x < pgmNumDevices; x++) {
                printf("\t\t\t Device Number %d:", x);
                if (pgmBinarySizes[x] > 1) {
                printf("\n\n------------------ BEGIN --------------------\n");
                if (pgmBinarySizes[x] != 0) {
                    for (k=0; k<pgmBinarySizes[x]; k++) {
                        putchar(PRINTABLE(pgmBinaries[x][k]));
                    }
                }
                printf("\n------------------- END ---------------------\n\n");
                } else {
                    printf("\t\t\t\"\"\n");
                }
            }
#endif

            // ******************** clGetProgramBuildInfo ********************

            cl_build_status            pgmBuildStatus;
            char                    *pgmBuildOpts;
            size_t                    pgmBuildOptsSize;
            char                    *pgmBuildLog;
            size_t                    pgmBuildLogSize;

            printf("\n\t Program Build Properties: \n");

            errNum = clGetProgramBuildInfo(program, deviceID[j], CL_PROGRAM_BUILD_STATUS, sizeof(pgmBuildStatus), &pgmBuildStatus, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_PROGRAM_BUILD_STATUS: \t\t\t%d\n", pgmBuildStatus);

            errNum = clGetProgramBuildInfo(program, deviceID[j], CL_PROGRAM_BUILD_OPTIONS, NULL, NULL, &pgmBuildOptsSize);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_PROGRAM_BUILD_OPTIONS: \t\t\t\"");
            pgmBuildOpts = (char *) malloc(sizeof(char) * pgmBuildOptsSize);
            errNum = clGetProgramBuildInfo(program, deviceID[j], CL_PROGRAM_BUILD_OPTIONS, pgmBuildOptsSize, pgmBuildOpts, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("%s\"\n", pgmBuildOpts);

            errNum = clGetProgramBuildInfo(program, deviceID[j], CL_PROGRAM_BUILD_LOG, NULL, NULL, &pgmBuildLogSize);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_PROGRAM_BUILD_LOG:");

            if (pgmBuildLogSize > 1) {
                pgmBuildLog = (char *) malloc(sizeof(char) * pgmBuildLogSize);
                errNum = clGetProgramBuildInfo(program, deviceID[j], CL_PROGRAM_BUILD_LOG, pgmBuildLogSize, pgmBuildLog, NULL);
                clmCHECKERROR(errNum, CL_SUCCESS);
                printf("\n\n------------------ BEGIN --------------------\n");
                printf("%s", pgmBuildLog);
                printf("\n------------------- END ---------------------\n\n");
            } else {
                printf("\t\t\t\t\"\"\n");
            }

            // ******************** clGetKernelInfo ********************

            char                    *kernelFuncName;
            size_t                    kernelFuncNameSize;
            cl_context                kernelCtx;
            cl_uint                    kernelNumArgs;
            cl_uint                    kernelRefCnt;
            cl_program                kernelPgm;

            printf("\n>>>>>>>> Creating CLInfo kernel...\n");
            kernel = clCreateKernel(program, "hello", &errNum);
            clmCHECKERROR(errNum, CL_SUCCESS);

            printf("\n\n\t Kernel Properties: \n");

            errNum = clGetKernelInfo(kernel, CL_KERNEL_FUNCTION_NAME, NULL, NULL, &kernelFuncNameSize);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_KERNEL_FUNCTION_NAME: \t\t\t\"");
            kernelFuncName = (char *) malloc(sizeof(char) * kernelFuncNameSize);
            errNum = clGetKernelInfo(kernel, CL_KERNEL_FUNCTION_NAME, kernelFuncNameSize, kernelFuncName, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("%s\"\n", kernelFuncName);

            errNum = clGetKernelInfo(kernel, CL_KERNEL_CONTEXT, sizeof(kernelCtx), &kernelCtx, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_KERNEL_CONTEXT: \t\t\t\t0x%08x\n", kernelCtx);

            errNum = clGetKernelInfo(kernel, CL_KERNEL_PROGRAM, sizeof(kernelPgm), &kernelPgm, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_KERNEL_PROGRAM: \t\t\t\t0x%08x\n", kernelPgm);

            errNum = clGetKernelInfo(kernel, CL_KERNEL_NUM_ARGS, sizeof(kernelNumArgs), &kernelNumArgs, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_KERNEL_NUM_ARGS: \t\t\t\t%d\n", kernelNumArgs);

            errNum = clGetKernelInfo(kernel, CL_KERNEL_REFERENCE_COUNT, sizeof(kernelRefCnt), &kernelRefCnt, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_KERNEL_REFERENCE_COUNT: \t\t\t%d\n", kernelRefCnt);

            // ******************** clGetKernelWorkGroupInfo ********************

            size_t                    kernelWorkGrpSize;
            size_t                    kernelCompileWrkGrpSize[3];
            cl_ulong                kernelLocalMemSize;
#ifndef NVIDIA_OPENCL_1_0
            size_t                    kernelPrefWorkGroupSize;
            cl_ulong                kernelPrivMemSize;
#endif
            printf("\n\n\t Kernel Workgroup Properties: \n");

            errNum = clGetKernelWorkGroupInfo(kernel, deviceID[j], CL_KERNEL_WORK_GROUP_SIZE, sizeof(kernelWorkGrpSize), &kernelWorkGrpSize, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_KERNEL_WORK_GROUP_SIZE: \t\t\t%u\n", kernelWorkGrpSize);

#ifndef NVIDIA_OPENCL_1_0
            errNum = clGetKernelWorkGroupInfo(kernel, deviceID[j], CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, sizeof(kernelPrefWorkGroupSize), &kernelPrefWorkGroupSize, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE: \t%u\n", kernelPrefWorkGroupSize);
#endif
            errNum = clGetKernelWorkGroupInfo(kernel, deviceID[j], CL_KERNEL_COMPILE_WORK_GROUP_SIZE, sizeof(kernelCompileWrkGrpSize), &kernelCompileWrkGrpSize, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_KERNEL_COMPILE_WORK_GROUP_SIZE: \t\t");
            printf("%u\n\t\t\t\t\t\t\t\t", kernelCompileWrkGrpSize[0]);
            printf("%u\n\t\t\t\t\t\t\t\t", kernelCompileWrkGrpSize[1]);
            printf("%u\n", kernelCompileWrkGrpSize[2]);

            errNum = clGetKernelWorkGroupInfo(kernel, deviceID[j], CL_KERNEL_LOCAL_MEM_SIZE, sizeof(kernelLocalMemSize), &kernelLocalMemSize, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_KERNEL_LOCAL_MEM_SIZE: \t\t\t%u\n", (unsigned int)kernelLocalMemSize);

#ifndef NVIDIA_OPENCL_1_0
            errNum = clGetKernelWorkGroupInfo(kernel, deviceID[j], CL_KERNEL_PRIVATE_MEM_SIZE, sizeof(kernelPrivMemSize), &kernelPrivMemSize, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            printf("\t\t CL_KERNEL_PRIVATE_MEM_SIZE: \t\t\t%u\n", (unsigned int)kernelPrivMemSize);
#endif
        }
    }

    printf("\n>>>>>>>> Releasing CLInfo kernel...\n");
    errNum  = clReleaseKernel(kernel);
    clmCHECKERROR(errNum, CL_SUCCESS);

    printf("\n>>>>>>>> Releasing CLInfo program...\n");
    errNum = clReleaseProgram(program);
    clmCHECKERROR(errNum, CL_SUCCESS);

    printf("\n>>>>>>>> Releasing CLInfo command queue...\n");
    errNum = clReleaseCommandQueue(commandQueue);
    clmCHECKERROR(errNum, CL_SUCCESS);

    printf("\n>>>>>>>> Releasing CLInfo context...\n");
    errNum = clReleaseContext(context);
    clmCHECKERROR(errNum, CL_SUCCESS);


    fflush(stderr);
    printf("\n>>>>>>>> Exiting...\n");
}
