#ifndef __OVX12_VXC_BINARY_INTERFACE_H__
#define __OVX12_VXC_BINARY_INTERFACE_H__

typedef enum _ovx12_vxc_kernel_enum
{
    match_template,
    tensor_convert_depth,
    copy,
    non_max_suppression,
    transpose_2d_tensor,
    tensorlut,
    hog_features,
    bilateral_filter,
    scalarCopy,
    lutCopy,
    fillaccum,
    getlines,
    pyramid_copy_image,
    max,
    upsample_convert,
    min,
    lbp,
    arrayCopy,
    scalar_operation,
    imageCopy,
    hog_cells,
    multiply_2d_matrixes,
    threshold,
    upsample_padding,
    makepoints,
    remapCopy,
    OVX12_VXC_KERNEL_NUM /*OVX12_VXC_KERNEL_NUM should be the last item in the enum*/
}
ovx12_vxc_kernel_enum;

typedef void * (*GetOvx12KernelBinaryPtr_FUNC)(ovx12_vxc_kernel_enum, unsigned int *);
#endif