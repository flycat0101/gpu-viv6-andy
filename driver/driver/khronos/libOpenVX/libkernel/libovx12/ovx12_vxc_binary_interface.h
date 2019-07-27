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


#ifndef __OVX12_VXC_BINARY_INTERFACE_H__
#define __OVX12_VXC_BINARY_INTERFACE_H__

typedef enum _ovx12_vxc_kernel_enum
{
    upsample_convert,
    makepoints,
    fillaccum,
    max,
    remapCopy,
    transpose_2d_tensor,
    bilateral_filter,
    arrayCopy,
    lbp,
    hog_cells,
    copy,
    lutCopy,
    min,
    scalarCopy,
    scalar_operation,
    upsample_padding,
    tensor_convert_depth,
    threshold,
    getlines,
    match_template,
    pyramid_copy_image,
    non_max_suppression,
    multiply_2d_matrixes,
    hog_features,
    tensorlut,
    imageCopy,
    OVX12_VXC_KERNEL_NUM /*OVX12_VXC_KERNEL_NUM should be the last item in the enum*/
}
ovx12_vxc_kernel_enum;

typedef void * (*GetOvx12KernelBinaryPtr_FUNC)(ovx12_vxc_kernel_enum, unsigned int *);
#endif