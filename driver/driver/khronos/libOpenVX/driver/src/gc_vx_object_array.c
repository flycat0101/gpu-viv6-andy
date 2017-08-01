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


#include <gc_vx_common.h>

VX_INTERNAL_CALLBACK_API vx_bool vxoOA_IsValidObjectArray(vx_object_array array)
{
    vx_size i = 0u;
    if (array == NULL ||
        !vxoReference_IsValidAndSpecific(&array->base, VX_TYPE_OBJECT_ARRAY))
        return vx_false_e;

    if (array->itemCount > VX_MAX_REF_COUNT)
        return vx_false_e;

    for (i = 0u; i < array->itemCount; i++)
    {
        if (!vxoReference_IsValidAndSpecific(array->itemsTable[i], (vx_type_e)array->itemType))
            return vx_false_e;
    }

    return vx_true_e;
}

VX_INTERNAL_CALLBACK_API vx_status vxoOA_InitObjectArrayInt(vx_object_array arr, vx_reference exemplar, vx_size num_items)
{
    vx_status status = VX_SUCCESS;

    vx_bool is_virtual = arr->base.isVirtual;
    vx_enum item_type = exemplar->type;

    vx_uint32 image_width = 0, image_height = 0;
    vx_df_image image_format = VX_DF_IMAGE_U8;
    vx_size array_capacity = 0;
    vx_enum array_itemtype = VX_TYPE_KEYPOINT;
    vx_uint32 pyramid_width = 0, pyramid_height = 0;
    vx_df_image pyramid_format = VX_DF_IMAGE_U8;
    vx_size pyramid_levels = 0;
    vx_float32 pyramid_scale = .0f;
    vx_enum scalar_type = VX_TYPE_UINT8;
    vx_enum matrix_type = VX_TYPE_UINT8;
    vx_size matrix_rows = 0, matrix_cols = 0;
    vx_size distribution_bins = 0;
    vx_int32 distribution_offset = 0;
    vx_uint32 distribution_range = 0;
    vx_uint32 remap_srcwidth = 0, remap_srcheight = 0;
    vx_uint32 remap_dstwidth = 0, remap_dstheight = 0;
    vx_enum lut_type = VX_TYPE_UINT8;
    vx_size lut_count = 0;
    vx_enum threshold_type = VX_TYPE_UINT8, threshold_data_type = VX_TYPE_UINT8;

    if (is_virtual)
    {
        vx_graph graph = (vx_graph)arr->base.scope;
        vx_uint32 i = 0u;
        switch (item_type)
        {
            case VX_TYPE_IMAGE:
                if (vxQueryImage((vx_image)exemplar, VX_IMAGE_WIDTH, &image_width, sizeof(image_width)) != VX_SUCCESS ||
                    vxQueryImage((vx_image)exemplar, VX_IMAGE_HEIGHT, &image_height, sizeof(image_height)) != VX_SUCCESS ||
                    vxQueryImage((vx_image)exemplar, VX_IMAGE_FORMAT, &image_format, sizeof(image_format)) != VX_SUCCESS)
                    status = VX_ERROR_INVALID_REFERENCE;
                break;
            case VX_TYPE_ARRAY:
                if (vxQueryArray((vx_array)exemplar, VX_ARRAY_CAPACITY, &array_capacity, sizeof(array_capacity)) != VX_SUCCESS ||
                    vxQueryArray((vx_array)exemplar, VX_ARRAY_ITEMTYPE, &array_itemtype, sizeof(array_itemtype)) != VX_SUCCESS)
                    status = VX_ERROR_INVALID_REFERENCE;
                break;
            case VX_TYPE_PYRAMID:
                if (vxQueryPyramid((vx_pyramid)exemplar, VX_PYRAMID_WIDTH, &pyramid_width, sizeof(pyramid_width)) != VX_SUCCESS ||
                    vxQueryPyramid((vx_pyramid)exemplar, VX_PYRAMID_HEIGHT, &pyramid_height, sizeof(pyramid_height)) != VX_SUCCESS ||
                    vxQueryPyramid((vx_pyramid)exemplar, VX_PYRAMID_FORMAT, &pyramid_format, sizeof(pyramid_format)) != VX_SUCCESS ||
                    vxQueryPyramid((vx_pyramid)exemplar, VX_PYRAMID_LEVELS, &pyramid_levels, sizeof(pyramid_levels)) != VX_SUCCESS ||
                    vxQueryPyramid((vx_pyramid)exemplar, VX_PYRAMID_SCALE, &pyramid_scale, sizeof(pyramid_scale)) != VX_SUCCESS)
                    status = VX_ERROR_INVALID_REFERENCE;
                break;
            default:
                status =  VX_ERROR_INVALID_TYPE;
                break;
        }

        if (status != VX_SUCCESS)
            return status;

        for (i = 0u; i < num_items; i++)
        {
            vx_reference ref = NULL;

            switch (item_type)
            {
                case VX_TYPE_IMAGE:
                    ref = (vx_reference)vxCreateVirtualImage(graph, image_width, image_height, image_format);
                    break;
                case VX_TYPE_ARRAY:
                    ref = (vx_reference)vxCreateVirtualArray(graph, array_itemtype, array_capacity);
                    break;
                case VX_TYPE_PYRAMID:
                    ref = (vx_reference)vxCreateVirtualPyramid(graph, pyramid_levels, pyramid_scale, pyramid_width, pyramid_height, pyramid_format);
                    break;
                default:
                    ref = NULL;
                    break;
            }

            if (vxoReference_IsValidAndSpecific(ref, (vx_type_e)item_type))
            {
                arr->itemsTable[i] = ref;
                /* set the scope of the reference to the object array */
                ref->scope = (vx_reference)arr;
            }
            else
            {
                vx_uint32 j = 0u;
                /* free all allocated references */
                for (j = 0u; j < i; j++)
                {
                    vxoReference_Release(&(arr->itemsTable[j]), (vx_type_e)item_type, VX_REF_EXTERNAL);
                }

                return VX_ERROR_NO_RESOURCES;
            }
        }
    }
    else
    {
        vx_context context = (vx_context)arr->base.scope;
        vx_uint32 i = 0u;
        switch (item_type)
        {
            case VX_TYPE_IMAGE:
                if (vxQueryImage((vx_image)exemplar, VX_IMAGE_WIDTH, &image_width, sizeof(image_width)) != VX_SUCCESS ||
                    vxQueryImage((vx_image)exemplar, VX_IMAGE_HEIGHT, &image_height, sizeof(image_height)) != VX_SUCCESS ||
                    vxQueryImage((vx_image)exemplar, VX_IMAGE_FORMAT, &image_format, sizeof(image_format)) != VX_SUCCESS)
                    status = VX_ERROR_INVALID_REFERENCE;
                break;
            case VX_TYPE_ARRAY:
                if (vxQueryArray((vx_array)exemplar, VX_ARRAY_CAPACITY, &array_capacity, sizeof(array_capacity)) != VX_SUCCESS ||
                    vxQueryArray((vx_array)exemplar, VX_ARRAY_ITEMTYPE, &array_itemtype, sizeof(array_itemtype)) != VX_SUCCESS)
                    status = VX_ERROR_INVALID_REFERENCE;
                break;
            case VX_TYPE_PYRAMID:
                if (vxQueryPyramid((vx_pyramid)exemplar, VX_PYRAMID_WIDTH, &pyramid_width, sizeof(pyramid_width)) != VX_SUCCESS ||
                    vxQueryPyramid((vx_pyramid)exemplar, VX_PYRAMID_HEIGHT, &pyramid_height, sizeof(pyramid_height)) != VX_SUCCESS ||
                    vxQueryPyramid((vx_pyramid)exemplar, VX_PYRAMID_FORMAT, &pyramid_format, sizeof(pyramid_format)) != VX_SUCCESS ||
                    vxQueryPyramid((vx_pyramid)exemplar, VX_PYRAMID_LEVELS, &pyramid_levels, sizeof(pyramid_levels)) != VX_SUCCESS ||
                    vxQueryPyramid((vx_pyramid)exemplar, VX_PYRAMID_SCALE, &pyramid_scale, sizeof(pyramid_scale)) != VX_SUCCESS)
                    status = VX_ERROR_INVALID_REFERENCE;
                break;
            case VX_TYPE_SCALAR:
                if (vxQueryScalar((vx_scalar)exemplar, VX_SCALAR_TYPE, &scalar_type, sizeof(scalar_type)) != VX_SUCCESS)
                    status = VX_ERROR_INVALID_REFERENCE;
                break;
            case VX_TYPE_MATRIX:
                if (vxQueryMatrix((vx_matrix)exemplar, VX_MATRIX_TYPE, &matrix_type, sizeof(matrix_type)) != VX_SUCCESS ||
                    vxQueryMatrix((vx_matrix)exemplar, VX_MATRIX_ROWS, &matrix_rows, sizeof(matrix_rows)) != VX_SUCCESS ||
                    vxQueryMatrix((vx_matrix)exemplar, VX_MATRIX_COLUMNS, &matrix_cols, sizeof(matrix_cols)) != VX_SUCCESS)
                    status = VX_ERROR_INVALID_REFERENCE;
                break;
            case VX_TYPE_DISTRIBUTION:
                if (vxQueryDistribution((vx_distribution)exemplar, VX_DISTRIBUTION_BINS, &distribution_bins, sizeof(distribution_bins)) != VX_SUCCESS ||
                    vxQueryDistribution((vx_distribution)exemplar, VX_DISTRIBUTION_OFFSET, &distribution_offset, sizeof(distribution_offset)) != VX_SUCCESS ||
                    vxQueryDistribution((vx_distribution)exemplar, VX_DISTRIBUTION_RANGE, &distribution_range, sizeof(distribution_range)) != VX_SUCCESS)
                    status = VX_ERROR_INVALID_REFERENCE;
                break;
            case VX_TYPE_REMAP:
                if (vxQueryRemap((vx_remap)exemplar, VX_REMAP_SOURCE_WIDTH, &remap_srcwidth, sizeof(remap_srcwidth)) != VX_SUCCESS ||
                    vxQueryRemap((vx_remap)exemplar, VX_REMAP_SOURCE_HEIGHT, &remap_srcheight, sizeof(remap_srcheight)) != VX_SUCCESS ||
                    vxQueryRemap((vx_remap)exemplar, VX_REMAP_DESTINATION_WIDTH, &remap_dstwidth, sizeof(remap_dstwidth)) != VX_SUCCESS ||
                    vxQueryRemap((vx_remap)exemplar, VX_REMAP_DESTINATION_HEIGHT, &remap_dstheight, sizeof(remap_dstheight)) != VX_SUCCESS)
                    status = VX_ERROR_INVALID_REFERENCE;
                break;
            case VX_TYPE_LUT:
                if (vxQueryLUT((vx_lut)exemplar, VX_LUT_TYPE, &lut_type, sizeof(lut_type)) != VX_SUCCESS ||
                    vxQueryLUT((vx_lut)exemplar, VX_LUT_COUNT, &lut_count, sizeof(lut_count)) != VX_SUCCESS)
                    status = VX_ERROR_INVALID_REFERENCE;
                break;
            case VX_TYPE_THRESHOLD:
                if (vxQueryThreshold((vx_threshold)exemplar, VX_THRESHOLD_TYPE, &threshold_type, sizeof(threshold_type)) != VX_SUCCESS ||
                    vxQueryThreshold((vx_threshold)exemplar, VX_THRESHOLD_DATA_TYPE, &threshold_data_type, sizeof(threshold_data_type)) != VX_SUCCESS)
                    status = VX_ERROR_INVALID_REFERENCE;
                break;
            default:
                status =  VX_ERROR_INVALID_TYPE;
                break;
        }

        if (status != VX_SUCCESS)
            return status;

        for (i = 0u; i < num_items; i++)
        {
            vx_reference ref = NULL;

            switch (item_type)
            {
                case VX_TYPE_IMAGE:
                    ref = (vx_reference)vxCreateImage(context, image_width, image_height, image_format);
                    break;
                case VX_TYPE_ARRAY:
                    ref = (vx_reference)vxCreateArray(context, array_itemtype, array_capacity);
                    break;
                case VX_TYPE_PYRAMID:
                    ref = (vx_reference)vxCreatePyramid(context, pyramid_levels, pyramid_scale, pyramid_width, pyramid_height, pyramid_format);
                    break;
                case VX_TYPE_SCALAR:
                    ref = (vx_reference)vxCreateScalar(context, scalar_type, NULL);
                    break;
                case VX_TYPE_MATRIX:
                    ref = (vx_reference)vxCreateMatrix(context, matrix_type, matrix_cols, matrix_rows);
                    break;
                case VX_TYPE_DISTRIBUTION:
                    ref = (vx_reference)vxCreateDistribution(context, distribution_bins, distribution_offset, distribution_range);
                    break;
                case VX_TYPE_REMAP:
                    ref = (vx_reference)vxCreateRemap(context, remap_srcwidth, remap_srcheight, remap_dstwidth, remap_dstheight);
                    break;
                case VX_TYPE_LUT:
                    ref = (vx_reference)vxCreateLUT(context, lut_type, lut_count);
                    break;
                case VX_TYPE_THRESHOLD:
                    ref = (vx_reference)vxCreateThreshold(context, threshold_type, threshold_data_type);
                    break;
                default:
                    ref = NULL;
                    break;
            }

            if (vxoReference_IsValidAndSpecific(ref, (vx_type_e)item_type))
            {
                arr->itemsTable[i] = ref;
                /* set the scope of the reference to the object array */
                ref->scope = (vx_reference)arr;
            }
            else
            {
                vx_uint32 j = 0u;
                /* free all allocated references */
                for (j = 0u; j < i; j++)
                {
                    vxoReference_Release(&(arr->itemsTable[j]), (vx_type_e)item_type, VX_REF_EXTERNAL);
                }

                return VX_ERROR_NO_RESOURCES;
            }
        }
    }

    arr->itemType = item_type;
    arr->itemCount = num_items;

    return VX_SUCCESS;
}

