/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_cl_precomp.h"
extern gceTRACEMODE  vclTraceMode;

/******************************************************************************\
****************************** CL API DUMP SUPPORT ****************************
\******************************************************************************/
#define CL_LOG_API(...)  gcmPRINT(__VA_ARGS__)

__CL_INLINE void __clfLogSourceStrings(cl_uint count, const char ** string)
{
    cl_uint i, j;
    cl_char tmpbuf[256], *chptr;

    gcmPRINT("####\n");
    for (i = 0; i < count; i++)
    {
        chptr = (cl_char *)string[i];
        while (*chptr != '\0')
        {
            for (j = 0; (j < 255 && *chptr != '\n' && *chptr != '\0'); j++)
            {
                tmpbuf[j] = *chptr++;
            }
            while (*chptr == '\n') chptr++;
            tmpbuf[j] = '\0';
            gcmPRINT("%s\n", tmpbuf);
        }
    }
    gcmPRINT("####\n");
}

cl_int LogclGetPlatformIDs(cl_uint NumEntries, cl_platform_id * Platforms, cl_uint * NumPlatforms)
{
    cl_int platformAvailble = NumEntries;
    cl_int i   = 0;
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();

    if(NumPlatforms)
    {
        platformAvailble = NumEntries < (*NumPlatforms) ? NumEntries : (*NumPlatforms);
    }
    CL_LOG_API("CL(tid=%d): ClGetPlatformIDs, num_entries:%d, numPlatforms:0x%x\n", tid, NumEntries, NumPlatforms?*NumPlatforms:0);
    CL_LOG_API("CL(tid=%d): Number of platforms available: %d\n", tid, platformAvailble);
    if(Platforms)
    {
        for(i=0; i<platformAvailble; i++)
        {
            CL_LOG_API("CL(tid=%d): platformID[%d]: %p\n", tid, i, Platforms[i]);
        }
    }

    return 0;
}

cl_int LogclGetPlatformInfo(
    cl_platform_id   Platform,
    cl_platform_info ParamName,
    size_t           ParamValueSize,
    void *           ParamValue,
    size_t *         ParamValueSizeRet
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): ClGetPlatformInfo, platform:%p, ParamName:0x%x, ParamValueSize:%d\n", tid, Platform, ParamName, ParamValueSize);
    CL_LOG_API("CL(tid=%d): ParamValue:%p, ParamValueSizeRet:0x%x\n", tid, ParamValue, ParamValueSizeRet?*ParamValueSizeRet:0);
    return 0;
}

cl_int LogclGetDeviceIDs(
    cl_platform_id   Platform,
    cl_device_type   DeviceType,
    cl_uint          NumEntries,
    cl_device_id *   Devices,
    cl_uint *        NumDevices
    )
{
    cl_int deviceAvailble = NumEntries;
    cl_int i   = 0;
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();

    if(NumDevices)
    {
        deviceAvailble = NumEntries < (*NumDevices) ? NumEntries : (*NumDevices);
    }
    CL_LOG_API("CL(tid=%d): ClGetDeviceIDs, platform:%p, num_entries:%d, NumDevices:%d\n", tid, Platform, NumEntries, NumDevices?*NumDevices:0);
    CL_LOG_API("CL(tid=%d): Number of device available: %d\n", tid, deviceAvailble);
    if(Devices)
    {
        for(i=0; i<deviceAvailble; i++)
        {
            CL_LOG_API("CL(tid=%d): deviceID[%d]: %p\n", tid, i, Devices[i]);
        }
    }
    return 0;
}

cl_int LogclGetDeviceInfo(
    cl_device_id    Device,
    cl_device_info  ParamName,
    size_t          ParamValueSize,
    void *          ParamValue,
    size_t *        ParamValueSizeRet
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clGetDeviceInfo, Device:%p, ParamName:0x%x, ParamValueSize:%d\n", tid, Device, ParamName, ParamValueSize);
    CL_LOG_API("CL(tid=%d): ParamValue:%p, ParamValueSizeRet:%d\n", tid, ParamValue, ParamValueSizeRet?*ParamValueSizeRet:0);
    return 0;
}

cl_int LogclCreateSubDevices(
    cl_device_id                         InDevice,
    const cl_device_partition_property * Properties ,
    cl_uint                              NumDevices,
    cl_device_id *                       OutDevices,
    cl_uint *                            NumDevicesRet
    )
{
    cl_uint i = 0;
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clCreateSubDevices, InDevice:%p, Properties:%d, NumDevices:%d, NumDevicesRet:%d\n", tid, InDevice, Properties?*Properties:0, NumDevices, NumDevicesRet?*NumDevicesRet:0);
    if(OutDevices)
    {
        for(i=0; i<(*NumDevicesRet); i++)
        {
            CL_LOG_API("CL(tid=%d): OutDevices[%d]: %p\n", tid, i, OutDevices[i]);
        }
    }
    return 0;
}

