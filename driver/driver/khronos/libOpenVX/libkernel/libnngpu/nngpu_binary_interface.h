
#ifndef __NNGPU_BINARY_INTERFACE_H__
#define __NNGPU_BINARY_INTERFACE_H__

typedef enum _nngpu_kernel_enum
{
    TensorTranspose,
    Activation,
    Normalization,
    DepthwiseConv,
    AvgPooling,
    TensorStridedSlice,
    Batch2Space,
    Conv2D_1x1,
    TensorCopyROI,
    ROIPool,
    Space2Depth,
    TensorScale,
    TensorPad2,
    Rnn,
    TensorPad,
    Space2Batch,
    Gemm,
    BatchNorm,
    Gemm_noBias,
    Depth2Space,
    Tensor2Row,
    HashLUT,
    ShuffleChannel,
    L2NormAxis2,
    TensorMaxValue,
    LSTMUnitProjection,
    Svdf,
    TensorCopy,
    Floor,
    Reorg,
    LeakyRelu,
    TensorCrop,
    TensorEltwise,
    LSTMUnit,
    TensorReduceDiv,
    EmbeddingLUT,
    SoftMax,
    FullyConnected,
    L2NormAxis0,
    TensorTR,
    Prelu,
    L2Pooling,
    ResizeNearestNeighbor,
    L2NormAxis1,
    TensorMeanAxis,
    MaxPooling,
    Reverse,
    NNGPU_KERNEL_NUM /*NNGPU_KERNEL_NUM should be the last item in the enum*/
}
nngpu_kernel_enum;

#if gcdSTATIC_LINK
void * GetGPUNNVXCBinaryPtr(nngpu_kernel_enum, unsigned int *);
#else
typedef void * (*GetGPUNNVXCBinaryPtr_FUNC)(nngpu_kernel_enum, unsigned int *);
#endif
#endif
