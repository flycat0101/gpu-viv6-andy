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


#include <gc_nn_interface.h>

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
                vxReadScalarValue(scalar, objData->u.scalarInfo.scalarValuePtr);
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

VX_INTERNAL_API vx_status VX_CALLBACK vxoAddParameterToGraphByIndex(vx_graph graph, vx_node node, vx_uint32 index)
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
//LRN*****************************************************************
VX_PRIVATE_API vx_status VX_CALLBACK vxoLRN_ValidateInput(vx_node node, vx_uint32 index)
{
    if (index < 0 || index > 11) return VX_ERROR_INVALID_PARAMETERS;

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoLRN_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format  metaObj)
{
    vx_object_data_s objData = {0};

    if (index != 12) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoGetObjAttributeByNodeIndex(node, index, VX_TYPE_ARRAY, &objData) != VX_SUCCESS)
        return VX_ERROR_INVALID_PARAMETERS;

    metaObj->type                 = VX_TYPE_ARRAY;
    metaObj->u.arrayInfo.itemType = objData.u.arrayInfo.dataType;
    metaObj->u.arrayInfo.capacity = objData.u.arrayInfo.capacity;

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxLrnInitializer(vx_node nodObj, const vx_reference *paramObj, vx_uint32 paraNum)
{

#ifdef USE_LRN_VXC
    vx_kernel_execution_parameters_t shaderParam = {
        2,          // workdim
        {0, 0, 0},  // globalWorkOffset: control the start location be processed in the image
        {0, 0, 0},  // globalWorkScale: how many pixels could be processed by a single thread
        {0, 0, 0},  // localWorkSize: local group size in threads
        {0, 0, 0}}; // globalWorkSize: image size in threads

    vx_int32 width = 0;
    vx_scalar width_s = (vx_scalar)paramObj[1];
    vxReadScalarValue(width_s, &width);

    shaderParam.globalWorkOffset[0] = 0;
    shaderParam.globalWorkOffset[1] = 0;
    shaderParam.globalWorkScale[0]    = 4;
    shaderParam.globalWorkScale[1]    = 1;
    shaderParam.localWorkSize[0]    = 8;
    shaderParam.localWorkSize[1]    = 1;
    shaderParam.globalWorkSize[0]    = (((width + shaderParam.globalWorkScale[0]-1 ) / shaderParam.globalWorkScale[0]
                                      + shaderParam.localWorkSize[0] - 1) / shaderParam.localWorkSize[0])
                                     * shaderParam.localWorkSize[0];
    shaderParam.globalWorkSize[1]    = 1;//(imgHei - offset + shaderParam.globalWorkScale[1] - 1) / shaderParam.globalWorkScale[1];
    {
    vx_uint32 dp_fp16_1[16] = {
                0x15151515, // TCfg
                0x00000000, // ASelt
                0x03210210, 0x05430432, // ABin
                0x15151515, // BSelt
                0x03210210, 0x05430432, // BBin
                0x00000400, // AccumType, ConstantType, and PostShift
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
            };

       vx_uint32 dp_fp16tofp32[16] = {
        0x01010101, // TCfg
        0x00000000, // ASelt
        0x00020001, 0x00040003, // ABin
        0x02020202, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000100, // AccumType, ConstantType, and PostShift
        0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000 // Constant
    };

         vx_uint32 dp_u32tofp16[16] = {
        0x00001111, // TCfg
        0x00000000, // ASelt
        0x06040200, 0x00000000, // ABin
        0x00002222, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000400, // AccumType, ConstantType, and PostShift
        0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
    };
    vx_uint32 UniPackLow16bits[16] = {
        0x03030303, 0x00000000, // TCfg
        0x00200000, 0x60000400, 0x00000000, 0x00000000, 0x00000000, // BinSelect
        0x00006400, // AccumType, ConstantType, and PostShift
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
    };

    vxSetNodeUniform(nodObj, "UniPackLow16bits", 1, UniPackLow16bits);
    vxSetNodeUniform(nodObj, "dp_u32tofp16", 1, dp_u32tofp16);
    vxSetNodeUniform(nodObj, "dp_fp16tofp32", 1, dp_fp16tofp32);
    vxSetNodeUniform(nodObj, "dp_fp16_1", 1, dp_fp16_1);

    }
    vxSetNodeAttribute(nodObj, VX_NODE_ATTRIBUTE_KERNEL_EXECUTION_PARAMETERS, &shaderParam, sizeof(vx_kernel_execution_parameters_t));
#endif
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxLrnFunction(vx_node node, const vx_reference parameters[], vx_uint32 paramCount)
{
    vx_status   status = VX_SUCCESS;

    vx_reference *ucParameters = (vx_reference*)parameters;

    /*step1: pre-process */
    vx_image_s  imgObj, imgObj2;

    vx_array input_array = (vx_array)parameters[0];
    vx_scalar width = (vx_scalar)parameters[1];
    vx_scalar height = (vx_scalar)parameters[2];
    vx_scalar depth = (vx_scalar)parameters[3];
    vx_array output_array = (vx_array)parameters[12];

    int depth_s;
    int width_s, height_s;

    vxReadScalarValue(width, &width_s);
    vxReadScalarValue(depth, &depth_s);
    vxReadScalarValue(height, &height_s);

    {
        int itemSize = (int)input_array->itemSize;
        gcoOS_ZeroMemory(&imgObj, sizeof(vx_image_s));

        imgObj.base = input_array->base;
        imgObj.base.type = VX_TYPE_IMAGE;

        imgObj.width = width_s;
        imgObj.height = height_s * depth_s;
        imgObj.format = (itemSize == 4)?VX_DF_IMAGE_F32:VX_DF_IMAGE_S16;
        imgObj.planeCount = 1;
        imgObj.memory = input_array->memory;
    }

    {
        int itemSize = (int)output_array->itemSize;
        gcoOS_ZeroMemory(&imgObj2, sizeof(vx_image_s));

        imgObj2.base = output_array->base;
        imgObj2.base.type = VX_TYPE_IMAGE;
        imgObj2.width = width_s;
        imgObj2.height = height_s * depth_s;
        imgObj2.format = (itemSize == 4)?VX_DF_IMAGE_F32:VX_DF_IMAGE_S16;
        imgObj2.planeCount = 1;
        imgObj2.memory = output_array->memory;
    }


    ucParameters[0] = (vx_reference)(&imgObj);
    ucParameters[12] = (vx_reference)(&imgObj2);

    /*step2: call the base function */
    status = vxProgramKernel_Function(node, parameters, paramCount);

    /* step3 : post-process */
    ucParameters[0] = (vx_reference)(input_array);
    ucParameters[12] = (vx_reference)(output_array);

    gcoVX_Flush(gcvTRUE);
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxLrnDeinitializer(vx_node nodObj, const vx_reference *paraObj, vx_uint32 paraNum)
{
    return VX_SUCCESS;
}

//ROI*****************************************************************

vx_status VX_CALLBACK vxRectprocessValidateInput(vx_node node, vx_uint32 index)
{
    vx_status status = VX_SUCCESS;
    vx_parameter param = NULL;

    if (index < 0 || index > 1)
        return VX_ERROR_INVALID_PARAMETERS;
    if(index == 0)
    {
           param = vxGetParameterByIndex(node, index);
        if(param != NULL)
        {
            vx_array array = NULL;
            vx_enum type = 0;
            if(VX_SUCCESS == vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &array, sizeof(array)))
            {
                vxQueryArray(array, VX_ARRAY_ATTRIBUTE_ITEMTYPE, &type, sizeof(type));
                if (type == VX_TYPE_INT16 || type == VX_TYPE_FLOAT32) status = VX_SUCCESS;
                else status = VX_ERROR_INVALID_TYPE;
                status |= vxReleaseArray(&array);
            }
            status |= vxReleaseParameter(&param);
        }
    }

    if(status < 0)
        printf("error-%s,%d\n",__FILE__,__LINE__);
    return status;
}

vx_status VX_CALLBACK vxRectprocessValidateOutput(vx_node node, vx_uint32 index, vx_meta_format  metaObj)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    vx_parameter param = NULL;

    if (index <2 || index > 3)
        return VX_ERROR_INVALID_PARAMETERS;
    if(index == 2)
    {
        param = vxGetParameterByIndex(node, index);
        if(param != NULL)
        {
            vx_image img = NULL;
            vx_uint32 width = 0, height = 0;
            vx_df_image format = VX_DF_IMAGE_S16;
            if((VX_SUCCESS == vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &img, sizeof(img))))
            {
                status = vxQueryImage(img, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
                status |= vxQueryImage(img, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
                status |= vxSetMetaFormatAttribute(metaObj, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
                status |= vxSetMetaFormatAttribute(metaObj, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
                status |= vxSetMetaFormatAttribute(metaObj, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
                status |= vxReleaseImage(&img);
            }
            status |= vxReleaseParameter(&param);
        }
    }
    else  if(index == 3)
    {
        param = vxGetParameterByIndex(node, index);
        if(param != NULL)
        {
            vx_array array = NULL;
            if((VX_SUCCESS == vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &array, sizeof(vx_array))))
            {
                vx_enum type = 0;
                vx_size capacity = 0;
                vxQueryArray(array, VX_ARRAY_ATTRIBUTE_ITEMTYPE, &type, sizeof(type));
                vxQueryArray(array, VX_ARRAY_ATTRIBUTE_CAPACITY, &capacity, sizeof(capacity));
                status = vxSetMetaFormatAttribute(metaObj, VX_ARRAY_ATTRIBUTE_ITEMTYPE, &type, sizeof(type));
                status |= vxSetMetaFormatAttribute(metaObj, VX_ARRAY_ATTRIBUTE_CAPACITY, &capacity, sizeof(capacity));
                status |= vxSetArrayAttribute(array, VX_ARRAY_ATTRIBUTE_NUMITEMS, &capacity, sizeof(capacity));
                status |= vxReleaseArray(&array);
            }
            status |= vxReleaseParameter(&param);
        }
    }
       if(status < 0)
        printf("error-%s,%d\n",__FILE__,__LINE__);

    return status;
}

vx_status VX_CALLBACK vxRectprocessInitializer(vx_node nodObj, const vx_reference *paramObj, vx_uint32 paraNum)
{

#ifdef USE_RECTPROCESS_VXC
    vx_kernel_execution_parameters_t shaderParam = {
        2,          // workdim
        {0, 0, 0},  // globalWorkOffset: control the start location be processed in the image
        {0, 0, 0},  // globalWorkScale: how many pixels could be processed by a single thread
        {0, 0, 0},  // localWorkSize: local group size in threads
        {0, 0, 0}}; // globalWorkSize: image size in threads

    vx_array  arrRect = (vx_array)paramObj[0];
    vx_scalar depth = (vx_scalar)paramObj[2];
    vx_size item_count2,    item_size2;
    int depth_s = 0;
    int numRect = 0;
    vxReadScalarValue(depth, &depth_s);
    vxQueryArray(arrRect, VX_ARRAY_ATTRIBUTE_NUMITEMS, &item_count2, sizeof(vx_size));
    numRect = item_count2/4;



    shaderParam.globalWorkOffset[0] = 0;
    shaderParam.globalWorkOffset[1] = 0;
    shaderParam.globalWorkScale[0]    = 1;
    shaderParam.globalWorkScale[1]    = 1;
    shaderParam.localWorkSize[0]    = 8;
    shaderParam.localWorkSize[1]    = 1;
    shaderParam.globalWorkSize[0]    = ((depth_s+7)/8)*8;//(((imgWid - offset + shaderParam.globalWorkScale[0]-1 ) / shaderParam.globalWorkScale[0] + shaderParam.localWorkSize[0] - 1) / shaderParam.localWorkSize[0]) * shaderParam.localWorkSize[0];
    shaderParam.globalWorkSize[1]    = numRect;//(imgHei - offset + shaderParam.globalWorkScale[1] - 1) / shaderParam.globalWorkScale[1];

     vx_uint32 UniS16Pack02[16] = {
        0x00030303, 0x00000000, // TCfg
        0x00100000, 0x00000200, 0x00000000, 0x00000000, 0x00000000, // BinSelect
        0x00003400, // AccumType, ConstantType, and PostShift
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
    };
    vx_uint32 UniS16Pack35[16] = {
        0x00030303, 0x00000000, // TCfg
        0x00400003, 0x00000500, 0x00000000, 0x00000000, 0x00000000, // BinSelect
        0x00003400, // AccumType, ConstantType, and PostShift
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
    };
    vx_uint32 UniS16Pack68[16] = {
        0x00030303, 0x00000000, // TCfg
        0x00700006, 0x00000800, 0x00000000, 0x00000000, 0x00000000, // BinSelect
        0x00003400, // AccumType, ConstantType, and PostShift
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
    };
    vx_uint32 UniFp16toFp32[16] = {
        0x01010101, // TCfg
        0x00000000, // ASelt
        0x00010000, 0x00030002, // ABin
        0x02020202, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000100, // AccumType, ConstantType, and PostShift
        0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000 // Constant
    };
    vxSetNodeUniform(nodObj, "UniS16Pack02", 1, UniS16Pack02);
    vxSetNodeUniform(nodObj, "UniS16Pack35", 1, UniS16Pack35);
    vxSetNodeUniform(nodObj, "UniS16Pack68", 1, UniS16Pack68);
    vxSetNodeUniform(nodObj, "UniFp16toFp32", 1, UniFp16toFp32);
    // Please cut the following declarations to VXC source file.
        vx_uint32 UinTestData[16] = {
        0x33333333, // TCfg
        0x11110000, // ASelt
        0x03020100, 0x03020100, // ABin
        0x00000000, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00005400, // AccumType, ConstantType, and PostShift
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
    };
    vxSetNodeUniform(nodObj, "UinTestData", 1, UinTestData);





    vxSetNodeAttribute(nodObj, VX_NODE_ATTRIBUTE_KERNEL_EXECUTION_PARAMETERS, &shaderParam, sizeof(vx_kernel_execution_parameters_t));
#endif
    return VX_SUCCESS;
}

vx_status VX_CALLBACK vxRectprocessDeinitializer(vx_node nodObj, const vx_reference *paraObj, vx_uint32 paraNum)
{
    return VX_SUCCESS;
}
static vx_float32 __Round(vx_float32 x)
{
    return (vx_float32)(floorf((vx_float32)fabs(x) + 0.5f));
}
 vx_float32 Fp16toFp32(const vx_uint16 in)
{
    vx_int32 t1;
    vx_int32 t2;
    vx_int32 t3;
    vx_float32 out;
    t1 = in & 0x7fff;                       // Non-sign bits
    t2 = in & 0x8000;                       // Sign bit
    t3 = in & 0x7c00;                       // Exponent
    t1 <<= 13;                              // Align mantissa on MSB
    t2 <<= 16;                              // Shift sign bit into position
    t1 += 0x38000000;                       // Adjust bias
    t1 = (t3 == 0 ? 0 : t1);                // Denormals-as-zero
    t1 |= t2;                               // Re-insert sign bit
    *((uint32_t*)&out) = t1;
    return out;
}

 void RoiProcess(void *rois,int itemsz,int num,short* dst,int *hist)
 {
      short   mask0[8] = {0,0,0,0,0,0,0,0};
      short   mask1[8] = {1,0,0,0,0,0,0,0};
        short   mask2[8] = {1,1,0,0,0,0,0,0};
        short   mask3[8] = {1,1,1,0,0,0,0,0};
       short   mask4[8] = {1,1,1,1,0,0,0,0};
       short   mask5[8] = {1,1,1,1,1,0,0,0};
       short   mask6[8] = {1,1,1,1,1,1,0,0};
      short   mask7[8] = {1,1,1,1,1,1,1,0};
      short   mask8[8] = {1,1,1,1,1,1,1,1};
    int width = 51,height=39;
    vx_int32 num_rois = 256;
    vx_int32 pooled_width = 6, pooled_height = 6;    //channels =>depth
    vx_float32 spatial_scale_ = 0.0625;
    vx_int32 n,  ph, pw;
    short *rois_fp16 = (short*)rois;
    float *rois_fp32 = (float*)rois;
    int i;
    for( i = 0;i<8;i++)
        hist[i]=0;
    for (n = 0; n < num_rois; ++n)
    {
        vx_int32 offset = 0;
        vx_float32 roi_start_w = 0, roi_start_h = 0, roi_end_w = 0, roi_end_h = 0;
        vx_float32 roi_height = 0, roi_width = 0;
        vx_float32 bin_size_h = 0, bin_size_w = 0;
        int pw_start_arr[6] = {0};
        int ph_start_arr[6] = {0};
        int pw_sz_arr[6] = {0};
        int ph_sz_arr[6] = {0};
        int dd = 0;
        if(itemsz == 2)
        {
            roi_start_w = (vx_float32)(__Round(Fp16toFp32((vx_uint16)rois_fp16[offset])     * spatial_scale_));
            roi_start_h = (vx_float32)(__Round(Fp16toFp32((vx_uint16)rois_fp16[offset + 1]) * spatial_scale_));
            roi_end_w =   (vx_float32)(__Round(Fp16toFp32((vx_uint16)rois_fp16[offset + 2]) * spatial_scale_));
            roi_end_h =   (vx_float32)(__Round(Fp16toFp32((vx_uint16)rois_fp16[offset + 3]) * spatial_scale_));
            rois_fp16 += 4;
        }
        else
        {
            roi_start_w = (vx_float32)(__Round(rois_fp32[offset]     * spatial_scale_));
            roi_start_h = (vx_float32)(__Round(rois_fp32[offset + 1] * spatial_scale_));
            roi_end_w =   (vx_float32)(__Round(rois_fp32[offset + 2] * spatial_scale_));
            roi_end_h =   (vx_float32)(__Round(rois_fp32[offset + 3] * spatial_scale_));
             rois_fp32 += 4;

        }

        roi_height = (vx_float32)gcmMAX(roi_end_h - roi_start_h + 1, 1);
        roi_width = (vx_float32)gcmMAX(roi_end_w - roi_start_w + 1, 1);
        bin_size_h = (vx_float32)(roi_height) / (vx_float32)(pooled_height);
        bin_size_w = (vx_float32)(roi_width) / (vx_float32)(pooled_width);

        for( i = 0;i<6;i++)
        {
           vx_int32 hstart = (vx_int32)(floor((vx_float32)(i) * bin_size_h));
           vx_int32 wstart = (vx_int32)(floor((vx_float32)(i) * bin_size_w));
           vx_int32 hend = (vx_int32)(ceil((vx_float32)(i + 1) * bin_size_h));
           vx_int32 wend = (vx_int32)(ceil((vx_float32)(i + 1) * bin_size_w));

           hstart = gcmMIN(gcmMAX(hstart + (vx_int32)roi_start_h, 0), height);
           hend = gcmMIN(gcmMAX(hend + (vx_int32)roi_start_h, 0), height);
           wstart = gcmMIN(gcmMAX(wstart + (vx_int32)roi_start_w, 0), width);
           wend = gcmMIN(gcmMAX(wend + (vx_int32)roi_start_w, 0), width);

           pw_start_arr[i] = wstart;
           ph_start_arr[i] = hstart;
           pw_sz_arr[i] = wend-wstart;
           ph_sz_arr[i] = hend-hstart;
        }
        dst[0] = (short)pw_sz_arr[0]; //sz.x
        dst[1] = (short)ph_sz_arr[0]; //sz.y
        dst[2] =  (short)0;           //dst.x
        dst[3] =  (short)n;           //dst.y
        dd = 4;
        hist[dst[1]]++;
        for( i=0;i<6;i++)
        {
          int xx = pw_sz_arr[i];
          short *mask;
          if(xx==0)      mask = mask0;
          else if(xx==1) mask = mask1;
          else if(xx==2) mask = mask2;
          else if(xx==3) mask = mask3;
          else if(xx==4) mask = mask4;
          else if(xx==5) mask = mask5;
          else if(xx==6) mask = mask6;
          else if(xx==7) mask = mask7;
          else if(xx<=9) mask = mask8;
          else           mask = mask0;
          dst[dd++] = mask[0];
          dst[dd++] = mask[1];
          dst[dd++] = mask[2];
          dst[dd++] = mask[3];
          dst[dd++] = mask[4];
          dst[dd++] = mask[5];
          dst[dd++] = mask[6];
          dst[dd++] = mask[7];
        }
        for( i=0;i<6;i++)
        {
            int xx = pw_sz_arr[i];
            dst[dd++] = (short)xx;
        }

        for (ph = 0; ph < pooled_height; ++ph)
        {
            for (pw = 0; pw < pooled_width; ++pw)
            {
               //    int szx = pw_sz_arr[pw];
                int szy = ph_sz_arr[ph];
                int x0 =  pw_start_arr[pw];
                int y0 =  ph_start_arr[ph];
                int posx = y0*51+x0;
                int posy = szy;
                dst[dd++] = (short)posx;
                dst[dd++] = (short)posy;
                if(n==0  && ph==5 && pw==3)
                    n  = n;
            }
        }
        dst +=130;
    }

 }

 vx_status VX_CALLBACK vxRectprocessInternalKernel(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
#ifndef USE_RECTPROCESS_VXC
    vx_image  dstImage;
    int  width, height,num_rois;
    vx_array  arrRoi,arrHist;
    vx_scalar num_rois_s;
    void* imgBaseAddr[1] = {NULL};
    vx_rectangle_t imgRect[1] = { { 0, 0, 0, 0 }};
    vx_imagepatch_addressing_t imgInfo[1] = {VX_IMAGEPATCH_ADDR_INIT};
    vx_size item_count,    item_size;
    void *roi = NULL;
    vx_size item_count1,    item_size1;
    void *hist = NULL ;

    if (num != 4)
        return VX_ERROR_INVALID_PARAMETERS;

    arrRoi      = (vx_array) parameters[0];
    num_rois_s  = (vx_scalar)parameters[1];
    dstImage    = (vx_image) parameters[2];
    arrHist     = (vx_array) parameters[3];

    vxReadScalarValue(num_rois_s,   &num_rois);


    vxQueryImage(dstImage, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
    vxQueryImage(dstImage, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));

    status |= vxGetValidRegionImage(dstImage, &imgRect[0]);
    status |= vxAccessImagePatch(dstImage, &imgRect[0], 0, &imgInfo[0], &imgBaseAddr[0], VX_WRITE_ONLY);

    status |= vxQueryArray(arrRoi, VX_ARRAY_ATTRIBUTE_NUMITEMS, &item_count, sizeof(vx_size));
    status |= vxQueryArray(arrRoi, VX_ARRAY_ATTRIBUTE_ITEMSIZE, &item_size, sizeof(vx_size));
    status |= vxAccessArrayRange(arrRoi, 0, item_count, &item_size, &roi, VX_READ_ONLY);


    status |= vxQueryArray(arrHist, VX_ARRAY_ATTRIBUTE_NUMITEMS, &item_count1, sizeof(vx_size));
    status |= vxQueryArray(arrHist, VX_ARRAY_ATTRIBUTE_ITEMSIZE, &item_size1, sizeof(vx_size));
    status |= vxAccessArrayRange(arrHist, 0, item_count1, &item_size1, &hist, VX_WRITE_ONLY);

    RoiProcess((void*)roi,(int)item_size,num_rois, (short*)imgBaseAddr[0],(int*)hist);

    status |= vxCommitImagePatch(dstImage, &imgRect[0], 0, &imgInfo[0], imgBaseAddr[0]);

    status |= vxCommitArrayRange(arrRoi, 0, item_count, roi);
    status |= vxCommitArrayRange(arrHist, 0, item_count1, hist);

#endif
   return status;
 }


vx_status VX_CALLBACK vxResortingValidateInput(vx_node node, vx_uint32 index)
{
    vx_status status = VX_SUCCESS;
    vx_parameter param = NULL;

    if (index < 0 || index > 2)
        return VX_ERROR_INVALID_PARAMETERS;
    if(index == 0)
    {
        param = vxGetParameterByIndex(node, index);
        if(param != NULL)
        {
            vx_image img = NULL;
            vx_df_image format = 0;
            if((VX_SUCCESS == vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &img, sizeof(img))))
            {
                vxQueryImage(img, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
             if (VX_DF_IMAGE_S16 == format) status = VX_SUCCESS;
                else status = VX_ERROR_INVALID_VALUE;
                status |= vxReleaseImage(&img);
            }
            status |= vxReleaseParameter(&param);
        }
    }
    else if(index == 1)
    {
           param = vxGetParameterByIndex(node, index);
        if(param != NULL)
        {
            vx_array array = NULL;
            vx_enum type = 0;
            if(VX_SUCCESS == vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &array, sizeof(array)))
            {
                vxQueryArray(array, VX_ARRAY_ATTRIBUTE_ITEMTYPE, &type, sizeof(type));
                if (type == VX_TYPE_INT32) status = VX_SUCCESS;
                else status = VX_ERROR_INVALID_TYPE;
                status |= vxReleaseArray(&array);
            }
            status |= vxReleaseParameter(&param);
        }
    }


    if(status < 0)
        printf("error-%s,%d\n",__FILE__,__LINE__);
    return status;
}

vx_status VX_CALLBACK vxResortingValidateOutput(vx_node node, vx_uint32 index, vx_meta_format  metaObj)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    vx_parameter param = NULL;

    if (index != 3)
        return VX_ERROR_INVALID_PARAMETERS;

    if(index == 3)
    {
         param = vxGetParameterByIndex(node, index);
        if(param != NULL)
        {
            vx_image img = NULL;
            vx_uint32 width = 0, height = 0;
            vx_df_image format = VX_DF_IMAGE_S16;
            if((VX_SUCCESS == vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &img, sizeof(img))))
            {
                status = vxQueryImage(img, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
                status |= vxQueryImage(img, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
                status |= vxSetMetaFormatAttribute(metaObj, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
                status |= vxSetMetaFormatAttribute(metaObj, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
                status |= vxSetMetaFormatAttribute(metaObj, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
                status |= vxReleaseImage(&img);
            }
            status |= vxReleaseParameter(&param);
        }
    }
       if(status < 0)
        printf("error-%s,%d\n",__FILE__,__LINE__);

    return status;
}

vx_status VX_CALLBACK vxResortingInitializer(vx_node nodObj, const vx_reference *paramObj, vx_uint32 paraNum)
{

#ifdef USE_RESORTING_VXC
    vx_kernel_execution_parameters_t shaderParam = {
        2,          // workdim
        {0, 0, 0},  // globalWorkOffset: control the start location be processed in the image
        {0, 0, 0},  // globalWorkScale: how many pixels could be processed by a single thread
        {0, 0, 0},  // localWorkSize: local group size in threads
        {0, 0, 0}}; // globalWorkSize: image size in threads

    vx_array  arrRect = (vx_array)paramObj[0];
    vx_scalar depth = (vx_scalar)paramObj[2];
    vx_size item_count2,    item_size2;
    int depth_s = 0;
    int numRect = 0;
    vxReadScalarValue(depth, &depth_s);
    vxQueryArray(arrRect, VX_ARRAY_ATTRIBUTE_NUMITEMS, &item_count2, sizeof(vx_size));
    numRect = item_count2/4;



    shaderParam.globalWorkOffset[0] = 0;
    shaderParam.globalWorkOffset[1] = 0;
    shaderParam.globalWorkScale[0]    = 1;
    shaderParam.globalWorkScale[1]    = 1;
    shaderParam.localWorkSize[0]    = 8;
    shaderParam.localWorkSize[1]    = 1;
    shaderParam.globalWorkSize[0]    = ((depth_s+7)/8)*8;//(((imgWid - offset + shaderParam.globalWorkScale[0]-1 ) / shaderParam.globalWorkScale[0] + shaderParam.localWorkSize[0] - 1) / shaderParam.localWorkSize[0]) * shaderParam.localWorkSize[0];
    shaderParam.globalWorkSize[1]    = numRect;//(imgHei - offset + shaderParam.globalWorkScale[1] - 1) / shaderParam.globalWorkScale[1];

     vx_uint32 UniS16Pack02[16] = {
        0x00030303, 0x00000000, // TCfg
        0x00100000, 0x00000200, 0x00000000, 0x00000000, 0x00000000, // BinSelect
        0x00003400, // AccumType, ConstantType, and PostShift
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
    };
    vx_uint32 UniS16Pack35[16] = {
        0x00030303, 0x00000000, // TCfg
        0x00400003, 0x00000500, 0x00000000, 0x00000000, 0x00000000, // BinSelect
        0x00003400, // AccumType, ConstantType, and PostShift
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
    };
    vx_uint32 UniS16Pack68[16] = {
        0x00030303, 0x00000000, // TCfg
        0x00700006, 0x00000800, 0x00000000, 0x00000000, 0x00000000, // BinSelect
        0x00003400, // AccumType, ConstantType, and PostShift
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
    };
    vx_uint32 UniFp16toFp32[16] = {
        0x01010101, // TCfg
        0x00000000, // ASelt
        0x00010000, 0x00030002, // ABin
        0x02020202, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000100, // AccumType, ConstantType, and PostShift
        0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000 // Constant
    };
    vxSetNodeUniform(nodObj, "UniS16Pack02", 1, UniS16Pack02);
    vxSetNodeUniform(nodObj, "UniS16Pack35", 1, UniS16Pack35);
    vxSetNodeUniform(nodObj, "UniS16Pack68", 1, UniS16Pack68);
    vxSetNodeUniform(nodObj, "UniFp16toFp32", 1, UniFp16toFp32);
    // Please cut the following declarations to VXC source file.
        vx_uint32 UinTestData[16] = {
        0x33333333, // TCfg
        0x11110000, // ASelt
        0x03020100, 0x03020100, // ABin
        0x00000000, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00005400, // AccumType, ConstantType, and PostShift
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
    };
    vxSetNodeUniform(nodObj, "UinTestData", 1, UinTestData);





    vxSetNodeAttribute(nodObj, VX_NODE_ATTRIBUTE_KERNEL_EXECUTION_PARAMETERS, &shaderParam, sizeof(vx_kernel_execution_parameters_t));
#endif
    return VX_SUCCESS;
}

vx_status VX_CALLBACK vxResortingDeinitializer(vx_node nodObj, const vx_reference *paraObj, vx_uint32 paraNum)
{
    return VX_SUCCESS;
}

static void ReSorting(short *rois,int *hist,int num,short* dst)
 {
     int index0 = 0;
     int index1 = hist[0]+index0;
     int index2 = hist[1]+index1;
     int index3 = hist[2]+index2;
     int index4 = hist[3]+index3;
     int index5 = hist[4]+index4;
     int index6 = hist[5]+index5;
     int index7 = hist[6]+index6;
     int n=0;
      for( n=0;n<num;n++)
     {
       short *rt = &rois[n*130];
       int szy = rt[1];
       if(szy == 0)    {
           short *rtdst = &dst[index0*130];
           memcpy(rtdst,rt,130*sizeof(short));
           index0++;
       }
       else if(szy == 1) {
           short *rtdst = &dst[index1*130];
           memcpy(rtdst,rt,130*sizeof(short));
           index1++;
       }
       else if(szy == 2) {
           short *rtdst = &dst[index2*130];
           memcpy(rtdst,rt,130*sizeof(short));
           index2++;
       }
       else if(szy == 3) {
           short *rtdst = &dst[index3*130];
           memcpy(rtdst,rt,130*sizeof(short));
           index3++;
       }
       else if(szy == 4) {
           short *rtdst = &dst[index4*130];
           memcpy(rtdst,rt,130*sizeof(short));
           index4++;
       }
        else if(szy == 5) {
           short *rtdst = &dst[index5*130];
           memcpy(rtdst,rt,130*sizeof(short));
           index5++;
       }
       else if(szy == 6) {
           short *rtdst = &dst[index6*130];
           memcpy(rtdst,rt,130*sizeof(short));
           index6++;
       }
       else if(szy == 7) {
           short *rtdst = &dst[index7*130];
           memcpy(rtdst,rt,130*sizeof(short));
           index7++;
       }
      }
  }
 vx_status VX_CALLBACK vxResortingInternalKernel(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
#ifndef USE_RESORTING_VXC
    vx_image srcImage, dstImage;
    vx_scalar num_rois_s;
    int  num_rois,width,height;
    vx_array  arrHist;
    void* imgBaseAddr[2] = {NULL, NULL};
    vx_rectangle_t imgRect[2] = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }};
    vx_imagepatch_addressing_t imgInfo[2] = {VX_IMAGEPATCH_ADDR_INIT, VX_IMAGEPATCH_ADDR_INIT};

    vx_size item_count,    item_size;
    void *hist = NULL;

    if (num != 4)
        return VX_ERROR_INVALID_PARAMETERS;

    srcImage    = (vx_image) parameters[0];
    arrHist     = (vx_array)parameters[1];
    num_rois_s  = (vx_scalar)parameters[2];
    dstImage    = (vx_image)parameters[3];
    vxReadScalarValue(num_rois_s,   &num_rois);


    vxQueryImage(srcImage, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
    vxQueryImage(srcImage, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));

    status |= vxGetValidRegionImage(srcImage, &imgRect[0]);
    status |= vxAccessImagePatch(srcImage, &imgRect[0], 0, &imgInfo[0], &imgBaseAddr[0], VX_READ_ONLY);
    status |= vxAccessImagePatch(dstImage, &imgRect[0], 0, &imgInfo[0], &imgBaseAddr[1], VX_READ_ONLY);


    status |= vxQueryArray(arrHist, VX_ARRAY_ATTRIBUTE_NUMITEMS, &item_count, sizeof(vx_size));
    status |= vxQueryArray(arrHist, VX_ARRAY_ATTRIBUTE_ITEMSIZE, &item_size, sizeof(vx_size));
    status |= vxAccessArrayRange(arrHist, 0, item_count, &item_size, &hist, VX_READ_ONLY);

    ReSorting((short*)imgBaseAddr[0],(int*)hist,height,(short*)imgBaseAddr[1]);
     status |= vxCommitImagePatch(srcImage, &imgRect[0], 0, &imgInfo[0], imgBaseAddr[0]);
    status |= vxCommitImagePatch(dstImage, &imgRect[1], 0, &imgInfo[1], imgBaseAddr[1]);
     status |= vxCommitArrayRange(arrHist, 0, item_count, hist);

#endif
   return status;
 }

vx_status VX_CALLBACK vxHoripoolValidateInput(vx_node node, vx_uint32 index)
{
    vx_status status = VX_SUCCESS;
    vx_parameter param = NULL;

    if (index < 0 || index > 5)
        return VX_ERROR_INVALID_PARAMETERS;
    if(index == 0)
    {
          param = vxGetParameterByIndex(node, index);
        if(param != NULL)
        {
            vx_image img = NULL;
            vx_df_image format = 0;
            if((VX_SUCCESS == vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &img, sizeof(img))))
            {
                vxQueryImage(img, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
             if (VX_DF_IMAGE_S16 == format) status = VX_SUCCESS;
                else status = VX_ERROR_INVALID_VALUE;
                status |= vxReleaseImage(&img);
            }
            status |= vxReleaseParameter(&param);
        }
    }
    else if(index == 1)
    {
        param = vxGetParameterByIndex(node, index);
        if(param != NULL)
        {
            vx_image img = NULL;
            vx_df_image format = 0;
            if((VX_SUCCESS == vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &img, sizeof(img))))
            {
                vxQueryImage(img, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
             if (VX_DF_IMAGE_S16 == format) status = VX_SUCCESS;
                else status = VX_ERROR_INVALID_VALUE;
                status |= vxReleaseImage(&img);
            }
            status |= vxReleaseParameter(&param);
        }
    }


    if(status < 0)
        printf("error-%s,%d\n",__FILE__,__LINE__);
    return status;
}

vx_status VX_CALLBACK vxHoripoolValidateOutput(vx_node node, vx_uint32 index, vx_meta_format  metaObj)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    vx_parameter param = NULL;

    if (index != 5)
        return VX_ERROR_INVALID_PARAMETERS;

    if(index == 5)
    {
        param = vxGetParameterByIndex(node, index);
        if(param != NULL)
        {
            vx_array array = NULL;
            if((VX_SUCCESS == vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &array, sizeof(vx_array))))
            {
                vx_enum type = 0;
                vx_size capacity = 0;
                vxQueryArray(array, VX_ARRAY_ATTRIBUTE_ITEMTYPE, &type, sizeof(type));
                vxQueryArray(array, VX_ARRAY_ATTRIBUTE_CAPACITY, &capacity, sizeof(capacity));
                status = vxSetMetaFormatAttribute(metaObj, VX_ARRAY_ATTRIBUTE_ITEMTYPE, &type, sizeof(type));
                status |= vxSetMetaFormatAttribute(metaObj, VX_ARRAY_ATTRIBUTE_CAPACITY, &capacity, sizeof(capacity));
                status |= vxSetArrayAttribute(array, VX_ARRAY_ATTRIBUTE_NUMITEMS, &capacity, sizeof(capacity));
                status |= vxReleaseArray(&array);
            }
            status |= vxReleaseParameter(&param);
        }
    }
       if(status < 0)
        printf("error-%s,%d\n",__FILE__,__LINE__);

    return status;
}

vx_status VX_CALLBACK vxHoripoolInitializer(vx_node nodObj, const vx_reference *paramObj, vx_uint32 paraNum)
{

#ifdef USE_HORIPOOL_VXC
    vx_kernel_execution_parameters_t shaderParam = {
        2,          // workdim
        {0, 0, 0},  // globalWorkOffset: control the start location be processed in the image
        {0, 0, 0},  // globalWorkScale: how many pixels could be processed by a single thread
        {0, 0, 0},  // localWorkSize: local group size in threads
        {0, 0, 0}}; // globalWorkSize: image size in threads

    vx_image  imgRect = (vx_image)paramObj[0];
    vx_scalar depth = (vx_scalar)paramObj[2];
    int depth_s = 0;
    int numRect = 0;
    vxReadScalarValue(depth, &depth_s);
    vxQueryImage(imgRect, VX_IMAGE_ATTRIBUTE_HEIGHT, &numRect, sizeof(numRect));


    shaderParam.globalWorkOffset[0] = 0;
    shaderParam.globalWorkOffset[1] = 0;
    shaderParam.globalWorkScale[0]    = 1;
    shaderParam.globalWorkScale[1]    = 1;
    shaderParam.localWorkSize[0]    = 8;
    shaderParam.localWorkSize[1]    = 1;
    shaderParam.globalWorkSize[0]    = ((numRect+7)/8)*8;//(((imgWid - offset + shaderParam.globalWorkScale[0]-1 ) / shaderParam.globalWorkScale[0] + shaderParam.localWorkSize[0] - 1) / shaderParam.localWorkSize[0]) * shaderParam.localWorkSize[0];
    shaderParam.globalWorkSize[1]    = depth_s;//(imgHei - offset + shaderParam.globalWorkScale[1] - 1) / shaderParam.globalWorkScale[1];
    {
    vx_uint32 UniFp16toFp32[16] = {
        0x01010101, // TCfg
        0x00000000, // ASelt
        0x00010000, 0x00030002, // ABin
        0x02020202, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000100, // AccumType, ConstantType, and PostShift
        0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000 // Constant
    };
     vx_uint32 UniPack035[16] = {
        0x00030303, 0x00000000, // TCfg
        0x00300000, 0x00000500, 0x00000000, 0x00000000, 0x00000000, // BinSelect
        0x00003400, // AccumType, ConstantType, and PostShift
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
    };
    vx_uint32 Uni2x8MaskInt16[16] = {
        0x11111111, // TCfg
        0x00000000, // ASelt
        0x03020100, 0x07060504, // ABin
        0x11111111, // BSelt
        0x03020100, 0x07060504, // BBin
        0x00000400, // AccumType, ConstantType, and PostShift
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
    };
    vx_uint32 UniShort2Int[16] = {
        0x00000501, // TCfg
        0x00000400, // ASelt
        0x00010000, 0x00000000, // ABin
        0x00000a02, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000400, // AccumType, ConstantType, and PostShift
        0x00000001, 0x00000000, 0x00010001, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
    };
    vx_uint32 UniShort2intLow[16] = {
        0x05010501, // TCfg
        0x04000400, // ASelt
        0x00010000, 0x00030002, // ABin
        0x0a020a02, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00003400, // AccumType, ConstantType, and PostShift
        0x00000001, 0x00000000, 0x00010001, 0x00000000, 0x00000001, 0x00000000, 0x00010001, 0x00000000 // Constant
    };
    vx_uint32 UniShort2IntHigh[16] = {
        0x05010501, // TCfg
        0x04000400, // ASelt
        0x00050004, 0x00070006, // ABin
        0x0a020a02, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000400, // AccumType, ConstantType, and PostShift
        0x00000001, 0x00000000, 0x00010001, 0x00000000, 0x00000001, 0x00000000, 0x00010001, 0x00000000 // Constant
    };

    vxSetNodeUniform(nodObj, "UniFp16toFp32", 1, UniFp16toFp32);
    vxSetNodeUniform(nodObj, "UniPack035", 1, UniPack035);
    vxSetNodeUniform(nodObj, "Uni2x8MaskInt16", 1, Uni2x8MaskInt16);
    vxSetNodeUniform(nodObj, "UniShort2Int", 1, UniShort2Int);
    vxSetNodeUniform(nodObj, "UniShort2intLow", 1, UniShort2intLow);
    vxSetNodeUniform(nodObj, "UniShort2IntHigh", 1, UniShort2IntHigh);
    vxSetNodeAttribute(nodObj, VX_NODE_ATTRIBUTE_KERNEL_EXECUTION_PARAMETERS, &shaderParam, sizeof(vx_kernel_execution_parameters_t));
    }
#endif
    return VX_SUCCESS;
}

vx_status VX_CALLBACK vxHoripoolDeinitializer(vx_node nodObj, const vx_reference *paraObj, vx_uint32 paraNum)
{
    return VX_SUCCESS;
}
VX_PRIVATE_API vx_status VX_CALLBACK vxHoripoolFunction(vx_node node, const vx_reference parameters[], vx_uint32 paramCount)
{
    vx_status   status = VX_SUCCESS;
    vx_reference *ucParameters = (vx_reference*)parameters;

    /*step1: pre-process */
    vx_image_s   imgObj2;
    vx_array  input_array2 = (vx_array)parameters[0];
    vx_scalar depth  = (vx_scalar)parameters[2];
    vx_scalar width  = (vx_scalar)parameters[3];
    vx_scalar height = (vx_scalar)parameters[4];
    vx_array  output_array = (vx_array)parameters[5];

    int depth_s;
    int width_s, height_s;

    vxReadScalarValue(width, &width_s);
    vxReadScalarValue(depth, &depth_s);
    vxReadScalarValue(height, &height_s);
    {
//        int itemSize = output_array->itemSize;
        int itemCount = (int)input_array2->itemCount/4;
        gcoOS_ZeroMemory(&imgObj2, sizeof(vx_image_s));

        imgObj2.base = output_array->base;
        imgObj2.base.type = VX_TYPE_IMAGE;
        imgObj2.width = depth_s*36;
        imgObj2.height =itemCount;
        imgObj2.format = VX_DF_IMAGE_S16;
        imgObj2.planeCount = 1;
        imgObj2.memory = output_array->memory;
    }
    ucParameters[5] = (vx_reference)(&imgObj2);
    /*step2: call the base function */
    status = vxProgramKernel_Function(node, parameters, paramCount);
    /* step3 : post-process */
    ucParameters[5] = (vx_reference)(output_array);
    return status;

}
//Vertpool*****************************************************************


vx_status VX_CALLBACK vxVertpoolValidateInput(vx_node node, vx_uint32 index)
{
    vx_status status = VX_SUCCESS;
    vx_parameter param = NULL;

    if (index < 0 || index > 4)
        return VX_ERROR_INVALID_PARAMETERS;

    if(index == 0)
    {
           param = vxGetParameterByIndex(node, index);
        if(param != NULL)
        {
            vx_array array = NULL;
            vx_enum type = 0;
            if(VX_SUCCESS == vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &array, sizeof(array)))
            {
                vxQueryArray(array, VX_ARRAY_ATTRIBUTE_ITEMTYPE, &type, sizeof(type));
                if (type == VX_TYPE_INT16) status = VX_SUCCESS;
                else status = VX_ERROR_INVALID_TYPE;
                status |= vxReleaseArray(&array);
            }
            status |= vxReleaseParameter(&param);
        }
    }


    if(status < 0)
        printf("error-%s,%d\n",__FILE__,__LINE__);
    return status;
}

