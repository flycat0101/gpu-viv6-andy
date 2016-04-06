/****************************************************************************
*
*    Copyright 2012 - 2015 Vivante Corporation, Santa Clara, California.
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

#define VXV_KERNEL_NAME_GAUSSIAN3x3 "com.vivantecorp.extension.gaussian3x3"
#define VXV_KERNEL_GAUSSIAN3x3      100

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

char vxcGaussian3x3Source[] = {
    "#include \"cl_viv_vx_ext.h\" \n\
\n\
    _viv_uniform int height; \n\
    __kernel void gaussian3x3 \n\
    ( \n\
        __read_only image2d_t in_image, \n\
        __write_only image2d_t out_image \n\
    ) \n\
    { \n\
        int2 coord = (int2)(get_global_id(0), get_global_id(1)); \n\
\n\
        int2 coord1 = coord + (int2)(-1, -1); \n\
        vxc_uchar16 v0 = viv_intrinsic_vx_read_imageuc(in_image, coord1); \n\
\n\
        int2 coord2 = coord + (int2)(-1, 0); \n\
        vxc_uchar16 v1 = viv_intrinsic_vx_read_imageuc(in_image, coord2); \n\
\n\
        int info = VXC_MODIFIER(0, 13, 0, VXC_RM_TowardZero, 0); \n\
        int infox = VXC_MODIFIER_FILTER(0, 13, 0, VXC_FM_Guassian, 0); \n\
\n\
        do \n\
        { \n\
            int2 coord3 = coord + (int2)(-1, 1); \n\
            vxc_uchar16 v2 = viv_intrinsic_vx_read_imageuc(in_image, coord3); \n\
\n\
            vxc_uchar16 max = viv_intrinsic_vxmc_Filter_uc(v0, v1, v2, infox); \n\
            viv_intrinsic_vxmc_write_imageuc(out_image, coord, max, info); \n\
\n\
            v0 = v1; \n\
            v1 = v2; \n\
            coord.y++; \n\
        } \n\
        while(coord.y<height); \n\
    }"
};

static vx_param_description_s paramsGaussian3x3[] =
{
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED}
};

int freeRes(FILE** srcFile, FILE** destFile, u08** tmpBuf, u08** tmpBuf2, u08** imgBuf, vx_image imgObj[], vx_kernel* vxKernel, vx_node* vxNode, vx_graph* GraphVX, vx_context* ContextVX);
vx_status vxcGaussian3x3ValidateInput(vx_node node, vx_uint32 index);
vx_status vxcGaussian3x3ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta);
vx_status vxcGaussian3x3Initialize(vx_node node, vx_reference *parameters, vx_uint32 num);
vx_status vxcGaussian3x3RegisterKernel(vx_context context);

int main(int argc, char* argv[])
{
    /* VX variables */
    vx_context ContextVX = NULL;
    vx_graph GraphVX = NULL;
    vx_node vxNode = NULL;
    vx_kernel vxKernel = NULL;

    vx_image imgObj[2] = { NULL, NULL };
    vx_imagepatch_addressing_t imgInfo[2] = { VX_IMAGEPATCH_ADDR_INIT, VX_IMAGEPATCH_ADDR_INIT };
    void* imgAddr[2] = { NULL, NULL };
    void* imgPixel[2] = { NULL, NULL };
    vx_rectangle_t imgRect = { 0, 0, 0, 0 };
    vx_uint32 imgWid = 0, imgHei = 0;

    FILE* srcFile = NULL;
    FILE* destFile = NULL;
    u08* tmpBuf = NULL;
    u08* tmpBuf2 = NULL;
    u08* imgBuf = NULL;
    u32 fileSize = 0;
    u32 fileOffset = 0;
    u32 x = 0, y = 0;

    /* read image data */
    srcFile = fopen("lena_gray.bmp", "rb");
    if(NULL==srcFile)
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "file error.");


        freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, imgObj, &vxKernel, &vxNode, &GraphVX, &ContextVX);
        exit(-1);
    }

    fileSize = sizeof(FileHeader) + sizeof(InfoHeader);

    tmpBuf = (u08*)malloc(fileSize);
    if(NULL==tmpBuf)
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "memory error.");

        freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, imgObj, &vxKernel, &vxNode, &GraphVX, &ContextVX);
        exit(-1);
    }
    fread((u08*)tmpBuf, fileSize, 1, srcFile);

    fileOffset = ((((FileHeader*)tmpBuf)->fileOffset2) << 16) + (((FileHeader*)tmpBuf)->fileOffset1);
    imgHei = ((InfoHeader*)(tmpBuf + sizeof(FileHeader)))->imageHeight;
    imgHei = (vx_int32)imgHei < 0 ? (~imgHei + 1) : imgHei;
    imgWid = ((InfoHeader*)(tmpBuf + sizeof(FileHeader)))->imageWidth;

    tmpBuf2 = (u08*)malloc(fileOffset-fileSize);
    if(NULL==tmpBuf2)
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "memory error.");

        freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, imgObj, &vxKernel, &vxNode, &GraphVX, &ContextVX);
        exit(-1);
    }
    fread((u08*)tmpBuf2, (fileOffset-fileSize), 1, srcFile);

    imgBuf = (u08*)malloc(imgHei*imgWid*sizeof(u08));
    if(NULL==imgBuf)
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "memory error.");

        freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, imgObj, &vxKernel, &vxNode, &GraphVX, &ContextVX);
        exit(-1);
    }
    fread((u08*)imgBuf, imgHei*imgWid*sizeof(u08), 1, srcFile);

    /* vx procedure */
    ContextVX = vxCreateContext();
    if(NULL!=ContextVX)
    {
        vxcGaussian3x3RegisterKernel(ContextVX);
        GraphVX = vxCreateGraph(ContextVX);
        if(NULL!=GraphVX)
        {
            imgObj[0] = vxCreateImage(ContextVX, imgWid, imgHei, VX_DF_IMAGE_U8);
            imgObj[1] = vxCreateImage(ContextVX, imgWid, imgHei, VX_DF_IMAGE_U8);

            imgRect.start_x = 0;
            imgRect.start_y = 0;
            imgRect.end_x = imgWid;
            imgRect.end_y = imgHei;

            vxKernel = vxGetKernelByName(ContextVX, VXV_KERNEL_NAME_GAUSSIAN3x3);
            vxNode = vxCreateGenericNode(GraphVX, vxKernel);
            vxSetParameterByIndex(vxNode, 0, (vx_reference)imgObj[0]);
            vxSetParameterByIndex(vxNode, 1, (vx_reference)imgObj[1]);

            if(VX_SUCCESS!=vxVerifyGraph(GraphVX))
            {
                printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");

                freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, imgObj, &vxKernel, &vxNode, &GraphVX, &ContextVX);
                exit(-1);
            }
        }
        else
        {
            printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");

            freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, imgObj, &vxKernel, &vxNode, &GraphVX, &ContextVX);
            exit(-1);
        }
    }
    else
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");

        freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, imgObj, &vxKernel, &vxNode, &GraphVX, &ContextVX);
        exit(-1);
    }

    /* transfer image from cpu to gpu */
    if(VX_SUCCESS!=vxAccessImagePatch(imgObj[0], &imgRect, 0, &imgInfo[0], &imgAddr[0], VX_WRITE_ONLY))
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");

        freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, imgObj, &vxKernel, &vxNode, &GraphVX, &ContextVX);
        exit(-1);
    }

    if((1!=imgInfo[0].step_y)||(1!=imgInfo[0].step_x)||(imgHei!=imgInfo[0].dim_y)||(imgWid!=imgInfo[0].dim_x))
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");

        freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, imgObj, &vxKernel, &vxNode, &GraphVX, &ContextVX);
        exit(-1);
    }

    for(y=0; y<imgHei; y++)
    {
        for (x=0; x<imgWid; x++)
        {
            imgPixel[0] = vxFormatImagePatchAddress2d(imgAddr[0], x, y, &(imgInfo[0]));
            *((vx_uint8*)(imgPixel[0])) = imgBuf[y*imgWid+x];
        }
    }

    if(VX_SUCCESS!=vxCommitImagePatch(imgObj[0], &imgRect, 0, &(imgInfo[0]), imgAddr[0]))
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");

        freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, imgObj, &vxKernel, &vxNode, &GraphVX, &ContextVX);
        exit(-1);
    }
    imgAddr[0] = NULL;

    /* process image */
    if(VX_SUCCESS!=vxProcessGraph(GraphVX))
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");

        freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, imgObj, &vxKernel, &vxNode, &GraphVX, &ContextVX);
        exit(-1);
    }

    /* transfer image from gpu to cpu */
    if(VX_SUCCESS!=vxAccessImagePatch(imgObj[1], &imgRect, 0, &(imgInfo[1]), &imgAddr[1], VX_READ_ONLY))
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");

        freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, imgObj, &vxKernel, &vxNode, &GraphVX, &ContextVX);
        exit(-1);
    }

    if((imgInfo[1].dim_y!=imgHei)||(imgInfo[1].dim_x!=imgWid)||(imgInfo[1].step_y!=1)||(imgInfo[1].step_x!=1))
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");

        freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, imgObj, &vxKernel, &vxNode, &GraphVX, &ContextVX);
        exit(-1);
    }

    for(y=0; y<imgHei; y++)
    {
        for (x=0; x<imgWid; x++)
        {
            imgPixel[1] = vxFormatImagePatchAddress2d(imgAddr[1], x, y, &(imgInfo[1]));
            imgBuf[y*imgWid+x] = *((vx_uint8*)(imgPixel[1]));
        }
    }

    if(VX_SUCCESS!=vxCommitImagePatch(imgObj[1], NULL, 0, &(imgInfo[1]), imgAddr[1]))
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");

        freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, imgObj, &vxKernel, &vxNode, &GraphVX, &ContextVX);
        exit(-1);
    }
    imgAddr[1] = NULL;

    /* output image */
    destFile = fopen("lena_gaussian.bmp", "wb");
    if(NULL==destFile)
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "file error.");

        freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, imgObj, &vxKernel, &vxNode, &GraphVX, &ContextVX);
        return -1;
    }

    fwrite(tmpBuf, fileSize, 1, destFile);
    fwrite(tmpBuf2, (fileOffset-fileSize), 1, destFile);
    fwrite(imgBuf, imgHei*imgWid*sizeof(u08), 1, destFile);

    freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, imgObj, &vxKernel, &vxNode, &GraphVX, &ContextVX);

    printf("vx process success.\n");

    return 0;
}

