/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <gc_vx_common.h>
#include <gc_vx_internal_node_api.h>

#define _GC_OBJ_ZONE            gcdZONE_VX_NODE

VX_INTERNAL_API vx_node vxSobelMxNNode(vx_graph graph, vx_image input, vx_scalar ws, vx_image gx, vx_image gy)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)ws,
        (vx_reference)gx,
        (vx_reference)gy
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_SOBEL_MxN, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxSobelMxNF16Node(vx_graph graph, vx_image input, vx_scalar ws, vx_scalar shift, vx_image gx, vx_image gy)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)ws,
        (vx_reference)shift,
        (vx_reference)gx,
        (vx_reference)gy
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_SOBEL_MxN_F16, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxHarrisScoreNode(
        vx_graph graph, vx_image gx, vx_image gy, vx_scalar sensitivity, vx_scalar winSize, vx_scalar blockSize, vx_scalar shift, vx_image score)
{
    vx_reference parameters[] = {
        (vx_reference)gx,
        (vx_reference)gy,
        (vx_reference)sensitivity,
        (vx_reference)winSize,
        (vx_reference)blockSize,
        (vx_reference)shift,
        (vx_reference)score
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_HARRIS_SCORE, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxEuclideanNonMaxNode(
        vx_graph graph, vx_image input, vx_scalar strengthThresh, vx_scalar minDistance, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)strengthThresh,
        (vx_reference)minDistance,
        (vx_reference)output
    };

    return vxoNode_CreateSpecific(
                graph, VX_KERNEL_INTERNAL_EUCLIDEAN_NONMAXSUPPRESSION, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxImageListerNode(vx_graph graph, vx_image input, vx_array arr, vx_scalar numPoints)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)arr,
        (vx_reference)numPoints
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_IMAGE_LISTER, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxElementwiseNormNode(
        vx_graph graph, vx_image input_x, vx_image input_y, vx_scalar norm_type, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input_x,
        (vx_reference)input_y,
        (vx_reference)norm_type,
        (vx_reference)output
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_ELEMENTWISE_NORM, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxElementwiseNormF16Node(
        vx_graph graph, vx_image input_x, vx_image input_y, vx_scalar norm_type, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input_x,
        (vx_reference)input_y,
        (vx_reference)norm_type,
        (vx_reference)output
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_ELEMENTWISE_NORM_F16, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxPhaseF16Node(vx_graph graph, vx_image grad_x, vx_image grad_y, vx_image orientation)
{
    vx_reference parameters[] = {
       (vx_reference)grad_x,
       (vx_reference)grad_y,
       (vx_reference)orientation
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_PHASE_F16, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxNonMaxSuppressionCannyNode(vx_graph graph, vx_image mag, vx_image phase, vx_image edge)
{
    vx_reference parameters[] = {
        (vx_reference)mag,
        (vx_reference)phase,
        (vx_reference)edge
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_NONMAXSUPPRESSION_CANNY, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxEdgeTraceNode(vx_graph graph, vx_image norm, vx_threshold threshold, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)norm,
        (vx_reference)threshold,
        (vx_reference)output,
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_EDGE_TRACE, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxCopyImageNode(vx_graph graph, vx_image input, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)output
    };

    /* if parameter is created as vitual image, rewrite the elements of image as non-vitual image in the node */
    if(input->base.isVirtual == gcvTRUE)
    {
        gcmASSERT(output->base.isVirtual == gcvFALSE);
        input->format = output->format;
        input->planeCount = output->planeCount;
        input->memory.planeCount = output->memory.planeCount;
        input->width = output->width;
        input->height = output->height;
    }
    if(output->base.isVirtual == gcvTRUE)
    {
        gcmASSERT(input->base.isVirtual == gcvFALSE);
        output->format = input->format;
        output->planeCount = input->planeCount;
        output->memory.planeCount = input->memory.planeCount;
        output->width = input->width;
        output->height = input->height;
    }

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_COPY_IMAGE, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxFast9CornersStrengthNode(vx_graph graph, vx_image input, vx_scalar t, vx_scalar do_nonmax, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)t,
        (vx_reference)do_nonmax,
        (vx_reference)output
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_FAST9CORNERS_STRENGTH, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxFast9CornersNonMaxNode(vx_graph graph, vx_image input, vx_scalar t, vx_scalar do_nonmax, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)t,
        (vx_reference)do_nonmax,
        (vx_reference)output
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_FAST9CORNERS_NONMAX, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxCreateListerNode(vx_graph graph, vx_image input, vx_image countImg, vx_array array)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)countImg,
        (vx_reference)array
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_CREATE_LISTER, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxPackArraysNode(vx_graph graph, vx_image inputImage, vx_array inputArray, vx_scalar widthScalar, vx_scalar heightScalar, vx_array dstArray, vx_scalar numScalar)
{
    vx_reference parameters[] = {
        (vx_reference)inputImage,
        (vx_reference)inputArray,
        (vx_reference)widthScalar,
        (vx_reference)heightScalar,
        (vx_reference)dstArray,
        (vx_reference)numScalar
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_PACK_ARRAYS, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxMinMaxLocFilterNode(vx_graph graph, vx_image inputImage, vx_scalar filterMin, vx_scalar filterMax)
{
    vx_reference parameters[] = {
        (vx_reference)inputImage,
        (vx_reference)filterMin,
        (vx_reference)filterMax
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_MINMAXLOC_FILTER, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxGetLocationNode(vx_graph graph, vx_image inputImage, vx_scalar filterMinValue, vx_scalar filterMaxValue,
                  vx_image minImage, vx_image maxImage, vx_array minArray, vx_array maxArray, vx_scalar filterMinCount, vx_scalar filterMaxCount)
{
    vx_reference parameters[] = {
        (vx_reference)inputImage,
        (vx_reference)filterMinValue,
        (vx_reference)filterMaxValue,
        (vx_reference)minImage,
        (vx_reference)maxImage,
        (vx_reference)minArray,
        (vx_reference)maxArray,
        (vx_reference)filterMinCount,
        (vx_reference)filterMaxCount
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_GET_LOCATION, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxMinMaxLocPackArrayNode(vx_graph graph, vx_image inputImage, vx_array inputArray, vx_scalar widthScalar, vx_scalar heightScalar, vx_scalar countScalar, vx_array dstArray)
{
    vx_reference parameters[] = {
        (vx_reference)inputImage,
        (vx_reference)inputArray,
        (vx_reference)widthScalar,
        (vx_reference)heightScalar,
        (vx_reference)countScalar,
        (vx_reference)dstArray
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_MIN_MAX_LOC_PACK_ARRAYS, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxEdgeTraceThresholdNode(vx_graph graph, vx_image inputImage, vx_threshold threshold, vx_image outputImage)
{
    vx_reference parameters[] = {
        (vx_reference)inputImage,
        (vx_reference)threshold,
        (vx_reference)outputImage
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_EDGE_TRACE_THRESHOLD, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxEdgeTraceHysteresisNode(vx_graph graph, vx_image image, vx_scalar flag)
{
    vx_reference parameters[] = {
        (vx_reference)image,
        (vx_reference)flag
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_EDGE_TRACE_HYSTERESIS, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxEdgeTraceClampNode(vx_graph graph, vx_image inputImage, vx_image outputImage)
{
    vx_reference parameters[] = {
        (vx_reference)inputImage,
        (vx_reference)outputImage
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_EDGE_TRACE_CLAMP, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxIntegralImageStepNode(vx_graph graph, vx_image inputImage, vx_scalar stepScalar, vx_image outputImage)
{
    vx_reference parameters[] = {
        (vx_reference)inputImage,
        (vx_reference)stepScalar,
        (vx_reference)outputImage,
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_INTEGRAL_IMAGE_STEP, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxScharr3x3Node(vx_graph graph, vx_image inputImage, vx_image gradXImage, vx_image gradYImage)
{
    vx_reference parameters[] = {
        (vx_reference)inputImage,
        (vx_reference)gradXImage,
        (vx_reference)gradYImage,
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_SCHARR3x3, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxVLKTrackerNode(vx_graph graph, vx_pyramid oldPyramid, vx_pyramid newPyramid, vx_pyramid gradXPyramid, vx_pyramid gradYPyramid,
                                         vx_array prevPts, vx_array estimatedPts, vx_array nextPts,
                                         vx_scalar criteriaScalar, vx_scalar epsilonScalar, vx_scalar numIterationsScalar, vx_scalar isUseInitialEstimateScalar,
                                         vx_scalar winSizeScalar)
{
    vx_reference parameters[] = {
        (vx_reference)oldPyramid,
        (vx_reference)newPyramid,
        (vx_reference)gradXPyramid,
        (vx_reference)gradYPyramid,
        (vx_reference)prevPts,
        (vx_reference)estimatedPts,
        (vx_reference)nextPts,
        (vx_reference)criteriaScalar,
        (vx_reference)epsilonScalar,
        (vx_reference)numIterationsScalar,
        (vx_reference)isUseInitialEstimateScalar,
        (vx_reference)winSizeScalar
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_VLK_TRACKER, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxEqualizeHistHistNode(vx_graph graph, vx_image srcImage, vx_image histImage, vx_scalar minIndexScalar)
{
    vx_reference parameters[] = {
        (vx_reference)srcImage,
        (vx_reference)histImage,
        (vx_reference)minIndexScalar
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_EQUALIZE_HISTOGRAM_HIST, parameters, vxmLENGTH_OF(parameters));
}


VX_INTERNAL_API vx_node vxEqualizeHistGcdfNode(vx_graph graph, vx_image histImage, vx_scalar minIndexScalar, vx_image cdfImage, vx_scalar minValueScalar)
{
    vx_reference parameters[] = {
        (vx_reference)histImage,
        (vx_reference)minIndexScalar,
        (vx_reference)cdfImage,
        (vx_reference)minValueScalar
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_EQUALIZE_HISTOGRAM_GCDF, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxEqualizeHistCdfNode(vx_graph graph, vx_image srcImage, vx_image cdfImage, vx_scalar minValueScalar, vx_image histImage)
{
    vx_reference parameters[] = {
        (vx_reference)srcImage,
        (vx_reference)cdfImage,
        (vx_reference)minValueScalar,
        (vx_reference)histImage
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_EQUALIZE_HISTOGRAM_CDF, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxEqualizeHistLutNode(vx_graph graph, vx_image srcImage, vx_image histImage, vx_image dstImage)
{
    vx_reference parameters[] = {
        (vx_reference)srcImage,
        (vx_reference)histImage,
        (vx_reference)dstImage
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_EQUALIZE_HISTOGRAM_LUT, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxSgmCostNode(vx_graph graph, vx_image right_img, vx_image left_img, vx_scalar range, vx_image cost)
{
    vx_reference parameters[] = {
        (vx_reference)right_img,
        (vx_reference)left_img,
        (vx_reference)range,
        (vx_reference)cost
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_SGM_COST, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxSgmCostPath90Node(vx_graph graph, vx_image cost, vx_scalar range, vx_image path)
{
    vx_reference parameters[] = {
        (vx_reference)cost,
        (vx_reference)range,
        (vx_reference)path
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_SGM_PATH90, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxSgmCostPath45Node(vx_graph graph, vx_image cost, vx_scalar range, vx_image path)
{
    vx_reference parameters[] = {
        (vx_reference)cost,
        (vx_reference)range,
        (vx_reference)path
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_SGM_PATH45, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxSgmCostPath135Node(vx_graph graph, vx_image cost, vx_scalar range, vx_image path)
{
    vx_reference parameters[] = {
        (vx_reference)cost,
        (vx_reference)range,
        (vx_reference)path
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_SGM_PATH135, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxSgmCostPath0Node(vx_graph graph, vx_image cost, vx_scalar range, vx_image path)
{
    vx_reference parameters[] = {
        (vx_reference)cost,
        (vx_reference)range,
        (vx_reference)path
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_SGM_PATH0, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxSgmGetDispNode(vx_graph graph, vx_image path, vx_scalar range, vx_image depth)
{
    vx_reference parameters[] = {
        (vx_reference)path,
        (vx_reference)range,
        (vx_reference)depth
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_SGM_DISP, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxLaplacian3x3Node(vx_graph graph, vx_image src, vx_image dst)
{
    vx_reference parameters[] = {
        (vx_reference)src,
        (vx_reference)dst,
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_LAPLACIAN3x3, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxAdapterLayer(
    vx_graph                    graph,
    vx_tensor                   input,
    const vx_nn_reorg_params    reorg_params,
    vx_size                     size_of_reorg_params,
    vx_tensor                   output
    )
{
    vx_reference parameters[] = {
        (vx_reference)input,
        VX_NULL,
        (vx_reference)output
    };

    vx_node node = VX_NULL;
    vx_scalar type = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &reorg_params->type);
    if (vxoReference_GetStatus((vx_reference)type) != VX_SUCCESS) return (vx_node)type;

    parameters[1]  = (vx_reference)type;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_ADAPTER, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&type);

    return node;
}

VX_INTERNAL_API vx_node vxUpSamplePaddingNode(vx_graph graph, vx_image inputImage, vx_image outputImage)
{
    vx_reference parameters[] = {
        (vx_reference)inputImage,
        (vx_reference)outputImage
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_UPSAMPLE_PADDING, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxUpSampleConvertNode(vx_graph graph, vx_image inputImage, vx_image outputImage)
{
    vx_reference parameters[] = {
        (vx_reference)inputImage,
        (vx_reference)outputImage
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_UPSAMPLE_CONVERT, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxPyramidCopyImageNode(vx_graph graph, vx_image inputImage, vx_image outputImage)
{
    vx_reference parameters[] = {
        (vx_reference)inputImage,
        (vx_reference)outputImage
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_PYRAMID_COPY_IMAGE, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxTransPose2DTensorNode(vx_graph graph, vx_tensor inputTensor, vx_tensor outputTensor)
{
    vx_reference parameters[] = {
        (vx_reference)inputTensor,
        (vx_reference)outputTensor
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_TRANSPOSE_2D_TENSOR, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxMultiply2DMatrixesNode(vx_graph graph, vx_tensor inputTensor0, vx_tensor inputTensor1, vx_tensor inputTensor2, vx_scalar enable_tensorC, vx_tensor outputTensor)
{
    vx_reference parameters[] = {
        (vx_reference)inputTensor0,
        (vx_reference)inputTensor1,
        (vx_reference)inputTensor2,
        (vx_reference)enable_tensorC,
        (vx_reference)outputTensor
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_MULTIPLY_2D_MATRIXES, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxROIPoolingReluLayer(
    vx_graph  graph,
    vx_tensor input_data,
    vx_tensor input_rois,
    const vx_nn_roi_pool_params_t *roi_pool_params,
    vx_size size_of_roi_params,
    vx_tensor output_arr,
    vx_bool enable_relu
    )
{
    gcmHEADER_ARG("graph=%p, input_data=%p, input_rois=%p, roi_pool_params=%p, size_of_roi_params=0x%lx, output_arr=%p",
        graph, input_data, input_rois, roi_pool_params, size_of_roi_params, output_arr);
    gcmDUMP_API("$VX vxROIPoolingLayer: graph=%p, input_data=%p, input_rois=%p, roi_pool_params=%p, size_of_roi_params=0x%lx, output_arr=%p",
        graph, input_data, input_rois, roi_pool_params, size_of_roi_params, output_arr);


    if (size_of_roi_params != sizeof(vx_nn_roi_pool_params_ext_t))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    else
    {

        vx_reference parameters[] = {
            (vx_reference)input_data,
            (vx_reference)input_rois,
            NULL,
            NULL,
            NULL,
            NULL,
            (vx_reference)output_arr,
            NULL
        };

        vx_context context = vxGetContext((vx_reference)graph);
        vx_node node;

        vx_scalar pool_types = NULL;
        vx_scalar spatial_scales = NULL;
        vx_scalar pooled_heights = NULL;
        vx_scalar pooled_widths = NULL;
        vx_scalar relu = NULL;

        vx_nn_roi_pool_params_ext_t * roi_pool_params_ext = (vx_nn_roi_pool_params_ext_t *)roi_pool_params;

        pool_types = vxCreateScalar(context, VX_TYPE_ENUM, &roi_pool_params_ext->khr.pool_type);
        if (vxoReference_GetStatus((vx_reference)pool_types) != VX_SUCCESS) {
            gcmFOOTER_NO();
            return (vx_node)pool_types;
        }
        spatial_scales = vxCreateScalar(context, VX_TYPE_FLOAT32, &roi_pool_params_ext->spatial_scale);
        if (vxoReference_GetStatus((vx_reference)spatial_scales) != VX_SUCCESS) {
            gcmFOOTER_NO();
            return (vx_node)spatial_scales;
        }
        pooled_heights = vxCreateScalar(context, VX_TYPE_INT32, &roi_pool_params_ext->pooled_height);
        if (vxoReference_GetStatus((vx_reference)pooled_heights) != VX_SUCCESS) {
            gcmFOOTER_NO();
            return (vx_node)pooled_heights;
        }
        pooled_widths = vxCreateScalar(context, VX_TYPE_INT32, &roi_pool_params_ext->pooled_width);
        if (vxoReference_GetStatus((vx_reference)pooled_widths) != VX_SUCCESS) {
            gcmFOOTER_NO();
            return (vx_node)pooled_widths;
        }

        relu = vxCreateScalar(context, VX_TYPE_BOOL, &enable_relu);
        if (vxoReference_GetStatus((vx_reference)relu) != VX_SUCCESS) {
            gcmFOOTER_NO();
            return (vx_node)relu;
        }
        parameters[2]  = (vx_reference)pool_types;
        parameters[3]  = (vx_reference)spatial_scales;
        parameters[4]  = (vx_reference)pooled_heights;
        parameters[5]  = (vx_reference)pooled_widths;
        parameters[7]  = (vx_reference)relu;

        node = vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_ROI_POOLING_RELU_LAYER, parameters, vxmLENGTH_OF(parameters));

        vxReleaseScalar(&pool_types);
        vxReleaseScalar(&spatial_scales);
        vxReleaseScalar(&pooled_heights);
        vxReleaseScalar(&pooled_widths);
        vxReleaseScalar(&relu);

        gcmFOOTER_NO();
        return node;
    }
}

VX_INTERNAL_API vx_node vxConvolve5x5Node(vx_graph graph, vx_image input, vx_convolution conv, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)conv,
        (vx_reference)output
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_CONVOLVE5X5, parameters, vxmLENGTH_OF(parameters));
}