vx_status VX_CALLBACK vxVertpoolValidateOutput(vx_node node, vx_uint32 index, vx_meta_format  metaObj)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    vx_parameter param = NULL;

    if (index != 4)
        return VX_ERROR_INVALID_PARAMETERS;

    if(index == 4)
    {
        param = vxGetParameterByIndex(node, index);
        if(param != NULL)
        {
            vx_image img = NULL;
            vx_uint32 width = 0, height = 0;
            vx_df_image format = VX_DF_IMAGE_S16;
            if((VX_SUCCESS == vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &img, sizeof(img))))
            {
                status = vxQueryImage(img, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
                status |= vxQueryImage(img, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
                status |= vxSetMetaFormatAttribute(metaObj, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
                status |= vxSetMetaFormatAttribute(metaObj, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
                status |= vxSetMetaFormatAttribute(metaObj, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
                status |= vxReleaseImage(&img);
            }
            status |= vxReleaseParameter(&param);
        }
    }
       if(status < 0)
        printf("error-%s,%d\n",__FILE__,__LINE__);

    return status;
}

vx_status VX_CALLBACK vxVertpoolInitializer(vx_node nodObj, const vx_reference *paramObj, vx_uint32 paraNum)
{

#ifdef USE_VERTPOOL_VXC
    vx_kernel_execution_parameters_t shaderParam = {
        2,          // workdim
        {0, 0, 0},  // globalWorkOffset: control the start location be processed in the image
        {0, 0, 0},  // globalWorkScale: how many pixels could be processed by a single thread
        {0, 0, 0},  // localWorkSize: local group size in threads
        {0, 0, 0}}; // globalWorkSize: image size in threads

    vx_scalar depth_s = (vx_scalar)paramObj[1];
    vx_scalar width_s = (vx_scalar)paramObj[2];
    vx_scalar height_s = (vx_scalar)paramObj[3];
    vx_image  imageVpool = (vx_image)paramObj[4];
    vx_status status = VX_SUCCESS;
//     void *pool = NULL;
     int c = 0;
    int depth,width,height;
    vxReadScalarValue(depth_s, &depth);
    vxReadScalarValue(width_s, &width);
    vxReadScalarValue(height_s, &height);

    {
    vx_rectangle_t rect = {0, 0, 0, 0};
    vx_imagepatch_addressing_t imgInfo = VX_IMAGEPATCH_ADDR_INIT;
    void* imgBaseAddr = 0;
    int width_img,height_img;
    status = vxQueryImage(imageVpool, VX_IMAGE_ATTRIBUTE_WIDTH, &width_img, sizeof(width_img));
    status |= vxQueryImage(imageVpool, VX_IMAGE_ATTRIBUTE_HEIGHT, &height_img, sizeof(height_img));
    rect.end_x = width_img;
    rect.end_y = height_img;
       vxAccessImagePatch(imageVpool, &rect, 0, &imgInfo, &imgBaseAddr, VX_WRITE_ONLY);
    for( c = 0;c < depth; c++)
    {
        vx_int16* ptr = (vx_int16*)imgBaseAddr + c*8*(width*height) ;
        memset(ptr,0,8*width*height*sizeof(vx_int16));
    }
    vxCommitImagePatch(imageVpool, &rect, 0, &imgInfo, imgBaseAddr);
    }

    shaderParam.globalWorkOffset[0] = 0;
    shaderParam.globalWorkOffset[1] = 0;
    shaderParam.globalWorkScale[0]    = 8;
    shaderParam.globalWorkScale[1]    = 1;
    shaderParam.localWorkSize[0]    = 7;
    shaderParam.localWorkSize[1]    = 1;
    shaderParam.globalWorkSize[0]    = 7;//(((imgWid - offset + shaderParam.globalWorkScale[0]-1 ) / shaderParam.globalWorkScale[0] + shaderParam.localWorkSize[0] - 1) / shaderParam.localWorkSize[0]) * shaderParam.localWorkSize[0];
    shaderParam.globalWorkSize[1]    = depth;//(imgHei - offset + shaderParam.globalWorkScale[1] - 1) / shaderParam.globalWorkScale[1];

    vxSetNodeAttribute(nodObj, VX_NODE_ATTRIBUTE_KERNEL_EXECUTION_PARAMETERS, &shaderParam, sizeof(vx_kernel_execution_parameters_t));
#endif
    return VX_SUCCESS;
}

vx_status VX_CALLBACK vxVertpoolDeinitializer(vx_node nodObj, const vx_reference *paraObj, vx_uint32 paraNum)
{
    return VX_SUCCESS;
}
VX_PRIVATE_API vx_status VX_CALLBACK vxVertpoolFunction(vx_node node, const vx_reference parameters[], vx_uint32 paramCount)
{
    vx_status   status = VX_SUCCESS;
    vx_reference *ucParameters = (vx_reference*)parameters;

    /*step1: pre-process */
    vx_image_s  imgObj;

    vx_array input_array = (vx_array)parameters[0];
    vx_scalar depth = (vx_scalar)parameters[1];
    vx_scalar width = (vx_scalar)parameters[2];
    vx_scalar height = (vx_scalar)parameters[3];
   // vx_array output_array = (vx_array)parameters[4];

    int depth_s;
    int width_s, height_s;

    vxReadScalarValue(width, &width_s);
    vxReadScalarValue(depth, &depth_s);
    vxReadScalarValue(height, &height_s);

    {
//        int itemSize = input_array->itemSize;
        gcoOS_ZeroMemory(&imgObj, sizeof(vx_image_s));

        imgObj.base = input_array->base;
        imgObj.base.type = VX_TYPE_IMAGE;

        imgObj.width = width_s;
        imgObj.height = height_s * depth_s;
        imgObj.format = VX_DF_IMAGE_S16;
        imgObj.planeCount = 1;
        imgObj.memory = input_array->memory;
    }

    ucParameters[0] = (vx_reference)(&imgObj);
//    ucParameters[4] = (vx_reference)(&imgObj2);
    /*step2: call the base function */
    status = vxProgramKernel_Function(node, parameters, paramCount);
    /* step3 : post-process */
    ucParameters[0] = (vx_reference)(input_array);
 //   ucParameters[4] = (vx_reference)(output_array);
    return status;

}
//Roipool******************************************
vx_status VX_CALLBACK vxRoipoolInitializer(vx_node nodObj, const vx_reference *paramObj, vx_uint32 paraNum)
{
    vx_status        status = VX_SUCCESS;
    vx_context       context;
    vx_graph         graph;
    vx_image         vpool, image_rect,image_sort ;
    vx_array array_hist;
    vx_node          nodeVert=NULL,nodeHori=NULL,nodeProc=NULL,nodeSort=NULL;
    vx_kernel kernelVert = NULL,kernelHori = NULL,kernelProc = NULL,kernelSort = NULL;
    vx_array input1;
    vx_array input2;
    vx_array  dst;
    int index = 0;
    vx_scalar width_s, height_s, depth_s,num_rois_s;
    int width = 51;
    int height = 39;
    int depth = 256;
    int num_rois = 256;
//    int  sz_vpool = height * width * depth * 8;
     vx_border_mode_t border;
    border.mode = VX_BORDER_MODE_CONSTANT;
    border.constant_value = 0;

    if (paraNum != 6)
        return VX_ERROR_INVALID_PARAMETERS;

    input1   = (vx_array )paramObj[0];
    input2   = (vx_array )paramObj[1];
    dst      = (vx_array )paramObj[5];

    context      = vxGetContext((vx_reference)nodObj);
    graph        = vxCreateGraph(context);
    width_s  = vxCreateScalar(context, VX_TYPE_INT32, &width);
    height_s = vxCreateScalar(context, VX_TYPE_INT32, &height);
    depth_s  = vxCreateScalar(context, VX_TYPE_INT32, &depth);
    num_rois_s = vxCreateScalar(context, VX_TYPE_INT32, &num_rois);



    if (graph == VX_NULL)
        return VX_ERROR_INVALID_GRAPH;
    image_rect  = vxCreateImage(context, 130,256, VX_DF_IMAGE_S16);
    image_sort  = vxCreateImage(context, 130,256, VX_DF_IMAGE_S16);
    array_hist = vxCreateArray(context, VX_TYPE_INT32, 8);

   kernelProc = vxGetKernelByEnum(context,VX_KERNEL_ENUM_RECTPROCESS);
    if(kernelProc)
    {
       nodeProc = vxCreateGenericNode(graph, kernelProc) ;
       if(nodeProc)
       {
                 index = 0;
                status |= vxSetParameterByIndex(nodeProc, index++, (vx_reference)input2);
                status |= vxSetParameterByIndex(nodeProc, index++, (vx_reference)num_rois_s);
                status |= vxSetParameterByIndex(nodeProc, index++, (vx_reference)image_rect);
                status |= vxSetParameterByIndex(nodeProc, index++, (vx_reference)array_hist);

                status |= vxSetNodeAttribute(nodeProc, VX_NODE_ATTRIBUTE_BORDER_MODE, &border, sizeof(border));
                if(status != VX_SUCCESS)
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                    vxReleaseNode(&nodeProc);
                    vxReleaseKernel(&kernelProc);
                }
         }
         else
         {
               vxReleaseKernel(&kernelProc);
         }
     }
    kernelSort = vxGetKernelByEnum(context,VX_KERNEL_ENUM_RESORTING);
    if(kernelSort)
    {
       nodeSort = vxCreateGenericNode(graph, kernelSort) ;
       if(nodeSort)
       {
                index = 0;
                status |= vxSetParameterByIndex(nodeSort, index++, (vx_reference)image_rect);
                status |= vxSetParameterByIndex(nodeSort, index++, (vx_reference)array_hist);
                status |= vxSetParameterByIndex(nodeSort, index++, (vx_reference)num_rois_s);
                status |= vxSetParameterByIndex(nodeSort, index++, (vx_reference)image_sort);

                status |= vxSetNodeAttribute(nodeSort, VX_NODE_ATTRIBUTE_BORDER_MODE, &border, sizeof(border));
                if(status != VX_SUCCESS)
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                    vxReleaseNode(&nodeSort);
                    vxReleaseKernel(&kernelSort);
                }
         }
         else
         {
               vxReleaseKernel(&kernelSort);
         }
     }

    vpool = vxCreateImage(context, width*height, depth*8, VX_DF_IMAGE_S16);
    kernelVert = vxGetKernelByEnum(context,VX_KERNEL_ENUM_VERTPOOL);
    if(kernelVert)
    {
       nodeVert = vxCreateGenericNode(graph, kernelVert) ;
       if(nodeVert)
       {
                index = 0;
                status |= vxSetParameterByIndex(nodeVert, index++, (vx_reference)input1);
                status |= vxSetParameterByIndex(nodeVert, index++, (vx_reference)depth_s);
                status |= vxSetParameterByIndex(nodeVert, index++, (vx_reference)width_s);
                status |= vxSetParameterByIndex(nodeVert, index++, (vx_reference)height_s);
                status |= vxSetParameterByIndex(nodeVert, index++, (vx_reference)vpool);
                status |= vxSetNodeAttribute(nodeVert, VX_NODE_ATTRIBUTE_BORDER_MODE, &border, sizeof(border));
                if(status != VX_SUCCESS)
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                    vxReleaseNode(&nodeVert);
                    vxReleaseKernel(&kernelVert);
                }
         }
         else
         {
               vxReleaseKernel(&kernelVert);
         }
     }

    kernelHori = vxGetKernelByEnum(context,VX_KERNEL_ENUM_HORIPOOL);
    if(kernelHori)
    {
       nodeHori = vxCreateGenericNode(graph, kernelHori) ;
       if(nodeHori)
       {
                index = 0;
                status |= vxSetParameterByIndex(nodeHori, index++, (vx_reference)image_sort);
                status |= vxSetParameterByIndex(nodeHori, index++, (vx_reference)vpool);
                status |= vxSetParameterByIndex(nodeHori, index++, (vx_reference)depth_s);
                status |= vxSetParameterByIndex(nodeHori, index++, (vx_reference)width_s);
                status |= vxSetParameterByIndex(nodeHori, index++, (vx_reference)height_s);
                status |= vxSetParameterByIndex(nodeHori, index++, (vx_reference)dst);

                status |= vxSetNodeAttribute(nodeHori, VX_NODE_ATTRIBUTE_BORDER_MODE, &border, sizeof(border));
                if(status != VX_SUCCESS)
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                    vxReleaseNode(&nodeHori);
                    vxReleaseKernel(&kernelHori);
                }
         }
         else
         {
               vxReleaseKernel(&kernelHori);
         }
     }
    status  = vxoAddParameterToGraphByIndex(graph, nodeVert, 0);
    status |= vxoAddParameterToGraphByIndex(graph, nodeProc, 0);
    status |= vxoAddParameterToGraphByIndex(graph, nodeHori, 2);
    status |= vxoAddParameterToGraphByIndex(graph, nodeHori, 3);
    status |= vxoAddParameterToGraphByIndex(graph, nodeHori, 4);
    status |= vxoAddParameterToGraphByIndex(graph, nodeHori, 5);

    status |= vxoNode_SetChildGraph(nodObj, graph);
    status |= vxVerifyGraph(graph);

    vxReleaseNode(&nodeVert);
    vxReleaseNode(&nodeHori);
    vxReleaseKernel(&kernelVert);
    vxReleaseKernel(&kernelHori);

    vxReleaseNode(&nodeProc);
    vxReleaseNode(&nodeSort);
    vxReleaseKernel(&kernelProc);
    vxReleaseKernel(&kernelSort);

    vxReleaseImage(&vpool);
    vxReleaseImage(&image_rect);
    vxReleaseImage(&image_sort);
    vxReleaseArray(&array_hist);
    vxReleaseGraph(&graph);

    return status;


}
vx_status VX_CALLBACK vxRoipoolDeinitializer(vx_node nodObj, const vx_reference *paraObj, vx_uint32 paraNum)
{
     if (paraNum != 6) return VX_ERROR_INVALID_PARAMETERS;

    return vxoNode_SetChildGraph(nodObj, 0);
}
vx_status VX_CALLBACK vxRoipoolValidateInput(vx_node node, vx_uint32 index)
{
    if (index != 0 && index != 1 && index != 2 && index != 3 && index != 4)
        return VX_ERROR_INVALID_PARAMETERS;

    return VX_SUCCESS;
}
vx_status VX_CALLBACK vxRoipoolValidateOutput(vx_node node, vx_uint32 index, vx_meta_format  metaObj)
{
   vx_status status = VX_ERROR_INVALID_PARAMETERS;
    vx_parameter param = NULL;

    if (index != 5)
        return VX_ERROR_INVALID_PARAMETERS;
    if(index == 5)
    {
        param = vxGetParameterByIndex(node, index);
        if(param != NULL)
        {
            vx_array array = NULL;
            if((VX_SUCCESS == vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &array, sizeof(vx_array))))
            {
                vx_enum type = 0;
                vx_size capacity = 0;
                vxQueryArray(array, VX_ARRAY_ATTRIBUTE_ITEMTYPE, &type, sizeof(type));
                vxQueryArray(array, VX_ARRAY_ATTRIBUTE_CAPACITY, &capacity, sizeof(capacity));
                status = vxSetMetaFormatAttribute(metaObj, VX_ARRAY_ATTRIBUTE_ITEMTYPE, &type, sizeof(type));
                status |= vxSetMetaFormatAttribute(metaObj, VX_ARRAY_ATTRIBUTE_CAPACITY, &capacity, sizeof(capacity));
                status |= vxSetArrayAttribute(array, VX_ARRAY_ATTRIBUTE_NUMITEMS, &capacity, sizeof(capacity));
                status |= vxReleaseArray(&array);
            }
            status |= vxReleaseParameter(&param);
        }
    }
       if(status < 0)
        printf("error-%s,%d\n",__FILE__,__LINE__);

    return status;
}
vx_status VX_CALLBACK vxRoipoolInternalKernel(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    vx_graph graph;
    if (num != 6) return VX_ERROR_INVALID_PARAMETERS;
    node->kernelAttributes.isAllGPU = vx_true_e;

    graph = vxoNode_GetChildGraph(node);

    return vxProcessGraph(graph);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxRoipoolFunction(vx_node node, const vx_reference parameters[], vx_uint32 paramCount)
{
    //vx_status status = vxProgramKernel_Function(node, parameters, paramCount);
    vx_graph graph;
    if (paramCount != 6)
        return VX_ERROR_INVALID_PARAMETERS;

    node->kernelAttributes.isAllGPU = vx_true_e;
    graph = vxoNode_GetChildGraph(node);

    return vxProcessGraph(graph);

}

//nms**********************************************
vx_status VX_CALLBACK vxNmsValidateInput(vx_node node, vx_uint32 index)
{
    vx_status status = VX_SUCCESS;
    vx_parameter param = NULL;

    if (index < 0 || index > 2)
        return VX_ERROR_INVALID_PARAMETERS;
    if(index == 0)
    {
           param = vxGetParameterByIndex(node, index);
        if(param != NULL)
        {
            vx_array array = NULL;
            vx_enum type = 0;
            if(VX_SUCCESS == vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &array, sizeof(array)))
            {
                vxQueryArray(array, VX_ARRAY_ATTRIBUTE_ITEMTYPE, &type, sizeof(type));
                if (type == VX_TYPE_FLOAT32) status = VX_SUCCESS;
                else status = VX_ERROR_INVALID_TYPE;
                status |= vxReleaseArray(&array);
            }
            status |= vxReleaseParameter(&param);
        }
    }



    if(status < 0)
        printf("error-%s,%d\n",__FILE__,__LINE__);
    return status;
}

