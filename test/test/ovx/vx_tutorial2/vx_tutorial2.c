/****************************************************************************
*
*    Copyright 2012 - 2016 Vivante Corporation, Santa Clara, California.
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

typedef unsigned char     u08;
typedef unsigned short   u16;
typedef unsigned int        u32;
typedef int                            s32;

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

int freeRes(FILE** srcFile, FILE** destFile, u08** tmpBuf, u08** tmpBuf2, u08** imgBuf, vx_threshold* thrObj, vx_scalar* shiftObj, vx_image imgObj[], vx_node vxNode[], vx_graph* GraphVX, vx_context* ContextVX);

int main(int argc, char* argv[])
{
    /* VX variables */
    vx_context ContextVX = NULL;
    vx_graph GraphVX = NULL;
    vx_node vxNode[5] = { NULL, NULL, NULL, NULL, NULL };

    vx_image imgObj[7] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL };
    vx_imagepatch_addressing_t imgInfo[2] = { VX_IMAGEPATCH_ADDR_INIT, VX_IMAGEPATCH_ADDR_INIT };
    void* imgAddr[2] = { NULL, NULL };
    void* imgPixel[2] = { NULL, NULL };
    vx_rectangle_t imgRect = { 0, 0, 0, 0 };
    vx_uint32 imgWid = 0;
    vx_uint32 imgHei = 0;

    vx_threshold thrObj = NULL;
    vx_int32 thrVal = 100;
    vx_scalar shiftObj = NULL;
    vx_int32 shiftVal = 0;

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


        freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, &thrObj, &shiftObj, imgObj, vxNode, &GraphVX, &ContextVX);
        exit(-1);
    }

    fileSize = sizeof(FileHeader) + sizeof(InfoHeader);

    tmpBuf = (u08*)malloc(fileSize);
    if(NULL==tmpBuf)
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "memory error.");

        freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, &thrObj, &shiftObj, imgObj, vxNode, &GraphVX, &ContextVX);
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

        freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, &thrObj, &shiftObj, imgObj, vxNode, &GraphVX, &ContextVX);
        exit(-1);
    }
    fread((u08*)tmpBuf2, (fileOffset-fileSize), 1, srcFile);

    imgBuf = (u08*)malloc(imgHei*imgWid*sizeof(u08));
    if(NULL==imgBuf)
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "memory error.");

        freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, &thrObj, &shiftObj, imgObj, vxNode, &GraphVX, &ContextVX);
        exit(-1);
    }
    fread((u08*)imgBuf, imgHei*imgWid*sizeof(u08), 1, srcFile);

    /* vx procedure */
    ContextVX = vxCreateContext();
    if(NULL!=ContextVX)
    {
        GraphVX = vxCreateGraph(ContextVX);
        if(NULL!=GraphVX)
        {
            imgObj[0] = vxCreateImage(ContextVX, imgWid, imgHei, VX_DF_IMAGE_U8);
            imgObj[1] = vxCreateImage(ContextVX, imgWid, imgHei, VX_DF_IMAGE_U8);
            imgObj[2] = vxCreateImage(ContextVX, imgWid, imgHei, VX_DF_IMAGE_U8);
            imgObj[3] = vxCreateImage(ContextVX, imgWid, imgHei, VX_DF_IMAGE_S16);
            imgObj[4] = vxCreateImage(ContextVX, imgWid, imgHei, VX_DF_IMAGE_S16);
            imgObj[5] = vxCreateImage(ContextVX, imgWid, imgHei, VX_DF_IMAGE_S16);
            imgObj[6] = vxCreateImage(ContextVX, imgWid, imgHei, VX_DF_IMAGE_U8);

            thrObj = vxCreateThreshold(ContextVX, VX_THRESHOLD_TYPE_BINARY, VX_TYPE_UINT8);
            shiftObj = vxCreateScalar(ContextVX, VX_TYPE_INT32, &shiftVal);

            imgRect.start_x = 0;
            imgRect.start_y = 0;
            imgRect.end_x = imgWid;
            imgRect.end_y = imgHei;

            if(VX_SUCCESS!=vxSetThresholdAttribute(thrObj, VX_THRESHOLD_ATTRIBUTE_THRESHOLD_VALUE, &thrVal, sizeof(thrVal)))
            {
                printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");

                freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, &thrObj, &shiftObj, imgObj, vxNode, &GraphVX, &ContextVX);
                exit(-1);
            }

            vxNode[0] = vxGaussian3x3Node(GraphVX, imgObj[0], imgObj[2]);
            vxNode[1] = vxSobel3x3Node(GraphVX, imgObj[2], imgObj[3], imgObj[4]);
            vxNode[2] = vxMagnitudeNode(GraphVX,  imgObj[3],  imgObj[4],  imgObj[5]);
            vxNode[3] = vxConvertDepthNode(GraphVX, imgObj[5], imgObj[6], VX_CONVERT_POLICY_SATURATE, shiftObj);
            vxNode[4] = vxThresholdNode(GraphVX, imgObj[6], thrObj, imgObj[1]);
            if(!(vxNode[0]&&vxNode[1]&&vxNode[2]&&vxNode[3]&&vxNode[4]))
            {
                printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");

                freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, &thrObj, &shiftObj, imgObj, vxNode, &GraphVX, &ContextVX);
                exit(-1);
            }

            if(VX_SUCCESS!=vxVerifyGraph(GraphVX))
            {
                printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");

                freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, &thrObj, &shiftObj, imgObj, vxNode, &GraphVX, &ContextVX);
                exit(-1);
            }
        }
        else
        {
            printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");

            freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, &thrObj, &shiftObj, imgObj, vxNode, &GraphVX, &ContextVX);
            exit(-1);
        }
    }
    else
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");

        freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, &thrObj, &shiftObj, imgObj, vxNode, &GraphVX, &ContextVX);
        exit(-1);
    }

    /* transfer image from cpu to gpu */
    if(VX_SUCCESS!=vxAccessImagePatch(imgObj[0], &imgRect, 0, &imgInfo[0], &imgAddr[0], VX_WRITE_ONLY))
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");

        freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, &thrObj, &shiftObj, imgObj, vxNode, &GraphVX, &ContextVX);
        exit(-1);
    }

    if((1!=imgInfo[0].step_y)||(1!=imgInfo[0].step_x)||(imgHei!=imgInfo[0].dim_y)||(imgWid!=imgInfo[0].dim_x))
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");

        freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, &thrObj, &shiftObj, imgObj, vxNode, &GraphVX, &ContextVX);
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

        freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, &thrObj, &shiftObj, imgObj, vxNode, &GraphVX, &ContextVX);
        exit(-1);
    }
    imgAddr[0] = NULL;

    /* process image */
    if(VX_SUCCESS!=vxProcessGraph(GraphVX))
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");

        freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, &thrObj, &shiftObj, imgObj, vxNode, &GraphVX, &ContextVX);
        exit(-1);
    }

    /* transfer image from gpu to cpu */
    if(VX_SUCCESS!=vxAccessImagePatch(imgObj[1], &imgRect, 0, &(imgInfo[1]), &imgAddr[1], VX_READ_ONLY))
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");

        freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, &thrObj, &shiftObj, imgObj, vxNode, &GraphVX, &ContextVX);
        exit(-1);
    }

    if((imgInfo[1].dim_y!=imgHei)||(imgInfo[1].dim_x!=imgWid)||(imgInfo[1].step_y!=1)||(imgInfo[1].step_x!=1))
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");

        freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, &thrObj, &shiftObj, imgObj, vxNode, &GraphVX, &ContextVX);
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

        freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, &thrObj, &shiftObj, imgObj, vxNode, &GraphVX, &ContextVX);
        exit(-1);
    }
    imgAddr[1] = NULL;

    /* output image */
    destFile = fopen("lena_sobel.bmp", "wb");
    if(NULL==destFile)
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "file error.");

        freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, &thrObj, &shiftObj, imgObj, vxNode, &GraphVX, &ContextVX);
        return -1;
    }

    fwrite(tmpBuf, fileSize, 1, destFile);
    fwrite(tmpBuf2, (fileOffset-fileSize), 1, destFile);
    fwrite(imgBuf, imgHei*imgWid*sizeof(u08), 1, destFile);

    freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, &thrObj, &shiftObj, imgObj, vxNode, &GraphVX, &ContextVX);

    printf("vx process success.\n");

    return 0;
}

