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


#include <VX/vx.h>

#include <gc_vx_common.h>
#include <gc_vx_interface.h>
#include <gc_vx_internal_node_api.h>

typedef struct _vx_object_data
{
    vx_enum                                 objType;

    union
    {
        struct
        {
            vx_uint32                       width;
            vx_uint32                       height;
            vx_df_image                     format;
        }
        imageInfo;

        struct
        {
            vx_enum                         dataType;
            void *                          scalarValuePtr;
        }
        scalarInfo;

        struct
        {
            vx_enum                         dataType;
        }
        lutArrayInfo;

        struct
        {
            vx_size                         numBins;
        }
        distributionInfo;

        struct
        {
            vx_enum                         dataType;
        }
        thresholdInfo;

        struct
        {
            vx_size                         rows;
            vx_size                         columns;
        }
        convolutionInfo;

        struct
        {
            vx_enum                         dataType;
            vx_size                         rows;
            vx_size                         columns;
        }
        matrixInfo;

        struct
        {
            vx_uint32                       srcWidth;
            vx_uint32                       srcHeight;
            vx_uint32                       dstWidth;
            vx_uint32                       dstHeight;
        }
        remapInfo;

        struct
        {
            vx_size                         numLevels;
            vx_float32                      scale;
        }
        pyramidInfo;

        struct
        {
            vx_enum                         dataType;
            vx_size                         capacity;
        }
        arrayInfo;
    }
    u;
}
vx_object_data_s;

const vx_size gaussian5x5scale = 256;
const vx_int16 gaussian5x5[5][5] =
{
    {1,  4,  6,  4, 1},
    {4, 16, 24, 16, 4},
    {6, 24, 36, 24, 6},
    {4, 16, 24, 16, 4},
    {1,  4,  6,  4, 1}
};

void vxoFillMetaData(vx_meta_format_s *ptr, vx_enum type, vx_df_image format, vx_uint32 width, vx_uint32 height, vx_enum dataInfoType)
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

vx_status vxoGetObjAttributeByNodeIndex(vx_node node, vx_uint32 index, vx_enum type, vx_object_data_s* objData)
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

vx_status vxoAddParameterToGraphByIndex(vx_graph graph, vx_node node, vx_uint32 index)
{
    vx_parameter param;
    vx_status    status;

    param = vxGetParameterByIndex(node, index);

    status = vxAddParameterToGraph(graph, param);

    vxReleaseParameter(&param);

    return status;
}

/*functions for invalid kernel*/
VX_PRIVATE_API vx_status vxoBaseKernel_Invalid(vx_node node, vx_reference paramTable[], vx_uint32 num)
{
    return VX_ERROR_NOT_SUPPORTED;
}

VX_PRIVATE_API vx_status vxoInvalid_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_ERROR_INVALID_PARAMETERS;
}

VX_PRIVATE_API vx_status vxoInvalid_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *info)
{
    return VX_ERROR_INVALID_PARAMETERS;
}

/*functions for colorConvert kernel*/
VX_PRIVATE_API vx_status vxoBaseKernel_ColorConvert(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_image srcImg;
    vx_image dstImg;

    if (num != 2) return VX_ERROR_INVALID_PARAMETERS;

    srcImg = (vx_image)parameters[0];
    dstImg = (vx_image)parameters[1];

    return vxConvertColor(srcImg, dstImg);
}

VX_PRIVATE_API vx_status vxoColorConvert_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_status        status = VX_SUCCESS;
    vx_object_data_s objData = {0};

    if (index != 0) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    switch (objData.u.imageInfo.format)
    {
        case VX_DF_IMAGE_RGB:
        case VX_DF_IMAGE_RGBX:
        case VX_DF_IMAGE_NV12:
        case VX_DF_IMAGE_NV21:
        case VX_DF_IMAGE_IYUV:
            if (objData.u.imageInfo.height & 1)
            {
                status =  VX_ERROR_INVALID_DIMENSION;
                break;
            }
        case VX_DF_IMAGE_YUYV:
        case VX_DF_IMAGE_UYVY:
            if (objData.u.imageInfo.width & 1)
            {
                status = VX_ERROR_INVALID_DIMENSION;
                break;
            }
            break;
        default:
            status = VX_ERROR_INVALID_FORMAT;
            break;
    }

    return status;
}

VX_PRIVATE_API vx_status vxoColorConvert_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    vx_status    status = VX_ERROR_INVALID_PARAMETERS;
    vx_object_data_s srcObjData = {0};
    vx_object_data_s dstObjData = {0};
    vx_uint32    i = 0;

    if (index != 1) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &srcObjData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 1, VX_TYPE_IMAGE, &dstObjData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    for (i = 0; i < vxmLENGTH_OF(colorConvert_InputOutputFormat); i++)
    {
        if ((srcObjData.u.imageInfo.format == colorConvert_InputOutputFormat[i][0]) &&
            (dstObjData.u.imageInfo.format == colorConvert_InputOutputFormat[i][1]))
        {
            vxoFillMetaData(ptr, VX_TYPE_IMAGE, dstObjData.u.imageInfo.format, srcObjData.u.imageInfo.width, srcObjData.u.imageInfo.height, 0);
            status = VX_SUCCESS;
            break;
        }
    }

    return status;
}

VX_PRIVATE_API vx_status vxoBaseKernel_ChannelExtract(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_image  srcImg;
    vx_scalar channel;
    vx_image  dstImg;

    if (num != 3) return VX_ERROR_INVALID_PARAMETERS;

    srcImg   = (vx_image)parameters[0];
    channel  = (vx_scalar)parameters[1];
    dstImg   = (vx_image)parameters[2];

    return vxChannelExtract(srcImg, channel, dstImg);
}

VX_PRIVATE_API vx_status vxoChannelExtract_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_status        status = VX_SUCCESS;
    vx_object_data_s objData0 = {0};
    vx_object_data_s objData1 = {0};

    if (index != 0 && index != 1) return VX_ERROR_INVALID_PARAMETERS;

    if (index == 0)
    {
        if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData0) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        switch (objData0.u.imageInfo.format)
        {
            case VX_DF_IMAGE_RGB:
            case VX_DF_IMAGE_RGBX:
            case VX_DF_IMAGE_YUV4:
                status = VX_SUCCESS;
                break;
            case VX_DF_IMAGE_NV12:
            case VX_DF_IMAGE_NV21:
            case VX_DF_IMAGE_IYUV:
                if (objData0.u.imageInfo.width % 2 != 0 || objData0.u.imageInfo.height % 2 != 0)
                    status = VX_ERROR_INVALID_DIMENSION;
                else
                    status = VX_SUCCESS;
                break;
            case VX_DF_IMAGE_UYVY:
            case VX_DF_IMAGE_YUYV:
                if (objData0.u.imageInfo.width % 2 != 0)
                    status = VX_ERROR_INVALID_DIMENSION;
                else
                    status = VX_SUCCESS;
                break;
            default:
                status = VX_ERROR_INVALID_FORMAT;
                break;
        }
    }
    else if (index == 1)
    {
        vx_enum channel = 0;
        vx_enum max_channel = 0;
        objData1.u.scalarInfo.scalarValuePtr = &channel;

        if (vxoGetObjAttributeByNodeIndex(node, 1, VX_TYPE_SCALAR, &objData1) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData0) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        max_channel = (objData0.u.imageInfo.format == VX_DF_IMAGE_RGBX ? VX_CHANNEL_3 : VX_CHANNEL_2);

        if (VX_CHANNEL_0 <= channel && channel <= max_channel)
        {
            status = VX_SUCCESS;
        }
        else
        {
            status = VX_ERROR_INVALID_VALUE;
        }
    }

    return status;
}

VX_PRIVATE_API vx_status vxoChannelExtract_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    vx_enum          channel = 0;
    vx_object_data_s objData0 = {0};
    vx_object_data_s objData1 = {0};

    objData1.u.scalarInfo.scalarValuePtr = &channel;

    if (index != 2) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData0) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 1, VX_TYPE_SCALAR, &objData1) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if (channel != VX_CHANNEL_0)
    {
        if (objData0.u.imageInfo.format == VX_DF_IMAGE_IYUV ||
            objData0.u.imageInfo.format == VX_DF_IMAGE_NV12 ||
            objData0.u.imageInfo.format == VX_DF_IMAGE_NV21)
        {
            objData0.u.imageInfo.width /= 2;
            objData0.u.imageInfo.height /= 2;
        }
        else if (objData0.u.imageInfo.format == VX_DF_IMAGE_YUYV ||
                 objData0.u.imageInfo.format == VX_DF_IMAGE_UYVY)
        {
            objData0.u.imageInfo.width /= 2;
        }
    }

    vxoFillMetaData(ptr, VX_TYPE_IMAGE, VX_DF_IMAGE_U8, objData0.u.imageInfo.width, objData0.u.imageInfo.height, 0);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoBaseKernel_ChannelCombine(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_image inputImage[4];
    vx_image outputImage;
    vx_int32 i;

    if (num != 5)  return VX_ERROR_INVALID_PARAMETERS;

    for (i = 0; i < 4; i++)
    {
        inputImage[i] = (vx_image)parameters[i];
    }

    outputImage = (vx_image)parameters[4];

    return vxChannelCombine(inputImage, outputImage);
}

VX_PRIVATE_API vx_status vxoChannelCombine_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData = {0};

    if (index >= 4) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    return (objData.u.imageInfo.format == VX_DF_IMAGE_U8) ? VX_SUCCESS : VX_ERROR_INVALID_PARAMETERS;
}

VX_PRIVATE_API vx_status vxoChannelCombine_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    vx_uint32        i;
    vx_uint32        uv_x_scale           = 0;
    vx_uint32        uv_y_scale           = 0;
    vx_bool          isInputPlaneValid[4] = {vx_false_e, vx_false_e, vx_false_e, vx_false_e};
    vx_bool          isValid              = vx_false_e;
    vx_object_data_s objData[5];

    if (index != 4) return VX_ERROR_INVALID_PARAMETERS;

    for (i = 0; i < index; i++)
    {
        if (vxoGetObjAttributeByNodeIndex(node, i, VX_TYPE_IMAGE, &objData[i]) != VX_SUCCESS)
            continue;

        isInputPlaneValid[i] = vx_true_e;
    }

    if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData[4]) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if (objData[0].u.imageInfo.width != 0 && objData[0].u.imageInfo.height != 0)
    {
        uv_x_scale = (objData[0].u.imageInfo.width == objData[1].u.imageInfo.width) ? 1 : (objData[0].u.imageInfo.width == 2 * objData[1].u.imageInfo.width) ? 2 : 0;
        uv_y_scale = (objData[0].u.imageInfo.height == objData[1].u.imageInfo.height) ? 1 : (objData[0].u.imageInfo.height == 2 * objData[1].u.imageInfo.height) ? 2 : 0;
    }

    if (objData[4].u.imageInfo.format == VX_DF_IMAGE_RGB || objData[4].u.imageInfo.format == VX_DF_IMAGE_YUV4)
    {
        isValid = (vx_bool)(isInputPlaneValid[0] && isInputPlaneValid[1] && isInputPlaneValid[2] &&
                            (uv_y_scale == 1) && (uv_x_scale == 1));
    }
    else if (objData[4].u.imageInfo.format == VX_DF_IMAGE_RGBX)
    {
        isValid = (vx_bool)(isInputPlaneValid[0] && isInputPlaneValid[1] && isInputPlaneValid[2] && isInputPlaneValid[3] &&
                            (uv_y_scale == 1) && (uv_x_scale == 1));
    }
    else if (objData[4].u.imageInfo.format == VX_DF_IMAGE_YUYV || objData[4].u.imageInfo.format == VX_DF_IMAGE_UYVY)
    {
        isValid = (vx_bool)(isInputPlaneValid[0] && isInputPlaneValid[1] && isInputPlaneValid[2] &&
                            (uv_y_scale == 1) && (uv_x_scale == 2));
    }
    else if (objData[4].u.imageInfo.format == VX_DF_IMAGE_NV12 || objData[4].u.imageInfo.format == VX_DF_IMAGE_NV21 || objData[4].u.imageInfo.format == VX_DF_IMAGE_IYUV)
    {
        isValid = (vx_bool)(isInputPlaneValid[0] && isInputPlaneValid[1] && isInputPlaneValid[2] &&
                            (uv_y_scale == 2) && (uv_x_scale == 2));
    }

    if (!isValid) return VX_ERROR_INVALID_PARAMETERS;

    vxoFillMetaData(ptr, VX_TYPE_IMAGE, objData[4].u.imageInfo.format, objData[0].u.imageInfo.width, objData[0].u.imageInfo.height, 0);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoBaseKernel_Sobel3x3(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_border_mode_t bordermode;
    vx_image         input;
    vx_image         grad_x;
    vx_image         grad_y;

    if (num != 3) return VX_ERROR_INVALID_PARAMETERS;

    input  = (vx_image)parameters[0];
    grad_x = (vx_image)parameters[1];
    grad_y = (vx_image)parameters[2];

    if (vxQueryNode(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &bordermode, sizeof(bordermode)) == VX_SUCCESS)
    {
        return vxSobel3x3(input, grad_x, grad_y, &bordermode);
    }
    else
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }
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

    if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    vxoFillMetaData(ptr, VX_TYPE_IMAGE, VX_DF_IMAGE_S16, objData.u.imageInfo.width, objData.u.imageInfo.height, 0);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoBaseKernel_Magnitude(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_image grad_x;
    vx_image grad_y;
    vx_image output;

    if (num != 3) return VX_ERROR_INVALID_PARAMETERS;

    grad_x = (vx_image)parameters[0];
    grad_y = (vx_image)parameters[1];
    output = (vx_image)parameters[2];

    node->kernelAttributes.isAllGPU = vx_true_e;
    return vxMagnitude(grad_x, grad_y, output);
}

