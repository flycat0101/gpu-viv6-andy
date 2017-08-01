/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#define VK_USE_PLATFORM_ANDROID_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vk_android_native_buffer.h>

#include <hardware/hwvulkan.h>
#include <utils/Log.h>

#include <errno.h>
#include <dlfcn.h>

static int hwvulkan_device_open(const struct hw_module_t * module,
                const char * name, struct hw_device_t ** device);

static int hwvulkan_device_close(struct hw_device_t *);

static struct hw_module_methods_t hwvulkan_module_methods =
{
    .open = hwvulkan_device_open
};

/* hwvulkan module. */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-variable-declarations"
__attribute__((visibility("default"))) hwvulkan_module_t HAL_MODULE_INFO_SYM =
{
    .common =
    {
        .tag                = HARDWARE_MODULE_TAG,
        .module_api_version = HWVULKAN_MODULE_API_VERSION_0_1,
        .hal_api_version    = HARDWARE_HAL_API_VERSION,
        .id                 = HWVULKAN_HARDWARE_MODULE_ID,
        .name               = "Vivante Vulkan Driver",
        .author             = "Vivante Corporation",
        .methods            = &hwvulkan_module_methods,
    },
};
#pragma clang diagnostic pop

/* hwvulkan device. */
static hwvulkan_device_t hwvulkan_device =
{
    .common =
    {
        .tag     = HARDWARE_DEVICE_TAG,
        .version = HWVULKAN_DEVICE_API_VERSION_0_1,
        .module  = &HAL_MODULE_INFO_SYM.common,
        .close   = hwvulkan_device_close,
    },

    .EnumerateInstanceExtensionProperties = NULL,
    .CreateInstance = NULL,
    .GetInstanceProcAddr = NULL,
};

/* vulkan driver library handle. */
static void * vk_dso = NULL;

static void hwvulkan_device_init(void)
{
    const char * vklib[] =
    {
        "libvulkan_" GPU_VENDOR ".so",
        "/system/vendor/lib/libvulkan_" GPU_VENDOR ".so",
        "/system/lib/libvulkan_" GPU_VENDOR ".so",
    };

    for (size_t i = 0; i < sizeof(vklib) / sizeof(vklib[0]); i++) {
        vk_dso = dlopen(vklib[i], RTLD_NOW);

        if (vk_dso) {
            break;
        }

        ALOGV("%s: %s", vklib[i], dlerror());
    }

    if (!vk_dso) {
        ALOGE("No vulkan driver found");
        return;
    }

    hwvulkan_device.EnumerateInstanceExtensionProperties =
        (PFN_vkEnumerateInstanceExtensionProperties) dlsym(vk_dso, "vkEnumerateInstanceExtensionProperties");

    hwvulkan_device.CreateInstance =
        (PFN_vkCreateInstance) dlsym(vk_dso, "vkCreateInstance");

    hwvulkan_device.GetInstanceProcAddr =
        (PFN_vkGetInstanceProcAddr) dlsym(vk_dso, "vkGetInstanceProcAddr");
}

int hwvulkan_device_open(const struct hw_module_t * module,
                const char * name, struct hw_device_t ** device)
{
    if (strcmp(name, HWVULKAN_DEVICE_0)) {
        /* Not a hwvulkan device. */
        return -ENOENT;
    }

    if (!vk_dso) {
        hwvulkan_device_init();
    }

    if (!vk_dso) {
        return -ENOENT;
    }

    *device = &hwvulkan_device.common;
    return 0;
}

int hwvulkan_device_close(struct hw_device_t *dev)
{
    (void) dev;
    if (vk_dso) {
        dlclose(vk_dso);
        vk_dso = NULL;
    }

    return 0;
}

