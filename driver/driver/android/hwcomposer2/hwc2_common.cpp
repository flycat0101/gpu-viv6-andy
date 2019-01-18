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


/*
 * hwc2_common.cpp
 *
 * Defines common hwc2 HAL functions, includes basic hwc2 features and blitter
 * feature.
 */
#include <log/log.h>

#include "hwc2_common.h"

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <gralloc_priv.h>

#include <sync/sync.h>

#include <cutils/properties.h>

/*
 * Device Functions
 *
 * All of these functions take as their first parameter a device pointer, so
 * this parameter is omitted from the described parameter lists.
 */

/* createVirtualDisplay(..., width, height, format, outDisplay)
 * Descriptor: HWC2_FUNCTION_CREATE_VIRTUAL_DISPLAY
 * Must be provided by all HWC2 devices
 *
 * Creates a new virtual display with the given width and height. The format
 * passed into this function is the default format requested by the consumer of
 * the virtual display output buffers. If a different format will be returned by
 * the device, it should be returned in this parameter so it can be set properly
 * when handing the buffers to the consumer.
 *
 * The display will be assumed to be on from the time the first frame is
 * presented until the display is destroyed.
 *
 * Parameters:
 *   width - width in pixels
 *   height - height in pixels
 *   format - prior to the call, the default output buffer format selected by
 *       the consumer; after the call, the format the device will produce
 *   outDisplay - the newly-created virtual display; pointer will be non-NULL
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_UNSUPPORTED - the width or height is too large for the device to
 *       be able to create a virtual display
 *   HWC2_ERROR_NO_RESOURCES - the device is unable to create a new virtual
 *       display at this time
 */
/*
 * Default createVirtualDisplay.
 * Create virtual display with HAL_PIXEL_FORMAT_RGBA_8888.
 * Default virtual display uses blitter when available.
 */
static int32_t /*hwc2_error_t*/ create_virtual_display(
        hwc2_device_t* device, uint32_t width, uint32_t height,
        int32_t* /*android_pixel_format_t*/ format, hwc2_display_t* outDisplay)
{
    __hwc2_device_t *dev = (__hwc2_device_t *)device;
    __hwc2_display_t *dpy;

    __hwc2_trace(0, "device=%p width=%u height=%u format=%p",
            device, width, height, format);

    dpy = __hwc2_alloc_display(dev, width, height, HAL_PIXEL_FORMAT_RGBA_8888,
            HWC2_DISPLAY_TYPE_VIRTUAL, sizeof(__hwc2_display_t));

    if (unlikely(!dpy)) {
        __hwc2_trace_error(1, HWC2_ERROR_NO_RESOURCES);
        return HWC2_ERROR_NO_RESOURCES;
    }

    snprintf(dpy->name, sizeof(dpy->name),
            "virtual-display-%ux%u", width, height);

    /* Virtual display is always connected and powered on. */
    dpy->connected  = 1;
    dpy->powerMode = HWC2_POWER_MODE_ON;

    if (format) {
        *format = HAL_PIXEL_FORMAT_RGBA_8888;
    }

    *outDisplay = (hwc2_display_t)(uintptr_t)dpy;

    __hwc2_trace(1, "out dpy=%p", dpy);
    return HWC2_ERROR_NONE;
}

/* destroyVirtualDisplay(..., display)
 * Descriptor: HWC2_FUNCTION_DESTROY_VIRTUAL_DISPLAY
 * Must be provided by all HWC2 devices
 *
 * Destroys a virtual display. After this call all resources consumed by this
 * display may be freed by the device and any operations performed on this
 * display should fail.
 *
 * Parameters:
 *   display - the virtual display to destroy
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_DISPLAY - an invalid display handle was passed in
 *   HWC2_ERROR_BAD_PARAMETER - the display handle which was passed in does not
 *       refer to a virtual display
 */
static int32_t /*hwc2_error_t*/ destroy_virtual_display(
        hwc2_device_t* device, hwc2_display_t display)
{
    __hwc2_device_t *dev = (__hwc2_device_t *)device;
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;

    sanity_check_display(device, dpy);

    __hwc2_trace(0, "dpy=%p", dpy);

    if (dpy->type != HWC2_DISPLAY_TYPE_VIRTUAL) {
        __hwc2_trace_error(1, HWC2_ERROR_BAD_PARAMETER);
        return HWC2_ERROR_BAD_PARAMETER;
    }

    __hwc2_free_display(dev, dpy);

    __hwc2_trace(1, "OK");
    return HWC2_ERROR_NONE;
}

static inline void store_dump(__hwc2_device_t *dev, const char *format, ...)
                        __attribute__((format(printf,2,3)));

void store_dump(__hwc2_device_t *dev, const char *format, ...)
{
    va_list ap;

    if (dev->dumpSize - dev->dumpPos < 256) {
        dev->dumpSize += 512;
        dev->dumpBuffer = (char *)realloc(dev->dumpBuffer, dev->dumpSize);
    }

    va_start(ap, format);
    dev->dumpPos += vsnprintf(&dev->dumpBuffer[dev->dumpPos], 256, format, ap);
    va_end(ap);
}

static inline void dump_layer(__hwc2_device_t *dev,
                        __hwc2_display_t *dpy, __hwc2_layer_t *layer)
{
    const hwc_frect_t *fr;
    const hwc_rect_t *r;
    hwc_color_t c;
    hwc_region_t *reg;

    /* general information. */
    store_dump(dev, "+ Layer=%p zorder=%u composition=%s(%d) composerSel=%d\n",
        layer, layer->zorder, composition_name(layer->composition),
        layer->composition, layer->composerSel);

    /* source. */
    if (layer->buffer) {
        store_dump(dev, "    buffer=%p transform=%s(%d) dataspace=%u\n",
            layer->buffer, transform_name(layer->transform),
            layer->transform, layer->dataspace);

    } else if (layer->sidebandStream) {
        store_dump(dev, "    sidebandStream=%p\n", layer->sidebandStream);
    } else {
        c = layer->solidColor;
        store_dump(dev, "    color=(%d,%d,%d,%d)\n", c.r, c.g, c.b, c.a);
    }

    /* source damage. */
    if (layer->buffer || layer->sidebandStream) {
        reg = &layer->surfaceDamage;
        if (reg->numRects == 0) {
            store_dump(dev, "    damage=(whole-surface)\n");
        } else {
            store_dump(dev, "    damage=\n");
        }

        for (uint32_t i = 0; i < reg->numRects; i++) {
            r = &reg->rects[i];
            store_dump(dev, "     [%d]: [%d,%d,%d,%d]\n",
                i, r->left, r->top, r->right, r->bottom);
        }
    }

    /* alpha blending. */
    store_dump(dev, "    blend=%s(%d) planeAlpha=%.2f\n",
        blend_mode_name(layer->blendMode), layer->blendMode,
        layer->planeAlpha);

    if (layer->buffer || layer->sidebandStream) {
        fr = &layer->sourceCrop;

        store_dump(dev, "    sourceCrop=[%.1f,%.1f,%.1f,%.1f]\n",
            fr->left, fr->top, fr->right, fr->bottom);
    }

    /* target rectangle. */
    r = &layer->displayFrame;
    store_dump(dev, "    displayFrame=[%d,%d,%d,%d]\n",
        r->left, r->top, r->right, r->bottom);

    /* visible region. */
    reg = &layer->visibleRegion;

    if (reg->numRects == 0) {
        store_dump(dev, "    visibleRegion=(empty)\n");
    } else {
        store_dump(dev, "    visibleRegion=\n");
    }

    for (uint32_t i = 0; i < reg->numRects; i++) {
        r = &reg->rects[i];
        store_dump(dev, "     [%d]: [%d,%d,%d,%d]\n",
            i, r->left, r->top, r->right, r->bottom);
    }
}

static inline void dump_display(__hwc2_device_t *dev, __hwc2_display_t *dpy)
{
    __hwc2_list_t *pos;
    __hwc2_layer_t *layer;

    store_dump(dev, "Display=%p type=%s(%d) size=%ux%u format=%u\n",
        dpy, display_type_name(dpy->type), dpy->type,
        dpy->width, dpy->height, dpy->format);

    store_dump(dev, "clientTarget=%p acquireFence=%i dataspace=%u\n",
        dpy->clientTarget, dpy->clientAcquireFence, dpy->clientDataspace);

    store_dump(dev, "outputBuffer=%p releaseFence=%i\n",
        dpy->outputBuffer, dpy->outputReleaseFence);

    __hwc2_list_for_each(pos, &dpy->layers) {
        layer = __hwc2_list_entry(pos, __hwc2_layer_t, link);
        dump_layer(dev, dpy, layer);
    }
}

/* dump(..., outSize, outBuffer)
 * Descriptor: HWC2_FUNCTION_DUMP
 * Must be provided by all HWC2 devices
 *
 * Retrieves implementation-defined debug information, which will be displayed
 * during, for example, `dumpsys SurfaceFlinger`.
 *
 * If called with outBuffer == NULL, the device should store a copy of the
 * desired output and return its length in bytes in outSize. If the device
 * already has a stored copy, that copy should be purged and replaced with a
 * fresh copy.
 *
 * If called with outBuffer != NULL, the device should copy its stored version
 * of the output into outBuffer and store how many bytes of data it copied into
 * outSize. Prior to this call, the client will have populated outSize with the
 * maximum number of bytes outBuffer can hold. The device must not write more
 * than this amount into outBuffer. If the device does not currently have a
 * stored copy, then it should return 0 in outSize.
 *
 * Any data written into outBuffer need not be null-terminated.
 *
 * Parameters:
 *   outSize - if outBuffer was NULL, the number of bytes needed to copy the
 *       device's stored output; if outBuffer was not NULL, the number of bytes
 *       written into it, which must not exceed the value stored in outSize
 *       prior to the call; pointer will be non-NULL
 *   outBuffer - the buffer to write the dump output into; may be NULL as
 *       described above; data written into this buffer need not be
 *       null-terminated
 */
static void dump(hwc2_device_t* device, uint32_t* outSize,
        char* outBuffer)
{
    __hwc2_device_t *dev = (__hwc2_device_t *)device;
    __hwc2_display_t *dpy;

    if (!outBuffer) {
        __hwc2_list_t *pos;

        /* Reset pos. */
        dev->dumpPos = 0;

        __hwc2_list_for_each(pos, &dev->displays) {
            dpy = __hwc2_list_entry(pos, __hwc2_display_t, link);
            dump_display(dev, dpy);
        }

        *outSize = dev->dumpPos;
        __hwc2_trace(2, "out outSize=%u", *outSize);
    } else {
        if (*outSize > dev->dumpPos)
            *outSize = dev->dumpPos;

        memcpy(outBuffer, dev->dumpBuffer, *outSize);
        __hwc2_trace(2, "out outSize=%u", *outSize);
    }
}

/* getMaxVirtualDisplayCount(...)
 * Descriptor: HWC2_FUNCTION_GET_MAX_VIRTUAL_DISPLAY_COUNT
 * Must be provided by all HWC2 devices
 *
 * Returns the maximum number of virtual displays supported by this device
 * (which may be 0). The client will not attempt to create more than this many
 * virtual displays on this device. This number must not change for the lifetime
 * of the device.
 */
static uint32_t get_max_virtual_display_count(
        hwc2_device_t* device)
{
    __hwc2_device_t *dev = (__hwc2_device_t *)device;

    __hwc2_trace(2, "return=%d", dev->maxVirtualDisplayCount);
    return dev->maxVirtualDisplayCount;
}

/* registerCallback(..., descriptor, callbackData, pointer)
 * Descriptor: HWC2_FUNCTION_REGISTER_CALLBACK
 * Must be provided by all HWC2 devices
 *
 * Provides a callback for the device to call. All callbacks take a callbackData
 * item as the first parameter, so this value should be stored with the callback
 * for later use. The callbackData may differ from one callback to another. If
 * this function is called multiple times with the same descriptor, later
 * callbacks replace earlier ones.
 *
 * Parameters:
 *   descriptor - which callback should be set
 *   callBackdata - opaque data which must be passed back through the callback
 *   pointer - a non-NULL function pointer corresponding to the descriptor
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_PARAMETER - descriptor was invalid
 */
int32_t /*hwc2_error_t*/ register_callback(
        hwc2_device_t* device,
        int32_t /*hwc2_callback_descriptor_t*/ descriptor,
        hwc2_callback_data_t callbackData, hwc2_function_pointer_t pointer)
{
    __hwc2_device_t *dev = (__hwc2_device_t *)device;
    int32_t err;

    __hwc2_trace(0, "device=%p descriptor=%s(%d) callbackData=%p pointer=%p",
            device, callback_descriptor_name(descriptor), descriptor,
            callbackData, pointer);

    err = __hwc2_register_callback(dev, descriptor, callbackData, pointer);

    __hwc2_trace(1, "return=%s(%d)", error_name(err), err);
    return err;
}

/*
 * Display Functions
 *
 * All of these functions take as their first two parameters a device pointer
 * and a display handle, so these parameters are omitted from the described
 * parameter lists.
 */

/* acceptDisplayChanges(...)
 * Descriptor: HWC2_FUNCTION_ACCEPT_DISPLAY_CHANGES
 * Must be provided by all HWC2 devices
 *
 * Accepts the changes required by the device from the previous validateDisplay
 * call (which may be queried using getChangedCompositionTypes) and revalidates
 * the display. This function is equivalent to requesting the changed types from
 * getChangedCompositionTypes, setting those types on the corresponding layers,
 * and then calling validateDisplay again.
 *
 * After this call it must be valid to present this display. Calling this after
 * validateDisplay returns 0 changes must succeed with HWC2_ERROR_NONE, but
 * should have no other effect.
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_DISPLAY - an invalid display handle was passed in
 *   HWC2_ERROR_NOT_VALIDATED - validateDisplay has not been called
 */
