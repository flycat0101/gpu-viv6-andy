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


#ifndef __hwc2_util_h_
#define __hwc2_util_h_

#include <hardware/hwcomposer_defs.h>
#include <stdlib.h>

/* Options. */
#define __HWC2_TRACE


/* Calc array size. */
#define ARRAY_SIZE(x)   (sizeof(x)/sizeof(x[0]))


/* Condition optimization. */
#define likely(x)       __builtin_expect(!!(x),1)
#define unlikely(x)     __builtin_expect((x),0)


/* Container-of macro. */
#define container_of(ptr, type, member) \
    ((type *)((uintptr_t)(ptr) - offsetof(type, member)))


/* Linked list functions. */
typedef struct __hwc2_list __hwc2_list_t;
struct __hwc2_list
{
    struct __hwc2_list *prev;
    struct __hwc2_list *next;
};

#define __HWC2_LIST_INIT(name)  { &(name), &(name) }

#define __HWC2_DEFINE_LIST(name) \
    __hwc2_list_t name = __HWC2_LIST_INIT(name)

#define __hwc2_list_entry(ptr, type, member) \
    container_of(ptr, type, member)

#define __hwc2_list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

#define __hwc2_list_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); \
        pos = n, n = pos->next)

/*
 * Init a list head.
 */
static inline void __hwc2_list_init(__hwc2_list_t *head)
{
    head->prev = head;
    head->next = head;
}

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void ___hwc2_list_add(__hwc2_list_t *_new,
                        __hwc2_list_t *prev,
                        __hwc2_list_t *next)
{
    next->prev = _new;
    _new->next = next;
    _new->prev = prev;
    prev->next = _new;
}

/*
 * __hwc2_list_add - add a new entry
 * @_new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 */
static inline void __hwc2_list_add(__hwc2_list_t *_new,
                        __hwc2_list_t *head)
{
    ___hwc2_list_add(_new, head, head->next);
}

/*
 * __hwc2_list_add_tail - add a new entry
 * @_new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 */
static inline void __hwc2_list_add_tail(__hwc2_list_t *_new,
                        __hwc2_list_t *head)
{
    ___hwc2_list_add(_new, head->prev, head);
}

/*
 * __hwc2_list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: __hwc2_list_empty() on entry does not return true after this, the entry
 * is in an undefined state.
 */
static inline void __hwc2_list_del(__hwc2_list_t *entry)
{
    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;

    entry->prev = NULL;
    entry->next = NULL;
}


/*
 * __hwc2_list_empty - tests whether a list is empty.
 */
static inline int __hwc2_list_empty(const __hwc2_list_t *head)
{
    return head->next == head;
}

/*
 * __hwc2_list_size - calulate list element count.
 */
static inline int __hwc2_list_size(const __hwc2_list_t *head)
{
    int num = 0;
    __hwc2_list_t *pos;

    __hwc2_list_for_each(pos, head) {
        num++;
    }
    return num;
}

/*
 * __hwc2_list_is_first - tests whether list is the first entry in list head
 */
static inline int __hwc2_list_is_first(const __hwc2_list_t *list,
                        const __hwc2_list_t *head)
{
    return (head->next == list);
}

/*
 * __hwc2_list_is_last - tests whether list is the last entry in list head
 */
static inline int __hwc2_list_is_last(const __hwc2_list_t *list,
                        const __hwc2_list_t *head)
{
    return (list->next == head);
}

/*
 * __hwc2_list_move - delete from one list and add as another's head
 */
static inline void __hwc2_list_move(__hwc2_list_t *list,
                        __hwc2_list_t *head)
{
    __hwc2_list_del(list);
    __hwc2_list_add(list, head);
}

/*
 * __hwc2_list_move_tail - delete from one list and add as another's head
 */
static inline void __hwc2_list_move_tail(__hwc2_list_t *list,
                        __hwc2_list_t *head)
{
    __hwc2_list_del(list);
    __hwc2_list_add_tail(list, head);
}

static inline void ___hwc2_list_splice(const __hwc2_list_t *list,
                        __hwc2_list_t *prev, __hwc2_list_t *next)
{
    __hwc2_list_t *first = list->next;
    __hwc2_list_t *last = list->prev;

    first->prev = prev;
    prev->next = first;

    last->next = next;
    next->prev = last;
}

/*
 * __hwc2_list_splice - join two lists.
 */
static inline void __hwc2_list_splice(const __hwc2_list_t *list,
                        __hwc2_list_t *head)
{
    if (!__hwc2_list_empty(list))
        ___hwc2_list_splice(list, head, head->next);
}

/*
 * __hwc2_list_splice_tail - join two lists, each list being a queue
 */