int freeRes(FILE** srcFile, FILE** destFile, u08** tmpBuf, u08** tmpBuf2, u08** imgBuf, vx_threshold* thrObj, vx_scalar* shiftObj, vx_image imgObj[], vx_node vxNode[], vx_graph* GraphVX, vx_context* ContextVX)
{
    int i = 0;

    if(NULL!=*srcFile) { fclose(*srcFile); *srcFile = NULL; }
    if(NULL!=*destFile) { fclose(*destFile); *destFile = NULL; }
    if(NULL!=*tmpBuf) { free(*tmpBuf); *tmpBuf = NULL; }
    if(NULL!=*tmpBuf2) { free(*tmpBuf2); *tmpBuf2 = NULL; }
    if(NULL!=*thrObj){ vxReleaseThreshold(thrObj); }
    if(NULL!=*shiftObj){ vxReleaseScalar(shiftObj); }
    for(i=0; i<7; i++) { if(NULL!=imgObj[i]) vxReleaseImage(&(imgObj[i])); }
    for(i=0; i<5; i++) { if(NULL!=vxNode[i]) vxReleaseNode(&vxNode[i]); }
    if(NULL!=*GraphVX){ vxReleaseGraph(GraphVX); }
    if(NULL!=*ContextVX){ vxReleaseContext(ContextVX); }

    return 0;
}




