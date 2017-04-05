/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <gc_vxk_common.h>

//#define COMPARE_TO_REF

#ifdef COMPARE_TO_REF
static vx_float32 fp16_to_float32(const vx_int16 in) {
        vx_int32 t1;
        vx_int32 t2;
        vx_int32 t3;
        vx_float32 out;

        t1 = in & 0x7fff;                       // Non-sign bits
        t2 = in & 0x8000;                       // Sign bit
        t3 = in & 0x7c00;                       // Exponent

        t1 <<= 13;                              // Align mantissa on MSB
        t2 <<= 16;                              // Shift sign bit into position

        t1 += 0x38000000;                       // Adjust bias

        t1 = (t3 == 0 ? 0 : t1);                // Denormals-as-zero

        t1 |= t2;                               // Re-insert sign bit

        *((uint32_t*)&out) = t1;

        return out;
    };
#endif

vx_status vxCnnLayer(vx_node node, vx_uint32 layerIndex, vx_uint32 bacthLevelIndex, vx_array nnCmdBuffer, vx_array inputKernelBuffer,
                     vx_array inputBuffer, vx_uint32 inputOffset,
                     vx_array outputBuffer, vx_uint32 outputOffset,
                     vx_scalar batchSize, vx_uint8 dataType, vx_uint32 netFlag)
{
    vx_status status = VX_SUCCESS;

    vx_uint32 *nnCmdBufferPtr = NULL;
    vx_uint32 *nnCmdBufferLogicAddress = NULL;
    vx_uint32 batchIndex;
    vx_uint32 dataTypeSize = (dataType == 0) ? sizeof(vx_int8) : sizeof(vx_int16);
    vx_uint32 inImageXSize = node->kernel->cnnAttributes.cnnkernelStreamInfo[layerIndex].inImageXSize;
    vx_uint32 inImageYSize = node->kernel->cnnAttributes.cnnkernelStreamInfo[layerIndex].inImageYSize;
    vx_uint32 sliceCount   = node->kernel->cnnAttributes.cnnkernelStreamInfo[layerIndex].sliceCount;

    gctUINT32 nnCmdBufferAddress = (gctUINT32)nnCmdBuffer->memory.physicals[0];
    gctUINT32 inputKernelAddress = (gctUINT32)inputKernelBuffer->memory.physicals[0];
    gctUINT32 inputBufferAddress = (gctUINT32)inputBuffer->memory.physicals[0] + inputOffset;
    gctUINT32 outputBufferAddress = (gctUINT32)outputBuffer->memory.physicals[0] + outputOffset;
    vx_uint32 batchSizeValue = batchSize->value->u32;

#if ENABLE_COV_BATCH
    vx_uint32 orgInImageYSize = node->kernel->cnnAttributes.cnnkernelStreamInfo[layerIndex].orgInImageYSize;
    vx_uint32 outFinalImageXSize = node->kernel->cnnAttributes.cnnkernelStreamInfo[layerIndex].outFinalImageXSize;
    vx_uint32 outFinalImageYSize = node->kernel->cnnAttributes.cnnkernelStreamInfo[layerIndex].outFinalImageYSize;
    vx_uint32 w;
#endif

#ifdef COMPARE_TO_REF
    FILE *outputFile = NULL;
    vx_float32 *pfNN_out, *pfRef_out;
    vx_float32 fDelta, fMax_delta, fPortion_BigDelta;
    vx_uint32 i, count_big_delta, offset_max_delta;
    /**/
    vx_char *fileName[] = { "fasterRcnn_resource//000456-proposal//layer1_relu1//output_1_96_300_400.dat",
                            "fasterRcnn_resource//000456-proposal//layer5_relu2//output_1_256_76_101.dat",
                            "fasterRcnn_resource//000456-proposal//layer9_relu3//output_1_384_39_51.dat",
                            "fasterRcnn_resource//000456-proposal//layer11_relu4//output_1_384_39_51.dat",
                            "fasterRcnn_resource//000456-proposal//layer13_relu5//output_1_256_39_51.dat",
                            "fasterRcnn_resource//000456-proposal//layer15_relu_proposal1//output_1_256_39_51.dat",
                            "fasterRcnn_resource//000456-proposal//layer17_proposal_cls_score//output_1_18_39_51.dat",
                            "fasterRcnn_resource//000456-detection//layer2_relu6//output_256_4096.dat",
                            "fasterRcnn_resource//000456-detection//layer5_relu7//output_256_4096.dat",
                            "fasterRcnn_resource//000456-detection//layer8_cls_score//output_256_21.dat"};

    vx_char fullFileName[256] = {'\0'};
    gctBOOL doCompare = gcvTRUE;
#endif

#if ENABLE_COV_BATCH
    if (netFlag == CNN_DAHUA_NET)
    {
       /* pad 0: FP16 only for now */
       vx_int16* inputBufferPtr = (vx_int16*)inputBuffer->memory.logicals[0];
       vx_int16  *toPadPtr, *toPadBasePtr;



       if (layerIndex == 6 || layerIndex == 7 || layerIndex == 8)
       {
           for(w = 0; w <  node->kernel->cnnAttributes.cnnkernelStreamInfo[layerIndex].orgInImageZSize; w++)
           {
               toPadBasePtr = inputBufferPtr + w * inImageXSize * (orgInImageYSize + 1) * batchSizeValue;

               for (batchIndex = 0; batchIndex < batchSizeValue; batchIndex ++)
               {
                   toPadPtr = toPadBasePtr + inImageXSize * ((orgInImageYSize + 1) * batchIndex + orgInImageYSize);
                   memset((void*) toPadPtr, 0, inImageXSize * sizeof(vx_int16));
               }
           }
       }
    }
#endif

    nnCmdBuffer->itemCount = 32;
    nnCmdBufferLogicAddress = (vx_uint32*)nnCmdBuffer->memory.logicals[0];
    nnCmdBufferPtr = (vx_uint32*)nnCmdBuffer->memory.logicals[0];

    nnCmdBufferPtr += 5;

    *nnCmdBufferPtr++ =   inputKernelAddress >> 6;    /* make sure low 6 bits are all 0 */

    *nnCmdBufferPtr++ =   inputBufferAddress;

    *nnCmdBufferPtr++ =   outputBufferAddress;

#ifdef COMPARE_TO_REF
    if (node->kernel->cnnAttributes.cnnkernelStreamInfo[layerIndex].repeats != 1)
    {
        static vx_uint32 repeats = 0;
        repeats++;
        if (repeats == node->kernel->cnnAttributes.cnnkernelStreamInfo[layerIndex].repeats)
        {
            doCompare = gcvTRUE;
            repeats = 0;
        }
        else doCompare = gcvFALSE;
    }
#endif

#if gcdDUMP
    gcmDUMP_BUFFER(gcvNULL,
                    "memory",
                    nnCmdBuffer->memory.physicals[0],
                    (gctPOINTER)nnCmdBuffer->memory.logicals[0],
                    0,
                    nnCmdBuffer->itemCount * nnCmdBuffer->itemSize);

    gcmDUMP_BUFFER(gcvNULL,
                    "memory",
                    inputBuffer->memory.physicals[0],
                    (gctPOINTER)inputBuffer->memory.logicals[0],
                    0,
                    inputBuffer->itemCount * inputBuffer->itemSize);

    gcmDUMP_BUFFER(gcvNULL,
                    "memory",
                    inputKernelBuffer->memory.physicals[0],
                    (gctPOINTER)inputKernelBuffer->memory.logicals[0],
                    0,
                    inputKernelBuffer->itemCount * inputKernelBuffer->itemSize);
#endif

    {
#if defined(__linux__)
    struct timeval start = gcfVX_PerfStart((vx_reference)node);
#endif
    status = gcfVX_Accel((gctUINT32)nnCmdBufferAddress, gcvVX_ACCELERATOR_NN, node->cnnTriggerEventID, node->forceWaitForEvent);
    outputBuffer->itemCount = outputBuffer->capacity;

#if defined(__linux__)
    if (node->base.context->perfEnable)
        printf("layer %3d execution time:%10d us\n", layerIndex, gcfVX_PerfEnd((vx_reference)node, start));
#endif
    }


#if gcdDUMP
    gcmDUMP_BUFFER(gcvNULL,
                    "verify",
                    outputBuffer->memory.physicals[0],
                    (gctPOINTER)outputBuffer->memory.logicals[0],
                    0,
                    node->kernel->cnnAttributes.cnnkernelStreamInfo[layerIndex].outItemCount * outputBuffer->itemSize > 16 ?
                    node->kernel->cnnAttributes.cnnkernelStreamInfo[layerIndex].outItemCount * outputBuffer->itemSize : 16);
#endif

#ifdef COMPARE_TO_REF
        if (doCompare)
        {
            pfNN_out =  (vx_float32*)malloc(node->kernel->cnnAttributes.cnnkernelStreamInfo[layerIndex].outItemCount * sizeof(vx_float32));
            pfRef_out = (vx_float32*)malloc(node->kernel->cnnAttributes.cnnkernelStreamInfo[layerIndex].outItemCount * sizeof(vx_float32));

            if (netFlag == CNN_FASTCNN_NET && (layerIndex == 6 || layerIndex == 9))
            {
                vx_uint32 mergeLayerCount = 2;
                vx_uint32 index = 0;
                vx_uint32 offset[2];
                vx_uint32 readDataItemCount[2];
                char mergeLayerFileName[2][256];
                vx_float32* pfRef_out_layer9[2] = {VX_NULL};

                sprintf(mergeLayerFileName[0], "%s", fileName[layerIndex]);

                offset[0] = 0;
                if (layerIndex == 6)
                {
                    offset[1] = 51*39*18;
                    readDataItemCount[0] = 51*39*18;
                    readDataItemCount[1] = 51*39*36;
                    sprintf(mergeLayerFileName[1], "%s", "fasterRcnn_resource//000456-proposal//layer18_proposal_bbox_pred//output_1_36_39_51.dat");
                }else if (layerIndex == 9)
                {
                    offset[1] = 256*21;
                    readDataItemCount[0] = 256*21;
                    readDataItemCount[1] = 256*84;
                    sprintf(mergeLayerFileName[1], "%s", "fasterRcnn_resource//000456-detection//layer9_bbox_pred//output_256_84.dat");
                }

                for (index = 0; index < mergeLayerCount; index++)
                {
                    if (layerIndex == 9)
                    {
                        pfRef_out_layer9[index] = (vx_float32*)malloc(readDataItemCount[index]*sizeof(vx_float32));
                    }
                    sprintf(fullFileName, "%s", mergeLayerFileName[index]);
                    outputFile = fopen(fullFileName,"rb");            /* compare to ref output file of each layer */
                    if (outputFile == NULL)
                    {
                        gcmPRINT("can't find file %s", fullFileName);
                        return VX_ERROR_INVALID_PARAMETERS;
                    }

                    if (layerIndex == 9)
                    {
                        if(fread(pfRef_out_layer9[index], 1, readDataItemCount[index]*sizeof(vx_float32), outputFile) != readDataItemCount[index]*sizeof(vx_float32) )
                        {
                            gcmPRINT("fread failure");
                            return VX_ERROR_INVALID_PARAMETERS;
                        }
                    }
                    else
                    {
                        if(fread(pfRef_out + offset[index], 1, readDataItemCount[index]*sizeof(vx_float32), outputFile) != readDataItemCount[index]*sizeof(vx_float32) )
                        {
                            gcmPRINT("fread failure");
                            return VX_ERROR_INVALID_PARAMETERS;
                        }
                    }
                    fclose(outputFile);
                }

                if (layerIndex == 9)
                {
                    vx_uint32 ii, jj;
                    vx_uint32 batchSizeValue2 = 256;
                    vx_uint32 itemSizeValue;
                    vx_float32 * pOutputBase;

                    itemSizeValue = 21;
                    pOutputBase = (vx_float32*)malloc(batchSizeValue2*itemSizeValue*sizeof(vx_float32));
                    memset(pOutputBase, 0, batchSizeValue2*itemSizeValue*sizeof(vx_float32));
                    itemSizeValue = 21;
                    for (ii=0; ii<batchSizeValue2; ii++)
                    {
                        for (jj=0; jj<itemSizeValue; jj++)
                        {
                            pOutputBase[jj*batchSizeValue2 + ii] = *(pfRef_out_layer9[0] + itemSizeValue*ii + jj);
                        }
                    }

                    memcpy(pfRef_out, pOutputBase, batchSizeValue2*itemSizeValue*sizeof(vx_float32));
                    free(pOutputBase);

                    itemSizeValue = 84;
                    pOutputBase = (vx_float32*)malloc(batchSizeValue2*itemSizeValue*sizeof(vx_float32));
                    memset(pOutputBase, 0, batchSizeValue2*itemSizeValue*sizeof(vx_float32));
                    for (ii=0; ii<batchSizeValue2; ii++)
                    {
                        for (jj=0; jj<itemSizeValue; jj++)
                        {
                            pOutputBase[jj*batchSizeValue2 + ii] = *(pfRef_out_layer9[1] + itemSizeValue*ii + jj);
                        }
                    }
                    memcpy(pfRef_out+21*256, pOutputBase, batchSizeValue2*itemSizeValue*sizeof(vx_float32));
                    free(pOutputBase);
                    free(pfRef_out_layer9[0]);
                    free(pfRef_out_layer9[1]);
                }
            }
            else
            {
                sprintf(fullFileName, "%s", fileName[layerIndex]);
                outputFile = fopen(fullFileName,"rb");            /* compare to ref output file of each layer */
                if (outputFile == NULL)
                {
                    gcmPRINT("can't find file %s", fullFileName);
                    return VX_ERROR_INVALID_PARAMETERS;
                }

                if(fread(pfRef_out, 1, node->kernel->cnnAttributes.cnnkernelStreamInfo[layerIndex].outItemCount*sizeof(vx_float32), outputFile) != node->kernel->cnnAttributes.cnnkernelStreamInfo[layerIndex].outItemCount*sizeof(vx_float32) )
                {
                    gcmPRINT("fread failure");
                    return VX_ERROR_INVALID_PARAMETERS;
                }
                fclose(outputFile);

                if (layerIndex == 7 || layerIndex == 8)
                {
                    vx_uint32 ii, jj;
                    vx_uint32 batchSizeValue2 = 256;
                    vx_uint32 itemSizeValue;
                    vx_float32 * pOutputBase;

                    itemSizeValue = 4096;
                    pOutputBase = (vx_float32*)malloc(batchSizeValue2*itemSizeValue*sizeof(vx_float32));
                    memset(pOutputBase, 0, batchSizeValue2*itemSizeValue*sizeof(vx_float32));

                    for (ii=0; ii<batchSizeValue2; ii++)
                    {
                        for (jj=0; jj<itemSizeValue; jj++)
                        {
                            pOutputBase[jj*batchSizeValue2 + ii] = *(pfRef_out + itemSizeValue*ii + jj);
                        }
                    }

                    memcpy(pfRef_out, pOutputBase, batchSizeValue2*itemSizeValue*sizeof(vx_float32));
                    free(pOutputBase);
                }
            }

            fMax_delta =0.0f;
            count_big_delta = 0;
            offset_max_delta = 0;
            for (i = 0; i < node->kernel->cnnAttributes.cnnkernelStreamInfo[layerIndex].outItemCount; i++)
            {
                pfNN_out[i] = fp16_to_float32(*((vx_int16*) outputBuffer->memory.logicals[0] + i));
                if (pfRef_out[i] != 0.0f)
                {
                    fDelta = (vx_float32)fabs((pfRef_out[i] - pfNN_out[i]) / pfRef_out[i]);
                }
                else if (pfNN_out[i] != 0.0f)
                {
                    fDelta = (vx_float32)fabs((pfRef_out[i] - pfNN_out[i]) / pfNN_out[i]);
                }
                else
                {
                    continue;
                }

                if (fDelta > 0.01f)
                {
                    count_big_delta++;
                }
                if (fDelta > fMax_delta)
                {
                    fMax_delta = fDelta;
                    offset_max_delta = i;
                }
            }

            fPortion_BigDelta = (float)count_big_delta/(float)node->kernel->cnnAttributes.cnnkernelStreamInfo[layerIndex].outItemCount;
            printf("Layer %d: Portion_BigDelta = %10.7f, Max delta = %10.6f, count_big_delta= %10d \n", layerIndex, fPortion_BigDelta, fMax_delta, count_big_delta);
            free(pfRef_out);
            free(pfNN_out);
         }
#endif

/****************************************************************************/
/* batching                                                                 */
/****************************************************************************/
#if ENABLE_COV_BATCH
    if ((layerIndex >= bacthLevelIndex) || ((layerIndex == 6 || layerIndex == 7) && netFlag == CNN_DAHUA_NET))
#else
    if (layerIndex >= bacthLevelIndex)
#endif
        return status;

    for (batchIndex = 1; batchIndex < batchSizeValue; batchIndex++)
    {
        nnCmdBufferPtr = (vx_uint32*)nnCmdBuffer->memory.logicals[0];

        if (layerIndex == 0)
        {
            *(nnCmdBufferPtr + 6) =   inputBufferAddress +
                                      batchIndex * inImageXSize * inImageYSize * sliceCount * dataTypeSize;
        }
        else
        {
#if ENABLE_COV_BATCH
            if (layerIndex == 8 && netFlag == CNN_DAHUA_NET)
            {
                /* shift to the next batch image */
                *(nnCmdBufferPtr + 6) =   inputBufferAddress + batchIndex * inImageXSize * (orgInImageYSize + 1) *  dataTypeSize;
            }
            else
#endif
            {
                *(nnCmdBufferPtr + 6) =   inputBufferAddress + batchIndex * node->kernel->cnnAttributes.cnnkernelStreamInfo[layerIndex-1].outItemCount * dataTypeSize;
            }
        }
#if ENABLE_COV_BATCH
        if (layerIndex == 5 && netFlag == CNN_DAHUA_NET)
        {
            /* interleave output slices in depth */
            *(nnCmdBufferPtr + 7) =   outputBufferAddress + batchIndex * outFinalImageXSize * (outFinalImageYSize + 1) * dataTypeSize;
        }
        else
#endif
        {
            *(nnCmdBufferPtr + 7) =   outputBufferAddress + batchIndex * node->kernel->cnnAttributes.cnnkernelStreamInfo[layerIndex].outItemCount * dataTypeSize;
        }

        status = gcfVX_Accel((gctUINT32)nnCmdBufferAddress, gcvVX_ACCELERATOR_NN, 0, gcvFALSE);
#ifdef COMPARE_TO_REF
            if (doCompare)
            {
                pfNN_out =  (vx_float32*)malloc(node->kernel->cnnAttributes.cnnkernelStreamInfo[layerIndex].outItemCount * sizeof(vx_float32));
                pfRef_out = (vx_float32*)malloc(node->kernel->cnnAttributes.cnnkernelStreamInfo[layerIndex].outItemCount * sizeof(vx_float32));

                sprintf(fullFileName, "dahua_net_out/layer0%d/%s", layerIndex, fileName[layerIndex]);
                outputFile = fopen(fullFileName,"rb");            /* compare to ref output file of each layer */
                if (outputFile == NULL)
                {
                    gcmPRINT("can't find file %s", fullFileName);
                    return VX_ERROR_INVALID_PARAMETERS;
                }

                if(fread(pfRef_out, 1, node->kernel->cnnAttributes.cnnkernelStreamInfo[layerIndex].outItemCount*sizeof(vx_float32), outputFile) != node->kernel->cnnAttributes.cnnkernelStreamInfo[layerIndex].outItemCount*sizeof(vx_float32) )
                {
                    gcmPRINT("fread failure");
                    return VX_ERROR_INVALID_PARAMETERS;
                }
                fclose(outputFile);

                fMax_delta =0.0f;
                count_big_delta = 0;
                offset_max_delta = 0;
                for (i = 0; i < node->kernel->cnnAttributes.cnnkernelStreamInfo[layerIndex].outItemCount; i++)
                {
                    pfNN_out[i] = fp16_to_float32(*((vx_int16*) outputBuffer->memory.logicals[0] + batchIndex * node->kernel->cnnAttributes.cnnkernelStreamInfo[layerIndex].outItemCount + i));
                    fDelta = (vx_float32)fabs((pfRef_out[i] - pfNN_out[i])/pfRef_out[i]);

                    if (pfRef_out[i] != 0.0f)
                    {
                        fDelta = (vx_float32)fabs((pfRef_out[i] - pfNN_out[i]) / pfRef_out[i]);
                    }
                    else if (pfNN_out[i] != 0.0f)
                    {
                        fDelta = (vx_float32)fabs((pfRef_out[i] - pfNN_out[i]) / pfNN_out[i]);
                    }
                    else
                    {
                        continue;
                    }

                    if (fDelta > 0.01f)
                    {
                        count_big_delta++;
                    }
                    if (fDelta > fMax_delta)
                    {
                        fMax_delta = fDelta;
                        offset_max_delta = i;
                    }
                }

                fPortion_BigDelta = (float)count_big_delta/(float)node->kernel->cnnAttributes.cnnkernelStreamInfo[layerIndex].outItemCount;
                printf("Portion_BigDelta = %10.7f, Max delta = %10.6f, count_big_delta= %10d \n", fPortion_BigDelta, fMax_delta, count_big_delta);
                free(pfRef_out);
                free(pfNN_out);
             }
#endif

    }
    return status;
}

