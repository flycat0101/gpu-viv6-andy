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


#if !defined(_WIN32)
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#if !defined(_WIN32)
#include <stdbool.h>
#endif

#include "procs.h"

const char    *get_error_string( int clErrorCode )
{
    switch( clErrorCode )
    {
    case CL_SUCCESS:                return "CL_SUCCESS";
    case CL_DEVICE_NOT_FOUND:        return "CL_DEVICE_NOT_FOUND";
    case CL_DEVICE_NOT_AVAILABLE:    return "CL_DEVICE_NOT_AVAILABLE";
    case CL_COMPILER_NOT_AVAILABLE:    return "CL_COMPILER_NOT_AVAILABLE";
    case CL_MEM_OBJECT_ALLOCATION_FAILURE:    return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
    case CL_OUT_OF_RESOURCES:        return "CL_OUT_OF_RESOURCES";
    case CL_OUT_OF_HOST_MEMORY:        return "CL_OUT_OF_HOST_MEMORY";
    case CL_PROFILING_INFO_NOT_AVAILABLE: return "CL_PROFILING_INFO_NOT_AVAILABLE";
    case CL_MEM_COPY_OVERLAP:        return "CL_MEM_COPY_OVERLAP";
    case CL_IMAGE_FORMAT_MISMATCH:    return "CL_IMAGE_FORMAT_MISMATCH";
    case CL_IMAGE_FORMAT_NOT_SUPPORTED:    return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
    case CL_BUILD_PROGRAM_FAILURE: return "CL_BUILD_PROGRAM_FAILURE";
    case CL_INVALID_VALUE:            return "CL_INVALID_VALUE";
    case CL_INVALID_DEVICE_TYPE: return "CL_INVALID_DEVICE_TYPE";
    case CL_INVALID_DEVICE:            return "CL_INVALID_DEVICE";
    case CL_INVALID_CONTEXT:        return "CL_INVALID_CONTEXT";
    case CL_INVALID_QUEUE_PROPERTIES:    return "CL_INVALID_QUEUE_PROPERTIES";
    case CL_INVALID_COMMAND_QUEUE:    return "CL_INVALID_COMMAND_QUEUE";
    case CL_INVALID_HOST_PTR:    return "CL_INVALID_HOST_PTR";
    case CL_INVALID_MEM_OBJECT:        return "CL_INVALID_MEM_OBJECT";
    case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:        return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
    case CL_INVALID_IMAGE_SIZE:        return "CL_INVALID_IMAGE_SIZE";
    case CL_INVALID_SAMPLER:        return "CL_INVALID_SAMPLER";
    case CL_INVALID_BINARY:        return "CL_INVALID_BINARY";
    case CL_INVALID_BUILD_OPTIONS:        return "CL_INVALID_BUILD_OPTIONS";
    case CL_INVALID_PROGRAM:        return "CL_INVALID_PROGRAM";
    case CL_INVALID_PROGRAM_EXECUTABLE:        return "CL_INVALID_PROGRAM_EXECUTABLE";
    case CL_INVALID_KERNEL_NAME:    return "CL_INVALID_KERNEL_NAME";
    case CL_INVALID_KERNEL_DEFINITION:    return "CL_INVALID_KERNEL_DEFINITION";
    case CL_INVALID_KERNEL:            return "CL_INVALID_KERNEL";
    case CL_INVALID_ARG_INDEX:        return "CL_INVALID_ARG_INDEX";
    case CL_INVALID_ARG_VALUE:        return "CL_INVALID_ARG_VALUE";
    case CL_INVALID_ARG_SIZE:        return "CL_INVALID_ARG_SIZE";
    case CL_INVALID_KERNEL_ARGS:    return "CL_INVALID_KERNEL_ARGS";
    case CL_INVALID_WORK_DIMENSION:        return "CL_INVALID_WORK_DIMENSION";
    case CL_INVALID_WORK_GROUP_SIZE:    return "CL_INVALID_WORK_GROUP_SIZE";
    case CL_INVALID_WORK_ITEM_SIZE:    return "CL_INVALID_WORK_ITEM_SIZE";
    case CL_INVALID_GLOBAL_OFFSET:        return "CL_INVALID_GLOBAL_OFFSET";
    case CL_INVALID_EVENT_WAIT_LIST:    return "CL_INVALID_EVENT_WAIT_LIST";
    case CL_INVALID_EVENT:            return "CL_INVALID_EVENT";
    case CL_INVALID_OPERATION:        return "CL_INVALID_OPERATION";
    case CL_INVALID_GL_OBJECT:        return "CL_INVALID_GL_OBJECT";
    case CL_INVALID_BUFFER_SIZE:    return "CL_INVALID_BUFFER_SIZE";
    default: return "(unknown)";
    }
}

