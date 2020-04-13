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
#ifdef ORI_NNARCHPERF
#include "nnArchPerfOri.h"
#else
#include "archModelInterface.h"
#include "nnArchPerf.h"
#endif
#define MAX_HISTO_COUNT 256
#define MAX_SIZE_HISTO_COUNT 9

#define BF16_BIAS_SIZE 3

#define FEATURE_BFP16_ZRL_ENABLE 0

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
        if(freq[i] >= 0)
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

void OutputAt(vx_int32 x, vx_uint32** kernelBufferPtr, vx_uint32 *bit_offset, CodeSymbol* codeSymbol)
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

        writeBits(kernelBufferPtr, bit_offset, codeSymbol[pos - 1].stageCode[0], codeSymbol[pos - 1].bitLength[0]);
        writeBits(kernelBufferPtr, bit_offset, codeSymbol[pos - 0].stageCode[0], codeSymbol[pos - 0].bitLength[0]);

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
            writeBits(kernelBufferPtr, bit_offset, codeSymbol[pos - 3].stageCode[1], codeSymbol[pos - 3].bitLength[1]);
            writeBits(kernelBufferPtr, bit_offset, codeSymbol[pos - 2].stageCode[1], codeSymbol[pos - 2].bitLength[1]);

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
            writeBits(kernelBufferPtr, bit_offset, codeSymbol[pos - 5].stageCode[2], codeSymbol[pos - 5].bitLength[2]);
            writeBits(kernelBufferPtr, bit_offset, codeSymbol[pos - 4].stageCode[2], codeSymbol[pos - 4].bitLength[2]);

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
void addDummy(vx_int32 x, vx_uint32** kernelBufferPtr, vx_uint32* bit_offset, CodeSymbol* codeSymbol, vx_uint32 dummyCount, vx_uint32* dummyStages, vx_uint32* dummyBits)
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
        OutputAt(x, kernelBufferPtr, bit_offset, codeSymbol);
        x++;
    }

    dummyS0 = (dummyCount & 0x1)? dummy1S0: dummy0S0;
    for (j = 0; j < 2*THROUGHPUT; j++)
    {
        codeSymbol[x%(3*THROUGHPUT)].stageCode[0] = dummyS0; /*huffCode[0];*/
        codeSymbol[x%(3*THROUGHPUT)].bitLength[0] = 3;
        /* StageCode[1] is dummy0S1 and dummy1S2, they both zero*/
        OutputAt(x, kernelBufferPtr, bit_offset, codeSymbol);
        x++;
    }
}

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
        vx_uint32 tempX;
        vx_uint32 tempY;
        /* do 64xN rehape */
        i = stride;
        tempX = 64 * i;
        tempY = sliceSize / tempX;

        while ((tempY > 0) && (tempY > maxInImageYSize) && (tempX < maxInImageXSize))
        {
            i = i + stride;
            tempX = 64 * i;
            if ((sliceSize % tempX) == 0)
            {
                tempY = sliceSize / tempX;
            }
        }

        if ((tempX > 0) && (tempX < maxInImageXSize) && (tempY > 0) && (tempY < maxInImageYSize))
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
        vx_uint32 tempX;
        vx_uint32 tempY;
        i = stride;
        tempX = 16 * i;
        tempY = sliceSize / tempX;

        while ((tempY > maxInImageYSize) && (tempX < maxInImageXSize))
        {
            i = i + stride;
            tempX = 16 * i;
            if ((sliceSize % tempX) == 0)
            {
                tempY = sliceSize / tempX;
            }
        }

        if ((tempX < maxInImageXSize) && (tempY < maxInImageYSize) && (tempY > 0))
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
                else if (poolingSize == 2 &&
                    ((inputX * inputY) / (i * stride)) % 2 == 0 &&
                    i % 2 == 0 &&
                    ((inputX * inputY) / (i * stride * 2)) % 2 == 0)
                {
                    /* 2x2 pooling out image size should be even */
                    *fitN = i;
                    return vx_true_e;
                }
                else if (poolingSize == 3 &&
                    ((inputX * inputY) / (i * stride)) % 2 == 0 &&
                    i % 2 == 0 &&
                    ((inputX * inputY) / (i * stride * 2)) % 2 == 1)
                {
                    /* 3x3 pooling out image size should be odd */
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

vx_size calculateWeightBiasBufferSizeForZeroRunLen(
    vx_context context,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_uint32 skip_value,
    vx_uint8 zero_run_len,
    gctPOINTER weight_data,
    vx_uint32* all_count,
    vx_uint32* non_zero_count,
    vx_size* orig_kernel_buf_size
    )
{
    vx_enum weightFomat = weight_format;
    vx_uint32 skipValue = skip_value;
    vx_uint8 zeroRunLen = zero_run_len;

    vx_uint32 nnCoreCount = (weight_format == VX_TYPE_INT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : (weight_format == VX_TYPE_FLOAT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountFloat16 : (weight_format == VX_TYPE_BFLOAT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountBFloat16 :context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 maxZeroRun         = (1 << zero_run_len) - 1;

    vx_size kernelBufferSize     = 0;
    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 filterCount        = filters_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;

    vx_uint32 weightCount        = weight_x * weight_y;
    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize(weightFomat);
    vx_uint32 weightBitSize      = weightSize * 8;
    vx_uint32 biasSize           = (vx_uint32)vxDataType_GetSize(bias_format);
    vx_uint32 biasBitSize        = biasSize * 8;

    vx_uint32 sliceCount         = slice_count;
    vx_uint32 filterSliceSize    = weightCount * weightSize;
    vx_uint32 filterSize         = weightCount * weightSize * sliceCount;
    vx_uint8* startDataPtr       = VX_NULL;
    vx_uint8* kernelDataPtr      = VX_NULL;

    vx_uint32 bit_offset          = 0;
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
        bit_offset = 8 + 16;

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

            if (weight_x == 1 && weight_y == 1 &&
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
                                bit_offset += zeroRunLen + weightBitSize;
                                if (bit_offset >= 32)
                                {
                                    bit_offset -= 32;
                                    kernelBufferSize += 4;
                                }
                                zeroRun = 0;

                                if (needToWriteBias)
                                {
                                    bit_offset += biasBitSize;
                                    /* Write bias. */
                                    if (bit_offset >= 32)
                                    {
                                        bit_offset -= 32;
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
                                    bit_offset += NN_Z_POSITION_OFFSET_BITS_VIP_V7;
                                else
                                    bit_offset += NN_Z_POSITION_OFFSET_BITS;
                                if (bit_offset >= 32)
                                {
                                    bit_offset -= 32;
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

                                    /* Add one slice data every filter. */
                                    kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * realSliceIndex;

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

                                            if ((zeroRun == maxZeroRun)
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
                                                /* Write zeroRun and weight. */
                                                bit_offset += zeroRunLen + weightBitSize;
                                                if (bit_offset >= 32)
                                                {
                                                    bit_offset -= 32;
                                                    kernelBufferSize += 4;
                                                }
                                                zeroRun = 0;

                                                if (needToWriteBias && !hasWriteBias)
                                                {
                                                    bit_offset += biasBitSize;
                                                    /* Write bias. */
                                                    if (bit_offset >= 32)
                                                    {
                                                        bit_offset -= 32;
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
                                            if (realSliceIndex == sliceCount - 1 && realWeightXIndex == weight_x - 1 && realWeightYIndex == weight_y - 1)
                                            {
                                                 /* Write offsetValue. */
                                                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                                    bit_offset += NN_Z_POSITION_OFFSET_BITS_VIP_V7;
                                                else
                                                    bit_offset += NN_Z_POSITION_OFFSET_BITS;

                                                if (bit_offset >= 32)
                                                {
                                                    bit_offset -= 32;
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

                        for (weightYIndex = 0; weightYIndex < weight_y; weightYIndex++)
                        {
                            vx_uint32 nonZeroCountInDP3 = 0; /*Check if there's non-zero point in one 3x1 block*/
                            for (weightXIndex = 0; weightXIndex < weight_x; weightXIndex++)
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
                                    if ((weightXIndex + 1) % 3 == 0 || weightXIndex == weight_x - 1)
                                    {
                                        blockCount++;
                                        if (nonZeroCountInDP3)
                                            nonZeroCount++;
                                        nonZeroCountInDP3 = 0;
                                    }
                                }

                                if ((zeroRun == maxZeroRun)
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
                                    /* Write zeroRun and weight. */
                                    bit_offset += zeroRunLen + weightBitSize;
                                    if (bit_offset >= 32)
                                    {
                                        bit_offset -= 32;
                                        kernelBufferSize += 4;
                                    }
                                    zeroRun = 0;

                                    if (needToWriteBias)
                                    {
                                        bit_offset += biasBitSize;
                                        /* Write bias. */
                                        if (bit_offset >= 32)
                                        {
                                            bit_offset -= 32;
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
                                bit_offset += NN_Z_POSITION_OFFSET_BITS_VIP_V7;
                            else
                                bit_offset += NN_Z_POSITION_OFFSET_BITS;
                            if (bit_offset >= 32)
                            {
                                bit_offset -= 32;
                                kernelBufferSize += 4;
                            }
                        }
                    }
                }
            }
        }

        /* pad 0 */
        if (bit_offset)
        {
            kernelBufferSize += 4;
        }
        kernelBufferSize = (kernelBufferSize + 63) & 0xFFFFFFC0;
    }

    if (all_count != VX_NULL)
        *all_count = blockCount;
    if (non_zero_count != VX_NULL)
        *non_zero_count = nonZeroCount;
    if (orig_kernel_buf_size != VX_NULL)
        *orig_kernel_buf_size = (filterSize + biasSize) * filterTotalCount;

    return kernelBufferSize;
}

vx_bool calculateWeightBiasBufferSizeForZeroRunLenEx(
    vx_context context,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_uint32 skip_value,
    gctPOINTER weight_data,
    vx_uint32* all_count,
    vx_uint32* non_zero_count,
    vx_size* orig_kernel_buf_size,
    vx_size* min_size,
    vx_uint8* zero_run_len
    )
{
    vx_enum weightFomat = weight_format;
    vx_uint32 skipValue = skip_value;

    vx_uint32 nnCoreCount = (weight_format == VX_TYPE_INT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : (weight_format == VX_TYPE_FLOAT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountFloat16 : (weight_format == VX_TYPE_BFLOAT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountBFloat16 :context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 filterCount        = filters_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;

    vx_uint32 weightCount        = weight_x * weight_y;
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

            if (weight_x == 1 && weight_y == 1 &&
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

                                    /* Add one slice data every filter. */
                                    kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * realSliceIndex;

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

                                            if (((realSliceIndex == 0)
                                                    && (realWeightXIndex == weight_x - 1)
                                                    && (realWeightYIndex == weight_y - 1)
                                                    && needToWriteBias)
                                                || ((realSliceIndex == slice_count - 1)
                                                    && (realWeightXIndex == weight_x - 1)
                                                    && (realWeightYIndex == weight_y - 1))
                                                || (weight != skipValue)
                                                || ((filterIndex == filterEnd)
                                                    && (realWeightXIndex == weight_x - 1)
                                                    && (realWeightYIndex == weight_y - 1)
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
                                            if (realSliceIndex == slice_count - 1 && realWeightXIndex == weight_x - 1 && realWeightYIndex == weight_y - 1)
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

                        for (weightYIndex = 0; weightYIndex < weight_y; weightYIndex++)
                        {
                            vx_uint32 nonZeroCountInDP3 = 0; /*Check if there's non-zero point in one 3x1 block*/

                            for (weightXIndex = 0; weightXIndex < weight_x; weightXIndex++)
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
                                    if ((weightXIndex + 1) % 3 == 0 || weightXIndex == weight_x - 1)
                                    {
                                        blockCount++;
                                        if (nonZeroCountInDP3)
                                            nonZeroCount++;
                                        nonZeroCountInDP3 = 0;
                                    }
                                }

                                if (((sliceIndex == 0)
                                        && (weightXIndex == weight_x - 1)
                                        && (weightYIndex == weight_y - 1)
                                        && needToWriteBias)
                                    || ((weightXIndex == weight_x - 1)
                                        && (weightYIndex == weight_y - 1)
                                        && (sliceIndex == slice_count - 1))
                                    || (weight != skipValue)
                                    || ((filterIndex == filterEnd)
                                        && (weightXIndex == weight_x - 1)
                                        && (weightYIndex == weight_y - 1)
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
            bits += origBitsArray[coreIndex];
            size += (gcmALIGN(bits, 32) / 32) * 4;
            size  = gcmALIGN(size, 64);
        }

        if (size < minSize)
        {
            minSize = size;
            minZrl = (vx_uint8)i;
        }

        if (!complete) break;
    }

    if (origBitsArray)
        vxFree(origBitsArray);
    if (maxBasePad)
        vxFree(maxBasePad);
    if (zerosPerCoreArray)
        vxFree(zerosPerCoreArray);

    if (all_count != VX_NULL) *all_count = blockCount;
    if (non_zero_count != VX_NULL) *non_zero_count = nonZeroCount;
    if (orig_kernel_buf_size != VX_NULL) *orig_kernel_buf_size = (filterSize + biasSize) * filterTotalCount;

    if (min_size != VX_NULL) *min_size = minSize;
    if (zero_run_len != VX_NULL) *zero_run_len = minZrl;

    return complete;
}

vx_size calculateWeightBiasBalanceSizeForZeroRunLen(
    vx_context context,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_uint32 skip_value,
    vx_uint8 zero_run_len,
    gctPOINTER weight_data,
    vx_uint32* all_count,
    vx_uint32* non_zero_count,
    vx_size* orig_kernel_buf_size
    )
{
    vx_enum weightFomat = weight_format;
    vx_uint32 skipValue = skip_value;
    vx_uint8 zeroRunLen = zero_run_len;

    vx_uint32 nnCoreCount = (weight_format == VX_TYPE_INT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : (weight_format == VX_TYPE_FLOAT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountFloat16 : (weight_format == VX_TYPE_BFLOAT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountBFloat16 :context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 maxZeroRun         = (1 << zero_run_len) - 1;

    vx_size kernelBufferSize     = 0;
    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 filterCount        = filters_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;

    vx_uint32 weightCount        = weight_x * weight_y;
    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize(weightFomat);
    vx_uint32 weightBitSize      = weightSize * 8;
    vx_uint32 biasSize           = (vx_uint32)vxDataType_GetSize(bias_format);
    vx_uint32 biasBitSize        = biasSize * 8;

    vx_uint32 sliceCount         = slice_count;
    vx_uint32 filterSliceSize    = weightCount * weightSize;
    vx_uint32 filterSize         = weightCount * weightSize * sliceCount;
    vx_uint8* startDataPtr       = VX_NULL;
    vx_uint8* kernelDataPtr      = VX_NULL;

    vx_uint32 bit_offset          = 0;
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
        bit_offset = 8 + 16;

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

            if (weight_x == 1 && weight_y == 1 &&
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
                                bit_offset += zeroRunLen + weightBitSize;
                                if (bit_offset >= 32)
                                {
                                    bit_offset -= 32;
                                    kernelBufferSize += 4;
                                }
                                zeroRun = 0;

                                if (needToWriteBias)
                                {
                                    bit_offset += biasBitSize;
                                    /* Write bias. */
                                    if (bit_offset >= 32)
                                    {
                                        bit_offset -= 32;
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
                                    bit_offset += NN_Z_POSITION_OFFSET_BITS_VIP_V7;
                                else
                                    bit_offset += NN_Z_POSITION_OFFSET_BITS;
                                if (bit_offset >= 32)
                                {
                                    bit_offset -= 32;
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

                                    /* Add one slice data every filter. */
                                    kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * realSliceIndex;

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

                                            if ((zeroRun == maxZeroRun)
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
                                                /* Write zeroRun and weight. */
                                                bit_offset += zeroRunLen + weightBitSize;
                                                if (bit_offset >= 32)
                                                {
                                                    bit_offset -= 32;
                                                    kernelBufferSize += 4;
                                                }
                                                zeroRun = 0;

                                                if (needToWriteBias && !hasWriteBias)
                                                {
                                                    bit_offset += biasBitSize;
                                                    /* Write bias. */
                                                    if (bit_offset >= 32)
                                                    {
                                                        bit_offset -= 32;
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
                                            if (realSliceIndex == sliceCount - 1 && realWeightXIndex == weight_x - 1 && realWeightYIndex == weight_y - 1)
                                            {
                                                 /* Write offsetValue. */
                                                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                                    bit_offset += NN_Z_POSITION_OFFSET_BITS_VIP_V7;
                                                else
                                                    bit_offset += NN_Z_POSITION_OFFSET_BITS;

                                                if (bit_offset >= 32)
                                                {
                                                    bit_offset -= 32;
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

                        for (weightYIndex = 0; weightYIndex < weight_y; weightYIndex++)
                        {
                            vx_uint32 nonZeroCountInDP3 = 0; /*Check if there's non-zero point in one 3x1 block*/
                            for (weightXIndex = 0; weightXIndex < weight_x; weightXIndex++)
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
                                    if ((weightXIndex + 1) % 3 == 0 || weightXIndex == weight_x - 1)
                                    {
                                        blockCount++;
                                        if (nonZeroCountInDP3)
                                            nonZeroCount++;
                                        nonZeroCountInDP3 = 0;
                                    }
                                }

                                if ((zeroRun == maxZeroRun)
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
                                    /* Write zeroRun and weight. */
                                    bit_offset += zeroRunLen + weightBitSize;
                                    if (bit_offset >= 32)
                                    {
                                        bit_offset -= 32;
                                        kernelBufferSize += 4;
                                    }
                                    zeroRun = 0;

                                    if (needToWriteBias)
                                    {
                                        bit_offset += biasBitSize;
                                        /* Write bias. */
                                        if (bit_offset >= 32)
                                        {
                                            bit_offset -= 32;
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
                                bit_offset += NN_Z_POSITION_OFFSET_BITS_VIP_V7;
                            else
                                bit_offset += NN_Z_POSITION_OFFSET_BITS;
                            if (bit_offset >= 32)
                            {
                                bit_offset -= 32;
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
        if (bit_offset)
        {
            kernelBufferSize += 4;
        }
        kernelBufferSize = (kernelBufferSize + 63) & 0xFFFFFFC0;
    }

    if (all_count != VX_NULL)
        *all_count = blockCount;
    if (non_zero_count != VX_NULL)
        *non_zero_count = nonZeroCount;
    if (orig_kernel_buf_size != VX_NULL)
        *orig_kernel_buf_size = (filterSize + biasSize) * filterTotalCount;

    if (nonZeroWeights)
        vxFree(nonZeroWeights);

    return kernelBufferSize;
}

vx_bool calculateWeightBiasBalanceSizeForZeroRunLenEx(
    vx_context context,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_uint32 skip_value,
    gctPOINTER weight_data,
    vx_uint32* all_count,
    vx_uint32* non_zero_count,
    vx_size* orig_kernel_buf_size,
    vx_size* min_size,
    vx_uint8* zero_run_len
    )
{
    vx_enum weightFomat = weight_format;
    vx_uint32 skipValue = skip_value;

    vx_uint32 nnCoreCount = (weight_format == VX_TYPE_INT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : (weight_format == VX_TYPE_FLOAT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountFloat16 : (weight_format == VX_TYPE_BFLOAT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountBFloat16 :context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 filterCount        = filters_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;

    vx_uint32 weightCount        = weight_x * weight_y;
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

            if (weight_x == 1 && weight_y == 1 &&
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

                                    /* Add one slice data every filter. */
                                    kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * realSliceIndex;

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

                                            if (((realSliceIndex == 0)
                                                    && (realWeightXIndex == weight_x - 1)
                                                    && (realWeightYIndex == weight_y - 1)
                                                    && needToWriteBias)
                                                || ((realSliceIndex == slice_count - 1)
                                                    && (realWeightXIndex == weight_x - 1)
                                                    && (realWeightYIndex == weight_y - 1))
                                                || (weight != skipValue)
                                                || ((kid == actualFilterCount - 1)
                                                    && (realWeightXIndex == weight_x - 1)
                                                    && (realWeightYIndex == weight_y - 1)
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
                                            if (realSliceIndex == slice_count - 1 && realWeightXIndex == weight_x - 1 && realWeightYIndex == weight_y - 1)
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

                        for (weightYIndex = 0; weightYIndex < weight_y; weightYIndex++)
                        {
                            vx_uint32 nonZeroCountInDP3 = 0; /*Check if there's non-zero point in one 3x1 block*/

                            for (weightXIndex = 0; weightXIndex < weight_x; weightXIndex++)
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
                                    if ((weightXIndex + 1) % 3 == 0 || weightXIndex == weight_x - 1)
                                    {
                                        blockCount++;
                                        if (nonZeroCountInDP3)
                                            nonZeroCount++;
                                        nonZeroCountInDP3 = 0;
                                    }
                                }

                                if (((sliceIndex == 0)
                                        && (weightXIndex == weight_x - 1)
                                        && (weightYIndex == weight_y - 1)
                                        && needToWriteBias)
                                    || ((weightXIndex == weight_x - 1)
                                        && (weightYIndex == weight_y - 1)
                                        && (sliceIndex == slice_count - 1))
                                    || (weight != skipValue)
                                    || ((kid == actualFilterCount - 1)
                                        && (weightXIndex == weight_x - 1)
                                        && (weightYIndex == weight_y - 1)
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
            bits += origBitsArray[coreIndex];
            size += (gcmALIGN(bits, 32) / 32) * 4;
            size  = gcmALIGN(size, 64);
        }

        if (size < minSize)
        {
            minSize = size;
            minZrl = (vx_uint8)i;
        }

        if (!complete) break;
    }

    if (origBitsArray)
        vxFree(origBitsArray);
    if (maxBasePad)
        vxFree(maxBasePad);
    if (zerosPerCoreArray)
        vxFree(zerosPerCoreArray);
    if (nonZeroWeights)
        vxFree(nonZeroWeights);

    if (all_count != VX_NULL) *all_count = blockCount;
    if (non_zero_count != VX_NULL) *non_zero_count = nonZeroCount;
    if (orig_kernel_buf_size != VX_NULL) *orig_kernel_buf_size = (filterSize + biasSize) * filterTotalCount;

    if (min_size != VX_NULL) *min_size = minSize;
    if (zero_run_len != VX_NULL) *zero_run_len = minZrl;

    return complete;
}

void reorderWeightBiasBufferForHuffman(
    vx_context context,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_int32 weight_zp,
    vx_enum  bias_format,
    vx_int32 input_zp,
    vx_int32 z_offset,
    vx_uint32 skip_value,
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
    vx_uint32 nnCoreCount = (weight_format == VX_TYPE_INT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : (weight_format == VX_TYPE_FLOAT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountFloat16 : (weight_format == VX_TYPE_BFLOAT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountBFloat16 :context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;
    vx_uint32 filterCount        = filters_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;

    vx_uint32 weightCount        = weight_x * weight_y;
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

    vx_uint32 inputZP = input_zp;
    vx_uint32 coefZP = weight_zp;

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

    if (calc_perf)
    {
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

            if (weight_x == 1 && weight_y == 1 &&
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

                                offsetValue = (vx_uint32)z_offset * filterIndex;

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
                        for (weightYIndex = 0; weightYIndex < weight_y; weightYIndex += yStep)
                        {
                            subKySize = gcmMIN((weight_y - weightYIndex), yStep);
                            for (weightXIndex = 0; weightXIndex < weight_x; weightXIndex += xStep)
                            {
                                subKxSize = gcmMIN((weight_x - weightXIndex), xStep);
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
                                            vx_uint8_ptr realKernelDataPtr = kernelDataPtr + (realWeightYIndex * weight_x + realWeightXIndex) * weightSize;

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
                                                realWeightXIndex == weight_x - 1 &&
                                                realWeightYIndex == weight_y - 1)
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

                                                offsetValue = (vx_uint32)z_offset * filterIndex;

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

                        for (weightYIndex = 0; weightYIndex < weight_y; weightYIndex++)
                        {
                            vx_uint32 nonZeroCountInDP3 = 0; /*Check if there's non-zero point in one 3x1 block*/
                            for (weightXIndex = 0; weightXIndex < weight_x; weightXIndex++)
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
                                    if ((weightXIndex + 1) % 3 == 0 || weightXIndex == weight_x - 1)
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

                            offsetValue = (vx_uint32)z_offset * filterIndex;

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
            if (calc_perf)
                nonZeroRatioPerCorePerVZG[nonZrlIdx] = (vx_float64)nonZeroCount / (vx_float64)blockCount;
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
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_int32 weight_zp,
    vx_enum  bias_format,
    vx_int32 input_zp,
    vx_int32 z_offset,
    vx_uint32 skip_value,
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
    vx_uint32 nnCoreCount = (weight_format == VX_TYPE_INT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : (weight_format == VX_TYPE_FLOAT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountFloat16 : (weight_format == VX_TYPE_BFLOAT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountBFloat16 :context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 sliceCount         = slice_count;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;
    vx_uint32 filterCount        = filters_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;

    vx_uint32 weightCount        = weight_x * weight_y;
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

    vx_uint32 inputZP = input_zp;
    vx_uint32 coefZP = weight_zp;

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

            for (weightYIndex = 0; weightYIndex < weight_y; weightYIndex++)
            {
                for (weightXIndex = 0; weightXIndex < weight_x; weightXIndex++)
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

    if (calc_perf)
    {
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


            if (weight_x == 1 && weight_y == 1 &&
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

                                offsetValue = (vx_uint32)z_offset * filterIndex;

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
                        for (weightYIndex = 0; weightYIndex < weight_y; weightYIndex += yStep)
                        {
                            subKySize = gcmMIN((weight_y - weightYIndex), yStep);
                            for (weightXIndex = 0; weightXIndex < weight_x; weightXIndex += xStep)
                            {
                                subKxSize = gcmMIN((weight_x - weightXIndex), xStep);
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
                                            vx_uint8_ptr realKernelDataPtr = kernelDataPtr + (realWeightYIndex * weight_x + realWeightXIndex) * weightSize;

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
                                                realWeightXIndex == weight_x - 1 &&
                                                realWeightYIndex == weight_y - 1)
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

                                                offsetValue = (vx_uint32)z_offset * filterIndex;

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

                        for (weightYIndex = 0; weightYIndex < weight_y; weightYIndex++)
                        {
                            vx_uint32 nonZeroCountInDP3 = 0; /*Check if there's non-zero point in one 3x1 block*/
                            for (weightXIndex = 0; weightXIndex < weight_x; weightXIndex++)
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
                                    if ((weightXIndex + 1) % 3 == 0 || weightXIndex == weight_x - 1)
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

                            offsetValue = (vx_uint32)z_offset * filterIndex;

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
            if (calc_perf && blockCount != 0)
                nonZeroRatioPerCorePerVZG[nonZrlIdx] = (vx_float64)nonZeroCount / (vx_float64)blockCount;

            usedKid += actualFilterCount;
            if (filterGroup)
            {
                vxFree(filterGroup);
                filterGroup = VX_NULL;
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

#define DUMP_TP_ENC 0

void reorderTPKernelBufferHuffman(
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
                if (weight_format == VX_TYPE_BFLOAT16)
                {
                    vx_uint16 exp = (weight & 0x7F80) >> 7;
                    vx_uint16 mantissa = weight & 0x7F;
                    vx_uint8 signedBit = (weight & 0x8000) >> 15;
                    vx_uint16 temp = 0;

                    /*we didn't suppot INF & NAN*/
                    vxmASSERT(exp != 0xFFFF);

                    /* convert -zero to +zero*/
                    if (exp == 0 && mantissa == 0)
                        signedBit = 0;

                    /*newCoef [15:0] = {~OrgValue[14],OrgValue[13:0], OrgValue[15]}*/
#if !FEATURE_BFP16_ZRL_ENABLE
                    exp = exp ^ 0x80; /*~OrgValue[14]*/
#endif
                    temp = (exp << 8) | (mantissa << 1) | signedBit;

                    weight = temp;

                }
                else if (weight_format == VX_TYPE_FLOAT16)
                {
                    vx_uint16 exp = (weight & 0x7C00) >> 10;
                    vx_uint16 mantissa = weight & 0x3FF;
                    vx_uint8 signedBit = (weight & 0x8000) >> 15;
                    vx_uint16 temp = 0;

                    /* convert -zero to +zero*/
                    if (exp == 0 && mantissa == 0 && signedBit == 1)
                        signedBit = 0;
                    temp = (exp << 10) | mantissa | signedBit << 15;

                    weight = temp;

                }

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

#define DUMP_BF16_ENC 0

void reorderDepthWiseKernelBufferV8Huffman(
    vx_context context,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_int32* weight_zp,
    vx_enum  weight_format,
    vx_uint8_ptr reorder_stream,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_uint32 nn_core_count,
    vx_uint32* reoder_stream_count_per_core,
    vx_uint32* non_coef_index,
    vx_uint32* limit_zrl_Index,
    vx_bool hasNNPerFilterPostMultiply
    )
{
    vx_uint32 nnCoreCount = nn_core_count;
    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;
    vx_uint32 filterCount        = filters_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;

    vx_uint32 weightCount        = weight_x * weight_y;
    vx_uint32 filterWeightCount  = weightCount * slice_count;
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
    vx_bool isBias = vx_false_e;
    vx_uint32 biasIdx = 0;

#if DUMP_BF16_ENC
    static vx_uint32 n = 0;
    vx_char fileName[128];
    FILE * pfile1 = NULL;
    sprintf(fileName, "%s_%d.txt", "reordered_kernel", n++);

    pfile1 = fopen(fileName, "wt");
#endif

    if (weight_format == VX_TYPE_BFLOAT16 ||
        weight_format == VX_TYPE_FLOAT16)
        filterWeightCount += BF16_BIAS_SIZE;

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

            if(hasNNPerFilterPostMultiply && gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PER_CHANNEL_QUANT_ASYM) && weight_format == VX_TYPE_UINT8)
            {
                vx_uint32 index = 0;
                for(index = 0;  index <= actualFilterCount; ++index)
                {
                    *kernelBufferInt8Ptr = (vx_uint8)weight_zp[index];
                    kernelBufferInt8Ptr++;
                    elementIndex++;
                    reoder_stream_count_per_core[coreIndex] ++;
                }
            }

            for (kid = 0; kid < actualFilterCount; kid++)
            {
                if (groupIndex == groupCount - 1 &&
                    kid == core0FilterCount - 1)
                    filterIndex = adjFilterStart + kid * nnCoreCount - unusedCoreCount;
                else
                    filterIndex = adjFilterStart + kid * nnCoreCount;

                for (k = 0; k < filterWeightCount; k += linesInImageBuffer)
                {
                    kSize = gcmMIN((filterWeightCount - k), linesInImageBuffer);

                    for (sk = 0; sk < linesInImageBuffer; sk++)
                    {
                        isBias = ((k + sk >= filterWeightCount - BF16_BIAS_SIZE) && (sk < kSize) && (weight_format == VX_TYPE_BFLOAT16 || weight_format == VX_TYPE_FLOAT16)) ? vx_true_e : vx_false_e;
                        biasIdx = isBias ? (k + sk + BF16_BIAS_SIZE - filterWeightCount) : 0;

                        /*Driver split the Float32 Bias to BFloat16 BiasH, BiasM and BiasL and add them as coefs to the end of the coef stream*/
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
                                        vxmASSERT(expH != 0xFF);
                                        /* convert -zero to +zero*/
                                        if (exp == 0 && mantissa == 0)
                                            signedBit = 0;

                                        /*newCoef [15:0] = {~OrgValue[14],OrgValue[13:0], OrgValue[15]}*/
#if !FEATURE_BFP16_ZRL_ENABLE
                                        expH = expH ^ 0x80; /*~OrgValue[14]*/
#endif
                                        biasH = (expH << 8) | (mantissaH << 1) | signedBit;
#if DUMP_BF16_ENC
                                        if(pfile1 != NULL)
                                        {
                                            fprintf(pfile1, "vz:%d, compressed  biasH: 0x%x\n", filterIndex, biasH);
                                        }
#endif
                                    }

                                    *kernelBufferInt16Ptr = biasH;
                                    kernelBufferInt16Ptr++;
                                    non_coef_index[nonCoefIndex++] = elementIndex++;
                                    reoder_stream_count_per_core[coreIndex]++;
                                }
                                break;
                            case 1:
                                {
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
#if !FEATURE_BFP16_ZRL_ENABLE
                                        expM = expM ^ 0x80; /*~OrgValue[14]*/
#endif
                                        biasM = (expM << 8) | mantissaM | signedBit;
                                    }

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
#if !FEATURE_BFP16_ZRL_ENABLE
                                        expL = expL ^ 0x80; /*~OrgValue[14]*/
#endif
                                        biasL = (expL << 8) | mantissaL | signedBit;
                                    }

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
                        weightYIndex /= weight_x;
                        weightXIndex = (k + sk) % weight_x;
                        /*Packl 0 in the last dp group*/
                        if (sk >= kSize)
                        {
                            vx_uint32 skip_value;
                            if(hasNNPerFilterPostMultiply && gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PER_CHANNEL_QUANT_ASYM) && weight_format == VX_TYPE_UINT8)
                                skip_value = weight_zp[filterIndex];
                            else
                                skip_value = weight_zp[0];
                            coef = skip_value;

                        }
                        else
                        {
                            realKernelDataPtr = weight_base_ptr + filterIndex * filterSize +
                                filterSliceSize * weightZIndex +
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
                            vx_uint32 skip_value;
                            vx_int8 newCoef;
                            if(hasNNPerFilterPostMultiply)
                                skip_value = weight_zp[filterIndex];
                            else
                                skip_value = weight_zp[0];

                            newCoef = ((vx_int32)coef - (vx_int32)skip_value) & 0xFF;
                            *kernelBufferInt8Ptr = (vx_uint8)newCoef;
                            kernelBufferInt8Ptr++;
                        }
                        else
                        {
                            if (weight_format == VX_TYPE_BFLOAT16 ||
                                weight_format == VX_TYPE_FLOAT16)
                            {
                                vx_uint16 exp = (coef & 0x7F80) >> 7;
                                vx_uint16 mantissa = coef & 0x7F;
                                vx_uint8 signedBit = (coef & 0x8000) >> 15;
                                vx_uint16 temp = 0;

                                /*we didn't suppot INF & NAN*/
                                vxmASSERT(exp != 0xFF);

                                /* convert -zero to +zero*/
                                if (exp == 0 && mantissa == 0)
                                    signedBit = 0;

                                /*newCoef [15:0] = {~OrgValue[14],OrgValue[13:0], OrgValue[15]}*/
#if !FEATURE_BFP16_ZRL_ENABLE
                                exp = exp ^ 0x80; /*~OrgValue[14]*/
#endif
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
            }
        }
    }
#if DUMP_BF16_ENC
    if (pfile1 != NULL)
        fclose(pfile1);
#endif
    vxmASSERT(limitIndex == filterTotalCount);
}

void reorderKernelBufferV8Huffman(
    vx_context context,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_int32 *weight_zp,
    vx_enum  weight_format,
    vx_uint8_ptr reorder_stream,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_uint32 nn_core_count,
    vx_uint32* reoder_stream_count_per_core,
    vx_uint32* non_coef_index,
    vx_uint32* limit_zrl_Index,
    vx_bool hasNNPerFilterPostMultiply
    )
{
    vx_uint32 nnCoreCount = nn_core_count;
    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;
    vx_uint32 filterCount        = filters_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;

    vx_uint32 weightCount        = weight_x * weight_y;
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

    if (weight_format == VX_TYPE_BFLOAT16 ||
        weight_format == VX_TYPE_FLOAT16)
        filterWeightCount += BF16_BIAS_SIZE;

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


            if(hasNNPerFilterPostMultiply && gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PER_CHANNEL_QUANT_ASYM) && weight_format == VX_TYPE_UINT8)
            {
                vx_uint32 index = 0;
                for(index = filterStart;  index <= filterEnd; ++index)
                {
                    *kernelBufferInt8Ptr = (vx_uint8)weight_zp[index];
                    kernelBufferInt8Ptr++;
                    elementIndex++;
                    reoder_stream_count_per_core[coreIndex] ++;
                }
            }

            for (k = 0; k < filterWeightCount; k += linesInImageBuffer)
            {
                vx_uint32 maxKCount = ((filterWeightCount) % linesInImageBuffer == 0) ? (filterWeightCount / linesInImageBuffer) : (filterWeightCount / linesInImageBuffer + 1);
                kSize = gcmMIN((filterWeightCount - k), linesInImageBuffer);

                for (filterIndex = filterStart; filterIndex <= filterEnd; filterIndex++)
                {
                    for (sk = 0; sk < linesInImageBuffer; sk++)
                    {
                        isBias = ((k + sk >= filterWeightCount - BF16_BIAS_SIZE) && (sk < kSize) && (weight_format == VX_TYPE_BFLOAT16 || weight_format == VX_TYPE_FLOAT16)) ? vx_true_e : vx_false_e;
                        biasIdx = isBias ? (k + sk + BF16_BIAS_SIZE - filterWeightCount) : 0;

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
                                        vxmASSERT(expH != 0xFF);
                                        /* convert -zero to +zero*/
                                        if (exp == 0 && mantissa == 0)
                                            signedBit = 0;

                                        /*newCoef [15:0] = {~OrgValue[14],OrgValue[13:0], OrgValue[15]}*/
#if !FEATURE_BFP16_ZRL_ENABLE
                                        expH = expH ^ 0x80; /*~OrgValue[14]*/
#endif
                                        biasH = (expH << 8) | (mantissaH << 1) | signedBit;
#if DUMP_BF16_ENC
                                        if(pfile1 != NULL)
                                        {
                                            fprintf(pfile1, "vz:%d, compressed  biasH: 0x%x\n", filterIndex, biasH);
                                        }
#endif
                                    }

                                    *kernelBufferInt16Ptr = biasH;
                                    kernelBufferInt16Ptr++;
                                    non_coef_index[nonCoefIndex++] = elementIndex++;
                                    reoder_stream_count_per_core[coreIndex]++;
                                }
                                break;
                            case 1:
                                {
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
#if !FEATURE_BFP16_ZRL_ENABLE
                                        expM = expM ^ 0x80; /*~OrgValue[14]*/
#endif
                                        biasM = (expM << 8) | mantissaM | signedBit;
                                    }

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
#if !FEATURE_BFP16_ZRL_ENABLE
                                        expL = expL ^ 0x80; /*~OrgValue[14]*/
#endif
                                        biasL = (expL << 8) | mantissaL | signedBit;
                                    }

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
                        weightYIndex /= weight_x;
                        weightXIndex = (k + sk) % weight_x;
                        /*Packl 0 in the last dp group*/
                        if (sk >= kSize)
                        {
                            vx_uint32 skip_value;
                            if(hasNNPerFilterPostMultiply && gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PER_CHANNEL_QUANT_ASYM) && weight_format == VX_TYPE_UINT8)
                                skip_value = weight_zp[filterIndex];
                            else
                                skip_value = weight_zp[0];
                            coef = skip_value;
                        }
                        else
                        {
                            realKernelDataPtr = weight_base_ptr + filterIndex * filterSize +
                                filterSliceSize * weightZIndex +
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
                            vx_uint32 skip_value;
                            vx_int8 newCoef;
                            if(hasNNPerFilterPostMultiply)
                                skip_value = weight_zp[filterIndex];
                            else
                                skip_value = weight_zp[0];

                            newCoef = ((vx_int32)coef - (vx_int32)skip_value) & 0xFF;
                            *kernelBufferInt8Ptr = (vx_uint8)newCoef;
                            kernelBufferInt8Ptr++;
                        }
                        else
                        {
                            if (weight_format == VX_TYPE_BFLOAT16 ||
                                weight_format == VX_TYPE_FLOAT16)
                            {
                                vx_uint16 exp = (coef & 0x7F80) >> 7;
                                vx_uint16 mantissa = coef & 0x7F;
                                vx_uint8 signedBit = (coef & 0x8000) >> 15;
                                vx_uint16 temp = 0;

                                /*we didn't suppot INF & NAN*/
                                vxmASSERT(exp != 0xFF);

                                /* convert -zero to +zero*/
                                if (exp == 0 && mantissa == 0)
                                    signedBit = 0;

                                /*newCoef [15:0] = {~OrgValue[14],OrgValue[13:0], OrgValue[15]}*/
#if !FEATURE_BFP16_ZRL_ENABLE
                                exp = exp ^ 0x80; /*~OrgValue[14]*/
#endif
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
                                *kernelBufferInt16Ptr = (vx_uint16)coef;
                                kernelBufferInt16Ptr++;
                            }
                        }
                        reoder_stream_count_per_core[coreIndex]++;
                        if (weight_format == VX_TYPE_BFLOAT16 ||
                            weight_format == VX_TYPE_FLOAT16)
                        {
                            if(kSize > BF16_BIAS_SIZE)
                            {
                                if (k == (maxKCount - 1) * linesInImageBuffer && sk == (kSize - BF16_BIAS_SIZE - 1))
                                    limit_zrl_Index[limitIndex++] = elementIndex;
                            }
                            else
                            {
                                if (k == (maxKCount - 1) * linesInImageBuffer && sk == (linesInImageBuffer + kSize - BF16_BIAS_SIZE - 1))
                                    limit_zrl_Index[limitIndex++] = elementIndex;
                            }
                        }
                        else if (k == (maxKCount - 1) * linesInImageBuffer &&  sk == linesInImageBuffer - 1)
                        {
                            limit_zrl_Index[limitIndex++] = elementIndex;
                        }

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
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_uint32* weight_zp,
    vx_enum  weight_format,
    vx_uint8_ptr reorder_stream,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32 nn_core_count,
    vx_uint32* reoder_stream_count_per_core,
    vx_uint32* non_coef_index,
    vx_uint32* real_vz_index,
    vx_uint32* limit_zrl_Index,
    vx_bool hasNNPerFilterPostMultiply
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

    vx_uint32 weightCount        = weight_x * weight_y;
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
        vx_uint32 skipValue = weight_zp[0];

        if(hasNNPerFilterPostMultiply)
            skipValue = weight_zp[filterIndex];

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

            if(hasNNPerFilterPostMultiply && gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PER_CHANNEL_QUANT_ASYM) && weight_format == VX_TYPE_UINT8)
            {
                vx_uint32 index = 0;
                for(index = 0;  index <= actualFilterCount; ++index)
                {
                    *kernelBufferInt8Ptr = (vx_uint8)weight_zp[index];
                    kernelBufferInt8Ptr++;
                    elementIndex++;
                    reoder_stream_count_per_core[coreIndex] ++;
                }
            }


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
                        weightYIndex /= weight_x;
                        weightXIndex = (k + sk) % weight_x;
                        /*Packl 0 in the last dp group*/
                        if (sk >= kSize)
                        {
                            if(hasNNPerFilterPostMultiply)
                                coef = weight_zp[filterIndex];
                            else
                                coef = weight_zp[0];
                        }
                        else
                        {
                            realKernelDataPtr = weight_base_ptr + filterIndex * filterSize +
                                filterSliceSize * weightZIndex +
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
                            vx_int8 newCoef = 0;
                            vx_uint32 skip_value = 0;

                            if(hasNNPerFilterPostMultiply)
                                skip_value = weight_zp[filterIndex];
                            else
                                skip_value = weight_zp[0];

                            newCoef = ((vx_int32)coef - (vx_int32)skip_value) & 0xFF;
                            *kernelBufferInt8Ptr = (vx_uint8)newCoef;
                            kernelBufferInt8Ptr++;
                        }
                        else
                        {
                            if (weight_format == VX_TYPE_FLOAT16)
                                coef = (coef & 0x7fff) * 2 + ((coef & (1<<15)) >> 15);

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

void analysisKernelStreamForHuffman(
    vx_context context,
    vx_weight_bias_huffman_cfg huffman_config,
    vx_enum   weight_format,
    vx_uint32 skipValue,
    vx_uint32 nnCoreCount,
    vx_uint8* reorderStream,
    vx_uint32 reorderStreamSize,
    vx_uint32* reorderStreamPerCoreCount,
    vx_uint32* invSizeOrder,
    vx_uint32* nonCoefIndex,
    vx_uint32 nonCoefCount,
    vx_uint32* limitZRLIndex,
    vx_uint32 limitZRLCount
    )
{
    vx_uint8  * pBS_U08      = (unsigned char * )reorderStream;
    vx_uint16 * pBS_U16      = (unsigned short *)reorderStream;
    vx_enum weightFormat = weight_format;
    vx_uint8 bit16_flag   = (weightFormat == VX_TYPE_INT8 || weightFormat == VX_TYPE_UINT8) ? 0 : 1;
    vx_uint32 kernelDataCount  = reorderStreamSize >> bit16_flag;
    vx_uint32 kernelDataBytes  = reorderStreamSize;
    vx_uint32 dataCountToPross = kernelDataCount;
    vx_weight_bias_huffman_cfg huffmanConfig = huffman_config;

    vx_int32 i, j, k, bias;
    vx_int32 x=0;
    vx_int32 runZeros[256], run; /*The histogram of how many runs*/
    vx_int32 histo[256], sizeHisto[9] = {0,0,0,0,0,0,0,0,0};
    vx_int32 sizeOrder[]  = {0,1,2,3,4,5,6,7,8,};
    vx_int32 coding_type   = HUFFMAN_CODING_TYPE_RUN_LEN; /* defalt RZL*/
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

    vxmASSERT(huffmanConfig != VX_NULL);

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

        if (!bit16_flag)
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
        histo[i>>(bit16_flag*8)]++;
        /*Get statistic of prediction from previous pixel*/
        if(bit16_flag == 0 && !isV8)
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
    if (j*3 < histo[0]*4 && bit16_flag == 0)
        bias = 0;

    /*16 bits, we only get the high bit bias, we need to get the low bit bias*/
    if (bit16_flag)
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

    if (bit16_flag == 0)
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
            if (bit16_flag == 0)
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

            histo[(i>>(bit16_flag*8))&0xff]++;
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

    if (bit16_flag == 0 && ((unsigned int)histo[0] <= reorderStreamSize / 8 || histo[0] < (histo[1] + histo[255]) * 3/4 || entropy >= 5.25f) && entropy > 3.5f )
    {
         coding_type = HUFFMAN_CODING_TYPE_NON_RUN_LEN;
    }
    if (bit16_flag && run < (histo[1] + histo[255]) / 4)
    {
         coding_type = HUFFMAN_CODING_TYPE_NON_RUN_LEN;
    }
    if (bit16_flag && entropy > 5.25f)
        coding_type = 0; /*Force to test run-length*/


    if (coding_type == HUFFMAN_CODING_TYPE_RUN_LEN)
    {
        FindBest2PlusRunSets(runZeros, sizeof(runZeros)/sizeof(int), (reorderStreamSize >> bit16_flag) - run);
    }
    else
    {
        memset(outBest2Run, 0, sizeof(outBest2Run));
        memset(bestRunsSorted, 0, sizeof(bestRunsSorted));
        memset(best2Runs, 0, sizeof(best2Runs));
        memset(freq, 0, sizeof(freq));
        numBestRuns = freqSum = 0;
    }

    if (coding_type == HUFFMAN_CODING_TYPE_NON_RUN_LEN)
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

    if (coding_type == HUFFMAN_CODING_TYPE_RUN_LEN)
    {
        j = 1;
        if (bit16_flag)
           histo[0] -= run; /*The zeros not runs*/

        run = 0;
        for (i = 1; i < 8; i++)
        {
            for (; j < (1<<i); j++)
            {
                sizeHisto[i] += histo[j - bit16_flag] + histo[(-j)&0xff];
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
    huffmanConfig->pre_encode = (vx_uint8)prevEncode;
    huffmanConfig->bit16_flag  = bit16_flag;
    huffmanConfig->fp16_flag   = (weightFormat == VX_TYPE_FLOAT16) ? 1 : 0;
    huffmanConfig->reserved   = 0x0; /* must be zero*/
    huffmanConfig->version    = 0x1; /* 4'b0001 for version 1.0*/
    huffmanConfig->run_len_table_size = (coding_type == HUFFMAN_CODING_TYPE_NON_RUN_LEN) ? 0: (vx_uint8)numBestRuns;

    /* runlenTable 8 * 18 = 144*/
    for(i = 0; i<SET_1_SIZE; i++)
    {
        huffmanConfig->run_len_table[i] = (vx_uint8)best2Runs[i];
    }
    for(; i< numBestRuns; i++)
    {
        huffmanConfig->run_len_table[i] = (vx_uint8)outBest2Run[i - SET_1_SIZE];
    }
    for(i = 0; i<8; i+=2)
    {
        huffmanConfig->map_to_huffman[i/2] = (vx_uint8)(sizeOrder[i+1]*16 + sizeOrder[i]);
    }
    huffmanConfig->avg_bias = (vx_uint8)bias;
    huffmanConfig->reserved = 0x0; /* reserved 16, must be zero*/

exit:
    if (accumCountEndOfCore != VX_NULL)
        vxFree(accumCountEndOfCore);
}

void analysisTPStreamForHuffman(
    vx_weight_bias_huffman_cfg huffman_config,
    vx_uint32 sliceCount,
    vx_uint32 filterCount,
    vx_enum  weight_format,
    vx_uint8* reorderStream,
    vx_uint32* invSizeOrder
    )
{
    vx_uint8  * pBS_U08      = (unsigned char * )reorderStream;
    vx_uint16 * pBS_U16      = (unsigned short *)reorderStream;
    vx_enum weightFormat = weight_format;
    vx_uint32 weightSize = (vx_uint32)vxDataType_GetSize((vx_type_e)weightFormat);
    vx_uint8 bit16_flag   = (weightFormat == VX_TYPE_INT8 || weightFormat == VX_TYPE_UINT8) ? 0 : 1;
    vx_uint32 kernelDataCount  = sliceCount * filterCount;
    vx_uint32 kernelDataBytes  = kernelDataCount * weightSize;
    vx_uint32 dataCountToPross = kernelDataCount;
    vx_weight_bias_huffman_cfg huffmanConfig = huffman_config;

    vx_int32 i, j, k, bias;
    vx_int32 x=0;
    vx_int32 runZeros[256], run; /*The histogram of how many runs*/
    vx_int32 histo[256], sizeHisto[9] = {0,0,0,0,0,0,0,0,0};
    vx_int32 sizeOrder[]  = {0,1,2,3,4,5,6,7,8,};
    vx_int32 coding_type   = HUFFMAN_CODING_TYPE_RUN_LEN; /* defalt RZL*/
    vx_int32 prevEncode   = 0, prev = 0, prevHisto[256], prevRunZeros[256], prevRun; /*previous pixel*/
    vx_float32 p, entropy = 0.f, prevEntropy = 0.f;
    vx_int32 sliceIndex = 0, accumSize = 0x0;

    /* this variable indicate DataAccumCount when Start of core*/
    vx_int32 *accumCountEndOfCore = (vx_int32 *)vxAllocateAndZeroMemory(sliceCount * sizeof(vx_int32));

    vxmASSERT(huffmanConfig != VX_NULL);

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

        if (!bit16_flag)
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
        histo[i>>(bit16_flag*8)]++;
        /*Get statistic of prediction from previous pixel*/
        if(bit16_flag == 0)
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
    if (j*3 < histo[0]*4 && bit16_flag == 0)
        bias = 0;

    /*16 bits, we only get the high bit bias, we need to get the low bit bias*/
    if (bit16_flag)
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

    if (bit16_flag == 0)
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

            if (bit16_flag == 0)
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

            histo[(i>>(bit16_flag*8))&0xff]++;
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

    if (bit16_flag == 0 && ((unsigned int)histo[0] <= kernelDataBytes / 8 || histo[0] < (histo[1] + histo[255]) * 3/4 || entropy >= 5.25f) && entropy > 3.5f )
    {
         coding_type = HUFFMAN_CODING_TYPE_NON_RUN_LEN;
    }
    if (bit16_flag && run < (histo[1] + histo[255]) / 4)
    {
         coding_type = HUFFMAN_CODING_TYPE_NON_RUN_LEN;
    }
    if (bit16_flag && entropy > 5.25f)
        coding_type = 0; /*Force to test run-length*/


    if (coding_type == HUFFMAN_CODING_TYPE_RUN_LEN)
    {
        FindBest2PlusRunSets(runZeros, sizeof(runZeros)/sizeof(int), (kernelDataBytes >> bit16_flag) - run);
    }

    if (coding_type == HUFFMAN_CODING_TYPE_NON_RUN_LEN)
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

    if (coding_type == HUFFMAN_CODING_TYPE_RUN_LEN)
    {
        j = 1;
        if (bit16_flag)
           histo[0] -= run; /*The zeros not runs*/

        run = 0;
        for (i = 1; i < 8; i++)
        {
            for (; j < (1<<i); j++)
            {
                sizeHisto[i] += histo[j - bit16_flag] + histo[(-j)&0xff];
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
    huffmanConfig->pre_encode = (vx_uint8)prevEncode;
    huffmanConfig->bit16_flag  = bit16_flag;
    huffmanConfig->fp16_flag   = (weightFormat == VX_TYPE_FLOAT16) ? 1 : 0;
    huffmanConfig->reserved   = 0x0; /* must be zero*/
    huffmanConfig->version    = 0x1; /* 4'b0001 for version 1.0*/
    huffmanConfig->run_len_table_size = (coding_type == HUFFMAN_CODING_TYPE_NON_RUN_LEN) ? 0: (vx_uint8)numBestRuns;

    /* runlenTable 8 * 18 = 144*/
    for(i = 0; i<SET_1_SIZE; i++)
    {
        huffmanConfig->run_len_table[i] = (vx_uint8)best2Runs[i];
    }
    for(; i< numBestRuns; i++)
    {
        huffmanConfig->run_len_table[i] = (vx_uint8)outBest2Run[i - SET_1_SIZE];
    }
    for(i = 0; i<8; i+=2)
    {
        huffmanConfig->map_to_huffman[i/2] = (vx_uint8)(sizeOrder[i+1]*16 + sizeOrder[i]);
    }

    huffmanConfig->avg_bias = (vx_uint8)bias;
    huffmanConfig->reserved = 0x0; /* reserved 16, must be zero*/

exit:
    if (accumCountEndOfCore != VX_NULL)
        vxFree(accumCountEndOfCore);
}

vx_uint32 calcKernelStreamSizeHuffman(
    vx_context context,
    vx_weight_bias_huffman_cfg huffman_config,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_int32 weight_zp,
    vx_enum  bias_format,
    vx_int32 input_zp,
    vx_uint32 skip_value,
    vx_int32 z_offset,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr
    )
{
    vx_uint32 kernelSize = 0;
    vx_uint32 kernelBitSize = 0;

    vx_uint32 nnCoreCount = (weight_format == VX_TYPE_INT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : (weight_format == VX_TYPE_FLOAT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountFloat16 : (weight_format == VX_TYPE_BFLOAT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountBFloat16 :context->nnConfig.fixedFeature.nnCoreCount;

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

    vx_weight_bias_huffman_cfg huffmanConfig = huffman_config;

    vx_uint32 coreIndex;
    vx_uint32 totalCountIdx = 0;
    vx_uint32 nonCoefCountIdx = 0;

    vx_uint32 bit16_flag = (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8) ? 0 : 1;

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

    vxmASSERT(huffmanConfig != VX_NULL);

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
        reorderKernelBufferV7HuffmanBalance(context, weight_x, weight_y, slice_count, z_count, filters_per_core, weight_format, weight_zp, bias_format, input_zp, z_offset, skip_value, reorderStream, weight_base_ptr, bias_base_ptr, VX_NULL, VX_NULL, reorderStreamPerCoreCount, nonCoefIndex, limitZRLIndex, vx_false_e);

    }
    else
    {
        reorderWeightBiasBufferForHuffman(context, weight_x, weight_y, slice_count, z_count, filters_per_core, weight_format, weight_zp, bias_format, input_zp, z_offset, skip_value, reorderStream, weight_base_ptr, bias_base_ptr, VX_NULL, VX_NULL, reorderStreamPerCoreCount, nonCoefIndex, limitZRLIndex, vx_false_e);

    }

    analysisKernelStreamForHuffman(context, huffman_config, weight_format, 0, nnCoreCount, reorderStream, reorderStreamSize, reorderStreamPerCoreCount, invSizeOrder, nonCoefIndex, nonCoefCount, limitZRLIndex, limitZRLCount);

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
        vx_bool setDummy = vx_false_e;
        vx_uint32 dummyBit2 = 0;
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
                limitZRLCountIdx < limitZRLCount)
            {
                isLimitZRL = vx_true_e;
                limitZRLCountIdx++;
            }
            else
                isLimitZRL = vx_false_e;

            totalCountIdx++;

            if (bit16_flag == 0)
            {
                coef = (coef - huffmanConfig->avg_bias) & 0xFF;
                if (huffmanConfig->pre_encode)
                {
                    tmp = coef;
                    coef = (coef - prev) & 0xFF;
                    prev = tmp;
                }
            }
            else
            {
                if (huffmanConfig->fp16_flag)
                {
                    coef16 = (coef16 & 0x7FFF) * 2 + ((coef16 & (1<<15)) >> 15);
                }
                coef16 -= huffmanConfig->avg_bias;
                if (huffmanConfig->run_len_table_size == 0x0)
                    coef = (coef16 >> 8) & 0xFF; /*Non RZL*/
                else
                    coef = coef16 & 0xFFFF; /*RZL path*/
            }

            if (huffmanConfig->run_len_table_size == 0x0) /* non RZL*/
            {
                if (!setDummy)
                {
                    /*Set dummy all to 0,
                    in nonZRL path 8-bit, 0 belong to group0, -1~0, residue bit length is 1, residue code = 0*/
                    size = 0;
                    k = invSizeOrder[size];

                    if (size == 0) size = 1; /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                    if (sizeCodeLen[k]%2 == 0)
                        dummyBit2 = size - 1;
                    else
                        dummyBit2 = size;
                    if (huffmanConfig->bit16_flag)
                       dummyBit2 += 8;
                    setDummy = vx_true_e;
                }

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

                if (huffmanConfig->bit16_flag)
                {/*pack the raw data of the lower 8 bits*/
                    kernelBitSize += 8;
                }
            }
            else /* RZL enable*/
            {
                if (!setDummy)
                {
                    /*Set dummy all to 0,
                    in ZRL path 8-bit, 0 belong to group7, -128~127, residue bit length is 8, residue code = 0*/
                    size = 7;
                    j = invSizeOrder[size];
                    if (size == 7)
                        size = 8;
                    if (sizeCodeLen[j] % 2 == 0)
                        dummyBit2 = size - 1;
                    else
                        dummyBit2 = size;
                    if (huffmanConfig->bit16_flag)
                        dummyBit2 += 8;
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
                        coef>>=(huffmanConfig->bit16_flag * 8);
                        if(coef<0x80)
                            coef+=huffmanConfig->bit16_flag; /*Since we have to encode zero (run-length only handle 16-bits zero, 8 bit zero should use Size code*/
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
                        if (huffmanConfig->bit16_flag)
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
                        if (huffmanConfig->bit16_flag)
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

                kernelBitSize += dummyBit2;

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

vx_uint32 calcNonZeroCountV8Huffman(
    vx_context context,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
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
                  context->nnConfig.fixedFeature.nnCoreCountFloat16 : (weight_format == VX_TYPE_BFLOAT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountBFloat16 :context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;
    vx_uint32 filterCount        = filters_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;

    vx_uint32 weightCount        = weight_x * weight_y;
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
                        weightYIndex /= weight_x;
                        weightXIndex = (k + sk) % weight_x;

                        realKernelDataPtr = weight_base_ptr + filterIndex * filterSize +
                            filterSliceSize * weightZIndex +
                            (weightYIndex * weight_x + weightXIndex) * weightSize;

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
                    else if ((zeroBlock0 || zeroBlock1 || zeroBlock2) &&
                            (weight_format == VX_TYPE_UINT8 || weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_INT16))
                        nonZeroCount += 6;
                    else
                        nonZeroCount += 9;
                }
            }
        }
    }
    return nonZeroCount;
}

vx_uint32 calcKernelSizeV8Huffman(
    vx_context context,
    vx_weight_bias_huffman_cfg huffman_config,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum   weight_format,
    vx_uint32 *weight_zp,
    vx_bool   is_depth_wise,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_weights_biases_parameter  wb
    )
{
    vx_uint32 nnCoreCount = (weight_format == VX_TYPE_INT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : (weight_format == VX_TYPE_FLOAT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountFloat16 : (weight_format == VX_TYPE_BFLOAT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountBFloat16 :context->nnConfig.fixedFeature.nnCoreCount;

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

    vx_weight_bias_huffman_cfg huffmanConfig = huffman_config;

    vx_uint8_ptr kernelDataPtr   = VX_NULL;

    vx_uint32 coreIndex;
    vx_uint32 i;

    vx_uint32 bit16_flag = (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8) ? 0 : 1;

    vx_uint32 reorderStreamSize = 0;
    vx_uint32 reorderStreamAllCount = 0;
    vx_uint32 reorderStreamCheckCount = 0;
    vx_uint32* reorderStreamPerCoreCount = VX_NULL;
    vx_uint32 filterIndex = 0;

    vx_uint8* reorderStream = VX_NULL;
    vx_uint32 runSet2Bits = 1;
    CodeSymbol codeSymbol[THROUGHPUT * 3];

    vx_bool hasNoBias = ((weight_format == VX_TYPE_BFLOAT16) || (weight_format == VX_TYPE_FLOAT16)) ? vx_true_e : vx_false_e;
    vx_bool hasNoZOffset = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_NO_Z_LOCATION_OFFSET);
    vx_bool hasNNPreLU = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PRELU);
    vx_bool hasZDP3 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3);
    vx_bool hasNNPerFilterPostMultiply = (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PER_CHANNEL_QUANT) &&
                       ((gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PRELU) && wb->alpha_ref!= VX_NULL && WB_OPERATION_TYPE(wb) == VX_NN_CONV_PRELU) ||
                       ((WB_WEIGHT_QUANT_FORMAT(wb) == VX_QUANT_AFFINE_SCALE_PER_CHANNEL) && (WB_WEIGHT_DATA_FORMAT(wb) == VX_TYPE_UINT8 || WB_WEIGHT_DATA_FORMAT(wb) == VX_TYPE_INT8 ))));


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

    vxmASSERT(huffmanConfig != VX_NULL);

    if (weightBitSize == 0)
    {
        vxError("%s: weightBitSize should not be zero.", __FUNCTION__);
        goto exit;
    }

    if (weight_format == VX_TYPE_BFLOAT16 ||
        weight_format == VX_TYPE_FLOAT16)
    {
        numOfInImageBuffer = (vx_uint32)gcoMATH_Ceiling((vx_float32)(weightCount * slice_count + BF16_BIAS_SIZE) / (vx_float32)linesInImageBuffer);
    }

    reorderStreamAllCount = (numOfInImageBuffer * linesInImageBuffer * filterTotalCount)
        + (usedCoreCount * (16 / weightBitSize));/*filterPerCore need 16 bit * nnCoreCount */
    reorderStreamSize = reorderStreamAllCount * weightSize;
    if(hasNNPerFilterPostMultiply && gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PER_CHANNEL_QUANT_ASYM) && weight_format == VX_TYPE_UINT8)
    {
        reorderStreamSize += filterTotalCount; /*Per channel 8 bit zero point*/
        reorderStreamAllCount += filterTotalCount;
    }

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
    if (weight_format == VX_TYPE_BFLOAT16 ||
        weight_format == VX_TYPE_FLOAT16)
        nonCoefCount += (filterTotalCount * BF16_BIAS_SIZE); /*BFLOAT16 fp32 bias will be splitted to 3 BF16 to coef stream*/


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

    if (is_depth_wise)
        reorderDepthWiseKernelBufferV8Huffman(context, weight_x, weight_y, slice_count, z_count, filters_per_core, (vx_int32 *)weight_zp, weight_format, reorderStream, weight_base_ptr, bias_base_ptr, nnCoreCount, reorderStreamPerCoreCount, nonCoefIndex, limitZRLIndex, hasNNPerFilterPostMultiply);

    else if (context->options.enableNonZeroBalance && !hasNoZOffset) /*Only those chip has z_offset support zero balance*/
        reorderKernelBufferV8HuffmanBalance(context, weight_x, weight_y, slice_count, z_count, filters_per_core, (vx_uint32 *)weight_zp, weight_format, reorderStream, weight_base_ptr, nnCoreCount, reorderStreamPerCoreCount, nonCoefIndex, VX_NULL, limitZRLIndex, hasNNPerFilterPostMultiply);

    else
        reorderKernelBufferV8Huffman(context, weight_x, weight_y, slice_count, z_count, filters_per_core, (vx_int32 *)weight_zp, weight_format, reorderStream, weight_base_ptr, bias_base_ptr, nnCoreCount, reorderStreamPerCoreCount, nonCoefIndex, limitZRLIndex, hasNNPerFilterPostMultiply);


    analysisKernelStreamForHuffman(context, huffman_config, weight_format, 0, nnCoreCount, reorderStream, reorderStreamSize, reorderStreamPerCoreCount, invSizeOrder, nonCoefIndex, nonCoefCount, limitZRLIndex, filterTotalCount);

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
        vx_int32 code = 0;
        vx_int32 coef16 = 0;
        vx_int32 k = 0, j = 0, m = 0;
        vx_bool isCoef;
        vx_bool isLimitZRL;
        vx_bool setDummy = vx_false_e;
        vx_uint32 dummyBit2 = 0;
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

            if (bit16_flag == 0)
            {
                coef = (coef - huffmanConfig->avg_bias) & 0xFF;
                if (huffmanConfig->pre_encode)
                {
                    tmp = coef;
                    coef = (coef - prev) & 0xFF;
                    prev = tmp;
                }
            }
            else
            {
                if (huffmanConfig->fp16_flag)
                {
                    coef16 = (coef16 & 0x7FFF) * 2 + ((coef16 & (1<<15)) >> 15);
                }
                coef16 -= huffmanConfig->avg_bias;
                if (huffmanConfig->run_len_table_size == 0x0)
                    coef = (coef16 >> 8) & 0xFF; /*Non RZL*/
                else
                    coef = coef16 & 0xFFFF; /*RZL path*/
            }

            if (huffmanConfig->run_len_table_size == 0x0) /* non RZL*/
            {
                if (!setDummy)
                {
                    if (sizeCodeLen[invSizeOrder[0]] % 2 == 0)
                        dummyBit2 = 0;
                    else
                        dummyBit2 = 1;
                    if (huffmanConfig->bit16_flag)
                        dummyBit2 += 8;
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

                if (huffmanConfig->bit16_flag)
                {/*pack the raw data of the lower 8 bits*/
                    kernelBitSize += 8;
                }
            }
            else /* RZL enable*/
            {
                if (!setDummy)
                {
                    if (sizeCodeLen[invSizeOrder[7]] % 2 == 0)
                        dummyBit2 = 7;
                    else
                        dummyBit2 = 8;
                    if (huffmanConfig->bit16_flag)
                        dummyBit2 += 8;
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
                                vx_bool finded = vx_false_e;
                                vx_int32 q = 0;
                                for (q = 0; q < SET_1_SIZE; q++)
                                {
                                    if (best2Runs[q] == bestRunsSorted[j])
                                    {
                                        finded = vx_true_e;
                                    }
                                }

                                for (q = 0; q < numBestRuns - SET_1_SIZE; q++)
                                {
                                    if (outBest2Run[q] == bestRunsSorted[j])
                                    {
                                        finded = vx_true_e;
                                    }
                                }

                                if (finded)
                                {
                                    k = run / (bestRunsSorted[j] + 1);
                                    run %= (bestRunsSorted[j] + 1);
                                    break;
                                }

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
                        coef>>=(huffmanConfig->bit16_flag * 8);
                        if(coef<0x80)
                            coef+=huffmanConfig->bit16_flag; /*Since we have to encode zero (run-length only handle 16-bits zero, 8 bit zero should use Size code*/
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
                        if (huffmanConfig->bit16_flag)
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
                        code = huffCode[j];

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
                        if (huffmanConfig->bit16_flag)
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

                kernelBitSize += dummyBit2;

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

        if (hasNNPerFilterPostMultiply || (hasNNPreLU && WB_OPERATION_TYPE(wb) == VX_NN_CONV_PRELU && wb->alpha_ref != VX_NULL))
        {
            kernelStreamSizePerCore += 4;
        }

        if (!hasNoZOffset)
        {
            kernelStreamSizePerCore += NN_Z_POSITION_OFFSET_BITS_VIP_V7 / 8;
        }
        else if (hasNNPreLU && WB_OPERATION_TYPE(wb) == VX_NN_CONV_PRELU && wb->alpha_ref != VX_NULL)
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

void calcTPKernelBufferSizeHuffman(
    vx_weight_bias_huffman_cfg huffman_config,
    vx_uint32 weight_z,
    vx_uint32 filter_count,
    vx_uint32 total_weight_z,
    vx_enum  weight_format,
    vx_uint32 skip_value,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* non_zero_count,
    vx_size*   kernel_stream_size
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

    vx_uint32 bit16_flag = (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8) ? 0 : 1;

    vx_uint32 reorderStreamAllCount = sliceCount * filterCount;
    vx_uint32 reorderStreamSize = reorderStreamAllCount * weightSize;

    vx_uint8* reorderStream = VX_NULL;
    vx_uint32 runSet2Bits = 1;

    vx_weight_bias_huffman_cfg huffmanConfig = huffman_config;

    vxmASSERT(huffmanConfig != VX_NULL);

    reorderStream = (vx_uint8 *)vxAllocateAndZeroMemory(reorderStreamSize);

    if (!reorderStream)
    {
        vxError("fillinKernelBufferHuffman: OUT OF MEMORY");
        goto exit;
    }

    reorderTPKernelBufferHuffman(weight_z, filter_count, total_weight_z, weight_format, reorderStream, weight_base_ptr, VX_NULL, VX_NULL);

    analysisTPStreamForHuffman(huffman_config, sliceCount, filterCount, weight_format, reorderStream, invSizeOrder);

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

    while(((huffmanConfig->run_len_table_size - SET_1_SIZE)>>runSet2Bits) > 1)
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
        vx_int32 code = 0;
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

            if (bit16_flag == 0)
            {

                coef = (coef - huffmanConfig->avg_bias) & 0xFF;
                if (huffmanConfig->pre_encode)
                {
                    tmp = coef;
                    coef = (coef - prev) & 0xFF;
                    prev = tmp;
                }
            }
            else
            {

                if (huffmanConfig->fp16_flag)
                {
                    coef16 = (coef16 & 0x7FFF) * 2 + ((coef16 & (1<<15)) >> 15);
                }
                coef16 -= huffmanConfig->avg_bias;
                if (huffmanConfig->run_len_table_size == 0x0)
                    coef = (coef16 >> 8) & 0xFF; /*Non RZL*/
                else
                    coef = coef16 & 0xFFFF; /*RZL path*/
            }

            if (huffmanConfig->run_len_table_size == 0x0) /* non RZL*/
            {
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

                if (huffmanConfig->bit16_flag)
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
                        for (j = huffmanConfig->run_len_table_size - 1; j>=0; j--)
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
                                for (m = 0; m < huffmanConfig->run_len_table_size - SET_1_SIZE; m++)
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
                        coef>>=(huffmanConfig->bit16_flag * 8);
                        if(coef<0x80)
                            coef+=huffmanConfig->bit16_flag; /*Since we have to encode zero (run-length only handle 16-bits zero, 8 bit zero should use Size code*/
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
                        if (huffmanConfig->bit16_flag)
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

                if (huffmanConfig->bit16_flag)
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

    if (non_zero_count != VX_NULL)
        *non_zero_count = nonZeroCoefCount;
    if (kernel_stream_size != VX_NULL)
        *kernel_stream_size = kernelSize;
exit:
    if (reorderStream)
        vxFree(reorderStream);

    return;
}

void calculateWeightBiasStreamRelatedSize(
    vx_context context,
    vx_weight_bias_huffman_cfg huffman_config,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 kernels_per_core,
    vx_enum weight_format,
    vx_uint32 *weight_zp,
    vx_enum bias_format,
    vx_uint32 input_zp,
    vx_uint32 skip_value,
    vx_uint32 z_offset,
    vx_int8  set_zrl,
    vx_uint8 option_zero_run_len,
    vx_bool  is_depth_wise,
    gctPOINTER weight_data,
    gctPOINTER bias_data,
    vx_uint32* all_count,
    vx_uint32* non_zero_count,
    vx_size* orig_kernel_buf_size,
    vx_size* min_kernel_buf_size,
    vx_uint8*  min_zero_run_len,
    vx_uint32* max_zero_run_len,
    vx_weights_biases_parameter  wb
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
        /*VIP V8*/
        minKernelBufferSize = calcKernelSizeV8Huffman(
                                context,
                                huffman_config,
                                weight_x,
                                weight_y,
                                slice_count,
                                z_count,
                                kernels_per_core,
                                weight_format,
                                weight_zp,
                                is_depth_wise,
                                (vx_uint8_ptr)weight_data,
                                (vx_uint32_ptr)bias_data,
                                wb
                                );

        if (all_count != VX_NULL)
        {
            *all_count = weight_x * weight_y * slice_count * z_count;

            if(gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PER_CHANNEL_QUANT) && gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PER_CHANNEL_QUANT_ASYM) && weight_format == VX_TYPE_UINT8)
                *all_count += z_count;
        }
        if (non_zero_count != VX_NULL)
            *non_zero_count = calcNonZeroCountV8Huffman(context, weight_x, weight_y, slice_count, z_count, kernels_per_core, weight_format, (vx_uint8_ptr)weight_data, skip_value);
        if (orig_kernel_buf_size != VX_NULL)
            *orig_kernel_buf_size = weight_x * weight_y * slice_count * z_count * weightSize + z_count * biasSize;
    }
    else if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_COEF_COMPRESSION_ENHANCEMENT))
    {
        /*V7 Huffman*/
        minKernelBufferSize = calcKernelStreamSizeHuffman(
                                context,
                                huffman_config,
                                weight_x,
                                weight_y,
                                slice_count,
                                z_count,
                                kernels_per_core,
                                weight_format,
                                weight_zp[0],
                                bias_format,
                                input_zp,
                                skip_value,
                                z_offset,
                                (vx_uint8_ptr)weight_data,
                                (vx_uint32_ptr)bias_data);

        /*Only use old path to calculate nonZeroRatio & origKenelSize*/
        calculateWeightBiasBufferSizeForZeroRunLen(context, weight_x, weight_y, slice_count, z_count, kernels_per_core, weight_format, bias_format, skip_value, 0, weight_data, all_count, non_zero_count, orig_kernel_buf_size);
    }
    else if (context->options.enableNonZeroBalance)
    {
        if ((weight_x == 1 && weight_y == 1
            && (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3) || vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6))
            && (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8))
            && !gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_ZDP3_NO_COMPRESS_FIX))
        {
            /* Per HW ZDP bug, ZDP3 & ZDP6 zero length should always be 0*/
            minKernelBufferSize = calculateWeightBiasBalanceSizeForZeroRunLen(context, weight_x, weight_y, slice_count, z_count, kernels_per_core, weight_format, bias_format, skip_value, 0, weight_data, all_count, non_zero_count, orig_kernel_buf_size);
            minZeroRunLen = 0;
        }
        else if (set_zrl >= 0)
        {
            minKernelBufferSize = calculateWeightBiasBalanceSizeForZeroRunLen(context, weight_x, weight_y, slice_count, z_count, kernels_per_core, weight_format, bias_format, skip_value, set_zrl, weight_data, all_count, non_zero_count, orig_kernel_buf_size);

            minZeroRunLen = set_zrl;
        }
        else if (zeroRunLen > 5)
        {
            if (!calculateWeightBiasBalanceSizeForZeroRunLenEx(context, weight_x, weight_y, slice_count, z_count, kernels_per_core, weight_format, bias_format, skip_value, weight_data, all_count, non_zero_count, orig_kernel_buf_size, &minKernelBufferSize, &minZeroRunLen))
            {
                /* Back to calulate kernelBufferSize for other zeroRunLen. */
                for (zeroRunLen = 0; zeroRunLen <= context->nnConfig.fixedFeature.zrlBits; zeroRunLen++)
                {
                    vx_size kernelBufferSize = calculateWeightBiasBalanceSizeForZeroRunLen(context, weight_x, weight_y, slice_count, z_count, kernels_per_core, weight_format, bias_format, skip_value, zeroRunLen, weight_data, all_count, non_zero_count, orig_kernel_buf_size);

                    if (kernelBufferSize < minKernelBufferSize)
                    {
                        minKernelBufferSize = kernelBufferSize;
                        minZeroRunLen = zeroRunLen;
                    }
                }
            }
        }
        else
        {
            minKernelBufferSize = calculateWeightBiasBalanceSizeForZeroRunLen(context, weight_x, weight_y, slice_count, z_count, kernels_per_core, weight_format, bias_format, skip_value, zeroRunLen, weight_data, all_count, non_zero_count, orig_kernel_buf_size);

            minZeroRunLen = zeroRunLen;
        }
    }
    else
    {
        if ((weight_x == 1 && weight_y == 1
            && (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3) || vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6))
            && (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8))
            && !gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_ZDP3_NO_COMPRESS_FIX))
        {
            /* Per HW ZDP bug, ZDP3 & ZDP6 zero length should always be 0*/
            minKernelBufferSize = calculateWeightBiasBufferSizeForZeroRunLen(context, weight_x, weight_y, slice_count, z_count, kernels_per_core, weight_format, bias_format, skip_value, 0, weight_data, all_count, non_zero_count, orig_kernel_buf_size);
            minZeroRunLen = 0;
        }
        else if (set_zrl >= 0)
        {
            minKernelBufferSize = calculateWeightBiasBufferSizeForZeroRunLen(context, weight_x, weight_y, slice_count, z_count, kernels_per_core, weight_format, bias_format, skip_value, set_zrl, weight_data, all_count, non_zero_count, orig_kernel_buf_size);

            minZeroRunLen = set_zrl;
        }
        else if (zeroRunLen > 5)
        {
            if (!calculateWeightBiasBufferSizeForZeroRunLenEx(context, weight_x, weight_y, slice_count, z_count, kernels_per_core, weight_format, bias_format, skip_value, weight_data, all_count, non_zero_count, orig_kernel_buf_size, &minKernelBufferSize, &minZeroRunLen))
            {
                /* Back to calulate kernelBufferSize for other zeroRunLen. */
                for (zeroRunLen = 0; zeroRunLen <= context->nnConfig.fixedFeature.zrlBits; zeroRunLen++)
                {
                    vx_size kernelBufferSize = calculateWeightBiasBufferSizeForZeroRunLen(context, weight_x, weight_y, slice_count, z_count, kernels_per_core, weight_format, bias_format, skip_value, zeroRunLen, weight_data, all_count, non_zero_count, orig_kernel_buf_size);

                    if (kernelBufferSize < minKernelBufferSize)
                    {
                        minKernelBufferSize = kernelBufferSize;
                        minZeroRunLen = zeroRunLen;
                    }
                }
            }
        }
        else
        {
            minKernelBufferSize = calculateWeightBiasBufferSizeForZeroRunLen(context, weight_x, weight_y, slice_count, z_count, kernels_per_core, weight_format, bias_format, skip_value, zeroRunLen, weight_data, all_count, non_zero_count, orig_kernel_buf_size);

            minZeroRunLen = zeroRunLen;
        }
    }

    if (min_kernel_buf_size != VX_NULL)
        *min_kernel_buf_size = minKernelBufferSize;

    if (min_zero_run_len != VX_NULL)
        *min_zero_run_len = minZeroRunLen;
    if (max_zero_run_len != VX_NULL)
        *max_zero_run_len = (1 << minZeroRunLen) - 1;
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
    vx_float32 nonZeroRatio;

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

    nonZeroRatio = (vx_float32)nonZeroCount / (vx_float32)blockCount;

    return nonZeroRatio;
}

vx_size calculateTPWeightStreamSizeForZeroRunLen(
    vx_uint32 slice_count,
    vx_uint32 filter_count,
    vx_uint32 total_slice_count,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_uint32 skip_value,
    vx_uint8 zrl,
    vx_uint8_ptr weight_base_ptr,
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
    vx_uint32 bit_offset         = 0;
    vx_uint32 zeroRun           = 0;
    vx_uint32 filterIndex;
    vx_uint32 rsvWeightCount = 0, nonZeroCount = 0;

    /* Fill zeroRunLenBitWidth. */
    bit_offset = 4;
    for (filterIndex = 0; filterIndex < filterCount; filterIndex++)
    {
        vx_uint32 weight = 0;

        if (weight_format == VX_TYPE_INT8)
            weight = *((vx_int8 *)kernelDataPtr);
        else if (weight_format == VX_TYPE_UINT8)
            weight = *((vx_uint8 *)kernelDataPtr);
        else
            weight = *((vx_uint16 *)kernelDataPtr);

        kernelDataPtr += filterSize;

        if ((zeroRun == maxZeroRun)
            || (weight != skipValue)
            || (filterIndex == (filterCount - 1)))
        {
            /* Write zeroRun and weight. */
            bit_offset += zrl + weightBitSize;
            if (bit_offset >= 32)
            {
                bit_offset -= 32;
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
    if (bit_offset)
    {
        kernelBufferSize += 4;
    }
    kernelBufferSize = (kernelBufferSize + 63) & 0xFFFFFFC0;

    if (non_zero_count != VX_NULL) *non_zero_count = nonZeroCount;

    return kernelBufferSize;
}

void calculateWeightBiasTPBufferRelatedSize(
    vx_context context,
    vx_weight_bias_huffman_cfg huffman_config,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 weight_z,
    vx_uint32 slice_count,
    vx_uint32 filter_count,
    vx_uint32 total_slice_count,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_uint32 skip_value,
    vx_int8 set_zrl,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* all_count,
    vx_uint32* non_zero_count,
    vx_size* orig_kernel_buf_size,
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
    vx_uint32 bit_offset             = 0;
    vx_int8 zrlBitWidth             = set_zrl;
    vx_uint32 filterIndex, sliceIndex;
    vx_uint8 minZrlBitWidth;
    vx_size minWeightStreamSize;
    vx_uint32 nonZeroCount = 0;

    if(bias_format == VX_TYPE_INT64)
        biasSize = 4;

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_COMPRESSION_ENHANCEMENT))
    {
        if (all_count != VX_NULL)
            *all_count = weight_x * weight_y * slice_count * filter_count;
        if (orig_kernel_buf_size != VX_NULL)
            *orig_kernel_buf_size = slice_count * filter_count * weightSize + filter_count * biasSize;

        calcTPKernelBufferSizeHuffman(huffman_config, sliceCount, filterCount, weight_z, weight_format, skip_value, weight_base_ptr, non_zero_count, min_kernel_buf_size);
    }
    else
    {
        /* Fill kernel stream size for each slice. */
        for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
        {
            bit_offset += 5;
            if (bit_offset >= 32)
            {
                bit_offset -= 32;
                kernelBufferSize += 4;
            }
        }
        /* pad 0 */
        if (bit_offset)
        {
            bit_offset = 0;
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
                vx_uint32 nzcount;
                kernelDataPtr = weight_base_ptr + weightSize * sliceIndex;
                minWeightStreamSize = calculateTPWeightStreamSizeForZeroRunLen(slice_count, filter_count, total_slice_count, weight_format, bias_format, skip_value, zrlBitWidth, kernelDataPtr, &nzcount);

                nonZeroCount += nzcount;
                kernelBufferSize += minWeightStreamSize;
                if (min_zero_run_len != VX_NULL)
                    min_zero_run_len[sliceIndex] = zrlBitWidth;
            }
        }
        else
        {
            for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
            {
                vx_uint32 mnzcount = 0;

                kernelDataPtr = weight_base_ptr + weightSize * sliceIndex;
                /* Calculate the stream size of 0 zrlBitWidth. */
                minZrlBitWidth = 0;
                minWeightStreamSize = (1 + filterCount * weightSize + 63) & 0xFFFFFFC0;

                /* Calulate weightStreamSize for the rest zrlBitWidth. */
                for (zrlBitWidth = 1; zrlBitWidth <= 9; zrlBitWidth++)
                {
                    vx_uint32 nzcount;
                    vx_size weightStreamSize = calculateTPWeightStreamSizeForZeroRunLen(slice_count, filter_count, total_slice_count, weight_format, bias_format, skip_value, zrlBitWidth, kernelDataPtr, &nzcount);

                    if (!mnzcount) mnzcount = nzcount;
                    if (weightStreamSize < minWeightStreamSize)
                    {
                        minWeightStreamSize = weightStreamSize;
                        minZrlBitWidth = zrlBitWidth;
                        mnzcount = nzcount;
                    }
                }

                nonZeroCount += mnzcount;
                kernelBufferSize += minWeightStreamSize;
                if (min_zero_run_len != VX_NULL)
                    min_zero_run_len[sliceIndex] = minZrlBitWidth;
            }
        }

        if (all_count != VX_NULL)
            *all_count = weight_x * weight_y * slice_count * filter_count;
        if (orig_kernel_buf_size != VX_NULL)
            *orig_kernel_buf_size = slice_count * filter_count * (vx_uint32)vxDataType_GetSize(weight_format) + filter_count * biasSize;

        if (non_zero_count != VX_NULL)
            *non_zero_count  = nonZeroCount;
        if (min_kernel_buf_size != VX_NULL)
            *min_kernel_buf_size = kernelBufferSize;
    }
}

vx_uint32 fillinKernelBuffer(
    vx_context context,
    vx_uint8 min_zero_run_len,
    vx_uint32 max_zero_run,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 weight_z,
    vx_uint32 filter_count,
    vx_uint32 kernels_per_core,
    vx_enum  weight_format,
    vx_int32 coef_zp,
    vx_enum  bias_format,
    vx_int32 input_zp,
    vx_uint32 skip_value,
    vx_int32 z_offset,
    vx_uint8_ptr wb_base_ptr,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_weight_bias_z_offset z_offset_handle,
    vx_uint32* num_of_vz,
    vx_size* kernel_align_stream_size,
    vx_size* kernel_stream_full_cache_size,
    vx_size* kernel_max_stream_size_percore
    )
{
    vx_enum weightFomat = weight_format;
    vx_int32 inputZP = input_zp;
    vx_int32 coefZP = coef_zp;
    vx_uint32 skipValue = skip_value;

    vx_uint32 nnCoreCount = (weight_format == VX_TYPE_INT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : (weight_format == VX_TYPE_FLOAT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountFloat16 : (weight_format == VX_TYPE_BFLOAT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountBFloat16 :context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 coreIndex;
    vx_uint32 groupIndex, filterIndex, sliceIndex;
    vx_uint32 weightXIndex, weightYIndex;
    vx_uint32 bit_offset          = 0;
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
    vx_weight_bias_z_offset zOffsetHandle = z_offset_handle;

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

    if (num_of_vz != VX_NULL)
        *num_of_vz = filter_count;

    nonZeroRatioPerCorePerVZG = (vx_float64*)vxAllocateAndZeroMemory(nnCoreCount * groupCount * sizeof(vx_float64));
    if (nonZeroRatioPerCorePerVZG == VX_NULL)
    {
        vxError("fillinKernelBuffer: OUT OF MEMORY");
        goto exit;
    }

    /* kernel Buffer size for each core */
    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        writeBits(&kernelBufferPtr, &bit_offset, kernelStreamSize, 32);
    }
    packZeros(&kernelBufferPtr, &bit_offset, alignedOffset);
    if (kernel_stream_full_cache_size != VX_NULL)
        *kernel_stream_full_cache_size = 0;

    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_uint32 coreFilterCount = (coreIndex < oddFilterPerCore) ? totalFilterPerCore + 1 : totalFilterPerCore;
        kernelStreamBasePtr = (vx_uint8_ptr)kernelBufferPtr;

        /* zeroRunLen */
        if (coreFilterCount > 0)
        {
            /* zeroRunLen */
            writeBits(&kernelBufferPtr, &bit_offset, min_zero_run_len, 8);

            writeBits(&kernelBufferPtr, &bit_offset, coreFilterCount, 16);
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
                                    writeBits(&kernelBufferPtr, &bit_offset, zeroRun, min_zero_run_len);
                                }
                                writeBits(&kernelBufferPtr, &bit_offset, weight, weightBitSize);
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
                                        writeBits(&kernelBufferPtr, &bit_offset, bias32, 32);
                                        bias16 = (vx_int16)(bias64 >> 32);
                                        writeBits(&kernelBufferPtr, &bit_offset, (vx_int32)bias16, 16);
                                    }
                                    else
                                        writeBits(&kernelBufferPtr, &bit_offset, biasData, biasBitSize);
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
                                vx_uint32 offsetValue = (vx_uint32)z_offset * filterIndex;

                                if (zOffsetHandle != VX_NULL)
                                {
                                    zOffsetHandle->ptr_offset = (vx_uint32)((vx_uint8 *)kernelBufferPtr - wb_base_ptr);
                                    zOffsetHandle->bit_offset = bit_offset;
                                }

                                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                    writeBits(&kernelBufferPtr, &bit_offset, offsetValue, NN_Z_POSITION_OFFSET_BITS_VIP_V7);
                                else
                                    writeBits(&kernelBufferPtr, &bit_offset, offsetValue, NN_Z_POSITION_OFFSET_BITS);
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
                                                    writeBits(&kernelBufferPtr, &bit_offset, zeroRun, min_zero_run_len);
                                                }
                                                writeBits(&kernelBufferPtr, &bit_offset, weight, weightBitSize);
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
                                                        writeBits(&kernelBufferPtr, &bit_offset, bias32, 32);
                                                        bias16 = (vx_int16)(bias64 >> 32);
                                                        writeBits(&kernelBufferPtr, &bit_offset, (vx_int32)bias16, 16);
                                                    }
                                                    else
                                                        writeBits(&kernelBufferPtr, &bit_offset, biasData, biasBitSize);
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
                                                vx_uint32 offsetValue = (vx_uint32)z_offset * filterIndex;

                                                if (zOffsetHandle != VX_NULL)
                                                {
                                                    zOffsetHandle->ptr_offset = (vx_uint32)((vx_uint8 *)kernelBufferPtr - wb_base_ptr);
                                                    zOffsetHandle->bit_offset = bit_offset;
                                                }

                                                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                                    writeBits(&kernelBufferPtr, &bit_offset, offsetValue, NN_Z_POSITION_OFFSET_BITS_VIP_V7);
                                                else
                                                    writeBits(&kernelBufferPtr, &bit_offset, offsetValue, NN_Z_POSITION_OFFSET_BITS);
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
                                    if ((weightXIndex + 1) % 3 == 0 || weightXIndex == weight_x - 1)
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
                                        writeBits(&kernelBufferPtr, &bit_offset, zeroRun, min_zero_run_len);
                                    }
                                    writeBits(&kernelBufferPtr, &bit_offset, weight, weightBitSize);
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
                                            writeBits(&kernelBufferPtr, &bit_offset, bias32, 32);
                                            bias16 = (vx_int16)(bias64 >> 32);
                                            writeBits(&kernelBufferPtr, &bit_offset, (vx_int32)bias16, 16);
                                        }
                                        else
                                            writeBits(&kernelBufferPtr, &bit_offset, biasData, biasBitSize);
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
                            vx_uint32 offsetValue = (vx_uint32)z_offset * filterIndex;

                            if (zOffsetHandle != VX_NULL)
                            {
                                zOffsetHandle->ptr_offset = (vx_uint32)((vx_uint8 *)kernelBufferPtr - wb_base_ptr);
                                zOffsetHandle->bit_offset = bit_offset;
                            }

                            if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                writeBits(&kernelBufferPtr, &bit_offset, offsetValue, NN_Z_POSITION_OFFSET_BITS_VIP_V7);
                            else
                                writeBits(&kernelBufferPtr, &bit_offset, offsetValue, NN_Z_POSITION_OFFSET_BITS);

                        }
                    }
                }
            }
            nonZeroRatioPerCorePerVZG[nonZrlIdx] = (vx_float64)nonZeroCount / (vx_float64)blockCount;
        }

        /* pad 0 */
        packZeros(&kernelBufferPtr, &bit_offset, alignedOffset);

        /* Go back to update kernelStreamSize. */
        kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);
        /*With RTL bug fix, full cache mode size needn't using maxSize * coreCount*/
        if (kernel_stream_full_cache_size != VX_NULL)
            *kernel_stream_full_cache_size += kernelStreamSize;

        if (kernelStreamSize > maxKernelStreamSizePerCore)
            maxKernelStreamSizePerCore = kernelStreamSize;

        writeBits(&kernelStreamSizePtr, &streamSizeBitOffset, kernelStreamSize, 32);
    }

    if (kernel_align_stream_size != VX_NULL)
    {
                /*bug1980*/
        if(gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_IMAGE_NOT_PACKED_IN_SRAM_FIX))
            *kernel_align_stream_size += (vx_size)(maxKernelStreamSizePerCore * usedCoreCount );
        else
            *kernel_align_stream_size += (vx_size)(maxKernelStreamSizePerCore * nnCoreCount);

    }

    if (!hasKernelFullCacheInterleaveFix && kernel_stream_full_cache_size != VX_NULL)
    {
        /*if did't fix full cache interleave fix, full cache size also need use align size*/
        *kernel_stream_full_cache_size = (vx_size)(maxKernelStreamSizePerCore * usedCoreCount);
    }

    if (kernel_max_stream_size_percore != VX_NULL)
        *kernel_max_stream_size_percore = maxKernelStreamSizePerCore;

exit:
    if (weightsMinusZP)
        vxFree(weightsMinusZP);
    if (nonZeroRatioPerCorePerVZG)
        vxFree(nonZeroRatioPerCorePerVZG);

    return (vx_uint32)((vx_uint8 *)kernelBufferPtr - wb_base_ptr);
}

vx_uint32 fillinDepthWiseKernelBuffer(
    vx_context context,
    vx_uint8 min_zero_run_len,
    vx_uint32 max_zero_run,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 weight_z,
    vx_uint32 filter_count,
    vx_uint32 kernels_per_core,
    vx_enum  weight_format,
    vx_enum  weight_quant_format,
    vx_int32 coef_zp,
    vx_enum  bias_format,
    vx_int32 input_zp,
    vx_uint32 skip_value,
    vx_int32 z_offset,
    vx_uint8_ptr wb_base_ptr,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_uint32* num_of_vz,
    vx_size* kernel_align_stream_size,
    vx_size* kernel_stream_full_cache_size,
    vx_size* kernel_max_stream_size_percore
    )
{
    vx_enum weightFomat = weight_format;
    vx_int32 inputZP = input_zp;
    vx_int32 coefZP = coef_zp;

    vx_uint32 nnCoreCount = (weight_format == VX_TYPE_INT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : (weight_format == VX_TYPE_FLOAT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountFloat16 : (weight_format == VX_TYPE_BFLOAT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountBFloat16 :context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 coreIndex;
    vx_uint32 sliceIndex;
    vx_uint32 weightXIndex, weightYIndex;
    vx_uint32 bit_offset          = 0;

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
    vx_uint32 usedCoreCount      = (filterTotalCount > nnCoreCount) ? nnCoreCount : filterTotalCount;

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

    if (num_of_vz != VX_NULL)
        *num_of_vz = filterTotalCount;

    /* kernel Buffer size for each core */
    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        writeBits(&kernelBufferPtr, &bit_offset, kernelStreamSize, 32);
    }
    packZeros(&kernelBufferPtr, &bit_offset, alignedOffset);
    if (kernel_stream_full_cache_size != VX_NULL)
        *kernel_stream_full_cache_size = 0;

    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_uint32 coreFilterCount = (coreIndex < oddFilterPerCore) ? totalFilterPerCore + 1 : totalFilterPerCore;
        vx_uint32 groupIndex;

        kernelStreamBasePtr = (vx_uint8_ptr)kernelBufferPtr;

        /* zeroRunLen */
        writeBits(&kernelBufferPtr, &bit_offset, min_zero_run_len, 8);
        /*kernels num in each core*/
        writeBits(&kernelBufferPtr, &bit_offset, coreFilterCount, 16);

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
                                        writeBits(&kernelBufferPtr, &bit_offset, bias32, 32);
                                        bias16 = (vx_int16)(bias64 >> 32);
                                        writeBits(&kernelBufferPtr, &bit_offset, (vx_int32)bias16, 16);
                                    }
                                    else
                                        writeBits(&kernelBufferPtr, &bit_offset, biasData, biasBitSize);
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
                                        writeBits(&kernelBufferPtr, &bit_offset, zeroRun, min_zero_run_len);
                                    }
                                    writeBits(&kernelBufferPtr, &bit_offset, weight, weightBitSize);
                                    zeroRun = 0;
                                }
                                else
                                {
                                    zeroRun++;
                                }

                                writeBits(&kernelBufferPtr, &bit_offset, weight, weightBitSize);

                                if (realWeightX == weight_x - 1 &&
                                    realWeightY == weight_y - 1)
                                {
                                    /*ZOFFSET*/
                                    vx_uint32 offsetValue = (vx_uint32)z_offset * realSliceIndex;

                                    writeBits(&kernelBufferPtr, &bit_offset, offsetValue, NN_Z_POSITION_OFFSET_BITS_VIP_V7);

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
                                            writeBits(&kernelBufferPtr, &bit_offset, bias32, 32);
                                            bias16 = (vx_int16)(bias64 >> 32);
                                            writeBits(&kernelBufferPtr, &bit_offset, (vx_int32)bias16, 16);
                                        }
                                        else
                                            writeBits(&kernelBufferPtr, &bit_offset, biasData, biasBitSize);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        /* pad 0 */
        packZeros(&kernelBufferPtr, &bit_offset, alignedOffset);

        /* Go back to update kernelStreamSize. */
        kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);
        /*With RTL bug fix, full cache mode size needn't using maxSize * coreCount*/
        if (kernel_stream_full_cache_size != VX_NULL)
            *kernel_stream_full_cache_size += kernelStreamSize;

        if (kernelStreamSize > maxKernelStreamSizePerCore)
            maxKernelStreamSizePerCore = kernelStreamSize;

        writeBits(&kernelStreamSizePtr, &streamSizeBitOffset, kernelStreamSize, 32);
    }

    if (kernel_align_stream_size != VX_NULL)
    {
                        /*bug1980*/
        if(gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_IMAGE_NOT_PACKED_IN_SRAM_FIX))
            *kernel_align_stream_size += (vx_size)(maxKernelStreamSizePerCore * usedCoreCount );
        else
            *kernel_align_stream_size += (vx_size)(maxKernelStreamSizePerCore * nnCoreCount);

    }

    if (!hasKernelFullCacheInterleaveFix && kernel_stream_full_cache_size != VX_NULL)
    {
        /*if did't fix full cache interleave fix, full cache size also need use align size*/
        *kernel_stream_full_cache_size = (vx_size)(maxKernelStreamSizePerCore * nnCoreCount);
    }

    if (kernel_max_stream_size_percore != VX_NULL)
        *kernel_max_stream_size_percore = maxKernelStreamSizePerCore;

exit:
    if (weightsMinusZP)
        vxFree(weightsMinusZP);

    return (vx_uint32)((vx_uint8 *)kernelBufferPtr - wb_base_ptr);
}

vx_uint32 fillinKernelBufferBalance(
    vx_context context,
    vx_uint8 min_zero_run_len,
    vx_uint32 max_zero_run,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 weight_z,
    vx_uint32 filter_count,
    vx_uint32 kernels_per_core,
    vx_enum  weight_format,
    vx_int32 coef_zp,
    vx_enum  bias_format,
    vx_int32 input_zp,
    vx_uint32 skip_value,
    vx_int32 z_offset,
    vx_uint8_ptr wb_base_ptr,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_weight_bias_z_offset z_offset_handle,
    vx_uint32* num_of_vz,
    vx_size* kernel_align_stream_size,
    vx_size* kernel_stream_full_cache_size,
    vx_size* kernel_max_stream_size_percore
    )
{
    vx_enum weightFomat = weight_format;
    vx_int32 inputZP = input_zp;
    vx_int32 coefZP = coef_zp;
    vx_uint32 skipValue = skip_value;

    vx_uint32 nnCoreCount = (weight_format == VX_TYPE_INT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : (weight_format == VX_TYPE_FLOAT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountFloat16 : (weight_format == VX_TYPE_BFLOAT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountBFloat16 :context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 coreIndex;
    vx_uint32 groupIndex, filterIndex, sliceIndex;
    vx_uint32 weightXIndex, weightYIndex;
    vx_uint32 bit_offset          = 0;
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
    vx_weight_bias_z_offset zOffsetHandle = z_offset_handle;

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

    nonZeroRatioPerCorePerVZG = (vx_float64*)vxAllocateAndZeroMemory(nnCoreCount * groupCount * sizeof(vx_float64));
    if (nonZeroRatioPerCorePerVZG == VX_NULL)
    {
        vxError("fillinKernelBuffer: OUT OF MEMORY");
        goto exit;
    }

    if (num_of_vz != VX_NULL)
        *num_of_vz = filter_count;

    /* kernel Buffer size for each core */
    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        writeBits(&kernelBufferPtr, &bit_offset, kernelStreamSize, 32);
    }
    packZeros(&kernelBufferPtr, &bit_offset, alignedOffset);
    if (kernel_stream_full_cache_size != VX_NULL)
        *kernel_stream_full_cache_size = 0;

    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_uint32 coreFilterCount = (coreIndex < oddFilterPerCore) ? totalFilterPerCore + 1 : totalFilterPerCore;
        vx_uint32 usedKid = 0;
        vx_uint32* filterGroup = VX_NULL;

        kernelStreamBasePtr = (vx_uint8_ptr)kernelBufferPtr;
        if (coreFilterCount > 0)
        {
            /* zeroRunLen */
            writeBits(&kernelBufferPtr, &bit_offset, min_zero_run_len, 8);

            writeBits(&kernelBufferPtr, &bit_offset, coreFilterCount, 16);
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
                                    writeBits(&kernelBufferPtr, &bit_offset, zeroRun, min_zero_run_len);
                                }
                                writeBits(&kernelBufferPtr, &bit_offset, weight, weightBitSize);
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
                                        writeBits(&kernelBufferPtr, &bit_offset, bias32, 32);
                                        bias16 = (vx_int16)(bias64 >> 32);
                                        writeBits(&kernelBufferPtr, &bit_offset, (vx_int32)bias16, 16);
                                    }
                                    else
                                        writeBits(&kernelBufferPtr, &bit_offset, biasData, biasBitSize);
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
                                vx_uint32 offsetValue = (vx_uint32)z_offset * filterIndex;

                                if (zOffsetHandle != VX_NULL)
                                {
                                    zOffsetHandle->ptr_offset = (vx_uint32)((vx_uint8 *)kernelBufferPtr - wb_base_ptr);
                                    zOffsetHandle->bit_offset = bit_offset;
                                }

                                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                    writeBits(&kernelBufferPtr, &bit_offset, offsetValue, NN_Z_POSITION_OFFSET_BITS_VIP_V7);
                                else
                                    writeBits(&kernelBufferPtr, &bit_offset, offsetValue, NN_Z_POSITION_OFFSET_BITS);
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
                                                    writeBits(&kernelBufferPtr, &bit_offset, zeroRun, min_zero_run_len);
                                                }
                                                writeBits(&kernelBufferPtr, &bit_offset, weight, weightBitSize);
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
                                                        writeBits(&kernelBufferPtr, &bit_offset, bias32, 32);
                                                        bias16 = (vx_int16)(bias64 >> 32);
                                                        writeBits(&kernelBufferPtr, &bit_offset, (vx_int32)bias16, 16);
                                                    }
                                                    else
                                                        writeBits(&kernelBufferPtr, &bit_offset, biasData, biasBitSize);
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
                                                vx_uint32 offsetValue = (vx_uint32)z_offset * filterIndex;

                                                if (zOffsetHandle)
                                                {
                                                    zOffsetHandle->ptr_offset = (vx_uint32)((vx_uint8 *)kernelBufferPtr - wb_base_ptr);
                                                    zOffsetHandle->bit_offset = bit_offset;
                                                }

                                                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                                    writeBits(&kernelBufferPtr, &bit_offset, offsetValue, NN_Z_POSITION_OFFSET_BITS_VIP_V7);
                                                else
                                                    writeBits(&kernelBufferPtr, &bit_offset, offsetValue, NN_Z_POSITION_OFFSET_BITS);
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
                                    if ((weightXIndex + 1) % 3 == 0 || weightXIndex == weight_x - 1)
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
                                        writeBits(&kernelBufferPtr, &bit_offset, zeroRun, min_zero_run_len);
                                    }
                                    writeBits(&kernelBufferPtr, &bit_offset, weight, weightBitSize);
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
                                                writeBits(&kernelBufferPtr, &bit_offset, bias32, 32);
                                                bias16 = (vx_int16)(biasData_64bits >> 32);
                                                writeBits(&kernelBufferPtr, &bit_offset, (vx_int32)bias16, 16);
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
                                                writeBits(&kernelBufferPtr, &bit_offset, bias32, 32);
                                                bias16 = (vx_int16)(bias64 >> 32);
                                                writeBits(&kernelBufferPtr, &bit_offset, (vx_int32)bias16, 16);
                                            }
                                        }
                                        else
                                            writeBits(&kernelBufferPtr, &bit_offset, biasData, biasBitSize);
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
                            vx_uint32 offsetValue = (vx_uint32)z_offset * filterIndex;

                            if (zOffsetHandle)
                            {
                                zOffsetHandle->ptr_offset = (vx_uint32)((vx_uint8 *)kernelBufferPtr - wb_base_ptr);
                                zOffsetHandle->bit_offset = bit_offset;
                            }

                            if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                writeBits(&kernelBufferPtr, &bit_offset, offsetValue, NN_Z_POSITION_OFFSET_BITS_VIP_V7);
                            else
                                writeBits(&kernelBufferPtr, &bit_offset, offsetValue, NN_Z_POSITION_OFFSET_BITS);

                        }
                    }
                }
            }
            nonZeroRatioPerCorePerVZG[nonZrlIdx] = (vx_float64)nonZeroCount / (vx_float64)blockCount;

            usedKid += actualFilterCount;
            if (filterGroup)
            {
                vxFree(filterGroup);
                filterGroup = VX_NULL;
            }
        }

        /* pad 0 */
        packZeros(&kernelBufferPtr, &bit_offset, alignedOffset);

        /* Go back to update kernelStreamSize. */
        kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);
        /*With RTL bug fix, full cache mode size needn't using maxSize * coreCount*/
        if (kernel_stream_full_cache_size != VX_NULL)
            *kernel_stream_full_cache_size += kernelStreamSize;

        if (kernelStreamSize > maxKernelStreamSizePerCore)
            maxKernelStreamSizePerCore = kernelStreamSize;

        writeBits(&kernelStreamSizePtr, &streamSizeBitOffset, kernelStreamSize, 32);
#if PRINT_PER_CORE_SIZE
        printf("coreIdx %d, kernel stream size is %d, non-zero coef count is %d\n", coreIndex, kernelStreamSize, nonZeroCoefPerCore[coreIndex]);
#endif
    }

    if (kernel_align_stream_size != VX_NULL)
    {
        /*bug1980*/
        if(gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_IMAGE_NOT_PACKED_IN_SRAM_FIX))
            *kernel_align_stream_size += (vx_size)(maxKernelStreamSizePerCore * usedCoreCount );
        else
            *kernel_align_stream_size += (vx_size)(maxKernelStreamSizePerCore * nnCoreCount);
    }


    if (!hasKernelFullCacheInterleaveFix && kernel_stream_full_cache_size != VX_NULL)
    {
        /*if did't fix full cache interleave fix, full cache size also need use align size*/
        *kernel_stream_full_cache_size = (vx_size)(maxKernelStreamSizePerCore * usedCoreCount);
    }

    if (kernel_max_stream_size_percore != VX_NULL)
        *kernel_max_stream_size_percore = maxKernelStreamSizePerCore;

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

    return (vx_uint32)((vx_uint8 *)kernelBufferPtr - wb_base_ptr);
}

vx_uint32 fillinTPKernelBuffer(
    vx_context context,
    vx_uint8* zero_run_len_bit_width,
    vx_uint32 weight_z,
    vx_uint32 filter_count,
    vx_uint32 total_weight_z,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_int8  bias_fpp,
    vx_uint32 skip_value,
    vx_uint8_ptr wb_base_ptr,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr
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
    vx_uint32 bit_offset             = 0;
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
        writeBits(&kernelBufferPtr, &bit_offset, kernelStreamSize, 5);
    }
    packZeros(&kernelBufferPtr, &bit_offset, alignedOffset);

    /* Fill bias for each filter. */
    for (filterIndex = 0; filterIndex < filterCount; filterIndex++)
    {
        if(vxoContext_IsFeatureAvailable(context, VX_TP_FEATURE_FP32_BIAS))
        {
            vx_float32 biasFloat32;
           if(bias_format == VX_TYPE_INT64)
            {
                vx_int64  biasData64bits         = 0;
                biasBitSize = 32;
                biasData64bits = bias_base_ptr == VX_NULL ? 0 : *((vx_int64 *)bias_base_ptr + filterIndex);
                biasFloat32 = Int64toFp32(biasData64bits, bias_fpp);
            }
            else
            {
                biasData = bias_base_ptr == VX_NULL ? 0 : *(bias_base_ptr + filterIndex);
                biasFloat32 = Int32toFp32(biasData, bias_fpp);
            }
           writeBits(&kernelBufferPtr, &bit_offset, *(vx_uint32 *)&biasFloat32, biasBitSize);
        }
        else
        {
            if(bias_format == VX_TYPE_INT64)
            {
                vx_int32 bias32;
                vx_int64  biasData64bits         = 0;
                biasBitSize = 32;
                biasData64bits = bias_base_ptr == VX_NULL ? 0 : *((vx_int64 *)bias_base_ptr + filterIndex);
                bias32 = biasData64bits & 0xFFFFFFFF;
                writeBits(&kernelBufferPtr, &bit_offset, bias32, biasBitSize);
            }
            else
            {
                biasData = bias_base_ptr == VX_NULL ? 0 : *(bias_base_ptr + filterIndex);
                writeBits(&kernelBufferPtr, &bit_offset, biasData, biasBitSize);
            }
        }
    }
    packZeros(&kernelBufferPtr, &bit_offset, alignedOffset);

    /* Fill weight value for every slice */
    zeroRun = 0;
    for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
    {
        vx_uint8 zeroRunLenBitWidth = zero_run_len_bit_width[sliceIndex];
        vx_uint32 maxZeroRun = (1 << zeroRunLenBitWidth) - 1;

        kernelStreamBasePtr = (vx_uint8_ptr)kernelBufferPtr;
        writeBits(&kernelBufferPtr, &bit_offset, zeroRunLenBitWidth, 4);

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
                    writeBits(&kernelBufferPtr, &bit_offset, zeroRun, zeroRunLenBitWidth);
                }

                if (weight_format == VX_TYPE_BFLOAT16)
                {
                    vx_uint16 exp = (weight & 0x7F80) >> 7;
                    vx_uint16 mantissa = weight & 0x7F;
                    vx_uint8 signedBit = (weight & 0x8000) >> 15;
                    vx_uint16 temp = 0;

                    /*we didn't suppot INF & NAN*/
                    vxmASSERT(exp != 0xFFFF);

                    /* convert -zero to +zero*/
                    if (exp == 0 && mantissa == 0)
                        signedBit = 0;

                    /*newCoef [15:0] = {~OrgValue[14],OrgValue[13:0], OrgValue[15]}*/
#if !FEATURE_BFP16_ZRL_ENABLE
                    exp = exp ^ 0x80; /*~OrgValue[14]*/
#endif
                    temp = (exp << 8) | (mantissa << 1) | signedBit;

                    weight = temp;

                }
                else if (weight_format == VX_TYPE_FLOAT16)
                {
                    vx_uint16 exp = (weight & 0x7C00) >> 10;
                    vx_uint16 mantissa = weight & 0x3FF;
                    vx_uint8 signedBit = (weight & 0x8000) >> 15;
                    vx_uint16 temp = 0;

                    /* convert -zero to +zero*/
                    if (exp == 0 && mantissa == 0 && signedBit == 1)
                        signedBit = 0;
                    temp = (exp << 10) | mantissa | signedBit << 15;

                    weight = temp;

                }

                writeBits(&kernelBufferPtr, &bit_offset, weight, weightBitSize);
                zeroRun = 0;
                rsvWeightCount++;
            }
            else
            {
                zeroRun++;
            }
        }
        packZeros(&kernelBufferPtr, &bit_offset, alignedOffset);

        /* Go back to update kernelStreamSize. */
        kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);
        gcmASSERT((kernelStreamSize & 0x3F) == 0);
        gcmASSERT(kernelStreamSize < 2048);
        writeBits(&kernelStreamSizePtr, &streamSizeBitOffset, kernelStreamSize >> 6, 5);
    }

    return (vx_uint32)((vx_uint8 *)kernelBufferPtr - wb_base_ptr);
}

vx_uint32 fillinTPKernelBufferHuffman(
    vx_context context,
    vx_weight_bias_huffman_cfg huffman_config,
    vx_uint8* zero_run_len_bit_width,
    vx_uint32 weight_z,
    vx_uint32 filter_count,
    vx_uint32 total_weight_z,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_int8  bias_fpp,
    vx_uint32 skip_value,
    vx_uint8_ptr wb_base_ptr,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr
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
    vx_uint32 bit_offset             = 0;
    vx_uint32* kernelStreamSizePtr  = kernelBufferPtr;
    vx_uint32 streamSizeBitOffset   = 0;
    vx_uint32 alignedOffset         = ((vx_uint32)(gctUINTPTR_T)kernelBufferPtr) & 0x3F;
    vx_uint8_ptr kernelDataPtr      = VX_NULL;
    vx_uint8_ptr kernelStreamBasePtr = VX_NULL;

    vx_weight_bias_huffman_cfg huffmanConfig = huffman_config;

    vx_uint32 i;
    vx_uint32 sliceIndex = 0;
    vx_uint32 filterIndex = 0;

    vx_uint32 bit16_flag = (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8) ? 0 : 1;

    vx_uint32 reorderStreamAllCount = sliceCount * filterCount;
    vx_uint32 reorderStreamSize = reorderStreamAllCount * weightSize;

    vx_uint8* reorderStream = VX_NULL;
    vx_uint32 runSet2Bits = 1;
    CodeSymbol codeSymbol[THROUGHPUT * 3];

    vx_uint32 dummyStage[3] = {0};
    vx_uint32 dummyBitLength[3] = {0};
    vx_bool setDummy = vx_false_e;

    vxmASSERT(huffmanConfig != VX_NULL);

    reorderStream = (vx_uint8 *)vxAllocateAndZeroMemory(reorderStreamSize);

    if (!reorderStream)
    {
        vxError("fillinKernelBufferHuffman: OUT OF MEMORY");
        goto exit;
    }

    reorderTPKernelBufferHuffman(weight_z, filter_count, total_weight_z, weight_format, reorderStream, weight_base_ptr, VX_NULL, VX_NULL);

    analysisTPStreamForHuffman(huffman_config, sliceCount, filterCount, weight_format, reorderStream, invSizeOrder);

    /* Write huffman header*/
    writeBits(&kernelBufferPtr, &bit_offset, huffmanConfig->pre_encode, 1);
    writeBits(&kernelBufferPtr, &bit_offset, huffmanConfig->bit16_flag, 1);
    writeBits(&kernelBufferPtr, &bit_offset, huffmanConfig->fp16_flag, 1);
    writeBits(&kernelBufferPtr, &bit_offset, huffmanConfig->reserved, 1);
    writeBits(&kernelBufferPtr, &bit_offset, huffmanConfig->version, 4);
    writeBits(&kernelBufferPtr, &bit_offset, huffmanConfig->run_len_table_size, 8);

    for(i = 0; i < MAX_RUNLEN_SIZE; i++)
    {
        writeBits(&kernelBufferPtr, &bit_offset, huffmanConfig->run_len_table[i], 8);
    }
    for(i = 0; i < LOG_RUN_SIZE1; i++)
    {
        writeBits(&kernelBufferPtr, &bit_offset, huffmanConfig->map_to_huffman[i], 8);
    }
    writeBits(&kernelBufferPtr, &bit_offset, huffmanConfig->avg_bias, 16);
    writeBits(&kernelBufferPtr, &bit_offset, 0, 16); /*reserved, must zero*/

    kernelStreamSizePtr = kernelBufferPtr;
    streamSizeBitOffset = bit_offset;

    /* Fill kernel stream size for each slice. */
    for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
    {
        writeBits(&kernelBufferPtr, &bit_offset, kernelStreamSize, 14);
    }
    packZeros(&kernelBufferPtr, &bit_offset, alignedOffset);

    /* Fill bias for each filter. */
    for (filterIndex = 0; filterIndex < filterCount; filterIndex++)
    {
        if(vxoContext_IsFeatureAvailable(context, VX_TP_FEATURE_FP32_BIAS))
        {
            vx_float32 biasFloat32;
           if(bias_format == VX_TYPE_INT64)
            {
                vx_int64  biasData64bits         = 0;
                biasBitSize = 32;
                biasData64bits = bias_base_ptr == VX_NULL ? 0 : *((vx_int64 *)bias_base_ptr + filterIndex);
                biasFloat32 = Int64toFp32(biasData64bits, bias_fpp);
            }
            else
            {
                biasData = bias_base_ptr == VX_NULL ? 0 : *(bias_base_ptr + filterIndex);
                biasFloat32 = Int32toFp32(biasData, bias_fpp);
            }
           writeBits(&kernelBufferPtr, &bit_offset, *(vx_uint32 *)&biasFloat32, biasBitSize);
        }
        else
        {
            if(bias_format == VX_TYPE_INT64)
            {
                vx_int32 bias32;
                vx_int64  biasData64bits         = 0;
                biasBitSize = 32;
                biasData64bits = bias_base_ptr == VX_NULL ? 0 : *((vx_int64 *)bias_base_ptr + filterIndex);
                bias32 = biasData64bits & 0xFFFFFFFF;
                writeBits(&kernelBufferPtr, &bit_offset, bias32, biasBitSize);
            }
            else
            {
                biasData = bias_base_ptr == VX_NULL ? 0 : *(bias_base_ptr + filterIndex);
                writeBits(&kernelBufferPtr, &bit_offset, biasData, biasBitSize);
            }
        }
    }
    packZeros(&kernelBufferPtr, &bit_offset, alignedOffset);


    for (i = 0; i < THROUGHPUT * 3; i++)
    {
        codeSymbol[i].stageCode[0] = codeSymbol[i].stageCode[1] = codeSymbol[i].stageCode[2] = 0;
        codeSymbol[i].bitLength[0] = codeSymbol[i].bitLength[1] = codeSymbol[i].bitLength[2] = 0;
    }

    while(((huffmanConfig->run_len_table_size - SET_1_SIZE)>>runSet2Bits) > 1)
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

            if (bit16_flag == 0)
            {
                coef = (coef - huffmanConfig->avg_bias) & 0xFF;
                if (huffmanConfig->pre_encode)
                {
                    tmp = coef;
                    coef = (coef - prev) & 0xFF;
                    prev = tmp;
                }
            }
            else
            {
                if (huffmanConfig->fp16_flag)
                {
                    coef16 = (coef16 & 0x7FFF) * 2 + ((coef16 & (1<<15)) >> 15);
                }
                coef16 -= huffmanConfig->avg_bias;
                if (huffmanConfig->run_len_table_size == 0x0)
                    coef = (coef16 >> 8) & 0xFF; /*Non RZL*/
                else
                    coef = coef16 & 0xFFFF; /*RZL path*/
            }

            if (huffmanConfig->run_len_table_size == 0x0) /* non RZL*/
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

                    if (huffmanConfig->bit16_flag)
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

                if (huffmanConfig->bit16_flag)
                {/*pack the raw data of the lower 8 bits*/

                    codeSymbol[pos].stageCode[2] = codeSymbol[x%(3*THROUGHPUT)].stageCode[2]* 256 + (coef16 &0xff);
                    codeSymbol[pos].bitLength[2] += 8;
                }
                OutputAt(x, &kernelBufferPtr, &bit_offset, codeSymbol);
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
                    if (huffmanConfig->bit16_flag)
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
                        for (j = huffmanConfig->run_len_table_size - 1; j>=0; j--)
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
                                for (m = 0; m < huffmanConfig->run_len_table_size - SET_1_SIZE; m++)
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

                                OutputAt(x, &kernelBufferPtr, &bit_offset, codeSymbol);
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
                        coef>>=(huffmanConfig->bit16_flag * 8);
                        if(coef<0x80)
                            coef+=huffmanConfig->bit16_flag; /*Since we have to encode zero (run-length only handle 16-bits zero, 8 bit zero should use Size code*/
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
                            if(huffmanConfig->bit16_flag)
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
                        if (huffmanConfig->bit16_flag)
                        {
                            codeSymbol[pos].stageCode[2] = codeSymbol[x%(3*THROUGHPUT)].stageCode[2]* 256 + (coef16 & 0xff);
                            codeSymbol[pos].bitLength[2] += 8;
                        }
                        OutputAt(x, &kernelBufferPtr, &bit_offset, codeSymbol);
                        x++; /*counter*/
                    }

                }
            }

            if(dataRemainingOfCore == 0)
            {
                vx_uint32 dummyCount = (x & 0x1)? 5: 4;
                /*Hit the end bitsteam of core, add dummy stage process the next*/
                vxmASSERT(setDummy == vx_true_e);
                addDummy(x, &kernelBufferPtr, &bit_offset, codeSymbol, dummyCount, dummyStage, dummyBitLength);
                break;
            }
        }

        kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);
        /*There's 14 bit to save kernel stream bit size*/
        gcmASSERT((kernelStreamSize*8 + bit_offset) < 16384);
        /* Go back to update kernelStreamSize. */
        writeBits(&kernelStreamSizePtr, &streamSizeBitOffset, kernelStreamSize*8 + bit_offset, 14);
        /*align to 64 byte after each kz*/
        packZeros(&kernelBufferPtr, &bit_offset, alignedOffset);
        prev = 0;

    }

    kernelSizePerCore = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStartPtr);

exit:
    if (reorderStream)
        vxFree(reorderStream);

    return kernelSizePerCore;
}

vx_uint32 fillinKernelBufferHuffman(
    vx_context context,
    vx_weight_bias_huffman_cfg huffman_config,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_int32 weight_zp,
    vx_enum  bias_format,
    vx_int32 input_zp,
    vx_uint32 skip_value,
    vx_int32 z_offset,
    vx_uint8_ptr wb_base_ptr,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_size* kernel_align_stream_size,
    vx_size* kernel_stream_full_cache_size,
    vx_size* kernel_max_stream_size_percore
    )
{
    vx_uint32 nnCoreCount = (weight_format == VX_TYPE_INT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : (weight_format == VX_TYPE_FLOAT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountFloat16 : (weight_format == VX_TYPE_BFLOAT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountBFloat16 :context->nnConfig.fixedFeature.nnCoreCount;

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

    vx_uint32 bit16_flag = (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8) ? 0 : 1;
    vx_uint32 bit_offset = 0;

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
    vx_weight_bias_huffman_cfg huffmanConfig = huffman_config;

    vxmASSERT(huffmanConfig != VX_NULL);

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
        reorderKernelBufferV7HuffmanBalance(context, weight_x, weight_y, slice_count, z_count, filters_per_core, weight_format, weight_zp, bias_format, input_zp, z_offset, skip_value, reorderStream, weight_base_ptr, bias_base_ptr, VX_NULL, VX_NULL, reorderStreamPerCoreCount, nonCoefIndex, limitZRLIndex, vx_false_e);

    }
    else
    {
        reorderWeightBiasBufferForHuffman(context, weight_x, weight_y, slice_count, z_count, filters_per_core, weight_format, weight_zp, bias_format, input_zp, z_offset, skip_value, reorderStream, weight_base_ptr, bias_base_ptr, VX_NULL, VX_NULL, reorderStreamPerCoreCount, nonCoefIndex, limitZRLIndex, vx_false_e);

    }

    analysisKernelStreamForHuffman(context, huffman_config, weight_format, 0, nnCoreCount, reorderStream, reorderStreamSize, reorderStreamPerCoreCount, invSizeOrder, nonCoefIndex, nonCoefCount, limitZRLIndex, limitZRLCount);

    /* Write huffman header*/
    writeBits(&kernelBufferPtr, &bit_offset, huffmanConfig->pre_encode, 1);
    writeBits(&kernelBufferPtr, &bit_offset, huffmanConfig->bit16_flag, 1);
    writeBits(&kernelBufferPtr, &bit_offset, huffmanConfig->fp16_flag, 1);
    writeBits(&kernelBufferPtr, &bit_offset, huffmanConfig->reserved, 1);
    writeBits(&kernelBufferPtr, &bit_offset, huffmanConfig->version, 4);
    writeBits(&kernelBufferPtr, &bit_offset, huffmanConfig->run_len_table_size, 8);

    for(i = 0; i < MAX_RUNLEN_SIZE; i++)
    {
        writeBits(&kernelBufferPtr, &bit_offset, huffmanConfig->run_len_table[i], 8);
    }
    for(i = 0; i < LOG_RUN_SIZE1; i++)
    {
        writeBits(&kernelBufferPtr, &bit_offset, huffmanConfig->map_to_huffman[i], 8);
    }
    writeBits(&kernelBufferPtr, &bit_offset, huffmanConfig->avg_bias, 16);
    writeBits(&kernelBufferPtr, &bit_offset, 0, 16); /*reserved, must zero*/

    kernelStreamSizePtr = kernelBufferPtr;
    streamSizeBitOffset = bit_offset;
    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        writeBits(&kernelBufferPtr, &bit_offset, kernelStreamSize, 32);
    }
    /*align to 64 byte*/
    packZeros(&kernelBufferPtr, &bit_offset, alignedOffset);
    if (kernel_stream_full_cache_size != VX_NULL)
        *kernel_stream_full_cache_size = 0;

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
                limitZRLCountIdx < limitZRLCount)
            {
                isLimitZRL = vx_true_e;
                limitZRLCountIdx++;
            }
            else
                isLimitZRL = vx_false_e;

            totalCountIdx++;

            if (bit16_flag == 0)
            {

                coef = (coef - huffmanConfig->avg_bias) & 0xFF;
                if (huffmanConfig->pre_encode)
                {
                    tmp = coef;
                    coef = (coef - prev) & 0xFF;
                    prev = tmp;
                }
            }
            else
            {

                if (huffmanConfig->fp16_flag)
                {
                    coef16 = (coef16 & 0x7FFF) * 2 + ((coef16 & (1<<15)) >> 15);
                }
                coef16 -= huffmanConfig->avg_bias;
                if (huffmanConfig->run_len_table_size == 0x0)
                    coef = (coef16 >> 8) & 0xFF; /*Non RZL*/
                else
                    coef = coef16 & 0xFFFF; /*RZL path*/
            }

            if (huffmanConfig->run_len_table_size == 0x0) /* non RZL*/
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

                    if (huffmanConfig->bit16_flag)
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

                if (huffmanConfig->bit16_flag)
                {/*pack the raw data of the lower 8 bits*/

                    codeSymbol[pos].stageCode[2] = codeSymbol[x%(3*THROUGHPUT)].stageCode[2]* 256 + (coef16 &0xff);
                    codeSymbol[pos].bitLength[2] += 8;
                }
                OutputAt(x, &kernelBufferPtr, &bit_offset, codeSymbol);
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
                    if (huffmanConfig->bit16_flag)
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

                                OutputAt(x, &kernelBufferPtr, &bit_offset, codeSymbol);
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
                        coef>>=(huffmanConfig->bit16_flag * 8);
                        if(coef<0x80)
                            coef+=huffmanConfig->bit16_flag; /*Since we have to encode zero (run-length only handle 16-bits zero, 8 bit zero should use Size code*/
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
                            if(huffmanConfig->bit16_flag)
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
                        if (huffmanConfig->bit16_flag)
                        {
                            codeSymbol[pos].stageCode[2] = codeSymbol[x%(3*THROUGHPUT)].stageCode[2]* 256 + (coef16 & 0xff);
                            codeSymbol[pos].bitLength[2] += 8;
                        }
                        OutputAt(x, &kernelBufferPtr, &bit_offset, codeSymbol);
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
                        if (huffmanConfig->bit16_flag)
                        {
                            codeSymbol[pos].stageCode[2] = codeSymbol[x%(3*THROUGHPUT)].stageCode[2]* 256 + (coef16 & 0xff);
                            codeSymbol[pos].bitLength[2] += 8;
                        }
                        OutputAt(x, &kernelBufferPtr, &bit_offset, codeSymbol);
                        x++; /*counter*/
                    }
                }
            }

            if(dataRemainingOfCore == 0)
            {
                vx_uint32 dummyCount = (x & 0x1)? 5: 4;
                /*Hit the end bitsteam of core, add dummy stage process the next*/
                vxmASSERT(setDummy == vx_true_e);
                addDummy(x, &kernelBufferPtr, &bit_offset, codeSymbol, dummyCount, dummyStage, dummyBitLength);
                break;
            }
        }

        /* Go back to update kernelStreamSize. */
        kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);
        writeBits(&kernelStreamSizePtr, &streamSizeBitOffset, kernelStreamSize*8 + bit_offset, 32);

        /* pad 0 at the end of core*/
        packZeros(&kernelBufferPtr, &bit_offset, alignedOffset);
        kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);

        /*With RTL bug fix, full cache mode size needn't using maxSize * coreCount*/
        if (kernel_stream_full_cache_size != VX_NULL)
            *kernel_stream_full_cache_size += kernelStreamSize;

        if (maxKernelStreamSizePerCore < kernelStreamSize)
            maxKernelStreamSizePerCore = kernelStreamSize;

        prev = 0;
    }
    vxmASSERT(reorderStreamAllCount == reorderStreamCheckCount); /*Check if the data count is the same*/
    if (kernel_align_stream_size != VX_NULL)
    {
                /*bug1980*/
        if(gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_IMAGE_NOT_PACKED_IN_SRAM_FIX))
            *kernel_align_stream_size += (vx_size)(maxKernelStreamSizePerCore * usedCoreCount );
        else
            *kernel_align_stream_size += (vx_size)(maxKernelStreamSizePerCore * nnCoreCount);
    }

    if (!hasKernelFullCacheInterleaveFix && kernel_stream_full_cache_size != VX_NULL)
    {
        /*if did't fix full cache interleave fix, full cache size also need use align size*/
        *kernel_stream_full_cache_size = (vx_size)(maxKernelStreamSizePerCore * usedCoreCount);
    }

    if (kernel_max_stream_size_percore != VX_NULL)
        *kernel_max_stream_size_percore = maxKernelStreamSizePerCore;

exit:
    if (reorderStream)
        vxFree(reorderStream);
    if (reorderStreamPerCoreCount)
        vxFree(reorderStreamPerCoreCount);
    if (nonCoefIndex)
        vxFree(nonCoefIndex);
    if (limitZRLIndex)
        vxFree(limitZRLIndex);

    return (vx_uint32)((vx_uint8 *)kernelBufferPtr - wb_base_ptr);
}

vx_uint32 fillinKernelBufferV8Huffman(
    vx_context context,
    vx_weight_bias_huffman_cfg huffman_config,
    vx_enum  input_format,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_enum  weight_quant_format,
    vx_int32 *weight_zp,
    vx_enum  bias_format,
    vx_int32 input_zp,
    vx_uint32 skip_value,
    vx_int32 z_offset,
    vx_bool  is_depth_wise,
    vx_uint8_ptr wb_base_ptr,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_uint32* post_mul_shift,
    vx_uint32* neg_post_mul_shift,
    vx_size* kernel_align_stream_size,
    vx_size* kernel_stream_full_cache_size,
    vx_size* kernel_max_stream_size_percore
    )
{
     vx_uint32 nnCoreCount = (weight_format == VX_TYPE_INT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : (weight_format == VX_TYPE_FLOAT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountFloat16 : (weight_format == VX_TYPE_BFLOAT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountBFloat16 :context->nnConfig.fixedFeature.nnCoreCount;

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

    vx_uint32 bit16_flag = (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8) ? 0 : 1;
    vx_uint32 bit_offset = 0;

    vx_uint32 reorderStreamSize = 0;
    vx_uint32 reorderStreamAllCount = 0;
    vx_uint32 reorderStreamCheckCount = 0;
    vx_uint32* reorderStreamPerCoreCount = VX_NULL;
    vx_uint32 filterIndex = 0;

    vx_uint8* reorderStream = VX_NULL;
    vx_uint32 runSet2Bits = 1;
    CodeSymbol codeSymbol[THROUGHPUT * 3];

    vx_bool hasNoBias = ((weight_format == VX_TYPE_BFLOAT16) || (weight_format == VX_TYPE_FLOAT16)) ? vx_true_e : vx_false_e;
    vx_bool hasNoZOffset = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_NO_Z_LOCATION_OFFSET);
    vx_bool hasNNPerFilterPostMultiply = (post_mul_shift !=  VX_NULL) ? vx_true_e : vx_false_e;/*(gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PER_CHANNEL_QUANT) &&
                                            ((weight_quant_format == VX_QUANT_AFFINE_SCALE_PER_CHANNEL
                                            && (weight_format == VX_TYPE_UINT8 || weight_format == VX_TYPE_INT8)) ||
                                            (weight_quant_format == VX_QUANT_AFFINE_SCALE && weight_format == VX_TYPE_UINT8)) &&
                                            input_format == VX_TYPE_UINT8);
                                            */
    //vx_bool hasNNPreLU = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PRELU);
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

    vx_weight_bias_huffman_cfg huffmanConfig = huffman_config;

    typedef struct _gcVXWeightsMinusZP
    {
        vx_int32 sum;
        vx_uint32 filterIdx;
    }gcVXWeightsMinusZP;
    gcVXWeightsMinusZP *weightsMinusZP = VX_NULL;

    vxmASSERT(huffmanConfig != VX_NULL);

    if (weight_format == VX_TYPE_INT16 || weight_format == VX_TYPE_UINT8 || (weight_format == VX_TYPE_INT8 && (post_mul_shift)))
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
        if ((input_zp != 0) &&
            ((vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT) &&
            weight_quant_format == VX_QUANT_AFFINE_SCALE &&
            weight_format == VX_TYPE_UINT8) ||
            (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PER_CHANNEL_QUANT) &&
            weight_quant_format == VX_QUANT_AFFINE_SCALE_PER_CHANNEL
            && weight_format == VX_TYPE_INT8)))
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
            vx_int32    weightZp = 0;
            if(gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PER_CHANNEL_QUANT) &&
                weight_quant_format == VX_QUANT_AFFINE_SCALE_PER_CHANNEL &&
                (weight_format == VX_TYPE_UINT8 || weight_format == VX_TYPE_INT8))
            {
                weightZp  = weight_zp[filterIndex];
            }
            else if(vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT) &&
                    weight_quant_format == VX_QUANT_AFFINE_SCALE &&
                    weight_format == VX_TYPE_UINT8)
            {
                weightZp = weight_zp[0];
            }
            else
            {
                weightZp = weight_zp[0];
                vxmASSERT(weightZp == 0);
            }

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
                        if (input_format == VX_TYPE_UINT8 || input_format == weight_format) /*VX_TYPE_INVALID :some older case never sent inputFormat*/
                        {
                            if(weight_format == VX_TYPE_UINT8)
                            {
                                weight = *((vx_uint8 *)kernelDataPtr);
                                coefSum[filterIndex] += (weight - weightZp);
                            }
                            else if(weight_format == VX_TYPE_INT8 && (post_mul_shift))/*only support per-channel quantization*/
                            {
                                weight = *((vx_int8 *)kernelDataPtr);
                                coefSum[filterIndex] += (weight - weightZp);
                            }
                            else if(weight_format == VX_TYPE_INT16)
                            {
                                weight = *((vx_int16 *)kernelDataPtr);
                                coefSum[filterIndex] += weight;
                            }
                            kernelDataPtr = kernelDataPtr + weightSize;
                        }

                        if (weightsMinusZP != VX_NULL)
                        {
                            /*Calc sum((coef[i] - coefZP) * InZP) */
                            weightsMinusZP[filterIndex].sum += (weight - weightZp) * input_zp;
                        }
                    }
                }
            }
        }
    }

    if (weight_format == VX_TYPE_BFLOAT16 ||
        weight_format == VX_TYPE_FLOAT16)
    {
        numOfInImageBuffer = (vx_uint32)gcoMATH_Ceiling((vx_float32)(weightCount * slice_count + BF16_BIAS_SIZE) / (vx_float32)linesInImageBuffer);
    }
    reorderStreamAllCount = (numOfInImageBuffer * linesInImageBuffer * filterTotalCount)
        + (usedCoreCount * (16 / weightBitSize));/*filterPerCore need 16 bit * nnCoreCount */

    reorderStreamSize = reorderStreamAllCount * weightSize;

    if (hasNNPerFilterPostMultiply && gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PER_CHANNEL_QUANT_ASYM) && weight_format == VX_TYPE_UINT8)
    {
        reorderStreamSize += filterTotalCount; /*Per channel 8 bit zero point*/
        reorderStreamAllCount += filterTotalCount;
    }

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
    if (weight_format == VX_TYPE_BFLOAT16 ||
        weight_format == VX_TYPE_FLOAT16)
        nonCoefCount += (filterTotalCount * BF16_BIAS_SIZE); /*BFLOAT16 fp32 bias will be splitted to 3 BF16 to coef stream*/

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

    if (is_depth_wise)
        reorderDepthWiseKernelBufferV8Huffman(context, weight_x, weight_y, slice_count, z_count, filters_per_core, weight_zp, weight_format, reorderStream, weight_base_ptr, bias_base_ptr, nnCoreCount, reorderStreamPerCoreCount, nonCoefIndex, limitZRLIndex, hasNNPerFilterPostMultiply);

    else if (context->options.enableNonZeroBalance && !hasNoZOffset)
    {
        /*Only those chip has z_offset support zero balance*/
        realVzIndex = (vx_uint32 *)vxAllocateAndZeroMemory(z_count * sizeof(vx_uint32));
        if (!realVzIndex)
        {
            vxError("fillinKernelBufferV8Huffman: OUT OF MEMORY");
            goto exit;
        }
        reorderKernelBufferV8HuffmanBalance(context, weight_x, weight_y, slice_count, z_count, filters_per_core, (vx_uint32 *)weight_zp, weight_format, reorderStream, weight_base_ptr, nnCoreCount, reorderStreamPerCoreCount, nonCoefIndex, realVzIndex, limitZRLIndex, hasNNPerFilterPostMultiply);

    }
    else
        reorderKernelBufferV8Huffman(context, weight_x, weight_y, slice_count, z_count, filters_per_core, weight_zp, weight_format, reorderStream, weight_base_ptr, bias_base_ptr, nnCoreCount, reorderStreamPerCoreCount, nonCoefIndex, limitZRLIndex, hasNNPerFilterPostMultiply);


    analysisKernelStreamForHuffman(context, huffman_config, weight_format, 0, nnCoreCount, reorderStream, reorderStreamSize, reorderStreamPerCoreCount, invSizeOrder, nonCoefIndex, nonCoefCount, limitZRLIndex, filterTotalCount);

    /* Write huffman header*/
    writeBits(&kernelBufferPtr, &bit_offset, huffmanConfig->pre_encode, 1);
    writeBits(&kernelBufferPtr, &bit_offset, huffmanConfig->bit16_flag, 1);
    writeBits(&kernelBufferPtr, &bit_offset, huffmanConfig->fp16_flag, 1);
    writeBits(&kernelBufferPtr, &bit_offset, huffmanConfig->reserved, 1);
    writeBits(&kernelBufferPtr, &bit_offset, huffmanConfig->version, 4);
    writeBits(&kernelBufferPtr, &bit_offset, huffmanConfig->run_len_table_size, 8);

    for(i = 0; i < MAX_RUNLEN_SIZE; i++)
    {
        writeBits(&kernelBufferPtr, &bit_offset, huffmanConfig->run_len_table[i], 8);
    }
    for(i = 0; i < LOG_RUN_SIZE1; i++)
    {
        writeBits(&kernelBufferPtr, &bit_offset, huffmanConfig->map_to_huffman[i], 8);
    }
    writeBits(&kernelBufferPtr, &bit_offset, huffmanConfig->avg_bias, 16);
    writeBits(&kernelBufferPtr, &bit_offset, 0, 16); /*reserved, must zero*/

    kernelStreamSizePtr = kernelBufferPtr;
    streamSizeBitOffset = bit_offset;
    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        writeBits(&kernelBufferPtr, &bit_offset, kernelStreamSize, 32);
    }
    /*align to 64 byte*/
    packZeros(&kernelBufferPtr, &bit_offset, alignedOffset);
    if (kernel_stream_full_cache_size != VX_NULL)
        *kernel_stream_full_cache_size = 0;

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

            if (bit16_flag == 0)
            {

                coef = (coef - huffmanConfig->avg_bias) & 0xFF;
                if (huffmanConfig->pre_encode)
                {
                    tmp = coef;
                    coef = (coef - prev) & 0xFF;
                    prev = tmp;
                }
            }
            else
            {

                if (huffmanConfig->fp16_flag)
                {
                    coef16 = (coef16 & 0x7FFF) * 2 + ((coef16 & (1<<15)) >> 15);
                }
                coef16 -= huffmanConfig->avg_bias;
                if (huffmanConfig->run_len_table_size == 0x0)
                    coef = (coef16 >> 8) & 0xFF; /*Non RZL*/
                else
                    coef = coef16 & 0xFFFF; /*RZL path*/
            }

            if (huffmanConfig->run_len_table_size == 0x0) /* non RZL*/
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

                    if (huffmanConfig->bit16_flag)
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

                if (huffmanConfig->bit16_flag)
                {/*pack the raw data of the lower 8 bits*/

                    codeSymbol[pos].stageCode[2] = codeSymbol[x%(3*THROUGHPUT)].stageCode[2]* 256 + (coef16 &0xff);
                    codeSymbol[pos].bitLength[2] += 8;
                }
                OutputAt(x, &kernelBufferPtr, &bit_offset, codeSymbol);
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
                    if (huffmanConfig->bit16_flag)
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
                                vx_bool finded = vx_false_e;
                                vx_int32 q = 0;
                                for (q = 0; q < SET_1_SIZE; q++)
                                {
                                    if (best2Runs[q] == bestRunsSorted[j])
                                    {
                                        finded = vx_true_e;
                                    }
                                }

                                for (q = 0; q < numBestRuns - SET_1_SIZE; q++)
                                {
                                    if (outBest2Run[q] == bestRunsSorted[j])
                                    {
                                        finded = vx_true_e;
                                    }
                                }

                                if (finded)
                                {
                                    k = run / (bestRunsSorted[j] + 1);
                                    run %= (bestRunsSorted[j] + 1);
                                    break;
                                }
                            }
                        }
                        if (j >= 0)
                        {
                            vx_int32 n = bestRunsSorted[j];
                            vx_bool finded = vx_false_e;
                            size = 8;
                            for (m = 0; m<SET_1_SIZE; m++)
                            {
                                if (best2Runs[m] == n)
                                {
                                    size = 0;
                                    finded = vx_true_e;
                                    break;
                                }
                            }
                            if (size == 8)
                            {
                                for (m = 0; m<numBestRuns - SET_1_SIZE; m++)
                                {
                                    if (outBest2Run[m] == n)
                                    {
                                        finded = vx_true_e;
                                        break;
                                    }
                                }

                            }

                            if(!finded)
                            {
                                vxInfo("\n bestRunsSorted mismatch \n");
                                vxmASSERT(finded);
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

                                OutputAt(x, &kernelBufferPtr, &bit_offset, codeSymbol);
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
                        coef>>=(huffmanConfig->bit16_flag * 8);
                        if(coef<0x80)
                            coef+=huffmanConfig->bit16_flag; /*Since we have to encode zero (run-length only handle 16-bits zero, 8 bit zero should use Size code*/
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
                            if(huffmanConfig->bit16_flag)
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
                        if (huffmanConfig->bit16_flag)
                        {
                            codeSymbol[pos].stageCode[2] = codeSymbol[x%(3*THROUGHPUT)].stageCode[2]* 256 + (coef16 & 0xff);
                            codeSymbol[pos].bitLength[2] += 8;
                        }
                        OutputAt(x, &kernelBufferPtr, &bit_offset, codeSymbol);
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
                        if (huffmanConfig->bit16_flag)
                        {
                            codeSymbol[pos].stageCode[2] = codeSymbol[x%(3*THROUGHPUT)].stageCode[2]* 256 + (coef16 & 0xff);
                            codeSymbol[pos].bitLength[2] += 8;
                        }
                        OutputAt(x, &kernelBufferPtr, &bit_offset, codeSymbol);
                        x++; /*counter*/
                    }
                }
            }

            if(dataRemainingOfCore == 0)
            {
                vx_uint32 dummyCount = (x & 0x1)? 5: 4;
                /*Hit the end bitsteam of core, add dummy stage process the next*/
                vxmASSERT(setDummy == vx_true_e);
                addDummy(x, &kernelBufferPtr, &bit_offset, codeSymbol, dummyCount, dummyStage, dummyBitLength);
                break;
            }
        }

        /* Go back to update kernelStreamSize. */
        kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);
        writeBits(&kernelStreamSizePtr, &streamSizeBitOffset, kernelStreamSize*8 + bit_offset, 32);

        /* pad 0 at the end of core*/
        packZeros(&kernelBufferPtr, &bit_offset, alignedOffset);
        kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);
        /*With RTL bug fix, full cache mode size needn't using maxSize * coreCount*/
        if (kernel_stream_full_cache_size != VX_NULL)
            *kernel_stream_full_cache_size += kernelStreamSize;

        if (maxKernelStreamSizePerCore < kernelStreamSize)
            maxKernelStreamSizePerCore = kernelStreamSize;

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
                !is_depth_wise &&
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
            if (weightsMinusZP != VX_NULL)
            {
                biasData -= weightsMinusZP[realFilterIndex].sum;
            }

            if (input_format == VX_TYPE_UINT8 || input_format == weight_format)/*VX_TYPE_INVALID :some older case never sent inputFormat*/
            {
                if (weight_format == VX_TYPE_UINT8 || (weight_format == VX_TYPE_INT8 && (post_mul_shift != VX_NULL)))
                {
                    biasData += coefSum[realFilterIndex] * 128;
                    writeBits(&kernelBufferPtr, &bit_offset, biasData, biasBitSize);
                }
                else if (weight_format == VX_TYPE_INT16 )
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
                    writeBits(&kernelBufferPtr, &bit_offset, bias32, 32);
                    bias16 = (vx_int16)(bias64 >> 32);
                    writeBits(&kernelBufferPtr, &bit_offset, (vx_int32)bias16, 16);
                }
                else
                {
                    writeBits(&kernelBufferPtr, &bit_offset, biasData, biasBitSize);
                }
            }
            else
            {
                writeBits(&kernelBufferPtr, &bit_offset, biasData, biasBitSize);
            }
        }

        if (post_mul_shift != VX_NULL)
        {
            if(gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_FLOAT_POST_MULT))
            {
                vx_int32 postMul = 0;
                postMul = *(post_mul_shift + filterIndex);
                writeBits(&kernelBufferPtr, &bit_offset, postMul, 32);
            }
            else
            {
                vx_uint32 postMul = 0;
                vx_uint32 postShift = 0;
                vx_uint32 postMulShift = *(post_mul_shift  + filterIndex);

                postMul = postMulShift & 0x7FFFFF;/*23 bits postMultiply*/
                postShift = (postMulShift >> 23) & 0x7F;/*7 bits postShift*/

                writeBits(&kernelBufferPtr, &bit_offset, postMul, 23);
                writeBits(&kernelBufferPtr, &bit_offset, postShift, 7);
                /*unused zero for 10 bit*/
                writeBits(&kernelBufferPtr, &bit_offset, 0, 2);
            }
        }

        if (!hasNoZOffset)
        {
            vx_uint32 offsetValue;
            vx_uint32 realFilterIndex;

            if (context->options.enableNonZeroBalance &&
                !is_depth_wise &&
                realVzIndex != VX_NULL)
                realFilterIndex = realVzIndex[filterIndex];
            else
                realFilterIndex = filterIndex;

            offsetValue = (vx_uint32)z_offset * realFilterIndex;

            writeBits(&kernelBufferPtr, &bit_offset, offsetValue, NN_Z_POSITION_OFFSET_BITS_VIP_V7);
        }
        else if (neg_post_mul_shift)
        {
            if(gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_FLOAT_POST_MULT))
            {
                vx_int32 negPostMul = 0;
                negPostMul = *(neg_post_mul_shift + filterIndex);
                writeBits(&kernelBufferPtr, &bit_offset, negPostMul, 32);
            }
            else
            {
                /*per PRD, prelu only enable when hasNoZOffset == 1 && hasNNPerFilterPostMultiply == 1*/
                vx_int32 negPostMul = 0;
                vx_int32 negPostShift = 0;
                vx_uint8 negPostSign = 0;
                vx_uint32 negPostMulShift = *(neg_post_mul_shift  + filterIndex);

                negPostMul = negPostMulShift & 0x7FFFFF;/*23 bits postMultiply*/
                negPostShift = (negPostMulShift >> 23) & 0x7F;/*7 bits postShift*/
                negPostSign = (negPostMulShift >> 31) & 0x1; /*1 bit sign*/


                writeBits(&kernelBufferPtr, &bit_offset, negPostMul, 23);
                writeBits(&kernelBufferPtr, &bit_offset, negPostShift, 7);

                /*unused zero for 1 bit*/
                writeBits(&kernelBufferPtr, &bit_offset, 0, 1);
                /*PRD put the negSign at the last bit*/
                writeBits(&kernelBufferPtr, &bit_offset, negPostSign, 1);
            }
        }
    }
    /*Also align to 64 byte for post processing stream*/
    packZeros(&kernelBufferPtr, &bit_offset, alignedOffset);
    kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);
    if (kernel_stream_full_cache_size != VX_NULL)
        *kernel_stream_full_cache_size += kernelStreamSize;

    if (maxKernelStreamSizePerCore < kernelStreamSize)
        maxKernelStreamSizePerCore = kernelStreamSize;
    /*Per HW, post processing stream size is needed in SRAM, when partial mode, need be noted as one core*/
    if (kernel_align_stream_size != VX_NULL)
    {
        /*bug1980*/
        if(gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_IMAGE_NOT_PACKED_IN_SRAM_FIX))
            *kernel_align_stream_size += (vx_size)(maxKernelStreamSizePerCore * (usedCoreCount + 1) );
        else
            *kernel_align_stream_size += (vx_size)(maxKernelStreamSizePerCore * (nnCoreCount + 1));

    }

    if (!hasKernelFullCacheInterleaveFix && kernel_stream_full_cache_size != VX_NULL)
    {
        /*if did't fix full cache interleave fix, full cache size also need use align size*/
        *kernel_stream_full_cache_size = (vx_size)(maxKernelStreamSizePerCore * (usedCoreCount + 1));
    }

    if (kernel_max_stream_size_percore != VX_NULL)
        *kernel_max_stream_size_percore = maxKernelStreamSizePerCore;

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

    return (vx_uint32)((vx_uint8 *)kernelBufferPtr - wb_base_ptr);
}

