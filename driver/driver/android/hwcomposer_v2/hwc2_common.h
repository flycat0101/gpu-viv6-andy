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


#ifndef __hwc2_common_h_
#define __hwc2_common_h_

#define HWC2_INCLUDE_STRINGIFICATION
#include <hardware/hwcomposer2.h>

/* Reference. */
#include "hwc2_util.h"
#include "hwc2_blitter.h"


/* Use "__hwc2" prefix for internal hwc2 types/functions. */
typedef struct __hwc2_device    __hwc2_device_t;
typedef struct __hwc2_display   __hwc2_display_t;
typedef struct __hwc2_layer     __hwc2_layer_t;
typedef struct __hwc2_config    __hwc2_config_t;

/* Basic device structure. */
struct __hwc2_device
{
    hwc2_device_t device;

    /* Callback functions. */
    HWC2_PFN_HOTPLUG hotplug;
    hwc2_callback_data_t hotplugData;

    HWC2_PFN_REFRESH refresh;
    hwc2_callback_data_t refreshData;

    HWC2_PFN_VSYNC vsync;
    hwc2_callback_data_t vsyncData;

    __hwc2_list_t displays;
    int32_t numDisplays;

    /* Dump. */
    char *dumpBuffer;       /* string buffer. */
    uint32_t dumpSize;      /* buffer size. */
    uint32_t dumpPos;       /* current pos. */

    /* Nested blitter device. */
    struct blitter_device blt;

    /* Device features >>> */
    /* XXX: Should be set by Soc-Vendor. */
    int32_t capabilities[8];
    uint32_t numCapabilities;

    uint32_t maxVirtualDisplayCount;
    /* <<< End device features. */
};

/* Basic display structure. */
struct __hwc2_display
{
    /* Parent device. */
    __hwc2_device_t *device;

    char name[64];

    int32_t id;             /* 0: for primary, 1 for external, etc. */
    int32_t type;           /* hwc2_display_type_t */

    int32_t connected;      /* boolean, display is ready to output. */

    /* Display features >>> */
    /* XXX: Should be set by Soc-Vendor. */
    /* Doze support. */
    int32_t dozeSupport;    /* 1 for doze support, 0 for no. */

    /* Supported hdr types. */
    int32_t hdrTypes[8];    /* android_hdr_t */
    uint32_t numHdrTypes;
    float maxLuminance;
    float maxAverageLuminance;
    float minLuminance;

    /* Supported configs. */
    __hwc2_config_t *configs[32];
    uint32_t numConfigs;

    /* Supported color modes. */
    int32_t colorModes[8];
    uint32_t numColorModes;
    /* <<< End display features. */

    /* Display attributes. */
    uint32_t width;         /* current resolution, shoudl be same as -- */
    uint32_t height;        /* -- activeConfig for physical display. */
    int32_t format;         /* android_pixel_format_t */
    int32_t dataspace;      /* android_dataspace_t */

    /*
     * Current display validation state.
     * Initially,                       HWC2_ERROR_NOT_VALIDATED,
     * After validate but has changes:  HWC2_ERROR_HAS_CHANGES,
     * After validate and success:      HWC2_ERROR_NONE,
     * After set layer states:          HWC2_ERROR_NOT_VALIDATED
     */
    int32_t validation;     /* hwc2_error_t */

    /* Current display requests. */
    int32_t request;        /* hwc2_display_request_t */

    /* Current client target. */
    buffer_handle_t clientTarget;
    int32_t clientAcquireFence;
    int32_t clientDataspace;
    hwc_region_t clientDamage;

    /* Current output buffer for virtual display. */
    buffer_handle_t outputBuffer;
    int32_t outputReleaseFence;

    /* Current active config. Invalid for virtual display. */
    __hwc2_config_t *activeConfig;

    /* Current color mode. */
    /* XXX: Future use in android framework. */
    int32_t colorMode;      /* android_color_mode_t */

    /* Current color transform. */
    float colorTransform[16];
    int32_t colorTransformHint;