static inline void __hwc2_list_splice_tail(__hwc2_list_t *list,
                        __hwc2_list_t *head)
{
    if (!__hwc2_list_empty(list))
        ___hwc2_list_splice(list, head->prev, head);
}

/*
 * __hwc2_list_splice_init - join two lists and reinitialise the emptied list.
 *
 * The list 'list' is reinitialised
 */
static inline void __hwc2_list_splice_init(__hwc2_list_t *list,
                        __hwc2_list_t *head)
{
    if (!__hwc2_list_empty(list)) {
        ___hwc2_list_splice(list, head, head->next);
        __hwc2_list_init(list);
    }
}

/*
 * __hwc2_list_splice_tail_init - join two lists and reinitialise the emptied list
 *
 * The list 'list' is reinitialised
 */
static inline void __hwc2_list_splice_tail_init(__hwc2_list_t *list,
                        __hwc2_list_t *head)
{
    if (!__hwc2_list_empty(list)) {
        ___hwc2_list_splice(list, head->prev, head);
        __hwc2_list_init(list);
    }
}


/* Region functions. */
/*
 * __hwc2_region_copy - copy region to region to.
 */
static inline void __hwc2_region_copy(hwc_region_t *to,
                        const hwc_region_t from)
{
    if (unlikely(from.numRects == 0)) {
        to->numRects = 0;
        return;
    }

    if (to->numRects < from.numRects) {
        if (likely(to->rects)) {
            void *ptr = (void *)to->rects;
            to->rects = (hwc_rect_t *)realloc(
                    ptr, sizeof(hwc_rect_t) * from.numRects);
        } else {
            to->rects = (hwc_rect_t *)malloc(
                    sizeof(hwc_rect_t) * from.numRects);
        }
    }

    memcpy((void *)to->rects, from.rects, sizeof(hwc_rect_t) * from.numRects);
    to->numRects = from.numRects;
}

/*
 * __hwc2_region_compare - compare two regions.
 * returns 0 if the same, non-zero if different.
 */
static inline int __hwc2_region_compare(const hwc_region_t r1,
                        const hwc_region_t r2)
{
    if (r1.numRects != r2.numRects) {
        return 1;
    }

    return memcmp(r1.rects, r2.rects, sizeof(hwc_rect_t) * r1.numRects);
}

/*
 * __hwc2_region_is_empty - check if the region is empty.
 * returns 1 if empty, empty only when numRects == 1 and rects[0] is [0,0,0,0].
 */
static inline int __hwc2_region_is_empty(const hwc_region_t r)
{
    return (r.numRects == 1 &&
        r.rects[0].left == 0 && r.rects[0].right == 0 &&
        r.rects[0].top  == 0 && r.rects[0].bottom == 0);
}


/* Enum value string aliases. */
#define attribute_name(x)           getAttributeName(hwc2_attribute_t(x))
#define blend_mode_name(x)          getBlendModeName(hwc2_blend_mode_t(x))
#define callback_descriptor_name(x) getCallbackDescriptorName(hwc2_callback_descriptor_t(x))
#define capability_name(x)          getCapabilityName(hwc2_capability_t(x))
#define composition_name(x)         getCompositionName(hwc2_composition_t(x))
#define connection_name(x)          getConnectionName(hwc2_connection_t(x))
#define display_request_name(x)     getDisplayRequestName(hwc2_display_request_t(x))
#define display_type_name(x)        getDisplayTypeName(hwc2_display_type_t(x))
#define error_name(x)               getErrorName(hwc2_error_t(x))
#define function_descriptor_name(x) getFunctionDescriptorName(hwc2_function_descriptor_t(x))
#define layer_request_name(x)       getLayerRequestName(hwc2_layer_request_t(x))
#define power_mode_name(x)          getPowerModeName(hwc2_power_mode_t(x))
#define transform_name(x)           getTransformName(hwc_transform_t(x))
#define vsync_name(x)               getVsyncName(hwc2_vsync_t(x))


/* Trace functions. */
extern uint32_t __hwc2_traceEnabled;
extern __thread int __hwc2_traceDepth;

#ifdef __HWC2_TRACE

#define TRACE_IN    0
#define TRACE_OUT   1
#define TRACE_OTHER 2

#  define __hwc2_trace_init() \
    do { \
        char property[PROPERTY_VALUE_MAX]; \
        if (property_get("hwc2.trace", property, NULL) > 0) { \
            __hwc2_traceEnabled = property[0] != '0'; \
        } \
    } while (0)