int create_kernel( cl_context context, cl_program *outProgram, cl_kernel *outKernel, unsigned int numKernelLines, const char **kernelProgram, const char *kernelName )
{
    int error = CL_SUCCESS;

    /* Create the program object from source */
    *outProgram = clCreateProgramWithSource( context, numKernelLines, kernelProgram, NULL, &error );
    if( *outProgram == NULL || error != CL_SUCCESS)
    {
        print_error( error, "clCreateProgramWithSource failed" );
        return error;
    }

    /* Compile the program */
    int buildProgramFailed = 0;
    int printedSource = 0;
    error = clBuildProgram( *outProgram, 0, NULL, NULL, NULL, NULL );
    if (error != CL_SUCCESS)
    {
        unsigned int i;
        print_error(error, "clBuildProgram failed");
        buildProgramFailed = 1;
        printedSource = 1;
        printf( "Original source is: ------------\n" );
        for( i = 0; i < numKernelLines; i++ )
            printf( "%s", kernelProgram[ i ] );
    }

    // Verify the build status on all devices
    cl_uint deviceCount = 0;
    error = clGetProgramInfo( *outProgram, CL_PROGRAM_NUM_DEVICES, sizeof( deviceCount ), &deviceCount, NULL );
    if (error != CL_SUCCESS) {
        print_error(error, "clGetProgramInfo CL_PROGRAM_NUM_DEVICES failed");
        return error;
    }

    if (deviceCount == 0) {
        printf("No devices found for program.\n");
        return -1;
    }

    cl_device_id    *devices = (cl_device_id*) malloc( deviceCount * sizeof( cl_device_id ) );
    if( NULL == devices )
        return -1;
    memset( devices, 0, deviceCount * sizeof( cl_device_id ));
    error = clGetProgramInfo( *outProgram, CL_PROGRAM_DEVICES, sizeof( cl_device_id ) * deviceCount, devices, NULL );
    if (error != CL_SUCCESS) {
        print_error(error, "clGetProgramInfo CL_PROGRAM_DEVICES failed");
        free( devices );
        return error;
    }

    for(cl_uint z = 0; z < deviceCount; z++ )
    {
        char deviceName[4096] = "";
        error = clGetDeviceInfo(devices[z], CL_DEVICE_NAME, sizeof( deviceName), deviceName, NULL);
        if (error != CL_SUCCESS || deviceName[0] == '\0') {
            printf("Device \"%d\" failed to return a name\n", z);
            print_error(error, "clGetDeviceInfo CL_DEVICE_NAME failed");
        }

        cl_build_status buildStatus;
        error = clGetProgramBuildInfo(*outProgram, devices[z], CL_PROGRAM_BUILD_STATUS, sizeof(buildStatus), &buildStatus, NULL);
        if (error != CL_SUCCESS) {
            print_error(error, "clGetProgramBuildInfo CL_PROGRAM_BUILD_STATUS failed");
            free( devices );
            return error;
        }

        if (buildStatus != CL_BUILD_SUCCESS || buildProgramFailed) {
            char log[10240] = "";
            if (buildStatus == CL_BUILD_SUCCESS && buildProgramFailed) printf("clBuildProgram returned an error, but buildStatus is marked as CL_BUILD_SUCCESS.\n");

            char statusString[64] = "";
            if (buildStatus == (cl_build_status)CL_BUILD_SUCCESS)
                sprintf(statusString, "CL_BUILD_SUCCESS");
            else if (buildStatus == (cl_build_status)CL_BUILD_NONE)
                sprintf(statusString, "CL_BUILD_NONE");
            else if (buildStatus == (cl_build_status)CL_BUILD_ERROR)
                sprintf(statusString, "CL_BUILD_ERROR");
            else if (buildStatus == (cl_build_status)CL_BUILD_IN_PROGRESS)
                sprintf(statusString, "CL_BUILD_IN_PROGRESS");
            else
                sprintf(statusString, "UNKNOWN (%d)", buildStatus);

            if (buildStatus != CL_BUILD_SUCCESS) printf("Build not successful for device \"%s\", status: %s\n", deviceName, statusString);
            error = clGetProgramBuildInfo( *outProgram, devices[z], CL_PROGRAM_BUILD_LOG, sizeof(log), log, NULL );
            if (error != CL_SUCCESS || log[0]=='\0'){
                printf("Device %d (%s) failed to return a build log\n", z, deviceName);
                if (error) {
                    print_error(error, "clGetProgramBuildInfo CL_PROGRAM_BUILD_LOG failed");
                    free( devices );
                    return error;
                } else {
                    printf("clGetProgramBuildInfo returned an empty log.\n");
                    free( devices );
                    return -1;
                }
            }
            // In this case we've already printed out the code above.
            if (!printedSource)
            {
                unsigned int i;
                printf( "Original source is: ------------\n" );
                for( i = 0; i < numKernelLines; i++ )
                    printf( "%s", kernelProgram[ i ] );
                printedSource = 1;
            }
            printf( "Build log for device \"%s\" is: ------------\n", deviceName );
            printf( "%s\n", log );
            printf( "\n----------\n" );
            free( devices );
            return -1;
        }
    }

    /* And create a kernel from it */
    *outKernel = clCreateKernel( *outProgram, kernelName, &error );
    if( *outKernel == NULL || error != CL_SUCCESS)
    {
        print_error( error, "Unable to create kernel" );
        free( devices );
        return error;
    }

    free( devices );
    return 0;
}