    /* Current power mode. */
    int32_t powerMode;      /* hwc2_power_mode_t */

    /* Current vsync state. */
    int32_t vsyncEnabled;   /* boolean, vsync enabling. */

    /* All layers belongs to the display. */
    __hwc2_list_t layers;

    /* Layers with changed composition type after validateDisplay. */
    __hwc2_list_t layersChanged;

    /* Nested blitter display context. */
    struct blitter_display blt;

    /* link displays in a list. */
    __hwc2_list_t link;
};

/* Basic config structure. */
struct __hwc2_config
{
    hwc2_config_t config;   /* equals to (INDEX + 1). */

    uint32_t width;         /* resolution in pixels. */
    uint32_t height;
    float   fps;            /* In float, for precision. */
    int32_t vsyncPeriod;    /* In nanoseconds. */
    /* Dots per thousand inches (DPI * 1000) . */
    int32_t xdpi;
    int32_t ydpi;
};

typedef enum
{
    COMP_USE_NULL = 0,
    COMP_USE_BLIT = 1
} __hwc2_composer_selection_t;

/* Basic layer structure. */
struct __hwc2_layer
{
    /* Layer index, 0 for bottom layer. */
    uint32_t id;

    /* Following layer content change does not require a validation. */
    /* Set by client. */
    buffer_handle_t buffer;     /* Valid when HWC2_COMPOSITION_DEVICE -- */
    int32_t acquireFence;       /* -- and HWC2_COMPOSITION_CURSOR */
    hwc_region_t surfaceDamage;

    /* Cursor position. */
    int32_t cursorX;
    int32_t cursorY;

    /*
     * Layer states below >>>
     * Layer state change requires subsequent display validation.
     * Set by client.
     */
    int32_t composition;        /* hwc2_composition_t */

    hwc_color_t solidColor;         /* HWC2_COMPOSITION_SOLID_COLOR */
    buffer_handle_t sidebandStream; /* HWC2_COMPOSITION_SIDEBAND */

    int32_t dataspace;          /* android_dataspace_t */

    int32_t blendMode;          /* hwc2_blend_mode_t */
    float planeAlpha;           /* [0.0, 1.0] */

    /* Validate when buffer exists. */
    hwc_frect_t sourceCrop;
    int32_t transform;          /* hwc_transform_t */

    hwc_rect_t displayFrame;
    hwc_region_t visibleRegion;

    uint32_t zorder;
    /* <<< End layer states. */

    /* Layer state change flags. */
    /*
     * sourceCrop, transform, displayFrame, visibleRegion, blendMode
     * changed.
     * NOTICE: blendMode may impact layer relationship.
     */
    uint32_t geometryChanged;
    uint32_t blendChanged;          /* blendMode, planeAlpha changed. */
    uint32_t colorChanged;          /* solidColor,dataspace changed. */
    uint32_t visibleRegionChanged;  /* visibleRegion changed. */

    hwc_region_t visibleRegionPrev; /* Previous visibleRegion. */

    /*
     * Valid compositiono type changes:
     *
     * HWC2_COMPOSITION_CLIENT:
     *     must not change
     *
     * HWC2_COMPOSITION_DEVICE:
     *    can be changed to HWC2_COMPOSITION_CLIENT
     *
     * HWC2_COMPOSITION_SOLID_COLOR:
     *    can be changed to HWC2_COMPOSITION_DEVICE or HWC2_COMPOSITION_CLIENT
     *
     * HWC2_COMPOSITION_CURSOR:
     *    can be changed to HWC2_COMPOSITION_DEVICE or HWC2_COMPOSITION_CLIENT
     *
     * HWC2_COMPOSITION_SIDEBAND:
     *    can be changed to HWC2_COMPOSITION_DEVICE or HWC2_COMPOSITION_CLIENT
     *
     * Set when validateDisplay.
     */
    int32_t compositionChanged; /* hwc2_composition_t */

    /* link layers with changed composition types. */
    __hwc2_list_t linkChanged;