VX_PRIVATE_API vx_status vxoMagnitude_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData[2];

    if (index != 0 && index != 1) return VX_ERROR_INVALID_PARAMETERS;

    switch (index)
    {
    case 1:
        if (vxoGetObjAttributeByNodeIndex(node, 1, VX_TYPE_IMAGE, &objData[1]) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;
    case 0:
        if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData[0]) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;
        break;
    default:
        break;
    }

    if (objData[index].u.imageInfo.format != VX_DF_IMAGE_S16) return VX_ERROR_INVALID_PARAMETERS;

    if (index == 0)
    {
        return VX_SUCCESS;
    }
    else
    {
        if ((objData[0].u.imageInfo.width == objData[1].u.imageInfo.width) &&
            (objData[0].u.imageInfo.height == objData[1].u.imageInfo.height))
        {
            return VX_SUCCESS;
        }
        else
        {
            return VX_ERROR_INVALID_PARAMETERS;
        }
    }

}

VX_PRIVATE_API vx_status vxoMagnitude_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    vx_object_data_s objData0 = {0};
    vx_object_data_s objData2 = {0};

    if (index != 2) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData0) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 2, VX_TYPE_IMAGE, &objData2) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if (objData2.u.imageInfo.format != VX_DF_IMAGE_U8) objData2.u.imageInfo.format = VX_DF_IMAGE_S16;

    vxoFillMetaData(ptr, VX_TYPE_IMAGE, objData2.u.imageInfo.format, objData0.u.imageInfo.width, objData0.u.imageInfo.height, 0);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoBaseKernel_Phase(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_image grad_x;
    vx_image grad_y;
    vx_image output;

    if (num != 3) return VX_ERROR_INVALID_PARAMETERS;

    grad_x = (vx_image)parameters[0];
    grad_y = (vx_image)parameters[1];
    output = (vx_image)parameters[2];

    return vxPhase(grad_x, grad_y, output);
}

VX_PRIVATE_API vx_status vxoPhase_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData[2];

    if (index != 0 && index != 1) return VX_ERROR_INVALID_PARAMETERS;

    switch (index)
    {
    case 1:
        if (vxoGetObjAttributeByNodeIndex(node, 1, VX_TYPE_IMAGE, &objData[1]) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;
    case 0:
        if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData[0]) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;
        break;
    default:
        break;
    }

    if (objData[index].u.imageInfo.format != VX_DF_IMAGE_S16) return VX_ERROR_INVALID_PARAMETERS;

    if (index == 0)
    {
        return VX_SUCCESS;
    }
    else if ((objData[0].u.imageInfo.width == objData[1].u.imageInfo.width) &&
             (objData[0].u.imageInfo.height == objData[1].u.imageInfo.height))
    {
        return VX_SUCCESS;
    }
    else
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }
}

VX_PRIVATE_API vx_status vxoPhase_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    vx_object_data_s objData;

    if (index != 2) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    vxoFillMetaData(ptr, VX_TYPE_IMAGE, VX_DF_IMAGE_U8, objData.u.imageInfo.width, objData.u.imageInfo.height, 0);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoBaseKernel_TableLookup(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_image srcImage;
    vx_lut   lut;
    vx_image dstImage;

    if (num != 3) return VX_ERROR_INVALID_PARAMETERS;

    srcImage = (vx_image) parameters[0];
    lut      = (vx_lut)parameters[1];
    dstImage = (vx_image) parameters[2];

    return vxTableLookup(srcImage, lut, dstImage);
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

VX_PRIVATE_API vx_status vxoBaseKernel_ScaleImage(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_image         srcImage;
    vx_image         dstImage;
    vx_scalar        type;
    vx_border_mode_t borderMode;
    vx_float64 *     localDataPtr = VX_NULL;
    vx_size          size = 0ul;

    if (num != 3) return VX_ERROR_INVALID_PARAMETERS;

    srcImage                  = (vx_image) parameters[0];
    dstImage                  = (vx_image) parameters[1];
    type                      = (vx_scalar)parameters[2];
    borderMode.mode           = VX_BORDER_MODE_UNDEFINED;
    borderMode.constant_value = 0;

    vxQueryNode(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &borderMode, sizeof(borderMode));
    vxQueryNode(node, VX_NODE_ATTRIBUTE_LOCAL_DATA_PTR, &localDataPtr, sizeof(localDataPtr));
    vxQueryNode(node, VX_NODE_ATTRIBUTE_LOCAL_DATA_SIZE,&size, sizeof(size));

    node->kernelAttributes.isAllGPU = vx_true_e;
    return vxScaleImage(srcImage, dstImage, type, &borderMode, localDataPtr, size);
}

VX_PRIVATE_API vx_status vxoScaleImage_Initializer(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_size size = 1;

    if (num != 3) return VX_ERROR_INVALID_PARAMETERS;

    return vxSetNodeAttribute(node, VX_NODE_ATTRIBUTE_LOCAL_DATA_SIZE, &size, sizeof(size));
}

VX_PRIVATE_API vx_status vxoScaleImage_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData = {0};

    if (index != 0 && index != 2) return VX_ERROR_INVALID_PARAMETERS;

    if (index == 0)
    {
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData.u.imageInfo.format != VX_DF_IMAGE_U8) return VX_ERROR_INVALID_FORMAT;
    }
    else if (index == 2)
    {
        vx_enum interp = 0;

        objData.u.scalarInfo.scalarValuePtr = &interp;


        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_SCALAR, &objData) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData.u.scalarInfo.dataType != VX_TYPE_ENUM) return VX_ERROR_INVALID_TYPE;

        if ((interp != VX_INTERPOLATION_TYPE_NEAREST_NEIGHBOR) &&
            (interp != VX_INTERPOLATION_TYPE_BILINEAR) &&
            (interp != VX_INTERPOLATION_TYPE_AREA))
        {
            return VX_ERROR_INVALID_VALUE;
        }
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoScaleImage_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    vx_object_data_s objData[2];

    objData[0].u.imageInfo.format = VX_DF_IMAGE_VIRT;
    objData[1].u.imageInfo.format = VX_DF_IMAGE_VIRT;

    if (index != 1) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData[0]) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData[1]) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if (objData[1].u.imageInfo.width == 0 || objData[1].u.imageInfo.height == 0) return VX_ERROR_INVALID_PARAMETERS;

    if (objData[1].u.imageInfo.format == VX_DF_IMAGE_VIRT) return VX_ERROR_INVALID_FORMAT;

    if (objData[0].u.imageInfo.format != objData[1].u.imageInfo.format) return VX_ERROR_INVALID_FORMAT;

    vxoFillMetaData(ptr, VX_TYPE_IMAGE,
                    objData[1].u.imageInfo.format,
                    objData[1].u.imageInfo.width,
                    objData[1].u.imageInfo.height,
                    0);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoBasekernel_HalfscaleGaussian(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_graph graph;

    if (num != 3) return VX_ERROR_INVALID_PARAMETERS;

    node->kernelAttributes.isAllGPU = vx_true_e;
    graph = vxoNode_GetChildGraph(node);
    return vxProcessGraph(graph);
}

VX_PRIVATE_API vx_status vxoHalfscaleGaussian_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData0 = {0};
    vx_object_data_s objData2 = {0};

    if (index != 0 && index != 2) return VX_ERROR_INVALID_PARAMETERS;

    if (index == 0)
    {

        if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData0) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData0.u.imageInfo.format != VX_DF_IMAGE_U8) return VX_ERROR_INVALID_FORMAT;
    }
    else if (index == 2)
    {
        vx_int32 kSize = 0;
        objData2.u.scalarInfo.scalarValuePtr = &kSize;

        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_SCALAR, &objData2) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData2.u.scalarInfo.dataType != VX_TYPE_INT32) return VX_ERROR_INVALID_TYPE;

        if ((kSize != 3) && (kSize != 5)) return VX_ERROR_INVALID_VALUE;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoHalfscaleGaussian_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    vx_object_data_s objData0 = {0};
    vx_object_data_s objData1 = {0};

    if (index != 1) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData0) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData1) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    vxoFillMetaData(ptr, VX_TYPE_IMAGE,
                    objData0.u.imageInfo.format,
                    (objData0.u.imageInfo.width + 1) / 2,
                    (objData0.u.imageInfo.height + 1) / 2,
                    0);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_convolution vxCreateGaussian5x5Convolution(vx_context context)
{
    vx_convolution conv;
    vx_status      status;

    conv = vxCreateConvolution(context, 5, 5);
    status = vxAccessConvolutionCoefficients(conv, NULL);
    if (status != VX_SUCCESS) goto ErrorExit;

    status = vxCommitConvolutionCoefficients(conv, (vx_int16 *)gaussian5x5);
    if (status != VX_SUCCESS) goto ErrorExit;

    vxSetConvolutionAttribute(conv, VX_CONVOLUTION_ATTRIBUTE_SCALE, (void *)&gaussian5x5scale, sizeof(vx_uint32));
    if (status != VX_SUCCESS) goto ErrorExit;

    return conv;

ErrorExit:
    vxReleaseConvolution(&conv);
    return VX_NULL;
}

VX_PRIVATE_API vx_status vxoHalfscaleGaussian_Initializer(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_image         inputImage     = VX_NULL;
    vx_image         outputImage    = VX_NULL;
    vx_convolution   convolution5x5 = VX_NULL;
    vx_context       context        = VX_NULL;
    vx_graph         graph          = VX_NULL;
    vx_image         virtualImage   = VX_NULL;
    vx_node          filterNodes[2] = {VX_NULL, VX_NULL};
    vx_border_mode_t borderModes;
    vx_uint32        i;
    vx_int32         kernelSize     = 0;
    vx_status        status         = VX_ERROR_INVALID_PARAMETERS;

    if (num != 3) return VX_ERROR_INVALID_PARAMETERS;

    inputImage = (vx_image)parameters[0];
    outputImage = (vx_image)parameters[1];
    context = vxGetContext((vx_reference)node);
    graph = vxCreateGraph(context);

    if (graph == NULL) return VX_ERROR_INVALID_PARAMETERS;

    vxAccessScalarValue((vx_scalar)parameters[2], &kernelSize);

    if (kernelSize != 3 && kernelSize != 5) return VX_ERROR_INVALID_PARAMETERS;

    if (kernelSize == 5)
    {
        convolution5x5 = vxCreateGaussian5x5Convolution(context);
    }

    virtualImage = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8);

    if (kernelSize == 3)
    {
        filterNodes[0] = vxGaussian3x3Node(graph, inputImage, virtualImage);
    }
    else
    {
        filterNodes[0] = vxConvolveNode(graph, inputImage, convolution5x5, virtualImage);
    }

    filterNodes[1] = vxScaleImageNode(graph, virtualImage, outputImage, VX_INTERPOLATION_TYPE_NEAREST_NEIGHBOR);

    vxQueryNode(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &borderModes, sizeof(borderModes));

    for (i = 0; i < vxmLENGTH_OF(filterNodes); i++)
    {
        vxSetNodeAttribute(filterNodes[i], VX_NODE_ATTRIBUTE_BORDER_MODE, &borderModes, sizeof(borderModes));
    }

    status  = vxoAddParameterToGraphByIndex(graph, filterNodes[0], 0);
    status |= vxoAddParameterToGraphByIndex(graph, filterNodes[1], 1);
    status |= vxoAddParameterToGraphByIndex(graph, node, 2);
    status |= vxVerifyGraph(graph);
    status |= vxoNode_SetChildGraph(node, graph);

    if (convolution5x5 != VX_NULL) vxReleaseConvolution(&convolution5x5);
    if (virtualImage != VX_NULL) vxReleaseImage(&virtualImage);
    for (i = 0; i < vxmLENGTH_OF(filterNodes); i++)
    {
        if (filterNodes[i] != VX_NULL) vxReleaseNode(&filterNodes[i]);
    }
    if (graph != VX_NULL) vxReleaseGraph(&graph);
    return status;
}

VX_PRIVATE_API vx_status vxoHalfscaleGaussian_Deinitializer(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    if (num != 3) return VX_ERROR_INVALID_PARAMETERS;

    return vxoNode_SetChildGraph(node, 0);
}

