#ifndef __OVX12_VXC_BINARY_INTERFACE_H__
#define __OVX12_VXC_BINARY_INTERFACE_H__

typedef enum _ovx12_vxc_kernel_enum
{
    makepoints,
    bilateral_filter,
    hog_features,
    hog_cells,
    multiply_2d_matrixes,
    remapCopy,
    tensor_convert_depth,
    getlines,
    max,
    imageCopy,
    match_template,
    threshold,
    upsample_convert,
    min,
    arrayCopy,
    copy,
    upsample_padding,
    non_max_suppression,
    tensorlut,
    scalar_operation,
    scalarCopy,
    pyramid_copy_image,
    lbp,
    transpose_2d_tensor,
    lutCopy,
    fillaccum,
    OVX12_VXC_KERNEL_NUM /*OVX12_VXC_KERNEL_NUM should be the last item in the enum*/
}
ovx12_vxc_kernel_enum;

typedef void * (*GetOvx12KernelBinaryPtr_FUNC)(ovx12_vxc_kernel_enum, unsigned int *);
#endif