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


#include "PreComp.h"
#include "bitmap.h"
#include <stdlib.h>

#define TILED_DESTINATION 0

/******************************************************************************\
**************************** gcoBITMAP Object Type ****************************
\******************************************************************************/

enum BITMAP_TYPE
{
    gcvOBJ_BITMAP = gcmCC('B','M','P',' ')
};

#ifdef LINUX
#pragma pack(1)

typedef unsigned short WORD;
typedef unsigned long  DWORD;
typedef long  LONG;

typedef struct tagBITMAPFILEHEADER
{
    WORD  bfType;
    DWORD bfSize;
    WORD  bfReserved1;
    WORD  bfReserved2;
    DWORD bfOffBits;
}
BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER
{
    DWORD  biSize;
    LONG   biWidth;
    LONG   biHeight;
    WORD   biPlanes;
    WORD   biBitCount;
    DWORD  biCompression;
    DWORD  biSizeImage;
    LONG   biXPelsPerMeter;
    LONG   biYPelsPerMeter;
    DWORD  biClrUsed;
    DWORD  biClrImportant;
}
BITMAPINFOHEADER, *PBITMAPINFOHEADER;

#pragma pack()
#endif

/******************************************************************************\
******************************* gcoBITMAP Object *******************************
\******************************************************************************/

struct _gcoBITMAP
{
    /* Object. */
    gcsOBJECT object;

    /* Pointer to an gcoOS object. */
    gcoOS os;

    /* Path of module. */
#ifdef UNDER_CE
    char  path[MAX_PATH];
#endif

    /* Bitmap pointers. */
    BITMAPFILEHEADER * fileHeader;
    BITMAPINFOHEADER * fileInfo;
    gctPOINTER bits;

    /* Sizes. */
    gctUINT pixelSize;
    gctUINT fileStride;
    gctUINT dstStride;
};


static gctUINT32 ReorderTo32(gctUINT8 *buf)
{
    return ((gctUINT32)buf[3] << 24) + ((gctUINT32)buf[2] << 16)
        + ((gctUINT32)buf[1] << 8) + (gctUINT32)buf[0];
}

static gctUINT16 ReorderTo16(gctUINT8 *buf)
{
    return ((gctUINT32)buf[1] << 8) + (gctUINT32)buf[0];
}

/******************************************************************************\
****************************** gcoBITMAP API Code ******************************
\******************************************************************************/

