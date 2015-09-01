/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


/*
 * Copyright (c) 2009 The Khronos Group Inc.  All rights reserved.
 *
 * NOTICE TO KHRONOS MEMBER:
 *
 * NVIDIA has assigned the copyright for this object code to Khronos.
 * This object code is subject to Khronos ownership rights under U.S. and
 * international Copyright laws.
 *
 * Permission is hereby granted, free of charge, to any Khronos Member
 * obtaining a copy of this software and/or associated documentation files
 * (the "Materials"), to use, copy, modify and merge the Materials in object
 * form only and to publish, distribute and/or sell copies of the Materials
 * solely in object code form as part of conformant OpenCL API implementations,
 * subject to the following conditions:
 *
 * Khronos Members shall ensure that their respective ICD implementation,
 * that is installed over another Khronos Members' ICD implementation, will
 * continue to support all OpenCL devices (hardware and software) supported
 * by the replaced ICD implementation. For the purposes of this notice, "ICD"
 * shall mean a library that presents an implementation of the OpenCL API for
 * the purpose routing API calls to different vendor implementation.
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Materials.
 *
 * KHRONOS AND NVIDIA MAKE NO REPRESENTATION ABOUT THE SUITABILITY OF THIS
 * SOURCE CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR
 * IMPLIED WARRANTY OF ANY KIND.  KHRONOS AND NVIDIA DISCLAIM ALL WARRANTIES
 * WITH REGARD TO THIS SOURCE CODE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL KHRONOS OR NVIDIA BE LIABLE FOR ANY SPECIAL, INDIRECT,
 * INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH
 * THE USE OR PERFORMANCE OF THIS SOURCE CODE.
 *
 * U.S. Government End Users.   This source code is a "commercial item" as
 * that term is defined at 48 C.F.R. 2.101 (OCT 1995), consisting of
 * "commercial computer software" and "commercial computer software
 * documentation" as such terms are used in 48 C.F.R. 12.212 (SEPT 1995)
 * and is provided to the U.S. Government only as a commercial end item.
 * Consistent with 48 C.F.R.12.212 and 48 C.F.R. 227.7202-1 through
 * 227.7202-4 (JUNE 1995), all U.S. Government End Users acquire the
 * source code with only those rights set forth herein.
 */

#include "icd_dispatch.h"
#include "icd.h"
#include <stdlib.h>
#include <string.h>