vx_status replaceKernelBufferZOffset(
    vx_weight_bias_z_offset z_offset_handle,
    vx_uint32 num_of_vz,
    vx_uint8_ptr wb_base_ptr,
    vx_int32 z_offset
    )
{
    vx_uint32 filterIndex;
    vx_uint32* kernelBufferPtr = VX_NULL;

    if (z_offset_handle == VX_NULL)
    {
        vxError("replaceKernelBufferZOffset: No offset");
        return VX_ERROR_INVALID_PARAMETERS;
    }

    vxmASSERT(z_offset > 0);

    for (filterIndex = 0; filterIndex < num_of_vz; filterIndex++)
    {
        vx_uint32 offsetValue = z_offset * filterIndex;
        vx_uint32 bit_offset = z_offset_handle[filterIndex].bit_offset;
        kernelBufferPtr = (vx_uint32*)(wb_base_ptr + z_offset_handle[filterIndex].ptr_offset);

        if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
            replaceBits(&kernelBufferPtr, &bit_offset, offsetValue, NN_Z_POSITION_OFFSET_BITS_VIP_V7);
        else
            replaceBits(&kernelBufferPtr, &bit_offset, offsetValue, NN_Z_POSITION_OFFSET_BITS);
    }

    return VX_SUCCESS;
}

