
#ifndef __OVX12_VXC_BINARY_INTERFACE_H__
#define __OVX12_VXC_BINARY_INTERFACE_H__

typedef enum _ovx12_vxc_kernel_enum
{
    multiply_2d_matrixes,
    pyramid_copy_image,
    scharr3x3,
    hog_cells,
    match_template,
    gaussian_3x3,
    min,
    hog_features,
    transpose_2d_tensor,
    copy,
    arrayCopy,
    dilate_3x3,
    warp_affine,
    lutCopy,
    remapCopy,
    tensorlut,
    max,
    bilateral_filter,
    scalarCopy,
    fillaccum,
    non_max_suppression,
    threshold,
    upsample_padding,
    scalar_operation,
    lbp,
    image_crop,
    imageCopy,
    tensor_convert_depth,
    makepoints,
    erode_3x3,
    convolve5x5,
    getlines,
    weighted_average,
    upsample_convert,
    OVX12_VXC_KERNEL_NUM /*OVX12_VXC_KERNEL_NUM should be the last item in the enum*/
}
ovx12_vxc_kernel_enum;

#if gcdSTATIC_LINK
void * GetOvx12KernelBinaryPtr(ovx12_vxc_kernel_enum, unsigned int *);
#else
typedef void * (*GetOvx12KernelBinaryPtr_FUNC)(ovx12_vxc_kernel_enum, unsigned int *);
#endif
#endif