VX_INTERNAL_CALLBACK_API vx_object_array vxoOA_CreateObjectArrayInt(vx_reference scope, vx_reference exemplar, vx_size count, vx_bool is_virtual)
{
    vx_context context = scope->context ? scope->context : (vx_context)scope;

    vx_object_array arr = (vx_object_array)vxoReference_Create(context, VX_TYPE_OBJECT_ARRAY, VX_REF_EXTERNAL, scope);
    if (vxGetStatus((vx_reference)arr) == VX_SUCCESS && arr->base.type == VX_TYPE_OBJECT_ARRAY)
    {
        arr->base.scope = scope;
        arr->base.isVirtual = is_virtual;

        if (vxoOA_InitObjectArrayInt(arr, exemplar, count) != VX_SUCCESS)
        {
            vxoReference_Release((vx_reference *)&arr, VX_TYPE_OBJECT_ARRAY, VX_REF_EXTERNAL);
            arr = (vx_object_array)vxoError_GetErrorObject(context, VX_ERROR_NO_MEMORY);
        }
    }
    return arr;
}

VX_INTERNAL_CALLBACK_API void vxoOA_DestructObjectArray(vx_reference ref)
{
    vx_object_array arr = (vx_object_array)ref;
    vx_uint32 i = 0u;
    vxError("Releasing object array "VX_FMT_REF"\n", (void *)ref);
    for (i = 0u; i < arr->itemCount; i++)
    {
        /* NULL means standard destructor */
        vx_status status = vxoReference_Release(&(arr->itemsTable[i]), (vx_type_e)arr->itemType, VX_REF_EXTERNAL);

        if (status != VX_SUCCESS)
        {
            vxError("Invalid Reference!\n");
        }
    }
}

