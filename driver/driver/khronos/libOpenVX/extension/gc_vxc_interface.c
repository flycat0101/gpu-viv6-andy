/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <VX/vx.h>

#include <gc_vx_common.h>
#include <gc_vxc_interface.h>
#include <gc_vx_internal_node_api.h>

VX_PRIVATE_API void vxoFillMetaData(vx_meta_format_s *ptr, vx_enum type, vx_df_image format, vx_uint32 width, vx_uint32 height, vx_enum dataInfoType)
{
    ptr->type               = type;

    switch (type)
    {
        case VX_TYPE_IMAGE:
            ptr->u.imageInfo.format = format;
            ptr->u.imageInfo.width  = width;
            ptr->u.imageInfo.height = height;
            break;

        case VX_TYPE_DISTRIBUTION:
            break;

        case VX_TYPE_SCALAR:
            ptr->u.scalarInfo.type = dataInfoType;

        default:
            break;
    }

}

VX_PRIVATE_API vx_status vxoGetObjAttributeByNodeIndex(vx_node node, vx_uint32 index, vx_enum type, vx_object_data_s* objData)
{
    vx_parameter    param     = VX_NULL;
    vx_image        image     = VX_NULL;
    vx_scalar       scalar    = VX_NULL;
    vx_lut          lut       = VX_NULL;
    vx_distribution dist      = VX_NULL;
    vx_threshold    threshold = VX_NULL;
    vx_convolution  conv      = VX_NULL;
    vx_matrix       matrix    = VX_NULL;
    vx_remap        remap     = VX_NULL;
    vx_pyramid      pyramid   = VX_NULL;
    vx_array        array     = VX_NULL;
    vx_status       status    = VX_ERROR_INVALID_PARAMETERS;

    objData->objType = type;

    param = vxGetParameterByIndex(node, index);
    if (param == VX_NULL) return VX_ERROR_INVALID_PARAMETERS;

    switch (type)
    {
        case VX_TYPE_IMAGE:
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &image, sizeof(image));
            if (image == VX_NULL) goto ErrorExit;

            vxQueryImage(image, VX_IMAGE_ATTRIBUTE_FORMAT, &objData->u.imageInfo.format, sizeof(vx_df_image));
            vxQueryImage(image, VX_IMAGE_ATTRIBUTE_WIDTH, &objData->u.imageInfo.width, sizeof(vx_uint32));
            vxQueryImage(image, VX_IMAGE_ATTRIBUTE_HEIGHT, &objData->u.imageInfo.height, sizeof(vx_uint32));

            break;

        case VX_TYPE_SCALAR:
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &scalar, sizeof(scalar));
            if (scalar == VX_NULL) goto ErrorExit;

            vxQueryScalar(scalar, VX_SCALAR_ATTRIBUTE_TYPE, &objData->u.scalarInfo.dataType, sizeof(vx_enum));
            if (objData->u.scalarInfo.scalarValuePtr != VX_NULL)
            {
                vxAccessScalarValue(scalar, objData->u.scalarInfo.scalarValuePtr);
            }

            break;

        case VX_TYPE_LUT:
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &lut, sizeof(lut));
            if (lut == VX_NULL) goto ErrorExit;

            vxQueryLUT(lut, VX_LUT_ATTRIBUTE_TYPE, &objData->u.lutArrayInfo.dataType, sizeof(vx_enum));

            break;

        case VX_TYPE_DISTRIBUTION:
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &dist, sizeof(dist));
            if (dist == VX_NULL) goto ErrorExit;

            vxQueryDistribution(dist, VX_DISTRIBUTION_ATTRIBUTE_BINS, &objData->u.distributionInfo.numBins, sizeof(vx_size));

            break;
        case VX_TYPE_THRESHOLD:
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &threshold, sizeof(threshold));
            if (threshold == VX_NULL) goto ErrorExit;

            vxQueryThreshold(threshold, VX_THRESHOLD_ATTRIBUTE_TYPE, &objData->u.thresholdInfo.dataType, sizeof(vx_enum));
            break;

        case VX_TYPE_CONVOLUTION:
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &conv, sizeof(conv));
            if (conv == VX_NULL) goto ErrorExit;

            vxQueryConvolution(conv, VX_CONVOLUTION_ATTRIBUTE_COLUMNS, &objData->u.convolutionInfo.columns, sizeof(vx_size));
            vxQueryConvolution(conv, VX_CONVOLUTION_ATTRIBUTE_ROWS,&objData->u.convolutionInfo.rows, sizeof(vx_size));
            break;

        case VX_TYPE_MATRIX:
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &matrix, sizeof(matrix));
            if (matrix == VX_NULL) goto ErrorExit;

            vxQueryMatrix(matrix, VX_MATRIX_ATTRIBUTE_TYPE, &objData->u.matrixInfo.dataType, sizeof(vx_enum));
            vxQueryMatrix(matrix, VX_MATRIX_ATTRIBUTE_ROWS, &objData->u.matrixInfo.rows, sizeof(vx_size));
            vxQueryMatrix(matrix, VX_MATRIX_ATTRIBUTE_COLUMNS, &objData->u.matrixInfo.columns, sizeof(vx_size));
            break;

        case VX_TYPE_REMAP:
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &remap, sizeof(remap));
            if (remap == VX_NULL) goto ErrorExit;

            vxQueryRemap(remap, VX_REMAP_ATTRIBUTE_SOURCE_WIDTH, &objData->u.remapInfo.srcWidth, sizeof(vx_uint32));
            vxQueryRemap(remap, VX_REMAP_ATTRIBUTE_SOURCE_HEIGHT, &objData->u.remapInfo.srcHeight, sizeof(vx_uint32));
            vxQueryRemap(remap, VX_REMAP_ATTRIBUTE_DESTINATION_WIDTH, &objData->u.remapInfo.dstWidth, sizeof(vx_uint32));
            vxQueryRemap(remap, VX_REMAP_ATTRIBUTE_DESTINATION_HEIGHT, &objData->u.remapInfo.dstHeight, sizeof(vx_uint32));
            break;

        case VX_TYPE_PYRAMID:
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &pyramid, sizeof(pyramid));
            if (pyramid == VX_NULL) goto ErrorExit;

            vxQueryPyramid(pyramid, VX_PYRAMID_ATTRIBUTE_LEVELS, &objData->u.pyramidInfo.numLevels, sizeof(vx_size));
            vxQueryPyramid(pyramid, VX_PYRAMID_ATTRIBUTE_SCALE, &objData->u.pyramidInfo.scale, sizeof(vx_float32));
            break;

        case VX_TYPE_ARRAY:
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &array, sizeof(array));
            if (array == VX_NULL) goto ErrorExit;

            vxQueryArray(array, VX_ARRAY_ATTRIBUTE_ITEMTYPE, &objData->u.arrayInfo.dataType, sizeof(vx_enum));

            vxQueryArray(array, VX_ARRAY_ATTRIBUTE_CAPACITY, &objData->u.arrayInfo.capacity, sizeof(vx_size));
            break;
    }

    status = VX_SUCCESS;

ErrorExit:
    if (image != VX_NULL) vxReleaseImage(&image);

    if (scalar != VX_NULL) vxReleaseScalar(&scalar);

    if (lut != VX_NULL) vxReleaseLUT(&lut);

    if (dist != VX_NULL) vxReleaseDistribution(&dist);

    if (threshold != VX_NULL) vxReleaseThreshold(&threshold);

    if (conv != VX_NULL) vxReleaseConvolution(&conv);

    if (matrix != VX_NULL) vxReleaseMatrix(&matrix);

    if (param != VX_NULL) vxReleaseParameter(&param);

    if (remap != VX_NULL) vxReleaseRemap(&remap);

    if (pyramid != VX_NULL) vxReleasePyramid(&pyramid);

    if (array != VX_NULL) vxReleaseArray(&array);

    return status;
}