    /*
     * If composerSel is not '0', this layer is reserved as blitter composition.
     * Should not composite this layer by other means in this case.
     * When 'composerSel' enabled, composition type should be one of the following:
     * o HWC2_COMPOSITION_DEVICE,
     * o HWC2_COMPOSITION_SOLID_COLOR
     * o HWC2_COMPOSITION_CURSOR
     *
     * If composerSel is '0', blitter will not touch this layer.
     *
     * Set by __hwc2_validate_blitter when validateDisplay.
     * See HWC2_FUNCTION_VALIDATE_DISPLAY below for more information.
     */
    int32_t composerSel;        /* __hwc2_composer_selection_t */

    /* Layer request, set when validateDisplay. */
    int32_t request;            /* hwc2_layer_request_t */

    /*
     * Layer release fence, valid only when 'buffer' exists.
     * Set when presentDisplay.
     */
    int32_t releaseFence;

    /* Nested blitter layer context. */
    struct blitter_layer blt;

    /* link layers belongs to display in a list. */
    __hwc2_list_t link;
};

/*
 * HWC2 HAL Framework and Helper functions.
 */

/*
 * Config functions.
 *
 * Value of hwc2_config_t equals to (INDEX + 1), where INDEX is index
 * of the __hwc2_config_t in __hwc2_display_t::configs array.
 */
/* (hwc2_config_t) to (__hwc2_config_t *) */
static inline __hwc2_config_t * __hwc2_get_config(__hwc2_display_t *dpy,
                                        hwc2_config_t config)
{
    if (unlikely((uint32_t)config - 1 >= dpy->numConfigs)) {
        return NULL;
    }

    return dpy->configs[(uint32_t)config - 1];
}

/*
 * __hwc2_get_capabilities
 *
 * Default getCapabilities.
 * Returns __hwc2_device_t::capabilities[].
 */
void __hwc2_get_capabilities(struct hwc2_device* device, uint32_t* outCount,
        int32_t* /*hwc2_capability_t*/ outCapabilities);