VX_PRIVATE_API vx_status vxoBaseKernel_Histogram(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_image        srcImage;
    vx_distribution dist;

    if (num != 2) return VX_ERROR_INVALID_PARAMETERS;

    srcImage   = (vx_image) parameters[0];
    dist = (vx_distribution)parameters[1];

    node->kernelAttributes.isAllGPU = vx_false_e;

    return vxHistogram(srcImage, dist, node->kernelAttributes.stagings);
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

VX_PRIVATE_API vx_status vxoHistogram_Initializer(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
    vx_size numBins = 0;
    vx_uint32 window_size = 0;
    vx_uint32* dist_src = NULL;
	vx_distribution dist = (vx_distribution)parameters[1];

    vxQueryDistribution(dist, VX_DISTRIBUTION_ATTRIBUTE_BINS, &numBins, sizeof(numBins));
    vxQueryDistribution(dist, VX_DISTRIBUTION_ATTRIBUTE_WINDOW, &window_size, sizeof(window_size));

    node->kernelAttributes.stagings[0] = (vx_reference)vxCreateDistribution(vxGetContext((vx_reference)dist), 16, 0, window_size*16);

    status |= vxAccessDistribution((vx_distribution)node->kernelAttributes.stagings[0], (void **)&dist_src, VX_READ_ONLY);
    status |= vxCommitDistribution((vx_distribution)node->kernelAttributes.stagings[0], dist_src);

    return status;
}

VX_PRIVATE_API vx_status vxoHistogram_Deinitializer(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

	status |= vxReleaseDistribution((vx_distribution*)&node->kernelAttributes.stagings[0]);

    return status;
}

VX_PRIVATE_API vx_status vxoBaseKernel_EqualizeHist(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_image srcImage;
    vx_image dstImage;

    if (num != 2) return VX_ERROR_INVALID_PARAMETERS;

    srcImage = (vx_image)parameters[0];
    dstImage = (vx_image)parameters[1];

    return vxEqualizeHist(srcImage, dstImage, node->kernelAttributes.stagings);
}

VX_PRIVATE_API vx_status vxoEqualizeHist_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData = {0};

    if (index != 0) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if (objData.u.imageInfo.format != VX_DF_IMAGE_U8) return VX_ERROR_INVALID_FORMAT;

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoEqualizeHist_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    vx_object_data_s objData = {0};

    if (index != 1) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    vxoFillMetaData(ptr, VX_TYPE_IMAGE, VX_DF_IMAGE_U8,
                    objData.u.imageInfo.width,
                    objData.u.imageInfo.height,
                    0);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoEqualizeHist_Initializer(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 width = 0, height = 0;
	vx_image src = (vx_image)parameters[0];

    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));

	node->kernelAttributes.stagings[0] = (vx_reference)vxCreateImage(vxGetContext((vx_reference)src), 256 * 2, 1, VX_DF_IMAGE_U16);
    node->kernelAttributes.stagings[1] = (vx_reference)vxCreateImage(vxGetContext((vx_reference)src), 256 * 2, 1, VX_DF_IMAGE_U16);

	if (!vxoImage_AllocateMemory((vx_image)node->kernelAttributes.stagings[0])
		|| !vxoImage_AllocateMemory((vx_image)node->kernelAttributes.stagings[1]))
		status |= VX_ERROR_NO_MEMORY;

    return status;
}

VX_PRIVATE_API vx_status vxoEqualizeHist_Deinitializer(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

	status |= vxReleaseImage((vx_image*)&node->kernelAttributes.stagings[0]);
	status |= vxReleaseImage((vx_image*)&node->kernelAttributes.stagings[1]);

    return status;
}

VX_PRIVATE_API vx_status vxoBaseKernel_AbsDiff(vx_node node, vx_reference parameters[], vx_uint32 num)
{
    vx_image inputImage1;
    vx_image inputImage2;
    vx_image outputImage;

    if (num != 3) return VX_ERROR_INVALID_PARAMETERS;

    inputImage1 = (vx_image)parameters[0];
    inputImage2 = (vx_image)parameters[1];
    outputImage = (vx_image)parameters[2];

    node->kernelAttributes.isAllGPU = vx_true_e;
    return vxAbsDiff(inputImage1, inputImage2, outputImage);
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

VX_PRIVATE_API vx_status vxoBaseKernel_MeanStdDev(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_image  inputImage;
    vx_scalar meanScalar;
    vx_scalar stddevScalar;

    if (num != 3) return VX_ERROR_INVALID_PARAMETERS;

    inputImage   = (vx_image) parameters[0];
    meanScalar   = (vx_scalar)parameters[1];
    stddevScalar = (vx_scalar)parameters[2];

    node->kernelAttributes.isAllGPU = vx_false_e;
    return vxMeanStdDev(inputImage, meanScalar, stddevScalar);
}

VX_PRIVATE_API vx_status vxoMeanStdDev_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData = {0};

    if (index != 0) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if (objData.u.imageInfo.format != VX_DF_IMAGE_U8 && objData.u.imageInfo.format != VX_DF_IMAGE_U16)
        return VX_ERROR_INVALID_PARAMETERS;

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoMeanStdDev_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    if (index != 1 && index != 2) return VX_ERROR_INVALID_PARAMETERS;

    vxoFillMetaData(ptr, VX_TYPE_SCALAR, 0, 0, 0, VX_TYPE_FLOAT32);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoBaseKernel_Threshold(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_image     srcImage;
    vx_threshold threshold;
    vx_image     dstImage;

    if (num != 3) return VX_ERROR_INVALID_PARAMETERS;

    srcImage = (vx_image)    parameters[0];
    threshold = (vx_threshold)parameters[1];
    dstImage = (vx_image)    parameters[2];

    return vxThreshold(srcImage, threshold, dstImage);
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

VX_PRIVATE_API vx_status vxoBaseKernel_IntegralImage(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_image srcImage;
    vx_image dstImage;

    if (num != 2) return VX_ERROR_INVALID_PARAMETERS;

    srcImage = (vx_image)parameters[0];
    dstImage = (vx_image)parameters[1];

    /* To clean up - Cannot support multipass yet. Need deal with the staging resources */
    node->kernelAttributes.isAllGPU = vx_false_e;
	return vxIntegralImage(srcImage, dstImage);
}

VX_PRIVATE_API vx_status vxoIntegral_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData;

    if (index != 0) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if (objData.u.imageInfo.format != VX_DF_IMAGE_U8) return VX_ERROR_INVALID_PARAMETERS;

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoIntegral_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    vx_object_data_s objData;

    if (index != 1) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    vxoFillMetaData(ptr, VX_TYPE_IMAGE, VX_DF_IMAGE_U32, objData.u.imageInfo.width, objData.u.imageInfo.height, 0);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoBaseKernel_Erode3x3(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_border_mode_t bordermode;
    vx_image srcImage;
    vx_image dstImage;

    if (num != 2) return VX_ERROR_INVALID_PARAMETERS;

    srcImage = (vx_image)parameters[0];
    dstImage = (vx_image)parameters[1];

    if (vxQueryNode(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &bordermode, sizeof(bordermode)) == VX_SUCCESS)
    {
        return vxErode3x3(srcImage, dstImage, &bordermode);
    }

    return VX_ERROR_INVALID_PARAMETERS;
}

VX_PRIVATE_API vx_status vxoBaseKernel_Dilate3x3(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_border_mode_t bordermode;
    vx_image srcImage;
    vx_image dstImage;

    if (num != 2) return VX_ERROR_INVALID_PARAMETERS;

    srcImage = (vx_image)parameters[0];
    dstImage = (vx_image)parameters[1];

    if (vxQueryNode(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &bordermode, sizeof(bordermode)) == VX_SUCCESS)
    {
        return vxDilate3x3(srcImage, dstImage, &bordermode);
    }

    return VX_ERROR_INVALID_PARAMETERS;
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

VX_PRIVATE_API vx_status vxoBaseKernel_Median3x3(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_border_mode_t bordermode;
    vx_image         srcImage;
    vx_image         dstImage;

    if (num != 2) return VX_ERROR_INVALID_PARAMETERS;

    srcImage = (vx_image)parameters[0];
    dstImage = (vx_image)parameters[1];

    if (vxQueryNode(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &bordermode, sizeof(bordermode)) == VX_SUCCESS)
    {
        return vxMedian3x3(srcImage, dstImage, &bordermode);
    }

    node->kernelAttributes.isAllGPU = vx_true_e;
    return VX_ERROR_INVALID_PARAMETERS;
}

VX_PRIVATE_API vx_status vxoBaseKernel_Box3x3(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_border_mode_t bordermode;
    vx_image         srcImage;
    vx_image         dstImage;

    if (num != 2) return VX_ERROR_INVALID_PARAMETERS;

    srcImage  = (vx_image)parameters[0];
    dstImage = (vx_image)parameters[1];

    node->kernelAttributes.isAllGPU = vx_true_e;
    if (vxQueryNode(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &bordermode, sizeof(bordermode)) == VX_SUCCESS)
    {
        return vxBox3x3(srcImage, dstImage, &bordermode);
    }

    return VX_ERROR_INVALID_PARAMETERS;
}

VX_PRIVATE_API vx_status vxoBaseKernel_Gaussian3x3(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_border_mode_t bordermode;
    vx_image         srcImage;
    vx_image         dstImage;

    if (num != 2) return VX_ERROR_INVALID_PARAMETERS;

    srcImage  = (vx_image)parameters[0];
    dstImage = (vx_image)parameters[1];

    node->kernelAttributes.isAllGPU = vx_true_e;
    if (vxQueryNode(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &bordermode, sizeof(bordermode)) == VX_SUCCESS)
    {
        return vxGaussian3x3(srcImage, dstImage, &bordermode);
    }

    return VX_ERROR_INVALID_PARAMETERS;
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

VX_PRIVATE_API vx_status vxoBasekernel_Convolve(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_border_mode_t bordermode;
    vx_image         srcImage;
    vx_convolution   conv;
    vx_image         dstImage;

    if (num != 3) return VX_ERROR_INVALID_PARAMETERS;

    srcImage  = (vx_image)parameters[0];
    conv = (vx_convolution)parameters[1];
    dstImage  = (vx_image)parameters[2];

    vxQueryNode(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &bordermode, sizeof(bordermode));

    return vxConvolve(srcImage, conv, dstImage, &bordermode);
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

VX_PRIVATE_API vx_status vxoBaseKernel_Pyramid(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_graph graph = VX_NULL;
    vx_size  size  = 0;

    if (num != 2) return VX_ERROR_INVALID_PARAMETERS;

    vxQueryNode(node, VX_NODE_ATTRIBUTE_LOCAL_DATA_PTR, &graph, sizeof(graph));
    vxQueryNode(node, VX_NODE_ATTRIBUTE_LOCAL_DATA_SIZE, &size, sizeof(size));

    if (graph == VX_NULL) return VX_ERROR_INVALID_GRAPH;

    if (size != sizeof(graph)) return VX_ERROR_INVALID_GRAPH;

    return vxProcessGraph(graph);
}

VX_PRIVATE_API vx_status vxoPyramid_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData = {0};

    if (index != 0) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if (objData.u.imageInfo.format != VX_DF_IMAGE_U8) return VX_ERROR_INVALID_FORMAT;

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoPyramid_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    vx_object_data_s objData[2] = {{0}};

    if (index != 1) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData[0]) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_PYRAMID, &objData[1]) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    ptr->type                       = VX_TYPE_PYRAMID;
    ptr->u.pyramidInfo.width        = objData[0].u.imageInfo.width;
    ptr->u.pyramidInfo.height       = objData[0].u.imageInfo.height;
    ptr->u.pyramidInfo.format       = objData[0].u.imageInfo.format;
    ptr->u.pyramidInfo.levelCount   = objData[1].u.pyramidInfo.numLevels;
    ptr->u.pyramidInfo.scale        = objData[1].u.pyramidInfo.scale;

    return VX_SUCCESS;
}

vx_status vxoCopyImage(vx_image input, vx_image output)
{
    vx_imagepatch_addressing_t srcAddrInfo;
    vx_imagepatch_addressing_t dstAddrInfo;
    vx_rectangle_t             rect;
    void *                     baseAddressSrc = VX_NULL;
    void *                     baseAddressDst = VX_NULL;
    void *                     srcPixelValue  = VX_NULL;
    void *                     dstPixelValue  = VX_NULL;
    vx_size                    numplanes = 0;
    vx_uint32                  plane = 0, y = 0, len = 0;

    if (vxQueryImage(input, VX_IMAGE_ATTRIBUTE_PLANES, &numplanes, sizeof(vx_size)) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if (vxGetValidRegionImage(input, &rect) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    for (plane = 0; plane < numplanes; plane++)
    {
        baseAddressSrc = baseAddressDst = NULL;

        if (vxAccessImagePatch(input, &rect, plane, &srcAddrInfo, &baseAddressSrc, VX_READ_ONLY) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (vxAccessImagePatch(output, &rect, plane, &dstAddrInfo, &baseAddressDst, VX_WRITE_ONLY) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        for (y = 0; y < srcAddrInfo.dim_y; y += srcAddrInfo.step_y)
        {
            srcPixelValue = vxFormatImagePatchAddress2d(baseAddressSrc, 0, y, &srcAddrInfo);
            dstPixelValue = vxFormatImagePatchAddress2d(baseAddressDst, 0, y, &dstAddrInfo);

            len = (srcAddrInfo.stride_x * srcAddrInfo.dim_x * srcAddrInfo.scale_x) / VX_SCALE_UNITY;

            memcpy(dstPixelValue, srcPixelValue, len);
        }

        vxCommitImagePatch(input, NULL, plane, &srcAddrInfo, baseAddressSrc);
        vxCommitImagePatch(output, &rect, plane, &dstAddrInfo, baseAddressDst);
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoPyramid_Initializer(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status        status = VX_SUCCESS;
    vx_size          level, numLevels = 1;
    vx_border_mode_t border;
    vx_image         input;
    vx_pyramid       gaussian;
    vx_context       context;
    vx_graph         graph;
    vx_enum          interp;
    vx_image         level0;

    if (num != 2) return VX_ERROR_INVALID_PARAMETERS;

    input    = (vx_image)parameters[0];
    gaussian = (vx_pyramid)parameters[1];

    context  = vxGetContext((vx_reference)node);
    graph    = vxCreateGraph(context);

    if (graph == VX_NULL) return VX_ERROR_INVALID_GRAPH;

    if (vxQueryNode(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &border, sizeof(border)) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    interp   = VX_INTERPOLATION_TYPE_NEAREST_NEIGHBOR;

    level0 = vxGetPyramidLevel(gaussian, 0);

    vxoCopyImage(input, level0);

    vxReleaseImage(&level0);

    status |= vxQueryPyramid(gaussian, VX_PYRAMID_ATTRIBUTE_LEVELS, &numLevels, sizeof(numLevels));

    for (level = 1; level < numLevels; level++)
    {
        vx_image       inputImage = VX_NULL;
        vx_image       dstImage   = VX_NULL;
        vx_image       virtImage  = VX_NULL;
        vx_convolution conv       = VX_NULL;
        vx_node        gNode      = VX_NULL;
        vx_node        sNode      = VX_NULL;

        conv = vxCreateGaussian5x5Convolution(context);
        virtImage = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8);

        if (level == 1)
        {
            gNode = vxConvolveNode(graph, input, conv, virtImage);
        }
        else
        {
            inputImage = vxGetPyramidLevel(gaussian, (vx_uint32)(level-1));
            gNode = vxConvolveNode(graph, inputImage, conv, virtImage);
        }

        vxSetNodeAttribute(gNode, VX_NODE_ATTRIBUTE_BORDER_MODE, &border, sizeof(border));

        dstImage = vxGetPyramidLevel(gaussian, (vx_uint32)level);

        sNode    = vxScaleImageNode(graph, virtImage, dstImage, interp);

        if (inputImage != VX_NULL) vxReleaseImage(&inputImage);
        if (conv != VX_NULL )      vxReleaseConvolution(&conv);
        if (dstImage != VX_NULL)   vxReleaseImage(&dstImage);
        if (virtImage != VX_NULL)  vxReleaseImage(&virtImage);
        if (gNode != VX_NULL)      vxReleaseNode(&gNode);
        if (sNode != VX_NULL )     vxReleaseNode(&sNode);
    }

    if (numLevels > 1) status = vxVerifyGraph(graph);

    if (status == VX_SUCCESS)
    {
        vx_size size = sizeof(vx_graph);

        status |= vxSetNodeAttribute(node, VX_NODE_ATTRIBUTE_LOCAL_DATA_SIZE, &size, sizeof(size));
        status |= vxSetNodeAttribute(node, VX_NODE_ATTRIBUTE_LOCAL_DATA_PTR, &graph, sizeof(graph));
    }

    return status;
}

VX_PRIVATE_API vx_status vxoPyramid_Deinitializer(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
    vx_graph  graph  = 0;
    vx_size   size   = 0;

    if (num != 2) return VX_ERROR_INVALID_PARAMETERS;

    status  = vxQueryNode(node, VX_NODE_ATTRIBUTE_LOCAL_DATA_PTR, &graph, sizeof(graph));
    status |= vxQueryNode(node, VX_NODE_ATTRIBUTE_LOCAL_DATA_SIZE, &size, sizeof(size));

    if (graph == VX_NULL) return VX_ERROR_INVALID_GRAPH;

    if (size != sizeof(graph)) return VX_ERROR_INVALID_GRAPH;

    vxReleaseGraph(&graph);

    graph   = 0;
    size    = 0;
    status |= vxSetNodeAttribute(node, VX_NODE_ATTRIBUTE_LOCAL_DATA_PTR, &graph, sizeof(graph));
    status |= vxSetNodeAttribute(node, VX_NODE_ATTRIBUTE_LOCAL_DATA_SIZE, &size, sizeof(size));

    return status;
}

VX_PRIVATE_API vx_status vxoBaseKernel_Accumulate(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_image inputImage;
    vx_image accumImage;

    if (num != 2)  return VX_ERROR_INVALID_PARAMETERS;

    inputImage = (vx_image)parameters[0];
    accumImage = (vx_image)parameters[1];

    node->kernelAttributes.isAllGPU = vx_true_e;
    return vxAccumulate(inputImage, accumImage);
}

VX_PRIVATE_API vx_status vxoAccumulate_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData0 = {0};
    vx_object_data_s objData1 = {0};

    if (index != 0 && index != 1) return VX_ERROR_INVALID_PARAMETERS;

    if (index == 0)
    {
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData0) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData0.u.imageInfo.format != VX_DF_IMAGE_U8) return VX_ERROR_INVALID_FORMAT;
    }
    else if (index == 1)
    {
        if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData0) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData1) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData0.u.imageInfo.width != objData1.u.imageInfo.width ||
            objData0.u.imageInfo.height != objData1.u.imageInfo.height ||
            objData0.u.imageInfo.format != VX_DF_IMAGE_U8 ||
            objData1.u.imageInfo.format != VX_DF_IMAGE_S16)
        {
            return VX_ERROR_INVALID_PARAMETERS;
        }
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoBaseKernel_AccumulateWeighted(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_image inputImage;
    vx_scalar scalar;
    vx_image accumImage;

    if (num != 3) return VX_ERROR_INVALID_PARAMETERS;

    inputImage = (vx_image)parameters[0];
    scalar = (vx_scalar)parameters[1];
    accumImage = (vx_image)parameters[2];

    node->kernelAttributes.isAllGPU = vx_true_e;
    return vxAccumulateWeighted(inputImage, scalar, accumImage);
}

VX_PRIVATE_API vx_status vxoAccumulateWeighted_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData[3] = {{0}};
    if (index != 0 && index != 1 && index != 2) return VX_ERROR_INVALID_PARAMETERS;

    if (index == 0 )
    {
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData[0]) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData[0].u.imageInfo.format != VX_DF_IMAGE_U8) return VX_ERROR_INVALID_FORMAT;
    }
    else if (index == 2)
    {
        if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData[0]) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData[2]) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData[0].u.imageInfo.width != objData[2].u.imageInfo.width ||
            objData[0].u.imageInfo.height != objData[2].u.imageInfo.height ||
            objData[0].u.imageInfo.format != VX_DF_IMAGE_U8 ||
            objData[2].u.imageInfo.format != VX_DF_IMAGE_U8)
        {
            return VX_ERROR_INVALID_PARAMETERS;
        }
    }
    else if (index == 1)
    {
        vx_float32 alpha = 0.0f;
        objData[1].u.scalarInfo.scalarValuePtr = &alpha;

        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_SCALAR, &objData[1]) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData[1].u.scalarInfo.dataType != VX_TYPE_FLOAT32) return VX_ERROR_INVALID_PARAMETERS;

        if ((alpha < 0.0f) || (alpha > 1.0f)) return VX_ERROR_INVALID_VALUE;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoAccumulate_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_ERROR_INVALID_PARAMETERS;
}

