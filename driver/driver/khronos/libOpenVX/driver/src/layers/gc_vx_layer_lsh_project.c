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
#include <layers/gc_vx_layer_lsh_project.h>


/***************************************************************************************************************************
 *                                                 LSH Projection
 ***************************************************************************************************************************/
static const vx_uint64 k0 = 0xc3a5c85c97cb3127ULL;
static const vx_uint64 k1 = 0xb492b66fbe98f273ULL;
static const vx_uint64 k2 = 0x9ae16a3b2f90404fULL;

#define Fetch Fetch64
#define Rotate vxnneExecuteSWLSHRotate64

VX_PRIVATE_API vx_uint64 Fetch64(const vx_uint8_ptr p) {
    vx_uint64 result;
    memcpy(&result, p, sizeof(result));
    return result;
}

VX_PRIVATE_API vx_uint64 vxnneExecuteSWLSHFetch32(const vx_uint8_ptr p) {
    vx_uint32 result;
    memcpy(&result, p, sizeof(result));
    return result;
}

VX_PRIVATE_API vx_uint64 vxnneExecuteSWLSHRotate64(vx_uint64 val, vx_int32 shift) {
  return shift == 0 ? val : ((val >> shift) | (val << (64 - shift)));
}

VX_PRIVATE_API vx_uint64 ShiftMix(vx_uint64 val) {
    return val ^ (val >> 47);
}

VX_PRIVATE_API vx_uint64 vxnneExecuteSWLSHHashLen16(uint64_t u, uint64_t v, uint64_t mul) {
    vx_uint64 b = 0, a = (u ^ v) * mul;
    a ^= (a >> 47);
    b = (v ^ a) * mul;
    b ^= (b >> 47);
    b *= mul;
    return b;
}

VX_PRIVATE_API vx_uint64 vxnneExecuteSWHashLen0to16(const vx_uint8_ptr s, vx_size len) {
    if (len >= 8) {
        vx_uint64 mul = k2 + len * 2;
        vx_uint64 a = Fetch(s) + k2;
        vx_uint64 b = Fetch(s + len - 8);
        vx_uint64 c = Rotate(b, 37) * mul + a;
        vx_uint64 d = (Rotate(a, 25) + b) * mul;
        return vxnneExecuteSWLSHHashLen16(c, d, mul);
    }
    if (len >= 4) {
        vx_uint64 mul = k2 + len * 2;
        vx_uint64 a = vxnneExecuteSWLSHFetch32(s);
        return vxnneExecuteSWLSHHashLen16(len + (a << 3), vxnneExecuteSWLSHFetch32(s + len - 4), mul);
    }
    if (len > 0) {
        vx_int8 a = s[0];
        vx_int8 b = s[len >> 1];
        vx_int8 c = s[len - 1];
        vx_int32 y = (vx_int32)(a) + ((vx_int32)(b) << 8);
        vx_int32 z = (vx_int32)len + ((vx_int32)(c) << 2);
        return ShiftMix(y * k2 ^ z * k0) * k2;
    }
    return k2;
}

VX_PRIVATE_API vx_uint64 vxnneExecuteSWHashLen17to32(const vx_uint8_ptr s, vx_size len) {
    vx_uint64 mul = k2 + len * 2;
    vx_uint64 a = Fetch(s) * k1;
    vx_uint64 b = Fetch(s + 8);
    vx_uint64 c = Fetch(s + len - 8) * mul;
    vx_uint64 d = Fetch(s + len - 16) * k2;
    return vxnneExecuteSWLSHHashLen16(Rotate(a + b, 43) + Rotate(c, 30) + d,
                     a + Rotate(b + k2, 18) + c, mul);
}

VX_PRIVATE_API vx_uint64 vxnneExecuteSWLSHHashLen33to64(const vx_uint8_ptr s, vx_size len) {
    vx_uint64 mul = k2 + len * 2;
    vx_uint64 a = Fetch(s) * k2;
    vx_uint64 b = Fetch(s + 8);
    vx_uint64 c = Fetch(s + len - 8) * mul;
    vx_uint64 d = Fetch(s + len - 16) * k2;
    vx_uint64 y = Rotate(a + b, 43) + Rotate(c, 30) + d;
    vx_uint64 z = vxnneExecuteSWLSHHashLen16(y, a + Rotate(b + k2, 18) + c, mul);
    vx_uint64 e = Fetch(s + 16) * mul;
    vx_uint64 f = Fetch(s + 24);
    vx_uint64 g = (y + Fetch(s + len - 32)) * mul;
    vx_uint64 h = (z + Fetch(s + len - 24)) * mul;
    return vxnneExecuteSWLSHHashLen16(Rotate(e + f, 43) + Rotate(g, 30) + h,
                     e + Rotate(f + a, 18) + g, mul);
}
VX_PRIVATE_API vx_int64 vxnneExecuteSWLSHHash64(vx_uint8_ptr s, vx_size lenght)
{
    if (lenght <= 32) {
        if (lenght <= 16) {
            return vxnneExecuteSWHashLen0to16(s, lenght);
        } else {
            return vxnneExecuteSWHashLen17to32(s, lenght);
        }
    } else if (lenght <= 64) {
        return vxnneExecuteSWLSHHashLen33to64(s, lenght);
    }

    return 0;
}

