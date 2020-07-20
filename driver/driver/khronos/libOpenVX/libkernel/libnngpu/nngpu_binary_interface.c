
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#include "nngpu_binary_interface.h"
#if gcdUSE_VXC_BINARY
#include "gpu_binaries.h"
#endif

void * GetGPUNNVXCBinaryPtr(nngpu_kernel_enum type, unsigned int* len)
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
    
        case Space2Depth:
            *len = VXC_BIN_SPACE2DEPTH_LEN;
            return vxcBinSpace2Depth;
    
        case BatchNorm:
            *len = VXC_BIN_BATCHNORM_LEN;
            return vxcBinBatchNorm;
    
        case Batch2Space:
            *len = VXC_BIN_BATCH2SPACE_LEN;
            return vxcBinBatch2Space;
    
        case TensorMeanAxis:
            *len = VXC_BIN_TENSORMEANAXIS_LEN;
            return vxcBinTensorMeanAxis;
    
        case LSTMUnit:
            *len = VXC_BIN_LSTMUNIT_LEN;
            return vxcBinLSTMUnit;
    
        case ShuffleChannel:
            *len = VXC_BIN_SHUFFLECHANNEL_LEN;
            return vxcBinShuffleChannel;
    
        case TensorReduceDiv:
            *len = VXC_BIN_TENSORREDUCEDIV_LEN;
            return vxcBinTensorReduceDiv;
    
        case Floor:
            *len = VXC_BIN_FLOOR_LEN;
            return vxcBinFloor;
    
        case LeakyRelu:
            *len = VXC_BIN_LEAKYRELU_LEN;
            return vxcBinLeakyRelu;
    
        case NormalizationAxis0:
            *len = VXC_BIN_NORMALIZATIONAXIS0_LEN;
            return vxcBinNormalizationAxis0;
    
        case Depth2Space:
            *len = VXC_BIN_DEPTH2SPACE_LEN;
            return vxcBinDepth2Space;
    
        case TensorCrop:
            *len = VXC_BIN_TENSORCROP_LEN;
            return vxcBinTensorCrop;
    
        case Gemm_noBias:
            *len = VXC_BIN_GEMM_NOBIAS_LEN;
            return vxcBinGemm_noBias;
    
        case Space2Batch:
            *len = VXC_BIN_SPACE2BATCH_LEN;
            return vxcBinSpace2Batch;
    
        case TensorExpand:
            *len = VXC_BIN_TENSOREXPAND_LEN;
            return vxcBinTensorExpand;
    
        case TensorCopyROI:
            *len = VXC_BIN_TENSORCOPYROI_LEN;
            return vxcBinTensorCopyROI;
    
        case Reorg:
            *len = VXC_BIN_REORG_LEN;
            return vxcBinReorg;
    
        case SoftMax:
            *len = VXC_BIN_SOFTMAX_LEN;
            return vxcBinSoftMax;
    
        case MaxPooling:
            *len = VXC_BIN_MAXPOOLING_LEN;
            return vxcBinMaxPooling;
    
        case ResizeNearestNeighbor:
            *len = VXC_BIN_RESIZENEARESTNEIGHBOR_LEN;
            return vxcBinResizeNearestNeighbor;
    
        case DepthwiseConv:
            *len = VXC_BIN_DEPTHWISECONV_LEN;
            return vxcBinDepthwiseConv;
    
        case TensorEltwise:
            *len = VXC_BIN_TENSORELTWISE_LEN;
            return vxcBinTensorEltwise;
    
        case Svdf:
            *len = VXC_BIN_SVDF_LEN;
            return vxcBinSvdf;
    
        case Conv2D_1x1:
            *len = VXC_BIN_CONV2D_1X1_LEN;
            return vxcBinConv2D_1x1;
    
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
    
        case TensorMaxValue:
            *len = VXC_BIN_TENSORMAXVALUE_LEN;
            return vxcBinTensorMaxValue;
    
        case Gemm:
            *len = VXC_BIN_GEMM_LEN;
            return vxcBinGemm;
    
        case TensorPad:
            *len = VXC_BIN_TENSORPAD_LEN;
            return vxcBinTensorPad;
    
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
    
        case Rnn:
            *len = VXC_BIN_RNN_LEN;
            return vxcBinRnn;
    
        case Swish:
            *len = VXC_BIN_SWISH_LEN;
            return vxcBinSwish;
    
        case L2NormAxis1:
            *len = VXC_BIN_L2NORMAXIS1_LEN;
            return vxcBinL2NormAxis1;
    
        case L2Pooling:
            *len = VXC_BIN_L2POOLING_LEN;
            return vxcBinL2Pooling;
    
        case HSwish:
            *len = VXC_BIN_HSWISH_LEN;
            return vxcBinHSwish;
    
        case Prelu:
            *len = VXC_BIN_PRELU_LEN;
            return vxcBinPrelu;
    
        case ROIPool:
            *len = VXC_BIN_ROIPOOL_LEN;
            return vxcBinROIPool;
    
        case FullyConnected:
            *len = VXC_BIN_FULLYCONNECTED_LEN;
            return vxcBinFullyConnected;
    
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
        printf("ERROR: Invalid nngpu kernel binary type!\n");
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
