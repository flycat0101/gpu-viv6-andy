/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_cl_precomp.h"
#include "stdio.h"

#define __NEXT_MSG_ID__     001007

static struct _cl_device_id _device =
{
    gcvNULL,                    /* Dispatch. */
    clvOBJECT_DEVICE,           /* Object type.     */
    0,                          /* Id.              */
    gcvNULL,                    /* Reference count object. */

    gcvNULL,                    /* Platform         */
    "Vivante OpenCL Device GCxxxx.xxxx.xxxx",    /* Name             */
    "Vivante Corporation",      /* Vendor           */
    gcvNULL,                    /* Device Version   */
    gcvNULL,                    /* Driver Version   */
    gcvNULL,                    /* OpenCL C Version */
    "FULL_PROFILE",         /* Profile          */
#if BUILD_OPENCL_ICD
    "cl_khr_icd "
#endif
    "cl_khr_byte_addressable_store",
                                /* Extensions       */

    CL_DEVICE_TYPE_GPU,         /* Type             */
    clvDEVICE_VENDOR_ID,        /* Vendor ID        */

    {0,0,{0,0,0},0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     gcvFALSE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     gcvFALSE,gcvFALSE,0,0,0,gcvFALSE,gcvFALSE,gcvFALSE,0, 0, 0},
                                /* Device Info      */
};

#if defined (__QNXNTO__)
        static char *extension_w_atomic = "cl_khr_byte_addressable_store "
                                          "cl_khr_global_int32_base_atomics "
                                          "cl_khr_global_int32_extended_atomics "
                                          "cl_khr_local_int32_base_atomics "
                                          "cl_khr_local_int32_extended_atomics ";

        static char *extension_without_atomic = "cl_khr_byte_addressable_store ";
#else
    #if gcvVERSION_MAJOR >= 5
        #if defined(ANDROID)
            #if ANDROID_SDK_VERSION >= 20
                static char *extension_w_atomic = "cl_khr_byte_addressable_store "
                                                  "cl_khr_global_int32_base_atomics "
                                                  "cl_khr_global_int32_extended_atomics "
                                                  "cl_khr_local_int32_base_atomics "
                                                  "cl_khr_local_int32_extended_atomics "
                                                  "cl_khr_gl_sharing ";

                static char *extension_without_atomic = "cl_khr_byte_addressable_store "
                                                        "cl_khr_gl_sharing ";
                static char *extension_w_atomic_wo_glsharing = "cl_khr_byte_addressable_store "
                                                  "cl_khr_global_int32_base_atomics "
                                                  "cl_khr_global_int32_extended_atomics "
                                                  "cl_khr_local_int32_base_atomics "
                                                  "cl_khr_local_int32_extended_atomics ";

                static char *extension_without_atomic_wo_glsharing = "cl_khr_byte_addressable_store ";
            #else
                static char *extension_w_atomic = "cl_khr_byte_addressable_store "
                                                  "cl_khr_global_int32_base_atomics "
                                                  "cl_khr_global_int32_extended_atomics "
                                                  "cl_khr_local_int32_base_atomics "
                                                  "cl_khr_local_int32_extended_atomics ";

                static char *extension_without_atomic = "cl_khr_byte_addressable_store ";
            #endif
        #else
            static char *extension_w_atomic = "cl_khr_byte_addressable_store "
                                              "cl_khr_global_int32_base_atomics "
                                              "cl_khr_global_int32_extended_atomics "
                                              "cl_khr_local_int32_base_atomics "
                                              "cl_khr_local_int32_extended_atomics "
                                              "cl_khr_gl_sharing ";

            static char *extension_without_atomic = "cl_khr_byte_addressable_store "
                                                    "cl_khr_gl_sharing ";
        #endif
    #else
        static char *extension_w_atomic = "cl_khr_byte_addressable_store "
                                          "cl_khr_global_int32_base_atomics "
                                          "cl_khr_global_int32_extended_atomics "
                                          "cl_khr_local_int32_base_atomics "
                                          "cl_khr_local_int32_extended_atomics ";

        static char *extension_without_atomic = "cl_khr_byte_addressable_store ";
    #endif
#endif

cl_device_id clgDefaultDevice = gcvNULL;
clsDeviceId_PTR clgDevices = gcvNULL;

/*****************************************************************************\
|*                         Supporting functions                              *|
\*****************************************************************************/
void
clfGetDefaultDevice(
    clsDeviceId_PTR * Device
    )
{
    gcmHEADER_ARG("Device=%p", Device);

    if (clgDefaultDevice == gcvNULL)
    {
        clgDefaultDevice = &_device;
    }

    if (Device)
    {
        *Device = clgDefaultDevice;
    }

    gcmFOOTER_ARG("*Device=0x%x", gcmOPT_POINTER(Device));
}

