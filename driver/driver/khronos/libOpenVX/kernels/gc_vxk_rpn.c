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
#include "anchors.h"

extern vx_int16 F32toF16(vx_float32 val);
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

#define MIN_BOX_SIZE 16
#define SCALE_BACK_RATIO    1.0f /* 500x375 -> 800x600 */
#define CLIP_WIDTH          800
#define CLIP_HEIGHT         600

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#define INPUT_F16 0
#define RPN_NMS 1

#if INPUT_F16
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

int compare(const void * a, const void * b)
{
    float f0 = *(float*)a;
    float f1 = *(float*)b;
    float df = f1 - f0;
    int result = (df == 0.0f)? 0: 1;

    int s = (((*(int*)&df) >>31) <<1) + 1;

    result *= s;

    return result;
}

#define SAVE_ORIG_ROI
#define TOP_N            6000
#define OVERLAP_THRES    0.7f

static void nms(vx_float32 *pProbBox, vx_uint32 *pPicked, vx_float32 fOverlap)
{
    vx_float32 fProb;
    vx_float32 area[TOP_N], w, h, xx0, yy0, xx1, yy1, inter, iou;


    vx_uint32 i, r, p=0;
    vx_uint32 toPick, picked;

    r = *pPicked;


    for (i=0; i<r; i++)
    {
        w = *(pProbBox +i*5 + 3) - *(pProbBox + i*5 + 1) + 1.0f;
        h = *(pProbBox + i*5 + 4) - *(pProbBox + i*5 + 2) + 1.0f;
        area[i] = w * h;
    }

    picked = 0;
    toPick = r;
    p = 0;

    while (toPick > 1 && p < r)
    {
        /* Pick p. And remove overlapped with p*/
        for (i= p+1; i<r; i++)
        {
            fProb = *(pProbBox + i*5);
            if (fProb <= 0.0)
                continue;

            xx0 = max(*(pProbBox + i*5 + 1), *(pProbBox + p*5 + 1));
            yy0 = max(*(pProbBox + i*5 + 2), *(pProbBox + p*5 + 2));
            xx1 = min(*(pProbBox + i*5 + 3), *(pProbBox + p*5 + 3));
            yy1 = min(*(pProbBox + i*5 + 4), *(pProbBox + p*5 + 4));

            w = max(0.0f, xx1-xx0+1.0f);
            h = max(0.0f, yy1-yy0+1.0f);

            inter = w*h;
            iou = inter /(area[i]+area[p] - inter);

            if (iou > fOverlap)
            {
                *(pProbBox + i*5) = -1.0f; /* remove */
                toPick--;
            }
        }

        picked++;
        toPick--;

        do {
            p++;
        }
        while (*(pProbBox + p*5) <= 0.0 && p < r);
    }

    *pPicked = picked;
}