VX_PRIVATE_API vx_status vxoBaseKernel_AccumulateSquare(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_image inputImage;
    vx_scalar scalar;
    vx_image accumImage;

    if (num != 3) return VX_ERROR_INVALID_PARAMETERS;

    inputImage = (vx_image)parameters[0];
    scalar     = (vx_scalar)parameters[1];
    accumImage = (vx_image)parameters[2];

    node->kernelAttributes.isAllGPU = vx_true_e;
    return vxAccumulateSquare(inputImage, scalar, accumImage);
}

VX_PRIVATE_API vx_status vxoAccumulateSquared_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData[3] = {{0}};

    if (index != 0 && index != 1 && index != 2) return VX_ERROR_INVALID_PARAMETERS;

    if (index == 0 )
    {
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData[0]) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData[0].u.imageInfo.format != VX_DF_IMAGE_U8) return VX_ERROR_INVALID_FORMAT;
    }
    else if (index == 2)
    {
        if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData[0]) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData[2]) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData[0].u.imageInfo.width != objData[2].u.imageInfo.width ||
            objData[0].u.imageInfo.height != objData[2].u.imageInfo.height ||
            objData[0].u.imageInfo.format != VX_DF_IMAGE_U8 ||
            objData[2].u.imageInfo.format != VX_DF_IMAGE_S16)
        {
            return VX_ERROR_INVALID_VALUE;
        }
    }
    else if (index == 1)
    {
        vx_uint32 shift = 0u;
        objData[1].u.scalarInfo.scalarValuePtr = &shift;

        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_SCALAR, &objData[1]) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if ((shift < 0) || (shift > 15)) return VX_ERROR_INVALID_VALUE;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoBaseKernel_MinMaxLoc(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_image inputImage;
    vx_scalar minVal;
    vx_scalar maxVal;
    vx_array minLoc;
    vx_array maxLoc;
    vx_scalar minCount;
    vx_scalar maxCount;

    if (num != 7) return VX_ERROR_INVALID_PARAMETERS;

    inputImage = (vx_image)parameters[0];
    minVal     = (vx_scalar)parameters[1];
    maxVal     = (vx_scalar)parameters[2];
    minLoc     = (vx_array)parameters[3];
    maxLoc     = (vx_array)parameters[4];
    minCount   = (vx_scalar)parameters[5];
    maxCount   = (vx_scalar)parameters[6];

    node->kernelAttributes.isAllGPU = vx_false_e;
	return vxMinMaxLoc(inputImage, minVal, maxVal, minLoc, maxLoc, minCount, maxCount, node->kernelAttributes.stagings);
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

VX_PRIVATE_API vx_status vxoMinMaxLoc_Initializer(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 width = 0, height = 0;
	vx_int32 value[2];
    vx_df_image format;
    vx_enum itemType;
	vx_image src = (vx_image)parameters[0];
    vx_array min_loc = (vx_array)parameters[3];
    vx_array max_loc = (vx_array)parameters[4];

    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));

	value[0] = (format == VX_DF_IMAGE_S16)? 0x7fff: 0xff;
	value[1] = (format == VX_DF_IMAGE_S16)? -0x7fff: 0;

	node->kernelAttributes.stagings[0] = (vx_reference)vxCreateScalar(vxGetContext((vx_reference)src), VX_TYPE_INT32, &value[0]);
	node->kernelAttributes.stagings[1] = (vx_reference)vxCreateScalar(vxGetContext((vx_reference)src), VX_TYPE_INT32, &value[1]);
    node->kernelAttributes.stagings[2] = (vx_reference)vxCreateImage(vxGetContext((vx_reference)src), 2, height, VX_DF_IMAGE_U16);

	if (!vxoImage_AllocateMemory((vx_image)node->kernelAttributes.stagings[2]))
		status |= VX_ERROR_NO_MEMORY;

    if (min_loc)
    {
        vxQueryArray(min_loc, VX_ARRAY_ATTRIBUTE_ITEMTYPE, &itemType, sizeof(itemType));
        node->kernelAttributes.stagings[3] = (vx_reference)vxCreateArray(vxGetContext((vx_reference)min_loc), itemType, width*height);

        if (!vxoArray_AllocateMemory((vx_array)node->kernelAttributes.stagings[3]))
        {
            status |= VX_ERROR_NO_MEMORY;
        }

    }
    if (max_loc)
    {
        vxQueryArray(max_loc, VX_ARRAY_ATTRIBUTE_ITEMTYPE, &itemType, sizeof(itemType));
        node->kernelAttributes.stagings[4] = (vx_reference)vxCreateArray(vxGetContext((vx_reference)max_loc), itemType, width*height);

        if ( !vxoArray_AllocateMemory((vx_array)node->kernelAttributes.stagings[4]))
        {
		    status |= VX_ERROR_NO_MEMORY;
        }
    }
	vxCommitScalarValue((vx_scalar)node->kernelAttributes.stagings[0], &value[0]);
	vxCommitScalarValue((vx_scalar)node->kernelAttributes.stagings[1], &value[1]);

    return status;
}

VX_PRIVATE_API vx_status vxoMinMaxLoc_Deinitializer(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

	status |= vxReleaseScalar((vx_scalar*)&node->kernelAttributes.stagings[0]);
	status |= vxReleaseScalar((vx_scalar*)&node->kernelAttributes.stagings[1]);
    status |= vxReleaseImage((vx_image*)&node->kernelAttributes.stagings[2]);
    if (node->kernelAttributes.stagings[3])
    {
        status |= vxReleaseArray((vx_array*)&node->kernelAttributes.stagings[3]);
    }
    if (node->kernelAttributes.stagings[4])
    {
        status |= vxReleaseArray((vx_array*)&node->kernelAttributes.stagings[4]);
    }

    return status;
}

VX_PRIVATE_API vx_status vxoBaseKernel_ConvertDepth(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_image  inputImage;
    vx_image  outputImage;
    vx_scalar spol;
    vx_scalar sshf;

    if (num != 4) return VX_ERROR_INVALID_PARAMETERS;

    inputImage  = (vx_image) parameters[0];
    outputImage = (vx_image) parameters[1];
    spol        = (vx_scalar)parameters[2];
    sshf        = (vx_scalar)parameters[3];

    return vxConvertDepth(inputImage, outputImage, spol, sshf);
}

