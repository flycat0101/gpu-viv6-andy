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


#ifndef BMPINTERFACE_H
#define BMPINTERFACE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <string>

#ifdef LINUX
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/fb.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>
#endif

#define CHANNELNUM 4

using namespace std;

typedef unsigned char      u08;
typedef unsigned short    u16;
typedef unsigned int         u32;
typedef int                             s32;

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

class BmpInterface
{
public:
    FILE* srcFile;
    FILE* desFile;

    u08* imgBuf1;
    u08* imgBuf2;
    u08* outBuf;
    u32 imgWid;
    u32 imgHei;

#ifdef LINUX
    struct timeval tmsStart, tmsEnd;
    u32 fpsNum[10];
#endif

    BmpInterface(void);
    int writeBMP( u08* inputBuf, u32 width, u32 height, int cn );
    int readBMP(const char* filename, u08 *inputBuf, u32 xSize, u32 ySize, int cn);
    int rgb2YUV(u08* inputBuf, u32 xSize, u32 ySize);
    int parseOption(int argc, char** argv, string &fArgv, u32 &wArgv, u32 &hArgv, float &cArgv, string &nArgv, u32 &sArgv);
    void help(void);
    void clearRes(void);

#ifdef LINUX
    int showFPGA(u08* inputBuf, u32 xSize, u32 ySize);

    void startFPS(void);
    void endFPS(void);
    void printFPS(u32 scaleVal, string &appName);
    void getFPS(u32 scaleVal, string &appName, u32 xGroup, u32 yGroup);
#endif
};

BmpInterface::BmpInterface(void)
{
    srcFile = NULL;
    desFile = NULL;
    imgBuf1 = NULL;
    imgBuf2 = NULL;
    outBuf = NULL;
    imgWid = 0;
    imgHei = 0;
}

void BmpInterface::clearRes(void)
{
    if(NULL!=srcFile) { fclose(srcFile); srcFile = NULL; }
    if(NULL!=desFile) { fclose(desFile); desFile = NULL; }
    if(NULL!=imgBuf1) { free(imgBuf1); imgBuf1 = NULL; }
    if(NULL!=imgBuf2) { free(imgBuf2); imgBuf2 = NULL; }
    if(NULL!=outBuf) { free(outBuf); outBuf = NULL; }
}

void BmpInterface::help(void)
{
    printf("\n");
    printf("----------------help---------------------\n");
    printf("\n");
    printf("version:12142016\n");
    printf("vx_tutorial4 [-f ] [-c ] [-w] [-h]\n");
    printf("-f: input data filelist, a necessary parameter.\n");
    printf("-c: FPGA clk MHz, default is off to generate bmp, set it >0 to change app to perf mode.\n");
    printf("-n: app name, default value is debayer_vxc.\n");
    printf("-w: image width,  default value is 640.\n");
    printf("-h: image height, default value is 480.\n");
    printf("\n");
}

int BmpInterface::parseOption(int argc, char** argv, string &fArgv, u32 &wArgv, u32 &hArgv, float &cArgv, string &nArgv, u32 &sArgv)
{
    if(1==argc)
    {
        return -1;
    }

    //parse options
    for(s32 i=1; i<argc; i++)
    {
        if('-'==argv[i][0])
        {
            switch(argv[i][1])
            {
            case 'f':
                {
                    if(++i<argc)
                        fArgv = argv[i];
                    else
                        return -1;
                }
                break;
            case 's':
                {
                    if(++i<argc)
                        sArgv = (u32)atoi(argv[i]);
                    else
                        return -1;
                }
                break;


            case 'n':
                {
                    if(++i<argc)
                        nArgv = argv[i];
                    else
                        return -1;
                }
                break;

            case 'w':
                {
                    if(++i<argc)
                        wArgv = (u32)atoi(argv[i]);
                    else
                        return -1;
                }
                break;

            case 'h':
                {
                    if(++i<argc)
                        hArgv = (u32)atoi(argv[i]);
                    else
                        return -1;
                }
                break;

            case 'c':
                {
                    if(++i<argc)
                        cArgv = (float)atof(argv[i]);
                    else
                        return -1;
                }
                break;

            default :
                return -1;
            }
        }
        else
        {
            return -1;
        }
    }

    return 0;
}


