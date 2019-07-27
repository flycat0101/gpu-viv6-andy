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


#ifndef __NN_GPU_BINARY_INTERFACE_H__
#define __NN_GPU_BINARY_INTERFACE_H__

typedef enum _nngpu_kernel_enum
{
    Depth2Space,
    Normalization,
    L2Pooling,
    Batch2Space,
    TensorCrop,
    SoftMax,
    TensorTranspose,
    LSTMUnitProjection,
    AvgPooling,
    TensorMaxValue,
    TensorElewise,
    EmbeddingLUT,
    HashLUT,
    Tensor2Row,
    FullyConnected,
    TensorScale,
    Reverse,
    TensorCopy,
    L2NormSumSqrt,
    TensorMeanAxis0,
    MaxPooling,
    DepthwiseConv,
    TensorPad,
    Space2Depth,
    L2NormSumScale,
    TensorReduceDiv,
    Svdf,
    TensorStridedSlice,
    Activation,
    LSTMUnit,
    Gemm,
    Rnn,
    Gemm_noBias,
    Floor,
    Space2Batch,
    NNGPU_KERNEL_NUM /*NNVXC_KERNEL_NUM should be the last item in the enum*/
}
nngpu_kernel_enum;

typedef void * (*GetBinaryPtr_FUNC)(nngpu_kernel_enum, unsigned int *);
#endif
