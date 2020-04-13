
#ifndef __NNVXC_BINARY_INTERFACE_H__
#define __NNVXC_BINARY_INTERFACE_H__

typedef enum _nnvxc_kernel_enum
{
    TensorTranspose,
    Activation,
    Normalization,
    AvgPooling_UInt8,
    NormalizationUint8,
    TensorPadRef,
    LSTMUnitHiddenOut_Packed,
    DepthwiseConv,
    AvgPooling,
    LayerNorm,
    TensorStridedSlice,
    Batch2Space,
    Reshuffle,
    HorzMaxPool,
    RPNNms,
    TensorAddMeanStddevNorm,
    ROIPool,
    Space2Depth,
    TensorScale,
    TensorPad2,
    SoftmaxAxis2,
    Rnn,
    RPNSort,
    TensorPad,
    Space2Batch,
    Gemm,
    BatchNorm,
    Gemm_noBias,
    Depth2Space,
    LSTMUnitHiddenOut,
    VertMaxPool,
    LSTMUnitHiddenOutExt,
    Tensor2Row,
    HashLUT,
    ShuffleChannel,
    L2NormAxis2,
    PreTreatedRect,
    LSTMUnitStateOutExt,
    LSTMUnitProjection,
    SoftmaxAxis1,
    Svdf,
    TensorCopy,
    Floor,
    Reorg,
    Tensor2DAdd,
    LeakyRelu,
    TensorDiv,
    SoftmaxAxis0,
    TensorCrop,
    LSTMUnit,
    PRelu,
    EmbeddingLUT,
    AvgPooling_Int16,
    TensorPadSym,
    TensorMul,
    RPNRegression,
    RPNRetrieve,
    LSTMUnitLayerNormStateOut,
    Activation_UInt8,
    DeConvolution,
    RPNSoftMax,
    TFAvgPooling,
    TensorAdd,
    FullyConnected,
    L2NormAxis0,
    TensorMulSatRTE,
    TensorTR,
    DepthwiseConvNoBias,
    MeanStddevNorm,
    TensorAbs,
    L2Pooling,
    ROIRect2ROIList,
    LSTMLayer,
    ResizeNearestNeighbor,
    L2NormAxis1,
    TensorMeanAxis,
    MaxPooling,
    TensorConvFormat,
    Reverse,
    NNVXC_KERNEL_NUM /*NNVXC_KERNEL_NUM should be the last item in the enum*/
}
nnvxc_kernel_enum;

#if gcdSTATIC_LINK
void * GetVIPNNVXCBinaryPtr(nnvxc_kernel_enum, unsigned int *);
#else
typedef void * (*GetVIPNNVXCBinaryPtr_FUNC)(nnvxc_kernel_enum, unsigned int *);
#endif
#endif