VX_PRIVATE_API vx_status vxoAddParameterToGraphByIndex(vx_graph graph, vx_node node, vx_uint32 index)
{
    vx_parameter param;
    vx_status    status;

    param = vxGetParameterByIndex(node, index);

    status = vxAddParameterToGraph(graph, param);

    vxReleaseParameter(&param);

    return status;
}

VX_INTERNAL_API vx_bool vxoKernel_IsUnique(vx_kernel kernel)
{
    vx_context context;
    vx_uint32 i, k;

    vxmASSERT(kernel);

    context = kernel->base.context;

    vxmASSERT(context);

    for (i = 0u; i < context->targetCount; i++)
    {
        for (k = 0u; k < context->targetTable[i].kernelCount; k++)
        {
            if (context->targetTable[i].kernelTable[k].enabled
                && context->targetTable[i].kernelTable[k].enumeration == kernel->enumeration)
            {
                return vx_false_e;
            }
        }
    }

    return vx_true_e;
}

VX_PRIVATE_API vx_string gcoVX_LoadSources(vx_char *filename, vx_size *programSize)
{
    FILE *pFile = NULL;
    vx_string path, programSource = NULL;
    vx_char fullname[1024] = "\0";
    vx_char defname[1024] = VX_SHADER_SOURCE_PATH;

    gcoOS_GetEnv(gcvNULL, "VX_SHADER_SOURCE_PATH", &path);
    strcat(fullname, (path != NULL)?path:defname);

    strcat(fullname, filename);

    pFile = fopen(fullname, "rb");
    if (pFile != NULL && programSize)
    {
        vx_int32 size = 0;
        /* obtain file size:*/
        fseek(pFile, 0, SEEK_END);
        *programSize = ftell(pFile);
        rewind(pFile);

        size = (int)(*programSize + 1);
        programSource = (char*)malloc(sizeof(char)*(size));
        if (programSource == gcvNULL)
        {
            fclose(pFile);
            free(programSource);
            return gcvNULL;
        }

        fread(programSource, sizeof(char), *programSize, pFile);
        programSource[*programSize] = '\0';
        fclose(pFile);
    }

    return programSource;
}

