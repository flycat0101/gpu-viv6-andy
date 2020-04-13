
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#include "nnvxc_binary_interface.h"
#if gcdUSE_VXC_BINARY
#include "vxc_binaries.h"
#endif

void * GetVIPNNVXCBinaryPtr(nnvxc_kernel_enum type, unsigned int* len)
{
    switch(type)
    {
    
        case TensorTranspose:
            *len = VXC_BIN_TENSORTRANSPOSE_LEN;
            return vxcBinTensorTranspose;
    
        case Activation:
            *len = VXC_BIN_ACTIVATION_LEN;
            return vxcBinActivation;
    
        case Normalization:
            *len = VXC_BIN_NORMALIZATION_LEN;
            return vxcBinNormalization;
    
        case AvgPooling_UInt8:
            *len = VXC_BIN_AVGPOOLING_UINT8_LEN;
            return vxcBinAvgPooling_UInt8;
    
        case NormalizationUint8:
            *len = VXC_BIN_NORMALIZATIONUINT8_LEN;
            return vxcBinNormalizationUint8;
    
        case TensorPadRef:
            *len = VXC_BIN_TENSORPADREF_LEN;
            return vxcBinTensorPadRef;
    
        case LSTMUnitHiddenOut_Packed:
            *len = VXC_BIN_LSTMUNITHIDDENOUT_PACKED_LEN;
            return vxcBinLSTMUnitHiddenOut_Packed;
    
        case DepthwiseConv:
            *len = VXC_BIN_DEPTHWISECONV_LEN;
            return vxcBinDepthwiseConv;
    
        case AvgPooling:
            *len = VXC_BIN_AVGPOOLING_LEN;
            return vxcBinAvgPooling;
    
        case LayerNorm:
            *len = VXC_BIN_LAYERNORM_LEN;
            return vxcBinLayerNorm;
    
        case TensorStridedSlice:
            *len = VXC_BIN_TENSORSTRIDEDSLICE_LEN;
            return vxcBinTensorStridedSlice;
    
        case Batch2Space:
            *len = VXC_BIN_BATCH2SPACE_LEN;
            return vxcBinBatch2Space;
    
        case Reshuffle:
            *len = VXC_BIN_RESHUFFLE_LEN;
            return vxcBinReshuffle;
    
        case HorzMaxPool:
            *len = VXC_BIN_HORZMAXPOOL_LEN;
            return vxcBinHorzMaxPool;
    
        case RPNNms:
            *len = VXC_BIN_RPNNMS_LEN;
            return vxcBinRPNNms;
    
        case TensorAddMeanStddevNorm:
            *len = VXC_BIN_TENSORADDMEANSTDDEVNORM_LEN;
            return vxcBinTensorAddMeanStddevNorm;
    
        case ROIPool:
            *len = VXC_BIN_ROIPOOL_LEN;
            return vxcBinROIPool;
    
        case Space2Depth:
            *len = VXC_BIN_SPACE2DEPTH_LEN;
            return vxcBinSpace2Depth;
    
        case TensorScale:
            *len = VXC_BIN_TENSORSCALE_LEN;
            return vxcBinTensorScale;
    
        case TensorPad2:
            *len = VXC_BIN_TENSORPAD2_LEN;
            return vxcBinTensorPad2;
    
        case SoftmaxAxis2:
            *len = VXC_BIN_SOFTMAXAXIS2_LEN;
            return vxcBinSoftmaxAxis2;
    
        case Rnn:
            *len = VXC_BIN_RNN_LEN;
            return vxcBinRnn;
    
        case RPNSort:
            *len = VXC_BIN_RPNSORT_LEN;
            return vxcBinRPNSort;
    
        case TensorPad:
            *len = VXC_BIN_TENSORPAD_LEN;
            return vxcBinTensorPad;
    
        case Space2Batch:
            *len = VXC_BIN_SPACE2BATCH_LEN;
            return vxcBinSpace2Batch;
    
        case Gemm:
            *len = VXC_BIN_GEMM_LEN;
            return vxcBinGemm;
    
        case BatchNorm:
            *len = VXC_BIN_BATCHNORM_LEN;
            return vxcBinBatchNorm;
    
        case Gemm_noBias:
            *len = VXC_BIN_GEMM_NOBIAS_LEN;
            return vxcBinGemm_noBias;
    
        case Depth2Space:
            *len = VXC_BIN_DEPTH2SPACE_LEN;
            return vxcBinDepth2Space;
    
        case LSTMUnitHiddenOut:
            *len = VXC_BIN_LSTMUNITHIDDENOUT_LEN;
            return vxcBinLSTMUnitHiddenOut;
    
        case VertMaxPool:
            *len = VXC_BIN_VERTMAXPOOL_LEN;
            return vxcBinVertMaxPool;
    
        case LSTMUnitHiddenOutExt:
            *len = VXC_BIN_LSTMUNITHIDDENOUTEXT_LEN;
            return vxcBinLSTMUnitHiddenOutExt;
    
        case Tensor2Row:
            *len = VXC_BIN_TENSOR2ROW_LEN;
            return vxcBinTensor2Row;
    
        case HashLUT:
            *len = VXC_BIN_HASHLUT_LEN;
            return vxcBinHashLUT;
    
        case ShuffleChannel:
            *len = VXC_BIN_SHUFFLECHANNEL_LEN;
            return vxcBinShuffleChannel;
    
        case L2NormAxis2:
            *len = VXC_BIN_L2NORMAXIS2_LEN;
            return vxcBinL2NormAxis2;
    
        case PreTreatedRect:
            *len = VXC_BIN_PRETREATEDRECT_LEN;
            return vxcBinPreTreatedRect;
    
        case LSTMUnitStateOutExt:
            *len = VXC_BIN_LSTMUNITSTATEOUTEXT_LEN;
            return vxcBinLSTMUnitStateOutExt;
    
        case LSTMUnitProjection:
            *len = VXC_BIN_LSTMUNITPROJECTION_LEN;
            return vxcBinLSTMUnitProjection;
    
        case SoftmaxAxis1:
            *len = VXC_BIN_SOFTMAXAXIS1_LEN;
            return vxcBinSoftmaxAxis1;
    
        case Svdf:
            *len = VXC_BIN_SVDF_LEN;
            return vxcBinSvdf;
    
        case TensorCopy:
            *len = VXC_BIN_TENSORCOPY_LEN;
            return vxcBinTensorCopy;
    
        case Floor:
            *len = VXC_BIN_FLOOR_LEN;
            return vxcBinFloor;
    
        case Reorg:
            *len = VXC_BIN_REORG_LEN;
            return vxcBinReorg;
    
        case Tensor2DAdd:
            *len = VXC_BIN_TENSOR2DADD_LEN;
            return vxcBinTensor2DAdd;
    
        case LeakyRelu:
            *len = VXC_BIN_LEAKYRELU_LEN;
            return vxcBinLeakyRelu;
    
        case TensorDiv:
            *len = VXC_BIN_TENSORDIV_LEN;
            return vxcBinTensorDiv;
    
        case SoftmaxAxis0:
            *len = VXC_BIN_SOFTMAXAXIS0_LEN;
            return vxcBinSoftmaxAxis0;
    
        case TensorCrop:
            *len = VXC_BIN_TENSORCROP_LEN;
            return vxcBinTensorCrop;
    
        case LSTMUnit:
            *len = VXC_BIN_LSTMUNIT_LEN;
            return vxcBinLSTMUnit;
    
        case PRelu:
            *len = VXC_BIN_PRELU_LEN;
            return vxcBinPRelu;
    
        case EmbeddingLUT:
            *len = VXC_BIN_EMBEDDINGLUT_LEN;
            return vxcBinEmbeddingLUT;
    
        case AvgPooling_Int16:
            *len = VXC_BIN_AVGPOOLING_INT16_LEN;
            return vxcBinAvgPooling_Int16;
    
        case TensorPadSym:
            *len = VXC_BIN_TENSORPADSYM_LEN;
            return vxcBinTensorPadSym;
    
        case TensorMul:
            *len = VXC_BIN_TENSORMUL_LEN;
            return vxcBinTensorMul;
    
        case RPNRegression:
            *len = VXC_BIN_RPNREGRESSION_LEN;
            return vxcBinRPNRegression;
    
        case RPNRetrieve:
            *len = VXC_BIN_RPNRETRIEVE_LEN;
            return vxcBinRPNRetrieve;
    
        case LSTMUnitLayerNormStateOut:
            *len = VXC_BIN_LSTMUNITLAYERNORMSTATEOUT_LEN;
            return vxcBinLSTMUnitLayerNormStateOut;
    
        case Activation_UInt8:
            *len = VXC_BIN_ACTIVATION_UINT8_LEN;
            return vxcBinActivation_UInt8;
    
        case DeConvolution:
            *len = VXC_BIN_DECONVOLUTION_LEN;
            return vxcBinDeConvolution;
    
        case RPNSoftMax:
            *len = VXC_BIN_RPNSOFTMAX_LEN;
            return vxcBinRPNSoftMax;
    
        case TFAvgPooling:
            *len = VXC_BIN_TFAVGPOOLING_LEN;
            return vxcBinTFAvgPooling;
    
        case TensorAdd:
            *len = VXC_BIN_TENSORADD_LEN;
            return vxcBinTensorAdd;
    
        case FullyConnected:
            *len = VXC_BIN_FULLYCONNECTED_LEN;
            return vxcBinFullyConnected;
    
        case L2NormAxis0:
            *len = VXC_BIN_L2NORMAXIS0_LEN;
            return vxcBinL2NormAxis0;
    
        case TensorMulSatRTE:
            *len = VXC_BIN_TENSORMULSATRTE_LEN;
            return vxcBinTensorMulSatRTE;
    
        case TensorTR:
            *len = VXC_BIN_TENSORTR_LEN;
            return vxcBinTensorTR;
    
        case DepthwiseConvNoBias:
            *len = VXC_BIN_DEPTHWISECONVNOBIAS_LEN;
            return vxcBinDepthwiseConvNoBias;
    
        case MeanStddevNorm:
            *len = VXC_BIN_MEANSTDDEVNORM_LEN;
            return vxcBinMeanStddevNorm;
    
        case TensorAbs:
            *len = VXC_BIN_TENSORABS_LEN;
            return vxcBinTensorAbs;
    
        case L2Pooling:
            *len = VXC_BIN_L2POOLING_LEN;
            return vxcBinL2Pooling;
    
        case ROIRect2ROIList:
            *len = VXC_BIN_ROIRECT2ROILIST_LEN;
            return vxcBinROIRect2ROIList;
    
        case LSTMLayer:
            *len = VXC_BIN_LSTMLAYER_LEN;
            return vxcBinLSTMLayer;
    
        case ResizeNearestNeighbor:
            *len = VXC_BIN_RESIZENEARESTNEIGHBOR_LEN;
            return vxcBinResizeNearestNeighbor;
    
        case L2NormAxis1:
            *len = VXC_BIN_L2NORMAXIS1_LEN;
            return vxcBinL2NormAxis1;
    
        case TensorMeanAxis:
            *len = VXC_BIN_TENSORMEANAXIS_LEN;
            return vxcBinTensorMeanAxis;
    
        case MaxPooling:
            *len = VXC_BIN_MAXPOOLING_LEN;
            return vxcBinMaxPooling;
    
        case TensorConvFormat:
            *len = VXC_BIN_TENSORCONVFORMAT_LEN;
            return vxcBinTensorConvFormat;
    
        case Reverse:
            *len = VXC_BIN_REVERSE_LEN;
            return vxcBinReverse;
    
    default:
        printf("ERROR: Invalid nnvxc kernel binary type!\n");
    }
    *len = 0;
    return NULL;
}

#if defined(_WINDOWS) && !gcdSTATIC_LINK
#include <windows.h>

int WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved)
{
    hInst = hInst;
    reason = reason;
    reserved = reserved;

    switch (reason)
    {
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_DETACH:
        break;

    default:
        break;

    }
    return TRUE;
}
#endif