VX_PRIVATE_API vx_status vxoConvertDepth_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData;
    vx_enum          overflow_policy = 0;
    vx_int32         shift = 0;

    if (index != 0 && index != 2 && index != 3) return VX_ERROR_INVALID_PARAMETERS;

    switch(index)
    {
    case 0:
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
                return VX_ERROR_INVALID_PARAMETERS;

        if ((objData.u.imageInfo.format != VX_DF_IMAGE_U8)  &&
            (objData.u.imageInfo.format != VX_DF_IMAGE_U16) &&
            (objData.u.imageInfo.format != VX_DF_IMAGE_U32) &&
            (objData.u.imageInfo.format != VX_DF_IMAGE_S16) &&
            (objData.u.imageInfo.format != VX_DF_IMAGE_S32) &&
            (objData.u.imageInfo.format != VX_DF_IMAGE_F32))
        {
            return VX_ERROR_INVALID_FORMAT;
        }
        break;

    case 2:
        objData.u.scalarInfo.scalarValuePtr = &overflow_policy;

        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_SCALAR, &objData) != VX_SUCCESS)
                return VX_ERROR_INVALID_PARAMETERS;

        if (objData.u.scalarInfo.dataType != VX_TYPE_ENUM) return VX_ERROR_INVALID_TYPE;

        if ((overflow_policy != VX_CONVERT_POLICY_WRAP) &&
            (overflow_policy != VX_CONVERT_POLICY_SATURATE))
        {
            return VX_ERROR_INVALID_VALUE;
        }
        break;

    case 3:
        objData.u.scalarInfo.scalarValuePtr = &shift;

        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_SCALAR, &objData) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData.u.scalarInfo.dataType != VX_TYPE_INT32) return VX_ERROR_INVALID_TYPE;

        if (shift < 0 || shift >= 32) return VX_ERROR_INVALID_VALUE;

        break;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoConvertDepth_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    vx_status        status = VX_ERROR_INVALID_PARAMETERS;
    vx_object_data_s objData[2] = {{0}};
    vx_uint32        i;

    if (index != 1) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData[0]) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 1, VX_TYPE_IMAGE, &objData[1]) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    for (i = 0; i < vxmLENGTH_OF(convertDepth_InputOutputFormat); i++)
    {
        if ((objData[0].u.imageInfo.format == convertDepth_InputOutputFormat[i][0]) &&
            (objData[1].u.imageInfo.format == convertDepth_InputOutputFormat[i][1]))
        {
            vxoFillMetaData(ptr, VX_TYPE_IMAGE, objData[1].u.imageInfo.format, objData[0].u.imageInfo.width, objData[0].u.imageInfo.height, 0);
            status = VX_SUCCESS;
            break;
        }
    }

    return status;
}

VX_PRIVATE_API vx_status vxoBaseKernel_CannyEdge(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_graph graph;

    if (num != 5) return VX_ERROR_INVALID_PARAMETERS;

    graph = vxoNode_GetChildGraph(node);

    return vxProcessGraph(graph);
}

VX_PRIVATE_API vx_status vxoCannyEdge_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData = {0};
    vx_int32         gs      = 0;
    vx_enum          norm    = 0;

    if (index != 0 && index != 1 && index != 2 && index != 3) return VX_ERROR_INVALID_PARAMETERS;

    switch (index)
    {
    case 0:
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData.u.imageInfo.format != VX_DF_IMAGE_U8) return VX_ERROR_INVALID_FORMAT;

        break;
    case 1:
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_THRESHOLD, &objData) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData.u.thresholdInfo.dataType != VX_THRESHOLD_TYPE_RANGE) return VX_ERROR_INVALID_TYPE;

        break;
    case 2:
        objData.u.scalarInfo.scalarValuePtr = &gs;
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_SCALAR, &objData) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData.u.scalarInfo.dataType != VX_TYPE_INT32) return VX_ERROR_INVALID_TYPE;

        if ((gs != 3) && (gs != 5) && (gs != 7)) return VX_ERROR_INVALID_VALUE;

        break;
    case 3:
        objData.u.scalarInfo.scalarValuePtr = &norm;
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_SCALAR, &objData) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if ((norm != VX_NORM_L1) && (norm != VX_NORM_L2)) return VX_ERROR_INVALID_VALUE;

        break;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoCannyEdge_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    vx_object_data_s objData = {0};

    if (index != 4) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    vxoFillMetaData(ptr, VX_TYPE_IMAGE, objData.u.imageInfo.format, objData.u.imageInfo.width, objData.u.imageInfo.height, 0);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoCannyEdge_Initializer(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status        status;
    vx_context       context;
    vx_graph         graph;
    vx_image         input;
    vx_threshold     hyst;
    vx_scalar        gradientSize;
    vx_scalar        normType;
    vx_image         output;
    vx_uint32        i;
    vx_image         virtImages[5];
    vx_node          nodes[5];
    vx_border_mode_t borders;

    if (num != 5) return VX_ERROR_INVALID_PARAMETERS;

    input        = (vx_image)parameters[0];
    hyst         = (vx_threshold)parameters[1];
    gradientSize = (vx_scalar)parameters[2];
    normType     = (vx_scalar)parameters[3];
    output       = (vx_image)parameters[4];

    context      = vxGetContext((vx_reference)node);
    graph        = vxCreateGraph(context);

    if (graph == VX_NULL) return VX_ERROR_INVALID_GRAPH;

    for (i = 0; i < vxmLENGTH_OF(virtImages); i++)
    {
        virtImages[i] = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_VIRT);
    }

    nodes[0] = vxSobelMxNNode(graph, input, gradientSize, virtImages[0], virtImages[1]);
    nodes[1] = vxElementwiseNormNode(graph, virtImages[0], virtImages[1], normType, virtImages[2]);
    nodes[2] = vxPhaseNode(graph, virtImages[0], virtImages[1], virtImages[3]);
    nodes[3] = vxNonMaxSuppressionNode(graph, virtImages[2], virtImages[3], virtImages[4]);
    nodes[4] = vxEdgeTraceNode(graph, virtImages[4], hyst, output);


    vxQueryNode(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &borders, sizeof(borders));

    for (i = 0; i < vxmLENGTH_OF(nodes); i++)
    {
        vxSetNodeAttribute(nodes[i], VX_NODE_ATTRIBUTE_BORDER_MODE, &borders, sizeof(borders));
    }

    status  = vxoAddParameterToGraphByIndex(graph, nodes[0], 0);
    status |= vxoAddParameterToGraphByIndex(graph, nodes[4], 1);
    status |= vxoAddParameterToGraphByIndex(graph, nodes[0], 1);
    status |= vxoAddParameterToGraphByIndex(graph, nodes[1], 2);
    status |= vxoAddParameterToGraphByIndex(graph, nodes[4], 2);

    status |= vxVerifyGraph(graph);

    for (i = 0; i < vxmLENGTH_OF(nodes); i++)
    {
        vxReleaseNode(&nodes[i]);
    }

    for (i = 0; i < vxmLENGTH_OF(virtImages); i++)
    {
        vxReleaseImage(&virtImages[i]);
    }

    status = vxoNode_SetChildGraph(node, graph);

    vxReleaseGraph(&graph);

    return status;
}

VX_PRIVATE_API vx_status vxoCannyEdge_Deinitializer(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    if (num != 5) return VX_ERROR_INVALID_PARAMETERS;

    return vxoNode_SetChildGraph(node, 0);
}

VX_PRIVATE_API vx_status vxoBinaryBitwise_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData[2] = {{0}};

    if (index != 0 && index != 1) return VX_ERROR_INVALID_PARAMETERS;

    if (index == 0)
    {
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData[0]) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData[0].u.imageInfo.format != VX_DF_IMAGE_U8) return VX_ERROR_INVALID_FORMAT;
    }
    else if (index == 1)
    {
        if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData[0]) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (vxoGetObjAttributeByNodeIndex(node, 1, VX_TYPE_IMAGE, &objData[1]) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData[0].u.imageInfo.width != objData[1].u.imageInfo.width ||
            objData[0].u.imageInfo.height != objData[1].u.imageInfo.height ||
            objData[0].u.imageInfo.format != objData[1].u.imageInfo.format)
        {
            return VX_ERROR_INVALID_VALUE;
        }
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoBinaryBitwise_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    vx_object_data_s objData;

    if (index != 2) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    vxoFillMetaData(ptr, VX_TYPE_IMAGE, VX_DF_IMAGE_U8, objData.u.imageInfo.width, objData.u.imageInfo.height, 0);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoBaseKernel_And(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_image inputImage1;
    vx_image inputImage2;
    vx_image outputImage;

    if (num != 3) return VX_ERROR_INVALID_PARAMETERS;

    inputImage1 = (vx_image)parameters[0];
    inputImage2 = (vx_image)parameters[1];
    outputImage = (vx_image)parameters[2];

    node->kernelAttributes.isAllGPU = vx_true_e;
    return vxAnd(inputImage1, inputImage2, outputImage);
}

VX_PRIVATE_API vx_status vxoBaseKernel_Or(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_image inputImage1;
    vx_image inputImage2;
    vx_image outputImage;

    if (num != 3) return VX_ERROR_INVALID_PARAMETERS;

    inputImage1 = (vx_image)parameters[0];
    inputImage2 = (vx_image)parameters[1];
    outputImage = (vx_image)parameters[2];

    return vxOr(inputImage1, inputImage2, outputImage);
}

VX_PRIVATE_API vx_status vxoBasekernel_Xor(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_image inputImage1;
    vx_image inputImage2;
    vx_image outputImage;

    if (num != 3) return VX_ERROR_INVALID_PARAMETERS;

    inputImage1 = (vx_image)parameters[0];
    inputImage2 = (vx_image)parameters[1];
    outputImage = (vx_image)parameters[2];

    node->kernelAttributes.isAllGPU = vx_true_e;

    return vxXor(inputImage1, inputImage2, outputImage);
}

VX_PRIVATE_API vx_status vxoUnaryBitwise_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData = {0};
    if (index != 0) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if (objData.u.imageInfo.format != VX_DF_IMAGE_U8) return VX_ERROR_INVALID_FORMAT;

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoUnaryBitwise_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    vx_object_data_s objData;

    if (index != 1) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    vxoFillMetaData(ptr, VX_TYPE_IMAGE, VX_DF_IMAGE_U8, objData.u.imageInfo.width, objData.u.imageInfo.height, 0);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoBaseKernel_Not(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_image inputImage;
    vx_image outputImage;

    if (num != 2) return VX_ERROR_INVALID_PARAMETERS;

    inputImage  = (vx_image)parameters[0];
    outputImage = (vx_image)parameters[1];

    node->kernelAttributes.isAllGPU = vx_true_e;
    return vxNot(inputImage, outputImage);
}

VX_PRIVATE_API vx_status vxoBaseKernel_Multiply(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_image  inputImage0;
    vx_image  inputImage1;
    vx_scalar scale_param;
    vx_scalar opolicy_param;
    vx_scalar rpolicy_param;
    vx_image  outputImage;

    if (num != 6) return VX_ERROR_INVALID_PARAMETERS;

    inputImage0   = (vx_image)parameters[0];
    inputImage1   = (vx_image)parameters[1];
    scale_param   = (vx_scalar)parameters[2];
    opolicy_param = (vx_scalar)parameters[3];
    rpolicy_param = (vx_scalar)parameters[4];
    outputImage   = (vx_image)parameters[5];

    node->kernelAttributes.isAllGPU = vx_true_e;
    return vxMultiply(inputImage0, inputImage1, scale_param, opolicy_param, rpolicy_param, outputImage);
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

VX_PRIVATE_API vx_status vxoBaseKernel_Add(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_image  inputImage0;
    vx_image  inputImage1;
    vx_scalar policy_param;
    vx_image  outputImage;

    if (num != 4) return VX_ERROR_INVALID_PARAMETERS;

    inputImage0  = (vx_image)parameters[0];
    inputImage1  = (vx_image)parameters[1];
    policy_param = (vx_scalar)parameters[2];
    outputImage  = (vx_image)parameters[3];

    node->kernelAttributes.isAllGPU = vx_true_e;
    return vxAddition(inputImage0, inputImage1, policy_param, outputImage);
}

VX_PRIVATE_API vx_status vxoBaseKernel_Sub(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_image  inputImage0;
    vx_image  inputImage1;
    vx_scalar policy_param;
    vx_image  outputImage;

    if (num != 4) return VX_ERROR_INVALID_PARAMETERS;

    inputImage0 = (vx_image)parameters[0];
    inputImage1 = (vx_image)parameters[1];
    policy_param = (vx_scalar)parameters[2];
    outputImage = (vx_image)parameters[3];

    return vxSubtraction(inputImage0, inputImage1, policy_param, outputImage);
}

VX_PRIVATE_API vx_status vxoAddSubtract_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData[3] = {{0}};
    vx_enum          overflowPolicy = 0;
    if (index != 0 && index != 1 && index != 2) return VX_ERROR_INVALID_PARAMETERS;

    switch(index)
    {
        case 0:
            if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData[0]) != VX_SUCCESS)
                return VX_ERROR_INVALID_PARAMETERS;

            if (objData[0].u.imageInfo.format != VX_DF_IMAGE_U8 && objData[0].u.imageInfo.format != VX_DF_IMAGE_S16)
                return VX_ERROR_INVALID_PARAMETERS;

            break;

        case 1:
            if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData[0]) != VX_SUCCESS)
                return VX_ERROR_INVALID_PARAMETERS;

            if (vxoGetObjAttributeByNodeIndex(node, 1, VX_TYPE_IMAGE, &objData[1]) != VX_SUCCESS)
                return VX_ERROR_INVALID_PARAMETERS;

            if (objData[0].u.imageInfo.width != objData[1].u.imageInfo.width ||
                objData[0].u.imageInfo.height != objData[1].u.imageInfo.height)
            {
                return VX_ERROR_INVALID_PARAMETERS;
            }

            if (objData[1].u.imageInfo.format != VX_DF_IMAGE_U8 &&
                objData[1].u.imageInfo.format != VX_DF_IMAGE_S16)
            {
                return VX_ERROR_INVALID_FORMAT;
            }

            break;

        case 2:
            objData[2].u.scalarInfo.scalarValuePtr = &overflowPolicy;

            if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_SCALAR, &objData[2]) != VX_SUCCESS)
                return VX_ERROR_INVALID_PARAMETERS;

            if (objData[2].u.scalarInfo.dataType != VX_TYPE_ENUM) return VX_ERROR_INVALID_TYPE;

            if ((overflowPolicy != VX_CONVERT_POLICY_WRAP) && (overflowPolicy != VX_CONVERT_POLICY_SATURATE))
                return VX_ERROR_INVALID_VALUE;

            break;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoAddSubtract_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    vx_object_data_s objData[3] = {{0}};
    if (index != 3) return VX_ERROR_INVALID_PARAMETERS;

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