VX_INTERNAL_CALLBACK_API vx_bool vxoOA_ValidateObjectArray(vx_object_array objarr, vx_enum item_type, vx_size num_items)
{
    vx_bool res = vx_false_e;
    vx_size i = 0u;
    if (objarr->itemType == item_type &&
        objarr->itemCount == num_items)
    {
        for (i = 0u; i < objarr->itemCount; i++)
        {
            if (!vxoReference_IsValidAndSpecific(objarr->itemsTable[i], (vx_type_e)item_type))
                return vx_false_e;
        }
        res = vx_true_e;
    }
    return res;
}

VX_INTERNAL_CALLBACK_API vx_object_array vxoOA_CreateObjectArrayEmpty(vx_reference scope, vx_enum item_type, vx_size count)
{
    vx_context context = scope->context ? scope->context : (vx_context)scope;

    vx_object_array arr = (vx_object_array)vxoReference_Create(context, VX_TYPE_OBJECT_ARRAY, VX_REF_EXTERNAL, scope);

    if (vxGetStatus((vx_reference)arr) == VX_SUCCESS && arr->base.type == VX_TYPE_OBJECT_ARRAY)
    {
        arr->base.scope = scope;
        arr->base.isVirtual = vx_false_e;
        switch (item_type)
        {
            case VX_TYPE_IMAGE:
            case VX_TYPE_ARRAY:
            case VX_TYPE_PYRAMID:
            case VX_TYPE_SCALAR:
            case VX_TYPE_MATRIX:
            case VX_TYPE_DISTRIBUTION:
            case VX_TYPE_REMAP:
            case VX_TYPE_LUT:
            case VX_TYPE_THRESHOLD:
                arr->itemType = item_type;
                break;
            default:
                vxoReference_Release((vx_reference *)&arr, VX_TYPE_OBJECT_ARRAY, VX_REF_EXTERNAL);
                arr = (vx_object_array)vxoError_GetErrorObject(context, VX_ERROR_INVALID_TYPE);
        }
        arr->itemCount = 0;
    }

    return arr;
}