vx_status vxRPN(vx_node node, vx_array src, vx_array dst0, vx_array dst1)
{
    vx_status status = VX_SUCCESS;


    vx_float32 *pSrcStage, *pScore, *pDeltaBox, *pProbBox, *pBBox;
    vx_uint32  x, y, z, ii, jj;
    vx_uint32 RdOffset = 0;
    vx_uint32 WtOffset = 0;
    vx_uint32 realNumROI = TOP_N;

    vx_float32 f0, f1, fmax, ff;


    /* (0) Copy to CPU cached memory.
        NEED REUSE STAGING BUFFER allocated in verifyGraph*/
#if defined(__linux__)
    struct timeval start = gcfVX_PerfStart((vx_reference)node);
    struct timeval start1 = gcfVX_PerfStart((vx_reference)node);
#endif

#if INPUT_F16
     {
         vx_uint32 i = 0;
         vx_int16*p = (vx_int16*)src->memory.logicals[0];
         for (i = 0; i < src_width * src_height * src_depth; i ++)
         {
             pSrcStage[i] = fp16_to_float32(p[i]);
         }
     }
#else
     pSrcStage = (vx_float32*)src->memory.logicals[0];
#endif

    /* (1) FP16-> FP32 */

     /* (2) Reshape score 51 x 39 x 18 -> 17901 x 2 */
     pScore = pSrcStage;
     pDeltaBox = pSrcStage + 17901*2;

    /* (3) Softmax */
    for (y=0; y < 17901; y++)
    {
        ff = 0.0f;

        f0 = pScore[y];
        f1 = pScore[y + 17901];
        fmax = (f0 > f1)? f0:f1;
        f0 -=fmax;
        f1 -=fmax;

        f0 = expf(f0);
        f1 = expf(f1);
        ff = f0 + f1;

        pScore[y] = f0/ff;
        pScore[y + 17901] = f1/ff;
    }

#if defined(__linux__)
    if (node->base.context->perfEnable)
        printf("fast rcnn rpn softmax CPU  time:%10d us\n", gcfVX_PerfEnd((vx_reference)node, start1));

    start1 = gcfVX_PerfStart((vx_reference)node);
#endif

    /* (4) reshape score probability. Background is the first. Object is the second. Use the second pScore + 17901.*/
    pProbBox = pDeltaBox + 51*39*36;
    //pProbBox = (vx_float32*)malloc(17901 * 5 * sizeof(vx_float32));

    for (x=0; x<51; x++)
    {
        for (y=0; y<39; y++)
        {
            for (z=0; z<9; z++)
            {
                RdOffset = z* 51 * 39 + y*51 +x + 17901;
                *(pProbBox + WtOffset) = *(pScore + RdOffset);
                WtOffset += 5;
            }
        }
    }

    /* (5) BBox reshape  Reshaped Prob BBox merged in pProbBox */
    WtOffset = 0;

    for (x=0; x<51; x++)
    {
        for (y=0; y<39; y++)
        {
            for (z=0; z<36; z++)
            {
                if (z%4 ==0)
                {
                    WtOffset++;
                }
                RdOffset = z* 51 * 39 + y*51 +x;
                *(pProbBox + WtOffset) = *(pDeltaBox + RdOffset);
                WtOffset++;
            }
        }
    }

#if defined(__linux__)
    if (node->base.context->perfEnable)
        printf("fast rcnn rpn reshape CPU  time:%10d us\n", gcfVX_PerfEnd((vx_reference)node, start1));

    start1 = gcfVX_PerfStart((vx_reference)node);
#endif

    /* (6)  BBox transform */
    for (ii=0; ii< 17901; ii++) {
        // src_w = double(boxes(:,3) - boxes(:,1) + 1);
        // src_h = double(boxes(:,4) - boxes(:,2) + 1);
        // src_ctr_x = double(boxes(:,1) + 0.5*(src_w-1));
        // src_ctr_y = double(boxes(:,2) + 0.5*(src_h-1));
        float src_w = anchors[ii][2] - anchors[ii][0] + 1;
        float src_h = anchors[ii][3] - anchors[ii][1] + 1;
        float src_ctr_x = anchors[ii][0] + 0.5f*(src_w-1);
        float src_ctr_y = anchors[ii][1] + 0.5f*(src_h-1);

        // dst_ctr_x = double(box_deltas(:,1 : 4 : end));
        // dst_ctr_y = double(box_deltas(:,2 : 4 : end));
        // dst_scl_x = double(box_deltas(:,3 : 4 : end));
        // dst_scl_y = double(box_deltas(:,4 : 4 : end));
        float dst_ctr_x = *(pProbBox + ii * 5 + 1);
        float dst_ctr_y = *(pProbBox + ii * 5 + 2);
        float dst_scl_x = *(pProbBox + ii * 5 + 3);
        float dst_scl_y = *(pProbBox + ii * 5 + 4);

        // pred_ctr_x = bsxfun(@plus,bsxfun(@times,dst_ctr_x,src_w),src_ctr_x);
        // pred_ctr_y = bsxfun(@plus,bsxfun(@times,dst_ctr_y,src_h),src_ctr_y);
        float pred_ctr_x = (dst_ctr_x*src_w) + src_ctr_x;
        float pred_ctr_y = (dst_ctr_y*src_h) + src_ctr_y;

        // pred_w = bsxfun(@times,exp(dst_scl_x),src_w);
        // pred_h = bsxfun(@times,exp(dst_scl_y),src_h);
        float pred_w = expf(dst_scl_x) * src_w;
        float pred_h = expf(dst_scl_y) * src_h;

        // pred_boxes(:,1 : 4 : end) = pred_ctr_x - 0.5*(pred_w-1);
        // pred_boxes(:,2 : 4 : end) = pred_ctr_y - 0.5*(pred_h-1);
        // pred_boxes(:,3 : 4 : end) = pred_ctr_x + 0.5*(pred_w-1);
        // pred_boxes(:,4 : 4 : end) = pred_ctr_y + 0.5*(pred_h-1);
        *(pProbBox + ii * 5 + 1) = max(min((pred_ctr_x - 0.5f*(pred_w-1)) * SCALE_BACK_RATIO, CLIP_WIDTH), 1);
        *(pProbBox + ii * 5 + 2) = max(min((pred_ctr_y - 0.5f*(pred_h-1)) * SCALE_BACK_RATIO, CLIP_HEIGHT), 1);
        *(pProbBox + ii * 5 + 3) = max(min((pred_ctr_x + 0.5f*(pred_w-1)) * SCALE_BACK_RATIO, CLIP_WIDTH), 1);
        *(pProbBox + ii * 5 + 4) = max(min((pred_ctr_y + 0.5f*(pred_h-1)) * SCALE_BACK_RATIO, CLIP_HEIGHT), 1);
        }

#if defined(__linux__)
        if (node->base.context->perfEnable)
            printf("fast rcnn rpn transform CPU  time:%10d us\n", gcfVX_PerfEnd((vx_reference)node, start1));

        start1 = gcfVX_PerfStart((vx_reference)node);
#endif

    /* (7) filter out too small boxes */

    /* (8) sort */
    qsort ((void*)pProbBox, 17901, 5*sizeof(vx_float32), compare);

#if defined(__linux__)
    if (node->base.context->perfEnable)
        printf("fast rcnn rpn sort CPU  time:%10d us\n", gcfVX_PerfEnd((vx_reference)node, start1));

    start = gcfVX_PerfStart((vx_reference)node);
#endif

    /* (9) NMS */

#if RPN_NMS
    nms(pProbBox, &realNumROI, OVERLAP_THRES);
#endif

#if defined(__linux__)
    if (node->base.context->perfEnable)
        printf("fast rcnn rpn nms CPU  time:%10d us\n", gcfVX_PerfEnd((vx_reference)node, start1));
#endif

    realNumROI = min(realNumROI, NUM_ROI);

    /* save to wStageD (dst1) */
    pBBox = (vx_float32*)(dst1->memory.logicals[0]);

    jj = 0;
    for (ii=0; ii< TOP_N; ii++)
    {
        if (*(pProbBox + ii*5) > 0.0f)
        {
            memcpy(pBBox + jj*4, pProbBox + ii *5 + 1, 4 * sizeof(vx_float32));
            jj++;
            if (jj > realNumROI)
            {
                break;
            }
        }
    }

    memset((void*)(pBBox + realNumROI*4), 4*(NUM_ROI - realNumROI), sizeof(vx_float32));

   dst1->itemSize = 4;
   dst1->itemCount = NUM_ROI;

    /* (10) Copy to dst.  */
   for (ii=0; ii< NUM_ROI; ii++) {
      *((vx_float32*)dst0->memory.logicals[0] + ii*4)     = (vx_float32)pBBox[ii*4];
      *((vx_float32*)dst0->memory.logicals[0] + ii*4 + 1) = (vx_float32)pBBox[ii*4 + 1];
      *((vx_float32*)dst0->memory.logicals[0] + ii*4 + 2) = (vx_float32)pBBox[ii*4 + 2];
      *((vx_float32*)dst0->memory.logicals[0] + ii*4 + 3) = (vx_float32)pBBox[ii*4 + 3];
   }

   dst0->itemSize = 4;
   dst0->itemCount = NUM_ROI;

   //free(pProbBox);

#if defined(__linux__)
    if (node->base.context->perfEnable)
        printf("fast rcnn rpn         CPU  time:%10d us\n", gcfVX_PerfEnd((vx_reference)node, start));
#endif

#ifdef COMPARE_TO_REF

    {
        vx_uint32 i = 0, j = 0, count = 0, count1 = 0, count2 = 0, count3 = 0;
        vx_float32 *dest = (vx_float32*)dst->memory.logicals[0];
        vx_float32 * ref = (vx_float32*)malloc(256 * 5 * 1 * 1 * sizeof(vx_float32));
        FILE* ref_f = fopen("output_256_5_1_1.dat", "rb");
        fread(ref, 256 * 5 * 1 * 1, sizeof(vx_float32), ref_f);
        fclose(ref_f);


        for (i = 0, j = 0; j < 256 * 5 * 1 * 1; i ++, j++)
        {
            if(i%4 == 0)
                j ++;

            if(fabs(dest[i] - ref[j]) > 5)
                count1++;
            else if(fabs(dest[i] - ref[j])/dest[i] > 0.01)
                count++;
            else
                count3++;


            if(dest[i] != ref[j])
                count2++;
        }


        printf("rpn result have %d different pixels\n", count);
    }
#endif

    return    status;
}