vx_status VX_CALLBACK vxNmsValidateOutput(vx_node node, vx_uint32 index, vx_meta_format  metaObj)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    vx_parameter param = NULL;

      if (index < 0 || index >3)
        return VX_ERROR_INVALID_PARAMETERS;

    if(index == 0)
    {
        param = vxGetParameterByIndex(node, index);
        if(param != NULL)
        {
            vx_array array = NULL;
            if((VX_SUCCESS == vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &array, sizeof(vx_array))))
            {
                vx_enum type = 0;
                vx_size capacity = 0;
                vxQueryArray(array, VX_ARRAY_ATTRIBUTE_ITEMTYPE, &type, sizeof(type));
                vxQueryArray(array, VX_ARRAY_ATTRIBUTE_CAPACITY, &capacity, sizeof(capacity));
                status = vxSetMetaFormatAttribute(metaObj, VX_ARRAY_ATTRIBUTE_ITEMTYPE, &type, sizeof(type));
                status |= vxSetMetaFormatAttribute(metaObj, VX_ARRAY_ATTRIBUTE_CAPACITY, &capacity, sizeof(capacity));
                status |= vxSetArrayAttribute(array, VX_ARRAY_ATTRIBUTE_NUMITEMS, &capacity, sizeof(capacity));
                status |= vxReleaseArray(&array);
            }
            status |= vxReleaseParameter(&param);
        }
    }
    else if(index == 3)
    {
         vx_enum type = VX_TYPE_UINT32;
        status = VX_SUCCESS;
        status |= vxSetMetaFormatAttribute(metaObj, VX_SCALAR_ATTRIBUTE_TYPE, &type, sizeof(type));
    }

       if(status < 0)
        printf("error-%s,%d\n",__FILE__,__LINE__);

    return status;
}