static int32_t /*hwc2_error_t*/ accept_display_changes(
        hwc2_device_t* device, hwc2_display_t display)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;
    __hwc2_list_t *pos;

    sanity_check_display(device, dpy);

    __hwc2_trace(0, "dpy=%p", dpy);

    __hwc2_list_for_each(pos, &dpy->layersChanged) {
        __hwc2_layer_t *layer;
        layer = __hwc2_list_entry(pos, __hwc2_layer_t, linkChanged);

        /* Apply composition type changes. */
        layer->composition = layer->compositionChanged;

        /* Directly close acquireFence for CLIENT composited layers. */
        int32_t fd = layer->acquireFence;
        if (layer->composition == HWC2_COMPOSITION_CLIENT && fd != -1) {
            close(fd);
            layer->acquireFence = -1;
        }
    }

#ifdef __HWC2_TRACE
    __hwc2_list_for_each(pos, &dpy->layers) {
        __hwc2_layer_t *layer = __hwc2_list_entry(pos, __hwc2_layer_t, link);

        __hwc2_trace(2, "layer=%p(%u) composition=%s(%d)", layer, layer->id,
                composition_name(layer->composition), layer->composition);
    }
#endif

    /* Reinit layersChanged list. */
    __hwc2_list_init(&dpy->layersChanged);
    dpy->validation = HWC2_ERROR_NONE;

    __hwc2_trace(1, "OK");
    return 0;
}

/* createLayer(..., outLayer)
 * Descriptor: HWC2_FUNCTION_CREATE_LAYER
 * Must be provided by all HWC2 devices
 *
 * Creates a new layer on the given display.
 *
 * Parameters:
 *   outLayer - the handle of the new layer; pointer will be non-NULL
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_DISPLAY - an invalid display handle was passed in
 *   HWC2_ERROR_NO_RESOURCES - the device was unable to create this layer
 */
static int32_t /*hwc2_error_t*/ create_layer(hwc2_device_t* device,
        hwc2_display_t display, hwc2_layer_t* outLayer)
{
    __hwc2_device_t *dev = (__hwc2_device_t *)device;
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;
    __hwc2_layer_t *layer;

    sanity_check_display(device, dpy);

    __hwc2_trace(0, "dpy=%p", dpy);

    layer = __hwc2_alloc_layer(dev, dpy, sizeof(__hwc2_layer_t));
    if (unlikely(!layer)) {
        __hwc2_trace_error(1, HWC2_ERROR_NO_RESOURCES);
        return HWC2_ERROR_NO_RESOURCES;
    }

    /*
     * XXX:
     * Client should at least call validateDisplay after new layer created,
     * but for the sake of security, explicitly request refresh here.
     */
    if (dev->refresh) {
        dev->refresh(dev->refreshData, display);
    }

    *outLayer = (hwc2_layer_t)(uintptr_t)layer;

    __hwc2_trace(1, "out outLayer=%p(%u)", layer, layer->id);
    return HWC2_ERROR_NONE;
}

/* destroyLayer(..., layer)
 * Descriptor: HWC2_FUNCTION_DESTROY_LAYER
 * Must be provided by all HWC2 devices
 *
 * Destroys the given layer.
 *
 * Parameters:
 *   layer - the handle of the layer to destroy
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_DISPLAY - an invalid display handle was passed in
 *   HWC2_ERROR_BAD_LAYER - an invalid layer handle was passed in
 */
static int32_t /*hwc2_error_t*/ destroy_layer(
        hwc2_device_t* device, hwc2_display_t display, hwc2_layer_t layer)
{
    __hwc2_device_t *dev = (__hwc2_device_t *)device;
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;
    __hwc2_layer_t *ly = (__hwc2_layer_t *)(uintptr_t)layer;

    sanity_check_display(device, dpy);
    sanity_check_layer(dpy, ly);

    __hwc2_trace(0, "dpy=%p layer=%p(%u)", dpy, ly, ly->id);
    __hwc2_free_layer(dev, dpy, ly);

    /*
     * XXX:
     * Client should at least call validateDisplay after layer removed,
     * but for the sake of security, explicitly request refresh here.
     */
    if (dev->refresh) {
        dev->refresh(dev->refreshData, display);
    }

    __hwc2_trace(1, "OK");
    return HWC2_ERROR_NONE;
}

/* getActiveConfig(..., outConfig)
 * Descriptor: HWC2_FUNCTION_GET_ACTIVE_CONFIG
 * Must be provided by all HWC2 devices
 *
 * Retrieves which display configuration is currently active.
 *
 * If no display configuration is currently active, this function must return
 * HWC2_ERROR_BAD_CONFIG and place no configuration handle in outConfig. It is
 * the responsibility of the client to call setActiveConfig with a valid
 * configuration before attempting to present anything on the display.
 *
 * Parameters:
 *   outConfig - the currently active display configuration; pointer will be
 *       non-NULL
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_DISPLAY - an invalid display handle was passed in
 *   HWC2_ERROR_BAD_CONFIG - no configuration is currently active
 */
/*
 * Default getActiveConfig.
 * Returns index of __hwc2_display_t::activeConfig.
 */
static int32_t /*hwc2_error_t*/ get_active_config(
        hwc2_device_t* device, hwc2_display_t display,
        hwc2_config_t* outConfig)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;
    __hwc2_trace(0, "dpy=%p", dpy);

    sanity_check_display(device, dpy);

    if (!dpy->activeConfig) {
        __hwc2_trace_error(2, HWC2_ERROR_BAD_CONFIG);
        return HWC2_ERROR_BAD_CONFIG;
    }

    sanity_check_config(dpy, dpy->activeConfig);

    *outConfig = dpy->activeConfig->config;
    __hwc2_trace(1, "out outConfig=%u", (uint32_t)*outConfig);
    return HWC2_ERROR_NONE;
}

/* getChangedCompositionTypes(..., outNumElements, outLayers, outTypes)
 * Descriptor: HWC2_FUNCTION_GET_CHANGED_COMPOSITION_TYPES
 * Must be provided by all HWC2 devices
 *
 * Retrieves the layers for which the device requires a different composition
 * type than had been set prior to the last call to validateDisplay. The client
 * will either update its state with these types and call acceptDisplayChanges,
 * or will set new types and attempt to validate the display again.
 *
 * outLayers and outTypes may be NULL to retrieve the number of elements which
 * will be returned. The number of elements returned must be the same as the
 * value returned in outNumTypes from the last call to validateDisplay.
 *
 * Parameters:
 *   outNumElements - if outLayers or outTypes were NULL, the number of layers
 *       and types which would have been returned; if both were non-NULL, the
 *       number of elements returned in outLayers and outTypes, which must not
 *       exceed the value stored in outNumElements prior to the call; pointer
 *       will be non-NULL
 *   outLayers - an array of layer handles
 *   outTypes - an array of composition types, each corresponding to an element
 *       of outLayers
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_DISPLAY - an invalid display handle was passed in
 *   HWC2_ERROR_NOT_VALIDATED - validateDisplay has not been called for this
 *       display
 */
/*
 * Default getChangedCompositionTypes.
 * Returns layers in __hwc2_display_t::layersChanged.
 */
static int32_t /*hwc2_error_t*/ get_changed_composition_types(
        hwc2_device_t* device, hwc2_display_t display,
        uint32_t* outNumElements, hwc2_layer_t* outLayers,
        int32_t* /*hwc2_composition_t*/ outTypes)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;
    __hwc2_list_t *pos;
    uint32_t num = 0;

    sanity_check_display(device, dpy);

    __hwc2_trace(0, "dpy=%p outLayers=%p outTypes=%p",
            dpy, outLayers, outTypes);

    if (dpy->validation == HWC2_ERROR_NOT_VALIDATED) {
        __hwc2_trace_error(1, HWC2_ERROR_NOT_VALIDATED);
        return HWC2_ERROR_NOT_VALIDATED;
    }

    if (!outLayers || !outTypes) {
        __hwc2_list_for_each(pos, &dpy->layersChanged) {
            num++;
        }
    } else {
        __hwc2_list_for_each(pos, &dpy->layersChanged) {
            __hwc2_layer_t *layer;
            layer = __hwc2_list_entry(pos, __hwc2_layer_t, linkChanged);

            if (num == *outNumElements)
                break;

            __hwc2_trace(2, "out outLayers[%d]=%p outTypes[%d]=%s(%d)",
                    num, layer, num,
                    composition_name(layer->compositionChanged),
                    layer->compositionChanged);

            outLayers[num] = (hwc2_layer_t)(uintptr_t)layer;
            outTypes[num]  = layer->compositionChanged;
            num++;
        }
    }

    *outNumElements = num;
    __hwc2_trace(1, "out outNumElements=%u", num);
    return HWC2_ERROR_NONE;
}

/* getClientTargetSupport(..., width, height, format, dataspace)
 * Descriptor: HWC2_FUNCTION_GET_CLIENT_TARGET_SUPPORT
 * Must be provided by all HWC2 devices
 *
 * Returns whether a client target with the given properties can be handled by
 * the device.
 *
 * The valid formats can be found in android_pixel_format_t in
 * <system/graphics.h>.
 *
 * For more about dataspaces, see setLayerDataspace.
 *
 * This function must return true for a client target with width and height
 * equal to the active display configuration dimensions,
 * HAL_PIXEL_FORMAT_RGBA_8888, and HAL_DATASPACE_UNKNOWN. It is not required to
 * return true for any other configuration.
 *
 * Parameters:
 *   width - client target width in pixels
 *   height - client target height in pixels
 *   format - client target format
 *   dataspace - client target dataspace, as described in setLayerDataspace
 *
 * Returns HWC2_ERROR_NONE if the given configuration is supported or one of the
 * following errors:
 *   HWC2_ERROR_BAD_DISPLAY - an invalid display handle was passed in
 *   HWC2_ERROR_UNSUPPORTED - the given configuration is not supported
 */
/*
 * Default getClientTargetSupport.
 * Supports only target with the same attributes.
 */
static int32_t /*hwc2_error_t*/ get_client_target_support(
        hwc2_device_t* device, hwc2_display_t display, uint32_t width,
        uint32_t height, int32_t /*android_pixel_format_t*/ format,
        int32_t /*android_dataspace_t*/ dataspace)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;

    sanity_check_display(device, dpy);

    __hwc2_trace(0, "dpy=%p width=%u height=%u format=%d dataspace=%d",
            dpy, width, height, format, dataspace);

    if (width == dpy->width && height == dpy->height &&
        format == dpy->format && dataspace == dpy->dataspace) {
        __hwc2_trace(1, "OK");
        return HWC2_ERROR_NONE;
    }

    __hwc2_trace_error(1, HWC2_ERROR_UNSUPPORTED);
    return HWC2_ERROR_UNSUPPORTED;
}

/* getColorModes(..., outNumModes, outModes)
 * Descriptor: HWC2_FUNCTION_GET_COLOR_MODES
 * Must be provided by all HWC2 devices
 *
 * Returns the color modes supported on this display.
 *
 * The valid color modes can be found in android_color_mode_t in
 * <system/graphics.h>. All HWC2 devices must support at least
 * HAL_COLOR_MODE_NATIVE.
 *
 * outNumModes may be NULL to retrieve the number of modes which will be
 * returned.
 *
 * Parameters:
 *   outNumModes - if outModes was NULL, the number of modes which would have
 *       been returned; if outModes was not NULL, the number of modes returned,
 *       which must not exceed the value stored in outNumModes prior to the
 *       call; pointer will be non-NULL
 *   outModes - an array of color modes
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_DISPLAY - an invalid display handle was passed in
 */
/*
 * Default getColorModes.
 * Returns __hwc2_display_t::colorModes
 */
static int32_t /*hwc2_error_t*/ get_color_modes(
        hwc2_device_t* device, hwc2_display_t display, uint32_t* outNumModes,
        int32_t* /*android_color_mode_t*/ outModes)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;
    __hwc2_trace(0, "dpy=%p", dpy);

    sanity_check_display(device, dpy);

    /* TODO: Android framework itself does not support it currently. */
    *outNumModes = 0;

    __hwc2_trace(1, "out outNumModes=%d", *outNumModes);
    return HWC2_ERROR_NONE;
}

/* getDisplayAttribute(..., config, attribute, outValue)
 * Descriptor: HWC2_FUNCTION_GET_DISPLAY_ATTRIBUTE
 * Must be provided by all HWC2 devices
 *
 * Returns a display attribute value for a particular display configuration.
 *
 * Any attribute which is not supported or for which the value is unknown by the
 * device must return a value of -1.
 *
 * Parameters:
 *   config - the display configuration for which to return attribute values
 *   attribute - the attribute to query
 *   outValue - the value of the attribute; the pointer will be non-NULL
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_DISPLAY - an invalid display handle was passed in
 *   HWC2_ERROR_BAD_CONFIG - config does not name a valid configuration for this
 *       display
 */
/*
 * Default getDisplayAttributes.
 * Returns attribute of specified __hwc2_config_t.
 */
static int32_t /*hwc2_error_t*/ get_display_attribute(
        hwc2_device_t* device, hwc2_display_t display, hwc2_config_t config,
        int32_t /*hwc2_attribute_t*/ attribute, int32_t* outValue)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;
    __hwc2_config_t *cfg = NULL;

    sanity_check_display(device, dpy);

    __hwc2_trace(0, "dpy=%p attribute=%s(%d)", dpy,
            attribute_name(attribute), attribute);

    cfg = __hwc2_get_config(dpy, config);
    if (unlikely(!cfg)) {
        __hwc2_trace_error(1, HWC2_ERROR_BAD_CONFIG);
        return HWC2_ERROR_BAD_CONFIG;
    }

    sanity_check_config(dpy, cfg);

    switch (attribute) {
        case HWC2_ATTRIBUTE_WIDTH:
            *outValue = (int32_t)cfg->width;
            break;
        case HWC2_ATTRIBUTE_HEIGHT:
            *outValue = (int32_t)cfg->height;
            break;
        case HWC2_ATTRIBUTE_VSYNC_PERIOD:
            *outValue = cfg->vsyncPeriod;
            break;
        case HWC2_ATTRIBUTE_DPI_X:
            *outValue = cfg->xdpi;
            break;
        case HWC2_ATTRIBUTE_DPI_Y:
            *outValue = cfg->ydpi;
            break;
        default:
            /* XXX: Follows HWC2On1Adapter, not an error. */
            *outValue = -1;
    }

    __hwc2_trace(1, "out outValue=%d", *outValue);
    return HWC2_ERROR_NONE;
}

