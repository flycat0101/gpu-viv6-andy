
#ifndef __NNGPU_BINARY_INTERFACE_H__
#define __NNGPU_BINARY_INTERFACE_H__

typedef enum _nngpu_kernel_enum
{
    TensorScale,
    TensorLinear,
    L2NormAxis2,
    TensorTranspose,
    AvgPooling,
    Tensor2Row,
    Space2Depth,
    BatchNorm,
    Batch2Space,
    TensorMeanAxis,
    LSTMUnit,
    ShuffleChannel,
    TensorReduceDiv,
    Floor,
    LeakyRelu,
    NormalizationAxis0,
    Depth2Space,
    TensorCrop,
    Gemm_noBias,
    Space2Batch,
    TensorExpand,
    TensorCopyROI,
    Reorg,
    SoftMax,
    MaxPooling,
    ResizeNearestNeighbor,
    DepthwiseConv,
    TensorEltwise,
    Svdf,
    Conv2D_1x1,
    TensorStridedSlice,
    Reverse,
    Normalization,
    EmbeddingLUT,
    TensorMaxValue,
    Gemm,
    TensorPad,
    LSTMUnitProjection,
    TensorCopy,
    HashLUT,
    L2NormAxis0,
    Rnn,
    Swish,
    L2NormAxis1,
    L2Pooling,
    HSwish,
    Prelu,
    ROIPool,
    FullyConnected,
    NormalizationAxis1,
    TensorPad2,
    Activation,
    TensorTR,
    NNGPU_KERNEL_NUM /*NNGPU_KERNEL_NUM should be the last item in the enum*/
}
nngpu_kernel_enum;

#if gcdSTATIC_LINK
void * GetGPUNNVXCBinaryPtr(nngpu_kernel_enum, unsigned int *);
#else
typedef void * (*GetGPUNNVXCBinaryPtr_FUNC)(nngpu_kernel_enum, unsigned int *);
#endif
#endif
