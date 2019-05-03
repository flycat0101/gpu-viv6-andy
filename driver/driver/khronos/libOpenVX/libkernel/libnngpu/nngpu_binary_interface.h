#ifndef __NN_GPU_BINARY_INTERFACE_H__
#define __NN_GPU_BINARY_INTERFACE_H__

typedef enum _nngpu_kernel_enum
{
    TensorTranspose,
    Batch2Space,
    TensorReduceDiv,
    L2Pooling,
    Activation,
    Space2Batch,
    Rnn,
    L2NormSumSqrt,
    Gemm_noBias,
    Floor,
    TensorStridedSlice,
    TensorElewise,
    TensorCopy,
    TensorPad,
    TensorMaxValue,
    Space2Depth,
    Tensor2Row,
    FullyConnected,
    Reverse,
    L2NormSumScale,
    Normalization,
    MaxPooling,
    LSTMUnitProjection,
    AvgPooling,
    Depth2Space,
    TensorMeanAxis0,
    Svdf,
    HashLUT,
    SoftMax,
    TensorCrop,
    Gemm,
    EmbeddingLUT,
    LSTMUnit,
    TensorScale,
    DepthwiseConv,
    NNGPU_KERNEL_NUM /*NNVXC_KERNEL_NUM should be the last item in the enum*/
}
nngpu_kernel_enum;

typedef void * (*GetBinaryPtr_FUNC)(nngpu_kernel_enum, unsigned int *);
#endif