int BmpInterface::writeBMP( u08* inputBuf, u32 width, u32 height, int cn )
{
    if(cn == 1)
    {
        FileHeader* vFileHeader = NULL;
        InfoHeader* vInfoHeader = NULL;
        u08* palleteData = NULL;
        u08* imageData = NULL;
        u08* bmpBuffer = NULL;


        u32 imgSize = width * height;
        u32 headerSize = 1024 + sizeof(FileHeader) + sizeof(InfoHeader);
        u32 fileSize = headerSize + imgSize;

        u32 i, val, length = 1 << 8;

        bmpBuffer = (u08*)malloc(fileSize);
        if(NULL==bmpBuffer)
        {
            printf("invalid buffer\n");
            return -1;
        }
        else
        {
            vFileHeader = (FileHeader*)bmpBuffer;
            vInfoHeader = (InfoHeader*)(bmpBuffer + sizeof(FileHeader));
            palleteData = bmpBuffer + sizeof(FileHeader) + sizeof(InfoHeader);
            imageData = bmpBuffer + headerSize;
        }

        // assign vFileHeader & vInfoHeader
        vFileHeader->fileType = 0x4d42;
        vFileHeader->fileSize1 = (u16)((fileSize << 16) >> 16);
        vFileHeader->fileSize2 = (u16)(fileSize >> 16);
        vFileHeader->fileReserved1 = 0;
        vFileHeader->fileReserved2 = 0;
        vFileHeader->fileOffset1 = (u16)((headerSize << 16) >> 16);
        vFileHeader->fileOffset2 = (u16)(headerSize >> 16);

        vInfoHeader->infoSize = sizeof(InfoHeader);
        vInfoHeader->imageWidth = width;
        vInfoHeader->imageHeight = height;
        vInfoHeader->imagePlane = 1;
        vInfoHeader->imageCount = 8;
        vInfoHeader->imageCompression = 0;
        vInfoHeader->imageSize = 0;
        vInfoHeader->hResolution = 0;
        vInfoHeader->vResolution = 0;
        vInfoHeader->clrUsed = 0;
        vInfoHeader->clrImportant = 0;

        for( i = 0; i < length; i++ )
        {
            val = (i * 255/(length - 1)) ^ 0;
            palleteData[0] = (u08)val;
            palleteData[1] = (u08)val;
            palleteData[2] = (u08)val;
            palleteData[3] = (u08)0;
            palleteData += 4;
        }
        // assign image data
        memcpy(imageData, inputBuf, imgSize);

        // write desFile
        if(NULL==desFile)
        {
            if(NULL!=bmpBuffer)
            {
                free(bmpBuffer);
                bmpBuffer = NULL;
            }

            printf("desFile is NULL\n");
            return -1;
        }
        else
        {
            fwrite(bmpBuffer, fileSize, 1, desFile);

            if(NULL!=bmpBuffer)
            {
                free(bmpBuffer);
                bmpBuffer = NULL;
            }

        }
    }
    else if(cn == CHANNELNUM)
    {
        FileHeader* vFileHeader = NULL;
        InfoHeader* vInfoHeader = NULL;
        u08* imageData = NULL;
        u08* bmpBuffer = NULL;

        u32 imgSize = width * height;
        u32 fileSize = sizeof(FileHeader) + sizeof(InfoHeader) + imgSize * CHANNELNUM;

        bmpBuffer = (u08*)malloc(fileSize);
        if(NULL==bmpBuffer)
        {
            printf("malloc bmpBuffer failed\n");
            return -1;
        }
        else
        {
            vFileHeader = (FileHeader*)bmpBuffer;
            vInfoHeader = (InfoHeader*)(bmpBuffer + sizeof(FileHeader));
            imageData = bmpBuffer + sizeof(FileHeader) + sizeof(InfoHeader);
        }

        // assign vFileHeader & vInfoHeader
        vFileHeader->fileType = 0x4d42;
        vFileHeader->fileSize1 = (u16)((fileSize << 16) >> 16);
        vFileHeader->fileSize2 = (u16)(fileSize >> 16);
        vFileHeader->fileReserved1 = 0;
        vFileHeader->fileReserved2 = 0;
        vFileHeader->fileOffset1 = (u16)(((sizeof(FileHeader) + sizeof(InfoHeader)) << 16) >> 16);
        vFileHeader->fileOffset2 = (u16)((sizeof(FileHeader) + sizeof(InfoHeader)) >> 16);

        vInfoHeader->infoSize = sizeof(InfoHeader);
        vInfoHeader->imageWidth = width;
        vInfoHeader->imageHeight = -((s32)height);
        vInfoHeader->imagePlane = 1;
        vInfoHeader->imageCount = 32;
        vInfoHeader->imageCompression = 0;
        vInfoHeader->imageSize = imgSize * CHANNELNUM;
        vInfoHeader->hResolution = 0;
        vInfoHeader->vResolution = 0;
        vInfoHeader->clrUsed = 0;
        vInfoHeader->clrImportant = 0;

        // assign image data
        memcpy(imageData, inputBuf, imgSize*CHANNELNUM);

        // write desFile
        if(NULL==desFile)
        {
            if(NULL!=bmpBuffer)
            {
                free(bmpBuffer);
                bmpBuffer = NULL;
            }

            printf("desFile is NULL\n");
            return -1;
        }
        else
        {
            fwrite(bmpBuffer, fileSize, 1, desFile);

            if(NULL!=bmpBuffer)
            {
                free(bmpBuffer);
                bmpBuffer = NULL;
            }
        }
    }


    return 0;
}


