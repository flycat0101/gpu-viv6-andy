
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "gc_hal_options.h"
#include "ovx12_vxc_binary_interface.h"
#if gcdUSE_VXC_BINARY
#include "ovx12_vxc_binaries.h"
#endif

void * GetOvx12KernelBinaryPtr(ovx12_vxc_kernel_enum type, unsigned int* len)
{
    switch(type)
    {
    
        case multiply_2d_matrixes:
            *len = VXC_BIN_MULTIPLY_2D_MATRIXES_LEN;
            return vxcBinmultiply_2d_matrixes;
    
        case pyramid_copy_image:
            *len = VXC_BIN_PYRAMID_COPY_IMAGE_LEN;
            return vxcBinpyramid_copy_image;
    
        case scharr3x3:
            *len = VXC_BIN_SCHARR3X3_LEN;
            return vxcBinscharr3x3;
    
        case hog_cells:
            *len = VXC_BIN_HOG_CELLS_LEN;
            return vxcBinhog_cells;
    
        case match_template:
            *len = VXC_BIN_MATCH_TEMPLATE_LEN;
            return vxcBinmatch_template;
    
        case gaussian_3x3:
            *len = VXC_BIN_GAUSSIAN_3X3_LEN;
            return vxcBingaussian_3x3;
    
        case min:
            *len = VXC_BIN_MIN_LEN;
            return vxcBinmin;
    
        case hog_features:
            *len = VXC_BIN_HOG_FEATURES_LEN;
            return vxcBinhog_features;
    
        case transpose_2d_tensor:
            *len = VXC_BIN_TRANSPOSE_2D_TENSOR_LEN;
            return vxcBintranspose_2d_tensor;
    
        case copy:
            *len = VXC_BIN_COPY_LEN;
            return vxcBincopy;
    
        case arrayCopy:
            *len = VXC_BIN_ARRAYCOPY_LEN;
            return vxcBinarrayCopy;
    
        case dilate_3x3:
            *len = VXC_BIN_DILATE_3X3_LEN;
            return vxcBindilate_3x3;
    
        case warp_affine:
            *len = VXC_BIN_WARP_AFFINE_LEN;
            return vxcBinwarp_affine;
    
        case lutCopy:
            *len = VXC_BIN_LUTCOPY_LEN;
            return vxcBinlutCopy;
    
        case remapCopy:
            *len = VXC_BIN_REMAPCOPY_LEN;
            return vxcBinremapCopy;
    
        case tensorlut:
            *len = VXC_BIN_TENSORLUT_LEN;
            return vxcBintensorlut;
    
        case max:
            *len = VXC_BIN_MAX_LEN;
            return vxcBinmax;
    
        case bilateral_filter:
            *len = VXC_BIN_BILATERAL_FILTER_LEN;
            return vxcBinbilateral_filter;
    
        case scalarCopy:
            *len = VXC_BIN_SCALARCOPY_LEN;
            return vxcBinscalarCopy;
    
        case fillaccum:
            *len = VXC_BIN_FILLACCUM_LEN;
            return vxcBinfillaccum;
    
        case non_max_suppression:
            *len = VXC_BIN_NON_MAX_SUPPRESSION_LEN;
            return vxcBinnon_max_suppression;
    
        case threshold:
            *len = VXC_BIN_THRESHOLD_LEN;
            return vxcBinthreshold;
    
        case upsample_padding:
            *len = VXC_BIN_UPSAMPLE_PADDING_LEN;
            return vxcBinupsample_padding;
    
        case scalar_operation:
            *len = VXC_BIN_SCALAR_OPERATION_LEN;
            return vxcBinscalar_operation;
    
        case lbp:
            *len = VXC_BIN_LBP_LEN;
            return vxcBinlbp;
    
        case image_crop:
            *len = VXC_BIN_IMAGE_CROP_LEN;
            return vxcBinimage_crop;
    
        case imageCopy:
            *len = VXC_BIN_IMAGECOPY_LEN;
            return vxcBinimageCopy;
    
        case tensor_convert_depth:
            *len = VXC_BIN_TENSOR_CONVERT_DEPTH_LEN;
            return vxcBintensor_convert_depth;
    
        case makepoints:
            *len = VXC_BIN_MAKEPOINTS_LEN;
            return vxcBinmakepoints;
    
        case erode_3x3:
            *len = VXC_BIN_ERODE_3X3_LEN;
            return vxcBinerode_3x3;
    
        case convolve5x5:
            *len = VXC_BIN_CONVOLVE5X5_LEN;
            return vxcBinconvolve5x5;
    
        case getlines:
            *len = VXC_BIN_GETLINES_LEN;
            return vxcBingetlines;
    
        case weighted_average:
            *len = VXC_BIN_WEIGHTED_AVERAGE_LEN;
            return vxcBinweighted_average;
    
        case upsample_convert:
            *len = VXC_BIN_UPSAMPLE_CONVERT_LEN;
            return vxcBinupsample_convert;
    
    default:
        printf("ERROR: Invalid ovx12_vxc kernel binary type!\n");
    }
    *len = 0;
    return NULL;
}

#if defined(_WINDOWS) && !gcdSTATIC_LINK
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
