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


#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "gc_hal_options.h"
#include"ovx12_vxc_binary_interface.h"
#if gcdUSE_VXC_BINARY
#include"ovx12_vxc_binaries.h"
#endif

void * GetOvx12KernelBinaryPtr(ovx12_vxc_kernel_enum type, unsigned int* len)
{
    switch(type)
    {

        case upsample_convert:
            *len = VXC_BIN_UPSAMPLE_CONVERT_LEN;
            return vxcBinupsample_convert;
    
        case makepoints:
            *len = VXC_BIN_MAKEPOINTS_LEN;
            return vxcBinmakepoints;
    
        case fillaccum:
            *len = VXC_BIN_FILLACCUM_LEN;
            return vxcBinfillaccum;
    
        case max:
            *len = VXC_BIN_MAX_LEN;
            return vxcBinmax;
    
        case remapCopy:
            *len = VXC_BIN_REMAPCOPY_LEN;
            return vxcBinremapCopy;
    
        case transpose_2d_tensor:
            *len = VXC_BIN_TRANSPOSE_2D_TENSOR_LEN;
            return vxcBintranspose_2d_tensor;
    
        case bilateral_filter:
            *len = VXC_BIN_BILATERAL_FILTER_LEN;
            return vxcBinbilateral_filter;
    
        case arrayCopy:
            *len = VXC_BIN_ARRAYCOPY_LEN;
            return vxcBinarrayCopy;
    
        case lbp:
            *len = VXC_BIN_LBP_LEN;
            return vxcBinlbp;
    
        case hog_cells:
            *len = VXC_BIN_HOG_CELLS_LEN;
            return vxcBinhog_cells;
    
        case copy:
            *len = VXC_BIN_COPY_LEN;
            return vxcBincopy;
    
        case lutCopy:
            *len = VXC_BIN_LUTCOPY_LEN;
            return vxcBinlutCopy;
    
        case min:
            *len = VXC_BIN_MIN_LEN;
            return vxcBinmin;
    
        case scalarCopy:
            *len = VXC_BIN_SCALARCOPY_LEN;
            return vxcBinscalarCopy;
    
        case scalar_operation:
            *len = VXC_BIN_SCALAR_OPERATION_LEN;
            return vxcBinscalar_operation;
    
        case upsample_padding:
            *len = VXC_BIN_UPSAMPLE_PADDING_LEN;
            return vxcBinupsample_padding;
    
        case tensor_convert_depth:
            *len = VXC_BIN_TENSOR_CONVERT_DEPTH_LEN;
            return vxcBintensor_convert_depth;
    
        case threshold:
            *len = VXC_BIN_THRESHOLD_LEN;
            return vxcBinthreshold;
    
        case getlines:
            *len = VXC_BIN_GETLINES_LEN;
            return vxcBingetlines;
    
        case match_template:
            *len = VXC_BIN_MATCH_TEMPLATE_LEN;
            return vxcBinmatch_template;
    
        case pyramid_copy_image:
            *len = VXC_BIN_PYRAMID_COPY_IMAGE_LEN;
            return vxcBinpyramid_copy_image;
    
        case non_max_suppression:
            *len = VXC_BIN_NON_MAX_SUPPRESSION_LEN;
            return vxcBinnon_max_suppression;
    
        case multiply_2d_matrixes:
            *len = VXC_BIN_MULTIPLY_2D_MATRIXES_LEN;
            return vxcBinmultiply_2d_matrixes;
    
        case hog_features:
            *len = VXC_BIN_HOG_FEATURES_LEN;
            return vxcBinhog_features;
    
        case tensorlut:
            *len = VXC_BIN_TENSORLUT_LEN;
            return vxcBintensorlut;
    
        case imageCopy:
            *len = VXC_BIN_IMAGECOPY_LEN;
            return vxcBinimageCopy;
        default:
        printf("ERROR: Invalid ovx1.2 vxc kernel binary type!\n");
    }
    *len = 0;
    return NULL;
}

#if defined(_WINDOWS)
#include <windows.h>

int WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved)
{
    hInst = hInst;
    reason = reason;
    reserved = reserved;

    switch (reason)
    {
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_DETACH:
        break;

    default:
        break;

    }
    return TRUE;
}
#endif