/* getDisplayConfigs(..., outNumConfigs, outConfigs)
 * Descriptor: HWC2_FUNCTION_GET_DISPLAY_CONFIGS
 * Must be provided by all HWC2 devices
 *
 * Returns handles for all of the valid display configurations on this display.
 *
 * outConfigs may be NULL to retrieve the number of elements which will be
 * returned.
 *
 * Parameters:
 *   outNumConfigs - if outConfigs was NULL, the number of configurations which
 *       would have been returned; if outConfigs was not NULL, the number of
 *       configurations returned, which must not exceed the value stored in
 *       outNumConfigs prior to the call; pointer will be non-NULL
 *   outConfigs - an array of configuration handles
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_DISPLAY - an invalid display handle was passed in
 */
/*
 * Default getDisplayConfigs.
 * Returns indices of __hwc2_display_t::configs.
 */
static int32_t /*hwc2_error_t*/ get_display_configs(
        hwc2_device_t* device, hwc2_display_t display, uint32_t* outNumConfigs,
        hwc2_config_t* outConfigs)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;

    sanity_check_display(device, dpy);

    __hwc2_trace(0, "dpy=%p outConfigs=%p", dpy, outConfigs);

    if (!outConfigs) {
        *outNumConfigs = dpy->numConfigs;
    } else {
        if (*outNumConfigs > dpy->numConfigs) {
            *outNumConfigs = dpy->numConfigs;
        }

        for (uint32_t i = 0; i < *outNumConfigs; i++) {
            outConfigs[i] = dpy->configs[i]->config;
            __hwc2_trace(2, "outConfigs[%d]=%u", i, (uint32_t)outConfigs[i]);
        }
    }

    __hwc2_trace(1, "out outNumConfigs=%u", *outNumConfigs);
    return HWC2_ERROR_NONE;
}

/* getDisplayName(..., outSize, outName)
 * Descriptor: HWC2_FUNCTION_GET_DISPLAY_NAME
 * Must be provided by all HWC2 devices
 *
 * Returns a human-readable version of the display's name.
 *
 * outName may be NULL to retrieve the length of the name.
 *
 * Parameters:
 *   outSize - if outName was NULL, the number of bytes needed to return the
 *       name if outName was not NULL, the number of bytes written into it,
 *       which must not exceed the value stored in outSize prior to the call;
 *       pointer will be non-NULL
 *   outName - the display's name
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_DISPLAY - an invalid display handle was passed in
 */
/*
 * Default getDisplayName.
 * Returns __hwc2_display_t::name.
 */
static int32_t /*hwc2_error_t*/ get_display_name(
        hwc2_device_t* device, hwc2_display_t display, uint32_t* outSize,
        char* outName)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;

    sanity_check_display(device, dpy);

    __hwc2_trace(0, "dpy=%p outName=%p", dpy, outName);

    if (!outName) {
        *outSize = strlen(dpy->name);
    } else {
        size_t len = strlen(dpy->name);
        len = len > *outSize ?: *outSize;

        strncpy(outName, dpy->name, len);
        outName[len-1] = '\0';
        __hwc2_trace(2, "out outname=%s", outName);
    }

    __hwc2_trace(1, "out outSize=%u", *outSize);
    return HWC2_ERROR_NONE;
}

/* getDisplayRequests(..., outDisplayRequests, outNumElements, outLayers,
 *     outLayerRequests)
 * Descriptor: HWC2_FUNCTION_GET_DISPLAY_REQUESTS
 * Must be provided by all HWC2 devices
 *
 * Returns the display requests and the layer requests required for the last
 * validated configuration.
 *
 * Display requests provide information about how the client should handle the
 * client target. Layer requests provide information about how the client
 * should handle an individual layer.
 *
 * If outLayers or outLayerRequests is NULL, the required number of layers and
 * requests must be returned in outNumElements, but this number may also be
 * obtained from validateDisplay as outNumRequests (outNumElements must be equal
 * to the value returned in outNumRequests from the last call to
 * validateDisplay).
 *
 * Parameters:
 *   outDisplayRequests - the display requests for the current validated state
 *   outNumElements - if outLayers or outLayerRequests were NULL, the number of
 *       elements which would have been returned, which must be equal to the
 *       value returned in outNumRequests from the last validateDisplay call on
 *       this display; if both were not NULL, the number of elements in
 *       outLayers and outLayerRequests, which must not exceed the value stored
 *       in outNumElements prior to the call; pointer will be non-NULL
 *   outLayers - an array of layers which all have at least one request
 *   outLayerRequests - the requests corresponding to each element of outLayers
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_DISPLAY - an invalid display handle was passed in
 *   HWC2_ERROR_NOT_VALIDATED - validateDisplay has not been called for this
 *       display
 */
/*
 * Default getDisplayRequests.
 * Returns __hwc2_display_t::request and __hwc2_layer_t::request.
 */
static int32_t /*hwc2_error_t*/ get_display_requests(
        hwc2_device_t* device, hwc2_display_t display,
        int32_t* /*hwc2_display_request_t*/ outDisplayRequests,
        uint32_t* outNumElements, hwc2_layer_t* outLayers,
        int32_t* /*hwc2_layer_request_t*/ outLayerRequests)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;
    __hwc2_list_t *pos;
    uint32_t num  = 0;

    sanity_check_display(device, dpy);
    __hwc2_trace(0, "dpy=%p outLayers=%p outLayerRequests=%p",
            dpy, outLayers, outLayerRequests);

    if (!outLayers || !outLayerRequests) {
        __hwc2_list_for_each(pos, &dpy->layers) {
            __hwc2_layer_t *layer = __hwc2_list_entry(pos, __hwc2_layer_t, link);

            if (!layer->request)
                continue;

            num++;
        }
    } else {
        __hwc2_list_for_each(pos, &dpy->layers) {
            __hwc2_layer_t *layer = __hwc2_list_entry(pos, __hwc2_layer_t, link);

            if (num == *outNumElements)
                break;

            if (!layer->request)
                continue;

            __hwc2_trace(2, "out outLayers[%d]=%p outLayerRequests[%d]=%s(%x)",
                    num, layer, num,
                    layer_request_name(layer->request), layer->request);

            outLayers[num] = (hwc2_layer_t)(uintptr_t)layer;
            outLayerRequests[num] = layer->request;
            num++;
        }

    }

    *outDisplayRequests = dpy->request;
    *outNumElements = num;

    __hwc2_trace(1, "out outDisplayRequests=%s(%i) outNumElements=%u",
            display_request_name(*outDisplayRequests), *outDisplayRequests, num);
    return HWC2_ERROR_NONE;
}


/* getDisplayType(..., outType)
 * Descriptor: HWC2_FUNCTION_GET_DISPLAY_TYPE
 * Must be provided by all HWC2 devices
 *
 * Returns whether the given display is a physical or virtual display.
 *
 * Parameters:
 *   outType - the type of the display; pointer will be non-NULL
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_DISPLAY - an invalid display handle was passed in
 */
/*
 * Default getDisplayType.
 * Returns __hwc2_display_t::type.
 */
static int32_t /*hwc2_error_t*/ get_display_type(
        hwc2_device_t* device, hwc2_display_t display,
        int32_t* /*hwc2_display_type_t*/ outType)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;

    sanity_check_display(device, dpy);

    *outType = dpy->type;

    /*
     * Disabled trace for this function. The 'vsync' callback will call
     * into this function.
     */
    return HWC2_ERROR_NONE;
}

/* getDozeSupport(..., outSupport)
 * Descriptor: HWC2_FUNCTION_GET_DOZE_SUPPORT
 * Must be provided by all HWC2 devices
 *
 * Returns whether the given display supports HWC2_POWER_MODE_DOZE and
 * HWC2_POWER_MODE_DOZE_SUSPEND. DOZE_SUSPEND may not provide any benefit over
 * DOZE (see the definition of hwc2_power_mode_t for more information), but if
 * both DOZE and DOZE_SUSPEND are no different from HWC2_POWER_MODE_ON, the
 * device should not claim support.
 *
 * Parameters:
 *   outSupport - whether the display supports doze modes (1 for yes, 0 for no);
 *       pointer will be non-NULL
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_DISPLAY - an invalid display handle was passed in
 */
/*
 * Default getDozeSupport.
 * Returns __hwc2_display_t::dozeSupport.
 */
static int32_t /*hwc2_error_t*/ get_doze_support(
        hwc2_device_t* device, hwc2_display_t display, int32_t* outSupport)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;

    sanity_check_display(device, dpy);

    __hwc2_trace(0, "dpy=%p", dpy);

    *outSupport = dpy->dozeSupport;

    __hwc2_trace(1, "out outSupport=%d", *outSupport);
    return HWC2_ERROR_NONE;
}

/* getHdrCapabilities(..., outNumTypes, outTypes, outMaxLuminance,
 *     outMaxAverageLuminance, outMinLuminance)
 * Descriptor: HWC2_FUNCTION_GET_HDR_CAPABILITIES
 * Must be provided by all HWC2 devices
 *
 * Returns the high dynamic range (HDR) capabilities of the given display, which
 * are invariant with regard to the active configuration.
 *
 * Displays which are not HDR-capable must return no types in outTypes and set
 * outNumTypes to 0.
 *
 * If outTypes is NULL, the required number of HDR types must be returned in
 * outNumTypes.
 *
 * Parameters:
 *   outNumTypes - if outTypes was NULL, the number of types which would have
 *       been returned; if it was not NULL, the number of types stored in
 *       outTypes, which must not exceed the value stored in outNumTypes prior
 *       to the call; pointer will be non-NULL
 *   outTypes - an array of HDR types, may have 0 elements if the display is not
 *       HDR-capable
 *   outMaxLuminance - the desired content maximum luminance for this display in
 *       cd/m^2; pointer will be non-NULL
 *   outMaxAverageLuminance - the desired content maximum frame-average
 *       luminance for this display in cd/m^2; pointer will be non-NULL
 *   outMinLuminance - the desired content minimum luminance for this display in
 *       cd/m^2; pointer will be non-NULL
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_DISPLAY - an invalid display handle was passed in
 */
/*
 * Default getHdrCapabilities.
 * Returns __hwc2_display_t::hdrTypes,
 *         __hwc2_display_t::maxLuminance,
 *         __hwc2_display_t::maxAverageLuminance,
 *         __hwc2_display_t::minLuminance.
 */
static int32_t /*hwc2_error_t*/ get_hdr_capabilities(
        hwc2_device_t* device, hwc2_display_t display, uint32_t* outNumTypes,
        int32_t* /*android_hdr_t*/ outTypes, float* outMaxLuminance,
        float* outMaxAverageLuminance, float* outMinLuminance)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;

    sanity_check_display(device, dpy);

    __hwc2_trace(0, "dpy=%p outTypes=%p", dpy, outTypes);

    if (!outTypes) {
        *outNumTypes = dpy->numHdrTypes;
    } else {
        if (*outNumTypes > dpy->numHdrTypes) {
            *outNumTypes = dpy->numHdrTypes;
        }

        for (uint32_t i = 0; i < *outNumTypes; i++) {
            __hwc2_trace(2, "out outTypes[%d]=%i", i, outTypes[i]);
            outTypes[i] = dpy->hdrTypes[i];
        }
    }

    *outMaxLuminance        = dpy->maxLuminance;
    *outMaxAverageLuminance = dpy->maxAverageLuminance;
    *outMinLuminance        = dpy->minLuminance;

    __hwc2_trace(1, "out outNumTypes=%u "
        "outMaxLuminance=%f outMaxAverageLuminance=%f outMinLuminance=%f",
        *outNumTypes,
        *outMaxLuminance, *outMaxAverageLuminance, *outMinLuminance);
    return HWC2_ERROR_NONE;
}

/* getReleaseFences(..., outNumElements, outLayers, outFences)
 * Descriptor: HWC2_FUNCTION_GET_RELEASE_FENCES
 * Must be provided by all HWC2 devices
 *
 * Retrieves the release fences for device layers on this display which will
 * receive new buffer contents this frame.
 *
 * A release fence is a file descriptor referring to a sync fence object which
 * will be signaled after the device has finished reading from the buffer
 * presented in the prior frame. This indicates that it is safe to start writing
 * to the buffer again. If a given layer's fence is not returned from this
 * function, it will be assumed that the buffer presented on the previous frame
 * is ready to be written.
 *
 * The fences returned by this function should be unique for each layer (even if
 * they point to the same underlying sync object), and ownership of the fences
 * is transferred to the client, which is responsible for closing them.
 *
 * If outLayers or outFences is NULL, the required number of layers and fences
 * must be returned in outNumElements.
 *
 * Parameters:
 *   outNumElements - if outLayers or outFences were NULL, the number of
 *       elements which would have been returned; if both were not NULL, the
 *       number of elements in outLayers and outFences, which must not exceed
 *       the value stored in outNumElements prior to the call; pointer will be
 *       non-NULL
 *   outLayers - an array of layer handles
 *   outFences - an array of sync fence file descriptors as described above,
 *       each corresponding to an element of outLayers
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_DISPLAY - an invalid display handle was passed in
 */