vx_status VX_CALLBACK vxNmsInitializer(vx_node nodObj, const vx_reference *paramObj, vx_uint32 paraNum)
{

#ifdef USE_NMS_VXC
    vx_kernel_execution_parameters_t shaderParam = {
        2,          // workdim
        {0, 0, 0},  // globalWorkOffset: control the start location be processed in the image
        {0, 0, 0},  // globalWorkScale: how many pixels could be processed by a single thread
        {0, 0, 0},  // localWorkSize: local group size in threads
        {0, 0, 0}}; // globalWorkSize: image size in threads

    shaderParam.globalWorkOffset[0] = 0;
    shaderParam.globalWorkOffset[1] = 0;
    shaderParam.globalWorkScale[0]    = 1;
    shaderParam.globalWorkScale[1]    = 1;
    shaderParam.localWorkSize[0]    = 8;
    shaderParam.localWorkSize[1]    = 1;
    shaderParam.globalWorkSize[0]    = 8;//(((imgWid - offset + shaderParam.globalWorkScale[0]-1 ) / shaderParam.globalWorkScale[0] + shaderParam.localWorkSize[0] - 1) / shaderParam.localWorkSize[0]) * shaderParam.localWorkSize[0];
    shaderParam.globalWorkSize[1]    = 1;//(imgHei - offset + shaderParam.globalWorkScale[1] - 1) / shaderParam.globalWorkScale[1];
    {
    // Please cut the following declarations to VXC source file.
        vx_uint32 UniTestData[16] = {
        0x33333333, // TCfg
        0x11110000, // ASelt
        0x03020100, 0x03020100, // ABin
        0x00000000, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00005400, // AccumType, ConstantType, and PostShift
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
    };
    vxSetNodeUniform(nodObj, "UniTestData", 1, UniTestData);
    }

    vxSetNodeAttribute(nodObj, VX_NODE_ATTRIBUTE_KERNEL_EXECUTION_PARAMETERS, &shaderParam, sizeof(vx_kernel_execution_parameters_t));
#endif
    return VX_SUCCESS;
}