VX_PRIVATE_API vx_status vxoSobel3x3_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData = {0};

    if (index != 0) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if (objData.u.imageInfo.width < 3 ||
        objData.u.imageInfo.height < 3 ||
        objData.u.imageInfo.format != VX_DF_IMAGE_U8)
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoSobel3x3_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    vx_object_data_s objData = {0};

    if (index != 1 && index != 2) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    vxoFillMetaData(ptr, VX_TYPE_IMAGE, VX_DF_IMAGE_S16, objData.u.imageInfo.width, objData.u.imageInfo.height, 0);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoTableLookup_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData = {0};

    if (index != 0 && index != 1) return VX_ERROR_INVALID_PARAMETERS;

    if (index == 0)
    {
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData.u.imageInfo.format != VX_DF_IMAGE_U8 && objData.u.imageInfo.format != VX_DF_IMAGE_S16)
            return VX_ERROR_INVALID_FORMAT;

    }
    else if (index == 1)
    {
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_LUT, &objData) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if ((objData.u.lutArrayInfo.dataType != VX_TYPE_UINT8) && (objData.u.lutArrayInfo.dataType != VX_TYPE_INT16))
            return VX_ERROR_INVALID_TYPE;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoTableLookup_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    vx_object_data_s objData = {0};

    if (index != 2) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    vxoFillMetaData(ptr, VX_TYPE_IMAGE, VX_DF_IMAGE_U8, objData.u.imageInfo.width, objData.u.imageInfo.height, 0);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoHistogram_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData = {0};

    if (index != 0) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if (objData.u.imageInfo.format != VX_DF_IMAGE_U8 && objData.u.imageInfo.format != VX_DF_IMAGE_U16)
        return VX_ERROR_INVALID_FORMAT;

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoHistogram_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    vx_object_data_s objData0 = {0};
    vx_object_data_s objData1 = {0};

    if (index != 1) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData0) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 1, VX_TYPE_DISTRIBUTION, &objData1) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    vxoFillMetaData(ptr, VX_TYPE_DISTRIBUTION, 0, 0, 0, 0);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoAbsDiff_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData0 = {0};
    vx_object_data_s objData1 = {0};

    if (index != 0 && index != 1) return VX_ERROR_INVALID_PARAMETERS;

    if (index == 0 )
    {
        if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData0) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData0.u.imageInfo.format != VX_DF_IMAGE_U8 &&
            objData0.u.imageInfo.format != VX_DF_IMAGE_S16 &&
            objData0.u.imageInfo.format != VX_DF_IMAGE_U16)
        {
            return VX_ERROR_INVALID_FORMAT;
        }
    }
    else if (index == 1)
    {
        if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData0) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (vxoGetObjAttributeByNodeIndex(node, 1, VX_TYPE_IMAGE, &objData1) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData0.u.imageInfo.width != objData1.u.imageInfo.width ||
            objData0.u.imageInfo.height != objData1.u.imageInfo.height ||
            objData0.u.imageInfo.format != objData1.u.imageInfo.format)
        {
            return VX_ERROR_INVALID_PARAMETERS;
        }
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoAbsDiff_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    vx_object_data_s objData0 = {0};
    vx_object_data_s objData1 = {0};

    if (index != 2) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData0) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 1, VX_TYPE_IMAGE, &objData1) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if (objData0.u.imageInfo.width != objData1.u.imageInfo.width ||
        objData0.u.imageInfo.height != objData1.u.imageInfo.height)
    {
        return VX_ERROR_INVALID_VALUE;
    }

    if (objData0.u.imageInfo.format != VX_DF_IMAGE_U8 &&
        objData0.u.imageInfo.format != VX_DF_IMAGE_U16)
    {
        return VX_ERROR_INVALID_FORMAT;
    }

    vxoFillMetaData(ptr, VX_TYPE_IMAGE,
                    objData0.u.imageInfo.format,
                    objData0.u.imageInfo.width,
                    objData0.u.imageInfo.height,
                    0);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoThreshold_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData = {0};

    if (index != 0 && index != 1) return VX_ERROR_INVALID_PARAMETERS;

    if (index == 0)
    {
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData.u.imageInfo.format != VX_DF_IMAGE_U8) return VX_ERROR_INVALID_FORMAT;
    }
    else if (index == 1)
    {
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_THRESHOLD, &objData) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if ((objData.u.thresholdInfo.dataType != VX_THRESHOLD_TYPE_BINARY) &&
            (objData.u.thresholdInfo.dataType != VX_THRESHOLD_TYPE_RANGE))
            return VX_ERROR_INVALID_TYPE;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoThreshold_ValidatorOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    vx_object_data_s objData = {0};

    if (index != 2) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    vxoFillMetaData(ptr, VX_TYPE_IMAGE,
                    objData.u.imageInfo.format,
                    objData.u.imageInfo.width,
                    objData.u.imageInfo.height,
                    0);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoMorphology_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData = {0};

    if (index != 0) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if (objData.u.imageInfo.format != VX_DF_IMAGE_U8) return VX_ERROR_INVALID_FORMAT;

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoMorphology_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s*ptr)
{
    vx_object_data_s objData = {0};

    if (index != 1) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    vxoFillMetaData(ptr, VX_TYPE_IMAGE, VX_DF_IMAGE_U8, objData.u.imageInfo.width, objData.u.imageInfo.height, 0);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoFilter_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData;

    if (index != 0) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if (objData.u.imageInfo.format != VX_DF_IMAGE_U8) return VX_SUCCESS;

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoFilter_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    vx_object_data_s objData;

    if (index != 1) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    vxoFillMetaData(ptr, VX_TYPE_IMAGE, VX_DF_IMAGE_U8, objData.u.imageInfo.width, objData.u.imageInfo.height, 0);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoConvolve_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData = {0};

    if (index != 0 && index != 1) return VX_ERROR_INVALID_PARAMETERS;

    if (index == 0)
    {
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if ((objData.u.imageInfo.width <= VX_MAX_CONVOLUTION_DIM) ||
            (objData.u.imageInfo.height <= VX_MAX_CONVOLUTION_DIM))

        {
            return VX_ERROR_INVALID_PARAMETERS;
        }

        if ((objData.u.imageInfo.format != VX_DF_IMAGE_U8) &&
            (objData.u.imageInfo.format != VX_DF_IMAGE_S16))
        {
            return VX_ERROR_INVALID_FORMAT;
        }
    }
    else  if (index == 1)
    {
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_CONVOLUTION, &objData) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData.u.convolutionInfo.rows > VX_MAX_CONVOLUTION_DIM ||
            objData.u.convolutionInfo.columns > VX_MAX_CONVOLUTION_DIM)
        {
            return VX_ERROR_INVALID_DIMENSION;
        }
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoConvolve_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    vx_object_data_s objData0 = {0};
    vx_object_data_s objData2 = {0};

    if (index != 2) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData0) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData2) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if (objData2.u.imageInfo.format == VX_DF_IMAGE_U8)
    {
        vxoFillMetaData(ptr, VX_TYPE_IMAGE, VX_DF_IMAGE_U8, objData0.u.imageInfo.width, objData0.u.imageInfo.height, 0);
    }
    else
    {
        vxoFillMetaData(ptr, VX_TYPE_IMAGE, VX_DF_IMAGE_S16, objData0.u.imageInfo.width, objData0.u.imageInfo.height, 0);
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoMinMaxLoc_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData;

    if (index != 0) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if ((objData.u.imageInfo.format != VX_DF_IMAGE_U8) &&
        (objData.u.imageInfo.format != VX_DF_IMAGE_U16) &&
        (objData.u.imageInfo.format != VX_DF_IMAGE_S16) &&
        (objData.u.imageInfo.format != VX_DF_IMAGE_U32) &&
        (objData.u.imageInfo.format != VX_DF_IMAGE_S32))
    {
        return VX_ERROR_INVALID_FORMAT;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoMinMaxLoc_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    vx_object_data_s objData;
    vx_enum          type = VX_TYPE_INVALID;

    switch(index)
    {
        case 1:
        case 2:
            if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
                return VX_ERROR_INVALID_PARAMETERS;

            switch (objData.u.imageInfo.format)
            {
                case VX_DF_IMAGE_U8:
                    type = VX_TYPE_UINT8;
                    break;
                case VX_DF_IMAGE_U16:
                    type = VX_TYPE_UINT16;
                    break;
                case VX_DF_IMAGE_U32:
                    type = VX_TYPE_UINT32;
                    break;
                case VX_DF_IMAGE_S16:
                    type = VX_TYPE_INT16;
                    break;
                case VX_DF_IMAGE_S32:
                    type = VX_TYPE_INT32;
                    break;
                default:
                   return VX_ERROR_INVALID_TYPE;
            }

            ptr->type = VX_TYPE_SCALAR;
            ptr->u.scalarInfo.type = type;
            break;

        case 3:
        case 4:
            ptr->u.arrayInfo.itemType = VX_TYPE_COORDINATES2D;
            ptr->u.arrayInfo.capacity = 1;
            break;

        case 5:
        case 6:
            ptr->u.scalarInfo.type = VX_TYPE_UINT32;
            break;
        default:
            return VX_ERROR_INVALID_PARAMETERS;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoMultiply_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData[2] = {{0}};
    vx_float32       scale = 0.0f;
    vx_enum          overflow_policy = 0;
    vx_enum          rouding_policy = 0;

    if (index != 0 && index != 1 && index != 2 && index !=3 && index != 4) return VX_ERROR_INVALID_PARAMETERS;

    switch(index)
    {
    case 0:
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData[0]) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData[0].u.imageInfo.format != VX_DF_IMAGE_U8 && objData[0].u.imageInfo.format != VX_DF_IMAGE_S16)
            return VX_ERROR_INVALID_FORMAT;

        break;

    case 1:
        if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData[0]) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (vxoGetObjAttributeByNodeIndex(node, 1, VX_TYPE_IMAGE, &objData[1]) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData[0].u.imageInfo.width != objData[1].u.imageInfo.width ||
            objData[0].u.imageInfo.height != objData[1].u.imageInfo.height)
        {
            return VX_ERROR_INVALID_VALUE;
        }

        if (objData[1].u.imageInfo.format != VX_DF_IMAGE_U8 &&
            objData[1].u.imageInfo.format != VX_DF_IMAGE_S16)
        {
            return VX_ERROR_INVALID_FORMAT;
        }

        break;

    case 2:
        objData[0].u.scalarInfo.scalarValuePtr = &scale;

        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_SCALAR, &objData[0]) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData[0].u.scalarInfo.dataType != VX_TYPE_FLOAT32) return VX_ERROR_INVALID_TYPE;

        if (scale < 0) return VX_ERROR_INVALID_VALUE;

        break;

    case 3:
        objData[0].u.scalarInfo.scalarValuePtr = &overflow_policy;

        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_SCALAR, &objData[0]) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData[0].u.scalarInfo.dataType != VX_TYPE_ENUM) return VX_ERROR_INVALID_TYPE;

        if ((overflow_policy != VX_CONVERT_POLICY_WRAP) &&
            (overflow_policy != VX_CONVERT_POLICY_SATURATE))
        {
            return VX_ERROR_INVALID_VALUE;
        }

        break;

    case 4:
        objData[0].u.scalarInfo.scalarValuePtr = &rouding_policy;

        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_SCALAR, &objData[0]) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData[0].u.scalarInfo.dataType != VX_TYPE_ENUM) return VX_ERROR_INVALID_TYPE;

        if ((rouding_policy != VX_ROUND_POLICY_TO_ZERO) &&
            (rouding_policy != VX_ROUND_POLICY_TO_NEAREST_EVEN))
        {
            return VX_ERROR_INVALID_VALUE;
        }

        break;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoMultiply_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    vx_object_data_s objData[3] = {{0}};

    if (index != 5) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData[0]) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 1, VX_TYPE_IMAGE, &objData[1]) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData[2]) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if (objData[0].u.imageInfo.format == VX_DF_IMAGE_U8 &&
        objData[1].u.imageInfo.format == VX_DF_IMAGE_U8 &&
        objData[2].u.imageInfo.format == VX_DF_IMAGE_U8)
    {
        vxoFillMetaData(ptr, VX_TYPE_IMAGE, VX_DF_IMAGE_U8, objData[0].u.imageInfo.width, objData[0].u.imageInfo.height, 0);
    }
    else
    {
        vxoFillMetaData(ptr, VX_TYPE_IMAGE, VX_DF_IMAGE_S16, objData[0].u.imageInfo.width, objData[0].u.imageInfo.height, 0);
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoWarp_ValidateInput(vx_node node, vx_uint32 index, vx_size mat_columns)
{
    vx_object_data_s objData[3] = {{0}};
    vx_enum          interp = 0;

    if (index != 0 && index != 1 && index != 2) return VX_ERROR_INVALID_PARAMETERS;

    switch(index)
    {
    case 0:
        if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData[0]) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData[0].u.imageInfo.format != VX_DF_IMAGE_U8) return VX_ERROR_INVALID_FORMAT;

        break;
    case 1:
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_MATRIX, &objData[1]) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if ((objData[1].u.matrixInfo.dataType != VX_TYPE_FLOAT32) ||
            (objData[1].u.matrixInfo.columns != mat_columns) ||
            (objData[1].u.matrixInfo.rows != 3))
        {
            return VX_ERROR_INVALID_VALUE;
        }
        break;
    case 2:
        objData[2].u.scalarInfo.scalarValuePtr = &interp;
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_SCALAR, &objData[2]) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData[2].u.scalarInfo.dataType != VX_TYPE_ENUM) return VX_ERROR_INVALID_TYPE;

        if ((interp != VX_INTERPOLATION_TYPE_NEAREST_NEIGHBOR) &&
            (interp != VX_INTERPOLATION_TYPE_BILINEAR))
        {
            return VX_ERROR_INVALID_VALUE;
        }
        break;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoWarpAffine_ValidateInput(vx_node node, vx_uint32 index)
{
    return vxoWarp_ValidateInput(node, index, 2);
}