/*
 * __hwc2_get_function
 *
 * Default getFunction.
 *
 * Returns a default function pointer which implements the requested
 * description.
 *
 * Default functions only support blitter and client composition, does not
 * include any practical display-controller relevant operations.
 *
 * XXX: Soc-Vendor:
 * NOTICE that some function requires taking effect immediately upon
 * function returns. Must override the default function, ie implement practical
 * functionalities for such descriptors.
 *
 * HWC2_FUNCTION_PRESENT_DISPLAY
 * ===============================
 * Presents the current display contents on the screen (or in the case of
 * virtual displays, into the output buffer).
 *
 * Default function does not know how to present physical displays.
 * Please use __hwc2_queue_blit to launch blitter composition for layers with
 * 'composerSel' enabled. __hwc2_blit_virtual_display is a shortcut for virtual
 * display.
 *
 *
 * HWC2_FUNCTION_SET_ACTIVE_CONFIG
 * ===============================
 * Sets the active configuration for this display. Upon returning, the given
 * display configuration should be active and remain so until either this
 * function is called again or the display is disconnected.
 *
 *
 * HWC2_FUNCTION_SET_COLOR_MODE
 * ============================
 * Sets the color mode of the given display.
 *
 * Upon returning from this function, the color mode change must have fully
 * taken effect.
 *
 *
 * HWC2_FUNCTION_SET_POWER_MODE
 * ============================
 * Sets the power mode of the given display. The transition must be complete
 * when this function returns.
 *
 *
 * May also need implement some other function, rather than default:
 *
 * HWC2_FUNCTION_CREATE_VIRTUAL_DISPLAY, HWC2_FUNCTION_DESTROY_VIRTUAL_DISPLAY
 * ====================================  =====================================
 * Creates/destroys virtual display.
 *
 * To support compoisition by other means rather than blitter or CLIENT, please
 * create virtual displays whcih extend __hwc2_display_t.
 *
 *
 * HWC2_FUNCTION_REGISTER_CALLBACK
 * ===============================
 * Provides a callback for the device to call.
 *
 * It is a good point to hotplug physical displays, right after the 'hotplug'
 * callback function registered.
 *
 *
 * HWC2_FUNCTION_CREATE_LAYER, HWC2_FUNCTION_DESTROY_LAYER
 * ==========================  ===========================
 * Creates/destroys a layer on the given display.
 *
 * To support composition by other means rather than blitter or CLIENT, please
 * create layers which extend __hwc2_layer_t.
 *
 *
 * HWC2_FUNCTION_GET_CLIENT_TARGET_SUPPORT
 * =======================================
 * Returns whether a client target with the given properties can be handled.
 *
 * Client tagert support depends on display hardware.
 *
 *
 * HWC2_FUNCTION_SET_VSYNC_ENABLED
 * ===============================
 * Enables or disables the vsync signal for the given display.
 *
 * Default function only sets __hwc2_display_t::vsyncEnabled blindly. There may
 * be some extra works for practical vsync.
 *
 *
 * HWC2_FUNCTION_VALIDATE_DISPLAY
 * ==============================
 * Instructs the device to inspect all of the layer state and determine if
 * there are any composition type changes necessary before presenting the
 * display.
 *
 * To support composition by other means rather than blitter or CLIENT, please
 * override default validateDisplay.
 *
 * To use blitter inside non-default validateDisplay, please call
 * __hwc2_validate_blitter to validate with blitter. Layers can be handled by
 * blitter will have 'composerSel' set upon function return.
 *
 * If do want to use blitter for specific layers, please keep 'composerSel' set
 * to blitter.
 * Blitter composition will be done in __hwc2_queue_blit, which should be called
 * inside (overriden) presentDisplay.
 *
 * Otherwise if don't want to use blitter, please set 'composerSel' to '0'.
 * Do NOT change 'composerSel' outside of validate-display, ie, can not change it
 * on the fly.
 *
 *
 * HWC2_FUNCTION_SET_CURSOR_POSITION
 * =================================
 * Asynchonously sets the position of a cursor layer.
 *
 * Once a cursor layer has been presented, its position may be set by this function
 * at any time between presentDisplay and any subsequent validateDisplay.
 *
 * To support hardware cursor, please override default setCursorPosition.
 */
hwc2_function_pointer_t __hwc2_get_function(struct hwc2_device* device,
                            int32_t /*hwc2_function_descriptor_t*/ descriptor);

/*
 * Initialize basic device.
 * The function will NOT change parent structure __hwc2_device_t::device.
 * NOTICE:
 * Device features are not changed/initialized.
 */
int32_t __hwc2_init_device(__hwc2_device_t *device);

/* Close basic device. */
void __hwc2_close_device(__hwc2_device_t *device);

/*
 * Allocate an initialized basic display
 * Will add to __hwc2_device_t::displays linked list of the device upon return.
 *
 * NOTICE:
 * Do not forget to initialize display features for the returned display.
 *
 * @format: HAL_PIXEL_FORMAT_xxx
 * @type:   HWC2_DISPLAY_TYPE_PHYSICAL or HWC2_DISPLAY_TYPE_VIRTUAL
 * @size:   the structure size, can be larger than sizeof(__hwc2_display_t).
 */
__hwc2_display_t * __hwc2_alloc_display(__hwc2_device_t *device,
                        uint32_t width, uint32_t height, int32_t format,
                        int32_t type, size_t size);

/*
 * Free basic display.
 * Will remove from __hwc2_device_t::displays linked list of the device.
 */
void __hwc2_free_display(__hwc2_device_t *device, __hwc2_display_t *dpy);

/*
 * Allocate an initialized basic layer.
 * Will add to __hwc2_display_t::layers linked list of the display upon return.
 *
 * @size:   the structure size, can be larger than sizeof(__hwc2_layer_t).
 */
__hwc2_layer_t * __hwc2_alloc_layer(__hwc2_device_t *device,
                        __hwc2_display_t *dpy, size_t size);

/*
 * Free basic layer.
 * Will remove from __hwc2_display_t::layers linked list of the display.
 */