VX_INTERNAL_CALLBACK_API vx_bool vxoOA_SetObjectArrayItem(vx_object_array arr, vx_reference item)
{
    vx_enum item_type = item->type;

    if (vxoOA_IsValidObjectArray(arr) != vx_true_e) return vx_false_e;

    switch (item_type)
    {
        case VX_TYPE_IMAGE:
        case VX_TYPE_ARRAY:
        case VX_TYPE_PYRAMID:
        case VX_TYPE_SCALAR:
        case VX_TYPE_MATRIX:
        case VX_TYPE_DISTRIBUTION:
        case VX_TYPE_REMAP:
        case VX_TYPE_LUT:
        case VX_TYPE_THRESHOLD:
            if (arr->itemType != item_type) return vx_false_e;
            else break;
        default:
            return vx_false_e;
    }

    if (arr->itemCount < VX_MAX_REF_COUNT - 1)
    {
        if (vxoReference_IsValidAndSpecific(item, (vx_type_e)item_type))
        {
            arr->itemsTable[arr->itemCount++] = item;
            item->scope = (vx_reference)arr;
        }
    }
    else
    {
        return vx_false_e;
    }

    return vx_true_e;
}

VX_API_ENTRY vx_object_array VX_API_CALL vxCreateObjectArray(vx_context context, vx_reference exemplar, vx_size count)
{
    vx_object_array arr = NULL;

    if (vxoContext_IsValid(context) == vx_true_e)
    {
        if (vxoReference_IsValidAndNoncontext(exemplar) &&
            (exemplar->type != VX_TYPE_DELAY) &&
            (exemplar->type != VX_TYPE_OBJECT_ARRAY))
        {
            arr = vxoOA_CreateObjectArrayInt((vx_reference)context, exemplar, count, vx_false_e);

            if (arr == NULL)
            {
                arr = (vx_object_array)vxoError_GetErrorObject(context, VX_ERROR_NO_MEMORY);
            }
        }
        else
        {
            arr = (vx_object_array)vxoError_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
        }
    }

    return arr;
}