int BmpInterface::readBMP(const char* filename, u08 *inputBuf, u32 width, u32 height, int cn)
{
    //read gray image
    u08* tmpBuf2 = NULL;
    u08* tmpBuf = NULL;
    if(cn==1)
    {
        /* read image data */
        FILE *srcFile = fopen(filename, "rb");
        u32 fileSize = 0;
        u32 fileOffset = 0;

        if(NULL==srcFile)
        {
            printf("%s:%d, %s\n", __FILE__, __LINE__, "file error.");
            return -1;
        }
        fileSize = sizeof(FileHeader) + sizeof(InfoHeader);
	    if( NULL==tmpBuf )
		    tmpBuf = (u08*)malloc(fileSize);

        fread(tmpBuf, fileSize, 1, srcFile);
        fileOffset = ((((FileHeader*)tmpBuf)->fileOffset2) << 16) + (((FileHeader*)tmpBuf)->fileOffset1);
        if( NULL==tmpBuf2 )
            tmpBuf2 = (u08*)malloc(fileOffset-fileSize);
        if(NULL==tmpBuf2)
        {
            printf("%s:%d, %s\n", __FILE__, __LINE__, "memory error.");
            return -1;
        }


        fread((u08*)tmpBuf2, (fileOffset-fileSize), 1, srcFile);
        fread(inputBuf, width * height * cn * sizeof(u08), 1, srcFile);

        free(tmpBuf);
        free(tmpBuf2);
        fclose(srcFile);
        srcFile = NULL;

        return 0;
    }
    else if(cn == CHANNELNUM)
    {

        FILE *srcFile = fopen(filename, "rb");
        u32 fileSize = 0;
        u32 fileOffset = 0;

        if(NULL==srcFile)
        {
            printf("%s:%d, %s\n", __FILE__, __LINE__, "file error.");
            return -1;
        }
        fileSize = sizeof(FileHeader) + sizeof(InfoHeader);
        if( NULL==tmpBuf )
            tmpBuf = (u08*)malloc(fileSize);
        if(NULL==tmpBuf)
        {
            printf("%s:%d, %s\n", __FILE__, __LINE__, "memory error.");
            return -1;
        }
        fread((u08*)tmpBuf, fileSize, 1, srcFile);

        fileOffset = ((((FileHeader*)tmpBuf)->fileOffset2) << 16) + (((FileHeader*)tmpBuf)->fileOffset1);
        int imgHei = ((InfoHeader*)(tmpBuf + sizeof(FileHeader)))->imageHeight;
        imgHei = (vx_int32)imgHei < 0 ? (~imgHei + 1) : imgHei;

        if( NULL==tmpBuf2 )
            tmpBuf2 = (u08*)malloc(fileOffset-fileSize);
        if(NULL==tmpBuf2)
        {
            printf("%s:%d, %s\n", __FILE__, __LINE__, "memory error.");
            return -1;
        }
        fread((u08*)tmpBuf2, (fileOffset-fileSize), 1, srcFile);

        fread((u08*)inputBuf, width * height * cn * sizeof(u08), 1, srcFile);

        fclose(srcFile);
        srcFile = NULL;
        free(tmpBuf);
        free(tmpBuf2);
    }
    return 0;
}

