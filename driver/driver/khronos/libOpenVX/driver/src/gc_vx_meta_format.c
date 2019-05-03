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


#include <gc_vx_common.h>

#define _GC_OBJ_ZONE            gcdZONE_VX_METAFMT

VX_INTERNAL_API vx_meta_format vxoMetaFormat_Create(vx_context context)
{
    vx_meta_format metaFormat;

    gcmHEADER_ARG("context=%p", context);
    vxmASSERT(context);

    metaFormat = (vx_meta_format)vxoReference_Create(context, VX_TYPE_META_FORMAT, VX_REF_EXTERNAL, &context->base);

    if (vxoReference_GetStatus((vx_reference)metaFormat) != VX_SUCCESS)
    {
        gcmFOOTER_ARG("metaFormat=%p", metaFormat);
        return metaFormat;
    }
    metaFormat->size = sizeof(vx_meta_format_s);
    metaFormat->type = VX_TYPE_INVALID;

    gcmFOOTER_ARG("metaFormat=%p", metaFormat);
    return metaFormat;
}

VX_INTERNAL_API void vxoMetaFormat_Release(vx_meta_format_ptr metaFormatPtr)
{
    vxoReference_Release((vx_reference_ptr)metaFormatPtr, VX_TYPE_META_FORMAT, VX_REF_EXTERNAL);
}