/*
 * Default getReleaseFences.
 * Returns __hwc2_layer_t::releaseFence of all layers.
 */
static int32_t /*hwc2_error_t*/ get_release_fences(
        hwc2_device_t* device, hwc2_display_t display, uint32_t* outNumElements,
        hwc2_layer_t* outLayers, int32_t* outFences)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;
    __hwc2_list_t *pos;
    uint32_t num = 0;

    sanity_check_display(device, dpy);

    __hwc2_trace(0, "dpy=%p outLayers=%p outFences=%p", dpy, outLayers, outFences);

    if (!outLayers || !outFences) {
        __hwc2_list_for_each(pos, &dpy->layers) {
            __hwc2_layer_t *layer = __hwc2_list_entry(pos, __hwc2_layer, link);
            if (layer->releaseFence != -1) {
                num++;
            }
        }
    } else {
        __hwc2_list_for_each(pos, &dpy->layers) {
            __hwc2_layer_t *layer = __hwc2_list_entry(pos, __hwc2_layer, link);

            if (num == *outNumElements)
                break;

            if (layer->releaseFence != -1) {
                __hwc2_trace(2, "out outLayers[%d]=%p(%u) outFences[%d]=%d",
                        num, layer, layer->id, num, layer->releaseFence);

                outLayers[num] = (hwc2_layer_t)(uintptr_t)layer;
                outFences[num] = layer->releaseFence;
                num++;
            }
        }
    }

    *outNumElements = num;
    __hwc2_trace(1, "out outNumElements=%u", num);
    return HWC2_ERROR_NONE;
}

/* presentDisplay(..., outPresentFence)
 * Descriptor: HWC2_FUNCTION_PRESENT_DISPLAY
 * Must be provided by all HWC2 devices
 *
 * Presents the current display contents on the screen (or in the case of
 * virtual displays, into the output buffer).
 *
 * Prior to calling this function, the display must be successfully validated
 * with validateDisplay. Note that setLayerBuffer and setLayerSurfaceDamage
 * specifically do not count as layer state, so if there are no other changes
 * to the layer state (or to the buffer's properties as described in
 * setLayerBuffer), then it is safe to call this function without first
 * validating the display.
 *
 * If this call succeeds, outPresentFence will be populated with a file
 * descriptor referring to a present sync fence object. For physical displays,
 * this fence will be signaled at the vsync when the result of composition of
 * this frame starts to appear (for video-mode panels) or starts to transfer to
 * panel memory (for command-mode panels). For virtual displays, this fence will
 * be signaled when writes to the output buffer have completed and it is safe to
 * read from it.
 *
 * Parameters:
 *   outPresentFence - a sync fence file descriptor as described above; pointer
 *       will be non-NULL
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_DISPLAY - an invalid display handle was passed in
 *   HWC2_ERROR_NO_RESOURCES - no valid output buffer has been set for a virtual
 *       display
 *   HWC2_ERROR_NOT_VALIDATED - validateDisplay has not successfully been called
 *       for this display
 */
/*
 * presentDisplay.
 * Prints an warning, since can not do default present.
 *
 * XXX: Must implement practical function.
 */
static int32_t /*hwc2_error_t*/ present_display(
        hwc2_device_t* device, hwc2_display_t display, int32_t* outPresentFence)
{
    __hwc2_device_t *dev = (__hwc2_device_t *)device;
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;

    sanity_check_display(device, dpy);

    __hwc2_trace(0, "dpy=%p", dpy);

    if (dpy->type == HWC2_DISPLAY_TYPE_VIRTUAL) {
        __hwc2_queue_blit_virtual(dev, dpy, outPresentFence);
    } else {
        /* composerSel must be disabled already. */
        __hwc2_queue_blit(dev, dpy, NULL, NULL, outPresentFence);
    }

    int32_t fd = dpy->clientAcquireFence;
    if (fd != -1) {
        sync_wait(fd, -1);
        close(fd);
        dpy->clientAcquireFence = -1;
    }

    __hwc2_frame_end(dev, dpy);

    ALOGW("presentDisplay: display=%p", dpy);
    *outPresentFence = -1;

    __hwc2_trace(1, "OK");
    return HWC2_ERROR_NONE;
}

/* setActiveConfig(..., config)
 * Descriptor: HWC2_FUNCTION_SET_ACTIVE_CONFIG
 * Must be provided by all HWC2 devices
 *
 * Sets the active configuration for this display. Upon returning, the given
 * display configuration should be active and remain so until either this
 * function is called again or the display is disconnected.
 *
 * Parameters:
 *   config - the new display configuration
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_DISPLAY - an invalid display handle was passed in
 *   HWC2_ERROR_BAD_CONFIG - the configuration handle passed in is not valid for
 *       this display
 */
/*
 * Default setActiveConfig.
 * Set config to __hwc2_display_t::activeConfig and prints a warning message.
 *
 * XXX: Must implement real function, because the given configuration
 * should be active.
 */
static int32_t /*hwc2_error_t*/ set_active_config(
        hwc2_device_t* device, hwc2_display_t display, hwc2_config_t config)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;
    __hwc2_config_t *cfg = NULL;

    sanity_check_display(device, dpy);

    __hwc2_trace(2, "dpy=%p config=%u", dpy, (uint32_t)config);

    cfg = __hwc2_get_config(dpy, config);

    if (unlikely(!cfg)) {
        __hwc2_trace_error(2, HWC2_ERROR_BAD_CONFIG);
        return HWC2_ERROR_BAD_CONFIG;
    }

    dpy->activeConfig = cfg;

    /* update current resolution. */
    dpy->width  = cfg->width;
    dpy->height = cfg->height;

    ALOGW("setActiveConfig: dpy=%p config=%u", dpy, config);
    return HWC2_ERROR_NONE;
}

/* setClientTarget(..., target, acquireFence, dataspace, damage)
 * Descriptor: HWC2_FUNCTION_SET_CLIENT_TARGET
 * Must be provided by all HWC2 devices
 *
 * Sets the buffer handle which will receive the output of client composition.
 * Layers marked as HWC2_COMPOSITION_CLIENT will be composited into this buffer
 * prior to the call to presentDisplay, and layers not marked as
 * HWC2_COMPOSITION_CLIENT should be composited with this buffer by the device.
 *
 * The buffer handle provided may be null if no layers are being composited by
 * the client. This must not result in an error (unless an invalid display
 * handle is also provided).
 *
 * Also provides a file descriptor referring to an acquire sync fence object,
 * which will be signaled when it is safe to read from the client target buffer.
 * If it is already safe to read from this buffer, -1 may be passed instead.
 * The device must ensure that it is safe for the client to close this file
 * descriptor at any point after this function is called.
 *
 * For more about dataspaces, see setLayerDataspace.
 *
 * The damage parameter describes a surface damage region as defined in the
 * description of setLayerSurfaceDamage.
 *
 * Will be called before presentDisplay if any of the layers are marked as
 * HWC2_COMPOSITION_CLIENT. If no layers are so marked, then it is not
 * necessary to call this function. It is not necessary to call validateDisplay
 * after changing the target through this function.
 *
 * Parameters:
 *   target - the new target buffer
 *   acquireFence - a sync fence file descriptor as described above
 *   dataspace - the dataspace of the buffer, as described in setLayerDataspace
 *   damage - the surface damage region
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_DISPLAY - an invalid display handle was passed in
 *   HWC2_ERROR_BAD_PARAMETER - the new target handle was invalid
 */
/*
 * Default setClientTarget.
 * Sets __hwc2_display_t::clientTarget, etc.
 */
static int32_t /*hwc2_error_t*/ set_client_target(
        hwc2_device_t* device, hwc2_display_t display, buffer_handle_t target,
        int32_t acquireFence, int32_t /*android_dataspace_t*/ dataspace,
        hwc_region_t damage)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;

    sanity_check_buffer(target);
    sanity_check_display(device, dpy);

    __hwc2_trace(2, "dpy=%p target=%p acquireFence=%d dataspace=%d damage=",
            dpy, target, acquireFence, dataspace);
    __hwc2_trace_region(damage);

    dpy->clientTarget = target;
    dpy->clientAcquireFence = acquireFence;
    dpy->clientDataspace = dataspace;
    __hwc2_region_copy(&dpy->clientDamage, damage);

    __hwc2_trace_init();

    return HWC2_ERROR_NONE;
}

/* setColorMode(..., mode)
 * Descriptor: HWC2_FUNCTION_SET_COLOR_MODE
 * Must be provided by all HWC2 devices
 *
 * Sets the color mode of the given display.
 *
 * Upon returning from this function, the color mode change must have fully
 * taken effect.
 *
 * The valid color modes can be found in android_color_mode_t in
 * <system/graphics.h>. All HWC2 devices must support at least
 * HAL_COLOR_MODE_NATIVE, and displays are assumed to be in this mode upon
 * hotplug.
 *
 * Parameters:
 *   mode - the mode to set
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_DISPLAY - an invalid display handle was passed in
 *   HWC2_ERROR_BAD_PARAMETER - mode is not a valid color mode
 *   HWC2_ERROR_UNSUPPORTED - mode is not supported on this display
 */
/*
 * Default setColorMode.
 * Checks color mode and sets to __hwc2_display_t::colorMode.
 *
 * XXX: Should implement this function, because color mode change must have
 * fully taken effect upon returning from this function.
 */
static int32_t /*hwc2_error_t*/ set_color_mode(
        hwc2_device_t* device, hwc2_display_t display,
        int32_t /*android_color_mode_t*/ mode)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;

    sanity_check_display(device, dpy);

    __hwc2_trace(2, "dpy=%p mode=%d", dpy, mode);

    ALOGW("setColorMode: dpy=%p mode=%" PRIi32, dpy, mode);

    for (uint32_t i = 0; i < dpy->numColorModes; i++) {
        if (dpy->colorModes[i] == mode) {
            /* Supported color mode. */
            dpy->colorMode = mode;
            return HWC2_ERROR_NONE;
        }
    }

    return HWC2_ERROR_BAD_PARAMETER;
}

/* setColorTransform(..., matrix, hint)
 * Descriptor: HWC2_FUNCTION_SET_COLOR_TRANSFORM
 * Must be provided by all HWC2 devices
 *
 * Sets a color transform which will be applied after composition.
 *
 * If hint is not HAL_COLOR_TRANSFORM_ARBITRARY, then the device may use the
 * hint to apply the desired color transform instead of using the color matrix
 * directly.
 *
 * If the device is not capable of either using the hint or the matrix to apply
 * the desired color transform, it should force all layers to client composition
 * during validateDisplay.
 *
 * If HWC2_CAPABILITY_SKIP_CLIENT_COLOR_TRANSFORM is present, then the client
 * will never apply the color transform during client composition, even if all
 * layers are being composed by the client.
 *
 * The matrix provided is an affine color transformation of the following form:
 *
 * |r.r r.g r.b 0|
 * |g.r g.g g.b 0|
 * |b.r b.g b.b 0|
 * |Tr  Tg  Tb  1|
 *
 * This matrix will be provided in row-major form: {r.r, r.g, r.b, 0, g.r, ...}.
 *
 * Given a matrix of this form and an input color [R_in, G_in, B_in], the output
 * color [R_out, G_out, B_out] will be:
 *
 * R_out = R_in * r.r + G_in * g.r + B_in * b.r + Tr
 * G_out = R_in * r.g + G_in * g.g + B_in * b.g + Tg
 * B_out = R_in * r.b + G_in * g.b + B_in * b.b + Tb
 *
 * Parameters:
 *   matrix - a 4x4 transform matrix (16 floats) as described above
 *   hint - a hint value which may be used instead of the given matrix unless it
 *       is HAL_COLOR_TRANSFORM_ARBITRARY
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_DISPLAY - an invalid display handle was passed in
 *   HWC2_ERROR_BAD_PARAMETER - hint is not a valid color transform hint
 */
/*
 * Default setColorTransform.
 * Sets __hwc2_display_t::colorTransform, __hwc2_display_t::colorTransformHint.
 */
static int32_t /*hwc2_error_t*/ set_color_transform(
        hwc2_device_t* device, hwc2_display_t display, const float* matrix,
        int32_t /*android_color_transform_t*/ hint)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;

    sanity_check_display(device, dpy);

    __hwc2_trace(2, "dpy=%p matrix=%p hint=%x", dpy, matrix, hint);

    dpy->colorTransformHint = hint;
    memcpy(dpy->colorTransform, matrix, sizeof(float) * 16);

    return HWC2_ERROR_NONE;
}

/* setOutputBuffer(..., buffer, releaseFence)
 * Descriptor: HWC2_FUNCTION_SET_OUTPUT_BUFFER
 * Must be provided by all HWC2 devices
 *
 * Sets the output buffer for a virtual display. That is, the buffer to which
 * the composition result will be written.
 *
 * Also provides a file descriptor referring to a release sync fence object,
 * which will be signaled when it is safe to write to the output buffer. If it
 * is already safe to write to the output buffer, -1 may be passed instead. The
 * device must ensure that it is safe for the client to close this file
 * descriptor at any point after this function is called.
 *
 * Must be called at least once before presentDisplay, but does not have any
 * interaction with layer state or display validation.
 *
 * Parameters:
 *   buffer - the new output buffer
 *   releaseFence - a sync fence file descriptor as described above
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_DISPLAY - an invalid display handle was passed in
 *   HWC2_ERROR_BAD_PARAMETER - the new output buffer handle was invalid
 *   HWC2_ERROR_UNSUPPORTED - display does not refer to a virtual display
 */
/*
 * Default setOutputBuffer.
 * Sets __hwc2_display_t::outputBuffer, __hwc2_display_t::outputReleaseFence.
 */
