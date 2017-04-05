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
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sstream>
#include <fstream>
#include <map>
#include <vector>

using namespace std;

#include "bmp_interface.hpp"

#define OBJCHECK(objVX) if(!(objVX)) { printf("%s:%d, %s\n",__FILE__, __LINE__, "obj create error.");status=-1;goto exit; }
#define FUNCHECK(funRet) if(VX_SUCCESS!=(funRet)) { printf("%s:%d, %s\n",__FILE__, __LINE__, "function error.");status=funRet;goto exit;}
#define MEMCHECK(memPtr) if(!(memPtr)) { printf("%s:%d, %s\n",__FILE__, __LINE__, "memory create error.");status=-1;goto exit;}
#define FILECHECK(filePtr) if(!(filePtr)) { printf("%s:%d, %s\n",__FILE__, __LINE__, "file open error.");status=-1;goto exit; }

#define _RGB_(r,g,b) (((r&0xff) << 16)| ((g&0xff) << 8) | (b &0xff)  )
#define IMAGE_OBJ_NUM (2)
#define SCALAR_OBJ_NUM (2)
#define ARRAY_OBJ_NUM (2)

vx_node vxTutorial4Node(vx_graph graph, vx_image in_image, vx_image out_image);
int freeRes(BmpInterface* bmpInterface, vx_context* context, vx_graph* graph, vx_image **pImgObj);