vx_status calculateZOffset(
    vx_uint32* pooling_output_dims,
    vx_enum    output_format,
    vx_enum    weight_format,
    vx_uint32* z_offset
    )
{
    vx_uint32 zOffset;

    vxmASSERT(z_offset != VX_NULL);

    zOffset = *z_offset;

    if (zOffset == 0)
    {
        vx_uint32 outputSize = vxDataType_GetSize((vx_type_e)output_format);
        vxmASSERT(pooling_output_dims != VX_NULL);

        if (outputSize > 0)
        {
            zOffset = pooling_output_dims[0] * pooling_output_dims[1] * outputSize;
        }
        else
        {
            vx_uint32 weightSize = vxDataType_GetSize((vx_type_e)weight_format);
            zOffset = pooling_output_dims[0] * pooling_output_dims[1] * weightSize;
        }
    }

    *z_offset = zOffset;

    return VX_SUCCESS;
}

vx_tensor reshuffleKernelTensor(
    vx_context   context,
    vx_tensor    weight,
    vx_uint32*   output_dims,
    vx_uint32    stride_x,
    vx_uint32    stride_y
    )
{
    if (!vxoReference_IsValidAndSpecific(&weight->base, VX_TYPE_TENSOR))
    {
        return VX_NULL;
    }

    if ((TENSOR_VIEW_SIZE_INDEX(weight, 0) == 1 && TENSOR_VIEW_SIZE_INDEX(weight, 1) == 1) ||
        (stride_x == 1 && stride_y == 1))
    {
        return weight;
    }
    else
    {
        vx_nn_reshuffle_s src = {
            NULL,
            vxnneAlignWithStride(TENSOR_VIEW_SIZE_INDEX(weight, 0), stride_x),
            vxnneAlignWithStride(TENSOR_VIEW_SIZE_INDEX(weight, 1), stride_y),
            TENSOR_VIEW_SIZE_INDEX(weight, 2),
            TENSOR_VIEW_SIZE_INDEX(weight, 3),
            (vx_type_e)TENSOR_DATA_TYPE(weight)
        };
        vx_nn_reshuffle_s dst = {
            NULL,
            output_dims[0],
            output_dims[1],
            output_dims[2],
            output_dims[3],
            (vx_type_e)TENSOR_DATA_TYPE(weight)
        };

        vx_uint32 x, y, z, w;
        vx_uint32 orgXSize = TENSOR_VIEW_SIZE_INDEX(weight, 0);
        vx_uint32 orgYSize = TENSOR_VIEW_SIZE_INDEX(weight, 1);
        vx_uint32 orgZSize = TENSOR_VIEW_SIZE_INDEX(weight, 2);
        vx_uint32 weightItemCount = output_dims[0] * output_dims[1] * output_dims[2] * output_dims[3];
        vx_uint32 weightSize = TENSOR_DATA_SIZE(weight);

        vx_uint8_ptr startWeightDataPtr = VX_NULL, reshuffledWeightDataPtr = VX_NULL;
        gctPOINTER convertedWeightData  = VX_NULL;

        vx_tensor reshuffledTensor = VX_NULL;
        vx_tensor_create_params_t params;

        params.num_of_dims = TENSOR_DIM_NUM(weight);
        params.sizes = output_dims;
        params.data_format = TENSOR_DATA_TYPE(weight);
        params.quant_format = TENSOR_QUANT_TYPE(weight);
        if (TENSOR_QUANT_TYPE(weight) == VX_QUANT_DYNAMIC_FIXED_POINT)
        {
            params.quant_data.dfp.fixed_point_pos = TENSOR_POS(weight);
        }
        else if(TENSOR_QUANT_TYPE(weight) == VX_QUANT_AFFINE_SCALE_PER_CHANNEL)
        {
            vx_uint32 channelCount = output_dims[3];

            params.quant_data.affinePerChannel.channelDim = TENSOR_TF_CHANNEL_DIMS(weight);
            params.quant_data.affinePerChannel.scaleCount = output_dims[3];
            params.quant_data.affinePerChannel.scales = (vx_float32 *)vxAllocateAndZeroMemory(channelCount * sizeof(vx_float32));
            params.quant_data.affinePerChannel.zeroPoint = (vx_int32 *)vxAllocateAndZeroMemory(channelCount * sizeof(vx_int32));

            gcoOS_MemCopy(params.quant_data.affinePerChannel.scales, TENSOR_TF_SCALE_POINTER(weight), channelCount * sizeof(vx_float32));
            gcoOS_MemCopy(params.quant_data.affinePerChannel.zeroPoint, TENSOR_TF_ZEROPOINT_POINTER(weight), channelCount * sizeof(vx_float32));

        }
        else
        {
            params.quant_data.affine.scale = TENSOR_TF_SCALE(weight);
            params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(weight);
        }

        reshuffledTensor = vxoTensor_CreateTensor2(context, &params, sizeof(vx_tensor_create_params_t));
        if (reshuffledTensor == VX_NULL)
        {
            return VX_NULL;
        }
        if (vxoTensor_AllocateMemory(reshuffledTensor) != VX_SUCCESS)
        {
            return VX_NULL;
        }

        convertedWeightData = vxAllocateAndZeroMemory(weightItemCount * weightSize);
        if (convertedWeightData == VX_NULL)
        {
            return VX_NULL;
        }

        vxoTensor_GetTensorViewMemory(weight, (gctPOINTER*)(&startWeightDataPtr), VX_NULL);
        vxoTensor_GetTensorViewMemory(reshuffledTensor, (gctPOINTER*)(&reshuffledWeightDataPtr), VX_NULL);

        for (w = 0; w < src.wSize; w++)
        {
            for (z = 0; z < src.zSize; z++)
            {
                /* convert float32 to float16 and fill edge with 0 */
                for (y = 0; y < src.ySize; y++)
                {
                    for (x = 0; x < src.xSize; x++)
                    {
                        vx_uint32 orgOffsetSize = (w * orgZSize * orgYSize * orgXSize + z * orgYSize * orgXSize + y * orgXSize + x) * weightSize;
                        vx_uint32 fpOffsetSize = (w * src.zSize * src.ySize * src.xSize + z * src.ySize * src.xSize + y * src.xSize + x) * weightSize;
                        vx_uint32 zero = (TENSOR_DATA_TYPE(weight) == VX_TYPE_UINT8 && TENSOR_QUANT_TYPE(weight) == VX_QUANT_AFFINE_SCALE) ? TENSOR_TF_ZEROPOINT(weight) : 0;
                        vx_uint8 *converted, *orig;

                        converted = (vx_uint8*)convertedWeightData + fpOffsetSize;

                        if ((x > orgXSize - 1) || (y > orgYSize - 1))
                        {
                            _DataGeneralConvert((void*)&zero, (void*)converted, weightSize, weightSize);
                        }
                        else
                        {
                            orig = startWeightDataPtr + orgOffsetSize;
                            _DataGeneralConvert((void*)orig, (void*)converted, weightSize, weightSize);
                        }
                    }
                }
            }
        }

        /*reshuffle kernel data*/
        src.data   = convertedWeightData;
        dst.data   = reshuffledWeightDataPtr;
        reshuffleData(&src, stride_x, stride_y, &dst);

        if (convertedWeightData != VX_NULL)
        {
            vxFree(convertedWeightData);
        }
        if(TENSOR_QUANT_TYPE(weight) == VX_QUANT_AFFINE_SCALE_PER_CHANNEL && params.quant_data.affinePerChannel.scales != VX_NULL)
        {
            vxFree(params.quant_data.affinePerChannel.scales);
            params.quant_data.affinePerChannel.scales = VX_NULL;
        }
        if(TENSOR_QUANT_TYPE(weight) == VX_QUANT_AFFINE_SCALE_PER_CHANNEL && params.quant_data.affinePerChannel.zeroPoint != VX_NULL)
        {
            vxFree(params.quant_data.affinePerChannel.zeroPoint);
            params.quant_data.affinePerChannel.zeroPoint= VX_NULL;
        }

        return reshuffledTensor;
    }
}

