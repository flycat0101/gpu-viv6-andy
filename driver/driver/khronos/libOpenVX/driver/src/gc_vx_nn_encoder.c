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


#include <VX/vx.h>
#ifdef WIN32
#include <dirent_win.h>
#else
#include <dirent.h>
#endif

#include <gc_vx_common.h>
#include <gc_vx_nn_encoder.h>
#include <gc_vx_nn_util.h>
#include "gc_nn_arch_model.h"
#ifdef USE_LIB_NN_ARCH_PERF
#include "nnArchPerf.h"
#endif

#define MAX_HISTO_COUNT 256
#define MAX_SIZE_HISTO_COUNT 9

#define SET_1_LOG (1) /*Default is 1, can be 0, 1, 2, 0 is the best (small win), but can only put in 3 bit Huffman code*/
#define SET_1_SIZE (1<<SET_1_LOG)
#define THROUGHPUT 2       /*The decoder throughput, version 1.0, 2 bits*/

vx_int32 outBest2Run[MAX_RUNLEN_SIZE];
vx_int32 bestRunsSorted[MAX_RUNLEN_SIZE];
vx_int32 best2Runs[SET_1_SIZE], freq[SET_1_SIZE+MAX_RUNLEN_SIZE];
vx_int32 numBestRuns,freqSum;

const vx_int32 sizeCodeLen[] = {2, 2, 3, 3, 3, 4, 5, 5,};
const vx_int32 huffCode[8] = {0, 1, 7, 3, 6, 10, 18, 2};

typedef struct CmpStages2Sym
{
    vx_int32 stageCode[3];
    vx_int32 bitLength[3];
    vx_int32 hideBit;
    vx_int32 size;
}CodeSymbol;

#define DEBUG_COMP_ENHANCE_ENC 0
vx_int32 FindBestSubset(vx_int32 *runZeros, vx_int32 size, vx_int32 subSize, vx_int32 *subSet) /*Return how many bits needs*/
{
    vx_int32 i,j,k = 0, maxFreq;
    vx_int32 runs[256], addBreakFreq[256], minBreak;
    vx_float32 entropy, p;
    for(i = 0; i<256; i++)
        runs[i] = i+1;
    for(i = 0; i<2; i++){
       subSet[i] = i;
        freq[i] = runZeros[i];
        runs[i] = 0;
    }
    for(i = 2; i<subSize ; i++){
        for(j = 0; j<256; j++)
            addBreakFreq[j] = 0x7fffffff; /*big number*/
        for(j = 0; j<size; j++){
            if(runs[j] == 0)
                continue;
            subSet[i] = j;
            for(k = 0; k<i; k++){/*sorting*/
                if(subSet[k] > j){
                    break;
                }
            }
            if(k<i){ /*inset j to subSet[k]*/
                int n;
                for(n = i; n>k; n--)
                    subSet[n] = subSet[n-1];
                subSet[k] = j;
            }
            addBreakFreq[j] = runZeros[j];
            for(k = 0; k<size; k++){
                int n,m;
                if(k == j || runs[k] == 0)
                    continue;
                n = k+ 1;
                while(n){
                   for(m = 0; m<=i; m++){
                        if(subSet[i - m]+1 <= n){
                            addBreakFreq[j] += n/(subSet[i - m]+1) * runZeros[k];
                             n %= subSet[i - m]+1; /*k is separated to (subSet[j]+1)*p + k % (subSet[j]+1), but freq[j] also increase*/
                            break;
                        }
                    }
                }
            }
            for(k = 0; k<i; k++){/*Squeeze out j*/
                if(subSet[k] >= j)
                    subSet[k] = subSet[k+1];
            }
        }
        /*Now we find minimum added in*/
        minBreak = 0x7fffffff;
        for(j = 0; j<size; j++){
            if(addBreakFreq[j] < minBreak){
                k = j;
                minBreak = addBreakFreq[j];
            }
        }
        subSet[i] = k; /*We find the k, with less break;*/
        runs[k] = 0;
        freq[i] = runZeros[k];
        j = k;
        {   /*Sorting in increase order*/
            for(k = 0; k<i; k++){/*sorting*/
                if(subSet[k] > j){
                    break;
                }
            }
            if(k<i){ /*inset j to subSet[k]*/
                int n;
                for(n = i; n>k; n--){
                    subSet[n] = subSet[n-1];
                    freq[n] = freq[n-1];
                }
                subSet[k] = j;
                freq[k] = runZeros[j];
            }
        }

    }
    /*Now we sorted the subset, from largest to smallest*/
    for(i = 0; i<subSize; i++){
        freq[i] = runZeros[subSet[i]];
    }

    /*Since it is a subset, we have to make the other length not in the subset to the sum of subset element*/
    for(i = size - 1; i >= 0; i--){ /*run-length  = i+1*/
        if(runZeros[i]*runs[i]){
            k = i+1; /*runs*/
            while(k){
                for(j = subSize - 1; j >= 0; j--){
                    if(subSet[j]+1 <= k){
                        freq[j] += k/(subSet[j]+1) * runZeros[i];
                         k %= subSet[j]+1; /*k is separated to (subSet[j]+1)*p + k % (subSet[j]+1), but freq[j] also increase*/
                        break;
                    }
                }
                if(k && j < 0){
                    /*printf("Error at %d,%d\n",i,k);*/
                    break;
                }
            }
        }
    }
    /*Now we collect the best 2.*/
    freqSum = 0;
    for(i = 0; i < subSize; i++)
        freqSum += freq[i];

    for(i = 0; i<SET_1_SIZE; i++){
        maxFreq = 0;
        k = i;
        for(j = 0; j<subSize; j++){
            if(maxFreq < freq[j]){
                maxFreq = freq[j];
                k = j;
            }
        }
        if(1){ /*Swap the runlength and frequency*/
            freq[k] *= -1; /*negative it, never be selected again*/
        }
        best2Runs[i] = subSet[k];
    }
    maxFreq = 0;
    for(i = 0; i<subSize; i++){
        if(freq[i]<0){ /*best of 2*/
            maxFreq += -freq[i];
        }
    }
    for(j = 0, i = 0; i < subSize; i++){
        if(freq[i] > 0)
            outBest2Run[j++] = subSet[i];
    }
    /*Finally we calculate the number of bits spending. max2Freq, 1 bit, the rest, Log(subSize-2) bits*/
    p = maxFreq*1.0f/freqSum;
    entropy = (vx_float32)(-(log(p)*p + (1.f-p)*log(1.f-p))/log(2.0f)*freqSum); /*the cost to separate the max2Freq and the others*/
    entropy += maxFreq*SET_1_LOG; /*1-bit for each the max2Freq*/

    k = 0;
    while((1<<k)<(subSize-SET_1_SIZE))k++;
    entropy += k*(freqSum-maxFreq); /*k-bits fro each less frequency*/
    return (vx_int32) entropy;
}

vx_int32 FindBest2PlusRunSets(vx_int32 *runZeros, vx_int32 size, vx_int32 nonZeros)  /*Find the best running set, and set the */
{
    vx_int32  allBits[8];
    vx_int32 i,j, k;
    vx_int32 tempRun[256], keepLast = 0, allSymbols;
    vx_int32 logSize = 0;
    vx_float32 entropy,p;
    while((1<<logSize) < size)
        logSize++;
    if(runZeros[size - 1]*size >= runZeros[0]){
        keepLast = 1;
        nonZeros += runZeros[size - 1];
    }

    for(k = 0; k<logSize*0+1 ; k++){ /*The best entropy, but needs 256 entry huffman table....*/
        for(i = 0; i<size / (1<<k); i++){
            tempRun[i] = runZeros[i];
        }
        for(; i<size-keepLast; i++){
            tempRun[size / (1<<k) - 1] += ((i+1)/ (size / (1<<k)))*runZeros[i];
            j = (i+1)%(size / (1<<k));
            if(j > 0)
                tempRun[j - 1] += runZeros[i];
        }
        allBits[k] = 0;
        for(i = 0; i<size / (1<<k); i++){
            allBits[k] += tempRun[i];
        }
        /*More bits to distinguish non-zeros*/
        allSymbols = nonZeros + allBits[k];
        p = nonZeros/(vx_float32)allSymbols;
        entropy = (vx_float32)(-(log(p)*p + (1.f-p)*log(1.f-p))/log(2.0f)*allSymbols);
        for(i = 0; i<size / (1<<k); i++){
            p = tempRun[i]/(vx_float32)allBits[k];
            if(p > 0.)
            entropy += (vx_float32)(-(log(p)*p + (1.f-p)*log(1.f-p))/log(2.0f)*allBits[k]);
        }
        allBits[k] = (vx_int32)(entropy);
    }
    for(k = LOG_RUN_SIZE1; k >=1 ; k--){
        allBits[k] = FindBestSubset(runZeros, size, SET_1_SIZE+(1<<k), tempRun);
        allSymbols = nonZeros + freqSum;
        p = nonZeros / (vx_float32)allSymbols;
        entropy = (vx_float32)(-(log(p)*p + (1.f-p)*log(1.f-p))/log(2.0f));
        allBits[k] += (vx_int32)(entropy*allSymbols);
    }
    j = 1;
    allBits[0] = allBits[1];
    for (i = 2; i<=LOG_RUN_SIZE1; i++)
    {
        if (allBits[i] < allBits[0])
        {
            j = i;
            allBits[0] = allBits[i];
        }
    }


    FindBestSubset(runZeros, size, SET_1_SIZE+(1<<j), bestRunsSorted);
    return numBestRuns = SET_1_SIZE +(1<<j);
}

void OutputAt(vx_int32 x, vx_uint32** kernelBufferPtr, vx_uint32 *bitOffset, CodeSymbol* codeSymbol)
{
    vx_int32 pos = 0x0;

#if DEBUG_COMP_ENHANCE_ENC
    FILE * pfileSteps = NULL;
    pfileSteps = fopen("stage_steps_enc.txt", "a");

#endif
    if (x % THROUGHPUT == (THROUGHPUT - 1) )
    { /*The whole cycle is down, need to do something*/
#if DEBUG_COMP_ENHANCE_ENC
        if(pfileSteps != NULL)
        {
            fprintf(pfileSteps, "---x:%d\n", x);
        }
#endif
        pos = (x) % (3*THROUGHPUT);

        if (codeSymbol[pos - 1].bitLength[0] < 32)
        {
            codeSymbol[pos - 1].stageCode[0] &= ((1LL << codeSymbol[pos - 1].bitLength[0]) - 1);
        }
        if (codeSymbol[pos - 0].bitLength[0] < 32)
        {
            codeSymbol[pos - 0].stageCode[0] &= ((1LL << codeSymbol[pos - 0].bitLength[0]) - 1);
        }

        writeBits(kernelBufferPtr, bitOffset, codeSymbol[pos - 1].stageCode[0], codeSymbol[pos - 1].bitLength[0]);
        writeBits(kernelBufferPtr, bitOffset, codeSymbol[pos - 0].stageCode[0], codeSymbol[pos - 0].bitLength[0]);

#if DEBUG_COMP_ENHANCE_ENC
        if(pfileSteps != NULL)
        {
            vx_int32 data;
            vx_int32 length;
            data   = codeSymbol[pos - 1].stageCode[0];
            length = codeSymbol[pos - 1].bitLength[0];
            if (length < 32)
            {
                data &= ((1LL << length) - 1);
            }
            fprintf(pfileSteps, "stage 0, code:%d, length:%d\n", data, length);

            data   = codeSymbol[pos - 0].stageCode[0];
            length = codeSymbol[pos - 0].bitLength[0];
            if (length < 32)
            {
                data &= ((1LL << length) - 1);
            }


            fprintf(pfileSteps, "stage 0, code:%d, length:%d\n", data, length);
        }
#endif

        codeSymbol[pos - 1].bitLength[0] = 0;
        codeSymbol[pos - 0].bitLength[0] = 0;

        if (x >= 3)
        {
            if(pos - 2 < 0)
            {
                pos += 6;
            }
            if (codeSymbol[pos - 3].bitLength[1] < 32)
            {
                codeSymbol[pos - 3].stageCode[1] &= ((1LL << codeSymbol[pos - 3].bitLength[1]) - 1);
            }
            if (codeSymbol[pos - 2].bitLength[1] < 32)
            {
                codeSymbol[pos - 2].stageCode[1] &= ((1LL << codeSymbol[pos - 2].bitLength[1]) - 1);
            }
            writeBits(kernelBufferPtr, bitOffset, codeSymbol[pos - 3].stageCode[1], codeSymbol[pos - 3].bitLength[1]);
            writeBits(kernelBufferPtr, bitOffset, codeSymbol[pos - 2].stageCode[1], codeSymbol[pos - 2].bitLength[1]);

#if DEBUG_COMP_ENHANCE_ENC
            if(pfileSteps != NULL)
            {
                vx_int32 data;
                vx_int32 length;

                data   = codeSymbol[pos - 3].stageCode[1];
                length = codeSymbol[pos - 3].bitLength[1];
                if (length < 32)
                {
                    data &= ((1LL << length) - 1);
                }
                fprintf(pfileSteps, "stage 1, code:%d, length:%d\n", data, codeSymbol[pos - 3].bitLength[1]);

                data   = codeSymbol[pos - 2].stageCode[1];
                length = codeSymbol[pos - 2].bitLength[1];
                if (length < 32)
                {
                    data &= ((1LL << length) - 1);
                }
                fprintf(pfileSteps, "stage 1, code:%d, length:%d\n", data, codeSymbol[pos - 2].bitLength[1]);
            }
#endif

            codeSymbol[pos - 3].bitLength[1] = 0;
            codeSymbol[pos - 2].bitLength[1] = 0;
        }

        if (x >= 5)
        {
            if(pos - 4 < 0)
            {
                pos += 6;
            }
            if (codeSymbol[pos - 5].bitLength[2] < 32)
            {
                codeSymbol[pos - 5].stageCode[2] &= ((1LL << codeSymbol[pos - 5].bitLength[2]) - 1);
            }
            if (codeSymbol[pos - 4].bitLength[2] < 32)
            {
                codeSymbol[pos - 4].stageCode[2] &= ((1LL << codeSymbol[pos - 4].bitLength[2]) - 1);
            }
            writeBits(kernelBufferPtr, bitOffset, codeSymbol[pos - 5].stageCode[2], codeSymbol[pos - 5].bitLength[2]);
            writeBits(kernelBufferPtr, bitOffset, codeSymbol[pos - 4].stageCode[2], codeSymbol[pos - 4].bitLength[2]);

#if DEBUG_COMP_ENHANCE_ENC
            if(pfileSteps != NULL)
            {
                vx_int32 data;
                vx_int32 length;

                data   = codeSymbol[pos - 5].stageCode[2];
                length = codeSymbol[pos - 5].bitLength[2];
                if (length < 32)
                {
                    data &= ((1LL << length) - 1);
                }
                fprintf(pfileSteps, "stage 2, code:%d, length:%d\n", data, codeSymbol[pos - 5].bitLength[2]);
                data   = codeSymbol[pos - 4].stageCode[2];
                length = codeSymbol[pos - 4].bitLength[2];
                if (length < 32)
                {
                    data &= ((1LL << length) - 1);
                }
                fprintf(pfileSteps, "stage 2, code:%d, length:%d\n", data, codeSymbol[pos - 4].bitLength[2]);
            }
#endif

            codeSymbol[pos - 5].bitLength[2] = 0;
            codeSymbol[pos - 4].bitLength[2] = 0;
        }
    }
#if DEBUG_COMP_ENHANCE_ENC
    if(pfileSteps != NULL)
    {
        fclose(pfileSteps);
        pfileSteps = NULL;
    }
#endif
}


/*each core end dummy code for stage*/
vx_uint8 dummy0S0 = 0x0; /* stage0 dummy0, 3'b000 for even*/
vx_uint8 dummy1S0 = 0x4; /* stage0 dummy1, 3'b100 for odd*/
vx_uint8 dummyS1  = 0x0; /* stage1 dummy0 and dummy1, zero*/
void addDummy(vx_int32 x, vx_uint32** kernelBufferPtr, vx_uint32* bitOffset, CodeSymbol* codeSymbol, vx_uint32 dummyCount, vx_uint32* dummyStages, vx_uint32* dummyBits)
{
    vx_uint8 dummyS0 = 0;
    vx_int32 j;
    while(x % THROUGHPUT)
    {/*some stage0 didn't output yet*/
        codeSymbol[x%(3*THROUGHPUT)].stageCode[0] = dummyStages[0]; /*huffCode[0];*/
        codeSymbol[x%(3*THROUGHPUT)].bitLength[0] = 3;
        codeSymbol[x%(3*THROUGHPUT)].stageCode[1] = dummyStages[1]; /*huffCode[1];*/
        codeSymbol[x%(3*THROUGHPUT)].bitLength[1] = dummyBits[1]; /*bitLengtg[1];*/
        codeSymbol[x%(3*THROUGHPUT)].stageCode[2] = dummyStages[2]; /*huffCode[2];*/
        codeSymbol[x%(3*THROUGHPUT)].bitLength[2] = dummyBits[2]; /*bitLengtg[2];*/
        OutputAt(x, kernelBufferPtr, bitOffset, codeSymbol);
        x++;
    }

    dummyS0 = (dummyCount & 0x1)? dummy1S0: dummy0S0;
    for (j = 0; j < 2*THROUGHPUT; j++)
    {
        codeSymbol[x%(3*THROUGHPUT)].stageCode[0] = dummyS0; /*huffCode[0];*/
        codeSymbol[x%(3*THROUGHPUT)].bitLength[0] = 3;
        /* StageCode[1] is dummy0S1 and dummy1S2, they both zero*/
        OutputAt(x, kernelBufferPtr, bitOffset, codeSymbol);
        x++;
    }
}

extern vx_status vxnneAdapter_SWCWHN2WHCN(
            vx_uint8_ptr input_ptr, vx_type_e input_format, vx_enum input_quant_type, vx_uint32 input_depth, vx_uint32 input_width, vx_uint32 input_height,
            vx_uint32 input_batch, vx_int8 in_fixpoint, vx_int32 in_tf_zp, vx_float32 in_tf_scale,
            vx_uint8_ptr output_ptr, vx_type_e output_format, vx_enum output_quant_type, vx_uint32 output_depth, vx_uint32 output_width, vx_uint32 output_height,
            vx_int8 out_fixpoint, vx_int32 out_tf_zp, vx_float32 out_tf_scale, vx_enum out_rounding_mode);

vx_uint32 calcFit1xN(vx_context context, vx_uint32 kernelZ, vx_uint32 inputX, vx_uint32 inputY)
{
    vx_uint32 bitLenthOfKernelSize = 4; /*kernel_x_y_size reg only has 4 bit, the max kernel y value is 15*/
    vx_uint32 bitLenthOfInImageSize = 13; /*inImageXsize reg only has 13 bit, the max in image x value is 8191*/
    vx_uint32 maxKernelSize = (1 << bitLenthOfKernelSize) - 1;
    vx_uint32 maxInImageXSize = (1 << bitLenthOfInImageSize) - 1;
    vx_uint32 maxN = gcmMIN(gcmMIN(context->nnConfig.fixedFeature.nnAccumBufferDepth, maxKernelSize), context->nnConfig.fixedFeature.nnInputBufferDepth);
    vx_uint32 fitN = 1;
    vx_uint32 i;

    if (inputX * inputY > maxInImageXSize)
    {
        return 1;
    }

    for (i = 2; i < maxN; i++)
    {
        if (kernelZ % i == 0)
        {
            fitN = i;
            break;
        }
    }
    return fitN;
}

vx_bool calcFitZdp3N(vx_context context,vx_uint32 inputX, vx_uint32 inputY, vx_uint32* fitN, vx_uint32 stride, vx_uint32 poolingSize)
{
    vx_uint32 bitLenthOfKernelSize = 4; /*kernel_x_y_size reg only has 4 bit, the max kernel y value is 15*/
    vx_uint32 bitLenthOfInImageSize = 13; /*inImageXsize reg only has 13 bit, the max in image x value is 8191*/
    vx_uint32 maxKernelSize = (1 << bitLenthOfKernelSize) - 1;
    vx_uint32 maxInImageXSize = (1 << bitLenthOfInImageSize) - 1;
    vx_uint32 maxN = gcmMIN(gcmMIN(context->nnConfig.fixedFeature.nnAccumBufferDepth, maxKernelSize), context->nnConfig.fixedFeature.nnInputBufferDepth);
    vx_bool   isV8 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP0);
    vx_uint32 i;
    vx_uint32 maxInImageYSize = maxInImageXSize;
    vx_uint32 sliceSize = inputX * inputY;

    /*NX1 not support pooling now*/
    if (poolingSize > 1)
        return vx_false_e;

    if ((inputX % 64) == 0)
        return vx_false_e; /*if inputX align to 64, don't need to do 64xN reshap*/

    if ((sliceSize % 64) == 0)
    {
        /* do 64xN rehape */
        vx_uint32 tempX = 64 * 1;
        vx_uint32 tempY = sliceSize / tempX;
        i = 1;

        while ((tempY > maxInImageYSize) && (tempX < maxInImageXSize))
        {
            i++;
            tempX = 64 * i;
            if ((sliceSize % tempX) == 0)
            {
                tempY = sliceSize / tempX;
            }
        }

        if ((tempX < maxInImageXSize) && (tempY < maxInImageYSize))
        {
            *fitN = tempY;
            return vx_true_e;
        }
    }

    if ((inputX % 16) == 0)
       return vx_false_e; /*if inputX align to 16, don't need to do 16xN reshap*/

    if ((sliceSize % 16) == 0)
    {
        /* do 16xN rehape */
        vx_uint32 tempX = 16 * 1;
        vx_uint32 tempY = sliceSize / tempX;
        i = 1;

        while ((tempY > maxInImageYSize) && (tempX < maxInImageXSize))
        {
            i++;
            tempX = 16 * i;
            if ((sliceSize % tempX) == 0)
            {
                tempY = sliceSize / tempX;
            }
        }

        if ((tempX < maxInImageXSize) && (tempY < maxInImageYSize))
        {
            *fitN = tempY;
            return vx_true_e;
        }
    }

    if ((inputX * inputY) < maxInImageXSize && stride == 1 && poolingSize <= 1)
    {
        if (isV8 && ((inputX * inputY) % 2 == 0) && (inputX * inputY) > 64)
            *fitN = 2;
        else
            *fitN = 1;
        return vx_true_e;
    }
    else
    {
        for (i = 2; i < maxN; i++)
        {
            if ((inputX * inputY) % i == 0 && (inputX * inputY / i) <= maxInImageXSize && i % stride == 0)
            {
                /* Check pooling size with pooling stride = 2*/
                if (poolingSize <= 1)
                {
                    *fitN = i;
                    return vx_true_e;
                }
                else
                    continue;
            }
        }
    }

    return vx_false_e;
}

vx_status replaceKernelBufferZOffset(
    vx_weights_biases_parameter wb,
    vx_uint8_ptr wb_base_ptr,
    vx_int32 z_offset
    )
{
    vx_uint32 filterIndex;
    vx_uint32* kernelBufferPtr = VX_NULL;

    if (wb->zOffsetHandle == VX_NULL)
    {
        vxError("replaceKernelBufferZOffset: No offset");
        return VX_ERROR_INVALID_PARAMETERS;
    }

    vxmASSERT(z_offset > 0);

    for (filterIndex = 0; filterIndex < wb->numOfVz; filterIndex++)
    {
        vx_uint32 offsetValue = z_offset * filterIndex;
        vx_uint32 bitOffset = wb->zOffsetHandle[filterIndex].bitOffset;
        kernelBufferPtr = (vx_uint32*)(wb_base_ptr + wb->zOffsetHandle[filterIndex].ptrOffset);

        if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
            replaceBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS_VIP_V7);
        else
            replaceBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS);
    }

    wb->orgZOffsetValue = z_offset;

    return VX_SUCCESS;
}

vx_status vxoWeightsBiasesParameter_ProcessHead(
    vx_weights_biases_parameter     weights_bias,
    vx_enum                         usage
    )
{
    vx_uint8_ptr buff = (vx_uint8_ptr)(WB_MEM_LOGICAL_BASE_ADDR(weights_bias) - WB_MEM_HEAD_OFFSET(weights_bias));

    if (VX_READ_ONLY == usage || VX_READ_AND_WRITE == usage)
    {
        *(vx_int32_ptr)buff = 0x0A0B0C0D;
        buff += 4;

        memcpy(buff, weights_bias, sizeof(vx_weights_biases_parameter_s));
        buff += sizeof(vx_weights_biases_parameter_s);

        memcpy(buff, weights_bias->wb_base, sizeof(vx_weights_biases_parameter_base_s));
        buff += sizeof(vx_weights_biases_parameter_base_s);

        memcpy(buff, weights_bias->slice_array, sizeof(vx_weights_biases_slice_s) * weights_bias->slice_num);
        buff += sizeof(vx_weights_biases_slice_s) * weights_bias->slice_num;

        if (weights_bias->zOffsetHandle != VX_NULL)
        {
            *buff++ = 0x1;
            memcpy(buff, weights_bias->zOffsetHandle, sizeof(vx_weights_biases_z_offset_s));
            buff += sizeof(vx_weights_biases_z_offset_s);
        }
        else
        {
            *buff++ = 0x0;
        }

        if (weights_bias->archPerfHandle != VX_NULL)
        {
            *buff++ = 0x1;
            memcpy(buff, weights_bias->archPerfHandle, sizeof(vx_arch_perf_s));
            buff += sizeof(vx_arch_perf_s);
        }
        else
        {
            *buff++ = 0x0;
        }
    }
    else if (VX_WRITE_ONLY == usage || VX_READ_AND_WRITE == usage)
    {
        if (*(vx_int32_ptr)buff != 0x0A0B0C0D)
            return VX_ERROR_INVALID_VALUE;
        buff += 4;

        memcpy(weights_bias, buff, sizeof(vx_weights_biases_parameter_s));
        buff += sizeof(vx_weights_biases_parameter_s);

        weights_bias->wb_base = (vx_weights_biases_parameter_base)vxAllocateAndZeroMemory(sizeof(vx_weights_biases_parameter_base_s));
        if (weights_bias->wb_base == VX_NULL) return VX_ERROR_NO_MEMORY;
        memcpy(weights_bias->wb_base, buff, sizeof(vx_weights_biases_parameter_base_s));
        buff += sizeof(vx_weights_biases_parameter_base_s);

        memcpy(weights_bias->slice_array, buff, sizeof(vx_weights_biases_slice_s) * weights_bias->slice_num);
        buff += sizeof(vx_weights_biases_slice_s) * weights_bias->slice_num;

        if (*buff++ == 1)
        {
            weights_bias->zOffsetHandle = (vx_weights_biases_z_offset)vxAllocateAndZeroMemory(sizeof(vx_weights_biases_z_offset_s));
            if (weights_bias->zOffsetHandle == VX_NULL) return VX_ERROR_NO_MEMORY;
            memcpy(weights_bias->zOffsetHandle, buff, sizeof(vx_weights_biases_z_offset_s));
            buff += sizeof(vx_weights_biases_z_offset_s);
        }

        if (*buff++ == 1)
        {
            weights_bias->archPerfHandle = (vx_arch_perf)vxAllocateAndZeroMemory(sizeof(vx_arch_perf_s));
            if (weights_bias->archPerfHandle == VX_NULL) return VX_ERROR_NO_MEMORY;
            memcpy(weights_bias->archPerfHandle, buff, sizeof(vx_arch_perf_s));
            buff += sizeof(vx_arch_perf_s);
        }
    }
    else
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    return VX_SUCCESS;
}

#if MAP_UNMAP_REFERENCE
vx_status vxoWeightsBiasesParameter_Map(
    vx_weights_biases_parameter     weights_biases,
    vx_map_id *                     map_id,
    vx_uint32 *                     stride,
    void **                         ptr,
    vx_enum                         usage,
    vx_enum                         mem_type,
    vx_uint32                       flags
    )
{
    vx_status status = VX_FAILURE;

    /* bad parameters */
    if ((usage < VX_READ_ONLY) || (VX_READ_AND_WRITE < usage) ||
        (ptr == NULL) || (stride == NULL))
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    /* if weight_bias is allocated.
     */
    if (weights_biases->memory.allocated == vx_false_e)
    {
        return VX_ERROR_INVALID_REFERENCE;
    }

    vxInfo("vxoWeightsBiasesParameter_Map from "VX_FMT_REF" to ptr %p\n", weights_biases, *ptr);
    {
        vx_uint8 *buf = NULL;
        vx_size size = weights_biases->memory_size;

        if (vxoContext_MemoryMap(
                weights_biases->base.context, (vx_reference)weights_biases, size, usage, mem_type, flags, VX_NULL, (void **)&buf, map_id))
        {
            if (VX_READ_ONLY == usage || VX_READ_AND_WRITE == usage)
            {
                if (vxAcquireMutex(weights_biases->memory.writeLocks[0]) == vx_true_e)
                {
                    vx_uint8 *pSrc = (vx_uint8 *)&weights_biases->memory.logicals[0] - WB_MEM_HEAD_OFFSET(weights_biases);
                    vx_uint8 *pDst = (vx_uint8 *)buf;
                    *stride = (vx_uint32)weights_biases->memory_size;

                    status = vxoWeightsBiasesParameter_ProcessHead(weights_biases, usage);
                    if (status == VX_SUCCESS)
                    {
                        memcpy(pDst, pSrc, size);

                        *ptr = buf;
                        vxoReference_Increment(&weights_biases->base, VX_REF_EXTERNAL);
                    }

                    vxReleaseMutex(weights_biases->memory.writeLocks[0]);
                }
                else
                {
                    status = VX_ERROR_NO_RESOURCES;
                }
            }
            else
            {
                /* write only mode */
                *stride = (vx_uint32)weights_biases->memory_size;
                *ptr = buf;
                vxoReference_Increment(&weights_biases->base, VX_REF_EXTERNAL);
                status = VX_SUCCESS;
            }
        }
        else
        {
            status = VX_FAILURE;
        }
    }

    return status;
}

vx_status vxoWeightsBiasesParameter_Unmap(
    vx_weights_biases_parameter weights_biases,
    vx_map_id                   map_id
    )
{
    vx_status status = VX_FAILURE;

    /* bad parameters */
    if (vxoContext_FindMemoryMap(weights_biases->base.context, (vx_reference)weights_biases, map_id) != vx_true_e)
    {
        vxError("Invalid parameters to unmap weight biases parameter\n");
        return VX_ERROR_INVALID_PARAMETERS;
    }

    vxInfo("vxoWeightsBiasesParameter_Unmap from "VX_FMT_REF"\n", weights_biases);
    {
        vx_context context = weights_biases->base.context;
        vx_memory_map_s* map = &context->memoryMaps[map_id];

        if (map->used && map->ref == (vx_reference)weights_biases)
        {
            if (VX_WRITE_ONLY == map->usage || VX_READ_AND_WRITE == map->usage)
            {
                if (vxAcquireMutex(weights_biases->memory.writeLocks[0]) == vx_true_e)
                {
                    vx_uint8 *pSrc = (vx_uint8 *)map->logical;
                    vx_uint8 *pDst = (vx_uint8 *)&weights_biases->memory.logicals[0] - WB_MEM_HEAD_OFFSET(weights_biases);
                    vx_size size = weights_biases->memory_size;

                    memcpy(pDst, pSrc, size);
                    vxoWeightsBiasesParameter_ProcessHead(weights_biases, map->usage);

                    vxoContext_MemoryUnmap(context, map_id);
                    vxoReference_Decrement(&weights_biases->base, VX_REF_EXTERNAL);
                    vxReleaseMutex(weights_biases->memory.writeLocks[0]);
                    status = VX_SUCCESS;
                }
                else
                {
                    status = VX_ERROR_NO_RESOURCES;
                }
            }
            else
            {
                /* rean only mode */
                vxoContext_MemoryUnmap(weights_biases->base.context, map_id);
                vxoReference_Decrement(&weights_biases->base, VX_REF_EXTERNAL);
                status = VX_SUCCESS;
            }
        }
        else
        {
            status = VX_FAILURE;
        }

        return status;
    }
}
#else
vx_status vxoWeightsBiasesParameter_Map(
    vx_weights_biases_parameter     weights_biases,
    vx_map_id *                     map_id,
    vx_uint32 *                     stride,
    void **                         ptr,
    vx_enum                         usage,
    vx_enum                         mem_type,
    vx_uint32                       flags
    )
{
    vx_status status = VX_FAILURE;

    /* bad parameters */
    if ((usage < VX_READ_ONLY) || (VX_READ_AND_WRITE < usage) ||
        (ptr == NULL) || (stride == NULL))
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    /* if weight_bias is allocated.
     */
    if (weights_biases->memory.allocated == vx_false_e)
    {
        return VX_ERROR_INVALID_REFERENCE;
    }

    vxInfo("vxoWeightsBiasesParameter_Map from "VX_FMT_REF" to ptr %p\n", weights_biases, *ptr);
    {
        vx_uint8 *buf = NULL;
        vx_size size = weights_biases->memory_size;

        if (vxoContext_MemoryMap(
                weights_biases->base.context, (vx_reference)weights_biases, size, usage, mem_type, flags, VX_NULL, (void **)&buf, map_id))
        {
            /* write only mode */
            *stride = (vx_uint32)weights_biases->memory_size;
            *ptr = buf;
            vxoReference_Increment(&weights_biases->base, VX_REF_EXTERNAL);
            status = VX_SUCCESS;
        }
        else
        {
            status = VX_FAILURE;
        }
    }

    return status;
}

vx_status vxoWeightsBiasesParameter_Unmap(
    vx_weights_biases_parameter weights_biases,
    vx_map_id                   map_id
    )
{
    vx_status status = VX_FAILURE;

    /* bad parameters */
    if (vxoContext_FindMemoryMap(weights_biases->base.context, (vx_reference)weights_biases, map_id) != vx_true_e)
    {
        vxError("Invalid parameters to unmap weight biases parameter\n");
        return VX_ERROR_INVALID_PARAMETERS;
    }

    vxInfo("vxoWeightsBiasesParameter_Unmap from "VX_FMT_REF"\n", weights_biases);
    {
        vx_context context = weights_biases->base.context;
        vx_memory_map_s* map = &context->memoryMaps[map_id];

        if (map->used && map->ref == (vx_reference)weights_biases)
        {
            /* rean only mode */
            vxoContext_MemoryUnmap(weights_biases->base.context, map_id);
            vxoReference_Decrement(&weights_biases->base, VX_REF_EXTERNAL);
            status = VX_SUCCESS;
        }
        else
        {
            status = VX_FAILURE;
        }

        return status;
    }
}
#endif

vx_bool vxoWeightsBiasesParameter_IsValid(
    vx_weights_biases_parameter     weights_biases
    )
{
    if (weights_biases == VX_NULL)
        return vx_false_e;

    if (!vxoReference_IsValidAndSpecific(&weights_biases->base, VX_TYPE_WEIGHTS_BIASES_PARAMETER))
        return vx_false_e;

    if (weights_biases->weights_sizes[0] == 0)
        return vx_false_e;

    return vx_true_e;
}

vx_size calculateWeightBiasBufferSizeForZeroRunLen(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_uint32 skip_value,
    vx_uint8 zero_run_len,
    gctPOINTER weight_data,
    vx_uint32 index
    )
{
    vx_enum weightFomat = weight_format;
    vx_uint32 skipValue = skip_value;
    vx_uint8 zeroRunLen = zero_run_len;

    vx_uint32 nnCoreCount = weightFomat == VX_TYPE_INT16 ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : weightFomat == VX_TYPE_FLOAT16 ?
                      context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 maxZeroRun         = (1 << zero_run_len) - 1;

    vx_size kernelBufferSize     = 0;
    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 filterCount        = filters_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;

    vx_uint32 weightCount        = wb->weights_sizes[0] * wb->weights_sizes[1];
    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize(weightFomat);
    vx_uint32 weightBitSize      = weightSize * 8;
    vx_uint32 biasSize           = (vx_uint32)vxDataType_GetSize(bias_format);
    vx_uint32 biasBitSize        = biasSize * 8;

    vx_uint32 sliceCount         = slice_count;
    vx_uint32 filterSliceSize    = weightCount * weightSize;
    vx_uint32 filterSize         = weightCount * weightSize * sliceCount;
    vx_uint8* startDataPtr       = VX_NULL;
    vx_uint8* kernelDataPtr      = VX_NULL;

    vx_uint32 bitOffset          = 0;
    vx_uint32 rsvWeightCount = 0, blockCount = 0, nonZeroCount = 0;

    vx_uint32 coreIndex;
    vx_uint32 groupIndex, filterIndex, sliceIndex;
    vx_uint32 weightXIndex, weightYIndex;
    vx_uint32 zeroRun;

    vx_bool hasVipV7Feature = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7) == gcvSTATUS_TRUE ? vx_true_e : vx_false_e;
    vx_bool hasZDP3 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3);
    vx_bool hasZDP6 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6);
    vx_bool hasXYDP9 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP9);
    vx_bool hasXYDP6 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP6);

    if (weightFomat == VX_TYPE_INT16) biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7_INT16;

    startDataPtr = (vx_uint8*)weight_data;

    /* Write kernel Buffer size for each core. */
    /* Assume core count is at most 16. */
    kernelBufferSize = 64;

    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_uint32 coreFilterCount = (coreIndex < oddFilterPerCore) ? totalFilterPerCore + 1 : totalFilterPerCore;

        /* Write zeroRunLen and coreFilterCount. */
        bitOffset = 8 + 16;

        /* Write weight value and bias for every group. */
        zeroRun = 0;
        for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
        {
            vx_uint32 actualFilterCount = (groupIndex == groupCount - 1) ?
                                            (coreFilterCount - groupIndex * filterCount) : filterCount;
            vx_uint32 filterStart, filterEnd;

            if ((groupIndex == groupCount - 1)
                && (coreIndex >= (filterTotalCount % nnCoreCount)))
            {
                filterStart = groupIndex * nnCoreCount * filterCount
                            + (filterTotalCount % nnCoreCount) * (actualFilterCount + 1)
                            + (coreIndex - (filterTotalCount % nnCoreCount)) * actualFilterCount;
            }
            else
            {
                filterStart = groupIndex * nnCoreCount * filterCount + coreIndex * actualFilterCount;
            }
            filterEnd = filterStart + actualFilterCount - 1;

            if (wb->weights_sizes[0] == 1 && wb->weights_sizes[1] == 1 &&
                (hasZDP3 || hasZDP6) &&
                (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8))
            {
                vx_uint32 zdpNum = hasZDP6 ? 6 : 3;

                /* zdp3 zdp6 can not enable same time */
                vxmASSERT(!(hasZDP3 && hasZDP6));

                for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex += (zdpNum * 2))
                {
                    vx_uint8 group2DCoded = 0;
                    vx_uint32 weight = 0;
                    vx_uint32 realSliceIndex = 0;

                    /* add slices of every filter*/
                    for (filterIndex = filterStart; filterIndex <= filterEnd; filterIndex++)
                    {
                        vx_uint32 nonZeroInBlock1Count = 0; /*Check if there's non-zero point in first zdpNum block*/
                        vx_uint32 nonZeroInBlock2Count = 0; /*Check if there's non-zero point in another zdpNum block*/
                        for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + (zdpNum * 2); realSliceIndex++)
                        {
                            vx_uint8 needToWriteBias = (realSliceIndex == 0) ? 1 : 0;

                            if (realSliceIndex >= sliceCount)
                                break;

                            /* add one slice data every filter */
                            kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * realSliceIndex;
                            if (weightFomat == VX_TYPE_INT8)
                                weight = *((vx_int8 *)kernelDataPtr);
                            else if (weightFomat == VX_TYPE_UINT8)
                                weight = *((vx_uint8 *)kernelDataPtr);
                            else
                                weight = *((vx_uint16 *)kernelDataPtr);

                            if (realSliceIndex - sliceIndex < zdpNum && weight != skipValue)
                                nonZeroInBlock1Count++;
                            else if (weight != skipValue)
                                nonZeroInBlock2Count++;

                            if ((zeroRun == maxZeroRun)
                                || ((realSliceIndex == 0)
                                    && needToWriteBias)
                                || (realSliceIndex == sliceCount - 1)
                                || (weight != skipValue)
                                || ((filterIndex == filterEnd)
                                    && (group2DCoded == 0)))
                            {
                                /* Write zeroRun and weight. */
                                bitOffset += zeroRunLen + weightBitSize;
                                if (bitOffset >= 32)
                                {
                                    bitOffset -= 32;
                                    kernelBufferSize += 4;
                                }
                                zeroRun = 0;

                                if (needToWriteBias)
                                {
                                    bitOffset += biasBitSize;
                                    /* Write bias. */
                                    if (bitOffset >= 32)
                                    {
                                        bitOffset -= 32;
                                        kernelBufferSize += 4;
                                    }
                                    needToWriteBias = 0;
                                }
                                group2DCoded = 1;
                                rsvWeightCount++;
                            }
                            else
                            {
                                zeroRun++;
                            }

                            /* add offet behind the last point of the last slice of each filter */
                            if (realSliceIndex == sliceCount - 1)
                            {
                                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                    bitOffset += NN_Z_POSITION_OFFSET_BITS_VIP_V7;
                                else
                                    bitOffset += NN_Z_POSITION_OFFSET_BITS;
                                if (bitOffset >= 32)
                                {
                                    bitOffset -= 32;
                                    kernelBufferSize += 4;
                                }
                            }
                        }
                        if (sliceIndex + zdpNum >= sliceCount)
                            blockCount++;
                        else
                            blockCount += 2;

                        if (nonZeroInBlock1Count)
                            nonZeroCount++;
                        if (nonZeroInBlock2Count)
                            nonZeroCount++;
                    }
                }
            }
            else if ((hasXYDP9 || hasXYDP6)
                     && (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8))
            {
                vx_uint32 xStep = 3;
                vx_uint32 yStep = 0;
                vx_uint32 inImageBufferSize = hasXYDP9 ? 2 : 1;

                if (hasXYDP6)
                    yStep = 2;
                else
                    yStep = 3;

                for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex += inImageBufferSize)
                {
                    vx_uint8 group2DCoded = 0;
                    vx_uint32 weight = 0;
                    vx_uint32 realSliceIndex = 0;
                    vx_uint32 realWeightXIndex = 0, realWeightYIndex = 0;
                    vx_uint32 subKxSize = 0, subKySize = 0, subKzSize = 0;

                    subKzSize = gcmMIN((sliceCount - sliceIndex), inImageBufferSize);
                    /* add slices of every filter*/
                    for (filterIndex = filterStart; filterIndex <= filterEnd; filterIndex++)
                    {
                        vx_bool hasWriteBias = vx_false_e;

                        for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex += yStep)
                        {
                            subKySize = gcmMIN((wb->weights_sizes[1] - weightYIndex), yStep);
                            for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex += xStep)
                            {
                                subKxSize = gcmMIN((wb->weights_sizes[0] - weightXIndex), xStep);
                                for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + subKzSize; realSliceIndex++)
                                {
                                    vx_uint8 needToWriteBias = (sliceIndex == 0) ? 1 : 0;
                                    vx_uint32 nonZeroInBlockCount = 0; /*Check if there's non-zero point in one 3x3 block*/

                                    /* Add one slice data every filter. */
                                    kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * realSliceIndex;

                                    for (realWeightYIndex = weightYIndex; realWeightYIndex < weightYIndex + subKySize; realWeightYIndex++)
                                    {
                                        for (realWeightXIndex = weightXIndex; realWeightXIndex < weightXIndex + subKxSize; realWeightXIndex++)
                                        {
                                            vx_uint8_ptr realKernelDataPtr = kernelDataPtr + (realWeightYIndex * wb->weights_sizes[0] + realWeightXIndex) * weightSize;

                                            if (weightFomat == VX_TYPE_INT8)
                                                weight = *((vx_int8 *)realKernelDataPtr);
                                            else if (weightFomat == VX_TYPE_UINT8)
                                                weight = *((vx_uint8 *)realKernelDataPtr);
                                            else
                                                weight = *((vx_uint16 *)realKernelDataPtr);

                                            if (weight != skipValue)
                                                nonZeroInBlockCount++;

                                            if ((zeroRun == maxZeroRun)
                                                || ((realSliceIndex == 0)
                                                    && (realWeightXIndex == wb->weights_sizes[0] - 1)
                                                    && (realWeightYIndex == wb->weights_sizes[1] - 1)
                                                    && needToWriteBias)
                                                || ((realSliceIndex == sliceCount - 1)
                                                    && (realWeightXIndex == wb->weights_sizes[0] - 1)
                                                    && (realWeightYIndex == wb->weights_sizes[1] - 1))
                                                || (weight != skipValue)
                                                || ((filterIndex == filterEnd)
                                                    && (realWeightXIndex == wb->weights_sizes[0] - 1)
                                                    && (realWeightYIndex == wb->weights_sizes[1] - 1)
                                                    && (group2DCoded == 0)))
                                            {
                                                /* Write zeroRun and weight. */
                                                bitOffset += zeroRunLen + weightBitSize;
                                                if (bitOffset >= 32)
                                                {
                                                    bitOffset -= 32;
                                                    kernelBufferSize += 4;
                                                }
                                                zeroRun = 0;

                                                if (needToWriteBias && !hasWriteBias)
                                                {
                                                    bitOffset += biasBitSize;
                                                    /* Write bias. */
                                                    if (bitOffset >= 32)
                                                    {
                                                        bitOffset -= 32;
                                                        kernelBufferSize += 4;
                                                    }
                                                    needToWriteBias = 0;
                                                    hasWriteBias = vx_true_e;
                                                }
                                                group2DCoded = 1;
                                                rsvWeightCount++;
                                            }
                                            else
                                            {
                                                zeroRun++;
                                            }

                                            /* add offet behind the last point of the last slice of each filter */
                                            if (realSliceIndex == sliceCount - 1 && realWeightXIndex == wb->weights_sizes[0] - 1 && realWeightYIndex == wb->weights_sizes[1] - 1)
                                            {
                                                 /* Write offsetValue. */
                                                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                                    bitOffset += NN_Z_POSITION_OFFSET_BITS_VIP_V7;
                                                else
                                                    bitOffset += NN_Z_POSITION_OFFSET_BITS;

                                                if (bitOffset >= 32)
                                                {
                                                    bitOffset -= 32;
                                                    kernelBufferSize += 4;
                                                }
                                            }
                                        }
                                    }
                                    blockCount++;
                                    if (nonZeroInBlockCount)
                                        nonZeroCount++;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
                {
                    vx_uint8 group2DCoded = 0;

                    /* Add slices of every filter. */
                    for (filterIndex = filterStart; filterIndex <= filterEnd; filterIndex++)
                    {
                        uint8_t needToWriteBias = (sliceIndex == 0) ? 1 : 0;

                        /* Add one slice data every filter. */
                        kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * sliceIndex;

                        for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex++)
                        {
                            vx_uint32 nonZeroCountInDP3 = 0; /*Check if there's non-zero point in one 3x1 block*/
                            for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex++)
                            {
                                vx_uint32 weight;

                                if (weightFomat == VX_TYPE_INT8)
                                    weight = *((vx_int8 *)kernelDataPtr);
                                else if (weightFomat == VX_TYPE_UINT8)
                                    weight = *((vx_uint8 *)kernelDataPtr);
                                else
                                    weight = *((vx_uint16 *)kernelDataPtr);
                                kernelDataPtr = kernelDataPtr + weightSize;

                                if(!hasVipV7Feature || weightFomat == VX_TYPE_FLOAT16)
                                {
                                    /*V6 or FP16, DP1, a block is 1x1*/
                                    blockCount++;
                                    if (weight != skipValue)
                                        nonZeroCount++;
                                }
                                else
                                {
                                    if (weight != skipValue)
                                        nonZeroCountInDP3++;

                                    /*V7, DP3, one block is 3x1*/
                                    if ((weightXIndex + 1) % 3 == 0 || weightXIndex == wb->weights_sizes[0] - 1)
                                    {
                                        blockCount++;
                                        if (nonZeroCountInDP3)
                                            nonZeroCount++;
                                        nonZeroCountInDP3 = 0;
                                    }
                                }

                                if ((zeroRun == maxZeroRun)
                                    || ((sliceIndex == 0)
                                        && (weightXIndex == wb->weights_sizes[0] - 1)
                                        && (weightYIndex == wb->weights_sizes[1] - 1)
                                        && needToWriteBias)
                                    || ((weightXIndex == wb->weights_sizes[0] - 1)
                                        && (weightYIndex == wb->weights_sizes[1] - 1)
                                        && (sliceIndex == sliceCount - 1))
                                    || (weight != skipValue)
                                    || ((filterIndex == filterEnd)
                                        && (weightXIndex == wb->weights_sizes[0] - 1)
                                        && (weightYIndex == wb->weights_sizes[1] - 1)
                                        && (group2DCoded == 0)))
                                {
                                    /* Write zeroRun and weight. */
                                    bitOffset += zeroRunLen + weightBitSize;
                                    if (bitOffset >= 32)
                                    {
                                        bitOffset -= 32;
                                        kernelBufferSize += 4;
                                    }
                                    zeroRun = 0;

                                    if (needToWriteBias)
                                    {
                                        bitOffset += biasBitSize;
                                        /* Write bias. */
                                        if (bitOffset >= 32)
                                        {
                                            bitOffset -= 32;
                                            kernelBufferSize += 4;
                                        }
                                        needToWriteBias = 0;
                                    }
                                    group2DCoded = 1;
                                    rsvWeightCount++;
                                }
                                else
                                {
                                    zeroRun++;
                                }
                            }
                        }

                        /* add offet behind the last point of the last slice of each filter */
                        if (sliceIndex == sliceCount - 1)
                        {
                            /* Write offsetValue. */
                            if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                bitOffset += NN_Z_POSITION_OFFSET_BITS_VIP_V7;
                            else
                                bitOffset += NN_Z_POSITION_OFFSET_BITS;
                            if (bitOffset >= 32)
                            {
                                bitOffset -= 32;
                                kernelBufferSize += 4;
                            }
                        }
                    }
                }
            }
        }

        /* pad 0 */
        if (bitOffset)
        {
            kernelBufferSize += 4;
        }
        kernelBufferSize = (kernelBufferSize + 63) & 0xFFFFFFC0;
    }

    wb->slice_array[index].all_count = blockCount;
    wb->slice_array[index].reserve_weight_count = rsvWeightCount;
    wb->slice_array[index].non_zero_count = nonZeroCount;
    wb->slice_array[index].kernel_orig_size = (filterSize + biasSize) * filterTotalCount;

    return kernelBufferSize;
}

vx_bool calculateWeightBiasBufferSizeForZeroRunLenEx(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_uint32 skip_value,
    gctPOINTER weight_data,
    vx_uint32 index,
    vx_size* min_size,
    vx_uint8* zero_run_len
    )
{
    vx_enum weightFomat = weight_format;
    vx_uint32 skipValue = skip_value;

    vx_uint32 nnCoreCount = weightFomat == VX_TYPE_INT16 ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : weightFomat == VX_TYPE_FLOAT16 ?
                      context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 filterCount        = filters_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;

    vx_uint32 weightCount        = wb->weights_sizes[0] * wb->weights_sizes[1];
    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize(weightFomat);
    vx_uint32 weightBitSize      = weightSize * 8;
    vx_uint32 biasSize           = (vx_uint32)vxDataType_GetSize(bias_format);
    vx_uint32 biasBitSize        = biasSize * 8;

    vx_uint32 sliceCount         = slice_count;
    vx_uint32 filterSliceSize    = weightCount * weightSize;
    vx_uint32 filterSize         = weightCount * weightSize * sliceCount;
    vx_uint8* startDataPtr       = VX_NULL;
    vx_uint8* kernelDataPtr      = VX_NULL;

    vx_uint32 bitOffset          = 0;

    vx_uint32 coreIndex;
    vx_uint32 i, j, groupIndex, filterIndex, sliceIndex;
    vx_uint32 weightXIndex, weightYIndex;

    vx_uint32 *origBitsArray = VX_NULL, *maxBasePad = VX_NULL;
    vx_uint32 *zerosPerCoreArray = VX_NULL; /* zerosPerCoreArray[nnCoreCount][MAX_ZRL_LEN + 1 + 2] */
    vx_uint32 minSize = (vx_uint32)~0UL, maxZRL = 0, maxZRLType, bigZRL = 0, blockCount = 0, nonZeroCount = 0;
    vx_uint8  minZrl = 0;
    vx_bool  complete = vx_false_e;
    vx_bool hasVipV7Feature = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7) == gcvSTATUS_TRUE ? vx_true_e : vx_false_e;
    vx_bool hasZDP3 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3);
    vx_bool hasZDP6 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6);
    vx_bool hasXYDP9 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP9);
    vx_bool hasXYDP6 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP6);

    vx_uint32 maxZeroRunLen = (1 << context->nnConfig.fixedFeature.zrlBits) - 1;

    if (weightFomat == VX_TYPE_INT16) biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7_INT16;

    origBitsArray = (vx_uint32 *)vxAllocateAndZeroMemory(nnCoreCount * sizeof(vx_uint32));
    vxmASSERT(origBitsArray != NULL);
    maxBasePad = (vx_uint32 *)vxAllocateAndZeroMemory(nnCoreCount * sizeof(vx_uint32));
    vxmASSERT(maxBasePad != NULL);
    zerosPerCoreArray = (vx_uint32 *)vxAllocateAndZeroMemory(nnCoreCount * (maxZeroRunLen + 1 + 2) * sizeof(vx_uint32));
    vxmASSERT(zerosPerCoreArray != NULL);
    if (origBitsArray == VX_NULL || maxBasePad == NULL || zerosPerCoreArray == NULL)
    {
        if (origBitsArray)
            vxFree(origBitsArray);
        if (maxBasePad)
            vxFree(maxBasePad);
        if (zerosPerCoreArray)
            vxFree(zerosPerCoreArray);
        vxError("calculateWeightBiasBufferSizeForZeroRunLenEx: OUT OF MEMORY");
        return vx_false_e;
    }
    startDataPtr = (vx_uint8*)weight_data;

    /* Write kernel Buffer size for each core. */
    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_uint32 coreFilterCount = (coreIndex < oddFilterPerCore) ? totalFilterPerCore + 1 : totalFilterPerCore;
        vx_uint32 zeroRun = 0;

        /* Write zeroRunLen and coreFilterCount. */
        bitOffset = 8 + 16;

        /* Write weight value and bias for every group. */
        for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
        {
            vx_uint32 actualFilterCount = (groupIndex == groupCount - 1) ? (coreFilterCount - groupIndex * filterCount) : filterCount;
            vx_uint32 filterStart, filterEnd;

            if ((groupIndex == groupCount - 1)
                && (coreIndex >= (filterTotalCount % nnCoreCount)))
            {
                filterStart = groupIndex * nnCoreCount * filterCount
                            + (filterTotalCount % nnCoreCount) * (actualFilterCount + 1)
                            + (coreIndex - (filterTotalCount % nnCoreCount)) * actualFilterCount;
            }
            else
            {
                filterStart = groupIndex * nnCoreCount *filterCount + coreIndex * actualFilterCount;
            }
            filterEnd = filterStart + actualFilterCount - 1;

            if (wb->weights_sizes[0] == 1 && wb->weights_sizes[1] == 1 &&
                (hasZDP3 || hasZDP6) &&
                (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8))
            {
                vx_uint32 zdpNum = hasZDP6 ? 6 : 3;

                /* zdp3 zdp6 can not enable same time */
                vxmASSERT(!(hasZDP3 && hasZDP6));

                for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex += (zdpNum * 2))
                {
                    vx_uint8 group2DCoded = 0;
                    vx_uint32 weight = 0;
                    vx_uint32 realSliceIndex = 0;

                    /* add slices of every filter*/
                    for (filterIndex = filterStart; filterIndex <= filterEnd; filterIndex++)
                    {
                        vx_uint32 nonZeroInBlock1Count = 0; /*Check if there's non-zero point in first zdpNum block*/
                        vx_uint32 nonZeroInBlock2Count = 0; /*Check if there's non-zero point in another zdpNum block*/

                        for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + (zdpNum * 2); realSliceIndex++)
                        {
                            vx_uint8 needToWriteBias = (realSliceIndex == 0) ? 1 : 0;

                            if (realSliceIndex >= slice_count)
                                break;

                            /* add one slice data every filter */
                            kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * realSliceIndex;
                            if (weightFomat == VX_TYPE_INT8)
                                weight = *((vx_int8 *)kernelDataPtr);
                            else if (weightFomat == VX_TYPE_UINT8)
                                weight = *((vx_uint8 *)kernelDataPtr);
                            else
                                weight = *((vx_uint16 *)kernelDataPtr);

                            if (realSliceIndex - sliceIndex < zdpNum && weight != skipValue)
                                nonZeroInBlock1Count++;
                            else if (weight != skipValue)
                                nonZeroInBlock2Count++;

                            if (((realSliceIndex == 0)
                                    && needToWriteBias)
                                || (realSliceIndex == slice_count - 1)
                                || (weight != skipValue)
                                || ((filterIndex == filterEnd)
                                    && (group2DCoded == 0)))
                            {
                                /* Write zeroRun and weight. */
                                if (zeroRun > maxZeroRunLen)
                                {
                                    *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 1) += 1;
                                    *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 2) += zeroRun - maxZeroRunLen;
                                    maxBasePad[coreIndex] += (maxZeroRunLen + 1 + zeroRun) / (maxZeroRunLen + 1);
                                }
                                else
                                {
                                    *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + zeroRun) += 1;
                                }
                                if (zeroRun > maxZRL) maxZRL = zeroRun;
                                zeroRun = 0;

                                if (needToWriteBias)
                                {
                                    bitOffset += biasBitSize;
                                    needToWriteBias = 0;
                                }
                                group2DCoded = 1;
                            }
                            else
                            {
                                zeroRun++;
                            }

                            /* add offet behind the last point of the last slice of each filter */
                            if (realSliceIndex == slice_count - 1)
                            {
                                if (hasVipV7Feature)
                                    bitOffset += NN_Z_POSITION_OFFSET_BITS_VIP_V7;
                                else
                                    bitOffset += NN_Z_POSITION_OFFSET_BITS;
                            }
                        }

                        if (sliceIndex + zdpNum >= slice_count)
                            blockCount++;
                        else
                            blockCount += 2;

                        if (nonZeroInBlock1Count)
                            nonZeroCount++;
                        if (nonZeroInBlock2Count)
                            nonZeroCount++;
                    }
                }
            }
            else if ((hasXYDP9 || hasXYDP6)
                && (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8))
            {
                vx_uint32 xStep = 3;
                vx_uint32 yStep = 0;
                vx_uint32 inImageBufferSize = hasXYDP9 ? 2 : 1;

                if (hasXYDP6)
                    yStep = 2;
                else
                    yStep = 3;

                for (sliceIndex = 0; sliceIndex < slice_count; sliceIndex += inImageBufferSize)
                {
                    vx_uint8 group2DCoded = 0;
                    vx_uint32 weight = 0;
                    vx_uint32 realSliceIndex = 0;
                    vx_uint32 realWeightXIndex = 0, realWeightYIndex = 0;
                    vx_uint32 subKxSize = 0, subKySize = 0, subKzSize = 0;

                    subKzSize = gcmMIN((slice_count - sliceIndex), inImageBufferSize);
                    /* add slices of every filter*/
                    for (filterIndex = filterStart; filterIndex <= filterEnd; filterIndex++)
                    {
                        vx_bool hasWriteBias = vx_false_e;

                        for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex += yStep)
                        {
                            subKySize = gcmMIN((wb->weights_sizes[1] - weightYIndex), yStep);
                            for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex += xStep)
                            {
                                subKxSize = gcmMIN((wb->weights_sizes[0] - weightXIndex), xStep);
                                for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + subKzSize; realSliceIndex++)
                                {
                                    vx_uint8 needToWriteBias = (sliceIndex == 0) ? 1 : 0;
                                    vx_uint32 nonZeroInBlockCount = 0; /*Check if there's non-zero point in one 3x3 block*/

                                    /* Add one slice data every filter. */
                                    kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * realSliceIndex;

                                    for (realWeightYIndex = weightYIndex; realWeightYIndex < weightYIndex + subKySize; realWeightYIndex++)
                                    {
                                        for (realWeightXIndex = weightXIndex; realWeightXIndex < weightXIndex + subKxSize; realWeightXIndex++)
                                        {
                                            vx_uint8_ptr realKernelDataPtr = kernelDataPtr + (realWeightYIndex * wb->weights_sizes[0] + realWeightXIndex) * weightSize;

                                            if (weightFomat == VX_TYPE_INT8)
                                                weight = *((vx_int8 *)realKernelDataPtr);
                                            else if (weightFomat == VX_TYPE_UINT8)
                                                weight = *((vx_uint8 *)realKernelDataPtr);
                                            else
                                                weight = *((vx_uint16 *)realKernelDataPtr);

                                            if (weight != skipValue)
                                                nonZeroInBlockCount++;

                                            if (((realSliceIndex == 0)
                                                    && (realWeightXIndex == wb->weights_sizes[0] - 1)
                                                    && (realWeightYIndex == wb->weights_sizes[1] - 1)
                                                    && needToWriteBias)
                                                || ((realSliceIndex == slice_count - 1)
                                                    && (realWeightXIndex == wb->weights_sizes[0] - 1)
                                                    && (realWeightYIndex == wb->weights_sizes[1] - 1))
                                                || (weight != skipValue)
                                                || ((filterIndex == filterEnd)
                                                    && (realWeightXIndex == wb->weights_sizes[0] - 1)
                                                    && (realWeightYIndex == wb->weights_sizes[1] - 1)
                                                    && (group2DCoded == 0)))
                                            {
                                                if (zeroRun > maxZeroRunLen)
                                                {
                                                    *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 1) += 1;
                                                    *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 2) += zeroRun - maxZeroRunLen;
                                                    maxBasePad[coreIndex] += (maxZeroRunLen + 1 + zeroRun) / (maxZeroRunLen + 1);
                                                }
                                                else
                                                {
                                                    *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + zeroRun) += 1;
                                                }
                                                if (zeroRun > maxZRL) maxZRL = zeroRun;
                                                zeroRun = 0;

                                                if (needToWriteBias && !hasWriteBias)
                                                {
                                                    bitOffset += biasBitSize;
                                                    needToWriteBias = 0;
                                                    hasWriteBias = vx_true_e;
                                                }
                                                group2DCoded = 1;
                                            }
                                            else
                                            {
                                                zeroRun++;
                                            }

                                            /* add offet behind the last point of the last slice of each filter */
                                            if (realSliceIndex == slice_count - 1 && realWeightXIndex == wb->weights_sizes[0] - 1 && realWeightYIndex == wb->weights_sizes[1] - 1)
                                            {
                                                 /* Write offsetValue. */
                                                if (hasVipV7Feature)
                                                    bitOffset += NN_Z_POSITION_OFFSET_BITS_VIP_V7;
                                                else
                                                    bitOffset += NN_Z_POSITION_OFFSET_BITS;
                                            }
                                        }
                                    }

                                    blockCount++;
                                    if (nonZeroInBlockCount)
                                        nonZeroCount++;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                for (sliceIndex = 0; sliceIndex < slice_count; sliceIndex++)
                {
                    vx_uint8 group2DCoded = 0;

                    /* Add slices of every filter. */
                    for (filterIndex = filterStart; filterIndex <= filterEnd; filterIndex++)
                    {
                        vx_uint8 needToWriteBias = (sliceIndex == 0) ? 1 : 0;

                        /* Add one slice data every filter. */
                        kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * sliceIndex;

                        for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex++)
                        {
                            vx_uint32 nonZeroCountInDP3 = 0; /*Check if there's non-zero point in one 3x1 block*/

                            for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex++)
                            {
                                vx_uint32 weight;

                                if (weightFomat == VX_TYPE_INT8)
                                    weight = *((vx_int8 *)kernelDataPtr);
                                else if (weightFomat == VX_TYPE_UINT8)
                                    weight = *((vx_uint8 *)kernelDataPtr);
                                else
                                    weight = *((vx_uint16 *)kernelDataPtr);
                                kernelDataPtr = kernelDataPtr + weightSize;

                                if(!hasVipV7Feature || weightFomat == VX_TYPE_FLOAT16)
                                {
                                    /*V6 or FP16, DP1, a block is 1x1*/
                                    blockCount++;
                                    if (weight != skipValue)
                                        nonZeroCount++;
                                }
                                else
                                {
                                    if (weight != skipValue)
                                        nonZeroCountInDP3++;

                                    /*V7, DP3, one block is 3x1*/
                                    if ((weightXIndex + 1) % 3 == 0 || weightXIndex == wb->weights_sizes[0] - 1)
                                    {
                                        blockCount++;
                                        if (nonZeroCountInDP3)
                                            nonZeroCount++;
                                        nonZeroCountInDP3 = 0;
                                    }
                                }

                                if (((sliceIndex == 0)
                                        && (weightXIndex == wb->weights_sizes[0] - 1)
                                        && (weightYIndex == wb->weights_sizes[1] - 1)
                                        && needToWriteBias)
                                    || ((weightXIndex == wb->weights_sizes[0] - 1)
                                        && (weightYIndex == wb->weights_sizes[1] - 1)
                                        && (sliceIndex == slice_count - 1))
                                    || (weight != skipValue)
                                    || ((filterIndex == filterEnd)
                                        && (weightXIndex == wb->weights_sizes[0] - 1)
                                        && (weightYIndex == wb->weights_sizes[1] - 1)
                                        && (group2DCoded == 0))
                                    )
                                {
                                    if (zeroRun > maxZeroRunLen)
                                    {
                                        *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 1) += 1;
                                        *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 2) += zeroRun - maxZeroRunLen;
                                        maxBasePad[coreIndex] += (maxZeroRunLen + 1 + zeroRun) / (maxZeroRunLen + 1);
                                    }
                                    else
                                    {
                                        *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + zeroRun) += 1;
                                    }
                                    if (zeroRun > maxZRL) maxZRL = zeroRun;
                                    zeroRun = 0;

                                    if (needToWriteBias)
                                    {
                                        bitOffset += biasBitSize;
                                        needToWriteBias = 0;
                                    }
                                    group2DCoded = 1;
                                }
                                else
                                {
                                    zeroRun++;
                                }
                            }
                        }

                        /* add offet behind the last point of the last slice of each filter */
                        if (sliceIndex == slice_count - 1)
                        {
                            /* Write offsetValue. */
                            if (hasVipV7Feature)
                                bitOffset += NN_Z_POSITION_OFFSET_BITS_VIP_V7;
                            else
                                bitOffset += NN_Z_POSITION_OFFSET_BITS;
                        }
                    }
                }
            }
        }

        origBitsArray[coreIndex] = bitOffset;
        bigZRL += *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 1);
    }

    if (blockCount == 0)
    {
        vxError("%s: blockCount should not be zero\n", __FUNCTION__);
        return vx_false_e;
    }

    /* analyze compressed size with different zrl */
    maxZRL = gcmMIN(maxZRL, maxZeroRunLen);
    maxZRLType = maxZRL ? gcmMIN((vx_uint32)ceilf((vx_float32)(log(maxZRL) / log(2))), context->nnConfig.fixedFeature.zrlBits) : 0;

    if(blockCount != 0)
        complete = (vx_float32)bigZRL / blockCount > 0.1f ? vx_false_e : vx_true_e;

    for (i = 0; i <= maxZRLType; i++)
    {
        vx_uint32 size = 64;
        vx_uint32 base = (1 << i) - 1;
        vx_uint32 rsvWeightCount = 0;

        for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
        {
            vx_uint32 bits = 0, rwc = 0;

            if (base == 0)
            {
                /* take care zrl = 0 */
                for (j = 0; j <= maxZRL; j++)
                {
                    rwc += *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + j) * (j+1);
                }

                if (*(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 1) != 0)
                {
                    vx_uint32 nsum = *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 1);
                    vx_uint32 dsum = *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 2);

                    rwc += maxZeroRunLen * nsum + dsum + nsum;
                }

                bits += rwc * weightBitSize;
            }
            else
            {
                for (j = 0; j <= maxZRL; j++)
                {
                    vx_uint32 zerosPerCore = *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + j);
                    if (!zerosPerCore) continue;
                    if (j <= base) rwc += zerosPerCore;
                    else rwc += ((j + 1 + base) / (base + 1)) * zerosPerCore;
                }

                if (*(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 1) != 0)
                {
                    vx_uint32 nsum = *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 1);
                    vx_uint32 dsum = *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 2);
                    if (base != maxZeroRunLen)
                    {
                        rwc += ((maxZeroRunLen + 1 + base) * nsum + dsum) / (base + 1);
                    }
                    else
                    {
                        rwc += maxBasePad[coreIndex];
                    }
                }

                bits += rwc * (i + weightBitSize);
            }

            /* other bits */
            rsvWeightCount += rwc;
            bits += origBitsArray[coreIndex];
            size += (gcmALIGN(bits, 32) / 32) * 4;
            size  = gcmALIGN(size, 64);
        }

        if (size < minSize)
        {
            minSize = size;
            minZrl = (vx_uint8)i;
            wb->slice_array[index].reserve_weight_count = rsvWeightCount;
        }

        if (!complete) break;
    }

    wb->slice_array[index].all_count = blockCount;
    wb->slice_array[index].non_zero_count = nonZeroCount;
    wb->slice_array[index].kernel_orig_size = (filterSize + biasSize) * filterTotalCount;
    wb->slice_array[index].kernel_stream_size = minSize;

    if (origBitsArray)
        vxFree(origBitsArray);
    if (maxBasePad)
        vxFree(maxBasePad);
    if (zerosPerCoreArray)
        vxFree(zerosPerCoreArray);
    if (min_size != VX_NULL) *min_size = minSize;
    if (zero_run_len != VX_NULL) *zero_run_len = minZrl;
    return complete;
}

vx_size calculateWeightBiasBalanceSizeForZeroRunLen(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_uint32 skip_value,
    vx_uint8 zero_run_len,
    gctPOINTER weight_data,
    vx_uint32 index
    )
{
    vx_enum weightFomat = weight_format;
    vx_uint32 skipValue = skip_value;
    vx_uint8 zeroRunLen = zero_run_len;

    vx_uint32 nnCoreCount = weightFomat == VX_TYPE_INT16 ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : weightFomat == VX_TYPE_FLOAT16 ?
                      context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 maxZeroRun         = (1 << zero_run_len) - 1;

    vx_size kernelBufferSize     = 0;
    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 filterCount        = filters_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;

    vx_uint32 weightCount        = wb->weights_sizes[0] * wb->weights_sizes[1];
    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize(weightFomat);
    vx_uint32 weightBitSize      = weightSize * 8;
    vx_uint32 biasSize           = (vx_uint32)vxDataType_GetSize(bias_format);
    vx_uint32 biasBitSize        = biasSize * 8;

    vx_uint32 sliceCount         = slice_count;
    vx_uint32 filterSliceSize    = weightCount * weightSize;
    vx_uint32 filterSize         = weightCount * weightSize * sliceCount;
    vx_uint8* startDataPtr       = VX_NULL;
    vx_uint8* kernelDataPtr      = VX_NULL;

    vx_uint32 bitOffset          = 0;
    vx_uint32 rsvWeightCount = 0, blockCount = 0, nonZeroCount = 0;

    vx_uint32 coreIndex;
    vx_uint32 groupIndex, filterIndex, sliceIndex;
    vx_uint32 weightXIndex, weightYIndex;
    vx_uint32 zeroRun;

    vx_bool hasVipV7Feature = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7) == gcvSTATUS_TRUE ? vx_true_e : vx_false_e;
    vx_bool hasZDP3 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3);
    vx_bool hasZDP6 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6);
    vx_bool hasXYDP9 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP9);
    vx_bool hasXYDP6 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP6);

    vx_uint32 totalNonZeroCount = 0;
    vx_float32 averageNZCPerFilter = 0;
    vx_float32 varianceFL32 = 0;
    vx_uint32 variance = 0;
    vx_bool reorder = vx_false_e;

    typedef struct _gcVXNonZeroWeights
    {
        vx_uint32 nonZeroCount;
        vx_uint32 filterIdx;
    }gcVXNonZeroWeights;

    gcVXNonZeroWeights *nonZeroWeights = VX_NULL;
    vx_int32 m = 0;
    vx_uint32 core0FilterCount = 0;
    gcVXNonZeroWeights tmp;

    nonZeroWeights = (gcVXNonZeroWeights*)vxAllocateAndZeroMemory(filterTotalCount * sizeof(gcVXNonZeroWeights));
    if (nonZeroWeights == VX_NULL)
    {
        vxError("fillinKernelBuffer: OUT OF MEMORY");
        return 0;
    }

    /* calc each filter non-zero weights num*/
    for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
    {
        nonZeroWeights[filterIndex].filterIdx = filterIndex;
        for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
        {
            /* add one slice data every filter */
            kernelDataPtr = (vx_uint8*)weight_data + filterIndex * filterSize + filterSliceSize * sliceIndex;

            for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex++)
            {
                for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex++)
                {
                    vx_uint32 weight = 0;

                    if (weightFomat == VX_TYPE_INT8)
                        weight = *((vx_int8 *)kernelDataPtr);
                    else if (weightFomat == VX_TYPE_UINT8)
                        weight = *((vx_uint8 *)kernelDataPtr);
                    else
                        weight = *((vx_uint16 *)kernelDataPtr);
                    kernelDataPtr = kernelDataPtr + weightSize;

                    if (weight != skipValue)
                        nonZeroWeights[filterIndex].nonZeroCount++;

                }
            }
        }
        totalNonZeroCount += nonZeroWeights[filterIndex].nonZeroCount;
    }

    /*Sort*/
    averageNZCPerFilter = (vx_float32)totalNonZeroCount / filterTotalCount;
    for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
    {
        varianceFL32 += gcoMATH_Power((nonZeroWeights[filterIndex].nonZeroCount - averageNZCPerFilter), 2);
    }
    variance = (vx_uint32)(varianceFL32 / filterTotalCount + 0.5f);

    if (variance > 12)
        reorder = vx_true_e;

    if (reorder)
    {
        /*Sort*/
        for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
        {
            tmp.nonZeroCount = nonZeroWeights[filterIndex].nonZeroCount;
            tmp.filterIdx = filterIndex;

            for (m = filterIndex - 1; m >= 0 && nonZeroWeights[m].nonZeroCount > tmp.nonZeroCount; m--)
            {
                nonZeroWeights[m+1].nonZeroCount = nonZeroWeights[m].nonZeroCount;
                nonZeroWeights[m+1].filterIdx = nonZeroWeights[m].filterIdx;
            }
            nonZeroWeights[m+1].nonZeroCount = tmp.nonZeroCount;
            nonZeroWeights[m+1].filterIdx = tmp.filterIdx;
        }
    }

    if (weightFomat == VX_TYPE_INT16) biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7_INT16;

    startDataPtr = (vx_uint8*)weight_data;

    /* Write kernel Buffer size for each core. */
    /* Assume core count is at most 16. */
    kernelBufferSize = 64;

    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_uint32 coreFilterCount = (coreIndex < oddFilterPerCore) ? totalFilterPerCore + 1 : totalFilterPerCore;
        vx_uint32 usedKid = 0;
        vx_uint32* filterGroup = VX_NULL;

        /* Write zeroRunLen and coreFilterCount. */
        bitOffset = 8 + 16;

        /* Write weight value and bias for every group. */
        zeroRun = 0;
        for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
        {
            vx_uint32 actualFilterCount = (groupIndex == groupCount - 1) ? (coreFilterCount - groupIndex * filterCount) : filterCount;
            vx_uint32 groupFilterStart = groupIndex * nnCoreCount * filterCount;
            vx_uint32 kid = 0;
            vx_uint32 unusedCoreCount = (filterTotalCount % nnCoreCount == 0) ? 0 : nnCoreCount - (filterTotalCount % nnCoreCount);
            vx_uint32 sortedIndex = 0;
            vx_uint32 filterStart = 0;

            if (coreIndex == 0)
                core0FilterCount = actualFilterCount;

            if (actualFilterCount)
            {
                filterGroup = (vx_uint32*)vxAllocateAndZeroMemory(actualFilterCount * sizeof(vx_uint32));
                if (filterGroup == VX_NULL)
                {
                    vxError("fillinKernelBufferBalance: OUT OF MEMORY");
                    if (nonZeroWeights)
                        vxFree(nonZeroWeights);
                    return vx_false_e;
                }
            }

            if (reorder)
            {
                /*Dispatch filter to each vzGroup*/
                for (kid = 0; kid < actualFilterCount; kid++)
                {
                    if (groupIndex == groupCount - 1 &&
                        kid == core0FilterCount - 1)
                    {
                        if ((kid + usedKid) % 2 == 0)
                            sortedIndex = groupFilterStart + kid * nnCoreCount + coreIndex;
                        else
                            sortedIndex = groupFilterStart + (kid + 1) * nnCoreCount - coreIndex - 1 - unusedCoreCount;
                    }
                    else
                    {
                        if ((kid + usedKid) % 2 == 0)
                            sortedIndex = groupFilterStart + kid * nnCoreCount + coreIndex;
                        else
                            sortedIndex = groupFilterStart + (kid + 1) * nnCoreCount - coreIndex - 1;
                    }

                    filterGroup[kid] = nonZeroWeights[sortedIndex].filterIdx;
                }

                /*sort the vz group real filter to avoid write output cross*/
                for (kid = 0; kid < actualFilterCount; kid++)
                {
                    vx_uint32 tmp = filterGroup[kid];

                    for (m = kid - 1; m >= 0 && filterGroup[m] > tmp; m--)
                    {
                        filterGroup[m+1] = filterGroup[m];
                    }
                    filterGroup[m+1] = tmp;
                }
            }
            else
            {
                /*if variance is small, don't reorder the filters*/
                if ((groupIndex == groupCount - 1)
                && (coreIndex >= (filterTotalCount % nnCoreCount)))
                {
                    filterStart = groupIndex * nnCoreCount * filterCount
                                + (filterTotalCount % nnCoreCount) * (actualFilterCount + 1)
                                + (coreIndex - (filterTotalCount % nnCoreCount)) * actualFilterCount;
                }
                else
                {
                    filterStart = groupIndex * nnCoreCount *filterCount + coreIndex * actualFilterCount;
                }

                for (kid = 0; kid < actualFilterCount; kid++)
                {
                    filterGroup[kid] = filterStart + kid;
                }
            }

            if (wb->weights_sizes[0] == 1 && wb->weights_sizes[1] == 1 &&
                (hasZDP3 || hasZDP6) &&
                (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8))
            {
                vx_uint32 zdpNum = hasZDP6 ? 6 : 3;

                /* zdp3 zdp6 can not enable same time */
                vxmASSERT(!(hasZDP3 && hasZDP6));

                for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex += (zdpNum * 2))
                {
                    vx_uint8 group2DCoded = 0;
                    vx_uint32 weight = 0;
                    vx_uint32 realSliceIndex = 0;

                    /* add slices of every filter*/
                    for (kid = 0; kid < actualFilterCount; kid++)
                    {
                        vx_uint32 nonZeroInBlock1Count = 0; /*Check if there's non-zero point in first zdpNum block*/
                        vx_uint32 nonZeroInBlock2Count = 0; /*Check if there's non-zero point in another zdpNum block*/

                        filterIndex = filterGroup[kid];
                        for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + (zdpNum * 2); realSliceIndex++)
                        {
                            vx_uint8 needToWriteBias = (realSliceIndex == 0) ? 1 : 0;

                            if (realSliceIndex >= sliceCount)
                                break;

                            /* add one slice data every filter */
                            kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * realSliceIndex;
                            if (weightFomat == VX_TYPE_INT8)
                                weight = *((vx_int8 *)kernelDataPtr);
                            else if (weightFomat == VX_TYPE_UINT8)
                                weight = *((vx_uint8 *)kernelDataPtr);
                            else
                                weight = *((vx_uint16 *)kernelDataPtr);

                            if (realSliceIndex - sliceIndex < zdpNum && weight != skipValue)
                                nonZeroInBlock1Count++;
                            else if (weight != skipValue)
                                nonZeroInBlock2Count++;

                            if ((zeroRun == maxZeroRun)
                                || ((realSliceIndex == 0)
                                    && needToWriteBias)
                                || (realSliceIndex == sliceCount - 1)
                                || (weight != skipValue)
                                || ((kid == actualFilterCount - 1)
                                    && (group2DCoded == 0)))
                            {
                                /* Write zeroRun and weight. */
                                bitOffset += zeroRunLen + weightBitSize;
                                if (bitOffset >= 32)
                                {
                                    bitOffset -= 32;
                                    kernelBufferSize += 4;
                                }
                                zeroRun = 0;

                                if (needToWriteBias)
                                {
                                    bitOffset += biasBitSize;
                                    /* Write bias. */
                                    if (bitOffset >= 32)
                                    {
                                        bitOffset -= 32;
                                        kernelBufferSize += 4;
                                    }
                                    needToWriteBias = 0;
                                }
                                group2DCoded = 1;
                                rsvWeightCount++;
                            }
                            else
                            {
                                zeroRun++;
                            }

                            /* add offet behind the last point of the last slice of each filter */
                            if (realSliceIndex == sliceCount - 1)
                            {
                                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                    bitOffset += NN_Z_POSITION_OFFSET_BITS_VIP_V7;
                                else
                                    bitOffset += NN_Z_POSITION_OFFSET_BITS;
                                if (bitOffset >= 32)
                                {
                                    bitOffset -= 32;
                                    kernelBufferSize += 4;
                                }
                            }
                        }
                        if (sliceIndex + zdpNum >= sliceCount)
                            blockCount++;
                        else
                            blockCount += 2;

                        if (nonZeroInBlock1Count)
                            nonZeroCount++;
                        if (nonZeroInBlock2Count)
                            nonZeroCount++;
                    }
                }
            }
            else if ((hasXYDP9 || hasXYDP6)
                     && (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8))
            {
                vx_uint32 xStep = 3;
                vx_uint32 yStep = 0;
                vx_uint32 inImageBufferSize = hasXYDP9 ? 2 : 1;

                if (hasXYDP6)
                    yStep = 2;
                else
                    yStep = 3;

                for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex += inImageBufferSize)
                {
                    vx_uint8 group2DCoded = 0;
                    vx_uint32 weight = 0;
                    vx_uint32 realSliceIndex = 0;
                    vx_uint32 realWeightXIndex = 0, realWeightYIndex = 0;
                    vx_uint32 subKxSize = 0, subKySize = 0, subKzSize = 0;

                    subKzSize = gcmMIN((sliceCount - sliceIndex), inImageBufferSize);
                    /* add slices of every filter*/
                    for (kid = 0; kid < actualFilterCount; kid++)
                    {
                        vx_bool hasWriteBias = vx_false_e;
                        filterIndex = filterGroup[kid];

                        for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex += yStep)
                        {
                            subKySize = gcmMIN((wb->weights_sizes[1] - weightYIndex), yStep);
                            for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex += xStep)
                            {
                                subKxSize = gcmMIN((wb->weights_sizes[0] - weightXIndex), xStep);
                                for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + subKzSize; realSliceIndex++)
                                {
                                    vx_uint8 needToWriteBias = (sliceIndex == 0) ? 1 : 0;
                                    vx_uint32 nonZeroInBlockCount = 0; /*Check if there's non-zero point in one 3x3 block*/

                                    /* Add one slice data every filter. */
                                    kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * realSliceIndex;

                                    for (realWeightYIndex = weightYIndex; realWeightYIndex < weightYIndex + subKySize; realWeightYIndex++)
                                    {
                                        for (realWeightXIndex = weightXIndex; realWeightXIndex < weightXIndex + subKxSize; realWeightXIndex++)
                                        {
                                            vx_uint8_ptr realKernelDataPtr = kernelDataPtr + (realWeightYIndex * wb->weights_sizes[0] + realWeightXIndex) * weightSize;

                                            if (weightFomat == VX_TYPE_INT8)
                                                weight = *((vx_int8 *)realKernelDataPtr);
                                            else if (weightFomat == VX_TYPE_UINT8)
                                                weight = *((vx_uint8 *)realKernelDataPtr);
                                            else
                                                weight = *((vx_uint16 *)realKernelDataPtr);

                                            if (weight != skipValue)
                                                nonZeroInBlockCount++;

                                            if ((zeroRun == maxZeroRun)
                                                || ((realSliceIndex == 0)
                                                    && (realWeightXIndex == wb->weights_sizes[0] - 1)
                                                    && (realWeightYIndex == wb->weights_sizes[1] - 1)
                                                    && needToWriteBias)
                                                || ((realSliceIndex == sliceCount - 1)
                                                    && (realWeightXIndex == wb->weights_sizes[0] - 1)
                                                    && (realWeightYIndex == wb->weights_sizes[1] - 1))
                                                || (weight != skipValue)
                                                || ((kid == actualFilterCount - 1)
                                                    && (realWeightXIndex == wb->weights_sizes[0] - 1)
                                                    && (realWeightYIndex == wb->weights_sizes[1] - 1)
                                                    && (group2DCoded == 0)))
                                            {
                                                /* Write zeroRun and weight. */
                                                bitOffset += zeroRunLen + weightBitSize;
                                                if (bitOffset >= 32)
                                                {
                                                    bitOffset -= 32;
                                                    kernelBufferSize += 4;
                                                }
                                                zeroRun = 0;

                                                if (needToWriteBias && !hasWriteBias)
                                                {
                                                    bitOffset += biasBitSize;
                                                    /* Write bias. */
                                                    if (bitOffset >= 32)
                                                    {
                                                        bitOffset -= 32;
                                                        kernelBufferSize += 4;
                                                    }
                                                    needToWriteBias = 0;
                                                    hasWriteBias = vx_true_e;
                                                }
                                                group2DCoded = 1;
                                                rsvWeightCount++;
                                            }
                                            else
                                            {
                                                zeroRun++;
                                            }

                                            /* add offet behind the last point of the last slice of each filter */
                                            if (realSliceIndex == sliceCount - 1 && realWeightXIndex == wb->weights_sizes[0] - 1 && realWeightYIndex == wb->weights_sizes[1] - 1)
                                            {
                                                 /* Write offsetValue. */
                                                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                                    bitOffset += NN_Z_POSITION_OFFSET_BITS_VIP_V7;
                                                else
                                                    bitOffset += NN_Z_POSITION_OFFSET_BITS;

                                                if (bitOffset >= 32)
                                                {
                                                    bitOffset -= 32;
                                                    kernelBufferSize += 4;
                                                }
                                            }
                                        }
                                    }
                                    blockCount++;
                                    if (nonZeroInBlockCount)
                                        nonZeroCount++;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
                {
                    vx_uint8 group2DCoded = 0;

                    /* Add slices of every filter. */
                    for (kid = 0; kid < actualFilterCount; kid++)
                    {
                        vx_uint8 needToWriteBias = (sliceIndex == 0) ? 1 : 0;

                        filterIndex = filterGroup[kid];

                        /* Add one slice data every filter. */
                        kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * sliceIndex;

                        for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex++)
                        {
                            vx_uint32 nonZeroCountInDP3 = 0; /*Check if there's non-zero point in one 3x1 block*/
                            for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex++)
                            {
                                vx_uint32 weight;

                                if (weightFomat == VX_TYPE_INT8)
                                    weight = *((vx_int8 *)kernelDataPtr);
                                else if (weightFomat == VX_TYPE_UINT8)
                                    weight = *((vx_uint8 *)kernelDataPtr);
                                else
                                    weight = *((vx_uint16 *)kernelDataPtr);
                                kernelDataPtr = kernelDataPtr + weightSize;

                                if(!hasVipV7Feature || weightFomat == VX_TYPE_FLOAT16)
                                {
                                    /*V6 or FP16, DP1, a block is 1x1*/
                                    blockCount++;
                                    if (weight != skipValue)
                                        nonZeroCount++;
                                }
                                else
                                {
                                    if (weight != skipValue)
                                        nonZeroCountInDP3++;

                                    /*V7, DP3, one block is 3x1*/
                                    if ((weightXIndex + 1) % 3 == 0 || weightXIndex == wb->weights_sizes[0] - 1)
                                    {
                                        blockCount++;
                                        if (nonZeroCountInDP3)
                                            nonZeroCount++;
                                        nonZeroCountInDP3 = 0;
                                    }
                                }

                                if ((zeroRun == maxZeroRun)
                                    || ((sliceIndex == 0)
                                        && (weightXIndex == wb->weights_sizes[0] - 1)
                                        && (weightYIndex == wb->weights_sizes[1] - 1)
                                        && needToWriteBias)
                                    || ((weightXIndex == wb->weights_sizes[0] - 1)
                                        && (weightYIndex == wb->weights_sizes[1] - 1)
                                        && (sliceIndex == sliceCount - 1))
                                    || (weight != skipValue)
                                    || ((kid == actualFilterCount - 1)
                                        && (weightXIndex == wb->weights_sizes[0] - 1)
                                        && (weightYIndex == wb->weights_sizes[1] - 1)
                                        && (group2DCoded == 0)))
                                {
                                    /* Write zeroRun and weight. */
                                    bitOffset += zeroRunLen + weightBitSize;
                                    if (bitOffset >= 32)
                                    {
                                        bitOffset -= 32;
                                        kernelBufferSize += 4;
                                    }
                                    zeroRun = 0;

                                    if (needToWriteBias)
                                    {
                                        bitOffset += biasBitSize;
                                        /* Write bias. */
                                        if (bitOffset >= 32)
                                        {
                                            bitOffset -= 32;
                                            kernelBufferSize += 4;
                                        }
                                        needToWriteBias = 0;
                                    }
                                    group2DCoded = 1;
                                    rsvWeightCount++;
                                }
                                else
                                {
                                    zeroRun++;
                                }
                            }
                        }

                        /* add offet behind the last point of the last slice of each filter */
                        if (sliceIndex == sliceCount - 1)
                        {
                            /* Write offsetValue. */
                            if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                bitOffset += NN_Z_POSITION_OFFSET_BITS_VIP_V7;
                            else
                                bitOffset += NN_Z_POSITION_OFFSET_BITS;
                            if (bitOffset >= 32)
                            {
                                bitOffset -= 32;
                                kernelBufferSize += 4;
                            }
                        }
                    }
                }
            }
            usedKid += actualFilterCount;
            if (filterGroup)
            {
                vxFree(filterGroup);
                filterGroup = VX_NULL;
            }
        }

        /* pad 0 */
        if (bitOffset)
        {
            kernelBufferSize += 4;
        }
        kernelBufferSize = (kernelBufferSize + 63) & 0xFFFFFFC0;
    }

    wb->slice_array[index].all_count = blockCount;
    wb->slice_array[index].reserve_weight_count = rsvWeightCount;
    wb->slice_array[index].non_zero_count = nonZeroCount;
    wb->slice_array[index].kernel_orig_size = (filterSize + biasSize) * filterTotalCount;

    if (nonZeroWeights)
        vxFree(nonZeroWeights);

    return kernelBufferSize;
}

vx_bool calculateWeightBiasBalanceSizeForZeroRunLenEx(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_uint32 skip_value,
    gctPOINTER weight_data,
    vx_uint32 index,
    vx_size* min_size,
    vx_uint8* zero_run_len
    )
{
    vx_enum weightFomat = weight_format;
    vx_uint32 skipValue = skip_value;

    vx_uint32 nnCoreCount = weightFomat == VX_TYPE_INT16 ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : weightFomat == VX_TYPE_FLOAT16 ?
                      context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 filterCount        = filters_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;

    vx_uint32 weightCount        = wb->weights_sizes[0] * wb->weights_sizes[1];
    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize(weightFomat);
    vx_uint32 weightBitSize      = weightSize * 8;
    vx_uint32 biasSize           = (vx_uint32)vxDataType_GetSize(bias_format);
    vx_uint32 biasBitSize        = biasSize * 8;

    vx_uint32 sliceCount         = slice_count;
    vx_uint32 filterSliceSize    = weightCount * weightSize;
    vx_uint32 filterSize         = weightCount * weightSize * sliceCount;
    vx_uint8* startDataPtr       = VX_NULL;
    vx_uint8* kernelDataPtr      = VX_NULL;

    vx_uint32 bitOffset          = 0;

    vx_uint32 coreIndex;
    vx_uint32 i, j, groupIndex, filterIndex, sliceIndex;
    vx_uint32 weightXIndex, weightYIndex;

    vx_uint32 *origBitsArray = VX_NULL, *maxBasePad = VX_NULL;
    vx_uint32 *zerosPerCoreArray = VX_NULL; /* zerosPerCoreArray[nnCoreCount][MAX_ZRL_LEN + 1 + 2] */
    vx_uint32 minSize = (vx_uint32)~0UL, maxZRL = 0, maxZRLType, bigZRL = 0, blockCount = 0, nonZeroCount = 0;
    vx_uint8  minZrl = 0;
    vx_bool  complete = vx_false_e;
    vx_bool hasVipV7Feature = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7) == gcvSTATUS_TRUE ? vx_true_e : vx_false_e;
    vx_bool hasZDP3 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3);
    vx_bool hasZDP6 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6);
    vx_bool hasXYDP9 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP9);
    vx_bool hasXYDP6 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP6);

    vx_uint32 maxZeroRunLen = (1 << context->nnConfig.fixedFeature.zrlBits) - 1;

    vx_uint32 totalNonZeroCount = 0;
    vx_float32 averageNZCPerFilter = 0;
    vx_float32 varianceFL32 = 0;
    vx_uint32 variance = 0;
    vx_bool reorder = vx_false_e;

    typedef struct _gcVXNonZeroWeights
    {
        vx_uint32 nonZeroCount;
        vx_uint32 filterIdx;
    }gcVXNonZeroWeights;

    gcVXNonZeroWeights *nonZeroWeights = VX_NULL;
    vx_int32 m = 0;
    vx_uint32 core0FilterCount = 0;
    gcVXNonZeroWeights tmp;

    nonZeroWeights = (gcVXNonZeroWeights*)vxAllocateAndZeroMemory(filterTotalCount * sizeof(gcVXNonZeroWeights));
    if (nonZeroWeights == VX_NULL)
    {
        vxError("fillinKernelBuffer: OUT OF MEMORY");
        return vx_false_e;
    }

    /* calc each filter non-zero weights num*/
    for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
    {
        nonZeroWeights[filterIndex].filterIdx = filterIndex;
        for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
        {
            /* add one slice data every filter */
            kernelDataPtr = (vx_uint8*)weight_data + filterIndex * filterSize + filterSliceSize * sliceIndex;

            for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex++)
            {
                for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex++)
                {
                    vx_uint32 weight = 0;

                    if (weightFomat == VX_TYPE_INT8)
                        weight = *((vx_int8 *)kernelDataPtr);
                    else if (weightFomat == VX_TYPE_UINT8)
                        weight = *((vx_uint8 *)kernelDataPtr);
                    else
                        weight = *((vx_uint16 *)kernelDataPtr);
                    kernelDataPtr = kernelDataPtr + weightSize;

                    if (weight != skipValue)
                        nonZeroWeights[filterIndex].nonZeroCount++;

                }
            }
        }
        totalNonZeroCount += nonZeroWeights[filterIndex].nonZeroCount;
    }

    /*Sort*/
    averageNZCPerFilter = (vx_float32)totalNonZeroCount / filterTotalCount;
    for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
    {
        varianceFL32 += gcoMATH_Power((nonZeroWeights[filterIndex].nonZeroCount - averageNZCPerFilter), 2);
    }
    variance = (vx_uint32)(varianceFL32 / filterTotalCount + 0.5f);

    if (variance > 12)
        reorder = vx_true_e;

    if (reorder)
    {
        /*Sort*/
        for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
        {
            tmp.nonZeroCount = nonZeroWeights[filterIndex].nonZeroCount;
            tmp.filterIdx = filterIndex;

            for (m = filterIndex - 1; m >= 0 && nonZeroWeights[m].nonZeroCount > tmp.nonZeroCount; m--)
            {
                nonZeroWeights[m+1].nonZeroCount = nonZeroWeights[m].nonZeroCount;
                nonZeroWeights[m+1].filterIdx = nonZeroWeights[m].filterIdx;
            }
            nonZeroWeights[m+1].nonZeroCount = tmp.nonZeroCount;
            nonZeroWeights[m+1].filterIdx = tmp.filterIdx;
        }
    }


    if (weightFomat == VX_TYPE_INT16) biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7_INT16;

    origBitsArray = (vx_uint32 *)vxAllocateAndZeroMemory(nnCoreCount * sizeof(vx_uint32));
    vxmASSERT(origBitsArray != NULL);
    maxBasePad = (vx_uint32 *)vxAllocateAndZeroMemory(nnCoreCount * sizeof(vx_uint32));
    vxmASSERT(maxBasePad != NULL);
    zerosPerCoreArray = (vx_uint32 *)vxAllocateAndZeroMemory(nnCoreCount * (maxZeroRunLen + 1 + 2) * sizeof(vx_uint32));
    vxmASSERT(zerosPerCoreArray != NULL);
    if (origBitsArray == VX_NULL || maxBasePad == NULL || zerosPerCoreArray == NULL)
    {
        if (origBitsArray)
            vxFree(origBitsArray);
        if (maxBasePad)
            vxFree(maxBasePad);
        if (zerosPerCoreArray)
            vxFree(zerosPerCoreArray);
        if (nonZeroWeights)
            vxFree(nonZeroWeights);
        vxError("calculateWeightBiasBufferSizeForZeroRunLenEx: OUT OF MEMORY");
        return vx_false_e;
    }
    startDataPtr = (vx_uint8*)weight_data;

    /* Write kernel Buffer size for each core. */
    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_uint32 coreFilterCount = (coreIndex < oddFilterPerCore) ? totalFilterPerCore + 1 : totalFilterPerCore;
        vx_uint32 zeroRun = 0;
        vx_uint32 usedKid = 0;
        vx_uint32* filterGroup;

        /* Write zeroRunLen and coreFilterCount. */
        bitOffset = 8 + 16;

        /* Write weight value and bias for every group. */
        for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
        {
            vx_uint32 actualFilterCount = (groupIndex == groupCount - 1) ? (coreFilterCount - groupIndex * filterCount) : filterCount;
            vx_uint32 groupFilterStart = groupIndex * nnCoreCount * filterCount;
            vx_uint32 kid = 0;
            vx_uint32 unusedCoreCount = (filterTotalCount % nnCoreCount == 0) ? 0 : nnCoreCount - (filterTotalCount % nnCoreCount);
            vx_uint32 sortedIndex = 0;
            vx_uint32 filterStart = 0;

            if (coreIndex == 0)
                core0FilterCount = actualFilterCount;
            if (actualFilterCount)
            {
                filterGroup = (vx_uint32*)vxAllocateAndZeroMemory(actualFilterCount * sizeof(vx_uint32));
                if (filterGroup == VX_NULL)
                {
                    vxError("fillinKernelBufferBalance: OUT OF MEMORY");
                    if (nonZeroWeights)
                        vxFree(nonZeroWeights);
                    if (origBitsArray)
                        vxFree(origBitsArray);
                    if (maxBasePad)
                        vxFree(maxBasePad);
                    if (zerosPerCoreArray)
                        vxFree(zerosPerCoreArray);
                    return vx_false_e;
                }
            }
            else
                continue;

            if (reorder)
            {
                /*Dispatch filter to each vzGroup*/
                for (kid = 0; kid < actualFilterCount; kid++)
                {
                    if (groupIndex == groupCount - 1 &&
                        kid == core0FilterCount - 1)
                    {
                        if ((kid + usedKid) % 2 == 0)
                            sortedIndex = groupFilterStart + kid * nnCoreCount + coreIndex;
                        else
                            sortedIndex = groupFilterStart + (kid + 1) * nnCoreCount - coreIndex - 1 - unusedCoreCount;
                    }
                    else
                    {
                        if ((kid + usedKid) % 2 == 0)
                            sortedIndex = groupFilterStart + kid * nnCoreCount + coreIndex;
                        else
                            sortedIndex = groupFilterStart + (kid + 1) * nnCoreCount - coreIndex - 1;
                    }

                    filterGroup[kid] = nonZeroWeights[sortedIndex].filterIdx;
                }

                /*sort the vz group real filter to avoid write output cross*/
                for (kid = 0; kid < actualFilterCount; kid++)
                {
                    vx_uint32 tmp = filterGroup[kid];

                    for (m = kid - 1; m >= 0 && filterGroup[m] > tmp; m--)
                    {
                        filterGroup[m+1] = filterGroup[m];
                    }
                    filterGroup[m+1] = tmp;
                }
            }
            else
            {
                /*if variance is small, don't reorder the filters*/
                if ((groupIndex == groupCount - 1)
                && (coreIndex >= (filterTotalCount % nnCoreCount)))
                {
                    filterStart = groupIndex * nnCoreCount * filterCount
                                + (filterTotalCount % nnCoreCount) * (actualFilterCount + 1)
                                + (coreIndex - (filterTotalCount % nnCoreCount)) * actualFilterCount;
                }
                else
                {
                    filterStart = groupIndex * nnCoreCount *filterCount + coreIndex * actualFilterCount;
                }

                for (kid = 0; kid < actualFilterCount; kid++)
                {
                    filterGroup[kid] = filterStart + kid;
                }
            }

            if (wb->weights_sizes[0] == 1 && wb->weights_sizes[1] == 1 &&
                (hasZDP3 || hasZDP6) &&
                (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8))
            {
                vx_uint32 zdpNum = hasZDP6 ? 6 : 3;

                /* zdp3 zdp6 can not enable same time */
                vxmASSERT(!(hasZDP3 && hasZDP6));

                for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex += (zdpNum * 2))
                {
                    vx_uint8 group2DCoded = 0;
                    vx_uint32 weight = 0;
                    vx_uint32 realSliceIndex = 0;

                    /* add slices of every filter*/
                    for (kid = 0; kid < actualFilterCount; kid++)
                    {
                        vx_uint32 nonZeroInBlock1Count = 0; /*Check if there's non-zero point in first zdpNum block*/
                        vx_uint32 nonZeroInBlock2Count = 0; /*Check if there's non-zero point in another zdpNum block*/

                        filterIndex = filterGroup[kid];


                        for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + (zdpNum * 2); realSliceIndex++)
                        {
                            vx_uint8 needToWriteBias = (realSliceIndex == 0) ? 1 : 0;

                            if (realSliceIndex >= slice_count)
                                break;

                            /* add one slice data every filter */
                            kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * realSliceIndex;
                            if (weightFomat == VX_TYPE_INT8)
                                weight = *((vx_int8 *)kernelDataPtr);
                            else if (weightFomat == VX_TYPE_UINT8)
                                weight = *((vx_uint8 *)kernelDataPtr);
                            else
                                weight = *((vx_uint16 *)kernelDataPtr);

                            if (realSliceIndex - sliceIndex < zdpNum && weight != skipValue)
                                nonZeroInBlock1Count++;
                            else if (weight != skipValue)
                                nonZeroInBlock2Count++;

                            if (((realSliceIndex == 0)
                                    && needToWriteBias)
                                || (realSliceIndex == slice_count - 1)
                                || (weight != skipValue)
                                || ((kid == actualFilterCount - 1)
                                    && (group2DCoded == 0)))
                            {
                                /* Write zeroRun and weight. */
                                if (zeroRun > maxZeroRunLen)
                                {
                                    *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 1) += 1;
                                    *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 2) += zeroRun - maxZeroRunLen;
                                    maxBasePad[coreIndex] += (maxZeroRunLen + 1 + zeroRun) / (maxZeroRunLen + 1);
                                }
                                else
                                {
                                    *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + zeroRun) += 1;
                                }
                                if (zeroRun > maxZRL) maxZRL = zeroRun;
                                zeroRun = 0;

                                if (needToWriteBias)
                                {
                                    bitOffset += biasBitSize;
                                    needToWriteBias = 0;
                                }
                                group2DCoded = 1;
                            }
                            else
                            {
                                zeroRun++;
                            }

                            /* add offet behind the last point of the last slice of each filter */
                            if (realSliceIndex == slice_count - 1)
                            {
                                if (hasVipV7Feature)
                                    bitOffset += NN_Z_POSITION_OFFSET_BITS_VIP_V7;
                                else
                                    bitOffset += NN_Z_POSITION_OFFSET_BITS;
                            }
                        }

                        if (sliceIndex + zdpNum >= slice_count)
                            blockCount++;
                        else
                            blockCount += 2;

                        if (nonZeroInBlock1Count)
                            nonZeroCount++;
                        if (nonZeroInBlock2Count)
                            nonZeroCount++;
                    }
                }
            }
            else if ((hasXYDP9 || hasXYDP6)
                && (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8))
            {
                vx_uint32 xStep = 3;
                vx_uint32 yStep = 0;
                vx_uint32 inImageBufferSize = hasXYDP9 ? 2 : 1;

                if (hasXYDP6)
                    yStep = 2;
                else
                    yStep = 3;

                for (sliceIndex = 0; sliceIndex < slice_count; sliceIndex += inImageBufferSize)
                {
                    vx_uint8 group2DCoded = 0;
                    vx_uint32 weight = 0;
                    vx_uint32 realSliceIndex = 0;
                    vx_uint32 realWeightXIndex = 0, realWeightYIndex = 0;
                    vx_uint32 subKxSize = 0, subKySize = 0, subKzSize = 0;

                    subKzSize = gcmMIN((slice_count - sliceIndex), inImageBufferSize);
                    /* add slices of every filter*/
                    for (kid = 0; kid < actualFilterCount; kid++)
                    {
                        vx_bool hasWriteBias = vx_false_e;

                        filterIndex = filterGroup[kid];


                        for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex += yStep)
                        {
                            subKySize = gcmMIN((wb->weights_sizes[1] - weightYIndex), yStep);
                            for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex += xStep)
                            {
                                subKxSize = gcmMIN((wb->weights_sizes[0] - weightXIndex), xStep);
                                for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + subKzSize; realSliceIndex++)
                                {
                                    vx_uint8 needToWriteBias = (sliceIndex == 0) ? 1 : 0;
                                    vx_uint32 nonZeroInBlockCount = 0; /*Check if there's non-zero point in one 3x3 block*/

                                    /* Add one slice data every filter. */
                                    kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * realSliceIndex;

                                    for (realWeightYIndex = weightYIndex; realWeightYIndex < weightYIndex + subKySize; realWeightYIndex++)
                                    {
                                        for (realWeightXIndex = weightXIndex; realWeightXIndex < weightXIndex + subKxSize; realWeightXIndex++)
                                        {
                                            vx_uint8_ptr realKernelDataPtr = kernelDataPtr + (realWeightYIndex * wb->weights_sizes[0] + realWeightXIndex) * weightSize;

                                            if (weightFomat == VX_TYPE_INT8)
                                                weight = *((vx_int8 *)realKernelDataPtr);
                                            else if (weightFomat == VX_TYPE_UINT8)
                                                weight = *((vx_uint8 *)realKernelDataPtr);
                                            else
                                                weight = *((vx_uint16 *)realKernelDataPtr);

                                            if (weight != skipValue)
                                                nonZeroInBlockCount++;

                                            if (((realSliceIndex == 0)
                                                    && (realWeightXIndex == wb->weights_sizes[0] - 1)
                                                    && (realWeightYIndex == wb->weights_sizes[1] - 1)
                                                    && needToWriteBias)
                                                || ((realSliceIndex == slice_count - 1)
                                                    && (realWeightXIndex == wb->weights_sizes[0] - 1)
                                                    && (realWeightYIndex == wb->weights_sizes[1] - 1))
                                                || (weight != skipValue)
                                                || ((kid == actualFilterCount - 1)
                                                    && (realWeightXIndex == wb->weights_sizes[0] - 1)
                                                    && (realWeightYIndex == wb->weights_sizes[1] - 1)
                                                    && (group2DCoded == 0)))
                                            {
                                                if (zeroRun > maxZeroRunLen)
                                                {
                                                    *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 1) += 1;
                                                    *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 2) += zeroRun - maxZeroRunLen;
                                                    maxBasePad[coreIndex] += (maxZeroRunLen + 1 + zeroRun) / (maxZeroRunLen + 1);
                                                }
                                                else
                                                {
                                                    *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + zeroRun) += 1;
                                                }
                                                if (zeroRun > maxZRL) maxZRL = zeroRun;
                                                zeroRun = 0;

                                                if (needToWriteBias && !hasWriteBias)
                                                {
                                                    bitOffset += biasBitSize;
                                                    needToWriteBias = 0;
                                                    hasWriteBias = vx_true_e;
                                                }
                                                group2DCoded = 1;
                                            }
                                            else
                                            {
                                                zeroRun++;
                                            }

                                            /* add offet behind the last point of the last slice of each filter */
                                            if (realSliceIndex == slice_count - 1 && realWeightXIndex == wb->weights_sizes[0] - 1 && realWeightYIndex == wb->weights_sizes[1] - 1)
                                            {
                                                 /* Write offsetValue. */
                                                if (hasVipV7Feature)
                                                    bitOffset += NN_Z_POSITION_OFFSET_BITS_VIP_V7;
                                                else
                                                    bitOffset += NN_Z_POSITION_OFFSET_BITS;
                                            }
                                        }
                                    }

                                    blockCount++;
                                    if (nonZeroInBlockCount)
                                        nonZeroCount++;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                for (sliceIndex = 0; sliceIndex < slice_count; sliceIndex++)
                {
                    vx_uint8 group2DCoded = 0;

                    /* Add slices of every filter. */
                    for (kid = 0; kid < actualFilterCount; kid++)
                    {
                        vx_uint8 needToWriteBias = (sliceIndex == 0) ? 1 : 0;

                        filterIndex = filterGroup[kid];


                        /* Add one slice data every filter. */
                        kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * sliceIndex;

                        for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex++)
                        {
                            vx_uint32 nonZeroCountInDP3 = 0; /*Check if there's non-zero point in one 3x1 block*/

                            for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex++)
                            {
                                vx_uint32 weight;

                                if (weightFomat == VX_TYPE_INT8)
                                    weight = *((vx_int8 *)kernelDataPtr);
                                else if (weightFomat == VX_TYPE_UINT8)
                                    weight = *((vx_uint8 *)kernelDataPtr);
                                else
                                    weight = *((vx_uint16 *)kernelDataPtr);
                                kernelDataPtr = kernelDataPtr + weightSize;

                                if(!hasVipV7Feature || weightFomat == VX_TYPE_FLOAT16)
                                {
                                    /*V6 or FP16, DP1, a block is 1x1*/
                                    blockCount++;
                                    if (weight != skipValue)
                                        nonZeroCount++;
                                }
                                else
                                {
                                    if (weight != skipValue)
                                        nonZeroCountInDP3++;

                                    /*V7, DP3, one block is 3x1*/
                                    if ((weightXIndex + 1) % 3 == 0 || weightXIndex == wb->weights_sizes[0] - 1)
                                    {
                                        blockCount++;
                                        if (nonZeroCountInDP3)
                                            nonZeroCount++;
                                        nonZeroCountInDP3 = 0;
                                    }
                                }

                                if (((sliceIndex == 0)
                                        && (weightXIndex == wb->weights_sizes[0] - 1)
                                        && (weightYIndex == wb->weights_sizes[1] - 1)
                                        && needToWriteBias)
                                    || ((weightXIndex == wb->weights_sizes[0] - 1)
                                        && (weightYIndex == wb->weights_sizes[1] - 1)
                                        && (sliceIndex == slice_count - 1))
                                    || (weight != skipValue)
                                    || ((kid == actualFilterCount - 1)
                                        && (weightXIndex == wb->weights_sizes[0] - 1)
                                        && (weightYIndex == wb->weights_sizes[1] - 1)
                                        && (group2DCoded == 0))
                                    )
                                {
                                    if (zeroRun > maxZeroRunLen)
                                    {
                                        *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 1) += 1;
                                        *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 2) += zeroRun - maxZeroRunLen;
                                        maxBasePad[coreIndex] += (maxZeroRunLen + 1 + zeroRun) / (maxZeroRunLen + 1);
                                    }
                                    else
                                    {
                                        *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + zeroRun) += 1;
                                    }
                                    if (zeroRun > maxZRL) maxZRL = zeroRun;
                                    zeroRun = 0;

                                    if (needToWriteBias)
                                    {
                                        bitOffset += biasBitSize;
                                        needToWriteBias = 0;
                                    }
                                    group2DCoded = 1;
                                }
                                else
                                {
                                    zeroRun++;
                                }
                            }
                        }

                        /* add offet behind the last point of the last slice of each filter */
                        if (sliceIndex == slice_count - 1)
                        {
                            /* Write offsetValue. */
                            if (hasVipV7Feature)
                                bitOffset += NN_Z_POSITION_OFFSET_BITS_VIP_V7;
                            else
                                bitOffset += NN_Z_POSITION_OFFSET_BITS;
                        }
                    }
                }
            }
            usedKid += actualFilterCount;
            if (filterGroup)
            {
                vxFree(filterGroup);
                filterGroup = VX_NULL;
            }
        }

        origBitsArray[coreIndex] = bitOffset;
        bigZRL += *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 1);
    }

    /* analyze compressed size with different zrl */
    maxZRL = gcmMIN(maxZRL, maxZeroRunLen);
    maxZRLType = maxZRL ? gcmMIN((vx_uint32)ceilf((vx_float32)(log(maxZRL) / log(2))), context->nnConfig.fixedFeature.zrlBits) : 0;

    if(blockCount != 0)
        complete = (vx_float32)bigZRL / blockCount > 0.1f ? vx_false_e : vx_true_e;

    for (i = 0; i <= maxZRLType; i++)
    {
        vx_uint32 size = 64;
        vx_uint32 base = (1 << i) - 1;
        vx_uint32 rsvWeightCount = 0;

        for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
        {
            vx_uint32 bits = 0, rwc = 0;

            if (base == 0)
            {
                /* take care zrl = 0 */
                for (j = 0; j <= maxZRL; j++)
                {
                    rwc += *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + j) * (j+1);
                }

                if (*(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 1) != 0)
                {
                    vx_uint32 nsum = *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 1);
                    vx_uint32 dsum = *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 2);

                    rwc += maxZeroRunLen * nsum + dsum + nsum;
                }

                bits += rwc * weightBitSize;
            }
            else
            {
                for (j = 0; j <= maxZRL; j++)
                {
                    vx_uint32 zerosPerCore = *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + j);
                    if (!zerosPerCore) continue;
                    if (j <= base) rwc += zerosPerCore;
                    else rwc += ((j + 1 + base) / (base + 1)) * zerosPerCore;
                }

                if (*(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 1) != 0)
                {
                    vx_uint32 nsum = *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 1);
                    vx_uint32 dsum = *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 2);
                    if (base != maxZeroRunLen)
                    {
                        rwc += ((maxZeroRunLen + 1 + base) * nsum + dsum) / (base + 1);
                    }
                    else
                    {
                        rwc += maxBasePad[coreIndex];
                    }
                }

                bits += rwc * (i + weightBitSize);
            }

            /* other bits */
            rsvWeightCount += rwc;
            bits += origBitsArray[coreIndex];
            size += (gcmALIGN(bits, 32) / 32) * 4;
            size  = gcmALIGN(size, 64);
        }

        if (size < minSize)
        {
            minSize = size;
            minZrl = (vx_uint8)i;
            wb->slice_array[index].reserve_weight_count = rsvWeightCount;
        }

        if (!complete) break;
    }

    wb->slice_array[index].all_count = blockCount;
    wb->slice_array[index].non_zero_count = nonZeroCount;
    wb->slice_array[index].kernel_orig_size = (filterSize + biasSize) * filterTotalCount;
    wb->slice_array[index].kernel_stream_size = minSize;

    if (origBitsArray)
        vxFree(origBitsArray);
    if (maxBasePad)
        vxFree(maxBasePad);
    if (zerosPerCoreArray)
        vxFree(zerosPerCoreArray);
    if (min_size != VX_NULL) *min_size = minSize;
    if (zero_run_len != VX_NULL) *zero_run_len = minZrl;

    if (nonZeroWeights)
        vxFree(nonZeroWeights);

    return complete;
}

void reorderWeightBiasBufferForHuffman(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_int32 z_offset,
    vx_int32 output_size,
    vx_uint32 skip_value,
    vx_uint32 output_final_x,
    vx_uint32 output_final_y,
    vx_uint8_ptr reorder_stream,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_uint8* postMul,
    vx_uint8* postShift,
    vx_uint32* reoder_stream_count_per_core,
    vx_uint32* non_coef_index,
    vx_uint32* zrl_limit_index,
    vx_bool calc_perf
    )
{
    vx_uint32 nnCoreCount = weight_format == VX_TYPE_INT16 ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : weight_format == VX_TYPE_FLOAT16 ?
                      context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;
    vx_uint32 filterCount        = filters_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;

    vx_uint32 weightCount        = wb->weights_sizes[0] * wb->weights_sizes[1];
    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize((vx_type_e)weight_format);
    vx_uint32 weightBitSize      = weightSize * 8;
    vx_uint32 biasSize           = (vx_uint32)vxDataType_GetSize((vx_type_e)bias_format);
    vx_uint32 biasBitSize        = biasSize * 8;
    vx_uint32 biasData           = 0;
    vx_int64  biasData64         = 0;

    vx_uint32 filterSliceSize    = weightCount * weightSize;
    vx_uint32 filterSize         = weightCount * weightSize * slice_count;
    vx_uint8* kernelDataPtr      = VX_NULL;
    vx_uint8* kernelBufferInt8Ptr    = reorder_stream;
    vx_uint16* kernelBufferInt16Ptr    = (vx_uint16*)reorder_stream;

    vx_uint32 inputZP = wb->wb_base->inputZP;
    vx_uint32 coefZP = wb->wb_base->coefZP;

    vx_uint32 coreIndex;
    vx_uint32 elementIndex = 0;
    vx_uint32 nonCoefIndex = 0;
    vx_uint32 limitZRLIndex = 0;
    vx_uint32 idx = 0;
    vx_uint32 filterIndex, sliceIndex;
    vx_uint32 weightXIndex, weightYIndex;
    vx_uint32 skipValue = skip_value;

    vx_bool hasZDP3 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3);
    vx_bool hasZDP6 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6);
    vx_bool hasXYDP9 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP9);
    vx_bool hasXYDP6 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP6);
    vx_bool hasBiasAtEnd = vx_false_e;
    vx_bool hasNNPerFilterPostMultiply = vx_false_e;
    vx_bool hasNNPerFilterPostShift = vx_false_e;

    vx_float64* nonZeroRatioPerCorePerVZG = VX_NULL;
    vx_uint32 groupIndex;

    typedef struct _gcVXWeightsMinusZP
    {
        vx_int32 sum;
        vx_uint32 filterIdx;
    }gcVXWeightsMinusZP;

    gcVXWeightsMinusZP *weightsMinusZP = VX_NULL;

    if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
    {
        if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
        {
            biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7;
        }
        else
        {
            biasBitSize = NN_INTEGER_BIAS_BITS;
        }
    }
    else if (weight_format == VX_TYPE_INT16)
    {
        biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7_INT16;
    }

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT) && inputZP != 0 && weight_format == VX_TYPE_UINT8)
    {
        weightsMinusZP = (gcVXWeightsMinusZP*)vxAllocateAndZeroMemory(filterTotalCount * sizeof(gcVXWeightsMinusZP));

        if (!weightsMinusZP)
        {
            vxError("fillinKernelBuffer: OUT OF MEMORY");
            goto exit;
        }

        /*Calc sum((coef[i] - coefZP) * InZP) */
        for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
        {
            weightsMinusZP[filterIndex].filterIdx = filterIndex;
            weightsMinusZP[filterIndex].sum = 0;
            for (sliceIndex = 0; sliceIndex < slice_count; sliceIndex++)
            {
                kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * sliceIndex;

                for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex++)
                {
                    for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex++)
                    {
                        vx_int32 weight = 0;
                        weight = *((vx_uint8 *)kernelDataPtr);
                        kernelDataPtr = kernelDataPtr + weightSize;

                        weightsMinusZP[filterIndex].sum += (weight - coefZP) * inputZP;
                    }
                }
            }
        }
    }

    if (calc_perf)
    {
        if (wb->max_per_core_per_vzgroup_nzr == VX_NULL)
        {
            wb->max_per_core_per_vzgroup_nzr = (vx_float64*)vxAllocateAndZeroMemory(groupCount * sizeof(vx_float64));
            if (wb->max_per_core_per_vzgroup_nzr == VX_NULL)
            {
                vxError("reorderWeightBiasBufferForHuffman: OUT OF MEMORY\n");
                goto exit;
            }
        }

        nonZeroRatioPerCorePerVZG = (vx_float64*)vxAllocateAndZeroMemory(nnCoreCount * groupCount * sizeof(vx_float64));
        if (nonZeroRatioPerCorePerVZG == VX_NULL)
        {
            vxError("reorderWeightBiasBufferForHuffman: OUT OF MEMORY");
            goto exit;
        }
    }
    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_uint32 coreFilterCount = (coreIndex < oddFilterPerCore) ? totalFilterPerCore + 1 : totalFilterPerCore;

        reoder_stream_count_per_core[coreIndex] = 0;


        if (coreFilterCount > 0)
        {
            /* Save kernels per core*/
            if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
            {
                *((vx_uint16 *)kernelBufferInt8Ptr) = (vx_uint16)coreFilterCount;
                kernelBufferInt8Ptr += 2;
                reoder_stream_count_per_core[coreIndex] += (16 / weightBitSize);
            }
            else
            {
                *kernelBufferInt16Ptr = (vx_uint16)coreFilterCount;
                kernelBufferInt16Ptr ++;
                reoder_stream_count_per_core[coreIndex] += (16 / weightBitSize);
            }
            for (idx = 0; idx < (16 / weightBitSize); idx++)
            {
                non_coef_index[nonCoefIndex++] = elementIndex++;
            }
        }

        for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
        {
            vx_uint32 actualFilterCount = (groupIndex == groupCount - 1) ? (coreFilterCount - groupIndex * filterCount) : filterCount;
            vx_uint32 filterStart, filterEnd;
            vx_uint32 blockCount = 0;
            vx_uint32 nonZeroCount = 0;
            vx_uint32 nonZrlIdx = groupIndex + coreIndex * groupCount;

            if ((groupIndex == groupCount - 1)
                && (coreIndex >= (filterTotalCount % nnCoreCount)))
            {
                filterStart = groupIndex * nnCoreCount * filterCount
                            + (filterTotalCount % nnCoreCount) * (actualFilterCount + 1)
                            + (coreIndex - (filterTotalCount % nnCoreCount)) * actualFilterCount;
            }
            else
            {
                filterStart = groupIndex * nnCoreCount *filterCount + coreIndex * actualFilterCount;
            }
            filterEnd = filterStart + actualFilterCount - 1;

            if (wb->weights_sizes[0] == 1 && wb->weights_sizes[1] == 1 &&
                (hasZDP3 || hasZDP6) &&
                (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8))
            {
                vx_uint32 zdpNum = hasZDP6 ? 6 : 3;

                /* zdp3 zdp6 can not enable same time */
                vxmASSERT(!(hasZDP3 && hasZDP6));

                for (sliceIndex = 0; sliceIndex < slice_count; sliceIndex += (zdpNum * 2))
                {
                    vx_uint32 weight = 0;
                    vx_uint32 realSliceIndex = 0;

                    /* add slices of every filter*/
                    for (filterIndex = filterStart; filterIndex <= filterEnd; filterIndex++)
                    {
                        vx_uint32 nonZeroInBlock1Count = 0; /*Check if there's non-zero point in first zdpNum block*/
                        vx_uint32 nonZeroInBlock2Count = 0; /*Check if there's non-zero point in another zdpNum block*/
                        for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + (zdpNum * 2); realSliceIndex++)
                        {
                            if (realSliceIndex >= slice_count)
                                break;

                            if (realSliceIndex == 0)
                            {
                                if (bias_base_ptr)
                                    biasData = *(bias_base_ptr + filterIndex);
                                else
                                    biasData = 0;

                                /* NewBias = sum((Coef[i] - CoefZP) * - InZP) + Bias*/
                                if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT)
                                    && inputZP != 0
                                    && weight_format == VX_TYPE_UINT8)
                                {
                                    biasData -= weightsMinusZP[filterIndex].sum;
                                }

                                if (!hasBiasAtEnd)
                                {
                                    *((vx_uint32 *)kernelBufferInt8Ptr) = biasData;
                                    kernelBufferInt8Ptr += 4;
                                    reoder_stream_count_per_core[coreIndex] += (biasBitSize / weightBitSize);
                                    for (idx = 0; idx < (biasBitSize / weightBitSize); idx++)
                                    {
                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                    }
                                }
                            }

                            /* add one slice data every filter */
                            kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * realSliceIndex;
                            if (weight_format == VX_TYPE_INT8)
                                weight = *((vx_int8 *)kernelDataPtr);
                            else if (weight_format == VX_TYPE_UINT8)
                                weight = *((vx_uint8 *)kernelDataPtr);
                            else
                                weight = *((vx_uint16 *)kernelDataPtr);

                            if (realSliceIndex - sliceIndex < zdpNum && weight != skipValue)
                                nonZeroInBlock1Count++;
                            else if (weight != skipValue)
                                nonZeroInBlock2Count++;

                            *kernelBufferInt8Ptr = (vx_uint8)weight;
                            kernelBufferInt8Ptr++;
                            reoder_stream_count_per_core[coreIndex] += 1;

                            if (filterIndex == filterEnd && realSliceIndex == 0)
                                zrl_limit_index[limitZRLIndex++] = elementIndex;

                            elementIndex++;

                            /* add offet behind the last point of the last slice of each filter */
                            if (realSliceIndex == slice_count - 1)
                            {
                                vx_uint32 offsetValue;

                                if (hasNNPerFilterPostMultiply)
                                {
                                    /*add post multiply to each filter*/

                                    vx_uint32 postMulSize = 8;
                                    *kernelBufferInt8Ptr = postMul[filterIndex];
                                    kernelBufferInt8Ptr++;
                                    reoder_stream_count_per_core[coreIndex] += (postMulSize / weightBitSize);
                                    for (idx = 0; idx < (postMulSize / weightBitSize); idx++)
                                    {
                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                    }
                                }
                                if (hasNNPerFilterPostShift)
                                {
                                    /*add post multiply to each filter*/

                                    vx_uint32 postShiftSize = 8;
                                    *kernelBufferInt8Ptr = postShift[filterIndex];
                                    kernelBufferInt8Ptr++;
                                    reoder_stream_count_per_core[coreIndex] += (postShiftSize / weightBitSize);
                                    for (idx = 0; idx < (postShiftSize / weightBitSize); idx++)
                                    {
                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                    }
                                }

                                if (z_offset > 0)
                                    offsetValue = (vx_uint32)z_offset * filterIndex;
                                else if (output_size > 0)
                                    offsetValue = output_final_x * output_final_y * output_size * filterIndex;
                                else
                                    offsetValue = output_final_x * output_final_y * weightSize * filterIndex;

                                if (hasBiasAtEnd)
                                {
                                    *((vx_uint32 *)kernelBufferInt8Ptr) = biasData;
                                    kernelBufferInt8Ptr += 4;
                                    reoder_stream_count_per_core[coreIndex] += (biasBitSize / weightBitSize);

                                    for (idx = 0; idx < (biasBitSize / weightBitSize); idx++)
                                    {
                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                    }
                                }

                                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                {
                                    *((vx_uint32 *)kernelBufferInt8Ptr) = offsetValue;
                                    kernelBufferInt8Ptr += 4;
                                    reoder_stream_count_per_core[coreIndex] += (NN_Z_POSITION_OFFSET_BITS_VIP_V7 / weightBitSize);
                                    for (idx = 0; idx < (NN_Z_POSITION_OFFSET_BITS_VIP_V7 / weightBitSize); idx++)
                                    {
                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                    }
                                }
                            }
                        }
                        if (sliceIndex + zdpNum >= slice_count)
                            blockCount++;
                        else
                            blockCount += 2;

                        if (nonZeroInBlock1Count)
                            nonZeroCount++;
                        if (nonZeroInBlock2Count)
                            nonZeroCount++;
                    }
                }
            }
            /*Only INT8/UINT8 support XY DP9 now*/
            else if ((hasXYDP9 || hasXYDP6)
                && (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8))
            {
                vx_uint32 xStep = 0, yStep = 0;
                vx_uint32 inImageBufferSize = hasXYDP9 ? 2 : 1;

                /* xydp9 xydp6 can not enable same time */
                vxmASSERT(!(hasXYDP9 && hasXYDP6));

                if (hasXYDP6)
                {
                    xStep = 3;
                    yStep = 2;
                }
                else
                {
                    xStep = 3;
                    yStep = 3;
                }

                for (sliceIndex = 0; sliceIndex < slice_count; sliceIndex += inImageBufferSize)
                {
                    vx_uint32 weight = 0;
                    vx_uint32 realSliceIndex = 0;
                    vx_uint32 realWeightXIndex = 0, realWeightYIndex = 0;
                    vx_uint32 subKxSize = 0, subKySize = 0, subKzSize = 0;

                    subKzSize = gcmMIN((slice_count - sliceIndex), inImageBufferSize);
                    /* add slices of every filter*/
                    for (filterIndex = filterStart; filterIndex <= filterEnd; filterIndex++)
                    {
                        for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex += yStep)
                        {
                            subKySize = gcmMIN((wb->weights_sizes[1] - weightYIndex), yStep);
                            for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex += xStep)
                            {
                                subKxSize = gcmMIN((wb->weights_sizes[0] - weightXIndex), xStep);
                                for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + subKzSize; realSliceIndex++)
                                {
                                    vx_uint32 nonZeroInBlockCount = 0; /*Check if there's non-zero point in one 3x3 block*/
                                    /* add one slice data every filter */
                                    kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * realSliceIndex;
                                    if (realSliceIndex == 0)
                                    {
                                        if (bias_base_ptr)
                                            biasData = *(bias_base_ptr + filterIndex);
                                        else
                                            biasData = 0;

                                        /* NewBias = sum((Coef[i] - CoefZP) * - InZP) + Bias*/
                                        if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT)
                                            && inputZP != 0
                                            && weight_format == VX_TYPE_UINT8)
                                        {
                                            biasData -= weightsMinusZP[filterIndex].sum;
                                        }
                                    }

                                    for (realWeightYIndex = weightYIndex; realWeightYIndex < weightYIndex + subKySize; realWeightYIndex++)
                                    {
                                        for (realWeightXIndex = weightXIndex; realWeightXIndex < weightXIndex + subKxSize; realWeightXIndex++)
                                        {
                                            vx_uint8_ptr realKernelDataPtr = kernelDataPtr + (realWeightYIndex * wb->weights_sizes[0] + realWeightXIndex) * weightSize;

                                            if (weight_format == VX_TYPE_INT8)
                                                weight = *((vx_int8 *)realKernelDataPtr);
                                            else if (weight_format == VX_TYPE_UINT8)
                                                weight = *((vx_uint8 *)realKernelDataPtr);
                                            else
                                                weight = *((vx_uint16 *)realKernelDataPtr);

                                            if (weight != skipValue)
                                                nonZeroInBlockCount++;

                                            if (!hasBiasAtEnd &&
                                                realWeightYIndex == 0 &&
                                                realWeightXIndex == 0 &&
                                                realSliceIndex == 0)
                                            {
                                                *((vx_uint32 *)kernelBufferInt8Ptr) = biasData;
                                                kernelBufferInt8Ptr += 4;
                                                reoder_stream_count_per_core[coreIndex] += (biasBitSize / weightBitSize);
                                                for (idx = 0; idx < (biasBitSize / weightBitSize); idx++)
                                                {
                                                    non_coef_index[nonCoefIndex++] = elementIndex++;
                                                }
                                            }
                                            *kernelBufferInt8Ptr = (vx_uint8)weight;
                                            kernelBufferInt8Ptr++;
                                            reoder_stream_count_per_core[coreIndex] += 1;

                                            if (filterIndex == filterEnd && realSliceIndex == 0 &&
                                                realWeightXIndex == 0 && realWeightYIndex == 0)
                                                zrl_limit_index[limitZRLIndex++] = elementIndex;

                                            elementIndex++;

                                            /* add offet behind the last point of the last slice of each filter */
                                            if (realSliceIndex == slice_count - 1 &&
                                                realWeightXIndex == wb->weights_sizes[0] - 1 &&
                                                realWeightYIndex == wb->weights_sizes[1] - 1)
                                            {
                                                vx_uint32 offsetValue;

                                                if (hasNNPerFilterPostMultiply)
                                                {
                                                    /*add post multiply to each filter*/

                                                    vx_uint32 postMulSize = 8;
                                                    *kernelBufferInt8Ptr = postMul[filterIndex];
                                                    kernelBufferInt8Ptr++;
                                                    reoder_stream_count_per_core[coreIndex] += (postMulSize / weightBitSize);
                                                    for (idx = 0; idx < (postMulSize / weightBitSize); idx++)
                                                    {
                                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                                    }
                                                }
                                                if (hasNNPerFilterPostShift)
                                                {
                                                    /*add post multiply to each filter*/

                                                    vx_uint32 postShiftSize = 8;
                                                    *kernelBufferInt8Ptr = postShift[filterIndex];
                                                    kernelBufferInt8Ptr++;
                                                    reoder_stream_count_per_core[coreIndex] += (postShiftSize / weightBitSize);
                                                    for (idx = 0; idx < (postShiftSize / weightBitSize); idx++)
                                                    {
                                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                                    }
                                                }

                                                if (z_offset > 0)
                                                    offsetValue = (vx_uint32)z_offset * filterIndex;
                                                else if (output_size > 0)
                                                    offsetValue = output_final_x * output_final_y * output_size * filterIndex;
                                                else
                                                    offsetValue = output_final_x * output_final_y * weightSize * filterIndex;

                                                if (hasBiasAtEnd)
                                                {
                                                    *((vx_uint32 *)kernelBufferInt8Ptr) = biasData;
                                                    kernelBufferInt8Ptr += 4;
                                                    reoder_stream_count_per_core[coreIndex] += (biasBitSize / weightBitSize);
                                                    for (idx = 0; idx < (biasBitSize / weightBitSize); idx++)
                                                    {
                                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                                    }
                                                }

                                                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                                {
                                                    *((vx_uint32 *)kernelBufferInt8Ptr) = offsetValue;
                                                    kernelBufferInt8Ptr += 4;
                                                    reoder_stream_count_per_core[coreIndex] += (NN_Z_POSITION_OFFSET_BITS_VIP_V7 / weightBitSize);
                                                    for (idx = 0; idx < (NN_Z_POSITION_OFFSET_BITS_VIP_V7 / weightBitSize); idx++)
                                                    {
                                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                                    }
                                                }

                                            }
                                        }
                                    }
                                    blockCount++;
                                    if (nonZeroInBlockCount)
                                        nonZeroCount++;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                for (sliceIndex = 0; sliceIndex < slice_count; sliceIndex++)
                {
                    /* add slices of every filter*/
                    for (filterIndex = filterStart; filterIndex <= filterEnd; filterIndex++)
                    {

                        /* add one slice data every filter */
                        kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * sliceIndex;
                        if (sliceIndex == 0)
                        {
                            if (bias_base_ptr)
                            {
                                if (bias_format == VX_TYPE_INT64)
                                    biasData64 = *(((vx_int64 *)bias_base_ptr) + filterIndex);
                                else
                                    biasData = *(bias_base_ptr + filterIndex);
                            }
                            else
                                biasData = 0;

                            /* NewBias = sum((Coef[i] - CoefZP) * - InZP) + Bias*/
                            if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT)
                                && inputZP != 0
                                && weight_format == VX_TYPE_UINT8)
                            {
                                biasData -= weightsMinusZP[filterIndex].sum;
                            }
                        }

                        for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex++)
                        {
                            vx_uint32 nonZeroCountInDP3 = 0; /*Check if there's non-zero point in one 3x1 block*/
                            for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex++)
                            {
                                vx_uint32 weight = 0;

                                if (weight_format == VX_TYPE_INT8)
                                    weight = *((vx_int8 *)kernelDataPtr);
                                else if (weight_format == VX_TYPE_UINT8)
                                    weight = *((vx_uint8 *)kernelDataPtr);
                                else
                                    weight = *((vx_uint16 *)kernelDataPtr);
                                kernelDataPtr = kernelDataPtr + weightSize;

                                {
                                    if (weight != skipValue)
                                        nonZeroCountInDP3++;

                                    /*V7, DP3, one block is 3x1*/
                                    if ((weightXIndex + 1) % 3 == 0 || weightXIndex == wb->weights_sizes[0] - 1)
                                    {
                                        blockCount++;
                                        if (nonZeroCountInDP3)
                                            nonZeroCount++;
                                        nonZeroCountInDP3 = 0;
                                    }
                                }

                                if (!hasBiasAtEnd &&
                                    weightXIndex == 0 &&
                                    weightYIndex == 0 &&
                                    sliceIndex == 0)
                                {
                                    if (weight_format == VX_TYPE_INT16)
                                    {
                                        vx_int64 bias64 = 0;
                                        vx_int32 bias32;
                                        vx_int16 bias16;

                                        if (bias_format == VX_TYPE_INT64)
                                            bias64 = biasData64;
                                        else
                                        {
                                            if ((vx_int32)biasData < 0)
                                            {
                                                bias64 = ~0;
                                                bias64 = (bias64 >> 32) << 32;
                                                bias64 = bias64 | (vx_int64)biasData;
                                            }
                                            else
                                                bias64 = (vx_int64)biasData;
                                        }
                                        bias32 = (vx_int32)bias64;
                                        *((vx_uint32 *)kernelBufferInt16Ptr) = bias32;
                                        kernelBufferInt16Ptr += 2;
                                        reoder_stream_count_per_core[coreIndex] += (32 / weightBitSize);
                                        bias16 = (vx_int16)(bias64 >> 32);
                                        *kernelBufferInt16Ptr = bias16;
                                        kernelBufferInt16Ptr++;
                                        reoder_stream_count_per_core[coreIndex] += (16 / weightBitSize);
                                    }
                                    else if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
                                    {
                                        *((vx_uint32 *)kernelBufferInt8Ptr) = biasData;
                                        kernelBufferInt8Ptr += 4;
                                        reoder_stream_count_per_core[coreIndex] += (biasBitSize / weightBitSize);
                                    }
                                    else
                                    {
                                        *((vx_uint32 *)kernelBufferInt16Ptr) = biasData;
                                        kernelBufferInt16Ptr += 2;
                                        reoder_stream_count_per_core[coreIndex] += (biasBitSize / weightBitSize);
                                    }

                                    for (idx = 0; idx < (biasBitSize / weightBitSize); idx++)
                                    {
                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                    }
                                }

                                if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
                                {
                                    *kernelBufferInt8Ptr = (vx_uint8)weight;
                                    kernelBufferInt8Ptr++;
                                }
                                else
                                {
                                    if (weight_format == VX_TYPE_FLOAT16)
                                        weight = (weight & 0x7fff) * 2 + weight/(1<<15);

                                    *kernelBufferInt16Ptr = (vx_uint16)weight;
                                    kernelBufferInt16Ptr++;
                                }
                                reoder_stream_count_per_core[coreIndex] += 1;

                                if (filterIndex == filterEnd && sliceIndex == 0 &&
                                    weightXIndex == 0 && weightYIndex == 0)
                                    zrl_limit_index[limitZRLIndex++] = elementIndex;

                                elementIndex++;
                            }
                        }

                        /* add offet behind the last point of the last slice of each filter */
                        if (sliceIndex == slice_count - 1)
                        {
                            vx_uint32 offsetValue;

                            if (hasNNPerFilterPostMultiply && hasNNPerFilterPostShift)
                            {
                                if (weight_format == VX_TYPE_INT16 || weight_format == VX_TYPE_FLOAT16)
                                {
                                    vx_uint32 postMulSize = 16;
                                    vx_uint32 postShiftSize = 16;
                                    /*add post multiply to each filter*/
                                    *kernelBufferInt16Ptr = (vx_uint16)postMul[filterIndex];
                                    kernelBufferInt16Ptr++;
                                    reoder_stream_count_per_core[coreIndex] += (postMulSize / weightBitSize);
                                    /*add post multiply to each filter*/
                                    *kernelBufferInt16Ptr = (vx_uint16)postShift[filterIndex];
                                    kernelBufferInt16Ptr++;
                                    reoder_stream_count_per_core[coreIndex] += (postShiftSize / weightBitSize);
                                }
                                else
                                {
                                    vx_uint32 postMulSize = 8;
                                    vx_uint32 postShiftSize = 8;
                                    /*add post multiply to each filter*/
                                    *kernelBufferInt8Ptr = postMul[filterIndex];
                                    kernelBufferInt8Ptr++;
                                    reoder_stream_count_per_core[coreIndex] += (postMulSize / weightBitSize);
                                    /*add post multiply to each filter*/
                                    *kernelBufferInt8Ptr = postShift[filterIndex];
                                    kernelBufferInt8Ptr++;
                                    reoder_stream_count_per_core[coreIndex] += (postShiftSize / weightBitSize);
                                }

                                for (idx = 0; idx < 2; idx++)
                                {
                                    non_coef_index[nonCoefIndex++] = elementIndex++;
                                }
                            }

                            if (z_offset > 0)
                                offsetValue = (vx_uint32)z_offset * filterIndex;
                            else if (output_size > 0)
                                offsetValue = output_final_x * output_final_y * output_size * filterIndex;
                            else
                                offsetValue = output_final_x * output_final_y * weightSize * filterIndex;

                            if (hasBiasAtEnd)
                            {
                                if (weight_format == VX_TYPE_INT16)
                                {
                                    vx_int64 bias64 = 0;
                                    vx_int32 bias32;
                                    vx_int16 bias16;

                                    if ((vx_int32)biasData < 0)
                                    {
                                        bias64 = ~0;
                                        bias64 = (bias64 >> 32) << 32;
                                        bias64 = bias64 | (vx_int64)biasData;
                                    }
                                    else
                                        bias64 = (vx_int64)biasData;

                                    bias32 = (vx_int32)bias64;
                                    *((vx_uint32 *)kernelBufferInt16Ptr) = bias32;
                                    kernelBufferInt16Ptr += 2;
                                    reoder_stream_count_per_core[coreIndex] += (32 / weightBitSize);
                                    bias16 = (vx_int16)(bias64 >> 32);
                                    *kernelBufferInt16Ptr = bias16;
                                    kernelBufferInt16Ptr++;
                                    reoder_stream_count_per_core[coreIndex] += (16 / weightBitSize);
                                }
                                else if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
                                {
                                    *((vx_uint32 *)kernelBufferInt8Ptr) = biasData;
                                    kernelBufferInt8Ptr += 4;
                                    reoder_stream_count_per_core[coreIndex] += (biasBitSize / weightBitSize);
                                }
                                else
                                {
                                    *((vx_uint32 *)kernelBufferInt16Ptr) = biasData;
                                    kernelBufferInt16Ptr += 2;
                                    reoder_stream_count_per_core[coreIndex] += (biasBitSize / weightBitSize);
                                }

                                for (idx = 0; idx < (biasBitSize / weightBitSize); idx++)
                                {
                                    non_coef_index[nonCoefIndex++] = elementIndex++;
                                }
                            }

                            if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                            {
                                if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
                                {
                                    *((vx_uint32 *)kernelBufferInt8Ptr) = offsetValue;
                                    kernelBufferInt8Ptr += 4;
                                }
                                else
                                {
                                    *((vx_uint32 *)kernelBufferInt16Ptr) = offsetValue;
                                    kernelBufferInt16Ptr += 2;
                                }
                                reoder_stream_count_per_core[coreIndex] += (NN_Z_POSITION_OFFSET_BITS_VIP_V7 / weightBitSize);
                                for (idx = 0; idx < (NN_Z_POSITION_OFFSET_BITS_VIP_V7 / weightBitSize); idx++)
                                {
                                    non_coef_index[nonCoefIndex++] = elementIndex++;
                                }
                            }
                        }
                    }
                }
            }
            if (calc_perf && (blockCount != 0))
                nonZeroRatioPerCorePerVZG[nonZrlIdx] = (vx_float64)nonZeroCount / (vx_float64)blockCount;
        }
    }

    if (calc_perf)
    {
        for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
        {
            for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
            {
                vx_uint32 nonZrlIdx = groupIndex + coreIndex * groupCount;
                if (wb->max_per_core_per_vzgroup_nzr[groupIndex] < nonZeroRatioPerCorePerVZG[nonZrlIdx])
                    wb->max_per_core_per_vzgroup_nzr[groupIndex] = nonZeroRatioPerCorePerVZG[nonZrlIdx];
            }
        }
    }

#if DEBUG_COMP_ENHANCE_ENC
    {
        unsigned char * pChar = (unsigned char*)reorder_stream;
        unsigned char data;
        vx_uint32 i;
        vx_uint32 totalByteCount = weightCount * slice_count * z_count * weightSize + z_count * 4 + z_count * (biasBitSize / 8) + nnCoreCount * 2;
        FILE * pfile1 = NULL;
        pfile1 = fopen("reordered_kernel.txt", "wb");

        if(pfile1 != NULL)
        {
            for(i=0; i!=totalByteCount; i++)
            {
                data = *pChar;
                fprintf(pfile1, "coef: 0x%x\n", data);
                pChar++;
            }

            fclose(pfile1);
        }

    }
#endif

exit:
    if (weightsMinusZP)
        vxFree(weightsMinusZP);
    if (nonZeroRatioPerCorePerVZG)
        vxFree(nonZeroRatioPerCorePerVZG);
    return;
}

#define PRINT_PER_CORE_SIZE 0
void reorderKernelBufferV7HuffmanBalance(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_int32 z_offset,
    vx_int32 output_size,
    vx_uint32 skip_value,
    vx_uint32 output_final_x,
    vx_uint32 output_final_y,
    vx_uint8_ptr reorder_stream,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_uint8* postMul,
    vx_uint8* postShift,
    vx_uint32* reoder_stream_count_per_core,
    vx_uint32* non_coef_index,
    vx_uint32* zrl_limit_index,
    vx_bool calc_perf
    )
{
    vx_uint32 nnCoreCount = weight_format == VX_TYPE_INT16 ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : weight_format == VX_TYPE_FLOAT16 ?
                      context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 sliceCount         = slice_count;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;
    vx_uint32 filterCount        = filters_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;

    vx_uint32 weightCount        = wb->weights_sizes[0] * wb->weights_sizes[1];
    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize((vx_type_e)weight_format);
    vx_uint32 weightBitSize      = weightSize * 8;
    vx_uint32 biasSize           = (vx_uint32)vxDataType_GetSize((vx_type_e)bias_format);
    vx_uint32 biasBitSize        = biasSize * 8;
    vx_uint32 biasData           = 0;
    vx_uint64 biasData64         = 0;

    vx_uint32 filterSliceSize    = weightCount * weightSize;
    vx_uint32 filterSize         = weightCount * weightSize * slice_count;
    vx_uint8* kernelDataPtr      = VX_NULL;
    vx_uint8* kernelBufferInt8Ptr    = reorder_stream;
    vx_uint16* kernelBufferInt16Ptr    = (vx_uint16*)reorder_stream;

    vx_uint32 inputZP = wb->wb_base->inputZP;
    vx_uint32 coefZP = wb->wb_base->coefZP;

    vx_uint32 coreIndex;
    vx_uint32 elementIndex = 0;
    vx_uint32 nonCoefIndex = 0;
    vx_uint32 limitZRLIndex = 0;
    vx_uint32 idx = 0;
    vx_uint32 filterIndex, sliceIndex;
    vx_uint32 weightXIndex, weightYIndex;
    vx_uint32 skipValue = skip_value;

    vx_bool hasZDP3 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3);
    vx_bool hasZDP6 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6);
    vx_bool hasXYDP9 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP9);
    vx_bool hasXYDP6 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP6);
    vx_bool hasBiasAtEnd = vx_false_e;
    vx_bool hasNNPerFilterPostMultiply = vx_false_e;
    vx_bool hasNNPerFilterPostShift = vx_false_e;

    vx_uint32 totalNonZeroCount = 0;
    vx_float32 averageNZCPerFilter = 0;
    vx_float32 varianceFL32 = 0;
    vx_uint32 variance = 0;
    vx_bool reorder = vx_false_e;

    vx_float64* nonZeroRatioPerCorePerVZG = VX_NULL;
    vx_uint32 groupIndex;

    typedef struct _gcVXWeightsMinusZP
    {
        vx_int32 sum;
        vx_uint32 filterIdx;
    }gcVXWeightsMinusZP;

    gcVXWeightsMinusZP *weightsMinusZP = VX_NULL;

    typedef struct _gcVXNonZeroWeights
    {
        vx_uint32 nonZeroCount;
        vx_uint32 filterIdx;
    }gcVXNonZeroWeights;

#if PRINT_PER_CORE_SIZE
    vx_uint32* nonZeroCoefPerCore = VX_NULL;
#endif

    gcVXNonZeroWeights *nonZeroWeights = VX_NULL;
    vx_int32 i = 0;
    vx_uint32 core0FilterCount = 0;
    gcVXNonZeroWeights tmp;

    nonZeroWeights = (gcVXNonZeroWeights*)vxAllocateAndZeroMemory(filterTotalCount * sizeof(gcVXNonZeroWeights));
    if (nonZeroWeights == VX_NULL)
    {
        vxError("fillinKernelBuffer: OUT OF MEMORY");
        goto exit;
    }
#if PRINT_PER_CORE_SIZE
    nonZeroCoefPerCore = (vx_uint32*)vxAllocateAndZeroMemory(nnCoreCount * sizeof(vx_uint32));
    if (nonZeroCoefPerCore == VX_NULL)
    {
        vxError("fillinKernelBuffer: OUT OF MEMORY");
        goto exit;
    }
#endif

    /* calc each filter non-zero weights num*/
    for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
    {
        nonZeroWeights[filterIndex].filterIdx = filterIndex;
        for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
        {
            /* add one slice data every filter */
            kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * sliceIndex;

            for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex++)
            {
                for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex++)
                {
                    vx_uint32 weight = 0;

                    if (weight_format == VX_TYPE_INT8)
                        weight = *((vx_int8 *)kernelDataPtr);
                    else if (weight_format == VX_TYPE_UINT8)
                        weight = *((vx_uint8 *)kernelDataPtr);
                    else
                        weight = *((vx_uint16 *)kernelDataPtr);
                    kernelDataPtr = kernelDataPtr + weightSize;

                    if (weight != skipValue)
                        nonZeroWeights[filterIndex].nonZeroCount++;

                }
            }
        }
        totalNonZeroCount += nonZeroWeights[filterIndex].nonZeroCount;
    }

    /*Sort*/
    averageNZCPerFilter = (vx_float32)totalNonZeroCount / filterTotalCount;
    for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
    {
        varianceFL32 += gcoMATH_Power((nonZeroWeights[filterIndex].nonZeroCount - averageNZCPerFilter), 2);
    }
    variance = (vx_uint32)(varianceFL32 / filterTotalCount + 0.5f);

    reorder = vx_true_e;

    if (reorder)
    {
        /*Sort*/
        for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
        {
            tmp.nonZeroCount = nonZeroWeights[filterIndex].nonZeroCount;
            tmp.filterIdx = filterIndex;

            for (i = filterIndex - 1; i >= 0 && nonZeroWeights[i].nonZeroCount > tmp.nonZeroCount; i--)
            {
                nonZeroWeights[i+1].nonZeroCount = nonZeroWeights[i].nonZeroCount;
                nonZeroWeights[i+1].filterIdx = nonZeroWeights[i].filterIdx;
            }
            nonZeroWeights[i+1].nonZeroCount = tmp.nonZeroCount;
            nonZeroWeights[i+1].filterIdx = tmp.filterIdx;
        }
    }

    if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
    {
        if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
        {
            biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7;
        }
        else
        {
            biasBitSize = NN_INTEGER_BIAS_BITS;
        }
    }
    else if (weight_format == VX_TYPE_INT16)
    {
        biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7_INT16;
    }

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT) && inputZP != 0 && weight_format == VX_TYPE_UINT8)
    {
        weightsMinusZP = (gcVXWeightsMinusZP*)vxAllocateAndZeroMemory(filterTotalCount * sizeof(gcVXWeightsMinusZP));

        if (!weightsMinusZP)
        {
            vxError("fillinKernelBuffer: OUT OF MEMORY");
            goto exit;
        }

        /*Calc sum((coef[i] - coefZP) * InZP) */
        for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
        {
            weightsMinusZP[filterIndex].filterIdx = filterIndex;
            weightsMinusZP[filterIndex].sum = 0;
            for (sliceIndex = 0; sliceIndex < slice_count; sliceIndex++)
            {
                kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * sliceIndex;

                for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex++)
                {
                    for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex++)
                    {
                        vx_int32 weight = 0;
                        weight = *((vx_uint8 *)kernelDataPtr);
                        kernelDataPtr = kernelDataPtr + weightSize;

                        weightsMinusZP[filterIndex].sum += (weight - coefZP) * inputZP;
                    }
                }
            }
        }
    }

    if (calc_perf)
    {
        if (wb->max_per_core_per_vzgroup_nzr == VX_NULL)
        {
            wb->max_per_core_per_vzgroup_nzr = (vx_float64*)vxAllocateAndZeroMemory(groupCount * sizeof(vx_float64));
            if (wb->max_per_core_per_vzgroup_nzr == VX_NULL)
            {
                vxError("reorderWeightBiasBufferForHuffman: OUT OF MEMORY\n");
                goto exit;
            }
        }

        nonZeroRatioPerCorePerVZG = (vx_float64*)vxAllocateAndZeroMemory(nnCoreCount * groupCount * sizeof(vx_float64));
        if (nonZeroRatioPerCorePerVZG == VX_NULL)
        {
            vxError("reorderWeightBiasBufferForHuffman: OUT OF MEMORY");
            goto exit;
        }
    }

    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_uint32 coreFilterCount = (coreIndex < oddFilterPerCore) ? totalFilterPerCore + 1 : totalFilterPerCore;

        vx_uint32 usedKid = 0;
        vx_uint32* filterGroup = VX_NULL;

        reoder_stream_count_per_core[coreIndex] = 0;

        if (coreFilterCount > 0)
        {
            /* Save kernels per core*/
            if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
            {
                *((vx_uint16 *)kernelBufferInt8Ptr) = (vx_uint16)coreFilterCount;
                kernelBufferInt8Ptr += 2;
                reoder_stream_count_per_core[coreIndex] += (16 / weightBitSize);
            }
            else
            {
                *kernelBufferInt16Ptr = (vx_uint16)coreFilterCount;
                kernelBufferInt16Ptr ++;
                reoder_stream_count_per_core[coreIndex] += (16 / weightBitSize);
            }
            for (idx = 0; idx < (16 / weightBitSize); idx++)
            {
                non_coef_index[nonCoefIndex++] = elementIndex++;
            }
        }

        for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
        {
            vx_uint32 blockCount = 0;
            vx_uint32 nonZeroCount = 0;
            vx_uint32 nonZrlIdx = groupIndex + coreIndex * groupCount;

            vx_uint32 actualFilterCount = (groupIndex == groupCount - 1) ? (coreFilterCount - groupIndex * filterCount) : filterCount;
            vx_uint32 groupFilterStart = groupIndex * nnCoreCount * filterCount;
            vx_uint32 kid = 0;
            vx_uint32 unusedCoreCount = (filterTotalCount % nnCoreCount == 0) ? 0 : nnCoreCount - (filterTotalCount % nnCoreCount);
            vx_uint32 sortedIndex = 0;
            vx_uint32 filterStart = 0;

            if (coreIndex == 0)
                core0FilterCount = actualFilterCount;

            if (actualFilterCount)
            {
                filterGroup = (vx_uint32*)vxAllocateAndZeroMemory(actualFilterCount * sizeof(vx_uint32));
                if (filterGroup == VX_NULL)
                {
                    vxError("fillinKernelBufferBalance: OUT OF MEMORY");
                    goto exit;
                }
            }

            if (reorder)
            {
                /*Dispatch filter to each vzGroup*/
                for (kid = 0; kid < actualFilterCount; kid++)
                {
                    if (groupIndex == groupCount - 1 &&
                        kid == core0FilterCount - 1)
                    {
                        if ((kid + usedKid) % 2 == 0)
                            sortedIndex = groupFilterStart + kid * nnCoreCount + coreIndex;
                        else
                            sortedIndex = groupFilterStart + (kid + 1) * nnCoreCount - coreIndex - 1 - unusedCoreCount;
                    }
                    else
                    {
                        if ((kid + usedKid) % 2 == 0)
                            sortedIndex = groupFilterStart + kid * nnCoreCount + coreIndex;
                        else
                            sortedIndex = groupFilterStart + (kid + 1) * nnCoreCount - coreIndex - 1;
                    }

                    filterGroup[kid] = nonZeroWeights[sortedIndex].filterIdx;
#if PRINT_PER_CORE_SIZE
                    nonZeroCoefPerCore[coreIndex] += nonZeroWeights[sortedIndex].nonZeroCount;
#endif
                }

                /*sort the vz group real filter to avoid write output cross*/
                for (kid = 0; kid < actualFilterCount; kid++)
                {
                    vx_uint32 tmp = filterGroup[kid];

                    for (i = kid - 1; i >= 0 && filterGroup[i] > tmp; i--)
                    {
                        filterGroup[i+1] = filterGroup[i];
                    }
                    filterGroup[i+1] = tmp;
                }
            }
            else
            {
                /*if variance is small, don't reorder the filters*/
                if ((groupIndex == groupCount - 1)
                && (coreIndex >= (filterTotalCount % nnCoreCount)))
                {
                    filterStart = groupIndex * nnCoreCount * filterCount
                                + (filterTotalCount % nnCoreCount) * (actualFilterCount + 1)
                                + (coreIndex - (filterTotalCount % nnCoreCount)) * actualFilterCount;
                }
                else
                {
                    filterStart = groupIndex * nnCoreCount *filterCount + coreIndex * actualFilterCount;
                }

                for (kid = 0; kid < actualFilterCount; kid++)
                {
                    filterGroup[kid] = filterStart + kid;
                }
            }


            if (wb->weights_sizes[0] == 1 && wb->weights_sizes[1] == 1 &&
                (hasZDP3 || hasZDP6) &&
                (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8))
            {
                vx_uint32 zdpNum = hasZDP6 ? 6 : 3;

                /* zdp3 zdp6 can not enable same time */
                vxmASSERT(!(hasZDP3 && hasZDP6));

                for (sliceIndex = 0; sliceIndex < slice_count; sliceIndex += (zdpNum * 2))
                {
                    vx_uint32 weight = 0;
                    vx_uint32 realSliceIndex = 0;

                    /* add slices of every filter*/
                    for (kid = 0; kid < actualFilterCount; kid++)
                    {
                        vx_uint32 nonZeroInBlock1Count = 0; /*Check if there's non-zero point in first zdpNum block*/
                        vx_uint32 nonZeroInBlock2Count = 0; /*Check if there's non-zero point in another zdpNum block*/

                        filterIndex = filterGroup[kid];

                        for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + (zdpNum * 2); realSliceIndex++)
                        {
                            if (realSliceIndex >= slice_count)
                                break;

                            if (realSliceIndex == 0)
                            {
                                if (bias_base_ptr)
                                    biasData = *(bias_base_ptr + filterIndex);
                                else
                                    biasData = 0;

                                /* NewBias = sum((Coef[i] - CoefZP) * - InZP) + Bias*/
                                if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT)
                                    && inputZP != 0
                                    && weight_format == VX_TYPE_UINT8)
                                {
                                    biasData -= weightsMinusZP[filterIndex].sum;
                                }

                                if (!hasBiasAtEnd)
                                {
                                    *((vx_uint32 *)kernelBufferInt8Ptr) = biasData;
                                    kernelBufferInt8Ptr += 4;
                                    reoder_stream_count_per_core[coreIndex] += (biasBitSize / weightBitSize);
                                    for (idx = 0; idx < (biasBitSize / weightBitSize); idx++)
                                    {
                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                    }
                                }
                            }

                            /* add one slice data every filter */
                            kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * realSliceIndex;
                            if (weight_format == VX_TYPE_INT8)
                                weight = *((vx_int8 *)kernelDataPtr);
                            else if (weight_format == VX_TYPE_UINT8)
                                weight = *((vx_uint8 *)kernelDataPtr);
                            else
                                weight = *((vx_uint16 *)kernelDataPtr);

                            if (realSliceIndex - sliceIndex < zdpNum && weight != skipValue)
                                nonZeroInBlock1Count++;
                            else if (weight != skipValue)
                                nonZeroInBlock2Count++;

                            *kernelBufferInt8Ptr = (vx_uint8)weight;
                            kernelBufferInt8Ptr++;
                            reoder_stream_count_per_core[coreIndex] += 1;

                            if ((kid == actualFilterCount - 1) && realSliceIndex == 0)
                                zrl_limit_index[limitZRLIndex++] = elementIndex;

                            elementIndex++;

                            /* add offet behind the last point of the last slice of each filter */
                            if (realSliceIndex == slice_count - 1)
                            {
                                vx_uint32 offsetValue;

                                if (hasNNPerFilterPostMultiply)
                                {
                                    /*add post multiply to each filter*/

                                    vx_uint32 postMulSize = 8;
                                    *kernelBufferInt8Ptr = postMul[filterIndex];
                                    kernelBufferInt8Ptr++;
                                    reoder_stream_count_per_core[coreIndex] += (postMulSize / weightBitSize);
                                    for (idx = 0; idx < (postMulSize / weightBitSize); idx++)
                                    {
                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                    }
                                }
                                if (hasNNPerFilterPostShift)
                                {
                                    /*add post multiply to each filter*/

                                    vx_uint32 postShiftSize = 8;
                                    *kernelBufferInt8Ptr = postShift[filterIndex];
                                    kernelBufferInt8Ptr++;
                                    reoder_stream_count_per_core[coreIndex] += (postShiftSize / weightBitSize);
                                    for (idx = 0; idx < (postShiftSize / weightBitSize); idx++)
                                    {
                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                    }
                                }

                                if (z_offset > 0)
                                    offsetValue = (vx_uint32)z_offset * filterIndex;
                                else if (output_size > 0)
                                    offsetValue = output_final_x * output_final_y * output_size * filterIndex;
                                else
                                    offsetValue = output_final_x * output_final_y * weightSize * filterIndex;

                                if (hasBiasAtEnd)
                                {
                                    *((vx_uint32 *)kernelBufferInt8Ptr) = biasData;
                                    kernelBufferInt8Ptr += 4;
                                    reoder_stream_count_per_core[coreIndex] += (biasBitSize / weightBitSize);

                                    for (idx = 0; idx < (biasBitSize / weightBitSize); idx++)
                                    {
                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                    }
                                }

                                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                {
                                    *((vx_uint32 *)kernelBufferInt8Ptr) = offsetValue;
                                    kernelBufferInt8Ptr += 4;
                                    reoder_stream_count_per_core[coreIndex] += (NN_Z_POSITION_OFFSET_BITS_VIP_V7 / weightBitSize);
                                    for (idx = 0; idx < (NN_Z_POSITION_OFFSET_BITS_VIP_V7 / weightBitSize); idx++)
                                    {
                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                    }
                                }
                            }
                        }
                        if (sliceIndex + zdpNum >= slice_count)
                            blockCount++;
                        else
                            blockCount += 2;

                        if (nonZeroInBlock1Count)
                            nonZeroCount++;
                        if (nonZeroInBlock2Count)
                            nonZeroCount++;
                    }
                }
            }
            /*Only INT8/UINT8 support XY DP9 now*/
            else if ((hasXYDP9 || hasXYDP6)
                && (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8))
            {
                vx_uint32 xStep = 0, yStep = 0;
                vx_uint32 inImageBufferSize = hasXYDP9 ? 2 : 1;

                /* xydp9 xydp6 can not enable same time */
                vxmASSERT(!(hasXYDP9 && hasXYDP6));

                if (hasXYDP6)
                {
                    xStep = 3;
                    yStep = 2;
                }
                else
                {
                    xStep = 3;
                    yStep = 3;
                }

                for (sliceIndex = 0; sliceIndex < slice_count; sliceIndex += inImageBufferSize)
                {
                    vx_uint32 weight = 0;
                    vx_uint32 realSliceIndex = 0;
                    vx_uint32 realWeightXIndex = 0, realWeightYIndex = 0;
                    vx_uint32 subKxSize = 0, subKySize = 0, subKzSize = 0;

                    subKzSize = gcmMIN((slice_count - sliceIndex), inImageBufferSize);
                    /* add slices of every filter*/
                    for (kid = 0; kid < actualFilterCount; kid++)
                    {
                        filterIndex = filterGroup[kid];
                        for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex += yStep)
                        {
                            subKySize = gcmMIN((wb->weights_sizes[1] - weightYIndex), yStep);
                            for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex += xStep)
                            {
                                subKxSize = gcmMIN((wb->weights_sizes[0] - weightXIndex), xStep);
                                for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + subKzSize; realSliceIndex++)
                                {
                                    vx_uint32 nonZeroInBlockCount = 0; /*Check if there's non-zero point in one 3x3 block*/
                                    /* add one slice data every filter */
                                    kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * realSliceIndex;
                                    if (realSliceIndex == 0)
                                    {
                                        if (bias_base_ptr)
                                            biasData = *(bias_base_ptr + filterIndex);
                                        else
                                            biasData = 0;

                                        /* NewBias = sum((Coef[i] - CoefZP) * - InZP) + Bias*/
                                        if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT)
                                            && inputZP != 0
                                            && weight_format == VX_TYPE_UINT8)
                                        {
                                            biasData -= weightsMinusZP[filterIndex].sum;
                                        }
                                    }

                                    for (realWeightYIndex = weightYIndex; realWeightYIndex < weightYIndex + subKySize; realWeightYIndex++)
                                    {
                                        for (realWeightXIndex = weightXIndex; realWeightXIndex < weightXIndex + subKxSize; realWeightXIndex++)
                                        {
                                            vx_uint8_ptr realKernelDataPtr = kernelDataPtr + (realWeightYIndex * wb->weights_sizes[0] + realWeightXIndex) * weightSize;

                                            if (weight_format == VX_TYPE_INT8)
                                                weight = *((vx_int8 *)realKernelDataPtr);
                                            else if (weight_format == VX_TYPE_UINT8)
                                                weight = *((vx_uint8 *)realKernelDataPtr);
                                            else
                                                weight = *((vx_uint16 *)realKernelDataPtr);

                                            if (weight != skipValue)
                                                nonZeroInBlockCount++;

                                            if (!hasBiasAtEnd &&
                                                realWeightYIndex == 0 &&
                                                realWeightXIndex == 0 &&
                                                realSliceIndex == 0)
                                            {
                                                *((vx_uint32 *)kernelBufferInt8Ptr) = biasData;
                                                kernelBufferInt8Ptr += 4;
                                                reoder_stream_count_per_core[coreIndex] += (biasBitSize / weightBitSize);
                                                for (idx = 0; idx < (biasBitSize / weightBitSize); idx++)
                                                {
                                                    non_coef_index[nonCoefIndex++] = elementIndex++;
                                                }
                                            }
                                            *kernelBufferInt8Ptr = (vx_uint8)weight;
                                            kernelBufferInt8Ptr++;
                                            reoder_stream_count_per_core[coreIndex] += 1;

                                            if ((kid == actualFilterCount - 1) && realSliceIndex == 0 &&
                                                realWeightXIndex == 0 && realWeightYIndex == 0)
                                                zrl_limit_index[limitZRLIndex++] = elementIndex;

                                            elementIndex++;

                                            /* add offet behind the last point of the last slice of each filter */
                                            if (realSliceIndex == slice_count - 1 &&
                                                realWeightXIndex == wb->weights_sizes[0] - 1 &&
                                                realWeightYIndex == wb->weights_sizes[1] - 1)
                                            {
                                                vx_uint32 offsetValue;

                                                if (hasNNPerFilterPostMultiply)
                                                {
                                                    /*add post multiply to each filter*/

                                                    vx_uint32 postMulSize = 8;
                                                    *kernelBufferInt8Ptr = postMul[filterIndex];
                                                    kernelBufferInt8Ptr++;
                                                    reoder_stream_count_per_core[coreIndex] += (postMulSize / weightBitSize);
                                                    for (idx = 0; idx < (postMulSize / weightBitSize); idx++)
                                                    {
                                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                                    }
                                                }
                                                if (hasNNPerFilterPostShift)
                                                {
                                                    /*add post multiply to each filter*/

                                                    vx_uint32 postShiftSize = 8;
                                                    *kernelBufferInt8Ptr = postShift[filterIndex];
                                                    kernelBufferInt8Ptr++;
                                                    reoder_stream_count_per_core[coreIndex] += (postShiftSize / weightBitSize);
                                                    for (idx = 0; idx < (postShiftSize / weightBitSize); idx++)
                                                    {
                                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                                    }
                                                }

                                                if (z_offset > 0)
                                                    offsetValue = (vx_uint32)z_offset * filterIndex;
                                                else if (output_size > 0)
                                                    offsetValue = output_final_x * output_final_y * output_size * filterIndex;
                                                else
                                                    offsetValue = output_final_x * output_final_y * weightSize * filterIndex;

                                                if (hasBiasAtEnd)
                                                {
                                                    *((vx_uint32 *)kernelBufferInt8Ptr) = biasData;
                                                    kernelBufferInt8Ptr += 4;
                                                    reoder_stream_count_per_core[coreIndex] += (biasBitSize / weightBitSize);
                                                    for (idx = 0; idx < (biasBitSize / weightBitSize); idx++)
                                                    {
                                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                                    }
                                                }

                                                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                                {
                                                    *((vx_uint32 *)kernelBufferInt8Ptr) = offsetValue;
                                                    kernelBufferInt8Ptr += 4;
                                                    reoder_stream_count_per_core[coreIndex] += (NN_Z_POSITION_OFFSET_BITS_VIP_V7 / weightBitSize);
                                                    for (idx = 0; idx < (NN_Z_POSITION_OFFSET_BITS_VIP_V7 / weightBitSize); idx++)
                                                    {
                                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                                    }
                                                }

                                            }
                                        }
                                    }
                                    blockCount++;
                                    if (nonZeroInBlockCount)
                                        nonZeroCount++;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                for (sliceIndex = 0; sliceIndex < slice_count; sliceIndex++)
                {
                    /* add slices of every filter*/
                    for (kid = 0; kid < actualFilterCount; kid++)
                    {
                        filterIndex = filterGroup[kid];
                        /* add one slice data every filter */
                        kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * sliceIndex;
                        if (sliceIndex == 0)
                        {
                            if (bias_base_ptr)
                            {
                                if (bias_format == VX_TYPE_INT64)
                                    biasData64 = *(((vx_int64 *)bias_base_ptr) + filterIndex);
                                else
                                    biasData = *(bias_base_ptr + filterIndex);
                            }
                            else
                                biasData = 0;

                            /* NewBias = sum((Coef[i] - CoefZP) * - InZP) + Bias*/
                            if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT)
                                && inputZP != 0
                                && weight_format == VX_TYPE_UINT8)
                            {
                                biasData -= weightsMinusZP[filterIndex].sum;
                            }
                        }

                        for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex++)
                        {
                            vx_uint32 nonZeroCountInDP3 = 0; /*Check if there's non-zero point in one 3x1 block*/
                            for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex++)
                            {
                                vx_uint32 weight = 0;

                                if (weight_format == VX_TYPE_INT8)
                                    weight = *((vx_int8 *)kernelDataPtr);
                                else if (weight_format == VX_TYPE_UINT8)
                                    weight = *((vx_uint8 *)kernelDataPtr);
                                else
                                    weight = *((vx_uint16 *)kernelDataPtr);
                                kernelDataPtr = kernelDataPtr + weightSize;

                                {
                                    if (weight != skipValue)
                                        nonZeroCountInDP3++;

                                    /*V7, DP3, one block is 3x1*/
                                    if ((weightXIndex + 1) % 3 == 0 || weightXIndex == wb->weights_sizes[0] - 1)
                                    {
                                        blockCount++;
                                        if (nonZeroCountInDP3)
                                            nonZeroCount++;
                                        nonZeroCountInDP3 = 0;
                                    }
                                }

                                if (!hasBiasAtEnd &&
                                    weightXIndex == 0 &&
                                    weightYIndex == 0 &&
                                    sliceIndex == 0)
                                {
                                    if (weight_format == VX_TYPE_INT16)
                                    {
                                        vx_int64 bias64 = 0;
                                        vx_int32 bias32;
                                        vx_int16 bias16;

                                        if (bias_format == VX_TYPE_INT64)
                                            bias64 = biasData64;
                                        else
                                        {
                                            if ((vx_int32)biasData < 0)
                                            {
                                                bias64 = ~0;
                                                bias64 = (bias64 >> 32) << 32;
                                                bias64 = bias64 | (vx_int64)biasData;
                                            }
                                            else
                                                bias64 = (vx_int64)biasData;
                                        }
                                        bias32 = (vx_int32)bias64;
                                        *((vx_uint32 *)kernelBufferInt16Ptr) = bias32;
                                        kernelBufferInt16Ptr += 2;
                                        reoder_stream_count_per_core[coreIndex] += (32 / weightBitSize);
                                        bias16 = (vx_int16)(bias64 >> 32);
                                        *kernelBufferInt16Ptr = bias16;
                                        kernelBufferInt16Ptr++;
                                        reoder_stream_count_per_core[coreIndex] += (16 / weightBitSize);
                                    }
                                    else if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
                                    {
                                        *((vx_uint32 *)kernelBufferInt8Ptr) = biasData;
                                        kernelBufferInt8Ptr += 4;
                                        reoder_stream_count_per_core[coreIndex] += (biasBitSize / weightBitSize);
                                    }
                                    else
                                    {
                                        *((vx_uint32 *)kernelBufferInt16Ptr) = biasData;
                                        kernelBufferInt16Ptr += 2;
                                        reoder_stream_count_per_core[coreIndex] += (biasBitSize / weightBitSize);
                                    }

                                    for (idx = 0; idx < (biasBitSize / weightBitSize); idx++)
                                    {
                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                    }
                                }

                                if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
                                {
                                    *kernelBufferInt8Ptr = (vx_uint8)weight;
                                    kernelBufferInt8Ptr++;
                                }
                                else
                                {
                                    if (weight_format == VX_TYPE_FLOAT16)
                                        weight = (weight & 0x7fff) * 2 + weight/(1<<15);

                                    *kernelBufferInt16Ptr = (vx_uint16)weight;
                                    kernelBufferInt16Ptr++;
                                }
                                reoder_stream_count_per_core[coreIndex] += 1;

                                if ((kid == actualFilterCount - 1) && sliceIndex == 0 &&
                                    weightXIndex == 0 && weightYIndex == 0)
                                    zrl_limit_index[limitZRLIndex++] = elementIndex;

                                elementIndex++;
                            }
                        }

                        /* add offet behind the last point of the last slice of each filter */
                        if (sliceIndex == slice_count - 1)
                        {
                            vx_uint32 offsetValue;

                            if (hasNNPerFilterPostMultiply && hasNNPerFilterPostShift)
                            {
                                if (weight_format == VX_TYPE_INT16 || weight_format == VX_TYPE_FLOAT16)
                                {
                                    vx_uint32 postMulSize = 16;
                                    vx_uint32 postShiftSize = 16;
                                    /*add post multiply to each filter*/
                                    *kernelBufferInt16Ptr = (vx_uint16)postMul[filterIndex];
                                    kernelBufferInt16Ptr++;
                                    reoder_stream_count_per_core[coreIndex] += (postMulSize / weightBitSize);
                                    /*add post multiply to each filter*/
                                    *kernelBufferInt16Ptr = (vx_uint16)postShift[filterIndex];
                                    kernelBufferInt16Ptr++;
                                    reoder_stream_count_per_core[coreIndex] += (postShiftSize / weightBitSize);
                                }
                                else
                                {
                                    vx_uint32 postMulSize = 8;
                                    vx_uint32 postShiftSize = 8;
                                    /*add post multiply to each filter*/
                                    *kernelBufferInt8Ptr = postMul[filterIndex];
                                    kernelBufferInt8Ptr++;
                                    reoder_stream_count_per_core[coreIndex] += (postMulSize / weightBitSize);
                                    /*add post multiply to each filter*/
                                    *kernelBufferInt8Ptr = postShift[filterIndex];
                                    kernelBufferInt8Ptr++;
                                    reoder_stream_count_per_core[coreIndex] += (postShiftSize / weightBitSize);
                                }

                                for (idx = 0; idx < 2; idx++)
                                {
                                    non_coef_index[nonCoefIndex++] = elementIndex++;
                                }
                            }

                            if (z_offset > 0)
                                offsetValue = (vx_uint32)z_offset * filterIndex;
                            else if (output_size > 0)
                                offsetValue = output_final_x * output_final_y * output_size * filterIndex;
                            else
                                offsetValue = output_final_x * output_final_y * weightSize * filterIndex;

                            if (hasBiasAtEnd)
                            {
                                if (weight_format == VX_TYPE_INT16)
                                {
                                    vx_int64 bias64 = 0;
                                    vx_int32 bias32;
                                    vx_int16 bias16;

                                    if ((vx_int32)biasData < 0)
                                    {
                                        bias64 = ~0;
                                        bias64 = (bias64 >> 32) << 32;
                                        bias64 = bias64 | (vx_int64)biasData;
                                    }
                                    else
                                        bias64 = (vx_int64)biasData;

                                    bias32 = (vx_int32)bias64;
                                    *((vx_uint32 *)kernelBufferInt16Ptr) = bias32;
                                    kernelBufferInt16Ptr += 2;
                                    reoder_stream_count_per_core[coreIndex] += (32 / weightBitSize);
                                    bias16 = (vx_int16)(bias64 >> 32);
                                    *kernelBufferInt16Ptr = bias16;
                                    kernelBufferInt16Ptr++;
                                    reoder_stream_count_per_core[coreIndex] += (16 / weightBitSize);
                                }
                                else if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
                                {
                                    *((vx_uint32 *)kernelBufferInt8Ptr) = biasData;
                                    kernelBufferInt8Ptr += 4;
                                    reoder_stream_count_per_core[coreIndex] += (biasBitSize / weightBitSize);
                                }
                                else
                                {
                                    *((vx_uint32 *)kernelBufferInt16Ptr) = biasData;
                                    kernelBufferInt16Ptr += 2;
                                    reoder_stream_count_per_core[coreIndex] += (biasBitSize / weightBitSize);
                                }

                                for (idx = 0; idx < (biasBitSize / weightBitSize); idx++)
                                {
                                    non_coef_index[nonCoefIndex++] = elementIndex++;
                                }
                            }

                            if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                            {
                                if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
                                {
                                    *((vx_uint32 *)kernelBufferInt8Ptr) = offsetValue;
                                    kernelBufferInt8Ptr += 4;
                                }
                                else
                                {
                                    *((vx_uint32 *)kernelBufferInt16Ptr) = offsetValue;
                                    kernelBufferInt16Ptr += 2;
                                }
                                reoder_stream_count_per_core[coreIndex] += (NN_Z_POSITION_OFFSET_BITS_VIP_V7 / weightBitSize);
                                for (idx = 0; idx < (NN_Z_POSITION_OFFSET_BITS_VIP_V7 / weightBitSize); idx++)
                                {
                                    non_coef_index[nonCoefIndex++] = elementIndex++;
                                }
                            }
                        }
                    }
                }
            }
            if (calc_perf && (blockCount != 0))
                nonZeroRatioPerCorePerVZG[nonZrlIdx] = (vx_float64)nonZeroCount / (vx_float64)blockCount;

            usedKid += actualFilterCount;
            if (filterGroup)
            {
                vxFree(filterGroup);
                filterGroup = VX_NULL;
            }
        }
    }

    if (calc_perf)
    {
        for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
        {
            for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
            {
                vx_uint32 nonZrlIdx = groupIndex + coreIndex * groupCount;
                if (wb->max_per_core_per_vzgroup_nzr[groupIndex] < nonZeroRatioPerCorePerVZG[nonZrlIdx])
                    wb->max_per_core_per_vzgroup_nzr[groupIndex] = nonZeroRatioPerCorePerVZG[nonZrlIdx];
            }
        }
    }
#if DEBUG_COMP_ENHANCE_ENC
    {
        unsigned char * pChar = (unsigned char*)reorder_stream;
        unsigned char data;
        vx_uint32 i;
        vx_uint32 totalByteCount = weightCount * slice_count * z_count * weightSize + z_count * 4 + z_count * (biasBitSize / 8) + nnCoreCount * 2;
        FILE * pfile1 = NULL;
        pfile1 = fopen("reordered_kernel.txt", "wb");

        if(pfile1 != NULL)
        {
            for(i=0; i!=totalByteCount; i++)
            {
                data = *pChar;
                fprintf(pfile1, "coef: 0x%x\n", data);
                pChar++;
            }

            fclose(pfile1);
        }

    }
#endif

exit:
    if (weightsMinusZP)
        vxFree(weightsMinusZP);
    if (nonZeroRatioPerCorePerVZG)
        vxFree(nonZeroRatioPerCorePerVZG);
    if (nonZeroWeights)
        vxFree(nonZeroWeights);
#if PRINT_PER_CORE_SIZE
    if(nonZeroCoefPerCore)
        vxFree(nonZeroCoefPerCore);
#endif
    return;
}

void analysisKernelStreamForHuffman(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint32 skipValue,
    vx_uint32 nnCoreCount,
    vx_uint8* reorderStream,
    vx_uint32 reorderStreamSize,
    vx_uint32* reorderStreamPerCoreCount,
    vx_uint32* invSizeOrder,
    vx_uint32* nonCoefIndex,
    vx_uint32 nonCoefCount,
    vx_uint32* limitZRLIndex,
    vx_uint32 limitZRLCount,
    vx_uint32 index
    )
{
    vx_uint8  * pBS_U08      = (unsigned char * )reorderStream;
    vx_uint16 * pBS_U16      = (unsigned short *)reorderStream;
    vx_enum weightFormat = wb->wb_base->weights_data_format;
    vx_uint8 bit16Flag   = (weightFormat == VX_TYPE_INT8 || weightFormat == VX_TYPE_UINT8) ? 0 : 1;
    vx_uint32 kernelDataCount  = reorderStreamSize >> bit16Flag;
    vx_uint32 kernelDataBytes  = reorderStreamSize;
    vx_uint32 dataCountToPross = kernelDataCount;

    vx_int32 i, j, k, bias;
    vx_int32 x=0;
    vx_int32 runZeros[256], run; /*The histogram of how many runs*/
    vx_int32 histo[256], sizeHisto[9] = {0,0,0,0,0,0,0,0,0};
    vx_int32 sizeOrder[]  = {0,1,2,3,4,5,6,7,8,};
    vx_int32 codingType   = HUFFMAN_CODING_TYPE_RUN_LEN; /* defalt RZL*/
    vx_int32 prevEncode   = 0, prev = 0, prevHisto[256], prevRunZeros[256], prevRun; /*previous pixel*/
    vx_float32 p, entropy = 0.f, prevEntropy = 0.f;
    vx_int32 coreId = 0, accumSize = 0x0;

    vx_uint32 totalCountIdx = 0;
    vx_uint32 nonCoefCountIdx = 0;
    vx_uint32 limitZRLCountIdx = 0;
    vx_bool isCoef;
    vx_bool isLimitZRL;
    /*per PRD, V8 will disable pre & bias mode*/
    vx_bool isV8 = (vx_bool)(context->nnConfig.derivedFeature.nnXYDPX == 0 || context->nnConfig.derivedFeature.nnXYDPY == 0);

    /* this variable indicate DataAccumCount when Start of core*/
    vx_int32 *accumCountEndOfCore = (vx_int32 *)vxAllocateAndZeroMemory(nnCoreCount * sizeof(vx_int32));

    if (accumCountEndOfCore == VX_NULL)
    {
        vxError("analysisKernelStreamForHuffman: OUT OF MEMORY\n");
        goto exit;
    }

    for (i = 0; i < 256; i++)
    {
        runZeros[i] = histo[i] = 0;
        prevHisto[i] = prevRunZeros[i] = 0;
    }


    for (coreId = 0; coreId != (vx_int32)nnCoreCount; coreId++)
    {
        accumSize += reorderStreamPerCoreCount[coreId];
        accumCountEndOfCore[coreId] = accumSize - 1;
    }

    prevRun = run = 0;
    coreId = 0;
    while(dataCountToPross--)
    {
        vx_uint32 endOfCore = (dataCountToPross + accumCountEndOfCore[coreId] == kernelDataCount - 1) ? 1 : 0;
        if(endOfCore)
        {
            coreId++;
        }

        if (!bit16Flag)
        {
            i = *(pBS_U08++);
        }
        else
        {
            i = *(pBS_U16++);
            if (weightFormat == VX_TYPE_FLOAT16)
            {
                i = (i & 0x7fff) * 2 + i/(1<<15); /*(i&0x8000)/128 + (i&0x7f00)*2 + (i&0xff);*/
            }
        }

        if (nonCoefIndex != VX_NULL &&
            totalCountIdx == nonCoefIndex[nonCoefCountIdx] &&
            nonCoefCountIdx < nonCoefCount)
        {
            isCoef = vx_false_e;
            nonCoefCountIdx++;
        }
        else
            isCoef = vx_true_e;

        if (limitZRLIndex != VX_NULL &&
            totalCountIdx == limitZRLIndex[limitZRLCountIdx] &&
            limitZRLCountIdx < limitZRLCount)
        {
            isLimitZRL = vx_true_e;
            limitZRLCountIdx++;
        }
        else
            isLimitZRL = vx_false_e;

        totalCountIdx++;

        if (i == 0 && isCoef && !isLimitZRL)
            run++;
        if ((run == sizeof(runZeros)/sizeof(int)) || i || endOfCore || !isCoef || isLimitZRL)
        {
            if (run)
            {
                runZeros[run - 1]++;
                run = 0;
            }
        }
        histo[i>>(bit16Flag*8)]++;
        /*Get statistic of prediction from previous pixel*/
        if(bit16Flag == 0 && !isV8)
        {
            j = i;
            i = (i-prev)&0xff;
            prev = j;
            if (i == 0 && isCoef && !isLimitZRL) prevRun++;
            if (prevRun == sizeof(prevRunZeros)/sizeof(int) || i || endOfCore || !isCoef || isLimitZRL)
            {
                if (prevRun){
                    prevRunZeros[prevRun - 1]++;
                    prevRun = 0;
                }
            }
            prevHisto[i]++;
        }

        /* prev encode should be truncated between cores, when analysis, reset prev.*/
        if (endOfCore)
        {
            prev = 0x0;
        }
    }
    if (run)
    { /*last run*/
        runZeros[run - 1]++;
        run = 0;
    }
    if (prevRun)
    { /*last run*/
        prevRunZeros[prevRun - 1]++;
        prevRun = 0;
    }

    j = histo[0];
    bias = 0;

    for (i = 1; i < 256; i++)
    {
        if (histo[i] > j)
        {
            j = histo[i];
            bias = i;
        }
    }
    if (j*3 < histo[0]*4 && bit16Flag == 0)
        bias = 0;

    /*16 bits, we only get the high bit bias, we need to get the low bit bias*/
    if (bit16Flag)
    {
        pBS_U16 = (unsigned short *)reorderStream;
        dataCountToPross = kernelDataCount;
        while(dataCountToPross--)
        {
            i = *(pBS_U16);
            pBS_U16++;
            if (weightFormat == VX_TYPE_FLOAT16)
                i = (i&0x7fff)*2 + i/(1<<15);

            if ((i>>8) == bias)
                prevHisto[i&0xff]++;
        }

        j = prevHisto[0];
        k = 0;

        for (i = 1; i < 256; i++)
        {
            if (prevHisto[i] > j)
            {
                j = prevHisto[i];
                k = i;
            }
        }
        bias = bias*256+k;
    }

    x = 0;
    for (i = 0; i<256; i++)
    {
        p = histo[i] / (vx_float32)(kernelDataCount);
        if (histo[i])
            entropy += -p*(vx_float32)log(p)/(vx_float32)log(2.0f);

        p =  prevHisto[i] / (vx_float32)reorderStreamSize;
        if (prevHisto[i])
            prevEntropy += - p*(vx_float32)log(p)/(vx_float32)log(2.0f);
    }

    if (bit16Flag == 0)
    {
        if ((prevEntropy < entropy || 2*prevHisto[0] + prevHisto[255] + prevHisto[1] > 3*histo[bias] + histo[(bias+1)&0xff] + histo[(bias-1)&0xff]) && !isV8)
        {
            /*per PRD, V8 will disable pre & bias mode*/
            entropy = prevEntropy;
            prevEncode = 1; /*Using prevEncode, prediction from previous pixel*/
            for (i = 0; i < 256; i++)
            {
                histo[i] = prevHisto[i];
                runZeros[i] = prevRunZeros[i];
            }
            bias = 0;
        }
    }
    prevEntropy = entropy;


    if (isV8)
        bias = 0;

    if (bias != 0)
    {
        totalCountIdx = 0;
        nonCoefCountIdx = 0;
        limitZRLCountIdx = 0;
        for (i = 0; i < 256; i++)
        {
            histo[i] = runZeros[i] = 0;
        }
        dataCountToPross = kernelDataCount;
        pBS_U08 = (vx_uint8 * )reorderStream;
        pBS_U16 = (vx_uint16 *)reorderStream;
        while(dataCountToPross--)
        {
            if (bit16Flag == 0)
            {
                i = *(pBS_U08++);
            }
            else
            {
                i = *(pBS_U16++);
                if (weightFormat == VX_TYPE_FLOAT16)
                {
                    i = (i&0x7fff)*2 + i/(1<<15);
                }
            }

            i = (i - bias);

            if (nonCoefIndex != VX_NULL &&
                totalCountIdx == nonCoefIndex[nonCoefCountIdx] &&
                nonCoefCountIdx < nonCoefCount)
            {
                isCoef = vx_false_e;
                nonCoefCountIdx++;
            }
            else
                isCoef = vx_true_e;

            if (limitZRLIndex != VX_NULL &&
                totalCountIdx == limitZRLIndex[limitZRLCountIdx] &&
                limitZRLCountIdx < limitZRLCount)
            {
                isLimitZRL = vx_true_e;
                limitZRLCountIdx++;
            }
            else
                isLimitZRL = vx_false_e;

            totalCountIdx++;

            if (i == 0 && isCoef && !isLimitZRL) run++;
            if (run == sizeof(runZeros) / sizeof(vx_int32) || i || !isCoef || isLimitZRL)
            {
                if(run)
                {
                    runZeros[run - 1]++;
                    run = 0;
                }
            }

            histo[(i>>(bit16Flag*8))&0xff]++;
        }
        if (run)
        { /*last run*/
            runZeros[run - 1]++;
            run = 0;
        }
    }

    /*Get all the runZeros present. supposed to be histo[0], but, for 16-bit, we only count the runs for double zeros.*/
    for (i = 0; i < 256; i++)
    {
        run += runZeros[i] * (i+1);
    }

    if (bit16Flag == 0 && ((unsigned int)histo[0] <= reorderStreamSize / 8 || histo[0] < (histo[1] + histo[255]) * 3/4 || entropy >= 5.25f) && entropy > 3.5f )
    {
         codingType = HUFFMAN_CODING_TYPE_NON_RUN_LEN;
    }
    if (bit16Flag && run < (histo[1] + histo[255]) / 4)
    {
         codingType = HUFFMAN_CODING_TYPE_NON_RUN_LEN;
    }
    if (bit16Flag && entropy > 5.25f)
        codingType = 0; /*Force to test run-length*/


    if (codingType == HUFFMAN_CODING_TYPE_RUN_LEN)
    {
        FindBest2PlusRunSets(runZeros, sizeof(runZeros)/sizeof(int), (reorderStreamSize >> bit16Flag) - run);
    }

    if (codingType == HUFFMAN_CODING_TYPE_NON_RUN_LEN)
    {
        j = 0;
        for (i = 0; i < 8;i++){
            for (; j<(1<<i); j++){
                sizeHisto[i] += histo[j] + histo[j^0xff];
            }
        }

        for (i = 0; i < 7; i++){
            vx_int32 maxFreq = 0, maxPos = i;
            for (j = i; j<8; j++){
                if (maxFreq<sizeHisto[j])
                {
                    maxFreq = sizeHisto[j];
                    maxPos = j;
                }
            }
            if (maxPos > i)
            {
                j = sizeHisto[maxPos];
                sizeHisto[maxPos] = sizeHisto[i];
                sizeHisto[i] = j;

                j = sizeOrder[maxPos];
                sizeOrder[maxPos] = sizeOrder[i];
                sizeOrder[i] = j;
            }
        }
        for (i = 0; i < 8; i++)
        {
            invSizeOrder[sizeOrder[i]] = i;
        }
        entropy = 0.f;
        j = 0;
        for (i = 0; i < 8; i++)
        {
            p = sizeHisto[i]/(vx_float32)kernelDataBytes;
            if(p>0.f)
                entropy += -p*(vx_float32)log(p)/(vx_float32)log(2.0f);
            j += sizeHisto[i]*sizeCodeLen[i];
        }
    }

    if (codingType == HUFFMAN_CODING_TYPE_RUN_LEN)
    {
        j = 1;
        if (bit16Flag)
           histo[0] -= run; /*The zeros not runs*/

        run = 0;
        for (i = 1; i < 8; i++)
        {
            for (; j < (1<<i); j++)
            {
                sizeHisto[i] += histo[j - bit16Flag] + histo[(-j)&0xff];
            }
        }
        sizeHisto[7] += histo[0x80];/*128 haven't put in yet.*/

        /*Get the frequency of run-length.*/
        for (i = 0; i < numBestRuns; i++)
        {
            if (freq[i] < 0) /*The set-1*/
                sizeHisto[0] += abs(freq[i]);
            else /*Set-2*/
                sizeHisto[8] += abs(freq[i]);
        }

        k = sizeHisto[1];
        for (i = 2; i < 7; i++)
        {
            if (sizeHisto[i] < k)
                k = sizeHisto[i];
        }

        /*avoid the run-length be the last*/
        if (sizeHisto[0] <= k)
            sizeHisto[0] = k+2;
        if (sizeHisto[8] <= k)
            sizeHisto[8] = k+2;

        if (sizeHisto[7] <= k) /*The 7 is the escape code, shouldn't be the last (the last will be mergy to 7).*/
            sizeHisto[7] = k+1;


         /*sorting the sizeHisto*/
         for (i = 0; i<8; i++)
         {
            vx_int32 maxFreq = 0, maxPos = i;
            for (j = i; j < 9; j++)
            {
                if(maxFreq < sizeHisto[j])
                {
                    maxFreq = sizeHisto[j];
                    maxPos = j;
                }
            }
            if (maxPos > i)
            {
                j = sizeHisto[maxPos];
                sizeHisto[maxPos] = sizeHisto[i];
                sizeHisto[i] = j;

                j = sizeOrder[maxPos];
                sizeOrder[maxPos] = sizeOrder[i];
                sizeOrder[i] = j;
            }
        }

        if(sizeOrder[7]%8 == 0 && sizeOrder[6]%8 > 0)
        { /*The run-length cannot be the escape code. swap to size 6*/
            j = sizeOrder[6];
            sizeOrder[6] = sizeOrder[7];
            sizeOrder[7] = j;

            j = sizeHisto[6];
            sizeHisto[6] = sizeHisto[7];
            sizeHisto[7] = j;

        }
        else if(sizeOrder[7]%8 == 0 && sizeOrder[6]%8 == 0)
        { /*The run-length cannot be the escape code. swap to size 6*/
            /*5,7,6*/
            j = sizeOrder[6];
            sizeOrder[6] = sizeOrder[7];
            sizeOrder[7] = j;

            j = sizeHisto[6];
            sizeHisto[6] = sizeHisto[7];
            sizeHisto[7] = j;
            /*6,7,5*/
            j = sizeOrder[5];
            sizeOrder[5] = sizeOrder[7];
            sizeOrder[7] = j;

            j = sizeHisto[5];
            sizeHisto[5] = sizeHisto[7];
            sizeHisto[7] = j;
        }

        for(i = 0; i < 9; i++)
            invSizeOrder[sizeOrder[i]] = i;

        entropy = 0.f;
        j = 0;
        x = 0;
        for(i = 0; i < 9; i++)
        {
            x += sizeHisto[i];
        }
        for(i = 0; i < 8; i++)
        {
            p = sizeHisto[i] / (vx_float32)x;
            if (p > 0.f)
                entropy += -p*(vx_float32)log(p)/(vx_float32)log(2.0f);
            j += sizeHisto[i]*sizeCodeLen[i];
        }
    }

    /*per PRD, V8 will disable pre & bias mode*/
    if (isV8)
    {
        gcmASSERT(prevEncode == 0 && bias == 0);
    }
    /* update compression header*/
    wb->huffmanConfig[index].preEncode = (vx_uint8)prevEncode;
    wb->huffmanConfig[index].bit16Flag  = bit16Flag;
    wb->huffmanConfig[index].fp16Flag   = (weightFormat == VX_TYPE_FLOAT16) ? 1 : 0;
    wb->huffmanConfig[index].reserved   = 0x0; /* must be zero*/
    wb->huffmanConfig[index].version    = 0x1; /* 4'b0001 for version 1.0*/
    wb->huffmanConfig[index].runLenTableSize = (codingType == HUFFMAN_CODING_TYPE_NON_RUN_LEN) ? 0: (vx_uint8)numBestRuns;

    /* runlenTable 8 * 18 = 144*/
    for(i = 0; i<SET_1_SIZE; i++)
    {
        wb->huffmanConfig[index].runLenTable[i] = (vx_uint8)best2Runs[i];
    }
    for(; i< numBestRuns; i++)
    {
        wb->huffmanConfig[index].runLenTable[i] = (vx_uint8)outBest2Run[i - SET_1_SIZE];
    }
    for(i = 0; i<8; i+=2)
    {
        wb->huffmanConfig[index].mapToHuffman[i/2] = (vx_uint8)(sizeOrder[i+1]*16 + sizeOrder[i]);
    }
    wb->huffmanConfig[index].avgBias = (vx_uint8)bias;
    wb->huffmanConfig[index].reserved = 0x0; /* reserved 16, must be zero*/

exit:
    if (accumCountEndOfCore != VX_NULL)
        vxFree(accumCountEndOfCore);
}

vx_uint32 calcKernelStreamSizeHuffman(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_int32 input_zp,
    vx_int32 coef_zp,
    vx_uint32 skip_value,
    vx_uint32 output_final_x,
    vx_uint32 output_final_y,
    vx_int32 z_offset,
    vx_uint32 output_size,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_uint32 index
    )
{
    vx_uint32 kernelSize = 0;
    vx_uint32 kernelBitSize = 0;

    vx_uint32 nnCoreCount = weight_format == VX_TYPE_INT16 ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : weight_format == VX_TYPE_FLOAT16 ?
                      context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 invSizeOrder[MAX_SIZE_HISTO_COUNT] = {0};
    vx_int32 coef = 0, tmp = 0, prev = 0;

    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 usedCoreCount      = (filterTotalCount > nnCoreCount) ? nnCoreCount : filterTotalCount;
    vx_uint32 filterCount        = filters_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;
    vx_uint32 limitZRLCount      = groupCount * usedCoreCount;
    vx_uint32 *limitZRLIndex     = VX_NULL;

    vx_uint32 weightCount        = weight_x * weight_y;
    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize((vx_type_e)weight_format);
    vx_uint32 weightBitSize = weightSize * 8;
    vx_uint32 biasBitSize      = 0;

    vx_uint32 kernelStreamSizePerCore = 0;
    vx_uint8_ptr kernelDataPtr   = VX_NULL;

    vx_uint32 coreIndex;
    vx_uint32 totalCountIdx = 0;
    vx_uint32 nonCoefCountIdx = 0;

    vx_uint32 bit16Flag = (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8) ? 0 : 1;

    vx_uint32 reorderStreamSize = 0;
    vx_uint32 reorderStreamAllCount = 0;
    vx_uint32 reorderStreamCheckCount = 0;
    vx_uint32* reorderStreamPerCoreCount = VX_NULL;
    vx_uint32 nonCoefCount = 0;
    vx_uint32 limitZRLCountIdx = 0;
    vx_uint32* nonCoefIndex = VX_NULL;

    vx_uint8* reorderStream = VX_NULL;
    vx_uint32 runSet2Bits = 1;
    CodeSymbol codeSymbol[THROUGHPUT * 3];

    if (weightBitSize == 0)
    {
        vxError("%s: weightBitSize should not be zero.", __FUNCTION__);
        goto exit;
    }

    if (weight_format == VX_TYPE_INT16)
        biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7_INT16;
    else
        biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7;

    reorderStreamAllCount = (filterTotalCount * (biasBitSize / weightBitSize))
        + (weightCount * slice_count * filterTotalCount)
        + (filterTotalCount * (NN_Z_POSITION_OFFSET_BITS_VIP_V7 / weightBitSize))
        + (nnCoreCount * (16 / weightBitSize));/*filterPerCore need 16 bit * nnCoreCount */

    reorderStreamSize = reorderStreamAllCount * weightSize;

    reorderStream = (vx_uint8 *)vxAllocateAndZeroMemory(reorderStreamSize);

    if (!reorderStream)
    {
        vxError("fillinKernelBufferHuffman: OUT OF MEMORY");
        goto exit;
    }

    reorderStreamPerCoreCount = (vx_uint32 *)vxAllocateAndZeroMemory(nnCoreCount * sizeof(vx_uint32));
    if (!reorderStreamPerCoreCount)
    {
        vxError("fillinKernelBufferHuffman: OUT OF MEMORY");
        goto exit;
    }

    nonCoefCount = (filterTotalCount * (biasBitSize / weightBitSize))
        + (filterTotalCount * (NN_Z_POSITION_OFFSET_BITS_VIP_V7 / weightBitSize))
        + (nnCoreCount * (16 / weightBitSize)); /*Those non coef count contain bias, Z_OFF_SET & kernels_per_core*/
    nonCoefIndex = (vx_uint32 *)vxAllocateAndZeroMemory(nonCoefCount * sizeof(vx_uint32));
    if (!nonCoefIndex)
    {
        vxError("fillinKernelBufferHuffman: OUT OF MEMORY");
        goto exit;
    }

    limitZRLIndex = (vx_uint32 *)vxAllocateAndZeroMemory(limitZRLCount * sizeof(vx_uint32));
    if (!limitZRLIndex)
    {
        vxError("fillinKernelBufferHuffman: OUT OF MEMORY");
        goto exit;
    }

    if (context->options.enableNonZeroBalance)
    {
        reorderKernelBufferV7HuffmanBalance(context, wb, slice_count, z_count, filters_per_core, weight_format, bias_format, z_offset,
        output_size, skip_value, output_final_x, output_final_y, reorderStream, weight_base_ptr, bias_base_ptr, VX_NULL, VX_NULL, reorderStreamPerCoreCount, nonCoefIndex, limitZRLIndex, vx_false_e);
    }
    else
    {
        reorderWeightBiasBufferForHuffman(context, wb, slice_count, z_count, filters_per_core, weight_format, bias_format, z_offset,
            output_size, skip_value, output_final_x, output_final_y, reorderStream, weight_base_ptr, bias_base_ptr, VX_NULL, VX_NULL, reorderStreamPerCoreCount, nonCoefIndex, limitZRLIndex, vx_false_e);
    }

    analysisKernelStreamForHuffman(context, wb, 0, nnCoreCount, reorderStream, reorderStreamSize, reorderStreamPerCoreCount, invSizeOrder, nonCoefIndex, nonCoefCount, limitZRLIndex, limitZRLCount, index);

    kernelBitSize += 16;
    kernelBitSize += 8 * MAX_RUNLEN_SIZE;
    kernelBitSize += 8 * LOG_RUN_SIZE1;
    kernelBitSize += 32;
    kernelBitSize += 32 * nnCoreCount;

    kernelSize = (kernelBitSize % 8 == 0) ? kernelBitSize / 8 : kernelBitSize / 8 + 1;
    /*align to 64 byte*/
    kernelSize = (kernelSize + 63) & 0xFFFFFFC0;
    kernelBitSize = 0;

    while(((numBestRuns-SET_1_SIZE)>>runSet2Bits) > 1)
    {
        runSet2Bits++;
    }

    kernelDataPtr = reorderStream;
    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_int32 run = 0x0;
        vx_uint32 dataSizeOfCore = reorderStreamPerCoreCount[coreIndex];
        vx_uint32 dataRemainingOfCore = dataSizeOfCore;
        vx_int32 size = 0;
        //vx_int32 code = 0;
        vx_int32 coef16 = 0;
        vx_int32 k = 0, j = 0, m = 0;
        vx_bool isCoef;
        vx_bool isLimitZRL;
        /* update position of each core's beginning of compressed kernel*/
        reorderStreamCheckCount += reorderStreamPerCoreCount[coreIndex];

        memset(codeSymbol, 0, sizeof(CodeSymbol));

        while(dataRemainingOfCore --)
        {
            if (weight_format == VX_TYPE_INT8)
                coef = *((vx_int8 *)kernelDataPtr);
            else if (weight_format == VX_TYPE_UINT8)
                coef = *((vx_uint8 *)kernelDataPtr);
            else if (weight_format == VX_TYPE_INT16 ||
                weight_format == VX_TYPE_FLOAT16)
                coef16 = *((vx_int16 *)kernelDataPtr);
            else
            {
                /*Other format not suppoted now*/
                vxmASSERT(0);
            }
            kernelDataPtr += weightSize;

            if (totalCountIdx == nonCoefIndex[nonCoefCountIdx] &&
                nonCoefCountIdx < nonCoefCount)
            {
                isCoef = vx_false_e;
                nonCoefCountIdx++;
            }
            else
                isCoef = vx_true_e;

            if (limitZRLIndex != VX_NULL &&
                totalCountIdx == limitZRLIndex[limitZRLCountIdx] &&
                limitZRLCountIdx < limitZRLCount)
            {
                isLimitZRL = vx_true_e;
                limitZRLCountIdx++;
            }
            else
                isLimitZRL = vx_false_e;

            totalCountIdx++;

            if (bit16Flag == 0)
            {

                coef = (coef - wb->huffmanConfig[index].avgBias) & 0xFF;
                if (wb->huffmanConfig[index].preEncode)
                {
                    tmp = coef;
                    coef = (coef - prev) & 0xFF;
                    prev = tmp;
                }
            }
            else
            {

                if (wb->huffmanConfig[index].fp16Flag)
                {
                    coef16 = (coef16 & 0x7FFF) * 2 + coef16 / (1<<15);
                }
                coef16 -= wb->huffmanConfig[index].avgBias;
                if (wb->huffmanConfig[index].runLenTableSize == 0x0)
                    coef = (coef16 >> 8) & 0xFF; /*Non RZL*/
                else
                    coef = coef16 & 0xFFFF; /*RZL path*/
            }

            if (wb->huffmanConfig[index].runLenTableSize == 0x0) /* non RZL*/
            {
                tmp = (coef & 0x80)? (coef ^ 0xff): coef;
                for (size = 0; size<7; size++)
                {
                    if (tmp<(1<<size))
                        break;
                }
                k = invSizeOrder[size];
                kernelBitSize += 3;

                if (sizeCodeLen[k] > 3)
                {
                    kernelBitSize += 2;
                }

                if (size == 0) size = 1; /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                if (sizeCodeLen[k]%2 == 0)
                {
                    kernelBitSize += (size - 1);
                }
                else
                {
                    kernelBitSize += size;
                }

                if (wb->huffmanConfig[index].bit16Flag)
                {/*pack the raw data of the lower 8 bits*/
                    kernelBitSize += 8;
                }
            }
            else /* RZL enable*/
            {
                if (coef == 0 && isCoef && !isLimitZRL)
                {
                    run++;
                    if (dataRemainingOfCore != 0)
                    {
                        continue;
                    }
                }
                if (coef || (dataRemainingOfCore == 0) || !isCoef || isLimitZRL) /* if last pixel, force process run zeros*/
                {
                    /* process run zeros*/
                    while(run)
                    {
                        for (j = numBestRuns - 1; j>=0; j--)
                        {
                            if (run >= bestRunsSorted[j] + 1)
                            {
                                k = run / (bestRunsSorted[j] + 1);
                                run %= (bestRunsSorted[j] + 1);
                                break;
                            }
                        }
                        if (j >= 0)
                        {
                            vx_int32 n = bestRunsSorted[j];
                            size = 8;
                            for (m = 0; m<SET_1_SIZE; m++)
                            {
                                if (best2Runs[m] == n)
                                {
                                    size = 0;
                                    break;
                                }
                            }
                            if (size == 8)
                            {
                                for (m = 0; m<numBestRuns - SET_1_SIZE; m++)
                                    if (outBest2Run[m] == n)
                                        break;

                            }
                            j = invSizeOrder[size];
                            while(k--)
                            {/*k of the runs*/
                                kernelBitSize += 3;

                                if (sizeCodeLen[j] > 3)
                                {
                                    kernelBitSize += 2;
                                }
                                if (size == 0 )
                                {/*The m already code*/
                                    kernelBitSize += ((sizeCodeLen[j]%2 == 0) ? 0:1);
                                }
                                else if (sizeCodeLen[j]%2 == 0)
                                {
                                    kernelBitSize += (runSet2Bits - 1);
                                }
                                else
                                {
                                    kernelBitSize += runSet2Bits;
                                }
                            }
                        }
                    }
                    /*process current pixel*/
                    if (coef)
                    {
                        vx_int8 ch;
                        coef>>=(wb->huffmanConfig[index].bit16Flag * 8);
                        if(coef<0x80)
                            coef+=wb->huffmanConfig[index].bit16Flag; /*Since we have to encode zero (run-length only handle 16-bits zero, 8 bit zero should use Size code*/
                        ch = (vx_int8)coef;
                        coef = abs(ch);
                        for (j = 6; j>=0; j--)
                        {
                            if (coef>=(1<<j))
                            {
                                break;
                            }
                        }
                        size = j+1;
                        if (invSizeOrder[size]==8)
                        { /*The minimum freq, we out the raw data*/
                            size = 7;
                        }

                        j = invSizeOrder[size];
                        kernelBitSize += 3;

                        if (sizeCodeLen[j] > 3)
                        {
                            kernelBitSize += 2;
                        }

                        if (size == 7)
                        {
                            size = 8;
                        }
                        /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                        if (size == 0)
                            size = 1;
                        if (sizeCodeLen[j]%2 == 0)
                        {
                            kernelBitSize += (size - 1);
                        }
                        else
                        {
                            kernelBitSize += size;
                        }
                        if (wb->huffmanConfig[index].bit16Flag)
                        {
                            kernelBitSize += 8;
                        }
                    }
                    else
                    {
                        /*those non-coef count like bias, num_of_vz & z_offset couldn't skip by zero-run as HW requres,
                        need also encode those 0 to huffman code,
                        in ZRL path 8-bit, 0 belong to group7, -128~127, residue bit length is 8, residue code = 0*/
                        size = 7;
                        j = invSizeOrder[size];
                        kernelBitSize += 3;

                        if (sizeCodeLen[j] > 3)
                        {
                            kernelBitSize += 2;
                        }

                        if (size == 7)
                        {
                            size = 8;
                        }

                        if (sizeCodeLen[j] % 2 == 0)
                        {
                            kernelBitSize += (size - 1);
                        }
                        else
                        {
                            kernelBitSize += size;
                        }
                        if (wb->huffmanConfig[index].bit16Flag)
                        {
                            kernelBitSize += 8;
                        }
                    }
                }
            }

            if(dataRemainingOfCore == 0)
            {
                /*Hit the end bitsteam of core, add dummy stage process the next*/
                kernelBitSize += 3;
                kernelBitSize += 2;
                kernelBitSize += 7;

                if (wb->huffmanConfig[index].bit16Flag)
                    kernelBitSize += 8;

                for (j = 0; j < 2*THROUGHPUT; j++)
                {
                    kernelBitSize += 3;
                }
                break;
            }
        }

        /* Go back to update kernelStreamSize. */
        kernelStreamSizePerCore = (kernelBitSize % 8 == 0) ? kernelBitSize / 8 : kernelBitSize / 8 + 1;
        /*align to 64 byte*/
        kernelStreamSizePerCore = (kernelStreamSizePerCore + 63) & 0xFFFFFFC0;
        kernelBitSize = 0;
        kernelSize += kernelStreamSizePerCore;
    }

exit:
    if (reorderStream)
        vxFree(reorderStream);
    if (reorderStreamPerCoreCount)
        vxFree(reorderStreamPerCoreCount);
    if (nonCoefIndex)
        vxFree(nonCoefIndex);
    if (limitZRLIndex)
        vxFree(limitZRLIndex);

    return kernelSize;
}

void fillinKernelBufferHuffman(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_int32 input_zp,
    vx_int32 coef_zp,
    vx_uint32 skip_value,
    vx_uint32 output_final_x,
    vx_uint32 output_final_y,
    vx_int32 z_offset,
    vx_uint32 output_size,
    vx_uint8_ptr wb_base_ptr,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_uint32 index
    )
{

    vx_uint32 nnCoreCount = weight_format == VX_TYPE_INT16 ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : weight_format == VX_TYPE_FLOAT16 ?
                      context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 invSizeOrder[MAX_SIZE_HISTO_COUNT] = {0};
    vx_int32 coef = 0, tmp = 0, prev = 0;

    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 usedCoreCount      = (filterTotalCount > nnCoreCount) ? nnCoreCount : filterTotalCount;
    vx_uint32 filterCount        = filters_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;
    vx_uint32 limitZRLCount      = groupCount * usedCoreCount;
    vx_uint32 *limitZRLIndex     = VX_NULL;

    vx_uint32 weightCount        = weight_x * weight_y;
    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize((vx_type_e)weight_format);
    vx_uint32 weightBitSize = weightSize * 8;
    vx_uint32 biasSize         = (vx_uint32)vxDataType_GetSize((vx_type_e)bias_format);
    vx_uint32 biasBitSize      = 0;

    vx_uint32 filterSize         = weightCount * weightSize * slice_count;
    vx_uint32* kernelBufferPtr   = (vx_uint32*)wb_base_ptr;

    vx_uint32* kernelStreamSizePtr = VX_NULL;
    vx_uint32 kernelStreamSize = ((filterSize * filterTotalCount + filterTotalCount * biasSize + filterTotalCount * 3 + 3) + 63) & ~63;
    vx_uint32 alignedOffset      = ((vx_uint32)(gctUINTPTR_T)kernelBufferPtr) & 0x3F;
    vx_uint8_ptr kernelDataPtr   = VX_NULL;
    vx_uint8_ptr kernelStreamBasePtr   = VX_NULL;
    vx_uint32 streamSizeBitOffset = 0;
    vx_uint32 maxKernelStreamSizePerCore = 0;

    vx_uint32 coreIndex;
    vx_uint32 i;
    vx_uint32 totalCountIdx = 0;
    vx_uint32 nonCoefCountIdx = 0;
    vx_uint32 limitZRLCountIdx = 0;

    vx_uint32 bit16Flag = (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8) ? 0 : 1;
    vx_uint32 bitOffset = 0;

    vx_uint32 reorderStreamSize = 0;
    vx_uint32 reorderStreamAllCount = 0;
    vx_uint32 reorderStreamCheckCount = 0;
    vx_uint32* reorderStreamPerCoreCount = VX_NULL;
    vx_uint32 nonCoefCount = 0;
    vx_uint32* nonCoefIndex = VX_NULL;

    vx_uint8* reorderStream = VX_NULL;
    vx_uint32 runSet2Bits = 1;
    CodeSymbol codeSymbol[THROUGHPUT * 3];

    vx_uint32 dummyStage[3] = {0};
    vx_uint32 dummyBitLength[3] = {0};
    vx_bool setDummy = vx_false_e;
    vx_bool hasKernelFullCacheInterleaveFix = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_FULLCACHE_KERNEL_INTERLEAVE_FIX);

    if (weight_format == VX_TYPE_INT16)
        biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7_INT16;
    else
        biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7;

    reorderStreamAllCount = (filterTotalCount * (biasBitSize / weightBitSize))
        + (weightCount * slice_count * filterTotalCount)
        + (filterTotalCount * (NN_Z_POSITION_OFFSET_BITS_VIP_V7 / weightBitSize))
        + (usedCoreCount * (16 / weightBitSize));/*filterPerCore need 16 bit * usedCoreCount */

    reorderStreamSize = reorderStreamAllCount * weightSize;

    reorderStream = (vx_uint8 *)vxAllocateAndZeroMemory(reorderStreamSize);

    if (!reorderStream)
    {
        vxError("fillinKernelBufferHuffman: OUT OF MEMORY");
        goto exit;
    }

    reorderStreamPerCoreCount = (vx_uint32 *)vxAllocateAndZeroMemory(nnCoreCount * sizeof(vx_uint32));
    if (!reorderStreamPerCoreCount)
    {
        vxError("fillinKernelBufferHuffman: OUT OF MEMORY");
        goto exit;
    }

    nonCoefCount = (filterTotalCount * (biasBitSize / weightBitSize))
        + (filterTotalCount * (NN_Z_POSITION_OFFSET_BITS_VIP_V7 / weightBitSize))
        + (nnCoreCount * (16 / weightBitSize)); /*Those non coef count contain bias, Z_OFF_SET & kernels_per_core*/
    nonCoefIndex = (vx_uint32 *)vxAllocateAndZeroMemory(nonCoefCount * sizeof(vx_uint32));
    if (!nonCoefIndex)
    {
        vxError("fillinKernelBufferHuffman: OUT OF MEMORY");
        goto exit;
    }

    limitZRLIndex = (vx_uint32 *)vxAllocateAndZeroMemory(limitZRLCount * sizeof(vx_uint32));
    if (!limitZRLIndex)
    {
        vxError("fillinKernelBufferHuffman: OUT OF MEMORY");
        goto exit;
    }

    if (context->options.enableNonZeroBalance)
    {
        reorderKernelBufferV7HuffmanBalance(context, wb, slice_count, z_count, filters_per_core, weight_format, bias_format, z_offset,
            output_size, skip_value, output_final_x, output_final_y, reorderStream, weight_base_ptr, bias_base_ptr, VX_NULL, VX_NULL, reorderStreamPerCoreCount, nonCoefIndex, limitZRLIndex, vx_false_e);
    }
    else
    {
        reorderWeightBiasBufferForHuffman(context, wb, slice_count, z_count, filters_per_core, weight_format, bias_format, z_offset,
            output_size, skip_value, output_final_x, output_final_y, reorderStream, weight_base_ptr, bias_base_ptr, VX_NULL, VX_NULL, reorderStreamPerCoreCount, nonCoefIndex, limitZRLIndex, vx_false_e);
    }

    analysisKernelStreamForHuffman(context, wb, 0, nnCoreCount, reorderStream, reorderStreamSize, reorderStreamPerCoreCount, invSizeOrder, nonCoefIndex, nonCoefCount, limitZRLIndex, limitZRLCount, index);

    /* Write huffman header*/
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].preEncode, 1);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].bit16Flag, 1);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].fp16Flag, 1);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].reserved, 1);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].version, 4);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].runLenTableSize, 8);

    for(i = 0; i < MAX_RUNLEN_SIZE; i++)
    {
        writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].runLenTable[i], 8);
    }
    for(i = 0; i < LOG_RUN_SIZE1; i++)
    {
        writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].mapToHuffman[i], 8);
    }
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].avgBias, 16);
    writeBits(&kernelBufferPtr, &bitOffset, 0, 16); /*reserved, must zero*/

    kernelStreamSizePtr = kernelBufferPtr;
    streamSizeBitOffset = bitOffset;
    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        writeBits(&kernelBufferPtr, &bitOffset, kernelStreamSize, 32);
    }
    /*align to 64 byte*/
    packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);
    wb->slice_array[index].kernel_stream_full_cache_size = 0;

    for (i = 0; i < THROUGHPUT * 3; i++)
    {
        codeSymbol[i].stageCode[0] = codeSymbol[i].stageCode[1] = codeSymbol[i].stageCode[2] = 0;
        codeSymbol[i].bitLength[0] = codeSymbol[i].bitLength[1] = codeSymbol[i].bitLength[2] = 0;
    }

    while(((numBestRuns-SET_1_SIZE)>>runSet2Bits) > 1)
    {
        runSet2Bits++;
    }

    kernelDataPtr = reorderStream;
    wb->max_per_core_compression_ratio = 0;

    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {

        vx_uint32 x = 0x0;
        vx_int32 pos = 0x0;
        vx_int32 run = 0x0;
        vx_uint32 dataSizeOfCore = reorderStreamPerCoreCount[coreIndex];
        vx_uint32 dataRemainingOfCore = dataSizeOfCore;
        vx_int32 size = 0;
        vx_int32 code = 0;
        vx_int32 coef16 = 0;
        vx_int32 k = 0, j = 0, m = 0;
        vx_bool isCoef;
        vx_bool isLimitZRL;
        vx_float64 compressionRatio = 0;
        /* update position of each core's beginning of compressed kernel*/
        kernelStreamBasePtr = (vx_uint8*)kernelBufferPtr;
        reorderStreamCheckCount += reorderStreamPerCoreCount[coreIndex];

        memset(codeSymbol, 0, sizeof(CodeSymbol));
        memset(dummyStage, 0, (sizeof(vx_uint32) * 3));
        memset(dummyBitLength, 0, (sizeof(vx_uint32) * 3));
        setDummy = vx_false_e;

        while(dataRemainingOfCore --)
        {

            pos = (x) % (3*THROUGHPUT);

            if (weight_format == VX_TYPE_INT8)
                coef = *((vx_int8 *)kernelDataPtr);
            else if (weight_format == VX_TYPE_UINT8)
                coef = *((vx_uint8 *)kernelDataPtr);
            else if (weight_format == VX_TYPE_INT16 ||
                weight_format == VX_TYPE_FLOAT16)
                coef16 = *((vx_int16 *)kernelDataPtr);
            else
            {
                /*Other format not suppoted now*/
                vxmASSERT(0);
            }
            kernelDataPtr += weightSize;

            if (totalCountIdx == nonCoefIndex[nonCoefCountIdx] &&
                nonCoefCountIdx < nonCoefCount)
            {
                isCoef = vx_false_e;
                nonCoefCountIdx++;
            }
            else
                isCoef = vx_true_e;

            if (limitZRLIndex != VX_NULL &&
                totalCountIdx == limitZRLIndex[limitZRLCountIdx] &&
                limitZRLCountIdx < limitZRLCount)
            {
                isLimitZRL = vx_true_e;
                limitZRLCountIdx++;
            }
            else
                isLimitZRL = vx_false_e;

            totalCountIdx++;

            if (bit16Flag == 0)
            {

                coef = (coef - wb->huffmanConfig[index].avgBias) & 0xFF;
                if (wb->huffmanConfig[index].preEncode)
                {
                    tmp = coef;
                    coef = (coef - prev) & 0xFF;
                    prev = tmp;
                }
            }
            else
            {

                if (wb->huffmanConfig[index].fp16Flag)
                {
                    coef16 = (coef16 & 0x7FFF) * 2 + coef16 / (1<<15);
                }
                coef16 -= wb->huffmanConfig[index].avgBias;
                if (wb->huffmanConfig[index].runLenTableSize == 0x0)
                    coef = (coef16 >> 8) & 0xFF; /*Non RZL*/
                else
                    coef = coef16 & 0xFFFF; /*RZL path*/
            }

            if (wb->huffmanConfig[index].runLenTableSize == 0x0) /* non RZL*/
            {
                if (!setDummy)
                {
                    /*Set dummy all to 0,
                    in nonZRL path 8-bit, 0 belong to group0, -1~0, residue bit length is 1, residue code = 0*/
                    size = 0;
                    k = invSizeOrder[size];
                    code = huffCode[k];

                    dummyStage[0] = code & 0x07;
                    dummyBitLength[0] = 3;

                    if (sizeCodeLen[k] > 3)
                    {
                        dummyStage[1] = code >> 3;
                        dummyBitLength[1] = 2;
                    }

                    dummyStage[2] = 0;
                    if (size == 0) size = 1; /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                    if (sizeCodeLen[k]%2 == 0)
                    {
                        dummyBitLength[2] = size - 1;
                    }
                    else
                    {
                        dummyBitLength[2] = size;
                    }

                    if (wb->huffmanConfig[index].bit16Flag)
                    {/*pack the raw data of the lower 8 bits*/

                        dummyBitLength[2] += 8;
                    }
                    setDummy = vx_true_e;
                }

                tmp = (coef & 0x80)? (coef ^ 0xff): coef;
                for (size = 0; size<7; size++)
                {
                    if (tmp<(1<<size))
                        break;
                }
                k = invSizeOrder[size];
                code = huffCode[k];

                if (sizeCodeLen[k]%2 == 0)/*huffCode length = 2 or 4, pack 1 bit to 3 or 5*/
                    code |= ((coef>>7)<<(sizeCodeLen[k]) );
                codeSymbol[pos].stageCode[0] = code & 0x07;
                codeSymbol[pos].bitLength[0] = 3;

                if (sizeCodeLen[k] > 3)
                {
                    codeSymbol[pos].stageCode[1] = code >> 3;
                    codeSymbol[pos].bitLength[1] = 2;
                }

                codeSymbol[pos].stageCode[2] = tmp &((1<<size) - 1 );
                if (size == 0) size = 1; /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                if (sizeCodeLen[k]%2 == 0)
                {
                    codeSymbol[pos].bitLength[2] = size - 1;
                }
                else
                {
                    codeSymbol[pos].stageCode[2] <<= 1;
                    codeSymbol[pos].stageCode[2] |= (coef>>7); /*The sign bit*/
                    codeSymbol[pos].bitLength[2] = size;
                }

                if (wb->huffmanConfig[index].bit16Flag)
                {/*pack the raw data of the lower 8 bits*/

                    codeSymbol[pos].stageCode[2] = codeSymbol[x%(3*THROUGHPUT)].stageCode[2]* 256 + (coef16 &0xff);
                    codeSymbol[pos].bitLength[2] += 8;
                }
                OutputAt(x, &kernelBufferPtr, &bitOffset, codeSymbol);
                x++; /*counter*/
            }
            else /* RZL enable*/
            {
                if (!setDummy)
                {
                    /*Set dummy all to 0,
                    in ZRL path 8-bit, 0 belong to group7, -128~127, residue bit length is 8, residue code = 0*/
                    size = 7;
                    j = invSizeOrder[size];
                    code = huffCode[j];

                    dummyStage[0] = code & 0x07;
                    dummyBitLength[0] = 3;

                    if (sizeCodeLen[j] > 3)
                    {
                        dummyStage[1] = code >> 3;
                        dummyBitLength[1] = 2;
                    }

                    if (size == 7)
                    {
                        size = 8;
                    }

                    if (sizeCodeLen[j] % 2 == 0)
                    {
                        dummyStage[2] = 0;  /*last bit was coded*/
                        dummyBitLength[2] = size - 1;
                    }
                    else
                    {
                        dummyStage[2] = 0;
                        dummyBitLength[2] = size;
                    }
                    if (wb->huffmanConfig[index].bit16Flag)
                    {
                        dummyStage[2] = 0;
                        dummyBitLength[2] += 8;
                    }

                    setDummy = vx_true_e;
                }

                if (coef == 0 && isCoef && !isLimitZRL)
                {
                    run++;
                    if (dataRemainingOfCore != 0)
                    {
                        continue;
                    }
                }
                if (coef || (dataRemainingOfCore == 0) || !isCoef || isLimitZRL) /* if last pixel, force process run zeros*/
                {
                    /* process run zeros*/
                    while(run)
                    {
                        for (j = numBestRuns - 1; j>=0; j--)
                        {
                            if (run >= bestRunsSorted[j] + 1)
                            {
                                k = run / (bestRunsSorted[j] + 1);
                                run %= (bestRunsSorted[j] + 1);
                                break;
                            }
                        }
                        if (j >= 0)
                        {
                            vx_int32 n = bestRunsSorted[j];
                            size = 8;
                            for (m = 0; m<SET_1_SIZE; m++)
                            {
                                if (best2Runs[m] == n)
                                {
                                    size = 0;
                                    break;
                                }
                            }
                            if (size == 8)
                            {
                                for (m = 0; m<numBestRuns - SET_1_SIZE; m++)
                                    if (outBest2Run[m] == n)
                                        break;

                            }
                            j = invSizeOrder[size];
                            while(k--){/*k of the runs*/
                                code = huffCode[j];
                                if (sizeCodeLen[j] % 2 == 0)/*huffCode length = 2 or 4, pack 1 bit to 3 or 5*/
                                    code |= ((m&1)<<(sizeCodeLen[j]) );
                                codeSymbol[pos].stageCode[0] = code & 0x07;
                                codeSymbol[pos].bitLength[0] = 3;

                                if (sizeCodeLen[j] > 3)
                                {
                                    codeSymbol[pos].stageCode[1] = code >> 3;
                                    codeSymbol[pos].bitLength[1] = 2;
                                }
                                if (size == 0 )
                                {/*The m already code*/
                                    codeSymbol[pos].bitLength[2] = (sizeCodeLen[j]%2 == 0) ? 0:1;
                                    codeSymbol[pos].stageCode[2] = m; /*If (sizeCodeLen[j]%2 == 0), should be m/2, but 0 bit output, doesn't matter*/
                                }
                                else if (sizeCodeLen[j]%2 == 0)
                                {
                                    codeSymbol[pos].stageCode[2] = m/2;
                                    codeSymbol[pos].bitLength[2] = runSet2Bits - 1;
                                }
                                else
                                {
                                    codeSymbol[pos].stageCode[2] = m;
                                    codeSymbol[pos].bitLength[2] = runSet2Bits;
                                }

                                OutputAt(x, &kernelBufferPtr, &bitOffset, codeSymbol);
                                x++;
                                /* update pos*/
                                pos = (x) % (3*THROUGHPUT);
                            }
                        }
                    }
                    /*process current pixel*/
                    if (coef)
                    {
                        vx_int8 ch;
                        coef>>=(wb->huffmanConfig[index].bit16Flag * 8);
                        if(coef<0x80)
                            coef+=wb->huffmanConfig[index].bit16Flag; /*Since we have to encode zero (run-length only handle 16-bits zero, 8 bit zero should use Size code*/
                        ch = (vx_int8)coef;
                        coef = abs(ch);
                        m = coef*2 + (ch<0?1:0);
                        for (j = 6; j>=0; j--)
                        {
                            if (coef>=(1<<j))
                            {
                                break;
                            }
                        }
                        size = j+1;
                        if (invSizeOrder[size]==8)
                        { /*The minimum freq, we out the raw data*/
                            size = 7;
                        }
                        if (size == 7)
                        {
                            m = ch&0xff;
                            if(wb->huffmanConfig[index].bit16Flag)
                                m = (coef16 >> 8) & 0xff;
                        }

                        j = invSizeOrder[size];
                        code = huffCode[j];
                        if (sizeCodeLen[j]%2 == 0)/*huffCode length = 2 or 4, pack 1 bit to 3 or 5*/
                            code |= ((m&1) << (sizeCodeLen[j]));
                        codeSymbol[pos].stageCode[0] = code & 0x07;
                        codeSymbol[pos].bitLength[0] = 3;

                        if (sizeCodeLen[j] > 3)
                        {
                            codeSymbol[pos].stageCode[1] = code >> 3;
                            codeSymbol[pos].bitLength[1] = 2;
                        }

                        if (size == 7)
                        {
                            size = 8;
                        }
                        /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                        if (size == 0)
                            size = 1;
                        if (sizeCodeLen[j]%2 == 0)
                        {
                            codeSymbol[pos].stageCode[2] = m/2;  /*last bit was coded*/
                            codeSymbol[pos].bitLength[2] = size - 1;
                        }
                        else
                        {
                            codeSymbol[pos].stageCode[2] = m;
                            codeSymbol[pos].bitLength[2] = size;
                        }
                        if (wb->huffmanConfig[index].bit16Flag)
                        {
                            codeSymbol[pos].stageCode[2] = codeSymbol[x%(3*THROUGHPUT)].stageCode[2]* 256 + (coef16 & 0xff);
                            codeSymbol[pos].bitLength[2] += 8;
                        }
                        OutputAt(x, &kernelBufferPtr, &bitOffset, codeSymbol);
                        x++; /*counter*/
                    }
                    else
                    {
                        /*those non-coef count like bias, num_of_vz & z_offset couldn't skip by zero-run as HW requres,
                        need also encode those 0 to huffman code,
                        in ZRL path 8-bit, 0 belong to group7, -128~127, residue bit length is 8, residue code = 0*/
                        size = 7;
                        j = invSizeOrder[size];
                        code = huffCode[j];

                        codeSymbol[pos].stageCode[0] = code & 0x07;
                        codeSymbol[pos].bitLength[0] = 3;

                        if (sizeCodeLen[j] > 3)
                        {
                            codeSymbol[pos].stageCode[1] = code >> 3;
                            codeSymbol[pos].bitLength[1] = 2;
                        }

                        if (size == 7)
                        {
                            size = 8;
                        }

                        if (sizeCodeLen[j] % 2 == 0)
                        {
                            codeSymbol[pos].stageCode[2] = 0;  /*last bit was coded*/
                            codeSymbol[pos].bitLength[2] = size - 1;
                        }
                        else
                        {
                            codeSymbol[pos].stageCode[2] = 0;
                            codeSymbol[pos].bitLength[2] = size;
                        }
                        if (wb->huffmanConfig[index].bit16Flag)
                        {
                            codeSymbol[pos].stageCode[2] = codeSymbol[x%(3*THROUGHPUT)].stageCode[2]* 256 + (coef16 & 0xff);
                            codeSymbol[pos].bitLength[2] += 8;
                        }
                        OutputAt(x, &kernelBufferPtr, &bitOffset, codeSymbol);
                        x++; /*counter*/
                    }
                }
            }

            if(dataRemainingOfCore == 0)
            {
                vx_uint32 dummyCount = (x & 0x1)? 5: 4;
                /*Hit the end bitsteam of core, add dummy stage process the next*/
                vxmASSERT(setDummy == vx_true_e);
                addDummy(x, &kernelBufferPtr, &bitOffset, codeSymbol, dummyCount, dummyStage, dummyBitLength);
                break;
            }
        }

        /* Go back to update kernelStreamSize. */
        kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);
        writeBits(&kernelStreamSizePtr, &streamSizeBitOffset, kernelStreamSize*8 + bitOffset, 32);

        /* pad 0 at the end of core*/
        packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);
        kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);

        /*With RTL bug fix, full cache mode size needn't using maxSize * coreCount*/
        wb->slice_array[index].kernel_stream_full_cache_size += kernelStreamSize;

        compressionRatio = (vx_float64)kernelStreamSize / (vx_float64)(dataSizeOfCore * weightSize);
        if (wb->max_per_core_compression_ratio < compressionRatio)
            wb->max_per_core_compression_ratio = compressionRatio;

        if (maxKernelStreamSizePerCore < kernelStreamSize)
            maxKernelStreamSizePerCore = kernelStreamSize;

        prev = 0;
    }
    vxmASSERT(reorderStreamAllCount == reorderStreamCheckCount); /*Check if the data count is the same*/
        /*bug1980*/
    if(gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_IMAGE_NOT_PACKED_IN_SRAM_FIX))
        wb->slice_array[index].kernel_align_stream_size += (vx_size)(maxKernelStreamSizePerCore * usedCoreCount );
    else
        wb->slice_array[index].kernel_align_stream_size += (vx_size)(maxKernelStreamSizePerCore * nnCoreCount);

    if (!hasKernelFullCacheInterleaveFix)
    {
        /*if did't fix full cache interleave fix, full cache size also need use align size*/
        wb->slice_array[index].kernel_stream_full_cache_size = wb->slice_array[index].kernel_align_stream_size;
    }

    wb->slice_array[index].kernel_max_stream_size_percore = maxKernelStreamSizePerCore;

exit:
    if (reorderStream)
        vxFree(reorderStream);
    if (reorderStreamPerCoreCount)
        vxFree(reorderStreamPerCoreCount);
    if (nonCoefIndex)
        vxFree(nonCoefIndex);
    if (limitZRLIndex)
        vxFree(limitZRLIndex);

    return;
}

vx_uint32 calcNonZeroCountV8Huffman(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32 skip_value
    )
{
    vx_uint32 nnCoreCount = (weight_format == VX_TYPE_INT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : (weight_format == VX_TYPE_FLOAT16) ?
                      context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;
    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;
    vx_uint32 filterCount        = filters_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;

    vx_uint32 weightCount        = wb->weights_sizes[0] * wb->weights_sizes[1];
    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize((vx_type_e)weight_format);

    vx_uint32 filterSliceSize    = weightCount * weightSize;
    vx_uint32 filterSize         = weightCount * weightSize * slice_count;

    vx_uint32 coreIndex;
    vx_uint32 filterIndex;
    vx_uint32 k, sk;
    vx_uint32 kSize;
    vx_uint32 weightXIndex, weightYIndex, weightZIndex;

    vx_bool hasZDP3 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3);
    vx_uint32 dpAmount = hasZDP3 ? 3 : 1;
    vx_uint32 zDPLoopCount = 3;
    vx_uint32 linesInImageBuffer = dpAmount * zDPLoopCount;

    vx_uint32 nonZeroCount = 0;

    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_uint32 coreFilterCount = (coreIndex < oddFilterPerCore) ? totalFilterPerCore + 1 : totalFilterPerCore;
        vx_uint32 groupIndex;
        vx_uint8_ptr realKernelDataPtr = VX_NULL;

        for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
        {
            vx_uint32 actualFilterCount = (groupIndex == groupCount - 1) ? (coreFilterCount - groupIndex * filterCount) : filterCount;
            vx_uint32 filterStart, filterEnd;

            if ((groupIndex == groupCount - 1)
                && (coreIndex >= (filterTotalCount % nnCoreCount)))
            {
                filterStart = groupIndex * nnCoreCount * filterCount
                            + (filterTotalCount % nnCoreCount) * (actualFilterCount + 1)
                            + (coreIndex - (filterTotalCount % nnCoreCount)) * actualFilterCount;
            }
            else
            {
                filterStart = groupIndex * nnCoreCount *filterCount + coreIndex * actualFilterCount;
            }
            filterEnd = filterStart + actualFilterCount - 1;

            for (k = 0; k < weightCount * slice_count; k += linesInImageBuffer)
            {
                kSize = gcmMIN((weightCount * slice_count - k), linesInImageBuffer);

                for (filterIndex = filterStart; filterIndex <= filterEnd; filterIndex++)
                {
                    vx_uint32 coef[9] = {skip_value};
                    vx_bool zeroBlock0 = vx_false_e;
                    vx_bool zeroBlock1 = vx_false_e;
                    vx_bool zeroBlock2 = vx_false_e;

                    for (sk = 0; sk < kSize; sk++)
                    {
                        weightZIndex = (k + sk) / weightCount;
                        weightYIndex = (k + sk) % weightCount;
                        weightYIndex /= wb->weights_sizes[0];
                        weightXIndex = (k + sk) % wb->weights_sizes[0];

                        realKernelDataPtr = weight_base_ptr + filterIndex * filterSize +
                            filterSliceSize * weightZIndex +
                            (weightYIndex * wb->weights_sizes[0] + weightXIndex) * weightSize;

                        if (weight_format == VX_TYPE_INT8)
                            coef[sk] = *((vx_int8 *)realKernelDataPtr);
                        else if (weight_format == VX_TYPE_UINT8)
                            coef[sk] = *((vx_uint8 *)realKernelDataPtr);
                        else
                            coef[sk] = *((vx_uint16 *)realKernelDataPtr);

                    }

                    if (coef[0] == skip_value &&
                        coef[1] == skip_value &&
                        coef[2] == skip_value)
                        zeroBlock0 = vx_true_e;

                    if (coef[3] == skip_value &&
                        coef[4] == skip_value &&
                        coef[5] == skip_value)
                        zeroBlock1 = vx_true_e;

                    if (coef[6] == skip_value &&
                        coef[7] == skip_value &&
                        coef[8] == skip_value)
                        zeroBlock2 = vx_true_e;

                    if (zeroBlock0 && zeroBlock1 && zeroBlock2)
                        nonZeroCount += 0;
                    else if (zeroBlock0 || zeroBlock1 || zeroBlock2)
                        nonZeroCount += 6;
                    else
                        nonZeroCount += 9;
                }
            }
        }
    }
    return nonZeroCount;
}

#define DUMP_BF16_ENC 0

void reorderDepthWiseKernelBufferV8Huffman(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_uint32 skip_value,
    vx_enum  weight_format,
    vx_uint8_ptr reorder_stream,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_uint32 nn_core_count,
    vx_uint32* reoder_stream_count_per_core,
    vx_uint32* non_coef_index,
    vx_uint32* limit_zrl_Index
    )
{
    vx_uint32 nnCoreCount = nn_core_count;
    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;
    vx_uint32 filterCount        = filters_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;

    vx_uint32 weightCount        = wb->weights_sizes[0] * wb->weights_sizes[1];
    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize((vx_type_e)weight_format);
    vx_uint32 weightBitSize      = weightSize * 8;

    vx_uint32 filterSliceSize    = weightCount * weightSize;
    vx_uint32 filterSize         = weightCount * weightSize * slice_count;
    vx_uint8* kernelBufferInt8Ptr      = reorder_stream;
    vx_uint16* kernelBufferInt16Ptr    = (vx_uint16*)reorder_stream;

    vx_uint32 coreIndex;
    vx_uint32 filterIndex;
    vx_uint32 core0FilterCount = 0;
    vx_uint32 k, sk;
    vx_uint32 kSize;
    vx_uint32 weightXIndex, weightYIndex, weightZIndex;

    vx_bool hasZDP3 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3);
    vx_uint32 dpAmount = hasZDP3 ? 3 : 1;
    vx_uint32 zDPLoopCount = 3;
    vx_uint32 linesInImageBuffer = dpAmount * zDPLoopCount;

    vx_uint32 nonCoefIndex = 0, idx = 0, elementIndex = 0;
    vx_uint32 limitIndex = 0;

    if (weightBitSize == 0)
    {
        vxError("%s: weightBitSize should not be zero.", __FUNCTION__);
        return;
    }

    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_uint32 coreFilterCount = (coreIndex < oddFilterPerCore) ? totalFilterPerCore + 1 : totalFilterPerCore;
        vx_uint32 groupIndex;
        vx_uint32 coef = 0;
        vx_uint8_ptr realKernelDataPtr = VX_NULL;
        reoder_stream_count_per_core[coreIndex] = 0;

        if (coreFilterCount > 0)
        {
            /* Save kernels per core*/
            if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
            {
                *((vx_uint16 *)kernelBufferInt8Ptr) = (vx_uint16)coreFilterCount;
                kernelBufferInt8Ptr += 2;
            }
            else
            {
                *kernelBufferInt16Ptr = (vx_uint16)coreFilterCount;
                kernelBufferInt16Ptr ++;
            }
            reoder_stream_count_per_core[coreIndex] += (16 / weightBitSize);

            for (idx = 0; idx < (16 / weightBitSize); idx++)
            {
                non_coef_index[nonCoefIndex++] = elementIndex++;
            }
        }

        for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
        {
            vx_uint32 actualFilterCount = (groupIndex == groupCount - 1) ? (coreFilterCount - groupIndex * filterCount) : filterCount;
            vx_uint32 groupFilterStart = groupIndex * nnCoreCount * filterCount;
            vx_uint32 adjFilterStart = groupFilterStart + nnCoreCount - coreIndex - 1;
            vx_uint32 kid = 0;
            vx_uint32 unusedCoreCount = (filterTotalCount % nnCoreCount == 0) ? 0 : nnCoreCount - (filterTotalCount % nnCoreCount);

            if (coreIndex == 0)
                core0FilterCount = actualFilterCount;

            for (kid = 0; kid < actualFilterCount; kid++)
            {
                if (groupIndex == groupCount - 1 &&
                    kid == core0FilterCount - 1)
                    filterIndex = adjFilterStart + kid * nnCoreCount - unusedCoreCount;
                else
                    filterIndex = adjFilterStart + kid * nnCoreCount;

                for (k = 0; k < weightCount * slice_count; k += linesInImageBuffer)
                {
                    kSize = gcmMIN((weightCount * slice_count - k), linesInImageBuffer);

                    for (sk = 0; sk < linesInImageBuffer; sk++)
                    {
                        weightZIndex = (k + sk) / weightCount;
                        weightYIndex = (k + sk) % weightCount;
                        weightYIndex /= wb->weights_sizes[0];
                        weightXIndex = (k + sk) % wb->weights_sizes[0];
                        /*Packl 0 in the last dp group*/
                        if (sk >= kSize)
                            coef = skip_value;
                        else
                        {
                            realKernelDataPtr = weight_base_ptr + filterIndex * filterSize +
                                filterSliceSize * weightZIndex +
                                (weightYIndex * wb->weights_sizes[0] + weightXIndex) * weightSize;

                            if (weight_format == VX_TYPE_INT8)
                                coef = *((vx_int8 *)realKernelDataPtr);
                            else if (weight_format == VX_TYPE_UINT8)
                                coef = *((vx_uint8 *)realKernelDataPtr);
                            else
                                coef = *((vx_uint16 *)realKernelDataPtr);
                        }
                        if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
                        {
                            vx_int8 newCoef = ((vx_int32)coef - (vx_int32)skip_value) & 0xFF;
                            *kernelBufferInt8Ptr = (vx_uint8)newCoef;
                            kernelBufferInt8Ptr++;
                        }
                        else
                        {
                            if (weight_format == VX_TYPE_BFLOAT16)
                            {
                                vx_uint16 exp = (coef & 0x7F80) >> 7;
                                vx_uint16 mantissa = coef & 0x7F;
                                vx_uint8 signedBit = (coef & 0x8000) >> 15;
                                vx_uint16 temp = 0;

                                /*we didn't suppot INF & NAN*/
                                vxmASSERT(exp != 0xFFFF);

                                /* convert -zero to +zero*/
                                if (exp == 0 && mantissa == 0)
                                    signedBit = 0;

                                /*newCoef [15:0] = {~OrgValue[14],OrgValue[13:0], OrgValue[15]}*/
                                exp = exp ^ 0x80; /*~OrgValue[14]*/
                                temp = (exp << 8) | (mantissa << 1) | signedBit;

                                *kernelBufferInt16Ptr = temp;
                                kernelBufferInt16Ptr++;

                            }
                            else
                            {
                                if (weight_format == VX_TYPE_FLOAT16)
                                    coef = (coef & 0x7fff) * 2 + coef/(1<<15);

                                *kernelBufferInt16Ptr = (vx_uint16)coef;
                                kernelBufferInt16Ptr++;
                            }
                        }
                        reoder_stream_count_per_core[coreIndex]++;
                        if (weightXIndex == 0 && weightYIndex == 0 && weightZIndex == 0)
                            limit_zrl_Index[limitIndex++] = elementIndex;
                        elementIndex++;
                    }
                }

                /*Driver split the Float32 Bias to BFloat16 BiasH, BiasM and BiasL and add them as coefs to the end of the coef stream*/
                if (weight_format == VX_TYPE_BFLOAT16)
                {
                    vx_uint16 biasL = 0;
                    vx_uint16 expL = 0;
                    vx_uint16 mantissaL = 0;
                    vx_uint16 biasM = 0;
                    vx_uint16 expM = 0;
                    vx_uint16 mantissaM = 0;
                    vx_uint16 biasH = 0;
                    vx_uint16 expH = 0;
                    vx_uint16 mantissaH = 0;
                    vx_uint32 biasData = 0;
                    vx_uint8 exp = 0;
                    vx_uint32 mantissa = 0;
                    vx_uint8 shift = 0;
                    vx_uint8 i = 0;
                    vx_uint8 signedBit = 0;

                    if (bias_base_ptr != VX_NULL)
                        biasData = *(bias_base_ptr + filterIndex);
                    else
                        biasData = 0;

                    exp = (vx_uint8)((biasData & 0x7F800000) >> 23);
                    mantissa = biasData & 0x7FFFFF;
                    signedBit = (biasData & 0x80000000) >> 31;


                    /*biasH is the high 16 bit of orginal 32 bit bias*/
                    biasH = biasData >> 16;
                    expH = (biasH & 0x7F80) >> 7;
                    mantissaH = biasH & 0x7F;

                    /*we didn't suppot INF & NAN*/
                    vxmASSERT(expH != 0xFFFF);
                    /* convert -zero to +zero*/
                    if (exp == 0 && mantissa == 0)
                        signedBit = 0;

                    /*newCoef [15:0] = {~OrgValue[14],OrgValue[13:0], OrgValue[15]}*/
                    expH = expH ^ 0x80; /*~OrgValue[14]*/
                    biasH = (expH << 8) | (mantissaH << 1) | signedBit;
                    *kernelBufferInt16Ptr = biasH;
                    kernelBufferInt16Ptr++;
                    non_coef_index[nonCoefIndex++] = elementIndex++;

                    /*biasM contain 8 bit mantissa[15:8]*/
                    mantissaM = (mantissa & 0xFF00) >> 8;
                    expM = exp;
                    shift = 0;

                    if (mantissaM == 0)
                    {
                        biasM = 0;
                        expM = 0;
                        signedBit = 0;
                    }
                    else
                    {
                        signedBit = (biasData & 0x80000000) >> 31;
                        /*if find the most left 1 of manitssa, will set it as hidden bit*/
                        for (i = 0; i < 8; i++)
                        {
                            vx_uint8 temp = mantissaM & 0x80;
                            mantissaM = mantissaM << 1;

                            if (temp != 0)
                            {
                                mantissaM &= 0xFF;
                                break;
                            }

                            shift++;
                        }
                        expM -= shift;

                    }

                    /*newCoef [15:0] = {~OrgValue[14],OrgValue[13:0], OrgValue[15]}*/
                    expM = expM ^ 0x80; /*~OrgValue[14]*/
                    biasM = (expM << 8) | mantissaM | signedBit;

                    *kernelBufferInt16Ptr = biasM;
                    kernelBufferInt16Ptr++;
                    non_coef_index[nonCoefIndex++] = elementIndex++;

                    /*biasL contain 8 bit mantissa[7:0]*/
                    mantissaL = mantissa & 0xFF;
                    expL = exp;
                    shift = 0;

                    if (mantissaL == 0)
                    {
                        biasL = 0;
                        expL = 0;
                        signedBit = 0;
                    }
                    else
                    {
                        signedBit = (biasData & 0x80000000) >> 31;
                        /*if find the most left 1 of manitssa, will set it as hidden bit*/
                        for (i = 0; i < 8; i++)
                        {
                            vx_uint8 temp = mantissaL & 0x80;
                            mantissaL = mantissaL << 1;

                            if (temp != 0)
                            {
                                mantissaL &= 0xFF;
                                break;
                            }

                            shift++;
                        }
                        expL -= shift;
                        /*newCoef [15:0] = {~OrgValue[14],OrgValue[13:0], OrgValue[15]}*/
                    }

                    expL = expL ^ 0x80; /*~OrgValue[14]*/
                    biasL = (expL << 8) | mantissaL | signedBit;

                    *kernelBufferInt16Ptr = biasL;
                    kernelBufferInt16Ptr++;
                    non_coef_index[nonCoefIndex++] = elementIndex++;
                    reoder_stream_count_per_core[coreIndex] += 3;

                }
            }
        }
    }
    vxmASSERT(limitIndex == filterTotalCount);
}

void reorderKernelBufferV8Huffman(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_uint32 skip_value,
    vx_enum  weight_format,
    vx_uint8_ptr reorder_stream,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_uint32 nn_core_count,
    vx_uint32* reoder_stream_count_per_core,
    vx_uint32* non_coef_index,
    vx_uint32* limit_zrl_Index
    )
{
    vx_uint32 nnCoreCount = nn_core_count;
    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;
    vx_uint32 filterCount        = filters_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;

    vx_uint32 weightCount        = wb->weights_sizes[0] * wb->weights_sizes[1];
    vx_uint32 filterWeightCount  = weightCount * slice_count;
    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize((vx_type_e)weight_format);
    vx_uint32 weightBitSize      = weightSize * 8;

    vx_uint32 filterSliceSize    = weightCount * weightSize;
    vx_uint32 filterSize         = weightCount * weightSize * slice_count;
    vx_uint8* kernelBufferInt8Ptr    = reorder_stream;
    vx_uint16* kernelBufferInt16Ptr    = (vx_uint16*)reorder_stream;

    vx_uint32 coreIndex;
    vx_uint32 limitIndex = 0;
    vx_uint32 filterIndex;
    vx_uint32 k, sk;
    vx_uint32 kSize;
    vx_uint32 weightXIndex, weightYIndex, weightZIndex;

    vx_bool hasZDP3 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3);
    vx_uint32 dpAmount = hasZDP3 ? 3 : 1;
    vx_uint32 zDPLoopCount = 3;
    vx_uint32 linesInImageBuffer = dpAmount * zDPLoopCount;

    vx_uint32 nonCoefIndex = 0, idx = 0, elementIndex = 0;
    vx_bool isBias = vx_false_e;
    vx_uint32 biasIdx = 0;
#if DUMP_BF16_ENC
    static vx_uint32 n = 0, offset = 0;
    vx_char fileName[128];
    FILE * pfile1 = NULL;
    gcoOS_PrintStrSafe(fileName, sizeof(fileName), &offset, "%s_%d.txt", "reordered_kernel", n++);

    pfile1 = fopen(fileName, "wt");
#endif

    if (weightBitSize == 0)
    {
        vxError("%s: weightBitSize should not be zero.", __FUNCTION__);
#if DUMP_BF16_ENC
        if (pfile1) fclose(pfile1);
#endif
        return;
    }

    if (weight_format == VX_TYPE_BFLOAT16)
        filterWeightCount += 3;

    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_uint32 coreFilterCount = (coreIndex < oddFilterPerCore) ? totalFilterPerCore + 1 : totalFilterPerCore;
        vx_uint32 groupIndex;
        vx_uint32 coef = 0;
        vx_uint8_ptr realKernelDataPtr = VX_NULL;
        reoder_stream_count_per_core[coreIndex] = 0;

        if (coreFilterCount > 0)
        {
            /* Save kernels per core*/
            if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
            {
                *((vx_uint16 *)kernelBufferInt8Ptr) = (vx_uint16)coreFilterCount;
                kernelBufferInt8Ptr += 2;
            }
            else
            {
                *kernelBufferInt16Ptr = (vx_uint16)coreFilterCount;
                kernelBufferInt16Ptr ++;
            }
            reoder_stream_count_per_core[coreIndex] += (16 / weightBitSize);

            for (idx = 0; idx < (16 / weightBitSize); idx++)
            {
                non_coef_index[nonCoefIndex++] = elementIndex++;
            }
        }

        for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
        {
            vx_uint32 actualFilterCount = (groupIndex == groupCount - 1) ? (coreFilterCount - groupIndex * filterCount) : filterCount;
            vx_uint32 filterStart, filterEnd;

            if ((groupIndex == groupCount - 1)
                && (coreIndex >= (filterTotalCount % nnCoreCount)))
            {
                filterStart = groupIndex * nnCoreCount * filterCount
                            + (filterTotalCount % nnCoreCount) * (actualFilterCount + 1)
                            + (coreIndex - (filterTotalCount % nnCoreCount)) * actualFilterCount;
            }
            else
            {
                filterStart = groupIndex * nnCoreCount *filterCount + coreIndex * actualFilterCount;
            }
            filterEnd = filterStart + actualFilterCount - 1;

            for (k = 0; k < filterWeightCount; k += linesInImageBuffer)
            {
                vx_uint32 maxKCount = ((filterWeightCount) % linesInImageBuffer == 0) ? (filterWeightCount / linesInImageBuffer) : (filterWeightCount / linesInImageBuffer + 1);
                kSize = gcmMIN((filterWeightCount - k), linesInImageBuffer);

                for (filterIndex = filterStart; filterIndex <= filterEnd; filterIndex++)
                {
                    for (sk = 0; sk < linesInImageBuffer; sk++)
                    {
                        isBias = ((k + sk >= filterWeightCount - 3) && (sk < kSize) && (weight_format == VX_TYPE_BFLOAT16)) ? vx_true_e : vx_false_e;
                        biasIdx = isBias ? (k + sk + 3 - filterWeightCount) : 0;

                        if (isBias)
                        {
                            vx_uint16 biasL = 0;
                            vx_uint16 expL = 0;
                            vx_uint16 mantissaL = 0;
                            vx_uint16 biasM = 0;
                            vx_uint16 expM = 0;
                            vx_uint16 mantissaM = 0;
                            vx_uint16 biasH = 0;
                            vx_uint16 expH = 0;
                            vx_uint16 mantissaH = 0;
                            vx_uint32 biasData = 0;
                            vx_uint8 exp = 0;
                            vx_uint32 mantissa = 0;
                            vx_uint8 shift = 0;
                            vx_uint8 i = 0;
                            vx_uint8 signedBit = 0;

                            if (bias_base_ptr != VX_NULL)
                                biasData = *(bias_base_ptr + filterIndex);
                            else
                                biasData = 0;

                            exp = (vx_uint8)((biasData & 0x7F800000) >> 23);
                            mantissa = biasData & 0x7FFFFF;
                            signedBit = (biasData & 0x80000000) >> 31;

                            switch (biasIdx)
                            {
                            case 0:
                                {
                                    /*biasH is the high 16 bit of orginal 32 bit bias*/
                                    biasH = biasData >> 16;
                                    expH = (biasH & 0x7F80) >> 7;
                                    mantissaH = biasH & 0x7F;
#if DUMP_BF16_ENC
                                    if(pfile1 != NULL)
                                    {
                                        fprintf(pfile1, "vz:%d, biasH: 0x%x\n", filterIndex, biasH);
                                    }
#endif
                                    /*we didn't suppot INF & NAN*/
                                    vxmASSERT(expH != 0xFFFF);
                                    /* convert -zero to +zero*/
                                    if (exp == 0 && mantissa == 0)
                                        signedBit = 0;

                                    /*newCoef [15:0] = {~OrgValue[14],OrgValue[13:0], OrgValue[15]}*/
                                    expH = expH ^ 0x80; /*~OrgValue[14]*/
                                    biasH = (expH << 8) | (mantissaH << 1) | signedBit;
                                    *kernelBufferInt16Ptr = biasH;
                                    kernelBufferInt16Ptr++;
                                    non_coef_index[nonCoefIndex++] = elementIndex++;
                                    reoder_stream_count_per_core[coreIndex]++;
#if DUMP_BF16_ENC
                                    if(pfile1 != NULL)
                                    {
                                        fprintf(pfile1, "vz:%d, compressed  biasH: 0x%x\n", filterIndex, biasH);
                                    }
#endif
                                }
                                break;
                            case 1:
                                {
                                    /*biasM contain 8 bit mantissa[15:8]*/
                                    mantissaM = (mantissa & 0xFF00) >> 8;
                                    expM = exp;
                                    shift = 0;

                                    if (mantissaM == 0)
                                    {
                                        biasM = 0;
                                        expM = 0;
                                        signedBit = 0;
                                    }
                                    else
                                    {
                                        signedBit = (biasData & 0x80000000) >> 31;
                                        /*if find the most left 1 of manitssa, will set it as hidden bit*/
                                        for (i = 0; i < 8; i++)
                                        {
                                            vx_uint8 temp = mantissaM & 0x80;
                                            mantissaM = mantissaM << 1;

                                            if (temp != 0)
                                            {
                                                mantissaM &= 0xFF;
                                                break;
                                            }

                                            shift++;
                                        }
                                        expM -= shift;
                                        /*newCoef [15:0] = {~OrgValue[14],OrgValue[13:0], OrgValue[15]}*/
                                    }
#if DUMP_BF16_ENC
                                    if(pfile1 != NULL)
                                    {
                                        biasM = (expM << 7) | (mantissaM >> 1) | ((vx_uint16)signedBit << 15);
                                        fprintf(pfile1, "vz:%d, biasM: 0x%x\n", filterIndex, biasM);
                                    }
#endif
                                    /*newCoef [15:0] = {~OrgValue[14],OrgValue[13:0], OrgValue[15]}*/
                                    expM = expM ^ 0x80; /*~OrgValue[14]*/
                                    biasM = (expM << 8) | mantissaM | signedBit;

                                    *kernelBufferInt16Ptr = biasM;
                                    kernelBufferInt16Ptr++;
                                    non_coef_index[nonCoefIndex++] = elementIndex++;
                                    reoder_stream_count_per_core[coreIndex]++;
#if DUMP_BF16_ENC
                                    if(pfile1 != NULL)
                                    {
                                        fprintf(pfile1, "vz:%d, compressed biasM: 0x%x\n", filterIndex, biasM);
                                    }
#endif
                                }
                                break;
                            case 2:
                                {
                                    /*biasL contain 8 bit mantissa[7:0]*/
                                    mantissaL = mantissa & 0xFF;
                                    expL = exp;
                                    shift = 0;

                                    if (mantissaL == 0)
                                    {
                                        biasL = 0;
                                        expL = 0;
                                        signedBit = 0;
                                    }
                                    else
                                    {
                                        signedBit = (biasData & 0x80000000) >> 31;
                                        /*if find the most left 1 of manitssa, will set it as hidden bit*/
                                        for (i = 0; i < 8; i++)
                                        {
                                            vx_uint8 temp = mantissaL & 0x80;
                                            mantissaL = mantissaL << 1;

                                            if (temp != 0)
                                            {
                                                mantissaL &= 0xFF;
                                                break;
                                            }

                                            shift++;
                                        }
                                        expL -= shift;
                                        /*newCoef [15:0] = {~OrgValue[14],OrgValue[13:0], OrgValue[15]}*/
                                    }
#if DUMP_BF16_ENC
                                    if(pfile1 != NULL)
                                    {
                                        biasL = (expL << 7) | (mantissaL >> 1) | ((vx_uint16)signedBit << 15);
                                        fprintf(pfile1, "vz:%d, biasL: 0x%x\n", filterIndex, biasL);
                                    }
#endif
                                    expL = expL ^ 0x80; /*~OrgValue[14]*/
                                    biasL = (expL << 8) | mantissaL | signedBit;

                                    *kernelBufferInt16Ptr = biasL;
                                    kernelBufferInt16Ptr++;
                                    non_coef_index[nonCoefIndex++] = elementIndex++;
                                    reoder_stream_count_per_core[coreIndex]++;
#if DUMP_BF16_ENC
                                    if(pfile1 != NULL)
                                    {
                                        fprintf(pfile1, "vz:%d, compressed biasL: 0x%x\n", filterIndex, biasL);
                                    }
#endif
                                }
                                break;
                            default:
                                vxmASSERT(0);
                                break;
                            }
                            continue;
                        }

                        weightZIndex = (k + sk) / weightCount;
                        weightYIndex = (k + sk) % weightCount;
                        weightYIndex /= wb->weights_sizes[0];
                        weightXIndex = (k + sk) % wb->weights_sizes[0];
                        /*Packl 0 in the last dp group*/
                        if (sk >= kSize)
                            coef = skip_value;
                        else
                        {
                            realKernelDataPtr = weight_base_ptr + filterIndex * filterSize +
                                filterSliceSize * weightZIndex +
                                (weightYIndex * wb->weights_sizes[0] + weightXIndex) * weightSize;

                            if (weight_format == VX_TYPE_INT8)
                                coef = *((vx_int8 *)realKernelDataPtr);
                            else if (weight_format == VX_TYPE_UINT8)
                                coef = *((vx_uint8 *)realKernelDataPtr);
                            else
                                coef = *((vx_uint16 *)realKernelDataPtr);
                        }

                        if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
                        {
                            vx_int8 newCoef = ((vx_int32)coef - (vx_int32)skip_value) & 0xFF;
                            *kernelBufferInt8Ptr = (vx_uint8)newCoef;
                            kernelBufferInt8Ptr++;
                        }
                        else
                        {
                            if (weight_format == VX_TYPE_BFLOAT16)
                            {
                                vx_uint16 exp = (coef & 0x7F80) >> 7;
                                vx_uint16 mantissa = coef & 0x7F;
                                vx_uint8 signedBit = (coef & 0x8000) >> 15;
                                vx_uint16 temp = 0;

                                /*we didn't suppot INF & NAN*/
                                vxmASSERT(exp != 0xFFFF);

                                /* convert -zero to +zero*/
                                if (exp == 0 && mantissa == 0)
                                    signedBit = 0;

                                /*newCoef [15:0] = {~OrgValue[14],OrgValue[13:0], OrgValue[15]}*/
                                exp = exp ^ 0x80; /*~OrgValue[14]*/
                                temp = (exp << 8) | (mantissa << 1) | signedBit;

                                *kernelBufferInt16Ptr = temp;
                                kernelBufferInt16Ptr++;
#if DUMP_BF16_ENC
                                if(pfile1 != NULL)
                                {
                                    fprintf(pfile1, "kx:%d, ky:%d, kz:%d, vz:%d, org coef:0x%x, copressed coef: 0x%x\n", weightXIndex, weightYIndex, weightZIndex, filterIndex, coef, temp);
                                }
#endif
                            }
                            else
                            {
                                if (weight_format == VX_TYPE_FLOAT16)
                                    coef = (coef & 0x7fff) * 2 + coef/(1<<15);

                                *kernelBufferInt16Ptr = (vx_uint16)coef;
                                kernelBufferInt16Ptr++;
                            }
                        }
                        reoder_stream_count_per_core[coreIndex]++;

                        if (k == (maxKCount - 1) * linesInImageBuffer && sk == linesInImageBuffer - 1)
                            limit_zrl_Index[limitIndex++] = elementIndex;

                        elementIndex++;
                    }
                }
            }
        }
    }
#if DUMP_BF16_ENC
    if (pfile1 != NULL)
        fclose(pfile1);
#endif
    vxmASSERT(limitIndex == filterTotalCount);
}

void reorderKernelBufferV8HuffmanBalance(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_uint32 skip_value,
    vx_enum  weight_format,
    vx_uint8_ptr reorder_stream,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32 nn_core_count,
    vx_uint32* reoder_stream_count_per_core,
    vx_uint32* non_coef_index,
    vx_uint32* real_vz_index,
    vx_uint32* limit_zrl_Index
    )
{
    vx_uint32 nnCoreCount = nn_core_count;
    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 sliceCount         = slice_count;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;
    vx_uint32 filterCount        = filters_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;

    vx_uint32 weightCount        = wb->weights_sizes[0] * wb->weights_sizes[1];
    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize((vx_type_e)weight_format);
    vx_uint32 weightBitSize      = weightSize * 8;

    vx_uint32 filterSliceSize    = weightCount * weightSize;
    vx_uint32 filterSize         = weightCount * weightSize * slice_count;
    vx_uint8* kernelBufferInt8Ptr    = reorder_stream;
    vx_uint16* kernelBufferInt16Ptr    = (vx_uint16*)reorder_stream;

    vx_uint32 coreIndex;
    vx_uint32 limitIndex = 0;
    vx_uint32 filterIndex;
    vx_uint32 sliceIndex;
    vx_uint32 k, sk;
    vx_uint32 kSize;
    vx_uint32 weightXIndex, weightYIndex, weightZIndex;

    vx_bool hasZDP3 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3);
    vx_uint32 dpAmount = hasZDP3 ? 3 : 1;
    vx_uint32 zDPLoopCount = 3;
    vx_uint32 linesInImageBuffer = dpAmount * zDPLoopCount;

    vx_uint32 nonCoefIndex = 0, idx = 0, elementIndex = 0;
    vx_uint8* kernelDataPtr      = VX_NULL;

    vx_uint32 totalNonZeroCount = 0;
    vx_float32 averageNZCPerFilter = 0;
    vx_float32 varianceFL32 = 0;
    vx_uint32 variance = 0;
    vx_bool reorder = vx_false_e;

    typedef struct _gcVXNonZeroWeights
    {
        vx_uint32 nonZeroCount;
        vx_uint32 filterIdx;
    }gcVXNonZeroWeights;

#if PRINT_PER_CORE_SIZE
    vx_uint32* nonZeroCoefPerCore = VX_NULL;
#endif

    gcVXNonZeroWeights *nonZeroWeights = VX_NULL;
    vx_int32 i = 0;
    vx_uint32 core0FilterCount = 0;
    gcVXNonZeroWeights tmp;

    if (weightBitSize == 0)
    {
        vxError("%s: weightBitSize should not be zero.", __FUNCTION__);
        goto exit;
    }

    nonZeroWeights = (gcVXNonZeroWeights*)vxAllocateAndZeroMemory(filterTotalCount * sizeof(gcVXNonZeroWeights));
    if (nonZeroWeights == VX_NULL)
    {
        vxError("fillinKernelBuffer: OUT OF MEMORY");
        goto exit;
    }
#if PRINT_PER_CORE_SIZE
    nonZeroCoefPerCore = (vx_uint32*)vxAllocateAndZeroMemory(nnCoreCount * sizeof(vx_uint32));
    if (nonZeroCoefPerCore == VX_NULL)
    {
        vxError("fillinKernelBuffer: OUT OF MEMORY");
        goto exit;
    }
#endif

    /* calc each filter non-zero weights num*/
    for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
    {
        nonZeroWeights[filterIndex].filterIdx = filterIndex;
        for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
        {
            /* add one slice data every filter */
            kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * sliceIndex;

            for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex++)
            {
                for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex++)
                {
                    vx_uint32 weight = 0;

                    if (weight_format == VX_TYPE_INT8)
                        weight = *((vx_int8 *)kernelDataPtr);
                    else if (weight_format == VX_TYPE_UINT8)
                        weight = *((vx_uint8 *)kernelDataPtr);
                    else
                        weight = *((vx_uint16 *)kernelDataPtr);
                    kernelDataPtr = kernelDataPtr + weightSize;

                    if (weight != skip_value)
                        nonZeroWeights[filterIndex].nonZeroCount++;

                }
            }
        }
        totalNonZeroCount += nonZeroWeights[filterIndex].nonZeroCount;
    }

    /*Sort*/
    averageNZCPerFilter = (vx_float32)totalNonZeroCount / filterTotalCount;
    for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
    {
        varianceFL32 += gcoMATH_Power((nonZeroWeights[filterIndex].nonZeroCount - averageNZCPerFilter), 2);
    }
    variance = (vx_uint32)(varianceFL32 / filterTotalCount + 0.5f);

    reorder = vx_true_e;

    if (reorder)
    {
        /*Sort*/
        for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
        {
            tmp.nonZeroCount = nonZeroWeights[filterIndex].nonZeroCount;
            tmp.filterIdx = filterIndex;

            for (i = filterIndex - 1; i >= 0 && nonZeroWeights[i].nonZeroCount > tmp.nonZeroCount; i--)
            {
                nonZeroWeights[i+1].nonZeroCount = nonZeroWeights[i].nonZeroCount;
                nonZeroWeights[i+1].filterIdx = nonZeroWeights[i].filterIdx;
            }
            nonZeroWeights[i+1].nonZeroCount = tmp.nonZeroCount;
            nonZeroWeights[i+1].filterIdx = tmp.filterIdx;
        }
    }

    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_uint32 coreFilterCount = (coreIndex < oddFilterPerCore) ? totalFilterPerCore + 1 : totalFilterPerCore;
        vx_uint32 groupIndex;
        vx_uint32 coef = 0;
        vx_uint32 usedKid = 0;
        vx_uint32* filterGroup = VX_NULL;
        vx_uint8_ptr realKernelDataPtr = VX_NULL;
        reoder_stream_count_per_core[coreIndex] = 0;

        if (coreFilterCount > 0)
        {
            /* Save kernels per core*/
            if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
            {
                *((vx_uint16 *)kernelBufferInt8Ptr) = (vx_uint16)coreFilterCount;
                kernelBufferInt8Ptr += 2;
            }
            else
            {
                *kernelBufferInt16Ptr = (vx_uint16)coreFilterCount;
                kernelBufferInt16Ptr ++;
            }
            reoder_stream_count_per_core[coreIndex] += (16 / weightBitSize);

            for (idx = 0; idx < (16 / weightBitSize); idx++)
            {
                non_coef_index[nonCoefIndex++] = elementIndex++;
            }
        }

        for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
        {
            vx_uint32 actualFilterCount = (groupIndex == groupCount - 1) ? (coreFilterCount - groupIndex * filterCount) : filterCount;
            vx_uint32 groupFilterStart = groupIndex * nnCoreCount * filterCount;
            vx_uint32 kid = 0;
            vx_uint32 unusedCoreCount = (filterTotalCount % nnCoreCount == 0) ? 0 : nnCoreCount - (filterTotalCount % nnCoreCount);
            vx_uint32 sortedIndex = 0;
            vx_uint32 filterStart, filterEnd;

            if ((groupIndex == groupCount - 1)
                && (coreIndex >= (filterTotalCount % nnCoreCount)))
            {
                filterStart = groupIndex * nnCoreCount * filterCount
                            + (filterTotalCount % nnCoreCount) * (actualFilterCount + 1)
                            + (coreIndex - (filterTotalCount % nnCoreCount)) * actualFilterCount;
            }
            else
            {
                filterStart = groupIndex * nnCoreCount *filterCount + coreIndex * actualFilterCount;
            }
            filterEnd = filterStart + actualFilterCount - 1;

            if (coreIndex == 0)
                core0FilterCount = actualFilterCount;

            if (actualFilterCount)
            {
                filterGroup = (vx_uint32*)vxAllocateAndZeroMemory(actualFilterCount * sizeof(vx_uint32));
                if (filterGroup == VX_NULL)
                {
                    vxError("fillinKernelBufferBalance: OUT OF MEMORY");
                    goto exit;
                }
            }

            if (reorder)
            {
                /*Dispatch filter to each vzGroup*/
                for (kid = 0; kid < actualFilterCount; kid++)
                {
                    if (groupIndex == groupCount - 1 &&
                        kid == core0FilterCount - 1)
                    {
                        if ((kid + usedKid) % 2 == 0)
                            sortedIndex = groupFilterStart + kid * nnCoreCount + coreIndex;
                        else
                            sortedIndex = groupFilterStart + (kid + 1) * nnCoreCount - coreIndex - 1 - unusedCoreCount;
                    }
                    else
                    {
                        if ((kid + usedKid) % 2 == 0)
                            sortedIndex = groupFilterStart + kid * nnCoreCount + coreIndex;
                        else
                            sortedIndex = groupFilterStart + (kid + 1) * nnCoreCount - coreIndex - 1;
                    }

                    filterGroup[kid] = nonZeroWeights[sortedIndex].filterIdx;
#if PRINT_PER_CORE_SIZE
                    nonZeroCoefPerCore[coreIndex] += nonZeroWeights[sortedIndex].nonZeroCount;
#endif
                }

                /*sort the vz group real filter to avoid write output cross*/
                for (kid = 0; kid < actualFilterCount; kid++)
                {
                    vx_uint32 tmp = filterGroup[kid];

                    for (i = kid - 1; i >= 0 && filterGroup[i] > tmp; i--)
                    {
                        filterGroup[i+1] = filterGroup[i];
                    }
                    filterGroup[i+1] = tmp;
                }
            }
            else
            {
                /*if variance is small, don't reorder the filters*/
                if ((groupIndex == groupCount - 1)
                && (coreIndex >= (filterTotalCount % nnCoreCount)))
                {
                    filterStart = groupIndex * nnCoreCount * filterCount
                                + (filterTotalCount % nnCoreCount) * (actualFilterCount + 1)
                                + (coreIndex - (filterTotalCount % nnCoreCount)) * actualFilterCount;
                }
                else
                {
                    filterStart = groupIndex * nnCoreCount *filterCount + coreIndex * actualFilterCount;
                }

                for (kid = 0; kid < actualFilterCount; kid++)
                {
                    filterGroup[kid] = filterStart + kid;
                }
            }

            if (real_vz_index)
            {
                vx_uint32 id = 0;
                for (filterIndex = filterStart; filterIndex <= filterEnd; filterIndex++)
                {
                    /*Save real filter index*/
                    real_vz_index[filterIndex] = filterGroup[id++];
                }
            }

            for (k = 0; k < weightCount * slice_count; k += linesInImageBuffer)
            {
                vx_uint32 maxKCount = ((weightCount * slice_count) % linesInImageBuffer == 0) ? (weightCount * slice_count / linesInImageBuffer) : (weightCount * slice_count / linesInImageBuffer + 1);
                kSize = gcmMIN((weightCount * slice_count - k), linesInImageBuffer);

                for (kid = 0; kid < actualFilterCount; kid++)
                {
                    filterIndex = filterGroup[kid];

                    for (sk = 0; sk < linesInImageBuffer; sk++)
                    {
                        weightZIndex = (k + sk) / weightCount;
                        weightYIndex = (k + sk) % weightCount;
                        weightYIndex /= wb->weights_sizes[0];
                        weightXIndex = (k + sk) % wb->weights_sizes[0];
                        /*Packl 0 in the last dp group*/
                        if (sk >= kSize)
                            coef = skip_value;
                        else
                        {
                            realKernelDataPtr = weight_base_ptr + filterIndex * filterSize +
                                filterSliceSize * weightZIndex +
                                (weightYIndex * wb->weights_sizes[0] + weightXIndex) * weightSize;

                            if (weight_format == VX_TYPE_INT8)
                                coef = *((vx_int8 *)realKernelDataPtr);
                            else if (weight_format == VX_TYPE_UINT8)
                                coef = *((vx_uint8 *)realKernelDataPtr);
                            else
                                coef = *((vx_uint16 *)realKernelDataPtr);
                        }

                        if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
                        {
                            vx_int8 newCoef = ((vx_int32)coef - (vx_int32)skip_value) & 0xFF;
                            *kernelBufferInt8Ptr = (vx_uint8)newCoef;
                            kernelBufferInt8Ptr++;
                        }
                        else
                        {
                            if (weight_format == VX_TYPE_FLOAT16)
                                coef = (coef & 0x7fff) * 2 + coef/(1<<15);

                            *kernelBufferInt16Ptr = (vx_uint16)coef;
                            kernelBufferInt16Ptr++;
                        }
                        reoder_stream_count_per_core[coreIndex]++;
                        if (k == (maxKCount - 1) * linesInImageBuffer && sk == linesInImageBuffer - 1)
                            limit_zrl_Index[limitIndex++] = elementIndex;
                        elementIndex++;
                    }
                }
            }
            usedKid += actualFilterCount;
            if (filterGroup)
            {
                vxFree(filterGroup);
                filterGroup = VX_NULL;
            }
        }
    }
    vxmASSERT(limitIndex == filterTotalCount);
exit:

    if (nonZeroWeights)
        vxFree(nonZeroWeights);
#if PRINT_PER_CORE_SIZE
    if(nonZeroCoefPerCore)
        vxFree(nonZeroCoefPerCore);
#endif
    return;
}

vx_uint32 calcKernelSizeV8Huffman(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_uint32 skip_value,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_uint32* post_mul,
    vx_uint32* post_shift,
    vx_uint32* neg_post_mul,
    vx_uint32* neg_post_shift,
    vx_uint32 index
    )
{
    vx_uint32 nnCoreCount = (weight_format == VX_TYPE_INT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : (weight_format == VX_TYPE_FLOAT16) ?
                      context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 kernelSize = 0;
    vx_uint32 kernelBitSize = 0;
    vx_uint32 invSizeOrder[MAX_SIZE_HISTO_COUNT] = {0};
    vx_int32 coef = 0, tmp = 0, prev = 0;

    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 usedCoreCount      = (filterTotalCount > nnCoreCount) ? nnCoreCount : filterTotalCount;

    vx_uint32 weightCount        = weight_x * weight_y;
    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize((vx_type_e)weight_format);
    vx_uint32 weightBitSize      = weightSize * 8;
    vx_uint32 biasBitSize        = (weight_format == VX_TYPE_INT16) ? NN_INTEGER_BIAS_BITS_VIP_V7_INT16 : NN_INTEGER_BIAS_BITS_VIP_V7;
    vx_uint32 biasSize           = biasBitSize / 8;

    vx_uint8_ptr kernelDataPtr   = VX_NULL;

    vx_uint32 coreIndex;
    vx_uint32 i;

    vx_uint32 bit16Flag = (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8) ? 0 : 1;

    vx_uint32 reorderStreamSize = 0;
    vx_uint32 reorderStreamAllCount = 0;
    vx_uint32 reorderStreamCheckCount = 0;
    vx_uint32* reorderStreamPerCoreCount = VX_NULL;
    vx_uint32 filterIndex = 0;

    vx_uint8* reorderStream = VX_NULL;
    vx_uint32 runSet2Bits = 1;
    CodeSymbol codeSymbol[THROUGHPUT * 3];

    vx_bool hasNoBias = vx_false_e;
    vx_bool hasNoZOffset = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_NO_Z_LOCATION_OFFSET);
    vx_bool hasNNPerFilterPostMultiply = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PER_CHANNEL_POST_MULTIPLY);
    vx_bool hasNNPreLU = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PRELU);
    vx_bool hasZDP3 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3);
    vx_uint32 dpAmount = hasZDP3 ? 3 : 1;
    vx_uint32 zDPLoopCount = 3;
    vx_uint32 linesInImageBuffer = dpAmount * zDPLoopCount;
    vx_uint32 numOfInImageBuffer = (vx_uint32)gcoMATH_Ceiling((vx_float32)(weightCount * slice_count) / (vx_float32)linesInImageBuffer);

    vx_uint32 nonCoefCount = 0;
    vx_uint32* nonCoefIndex = VX_NULL;
    vx_uint32* limitZRLIndex = VX_NULL;
    vx_uint32 totalCountIdx = 0;
    vx_uint32 nonCoefCountIdx = 0;
    vx_uint32 kernelStreamSizePerCore = 0;
    vx_uint32 limitZRLCountIdx = 0;

    if (weightBitSize == 0)
    {
        vxError("%s: weightBitSize should not be zero.", __FUNCTION__);
        goto exit;
    }

    if (weight_format == VX_TYPE_BFLOAT16)
    {
        numOfInImageBuffer = (vx_uint32)gcoMATH_Ceiling((vx_float32)(weightCount * slice_count + 3) / (vx_float32)linesInImageBuffer);
    }

    reorderStreamAllCount = (numOfInImageBuffer * linesInImageBuffer * filterTotalCount)
        + (usedCoreCount * (16 / weightBitSize));/*filterPerCore need 16 bit * nnCoreCount */
    reorderStreamSize = reorderStreamAllCount * weightSize;
    if (hasNNPerFilterPostMultiply)
        reorderStreamSize += filterTotalCount; /*Per channel 8 bit zero point*/

    reorderStream = (vx_uint8 *)vxAllocateAndZeroMemory(reorderStreamSize);

    if (!reorderStream)
    {
        vxError("fillinKernelBufferHuffman: OUT OF MEMORY");
        goto exit;
    }

    reorderStreamPerCoreCount = (vx_uint32 *)vxAllocateAndZeroMemory(nnCoreCount * sizeof(vx_uint32));
    if (!reorderStreamPerCoreCount)
    {
        vxError("fillinKernelBufferHuffman: OUT OF MEMORY");
        goto exit;
    }

    nonCoefCount = usedCoreCount * (16 / weightBitSize);
    if (hasNNPerFilterPostMultiply)
        nonCoefCount += filterTotalCount;
    if (weight_format == VX_TYPE_BFLOAT16)
        nonCoefCount += filterTotalCount * 3; /*BFLOAT16 fp32 bias will be splitted to 3 BF16 to coef stream*/


    nonCoefIndex = (vx_uint32 *)vxAllocateAndZeroMemory(nonCoefCount * sizeof(vx_uint32));
    if (!nonCoefIndex)
    {
        vxError("fillinKernelBufferHuffman: OUT OF MEMORY");
        goto exit;
    }

    limitZRLIndex = (vx_uint32 *)vxAllocateAndZeroMemory(filterTotalCount * sizeof(vx_uint32));
    if (!limitZRLIndex)
    {
        vxError("fillinKernelBufferHuffman: OUT OF MEMORY");
        goto exit;
    }

    if (wb->wb_base->hw_depth_wise)
        reorderDepthWiseKernelBufferV8Huffman(context, wb, slice_count, z_count, filters_per_core, skip_value, weight_format, reorderStream, weight_base_ptr, bias_base_ptr, nnCoreCount, reorderStreamPerCoreCount, nonCoefIndex, limitZRLIndex);
    else if (context->options.enableNonZeroBalance && !hasNoZOffset) /*Only those chip has z_offset support zero balance*/
        reorderKernelBufferV8HuffmanBalance(context, wb, slice_count, z_count, filters_per_core, skip_value, weight_format, reorderStream, weight_base_ptr, nnCoreCount, reorderStreamPerCoreCount, nonCoefIndex, VX_NULL, limitZRLIndex);
    else
        reorderKernelBufferV8Huffman(context, wb, slice_count, z_count, filters_per_core, skip_value, weight_format, reorderStream, weight_base_ptr, bias_base_ptr, nnCoreCount, reorderStreamPerCoreCount, nonCoefIndex, limitZRLIndex);

    analysisKernelStreamForHuffman(context, wb, 0, nnCoreCount, reorderStream, reorderStreamSize, reorderStreamPerCoreCount, invSizeOrder, nonCoefIndex, nonCoefCount, limitZRLIndex, filterTotalCount, index);

    kernelBitSize += 16;
    kernelBitSize += 8 * MAX_RUNLEN_SIZE;
    kernelBitSize += 8 * LOG_RUN_SIZE1;
    kernelBitSize += 32;
    kernelBitSize += 32 * nnCoreCount;

    kernelSize = (kernelBitSize % 8 == 0) ? kernelBitSize / 8 : kernelBitSize / 8 + 1;
    /*align to 64 byte*/
    kernelSize = (kernelSize + 63) & 0xFFFFFFC0;
    kernelBitSize = 0;

    for (i = 0; i < THROUGHPUT * 3; i++)
    {
        codeSymbol[i].stageCode[0] = codeSymbol[i].stageCode[1] = codeSymbol[i].stageCode[2] = 0;
        codeSymbol[i].bitLength[0] = codeSymbol[i].bitLength[1] = codeSymbol[i].bitLength[2] = 0;
    }

    while(((numBestRuns-SET_1_SIZE)>>runSet2Bits) > 1)
    {
        runSet2Bits++;
    }

    kernelDataPtr = reorderStream;
    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_int32 run = 0x0;
        vx_uint32 dataSizeOfCore = reorderStreamPerCoreCount[coreIndex];
        vx_uint32 dataRemainingOfCore = dataSizeOfCore;
        vx_int32 size = 0;
        //vx_int32 code = 0;
        vx_int32 coef16 = 0;
        vx_int32 k = 0, j = 0, m = 0;
        vx_bool isCoef;
        vx_bool isLimitZRL;
        /* update position of each core's beginning of compressed kernel*/
        reorderStreamCheckCount += reorderStreamPerCoreCount[coreIndex];

        memset(codeSymbol, 0, sizeof(CodeSymbol));

        while(dataRemainingOfCore --)
        {
            if (weight_format == VX_TYPE_INT8)
                coef = *((vx_int8 *)kernelDataPtr);
            else if (weight_format == VX_TYPE_UINT8)
                coef = *((vx_uint8 *)kernelDataPtr);
            else if (weight_format == VX_TYPE_INT16 ||
                weight_format == VX_TYPE_FLOAT16 ||
                weight_format == VX_TYPE_BFLOAT16)
                coef16 = *((vx_int16 *)kernelDataPtr);
            else
            {
                /*Other format not suppoted now*/
                vxmASSERT(0);
            }
            kernelDataPtr += weightSize;

            if (totalCountIdx == nonCoefIndex[nonCoefCountIdx] &&
                nonCoefCountIdx < nonCoefCount)
            {
                isCoef = vx_false_e;
                nonCoefCountIdx++;
            }
            else
                isCoef = vx_true_e;

            if (limitZRLIndex != VX_NULL &&
                totalCountIdx == limitZRLIndex[limitZRLCountIdx] &&
                limitZRLCountIdx < filterTotalCount)
            {
                isLimitZRL = vx_true_e;
                limitZRLCountIdx++;
            }
            else
                isLimitZRL = vx_false_e;

            totalCountIdx++;

            if (bit16Flag == 0)
            {

                coef = (coef - wb->huffmanConfig[index].avgBias) & 0xFF;
                if (wb->huffmanConfig[index].preEncode)
                {
                    tmp = coef;
                    coef = (coef - prev) & 0xFF;
                    prev = tmp;
                }
            }
            else
            {

                if (wb->huffmanConfig[index].fp16Flag)
                {
                    coef16 = (coef16 & 0x7FFF) * 2 + coef16 / (1<<15);
                }
                coef16 -= wb->huffmanConfig[index].avgBias;
                if (wb->huffmanConfig[index].runLenTableSize == 0x0)
                    coef = (coef16 >> 8) & 0xFF; /*Non RZL*/
                else
                    coef = coef16 & 0xFFFF; /*RZL path*/
            }

            if (wb->huffmanConfig[index].runLenTableSize == 0x0) /* non RZL*/
            {
                tmp = (coef & 0x80)? (coef ^ 0xff): coef;
                for (size = 0; size<7; size++)
                {
                    if (tmp<(1<<size))
                        break;
                }
                k = invSizeOrder[size];
                kernelBitSize += 3;

                if (sizeCodeLen[k] > 3)
                {
                    kernelBitSize += 2;
                }

                if (size == 0) size = 1; /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                if (sizeCodeLen[k]%2 == 0)
                {
                    kernelBitSize += (size - 1);
                }
                else
                {
                    kernelBitSize += size;
                }

                if (wb->huffmanConfig[index].bit16Flag)
                {/*pack the raw data of the lower 8 bits*/
                    kernelBitSize += 8;
                }
            }
            else /* RZL enable*/
            {
                if (coef == 0 && isCoef && !isLimitZRL)
                {
                    run++;
                    if (dataRemainingOfCore != 0)
                    {
                        continue;
                    }
                }
                if (coef || (dataRemainingOfCore == 0) || !isCoef || isLimitZRL) /* if last pixel, force process run zeros*/
                {
                    /* process run zeros*/
                    while(run)
                    {
                        for (j = numBestRuns - 1; j>=0; j--)
                        {
                            if (run >= bestRunsSorted[j] + 1)
                            {
                                k = run / (bestRunsSorted[j] + 1);
                                run %= (bestRunsSorted[j] + 1);
                                break;
                            }
                        }
                        if (j >= 0)
                        {
                            vx_int32 n = bestRunsSorted[j];
                            size = 8;
                            for (m = 0; m<SET_1_SIZE; m++)
                            {
                                if (best2Runs[m] == n)
                                {
                                    size = 0;
                                    break;
                                }
                            }
                            if (size == 8)
                            {
                                for (m = 0; m<numBestRuns - SET_1_SIZE; m++)
                                    if (outBest2Run[m] == n)
                                        break;

                            }
                            j = invSizeOrder[size];
                            while(k--)
                            {/*k of the runs*/
                                kernelBitSize += 3;

                                if (sizeCodeLen[j] > 3)
                                {
                                    kernelBitSize += 2;
                                }
                                if (size == 0 )
                                {/*The m already code*/
                                    kernelBitSize += ((sizeCodeLen[j]%2 == 0) ? 0:1);
                                }
                                else if (sizeCodeLen[j]%2 == 0)
                                {
                                    kernelBitSize += (runSet2Bits - 1);
                                }
                                else
                                {
                                    kernelBitSize += runSet2Bits;
                                }
                            }
                        }
                    }
                    /*process current pixel*/
                    if (coef)
                    {
                        vx_int8 ch;
                        coef>>=(wb->huffmanConfig[index].bit16Flag * 8);
                        if(coef<0x80)
                            coef+=wb->huffmanConfig[index].bit16Flag; /*Since we have to encode zero (run-length only handle 16-bits zero, 8 bit zero should use Size code*/
                        ch = (vx_int8)coef;
                        coef = abs(ch);
                        for (j = 6; j>=0; j--)
                        {
                            if (coef>=(1<<j))
                            {
                                break;
                            }
                        }
                        size = j+1;
                        if (invSizeOrder[size]==8)
                        { /*The minimum freq, we out the raw data*/
                            size = 7;
                        }

                        j = invSizeOrder[size];
                        kernelBitSize += 3;

                        if (sizeCodeLen[j] > 3)
                        {
                            kernelBitSize += 2;
                        }

                        if (size == 7)
                        {
                            size = 8;
                        }
                        /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                        if (size == 0)
                            size = 1;
                        if (sizeCodeLen[j]%2 == 0)
                        {
                            kernelBitSize += (size - 1);
                        }
                        else
                        {
                            kernelBitSize += size;
                        }
                        if (wb->huffmanConfig[index].bit16Flag)
                        {
                            kernelBitSize += 8;
                        }
                    }
                    else if (!isCoef || (dataRemainingOfCore != 0x0) || isLimitZRL)
                    {
                        /*those non-coef count like bias, num_of_vz & z_offset couldn't skip by zero-run as HW requres,
                        need also encode those 0 to huffman code,
                        in ZRL path 8-bit, 0 belong to group7, -128~127, residue bit length is 8, residue code = 0*/
                        size = 7;
                        j = invSizeOrder[size];
                        kernelBitSize += 3;

                        if (sizeCodeLen[j] > 3)
                        {
                            kernelBitSize += 2;
                        }

                        if (size == 7)
                        {
                            size = 8;
                        }

                        if (sizeCodeLen[j] % 2 == 0)
                        {
                            kernelBitSize += (size - 1);
                        }
                        else
                        {
                            kernelBitSize += size;
                        }
                        if (wb->huffmanConfig[index].bit16Flag)
                        {
                            kernelBitSize += 8;
                        }
                    }
                }
            }

            if(dataRemainingOfCore == 0)
            {
                /*Hit the end bitsteam of core, add dummy stage process the next*/
                kernelBitSize += 3;
                kernelBitSize += 2;
                kernelBitSize += 7;

                if (wb->huffmanConfig[index].bit16Flag)
                    kernelBitSize += 8;

                for (j = 0; j < 2*THROUGHPUT; j++)
                {
                    kernelBitSize += 3;
                }
                break;
            }
        }

        /* Go back to update kernelStreamSize. */
        kernelStreamSizePerCore = (kernelBitSize % 8 == 0) ? kernelBitSize / 8 : kernelBitSize / 8 + 1;
        /*align to 64 byte*/
        kernelStreamSizePerCore = (kernelStreamSizePerCore + 63) & 0xFFFFFFC0;
        kernelBitSize = 0;
        kernelSize += kernelStreamSizePerCore;
    }

    /*Add non compressed biases & Z_OFFSET & perFilterPostMul & perFilterPostShift after huffman bit stream*/
    kernelStreamSizePerCore = 0;
    for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
    {
        if (!hasNoBias)
        {
            kernelStreamSizePerCore += biasSize;
        }

        if (hasNNPerFilterPostMultiply)
        {
            kernelStreamSizePerCore += 4;
        }

        if (!hasNoZOffset)
        {
            kernelStreamSizePerCore += NN_Z_POSITION_OFFSET_BITS_VIP_V7 / 8;
        }
        else if (hasNNPreLU)
        {
            kernelStreamSizePerCore += 4;
        }
    }
    /*align to 64 byte*/
    kernelStreamSizePerCore = (kernelStreamSizePerCore + 63) & 0xFFFFFFC0;
    kernelSize += kernelStreamSizePerCore;

exit:
    if (reorderStream)
        vxFree(reorderStream);
    if (reorderStreamPerCoreCount)
        vxFree(reorderStreamPerCoreCount);
    if (nonCoefIndex)
        vxFree(nonCoefIndex);
    if (limitZRLIndex)
        vxFree(limitZRLIndex);

    return kernelSize;
}

void fillinKernelBufferV8Huffman(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_int32 input_zp,
    vx_int32 coef_zp,
    vx_uint32 skip_value,
    vx_uint32 output_final_x,
    vx_uint32 output_final_y,
    vx_int32 z_offset,
    vx_uint32 output_size,
    vx_uint8_ptr wb_base_ptr,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_uint32* post_mul,
    vx_uint32* post_shift,
    vx_tensor alpha,
    vx_uint32 index
    )
{

    vx_uint32 nnCoreCount = (weight_format == VX_TYPE_INT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : (weight_format == VX_TYPE_FLOAT16) ?
                      context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 invSizeOrder[MAX_SIZE_HISTO_COUNT] = {0};
    vx_int32 coef = 0, tmp = 0, prev = 0;

    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 usedCoreCount      = (filterTotalCount > nnCoreCount) ? nnCoreCount : filterTotalCount;
    vx_uint32 weightCount        = weight_x * weight_y;
    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize((vx_type_e)weight_format);
    vx_uint32 weightBitSize      = weightSize * 8;
    vx_uint32 biasBitSize        = (weight_format == VX_TYPE_INT16) ? NN_INTEGER_BIAS_BITS_VIP_V7_INT16 : NN_INTEGER_BIAS_BITS_VIP_V7;

    vx_uint32 filterSize         = weightCount * weightSize * slice_count;
    vx_uint32* kernelBufferPtr   = (vx_uint32*)wb_base_ptr;

    vx_uint32* kernelStreamSizePtr = VX_NULL;
    vx_uint32 kernelStreamSize = 0;
    vx_uint32 alignedOffset      = ((vx_uint32)(gctUINTPTR_T)kernelBufferPtr) & 0x3F;
    vx_uint8_ptr kernelDataPtr   = VX_NULL;
    vx_uint8_ptr kernelStreamBasePtr   = VX_NULL;
    vx_uint32 streamSizeBitOffset = 0;
    vx_uint32 maxKernelStreamSizePerCore = 0;

    vx_uint32 coreIndex;
    vx_uint32 i;

    vx_uint32 bit16Flag = (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8) ? 0 : 1;
    vx_uint32 bitOffset = 0;

    vx_uint32 reorderStreamSize = 0;
    vx_uint32 reorderStreamAllCount = 0;
    vx_uint32 reorderStreamCheckCount = 0;
    vx_uint32* reorderStreamPerCoreCount = VX_NULL;
    vx_uint32 filterIndex = 0;

    vx_uint8* reorderStream = VX_NULL;
    vx_uint32 runSet2Bits = 1;
    CodeSymbol codeSymbol[THROUGHPUT * 3];

    vx_bool hasNoBias = vx_false_e;
    vx_bool hasNoZOffset = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_NO_Z_LOCATION_OFFSET);
    vx_bool hasNNPerFilterPostMultiply = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PER_CHANNEL_POST_MULTIPLY);
    vx_bool hasNNPreLU = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PRELU);
    vx_bool hasZDP3 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3);
    vx_bool hasKernelFullCacheInterleaveFix = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_FULLCACHE_KERNEL_INTERLEAVE_FIX);
    vx_uint32 dpAmount = hasZDP3 ? 3 : 1;
    vx_uint32 zDPLoopCount = 3;
    vx_uint32 linesInImageBuffer = dpAmount * zDPLoopCount;
    vx_uint32 numOfInImageBuffer = (vx_uint32)gcoMATH_Ceiling((vx_float32)(weightCount * slice_count) / (vx_float32)linesInImageBuffer);

    vx_uint32 dummyStage[3] = {0};
    vx_uint32 dummyBitLength[3] = {0};
    vx_bool setDummy = vx_false_e;

    vx_uint32 nonCoefCount = 0;
    vx_uint32 totalCountIdx = 0;
    vx_uint32 nonCoefCountIdx = 0;
    vx_uint32* nonCoefIndex = VX_NULL;

    vx_int32* coefSum = VX_NULL;
    vx_uint32* realVzIndex = VX_NULL;
    vx_uint32* limitZRLIndex = VX_NULL;
    vx_uint32 limitZRLCountIdx = 0;
    /*prelu parameter alpha*/
    gctPOINTER alphaBase = VX_NULL;
    vx_int32    alphaZP  = 0;
    vx_int8     alphaFP  = 0;
    vx_float32  alphaScale = 0;
    vx_type_e   alphaFormat = VX_TYPE_FLOAT32;
    vx_enum     alphaQuantFormat = 0;

    typedef struct _gcVXWeightsMinusZP
    {
        vx_int32 sum;
        vx_uint32 filterIdx;
    }gcVXWeightsMinusZP;
    gcVXWeightsMinusZP *weightsMinusZP = VX_NULL;

    if (alpha != VX_NULL)
    {
        vxoTensor_GetTensorViewMemory(alpha, &alphaBase, VX_NULL);
        alphaZP = alpha->zeroPoint;
        alphaScale = alpha->scale;
        alphaFP = alpha->fixedPointPos;
        alphaFormat = (vx_type_e)alpha->tensorBuffer->dataFormat;
        alphaQuantFormat = alpha->quantFormat;
    }

    if (weightBitSize == 0)
    {
        vxError("%s: weightBitSize should not be zero.", __FUNCTION__);
        goto exit;
    }

    if (weight_format == VX_TYPE_INT16 || weight_format == VX_TYPE_UINT8)
    {
        vx_uint32 sliceIndex, weightXIndex, weightYIndex;
        vx_uint32 filterSliceSize = weight_x * weight_y * weightSize;

        coefSum = (vx_int32*)vxAllocateAndZeroMemory(filterTotalCount * sizeof(vx_int32));
        if (!coefSum)
        {
            vxError("fillinKernelBufferV8Huffman: OUT OF MEMORY");
            goto exit;
        }

        /*Calc sum((coef[i] - coefZP) * InZP) */
        if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT) &&
            input_zp != 0 &&
            wb->wb_base->weights_quant_format == VX_QUANT_AFFINE_SCALE &&
            weight_format == VX_TYPE_UINT8)
        {
            weightsMinusZP = (gcVXWeightsMinusZP*)vxAllocateAndZeroMemory(filterTotalCount * sizeof(gcVXWeightsMinusZP));

            if (!weightsMinusZP)
            {
                vxError("fillinKernelBufferV8Huffman: OUT OF MEMORY");
                goto exit;
            }
        }

        for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
        {
            if (weightsMinusZP != VX_NULL)
            {
                weightsMinusZP[filterIndex].filterIdx = filterIndex;
                weightsMinusZP[filterIndex].sum = 0;
            }
            for (sliceIndex = 0; sliceIndex < slice_count; sliceIndex++)
            {
                /* add one slice data every filter */
                kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * sliceIndex;

                for (weightYIndex = 0; weightYIndex < weight_y; weightYIndex++)
                {
                    for (weightXIndex = 0; weightXIndex < weight_x; weightXIndex++)
                    {
                        vx_int32 weight = 0;
                        if (weight_format == VX_TYPE_UINT8)
                        {
                            weight = *((vx_uint8 *)kernelDataPtr);
                            coefSum[filterIndex] += (weight - skip_value);
                        }
                        else
                        {
                            weight = *((vx_int16 *)kernelDataPtr);
                            coefSum[filterIndex] += weight;
                        }
                        kernelDataPtr = kernelDataPtr + weightSize;

                        if (weightsMinusZP != VX_NULL)
                        {
                            /*Calc sum((coef[i] - coefZP) * InZP) */
                            weightsMinusZP[filterIndex].sum += (weight - coef_zp) * input_zp;
                        }
                    }
                }
            }
        }
    }

    if (weight_format == VX_TYPE_BFLOAT16)
    {
        numOfInImageBuffer = (vx_uint32)gcoMATH_Ceiling((vx_float32)(weightCount * slice_count + 3) / (vx_float32)linesInImageBuffer);
    }
    reorderStreamAllCount = (numOfInImageBuffer * linesInImageBuffer * filterTotalCount)
        + (usedCoreCount * (16 / weightBitSize));/*filterPerCore need 16 bit * nnCoreCount */

    reorderStreamSize = reorderStreamAllCount * weightSize;

    if (hasNNPerFilterPostMultiply)
        reorderStreamSize += filterTotalCount; /*Per channel 8 bit zero point*/

    reorderStream = (vx_uint8 *)vxAllocateAndZeroMemory(reorderStreamSize);

    if (!reorderStream)
    {
        vxError("fillinKernelBufferV8Huffman: OUT OF MEMORY");
        goto exit;
    }

    reorderStreamPerCoreCount = (vx_uint32 *)vxAllocateAndZeroMemory(nnCoreCount * sizeof(vx_uint32));
    if (!reorderStreamPerCoreCount)
    {
        vxError("fillinKernelBufferV8Huffman: OUT OF MEMORY");
        goto exit;
    }

    nonCoefCount = usedCoreCount * (16 / weightBitSize);
    if (hasNNPerFilterPostMultiply)
        nonCoefCount += filterTotalCount;
    if (weight_format == VX_TYPE_BFLOAT16)
        nonCoefCount += filterTotalCount * 3; /*BFLOAT16 fp32 bias will be splitted to 3 BF16 to coef stream*/

    nonCoefIndex = (vx_uint32 *)vxAllocateAndZeroMemory(nonCoefCount * sizeof(vx_uint32));
    if (!nonCoefIndex)
    {
        vxError("fillinKernelBufferV8Huffman: OUT OF MEMORY");
        goto exit;
    }

    limitZRLIndex = (vx_uint32 *)vxAllocateAndZeroMemory(filterTotalCount * sizeof(vx_uint32));
    if (!limitZRLIndex)
    {
        vxError("fillinKernelBufferHuffman: OUT OF MEMORY");
        goto exit;
    }

    if (wb->wb_base->hw_depth_wise)
        reorderDepthWiseKernelBufferV8Huffman(context, wb, slice_count, z_count, filters_per_core, skip_value, weight_format, reorderStream, weight_base_ptr, bias_base_ptr, nnCoreCount, reorderStreamPerCoreCount, nonCoefIndex, limitZRLIndex);
    else if (context->options.enableNonZeroBalance && !hasNoZOffset)
    {
        /*Only those chip has z_offset support zero balance*/
        realVzIndex = (vx_uint32 *)vxAllocateAndZeroMemory(z_count * sizeof(vx_uint32));
        if (!realVzIndex)
        {
            vxError("fillinKernelBufferV8Huffman: OUT OF MEMORY");
            goto exit;
        }
        reorderKernelBufferV8HuffmanBalance(context, wb, slice_count, z_count, filters_per_core, skip_value, weight_format, reorderStream, weight_base_ptr, nnCoreCount, reorderStreamPerCoreCount, nonCoefIndex, realVzIndex, limitZRLIndex);
    }
    else
        reorderKernelBufferV8Huffman(context, wb, slice_count, z_count, filters_per_core, skip_value, weight_format, reorderStream, weight_base_ptr, bias_base_ptr, nnCoreCount, reorderStreamPerCoreCount, nonCoefIndex, limitZRLIndex);

    analysisKernelStreamForHuffman(context, wb, 0, nnCoreCount, reorderStream, reorderStreamSize, reorderStreamPerCoreCount, invSizeOrder, nonCoefIndex, nonCoefCount, limitZRLIndex, filterTotalCount, index);

    /* Write huffman header*/
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].preEncode, 1);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].bit16Flag, 1);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].fp16Flag, 1);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].reserved, 1);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].version, 4);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].runLenTableSize, 8);

    for(i = 0; i < MAX_RUNLEN_SIZE; i++)
    {
        writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].runLenTable[i], 8);
    }
    for(i = 0; i < LOG_RUN_SIZE1; i++)
    {
        writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].mapToHuffman[i], 8);
    }
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].avgBias, 16);
    writeBits(&kernelBufferPtr, &bitOffset, 0, 16); /*reserved, must zero*/

    kernelStreamSizePtr = kernelBufferPtr;
    streamSizeBitOffset = bitOffset;
    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        writeBits(&kernelBufferPtr, &bitOffset, kernelStreamSize, 32);
    }
    /*align to 64 byte*/
    packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);
    wb->slice_array[index].kernel_stream_full_cache_size = 0;

    for (i = 0; i < THROUGHPUT * 3; i++)
    {
        codeSymbol[i].stageCode[0] = codeSymbol[i].stageCode[1] = codeSymbol[i].stageCode[2] = 0;
        codeSymbol[i].bitLength[0] = codeSymbol[i].bitLength[1] = codeSymbol[i].bitLength[2] = 0;
    }

    while(((numBestRuns-SET_1_SIZE)>>runSet2Bits) > 1)
    {
        runSet2Bits++;
    }

    kernelDataPtr = reorderStream;
    wb->max_per_core_compression_ratio = 0;

    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {

        vx_uint32 x = 0x0;
        vx_int32 pos = 0x0;
        vx_int32 run = 0x0;
        vx_uint32 dataSizeOfCore = reorderStreamPerCoreCount[coreIndex];
        vx_uint32 dataRemainingOfCore = dataSizeOfCore;
        vx_int32 size = 0;
        vx_int32 code = 0;
        vx_int32 coef16 = 0;
        vx_int32 k = 0, j = 0, m = 0;
        vx_bool isCoef;
        vx_bool isLimitZRL;
        vx_float64 compressionRatio = 0;
        /* update position of each core's beginning of compressed kernel*/
        kernelStreamBasePtr = (vx_uint8*)kernelBufferPtr;
        reorderStreamCheckCount += reorderStreamPerCoreCount[coreIndex];

        memset(codeSymbol, 0, sizeof(CodeSymbol));
        memset(dummyStage, 0, (sizeof(vx_uint32) * 3));
        memset(dummyBitLength, 0, (sizeof(vx_uint32) * 3));
        setDummy = vx_false_e;

        while(dataRemainingOfCore --)
        {

            pos = (x) % (3*THROUGHPUT);

            if (weight_format == VX_TYPE_INT8)
                coef = *((vx_int8 *)kernelDataPtr);
            else if (weight_format == VX_TYPE_UINT8)
                coef = *((vx_uint8 *)kernelDataPtr);
            else if (weight_format == VX_TYPE_INT16 ||
                weight_format == VX_TYPE_FLOAT16 ||
                weight_format == VX_TYPE_BFLOAT16
                )
                coef16 = *((vx_int16 *)kernelDataPtr);
            else
            {
                /*Other format not suppoted now*/
                vxmASSERT(0);
            }
            kernelDataPtr += weightSize;

            if (totalCountIdx == nonCoefIndex[nonCoefCountIdx] &&
                nonCoefCountIdx < nonCoefCount)
            {
                isCoef = vx_false_e;
                nonCoefCountIdx++;
            }
            else
                isCoef = vx_true_e;

            if (limitZRLIndex != VX_NULL &&
                totalCountIdx == limitZRLIndex[limitZRLCountIdx] &&
                limitZRLCountIdx < filterTotalCount)
            {
                isLimitZRL = vx_true_e;
                limitZRLCountIdx++;
            }
            else
                isLimitZRL = vx_false_e;

            totalCountIdx++;

            if (bit16Flag == 0)
            {

                coef = (coef - wb->huffmanConfig[index].avgBias) & 0xFF;
                if (wb->huffmanConfig[index].preEncode)
                {
                    tmp = coef;
                    coef = (coef - prev) & 0xFF;
                    prev = tmp;
                }
            }
            else
            {

                if (wb->huffmanConfig[index].fp16Flag)
                {
                    coef16 = (coef16 & 0x7FFF) * 2 + coef16 / (1<<15);
                }
                coef16 -= wb->huffmanConfig[index].avgBias;
                if (wb->huffmanConfig[index].runLenTableSize == 0x0)
                    coef = (coef16 >> 8) & 0xFF; /*Non RZL*/
                else
                    coef = coef16 & 0xFFFF; /*RZL path*/
            }

            if (wb->huffmanConfig[index].runLenTableSize == 0x0) /* non RZL*/
            {
                if (!setDummy)
                {
                    /*Set dummy all to 0,
                    in nonZRL path 8-bit, 0 belong to group0, -1~0, residue bit length is 1, residue code = 0*/
                    size = 0;
                    k = invSizeOrder[size];

                    code = huffCode[k];

                    dummyStage[0] = code & 0x07;
                    dummyBitLength[0] = 3;

                    if (sizeCodeLen[k] > 3)
                    {
                        dummyStage[1] = code >> 3;
                        dummyBitLength[1] = 2;
                    }

                    dummyStage[2] = 0;
                    if (size == 0) size = 1; /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                    if (sizeCodeLen[k]%2 == 0)
                    {
                        dummyBitLength[2] = size - 1;
                    }
                    else
                    {
                        dummyBitLength[2] = size;
                    }

                    if (wb->huffmanConfig[index].bit16Flag)
                    {/*pack the raw data of the lower 8 bits*/

                        dummyBitLength[2] += 8;
                    }
                    setDummy = vx_true_e;
                }

                tmp = (coef & 0x80)? (coef ^ 0xff): coef;
                for (size = 0; size<7; size++)
                {
                    if (tmp<(1<<size))
                        break;
                }
                k = invSizeOrder[size];
                code = huffCode[k];

                if (sizeCodeLen[k]%2 == 0)/*huffCode length = 2 or 4, pack 1 bit to 3 or 5*/
                    code |= ((coef>>7)<<(sizeCodeLen[k]) );
                codeSymbol[pos].stageCode[0] = code & 0x07;
                codeSymbol[pos].bitLength[0] = 3;

                if (sizeCodeLen[k] > 3)
                {
                    codeSymbol[pos].stageCode[1] = code >> 3;
                    codeSymbol[pos].bitLength[1] = 2;
                }

                codeSymbol[pos].stageCode[2] = tmp &((1<<size) - 1 );
                if (size == 0) size = 1; /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                if (sizeCodeLen[k]%2 == 0)
                {
                    codeSymbol[pos].bitLength[2] = size - 1;
                }
                else
                {
                    codeSymbol[pos].stageCode[2] <<= 1;
                    codeSymbol[pos].stageCode[2] |= (coef>>7); /*The sign bit*/
                    codeSymbol[pos].bitLength[2] = size;
                }

                if (wb->huffmanConfig[index].bit16Flag)
                {/*pack the raw data of the lower 8 bits*/

                    codeSymbol[pos].stageCode[2] = codeSymbol[x%(3*THROUGHPUT)].stageCode[2]* 256 + (coef16 &0xff);
                    codeSymbol[pos].bitLength[2] += 8;
                }
                OutputAt(x, &kernelBufferPtr, &bitOffset, codeSymbol);
                x++; /*counter*/
            }
            else /* RZL enable*/
            {
                if (!setDummy)
                {
                    /*Set dummy all to 0,
                    in ZRL path 8-bit, 0 belong to group7, -128~127, residue bit length is 8, residue code = 0*/
                    size = 7;
                    j = invSizeOrder[size];
                    code = huffCode[j];

                    dummyStage[0] = code & 0x07;
                    dummyBitLength[0] = 3;

                    if (sizeCodeLen[j] > 3)
                    {
                        dummyStage[1] = code >> 3;
                        dummyBitLength[1] = 2;
                    }

                    if (size == 7)
                    {
                        size = 8;
                    }

                    if (sizeCodeLen[j] % 2 == 0)
                    {
                        dummyStage[2] = 0;  /*last bit was coded*/
                        dummyBitLength[2] = size - 1;
                    }
                    else
                    {
                        dummyStage[2] = 0;
                        dummyBitLength[2] = size;
                    }
                    if (wb->huffmanConfig[index].bit16Flag)
                    {
                        dummyStage[2] = 0;
                        dummyBitLength[2] += 8;
                    }

                    setDummy = vx_true_e;
                }

                if (coef == 0 && isCoef && !isLimitZRL)
                {
                    run++;
                    if (dataRemainingOfCore != 0)
                    {
                        continue;
                    }
                }
                if (coef || (dataRemainingOfCore == 0) || !isCoef || isLimitZRL) /* if last pixel, force process run zeros*/
                {
                    /* process run zeros*/
                    while(run)
                    {
                        for (j = numBestRuns - 1; j>=0; j--)
                        {
                            if (run >= bestRunsSorted[j] + 1)
                            {
                                k = run / (bestRunsSorted[j] + 1);
                                run %= (bestRunsSorted[j] + 1);
                                break;
                            }
                        }
                        if (j >= 0)
                        {
                            vx_int32 n = bestRunsSorted[j];
                            size = 8;
                            for (m = 0; m<SET_1_SIZE; m++)
                            {
                                if (best2Runs[m] == n)
                                {
                                    size = 0;
                                    break;
                                }
                            }
                            if (size == 8)
                            {
                                for (m = 0; m<numBestRuns - SET_1_SIZE; m++)
                                    if (outBest2Run[m] == n)
                                        break;

                            }
                            j = invSizeOrder[size];
                            while(k--){/*k of the runs*/
                                code = huffCode[j];
                                if (sizeCodeLen[j] % 2 == 0)/*huffCode length = 2 or 4, pack 1 bit to 3 or 5*/
                                    code |= ((m&1)<<(sizeCodeLen[j]) );
                                codeSymbol[pos].stageCode[0] = code & 0x07;
                                codeSymbol[pos].bitLength[0] = 3;

                                if (sizeCodeLen[j] > 3)
                                {
                                    codeSymbol[pos].stageCode[1] = code >> 3;
                                    codeSymbol[pos].bitLength[1] = 2;
                                }
                                if (size == 0 )
                                {/*The m already code*/
                                    codeSymbol[pos].bitLength[2] = (sizeCodeLen[j]%2 == 0) ? 0:1;
                                    codeSymbol[pos].stageCode[2] = m; /*If (sizeCodeLen[j]%2 == 0), should be m/2, but 0 bit output, doesn't matter*/
                                }
                                else if (sizeCodeLen[j]%2 == 0)
                                {
                                    codeSymbol[pos].stageCode[2] = m/2;
                                    codeSymbol[pos].bitLength[2] = runSet2Bits - 1;
                                }
                                else
                                {
                                    codeSymbol[pos].stageCode[2] = m;
                                    codeSymbol[pos].bitLength[2] = runSet2Bits;
                                }

                                OutputAt(x, &kernelBufferPtr, &bitOffset, codeSymbol);
                                x++;
                                /* update pos*/
                                pos = (x) % (3*THROUGHPUT);
                            }
                        }
                    }
                    /*process current pixel*/
                    if (coef)
                    {
                        vx_int8 ch;
                        coef>>=(wb->huffmanConfig[index].bit16Flag * 8);
                        if(coef<0x80)
                            coef+=wb->huffmanConfig[index].bit16Flag; /*Since we have to encode zero (run-length only handle 16-bits zero, 8 bit zero should use Size code*/
                        ch = (vx_int8)coef;
                        coef = abs(ch);
                        m = coef*2 + (ch<0?1:0);
                        for (j = 6; j>=0; j--)
                        {
                            if (coef>=(1<<j))
                            {
                                break;
                            }
                        }
                        size = j+1;
                        if (invSizeOrder[size]==8)
                        { /*The minimum freq, we out the raw data*/
                            size = 7;
                        }
                        if (size == 7)
                        {
                            m = ch&0xff;
                            if(wb->huffmanConfig[index].bit16Flag)
                                m = (coef16 >> 8) & 0xff;
                        }

                        j = invSizeOrder[size];
                        code = huffCode[j];
                        if (sizeCodeLen[j]%2 == 0)/*huffCode length = 2 or 4, pack 1 bit to 3 or 5*/
                            code |= ((m&1) << (sizeCodeLen[j]));
                        codeSymbol[pos].stageCode[0] = code & 0x07;
                        codeSymbol[pos].bitLength[0] = 3;

                        if (sizeCodeLen[j] > 3)
                        {
                            codeSymbol[pos].stageCode[1] = code >> 3;
                            codeSymbol[pos].bitLength[1] = 2;
                        }

                        if (size == 7)
                        {
                            size = 8;
                        }
                        /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                        if (size == 0)
                            size = 1;
                        if (sizeCodeLen[j]%2 == 0)
                        {
                            codeSymbol[pos].stageCode[2] = m/2;  /*last bit was coded*/
                            codeSymbol[pos].bitLength[2] = size - 1;
                        }
                        else
                        {
                            codeSymbol[pos].stageCode[2] = m;
                            codeSymbol[pos].bitLength[2] = size;
                        }
                        if (wb->huffmanConfig[index].bit16Flag)
                        {
                            codeSymbol[pos].stageCode[2] = codeSymbol[x%(3*THROUGHPUT)].stageCode[2]* 256 + (coef16 & 0xff);
                            codeSymbol[pos].bitLength[2] += 8;
                        }
                        OutputAt(x, &kernelBufferPtr, &bitOffset, codeSymbol);
                        x++; /*counter*/
                    }
                    else if (!isCoef || (dataRemainingOfCore != 0x0) || isLimitZRL)
                    {
                        /*those non-coef count like bias, num_of_vz & z_offset couldn't skip by zero-run as HW requres,
                        need also encode those 0 to huffman code,
                        in ZRL path 8-bit, 0 belong to group7, -128~127, residue bit length is 8, residue code = 0*/
                        size = 7;
                        j = invSizeOrder[size];
                        code = huffCode[j];

                        codeSymbol[pos].stageCode[0] = code & 0x07;
                        codeSymbol[pos].bitLength[0] = 3;

                        if (sizeCodeLen[j] > 3)
                        {
                            codeSymbol[pos].stageCode[1] = code >> 3;
                            codeSymbol[pos].bitLength[1] = 2;
                        }

                        if (size == 7)
                        {
                            size = 8;
                        }

                        if (sizeCodeLen[j] % 2 == 0)
                        {
                            codeSymbol[pos].stageCode[2] = 0;  /*last bit was coded*/
                            codeSymbol[pos].bitLength[2] = size - 1;
                        }
                        else
                        {
                            codeSymbol[pos].stageCode[2] = 0;
                            codeSymbol[pos].bitLength[2] = size;
                        }
                        if (wb->huffmanConfig[index].bit16Flag)
                        {
                            codeSymbol[pos].stageCode[2] = codeSymbol[x%(3*THROUGHPUT)].stageCode[2]* 256 + (coef16 & 0xff);
                            codeSymbol[pos].bitLength[2] += 8;
                        }
                        OutputAt(x, &kernelBufferPtr, &bitOffset, codeSymbol);
                        x++; /*counter*/
                    }
                }
            }

            if(dataRemainingOfCore == 0)
            {
                vx_uint32 dummyCount = (x & 0x1)? 5: 4;
                /*Hit the end bitsteam of core, add dummy stage process the next*/
                vxmASSERT(setDummy == vx_true_e);
                addDummy(x, &kernelBufferPtr, &bitOffset, codeSymbol, dummyCount, dummyStage, dummyBitLength);
                break;
            }
        }

        /* Go back to update kernelStreamSize. */
        kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);
        writeBits(&kernelStreamSizePtr, &streamSizeBitOffset, kernelStreamSize*8 + bitOffset, 32);

        /* pad 0 at the end of core*/
        packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);
        kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);
        /*With RTL bug fix, full cache mode size needn't using maxSize * coreCount*/
        wb->slice_array[index].kernel_stream_full_cache_size += kernelStreamSize;

        if (maxKernelStreamSizePerCore < kernelStreamSize)
            maxKernelStreamSizePerCore = kernelStreamSize;

        compressionRatio = (vx_float64)kernelStreamSize / (vx_float64)(dataSizeOfCore * weightSize);
        if (wb->max_per_core_compression_ratio < compressionRatio)
            wb->max_per_core_compression_ratio = compressionRatio;

        prev = 0;
    }
    vxmASSERT(reorderStreamAllCount == reorderStreamCheckCount); /*Check if the data count is the same*/

    /*Add non compressed biases & Z_OFFSET & perFilterPostMul & perFilterPostShift after huffman bit stream*/
    kernelStreamBasePtr = (vx_uint8*)kernelBufferPtr;
    for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
    {

        if (!hasNoBias)
        {
            vx_uint32 biasData = 0;
            vx_int64 bias64 = 0;
            vx_uint32 realFilterIndex;

            if (context->options.enableNonZeroBalance &&
                !hasNoZOffset &&
                !wb->wb_base->hw_depth_wise &&
                realVzIndex != VX_NULL)
                realFilterIndex = realVzIndex[filterIndex];
            else
                realFilterIndex = filterIndex;

            if (bias_base_ptr)
            {
                if(bias_format == VX_TYPE_INT64)
                    bias64 = *(((vx_int64 *)bias_base_ptr) + filterIndex);
                else
                    biasData = *(bias_base_ptr + realFilterIndex);
            }
            else
                biasData = 0;

            /* NewBias = sum((Coef[i] - CoefZP) * - InZP) + Bias*/
            if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT)
                && input_zp != 0
                && weight_format == VX_TYPE_UINT8)
            {
                biasData -= weightsMinusZP[realFilterIndex].sum;
            }

            if (weight_format == VX_TYPE_UINT8)
                biasData += coefSum[realFilterIndex] * 128;

            if (weight_format == VX_TYPE_INT16)
            {
                vx_int64 sum64 = 0;
                vx_int32 bias32;
                vx_int16 bias16;

                if (bias_format != VX_TYPE_INT64)
                {
                    if ((vx_int32)biasData < 0)
                    {
                        bias64 = ~0;
                        bias64 = (bias64 >> 32) << 32;
                        bias64 = bias64 | (vx_int64)biasData;
                    }
                    else
                        bias64 = (vx_int64)biasData;
                }

                if (coefSum[realFilterIndex] < 0)
                {
                    sum64 = ~0;
                    sum64 = (sum64 >> 32) << 32;
                    sum64 = sum64 | (vx_int64)coefSum[realFilterIndex];
                }
                else
                    sum64 = (vx_int64)coefSum[realFilterIndex];

                bias64 += sum64 * 128;
                bias32 = (vx_int32)bias64;
                writeBits(&kernelBufferPtr, &bitOffset, bias32, 32);
                bias16 = (vx_int16)(bias64 >> 32);
                writeBits(&kernelBufferPtr, &bitOffset, (vx_int32)bias16, 16);
            }
            else
                writeBits(&kernelBufferPtr, &bitOffset, biasData, biasBitSize);
        }

        if (hasNNPerFilterPostMultiply)
        {
            vx_uint32 postMul = 0;
            vx_uint32 postShift = 0;
            vxmASSERT(post_mul != VX_NULL && post_shift != VX_NULL);

            postMul = *(post_mul + filterIndex);
            postShift = *(post_shift + filterIndex);

            writeBits(&kernelBufferPtr, &bitOffset, postMul, 23);
            writeBits(&kernelBufferPtr, &bitOffset, postShift, 7);
            /*unused zero for 10 bit*/
            writeBits(&kernelBufferPtr, &bitOffset, 0, 2);
        }

        if (!hasNoZOffset)
        {
            vx_uint32 offsetValue;
            vx_uint32 realFilterIndex;

            if (context->options.enableNonZeroBalance &&
                !wb->wb_base->hw_depth_wise &&
                realVzIndex != VX_NULL)
                realFilterIndex = realVzIndex[filterIndex];
            else
                realFilterIndex = filterIndex;

            if (z_offset > 0)
                offsetValue = (vx_uint32)z_offset * realFilterIndex;
            else if (output_size > 0)
                offsetValue = output_final_x * output_final_y * output_size * realFilterIndex;
            else
                offsetValue = output_final_x * output_final_y * weightSize * realFilterIndex;

            writeBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS_VIP_V7);
        }
        else if (hasNNPerFilterPostMultiply && hasNNPreLU)
        {
            /*per PRD, prelu only enable when hasNoZOffset == 1 && hasNNPerFilterPostMultiply == 1*/
            vx_int32 negPostMul = 0;
            vx_int32 negPostShift = 0;
            vx_float32 alphaValue = 0;

            if (alphaBase != VX_NULL)
            {
                vx_uint32 uintAlpha;
                vx_int32 exp;

                alphaValue = vxnneGetDataExt(alphaFormat, alphaQuantFormat, filterIndex, (vx_uint8_ptr)alphaBase, alphaFP, alphaZP, alphaScale);

                uintAlpha = *((vx_uint32*)(&alphaValue));
                exp = (uintAlpha & 0x7F800000) >> 23;

                negPostShift = 127 - exp;
                negPostMul = (uintAlpha & 0x7FFFFF) >> 8; /* negMultiply only has 15 bit, using high 15-bit of alpha's mantissa*/
            }

            writeBits(&kernelBufferPtr, &bitOffset, negPostMul, 23);
            writeBits(&kernelBufferPtr, &bitOffset, negPostShift, 7);
            /*unused zero for 10 bit*/
            writeBits(&kernelBufferPtr, &bitOffset, 0, 2);
        }
    }
    /*Also align to 64 byte for post processing stream*/
    packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);
    kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);
    wb->slice_array[index].kernel_stream_full_cache_size += kernelStreamSize;

    if (maxKernelStreamSizePerCore < kernelStreamSize)
        maxKernelStreamSizePerCore = kernelStreamSize;
    /*Per HW, post processing stream size is needed in SRAM, when partial mode, need be noted as one core*/

        /*bug1980*/
    if(gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_IMAGE_NOT_PACKED_IN_SRAM_FIX))
        wb->slice_array[index].kernel_align_stream_size = (vx_size)(maxKernelStreamSizePerCore * (usedCoreCount + 1));
    else
        wb->slice_array[index].kernel_align_stream_size = (vx_size)(maxKernelStreamSizePerCore * (nnCoreCount + 1));

    if (!hasKernelFullCacheInterleaveFix)
    {
        /*if did't fix full cache interleave fix, full cache size also need use align size*/
        wb->slice_array[index].kernel_stream_full_cache_size = wb->slice_array[index].kernel_align_stream_size;
    }

    wb->slice_array[index].kernel_max_stream_size_percore = maxKernelStreamSizePerCore;

exit:
    if (coefSum)
        vxFree(coefSum);
    if (weightsMinusZP)
        vxFree(weightsMinusZP);
    if (reorderStream)
        vxFree(reorderStream);
    if (reorderStreamPerCoreCount)
        vxFree(reorderStreamPerCoreCount);
    if (nonCoefIndex)
        vxFree(nonCoefIndex);
    if (realVzIndex)
        vxFree(realVzIndex);
    if (limitZRLIndex)
        vxFree(limitZRLIndex);

    return;
}



void reorderKernelBufferV8MergeDW1x1(
    vx_context context,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 slice_count_dw,
    vx_uint32 z_count_dw,
    vx_uint32 slice_count_1x1,
    vx_uint32 z_count_1x1,
    vx_uint32 filters_per_core,
    vx_uint32 skip_value_dw,
    vx_uint32 skip_value_1x1,
    vx_enum  weight_format,
    vx_uint8_ptr reorder_stream,
    vx_uint8_ptr weight_base_ptr_dw,
    vx_uint8_ptr weight_base_ptr_1x1,
    vx_uint32 nn_core_count_1x1,
    vx_uint32* reoder_stream_count_per_core,
    vx_uint32* non_coef_index,
    vx_uint32* limit_zrl_Index
    )
{
    vx_uint32 nnCoreCount1x1        = nn_core_count_1x1;
    vx_uint32 filterTotalCount1x1   = z_count_1x1;
    vx_uint32 totalFilterPerCore1x1 = filterTotalCount1x1 / nnCoreCount1x1;
    vx_uint32 oddFilterPerCore1x1   = filterTotalCount1x1 % nnCoreCount1x1;
    vx_uint32 filterCount           = filters_per_core; /* filter count every group */
    vx_uint32 batchSize             = nnCoreCount1x1 * filterCount;
    vx_uint32 groupCount            = (filterTotalCount1x1 + batchSize - 1) / batchSize;

    vx_uint32 weightCountDW         = weight_x * weight_y;
    vx_uint32 weightX1x1            = 1;
    vx_uint32 weightY1x1            = 1;
    vx_uint32 weightCount1x1        = weightX1x1 * weightY1x1;
    vx_uint32 weightSize            = (vx_uint32)vxDataType_GetSize((vx_type_e)weight_format);
    vx_uint32 weightBitSize         = weightSize * 8;

    vx_uint32 filterSliceSizeDW     = weightCountDW * weightSize;
    vx_uint32 filterSizeDW          = weightCountDW * weightSize * slice_count_dw;
    vx_uint32 filterSliceSize1x1    = weightCount1x1 * weightSize;
    vx_uint32 filterSize1x1         = weightCount1x1 * weightSize * slice_count_1x1;
    vx_uint8* kernelBufferInt8Ptr   = reorder_stream;
    vx_uint16* kernelBufferInt16Ptr = (vx_uint16*)reorder_stream;

    vx_uint32 coreIndex;
    vx_uint32 limitIndex = 0;
    vx_uint32 filterIndex;
    vx_uint32 k, sk;
    vx_uint32 kSize;
    vx_uint32 weightXIndex, weightYIndex, weightZIndex;

    vx_bool hasZDP3 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3);
    vx_uint32 dpAmount = hasZDP3 ? 3 : 1;
    vx_uint32 zDPLoopCount = 3;
    vx_uint32 linesInImageBuffer = dpAmount * zDPLoopCount;

    vx_uint32 nonCoefIndex = 0, idx = 0, elementIndex = 0;

    if (weightBitSize == 0)
    {
        vxError("%s: weightBitSize should not be zero.", __FUNCTION__);
        return;
    }

    for (coreIndex = 0; coreIndex < nnCoreCount1x1 + 1; coreIndex++)
    {
        if (coreIndex == 0)
        {
            /* depth-wise part */
            vx_uint32 coef = 0;
            vx_uint8_ptr realKernelDataPtr = VX_NULL;
            reoder_stream_count_per_core[coreIndex] = 0;

            /* Save kernels per core*/
            if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
            {
                *((vx_uint16 *)kernelBufferInt8Ptr) = (vx_uint16)z_count_dw;
                kernelBufferInt8Ptr += 2;
            }
            else
            {
                *kernelBufferInt16Ptr = (vx_uint16)z_count_dw;
                kernelBufferInt16Ptr ++;
            }
            reoder_stream_count_per_core[coreIndex] += (16 / weightBitSize);

            for (idx = 0; idx < (16 / weightBitSize); idx++)
            {
                non_coef_index[nonCoefIndex++] = elementIndex++;
            }

            for (filterIndex = 0; filterIndex < z_count_dw; filterIndex++)
            {
                for (k = 0; k < weightCountDW * slice_count_dw; k += linesInImageBuffer)
                {
                    kSize = gcmMIN((weightCountDW * slice_count_dw - k), linesInImageBuffer);

                    for (sk = 0; sk < linesInImageBuffer; sk++)
                    {
                        weightZIndex = (k + sk) / weightCountDW;
                        weightYIndex = (k + sk) % weightCountDW;
                        weightYIndex /= weight_x;
                        weightXIndex = weight_x;
                        /*Packl 0 in the last dp group*/
                        if (sk >= kSize)
                            coef = skip_value_dw;
                        else
                        {
                            realKernelDataPtr = weight_base_ptr_dw + filterIndex * filterSizeDW +
                                filterSliceSizeDW * weightZIndex +
                                (weightYIndex * weight_x + weightXIndex) * weightSize;

                            if (weight_format == VX_TYPE_INT8)
                                coef = *((vx_int8 *)realKernelDataPtr);
                            else if (weight_format == VX_TYPE_UINT8)
                                coef = *((vx_uint8 *)realKernelDataPtr);
                            else
                                coef = *((vx_uint16 *)realKernelDataPtr);
                        }
                        if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
                        {
                            vx_int8 newCoef = ((vx_int32)coef - (vx_int32)skip_value_dw) & 0xFF;
                            *kernelBufferInt8Ptr = (vx_uint8)newCoef;
                            kernelBufferInt8Ptr++;
                        }
                        else
                        {
                            if (weight_format == VX_TYPE_FLOAT16)
                                coef = (coef & 0x7fff) * 2 + coef/(1<<15);

                            *kernelBufferInt16Ptr = (vx_uint16)coef;
                            kernelBufferInt16Ptr++;
                        }
                        reoder_stream_count_per_core[coreIndex]++;
                        if (weightXIndex == 0 && weightYIndex == 0 && weightZIndex == 0)
                            limit_zrl_Index[limitIndex++] = elementIndex;
                        elementIndex++;
                    }
                }
            }
        }
        else
        {
            /* 1x1 part*/
            vx_uint32 coreFilterCount = ((coreIndex - 1) < oddFilterPerCore1x1) ? totalFilterPerCore1x1 + 1 : totalFilterPerCore1x1;
            vx_uint32 groupIndex;
            vx_uint32 coef = 0;
            vx_uint8_ptr realKernelDataPtr = VX_NULL;
            reoder_stream_count_per_core[coreIndex] = 0;

            if (coreFilterCount > 0)
            {
                /* Save kernels per core*/
                if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
                {
                    *((vx_uint16 *)kernelBufferInt8Ptr) = (vx_uint16)coreFilterCount;
                    kernelBufferInt8Ptr += 2;
                }
                else
                {
                    *kernelBufferInt16Ptr = (vx_uint16)coreFilterCount;
                    kernelBufferInt16Ptr ++;
                }
                reoder_stream_count_per_core[coreIndex] += (16 / weightBitSize);

                for (idx = 0; idx < (16 / weightBitSize); idx++)
                {
                    non_coef_index[nonCoefIndex++] = elementIndex++;
                }
            }

            for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
            {
                vx_uint32 actualFilterCount = (groupIndex == groupCount - 1) ? (coreFilterCount - groupIndex * filterCount) : filterCount;
                vx_uint32 filterStart, filterEnd;

                if ((groupIndex == groupCount - 1)
                    && ((coreIndex - 1) >= (filterTotalCount1x1 % nnCoreCount1x1)))
                {
                    filterStart = groupIndex * nnCoreCount1x1 * filterCount
                                + (filterTotalCount1x1 % nnCoreCount1x1) * (actualFilterCount + 1)
                                + ((coreIndex - 1) - (filterTotalCount1x1 % nnCoreCount1x1)) * actualFilterCount;
                }
                else
                {
                    filterStart = groupIndex * nnCoreCount1x1 * filterCount + (coreIndex - 1) * actualFilterCount;
                }
                filterEnd = filterStart + actualFilterCount - 1;

                for (k = 0; k < weightCount1x1 * slice_count_1x1; k += linesInImageBuffer)
                {
                    vx_uint32 maxKCount = ((weightCount1x1 * slice_count_1x1) % linesInImageBuffer == 0) ? (weightCount1x1 * slice_count_1x1 / linesInImageBuffer) : (weightCount1x1 * slice_count_1x1 / linesInImageBuffer + 1);
                    kSize = gcmMIN((weightCount1x1 * slice_count_1x1 - k), linesInImageBuffer);

                    for (filterIndex = filterStart; filterIndex <= filterEnd; filterIndex++)
                    {
                        for (sk = 0; sk < linesInImageBuffer; sk++)
                        {
                            weightZIndex = (k + sk) / weightCount1x1;
                            weightYIndex = (k + sk) % weightCount1x1;
                            weightYIndex /= weightX1x1;
                            weightXIndex = weightX1x1;
                            /*Packl 0 in the last dp group*/
                            if (sk >= kSize)
                                coef = skip_value_1x1;
                            else
                            {
                                realKernelDataPtr = weight_base_ptr_1x1 + filterIndex * filterSize1x1 +
                                    filterSliceSize1x1 * weightZIndex +
                                    (weightYIndex * weightX1x1 + weightXIndex) * weightSize;

                                if (weight_format == VX_TYPE_INT8)
                                    coef = *((vx_int8 *)realKernelDataPtr);
                                else if (weight_format == VX_TYPE_UINT8)
                                    coef = *((vx_uint8 *)realKernelDataPtr);
                                else
                                    coef = *((vx_uint16 *)realKernelDataPtr);
                            }

                            if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
                            {
                                vx_int8 newCoef = ((vx_int32)coef - (vx_int32)skip_value_1x1) & 0xFF;
                                *kernelBufferInt8Ptr = (vx_uint8)newCoef;
                                kernelBufferInt8Ptr++;
                            }
                            else
                            {
                                if (weight_format == VX_TYPE_FLOAT16)
                                    coef = (coef & 0x7fff) * 2 + coef/(1<<15);

                                *kernelBufferInt16Ptr = (vx_uint16)coef;
                                kernelBufferInt16Ptr++;
                            }
                            reoder_stream_count_per_core[coreIndex]++;

                            if (k == (maxKCount - 1) * linesInImageBuffer && sk == linesInImageBuffer - 1)
                                limit_zrl_Index[limitIndex++] = elementIndex;

                            elementIndex++;
                        }
                    }
                }
            }
        }
    }
    vxmASSERT(limitIndex == (z_count_dw + z_count_1x1));
}

void fillinKernelBufferV8MergeDW1x1(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 slice_count_dw,
    vx_uint32 z_count_dw,
    vx_uint32 slice_count_1x1,
    vx_uint32 z_count_1x1,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_int32 input_zp_dw,
    vx_int32 input_zp_1x1,
    vx_int32 coef_zp_dw,
    vx_int32 coef_zp_1x1,
    vx_uint32 output_final_x,
    vx_uint32 output_final_y,
    vx_int32 z_offset,
    vx_uint32 output_size,
    vx_uint8_ptr wb_base_ptr,
    vx_uint8_ptr weight_base_ptr_dw,
    vx_uint8_ptr weight_base_ptr_1x1,
    vx_uint32* bias_base_ptr_dw,
    vx_uint32* bias_base_ptr_1x1,
    vx_uint32 index
    )
{

    vx_uint32 nnCoreCount = (weight_format == VX_TYPE_INT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : (weight_format == VX_TYPE_FLOAT16) ?
                      context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 invSizeOrder[MAX_SIZE_HISTO_COUNT] = {0};
    vx_int32 coef = 0, tmp = 0, prev = 0;

    vx_uint32 filterTotalCountDW  = z_count_dw;
    vx_uint32 filterTotalCount1x1 = z_count_1x1;
    vx_uint32 usedCoreCount1x1    = (filterTotalCount1x1 > (nnCoreCount - 1)) ? (nnCoreCount - 1) : filterTotalCount1x1;
    vx_uint32 weightCountDW       = weight_x * weight_y;
    vx_uint32 weightX1x1          = 1;
    vx_uint32 weightY1x1          = 1;
    vx_uint32 weightCount1x1      = weightX1x1 * weightY1x1;
    vx_uint32 weightSize          = (vx_uint32)vxDataType_GetSize((vx_type_e)weight_format);
    vx_uint32 weightBitSize       = weightSize * 8;
    vx_uint32 biasBitSize         = (weight_format == VX_TYPE_INT16) ? NN_INTEGER_BIAS_BITS_VIP_V7_INT16 : NN_INTEGER_BIAS_BITS_VIP_V7;

    vx_uint32 filterSizeDW        = weightCountDW * weightSize * slice_count_dw;
    vx_uint32 filterSize1x1       = weightCountDW * weightSize * slice_count_dw;
    vx_uint32* kernelBufferPtr    = (vx_uint32*)wb_base_ptr;

    vx_uint32* kernelStreamSizePtr = VX_NULL;
    vx_uint32 kernelStreamSize = 0;
    vx_uint32 alignedOffset      = ((vx_uint32)(gctUINTPTR_T)kernelBufferPtr) & 0x3F;
    vx_uint8_ptr kernelDataPtr   = VX_NULL;
    vx_uint8_ptr kernelStreamBasePtr   = VX_NULL;
    vx_uint32 streamSizeBitOffset = 0;
    vx_uint32 maxKernelStreamSizePerCore = 0;

    vx_uint32 coreIndex;
    vx_uint32 i;

    vx_uint32 bit16Flag = (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8) ? 0 : 1;
    vx_uint32 bitOffset = 0;

    vx_uint32 reorderStreamSize = 0;
    vx_uint32 reorderStreamAllCount = 0;
    vx_uint32 reorderStreamCheckCount = 0;
    vx_uint32* reorderStreamPerCoreCount = VX_NULL;
    vx_uint32 filterIndex = 0;

    vx_uint8* reorderStream = VX_NULL;
    vx_uint32 runSet2Bits = 1;
    CodeSymbol codeSymbol[THROUGHPUT * 3];

    vx_bool hasNoBias = vx_false_e;
    vx_bool hasNoZOffset = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_NO_Z_LOCATION_OFFSET);
    vx_bool hasZDP3 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3);
    vx_bool hasKernelFullCacheInterleaveFix = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_FULLCACHE_KERNEL_INTERLEAVE_FIX);
    vx_uint32 dpAmount = hasZDP3 ? 3 : 1;
    vx_uint32 zDPLoopCount = 3;
    vx_uint32 linesInImageBuffer = dpAmount * zDPLoopCount;
    vx_uint32 numOfInImageBufferDW = (vx_uint32)gcoMATH_Ceiling((vx_float32)(weightCountDW * slice_count_dw) / (vx_float32)linesInImageBuffer);
    vx_uint32 numOfInImageBuffer1x1 = (vx_uint32)gcoMATH_Ceiling((vx_float32)(weightCount1x1 * slice_count_1x1) / (vx_float32)linesInImageBuffer);

    vx_uint32 dummyStage[3] = {0};
    vx_uint32 dummyBitLength[3] = {0};
    vx_bool setDummy = vx_false_e;

    vx_uint32 nonCoefCount = 0;
    vx_uint32 totalCountIdx = 0;
    vx_uint32 nonCoefCountIdx = 0;
    vx_uint32* nonCoefIndex = VX_NULL;

    vx_int32* coefSumDW = VX_NULL;
    vx_int32* coefSum1x1 = VX_NULL;
    vx_uint32* realVzIndex = VX_NULL;
    vx_uint32* limitZRLIndex = VX_NULL;
    vx_uint32 limitZRLCountIdx = 0;

    typedef struct _gcVXWeightsMinusZP
    {
        vx_int32 sum;
        vx_uint32 filterIdx;
    }gcVXWeightsMinusZP;
    gcVXWeightsMinusZP *weightsMinusZPDW = VX_NULL;
    gcVXWeightsMinusZP *weightsMinusZP1x1 = VX_NULL;

    if (weight_format == VX_TYPE_INT16 || weight_format == VX_TYPE_UINT8)
    {
        vx_uint32 sliceIndex, weightXIndex, weightYIndex;
        vx_uint32 filterSliceSizeDW = weight_x * weight_y * weightSize;
        vx_uint32 filterSliceSize1x1 = weightX1x1 * weightY1x1 * weightSize;

        coefSumDW = (vx_int32*)vxAllocateAndZeroMemory(filterTotalCountDW * sizeof(vx_int32));
        if (!coefSumDW)
        {
            vxError("fillinKernelBufferV8Merge: OUT OF MEMORY");
            goto exit;
        }

        coefSum1x1 = (vx_int32*)vxAllocateAndZeroMemory(filterTotalCount1x1 * sizeof(vx_int32));
        if (!coefSum1x1)
        {
            vxError("fillinKernelBufferV8Merge: OUT OF MEMORY");
            goto exit;
        }

        /*Calc depth-wise sum((coef[i] - coefZP) * InZP) */
        if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT) &&
            input_zp_dw != 0 &&
            wb->wb_base->weights_quant_format == VX_QUANT_AFFINE_SCALE &&
            weight_format == VX_TYPE_UINT8)
        {
            weightsMinusZPDW = (gcVXWeightsMinusZP*)vxAllocateAndZeroMemory(filterTotalCountDW * sizeof(gcVXWeightsMinusZP));

            if (!weightsMinusZPDW)
            {
                vxError("fillinKernelBufferV8Merge: OUT OF MEMORY");
                goto exit;
            }
        }

        for (filterIndex = 0; filterIndex < filterTotalCountDW; filterIndex++)
        {
            if (weightsMinusZPDW != VX_NULL)
            {
                weightsMinusZPDW[filterIndex].filterIdx = filterIndex;
                weightsMinusZPDW[filterIndex].sum = 0;
            }
            for (sliceIndex = 0; sliceIndex < slice_count_dw; sliceIndex++)
            {
                /* add one slice data every filter */
                kernelDataPtr = weight_base_ptr_dw + filterIndex * filterSizeDW + filterSliceSizeDW * sliceIndex;

                for (weightYIndex = 0; weightYIndex < weight_y; weightYIndex++)
                {
                    for (weightXIndex = 0; weightXIndex < weight_x; weightXIndex++)
                    {
                        vx_int32 weight = 0;
                        if (weight_format == VX_TYPE_UINT8)
                        {
                            weight = *((vx_uint8 *)kernelDataPtr);
                            coefSumDW[filterIndex] += (weight - coef_zp_dw);
                        }
                        else
                        {
                            weight = *((vx_int16 *)kernelDataPtr);
                            coefSumDW[filterIndex] += weight;
                        }
                        kernelDataPtr = kernelDataPtr + weightSize;

                        if (weightsMinusZPDW != VX_NULL)
                        {
                            /*Calc sum((coef[i] - coefZP) * InZP) */
                            weightsMinusZPDW[filterIndex].sum += (weight - coef_zp_dw) * input_zp_dw;
                        }
                    }
                }
            }
        }


        /*Calc 1x1 conv sum((coef[i] - coefZP) * InZP) */
        if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT) &&
            input_zp_1x1 != 0 &&
            wb->wb_base->weights_quant_format == VX_QUANT_AFFINE_SCALE &&
            weight_format == VX_TYPE_UINT8)
        {
            weightsMinusZP1x1 = (gcVXWeightsMinusZP*)vxAllocateAndZeroMemory(filterTotalCountDW * sizeof(gcVXWeightsMinusZP));

            if (!weightsMinusZP1x1)
            {
                vxError("fillinKernelBufferV8Merge: OUT OF MEMORY");
                goto exit;
            }
        }

        for (filterIndex = 0; filterIndex < filterTotalCount1x1; filterIndex++)
        {
            if (weightsMinusZP1x1 != VX_NULL)
            {
                weightsMinusZP1x1[filterIndex].filterIdx = filterIndex;
                weightsMinusZP1x1[filterIndex].sum = 0;
            }
            for (sliceIndex = 0; sliceIndex < slice_count_1x1; sliceIndex++)
            {
                /* add one slice data every filter */
                kernelDataPtr = weight_base_ptr_1x1 + filterIndex * filterSize1x1 + filterSliceSize1x1 * sliceIndex;

                for (weightYIndex = 0; weightYIndex < weight_y; weightYIndex++)
                {
                    for (weightXIndex = 0; weightXIndex < weight_x; weightXIndex++)
                    {
                        vx_int32 weight = 0;
                        if (weight_format == VX_TYPE_UINT8)
                        {
                            weight = *((vx_uint8 *)kernelDataPtr);
                            coefSum1x1[filterIndex] += (weight - coef_zp_1x1);
                        }
                        else
                        {
                            weight = *((vx_int16 *)kernelDataPtr);
                            coefSum1x1[filterIndex] += weight;
                        }
                        kernelDataPtr = kernelDataPtr + weightSize;

                        if (weightsMinusZP1x1 != VX_NULL)
                        {
                            /*Calc sum((coef[i] - coefZP) * InZP) */
                            weightsMinusZP1x1[filterIndex].sum += (weight - coef_zp_1x1) * input_zp_1x1;
                        }
                    }
                }
            }
        }
    }

    reorderStreamAllCount = (numOfInImageBufferDW * linesInImageBuffer * filterTotalCountDW)
        + (16 / weightBitSize) /*filterPerCore need 16 bit for DW*/
        + (numOfInImageBuffer1x1 * linesInImageBuffer * filterTotalCount1x1)
        + (usedCoreCount1x1 * (16 / weightBitSize));/*filterPerCore need 16 bit * nnCoreCount */

    reorderStreamSize = reorderStreamAllCount * weightSize;

    reorderStream = (vx_uint8 *)vxAllocateAndZeroMemory(reorderStreamSize);

    if (!reorderStream)
    {
        vxError("fillinKernelBufferV8Merge: OUT OF MEMORY");
        goto exit;
    }

    reorderStreamPerCoreCount = (vx_uint32 *)vxAllocateAndZeroMemory(nnCoreCount * sizeof(vx_uint32));
    if (!reorderStreamPerCoreCount)
    {
        vxError("fillinKernelBufferV8Merge: OUT OF MEMORY");
        goto exit;
    }

    nonCoefCount = (usedCoreCount1x1 + 1) * (16 / weightBitSize);

    nonCoefIndex = (vx_uint32 *)vxAllocateAndZeroMemory(nonCoefCount * sizeof(vx_uint32));
    if (!nonCoefIndex)
    {
        vxError("fillinKernelBufferV8Merge: OUT OF MEMORY");
        goto exit;
    }

    limitZRLIndex = (vx_uint32 *)vxAllocateAndZeroMemory((filterTotalCountDW + filterTotalCount1x1) * sizeof(vx_uint32));
    if (!limitZRLIndex)
    {
        vxError("fillinKernelBufferV8Merge: OUT OF MEMORY");
        goto exit;
    }

    reorderKernelBufferV8MergeDW1x1(context, weight_x, weight_y, slice_count_dw, z_count_dw, slice_count_1x1, z_count_1x1, filters_per_core, coef_zp_dw, coef_zp_1x1,
                                    weight_format, reorderStream, weight_base_ptr_dw, weight_base_ptr_1x1, usedCoreCount1x1, reorderStreamPerCoreCount, nonCoefIndex, limitZRLIndex);

    analysisKernelStreamForHuffman(context, wb, 0, nnCoreCount, reorderStream, reorderStreamSize, reorderStreamPerCoreCount, invSizeOrder, nonCoefIndex, nonCoefCount, limitZRLIndex, (filterTotalCountDW + filterTotalCount1x1), index);

    /* Write huffman header*/
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].preEncode, 1);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].bit16Flag, 1);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].fp16Flag, 1);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].reserved, 1);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].version, 4);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].runLenTableSize, 8);

    for(i = 0; i < MAX_RUNLEN_SIZE; i++)
    {
        writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].runLenTable[i], 8);
    }
    for(i = 0; i < LOG_RUN_SIZE1; i++)
    {
        writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].mapToHuffman[i], 8);
    }
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].avgBias, 16);
    writeBits(&kernelBufferPtr, &bitOffset, 0, 16); /*reserved, must zero*/

    kernelStreamSizePtr = kernelBufferPtr;
    streamSizeBitOffset = bitOffset;
    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        writeBits(&kernelBufferPtr, &bitOffset, kernelStreamSize, 32);
    }
    /*align to 64 byte*/
    packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);
    wb->slice_array[index].kernel_stream_full_cache_size = 0;

    for (i = 0; i < THROUGHPUT * 3; i++)
    {
        codeSymbol[i].stageCode[0] = codeSymbol[i].stageCode[1] = codeSymbol[i].stageCode[2] = 0;
        codeSymbol[i].bitLength[0] = codeSymbol[i].bitLength[1] = codeSymbol[i].bitLength[2] = 0;
    }

    while(((numBestRuns-SET_1_SIZE)>>runSet2Bits) > 1)
    {
        runSet2Bits++;
    }

    kernelDataPtr = reorderStream;
    wb->max_per_core_compression_ratio = 0;

    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {

        vx_uint32 x = 0x0;
        vx_int32 pos = 0x0;
        vx_int32 run = 0x0;
        vx_uint32 dataSizeOfCore = reorderStreamPerCoreCount[coreIndex];
        vx_uint32 dataRemainingOfCore = dataSizeOfCore;
        vx_int32 size = 0;
        vx_int32 code = 0;
        vx_int32 coef16 = 0;
        vx_int32 k = 0, j = 0, m = 0;
        vx_bool isCoef;
        vx_bool isLimitZRL;
        vx_float64 compressionRatio = 0;
        /* update position of each core's beginning of compressed kernel*/
        kernelStreamBasePtr = (vx_uint8*)kernelBufferPtr;
        reorderStreamCheckCount += reorderStreamPerCoreCount[coreIndex];

        memset(codeSymbol, 0, sizeof(CodeSymbol));
        memset(dummyStage, 0, (sizeof(vx_uint32) * 3));
        memset(dummyBitLength, 0, (sizeof(vx_uint32) * 3));
        setDummy = vx_false_e;

        while(dataRemainingOfCore --)
        {

            pos = (x) % (3*THROUGHPUT);

            if (weight_format == VX_TYPE_INT8)
                coef = *((vx_int8 *)kernelDataPtr);
            else if (weight_format == VX_TYPE_UINT8)
                coef = *((vx_uint8 *)kernelDataPtr);
            else if (weight_format == VX_TYPE_INT16 ||
                weight_format == VX_TYPE_FLOAT16)
                coef16 = *((vx_int16 *)kernelDataPtr);
            else
            {
                /*Other format not suppoted now*/
                vxmASSERT(0);
            }
            kernelDataPtr += weightSize;

            if (totalCountIdx == nonCoefIndex[nonCoefCountIdx] &&
                nonCoefCountIdx < nonCoefCount)
            {
                isCoef = vx_false_e;
                nonCoefCountIdx++;
            }
            else
                isCoef = vx_true_e;

            if (limitZRLIndex != VX_NULL &&
                totalCountIdx == limitZRLIndex[limitZRLCountIdx] &&
                limitZRLCountIdx < (filterTotalCountDW + filterTotalCount1x1))
            {
                isLimitZRL = vx_true_e;
                limitZRLCountIdx++;
            }
            else
                isLimitZRL = vx_false_e;

            totalCountIdx++;

            if (bit16Flag == 0)
            {

                coef = (coef - wb->huffmanConfig[index].avgBias) & 0xFF;
                if (wb->huffmanConfig[index].preEncode)
                {
                    tmp = coef;
                    coef = (coef - prev) & 0xFF;
                    prev = tmp;
                }
            }
            else
            {

                if (wb->huffmanConfig[index].fp16Flag)
                {
                    coef16 = (coef16 & 0x7FFF) * 2 + coef16 / (1<<15);
                }
                coef16 -= wb->huffmanConfig[index].avgBias;
                if (wb->huffmanConfig[index].runLenTableSize == 0x0)
                    coef = (coef16 >> 8) & 0xFF; /*Non RZL*/
                else
                    coef = coef16 & 0xFFFF; /*RZL path*/
            }

            if (wb->huffmanConfig[index].runLenTableSize == 0x0) /* non RZL*/
            {
                if (!setDummy)
                {
                    /*Set dummy all to 0,
                    in nonZRL path 8-bit, 0 belong to group0, -1~0, residue bit length is 1, residue code = 0*/
                    size = 0;
                    k = invSizeOrder[size];
                    code = huffCode[k];

                    dummyStage[0] = code & 0x07;
                    dummyBitLength[0] = 3;

                    if (sizeCodeLen[k] > 3)
                    {
                        dummyStage[1] = code >> 3;
                        dummyBitLength[1] = 2;
                    }

                    dummyStage[2] = 0;
                    if (size == 0) size = 1; /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                    if (sizeCodeLen[k]%2 == 0)
                    {
                        dummyBitLength[2] = size - 1;
                    }
                    else
                    {
                        dummyBitLength[2] = size;
                    }

                    if (wb->huffmanConfig[index].bit16Flag)
                    {/*pack the raw data of the lower 8 bits*/

                        dummyBitLength[2] += 8;
                    }
                    setDummy = vx_true_e;
                }

                tmp = (coef & 0x80)? (coef ^ 0xff): coef;
                for (size = 0; size<7; size++)
                {
                    if (tmp<(1<<size))
                        break;
                }
                k = invSizeOrder[size];
                code = huffCode[k];

                if (sizeCodeLen[k]%2 == 0)/*huffCode length = 2 or 4, pack 1 bit to 3 or 5*/
                    code |= ((coef>>7)<<(sizeCodeLen[k]) );
                codeSymbol[pos].stageCode[0] = code & 0x07;
                codeSymbol[pos].bitLength[0] = 3;

                if (sizeCodeLen[k] > 3)
                {
                    codeSymbol[pos].stageCode[1] = code >> 3;
                    codeSymbol[pos].bitLength[1] = 2;
                }

                codeSymbol[pos].stageCode[2] = tmp &((1<<size) - 1 );
                if (size == 0) size = 1; /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                if (sizeCodeLen[k]%2 == 0)
                {
                    codeSymbol[pos].bitLength[2] = size - 1;
                }
                else
                {
                    codeSymbol[pos].stageCode[2] <<= 1;
                    codeSymbol[pos].stageCode[2] |= (coef>>7); /*The sign bit*/
                    codeSymbol[pos].bitLength[2] = size;
                }

                if (wb->huffmanConfig[index].bit16Flag)
                {/*pack the raw data of the lower 8 bits*/

                    codeSymbol[pos].stageCode[2] = codeSymbol[x%(3*THROUGHPUT)].stageCode[2]* 256 + (coef16 &0xff);
                    codeSymbol[pos].bitLength[2] += 8;
                }
                OutputAt(x, &kernelBufferPtr, &bitOffset, codeSymbol);
                x++; /*counter*/
            }
            else /* RZL enable*/
            {
                if (!setDummy)
                {
                    /*Set dummy all to 0,
                    in ZRL path 8-bit, 0 belong to group7, -128~127, residue bit length is 8, residue code = 0*/
                    size = 7;
                    j = invSizeOrder[size];
                    code = huffCode[j];

                    dummyStage[0] = code & 0x07;
                    dummyBitLength[0] = 3;

                    if (sizeCodeLen[j] > 3)
                    {
                        dummyStage[1] = code >> 3;
                        dummyBitLength[1] = 2;
                    }

                    if (size == 7)
                    {
                        size = 8;
                    }

                    if (sizeCodeLen[j] % 2 == 0)
                    {
                        dummyStage[2] = 0;  /*last bit was coded*/
                        dummyBitLength[2] = size - 1;
                    }
                    else
                    {
                        dummyStage[2] = 0;
                        dummyBitLength[2] = size;
                    }
                    if (wb->huffmanConfig[index].bit16Flag)
                    {
                        dummyStage[2] = 0;
                        dummyBitLength[2] += 8;
                    }

                    setDummy = vx_true_e;
                }

                if (coef == 0 && isCoef && !isLimitZRL)
                {
                    run++;
                    if (dataRemainingOfCore != 0)
                    {
                        continue;
                    }
                }
                if (coef || (dataRemainingOfCore == 0) || !isCoef || isLimitZRL) /* if last pixel, force process run zeros*/
                {
                    /* process run zeros*/
                    while(run)
                    {
                        for (j = numBestRuns - 1; j>=0; j--)
                        {
                            if (run >= bestRunsSorted[j] + 1)
                            {
                                k = run / (bestRunsSorted[j] + 1);
                                run %= (bestRunsSorted[j] + 1);
                                break;
                            }
                        }
                        if (j >= 0)
                        {
                            vx_int32 n = bestRunsSorted[j];
                            size = 8;
                            for (m = 0; m<SET_1_SIZE; m++)
                            {
                                if (best2Runs[m] == n)
                                {
                                    size = 0;
                                    break;
                                }
                            }
                            if (size == 8)
                            {
                                for (m = 0; m<numBestRuns - SET_1_SIZE; m++)
                                    if (outBest2Run[m] == n)
                                        break;

                            }
                            j = invSizeOrder[size];
                            while(k--){/*k of the runs*/
                                code = huffCode[j];
                                if (sizeCodeLen[j] % 2 == 0)/*huffCode length = 2 or 4, pack 1 bit to 3 or 5*/
                                    code |= ((m&1)<<(sizeCodeLen[j]) );
                                codeSymbol[pos].stageCode[0] = code & 0x07;
                                codeSymbol[pos].bitLength[0] = 3;

                                if (sizeCodeLen[j] > 3)
                                {
                                    codeSymbol[pos].stageCode[1] = code >> 3;
                                    codeSymbol[pos].bitLength[1] = 2;
                                }
                                if (size == 0 )
                                {/*The m already code*/
                                    codeSymbol[pos].bitLength[2] = (sizeCodeLen[j]%2 == 0) ? 0:1;
                                    codeSymbol[pos].stageCode[2] = m; /*If (sizeCodeLen[j]%2 == 0), should be m/2, but 0 bit output, doesn't matter*/
                                }
                                else if (sizeCodeLen[j]%2 == 0)
                                {
                                    codeSymbol[pos].stageCode[2] = m/2;
                                    codeSymbol[pos].bitLength[2] = runSet2Bits - 1;
                                }
                                else
                                {
                                    codeSymbol[pos].stageCode[2] = m;
                                    codeSymbol[pos].bitLength[2] = runSet2Bits;
                                }

                                OutputAt(x, &kernelBufferPtr, &bitOffset, codeSymbol);
                                x++;
                                /* update pos*/
                                pos = (x) % (3*THROUGHPUT);
                            }
                        }
                    }
                    /*process current pixel*/
                    if (coef)
                    {
                        vx_int8 ch;
                        coef>>=(wb->huffmanConfig[index].bit16Flag * 8);
                        if(coef<0x80)
                            coef+=wb->huffmanConfig[index].bit16Flag; /*Since we have to encode zero (run-length only handle 16-bits zero, 8 bit zero should use Size code*/
                        ch = (vx_int8)coef;
                        coef = abs(ch);
                        m = coef*2 + (ch<0?1:0);
                        for (j = 6; j>=0; j--)
                        {
                            if (coef>=(1<<j))
                            {
                                break;
                            }
                        }
                        size = j+1;
                        if (invSizeOrder[size]==8)
                        { /*The minimum freq, we out the raw data*/
                            size = 7;
                        }
                        if (size == 7)
                        {
                            m = ch&0xff;
                            if(wb->huffmanConfig[index].bit16Flag)
                                m = (coef16 >> 8) & 0xff;
                        }

                        j = invSizeOrder[size];
                        code = huffCode[j];
                        if (sizeCodeLen[j]%2 == 0)/*huffCode length = 2 or 4, pack 1 bit to 3 or 5*/
                            code |= ((m&1) << (sizeCodeLen[j]));
                        codeSymbol[pos].stageCode[0] = code & 0x07;
                        codeSymbol[pos].bitLength[0] = 3;

                        if (sizeCodeLen[j] > 3)
                        {
                            codeSymbol[pos].stageCode[1] = code >> 3;
                            codeSymbol[pos].bitLength[1] = 2;
                        }

                        if (size == 7)
                        {
                            size = 8;
                        }
                        /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                        if (size == 0)
                            size = 1;
                        if (sizeCodeLen[j]%2 == 0)
                        {
                            codeSymbol[pos].stageCode[2] = m/2;  /*last bit was coded*/
                            codeSymbol[pos].bitLength[2] = size - 1;
                        }
                        else
                        {
                            codeSymbol[pos].stageCode[2] = m;
                            codeSymbol[pos].bitLength[2] = size;
                        }
                        if (wb->huffmanConfig[index].bit16Flag)
                        {
                            codeSymbol[pos].stageCode[2] = codeSymbol[x%(3*THROUGHPUT)].stageCode[2]* 256 + (coef16 & 0xff);
                            codeSymbol[pos].bitLength[2] += 8;
                        }
                        OutputAt(x, &kernelBufferPtr, &bitOffset, codeSymbol);
                        x++; /*counter*/
                    }
                    else if (!isCoef || (dataRemainingOfCore != 0x0) || isLimitZRL)
                    {
                        /*those non-coef count like bias, num_of_vz & z_offset couldn't skip by zero-run as HW requres,
                        need also encode those 0 to huffman code,
                        in ZRL path 8-bit, 0 belong to group7, -128~127, residue bit length is 8, residue code = 0*/
                        size = 7;
                        j = invSizeOrder[size];
                        code = huffCode[j];

                        codeSymbol[pos].stageCode[0] = code & 0x07;
                        codeSymbol[pos].bitLength[0] = 3;

                        if (sizeCodeLen[j] > 3)
                        {
                            codeSymbol[pos].stageCode[1] = code >> 3;
                            codeSymbol[pos].bitLength[1] = 2;
                        }

                        if (size == 7)
                        {
                            size = 8;
                        }

                        if (sizeCodeLen[j] % 2 == 0)
                        {
                            codeSymbol[pos].stageCode[2] = 0;  /*last bit was coded*/
                            codeSymbol[pos].bitLength[2] = size - 1;
                        }
                        else
                        {
                            codeSymbol[pos].stageCode[2] = 0;
                            codeSymbol[pos].bitLength[2] = size;
                        }
                        if (wb->huffmanConfig[index].bit16Flag)
                        {
                            codeSymbol[pos].stageCode[2] = codeSymbol[x%(3*THROUGHPUT)].stageCode[2]* 256 + (coef16 & 0xff);
                            codeSymbol[pos].bitLength[2] += 8;
                        }
                        OutputAt(x, &kernelBufferPtr, &bitOffset, codeSymbol);
                        x++; /*counter*/
                    }
                }
            }

            if(dataRemainingOfCore == 0)
            {
                vx_uint32 dummyCount = (x & 0x1)? 5: 4;
                /*Hit the end bitsteam of core, add dummy stage process the next*/
                vxmASSERT(setDummy == vx_true_e);
                addDummy(x, &kernelBufferPtr, &bitOffset, codeSymbol, dummyCount, dummyStage, dummyBitLength);
                break;
            }
        }

        /* Go back to update kernelStreamSize. */
        kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);
        writeBits(&kernelStreamSizePtr, &streamSizeBitOffset, kernelStreamSize*8 + bitOffset, 32);

        /* pad 0 at the end of core*/
        packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);
        kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);
        /*With RTL bug fix, full cache mode size needn't using maxSize * coreCount*/
        wb->slice_array[index].kernel_stream_full_cache_size += kernelStreamSize;

        if (maxKernelStreamSizePerCore < kernelStreamSize)
            maxKernelStreamSizePerCore = kernelStreamSize;

        compressionRatio = (vx_float64)kernelStreamSize / (vx_float64)(dataSizeOfCore * weightSize);
        if (wb->max_per_core_compression_ratio < compressionRatio)
            wb->max_per_core_compression_ratio = compressionRatio;

        prev = 0;
    }
    vxmASSERT(reorderStreamAllCount == reorderStreamCheckCount); /*Check if the data count is the same*/

    /*Add non compressed biases & Z_OFFSET & perFilterPostMul & perFilterPostShift after huffman bit stream*/
    kernelStreamBasePtr = (vx_uint8*)kernelBufferPtr;
    for (filterIndex = 0; filterIndex < filterTotalCountDW; filterIndex++)
    {

        if (!hasNoBias)
        {
            vx_uint32 biasData = 0;
            vx_int64 bias64 = 0;
            vx_uint32 realFilterIndex;

            if (context->options.enableNonZeroBalance &&
                !hasNoZOffset &&
                !wb->wb_base->hw_depth_wise &&
                realVzIndex != VX_NULL)
                realFilterIndex = realVzIndex[filterIndex];
            else
                realFilterIndex = filterIndex;

            if (bias_base_ptr_dw)
            {
                if(bias_format == VX_TYPE_INT64)
                    bias64 = *(((vx_int64 *)bias_base_ptr_dw) + filterIndex);
                else
                    biasData = *(bias_base_ptr_dw + realFilterIndex);
            }
            else
                biasData = 0;

            /* NewBias = sum((Coef[i] - CoefZP) * - InZP) + Bias*/
            if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT)
                && input_zp_dw != 0
                && weight_format == VX_TYPE_UINT8)
            {
                biasData -= weightsMinusZPDW[realFilterIndex].sum;
            }

            if (weight_format == VX_TYPE_UINT8)
                biasData += coefSumDW[realFilterIndex] * 128;

            if (weight_format == VX_TYPE_INT16)
            {
                vx_int64 sum64 = 0;
                vx_int32 bias32;
                vx_int16 bias16;

                if (bias_format != VX_TYPE_INT64)
                {
                    if ((vx_int32)biasData < 0)
                    {
                        bias64 = ~0;
                        bias64 = (bias64 >> 32) << 32;
                        bias64 = bias64 | (vx_int64)biasData;
                    }
                    else
                        bias64 = (vx_int64)biasData;
                }

                if (coefSumDW[realFilterIndex] < 0)
                {
                    sum64 = ~0;
                    sum64 = (sum64 >> 32) << 32;
                    sum64 = sum64 | (vx_int64)coefSumDW[realFilterIndex];
                }
                else
                    sum64 = (vx_int64)coefSumDW[realFilterIndex];

                bias64 += sum64 * 128;
                bias32 = (vx_int32)bias64;
                writeBits(&kernelBufferPtr, &bitOffset, bias32, 32);
                bias16 = (vx_int16)(bias64 >> 32);
                writeBits(&kernelBufferPtr, &bitOffset, (vx_int32)bias16, 16);
            }
            else
                writeBits(&kernelBufferPtr, &bitOffset, biasData, biasBitSize);
        }

        if (!hasNoZOffset)
        {
            vx_uint32 offsetValue;
            vx_uint32 realFilterIndex;

            if (context->options.enableNonZeroBalance &&
                !wb->wb_base->hw_depth_wise &&
                realVzIndex != VX_NULL)
                realFilterIndex = realVzIndex[filterIndex];
            else
                realFilterIndex = filterIndex;

            if (z_offset > 0)
                offsetValue = (vx_uint32)z_offset * realFilterIndex;
            else if (output_size > 0)
                offsetValue = output_final_x * output_final_y * output_size * realFilterIndex;
            else
                offsetValue = output_final_x * output_final_y * weightSize * realFilterIndex;

            writeBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS_VIP_V7);
        }

    }

    for (filterIndex = 0; filterIndex < filterTotalCount1x1; filterIndex++)
    {

        if (!hasNoBias)
        {
            vx_uint32 biasData = 0;
            vx_int64 bias64 = 0;
            vx_uint32 realFilterIndex;

            if (context->options.enableNonZeroBalance &&
                !hasNoZOffset &&
                !wb->wb_base->hw_depth_wise &&
                realVzIndex != VX_NULL)
                realFilterIndex = realVzIndex[filterIndex];
            else
                realFilterIndex = filterIndex;

            if (bias_base_ptr_1x1)
            {
                if(bias_format == VX_TYPE_INT64)
                    bias64 = *(((vx_int64 *)bias_base_ptr_1x1) + filterIndex);
                else
                    biasData = *(bias_base_ptr_1x1 + realFilterIndex);
            }
            else
                biasData = 0;

            /* NewBias = sum((Coef[i] - CoefZP) * - InZP) + Bias*/
            if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT)
                && input_zp_1x1 != 0
                && weight_format == VX_TYPE_UINT8)
            {
                biasData -= weightsMinusZP1x1[realFilterIndex].sum;
            }

            if (weight_format == VX_TYPE_UINT8)
                biasData += coefSum1x1[realFilterIndex] * 128;

            if (weight_format == VX_TYPE_INT16)
            {
                vx_int64 sum64 = 0;
                vx_int32 bias32;
                vx_int16 bias16;

                if (bias_format != VX_TYPE_INT64)
                {
                    if ((vx_int32)biasData < 0)
                    {
                        bias64 = ~0;
                        bias64 = (bias64 >> 32) << 32;
                        bias64 = bias64 | (vx_int64)biasData;
                    }
                    else
                        bias64 = (vx_int64)biasData;
                }

                if (coefSum1x1[realFilterIndex] < 0)
                {
                    sum64 = ~0;
                    sum64 = (sum64 >> 32) << 32;
                    sum64 = sum64 | (vx_int64)coefSum1x1[realFilterIndex];
                }
                else
                    sum64 = (vx_int64)coefSum1x1[realFilterIndex];

                bias64 += sum64 * 128;
                bias32 = (vx_int32)bias64;
                writeBits(&kernelBufferPtr, &bitOffset, bias32, 32);
                bias16 = (vx_int16)(bias64 >> 32);
                writeBits(&kernelBufferPtr, &bitOffset, (vx_int32)bias16, 16);
            }
            else
                writeBits(&kernelBufferPtr, &bitOffset, biasData, biasBitSize);
        }

        if (!hasNoZOffset)
        {
            vx_uint32 offsetValue;
            vx_uint32 realFilterIndex;

            if (context->options.enableNonZeroBalance &&
                !wb->wb_base->hw_depth_wise &&
                realVzIndex != VX_NULL)
                realFilterIndex = realVzIndex[filterIndex];
            else
                realFilterIndex = filterIndex;

            if (z_offset > 0)
                offsetValue = (vx_uint32)z_offset * realFilterIndex;
            else if (output_size > 0)
                offsetValue = output_final_x * output_final_y * output_size * realFilterIndex;
            else
                offsetValue = output_final_x * output_final_y * weightSize * realFilterIndex;

            writeBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS_VIP_V7);
        }

    }

    /*Also align to 64 byte for post processing stream*/
    packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);
    kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);
    wb->slice_array[index].kernel_stream_full_cache_size += kernelStreamSize;

    if (maxKernelStreamSizePerCore < kernelStreamSize)
        maxKernelStreamSizePerCore = kernelStreamSize;
    /*Per HW, post processing stream size is needed in SRAM, when partial mode, need be noted as one core*/
       /*bug1980*/
    if(gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_IMAGE_NOT_PACKED_IN_SRAM_FIX))
        wb->slice_array[index].kernel_align_stream_size = (vx_size)(maxKernelStreamSizePerCore *((usedCoreCount1x1 + 1) + 1));
    else
        wb->slice_array[index].kernel_align_stream_size = (vx_size)(maxKernelStreamSizePerCore * ((nnCoreCount + 1) + 1));

    if (!hasKernelFullCacheInterleaveFix)
    {
        /*if did't fix full cache interleave fix, full cache size also need use align size*/
        wb->slice_array[index].kernel_stream_full_cache_size = wb->slice_array[index].kernel_align_stream_size;
    }

    wb->slice_array[index].kernel_max_stream_size_percore = maxKernelStreamSizePerCore;

exit:
    if (coefSumDW)
        vxFree(coefSumDW);
    if (weightsMinusZPDW)
        vxFree(weightsMinusZPDW);
    if (coefSum1x1)
        vxFree(coefSum1x1);
    if (weightsMinusZP1x1)
        vxFree(weightsMinusZP1x1);
    if (reorderStream)
        vxFree(reorderStream);
    if (reorderStreamPerCoreCount)
        vxFree(reorderStreamPerCoreCount);
    if (nonCoefIndex)
        vxFree(nonCoefIndex);
    if (realVzIndex)
        vxFree(realVzIndex);
    if (limitZRLIndex)
        vxFree(limitZRLIndex);

    return;
}


#define DUMP_TP_ENC 0

void reorderTPKernelBufferHuffman(
    vx_weights_biases_parameter wb,
    vx_uint32 weight_z,
    vx_uint32 filter_count,
    vx_uint32 total_weight_z,
    vx_enum  weight_format,
    vx_uint8_ptr reorder_stream,
    vx_uint8_ptr weight_base_ptr,
    vx_uint8* postMul,
    vx_uint8* postShift
    )
{
    vx_enum weightFomat             = weight_format;
    vx_uint32 weightSize            = (vx_uint32)vxDataType_GetSize((vx_type_e)weightFomat);
    vx_uint32 sliceCount            = weight_z;
    vx_uint32 filterSize            = total_weight_z * weightSize;
    vx_uint32 filterCount           = filter_count;
    vx_uint8* kernelDataPtr         = VX_NULL;
    vx_uint8* kernelBufferInt8Ptr   = reorder_stream;
    vx_uint16* kernelBufferInt16Ptr = (vx_uint16*)reorder_stream;
    vx_uint32 filterIndex, sliceIndex;
#if DUMP_TP_ENC
    static vx_uint32 n = 0;
    vx_char fileName[128];
    FILE * pfile1 = NULL;
    sprintf(fileName, "%s_%d.txt", "reordered_kernel", n++);

    pfile1 = fopen(fileName, "wt");
#endif

    for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
    {
        /* add slices of every filter*/
        kernelDataPtr = weight_base_ptr + weightSize * sliceIndex;
        for (filterIndex = 0; filterIndex < filterCount; filterIndex++)
        {
            vx_uint32 weight = 0;

            if (weightFomat == VX_TYPE_INT8)
                weight = *((vx_int8 *)kernelDataPtr);
            else if (weightFomat == VX_TYPE_UINT8)
                weight = *((vx_uint8 *)kernelDataPtr);
            else
                weight = *((vx_uint16 *)kernelDataPtr);
            kernelDataPtr += filterSize;

            if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
            {
                *kernelBufferInt8Ptr = (vx_uint8)weight;
                kernelBufferInt8Ptr ++;
#if DUMP_TP_ENC
                if(pfile1 != NULL)
                {
                    fprintf(pfile1, "kz:%d, vz:%d, coef:%d\n", sliceIndex, filterIndex, weight);
                }
#endif
            }
            else
            {
                *kernelBufferInt16Ptr = (vx_uint16)weight;
                kernelBufferInt16Ptr ++;
            }
        }
    }

#if DUMP_TP_ENC
    if(pfile1 != NULL)
    {
        fclose(pfile1);
    }
#endif
}

void analysisTPStreamForHuffman(
    vx_weights_biases_parameter wb,
    vx_uint32 sliceCount,
    vx_uint32 filterCount,
    vx_uint8* reorderStream,
    vx_uint32* invSizeOrder,
    vx_uint32 index
    )
{
    vx_uint8  * pBS_U08      = (unsigned char * )reorderStream;
    vx_uint16 * pBS_U16      = (unsigned short *)reorderStream;
    vx_enum weightFormat = wb->wb_base->weights_data_format;
    vx_uint32 weightSize = (vx_uint32)vxDataType_GetSize((vx_type_e)weightFormat);
    vx_uint8 bit16Flag   = (weightFormat == VX_TYPE_INT8 || weightFormat == VX_TYPE_UINT8) ? 0 : 1;
    vx_uint32 kernelDataCount  = sliceCount * filterCount;
    vx_uint32 kernelDataBytes  = kernelDataCount * weightSize;
    vx_uint32 dataCountToPross = kernelDataCount;

    vx_int32 i, j, k, bias;
    vx_int32 x=0;
    vx_int32 runZeros[256], run; /*The histogram of how many runs*/
    vx_int32 histo[256], sizeHisto[9] = {0,0,0,0,0,0,0,0,0};
    vx_int32 sizeOrder[]  = {0,1,2,3,4,5,6,7,8,};
    vx_int32 codingType   = HUFFMAN_CODING_TYPE_RUN_LEN; /* defalt RZL*/
    vx_int32 prevEncode   = 0, prev = 0, prevHisto[256], prevRunZeros[256], prevRun; /*previous pixel*/
    vx_float32 p, entropy = 0.f, prevEntropy = 0.f;
    vx_int32 sliceIndex = 0, accumSize = 0x0;

    /* this variable indicate DataAccumCount when Start of core*/
    vx_int32 *accumCountEndOfCore = (vx_int32 *)vxAllocateAndZeroMemory(sliceCount * sizeof(vx_int32));

    if (accumCountEndOfCore == VX_NULL)
    {
        vxError("analysisKernelStreamForHuffman: OUT OF MEMORY\n");
        goto exit;
    }

    for (i = 0; i < 256; i++)
    {
        runZeros[i] = histo[i] = 0;
        prevHisto[i] = prevRunZeros[i] = 0;
    }


    for (sliceIndex = 0; sliceIndex != (vx_int32)sliceCount; sliceIndex++)
    {
        accumSize += filterCount;
        accumCountEndOfCore[sliceIndex] = accumSize - 1;
    }

    prevRun = run = 0;
    sliceIndex = 0;
    while(dataCountToPross--)
    {
        vx_uint32 endOfCore = (dataCountToPross + accumCountEndOfCore[sliceIndex] == kernelDataCount - 1) ? 1 : 0;
        if(endOfCore)
        {
            sliceIndex++;
        }

        if (!bit16Flag)
        {
            i = *(pBS_U08++);
        }
        else
        {
            i = *(pBS_U16++);
            if (weightFormat == VX_TYPE_FLOAT16)
            {
                i = (i & 0x7fff) * 2 + i/(1<<15); /*(i&0x8000)/128 + (i&0x7f00)*2 + (i&0xff);*/
            }
        }

        if (i == 0) run++;
        if ((run == sizeof(runZeros)/sizeof(int)) || i || endOfCore)
        {
            if (run)
            {
                runZeros[run - 1]++;
                run = 0;
            }
        }
        histo[i>>(bit16Flag*8)]++;
        /*Get statistic of prediction from previous pixel*/
        if(bit16Flag == 0)
        {
            j = i;
            i = (i-prev)&0xff;
            prev = j;
            if (i == 0) prevRun++;
            if (prevRun == sizeof(prevRunZeros)/sizeof(int) || i || endOfCore)
            {
                if (prevRun){
                    prevRunZeros[prevRun - 1]++;
                    prevRun = 0;
                }
            }
            prevHisto[i]++;
        }

        /* prev encode should be truncated between cores, when analysis, reset prev.*/
        if (endOfCore)
        {
            prev = 0x0;
        }
    }
    if (run)
    { /*last run*/
        runZeros[run - 1]++;
        run = 0;
    }
    if (prevRun)
    { /*last run*/
        prevRunZeros[prevRun - 1]++;
        prevRun = 0;
    }

    j = histo[0];
    bias = 0;

    for (i = 1; i < 256; i++)
    {
        if (histo[i] > j)
        {
            j = histo[i];
            bias = i;
        }
    }
    if (j*3 < histo[0]*4 && bit16Flag == 0)
        bias = 0;

    /*16 bits, we only get the high bit bias, we need to get the low bit bias*/
    if (bit16Flag)
    {
        pBS_U16 = (unsigned short *)reorderStream;
        dataCountToPross = kernelDataCount;
        while(dataCountToPross--)
        {
            i = *(pBS_U16);
            pBS_U16++;
            if (weightFormat == VX_TYPE_FLOAT16)
                i = (i&0x7fff)*2 + i/(1<<15);

            if ((i>>8) == bias)
                prevHisto[i&0xff]++;
        }

        j = prevHisto[0];
        k = 0;

        for (i = 1; i < 256; i++)
        {
            if (prevHisto[i] > j)
            {
                j = prevHisto[i];
                k = i;
            }
        }
        bias = bias*256+k;
    }

    x = 0;
    for (i = 0; i<256; i++)
    {
        p = histo[i] / (vx_float32)(kernelDataCount);
        if (histo[i])
            entropy += -p*(vx_float32)log(p)/(vx_float32)log(2.0f);

        p =  prevHisto[i] / (vx_float32)kernelDataBytes;
        if (prevHisto[i])
            prevEntropy += - p*(vx_float32)log(p)/(vx_float32)log(2.0f);
    }

    if (bit16Flag == 0)
    {
        if (prevEntropy < entropy || 2*prevHisto[0] + prevHisto[255] + prevHisto[1] > 3*histo[bias] + histo[(bias+1)&0xff] + histo[(bias-1)&0xff])
        {
            entropy = prevEntropy;
            prevEncode = 1; /*Using prevEncode, prediction from previous pixel*/
            for (i = 0; i < 256; i++)
            {
                histo[i] = prevHisto[i];
                runZeros[i] = prevRunZeros[i];
            }
            bias = 0;
        }
    }
    prevEntropy = entropy;

    if (bias != 0)
    {
        for (i = 0; i < 256; i++)
        {
            histo[i] = runZeros[i] = 0;
        }
        dataCountToPross = kernelDataCount;
        pBS_U08 = (vx_uint8 * )reorderStream;
        pBS_U16 = (vx_uint16 *)reorderStream;
        sliceIndex = 0;
        while(dataCountToPross--)
        {
            vx_uint32 endOfCore = (dataCountToPross + accumCountEndOfCore[sliceIndex] == kernelDataCount - 1) ? 1 : 0;
            if(endOfCore)
            {
                sliceIndex++;
            }

            if (bit16Flag == 0)
            {
                i = *(pBS_U08++);
            }
            else
            {
                i = *(pBS_U16++);
                if (weightFormat == VX_TYPE_FLOAT16)
                {
                    i = (i&0x7fff)*2 + i/(1<<15);
                }
            }

            i = (i - bias);
            if (i == 0) run++;
            if (run == sizeof(runZeros) / sizeof(vx_int32) || i || endOfCore)
            {
                if(run)
                {
                    runZeros[run - 1]++;
                    run = 0;
                }
            }

            histo[(i>>(bit16Flag*8))&0xff]++;
        }
        if (run)
        { /*last run*/
            runZeros[run - 1]++;
            run = 0;
        }
    }

    /*Get all the runZeros present. supposed to be histo[0], but, for 16-bit, we only count the runs for double zeros.*/
    for (i = 0; i < 256; i++)
    {
        run += runZeros[i] * (i+1);
    }

    if (bit16Flag == 0 && ((unsigned int)histo[0] <= kernelDataBytes / 8 || histo[0] < (histo[1] + histo[255]) * 3/4 || entropy >= 5.25f) && entropy > 3.5f )
    {
         codingType = HUFFMAN_CODING_TYPE_NON_RUN_LEN;
    }
    if (bit16Flag && run < (histo[1] + histo[255]) / 4)
    {
         codingType = HUFFMAN_CODING_TYPE_NON_RUN_LEN;
    }
    if (bit16Flag && entropy > 5.25f)
        codingType = 0; /*Force to test run-length*/


    if (codingType == HUFFMAN_CODING_TYPE_RUN_LEN)
    {
        FindBest2PlusRunSets(runZeros, sizeof(runZeros)/sizeof(int), (kernelDataBytes >> bit16Flag) - run);
    }

    if (codingType == HUFFMAN_CODING_TYPE_NON_RUN_LEN)
    {
        j = 0;
        for (i = 0; i < 8;i++){
            for (; j<(1<<i); j++){
                sizeHisto[i] += histo[j] + histo[j^0xff];
            }
        }

        for (i = 0; i < 7; i++){
            vx_int32 maxFreq = 0, maxPos = i;
            for (j = i; j<8; j++){
                if (maxFreq<sizeHisto[j])
                {
                    maxFreq = sizeHisto[j];
                    maxPos = j;
                }
            }
            if (maxPos > i)
            {
                j = sizeHisto[maxPos];
                sizeHisto[maxPos] = sizeHisto[i];
                sizeHisto[i] = j;

                j = sizeOrder[maxPos];
                sizeOrder[maxPos] = sizeOrder[i];
                sizeOrder[i] = j;
            }
        }
        for (i = 0; i < 8; i++)
        {
            invSizeOrder[sizeOrder[i]] = i;
        }
        entropy = 0.f;
        j = 0;
        for (i = 0; i < 8; i++)
        {
            p = sizeHisto[i]/(vx_float32)kernelDataBytes;
            if(p>0.f)
                entropy += -p*(vx_float32)log(p)/(vx_float32)log(2.0f);
            j += sizeHisto[i]*sizeCodeLen[i];
        }
    }

    if (codingType == HUFFMAN_CODING_TYPE_RUN_LEN)
    {
        j = 1;
        if (bit16Flag)
           histo[0] -= run; /*The zeros not runs*/

        run = 0;
        for (i = 1; i < 8; i++)
        {
            for (; j < (1<<i); j++)
            {
                sizeHisto[i] += histo[j - bit16Flag] + histo[(-j)&0xff];
            }
        }
        sizeHisto[7] += histo[0x80];/*128 haven't put in yet.*/

        /*Get the frequency of run-length.*/
        for (i = 0; i < numBestRuns; i++)
        {
            if (freq[i] < 0) /*The set-1*/
                sizeHisto[0] += abs(freq[i]);
            else /*Set-2*/
                sizeHisto[8] += abs(freq[i]);
        }

        k = sizeHisto[1];
        for (i = 2; i < 7; i++)
        {
            if (sizeHisto[i] < k)
                k = sizeHisto[i];
        }

        /*avoid the run-length be the last*/
        if (sizeHisto[0] <= k)
            sizeHisto[0] = k+2;
        if (sizeHisto[8] <= k)
            sizeHisto[8] = k+2;

        if (sizeHisto[7] <= k) /*The 7 is the escape code, shouldn't be the last (the last will be mergy to 7).*/
            sizeHisto[7] = k+1;


         /*sorting the sizeHisto*/
         for (i = 0; i<8; i++)
         {
            vx_int32 maxFreq = 0, maxPos = i;
            for (j = i; j < 9; j++)
            {
                if(maxFreq < sizeHisto[j])
                {
                    maxFreq = sizeHisto[j];
                    maxPos = j;
                }
            }
            if (maxPos > i)
            {
                j = sizeHisto[maxPos];
                sizeHisto[maxPos] = sizeHisto[i];
                sizeHisto[i] = j;

                j = sizeOrder[maxPos];
                sizeOrder[maxPos] = sizeOrder[i];
                sizeOrder[i] = j;
            }
        }

        if(sizeOrder[7]%8 == 0 && sizeOrder[6]%8 > 0)
        { /*The run-length cannot be the escape code. swap to size 6*/
            j = sizeOrder[6];
            sizeOrder[6] = sizeOrder[7];
            sizeOrder[7] = j;

            j = sizeHisto[6];
            sizeHisto[6] = sizeHisto[7];
            sizeHisto[7] = j;

        }
        else if(sizeOrder[7]%8 == 0 && sizeOrder[6]%8 == 0)
        { /*The run-length cannot be the escape code. swap to size 6*/
            /*5,7,6*/
            j = sizeOrder[6];
            sizeOrder[6] = sizeOrder[7];
            sizeOrder[7] = j;

            j = sizeHisto[6];
            sizeHisto[6] = sizeHisto[7];
            sizeHisto[7] = j;
            /*6,7,5*/
            j = sizeOrder[5];
            sizeOrder[5] = sizeOrder[7];
            sizeOrder[7] = j;

            j = sizeHisto[5];
            sizeHisto[5] = sizeHisto[7];
            sizeHisto[7] = j;
        }

        for(i = 0; i < 9; i++)
            invSizeOrder[sizeOrder[i]] = i;

        entropy = 0.f;
        j = 0;
        x = 0;
        for(i = 0; i < 9; i++)
        {
            x += sizeHisto[i];
        }
        for(i = 0; i < 8; i++)
        {
            p = sizeHisto[i] / (vx_float32)x;
            if (p > 0.f)
                entropy += -p*(vx_float32)log(p)/(vx_float32)log(2.0f);
            j += sizeHisto[i]*sizeCodeLen[i];
        }
    }


    /* update compression header*/
    wb->huffmanConfig[index].preEncode = (vx_uint8)prevEncode;
    wb->huffmanConfig[index].bit16Flag  = bit16Flag;
    wb->huffmanConfig[index].fp16Flag   = (weightFormat == VX_TYPE_FLOAT16) ? 1 : 0;
    wb->huffmanConfig[index].reserved   = 0x0; /* must be zero*/
    wb->huffmanConfig[index].version    = 0x1; /* 4'b0001 for version 1.0*/
    wb->huffmanConfig[index].runLenTableSize = (codingType == HUFFMAN_CODING_TYPE_NON_RUN_LEN) ? 0: (vx_uint8)numBestRuns;

    /* runlenTable 8 * 18 = 144*/
    for(i = 0; i<SET_1_SIZE; i++)
    {
        wb->huffmanConfig[index].runLenTable[i] = (vx_uint8)best2Runs[i];
    }
    for(; i< numBestRuns; i++)
    {
        wb->huffmanConfig[index].runLenTable[i] = (vx_uint8)outBest2Run[i - SET_1_SIZE];
    }
    for(i = 0; i<8; i+=2)
    {
        wb->huffmanConfig[index].mapToHuffman[i/2] = (vx_uint8)(sizeOrder[i+1]*16 + sizeOrder[i]);
    }
    wb->huffmanConfig[index].avgBias = (vx_uint8)bias;
    wb->huffmanConfig[index].reserved = 0x0; /* reserved 16, must be zero*/

exit:
    if (accumCountEndOfCore != VX_NULL)
        vxFree(accumCountEndOfCore);
}

void calcTPKernelBufferSizeHuffman(
    vx_weights_biases_parameter wb,
    vx_uint32 weight_z,
    vx_uint32 filter_count,
    vx_uint32 total_weight_z,
    vx_enum  weight_format,
    vx_uint32 skip_value,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32 index
    )
{
    vx_uint32 invSizeOrder[MAX_SIZE_HISTO_COUNT] = {0};
    vx_int32 coef = 0, tmp = 0, prev = 0;

    vx_uint32 weightSize       = (vx_uint32)vxDataType_GetSize((vx_type_e)weight_format);

    vx_uint32 sliceCount            = weight_z;
    vx_uint32 filterCount           = filter_count;
    vx_uint32 kernelStreamSize      = 0;
    vx_uint32 biasSize              = 0;
    vx_uint32 kernelSize            = 0;
    vx_uint32 kernelBitSize         = 0;

    vx_uint32 nonZeroCoefCount      = 0;
    vx_uint32 rsvCoefCount          = 0;

    vx_uint8_ptr kernelDataPtr      = VX_NULL;

    vx_uint32 sliceIndex = 0;

    vx_uint32 bit16Flag = (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8) ? 0 : 1;

    vx_uint32 reorderStreamAllCount = sliceCount * filterCount;
    vx_uint32 reorderStreamSize = reorderStreamAllCount * weightSize;

    vx_uint8* reorderStream = VX_NULL;
    vx_uint32 runSet2Bits = 1;

    wb->slice_array[index].kernel_stream_size = 0;
    reorderStream = (vx_uint8 *)vxAllocateAndZeroMemory(reorderStreamSize);

    if (!reorderStream)
    {
        vxError("fillinKernelBufferHuffman: OUT OF MEMORY");
        goto exit;
    }

    reorderTPKernelBufferHuffman(wb, weight_z, filter_count, total_weight_z, weight_format,
        reorderStream, weight_base_ptr, VX_NULL, VX_NULL);

    analysisTPStreamForHuffman(wb, sliceCount, filterCount, reorderStream, invSizeOrder, index);

    kernelBitSize += 16;
    kernelBitSize += 8 * MAX_RUNLEN_SIZE;
    kernelBitSize += 8 * LOG_RUN_SIZE1;
    kernelBitSize += 32;
    /*TP huffman update 14 bit to save kernel stream bit size*/
    kernelBitSize += 14 * sliceCount;

    kernelSize = (kernelBitSize % 8 == 0) ? kernelBitSize / 8 : kernelBitSize / 8 + 1;
    /*align to 64 byte*/
    kernelSize = (kernelSize + 63) & 0xFFFFFFC0;
    kernelBitSize = 0;

    biasSize = 4 * filterCount;
    /*align to 64 byte*/
    biasSize = (biasSize + 63) & 0xFFFFFFC0;
    kernelSize += biasSize;

    while(((wb->huffmanConfig[index].runLenTableSize - SET_1_SIZE)>>runSet2Bits) > 1)
    {
        runSet2Bits++;
    }

    kernelDataPtr = reorderStream;
    for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
    {
        vx_int32 run = 0x0;
        vx_uint32 dataSizeOfCore = filterCount;
        vx_uint32 dataRemainingOfCore = dataSizeOfCore;
        vx_int32 size = 0;
        //vx_int32 code = 0;
        vx_int32 coef16 = 0;
        vx_int32 k = 0, j = 0, m = 0;

        while(dataRemainingOfCore --)
        {
            if (weight_format == VX_TYPE_INT8)
            {
                coef = *((vx_int8 *)kernelDataPtr);
                if (coef != (vx_int32)skip_value)
                    nonZeroCoefCount++;
            }
            else if (weight_format == VX_TYPE_UINT8)
            {
                coef = *((vx_uint8 *)kernelDataPtr);
                if (coef != (vx_int32)skip_value)
                    nonZeroCoefCount++;
            }
            else if (weight_format == VX_TYPE_INT16 ||
                weight_format == VX_TYPE_FLOAT16 ||
                weight_format == VX_TYPE_BFLOAT16
                )
            {
                coef16 = *((vx_int16 *)kernelDataPtr);
                if (coef16 != (vx_int32)skip_value)
                    nonZeroCoefCount++;
            }
            else
            {
                /*Other format not suppoted now*/
                vxmASSERT(0);
            }
            kernelDataPtr += weightSize;

            if (bit16Flag == 0)
            {

                coef = (coef - wb->huffmanConfig[index].avgBias) & 0xFF;
                if (wb->huffmanConfig[index].preEncode)
                {
                    tmp = coef;
                    coef = (coef - prev) & 0xFF;
                    prev = tmp;
                }
            }
            else
            {

                if (wb->huffmanConfig[index].fp16Flag)
                {
                    coef16 = (coef16 & 0x7FFF) * 2 + coef16 / (1<<15);
                }
                coef16 -= wb->huffmanConfig[index].avgBias;
                if (wb->huffmanConfig[index].runLenTableSize == 0x0)
                    coef = (coef16 >> 8) & 0xFF; /*Non RZL*/
                else
                    coef = coef16 & 0xFFFF; /*RZL path*/
            }

            if (wb->huffmanConfig[index].runLenTableSize == 0x0) /* non RZL*/
            {
                tmp = (coef & 0x80)? (coef ^ 0xff): coef;
                for (size = 0; size<7; size++)
                {
                    if (tmp<(1<<size))
                        break;
                }
                k = invSizeOrder[size];
                kernelBitSize += 3;

                if (sizeCodeLen[k] > 3)
                {
                    kernelBitSize += 2;
                }

                if (size == 0) size = 1; /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                if (sizeCodeLen[k]%2 == 0)
                {
                    kernelBitSize += (size - 1);
                }
                else
                {
                    kernelBitSize += size;
                }

                if (wb->huffmanConfig[index].bit16Flag)
                {/*pack the raw data of the lower 8 bits*/
                    kernelBitSize += 8;
                }
                rsvCoefCount++;
            }
            else /* RZL enable*/
            {
                if (coef == 0)
                {
                    run++;
                    if (dataRemainingOfCore != 0)
                    {
                        continue;
                    }
                }
                if (coef || (dataRemainingOfCore == 0)) /* if last pixel, force process run zeros*/
                {
                    /* process run zeros*/
                    while(run)
                    {
                        for (j = wb->huffmanConfig[index].runLenTableSize - 1; j>=0; j--)
                        {
                            if (run >= bestRunsSorted[j] + 1)
                            {
                                k = run / (bestRunsSorted[j] + 1);
                                run %= (bestRunsSorted[j] + 1);
                                break;
                            }
                        }
                        if (j >= 0)
                        {
                            vx_int32 n = bestRunsSorted[j];
                            size = 8;
                            for (m = 0; m<SET_1_SIZE; m++)
                            {
                                if (best2Runs[m] == n)
                                {
                                    size = 0;
                                    break;
                                }
                            }
                            if (size == 8)
                            {
                                for (m = 0; m < wb->huffmanConfig[index].runLenTableSize - SET_1_SIZE; m++)
                                    if (outBest2Run[m] == n)
                                        break;

                            }
                            j = invSizeOrder[size];
                            while(k--)
                            {/*k of the runs*/
                                kernelBitSize += 3;

                                if (sizeCodeLen[j] > 3)
                                {
                                    kernelBitSize += 2;
                                }
                                if (size == 0 )
                                {/*The m already code*/
                                    kernelBitSize += ((sizeCodeLen[j]%2 == 0) ? 0:1);
                                }
                                else if (sizeCodeLen[j]%2 == 0)
                                {
                                    kernelBitSize += (runSet2Bits - 1);
                                }
                                else
                                {
                                    kernelBitSize += runSet2Bits;
                                }
                                rsvCoefCount++;
                            }
                        }
                    }
                    /*process current pixel*/
                    if (coef)
                    {
                        vx_int8 ch;
                        coef>>=(wb->huffmanConfig[index].bit16Flag * 8);
                        if(coef<0x80)
                            coef+=wb->huffmanConfig[index].bit16Flag; /*Since we have to encode zero (run-length only handle 16-bits zero, 8 bit zero should use Size code*/
                        ch = (vx_int8)coef;
                        coef = abs(ch);
                        for (j = 6; j>=0; j--)
                        {
                            if (coef>=(1<<j))
                            {
                                break;
                            }
                        }
                        size = j+1;
                        if (invSizeOrder[size]==8)
                        { /*The minimum freq, we out the raw data*/
                            size = 7;
                        }

                        j = invSizeOrder[size];
                        kernelBitSize += 3;

                        if (sizeCodeLen[j] > 3)
                        {
                            kernelBitSize += 2;
                        }

                        if (size == 7)
                        {
                            size = 8;
                        }
                        /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                        if (size == 0)
                            size = 1;
                        if (sizeCodeLen[j]%2 == 0)
                        {
                            kernelBitSize += (size - 1);
                        }
                        else
                        {
                            kernelBitSize += size;
                        }
                        if (wb->huffmanConfig[index].bit16Flag)
                        {
                            kernelBitSize += 8;
                        }
                        rsvCoefCount++;
                    }
                }
            }

            if(dataRemainingOfCore == 0)
            {
                /*Hit the end bitsteam of core, add dummy stage process the next*/
                kernelBitSize += 3;
                kernelBitSize += 2;
                kernelBitSize += 8;

                if (wb->huffmanConfig[index].bit16Flag)
                    kernelBitSize += 8;

                for (j = 0; j < 2*THROUGHPUT; j++)
                {
                    kernelBitSize += 8;
                }
                break;
            }
        }

        prev = 0;
        /* Go back to update kernelStreamSize. */
        kernelStreamSize = (kernelBitSize % 8 == 0) ? kernelBitSize / 8 : kernelBitSize / 8 + 1;
        /*align to 64 byte*/
        kernelStreamSize = (kernelStreamSize + 63) & 0xFFFFFFC0;
        kernelBitSize = 0;
        kernelSize += kernelStreamSize;
    }
    wb->slice_array[index].kernel_stream_size = kernelSize;
    wb->slice_array[index].non_zero_count = nonZeroCoefCount;
    wb->slice_array[index].reserve_weight_count = rsvCoefCount;
exit:
    if (reorderStream)
        vxFree(reorderStream);

    return;
}

void fillinTPKernelBufferHuffman(
    vx_weights_biases_parameter wb,
    vx_uint8* zero_run_len_bit_width,
    vx_uint32 weight_z,
    vx_uint32 filter_count,
    vx_uint32 total_weight_z,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_uint32 skip_value,
    vx_uint8_ptr wb_base_ptr,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_uint32 index
    )
{
    vx_uint32 invSizeOrder[MAX_SIZE_HISTO_COUNT] = {0};
    vx_int32 coef = 0, tmp = 0, prev = 0;

    vx_uint32 weightSize       = (vx_uint32)vxDataType_GetSize((vx_type_e)weight_format);
    vx_uint32 biasBitSize      = NN_INTEGER_BIAS_BITS_VIP_V7;
    vx_uint32 biasData         = 0;

    vx_uint32 sliceCount            = weight_z;
    vx_uint32 filterCount           = filter_count;
    vx_uint32 kernelStreamSize      = 1;
    vx_uint32 kernelSizePerCore     = 0;
    vx_uint32* kernelBufferPtr      = (vx_uint32*) wb_base_ptr;
    vx_uint8* kernelStartPtr        = wb_base_ptr;
    vx_uint32 bitOffset             = 0;
    vx_uint32* kernelStreamSizePtr  = kernelBufferPtr;
    vx_uint32 streamSizeBitOffset   = 0;
    vx_uint32 alignedOffset         = ((vx_uint32)(gctUINTPTR_T)kernelBufferPtr) & 0x3F;
    vx_uint8_ptr kernelDataPtr      = VX_NULL;
    vx_uint8_ptr kernelStreamBasePtr = VX_NULL;

    vx_uint32 i;
    vx_uint32 sliceIndex = 0;
    vx_uint32 filterIndex = 0;

    vx_uint32 bit16Flag = (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8) ? 0 : 1;

    vx_uint32 reorderStreamAllCount = sliceCount * filterCount;
    vx_uint32 reorderStreamSize = reorderStreamAllCount * weightSize;

    vx_uint8* reorderStream = VX_NULL;
    vx_uint32 runSet2Bits = 1;
    CodeSymbol codeSymbol[THROUGHPUT * 3];

    vx_uint32 dummyStage[3] = {0};
    vx_uint32 dummyBitLength[3] = {0};
    vx_bool setDummy = vx_false_e;

    reorderStream = (vx_uint8 *)vxAllocateAndZeroMemory(reorderStreamSize);

    if (!reorderStream)
    {
        vxError("fillinKernelBufferHuffman: OUT OF MEMORY");
        goto exit;
    }

    reorderTPKernelBufferHuffman(wb, weight_z, filter_count, total_weight_z, weight_format,
        reorderStream, weight_base_ptr, VX_NULL, VX_NULL);

    analysisTPStreamForHuffman(wb, sliceCount, filterCount, reorderStream, invSizeOrder, index);

    /* Write huffman header*/
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].preEncode, 1);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].bit16Flag, 1);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].fp16Flag, 1);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].reserved, 1);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].version, 4);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].runLenTableSize, 8);

    for(i = 0; i < MAX_RUNLEN_SIZE; i++)
    {
        writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].runLenTable[i], 8);
    }
    for(i = 0; i < LOG_RUN_SIZE1; i++)
    {
        writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].mapToHuffman[i], 8);
    }
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].avgBias, 16);
    writeBits(&kernelBufferPtr, &bitOffset, 0, 16); /*reserved, must zero*/

    kernelStreamSizePtr = kernelBufferPtr;
    streamSizeBitOffset = bitOffset;

    /* Fill kernel stream size for each slice. */
    for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
    {
        writeBits(&kernelBufferPtr, &bitOffset, kernelStreamSize, 14);
    }
    packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);

    /* Fill bias for each filter. */
    for (filterIndex = 0; filterIndex < filterCount; filterIndex++)
    {
        if(vxoContext_IsFeatureAvailable(wb->base.context, VX_TP_FEATURE_FP32_BIAS))
        {
            vx_float32 biasFloat32;
           if(bias_format == VX_TYPE_INT64)
            {
                vx_int64  biasData64bits         = 0;
                biasBitSize = biasBitSize >= 32 ? 32: biasBitSize;
                biasData64bits = bias_base_ptr == VX_NULL ? 0 : *((vx_int64 *)bias_base_ptr + filterIndex);
                biasFloat32 = Int64toFp32(biasData64bits, wb->wb_base->biases_fixed_point_pos);
            }
            else
            {
                biasData = bias_base_ptr == VX_NULL ? 0 : *(bias_base_ptr + filterIndex);
                biasFloat32 = Int32toFp32(biasData, wb->wb_base->biases_fixed_point_pos);
            }
           writeBits(&kernelBufferPtr, &bitOffset, *(vx_uint32 *)&biasFloat32, biasBitSize);
        }
        else
        {
            if(bias_format == VX_TYPE_INT64)
            {
                vx_int32 bias32;
                vx_int64  biasData64bits         = 0;
                biasBitSize = biasBitSize >= 32 ? 32: biasBitSize;
                biasData64bits = bias_base_ptr == VX_NULL ? 0 : *((vx_int64 *)bias_base_ptr + filterIndex);
                bias32 = (biasData64bits >>16) & 0xFFFFFFFF;
                writeBits(&kernelBufferPtr, &bitOffset, bias32, biasBitSize);
            }
            else
            {
                biasData = bias_base_ptr == VX_NULL ? 0 : *(bias_base_ptr + filterIndex);
                writeBits(&kernelBufferPtr, &bitOffset, biasData, biasBitSize);
            }
        }
    }
    packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);


    for (i = 0; i < THROUGHPUT * 3; i++)
    {
        codeSymbol[i].stageCode[0] = codeSymbol[i].stageCode[1] = codeSymbol[i].stageCode[2] = 0;
        codeSymbol[i].bitLength[0] = codeSymbol[i].bitLength[1] = codeSymbol[i].bitLength[2] = 0;
    }

    while(((wb->huffmanConfig[index].runLenTableSize - SET_1_SIZE)>>runSet2Bits) > 1)
    {
        runSet2Bits++;
    }

    kernelDataPtr = reorderStream;
    for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
    {

        vx_uint32 x = 0x0;
        vx_int32 pos = 0x0;
        vx_int32 run = 0x0;
        vx_uint32 dataSizeOfCore = filterCount;
        vx_uint32 dataRemainingOfCore = dataSizeOfCore;
        vx_int32 size = 0;
        vx_int32 code = 0;
        vx_int32 coef16 = 0;
        vx_int32 k = 0, j = 0, m = 0;

        /* update position of each core's beginning of compressed kernel*/
        kernelStreamBasePtr = (vx_uint8*)kernelBufferPtr;

        memset(codeSymbol, 0, sizeof(CodeSymbol));
        memset(dummyStage, 0, (sizeof(vx_uint32) * 3));
        memset(dummyBitLength, 0, (sizeof(vx_uint32) * 3));
        setDummy = vx_false_e;

        while(dataRemainingOfCore --)
        {

            pos = (x) % (3*THROUGHPUT);

            if (weight_format == VX_TYPE_INT8)
                coef = *((vx_int8 *)kernelDataPtr);
            else if (weight_format == VX_TYPE_UINT8)
                coef = *((vx_uint8 *)kernelDataPtr);
            else if (weight_format == VX_TYPE_INT16 ||
                weight_format == VX_TYPE_FLOAT16 ||
                weight_format == VX_TYPE_BFLOAT16
                )
                coef16 = *((vx_int16 *)kernelDataPtr);
            else
            {
                /*Other format not suppoted now*/
                vxmASSERT(0);
            }
            kernelDataPtr += weightSize;

            if (bit16Flag == 0)
            {

                coef = (coef - wb->huffmanConfig[index].avgBias) & 0xFF;
                if (wb->huffmanConfig[index].preEncode)
                {
                    tmp = coef;
                    coef = (coef - prev) & 0xFF;
                    prev = tmp;
                }
            }
            else
            {
                if (wb->huffmanConfig[index].fp16Flag)
                {
                    coef16 = (coef16 & 0x7FFF) * 2 + coef16 / (1<<15);
                }
                coef16 -= wb->huffmanConfig[index].avgBias;
                if (wb->huffmanConfig[index].runLenTableSize == 0x0)
                    coef = (coef16 >> 8) & 0xFF; /*Non RZL*/
                else
                    coef = coef16 & 0xFFFF; /*RZL path*/
            }

            if (wb->huffmanConfig[index].runLenTableSize == 0x0) /* non RZL*/
            {
                if (!setDummy)
                {
                    /*Set dummy all to 0,
                    in nonZRL path 8-bit, 0 belong to group0, -1~0, residue bit length is 1, residue code = 0*/
                    size = 0;
                    k = invSizeOrder[size];
                    code = huffCode[k];

                    dummyStage[0] = code & 0x07;
                    dummyBitLength[0] = 3;

                    if (sizeCodeLen[k] > 3)
                    {
                        dummyStage[1] = code >> 3;
                        dummyBitLength[1] = 2;
                    }

                    dummyStage[2] = 0;
                    if (size == 0) size = 1; /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                    if (sizeCodeLen[k]%2 == 0)
                    {
                        dummyBitLength[2] = size - 1;
                    }
                    else
                    {
                        dummyBitLength[2] = size;
                    }

                    if (wb->huffmanConfig[index].bit16Flag)
                    {/*pack the raw data of the lower 8 bits*/

                        dummyBitLength[2] += 8;
                    }
                    setDummy = vx_true_e;
                }

                tmp = (coef & 0x80)? (coef ^ 0xff): coef;
                for (size = 0; size<7; size++)
                {
                    if (tmp<(1<<size))
                        break;
                }
                k = invSizeOrder[size];
                code = huffCode[k];

                if (sizeCodeLen[k]%2 == 0)/*huffCode length = 2 or 4, pack 1 bit to 3 or 5*/
                    code |= ((coef>>7)<<(sizeCodeLen[k]) );
                codeSymbol[pos].stageCode[0] = code & 0x07;
                codeSymbol[pos].bitLength[0] = 3;

                if (sizeCodeLen[k] > 3)
                {
                    codeSymbol[pos].stageCode[1] = code >> 3;
                    codeSymbol[pos].bitLength[1] = 2;
                }

                codeSymbol[pos].stageCode[2] = tmp &((1<<size) - 1 );
                if (size == 0) size = 1; /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                if (sizeCodeLen[k]%2 == 0)
                {
                    codeSymbol[pos].bitLength[2] = size - 1;
                }
                else
                {
                    codeSymbol[pos].stageCode[2] <<= 1;
                    codeSymbol[pos].stageCode[2] |= (coef>>7); /*The sign bit*/
                    codeSymbol[pos].bitLength[2] = size;
                }

                if (wb->huffmanConfig[index].bit16Flag)
                {/*pack the raw data of the lower 8 bits*/

                    codeSymbol[pos].stageCode[2] = codeSymbol[x%(3*THROUGHPUT)].stageCode[2]* 256 + (coef16 &0xff);
                    codeSymbol[pos].bitLength[2] += 8;
                }
                OutputAt(x, &kernelBufferPtr, &bitOffset, codeSymbol);
                x++; /*counter*/
            }
            else /* RZL enable*/
            {
                if (!setDummy)
                {
                    /*Set dummy all to 0,
                    in ZRL path 8-bit, 0 belong to group7, -128~127, residue bit length is 8, residue code = 0*/
                    size = 7;
                    j = invSizeOrder[size];
                    code = huffCode[j];

                    dummyStage[0] = code & 0x07;
                    dummyBitLength[0] = 3;

                    if (sizeCodeLen[j] > 3)
                    {
                        dummyStage[1] = code >> 3;
                        dummyBitLength[1] = 2;
                    }

                    if (size == 7)
                    {
                        size = 8;
                    }

                    if (sizeCodeLen[j] % 2 == 0)
                    {
                        dummyStage[2] = 0;  /*last bit was coded*/
                        dummyBitLength[2] = size - 1;
                    }
                    else
                    {
                        dummyStage[2] = 0;
                        dummyBitLength[2] = size;
                    }
                    if (wb->huffmanConfig[index].bit16Flag)
                    {
                        dummyStage[2] = 0;
                        dummyBitLength[2] += 8;
                    }

                    setDummy = vx_true_e;
                }

                if (coef == 0)
                {
                    run++;
                    if (dataRemainingOfCore != 0)
                    {
                        continue;
                    }
                }
                if (coef || (dataRemainingOfCore == 0)) /* if last pixel, force process run zeros*/
                {
                    /* process run zeros*/
                    while(run)
                    {
                        for (j = wb->huffmanConfig[index].runLenTableSize - 1; j>=0; j--)
                        {
                            if (run >= bestRunsSorted[j] + 1)
                            {
                                k = run / (bestRunsSorted[j] + 1);
                                run %= (bestRunsSorted[j] + 1);
                                break;
                            }
                        }
                        if (j >= 0)
                        {
                            vx_int32 n = bestRunsSorted[j];
                            size = 8;
                            for (m = 0; m<SET_1_SIZE; m++)
                            {
                                if (best2Runs[m] == n)
                                {
                                    size = 0;
                                    break;
                                }
                            }
                            if (size == 8)
                            {
                                for (m = 0; m < wb->huffmanConfig[index].runLenTableSize - SET_1_SIZE; m++)
                                    if (outBest2Run[m] == n)
                                        break;

                            }
                            j = invSizeOrder[size];
                            while(k--){/*k of the runs*/
                                code = huffCode[j];
                                if (sizeCodeLen[j] % 2 == 0)/*huffCode length = 2 or 4, pack 1 bit to 3 or 5*/
                                    code |= ((m&1)<<(sizeCodeLen[j]) );
                                codeSymbol[pos].stageCode[0] = code & 0x07;
                                codeSymbol[pos].bitLength[0] = 3;

                                if (sizeCodeLen[j] > 3)
                                {
                                    codeSymbol[pos].stageCode[1] = code >> 3;
                                    codeSymbol[pos].bitLength[1] = 2;
                                }
                                if (size == 0 )
                                {/*The m already code*/
                                    codeSymbol[pos].bitLength[2] = (sizeCodeLen[j]%2 == 0) ? 0:1;
                                    codeSymbol[pos].stageCode[2] = m; /*If (sizeCodeLen[j]%2 == 0), should be m/2, but 0 bit output, doesn't matter*/
                                }
                                else if (sizeCodeLen[j]%2 == 0)
                                {
                                    codeSymbol[pos].stageCode[2] = m/2;
                                    codeSymbol[pos].bitLength[2] = runSet2Bits - 1;
                                }
                                else
                                {
                                    codeSymbol[pos].stageCode[2] = m;
                                    codeSymbol[pos].bitLength[2] = runSet2Bits;
                                }

                                OutputAt(x, &kernelBufferPtr, &bitOffset, codeSymbol);
                                x++;
                                /* update pos*/
                                pos = (x) % (3*THROUGHPUT);
                            }
                        }
                    }
                    /*process current pixel*/
                    if (coef)
                    {
                        vx_int8 ch;
                        coef>>=(wb->huffmanConfig[index].bit16Flag * 8);
                        if(coef<0x80)
                            coef+=wb->huffmanConfig[index].bit16Flag; /*Since we have to encode zero (run-length only handle 16-bits zero, 8 bit zero should use Size code*/
                        ch = (vx_int8)coef;
                        coef = abs(ch);
                        m = coef*2 + (ch<0?1:0);
                        for (j = 6; j>=0; j--)
                        {
                            if (coef>=(1<<j))
                            {
                                break;
                            }
                        }
                        size = j+1;
                        if (invSizeOrder[size]==8)
                        { /*The minimum freq, we out the raw data*/
                            size = 7;
                        }
                        if (size == 7)
                        {
                            m = ch&0xff;
                            if(wb->huffmanConfig[index].bit16Flag)
                                m = (coef16 >> 8) & 0xff;
                        }

                        j = invSizeOrder[size];
                        code = huffCode[j];
                        if (sizeCodeLen[j]%2 == 0)/*huffCode length = 2 or 4, pack 1 bit to 3 or 5*/
                            code |= ((m&1) << (sizeCodeLen[j]));
                        codeSymbol[pos].stageCode[0] = code & 0x07;
                        codeSymbol[pos].bitLength[0] = 3;

                        if (sizeCodeLen[j] > 3)
                        {
                            codeSymbol[pos].stageCode[1] = code >> 3;
                            codeSymbol[pos].bitLength[1] = 2;
                        }

                        if (size == 7)
                        {
                            size = 8;
                        }
                        /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                        if (size == 0)
                            size = 1;
                        if (sizeCodeLen[j]%2 == 0)
                        {
                            codeSymbol[pos].stageCode[2] = m/2;  /*last bit was coded*/
                            codeSymbol[pos].bitLength[2] = size - 1;
                        }
                        else
                        {
                            codeSymbol[pos].stageCode[2] = m;
                            codeSymbol[pos].bitLength[2] = size;
                        }
                        if (wb->huffmanConfig[index].bit16Flag)
                        {
                            codeSymbol[pos].stageCode[2] = codeSymbol[x%(3*THROUGHPUT)].stageCode[2]* 256 + (coef16 & 0xff);
                            codeSymbol[pos].bitLength[2] += 8;
                        }
                        OutputAt(x, &kernelBufferPtr, &bitOffset, codeSymbol);
                        x++; /*counter*/
                    }

                }
            }

            if(dataRemainingOfCore == 0)
            {
                vx_uint32 dummyCount = (x & 0x1)? 5: 4;
                /*Hit the end bitsteam of core, add dummy stage process the next*/
                vxmASSERT(setDummy == vx_true_e);
                addDummy(x, &kernelBufferPtr, &bitOffset, codeSymbol, dummyCount, dummyStage, dummyBitLength);
                break;
            }
        }

        kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);
        /*There's 14 bit to save kernel stream bit size*/
        gcmASSERT((kernelStreamSize*8 + bitOffset) < 16384);
        /* Go back to update kernelStreamSize. */
        writeBits(&kernelStreamSizePtr, &streamSizeBitOffset, kernelStreamSize*8 + bitOffset, 14);
        /*align to 64 byte after each kz*/
        packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);
        prev = 0;

    }
    kernelSizePerCore = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStartPtr);
    if (!(kernelSizePerCore <= wb->slice_array[index].kernel_stream_size))
    {
        vxmASSERT(0);
    }

exit:
    if (reorderStream)
        vxFree(reorderStream);

    return;
}

void calculateWeightBiasStreamRelatedSize(
    vx_context context,
    vx_weights_biases_parameter wb,
    gctPOINTER weight_data,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 kernels_per_core,
    vx_enum weight_format,
    vx_enum bias_format,
    vx_uint32 skip_value,
    vx_int8  set_zrl,
    vx_uint8 option_zero_run_len,
    vx_uint32 index,
    vx_size*   min_kernel_buf_size,
    vx_uint8*  min_zero_run_len,
    vx_uint32* max_zero_run_len
    )
{
    vx_uint8 minZeroRunLen = 0, zeroRunLen = option_zero_run_len;
    vx_size minKernelBufferSize = ~0UL;
    vx_uint32 weightSize = (vx_uint32)vxDataType_GetSize((vx_type_e)weight_format);
    vx_uint32 biasSize = (vx_uint32)vxDataType_GetSize((vx_type_e)bias_format);

    if (weight_format == VX_TYPE_INT16)
        biasSize = 3;

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP0))
    {
        vx_uint32 nnCoreCount = (weight_format == VX_TYPE_INT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : (weight_format == VX_TYPE_FLOAT16) ?
                      context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;
        vx_uint32 filterPerCore = (z_count % nnCoreCount == 0) ? z_count / nnCoreCount : z_count / nnCoreCount + 1;
        /*VIP V8*/
        minKernelBufferSize = calcKernelSizeV8Huffman(
                                context,
                                wb,
                                wb->weights_sizes[0],
                                wb->weights_sizes[1],
                                wb->weights_sizes[2],
                                wb->weights_sizes[3],
                                filterPerCore,
                                weight_format,
                                skip_value,
                                (vx_uint8_ptr)weight_data,
                                VX_NULL,
                                VX_NULL,
                                VX_NULL,
                                VX_NULL,
                                VX_NULL,
                                index
                                );

        wb->slice_array[index].non_zero_count = calcNonZeroCountV8Huffman(context, wb, slice_count, z_count, kernels_per_core, weight_format, (vx_uint8_ptr)weight_data, skip_value);
        wb->slice_array[index].kernel_stream_size = minKernelBufferSize;
        wb->slice_array[index].all_count = wb->weights_sizes[0] * wb->weights_sizes[1] * wb->weights_sizes[2] * wb->weights_sizes[3];
        wb->slice_array[index].kernel_orig_size = wb->slice_array[index].all_count * weightSize + z_count * biasSize;
    }
    else if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_COEF_COMPRESSION_ENHANCEMENT))
    {
        /*V7 Huffman*/
        minKernelBufferSize = calcKernelStreamSizeHuffman(
                                context,
                                wb,
                                wb->weights_sizes[0],
                                wb->weights_sizes[1],
                                wb->weights_sizes[2],
                                wb->weights_sizes[3],
                                kernels_per_core,
                                weight_format,
                                bias_format,
                                wb->wb_base->inputZP,
                                wb->wb_base->coefZP,
                                skip_value,
                                1,
                                1,
                                -1,
                                vxDataType_GetSize((vx_type_e)weight_format),
                                (vx_uint8_ptr)weight_data,
                                VX_NULL,
                                index
                                );
        wb->slice_array[index].kernel_stream_size = minKernelBufferSize;

        /*Only use old path to calculate nonZeroRatio & origKenelSize*/
        calculateWeightBiasBufferSizeForZeroRunLen(context, wb, slice_count, z_count, kernels_per_core, weight_format, bias_format, skip_value, 0, weight_data, index);
    }
    else if (context->options.enableNonZeroBalance)
    {
        if ((wb->weights_sizes[0] == 1 && wb->weights_sizes[1] == 1
            && (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3) || vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6))
            && (wb->wb_base->weights_data_format == VX_TYPE_INT8 || wb->wb_base->weights_data_format == VX_TYPE_UINT8))
            && !gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_ZDP3_NO_COMPRESS_FIX))
        {
            /* Per HW ZDP bug, ZDP3 & ZDP6 zero length should always be 0*/
            minKernelBufferSize = calculateWeightBiasBalanceSizeForZeroRunLen(context, wb, slice_count, z_count, kernels_per_core, weight_format, bias_format, skip_value, 0, weight_data, index);
            minZeroRunLen = 0;
            wb->slice_array[index].kernel_stream_size = minKernelBufferSize;
        }
        else if (set_zrl >= 0)
        {
            minKernelBufferSize = calculateWeightBiasBalanceSizeForZeroRunLen(context, wb, slice_count, z_count, kernels_per_core, weight_format, bias_format, skip_value, set_zrl, weight_data, index);
            minZeroRunLen = set_zrl;
            wb->slice_array[index].kernel_stream_size = minKernelBufferSize;
        }
        else if (zeroRunLen > 5)
        {
            if (!calculateWeightBiasBalanceSizeForZeroRunLenEx(context, wb, slice_count, z_count, kernels_per_core, weight_format, bias_format, skip_value, weight_data, index, &minKernelBufferSize, &minZeroRunLen))
            {
                vx_uint32 minRsvCount = wb->slice_array[index].reserve_weight_count;

                /* Back to calulate kernelBufferSize for other zeroRunLen. */
                for (zeroRunLen = 0; zeroRunLen <= context->nnConfig.fixedFeature.zrlBits; zeroRunLen++)
                {
                    vx_size kernelBufferSize = calculateWeightBiasBalanceSizeForZeroRunLen(context, wb, slice_count, z_count, kernels_per_core, weight_format, bias_format, skip_value, zeroRunLen, weight_data, index);
                    if (kernelBufferSize < minKernelBufferSize)
                    {
                        minKernelBufferSize = kernelBufferSize;
                        minZeroRunLen = zeroRunLen;
                        minRsvCount = wb->slice_array[index].reserve_weight_count;
                    }
                }

                wb->slice_array[index].reserve_weight_count = minRsvCount;
                wb->slice_array[index].kernel_stream_size = minKernelBufferSize;
            }
        }
        else
        {
            minKernelBufferSize = calculateWeightBiasBalanceSizeForZeroRunLen(context, wb, slice_count, z_count, kernels_per_core, weight_format, bias_format, skip_value, zeroRunLen, weight_data, index);
            minZeroRunLen = zeroRunLen;
            wb->slice_array[index].kernel_stream_size = minKernelBufferSize;
        }
    }
    else
    {
        if ((wb->weights_sizes[0] == 1 && wb->weights_sizes[1] == 1
            && (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3) || vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6))
            && (wb->wb_base->weights_data_format == VX_TYPE_INT8 || wb->wb_base->weights_data_format == VX_TYPE_UINT8))
            && !gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_ZDP3_NO_COMPRESS_FIX))
        {
            /* Per HW ZDP bug, ZDP3 & ZDP6 zero length should always be 0*/
            minKernelBufferSize = calculateWeightBiasBufferSizeForZeroRunLen(context, wb, slice_count, z_count, kernels_per_core, weight_format, bias_format, skip_value, 0, weight_data, index);
            minZeroRunLen = 0;
            wb->slice_array[index].kernel_stream_size = minKernelBufferSize;
        }
        else if (set_zrl >= 0)
        {
            minKernelBufferSize = calculateWeightBiasBufferSizeForZeroRunLen(context, wb, slice_count, z_count, kernels_per_core, weight_format, bias_format, skip_value, set_zrl, weight_data, index);
            minZeroRunLen = set_zrl;
            wb->slice_array[index].kernel_stream_size = minKernelBufferSize;
        }
        else if (zeroRunLen > 5)
        {
            if (!calculateWeightBiasBufferSizeForZeroRunLenEx(context, wb, slice_count, z_count, kernels_per_core, weight_format, bias_format, skip_value, weight_data, index, &minKernelBufferSize, &minZeroRunLen))
            {
                vx_uint32 minRsvCount = wb->slice_array[index].reserve_weight_count;

                /* Back to calulate kernelBufferSize for other zeroRunLen. */
                for (zeroRunLen = 0; zeroRunLen <= context->nnConfig.fixedFeature.zrlBits; zeroRunLen++)
                {
                    vx_size kernelBufferSize = calculateWeightBiasBufferSizeForZeroRunLen(context, wb, slice_count, z_count, kernels_per_core, weight_format, bias_format, skip_value, zeroRunLen, weight_data, index);
                    if (kernelBufferSize < minKernelBufferSize)
                    {
                        minKernelBufferSize = kernelBufferSize;
                        minZeroRunLen = zeroRunLen;
                        minRsvCount = wb->slice_array[index].reserve_weight_count;
                    }
                }

                wb->slice_array[index].reserve_weight_count = minRsvCount;
                wb->slice_array[index].kernel_stream_size = minKernelBufferSize;
            }
        }
        else
        {
            minKernelBufferSize = calculateWeightBiasBufferSizeForZeroRunLen(context, wb, slice_count, z_count, kernels_per_core, weight_format, bias_format, skip_value, zeroRunLen, weight_data, index);
            minZeroRunLen = zeroRunLen;
            wb->slice_array[index].kernel_stream_size = minKernelBufferSize;
        }
    }
    if (min_kernel_buf_size != VX_NULL)
        *min_kernel_buf_size = minKernelBufferSize;
    if (min_zero_run_len != VX_NULL)
        *min_zero_run_len = minZeroRunLen;
    if (max_zero_run_len != VX_NULL)
        *max_zero_run_len = (1 << minZeroRunLen) - 1;
}

vx_size estimateWeightBiasStreamSize(
    vx_weights_biases_parameter wb,
    gctPOINTER weight_data
    )
{
    vx_size kernelStreamSize = 0;
    vx_context context = vxGetContext((vx_reference)wb);
    vx_uint32 sliceCount = wb->wb_base->weights_sizes[2];
    vx_uint32 filterCount = wb->wb_base->weights_sizes[3];
    vx_enum weightFormat = wb->wb_base->weights_data_format;
    vx_enum biasFormat = wb->wb_base->biases_data_format;
    vx_uint32 nnCoreCount = (weightFormat == VX_TYPE_INT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : (weightFormat == VX_TYPE_FLOAT16) ?
                      context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;
    vx_uint32 filterPerCore = (filterCount % nnCoreCount == 0) ? filterCount / nnCoreCount : filterCount / nnCoreCount + 1;

    calculateWeightBiasStreamRelatedSize(
            context,
            wb,
            weight_data,
            sliceCount, /* slice */
            filterCount, /* z count */
            filterPerCore, /* kernel per core */
            weightFormat,
            biasFormat,
            wb->wb_base->skipValue,
            /*wb->weights_sizes[2] <= 1 ? 0 : */wb->wb_base->setZeroLength,
            (vx_uint8)context->options.nnZeroRunLen,
            0,
            &kernelStreamSize, VX_NULL, VX_NULL);

    return kernelStreamSize;
}

vx_float32 calculateWeightNonZeroRatio(
    vx_context context,
    vx_uint32 skip_value,
    vx_tensor weights
    )
{
    vx_enum weightFomat = TENSOR_DATA_TYPE(weights);
    vx_uint32 skipValue = skip_value;

    gctPOINTER weightData = VX_NULL;

    vx_uint32 weightCount        = weights->dims[0] * weights->dims[1];
    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize((vx_type_e)weightFomat);

    vx_uint32 sliceCount         = weights->dims[2];
    vx_uint32 filterCount        = weights->dims[3];
    vx_uint32 filterSliceSize    = weightCount * weightSize;
    vx_uint32 filterSize         = weightCount * weightSize * sliceCount;
    vx_uint8* startDataPtr       = VX_NULL;
    vx_uint8* kernelDataPtr      = VX_NULL;

    vx_uint32 blockCount = 0, nonZeroCount = 0;
    vx_float32 nonZeroRatio = 0;

    vx_uint32 filterIndex, sliceIndex;
    vx_uint32 weightXIndex, weightYIndex;

    vx_bool hasVipV7Feature = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7) == gcvSTATUS_TRUE ? vx_true_e : vx_false_e;
    vx_bool hasZDP3 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3);
    vx_bool hasZDP6 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6);
    vx_bool hasXYDP9 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP9);
    vx_bool hasXYDP6 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP6);
    vx_uint32 filterStart = 0;
    vx_uint32 filterEnd = filterCount;


    vxoTensor_GetTensorViewMemory(weights, &weightData, VX_NULL);
    startDataPtr = (vx_uint8*)weightData;


    if (weights->dims[0] == 1 && weights->dims[1] == 1 &&
        (hasZDP3 || hasZDP6) &&
        (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8))
    {
        vx_uint32 zdpNum = hasZDP6 ? 6 : 3;

        /* zdp3 zdp6 can not enable same time */
        vxmASSERT(!(hasZDP3 && hasZDP6));

        for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex += (zdpNum * 2))
        {
            vx_uint32 weight = 0;
            vx_uint32 realSliceIndex = 0;

            /* add slices of every filter*/
            for (filterIndex = filterStart; filterIndex < filterEnd; filterIndex++)
            {
                vx_uint32 nonZeroInBlock1Count = 0; /*Check if there's non-zero point in first zdpNum block*/
                vx_uint32 nonZeroInBlock2Count = 0; /*Check if there's non-zero point in another zdpNum block*/
                for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + (zdpNum * 2); realSliceIndex++)
                {
                    if (realSliceIndex >= sliceCount)
                        break;

                    /* add one slice data every filter */
                    kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * realSliceIndex;
                    if (weightFomat == VX_TYPE_INT8)
                        weight = *((vx_int8 *)kernelDataPtr);
                    else if (weightFomat == VX_TYPE_UINT8)
                        weight = *((vx_uint8 *)kernelDataPtr);
                    else
                        weight = *((vx_uint16 *)kernelDataPtr);

                    if (realSliceIndex - sliceIndex < zdpNum && weight != skipValue)
                        nonZeroInBlock1Count++;
                    else if (weight != skipValue)
                        nonZeroInBlock2Count++;
                }
                if (sliceIndex + zdpNum >= sliceCount)
                    blockCount++;
                else
                    blockCount += 2;

                if (nonZeroInBlock1Count)
                    nonZeroCount++;
                if (nonZeroInBlock2Count)
                    nonZeroCount++;
            }
        }
    }
    else if ((hasXYDP9 || hasXYDP6)
                && (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8))
    {
        vx_uint32 xStep = 3;
        vx_uint32 yStep = 0;
        vx_uint32 inImageBufferSize = hasXYDP9 ? 2 : 1;

        if (hasXYDP6)
            yStep = 2;
        else
            yStep = 3;

        for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex += inImageBufferSize)
        {
            vx_uint32 weight = 0;
            vx_uint32 realSliceIndex = 0;
            vx_uint32 realWeightXIndex = 0, realWeightYIndex = 0;
            vx_uint32 subKxSize = 0, subKySize = 0, subKzSize = 0;

            subKzSize = gcmMIN((sliceCount - sliceIndex), inImageBufferSize);
            /* add slices of every filter*/
            for (filterIndex = filterStart; filterIndex < filterEnd; filterIndex++)
            {
                for (weightYIndex = 0; weightYIndex < weights->dims[1]; weightYIndex += yStep)
                {
                    subKySize = gcmMIN((weights->dims[1] - weightYIndex), yStep);
                    for (weightXIndex = 0; weightXIndex < weights->dims[0]; weightXIndex += xStep)
                    {
                        subKxSize = gcmMIN((weights->dims[0] - weightXIndex), xStep);
                        for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + subKzSize; realSliceIndex++)
                        {
                            vx_uint32 nonZeroInBlockCount = 0; /*Check if there's non-zero point in one 3x3 block*/

                            /* Add one slice data every filter. */
                            kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * realSliceIndex;

                            for (realWeightYIndex = weightYIndex; realWeightYIndex < weightYIndex + subKySize; realWeightYIndex++)
                            {
                                for (realWeightXIndex = weightXIndex; realWeightXIndex < weightXIndex + subKxSize; realWeightXIndex++)
                                {
                                    vx_uint8_ptr realKernelDataPtr = kernelDataPtr + (realWeightYIndex * weights->dims[1] + realWeightXIndex) * weightSize;

                                    if (weightFomat == VX_TYPE_INT8)
                                        weight = *((vx_int8 *)realKernelDataPtr);
                                    else if (weightFomat == VX_TYPE_UINT8)
                                        weight = *((vx_uint8 *)realKernelDataPtr);
                                    else
                                        weight = *((vx_uint16 *)realKernelDataPtr);

                                    if (weight != skipValue)
                                        nonZeroInBlockCount++;

                                }
                            }
                            blockCount++;
                            if (nonZeroInBlockCount)
                                nonZeroCount++;
                        }
                    }
                }
            }
        }
    }
    else
    {
        for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
        {
            /* Add slices of every filter. */
            for (filterIndex = filterStart; filterIndex < filterEnd; filterIndex++)
            {
                /* Add one slice data every filter. */
                kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * sliceIndex;

                for (weightYIndex = 0; weightYIndex < weights->dims[1]; weightYIndex++)
                {
                    vx_uint32 nonZeroCountInDP3 = 0; /*Check if there's non-zero point in one 3x1 block*/
                    for (weightXIndex = 0; weightXIndex < weights->dims[0]; weightXIndex++)
                    {
                        vx_uint32 weight;

                        if (weightFomat == VX_TYPE_INT8)
                            weight = *((vx_int8 *)kernelDataPtr);
                        else if (weightFomat == VX_TYPE_UINT8)
                            weight = *((vx_uint8 *)kernelDataPtr);
                        else
                            weight = *((vx_uint16 *)kernelDataPtr);
                        kernelDataPtr = kernelDataPtr + weightSize;

                        if(!hasVipV7Feature || weightFomat == VX_TYPE_FLOAT16)
                        {
                            /*V6 or FP16, DP1, a block is 1x1*/
                            blockCount++;
                            if (weight != skipValue)
                                nonZeroCount++;
                        }
                        else
                        {
                            if (weight != skipValue)
                                nonZeroCountInDP3++;

                            /*V7, DP3, one block is 3x1*/
                            if ((weightXIndex + 1) % 3 == 0 || weightXIndex == weights->dims[0] - 1)
                            {
                                blockCount++;
                                if (nonZeroCountInDP3)
                                    nonZeroCount++;
                                nonZeroCountInDP3 = 0;
                            }
                        }
                    }
                }
            }
        }
    }

    if (blockCount == 0)
    {
        vxmASSERT(0);
        vxError("%s: blockCount should not be zero.\n", __FUNCTION__);
        return 0;
    }

    if(blockCount != 0)
        nonZeroRatio = (vx_float32)nonZeroCount / (vx_float32)blockCount;

    return nonZeroRatio;
}

vx_size calculateTPWeightStreamSizeForZeroRunLen(
    vx_weights_biases_parameter wb,
    vx_uint32 slice_count,
    vx_uint32 filter_count,
    vx_uint32 total_slice_count,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_uint32 skip_value,
    vx_uint8 zrl,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32_ptr reserve_count,
    vx_uint32_ptr non_zero_count
    )
{
    vx_uint32 maxZeroRun        = (1 << zrl) - 1;
    vx_uint32 filterCount       = filter_count;
    vx_uint32 weightSize        = (vx_uint32)vxDataType_GetSize(weight_format);
    vx_uint32 weightBitSize     = weightSize * 8;
    vx_uint32 filterSize        = weightSize * total_slice_count;
    vx_uint8* kernelDataPtr     = weight_base_ptr;
    vx_uint32 skipValue         = skip_value;
    vx_size kernelBufferSize    = 0;
    vx_uint32 bitOffset         = 0;
    vx_uint32 zeroRun           = 0;
    vx_uint32 filterIndex;
    vx_uint32 rsvWeightCount = 0, nonZeroCount = 0;

    /* Fill zeroRunLenBitWidth. */
    bitOffset = 4;
    for (filterIndex = 0; filterIndex < filterCount; filterIndex++)
    {
        vx_uint32 weight = 0;

        if (WB_WEIGHT_DATA_FORMAT(wb) == VX_TYPE_INT8)
            weight = *((vx_int8 *)kernelDataPtr);
        else if (WB_WEIGHT_DATA_FORMAT(wb) == VX_TYPE_UINT8)
            weight = *((vx_uint8 *)kernelDataPtr);
        else
            weight = *((vx_uint16 *)kernelDataPtr);

        kernelDataPtr += filterSize;

        if ((zeroRun == maxZeroRun)
            || (weight != skipValue)
            || (filterIndex == (filterCount - 1)))
        {
            /* Write zeroRun and weight. */
            bitOffset += zrl + weightBitSize;
            if (bitOffset >= 32)
            {
                bitOffset -= 32;
                kernelBufferSize += 4;
            }
            zeroRun = 0;
            rsvWeightCount++;
        }
        else
        {
            zeroRun++;
        }
        if (weight != skip_value) nonZeroCount++;
    }
    /* pad 0 */
    if (bitOffset)
    {
        kernelBufferSize += 4;
    }
    kernelBufferSize = (kernelBufferSize + 63) & 0xFFFFFFC0;

    if (reserve_count != VX_NULL) *reserve_count = rsvWeightCount;
    if (non_zero_count != VX_NULL) *non_zero_count = nonZeroCount;

    return kernelBufferSize;
}

void calculateWeightBiasTPBufferRelatedSize(
    vx_weights_biases_parameter wb,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32 slice_count,
    vx_uint32 filter_count,
    vx_uint32 total_slice_count,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_uint32 skip_value,
    vx_int8 set_zrl,
    vx_int32 index,
    vx_size* min_kernel_buf_size,
    vx_uint8* min_zero_run_len
    )
{
    vx_uint32 weightSize            = (vx_uint32)vxDataType_GetSize(weight_format);
    vx_uint32 biasSize              = (vx_uint32)vxDataType_GetSize(bias_format);
    vx_uint32 sliceCount            = slice_count;
    vx_uint32 filterCount           = filter_count;
    vx_uint8_ptr kernelDataPtr      = VX_NULL;
    vx_size kernelBufferSize        = 0;
    vx_uint32 bitOffset             = 0;
    vx_int8 zrlBitWidth             = set_zrl;
    vx_uint32 filterIndex, sliceIndex;
    vx_uint8 minZrlBitWidth;
    vx_size minWeightStreamSize;
    vx_uint32 rsvWeightCount = 0, nonZeroCount = 0;
    vx_context context = vxGetContext((vx_reference)wb);

    if(bias_format == VX_TYPE_INT64)
        biasSize = 4;

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_COMPRESSION_ENHANCEMENT))
    {
        calcTPKernelBufferSizeHuffman(wb, sliceCount, filterCount, wb->wb_base->weights_sizes[2], weight_format, skip_value, weight_base_ptr, index);

        wb->slice_array[index].all_count = wb->weights_sizes[0] * wb->weights_sizes[1] * slice_count * filter_count;
        wb->slice_array[index].kernel_orig_size = slice_count * filter_count * weightSize +
                                                  filter_count * biasSize;

        *min_kernel_buf_size = wb->slice_array[index].kernel_stream_size;
    }
    else
    {
        /* Fill kernel stream size for each slice. */
        for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
        {
            bitOffset += 5;
            if (bitOffset >= 32)
            {
                bitOffset -= 32;
                kernelBufferSize += 4;
            }
        }
        /* pad 0 */
        if (bitOffset)
        {
            bitOffset = 0;
            kernelBufferSize += 4;
        }
        kernelBufferSize = (kernelBufferSize + 63) & 0xFFFFFFC0;

        /* Fill bias for each filter. */
        for (filterIndex = 0; filterIndex < filterCount; filterIndex++)
        {
            kernelBufferSize += biasSize;
        }
        /* pad 0 */
        kernelBufferSize = (kernelBufferSize + 63) & 0xFFFFFFC0;

        /* Fill weight value for every slice */
        if (zrlBitWidth >= 0 && zrlBitWidth <= 9)
        {
            for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
            {
                vx_uint32 rscount, nzcount;
                kernelDataPtr = weight_base_ptr + weightSize * sliceIndex;
                minWeightStreamSize = calculateTPWeightStreamSizeForZeroRunLen(wb, slice_count, filter_count, total_slice_count, weight_format, bias_format, skip_value, zrlBitWidth, kernelDataPtr, &rscount, &nzcount);
                kernelBufferSize += minWeightStreamSize;
                min_zero_run_len[sliceIndex] = zrlBitWidth;
                rsvWeightCount += rscount;
                nonZeroCount += nzcount;
            }
        }
        else
        {
            for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
            {
                vx_uint32 mrscount = 0, mnzcount = 0;

                kernelDataPtr = weight_base_ptr + weightSize * sliceIndex;
                /* Calculate the stream size of 0 zrlBitWidth. */
                minZrlBitWidth = 0;
                minWeightStreamSize = (1 + filterCount * weightSize + 63) & 0xFFFFFFC0;

                /* Calulate weightStreamSize for the rest zrlBitWidth. */
                for (zrlBitWidth = 1; zrlBitWidth <= 9; zrlBitWidth++)
                {
                    vx_uint32 rscount, nzcount;
                    vx_size weightStreamSize = calculateTPWeightStreamSizeForZeroRunLen(wb, slice_count, filter_count, total_slice_count, weight_format, bias_format, skip_value, zrlBitWidth, kernelDataPtr, &rscount, &nzcount);

                    if (!mrscount) mrscount = rscount;
                    if (!mnzcount) mnzcount = nzcount;
                    if (weightStreamSize < minWeightStreamSize)
                    {
                        minWeightStreamSize = weightStreamSize;
                        minZrlBitWidth = zrlBitWidth;
                        mrscount = rscount;
                        mnzcount = nzcount;
                    }
                }
                kernelBufferSize += minWeightStreamSize;
                min_zero_run_len[sliceIndex] = minZrlBitWidth;
                rsvWeightCount += mrscount;
                nonZeroCount += mnzcount;
            }
        }

        wb->slice_array[index].all_count = wb->weights_sizes[0] * wb->weights_sizes[1] * slice_count * filter_count;
        wb->slice_array[index].reserve_weight_count = rsvWeightCount;
        wb->slice_array[index].non_zero_count = nonZeroCount;
        wb->slice_array[index].kernel_stream_size = kernelBufferSize;
        wb->slice_array[index].kernel_orig_size = slice_count * filter_count * (vx_uint32)vxDataType_GetSize(weight_format) +
            filter_count * biasSize;

        *min_kernel_buf_size = kernelBufferSize;
    }
}

void fillinKernelBuffer(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint8 min_zero_run_len,
    vx_uint32 max_zero_run,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 weight_z,
    vx_uint32 filter_count,
    vx_uint32 kernels_per_core,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_int32 input_zp,
    vx_int32 coef_zp,
    vx_uint32 skip_value,
    vx_uint32 output_final_x,
    vx_uint32 output_final_y,
    vx_int32 z_offset,
    vx_uint32 outputSize,
    vx_uint8_ptr wb_base_ptr,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_uint32 index
    )
{
    vx_enum weightFomat = weight_format;
    vx_int32 inputZP = input_zp;
    vx_int32 coefZP = coef_zp;
    vx_uint32 skipValue = skip_value;

    vx_uint32 nnCoreCount = (weightFomat == VX_TYPE_INT16) ?
        context->nnConfig.fixedFeature.nnCoreCountInt16 : (weightFomat == VX_TYPE_FLOAT16) ?
            context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 coreIndex;
    vx_uint32 groupIndex, filterIndex, sliceIndex;
    vx_uint32 weightXIndex, weightYIndex;
    vx_uint32 bitOffset          = 0;
    vx_uint32 zeroRun            = 0;

    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize(weightFomat);
    vx_uint32 weightBitSize      = weightSize * 8;
    vx_uint32 biasSize           = (vx_uint32)vxDataType_GetSize(bias_format);
    vx_uint32 biasBitSize        = biasSize * 8;
    vx_uint32 biasData           = 0;

    vx_uint32 sliceCount         = weight_z;
    vx_uint32 filterSliceSize    = weight_x * weight_y * weightSize;
    vx_uint32 filterSize         = weight_x * weight_y * sliceCount * weightSize;

    vx_uint32 filterTotalCount   = filter_count;
    vx_uint32 filterCount        = kernels_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;

    vx_uint32 kernelStreamSize   = ((filterSize * filterTotalCount + filterTotalCount * biasSize + filterTotalCount * 3 + 3) + 63) & ~63;
    vx_uint32* kernelBufferPtr   = (vx_uint32*) wb_base_ptr;
    vx_uint32* kernelStreamSizePtr = kernelBufferPtr;
    vx_uint32 streamSizeBitOffset = 0;
    vx_uint32 alignedOffset      = ((vx_uint32)(gctUINTPTR_T)kernelBufferPtr) & 0x3F;
    vx_uint8_ptr kernelDataPtr   = VX_NULL;
    vx_uint8_ptr kernelStreamBasePtr = VX_NULL;
    vx_uint32 usedCoreCount      = (filterTotalCount > nnCoreCount) ? nnCoreCount : filterTotalCount;
    vx_uint32 maxKernelStreamSizePerCore = 0;
    vx_float64* nonZeroRatioPerCorePerVZG = 0;
    vx_bool hasKernelFullCacheInterleaveFix = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_FULLCACHE_KERNEL_INTERLEAVE_FIX);

    typedef struct _gcVXWeightsMinusZP
    {
        vx_int32 sum;
        vx_uint32 filterIdx;
    }gcVXWeightsMinusZP;

    gcVXWeightsMinusZP *weightsMinusZP = VX_NULL;

    if (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8)
    {
        if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
        {
            biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7;
        }
        else
        {
            biasBitSize = NN_INTEGER_BIAS_BITS;
        }
    }
    else if (weightFomat == VX_TYPE_INT16)
    {
        biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7_INT16;
    }

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT) && inputZP != 0 && weightFomat == VX_TYPE_UINT8)
    {
        weightsMinusZP = (gcVXWeightsMinusZP*)vxAllocateAndZeroMemory(filter_count * sizeof(gcVXWeightsMinusZP));

        if (!weightsMinusZP)
        {
            vxError("fillinKernelBuffer: OUT OF MEMORY");
            goto exit;
        }

        /*Calc sum((coef[i] - coefZP) * InZP) */
        for (filterIndex = 0; filterIndex < filter_count; filterIndex++)
        {
            weightsMinusZP[filterIndex].filterIdx = filterIndex;
            weightsMinusZP[filterIndex].sum = 0;
            for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
            {
                /* add one slice data every filter */
                kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * sliceIndex;

                for (weightYIndex = 0; weightYIndex < weight_y; weightYIndex++)
                {
                    for (weightXIndex = 0; weightXIndex < weight_x; weightXIndex++)
                    {
                        vx_int32 weight = 0;
                        weight = *((vx_uint8 *)kernelDataPtr);
                        kernelDataPtr = kernelDataPtr + weightSize;

                        weightsMinusZP[filterIndex].sum += (weight - coefZP) * inputZP;
                    }
                }
            }
        }
    }

    if (wb->zOffsetHandle == VX_NULL)
    {
        wb->zOffsetHandle = (vx_weights_biases_z_offset)vxAllocateAndZeroMemory(filter_count * sizeof(vx_weights_biases_z_offset_s));
        if (wb->zOffsetHandle == VX_NULL)
        {
            vxError("fillinKernelBuffer: OUT OF MEMORY");
            goto exit;
        }
    }
    wb->numOfVz = filter_count;

    if (wb->max_per_core_per_vzgroup_nzr == VX_NULL)
    {
        wb->max_per_core_per_vzgroup_nzr = (vx_float64*)vxAllocateAndZeroMemory(groupCount * sizeof(vx_float64));
        if (wb->max_per_core_per_vzgroup_nzr == VX_NULL)
        {
            vxError("fillinKernelBuffer: OUT OF MEMORY\n");
            goto exit;
        }
    }

    nonZeroRatioPerCorePerVZG = (vx_float64*)vxAllocateAndZeroMemory(nnCoreCount * groupCount * sizeof(vx_float64));
    if (nonZeroRatioPerCorePerVZG == VX_NULL)
    {
        vxError("fillinKernelBuffer: OUT OF MEMORY");
        goto exit;
    }

    /* kernel Buffer size for each core */
    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        writeBits(&kernelBufferPtr, &bitOffset, kernelStreamSize, 32);
    }
    packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);
    wb->slice_array[index].kernel_stream_full_cache_size = 0;

    wb->max_per_core_compression_ratio = 0;

    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_uint32 coreFilterCount = (coreIndex < oddFilterPerCore) ? totalFilterPerCore + 1 : totalFilterPerCore;
        vx_uint32 coreOrgKernelSize = coreFilterCount * (filterSize + biasSize);
        vx_float64 compressionRatio = 0;
        kernelStreamBasePtr = (vx_uint8_ptr)kernelBufferPtr;

        /* zeroRunLen */
        if (coreFilterCount > 0)
        {
            /* zeroRunLen */
            writeBits(&kernelBufferPtr, &bitOffset, min_zero_run_len, 8);

            writeBits(&kernelBufferPtr, &bitOffset, coreFilterCount, 16);
        }
        /* fill weight value and bias for every group */
        zeroRun = 0;
        for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
        {
            vx_uint32 actualFilterCount = (groupIndex == groupCount - 1) ? (coreFilterCount - groupIndex * filterCount) : filterCount;
            vx_uint32 filterStart, filterEnd;
            vx_uint32 blockCount = 0;
            vx_uint32 nonZeroCount = 0;
            vx_uint32 nonZrlIdx = groupIndex + groupCount * coreIndex;

            if ((groupIndex == groupCount - 1)
                && (coreIndex >= (filterTotalCount % nnCoreCount)))
            {
                filterStart = groupIndex * nnCoreCount * filterCount
                            + (filterTotalCount % nnCoreCount) * (actualFilterCount + 1)
                            + (coreIndex - (filterTotalCount % nnCoreCount)) * actualFilterCount;
            }
            else
            {
                filterStart = groupIndex * nnCoreCount *filterCount + coreIndex * actualFilterCount;
            }
            filterEnd = filterStart + actualFilterCount - 1;

            /* Only UINT8/INT8 support ZDP3/ZDP6 */
            if (weight_x == 1 && weight_y == 1
                && (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3) || vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6))
                && (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8))
            {
                vx_uint32 zdpNum = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6) ? 6 : 3;
                /* zdp3 zdp6 can not enable same time */
                vxmASSERT(!(vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3) && vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6)));

                for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex += (zdpNum * 2))
                {
                    vx_uint8 group2DCoded = 0;
                    vx_uint32 weight = 0;
                    vx_uint32 realSliceIndex = 0;

                    /* add slices of every filter*/
                    for (filterIndex = filterStart; filterIndex <= filterEnd; filterIndex++)
                    {
                        vx_uint32 nonZeroInBlock1Count = 0; /*Check if there's non-zero point in first zdpNum block*/
                        vx_uint32 nonZeroInBlock2Count = 0; /*Check if there's non-zero point in another zdpNum block*/
                        for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + (zdpNum * 2); realSliceIndex++)
                        {
                            vx_uint8 needToWriteBias = (realSliceIndex == 0) ? 1 : 0;

                            if (realSliceIndex >= sliceCount)
                                break;

                            /* add one slice data every filter */
                            kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * realSliceIndex;
                            if (realSliceIndex == 0)
                            {
                                if (bias_base_ptr)
                                    biasData = *(bias_base_ptr + filterIndex);
                                else
                                    biasData = 0;

                                /* NewBias = sum((Coef[i] - CoefZP) * - InZP) + Bias*/
                                if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT)
                                    && inputZP != 0
                                    && weightFomat == VX_TYPE_UINT8)
                                {
                                    biasData -= weightsMinusZP[filterIndex].sum;
                                }

                            }

                            if (weightFomat == VX_TYPE_INT8)
                                weight = *((vx_int8 *)kernelDataPtr);
                            else if (weightFomat == VX_TYPE_UINT8)
                                weight = *((vx_uint8 *)kernelDataPtr);
                            else
                                weight = *((vx_uint16 *)kernelDataPtr);

                            if (realSliceIndex - sliceIndex < zdpNum && weight != skipValue)
                                nonZeroInBlock1Count++;
                            else if (weight != skipValue)
                                nonZeroInBlock2Count++;

                            if ((zeroRun == max_zero_run)
                                || ((realSliceIndex == 0)
                                    && needToWriteBias)
                                || (realSliceIndex == sliceCount - 1)
                                || (weight != skipValue)
                                || ((filterIndex == filterEnd)
                                    && (group2DCoded == 0)))
                            {
                                if (min_zero_run_len)
                                {
                                    writeBits(&kernelBufferPtr, &bitOffset, zeroRun, min_zero_run_len);
                                }
                                writeBits(&kernelBufferPtr, &bitOffset, weight, weightBitSize);
                                zeroRun = 0;

                                if (needToWriteBias)
                                {
                                    if (weightFomat == VX_TYPE_INT16)
                                    {
                                        vx_int64 bias64 = 0;
                                        vx_int32 bias32;
                                        vx_int16 bias16;

                                        if ((vx_int32)biasData < 0)
                                        {
                                            bias64 = ~0;
                                            bias64 = (bias64 >> 32) << 32;
                                            bias64 = bias64 | (vx_int64)biasData;
                                        }
                                        else
                                            bias64 = (vx_int64)biasData;

                                        bias32 = (vx_int32)bias64;
                                        writeBits(&kernelBufferPtr, &bitOffset, bias32, 32);
                                        bias16 = (vx_int16)(bias64 >> 32);
                                        writeBits(&kernelBufferPtr, &bitOffset, (vx_int32)bias16, 16);
                                    }
                                    else
                                        writeBits(&kernelBufferPtr, &bitOffset, biasData, biasBitSize);
                                    needToWriteBias = 0;
                                }
                                group2DCoded = 1;
                            }
                            else
                            {
                                zeroRun++;
                            }

                            /* add offet behind the last point of the last slice of each filter */
                            if (realSliceIndex == sliceCount - 1)
                            {
                                vx_uint32 offsetValue;
                                if (z_offset > 0)
                                    offsetValue = (vx_uint32)z_offset * filterIndex;
                                else if (outputSize > 0)
                                    offsetValue = output_final_x * output_final_y * outputSize * filterIndex;
                                else
                                    offsetValue = output_final_x * output_final_y * weightSize * filterIndex;

                                if (wb->zOffsetHandle)
                                {
                                    wb->zOffsetHandle[filterIndex].ptrOffset = (vx_uint32)((vx_uint8 *)kernelBufferPtr - wb_base_ptr);
                                    wb->zOffsetHandle[filterIndex].bitOffset = bitOffset;
                                }

                                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                    writeBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS_VIP_V7);
                                else
                                    writeBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS);
                            }
                        }
                        if (sliceIndex + zdpNum >= sliceCount)
                            blockCount++;
                        else
                            blockCount += 2;

                        if (nonZeroInBlock1Count)
                            nonZeroCount++;
                        if (nonZeroInBlock2Count)
                            nonZeroCount++;
                    }
                }
            }
            /*Only INT8/UINT8 support XY DP9 now*/
            else if ((vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP9) || vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP6))
                && (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8))
            {
                vx_uint32 xStep = 0, yStep = 0;
                vx_uint32 inImageBufferSize = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP9) ? 2 : 1;

                /* xydp9 xydp6 can not enable same time */
                vxmASSERT(!(vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP9) && vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP6)));

                if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP6))
                {
                    xStep = 3;
                    yStep = 2;
                }
                else
                {
                    xStep = 3;
                    yStep = 3;
                }

                for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex += inImageBufferSize)
                {
                    vx_uint8 group2DCoded = 0;
                    vx_uint32 weight = 0;
                    vx_uint32 realSliceIndex = 0;
                    vx_uint32 realWeightXIndex = 0, realWeightYIndex = 0;
                    vx_uint32 subKxSize = 0, subKySize = 0, subKzSize = 0;

                    subKzSize = gcmMIN((sliceCount - sliceIndex), inImageBufferSize);
                    /* add slices of every filter*/
                    for (filterIndex = filterStart; filterIndex <= filterEnd; filterIndex++)
                    {
                        vx_bool hasWriteBias = vx_false_e;
                        for (weightYIndex = 0; weightYIndex < weight_y; weightYIndex += yStep)
                        {
                            subKySize = gcmMIN((weight_y - weightYIndex), yStep);
                            for (weightXIndex = 0; weightXIndex < weight_x; weightXIndex += xStep)
                            {
                                subKxSize = gcmMIN((weight_x - weightXIndex), xStep);
                                for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + subKzSize; realSliceIndex++)
                                {
                                    vx_uint8 needToWriteBias = (sliceIndex == 0) ? 1 : 0;
                                    vx_uint32 nonZeroInBlockCount = 0; /*Check if there's non-zero point in one 3x3 block*/

                                    /* add one slice data every filter */
                                    kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * realSliceIndex;
                                    if (realSliceIndex == 0)
                                    {
                                        if (bias_base_ptr)
                                            biasData = *(bias_base_ptr + filterIndex);
                                        else
                                            biasData = 0;

                                        /* NewBias = sum((Coef[i] - CoefZP) * - InZP) + Bias*/
                                        if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT)
                                            && inputZP != 0
                                            && weightFomat == VX_TYPE_UINT8)
                                        {
                                            biasData -= weightsMinusZP[filterIndex].sum;
                                        }
                                    }

                                    for (realWeightYIndex = weightYIndex; realWeightYIndex < weightYIndex + subKySize; realWeightYIndex++)
                                    {
                                        for (realWeightXIndex = weightXIndex; realWeightXIndex < weightXIndex + subKxSize; realWeightXIndex++)
                                        {
                                            vx_uint8_ptr realKernelDataPtr = kernelDataPtr + (realWeightYIndex * weight_x + realWeightXIndex) * weightSize;

                                            if (weightFomat == VX_TYPE_INT8)
                                                weight = *((vx_int8 *)realKernelDataPtr);
                                            else if (weightFomat == VX_TYPE_UINT8)
                                                weight = *((vx_uint8 *)realKernelDataPtr);
                                            else
                                                weight = *((vx_uint16 *)realKernelDataPtr);

                                            if (weight != skipValue)
                                                nonZeroInBlockCount++;

                                            if ((zeroRun == max_zero_run)
                                                || ((realSliceIndex == 0)
                                                    && (realWeightXIndex == weight_x - 1)
                                                    && (realWeightYIndex == weight_y - 1)
                                                    && needToWriteBias)
                                                || ((realSliceIndex == sliceCount - 1)
                                                    && (realWeightXIndex == weight_x - 1)
                                                    && (realWeightYIndex == weight_y - 1))
                                                || (weight != skipValue)
                                                || ((filterIndex == filterEnd)
                                                    && (realWeightXIndex == weight_x - 1)
                                                    && (realWeightYIndex == weight_y - 1)
                                                    && (group2DCoded == 0)))
                                            {
                                                if (min_zero_run_len)
                                                {
                                                    writeBits(&kernelBufferPtr, &bitOffset, zeroRun, min_zero_run_len);
                                                }
                                                writeBits(&kernelBufferPtr, &bitOffset, weight, weightBitSize);
                                                zeroRun = 0;

                                                if (needToWriteBias && !hasWriteBias)
                                                {
                                                    if (weightFomat == VX_TYPE_INT16)
                                                    {
                                                        vx_int64 bias64 = 0;
                                                        vx_int32 bias32;
                                                        vx_int16 bias16;

                                                        if ((vx_int32)biasData < 0)
                                                        {
                                                            bias64 = ~0;
                                                            bias64 = (bias64 >> 32) << 32;
                                                            bias64 = bias64 | (vx_int64)biasData;
                                                        }
                                                        else
                                                            bias64 = (vx_int64)biasData;

                                                        bias32 = (vx_int32)bias64;
                                                        writeBits(&kernelBufferPtr, &bitOffset, bias32, 32);
                                                        bias16 = (vx_int16)(bias64 >> 32);
                                                        writeBits(&kernelBufferPtr, &bitOffset, (vx_int32)bias16, 16);
                                                    }
                                                    else
                                                        writeBits(&kernelBufferPtr, &bitOffset, biasData, biasBitSize);
                                                    needToWriteBias = 0;
                                                    hasWriteBias = vx_true_e;
                                                }
                                                group2DCoded = 1;
                                            }
                                            else
                                            {
                                                zeroRun++;
                                            }

                                            /* add offet behind the last point of the last slice of each filter */
                                            if (realSliceIndex == sliceCount - 1 && realWeightXIndex == weight_x - 1 && realWeightYIndex == weight_y - 1)
                                            {
                                                vx_uint32 offsetValue;

                                                if (z_offset > 0)
                                                    offsetValue = (vx_uint32)z_offset * filterIndex;
                                                else if (outputSize > 0)
                                                    offsetValue = output_final_x * output_final_y * outputSize * filterIndex;
                                                else
                                                    offsetValue = output_final_x * output_final_y * weightSize * filterIndex;

                                                if (wb->zOffsetHandle)
                                                {
                                                    wb->zOffsetHandle[filterIndex].ptrOffset = (vx_uint32)((vx_uint8 *)kernelBufferPtr - wb_base_ptr);
                                                    wb->zOffsetHandle[filterIndex].bitOffset = bitOffset;
                                                }

                                                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                                    writeBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS_VIP_V7);
                                                else
                                                    writeBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS);
                                            }
                                        }
                                    }
                                    blockCount++;
                                    if (nonZeroInBlockCount)
                                        nonZeroCount++;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
                {
                    vx_uint8 group2DCoded = 0;
                    /* add slices of every filter*/
                    for (filterIndex = filterStart; filterIndex <= filterEnd; filterIndex++)
                    {
                        vx_uint8 needToWriteBias = (sliceIndex == 0) ? 1 : 0;

                        /* add one slice data every filter */
                        kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * sliceIndex;
                        if (sliceIndex == 0)
                        {
                            if (bias_base_ptr)
                                biasData = *(bias_base_ptr + filterIndex);
                            else
                                biasData = 0;

                            /* NewBias = sum((Coef[i] - CoefZP) * - InZP) + Bias*/
                            if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT)
                                && inputZP != 0
                                && weightFomat == VX_TYPE_UINT8)
                            {
                                biasData -= weightsMinusZP[filterIndex].sum;
                            }
                        }

                        for (weightYIndex = 0; weightYIndex < weight_y; weightYIndex++)
                        {
                            vx_uint32 nonZeroCountInDP3 = 0; /*Check if there's non-zero point in one 3x1 block*/
                            for (weightXIndex = 0; weightXIndex < weight_x; weightXIndex++)
                            {
                                vx_uint32 weight = 0;

                                if (weightFomat == VX_TYPE_INT8)
                                    weight = *((vx_int8 *)kernelDataPtr);
                                else if (weightFomat == VX_TYPE_UINT8)
                                    weight = *((vx_uint8 *)kernelDataPtr);
                                else
                                    weight = *((vx_uint16 *)kernelDataPtr);
                                kernelDataPtr = kernelDataPtr + weightSize;

                                {
                                    if (weight != skipValue)
                                        nonZeroCountInDP3++;

                                    /*V7, DP3, one block is 3x1*/
                                    if ((weightXIndex + 1) % 3 == 0 || weightXIndex == wb->weights_sizes[0] - 1)
                                    {
                                        blockCount++;
                                        if (nonZeroCountInDP3)
                                            nonZeroCount++;
                                        nonZeroCountInDP3 = 0;
                                    }
                                }

                                if ((zeroRun == max_zero_run)
                                    || ((sliceIndex == 0)
                                        && (weightXIndex == weight_x - 1)
                                        && (weightYIndex == weight_y - 1)
                                        && needToWriteBias)
                                    || ((weightXIndex == weight_x - 1)
                                        && (weightYIndex == weight_y - 1)
                                        && (sliceIndex == sliceCount - 1))
                                    || (weight != skipValue)
                                    || ((filterIndex == filterEnd)
                                        && (weightXIndex == weight_x - 1)
                                        && (weightYIndex == weight_y - 1)
                                        && (group2DCoded == 0)))
                                {
                                    if (min_zero_run_len)
                                    {
                                        writeBits(&kernelBufferPtr, &bitOffset, zeroRun, min_zero_run_len);
                                    }
                                    writeBits(&kernelBufferPtr, &bitOffset, weight, weightBitSize);
                                    zeroRun = 0;

                                    if (needToWriteBias)
                                    {
                                        if (weightFomat == VX_TYPE_INT16)
                                        {
                                            vx_int64 bias64 = 0;
                                            vx_int32 bias32;
                                            vx_int16 bias16;

                                            if ((vx_int32)biasData < 0)
                                            {
                                                bias64 = ~0;
                                                bias64 = (bias64 >> 32) << 32;
                                                bias64 = bias64 | (vx_int64)biasData;
                                            }
                                            else
                                                bias64 = (vx_int64)biasData;

                                            bias32 = (vx_int32)bias64;
                                            writeBits(&kernelBufferPtr, &bitOffset, bias32, 32);
                                            bias16 = (vx_int16)(bias64 >> 32);
                                            writeBits(&kernelBufferPtr, &bitOffset, (vx_int32)bias16, 16);
                                        }
                                        else
                                            writeBits(&kernelBufferPtr, &bitOffset, biasData, biasBitSize);
                                        needToWriteBias = 0;
                                    }
                                    group2DCoded = 1;
                                }
                                else
                                {
                                    zeroRun++;
                                }
                            }
                        }

                        /* add offet behind the last point of the last slice of each filter */
                        if (sliceIndex == sliceCount - 1)
                        {
                            vx_uint32 offsetValue;
                            if (z_offset > 0)
                                offsetValue = (vx_uint32)z_offset * filterIndex;
                            else if (outputSize > 0)
                                offsetValue = output_final_x * output_final_y * outputSize * filterIndex;
                            else
                                offsetValue = output_final_x * output_final_y * weightSize * filterIndex;

                            if (wb->zOffsetHandle)
                            {
                                wb->zOffsetHandle[filterIndex].ptrOffset = (vx_uint32)((vx_uint8 *)kernelBufferPtr - wb_base_ptr);
                                wb->zOffsetHandle[filterIndex].bitOffset = bitOffset;
                            }

                            if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                writeBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS_VIP_V7);
                            else
                                writeBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS);

                        }
                    }
                }
            }
            if(blockCount != 0)
                nonZeroRatioPerCorePerVZG[nonZrlIdx] = (vx_float64)nonZeroCount / (vx_float64)blockCount;

        }

        /* pad 0 */
        packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);

        /* Go back to update kernelStreamSize. */
        kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);
        /*With RTL bug fix, full cache mode size needn't using maxSize * coreCount*/
        wb->slice_array[index].kernel_stream_full_cache_size += kernelStreamSize;

        compressionRatio = (vx_float64)kernelStreamSize / (vx_float64)coreOrgKernelSize;
        if (wb->max_per_core_compression_ratio < compressionRatio)
            wb->max_per_core_compression_ratio = compressionRatio;

        if (kernelStreamSize > maxKernelStreamSizePerCore)
            maxKernelStreamSizePerCore = kernelStreamSize;

        writeBits(&kernelStreamSizePtr, &streamSizeBitOffset, kernelStreamSize, 32);
    }

        /*bug1980*/
    if(gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_IMAGE_NOT_PACKED_IN_SRAM_FIX))
        wb->slice_array[index].kernel_align_stream_size += (vx_size)(maxKernelStreamSizePerCore * usedCoreCount );
    else
        wb->slice_array[index].kernel_align_stream_size += (vx_size)(maxKernelStreamSizePerCore * nnCoreCount);

    if (!hasKernelFullCacheInterleaveFix)
    {
        /*if did't fix full cache interleave fix, full cache size also need use align size*/
        wb->slice_array[index].kernel_stream_full_cache_size = wb->slice_array[index].kernel_align_stream_size;
    }

    wb->slice_array[index].kernel_max_stream_size_percore = maxKernelStreamSizePerCore;

    if (z_offset > 0)
        wb->orgZOffsetValue = z_offset;
    else if (outputSize > 0)
        wb->orgZOffsetValue = output_final_x * output_final_y * outputSize;
    else
        wb->orgZOffsetValue = output_final_x * output_final_y * weightSize;

    for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
    {
        for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
        {
            vx_uint32 nonZrlIdx = groupIndex + coreIndex * groupCount;
            if (wb->max_per_core_per_vzgroup_nzr[groupIndex] < nonZeroRatioPerCorePerVZG[nonZrlIdx])
                wb->max_per_core_per_vzgroup_nzr[groupIndex] = nonZeroRatioPerCorePerVZG[nonZrlIdx];
        }
    }

exit:
    if (weightsMinusZP)
        vxFree(weightsMinusZP);
    if (nonZeroRatioPerCorePerVZG)
        vxFree(nonZeroRatioPerCorePerVZG);
    return;
}

void fillinDepthWiseKernelBuffer(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint8 min_zero_run_len,
    vx_uint32 max_zero_run,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 weight_z,
    vx_uint32 filter_count,
    vx_uint32 kernels_per_core,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_int32 input_zp,
    vx_int32 coef_zp,
    vx_uint32 skip_value,
    vx_uint32 output_final_x,
    vx_uint32 output_final_y,
    vx_int32 z_offset,
    vx_uint32 outputSize,
    vx_uint8_ptr wb_base_ptr,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_uint32 index
    )
{
    vx_enum weightFomat = weight_format;
    vx_int32 inputZP = input_zp;
    vx_int32 coefZP = coef_zp;

    vx_uint32 nnCoreCount = (weightFomat == VX_TYPE_INT16) ?
        context->nnConfig.fixedFeature.nnCoreCountInt16 : (weightFomat == VX_TYPE_FLOAT16) ?
            context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 coreIndex;
    vx_uint32 sliceIndex;
    vx_uint32 weightXIndex, weightYIndex;
    vx_uint32 bitOffset          = 0;

    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize((vx_type_e)weightFomat);
    vx_uint32 weightBitSize      = weightSize * 8;
    vx_uint32 biasSize           = (vx_uint32)vxDataType_GetSize((vx_type_e)bias_format);
    vx_uint32 biasBitSize        = biasSize * 8;
    vx_uint32 biasData           = 0;

    vx_uint32 filterSliceSize    = weight_x * weight_y * weightSize;

    vx_uint32 filterCount        = kernels_per_core; /* filter count every group */
    vx_uint32 filterTotalCount   = filter_count;
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;

    vx_uint32 kernelStreamSize   = 0;
    vx_uint32* kernelBufferPtr   = (vx_uint32*) wb_base_ptr;
    vx_uint32* kernelStreamSizePtr = kernelBufferPtr;
    vx_uint32 streamSizeBitOffset = 0;
    vx_uint32 alignedOffset      = ((vx_uint32)(gctUINTPTR_T)kernelBufferPtr) & 0x3F;
    vx_uint8_ptr kernelDataPtr   = VX_NULL;
    vx_uint8_ptr kernelStreamBasePtr = VX_NULL;

    vx_uint32 maxKernelStreamSizePerCore = 0;
    vx_uint32 core0FilterCount = 0;

    vx_bool hasXYDP9 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP9);
    vx_bool hasXYDP6 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP6);
    vx_bool biasAtEnd = vx_false_e;
    vx_bool hasKernelFullCacheInterleaveFix = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_FULLCACHE_KERNEL_INTERLEAVE_FIX);

    typedef struct _gcVXWeightsMinusZP
    {
        vx_int32 sum;
        vx_uint32 filterIdx;
    }gcVXWeightsMinusZP;

    gcVXWeightsMinusZP *weightsMinusZP = VX_NULL;

    if (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8)
    {
        if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
        {
            biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7;
        }
        else
        {
            biasBitSize = NN_INTEGER_BIAS_BITS;
        }
    }
    else if (weightFomat == VX_TYPE_INT16)
    {
        biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7_INT16;
    }

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT) && inputZP != 0 && weightFomat == VX_TYPE_UINT8)
    {
        weightsMinusZP = (gcVXWeightsMinusZP*)vxAllocateAndZeroMemory(filterTotalCount * sizeof(gcVXWeightsMinusZP));

        if (!weightsMinusZP)
        {
            vxError("fillinKernelBuffer: OUT OF MEMORY");
            goto exit;
        }

        /*Calc sum((coef[i] - coefZP) * InZP) */
        for (sliceIndex = 0; sliceIndex < filterTotalCount; sliceIndex++)
        {
            weightsMinusZP[sliceIndex].filterIdx = sliceIndex;
            weightsMinusZP[sliceIndex].sum = 0;

            {
                /* add one slice data every filter */
                kernelDataPtr = weight_base_ptr + filterSliceSize * sliceIndex;

                for (weightYIndex = 0; weightYIndex < weight_y; weightYIndex++)
                {
                    for (weightXIndex = 0; weightXIndex < weight_x; weightXIndex++)
                    {
                        vx_int32 weight = 0;
                        weight = *((vx_uint8 *)kernelDataPtr);
                        kernelDataPtr = kernelDataPtr + weightSize;

                        weightsMinusZP[sliceIndex].sum += (weight - coefZP) * inputZP;
                    }
                }
            }
        }
    }
    wb->numOfVz = filterTotalCount;

    /* kernel Buffer size for each core */
    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        writeBits(&kernelBufferPtr, &bitOffset, kernelStreamSize, 32);
    }
    packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);
    wb->slice_array[index].kernel_stream_full_cache_size = 0;

    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_uint32 coreFilterCount = (coreIndex < oddFilterPerCore) ? totalFilterPerCore + 1 : totalFilterPerCore;
        vx_uint32 groupIndex;

        kernelStreamBasePtr = (vx_uint8_ptr)kernelBufferPtr;

        /* zeroRunLen */
        writeBits(&kernelBufferPtr, &bitOffset, min_zero_run_len, 8);
        /*kernels num in each core*/
        writeBits(&kernelBufferPtr, &bitOffset, coreFilterCount, 16);

        for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
        {
            vx_uint32 actualFilterCount = (groupIndex == groupCount - 1) ? (coreFilterCount - groupIndex * filterCount) : filterCount;
            vx_uint32 groupFilterStart = groupIndex * nnCoreCount * filterCount;
            vx_uint32 adjFilterStart = groupFilterStart + nnCoreCount - coreIndex - 1;
            vx_uint32 kid = 0;
            vx_uint32 unusedCoreCount = (filterTotalCount % nnCoreCount == 0) ? 0 : nnCoreCount - (filterTotalCount % nnCoreCount);
            vx_uint32 realSliceIndex = 0;

            if (coreIndex == 0)
                core0FilterCount = actualFilterCount;

            for (kid = 0; kid < actualFilterCount; kid++)
            {
                vx_uint32 xStep = 0, yStep = 0;
                vx_uint32 skxSize = 0, skySize = 0;
                vx_uint32 skx = 0, sky = 0;
                vx_uint32 zeroRun = 0;

                if (groupIndex == groupCount - 1 &&
                    kid == core0FilterCount - 1 &&
                    kid != 0)
                    realSliceIndex = adjFilterStart + kid * nnCoreCount - unusedCoreCount;
                else
                    realSliceIndex = adjFilterStart + kid * nnCoreCount;

                xStep = 3;
                yStep = hasXYDP9 ? 3 : hasXYDP6 ? 2 : 1;
                kernelDataPtr = weight_base_ptr + realSliceIndex * filterSliceSize;

                if (bias_base_ptr)
                    biasData = *(bias_base_ptr + realSliceIndex);
                else
                    biasData = 0;

                /* NewBias = sum((Coef[i] - CoefZP) * - InZP) + Bias*/
                if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT)
                    && inputZP != 0
                    && weightFomat == VX_TYPE_UINT8)
                {
                    biasData -= weightsMinusZP[realSliceIndex].sum;
                }

                for (weightYIndex = 0; weightYIndex < weight_y; weightYIndex += yStep)
                {
                    skySize = gcmMIN(weight_y - weightYIndex, yStep);
                    for (weightXIndex = 0; weightXIndex < weight_x; weightXIndex += xStep)
                    {
                        skxSize = gcmMIN(weight_x - weightXIndex, xStep);
                        for (sky = 0; sky < skySize; sky++)
                        {
                            for (skx = 0; skx < skxSize; skx++)
                            {
                                vx_uint32 realWeightX = weightXIndex + skx;
                                vx_uint32 realWeightY = weightYIndex + sky;
                                vx_uint32 weight;

                                vx_uint8_ptr realKernelDataPtr = kernelDataPtr + (realWeightX * weight_x + realWeightY) * weightSize;

                                if (realWeightX == 0 &&
                                    realWeightY == 0 &&
                                    !biasAtEnd)
                                {
                                    if (weightFomat == VX_TYPE_INT16)
                                    {
                                        vx_int64 bias64 = 0;
                                        vx_int32 bias32;
                                        vx_int16 bias16;

                                        if ((vx_int32)biasData < 0)
                                        {
                                            bias64 = ~0;
                                            bias64 = (bias64 >> 32) << 32;
                                            bias64 = bias64 | (vx_int64)biasData;
                                        }
                                        else
                                            bias64 = (vx_int64)biasData;

                                        bias32 = (vx_int32)bias64;
                                        writeBits(&kernelBufferPtr, &bitOffset, bias32, 32);
                                        bias16 = (vx_int16)(bias64 >> 32);
                                        writeBits(&kernelBufferPtr, &bitOffset, (vx_int32)bias16, 16);
                                    }
                                    else
                                        writeBits(&kernelBufferPtr, &bitOffset, biasData, biasBitSize);
                                }

                                if (weightFomat == VX_TYPE_INT8)
                                    weight = *((vx_int8 *)realKernelDataPtr);
                                else if (weightFomat == VX_TYPE_UINT8)
                                    weight = *((vx_uint8 *)realKernelDataPtr);
                                else
                                    weight = *((vx_uint16 *)realKernelDataPtr);

                                if ((zeroRun == max_zero_run)
                                        || ((realWeightX == weight_x - 1)
                                        && (realWeightY == weight_y - 1))
                                        || (weight != skip_value))
                                {
                                    if (min_zero_run_len)
                                    {
                                        writeBits(&kernelBufferPtr, &bitOffset, zeroRun, min_zero_run_len);
                                    }
                                    writeBits(&kernelBufferPtr, &bitOffset, weight, weightBitSize);
                                    zeroRun = 0;
                                }
                                else
                                {
                                    zeroRun++;
                                }

                                writeBits(&kernelBufferPtr, &bitOffset, weight, weightBitSize);

                                if (realWeightX == weight_x - 1 &&
                                    realWeightY == weight_y - 1)
                                {
                                    /*ZOFFSET*/
                                    vx_uint32 offsetValue;
                                    if (z_offset > 0)
                                        offsetValue = (vx_uint32)z_offset * realSliceIndex;
                                    else if (outputSize > 0)
                                        offsetValue = output_final_x * output_final_y * outputSize * realSliceIndex;
                                    else
                                        offsetValue = output_final_x * output_final_y * weightSize * realSliceIndex;

                                    writeBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS_VIP_V7);

                                    /*Bias at the end*/
                                    if (biasAtEnd)
                                    {
                                        if (weightFomat == VX_TYPE_INT16)
                                        {
                                            vx_int64 bias64 = 0;
                                            vx_int32 bias32;
                                            vx_int16 bias16;

                                            if ((vx_int32)biasData < 0)
                                            {
                                                bias64 = ~0;
                                                bias64 = (bias64 >> 32) << 32;
                                                bias64 = bias64 | (vx_int64)biasData;
                                            }
                                            else
                                                bias64 = (vx_int64)biasData;

                                            bias32 = (vx_int32)bias64;
                                            writeBits(&kernelBufferPtr, &bitOffset, bias32, 32);
                                            bias16 = (vx_int16)(bias64 >> 32);
                                            writeBits(&kernelBufferPtr, &bitOffset, (vx_int32)bias16, 16);
                                        }
                                        else
                                            writeBits(&kernelBufferPtr, &bitOffset, biasData, biasBitSize);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        /* pad 0 */
        packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);

        /* Go back to update kernelStreamSize. */
        kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);
        /*With RTL bug fix, full cache mode size needn't using maxSize * coreCount*/
        wb->slice_array[index].kernel_stream_full_cache_size += kernelStreamSize;

        if (kernelStreamSize > maxKernelStreamSizePerCore)
            maxKernelStreamSizePerCore = kernelStreamSize;

        writeBits(&kernelStreamSizePtr, &streamSizeBitOffset, kernelStreamSize, 32);
    }

    wb->slice_array[index].kernel_align_stream_size += (vx_size)(maxKernelStreamSizePerCore * nnCoreCount);

    if (!hasKernelFullCacheInterleaveFix)
    {
        /*if did't fix full cache interleave fix, full cache size also need use align size*/
        wb->slice_array[index].kernel_stream_full_cache_size = wb->slice_array[index].kernel_align_stream_size;
    }

    wb->slice_array[index].kernel_max_stream_size_percore = maxKernelStreamSizePerCore;

    if (z_offset > 0)
        wb->orgZOffsetValue = z_offset;
    else if (outputSize > 0)
        wb->orgZOffsetValue = output_final_x * output_final_y * outputSize;
    else
        wb->orgZOffsetValue = output_final_x * output_final_y * weightSize;

exit:
    if (weightsMinusZP)
        vxFree(weightsMinusZP);

    return;
}

void fillinKernelBufferBalance(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint8 min_zero_run_len,
    vx_uint32 max_zero_run,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 weight_z,
    vx_uint32 filter_count,
    vx_uint32 kernels_per_core,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_int32 input_zp,
    vx_int32 coef_zp,
    vx_uint32 skip_value,
    vx_uint32 output_final_x,
    vx_uint32 output_final_y,
    vx_int32 z_offset,
    vx_uint32 outputSize,
    vx_uint8_ptr wb_base_ptr,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_uint32 index
    )
{
    vx_enum weightFomat = weight_format;
    vx_int32 inputZP = input_zp;
    vx_int32 coefZP = coef_zp;
    vx_uint32 skipValue = skip_value;

    vx_uint32 nnCoreCount = (weightFomat == VX_TYPE_INT16) ?
        context->nnConfig.fixedFeature.nnCoreCountInt16 : (weightFomat == VX_TYPE_FLOAT16) ?
            context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 coreIndex;
    vx_uint32 groupIndex, filterIndex, sliceIndex;
    vx_uint32 weightXIndex, weightYIndex;
    vx_uint32 bitOffset          = 0;
    vx_uint32 zeroRun            = 0;

    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize(weightFomat);
    vx_uint32 weightBitSize      = weightSize * 8;
    vx_uint32 biasSize           = (vx_uint32)vxDataType_GetSize(bias_format);
    vx_uint32 biasBitSize        = biasSize * 8;
    vx_uint32 biasData           = 0;
    vx_int64  biasData_64bits    = 0;

    vx_uint32 sliceCount         = weight_z;
    vx_uint32 filterSliceSize    = weight_x * weight_y * weightSize;
    vx_uint32 filterSize         = weight_x * weight_y * sliceCount * weightSize;

    vx_uint32 filterTotalCount   = filter_count;
    vx_uint32 filterCount        = kernels_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;

    vx_uint32 kernelStreamSize   = ((filterSize * filterTotalCount + filterTotalCount * biasSize + filterTotalCount * 3 + 3) + 63) & ~63;
    vx_uint32* kernelBufferPtr   = (vx_uint32*) wb_base_ptr;
    vx_uint32* kernelStreamSizePtr = kernelBufferPtr;
    vx_uint32 streamSizeBitOffset = 0;
    vx_uint32 alignedOffset      = ((vx_uint32)(gctUINTPTR_T)kernelBufferPtr) & 0x3F;
    vx_uint8_ptr kernelDataPtr   = VX_NULL;
    vx_uint8_ptr kernelStreamBasePtr = VX_NULL;
    vx_uint32 usedCoreCount      = (filterTotalCount > nnCoreCount) ? nnCoreCount : filterTotalCount;
    vx_uint32 maxKernelStreamSizePerCore = 0;
    vx_float64* nonZeroRatioPerCorePerVZG = 0;
    vx_uint32 totalNonZeroCount = 0;
    vx_float32 averageNZCPerFilter = 0;
    vx_float32 varianceFL32 = 0;
    vx_uint32 variance = 0;
    vx_bool reorder = vx_false_e;
    vx_bool hasKernelFullCacheInterleaveFix = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_FULLCACHE_KERNEL_INTERLEAVE_FIX);

    typedef struct _gcVXWeightsMinusZP
    {
        vx_int32 sum;
        vx_uint32 filterIdx;
    }gcVXWeightsMinusZP;

    gcVXWeightsMinusZP *weightsMinusZP = VX_NULL;

    typedef struct _gcVXNonZeroWeights
    {
        vx_uint32 nonZeroCount;
        vx_uint32 filterIdx;
    }gcVXNonZeroWeights;

#if PRINT_PER_CORE_SIZE
    vx_uint32* nonZeroCoefPerCore = VX_NULL;
#endif
    gcVXNonZeroWeights *nonZeroWeights = VX_NULL;
    vx_int32 i = 0;
    vx_uint32 core0FilterCount = 0;
    gcVXNonZeroWeights tmp;

    nonZeroWeights = (gcVXNonZeroWeights*)vxAllocateAndZeroMemory(filter_count * sizeof(gcVXNonZeroWeights));
    if (nonZeroWeights == VX_NULL)
    {
        vxError("fillinKernelBuffer: OUT OF MEMORY");
        goto exit;
    }
#if PRINT_PER_CORE_SIZE
    nonZeroCoefPerCore = (vx_uint32*)vxAllocateAndZeroMemory(nnCoreCount * sizeof(vx_uint32));
    if (nonZeroCoefPerCore == VX_NULL)
    {
        vxError("fillinKernelBuffer: OUT OF MEMORY");
        goto exit;
    }
#endif
    /* calc each filter non-zero weights num*/
    for (filterIndex = 0; filterIndex < filter_count; filterIndex++)
    {
        nonZeroWeights[filterIndex].filterIdx = filterIndex;
        for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
        {
            /* add one slice data every filter */
            kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * sliceIndex;

            for (weightYIndex = 0; weightYIndex < weight_y; weightYIndex++)
            {
                for (weightXIndex = 0; weightXIndex < weight_x; weightXIndex++)
                {
                    vx_uint32 weight = 0;

                    if (weightFomat == VX_TYPE_INT8)
                        weight = *((vx_int8 *)kernelDataPtr);
                    else if (weightFomat == VX_TYPE_UINT8)
                        weight = *((vx_uint8 *)kernelDataPtr);
                    else
                        weight = *((vx_uint16 *)kernelDataPtr);
                    kernelDataPtr = kernelDataPtr + weightSize;

                    if (weight != skipValue)
                        nonZeroWeights[filterIndex].nonZeroCount++;

                }
            }
        }
        totalNonZeroCount += nonZeroWeights[filterIndex].nonZeroCount;
    }

    /*Sort*/
    averageNZCPerFilter = (vx_float32)totalNonZeroCount / filter_count;
    for (filterIndex = 0; filterIndex < filter_count; filterIndex++)
    {
        varianceFL32 += gcoMATH_Power((nonZeroWeights[filterIndex].nonZeroCount - averageNZCPerFilter), 2);
    }
    variance = (vx_uint32)(varianceFL32 / filter_count + 0.5f);

    if (variance > 12)
        reorder = vx_true_e;

    if (reorder)
    {
        /*Sort*/
        for (filterIndex = 0; filterIndex < filter_count; filterIndex++)
        {
            tmp.nonZeroCount = nonZeroWeights[filterIndex].nonZeroCount;
            tmp.filterIdx = filterIndex;

            for (i = filterIndex - 1; i >= 0 && nonZeroWeights[i].nonZeroCount > tmp.nonZeroCount; i--)
            {
                nonZeroWeights[i+1].nonZeroCount = nonZeroWeights[i].nonZeroCount;
                nonZeroWeights[i+1].filterIdx = nonZeroWeights[i].filterIdx;
            }
            nonZeroWeights[i+1].nonZeroCount = tmp.nonZeroCount;
            nonZeroWeights[i+1].filterIdx = tmp.filterIdx;
        }
    }

    if (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8)
    {
        if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
        {
            biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7;
        }
        else
        {
            biasBitSize = NN_INTEGER_BIAS_BITS;
        }
    }
    else if (weightFomat == VX_TYPE_INT16)
    {
        biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7_INT16;
    }

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT) && inputZP != 0 && weightFomat == VX_TYPE_UINT8)
    {
        weightsMinusZP = (gcVXWeightsMinusZP*)vxAllocateAndZeroMemory(filter_count * sizeof(gcVXWeightsMinusZP));

        if (!weightsMinusZP)
        {
            vxError("fillinKernelBufferBalance: OUT OF MEMORY");
            goto exit;
        }

        /*Calc sum((coef[i] - coefZP) * InZP) */
        for (filterIndex = 0; filterIndex < filter_count; filterIndex++)
        {
            weightsMinusZP[filterIndex].filterIdx = filterIndex;
            weightsMinusZP[filterIndex].sum = 0;
            for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
            {
                /* add one slice data every filter */
                kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * sliceIndex;

                for (weightYIndex = 0; weightYIndex < weight_y; weightYIndex++)
                {
                    for (weightXIndex = 0; weightXIndex < weight_x; weightXIndex++)
                    {
                        vx_int32 weight = 0;
                        weight = *((vx_uint8 *)kernelDataPtr);
                        kernelDataPtr = kernelDataPtr + weightSize;

                        weightsMinusZP[filterIndex].sum += (weight - coefZP) * inputZP;
                    }
                }
            }
        }
    }

    if (wb->zOffsetHandle == VX_NULL)
    {
        wb->zOffsetHandle = (vx_weights_biases_z_offset)vxAllocateAndZeroMemory(filter_count * sizeof(vx_weights_biases_z_offset_s));
        if (wb->zOffsetHandle == VX_NULL)
        {
            vxError("fillinKernelBufferBalance: OUT OF MEMORY");
            goto exit;
        }
    }

    if (wb->max_per_core_per_vzgroup_nzr == VX_NULL)
    {
        wb->max_per_core_per_vzgroup_nzr = (vx_float64*)vxAllocateAndZeroMemory(groupCount * sizeof(vx_float64));
        if (wb->max_per_core_per_vzgroup_nzr == VX_NULL)
        {
            vxError("fillinKernelBuffer: OUT OF MEMORY\n");
            goto exit;
        }
    }

    nonZeroRatioPerCorePerVZG = (vx_float64*)vxAllocateAndZeroMemory(nnCoreCount * groupCount * sizeof(vx_float64));
    if (nonZeroRatioPerCorePerVZG == VX_NULL)
    {
        vxError("fillinKernelBuffer: OUT OF MEMORY");
        goto exit;
    }

    wb->numOfVz = filter_count;

    /* kernel Buffer size for each core */
    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        writeBits(&kernelBufferPtr, &bitOffset, kernelStreamSize, 32);
    }
    packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);
    wb->slice_array[index].kernel_stream_full_cache_size = 0;

    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_uint32 coreFilterCount = (coreIndex < oddFilterPerCore) ? totalFilterPerCore + 1 : totalFilterPerCore;
        vx_uint32 usedKid = 0;
        vx_uint32* filterGroup = VX_NULL;
        vx_uint32 coreOrgKernelSize = coreFilterCount * (filterSize + biasSize);
        vx_float64 compressionRatio = 0;

        kernelStreamBasePtr = (vx_uint8_ptr)kernelBufferPtr;
        if (coreFilterCount > 0)
        {
            /* zeroRunLen */
            writeBits(&kernelBufferPtr, &bitOffset, min_zero_run_len, 8);

            writeBits(&kernelBufferPtr, &bitOffset, coreFilterCount, 16);
        }
        /* fill weight value and bias for every group */
        zeroRun = 0;
        for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
        {
            vx_uint32 actualFilterCount = (groupIndex == groupCount - 1) ? (coreFilterCount - groupIndex * filterCount) : filterCount;
            vx_uint32 groupFilterStart = groupIndex * nnCoreCount * filterCount;
            vx_uint32 kid = 0;
            vx_uint32 unusedCoreCount = (filterTotalCount % nnCoreCount == 0) ? 0 : nnCoreCount - (filterTotalCount % nnCoreCount);
            vx_uint32 sortedIndex = 0;
            vx_uint32 blockCount = 0;
            vx_uint32 nonZeroCount = 0;
            vx_uint32 nonZrlIdx = groupIndex + groupCount * coreIndex;
            vx_uint32 filterStart;

            if (coreIndex == 0)
                core0FilterCount = actualFilterCount;

            if (actualFilterCount)
            {
                filterGroup = (vx_uint32*)vxAllocateAndZeroMemory(actualFilterCount * sizeof(vx_uint32));
                if (filterGroup == VX_NULL)
                {
                    vxError("fillinKernelBufferBalance: OUT OF MEMORY");
                    goto exit;
                }
            }

            if (reorder)
            {
                /*Dispatch filter to each vzGroup*/
                for (kid = 0; kid < actualFilterCount; kid++)
                {
                    if (groupIndex == groupCount - 1 &&
                        kid == core0FilterCount - 1)
                    {
                        if ((kid + usedKid) % 2 == 0)
                            sortedIndex = groupFilterStart + kid * nnCoreCount + coreIndex;
                        else
                            sortedIndex = groupFilterStart + (kid + 1) * nnCoreCount - coreIndex - 1 - unusedCoreCount;
                    }
                    else
                    {
                        if ((kid + usedKid) % 2 == 0)
                            sortedIndex = groupFilterStart + kid * nnCoreCount + coreIndex;
                        else
                            sortedIndex = groupFilterStart + (kid + 1) * nnCoreCount - coreIndex - 1;
                    }

                    filterGroup[kid] = nonZeroWeights[sortedIndex].filterIdx;
#if PRINT_PER_CORE_SIZE
                    nonZeroCoefPerCore[coreIndex] += nonZeroWeights[sortedIndex].nonZeroCount;
#endif
                }

                /*sort the vz group real filter to avoid write output cross*/
                for (kid = 0; kid < actualFilterCount; kid++)
                {
                    vx_uint32 tmp = filterGroup[kid];

                    for (i = kid - 1; i >= 0 && filterGroup[i] > tmp; i--)
                    {
                        filterGroup[i+1] = filterGroup[i];
                    }
                    filterGroup[i+1] = tmp;
                }
            }
            else
            {
                /*if variance is small, don't reorder the filters*/
                if ((groupIndex == groupCount - 1)
                && (coreIndex >= (filterTotalCount % nnCoreCount)))
                {
                    filterStart = groupIndex * nnCoreCount * filterCount
                                + (filterTotalCount % nnCoreCount) * (actualFilterCount + 1)
                                + (coreIndex - (filterTotalCount % nnCoreCount)) * actualFilterCount;
                }
                else
                {
                    filterStart = groupIndex * nnCoreCount *filterCount + coreIndex * actualFilterCount;
                }

                for (kid = 0; kid < actualFilterCount; kid++)
                {
                    filterGroup[kid] = filterStart + kid;
                }
            }

            /* Only UINT8/INT8 support ZDP3/ZDP6 */
            if (weight_x == 1 && weight_y == 1
                && (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3) || vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6))
                && (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8))
            {
                vx_uint32 zdpNum = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6) ? 6 : 3;
                /* zdp3 zdp6 can not enable same time */
                vxmASSERT(!(vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3) && vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6)));

                for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex += (zdpNum * 2))
                {
                    vx_uint8 group2DCoded = 0;
                    vx_uint32 weight = 0;
                    vx_uint32 realSliceIndex = 0;

                    /* add slices of every filter*/
                    for (kid = 0; kid < actualFilterCount; kid++)
                    {
                        vx_uint32 nonZeroInBlock1Count = 0; /*Check if there's non-zero point in first zdpNum block*/
                        vx_uint32 nonZeroInBlock2Count = 0; /*Check if there's non-zero point in another zdpNum block*/
                        filterIndex = filterGroup[kid];

                        for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + (zdpNum * 2); realSliceIndex++)
                        {
                            vx_uint8 needToWriteBias = (realSliceIndex == 0) ? 1 : 0;

                            if (realSliceIndex >= sliceCount)
                                break;

                            /* add one slice data every filter */
                            kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * realSliceIndex;
                            if (realSliceIndex == 0)
                            {
                                if (bias_base_ptr)
                                    biasData = *(bias_base_ptr + filterIndex);
                                else
                                    biasData = 0;

                                /* NewBias = sum((Coef[i] - CoefZP) * - InZP) + Bias*/
                                if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT)
                                    && inputZP != 0
                                    && weightFomat == VX_TYPE_UINT8)
                                {
                                    biasData -= weightsMinusZP[filterIndex].sum;
                                }

                            }

                            if (weightFomat == VX_TYPE_INT8)
                                weight = *((vx_int8 *)kernelDataPtr);
                            else if (weightFomat == VX_TYPE_UINT8)
                                weight = *((vx_uint8 *)kernelDataPtr);
                            else
                                weight = *((vx_uint16 *)kernelDataPtr);

                            if (realSliceIndex - sliceIndex < zdpNum && weight != skipValue)
                                nonZeroInBlock1Count++;
                            else if (weight != skipValue)
                                nonZeroInBlock2Count++;

                            if ((zeroRun == max_zero_run)
                                || ((realSliceIndex == 0)
                                    && needToWriteBias)
                                || (realSliceIndex == sliceCount - 1)
                                || (weight != skipValue)
                                || ((kid == actualFilterCount - 1)
                                    && (group2DCoded == 0)))
                            {
                                if (min_zero_run_len)
                                {
                                    writeBits(&kernelBufferPtr, &bitOffset, zeroRun, min_zero_run_len);
                                }
                                writeBits(&kernelBufferPtr, &bitOffset, weight, weightBitSize);
                                zeroRun = 0;

                                if (needToWriteBias)
                                {
                                    if (weightFomat == VX_TYPE_INT16)
                                    {
                                        vx_int64 bias64 = 0;
                                        vx_int32 bias32;
                                        vx_int16 bias16;

                                        if ((vx_int32)biasData < 0)
                                        {
                                            bias64 = ~0;
                                            bias64 = (bias64 >> 32) << 32;
                                            bias64 = bias64 | (vx_int64)biasData;
                                        }
                                        else
                                            bias64 = (vx_int64)biasData;

                                        bias32 = (vx_int32)bias64;
                                        writeBits(&kernelBufferPtr, &bitOffset, bias32, 32);
                                        bias16 = (vx_int16)(bias64 >> 32);
                                        writeBits(&kernelBufferPtr, &bitOffset, (vx_int32)bias16, 16);
                                    }
                                    else
                                        writeBits(&kernelBufferPtr, &bitOffset, biasData, biasBitSize);
                                    needToWriteBias = 0;
                                }
                                group2DCoded = 1;
                            }
                            else
                            {
                                zeroRun++;
                            }

                            /* add offet behind the last point of the last slice of each filter */
                            if (realSliceIndex == sliceCount - 1)
                            {
                                vx_uint32 offsetValue;
                                if (z_offset > 0)
                                    offsetValue = (vx_uint32)z_offset * filterIndex;
                                else if (outputSize > 0)
                                    offsetValue = output_final_x * output_final_y * outputSize * filterIndex;
                                else
                                    offsetValue = output_final_x * output_final_y * weightSize * filterIndex;

                                if (wb->zOffsetHandle)
                                {
                                    wb->zOffsetHandle[filterIndex].ptrOffset = (vx_uint32)((vx_uint8 *)kernelBufferPtr - wb_base_ptr);
                                    wb->zOffsetHandle[filterIndex].bitOffset = bitOffset;
                                }

                                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                    writeBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS_VIP_V7);
                                else
                                    writeBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS);
                            }
                        }
                        if (sliceIndex + zdpNum >= sliceCount)
                            blockCount++;
                        else
                            blockCount += 2;

                        if (nonZeroInBlock1Count)
                            nonZeroCount++;
                        if (nonZeroInBlock2Count)
                            nonZeroCount++;
                    }
                }
            }
            /*Only INT8/UINT8 support XY DP9 now*/
            else if ((vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP9) || vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP6))
                && (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8))
            {
                vx_uint32 xStep = 0, yStep = 0;
                vx_uint32 inImageBufferSize = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP9) ? 2 : 1;

                /* xydp9 xydp6 can not enable same time */
                vxmASSERT(!(vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP9) && vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP6)));

                if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP6))
                {
                    xStep = 3;
                    yStep = 2;
                }
                else
                {
                    xStep = 3;
                    yStep = 3;
                }

                for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex += inImageBufferSize)
                {
                    vx_uint8 group2DCoded = 0;
                    vx_uint32 weight = 0;
                    vx_uint32 realSliceIndex = 0;
                    vx_uint32 realWeightXIndex = 0, realWeightYIndex = 0;
                    vx_uint32 subKxSize = 0, subKySize = 0, subKzSize = 0;

                    subKzSize = gcmMIN((sliceCount - sliceIndex), inImageBufferSize);
                    /* add slices of every filter*/
                    for (kid = 0; kid < actualFilterCount; kid++)
                    {
                        vx_bool hasWriteBias = vx_false_e;

                        filterIndex = filterGroup[kid];

                        for (weightYIndex = 0; weightYIndex < weight_y; weightYIndex += yStep)
                        {
                            subKySize = gcmMIN((weight_y - weightYIndex), yStep);
                            for (weightXIndex = 0; weightXIndex < weight_x; weightXIndex += xStep)
                            {
                                subKxSize = gcmMIN((weight_x - weightXIndex), xStep);
                                for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + subKzSize; realSliceIndex++)
                                {
                                    vx_uint8 needToWriteBias = (sliceIndex == 0) ? 1 : 0;
                                    vx_uint32 nonZeroInBlockCount = 0; /*Check if there's non-zero point in one 3x3 block*/

                                    /* add one slice data every filter */
                                    kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * realSliceIndex;
                                    if (realSliceIndex == 0)
                                    {
                                        if (bias_base_ptr)
                                            biasData = *(bias_base_ptr + filterIndex);
                                        else
                                            biasData = 0;

                                        /* NewBias = sum((Coef[i] - CoefZP) * - InZP) + Bias*/
                                        if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT)
                                            && inputZP != 0
                                            && weightFomat == VX_TYPE_UINT8)
                                        {
                                            biasData -= weightsMinusZP[filterIndex].sum;
                                        }
                                    }

                                    for (realWeightYIndex = weightYIndex; realWeightYIndex < weightYIndex + subKySize; realWeightYIndex++)
                                    {
                                        for (realWeightXIndex = weightXIndex; realWeightXIndex < weightXIndex + subKxSize; realWeightXIndex++)
                                        {
                                            vx_uint8_ptr realKernelDataPtr = kernelDataPtr + (realWeightYIndex * weight_x + realWeightXIndex) * weightSize;

                                            if (weightFomat == VX_TYPE_INT8)
                                                weight = *((vx_int8 *)realKernelDataPtr);
                                            else if (weightFomat == VX_TYPE_UINT8)
                                                weight = *((vx_uint8 *)realKernelDataPtr);
                                            else
                                                weight = *((vx_uint16 *)realKernelDataPtr);

                                            if (weight != skipValue)
                                                nonZeroInBlockCount++;

                                            if ((zeroRun == max_zero_run)
                                                || ((realSliceIndex == 0)
                                                    && (realWeightXIndex == weight_x - 1)
                                                    && (realWeightYIndex == weight_y - 1)
                                                    && needToWriteBias)
                                                || ((realSliceIndex == sliceCount - 1)
                                                    && (realWeightXIndex == weight_x - 1)
                                                    && (realWeightYIndex == weight_y - 1))
                                                || (weight != skipValue)
                                                || ((kid == actualFilterCount - 1)
                                                    && (realWeightXIndex == weight_x - 1)
                                                    && (realWeightYIndex == weight_y - 1)
                                                    && (group2DCoded == 0)))
                                            {
                                                if (min_zero_run_len)
                                                {
                                                    writeBits(&kernelBufferPtr, &bitOffset, zeroRun, min_zero_run_len);
                                                }
                                                writeBits(&kernelBufferPtr, &bitOffset, weight, weightBitSize);
                                                zeroRun = 0;

                                                if (needToWriteBias && !hasWriteBias)
                                                {
                                                    if (weightFomat == VX_TYPE_INT16)
                                                    {
                                                        vx_int64 bias64 = 0;
                                                        vx_int32 bias32;
                                                        vx_int16 bias16;

                                                        if ((vx_int32)biasData < 0)
                                                        {
                                                            bias64 = ~0;
                                                            bias64 = (bias64 >> 32) << 32;
                                                            bias64 = bias64 | (vx_int64)biasData;
                                                        }
                                                        else
                                                            bias64 = (vx_int64)biasData;

                                                        bias32 = (vx_int32)bias64;
                                                        writeBits(&kernelBufferPtr, &bitOffset, bias32, 32);
                                                        bias16 = (vx_int16)(bias64 >> 32);
                                                        writeBits(&kernelBufferPtr, &bitOffset, (vx_int32)bias16, 16);
                                                    }
                                                    else
                                                        writeBits(&kernelBufferPtr, &bitOffset, biasData, biasBitSize);
                                                    needToWriteBias = 0;
                                                    hasWriteBias = vx_true_e;
                                                }
                                                group2DCoded = 1;
                                            }
                                            else
                                            {
                                                zeroRun++;
                                            }

                                            /* add offet behind the last point of the last slice of each filter */
                                            if (realSliceIndex == sliceCount - 1 && realWeightXIndex == weight_x - 1 && realWeightYIndex == weight_y - 1)
                                            {
                                                vx_uint32 offsetValue;

                                                if (z_offset > 0)
                                                    offsetValue = (vx_uint32)z_offset * filterIndex;
                                                else if (outputSize > 0)
                                                    offsetValue = output_final_x * output_final_y * outputSize * filterIndex;
                                                else
                                                    offsetValue = output_final_x * output_final_y * weightSize * filterIndex;

                                                if (wb->zOffsetHandle)
                                                {
                                                    wb->zOffsetHandle[filterIndex].ptrOffset = (vx_uint32)((vx_uint8 *)kernelBufferPtr - wb_base_ptr);
                                                    wb->zOffsetHandle[filterIndex].bitOffset = bitOffset;
                                                }

                                                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                                    writeBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS_VIP_V7);
                                                else
                                                    writeBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS);
                                            }
                                        }
                                    }
                                    blockCount++;
                                    if (nonZeroInBlockCount)
                                        nonZeroCount++;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
                {
                    vx_uint8 group2DCoded = 0;
                    /* add slices of every filter*/
                    for (kid = 0; kid < actualFilterCount; kid++)
                    {
                        vx_uint8 needToWriteBias = (sliceIndex == 0) ? 1 : 0;
                        filterIndex = filterGroup[kid];

                        /* add one slice data every filter */
                        kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * sliceIndex;
                        if (sliceIndex == 0)
                        {
                            if (bias_base_ptr)
                            {
                                if(bias_format == VX_TYPE_INT64)
                                    biasData_64bits = *(((vx_int64 *)bias_base_ptr) + filterIndex);
                                else
                                    biasData = *(bias_base_ptr + filterIndex);
                            }
                            else
                                biasData = 0;

                            /* NewBias = sum((Coef[i] - CoefZP) * - InZP) + Bias*/
                            if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT)
                                && inputZP != 0
                                && weightFomat == VX_TYPE_UINT8)
                            {
                                biasData -= weightsMinusZP[filterIndex].sum;
                            }
                        }

                        for (weightYIndex = 0; weightYIndex < weight_y; weightYIndex++)
                        {
                            vx_uint32 nonZeroCountInDP3 = 0; /*Check if there's non-zero point in one 3x1 block*/
                            for (weightXIndex = 0; weightXIndex < weight_x; weightXIndex++)
                            {
                                vx_uint32 weight = 0;

                                if (weightFomat == VX_TYPE_INT8)
                                    weight = *((vx_int8 *)kernelDataPtr);
                                else if (weightFomat == VX_TYPE_UINT8)
                                    weight = *((vx_uint8 *)kernelDataPtr);
                                else
                                    weight = *((vx_uint16 *)kernelDataPtr);
                                kernelDataPtr = kernelDataPtr + weightSize;

                                {
                                    if (weight != skipValue)
                                        nonZeroCountInDP3++;

                                    /*V7, DP3, one block is 3x1*/
                                    if ((weightXIndex + 1) % 3 == 0 || weightXIndex == wb->weights_sizes[0] - 1)
                                    {
                                        blockCount++;
                                        if (nonZeroCountInDP3)
                                            nonZeroCount++;
                                        nonZeroCountInDP3 = 0;
                                    }
                                }

                                if ((zeroRun == max_zero_run)
                                    || ((sliceIndex == 0)
                                        && (weightXIndex == weight_x - 1)
                                        && (weightYIndex == weight_y - 1)
                                        && needToWriteBias)
                                    || ((weightXIndex == weight_x - 1)
                                        && (weightYIndex == weight_y - 1)
                                        && (sliceIndex == sliceCount - 1))
                                    || (weight != skipValue)
                                    || ((kid == actualFilterCount - 1)
                                        && (weightXIndex == weight_x - 1)
                                        && (weightYIndex == weight_y - 1)
                                        && (group2DCoded == 0)))
                                {
                                    if (min_zero_run_len)
                                    {
                                        writeBits(&kernelBufferPtr, &bitOffset, zeroRun, min_zero_run_len);
                                    }
                                    writeBits(&kernelBufferPtr, &bitOffset, weight, weightBitSize);
                                    zeroRun = 0;

                                    if (needToWriteBias)
                                    {
                                        if (weightFomat == VX_TYPE_INT16)
                                        {
                                            vx_int64 bias64 = 0;
                                            vx_int32 bias32;
                                            vx_int16 bias16;

                                            /*64bits bias*/
                                            if(bias_format == VX_TYPE_INT64)
                                            {
                                                bias32 = (vx_int32)biasData_64bits;
                                                writeBits(&kernelBufferPtr, &bitOffset, bias32, 32);
                                                bias16 = (vx_int16)(biasData_64bits >> 32);
                                                writeBits(&kernelBufferPtr, &bitOffset, (vx_int32)bias16, 16);
                                            }
                                            else    /*32bits bias*/
                                            {
                                                if ((vx_int32)biasData < 0)
                                                {
                                                    bias64 = ~0;
                                                    bias64 = (bias64 >> 32) << 32;
                                                    bias64 = bias64 | (vx_int64)biasData;
                                                }
                                                else
                                                    bias64 = (vx_int64)biasData;

                                                bias32 = (vx_int32)bias64;
                                                writeBits(&kernelBufferPtr, &bitOffset, bias32, 32);
                                                bias16 = (vx_int16)(bias64 >> 32);
                                                writeBits(&kernelBufferPtr, &bitOffset, (vx_int32)bias16, 16);
                                            }
                                        }
                                        else
                                            writeBits(&kernelBufferPtr, &bitOffset, biasData, biasBitSize);
                                        needToWriteBias = 0;
                                    }
                                    group2DCoded = 1;
                                }
                                else
                                {
                                    zeroRun++;
                                }
                            }
                        }

                        /* add offet behind the last point of the last slice of each filter */
                        if (sliceIndex == sliceCount - 1)
                        {
                            vx_uint32 offsetValue;
                            if (z_offset > 0)
                                offsetValue = (vx_uint32)z_offset * filterIndex;
                            else if (outputSize > 0)
                                offsetValue = output_final_x * output_final_y * outputSize * filterIndex;
                            else
                                offsetValue = output_final_x * output_final_y * weightSize * filterIndex;

                            if (wb->zOffsetHandle)
                            {
                                wb->zOffsetHandle[filterIndex].ptrOffset = (vx_uint32)((vx_uint8 *)kernelBufferPtr - wb_base_ptr);
                                wb->zOffsetHandle[filterIndex].bitOffset = bitOffset;
                            }

                            if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                writeBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS_VIP_V7);
                            else
                                writeBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS);

                        }
                    }
                }
            }
            if(blockCount != 0)
                nonZeroRatioPerCorePerVZG[nonZrlIdx] = (vx_float64)nonZeroCount / (vx_float64)blockCount;

            usedKid += actualFilterCount;
            if (filterGroup)
            {
                vxFree(filterGroup);
                filterGroup = VX_NULL;
            }
        }

        /* pad 0 */
        packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);

        /* Go back to update kernelStreamSize. */
        kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);
        /*With RTL bug fix, full cache mode size needn't using maxSize * coreCount*/
        wb->slice_array[index].kernel_stream_full_cache_size += kernelStreamSize;

        compressionRatio = (vx_float64)kernelStreamSize / (vx_float64)coreOrgKernelSize;
        if (wb->max_per_core_compression_ratio < compressionRatio)
            wb->max_per_core_compression_ratio = compressionRatio;

        if (kernelStreamSize > maxKernelStreamSizePerCore)
            maxKernelStreamSizePerCore = kernelStreamSize;

        writeBits(&kernelStreamSizePtr, &streamSizeBitOffset, kernelStreamSize, 32);
#if PRINT_PER_CORE_SIZE
        printf("coreIdx %d, kernel stream size is %d, non-zero coef count is %d\n", coreIndex, kernelStreamSize, nonZeroCoefPerCore[coreIndex]);
#endif
    }

            /*bug1980*/
    if(gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_IMAGE_NOT_PACKED_IN_SRAM_FIX))
        wb->slice_array[index].kernel_align_stream_size += (vx_size)(maxKernelStreamSizePerCore * usedCoreCount );
    else
        wb->slice_array[index].kernel_align_stream_size += (vx_size)(maxKernelStreamSizePerCore * nnCoreCount);


    if (!hasKernelFullCacheInterleaveFix)
    {
        /*if did't fix full cache interleave fix, full cache size also need use align size*/
        wb->slice_array[index].kernel_stream_full_cache_size = wb->slice_array[index].kernel_align_stream_size;
    }

    wb->slice_array[index].kernel_max_stream_size_percore = maxKernelStreamSizePerCore;

    if (z_offset > 0)
        wb->orgZOffsetValue = z_offset;
    else if (outputSize > 0)
        wb->orgZOffsetValue = output_final_x * output_final_y * outputSize;
    else
        wb->orgZOffsetValue = output_final_x * output_final_y * weightSize;

    for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
    {
        for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
        {
            vx_uint32 nonZrlIdx = groupIndex + coreIndex * groupCount;
            if (wb->max_per_core_per_vzgroup_nzr[groupIndex] < nonZeroRatioPerCorePerVZG[nonZrlIdx])
                wb->max_per_core_per_vzgroup_nzr[groupIndex] = nonZeroRatioPerCorePerVZG[nonZrlIdx];
        }
    }

exit:
    if (nonZeroWeights)
        vxFree(nonZeroWeights);
#if PRINT_PER_CORE_SIZE
    if (nonZeroCoefPerCore)
        vxFree(nonZeroCoefPerCore);
#endif
    if (weightsMinusZP)
        vxFree(weightsMinusZP);

    if (nonZeroRatioPerCorePerVZG)
        vxFree(nonZeroRatioPerCorePerVZG);

    return;
}

void fillinTPKernelBuffer(
    vx_weights_biases_parameter wb,
    vx_uint8* zero_run_len_bit_width,
    vx_uint32 weight_z,
    vx_uint32 filter_count,
    vx_uint32 total_weight_z,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_uint32 skip_value,
    vx_uint8_ptr wb_base_ptr,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_uint32 index
    )
{
    vx_uint32 skipValue             = skip_value;
    vx_enum weightFomat             = weight_format;
    vx_uint32 zeroRun               = 0;
    vx_uint32 weightSize            = (vx_uint32)vxDataType_GetSize(weightFomat);
    vx_uint32 weightBitSize         = weightSize * 8;
    vx_uint32 biasSize              = (vx_uint32)vxDataType_GetSize(bias_format);
    vx_uint32 biasBitSize           = biasSize * 8;
    vx_uint32 biasData              = 0;
    vx_uint32 sliceCount            = weight_z;
    vx_uint32 filterSize            = total_weight_z * weightSize;
    vx_uint32 filterCount           = filter_count;
    vx_uint32 kernelStreamSize      = 1;
    vx_uint32* kernelBufferPtr      = (vx_uint32*) wb_base_ptr;
    vx_uint32 bitOffset             = 0;
    vx_uint32* kernelStreamSizePtr  = kernelBufferPtr;
    vx_uint32 streamSizeBitOffset   = 0;
    vx_uint32 alignedOffset         = ((vx_uint32)(gctUINTPTR_T)kernelBufferPtr) & 0x3F;
    vx_uint8_ptr kernelDataPtr      = VX_NULL;
    vx_uint8_ptr kernelStreamBasePtr = VX_NULL;
    vx_uint32 filterIndex, sliceIndex;
    vx_uint32 rsvWeightCount = 0;

    if (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8)
    {
        if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
        {
            biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7;
        }
        else
        {
            biasBitSize = NN_INTEGER_BIAS_BITS;
        }
    }

    /* Fill kernel stream size for each slice. */
    for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
    {
        writeBits(&kernelBufferPtr, &bitOffset, kernelStreamSize, 5);
    }
    packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);

    /* Fill bias for each filter. */
    for (filterIndex = 0; filterIndex < filterCount; filterIndex++)
    {
        if(vxoContext_IsFeatureAvailable(wb->base.context, VX_TP_FEATURE_FP32_BIAS))
        {
            vx_float32 biasFloat32;
           if(bias_format == VX_TYPE_INT64)
            {
                vx_int64  biasData64bits         = 0;
                biasBitSize = biasBitSize >= 32 ? 32: biasBitSize;
                biasData64bits = bias_base_ptr == VX_NULL ? 0 : *((vx_int64 *)bias_base_ptr + filterIndex);
                biasFloat32 = Int64toFp32(biasData64bits, wb->wb_base->biases_fixed_point_pos);
            }
            else
            {
                biasData = bias_base_ptr == VX_NULL ? 0 : *(bias_base_ptr + filterIndex);
                biasFloat32 = Int32toFp32(biasData, wb->wb_base->biases_fixed_point_pos);
            }
           writeBits(&kernelBufferPtr, &bitOffset, *(vx_uint32 *)&biasFloat32, biasBitSize);
        }
        else
        {
            if(bias_format == VX_TYPE_INT64)
            {
                vx_int32 bias32;
                vx_int64  biasData64bits         = 0;
                biasBitSize = biasBitSize >= 32 ? 32: biasBitSize;
                biasData64bits = bias_base_ptr == VX_NULL ? 0 : *((vx_int64 *)bias_base_ptr + filterIndex);
                bias32 = (biasData64bits >>16) & 0xFFFFFFFF;
                writeBits(&kernelBufferPtr, &bitOffset, bias32, biasBitSize);
            }
            else
            {
                biasData = bias_base_ptr == VX_NULL ? 0 : *(bias_base_ptr + filterIndex);
                writeBits(&kernelBufferPtr, &bitOffset, biasData, biasBitSize);
            }
        }
    }
    packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);

    /* Fill weight value for every slice */
    zeroRun = 0;
    for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
    {
        vx_uint8 zeroRunLenBitWidth = zero_run_len_bit_width[sliceIndex];
        vx_uint32 maxZeroRun = (1 << zeroRunLenBitWidth) - 1;

        kernelStreamBasePtr = (vx_uint8_ptr)kernelBufferPtr;
        writeBits(&kernelBufferPtr, &bitOffset, zeroRunLenBitWidth, 4);

        /* add slices of every filter*/
        kernelDataPtr = weight_base_ptr + weightSize * sliceIndex;
        for (filterIndex = 0; filterIndex < filterCount; filterIndex++)
        {
            vx_uint32 weight = 0;

            if (weightFomat == VX_TYPE_INT8)
                weight = *((vx_int8 *)kernelDataPtr);
            else if (weightFomat == VX_TYPE_UINT8)
                weight = *((vx_uint8 *)kernelDataPtr);
            else
                weight = *((vx_uint16 *)kernelDataPtr);
            kernelDataPtr += filterSize;

            if ((zeroRun == maxZeroRun)
                || (weight != skipValue)
                || (filterIndex == (filterCount - 1)))
            {
                if (zeroRunLenBitWidth)
                {
                    writeBits(&kernelBufferPtr, &bitOffset, zeroRun, zeroRunLenBitWidth);
                }
                writeBits(&kernelBufferPtr, &bitOffset, weight, weightBitSize);
                zeroRun = 0;
                rsvWeightCount++;
            }
            else
            {
                zeroRun++;
            }
        }
        packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);

        /* Go back to update kernelStreamSize. */
        kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);
        gcmASSERT((kernelStreamSize & 0x3F) == 0);
        gcmASSERT(kernelStreamSize < 2048);
        writeBits(&kernelStreamSizePtr, &streamSizeBitOffset, kernelStreamSize >> 6, 5);
    }
}

vx_bool WeightBiasBufferAllocate(
    vx_context context,
    vx_weights_biases_parameter weight_bias,
    vx_size size
    )
{
    vx_memory memory;

    gcmASSERT(context);
    gcmASSERT(weight_bias);

    if (!weight_bias->wb_base->memory_head_offset)
    {
        /* weight bias file has marks in its head which is aligned to 64 bytes.
         * ---|0x0A0B0C0D|wb_base|slice_num|slice_array[]|1+offsethandle|1+perf|---
         */
        WB_MEM_HEAD_OFFSET(weight_bias) = gcmALIGN((6 +
                                                    sizeof(vx_weights_biases_parameter_s) +
                                                    sizeof(vx_weights_biases_parameter_base_s) +
                                                    sizeof(vx_weights_biases_slice_s) * weight_bias->slice_num +
                                                    sizeof(vx_weights_biases_z_offset_s) +
                                                    sizeof(vx_arch_perf_s)), 64);
    }

    memory = &weight_bias->memory;

    if (memory->allocated) return vx_true_e;

    size += WB_MEM_HEAD_OFFSET(weight_bias);
    if (context->options.enableAllocateContigousMemForKernel)
    {
        if (context->CurrentContigousSize >= size)
        {
            memory->physicals[0] = *context->Physical;
            *context->Physical += (vx_uint32)size;
            memory->logicals[0] = (vx_uint8_ptr)(*context->Logical);
            (*context->Logical) = (*context->Logical) + size;
            memory->allocType = VXNNE_MEM_POOL_TYPE_ORIG_DDR;
            context->CurrentContigousSize -= (vx_uint32)size;
            if (!vxCreateMutex(OUT &memory->writeLocks[0]))
            {
                memory->writeLocks[0] = VX_NULL;
            }
        }
        else
        {
            if (!vxoMemory_AllocateSize(context, memory, size))
                return vx_false_e;
        }
    }
    else
    {
        if (!vxoMemory_AllocateSize(context, memory, size))
            return vx_false_e;
    }
    memory->allocated = vx_true_e;

    weight_bias->memory_size = size;

    {
        memory->physicals[0] += WB_MEM_HEAD_OFFSET(weight_bias);
    }
    memory->logicals[0] += WB_MEM_HEAD_OFFSET(weight_bias);

    vxoMemory_Dump(memory);

    return vx_true_e;
}

vx_bool vxoNNExternsionAdjustWeightsBiases(
    vx_context                   context,
    vx_weights_biases_parameter  wb,
    vx_size                      wb_size,
    vx_uint32_ptr                kz_num_ptr,
    vx_uint32_ptr                z_num_ptr,
    vx_uint32_ptr                kz_array,
    vx_uint32_ptr                z_array
    )
{
#define FC_SIZE_MAX 134217728
    vx_uint32 i = 0, tmp = wb->weights_sizes[3];
    vx_size max = FC_SIZE_MAX;

    if (wb_size > max)
    {
        /* Weight_bias data cannot large than 128MB in old hw version. Current only for vgg FC layer. */
        do
        {
            if (tmp >= 1024)
            {
                z_array[i++] = 1024;
                tmp -= 1024;
            }
            else
            {
                z_array[i++] = tmp;
                break;
            }
        } while (tmp && i < MAX_WEIGHT_BIAS_GROUPS);

        if (i > MAX_WEIGHT_BIAS_GROUPS) return vx_false_e;
        else *z_num_ptr = i;
    }
    else
    {
        z_array[0] = wb->weights_sizes[3];
        *z_num_ptr = 1;
    }

    if (wb->weights_sizes[0] == 1 && wb->weights_sizes[1] == 1 && wb->weights_sizes[2] >= context->options.fcZMax)
    {
        vx_uint32 y = 4;
        vx_uint32 z = wb->weights_sizes[2] / y;

        while (z >= 16384)
        {
            y <<= 1;
            z = wb->weights_sizes[2] / y;
        }

        wb->weights_sizes[1] = y;
        wb->weights_sizes[2] = z;
    }

    kz_array[0] = wb->weights_sizes[2];
    *kz_num_ptr = 1;

    return vx_true_e;
}

vx_weights_biases_parameter_base vxoWeightsBiasesBase_Create(
    vx_context  context,
    vx_enum     layer_type,
    vx_uint32 * inputs_dims,
    vx_uint32   pad_x_left,
    vx_uint32   pad_x_right,
    vx_uint32   pad_y_top,
    vx_uint32   pad_y_bottom,
    vx_uint32   pooling_size_x,
    vx_uint32   pooling_size_y,
    vx_uint32   stride_x,
    vx_uint32   stride_y,
    vx_enum     down_scale_size_rounding,
    vx_uint32 * outputs_dims,
    vx_uint32 * pooling_outputs_dims,
    vx_uint32   weights_num_of_dims,
    vx_uint32 * weights_dims,
    vx_enum     weights_data_format,
    vx_enum     weights_quant_format,
    vx_int8     weights_fixed_point_pos,
    vx_int32    weights_zero_point,
    vx_float32  weights_scale,
    vx_uint32   biases_num_of_dims,
    vx_uint32 * biases_dims,
    vx_enum     biases_data_format,
    vx_enum     biases_quant_format,
    vx_int8     biases_fixed_point_pos,
    vx_float32  biases_scale,
    vx_weights_biases_parameter_optimizations_t *optimizations
    )
{
    vx_weights_biases_parameter_base wb_base = VX_NULL;
    vx_uint32 strideX = 1, strideY = 1;
    vx_uint32 alignedWidth;
    vx_uint32 alignedHeight;

    vxmASSERT(context);

    wb_base = (vx_weights_biases_parameter_base)vxoReference_Create(context, VX_TYPE_WEIGHTS_BIASES_PARAMETER_BASE, VX_REF_INTERNAL, &context->base);
    if (vxoReference_GetStatus((vx_reference)wb_base) != VX_SUCCESS)
    {
        vxError("vxoWeightsBiases_CreateBase: FAIL TO CREATE WB BASE\n");
        return VX_NULL;
    }

    wb_base->weights_num_of_dims = weights_num_of_dims;
    wb_base->weights_data_format = weights_data_format;
    wb_base->weights_quant_format = weights_quant_format;
    wb_base->weights_fixed_point_pos = weights_fixed_point_pos;
    wb_base->biases_num_of_dims = biases_num_of_dims;
    wb_base->biases_data_format = biases_data_format;
    wb_base->biases_quant_format = biases_quant_format;
    wb_base->biases_fixed_point_pos = biases_fixed_point_pos;
    wb_base->pooling_size_x = pooling_size_x;
    wb_base->pooling_size_y = pooling_size_y;
    wb_base->pad_x_left = pad_x_left;
    wb_base->pad_x_right = pad_x_right;
    wb_base->pad_y_top = pad_y_top;
    wb_base->pad_y_bottom = pad_y_bottom;
    wb_base->down_scale_size_rounding = down_scale_size_rounding;
    wb_base->setZeroLength = -1;
    if (optimizations) wb_base->setZeroLength = optimizations->zrl;
    wb_base->inputZP = 0;
    if (optimizations) wb_base->inputZP = optimizations->inputZeroPoint;
    wb_base->coefZP    = weights_zero_point;
    wb_base->coefScale = weights_scale;
    wb_base->biasScale = biases_scale;

    if (wb_base->weights_data_format == VX_TYPE_UINT8)
    {
        /* Current ZeroPoint range is 0-255(uint8)*/
        gcmASSERT(wb_base->inputZP <= 255 && wb_base->coefZP <= 255);
    }

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT) && wb_base->weights_data_format == VX_TYPE_UINT8)
        wb_base->skipValue = wb_base->coefZP;
    else
        wb_base->skipValue = 0;

    vxMemCopy(wb_base->org_weights_sizes, weights_dims, weights_num_of_dims * sizeof(vx_uint32));
    if (biases_dims && biases_num_of_dims)
        vxMemCopy(wb_base->biases_sizes, biases_dims, biases_num_of_dims * sizeof(vx_uint32));

    wb_base->weights_sizes[3] = weights_dims[3];

    if (outputs_dims != VX_NULL && inputs_dims != NULL)
    {
        if (layer_type == VX_NN_FULLYCONNECTED_LAYER && inputs_dims[0] != 1 && inputs_dims[1] == 1)
            wb_base->nn_fc_batch_mode = vx_true_e;

        if ((stride_x > 0) && (stride_y > 0))
        {
            strideX = stride_x;
            strideY = stride_y;
        }
        else if (layer_type == VX_NN_FULLYCONNECTED_LAYER)
        {
            /* it is fully connected layer */
            strideX = strideY = 1;
        }
        else
        {
            /* Calculate stride = (w + pad_x_left + pad_x_right - weight)/(output_w - 1) */
            strideX = (outputs_dims[0] == 1) ? 1 :
                vxoNNExternsionConvlutionRound((vx_float32)(inputs_dims[0] + pad_x_left + pad_x_right - weights_dims[0]) / (outputs_dims[0] - 1), down_scale_size_rounding);
            strideY = (outputs_dims[1] == 1) ? 1 :
                vxoNNExternsionConvlutionRound((vx_float32)(inputs_dims[1] + pad_y_top + pad_y_bottom - weights_dims[1]) / (outputs_dims[1] - 1), down_scale_size_rounding);
        }
    }

    wb_base->strideX = strideX;
    wb_base->strideY = strideY;

    if (((strideX > 1) || (strideY > 1)) && (weights_dims[0] == 1) && (weights_dims[1] == 1))
    {
        wb_base->weights_sizes[0] = weights_dims[0];
        wb_base->weights_sizes[1] = weights_dims[1];
        wb_base->weights_sizes[2] = weights_dims[2];
    }
    else
    {
        /* Calculate weight_bias' weight width & height */
        alignedWidth = ((weights_dims[0] % strideX == 0) ? weights_dims[0] : (weights_dims[0] + (strideX - weights_dims[0] % strideX)));
        alignedHeight = ((weights_dims[1] % strideY == 0) ? weights_dims[1] : (weights_dims[1] + (strideY - weights_dims[1] % strideY)));
        wb_base->weights_sizes[0] = alignedWidth / strideX;
        wb_base->weights_sizes[1] = alignedHeight / strideY;
        wb_base->weights_sizes[2] = weights_dims[2] * strideX * strideY;
    }

    if (outputs_dims != VX_NULL && pooling_outputs_dims != VX_NULL)
        wb_base->pooling_stride = outputs_dims[0] / pooling_outputs_dims[0];
    else
        wb_base->pooling_stride = 1;

    wb_base->memory_pad = 64;

    return wb_base;
}

VX_INTERNAL_CALLBACK_API void vxoWeightsBiasesBase_Destructor(vx_reference ref)
{
    vx_weights_biases_parameter_base wbBase = (vx_weights_biases_parameter_base)ref;

    if (wbBase->zrlTpFcPtr != VX_NULL)
    {
        vxFree(wbBase->zrlTpFcPtr);
        wbBase->zrlTpFcPtr = VX_NULL;
    }

    if (wbBase->reshuffleWeightPtr != VX_NULL)
    {
        vxFree(wbBase->reshuffleWeightPtr);
        wbBase->reshuffleWeightPtr = VX_NULL;
    }

    if (wbBase->origWeight != VX_NULL)
    {
        vxoReference_Release((vx_reference_ptr)&wbBase->origWeight, VX_TYPE_TENSOR, VX_REF_INTERNAL);
        wbBase->origWeight = VX_NULL;
    }

    if (wbBase->origBias != VX_NULL)
    {
        vxoReference_Release((vx_reference_ptr)&wbBase->origBias, VX_TYPE_TENSOR, VX_REF_INTERNAL);
        wbBase->origBias = VX_NULL;
    }

    if (wbBase->origAlpha != VX_NULL)
    {
        vxoReference_Release((vx_reference_ptr)&wbBase->origAlpha, VX_TYPE_TENSOR, VX_REF_INTERNAL);
        wbBase->origAlpha = VX_NULL;
    }
}

vx_weights_biases_parameter vxoWeightsBiases_Create(
    vx_context                       context,
    vx_weights_biases_parameter_base wb_base,
    vx_uint32 *                      weight_dims,
    vx_enum                          layer_type,
    vx_bool                          first_time
    )
{
    vx_status status = VX_SUCCESS;
    vx_weights_biases_parameter wb = VX_NULL;
    vx_uint32 zArray[MAX_ZGROUP_COUNT], kzArray[MAX_KZGROUP_COUNT];
    vx_uint32 sliceCount = 0, filterCount = 0;
    vx_size minTotalKernelBufferSize = 0;
    vx_size minKernelBufferSize[MAX_ZGROUP_COUNT*MAX_KZGROUP_COUNT];
    vx_uint8_ptr zrlTmpPtr = VX_NULL;
    vx_uint32 kzNum = 0, zNum = 0, oneFilterSize = 0, weightSize = 0, i = 0, j = 0, kzoffset = 0;
    vx_uint8_ptr weight_ptr = VX_NULL, Weight_Gpuptr = VX_NULL, Weight_Cpuptr = VX_NULL;

    vxmASSERT(context);
    vxmASSERT(wb_base);

    if (wb_base->reshuffleWeightPtr)
    {
        weight_ptr = wb_base->reshuffleWeightPtr;
    }
    else
    {
        vx_uint32 size = 0;
        size = wb_base->weights_sizes[0] * wb_base->weights_sizes[1] * wb_base->weights_sizes[2] * wb_base->weights_sizes[3] *
               vxDataType_GetSize((vx_type_e)wb_base->weights_data_format);;
        Weight_Cpuptr = (vx_uint8_ptr)vxAllocate(size);
        if (Weight_Cpuptr == VX_NULL)
        {
            status = VX_ERROR_NO_MEMORY;
            goto exit;
        }

        vxoTensor_GetTensorViewMemory(wb_base->origWeight, (gctPOINTER*)(&Weight_Gpuptr), VX_NULL);
        vxMemCopy(Weight_Cpuptr, Weight_Gpuptr, size);
        weight_ptr = Weight_Cpuptr;
    }

    wb = (vx_weights_biases_parameter)vxoReference_Create(context, VX_TYPE_WEIGHTS_BIASES_PARAMETER, VX_REF_INTERNAL, &context->base);
    if (vxoReference_GetStatus((vx_reference)wb) != VX_SUCCESS)
    {
        vxError("vxoWeightsBiases_Create: FAIL TO CREATE WB\n");
        goto exit;
    }

    wb->wb_base = wb_base;
    wb->weights_sizes[0] = weight_dims[0];
    wb->weights_sizes[1] = weight_dims[1];
    wb->weights_sizes[2] = weight_dims[2];
    wb->weights_sizes[3] = weight_dims[3];

    if (weight_ptr != VX_NULL)
    {
        vx_uint32 splitIndex = 0, index = 0, woffset = 0, acount = 0, nzcount = 0, asize = 0;

        weightSize = (vx_uint32)vxDataType_GetSize(wb_base->weights_data_format);
        oneFilterSize = wb_base->weights_sizes[0] *
                        wb_base->weights_sizes[1] *
                        wb_base->weights_sizes[2] *
                        weightSize;

        /* Estimate sub wb stream size first */
        if (layer_type == VX_NN_FULLYCONNECTED_LAYER &&
            vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_SINGLE_FC) &&
            weight_dims[3] > 1 && wb_base->nn_fc_batch_mode == vx_false_e &&
            ((!context->options.enableForce64BitsBiasNN && wb_base->biases_data_format == VX_TYPE_INT64) || wb_base->biases_data_format != VX_TYPE_INT64))
        {
            vx_uint32 coreCount = context->nnConfig.fixedFeature.tpCoreCount + context->nnConfig.fixedFeature.tpliteCoreCount;
            sliceCount = weight_dims[2];
            filterCount = weight_dims[3];

            gcmASSERT(weight_dims[0] == 1);
            gcmASSERT(weight_dims[1] == 1);

            /* TP FC can handle up to 512 filters. */
            if (context->options.enableMultiTP && filterCount >= 2 * coreCount)
            {
                /* multi TP path */
                vx_uint32 snum = (filterCount + TP_FC_Z_MAX - 1) / TP_FC_Z_MAX;
                i = snum % coreCount ? gcmALIGN_NP2(snum, coreCount) : snum;
                calculateSplitSize(filterCount, i, zArray, VX_NULL);
            }
            else
            {
                for (;;)
                {
                    if (filterCount > TP_FC_Z_MAX)
                    {
                        if (i < MAX_ZGROUP_COUNT)
                            zArray[i] = TP_FC_Z_MAX;
                        filterCount -= TP_FC_Z_MAX;
                        i++;
                    }
                    else
                    {
                        if (i < MAX_ZGROUP_COUNT)
                            zArray[i] = filterCount;
                        i++;
                        break;
                    }
                }
            }

            if (i > MAX_ZGROUP_COUNT)
                goto exit;
            zNum = i;
            //kzNum = (sliceCount % (0x1 << 16) == 0 ) ? sliceCount / (0x1 << 16) : sliceCount / (0x1 << 16) + 1;
            kzNum = (sliceCount % (0x1 << MAX_KZGROUP_COUNT) == 0 ) ? sliceCount / (0x1 << MAX_KZGROUP_COUNT) : sliceCount / (0x1 << MAX_KZGROUP_COUNT) + 1;
            for(splitIndex = 0; splitIndex < kzNum; splitIndex ++)
            {
                if(splitIndex == (kzNum - 1))
                {
                    kzArray[splitIndex] = (sliceCount - (0x1 << MAX_KZGROUP_COUNT) * splitIndex);
                }
                else
                {
                    kzArray[splitIndex] = 0x1 << MAX_KZGROUP_COUNT;
                }
            }

            //calculateSplitSize(sliceCount, kzNum, kzArray, VX_NULL);
            zrlTmpPtr = wb_base->zrlTpFcPtr = (vx_uint8 *)vxAllocate(weight_dims[2] * zNum);
            if (wb_base->zrlTpFcPtr == NULL)
            {
                status = VX_ERROR_NO_MEMORY;
                goto exit;
            }

            wb->use_tp_fc = vx_true_e;
        }
        else if (!vxoNNExternsionAdjustWeightsBiases(context, wb,
                                                     oneFilterSize * wb_base->weights_sizes[3],
                                                     &kzNum, &zNum, kzArray, zArray))
        {
            status = VX_FAILURE;
            goto exit;
        }

        /* Create slice for each split wb */
        wb->slice_num = kzNum * zNum;
        wb->slice_array = (vx_weights_biases_slice)vxAllocateAndZeroMemory(sizeof(vx_weights_biases_slice_s) * wb->slice_num);
        if (wb->slice_array == VX_NULL)
        {
            status = VX_ERROR_NO_MEMORY;
            goto exit;
        }

        if ((vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_COMPRESSION_ENHANCEMENT) && wb->use_tp_fc) ||
            (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_COEF_COMPRESSION_ENHANCEMENT) && !wb->use_tp_fc))
        {
            wb->huffmanConfig = (vx_weights_biases_huffman_cfg)vxAllocateAndZeroMemory(sizeof(vx_weights_biases_huffman_cfg_s) * wb->slice_num);
            if (wb->huffmanConfig == VX_NULL)
            {
                status = VX_ERROR_NO_MEMORY;
                goto exit;
            }
        }

        for (i = 0; i < zNum; i++)
        {
            kzoffset = 0;
            filterCount = zArray[i];

            for (j = 0; j < kzNum; j++)
            {
                sliceCount = kzArray[j];

                if (wb->use_tp_fc)
                {
                    calculateWeightBiasTPBufferRelatedSize(
                        wb,
                        weight_ptr + kzoffset + woffset,
                        sliceCount,
                        filterCount,
                        wb_base->weights_sizes[2],
                        wb_base->weights_data_format,
                        wb_base->biases_data_format,
                        wb_base->skipValue,
                        wb_base->setZeroLength >= 0 && wb_base->setZeroLength <= 9 ?
                            wb_base->setZeroLength : (vx_int8)context->options.tpZeroRunLen,
                        index,
                        &minKernelBufferSize[index],
                        zrlTmpPtr);

                    zrlTmpPtr += sliceCount;
                    minTotalKernelBufferSize += minKernelBufferSize[index];
                }
                else
                {
                    vx_uint32 nnCoreCount = (wb_base->weights_data_format == VX_TYPE_INT16) ?
                                    context->nnConfig.fixedFeature.nnCoreCountInt16 : (wb_base->weights_data_format == VX_TYPE_FLOAT16) ?
                                        context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;
                    if (nnCoreCount == 0)
                    {
                        vxInfo("%s[%d]: current NN didn't support this format 0x%x\n", __FUNCTION__, __LINE__, wb->wb_base->weights_data_format);
                        goto exit;
                    }

                    calculateWeightBiasStreamRelatedSize(
                        context,
                        wb,
                        weight_ptr + kzoffset + woffset,
                        sliceCount, /* slice */
                        filterCount, /* z count */
                        filterCount, /* kernel per core */
                        wb_base->weights_data_format,
                        wb_base->biases_data_format,
                        wb_base->skipValue,
                        weight_dims[2] <= 1 ? 0 : wb_base->setZeroLength,
                        (vx_uint8)context->options.nnZeroRunLen,
                        0,
                        VX_NULL, VX_NULL, VX_NULL);

                    minTotalKernelBufferSize += wb->slice_array[index].kernel_stream_size;
                }

                kzoffset += sliceCount * weightSize;

                acount  += wb->slice_array[index].all_count;
                nzcount += wb->slice_array[index].non_zero_count;
                asize   += (vx_uint32)wb->slice_array[index].kernel_orig_size;

                index++;
            }

            woffset += oneFilterSize * filterCount;
        }

        if (Weight_Cpuptr != VX_NULL)
        {
            vxFree(Weight_Cpuptr);
            Weight_Cpuptr = VX_NULL;
        }

        wb->non_zero_ratio = gcmMIN(1.0f, (vx_float64)nzcount / acount);
        if ((wb->weights_sizes[0] == 1 && wb->weights_sizes[1] == 1
        && (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3) || vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6))
        && (wb->wb_base->weights_data_format == VX_TYPE_INT8 || wb->wb_base->weights_data_format == VX_TYPE_UINT8))
        && !gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_ZDP3_NO_COMPRESS_FIX)
        && !wb->use_tp_fc)
        {
            /*force non_zero_ratio to 1 as HW will not be skipping any zero without ZDP3_NO_COMPRESS_FIX*/
            wb->non_zero_ratio = 1.0f;
        }
        wb->general_compression_ratio = (vx_float64)minTotalKernelBufferSize / asize;

        if (wb->use_tp_fc) wb->wb_base->tpFcStreamTotalSize = minTotalKernelBufferSize;

        for (i = 0; i < zNum; i++)
        {
            for (j = 0; j < kzNum; j++)
            {
                vx_uint32 index = i * kzNum + j;
                wb->slice_array[index].kz_count = kzArray[j];
                wb->slice_array[index].z_count = zArray[i];
            }
        }
        wb->slice_z_num = zNum;
        wb->slice_kz_num = kzNum;

        /* Calculate arch perf in this step and pass it to operation for non-swtiling path. */
        wb->archPerfHandle = (vx_arch_perf)vxAllocateAndZeroMemory(sizeof(vx_arch_perf_s));
        if (wb->archPerfHandle == VX_NULL)
        {
            status = VX_ERROR_NO_MEMORY;
            goto exit;
        }
    }

    if (!first_time) vxoReference_Increment(&wb_base->base, VX_REF_INTERNAL);

exit:
    if (Weight_Cpuptr != VX_NULL)
    {
        vxFree(Weight_Cpuptr);
        Weight_Cpuptr = VX_NULL;
    }
    return status == VX_SUCCESS ? wb : VX_NULL;
}

vx_status vxoWeightsBiases_Compress(
    vx_context                       context,
    vx_weights_biases_parameter      wb,
    vx_uint32                        kernel_per_core,
    vx_uint32 *                      pooling_output_dims,
    vx_enum                          output_format,
    vx_int32                         z_offset
    )
{
    vx_status status = VX_SUCCESS;
    vx_size minKernelBufferSize[MAX_ZGROUP_COUNT*MAX_KZGROUP_COUNT];
    vx_uint8 minZeroRunLen[MAX_ZGROUP_COUNT*MAX_KZGROUP_COUNT] = {0};
    vx_uint32 maxZeroRunLen[MAX_ZGROUP_COUNT] = {0};
    vx_size minTotalKernelBufferSize = 0;
    vx_uint32 oneFilterSize, weightSize, i, j, kzoffset, sliceCount, filterCount;
    vx_weights_biases_parameter_base wb_base = wb->wb_base;
    vx_uint8_ptr zrlTmpPtr = VX_NULL, minZeroRunLenTPFC = wb_base->zrlTpFcPtr;
    vx_uint8_ptr weight_ptr = VX_NULL, Weight_Gpuptr = VX_NULL, Weight_Cpuptr = VX_NULL;
    vx_uint32_ptr bias_ptr = VX_NULL, Bias_Gpuptr = VX_NULL, Bias_Cpuptr = VX_NULL;
    vx_uint32 size = 0;

    vxmASSERT(context);
    vxmASSERT(wb);
    vxmASSERT(pooling_output_dims != VX_NULL || z_offset > 0);

    if (WB_MEM_SIZE_INDEX(wb, 0) > 0) return status;

    if (WB_BASE(wb)->reshuffleWeightPtr != VX_NULL)
    {
        weight_ptr = WB_BASE(wb)->reshuffleWeightPtr;
    }
    else
    {
        size = wb_base->weights_sizes[0] * wb_base->weights_sizes[1] * wb_base->weights_sizes[2] * wb_base->weights_sizes[3] *
               vxDataType_GetSize((vx_type_e)wb_base->weights_data_format);;
        Weight_Cpuptr = (vx_uint8_ptr)vxAllocate(size);
        if (Weight_Cpuptr == VX_NULL)
        {
            status = VX_ERROR_NO_MEMORY;
            goto exit;
        }

        vxoTensor_GetTensorViewMemory(wb_base->origWeight, (gctPOINTER*)(&Weight_Gpuptr), VX_NULL);
        vxMemCopy(Weight_Cpuptr, Weight_Gpuptr, size);
        weight_ptr = Weight_Cpuptr;
    }

    if (WB_BIAS_TENSOR(wb))
    {
        size = wb_base->weights_sizes[3] * vxDataType_GetSize((vx_type_e)wb_base->biases_data_format);
        Bias_Cpuptr = (vx_uint32_ptr)vxAllocate(size);
        if (Bias_Cpuptr == VX_NULL)
        {
            status = VX_ERROR_NO_MEMORY;
            goto exit;
        }

        vxoTensor_GetTensorViewMemory(WB_BIAS_TENSOR(wb), (gctPOINTER*)(&Bias_Gpuptr), VX_NULL);
        vxMemCopy(Bias_Cpuptr, Bias_Gpuptr, size);
        bias_ptr = Bias_Cpuptr;
    }

    vxmASSERT(weight_ptr != VX_NULL);

    weightSize = (vx_uint32)vxDataType_GetSize((vx_type_e)wb_base->weights_data_format);
    oneFilterSize = wb_base->weights_sizes[0] *
                    wb_base->weights_sizes[1] *
                    wb_base->weights_sizes[2] *
                    weightSize;

    if (!wb->use_tp_fc)
    {
        if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP0))
        {
            /* caclulate V8 HUFFMAN stream size need real kernels per core*/
            minTotalKernelBufferSize = calcKernelSizeV8Huffman(
                                        context,
                                        wb,
                                        wb->weights_sizes[0],
                                        wb->weights_sizes[1],
                                        wb->weights_sizes[2],
                                        wb->weights_sizes[3],
                                        kernel_per_core,
                                        wb_base->weights_data_format,
                                        wb_base->skipValue,
                                        weight_ptr,
                                        bias_ptr,
                                        VX_NULL,
                                        VX_NULL,
                                        VX_NULL,
                                        VX_NULL,
                                        0
                                        );
            minKernelBufferSize[0] = minTotalKernelBufferSize;
        }
        else if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_COEF_COMPRESSION_ENHANCEMENT))
        {
            /* caclulate V7 HUFFMAN stream size need bias real data*/
            minTotalKernelBufferSize = calcKernelStreamSizeHuffman(
                                        context,
                                        wb,
                                        wb->weights_sizes[0],
                                        wb->weights_sizes[1],
                                        wb->weights_sizes[2],
                                        wb->weights_sizes[3],
                                        kernel_per_core,
                                        wb_base->weights_data_format,
                                        wb_base->biases_data_format,
                                        wb_base->inputZP,
                                        wb_base->coefZP,
                                        wb_base->skipValue,
                                        pooling_output_dims == VX_NULL ? 0 : pooling_output_dims[0],
                                        pooling_output_dims == VX_NULL ? 0 : pooling_output_dims[1],
                                        z_offset,
                                        vxDataType_GetSize((vx_type_e)output_format),
                                        weight_ptr,
                                        (bias_ptr != VX_NULL) ? bias_ptr: VX_NULL,
                                        0
                                        );
            minKernelBufferSize[0] = minTotalKernelBufferSize;
        }
        else
        {
            vx_uint32 index = 0;
            vx_size weightDataBytesOffset = 0;

            for (i = 0; i < wb->slice_z_num; i++)
            {
                filterCount = wb->slice_array[index].z_count;
                kzoffset = 0;

                for (j = 0; j < wb->slice_kz_num; j++)
                {
                    sliceCount = wb->slice_array[index].kz_count;

                    calculateWeightBiasStreamRelatedSize(
                        context,
                        wb,
                        weight_ptr + weightDataBytesOffset + kzoffset,
                        sliceCount, /* slice */
                        filterCount, /* z count */
                        kernel_per_core, /* kernel per core */
                        wb_base->weights_data_format,
                        wb_base->biases_data_format,
                        wb_base->skipValue,
                        /*wb->weights_sizes[2] <= 1 ? 0 : */wb_base->setZeroLength,
                        (vx_uint8)context->options.nnZeroRunLen,
                        0,
                        &minKernelBufferSize[index], &minZeroRunLen[index], &maxZeroRunLen[index]);

                    kzoffset +=  (wb->wb_base->weights_sizes[0] * wb->wb_base->weights_sizes[1]* sliceCount * weightSize);
                    minTotalKernelBufferSize += minKernelBufferSize[index];
                    index++;
                }

                weightDataBytesOffset += oneFilterSize * filterCount;
            }
        }
    }
    else
    {
        minTotalKernelBufferSize = wb_base->tpFcStreamTotalSize;
    }

    /* Allocate memory for sub wb */
    if (!WeightBiasBufferAllocate(context,
                                  wb,
                                  minTotalKernelBufferSize + wb_base->memory_pad * wb->slice_num))
    {
        status = VX_ERROR_NO_MEMORY;
        goto exit;
    }

    /* Compress sub wb */
    vxAcquireMutex(wb->memory.writeLocks[0]);
    {
        vx_size weightDataBytesOffset = 0;
        vx_size biasDataDWordOffset = 0;
        vx_size compressDataBytesOffset = 0;
        vx_uint32 index = 0;

        kzoffset = 0;
        zrlTmpPtr = minZeroRunLenTPFC;
        for (i = 0; i < wb->slice_z_num; i++)
        {
            kzoffset = 0;
            filterCount = wb->slice_array[index].z_count;

            for (j = 0; j < wb->slice_kz_num; j++)
            {
                vx_uint8_ptr kernelBufferPtr = wb->memory.logicals[0] + compressDataBytesOffset;

                sliceCount = wb->slice_array[index].kz_count;

                if (wb->use_tp_fc &&
                    vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_COMPRESSION_ENHANCEMENT))
                {
                    fillinTPKernelBufferHuffman(
                        wb,
                        zrlTmpPtr,
                        sliceCount,
                        filterCount,
                        wb_base->weights_sizes[2],
                        wb_base->weights_data_format,
                        wb_base->biases_data_format,
                        wb_base->skipValue,
                        kernelBufferPtr,
                        weight_ptr + kzoffset + weightDataBytesOffset,
                        !j ? ((bias_ptr != VX_NULL) ? bias_ptr + biasDataDWordOffset : VX_NULL) : VX_NULL,
                        index);

                    zrlTmpPtr += sliceCount;
                }
                else if (wb->use_tp_fc)
                {
                    fillinTPKernelBuffer(
                        wb,
                        zrlTmpPtr,
                        sliceCount,
                        filterCount,
                        wb_base->weights_sizes[2],
                        wb_base->weights_data_format,
                        wb_base->biases_data_format,
                        wb_base->skipValue,
                        kernelBufferPtr,
                        weight_ptr + kzoffset + weightDataBytesOffset,
                        !j ? ((bias_ptr != VX_NULL) ? bias_ptr + biasDataDWordOffset : VX_NULL) : VX_NULL,
                        index);

                    zrlTmpPtr += sliceCount;
                }
                else
                {
                    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP0))
                    {
                        fillinKernelBufferV8Huffman(
                            context,
                            wb,
                            wb->weights_sizes[0],
                            wb->weights_sizes[1],
                            sliceCount,
                            filterCount,
                            kernel_per_core,
                            wb_base->weights_data_format,
                            wb_base->biases_data_format,
                            wb_base->inputZP,
                            wb_base->coefZP,
                            wb_base->skipValue,
                            pooling_output_dims == VX_NULL ? 0 : pooling_output_dims[0],
                            pooling_output_dims == VX_NULL ? 0 : pooling_output_dims[1],
                            z_offset,
                            vxDataType_GetSize((vx_type_e)output_format),
                            kernelBufferPtr,
                            weight_ptr + kzoffset + weightDataBytesOffset,
                            (bias_ptr != VX_NULL) ? bias_ptr + biasDataDWordOffset : VX_NULL,
                            VX_NULL,
                            VX_NULL,
                            wb->wb_base->origAlpha,
                            index);
                    }
                    else if (wb->wb_base->hw_depth_wise)
                    {
                        fillinDepthWiseKernelBuffer(
                            context,
                            wb,
                            minZeroRunLen[index],
                            maxZeroRunLen[index],
                            wb->weights_sizes[0],
                            wb->weights_sizes[1],
                            sliceCount,
                            filterCount,
                            kernel_per_core,
                            wb_base->weights_data_format,
                            wb_base->biases_data_format,
                            wb_base->inputZP,
                            wb_base->coefZP,
                            wb_base->skipValue,
                            pooling_output_dims == VX_NULL ? 0 : pooling_output_dims[0],
                            pooling_output_dims == VX_NULL ? 0 : pooling_output_dims[1],
                            z_offset,
                            vxDataType_GetSize((vx_type_e)output_format),
                            kernelBufferPtr,
                            weight_ptr + kzoffset + weightDataBytesOffset,
                            (bias_ptr != VX_NULL) ? bias_ptr + biasDataDWordOffset : VX_NULL,
                            index);
                    }
                    else if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_COEF_COMPRESSION_ENHANCEMENT))
                    {
                        fillinKernelBufferHuffman(
                            context,
                            wb,
                            wb->weights_sizes[0],
                            wb->weights_sizes[1],
                            sliceCount,
                            filterCount,
                            kernel_per_core,
                            wb_base->weights_data_format,
                            wb_base->biases_data_format,
                            wb_base->inputZP,
                            wb_base->coefZP,
                            wb_base->skipValue,
                            pooling_output_dims == VX_NULL ? 0 : pooling_output_dims[0],
                            pooling_output_dims == VX_NULL ? 0 : pooling_output_dims[1],
                            z_offset,
                            vxDataType_GetSize((vx_type_e)output_format),
                            kernelBufferPtr,
                            weight_ptr + kzoffset + weightDataBytesOffset,
                            (bias_ptr != VX_NULL) ? bias_ptr + biasDataDWordOffset : VX_NULL,
                            index
                            );
                    }
                    else if (context->options.enableNonZeroBalance)
                    {
                        fillinKernelBufferBalance(
                            context,
                            wb,
                            minZeroRunLen[index],
                            maxZeroRunLen[index],
                            wb->weights_sizes[0],
                            wb->weights_sizes[1],
                            sliceCount,
                            filterCount,
                            kernel_per_core,
                            wb_base->weights_data_format,
                            wb_base->biases_data_format,
                            wb_base->inputZP,
                            wb_base->coefZP,
                            wb_base->skipValue,
                            pooling_output_dims == VX_NULL ? 0 : pooling_output_dims[0],
                            pooling_output_dims == VX_NULL ? 0 : pooling_output_dims[1],
                            z_offset,
                            vxDataType_GetSize((vx_type_e)output_format),
                            kernelBufferPtr,
                            weight_ptr + kzoffset + weightDataBytesOffset,
                            (bias_ptr != VX_NULL) ? bias_ptr + biasDataDWordOffset : VX_NULL,
                            index);
                    }
                    else
                    {
                        fillinKernelBuffer(
                            context,
                            wb,
                            minZeroRunLen[index],
                            maxZeroRunLen[index],
                            wb->weights_sizes[0],
                            wb->weights_sizes[1],
                            sliceCount,
                            filterCount,
                            kernel_per_core,
                            wb_base->weights_data_format,
                            wb_base->biases_data_format,
                            wb_base->inputZP,
                            wb_base->coefZP,
                            wb_base->skipValue,
                            pooling_output_dims == VX_NULL ? 0 : pooling_output_dims[0],
                            pooling_output_dims == VX_NULL ? 0 : pooling_output_dims[1],
                            z_offset,
                            vxDataType_GetSize((vx_type_e)output_format),
                            kernelBufferPtr,
                            weight_ptr + kzoffset + weightDataBytesOffset,
                            (bias_ptr != VX_NULL) ? bias_ptr + biasDataDWordOffset : VX_NULL,
                            index);
                    }
                }

                kzoffset += sliceCount * weightSize;

                wb->slice_array[index].memory_offset = compressDataBytesOffset;
                wb->slice_array[index].memory_size = wb_base->memory_pad + (wb->use_tp_fc ?
                                                        wb->slice_array[index].kernel_stream_size : minKernelBufferSize[index]);
                compressDataBytesOffset += wb->slice_array[index].memory_size;

                index++;
            }

            weightDataBytesOffset += oneFilterSize * filterCount;
            biasDataDWordOffset += filterCount;
        }
    }
    vxReleaseMutex(wb->memory.writeLocks[0]);

#if gcdDUMP
    gcmDUMP(gcvNULL, "#[weights and biases]\n");
    gcmDUMP_BUFFER(gcvNULL,
                   gcvDUMP_BUFFER_MEMORY,
                   wb->memory.physicals[0],
                   (gctPOINTER)wb->memory.logicals[0],
                   0,
                   wb->memory_size - wb->wb_base->memory_head_offset);
#endif

exit:
    if (Weight_Cpuptr != VX_NULL)
    {
        vxFree(Weight_Cpuptr);
        Weight_Cpuptr = VX_NULL;
    }

    if (Bias_Cpuptr != VX_NULL)
    {
        vxFree(Bias_Cpuptr);
        Bias_Cpuptr = VX_NULL;
    }
    return status;
}

vx_status vxoWeightsBiases_Decompress(
    vx_context                       context,
    vx_weights_biases_parameter      wb
    )
{
    vx_uint32 index;

    for (index = 0; index < wb->slice_num; index++)
    {
        wb->slice_array[index].memory_offset = 0;
        wb->slice_array[index].memory_size = 0;
    }

    if (wb->memory.nodePtrs[0] != VX_NULL)
    {
        vxoMemory_Free(wb->base.context, &wb->memory);
    }

    wb->memory.allocated = vx_false_e;
    wb->memory_size = 0;

    return VX_SUCCESS;
}

VX_INTERNAL_CALLBACK_API void vxoWeightsBiases_Destructor(vx_reference ref)
{
    vx_weights_biases_parameter wb = (vx_weights_biases_parameter)ref;

    vxoWeightsBiases_Destroy(wb);
}

void vxoWeightsBiases_Destroy(
    vx_weights_biases_parameter wb
    )
{
    if (wb->mGpuWBTable != VX_NULL)
    {
        vxFree(wb->mGpuWBTable);
        wb->mGpuWBTable = VX_NULL;
        wb->mGpuWBCount = 0;
    }

    if (wb->slice_array != VX_NULL)
    {
        vxFree(wb->slice_array);
        wb->slice_array = VX_NULL;
    }

    if (wb->archPerfHandle != VX_NULL)
    {
        vxFree(wb->archPerfHandle);
        wb->archPerfHandle = VX_NULL;
    }

    if (wb->memory.nodePtrs[0] != VX_NULL)
    {
        vxoMemory_Free(wb->base.context, &wb->memory);
    }

    if (wb->sub_wb_vdata)
    {
        if (wb->sub_wb_vdata->slice_array != VX_NULL)
        {
            vxFree(wb->sub_wb_vdata->slice_array);
            wb->sub_wb_vdata->slice_array = VX_NULL;
        }

        if (wb->sub_wb_vdata->wb_memory_ptr != VX_NULL)
        {
            vxFree(wb->sub_wb_vdata->wb_memory_ptr);
            wb->sub_wb_vdata->wb_memory_ptr = VX_NULL;
        }
        vxFree(wb->sub_wb_vdata);
        wb->sub_wb_vdata = VX_NULL;
    }

    if (wb->zOffsetHandle != VX_NULL)
    {
        vxFree(wb->zOffsetHandle);
        wb->zOffsetHandle = VX_NULL;
    }

    if (wb->huffmanConfig != VX_NULL)
    {
        vxFree(wb->huffmanConfig);
        wb->huffmanConfig = VX_NULL;
    }

    if (wb->max_per_core_per_vzgroup_nzr != VX_NULL)
    {
        vxFree(wb->max_per_core_per_vzgroup_nzr);
        wb->max_per_core_per_vzgroup_nzr = VX_NULL;
    }

    vxoReference_Release((vx_reference_ptr)&(wb->wb_base), VX_TYPE_WEIGHTS_BIASES_PARAMETER_BASE, VX_REF_INTERNAL);
}

void vxoWeightsBiases_Clear(vx_weights_biases_parameter wb)
{
    vx_weights_biases_parameter_base wbBase;

    vxmASSERT(wb);

    wbBase = wb->wb_base;

    vxmASSERT(wbBase);

    if (wbBase->reshuffleWeightPtr != VX_NULL)
    {
        vxFree(wbBase->reshuffleWeightPtr);
        wbBase->reshuffleWeightPtr = VX_NULL;
    }

    if (wbBase->origWeight != VX_NULL)
    {
        vxoReference_Release((vx_reference_ptr)&wbBase->origWeight, VX_TYPE_TENSOR, VX_REF_INTERNAL);
        wbBase->origWeight = VX_NULL;
    }

    if (wbBase->origBias != VX_NULL)
    {
        vxoReference_Release((vx_reference_ptr)&wbBase->origBias, VX_TYPE_TENSOR, VX_REF_INTERNAL);
        wbBase->origBias = VX_NULL;
    }
}

vx_weights_biases_parameter _createWeightsBiasesParameterFromTensors(
    vx_context  context,
    vx_enum     layer_type,
    vx_uint32 * inputs_dims,
    vx_uint32   num_of_input_dims,
    vx_uint32   num_of_output_dims,
    vx_uint32   pad_x_left,
    vx_uint32   pad_x_right,
    vx_uint32   pad_y_top,
    vx_uint32   pad_y_bottom,
    vx_uint32   pooling_size_x,
    vx_uint32   pooling_size_y,
    vx_uint32   stride_x,
    vx_uint32   stride_y,
    vx_enum     down_scale_size_rounding,
    vx_uint32 * convolution_outputs_dims,
    vx_uint32 * pooling_outputs_dims,
    vx_weights_biases_parameter_optimizations_t * optimizations,
    vx_enum     output_format,
    vx_enum     convert_format,
    vx_enum     rank_mode,
    vx_tensor   weights,
    vx_tensor   biases,
    vx_tensor   alpha,
    vx_bool     doPRelu,
    vx_bool     do1xN
    )
{
    vx_weights_biases_parameter_base wb_base;
    vx_weights_biases_parameter weight_bias = VX_NULL;

    vx_uint32 weightDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32 weightViewStarts[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32 weightViewEnds[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32 biasDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32 biasViewStarts[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32 biasViewEnds[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32 inputDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32 convOutputDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32 finalOutputDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];

    vx_uint32 weightDimCount    = TENSOR_DIM_NUM(weights);
    vx_uint32 orgWeightDimCount = TENSOR_DIM_NUM(weights);
    vx_enum weightDataType          = TENSOR_DATA_TYPE(weights);
    vx_enum weightQuantType     = TENSOR_QUANT_TYPE(weights);
    vx_uint32 weightSize        = (vx_uint32)vxDataType_GetSize((vx_type_e)weightDataType);
    vx_uint32 orgWeightSize     = weightSize;

    vx_uint32 biasDimCount      = 0;
    vx_int8 biasFp              = 0;
    vx_float32 biasScale        = 0;
    vx_enum biasDataType        = VX_TYPE_FLOAT32;
    vx_enum biasQuantType       = biases ? TENSOR_QUANT_TYPE(biases) : TENSOR_QUANT_TYPE(weights);
    vx_uint32 i;

    gctPOINTER convertedWeightData  = VX_NULL;
    gctPOINTER weightData           = VX_NULL;
    vx_bool    reallyDo1xN          = vx_false_e;
    vx_bool    doZdpOpt             = vx_false_e;

    vx_bool nnSupportFormat = vx_false_e;

    vx_uint32 strideX, strideY;
    vx_bool hasHwDepthWise = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_NN_DEPTHWISE_SUPPORT);
    vx_bool isV8 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP0);
    vx_bool isOrigNNBatchFc = vx_false_e;
    vx_status status             = VX_SUCCESS;

    nnSupportFormat = vxnneIsNNSupportFormat(context, weights, VX_NULL, VX_NULL);

    if (!nnSupportFormat && layer_type == VX_NN_CONVOLUTION_LAYER)
    {
        status = VX_ERROR_INVALID_TYPE;
        vxError("_createWeightsBiasesParameterFromTensors: current NN didn't support this format 0x%x", weightDataType);
        goto exit;
    }

    vxoTensor_GetTensorViewRegion(weights, weightDimCount, weightViewStarts, weightViewEnds);

    if (layer_type == VX_NN_FULLYCONNECTED_LAYER && weightDimCount == 2)
        weightDimCount = 4;

    /*Hw didn't support FP32, need convert to supported format*/
    if (weightDataType == VX_TYPE_FLOAT32 && convert_format != 0)
    {
        weightDataType = convert_format;
        weightSize = (vx_uint32)vxDataType_GetSize((vx_type_e)weightDataType);
    }

    if (rank_mode == VX_TENSOR_RANK_CWHN)
    {
        /* If shape is (channel, width, height, batch), need trans to (width, height, channel, batch) */
        vx_uint32 cwhn_dims[4] = {0};
        vx_uint32 input_batch, input_height, input_width, input_depth;
        vx_uint32 output_height, output_width, output_depth;
        vx_uint32 weights_total;
        vx_uint8_ptr buffer = VX_NULL;
        vx_uint8_ptr org_weight_ptr = TENSOR_LOGICAL_ADDR(weights);

        cwhn_dims[0] = TENSOR_VIEW_SIZE_INDEX(weights, 0);
        cwhn_dims[1] = TENSOR_VIEW_SIZE_INDEX(weights, 1);
        cwhn_dims[2] = TENSOR_VIEW_SIZE_INDEX(weights, 2);
        cwhn_dims[3] = TENSOR_VIEW_SIZE_INDEX(weights, 3);

        if (TENSOR_DIM_NUM(weights) == 2)
        {
            weightDims[0] = 1;
            weightDims[1] = 1;
            weightDims[2] = cwhn_dims[1];
            weightDims[3] = cwhn_dims[0];
        }
        else
        {

            weightDims[0] = cwhn_dims[2];
            weightDims[1] = cwhn_dims[1];
            weightDims[2] = cwhn_dims[3];
            weightDims[3] = cwhn_dims[0];
        }

        input_batch = cwhn_dims[0];
        input_height = cwhn_dims[1];
        input_width = cwhn_dims[2];
        input_depth = cwhn_dims[3];

        output_width = weightDims[0];
        output_height = weightDims[1];
        output_depth = weightDims[2];

        weights_total = input_batch * input_height * input_width * input_depth * orgWeightSize;
        buffer = (vx_uint8_ptr)vxAllocateAndZeroMemory(weights_total);

        vxMemCopy(buffer, org_weight_ptr, weights_total);
        gcoOS_MemFill(org_weight_ptr, 0, weights_total);

        vxnneAdapter_SWCWHN2WHCN(buffer, (vx_type_e)TENSOR_DATA_TYPE(weights), TENSOR_QUANT_TYPE(weights), input_depth, input_width, input_height, input_batch, TENSOR_POS(weights), TENSOR_TF_ZEROPOINT(weights), TENSOR_TF_SCALE(weights),
                org_weight_ptr, (vx_type_e)weightDataType, TENSOR_QUANT_TYPE(weights), output_depth, output_width, output_height, TENSOR_POS(weights), TENSOR_TF_ZEROPOINT(weights), TENSOR_TF_SCALE(weights), TENSOR_ROUNDING_MODE(weights));

        vxFree(buffer);
    }
    else if (layer_type == VX_NN_FULLYCONNECTED_LAYER && orgWeightDimCount == 2)
    {
        weightDims[0] = weightDims[1] = 1;
        if (weights->isViewed)
        {
            for (i = 0; i < orgWeightDimCount; i++)
            {
                weightDims[i+2] = weightViewEnds[i] - weightViewStarts[i];
            }
        }
        else
        {
            weightDims[2] = weights->dims[0];
            weightDims[3] = weights->dims[1];
        }
    }
    else if (weights->isViewed)
    {
        for (i = 0; i < weightDimCount; i++)
        {
            weightDims[i] = weightViewEnds[i] - weightViewStarts[i];
        }
    }
    else
    {
        vxMemCopy(weightDims, TENSOR_SIZES(weights), weightDimCount * sizeof(vx_uint32));
    }

    if ((layer_type == VX_NN_FULLYCONNECTED_LAYER) && ((weightDims[0] != 1) && (weightDims[1] != 1)))
    {
        vx_int32 index;
        for (index = weightDimCount - 1; index >= 0; index--)
        {
            weightDims[index] = (index == 0 || index == 1) ? 1 : (index == 2) ?
                (weightDims[index] * weightDims[index-1] * weightDims[index-2]) : weightDims[index];
        }
    }

    for (i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
    {
        if (inputs_dims[i])
            inputDims[i] = inputs_dims[i];
        if (convolution_outputs_dims[i])
            convOutputDims[i] = convolution_outputs_dims[i];
        if (pooling_outputs_dims[i])
            finalOutputDims[i] = pooling_outputs_dims[i];
    }

    if (layer_type == VX_NN_FULLYCONNECTED_LAYER)
    {
        if (num_of_input_dims == 1)
        {
            inputDims[0] = 1;
            inputDims[1] = 1;
            inputDims[2] = inputs_dims[0];
        }
        else if (num_of_input_dims == 2)
        {
            inputDims[0] = 1;
            inputDims[1] = 1;
            inputDims[2] = inputs_dims[0];
            inputDims[3] = inputs_dims[1];
        }

        if (num_of_output_dims == 1)
        {
            convOutputDims[0] = 1;
            convOutputDims[1] = 1;
            convOutputDims[2] = convolution_outputs_dims[0];

            finalOutputDims[0] = 1;
            finalOutputDims[1] = 1;
            finalOutputDims[2] = pooling_outputs_dims[0];
        }
        else if (num_of_output_dims == 2)
        {
            convOutputDims[0] = 1;
            convOutputDims[1] = 1;
            convOutputDims[2] = convolution_outputs_dims[0];
            convOutputDims[3] = convolution_outputs_dims[1];

            finalOutputDims[0] = 1;
            finalOutputDims[1] = 1;
            finalOutputDims[2] = pooling_outputs_dims[0];
            finalOutputDims[3] = pooling_outputs_dims[1];
        }
    }

    if ((vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3) || vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6)) &&
        weightDims[0] == 1 &&
        weightDims[1] == 1 &&
        (pad_x_left == 0 && pad_x_right == 0 && pad_y_top == 0 && pad_y_bottom == 0) &&
        layer_type == VX_NN_CONVOLUTION_LAYER &&
        (TENSOR_DATA_TYPE(weights) == VX_TYPE_UINT8 || TENSOR_DATA_TYPE(weights) == VX_TYPE_INT8) &&
        context->options.enableZdpOpt &&
        do1xN)
    {
        vx_uint32 fitN = 0;
        vx_uint32 fitOutN = 0;

        vx_uint32 index_w = 0, index_h = 1;

        if (rank_mode == VX_TENSOR_RANK_CWHN)
        {
            index_w = 2;
            index_h = 1;
        }

        stride_x = (stride_x > 0) ? stride_x : gcmCEIL((vx_float32)inputDims[index_w] / convOutputDims[index_w]);
        stride_y = (stride_y > 0) ? stride_y : gcmCEIL((vx_float32)inputDims[index_h] / convOutputDims[index_h]);

        doZdpOpt = calcFitZdp3N(context, inputs_dims[index_w], inputs_dims[index_h], &fitN, stride_x, pooling_size_x);
        fitOutN = fitN / stride_x;

        if (doZdpOpt)
        {
            /* Need reshape input[x, y, kz] --> [x*y/fitN, fitN, kz] */
            /* Need reshape output[x, y, vz] --> [x*y/fitN, fitN, vz] */
            inputDims[index_w] = inputDims[index_w] * inputDims[index_h] / fitN;
            inputDims[index_h] = fitN;

            convOutputDims[index_w] = convOutputDims[index_w] * convOutputDims[index_h] / fitOutN;
            convOutputDims[index_h] = fitOutN;

            finalOutputDims[index_w] = convOutputDims[index_w] / (convolution_outputs_dims[index_w] / pooling_outputs_dims[index_w]);
            finalOutputDims[index_h] = convOutputDims[index_h] / (convolution_outputs_dims[index_h] / pooling_outputs_dims[index_h]);
        }
    }
    else if (!(vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3) || vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6)) &&
        !gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_CONV1x1_PERF_FIX) &&
        !isV8 && /*XYDP0 means V8, need disable this WAR*/
        weightDims[0] == 1 &&
        weightDims[1] == 1 &&
        pooling_size_x <= 1 &&
        layer_type == VX_NN_CONVOLUTION_LAYER &&
        (pad_x_left == 0 && pad_x_right == 0 && pad_y_top == 0 && pad_y_bottom == 0) &&
        context->options.nn1x1To1xN &&
        do1xN)
    {
        vx_uint32 fitN = calcFit1xN(context, weightDims[2], inputs_dims[0], inputs_dims[1]);
        stride_x = (stride_x > 0) ? stride_x : gcmCEIL((vx_float32)inputDims[0] / convOutputDims[0]);
        stride_y = (stride_y > 0) ? stride_y : gcmCEIL((vx_float32)inputDims[1] / convOutputDims[1]);
        gcmASSERT(stride_x == stride_y);

        if (fitN > 1 && stride_x == 1)
        {
            reallyDo1xN = vx_true_e;
            /* Need reshape input[x, y, kz] --> [x*y, fitN, kz/fitN] */
            /* Need reshape output[x, y, vz] --> [x*y, 1, vz] */
            /* Need reshape weight[1, 1, kz, vz] --> [1, fitN, kz/fitN, vz] */
            weightDims[1] = fitN;
            weightDims[2] /= fitN;

            inputDims[0] *= inputDims[1];
            inputDims[1] = fitN;
            inputDims[2] /= fitN;

            convOutputDims[0] *= convOutputDims[1];
            convOutputDims[1] = 1;

            finalOutputDims[0] =  convOutputDims[0] / (convolution_outputs_dims[0] / pooling_outputs_dims[0]);
            finalOutputDims[1] =  convOutputDims[1] / (convolution_outputs_dims[1] / pooling_outputs_dims[1]);
        }
    }

    if (biases)
    {
        biasDimCount = TENSOR_DIM_NUM(biases);
        biasDataType = TENSOR_DATA_TYPE(biases);
        biasScale = TENSOR_TF_SCALE(biases);
        biasFp = TENSOR_POS(biases);

        if (biasDataType != VX_TYPE_INT32 &&
            biasDataType != VX_TYPE_FLOAT32 &&
            biasDataType != VX_TYPE_INT64)
        {
            status = VX_ERROR_INVALID_TYPE;
            vxError("_createWeightsBiasesParameterFromTensors: current NN didn't support this bias format 0x%x", biasDataType);
            goto exit;
        }
        vxoTensor_GetTensorViewRegion(biases, biasDimCount, biasViewStarts, biasViewEnds);
        if (biases->isViewed)
        {
            for (i = 0; i < biasDimCount; i++)
            {
                biasDims[i] = biasViewEnds[i] - biasViewStarts[i];
            }
        }
        else
        {
            vxMemCopy(biasDims, TENSOR_SIZES(biases), biasDimCount * sizeof(vx_uint32));
        }
    }

    wb_base = vxoWeightsBiasesBase_Create(context,
                                          layer_type,
                                          inputDims,
                                          pad_x_left,
                                          pad_x_right,
                                          pad_y_top,
                                          pad_y_bottom,
                                          pooling_size_x,
                                          pooling_size_y,
                                          stride_x,
                                          stride_y,
                                          down_scale_size_rounding,
                                          convOutputDims,
                                          finalOutputDims,
                                          weightDimCount,
                                          weightDims,
                                          weightDataType,
                                          weightQuantType,
                                          TENSOR_POS(weights),
                                          TENSOR_TF_ZEROPOINT(weights),
                                          TENSOR_TF_SCALE(weights),
                                          biasDimCount,
                                          biasDims,
                                          biasDataType,
                                          biasQuantType,
                                          biasFp,
                                          biasScale,
                                          optimizations);
    if (wb_base == VX_NULL)
    {
        status = VX_FAILURE;
        goto exit;
    }

    if (isOrigNNBatchFc)
        wb_base->nn_fc_batch_mode = vx_true_e;

    if (!isV8 && layer_type == VX_NN_CONVOLUTION_LAYER &&
        wb_base->weights_sizes[0] != wb_base->weights_sizes[1] &&
        wb_base->weights_sizes[0] != 1)
    {
        /*V7 didn't support MxN, only supoort 1xN*/
        status = VX_ERROR_INVALID_VALUE;
        vxError("_createWeightsBiasesParameterFromTensors: current NN didn't support this kernel size %d x %d", wb_base->weights_sizes[0], wb_base->weights_sizes[1]);
        goto exit;
    }

    wb_base->origWeight = weights;
    vxoReference_Increment((vx_reference)wb_base->origWeight, VX_REF_INTERNAL);
    if (biases != VX_NULL)
    {
        wb_base->origBias = biases;
        wb_base->no_bias = vx_false_e;
        vxoReference_Increment((vx_reference)wb_base->origBias, VX_REF_INTERNAL);
    }
    else
        wb_base->no_bias = vx_true_e;

    if (doPRelu && alpha != VX_NULL)
    {
        wb_base->origAlpha = alpha;
        vxoReference_Increment((vx_reference)wb_base->origAlpha, VX_REF_INTERNAL);
    }

    wb_base->do_1xN = reallyDo1xN;
    wb_base->do_zdp_opt = doZdpOpt;

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_FIRST_PIXEL_POOLING) &&
        wb_base->strideX == 2 && wb_base->strideY == 2 &&
        !wb_base->do_zdp_opt &&
        (wb_base->weights_data_format == VX_TYPE_INT8 || wb_base->weights_data_format == VX_TYPE_UINT8) &&
        pooling_size_x == 0 &&
        ((inputDims[0] % 2 == 0 && layer_type == VX_NN_CONVOLUTION_LAYER) || (layer_type == VX_NN_DEPTH_WISE_CONVOLUTION_LAYER && hasHwDepthWise)))
    {
        /* Per Arch, only support INT8 3x3 conv right now*/
        /* First pixel pooling is 2x2 poooling stride is 2, so convolution output should be even*/
        vx_float32 nonZeroRatio = calculateWeightNonZeroRatio(context, wb_base->skipValue, weights);

        /*V8 has limitation for 1x2 & 2x1, those shape with 2x2 stride can't do FFP*/
        if ((nonZeroRatio * weights->dims[0] * weights->dims[1] < 6.3 || (layer_type == VX_NN_DEPTH_WISE_CONVOLUTION_LAYER && hasHwDepthWise)) &&
            !(isV8 && ((weights->dims[0] == 1 && weights->dims[1] == 2) || (weights->dims[0] == 2 && weights->dims[1] == 1))) &&
            weights->dims[0] <= 15 && weights->dims[1] <= 15)
        {
            /* If no pooling & stride = 2 && non-zero-ratio * kx * ky less than 0.7 * 9, do first pixel pooling(2x2, stride = 2), no need reshuffle */
            wb_base->strideX = 1;
            wb_base->strideY = 1;
            wb_base->do_fisrt_pixel_pool = vx_true_e;
            wb_base->pooling_size_x = 2;
            wb_base->pooling_size_y = 2;
            wb_base->pooling_stride = 2;

            wb_base->weights_sizes[0] = weights->dims[0];
            wb_base->weights_sizes[1] = weights->dims[1];
            wb_base->weights_sizes[2] = weights->dims[2];
        }

        if (layer_type == VX_NN_DEPTH_WISE_CONVOLUTION_LAYER && hasHwDepthWise &&
            (isV8 && ((weights->dims[0] == 1 && weights->dims[1] == 2) || (weights->dims[0] == 2 && weights->dims[1] == 1))))
        {
            /*V8 has limitation for 1x2 && 2x1, so don't support depth wise with this shape & 2x2 stride*/
            status = VX_ERROR_INVALID_VALUE;
            vxError("_createWeightsBiasesParameterFromTensors: current NN didn't support this kernel size %d x %d", wb_base->weights_sizes[0], wb_base->weights_sizes[1]);
            goto exit;
        }
    }

    strideX = wb_base->strideX;
    strideY = wb_base->strideY;

    if (layer_type == VX_NN_DEPTH_WISE_CONVOLUTION_LAYER && hasHwDepthWise &&
       strideX == 1 &&
       strideY == 1 &&
       (wb_base->weights_data_format == VX_TYPE_INT8 || wb_base->weights_data_format == VX_TYPE_UINT8) &&
       (wb_base->weights_sizes[2] == 1 || wb_base->weights_sizes[3] == 1))
    {
        wb_base->hw_depth_wise = vx_true_e;
        if (wb_base->weights_sizes[2] != 1 && wb_base->weights_sizes[3] == 1)
        {
            /*default set kz = 1, vz = outZ*/
            wb_base->weights_sizes[3] = wb_base->weights_sizes[2];
            wb_base->weights_sizes[2] = 1;
        }
    }
    else
        wb_base->hw_depth_wise = vx_false_e;


    /* reshuffle weight data and save in wb_base->reshuffleWeightPtr if kernel stride > 1 */
    vxoWeightsBiases_Reshuffle(wb_base);
    weight_bias = vxoWeightsBiases_Create(context,
                                          wb_base,
                                          wb_base->weights_sizes,
                                          layer_type,
                                          vx_true_e);
    if (weight_bias == VX_NULL)
    {
        status = VX_FAILURE;
        goto exit;
    }

    weight_bias->mGpuWBTable = VX_NULL;
    weight_bias->mGpuWBCount = 0;


exit:
    /* Free temp weight data buffer */
    if (convertedWeightData != VX_NULL)
    {
        {
            vxFree(convertedWeightData);
        }

        convertedWeightData = VX_NULL;
    }

    if (weightData != VX_NULL)
    {
        {
            vxFree(weightData);
        }
    }

    return status == VX_SUCCESS ? weight_bias : VX_NULL;
}

vx_weights_biases_parameter _createWeightsBiasesParameterFromParams(
    vx_context  context,
    vx_enum     layer_type,
    vx_uint32 * inputs_dims,
    vx_uint32   pad_x_left,
    vx_uint32   pad_x_right,
    vx_uint32   pad_y_top,
    vx_uint32   pad_y_bottom,
    vx_uint32   pooling_size_x,
    vx_uint32   pooling_size_y,
    vx_enum     down_scale_size_rounding,
    vx_uint32 * convolution_outputs_dims,
    vx_uint32 * pooling_outputs_dims,
    vx_uint32   weights_num_of_dims,
    vx_uint32 * weights_base_dims,
    vx_uint32 * weights_dims,
    vx_enum     weights_data_format,
    vx_enum     weights_quant_format,
    vx_int8     weights_fixed_point_pos,
    vx_uint32   biases_num_of_dims,
    vx_uint32 * biases_base_dims,
    vx_uint32 * biases_dims,
    vx_enum     biases_data_format,
    vx_enum     biases_quant_format,
    vx_int8     biases_fixed_point_pos
    )
{
    vx_weights_biases_parameter wb;
    vx_weights_biases_parameter_base wb_base;

    wb_base = vxoWeightsBiasesBase_Create(context,
                                          layer_type,
                                          inputs_dims,
                                          pad_x_left,
                                          pad_x_right,
                                          pad_y_top,
                                          pad_y_bottom,
                                          pooling_size_x,
                                          pooling_size_y,
                                          1,
                                          1,
                                          down_scale_size_rounding,
                                          convolution_outputs_dims,
                                          pooling_outputs_dims,
                                          weights_num_of_dims,
                                          weights_base_dims,
                                          weights_data_format,
                                          weights_quant_format,
                                          weights_fixed_point_pos,
                                          1,
                                          1.0f,
                                          biases_num_of_dims,
                                          biases_base_dims,
                                          biases_data_format,
                                          biases_quant_format,
                                          biases_fixed_point_pos,
                                          1.0f,
                                          VX_NULL);
    if (wb_base == VX_NULL) return VX_NULL;

    wb = vxoWeightsBiases_Create(context,
                                 wb_base,
                                 weights_dims,
                                 layer_type,
                                 vx_true_e);
    if (wb == VX_NULL) return VX_NULL;

    return wb;
}