VX_PRIVATE_API vx_status vxoBaseKernel_WarpPerspective(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_image        inputImage;
    vx_matrix        matrix;
    vx_scalar        scalarType;
    vx_image         outputImage;
    vx_border_mode_t borders;

    if (num != 4) return VX_ERROR_INVALID_PARAMETERS;

    inputImage  = (vx_image) parameters[0];
    matrix      = (vx_matrix)parameters[1];
    scalarType  = (vx_scalar)parameters[2];
    outputImage = (vx_image) parameters[3];

    vxQueryNode(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &borders, sizeof(borders));

    return vxWarpPerspective(inputImage, matrix, scalarType, outputImage, &borders);
}

VX_PRIVATE_API vx_status vxWarpAffineKernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_image        inputImage;
    vx_matrix        matrix;
    vx_scalar        scalarType;
    vx_image         outputImage;
    vx_border_mode_t borders;

    if (num != 4) return VX_ERROR_INVALID_PARAMETERS;

    inputImage  = (vx_image) parameters[0];
    matrix      = (vx_matrix)parameters[1];
    scalarType  = (vx_scalar)parameters[2];
    outputImage = (vx_image) parameters[3];

    vxQueryNode(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &borders, sizeof(borders));

    return vxWarpAffine(inputImage, matrix, scalarType, outputImage, &borders);
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

VX_PRIVATE_API vx_status vxoWarpPerspective_ValidateInput(vx_node node, vx_uint32 index)
{
    return vxoWarp_ValidateInput(node, index, 3);
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

VX_PRIVATE_API vx_status vxoBaseKernel_HarrisCorners(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_graph graph;

    if (num != vxmLENGTH_OF(basekernel_harris_params)) return VX_ERROR_INVALID_PARAMETERS;

    node->kernelAttributes.isAllGPU = vx_false_e;

    graph = vxoNode_GetChildGraph(node);

    return vxProcessGraph(graph);
}

VX_PRIVATE_API vx_status vxoHarris_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData[5] = {{0}};
    vx_float32       d          = 0.0f;
    vx_float32       k          = 0.0f;
    vx_int32         size       = 0;

    switch (index)
    {
        case 0:
            if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData[0]) != VX_SUCCESS)
                return VX_ERROR_INVALID_PARAMETERS;

            if ((objData[0].u.imageInfo.format != VX_DF_IMAGE_U8)) return VX_ERROR_INVALID_FORMAT;

            break;
        case 1:
            if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_SCALAR, &objData[1]) != VX_SUCCESS)
                return VX_ERROR_INVALID_PARAMETERS;

            if (objData[1].u.scalarInfo.dataType != VX_TYPE_FLOAT32) return VX_ERROR_INVALID_TYPE;

            break;
        case 2:
            objData[2].u.scalarInfo.scalarValuePtr = &d;
            if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_SCALAR, &objData[2]) != VX_SUCCESS)
                return VX_ERROR_INVALID_PARAMETERS;

            if (objData[2].u.scalarInfo.dataType != VX_TYPE_FLOAT32) return VX_ERROR_INVALID_TYPE;

            if (d < 1.0 || d > 5.0) return VX_ERROR_INVALID_VALUE;

            break;
        case 3:
            objData[3].u.scalarInfo.scalarValuePtr = &k;
            if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_SCALAR, &objData[3]) != VX_SUCCESS)
                return VX_ERROR_INVALID_PARAMETERS;

            if (objData[3].u.scalarInfo.dataType != VX_TYPE_FLOAT32) return VX_ERROR_INVALID_TYPE;

            if (k < 0.040000f || k >= 0.150001f) return VX_ERROR_INVALID_VALUE;

            break;
        case 4:
        case 5:
            objData[4].u.scalarInfo.scalarValuePtr = &size;
            if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_SCALAR, &objData[4]) != VX_SUCCESS)
                return VX_ERROR_INVALID_PARAMETERS;

            if (objData[4].u.scalarInfo.dataType != VX_TYPE_INT32) return VX_ERROR_INVALID_PARAMETERS;

            if ((size != 3) && (size != 5) && (size != 7)) return VX_ERROR_INVALID_VALUE;

            break;
        default:
            return VX_ERROR_INVALID_PARAMETERS;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoHarris_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    if (index != 6 && index != 7) return VX_ERROR_INVALID_PARAMETERS;

    switch (index)
    {
        case 6:
            ptr->type                 = VX_TYPE_ARRAY;
            ptr->u.arrayInfo.itemType = VX_TYPE_KEYPOINT;
            ptr->u.arrayInfo.capacity = 0;
            break;

        case 7:
            ptr->u.scalarInfo.type    = VX_TYPE_UINT32;
            break;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoHarris_Initializer(vx_node node, vx_reference parameters[], vx_uint32 num)
{
    vx_status  status = VX_FAILURE;
    vx_image   srcImage;
    vx_scalar  strScalar;
    vx_scalar  minScalar;
    vx_scalar  senScalar;
    vx_scalar  winScalar;
    vx_scalar  blkScalar;
    vx_array   array;
    vx_scalar  numCornersScalar;
    vx_context context;
    vx_graph   graph;

    vx_uint32 i = 0;
    vx_int32 ds = 4;
    vx_scalar shiftScalar;
    vx_image virtImages[5];
    vx_node nodes[4];

    if (num != vxmLENGTH_OF(basekernel_harris_params)) return VX_ERROR_INVALID_PARAMETERS;

    srcImage         = (vx_image)parameters[0];
    strScalar        = (vx_scalar)parameters[1];
    minScalar        = (vx_scalar)parameters[2];
    senScalar        = (vx_scalar)parameters[3];
    winScalar        = (vx_scalar)parameters[4];
    blkScalar        = (vx_scalar)parameters[5];
    array            = (vx_array)parameters[6];
    numCornersScalar = (vx_scalar)parameters[7];
    context          = vxGetContext((vx_reference)node);
    graph            = vxCreateGraph(context);

    if (graph == VX_NULL) return VX_ERROR_INVALID_GRAPH;

    i = 0;
    ds = 4;
    shiftScalar = vxCreateScalar(context, VX_TYPE_INT32, &ds);

    for (i = 0; i < vxmLENGTH_OF(virtImages); i++)
    {
        virtImages[i] = i != 4 ? vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_VIRT) :
                                 vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8);
    }

    nodes[0] = vxSobelMxNNode(graph, srcImage, winScalar, virtImages[0], virtImages[1]),
    nodes[1] = vxHarrisScoreNode(graph, virtImages[0], virtImages[1], senScalar, blkScalar, virtImages[2]),
    nodes[2] = vxEuclideanNonMaxNode(graph, virtImages[2], strScalar, minScalar, virtImages[3]),
    nodes[3] = vxImageListerNode(graph, virtImages[3], array, numCornersScalar);

    status = VX_SUCCESS;
    status |= vxoAddParameterToGraphByIndex(graph, nodes[0], 0);
    status |= vxoAddParameterToGraphByIndex(graph, nodes[2], 1);
    status |= vxoAddParameterToGraphByIndex(graph, nodes[2], 2);
    status |= vxoAddParameterToGraphByIndex(graph, nodes[1], 2);
    status |= vxoAddParameterToGraphByIndex(graph, nodes[0], 1);
    status |= vxoAddParameterToGraphByIndex(graph, nodes[1], 3);
    status |= vxoAddParameterToGraphByIndex(graph, nodes[3], 1);
    status |= vxoAddParameterToGraphByIndex(graph, nodes[3], 2);

    for (i = 0; i < vxmLENGTH_OF(nodes); i++)
    {
        vxReleaseNode(&nodes[i]);
    }
    for (i = 0; i < vxmLENGTH_OF(virtImages); i++)
    {
        vxReleaseImage(&virtImages[i]);
    }
    vxReleaseScalar(&shiftScalar);

    status |= vxVerifyGraph(graph);

    if (status == VX_SUCCESS)
    {
        status = vxoNode_SetChildGraph(node, graph);
    }
    else
    {
        vxReleaseGraph(&graph);
    }

    return status;
}