vx_bool isInt64BiasOverflow(
    vx_context context,
    vx_enum weight_format,
    vx_enum bias_format,
    vx_uint32 z_count,
    vx_tensor bias_tensor
    )
{
    vx_bool status = vx_false_e;

    if (weight_format == VX_TYPE_INT16 && bias_format == VX_TYPE_INT64 &&
        !vxoContext_IsFeatureAvailable(context, VX_TP_FEATURE_FP32_BIAS))
    {
        vx_uint32 filterIndex = 0, filterCount = z_count;
        vx_int64 *bias_base_ptr = VX_NULL;

        if (bias_tensor != VX_NULL)
        {
            vxoTensor_GetTensorViewMemory(bias_tensor, (gctPOINTER*)(&bias_base_ptr), VX_NULL);
        }

        for (filterIndex = 0; filterIndex < filterCount; filterIndex++)
        {
            vx_int64  biasData64bits         = 0;
            biasData64bits = bias_base_ptr == VX_NULL ? 0 : *(bias_base_ptr + filterIndex);
            if((((biasData64bits >>31) & 0x1FFFFFFFF) != 0) && (((biasData64bits >>31) & 0x1FFFFFFFF) != 0x1FFFFFFFF))
            {
                vxInfo("\n Real Int64 bias -> NN \n ");
                return vx_true_e;
            }
        }
        vxInfo("\n Fake Int64 bias -> TP \n ");
    }

    return status;
}

