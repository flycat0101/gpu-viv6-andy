/****************************************************************************
*
*    Copyright 2012 - 2019 Vivante Corporation, Santa Clara, California.
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
#include <VX/vx_helper.h>
#include <VX/vx_compatibility.h>

#include <stdio.h>
#include <math.h>
#include <malloc.h>

#include "tutorial4_vxc.hpp"

#define VX_KERNEL_NAME_TUTORIAL4        "com.vivantecorp.extension.tutorial4VXC"
#define VX_KERNEL_ENUM_TUTORIAL4        100

#define USE_TUTORIAL4_VXC


vx_status VX_CALLBACK vxTutorial4InputValidator(vx_node node, vx_uint32 index)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    vx_parameter param = NULL;

    if(index == 0)
    {
        param = vxGetParameterByIndex(node, index);
        if(param != NULL)
        {
            vx_image img = NULL;
            vx_df_image format = 0;
            status = vxQueryParameter(param, VX_PARAMETER_REF, &img, sizeof(img));
            if(status == VX_SUCCESS)
            {
                status = vxQueryImage(img, VX_IMAGE_FORMAT, &format, sizeof(format));
                if (format == VX_DF_IMAGE_U8)
                    status = VX_SUCCESS;
                else
                    status = VX_ERROR_INVALID_VALUE;
                status |= vxReleaseImage(&img);
            }
            status |= vxReleaseParameter(&param);
        }
    }

    return status;
}
vx_status VX_CALLBACK vxTutorial4OutputValidator(vx_node node, vx_uint32 index, vx_meta_format metaObj)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    vx_parameter param = NULL;

    if(index == 1)
    {
        param = vxGetParameterByIndex(node, index);
        if(param != NULL)
        {
            vx_image img = NULL;
            vx_uint32 width = 0, height = 0;
            vx_df_image format = VX_DF_IMAGE_U8;
            status = vxQueryParameter(param, VX_PARAMETER_REF, &img, sizeof(img));
            if(status == VX_SUCCESS)
            {
                status = vxQueryImage(img, VX_IMAGE_WIDTH, &width, sizeof(width));
                status |= vxQueryImage(img, VX_IMAGE_HEIGHT, &height, sizeof(height));
                status |= vxSetMetaFormatAttribute(metaObj, VX_IMAGE_FORMAT, &format, sizeof(format));
                status |= vxSetMetaFormatAttribute(metaObj, VX_IMAGE_WIDTH, &width, sizeof(width));
                status |= vxSetMetaFormatAttribute(metaObj, VX_IMAGE_HEIGHT, &height, sizeof(height));
                status |= vxReleaseImage(&img);
            }
            status |= vxReleaseParameter(&param);
        }
    }

    return status;
}
vx_status VX_CALLBACK vxTutorial4Validator(vx_node node, const vx_reference parameters[], vx_uint32 num, vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;
    vx_uint32 index = 0;
    for(index = 0; index < num; index++)
    {
        if(index < 1)
        {
            status |= vxTutorial4InputValidator(node,index);
        }
        else
        {
            status |= vxTutorial4OutputValidator(node,index,metas[index]);
        }

    }
    return status;
}

#ifndef USE_TUTORIAL4_VXC

#define CLIP_MIN_MAX(p, MIN, MAX)   (((p) < MIN) ? MIN : ( (p) > (MAX) ? (MAX) : (p)))
void paddingCheck(int x, int y, int *out_x, int *out_y, int W, int H)
{
    *out_x = x;  *out_y = y;

    if(*out_x < 0)  *out_x = 0;
    if(*out_y < 0)  *out_y = 0;

    if(*out_x >= W) *out_x = W - 1;
    if(*out_y >= H) *out_y = H - 1;
}
void tutorial4Thread2x4(unsigned char* bayer, unsigned char*rgb, int width, int height, int x, int y)
{
    int kA[4] = {-2, -3,  1, -2};
    int kB[4] = {2,  0,  0,  4};
    int kC[4] = {4,  6,  5,  5};
    int kD[4] = {0,  2, -1, -1};
    int kE[4] = {-2, -3, -2,  1};
    int kF[4] = {2,  0,  4, 0};
    int i, j, k, l, piColor;
    int tmp_x, tmp_y;
    int result_G, result_R, result_B;
    int p[5][5];
    int bitDepth = 8;
    int maxI = (int)(pow(2.0, bitDepth)-1);
    int A,B,C,D,E,F,PATTERN[4];
    int ii, jj;

    for( jj = 0; jj < 2; jj++)
    {
        j = y + jj;
        for(ii = 0; ii < 4; ii++)
        {
            i = x + ii;
            piColor = 2*(j&1) + (i&1);

            //fill the data into 5X5 array
            for(l = -2; l <=2; l++)
            {
                for(k = -2; k <=2; k++)
                {
                    paddingCheck(i+k, j+l, &tmp_x, &tmp_y, width, height);
                    p[l+2][k+2] =(int) ((unsigned char*)bayer + width*tmp_y)[tmp_x];
                }

                A = p[0][2] + p[4][2];
                B = p[1][2] + p[3][2];
                C = p[2][2];
                D = p[1][1] + p[1][3] + p[3][1] + p[3][3];
                E = p[2][0] + p[2][4];
                F = p[2][1] + p[2][3];

                //PATTERN = (kC.xyz * C).xyzz
                PATTERN[0] = kC[0]*C * 2;
                PATTERN[1] = kC[1]*C * 2;
                PATTERN[3] = PATTERN[2] = kC[2]*C * 2;

                //PATTERN.yzw += (kD.yz * D).xyy
                PATTERN[1] += kD[1]*D * 2;
                PATTERN[2] += kD[2]*D * 2;
                PATTERN[3] += kD[2]*D * 2;

                //PATTERN += (kA.xyz * A).xyzx + (kE.xyw * E).xyxz;
                PATTERN[0] += ((kA[0]*A + kE[0]*E));
                PATTERN[1] += ((kA[1]*A + kE[1]*E));
                PATTERN[2] += ((kA[2]*A + kE[0]*E));
                PATTERN[3] += ((kA[0]*A + kE[3]*E));

                //PATTERN.xw  += kB.xw * B
                PATTERN[0] += kB[0]*B * 2;
                PATTERN[3] += kB[3]*B * 2;

                //PATTERN.xz  += kF.xz * F
                PATTERN[0] += kF[0]*F * 2;
                PATTERN[2] += kF[2]*F * 2;

                if (piColor==0)
                {
                    result_R = C; result_G = PATTERN[0]>>4; result_B = PATTERN[1]>>4;
                }
                else if (piColor==1)
                {
                    result_R = PATTERN[2]>>4; result_G = C; result_B = PATTERN[3]>>4;
                }
                else if (piColor==2)
                {
                    result_R = PATTERN[3]>>4; result_G = C; result_B = PATTERN[2]>>4;
                }
                else if (piColor==3)
                {
                    result_R = PATTERN[1]>>4; result_G = PATTERN[0]>>4; result_B = C;
                }

                (rgb + 3*width*j)[i*3]   =  CLIP_MIN_MAX(result_B, 0, maxI);  // B
                (rgb + 3*width*j)[i*3+1] =  CLIP_MIN_MAX(result_G, 0, maxI);  // G
                (rgb + 3*width*j)[i*3+2] =  CLIP_MIN_MAX(result_R, 0, maxI);  // R
            }
        }
    }
}
void tutorial4Filter(unsigned char* bayer, unsigned char*rgb, int width, int height)
{
    for(int j = 0; j < height; j+=2)
    {
        for(int i = 0; i < width; i+=4)
        {
            tutorial4Thread2x4(bayer, rgb, width, height, i, j);
        }
    }
}

vx_status VX_CALLBACK vxTutorial4Kernel(vx_node node, const vx_reference* paramObj, vx_uint32 paramNum)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;

    if(paramNum == 2)
    {
        status = VX_SUCCESS;
        vx_image imgObj[2] = {NULL, NULL};
        void* imgBaseAddr[2] = {NULL, NULL};
        vx_rectangle_t imgRect[2] = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }};
        vx_imagepatch_addressing_t imgInfo[2] = {VX_IMAGEPATCH_ADDR_INIT, VX_IMAGEPATCH_ADDR_INIT};
        vx_uint32 width = 0, height = 0;
        vx_map_id map_id,map_id1;
        imgObj[0]    = (vx_image)paramObj[0];
        imgObj[1]    = (vx_image)paramObj[1];
        vxQueryImage(imgObj[0], VX_IMAGE_WIDTH, &width, sizeof(width));
        vxQueryImage(imgObj[0], VX_IMAGE_HEIGHT, &height, sizeof(height));
        status |= vxGetValidRegionImage(imgObj[0], &imgRect[0]);
        status |= vxMapImagePatch(imgObj[0], &imgRect[0], 0, &map_id,&imgInfo[0], &imgBaseAddr[0], VX_READ_ONLY,VX_MEMORY_TYPE_HOST,0);
        status |= vxGetValidRegionImage(imgObj[1], &imgRect[1]);
        status |= vxMapImagePatch(imgObj[1], &imgRect[1], 0,&map_id1, &imgInfo[1], &imgBaseAddr[1], VX_WRITE_ONLY,VX_MEMORY_TYPE_HOST,0);

        // simulate the execution of GPU
        tutorial4Filter((unsigned char*)imgBaseAddr[0], (unsigned char*)imgBaseAddr[1], width, height);

        status |= vxUnmapImagePatch(imgObj[0], map_id);
        status |= vxUnmapImagePatch(imgObj[1], map_id1);
    }

    return status;
}
#endif

vx_status VX_CALLBACK vxTutorial4Initializer(vx_node nodObj, const vx_reference *paramObj, vx_uint32 paraNum)
{
#ifdef USE_TUTORIAL4_VXC
    vx_kernel_execution_parameters_t shaderParam = {
        2,          // workdim
        {0, 0, 0},  // globalWorkOffset: control the start location be processed in the image
        {0, 0, 0},  // globalWorkScale: how many pixels could be processed by a single thread
        {0, 0, 0},  // localWorkSize: local group size in threads
        {0, 0, 0}}; // globalWorkSize: image size in threads

    vx_int32 imgWid = 0, imgHei = 0;
    vx_image imgObj = (vx_image)paramObj[0];

    vxQueryImage(imgObj, VX_IMAGE_WIDTH, &imgWid, sizeof(vx_uint32));
    vxQueryImage(imgObj, VX_IMAGE_HEIGHT, &imgHei, sizeof(vx_uint32));

    shaderParam.globalWorkOffset[0] = 0;
    shaderParam.globalWorkOffset[1] = 0;
    shaderParam.globalWorkScale[0]  = 4;
    shaderParam.globalWorkScale[1]  = 2;
    shaderParam.localWorkSize[0]    = 8;
    shaderParam.localWorkSize[1]    = 1;
    shaderParam.globalWorkSize[0]   = (((imgWid + shaderParam.globalWorkScale[0] - 1) / shaderParam.globalWorkScale[0] + shaderParam.localWorkSize[0] - 1) / shaderParam.localWorkSize[0]) * shaderParam.localWorkSize[0];
    shaderParam.globalWorkSize[1]   = (imgHei + shaderParam.globalWorkScale[1] - 1) / shaderParam.globalWorkScale[1];

    // uniforms
    vx_uint32 uni4x8_r1_r2[16] = {
        0x9a291606, 0x9a291606, // TCfg
        0xe2200242, 0x30524304, 0x0284a4e4, 0x85056640, 0xb568505a, // BinSelect
        0x00007400, // AccumType, ConstantType, and PostShift
        0x00000402, 0x00040403, 0x00020201, 0x02080202, 0x00000402, 0x00040403, 0x00020201, 0x02080202 // Constant
    };
    vx_uint32 uni4x8_r3_r3[16] = {
        0x37952595, 0x37952595, // TCfg
        0x04020c20, 0x12904101, 0x56720146, 0x9305a92b, 0x05eb3bda, // BinSelect
        0x00007400, // AccumType, ConstantType, and PostShift
        0x020404fe, 0x00030cfd, 0x020808fe, 0x00000a00, 0x020404fe, 0x00030cfd, 0x020808fe, 0x00000a00 // Constant
    };
    vx_uint32 uni4x8_r2_r3[16] = {
        0x05152b99, 0x05152b99, // TCfg
        0xe229ca22, 0x30524304, 0xd2640026, 0x8505664a, 0x002a505a, // BinSelect
        0x00007400, // AccumType, ConstantType, and PostShift
        0x020802fe, 0x00020200, 0x000404fd, 0x000004fe, 0x020802fe, 0x00020200, 0x000404fd, 0x000004fe // Constant
    };
    vx_uint32 uni4x8_r4_r4[16] = {
        0x95259537, 0x95259537, // TCfg
        0xc2001040, 0x10146120, 0x5a922904, 0xb3b56720, 0xbda9305e, // BinSelect
        0x00007400, // AccumType, ConstantType, and PostShift
        0x00000a00, 0x020808fe, 0x00030cfd, 0x020404fe, 0x00000a00, 0x020808fe, 0x00030cfd, 0x020404fe // Constant
    };
    vx_uint32 uni2x8_r3_d3_0[16] = {
        0x73713173, // TCfg
        0x11101011, // ASelt
        0x03022001, 0x44053203, // ABin
        0x80820280, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00003404, // AccumType, ConstantType, and PostShift
        0x00000000, 0x00080000, 0x00000010, 0x00000000, 0x00000010, 0x000a0000, 0x00000000, 0x00080000 // Constant
    };
    vx_uint32 uni2x8_r3_d3_1[16] = {
        0x00007131, // TCfg
        0x00001010, // ASelt
        0x56050704, 0x00000000, // ABin
        0x00008202, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00003404, // AccumType, ConstantType, and PostShift
        0x00000010, 0x00000000, 0x00000010, 0x000a0000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
    };
     vx_uint32 uni2x8_r4_d7_0[16] = {
        0x17371317, // TCfg
        0x01110101, // ASelt
        0x03000221, 0x04450233, // ABin
        0x28082028, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00003404, // AccumType, ConstantType, and PostShift
        0x000a0000, 0x00000010, 0x00000000, 0x00000010, 0x00080000, 0x00000000, 0x000a0000, 0x00000010 // Constant
    };
    vx_uint32 uni2x8_r4_d7_1[16] = {
        0x00003713, // TCfg
        0x00001101, // ASelt
        0x06570504, 0x00000000, // ABin
        0x00000820, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00003404, // AccumType, ConstantType, and PostShift
        0x00000000, 0x00000010, 0x00080000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
    };

    vxSetNodeUniform(nodObj, "EE_EO_AccTwoLines", 1, uni4x8_r1_r2);
    vxSetNodeUniform(nodObj, "EE_EO_AccOneLine", 1, uni4x8_r3_r3);
    vxSetNodeUniform(nodObj, "EE_EO_PackBGR_0", 1, uni2x8_r3_d3_0);
    vxSetNodeUniform(nodObj, "EE_EO_PackBGR_1", 1, uni2x8_r3_d3_1);

    vxSetNodeUniform(nodObj, "EO_OO_AccTwoLines", 1, uni4x8_r2_r3);
    vxSetNodeUniform(nodObj, "EO_OO_AccOneLine", 1, uni4x8_r4_r4);
    vxSetNodeUniform(nodObj, "EO_OO_PackBGR_0", 1, uni2x8_r4_d7_0);
    vxSetNodeUniform(nodObj, "EO_OO_PackBGR_1", 1, uni2x8_r4_d7_1);

    // Node attribute setting
    vxSetNodeAttribute(nodObj, VX_NODE_ATTRIBUTE_KERNEL_EXECUTION_PARAMETERS, &shaderParam, sizeof(vx_kernel_execution_parameters_t));
#endif
    return VX_SUCCESS;
}

vx_status VX_CALLBACK vxTutorial4Deinitializer(vx_node nodObj, const vx_reference *paraObj, vx_uint32 paraNum)
{
    return VX_SUCCESS;
}



static vx_param_description_t vxTutorial4KernelParam[] =
{
    // inputs
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    // outputs
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

vx_kernel_description_t vxTutorial4KernelInfo =
{
    VX_KERNEL_ENUM_TUTORIAL4,
    VX_KERNEL_NAME_TUTORIAL4,
#ifdef USE_TUTORIAL4_VXC
    NULL,
#else
    vxTutorial4Kernel,
#endif
    vxTutorial4KernelParam,
    (sizeof(vxTutorial4KernelParam)/sizeof(vxTutorial4KernelParam[0])),
    vxTutorial4Validator,
    NULL,
    NULL,
    vxTutorial4Initializer,
    vxTutorial4Deinitializer
};

static vx_kernel_description_t* kernels[] =
{
    &vxTutorial4KernelInfo
};

extern "C" vx_status VX_API_CALL vxPublishKernels(vx_context ContextVX)
{
    vx_status status = VX_FAILURE;
    vx_kernel kernelObj = NULL;

#ifdef USE_TUTORIAL4_VXC
    vx_program programObj = NULL;
    const vx_char* programSrc[] = { vxcTutorial4Source };
    vx_size programLen = strlen(vxcTutorial4Source);

    programObj = vxCreateProgramWithSource(ContextVX, 1, programSrc, &programLen);
    status = vxBuildProgram(programObj, "-cl-viv-vx-extension");
    if (status != VX_SUCCESS)
    {
        printf("vxBuildProgram error, file:%s,line:%d\n",__FILE__, __LINE__);
        return status;
    }

    for(int i=0; i < (int)(sizeof(kernels)/sizeof(kernels[0])); i++)
    {
        kernelObj = vxAddKernelInProgram(programObj,
            kernels[i]->name,
            kernels[i]->enumeration,
            kernels[i]->numParams,
            kernels[i]->validate,
            kernels[i]->initialize,
            kernels[i]->deinitialize
            );
        status = vxGetStatus((vx_reference)kernelObj);
        if(status == VX_SUCCESS)
        {
            status = VX_SUCCESS;
            for(int j=0; j < (int)kernels[i]->numParams; j++)
            {
                status = vxAddParameterToKernel(kernelObj,
                    j,
                    kernels[i]->parameters[j].direction,
                    kernels[i]->parameters[j].data_type,
                    kernels[i]->parameters[j].state
                    );
                if(status != VX_SUCCESS)
                {
                    printf("Failed to add parameter %d to kernel %s.\n", j, kernels[i]->name);
                    break;
                }
            }

            if(status == VX_SUCCESS)
            {
                status = vxFinalizeKernel(kernelObj);
                if (status!=VX_SUCCESS)
                {
                    printf("Failed to finalize kernel[%u]=%s\n",i, kernels[i]->name);

                    if(VX_SUCCESS != vxRemoveKernel(kernelObj))
                        printf("Failed to remove kernel[%u]=%s\n",i, kernels[i]->name);
                }
            }
            else
            {
                if(VX_SUCCESS != vxRemoveKernel(kernelObj))
                    printf("Failed to remove kernel[%u]=%s\n",i, kernels[i]->name);
            }
        }
        else
        {
            printf("Failed to add kernel %s\n", kernels[i]->name);
        }
    }
#else
    for(int i=0; i < (int)(sizeof(kernels)/sizeof(kernels[0])); i++)
    {
        kernelObj = vxAddUserKernel(ContextVX,
            kernels[i]->name,
            kernels[i]->enumeration,
            kernels[i]->function,
            kernels[i]->numParams,
            kernels[i]->validate,
            kernels[i]->initialize,
            kernels[i]->deinitialize
            );
        status = vxGetStatus((vx_reference)kernelObj);
        if(status == VX_SUCCESS)
        {
            status = VX_SUCCESS;
            for(int j=0; j < (int)kernels[i]->numParams; j++)
            {
                status = vxAddParameterToKernel(kernelObj,
                    j,
                    kernels[i]->parameters[j].direction,
                    kernels[i]->parameters[j].data_type,
                    kernels[i]->parameters[j].state
                    );
                if(status != VX_SUCCESS)
                {
                    printf("Failed to add parameter %d to kernel %s.\n", j, kernels[i]->name);
                    break;
                }
            }

            if(VX_SUCCESS == status)
            {
                status = vxFinalizeKernel(kernelObj);
                if (status!=VX_SUCCESS)
                {
                    printf("Failed to finalize kernel[%u]=%s\n",i, kernels[i]->name);

                    if(VX_SUCCESS != vxRemoveKernel(kernelObj))
                        printf("Failed to remove kernel[%u]=%s\n",i, kernels[i]->name);
                }
            }
            else
            {
                if(VX_SUCCESS != vxRemoveKernel(kernelObj))
                    printf("Failed to remove kernel[%u]=%s\n",i, kernels[i]->name);
            }
        }
        else
        {
            printf("Failed to add kernel %s\n", kernels[i]->name);
        }
    }
#endif
    return status;
}
