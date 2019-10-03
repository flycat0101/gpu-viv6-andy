#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include"nngpu_binary_interface.h"
#if gcdUSE_VXC_BINARY
#include"gpu_binaries.h"
#endif

void * GetBinaryPtr(nngpu_kernel_enum type, unsigned int* len)
{
    switch(type)
    {

        case TensorPad:
            *len = VXC_BIN_TENSORPAD_LEN;
            return vxcBinTensorPad;
    
        case TensorStridedSlice:
            *len = VXC_BIN_TENSORSTRIDEDSLICE_LEN;
            return vxcBinTensorStridedSlice;
    
        case EmbeddingLUT:
            *len = VXC_BIN_EMBEDDINGLUT_LEN;
            return vxcBinEmbeddingLUT;
    
        case Tensor2Row:
            *len = VXC_BIN_TENSOR2ROW_LEN;
            return vxcBinTensor2Row;
    
        case MaxPooling:
            *len = VXC_BIN_MAXPOOLING_LEN;
            return vxcBinMaxPooling;
    
        case TensorTranspose:
            *len = VXC_BIN_TENSORTRANSPOSE_LEN;
            return vxcBinTensorTranspose;
    
        case Space2Batch:
            *len = VXC_BIN_SPACE2BATCH_LEN;
            return vxcBinSpace2Batch;
    
        case Space2Depth:
            *len = VXC_BIN_SPACE2DEPTH_LEN;
            return vxcBinSpace2Depth;
    
        case DepthwiseConv:
            *len = VXC_BIN_DEPTHWISECONV_LEN;
            return vxcBinDepthwiseConv;
    
        case L2NormSumSqrt:
            *len = VXC_BIN_L2NORMSUMSQRT_LEN;
            return vxcBinL2NormSumSqrt;
    
        case Depth2Space:
            *len = VXC_BIN_DEPTH2SPACE_LEN;
            return vxcBinDepth2Space;
    
        case TensorScale:
            *len = VXC_BIN_TENSORSCALE_LEN;
            return vxcBinTensorScale;
    
        case Activation:
            *len = VXC_BIN_ACTIVATION_LEN;
            return vxcBinActivation;
    
        case TensorElewise:
            *len = VXC_BIN_TENSORELEWISE_LEN;
            return vxcBinTensorElewise;
    
        case Reverse:
            *len = VXC_BIN_REVERSE_LEN;
            return vxcBinReverse;
    
        case Conv2D_1x1:
            *len = VXC_BIN_CONV2D_1X1_LEN;
            return vxcBinConv2D_1x1;
    
        case TensorReduceDiv:
            *len = VXC_BIN_TENSORREDUCEDIV_LEN;
            return vxcBinTensorReduceDiv;
    
        case TensorMaxValue:
            *len = VXC_BIN_TENSORMAXVALUE_LEN;
            return vxcBinTensorMaxValue;
    
        case L2NormSumScale:
            *len = VXC_BIN_L2NORMSUMSCALE_LEN;
            return vxcBinL2NormSumScale;
    
        case LSTMUnit:
            *len = VXC_BIN_LSTMUNIT_LEN;
            return vxcBinLSTMUnit;
    
        case Svdf:
            *len = VXC_BIN_SVDF_LEN;
            return vxcBinSvdf;
    
        case TensorMeanAxis0:
            *len = VXC_BIN_TENSORMEANAXIS0_LEN;
            return vxcBinTensorMeanAxis0;
    
        case TensorCrop:
            *len = VXC_BIN_TENSORCROP_LEN;
            return vxcBinTensorCrop;
    
        case Gemm_noBias:
            *len = VXC_BIN_GEMM_NOBIAS_LEN;
            return vxcBinGemm_noBias;
    
        case Rnn:
            *len = VXC_BIN_RNN_LEN;
            return vxcBinRnn;
    
        case AvgPooling:
            *len = VXC_BIN_AVGPOOLING_LEN;
            return vxcBinAvgPooling;
    
        case SoftMax:
            *len = VXC_BIN_SOFTMAX_LEN;
            return vxcBinSoftMax;
    
        case LSTMUnitProjection:
            *len = VXC_BIN_LSTMUNITPROJECTION_LEN;
            return vxcBinLSTMUnitProjection;
    
        case TensorCopyROI:
            *len = VXC_BIN_TENSORCOPYROI_LEN;
            return vxcBinTensorCopyROI;
    
        case TensorCopy:
            *len = VXC_BIN_TENSORCOPY_LEN;
            return vxcBinTensorCopy;
    
        case L2Pooling:
            *len = VXC_BIN_L2POOLING_LEN;
            return vxcBinL2Pooling;
    
        case FullyConnected:
            *len = VXC_BIN_FULLYCONNECTED_LEN;
            return vxcBinFullyConnected;
    
        case HashLUT:
            *len = VXC_BIN_HASHLUT_LEN;
            return vxcBinHashLUT;
    
        case Normalization:
            *len = VXC_BIN_NORMALIZATION_LEN;
            return vxcBinNormalization;
    
        case Gemm:
            *len = VXC_BIN_GEMM_LEN;
            return vxcBinGemm;
    
        case Batch2Space:
            *len = VXC_BIN_BATCH2SPACE_LEN;
            return vxcBinBatch2Space;
    
        case Floor:
            *len = VXC_BIN_FLOOR_LEN;
            return vxcBinFloor;
        default:
        printf("ERROR: Invalid nngpu kernel binary type!\n");
    }
    *len = 0;
    return NULL;
}

#if defined(_WINDOWS)
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