int main(int argc, const char *argv[])
{
    int err = 0;
    int totalTests = 0;
    int passed = 0;
    int failed = 0;
    int     gFlushDenormsToZero = 0;
    int     gInfNanSupport = 1;
    int     gIsEmbedded = 0;
    int     gHasLong = 1;
    int     gHasHalf = 1;

    cl_device_type    device_type = CL_DEVICE_TYPE_DEFAULT;
    cl_platform_id     platform;
    cl_device_id       device;
    cl_uint            num_devices = 0;
    cl_device_id       *devices = NULL;
    cl_uint            choosen_device_index = 0;

    /* Get the platform */
    err = clGetPlatformIDs(1, &platform, NULL);
    if (err) {
        print_error(err, "clGetPlatformIDs failed");
        return -1;
    }

    /* Get the number of requested devices */
    err = clGetDeviceIDs(platform,  device_type, 0, NULL, &num_devices );
    if (err) {
        print_error(err, "clGetDeviceIDs failed");
        return -1;
    }

    devices = (cl_device_id *) malloc( num_devices * sizeof( cl_device_id ) );
    if (!devices || choosen_device_index >= num_devices) {
        printf( "device index out of range -- choosen_device_index (%d) >= num_devices (%d)\n", choosen_device_index, num_devices );

        return -1;
    }

    /* Get the requested device */
    err = clGetDeviceIDs(platform,  device_type, num_devices, devices, NULL );
    if (err) {
        print_error(err, "clGetDeviceIDs failed");

        return -1;
    }

    device = devices[choosen_device_index];
    free(devices);
    devices = NULL;


    cl_device_fp_config fpconfig = 0;
    err = clGetDeviceInfo( device, CL_DEVICE_SINGLE_FP_CONFIG, sizeof( fpconfig ), &fpconfig, NULL );
    if (err) {
        print_error(err, "clGetDeviceInfo for CL_DEVICE_SINGLE_FP_CONFIG failed");

        return -1;
    }

    gFlushDenormsToZero = ( 0 == (fpconfig & CL_FP_DENORM));
    //printf( "Supports single precision denormals: %s\n", gFlushDenormsToZero ? "NO" : "YES" );
    //printf( "sizeof( void*) = %d  (host)\n", (int) sizeof( void* ) );

    //detect whether profile of the device is embedded
    char profile[1024] = "";
    err = clGetDeviceInfo(device, CL_DEVICE_PROFILE, sizeof(profile), profile, NULL);
    if (err)
    {
        print_error(err, "clGetDeviceInfo for CL_DEVICE_PROFILE failed\n" );

        return -1;
    }
    gIsEmbedded = NULL != strstr(profile, "EMBEDDED_PROFILE");

    //detect the floating point capabilities
    cl_device_fp_config floatCapabilities = 0;
    err = clGetDeviceInfo(device, CL_DEVICE_SINGLE_FP_CONFIG, sizeof(floatCapabilities), &floatCapabilities, NULL);
    if (err)
    {
        print_error(err, "clGetDeviceInfo for CL_DEVICE_SINGLE_FP_CONFIG failed\n");

        return -1;
    }

    // Check for problems that only embedded will have
    if( gIsEmbedded )
    {
        //If the device is embedded, we need to detect if the device supports Infinity and NaN
        if ((floatCapabilities & CL_FP_INF_NAN) == 0)
            gInfNanSupport = 0;

        // check the extensions list to see if ulong and long are supported
        size_t extensionsStringSize = 0;
        if( (err = clGetDeviceInfo( device, CL_DEVICE_EXTENSIONS, 0, NULL, &extensionsStringSize ) ))
        {
            print_error( err, "Unable to get extensions string size for embedded device" );

            return -1;
        }
        char *extensions_string = (char*) malloc(extensionsStringSize);
        if( NULL == extensions_string )
        {
            print_error( CL_OUT_OF_HOST_MEMORY, "Unable to allocate storage for extensions string for embedded device" );

            return -1;
        }

        if( (err = clGetDeviceInfo( device, CL_DEVICE_EXTENSIONS, extensionsStringSize, extensions_string, NULL ) ))
        {
            print_error( err, "Unable to get extensions string for embedded device" );

            return -1;
        }

        if( extensions_string[extensionsStringSize-1] != '\0' )
        {
            printf( "FAILURE: extensions string for embedded device is not NULL terminated" );

            return -1;
        }

        if( NULL == strstr( extensions_string, "cles_khr_int64" ))
            gHasLong = 0;

        if( NULL == strstr( extensions_string, "cl_khr_fp16" ))
            gHasHalf = 0;

        free(extensions_string);
    }


    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err );
    if (!context)
    {
        print_error( err, "Unable to create testing context" );
        return 1;
    }

    cl_command_queue queue = clCreateCommandQueue( context, device, NULL, &err );
    if( queue == NULL )
    {
        print_error( err, "Unable to create testing command queue" );
        return 1;
    }

    //START THE TESTS
    printf("TESTS STARTED...\n");
    test_main_kernel2(device, context, queue, 1024, totalTests, passed, failed);
    test_main_kernel3(device, context, queue, 1024, totalTests, passed, failed);
    test_main_kernel4(device, context, queue, 1024, totalTests, passed, failed);
    printf("Total Tests: %d\nPassed: %d\nFailed:%d\n",totalTests, passed, failed);
    printf("TESTS ENDED\n");

    clReleaseCommandQueue( queue );
    clReleaseContext( context );

    return err;
}