vx_status VX_CALLBACK vxNmsDeinitializer(vx_node nodObj, const vx_reference *paraObj, vx_uint32 paraNum)
{
    return VX_SUCCESS;
}
VX_PRIVATE_API vx_status VX_CALLBACK vxNmsFunction(vx_node node, const vx_reference parameters[], vx_uint32 paramCount)
{
    vx_status   status = VX_SUCCESS;
    /*step2: call the base function */
    status = vxProgramKernel_Function(node, parameters, paramCount);
    return status;

}

//fasterRcnnReshuffleImage*********************
VX_PRIVATE_API vx_status VX_CALLBACK vxfasterRcnnReshuffleImageInputValidator(vx_node node, vx_uint32 index)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    vx_parameter param = NULL;
     if(index == 0)
    {
        param = vxGetParameterByIndex(node, index);
        if(param != NULL)
        {
            vx_image img = NULL;
            vx_df_image format = 0;
            if((VX_SUCCESS == vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &img, sizeof(img))))
            {
                vxQueryImage(img, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
                if (VX_DF_IMAGE_RGB == format) status = VX_SUCCESS;
                else status = VX_ERROR_INVALID_VALUE;
                status |= vxReleaseImage(&img);
            }
            status |= vxReleaseParameter(&param);
        }
    }
    if(index == 1)
    {
           param = vxGetParameterByIndex(node, index);
        if(param != NULL)
        {
            vx_array array = NULL;
            vx_enum type = 0;
            if(VX_SUCCESS == vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &array, sizeof(array)))
            {
                vxQueryArray(array, VX_ARRAY_ATTRIBUTE_ITEMTYPE, &type, sizeof(type));
                if (type == VX_TYPE_FLOAT32) status = VX_SUCCESS;
                else status = VX_ERROR_INVALID_TYPE;
                status |= vxReleaseArray(&array);
            }
            status |= vxReleaseParameter(&param);
        }
    }
    else if(index == 2 || index == 3 || index == 4 || index == 5 || index == 6 || index == 7)
    {
        param = vxGetParameterByIndex(node, index);
        if(param != NULL)
        {
            vx_scalar scalar = NULL;
            vx_enum type = 0;
            if(VX_SUCCESS == vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &scalar, sizeof(scalar)))
            {
                vxQueryScalar(scalar, VX_SCALAR_ATTRIBUTE_TYPE, &type, sizeof(type));
                if (VX_TYPE_INT32 == type || VX_TYPE_UINT32 == type || VX_TYPE_UINT8 == type) status = VX_SUCCESS;
                else status = VX_ERROR_INVALID_TYPE;
                status |= vxReleaseScalar(&scalar);
            }
            status |= vxReleaseParameter(&param);
        }
    }
    if(status < 0)
        printf("error-%s,%d\n",__FILE__,__LINE__);
    return status;
}
// metaObj: specify the meta data of the expected output data object
VX_PRIVATE_API vx_status VX_CALLBACK vxfasterRcnnReshuffleImageOutputValidator(vx_node node, vx_uint32 index, vx_meta_format metaObj)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    vx_parameter param = NULL;

    if(index == 8)
    {
        param = vxGetParameterByIndex(node, index);
        if(param != NULL)
        {
            vx_array array = NULL;
            if((VX_SUCCESS == vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &array, sizeof(vx_array))))
            {
                vx_enum type = 0;
                vx_size capacity = 0;
                vxQueryArray(array, VX_ARRAY_ATTRIBUTE_ITEMTYPE, &type, sizeof(type));
                vxQueryArray(array, VX_ARRAY_ATTRIBUTE_CAPACITY, &capacity, sizeof(capacity));
                status = vxSetMetaFormatAttribute(metaObj, VX_ARRAY_ATTRIBUTE_ITEMTYPE, &type, sizeof(type));
                status |= vxSetMetaFormatAttribute(metaObj, VX_ARRAY_ATTRIBUTE_CAPACITY, &capacity, sizeof(capacity));
                status |= vxSetArrayAttribute(array, VX_ARRAY_ATTRIBUTE_NUMITEMS, &capacity, sizeof(capacity));
                status |= vxReleaseArray(&array);
            }
            status |= vxReleaseParameter(&param);
        }
    }
    if(status < 0)
        printf("error-%s,%d\n",__FILE__,__LINE__);
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxfasterRcnnReshuffleImageInitializer(vx_node nodObj, const vx_reference *paramObj, vx_uint32 paraNum)
{
#ifdef USE_FASTERRCNNRESHUFFLEIMAGE_VXC
    vx_kernel_execution_parameters_t shaderParam = {
        2,          // workdim
        {0, 0, 0},  // globalWorkOffset: control the start location be processed in the image
        {0, 0, 0},  // globalWorkScale: how many pixels could be processed by a single thread
        {0, 0, 0},  // localWorkSize: local group size in threads
        {0, 0, 0}}; // globalWorkSize: image size in threads

    vx_int32 srcWidth = 0,srcHeight = 0, imgWid = 0, imgHei = 0;
    vx_int32 channel_s = 3;
    vx_uint32 val_MeanFlag = 0, padScalarValue = 0, padWidth = 0, padHeight = 0;
    vx_uint8 strideXValue, strideYValue;
    vx_uint32 levelIndex, batchSizeValue, dstWidth, dstHeight, dstDepth;
    vx_image  imgObj      = (vx_image)paramObj[0];
    vx_array  mean_array  = (vx_array)paramObj[1];
    vx_scalar levelScalar = (vx_scalar)paramObj[2];
    vx_scalar padScalar   = (vx_scalar)paramObj[3];
    vx_scalar strideX     = (vx_scalar)paramObj[4];
    vx_scalar strideY     = (vx_scalar)paramObj[5];
    vx_scalar batchSize     = (vx_scalar)paramObj[6];

    levelIndex       = levelScalar->value->u32;
    padScalarValue   = padScalar->value->u32;
    strideXValue     = strideX->value->u8;
    strideYValue     = strideY->value->u8;
    batchSizeValue   = batchSize->value->u8;
    srcWidth = imgObj->width;
    srcHeight = nodObj->kernel->cnnAttributes.cnnkernelStreamInfo[levelIndex].orgInImageYSize;
    imgWid = srcWidth * channel_s;

    padWidth = padScalarValue + padScalarValue;
    padHeight = padScalarValue + padScalarValue;
    dstWidth  = gcmALIGN(padWidth + srcWidth, strideXValue)/strideXValue;
    dstHeight = gcmALIGN(padHeight + srcHeight, strideYValue)/strideYValue;
    dstDepth  = nodObj->kernel->cnnAttributes.cnnkernelStreamInfo[levelIndex].orgInImageZSize * strideXValue * strideYValue;

    if (mean_array != NULL)
    {
        val_MeanFlag = 1;
        imgHei = srcHeight;

        shaderParam.globalWorkOffset[0] = 0;
        shaderParam.globalWorkOffset[1] = 0;
        shaderParam.globalWorkScale[0]  = 24;
        shaderParam.globalWorkScale[1]  = 2;
        shaderParam.localWorkSize[0]    = 4;
        shaderParam.localWorkSize[1]    = 4;
        shaderParam.globalWorkSize[0]   = (((imgWid + shaderParam.globalWorkScale[0] - 1) / shaderParam.globalWorkScale[0] + shaderParam.localWorkSize[0] - 1) / shaderParam.localWorkSize[0]) * shaderParam.localWorkSize[0];
        shaderParam.globalWorkSize[1]   = (((imgHei + shaderParam.globalWorkScale[1] - 1) / shaderParam.globalWorkScale[1] + shaderParam.localWorkSize[1] - 1) / shaderParam.localWorkSize[1]) * shaderParam.localWorkSize[1];
        {
            // uniforms
            vx_uint32 UniPackRevenU8toFP16_P1[16] = {
                0x00001111, // TCfg
                0x00001100, // ASelt
                0x08020802, 0x00000000, // ABin
                0x00002222, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00000100, // AccumType, ConstantType, and PostShift
                0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
            };
            vx_uint32 UniPackRoddU8toFP16_P1[16] = {
                0x00001111, // TCfg
                0x00001100, // ASelt
                0x0b050b05, 0x00000000, // ABin
                0x00002222, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00000100, // AccumType, ConstantType, and PostShift
                0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
            };
            vx_uint32 UniPackGevenU8toFP16_P1[16] = {
                0x00001111, // TCfg
                0x00001100, // ASelt
                0x07010701, 0x00000000, // ABin
                0x00002222, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00000100, // AccumType, ConstantType, and PostShift
                0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
            };
            vx_uint32 UniPackGoddU8toFP16_P1[16] = {
                0x00001111, // TCfg
                0x00001100, // ASelt
                0x0a040a04, 0x00000000, // ABin
                0x00002222, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00000100, // AccumType, ConstantType, and PostShift
                0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
            };
            vx_uint32 UniPackBevenU8toFP16_P1[16] = {
                0x00001111, // TCfg
                0x00001100, // ASelt
                0x06000600, 0x00000000, // ABin
                0x00002222, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00000100, // AccumType, ConstantType, and PostShift
                0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
            };
            vx_uint32 UniPackBoddU8toFP16_P1[16] = {
                0x00001111, // TCfg
                0x00001100, // ASelt
                0x09030903, 0x00000000, // ABin
                0x00002222, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00000100, // AccumType, ConstantType, and PostShift
                0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
            };
            vx_uint32 UniPackLow16bits2x8_P1[16] = {
                0x00001111, // TCfg
                0x00000000, // ASelt
                0x06040200, 0x00000000, // ABin
                0x00002222, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00000100, // AccumType, ConstantType, and PostShift
                0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
            };
            vx_uint32 UniPackLow16bits2x8_P2[16] = {
                0x11111111, // TCfg
                0x11110000, // ASelt
                0x03020100, 0x06040200, // ABin
                0x22222222, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00000100, // AccumType, ConstantType, and PostShift
                0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00 // Constant
            };
            vx_uint32 UniPackMean_even[16] = {
                0x00001111, // TCfg
                0x00000000, // ASelt
                0x06040200, 0x00000000, // ABin
                0x00002222, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00000400, // AccumType, ConstantType, and PostShift
                0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
            };
            vx_uint32 UniPackMean_odd[16] = {
                0x00001111, // TCfg
                0x00000000, // ASelt
                0x07050301, 0x00000000, // ABin
                0x00002222, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00000400, // AccumType, ConstantType, and PostShift
                0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
            };
            vx_uint32 UniSrcSubMean2x8[16] = {
                0x00009999, // TCfg
                0x00004444, // ASelt
                0x33221100, 0x00000000, // ABin
                0x0000aaaa, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00000100, // AccumType, ConstantType, and PostShift
                0x3c003c00, 0x3c003c00, 0x3c003c00, 0x3c003c00, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
            };
            vxSetNodeUniform(nodObj, "UniPackRevenU8toFP16_P1", 1, UniPackRevenU8toFP16_P1);
            vxSetNodeUniform(nodObj, "UniPackRoddU8toFP16_P1", 1, UniPackRoddU8toFP16_P1);
            vxSetNodeUniform(nodObj, "UniPackGevenU8toFP16_P1", 1, UniPackGevenU8toFP16_P1);
            vxSetNodeUniform(nodObj, "UniPackGoddU8toFP16_P1", 1, UniPackGoddU8toFP16_P1);
            vxSetNodeUniform(nodObj, "UniPackBevenU8toFP16_P1", 1, UniPackBevenU8toFP16_P1);
            vxSetNodeUniform(nodObj, "UniPackBoddU8toFP16_P1", 1, UniPackBoddU8toFP16_P1);
            vxSetNodeUniform(nodObj, "UniPackMean_even", 1, UniPackMean_even);
            vxSetNodeUniform(nodObj, "UniPackMean_odd", 1, UniPackMean_odd);
            vxSetNodeUniform(nodObj, "UniSrcSubMean2x8", 1, UniSrcSubMean2x8);
            vxSetNodeUniform(nodObj, "UniPackLow16bits2x8_P1", 1, UniPackLow16bits2x8_P1);
            vxSetNodeUniform(nodObj, "UniPackLow16bits2x8_P2", 1, UniPackLow16bits2x8_P2);

        }
    }
    else
    {
        imgHei = srcHeight * batchSizeValue;
        shaderParam.globalWorkOffset[0] = 0;
        shaderParam.globalWorkOffset[1] = 0;
        shaderParam.globalWorkScale[0]  = imgWid;
        shaderParam.globalWorkScale[1]  = 1;
        shaderParam.localWorkSize[0]    = 1;
        shaderParam.localWorkSize[1]    = 8;
        shaderParam.globalWorkSize[0]   = (((imgWid + shaderParam.globalWorkScale[0] - 1) / shaderParam.globalWorkScale[0] + shaderParam.localWorkSize[0] - 1) / shaderParam.localWorkSize[0]) * shaderParam.localWorkSize[0];
        shaderParam.globalWorkSize[1]   = (((imgHei + shaderParam.globalWorkScale[1] - 1) / shaderParam.globalWorkScale[1] + shaderParam.localWorkSize[1] - 1) / shaderParam.localWorkSize[1]) * shaderParam.localWorkSize[1];
        {
            //uniform
            vx_uint32 Step1PackU8_B[16] = {
                0x33333333, // TCfg
                0x11110000, // ASelt
                0x09060300, 0x09060300, // ABin
                0x00000000, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00007700, // AccumType, ConstantType, and PostShift
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
            };
            vx_uint32 Step1PackU8_G[16] = {
                0x33333333, // TCfg
                0x11110000, // ASelt
                0x0a070401, 0x0a070401, // ABin
                0x00000000, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00007700, // AccumType, ConstantType, and PostShift
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
            };
            vx_uint32 Step1PackU8_R[16] = {
                0x33333333, // TCfg
                0x11110000, // ASelt
                0x0b080502, 0x0b080502, // ABin
                0x00000000, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00007700, // AccumType, ConstantType, and PostShift
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
            };
            vx_uint32 Step2PackU8_B03[16] = {
                0x33333333, // TCfg
                0x11001100, // ASelt
                0x0c080400, 0x0d090501, // ABin
                0x00000000, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00007700, // AccumType, ConstantType, and PostShift
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
            };
            vx_uint32 Step2PackU8_B69[16] = {
                0x33333333, // TCfg
                0x11001100, // ASelt
                0x0e0a0602, 0x0f0b0703, // ABin
                0x00000000, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00007700, // AccumType, ConstantType, and PostShift
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
            };
            vx_uint32 Step2PackU8_G14[16] = {
                0x33333333, // TCfg
                0x11001100, // ASelt
                0x04000c08, 0x05010d09, // ABin
                0x00000000, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00007700, // AccumType, ConstantType, and PostShift
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
            };
            vx_uint32 Step2PackU8_G710[16] = {
                0x33333333, // TCfg
                0x11001100, // ASelt
                0x06020e0a, 0x07030f0b, // ABin
                0x00000000, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00007700, // AccumType, ConstantType, and PostShift
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
            };
            vx_uint32 Step2PackU8_R25[16] = {
                0x33333333, // TCfg
                0x11001100, // ASelt
                0x0c080400, 0x0d090501, // ABin
                0x00000000, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00007700, // AccumType, ConstantType, and PostShift
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
            };
            vx_uint32 Step2PackU8_R811[16] = {
                0x33333333, // TCfg
                0x11001100, // ASelt
                0x0e0a0602, 0x0f0b0703, // ABin
                0x00000000, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00007700, // AccumType, ConstantType, and PostShift
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
            };
            vx_uint32 Step3PackU8toFP16_pos0[16] = {
                0x11111111, // TCfg
                0x11110000, // ASelt
                0x03020100, 0x03020100, // ABin
                0x22222222, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00000100, // AccumType, ConstantType, and PostShift
                0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00 // Constant
            };
            vx_uint32 Step3PackU8toFP16_pos1[16] = {
                0x11111111, // TCfg
                0x11110000, // ASelt
                0x07060504, 0x07060504, // ABin
                0x22222222, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00000100, // AccumType, ConstantType, and PostShift
                0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00 // Constant
            };
            vx_uint32 Step3PackU8toFP16_pos2[16] = {
                0x11111111, // TCfg
                0x11110000, // ASelt
                0x0b0a0908, 0x0b0a0908, // ABin
                0x22222222, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00000100, // AccumType, ConstantType, and PostShift
                0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00 // Constant
            };
            vx_uint32 Step3PackU8toFP16_pos3[16] = {
                0x11111111, // TCfg
                0x11110000, // ASelt
                0x0f0e0d0c, 0x0f0e0d0c, // ABin
                0x22222222, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00000100, // AccumType, ConstantType, and PostShift
                0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00 // Constant
            };
            vx_uint32 Last9PackU8toFP16_P1[16] = {
                0x11111111, // TCfg
                0x00000000, // ASelt
                0x01080502, 0x03000704, // ABin
                0x22222222, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00000100, // AccumType, ConstantType, and PostShift
                0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00 // Constant
            };
            vx_uint32 Last9PackU8toFP16_P2[16] = {
                0x00000001, // TCfg
                0x00000000, // ASelt
                0x00000006, 0x00000000, // ABin
                0x00000002, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00000100, // AccumType, ConstantType, and PostShift
                0x00003c00, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
            };
            vxSetNodeUniform(nodObj, "Step1PackU8_B", 1, Step1PackU8_B);
            vxSetNodeUniform(nodObj, "Step1PackU8_G", 1, Step1PackU8_G);
            vxSetNodeUniform(nodObj, "Step1PackU8_R", 1, Step1PackU8_R);
            vxSetNodeUniform(nodObj, "Step2PackU8_B03", 1, Step2PackU8_B03);
            vxSetNodeUniform(nodObj, "Step2PackU8_B69", 1, Step2PackU8_B69);
            vxSetNodeUniform(nodObj, "Step2PackU8_G14", 1, Step2PackU8_G14);
            vxSetNodeUniform(nodObj, "Step2PackU8_G710", 1, Step2PackU8_G710);
            vxSetNodeUniform(nodObj, "Step2PackU8_R25", 1, Step2PackU8_R25);
            vxSetNodeUniform(nodObj, "Step2PackU8_R811", 1, Step2PackU8_R811);
            vxSetNodeUniform(nodObj, "Step3PackU8toFP16_pos0", 1, Step3PackU8toFP16_pos0);
            vxSetNodeUniform(nodObj, "Step3PackU8toFP16_pos1", 1, Step3PackU8toFP16_pos1);
            vxSetNodeUniform(nodObj, "Step3PackU8toFP16_pos2", 1, Step3PackU8toFP16_pos2);
            vxSetNodeUniform(nodObj, "Step3PackU8toFP16_pos3", 1, Step3PackU8toFP16_pos3);
            vxSetNodeUniform(nodObj, "Last9PackU8toFP16_P1", 1, Last9PackU8toFP16_P1);
            vxSetNodeUniform(nodObj, "Last9PackU8toFP16_P2", 1, Last9PackU8toFP16_P2);
        }
    }
    {
        vx_uint32 img_inWidth[1] = {imgWid};
        vx_uint32 img_inHeight[1] = {srcHeight};
        vx_uint32 out_Width[1] = {dstWidth};
        vx_uint32 out_Height[1] = {dstHeight};
        vx_uint32 out_Depth[1] = {dstDepth};
        vx_uint32 MeanFlag[1] = {val_MeanFlag};

        vxSetNodeUniform(nodObj, "img_inWidth", 1, img_inWidth);
        vxSetNodeUniform(nodObj, "img_inHeight", 1, img_inHeight);
        vxSetNodeUniform(nodObj, "out_Width", 1, out_Width);
        vxSetNodeUniform(nodObj, "out_Height", 1, out_Height);
        vxSetNodeUniform(nodObj, "out_Depth", 1, out_Depth);
        vxSetNodeUniform(nodObj, "MeanFlag", 1, MeanFlag);
    }

    vxSetNodeAttribute(nodObj, VX_NODE_ATTRIBUTE_KERNEL_EXECUTION_PARAMETERS, &shaderParam, sizeof(vx_kernel_execution_parameters_t));
#endif
    return VX_SUCCESS;
}
VX_PRIVATE_API vx_status VX_CALLBACK vxfasterRcnnReshuffleImageFunction(vx_node node, const vx_reference parameters[], vx_uint32 paramCount)
{
    vx_status   status;

    vx_reference *ucParameters = (vx_reference*)parameters;

    /*step1: pre-process */
    vx_image_s   imgObj1, imgObj2, imgObj3;
    //vx_uint32 srcWidth, srcHeight;
    vx_uint32 channel_s = 3;

    vx_image imgObj       = (vx_image)parameters[0];
    vx_array input_array  = (vx_array)parameters[1];
    vx_scalar levelScalar = (vx_scalar)parameters[2];
    vx_scalar padScalar   = (vx_scalar)parameters[3];
    vx_scalar strideX     = (vx_scalar)parameters[4];
    vx_scalar strideY     = (vx_scalar)parameters[5];
    vx_scalar batchSize   = (vx_scalar)parameters[6];
    vx_array output_array = (vx_array)parameters[8];
    vx_uint32 levelIndex, padScalarValue, out_xSize, out_ySize, out_zSize;
    vx_uint8 strideXValue, strideYValue;
    vx_uint32 batchSizeValue, width_s, height_s, padWidth = 0, padHeight = 0;

    levelIndex       = levelScalar->value->u32;
    levelIndex       = levelScalar->value->u32;
    padScalarValue   = padScalar->value->u32;
    strideXValue     = strideX->value->u8;
    strideYValue     = strideY->value->u8;
    batchSizeValue   = batchSize->value->u8;

    width_s = imgObj->width;
    height_s = node->kernel->cnnAttributes.cnnkernelStreamInfo[levelIndex].orgInImageYSize;
    padWidth = padScalarValue + padScalarValue;
    padHeight = padScalarValue + padScalarValue;
    out_xSize = gcmALIGN(padWidth + width_s, strideXValue)/strideXValue;
    out_ySize = gcmALIGN(padHeight + height_s, strideYValue)/strideYValue;
    out_zSize  = node->kernel->cnnAttributes.cnnkernelStreamInfo[levelIndex].orgInImageZSize * strideXValue * strideYValue;
    if (input_array != NULL)
    {
        memset(output_array->memory.logicals[0], 0, out_xSize * out_ySize * out_zSize * sizeof(vx_int16));
        {
            gcoOS_ZeroMemory(&imgObj1, sizeof(vx_image_s));

            imgObj1.base = imgObj->base;
            imgObj1.base.type = VX_TYPE_IMAGE;

            imgObj1.width = width_s * channel_s;
            imgObj1.height = height_s;
            imgObj1.format = VX_DF_IMAGE_U8;
            imgObj1.planeCount = 1;
            imgObj1.memory = imgObj->memory;
        }
        {
            int itemSize = (int)input_array->itemSize;
            gcoOS_ZeroMemory(&imgObj2, sizeof(vx_image_s));

            imgObj2.base = input_array->base;
            imgObj2.base.type = VX_TYPE_IMAGE;

            imgObj2.width = width_s;
            imgObj2.height = height_s * channel_s;
            imgObj2.format = (itemSize == 4)?VX_DF_IMAGE_F32:VX_DF_IMAGE_S16;
            imgObj2.planeCount = 1;
            imgObj2.memory = input_array->memory;
        }
        {
            int itemSize = (int)output_array->itemSize;
            gcoOS_ZeroMemory(&imgObj3, sizeof(vx_image_s));

            imgObj3.base = output_array->base;
            imgObj3.base.type = VX_TYPE_IMAGE;
            imgObj3.width = out_xSize;
            imgObj3.height = out_ySize * out_zSize;
            imgObj3.format = (itemSize == 4)?VX_DF_IMAGE_F32:VX_DF_IMAGE_S16;
            imgObj3.planeCount = 1;
            imgObj3.memory = output_array->memory;
        }
    }
    else
    {
        memset(output_array->memory.logicals[0], 0, out_xSize * out_ySize * out_zSize * batchSizeValue * sizeof(vx_int16));
        {
            gcoOS_ZeroMemory(&imgObj1, sizeof(vx_image_s));

            imgObj1.base = imgObj->base;
            imgObj1.base.type = VX_TYPE_IMAGE;

            imgObj1.width = width_s * channel_s;
            imgObj1.height = height_s * batchSizeValue;
            imgObj1.format = VX_DF_IMAGE_U8;
            imgObj1.planeCount = 1;
            imgObj1.memory = imgObj->memory;
        }
        {
            gcoOS_ZeroMemory(&imgObj2, sizeof(vx_image_s));

            imgObj2.base.type = VX_TYPE_IMAGE;
        }
        {
            int itemSize = (int)output_array->itemSize;
            gcoOS_ZeroMemory(&imgObj3, sizeof(vx_image_s));

            imgObj3.base = output_array->base;
            imgObj3.base.type = VX_TYPE_IMAGE;
            imgObj3.width = out_xSize * out_ySize;
            imgObj3.height = out_zSize * batchSizeValue;
            imgObj3.format = (itemSize == 4)?VX_DF_IMAGE_F32:VX_DF_IMAGE_S16;
            imgObj3.planeCount = 1;
            imgObj3.memory = output_array->memory;
        }
    }
    ucParameters[0] = (vx_reference)(&imgObj1);
    ucParameters[1] = (vx_reference)(&imgObj2);
    ucParameters[8] = (vx_reference)(&imgObj3);

    /*step2: call the base function */
    status = vxProgramKernel_Function(node, parameters, paramCount);

    /* step3 : post-process */
    ucParameters[0] = (vx_reference)(imgObj);
    ucParameters[1] = (vx_reference)(input_array);
    ucParameters[8] = (vx_reference)(output_array);

#if (NN_MULTI_THREAD || NN_MULTI_THREAD2)
    if (node->base.context->cnnEvent != VX_NULL)
        vxSetEvent(node->base.context->cnnEvent);
#endif
    return status;
}
VX_PRIVATE_API vx_status VX_CALLBACK vxfasterRcnnReshuffleImageDeinitializer(vx_node nodObj, const vx_reference *paraObj, vx_uint32 paraNum)
{
    return VX_SUCCESS;
}
//fasterRcnnInterleave*********************
VX_PRIVATE_API vx_status VX_CALLBACK vxFasterRcnnInterleaveInputValidator(vx_node node, vx_uint32 index)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    vx_parameter param = NULL;

    if(index == 0)
    {
        status = VX_SUCCESS;
    }
    else if(index == 1 || index == 2)
    {
        param = vxGetParameterByIndex(node, index);
        if(param != NULL)
        {
            vx_scalar scalar = NULL;
            vx_enum type = 0;
            if(VX_SUCCESS == vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &scalar, sizeof(scalar)))
            {
                vxQueryScalar(scalar, VX_SCALAR_ATTRIBUTE_TYPE, &type, sizeof(type));
                if (VX_TYPE_INT32 == type || VX_TYPE_UINT32 == type) status = VX_SUCCESS;
                else status = VX_ERROR_INVALID_TYPE;
                status |= vxReleaseScalar(&scalar);
            }
            status |= vxReleaseParameter(&param);
        }
    }
    else  if(index == 3 || index == 4)
        status = VX_SUCCESS;

    if(status < 0)
        printf("error-%s,%d\n",__FILE__,__LINE__);
    return status;
}
// metaObj: specify the meta data of the expected output data object
VX_PRIVATE_API vx_status VX_CALLBACK vxFasterRcnnInterleaveOutputValidator(vx_node node, vx_uint32 index, vx_meta_format metaObj)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    vx_parameter param = NULL;

    if(index == 5)
    {
        param = vxGetParameterByIndex(node, index);
        if(param != NULL)
        {
            vx_array array = NULL;
            if((VX_SUCCESS == vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &array, sizeof(vx_array))))
            {
                vx_enum type = 0;
                vx_size capacity = 0;
                vxQueryArray(array, VX_ARRAY_ATTRIBUTE_ITEMTYPE, &type, sizeof(type));
                vxQueryArray(array, VX_ARRAY_ATTRIBUTE_CAPACITY, &capacity, sizeof(capacity));
                status = vxSetMetaFormatAttribute(metaObj, VX_ARRAY_ATTRIBUTE_ITEMTYPE, &type, sizeof(type));
                status |= vxSetMetaFormatAttribute(metaObj, VX_ARRAY_ATTRIBUTE_CAPACITY, &capacity, sizeof(capacity));
                status |= vxSetArrayAttribute(array, VX_ARRAY_ATTRIBUTE_NUMITEMS, &capacity, sizeof(capacity));
                status |= vxReleaseArray(&array);
            }
            status |= vxReleaseParameter(&param);
        }
    }
    if(status < 0)
        printf("error-%s,%d\n",__FILE__,__LINE__);
    return status;
}
VX_PRIVATE_API vx_status VX_CALLBACK vxFasterRcnnInterleaveInitializer(vx_node nodObj, const vx_reference *paramObj, vx_uint32 paraNum)
{
#ifdef USE_FASTERRCNNINTERLEAVE_VXC
    vx_kernel_execution_parameters_t shaderParam = {
        2,          // workdim
        {0, 0, 0},  // globalWorkOffset: control the start location be processed in the image
        {0, 0, 0},  // globalWorkScale: how many pixels could be processed by a single thread
        {0, 0, 0},  // localWorkSize: local group size in threads
        {0, 0, 0}}; // globalWorkSize: image size in threads

    vx_int32 imgWid = 0, imgHei = 0;
    vx_scalar scalarObjW = (vx_scalar)paramObj[1];
    vx_scalar scalarObjH = (vx_scalar)paramObj[2];
    vxReadScalarValue(scalarObjW, &imgWid);
    vxWriteScalarValue(scalarObjW, &imgWid);
    vxReadScalarValue(scalarObjH, &imgHei);
    vxWriteScalarValue(scalarObjH, &imgHei);

    shaderParam.globalWorkOffset[0] = 0;
    shaderParam.globalWorkOffset[1] = 0;
    shaderParam.globalWorkScale[0]  = 8;
    shaderParam.globalWorkScale[1]  = 8;
    shaderParam.localWorkSize[0]    = 8;
    shaderParam.localWorkSize[1]    = 1;
    shaderParam.globalWorkSize[0]   = (((imgWid + shaderParam.globalWorkScale[0] - 1 ) / shaderParam.globalWorkScale[0] + shaderParam.localWorkSize[0] - 1) / shaderParam.localWorkSize[0]) * shaderParam.localWorkSize[0];
    shaderParam.globalWorkSize[1]   = (((imgHei + shaderParam.globalWorkScale[1] - 1) / shaderParam.globalWorkScale[1] + shaderParam.localWorkSize[1] - 1) / shaderParam.localWorkSize[1]) * shaderParam.localWorkSize[1];
    {
        // uniforms
        vx_uint32 InterleaveStep1PackHead4[16] = {
            0x33333333, // TCfg
            0x10101010, // ASelt
            0x01010000, 0x03030202, // ABin
            0x00000000, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00003400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        vx_uint32 InterleaveStep1PackLast4[16] = {
            0x33333333, // TCfg
            0x10101010, // ASelt
            0x05050404, 0x07070606, // ABin
            0x00000000, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00003400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        vx_uint32 InterleaveStep2PackHead4[16] = {
            0x33333333, // TCfg
            0x11001100, // ASelt
            0x01000100, 0x03020302, // ABin
            0x00000000, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00003400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        vx_uint32 InterleaveStep2PackLast4[16] = {
            0x33333333, // TCfg
            0x11001100, // ASelt
            0x05040504, 0x07060706, // ABin
            0x00000000, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00003400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        vx_uint32 InterleaveStep3PackHead4[16] = {
            0x33333333, // TCfg
            0x11110000, // ASelt
            0x03020100, 0x03020100, // ABin
            0x00000000, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00003400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        vx_uint32 InterleaveStep3PackLast4[16] = {
            0x33333333, // TCfg
            0x11110000, // ASelt
            0x07060504, 0x07060504, // ABin
            0x00000000, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00003400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        vxSetNodeUniform(nodObj, "InterleaveStep1PackHead4", 1, InterleaveStep1PackHead4);
        vxSetNodeUniform(nodObj, "InterleaveStep1PackLast4", 1, InterleaveStep1PackLast4);
        vxSetNodeUniform(nodObj, "InterleaveStep2PackHead4", 1, InterleaveStep2PackHead4);
        vxSetNodeUniform(nodObj, "InterleaveStep2PackLast4", 1, InterleaveStep2PackLast4);
        vxSetNodeUniform(nodObj, "InterleaveStep3PackHead4", 1, InterleaveStep3PackHead4);
        vxSetNodeUniform(nodObj, "InterleaveStep3PackLast4", 1, InterleaveStep3PackLast4);
    }
    vxSetNodeAttribute(nodObj, VX_NODE_ATTRIBUTE_KERNEL_EXECUTION_PARAMETERS, &shaderParam, sizeof(vx_kernel_execution_parameters_t));
#endif
    return VX_SUCCESS;
}
VX_PRIVATE_API vx_status VX_CALLBACK vxFasterRcnnInterleaveFunction(vx_node node, const vx_reference parameters[], vx_uint32 paramCount)
{
    vx_status   status;

    vx_reference *ucParameters = (vx_reference*)parameters;

    /*step1: pre-process */
    vx_image_s  imgObj, imgObj2;

    vx_array input_array = (vx_array)parameters[0];
    vx_scalar width = (vx_scalar)parameters[1];
    vx_scalar height = (vx_scalar)parameters[2];
#if NN_MULTI_THREAD2
    vx_scalar set_event = (vx_scalar)parameters[4];
#endif
    vx_array output_array = (vx_array)parameters[5];

    int width_s, height_s;

    vxReadScalarValue(width, &width_s);
    vxReadScalarValue(height, &height_s);

    {
        int itemSize = (int)input_array->itemSize;
        gcoOS_ZeroMemory(&imgObj, sizeof(vx_image_s));

        imgObj.base = input_array->base;
        imgObj.base.type = VX_TYPE_IMAGE;

        imgObj.width = width_s;
        imgObj.height = height_s;
        imgObj.format = (itemSize == 4)?VX_DF_IMAGE_F32:VX_DF_IMAGE_S16;
        imgObj.planeCount = 1;
        imgObj.memory = input_array->memory;
    }

    {
        int itemSize = (int)output_array->itemSize;
        gcoOS_ZeroMemory(&imgObj2, sizeof(vx_image_s));

        imgObj2.base = output_array->base;
        imgObj2.base.type = VX_TYPE_IMAGE;
        imgObj2.width = height_s;
        imgObj2.height = width_s;
        imgObj2.format = (itemSize == 4)?VX_DF_IMAGE_F32:VX_DF_IMAGE_S16;
        imgObj2.planeCount = 1;
        imgObj2.memory = output_array->memory;
    }


    ucParameters[0] = (vx_reference)(&imgObj);
    ucParameters[5] = (vx_reference)(&imgObj2);

    /*step2: call the base function */
    status = vxProgramKernel_Function(node, parameters, paramCount);

    /* step3 : post-process */
    ucParameters[0] = (vx_reference)(input_array);
    ucParameters[5] = (vx_reference)(output_array);


#if NN_MULTI_THREAD2
    if (node->base.context->cnnEvent != VX_NULL && set_event->value->b)
    {
        gcoVX_Flush(gcvTRUE);

        vxSetEvent(node->base.context->cnnEvent);
    }
#endif

    return status;
}
VX_PRIVATE_API vx_status VX_CALLBACK vxFasterRcnnInterleaveDeinitializer(vx_node nodObj, const vx_reference *paraObj, vx_uint32 paraNum)
{
    return VX_SUCCESS;
}
//fasterRcnnReshuffleData*********************
VX_PRIVATE_API vx_status VX_CALLBACK vxfasterRcnnReshuffleDataInputValidator(vx_node node, vx_uint32 index)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    vx_parameter param = NULL;
    if(index == 0)
    {
           param = vxGetParameterByIndex(node, index);
        if(param != NULL)
        {
            vx_array array = NULL;
            vx_enum type = 0;
            if(VX_SUCCESS == vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &array, sizeof(array)))
            {
                vxQueryArray(array, VX_ARRAY_ATTRIBUTE_ITEMTYPE, &type, sizeof(type));
                if (type == VX_TYPE_INT16) status = VX_SUCCESS;
                else status = VX_ERROR_INVALID_TYPE;
                status |= vxReleaseArray(&array);
            }
            status |= vxReleaseParameter(&param);
        }
    }
    else if(index == 1 || index == 2 || index == 3 || index == 4 || index == 5)
    {
        param = vxGetParameterByIndex(node, index);
        if(param != NULL)
        {
            vx_scalar scalar = NULL;
            vx_enum type = 0;
            if(VX_SUCCESS == vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &scalar, sizeof(scalar)))
            {
                vxQueryScalar(scalar, VX_SCALAR_ATTRIBUTE_TYPE, &type, sizeof(type));
                if (VX_TYPE_INT32 == type  || VX_TYPE_UINT32 == type || VX_TYPE_UINT8 == type) status = VX_SUCCESS;
                else status = VX_ERROR_INVALID_TYPE;
                status |= vxReleaseScalar(&scalar);
            }
            status |= vxReleaseParameter(&param);
        }
    }
    if(status < 0)
        printf("error-%s,%d\n",__FILE__,__LINE__);
    return status;
}
// metaObj: specify the meta data of the expected output data object
VX_PRIVATE_API vx_status VX_CALLBACK vxfasterRcnnReshuffleDataOutputValidator(vx_node node, vx_uint32 index, vx_meta_format metaObj)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    vx_parameter param = NULL;

    if(index == 6)
    {
        param = vxGetParameterByIndex(node, index);
        if(param != NULL)
        {
            vx_array array = NULL;
            if((VX_SUCCESS == vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &array, sizeof(vx_array))))
            {
                vx_enum type = 0;
                vx_size capacity = 0;
                vxQueryArray(array, VX_ARRAY_ATTRIBUTE_ITEMTYPE, &type, sizeof(type));
                vxQueryArray(array, VX_ARRAY_ATTRIBUTE_CAPACITY, &capacity, sizeof(capacity));
                status = vxSetMetaFormatAttribute(metaObj, VX_ARRAY_ATTRIBUTE_ITEMTYPE, &type, sizeof(type));
                status |= vxSetMetaFormatAttribute(metaObj, VX_ARRAY_ATTRIBUTE_CAPACITY, &capacity, sizeof(capacity));
                status |= vxSetArrayAttribute(array, VX_ARRAY_ATTRIBUTE_NUMITEMS, &capacity, sizeof(capacity));
                status |= vxReleaseArray(&array);
            }
            status |= vxReleaseParameter(&param);
        }
    }
    if(status < 0)
        printf("error-%s,%d\n",__FILE__,__LINE__);
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxfasterRcnnReshuffleDataInitializer(vx_node nodObj, const vx_reference *paramObj, vx_uint32 paraNum)
{
#ifdef USE_FASTERRCNNRESHUFFLEDATA_VXC
    vx_kernel_execution_parameters_t shaderParam = {
        2,          // workdim
        {0, 0, 0},  // globalWorkOffset: control the start location be processed in the image
        {0, 0, 0},  // globalWorkScale: how many pixels could be processed by a single thread
        {0, 0, 0},  // localWorkSize: local group size in threads
        {0, 0, 0}}; // globalWorkSize: image size in threads

    vx_uint32 imgWid = 0, imgHei = 0, padWidth = 0, padHeight = 0;
    vx_uint32 levelIndex = 0, strideXValue = 0, strideYValue =  0;
    vx_uint32 src_xSize = 0, src_ySize = 0, src_zSize = 0, out_xSize = 0, out_ySize = 0, out_zSize = 0;
    vx_scalar levelScalar = (vx_scalar)paramObj[1];
    vx_scalar strideX     = (vx_scalar)paramObj[2];
    vx_scalar strideY     = (vx_scalar)paramObj[3];
    levelIndex       = levelScalar->value->u32;
    strideXValue     = strideX->value->u8;
    strideYValue     = strideY->value->u8;
    src_xSize = nodObj->kernel->cnnAttributes.cnnkernelStreamInfo[levelIndex].orgInImageXSize;
    src_ySize = nodObj->kernel->cnnAttributes.cnnkernelStreamInfo[levelIndex].orgInImageYSize;
    src_zSize = nodObj->kernel->cnnAttributes.cnnkernelStreamInfo[levelIndex].orgInImageZSize;
    imgWid = src_xSize;
    imgHei = src_zSize;
    if (levelIndex == 0) /* pad is 3 */
    {
        padWidth = 3 + 3;
        padHeight = 3 + 3;
    }
    else if (levelIndex == 1) /* pad is 2 */
    {
        padWidth = 2 + 2;
        padHeight = 2 + 2;
    }

    out_xSize = gcmALIGN(padWidth + src_xSize, strideXValue)/strideXValue;
    out_ySize = gcmALIGN(padHeight + src_ySize, strideYValue)/strideYValue;
    out_zSize = strideXValue * strideYValue * src_zSize;

    shaderParam.globalWorkOffset[0] = 0;
    shaderParam.globalWorkOffset[1] = 0;
    shaderParam.globalWorkScale[0]  = 16;
    shaderParam.globalWorkScale[1]  = 1;
    shaderParam.localWorkSize[0]    = 8;
    shaderParam.localWorkSize[1]    = 1;
    shaderParam.globalWorkSize[0]   = (((imgWid + shaderParam.globalWorkScale[0] - 1) / shaderParam.globalWorkScale[0] + shaderParam.localWorkSize[0] - 1) / shaderParam.localWorkSize[0]) * shaderParam.localWorkSize[0];
    shaderParam.globalWorkSize[1]   = (((imgHei + shaderParam.globalWorkScale[1] - 1) / shaderParam.globalWorkScale[1] + shaderParam.localWorkSize[1] - 1) / shaderParam.localWorkSize[1]) * shaderParam.localWorkSize[1];
    {
        // uniforms
        vx_uint32 UniPackData2x8_even[16] = {
            0x11111111, // TCfg
            0x11110000, // ASelt
            0x06040200, 0x06040200, // ABin
            0x22222222, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001 // Constant
        };
        vx_uint32 UniPackData2x8_odd[16] = {
            0x11111111, // TCfg
            0x11110000, // ASelt
            0x07050301, 0x07050301, // ABin
            0x22222222, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001 // Constant
        };
        vx_uint32 src_inxSize[1] = {src_xSize};
        vx_uint32 src_inySize[1] = {src_ySize};
        vx_uint32 src_inzSize[1] = {src_zSize};
        vx_uint32 dst_xSize[1] = {out_xSize};
        vx_uint32 dst_ySize[1] = {out_ySize};
        vx_uint32 dst_zSize[1] = {out_zSize};
        vxSetNodeUniform(nodObj, "UniPackData2x8_even", 1, UniPackData2x8_even);
        vxSetNodeUniform(nodObj, "UniPackData2x8_odd", 1, UniPackData2x8_odd);
        vxSetNodeUniform(nodObj, "src_inxSize", 1, src_inxSize);
        vxSetNodeUniform(nodObj, "src_inySize", 1, src_inySize);
        vxSetNodeUniform(nodObj, "src_inzSize", 1, src_inzSize);
        vxSetNodeUniform(nodObj, "dst_xSize", 1, dst_xSize);
        vxSetNodeUniform(nodObj, "dst_ySize", 1, dst_ySize);
        vxSetNodeUniform(nodObj, "dst_zSize", 1, dst_zSize);
    }

    vxSetNodeAttribute(nodObj, VX_NODE_ATTRIBUTE_KERNEL_EXECUTION_PARAMETERS, &shaderParam, sizeof(vx_kernel_execution_parameters_t));
#endif
    return VX_SUCCESS;
}
VX_PRIVATE_API vx_status VX_CALLBACK vxfasterRcnnReshuffleDataFunction(vx_node node, const vx_reference parameters[], vx_uint32 paramCount)
{
    vx_status   status;

    vx_reference *ucParameters = (vx_reference*)parameters;

    /*step1: pre-process */
    vx_image_s  imgObj, imgObj2;
    vx_uint32 src_xSize = 0, src_ySize = 0, src_zSize = 0, out_xSize = 0, out_ySize = 0, out_zSize = 0;
    vx_uint32 levelIndex = 0, strideXValue = 0, strideYValue =  0;
    vx_uint32 padWidth = 0, padHeight = 0;

    vx_array input_array  = (vx_array)parameters[0];
    vx_scalar levelScalar = (vx_scalar)parameters[1];
    vx_scalar strideX     = (vx_scalar)parameters[2];
    vx_scalar strideY     = (vx_scalar)parameters[3];
    vx_array output_array = (vx_array)parameters[6];

    levelIndex       = levelScalar->value->u32;
    strideXValue     = strideX->value->u8;
    strideYValue     = strideY->value->u8;
    src_xSize = node->kernel->cnnAttributes.cnnkernelStreamInfo[levelIndex].orgInImageXSize;
    src_ySize = node->kernel->cnnAttributes.cnnkernelStreamInfo[levelIndex].orgInImageYSize;
    src_zSize = node->kernel->cnnAttributes.cnnkernelStreamInfo[levelIndex].orgInImageZSize;

    if (levelIndex == 0) /* pad is 3 */
    {
        padWidth = 3 + 3;
        padHeight = 3 + 3;
    }
    else if (levelIndex == 1) /* pad is 2 */
    {
        padWidth = 2 + 2;
        padHeight = 2 + 2;
    }

    out_xSize = gcmALIGN(padWidth + src_xSize, strideXValue)/strideXValue;
    out_ySize = gcmALIGN(padHeight + src_ySize, strideYValue)/strideYValue;
    out_zSize = strideXValue * strideYValue * src_zSize;
    memset(output_array->memory.logicals[0], 0, out_xSize * out_ySize * out_zSize * sizeof(vx_int16));
    {
        int itemSize = (int)input_array->itemSize;
        gcoOS_ZeroMemory(&imgObj, sizeof(vx_image_s));

        imgObj.base = input_array->base;
        imgObj.base.type = VX_TYPE_IMAGE;

        imgObj.width = src_xSize;
        imgObj.height = src_ySize * src_zSize;
        imgObj.format = (itemSize == 4)?VX_DF_IMAGE_F32:VX_DF_IMAGE_S16;
        imgObj.planeCount = 1;
        imgObj.memory = input_array->memory;
    }

    {
        int itemSize = (int)output_array->itemSize;
        gcoOS_ZeroMemory(&imgObj2, sizeof(vx_image_s));

        imgObj2.base = output_array->base;
        imgObj2.base.type = VX_TYPE_IMAGE;
        imgObj2.width = out_xSize;
        imgObj2.height = out_ySize * out_zSize;
        imgObj2.format = (itemSize == 4)?VX_DF_IMAGE_F32:VX_DF_IMAGE_S16;
        imgObj2.planeCount = 1;
        imgObj2.memory = output_array->memory;
    }


    ucParameters[0] = (vx_reference)(&imgObj);
    ucParameters[6] = (vx_reference)(&imgObj2);

    /*step2: call the base function */
    status = vxProgramKernel_Function(node, parameters, paramCount);

    /* step3 : post-process */
    ucParameters[0] = (vx_reference)(input_array);
    ucParameters[6] = (vx_reference)(output_array);

    return status;
}
VX_PRIVATE_API vx_status VX_CALLBACK vxfasterRcnnReshuffleDataDeinitializer(vx_node nodObj, const vx_reference *paraObj, vx_uint32 paraNum)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxInitializeTarget(
        vx_context context, vx_kernel_description_s *kernelDescTable[], vx_uint32 kernelCount)
{
    vx_status status = VX_SUCCESS;
    vx_uint32   index, i;

    vxmASSERT(context);
    vxmASSERT(kernelDescTable);
    vxmASSERT(kernelCount > 0);

    gcoOS_StrCopySafe(context->targetTable[0].name, VX_MAX_TARGET_NAME, VX_DEFAULT_TARGET_NAME);

    for (index = 0; index < kernelCount; index++)
    {
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

            status = vxBuildProgram(program, "-cl-viv-vx-extension");
            if(status < 0)
                printf("vxBuildProgram error:index = %d\n",index);
            kernel = vxAddKernelInProgramEx(
                                    program,
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
            status |= vxAddParameterToKernel(kernel, i, parameters->direction, parameters->dataType, parameters->state);
        }

        status |= vxFinalizeKernel(kernel);

        if (vxoKernel_IsUnique(&context->targetTable[0].kernelTable[context->kernelCount - 1])) context->uniqueKernelCount++;
    }

    /* ToDo : Add more specific return status check */
    if (status || gcoVX_Initialize(&context->evisNoInst) != gcvSTATUS_OK)
    {
        return VX_FAILURE;
    }

    return VX_SUCCESS;
}

vx_kernel_description_s *target_program_kernels[] = {
   &internal_nn_lrn,
#if VX_NN_USE_SHADER_ROIPOOL
    &internal_kernel_horipool,
    &internal_kernel_vertpool,
    &internal_kernel_roipool,
    &internalkernel_rectprocess,
    &internalkernel_resorting,
#endif
    //&internal_kernel_nms,
    &vxfasterRcnnReshuffleImageKernelInfo,
    &vxFasterRcnnInterleaveKernelInfo,
    &vxfasterRcnnReshuffleDataKernelInfo,
};

vx_uint32 num_target_program_kernels = vxmLENGTH_OF(target_program_kernels);

VX_API_ENTRY vx_status VX_API_CALL vxPublishKernels(vx_context context)
{
    vx_status status = vxInitializeTarget(context,
                                        target_program_kernels,
                                        num_target_program_kernels
                                        );

    return status;
}
