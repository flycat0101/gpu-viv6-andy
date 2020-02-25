/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <gc_vx_common.h>
#include <gc_vx_nn_wb.h>
#include <layers/gc_vx_layer_deconv.h>

extern vx_status vxnneExecuteSWConvolution(vxnne_operation operation);

/***************************************************************************************************************************
 *                                                 DeConvolution
 ***************************************************************************************************************************/
VX_PRIVATE_API void vxnneLayerSW_gemm_nn(vx_type_e A_format, vx_type_e B_format, vx_type_e C_format, vx_int32 roundMode,
                                         vx_int8 A_fixedPointPos, vx_int8 B_fixedPointPos, vx_int8 C_fixedPointPos,
                                         vx_int32 M, vx_int32 N, vx_int32 K, vx_float32 ALPHA,
                                         vx_uint8_ptr A, vx_int32 lda,
                                         vx_uint8_ptr B, vx_int32 ldb,
                                         vx_uint8_ptr C, vx_int32 ldc)
{
    vx_int32 i,j,k;
    for(i = 0; i < M; ++i){
        for(k = 0; k < K; ++k){
            /*register float A_PART = ALPHA*A[i*lda+k];*/
            register float A_PART = ALPHA*vxnneGetData(A_format, i*lda+k, A, A_fixedPointPos);
            for(j = 0; j < N; ++j){
                /*C[i*ldc+j] += A_PART*B[k*ldb+j];*/
                vxnneSaveData(C_format, i*ldc+j, vxnneGetData(C_format, i*ldc+j, C, C_fixedPointPos) + A_PART*vxnneGetData(B_format, k*ldb+j, B, B_fixedPointPos), C, C_fixedPointPos, roundMode);
            }
        }
    }
}

VX_PRIVATE_API void vxnneLayerSW_gemm_nn_u8(vx_type_e A_format, vx_type_e B_format, vx_type_e C_format, vx_int32 roundMode,
                                            vx_int32 A_zeroPoint, vx_float32 A_scale,
                                            vx_int32 B_zeroPoint, vx_float32 B_scale,
                                            vx_int8 C_fixedPointPos,
                                            vx_int32 M, vx_int32 N, vx_int32 K, vx_float32 ALPHA,
                                            vx_uint8_ptr A, vx_int32 lda,
                                            vx_uint8_ptr B, vx_int32 ldb,
                                            vx_uint8_ptr C, vx_int32 ldc)
{
    vx_int32 i,j,k;
    for(i = 0; i < M; ++i){
        for(k = 0; k < K; ++k){
            /*register float A_PART = ALPHA*A[i*lda+k];*/
            register float A_PART = ALPHA*vxnneGetDataQuant(A_format, i*lda+k, A, A_zeroPoint, A_scale);
            for(j = 0; j < N; ++j){
                /*C[i*ldc+j] += A_PART*B[k*ldb+j];*/
                vxnneSaveData(C_format, i*ldc+j, vxnneGetData(C_format, i*ldc+j, C, C_fixedPointPos) + A_PART*vxnneGetDataQuant(B_format, k*ldb+j, B, B_zeroPoint, B_scale), C, C_fixedPointPos, roundMode);
            }
        }
    }
}

VX_PRIVATE_API void vxnneLayerSW_gemm_nt(vx_type_e A_format, vx_type_e B_format, vx_type_e C_format, vx_int32 roundMode,
                                         vx_int8 A_fixedPointPos, vx_int8 B_fixedPointPos, vx_int8 C_fixedPointPos,
        vx_int32 M, vx_int32 N, vx_int32 K, vx_float32 ALPHA,
        vx_uint8_ptr A, vx_int32 lda,
        vx_uint8_ptr B, vx_int32 ldb,
        vx_uint8_ptr C, vx_int32 ldc)
{
    vx_int32 i,j,k;
    for(i = 0; i < M; ++i){
        for(j = 0; j < N; ++j){
            register float sum = 0;
            for(k = 0; k < K; ++k){
                /*sum += ALPHA*A[i*lda+k]*B[j*ldb + k];*/
                sum += ALPHA * vxnneGetData(A_format, i*lda+k, A, A_fixedPointPos) * vxnneGetData(B_format, j*ldb + k, B, B_fixedPointPos);
            }
            /*C[i*ldc+j] += sum;*/
            vxnneSaveData(C_format, i*ldc+j, vxnneGetData(C_format, i*ldc+j, C, C_fixedPointPos) + sum, C, C_fixedPointPos, roundMode);
        }
    }
}

VX_PRIVATE_API void vxnneLayerSW_gemm_nt_u8(vx_type_e A_format, vx_type_e B_format, vx_type_e C_format, vx_int32 roundMode,
                                            vx_int32 A_zeroPoint, vx_float32 A_scale,
                                            vx_int32 B_zeroPoint, vx_float32 B_scale,
                                            vx_int8 C_fixedPointPos,
                                            vx_int32 M, vx_int32 N, vx_int32 K, vx_float32 ALPHA,
                                            vx_uint8_ptr A, vx_int32 lda,
                                            vx_uint8_ptr B, vx_int32 ldb,
                                            vx_uint8_ptr C, vx_int32 ldc)
{
    vx_int32 i,j,k;
    for(i = 0; i < M; ++i){
        for(j = 0; j < N; ++j){
            register float sum = 0;
            for(k = 0; k < K; ++k){
                /*sum += ALPHA*A[i*lda+k]*B[j*ldb + k];*/
                sum += ALPHA * vxnneGetDataQuant(A_format, i*lda+k, A, A_zeroPoint, A_scale) * vxnneGetDataQuant(B_format, j*ldb + k, B, B_zeroPoint, B_scale);
            }
            /*C[i*ldc+j] += sum;*/
            vxnneSaveData(C_format, i*ldc+j, vxnneGetData(C_format, i*ldc+j, C, C_fixedPointPos) + sum, C, C_fixedPointPos, roundMode);
        }
    }
}

VX_PRIVATE_API void vxnneLayerSW_gemm_tn(vx_type_e A_format, vx_type_e B_format, vx_type_e C_format, vx_int32 roundMode,
                                         vx_int8 A_fixedPointPos, vx_int8 B_fixedPointPos, vx_int8 C_fixedPointPos,
                                         vx_int32 M, vx_int32 N, vx_int32 K, vx_float32 ALPHA,
                                         vx_uint8_ptr A, vx_int32 lda,
                                         vx_uint8_ptr B, vx_int32 ldb,
                                         vx_uint8_ptr C, vx_int32 ldc)
{
    vx_int32 i,j,k;
    for(i = 0; i < M; ++i){
        for(k = 0; k < K; ++k){
            /*register float A_PART = ALPHA*A[k*lda+i];*/
            register float A_PART = ALPHA*vxnneGetData(A_format, k*lda+i, A, A_fixedPointPos);
            for(j = 0; j < N; ++j){
                /*C[i*ldc+j] += A_PART*B[k*ldb+j];*/
                vxnneSaveData(C_format, i*ldc+j, vxnneGetData(C_format, i*ldc+j, C, C_fixedPointPos) + A_PART*vxnneGetData(B_format, k*ldb+j, B, B_fixedPointPos), C, C_fixedPointPos, roundMode);
            }
        }
    }
}

VX_PRIVATE_API void vxnneLayerSW_gemm_tn_u8(vx_type_e A_format, vx_type_e B_format, vx_type_e C_format, vx_int32 roundMode,
                                            vx_int32 A_zeroPoint, vx_float32 A_scale,
                                            vx_int32 B_zeroPoint, vx_float32 B_scale,
                                            vx_int8 C_fixedPointPos,
                                            vx_int32 M, vx_int32 N, vx_int32 K, vx_float32 ALPHA,
                                            vx_uint8_ptr A, vx_int32 lda,
                                            vx_uint8_ptr B, vx_int32 ldb,
                                            vx_uint8_ptr C, vx_int32 ldc)
{
    vx_int32 i,j,k;
    for(i = 0; i < M; ++i){
        for(k = 0; k < K; ++k){
            /*register float A_PART = ALPHA*A[k*lda+i];*/
            register float A_PART = ALPHA*vxnneGetDataQuant(A_format, k*lda+i, A, A_zeroPoint, A_scale);
            for(j = 0; j < N; ++j){
                /*C[i*ldc+j] += A_PART*B[k*ldb+j];*/
                vxnneSaveData(C_format, i*ldc+j, vxnneGetData(C_format, i*ldc+j, C, C_fixedPointPos) + A_PART*vxnneGetDataQuant(B_format, k*ldb+j, B, B_zeroPoint, B_scale), C, C_fixedPointPos, roundMode);
            }
        }
    }
}

VX_PRIVATE_API void vxnneLayerSW_gemm_tt(vx_type_e A_format, vx_type_e B_format, vx_type_e C_format, vx_int32 roundMode,
                                         vx_int8 A_fixedPointPos, vx_int8 B_fixedPointPos, vx_int8 C_fixedPointPos,
                                         vx_int32 M, vx_int32 N, vx_int32 K, vx_float32 ALPHA,
                                         vx_uint8_ptr A, vx_int32 lda,
                                         vx_uint8_ptr B, vx_int32 ldb,
                                         vx_uint8_ptr C, vx_int32 ldc)
{
    vx_int32 i,j,k;
    for(i = 0; i < M; ++i){
        for(j = 0; j < N; ++j){
            register float sum = 0;
            for(k = 0; k < K; ++k){
                /*sum += ALPHA*A[i+k*lda]*B[k+j*ldb];*/
                sum += ALPHA * vxnneGetData(A_format, i+k*lda, A, A_fixedPointPos) * vxnneGetData(B_format, k+j*ldb, B, B_fixedPointPos);
            }
            /*C[i*ldc+j] += sum;*/
            vxnneSaveData(C_format, i*ldc+j, sum, C, C_fixedPointPos, roundMode);
        }
    }
}

VX_PRIVATE_API void vxnneLayerSW_gemm_tt_u8(vx_type_e A_format, vx_type_e B_format, vx_type_e C_format, vx_int32 roundMode,
                                            vx_int32 A_zeroPoint, vx_float32 A_scale,
                                            vx_int32 B_zeroPoint, vx_float32 B_scale,
                                            vx_int8 C_fixedPointPos,
                                            vx_int32 M, vx_int32 N, vx_int32 K, vx_float32 ALPHA,
                                            vx_uint8_ptr A, vx_int32 lda,
                                            vx_uint8_ptr B, vx_int32 ldb,
                                            vx_uint8_ptr C, vx_int32 ldc)
{
    vx_int32 i,j,k;
    for(i = 0; i < M; ++i){
        for(j = 0; j < N; ++j){
            register float sum = 0;
            for(k = 0; k < K; ++k){
                /*sum += ALPHA*A[i+k*lda]*B[k+j*ldb];*/
                sum += ALPHA * vxnneGetDataQuant(A_format, i+k*lda, A, A_zeroPoint, A_scale) * vxnneGetDataQuant(B_format, k+j*ldb, B, B_zeroPoint, B_scale);
            }
            /*C[i*ldc+j] += sum;*/
            vxnneSaveData(C_format, i*ldc+j, sum, C, C_fixedPointPos, roundMode);
        }
    }
}