int main(int argc, char** argv)
{
    string fArgv = "deadbeef";
    u32 imgWid = 640;
    u32 imgHei = 480;
    u32 sArgv = 2000;
    float cArgv = -1;
    string nArgv = "TUTORIAL4";
    int status = 0;

    // VX variables
    vx_context context = NULL;
    vx_graph graph = NULL;
    vx_rectangle_t rect = {0, 0, 0, 0};
    // image parameters
    vx_image *pImgObj = NULL;
    vx_int32 imgObjNum = IMAGE_OBJ_NUM;
    vx_imagepatch_addressing_t imgInfo = VX_IMAGEPATCH_ADDR_INIT;
    void* imgBaseAddr = 0;

    // bmp file interface variables
    BmpInterface bmpInterface;
    ifstream fileList;
    FILE* resFile = NULL;

    // get parameters of application
    if(-1==bmpInterface.parseOption(argc, argv, fArgv, imgWid, imgHei, cArgv, nArgv, sArgv))
    {
        bmpInterface.help();
        status = -1;
        goto exit;
    }
    if(!strcmp(fArgv.c_str(), "deadbeef"))
    {
        bmpInterface.help();
        status = -1;
        goto exit;
    }
    else
    {
        fileList.open(fArgv.c_str());
        if(!fileList)
        {
            printf("open %s failed\n", fArgv.c_str());
            status = -1;
            goto exit;
        }

        fArgv = "output_" + fArgv;
        resFile = fopen(fArgv.c_str(), "wb");
        if(NULL==resFile)
        {
            printf("open %s failed\n", fArgv.c_str());
            status = -1;
            goto exit;
        }
    }

    bmpInterface.imgWid = imgWid;
    bmpInterface.imgHei = imgHei;

    MEMCHECK(bmpInterface.imgBuf1 = (u08*)malloc(bmpInterface.imgHei * bmpInterface.imgWid * sizeof(u08)));
    MEMCHECK(bmpInterface.imgBuf2 = (u08*)malloc(bmpInterface.imgHei * bmpInterface.imgWid * CHANNELNUM * sizeof(u08)));
    MEMCHECK(bmpInterface.outBuf = (u08*)malloc(bmpInterface.imgHei* bmpInterface.imgWid * CHANNELNUM * sizeof(u08)));

    // prepare vx
    OBJCHECK(context = vxCreateContext());
    OBJCHECK(graph = vxCreateGraph(context));

    //malloc image objects
    MEMCHECK(pImgObj = (vx_image *)calloc(imgObjNum, sizeof(vx_image *)));
    OBJCHECK(pImgObj[0] = vxCreateImage(context, imgWid, imgHei, VX_DF_IMAGE_U8));
    OBJCHECK(pImgObj[1] = vxCreateImage(context, imgWid*3, imgHei, VX_DF_IMAGE_U8));

    // create nodes in graph
    OBJCHECK(vxTutorial4Node(graph, pImgObj[0], pImgObj[1]));
    FUNCHECK(vxVerifyGraph(graph));// parameter checking

    // process frame
    while(getline(fileList, fArgv))
    {
        // read srcFile
        FILECHECK(bmpInterface.srcFile = fopen(fArgv.c_str(), "rb"));

        // transfer image from cpu to gpu
        rect.end_x = imgWid;
        rect.end_y = imgHei;
        FUNCHECK(vxAccessImagePatch(pImgObj[0], &rect, 0, &imgInfo, &imgBaseAddr, VX_WRITE_ONLY));// get data pointer of image in GPU side
        //FUNCHECK(bmpInterface.readBMP(fArgv.c_str(), (u08*)imgBaseAddr, imgWid, imgHei, 1));// image read by GPU
        fread((u08*)imgBaseAddr, bmpInterface.imgHei*bmpInterface.imgWid*sizeof(u08), 1, bmpInterface.srcFile);
#ifdef LINUX
        memcpy(bmpInterface.imgBuf1, (u08*)imgBaseAddr, bmpInterface.imgHei*bmpInterface.imgWid*sizeof(u08));// copy image from GPU to CPU for display
#endif
        FUNCHECK(vxCommitImagePatch(pImgObj[0], &rect, 0, &imgInfo, imgBaseAddr));// match with vxAccessImagePatch()
        imgBaseAddr = NULL;

        fclose(bmpInterface.srcFile);
        bmpInterface.srcFile = NULL;

#ifdef LINUX
        bmpInterface.startFPS();
#endif
        // process graph (memory allocate if needed):
        // 1. If C model, call vxDebayerKernel in CPU
        // 2. If using VXC, run VXC code on GPU side directly
        FUNCHECK(vxProcessGraph(graph));

#ifdef LINUX
        bmpInterface.endFPS();
        bmpInterface.printFPS(800/cArgv, nArgv);
#endif

        // transfer image from GPU to CPU
        rect.end_x = imgWid * 3;
        rect.end_y = imgHei;
        FUNCHECK(vxAccessImagePatch(pImgObj[1], &rect, 0, &imgInfo, &imgBaseAddr, VX_READ_ONLY));
        void* imgPixel;
        for(u32 y=0; y < imgHei; y++)
        {
            for (u32 x=0; x < imgWid; x ++ )
            {
                imgPixel = vxFormatImagePatchAddress2d(imgBaseAddr, x * 3, y, &imgInfo);
                bmpInterface.outBuf[y * imgWid * CHANNELNUM + x * CHANNELNUM + 0] = ((vx_uint8*)(imgPixel))[0];
                bmpInterface.outBuf[y * imgWid * CHANNELNUM + x * CHANNELNUM + 1] = ((vx_uint8*)(imgPixel))[1];
                bmpInterface.outBuf[y * imgWid * CHANNELNUM + x * CHANNELNUM + 2] = ((vx_uint8*)(imgPixel))[2];
                bmpInterface.outBuf[y * imgWid * CHANNELNUM + x * CHANNELNUM + 3] = 0xCD;
#ifdef LINUX
                // for input image display
                bmpInterface.imgBuf2[y*imgWid * CHANNELNUM + x * CHANNELNUM + 0] = bmpInterface.imgBuf1[y*imgWid + x];
                bmpInterface.imgBuf2[y*imgWid * CHANNELNUM + x * CHANNELNUM + 1] = bmpInterface.imgBuf1[y*imgWid + x];
                bmpInterface.imgBuf2[y*imgWid * CHANNELNUM + x * CHANNELNUM + 2] = bmpInterface.imgBuf1[y*imgWid + x];
#endif
            }
        }
        FUNCHECK(vxCommitImagePatch(pImgObj[1], &rect, 0, &imgInfo, imgBaseAddr));
        imgBaseAddr = NULL;

#ifdef LINUX
        if(-1!=cArgv)
        {
            bmpInterface.printFPS(800/cArgv, nArgv);

            FUNCHECK(bmpInterface.showFPGA(bmpInterface.imgBuf2, bmpInterface.imgWid, bmpInterface.imgHei));
            usleep(sArgv * 1500);

            FUNCHECK(bmpInterface.showFPGA(bmpInterface.outBuf, bmpInterface.imgWid, bmpInterface.imgHei));
        }
        else
        {
            fArgv = "output_" + fArgv;
            fArgv = fArgv.substr(0,fArgv.find_last_of('.')) + ".bmp";
            FILECHECK(bmpInterface.desFile = fopen(fArgv.c_str(), "wb"));
            FUNCHECK(bmpInterface.writeBMP(bmpInterface.outBuf, imgWid, imgHei, CHANNELNUM));
            fclose(bmpInterface.desFile); bmpInterface.desFile = NULL;

            fprintf(resFile, "%s\n", fArgv.c_str());
            printf("generated %s\n", fArgv.c_str());
        }
#else
        fArgv = "output_" + fArgv;
        fArgv = fArgv.substr(0,fArgv.find_last_of('.')) + ".bmp";
        FILECHECK(bmpInterface.desFile = fopen(fArgv.c_str(), "wb"));
        FUNCHECK(bmpInterface.writeBMP(bmpInterface.outBuf, imgWid, imgHei, CHANNELNUM));
        fclose(bmpInterface.desFile); bmpInterface.desFile = NULL;

        fprintf(resFile, "%s\n", fArgv.c_str());
        printf("generated %s\n", fArgv.c_str());
#endif
    }

    status = 0;
    /* exit and free */
exit:
    if(fileList.is_open())
        fileList.close();
    if(resFile)
    {
        fclose(resFile);
        resFile = NULL;
    }
    freeRes(&bmpInterface, &context, &graph, &pImgObj);
    return status;
}

