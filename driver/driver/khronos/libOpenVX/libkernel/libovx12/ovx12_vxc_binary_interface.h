#ifndef __OVX12_VXC_BINARY_INTERFACE_H__
#define __OVX12_VXC_BINARY_INTERFACE_H__

typedef enum _ovx12_vxc_kernel_enum
{
    transpose_2d_tensor,
    pyramid_copy_image,
    threshold,
    getlines,
    tensor_convert_depth,
    remapCopy,
    match_template,
    lbp,
    bilateral_filter,
    min,
    non_max_suppression,
    imageCopy,
    max,
    copy,
    multiply_2d_matrixes,
    arrayCopy,
    makepoints,
    scalarCopy,
    lutCopy,
    scalar_operation,
    tensorlut,
    fillaccum,
    upsample_padding,
    hog_cells,
    hog_features,
    upsample_convert,
    OVX12_VXC_KERNEL_NUM /*OVX12_VXC_KERNEL_NUM should be the last item in the enum*/
}
ovx12_vxc_kernel_enum;

typedef void * (*GetOvx12KernelBinaryPtr_FUNC)(ovx12_vxc_kernel_enum, unsigned int *);
#endif