VX_PRIVATE_API vx_status vxoWarp_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    vx_object_data_s objData = {0};

    if (index != 3) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if ((objData.u.imageInfo.width == 0) || (objData.u.imageInfo.height == 0))
        return VX_ERROR_INVALID_VALUE;

    if (objData.u.imageInfo.format != VX_DF_IMAGE_U8) return VX_ERROR_INVALID_FORMAT;

    vxoFillMetaData(ptr, VX_TYPE_IMAGE, VX_DF_IMAGE_U8, objData.u.imageInfo.width, objData.u.imageInfo.height, 0);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoRemap_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData[3] = {{0}};
    vx_enum          policy     = 0;

    if (index != 0 && index != 1 && index != 2) return VX_ERROR_INVALID_PARAMETERS;

    switch (index)
    {
        case 0:
            if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData[0]) != VX_SUCCESS)
                return VX_ERROR_INVALID_PARAMETERS;

            if (objData[0].u.imageInfo.format != VX_DF_IMAGE_U8) return VX_ERROR_INVALID_FORMAT;

            break;
        case 1:
            if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_REMAP, &objData[1]) != VX_SUCCESS)
                return VX_ERROR_INVALID_PARAMETERS;

            break;

        case 2:
            objData[2].u.scalarInfo.scalarValuePtr = &policy;
            if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_SCALAR, &objData[2]) != VX_SUCCESS)
                return VX_ERROR_INVALID_PARAMETERS;

            if (objData[2].u.scalarInfo.dataType != VX_TYPE_ENUM) return VX_ERROR_INVALID_TYPE;

            if ((policy != VX_INTERPOLATION_TYPE_NEAREST_NEIGHBOR) &&
                (policy != VX_INTERPOLATION_TYPE_BILINEAR))
            {
                return VX_ERROR_INVALID_VALUE;
            }

            break;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoRemap_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    vx_object_data_s objData[3] = {{0}};

    if (index != 3) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData[0]) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 1, VX_TYPE_REMAP, &objData[1]) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData[2]) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if ((objData[0].u.imageInfo.width != objData[1].u.remapInfo.srcWidth) ||
        (objData[0].u.imageInfo.height != objData[1].u.remapInfo.srcHeight))
    {
        return VX_ERROR_INVALID_VALUE;
    }

    vxoFillMetaData(ptr, VX_TYPE_IMAGE, VX_DF_IMAGE_U8, objData[1].u.remapInfo.dstWidth, objData[1].u.remapInfo.dstHeight, 0);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoProgrameKernel_MinMaxLoc(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_graph graph = vxGetChildGraphOfNode(node);
    vx_status status = vxProcessGraph(graph);

    vx_array minLoc     = (vx_array)parameters[3];
    vx_array maxLoc     = (vx_array)parameters[4];
    vx_scalar minCount   = (vx_scalar)parameters[5];
    vx_scalar maxCount   = (vx_scalar)parameters[6];
    vx_uint32 min_count = 0, max_count = 0;
    vx_uint32 min_capacity = 0, max_capacity = 0;

    if((minCount == NULL) && (minLoc != NULL))
    {
        vx_parameter params = vxGetGraphParameterByIndex(graph, 5);
        minCount = (vx_scalar)(params->node->paramTable[5]);
        vxReleaseParameter(&params);
    }

    if((maxCount == NULL) && (maxLoc != NULL))
    {
        vx_parameter params = vxGetGraphParameterByIndex(graph, 6);
        maxCount = (vx_scalar)(params->node->paramTable[6]);
        vxReleaseParameter(&params);
    }

    vxAccessScalarValue(minCount, &min_count);
    vxCommitScalarValue(minCount, &min_count);

    vxAccessScalarValue(maxCount, &max_count);
    vxCommitScalarValue(maxCount, &max_count);

    vxQueryArray(minLoc, VX_ARRAY_ATTRIBUTE_CAPACITY, &min_capacity, sizeof(vx_uint32));
    vxQueryArray(maxLoc, VX_ARRAY_ATTRIBUTE_CAPACITY, &max_capacity, sizeof(vx_uint32));

    if(min_count > min_capacity)min_count = min_capacity;
    if(max_count > max_capacity)max_count = max_capacity;

    vxSetArrayAttribute(minLoc, VX_ARRAY_ATTRIBUTE_NUMITEMS, &min_count, sizeof(vx_uint32));
    vxSetArrayAttribute(maxLoc, VX_ARRAY_ATTRIBUTE_NUMITEMS, &max_count, sizeof(vx_uint32));

    if((parameters[5] == NULL) && (minLoc != NULL))
    {
        vxReleaseScalar(&minCount);
    }

    if((parameters[6] == NULL) && (maxLoc != NULL))
    {
        vxReleaseScalar(&maxCount);
    }
    return status;
}