VX_PRIVATE_API vx_status vxoHarris_Deinitializer(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_graph graph = VX_NULL;

    if (num != vxmLENGTH_OF(basekernel_harris_params)) return VX_ERROR_INVALID_PARAMETERS;

    graph = vxoNode_GetChildGraph(node);

    vxReleaseGraph(&graph);

    vxoNode_SetChildGraph(node, 0);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoBaseKernel_Fast9Corners(vx_node node, vx_reference parameters[], vx_uint32 num)
{
    vx_border_mode_t bordermode;
    vx_image         inputImage;
    vx_scalar        sens;
    vx_scalar        nonm;
    vx_array         points;
    vx_scalar        numCorners;

    if (num != 5) return VX_ERROR_INVALID_PARAMETERS;

    inputImage  = (vx_image)parameters[0];
    sens        = (vx_scalar)parameters[1];
    nonm        = (vx_scalar)parameters[2];
    points      = (vx_array)parameters[3];
    numCorners  = (vx_scalar)parameters[4];

    if (vxQueryNode(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &bordermode, sizeof(bordermode)) == VX_SUCCESS)
    {
		return vxFast9Corners(inputImage, sens, nonm, points, numCorners, &bordermode, node->kernelAttributes.stagings);
    }

    return VX_FAILURE;
}

VX_PRIVATE_API vx_status vxoFast9_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData[3] = {{0}};
    vx_float32       k = 0.0f;
    vx_bool          nonmax = vx_false_e;

    if (index != 0 && index != 1 && index != 2) return VX_ERROR_INVALID_PARAMETERS;
    switch (index)
    {
        case 0:
            if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData[0]) != VX_SUCCESS)
                return VX_ERROR_INVALID_PARAMETERS;

            if (objData[0].u.imageInfo.format != VX_DF_IMAGE_U8) return VX_ERROR_INVALID_FORMAT;

            break;

        case 1:
            objData[1].u.scalarInfo.scalarValuePtr = &k;

            if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_SCALAR, &objData[1]) != VX_SUCCESS)
                return VX_ERROR_INVALID_PARAMETERS;

            if (objData[1].u.scalarInfo.dataType != VX_TYPE_FLOAT32) return VX_ERROR_INVALID_TYPE;

            if ((k <= 0) || (k >= 256)) return VX_ERROR_INVALID_VALUE;

            break;
        case 2:
            objData[2].u.scalarInfo.scalarValuePtr = &nonmax;
            if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_SCALAR, &objData[2]) != VX_SUCCESS)
                return VX_ERROR_INVALID_PARAMETERS;

            if (objData[2].u.scalarInfo.dataType != VX_TYPE_BOOL) return VX_ERROR_INVALID_TYPE;

            if ((nonmax != vx_false_e) && (nonmax != vx_true_e)) return VX_ERROR_INVALID_VALUE;
            break;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoFast9_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    if (index != 3 && index != 4) return VX_ERROR_INVALID_PARAMETERS;

    switch (index)
    {
    case 3:
        ptr->type                 = VX_TYPE_ARRAY;
        ptr->u.arrayInfo.itemType = VX_TYPE_KEYPOINT;
        ptr->u.arrayInfo.capacity = 0;
        break;
    case 4:
        ptr->u.scalarInfo.type    = VX_TYPE_UINT32;
        break;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoFast9_Initializer(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
    vx_rectangle_t rect;
    vx_enum itemType;
	vx_image src = (vx_image)parameters[0];
    vx_array arrays = (vx_array)parameters[3];
    status |= vxGetValidRegionImage(src, &rect);

	node->kernelAttributes.stagings[0] = (vx_reference)vxCreateImage(vxGetContext((vx_reference)src), rect.end_x, rect.end_y, VX_DF_IMAGE_U8);
    node->kernelAttributes.stagings[1] = (vx_reference)vxCreateImage(vxGetContext((vx_reference)src), rect.end_x, rect.end_y, VX_DF_IMAGE_U8);
    node->kernelAttributes.stagings[2] = (vx_reference)vxCreateImage(vxGetContext((vx_reference)src), 2, rect.end_y, VX_DF_IMAGE_U16);

	if (!vxoImage_AllocateMemory((vx_image)node->kernelAttributes.stagings[0])
		|| !vxoImage_AllocateMemory((vx_image)node->kernelAttributes.stagings[1])
        || !vxoImage_AllocateMemory((vx_image)node->kernelAttributes.stagings[2]))
		status |= VX_ERROR_NO_MEMORY;

    if (arrays)
    {
        vxQueryArray(arrays, VX_ARRAY_ATTRIBUTE_ITEMTYPE, &itemType, sizeof(itemType));
        node->kernelAttributes.stagings[3] = (vx_reference)vxCreateArray(vxGetContext((vx_reference)arrays), itemType, rect.end_x*rect.end_y);

        if (!vxoArray_AllocateMemory((vx_array)node->kernelAttributes.stagings[3]))
        {
            status |= VX_ERROR_NO_MEMORY;
        }

    }

    return status;
}

VX_PRIVATE_API vx_status vxoFast9_Deinitializer(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

	status |= vxReleaseImage((vx_image*)&node->kernelAttributes.stagings[0]);
	status |= vxReleaseImage((vx_image*)&node->kernelAttributes.stagings[1]);
    status |= vxReleaseImage((vx_image*)&node->kernelAttributes.stagings[2]);
    if (node->kernelAttributes.stagings[3])
    {
        status |= vxReleaseArray((vx_array*)&node->kernelAttributes.stagings[3]);
    }

    return status;
}

VX_PRIVATE_API vx_status vxoBaseKernel_OpticalFlowPyrLK(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

    vx_pyramid oldPyramid                 =  (vx_pyramid)parameters[0];
    vx_pyramid newPyramid               =  (vx_pyramid)parameters[1];
    vx_array        prevPts                         =  (vx_array)parameters[2];
    vx_array        estimatedPts              =  (vx_array)parameters[3];
    vx_array        nextPts                         =  (vx_array)parameters[4];
    vx_scalar      criteriaScalar               =  (vx_scalar)parameters[5];
    vx_scalar      epsilonScalar              =  (vx_scalar)parameters[6];
    vx_scalar      numIterationsScalar =  (vx_scalar)parameters[7];
    vx_scalar      useInitialEstimate     =  (vx_scalar)parameters[8];
    vx_scalar      winSizeScalar              =  (vx_scalar)parameters[9];

    vx_bool    isUseInitialEstimate = vx_false_e;
    vx_size    listLength = 0, maxLevel = 0;
    vx_int32  level = 0;
    vx_float32 pyramidScale = 0.5f;
    vx_image prevImg[10];
    vx_image nextImg[10];
    vx_border_mode_t bordermode = { VX_BORDER_MODE_UNDEFINED, 0 };

    if (num != 10) return VX_ERROR_INVALID_PARAMETERS;

    node->kernelAttributes.isAllGPU = vx_false_e;

    /* checking and preparing */
    vxAccessScalarValue(useInitialEstimate, &isUseInitialEstimate);
    vxQueryArray(prevPts, VX_ARRAY_ATTRIBUTE_NUMITEMS, &listLength, sizeof(listLength));
    if (isUseInitialEstimate)
    {
        vx_size estimateListLength = 0;

        vxQueryArray(estimatedPts, VX_ARRAY_ATTRIBUTE_NUMITEMS, &estimateListLength, sizeof(vx_size));
        if (estimateListLength != listLength) return VX_ERROR_INVALID_PARAMETERS;
    }
    vxSetArrayAttribute(nextPts, VX_ARRAY_ATTRIBUTE_NUMITEMS, &listLength, sizeof(listLength));

    /* generate original, new and scharr images */
    vxQueryPyramid(oldPyramid, VX_PYRAMID_ATTRIBUTE_LEVELS, &maxLevel, sizeof(maxLevel));
    vxQueryPyramid(oldPyramid, VX_PYRAMID_ATTRIBUTE_SCALE , &pyramidScale, sizeof(pyramidScale));
    for(level=(vx_int32)maxLevel; level>0; level--)
    {
        prevImg[level-1]   = vxGetPyramidLevel(oldPyramid, level-1);
        nextImg[level-1] = vxGetPyramidLevel(newPyramid, level-1);

        status |= vxScharr3x3(prevImg[level-1], (vx_image)(node->kernelAttributes.stagings[level*2-2]), (vx_image)(node->kernelAttributes.stagings[level*2-1]), &bordermode);
    }

    /* do tracker */
    status |= VLKTracker(prevImg, nextImg, node->kernelAttributes.stagings,
                                               prevPts, estimatedPts, nextPts,
                                               criteriaScalar, epsilonScalar, numIterationsScalar, isUseInitialEstimate, winSizeScalar,
                                               (vx_int32)maxLevel, pyramidScale
                                               );

    /* to clean up */
    status |= gcfVX_Flush(gcvTRUE);

    for(level=(vx_int32)maxLevel; level>0; level--)
    {
        vxReleaseImage(&(prevImg[level-1]));
        vxReleaseImage(&(nextImg[level-1]));
    }

    return status;
}

VX_PRIVATE_API vx_status vxoOpticalFlowPyrLK_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData = {0};

    switch (index)
    {
    case 0:
    case 1:
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_PYRAMID, &objData) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData.u.pyramidInfo.numLevels == 0) return VX_ERROR_INVALID_VALUE;

        break;
    case 2:
    case 3:
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_ARRAY, &objData) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData.u.arrayInfo.dataType != VX_TYPE_KEYPOINT) return VX_ERROR_INVALID_TYPE;

        break;

    case 5:
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_SCALAR, &objData) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData.u.scalarInfo.dataType != VX_TYPE_ENUM) return VX_ERROR_INVALID_TYPE;

        break;

    case 6:
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_SCALAR, &objData) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData.u.scalarInfo.dataType != VX_TYPE_FLOAT32) return VX_ERROR_INVALID_TYPE;

        break;

    case 7:
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_SCALAR, &objData) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData.u.scalarInfo.dataType != VX_TYPE_UINT32) return VX_ERROR_INVALID_TYPE;

        break;

    case 8:
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_SCALAR, &objData) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData.u.scalarInfo.dataType != VX_TYPE_BOOL) return VX_ERROR_INVALID_TYPE;

        break;

    case 9:
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_SCALAR, &objData) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData.u.scalarInfo.dataType != VX_TYPE_SIZE) return VX_ERROR_INVALID_TYPE;

        break;

    default:
        return VX_ERROR_INVALID_PARAMETERS;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoOpticalFlowPyrLK_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    vx_object_data_s objData;

    if (index != 4) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 2, VX_TYPE_ARRAY, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    ptr->type = VX_TYPE_ARRAY;
    ptr->u.arrayInfo.itemType = VX_TYPE_KEYPOINT;
    ptr->u.arrayInfo.capacity = objData.u.arrayInfo.capacity;

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoOpticalFlowPyrLK_Initializer(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 level = 0;
    vx_size maxLevel = 0;
    vx_image oldImage = VX_NULL;
    vx_rectangle_t rec = { 0 , 0, 0, 0 };
    vx_pyramid oldPyramid = (vx_pyramid)parameters[0];

    vxQueryPyramid(oldPyramid, VX_PYRAMID_ATTRIBUTE_LEVELS, &maxLevel, sizeof(maxLevel));
    if((0==maxLevel)||(maxLevel>5)) return VX_ERROR_INVALID_PARAMETERS;

    for(level=(vx_uint32)maxLevel; level>0; level--)
    {
        oldImage = vxGetPyramidLevel(oldPyramid, level-1);

        status |= vxGetValidRegionImage(oldImage, &rec);

        node->kernelAttributes.stagings[level*2-2] = (vx_reference)vxCreateImage(vxGetContext((vx_reference)oldImage), (rec.end_x-rec.start_x), (rec.end_y-rec.start_y), VX_DF_IMAGE_S16);
        node->kernelAttributes.stagings[level*2-1] = (vx_reference)vxCreateImage(vxGetContext((vx_reference)oldImage), (rec.end_x-rec.start_x), (rec.end_y-rec.start_y), VX_DF_IMAGE_S16);

        if (!vxoImage_AllocateMemory((vx_image)(node->kernelAttributes.stagings[level*2-2]))
            ||!vxoImage_AllocateMemory((vx_image)(node->kernelAttributes.stagings[level*2-1])))
            status |= VX_ERROR_NO_MEMORY;

        vxReleaseImage(&oldImage);
    }

    return status;
}

VX_PRIVATE_API vx_status vxoOpticalFlowPyrLK_Deinitializer(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
    vx_size level = 0;
    vx_size maxLevel = 0;
    vx_pyramid oldPyramid = (vx_pyramid)parameters[0];

    vxQueryPyramid(oldPyramid, VX_PYRAMID_ATTRIBUTE_LEVELS, &maxLevel, sizeof(maxLevel));
    if((0==maxLevel)||(maxLevel>5)) return VX_ERROR_INVALID_PARAMETERS;

    for(level=maxLevel; level>0; level--)
    {
	    status |= vxReleaseImage((vx_image*)&(node->kernelAttributes.stagings[level*2-2]));
	    status |= vxReleaseImage((vx_image*)&(node->kernelAttributes.stagings[level*2-1]));
    }

    return status;
}

VX_PRIVATE_API vx_status vxoBaseKernel_Remap(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_image         inputImage;
    vx_remap         remapTable;
    vx_scalar        scalarType;
    vx_image         outputImage;
    vx_enum          policy = 0;
    vx_border_mode_t borders;

    if (num != 4) return VX_ERROR_INVALID_PARAMETERS;

    inputImage = (vx_image)parameters[0];
    remapTable = (vx_remap)parameters[1];
    scalarType = (vx_scalar)parameters[2];
    outputImage = (vx_image)parameters[3];

    vxAccessScalarValue(scalarType, &policy);

    vxQueryNode(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &borders, sizeof(borders));

    return vxRemap(inputImage, remapTable, policy, &borders, outputImage);
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

/* internal kernel */
VX_PRIVATE_API vx_status vxoInternalKernel_SobelMxN(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_image inputImage      = VX_NULL;
    vx_scalar winScalar      = VX_NULL;
    vx_image grad_x          = VX_NULL;
    vx_image grad_y          = VX_NULL;
    vx_border_mode_t borders = {VX_BORDER_MODE_UNDEFINED, 0};;

    if (num != 4) return VX_ERROR_INVALID_PARAMETERS;

    inputImage  = (vx_image)parameters[0];
    winScalar   = (vx_scalar)parameters[1];
    grad_x      = (vx_image)parameters[2];
    grad_y      = (vx_image)parameters[3];

    if (vxQueryNode(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &borders, sizeof(borders)) == VX_SUCCESS)
    {
        return vxSobelMxN(inputImage, winScalar, grad_x, grad_y, &borders);
    }

    return VX_FAILURE;
}

VX_PRIVATE_API vx_status vxoGradientMxN_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData = {0};
    vx_uint32        winSize = 0;

    if (index != 0 && index != 1) return VX_ERROR_INVALID_PARAMETERS;

    switch (index)
    {
    case 0:
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData.u.imageInfo.width < 3 || objData.u.imageInfo.height < 3) return VX_ERROR_INVALID_VALUE;

        if (objData.u.imageInfo.format != VX_DF_IMAGE_U8) return VX_ERROR_INVALID_FORMAT;

        break;
    case 1:
        objData.u.scalarInfo.scalarValuePtr = &winSize;
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_SCALAR, &objData) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData.u.scalarInfo.dataType != VX_TYPE_INT32) return VX_ERROR_INVALID_TYPE;

        if (winSize != 3 && winSize != 5 && winSize != 7) return VX_ERROR_INVALID_VALUE;

        break;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoGradientMxN_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta)
{
    vx_object_data_s objData = {0};

    if (index != 2 && index != 3) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    vxoFillMetaData(meta, VX_TYPE_IMAGE, VX_DF_IMAGE_S16, objData.u.imageInfo.width, objData.u.imageInfo.height, 0);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoInternalKernel_HarrisScore(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_image         grad_x   = VX_NULL;
    vx_image         grad_y   = VX_NULL;
    vx_scalar        sens     = VX_NULL;
    vx_scalar        winds    = VX_NULL;
    vx_image         dstImage = VX_NULL;
    vx_border_mode_t borders  = {VX_BORDER_MODE_UNDEFINED, 0};

    if (num != 5) return VX_ERROR_INVALID_PARAMETERS;

     grad_x   = (vx_image)parameters[0];
     grad_y   = (vx_image)parameters[1];
     sens     = (vx_scalar)parameters[2];
     winds    = (vx_scalar)parameters[3];
     dstImage = (vx_image)parameters[4];

     if (vxQueryNode(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &borders, sizeof(borders)) == VX_SUCCESS)
     {
         return vxHarrisScore(grad_x, grad_y, dstImage, winds, sens, borders);
     }

     return VX_FAILURE;
}

VX_PRIVATE_API vx_status vxoHarrisScore_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData = {0};
    vx_int32 size = 0;

    if (index != 0 && index != 1 && index != 2 && index != 3) return VX_ERROR_INVALID_PARAMETERS;

    switch (index)
    {
        case 0:
        case 1:
            if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
                return VX_ERROR_INVALID_PARAMETERS;

            if (objData.u.imageInfo.format != VX_DF_IMAGE_S16) return VX_ERROR_INVALID_PARAMETERS;

            break;
        case 2:
            if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_SCALAR, &objData) != VX_SUCCESS)
                return VX_ERROR_INVALID_PARAMETERS;

            if (objData.u.scalarInfo.dataType != VX_TYPE_FLOAT32) return VX_ERROR_INVALID_TYPE;

            break;
        case 3:
            objData.u.scalarInfo.scalarValuePtr = &size;
            if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_SCALAR, &objData) != VX_SUCCESS)
                return VX_ERROR_INVALID_PARAMETERS;

            if (objData.u.scalarInfo.dataType != VX_TYPE_INT32) return VX_ERROR_INVALID_TYPE;

            if (size != 3 && size != 5 && size != 7) return VX_ERROR_INVALID_VALUE;

            break;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoHarrisScore_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta)
{
    vx_object_data_s objData = {0};

    if (index != 4) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    vxoFillMetaData(meta, VX_TYPE_IMAGE, VX_DF_IMAGE_F32, objData.u.imageInfo.width, objData.u.imageInfo.height, 0);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoInternalKernel_EuclideanNonMaxSuppression(vx_node node, vx_reference parameters[], vx_uint32 num)
{
    vx_image  srcImage;
    vx_scalar thresh;
    vx_scalar radius;
    vx_image  dstImage;

    if (num != 4) return VX_ERROR_INVALID_PARAMETERS;

    srcImage = (vx_image) parameters[0];
    thresh = (vx_scalar)parameters[1];
    radius = (vx_scalar)parameters[2];
    dstImage = (vx_image) parameters[3];

    return vxEuclideanNonMaxSuppression(srcImage, thresh, radius, dstImage);
}

VX_PRIVATE_API vx_status vxoEuclideanNonMax_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData = {0};
    vx_float32 radius = 0;


    if (index != 0 && index != 1 && index != 2) return VX_ERROR_INVALID_PARAMETERS;

    switch (index)
    {
    case 0:
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if ((objData.u.imageInfo.format != VX_DF_IMAGE_S32) &&
            (objData.u.imageInfo.format != VX_DF_IMAGE_F32))
        {
            return VX_ERROR_INVALID_FORMAT;
        }
        break;
    case 1:
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_SCALAR, &objData) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData.u.scalarInfo.dataType != VX_TYPE_FLOAT32) return VX_ERROR_INVALID_TYPE;

        break;
    case 2:
        objData.u.scalarInfo.scalarValuePtr = &radius;
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_SCALAR, &objData) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData.u.scalarInfo.dataType != VX_TYPE_FLOAT32) return VX_ERROR_INVALID_TYPE;

        if ((radius <= 1.0) || (radius > 5.0)) return VX_ERROR_INVALID_VALUE;

        break;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoEuclideanNonMax_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta)
{
    vx_object_data_s objData = {0};

    if (index != 3) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    vxoFillMetaData(meta, VX_TYPE_IMAGE, objData.u.imageInfo.format, objData.u.imageInfo.width, objData.u.imageInfo.height, 0);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoInternalKernel_ImageLister(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_image  srcImage        = (vx_image)parameters[0];
    vx_array  array           = (vx_array)parameters[1];
    vx_scalar numPointsScalar = (vx_scalar)parameters[2];

    if (num != 3) return VX_ERROR_INVALID_PARAMETERS;
    return vxImageLister(srcImage, array, numPointsScalar, node->kernelAttributes.stagings);
}

VX_PRIVATE_API vx_status vxoLister_Initializer(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 width = 0, height = 0;
    vx_enum itemType;
	vx_image src = (vx_image)parameters[0];
    vx_array arrays = (vx_array)parameters[1];

    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));

    node->kernelAttributes.stagings[0] = (vx_reference)vxCreateImage(vxGetContext((vx_reference)src), 2, height, VX_DF_IMAGE_U16);

	if (!vxoImage_AllocateMemory((vx_image)node->kernelAttributes.stagings[0]))
    {
		status |= VX_ERROR_NO_MEMORY;
    }

    if (arrays)
    {
        vxQueryArray(arrays, VX_ARRAY_ATTRIBUTE_ITEMTYPE, &itemType, sizeof(itemType));
        node->kernelAttributes.stagings[1] = (vx_reference)vxCreateArray(vxGetContext((vx_reference)arrays), itemType, width*height);

        if (!vxoArray_AllocateMemory((vx_array)node->kernelAttributes.stagings[1]))
        {
            status |= VX_ERROR_NO_MEMORY;
        }

    }

    return status;
}