int BmpInterface::rgb2YUV(u08* inputBuf, u32 xSize, u32 ySize)
{
    u08 tmpPixel[4] = {0, 0, 0, 0};

    for(u32 y=0; y<ySize; y++)
    {
        for(u32 x=0; x<xSize; x++)
        {
            tmpPixel[3] = 0xff;
            tmpPixel[2] = ((66  * inputBuf[y*xSize*4+x*4+2] + 129 * inputBuf[y*xSize*4+x*4+1] + 25  * inputBuf[y*xSize*4+x*4] + 128) >> 8) + 16;  //Y
            tmpPixel[1] = ((-38 * inputBuf[y*xSize*4+x*4+2] - 74  * inputBuf[y*xSize*4+x*4+1] + 112 * inputBuf[y*xSize*4+x*4] + 128) >> 8) + 128; //U
            tmpPixel[0] = ((112 * inputBuf[y*xSize*4+x*4+2] - 94  * inputBuf[y*xSize*4+x*4+1] - 18  * inputBuf[y*xSize*4+x*4] + 128) >> 8) + 128; //V

            inputBuf[y*xSize*4+x*4+3] = 0xff;
            inputBuf[y*xSize*4+x*4+2] = tmpPixel[2]; //Y
            inputBuf[y*xSize*4+x*4+1] = tmpPixel[1]; //U
            inputBuf[y*xSize*4+x*4]   = tmpPixel[0]; //V
        }
    }

    return 0;
}