VX_PRIVATE_API vx_status vxoProgrameMinMaxLoc_Initializer(vx_node node, vx_reference *parameters, vx_uint32 num)
{

   vx_status status = VX_SUCCESS;
    vx_image inputImage;
    vx_scalar minVal;
    vx_scalar maxVal;
    vx_array minLoc;
    vx_array maxLoc;
    vx_scalar minCount;
    vx_scalar maxCount;
    vx_kernel kernel_minmax, kernel_location;
    vx_node node_minmax, node_location;
    vx_border_mode_t border;
    vx_context context = vxGetContext((vx_reference)node);
    vx_graph graph = vxCreateGraph(context);
    vx_uint32 min = 0xff, max = 0;
    vx_uint32 count = 0;
    vx_df_image format;
    vx_bool minc_opt = (vx_bool)(parameters[5] != NULL), maxc_opt = (vx_bool)(parameters[6] != NULL);
    vx_bool minl_opt = (vx_bool)(parameters[3] != NULL), maxl_opt = (vx_bool)(parameters[4] != NULL);

    inputImage = (vx_image)parameters[0];
    minVal     = (vx_scalar)parameters[1];
    maxVal     = (vx_scalar)parameters[2];
    minLoc     = (vx_array)parameters[3];
    maxLoc     = (vx_array)parameters[4];
    minCount   = (vx_scalar)parameters[5];
    maxCount   = (vx_scalar)parameters[6];

    if(!minc_opt && minl_opt)
    {
        minCount = vxCreateScalar(context, VX_TYPE_UINT32, &count);
    }
    else
    {
        vxAccessScalarValue(minCount, &count);
        count = 0;
        vxCommitScalarValue(minCount, &count);
    }

    if(!maxc_opt && maxl_opt)
    {
        maxCount = vxCreateScalar(context, VX_TYPE_UINT32, &count);
    }
    else
    {
        vxAccessScalarValue(maxCount, &count);
        count = 0;
        vxCommitScalarValue(maxCount, &count);
    }

    border.mode = VX_BORDER_MODE_REPLICATE;

    vxQueryImage(inputImage, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(vx_df_image));

    vxAccessScalarValue(minVal, &min);
    vxAccessScalarValue(maxVal, &max);

    min = 0xff;
    max = 0;

    if(format == VX_DF_IMAGE_S16)min = 0x7fff;
    else if(format == VX_DF_IMAGE_U16)min = 0xffff;

    vxCommitScalarValue(minVal, &min);
    vxCommitScalarValue(maxVal, &max);

    /* get min/max value */
    kernel_minmax = vxGetKernelByEnum(context, VX_KERNEL_EXTENSION_MINMAX);
    node_minmax = vxCreateGenericNode(graph, kernel_minmax);
    status |= vxSetNodeAttribute(node_minmax, VX_NODE_ATTRIBUTE_BORDER_MODE, &border, sizeof(border));

    status |= vxSetParameterByIndex(node_minmax, 0, (vx_reference)inputImage);
    status |= vxSetParameterByIndex(node_minmax, 1, (vx_reference)minVal);
    status |= vxSetParameterByIndex(node_minmax, 2, (vx_reference)maxVal);

    status |= vxoAddParameterToGraphByIndex(graph, node_minmax, 0);
    status |= vxoAddParameterToGraphByIndex(graph, node_minmax, 1);
    status |= vxoAddParameterToGraphByIndex(graph, node_minmax, 2);

    if(minc_opt || maxc_opt || minl_opt || maxl_opt)
    {
        kernel_location = vxGetKernelByEnum(context, VX_KERNEL_EXTENSION_MINMAXLOCATION);
        node_location = vxCreateGenericNode(graph, kernel_location);

        /* push parameters to nodes */
        status |= vxSetParameterByIndex(node_location, 0, (vx_reference)inputImage);
        status |= vxSetParameterByIndex(node_location, 1, (vx_reference)minVal);
        status |= vxSetParameterByIndex(node_location, 2, (vx_reference)maxVal);
        status |= vxSetParameterByIndex(node_location, 3, (vx_reference)minLoc);
        status |= vxSetParameterByIndex(node_location, 4, (vx_reference)maxLoc);
        status |= vxSetParameterByIndex(node_location, 5, (vx_reference)minCount);
        status |= vxSetParameterByIndex(node_location, 6, (vx_reference)maxCount);

        status |= vxoAddParameterToGraphByIndex(graph, node_location, 3);
        status |= vxoAddParameterToGraphByIndex(graph, node_location, 4);
        status |= vxoAddParameterToGraphByIndex(graph, node_location, 5);
        status |= vxoAddParameterToGraphByIndex(graph, node_location, 6);
    }

    status |= vxSetChildGraphOfNode(node, graph);
    if(status == VX_SUCCESS)
    {
        status |= vxVerifyGraph(graph);
    }
    else
    {
        status |= vxReleaseGraph(&graph);
    }

    status |= vxReleaseKernel(&kernel_minmax);
    status |= vxReleaseNode(&node_minmax);

    if(minc_opt || maxc_opt || minl_opt || maxl_opt)
    {
        status |= vxReleaseKernel(&kernel_location);
        status |= vxReleaseNode(&node_location);
    }
    return status;
}