VX_PRIVATE_API vx_status vxoLister_Deinitializer(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

	status |= vxReleaseImage((vx_image*)&node->kernelAttributes.stagings[0]);
    if (node->kernelAttributes.stagings[1])
    {
        status |= vxReleaseArray((vx_array*)&node->kernelAttributes.stagings[1]);
    }

    return status;
}

VX_PRIVATE_API vx_status vxoLister_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData = {0};

    if (index != 0) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    if ((objData.u.imageInfo.format != VX_DF_IMAGE_U8) &&
        (objData.u.imageInfo.format != VX_DF_IMAGE_S16) &&
        (objData.u.imageInfo.format != VX_DF_IMAGE_S32) &&
        (objData.u.imageInfo.format != VX_DF_IMAGE_F32))
    {
        return VX_ERROR_INVALID_FORMAT;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoLister_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta)
{
    if (index != 1 && index != 2) return VX_ERROR_INVALID_PARAMETERS;

    switch (index)
    {
    case 1:
        meta->u.arrayInfo.capacity = 0ul;
        meta->u.arrayInfo.itemType = VX_TYPE_KEYPOINT;
        break;
    case 2:
        meta->u.scalarInfo.type = VX_TYPE_UINT32;
        break;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoInternalKernel_Norm(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_image  inputX;
    vx_image  inputY;
    vx_scalar normType;
    vx_image  output;

    if (num != 4) return VX_ERROR_INVALID_PARAMETERS;

    inputX   = (vx_image)parameters[0];
    inputY   = (vx_image)parameters[1];
    normType = (vx_scalar)parameters[2];
    output   = (vx_image)parameters[3];

    return vxNorm(inputX, inputY, normType, output);
}

VX_PRIVATE_API vx_status vxoNorm_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData[2] = {{0}};
    vx_enum          value;

    if (index != 0 && index != 1 && index != 2) return VX_ERROR_INVALID_PARAMETERS;

    switch (index)
    {
    case 0:
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData[0]) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData[0].u.imageInfo.format != VX_DF_IMAGE_S16) return VX_ERROR_INVALID_FORMAT;

        break;
    case 1:
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData[0]) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData[0].u.imageInfo.format != VX_DF_IMAGE_S16) return VX_ERROR_INVALID_FORMAT;

        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData[1]) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData[0].u.imageInfo.width != objData[1].u.imageInfo.width &&
            objData[0].u.imageInfo.height != objData[1].u.imageInfo.height)
        {
            return VX_ERROR_INVALID_VALUE;
        }

        break;
    case 2:
        objData[0].u.scalarInfo.scalarValuePtr = &value;
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_SCALAR, &objData[0]) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData[0].u.scalarInfo.dataType != VX_TYPE_ENUM) return VX_ERROR_INVALID_TYPE;

        if (value != VX_NORM_L1 && value != VX_NORM_L2) return VX_ERROR_INVALID_VALUE;

        break;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoNorm_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta)
{
    vx_object_data_s objData = {0};

    if (index != 3) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    vxoFillMetaData(meta, VX_TYPE_IMAGE, VX_DF_IMAGE_U16, objData.u.imageInfo.width, objData.u.imageInfo.height, 0);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoInternalKernel_NonMaxSuppression(vx_node node, vx_reference parameters[], vx_uint32 num)
{
    vx_image i_mag;
    vx_image i_ang;
    vx_image i_edge;
    vx_border_mode_t borders;

    if (num != 3) return VX_ERROR_INVALID_PARAMETERS;

    i_mag  = (vx_image)parameters[0];
    i_ang  = (vx_image)parameters[1];
    i_edge = (vx_image)parameters[2];

    vxQueryNode(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &borders, sizeof(borders));

    return vxNonMaxSuppression(i_mag, i_ang, i_edge, &borders);

}

VX_PRIVATE_API vx_status vxoNonMaxSuppression_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData[2] = {{0}};

    if (index != 0 && index != 1) return VX_ERROR_INVALID_PARAMETERS;

    switch (index)
    {
    case 0:
        if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData[0]) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData[0].u.imageInfo.format != VX_DF_IMAGE_U8 &&
            objData[0].u.imageInfo.format != VX_DF_IMAGE_S16 &&
            objData[0].u.imageInfo.format != VX_DF_IMAGE_U16)
        {
            return VX_ERROR_INVALID_FORMAT;
        }
        break;
    case 1:
        if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData[0]) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (vxoGetObjAttributeByNodeIndex(node, 1, VX_TYPE_IMAGE, &objData[1]) != VX_SUCCESS)
            return VX_ERROR_INVALID_PARAMETERS;

        if (objData[1].u.imageInfo.format != VX_DF_IMAGE_U8) return VX_ERROR_INVALID_FORMAT;

        if ((objData[0].u.imageInfo.width != objData[1].u.imageInfo.width) ||
            (objData[0].u.imageInfo.height != objData[1].u.imageInfo.height))
        {
            return VX_ERROR_INVALID_VALUE;
        }

        break;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoNonMaxSuppression_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta)
{
    vx_object_data_s objData = {0};

    if (index != 2) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    vxoFillMetaData(meta, VX_TYPE_IMAGE, objData.u.imageInfo.format, objData.u.imageInfo.width, objData.u.imageInfo.height, 0);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoInternalKernel_EdgeTrace(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_image     norm;
    vx_threshold threshold;
    vx_image     output;

    if (num != 3) return VX_ERROR_INVALID_PARAMETERS;

    norm      = (vx_image)parameters[0];
    threshold = (vx_threshold)parameters[1];
    output    = (vx_image)parameters[2];

	return vxEdgeTrace(norm, threshold, output, node->kernelAttributes.stagings);
}

VX_PRIVATE_API vx_status vxoEdgeTrace_ValidateInput(vx_node node, vx_uint32 index)
{
    vx_object_data_s objData = {0};

    if (index != 0 && index != 1) return VX_ERROR_INVALID_PARAMETERS;

    switch (index)
    {
        case 0:
            if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
                return VX_ERROR_INVALID_PARAMETERS;

            if (objData.u.imageInfo.format != VX_DF_IMAGE_U16) return VX_ERROR_INVALID_FORMAT;

            break;
        case 1:
            if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_THRESHOLD, &objData) != VX_SUCCESS)
                return VX_ERROR_INVALID_PARAMETERS;

            if (objData.u.thresholdInfo.dataType != VX_THRESHOLD_TYPE_RANGE) return VX_ERROR_INVALID_TYPE;

            break;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoEdgeTrace_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta)
{
    vx_object_data_s objData = {0};

    if (index != 2) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, 0, VX_TYPE_IMAGE, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    vxoFillMetaData(meta, VX_TYPE_IMAGE, VX_DF_IMAGE_U8, objData.u.imageInfo.width, objData.u.imageInfo.height, 0);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoEdgeTrace_Initializer(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
    vx_rectangle_t rect;
    vx_uint32 count = 0;
	vx_image output = (vx_image)parameters[2];

    status |= vxGetValidRegionImage(output, &rect);

	node->kernelAttributes.stagings[0] = (vx_reference)vxCreateImage(vxGetContext((vx_reference)output), rect.end_x, rect.end_y, VX_DF_IMAGE_U8);
    node->kernelAttributes.stagings[1] = (vx_reference)vxCreateImage(vxGetContext((vx_reference)output), rect.end_x, rect.end_y, VX_DF_IMAGE_U8);
    node->kernelAttributes.stagings[2] = (vx_reference)vxCreateScalar(vxGetContext((vx_reference)output), VX_TYPE_UINT32, &count);

	if (!vxoImage_AllocateMemory((vx_image)node->kernelAttributes.stagings[0])
		|| !vxoImage_AllocateMemory((vx_image)node->kernelAttributes.stagings[1]))
		status |= VX_ERROR_NO_MEMORY;
    vxCommitScalarValue((vx_scalar)node->kernelAttributes.stagings[2], &count);

    return status;
}

VX_PRIVATE_API vx_status vxoEdgeTrace_Deinitializer(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

	status |= vxReleaseImage((vx_image*)&node->kernelAttributes.stagings[0]);
	status |= vxReleaseImage((vx_image*)&node->kernelAttributes.stagings[1]);
    status |= vxReleaseScalar((vx_scalar*)&node->kernelAttributes.stagings[2]);

    return status;
}

vx_kernel_description_s *target_kernels[] = {
    &invalid_kernel,
    &basekernel_colorconvert,
    &basekernel_channelextract,
    &basekernel_channelcombine,
    &basekernel_sobel3x3,
    &basekernel_magnitude,
    &basekernel_phase,
    &basekernel_scale_image,
    &basekernel_lut,
    &basekernel_histogram,
    &basekernel_equalize_hist,
    &basekernel_absdiff,
    &basekernel_mean_stddev,
    &basekernel_threshold,
    &basekernel_integral_image,
    &basekernel_erode3x3,
    &basekernel_dilate3x3,
    &basekernel_median3x3,
    &basekernel_box3x3,
    &basekernel_box3x3_2,
    &basekernel_gaussian3x3,
    &basekernel_convolution,
    &basekernel_pyramid,
    &basekernel_accumulate,
    &basekernel_accumulate_weighted,
    &basekernel_accumulate_square,
    &basekernel_minmaxloc,
    &basekernel_convertdepth,
    &basekernel_canny,
    &basekernel_and,
    &basekernel_or,
    &basekernel_xor,
    &basekernel_not,
    &basekernel_multiply,
    &basekernel_add,
    &basekernel_subtract,
    &basekernel_warp_affine,
    &basekernel_warp_perspective,
    &basekernel_harris,
    &basekernel_fast9,
    &basekernel_optpyrlk,
    &basekernel_remap,
    &basekernel_halfscale_gaussian,
    &internalkernel_sobelMxN,
    &internalkernel_harris_score,
    &internalkernel_euclidian_nonmax,
    &internalkernel_lister,
    &internalkernel_norm,
    &internalkernel_nonmax,
    &internalkernel_edge_trace
};

vx_uint32 num_target_kernels = vxmLENGTH_OF(target_kernels);



