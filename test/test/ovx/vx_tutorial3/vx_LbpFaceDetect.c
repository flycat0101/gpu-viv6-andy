/****************************************************************************
*
*    Copyright 2012 - 2017 Vivante Corporation, Santa Clara, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/


#include <VX/vx.h>
#include <VX/vxu.h>
#include <VX/vx_ext_program.h>
#include "C:\p4project\TEST\SW\Internal\cl11\FftMatrix\LbpDetect.txt"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>

#ifdef LINUX
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/fb.h>
#include <unistd.h>
#endif

#define VXV_KERNEL_NAME_VXCABSDIFF "com.vivantecorp.extension.vxcAbsDiff"
#define VXV_KERNEL_VXCABSDIFF      100
#define MEMCHECK(memPtr)
#define OBJCHECK(objVX)

typedef unsigned char     u08;
typedef unsigned short    u16;
typedef unsigned int      u32;
typedef int               s32;

typedef struct _file_header
{
    u16      fileType;
    u16      fileSize1;
    u16      fileSize2;
    u16      fileReserved1;
    u16      fileReserved2;
    u16      fileOffset1;
    u16      fileOffset2;
}FileHeader;

typedef struct _info_header
{
    u32      infoSize;
    u32      imageWidth;
    s32       imageHeight;
    u16      imagePlane;
    u16      imageCount;
    u32      imageCompression;
    u32      imageSize;
    u32      hResolution;
    u32      vResolution;
    u32      clrUsed;
    u32      clrImportant;
}InfoHeader;

typedef struct _vx_param_description_s
{
    vx_enum                 paramDire;
    vx_enum                 paramType;
    vx_enum                 paramStat;
}
vx_param_description_s;


//Now it needs 18 Registers, 18KB Local Memory
//to reduce Local memory to 16K, we don't load all features in to the Local Memory (140*48) = 6.7KB,
//we may keep first half, load 2nd half when it needs.

//For old IP, we have to use 2 kernals, kernal0 calculate overlapping integral mapping for each 64x64 blocks (3-D Image)
//Then we use Image load for integral pixel to avoid swizzle (now 16/58 inside the loop).
//Integral mapping only take small amount of time (can reach 500fps).
//We expect the cache hit instead of Local Memory for old IP.

//If we have scatter-gathering load, we can reduce number on instruction to
//58-32(Load&Swizzle) - 4 (And0xffff & Shift16) + 2 (ScatterGather) = 24
//3 more instructions can be further saved with LoopVector Increasement with offset, *16*3 can be *48, and if( f & (1<<symL) == 0)...
//If 64 threads/group can seperate to 2 of 32-thread groups. Maybe increase 20% performance? I don't know yet.




char vxcAbsDiffSource[] = {
    "#include \"cl_viv_vx_ext.h\" \n\
\n\
_viv_uniform uint height; \n\
_viv_uniform uint width; \n\
_viv_uniform uint xyIncr; \n\
_viv_uniform VXC_512Bits AreaFor3RectanglesInSrc0; \n\
_viv_uniform VXC_512Bits SubCenterRec; \n\
_viv_uniform VXC_512Bits symbHighLow; \n\
_viv_uniform VXC_512Bits Next2Lines; \n\
_viv_uniform VXC_512Bits V8AddScalar; \n\
_viv_uniform vxc_short8 sh8Const;\n\
\n\
#define BLOCK_DIM 64\n\
#define SUB_WIN      24\n\
#define WALK_IN_BLOCK (BLOCK_DIM - SUB_WIN)\n\
\n\
\n\
//#define DEBUG_ONE_XY\n\
#ifdef DEBUG_ONE_XY\n\
#define debugX  42   \n\
#define debugY  46   \n\
#endif\n\
\n\
#define OFFSET_STAGE_LEN     0\n\
#define OFFSET_STAGE_OFFSET 20\n\
#define OFFSET_STAGE_TH (OFFSET_STAGE_OFFSET + 20)\n\
#define OFFSET_FEATURE_POS (OFFSET_STAGE_TH + 20)\n\
#define OFFSET_FEATURE_VALUES    (OFFSET_FEATURE_POS + 140*(8+4))\n\
    __kernel void vxcAbsDiff \n\
    ( \n\
    __read_only image2d_t     imgGray,\n\
    __read_only image2d_t     in1_image, //No use in LBP\n\
    vx_array_uint ptInfArray, \n\
    vx_array_int OutPtInfArray, \n\
    __write_only image2d_t     out_image //No use in LBP \n\
    ) \n\
    { \n\
    int loopBlockY, loopBlockX, loopLocalY, loopLocalX, x, y, i, j, k, id;\n\
    const sampler_t sampler =\n\
        CLK_NORMALIZED_COORDS_FALSE |\n\
        CLK_ADDRESS_CLAMP_TO_EDGE |\n\
        CLK_FILTER_NEAREST;\n\
    local uint4 BigBlock4[((140*(4*4 + 4 + 8*4) + 20*3*4) + BLOCK_DIM*BLOCK_DIM*2)/16 ], *pInt4; //BigBlock4. 7520 + 8KB\n\
    local unsigned short *integralPixel = (unsigned short *) &BigBlock4[(140*(4*4 + 4 + 8*4) + 20*3*4)/16]; //8KB\n\
    local unsigned short pShortXY[WALK_IN_BLOCK*WALK_IN_BLOCK];  //I cannot eliminate, 3200 Bytes.\n\
    local int *pInt = (int *)&BigBlock4[OFFSET_FEATURE_VALUES/4]; //In order to make the inside loop access quick\n\
#ifdef ATOMIC_LS  \n\
    local int validCnt[1]; //How many positions valid for next stage  \n\
    local int sumLeftRight[64]; \n\
    local int allCnt[1]; \n\
#else \n\
    int *validCnt = OutPtInfArray.item+256; \n\
    int *sumLeftRight = validCnt + 1; \n\
    int *allCnt = &OutPtInfArray.item[0]; \n\
#endif    \n\
    short xy,s1 = 1;\n\
    ushort2  shV2;\n\
    ushort4  recV4;\n\
    uint4 recXY, i4;\n\
    id = get_local_id(0);\n\
    allCnt[0] = 0;\n\
    pInt4 = &BigBlock4[20*3*4/16];\n\
\n\
    for(j = id; j < (140*(4*4 + 4 + 8*4) + 20*3*4) / 16; j += BLOCK_DIM ){\n\
        BigBlock4[j] = vload4(j, ptInfArray.item);\n\
    }\n\
    \n\
    for(loopBlockY = 0; loopBlockY < (height - SUB_WIN); loopBlockY += WALK_IN_BLOCK){ //height - BLOCK_DIM should be height,  with image_read used\n\
        for(loopBlockX = 0; loopBlockX < (width - SUB_WIN); loopBlockX += WALK_IN_BLOCK){ //width - BLOCK_DIM should be width, with image_read used            \n\
#ifdef DEBUG_ONE_XY\n\
    if(loopBlockY > debugY || loopBlockY + WALK_IN_BLOCK <= debugY ||\n\
       loopBlockX > debugX || loopBlockX + WALK_IN_BLOCK <= debugX )\n\
        continue;\n\
#endif\n\
\n\
            int sum1 = 0;\n\
            int2 coord = (int2)(id + loopBlockX , (loopBlockY));\n\
            int  walkNum =  WALK_IN_BLOCK >> (xyIncr - 1); //Num of walk in initial block, actually it is WALK_IN_BLOCK/xyIncr, but we only support xyIncr = 1&2 so far\n\
            for(loopLocalY = 0; loopLocalY<BLOCK_DIM; loopLocalY++){\n\
                unsigned char pixel;\n\
                uint4 temp;\n\
                pixel = read_imageui(imgGray, sampler, coord).x;\n\
                //pixel = imgGray[id + loopBlockX + (loopBlockY + loopLocalY)*width]; //Should use image_read\n\
                sum1 += pixel;\n\
                integralPixel[loopLocalY*BLOCK_DIM + id] = sum1; //initial the integral intensity\n\
                coord.y ++;\n\
                \n\
                if(id  < walkNum && loopLocalY  < walkNum){ //Initial ValidXY\n\
                    pShortXY[id + loopLocalY * walkNum] = ((loopLocalY + 1)* xyIncr - 1)*BLOCK_DIM + ( (id+1)*xyIncr - 1);\n\
                        \n\
                }\n\
                    \n\
            }\n\
            barrier(CLK_LOCAL_MEM_FENCE);\n\
            //Now we finishing loading the image\n\
            //Start to calculate square_integral map, Sum up horizontally\n\
            sum1 = integralPixel[id *BLOCK_DIM +0];\n\
\n\
            for(loopLocalX = 1; loopLocalX<BLOCK_DIM; loopLocalX++){\n\
                k = id*BLOCK_DIM + loopLocalX;\n\
                sum1 += integralPixel[k];                    \n\
                integralPixel[k] = sum1;                    \n\
                \n\
            }\n\
            barrier(CLK_LOCAL_MEM_FENCE);\n\
            //Only handle xyIncr = 1 and 2 cases.\n\
            validCnt[0] = walkNum*walkNum;\n\
            \n\
\n\
#ifdef DEBUG_ONE_XY\n\
    validCnt[0] = 1;\n\
    pShortXY[0] = (debugX%WALK_IN_BLOCK) + (debugY%WALK_IN_BLOCK)*64;\n\
#endif\n\
\n\
            \n\
            int stageLoop;                \n\
            for(stageLoop = 0; stageLoop < 20; stageLoop++){ //20 threads\n\
            int curCnt = validCnt[0], numNode = pInt[-OFFSET_FEATURE_VALUES + stageLoop], offset = pInt[stageLoop - (OFFSET_FEATURE_VALUES - OFFSET_STAGE_OFFSET)];\n\
                float variance;\n\
                \n\
                if(curCnt == 0)\n\
                    break;\n\
                int curCntBy64 = curCnt & 0xffc0; //The main part of the pixels. each thread processes all the features for a pixel\n\
                \n\
                if(id == 0){\n\
                    validCnt[0] = 0;\n\
                }                    \n\
                //barrier(CLK_LOCAL_MEM_FENCE); //Not necessary?\n\
                \n\
                for(j = id; j<curCntBy64; j+=64){    \n\
                    xy = pShortXY[j];\n\
                    int sum = 0;\n\
                            \n\
                    for(int loopNode = 0; loopNode < numNode; loopNode++){\n\
                        vxc_short8 sh8_0, sh8_1;\n\
                        VXC_Vload4(sh8_0 ,  pInt4, offset*(12/4));\n\
                        _viv_asm(COPY,  sh8_1, xy, 16);//sh8_1.s0 = xy;\n\
                        VXC_DP2x8(sh8_0, sh8_0, sh8_1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), V8AddScalar);\n\
                         _viv_asm(COPY, recXY, sh8_0, 16);\n\
                        \n\
                     //Feature processing\n\
                        int symH, symL, f;//High 3 bits and Low 5 bits\n\
                        int2 lineXY;\n\
                        vxc_short8 u8, abcd,efgh, v8; \n\
                        ushort u0,u1,u2,u3,v0,v1,v2,v3,a,b,c,d,e,g,h, mid;\n\
                        vxc_short4 u4,v4;\n\
                        u8.s0 = integralPixel[recXY.x & 0xffff]; //line-0\n\
                        u8.s1 = integralPixel[recXY.x >>16];\n\
                        u8.s2 = integralPixel[recXY.y & 0xffff];\n\
                        u8.s3 = integralPixel[recXY.y >>16];\n\
\n\
                        u8.s4 = integralPixel[recXY.z & 0xffff]; //line-1\n\
                        u8.s5 = integralPixel[recXY.z >>16];\n\
                        u8.s6 = integralPixel[recXY.w & 0xffff];\n\
                        u8.s7 = integralPixel[recXY.w >>16];\n\
                    \n\
                        VXC_DP4x4(abcd, u8, u8, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), AreaFor3RectanglesInSrc0); \n\
\n\
                        VXC_DP2x8(sh8_1, sh8_0, sh8_0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), Next2Lines); \n\
                        _viv_asm(COPY, recXY, sh8_1, 16); \n\
                        u8.s0 = integralPixel[recXY.x & 0xffff]; //line-2 loaded\n\
                        u8.s1 = integralPixel[recXY.x >>16];\n\
                        u8.s2 = integralPixel[recXY.y & 0xffff];\n\
                        u8.s3 = integralPixel[recXY.y >>16];\n\
\n\
                        VXC_DP4x4(abcd, u8, u8, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0), AreaFor3RectanglesInSrc0); //this part is negative area \n\
\n\
                        u8.s4 = integralPixel[recXY.z & 0xffff]; //line-3\n\
                        u8.s5 = integralPixel[recXY.z >>16];\n\
                        u8.s6 = integralPixel[recXY.w & 0xffff];\n\
                        u8.s7 = integralPixel[recXY.w >>16];\n\
                                                        \n\
                        VXC_DP4x4(efgh, u8, u8, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), AreaFor3RectanglesInSrc0); \n\
\n\
                        VXC_DP2x8(u8, abcd, efgh, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), SubCenterRec); //Sub to the center rectangle\n\
\n\
                        _viv_asm(COPY,  v8, s1, 16); //This function make the next Dp8x2 know that 1 on v8.x is enough \n\
                        VXC_DP8x2(i4, u8, v8, VXC_MODIFIER(0, 1, 0, VXC_RM_TowardZero, 0), symbHighLow); //Sub to the center rectangle\n\
\n\
                        symH = i4.x;\n\
                        symL = i4.y; \n\
//                      f = pInt[(symH) - ( OFFSET_FEATURE_VALUES - (OFFSET_FEATURE_POS + 4) ) + offset*12]; //Old instr., SymH directly calculated \n\
                        f = pInt[(symH) + offset*12]; //new instr., now the constant part (-1676) combine with SymH calculation \n\
                        if( (f & (1 << symL)) == 0 ) //Reorder Left/Right Value, make f = (pInt[symH]>>symL)&1, sum += pInt[]*f, reduce 1 instr.\n\
                            sum += pInt[offset];\n\
                        offset++; //Should vectorize with the loop             \n\
                    }\n\
                    offset -= numNode; //recover the offset increament in the above loop.\n\
                    \n\
                    if(sum >= pInt[stageLoop  - (OFFSET_FEATURE_VALUES -  OFFSET_STAGE_TH)]){ //compare with stage threshold\n\
                        int cnt1 = atomic_inc(validCnt); \n\
                        pShortXY[cnt1] = xy;\n\
                    }\n\
                    barrier(CLK_LOCAL_MEM_FENCE); //64 pixels sync once\n\
                }\n\
\n\
                int restBy64 = curCnt - curCntBy64; //The rest of pixels\n\
                if(restBy64 == 0) //No needs go the breakdown processing\n\
                    continue;\n\
                sumLeftRight[id] = 0; //clean the sum\n\
                k = restBy64*numNode; //All pixel features need to process\n\
                \n\
                for(j = id; j<k; j+=64){\n\
                    int featureIndex = (int) ( (short)j / (short)restBy64 );\n\
                    int pixel = j - restBy64*featureIndex;\n\
                    xy = pShortXY[pixel + curCntBy64];\n\
                    offset += featureIndex;\n\
\n\
                    { \n\
                        vxc_short8 sh8_0, sh8_1;\n\
                        VXC_Vload4(sh8_0 ,  pInt4, offset*(12/4));\n\
                        _viv_asm(COPY,  sh8_1, xy, 16);//sh8_1.s0 = xy;\n\
                        VXC_DP2x8(sh8_0, sh8_0, sh8_1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), V8AddScalar);\n\
                         _viv_asm(COPY, recXY, sh8_0, 16);\n\
                            //Feature processing\n\
                        int symH, symL, f;//High 3 bits and Low 5 bits\n\
                        int2 lineXY;\n\
                        vxc_short8 u8, abcd,efgh, v8; \n\
                        ushort u0,u1,u2,u3,v0,v1,v2,v3,a,b,c,d,e,g,h, mid;\n\
                        vxc_short4 u4,v4;\n\
                        u8.s0 = integralPixel[recXY.x & 0xffff]; //line-0\n\
                        u8.s1 = integralPixel[recXY.x >>16];\n\
                        u8.s2 = integralPixel[recXY.y & 0xffff];\n\
                        u8.s3 = integralPixel[recXY.y >>16];\n\
                        \n\
                        u8.s4 = integralPixel[recXY.z & 0xffff]; //line-1\n\
                        u8.s5 = integralPixel[recXY.z >>16];\n\
                        u8.s6 = integralPixel[recXY.w & 0xffff];\n\
                        u8.s7 = integralPixel[recXY.w >>16];\n\
                        VXC_DP4x4(abcd, u8, u8, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), AreaFor3RectanglesInSrc0); \n\
\n\
                        VXC_DP2x8(sh8_1, sh8_0, sh8_0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), Next2Lines); \n\
                        _viv_asm(COPY, recXY, sh8_1, 16); \n\
                        u8.s0 = integralPixel[recXY.x & 0xffff]; //line-2 loaded\n\
                        u8.s1 = integralPixel[recXY.x >>16];\n\
                        u8.s2 = integralPixel[recXY.y & 0xffff];\n\
                        u8.s3 = integralPixel[recXY.y >>16];\n\
\n\
                        VXC_DP4x4(abcd, u8, u8, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0), AreaFor3RectanglesInSrc0); //this part is negative area \n\
\n\
                        u8.s4 = integralPixel[recXY.z & 0xffff]; //line-3\n\
                        u8.s5 = integralPixel[recXY.z >>16];\n\
                        u8.s6 = integralPixel[recXY.w & 0xffff];\n\
                        u8.s7 = integralPixel[recXY.w >>16];\n\
                        \n\
                        VXC_DP4x4(efgh, u8, u8, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), AreaFor3RectanglesInSrc0); \n\
\n\
                        VXC_DP2x8(u8, abcd, efgh, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), SubCenterRec); //Sub to the center rectangle\n\
\n\
                        _viv_asm(COPY,  v8, s1, 16); VXC_DP8x2(i4, u8, v8, VXC_MODIFIER(0, 1, 0, VXC_RM_TowardZero, 0), symbHighLow); //Sub to the center rectangle\n\
\n\
                        symH = i4.x;\n\
                        symL = i4.y; \n\
                        f = pInt[(symH)   + offset*12];\n\
                        if( (f & (1 << symL)) == 0 ){\n\
                            atomic_add(sumLeftRight + pixel,  pInt[offset]);    \n\
                        }\n\
                             \n\
                    }\n\
                    offset -= featureIndex; //recover the offset increament in the above loop.                    \n\
                }\n\
                barrier(CLK_LOCAL_MEM_FENCE);\n\
                if(id < restBy64){\n\
#ifndef DEBUG_ONE_XY\n\
                if(sumLeftRight[id] >= pInt[stageLoop  - (OFFSET_FEATURE_VALUES -  OFFSET_STAGE_TH)])\n\
#endif\n\
                    {\n\
                        int cnt1 = atomic_inc(validCnt); \n\
                        pShortXY[cnt1] = pShortXY[id + curCntBy64];                        \n\
                    }\n\
#ifdef DEBUG_ONE_XY\n\
                    OutPtInfArray.item[stageLoop + 2] = floor((sumLeftRight[id] - pInt[stageLoop  - (OFFSET_FEATURE_VALUES -  OFFSET_STAGE_TH)]));\n\
#endif    \n\
                }\n\
                barrier(CLK_LOCAL_MEM_FENCE);\n\
                                \n\
            } \n\
            \n\
            for(j = id; j<validCnt[0]; j+=64){\n\
                int cnt1 = atomic_inc(allCnt); \n\
                OutPtInfArray.item[cnt1 + 1] = (pShortXY[j]&0x3f) + loopBlockX +  256*256*( (pShortXY[j]>>6)+loopBlockY );                \n\
            }\n\
        }\n\
    }\n\
#ifdef ATOMIC_LS  \n\
    if(id == 0){\n\
        OutPtInfArray.item[0] = allCnt[0]; \n\
    }\n\
#endif \n\
    return;\n\
}"
};

static vx_param_description_s paramsAbsDiff[] =
{
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED}
};

int freeRes(FILE** srcFile0, FILE** srcFile1, FILE** destFile, u08** tmpBuf, u08** tmpBuf2, u08** imgBuf, vx_image imgObj[], vx_kernel* vxKernel, vx_node* vxNode, vx_graph* GraphVX, vx_context* ContextVX);
vx_status VX_CALLBACK vxcAbsDiffValidateInput(vx_node node, vx_uint32 index);
vx_status VX_CALLBACK vxcAbsDiffValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta);
vx_status VX_CALLBACK vxcAbsDiffInitialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_status vxcAbsDiffRegisterKernel(vx_context context);
vx_status VX_CALLBACK vxAbsDiffValidator(vx_node node, const vx_reference parameters[], vx_uint32 num, vx_meta_format metas[]);
int main(int argc, char* argv[])
{
    /* VX variables */
    vx_context ContextVX = NULL;
    vx_graph GraphVX = NULL;
    vx_node vxNode = NULL;
    vx_kernel vxKernel = NULL;

    vx_image imgObj[3] = { NULL, NULL, NULL };
    vx_imagepatch_addressing_t imgInfo[3] = { VX_IMAGEPATCH_ADDR_INIT, VX_IMAGEPATCH_ADDR_INIT, VX_IMAGEPATCH_ADDR_INIT };
    void* imgAddr[3] = { NULL, NULL, NULL };
    void* imgPixel[3] = { NULL, NULL, NULL };
    vx_rectangle_t imgRect = { 0, 0, 0, 0 };
    vx_uint32 imgWid = 0, imgHei = 0;
    vx_status status = VX_SUCCESS;
    vx_array    *pArrayObj = NULL;

    FILE* fFile0 = NULL;
    FILE* fFile1 = NULL;
    FILE* fOut = NULL;
    u08* tmpBuf = NULL;
    u08* tmpBuf2 = NULL;
    u08* imgBuf[2] = {NULL, NULL};
    u32 fileSize = 0;
    u32 fileOffset = 0;
    u32 x = 0, y = 0;
    int i = 0;

    vx_map_id map_id = 0;

    /* read image data */
  // fFile0 = fopen("left_gray.bmp", "rb");