static int32_t /*hwc2_error_t*/ set_output_buffer(
        hwc2_device_t* device, hwc2_display_t display, buffer_handle_t buffer,
        int32_t releaseFence)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;

    sanity_check_buffer(buffer);
    sanity_check_display(device, dpy);

    __hwc2_trace(2, "dpy=%p buffer=%p releaseFence=%d", dpy, buffer, releaseFence);

    if (unlikely(dpy->type != HWC2_DISPLAY_TYPE_VIRTUAL)) {
        __hwc2_trace_error(1, HWC2_ERROR_UNSUPPORTED);
        return HWC2_ERROR_UNSUPPORTED;
    }

    dpy->outputBuffer = buffer;
    dpy->outputReleaseFence = releaseFence;

    return HWC2_ERROR_NONE;
}

/* setPowerMode(..., mode)
 * Descriptor: HWC2_FUNCTION_SET_POWER_MODE
 * Must be provided by all HWC2 devices
 *
 * Sets the power mode of the given display. The transition must be complete
 * when this function returns. It is valid to call this function multiple times
 * with the same power mode.
 *
 * All displays must support HWC2_POWER_MODE_ON and HWC2_POWER_MODE_OFF. Whether
 * a display supports HWC2_POWER_MODE_DOZE or HWC2_POWER_MODE_DOZE_SUSPEND may
 * be queried using getDozeSupport.
 *
 * Parameters:
 *   mode - the new power mode
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_DISPLAY - an invalid display handle was passed in
 *   HWC2_ERROR_BAD_PARAMETER - mode was not a valid power mode
 *   HWC2_ERROR_UNSUPPORTED - mode was a valid power mode, but is not supported
 *       on this display
 */
/*
 * Default setPowerMode.
 * Sets to __hwc2_display_t::powerMode and prints a warning message.
 *
 * XXX: Must implement real function, because the transition must be complete
 * when this function returns.
 */
static int32_t /*hwc2_error_t*/ set_power_mode(
        hwc2_device_t* device, hwc2_display_t display,
        int32_t /*hwc2_power_mode_t*/ mode)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;

    sanity_check_display(device, dpy);

    __hwc2_trace(2, "dpy=%p mode=%s(%i)", dpy, power_mode_name(mode), mode);

    ALOGW("setPowerMode: dpy=%p mode=%i", dpy, mode);

    switch (mode) {
        case HWC2_POWER_MODE_OFF:
        case HWC2_POWER_MODE_DOZE:
        case HWC2_POWER_MODE_DOZE_SUSPEND:
        case HWC2_POWER_MODE_ON:
            dpy->powerMode = mode;
            break;
        default:
            __hwc2_trace_error(2, HWC2_ERROR_BAD_PARAMETER);
            return HWC2_ERROR_BAD_PARAMETER;
    }

    return HWC2_ERROR_NONE;
}

/* setVsyncEnabled(..., enabled)
 * Descriptor: HWC2_FUNCTION_SET_VSYNC_ENABLED
 * Must be provided by all HWC2 devices
 *
 * Enables or disables the vsync signal for the given display. Virtual displays
 * never generate vsync callbacks, and any attempt to enable vsync for a virtual
 * display though this function must return HWC2_ERROR_NONE and have no other
 * effect.
 *
 * Parameters:
 *   enabled - whether to enable or disable vsync
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_DISPLAY - an invalid display handle was passed in
 *   HWC2_ERROR_BAD_PARAMETER - enabled was an invalid value
 */
/*
 * Default setVsyncEnabled.
 * Sets to __hwc2_display_t::vsyncEnabled blindly.
 */
static int32_t /*hwc2_error_t*/ set_vsync_enabled(
        hwc2_device_t* device, hwc2_display_t display,
        int32_t /*hwc2_vsync_t*/ enabled)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;

    sanity_check_display(device, dpy);

    __hwc2_trace(2, "dpy=%p enabled=%s(%d)", dpy, vsync_name(enabled), enabled);

    if (dpy->type != HWC2_DISPLAY_TYPE_PHYSICAL) {
        __hwc2_trace_error(2, HWC2_ERROR_BAD_PARAMETER);
        return HWC2_ERROR_BAD_PARAMETER;
    }

    if (unlikely(enabled != HWC2_VSYNC_ENABLE &&
        enabled != HWC2_VSYNC_DISABLE)) {
        __hwc2_trace_error(2, HWC2_ERROR_BAD_PARAMETER);
        return HWC2_ERROR_BAD_PARAMETER;
    }

    dpy->vsyncEnabled = (enabled == HWC2_VSYNC_ENABLE);

    return HWC2_ERROR_NONE;
}

/* validateDisplay(..., outNumTypes, outNumRequests)
 * Descriptor: HWC2_FUNCTION_VALIDATE_DISPLAY
 * Must be provided by all HWC2 devices
 *
 * Instructs the device to inspect all of the layer state and determine if
 * there are any composition type changes necessary before presenting the
 * display. Permitted changes are described in the definition of
 * hwc2_composition_t above.
 *
 * Also returns the number of layer requests required
 * by the given layer configuration.
 *
 * Parameters:
 *   outNumTypes - the number of composition type changes required by the
 *       device; if greater than 0, the client must either set and validate new
 *       types, or call acceptDisplayChanges to accept the changes returned by
 *       getChangedCompositionTypes; must be the same as the number of changes
 *       returned by getChangedCompositionTypes (see the declaration of that
 *       function for more information); pointer will be non-NULL
 *   outNumRequests - the number of layer requests required by this layer
 *       configuration; must be equal to the number of layer requests returned
 *       by getDisplayRequests (see the declaration of that function for
 *       more information); pointer will be non-NULL
 *
 * Returns HWC2_ERROR_NONE if no changes are necessary and it is safe to present
 * the display using the current layer state. Otherwise returns one of the
 * following errors:
 *   HWC2_ERROR_BAD_DISPLAY - an invalid display handle was passed in
 *   HWC2_ERROR_HAS_CHANGES - outNumTypes was greater than 0 (see parameter list
 *       for more information)
 */
/*
 * Default validate display.
 * Validate with blitter.
 *
 * XXX: Need override it to support composition by other means rather than blitter.
 */
static int32_t /*hwc2_error_t*/ validate_display(
        hwc2_device_t* device, hwc2_display_t display,
        uint32_t* outNumTypes, uint32_t* outNumRequests)
{
    __hwc2_device_t *dev = (__hwc2_device_t *)device;
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;
    __hwc2_list_t *pos;
    int32_t numTypes    = 0;
    int32_t numRequests = 0;

    __hwc2_trace(0, "dpy=%p", dpy);

    sanity_check_display(device, dpy);

    if (dpy->validation == HWC2_ERROR_NONE) {
        /* Nothing changed since last frame. */
        __hwc2_trace(2, "Validated");
        goto out;
    }

    /* Reinit layersChanged list. */
    __hwc2_list_init(&dpy->layersChanged);

    /* Validate blitter, will be disabled below. */
    __hwc2_validate_blitter(dev, dpy);

    /*
     * No device composition in default function, set all to
     * CLIENT composition.
     */
    __hwc2_list_for_each(pos, &dpy->layers) {
        __hwc2_layer_t *layer = __hwc2_list_entry(pos, __hwc2_layer_t, link);

        /*
         * XXX: Do not use blitter composition in default function.
         * Blitter requires extra output buffer, but not provided in default
         * configuration.
         */
        layer->composerSel = COMP_USE_NULL;

        if (layer->composition != HWC2_COMPOSITION_CLIENT) {
            /* Add into changed list. */
            __hwc2_list_add_tail(&layer->linkChanged, &dpy->layersChanged);

            layer->compositionChanged = HWC2_COMPOSITION_CLIENT;
            numTypes++;
        }

        if (layer->request != 0)
            numRequests++;
    }

out:
    if (!numTypes) {
        __hwc2_list_t *pos;
        __hwc2_layer_t *layer;

        __hwc2_list_for_each(pos, &dpy->layers) {
            layer = __hwc2_list_entry(pos, __hwc2_layer_t, link);
            int32_t fd = layer->acquireFence;

            /* Directly close acquireFence for CLIENT composited layers. */
            if (layer->composition == HWC2_COMPOSITION_CLIENT && fd != -1) {
                close(fd);
                layer->acquireFence = -1;
            }
        }
    }

    *outNumTypes    = numTypes;
    *outNumRequests = numRequests;

    dpy->validation = (numTypes > 0) ? HWC2_ERROR_HAS_CHANGES
                    : HWC2_ERROR_NONE;

    /* Begines validate/present sequence. */
    dpy->lockCursor = 1;

    __hwc2_trace(1, "out outNumTypes=%u outNumRequests=%u return=%s(%d)",
            numTypes, numRequests, error_name(dpy->validation),
            dpy->validation);

    return dpy->validation;
}

/*
 * Layer Functions
 *
 * These are functions which operate on layers, but which do not modify state
 * that must be validated before use. See also 'Layer State Functions' below.
 *
 * All of these functions take as their first three parameters a device pointer,
 * a display handle for the display which contains the layer, and a layer
 * handle, so these parameters are omitted from the described parameter lists.
 */

/* setCursorPosition(..., x, y)
 * Descriptor: HWC2_FUNCTION_SET_CURSOR_POSITION
 * Must be provided by all HWC2 devices
 *
 * Asynchonously sets the position of a cursor layer.
 *
 * Prior to validateDisplay, a layer may be marked as HWC2_COMPOSITION_CURSOR.
 * If validation succeeds (i.e., the device does not request a composition
 * change for that layer), then once a buffer has been set for the layer and it
 * has been presented, its position may be set by this function at any time
 * between presentDisplay and any subsequent validateDisplay calls for this
 * display.
 *
 * Once validateDisplay is called, this function will not be called again until
 * the validate/present sequence is completed.
 *
 * May be called from any thread so long as it is not interleaved with the
 * validate/present sequence as described above.
 *
 * Parameters:
 *   x - the new x coordinate (in pixels from the left of the screen)
 *   y - the new y coordinate (in pixels from the top of the screen)
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_DISPLAY - an invalid display handle was passed in
 *   HWC2_ERROR_BAD_LAYER - the layer is invalid or is not currently marked as
 *       HWC2_COMPOSITION_CURSOR
 *   HWC2_ERROR_NOT_VALIDATED - the device is currently in the middle of the
 *       validate/present sequence
 */
/*
 * Default setCursorPosition.
 * Sets __hwc2_layer_t::cursorX, __hwc2_layer_t_cursorY.
 *
 * XXX: Need override it for hardware cursor.
 */
static int32_t /*hwc2_error_t*/ set_cursor_position(
        hwc2_device_t* device, hwc2_display_t display, hwc2_layer_t layer,
        int32_t x, int32_t y)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;
    __hwc2_layer_t *ly = (__hwc2_layer_t *)(uintptr_t)layer;

    sanity_check_display(device, dpy);
    sanity_check_layer(dpy, ly);

    __hwc2_trace(2, "dpy=%p layer=%p(%u) x=%d y=%d", dpy, ly, ly->id, x, y);

    if (ly->composition != HWC2_COMPOSITION_CURSOR) {
        __hwc2_trace_error(2, HWC2_ERROR_BAD_LAYER);
        return HWC2_ERROR_BAD_LAYER;
    }

    if (dpy->lockCursor) {
        __hwc2_trace_error(2, HWC2_ERROR_NOT_VALIDATED);
        return HWC2_ERROR_NOT_VALIDATED;
    }

    ly->cursorX = x;
    ly->cursorY = y;

    return HWC2_ERROR_NONE;
}

/* setLayerBuffer(..., buffer, acquireFence)
 * Descriptor: HWC2_FUNCTION_SET_LAYER_BUFFER
 * Must be provided by all HWC2 devices
 *
 * Sets the buffer handle to be displayed for this layer. If the buffer
 * properties set at allocation time (width, height, format, and usage) have not
 * changed since the previous frame, it is not necessary to call validateDisplay
 * before calling presentDisplay unless new state needs to be validated in the
 * interim.
 *
 * Also provides a file descriptor referring to an acquire sync fence object,
 * which will be signaled when it is safe to read from the given buffer. If it
 * is already safe to read from the buffer, -1 may be passed instead. The
 * device must ensure that it is safe for the client to close this file
 * descriptor at any point after this function is called.
 *
 * This function must return HWC2_ERROR_NONE and have no other effect if called
 * for a layer with a composition type of HWC2_COMPOSITION_SOLID_COLOR (because
 * it has no buffer) or HWC2_COMPOSITION_SIDEBAND or HWC2_COMPOSITION_CLIENT
 * (because synchronization and buffer updates for these layers are handled
 * elsewhere).
 *
 * Parameters:
 *   buffer - the buffer handle to set
 *   acquireFence - a sync fence file descriptor as described above
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_LAYER - an invalid layer handle was passed in
 *   HWC2_ERROR_BAD_PARAMETER - the buffer handle passed in was invalid
 */
/*
 * Default setLayerBuffer.
 * Sets to __hwc2_layer_t::buffer and __hwc2_layer_t::acquireFence.
 */
static int32_t /*hwc2_error_t*/ set_layer_buffer(
        hwc2_device_t* device, hwc2_display_t display, hwc2_layer_t layer,
        buffer_handle_t buffer, int32_t acquireFence)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;
    __hwc2_layer_t *ly = (__hwc2_layer_t *)(uintptr_t)layer;

    sanity_check_buffer(buffer);
    sanity_check_display(device, dpy);
    sanity_check_layer(dpy, ly);

    __hwc2_trace(2, "dpy=%p layer=%p(%u) buffer=%p acquireFence=%d",
            dpy, ly, ly->id, buffer, acquireFence);

    /*
     * XXX:
     * Seems android framework bug here:
     * Need close the 'acquireFence', otherwise fd leaks.
     * But close it does not following the description above in comments.
     *
     * 'acquireFence' close is when safe to present display, ie,
     * 1. validateDisplay (when no changes)
     * 2. acceptDisplayChanges
     */
    ly->buffer = buffer;
    ly->acquireFence = acquireFence;

    return HWC2_ERROR_NONE;
}