VX_PRIVATE_API void vxnneLayerSW_gemm(vx_type_e A_format, vx_type_e B_format, vx_type_e C_format, vx_int32 roundMode,
                                      vx_int8 A_fixedPointPos, vx_int8 B_fixedPointPos, vx_int8 C_fixedPointPos,
                                      vx_int32 TA, vx_int32 TB, vx_int32 M, vx_int32 N, vx_int32 K, vx_float32 ALPHA,
                                      vx_uint8_ptr A, vx_int32 lda,
                                      vx_uint8_ptr B, vx_int32 ldb,
                                      vx_float32 BETA,
                                      vx_uint8_ptr C, vx_int32 ldc)
{
    /*vxInfo("cpu: %d %d %d %d %d %f %d %d %f %d\n",TA, TB, M, N, K, ALPHA, lda, ldb, BETA, ldc);*/
    int i, j;
    for(i = 0; i < M; ++i){
        for(j = 0; j < N; ++j){
            /*C[i*ldc + j] *= BETA;*/
            vxnneSaveData(C_format, i*ldc + j, BETA * vxnneGetData(C_format, i*ldc + j, C, C_fixedPointPos), C, C_fixedPointPos, roundMode);
        }
    }
    if(!TA && !TB)
        vxnneLayerSW_gemm_nn(A_format, B_format, C_format, roundMode, A_fixedPointPos, B_fixedPointPos, C_fixedPointPos, M, N, K, ALPHA,A,lda, B, ldb, C,ldc);
    else if(TA && !TB)
        vxnneLayerSW_gemm_tn(A_format, B_format, C_format, roundMode, A_fixedPointPos, B_fixedPointPos, C_fixedPointPos, M, N, K, ALPHA,A,lda, B, ldb, C,ldc);
    else if(!TA && TB)
        vxnneLayerSW_gemm_nt(A_format, B_format, C_format, roundMode, A_fixedPointPos, B_fixedPointPos, C_fixedPointPos, M, N, K, ALPHA,A,lda, B, ldb, C,ldc);
    else
        vxnneLayerSW_gemm_tt(A_format, B_format, C_format, roundMode, A_fixedPointPos, B_fixedPointPos, C_fixedPointPos, M, N, K, ALPHA,A,lda, B, ldb, C,ldc);
}

VX_PRIVATE_API void vxnneLayerSW_gemm_u8(vx_type_e A_format, vx_type_e B_format, vx_type_e C_format, vx_int32 roundMode,
                                         vx_int32 A_zeroPoint, vx_float32 A_scale,
                                         vx_int32 B_zeroPoint, vx_float32 B_scale,
                                         vx_int8 C_fixedPointPos,
                                         vx_int32 TA, vx_int32 TB, vx_int32 M, vx_int32 N, vx_int32 K, vx_float32 ALPHA,
                                         vx_uint8_ptr A, vx_int32 lda,
                                         vx_uint8_ptr B, vx_int32 ldb,
                                         vx_float32 BETA,
                                         vx_uint8_ptr C, vx_int32 ldc)
{
    /*vxInfo("cpu: %d %d %d %d %d %f %d %d %f %d\n",TA, TB, M, N, K, ALPHA, lda, ldb, BETA, ldc);*/
    int i, j;
    for(i = 0; i < M; ++i){
        for(j = 0; j < N; ++j){
            /*C[i*ldc + j] *= BETA;*/
            vxnneSaveData(C_format, i*ldc + j, BETA * vxnneGetData(C_format, i*ldc + j, C, C_fixedPointPos), C, C_fixedPointPos, roundMode);
        }
    }
    if(!TA && !TB)
        vxnneLayerSW_gemm_nn_u8(A_format, B_format, C_format, roundMode, A_zeroPoint, A_scale, B_zeroPoint, B_scale, C_fixedPointPos, M, N, K, ALPHA,A,lda, B, ldb, C,ldc);
    else if(TA && !TB)
        vxnneLayerSW_gemm_tn_u8(A_format, B_format, C_format, roundMode, A_zeroPoint, A_scale, B_zeroPoint, B_scale, C_fixedPointPos, M, N, K, ALPHA,A,lda, B, ldb, C,ldc);
    else if(!TA && TB)
        vxnneLayerSW_gemm_nt_u8(A_format, B_format, C_format, roundMode, A_zeroPoint, A_scale, B_zeroPoint, B_scale, C_fixedPointPos, M, N, K, ALPHA,A,lda, B, ldb, C,ldc);
    else
        vxnneLayerSW_gemm_tt_u8(A_format, B_format, C_format, roundMode, A_zeroPoint, A_scale, B_zeroPoint, B_scale, C_fixedPointPos, M, N, K, ALPHA,A,lda, B, ldb, C,ldc);
}

VX_PRIVATE_API vx_bool vxnneLayerSW_is_a_ge_zero_and_a_lt_b(vx_int32 a, vx_int32 b) {
  return (((vx_uint32)a) < (vx_uint32)(b))?vx_true_e:vx_false_e;
}
VX_PRIVATE_API void vxnneLayerSW_col2im_add_bias(vx_type_e col_format, vx_type_e im_format,vx_type_e bias_format, vx_int32 roundMode,
                                        vx_int8 col_fixedPointPos, vx_int8 im_fixedPointPos, vx_int8 bias_fixedPointPos,
    vx_uint8_ptr data_col,vx_uint8_ptr biases,
    const vx_int32 channels, const vx_int32 height, const vx_int32 width,
    const vx_int32 kernel_h, const vx_int32 kernel_w,
    const vx_int32 pad_h, const vx_int32 pad_w,
    const vx_int32 pad_h_b, const vx_int32 pad_w_r,
    const vx_int32 stride_h, const vx_int32 stride_w,
    const vx_int32 dilation_h, const vx_int32 dilation_w,
    vx_uint8_ptr data_im, vx_float32_ptr tmp_ptr) {
    vx_int32 channel = 0, kernel_row = 0, kernel_col = 0, input_row = 0, output_rows = 0, input_col = 0;
    vx_int32 output_col = 0;

    const vx_int32 output_h = (height + pad_h_b + pad_h - (dilation_h * (kernel_h - 1) + 1)) / stride_h + 1;
    const vx_int32 output_w = (width + pad_w_r + pad_w - (dilation_w * (kernel_w - 1) + 1)) / stride_w + 1;
    const vx_int32 channel_size = height * width;
    vx_int32 i=0;

    for (channel = channels; channel--; data_im += vxnneGetTypeSize(im_format) *channel_size) {
        gcoOS_MemFill(tmp_ptr, 0, sizeof(vx_float32) * channel_size);

        for (kernel_row = 0; kernel_row < kernel_h; kernel_row++) {
            for (kernel_col = 0; kernel_col < kernel_w; kernel_col++) {
                input_row = -pad_h + kernel_row * dilation_h;
                for (output_rows = output_h; output_rows; output_rows--) {
                    if (!vxnneLayerSW_is_a_ge_zero_and_a_lt_b(input_row, height)) {
                        /*data_col += output_w;*/
                        data_col += vxnneGetTypeSize(col_format) * output_w;
                    } else {
                          input_col = -pad_w + kernel_col * dilation_w;
                          for (output_col = output_w; output_col; output_col--) {
                              if (vxnneLayerSW_is_a_ge_zero_and_a_lt_b(input_col, width)) {
                                  /*data_im[input_row * width + input_col] += *data_col;*/
                                  vx_int32 idx = input_row * width + input_col;
                                  vx_float64 val1 = vxnneGetData(col_format, 0, data_col, col_fixedPointPos);
                                  tmp_ptr[idx] += (vx_float32)val1;
                              }
                              /*data_col++;*/
                              data_col += vxnneGetTypeSize(col_format);

                              input_col += stride_w;
                          }
                    }
                    input_row += stride_h;
                }
            }
        }

        if(biases)
        {
            vx_float32 fbias = vxnneGetData(bias_format, channels-channel-1, biases, bias_fixedPointPos);
            for(i=0;i<channel_size;i++)
                vxnneSaveData(im_format, i, tmp_ptr[i]+fbias, data_im, im_fixedPointPos, roundMode);
        }
    }
}

VX_PRIVATE_API void vxnneLayerSW_col2im_add_bias_u8(vx_type_e col_format, vx_type_e im_format,vx_type_e bias_format, vx_int32 roundMode,
    vx_int8 col_fixedPointPos, vx_int32 im_zeroPoint, vx_float32 im_scale, vx_int32 bias_zeroPoint, vx_float32 bias_scale,
    vx_uint8_ptr data_col,vx_uint8_ptr biases,
    const vx_int32 channels, const vx_int32 height, const vx_int32 width,
    const vx_int32 kernel_h, const vx_int32 kernel_w,
    const vx_int32 pad_h, const vx_int32 pad_w,
    const vx_int32 pad_h_b, const vx_int32 pad_w_r,
    const vx_int32 stride_h, const vx_int32 stride_w,
    const vx_int32 dilation_h, const vx_int32 dilation_w,
    vx_uint8_ptr data_im, vx_float32_ptr tmp_ptr) {
    vx_int32 channel = 0, kernel_row = 0, kernel_col = 0, input_row = 0, output_rows = 0, input_col = 0;
    vx_int32 output_col = 0;

    const vx_int32 output_h = (height + 2 * gcmMAX(pad_h, pad_h_b) - (dilation_h * (kernel_h - 1) + 1)) / stride_h + 1;
    const vx_int32 output_w = (width + 2 * gcmMAX(pad_w, pad_w_r) - (dilation_w * (kernel_w - 1) + 1)) / stride_w + 1;
    const vx_int32 channel_size = height * width;
    vx_int32 i=0;

    for (channel = channels; channel--; data_im += vxnneGetTypeSize(im_format) *channel_size) {
        gcoOS_MemFill(tmp_ptr, 0, sizeof(vx_float32) * channel_size);

        for (kernel_row = 0; kernel_row < kernel_h; kernel_row++) {
            for (kernel_col = 0; kernel_col < kernel_w; kernel_col++) {
                input_row = -pad_h + kernel_row * dilation_h;
                for (output_rows = output_h; output_rows; output_rows--) {
                    if (!vxnneLayerSW_is_a_ge_zero_and_a_lt_b(input_row, height)) {
                        /*data_col += output_w;*/
                        data_col += vxnneGetTypeSize(col_format) * output_w;
                    } else {
                          input_col = -pad_w + kernel_col * dilation_w;
                          for (output_col = output_w; output_col; output_col--) {
                              if (vxnneLayerSW_is_a_ge_zero_and_a_lt_b(input_col, width)) {
                                  /*data_im[input_row * width + input_col] += *data_col;*/
                                  vx_int32 idx = input_row * width + input_col;
                                  vx_float64 val1 = vxnneGetData(col_format, 0, data_col, col_fixedPointPos);
                                  tmp_ptr[idx] += (vx_float32)val1;
                              }
                              /*data_col++;*/
                              data_col += vxnneGetTypeSize(col_format);

                              input_col += stride_w;
                          }
                    }
                    input_row += stride_h;
                }
            }
        }

        if(biases)
        {
            vx_float32 fbias = vxnneGetDataQuant(bias_format, channels-channel-1, biases, bias_zeroPoint, bias_scale);
            for(i=0;i<channel_size;i++)
                vxnneSaveDataQuant(im_format, i, tmp_ptr[i]+fbias, data_im, im_zeroPoint, im_scale, roundMode);
        }
    }
}

#define INDEX_WIDTH 0
#define INDEX_HEIGHT 1
#define INDEX_DEPTH 2
#define INDEX_BATCH 3