//   fFile0 = fopen("..\\..\\..\\..\\Internal\\cl11\\FftMatrix\\Images\\149x112.raw", "rb"); imgWid = 149;  imgHei = 112;
   fFile0 = fopen("..\\..\\..\\..\\Internal\\cl11\\FftMatrix\\Images\\86x65.raw", "rb"); imgWid = 80;  imgHei = 65;
//    fFile0 = fopen("..\\..\\..\\..\\Internal\\cl11\\FftMatrix\\Images\\309x231.raw", "rb"); imgWid = 309;  imgHei = 231;
   // fFile0 = fopen("..\\..\\..\\..\\Internal\\cl11\\FftMatrix\\003.raw", "rb"); imgWid = 640;  imgHei = 480;
    if(NULL==fFile0)
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "file error.");

        status = -1;
        goto exit;
    }

    fFile1 = fopen("right_gray.bmp", "rb");
    if(NULL==fFile1)
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "file error.");

        status = -1;
        goto exit;
    }


    for (i = 0; i < 2; i++)
    {

        imgBuf[i] = (u08*)malloc(imgHei*imgWid*sizeof(u08));
        if(NULL==imgBuf[i])
        {
            printf("%s:%d, %s\n", __FILE__, __LINE__, "memory error.");

            status = -1;
            goto exit;
        }

        fread((u08*)imgBuf[i], imgHei*imgWid*sizeof(u08), 1,  fFile0);
        rewind( fFile0);
    }


    /* vx procedure */
    ContextVX = vxCreateContext();
    if(NULL!=ContextVX)
    {
        vxcAbsDiffRegisterKernel(ContextVX);
        GraphVX = vxCreateGraph(ContextVX);

        pArrayObj = (vx_array *)malloc(2 * sizeof(vx_array));
        pArrayObj[0] = vxCreateArray(ContextVX, VX_TYPE_INT32, sizeof(FeaturePos_Th_Subset)/sizeof(int)); //For the features
        pArrayObj[1] = vxCreateArray(ContextVX, VX_TYPE_INT32, sizeof(FeaturePos_Th_Subset)/sizeof(int));
        vxAddArrayItems(pArrayObj[0], sizeof(FeaturePos_Th_Subset)/sizeof(int), &FeaturePos_Th_Subset[0], 0);
        vxAddArrayItems(pArrayObj[1], sizeof(FeaturePos_Th_Subset)/sizeof(int), &FeaturePos_Th_Subset[0], 0);

        if(NULL!=GraphVX)
        {
             vx_border_t border = {VX_BORDER_REPLICATE, 0};
            imgObj[0] = vxCreateImage(ContextVX, imgWid, imgHei, VX_DF_IMAGE_U8);
            imgObj[1] = vxCreateImage(ContextVX, imgWid, imgHei, VX_DF_IMAGE_U8);
            imgObj[2] = vxCreateImage(ContextVX, imgWid, imgHei, VX_DF_IMAGE_U8);

            imgRect.start_x = 0;
            imgRect.start_y = 0;
            imgRect.end_x = imgWid;
            imgRect.end_y = imgHei;

            vxKernel = vxGetKernelByName(ContextVX, VXV_KERNEL_NAME_VXCABSDIFF);
            status = vxGetStatus((vx_reference)vxKernel);
            if(status!= VX_SUCCESS)
            {
                printf("[%s : %d] vxGetKernelByName failed,status = %d \n", __FILE__, __LINE__, status);
                goto exit;
            }
            vxNode = vxCreateGenericNode(GraphVX, vxKernel);
            status = vxGetStatus((vx_reference)vxNode);
            if(status!= VX_SUCCESS)
            {
                printf("[%s : %d] vxCreateGenericNode failed,status = %d \n", __FILE__, __LINE__, status);
                goto exit;
            }
            status |= vxSetNodeAttribute(vxNode, VX_NODE_BORDER, &border, sizeof(border));

            vxSetNodeUniform(vxNode, "width", 1, &imgWid);
            vxSetNodeUniform(vxNode, "height", 1, &imgHei);
            {
                unsigned int xyIncr = imgWid >= 320? 2:1;
                unsigned short a[8] = {1,2,3,4,5,6,7,8};
                vxSetNodeUniform(vxNode, "xyIncr", 1, &xyIncr);
                vxSetNodeUniform(vxNode, "sh8Const", 1, &a[0]);


            }
            {
                vx_uint32 AreaFor3RectanglesInSrc0[16] = {
                    0x00696969, // TCfg
                    0x00000000, // ASelt
                    0x65215410, 0x00007632, // ABin
                    0x00aaaaaa, // BSelt
                    0x00000000, 0x00000000, // BBin
                    0x00000400, // AccumType, ConstantType, and PostShift
                    0x00010001, 0x00010001, 0x00010001, 0x00010001, 0x00010001, 0x00010001, 0x00000000, 0x00000000 // Constant
                };
                vx_uint32 SubCenterRec[16] = {
                    0x99999999, // TCfg
                    0x11100000, // ASelt
                    0x54525150, 0x52515056, // ABin
                    0xaaaaaaaa, // BSelt
                    0x00000000, 0x00000000, // BBin
                    0x0000030f, // AccumType, ConstantType, and PostShift
                    0xffff0001, 0xffff0001, 0xffff0001, 0xffffffff, 0xffffffff, 0xffff0001, 0xffff0001, 0xffff0001 // Constant
                };
                vx_uint32 symbHighLow[16] = {
                    0x05550095, // TCfg
                    0x04000040, // ASelt
                    0x00000210, 0x00076543, // ABin
                    0x0aaa00aa, // BSelt
                    0x00000000, 0x00000000, // BBin
                    0x00000300, // AccumType, ConstantType, and PostShift
                    0x00020004, 0x06850001, 0x00000000, 0x00000000, 0x00100001, 0x00040002, 0x001f0008, 0x00000000 // Constant
                };


                vx_uint32 Next2Lines[16] = {
                    0x66666666, // TCfg
                    0x55550000, // ASelt
                    0x73625140, 0x73625140, // ABin
                    0xaaaaaaaa, // BSelt
                    0x00000000, 0x00000000, // BBin
                    0x00000400, // AccumType, ConstantType, and PostShift
                    0x00020001, 0x00020001, 0x00020001, 0x00020001, 0x00030002, 0x00030002, 0x00030002, 0x00030002 // Constant
                };
                vx_uint32 V8AddScalar[16] = {
                    0x55555555, // TCfg
                    0x44444444, // ASelt
                    0x03020100, 0x07060504, // ABin
                    0xaaaaaaaa, // BSelt
                    0x00000000, 0x00000000, // BBin
                    0x00000400, // AccumType, ConstantType, and PostShift
                    0x00010001, 0x00010001, 0x00010001, 0x00010001, 0x00010001, 0x00010001, 0x00010001, 0x00010001 // Constant
                };
 // Please cut the following declarations to VXC source file.
                vxSetNodeUniform(vxNode, "V8AddScalar", 1, V8AddScalar);
                vxSetNodeUniform(vxNode, "Next2Lines", 1, Next2Lines);
                vxSetNodeUniform(vxNode, "SubCenterRec", 1, SubCenterRec);
                vxSetNodeUniform(vxNode, "AreaFor3RectanglesInSrc0", 1, AreaFor3RectanglesInSrc0);
                vxSetNodeUniform(vxNode, "symbHighLow", 1, symbHighLow); //To calculate SymH and SymLow From the comparision of Rec[i] vs. Rec[4]
            }


            status  = vxSetParameterByIndex(vxNode, 0, (vx_reference)imgObj[0]);
            status |= vxSetParameterByIndex(vxNode, 1, (vx_reference)imgObj[1]);
            status |= vxSetParameterByIndex(vxNode, 2, (vx_reference)pArrayObj[0]);
            status |= vxSetParameterByIndex(vxNode, 3, (vx_reference)pArrayObj[1]);
            status |= vxSetParameterByIndex(vxNode, 4, (vx_reference)imgObj[2]);
            if(status != VX_SUCCESS)
            {
                printf("[%s : %d] vxSetParameterByIndex failed,status = %d \n", __FILE__, __LINE__, status);
                goto exit;
            }

            status = vxVerifyGraph(GraphVX);
            if(status != VX_SUCCESS)
            {
                printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");
                goto exit;
            }
        }
        else
        {
            printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");

            status = -1;
            goto exit;
        }
    }
    else
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");

        status = -1;
        goto exit;
    }

    /* transfer image from cpu to gpu */
    for (i = 0; i < 2; i++)
    {
        status = vxMapImagePatch(imgObj[i], &imgRect, 0, &map_id, &imgInfo[i], &imgAddr[i], VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

        if(status != VX_SUCCESS)
        {
            printf("%s:%d, %s\n", __FILE__, __LINE__, "vx vxAccessImagePatch error.");
            goto exit;
        }

        if((1!=imgInfo[i].step_y)||(1!=imgInfo[i].step_x)||(imgHei!=imgInfo[i].dim_y)||(imgWid!=imgInfo[i].dim_x))
        {
            printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");
            status = -1;
            goto exit;
        }

        for(y=0; y < imgHei; y++)
        {
            for (x=0; x < imgWid; x++)
            {
                imgPixel[i] = vxFormatImagePatchAddress2d(imgAddr[i], x, y, &(imgInfo[i]));
                *((vx_uint8*)(imgPixel[i])) = imgBuf[i][y*imgWid+x];
            }
        }

        status = vxUnmapImagePatch(imgObj[i], map_id);

        if(status != VX_SUCCESS)
        {
            printf("%s:%d, %s\n", __FILE__, __LINE__, "vx vxCommitImagePatch error.");
            goto exit;
        }
        imgAddr[i] = NULL;
    }



    /* process image */
    status = vxProcessGraph(GraphVX);
    if(status != VX_SUCCESS)
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "vx vxProcessGraph error.");
        goto exit;
    }

    /* transfer image from gpu to cpu */
    status = vxMapImagePatch(imgObj[2], &imgRect, 0, &map_id, &(imgInfo[2]), &imgAddr[2], VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);
    if(status != VX_SUCCESS)
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "vx vxAccessImagePatch error.");
        goto exit;
    }

    if((imgInfo[2].dim_y!=imgHei)||(imgInfo[2].dim_x!=imgWid)||(imgInfo[2].step_y!=1)||(imgInfo[2].step_x!=1))
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");

        status = -1;
        goto exit;
    }

    for(y=0; y<imgHei; y++)
    {
        for (x=0; x<imgWid; x++)
        {
            imgPixel[2] = vxFormatImagePatchAddress2d(imgAddr[2], x, y, &(imgInfo[2]));
            imgBuf[0][y*imgWid+x] = *((vx_uint8*)(imgPixel[2]));
        }
    }

    status = vxUnmapImagePatch(imgObj[2], map_id);
    if(status != VX_SUCCESS)
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");
        goto exit;
    }
    imgAddr[2] = NULL;

    /* output image */
    //fOut = fopen("absdiff_out.bmp", "wb");
    //if(NULL==fOut)
    //{
    //    printf("%s:%d, %s\n", __FILE__, __LINE__, "file error.");
    //    status = -1;
    //    goto exit;
    //}

    //fwrite(tmpBuf, fileSize, 1, fOut);
    //fwrite(tmpBuf2, (fileOffset-fileSize), 1, fOut);
    //fwrite(imgBuf[0], imgHei*imgWid*sizeof(u08), 1, fOut);
    {//print out data result.
        int *pObjInf;
        vx_size itemNumber = 0;
        vx_size arrayStride = 0;
        vxQueryArray(pArrayObj[1], VX_ARRAY_NUMITEMS, &itemNumber, sizeof(itemNumber));
        vxQueryArray(pArrayObj[1], VX_ARRAY_ITEMSIZE, &arrayStride, sizeof(arrayStride));
        pObjInf = (int*)malloc(arrayStride*itemNumber);
        vxMapArrayRange(pArrayObj[1], 0, itemNumber, &map_id, &arrayStride, (void**)&pObjInf, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST,0);
        printf("%d:\n", pObjInf[0]);
        for(i = 0; i<pObjInf[0]; i++){
            printf("(%d, %d), ", pObjInf[i+1]&0xffff, pObjInf[i+1]>>16);
        }
        printf("\n");
        free(pObjInf);
    }

    status = VX_SUCCESS;
    printf("vx process success.\n");
    /* exit and free */
