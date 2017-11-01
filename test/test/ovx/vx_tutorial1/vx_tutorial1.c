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

int freeRes(FILE** srcFile, FILE** destFile, u08** tmpBuf, u08** tmpBuf2, u08** imgBuf, vx_image imgObj[], vx_context* ContextVX);

int main(int argc, char* argv[])
{
    vx_context ContextVX = NULL;

    vx_image imgObj[3]  = { NULL, NULL, NULL };
    void* imgAddr[3] = { NULL, NULL, NULL };
    vx_imagepatch_addressing_t imgInfo[3] = { VX_IMAGEPATCH_ADDR_INIT, VX_IMAGEPATCH_ADDR_INIT, VX_IMAGEPATCH_ADDR_INIT };
    void* imgPixel[3] = { NULL, NULL, NULL };
    vx_uint32 imgWid = 0;
    vx_uint32 imgHei = 0;
    vx_rectangle_t imgRect = { 0, 0, 0, 0 };

    FILE* srcFile = NULL;
    FILE* destFile = NULL;
    u08* tmpBuf = NULL;
    u08* tmpBuf2 = NULL;
    u08* imgBuf = NULL;
    u32 fileSize = 0;
    u32 fileOffset = 0;
    u32 x = 0, y = 0;
    u16 desPixel;
    int status = -1;

    vx_map_id map_id = 0;
    vx_map_id map_id1 = 0;

    /* read image data */
    srcFile = fopen("test_gray.bmp", "rb");
    if(NULL==srcFile)
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "file error.");

        status = -1;
        goto exit;
    }

    fileSize = sizeof(FileHeader) + sizeof(InfoHeader);

    tmpBuf = (u08*)malloc(fileSize);
    if(NULL==tmpBuf)
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "memory error.");

        status = -1;
        goto exit;
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

        status = -1;
        goto exit;
    }
    fread((u08*)tmpBuf2, (fileOffset-fileSize), 1, srcFile);

    imgBuf = (u08*)malloc(imgHei*imgWid*sizeof(u08));
    if(NULL==imgBuf)
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "memory error.");

        status = -1;
        goto exit;
    }
    fread((u08*)imgBuf, imgHei*imgWid*sizeof(u08), 1, srcFile);

    /* vx procedure */
    ContextVX = vxCreateContext();
    if (ContextVX)
    {
        vx_border_t borderMode = {0};

        imgRect.start_x = 0;
        imgRect.start_y = 0;
        imgRect.end_x = imgWid;
        imgRect.end_y = imgHei;

        /* transfer image from cpu to gpu */
        imgObj[0] = vxCreateImage(ContextVX, imgWid, imgHei, VX_DF_IMAGE_U8);
        imgObj[1] = vxCreateImage(ContextVX, imgWid, imgHei, VX_DF_IMAGE_S16);
        imgObj[2] = vxCreateImage(ContextVX, imgWid, imgHei, VX_DF_IMAGE_S16);

        if(VX_SUCCESS != vxMapImagePatch(imgObj[0], &imgRect, 0, &map_id, &imgInfo[0], &imgAddr[0], VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0))
        {
            printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");

            status = -1;
            goto exit;
        }

        if((1!=imgInfo[0].step_y)||(1!=imgInfo[0].step_x)||(imgHei!=imgInfo[0].dim_y)||(imgWid!=imgInfo[0].dim_x))
        {
            printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");

            status = -1;
            goto exit;
        }

        for(y=0; y<imgHei; y++)
        {
            for (x=0; x<imgWid; x++)
            {
                imgPixel[0] = vxFormatImagePatchAddress2d(imgAddr[0], x, y, &(imgInfo[0]));
                *((vx_uint8*)(imgPixel[0])) = imgBuf[y*imgWid+x];
            }
        }

        if(VX_SUCCESS!=vxUnmapImagePatch(imgObj[0], map_id))
        {
            printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");

            status = -1;
            goto exit;
        }
        imgAddr[0] = NULL;

        /*set border mode*/
        borderMode.mode = VX_BORDER_REPLICATE;
        vxSetContextAttribute(ContextVX, VX_CONTEXT_IMMEDIATE_BORDER, &borderMode, sizeof(vx_border_t));

        /* process image */
        if(VX_SUCCESS!=vxuSobel3x3(ContextVX, imgObj[0], imgObj[1], imgObj[2]))
        {
            printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");

            status = -1;
            goto exit;
        }

        /* transfer image from gpu to cpu */
        if(VX_SUCCESS!=vxMapImagePatch(imgObj[1], &imgRect, 0, &map_id, &(imgInfo[1]), &imgAddr[1], VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0))
        {
            printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");

            status = -1;
            goto exit;
        }

        if(VX_SUCCESS!=vxMapImagePatch(imgObj[2], &imgRect, 0, &map_id1, &(imgInfo[2]), &imgAddr[2], VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0))
        {
            printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");

            status = -1;
            goto exit;
        }

        if((imgInfo[2].dim_y!=imgInfo[1].dim_y)||(imgInfo[2].dim_x!=imgInfo[1].dim_x)||
            (imgInfo[2].step_y!=imgInfo[1].step_y)||(imgInfo[2].step_x!=imgInfo[1].step_x)||
            (imgInfo[2].dim_y!=imgHei)||(imgInfo[2].dim_x!=imgWid)||
            (imgInfo[2].step_y!=1)||(imgInfo[2].step_x!=1))
        {
            printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");

            status = -1;
            goto exit;
        }

        for(y=0; y<imgHei; y++)
        {
            for (x=0; x<imgWid; x++)
            {
                imgPixel[1] = vxFormatImagePatchAddress2d(imgAddr[1], x, y, &imgInfo[1]);
                imgPixel[2] = vxFormatImagePatchAddress2d(imgAddr[2], x, y, &imgInfo[2]);

                desPixel = (vx_uint16)sqrt((float)(*((vx_int16*)(imgPixel[1]))) * (float)(*((vx_int16*)(imgPixel[1]))) +
                                (float)(*((vx_int16*)(imgPixel[2]))) * (float)(*((vx_int16*)(imgPixel[2]))));
                if(desPixel>0xff) desPixel = 0xff;

                imgBuf[y*imgWid+x] = (u08)desPixel;
            }
        }

        if(VX_SUCCESS!=vxUnmapImagePatch(imgObj[1], map_id))
        {
            printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");

            status = -1;
            goto exit;
        }
        imgAddr[1] = NULL;


        if(VX_SUCCESS!=vxUnmapImagePatch(imgObj[2], map_id1))
        {
            printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");

            status = -1;
            goto exit;
        }
        imgAddr[2] = NULL;

        /* output image */
        destFile = fopen("test_sobel.bmp", "wb");
        if(NULL==destFile)
        {
            printf("%s:%d, %s\n", __FILE__, __LINE__, "file error.");

            status = -1;
            goto exit;
        }

        fwrite(tmpBuf, fileSize, 1, destFile);
        fwrite(tmpBuf2, (fileOffset-fileSize), 1, destFile);
        fwrite(imgBuf, imgHei*imgWid*sizeof(u08), 1, destFile);

        status = 0;
    }
    else
    {
        printf("%s:%d, %s\n", __FILE__, __LINE__, "vx procedure error.");

        status = -1;
        goto exit;
    }

    printf("vx process success.\n");

exit:
    freeRes(&srcFile, &destFile, &tmpBuf, &tmpBuf2, &imgBuf, imgObj, &ContextVX);
    return status;
}

int freeRes(FILE** srcFile, FILE** destFile, u08** tmpBuf, u08** tmpBuf2, u08** imgBuf, vx_image imgObj[], vx_context* ContextVX)
{
    int i = 0;

    if(NULL!=*srcFile) { fclose(*srcFile); *srcFile = NULL; }
    if(NULL!=*destFile) { fclose(*destFile); *destFile = NULL; }
    if(NULL!=*tmpBuf) { free(*tmpBuf); *tmpBuf = NULL; }
    if(NULL!=*tmpBuf2) { free(*tmpBuf2); *tmpBuf2 = NULL; }
    for(i=0; i<3; i++) { if(NULL!=imgObj[i]) vxReleaseImage(&(imgObj[i])); }
    if(NULL!=*ContextVX) { vxReleaseContext(ContextVX); }

    return 0;
}