cl_int LogclRetainDevice(cl_device_id  device )
{
    CL_LOG_API("CL(tid=%d): clRetainDevice, Device:0x%x\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), device);
    return 0;
}

cl_int LogclReleaseDevice(cl_device_id  device )
{
    CL_LOG_API("CL(tid=%d): clReleaseDevice, Device:%p\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), device);
    return 0;
}

cl_context LogclCreateContext_Pre(
    const cl_context_properties * Properties,
    cl_uint                       NumDevices,
    const cl_device_id *          Devices,
    void                          (CL_CALLBACK * PfnNotify)(const char *, const void *, size_t, void *),
    void *                        UserData,
    cl_int *                      ErrcodeRet
    )
{
    cl_uint i=0;
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clCreateContext_Pre, Properties:%p, NumDevices:%d, callbackFunc:%p, UserData:%p, ErrcodeRet:%d\n", tid, Properties, NumDevices, PfnNotify, UserData, ErrcodeRet?*ErrcodeRet:0);
    for(i=0; i<NumDevices; i++)
    {
        CL_LOG_API("CL(tid=%d): Devices[%d]: %p\n", tid, i, Devices[i]);
    }
    return gcvNULL;
}

cl_context LogclCreateContext_Post(
    const cl_context_properties * Properties,
    cl_uint                       NumDevices,
    const cl_device_id *          Devices,
    void                          (CL_CALLBACK * PfnNotify)(const char *, const void *, size_t, void *),
    void *                        UserData,
    cl_int *                      ErrcodeRet,
    cl_context                    context
    )
{
    cl_uint i=0;
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clCreateContext_Post, Properties:%p, NumDevices:%d, callbackFunc:%p, UserData:%p, ErrcodeRet:%d\n", tid, Properties, NumDevices, PfnNotify, UserData, ErrcodeRet?*ErrcodeRet:0);
    for(i=0; i<NumDevices; i++)
    {
        CL_LOG_API("CL(tid=%d): Devices[%d]: %p\n", tid, i, Devices[i]);
    }
    CL_LOG_API("CL(tid=%d): context:%p\n", tid, context);
    return gcvNULL;
}

cl_context LogclCreateContextFromType_Pre(
    const cl_context_properties * Properties,
    cl_device_type                DeviceType,
    void                          (CL_CALLBACK * PfnNotify)(const char *, const void *, size_t, void *),
    void *                        UserData,
    cl_int *                      ErrcodeRet
    )
{
    CL_LOG_API("CL(tid=%d): clCreateContextFromType_Pre, Properties:%p, DeviceType:0x%llx, callbackFunc:%p, UserData:%p, ErrcodeRet:%d\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), Properties, DeviceType, PfnNotify, UserData, ErrcodeRet?*ErrcodeRet:0);
    return gcvNULL;
}

cl_context LogclCreateContextFromType_Post(
    const cl_context_properties * Properties,
    cl_device_type                DeviceType,
    void                          (CL_CALLBACK * PfnNotify)(const char *, const void *, size_t, void *),
    void *                        UserData,
    cl_int *                      ErrcodeRet,
    cl_context                    context
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clCreateContextFromType_Post, Properties:%p, DeviceType:0x%llx, callbackFunc:%p, UserData:%p, ErrcodeRet:%d\n", tid, Properties, DeviceType, PfnNotify, UserData, ErrcodeRet?*ErrcodeRet:0);
    CL_LOG_API("CL(tid=%d): context:%p\n", tid, context);
    return gcvNULL;
}

cl_int LogclRetainContext(cl_context Context)
{
    CL_LOG_API("CL(tid=%d): clRetainContext, context:%p\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), Context);
    return 0;
}

cl_int LogclReleaseContext(cl_context Context)
{
    CL_LOG_API("CL(tid=%d): clReleaseContext, context:%p\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), Context);
    return 0;
}

cl_int LogclGetContextInfo(
    cl_context         Context,
    cl_context_info    ParamName,
    size_t             ParamValueSize,
    void *             ParamValue,
    size_t *           ParamValueSizeRet
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clGetContextInfo, context:%p, ParamName:0x%x, ParamValueSize:%d, ParamValueSizeRet:%d\n", tid, Context, ParamName, ParamValueSize, ParamValueSizeRet?*ParamValueSizeRet:0);
    CL_LOG_API("CL(tid=%d): clGetContextInfo, ParamValue:%p\n", tid, ParamValue);
    return 0;
}

cl_command_queue LogclCreateCommandQueue_Pre(
    cl_context                     Context,
    cl_device_id                   Device,
    cl_command_queue_properties    Properties,
    cl_int *                       ErrcodeRet
    )
{
    CL_LOG_API("CL(tid=%d): clCreateCommandQueue_Pre, context:%p, device:%p, Properties:0x%llx, ErrcodeRet:%d\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), Context, Device, Properties, ErrcodeRet?*ErrcodeRet:0);
    return gcvNULL;
}

cl_command_queue LogclCreateCommandQueue_Post(
    cl_context                     Context,
    cl_device_id                   Device,
    cl_command_queue_properties    Properties,
    cl_int *                       ErrcodeRet,
    cl_command_queue               Commandqueue
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clCreateCommandQueue_Post, context:%p, device:%p, Properties:0x%llx, ErrcodeRet:%d\n", tid, Context, Device, Properties, ErrcodeRet?*ErrcodeRet:0);
    CL_LOG_API("CL(tid=%d): clCreateCommandQueue_Post, Commandqueue:%p\n", tid, Commandqueue);
    return gcvNULL;
}

cl_int LogclRetainCommandQueue(cl_command_queue    CommandQueue)
{
    CL_LOG_API("CL(tid=%d): clRetainCommandQueue, Commandqueue:%p\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), CommandQueue);
    return 0;
}

cl_int LogclReleaseCommandQueue(cl_command_queue    CommandQueue)
{
    CL_LOG_API("CL(tid=%d): clReleaseCommandQueue, Commandqueue:%p\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), CommandQueue);
    return 0;
}

cl_int LogclGetCommandQueueInfo(
    cl_command_queue      CommandQueue,
    cl_command_queue_info ParamName,
    size_t                ParamValueSize,
    void *                ParamValue,
    size_t *              ParamValueSizeRet
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clGetCommandQueueInfo, CommandQueue:%p, ParamName:0x%x, ParamValueSize:%d\n", tid, CommandQueue, ParamName, ParamValueSize);
    CL_LOG_API("CL(tid=%d): clGetCommandQueueInfo, ParamValue:%p, ParamValueSizeRet:%d\n", tid, ParamValue, ParamValueSizeRet?*ParamValueSizeRet:0);
    return 0;
}

cl_mem LogclCreateBuffer_Pre(
    cl_context   Context,
    cl_mem_flags Flags,
    size_t       Size,
    void *       HostPtr,
    cl_int *     ErrcodeRet)
{
    CL_LOG_API("CL(tid=%d): clCreateBuffer_Pre, context:%p, Flags:0x%llx, Size:%d, HostPtr:%p, ErrcodeRet:%d\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), Context, Flags, Size, HostPtr, ErrcodeRet?*ErrcodeRet:0);
    return gcvNULL;
}

cl_mem LogclCreateBuffer_Post(
    cl_context   Context,
    cl_mem_flags Flags,
    size_t       Size,
    void *       HostPtr,
    cl_int *     ErrcodeRet,
    cl_mem       buffer)
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clCreateBuffer_Post, context:%p, Flags:0x%llx, Size:%d, HostPtr:%p, ErrcodeRet:%d\n", tid, Context, Flags, Size, HostPtr, ErrcodeRet?*ErrcodeRet:0);
    CL_LOG_API("CL(tid=%d): clCreateBuffer_Post, buffer:%p\n", tid, buffer);
    return gcvNULL;
}

cl_mem LogclCreateSubBuffer_Pre(
    cl_mem                   Buffer,
    cl_mem_flags             Flags,
    cl_buffer_create_type    BufferCreateType,
    const void *             BufferCreateInfo,
    cl_int *                 ErrcodeRet
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clCreateSubBuffer_Pre, Buffer:%p, Flags:0x%llx, BufferCreateType:0x%x, ErrcodeRet:%d\n", tid, Buffer, Flags, BufferCreateType, ErrcodeRet?*ErrcodeRet:0);
    CL_LOG_API("CL(tid=%d): clCreateSubBuffer_Pre, BufferCreateInfo->origin:%d, BufferCreateInfo->size:%d\n", tid, ((cl_buffer_region*)BufferCreateInfo)->origin, ((cl_buffer_region*)BufferCreateInfo)->size);
    return gcvNULL;
}

cl_mem LogclCreateSubBuffer_Post(
    cl_mem                   Buffer,
    cl_mem_flags             Flags,
    cl_buffer_create_type    BufferCreateType,
    const void *             BufferCreateInfo,
    cl_int *                 ErrcodeRet,
    cl_mem                   subBuffer
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clCreateSubBuffer_Post, Buffer:%p, Flags:0x%llx, BufferCreateType:0x%x, ErrcodeRet:%d\n", tid, Buffer, Flags, BufferCreateType, ErrcodeRet?*ErrcodeRet:0);
    CL_LOG_API("CL(tid=%d): clCreateSubBuffer_Post, BufferCreateInfo->origin:%d, BufferCreateInfo->size:%d\n", tid, ((cl_buffer_region*)BufferCreateInfo)->origin, ((cl_buffer_region*)BufferCreateInfo)->size);
    CL_LOG_API("CL(tid=%d): clCreateSubBuffer_Post, subBuffer:%p\n", tid, subBuffer);
    return gcvNULL;
}

cl_mem LogclCreateImage_Pre(
    cl_context               Context ,
    cl_mem_flags             Flags ,
    const cl_image_format *  ImageFormat ,
    const cl_image_desc *    ImageDesc ,
    void *                   HostPtr ,
    cl_int *                 ErrcodeRet
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clCreateImage_Pre, context:%p, flags:0x%llx, hostPtr:%p, ErrcodeRet:%d\n", tid, Context, Flags, HostPtr, ErrcodeRet?*ErrcodeRet:0);
    CL_LOG_API("CL(tid=%d): clCreateImage_Pre, image_channel_order:0x%x, image_channel_data_type:0x%x\n", tid, ImageFormat->image_channel_order, ImageFormat->image_channel_data_type);
    CL_LOG_API("CL(tid=%d): clCreateImage_Pre, image_type:0x%x, width:%d, height:%d, depth:%d\n", tid, ImageDesc->image_type, ImageDesc->image_width, ImageDesc->image_height, ImageDesc->image_depth);
    CL_LOG_API("CL(tid=%d): clCreateImage_Pre, image_array_size:%d, image_row_pitch:%d, image_slice_pitch:%d, buffer:%p\n", tid, ImageDesc->image_array_size, ImageDesc->image_row_pitch, ImageDesc->image_slice_pitch, ImageDesc->buffer);
    return gcvNULL;
}

cl_mem LogclCreateImage_Post(
    cl_context               Context ,
    cl_mem_flags             Flags ,
    const cl_image_format *  ImageFormat ,
    const cl_image_desc *    ImageDesc ,
    void *                   HostPtr ,
    cl_int *                 ErrcodeRet ,
    cl_mem                   Image
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clCreateImage_Pre, context:%p, flags:0x%llx, hostPtr:%p, ErrcodeRet:%d\n", tid, Context, Flags, HostPtr, ErrcodeRet?*ErrcodeRet:0);
    CL_LOG_API("CL(tid=%d): clCreateImage_Pre, image_channel_order:0x%x, image_channel_data_type:0x%x\n", tid, ImageFormat->image_channel_order, ImageFormat->image_channel_data_type);
    CL_LOG_API("CL(tid=%d): clCreateImage_Pre, image_type:0x%x, width:%d, height:%d, depth:%d\n", tid, ImageDesc->image_type, ImageDesc->image_width, ImageDesc->image_height, ImageDesc->image_depth);
    CL_LOG_API("CL(tid=%d): clCreateImage_Pre, image_array_size:%d, image_row_pitch:%d, image_slice_pitch:%d, buffer:%p\n", tid, ImageDesc->image_array_size, ImageDesc->image_row_pitch, ImageDesc->image_slice_pitch, ImageDesc->buffer);
    CL_LOG_API("CL(tid=%d): clCreateImage_Pre, Image:%p\n", tid, Image);
    return gcvNULL;
}

cl_int LogclRetainMemObject(cl_mem MemObj)
{
    CL_LOG_API("CL(tid=%d): clRetainMemObject, memObject:%p\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), MemObj);
    return 0;
}

cl_int LogclReleaseMemObject(cl_mem MemObj)
{
    CL_LOG_API("CL(tid=%d): clReleaseMemObject, memObject:%p\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), MemObj);
    return 0;
}

cl_int LogclGetSupportedImageFormats(
    cl_context           Context,
    cl_mem_flags         Flags,
    cl_mem_object_type   ImageType,
    cl_uint              NumEntries,
    cl_image_format *    ImageFormats,
    cl_uint *            NumImageFormats
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clGetSupportedImageFormats, Context:%p, Flags:0x%llx, ImageType:0x%x, NumEntries:%d, NumImageFormats:%d\n", tid, Context, Flags, ImageType, NumEntries, NumImageFormats?*NumImageFormats:0);
    if(ImageFormats)
    {
        for(i=0; i<NumEntries; i++)
        {
            CL_LOG_API("CL(tid=%d): clGetSupportedImageFormats, ImageFormat[%d] image_channel_data_type:0x%x, image_channel_order:0x%x\n", tid, i, ImageFormats[i].image_channel_data_type, ImageFormats[i].image_channel_order);
        }
    }
    return 0;
}

cl_int LogclGetMemObjectInfo(
    cl_mem          MemObj,
    cl_mem_info     ParamName,
    size_t          ParamValueSize,
    void *          ParamValue,
    size_t *        ParamValueSizeRet
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clGetMemObjectInfo, memObject:%p, ParamName:0x%x, ParamValueSize:%d, ParamValueSizeRet:%d\n", tid, MemObj, ParamName, ParamValueSize, ParamValueSizeRet?*ParamValueSizeRet:0);
    CL_LOG_API("CL(tid=%d): clGetMemObjectInfo, ParamValue:%p\n", tid, ParamValue);
    return 0;
}

cl_int LogclGetImageInfo(
    cl_mem           Image,
    cl_image_info    ParamName,
    size_t           ParamValueSize,
    void *           ParamValue,
    size_t *         ParamValueSizeRet
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clGetImageInfo, Image:%p, ParamName:0x%x, ParamValueSize:%d, ParamValueSizeRet:%d\n", tid, Image, ParamName, ParamValueSize, ParamValueSizeRet?*ParamValueSizeRet:0);
    CL_LOG_API("CL(tid=%d): clGetImageInfo, ParamValue:%p\n", tid, ParamValue);
    return 0;
}

cl_int LogclSetMemObjectDestructorCallback(
    cl_mem      MemObj,
    void        (CL_CALLBACK * PfnNotify)(cl_mem, void *),
    void *      UserData
    )
{
    CL_LOG_API("CL(tid=%d): clSetMemObjectDestructorCallback, memObject:%p, callbackfunc:%p, userdata:%p\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), MemObj, PfnNotify, UserData);
    return 0;
}

cl_sampler LogclCreateSampler_Pre(
    cl_context          Context,
    cl_bool             NormalizedCoords,
    cl_addressing_mode  AddressingMode,
    cl_filter_mode      FilterMode,
    cl_int *            ErrcodeRet
    )
{
    CL_LOG_API("CL(tid=%d): clCreateSampler_Pre, Context:%p, NormalizedCoords:%x, AddressingMode:0x%x, FilterMode:0x%x, ErrcodeRet:%d\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), Context, NormalizedCoords, AddressingMode, FilterMode, ErrcodeRet?*ErrcodeRet:0);
    return gcvNULL;
}

cl_sampler LogclCreateSampler_Post(
    cl_context          Context,
    cl_bool             NormalizedCoords,
    cl_addressing_mode  AddressingMode,
    cl_filter_mode      FilterMode,
    cl_int *            ErrcodeRet,
    cl_sampler          sampler
    )
{
    CL_LOG_API("CL(tid=%d): clCreateSampler_Post, Context:%p, NormalizedCoords:%x, AddressingMode:0x%x, FilterMode:0x%x, ErrcodeRet:%d, sampler:%p\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), Context, NormalizedCoords, AddressingMode, FilterMode, ErrcodeRet?*ErrcodeRet:0, sampler);
    return gcvNULL;
}

cl_int LogclRetainSampler(cl_sampler Sampler)
{
    CL_LOG_API("CL(tid=%d): clRetainSampler, sampler:%p\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), Sampler);
    return 0;
}

cl_int LogclReleaseSampler(cl_sampler Sampler)
{
    CL_LOG_API("CL(tid=%d): clReleaseSampler, sampler:%p\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), Sampler);
    return 0;
}

cl_int LogclGetSamplerInfo(
    cl_sampler         Sampler,
    cl_sampler_info    ParamName,
    size_t             ParamValueSize,
    void *             ParamValue,
    size_t *           ParamValueSizeRet
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clGetSamplerInfo, Sampler:%p, ParamName:0x%x, ParamValueSize:%d, ParamValueSizeRet:%d\n", tid, Sampler, ParamName, ParamValueSize, ParamValueSizeRet?*ParamValueSizeRet:0);
    CL_LOG_API("CL(tid=%d): clGetSamplerInfo, ParamValue:%d\n", tid, ParamValue ? (*((gctUINT *)ParamValue)) : 0);
    return 0;
}

cl_program LogclCreateProgramWithSource_Pre(
    cl_context      Context,
    cl_uint         Count,
    const char **   Strings,
    const size_t *  Lengths,
    cl_int *        ErrcodeRet
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clCreateProgramWithSource_Pre, Context:%p, Count:%d, ErrcodeRet:%d\n", tid, Context, Count, ErrcodeRet?*ErrcodeRet:0);
    for(i=0; i<Count; i++)
    {
        CL_LOG_API("CL(tid=%d): clCreateProgramWithSource_Pre, Length:%d, String[%d]:%s\n", tid, Lengths?Lengths[i]:0, i, Strings[i]);
    }

    if (vclTraceMode == gcvTRACEMODE_FULL)
    {
        __clfLogSourceStrings(Count, Strings);
    }
    return gcvNULL;
}

cl_program LogclCreateProgramWithSource_Post(
    cl_context      Context,
    cl_uint         Count,
    const char **   Strings,
    const size_t *  Lengths,
    cl_int *        ErrcodeRet,
    cl_program      program
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clCreateProgramWithSource_Post, Context:%p, Count:%d, ErrcodeRet:%d\n", tid, Context, Count, ErrcodeRet?*ErrcodeRet:0);
    for(i=0; i<Count; i++)
    {
        CL_LOG_API("CL(tid=%d): clCreateProgramWithSource_Post, Length:%d, String[%d]:%s\n", tid, Lengths?Lengths[i]:0, i, Strings[i]);
    }
    CL_LOG_API("CL(tid=%d): clCreateProgramWithSource_Post, program:%p\n", tid, program);
    return gcvNULL;
}

cl_program LogclCreateProgramWithBinary_Pre(
    cl_context                     Context,
    cl_uint                        NumDevices,
    const cl_device_id *           DeviceList,
    const size_t *                 Lengths,
    const unsigned char **         Binaries,
    cl_int *                       BinaryStatus,
    cl_int *                       ErrcodeRet
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clCreateProgramWithBinary_Pre, Context:%p, NumDevices:%d, ErrcodeRet:%d\n", tid, Context, NumDevices, ErrcodeRet?*ErrcodeRet:0);
    for(i=0; i<NumDevices; i++)
    {
         CL_LOG_API("CL(tid=%d): clCreateProgramWithBinary_Pre, Device[%d]:%p, Length:%d, Binaries[%d]:%p, BinaryStatus:0x%x\n", tid, i, DeviceList[i], Lengths?Lengths[i]:0, i, Binaries[i], BinaryStatus?BinaryStatus[i]:0);
    }
    return gcvNULL;
}

cl_program LogclCreateProgramWithBinary_Post(
    cl_context                     Context,
    cl_uint                        NumDevices,
    const cl_device_id *           DeviceList,
    const size_t *                 Lengths,
    const unsigned char **         Binaries,
    cl_int *                       BinaryStatus,
    cl_int *                       ErrcodeRet,
    cl_program                     program
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clCreateProgramWithBinary_Post, Context:%p, NumDevices:%d, ErrcodeRet:%d, Program:%p\n", tid, Context, NumDevices, ErrcodeRet?*ErrcodeRet:0, program);
    for(i=0; i<NumDevices; i++)
    {
         CL_LOG_API("CL(tid=%d): clCreateProgramWithBinary_Post, Device[%d]:%p, Length:%d, Binaries[%d]:%p, BinaryStatus:0x%x\n", tid, i, DeviceList[i], Lengths?Lengths[i]:0, i, Binaries[i], BinaryStatus?BinaryStatus[i]:0);
    }
    return gcvNULL;
}

cl_program LogclCreateProgramWithBuiltInKernels_Pre(
    cl_context             Context ,
    cl_uint                NumDevices ,
    const cl_device_id *   DeviceList ,
    const char *           KernelNames ,
    cl_int *               ErrcodeRet
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clCreateProgramWithBuiltInKernels_Pre, Context:%p, NumDevices:%d, ErrcodeRet:%d\n", tid, Context, NumDevices, ErrcodeRet?*ErrcodeRet:0);
    for(i=0; i<NumDevices; i++)
    {
        CL_LOG_API("CL(tid=%d): clCreateProgramWithBuiltInKernels_Pre, DeviceList[%d]:%p, KernelNames[%d]:%s\n", tid, i, DeviceList[i], i, KernelNames);
    }
    return gcvNULL;
}

cl_program LogclCreateProgramWithBuiltInKernels_Post(
    cl_context             Context ,
    cl_uint                NumDevices ,
    const cl_device_id *   DeviceList ,
    const char *           KernelNames ,
    cl_int *               ErrcodeRet ,
    cl_program             program
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clCreateProgramWithBuiltInKernels_Post, Context:%p, NumDevices:%d, ErrcodeRet:%d, Program:%p\n", tid, Context, NumDevices, ErrcodeRet?*ErrcodeRet:0, program);
    for(i=0; i<NumDevices; i++)
    {
        CL_LOG_API("CL(tid=%d): clCreateProgramWithBuiltInKernels_Post, DeviceList[%d]:%p, KernelNames[%d]:%s\n", tid, i, DeviceList[i], i, KernelNames);
    }
    return gcvNULL;
}

cl_int LogclRetainProgram(cl_program Program)
{
    CL_LOG_API("CL(tid=%d): clRetainProgram, Program:%p\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), Program);
    return 0;
}

cl_int LogclReleaseProgram(cl_program Program)
{
    CL_LOG_API("CL(tid=%d): clReleaseProgram, Program:%p\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), Program);
    return 0;
}

cl_int LogclBuildProgram(
    cl_program           Program,
    cl_uint              NumDevices,
    const cl_device_id * DeviceList,
    const char *         Options,
    void                 (CL_CALLBACK *  PfnNotify)(cl_program, void *),
    void *               UserData
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clBuildProgram, Program:%p, NumDevices:%d, Options:%s\n", tid, Program, NumDevices, Options);
    for(i=0; i<NumDevices; i++)
    {
        CL_LOG_API("CL(tid=%d): clBuildProgram, DeviceList[%d]:%p\n", tid, i, DeviceList[i]);
    }
    CL_LOG_API("CL(tid=%d): clBuildProgram, PfnNotify:%p, UserData:%p\n", tid, PfnNotify, UserData);
    return 0;
}

cl_int LogclCompileProgram(
    cl_program            Program ,
    cl_uint               NumDevices ,
    const cl_device_id *  DeviceList ,
    const char *          Options ,
    cl_uint               NumInputHeaders ,
    const cl_program *    InputHeaders ,
    const char **         HeaderIncludeNames ,
    void (CL_CALLBACK *   PfnNotify )(cl_program  , void * ),
    void *                UserData
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clCompileProgram, Program:%p, NumDevices:%d, Options:%s\n", tid, Program, NumDevices, Options);
    for(i=0; i<NumDevices; i++)
    {
        CL_LOG_API("CL(tid=%d): clCompileProgram, DeviceList[%d]:%p\n", tid, i, DeviceList[i]);
    }
    CL_LOG_API("CL(tid=%d): clCompileProgram, NumInputHeaders:%d, PfnNotify:%p, UserData:%p\n", tid, NumInputHeaders, PfnNotify, UserData);
    for(i=0; i<NumInputHeaders; i++)
    {
        CL_LOG_API("CL(tid=%d): clCompileProgram, InputHeaders[%d]:%p, HeaderIncludeNames[%d]:%s\n", tid, i, InputHeaders[i], i, HeaderIncludeNames[i]);
    }
    return 0;
}

cl_program LogclLinkProgram_Pre(
    cl_context            Context ,
    cl_uint               NumDevices ,
    const cl_device_id *  DeviceList ,
    const char *          Options ,
    cl_uint               NumInputPrograms ,
    const cl_program *    InputPrograms ,
    void (CL_CALLBACK *   PfnNotify )(cl_program  , void *  ),
    void *                UserData ,
    cl_int *              ErrcodeRet
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clLinkProgram_Pre, Context:%p, NumDevices:%d, Options:%s, ErrcodeRet:%d\n", tid, Context, NumDevices, Options, ErrcodeRet?*ErrcodeRet:0);
    for(i=0; i<NumDevices; i++)
    {
        CL_LOG_API("CL(tid=%d): clLinkProgram_Pre, DeviceList[%d]:%p\n", tid, i, DeviceList[i]);
    }
    CL_LOG_API("CL(tid=%d): clLinkProgram_Pre, NumInputPrograms:%d, PfnNotify:%p, UserData:%p\n", tid, NumInputPrograms, PfnNotify, UserData);
    for(i=0; i<NumInputPrograms; i++)
    {
        CL_LOG_API("CL(tid=%d): clLinkProgram_Pre, InputPrograms[%d]:%p\n", tid, i, InputPrograms[i]);
    }
    return gcvNULL;
}

cl_program LogclLinkProgram_Post(
    cl_context            Context ,
    cl_uint               NumDevices ,
    const cl_device_id *  DeviceList ,
    const char *          Options ,
    cl_uint               NumInputPrograms ,
    const cl_program *    InputPrograms ,
    void (CL_CALLBACK *   PfnNotify )(cl_program  , void *  ),
    void *                UserData ,
    cl_int *              ErrcodeRet ,
    cl_program            program
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clLinkProgram_Post, Context:%p, NumDevices:%d, Options:%s, ErrcodeRet:%d, program:%p\n", tid, Context, NumDevices, Options, ErrcodeRet?*ErrcodeRet:0, program);
    for(i=0; i<NumDevices; i++)
    {
        CL_LOG_API("CL(tid=%d): clLinkProgram_Post, DeviceList[%d]:%p\n", tid, i, DeviceList[i]);
    }
    CL_LOG_API("CL(tid=%d): clLinkProgram_Post, NumInputPrograms:%d, PfnNotify:%p, UserData:%p\n", tid, NumInputPrograms, PfnNotify, UserData);
    for(i=0; i<NumInputPrograms; i++)
    {
        CL_LOG_API("CL(tid=%d): clLinkProgram_Post, InputPrograms[%d]:%p\n", tid, i, InputPrograms[i]);
    }
    return gcvNULL;
}

cl_int LogclUnloadPlatformCompiler(
    cl_platform_id  Platform
    )
{
    CL_LOG_API("CL(tid=%d): clUnloadPlatformCompiler, Platform:%p\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), Platform);
    return 0;
}

cl_int LogclGetProgramInfo(
    cl_program          Program,
    cl_program_info     ParamName,
    size_t              ParamValueSize,
    void *              ParamValue,
    size_t *            ParamValueSizeRet
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clGetProgramInfo, Program:%p, ParamName:0x%x, ParamValueSize:%d, ParamValueSizeRet:%d\n", tid, Program, ParamName, ParamValueSize, ParamValueSizeRet?*ParamValueSizeRet:0);
    CL_LOG_API("CL(tid=%d): clGetProgramInfo, ParamValue:%p\n", tid, ParamValue);
    return 0;
}

cl_int LogclGetProgramBuildInfo(
    cl_program            Program,
    cl_device_id          Device,
    cl_program_build_info ParamName,
    size_t                ParamValueSize,
    void *                ParamValue,
    size_t *              ParamValueSizeRet
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clGetProgramBuildInfo, Program:%p, Device:%p, ParamName:0x%x, ParamValueSize:%d, ParamValueSizeRet:%d\n", tid, Program, Device, ParamName, ParamValueSize, ParamValueSizeRet?*ParamValueSizeRet:0);
    CL_LOG_API("CL(tid=%d): clGetProgramBuildInfo, ParamValue:%p\n", tid, ParamValue);
    return 0;
}

cl_kernel LogclCreateKernel_Pre(
    cl_program      Program,
    const char *    KernelName,
    cl_int *        ErrcodeRet
    )
{
    CL_LOG_API("CL(tid=%d): clCreateKernel_Pre, Program:%p, KernelName:%s, ErrcodeRet:%d\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), Program, KernelName, ErrcodeRet?*ErrcodeRet:0);
    return gcvNULL;
}

cl_kernel LogclCreateKernel_Post(
    cl_program      Program,
    const char *    KernelName,
    cl_int *        ErrcodeRet,
    cl_kernel       kernel
    )
{
    CL_LOG_API("CL(tid=%d): clCreateKernel_Post, Program:%p, KernelName:%s, ErrcodeRet:%d, kernel:%p\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), Program, KernelName, ErrcodeRet?*ErrcodeRet:0, kernel);
    return gcvNULL;
}

cl_int LogclCreateKernelsInProgram(
    cl_program     Program,
    cl_uint        NumKernels,
    cl_kernel *    Kernels,
    cl_uint *      NumKernelsRet
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clCreateKernelsInProgram, Program:%p, NumKernels:%d\n", tid, Program, NumKernels);
    for(i=0; i<NumKernels; i++)
    {
        CL_LOG_API("CL(tid=%d): clCreateKernelsInProgram, Kernels[%d]:%p, NumKernelsRet[%d]:0x%x\n", tid, i, Kernels[i], i, NumKernelsRet?NumKernelsRet[i]:0);
    }
    return 0;
}

cl_int LogclRetainKernel(cl_kernel Kernel)
{
    CL_LOG_API("CL(tid=%d): clRetainKernel, Kernel:%p\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), Kernel);
    return 0;
}

cl_int LogclReleaseKernel(cl_kernel Kernel)
{
    CL_LOG_API("CL(tid=%d): clReleaseKernel, Kernel:%p\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), Kernel);
    return 0;
}

cl_int LogclSetKernelArg(
    cl_kernel    Kernel,
    cl_uint      ArgIndex,
    size_t       ArgSize,
    const void * ArgValue
    )
{
    CL_LOG_API("CL(tid=%d): clSetKernelArg, Kernel:%p, ArgIndex:%d, ArgSize:%d, ArgValue:%p\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), Kernel, ArgIndex, ArgSize, ArgValue);
    return 0;
}

cl_int LogclGetKernelInfo(
    cl_kernel       Kernel,
    cl_kernel_info  ParamName,
    size_t          ParamValueSize,
    void *          ParamValue,
    size_t *        ParamValueSizeRet
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clGetKernelInfo, Kernel:%p, ParamName:0x%x, ParamValueSize:%d, ParamValueSizeRet:%d\n", tid, Kernel, ParamName, ParamValueSize, ParamValueSizeRet?*ParamValueSizeRet:0);
    CL_LOG_API("CL(tid=%d): clGetKernelInfo, ParamValue:%p\n", tid, ParamValue);
    return 0;
}

cl_int LogclGetKernelArgInfo(
    cl_kernel           Kernel ,
    cl_uint             ArgIndx ,
    cl_kernel_arg_info  ParamName ,
    size_t              ParamValueSize ,
    void *              ParamValue ,
    size_t *            ParamValueSizeRet
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clGetKernelArgInfo, Kernel:%p, ArgIndx:%d, ParamName:0x%x\n", tid, Kernel, ArgIndx, ParamName);
    CL_LOG_API("CL(tid=%d): clGetKernelArgInfo, ParamValue:%p, ParamValueSize:%d, ParamValueSizeRet:%d\n", tid, ParamValue, ParamValueSize, ParamValueSizeRet?*ParamValueSizeRet:0);
    return 0;
}

cl_int LogclGetKernelWorkGroupInfo(
    cl_kernel                  Kernel,
    cl_device_id               Device,
    cl_kernel_work_group_info  ParamName,
    size_t                     ParamValueSize,
    void *                     ParamValue,
    size_t *                   ParamValueSizeRet
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clGetKernelWorkGroupInfo, Kernel:%p, Device:%p, ParamName:0x%x\n", tid, Kernel, Device, ParamName);
    CL_LOG_API("CL(tid=%d): clGetKernelWorkGroupInfo, ParamValue:%p, ParamValueSize:%d, ParamValueSizeRet:%d\n", tid, ParamValue, ParamValueSize, ParamValueSizeRet?*ParamValueSizeRet:0);
    return 0;
}

cl_int LogclWaitForEvents(
    cl_uint             NumEvents,
    const cl_event *    EventList
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clWaitForEvents, NumEvents:%d\n", tid, NumEvents);
    for(i=0; i<NumEvents; i++)
    {
        CL_LOG_API("CL(tid=%d): clWaitForEvents, EventList[%d]:%p\n", tid, i, EventList[i]);
    }
    return 0;
}

cl_int LogclGetEventInfo(
    cl_event         Event,
    cl_event_info    ParamName,
    size_t           ParamValueSize,
    void *           ParamValue,
    size_t *         ParamValueSizeRet
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clGetEventInfo, Event:%p, ParamName:0x%x\n", tid, Event, ParamName);
    CL_LOG_API("CL(tid=%d): clGetEventInfo, ParamValue:%p, ParamValueSize:%d, ParamValueSizeRet:%d\n", tid, ParamValue, ParamValueSize, ParamValueSizeRet?*ParamValueSizeRet:0);
    return 0;
}

cl_event LogclCreateUserEvent_Pre(
    cl_context    Context,
    cl_int *      ErrcodeRet
    )
{
    CL_LOG_API("CL(tid=%d): clCreateUserEvent_Pre, Context:%p, ErrcodeRet:%d\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), Context, ErrcodeRet?*ErrcodeRet:0);
    return gcvNULL;
}

cl_event LogclCreateUserEvent_Post(
    cl_context    Context,
    cl_int *      ErrcodeRet,
    cl_event      Event
    )
{
    CL_LOG_API("CL(tid=%d): clCreateUserEvent_Post, Context:%p, ErrcodeRet:%d, Event:%p\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), Context, ErrcodeRet?*ErrcodeRet:0, Event);
    return gcvNULL;
}

cl_int LogclRetainEvent(
    cl_event Event
    )
{
    CL_LOG_API("CL(tid=%d): clRetainEvent, Event:%p\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), Event);
    return 0;
}

cl_int LogclReleaseEvent(
    cl_event Event
    )
{
    CL_LOG_API("CL(tid=%d): clReleaseEvent, Event:%p\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), Event);
    return 0;
}

cl_int LogclSetUserEventStatus(
    cl_event   Event,
    cl_int     ExecutionStatus
    )
{
    CL_LOG_API("CL(tid=%d): clSetUserEventStatus, Event:%p, ExecutionStatus:0x%x\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), Event, ExecutionStatus);
    return 0;
}

cl_int LogclSetEventCallback(
    cl_event    Event,
    cl_int      CommandExecCallbackType,
    void        (CL_CALLBACK * PfnNotify)(cl_event, cl_int, void *),
    void *      UserData
    )
{
    CL_LOG_API("CL(tid=%d): clSetEventCallback, Event:%p, CommandExecCallbackType:%d, PfnNotify:%p, UserData:%p\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), Event, CommandExecCallbackType, PfnNotify, UserData);
    return 0;
}


cl_int LogclGetEventProfilingInfo(
    cl_event            Event,
    cl_profiling_info   ParamName,
    size_t              ParamValueSize,
    void *              ParamValue,
    size_t *            ParamValueSizeRet
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clGetEventProfilingInfo, Event:%p, ParamName:0x%x\n", tid, Event, ParamName);
    CL_LOG_API("CL(tid=%d): clGetEventProfilingInfo, ParamValue:%d, ParamValueSize:%d, ParamValueSizeRet:%d\n", tid, ParamValue?(*((gctUINT *)ParamValue)):0, ParamValueSize, ParamValueSizeRet?*ParamValueSizeRet:0);
    return 0;
}

cl_int LogclFlush(
    cl_command_queue CommandQueue
    )
{
    CL_LOG_API("CL(tid=%d): clFlush, CommandQueue:%p\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), CommandQueue);
    return 0;
}

cl_int LogclFinish(
    cl_command_queue CommandQueue
    )
{
    CL_LOG_API("CL(tid=%d): clFinish, CommandQueue:%p\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), CommandQueue);
    return 0;
}

cl_int LogclEnqueueReadBuffer(
    cl_command_queue    CommandQueue,
    cl_mem              Buffer,
    cl_bool             BlockingRead,
    size_t              Offset,
    size_t              Cb,
    void *              Ptr,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clEnqueueReadBuffer, CommandQueue:%p, Buffer:%p, BlockingRead:%d, Offset:%d, Cb:%d, Ptr:%p, NumEventsInWaitList:%d\n", tid, CommandQueue, Buffer, BlockingRead, Offset, Cb, Ptr, NumEventsInWaitList);
    for(i=0; i< NumEventsInWaitList; i++)
    {
        CL_LOG_API("CL(tid=%d): clEnqueueReadBuffer, EventWaitList[%d]:%p\n", tid, i, EventWaitList[i]);
    }
    CL_LOG_API("CL(tid=%d): clEnqueueReadBuffer, Event:%p\n", tid, Event);
    return 0;
}

cl_int LogclEnqueueReadBufferRect(
    cl_command_queue    CommandQueue,
    cl_mem              Buffer,
    cl_bool             BlockingRead,
    const size_t *      BufferOrigin,
    const size_t *      HostOrigin,
    const size_t *      Region,
    size_t              BufferRowPitch,
    size_t              BufferSlicePitch,
    size_t              HostRowPitch,
    size_t              HostSlicePitch,
    void *              Ptr,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clEnqueueReadBufferRect, CommandQueue:%p, Buffer:%p, BlockingRead:%d, BufferOrigin:%p\n", tid, CommandQueue, Buffer, BlockingRead, BufferOrigin);
    CL_LOG_API("CL(tid=%d): clEnqueueReadBufferRect, HostOrigin:%p, Region:%p, BufferRowPitch:%d\n", tid, HostOrigin, Region, BufferRowPitch);
    CL_LOG_API("CL(tid=%d): clEnqueueReadBufferRect, BufferSlicePitch:%d, HostRowPitch:%d, HostSlicePitch:%d\n", tid, BufferSlicePitch, HostRowPitch, HostSlicePitch);
    for(i=0; i< NumEventsInWaitList; i++)
    {
        CL_LOG_API("CL(tid=%d): clEnqueueReadBufferRect, EventWaitList[%d]:%p\n", tid, i, EventWaitList[i]);
    }
    CL_LOG_API("CL(tid=%d): clEnqueueReadBufferRect, Ptr:%p, NumEventsInWaitList:%d, Event:%p\n", tid, Ptr, NumEventsInWaitList, Event);
    return 0;
}

cl_int LogclEnqueueWriteBuffer(
    cl_command_queue    CommandQueue,
    cl_mem              Buffer,
    cl_bool             BlockingWrite,
    size_t              Offset,
    size_t              Cb,
    const void *        Ptr,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clEnqueueWriteBuffer, CommandQueue:%p, Buffer:%p, BlockingWrite:%d, Offset:%d, Cb:%d, Ptr:%p, NumEventsInWaitList:%d\n", tid, CommandQueue, Buffer, BlockingWrite, Offset, Cb, Ptr, NumEventsInWaitList);
    for(i=0; i< NumEventsInWaitList; i++)
    {
        CL_LOG_API("CL(tid=%d): clEnqueueWriteBuffer, EventWaitList[%d]:%p\n", tid, i, EventWaitList[i]);
    }
    CL_LOG_API("CL(tid=%d): clEnqueueWriteBuffer, Event:%p\n", tid, Event);
    return 0;
}

cl_int LogclEnqueueWriteBufferRect(
    cl_command_queue    CommandQueue,
    cl_mem              Buffer,
    cl_bool             BlockingWrite,
    const size_t *      BufferOrigin,
    const size_t *      HostOrigin,
    const size_t *      Region,
    size_t              BufferRowPitch,
    size_t              BufferSlicePitch,
    size_t              HostRowPitch,
    size_t              HostSlicePitch,
    const void *        Ptr,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clEnqueueWriteBufferRect, CommandQueue:%p, Buffer:%p, BlockingWrite:%d, BufferOrigin:%p\n", tid, CommandQueue, Buffer, BlockingWrite, BufferOrigin);
    CL_LOG_API("CL(tid=%d): clEnqueueWriteBufferRect, HostOrigin:%p, Region:%p, BufferRowPitch:%d\n", tid, HostOrigin, Region, BufferRowPitch);
    CL_LOG_API("CL(tid=%d): clEnqueueWriteBufferRect, BufferSlicePitch:%d, HostRowPitch:%d, HostSlicePitch:%d\n", tid, BufferSlicePitch, HostRowPitch, HostSlicePitch);
    for(i=0; i< NumEventsInWaitList; i++)
    {
        CL_LOG_API("CL(tid=%d): clEnqueueWriteBufferRect, EventWaitList[%d]:%p\n", tid, i, EventWaitList[i]);
    }
    CL_LOG_API("CL(tid=%d): clEnqueueWriteBufferRect, Ptr:0x%x, NumEventsInWaitList:%d, Event:%p\n", tid, Ptr, NumEventsInWaitList, Event);
    return 0;
}

cl_int LogclEnqueueFillBuffer(
    cl_command_queue    CommandQueue ,
    cl_mem              Buffer ,
    const void *        Pattern ,
    size_t              PatternSize ,
    size_t              Offset ,
    size_t              Size ,
    cl_uint             NumEventsInWaitList ,
    const cl_event *    EventWaitList ,
    cl_event *          Event
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clEnqueueFillBuffer, CommandQueue:%p, Buffer:%p, Pattern:%p, PatternSize:%d, Offset:%d, Size:0x%x\n", tid, CommandQueue, Buffer, Pattern, PatternSize, Offset, Size);
    for(i=0; i< NumEventsInWaitList; i++)
    {
        CL_LOG_API("CL(tid=%d): clEnqueueFillBuffer, EventWaitList[%d]:%p\n", tid, i, EventWaitList[i]);
    }
    CL_LOG_API("CL(tid=%d): clEnqueueFillBuffer, NumEventsInWaitList:%d, Event:%p\n", tid, NumEventsInWaitList, Event);
    return 0;
}

cl_int LogclEnqueueCopyBuffer(
    cl_command_queue    CommandQueue,
    cl_mem              SrcBuffer,
    cl_mem              DstBuffer,
    size_t              SrcOffset,
    size_t              DstOffset,
    size_t              Cb,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clEnqueueCopyBuffer, CommandQueue:%p, SrcBuffer:%p, DstBuffer:%p\n", tid, CommandQueue, SrcBuffer, DstBuffer);
    CL_LOG_API("CL(tid=%d): clEnqueueCopyBuffer, SrcOffset:0x%x, DstOffset:0x%x, Cb:0x%x\n", tid, SrcOffset, DstOffset, Cb);
    for(i=0; i< NumEventsInWaitList; i++)
    {
        CL_LOG_API("CL(tid=%d): clEnqueueCopyBuffer, EventWaitList[%d]:%p\n", tid, i, EventWaitList[i]);
    }
    CL_LOG_API("CL(tid=%d): clEnqueueCopyBuffer, NumEventsInWaitList:%d, Event:%p\n", tid, NumEventsInWaitList, Event);
    return 0;
}

cl_int LogclEnqueueCopyBufferRect(
    cl_command_queue    CommandQueue,
    cl_mem              SrcBuffer,
    cl_mem              DstBuffer,
    const size_t *      SrcOrigin,
    const size_t *      DstOrigin,
    const size_t *      Region,
    size_t              SrcRowPitch,
    size_t              SrcSlicePitch,
    size_t              DstRowPitch,
    size_t              DstSlicePitch,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event
    )
{
     cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clEnqueueCopyBufferRect, CommandQueue:%p, SrcBuffer:%p, DstBuffer:%p\n", tid, CommandQueue, SrcBuffer, DstBuffer);
    CL_LOG_API("CL(tid=%d): clEnqueueCopyBufferRect, SrcOrigin:%p, DstOrigin:%p, Region:%p\n", tid, SrcOrigin, DstOrigin, Region);
    CL_LOG_API("CL(tid=%d): clEnqueueCopyBufferRect, SrcRowPitch:%d, SrcSlicePitch:%d, DstRowPitch:%d, DstSlicePitch:%d\n", tid, SrcRowPitch, SrcSlicePitch, DstRowPitch, DstSlicePitch);
    for(i=0; i< NumEventsInWaitList; i++)
    {
        CL_LOG_API("CL(tid=%d): clEnqueueCopyBufferRect, EventWaitList[%d]:%p\n", tid, i, EventWaitList[i]);
    }
    CL_LOG_API("CL(tid=%d): clEnqueueCopyBufferRect, NumEventsInWaitList:%d, Event:%p\n", tid, NumEventsInWaitList, Event);
    return 0;
}

cl_int LogclEnqueueReadImage(
    cl_command_queue    CommandQueue,
    cl_mem              Image,
    cl_bool             BlockingRead,
    const size_t *      Origin /*[3]*/,
    const size_t *      Region /*[3]*/,
    size_t              RowPitch,
    size_t              SlicePitch,
    void *              Ptr,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clEnqueueReadImage, CommandQueue:%p, Image:%p, BlockingRead:%d, Origin[0]:%d, Origin[1]:%d, Origin[2]:%d, NumEventsInWaitList:%d\n", tid, CommandQueue, Image, BlockingRead, Origin[0], Origin[1], Origin[2], NumEventsInWaitList);
    CL_LOG_API("CL(tid=%d): clEnqueueReadImage, Region[0]:%d, Region[1]:%d, Region[2]:%d, RowPitch:%d, SlicePitch:%d, Ptr:%p\n", tid, Region[0], Region[1], Region[2], RowPitch, SlicePitch, Ptr);
    for(i=0; i< NumEventsInWaitList; i++)
    {
        CL_LOG_API("CL(tid=%d): clEnqueueReadImage, EventWaitList[%d]:%p\n", tid, i, EventWaitList[i]);
    }
    CL_LOG_API("CL(tid=%d): clEnqueueReadImage, Event:%p\n", tid, Event);
    return 0;
}

cl_int LogclEnqueueWriteImage(
    cl_command_queue    CommandQueue,
    cl_mem              Image,
    cl_bool             BlockingWrite,
    const size_t *      Origin,
    const size_t *      Region,
    size_t              InputRowPitch,
    size_t              InputSlicePitch,
    const void *        Ptr,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clEnqueueWriteImage, CommandQueue:%p, Image:%p, BlockingWrite:%d, Origin[0]:%d, Origin[1]:%d, Origin[2]:%d, NumEventsInWaitList:%d\n", tid, CommandQueue, Image, BlockingWrite, Origin[0], Origin[1], Origin[2], NumEventsInWaitList);
    CL_LOG_API("CL(tid=%d): clEnqueueWriteImage, Region[0]:%d, Region[1]:%d, Region[2]:%d, InputRowPitch:%d, InputSlicePitch:%d, Ptr:%p\n", tid, Region[0], Region[1], Region[2], InputRowPitch, InputSlicePitch, Ptr);
    for(i=0; i< NumEventsInWaitList; i++)
    {
        CL_LOG_API("CL(tid=%d): clEnqueueWriteImage, EventWaitList[%d]:%p\n", tid, i, EventWaitList[i]);
    }
    CL_LOG_API("CL(tid=%d): clEnqueueWriteImage, Event:%p\n", tid, Event);
    return 0;
}

cl_int LogclEnqueueFillImage(
    cl_command_queue    CommandQueue ,
    cl_mem              Image ,
    const void *        FillColor ,
    const size_t *      Origin,
    const size_t *      Region,
    cl_uint             NumEventsInWaitList ,
    const cl_event *    EventWaitList ,
    cl_event *          Event
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clEnqueueFillImage, CommandQueue:%p, Image:%p, FillColor:%p, Origin[0]:%d, Origin[1]:%d, Origin[2]:%d, NumEventsInWaitList:%d\n", tid, CommandQueue, Image, FillColor, Origin[0], Origin[1], Origin[2], NumEventsInWaitList);
    CL_LOG_API("CL(tid=%d): clEnqueueFillImage, Region[0]:%d, Region[1]:%d, Region[2]:%d\n", tid, Region[0], Region[1], Region[2]);
    for(i=0; i< NumEventsInWaitList; i++)
    {
        CL_LOG_API("CL(tid=%d): clEnqueueFillImage, EventWaitList[%d]:%p\n", tid, i, EventWaitList[i]);
    }
    CL_LOG_API("CL(tid=%d): clEnqueueFillImage, Event:%p\n", tid, Event);
    return 0;
}

cl_int LogclEnqueueCopyImage(
    cl_command_queue    CommandQueue,
    cl_mem              SrcImage,
    cl_mem              DstImage,
    const size_t *      SrcOrigin /*[3]*/,
    const size_t *      DstOrigin /*[3]*/,
    const size_t *      Region /*[3]*/,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clEnqueueCopyImage, CommandQueue:%p, SrcImage:%p, DstImage:%p, SrcOrigin[0]:%d, SrcOrigin[1]:%d, SrcOrigin[2]:%d, NumEventsInWaitList:%d\n", tid, CommandQueue, SrcImage, DstImage, SrcOrigin[0], SrcOrigin[1], SrcOrigin[2], NumEventsInWaitList);
    CL_LOG_API("CL(tid=%d): clEnqueueCopyImage, DstOrigin[0]:%d, DstOrigin[1]:%d, DstOrigin[2]:%d, Region[0]:%d, Region[1]:%d, Region[2]:%d\n", tid, DstOrigin[0], DstOrigin[1], DstOrigin[2], Region[0], Region[1], Region[2]);
    for(i=0; i< NumEventsInWaitList; i++)
    {
        CL_LOG_API("CL(tid=%d): clEnqueueCopyImage, EventWaitList[%d]:%p\n", tid, i, EventWaitList[i]);
    }
    CL_LOG_API("CL(tid=%d): clEnqueueCopyImage, Event:%p\n", tid, Event);
    return 0;
}

cl_int LogclEnqueueCopyImageToBuffer(
    cl_command_queue    CommandQueue,
    cl_mem              SrcImage,
    cl_mem              DstBuffer,
    const size_t *      SrcOrigin,
    const size_t *      Region,
    size_t              DstOffset,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clEnqueueCopyImageToBuffer, CommandQueue:%p, SrcImage:%p, DstBuffer:%p, SrcOrigin:%p, Region:%p, DstOffset:%d, NumEventsInWaitList:%d\n", tid, CommandQueue, SrcImage, DstBuffer, SrcOrigin, Region, DstOffset, NumEventsInWaitList);
    for(i=0; i< NumEventsInWaitList; i++)
    {
        CL_LOG_API("CL(tid=%d): clEnqueueCopyImageToBuffer, EventWaitList[%d]:%p\n", tid, i, EventWaitList[i]);
    }
    CL_LOG_API("CL(tid=%d): clEnqueueCopyImageToBuffer, Event:%p\n", tid, Event);
    return 0;
}

cl_int LogclEnqueueCopyBufferToImage(
    cl_command_queue    CommandQueue,
    cl_mem              SrcBuffer,
    cl_mem              DstImage,
    size_t              SrcOffset,
    const size_t *      DstOrigin,
    const size_t *      Region,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clEnqueueCopyBufferToImage, CommandQueue:%p, SrcBuffer:%p, DstImage:%p, SrcOffset:0x%x, Region:%p, DstOrigin:%p, NumEventsInWaitList:%d\n", tid, CommandQueue, SrcBuffer, DstImage, SrcOffset, Region, DstOrigin, NumEventsInWaitList);
    for(i=0; i< NumEventsInWaitList; i++)
    {
        CL_LOG_API("CL(tid=%d): clEnqueueCopyBufferToImage, EventWaitList[%d]:%p\n", tid, i, EventWaitList[i]);
    }
    CL_LOG_API("CL(tid=%d): clEnqueueCopyBufferToImage, Event:%p\n", tid, Event);
    return 0;
}

void * LogclEnqueueMapBuffer_Pre(
    cl_command_queue    CommandQueue,
    cl_mem              Buffer,
    cl_bool             BlockingMap,
    cl_map_flags        MapFlags,
    size_t              Offset,
    size_t              Cb,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event,
    cl_int *            ErrCodeRet
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clEnqueueMapBuffer_Pre, CommandQueue:%p, Buffer:%p, BlockingMap:%d\n", tid, CommandQueue, Buffer, BlockingMap);
    CL_LOG_API("CL(tid=%d): clEnqueueMapBuffer_Pre, MapFlags:0x%x, Offset:%d, Cb:%d, NumEventsInWaitList:%d\n", tid, MapFlags, Offset, Cb, NumEventsInWaitList);
    for(i=0; i< NumEventsInWaitList; i++)
    {
        CL_LOG_API("CL(tid=%d): clEnqueueCopyImageToBuffer, EventWaitList[%d]:%p\n", tid, i, EventWaitList[i]);
    }
    CL_LOG_API("CL(tid=%d): clEnqueueCopyImageToBuffer, Event:%p, ErrCodeRet:%d\n", tid, Event, ErrCodeRet?*ErrCodeRet:0);
    return gcvNULL;
}

void * LogclEnqueueMapBuffer_Post(
    cl_command_queue    CommandQueue,
    cl_mem              Buffer,
    cl_bool             BlockingMap,
    cl_map_flags        MapFlags,
    size_t              Offset,
    size_t              Cb,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event,
    cl_int *            ErrCodeRet,
    void *              Ptr
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clEnqueueMapBuffer_Pre, CommandQueue:%p, Buffer:%p, BlockingMap:%d\n", tid, CommandQueue, Buffer, BlockingMap);
    CL_LOG_API("CL(tid=%d): clEnqueueMapBuffer_Pre, MapFlags:0x%x, Offset:%d, Cb:%d, NumEventsInWaitList:%d\n", tid, MapFlags, Offset, Cb, NumEventsInWaitList);
    for(i=0; i< NumEventsInWaitList; i++)
    {
        CL_LOG_API("CL(tid=%d): clEnqueueCopyImageToBuffer, EventWaitList[%d]:%p\n", tid, i, EventWaitList[i]);
    }
    CL_LOG_API("CL(tid=%d): clEnqueueCopyImageToBuffer, Event:0x%x, ErrCodeRet:%d, Ptr:%p\n", tid, Event, ErrCodeRet?*ErrCodeRet:0, Ptr);
    return gcvNULL;
}

void* LogclEnqueueMapImage_Pre(
    cl_command_queue    CommandQueue,
    cl_mem              Image,
    cl_bool             BlockingMap,
    cl_map_flags        MapFlags,
    const size_t *      Origin,
    const size_t *      Region,
    size_t *            ImageRowPitch,
    size_t *            ImageSlicePitch,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event,
    cl_int *            ErrCodeRet
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clEnqueueMapImage_Pre, CommandQueue:%p, Image:%p, BlockingMap:%d\n", tid, CommandQueue, Image, BlockingMap);
    CL_LOG_API("CL(tid=%d): clEnqueueMapImage_Pre, MapFlags:0x%x, ImageRowPitch:%d, ImageSlicePitch:%d, NumEventsInWaitList:%d\n", tid, MapFlags, (int)*ImageRowPitch, (int)(ImageSlicePitch ? *ImageSlicePitch : (size_t)-1), (int)NumEventsInWaitList);
    CL_LOG_API("CL(tid=%d): clEnqueueMapImage_Pre, Origin[0]:%d, Origin[1]:%d, Origin[2]:%d, Region[0]:%d, Region[1]:%d, Region[2]:%d\n", tid, Origin[0], Origin[1], Origin[2], Region[0], Region[1], Region[2]);
    for(i=0; i< NumEventsInWaitList; i++)
    {
        CL_LOG_API("CL(tid=%d): clEnqueueMapImage_Pre, EventWaitList[%d]:%p\n", tid, i, EventWaitList[i]);
    }
    CL_LOG_API("CL(tid=%d): clEnqueueMapImage_Pre, Event:%p, ErrCodeRet:%d\n", tid, Event, ErrCodeRet?*ErrCodeRet:0);
    return gcvNULL;
}

void* LogclEnqueueMapImage_Post(
    cl_command_queue    CommandQueue,
    cl_mem              Image,
    cl_bool             BlockingMap,
    cl_map_flags        MapFlags,
    const size_t *      Origin,
    const size_t *      Region,
    size_t *            ImageRowPitch,
    size_t *            ImageSlicePitch,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event,
    cl_int *            ErrCodeRet,
    void *              Ptr
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clEnqueueMapImage_Post, CommandQueue:%p, Image:%p, BlockingMap:%d\n", tid, CommandQueue, Image, BlockingMap);
    CL_LOG_API("CL(tid=%d): clEnqueueMapImage_Post, MapFlags:0x%x, ImageRowPitch:%d, ImageSlicePitch:%d, NumEventsInWaitList:%d\n", tid, MapFlags, (int)*ImageRowPitch, (int)(ImageSlicePitch ? *ImageSlicePitch : (size_t)-1), (int)NumEventsInWaitList);
    CL_LOG_API("CL(tid=%d): clEnqueueMapImage_Post, Origin[0]:%d, Origin[1]:%d, Origin[2]:%d, Region[0]:%d, Region[1]:%d, Region[2]:%d\n", tid, Origin[0], Origin[1], Origin[2], Region[0], Region[1], Region[2]);
    for(i=0; i< NumEventsInWaitList; i++)
    {
        CL_LOG_API("CL(tid=%d): clEnqueueMapImage_Post, EventWaitList[%d]:%p\n", tid, i, EventWaitList[i]);
    }
    CL_LOG_API("CL(tid=%d): clEnqueueMapImage_Post, Event:%p, ErrCodeRet:%d, Ptr:%p\n", tid, Event, ErrCodeRet?*ErrCodeRet:0, Ptr);
    return gcvNULL;
}

cl_int LogclEnqueueUnmapMemObject(
    cl_command_queue    CommandQueue,
    cl_mem              MemObj,
    void *              MappedPtr,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event
    )

{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clEnqueueUnmapMemObject, CommandQueue:%p, MemObj:%p, MappedPtr:%p, NumEventsInWaitList:%d\n", tid, CommandQueue, MemObj, MappedPtr, NumEventsInWaitList);
    for(i=0; i< NumEventsInWaitList; i++)
    {
        CL_LOG_API("CL(tid=%d): clEnqueueUnmapMemObject, EventWaitList[%d]:%p\n", tid, i, EventWaitList[i]);
    }
    CL_LOG_API("CL(tid=%d): clEnqueueUnmapMemObject, Event:%p\n", tid, Event);
    return 0;
}

cl_int LogclEnqueueMigrateMemObjects(
    cl_command_queue        CommandQueue ,
    cl_uint                 NumMemObjects ,
    const cl_mem *          MemObjects ,
    cl_mem_migration_flags  Flags ,
    cl_uint                 NumEventsInWaitList ,
    const cl_event *        EventWaitList ,
    cl_event *              Event
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clEnqueueMigrateMemObjects, CommandQueue:%p, NumMemObjects:%d, Flags:0x%x, NumEventsInWaitList:%d\n", tid, CommandQueue, NumMemObjects, Flags, NumEventsInWaitList);
    for(i=0; i<NumMemObjects; i++)
    {
        CL_LOG_API("CL(tid=%d): clEnqueueMigrateMemObjects, MemObjects[%d]:%p\n", tid, i, MemObjects[i]);
    }
    for(i=0; i<NumEventsInWaitList; i++)
    {
        CL_LOG_API("CL(tid=%d): clEnqueueMigrateMemObjects, EventWaitList[%d]:%p\n", tid, i, EventWaitList[i]);
    }
    CL_LOG_API("CL(tid=%d): clEnqueueUnmapMemObject, Event:%p\n", tid, Event);
    return 0;
}

cl_int LogclEnqueueNDRangeKernel(
    cl_command_queue    CommandQueue,
    cl_kernel           Kernel,
    cl_uint             WorkDim,
    const size_t *      GlobalWorkOffset,
    const size_t *      GlobalWorkSize,
    const size_t *      LocalWorkSize,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clEnqueueNDRangeKernel, CommandQueue:%p, Kernel:%p, WorkDim:%d\n", tid, CommandQueue, Kernel, WorkDim);
    for(i=0; i<WorkDim; i++)
    {
           CL_LOG_API("CL(tid=%d): clEnqueueNDRangeKernel, GlobalWorkOffset[%d]:%d, GlobalWorkSize[%d]:%d, LocalWorkSize[%d]:%d\n", tid, i, GlobalWorkOffset?GlobalWorkOffset[i]:0, i, GlobalWorkSize[i], i,LocalWorkSize? LocalWorkSize[i]:0);
    }
    for(i=0; i< NumEventsInWaitList; i++)
    {
        CL_LOG_API("CL(tid=%d): clEnqueueNDRangeKernel, EventWaitList[%d]:%p\n", tid, i, EventWaitList[i]);
    }
    CL_LOG_API("CL(tid=%d): clEnqueueNDRangeKernel, Event:%p\n", tid, Event);
    return 0;
}

cl_int LogclEnqueueTask(
    cl_command_queue    CommandQueue,
    cl_kernel           Kernel,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clEnqueueNDRangeKernel, CommandQueue:%p, Kernel:%p, NumEventsInWaitList:%d\n", tid, CommandQueue, Kernel, NumEventsInWaitList);
    for(i=0; i< NumEventsInWaitList; i++)
    {
        CL_LOG_API("CL(tid=%d): clEnqueueNDRangeKernel, EventWaitList[%d]:%p\n", tid, i, EventWaitList[i]);
    }
    CL_LOG_API("CL(tid=%d): clEnqueueNDRangeKernel, Event:%p\n", tid, Event);
    return 0;
}

cl_int LogclEnqueueNativeKernel(
    cl_command_queue    CommandQueue,
    void                (CL_CALLBACK * UserFunc)(void *),
    void *              Args,
    size_t              CbArgs,
    cl_uint             NumMemObjects,
    const cl_mem *      MemList,
    const void **       ArgsMemLoc,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clEnqueueNativeKernel, CommandQueue:%p, UserFunc:%p, Args:%p, CbArgs:%d\n", tid, CommandQueue, UserFunc, Args, CbArgs);
    CL_LOG_API("CL(tid=%d): clEnqueueNativeKernel, MemList:%p, ArgsMemLoc:%p, NumEventsInWaitList:%d, Event:0x%x\n", tid, MemList, ArgsMemLoc, NumEventsInWaitList, Event);
    for(i=0; i< NumEventsInWaitList; i++)
    {
        CL_LOG_API("CL(tid=%d): clEnqueueNDRangeKernel, EventWaitList[%d]:%p\n", tid, i, EventWaitList[i]);
    }
    return 0;
}

cl_int LogclEnqueueMarkerWithWaitList(
    cl_command_queue    CommandQueue,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clEnqueueMarkerWithWaitList, CommandQueue:%p, NumEventsInWaitList:%d, Event:%p\n", tid, CommandQueue, NumEventsInWaitList, Event);
    for(i=0; i< NumEventsInWaitList; i++)
    {
        CL_LOG_API("CL(tid=%d): clEnqueueMarkerWithWaitList, EventWaitList[%d]:%p\n", tid, i, EventWaitList[i]);
    }
    return 0;
}

cl_int LogclEnqueueBarrierWithWaitList(
    cl_command_queue   CommandQueue ,
    cl_uint            NumEventsInWaitList ,
    const cl_event *   EventWaitList ,
    cl_event *         Event
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clEnqueueBarrierWithWaitList, CommandQueue:%p, NumEventsInWaitList:%d, Event:%p\n", tid, CommandQueue, NumEventsInWaitList, Event);
    for(i=0; i< NumEventsInWaitList; i++)
    {
        CL_LOG_API("CL(tid=%d): clEnqueueBarrierWithWaitList, EventWaitList[%d]:%p\n", tid, i, EventWaitList[i]);
    }
    return 0;
}

void * LogclGetExtensionFunctionAddressForPlatform_Pre(
    cl_platform_id Platform ,
    const char *   FuncName
    )
{
    CL_LOG_API("CL(tid=%d): clGetExtensionFunctionAddress, Platform:%p, FuncName:%s\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), Platform, FuncName);
    return gcvNULL;
}

void * LogclGetExtensionFunctionAddressForPlatform_Post(
    cl_platform_id Platform ,
    const char *   FuncName ,
    void *         AddressPtr
    )
{
    CL_LOG_API("CL(tid=%d): clGetExtensionFunctionAddress, Platform:%p, FuncName:%s, AddressPtr:%p\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), Platform, FuncName, AddressPtr);
    return gcvNULL;
}

cl_mem LogclCreateImage2D_Pre(
    cl_context              Context,
    cl_mem_flags            Flags,
    const cl_image_format * ImageFormat,
    size_t                  ImageWidth,
    size_t                  ImageHeight,
    size_t                  ImageRowPitch,
    void *                  HostPtr,
    cl_int *                ErrcodeRet
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clCreateImage2D_Pre, context:%p, flags:0x%llx, hostPtr:%p, ErrcodeRet:%d\n", tid, Context, Flags, HostPtr, (ErrcodeRet?*ErrcodeRet:0));
    CL_LOG_API("CL(tid=%d): clCreateImage2D_Pre, image_channel_order:0x%x, image_channel_data_type:0x%x\n", tid, ImageFormat->image_channel_order, ImageFormat->image_channel_data_type);
    CL_LOG_API("CL(tid=%d): clCreateImage2D_Pre, width:%d, height:%d, ImageRowPitch:%d\n", tid, ImageWidth, ImageHeight, ImageRowPitch);
    return gcvNULL;
}

cl_mem LogclCreateImage2D_Post(
    cl_context              Context,
    cl_mem_flags            Flags,
    const cl_image_format * ImageFormat,
    size_t                  ImageWidth,
    size_t                  ImageHeight,
    size_t                  ImageRowPitch,
    void *                  HostPtr,
    cl_int *                ErrcodeRet,
    cl_mem                  image
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clCreateImage2D_Post, context:%p, flags:0x%llx, hostPtr:0x%x, ErrcodeRet:%d\n", tid, Context, Flags, HostPtr, ErrcodeRet?*ErrcodeRet:0);
    CL_LOG_API("CL(tid=%d): clCreateImage2D_Post, image_channel_order:0x%x, image_channel_data_type:0x%x\n", tid, ImageFormat->image_channel_order, ImageFormat->image_channel_data_type);
    CL_LOG_API("CL(tid=%d): clCreateImage2D_Post, width:%d, height:%d, ImageRowPitch:%d, image:%p\n", tid, ImageWidth, ImageHeight, ImageRowPitch, image);
    return gcvNULL;
}

cl_mem LogclCreateImage3D_Pre(
    cl_context              Context,
    cl_mem_flags            Flags,
    const cl_image_format * ImageFormat,
    size_t                  ImageWidth,
    size_t                  ImageHeight,
    size_t                  ImageDepth,
    size_t                  ImageRowPitch,
    size_t                  ImageSlicePitch,
    void *                  HostPtr,
    cl_int *                ErrcodeRet
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clCreateImage3D_Pre, context:%p, flags:0x%llx, hostPtr:%p, ErrcodeRet:%d\n", tid, Context, Flags, HostPtr, ErrcodeRet?*ErrcodeRet:0);
    CL_LOG_API("CL(tid=%d): clCreateImage3D_Pre, image_channel_order:0x%x, image_channel_data_type:0x%x\n", tid, ImageFormat->image_channel_order, ImageFormat->image_channel_data_type);
    CL_LOG_API("CL(tid=%d): clCreateImage3D_Pre, width:%d, height:%d, depth:%d, ImageRowPitch:%d, ImageSlicePitch:%d\n", tid, ImageWidth, ImageHeight, ImageDepth, ImageRowPitch, ImageSlicePitch);
    return gcvNULL;
}

cl_mem LogclCreateImage3D_Post(
    cl_context              Context,
    cl_mem_flags            Flags,
    const cl_image_format * ImageFormat,
    size_t                  ImageWidth,
    size_t                  ImageHeight,
    size_t                  ImageDepth,
    size_t                  ImageRowPitch,
    size_t                  ImageSlicePitch,
    void *                  HostPtr,
    cl_int *                ErrcodeRet,
    cl_mem                  image
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clCreateImage2D_Post, context:%p, flags:0x%llx, hostPtr:%p, ErrcodeRet:%d\n", tid, Context, Flags, HostPtr, ErrcodeRet?*ErrcodeRet:0);
    CL_LOG_API("CL(tid=%d): clCreateImage2D_Post, image_channel_order:0x%x, image_channel_data_type:0x%x\n", tid, ImageFormat->image_channel_order, ImageFormat->image_channel_data_type);
    CL_LOG_API("CL(tid=%d): clCreateImage3D_Pre, width:%d, height:%d, depth:%d, ImageRowPitch:%d, ImageSlicePitch:%d, image:%p\n", tid, ImageWidth, ImageHeight, ImageDepth, ImageRowPitch, ImageSlicePitch, image);
    return gcvNULL;
}

cl_int LogclEnqueueMarker(
    cl_command_queue    CommandQueue,
    cl_event *          Event
    )
{
    CL_LOG_API("CL(tid=%d): clEnqueueMarker, CommandQueue:%p, Event:%p\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), CommandQueue, Event);
    return 0;
}

cl_int LogclEnqueueWaitForEvents(
    cl_command_queue    CommandQueue,
    cl_uint             NumEvents,
    const cl_event *    EventList
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clEnqueueWaitForEvents, CommandQueue:%p, NumEvents:%d\n", tid, CommandQueue, NumEvents);
    for(i=0; i<NumEvents; i++)
    {
        CL_LOG_API("CL(tid=%d): clEnqueueWaitForEvents, EventList[%d]:%p\n", tid, i, EventList[i]);
    }
    return 0;
}

cl_int LogclEnqueueBarrier(
    cl_command_queue    CommandQueue
    )
{
    CL_LOG_API("CL(tid=%d): clEnqueueBarrier, CommandQueue:%p\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), CommandQueue);
    return 0;
}

cl_int LogclUnloadCompiler(
    void
    )
{
    CL_LOG_API("CL(tid=%d): clUnloadCompiler\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID());
    return 0;
}

void * LogclGetExtensionFunctionAddress_Pre(
    const char *    FuncName
    )
{
    CL_LOG_API("CL(tid=%d): clGetExtensionFunctionAddress_Pre, FuncName:%s\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), FuncName);
    return gcvNULL;
}

void * LogclGetExtensionFunctionAddress_Post(
    const char *    FuncName,
    void *          AddressPtr
    )
{
    CL_LOG_API("CL(tid=%d): clGetExtensionFunctionAddress_Pre, FuncName:%s, AddressPtr:%p\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), FuncName, AddressPtr);
    return gcvNULL;
}

cl_int LogclIcdGetPlatformIDsKHR(cl_uint              NumEntries,
                                 cl_platform_id *     Platforms,
                                 cl_uint *            NumPlatforms)
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_int i   = 0;
    cl_int count = NumEntries;

    if(NumPlatforms)
    {
        count = NumEntries<(*NumPlatforms)?NumEntries:(*NumPlatforms);
    }
    CL_LOG_API("CL(tid=%d): clIcdGetPlatformIDsKHR, NumEntries:%d, NumPlatforms:%d\n", tid, NumEntries, NumPlatforms?*NumPlatforms:0);
    if(Platforms)
    {
        for(i=0; i<count; i++)
        {
            CL_LOG_API("CL(tid=%d): clIcdGetPlatformIDsKHR, Platforms[%d]:%p\n", tid, i, Platforms[i]);
        }
    }
    return 0;
}

cl_mem LogclCreateFromGLBuffer_Pre(
    cl_context      Context,
    cl_mem_flags    Flags,
    cl_GLuint       BufObj,
    int *           ErrcodeRet
    )
{
    CL_LOG_API("CL(tid=%d): clCreateFromGLBuffer_Pre, Context:%p, Flags:0x%llx, BufObj:%d, ErrcodeRet:%d\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), Context, Flags, BufObj, ErrcodeRet?*ErrcodeRet:0);
    return gcvNULL;
}

cl_mem LogclCreateFromGLBuffer_Post(
    cl_context      Context,
    cl_mem_flags    Flags,
    cl_GLuint       BufObj,
    int *           ErrcodeRet,
    cl_mem          memObj
    )
{
    CL_LOG_API("CL(tid=%d): clCreateFromGLBuffer_Post, Context:%p, Flags:0x%llx, BufObj:%d, ErrcodeRet:%d, memObj:%p\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), Context, Flags, BufObj, ErrcodeRet?*ErrcodeRet:0, memObj);
    return 0;
}

cl_mem LogclCreateFromGLTexture_Pre(
    cl_context      Context,
    cl_mem_flags    Flags,
    cl_GLenum       Target,
    cl_GLint        MipLevel,
    cl_GLuint       Texture,
    cl_int *        ErrcodeRet
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clCreateFromGLTexture_Pre, Context:%p, Flags:0x%llx, Target:%d\n", tid, Context, Flags, Target);
    CL_LOG_API("CL(tid=%d): clCreateFromGLTexture_Pre, MipLevel:%d, Texture:%d, ErrcodeRet:%d\n", tid, MipLevel, Texture, ErrcodeRet?*ErrcodeRet:0);
    return gcvNULL;
}

cl_mem LogclCreateFromGLTexture_Post(
    cl_context      Context,
    cl_mem_flags    Flags,
    cl_GLenum       Target,
    cl_GLint        MipLevel,
    cl_GLuint       Texture,
    cl_int *        ErrcodeRet,
    cl_mem          memObj
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clCreateFromGLTexture_Pre, Context:%p, Flags:0x%llx, Target:%d\n", tid, Context, Flags, Target);
    CL_LOG_API("CL(tid=%d): clCreateFromGLTexture_Pre, MipLevel:%d, Texture:%d, ErrcodeRet:%d, memObj:%p\n", tid, MipLevel, Texture, ErrcodeRet?*ErrcodeRet:0, memObj);
    return gcvNULL;
}

cl_mem LogclCreateFromGLRenderbuffer_Pre(
    cl_context      Context,
    cl_mem_flags    Flags,
    cl_GLuint       Renderbuffer,
    cl_int *        ErrcodeRet
    )
{
    CL_LOG_API("CL(tid=%d): clCreateFromGLRenderbuffer_Pre, Context:%p, Flags:0x%llx, Renderbuffer:%d, ErrcodeRet:%d\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), Context, Flags, Renderbuffer, ErrcodeRet?*ErrcodeRet:0);
    return gcvNULL;
}

cl_mem LogclCreateFromGLRenderbuffer_Post(
    cl_context      Context,
    cl_mem_flags    Flags,
    cl_GLuint       Renderbuffer,
    cl_int *        ErrcodeRet,
    cl_mem          memObj
    )
{
    CL_LOG_API("CL(tid=%d): clCreateFromGLRenderbuffer_Post, Context:%p, Flags:0x%llx, Renderbuffer:%d, ErrcodeRet:%d, memObj:%p\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), Context, Flags, Renderbuffer, ErrcodeRet?*ErrcodeRet:0, memObj);
    return gcvNULL;
}

cl_int LogclGetGLObjectInfo(
    cl_mem                MemObj,
    cl_gl_object_type *   GLObjectType,
    cl_GLuint *           GLObjectName
    )
{
    CL_LOG_API("CL(tid=%d): clGetGLObjectInfo, MemObj:%p, GLObjectType:%d, GLObjectName:%d\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), MemObj, (GLObjectType?*GLObjectType:0), (GLObjectName?*GLObjectName:0));
    return 0;
}

cl_int LogclGetGLTextureInfo(
    cl_mem               MemObj,
    cl_gl_texture_info   ParamName,
    size_t               ParamValueSize,
    void *               ParamValue,
    size_t *             ParamValueSizeRet
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clGetGLTextureInfo, MemObj:%p, ParamName:%d, ParamValueSize:%d, ParamValueSizeRet:%d, ParamValue:%p\n", tid, MemObj, ParamName, ParamValueSize, ParamValueSizeRet?*ParamValueSizeRet:0, ParamValue);
    return 0;
}

cl_int LogclEnqueueAcquireGLObjects(
    cl_command_queue      CommandQueue,
    cl_uint               NumObjects,
    const cl_mem *        MemObjects,
    cl_uint               NumEventsInWaitList,
    const cl_event *      EventWaitList,
    cl_event *            Event
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clEnqueueAcquireGLObjects, CommandQueue:%p, NumObjects:%d, NumEventsInWaitList:%d, Event:%p\n", tid, CommandQueue, NumObjects, NumEventsInWaitList, Event);
    for(i=0; i<NumObjects; i++)
    {
        CL_LOG_API("CL(tid=%d): clEnqueueAcquireGLObjects, MemObjects[%d]:%p\n", tid, i, MemObjects[i]);
    }
    for(i=0; i< NumEventsInWaitList; i++)
    {
        CL_LOG_API("CL(tid=%d): clEnqueueAcquireGLObjects, EventWaitList[%d]:%p\n", tid, i, EventWaitList[i]);
    }
    return 0;
}

cl_int LogclEnqueueReleaseGLObjects(
    cl_command_queue      CommandQueue,
    cl_uint               NumObjects,
    const cl_mem *        MemObjects,
    cl_uint               NumEventsInWaitList,
    const cl_event *      EventWaitList,
    cl_event *            Event
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    cl_uint i   = 0;
    CL_LOG_API("CL(tid=%d): clEnqueueReleaseGLObjects, CommandQueue:%p, NumObjects:%d, NumEventsInWaitList:%d, Event:%p\n", tid, CommandQueue, NumObjects, NumEventsInWaitList, Event);
    for(i=0; i<NumObjects; i++)
    {
        CL_LOG_API("CL(tid=%d): clEnqueueReleaseGLObjects, MemObjects[%d]:%p\n", tid, i, MemObjects[i]);
    }
    for(i=0; i< NumEventsInWaitList; i++)
    {
        CL_LOG_API("CL(tid=%d): clEnqueueReleaseGLObjects, EventWaitList[%d]:%p\n", tid, i, EventWaitList[i]);
    }
    return 0;
}

cl_mem LogclCreateFromGLTexture2D_Pre(
    cl_context      Context,
    cl_mem_flags    Flags,
    cl_GLenum       Target,
    cl_GLint        MipLevel,
    cl_GLuint       Texture,
    cl_int *        ErrcodeRet
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clCreateFromGLTexture2D_Pre, Context:%p, Flags:0x%llx, Target:%d\n", tid, Context, Flags, Target);
    CL_LOG_API("CL(tid=%d): clCreateFromGLTexture2D_Pre, MipLevel:%d, Texture:%d, ErrcodeRet:%d\n", tid, MipLevel, Texture, ErrcodeRet?*ErrcodeRet:0);
    return gcvNULL;
}

cl_mem LogclCreateFromGLTexture2D_Post(
    cl_context      Context,
    cl_mem_flags    Flags,
    cl_GLenum       Target,
    cl_GLint        MipLevel,
    cl_GLuint       Texture,
    cl_int *        ErrcodeRet,
    cl_mem          memObj
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clCreateFromGLTexture2D_Pre, Context:%p, Flags:0x%llx, Target:%d\n", tid, Context, Flags, Target);
    CL_LOG_API("CL(tid=%d): clCreateFromGLTexture2D_Pre, MipLevel:%d, Texture:%d, ErrcodeRet:%d, memObj:%p\n", tid, MipLevel, Texture, ErrcodeRet?*ErrcodeRet:0, memObj);
    return gcvNULL;
}

cl_mem LogclCreateFromGLTexture3D_Pre(
    cl_context      Context,
    cl_mem_flags    Flags,
    cl_GLenum       Target,
    cl_GLint        MipLevel,
    cl_GLuint       Texture,
    cl_int *        ErrcodeRet
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clCreateFromGLTexture3D_Pre, Context:%p, Flags:0x%llx, Target:%d\n", tid, Context, Flags, Target);
    CL_LOG_API("CL(tid=%d): clCreateFromGLTexture3D_Pre, MipLevel:%d, Texture:%d, ErrcodeRet:%d\n", tid, MipLevel, Texture, ErrcodeRet?*ErrcodeRet:0);
    return gcvNULL;
}

cl_mem LogclCreateFromGLTexture3D_Post(
    cl_context      Context,
    cl_mem_flags    Flags,
    cl_GLenum       Target,
    cl_GLint        MipLevel,
    cl_GLuint       Texture,
    cl_int *        ErrcodeRet,
    cl_mem          memObj
    )
{
    cl_int tid = (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID();
    CL_LOG_API("CL(tid=%d): clCreateFromGLTexture3D_Post, Context:%p, Flags:0x%llx, Target:%d\n", tid, Context, Flags, Target);
    CL_LOG_API("CL(tid=%d): clCreateFromGLTexture3D_Post, MipLevel:%d, Texture:%d, ErrcodeRet:%d, memObj:%p\n", tid, MipLevel, Texture, ErrcodeRet?*ErrcodeRet:0, memObj);
    return gcvNULL;
}

cl_int LogclGetGLContextInfoKHR(
    const cl_context_properties * Properties,
    cl_gl_context_info            ParamName,
    size_t                        ParamValueSize,
    void *                        ParamValue,
    size_t *                      ParamValueSizeRet
    )
{
    CL_LOG_API("CL(tid=%d): clGetGLContextInfoKHR, Properties:%p, ParamName:%d, ParamValueSize:%d, ParamValue:%d, ParamValueSizeRet:%d\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), Properties, ParamName, ParamValueSize, ParamValue ? *((gctUINT *)ParamValue) : 0, ParamValueSizeRet?*ParamValueSizeRet:0);
    return 0;
}

cl_event LogclCreateEventFromGLsyncKHR_Pre(
    cl_context context,
    cl_GLsync sync,
    cl_int * errcode_ret)
{
    CL_LOG_API("CL(tid=%d): clCreateEventFromGLsyncKHR_Pre, context:%p, sync:%p, errcode_ret:%d\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), context, sync, errcode_ret?*errcode_ret:0);
    return gcvNULL;
}

cl_event LogclCreateEventFromGLsyncKHR_Post(
    cl_context  context,
    cl_GLsync   sync,
    cl_int      * errcode_ret,
    cl_event    Event)
{
    CL_LOG_API("CL(tid=%d): clCreateEventFromGLsyncKHR_Pre, context:%p, sync:%p, errcode_ret:%d, event:%p\n", (cl_int)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), context, sync, errcode_ret?*errcode_ret:0, Event);
    return gcvNULL;
}

clsTracerDispatchTableStruct vclTracerDispatchTable = {0};
clsTracerDispatchTableStruct vclLogFunctionTable={
    LogclGetPlatformIDs,
    LogclGetPlatformInfo,
    LogclGetDeviceIDs,
    LogclGetDeviceInfo,
    LogclCreateSubDevices,
    LogclRetainDevice,
    LogclReleaseDevice,
    LogclCreateContext_Pre,
    LogclCreateContext_Post,
    LogclCreateContextFromType_Pre,
    LogclCreateContextFromType_Post,
    LogclRetainContext,
    LogclReleaseContext,
    LogclGetContextInfo,
    LogclCreateCommandQueue_Pre,
    LogclCreateCommandQueue_Post,
    LogclRetainCommandQueue,
    LogclReleaseCommandQueue,
    LogclGetCommandQueueInfo,
    LogclCreateBuffer_Pre,
    LogclCreateBuffer_Post,
    LogclCreateSubBuffer_Pre,
    LogclCreateSubBuffer_Post,
    LogclCreateImage_Pre,
    LogclCreateImage_Post,
    LogclRetainMemObject,
    LogclReleaseMemObject,
    LogclGetSupportedImageFormats,
    LogclGetMemObjectInfo,
    LogclGetImageInfo,
    LogclSetMemObjectDestructorCallback,
    LogclCreateSampler_Pre,
    LogclCreateSampler_Post,
    LogclRetainSampler,
    LogclReleaseSampler,
    LogclGetSamplerInfo,
    LogclCreateProgramWithSource_Pre,
    LogclCreateProgramWithSource_Post,
    LogclCreateProgramWithBinary_Pre,
    LogclCreateProgramWithBinary_Post,
    LogclCreateProgramWithBuiltInKernels_Pre,
    LogclCreateProgramWithBuiltInKernels_Post,
    LogclRetainProgram,
    LogclReleaseProgram,
    LogclBuildProgram,
    LogclCompileProgram,
    LogclLinkProgram_Pre,
    LogclLinkProgram_Post,
    LogclUnloadPlatformCompiler,
    LogclGetProgramInfo,
    LogclGetProgramBuildInfo,
    LogclCreateKernel_Pre,
    LogclCreateKernel_Post,
    LogclCreateKernelsInProgram,
    LogclRetainKernel,
    LogclReleaseKernel,
    LogclSetKernelArg,
    LogclGetKernelInfo,
    LogclGetKernelArgInfo,
    LogclGetKernelWorkGroupInfo,
    LogclWaitForEvents,
    LogclGetEventInfo,
    LogclCreateUserEvent_Pre,
    LogclCreateUserEvent_Post,
    LogclRetainEvent,
    LogclReleaseEvent,
    LogclSetUserEventStatus,
    LogclSetEventCallback,
    LogclGetEventProfilingInfo,
    LogclFlush,
    LogclFinish,
    LogclEnqueueReadBuffer,
    LogclEnqueueReadBufferRect,
    LogclEnqueueWriteBuffer,
    LogclEnqueueWriteBufferRect,
    LogclEnqueueFillBuffer,
    LogclEnqueueCopyBuffer,
    LogclEnqueueCopyBufferRect,
    LogclEnqueueReadImage,
    LogclEnqueueWriteImage,
    LogclEnqueueFillImage,
    LogclEnqueueCopyImage,
    LogclEnqueueCopyImageToBuffer,
    LogclEnqueueCopyBufferToImage,
    LogclEnqueueMapBuffer_Pre,
    LogclEnqueueMapBuffer_Post,
    LogclEnqueueMapImage_Pre,
    LogclEnqueueMapImage_Post,
    LogclEnqueueUnmapMemObject,
    LogclEnqueueMigrateMemObjects,
    LogclEnqueueNDRangeKernel,
    LogclEnqueueTask,
    LogclEnqueueNativeKernel,
    LogclEnqueueMarkerWithWaitList,
    LogclEnqueueBarrierWithWaitList,
    LogclGetExtensionFunctionAddressForPlatform_Pre,
    LogclGetExtensionFunctionAddressForPlatform_Post,
    LogclCreateImage2D_Pre,
    LogclCreateImage2D_Post,
    LogclCreateImage3D_Pre,
    LogclCreateImage3D_Post,
    LogclEnqueueMarker,
    LogclEnqueueWaitForEvents,
    LogclEnqueueBarrier,
    LogclUnloadCompiler,
    LogclGetExtensionFunctionAddress_Pre,
    LogclGetExtensionFunctionAddress_Post,
    LogclIcdGetPlatformIDsKHR,
    LogclCreateFromGLBuffer_Pre,
    LogclCreateFromGLBuffer_Post,
    LogclCreateFromGLTexture_Pre,
    LogclCreateFromGLTexture_Post,
    LogclCreateFromGLRenderbuffer_Pre,
    LogclCreateFromGLRenderbuffer_Post,
    LogclGetGLObjectInfo,
    LogclGetGLTextureInfo,
    LogclEnqueueAcquireGLObjects,
    LogclEnqueueReleaseGLObjects,
    LogclCreateFromGLTexture2D_Pre,
    LogclCreateFromGLTexture2D_Post,
    LogclCreateFromGLTexture3D_Pre,
    LogclCreateFromGLTexture3D_Post,
    LogclGetGLContextInfoKHR,
    LogclCreateEventFromGLsyncKHR_Pre,
    LogclCreateEventFromGLsyncKHR_Post
};