exit:
    freeRes(&fFile0, &fFile1, &fOut, &tmpBuf, &tmpBuf2, imgBuf, imgObj, &vxKernel, &vxNode, &GraphVX, &ContextVX);
    vxReleaseArray(&(pArrayObj[0]));
    //vxReleaseArray(&(pArrayObj[1]));
    return status;
}

int freeRes(FILE** srcFile0, FILE** srcFile1, FILE** destFile, u08** tmpBuf, u08** tmpBuf2, u08** imgBuf, vx_image imgObj[], vx_kernel* vxKernel, vx_node* vxNode, vx_graph* GraphVX, vx_context* ContextVX)
{
    int i = 0;

    if(NULL!=*srcFile0) { fclose(*srcFile0); *srcFile0 = NULL; }
    if(NULL!=*srcFile1) { fclose(*srcFile1); *srcFile1 = NULL; }
    if(NULL!=*destFile) { fclose(*destFile); *destFile = NULL; }
    if(NULL!=*tmpBuf) { free(*tmpBuf); *tmpBuf = NULL; }
    for(i=0; i<2; i++) if(NULL!=imgBuf[i]) { free(imgBuf[i]); imgBuf[i] = NULL; }
    for(i=0; i<3; i++) { if(NULL!=imgObj[i]) vxReleaseImage(&(imgObj[i])); }
    if(NULL!=*vxKernel) { vxReleaseKernel(vxKernel); }
    if(NULL!=*vxNode) { vxReleaseNode(vxNode); }
    if(NULL!=*GraphVX){ vxReleaseGraph(GraphVX); }
    if(NULL!=*ContextVX){ vxReleaseContext(ContextVX); }

    return 0;
}