VX_API_ENTRY vx_object_array VX_API_CALL vxCreateVirtualObjectArray(vx_graph graph, vx_reference exemplar, vx_size count)
{
    vx_object_array arr = NULL;

    if (vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH) == vx_true_e)
    {
        if (vxoReference_IsValidAndNoncontext(exemplar) &&
            (exemplar->type != VX_TYPE_DELAY) &&
            (exemplar->type != VX_TYPE_OBJECT_ARRAY))
        {
            arr = vxoOA_CreateObjectArrayInt((vx_reference)graph, exemplar, count, vx_true_e);

            if (arr == NULL)
            {
                arr = (vx_object_array)vxoError_GetErrorObject(graph->base.context, VX_ERROR_NO_MEMORY);
            }
        }
        else
        {
            arr = (vx_object_array)vxoError_GetErrorObject(graph->base.context, VX_ERROR_INVALID_PARAMETERS);
        }
    }

    return arr;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseObjectArray(vx_object_array *arr)
{
    /* NULL means standard destructor */
    return vxoReference_Release((vx_reference_s **)arr, VX_TYPE_OBJECT_ARRAY, VX_REF_EXTERNAL);
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryObjectArray(vx_object_array arr, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;

    if (vxoOA_IsValidObjectArray(arr) == vx_true_e)
    {
        status = VX_SUCCESS;
        switch (attribute)
        {
            case VX_OBJECT_ARRAY_ITEMTYPE:
                vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);
                *(vx_enum *)ptr = arr->itemType;
                break;

            case VX_OBJECT_ARRAY_NUMITEMS:
                vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

                *(vx_size *)ptr = arr->itemCount;
                break;

            default:
                status = VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }

    return status;
}

VX_API_ENTRY vx_reference VX_API_CALL vxGetObjectArrayItem(vx_object_array arr, vx_uint32 index)
{
    vx_reference item = NULL;

    if (vxoOA_IsValidObjectArray(arr) == vx_true_e)
    {
        if (index < arr->itemCount)
        {
            item = arr->itemsTable[index];
            vxoReference_Increment(item, VX_REF_EXTERNAL);
        }
        else
        {
            item = (vx_reference)vxoError_GetErrorObject(arr->base.context, VX_ERROR_INVALID_PARAMETERS);
        }
    }

    return item;
}