/*****************************************************************************\
|*                           OpenCL Device API                               *|
\*****************************************************************************/
CL_API_ENTRY cl_int CL_API_CALL
clGetDeviceIDs(
    cl_platform_id   Platform,
    cl_device_type   DeviceType,
    cl_uint          NumEntries,
    cl_device_id *   Devices,
    cl_uint *        NumDevices
    )
{
    clsPlatformId_PTR   platform = Platform;
    gctINT              status;
    gcmHEADER_ARG("NumEntries=%u", NumEntries);
    gcmDUMP_API("${OCL clGetDeviceIDs %d}", NumEntries);

    gcoHAL_SetHardwareType(gcvNULL,gcvHARDWARE_3D);
    if (Devices && NumEntries == 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-001000: (clGetDeviceIDs) argument Devices is not NULL but argument NumEntries is 0.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (platform == gcvNULL)
    {
        clfGetDefaultPlatformID(&platform);
    }
    else if (platform->objectType != clvOBJECT_PLATFORM)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-001001: (clGetDeviceIDs) argument Platform is not valid plaftfrom object.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (platform->devices == gcvNULL)
    {
        gctUINT         numDevices;

        /* Query number GPUs. */
        gcoCL_QueryDeviceCount(&numDevices);

        if(clgDevices == gcvNULL)
        {
            gctPOINTER      pointer = gcvNULL;
            gctUINT         i;
            gctBOOL         version11 = gcvFALSE;

            /* Borrow compiler mutex for hardware initialization. */
            gcmVERIFY_OK(gcoOS_AcquireMutex(gcvNULL, platform->compilerMutex, gcvINFINITE));

            if (gcvSTATUS_TRUE != gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_PIPE_CL))
            {
                gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, platform->compilerMutex));

                gcmUSER_DEBUG_ERROR_MSG(
                        "OCL-001002: (clGetDeviceIDs) cannot initialized HW for OpenCL.\n");
                clmRETURN_ERROR(CL_INVALID_VALUE);
            }

            gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, platform->compilerMutex));

            /* Initialize default device info. */
            clfGetDefaultDevice(gcvNULL);
            clmONERROR(gcoCL_QueryDeviceInfo(&clgDefaultDevice->deviceInfo), CL_INVALID_VALUE);
            clgDefaultDevice->dispatch  = platform->dispatch;
            clgDefaultDevice->platform  = platform;

            version11 = ((clgDefaultDevice->deviceInfo.chipModel == gcv1500 && clgDefaultDevice->deviceInfo.chipRevision == 0x5246) ||
                    (clgDefaultDevice->deviceInfo.chipModel == gcv2000 && clgDefaultDevice->deviceInfo.chipRevision == 0x5108) ||
                    (clgDefaultDevice->deviceInfo.chipModel == gcv3000 && clgDefaultDevice->deviceInfo.chipRevision == 0x5451) ||
                    (clgDefaultDevice->deviceInfo.chipModel == gcv3000 && clgDefaultDevice->deviceInfo.chipRevision == 0x5513));
            if (version11)
            {
                /* No image3D support. */
                clgDefaultDevice->deviceInfo.image3DMaxWidth  = 0;
                clgDefaultDevice->deviceInfo.image3DMaxHeight = 0;
                clgDefaultDevice->deviceInfo.image3DMaxDepth  = 0;
                clgDefaultDevice->deviceVersion = cldVERSION11;
                clgDefaultDevice->driverVersion = clfVERSION11;
                clgDefaultDevice->openCLCVersion = clcVERSION11;
            }
            else
            {
                clgDefaultDevice->deviceVersion = cldVERSION12;
                clgDefaultDevice->driverVersion = clfVERSION12;
                clgDefaultDevice->openCLCVersion = clcVERSION12;
            }

            /* Allocate device array. */
            status = gcoOS_Allocate(gcvNULL, sizeof(clsDeviceId) * numDevices, &pointer);
            if (gcmIS_ERROR(status))
            {
                gcmUSER_DEBUG_ERROR_MSG(
                        "OCL-001003: (clGetDeviceIDs) cannot allocate memory for devices.\n");
                clmRETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
            }

            clgDevices = (clsDeviceId_PTR) pointer;
            for (i = 0; i < numDevices; i++)
            {
                gceCHIPMODEL  chipModel;
                gctUINT32 chipRevision;
                gcePATCH_ID patchId = platform->patchId;
                gctUINT offset;
                gctSTRING productName = gcvNULL;
#if defined(ANDROID) && (ANDROID_SDK_VERSION >= 20)
                gctBOOL skipCLGLSharingExtension = gcvFALSE;
#endif
                const gctSTRING epProfile = "EMBEDDED_PROFILE";
                gctBOOL chipEnableEP = gcvFALSE;
                chipModel = clgDefaultDevice->deviceInfo.chipModel;
                chipRevision = clgDefaultDevice->deviceInfo.chipRevision;

                chipEnableEP = ((chipModel == gcv1500 && chipRevision == 0x5246) ||
                        (chipModel == gcv2000 && chipRevision == 0x5108) ||
                        (chipModel == gcv3000 && chipRevision == 0x5513) ||
                        (chipModel == gcv3000 && chipRevision == 0x5451) ||
                        (chipModel == gcv5000));
                if((gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_SHADER_HAS_ATOMIC) != gcvSTATUS_TRUE) ||
                        (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_SHADER_HAS_RTNE) != gcvSTATUS_TRUE)   ||
                        chipEnableEP)
                {
                    /*if the features on the device are not availble, still report embedded profile even if BUILD_OPENCL_FP is 1*/
                    clgDefaultDevice->profile = epProfile;
                }

                gcoOS_MemCopy(&clgDevices[i], clgDefaultDevice, sizeof(clsDeviceId));
                clmONERROR(gcoOS_AtomIncrement(gcvNULL, clgGlobalId, (gctINT*)&clgDevices[i].id), CL_INVALID_VALUE);