VX_PRIVATE_API vx_status vxoProgrameMinMaxLoc_Deinitializer(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_graph graph = VX_NULL;

    graph = vxGetChildGraphOfNode(node);

    vxReleaseGraph(&graph);

    vxSetChildGraphOfNode(node, 0);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoAbsDiff_Initialize(vx_node node, vx_reference *parameters, vx_uint32 num)
{
                                                /*workdim,    globel offset,    globel scale    local size,    globel size,*/
    vx_kernel_execution_parameters_t shaderParam = {2,        {0, 0, 0},        {16, 1, 0},        {0, 0, 0},    {0, 0, 0}};
    vx_uint32 width = 0, height = 0;
    vx_image src = (vx_image)parameters[0];

    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(vx_uint32));

    shaderParam.globalWorkSize[0] = gcmALIGN_NP2(width, shaderParam.globalWorkScale[0])/shaderParam.globalWorkScale[0];
    shaderParam.globalWorkSize[1] = gcmALIGN_NP2(height, shaderParam.globalWorkScale[1])/shaderParam.globalWorkScale[1];

    vxSetNodeAttribute(node, VX_NODE_ATTRIBUTE_KERNEL_EXECUTION_PARAMETERS, &shaderParam, sizeof(vx_kernel_execution_parameters_t));

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoMinMax_Initialize(vx_node node, vx_reference *parameters, vx_uint32 num)
{
                                    /*workdim,    globel offset,    globel scale    local size,    globel size,*/
    vx_kernel_execution_parameters_t shaderParam = {2,        {0, 0, 0},        {1, 1, 0},        {1, 1, 0},        {0, 0, 0}};
    vx_uint32 width = 0, height = 0;
    vx_image src = (vx_image)parameters[0];

    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(vx_uint32));


    if(node->kernel->enumeration == VX_KERNEL_EXTENSION_MINMAX)shaderParam.globalWorkScale[0] = 8;

    shaderParam.globalWorkSize[0] = gcmALIGN_NP2(width, shaderParam.globalWorkScale[0])/shaderParam.globalWorkScale[0];
    shaderParam.globalWorkSize[1] = gcmALIGN_NP2(height, shaderParam.globalWorkScale[1])/shaderParam.globalWorkScale[1];

    vxSetNodeAttribute(node, VX_NODE_ATTRIBUTE_KERNEL_EXECUTION_PARAMETERS, &shaderParam, sizeof(vx_kernel_execution_parameters_t));

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoLUT_Initialize(vx_node node, vx_reference *parameters, vx_uint32 num)
{
                                    /*workdim,    globel offset,    globel scale    local size,    globel size,*/
    vx_kernel_execution_parameters_t shaderParam = {2,        {0, 0, 0},        {1, 1, 0},        {1, 1, 0},        {0, 0, 0}};
    vx_uint32 width = 0, height = 0;
    vx_image src = (vx_image)parameters[0];

    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(vx_uint32));

    shaderParam.globalWorkSize[0] = gcmALIGN_NP2(width, shaderParam.globalWorkScale[0])/shaderParam.globalWorkScale[0];
    shaderParam.globalWorkSize[1] = gcmALIGN_NP2(height, shaderParam.globalWorkScale[1])/shaderParam.globalWorkScale[1];

    if(node->kernel->enumeration == VX_KERNEL_EXTENSION_MINMAX)shaderParam.globalWorkScale[0] = 4;

    vxSetNodeAttribute(node, VX_NODE_ATTRIBUTE_KERNEL_EXECUTION_PARAMETERS, &shaderParam, sizeof(vx_kernel_execution_parameters_t));

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoMinMax_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData;

    if (index != 0) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if ((objData.u.imageInfo.format != VX_DF_IMAGE_U8) &&
        (objData.u.imageInfo.format != VX_DF_IMAGE_U16) &&
        (objData.u.imageInfo.format != VX_DF_IMAGE_S16) &&
        (objData.u.imageInfo.format != VX_DF_IMAGE_U32) &&
        (objData.u.imageInfo.format != VX_DF_IMAGE_S32))
    {
        return VX_ERROR_INVALID_FORMAT;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoMinMax_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    vx_object_data_s objData;
    vx_enum          type = VX_TYPE_INVALID;

    switch(index)
    {
        case 1:
        case 2:
            if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
                return VX_ERROR_INVALID_PARAMETERS;

            switch (objData.u.imageInfo.format)
            {
                case VX_DF_IMAGE_U8:
                    type = VX_TYPE_UINT8;
                    break;
                case VX_DF_IMAGE_U16:
                    type = VX_TYPE_UINT16;
                    break;
                case VX_DF_IMAGE_U32:
                    type = VX_TYPE_UINT32;
                    break;
                case VX_DF_IMAGE_S16:
                    type = VX_TYPE_INT16;
                    break;
                case VX_DF_IMAGE_S32:
                    type = VX_TYPE_INT32;
                    break;
                default:
                   return VX_ERROR_INVALID_TYPE;
            }

            ptr->type = VX_TYPE_SCALAR;
            ptr->u.scalarInfo.type = type;
            break;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoMinMaxLocation_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData;

    if ((index != 0) && (index != 1)  && (index != 2) ) return VX_ERROR_INVALID_PARAMETERS;

    if(index == 0)
    {
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if ((objData.u.imageInfo.format != VX_DF_IMAGE_U8) &&
            (objData.u.imageInfo.format != VX_DF_IMAGE_U16) &&
            (objData.u.imageInfo.format != VX_DF_IMAGE_S16) &&
            (objData.u.imageInfo.format != VX_DF_IMAGE_U32) &&
            (objData.u.imageInfo.format != VX_DF_IMAGE_S32))
        {
            return VX_ERROR_INVALID_FORMAT;
        }
    }
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoMinMaxLocation_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    switch(index)
    {
        case 3:
        case 4:
            ptr->u.arrayInfo.itemType = VX_TYPE_COORDINATES2D;
            ptr->u.arrayInfo.capacity = 1;
            break;

        case 5:
        case 6:
            ptr->u.scalarInfo.type = VX_TYPE_UINT32;
            break;
        default:
            return VX_ERROR_INVALID_PARAMETERS;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoMorphology_Initilize(vx_node node, vx_reference *parameters, vx_uint32 num)
{
                                    /*workdim,    globel offset,    globel scale    local size,    globel size,*/
    vx_kernel_execution_parameters_t shaderParam = {0};
    vx_uint32 width = 0, height = 0;
    vx_image src = (vx_image)parameters[0];

    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(vx_uint32));

    shaderParam.workDim = 2;
    shaderParam.globalWorkScale[0] = 14;
    shaderParam.globalWorkScale[1] = height;

    shaderParam.globalWorkSize[0] = gcmALIGN_NP2(width, shaderParam.globalWorkScale[0])/shaderParam.globalWorkScale[0];
    shaderParam.globalWorkSize[1] = gcmALIGN_NP2(height, shaderParam.globalWorkScale[1])/shaderParam.globalWorkScale[1];

    vxSetNodeUniform(node, "height", 1, &height);

    vxSetNodeAttribute(node, VX_NODE_ATTRIBUTE_KERNEL_EXECUTION_PARAMETERS, &shaderParam, sizeof(vx_kernel_execution_parameters_t));

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoSobel3x3_Initilize(vx_node node, vx_reference *parameters, vx_uint32 num)
{
                                    /*workdim,    globel offset,    globel scale    local size,    globel size,*/
    vx_kernel_execution_parameters_t shaderParam = {2,     {0, 0, 0},      {1, 1, 0},      {1, 1, 0},  {0, 0, 0},  };
    vx_uint32 width = 0, height = 0;
    vx_image src = (vx_image)parameters[0];

    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(vx_uint32));

    shaderParam.workDim = 2;
    shaderParam.globalWorkScale[0] = 6;
    shaderParam.globalWorkScale[1] = height;

    shaderParam.globalWorkSize[0] = gcmALIGN_NP2(width, shaderParam.globalWorkScale[0])/shaderParam.globalWorkScale[0];
    shaderParam.globalWorkSize[1] = gcmALIGN_NP2(height, shaderParam.globalWorkScale[1])/shaderParam.globalWorkScale[1];

    vxSetNodeUniform(node, "height", 1, &height);

    vxSetNodeAttribute(node, VX_NODE_ATTRIBUTE_KERNEL_EXECUTION_PARAMETERS, &shaderParam, sizeof(vx_kernel_execution_parameters_t));

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoBox3x3_Initilize(vx_node node, vx_reference *parameters, vx_uint32 num)
{
                                    /*workdim,    globel offset,    globel scale    local size,    globel size,*/
    vx_kernel_execution_parameters_t shaderParam = {0};
    vx_uint32 width = 0, height = 0;
    vx_image src = (vx_image)parameters[0];

    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(vx_uint32));

    shaderParam.workDim = 2;
    shaderParam.globalWorkScale[0] = 4;
    shaderParam.globalWorkScale[1] = height;

    shaderParam.globalWorkSize[0] = gcmALIGN_NP2(width, shaderParam.globalWorkScale[0])/shaderParam.globalWorkScale[0];
    shaderParam.globalWorkSize[1] = gcmALIGN_NP2(height, shaderParam.globalWorkScale[1])/shaderParam.globalWorkScale[1];

    vxSetNodeUniform(node, "height", 1, &height);

    vxSetNodeAttribute(node, VX_NODE_ATTRIBUTE_KERNEL_EXECUTION_PARAMETERS, &shaderParam, sizeof(vx_kernel_execution_parameters_t));

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoMultiply_Initilize(vx_node node, vx_reference *parameters, vx_uint32 num)
{
                                    /*workdim,    globel offset,    globel scale    local size,    globel size,*/
    vx_kernel_execution_parameters_t shaderParam = {0};
    vx_uint32 width = 0, height = 0;
    vx_image src = (vx_image)parameters[0];

    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(vx_uint32));

    shaderParam.workDim = 2;
    shaderParam.globalWorkScale[0] = 8;
    shaderParam.globalWorkScale[1] = height;

    shaderParam.globalWorkSize[0] = gcmALIGN_NP2(width, shaderParam.globalWorkScale[0])/shaderParam.globalWorkScale[0];
    shaderParam.globalWorkSize[1] = gcmALIGN_NP2(height, shaderParam.globalWorkScale[1])/shaderParam.globalWorkScale[1];

    vxSetNodeAttribute(node, VX_NODE_ATTRIBUTE_KERNEL_EXECUTION_PARAMETERS, &shaderParam, sizeof(vx_kernel_execution_parameters_t));

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoRemap_Initilize(vx_node node, vx_reference *parameters, vx_uint32 num)
{
                                    /*workdim,    globel offset,    globel scale    local size,    globel size,*/
    vx_kernel_execution_parameters_t shaderParam = {2,        {0, 0, 0},        {1, 1, 0},        {1, 1, 0},    {0, 0, 0},};
    vx_uint32 width = 0, height = 0;
    vx_image src = (vx_image)parameters[0];

    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(vx_uint32));

    shaderParam.workDim = 2;
    shaderParam.globalWorkScale[0] = 1;
    shaderParam.globalWorkScale[1] = 1;

    shaderParam.localWorkSize[0] = 1;
    shaderParam.localWorkSize[1] = 1;

    shaderParam.globalWorkSize[0] = gcmALIGN_NP2(width, shaderParam.globalWorkScale[0])/shaderParam.globalWorkScale[0];
    shaderParam.globalWorkSize[1] = gcmALIGN_NP2(height, shaderParam.globalWorkScale[1])/shaderParam.globalWorkScale[1];

    vxSetNodeAttribute(node, VX_NODE_ATTRIBUTE_KERNEL_EXECUTION_PARAMETERS, &shaderParam, sizeof(vx_kernel_execution_parameters_t));

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoHistogram_Initilize(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
                                    /*workdim,    globel offset,    globel scale    local size,    globel size,*/
    vx_kernel_execution_parameters_t shaderParam = {2,        {0, 0, 0},        {1, 1, 0},        {1, 1, 0},    {0, 0, 0},};
    vx_uint32 width = 0, height = 0;
    vx_image src = (vx_image)parameters[0];
    vx_distribution distribution = (vx_distribution)parameters[1];
    vx_uint32* dist_src = NULL;
    vx_size size = 0;

    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(vx_uint32));

    shaderParam.workDim = 2;
    shaderParam.globalWorkScale[0] = 1;
    shaderParam.globalWorkScale[1] = 1;

    shaderParam.localWorkSize[0] = 1;
    shaderParam.localWorkSize[1] = 1;

    shaderParam.globalWorkSize[0] = gcmALIGN_NP2(width, shaderParam.globalWorkScale[0])/shaderParam.globalWorkScale[0];
    shaderParam.globalWorkSize[1] = gcmALIGN_NP2(height, shaderParam.globalWorkScale[1])/shaderParam.globalWorkScale[1];

    status |= vxSetNodeAttribute(node, VX_NODE_ATTRIBUTE_KERNEL_EXECUTION_PARAMETERS, &shaderParam, sizeof(vx_kernel_execution_parameters_t));


    status |= vxAccessDistribution(distribution, (void **)&dist_src, VX_WRITE_ONLY);
    status |= vxQueryDistribution(distribution, VX_DISTRIBUTION_ATTRIBUTE_SIZE, &size, sizeof(size));
    gcoOS_ZeroMemory(dist_src, size);
    status |= vxCommitDistribution(distribution, dist_src);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status _getUniformForConvolve(vx_convolution convolution, vx_512bits_t *uniforms)
{
    vx_status status = VX_SUCCESS;
    vx_512bits_t *uniform;
    vx_int16 matrix[C_MAX_CONVOLUTION_DIM * C_MAX_CONVOLUTION_DIM] = {0};
    vx_uint32 i = 0;
    vx_size conv_width, conv_height;
    vx_size scale = 1;

    status |= vxQueryConvolution(convolution, VX_CONVOLUTION_ATTRIBUTE_COLUMNS, &conv_width, sizeof(conv_width));
    status |= vxQueryConvolution(convolution, VX_CONVOLUTION_ATTRIBUTE_ROWS, &conv_height, sizeof(conv_height));
    status |= vxQueryConvolution(convolution, VX_CONVOLUTION_ATTRIBUTE_SCALE, &scale, sizeof(scale));

    status |= vxAccessConvolutionCoefficients(convolution, matrix);

    /* Uniform512 for the first DP */
    uniform = &uniforms[0];

    uniform->termConfig.bin2.flag0 = VX_512BITS_ADD;
    uniform->termConfig.bin2.flag1 = VX_512BITS_ADD;
    uniform->termConfig.bin2.flag2 = VX_512BITS_ADD;
    uniform->termConfig.bin2.flag3 = VX_512BITS_ADD;
    uniform->termConfig.bin2.flag4 = VX_512BITS_ADD;
    uniform->termConfig.bin2.flag5 = VX_512BITS_ADD;

    uniform->termConfig.bin2.flag8  = VX_512BITS_ADD;
    uniform->termConfig.bin2.flag9  = VX_512BITS_ADD;
    uniform->termConfig.bin2.flag10 = VX_512BITS_ADD;
    uniform->termConfig.bin2.flag11 = VX_512BITS_ADD;
    uniform->termConfig.bin2.flag12 = VX_512BITS_ADD;
    uniform->termConfig.bin2.flag13 = VX_512BITS_ADD;

    uniform->aSelect.bin2.flag0 = VX_512BITS_SELECT_SRC0;
    uniform->aSelect.bin2.flag1 = VX_512BITS_SELECT_SRC0;
    uniform->aSelect.bin2.flag2 = VX_512BITS_SELECT_SRC0;
    uniform->aSelect.bin2.flag3 = VX_512BITS_SELECT_SRC1;
    uniform->aSelect.bin2.flag4 = VX_512BITS_SELECT_SRC1;
    uniform->aSelect.bin2.flag5 = VX_512BITS_SELECT_SRC1;

    uniform->aSelect.bin2.flag8  = VX_512BITS_SELECT_SRC0;
    uniform->aSelect.bin2.flag9  = VX_512BITS_SELECT_SRC0;
    uniform->aSelect.bin2.flag10 = VX_512BITS_SELECT_SRC0;
    uniform->aSelect.bin2.flag11 = VX_512BITS_SELECT_SRC1;
    uniform->aSelect.bin2.flag12 = VX_512BITS_SELECT_SRC1;
    uniform->aSelect.bin2.flag13 = VX_512BITS_SELECT_SRC1;

    uniform->aBin[0].bin4.flag0 = 0;
    uniform->aBin[0].bin4.flag1 = 1;
    uniform->aBin[0].bin4.flag2 = 2;
    uniform->aBin[0].bin4.flag3 = 0;
    uniform->aBin[0].bin4.flag4 = 1;
    uniform->aBin[0].bin4.flag5 = 2;

    uniform->aBin[1].bin4.flag0 = 0 + 1;
    uniform->aBin[1].bin4.flag1 = 1 + 1;
    uniform->aBin[1].bin4.flag2 = 2 + 1;
    uniform->aBin[1].bin4.flag3 = 0 + 1;
    uniform->aBin[1].bin4.flag4 = 1 + 1;
    uniform->aBin[1].bin4.flag5 = 2 + 1;

    uniform->bSelect.bin2.flag0 = VX_512BITS_SELECT_CONSTANTS;
    uniform->bSelect.bin2.flag1 = VX_512BITS_SELECT_CONSTANTS;
    uniform->bSelect.bin2.flag2 = VX_512BITS_SELECT_CONSTANTS;
    uniform->bSelect.bin2.flag3 = VX_512BITS_SELECT_CONSTANTS;
    uniform->bSelect.bin2.flag4 = VX_512BITS_SELECT_CONSTANTS;
    uniform->bSelect.bin2.flag5 = VX_512BITS_SELECT_CONSTANTS;

    uniform->bSelect.bin2.flag8  = VX_512BITS_SELECT_CONSTANTS;
    uniform->bSelect.bin2.flag9  = VX_512BITS_SELECT_CONSTANTS;
    uniform->bSelect.bin2.flag10 = VX_512BITS_SELECT_CONSTANTS;
    uniform->bSelect.bin2.flag11 = VX_512BITS_SELECT_CONSTANTS;
    uniform->bSelect.bin2.flag12 = VX_512BITS_SELECT_CONSTANTS;
    uniform->bSelect.bin2.flag13 = VX_512BITS_SELECT_CONSTANTS;

    uniform->miscConfig.post_shift        = (gctUINT32)gcoMATH_Log2((vx_float32)scale);
    uniform->miscConfig.constant_type    = VX_512BITS_TYPE_SIGNED16;
    uniform->miscConfig.accu_type        = VX_512BITS_TYPE_UNSIGNED32;

    for(i = 0; i < 16; i ++)
    {
        uniform->bins[0].bin16[i] = matrix[i];
    }

    /* Uniform512 for the second DP */
    uniform = &uniforms[1];

    uniform->termConfig.bin2.flag0 = VX_512BITS_ADD;
    uniform->termConfig.bin2.flag1 = VX_512BITS_ADD;
    uniform->termConfig.bin2.flag2 = VX_512BITS_ADD;
    uniform->termConfig.bin2.flag3 = VX_512BITS_ACCUMULATOR;

    uniform->termConfig.bin2.flag8  = VX_512BITS_ADD;
    uniform->termConfig.bin2.flag9  = VX_512BITS_ADD;
    uniform->termConfig.bin2.flag10 = VX_512BITS_ADD;
    uniform->termConfig.bin2.flag11 = VX_512BITS_ACCUMULATOR;

    uniform->aSelect.bin2.flag0 = VX_512BITS_SELECT_SRC0;
    uniform->aSelect.bin2.flag1 = VX_512BITS_SELECT_SRC0;
    uniform->aSelect.bin2.flag2 = VX_512BITS_SELECT_SRC0;
    uniform->aSelect.bin2.flag3 = VX_512BITS_SELECT_SRC1;

    uniform->aSelect.bin2.flag8  = VX_512BITS_SELECT_SRC0;
    uniform->aSelect.bin2.flag9  = VX_512BITS_SELECT_SRC0;
    uniform->aSelect.bin2.flag10 = VX_512BITS_SELECT_SRC0;
    uniform->aSelect.bin2.flag11 = VX_512BITS_SELECT_SRC1;

    uniform->aBin[0].bin4.flag0 = 0;
    uniform->aBin[0].bin4.flag1 = 1;
    uniform->aBin[0].bin4.flag2 = 2;
    uniform->aBin[0].bin4.flag3 = 2;

    uniform->aBin[1].bin4.flag0 = 0 + 1;
    uniform->aBin[1].bin4.flag1 = 1 + 1;
    uniform->aBin[1].bin4.flag2 = 2 + 1;
    uniform->aBin[1].bin4.flag3 = 2 + 1;

    uniform->bSelect.bin2.flag0 = VX_512BITS_SELECT_CONSTANTS;
    uniform->bSelect.bin2.flag1 = VX_512BITS_SELECT_CONSTANTS;
    uniform->bSelect.bin2.flag2 = VX_512BITS_SELECT_CONSTANTS;
    uniform->bSelect.bin2.flag3 = VX_512BITS_SELECT_CONSTANTS;
    uniform->bSelect.bin2.flag4 = VX_512BITS_SELECT_CONSTANTS;
    uniform->bSelect.bin2.flag5 = VX_512BITS_SELECT_CONSTANTS;

    uniform->bSelect.bin2.flag8  = VX_512BITS_SELECT_CONSTANTS;
    uniform->bSelect.bin2.flag9  = VX_512BITS_SELECT_CONSTANTS;
    uniform->bSelect.bin2.flag10 = VX_512BITS_SELECT_CONSTANTS;
    uniform->bSelect.bin2.flag11 = VX_512BITS_SELECT_CONSTANTS;
    uniform->bSelect.bin2.flag12 = VX_512BITS_SELECT_CONSTANTS;
    uniform->bSelect.bin2.flag13 = VX_512BITS_SELECT_CONSTANTS;

    uniform->miscConfig.post_shift        = (gctUINT32)gcoMATH_Log2((vx_float32)scale);
    uniform->miscConfig.constant_type    = VX_512BITS_TYPE_SIGNED16;
    uniform->miscConfig.accu_type        = VX_512BITS_TYPE_SIGNED32;

    for(i = 0; i < 16; i ++)
    {
        uniform->bins[0].bin16[i] = matrix[i];
    }

    status |= vxCommitConvolutionCoefficients(convolution, matrix);

    return status;
}

VX_PRIVATE_API vx_status vxoConvolve_Initilize(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
                                    /*workdim,    globel offset,    globel scale    local size,    globel size,*/
    vx_kernel_execution_parameters_t shaderParam = {2,        {0, 0, 0},        {1, 1, 0},        {1, 1, 0},    {0, 0, 0},};
    vx_uint32 width = 0, height = 0;
    vx_image src = (vx_image)parameters[0];

    vx_convolution convolution = (vx_convolution)parameters[1];
    vx_512bits_t uniforms[2];

    gcoOS_ZeroMemory(uniforms, sizeof(vx_512bits_t)*2);

    _getUniformForConvolve(convolution, uniforms);

    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(vx_uint32));

    shaderParam.workDim = 2;
    shaderParam.globalWorkScale[0] = 1;
    shaderParam.globalWorkScale[1] = 1;

    shaderParam.localWorkSize[0] = 1;
    shaderParam.localWorkSize[1] = 1;

    shaderParam.globalWorkSize[0] = gcmALIGN_NP2(width, shaderParam.globalWorkScale[0])/shaderParam.globalWorkScale[0];
    shaderParam.globalWorkSize[1] = gcmALIGN_NP2(height, shaderParam.globalWorkScale[1])/shaderParam.globalWorkScale[1];

    status |= vxSetNodeAttribute(node, VX_NODE_ATTRIBUTE_KERNEL_EXECUTION_PARAMETERS, &shaderParam, sizeof(vx_kernel_execution_parameters_t));

    status |= vxSetNodeUniform(node, "u512_0", 1, &uniforms[0]);
    status |= vxSetNodeUniform(node, "u512_1", 1, &uniforms[1]);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxInitializeTarget(
        vx_context context, vx_kernel_description_s *kernelDescTable[], vx_uint32 kernelCount)
{
    vx_uint32   index, i;

    vxmASSERT(context);
    vxmASSERT(kernelDescTable);
    vxmASSERT(kernelCount > 0);

    gcoOS_StrCopySafe(context->targetTable[0].name, VX_MAX_TARGET_NAME, VX_DEFAULT_TARGET_NAME);

    for (index = 0; index < kernelCount; index++)
    {
        vx_status status = VX_SUCCESS;
        vx_kernel kernel = vxGetKernelByEnum(context, kernelDescTable[index]->enumeration);

        if (kernel != NULL && kernel->base.type != VX_TYPE_ERROR)
        {
            kernel->enabled = vx_false_e;
            vxRemoveKernel(kernel);
        }

        if(kernelDescTable[index]->extension.source != NULL)
        {
            vx_program program = 0;
            vx_char** source = &kernelDescTable[index]->extension.source;
            vx_size programLength = strlen(*source);

            /* Load the souce from file, if source is path */
            if(!strchr(*source, '#'))
                *source = gcoVX_LoadSources(*source, &programLength);

            program = vxCreateProgramWithSource(context, 1, (const vx_char**)source, &programLength);

            vxBuildProgram(program, "-cl-viv-vx-extension");

            kernel = vxAddKernelInProgram(
                                    program,
                                    kernelDescTable[index]->name,
                                    kernelDescTable[index]->enumeration,
                                    kernelDescTable[index]->numParams,
                                    kernelDescTable[index]->inputValidate,
                                    kernelDescTable[index]->outputValidate,
                                    kernelDescTable[index]->initialize,
                                    kernelDescTable[index]->deinitialize
                                    );

        }
        else
        {
            kernel = vxAddKernel(context,
                                kernelDescTable[index]->name,
                                kernelDescTable[index]->enumeration,
                                kernelDescTable[index]->function,
                                kernelDescTable[index]->numParams,
                                kernelDescTable[index]->inputValidate,
                                kernelDescTable[index]->outputValidate,
                                kernelDescTable[index]->initialize,
                                kernelDescTable[index]->deinitialize
                                );
        }

        if (status != VX_SUCCESS) return status;

        for(i = 0; i < kernelDescTable[index]->numParams; i++)
        {
            vx_param_description_s * parameters = &(kernelDescTable[index]->parameters[i]);
            vxAddParameterToKernel(kernel, i, parameters->direction, parameters->dataType, parameters->state);
        }

        vxFinalizeKernel(kernel);

        if (vxoKernel_IsUnique(&context->targetTable[0].kernelTable[context->kernelCount - 1])) context->uniqueKernelCount++;
    }

    /* ToDo : Add more specific return status check */
    if (gcoVX_Initialize(&context->evisNoInst) != gcvSTATUS_OK)
    {
        return VX_FAILURE;
    }

    return VX_SUCCESS;
}

vx_kernel_description_s *target_program_kernels[] = {
    &programkernel_absdiff,
    &programkernel_multiply,
    &programkernel_remap,
    &programkernel_warp_affine,
    &programkernel_histogram,
    &programkernel_threshold,

    &programkernel_dilate3x3,
    &programkernel_erode3x3,
    &programkernel_median3x3,
    &programkernel_sobel3x3,
    &programkernel_gaussian3x3,
    &programkernel_box3x3,
    &programkernel_box3x3_2,

    &programkernel_minmaxloc,
    &programkernel_minmax,
    &programkernel_minmaxlocation,
    &programkernel_lut,
    &programkernel_convolution,
};

vx_uint32 num_target_program_kernels = vxmLENGTH_OF(target_program_kernels);

VX_PUBLIC_API vx_status vxPublishKernels(vx_context context)
{
    vx_status status = vxInitializeTarget(context,
                                        target_program_kernels,
                                        num_target_program_kernels
                                        );

    return status;
}