int freeRes(FILE** srcFile, FILE** destFile, u08** tmpBuf, u08** tmpBuf2, u08** imgBuf, vx_image imgObj[], vx_kernel* vxKernel, vx_node* vxNode, vx_graph* GraphVX, vx_context* ContextVX)
{
    int i = 0;

    if(NULL!=*srcFile) { fclose(*srcFile); *srcFile = NULL; }
    if(NULL!=*destFile) { fclose(*destFile); *destFile = NULL; }
    if(NULL!=*tmpBuf) { free(*tmpBuf); *tmpBuf = NULL; }
    if(NULL!=*tmpBuf2) { free(*tmpBuf2); *tmpBuf2 = NULL; }
    if(NULL!=*imgBuf) { free(*imgBuf); *imgBuf = NULL; }
    for(i=0; i<2; i++) { if(NULL!=imgObj[i]) vxReleaseImage(&(imgObj[i])); }
    if(NULL!=*vxKernel) { vxReleaseKernel(vxKernel); }
    if(NULL!=*vxNode) { vxReleaseNode(vxNode); }
    if(NULL!=*GraphVX){ vxReleaseGraph(GraphVX); }
    if(NULL!=*ContextVX){ vxReleaseContext(ContextVX); }

    return 0;
}

vx_status vxcGaussian3x3ValidateInput(vx_node node, vx_uint32 index)
{
    vx_image     imgObj = NULL;
    vx_df_image  imgFmt = VX_DF_IMAGE_U8;
    vx_parameter paramObj = NULL;
    vx_status    status = VX_ERROR_INVALID_PARAMETERS;

    if(0!=index) return VX_ERROR_INVALID_PARAMETERS;

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

vx_status vxcGaussian3x3ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta)
{
    vx_image     imgObj = NULL;
    vx_df_image  imgFmt = VX_DF_IMAGE_U8;
    vx_uint32    imgWid = 0, imgHei = 0;
    vx_parameter paramObj = NULL;
    vx_status    status = VX_ERROR_INVALID_PARAMETERS;

    if(1!=index) return VX_ERROR_INVALID_PARAMETERS;

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

vx_status vxcGaussian3x3Initialize(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    /* workdim,    globel offset,    globel scale,    local size,    globel size */
    vx_kernel_execution_parameters_t shaderParam = {2,    {0, 0, 0},    {0, 0, 0},    {8, 1, 0},    {0, 0, 0}};
    vx_int32 imgWid = 0, imgHei = 0;
    vx_image imgObj = (vx_image)parameters[0];

    vxQueryImage(imgObj, VX_IMAGE_ATTRIBUTE_WIDTH, &imgWid, sizeof(vx_uint32));
    vxQueryImage(imgObj, VX_IMAGE_ATTRIBUTE_HEIGHT, &imgHei, sizeof(vx_uint32));

    shaderParam.globalWorkScale[0] = 14;
    shaderParam.globalWorkScale[1] = 1;
    shaderParam.globalWorkSize[0] = ((((imgWid+13) / 14 ) + (shaderParam.localWorkSize[0] - 1)) / shaderParam.localWorkSize[0]) * shaderParam.localWorkSize[0];
    shaderParam.globalWorkSize[1] = 1;

    vxSetNodeUniform(node, "height", 1, &imgHei);
    vxSetNodeAttribute(node, VX_NODE_ATTRIBUTE_KERNEL_EXECUTION_PARAMETERS, &shaderParam, sizeof(vx_kernel_execution_parameters_t));

    return VX_SUCCESS;
}

vx_status vxcGaussian3x3RegisterKernel(vx_context context)
{
    vx_kernel kernelObj = 0;
    vx_uint32 j = 0, paramCnt = sizeof(paramsGaussian3x3)/sizeof(paramsGaussian3x3[0]);
    vx_program programObj = 0;
    const vx_char* srcKernel[] = { vxcGaussian3x3Source };
    vx_size programLen = strlen(vxcGaussian3x3Source);

    programObj = vxCreateProgramWithSource(context, 1, srcKernel, &programLen);
    vxBuildProgram(programObj, "-cl-viv-vx-extension");

    kernelObj = vxAddKernelInProgram(
                                  programObj,
                                  VXV_KERNEL_NAME_GAUSSIAN3x3,
                                  VXV_KERNEL_GAUSSIAN3x3,
                                  paramCnt,
                                  vxcGaussian3x3ValidateInput,
                                  vxcGaussian3x3ValidateOutput,
                                  vxcGaussian3x3Initialize,
                                  NULL
                                 );

    for(j = 0; j < paramCnt; j++)
    {
        vx_param_description_s paramObj = paramsGaussian3x3[j];
        vxAddParameterToKernel(kernelObj, j, paramObj.paramDire, paramObj.paramType, paramObj.paramStat);
    }

    vxFinalizeKernel(kernelObj);

    return VX_SUCCESS;
}