/* RCNNSoftEnd = memcpy + fp16 to fp32 + deinterleave/split + softmax + threshold + memcpy to final output */
/* Todo: Need re-use CPU stagging buffer */

vx_status vxRCNNSoftEnd(vx_node node, vx_array src, vx_array src_bbox, vx_array prob, vx_array bbox, vx_scalar batch, vx_scalar networkType)
{
    vx_status status = VX_SUCCESS;
    vx_float32 *pAStage, *pBStage, *pProbStage, *pDeltaBox, *pPredBox, *pBox, *pProb;
    vx_float32 fSum=0, fMax=0;
    vx_uint32 i, j, ii, batchIndex = batch->value->u32;
    vx_uint8 networkTypeValue = networkType->value->u8;
    //FILE *fp;
    float src_w, src_h, src_ctr_x, src_ctr_y, dst_ctr_x, dst_ctr_y, dst_scl_x, dst_scl_y, pred_ctr_x, pred_ctr_y, pred_w, pred_h;
#if defined(__linux__)
    struct timeval start = gcfVX_PerfStart((vx_reference)node);
#endif

    pAStage = (vx_float32*)(node->graph->wStageA[batchIndex]->memory.logicals[0]);
    pBStage = (vx_float32*)(node->graph->wStageB[batchIndex]->memory.logicals[0]);

    /* 0) MemCpy */
    for(i=0; i<NUM_ROI*105; i++)
    {
        if (networkTypeValue == 0)
        {
            pAStage[i] = (vx_float32)(*((vx_int8*) src->memory.logicals[0] + i));
        }
        else
        {
            pAStage[i] = fp16_to_float32(*((vx_int16*) src->memory.logicals[0] + i));
        }
    }

    /* 1) deinterleave A->B*/
    pProbStage = pBStage;
    for(i=0; i<NUM_ROI; i++)
    {
        for (j=0; j<21; j++)
        {
            *(pProbStage + i*21 + j) = *(pAStage + j*NUM_ROI + i);
        }
    }

    pDeltaBox = pBStage + NUM_ROI*21;
    for(i=0; i<NUM_ROI; i++)
    {
        for (j=0; j<84; j++)
        {
            *(pDeltaBox + i*84 + j)      = *(pAStage + j*NUM_ROI + i + 21 * NUM_ROI);
        }
    }

    /* 2) bbox transform, remove background, 20 classes B->A*/
    pPredBox = (vx_float32*)(src_bbox->memory.logicals[0]);;
    pBox = pAStage;

    for (i=0; i< NUM_ROI; i++) {
    for (j=0; j< 20; j++) {  /* remove background. First class means backgroud. Skip it. */
        // src_w = double(boxes(:,3) - boxes(:,1) + 1);
        // src_h = double(boxes(:,4) - boxes(:,2) + 1);
        // src_ctr_x = double(boxes(:,1) + 0.5*(src_w-1));
        // src_ctr_y = double(boxes(:,2) + 0.5*(src_h-1));



        src_w = *(pPredBox + i*4 + 2) - *(pPredBox +i*4) + 1;
        src_h = *(pPredBox + i*4 + 3) - *(pPredBox +i*4 +1) + 1;
        src_ctr_x = *(pPredBox + i*4 + 0) + 0.5f*(src_w-1);
        src_ctr_y = *(pPredBox + i*4 + 1) + 0.5f*(src_h-1);

        ii = i*21 + j +1;

        // dst_ctr_x = double(box_deltas(:,1 : 4 : end));
        // dst_ctr_y = double(box_deltas(:,2 : 4 : end));
        // dst_scl_x = double(box_deltas(:,3 : 4 : end));
        // dst_scl_y = double(box_deltas(:,4 : 4 : end));
        dst_ctr_x = *(pDeltaBox + ii * 4 + 0);
        dst_ctr_y = *(pDeltaBox + ii * 4 + 1);
        dst_scl_x = *(pDeltaBox + ii * 4 + 2);
        dst_scl_y = *(pDeltaBox + ii * 4 + 3);

        // pred_ctr_x = bsxfun(@plus,bsxfun(@times,dst_ctr_x,src_w),src_ctr_x);
        // pred_ctr_y = bsxfun(@plus,bsxfun(@times,dst_ctr_y,src_h),src_ctr_y);
         pred_ctr_x = (dst_ctr_x*src_w) + src_ctr_x;
         pred_ctr_y = (dst_ctr_y*src_h) + src_ctr_y;

        // pred_w = bsxfun(@times,exp(dst_scl_x),src_w);
        // pred_h = bsxfun(@times,exp(dst_scl_y),src_h);
        pred_w = expf(dst_scl_x) * src_w;
        pred_h = expf(dst_scl_y) * src_h;

        // pred_boxes(:,1 : 4 : end) = pred_ctr_x - 0.5*(pred_w-1);
        // pred_boxes(:,2 : 4 : end) = pred_ctr_y - 0.5*(pred_h-1);
        // pred_boxes(:,3 : 4 : end) = pred_ctr_x + 0.5*(pred_w-1);
        // pred_boxes(:,4 : 4 : end) = pred_ctr_y + 0.5*(pred_h-1);
        *(pBox + (i*20+j) * 4 + 0) = max(min((pred_ctr_x - 0.5f*(pred_w-1)) * SCALE_BACK_RATIO, CLIP_WIDTH), 1);
        *(pBox + (i*20+j) * 4 + 1) = max(min((pred_ctr_y - 0.5f*(pred_h-1)) * SCALE_BACK_RATIO, CLIP_HEIGHT), 1);
        *(pBox + (i*20+j) * 4 + 2) = max(min((pred_ctr_x + 0.5f*(pred_w-1)) * SCALE_BACK_RATIO, CLIP_WIDTH), 1);
        *(pBox + (i*20+j) * 4 + 3) = max(min((pred_ctr_y + 0.5f*(pred_h-1)) * SCALE_BACK_RATIO, CLIP_HEIGHT), 1);
        }
    }

    /* 3) soft max */
    pProb = pAStage + NUM_ROI*20*4;
    for (i=0; i<NUM_ROI; i++)
    {
        fMax = *(pProbStage + i*21);
        for (j=1; j<21; j++)
        {
            if (fMax < *(pProbStage + i*21 + j))
            {
                fMax = *(pProbStage + i*21 + j);
            }
        }

        fSum = 0.0f;
        for (j=0; j<21; j++)
        {
            *(pProbStage + i*21 + j) = expf(*(pProbStage + i*21 + j) - fMax);
            fSum += *(pProbStage + i*21 + j);
        }

        /* skip backgrough prob (first item) */
        for (j=0; j<20; j++)
        {
            *(pProb + i*20 +j) = *(pProbStage + i*21 + j + 1)/fSum;
        }
    }

    /* 4) Load to dst*/
    memcpy((void*)(prob->memory.logicals[0] + batchIndex * NUM_ROI *20*sizeof(vx_float32)), (void*)pProb, NUM_ROI*20*sizeof(vx_float32));
    memcpy((void*)(bbox->memory.logicals[0] + batchIndex * NUM_ROI * 20 * 4 * sizeof(vx_float32)), (void*)pBox, NUM_ROI*20*4*sizeof(vx_float32));

#if defined(__linux__)
    if (node->base.context->perfEnable)
        printf("fast rcnn softend     CPU  time:%10d us\n", gcfVX_PerfEnd((vx_reference)node, start));
#endif

#if NN_MULTI_THREAD2
    if ((node->base.context->cnnEvent != VX_NULL))
        vxSetEvent(node->base.context->cnnEvent);
#endif
    return status;

}

