
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
    
        case TensorScale:
            *len = VXC_BIN_TENSORSCALE_LEN;
            return vxcBinTensorScale;
    
        case TensorLinear:
            *len = VXC_BIN_TENSORLINEAR_LEN;
            return vxcBinTensorLinear;
    
        case L2NormAxis2:
            *len = VXC_BIN_L2NORMAXIS2_LEN;
            return vxcBinL2NormAxis2;
    
        case TensorTranspose:
            *len = VXC_BIN_TENSORTRANSPOSE_LEN;
            return vxcBinTensorTranspose;
    
        case AvgPooling:
            *len = VXC_BIN_AVGPOOLING_LEN;
            return vxcBinAvgPooling;
    
        case Tensor2Row:
            *len = VXC_BIN_TENSOR2ROW_LEN;
            return vxcBinTensor2Row;
    
        case RPNNms:
            *len = VXC_BIN_RPNNMS_LEN;
            return vxcBinRPNNms;
    
        case Space2Depth:
            *len = VXC_BIN_SPACE2DEPTH_LEN;
            return vxcBinSpace2Depth;
    
        case BatchNorm:
            *len = VXC_BIN_BATCHNORM_LEN;
            return vxcBinBatchNorm;
    
        case Tensor2DAdd:
            *len = VXC_BIN_TENSOR2DADD_LEN;
            return vxcBinTensor2DAdd;
    
        case Batch2Space:
            *len = VXC_BIN_BATCH2SPACE_LEN;
            return vxcBinBatch2Space;
    
        case LayerNorm:
            *len = VXC_BIN_LAYERNORM_LEN;
            return vxcBinLayerNorm;
    
        case TensorConvFormat:
            *len = VXC_BIN_TENSORCONVFORMAT_LEN;
            return vxcBinTensorConvFormat;
    
        case TensorMeanAxis:
            *len = VXC_BIN_TENSORMEANAXIS_LEN;
            return vxcBinTensorMeanAxis;
    
        case AvgPooling_Int16:
            *len = VXC_BIN_AVGPOOLING_INT16_LEN;
            return vxcBinAvgPooling_Int16;
    
        case LSTMUnit:
            *len = VXC_BIN_LSTMUNIT_LEN;
            return vxcBinLSTMUnit;
    
        case LSTMUnitHiddenOut:
            *len = VXC_BIN_LSTMUNITHIDDENOUT_LEN;
            return vxcBinLSTMUnitHiddenOut;
    
        case ShuffleChannel:
            *len = VXC_BIN_SHUFFLECHANNEL_LEN;
            return vxcBinShuffleChannel;
    
        case LSTMUnitStateOutExt:
            *len = VXC_BIN_LSTMUNITSTATEOUTEXT_LEN;
            return vxcBinLSTMUnitStateOutExt;
    
        case SoftmaxAxis2:
            *len = VXC_BIN_SOFTMAXAXIS2_LEN;
            return vxcBinSoftmaxAxis2;
    
        case PRelu:
            *len = VXC_BIN_PRELU_LEN;
            return vxcBinPRelu;
    
        case AvgPooling_UInt8:
            *len = VXC_BIN_AVGPOOLING_UINT8_LEN;
            return vxcBinAvgPooling_UInt8;
    
        case ROIRect2ROIList:
            *len = VXC_BIN_ROIRECT2ROILIST_LEN;
            return vxcBinROIRect2ROIList;
    
        case HorzMaxPool:
            *len = VXC_BIN_HORZMAXPOOL_LEN;
            return vxcBinHorzMaxPool;
    
        case Floor:
            *len = VXC_BIN_FLOOR_LEN;
            return vxcBinFloor;
    
        case LeakyRelu:
            *len = VXC_BIN_LEAKYRELU_LEN;
            return vxcBinLeakyRelu;
    
        case MeanStddevNorm:
            *len = VXC_BIN_MEANSTDDEVNORM_LEN;
            return vxcBinMeanStddevNorm;
    
        case FC_TPCheck:
            *len = VXC_BIN_FC_TPCHECK_LEN;
            return vxcBinFC_TPCheck;
    
        case TensorAdd:
            *len = VXC_BIN_TENSORADD_LEN;
            return vxcBinTensorAdd;
    
        case NormalizationAxis0:
            *len = VXC_BIN_NORMALIZATIONAXIS0_LEN;
            return vxcBinNormalizationAxis0;
    
        case Depth2Space:
            *len = VXC_BIN_DEPTH2SPACE_LEN;
            return vxcBinDepth2Space;
    
        case TensorCrop:
            *len = VXC_BIN_TENSORCROP_LEN;
            return vxcBinTensorCrop;
    
        case LSTMUnitHiddenOut_Packed:
            *len = VXC_BIN_LSTMUNITHIDDENOUT_PACKED_LEN;
            return vxcBinLSTMUnitHiddenOut_Packed;
    
        case Gemm_noBias:
            *len = VXC_BIN_GEMM_NOBIAS_LEN;
            return vxcBinGemm_noBias;
    
        case Space2Batch:
            *len = VXC_BIN_SPACE2BATCH_LEN;
            return vxcBinSpace2Batch;
    
        case PreTreatedRect:
            *len = VXC_BIN_PRETREATEDRECT_LEN;
            return vxcBinPreTreatedRect;
    
        case TensorExpand:
            *len = VXC_BIN_TENSOREXPAND_LEN;
            return vxcBinTensorExpand;
    
        case DepthwiseConvNoBias:
            *len = VXC_BIN_DEPTHWISECONVNOBIAS_LEN;
            return vxcBinDepthwiseConvNoBias;
    
        case RPNRetrieve:
            *len = VXC_BIN_RPNRETRIEVE_LEN;
            return vxcBinRPNRetrieve;
    
        case Reorg:
            *len = VXC_BIN_REORG_LEN;
            return vxcBinReorg;
    
        case MaxPooling:
            *len = VXC_BIN_MAXPOOLING_LEN;
            return vxcBinMaxPooling;
    
        case ResizeNearestNeighbor:
            *len = VXC_BIN_RESIZENEARESTNEIGHBOR_LEN;
            return vxcBinResizeNearestNeighbor;
    
        case LSTMUnitHiddenOutExt:
            *len = VXC_BIN_LSTMUNITHIDDENOUTEXT_LEN;
            return vxcBinLSTMUnitHiddenOutExt;
    
        case TFAvgPooling:
            *len = VXC_BIN_TFAVGPOOLING_LEN;
            return vxcBinTFAvgPooling;
    
        case LSTMUnitLayerNormStateOut:
            *len = VXC_BIN_LSTMUNITLAYERNORMSTATEOUT_LEN;
            return vxcBinLSTMUnitLayerNormStateOut;
    
        case DepthwiseConv:
            *len = VXC_BIN_DEPTHWISECONV_LEN;
            return vxcBinDepthwiseConv;
    
        case RPNRegression:
            *len = VXC_BIN_RPNREGRESSION_LEN;
            return vxcBinRPNRegression;
    
        case TensorAddMeanStddevNorm:
            *len = VXC_BIN_TENSORADDMEANSTDDEVNORM_LEN;
            return vxcBinTensorAddMeanStddevNorm;
    
        case Svdf:
            *len = VXC_BIN_SVDF_LEN;
            return vxcBinSvdf;
    
        case TensorStridedSlice:
            *len = VXC_BIN_TENSORSTRIDEDSLICE_LEN;
            return vxcBinTensorStridedSlice;
    
        case Reverse:
            *len = VXC_BIN_REVERSE_LEN;
            return vxcBinReverse;
    
        case Normalization:
            *len = VXC_BIN_NORMALIZATION_LEN;
            return vxcBinNormalization;
    
        case EmbeddingLUT:
            *len = VXC_BIN_EMBEDDINGLUT_LEN;
            return vxcBinEmbeddingLUT;
    
        case DeConvolution:
            *len = VXC_BIN_DECONVOLUTION_LEN;
            return vxcBinDeConvolution;
    
        case VertMaxPool:
            *len = VXC_BIN_VERTMAXPOOL_LEN;
            return vxcBinVertMaxPool;
    
        case Gemm:
            *len = VXC_BIN_GEMM_LEN;
            return vxcBinGemm;
    
        case Reshuffle:
            *len = VXC_BIN_RESHUFFLE_LEN;
            return vxcBinReshuffle;
    
        case TensorPad:
            *len = VXC_BIN_TENSORPAD_LEN;
            return vxcBinTensorPad;
    
        case TensorDiv:
            *len = VXC_BIN_TENSORDIV_LEN;
            return vxcBinTensorDiv;
    
        case LSTMUnitProjection:
            *len = VXC_BIN_LSTMUNITPROJECTION_LEN;
            return vxcBinLSTMUnitProjection;
    
        case TensorCopy:
            *len = VXC_BIN_TENSORCOPY_LEN;
            return vxcBinTensorCopy;
    
        case HashLUT:
            *len = VXC_BIN_HASHLUT_LEN;
            return vxcBinHashLUT;
    
        case L2NormAxis0:
            *len = VXC_BIN_L2NORMAXIS0_LEN;
            return vxcBinL2NormAxis0;
    
        case NormalizationUint8:
            *len = VXC_BIN_NORMALIZATIONUINT8_LEN;
            return vxcBinNormalizationUint8;
    
        case TensorPadSym:
            *len = VXC_BIN_TENSORPADSYM_LEN;
            return vxcBinTensorPadSym;
    
        case TensorPadRef:
            *len = VXC_BIN_TENSORPADREF_LEN;
            return vxcBinTensorPadRef;
    
        case Rnn:
            *len = VXC_BIN_RNN_LEN;
            return vxcBinRnn;
    
        case Swish:
            *len = VXC_BIN_SWISH_LEN;
            return vxcBinSwish;
    
        case L2NormAxis1:
            *len = VXC_BIN_L2NORMAXIS1_LEN;
            return vxcBinL2NormAxis1;
    
        case TensorAbs:
            *len = VXC_BIN_TENSORABS_LEN;
            return vxcBinTensorAbs;
    
        case L2Pooling:
            *len = VXC_BIN_L2POOLING_LEN;
            return vxcBinL2Pooling;
    
        case HSwish:
            *len = VXC_BIN_HSWISH_LEN;
            return vxcBinHSwish;
    
        case RPNSoftMax:
            *len = VXC_BIN_RPNSOFTMAX_LEN;
            return vxcBinRPNSoftMax;
    
        case ROIPool:
            *len = VXC_BIN_ROIPOOL_LEN;
            return vxcBinROIPool;
    
        case SoftmaxAxis1:
            *len = VXC_BIN_SOFTMAXAXIS1_LEN;
            return vxcBinSoftmaxAxis1;
    
        case FullyConnected:
            *len = VXC_BIN_FULLYCONNECTED_LEN;
            return vxcBinFullyConnected;
    
        case TensorMul:
            *len = VXC_BIN_TENSORMUL_LEN;
            return vxcBinTensorMul;
    
        case TensorMulSatRTE:
            *len = VXC_BIN_TENSORMULSATRTE_LEN;
            return vxcBinTensorMulSatRTE;
    
        case RPNSort:
            *len = VXC_BIN_RPNSORT_LEN;
            return vxcBinRPNSort;
    
        case SoftmaxAxis0:
            *len = VXC_BIN_SOFTMAXAXIS0_LEN;
            return vxcBinSoftmaxAxis0;
    
        case LSTMLayer:
            *len = VXC_BIN_LSTMLAYER_LEN;
            return vxcBinLSTMLayer;
    
        case NormalizationAxis1:
            *len = VXC_BIN_NORMALIZATIONAXIS1_LEN;
            return vxcBinNormalizationAxis1;
    
        case TensorPad2:
            *len = VXC_BIN_TENSORPAD2_LEN;
            return vxcBinTensorPad2;
    
        case Activation:
            *len = VXC_BIN_ACTIVATION_LEN;
            return vxcBinActivation;
    
        case TensorTR:
            *len = VXC_BIN_TENSORTR_LEN;
            return vxcBinTensorTR;
    
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
