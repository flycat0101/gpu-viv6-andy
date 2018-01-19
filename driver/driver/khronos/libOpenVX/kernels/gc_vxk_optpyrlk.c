/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <VX/vx.h>

#include <gc_vx_common.h>
#include <gc_vxk_common.h>

#define LKZERO10 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
#define LKZERO16 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }

vx_status vxVLKTracker(vx_node node, vx_pyramid oldPyramid, vx_pyramid newPyramid, vx_pyramid gradXPyramid, vx_pyramid gradYPyramid,
    const vx_array prevPts, const vx_array estimatedPts, vx_array nextPts,
    vx_scalar criteriaScalar, vx_scalar epsilonScalar, vx_scalar numIterationsScalar, vx_bool isUseInitialEstimate, vx_scalar winSizeScalar,
    vx_int32 maxLevel, vx_float32 pyramidScaleValue
    )
{
    vx_status status = VX_FAILURE;

    vx_uint32 width[10] = LKZERO10, height[10] = LKZERO10;
    vx_int32 level = 0;
    vx_float32 epsilon = 0.001f;
    vx_uint32 numIterations = 10;
    vx_float32 fnumIter = -10.0f;
    vx_size winSize = 5;
    vx_size capacity = 0, itemsize = 0;
    vx_uint32 bin[16] = LKZERO16;
    vx_image *prevImg, *nextImg;
    vx_image *gradXImage, *gradYImage;
    vx_int32 uniformNum = 0;
    vx_int32 uniformIndex = 0;

    gcoVX_Kernel_Context * kernelContext = gcvNULL;

#if gcdVX_OPTIMIZER
    if (node && node->kernelContext)
    {
        kernelContext = (gcoVX_Kernel_Context *) node->kernelContext;
    }
    else
#endif
    {
        if (node->kernelContext == VX_NULL)
        {
            /* Allocate a local copy for old flow. */
            node->kernelContext = (gcoVX_Kernel_Context *) vxAllocate(sizeof(gcoVX_Kernel_Context));
        }
        kernelContext = (gcoVX_Kernel_Context *)node->kernelContext;
        kernelContext->objects_num = 0;
        kernelContext->uniform_num = 0;
    }

    prevImg = (vx_image*)vxAllocateAndZeroMemory(maxLevel * sizeof(vx_image));
    nextImg = (vx_image*)vxAllocateAndZeroMemory(maxLevel * sizeof(vx_image));
    gradXImage = (vx_image*)vxAllocateAndZeroMemory(maxLevel * sizeof(vx_image));
    gradYImage = (vx_image*)vxAllocateAndZeroMemory(maxLevel * sizeof(vx_image));

    for(level=maxLevel; level>0; level--)
    {
        prevImg[level-1] = vxGetPyramidLevel(oldPyramid, level-1);
        nextImg[level-1] = vxGetPyramidLevel(newPyramid, level-1);

        gradXImage[level-1] = vxGetPyramidLevel(gradXPyramid, level-1);
        gradYImage[level-1] = vxGetPyramidLevel(gradYPyramid, level-1);

        vxQueryImage(prevImg[level-1], VX_IMAGE_WIDTH, &(width[level-1]), sizeof(width[level-1]));
        vxQueryImage(prevImg[level-1], VX_IMAGE_HEIGHT, &(height[level-1]), sizeof(height[level-1]));
    }
    vxReadScalarValue(epsilonScalar, &epsilon);
    vxReadScalarValue(numIterationsScalar, &numIterations);
    //vxReadScalarValue(winSizeScalar, &winSize);

    status = (gceSTATUS)vxQueryArray(prevPts, VX_ARRAY_CAPACITY, &capacity, sizeof(capacity));
    status = (gceSTATUS)vxQueryArray(prevPts, VX_ARRAY_ITEMSIZE, &itemsize, sizeof(itemsize));

    kernelContext->params.kernel = gcvVX_KERNEL_OPTICAL_FLOW_PYR_LK;
    kernelContext->params.xmax = (vx_uint32)(capacity * itemsize);
    kernelContext->params.ymax = 1;
    kernelContext->params.xstep = (vx_uint32)itemsize;
    kernelContext->params.ystep = 1;
#if gcdVX_OPTIMIZER
    kernelContext->borders = VX_BORDER_UNDEFINED;
#else
    kernelContext->params.borders = VX_BORDER_UNDEFINED;
#endif
    kernelContext->params.isUseInitialEstimate = (gctUINT8)isUseInitialEstimate;
    kernelContext->params.maxLevel = (gctINT32)maxLevel;
    kernelContext->params.winSize = (gctINT32)winSize;

    /* prevPts - index = 0 */
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_ARRAY, prevPts, GC_VX_INDEX_AUTO);

    /* nextPts - index = 1 */
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_ARRAY, nextPts, GC_VX_INDEX_AUTO);

    /* estimatedPts - index = 2 */
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_ARRAY, estimatedPts, GC_VX_INDEX_AUTO);

    /* prevImg[] - index = 3 ~ 3+maxLevel-1 */
    for(level=maxLevel; level>0; level--)
    {
        gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, prevImg[level-1], GC_VX_INDEX_AUTO);
    }

    /* nextImg[] - index = 3+maxLevel ~ 3+maxLevel*2-1 */
    for(level=maxLevel; level>0; level--)
    {
        gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, nextImg[level-1], GC_VX_INDEX_AUTO);
    }

    /* prevDerivIx[] - index = 3+maxLevel*2 ~ 3+maxLevel*3-1 */
    for(level=maxLevel; level>0; level--)
    {
        gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, (vx_image)(gradXImage[level-1]), GC_VX_INDEX_AUTO);
    }

    /* prevDerivIy[] - index = 3+maxLevel*3 ~ 3+maxLevel*4-1 */
    for(level=maxLevel; level>0; level--)
    {
        gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, (vx_image)(gradYImage[level-1]), GC_VX_INDEX_AUTO);
    }

    {
        /* DP8 - index = 3+maxLevel*4 ~ 3+maxLevel*4+3 */
        bin[0]   = 0x155;             /* config 5x5 window */
        bin[1]   = 0x0;                  /* a select */
        bin[2]   = 0x76543210;  /* a bin */
        bin[3]   = 0x0;

        bin[4]   = 0x55555555;  /* b select */
        bin[5]   = 0x76543210;  /* b bin */
        bin[6]   = 0x0;
        bin[7]   = 0x0;                   /*pos shift */

        bin[8]   =  0;
        bin[9]   =  0;
        bin[10] = 0;
        bin[11] = 0;

        bin[12] = 0;
        bin[13] = 0;
        bin[14] = 0;
        bin[15] = 0;

        gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, bin, sizeof(bin));
        kernelContext->uniforms[0].num         = 16 * 4;
        kernelContext->uniforms[0].index       = 3 + maxLevel * 4;

        /* epsilon, numIterations, minEig(104.8576f), D(109951.1627776f) - index = 3 + maxLevel * 4 + 4 */
        fnumIter = -1.0f * numIterations;
        bin[0] = *((vx_uint32*)(&epsilon));    /* epsilonScalar */
        bin[1] = *((vx_uint32*)(&fnumIter)); /* numIterationsScalar */
        bin[2] = 0x45a3d70a;     /*5242.88f; 0x42d1b717;  minEig(104.8576f) */
        bin[3] = 0x47d6bf95;                                  /* D(109951.1627776f) */

        gcoOS_MemCopy(&kernelContext->uniforms[1].uniform, bin, sizeof(bin));
        kernelContext->uniforms[1].num         = 4 * 4;
        kernelContext->uniforms[1].index       = 3 + maxLevel * 4 + 4;

        /* prevDelta(0.01), 1/pyramidScale, iteration step, pyramidScale - index = 3 + maxLevel * 4 + 5 */
        bin[0] = 0x3a83126f;                                            /* prevDelta(0.01) */
        bin[2] = 0xbf800000;                                            /* iteration step -1.0  */
        bin[3] = *((vx_uint32*)(&pyramidScaleValue)); /* pyramidScale   */
        pyramidScaleValue = 1.0f / pyramidScaleValue;
        bin[1] = *((vx_uint32*)(&pyramidScaleValue));  /* 1/pyramidScale */

        gcoOS_MemCopy(&kernelContext->uniforms[2].uniform, bin, sizeof(bin));
        kernelContext->uniforms[2].num         = 4 * 4;
        kernelContext->uniforms[2].index       = 3 + maxLevel * 4 + 5;

        /* bounding box--winSize/2, max width&height - index = 3 + maxLevel * 4 + 6 ~ 3 + maxLevel * 5 + 6 - 1 */
        for(level=maxLevel; level>0; level--)
        {
            bin[0] = (vx_uint32)(winSize / 2);
            bin[1] = (vx_uint32)(winSize / 2);
            bin[2] = (vx_uint32)(width[level-1] - winSize / 2 - 2) ;
            bin[3] = (vx_uint32)(height[level-1] - winSize / 2 - 2);

            gcoOS_MemCopy(&kernelContext->uniforms[3+maxLevel-level].uniform, bin, sizeof(bin));
            kernelContext->uniforms[3+maxLevel-level].num         = 4 * 4;
            kernelContext->uniforms[3+maxLevel-level].index       = 3 + maxLevel * 4 + 6 + maxLevel - level;
        }

        /* L shift - index = 3 + maxLevel * 5 + 6 */
        bin[0] = 0x20202020;
        bin[1] = 0x20202020;
        bin[2] = 0x20202020;
        bin[3] = 0x20202020;

        gcoOS_MemCopy(&kernelContext->uniforms[3+maxLevel].uniform, bin, sizeof(bin));
        kernelContext->uniforms[3+maxLevel].num         = 4 * 4;
        kernelContext->uniforms[3+maxLevel].index       = 3 + maxLevel * 5 + 6;

        /* itemNum, 0.5f, 4.0f, 0 - index = 3 + maxLevel * 5 + 7 */
        bin[0] = kernelContext->params.xmax;
        bin[1] = 0x3f000000;     /* 0.5f */
        bin[2] = 0x40800000;    /* 4.0f */
        bin[3] = 0x00000001;    /* 1 */

        gcoOS_MemCopy(&kernelContext->uniforms[3+maxLevel+1].uniform, bin, sizeof(bin));
        kernelContext->uniforms[3+maxLevel+1].num         = 4 * 4;
        kernelContext->uniforms[3+maxLevel+1].index       = 3 + maxLevel * 5 + 7;

        /* bilinear uniform1 - index = 3 + maxLevel * 5 + 8 */
        bin[0] = 0x0000ff00;
        bin[1] = 0x00;
        bin[2] = 0xff000000;
        bin[3] = 0x00;

        gcoOS_MemCopy(&kernelContext->uniforms[3+maxLevel+2].uniform, bin, sizeof(bin));
        kernelContext->uniforms[3+maxLevel+2].num         = 4 * 4;
        kernelContext->uniforms[3+maxLevel+2].index       = 3 + maxLevel * 5 + 8;

        /* bilinear uniform2 - index = 3 + maxLevel * 5 + 9 */
        bin[0] = 0xff000000;
        bin[1] = 0x00;
        bin[2] = 0;
        bin[3] = 0;

        gcoOS_MemCopy(&kernelContext->uniforms[3+maxLevel+3].uniform, bin, sizeof(bin));
        kernelContext->uniforms[3+maxLevel+3].num         = 4 * 4;
        kernelContext->uniforms[3+maxLevel+3].index       = 3 + maxLevel * 5 + 9;

        /* DP4x8 - index = 3 + maxLevel * 5 + 10 ~ 3 + maxLevel * 5 + 10 +3 */
        bin[0]   = 0x55555555;  /* config0 */
        bin[1]   = 0x55;                /* config1 */
        bin[2]   = 0xd8bde96a; /* bin0 */      /* {10,11,26,27,11,12,27,28,12,13,28,29,13,14,29,30,14,15,30,31} */
        bin[3]   = 0xdef1ace6;   /* bin0 */

        bin[4]   = 0xf9eef75c;   /* bin0 */
        bin[5]   = 0xf;                   /* bin0 */
        bin[6]   = 0x0;                  /* bin0 */
        bin[7]   = 0x03;                /*pos shift-[4:0] */

        bin[8]   = 0xefcdefcd;   /* bin1 */
        bin[9]   = 0xefcdefcd;   /* bin1 */
        bin[10] = 0xefcd;           /* bin1 */
        bin[11] = 0;                       /* bin1 */

        bin[12] = 0;
        bin[13] = 0;
        bin[14] = 0;
        bin[15] = 0;

        gcoOS_MemCopy(&kernelContext->uniforms[3+maxLevel+4].uniform, bin, sizeof(bin));
        kernelContext->uniforms[3+maxLevel+4].num         = 16 * 4;
        kernelContext->uniforms[3+maxLevel+4].index       = 3 + maxLevel * 5 + 10;

        /* DP4x8 - index = 3 + maxLevel * 5 + 14 ~ 3 + maxLevel * 5 + 14 +3 */
        bin[0]   = 0x55555555;  /* config0 */
        bin[1]   = 0x55;                /* config1 */
        bin[2]   = 0xd8bde96a; /* bin0 */
        bin[3]   = 0xdef1ace6;  /* bin0 */

        bin[4]   = 0xf9eef75c;   /* bin0 */
        bin[5]   = 0xf;                   /* bin0 */
        bin[6]   = 0x0;                  /* bin0 */
        bin[7]   = 0x03;                /*pos shift-[4:0] */

        bin[8]   = 0xcdefcdef;   /* bin1 */
        bin[9]   = 0xcdefcdef;   /* bin1 */
        bin[10] = 0xcdef;            /* bin1 */
        bin[11] = 0;                       /* bin1 */

        bin[12] = 0;
        bin[13] = 0;
        bin[14] = 0;
        bin[15] = 0;

        gcoOS_MemCopy(&kernelContext->uniforms[3+maxLevel+5].uniform, bin, sizeof(bin));
        kernelContext->uniforms[3+maxLevel+5].num         = 16 * 4;
        kernelContext->uniforms[3+maxLevel+5].index       = 3 + maxLevel * 5 + 14;

        /* DP16x1 - index = 3 + maxLevel * 5 + 18 ~ 3 + maxLevel * 5 + 18 +3 */
        bin[0]   = 0xd55;                 /* config_2 */
        bin[1]   = 0x000;                 /* aSelect_2 */
        bin[2]   = 0x343210;          /* aBin_4 */
        bin[3]   = 0;                          /* aBin1_4 */

        bin[4]   = 0x155;                 /* bSelect_2 */
        bin[5]   = 0x43210;            /* bBin_4 */
        bin[6]   = 0;                          /* bBin1_4 */
        bin[7]   = 0x2000;              /* accumulatorFormat_[14:12] */

        bin[8]   = 0;
        bin[9]   = 0;
        bin[10] = 0;
        bin[11] = 0;

        bin[12] = 0;
        bin[13] = 0;
        bin[14] = 0;
        bin[15] = 0;

        gcoOS_MemCopy(&kernelContext->uniforms[3+maxLevel+6].uniform, bin, sizeof(bin));
        kernelContext->uniforms[3+maxLevel+6].num         = 16 * 4;
        kernelContext->uniforms[3+maxLevel+6].index       = 3 + maxLevel * 5 + 18;

        /* DP16x1 - index = 3 + maxLevel * 5 + 22 ~ 3 + maxLevel * 5 + 22 +3 */
        bin[0]   = 0x155;                      /* config_2 */
        bin[1]   = 0x000;                      /* aSelect_2 */
        bin[2]   = 0x43210;                 /* aBin_4 */
        bin[3]   = 0;                               /* aBin1_4 */

        bin[4]   = 0x155;                      /* bSelect_2 */
        bin[5]   = 0x43210;                 /* bBin_4 */
        bin[6]   = 0;                               /* bBin1_4 */
        bin[7]   = 0;

        bin[8]   = 0;
        bin[9]   = 0;
        bin[10] = 0;
        bin[11] = 0;

        bin[12] = 0;
        bin[13] = 0;
        bin[14] = 0;
        bin[15] = 0;

        gcoOS_MemCopy(&kernelContext->uniforms[3+maxLevel+7].uniform, bin, sizeof(bin));
        kernelContext->uniforms[3+maxLevel+7].num         = 16 * 4;
        kernelContext->uniforms[3+maxLevel+7].index       = 3 + maxLevel * 5 + 22;

        uniformNum = 3 + maxLevel + 8; /* take attention */
    }

    uniformIndex = 3+maxLevel+7;

    if (node->base.context->evisNoInst.isVX2 || node->base.context->evisNoInst.noBilinear)
    {
        vx_uint8 constantData0[16] = {16, 32, 48, 64, 80, 0, 0, 0, 16, 16, 16, 16, 16, 0, 0, 0};

        uniformIndex++;
        gcoOS_MemCopy(&kernelContext->uniforms[uniformIndex].uniform, constantData0, sizeof(constantData0));
        kernelContext->uniforms[uniformIndex].num = vxmLENGTH_OF(constantData0);
        kernelContext->uniforms[uniformIndex].index = 3 + maxLevel * 5 + 26;
        uniformNum++;
    }

    kernelContext->uniform_num = uniformNum;

    kernelContext->params.evisNoInst = node->base.context->evisNoInst;

    kernelContext->node = node;

    status = gcfVX_Kernel(kernelContext);

    for(level=maxLevel; level>0; level--)
    {
        vxReleaseImage(&prevImg[level-1]);
        vxReleaseImage(&nextImg[level-1]);
        vxReleaseImage(&gradXImage[level-1]);
        vxReleaseImage(&gradYImage[level-1]);
    }

    vxFree(prevImg);
    vxFree(nextImg);
    vxFree(gradXImage);
    vxFree(gradYImage);

#if gcdVX_OPTIMIZER
    if (!node || !node->kernelContext)
    {
        vxFree(kernelContext);
    }
#endif

#if gcdDUMP
        /* Verify Output Array */
        gcmDUMP_BUFFER(gcvNULL,
                    "verify",
                    kernelContext->obj[1].info.physicals[0],
                    (gctPOINTER)kernelContext->obj[1].info.logicals[0],
                    0,
                    kernelContext->obj[1].info.bytes);
#endif

    return status;
}

