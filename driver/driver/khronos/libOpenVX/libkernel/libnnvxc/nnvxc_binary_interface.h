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


#ifndef __NN_VXC_BINARY_INTERFACE_H__
#define __NN_VXC_BINARY_INTERFACE_H__

typedef enum _nnvxc_kernel_enum
{
    Depth2Space,
    Normalization,
    TensorMulSatRTE,
    TensorConvFormat,
    LSTMUnitHiddenOut_Packed,
    L2Pooling,
    Batch2Space,
    LeakyRelu,
    TensorCrop,
    LSTMUnitHiddenOut,
    TensorAdd,
    TensorTranspose,
    RPNSort,
    LSTMUnitProjection,
    LSTMUnitHiddenOutExt,
    AvgPooling,
    RPNNms,
    MeanStddevNorm,
    Softmax,
    TensorDiv,
    HorzMaxPool,
    TensorAbs,
    EmbeddingLUT,
    HashLUT,
    ActivationSoftRelu,
    PRelu,
    ResizeNearestNeighbor,
    Tensor2Row,
    FullyConnected,
    TensorScale,
    Reverse,
    TensorCopy,
    L2NormSumSqrt,
    MaxPooling,
    BatchNorm,
    DepthwiseConv,
    LSTMLayer,
    TensorPad,
    Activation_UInt8,
    Space2Depth,
    RPNRegression,
    ROIPool,
    L2NormSumScale,
    RPNSoftMax,
    TFAvgPooling,
    AvgPooling_UInt8,
    RPNRetrieve,
    Svdf,
    TensorStridedSlice,
    Reorg,
    Activation,
    TensorTR,
    LSTMUnit,
    NormalizationUint8,
    Gemm,
    Reshuffle,
    PreTreatedRect,
    TensorMul,
    ROIRect2ROIList,
    LayerNorm,
    AvgPooling_Int16,
    Rnn,
    LSTMUnitLayerNormStateOut,
    DeConvolution,
    TensorAddMeanStddevNorm,
    Tensor2DAdd,
    LSTMUnitStateOutExt,
    Gemm_noBias,
    Floor,
    TensorMeanAxis,
    Space2Batch,
    VertMaxPool,
    NNVXC_KERNEL_NUM /*NNVXC_KERNEL_NUM should be the last item in the enum*/
}
nnvxc_kernel_enum;

typedef void * (*GetBinaryPtr_FUNC)(nnvxc_kernel_enum, unsigned int *);
#endif