/* setLayerSurfaceDamage(..., damage)
 * Descriptor: HWC2_FUNCTION_SET_LAYER_SURFACE_DAMAGE
 * Must be provided by all HWC2 devices
 *
 * Provides the region of the source buffer which has been modified since the
 * last frame. This region does not need to be validated before calling
 * presentDisplay.
 *
 * Once set through this function, the damage region remains the same until a
 * subsequent call to this function.
 *
 * If damage.numRects > 0, then it may be assumed that any portion of the source
 * buffer not covered by one of the rects has not been modified this frame. If
 * damage.numRects == 0, then the whole source buffer must be treated as if it
 * has been modified.
 *
 * If the layer's contents are not modified relative to the prior frame, damage
 * will contain exactly one empty rect([0, 0, 0, 0]).
 *
 * The damage rects are relative to the pre-transformed buffer, and their origin
 * is the top-left corner. They will not exceed the dimensions of the latched
 * buffer.
 *
 * Parameters:
 *   damage - the new surface damage region
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_LAYER - an invalid layer handle was passed in
 */
/*
 * Default setLayerSurfaceDamage.
 * Sets to __hwc2_layer_t::surfaceDamage.
 */
static int32_t /*hwc2_error_t*/ set_layer_surface_damage(
        hwc2_device_t* device, hwc2_display_t display, hwc2_layer_t layer,
        hwc_region_t damage)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;
    __hwc2_layer_t *ly = (__hwc2_layer_t *)(uintptr_t)layer;

    sanity_check_display(device, dpy);
    sanity_check_layer(dpy, ly);

    __hwc2_trace(2, "dpy=%p layer=%p(%u) damage=", dpy, ly, ly->id);
    __hwc2_trace_region(damage);

    __hwc2_region_copy(&ly->surfaceDamage, damage);

    return HWC2_ERROR_NONE;
}

/*
 * Layer State Functions
 *
 * These functions modify the state of a given layer. They do not take effect
 * until the display configuration is successfully validated with
 * validateDisplay and the display contents are presented with presentDisplay.
 *
 * All of these functions take as their first three parameters a device pointer,
 * a display handle for the display which contains the layer, and a layer
 * handle, so these parameters are omitted from the described parameter lists.
 */

/* setLayerBlendMode(..., mode)
 * Descriptor: HWC2_FUNCTION_SET_LAYER_BLEND_MODE
 * Must be provided by all HWC2 devices
 *
 * Sets the blend mode of the given layer.
 *
 * Parameters:
 *   mode - the new blend mode
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_LAYER - an invalid layer handle was passed in
 *   HWC2_ERROR_BAD_PARAMETER - an invalid blend mode was passed in
 */
/*
 * Default setLayerBlendMode
 * Sets to __hwc2_layer_t::blendMode
 */
static int32_t /*hwc2_error_t*/ set_layer_blend_mode(
        hwc2_device_t* device, hwc2_display_t display, hwc2_layer_t layer,
        int32_t /*hwc2_blend_mode_t*/ mode)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;
    __hwc2_layer_t *ly = (__hwc2_layer_t *)(uintptr_t)layer;

    sanity_check_display(device, dpy);
    sanity_check_layer(dpy, ly);

    __hwc2_trace(2, "dpy=%p layer=%p(%u) mode=%s(%x)",
            dpy, ly, ly->id, blend_mode_name(mode), mode);

    if (ly->blendMode == mode) {
        __hwc2_trace_string("  Not changed");
        return HWC2_ERROR_NONE;
    }

    switch (mode) {
        case HWC2_BLEND_MODE_NONE:
        case HWC2_BLEND_MODE_PREMULTIPLIED:
        case HWC2_BLEND_MODE_COVERAGE:
            break;
        default:
            __hwc2_trace_error(2, HWC2_ERROR_BAD_PARAMETER);
            return HWC2_ERROR_BAD_PARAMETER;
    }

    ly->blendMode = mode;

    /* Blend mode also impacts geometry. */
    ly->blendChanged    = 1;
    ly->geometryChanged = 1;

    /* Requires validation. */
    dpy->validation = HWC2_ERROR_NOT_VALIDATED;
    return HWC2_ERROR_NONE;
}

/* setLayerColor(..., color)
 * Descriptor: HWC2_FUNCTION_SET_LAYER_COLOR
 * Must be provided by all HWC2 devices
 *
 * Sets the color of the given layer. If the composition type of the layer is
 * not HWC2_COMPOSITION_SOLID_COLOR, this call must return HWC2_ERROR_NONE and
 * have no other effect.
 *
 * Parameters:
 *   color - the new color
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_LAYER - an invalid layer handle was passed in
 */
/*
 * Default setLayerColor
 * Sets __hwc2_layer_t::solidColor
 */
static int32_t /*hwc2_error_t*/ set_layer_color(
        hwc2_device_t* device, hwc2_display_t display, hwc2_layer_t layer,
        hwc_color_t color)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;
    __hwc2_layer_t *ly = (__hwc2_layer_t *)(uintptr_t)layer;

    sanity_check_display(device, dpy);
    sanity_check_layer(dpy, ly);

    __hwc2_trace(2, "dpy=%p layer=%p(%u) color=[%d,%d,%d,%d]",
            dpy, ly, ly->id, color.r, color.g, color.b, color.a);

    if ((ly->solidColor.r == color.r) && (ly->solidColor.g == color.g) &&
        (ly->solidColor.b == color.b) && (ly->solidColor.a == color.a)) {
        __hwc2_trace_string("  Not changed");
        return HWC2_ERROR_NONE;
    }

    if (ly->composition != HWC2_COMPOSITION_SOLID_COLOR) {
        /* no effect. */
        __hwc2_trace(2, "SKIP");
        return HWC2_ERROR_NONE;
    }

    ly->solidColor   = color;
    ly->colorChanged = 1;

    /* Requires validation. */
    dpy->validation = HWC2_ERROR_NOT_VALIDATED;
    return HWC2_ERROR_NONE;
}

/* setLayerCompositionType(..., type)
 * Descriptor: HWC2_FUNCTION_SET_LAYER_COMPOSITION_TYPE
 * Must be provided by all HWC2 devices
 *
 * Sets the desired composition type of the given layer. During validateDisplay,
 * the device may request changes to the composition types of any of the layers
 * as described in the definition of hwc2_composition_t above.
 *
 * Parameters:
 *   type - the new composition type
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_LAYER - an invalid layer handle was passed in
 *   HWC2_ERROR_BAD_PARAMETER - an invalid composition type was passed in
 *   HWC2_ERROR_UNSUPPORTED - a valid composition type was passed in, but it is
 *       not supported by this device
 */
/*
 * Default setLayerCompositionType.
 * Sets __hwc2_layer_t::composition.
 */
static int32_t /*hwc2_error_t*/ set_layer_composition_type(
        hwc2_device_t* device, hwc2_display_t display, hwc2_layer_t layer,
        int32_t /*hwc2_composition_t*/ type)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;
    __hwc2_layer_t *ly = (__hwc2_layer_t *)(uintptr_t)layer;

    sanity_check_display(device, dpy);
    sanity_check_layer(dpy, ly);

    __hwc2_trace(2, "dpy=%p layer=%p(%u) type=%s(%i)",
        dpy, ly, ly->id, composition_name(type), type);

    if (ly->composition == type) {
        __hwc2_trace_string("  Not changed");
        return HWC2_ERROR_NONE;
    }

    switch (type) {
        case HWC2_COMPOSITION_CLIENT:
        case HWC2_COMPOSITION_DEVICE:
        case HWC2_COMPOSITION_SOLID_COLOR:
        case HWC2_COMPOSITION_CURSOR:
        case HWC2_COMPOSITION_SIDEBAND:
            break;
        default:
            __hwc2_trace_error(2, HWC2_ERROR_BAD_PARAMETER);
            return HWC2_ERROR_BAD_PARAMETER;
    }

    ly->composition = type;

    /* Requires validation. */
    dpy->validation = HWC2_ERROR_NOT_VALIDATED;
    return HWC2_ERROR_NONE;
}

/* setLayerDataspace(..., dataspace)
 * Descriptor: HWC2_FUNCTION_SET_LAYER_DATASPACE
 * Must be provided by all HWC2 devices
 *
 * Sets the dataspace that the current buffer on this layer is in.
 *
 * The dataspace provides more information about how to interpret the buffer
 * contents, such as the encoding standard and color transform.
 *
 * See the values of android_dataspace_t in <system/graphics.h> for more
 * information.
 *
 * Parameters:
 *   dataspace - the new dataspace
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_LAYER - an invalid layer handle was passed in
 */
/*
 * Default setLayerDataspace
 * Sets to __hwc2_layer_t::dataspace.
 */
static int32_t /*hwc2_error_t*/ set_layer_dataspace(
        hwc2_device_t* device, hwc2_display_t display, hwc2_layer_t layer,
        int32_t /*android_dataspace_t*/ dataspace)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;
    __hwc2_layer_t *ly = (__hwc2_layer_t *)(uintptr_t)layer;

    sanity_check_display(device, dpy);
    sanity_check_layer(dpy, ly);

    __hwc2_trace(2, "dpy=%p layer=%p(%u) dataspace=%d",
            dpy, ly, ly->id, dataspace);

    if (ly->dataspace == dataspace) {
        __hwc2_trace_string("  Not changed");
        return HWC2_ERROR_NONE;
    }

    ly->dataspace    = dataspace;
    ly->colorChanged = 1;

    /* Requires validation. */
    dpy->validation = HWC2_ERROR_NOT_VALIDATED;
    return HWC2_ERROR_NONE;
}

/* setLayerDisplayFrame(..., frame)
 * Descriptor: HWC2_FUNCTION_SET_LAYER_DISPLAY_FRAME
 * Must be provided by all HWC2 devices
 *
 * Sets the display frame (the portion of the display covered by a layer) of the
 * given layer. This frame will not exceed the display dimensions.
 *
 * Parameters:
 *   frame - the new display frame
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_LAYER - an invalid layer handle was passed in
 */
/*
 * Default setLayerDisplayFrame
 * Sets to __hwc2_layer_t::displayFrame.
 */
static int32_t /*hwc2_error_t*/ set_layer_display_frame(
        hwc2_device_t* device, hwc2_display_t display, hwc2_layer_t layer,
        hwc_rect_t frame)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;
    __hwc2_layer_t *ly = (__hwc2_layer_t *)(uintptr_t)layer;

    sanity_check_display(device, dpy);
    sanity_check_layer(dpy, ly);

    __hwc2_trace(2, "dpy=%p layer=%p(%u) frame=[%d,%d,%d,%d]",
            dpy, ly, ly->id, frame.left, frame.top, frame.right, frame.bottom);

    if (!memcmp(&ly->displayFrame, &frame, sizeof(hwc_rect_t))) {
        __hwc2_trace_string("  Not changed");
        return HWC2_ERROR_NONE;
    }

    ly->displayFrame    = frame;
    ly->geometryChanged = 1;

    /* Requires validation. */
    dpy->validation = HWC2_ERROR_NOT_VALIDATED;
    return HWC2_ERROR_NONE;
}

/* setLayerPlaneAlpha(..., alpha)
 * Descriptor: HWC2_FUNCTION_SET_LAYER_PLANE_ALPHA
 * Must be provided by all HWC2 devices
 *
 * Sets an alpha value (a floating point value in the range [0.0, 1.0]) which
 * will be applied to the whole layer. It can be conceptualized as a
 * preprocessing step which applies the following function:
 *   if (blendMode == HWC2_BLEND_MODE_PREMULTIPLIED)
 *       out.rgb = in.rgb * planeAlpha
 *   out.a = in.a * planeAlpha
 *
 * If the device does not support this operation on a layer which is marked
 * HWC2_COMPOSITION_DEVICE, it must request a composition type change to
 * HWC2_COMPOSITION_CLIENT upon the next validateDisplay call.
 *
 * Parameters:
 *   alpha - the plane alpha value to apply
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_LAYER - an invalid layer handle was passed in
 */
/*
 * Default setLayerPlaneAlpha
 * Sets to __hwc2_layer_t::planeAlpha.
 */
static int32_t /*hwc2_error_t*/ set_layer_plane_alpha(
        hwc2_device_t* device, hwc2_display_t display, hwc2_layer_t layer,
        float alpha)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;
    __hwc2_layer_t *ly = (__hwc2_layer_t *)(uintptr_t)layer;

    sanity_check_display(device, dpy);
    sanity_check_layer(dpy, ly);

    __hwc2_trace(2, "dpy=%p layer=%p(%u) alpha=%.4f", dpy, ly, ly->id, alpha);

    if (ly->planeAlpha == alpha) {
        __hwc2_trace_string("  Not changed");
        return HWC2_ERROR_NONE;
    }

    ly->planeAlpha   = alpha;
    ly->blendChanged = 1;

    /* Requires validation. */
    dpy->validation = HWC2_ERROR_NOT_VALIDATED;
    return HWC2_ERROR_NONE;
}

/* setLayerSidebandStream(..., stream)
 * Descriptor: HWC2_FUNCTION_SET_LAYER_SIDEBAND_STREAM
 * Provided by HWC2 devices which support HWC2_CAPABILITY_SIDEBAND_STREAM
 *
 * Sets the sideband stream for this layer. If the composition type of the given
 * layer is not HWC2_COMPOSITION_SIDEBAND, this call must return HWC2_ERROR_NONE
 * and have no other effect.
 *
 * Parameters:
 *   stream - the new sideband stream
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_LAYER - an invalid layer handle was passed in
 *   HWC2_ERROR_BAD_PARAMETER - an invalid sideband stream was passed in
 */
/*
 * Default setLayerSidebandStream
 * Sets to __hwc2_layer_t::sidebandStream.
 */