VX_PRIVATE_API vx_int32 vxnneExecuteSWLSHRunningSignBit(vx_tensor input, vx_tensor weight, vx_float32 seed)
{
    vx_float64 score = .0, running_value = .0;
    vx_int32 count = TENSOR_SIZE_INDEX(input, 1);
    vx_int32 input_item_bytes = (vx_int32)vxoMemory_ComputeElementCount(&input->tensorBuffer->memory, 0) * TENSOR_DATA_SIZE(input) / count;
    vx_int32 i = 0, stride = input_item_bytes;
    vx_uint32 key_bytes = stride + sizeof(vx_float32);
    vx_uint8_ptr keys = (vx_uint8_ptr)vxAllocateAndZeroMemory(key_bytes);
    vx_int64 hash_signature = 0;
    vx_float32 w = .0f;

    for (i = 0; i < count; i ++)
    {
        memcpy(keys, &seed, sizeof(vx_float32));
        memcpy(keys + sizeof(vx_float32), TENSOR_LOGICAL_ADDR(input) + i * stride, stride);

        hash_signature = vxnneExecuteSWLSHHash64(keys, key_bytes);
        running_value = (vx_float64)hash_signature;
        w = (TENSOR_VALUED(weight) == vx_true_e) ? VX_GET_DATA_FROM_TENSOR(weight, i):1;

        score += w * running_value;
    }

    vxFree(keys);
    return (score > 0) ? 1 : 0;
}
vx_status vxnneExecuteSWLSHProjection(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_lshprojection_operation lshhashlutOperation = (vxnne_lshprojection_operation)operation;

    vx_tensor input     = lshhashlutOperation->inputs;
    vx_tensor hash      = lshhashlutOperation->hash_func;
    vx_tensor weight    = lshhashlutOperation->weight;
    vx_tensor types     = lshhashlutOperation->type;
    vx_tensor output    = lshhashlutOperation->outputs;

    vx_uint32 num_hash = TENSOR_SIZE_INDEX(hash, 1);
    vx_uint32 num_bits = TENSOR_SIZE_INDEX(hash, 0);
    vx_uint32 h = 0, b = 0;
    vx_int32 hash_signature = 0, bit = 0;
    vx_float32 seed = .0f;
    vx_int32 type = *((vx_int32_ptr)TENSOR_LOGICAL_ADDR((vx_tensor)types));


    for (h = 0; h < num_hash; h ++)
    {
        hash_signature = 0;
        for (b = 0; b < num_bits; b ++)
        {
            seed = VX_GET_DATA_FROM_TENSOR(hash, b + h * num_bits);
            bit = vxnneExecuteSWLSHRunningSignBit(input, weight, seed);

            switch (type)
            {
            case VX_LSH_PROJ_DENSE:
                VX_SAVE_DATA_TO_TENSOR(output, bit, b + h * num_bits);
                break;

            case VX_LSH_PROJ_SPARSE:
                hash_signature = (hash_signature << 1) | bit;
                break;
            }
        }

        if (type == VX_LSH_PROJ_SPARSE)
            VX_SAVE_DATA_TO_TENSOR(output, hash_signature, h);
    }

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNLSHProjection(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoLSHProjection_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}
VX_PRIVATE_API vx_status VX_CALLBACK vxoLSHProjection_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}
#if REGISTER_FRAME

VX_PRIVATE_API vx_status vxoNNLSH_Project_SW_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vx_tensor  inputs = (vx_tensor)parameters[0];
    vx_tensor  type = (vx_tensor)parameters[1];
    vx_tensor  hash_func = (vx_tensor)parameters[2];
    vx_tensor  weight = (vx_tensor)parameters[3];
    vx_tensor  outputs = (vx_tensor)parameters[4];

    vx_uint32  batchCount = TENSOR_SIZE_INDEX(inputs, 3);

    vxnne_lshprojection_layer  lshprojectionLayer = (vxnne_lshprojection_layer)ops_layer;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxnneOperation_Initialize(&lshprojectionLayer->lshprojection_sw_operation.base,
        &lshprojectionLayer->base,
        VXNNE_OPERATION_TARGET_SW,
        VXNNE_OPERATOR_LSH_PROJECTION,
        vxnneExecuteSWLSHProjection,
        VX_NULL,
        batchCount,
        0));

    vxmONERROR(vxnneLayer_SetOperation(
        &lshprojectionLayer->base,
        &lshprojectionLayer->lshprojection_sw_operation.base,
        0));

    lshprojectionLayer->lshprojection_sw_operation.inputs    = inputs;
    lshprojectionLayer->lshprojection_sw_operation.type      = type;
    lshprojectionLayer->lshprojection_sw_operation.hash_func = hash_func;
    lshprojectionLayer->lshprojection_sw_operation.weight    = weight;
    lshprojectionLayer->lshprojection_sw_operation.outputs   = outputs;

    vxmONERROR(vxnneOperation_AddReference(&lshprojectionLayer->lshprojection_sw_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&lshprojectionLayer->lshprojection_sw_operation.base, (vx_reference)hash_func, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&lshprojectionLayer->lshprojection_sw_operation.base, (vx_reference)weight, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&lshprojectionLayer->lshprojection_sw_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT));

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);
OnError:
    return status;
}


