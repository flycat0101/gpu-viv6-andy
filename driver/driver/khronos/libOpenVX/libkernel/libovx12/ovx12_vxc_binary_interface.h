
#ifndef __OVX12_VXC_BINARY_INTERFACE_H__
#define __OVX12_VXC_BINARY_INTERFACE_H__

typedef enum _ovx12_vxc_kernel_enum
{
    copy,
    makepoints,
    scalar_operation,
    hog_cells,
    threshold,
    upsample_padding,
    arrayCopy,
    bilateral_filter,
    lutCopy,
    non_max_suppression,
    scalarCopy,
    transpose_2d_tensor,
    hog_features,
    max,
    remapCopy,
    multiply_2d_matrixes,
    upsample_convert,
    fillaccum,
    min,
    tensor_convert_depth,
    getlines,
    pyramid_copy_image,
    imageCopy,
    lbp,
    match_template,
    tensorlut,
    OVX12_VXC_KERNEL_NUM /*OVX12_VXC_KERNEL_NUM should be the last item in the enum*/
}
ovx12_vxc_kernel_enum;

#if gcdSTATIC_LINK
void * GetOvx12KernelBinaryPtr(ovx12_vxc_kernel_enum, unsigned int *);
#else
typedef void * (*GetOvx12KernelBinaryPtr_FUNC)(ovx12_vxc_kernel_enum, unsigned int *);
#endif
#endif