// only called one time for graph construction
vx_node vxTutorial4Node(vx_graph graph, vx_image in_image, vx_image out_image)
{
    vx_context context = NULL;
    vx_kernel kernel = NULL;
    vx_node node = NULL;
    vx_status status = VX_FAILURE;
    vx_int32 index = 0;

    context = vxGetContext((vx_reference)graph);
    status = vxLoadKernels(context, "tutorial4_kernel");
    if(VX_SUCCESS == status)
    {
        kernel = vxGetKernelByName(context, "com.vivantecorp.extension.tutorial4VXC");
        status = vxGetStatus((vx_reference)kernel);
        if (status == VX_SUCCESS)
        {
            node = vxCreateGenericNode(graph, kernel);
            status = vxGetStatus((vx_reference)node);
            if (status == VX_SUCCESS)
            {
                status |= vxSetParameterByIndex(node, index++, (vx_reference)in_image);
                status |= vxSetParameterByIndex(node, index++, (vx_reference)out_image);
                if(status != VX_SUCCESS)
                {
                    vxReleaseNode(&node);
                    vxReleaseKernel(&kernel);
                    printf("[%s : %d] vxSetParameterByIndex failed,status = %d \n", __FILE__, __LINE__, status);
                    fflush(stdout);
                    return node;
                }

                // node attributes
                vx_border_mode_t border;
                border.mode = VX_BORDER_MODE_REPLICATE;
                border.constant_value = 0;
                status |= vxSetNodeAttribute(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &border, sizeof(border));
                if(VX_SUCCESS != status)
                {
                    vxReleaseNode(&node);
                    vxReleaseKernel(&kernel);
                    printf("[%s : %d] vxSetNodeAttribute failed,status = %d \n",__FILE__, __LINE__,status);
                    fflush(stdout);
                    return node;
                }
            }
            else
            {
                vxReleaseKernel(&kernel);
                printf("[%s : %d] vxCreateGenericNode failed,status = %d \n", __FILE__, __LINE__, status);
                fflush(stdout);
            }
        }
        else
        {
            printf("[%s : %d] vxGetKernelByName failed,status = %d \n", __FILE__, __LINE__, status);
            fflush(stdout);
        }
    }
    else
    {
        printf("[%s : %d] vxLoadKernels failed,status = %d \n", __FILE__, __LINE__, status);
        fflush(stdout);
    }

    return node;
}

int freeRes(BmpInterface* bmpInterface, vx_context* context, vx_graph* graph, vx_image **pImgObj)
{
    bmpInterface->clearRes();

    if(*pImgObj != NULL)
    {
        for(int i=0; i< IMAGE_OBJ_NUM; i++)
        {
            if((*pImgObj)[i] != NULL)
                vxReleaseImage(&((*pImgObj)[i]));
        }
        free(*pImgObj);
    }

    if(NULL!=*graph){ vxReleaseGraph(graph); }
    if(NULL!=*context){ vxReleaseContext(context); }

    return 0;
}