VX_PRIVATE_API vx_status vxoNNLayer_GetOperations(vxnne_layer ops_layer, vx_uint32_ptr max_num_operations, vxnne_operation **operations)
{
    vx_status  status = VX_SUCCESS;
    vxnne_lshprojection_layer  lshprojectionLayer = (vxnne_lshprojection_layer)ops_layer;

    *max_num_operations = gcmCOUNTOF(lshprojectionLayer->operations);

    *operations = lshprojectionLayer->operations;

    return status;
}
#endif

VX_PRIVATE_API vx_status VX_CALLBACK vxoLSHProjection_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{

    vx_status status = VX_SUCCESS;
#if REGISTER_FRAME

    vxnne_layer_imp_s registerLSH_Projects[] = {/* Please DON'T adjust the order, it's importent */
        { "LSH_Project NN", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "LSH_Project TP", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "LSH_Project SH EVIS", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "LSH_Project SH F32", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "LSH_Project SW ", vxoNNCommon_Support, vxoNNLSH_Project_SW_Initialize, VX_NULL },
    };

    REGISTER_LAYERS(registerLSH_Projects, vxnne_lshprojection_layer_s, "LSHProjection", vxoNNLayer_GetOperations);

OnError:
#else
    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_tensor  type                       = (vx_tensor)parameters[1];
    vx_tensor  hash_func                  = (vx_tensor)parameters[2];
    vx_tensor  weight                     = (vx_tensor)parameters[3];
    vx_tensor  outputs                    = (vx_tensor)parameters[4];

    vx_uint32  batchCount                 = TENSOR_SIZE_INDEX(inputs, 3);

    vxnne_lshprojection_layer  lshprojectionLayer     = VX_NULL;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }
    gcoOS_Allocate(gcvNULL, sizeof(vxnne_lshprojection_layer_s), (gctPOINTER*)&lshprojectionLayer);
    if (!lshprojectionLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("Out of Memory at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    gcoOS_ZeroMemory(lshprojectionLayer, sizeof(vxnne_lshprojection_layer_s));

    vxnneLayer_Initialize(&lshprojectionLayer->base,
                          "LSHProjection",
                          node,
                          vxmOPERATION_COUNT(lshprojectionLayer),
                          lshprojectionLayer->operations,
                          VX_NULL);

    vxnneOperation_Initialize(&lshprojectionLayer->lshprojection_sw_operation.base,
                              &lshprojectionLayer->base,
                              VXNNE_OPERATION_TARGET_SW,
                              VXNNE_OPERATOR_LSH_PROJECTION,
                              vxnneExecuteSWLSHProjection,
                              VX_NULL,
                              batchCount,
                              0);

    vxnneLayer_SetOperation(
        &lshprojectionLayer->base,
        &lshprojectionLayer->lshprojection_sw_operation.base,
        0);

    lshprojectionLayer->lshprojection_sw_operation.inputs           = inputs;
    lshprojectionLayer->lshprojection_sw_operation.type             = type;
    lshprojectionLayer->lshprojection_sw_operation.hash_func        = hash_func;
    lshprojectionLayer->lshprojection_sw_operation.weight           = weight;
    lshprojectionLayer->lshprojection_sw_operation.outputs          = outputs;

    vxnneOperation_AddReference(&lshprojectionLayer->lshprojection_sw_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
    vxnneOperation_AddReference(&lshprojectionLayer->lshprojection_sw_operation.base, (vx_reference)hash_func, VXNNE_OPERATION_REFENRENCE_INPUT);
    vxnneOperation_AddReference(&lshprojectionLayer->lshprojection_sw_operation.base, (vx_reference)weight, VXNNE_OPERATION_REFENRENCE_INPUT);
    vxnneOperation_AddReference(&lshprojectionLayer->lshprojection_sw_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

    node->layer = &lshprojectionLayer->base;
#endif

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoLSHProjection_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}