vx_status VX_CALLBACK vxcAbsDiffValidateInput(vx_node node, vx_uint32 index)
{
    vx_image     imgObj = NULL;
    vx_df_image  imgFmt = VX_DF_IMAGE_U8;
    vx_parameter paramObj = NULL;
    vx_status    status = VX_ERROR_INVALID_PARAMETERS;

//    if(0!=index && 1!=index) return VX_ERROR_INVALID_PARAMETERS;

    paramObj=vxGetParameterByIndex(node, index);
    if(NULL==paramObj) goto OnError;
if(index < 2){
    if(VX_SUCCESS!=vxQueryParameter(paramObj, VX_PARAMETER_REF, &imgObj, sizeof(vx_image)))
        goto OnError;

    if(VX_SUCCESS!=vxQueryImage(imgObj, VX_IMAGE_FORMAT, &imgFmt, sizeof(imgFmt)))
        goto OnError;

    if(VX_DF_IMAGE_U8!=imgFmt)
        goto OnError;
}
    status = VX_SUCCESS;

OnError:
    if (imgObj != NULL) vxReleaseImage(&imgObj);
    if (paramObj != NULL) vxReleaseParameter(&paramObj);

    return status;
}

vx_status VX_CALLBACK vxcAbsDiffValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta)
{
    vx_image     imgObj = NULL;
    vx_df_image  imgFmt = VX_DF_IMAGE_U8;
    vx_uint32    imgWid = 0, imgHei = 0;
    vx_parameter paramObj = NULL;
    vx_status    status = VX_ERROR_INVALID_PARAMETERS;

    if(index<3) return VX_ERROR_INVALID_PARAMETERS;

    paramObj = vxGetParameterByIndex(node, 0);
    if(NULL==paramObj) return VX_ERROR_INVALID_PARAMETERS;
if(0){
    if(VX_SUCCESS!=vxQueryParameter(paramObj, VX_PARAMETER_REF, &imgObj, sizeof(vx_image)))
        goto OnError;

    if(VX_SUCCESS!=vxQueryImage(imgObj, VX_IMAGE_WIDTH, &imgWid, sizeof(imgWid)))
        goto OnError;

    if(VX_SUCCESS!=vxQueryImage(imgObj, VX_IMAGE_HEIGHT, &imgHei, sizeof(imgHei)))
        goto OnError;

    if(VX_SUCCESS!=vxSetMetaFormatAttribute(meta, VX_IMAGE_WIDTH, &imgWid, sizeof(imgWid)))
        goto OnError;

    if(VX_SUCCESS!=vxSetMetaFormatAttribute(meta, VX_IMAGE_HEIGHT, &imgHei, sizeof(imgHei)))
        goto OnError;

    if(VX_SUCCESS!=vxSetMetaFormatAttribute(meta, VX_IMAGE_FORMAT, &imgFmt, sizeof(imgFmt)))
        goto OnError;
}
    status = VX_SUCCESS;

OnError:
    if (imgObj != NULL) vxReleaseImage(&imgObj);
    if (paramObj != NULL) vxReleaseParameter(&paramObj);

    return status;
}
vx_status VX_CALLBACK vxAbsDiffValidator(vx_node node, const vx_reference parameters[], vx_uint32 num, vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;
    vx_uint32 index = 0;
    for(index = 0; index < num; index++)
    {
        if(index < 3)
        {
            status |= vxcAbsDiffValidateInput(node,index);
        }
        else
        {
            status |= vxcAbsDiffValidateOutput(node,index,metas[index]);
        }

    }
    return status;
}
vx_status VX_CALLBACK vxcAbsDiffInitialize(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    /* workdim,    globel offset,    globel scale,    local size,    globel size */
    vx_kernel_execution_parameters_t shaderParam = {2,    {0, 0, 0},    {0, 0, 0},    {0, 0, 0},    {0, 0, 0}};
    vx_int32 imgWid = 0, imgHei = 0;
    vx_image imgObj = (vx_image)parameters[0];

    vxQueryImage(imgObj, VX_IMAGE_WIDTH, &imgWid, sizeof(vx_uint32));
    vxQueryImage(imgObj, VX_IMAGE_HEIGHT, &imgHei, sizeof(vx_uint32));

    shaderParam.globalWorkOffset[0] = 0;
    shaderParam.globalWorkOffset[1] = 0;
    shaderParam.globalWorkScale[0]    = 1;
    shaderParam.globalWorkScale[1]    = 1;
    shaderParam.localWorkSize[0]    = 64;
    shaderParam.localWorkSize[1]    = 1;
    shaderParam.globalWorkSize[0]    = 64;//(((imgWid + shaderParam.globalWorkScale[0] - 1 ) / shaderParam.globalWorkScale[0] + shaderParam.localWorkSize[0] - 1) / shaderParam.localWorkSize[0]) * shaderParam.localWorkSize[0];
    shaderParam.globalWorkSize[1]    = 1;//(imgHei + shaderParam.globalWorkScale[1] - 1) / shaderParam.globalWorkScale[1];


    vxSetNodeAttribute(node, VX_NODE_ATTRIBUTE_KERNEL_EXECUTION_PARAMETERS, &shaderParam, sizeof(vx_kernel_execution_parameters_t));

    return VX_SUCCESS;
}