static int32_t /*hwc2_error_t*/ set_layer_sideband_stream(
        hwc2_device_t* device, hwc2_display_t display, hwc2_layer_t layer,
        const native_handle_t* stream)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;
    __hwc2_layer_t *ly = (__hwc2_layer_t *)(uintptr_t)layer;

    sanity_check_buffer(stream);
    sanity_check_display(device, dpy);
    sanity_check_layer(dpy, ly);

    __hwc2_trace(2, "dpy=%p layer=%p(%u) stream=%p", dpy, ly, ly->id, stream);

    if (ly->sidebandStream == stream) {
        __hwc2_trace_string("  Not changed");
        return HWC2_ERROR_NONE;
    }

    ly->sidebandStream = stream;

    /* Requires validation. */
    dpy->validation = HWC2_ERROR_NOT_VALIDATED;
    return HWC2_ERROR_NONE;
}

/* setLayerSourceCrop(..., crop)
 * Descriptor: HWC2_FUNCTION_SET_LAYER_SOURCE_CROP
 * Must be provided by all HWC2 devices
 *
 * Sets the source crop (the portion of the source buffer which will fill the
 * display frame) of the given layer. This crop rectangle will not exceed the
 * dimensions of the latched buffer.
 *
 * If the device is not capable of supporting a true float source crop (i.e., it
 * will truncate or round the floats to integers), it should set this layer to
 * HWC2_COMPOSITION_CLIENT when crop is non-integral for the most accurate
 * rendering.
 *
 * If the device cannot support float source crops, but still wants to handle
 * the layer, it should use the following code (or similar) to convert to
 * an integer crop:
 *   intCrop.left = (int) ceilf(crop.left);
 *   intCrop.top = (int) ceilf(crop.top);
 *   intCrop.right = (int) floorf(crop.right);
 *   intCrop.bottom = (int) floorf(crop.bottom);
 *
 * Parameters:
 *   crop - the new source crop
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_LAYER - an invalid layer handle was passed in
 */
/*
 * Default setLayerSourceCrop
 * Sets to __hwc2_layer_t::sourceCrop.
 */
static int32_t /*hwc2_error_t*/ set_layer_source_crop(
        hwc2_device_t* device, hwc2_display_t display, hwc2_layer_t layer,
        hwc_frect_t crop)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;
    __hwc2_layer_t *ly = (__hwc2_layer_t *)(uintptr_t)layer;

    sanity_check_display(device, dpy);
    sanity_check_layer(dpy, ly);

    __hwc2_trace(2, "dpy=%p layer=%p(%u) crop=[%.2f,%.2f,%.2f,%.2f]",
            dpy, ly, ly->id, crop.left, crop.top, crop.right, crop.bottom);

    if (!memcmp(&ly->sourceCrop, &crop, sizeof(hwc_frect_t))) {
        __hwc2_trace_string("  Not changed");
        return HWC2_ERROR_NONE;
    }

    ly->sourceCrop      = crop;
    ly->geometryChanged = 1;

    /* Requires validation. */
    dpy->validation = HWC2_ERROR_NOT_VALIDATED;
    return HWC2_ERROR_NONE;
}

/* setLayerTransform(..., transform)
 * Descriptor: HWC2_FUNCTION_SET_LAYER_TRANSFORM
 * Must be provided by all HWC2 devices
 *
 * Sets the transform (rotation/flip) of the given layer.
 *
 * Parameters:
 *   transform - the new transform
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_LAYER - an invalid layer handle was passed in
 *   HWC2_ERROR_BAD_PARAMETER - an invalid transform was passed in
 */
/*
 * Default setLayerTransform
 * Sets to __hwc2_layer_t::transform.
 */
static int32_t /*hwc2_error_t*/ set_layer_transform(
        hwc2_device_t* device, hwc2_display_t display, hwc2_layer_t layer,
        int32_t /*hwc_transform_t*/ transform)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;
    __hwc2_layer_t *ly = (__hwc2_layer_t *)(uintptr_t)layer;

    sanity_check_display(device, dpy);
    sanity_check_layer(dpy, ly);

    __hwc2_trace(2, "dpy=%p layer=%p(%u) transform=%s(%x)",
            dpy, ly, ly->id, transform_name(transform), transform);

    if (ly->transform == transform) {
        __hwc2_trace_string("  Not changed");
        return HWC2_ERROR_NONE;
    }

    if (transform & ~0x7) {
        __hwc2_trace_error(2, HWC2_ERROR_BAD_PARAMETER);
        return HWC2_ERROR_BAD_PARAMETER;
    }

    ly->transform       = transform;
    ly->geometryChanged = 1;

    /* Requires validation. */
    dpy->validation = HWC2_ERROR_NOT_VALIDATED;
    return HWC2_ERROR_NONE;
}

/* setLayerVisibleRegion(..., visible)
 * Descriptor: HWC2_FUNCTION_SET_LAYER_VISIBLE_REGION
 * Must be provided by all HWC2 devices
 *
 * Specifies the portion of the layer that is visible, including portions under
 * translucent areas of other layers. The region is in screen space, and will
 * not exceed the dimensions of the screen.
 *
 * Parameters:
 *   visible - the new visible region, in screen space
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_LAYER - an invalid layer handle was passed in
 */
/*
 * Default setLayerVisibleRegion
 * Sets to __hwc2_layer_t::visibleRegion.
 */
static int32_t /*hwc2_error_t*/ set_layer_visible_region(
        hwc2_device_t* device, hwc2_display_t display, hwc2_layer_t layer,
        hwc_region_t visible)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;
    __hwc2_layer_t *ly = (__hwc2_layer_t *)(uintptr_t)layer;

    sanity_check_display(device, dpy);
    sanity_check_layer(dpy, ly);

    __hwc2_trace(2, "dpy=%p layer=%p(%u) visibleRegion=", dpy, ly, ly->id);
    __hwc2_trace_region(visible);

    if (!__hwc2_region_compare(ly->visibleRegion, visible)) {
        __hwc2_trace_string("  Not changed");
        return HWC2_ERROR_NONE;
    }

    if (!ly->visibleRegionChanged)
        __hwc2_region_copy(&ly->visibleRegionPrev, ly->visibleRegion);

    __hwc2_region_copy(&ly->visibleRegion, visible);

    ly->geometryChanged      = 1;
    ly->visibleRegionChanged = 1;

    /* Requires validation. */
    dpy->validation = HWC2_ERROR_NOT_VALIDATED;
    return HWC2_ERROR_NONE;
}

static void inline update_layer_index(__hwc2_device_t *device,
                __hwc2_display_t *dpy)
{
    uint32_t index = 0;
    __hwc2_list_t *pos;

    __hwc2_trace_string("layers of dpy=%p:", dpy);
    __hwc2_list_for_each(pos, &dpy->layers) {
        __hwc2_layer_t *layer;
        layer = __hwc2_list_entry(pos, __hwc2_layer_t, link);
        layer->id = index++;

        __hwc2_trace_string("  [%d]=%p zorder=%u",
                layer->id, layer, layer->zorder);
    }
}

static void insert_layer(__hwc2_device_t *device,
                __hwc2_display_t *dpy, __hwc2_layer_t *layer)
{
    __hwc2_list_t *pos;
    __hwc2_layer_t *next = NULL;

    __hwc2_list_for_each(pos, &dpy->layers) {
        next = __hwc2_list_entry(pos, __hwc2_layer_t, link);

        if (next->zorder > layer->zorder)
            break;
    }

    if (next) {
        if (next->zorder > layer->zorder) {
            __hwc2_list_add_tail(&layer->link, &next->link);
        } else {
            __hwc2_list_add(&layer->link, &next->link);
        }
    } else {
        /* It is the only one. */
        __hwc2_list_add(&layer->link, &dpy->layers);
    }

    update_layer_index(device, dpy);
}

/* setLayerZOrder(..., z)
 * Descriptor: HWC2_FUNCTION_SET_LAYER_Z_ORDER
 * Must be provided by all HWC2 devices
 *
 * Sets the desired Z order (height) of the given layer. A layer with a greater
 * Z value occludes a layer with a lesser Z value.
 *
 * Parameters:
 *   z - the new Z order
 *
 * Returns HWC2_ERROR_NONE or one of the following errors:
 *   HWC2_ERROR_BAD_LAYER - an invalid layer handle was passed in
 */
/*
 * Default setLayerZOrder
 * Sets to __hwc2_layer_t::zorder and sort layers by z.
 */
static int32_t /*hwc2_error_t*/ set_layer_zorder(
        hwc2_device_t* device, hwc2_display_t display, hwc2_layer_t layer,
        uint32_t z)
{
    __hwc2_display_t *dpy = (__hwc2_display_t *)(uintptr_t)display;
    __hwc2_layer_t *ly = (__hwc2_layer_t *)(uintptr_t)layer;

    sanity_check_display(device, dpy);
    sanity_check_layer(dpy, ly);

    __hwc2_trace(2, "dpy=%p layer=%p(%u) zorder=%u -> %u",
            dpy, ly, ly->id, ly->zorder, z);

    if (ly->zorder == z) {
        __hwc2_trace_string("  Not changed");
        return HWC2_ERROR_NONE;
    }

    ly->zorder = z;

    /* Remove and insert again to keep it sorted. */
    __hwc2_list_del(&ly->link);
    insert_layer((__hwc2_device_t *)device, dpy, ly);

    /* Requires validation. */
    dpy->validation = HWC2_ERROR_NOT_VALIDATED;
    return HWC2_ERROR_NONE;
}

/******************************************************************************/

/* getCapabilities(..., outCount, outCapabilities)
 *
 * Provides a list of capabilities (described in the definition of
 * hwc2_capability_t above) supported by this device. This list must
 * not change after the device has been loaded.
 *
 * Parameters:
 *   outCount - if outCapabilities was NULL, the number of capabilities
 *       which would have been returned; if outCapabilities was not NULL,
 *       the number of capabilities returned, which must not exceed the
 *       value stored in outCount prior to the call
 *   outCapabilities - a list of capabilities supported by this device; may
 *       be NULL, in which case this function must write into outCount the
 *       number of capabilities which would have been written into
 *       outCapabilities
 */
void __hwc2_get_capabilities(hwc2_device_t* device, uint32_t* outCount,
        int32_t* /*hwc2_capability_t*/ outCapabilities)
{
    __hwc2_device_t *dev = (__hwc2_device_t *)device;

    __hwc2_trace(0, "device=%p outCapabilities=%p", dev, outCapabilities);

    if (!outCapabilities) {
        *outCount = dev->numCapabilities;
    } else {
        if (*outCount > dev->numCapabilities)
            *outCount = dev->numCapabilities;

        for (uint32_t i = 0; i < *outCount; i++) {
            __hwc2_trace(2, "out outCapabilities[%d]=%s(%i)", i,
                capability_name(dev->capabilities[i]),
                dev->capabilities[i]);

            outCapabilities[i] = dev->capabilities[i];
        }
    }

    __hwc2_trace(1, "out outCount=%d", *outCount);
}

