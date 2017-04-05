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

char vxcAbsDiffSource[] = {
    "#include \"cl_viv_vx_ext.h\" \n\
\n\
    _viv_uniform int height; \n\
    __kernel void vxcAbsDiff \n\
    ( \n\
    __read_only image2d_t     in0_image, \n\
    __read_only image2d_t     in1_image, \n\
    __write_only image2d_t     out_image \n\
    ) \n\
    { \n\
    int2 coord = (int2)(get_global_id(0), get_global_id(1)); \n\
\n\
    vxc_uchar16 reg0, reg1, reg2, reg3; \n\
    vxc_uchar16 reg4, reg5, reg6, reg7; \n\
 \n\
    VXC_ReadImage(reg0, in0_image, coord, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
    VXC_ReadImage(reg1, in1_image, coord, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
    VXC_ReadImage(reg2, in0_image, coord, VXC_5BITOFFSET_XY(0, 1), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
    VXC_ReadImage(reg3, in1_image, coord, VXC_5BITOFFSET_XY(0, 1), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
    VXC_ReadImage(reg4, in0_image, coord, VXC_5BITOFFSET_XY(0, 2), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
    VXC_ReadImage(reg5, in1_image, coord, VXC_5BITOFFSET_XY(0, 2), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
    VXC_ReadImage(reg6, in0_image, coord, VXC_5BITOFFSET_XY(0, 3), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
    VXC_ReadImage(reg7, in1_image, coord, VXC_5BITOFFSET_XY(0, 3), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
    VXC_AbsDiff(reg0, reg0, reg1, VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
    VXC_AbsDiff(reg1, reg2, reg3, VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
    VXC_AbsDiff(reg2, reg4, reg5, VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
    VXC_AbsDiff(reg3, reg6, reg7, VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
    VXC_WriteImage(out_image, coord, reg0, VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
    coord.y += 1; \n\
    VXC_WriteImage(out_image, coord, reg1, VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
    coord.y += 1; \n\
    VXC_WriteImage(out_image, coord, reg2, VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
    coord.y += 1; \n\
    VXC_WriteImage(out_image, coord, reg3, VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
}"
};

static vx_param_description_s paramsAbsDiff[] =
{
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED}
};

int freeRes(FILE** srcFile0, FILE** srcFile1, FILE** destFile, u08** tmpBuf, u08** tmpBuf2, u08** imgBuf, vx_image imgObj[], vx_kernel* vxKernel, vx_node* vxNode, vx_graph* GraphVX, vx_context* ContextVX);
vx_status VX_CALLBACK vxcAbsDiffValidateInput(vx_node node, vx_uint32 index);
vx_status VX_CALLBACK vxcAbsDiffValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta);
vx_status VX_CALLBACK vxcAbsDiffInitialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_status vxcAbsDiffRegisterKernel(vx_context context);

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

    /* read image data */
    fFile0 = fopen("left_gray.bmp", "rb");
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
        FILE *fIn = i == 0 ? fFile0 : fFile1;
        fileSize = sizeof(FileHeader) + sizeof(InfoHeader);
        if(tmpBuf == NULL)
            tmpBuf = (u08*)malloc(fileSize);
        if(NULL==tmpBuf)
        {
            printf("%s:%d, %s\n", __FILE__, __LINE__, "memory error.");

            status = -1;
            goto exit;
        }
        fread((u08*)tmpBuf, fileSize, 1, fIn);

        fileOffset = ((((FileHeader*)tmpBuf)->fileOffset2) << 16) + (((FileHeader*)tmpBuf)->fileOffset1);
        imgHei = ((InfoHeader*)(tmpBuf + sizeof(FileHeader)))->imageHeight;
        imgHei = (vx_int32)imgHei < 0 ? (~imgHei + 1) : imgHei;
        imgWid = ((InfoHeader*)(tmpBuf + sizeof(FileHeader)))->imageWidth;

        if(tmpBuf2 == NULL)
            tmpBuf2 = (u08*)malloc(fileOffset-fileSize);
        if(NULL==tmpBuf2)
        {
            printf("%s:%d, %s\n", __FILE__, __LINE__, "memory error.");

            status = -1;
            goto exit;
        }
        fread((u08*)tmpBuf2, (fileOffset-fileSize), 1, fIn);

        imgBuf[i] = (u08*)malloc(imgHei*imgWid*sizeof(u08));
        if(NULL==imgBuf[i])
        {
            printf("%s:%d, %s\n", __FILE__, __LINE__, "memory error.");

            status = -1;
            goto exit;
        }

        fread((u08*)imgBuf[i], imgHei*imgWid*sizeof(u08), 1, fIn);
    }


    /* vx procedure */
    ContextVX = vxCreateContext();
    if(NULL!=ContextVX)
    {
        vxcAbsDiffRegisterKernel(ContextVX);
        GraphVX = vxCreateGraph(ContextVX);
        if(NULL!=GraphVX)
        {
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
            status = vxSetParameterByIndex(vxNode, 0, (vx_reference)imgObj[0]);
            status |= vxSetParameterByIndex(vxNode, 1, (vx_reference)imgObj[1]);
            status |= vxSetParameterByIndex(vxNode, 2, (vx_reference)imgObj[2]);
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
        status = vxAccessImagePatch(imgObj[i], &imgRect, 0, &imgInfo[i], &imgAddr[i], VX_WRITE_ONLY);
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

        status = vxCommitImagePatch(imgObj[i], &imgRect, 0, &(imgInfo[i]), imgAddr[i]);
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
    status = vxAccessImagePatch(imgObj[2], &imgRect, 0, &(imgInfo[2]), &imgAddr[2], VX_READ_ONLY);
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

    status = vxCommitImagePatch(imgObj[2], NULL, 0, &(imgInfo[2]), imgAddr[2]);
    if(status != VX_SUCCESS)
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");
        goto exit;
    }
    imgAddr[2] = NULL;

    /* output image */
    fOut = fopen("absdiff_out.bmp", "wb");
    if(NULL==fOut)
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "file error.");
        status = -1;
        goto exit;
    }

    fwrite(tmpBuf, fileSize, 1, fOut);
    fwrite(tmpBuf2, (fileOffset-fileSize), 1, fOut);
    fwrite(imgBuf[0], imgHei*imgWid*sizeof(u08), 1, fOut);

    status = VX_SUCCESS;
    printf("vx process success.\n");
    /* exit and free */
exit:
    freeRes(&fFile0, &fFile1, &fOut, &tmpBuf, &tmpBuf2, imgBuf, imgObj, &vxKernel, &vxNode, &GraphVX, &ContextVX);
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

    if(0!=index && 1!=index) return VX_ERROR_INVALID_PARAMETERS;

    paramObj=vxGetParameterByIndex(node, index);
    if(NULL==paramObj) goto OnError;

    if(VX_SUCCESS!=vxQueryParameter(paramObj, VX_PARAMETER_ATTRIBUTE_REF, &imgObj, sizeof(vx_image)))
        goto OnError;

    if(VX_SUCCESS!=vxQueryImage(imgObj, VX_IMAGE_ATTRIBUTE_FORMAT, &imgFmt, sizeof(imgFmt)))
        goto OnError;

    if(VX_DF_IMAGE_U8!=imgFmt)
        goto OnError;

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

    if(2!=index) return VX_ERROR_INVALID_PARAMETERS;

    paramObj = vxGetParameterByIndex(node, 0);
    if(NULL==paramObj) return VX_ERROR_INVALID_PARAMETERS;

    if(VX_SUCCESS!=vxQueryParameter(paramObj, VX_PARAMETER_ATTRIBUTE_REF, &imgObj, sizeof(vx_image)))
        goto OnError;

    if(VX_SUCCESS!=vxQueryImage(imgObj, VX_IMAGE_ATTRIBUTE_WIDTH, &imgWid, sizeof(imgWid)))
        goto OnError;

    if(VX_SUCCESS!=vxQueryImage(imgObj, VX_IMAGE_ATTRIBUTE_HEIGHT, &imgHei, sizeof(imgHei)))
        goto OnError;

    if(VX_SUCCESS!=vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_WIDTH, &imgWid, sizeof(imgWid)))
        goto OnError;

    if(VX_SUCCESS!=vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_HEIGHT, &imgHei, sizeof(imgHei)))
        goto OnError;

    if(VX_SUCCESS!=vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_FORMAT, &imgFmt, sizeof(imgFmt)))
        goto OnError;

    status = VX_SUCCESS;

OnError:
    if (imgObj != NULL) vxReleaseImage(&imgObj);
    if (paramObj != NULL) vxReleaseParameter(&paramObj);

    return status;
}

vx_status VX_CALLBACK vxcAbsDiffInitialize(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    /* workdim,    globel offset,    globel scale,    local size,    globel size */
    vx_kernel_execution_parameters_t shaderParam = {2,    {0, 0, 0},    {0, 0, 0},    {0, 0, 0},    {0, 0, 0}};
    vx_int32 imgWid = 0, imgHei = 0;
    vx_image imgObj = (vx_image)parameters[0];

    vxQueryImage(imgObj, VX_IMAGE_ATTRIBUTE_WIDTH, &imgWid, sizeof(vx_uint32));
    vxQueryImage(imgObj, VX_IMAGE_ATTRIBUTE_HEIGHT, &imgHei, sizeof(vx_uint32));

    shaderParam.globalWorkOffset[0] = 0;
    shaderParam.globalWorkOffset[1] = 0;
    shaderParam.globalWorkScale[0]    = 16;
    shaderParam.globalWorkScale[1]    = 4;
    shaderParam.localWorkSize[0]    = 8;
    shaderParam.localWorkSize[1]    = 1;
    shaderParam.globalWorkSize[0]    = (((imgWid + shaderParam.globalWorkScale[0] - 1 ) / shaderParam.globalWorkScale[0] + shaderParam.localWorkSize[0] - 1) / shaderParam.localWorkSize[0]) * shaderParam.localWorkSize[0];
    shaderParam.globalWorkSize[1]    = (imgHei + shaderParam.globalWorkScale[1] - 1) / shaderParam.globalWorkScale[1];


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
                                  vxcAbsDiffValidateInput,
                                  vxcAbsDiffValidateOutput,
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