#ifdef LINUX
int BmpInterface::showFPGA(u08* inputBuf, u32 xSize, u32 ySize)
{
#ifdef NO_SHOW	// for goke cann't call ioctl
	return 0;
#endif

    struct fb_var_screeninfo vInfo;
    struct fb_fix_screeninfo fInfo;
    s32  hFB = 0;
    u32 screenSize = 0;
    u32* pFB = 0;

    u32 xStart = 0;
    u32 xEnd = 0;
    u32 yStart = 0;
    u32 yEnd = 0;
    u32 desIndex=0;
    u32 srcIndex=0;
	u32 xWidthInPixel;

    // read frame buffer info
    hFB = open("/dev/fb0", O_RDWR);
    if (!hFB)
    {
        printf("open /dev/fb0 failed.\n");
        return -1;
    }

    if (ioctl(hFB, FBIOGET_FSCREENINFO, &fInfo))
    {
        close(hFB);

        printf("ioctl FBIOGET_FSCREENINFO information.\n");
        return -1;
    }

    if (ioctl(hFB, FBIOGET_VSCREENINFO, &vInfo))
    {
        close(hFB);

        printf("ioctl FBIOGET_VSCREENINFO information.\n");
        return -1;
    }

    xWidthInPixel = (fInfo.line_length/(vInfo.bits_per_pixel >> 3));

    screenSize = xWidthInPixel * vInfo.yres * CHANNELNUM;

    // mmap frame buffer data
    pFB = (u32*)mmap(0, screenSize, PROT_READ|PROT_WRITE, MAP_SHARED, hFB, 0);
    if (MAP_FAILED==pFB)
    {
        close(hFB);

        printf("mmap frame buffer.\n");
        return -1;
    }

    // write frame buffer
    if(xSize<vInfo.xres)
    {
        xStart = (vInfo.xres - xSize) / 2;
        xEnd   = xStart + xSize;
    }
    else
    {
        xStart = 0;
        xEnd   = vInfo.xres;
    }

    if(ySize<vInfo.yres)
    {
        yStart = (vInfo.yres - ySize) / 2;
        yEnd   = yStart + ySize;
    }
    else
    {
        yStart = 0;
        yEnd   = vInfo.yres;
    }

    for(u32 y=0; y<vInfo.yres; y++)
    {
        for(u32 x=0; x<vInfo.xres; x++)
        {
            desIndex = y * xWidthInPixel + x;
            srcIndex = (y - yStart) * xSize + (x - xStart);

            if((y>=yStart)&&(y<yEnd)&&(x>=xStart)&&(x<xEnd))
            {
                *(pFB + desIndex) = *((u32*)inputBuf + srcIndex);
            }
            else
            {
                *(pFB + desIndex) = 0;
            }
        }
    }

    // remove frame buffer
    munmap(pFB, screenSize);
    close(hFB);

    return 0;
}

void BmpInterface::startFPS(void)
{
    gettimeofday(&tmsStart, 0);
}

void BmpInterface::endFPS(void)
{
    gettimeofday(&tmsEnd, 0);
}

void BmpInterface::printFPS(u32 scaleVal, string &appName)
{
    float fpsVal;
    unsigned int tmpVal;

    tmpVal = 1000 * (tmsEnd.tv_sec - tmsStart.tv_sec) + (tmsEnd.tv_usec - tmsStart.tv_usec) / 1000;
    printf("graph execution time:%d ms\n", tmpVal);

    fpsVal = 1000000.0f / (1000000 * (tmsEnd.tv_sec - tmsStart.tv_sec) + (tmsEnd.tv_usec - tmsStart.tv_usec));
    scaleVal = scaleVal * fpsVal;
    printf("%s: %d FPS\n", appName.c_str(), scaleVal);
}

void BmpInterface::getFPS(u32 scaleVal, string &appName, u32 xGroup, u32 yGroup)
{
    float fpsVal;
    unsigned int tmpVal;

    fpsVal = 1000000.0f / (1000000 * (tmsEnd.tv_sec - tmsStart.tv_sec) + (tmsEnd.tv_usec - tmsStart.tv_usec));
    //printf("%.3f, %d, %d, %d, %d\n", fpsVal, tmsEnd.tv_sec, tmsStart.tv_sec, tmsEnd.tv_usec, tmsStart.tv_usec);  //debug

    scaleVal = scaleVal * fpsVal;
    printf("%s,%d,%d,%d\n", appName.c_str(), scaleVal, xGroup, yGroup);

    tmpVal = 1;
    for(u32 i=0;i<10;i++)
    {
        fpsNum[i] = 0;

        if(0!=scaleVal)
        {
            fpsNum[i] = scaleVal % (tmpVal * 10);
            scaleVal  = scaleVal - fpsNum[i];
            fpsNum[i] = fpsNum[i] / tmpVal;
            tmpVal    = tmpVal * 10;
        }
        else
        {
            fpsNum[i] = 0x1b;
            break;
        }
    }
}
#endif // LINUX

#endif // BMPINTERFACE_H