#if defined(ANDROID) && (ANDROID_SDK_VERSION >= 20)
                if(patchId == gcvPATCH_COMPUTBENCH_CL)
                {
                    skipCLGLSharingExtension = gcvTRUE;
                }
#endif

                clgDevices[i].gpuId = i;
                if (clgDefaultDevice->deviceInfo.atomicSupport)
                {
#if defined(ANDROID) && (ANDROID_SDK_VERSION >= 20)
                    clgDevices[i].extensions = skipCLGLSharingExtension ? extension_w_atomic_wo_glsharing : extension_w_atomic;
#else
                    clgDevices[i].extensions = extension_w_atomic;
#endif
                }
                else
                {
                    clgDevices[i].extensions = extension_without_atomic;
                }

                gcoHAL_GetProductName(gcvNULL, &productName);
                if(chipModel == gcv3000 && chipRevision == 0x5450)
                {
                }

                offset = 0;
                gcmVERIFY_OK(gcoOS_PrintStrSafe(clgDevices[i].name,
                            64,
                            &offset,
                            "Vivante OpenCL Device %s.%04x.%04d",
                            productName,
                            chipRevision,
                            patchId));

                gcmOS_SAFE_FREE(gcvNULL, productName);
            }
        }

        platform->numDevices = numDevices;
        platform->devices    = clgDevices;
    }

    switch((DeviceType)&0xFFFFFFFF)
    {
    case CL_DEVICE_TYPE_CPU:
    case CL_DEVICE_TYPE_ACCELERATOR:
        if (Devices)
        {
            *Devices = gcvNULL;
        }
        if (NumDevices)
        {
            *NumDevices = 0;
        }
        clmRETURN_ERROR(CL_DEVICE_NOT_FOUND);
        break;

    case CL_DEVICE_TYPE_DEFAULT:
    case CL_DEVICE_TYPE_GPU:
    case CL_DEVICE_TYPE_ALL:
        if (Devices)
        {
            gctUINT num = NumEntries < platform->numDevices ? NumEntries : platform->numDevices;
            gctUINT i;

            for (i = 0; i < num; i++)
            {
                Devices[i] = platform->devices + i;
            }
            if (NumDevices)
            {
                *NumDevices = num;
            }
        }
        else if (NumDevices)
        {
            *NumDevices = platform->numDevices;
        }
        clmCHECK_ERROR(platform->numDevices == 0, CL_DEVICE_NOT_FOUND);
        break;

    default:
        clmRETURN_ERROR(CL_INVALID_DEVICE_TYPE);
    }


    VCL_TRACE_API(GetDeviceIDs)(Platform, DeviceType, NumEntries, Devices, NumDevices);
    gcmFOOTER_ARG("%d *Devices=0x%x *NumDevices=%u",
                  CL_SUCCESS, gcmOPT_POINTER(Devices),
                  gcmOPT_VALUE(NumDevices));
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