#  define __hwc2_trace(inout, format, ...) \
    do { \
        const char *___s; \
        if (likely(!__hwc2_traceEnabled)) \
            break; \
        if (inout == 0) { \
            ___s = "+ "; \
        } else if (inout == 1) { \
            ___s = "- "; \
            __hwc2_traceDepth--; \
        } else { \
            ___s = "  "; \
        } \
        ALOGD("%*s%s%s#%d: " format, __hwc2_traceDepth * 2, "", \
             ___s, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        if (inout == 0) { \
            __hwc2_traceDepth++; \
        } \
    } while (0)

#  define __hwc2_trace_error(inout, err) \
    __hwc2_trace(inout, "error=%s(%d)", error_name(err), err)

#  define __hwc2_trace_region(region) \
    do { \
        if (likely(!__hwc2_traceEnabled)) \
            break; \
        if (unlikely((region).numRects == 0)) { \
            ALOGD("%*s   (whole-surface)", __hwc2_traceDepth * 2, ""); \
        } else { \
            for (size_t __i = 0; __i < (region).numRects; __i++) { \
                const hwc_rect_t * __rect = &(region).rects[__i]; \
                ALOGD("%*s   [%d]: [%d,%d,%d,%d]", \
                    __hwc2_traceDepth * 2, "", \
                    __i, __rect->left, __rect->top, __rect->right, __rect->bottom); \
            } \
        } \
    } while (0)

#  define __hwc2_trace_string(format, ...) \
    do { \
        if (likely(!__hwc2_traceEnabled)) \
            break; \
        ALOGD("%*s  " format, __hwc2_traceDepth * 2, "", ##__VA_ARGS__); \
    } while (0)

#else

#  define __hwc2_trace_init() \
    do {} while (0)

#  define __hwc2_trace(inout, format, ...) \
    do {} while (0)

#  define __hwc2_trace_error(inout, err) \
    do {} while (0)

#  define __hwc2_trace_region(region) \
    do {} while (0)

#  define __hwc2_trace_string(...) \
    do {} while (0)

#endif /* __HWC2_TRACE */

/* Sanity checks. */
// #define __HWC2_SANITY_CHECK

/******************************************************************************/

/*
 * sanity_check_display
 * validate display. Return BAD_DISPLAY if invalid.
 */
#ifdef __HWC2_SANITY_CHECK
#  define sanity_check_display(device, dpy) \
    do { \
        __hwc2_device_t *dev = (__hwc2_device_t *)device; \
        __hwc2_list_t *pos; \
     \
        __hwc2_list_for_each(pos, &dev->displays) { \
            if (pos == &(dpy)->link) \
                break; \
        } \
     \
        if (unlikely(pos != &(dpy)->link)) { \
            ALOGE("%s: BAD_DISPLAY", __FUNCTION__); \
            return HWC2_ERROR_BAD_DISPLAY; \
        } \
    } while (0)
#else
#  define sanity_check_display(device, dpy) \
    do {(void)(dpy);} while (0)
#endif

/*
 * sanity_check_layer
 * validate layer. Return BAD_LAYER if invalid.
 */
#ifdef __HWC2_SANITY_CHECK
#  define sanity_check_layer(dpy, layer) \
    do { \
        __hwc2_list_t *pos; \
     \
        __hwc2_list_for_each(pos, &(dpy)->layers) { \
            if (pos == &(layer)->link) \
                break; \
        } \
     \
        if (unlikely(pos != &(layer)->link)) { \
            ALOGE("%s: BAD_LAYER", __FUNCTION__); \
            return HWC2_ERROR_BAD_LAYER; \
        } \
    } while (0)
#else
#  define sanity_check_layer(dpy, layer) \
    do {(void)(dpy); (void)(layer);} while (0)
#endif

/*
 * sanity_check_buffer
 * validate buffer. Return BAD_PARAMETER if invalid.
 */
#if defined(__HWC2_SANITY_CHECK) && defined(__cplusplus)
#include <gralloc_priv.h>
#  define sanity_check_buffer(x) \
    do { \
        if ((x) != NULL && private_handle_t::validate((native_handle *)(x))) \
            return -HWC2_ERROR_BAD_PARAMETER; \
    } while(0)
#else
#  define sanity_check_buffer(x) \
    do {(void)(x);} while (0)
#endif

/*
 * sanity_check_config
 */
#ifdef __HWC2_SANITY_CHECK
#  define sanity_check_config(dpy, cfg) \
    do { \
        if ((dpy)->configs[(cfg)->config - 1] != (cfg)) { \
            ALOGE("%s(%d): invalid config=%p(%u)", \
                __FUNCTION__, __LINE__, (cfg), (uint32_t)((cfg)->config)); \
        } \
    } while (0)

#else
#  define sanity_check_config(dpy, cfg) \
    do {(void)(cfg);} while (0)
#endif

#endif /* __hwc2_util_h_ */