static struct
{
    int32_t descriptor;
    hwc2_function_pointer_t function;
}
default_functions[] =
{
    /* Device Functions. */
    {HWC2_FUNCTION_CREATE_VIRTUAL_DISPLAY,
            (hwc2_function_pointer_t) create_virtual_display},
    {HWC2_FUNCTION_DESTROY_VIRTUAL_DISPLAY,
            (hwc2_function_pointer_t) destroy_virtual_display},
    {HWC2_FUNCTION_DUMP,
            (hwc2_function_pointer_t) dump},
    {HWC2_FUNCTION_GET_MAX_VIRTUAL_DISPLAY_COUNT,
            (hwc2_function_pointer_t) get_max_virtual_display_count},
    {HWC2_FUNCTION_REGISTER_CALLBACK,
            (hwc2_function_pointer_t) register_callback},

    /* Display Functions. */
    {HWC2_FUNCTION_ACCEPT_DISPLAY_CHANGES,
            (hwc2_function_pointer_t) accept_display_changes},
    {HWC2_FUNCTION_CREATE_LAYER,
            (hwc2_function_pointer_t) create_layer},
    {HWC2_FUNCTION_DESTROY_LAYER,
            (hwc2_function_pointer_t) destroy_layer},
    {HWC2_FUNCTION_GET_ACTIVE_CONFIG,
            (hwc2_function_pointer_t) get_active_config},
    {HWC2_FUNCTION_GET_CHANGED_COMPOSITION_TYPES,
            (hwc2_function_pointer_t) get_changed_composition_types},
    {HWC2_FUNCTION_GET_CLIENT_TARGET_SUPPORT,
            (hwc2_function_pointer_t) get_client_target_support},
    {HWC2_FUNCTION_GET_COLOR_MODES,
            (hwc2_function_pointer_t) get_color_modes},
    {HWC2_FUNCTION_GET_DISPLAY_ATTRIBUTE,
            (hwc2_function_pointer_t) get_display_attribute},
    {HWC2_FUNCTION_GET_DISPLAY_CONFIGS,
            (hwc2_function_pointer_t) get_display_configs},
    {HWC2_FUNCTION_GET_DISPLAY_NAME,
            (hwc2_function_pointer_t) get_display_name},
    {HWC2_FUNCTION_GET_DISPLAY_REQUESTS,
            (hwc2_function_pointer_t) get_display_requests},
    {HWC2_FUNCTION_GET_DISPLAY_TYPE,
            (hwc2_function_pointer_t) get_display_type},
    {HWC2_FUNCTION_GET_DOZE_SUPPORT,
            (hwc2_function_pointer_t) get_doze_support},
    {HWC2_FUNCTION_GET_HDR_CAPABILITIES,
            (hwc2_function_pointer_t) get_hdr_capabilities},
    {HWC2_FUNCTION_GET_RELEASE_FENCES,
            (hwc2_function_pointer_t) get_release_fences},
    {HWC2_FUNCTION_PRESENT_DISPLAY,
            (hwc2_function_pointer_t) present_display},
    {HWC2_FUNCTION_SET_ACTIVE_CONFIG,
            (hwc2_function_pointer_t) set_active_config},
    {HWC2_FUNCTION_SET_CLIENT_TARGET,
            (hwc2_function_pointer_t) set_client_target},
    {HWC2_FUNCTION_SET_COLOR_MODE,
            (hwc2_function_pointer_t) set_color_mode},
    {HWC2_FUNCTION_SET_COLOR_TRANSFORM,
            (hwc2_function_pointer_t) set_color_transform},
    {HWC2_FUNCTION_SET_OUTPUT_BUFFER,
            (hwc2_function_pointer_t) set_output_buffer},
    {HWC2_FUNCTION_SET_POWER_MODE,
            (hwc2_function_pointer_t) set_power_mode},
    {HWC2_FUNCTION_SET_VSYNC_ENABLED,
            (hwc2_function_pointer_t) set_vsync_enabled},
    {HWC2_FUNCTION_VALIDATE_DISPLAY,
            (hwc2_function_pointer_t) validate_display},

    /* Layer Functions */
    {HWC2_FUNCTION_SET_CURSOR_POSITION,
            (hwc2_function_pointer_t) set_cursor_position},
    {HWC2_FUNCTION_SET_LAYER_BUFFER,
            (hwc2_function_pointer_t) set_layer_buffer},
    {HWC2_FUNCTION_SET_LAYER_SURFACE_DAMAGE,
            (hwc2_function_pointer_t) set_layer_surface_damage},

    /* Layer State Functions */
    {HWC2_FUNCTION_SET_LAYER_BLEND_MODE,
            (hwc2_function_pointer_t) set_layer_blend_mode},
    {HWC2_FUNCTION_SET_LAYER_COLOR,
            (hwc2_function_pointer_t) set_layer_color},
    {HWC2_FUNCTION_SET_LAYER_COMPOSITION_TYPE,
            (hwc2_function_pointer_t) set_layer_composition_type},
    {HWC2_FUNCTION_SET_LAYER_DATASPACE,
            (hwc2_function_pointer_t) set_layer_dataspace},
    {HWC2_FUNCTION_SET_LAYER_DISPLAY_FRAME,
            (hwc2_function_pointer_t) set_layer_display_frame},
    {HWC2_FUNCTION_SET_LAYER_PLANE_ALPHA,
            (hwc2_function_pointer_t) set_layer_plane_alpha},
    {HWC2_FUNCTION_SET_LAYER_SIDEBAND_STREAM,
            (hwc2_function_pointer_t) set_layer_sideband_stream},
    {HWC2_FUNCTION_SET_LAYER_SOURCE_CROP,
            (hwc2_function_pointer_t) set_layer_source_crop},
    {HWC2_FUNCTION_SET_LAYER_TRANSFORM,
            (hwc2_function_pointer_t) set_layer_transform},
    {HWC2_FUNCTION_SET_LAYER_VISIBLE_REGION,
            (hwc2_function_pointer_t) set_layer_visible_region},
    {HWC2_FUNCTION_SET_LAYER_Z_ORDER,
            (hwc2_function_pointer_t) set_layer_zorder},
};

/* getFunction(..., descriptor)
 *
 * Returns a function pointer which implements the requested description.
 *
 * Parameters:
 *   descriptor - the function to return
 *
 * Returns either a function pointer implementing the requested descriptor
 *   or NULL if the described function is not supported by this device.
 */
hwc2_function_pointer_t __hwc2_get_function(hwc2_device_t* device,
                            int32_t /*hwc2_function_descriptor_t*/ descriptor)
{
    __hwc2_trace(0, "device=%p descriptor=%s(%d)", device,
            function_descriptor_name(descriptor), descriptor);

    for (size_t i = 0; i < ARRAY_SIZE(default_functions); i++) {
        if (default_functions[i].descriptor == descriptor) {
            __hwc2_trace(1, "return=%p", default_functions[i].function);
            return default_functions[i].function;
        }
    }

    __hwc2_trace(1, "return=NULL");
    return (hwc2_function_pointer_t) NULL;
}

/******************************************************************************/
/* HWC2 HAL Framework and Helper functions. */

uint32_t __hwc2_traceEnabled = 0;
__thread int __hwc2_traceDepth = 0;

int32_t __hwc2_init_device(__hwc2_device_t *device)
{
    __hwc2_trace(0, "device=%p", device);

    __hwc2_list_init(&device->displays);
    device->numDisplays = 0;

    /* Initialize blitter. */
    blitter_init(device);

    __hwc2_trace(1, "OK");
    return 0;
}

void __hwc2_close_device(__hwc2_device_t *device)
{
    __hwc2_trace(2, "device=%p", device);

    /* Finalize blitter. */
    blitter_close(device);
}

__hwc2_display_t * __hwc2_alloc_display(__hwc2_device_t *device,
                        uint32_t width, uint32_t height, int32_t format,
                        int32_t type, size_t size)
{
    __hwc2_display_t *dpy;
    __hwc2_trace(0, "device=%p width=%u height=%u format=%i type=%i size=%zu",
            device, width, height, format, type, size);

    if (!size)
        size = sizeof(__hwc2_display_t);

    dpy = (__hwc2_display_t *)malloc(size);
    if (unlikely(!dpy)) {
        /* out of memory. */
        __hwc2_trace(1, "ENOMEM");
        return NULL;
    }

    /* Zero memory. */
    memset(dpy, 0, size);

    /* Reference parent device. */
    dpy->device = device;

    /* build a default name. */
    snprintf(dpy->name, sizeof(dpy->name),
        "dpy-%ux%u", width, height);

    dpy->id        = 0;
    dpy->type      = type;
    dpy->connected = 0;

    dpy->dozeSupport  = 0;

    dpy->hdrTypes[0]  = 0;
    dpy->numHdrTypes  = 0;
    dpy->maxLuminance        = 1.0f;
    dpy->maxAverageLuminance = 1.0f;
    dpy->minLuminance        = 0.0f;

    dpy->configs[0] = NULL;
    dpy->numConfigs = 0;

    dpy->colorModes[0] = 0;
    dpy->numColorModes = 0;

    dpy->width     = width;
    dpy->height    = height;
    dpy->format    = format;
    dpy->dataspace = HAL_DATASPACE_UNKNOWN;

    dpy->validation = HWC2_ERROR_NOT_VALIDATED;

    dpy->clientTarget          = NULL;
    dpy->clientAcquireFence    = -1;
    dpy->clientDataspace       = HAL_DATASPACE_UNKNOWN;
    dpy->clientDamage.numRects = 0;
    dpy->clientDamage.rects    = NULL;

    dpy->outputBuffer       = NULL;
    dpy->outputReleaseFence = -1;

    dpy->activeConfig   = NULL;

    dpy->colorMode      = 0;

    const float identity_matrix[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };

    memcpy(dpy->colorTransform, identity_matrix, sizeof(float) * 16);
    dpy->colorTransformHint = HAL_COLOR_TRANSFORM_IDENTITY;

    dpy->powerMode = HWC2_POWER_MODE_OFF;

    __hwc2_list_init(&dpy->layers);
    __hwc2_list_init(&dpy->layersChanged);

    __hwc2_list_add_tail(&dpy->link, &device->displays);

    /* Initialize blitter. */
    blitter_init_display(dpy);

    __hwc2_trace(1, "return=%p", dpy);
    return dpy;
}

void __hwc2_free_display(__hwc2_device_t *device, __hwc2_display_t *dpy)
{
    __hwc2_trace(0, "dpy=%p", dpy);

    /* Finalize blitter. */
    blitter_close_display(dpy);

    __hwc2_list_del(&dpy->link);
    free(dpy);

    __hwc2_trace(1, "done");
}

__hwc2_layer_t * __hwc2_alloc_layer(__hwc2_device_t *device,
                        __hwc2_display_t *dpy, size_t size)
{
    __hwc2_layer_t *layer;
    __hwc2_trace(0, "dpy=%p size=%zu", dpy, size);

    if (!size)
        size = sizeof(__hwc2_layer_t);

    layer = (__hwc2_layer_t *)malloc(size);
    if (unlikely(!layer)) {
        __hwc2_trace(1, "ENOMEM");
        return NULL;
    }

    memset(layer, 0, size);
    layer->acquireFence = -1;
    layer->releaseFence = -1;

    layer->composition = HWC2_COMPOSITION_CLIENT;
    layer->blendMode   = HWC2_BLEND_MODE_NONE;
    layer->planeAlpha  = 1.0f;

    layer->geometryChanged = 1;
    layer->blendChanged    = 1;

    layer->compositionChanged = HWC2_COMPOSITION_CLIENT;

    blitter_init_layer(dpy, layer);

    /* Insert and sort layer. */
    insert_layer(device, dpy, layer);

    /* Requires display validation. */
    dpy->validation = HWC2_ERROR_NOT_VALIDATED;

    __hwc2_trace(1, "return=%p", layer);
    return layer;
}

void __hwc2_free_layer(__hwc2_device_t *device, __hwc2_display_t *dpy,
            __hwc2_layer_t *layer)
{
    __hwc2_trace(0, "layer=%p(%u)", layer, layer->id);

    __hwc2_list_del(&layer->link);

    if (layer->visibleRegion.rects)
        free((void *)layer->visibleRegion.rects);

    if (layer->surfaceDamage.rects)
        free((void *)layer->surfaceDamage.rects);

    blitter_fini_layer(dpy, layer);

    free(layer);

    /* Requires display validation. */
    dpy->validation = HWC2_ERROR_NOT_VALIDATED;

    update_layer_index(device, dpy);

    __hwc2_trace(1, "done");
}

__hwc2_config_t * __hwc2_alloc_config(__hwc2_device_t *device,
                        __hwc2_display_t *dpy, size_t size)
{
    __hwc2_config_t *config;
    uint32_t i;

    __hwc2_trace(0, "dpy=%p size=%zu", dpy, size);

    if (!size)
        size = sizeof(__hwc2_config_t);

    config = (__hwc2_config_t *)malloc(size);
    if (unlikely(!config)) {
        __hwc2_trace(1, "ENOMEM");
        return NULL;
    }

    memset(config, 0, size);

    /* Reocrd into display, find a empty slot. */
    for (i = 0; i < ARRAY_SIZE(dpy->configs); i++) {
        if (!dpy->configs[i]) {
            dpy->configs[i] = config;
            /* Value of hwc2_config_t equals to (INDEX + 1). */
            config->config = (hwc2_config_t)(i + 1);
            break;
        }
    }

    /* Increase config count. */
    dpy->numConfigs++;

    if (unlikely(i >= ARRAY_SIZE(dpy->configs))) {
        LOG_ALWAYS_FATAL("%s(%d): too many configs", __FUNCTION__, __LINE__);
    }

    __hwc2_trace(1, "return=%p(%u)", config, config->config);
    return config;
}

void __hwc2_free_config(__hwc2_device_t *device, __hwc2_display_t *dpy,
            __hwc2_config_t *config)
{
    __hwc2_trace(0, "config=%p(%u)", config, config->config);

    sanity_check_config(dpy, config);

    dpy->configs[config->config - 1] = NULL;

    if (dpy->activeConfig == config) {
        dpy->activeConfig = NULL;
    }

    free(config);

    __hwc2_trace(1, "done");
}

int32_t __hwc2_register_callback(__hwc2_device_t *device,
                int32_t /*hwc2_callback_descriptor_t*/ descriptor,
                hwc2_callback_data_t callbackData,
                hwc2_function_pointer_t pointer)
{
    __hwc2_trace(0, "device=%p descritpor=%s(%d) callbackData=%p pointer=%p",
            device, callback_descriptor_name(descriptor),
            descriptor, callbackData, pointer);

    switch (descriptor) {
        case HWC2_CALLBACK_HOTPLUG:
            device->hotplug = (HWC2_PFN_HOTPLUG)pointer;
            device->hotplugData = callbackData;
            break;
        case HWC2_CALLBACK_REFRESH:
            device->refresh = (HWC2_PFN_REFRESH)pointer;
            device->refreshData = callbackData;
            break;
        case HWC2_CALLBACK_VSYNC:
            device->vsync = (HWC2_PFN_VSYNC)pointer;
            device->vsyncData = callbackData;
            break;
        default:
            __hwc2_trace_error(1, HWC2_ERROR_BAD_PARAMETER);
            return HWC2_ERROR_BAD_PARAMETER;
    }

    __hwc2_trace(1, "OK");
    return HWC2_ERROR_NONE;
}

void __hwc2_frame_end(__hwc2_device_t *device, __hwc2_display_t *dpy)
{
    __hwc2_list_t *pos;

    __hwc2_list_for_each(pos, &dpy->layers) {
        __hwc2_layer_t *layer;
        layer = __hwc2_list_entry(pos, __hwc2_layer_t, link);

        layer->geometryChanged = 0;
        layer->blendChanged    = 0;
        layer->colorChanged    = 0;
        layer->visibleRegionChanged = 0;
    }

    /* Ends validate/present sequence. */
    dpy->lockCursor = 0;
}

int32_t __hwc2_queue_post_display(__hwc2_device_t *device,
            __hwc2_display_t *dpy, int32_t acquireFence,
            __HWC2_PFN_POST_DISPLAY postDisplay, void *param)
{
    int32_t ret;
    __hwc2_trace(0, "acquireFence=%d postDisplay=%p param=%p",
            acquireFence, postDisplay, param);

    /* Currently wait and do present in this thread. */
    if (acquireFence != -1) {
        sync_wait(acquireFence, -1);
        close(acquireFence);
    }

    ret = postDisplay(device, dpy, param);

    __hwc2_trace(1, "return=%d", ret);
    return ret;
}