/*****************************************************************************\
|*   sub-device is not supported in our driver, only add the error check     *|
\*****************************************************************************/
CL_API_ENTRY cl_int CL_API_CALL
clCreateSubDevices(
    cl_device_id                         InDevice,
    const cl_device_partition_property * Properties ,
    cl_uint                              NumDevices,
    cl_device_id *                       OutDevices,
    cl_uint *                            NumDevicesRet
    )
{
    gctUINT i;
    gctUINT numSubDevices = 0;
    gctUINT maxSubComputeUnits = 0;
    gctUINT maxSubDevices = 0;
    gctINT  status;

    gcmHEADER_ARG("InDevice=0x%x Properties=0x%x NumDevices=%lu",
                  InDevice, Properties, NumDevices);
    gcmDUMP_API("${OCL clCreateSubDevices 0x%x, 0x%x, 0x%x}", InDevice, Properties, NumDevices);

    clmCHECK_ERROR(InDevice == gcvNULL, CL_INVALID_DEVICE);

    clmCHECK_ERROR(InDevice->objectType != clvOBJECT_DEVICE, CL_INVALID_DEVICE);

    /*get value of maxSubComputeUnits and maxSubDevices from InDevice's deviceInfo if sub-device is supported.*/
    /*maxSubComputeUnits = InDevice->deviceInfo.maxSubComputeUnits;
    maxSubDevices = InDevice->deviceInfo.maxSubDevices;*/

    if (Properties)
    {
        for (i = 0; Properties[i]; i++)
        {
            switch (Properties[i])
            {
            case CL_DEVICE_PARTITION_EQUALLY:
                i++;
                numSubDevices = InDevice->deviceInfo.maxComputeUnits / (gctUINT)Properties[i];
                break;

            case CL_DEVICE_PARTITION_BY_COUNTS:
                {
                    gctUINT totalSubComputeUnit = 0;
                    i++;
                    while(Properties[i] != CL_DEVICE_PARTITION_BY_COUNTS_LIST_END)
                    {
                        if ((Properties[i] < 0)
                            || ((gctUINT)(Properties[i]) > maxSubComputeUnits))
                        {
                            clmRETURN_ERROR(CL_INVALID_DEVICE_PARTITION_COUNT);
                        }

                        totalSubComputeUnit += (gctUINT)Properties[i];
                        if (totalSubComputeUnit > maxSubComputeUnits)
                        {
                            clmRETURN_ERROR(CL_INVALID_DEVICE_PARTITION_COUNT);
                        }

                        numSubDevices += 1;
                        if (numSubDevices > maxSubDevices)
                        {
                            clmRETURN_ERROR(CL_INVALID_DEVICE_PARTITION_COUNT);
                        }

                        i++;
                    }
                }
                break;

            case CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN:
                i++;
                if ((Properties[i] != CL_DEVICE_AFFINITY_DOMAIN_NUMA) && (Properties[i] != CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE)
                    && (Properties[i] != CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE) && (Properties[i] != CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE)
                    && (Properties[i] != CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE) && (Properties[i] != CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE))
                {
                    clmRETURN_ERROR(CL_INVALID_VALUE);
                }
                break;

            default:
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-001004: (clCreateSubDevices) invalid Properties[%d] (0x%x).\n", i, Properties[i]);
                clmRETURN_ERROR(CL_INVALID_VALUE);
            }
        }
    }

    if (OutDevices)
    {
        if (NumDevices < numSubDevices)
        {
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
    }

    if (NumDevicesRet)
    {
        *NumDevicesRet = numSubDevices;
    }

    VCL_TRACE_API(CreateSubDevices)(InDevice, Properties, NumDevices, OutDevices, NumDevicesRet);
    gcmFOOTER_ARG("%d *OutDevices=0x%x *NumDevicesRet=%u",
                  CL_DEVICE_PARTITION_FAILED, gcmOPT_POINTER(OutDevices),
                  gcmOPT_VALUE(NumDevicesRet));

    clmRETURN_ERROR(CL_DEVICE_PARTITION_FAILED);

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainDevice(cl_device_id  device )
{
     gctINT              status;

    gcmHEADER_ARG("Program=0x%x", device);
    gcmDUMP_API("${OCL clRetainDevice 0x%x}", device);

    if (device == gcvNULL ||
        device->objectType != clvOBJECT_DEVICE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-001005: (clRetainDevice) invalid Program.\n");
        clmRETURN_ERROR(CL_INVALID_DEVICE);
    }

    VCL_TRACE_API(RetainDevice)(device);
    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}


CL_API_ENTRY cl_int CL_API_CALL
clReleaseDevice(cl_device_id  device )
{
    gctINT              status;

    gcmHEADER_ARG("Device=0x%x", device);
    gcmDUMP_API("${OCL clReleaseDevice 0x%x}", device);

    if (device == gcvNULL ||
        device->objectType != clvOBJECT_DEVICE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-001006: (clReleaseDevice) invalid Device.\n");
        clmRETURN_ERROR(CL_INVALID_DEVICE);
    }

    VCL_TRACE_API(ReleaseDevice)(device);
    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetDeviceInfo(
    cl_device_id    Device,
    cl_device_info  ParamName,
    size_t          ParamValueSize,
    void *          ParamValue,
    size_t *        ParamValueSizeRet
    )
{
    gctSIZE_T        retParamSize = 0;
    gctSTRING        retParamStr = NULL;
    gctPOINTER       retParamPtr = NULL;
    gctBOOL          isRetParamString = gcvFALSE;
    size_t           retValue_size_t[3];
    cl_bitfield      retValue_bitfield;
    cl_bool          retValue_bool;
    gctINT           status;

    gcmHEADER_ARG("Device=0x%x ParamName=%u ParamValueSize=%lu ParamValue=0x%x",
                  Device, ParamName, ParamValueSize, ParamValue);
    gcmDUMP_API("${OCL clGetDeviceInfo 0x%x, 0x%x}", Device, ParamName);

    clmCHECK_ERROR(Device == gcvNULL, CL_INVALID_DEVICE);

    clmCHECK_ERROR(Device->objectType != clvOBJECT_DEVICE, CL_INVALID_DEVICE);

    switch (ParamName)
    {
    case CL_DEVICE_NAME:
        isRetParamString = gcvTRUE;
        retParamStr = Device->name;
        break;

    case CL_DEVICE_TYPE:
        retParamSize = gcmSIZEOF(Device->type);
        retParamPtr = &Device->type;
        break;

    case CL_DEVICE_VENDOR:
        isRetParamString = gcvTRUE;
        retParamStr = Device->vendor;
        break;

    case CL_DEVICE_OPENCL_C_VERSION:
        isRetParamString = gcvTRUE;
        retParamStr = Device->openCLCVersion;
        break;

    case CL_DEVICE_VENDOR_ID:
        retParamSize = gcmSIZEOF(Device->vendorId);
        retParamPtr = &Device->vendorId;
        break;

    case CL_DEVICE_PLATFORM:
        retParamSize = gcmSIZEOF(Device->platform);
        retParamPtr = &Device->platform;
        break;

    case CL_DEVICE_VERSION:
        isRetParamString = gcvTRUE;
        retParamStr = Device->deviceVersion;
        break;

    case CL_DEVICE_PROFILE:
        isRetParamString = gcvTRUE;
        retParamStr = Device->profile;
        break;

    case CL_DRIVER_VERSION:
        isRetParamString = gcvTRUE;
        retParamStr = Device->driverVersion;
        break;

    case CL_DEVICE_MAX_COMPUTE_UNITS:
        retParamSize = gcmSIZEOF(Device->deviceInfo.maxComputeUnits);
        retParamPtr = &Device->deviceInfo.maxComputeUnits;
        break;

    case CL_DEVICE_PARTITION_MAX_SUB_DEVICES:
    case CL_DEVICE_PARTITION_PROPERTIES:
    case CL_DEVICE_PARTITION_TYPE:
        /*sub device is not supported*/
        retValue_size_t[0] = 0;
        retParamSize = gcmSIZEOF((cl_uint)retValue_size_t[0]);
        retParamPtr = retValue_size_t;
        break;
    case CL_DEVICE_PARTITION_AFFINITY_DOMAIN:
        retValue_bitfield = 0;
        retParamSize = gcmSIZEOF(retValue_bitfield);
        retParamPtr = &retValue_bitfield;
        break;

    case CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:
        retParamSize = gcmSIZEOF(Device->deviceInfo.maxWorkItemDimensions);
        retParamPtr = &Device->deviceInfo.maxWorkItemDimensions;
        break;

    case CL_DEVICE_MAX_WORK_ITEM_SIZES:
        /*retParamSize = gcmSIZEOF(Device->deviceInfo.maxWorkItemSizes);
        retParamPtr = &Device->deviceInfo.maxWorkItemSizes;*/
        retValue_size_t[0] = Device->deviceInfo.maxWorkItemSizes[0];
        retValue_size_t[1] = Device->deviceInfo.maxWorkItemSizes[1];
        retValue_size_t[2] = Device->deviceInfo.maxWorkItemSizes[2];
        retParamSize = gcmSIZEOF(retValue_size_t);
        retParamPtr = retValue_size_t;
        break;

    case CL_DEVICE_MAX_WORK_GROUP_SIZE:
        /*retParamSize = gcmSIZEOF(Device->deviceInfo.maxWorkGroupSize);
        retParamPtr = &Device->deviceInfo.maxWorkGroupSize;*/
        retValue_size_t[0] = Device->deviceInfo.maxWorkGroupSize;
        retParamSize = gcmSIZEOF(retValue_size_t[0]);
        retParamPtr = retValue_size_t;
        break;

    case CL_DEVICE_MAX_CLOCK_FREQUENCY:
        retParamSize = gcmSIZEOF(Device->deviceInfo.clockFrequency);
        retParamPtr = &Device->deviceInfo.clockFrequency;
        break;

    case CL_DEVICE_IMAGE_SUPPORT:
        retParamSize = gcmSIZEOF(Device->deviceInfo.imageSupport);
        retParamPtr = &Device->deviceInfo.imageSupport;
        break;

    case CL_DEVICE_MAX_READ_IMAGE_ARGS:
        retParamSize = gcmSIZEOF(Device->deviceInfo.maxReadImageArgs);
        retParamPtr = &Device->deviceInfo.maxReadImageArgs;
        break;

    case CL_DEVICE_MAX_WRITE_IMAGE_ARGS:
        retParamSize = gcmSIZEOF(Device->deviceInfo.maxWriteImageArgs);
        retParamPtr = &Device->deviceInfo.maxWriteImageArgs;
        break;

    case CL_DEVICE_IMAGE2D_MAX_WIDTH:
        /*retParamSize = gcmSIZEOF(Device->deviceInfo.image2DMaxWidth);
        retParamPtr = &Device->deviceInfo.image2DMaxWidth;*/
        retValue_size_t[0] = Device->deviceInfo.image2DMaxWidth;
        retParamSize = gcmSIZEOF(retValue_size_t[0]);
        retParamPtr = retValue_size_t;
        break;

    case CL_DEVICE_IMAGE2D_MAX_HEIGHT:
        /*retParamSize = gcmSIZEOF(Device->deviceInfo.image2DMaxHeight);
        retParamPtr = &Device->deviceInfo.image2DMaxHeight;*/
        retValue_size_t[0] = Device->deviceInfo.image2DMaxHeight;
        retParamSize = gcmSIZEOF(retValue_size_t[0]);
        retParamPtr = retValue_size_t;
        break;

    case CL_DEVICE_IMAGE3D_MAX_WIDTH:
        /*retParamSize = gcmSIZEOF(Device->deviceInfo.image3DMaxWidth);
        retParamPtr = &Device->deviceInfo.image3DMaxWidth;*/
        retValue_size_t[0] = Device->deviceInfo.image3DMaxWidth;
        retParamSize = gcmSIZEOF(retValue_size_t[0]);
        retParamPtr = retValue_size_t;
        break;

    case CL_DEVICE_IMAGE3D_MAX_HEIGHT:
        /*retParamSize = gcmSIZEOF(Device->deviceInfo.image3DMaxHeight);
        retParamPtr = &Device->deviceInfo.image3DMaxHeight;*/
        retValue_size_t[0] = Device->deviceInfo.image3DMaxHeight;
        retParamSize = gcmSIZEOF(retValue_size_t[0]);
        retParamPtr = retValue_size_t;
        break;

    case CL_DEVICE_IMAGE_MAX_ARRAY_SIZE:
    case CL_DEVICE_IMAGE3D_MAX_DEPTH:
        /*retParamSize = gcmSIZEOF(Device->deviceInfo.image3DMaxDepth);
        retParamPtr = &Device->deviceInfo.image3DMaxDepth;*/
        retValue_size_t[0] = Device->deviceInfo.image3DMaxDepth;
        retParamSize = gcmSIZEOF(retValue_size_t[0]);
        retParamPtr = retValue_size_t;
        break;

    case CL_DEVICE_MAX_SAMPLERS:
        retParamSize = gcmSIZEOF(Device->deviceInfo.maxSamplers);
        retParamPtr = &Device->deviceInfo.maxSamplers;
        break;

    case CL_DEVICE_EXTENSIONS:
        isRetParamString = gcvTRUE;
        retParamStr = Device->extensions;
        break;

    case CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR:
    case CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR:
        retParamSize = gcmSIZEOF(Device->deviceInfo.vectorWidthChar);
        retParamPtr = &Device->deviceInfo.vectorWidthChar;
        break;

    case CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT:
    case CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT:
        retParamSize = gcmSIZEOF(Device->deviceInfo.vectorWidthShort);
        retParamPtr = &Device->deviceInfo.vectorWidthShort;
        break;

    case CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT:
    case CL_DEVICE_NATIVE_VECTOR_WIDTH_INT:
        retParamSize = gcmSIZEOF(Device->deviceInfo.vectorWidthInt);
        retParamPtr = &Device->deviceInfo.vectorWidthInt;
        break;

    case CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG:
    case CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG:
        retParamSize = gcmSIZEOF(Device->deviceInfo.vectorWidthLong);
        retParamPtr = &Device->deviceInfo.vectorWidthLong;
        break;

    case CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT:
    case CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT:
        retParamSize = gcmSIZEOF(Device->deviceInfo.vectorWidthFloat);
        retParamPtr = &Device->deviceInfo.vectorWidthFloat;
        break;

    case CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE:
    case CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE:
        retParamSize = gcmSIZEOF(Device->deviceInfo.vectorWidthDouble);
        retParamPtr = &Device->deviceInfo.vectorWidthDouble;
        break;

    case CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF:
    case CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF:
        retParamSize = gcmSIZEOF(Device->deviceInfo.vectorWidthHalf);
        retParamPtr = &Device->deviceInfo.vectorWidthHalf;
        break;

    case CL_DEVICE_MAX_PARAMETER_SIZE:
        /*retParamSize = gcmSIZEOF(Device->deviceInfo.maxParameterSize);
        retParamPtr = &Device->deviceInfo.maxParameterSize;*/
        retValue_size_t[0] = Device->deviceInfo.maxParameterSize;
        retParamSize = gcmSIZEOF(retValue_size_t[0]);
        retParamPtr = retValue_size_t;
        break;

    case CL_DEVICE_MEM_BASE_ADDR_ALIGN:
        retParamSize = gcmSIZEOF(Device->deviceInfo.memBaseAddrAlign);
        retParamPtr = &Device->deviceInfo.memBaseAddrAlign;
        break;

    case CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE:
        retParamSize = gcmSIZEOF(Device->deviceInfo.minDataTypeAlignSize);
        retParamPtr = &Device->deviceInfo.minDataTypeAlignSize;
        break;

    case CL_DEVICE_SINGLE_FP_CONFIG:
        retParamSize = gcmSIZEOF(Device->deviceInfo.singleFpConfig);
        retParamPtr = &Device->deviceInfo.singleFpConfig;
        break;

    case CL_DEVICE_DOUBLE_FP_CONFIG:
        retParamSize = gcmSIZEOF(Device->deviceInfo.doubleFpConfig);
        retParamPtr = &Device->deviceInfo.doubleFpConfig;
        break;

    case CL_DEVICE_ADDRESS_BITS:
        retParamSize = gcmSIZEOF(Device->deviceInfo.addrBits);
        retParamPtr = &Device->deviceInfo.addrBits;
        break;

    case CL_DEVICE_GLOBAL_MEM_SIZE:
        retParamSize = gcmSIZEOF(Device->deviceInfo.globalMemSize);
        retParamPtr = &Device->deviceInfo.globalMemSize;
        break;

    case CL_DEVICE_MAX_MEM_ALLOC_SIZE:
        retParamSize = gcmSIZEOF(Device->deviceInfo.maxMemAllocSize);
        retParamPtr = &Device->deviceInfo.maxMemAllocSize;
        break;

    case CL_DEVICE_GLOBAL_MEM_CACHE_TYPE:
        retParamSize = gcmSIZEOF(Device->deviceInfo.globalMemCacheType);
        retParamPtr = &Device->deviceInfo.globalMemCacheType;
        break;

    case CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE:
        retParamSize = gcmSIZEOF(Device->deviceInfo.globalMemCachelineSize);
        retParamPtr = &Device->deviceInfo.globalMemCachelineSize;
        break;

    case CL_DEVICE_GLOBAL_MEM_CACHE_SIZE:
        retParamSize = gcmSIZEOF(Device->deviceInfo.globalMemCacheSize);
        retParamPtr = &Device->deviceInfo.globalMemCacheSize;
        break;

    case CL_DEVICE_LOCAL_MEM_SIZE:
        retParamSize = gcmSIZEOF(Device->deviceInfo.localMemSize);
        retParamPtr = &Device->deviceInfo.localMemSize;
        break;

    case CL_DEVICE_LOCAL_MEM_TYPE:
        retParamSize = gcmSIZEOF(Device->deviceInfo.localMemType);
        retParamPtr = &Device->deviceInfo.localMemType;
        break;

    case CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE:
        retParamSize = gcmSIZEOF(Device->deviceInfo.maxConstantBufferSize);
        retParamPtr = &Device->deviceInfo.maxConstantBufferSize;
        break;

    case CL_DEVICE_MAX_CONSTANT_ARGS:
        retParamSize = gcmSIZEOF(Device->deviceInfo.maxConstantArgs);
        retParamPtr = &Device->deviceInfo.maxConstantArgs;
        break;

    case CL_DEVICE_ERROR_CORRECTION_SUPPORT:
        retParamSize = gcmSIZEOF(Device->deviceInfo.errorCorrectionSupport);
        retParamPtr = &Device->deviceInfo.errorCorrectionSupport;
        break;

    case CL_DEVICE_QUEUE_PROPERTIES:
        retParamSize = gcmSIZEOF(Device->deviceInfo.queueProperties);
        retParamPtr = &Device->deviceInfo.queueProperties;
        break;

    case CL_DEVICE_HOST_UNIFIED_MEMORY:
        retParamSize = gcmSIZEOF(Device->deviceInfo.hostUnifiedMemory);
        retParamPtr = &Device->deviceInfo.hostUnifiedMemory;
        break;

    case CL_DEVICE_PROFILING_TIMER_RESOLUTION:
        /*retParamSize = gcmSIZEOF(Device->deviceInfo.profilingTimingRes);
        retParamPtr = &Device->deviceInfo.profilingTimingRes;*/
        retValue_size_t[0] = Device->deviceInfo.profilingTimingRes;
        retParamSize = gcmSIZEOF(retValue_size_t[0]);
        retParamPtr = retValue_size_t;
        break;

    case CL_DEVICE_ENDIAN_LITTLE:
        retParamSize = gcmSIZEOF(Device->deviceInfo.endianLittle);
        retParamPtr = &Device->deviceInfo.endianLittle;
        break;

    case CL_DEVICE_AVAILABLE:
        retParamSize = gcmSIZEOF(Device->deviceInfo.deviceAvail);
        retParamPtr = &Device->deviceInfo.deviceAvail;
        break;

    case CL_DEVICE_COMPILER_AVAILABLE:
        retParamSize = gcmSIZEOF(Device->deviceInfo.compilerAvail);
        retParamPtr = &Device->deviceInfo.compilerAvail;
        break;

    case CL_DEVICE_EXECUTION_CAPABILITIES:
        retParamSize = gcmSIZEOF(Device->deviceInfo.execCapability);
        retParamPtr = &Device->deviceInfo.execCapability;
        break;

    case CL_DEVICE_IMAGE_MAX_BUFFER_SIZE:
        retParamSize = gcmSIZEOF(Device->deviceInfo.imageMaxBufferSize);
        retParamPtr = &Device->deviceInfo.imageMaxBufferSize;
        break;

    case CL_DEVICE_LINKER_AVAILABLE:
        retParamSize = gcmSIZEOF(Device->deviceInfo.linkerAvail);
        retParamPtr = &Device->deviceInfo.linkerAvail;
        break;

    case CL_DEVICE_PRINTF_BUFFER_SIZE:
        retParamSize = gcmSIZEOF(Device->deviceInfo.maxPrintfBufferSize);
        retParamPtr = &Device->deviceInfo.maxPrintfBufferSize;
        break;

    case CL_DEVICE_PREFERRED_INTEROP_USER_SYNC:
        retValue_bool = CL_TRUE;
        retParamSize = gcmSIZEOF(retValue_bool);
        retParamPtr = &retValue_bool;
        break;

    case CL_DEVICE_REFERENCE_COUNT:
        /* Always root device, value 1 is returned. */
        retValue_size_t[0] = 1;
        retParamSize = gcmSIZEOF((cl_uint)retValue_size_t[0]);
        retParamPtr = retValue_size_t;
        break;

    case CL_DEVICE_BUILT_IN_KERNELS:
    case CL_DEVICE_PARENT_DEVICE:
        retValue_size_t[0] = 0;
        retParamSize = gcmSIZEOF(retValue_size_t[0]);
        retParamPtr = retValue_size_t;
        break;

    default:
        gcmFOOTER_NO();
        return CL_INVALID_VALUE;
    }

    if (isRetParamString)
    {
        retParamSize = gcoOS_StrLen(retParamStr, gcvNULL) + 1;
        if (ParamValue)
        {
            clmCHECK_ERROR(ParamValueSize < retParamSize, CL_INVALID_VALUE);
            clmONERROR(gcoOS_StrCopySafe(ParamValue, retParamSize, retParamStr), CL_INVALID_VALUE);
        }
    }
    else
    {  /* Not string - treat as pointer */
        if (ParamValue)
        {
            clmCHECK_ERROR(ParamValueSize < retParamSize, CL_INVALID_VALUE);
            if (retParamSize)
            {
                gcoOS_MemCopy(ParamValue, retParamPtr, retParamSize);
            }
        }
    }

    if (ParamValueSizeRet)
    {
        *ParamValueSizeRet = retParamSize;
    }

    VCL_TRACE_API(GetDeviceInfo)(Device, ParamName, ParamValueSize, ParamValue, ParamValueSizeRet);
    gcmFOOTER_ARG("%d *ParamValueSizeRet=%lu",
                  CL_SUCCESS, gcmOPT_VALUE(ParamValueSizeRet));
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}