VX_PRIVATE_API vx_status vxnneExecuteSWDeConv_ReshuffleWeights(struct _vxnne_operation_s *operation)
{
    vxnne_deconvolution_reshuffle_operation deconvOperation   = (vxnne_deconvolution_reshuffle_operation)operation;

    vx_tensor weights               = deconvOperation->weights;
    vx_tensor reshuffle_weights     = deconvOperation->reshuffled_weights;
    vx_tensor biases                = deconvOperation->bias;
    vx_tensor reshuffled_biases     = deconvOperation->reshuffled_biases;
    vx_int32 stride_w               = deconvOperation->stride_x->value->n32;
    vx_int32 stride_h               = deconvOperation->stride_y->value->n32;
    vx_int32 group                  = deconvOperation->group->value->n32;
    vx_tensor inputs                = deconvOperation->inputs;

    vx_type_e weight_format = (vx_type_e)(TENSOR_DATA_TYPE(weights));
    vx_type_e reshuffle_weight_format = (vx_type_e)(TENSOR_DATA_TYPE(reshuffle_weights));

    vx_int32 kernel_size_x = TENSOR_SIZE_INDEX(weights, 0);
    vx_int32 kernel_size_y = TENSOR_SIZE_INDEX(weights, 1);
    vx_int32 kernel_size_c = TENSOR_SIZE_INDEX(weights, 3);

    vx_uint8_ptr reshuffled_weights = VX_NULL;

    vx_int32 reshuffle_width = gcmALIGN_NP2(kernel_size_x, stride_w)/stride_w, reshuffle_height = gcmALIGN_NP2(kernel_size_y, stride_h)/stride_h;
    vx_int32 slice_size = kernel_size_x * kernel_size_y, reshuffled_slice_size = reshuffle_width * reshuffle_height;

    vx_int32 kernel_reshuffle_pad_x_left = ((TENSOR_VIEW_SIZE_INDEX(deconvOperation->outputs, 0) - 1) + reshuffle_width - TENSOR_VIEW_SIZE_INDEX(deconvOperation->inputs, 0)) / 2;
    vx_int32 kernel_reshuffle_pad_x_right = ((TENSOR_VIEW_SIZE_INDEX(deconvOperation->outputs, 0) - 1) + reshuffle_width - TENSOR_VIEW_SIZE_INDEX(deconvOperation->inputs, 0)) - kernel_reshuffle_pad_x_left;
    vx_int32 kernel_reshuffle_pad_y_top = ((TENSOR_VIEW_SIZE_INDEX(deconvOperation->outputs, 1) - 1) + reshuffle_height - TENSOR_VIEW_SIZE_INDEX(deconvOperation->inputs, 1)) / 2;
    vx_int32 kernel_reshuffle_pad_y_bottom = ((TENSOR_VIEW_SIZE_INDEX(deconvOperation->outputs, 1) - 1) + reshuffle_height - TENSOR_VIEW_SIZE_INDEX(deconvOperation->inputs, 1)) - kernel_reshuffle_pad_y_top;

    vx_int32 w = 0, h = 0, sx = 0, sy = 0, c = 0, b = 0, batch = TENSOR_SIZE_INDEX(weights, 2);/*kernel_size_c;*/
    vx_uint8_ptr data = reshuffled_weights, buffer = VX_NULL;
    vx_int32 item_size = vxnneGetTypeSize(weight_format);
    vx_uint8_ptr weights_ptr = weights->tensorBuffer->memory.logicals[0];

    vx_int8 weightsFixedPointPos = TENSOR_POS(weights);
    vx_bool depthwise = (group != kernel_size_c) ? vx_false_e:vx_true_e;

    reshuffled_weights = reshuffle_weights->tensorBuffer->memory.logicals[0];

    if (deconvOperation->reshuffled)
        memcpy(reshuffled_weights, weights_ptr, item_size * slice_size * kernel_size_c * batch);
    else
    {
        buffer = (vx_uint8_ptr)vxAllocateAndZeroMemory(item_size * slice_size * kernel_size_c * batch);
        data = reshuffled_weights;
        if (group != kernel_size_c)
        {


        /* transpose */
        for (b = 0; b < batch; b++)
        {
            for (c = 0; c < kernel_size_c; c++)
            {
                memcpy(buffer + kernel_size_x * kernel_size_y * (b * kernel_size_c + c) * item_size, weights_ptr + kernel_size_x * kernel_size_y * (c * batch + b) * item_size, item_size * slice_size);
            }
        }
        }
        else
            memcpy(buffer, weights_ptr, item_size * slice_size * kernel_size_c * batch);

        if (depthwise)
        {
            /* reshuffle */
            for (b = 0; b < kernel_size_c; b++)
            {
                for (sy = 0; sy < stride_h; sy++)
                {
                    for (sx = 0; sx < stride_w; sx++)
                    {
                        for (c = 0; c < kernel_size_c; c++)
                        {
                            vx_uint8_ptr weight_output = reshuffled_weights + (b * reshuffled_slice_size * stride_w * stride_h * kernel_size_c + reshuffled_slice_size * kernel_size_c * (sy * stride_w + sx) + reshuffled_slice_size * c) * item_size;

                            data = buffer + b * slice_size * item_size;

                            for (h = 0; h < reshuffle_height; h++)
                            {
                                for (w = 0; w < reshuffle_width; w++)
                                {

                                    /*     ___________
                                    *    |     |     |
                                    *    |  3  |  2  |
                                    *    |_____|_____|
                                    *    |     |     |
                                    *    |  1  |  0  |
                                    *    |_____|_____|
                                    *
                                    */
                                    vx_uint8_ptr reshuffled_output = weight_output + (h * reshuffle_width + w) * item_size;
                                    vx_int32 input_index = ((reshuffle_height - 1 - h) * stride_w + sy) * kernel_size_x + ((reshuffle_width - 1 - w) * stride_h + sx);
                                    vx_int32 delat_x = reshuffle_width * stride_w - kernel_size_x, delat_y = reshuffle_height * stride_h - kernel_size_y;

                                    /**reshuffled_output = data[input_index];*/

                                    vx_int32 input_x = (reshuffle_width - 1 - w) * stride_w + sx - delat_x;
                                    vx_int32 input_y = (reshuffle_height - 1 - h) * stride_h + sy - delat_y;
                                    input_index = input_y * kernel_size_x + input_x;

                                    if ((input_x >= kernel_size_x) || (input_y >= kernel_size_y)
                                        || (input_x < 0) || (input_y < 0))
                                    {
                                        if (weight_format == VX_TYPE_UINT8 && TENSOR_QUANT_TYPE(reshuffle_weights) == VX_QUANT_AFFINE_SCALE)
                                            memset(reshuffled_output, TENSOR_TF_ZEROPOINT(weights), item_size);
                                        else
                                            memset(reshuffled_output, 0, item_size);
                                    }
                                    else if (reshuffle_weight_format == weight_format)
                                    {
                                        if (c == b)
                                            memcpy(reshuffled_output, data + input_index * item_size, item_size);
                                        else
                                        {
                                            if (weight_format == VX_TYPE_UINT8 && TENSOR_QUANT_TYPE(reshuffle_weights) == VX_QUANT_AFFINE_SCALE)
                                                memset(reshuffled_output, TENSOR_TF_ZEROPOINT(weights), item_size);
                                            else
                                                memset(reshuffled_output, 0, item_size);
                                        }
                                    }
                                    else
                                    {
                                        vxnneSaveDataExt(weight_format, TENSOR_QUANT_TYPE(reshuffle_weights), 0, vxnneGetDataExt(weight_format, TENSOR_QUANT_TYPE(reshuffle_weights), input_index, data, weightsFixedPointPos, TENSOR_TF_ZEROPOINT(reshuffle_weights), TENSOR_TF_SCALE(reshuffle_weights)), reshuffled_output, weightsFixedPointPos, TENSOR_TF_ZEROPOINT(reshuffle_weights), TENSOR_TF_SCALE(reshuffle_weights), 0);

                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {

            /* reshuffle */
            for (b = 0; b < batch; b++)
            {
                for (sy = 0; sy < stride_h; sy++)
                {
                    for (sx = 0; sx < stride_w; sx++)
                    {
                        for (c = 0; c < kernel_size_c; c++)
                        {
                            vx_uint8_ptr weight_output = reshuffled_weights + (b * reshuffled_slice_size * stride_w * stride_h * kernel_size_c + reshuffled_slice_size * kernel_size_c * (sy * stride_w + sx) + reshuffled_slice_size * c) * item_size;

                            data = buffer + (b * slice_size * kernel_size_c + slice_size * c) * item_size;

                            for (h = 0; h < reshuffle_height; h++)
                            {
                                for (w = 0; w < reshuffle_width; w++)
                                {

                                    /*     ___________
                                     *    |     |     |
                                     *    |  3  |  2  |
                                     *    |_____|_____|
                                     *    |     |     |
                                     *    |  1  |  0  |
                                     *    |_____|_____|
                                     *
                                     */
                                    vx_uint8_ptr reshuffled_output = weight_output + (h * reshuffle_width + w) * item_size;
                                    vx_int32 input_index = ((reshuffle_height - 1 - h) * stride_w + sy) * kernel_size_x + ((reshuffle_width - 1 - w) * stride_h + sx);
                                    vx_int32 delat_x = reshuffle_width * stride_w - kernel_size_x, delat_y = reshuffle_height * stride_h - kernel_size_y;

                                    /**reshuffled_output = data[input_index];*/

                                    vx_int32 input_x = (reshuffle_width - 1 - w) * stride_w + sx - delat_x;
                                    vx_int32 input_y = (reshuffle_height - 1 - h) * stride_h + sy - delat_y;
                                    input_index = input_y * kernel_size_x + input_x;

                                    if ((input_x >=  kernel_size_x) || (input_y >=  kernel_size_y)
                                        || (input_x < 0) || (input_y < 0))
                                    {
                                        if (weight_format == VX_TYPE_UINT8 && TENSOR_QUANT_TYPE(reshuffle_weights) == VX_QUANT_AFFINE_SCALE)
                                            memset(reshuffled_output, TENSOR_TF_ZEROPOINT(weights), item_size);
                                        else
                                            memset(reshuffled_output, 0, item_size);
                                    }
                                    else if (reshuffle_weight_format == weight_format)
                                    {
                                        memcpy(reshuffled_output, data + input_index * item_size, item_size);
                                    }
                                    else
                                    {
                                        vxnneSaveDataExt(weight_format, TENSOR_QUANT_TYPE(reshuffle_weights), 0, vxnneGetDataExt(weight_format, TENSOR_QUANT_TYPE(reshuffle_weights), input_index, data, weightsFixedPointPos, TENSOR_TF_ZEROPOINT(reshuffle_weights), TENSOR_TF_SCALE(reshuffle_weights)), reshuffled_output, weightsFixedPointPos, TENSOR_TF_ZEROPOINT(reshuffle_weights), TENSOR_TF_SCALE(reshuffle_weights), 0);

                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        vxFree(buffer);

        if (biases && reshuffled_biases)
        {
            vx_int32 i = 0;

            vx_int32 bias_c = TENSOR_SIZE_INDEX(biases, 3);
            vx_int32 r_bias_c = TENSOR_SIZE_INDEX(reshuffled_biases, 3);
            vx_int32 item_size = vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(biases));
            vx_uint8_ptr bias_ptr = TENSOR_LOGICAL_ADDR(biases);
            vx_uint8_ptr reshuffled_bias_ptr = TENSOR_LOGICAL_ADDR(reshuffled_biases);

            if (bias_c != r_bias_c)
            {

                for (i = 0; i < r_bias_c/(stride_w * stride_h); i ++)
                {
                    vx_int32 j = 0;
                    for (j = 0; j < (stride_w * stride_h); j ++)
                        memcpy(reshuffled_bias_ptr + (i  * (stride_w * stride_h) + j)* item_size, bias_ptr + i * item_size, item_size);

                }

            }
        }
    }

    if (deconvOperation->create_wbp)
    {
        if (weight_format == VX_TYPE_UINT8 && TENSOR_QUANT_TYPE(reshuffle_weights) == VX_QUANT_AFFINE_SCALE)
        {
            vx_weights_biases_parameter_optimizations_t opt;

            opt.inputZeroPoint = TENSOR_TF_ZEROPOINT(inputs);
            opt.zrl = -1;
            opt.outputFormat = VX_TYPE_UINT8;

            deconvOperation->weights_biaes = vxoCreateWeightsBiasesParameterFromTensors(
                                vxGetContext((vx_reference)deconvOperation->weights),
                                VX_NN_CONVOLUTION_LAYER,
                                deconvOperation->inputs->dims,/*inputs_dims,*/
                                deconvOperation->inputs->dimCount,
                                deconvOperation->inputs->dimCount,
                                kernel_reshuffle_pad_x_left,
                                kernel_reshuffle_pad_x_right,
                                kernel_reshuffle_pad_y_top,
                                kernel_reshuffle_pad_y_bottom,
                                0,/*pooling_size_x,*/
                                0,/*pooling_size_y,*/
                                0,
                                0,
                                VX_NN_DS_SIZE_ROUNDING_FLOOR,
                                deconvOperation->outputs->dims,/*convolution_outputs_dims,*/
                                deconvOperation->outputs->dims,/*pool_outputs_dims,*/
                                &opt, /*optimizations,*/
                                TENSOR_DATA_TYPE(weights),
                                0,
                                VX_TENSOR_RANK_WHCN,
                                deconvOperation->reshuffled_weights,
                                deconvOperation->reshuffled_biases,
                                VX_NULL,
                                vx_false_e,
                                vx_false_e
                                );
        }
        else
        {
            deconvOperation->weights_biaes = vxoCreateWeightsBiasesParameterFromTensors(
                                vxGetContext((vx_reference)deconvOperation->weights),
                                VX_NN_CONVOLUTION_LAYER,
                                deconvOperation->inputs->dims,/*inputs_dims,*/
                                deconvOperation->inputs->dimCount,
                                deconvOperation->inputs->dimCount,
                                kernel_reshuffle_pad_x_left,
                                kernel_reshuffle_pad_x_right,
                                kernel_reshuffle_pad_y_top,
                                kernel_reshuffle_pad_y_bottom,
                                0,/*pooling_size_x,*/
                                0,/*pooling_size_y,*/
                                0,
                                0,
                                VX_NN_DS_SIZE_ROUNDING_FLOOR,
                                deconvOperation->outputs->dims,/*convolution_outputs_dims,*/
                                deconvOperation->outputs->dims,/*pool_outputs_dims,*/
                                NULL, /*optimizations,*/
                                TENSOR_DATA_TYPE(weights),
                                0,
                                VX_TENSOR_RANK_WHCN,
                                deconvOperation->reshuffled_weights,
                                deconvOperation->reshuffled_biases,
                                VX_NULL,
                                vx_false_e,
                                vx_false_e
                                );
        }
    }

    return deconvOperation->weights_biaes == VX_NULL ? VX_FAILURE : VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxnneExecuteSWDeConv_UpSample(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_deconvolution_operation deconvOperation   = (vxnne_deconvolution_operation)operation;

    vx_tensor inputs        = deconvOperation->inputs;
    vx_tensor outputs       = deconvOperation->outputs;

    vx_type_e input_format = (vx_type_e)(TENSOR_DATA_TYPE(inputs));
    vx_type_e output_format = (vx_type_e)(TENSOR_DATA_TYPE(outputs));

    vx_int32 in_h = TENSOR_SIZE_INDEX(inputs, 1);
    vx_int32 in_w = TENSOR_SIZE_INDEX(inputs, 0);
    vx_int32 out_h = TENSOR_SIZE_INDEX(outputs, 1);
    vx_int32 out_w = TENSOR_SIZE_INDEX(outputs, 0);
    vx_int32 stride_w = (vx_int32)vxnneRound(out_w * 1.0f / in_w, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING);
    vx_int32 stride_h = (vx_int32)vxnneRound(out_h * 1.0f / in_h, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING);

    vx_int32 padding_x = deconvOperation->padding_x_left->value->n32;
    vx_int32 padding_y = deconvOperation->padding_y_top->value->n32;

    vx_int32 conv_out_channels = TENSOR_SIZE_INDEX(outputs, 2);

    vx_uint8_ptr input_ptr;
    vx_uint8_ptr output_ptr;

    vx_int32 i = 0, j = 0, b = 0;
    vx_int32 input_item_size = vxnneGetTypeSize(input_format);
    vx_int32 output_item_size = vxnneGetTypeSize(output_format);
    vx_int32 s_w = 0, s_h = 0;

    vx_int32 input_batch_offset = in_w * in_h * TENSOR_SIZE_INDEX(inputs, 2) * input_item_size *  operation->currBatchIndex;
    vx_int32 output_batch_offset = out_w * out_h * conv_out_channels * output_item_size *  operation->currBatchIndex;

    vxoTensor_GetTensorBatchArrayViewMemory(inputs, 0, (gctPOINTER*)&input_ptr, VX_NULL);
    vxoTensor_GetTensorBatchArrayViewMemory(outputs, 0, (gctPOINTER*)&output_ptr, VX_NULL);

    for (b = 0; b < conv_out_channels; b++)
    {
        vx_uint8_ptr output_base = output_ptr + b * out_w * out_h * output_item_size + output_batch_offset;
        vx_uint8_ptr input_base = input_ptr + b * in_w * in_h * stride_w * stride_h * input_item_size + input_batch_offset;

        for (s_h = 0; s_h < stride_h; s_h++)
        {
            for (s_w = 0; s_w < stride_w; s_w++)
            {
                vx_int32 slice_in_offset = (s_h * stride_w + s_w) * in_w * in_h;

                for (j = 0; j < in_h; j++)
                {
                    for (i = 0; i < in_w; i++)
                    {
                        vx_int32 out_x = (i * stride_w - padding_x + s_w), out_y = (j * stride_h - padding_y + s_h);
                        vx_int32 output_index = out_x  + out_y * out_w;
                        vx_int32 input_index = slice_in_offset + j * in_w + i;

                        if (out_x < 0 || out_y < 0 || out_x >= out_w || out_y >= out_h)
                            continue;

                        if (input_format == output_format)
                        {
                            memcpy(output_base + output_index * input_item_size, input_base + input_index * input_item_size, input_item_size);
                        }
                        else
                        {
                            /* output_base[output_index] = input_base[input_index];*/
                            vxnneSaveDataExt(output_format, TENSOR_QUANT_TYPE(outputs), output_index, vxnneGetDataExt(input_format, TENSOR_QUANT_TYPE(inputs), input_index, input_base, TENSOR_POS(inputs), TENSOR_TF_ZEROPOINT(inputs), TENSOR_TF_SCALE(inputs)), output_base, 0, TENSOR_TF_ZEROPOINT(outputs), TENSOR_TF_SCALE(outputs), TENSOR_POS(outputs));

                        }
                    }
                }
            }
        }
    }

    return status;
}

VX_PRIVATE_API vx_status vxnneExecuteSWDeConv_Reshuffle_DeInilition(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_deconvolution_reshuffle_operation reshuffle_operation   = (vxnne_deconvolution_reshuffle_operation)operation;

    if (reshuffle_operation->stride_x)
        vxReleaseScalar(&reshuffle_operation->stride_x);

    if (reshuffle_operation->stride_y)
        vxReleaseScalar(&reshuffle_operation->stride_y);

    if (reshuffle_operation->padding_x_left)
        vxReleaseScalar(&reshuffle_operation->padding_x_left);

    if (reshuffle_operation->padding_x_right)
        vxReleaseScalar(&reshuffle_operation->padding_x_right);

    if (reshuffle_operation->padding_y_top)
        vxReleaseScalar(&reshuffle_operation->padding_y_top);

    if (reshuffle_operation->padding_y_bottom)
        vxReleaseScalar(&reshuffle_operation->padding_y_bottom);

    return status;
}

VX_PRIVATE_API vx_status vxnneExecuteSWDeConv_Conv_DeInilition(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_convolution_operation conv_operation   = (vxnne_convolution_operation)operation;

    if (conv_operation->downScaleSizeRounding)
        vxReleaseScalar(&conv_operation->downScaleSizeRounding);

    return status;
}

VX_PRIVATE_API vx_status vxnneExecuteSWDeConv_UpSample_DeInilition(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_deconvolution_operation deconv_operation   = (vxnne_deconvolution_operation)operation;

    if (deconv_operation->inputs)
        vxoTensor_ReleaseTensor(&deconv_operation->inputs);

    return status;
}

VX_PRIVATE_API vx_status vxnneExecuteSWDeConvolution(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_deconvolution_operation deconvOperation   = (vxnne_deconvolution_operation)operation;

    vx_tensor inputs        = deconvOperation->inputs;
    vx_tensor weights       = deconvOperation->weights;
    vx_tensor bias          = (deconvOperation->biases != VX_NULL)?deconvOperation->biases:VX_NULL;
    vx_tensor outputs       = deconvOperation->outputs;
    vx_int32 padding_x      = deconvOperation->padding_x_left->value->n32;
    vx_int32 padding_x_right= deconvOperation->padding_x_right->value->n32;
    vx_int32 padding_y      = deconvOperation->padding_y_top->value->n32;
    vx_int32 padding_y_bottom = deconvOperation->padding_y_bottom->value->n32;
    vx_enum rounding_policy = VX_ROUND_POLICY_TO_NEAREST_EVEN;
    vx_int32 a_x            = deconvOperation->a_x->value->n32;
    vx_int32 a_y            = deconvOperation->a_y->value->n32;

    vx_type_e input_format  = (vx_type_e)(TENSOR_DATA_TYPE(inputs));
    vx_type_e output_format = (vx_type_e)(TENSOR_DATA_TYPE(outputs));
    vx_type_e weight_format = (vx_type_e)(TENSOR_DATA_TYPE(weights));
    vx_int8   input_fixPointPos = TENSOR_POS(inputs);
    vx_int8   output_fixPointPos = TENSOR_POS(outputs);
    vx_int8   weights_fixPointPos = TENSOR_POS(weights);
    vx_float32   input_scale = TENSOR_TF_SCALE(inputs);
    vx_float32   output_scale = TENSOR_TF_SCALE(outputs);
    vx_float32   weights_scale = TENSOR_TF_SCALE(weights);
    vx_int32   input_zp = TENSOR_TF_ZEROPOINT(inputs);
    vx_int32   output_zp = TENSOR_TF_ZEROPOINT(outputs);
    vx_int32   weights_zp = TENSOR_TF_ZEROPOINT(weights);
    vx_int8   bias_fixPointPos = (bias != VX_NULL) ? TENSOR_POS(bias) : 0;
    vx_float32   bias_scale = (bias != VX_NULL) ? TENSOR_TF_SCALE(bias) : 0;
    vx_int32   bias_zp = (bias != VX_NULL) ? TENSOR_TF_ZEROPOINT(bias) : 0;

    vx_int32 group = (deconvOperation->group->value->n32 > 0)?deconvOperation->group->value->n32:1;
    vx_int32 g = 0;
    vx_int32 kernel_size_x = weights->viewRegion.viewEnds[INDEX_WIDTH] - weights->viewRegion.viewStarts[INDEX_WIDTH];
    vx_int32 kernel_size_y = weights->viewRegion.viewEnds[INDEX_HEIGHT] - weights->viewRegion.viewStarts[INDEX_HEIGHT];
    vx_int32 kernel_size_c = weights->viewRegion.viewEnds[INDEX_DEPTH] - weights->viewRegion.viewStarts[INDEX_DEPTH];
    vx_int32 in_h = inputs->viewRegion.viewEnds[INDEX_HEIGHT] - inputs->viewRegion.viewStarts[INDEX_HEIGHT];
    vx_int32 in_w = inputs->viewRegion.viewEnds[INDEX_WIDTH] - inputs->viewRegion.viewStarts[INDEX_WIDTH];
    vx_int32 out_h = outputs->viewRegion.viewEnds[INDEX_HEIGHT] - outputs->viewRegion.viewStarts[INDEX_HEIGHT];
    vx_int32 out_w = outputs->viewRegion.viewEnds[INDEX_WIDTH] - outputs->viewRegion.viewStarts[INDEX_WIDTH];

    vx_int32 stride_w = (vx_int32)vxnneRound(out_w * 1.0f/ in_w, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING);
    vx_int32 stride_h = (vx_int32)vxnneRound(out_h * 1.0f / in_h, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING);

    vx_int32 conv_out_channels = inputs->viewRegion.viewEnds[INDEX_DEPTH] - inputs->viewRegion.viewStarts[INDEX_DEPTH];
    vx_int32 conv_in_channels = outputs->viewRegion.viewEnds[INDEX_DEPTH] - outputs->viewRegion.viewStarts[INDEX_DEPTH];
    vx_int32 kernel_dim = kernel_size_x * kernel_size_y * kernel_size_c;
    vx_int32 conv_in_spatial_dim = in_h * in_w;

    vx_int32 conv_out_spatial_dim = in_h * in_w;
    vx_int32 input_offset = conv_out_channels * conv_in_spatial_dim / group;
    vx_int32 weight_offset = conv_out_channels * kernel_dim / group;
    vx_int32 bias_offset = conv_out_channels / group;
    vx_uint8_ptr col_ptr = (vx_uint8_ptr)vxAllocateAndZeroMemory(sizeof(vx_float32) * kernel_dim * conv_out_spatial_dim * group);
    vx_float32_ptr tmp_ptr = (vx_float32_ptr)vxAllocateAndZeroMemory(sizeof(vx_float32) * out_w * out_h);

    vx_uint8_ptr output_ptr = VX_NULL;
    vx_uint8_ptr weight_ptr = VX_NULL;
    vx_uint8_ptr input_ptr = VX_NULL/*(conv_out_channels/group)*height*width*/;

    vx_bool input_stage = vx_false_e, output_stage = vx_false_e;
    vx_uint8_ptr bias_ptr = VX_NULL;
    vx_type_e bias_format  = VX_TYPE_INVALID;
    if(bias)
    {
        bias_format = (vx_type_e)(TENSOR_DATA_TYPE(bias));
        vxnneGetTensorMemeory(bias, (vx_ptr_ptr)&bias_ptr, input_stage, vx_false_e);
    }

    vxnneGetTensorMemeory(inputs, (vx_ptr_ptr)&input_ptr, input_stage, vx_false_e);
    vxnneGetTensorMemeory(weights, (vx_ptr_ptr)&weight_ptr, input_stage, vx_false_e);
    vxnneGetTensorMemeory(outputs, (vx_ptr_ptr)&output_ptr, output_stage, vx_true_e);
    gcoOS_MemFill(col_ptr, 0, sizeof(vx_float32) * kernel_dim * conv_out_spatial_dim * group);

    for(g = 0; g < group; ++g){
        vx_uint8_ptr g_col_ptr = sizeof(vx_float32) * kernel_dim * conv_out_spatial_dim * g + col_ptr;
        vx_uint8_ptr g_weight_ptr = weight_offset * vxnneGetTypeSize(weight_format) * g + weight_ptr;
        vx_uint8_ptr g_bias_ptr = bias_offset * vxnneGetTypeSize(bias_format) * g + bias_ptr;
        vx_uint8_ptr g_input_ptr =  input_offset * vxnneGetTypeSize(input_format) * g + input_ptr;
        vx_uint8_ptr g_output_ptr = conv_out_channels * out_h * out_w / group * vxnneGetTypeSize(output_format) * g + output_ptr;
#define CblasTrans 1
#define CblasNoTrans 0

        /* vxnneLayerSW_gemm(
                vx_int32 TA, vx_int32 TB,
                vx_int32 M, vx_int32 N, vx_int32 K, vx_float32 ALPHA,
                vx_uint8_ptr A, vx_int32 lda,
                vx_uint8_ptr B, vx_int32 ldb,
                vx_float32 BETA,
                vx_uint8_ptr C, vx_int32 ldc);

            CblasTrans  : 1;
            CblasNoTrans: 0;
            int lda = (TransA == CblasNoTrans) ? K : M;
            int ldb = (TransB == CblasNoTrans) ? N : K;


            vxnneLayerSW_col2im(
                vx_uint8_ptr data_col, const vx_int32 channels,
                const vx_int32 height, const vx_int32 width, const vx_int32 kernel_h, const vx_int32 kernel_w,
                const vx_int32 pad_h, const vx_int32 pad_w,
                const vx_int32 stride_h, const vx_int32 stride_w,
                const vx_int32 dilation_h, const vx_int32 dilation_w,
                vx_uint8_ptr data_im);

        */
        if(input_format == VX_TYPE_UINT8 && weight_format == VX_TYPE_UINT8)
        {
            vxnneLayerSW_gemm_u8(weight_format, input_format, VX_TYPE_FLOAT32, rounding_policy, weights_zp, weights_scale, input_zp, input_scale, 0,
                    CblasTrans, CblasNoTrans,
                    kernel_dim, conv_in_spatial_dim, (conv_out_channels/group), 1,
                    g_weight_ptr, kernel_dim,
                    g_input_ptr, conv_in_spatial_dim,
                    0,
                    g_col_ptr, conv_in_spatial_dim);
        }
        else
        {
            vxnneLayerSW_gemm(weight_format, input_format, VX_TYPE_FLOAT32, rounding_policy, weights_fixPointPos, input_fixPointPos, 0,
                CblasTrans, CblasNoTrans,
                kernel_dim, conv_in_spatial_dim, (conv_out_channels/group), 1,
                g_weight_ptr, kernel_dim,
                g_input_ptr, conv_in_spatial_dim,
                0,
                g_col_ptr, conv_in_spatial_dim);
        }

        if(output_format == VX_TYPE_UINT8)
        {
            vxnneLayerSW_col2im_add_bias_u8(VX_TYPE_FLOAT32, output_format, bias_format,rounding_policy, 0, output_zp, output_scale, bias_zp, bias_scale,
                    g_col_ptr, bias_ptr ? g_bias_ptr:NULL,conv_in_channels/group,
                    out_h, out_w, kernel_size_y, kernel_size_x,
                    padding_y, padding_x,
                    padding_y_bottom, padding_x_right,
                    stride_h, stride_w,
                    a_x, a_y,
                    g_output_ptr,tmp_ptr);
        }
        else
        {
            vxnneLayerSW_col2im_add_bias(VX_TYPE_FLOAT32, output_format, bias_format,rounding_policy, 0, output_fixPointPos,bias_fixPointPos,
                g_col_ptr, bias_ptr ? g_bias_ptr:NULL,conv_in_channels/group,
                out_h, out_w, kernel_size_y, kernel_size_x,
                padding_y, padding_x,
                padding_y_bottom, padding_x_right,
                stride_h, stride_w,
                a_x, a_y,
                g_output_ptr,tmp_ptr);
        }
    }

    vxFree(col_ptr);
    vxFree(tmp_ptr);

    if (input_stage)
    {
        vxFree(input_ptr);
        vxFree(weight_ptr);
    }

    if (output_stage)
    {
        vx_uint32 tensor_size = 0;
        vx_ptr outputs_tensor_logical = VX_NULL;
        vxoTensor_GetTensorSize(outputs, &tensor_size);
        vxoTensor_GetTensorViewMemory(outputs, &outputs_tensor_logical, VX_NULL);

        gcoOS_MemCopy(outputs_tensor_logical, output_ptr, tensor_size);

        vxFree(output_ptr);
    }

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNDeConvolutionLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNDeConvolutionLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNDeConvolutionLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

typedef enum _gcoNNDeConv_Mode
{
    gcoNNE_DECONV_MODE_SW,
    gcoNNE_DECONV_MODE_SW1,
    gcoNNE_DECONV_MODE_SHADER,
    gcoNNE_DECONV_MODE_SHADER1,
    gcoNNE_DECONV_MODE_NNE_TP,
}
gcoNNDeConv_Mode;

VX_PRIVATE_API vx_status vxoNNDeConvolution_GetPad(vx_int32 in, vx_int32 out, vx_int32 stride, vx_int32 kernel_reshuffle, vx_int32 pad_head, vx_int32 pad_tail,
                                    vx_int32_ptr decov_output, vx_int32_ptr kernel_resuffle_pad_head, vx_int32_ptr kernel_resuffle_pad_tail,
                                    vx_int32_ptr upsample_pad, vx_uint32_ptr sample_output, vx_uint32_ptr upsample_output)
{

            if ((out < in * stride))
            {
                *decov_output = in;

                *kernel_resuffle_pad_head = ((*decov_output - 1) + kernel_reshuffle - in) / 2;
                *kernel_resuffle_pad_tail = ((*decov_output - 1) + kernel_reshuffle - in) - *kernel_resuffle_pad_head;

                *sample_output = *decov_output;

                *upsample_output = *decov_output * stride;

                *upsample_pad = (pad_tail > pad_head)?(vx_int32)ceilf((*decov_output * stride - out)/2.0f):(*decov_output * stride - out)/2;
            }
            else if ((out > in * stride))
            {
                *decov_output = gcmALIGN_NP2(out, stride) / stride;

                *kernel_resuffle_pad_head = ((*decov_output - 1) + kernel_reshuffle - in) / 2;
                *kernel_resuffle_pad_tail = ((*decov_output - 1) + kernel_reshuffle - in) - *kernel_resuffle_pad_head;

                *sample_output = *decov_output;

                *upsample_output = *decov_output * stride;

                *upsample_pad = (vx_int32)ceilf((*decov_output * stride - out)/2.0f);
            }
            else
            {
                *upsample_pad = *kernel_resuffle_pad_head * (stride - 1);
            }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNDeConvolutionLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_tensor  weights                    = (vx_tensor)parameters[1];
    vx_tensor  bias                       = (vx_tensor)parameters[2];
    vx_scalar  padding_x                  = (vx_scalar)parameters[3];
    vx_scalar  padding_x_right            = (vx_scalar)parameters[4];
    vx_scalar  padding_y                  = (vx_scalar)parameters[5];
    vx_scalar  padding_y_bottom           = (vx_scalar)parameters[6];
    vx_scalar  overflow_policy            = (vx_scalar)parameters[7];
    vx_scalar  rounding_policy            = (vx_scalar)parameters[8];
    vx_scalar  a_x                        = (vx_scalar)parameters[9];
    vx_scalar  a_y                        = (vx_scalar)parameters[10];
    vx_scalar  group                      = (vx_scalar)parameters[11];
    vx_scalar  stride_w_s                 = (vx_scalar)parameters[12];
    vx_scalar  stride_h_s                 = (vx_scalar)parameters[13];
    vx_tensor  outputs                    = (vx_tensor)parameters[num - 1];
    gcoNNDeConv_Mode deconvolution_mode   = /* gcoNNE_DECONV_MODE_SW; gcoNNE_DECONV_MODE_SW1; */ gcoNNE_DECONV_MODE_NNE_TP;
    vx_uint32  batchCount                 = TENSOR_SIZE_INDEX(inputs, 3);
    vx_bool depthwise = vx_false_e;

    vxnne_deconvolution_layer  deconvolutionLayer = VX_NULL;
    vx_type_e  inputFormat      = (vx_type_e)TENSOR_DATA_TYPE(inputs);
    vx_type_e  outputFormat     = (vx_type_e)TENSOR_DATA_TYPE(outputs);

    vx_int32 in_h = TENSOR_SIZE_INDEX(inputs, 1);
    vx_int32 in_w = TENSOR_SIZE_INDEX(inputs, 0);
    vx_int32 out_h = TENSOR_SIZE_INDEX(outputs, 1);
    vx_int32 out_w = TENSOR_SIZE_INDEX(outputs, 0);
    vx_int32 pad_x      = padding_x->value->n32;
    vx_int32 pad_y      = padding_y->value->n32;
    vx_int32 pad_x_right= padding_x_right->value->n32;
    vx_int32 pad_y_bottom = padding_y_bottom->value->n32;
    vx_int32 kernel_size_x = TENSOR_SIZE_INDEX(weights, 0);
    vx_int32 kernel_size_y = TENSOR_SIZE_INDEX(weights, 1);
    /* de-covolution*/
    vx_int32 stride_w = (stride_w_s != VX_NULL) ? (stride_w_s->value->n32) : (vx_int32)vxnneRound((out_w - kernel_size_x - a_x->value->n32 + pad_x + pad_x_right) * 1.0f / (in_w - 1), VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING);
    vx_int32 stride_h = (stride_h_s != VX_NULL) ? (stride_h_s->value->n32) : (vx_int32)vxnneRound((out_h - kernel_size_y - a_y->value->n32 + pad_y + pad_y_bottom) * 1.0f / (in_h - 1), VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING);

    vx_int32 kernel_reshuffle_width = gcmALIGN_NP2(kernel_size_x, stride_w)/stride_w, kernel_reshuffle_height = gcmALIGN_NP2(kernel_size_y, stride_h)/stride_h;
    vx_int32 kernel_reshuffle_pad_x_right = (in_w - 1) * stride_w - (out_w + gcmMAX(pad_x, pad_x_right) - kernel_reshuffle_width * stride_w);
    vx_int32 kernel_reshuffle_pad_x_left = gcmMAX(pad_x, pad_x_right);
    vx_int32 kernel_reshuffle_pad_y_bottom = (in_h - 1) * stride_h - (out_h + gcmMAX(pad_y, pad_y_bottom) - kernel_reshuffle_height * stride_h);
    vx_int32 kernel_reshuffle_pad_y_top = gcmMAX(pad_y, pad_y_bottom);
    vx_int32 decov_output_w = in_w + kernel_reshuffle_pad_x_left + kernel_reshuffle_pad_x_right - kernel_reshuffle_width + 1;
    vx_int32 decov_output_h = in_h + kernel_reshuffle_pad_y_top + kernel_reshuffle_pad_y_bottom - kernel_reshuffle_height + 1;

    vx_bool need_upsample = (in_w == out_w && in_h == out_h) ? vx_false_e : vx_true_e;
    need_upsample = ((decov_output_w != out_w) || (decov_output_h != out_h))?vx_true_e:vx_false_e;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }
    gcoOS_Allocate(gcvNULL, sizeof(vxnne_deconvolution_layer_s), (gctPOINTER*)&deconvolutionLayer);
    if (!deconvolutionLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    gcoOS_ZeroMemory(deconvolutionLayer, sizeof(vxnne_deconvolution_layer_s));

    vxnneLayer_Initialize(&deconvolutionLayer->base,
                          "DeConvolutionLayer",
                          node,
                          vxmOPERATION_COUNT(deconvolutionLayer),
                          deconvolutionLayer->operations,
                          VX_NULL);

    {
        vx_uint32 group_size     = 0;
        vx_bool   dataTypeFlg    = vx_false_e;
        vx_uint32 kx             = TENSOR_VIEW_SIZE_INDEX(weights, 0);
        vx_uint32 ky             = TENSOR_VIEW_SIZE_INDEX(weights, 1);
        vx_uint32 src_width      = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
        vx_uint32 src_height     = TENSOR_VIEW_SIZE_INDEX(inputs, 1);
        vx_uint32 src_depth      = TENSOR_VIEW_SIZE_INDEX(inputs, 2);
        vx_uint32 dst_width      = TENSOR_VIEW_SIZE_INDEX(outputs, 0);
        vx_uint32 dst_height     = TENSOR_VIEW_SIZE_INDEX(outputs, 1);

        if((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
            || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8)
            || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8 && (kx == 2 && ky ==2))
            || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16 && (kx == 2 && ky ==2)))
            dataTypeFlg = vx_true_e;

        group_size = group->value->u32;
        depthwise = (group_size == src_depth && group_size != 1) ? vx_true_e : vx_false_e;

        if (((kx == 2 && ky == 2) || (kx == 4 && ky == 4))
            && src_width * 2 == dst_width && src_height * 2 == dst_height
            && group_size == src_depth && dataTypeFlg && bias != NULL)
            deconvolution_mode = gcoNNE_DECONV_MODE_SHADER;
    }
    /* check NN input and output format according to need_upsample in gcoNNE_DECONV_MODE_NNE_TP*/
    if (((need_upsample && !vxnneIsNNSupportFormat(vxGetContext((vx_reference)inputs), inputs, VX_NULL, VX_NULL)) ||
        (!need_upsample && !vxnneIsNNSupportFormat(vxGetContext((vx_reference)inputs), inputs, VX_NULL, outputs))) &&
        (deconvolution_mode != gcoNNE_DECONV_MODE_SHADER))
    {
        vxError("Shander not support this format, goto cpu path temporarily. function %s line %d\n", __FUNCTION__, __LINE__);
        deconvolution_mode = gcoNNE_DECONV_MODE_SW;
    }

    {
        gctSTRING envctrl = gcvNULL;
        if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "USE_DECONV_MODE_SW", &envctrl)) && envctrl)
        {
            vx_int32 flag = atoi(envctrl);
            if(flag)
                deconvolution_mode = gcoNNE_DECONV_MODE_SW;
        }
    }

    switch (deconvolution_mode)
    {
    case gcoNNE_DECONV_MODE_SHADER:
        {
            vxnne_shader_executable shaderExecutable = VX_NULL;
            shaderExecutable = vxnneDeConvolutionShaderExecutable(node->base.context, VXNNE_KERNEL_DECONVOLUTION, &node->kernelAttributes.borderMode,
                inputs,
                weights,
                bias,
                padding_x,
                padding_y,
                overflow_policy,
                rounding_policy,
                a_x,
                a_y,
                group,
                outputs);
            if (!shaderExecutable)
            {
                status = VX_FAILURE;
                goto exit;
            }

            status = vxnneShaderOperation_Initialize(&deconvolutionLayer->deconvolution_sh_operation,
                                          &deconvolutionLayer->base,
                                          VXNNE_OPERATOR_DECONVOLUTION,
                                          batchCount,
                                          shaderExecutable);

            if (status != VX_SUCCESS) goto exit;

            vxnneOperation_AddReference(&deconvolutionLayer->deconvolution_sh_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&deconvolutionLayer->deconvolution_sh_operation.base, (vx_reference)weights, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&deconvolutionLayer->deconvolution_sh_operation.base, (vx_reference)bias, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&deconvolutionLayer->deconvolution_sh_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            vxnneLayer_SetOperation(
                &deconvolutionLayer->base,
                &deconvolutionLayer->deconvolution_sh_operation.base,
                0);

            break;
        }

    case gcoNNE_DECONV_MODE_SW:
        {
            vxnneOperation_Initialize(&deconvolutionLayer->deconvolution_sw_operation.base,
                                      &deconvolutionLayer->base,
                                      VXNNE_OPERATION_TARGET_SW,
                                      VXNNE_OPERATOR_DECONVOLUTION,
                                      vxnneExecuteSWDeConvolution,
                                      VX_NULL,
                                      batchCount,
                                      0);

            vxnneLayer_SetOperation(
                &deconvolutionLayer->base,
                &deconvolutionLayer->deconvolution_sw_operation.base,
                0);

            deconvolutionLayer->deconvolution_sw_operation.inputs           = inputs;
            deconvolutionLayer->deconvolution_sw_operation.weights          = weights;
            deconvolutionLayer->deconvolution_sw_operation.biases           = bias;
            deconvolutionLayer->deconvolution_sw_operation.padding_x_left   = padding_x;
            deconvolutionLayer->deconvolution_sw_operation.padding_x_right  = padding_x_right;
            deconvolutionLayer->deconvolution_sw_operation.padding_y_top    = padding_y;
            deconvolutionLayer->deconvolution_sw_operation.padding_y_bottom = padding_y_bottom;
            deconvolutionLayer->deconvolution_sw_operation.overflow_policy  = overflow_policy;
            deconvolutionLayer->deconvolution_sw_operation.rounding_policy  = rounding_policy;
            deconvolutionLayer->deconvolution_sw_operation.a_x              = a_x;
            deconvolutionLayer->deconvolution_sw_operation.a_y              = a_y;
            deconvolutionLayer->deconvolution_sw_operation.group            = group;
            deconvolutionLayer->deconvolution_sw_operation.outputs          = outputs;

            vxnneOperation_AddReference(&deconvolutionLayer->deconvolution_sw_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&deconvolutionLayer->deconvolution_sw_operation.base, (vx_reference)weights, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&deconvolutionLayer->deconvolution_sw_operation.base, (vx_reference)bias, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&deconvolutionLayer->deconvolution_sw_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            break;
        }

    case gcoNNE_DECONV_MODE_SW1:
    case gcoNNE_DECONV_MODE_NNE_TP:
        {
            vx_int32 index = 0, idx = 0;

            vx_type_e output_format = (vx_type_e)(TENSOR_DATA_TYPE(outputs));
            vx_type_e weight_format = (vx_type_e)(TENSOR_DATA_TYPE(weights));

            vx_int32 kernel_channel = TENSOR_SIZE_INDEX(weights, 2);
            vx_int32 kernel_batch = TENSOR_SIZE_INDEX(weights, 3);

            vx_bool tp_upsample = (deconvolution_mode == gcoNNE_DECONV_MODE_NNE_TP)? vx_true_e : vx_false_e;
            vx_int32 upsample_pad_x = 0, upsample_pad_y = 0;
            vx_bool need_clip = vx_false_e;
            vx_int32 s = 1;

            vx_context context = vxGetContext((vx_reference)inputs);
            vx_tensor sample_output = VX_NULL, sampled_output = VX_NULL, reshuffled_weights = VX_NULL, reshuffled_bias = VX_NULL;

            vx_uint32 size[][4] = {
                 /*
                  * {2, 2, 21, 84},
                  * {17, 17, 84, 1},
                  */

                 {kernel_reshuffle_width, kernel_reshuffle_height, kernel_batch, stride_w * stride_h * kernel_channel}, /* reshuffled_weights */
                 {decov_output_w, decov_output_h, stride_w * stride_h * kernel_channel, batchCount}, /* sample_output */
                 {1, 1, 1, kernel_channel * stride_w * stride_h}, /* reshuffled_bias */
                 { decov_output_w * stride_w, decov_output_h * stride_h, kernel_channel, batchCount}, /* upsampled_output */
            };
            vx_tensor_create_params_t tensor_create_params;

            vx_int8 weight_fixed_point_pos = TENSOR_POS(weights);
            vx_int8 output_fixed_point_pos = TENSOR_POS(outputs);

            vx_bool tfQuant = vx_false_e;

            if (depthwise)
            {
                size[0][3] = stride_w * stride_h * kernel_batch;
                size[1][2] *= kernel_batch;
                size[3][2] = kernel_batch;
            }

            vxoNNDeConvolution_GetPad(in_w, out_w, stride_w, kernel_reshuffle_width, pad_x, pad_x_right,
                &decov_output_w, &kernel_reshuffle_pad_x_left, &kernel_reshuffle_pad_x_right,
                &upsample_pad_x, &size[1][0], &size[3][0]);

            vxoNNDeConvolution_GetPad(in_h, out_h, stride_h, kernel_reshuffle_height, pad_y, pad_y_bottom,
                &decov_output_h, &kernel_reshuffle_pad_y_top, &kernel_reshuffle_pad_y_bottom,
                &upsample_pad_y, &size[1][1], &size[3][1]);
            need_clip = ((upsample_pad_x > 0) || (upsample_pad_y > 0) || (out_w < in_w * stride_w) || (out_h < in_h * stride_h)) ? vx_true_e : vx_false_e;

            if (weight_format == VX_TYPE_UINT8 && output_format == VX_TYPE_UINT8 && TENSOR_QUANT_TYPE(weights) == VX_QUANT_AFFINE_SCALE && TENSOR_QUANT_TYPE(outputs) == VX_QUANT_AFFINE_SCALE)
            {
                tfQuant = vx_true_e;
            }

            gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
            tensor_create_params.num_of_dims = 4;
            tensor_create_params.sizes = size[1];
            tensor_create_params.data_format = tfQuant ? VX_TYPE_UINT8: output_format;
            tensor_create_params.quant_format = tfQuant ? VX_QUANT_AFFINE_SCALE : TENSOR_QUANT_TYPE(outputs);;
            if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
            {
                tensor_create_params.quant_data.dfp.fixed_point_pos = output_fixed_point_pos;
            }
            else
            {
                tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(outputs);
                tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(outputs);
            }

            if (need_upsample)
            {

                sample_output = vxoTensor_CreateTensor(context, node->graph, &tensor_create_params, vx_true_e);
                if (sample_output == VX_NULL)
                {
                    vxError("vxoTensor_CreateTensor fail at function %s, line %d", __FUNCTION__, __LINE__);
                    status = VX_ERROR_NO_MEMORY;
                    goto exit;
                }

                if (need_clip)
                {
                    tensor_create_params.num_of_dims = 4;
                    tensor_create_params.sizes = size[3];
                    sampled_output = vxoTensor_CreateTensor(context, node->graph, &tensor_create_params, vx_true_e);
                    if (sampled_output == VX_NULL)
                    {
                        vxError("vxoTensor_CreateTensor fail at function %s, line %d", __FUNCTION__, __LINE__);
                        status = VX_ERROR_NO_MEMORY;
                        goto exit;
                    }

                }
            }

            gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
            tensor_create_params.num_of_dims = 4;
            tensor_create_params.sizes = size[0];
            tensor_create_params.data_format = tfQuant ? VX_TYPE_UINT8: weight_format;
            tensor_create_params.quant_format = tfQuant ? VX_QUANT_AFFINE_SCALE : TENSOR_QUANT_TYPE(weights);
            if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
            {
                tensor_create_params.quant_data.dfp.fixed_point_pos = weight_fixed_point_pos;
            }
            else
            {
                tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(weights);
                tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(weights);
            }

            reshuffled_weights = vxoTensor_CreateTensor(context, node->graph, &tensor_create_params, vx_false_e);
            if (reshuffled_weights == VX_NULL)
            {
                vxError("vxoTensor_CreateTensor fail at function %s, line %d", __FUNCTION__, __LINE__);
                status = VX_ERROR_NO_MEMORY;
                goto exit;
            }

            if (bias != NULL)
            {
                gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
                tensor_create_params.num_of_dims = 4;
                tensor_create_params.sizes = size[2];
                tensor_create_params.data_format = TENSOR_DATA_TYPE(bias);;
                tensor_create_params.quant_format = tfQuant ? VX_QUANT_AFFINE_SCALE : TENSOR_QUANT_TYPE(bias);
                if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
                {
                    tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(bias);
                }
                else
                {
                    tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(bias);
                    tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(bias);
                }

                reshuffled_bias = vxoTensor_CreateTensor(context, node->graph, &tensor_create_params, vx_false_e);
                if (reshuffled_bias == VX_NULL)
                {
                    vxError("vxoTensor_CreateTensor fail at function %s, line %d", __FUNCTION__, __LINE__);
                    status = VX_ERROR_NO_MEMORY;
                    goto exit;
                }

                vxoTensor_AllocateMemory(reshuffled_bias);
            }
            if (deconvolution_mode == gcoNNE_DECONV_MODE_SW1)
            {
                vx_enum downScaleSizeRounding = VX_NN_DS_SIZE_ROUNDING_FLOOR;

                /* Initialize reshuffle weights operation */
                vxnneOperation_Initialize(&deconvolutionLayer->deconvolution_sw1_reshuffle_operation.base,
                                          &deconvolutionLayer->base,
                                          VXNNE_OPERATION_TARGET_SW,
                                          VXNNE_OPERATOR_DECONVOLUTION,
                                          vxnneExecuteSWDeConv_ReshuffleWeights,
                                          vxnneExecuteSWDeConv_Reshuffle_DeInilition,
                                          batchCount,
                                          0);
                vxnneLayer_SetOperation(
                    &deconvolutionLayer->base,
                    &deconvolutionLayer->deconvolution_sw1_reshuffle_operation.base,
                    index ++);

                deconvolutionLayer->deconvolution_sw1_reshuffle_operation.inputs           = inputs;
                deconvolutionLayer->deconvolution_sw1_reshuffle_operation.weights          = weights;
                deconvolutionLayer->deconvolution_sw1_reshuffle_operation.bias             = bias;
                deconvolutionLayer->deconvolution_sw1_reshuffle_operation.stride_x         = vxCreateScalar(context, VX_TYPE_INT32, &stride_w);
                deconvolutionLayer->deconvolution_sw1_reshuffle_operation.stride_y         = vxCreateScalar(context, VX_TYPE_INT32, &stride_h);
                deconvolutionLayer->deconvolution_sw1_reshuffle_operation.a_x              = a_x;
                deconvolutionLayer->deconvolution_sw1_reshuffle_operation.a_y              = a_y;
                deconvolutionLayer->deconvolution_sw1_reshuffle_operation.group            = group;
                deconvolutionLayer->deconvolution_sw1_reshuffle_operation.outputs          = need_upsample ? sample_output : outputs;
                deconvolutionLayer->deconvolution_sw1_reshuffle_operation.reshuffled_weights= reshuffled_weights;
                deconvolutionLayer->deconvolution_sw1_reshuffle_operation.reshuffled_biases = reshuffled_bias;

                vxnneOperation_AddReference(&deconvolutionLayer->deconvolution_sw1_reshuffle_operation.base, (vx_reference)weights, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&deconvolutionLayer->deconvolution_sw1_reshuffle_operation.base, (vx_reference)bias, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&deconvolutionLayer->deconvolution_sw1_reshuffle_operation.base, (vx_reference)reshuffled_weights, VXNNE_OPERATION_REFENRENCE_OUTPUT);
                vxnneOperation_AddReference(&deconvolutionLayer->deconvolution_sw1_reshuffle_operation.base, (vx_reference)reshuffled_bias, VXNNE_OPERATION_REFENRENCE_OUTPUT);


                /* Initialize covolution operation */
                vxnneOperation_Initialize(&deconvolutionLayer->deconvolution_sw1_convolution_operation.base,
                                          &deconvolutionLayer->base,
                                          VXNNE_OPERATION_TARGET_SW,
                                          VXNNE_OPERATOR_DECONVOLUTION,
                                          vxnneExecuteSWConvolution,
                                          vxnneExecuteSWDeConv_Conv_DeInilition,
                                          batchCount,
                                          0);

                vxnneLayer_SetOperation(
                    &deconvolutionLayer->base,
                    &deconvolutionLayer->deconvolution_sw1_convolution_operation.base,
                    index++);

                deconvolutionLayer->deconvolution_sw1_convolution_operation.inputs                  = inputs;
                deconvolutionLayer->deconvolution_sw1_convolution_operation.weights                 = reshuffled_weights;
                deconvolutionLayer->deconvolution_sw1_convolution_operation.biases                  = reshuffled_bias;
                deconvolutionLayer->deconvolution_sw1_convolution_operation.padX                    = vxCreateScalar(context, VX_TYPE_INT32, &kernel_reshuffle_pad_x_left);
                deconvolutionLayer->deconvolution_sw1_convolution_operation.padY                    = vxCreateScalar(context, VX_TYPE_INT32, &kernel_reshuffle_pad_y_top);
                deconvolutionLayer->deconvolution_sw1_convolution_operation.strideX                 = vxCreateScalar(context, VX_TYPE_INT32, &s);
                deconvolutionLayer->deconvolution_sw1_convolution_operation.strideY                 = vxCreateScalar(context, VX_TYPE_INT32, &s);
                deconvolutionLayer->deconvolution_sw1_convolution_operation.overflowPolicy          = overflow_policy;
                deconvolutionLayer->deconvolution_sw1_convolution_operation.roundingPolicy          = rounding_policy;
                deconvolutionLayer->deconvolution_sw1_convolution_operation.downScaleSizeRounding   = vxCreateScalar(context, VX_TYPE_ENUM, &downScaleSizeRounding);
                deconvolutionLayer->deconvolution_sw1_convolution_operation.outputs                 = need_upsample ? sample_output : outputs;

                vxnneOperation_AddReference(&deconvolutionLayer->deconvolution_sw1_convolution_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&deconvolutionLayer->deconvolution_sw1_convolution_operation.base, (vx_reference)reshuffled_weights, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&deconvolutionLayer->deconvolution_sw1_convolution_operation.base, (vx_reference)(need_upsample ? sample_output : outputs), VXNNE_OPERATION_REFENRENCE_OUTPUT);
            }
            else
            {
                deconvolutionLayer->deconvolution_sw1_reshuffle_operation.create_wbp                = vx_true_e;
                deconvolutionLayer->deconvolution_sw1_reshuffle_operation.inputs                    = inputs;
                deconvolutionLayer->deconvolution_sw1_reshuffle_operation.weights                   = weights;
                deconvolutionLayer->deconvolution_sw1_reshuffle_operation.bias                      = bias;
                deconvolutionLayer->deconvolution_sw1_reshuffle_operation.reshuffled_biases         = reshuffled_bias;
                deconvolutionLayer->deconvolution_sw1_reshuffle_operation.stride_x                  = vxCreateScalar(context, VX_TYPE_INT32, &stride_w);
                deconvolutionLayer->deconvolution_sw1_reshuffle_operation.stride_y                  = vxCreateScalar(context, VX_TYPE_INT32, &stride_h);
                deconvolutionLayer->deconvolution_sw1_reshuffle_operation.group                     = group;
                deconvolutionLayer->deconvolution_sw1_convolution_operation.outputs                 = need_upsample ? sample_output : outputs;
                deconvolutionLayer->deconvolution_sw1_reshuffle_operation.reshuffled_weights        = reshuffled_weights;
                deconvolutionLayer->deconvolution_sw1_reshuffle_operation.reshuffled_biases         = reshuffled_bias;
                deconvolutionLayer->deconvolution_sw1_reshuffle_operation.outputs = need_upsample ? sample_output : outputs;

                vxoTensor_AllocateMemory(deconvolutionLayer->deconvolution_sw1_reshuffle_operation.reshuffled_weights);
                if (bias != VX_NULL)
                {
                    vxoTensor_AllocateMemory(deconvolutionLayer->deconvolution_sw1_reshuffle_operation.reshuffled_biases);
                }

                status = vxnneExecuteSWDeConv_ReshuffleWeights(&deconvolutionLayer->deconvolution_sw1_reshuffle_operation.base);
                if (status != VX_SUCCESS) goto exit;

                if (deconvolutionLayer->deconvolution_sw1_reshuffle_operation.weights_biaes)
                {
                    vx_weights_biases_parameter weights_biases = deconvolutionLayer->deconvolution_sw1_reshuffle_operation.weights_biaes;

                    vxoCompressNNFirstTime(context, weights_biases, need_upsample ? sample_output : outputs);

                    /* Initialize covolution operation */
                    status = vxnneOperation_Initialize(&deconvolutionLayer->convolution_operation.base,
                                                       &deconvolutionLayer->base,
                                                       VXNNE_OPERATION_TARGET_NN,
                                                       VXNNE_OPERATOR_CONVOLUTION,
                                                       VX_NULL,
                                                       NULL,
                                                       batchCount,
                                                       NNE_COMMAND_SIZE);
                    if (status != VX_SUCCESS) goto exit;

                    vxnneLayer_SetOperation(
                        &deconvolutionLayer->base,
                        &deconvolutionLayer->convolution_operation.base,
                        index++);

                    deconvolutionLayer->convolution_operation.orig_inputs      = inputs;
                    deconvolutionLayer->convolution_operation.inputs           = inputs;
                    deconvolutionLayer->convolution_operation.weights_biases   = weights_biases;
                    deconvolutionLayer->convolution_operation.outputs          = need_upsample?sample_output: outputs;

                    vxnneOperation_AddReference(&deconvolutionLayer->convolution_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
                    vxnneOperation_AddReference(&deconvolutionLayer->convolution_operation.base, (vx_reference)(need_upsample ? sample_output : outputs), VXNNE_OPERATION_REFENRENCE_OUTPUT);
                    {
                        vx_op_param_s conv;
                        memset(&conv, 0, sizeof(vx_op_param_s));
                        conv.pad_x_left = kernel_reshuffle_pad_x_left;
                        conv.pad_x_right = kernel_reshuffle_pad_x_right;
                        conv.pad_y_top = kernel_reshuffle_pad_y_top;
                        conv.pad_y_bottom = kernel_reshuffle_pad_y_bottom;
                        conv.pad_mode = VX_PAD_CONSTANT;
                        conv.pad_const = TENSOR_TF_ZEROPOINT(inputs);
                        conv.pool_type = VX_NN_POOLING_MAX-1;
                        conv.conv_rounding_type = VX_NN_DS_SIZE_ROUNDING_FLOOR;
                        conv.enable_relu = vx_false_e;
                        conv.pool_size_x = conv.pool_size_y = 0;
                        memcpy(&deconvolutionLayer->convolution_operation.base.parameter, &conv, sizeof(vx_op_param_s));
                    }
                }
            }

            if (need_upsample)
            {
                if (tp_upsample && (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_UPSAMPLE)) &&
                    (need_clip || (!need_clip && vxnneIsTPSupportFormat(context, VX_NULL, VX_NULL, outputs))))
                {
                    vx_op_param_s conv = { 0 };
                    status = vxnneOperation_Initialize(&deconvolutionLayer->upsample_tp_operation.base,
                        &deconvolutionLayer->base,
                        VXNNE_OPERATION_TARGET_TP,
                        VXNNE_OPERATOR_UPSAMPLE,
                        VX_NULL,
                        vxnneExecuteSWDeConv_UpSample_DeInilition,
                        batchCount,
                        0);
                    if (status != VX_SUCCESS) goto exit;

                    vxnneLayer_SetOperation(
                        &deconvolutionLayer->base,
                        &deconvolutionLayer->upsample_tp_operation.base,
                        index++);

                    deconvolutionLayer->upsample_tp_operation.input = sample_output;
                    deconvolutionLayer->upsample_tp_operation.output = need_clip ? sampled_output : outputs;

                    vxnneOperation_AddReference(&deconvolutionLayer->upsample_tp_operation.base, (vx_reference)sample_output, VXNNE_OPERATION_REFENRENCE_INPUT);
                    vxnneOperation_AddReference(&deconvolutionLayer->upsample_tp_operation.base, (vx_reference)(need_clip ? sampled_output : outputs), VXNNE_OPERATION_REFENRENCE_OUTPUT);

                    conv.pad_x_left = 0;
                    conv.pad_y_top = 0;
                    conv.pool_size_x = 0;
                    conv.pool_size_y = 0;
                    conv.pool_stride = 1;
                    conv.enable_relu = vx_false_e;
                    conv.pad_mode = VX_PAD_CONSTANT;
                    conv.pad_const = 0;
                    conv.tpType = TP_UPSAMPLE;
                    conv.other_ref = gcvNULL;
                    conv.data_buff = gcvNULL;

                    memcpy(&deconvolutionLayer->upsample_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));

                    /* clip the output */
                    if (need_clip)
                    {

                        status = vxnneOperation_Initialize(&deconvolutionLayer->upsample_tp_operation_clip.base,
                            &deconvolutionLayer->base,
                            VXNNE_OPERATION_TARGET_TP,
                            VXNNE_OPERATOR_UPSAMPLE,
                            VX_NULL,
                            vxnneExecuteSWDeConv_UpSample_DeInilition,
                            batchCount,
                            0);
                        if (status != VX_SUCCESS) goto exit;

                        vxnneLayer_SetOperation(
                            &deconvolutionLayer->base,
                            &deconvolutionLayer->upsample_tp_operation_clip.base,
                            index++);

                        deconvolutionLayer->upsample_tp_operation_clip.input = sampled_output;
                        deconvolutionLayer->upsample_tp_operation_clip.output = outputs;

                        vxnneOperation_AddReference(&deconvolutionLayer->upsample_tp_operation_clip.base, (vx_reference)sampled_output, VXNNE_OPERATION_REFENRENCE_INPUT);
                        vxnneOperation_AddReference(&deconvolutionLayer->upsample_tp_operation_clip.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

                        conv.pad_x_left = upsample_pad_x;
                        conv.pad_y_top = upsample_pad_y;
                        conv.pool_size_x = 0;
                        conv.pool_size_y = 0;
                        conv.pool_stride = 1;
                        conv.enable_relu = vx_false_e;
                        conv.pad_mode = VX_PAD_CONSTANT;
                        conv.pad_const = 0;
                        conv.tpType = TP_UPSAMPLE_CLIP;
                        conv.other_ref = gcvNULL;
                        conv.data_buff = gcvNULL;

                        memcpy(&deconvolutionLayer->upsample_tp_operation_clip.base.parameter, &conv, sizeof(vx_op_param_s));

                        deconvolutionLayer->base.temp_tensors[idx++] = sampled_output;
                    }
                }
                else
                {
                    /* Initialize upsample operation */
                    vxnneOperation_Initialize(&deconvolutionLayer->deconvolution_sw1_upsample_operation.base,
                                              &deconvolutionLayer->base,
                                              VXNNE_OPERATION_TARGET_SW,
                                              VXNNE_OPERATOR_DECONVOLUTION,
                                              vxnneExecuteSWDeConv_UpSample,
                                              vxnneExecuteSWDeConv_UpSample_DeInilition,
                                              batchCount,
                                              0);

                    vxnneLayer_SetOperation(
                        &deconvolutionLayer->base,
                        &deconvolutionLayer->deconvolution_sw1_upsample_operation.base,
                        index++);

                    deconvolutionLayer->deconvolution_sw1_upsample_operation.inputs           = sample_output;

                    deconvolutionLayer->deconvolution_sw1_upsample_operation.overflow_policy  = overflow_policy;
                    deconvolutionLayer->deconvolution_sw1_upsample_operation.rounding_policy  = rounding_policy;
                    deconvolutionLayer->deconvolution_sw1_upsample_operation.a_x              = a_x;
                    deconvolutionLayer->deconvolution_sw1_upsample_operation.a_y              = a_y;
                    deconvolutionLayer->deconvolution_sw1_upsample_operation.group            = group;
                    deconvolutionLayer->deconvolution_sw1_upsample_operation.outputs          = outputs;
                    deconvolutionLayer->deconvolution_sw1_upsample_operation.padding_x_left   = vxCreateScalar(context, VX_TYPE_INT32, &upsample_pad_x);
                    deconvolutionLayer->deconvolution_sw1_upsample_operation.padding_y_top    = vxCreateScalar(context, VX_TYPE_INT32, &upsample_pad_y);

                    vxnneOperation_AddReference(&deconvolutionLayer->deconvolution_sw1_upsample_operation.base, (vx_reference)sample_output, VXNNE_OPERATION_REFENRENCE_INPUT);
                    vxnneOperation_AddReference(&deconvolutionLayer->deconvolution_sw1_upsample_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
                }

                deconvolutionLayer->base.temp_tensors[idx++]   = sample_output;
            }
            deconvolutionLayer->base.temp_tensors[idx ++]      = reshuffled_weights;
            deconvolutionLayer->base.temp_tensors[idx ++]      = reshuffled_bias;
            deconvolutionLayer->base.num_temp_tensors          = idx;

            break;
        }

    default:
        vxError("Not Support[DECONVOLUTION]!");
        break;

    }

    node->layer = &deconvolutionLayer->base;
     return status;
exit:
    if (deconvolutionLayer != NULL)
        gcoOS_Free(NULL, deconvolutionLayer);
     return status;

}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNDeConvolutionLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}