/*******************************************************************************
**
**  gcoBITMAP_Construct
**
**  Construct bitmap object.
**
**  INPUT:
**
**        gcoOS Os
**            Pointer to an gcoOS object.
**
**  OUTPUT:
**
**        gcoBITMAP * Bitmap
**            Pointer to a new gcoBITMAP object.
*/
gceSTATUS gcoBITMAP_Construct(
    IN gcoOS Os,
    IN OUT gcoBITMAP * Bitmap
    )
{
    gceSTATUS status;
    gcoBITMAP bitmap;
#ifdef UNDER_CE
    TCHAR buffer[MAX_PATH];
    int i, last;
#endif
    gcmHEADER_ARG("Os=0x%x Bitmap=0x%x", Os, Bitmap);
    gcmVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmVERIFY_ARGUMENT(Bitmap != gcvNULL);

    /* Allocate the gcoBRUSH object. */
    status = gcoOS_Allocate(Os, sizeof(struct _gcoBITMAP), (gctPOINTER *) &bitmap);

    if (status != gcvSTATUS_OK)
    {
        /* Error. */
        return status;
    }

    /* Initialize the gcoBRUSH object.*/
    bitmap->object.type = gcvOBJ_BITMAP;
    bitmap->os = Os;

    /* Set members. */
    bitmap->fileHeader = gcvNULL;
    bitmap->fileInfo = gcvNULL;
    bitmap->bits = gcvNULL;

    /* Sizes. */
    bitmap->pixelSize = 0;
    bitmap->fileStride = 0;
    bitmap->dstStride = 0;

    /* Set the result. */
    *Bitmap = bitmap;

#ifdef UNDER_CE
    GetModuleFileName(0, buffer, MAX_PATH);
    for (i = 0; buffer[i] != '\0'; i++)
    {
        bitmap->path[i]    = (char) buffer[i];
        if (buffer[i] == '\\')
        {
            last = i;
        }
    }
    bitmap->path[last + 1] = '\0';
#endif

    gcmFOOTER();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoBITMAP_Destroy
**
**  Destroy bitmap object.
**
**  INPUT:
**
**        gcoBITMAP Bitmap
**            Pointer to a gcoBITMAP object.
**
**  OUTPUT:
**
**        Nothing.
*/
gceSTATUS gcoBITMAP_Destroy(
    IN gcoBITMAP Bitmap
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("Bitmap=0x%x", Bitmap);
    gcmVERIFY_OBJECT(Bitmap, gcvOBJ_BITMAP);

    status = gcoBITMAP_Free(Bitmap);
    if (status != gcvSTATUS_OK)
    {
        return status;
    }

    /* Free the object. */
    gcmVERIFY_OK(gcoOS_Free(Bitmap->os, Bitmap));

    gcmFOOTER();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoBITMAP_Free
**
**  Free resources held by bitmap object.
**
**  INPUT:
**
**        gcoBITMAP Bitmap
**            Pointer to a gcoBITMAP object.
**
**  OUTPUT:
**
**        Nothing.
*/
gceSTATUS gcoBITMAP_Free(
    IN gcoBITMAP Bitmap
    )
{
    gcmHEADER_ARG("Bitmap=0x%x", Bitmap);
    gcmVERIFY_OBJECT(Bitmap, gcvOBJ_BITMAP);

    if (Bitmap->fileHeader != gcvNULL)
    {
        free(Bitmap->fileHeader);
        Bitmap->fileHeader = gcvNULL;
        Bitmap->fileInfo = gcvNULL;
        Bitmap->bits = gcvNULL;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoBITMAP_Load
**
**  Load a bitmap file.
**
**  INPUT:
**
**        gcoBITMAP Bitmap
**            Pointer to a gcoBITMAP object.
**
**        char * FileName
**            Name of a 24-bit bitmap file.
**
**  OUTPUT:
**
**        Nothing.
*/
gceSTATUS gcoBITMAP_Load(
    IN gcoBITMAP Bitmap,
    IN char * FileName
    )
{
    gceSTATUS status;
    FILE * file = gcvNULL;
    gctUINT32 fileLength;
    gctUINT32 readNum;
    gctUINT8 * fileImage;
    gctSIZE_T bytes;
    gctUINT8 * buffer;
#ifdef UNDER_CE
    char fileBuffer[MAX_PATH];
#endif
    DWORD i;

    gcmHEADER_ARG("Bitmap=0x%x FileName=0x%x", Bitmap, FileName);
    gcmVERIFY_OBJECT(Bitmap, gcvOBJ_BITMAP);

    /* Free image if loaded. */
    gcmVERIFY_OK(gcoBITMAP_Free(Bitmap));

    do
    {
        /* Open file. */
#ifdef UNDER_CE
        strcpy(fileBuffer, Bitmap->path);
        strcat(fileBuffer, FileName);
        file = fopen(fileBuffer, "rb");
#else
        file = fopen(FileName, "rb");
#endif
        if (file == gcvNULL)
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

        /* Determine the length of the file. */
        fileLength = 0;
        if (fseek(file, 0, SEEK_END) == 0)
        {
            fileLength = ftell(file);
            if (fileLength == ~0)
                fileLength = 0;
        }

        if (fileLength == 0)
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

        if (fseek(file, 0, SEEK_SET) != 0)
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

        /* Allocate the buffer + alignment bytes. */
        fileImage = malloc(fileLength + 6);
        if (fileImage == gcvNULL)
        {
            status = gcvSTATUS_OUT_OF_MEMORY;
            break;
        }

        /* Start at beginning of buffer. */
        buffer = fileImage;

        /* Read the BITMAPFILEHEADER structure. */
        bytes   = sizeof(BITMAPFILEHEADER);
        readNum = (gctUINT32) fread(buffer, 1, bytes, file);
        if (readNum != bytes)
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

        /* Save BITMAPFILEHEADER pointer. */
        Bitmap->fileHeader = (gctPOINTER) buffer;

        Bitmap->fileHeader->bfType =
            (WORD)ReorderTo16((gctUINT8 *)&Bitmap->fileHeader->bfType);
        Bitmap->fileHeader->bfSize =
            (DWORD)ReorderTo32((gctUINT8 *)&Bitmap->fileHeader->bfSize);
        Bitmap->fileHeader->bfOffBits =
            (DWORD)ReorderTo32((gctUINT8 *)&Bitmap->fileHeader->bfOffBits);

        // Validate.
        if (Bitmap->fileHeader->bfType != 0x4D42)    // 'BM'
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

        /* Update file pointer. */
        fileLength -= bytes;
        buffer     += gcmALIGN(bytes, 4);

        /* Read the BITMAPINFOHEADER structure. */
        bytes   = Bitmap->fileHeader->bfOffBits - bytes;
        readNum = (gctUINT32) fread(buffer, 1, bytes, file);
        if (readNum != bytes)
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

        /* Save BITMAPINFOHEADER pointer. */
        Bitmap->fileInfo = (gctPOINTER) buffer;

        Bitmap->fileInfo->biSize =
            (DWORD)ReorderTo32((gctUINT8 *)&Bitmap->fileInfo->biSize);
        Bitmap->fileInfo->biWidth =
            (LONG)ReorderTo32((gctUINT8 *)&Bitmap->fileInfo->biWidth);
        Bitmap->fileInfo->biHeight =
            (LONG)ReorderTo32((gctUINT8 *)&Bitmap->fileInfo->biHeight);
        Bitmap->fileInfo->biPlanes =
            (WORD)ReorderTo16((gctUINT8 *)&Bitmap->fileInfo->biPlanes);
        Bitmap->fileInfo->biBitCount =
            (WORD)ReorderTo16((gctUINT8 *)&Bitmap->fileInfo->biBitCount);
        Bitmap->fileInfo->biCompression =
            (DWORD)ReorderTo32((gctUINT8 *)&Bitmap->fileInfo->biCompression);
        Bitmap->fileInfo->biSizeImage =
            (DWORD)ReorderTo32((gctUINT8 *)&Bitmap->fileInfo->biSizeImage);
        Bitmap->fileInfo->biXPelsPerMeter =
            (LONG)ReorderTo32((gctUINT8 *)&Bitmap->fileInfo->biXPelsPerMeter);
        Bitmap->fileInfo->biYPelsPerMeter =
            (LONG)ReorderTo32((gctUINT8 *)&Bitmap->fileInfo->biYPelsPerMeter);
        Bitmap->fileInfo->biClrUsed =
            (DWORD)ReorderTo32((gctUINT8 *)&Bitmap->fileInfo->biClrUsed);
        Bitmap->fileInfo->biClrImportant =
            (DWORD)ReorderTo32((gctUINT8 *)&Bitmap->fileInfo->biClrImportant);

        // Validate.
        if (Bitmap->fileInfo->biBitCount != 32)
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

        /* Update file pointer. */
        fileLength -= bytes;
        buffer     += gcmALIGN(bytes, 4);

        /* Recompute image size. */
        Bitmap->fileInfo->biSizeImage = gcmALIGN(Bitmap->fileInfo->biWidth * 4, 4)
                                      * abs(Bitmap->fileInfo->biHeight);

        /* Read the bits. */
        bytes   = gcmMIN(Bitmap->fileInfo->biSizeImage, fileLength);
        readNum = (gctUINT32) fread(buffer, 1, bytes, file);
        if (readNum != bytes)
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

        /* Save BITS pointer. */
        Bitmap->bits = (gctPOINTER) buffer;

        for (i = 0; i < bytes / 4; i++)
        {
            DWORD *p = (DWORD *)buffer;
            p[i] = (DWORD)ReorderTo32((gctUINT8 *)&p[i]);
        }

        /* Close the file. */
        fclose(file);
        file = gcvNULL;

        /* Compute sizes. */
        Bitmap->pixelSize
            = Bitmap->fileInfo->biBitCount / 8;
        Bitmap->fileStride
            = gcmALIGN(Bitmap->fileInfo->biWidth * Bitmap->pixelSize, 4);

        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    if (Bitmap->fileHeader != gcvNULL)
    {
        free(Bitmap->fileHeader);
        Bitmap->fileHeader = gcvNULL;
    }

    if (file != gcvNULL);
    {
        fclose(file);
    }

    gcmFOOTER();

    /* Return status. */
    return status;
}

/*******************************************************************************
**
**  gcoBITMAP_GetSize
**
**  Return the size of the bitmap.
**
**  INPUT:
**
**        gcoBITMAP Bitmap
**            Pointer to a gcoBITMAP object.
**
**  OUTPUT:
**
**        gctUINT32 * Width
**        gctUINT32 * Height
**            Pointers to bitmap dimensions.
*/
gceSTATUS gcoBITMAP_GetSize(
    IN gcoBITMAP Bitmap,
    IN OUT gctUINT32 * Width,
    IN OUT gctUINT32 * Height
    )
{
    gcmHEADER_ARG("Bitmap=0x%x", Bitmap);
    gcmVERIFY_OBJECT(Bitmap, gcvOBJ_BITMAP);

    if (Width != gcvNULL)
    {
        *Width = Bitmap->fileInfo->biWidth;
    }

    if (Height != gcvNULL)
    {
        if (Bitmap->fileInfo->biHeight < 0)
        {
            *Height = -Bitmap->fileInfo->biHeight;
        }
        else
        {
            *Height = Bitmap->fileInfo->biHeight;
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoBITMAP_GetSize
**
**  Upload the bitmap to the specified destination.
**
**  INPUT:
**
**        gcoBITMAP Bitmap
**            Pointer to a gcoBITMAP object.
**
**        gctPOINTER Destination
**            Pointer to the destination for the bitmap to be uploaded to.
**
**  OUTPUT:
**
**        Destination filled with bitmap bits.
*/
gceSTATUS gcoBITMAP_Upload(
    IN gcoBITMAP Bitmap,
    IN gctPOINTER Destination
    )
{
    gctINT srcY, yStart, yEnd, yStep;
    gctUINT8 * fileLine;
    gctUINT8 * dstLine;
    gctUINT32 scanSize;

#if TILED_DESTINATION
    gctINT dstX, dstY;
    gctUINT8 destPixelSize;
    gctUINT32 * dstPixel;
    gctUINT32 pixel;
#endif

    gcmHEADER_ARG("Bitmap=0x%x", Bitmap);
    gcmVERIFY_OBJECT(Bitmap, gcvOBJ_BITMAP);

    /* Determine whether to flip or not. */
    if (Bitmap->fileInfo->biHeight < 0)
    {
        yStart = 0;
        yEnd   = -Bitmap->fileInfo->biHeight;
        yStep  = 1;
    }
    else
    {
        yStart = Bitmap->fileInfo->biHeight - 1;
        yEnd   = -1;
        yStep  = -1;
    }

    /* Init destination scan line pointer. */
    dstLine = Destination;
#if TILED_DESTINATION
    dstY = 0;
#endif

    /* Compute the size of a scan line in bytes. */
    scanSize = Bitmap->fileInfo->biWidth * Bitmap->pixelSize;

    /* Upload the bitmap. */
    for (srcY = yStart; srcY != yEnd; srcY += yStep)
    {
        /* Determine the source position. */
        fileLine = ((gctUINT8*)Bitmap->bits) + srcY * Bitmap->fileStride;

#if TILED_DESTINATION
        /* Tiled line copy. */
        destPixelSize = 2;
        for (dstX = 0; dstX < Bitmap->fileInfo->biWidth; dstX++)
        {
            /* Get the current pixel. */
            pixel = *(gctUINT32*)fileLine;

            /* Compute the destination. */
            dstPixel = (gctUINT32*) (dstLine

                     // Skip full rows of tiles.
                     + ((dstY & ~7) * Bitmap->dstStride)

                     // Skip full tiles on the target row of tiles.
                     + ((dstX & ~7) << (3 + destPixelSize))

                     // Skip in-tile rows.
                     + ((dstY & 7) << (3 + destPixelSize))

                     // Skip pixels in the target row.
                     + ((dstX & 7) << destPixelSize));

            /* Set the pixel. */
            *dstPixel = pixel;

            /* Advance to the next pixel. */
            fileLine += 4;
        }

        /* Advance destination. */
        dstY++;
#else
        /* Copy the line. */
        memcpy(dstLine, fileLine, scanSize);

        /* Update destination. */
        dstLine += Bitmap->dstStride;
#endif
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoBITMAP_Save
**
**  Save the bitmap to a file.
**
**  INPUT:
**
**        gctPOINTER source
**            Pointer to the location of the bitmap in memory.
**
**        gctUINT stride
**            Stride of the bitmap in bytes.
**
**        gctINT left
**        gctINT top
**        gctINT right
**        gctINT bottom
**            Coordinates of the region to save.
**
**        char * FileName
**            Name of a 24-bit bitmap file to be created.
**
**  OUTPUT:
**
**        Nothing.
*/
gceSTATUS gcoBITMAP_Save(
    IN gctPOINTER source,
    IN gctUINT stride,
    IN gctINT left,
    IN gctINT top,
    IN gctINT right,
    IN gctINT bottom,
    IN char * fileName
    )
{
    gceSTATUS status;
    FILE * file;
    gctINT x, y, width, height;
    gctUINT imageStride;
    gctUINT imageSize;
    gctUINT fileStride;
    gctUINT fileImageSize;
    gctUINT fileSize;
    gctUINT8 * fileBuffer;
    BITMAPFILEHEADER * fileHeader;
    BITMAPINFOHEADER * fileInfo;
    gctUINT8 * bits;
    gctUINT8 * srcLine;
    gctUINT8 * fileLine;
    gctUINT8* srcPixel32;
    gctUINT8* filePixel24;

    file = gcvNULL;
    fileBuffer = gcvNULL;

    do
    {
        /* Compute the size of the bitmap. */
        width = right - left;
        height = bottom - top;

        /* Compute the sizes. */
        imageStride = width * 3;    // 24-bit bitmap.
        imageSize = imageStride * height;
        fileStride = (imageStride + 3) & ~3;
        fileImageSize = fileStride * height;
        fileSize
            = sizeof(BITMAPFILEHEADER)
            + sizeof(BITMAPINFOHEADER)
            + fileImageSize;

        /* Allocate the buffer. */
        fileBuffer = malloc(fileSize);
        if (fileBuffer == gcvNULL)
        {
            status = gcvSTATUS_OUT_OF_MEMORY;
            break;
        }

        /* Initialize buffer pointers. */
        fileHeader
            = (BITMAPFILEHEADER*)fileBuffer;
        fileInfo
            = (BITMAPINFOHEADER*)(fileBuffer + sizeof(BITMAPFILEHEADER));
        bits
            = fileBuffer
            + sizeof(BITMAPFILEHEADER)
            + sizeof(BITMAPINFOHEADER);

        /* Initialize buffer info. */
        fileHeader->bfType    = 0x4D42;        // 'BM'
        fileHeader->bfSize    = fileSize;
        fileHeader->bfOffBits = sizeof(BITMAPFILEHEADER)
                              + sizeof(BITMAPINFOHEADER);

        fileInfo->biSize      = sizeof(BITMAPINFOHEADER);
        fileInfo->biWidth     = width;
        fileInfo->biHeight    = -height;
        fileInfo->biPlanes    = 1;
        fileInfo->biBitCount  = 24;
        fileInfo->biSizeImage = fileImageSize;

        /* Init scan line positions. */
        srcLine = source;
        fileLine = bits;

        /* Advance source to the specified coordinate. */
        srcLine += top * stride + left * 4;

        /* Copy the bitmap data. */
        for (y = 0; y < height; y++)
        {
            srcPixel32 = srcLine;
            filePixel24 = fileLine;

            for (x = 0; x < width; x++)
            {
                /* Set the output pixel. */
                filePixel24[0] = srcPixel32[0];
                filePixel24[1] = srcPixel32[1];
                filePixel24[2] = srcPixel32[2];

                /* Update pixel positions. */
                srcPixel32 += 4;
                filePixel24 += 3;
            }

            /* Update scan lines. */
            srcLine  += stride;
            fileLine += fileStride;
        }

        /* Open file. */
        file = fopen(fileName, "wb");
        if (file == gcvNULL)
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

        /* Write the bitmap. */
        fwrite(fileBuffer, 1, fileSize, file);

        /* Success. */
        status = gcvSTATUS_OK;
    }
    while (gcvFALSE);

    if (fileBuffer != gcvNULL)
    {
        free(fileBuffer);
    }

    if (file != gcvNULL)
    {
        fclose(file);
    }

    return status;
}

/*******************************************************************************
**
**  gcoBITMAP_LoadSurface
**
**  Load bitmap into a new surface.
**
**  INPUT:
**
**        gcoHAL Hal
**            Pointer to an gcoHAL object.
**
**        gcoOS Os
**            Pointer to an gcoOS object.
**
**        char * FileName
**            Name of a 24-bit bitmap file.
**
**  OUTPUT:
**
**      gcoSURF * Surface
**            Pointer to a surface object.
*/
gceSTATUS gcoBITMAP_LoadSurface(
    IN gcoHAL Hal,
    IN gcoOS Os,
    IN char * FileName,
    IN OUT gcoSURF * Surface
    )
{
    gceSTATUS status;
    gcoBITMAP bitmap = gcvNULL;
    gcoSURF surface = gcvNULL;
    gctUINT32 width, height;
    gctUINT32 address;
    gctPOINTER memory = gcvNULL;

    do
    {
        /* Create bitmap object. */
        status = gcoBITMAP_Construct(Os, &bitmap);
        if (status != gcvSTATUS_OK)
        {
            break;
        }

        /* Load bitmap. */
        status = gcoBITMAP_Load(bitmap, FileName);
        if (status != gcvSTATUS_OK)
        {
            break;
        }

        /* Get bitmap size. */
        status = gcoBITMAP_GetSize(bitmap, &width, &height);
        if (status != gcvSTATUS_OK)
        {
            break;
        }

        /* Create surface. */
        status = gcoSURF_Construct(Hal,
                                  width, height, 1,
                                  gcvSURF_BITMAP,
                                  gcvSURF_X8R8G8B8,
                                  gcvPOOL_DEFAULT,
                                  &surface);
        if (status != gcvSTATUS_OK)
        {
            break;
        }

        /* Update stride. */
        gcmVERIFY_OK(gcoSURF_GetAlignedSize(surface,
                                        gcvNULL,
                                        gcvNULL,
                                        (gctINT *) &bitmap->dstStride));

        /* Lock the surface. */
        status = gcoSURF_Lock(surface, &address, &memory);
        if (status != gcvSTATUS_OK)
        {
            break;
        }

        /* Upload bitmap. */
        status = gcoBITMAP_Upload(bitmap, memory);
        if (status != gcvSTATUS_OK)
        {
            break;
        }
    }
    while (gcvFALSE);

    if (memory != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Unlock(surface, memory));
    }

    if (bitmap != gcvNULL)
    {
        gcmVERIFY_OK(gcoBITMAP_Destroy(bitmap));
    }

    if ((status != gcvSTATUS_OK) && (surface != gcvNULL))
    {
        gcmVERIFY_OK(gcoSURF_Destroy(surface));
        surface = gcvNULL;
    }

    *Surface = surface;
    return status;
}

/*******************************************************************************
**
**  gcoBITMAP_SaveSurface
**
**  Save surface into a bitmap.
**
**  INPUT:
**
**      gcoSURF Surface
**            Pointer to a surface object.
**
**        char * FileName
**            Name of a 24-bit bitmap file to be created.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS gcoBITMAP_SaveSurface(
    IN gcoSURF Surface,
    IN char * FileName
    )
{
    gceSTATUS status;
    gctUINT width, height;
    gctUINT stride;
    gctUINT32 address;
    gctPOINTER memory = gcvNULL;

    do
    {
        /* Query surface size. */
        status = gcoSURF_GetSize(Surface, &width, &height, gcvNULL);
        if (status != gcvSTATUS_OK)
        {
            break;
        }

        /* Lock the surface. */
        status = gcoSURF_Lock(Surface, &address, &memory);
        if (status != gcvSTATUS_OK)
        {
            break;
        }

        /* Compute the stride. */
        stride = width * 4;

        /* Save bitmap. */
        status = gcoBITMAP_Save(memory, stride, 0, 0, width, height, FileName);
        if (status != gcvSTATUS_OK)
        {
            break;
        }
    }
    while (gcvFALSE);

    if (memory != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Unlock(Surface, memory));
    }

    return status;
}