VX_API_ENTRY vx_status VX_API_CALL vxSetMetaFormatAttribute(vx_meta_format meta_format, vx_enum attribute, const void *ptr, vx_size size)
{
    gcmHEADER_ARG("meta_format=%p, attribute=0x%x, ptr=%p, size=0x%lx", meta_format, attribute, ptr, size);
    gcmDUMP_API("$VX vxSetMetaFormatAttribute: meta_format=%p, attribute=0x%x, ptr=%p, size=0x%lx", meta_format, attribute, ptr, size);

    if (!vxoReference_IsValidAndSpecific(&meta_format->base, VX_TYPE_META_FORMAT))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
    if (VX_TYPE(attribute) != (vx_uint32)meta_format->type && (attribute != VX_VALID_RECT_CALLBACK))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_TYPE);
        return VX_ERROR_INVALID_TYPE;
    }
    switch (attribute)
    {
        case VX_IMAGE_FORMAT:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_df_image, 0x3);

            meta_format->u.imageInfo.format = *(vx_df_image *)ptr;
            break;

        case VX_IMAGE_HEIGHT:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            meta_format->u.imageInfo.height = *(vx_uint32 *)ptr;
            break;

        case VX_IMAGE_WIDTH:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            meta_format->u.imageInfo.width = *(vx_uint32 *)ptr;
            break;

        case VX_ARRAY_CAPACITY:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            meta_format->u.arrayInfo.capacity = *(vx_size *)ptr;
            break;

        case VX_ARRAY_ITEMTYPE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            meta_format->u.arrayInfo.itemType = *(vx_enum *)ptr;
            break;

        case VX_PYRAMID_FORMAT:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_df_image, 0x3);

            meta_format->u.pyramidInfo.format = *(vx_df_image *)ptr;
            break;

        case VX_PYRAMID_HEIGHT:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            meta_format->u.pyramidInfo.height = *(vx_uint32 *)ptr;
            break;

        case VX_PYRAMID_WIDTH:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            meta_format->u.pyramidInfo.width = *(vx_uint32 *)ptr;
            break;

        case VX_PYRAMID_LEVELS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            meta_format->u.pyramidInfo.levelCount = *(vx_size *)ptr;
            break;

        case VX_PYRAMID_SCALE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_float32, 0x3);

            meta_format->u.pyramidInfo.scale = *(vx_float32 *)ptr;
            break;

        case VX_SCALAR_TYPE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            meta_format->u.scalarInfo.type = *(vx_enum *)ptr;
            break;

        case VX_REF_ATTRIBUTE_TYPE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            meta_format->type = *(vx_enum *)ptr;
            break;

        /**********************************************************************/
        case VX_MATRIX_TYPE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            meta_format->u.matrixInfo.type = *(vx_enum *)ptr;
            break;
        case VX_MATRIX_ROWS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            meta_format->u.matrixInfo.rows = *(vx_size *)ptr;
            break;
        case VX_MATRIX_COLUMNS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);
            meta_format->u.matrixInfo.cols = *(vx_size *)ptr;

            break;
        /**********************************************************************/
        case VX_DISTRIBUTION_BINS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);
            meta_format->u.distributionInfo.bins = *(vx_size *)ptr;

            break;
        case VX_DISTRIBUTION_RANGE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);
            meta_format->u.distributionInfo.range = *(vx_uint32 *)ptr;

            break;
        case VX_DISTRIBUTION_OFFSET:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_int32, 0x3);
            meta_format->u.distributionInfo.offset = *(vx_int32 *)ptr;

            break;
        /**********************************************************************/
        case VX_REMAP_SOURCE_WIDTH:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);
            meta_format->u.remapInfo.src_width = *(vx_uint32 *)ptr;

            break;
        case VX_REMAP_SOURCE_HEIGHT:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);
            meta_format->u.remapInfo.src_height = *(vx_uint32 *)ptr;

            break;
        case VX_REMAP_DESTINATION_WIDTH:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);
            meta_format->u.remapInfo.dst_width = *(vx_uint32 *)ptr;

            break;
        case VX_REMAP_DESTINATION_HEIGHT:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);
            meta_format->u.remapInfo.dst_height = *(vx_uint32 *)ptr;

            break;
        /**********************************************************************/
        case VX_LUT_TYPE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);
            meta_format->u.lutInfo.type = *(vx_enum *)ptr;

            break;
        case VX_LUT_COUNT:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);
            meta_format->u.lutInfo.count = *(vx_size *)ptr;

            break;
        /**********************************************************************/
        case VX_THRESHOLD_TYPE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);
            meta_format->u.thresholdInfo.type = *(vx_enum *)ptr;

            break;

        case VX_THRESHOLD_INPUT_FORMAT:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_df_image, 0x3);
            meta_format->u.thresholdInfo.input_format = *(vx_df_image *)ptr;

        case VX_VALID_RECT_CALLBACK:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_kernel_image_valid_rectangle_f, 0x0);

            meta_format->setValidRectangleCallback = *(vx_kernel_image_valid_rectangle_f*)ptr;

            break;

        /**********************************************************************/
        case VX_OBJECT_ARRAY_ITEMTYPE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);
            meta_format->u.objectArrayInfo.item_type = *(vx_enum *)ptr;

            break;
        case VX_OBJECT_ARRAY_NUMITEMS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            meta_format->u.objectArrayInfo.item_count = *(vx_size *)ptr;
            break;

        case VX_TENSOR_DATA_TYPE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            meta_format->u.tensorInfo.dataFormat = *(vx_enum *)ptr;
            break;

        case VX_TENSOR_FIXED_POINT_POSITION:
            vxmVALIDATE_PARAMETERS_EX(ptr, size, vx_uint8);

            meta_format->u.tensorInfo.fixedPointPosition = *(vx_uint8 *)ptr;
           break;

        case VX_TENSOR_NUMBER_OF_DIMS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            meta_format->u.tensorInfo.numOfDims = *(vx_uint32 *)ptr;
           break;

        case VX_TENSOR_DIMS:
            if (size <= (sizeof(vx_size)*VX_CONTEXT_TENSOR_MAX_DIMENSION) && ((vx_size)ptr & 0x3) == 0)
            {
                vx_uint32 i;
                for (i = 0; i < size / sizeof(vx_size); i++)
                    meta_format->u.tensorInfo.dimensions[i] = (vx_uint32)(*(vx_size*)((vx_uint8*)ptr + i * sizeof(vx_size)));
            }
            else
            {
                gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
                return VX_ERROR_INVALID_PARAMETERS;
            }
           break;

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            gcmFOOTER_ARG("%d", VX_ERROR_NOT_SUPPORTED);
            return VX_ERROR_NOT_SUPPORTED;
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetMetaFormatFromReference(vx_meta_format meta, vx_reference examplar)
{
    vx_status status = VX_SUCCESS;

    gcmHEADER_ARG("meta=%p, examplar=%p", meta, examplar);
    gcmDUMP_API("$VX vxSetMetaFormatFromReference: meta=%p, examplar=%p", meta, examplar);

    if (vxoReference_IsValidAndSpecific(&meta->base, VX_TYPE_META_FORMAT) == vx_false_e)
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }

    if (vxoReference_IsValidAndNoncontext(examplar) == vx_false_e)
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
    switch (examplar->type)
    {
        case VX_TYPE_IMAGE:
        {
            vx_image image = (vx_image)examplar;
            meta->type = VX_TYPE_IMAGE;
            meta->u.imageInfo.width = image->width;
            meta->u.imageInfo.height = image->height;
            meta->u.imageInfo.format = image->format;
            break;
        }

        case VX_TYPE_PYRAMID:
        {
            vx_pyramid pyramid = (vx_pyramid)examplar;
            meta->type = VX_TYPE_PYRAMID;
            meta->u.pyramidInfo.width = pyramid->width;
            meta->u.pyramidInfo.height = pyramid->height;
            meta->u.pyramidInfo.format = pyramid->format;
            meta->u.pyramidInfo.levelCount = pyramid->levelCount;
            meta->u.pyramidInfo.scale = pyramid->scale;
            break;
        }

        case VX_TYPE_SCALAR:
        {
            vx_scalar scalar = (vx_scalar)examplar;
            meta->type = VX_TYPE_SCALAR;
            meta->u.scalarInfo.type = scalar->dataType;
            break;
        }

        case VX_TYPE_ARRAY:
        {
            vx_array array = (vx_array)examplar;
            meta->type = VX_TYPE_ARRAY;
            meta->u.arrayInfo.itemType = array->itemType;
            meta->u.arrayInfo.capacity = array->capacity;
            break;
        }

        case VX_TYPE_MATRIX:
        {
            vx_matrix matrix = (vx_matrix)examplar;

            meta->type = VX_TYPE_MATRIX;
            meta->u.matrixInfo.type = matrix->dataType;
            meta->u.matrixInfo.cols = matrix->columns;
            meta->u.matrixInfo.rows = matrix->rows;

            break;
        }
        case VX_TYPE_DISTRIBUTION:
        {
            vx_distribution distribution = (vx_distribution)examplar;

            meta->type = VX_TYPE_DISTRIBUTION;
            meta->u.distributionInfo.bins = distribution->memory.dims[0][VX_DIM_X];
            meta->u.distributionInfo.offset = distribution->offsetX;
            meta->u.distributionInfo.range  = distribution->rangeX;

            break;
        }
        case VX_TYPE_REMAP:
        {
            vx_remap remap = (vx_remap)examplar;

            meta->type = VX_TYPE_REMAP;
            meta->u.remapInfo.src_width  = remap->srcWidth;
            meta->u.remapInfo.src_height = remap->srcHeight;
            meta->u.remapInfo.dst_width  = remap->destWidth;
            meta->u.remapInfo.dst_height = remap->destHeight;

            break;
        }
        case VX_TYPE_LUT:
        {
            vx_lut_s *lut = (vx_lut_s *)examplar;

            meta->type = VX_TYPE_LUT;
            meta->u.lutInfo.type = lut->itemType;
            meta->u.lutInfo.count = lut->itemCount;

            break;
        }
        case VX_TYPE_THRESHOLD:
        {
            vx_threshold threshold = (vx_threshold)examplar;

            meta->type = VX_TYPE_THRESHOLD;
            meta->u.thresholdInfo.type = threshold->thresholdType;
            break;
        }
        case VX_TYPE_OBJECT_ARRAY:
        {
            vx_object_array objarray = (vx_object_array)examplar;
            vx_reference item = objarray->itemsTable[0];
            meta->type = VX_TYPE_OBJECT_ARRAY;
            meta->u.objectArrayInfo.item_type = objarray->itemType;
            meta->u.objectArrayInfo.item_count = objarray->itemCount;

            switch (objarray->itemType)
            {
            case VX_TYPE_IMAGE:
            {
                vx_image image = (vx_image)item;
                meta->u.imageInfo.width  = image->width;
                meta->u.imageInfo.height = image->height;
                meta->u.imageInfo.format = image->format;
                break;
            }
            case VX_TYPE_ARRAY:
            {
                vx_array array = (vx_array)item;
                meta->u.arrayInfo.itemType = array->itemType;
                meta->u.arrayInfo.capacity = array->capacity;
                break;
            }
            case VX_TYPE_PYRAMID:
            {
                vx_pyramid pyramid = (vx_pyramid)item;
                meta->u.pyramidInfo.width  = pyramid->width;
                meta->u.pyramidInfo.height = pyramid->height;
                meta->u.pyramidInfo.format = pyramid->format;
                meta->u.pyramidInfo.levelCount = pyramid->levelCount;
                meta->u.pyramidInfo.scale      = pyramid->scale;
                break;
            }
            case VX_TYPE_SCALAR:
            {
                vx_scalar scalar = (vx_scalar)item;
                meta->u.scalarInfo.type = scalar->dataType;
                break;
            }
            case VX_TYPE_MATRIX:
            {
                vx_matrix matrix = (vx_matrix)item;
                meta->u.matrixInfo.type = matrix->dataType;
                meta->u.matrixInfo.cols = matrix->columns;
                meta->u.matrixInfo.rows = matrix->rows;
                break;
            }
            case VX_TYPE_DISTRIBUTION:
            {
                vx_distribution distribution = (vx_distribution)item;
                meta->u.distributionInfo.bins = distribution->memory.dims[0][VX_DIM_X];
                meta->u.distributionInfo.offset = distribution->offsetX;
                meta->u.distributionInfo.range = distribution->rangeX;
                break;
            }
            case VX_TYPE_REMAP:
            {
                vx_remap remap = (vx_remap)item;
                meta->u.remapInfo.src_width = remap->srcWidth;
                meta->u.remapInfo.src_height = remap->srcHeight;
                meta->u.remapInfo.dst_width = remap->destWidth;
                meta->u.remapInfo.dst_height = remap->destHeight;
                break;
            }
            case VX_TYPE_LUT:
            {
                vx_lut_s *lut = (vx_lut_s *)item;
                meta->u.lutInfo.type = lut->itemType;
                meta->u.lutInfo.count = lut->itemCount;
                break;
            }
            case VX_TYPE_THRESHOLD:
            {
                vx_threshold threshold = (vx_threshold)item;
                meta->u.thresholdInfo.type = threshold->thresholdType;
                break;
            }
            default:
                status = VX_ERROR_INVALID_REFERENCE;
                break;
            }

            break;
        }
    default:
        status = VX_ERROR_INVALID_REFERENCE;
        break;
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

