#ifndef __NN_GPU_BINARY_INTERFACE_H__
#define __NN_GPU_BINARY_INTERFACE_H__

typedef enum _nngpu_kernel_enum
{
    TensorPad,
    Normalization,
    Depth2Space,
    TensorCopy,
    TensorScale,
    L2NormSumSqrt,
    LSTMUnit,
    AvgPooling,
    L2Pooling,
    Activation,
    Svdf,
    Space2Batch,
    L2NormSumScale,
    HashLUT,
    TensorMeanAxis0,
    Floor,
    Reverse,
    Space2Depth,
    Batch2Space,
    TensorStridedSlice,
    FullyConnected,
    Gemm,
    Gemm_noBias,
    TensorReduceDiv,
    Conv2D_1x1,
    LSTMUnitProjection,
    EmbeddingLUT,
    TensorCopyROI,
    TensorTranspose,
    SoftMax,
    Tensor2Row,
    MaxPooling,
    TensorMaxValue,
    DepthwiseConv,
    TensorElewise,
    Rnn,
    TensorCrop,
    NNGPU_KERNEL_NUM /*NNVXC_KERNEL_NUM should be the last item in the enum*/
}
nngpu_kernel_enum;

typedef void * (*GetBinaryPtr_FUNC)(nngpu_kernel_enum, unsigned int *);
#endif
