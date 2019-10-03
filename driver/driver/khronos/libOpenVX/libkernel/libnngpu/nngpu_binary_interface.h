#ifndef __NN_GPU_BINARY_INTERFACE_H__
#define __NN_GPU_BINARY_INTERFACE_H__

typedef enum _nngpu_kernel_enum
{
    TensorPad,
    TensorStridedSlice,
    EmbeddingLUT,
    Tensor2Row,
    MaxPooling,
    TensorTranspose,
    Space2Batch,
    Space2Depth,
    DepthwiseConv,
    L2NormSumSqrt,
    Depth2Space,
    TensorScale,
    Activation,
    TensorElewise,
    Reverse,
    Conv2D_1x1,
    TensorReduceDiv,
    TensorMaxValue,
    L2NormSumScale,
    LSTMUnit,
    Svdf,
    TensorMeanAxis0,
    TensorCrop,
    Gemm_noBias,
    Rnn,
    AvgPooling,
    SoftMax,
    LSTMUnitProjection,
    TensorCopyROI,
    TensorCopy,
    L2Pooling,
    FullyConnected,
    HashLUT,
    Normalization,
    Gemm,
    Batch2Space,
    Floor,
    NNGPU_KERNEL_NUM /*NNVXC_KERNEL_NUM should be the last item in the enum*/
}
nngpu_kernel_enum;

typedef void * (*GetBinaryPtr_FUNC)(nngpu_kernel_enum, unsigned int *);
#endif