/* Platform APIs */
CL_API_ENTRY cl_int CL_API_CALL
clGetPlatformIDs(cl_uint          num_entries,
                 cl_platform_id * platforms,
                 cl_uint *        num_platforms) CL_API_SUFFIX__VERSION_1_0
{
    KHRicdVendor* vendor = NULL;
    cl_uint i;

    /* initialize the platforms (in case they have not been already) */
    khrIcdInitialize();

    if (!num_entries && platforms)
    {
        return CL_INVALID_VALUE;
    }
    if (!platforms && !num_platforms)
    {
        return CL_INVALID_VALUE;
    }
    /* set num_platforms to 0 and set all platform pointers to NULL */
    if (num_platforms)
    {
        *num_platforms = 0;
    }
    for (i = 0; i < num_entries && platforms; ++i)
    {
        platforms[i] = NULL;
    }
    /* return error if we have no platforms */
    if (!khrIcdState.vendors)
    {
        return CL_PLATFORM_NOT_FOUND_KHR;
    }
    /* otherwise enumerate all platforms */
    for (vendor = khrIcdState.vendors; vendor; vendor = vendor->next)
    {
        if (num_entries && platforms)
        {
            *(platforms++) = vendor->platform;
            --num_entries;
        }
        if (num_platforms)
        {
            ++(*num_platforms);
        }
    }
    return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetPlatformInfo(cl_platform_id   platform,
                  cl_platform_info param_name,
                  size_t           param_value_size,
                  void *           param_value,
                  size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    /* initialize the platforms (in case they have not been already) */
    khrIcdInitialize();
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(platform, CL_INVALID_PLATFORM);
    return platform->dispatch->clGetPlatformInfo(
        platform,
        param_name,
        param_value_size,
        param_value,
        param_value_size_ret);
}

/* Device APIs */
CL_API_ENTRY cl_int CL_API_CALL
clGetDeviceIDs(cl_platform_id   platform,
               cl_device_type   device_type,
               cl_uint          num_entries,
               cl_device_id *   devices,
               cl_uint *        num_devices) CL_API_SUFFIX__VERSION_1_0
{
    /* initialize the platforms (in case they have not been already) */
    khrIcdInitialize();
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(platform, CL_INVALID_PLATFORM);
    return platform->dispatch->clGetDeviceIDs(
        platform,
        device_type,
        num_entries,
        devices,
        num_devices);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetDeviceInfo(
    cl_device_id    device,
    cl_device_info  param_name,
    size_t          param_value_size,
    void *          param_value,
    size_t *        param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(device, CL_INVALID_DEVICE);
    return device->dispatch->clGetDeviceInfo(
        device,
        param_name,
        param_value_size,
        param_value,
        param_value_size_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clCreateSubDevicesEXT(
    cl_device_id in_device,
    const cl_device_partition_property_ext * partition_properties,
    cl_uint num_entries,
    cl_device_id * out_devices,
    cl_uint * num_devices) CL_API_SUFFIX__VERSION_1_1
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(in_device, CL_INVALID_DEVICE);
    return in_device->dispatch->clCreateSubDevicesEXT(
        in_device,
        partition_properties,
        num_entries,
        out_devices,
        num_devices);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainDeviceEXT(cl_device_id device) CL_API_SUFFIX__VERSION_1_1
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(device, CL_INVALID_DEVICE);
    return device->dispatch->clRetainDeviceEXT(device);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseDeviceEXT(cl_device_id device) CL_API_SUFFIX__VERSION_1_1
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(device, CL_INVALID_DEVICE);
    return device->dispatch->clReleaseDeviceEXT(device);
}

/* Context APIs */
CL_API_ENTRY cl_context CL_API_CALL
clCreateContext(const cl_context_properties * properties,
                cl_uint                 num_devices,
                const cl_device_id *    devices,
                void (CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *),
                void *                  user_data,
                cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    /* initialize the platforms (in case they have not been already) */
    khrIcdInitialize();
    if (!num_devices)
    {
        if (errcode_ret)
        {
            *errcode_ret = CL_INVALID_VALUE;
        }
        return NULL;
    }
    KHR_ICD_VALIDATE_HANDLE_RETURN_HANDLE(devices[0], CL_INVALID_DEVICE);
    return devices[0]->dispatch->clCreateContext(
        properties,
        num_devices,
        devices,
        pfn_notify,
        user_data,
        errcode_ret);
}

CL_API_ENTRY cl_context CL_API_CALL
clCreateContextFromType(const cl_context_properties * properties,
                        cl_device_type          device_type,
                        void (CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *),
                        void *                  user_data,
                        cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    cl_platform_id platform = NULL;

    /* initialize the platforms (in case they have not been already) */
    khrIcdInitialize();

    /* determine the platform to use from the properties specified */
    khrIcdContextPropertiesGetPlatform(properties, &platform);

    if (!platform && khrIcdState.vendors)
    {
        platform = khrIcdState.vendors->platform;
    }
    /* validate the platform handle and dispatch */
    KHR_ICD_VALIDATE_HANDLE_RETURN_HANDLE(platform, CL_INVALID_PLATFORM);
    return platform->dispatch->clCreateContextFromType(
        properties,
        device_type,
        pfn_notify,
        user_data,
        errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainContext(cl_context context) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(context, CL_INVALID_CONTEXT);
    return context->dispatch->clRetainContext(context);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseContext(cl_context context) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(context, CL_INVALID_CONTEXT);
    return context->dispatch->clReleaseContext(context);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetContextInfo(cl_context         context,
                 cl_context_info    param_name,
                 size_t             param_value_size,
                 void *             param_value,
                 size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(context, CL_INVALID_CONTEXT);
    return context->dispatch->clGetContextInfo(
        context,
        param_name,
        param_value_size,
        param_value,
        param_value_size_ret);
}

/* Command Queue APIs */
CL_API_ENTRY cl_command_queue CL_API_CALL
clCreateCommandQueue(cl_context                     context,
                     cl_device_id                   device,
                     cl_command_queue_properties    properties,
                     cl_int *                       errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_HANDLE(context, CL_INVALID_CONTEXT);
    return context->dispatch->clCreateCommandQueue(
        context,
        device,
        properties,
        errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainCommandQueue(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(command_queue, CL_INVALID_COMMAND_QUEUE);
    return command_queue->dispatch->clRetainCommandQueue(command_queue);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseCommandQueue(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(command_queue, CL_INVALID_COMMAND_QUEUE);
    return command_queue->dispatch->clReleaseCommandQueue(command_queue);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetCommandQueueInfo(cl_command_queue      command_queue,
                      cl_command_queue_info param_name,
                      size_t                param_value_size,
                      void *                param_value,
                      size_t *              param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(command_queue, CL_INVALID_COMMAND_QUEUE);
    return command_queue->dispatch->clGetCommandQueueInfo(
        command_queue,
        param_name,
        param_value_size,
        param_value,
        param_value_size_ret);
}

#if defined(CL_VERSION_1_1) && !defined(CL_USE_DEPRECATED_OPENCL_1_0_APIS)
extern CL_API_ENTRY cl_int CL_API_CALL
clSetCommandQueueProperty(cl_command_queue              /* command_queue */,
                          cl_command_queue_properties   /* properties */,
                          cl_bool                        /* enable */,
                          cl_command_queue_properties * /* old_properties */) CL_EXT_SUFFIX__VERSION_1_0_DEPRECATED;
#endif

CL_API_ENTRY cl_int CL_API_CALL
clSetCommandQueueProperty(cl_command_queue              command_queue,
                          cl_command_queue_properties   properties,
                          cl_bool                        enable,
                          cl_command_queue_properties * old_properties) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(command_queue, CL_INVALID_COMMAND_QUEUE);
    return command_queue->dispatch->clSetCommandQueueProperty(
        command_queue,
        properties,
        enable,
        old_properties);
}

/* Memory Object APIs */
CL_API_ENTRY cl_mem CL_API_CALL
clCreateBuffer(cl_context   context,
               cl_mem_flags flags,
               size_t       size,
               void *       host_ptr,
               cl_int *     errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_HANDLE(context, CL_INVALID_CONTEXT);
    return context->dispatch->clCreateBuffer(
        context,
        flags,
        size,
        host_ptr,
        errcode_ret);
}

CL_API_ENTRY cl_mem CL_API_CALL
clCreateImage2D(cl_context              context,
                cl_mem_flags            flags,
                const cl_image_format * image_format,
                size_t                  image_width,
                size_t                  image_height,
                size_t                  image_row_pitch,
                void *                  host_ptr,
                cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_HANDLE(context, CL_INVALID_CONTEXT);
    return context->dispatch->clCreateImage2D(
        context,
        flags,
        image_format,
        image_width,
        image_height,
        image_row_pitch,
        host_ptr,
        errcode_ret);
}

CL_API_ENTRY cl_mem CL_API_CALL
clCreateImage3D(cl_context              context,
                cl_mem_flags            flags,
                const cl_image_format * image_format,
                size_t                  image_width,
                size_t                  image_height,
                size_t                  image_depth,
                size_t                  image_row_pitch,
                size_t                  image_slice_pitch,
                void *                  host_ptr,
                cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_HANDLE(context, CL_INVALID_CONTEXT);
    return context->dispatch->clCreateImage3D(
        context,
        flags,
        image_format,
        image_width,
        image_height,
        image_depth,
        image_row_pitch,
        image_slice_pitch,
        host_ptr,
        errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainMemObject(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(memobj, CL_INVALID_MEM_OBJECT);
    return memobj->dispatch->clRetainMemObject(memobj);
}


CL_API_ENTRY cl_int CL_API_CALL
clReleaseMemObject(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(memobj, CL_INVALID_MEM_OBJECT);
    return memobj->dispatch->clReleaseMemObject(memobj);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetSupportedImageFormats(cl_context           context,
                           cl_mem_flags         flags,
                           cl_mem_object_type   image_type,
                           cl_uint              num_entries,
                           cl_image_format *    image_formats,
                           cl_uint *            num_image_formats) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(context, CL_INVALID_CONTEXT);
    return context->dispatch->clGetSupportedImageFormats(
        context,
        flags,
        image_type,
        num_entries,
        image_formats,
        num_image_formats);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetMemObjectInfo(cl_mem           memobj,
                   cl_mem_info      param_name,
                   size_t           param_value_size,
                   void *           param_value,
                   size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(memobj, CL_INVALID_MEM_OBJECT);
    return memobj->dispatch->clGetMemObjectInfo(
        memobj,
        param_name,
        param_value_size,
        param_value,
        param_value_size_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetImageInfo(cl_mem           image,
               cl_image_info    param_name,
               size_t           param_value_size,
               void *           param_value,
               size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(image, CL_INVALID_MEM_OBJECT);
    return image->dispatch->clGetImageInfo(
        image,
        param_name,
        param_value_size,
        param_value,
        param_value_size_ret);
}

/* Sampler APIs */
CL_API_ENTRY cl_sampler CL_API_CALL
clCreateSampler(cl_context          context,
                cl_bool             normalized_coords,
                cl_addressing_mode  addressing_mode,
                cl_filter_mode      filter_mode,
                cl_int *            errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_HANDLE(context, CL_INVALID_CONTEXT);
    return context->dispatch->clCreateSampler(
        context,
        normalized_coords,
        addressing_mode,
        filter_mode,
        errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainSampler(cl_sampler sampler) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(sampler, CL_INVALID_SAMPLER);
    return sampler->dispatch->clRetainSampler(sampler);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseSampler(cl_sampler sampler) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(sampler, CL_INVALID_SAMPLER);
    return sampler->dispatch->clReleaseSampler(sampler);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetSamplerInfo(cl_sampler         sampler,
                 cl_sampler_info    param_name,
                 size_t             param_value_size,
                 void *             param_value,
                 size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(sampler, CL_INVALID_SAMPLER);
    return sampler->dispatch->clGetSamplerInfo(
        sampler,
        param_name,
        param_value_size,
        param_value,
        param_value_size_ret);
}

/* Program Object APIs */
CL_API_ENTRY cl_program CL_API_CALL
clCreateProgramWithSource(cl_context        context,
                          cl_uint           count,
                          const char **     strings,
                          const size_t *    lengths,
                          cl_int *          errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_HANDLE(context, CL_INVALID_CONTEXT);
    return context->dispatch->clCreateProgramWithSource(
        context,
        count,
        strings,
        lengths,
        errcode_ret);
}

CL_API_ENTRY cl_program CL_API_CALL
clCreateProgramWithBinary(cl_context                     context,
                          cl_uint                        num_devices,
                          const cl_device_id *           device_list,
                          const size_t *                 lengths,
                          const unsigned char **         binaries,
                          cl_int *                       binary_status,
                          cl_int *                       errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_HANDLE(context, CL_INVALID_CONTEXT);
    return context->dispatch->clCreateProgramWithBinary(
        context,
        num_devices,
        device_list,
        lengths,
        binaries,
        binary_status,
        errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainProgram(cl_program program) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(program, CL_INVALID_PROGRAM);
    return program->dispatch->clRetainProgram(program);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseProgram(cl_program program) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(program, CL_INVALID_PROGRAM);
    return program->dispatch->clReleaseProgram(program);
}

CL_API_ENTRY cl_int CL_API_CALL
clBuildProgram(cl_program           program,
               cl_uint              num_devices,
               const cl_device_id * device_list,
               const char *         options,
               void (CL_CALLBACK *pfn_notify)(cl_program program, void * user_data),
               void *               user_data) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(program, CL_INVALID_PROGRAM);
    return program->dispatch->clBuildProgram(
        program,
        num_devices,
        device_list,
        options,
        pfn_notify,
        user_data);
}

CL_API_ENTRY cl_int CL_API_CALL
clUnloadCompiler(void) CL_API_SUFFIX__VERSION_1_0
{
    return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetProgramInfo(cl_program         program,
                 cl_program_info    param_name,
                 size_t             param_value_size,
                 void *             param_value,
                 size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(program, CL_INVALID_PROGRAM);
    return program->dispatch->clGetProgramInfo(
        program,
        param_name,
        param_value_size,
        param_value,
        param_value_size_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetProgramBuildInfo(cl_program            program,
                      cl_device_id          device,
                      cl_program_build_info param_name,
                      size_t                param_value_size,
                      void *                param_value,
                      size_t *              param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(program, CL_INVALID_PROGRAM);
    return program->dispatch->clGetProgramBuildInfo(
        program,
        device,
        param_name,
        param_value_size,
        param_value,
        param_value_size_ret);
}

/* Kernel Object APIs */
CL_API_ENTRY cl_kernel CL_API_CALL
clCreateKernel(cl_program      program,
               const char *    kernel_name,
               cl_int *        errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_HANDLE(program, CL_INVALID_PROGRAM);
    return program->dispatch->clCreateKernel(
        program,
        kernel_name,
        errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clCreateKernelsInProgram(cl_program     program,
                         cl_uint        num_kernels,
                         cl_kernel *    kernels,
                         cl_uint *      num_kernels_ret) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(program, CL_INVALID_PROGRAM);
    return program->dispatch->clCreateKernelsInProgram(
        program,
        num_kernels,
        kernels,
        num_kernels_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainKernel(cl_kernel    kernel) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(kernel, CL_INVALID_KERNEL);
    return kernel->dispatch->clRetainKernel(kernel);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseKernel(cl_kernel   kernel) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(kernel, CL_INVALID_KERNEL);
    return kernel->dispatch->clReleaseKernel(kernel);
}

CL_API_ENTRY cl_int CL_API_CALL
clSetKernelArg(cl_kernel    kernel,
               cl_uint      arg_index,
               size_t       arg_size,
               const void * arg_value) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(kernel, CL_INVALID_KERNEL);
    return kernel->dispatch->clSetKernelArg(
        kernel,
        arg_index,
        arg_size,
        arg_value);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetKernelInfo(cl_kernel       kernel,
                cl_kernel_info  param_name,
                size_t          param_value_size,
                void *          param_value,
                size_t *        param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(kernel, CL_INVALID_KERNEL);
    return kernel->dispatch->clGetKernelInfo(
        kernel,
        param_name,
        param_value_size,
        param_value,
        param_value_size_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetKernelWorkGroupInfo(cl_kernel                  kernel,
                         cl_device_id               device,
                         cl_kernel_work_group_info  param_name,
                         size_t                     param_value_size,
                         void *                     param_value,
                         size_t *                   param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(kernel, CL_INVALID_KERNEL);
    return kernel->dispatch->clGetKernelWorkGroupInfo(
        kernel,
        device,
        param_name,
        param_value_size,
        param_value,
        param_value_size_ret);
}

/* Event Object APIs */
CL_API_ENTRY cl_int CL_API_CALL
clWaitForEvents(cl_uint             num_events,
                const cl_event *    event_list) CL_API_SUFFIX__VERSION_1_0
{
    if (!num_events)
    {
        return CL_INVALID_VALUE;
    }
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(event_list[0], CL_INVALID_EVENT);
    return event_list[0]->dispatch->clWaitForEvents(
        num_events,
        event_list);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetEventInfo(cl_event         event,
               cl_event_info    param_name,
               size_t           param_value_size,
               void *           param_value,
               size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(event, CL_INVALID_EVENT);
    return event->dispatch->clGetEventInfo(
        event,
        param_name,
        param_value_size,
        param_value,
        param_value_size_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainEvent(cl_event event) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(event, CL_INVALID_EVENT);
    return event->dispatch->clRetainEvent(event);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseEvent(cl_event event) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(event, CL_INVALID_EVENT);
    return event->dispatch->clReleaseEvent(event);
}

/* Profiling APIs */
CL_API_ENTRY cl_int CL_API_CALL
clGetEventProfilingInfo(cl_event            event,
                        cl_profiling_info   param_name,
                        size_t              param_value_size,
                        void *              param_value,
                        size_t *            param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(event, CL_INVALID_EVENT);
    return event->dispatch->clGetEventProfilingInfo(
        event,
        param_name,
        param_value_size,
        param_value,
        param_value_size_ret);
}

/* Flush and Finish APIs */
CL_API_ENTRY cl_int CL_API_CALL
clFlush(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(command_queue, CL_INVALID_COMMAND_QUEUE);
    return command_queue->dispatch->clFlush(command_queue);
}

CL_API_ENTRY cl_int CL_API_CALL
clFinish(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(command_queue, CL_INVALID_COMMAND_QUEUE);
    return command_queue->dispatch->clFinish(command_queue);
}

/* Enqueued Commands APIs */
CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReadBuffer(cl_command_queue    command_queue,
                    cl_mem              buffer,
                    cl_bool             blocking_read,
                    size_t              offset,
                    size_t              cb,
                    void *              ptr,
                    cl_uint             num_events_in_wait_list,
                    const cl_event *    event_wait_list,
                    cl_event *          event) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(command_queue, CL_INVALID_COMMAND_QUEUE);
    return command_queue->dispatch->clEnqueueReadBuffer(
        command_queue,
        buffer,
        blocking_read,
        offset,
        cb,
        ptr,
        num_events_in_wait_list,
        event_wait_list,
        event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWriteBuffer(cl_command_queue   command_queue,
                     cl_mem             buffer,
                     cl_bool            blocking_write,
                     size_t             offset,
                     size_t             cb,
                     const void *       ptr,
                     cl_uint            num_events_in_wait_list,
                     const cl_event *   event_wait_list,
                     cl_event *         event) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(command_queue, CL_INVALID_COMMAND_QUEUE);
    return command_queue->dispatch->clEnqueueWriteBuffer(
        command_queue,
        buffer,
        blocking_write,
        offset,
        cb,
        ptr,
        num_events_in_wait_list,
        event_wait_list,
        event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyBuffer(cl_command_queue    command_queue,
                    cl_mem              src_buffer,
                    cl_mem              dst_buffer,
                    size_t              src_offset,
                    size_t              dst_offset,
                    size_t              cb,
                    cl_uint             num_events_in_wait_list,
                    const cl_event *    event_wait_list,
                    cl_event *          event) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(command_queue, CL_INVALID_COMMAND_QUEUE);
    return command_queue->dispatch->clEnqueueCopyBuffer(
        command_queue,
        src_buffer,
        dst_buffer,
        src_offset,
        dst_offset,
        cb,
        num_events_in_wait_list,
        event_wait_list,
        event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReadImage(cl_command_queue     command_queue,
                   cl_mem               image,
                   cl_bool              blocking_read,
                   const size_t *       origin,
                   const size_t *       region,
                   size_t               row_pitch,
                   size_t               slice_pitch,
                   void *               ptr,
                   cl_uint              num_events_in_wait_list,
                   const cl_event *     event_wait_list,
                   cl_event *           event) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(command_queue, CL_INVALID_COMMAND_QUEUE);
    return command_queue->dispatch->clEnqueueReadImage(
        command_queue,
        image,
        blocking_read,
        origin,
        region,
        row_pitch,
        slice_pitch,
        ptr,
        num_events_in_wait_list,
        event_wait_list,
        event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWriteImage(cl_command_queue    command_queue,
                    cl_mem              image,
                    cl_bool             blocking_write,
                    const size_t *      origin,
                    const size_t *      region,
                    size_t              input_row_pitch,
                    size_t              input_slice_pitch,
                    const void *        ptr,
                    cl_uint             num_events_in_wait_list,
                    const cl_event *    event_wait_list,
                    cl_event *          event) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(command_queue, CL_INVALID_COMMAND_QUEUE);
    return command_queue->dispatch->clEnqueueWriteImage(
        command_queue,
        image,
        blocking_write,
        origin,
        region,
        input_row_pitch,
        input_slice_pitch,
        ptr,
        num_events_in_wait_list,
        event_wait_list,
        event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyImage(cl_command_queue     command_queue,
                   cl_mem               src_image,
                   cl_mem               dst_image,
                   const size_t *       src_origin,
                   const size_t *       dst_origin,
                   const size_t *       region,
                   cl_uint              num_events_in_wait_list,
                   const cl_event *     event_wait_list,
                   cl_event *           event) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(command_queue, CL_INVALID_COMMAND_QUEUE);
    return command_queue->dispatch->clEnqueueCopyImage(
        command_queue,
        src_image,
        dst_image,
        src_origin,
        dst_origin,
        region,
        num_events_in_wait_list,
        event_wait_list,
        event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyImageToBuffer(cl_command_queue command_queue,
                           cl_mem           src_image,
                           cl_mem           dst_buffer,
                           const size_t *   src_origin,
                           const size_t *   region,
                           size_t           dst_offset,
                           cl_uint          num_events_in_wait_list,
                           const cl_event * event_wait_list,
                           cl_event *       event) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(command_queue, CL_INVALID_COMMAND_QUEUE);
    return command_queue->dispatch->clEnqueueCopyImageToBuffer(
        command_queue,
        src_image,
        dst_buffer,
        src_origin,
        region,
        dst_offset,
        num_events_in_wait_list,
        event_wait_list,
        event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyBufferToImage(cl_command_queue command_queue,
                           cl_mem           src_buffer,
                           cl_mem           dst_image,
                           size_t           src_offset,
                           const size_t *   dst_origin,
                           const size_t *   region,
                           cl_uint          num_events_in_wait_list,
                           const cl_event * event_wait_list,
                           cl_event *       event) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(command_queue, CL_INVALID_COMMAND_QUEUE);
    return command_queue->dispatch->clEnqueueCopyBufferToImage(
        command_queue,
        src_buffer,
        dst_image,
        src_offset,
        dst_origin,
        region,
        num_events_in_wait_list,
        event_wait_list,
        event);
}

CL_API_ENTRY void * CL_API_CALL
clEnqueueMapBuffer(cl_command_queue command_queue,
                   cl_mem           buffer,
                   cl_bool          blocking_map,
                   cl_map_flags     map_flags,
                   size_t           offset,
                   size_t           cb,
                   cl_uint          num_events_in_wait_list,
                   const cl_event * event_wait_list,
                   cl_event *       event,
                   cl_int *         errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_HANDLE(command_queue, CL_INVALID_COMMAND_QUEUE);
    return command_queue->dispatch->clEnqueueMapBuffer(
        command_queue,
        buffer,
        blocking_map,
        map_flags,
        offset,
        cb,
        num_events_in_wait_list,
        event_wait_list,
        event,
        errcode_ret);
}

CL_API_ENTRY void * CL_API_CALL
clEnqueueMapImage(cl_command_queue  command_queue,
                  cl_mem            image,
                  cl_bool           blocking_map,
                  cl_map_flags      map_flags,
                  const size_t *    origin,
                  const size_t *    region,
                  size_t *          image_row_pitch,
                  size_t *          image_slice_pitch,
                  cl_uint           num_events_in_wait_list,
                  const cl_event *  event_wait_list,
                  cl_event *        event,
                  cl_int *          errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_HANDLE(command_queue, CL_INVALID_COMMAND_QUEUE);
    return command_queue->dispatch->clEnqueueMapImage(
        command_queue,
        image,
        blocking_map,
        map_flags,
        origin,
        region,
        image_row_pitch,
        image_slice_pitch,
        num_events_in_wait_list,
        event_wait_list,
        event,
        errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueUnmapMemObject(cl_command_queue command_queue,
                        cl_mem           memobj,
                        void *           mapped_ptr,
                        cl_uint          num_events_in_wait_list,
                        const cl_event *  event_wait_list,
                        cl_event *        event) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(command_queue, CL_INVALID_COMMAND_QUEUE);
    return command_queue->dispatch->clEnqueueUnmapMemObject(
        command_queue,
        memobj,
        mapped_ptr,
        num_events_in_wait_list,
        event_wait_list,
        event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueNDRangeKernel(cl_command_queue command_queue,
                       cl_kernel        kernel,
                       cl_uint          work_dim,
                       const size_t *   global_work_offset,
                       const size_t *   global_work_size,
                       const size_t *   local_work_size,
                       cl_uint          num_events_in_wait_list,
                       const cl_event * event_wait_list,
                       cl_event *       event) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(command_queue, CL_INVALID_COMMAND_QUEUE);
    return command_queue->dispatch->clEnqueueNDRangeKernel(
        command_queue,
        kernel,
        work_dim,
        global_work_offset,
        global_work_size,
        local_work_size,
        num_events_in_wait_list,
        event_wait_list,
        event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueTask(cl_command_queue  command_queue,
              cl_kernel         kernel,
              cl_uint           num_events_in_wait_list,
              const cl_event *  event_wait_list,
              cl_event *        event) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(command_queue, CL_INVALID_COMMAND_QUEUE);
    return command_queue->dispatch->clEnqueueTask(
        command_queue,
        kernel,
        num_events_in_wait_list,
        event_wait_list,
        event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueNativeKernel(cl_command_queue  command_queue,
                      void (*user_func)(void *),
                      void *            args,
                      size_t            cb_args,
                      cl_uint           num_mem_objects,
                      const cl_mem *    mem_list,
                      const void **     args_mem_loc,
                      cl_uint           num_events_in_wait_list,
                      const cl_event *  event_wait_list,
                      cl_event *        event) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(command_queue, CL_INVALID_COMMAND_QUEUE);
    return command_queue->dispatch->clEnqueueNativeKernel(
        command_queue,
        user_func,
        args,
        cb_args,
        num_mem_objects,
        mem_list,
        args_mem_loc,
        num_events_in_wait_list,
        event_wait_list,
        event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueMarker(cl_command_queue    command_queue,
                cl_event *          event) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(command_queue, CL_INVALID_COMMAND_QUEUE);
    return command_queue->dispatch->clEnqueueMarker(
        command_queue,
        event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWaitForEvents(cl_command_queue command_queue,
                       cl_uint          num_events,
                       const cl_event * event_list) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(command_queue, CL_INVALID_COMMAND_QUEUE);
    return command_queue->dispatch->clEnqueueWaitForEvents(
        command_queue,
        num_events,
        event_list);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueBarrier(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(command_queue, CL_INVALID_COMMAND_QUEUE);
    return command_queue->dispatch->clEnqueueBarrier(command_queue);
}

CL_API_ENTRY void * CL_API_CALL
clGetExtensionFunctionAddress(const char *function_name) CL_API_SUFFIX__VERSION_1_0
{
    size_t funciton_name_length = strlen(function_name);
    KHRicdVendor* vendor = NULL;

    /* make sure the ICD is initialized */
    khrIcdInitialize();

    /* return any ICD-aware extensions */
    #define CL_COMMON_EXTENSION_ENTRYPOINT_ADD(name) if (!strcmp(function_name, #name) ) return (void *)&name

    /* Are these core or ext?  This is unclear, but they appear to be */
    /* independent from cl_khr_gl_sharing. */
    CL_COMMON_EXTENSION_ENTRYPOINT_ADD(clCreateFromGLBuffer);
    CL_COMMON_EXTENSION_ENTRYPOINT_ADD(clCreateFromGLTexture2D);
    CL_COMMON_EXTENSION_ENTRYPOINT_ADD(clCreateFromGLTexture3D);
    CL_COMMON_EXTENSION_ENTRYPOINT_ADD(clCreateFromGLRenderbuffer);
    CL_COMMON_EXTENSION_ENTRYPOINT_ADD(clGetGLObjectInfo);
    CL_COMMON_EXTENSION_ENTRYPOINT_ADD(clGetGLTextureInfo);
    CL_COMMON_EXTENSION_ENTRYPOINT_ADD(clEnqueueAcquireGLObjects);
    CL_COMMON_EXTENSION_ENTRYPOINT_ADD(clEnqueueReleaseGLObjects);

    /* cl_khr_gl_sharing */
    CL_COMMON_EXTENSION_ENTRYPOINT_ADD(clGetGLContextInfoKHR);

#if defined(_WIN32) && CL_SUPPORT_D3D_SHARING
    /* cl_khr_d3d10_sharing */
    CL_COMMON_EXTENSION_ENTRYPOINT_ADD(clGetDeviceIDsFromD3D10KHR);
    CL_COMMON_EXTENSION_ENTRYPOINT_ADD(clCreateFromD3D10BufferKHR);
    CL_COMMON_EXTENSION_ENTRYPOINT_ADD(clCreateFromD3D10Texture2DKHR);
    CL_COMMON_EXTENSION_ENTRYPOINT_ADD(clCreateFromD3D10Texture3DKHR);
    CL_COMMON_EXTENSION_ENTRYPOINT_ADD(clEnqueueAcquireD3D10ObjectsKHR);
    CL_COMMON_EXTENSION_ENTRYPOINT_ADD(clEnqueueReleaseD3D10ObjectsKHR);
#endif

    /* cl_ext_device_fission */
    CL_COMMON_EXTENSION_ENTRYPOINT_ADD(clCreateSubDevicesEXT);
    CL_COMMON_EXTENSION_ENTRYPOINT_ADD(clRetainDeviceEXT);
    CL_COMMON_EXTENSION_ENTRYPOINT_ADD(clReleaseDeviceEXT);

    /* fall back to vendor extension detection */
    for (vendor = khrIcdState.vendors; vendor; vendor = vendor->next)
    {
        size_t vendor_suffix_length = strlen(vendor->suffix);
        if (vendor_suffix_length <= funciton_name_length && vendor_suffix_length > 0)
        {
            const char *function_suffix = function_name+funciton_name_length-vendor_suffix_length;
            if (!strcmp(function_suffix, vendor->suffix) )
            {
                return vendor->clGetExtensionFunctionAddress(function_name);
            }
        }
    }
    return NULL;
}

CL_API_ENTRY cl_mem CL_API_CALL clCreateFromGLBuffer(
    cl_context    context,
    cl_mem_flags  flags,
    GLuint        bufobj,
    int *         errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_HANDLE(context, CL_INVALID_CONTEXT);
    return context->dispatch->clCreateFromGLBuffer(
        context,
        flags,
        bufobj,
        errcode_ret);
}

CL_API_ENTRY cl_mem CL_API_CALL clCreateFromGLTexture2D(
    cl_context      context,
    cl_mem_flags    flags,
    GLenum          target,
    GLint           miplevel,
    GLuint          texture,
    cl_int *        errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_HANDLE(context, CL_INVALID_CONTEXT);
    return context->dispatch->clCreateFromGLTexture2D(
        context,
        flags,
        target,
        miplevel,
        texture,
        errcode_ret);
}

CL_API_ENTRY cl_mem CL_API_CALL clCreateFromGLTexture3D(
    cl_context      context,
    cl_mem_flags    flags,
    GLenum          target,
    GLint           miplevel,
    GLuint          texture,
    cl_int *        errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_HANDLE(context, CL_INVALID_CONTEXT);
    return context->dispatch->clCreateFromGLTexture3D(
        context,
        flags,
        target,
        miplevel,
        texture,
        errcode_ret);
}

CL_API_ENTRY cl_mem CL_API_CALL clCreateFromGLRenderbuffer(
    cl_context           context,
    cl_mem_flags         flags,
    GLuint               renderbuffer,
    cl_int *             errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_HANDLE(context, CL_INVALID_CONTEXT);
    return context->dispatch->clCreateFromGLRenderbuffer(
        context,
        flags,
        renderbuffer,
        errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL clGetGLObjectInfo(
    cl_mem               memobj,
    cl_gl_object_type *  gl_object_type,
    GLuint *             gl_object_name) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(memobj, CL_INVALID_MEM_OBJECT);
    return memobj->dispatch->clGetGLObjectInfo(
        memobj,
        gl_object_type,
        gl_object_name);
}

CL_API_ENTRY cl_int CL_API_CALL clGetGLTextureInfo(
    cl_mem               memobj,
    cl_gl_texture_info   param_name,
    size_t               param_value_size,
    void *               param_value,
    size_t *             param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(memobj, CL_INVALID_MEM_OBJECT);
    return memobj->dispatch->clGetGLTextureInfo(
        memobj,
        param_name,
        param_value_size,
        param_value,
        param_value_size_ret);
}

CL_API_ENTRY cl_int CL_API_CALL clEnqueueAcquireGLObjects(
    cl_command_queue     command_queue,
    cl_uint              num_objects,
    const cl_mem *       mem_objects,
    cl_uint              num_events_in_wait_list,
    const cl_event *     event_wait_list,
    cl_event *           event) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(command_queue, CL_INVALID_COMMAND_QUEUE);
    return command_queue->dispatch->clEnqueueAcquireGLObjects(
        command_queue,
        num_objects,
        mem_objects,
        num_events_in_wait_list,
        event_wait_list,
        event);
}

CL_API_ENTRY cl_int CL_API_CALL clEnqueueReleaseGLObjects(
    cl_command_queue     command_queue,
    cl_uint              num_objects,
    const cl_mem *       mem_objects,
    cl_uint              num_events_in_wait_list,
    const cl_event *     event_wait_list,
    cl_event *           event) CL_API_SUFFIX__VERSION_1_0
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(command_queue, CL_INVALID_COMMAND_QUEUE);
    return command_queue->dispatch->clEnqueueReleaseGLObjects(
        command_queue,
        num_objects,
        mem_objects,
        num_events_in_wait_list,
        event_wait_list,
        event);
}

CL_API_ENTRY cl_int CL_API_CALL clGetGLContextInfoKHR(
    const cl_context_properties *properties,
    cl_gl_context_info param_name,
    size_t param_value_size,
    void *param_value,
    size_t *param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    cl_platform_id platform = NULL;

    /* initialize the platforms (in case they have not been already) */
    khrIcdInitialize();

    /* determine the platform to use from the properties specified */
    khrIcdContextPropertiesGetPlatform(properties, &platform);

    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(platform, CL_INVALID_PLATFORM);
    return platform->dispatch->clGetGLContextInfoKHR(
        properties,
        param_name,
        param_value_size,
        param_value,
        param_value_size_ret);
}

#if defined(_WIN32) && CL_SUPPORT_D3D_SHARING
/*
 *
 * cl_d3d10_sharing_khr
 *
 */

CL_API_ENTRY cl_int CL_API_CALL
clGetDeviceIDsFromD3D10KHR(
    cl_platform_id platform,
    cl_d3d10_device_source_khr d3d_device_source,
    void *d3d_object,
    cl_d3d10_device_set_khr d3d_device_set,
    cl_uint num_entries,
    cl_device_id *devices,
    cl_uint *num_devices)
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(platform, CL_INVALID_PLATFORM);
    return platform->dispatch->clGetDeviceIDsFromD3D10KHR(
        platform,
        d3d_device_source,
        d3d_object,
        d3d_device_set,
        num_entries,
        devices,
        num_devices);
}

CL_API_ENTRY cl_mem CL_API_CALL
clCreateFromD3D10BufferKHR(
    cl_context context,
    cl_mem_flags flags,
    ID3D10Buffer *resource,
    cl_int *errcode_ret)
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_HANDLE(context, CL_INVALID_CONTEXT);
    return context->dispatch->clCreateFromD3D10BufferKHR(
        context,
        flags,
        resource,
        errcode_ret);
}

CL_API_ENTRY cl_mem CL_API_CALL
clCreateFromD3D10Texture2DKHR(
    cl_context        context,
    cl_mem_flags      flags,
    ID3D10Texture2D * resource,
    UINT              subresource,
    cl_int *          errcode_ret)
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_HANDLE(context, CL_INVALID_CONTEXT);
    return context->dispatch->clCreateFromD3D10Texture2DKHR(
        context,
        flags,
        resource,
        subresource,
        errcode_ret);
}

CL_API_ENTRY cl_mem CL_API_CALL
clCreateFromD3D10Texture3DKHR(
    cl_context context,
    cl_mem_flags flags,
    ID3D10Texture3D *resource,
    UINT subresource,
    cl_int *errcode_ret)
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_HANDLE(context, CL_INVALID_CONTEXT);
    return context->dispatch->clCreateFromD3D10Texture3DKHR(
        context,
        flags,
        resource,
        subresource,
        errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueAcquireD3D10ObjectsKHR(
    cl_command_queue command_queue,
    cl_uint num_objects,
    const cl_mem *mem_objects,
    cl_uint num_events_in_wait_list,
    const cl_event *event_wait_list,
    cl_event *event)
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(command_queue, CL_INVALID_COMMAND_QUEUE);
    return command_queue->dispatch->clEnqueueAcquireD3D10ObjectsKHR(
        command_queue,
        num_objects,
        mem_objects,
        num_events_in_wait_list,
        event_wait_list,
        event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReleaseD3D10ObjectsKHR(
    cl_command_queue command_queue,
    cl_uint num_objects,
    cl_mem *mem_objects,
    cl_uint num_events_in_wait_list,
    const cl_event *event_wait_list,
    cl_event *event)
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(command_queue, CL_INVALID_COMMAND_QUEUE);
    return command_queue->dispatch->clEnqueueReleaseD3D10ObjectsKHR(
        command_queue,
        num_objects,
        mem_objects,
        num_events_in_wait_list,
        event_wait_list,
        event);
}
#endif

CL_API_ENTRY cl_int CL_API_CALL
clSetEventCallback(
    cl_event event,
    cl_int command_exec_callback_type,
    void (CL_CALLBACK *pfn_notify)(cl_event, cl_int, void *),
    void *user_data) CL_API_SUFFIX__VERSION_1_1
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(event, CL_INVALID_EVENT);
    return event->dispatch->clSetEventCallback(
        event,
        command_exec_callback_type,
        pfn_notify,
        user_data);
}

CL_API_ENTRY cl_mem CL_API_CALL
clCreateSubBuffer(
    cl_mem buffer,
    cl_mem_flags flags,
    cl_buffer_create_type buffer_create_type,
    const void * buffer_create_info,
    cl_int * errcode_ret) CL_API_SUFFIX__VERSION_1_1
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_HANDLE(buffer, CL_INVALID_MEM_OBJECT);
    return buffer->dispatch->clCreateSubBuffer(
        buffer,
        flags,
        buffer_create_type,
        buffer_create_info,
        errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clSetMemObjectDestructorCallback(
    cl_mem memobj,
    void (CL_CALLBACK * pfn_notify)( cl_mem, void*),
    void * user_data )             CL_API_SUFFIX__VERSION_1_1
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(memobj, CL_INVALID_MEM_OBJECT);
    return memobj->dispatch->clSetMemObjectDestructorCallback(
        memobj,
        pfn_notify,
        user_data);
}

CL_API_ENTRY cl_event CL_API_CALL
clCreateUserEvent(
    cl_context context,
    cl_int * errcode_ret) CL_API_SUFFIX__VERSION_1_1
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_HANDLE(context, CL_INVALID_CONTEXT);
    return context->dispatch->clCreateUserEvent(
        context,
        errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clSetUserEventStatus(
    cl_event event,
    cl_int execution_status) CL_API_SUFFIX__VERSION_1_1
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(event, CL_INVALID_EVENT);
    return event->dispatch->clSetUserEventStatus(
        event,
        execution_status);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReadBufferRect(
    cl_command_queue command_queue,
    cl_mem buffer,
    cl_bool blocking_read,
    const size_t * buffer_offset,
    const size_t * host_offset,
    const size_t * region,
    size_t buffer_row_pitch,
    size_t buffer_slice_pitch,
    size_t host_row_pitch,
    size_t host_slice_pitch,
    void * ptr,
    cl_uint num_events_in_wait_list,
    const cl_event * event_wait_list,
    cl_event * event) CL_API_SUFFIX__VERSION_1_1
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(command_queue, CL_INVALID_COMMAND_QUEUE);
    return command_queue->dispatch->clEnqueueReadBufferRect(
        command_queue,
        buffer,
        blocking_read,
        buffer_offset,
        host_offset,
        region,
        buffer_row_pitch,
        buffer_slice_pitch,
        host_row_pitch,
        host_slice_pitch,
        ptr,
        num_events_in_wait_list,
        event_wait_list,
        event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWriteBufferRect(
    cl_command_queue command_queue,
    cl_mem buffer,
    cl_bool blocking_read,
    const size_t * buffer_offset,
    const size_t * host_offset,
    const size_t * region,
    size_t buffer_row_pitch,
    size_t buffer_slice_pitch,
    size_t host_row_pitch,
    size_t host_slice_pitch,
    const void * ptr,
    cl_uint num_events_in_wait_list,
    const cl_event * event_wait_list,
    cl_event * event) CL_API_SUFFIX__VERSION_1_1
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(command_queue, CL_INVALID_COMMAND_QUEUE);
    return command_queue->dispatch->clEnqueueWriteBufferRect(
        command_queue,
        buffer,
        blocking_read,
        buffer_offset,
        host_offset,
        region,
        buffer_row_pitch,
        buffer_slice_pitch,
        host_row_pitch,
        host_slice_pitch,
        ptr,
        num_events_in_wait_list,
        event_wait_list,
        event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyBufferRect(
    cl_command_queue command_queue,
    cl_mem src_buffer,
    cl_mem dst_buffer,
    const size_t * src_origin,
    const size_t * dst_origin,
    const size_t * region,
    size_t src_row_pitch,
    size_t src_slice_pitch,
    size_t dst_row_pitch,
    size_t dst_slice_pitch,
    cl_uint num_events_in_wait_list,
    const cl_event * event_wait_list,
    cl_event * event) CL_API_SUFFIX__VERSION_1_1
{
    KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(command_queue, CL_INVALID_COMMAND_QUEUE);
    return command_queue->dispatch->clEnqueueCopyBufferRect(
        command_queue,
        src_buffer,
        dst_buffer,
        src_origin,
        dst_origin,
        region,
        src_row_pitch,
        src_slice_pitch,
        dst_row_pitch,
        dst_slice_pitch,
        num_events_in_wait_list,
        event_wait_list,
        event);
}