void __hwc2_free_layer(__hwc2_device_t *device, __hwc2_display_t *dpy,
            __hwc2_layer_t *layer);

/*
 * Allocate an initialized basic config.
 * Will add to __hwc2_display_t::configs array upon return.
 *
 * @size:   the structure size, can be larger than sizeof(__hwc2_config_t).
 */
__hwc2_config_t * __hwc2_alloc_config(__hwc2_device_t *device,
                        __hwc2_display_t *dpy, size_t size);

/*
 * Free basic config.
 * Will remove from __hwc2_display_t::configs array.
 */
void __hwc2_free_config(__hwc2_device_t *device, __hwc2_display_t *dpy,
            __hwc2_config_t *config);

/*
 * Register callback functions.
 * Record the callbackData and function pointer into device object.
 *
 * Returns hwc2_error.
 */
int32_t __hwc2_register_callback(__hwc2_device_t *device,
                int32_t /*hwc2_callback_descriptor_t*/ descriptor,
                hwc2_callback_data_t callbackData,
                hwc2_function_pointer_t pointer);

/*
 * __hwc2_validate_blitter
 *
 * Validate display with blitter. Layers which can be composited by blitter
 * will set 'composerSel' upon function returns.
 *
 * This function will not change ANY other state of the display and its layers.
 *
 * Returns hwc2_error.
 */
int32_t __hwc2_validate_blitter(__hwc2_device_t *device,
                __hwc2_display_t *dpy);

/*
 * check if specified layer can use blitter.
 * As described in HWC2_FUNCTION_VALIDATE_DISPLAY, only need to check 'composerSel'.
 */
static inline int32_t __hwc2_can_blit_layer(__hwc2_display_t *dpy,
                            __hwc2_layer_t *layer)
{
    return (layer->composerSel == COMP_USE_BLIT);
}

/*
 * Disable blit specified layer.
 */
static inline void __hwc2_disable_blit_layer(__hwc2_display_t *dpy,
                        __hwc2_layer_t *layer)
{
    layer->composerSel = COMP_USE_NULL;
}

/*
 * Please call this function to re-initialize some per frame states,
 * inside later section of presentDisplay.
 * XXX: Important.
 */
void __hwc2_frame_end(__hwc2_device_t *device, __hwc2_display_t *dpy);

/*
 * Composition display with blitter.
 * Layers with __hwc2_layer_t::composerSel set to blitter are composited.
 *
 * output buffer and releaseFence:
 * For physical display, please allocate new buffer for display, and its
 * release fence.
 *
 * TODO: Not done yet.
 * The function is designed as return immediately.
 *
 * For virtual display, output buffer should be __hwc2_display_t::outputBuffer
 * and releaseFence should be __hwc2_display_t::outputReleaseFence.
 *
 * NOTICE:
 * Please call __hwc2_blit_[virtual_]display even no layers to be composited by
 * blitter, blitter need reference previous buffer for optimizations.
 *
 * Return value:
 * 0 on success.
 * negative values for error.
 */
int32_t __hwc2_queue_blit(__hwc2_device_t *device, __hwc2_display_t *dpy,
            buffer_handle_t buffer, int32_t releaseFence,
            int32_t *outAcquireFence);

static inline int32_t __hwc2_queue_blit_virtual(__hwc2_device_t *device,
                        __hwc2_display_t *dpy, int32_t *outRetireFence)
{
    return __hwc2_queue_blit(device, dpy, dpy->outputBuffer,
                dpy->outputReleaseFence, outRetireFence);
}

typedef int32_t (*__HWC2_PFN_POST_DISPLAY)(
        __hwc2_device_t* device, __hwc2_display_t *dpy,
        void *param);

/*
 * Queue post display.
 * Display is presented after acquireFence is signaled.
 */
int32_t __hwc2_queue_post_display(__hwc2_device_t *device,
            __hwc2_display_t *dpy, int32_t acquireFence,
            __HWC2_PFN_POST_DISPLAY postDisplay, void *param);

#endif /* __hwc2_common_h_ */