vx_status vxcAbsDiffRegisterKernel(vx_context context)
{
    vx_status status = VX_FAILURE;
    vx_kernel kernelObj = 0;
    vx_uint32 j = 0, paramCnt = sizeof(paramsAbsDiff)/sizeof(paramsAbsDiff[0]);
    vx_program programObj = 0;
    const vx_char* srcKernel[] = { vxcAbsDiffSource };
    vx_size programLen = strlen(vxcAbsDiffSource);

    programObj = vxCreateProgramWithSource(context, 1, srcKernel, &programLen);
    status = vxBuildProgram(programObj, "-cl-viv-vx-extension");
    if(status != VX_SUCCESS)
    {
        printf("vxBuildProgram error, file:%s,line:%d\n",__FILE__, __LINE__);
        return status;
    }

    kernelObj = vxAddKernelInProgram(
                                  programObj,
                                  VXV_KERNEL_NAME_VXCABSDIFF,
                                  VXV_KERNEL_VXCABSDIFF,
                                  paramCnt,
                                  vxAbsDiffValidator,
                                  vxcAbsDiffInitialize,
                                  NULL
                                 );

    status = vxGetStatus((vx_reference)kernelObj);
    if(status == VX_SUCCESS)
    {

        for(j = 0; j < paramCnt; j++)
        {
            vx_param_description_s paramObj = paramsAbsDiff[j];
            status = vxAddParameterToKernel(kernelObj, j, paramObj.paramDire, paramObj.paramType, paramObj.paramStat);
            if(status != VX_SUCCESS)
            {
                printf("Failed to add parameter %d to kernel %s.\n", j, VXV_KERNEL_NAME_VXCABSDIFF);
                break;
            }
        }
        if(status == VX_SUCCESS)
        {
            status = vxFinalizeKernel(kernelObj);
            if (status!=VX_SUCCESS)
            {
                printf("Failed to finalize kernel[%u]=%s\n", j, VXV_KERNEL_NAME_VXCABSDIFF);

                if(VX_SUCCESS != vxRemoveKernel(kernelObj))
                    printf("Failed to remove kernel[%u]=%s\n", j, VXV_KERNEL_NAME_VXCABSDIFF);
            }
        }
        else
        {
            if(VX_SUCCESS != vxRemoveKernel(kernelObj))
                printf("Failed to remove kernel[%u]=%s\n", j, VXV_KERNEL_NAME_VXCABSDIFF);
        }
    }
    else
    {
        printf("Failed to add kernel %s\n", VXV_KERNEL_NAME_VXCABSDIFF);
    }

    return status;
}

