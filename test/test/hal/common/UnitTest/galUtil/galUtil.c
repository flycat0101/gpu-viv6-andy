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


#ifdef _WIN32
#include <windows.h>
#if UNDER_CE >= 600
time_t time( time_t *timer );
#endif
#endif

#define __DLL_EXPORT
#include <galUtil.h>
#include <gc_hal_driver.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

static  FILE*        g_pFileError   = NULL;
static  FILE*        g_pFileResult  = NULL;
static  FILE*        g_pFileLog     = NULL;
static  FILE*        g_pFilePerf    = NULL;

static  char*   g_filenameError     = NULL;
static  char*   g_filenameResult    = NULL;
static  char*   g_filenameLog       = NULL;
static  char*   g_filenamePerf      = NULL;
/*
 * Functions for reading and writing 16- and 32-bit little-endian integers.
 */
static unsigned char  read_byte(FILE *fp);
static unsigned short read_word(FILE *fp);
static unsigned int   read_dword(FILE *fp);
static int            read_long(FILE *fp);

static int            write_word(FILE *fp, unsigned short w);
static int            write_dword(FILE *fp, unsigned int dw);
static int            write_long(FILE *fp, int l);

static unsigned short swap_word(unsigned short w);

static gceSTATUS
ConvertToGalTiling(
    IN gctUINT32 Value,
    OUT gctUINT32_PTR Tiling,
    OUT gctUINT32_PTR Vsupertile
    );

static void write_buffer(FILE *fp, unsigned char *buffer, int size, int width)
{
    switch (width)
    {
        case 1:
            fwrite(buffer, size, 1, fp);
            break;

        case 2:
            {
                unsigned short * p = (unsigned short *)buffer;
                int n = size / 2;

                while (n > 0)
                {
                    write_word(fp, *p);
                    p++;
                    n--;
                }
            }
            break;

        case 4:
            {
                unsigned int * p = (unsigned int *)buffer;
                int n = size / 4;

                while (n > 0)
                {
                    write_dword(fp, *p);
                    p++;
                    n--;
                }
            }
            break;

        case 3:
        default:
            GalOutput(GalOutputType_Error, "%s:%d: *ERROR*!!\n", __FUNCTION__, __LINE__);
            break;
    }
}

#ifdef UNDER_CE
DLL_API char CurrentModulePath[MAX_BUFFER_SIZE];
#endif
/*
 * 'LoadDIBitmap()' - Load a DIB/BMP file from disk.
 *
 * Returns a pointer to the bitmap if successful, NULL otherwise...
 */
unsigned char * CDECL GalLoadDIBitmap(const char *filename, BMPINFO **info)
{
    FILE            *fp;          /* Open file pointer */
    unsigned char   *bits;        /* Bitmap pixel bits */
    unsigned int    bitsize;      /* Size of bitmap */
    unsigned int    filesize = 0;
    int             infosize;     /* Size of header information */
    BMPFILEHEADER   header;       /* File header */

    /* Try opening the file; use "rb" mode to read this *binary* file. */
    if (gcmIS_ERROR(gcoOS_Open(gcvNULL, filename, gcvFILE_READ, &fp)))
    {
        /* Failed to open the file. */
        GalOutput(GalOutputType_Error, "*ERROR*  Failed to open the file %s", filename);
        return (NULL);
    }

    /* Read the file header and any following bitmap information... */
    header.bfType      = read_word(fp);
    header.bfSize      = read_dword(fp);
    header.bfReserved1 = read_word(fp);
    header.bfReserved2 = read_word(fp);
    header.bfOffBits   = read_dword(fp);

    if (header.bfType != BF_TYPE)
    {
        /* Not a bitmap file - return NULL... */
        GalOutput(GalOutputType_Error, "*ERROR*  Not a bitmap file");
        fclose(fp);
        return (NULL);
    }

    infosize = header.bfOffBits - 14; // sizeof(header);
    if ((*info = (BMPINFO *)malloc(sizeof(BMPINFO))) == NULL)
    {
        /* Couldn't allocate memory for bitmap info - return NULL... */
        fclose(fp);
        GalOutput(GalOutputType_Error, "*ERROR*  out-of-memory1");
        return (NULL);
    }

    (*info)->bmiHeader.biSize          = read_dword(fp);
    (*info)->bmiHeader.biWidth         = read_long(fp);
    (*info)->bmiHeader.biHeight        = read_long(fp);
    (*info)->bmiHeader.biPlanes        = read_word(fp);
    (*info)->bmiHeader.biBitCount      = read_word(fp);
    (*info)->bmiHeader.biCompression   = read_dword(fp);
    (*info)->bmiHeader.biSizeImage     = read_dword(fp);
    (*info)->bmiHeader.biXPelsPerMeter = read_long(fp);
    (*info)->bmiHeader.biYPelsPerMeter = read_long(fp);
    (*info)->bmiHeader.biClrUsed       = read_dword(fp);
    (*info)->bmiHeader.biClrImportant  = read_dword(fp);

    if (infosize > 40)
    {
        int n = (infosize - 40) / 4;
        unsigned int *p = (unsigned int *)((*info)->bmiColors);

        while (n > 0)
        {
            *p = read_dword(fp);
            p++;
            n--;
        }
    }

    fseek(fp, 0, SEEK_END);
    filesize = ftell(fp);

    /* Seek to the image. */
    if (fseek(fp, header.bfOffBits, SEEK_SET) != 0)
    {
        free(*info);
        fclose(fp);
        GalOutput(GalOutputType_Error, "*ERROR* bitmap file error");
        return (NULL);
    }

    /* Now that we have all the header info read in, allocate memory for *
     * the bitmap and read *it* in...                                    */
    if ((bitsize = (*info)->bmiHeader.biSizeImage) == 0)
    {
        bitsize = ((*info)->bmiHeader.biWidth) *
                   (((*info)->bmiHeader.biBitCount + 7) / 8) *
                 abs((*info)->bmiHeader.biHeight);
    }
    else
    {
        if ((int)bitsize < (gcmALIGN(((*info)->bmiHeader.biWidth) *
                   ((*info)->bmiHeader.biBitCount), 8) >> 3) *
                 abs((*info)->bmiHeader.biHeight))
        {
            GalOutput(GalOutputType_Error, "*ERROR* bitmap format wrong!");
            return (NULL);
        }
    }

    if (header.bfOffBits + bitsize > filesize)
    {
        GalOutput(GalOutputType_Error, "*ERROR* bitmap format wrong!");
        return (NULL);
    }

    if ((bits = (unsigned char *)malloc(bitsize)) == NULL)
    {
        /* Couldn't allocate memory - return NULL! */
        free(*info);
        fclose(fp);
        GalOutput(GalOutputType_Error, "*ERROR* out-of-memory2");
        return (NULL);
    }

    if (!GalIsBigEndian())
    {
        if (fread(bits, 1, bitsize, fp) < bitsize)
        {
            /* Couldn't read bitmap - free memory and return NULL! */
            free(*info);
            *info = gcvNULL;
            free(bits);
            fclose(fp);
            GalOutput(GalOutputType_Error, "*ERROR* read bmp file error");
            return (NULL);
        }
    }
    else
    {
        switch ((*info)->bmiHeader.biBitCount)
        {
            case 8:
                fread(bits, 1, bitsize, fp);
                break;

            case 16:
                {
                    unsigned short *p = (unsigned short *)bits;
                    int n = bitsize / 2;

                    while (n > 0)
                    {
                        *p = read_word(fp);
                        p++;
                        n--;
                    }
                }
                break;

            case 24:
                {
                    unsigned char *p = (unsigned char *)bits;
                    int n = bitsize / 3;

                    while (n > 0)
                    {
                        *(p + 2) = read_byte(fp);
                        *(p + 1) = read_byte(fp);
                        *(p)     = read_byte(fp);
                        p += 3;
                        n--;
                    }
                }
                break;

            case 1:
            case 32:
                {
                    unsigned int *p = (unsigned int *)bits;
                    int n = bitsize / 4;

                    while (n > 0)
                    {
                        *p = read_dword(fp);
                        p++;
                        n--;
                    }
                }
                break;

            default:
                GalOutput(GalOutputType_Error, "%s:%d: *ERROR*: unknown biBitCount=%d!!\n", __FUNCTION__, __LINE__,
                        (*info)->bmiHeader.biBitCount);
                break;
        }
    }

    /* OK, everything went fine - return the allocated bitmap... */
    fclose(fp);
    return (bits);
}

/*
 * 'SaveDIBitmap()' - Save a DIB/BMP file to disk.
 *
 * Returns 0 on success or -1 on failure...
 */
int static GalSaveDIBitmap(const char *filename, BMPINFO *info, unsigned char *bits, gctUINT stride)
{
    FILE *fp;                      /* Open file pointer */
    unsigned int    size,                     /* Size of file */
                    infosize,                 /* Size of bitmap info */
                    bitsize;                  /* Size of bitmap pixels */
    gctINT i;
    gctUINT bmpStride  =  info->bmiHeader.biWidth * ((info->bmiHeader.biBitCount + 7) / 8);

    if (bmpStride > stride)
    {
        GalOutput(GalOutputType_Error, "*ERROR*  invalid parameters");

        return -1;
    }

    if (gcmIS_ERROR(gcoOS_Open(gcvNULL, filename, gcvFILE_CREATE, &fp)))
    {
        /* Failed to open the file. */
        GalOutput(GalOutputType_Error, "*ERROR*  Failed to open the file %s", filename);

        return -1;
    }

    /* Figure out the bitmap size */
    if (info->bmiHeader.biSizeImage == 0)
    {
        bitsize =  info->bmiHeader.biWidth *
                    ((info->bmiHeader.biBitCount + 7) / 8) *
                abs(info->bmiHeader.biHeight);
    }
    else
    {
        bitsize = info->bmiHeader.biSizeImage;
    }

    /* Figure out the header size */
    infosize = sizeof(BMPINFOHEADER);
    switch (info->bmiHeader.biCompression)
    {
    case BIT_BITFIELDS :
        infosize += 12; /* Add 3 RGB doubleword masks */
        if (info->bmiHeader.biClrUsed == 0)
        break;
    case BIT_RGB :
        if (info->bmiHeader.biBitCount > 8 &&
        info->bmiHeader.biClrUsed == 0)
        break;
    case BIT_RLE8 :
    case BIT_RLE4 :
        if (info->bmiHeader.biClrUsed == 0)
            infosize += (1 << info->bmiHeader.biBitCount) * 4;
        else
            infosize += info->bmiHeader.biClrUsed * 4;
        break;
    }

    size = sizeof(BMPFILEHEADER) + infosize + bitsize;

    /* Write the file header, bitmap information, and bitmap pixel data... */
    write_word(fp, BF_TYPE);        /* bfType */
    write_dword(fp, size);          /* bfSize */
    write_word(fp, 0);              /* bfReserved1 */
    write_word(fp, 0);              /* bfReserved2 */
    write_dword(fp, 14 + infosize); /* bfOffBits */

    write_dword(fp, info->bmiHeader.biSize);
    write_long(fp, info->bmiHeader.biWidth);
    write_long(fp, abs(info->bmiHeader.biHeight)/*info->bmiHeader.biHeight*/); // bugbug: keep height positive
    write_word(fp, info->bmiHeader.biPlanes);
    write_word(fp, info->bmiHeader.biBitCount);
    write_dword(fp, info->bmiHeader.biCompression);
    write_dword(fp, info->bmiHeader.biSizeImage);
    write_long(fp, info->bmiHeader.biXPelsPerMeter);
    write_long(fp, info->bmiHeader.biYPelsPerMeter);
    write_dword(fp, info->bmiHeader.biClrUsed);
    write_dword(fp, info->bmiHeader.biClrImportant);

    if (infosize > 40)
    {
        int n = (infosize - 40) / 4;
        unsigned int *p = (unsigned int *)(info->bmiColors);

        while (n > 0)
        {
            write_dword(fp, *p);
            p++;
            n--;
        }
    }

    gcmASSERT(infosize + 14 == ftell(fp));

    for (i = 0; i < abs(info->bmiHeader.biHeight); i++)
    {
        if (info->bmiHeader.biHeight > 0)
        {
            if (GalIsBigEndian())
            {
                write_buffer(fp, bits + stride * i, bmpStride, (info->bmiHeader.biBitCount + 7) / 8);
            }
            else
            {
                fwrite(bits + stride * i, 1, bmpStride, fp);
            }
        }
        else
        {
            if (GalIsBigEndian())
            {
                write_buffer(fp, bits + stride * (abs(info->bmiHeader.biHeight) - 1 - i),
                        bmpStride, (info->bmiHeader.biBitCount + 7) / 8);
            }
            else
            {
                fwrite(bits + stride * (abs(info->bmiHeader.biHeight) - 1 - i), 1, bmpStride, fp);
            }
        }
    }

    /* OK, everything went fine - return... */
    fclose(fp);
    return (0);
}

/*
 * 'read_byte()' - Read a 8-bit unsigned integer.
 */
static unsigned char read_byte(FILE *fp)
{
    unsigned char b0;
    b0 = getc(fp);
    return b0;
}

/*
 * 'read_word()' - Read a 16-bit unsigned integer.
 */
static unsigned short read_word(FILE *fp)
{
    unsigned char b0, b1; /* Bytes from file */

    b0 = getc(fp);
    b1 = getc(fp);

    return ((b1 << 8) | b0);
}

/*
 * 'read_dword()' - Read a 32-bit unsigned integer.
 */
static unsigned int read_dword(FILE *fp)
{
    unsigned char b0, b1, b2, b3; /* Bytes from file */

    b0 = getc(fp);
    b1 = getc(fp);
    b2 = getc(fp);
    b3 = getc(fp);

    return ((((((b3 << 8) | b2) << 8) | b1) << 8) | b0);
}

/*
 * 'read_long()' - Read a 32-bit signed integer.
 */
static int read_long(FILE *fp)
{
    unsigned char b0, b1, b2, b3; /* Bytes from file */

    b0 = getc(fp);
    b1 = getc(fp);
    b2 = getc(fp);
    b3 = getc(fp);

    return ((int)(((((b3 << 8) | b2) << 8) | b1) << 8) | b0);
}

/*
 * 'write_word()' - Write a 16-bit unsigned integer.
 */
static int write_word(FILE *fp, unsigned short w)
{
    putc(w, fp);
    return (putc(w >> 8, fp));
}

/*
 * 'write_dword()' - Write a 32-bit unsigned integer.
 */
static int write_dword(FILE *fp, unsigned int dw)
{
    putc(dw, fp);
    putc(dw >> 8, fp);
    putc(dw >> 16, fp);
    return (putc(dw >> 24, fp));
}

/*
 * 'write_long()' - Write a 32-bit signed integer.
 */
static int write_long(FILE *fp,int  l)
{
    putc(l, fp);
    putc(l >> 8, fp);
    putc(l >> 16, fp);
    return (putc(l >> 24, fp));
}

static unsigned short swap_word(unsigned short w)
{
    return ((w << 8) | (w >> 8));
}


/*
 *  Output runtime data/message. They will be saved in corresponding files.
 */
void CDECL GalOutput(GalOutputType type, const char *format, ...)
{
    char    *buffer;
    int     len;
    va_list args;
    va_start( args, format );
    len = MAX_BUFFER_SIZE;
    buffer = (char*)malloc( len * sizeof(char) );
    if (!buffer)
    {
        return;
    }

    vsprintf( buffer, format, args );

    if ('\n' != buffer[strlen(buffer) - 1])
    {
        strcat(buffer, "\n");
    }

    if (type & GalOutputType_Console)
    {
        GalPrintf(buffer);
    }

    if (type & GalOutputType_Error)
    {
        if(g_pFileError == NULL)
        {
            if (g_filenameError != NULL)
            {
                g_pFileError = fopen(g_filenameError, "w");
                if (g_pFileError == NULL)
                {
                    GalPrintf("Cannot open file %s", g_filenameError);
                    return;
                }
            }
        }

        if (g_pFileError != NULL)
        {
            fwrite( buffer, strlen(buffer), 1, g_pFileError);
        }
    }

    if (type & GalOutputType_Result)
    {
        if(g_pFileResult == NULL)
        {
            if (g_filenameResult != NULL)
            {
                g_pFileResult = fopen(g_filenameResult, "w");
                if (g_pFileResult == NULL)
                {
                    GalPrintf("Cannot open file %s", g_filenameResult);
                    return;
                }
            }
        }

        if(g_pFileResult != NULL)
        {
            fwrite( buffer, strlen(buffer), 1, g_pFileResult);
            fflush(g_pFileResult);
        }
    }


    if (type & GalOutputType_Log)
    {
        if (g_pFileLog == NULL)
        {
            if (g_filenameLog != NULL)
            {
                g_pFileLog = fopen(g_filenameLog, "w");
                if (g_pFileLog == NULL)
                {
                    GalPrintf("Cannot open file %s", g_filenameLog);
                    return;
                }
            }
        }

        if (g_pFileLog != NULL)
        {
            fwrite( buffer, strlen(buffer), 1, g_pFileLog);
        }
    }

    if (type & GalOutputType_Perf)
    {
        if (g_pFilePerf == NULL)
        {
            if (g_filenamePerf != NULL)
            {
                g_pFilePerf = fopen(g_filenamePerf, "w");
                if (g_pFilePerf == NULL)
                {
                    GalPrintf("Cannot open file %s", g_filenamePerf);
                    return;
                }
            }
        }

        if (g_pFilePerf != NULL)
        {
            fwrite( buffer, strlen(buffer), 1, g_pFilePerf);
        }
    }


    free(buffer);
    va_end( args );
}

/*
 *  Initialize the files for output runtime data/message.
*/
void CDECL GalInitializeOutput(GalOutputType type, const char *name)
{
    size_t len = strlen( name );
    switch( type ){
        case GalOutputType_Error:
            if( g_filenameError != NULL ) free(g_filenameError);
            g_filenameError = (char *)malloc(len + 1);
            strcpy( g_filenameError, name );
            g_filenameError[len] = '\0';
            break;

        case GalOutputType_Result:
            if( g_filenameResult != NULL ) free(g_filenameResult);
            g_filenameResult = (char *)malloc(len + 1);
            strcpy( g_filenameResult, name );
            g_filenameResult[len] = '\0';
            break;

        case GalOutputType_Log:
            if( g_filenameLog != NULL ) free(g_filenameLog);
            g_filenameLog = (char *)malloc(len + 1);
            strcpy( g_filenameLog, name );
            g_filenameLog[len] = '\0';
            break;

        case GalOutputType_Perf:
            if( g_filenamePerf != NULL ) free(g_filenamePerf);
            g_filenamePerf = (char *)malloc(len + 1);
            strcpy( g_filenamePerf, name );
            g_filenamePerf[len] = '\0';
            break;

        default:
            break;
    }
}

/*
 *  Close the files that opened for output runtime data/message.
*/
void CDECL GalFinalizeOutput()
{
    if( g_pFileError != NULL )
        fclose( g_pFileError );

    if( g_pFileResult != NULL )
        fclose( g_pFileResult );

    if( g_pFileLog != NULL )
        fclose( g_pFileLog );

    if( g_pFilePerf != NULL )
        fclose( g_pFilePerf );

    if( g_filenameError != NULL ){
        free(g_filenameError);
        g_filenameError = NULL;
    }

    if( g_filenameResult != NULL ){
        free(g_filenameResult);
        g_filenameResult = NULL;
    }

    if( g_filenameLog != NULL ){
        free(g_filenameLog);
        g_filenameLog = NULL;
    }

    if( g_filenamePerf != NULL ){
        free(g_filenamePerf);
        g_filenamePerf = NULL;
    }
}

typedef struct _STATUS_MAP {
    gceSTATUS status;
    const char*string;
} STATUS_MAP;

static STATUS_MAP sStatusMap[] = {
    {gcvSTATUS_OK                    , "No Error" },
    {gcvSTATUS_NO_MORE_DATA         , "No More Data" },
    {gcvSTATUS_CACHED                , "Cached" },
    {gcvSTATUS_MIPMAP_TOO_LARGE        , "Mipmap Too Large" },
    {gcvSTATUS_NAME_NOT_FOUND        , "Name Not Found" },
    {gcvSTATUS_NOT_OUR_INTERRUPT    , "Not Our Interrupt" },
    {gcvSTATUS_MISMATCH                , "Mismatch" },
    {gcvSTATUS_TRUE                    , "True" },
    {gcvSTATUS_INVALID_ARGUMENT        , "Invalid Argument" },
    {gcvSTATUS_INVALID_OBJECT         , "Invalid Object" },
    {gcvSTATUS_OUT_OF_MEMORY         , "Out Of Memory" },
    {gcvSTATUS_MEMORY_LOCKED        , "Memory Locked" },
    {gcvSTATUS_MEMORY_UNLOCKED        , "Memory Unlocked" },
    {gcvSTATUS_HEAP_CORRUPTED        , "Heap Corrupted" },
    {gcvSTATUS_GENERIC_IO            , "Generic IO" },
    {gcvSTATUS_INVALID_ADDRESS        , "Invalid Address" },
    {gcvSTATUS_CONTEXT_LOSSED        , "Context Lossed" },
    {gcvSTATUS_TOO_COMPLEX            , "Too Complex" },
    {gcvSTATUS_BUFFER_TOO_SMALL        , "Buffer Too Small" },
    {gcvSTATUS_INTERFACE_ERROR        , "Interface Error" },
    {gcvSTATUS_NOT_SUPPORTED        , "Not Supported" },
    {gcvSTATUS_MORE_DATA            , "More Data" },
    {gcvSTATUS_TIMEOUT                , "Timeout" },
    {gcvSTATUS_OUT_OF_RESOURCES        , "Out Of Resources" },
    {gcvSTATUS_INVALID_DATA            , "Invalid Data" },
    {gcvSTATUS_INVALID_MIPMAP        , "Invalid Mipmap" },
    {gcvSTATUS_CHIP_NOT_READY        , "Chip Not Ready" },
    /* Linker errors. */
    {gcvSTATUS_GLOBAL_TYPE_MISMATCH    , "Global Type Mismatch" },
    {gcvSTATUS_TOO_MANY_ATTRIBUTES    , "Too Many Attributes" },
    {gcvSTATUS_TOO_MANY_UNIFORMS    , "Too Many Uniforms" },
    {gcvSTATUS_TOO_MANY_VARYINGS    , "Too Many Varyings" },
    {gcvSTATUS_UNDECLARED_VARYING    , "Undeclared Varying" },
    {gcvSTATUS_VARYING_TYPE_MISMATCH, "Varying Type Mismatch" },
    {gcvSTATUS_MISSING_MAIN            , "Missing Main" },
    {gcvSTATUS_NAME_MISMATCH        , "Name Mismatch" },
    {gcvSTATUS_INVALID_INDEX        , "Invalid Index" },
};


const char* CDECL GalStatusString(gceSTATUS status)
{
    int i;

    for (i = 0; i < sizeof(sStatusMap)/sizeof(sStatusMap[0]); i++)
    {
        if (sStatusMap[i].status == status)
            return sStatusMap[i].string;
    }

    return "Unknown Status";
}

/*
    return the number of "1" from LSB.
    If the 1s are not continuous, return -1.

*/
static gctINT GetBitsSize(gctUINT bits)
{
    gctINT i, n;

    for (i = 0, n = 0; i < 8 * sizeof(bits); i++)
    {

        if (bits & 1)
            ++n;
        else if (n)
            break;

        bits >>= 1;
    }

    if (bits)
        return -1;
    else
        return n;
}

/*
 * 'GalLoadDIB2Surface()' - Load a DIB/BMP file from disk to SURFACE.
 *
 * Returns NULL on failure...
 */
gcoSURF CDECL GalLoadDIB2Surface(gcoHAL hal, const char *filename)
{
    unsigned char * bits;
    BMPINFO *info;
    gcoSURF surf = NULL;

    bits = GalLoadDIBitmap(filename, &info);
    if (!bits) {
        GalOutput(GalOutputType_Error, "*ERROR* Failed to GalLoadDIBitmap()");
        return NULL;
    }

    do {
        gceSTATUS status;
        gctUINT32 width = gcmALIGN(abs(info->bmiHeader.biWidth), 4);
        gctUINT32 height = abs(info->bmiHeader.biHeight);
        gctUINT alignedWidth, alignedHeight;
        gctINT alignedStride;
        gctUINT32 i, j;
        gctUINT surfDepth, biDepth;
        gceSURF_FORMAT format;
        gctUINT32 pa;
        unsigned char *va;

        if (info->bmiHeader.biCompression == BIT_RGB)
        {
            if (info->bmiHeader.biBitCount == 32)
            {
                format = gcvSURF_A8R8G8B8;
                surfDepth = 4;
                biDepth = 4;
            }
            else if (info->bmiHeader.biBitCount == 24)
            {
                format = gcvSURF_A8R8G8B8;
                surfDepth = 4;
                biDepth = 3;
            }
            else if (info->bmiHeader.biBitCount == 16)
            {
                format = gcvSURF_R5G6B5;
                surfDepth = 2;
                biDepth = 2;
            }
            else if (info->bmiHeader.biBitCount == 8)
            {
                gco2D engn2d;
                format = gcvSURF_INDEX8;
                surfDepth = 1;
                biDepth = 1;
                gcmVERIFY_OK(gcoHAL_Get2DEngine(hal, &engn2d));
                gcmVERIFY_OK(gco2D_LoadPalette(engn2d, 0, 256, info->bmiColors, gcvTRUE));
            }
            else {
                GalOutput(GalOutputType_Error, "*ERROR* Unknown bmiHeader.biBitCount: %d", info->bmiHeader.biBitCount);
                break;
            }
        }
        else if (info->bmiHeader.biCompression == BIT_BITFIELDS)
        {
            gctINT sizeR, sizeG, sizeB, sizeA;
            sizeR = GetBitsSize(info->mask[0]);
            sizeG = GetBitsSize(info->mask[1]);
            sizeB = GetBitsSize(info->mask[2]);
            sizeA =  - sizeR - sizeG - sizeB;

            switch (info->bmiHeader.biBitCount)
            {
            case 32:

                if (info->mask[0] == 0x00FF0000
                    && info->mask[1] == 0x0000FF00
                    && info->mask[2] == 0x000000FF)
                {
                    format = gcvSURF_A8R8G8B8;
                    surfDepth = 4;
                    biDepth = 4;
                }
                else if (info->mask[0] == 0xFF000000
                    && info->mask[1] == 0x00FF0000
                    && info->mask[2] == 0x0000FF00)
                {
                    format = gcvSURF_R8G8B8A8;
                    surfDepth = 4;
                    biDepth = 4;
                }
                else if (info->mask[0] == 0x000000FF
                    && info->mask[1] == 0x0000FF00
                    && info->mask[2] == 0x00FF0000)
                {
                    format = gcvSURF_A8B8G8R8;
                    surfDepth = 4;
                    biDepth = 4;
                }
                else if (info->mask[0] == 0x0000FF00
                    && info->mask[1] == 0x00FF0000
                    && info->mask[2] == 0xFF000000)
                {
                    format = gcvSURF_B8G8R8A8;
                    surfDepth = 4;
                    biDepth = 4;
                }
                else if (info->mask[0] == 0xFFC00000
                    && info->mask[1] == 0x003FF000
                    && info->mask[2] == 0x00000FFC)
                {
                    format = gcvSURF_R10G10B10A2;
                    surfDepth = 4;
                    biDepth = 4;
                }
                else if (info->mask[0] == 0x00000FFC
                    && info->mask[1] == 0x003FF000
                    && info->mask[2] == 0xFFC00000)
                {
                    format = gcvSURF_B10G10R10A2;
                    surfDepth = 4;
                    biDepth = 4;
                }
                else if (info->mask[0] == 0x3FF00000
                    && info->mask[1] == 0x000FFC00
                    && info->mask[2] == 0x000003FF)
                {
                    format = gcvSURF_A2R10G10B10;
                    surfDepth = 4;
                    biDepth = 4;
                }
                else if (info->mask[0] == 0x000003FF
                    && info->mask[1] == 0x000FFC00
                    && info->mask[2] == 0x3FF00000)
                {
                    format = gcvSURF_A2B10G10R10;
                    surfDepth = 4;
                    biDepth = 4;
                }
                else
                {
                    GalOutput(GalOutputType_Error, "*ERROR* unsupported format");
                    gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
                }

                break;

            case 16:
                if (info->mask[0] == 0x00000F00
                    && info->mask[1] == 0x000000F0
                    && info->mask[2] == 0x0000000F)
                {
                    format = gcvSURF_A4R4G4B4;
                    surfDepth = 2;
                    biDepth = 2;
                }
                else if (info->mask[0] == 0x0000F000
                    && info->mask[1] == 0x00000F00
                    && info->mask[2] == 0x000000F0)
                {
                    format = gcvSURF_R4G4B4A4;
                    surfDepth = 2;
                    biDepth = 2;
                }
                else if (info->mask[0] == 0x0000000F
                    && info->mask[1] == 0x000000F0
                    && info->mask[2] == 0x00000F00)
                {
                    format = gcvSURF_A4B4G4R4;
                    surfDepth = 2;
                    biDepth = 2;
                }
                else if (info->mask[0] == 0x000000F0
                    && info->mask[1] == 0x00000F00
                    && info->mask[2] == 0x0000F000)
                {
                    format = gcvSURF_B4G4R4A4;
                    surfDepth = 2;
                    biDepth = 2;
                }
                else if (info->mask[0] == 0x0000F800
                    && info->mask[1] == 0x000007E0
                    && info->mask[2] == 0x0000001F)
                {
                    format = gcvSURF_R5G6B5;
                    surfDepth = 2;
                    biDepth = 2;
                }
                else if (info->mask[0] == 0x0000001F
                    && info->mask[1] == 0x000007E0
                    && info->mask[2] == 0x0000F800)
                {
                    format = gcvSURF_B5G6R5;
                    surfDepth = 2;
                    biDepth = 2;
                }
                else if (info->mask[0] == 0x00007C00
                    && info->mask[1] == 0x000003E0
                    && info->mask[2] == 0x0000001F)
                {
                    format = gcvSURF_A1R5G5B5;
                    surfDepth = 2;
                    biDepth = 2;
                }
                else if (info->mask[0] == 0x0000F800
                    && info->mask[1] == 0x000007C0
                    && info->mask[2] == 0x0000003E)
                {
                    format = gcvSURF_R5G5B5A1;
                    surfDepth = 2;
                    biDepth = 2;
                }
                else if (info->mask[0] == 0x0000003E
                    && info->mask[1] == 0x000007C0
                    && info->mask[2] == 0x0000F800)
                {
                    format = gcvSURF_B5G5R5A1;
                    surfDepth = 2;
                    biDepth = 2;
                }
                else if (info->mask[0] == 0x0000001F
                    && info->mask[1] == 0x000003E0
                    && info->mask[2] == 0x00007C00)
                {
                    format = gcvSURF_A1B5G5R5;
                    surfDepth = 2;
                    biDepth = 2;
                }
                else
                {
                    GalOutput(GalOutputType_Error, "*ERROR* unsupported format");
                    gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
                }

                break;

            default:
                GalOutput(GalOutputType_Error, "*ERROR* unsupported bpp:%d",
                    info->bmiHeader.biBitCount);
                gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
            }
        }
        else {
            GalOutput(GalOutputType_Error, "*ERROR* unknown bmiHeader");
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }

        gcmONERROR(gcoSURF_Construct(hal, width, height, 1, gcvSURF_BITMAP,
                    format, gcvPOOL_DEFAULT, &surf));

        gcmONERROR(gcoSURF_GetAlignedSize(surf, &alignedWidth, &alignedHeight, &alignedStride));

        gcmONERROR(gcoSURF_Lock(surf, &pa, (gctPOINTER *)&va));
        memset(va, 0, alignedHeight * alignedStride);

        for (i = 0; i < height; i++)
        {
            for (j = 0; j < width; j++)
            {
                gctUINT dstOff = i * alignedWidth + j;
                gctUINT srcOff = (info->bmiHeader.biHeight >= 0) ?
                    ((height - 1 - i) * width + j) : (i * width + j);

                if (GalIsBigEndian() && (biDepth == 3))
                {
                    memcpy(va + dstOff * surfDepth + 1, bits + srcOff * biDepth, biDepth);
                }
                else
                {
                    memcpy(va + dstOff * surfDepth, bits + srcOff * biDepth, biDepth);
                }
            }
        }

        gcmONERROR(gcoSURF_Unlock(surf, (gctPOINTER)va));

        if (bits)
        {
            free(bits);
        }

        if (info)
        {
            free(info);
        }

        return surf;

    } while (gcvFALSE);

OnError:
    if (bits)
    {
        free(bits);
    }

    if (info)
    {
        free(info);
    }

    if (surf)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(surf));
    }

    return gcvNULL;
}

gcoSURF CDECL GalLoadDIB2SurfaceWithPool(gcoHAL hal, const char *filename, gcePOOL pool)
{
    unsigned char * bits;
    BMPINFO *info;
    gcoSURF surf = NULL;

    bits = GalLoadDIBitmap(filename, &info);
    if (!bits) {
        GalOutput(GalOutputType_Error, "*ERROR* Failed to GalLoadDIBitmap()");
        return NULL;
    }

    do {
        gceSTATUS status;
        gctUINT32 width = gcmALIGN(abs(info->bmiHeader.biWidth), 4);
        gctUINT32 height = abs(info->bmiHeader.biHeight);
        gctUINT alignedWidth, alignedHeight;
        gctINT alignedStride;
        gctUINT32 i, j;
        gctUINT surfDepth, biDepth;
        gceSURF_FORMAT format;
        gctUINT32 pa;
        unsigned char *va;

        if (info->bmiHeader.biCompression == BIT_RGB)
        {
            if (info->bmiHeader.biBitCount == 32)
            {
                format = gcvSURF_A8R8G8B8;
                surfDepth = 4;
                biDepth = 4;
            }
            else if (info->bmiHeader.biBitCount == 24)
            {
                format = gcvSURF_A8R8G8B8;
                surfDepth = 4;
                biDepth = 3;
            }
            else if (info->bmiHeader.biBitCount == 16)
            {
                format = gcvSURF_R5G6B5;
                surfDepth = 2;
                biDepth = 2;
            }
            else if (info->bmiHeader.biBitCount == 8)
            {
                gco2D engn2d;
                format = gcvSURF_INDEX8;
                surfDepth = 1;
                biDepth = 1;
                gcmVERIFY_OK(gcoHAL_Get2DEngine(hal, &engn2d));
                gcmVERIFY_OK(gco2D_LoadPalette(engn2d, 0, 256, info->bmiColors, gcvTRUE));
            }
            else {
                GalOutput(GalOutputType_Error, "*ERROR* Unknown bmiHeader.biBitCount: %d", info->bmiHeader.biBitCount);
                break;
            }
        }
        else if (info->bmiHeader.biCompression == BIT_BITFIELDS)
        {
            gctINT sizeR, sizeG, sizeB, sizeA;
            sizeR = GetBitsSize(info->mask[0]);
            sizeG = GetBitsSize(info->mask[1]);
            sizeB = GetBitsSize(info->mask[2]);
            sizeA =  - sizeR - sizeG - sizeB;

            switch (info->bmiHeader.biBitCount)
            {
            case 32:

                if (info->mask[0] == 0x00FF0000
                    && info->mask[1] == 0x0000FF00
                    && info->mask[2] == 0x000000FF)
                {
                    format = gcvSURF_A8R8G8B8;
                    surfDepth = 4;
                    biDepth = 4;
                }
                else if (info->mask[0] == 0xFF000000
                    && info->mask[1] == 0x00FF0000
                    && info->mask[2] == 0x0000FF00)
                {
                    format = gcvSURF_R8G8B8A8;
                    surfDepth = 4;
                    biDepth = 4;
                }
                else if (info->mask[0] == 0x000000FF
                    && info->mask[1] == 0x0000FF00
                    && info->mask[2] == 0x00FF0000)
                {
                    format = gcvSURF_A8B8G8R8;
                    surfDepth = 4;
                    biDepth = 4;
                }
                else if (info->mask[0] == 0x0000FF00
                    && info->mask[1] == 0x00FF0000
                    && info->mask[2] == 0xFF000000)
                {
                    format = gcvSURF_B8G8R8A8;
                    surfDepth = 4;
                    biDepth = 4;
                }
                else
                {
                    GalOutput(GalOutputType_Error, "*ERROR* unsupported format");
                    gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
                }

                break;

            case 16:
                if (info->mask[0] == 0x00000F00
                    && info->mask[1] == 0x000000F0
                    && info->mask[2] == 0x0000000F)
                {
                    format = gcvSURF_A4R4G4B4;
                    surfDepth = 2;
                    biDepth = 2;
                }
                else if (info->mask[0] == 0x0000F000
                    && info->mask[1] == 0x00000F00
                    && info->mask[2] == 0x000000F0)
                {
                    format = gcvSURF_R4G4B4A4;
                    surfDepth = 2;
                    biDepth = 2;
                }
                else if (info->mask[0] == 0x0000000F
                    && info->mask[1] == 0x000000F0
                    && info->mask[2] == 0x00000F00)
                {
                    format = gcvSURF_A4B4G4R4;
                    surfDepth = 2;
                    biDepth = 2;
                }
                else if (info->mask[0] == 0x000000F0
                    && info->mask[1] == 0x00000F00
                    && info->mask[2] == 0x0000F000)
                {
                    format = gcvSURF_B4G4R4A4;
                    surfDepth = 2;
                    biDepth = 2;
                }
                else if (info->mask[0] == 0x0000F800
                    && info->mask[1] == 0x000007E0
                    && info->mask[2] == 0x0000001F)
                {
                    format = gcvSURF_R5G6B5;
                    surfDepth = 2;
                    biDepth = 2;
                }
                else if (info->mask[0] == 0x0000001F
                    && info->mask[1] == 0x000007E0
                    && info->mask[2] == 0x0000F800)
                {
                    format = gcvSURF_B5G6R5;
                    surfDepth = 2;
                    biDepth = 2;
                }
                else if (info->mask[0] == 0x00007C00
                    && info->mask[1] == 0x000003E0
                    && info->mask[2] == 0x0000001F)
                {
                    format = gcvSURF_A1R5G5B5;
                    surfDepth = 2;
                    biDepth = 2;
                }
                else if (info->mask[0] == 0x0000F800
                    && info->mask[1] == 0x000007C0
                    && info->mask[2] == 0x0000003E)
                {
                    format = gcvSURF_R5G5B5A1;
                    surfDepth = 2;
                    biDepth = 2;
                }
                else if (info->mask[0] == 0x0000003E
                    && info->mask[1] == 0x000007C0
                    && info->mask[2] == 0x0000F800)
                {
                    format = gcvSURF_B5G5R5A1;
                    surfDepth = 2;
                    biDepth = 2;
                }
                else if (info->mask[0] == 0x0000001F
                    && info->mask[1] == 0x000003E0
                    && info->mask[2] == 0x00007C00)
                {
                    format = gcvSURF_A1B5G5R5;
                    surfDepth = 2;
                    biDepth = 2;
                }
                else
                {
                    GalOutput(GalOutputType_Error, "*ERROR* unsupported format");
                    gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
                }

                break;

            default:
                GalOutput(GalOutputType_Error, "*ERROR* unsupported bpp:%d",
                    info->bmiHeader.biBitCount);
                gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
            }
        }
        else {
            GalOutput(GalOutputType_Error, "*ERROR* unknown bmiHeader");
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }

        gcmONERROR(gcoSURF_Construct(hal, width + 64, height + 64, 1, gcvSURF_BITMAP,
                    format, pool, &surf));

        gcmONERROR(gcoSURF_GetAlignedSize(surf, &alignedWidth, &alignedHeight, &alignedStride));

        gcmONERROR(gcoSURF_Lock(surf, &pa, (gctPOINTER *)&va));
        memset(va, 0, alignedHeight * alignedStride);

        for (i = 0; i < height; i++)
        {
            for (j = 0; j < width; j++)
            {
                gctUINT dstOff = i * alignedWidth + j;
                gctUINT srcOff = (info->bmiHeader.biHeight >= 0) ?
                    ((height - 1 - i) * width + j) : (i * width + j);

                if (GalIsBigEndian() && (biDepth == 3))
                {
                    memcpy(va + dstOff * surfDepth + 1, bits + srcOff * biDepth, biDepth);
                }
                else
                {
                    memcpy(va + dstOff * surfDepth, bits + srcOff * biDepth, biDepth);
                }
            }
        }

        gcmONERROR(gcoSURF_Unlock(surf, (gctPOINTER)va));

        if (bits)
        {
            free(bits);
        }

        if (info)
        {
            free(info);
        }

        return surf;

    } while (gcvFALSE);

OnError:
    if (bits)
    {
        free(bits);
    }

    if (info)
    {
        free(info);
    }

    if (surf)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(surf));
    }

    return gcvNULL;
}

gcoSURF CDECL GalLoadA82Surface(gcoHAL hal, const char *filename)
{
    unsigned char * bits;
    BMPINFO *info;
    gcoSURF surf = NULL;

    bits = GalLoadDIBitmap(filename, &info);

    if (!bits) {
        GalOutput(GalOutputType_Error, "*ERROR* Failed to GalLoadDIBitmap()");
        return NULL;
    }

    do {
        gctUINT32 width = gcmALIGN(abs(info->bmiHeader.biWidth), 4);
        gctUINT32 height = abs(info->bmiHeader.biHeight);
        gctUINT alignedWidth, alignedHeight;
        gctINT alignedStride;
        gctUINT32 i, j;
        gctUINT surfDepth, biDepth;
        gceSURF_FORMAT format;
        gctUINT32 pa;
        unsigned char *va;

        if ((info->bmiHeader.biCompression == BIT_RGB) && (info->bmiHeader.biBitCount == 8))
        {
            gco2D engn2d;
            format = gcvSURF_A8;
            surfDepth = 1;
            biDepth = 1;
            gcmVERIFY_OK(gcoHAL_Get2DEngine(hal, &engn2d));
            gcmVERIFY_OK(gco2D_LoadPalette(engn2d, 0, 256, info->bmiColors,gcvTRUE));
        }
        else {
            GalOutput(GalOutputType_Error, "*ERROR* Unknown bmiHeader.biBitCount: %d", info->bmiHeader.biBitCount);
            break;
        }

        if (gcoSURF_Construct(hal, width, height, 1, gcvSURF_BITMAP,
                    format, gcvPOOL_DEFAULT, &surf) != gcvSTATUS_OK) {
            GalOutput(GalOutputType_Error, "*ERROR* Failed to gcoSURF_Construct(gcvSURF_BITMAP)");
            break;
        }

        gcmVERIFY_OK(gcoSURF_GetAlignedSize(surf, &alignedWidth, &alignedHeight, &alignedStride));

        gcmVERIFY_OK(gcoSURF_Lock(surf, &pa, (gctPOINTER *)&va));
        memset(va, 0, alignedHeight * alignedStride);

        for (i = 0; i < height; i++)
        {
            for (j = 0; j < width; j++)
            {
                gctUINT dstOff = i * alignedWidth + j;
                gctUINT srcOff = (info->bmiHeader.biHeight >= 0) ?
                    ((height - 1 - i) * width + j) : (i * width + j);

                memcpy(va + dstOff * surfDepth, bits + srcOff * biDepth, biDepth);
            }
        }

        gcmVERIFY_OK(gcoSURF_Unlock(surf, (gctPOINTER)va));
        gcmVERIFY_OK(gcoSURF_CPUCacheOperation(surf, gcvCACHE_FLUSH));
    } while (0);

    if (bits)
        free(bits);

    if (info)
        free(info);

    return surf;
}

gctBOOL GalSaveDIB(
    gctPOINTER bits,
    gceSURF_FORMAT  format,
    gctUINT stride,
    gctUINT width,
    gctUINT height,
    const char *bmpFileName)
{
    gctBOOL ret = gcvFALSE;

    if (bits && bmpFileName && bmpFileName[0])
    {
        BMPINFO bitmap;
        gctINT bitCount;

        switch (format)
        {
        case gcvSURF_A8R8G8B8:
        case gcvSURF_X8R8G8B8:
            bitmap.mask[0] = 0x00FF0000;
            bitmap.mask[1] = 0x0000FF00;
            bitmap.mask[2] = 0x000000FF;
            bitCount = 32;
            break;

        case gcvSURF_R8G8B8A8:
        case gcvSURF_R8G8B8X8:
            bitmap.mask[0] = 0xFF000000;
            bitmap.mask[1] = 0x00FF0000;
            bitmap.mask[2] = 0x0000FF00;
            bitCount = 32;
            break;

        case gcvSURF_X8B8G8R8:
        case gcvSURF_A8B8G8R8:
            bitmap.mask[0] = 0x000000FF;
            bitmap.mask[1] = 0x0000FF00;
            bitmap.mask[2] = 0x00FF0000;
            bitCount = 32;
            break;

        case gcvSURF_B8G8R8A8:
        case gcvSURF_B8G8R8X8:
            bitmap.mask[0] = 0x0000FF00;
            bitmap.mask[1] = 0x00FF0000;
            bitmap.mask[2] = 0xFF000000;
            bitCount = 32;
            break;

        case gcvSURF_X4R4G4B4:
        case gcvSURF_A4R4G4B4:
            bitmap.mask[0] = 0x00000F00;
            bitmap.mask[1] = 0x000000F0;
            bitmap.mask[2] = 0x0000000F;
            bitCount = 16;
            break;

        case gcvSURF_R4G4B4X4:
        case gcvSURF_R4G4B4A4:
            bitmap.mask[0] = 0x0000F000;
            bitmap.mask[1] = 0x00000F00;
            bitmap.mask[2] = 0x000000F0;
            bitCount = 16;
            break;

        case gcvSURF_X4B4G4R4:
        case gcvSURF_A4B4G4R4:
            bitmap.mask[0] = 0x0000000F;
            bitmap.mask[1] = 0x000000F0;
            bitmap.mask[2] = 0x00000F00;
            bitCount = 16;
            break;

        case gcvSURF_B4G4R4X4:
        case gcvSURF_B4G4R4A4:
            bitmap.mask[0] = 0x000000F0;
            bitmap.mask[1] = 0x00000F00;
            bitmap.mask[2] = 0x0000F000;
            bitCount = 16;
            break;

        case gcvSURF_R5G6B5:
            bitmap.mask[0] = 0x0000F800;
            bitmap.mask[1] = 0x000007E0;
            bitmap.mask[2] = 0x0000001F;
            bitCount = 16;
            break;

        case gcvSURF_B5G6R5:
            bitmap.mask[0] = 0x0000001F;
            bitmap.mask[1] = 0x000007E0;
            bitmap.mask[2] = 0x0000F800;
            bitCount = 16;
            break;

        case gcvSURF_A1R5G5B5:
        case gcvSURF_X1R5G5B5:
            bitmap.mask[0] = 0x00007C00;
            bitmap.mask[1] = 0x000003E0;
            bitmap.mask[2] = 0x0000001F;
            bitCount = 16;
            break;

        case gcvSURF_R5G5B5X1:
        case gcvSURF_R5G5B5A1:
            bitmap.mask[0] = 0x0000F800;
            bitmap.mask[1] = 0x000007C0;
            bitmap.mask[2] = 0x0000003E;
            bitCount = 16;
            break;

        case gcvSURF_B5G5R5X1:
        case gcvSURF_B5G5R5A1:
            bitmap.mask[0] = 0x0000003E;
            bitmap.mask[1] = 0x000007C0;
            bitmap.mask[2] = 0x0000F800;
            bitCount = 16;
            break;

        case gcvSURF_X1B5G5R5:
        case gcvSURF_A1B5G5R5:
            bitmap.mask[0] = 0x0000001F;
            bitmap.mask[1] = 0x000003E0;
            bitmap.mask[2] = 0x00007C00;
            bitCount = 16;
            break;


        default:
            // can not save and display
            return gcvFALSE;
        }

        /* Fill in the BITMAPINFOHEADER information. */
        bitmap.bmiHeader.biSize = sizeof(bitmap.bmiHeader);
        bitmap.bmiHeader.biWidth = width;
        bitmap.bmiHeader.biHeight = -(gctINT)height;
        bitmap.bmiHeader.biPlanes = 1;
        bitmap.bmiHeader.biBitCount = bitCount;
        bitmap.bmiHeader.biCompression = BIT_BITFIELDS;
        bitmap.bmiHeader.biSizeImage = 0;
        bitmap.bmiHeader.biXPelsPerMeter = 0;
        bitmap.bmiHeader.biYPelsPerMeter = 0;
        bitmap.bmiHeader.biClrUsed = 0;
        bitmap.bmiHeader.biClrImportant = 0;

        if (GalSaveDIBitmap(bmpFileName, (BMPINFO *)&bitmap, bits, stride) == 0)
        {
            ret = gcvTRUE;
        }
    }

    return ret;
}

/*
 * 'GalSaveSurface2DIB()' - Save a DIB/BMP file to disk fram SURFACE.
 *
 * Returns gcvFALSE on failure...
 */
gctBOOL GalSaveSurface2DIB(gcoSURF surface, const char *bmpFileName)
{
    gctUINT alignedWidth, alignedHeight, width, height;
    gctINT bitsStride;
    BMPINFO bitmap;
    gctUINT32 resolveAddress;
    gceSURF_FORMAT  format;
    gctPOINTER bits;
    gctINT bitCount, i;
    RGB tmp;
    gctBOOL ret = gcvFALSE;

    if (surface && bmpFileName && bmpFileName[0])
    {
        gcmVERIFY_OK(gcoSURF_GetAlignedSize(surface, &alignedWidth, &alignedHeight, &bitsStride));
        gcmVERIFY_OK(gcoSURF_GetSize(surface, &width, &height, NULL));

        gcmVERIFY_OK(gcoSURF_GetFormat(surface, NULL, &format));
        switch (format)
        {
        case gcvSURF_A8R8G8B8:
        case gcvSURF_X8R8G8B8:
            bitmap.mask[0] = 0x00FF0000;
            bitmap.mask[1] = 0x0000FF00;
            bitmap.mask[2] = 0x000000FF;
            bitCount = 32;
            break;

        case gcvSURF_R8G8B8A8:
        case gcvSURF_R8G8B8X8:
            bitmap.mask[0] = 0xFF000000;
            bitmap.mask[1] = 0x00FF0000;
            bitmap.mask[2] = 0x0000FF00;
            bitCount = 32;
            break;

        case gcvSURF_X8B8G8R8:
        case gcvSURF_A8B8G8R8:
            bitmap.mask[0] = 0x000000FF;
            bitmap.mask[1] = 0x0000FF00;
            bitmap.mask[2] = 0x00FF0000;
            bitCount = 32;
            break;

        case gcvSURF_B8G8R8A8:
        case gcvSURF_B8G8R8X8:
            bitmap.mask[0] = 0x0000FF00;
            bitmap.mask[1] = 0x00FF0000;
            bitmap.mask[2] = 0xFF000000;
            bitCount = 32;
            break;

        case gcvSURF_X4R4G4B4:
        case gcvSURF_A4R4G4B4:
            bitmap.mask[0] = 0x00000F00;
            bitmap.mask[1] = 0x000000F0;
            bitmap.mask[2] = 0x0000000F;
            bitCount = 16;
            break;

        case gcvSURF_R4G4B4X4:
        case gcvSURF_R4G4B4A4:
            bitmap.mask[0] = 0x0000F000;
            bitmap.mask[1] = 0x00000F00;
            bitmap.mask[2] = 0x000000F0;
            bitCount = 16;
            break;

        case gcvSURF_X4B4G4R4:
        case gcvSURF_A4B4G4R4:
            bitmap.mask[0] = 0x0000000F;
            bitmap.mask[1] = 0x000000F0;
            bitmap.mask[2] = 0x00000F00;
            bitCount = 16;
            break;

        case gcvSURF_B4G4R4X4:
        case gcvSURF_B4G4R4A4:
            bitmap.mask[0] = 0x000000F0;
            bitmap.mask[1] = 0x00000F00;
            bitmap.mask[2] = 0x0000F000;
            bitCount = 16;
            break;

        case gcvSURF_R5G6B5:
            bitmap.mask[0] = 0x0000F800;
            bitmap.mask[1] = 0x000007E0;
            bitmap.mask[2] = 0x0000001F;
            bitCount = 16;
            break;

        case gcvSURF_B5G6R5:
            bitmap.mask[0] = 0x0000001F;
            bitmap.mask[1] = 0x000007E0;
            bitmap.mask[2] = 0x0000F800;
            bitCount = 16;
            break;

        case gcvSURF_A1R5G5B5:
        case gcvSURF_X1R5G5B5:
            bitmap.mask[0] = 0x00007C00;
            bitmap.mask[1] = 0x000003E0;
            bitmap.mask[2] = 0x0000001F;
            bitCount = 16;
            break;

        case gcvSURF_R5G5B5X1:
        case gcvSURF_R5G5B5A1:
            bitmap.mask[0] = 0x0000F800;
            bitmap.mask[1] = 0x000007C0;
            bitmap.mask[2] = 0x0000003E;
            bitCount = 16;
            break;

        case gcvSURF_B5G5R5X1:
        case gcvSURF_B5G5R5A1:
            bitmap.mask[0] = 0x0000003E;
            bitmap.mask[1] = 0x000007C0;
            bitmap.mask[2] = 0x0000F800;
            bitCount = 16;
            break;

        case gcvSURF_X1B5G5R5:
        case gcvSURF_A1B5G5R5:
            bitmap.mask[0] = 0x0000001F;
            bitmap.mask[1] = 0x000003E0;
            bitmap.mask[2] = 0x00007C00;
            bitCount = 16;
            break;

        case gcvSURF_A8:
            bitCount = 8;
            break;

        case gcvSURF_X2R10G10B10:
        case gcvSURF_A2R10G10B10:
            bitmap.mask[0] = 0x000003FF;
            bitmap.mask[1] = 0x000FFC00;
            bitmap.mask[2] = 0x3FF00000;
            bitCount = 32;
            break;

        default:
            // can not save and display
            return gcvFALSE;
        }

        /* Fill in the BITMAPINFOHEADER information. */
        bitmap.bmiHeader.biSize = sizeof(bitmap.bmiHeader);
        bitmap.bmiHeader.biWidth = width;
        bitmap.bmiHeader.biHeight = -(gctINT)height;
        bitmap.bmiHeader.biPlanes = 1;
        bitmap.bmiHeader.biBitCount = bitCount;
        if (format == gcvSURF_A8)
            bitmap.bmiHeader.biCompression = BIT_RGB;
        else
            bitmap.bmiHeader.biCompression = BIT_BITFIELDS;
        bitmap.bmiHeader.biSizeImage = 0;
        bitmap.bmiHeader.biXPelsPerMeter = 0;
        bitmap.bmiHeader.biYPelsPerMeter = 0;
        bitmap.bmiHeader.biClrUsed = 0;
        bitmap.bmiHeader.biClrImportant = 0;

        if (format == gcvSURF_A8)
        {
            for (i=0; i<256; i++)
            {
                tmp.rgbRed = tmp.rgbGreen = tmp.rgbReserved = 0;
                tmp.rgbBlue = i;
                bitmap.bmiColors[i] = tmp;
            }
        }

        /* Lock the resolve buffer. */
        gcmVERIFY_OK(gcoSURF_Lock(surface, &resolveAddress, &bits));

        if (GalSaveDIBitmap(bmpFileName, (BMPINFO *)&bitmap, (unsigned char *)bits, bitsStride) == 0)
        {
            ret = gcvTRUE;
        }

        gcmVERIFY_OK(gcoSURF_Unlock(surface, bits));
        gcmVERIFY_OK(gcoSURF_CPUCacheOperation(surface, gcvCACHE_FLUSH));
    }

    return ret;
}

static void SETBITS(gctUINT32_PTR Target, gctUINT32 Data, gctUINT32 Start, gctUINT32 End)
{
    gctUINT32 start = gcmMIN(Start, End);
    gctUINT32 end = gcmMAX(Start, End);

    *Target = (*Target & (~((~(~0U << (end - start + 1))) << start))) | ((Data & (~(~0U << (end - start + 1)))) << start);
}

/*
 * 'GalSaveSurface2V1()' - Save a YUV file to disk from SURFACE.
 *
 * Returns gcvFALSE on failure...
 */
gctBOOL GalSaveSurfaceToVimg(gcoSURF surface, const char *FileName)
{
    VIMG_FILEHEADER fileHead = {0};
    VIMG_V1 head = {0};
    gctUINT nPlane, n;
    gctUINT alignedWidth, alignedHeight;
    gctINT alignedStride;
    gctUINT32 lineSize, aStride;
    gctUINT32 width[3], height[3];
    gctUINT32 bpp[3] = {0, 0, 0};
    gctPOINTER memory[3] = {0, 0, 0};
    gctUINT32 resolveAddress[3] = {0, 0, 0};
    gceSURF_FORMAT  format;
    gctBOOL ret = gcvFALSE;
    FILE *file;
    gceSTATUS status;
    gctBOOL swap = gcvFALSE;

    if (surface && FileName && FileName[0])
    {
        fileHead.magic[0] = 'V';
        fileHead.magic[1] = 'I';
        fileHead.magic[2] = 'V';
        fileHead.version = 1;

        gcmONERROR(gcoSURF_GetAlignedSize(surface, &alignedWidth, &alignedHeight, &alignedStride));
        gcmONERROR(gcoSURF_GetSize(surface, &width[0], &height[0], NULL));
        gcmONERROR(gcoSURF_GetFormat(surface, NULL, &format));

        head.imageWidth = width[0];
        head.imageHeight = height[0];
        SETBITS(&head.tiling, G2D_TILING_LINEAR, G2D_TILING_TYPE_START, G2D_TILING_TYPE_END);

        switch (format)
        {
        case gcvSURF_YUY2:
            head.format = gcvSURF_YUY2;
            nPlane = 1;
            bpp[0] = 2;
            break;

        case gcvSURF_YVYU:
            head.format = gcvSURF_YVYU;
            nPlane = 1;
            bpp[0] = 2;
            break;

        case gcvSURF_UYVY:
            head.format = gcvSURF_UYVY;
            nPlane = 1;
            bpp[0] = 2;
            break;

        case gcvSURF_VYUY:
            head.format = gcvSURF_VYUY;
            nPlane = 1;
            bpp[0] = 2;
            break;

        case gcvSURF_I420:
            head.format = gcvSURF_I420;
            width[1] = width[2] = head.imageWidth / 2;
            height[1] = height[2] = head.imageHeight / 2;
            nPlane = 3;
            bpp[0] = 1;
            bpp[1] = 1;
            bpp[2] = 1;
            break;

        case gcvSURF_YV12:
            head.format = gcvSURF_YV12;
            width[1] = width[2] = head.imageWidth / 2;
            height[1] = height[2] = head.imageHeight / 2;
            nPlane = 3;
            bpp[0] = 1;
            bpp[1] = 1;
            bpp[2] = 1;
            swap = gcvTRUE;
            break;

        case gcvSURF_NV16:
            head.format = gcvSURF_NV16;
            width[1] = head.imageWidth / 2;
            height[1] = head.imageHeight;
            nPlane = 2;
            bpp[0] = 1;
            bpp[1] = 2;
            break;

        case gcvSURF_NV12:
            head.format = gcvSURF_NV12;
            width[1] = head.imageWidth / 2;
            height[1] = head.imageHeight / 2;
            nPlane = 2;
            bpp[0] = 1;
            bpp[1] = 2;
            break;

        case gcvSURF_NV21:
            head.format = gcvSURF_NV21;
            width[1] = head.imageWidth / 2;
            height[1] = head.imageHeight / 2;
            nPlane = 2;
            bpp[0] = 1;
            bpp[1] = 2;
            break;

        case gcvSURF_NV61:
            head.format = gcvSURF_NV61;
            width[1] = head.imageWidth / 2;
            height[1] = head.imageHeight;
            nPlane = 2;
            bpp[0] = 1;
            bpp[1] = 2;
            break;

        default:
            // can not save and display
            return gcvFALSE;
        }

        /* Lock the resolve buffer. */
        gcmONERROR(gcoSURF_Lock(surface, resolveAddress, memory));
        if (memory[0] == gcvNULL)
        {
            gcmONERROR(gcvSTATUS_INVALID_DATA);
        }

        head.imageStride = aStride = alignedWidth * bpp[0];
        lineSize = width[0] * bpp[0];

        if (gcmIS_ERROR(gcoOS_Open(gcvNULL, FileName, gcvFILE_CREATE, &file)))
        {
            /* Failed to open the file. */
            GalOutput(GalOutputType_Error, "*ERROR*  Failed to open the file %s", FileName);

            return -1;
        }

        head.bitsOffset = (sizeof(fileHead) + sizeof(head));

        gcmONERROR(gcoOS_Write(
                            gcvNULL,
                            file,
                            sizeof(fileHead),
                            &fileHead
                            ));

        gcmONERROR(gcoOS_Write(
                    gcvNULL,
                    file,
                    sizeof(head),
                    &head
                    ));

        for (n = 0; n < height[0]; n++)
        {
            /* Fill plane 1. */
            gctUINT8_PTR p = (gctUINT8_PTR)memory[0] + n * aStride;

            gcmONERROR(gcoOS_Write(
                    gcvNULL,
                    file,
                    lineSize,
                    p
                    ));
        }
        if (nPlane > 1)
        {
            gctUINT aStride1 = (alignedWidth/2) * bpp[1];
            gctUINT lineSize1 = width[1] * bpp[1];

            if (memory[1] == gcvNULL)
            {
                gcmONERROR(gcvSTATUS_INVALID_DATA);
            }

            for (n = 0; n < height[1]; n++)
            {
                /* Fill plane 2. */
                gctUINT8_PTR p = (gctUINT8_PTR)memory[swap ? 2 : 1] + n * aStride1;

                gcmONERROR(gcoOS_Write(
                        gcvNULL,
                        file,
                        lineSize1,
                        p
                        ));
            }

            if (nPlane > 2)
            {
                gctUINT aStride2;
                gctUINT lineSize2;

                if (nPlane != 3)
                {
                    gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
                }

                if (memory[2] == gcvNULL)
                {
                    gcmONERROR(gcvSTATUS_INVALID_DATA);
                }

                aStride2 = (alignedWidth/2) * bpp[2];
                lineSize2 = width[2] * bpp[2];
                for (n = 0; n < height[2]; n++)
                {
                    /* Fill plane 3. */
                    gctUINT8_PTR p = (gctUINT8_PTR)memory[swap ? 1 : 2] + n * aStride2;

                    gcmONERROR(gcoOS_Write(
                            gcvNULL,
                            file,
                            lineSize2,
                            p
                            ));
                }
            }
        }

        gcmONERROR(gcoOS_Close(gcvNULL, file));

        gcmONERROR(gcoSURF_Unlock(surface, memory));
        memory[0] = gcvNULL;

        gcmONERROR(gcoSURF_CPUCacheOperation(surface, gcvCACHE_FLUSH));
    }

    return gcvTRUE;

OnError:

    GalOutput(GalOutputType_Error, "*ERROR*  %s(%d) failed\n", __FUNCTION__, __LINE__);
    return gcvFALSE;
}

gceSTATUS GalSaveTSurfToVimg(
    IN T2D_SURF_PTR surf,
    IN gctCONST_STRING Filename
    )
{
    gctFILE file = gcvNULL;
    gceSTATUS status;

    do {
        VIMG_FILEHEADER filehead;
        VIMG_V1 *v1 = gcvNULL;
        VIMG_V2 *v2 = gcvNULL;
        VIMG_V2 vhead = {0};
        gctUINT n;
        gctBOOL swap = gcvFALSE;
        gctUINT32 width[3] = {0}, height[3] = {0};
        float bpp[3] = {0, 0, 0};
        gctUINT nPlane = 1;

        filehead.magic[0] = 'V';
        filehead.magic[1] = 'I';
        filehead.magic[2] = 'V';

        if(surf->tileStatusConfig == gcv2D_TSC_DISABLE)
        {
            filehead.version = 1;
        }
        else
        {
            filehead.version = 2;
        }

        gcmONERROR(gcoOS_Open(NULL, Filename, gcvFILE_CREATE, &file));

        gcmONERROR(gcoOS_Write(
            gcvNULL,
            file,
            sizeof(filehead),
            &filehead
            ));

        switch(surf->tiling)
        {
        case gcvLINEAR:
            SETBITS(&vhead.v1.tiling, G2D_TILING_LINEAR, G2D_TILING_TYPE_START, G2D_TILING_TYPE_END);
            break;

        case gcvTILED:
            SETBITS(&vhead.v1.tiling, G2D_TILING_TILE, G2D_TILING_TYPE_START, G2D_TILING_TYPE_END);
            break;

        case gcvMULTI_TILED:
            SETBITS(&vhead.v1.tiling, G2D_TILING_TILE, G2D_TILING_TYPE_START, G2D_TILING_TYPE_END);
            SETBITS(&vhead.v1.tiling, G2D_TILING_MULTI_ON, G2D_TILING_MULTI_START, G2D_TILING_MULTI_END);
            break;

        case gcvSUPERTILED:
            SETBITS(&vhead.v1.tiling, G2D_TILING_SUPERTILE, G2D_TILING_TYPE_START, G2D_TILING_TYPE_END);
            break;

        case gcvYMAJOR_SUPERTILED:
            SETBITS(&vhead.v1.tiling, G2D_TILING_YMAJORSUPERTILE, G2D_TILING_TYPE_START, G2D_TILING_TYPE_END);
            break;

        case gcvSUPERTILED_128B:
            SETBITS(&vhead.v1.tiling, G2D_TILING_SUPERTILE128B, G2D_TILING_TYPE_START, G2D_TILING_TYPE_END);
            break;

        case gcvSUPERTILED_256B:
            SETBITS(&vhead.v1.tiling, G2D_TILING_SUPERTILE256B, G2D_TILING_TYPE_START, G2D_TILING_TYPE_END);
            break;

        case gcvTILED_8X8_XMAJOR:
            SETBITS(&vhead.v1.tiling, G2D_TILING_XMAJOR8X8, G2D_TILING_TYPE_START, G2D_TILING_TYPE_END);
            break;

        case gcvTILED_8X8_YMAJOR:
            SETBITS(&vhead.v1.tiling, G2D_TILING_Y, G2D_TILING_TYPE_START, G2D_TILING_TYPE_END);
            break;

        case gcvTILED_8X4:
            SETBITS(&vhead.v1.tiling, G2D_TILING_8X4, G2D_TILING_TYPE_START, G2D_TILING_TYPE_END);
            break;

        case gcvTILED_4X8:
            SETBITS(&vhead.v1.tiling, G2D_TILING_4X8, G2D_TILING_TYPE_START, G2D_TILING_TYPE_END);
            break;

        case gcvTILED_32X4:
            SETBITS(&vhead.v1.tiling, G2D_TILING_32X4, G2D_TILING_TYPE_START, G2D_TILING_TYPE_END);
            break;

        case gcvTILED_64X4:
            SETBITS(&vhead.v1.tiling, G2D_TILING_64X4, G2D_TILING_TYPE_START, G2D_TILING_TYPE_END);
            break;

        case gcvMULTI_SUPERTILED:
            SETBITS(&vhead.v1.tiling, G2D_TILING_MULTI_ON, G2D_TILING_MULTI_START, G2D_TILING_MULTI_END);
            SETBITS(&vhead.v1.tiling, G2D_TILING_SUPERTILE, G2D_TILING_TYPE_START, G2D_TILING_TYPE_END);
            break;

        default:
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
            break;
        }

        if(surf->tiling == gcvSUPERTILED ||
           surf->tiling == gcvMULTI_SUPERTILED ||
           surf->tiling == gcvYMAJOR_SUPERTILED)
        {
            switch(surf->superTileVersion)
            {
            case gcv2D_SUPER_TILE_VERSION_V1:
                SETBITS(&vhead.v1.tiling, G2D_TILING_SUPERTILE_VERSION_1, G2D_TILING_SUPERTILE_VERSION_START, G2D_TILING_SUPERTILE_VERSION_END);
                break;

            case gcv2D_SUPER_TILE_VERSION_V2:
                SETBITS(&vhead.v1.tiling, G2D_TILING_SUPERTILE_VERSION_2, G2D_TILING_SUPERTILE_VERSION_START, G2D_TILING_SUPERTILE_VERSION_END);
                break;

            case gcv2D_SUPER_TILE_VERSION_V3:
                SETBITS(&vhead.v1.tiling, G2D_TILING_SUPERTILE_VERSION_3, G2D_TILING_SUPERTILE_VERSION_START, G2D_TILING_SUPERTILE_VERSION_END);
                break;

            default:
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }
        }

        vhead.v1.format = surf->format;
        vhead.v1.imageStride = surf->stride[0];
        vhead.v1.imageWidth = surf->width;
        vhead.v1.imageHeight = surf->height;

        if(GalIsYUVFormat(surf->format))
        {
            switch(surf->format)
            {
            case gcvSURF_I420:
                width[1] = width[2] = surf->width / 2;
                height[1] = height[2] = surf->height / 2;
                nPlane = 3;
                bpp[0] = 1;
                bpp[1] = 1;
                bpp[2] = 1;
                break;

            case gcvSURF_YV12:
                width[1] = width[2] = surf->width / 2;
                height[1] = height[2] = surf->height / 2;
                nPlane = 3;
                swap = gcvTRUE;
                bpp[0] = 1;
                bpp[1] = 1;
                bpp[2] = 1;
                break;

            case gcvSURF_NV16:
            case gcvSURF_NV61:
                width[1] = surf->width / 2;
                height[1] = surf->height;
                nPlane = 2;
                bpp[0] = 1;
                bpp[1] = 2;
                break;

            case gcvSURF_NV12:
            case gcvSURF_NV21:
                width[1] = surf->width / 2;
                height[1] = surf->height / 2;
                nPlane = 2;
                bpp[0] = 1;
                bpp[1] = 2;
                break;

            case gcvSURF_NV12_10BIT:
            case gcvSURF_NV21_10BIT:
                width[1] = surf->width / 2;
                height[1] = surf->height / 2;
                nPlane = 2;
                bpp[0] = 1.25;
                bpp[1] = 2.5;
                break;

            case gcvSURF_NV16_10BIT:
            case gcvSURF_NV61_10BIT:
                width[1] = surf->width / 2;
                height[1] = surf->height;
                nPlane = 2;
                bpp[0] = 1.25;
                bpp[1] = 2.5;
                break;

            case gcvSURF_P010:
                width[1] = surf->width / 2;
                height[1] = surf->height / 2;
                nPlane = 2;
                bpp[0] = 2;
                bpp[1] = 4;
                break;

            default:
                break;
            }
        }

        if (filehead.version == 1)
        {
            vhead.v1.bitsOffset = (sizeof(VIMG_FILEHEADER) + sizeof(VIMG_V1));
            v1 = &vhead.v1;

            gcmONERROR(gcoOS_Write(
                NULL,
                file,
                sizeof(VIMG_V1),
                v1
                ));
        }
        else if (filehead.version == 2)
        {
            gctUINT32 alignedHeight = surf->aHeight;
            vhead.v1.bitsOffset = (sizeof(VIMG_FILEHEADER) + sizeof(VIMG_V2));
            vhead.tileStatusConfig = surf->tileStatusConfig;
            vhead.tileStatusSize = surf->tileStatusNode.allocatedSize;
            vhead.compressedFormat = surf->tileStatusFormat;
            vhead.clearValue = surf->tileStatusClear;
            if(nPlane == 1 || (surf->tileStatusConfig & gcv2D_TSC_DEC_COMPRESSED))
            {
                vhead.tileStatusOffset = vhead.v1.bitsOffset + surf->vNode.allocatedSize;

                if (nPlane == 2)
                {
                    vhead.tileStatusOffsetEx[0] = vhead.tileStatusOffset + surf->tileStatusNode.allocatedSize;
                    vhead.tileStatusSizeEx[0] = surf->tileStatusNodeEx[0].allocatedSize;
                }
            }
            else if(nPlane == 2)
            {
                switch(surf->format)
                {
                case gcvSURF_NV16:
                case gcvSURF_NV61:
                    vhead.tileStatusOffset = vhead.v1.bitsOffset + vhead.v1.imageStride * alignedHeight * 3 / 2;
                    break;
                case gcvSURF_NV12:
                case gcvSURF_NV21:
                    vhead.tileStatusOffset = vhead.v1.bitsOffset + vhead.v1.imageStride * alignedHeight * 5 / 4;
                    break;
                default:
                    break;
                }
            }
            else if(nPlane == 3)
            {
                vhead.tileStatusOffset = vhead.v1.bitsOffset + vhead.v1.imageStride * alignedHeight * 3 / 2;
            }
            else
            {
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }

            v2 = &vhead;

            gcmONERROR(gcoOS_Write(
                NULL,
                file,
                sizeof(VIMG_V2),
                v2
                ));
        }
        else
        {
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }

        gcmONERROR(gcoOS_Seek(
                gcvNULL,
                file,
                vhead.v1.bitsOffset,
                gcvFILE_SEEK_SET
                ));

        gcmONERROR(gcoOS_Write(
                    gcvNULL,
                    file,
                    surf->vNode.size,
                    (gctUINT8_PTR)surf->logical[0]
                    ));

        if (nPlane > 1 && !(surf->tileStatusConfig & gcv2D_TSC_DEC_COMPRESSED))
        {
            gctUINT lineSize1 = (gctUINT)(width[1] * bpp[1]);

            if (surf->logical[1] == gcvNULL)
            {
                gcmONERROR(gcvSTATUS_INVALID_DATA);
            }

            for (n = 0; n < height[1]; n++)
            {
                gctUINT8_PTR p;
                /* Fill plane 2. */
                if(swap)
                {
                    p = (gctUINT8_PTR)surf->logical[2] + n * surf->stride[2];
                }
                else
                {
                    p = (gctUINT8_PTR)surf->logical[1] + n * surf->stride[1];
                }

                gcmONERROR(gcoOS_Write(
                        gcvNULL,
                        file,
                        lineSize1,
                        p
                        ));
            }

            if (nPlane > 2)
            {
                gctUINT lineSize2;

                if (nPlane != 3)
                {
                    gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
                }

                if (surf->logical[2] == gcvNULL)
                {
                    gcmONERROR(gcvSTATUS_INVALID_DATA);
                }

                lineSize2 = (gctUINT)(width[2] * bpp[2]);
                for (n = 0; n < height[2]; n++)
                {
                    gctUINT8_PTR p;
                    /* Fill plane 3. */
                    if(swap)
                    {
                        p = (gctUINT8_PTR)surf->logical[1] + n * surf->stride[1];
                    }
                    else
                    {
                        p = (gctUINT8_PTR)surf->logical[2] + n * surf->stride[2];
                    }

                    gcmONERROR(gcoOS_Write(
                            gcvNULL,
                            file,
                            lineSize2,
                            p
                            ));
                }
            }
        }

        gcmONERROR(gcoOS_CacheFlush(gcvNULL, surf->vNode.node, surf->vNode.memory, surf->vNode.size));

        if (v2 != gcvNULL)
        {
            gcmONERROR(gcoOS_Seek(
                    gcvNULL,
                    file,
                    v2->tileStatusOffset,
                    gcvFILE_SEEK_SET
                    ));

            gcmONERROR(gcoOS_Write(
                    gcvNULL,
                    file,
                    v2->tileStatusSize,
                    surf->tileStatuslogical
                    ));

            gcmONERROR(gcoOS_CacheFlush(gcvNULL, surf->tileStatusNode.node, surf->tileStatuslogical, surf->tileStatusNode.size));

            if (v2->tileStatusOffsetEx[0])
            {
                gcmONERROR(gcoOS_Seek(
                    gcvNULL,
                    file,
                    v2->tileStatusOffsetEx[0],
                    gcvFILE_SEEK_SET
                    ));

                gcmONERROR(gcoOS_Write(
                    gcvNULL,
                    file,
                    v2->tileStatusSizeEx[0],
                    surf->tileStatuslogicalEx[0]
                    ));

                gcmONERROR(gcoOS_CacheFlush(gcvNULL, surf->tileStatusNodeEx[0].node, surf->tileStatuslogicalEx[0], surf->tileStatusNodeEx[0].size));
            }
        }

        gcmONERROR(gcoOS_Close(gcvNULL, file));

        return gcvSTATUS_OK;

    } while (gcvFALSE);

OnError:

    if (file)
    {
        gcmVERIFY_OK(gcoOS_Close(gcvNULL, file));
    }

    return status;
}

gceSTATUS GalSaveTSurfToDIB(
    IN T2D_SURF_PTR TSurf,
    IN gctCONST_STRING Filename
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    BMPINFO bitmap;
    gctINT bitCount;

    if (!TSurf || !Filename || !Filename[0])
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    memset(&bitmap, 0, gcmSIZEOF(bitmap));

    switch (TSurf->format)
    {
    case gcvSURF_A8R8G8B8:
    case gcvSURF_X8R8G8B8:
        bitmap.mask[0] = 0x00FF0000;
        bitmap.mask[1] = 0x0000FF00;
        bitmap.mask[2] = 0x000000FF;
        bitCount = 32;
        break;

    case gcvSURF_R8G8B8A8:
    case gcvSURF_R8G8B8X8:
        bitmap.mask[0] = 0xFF000000;
        bitmap.mask[1] = 0x00FF0000;
        bitmap.mask[2] = 0x0000FF00;
        bitCount = 32;
        break;

    case gcvSURF_X8B8G8R8:
    case gcvSURF_A8B8G8R8:
        bitmap.mask[0] = 0x000000FF;
        bitmap.mask[1] = 0x0000FF00;
        bitmap.mask[2] = 0x00FF0000;
        bitCount = 32;
        break;

    case gcvSURF_B8G8R8A8:
    case gcvSURF_B8G8R8X8:
        bitmap.mask[0] = 0x0000FF00;
        bitmap.mask[1] = 0x00FF0000;
        bitmap.mask[2] = 0xFF000000;
        bitCount = 32;
        break;

    case gcvSURF_X4R4G4B4:
    case gcvSURF_A4R4G4B4:
        bitmap.mask[0] = 0x00000F00;
        bitmap.mask[1] = 0x000000F0;
        bitmap.mask[2] = 0x0000000F;
        bitCount = 16;
        break;

    case gcvSURF_R4G4B4X4:
    case gcvSURF_R4G4B4A4:
        bitmap.mask[0] = 0x0000F000;
        bitmap.mask[1] = 0x00000F00;
        bitmap.mask[2] = 0x000000F0;
        bitCount = 16;
        break;

    case gcvSURF_X4B4G4R4:
    case gcvSURF_A4B4G4R4:
        bitmap.mask[0] = 0x0000000F;
        bitmap.mask[1] = 0x000000F0;
        bitmap.mask[2] = 0x00000F00;
        bitCount = 16;
        break;

    case gcvSURF_B4G4R4X4:
    case gcvSURF_B4G4R4A4:
        bitmap.mask[0] = 0x000000F0;
        bitmap.mask[1] = 0x00000F00;
        bitmap.mask[2] = 0x0000F000;
        bitCount = 16;
        break;

    case gcvSURF_R5G6B5:
        bitmap.mask[0] = 0x0000F800;
        bitmap.mask[1] = 0x000007E0;
        bitmap.mask[2] = 0x0000001F;
        bitCount = 16;
        break;

    case gcvSURF_B5G6R5:
        bitmap.mask[0] = 0x0000001F;
        bitmap.mask[1] = 0x000007E0;
        bitmap.mask[2] = 0x0000F800;
        bitCount = 16;
        break;

    case gcvSURF_A1R5G5B5:
    case gcvSURF_X1R5G5B5:
        bitmap.mask[0] = 0x00007C00;
        bitmap.mask[1] = 0x000003E0;
        bitmap.mask[2] = 0x0000001F;
        bitCount = 16;
        break;

    case gcvSURF_R5G5B5X1:
    case gcvSURF_R5G5B5A1:
        bitmap.mask[0] = 0x0000F800;
        bitmap.mask[1] = 0x000007C0;
        bitmap.mask[2] = 0x0000003E;
        bitCount = 16;
        break;

    case gcvSURF_B5G5R5X1:
    case gcvSURF_B5G5R5A1:
        bitmap.mask[0] = 0x0000003E;
        bitmap.mask[1] = 0x000007C0;
        bitmap.mask[2] = 0x0000F800;
        bitCount = 16;
        break;

    case gcvSURF_X1B5G5R5:
    case gcvSURF_A1B5G5R5:
        bitmap.mask[0] = 0x0000001F;
        bitmap.mask[1] = 0x000003E0;
        bitmap.mask[2] = 0x00007C00;
        bitCount = 16;
        break;

    case gcvSURF_RG16:
        bitmap.mask[0] = 0x0000FF00;
        bitmap.mask[1] = 0x000000FF;
        bitmap.mask[2] = 0x00FF0000;
        bitCount = 16;
        break;

    case gcvSURF_R10G10B10A2:
        bitmap.mask[0] = 0xFFC00000;
        bitmap.mask[1] = 0x003FF000;
        bitmap.mask[2] = 0x00000FFC;
        bitCount = 32;
        break;

    case gcvSURF_B10G10R10A2:
        bitmap.mask[0] = 0x00000FFC;
        bitmap.mask[1] = 0x003FF000;
        bitmap.mask[2] = 0xFFC00000;
        bitCount = 32;
        break;

    case gcvSURF_A2R10G10B10:
        bitmap.mask[0] = 0x3FF00000;
        bitmap.mask[1] = 0x000FFC00;
        bitmap.mask[2] = 0x000003FF;
        bitCount = 32;
        break;

    case gcvSURF_A2B10G10R10:
        bitmap.mask[0] = 0x000003FF;
        bitmap.mask[1] = 0x000FFC00;
        bitmap.mask[2] = 0x3FF00000;
        bitCount = 32;
        break;

    default:
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    /* Fill in the BITMAPINFOHEADER information. */
    bitmap.bmiHeader.biSize = sizeof(bitmap.bmiHeader);
    bitmap.bmiHeader.biWidth = TSurf->width;
    bitmap.bmiHeader.biHeight = -(gctINT)TSurf->height;
    bitmap.bmiHeader.biPlanes = 1;
    bitmap.bmiHeader.biBitCount = bitCount;
    bitmap.bmiHeader.biCompression = BIT_BITFIELDS;
    bitmap.bmiHeader.biSizeImage = gcmALIGN(TSurf->width * TSurf->height * bitCount, 8) >> 3;
    bitmap.bmiHeader.biXPelsPerMeter = 0;
    bitmap.bmiHeader.biYPelsPerMeter = 0;
    bitmap.bmiHeader.biClrUsed = 0;
    bitmap.bmiHeader.biClrImportant = 0;

    if (GalSaveDIBitmap(Filename, (BMPINFO *)&bitmap, TSurf->vNode.memory,
        TSurf->stride[0]))
    {
        status = gcvSTATUS_INVALID_DATA;
    }

    gcmVERIFY_OK(gcoOS_CacheFlush(gcvNULL, TSurf->vNode.node,
        TSurf->vNode.memory, TSurf->vNode.size));

OnError:

    return status;
}

/*
 * 'Gal2DCleanSurface' - clean surface with A8R9G9B8 color.
 *
 * Returns gcvFALSE on failure...
 */
gceSTATUS CDECL Gal2DCleanSurface(gcoHAL hal, gcoSURF surface, gctUINT32 color)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctPOINTER bits[3] = {0, 0, 0};

    do {
        gctUINT width, height;
        gctINT stride;
        gcsRECT dstRect;
        gceSURF_TYPE type;
        gceSURF_FORMAT fmt;
        gctUINT32 addr[3];
        gco2D egn2D = NULL;

        gcmERR_BREAK(gcoHAL_Get2DEngine(hal, &egn2D));

        gcmERR_BREAK(gcoSURF_GetFormat(surface, &type, &fmt));
        if (type != gcvSURF_BITMAP)
        {
            gcmERR_BREAK(gcvSTATUS_NOT_SUPPORTED);
        }

        gcmERR_BREAK(gcoSURF_GetAlignedSize(surface, &width, &height, &stride));

        gcmERR_BREAK(gcoSURF_Lock(surface, addr, bits));

        dstRect.left = 0;
        dstRect.top = 0;
        dstRect.right = width;
        dstRect.bottom = height;

        gcmERR_BREAK(gco2D_SetClipping(egn2D, &dstRect));

        gcmERR_BREAK(gco2D_SetTransparencyAdvanced(egn2D, gcv2D_OPAQUE,
                                gcv2D_OPAQUE, gcv2D_OPAQUE));

        gcmERR_BREAK(gco2D_SetTarget(egn2D, addr[0], stride, 0, width));

        gcmERR_BREAK(gco2D_SetTargetTileStatus(
                    egn2D,
                    gcv2D_TSC_DISABLE,
                    gcvSURF_UNKNOWN,
                    0,
                    ~0U
                    ));

        gcmERR_BREAK(gco2D_Clear(egn2D, 1, &dstRect, color, 0xCC, 0xCC, fmt));

        gcmERR_BREAK(gcoHAL_Commit(hal, gcvTRUE));

        gcmERR_BREAK(gcoSURF_Unlock(surface, bits[0]));
        bits[0] = gcvNULL;

    } while (gcvFALSE);

    if (bits[0])
    {
        gcmVERIFY_OK(gcoSURF_Unlock(surface,bits));
    }

    return status;
}

/*
 * 'Gal2DRectangle' - draw a rectangle on surface with the specified brush.
 *
 * Returns FALSE on failure...
 */
gctBOOL CDECL Gal2DRectangle(gcoHAL hal, gcoSURF surface, gcoBRUSH brush, gcsRECT rect)
{
    gctUINT width, height;
    gctINT stride;
    gcsRECT dstRect;
    gceSURF_TYPE type;
    gceSURF_FORMAT fmt;
    gctPOINTER bits;
    gctUINT32 addr;
    gco2D egn2D = NULL;

    gcmVERIFY_OK(gcoSURF_GetFormat(surface, &type, &fmt));
    if (type != gcvSURF_BITMAP)
        return gcvFALSE;

    gcmVERIFY_OK(gcoSURF_GetAlignedSize(surface, &width, &height, &stride));

    dstRect.left = max(0, rect.left);
    dstRect.top = max(0, rect.top);
    dstRect.right = min(width, (gctUINT32)rect.right);
    dstRect.bottom = min(height, (gctUINT32)rect.bottom);

    if (dstRect.left >= dstRect.right || dstRect.top >= dstRect.bottom)
    {
        // invalid rectangle
        return gcvFALSE;
    }

    gcmVERIFY_OK(gcoSURF_Lock(surface, &addr, &bits));

    gcmVERIFY_OK(gcoHAL_Get2DEngine(hal, &egn2D));
    gcmASSERT(egn2D);

    gcmVERIFY_OK(gco2D_FlushBrush(egn2D, brush, fmt));

    gcmVERIFY_OK(gco2D_SetTarget(egn2D, addr, stride, 0, width));

    gcmVERIFY_OK(gco2D_SetClipping(egn2D, &dstRect));

    gcmVERIFY_OK(gco2D_Blit(egn2D, 1, &dstRect, 0xF0, 0xF0, fmt));

    gcmVERIFY_OK(gcoHAL_Commit(hal, gcvTRUE));

    gcmVERIFY_OK(gcoSURF_Unlock(surface,bits));

    return gcvTRUE;
}

/*******************************************************************************
**
**  PackStream
**
**  Pack an uncpacked monochrome stream.
**
**  INPUT:
**
**        gctUINT8_PTR UnpackedStream
**            Pointer to the unpacked monochrome stream.
**
**        gctUINT32 StreamWidth
**        gctUINT32 StreamHeight
**            The size of the stream in pixels.
**
**        gceSURF_MONOPACK Pack
**            Target packing.
**
**  OUTPUT:
**
**        gctUINT8_PTR * PackedStream
**            The resulting allocated and converted stream.
*/
gceSTATUS CDECL GalPackStream(
    gctUINT8_PTR UnpackedStream,
    gctUINT32 StreamWidth,
    gctUINT32 StreamHeight,
    gceSURF_MONOPACK Pack,
    gctUINT8_PTR * PackedStream
    )
{
    gceSTATUS status;
    gctUINT32 destPackWidth, destPackHeight;
    gctUINT32 srcWidth, srcHeight;
    gctUINT32 destWidth, destHeight;
    gctUINT32 srcStepX, srcStepY;
    gctUINT32 destStepX, destStepY;
    gctUINT8_PTR srcLine;
    gctUINT8_PTR destLine;
    gctUINT32 destSize;
    gctUINT32 x, y;

    do
    {
        /* Get stream pack size. */
        gcmERR_BREAK(gco2D_GetPackSize(
            Pack,
            &destPackWidth,
            &destPackHeight
            ));

        /* Determine the aligned size of the bitmaps. */
        srcWidth  = gcmALIGN(StreamWidth,  32);
        srcHeight = gcmALIGN(StreamHeight, 1);

        destWidth  = gcmALIGN(StreamWidth,  destPackWidth);
        destHeight = gcmALIGN(StreamHeight, destPackHeight);

        /* Determine the size of the new bitmap. */
        destSize = (destWidth * destHeight) >> 3;

        /* Allocate the new bitmap. */
        *PackedStream = malloc(destSize);

        /* Set stream pointers. */
        srcLine  = UnpackedStream;
        destLine = *PackedStream;

        /* Determine the steps. */
        srcStepX = destPackWidth >> 3;
        srcStepY = srcWidth      >> 3;

        destStepX = (destHeight * destPackWidth) >> 3;
        destStepY = destPackWidth >> 3;

        /* Convert the bitmap. */
        for (y = 0; y < destHeight; y++)
        {
            gctUINT8_PTR srcByte = srcLine;
            gctUINT8_PTR destByte = destLine;

            for (x = 0; x < destWidth; x += destPackWidth)
            {
                switch (Pack)
                {
                case gcvSURF_PACKED8:
                    if (GalIsBigEndian())
                    {
                        * (gctUINT8_PTR) gcmINT2PTR((gcmPTR2SIZE(destByte) & (~3)) + (3 - (gcmPTR2SIZE(destByte) % 4))) =
                            * (gctUINT8_PTR) gcmINT2PTR((gcmPTR2SIZE(srcByte) & (~3)) + (3 - (gcmPTR2SIZE(srcByte) % 4)));
                    }
                    else
                    {
                        *destByte = *srcByte;
                    }
                    break;

                case gcvSURF_PACKED16:
                    if (GalIsBigEndian())
                    {
                        * (gctUINT16_PTR) gcmINT2PTR((gcmPTR2SIZE(destByte) & (~3)) + (2 - (gcmPTR2SIZE(destByte) % 4))) =
                            * (gctUINT16_PTR) gcmINT2PTR((gcmPTR2SIZE(srcByte) & (~3)) + (2 - (gcmPTR2SIZE(srcByte) % 4)));
                    }
                    else
                    {
                        * (gctUINT16_PTR) destByte = * (gctUINT16_PTR) srcByte;
                    }
                    break;

                case gcvSURF_PACKED32:
                    * (gctUINT32_PTR) destByte = * (gctUINT32_PTR) srcByte;
                    break;

                default:
                    break;
                }

                srcByte  += srcStepX;
                destByte += destStepX;
            }

            srcLine  += srcStepY;
            destLine += destStepY;
        }
    }
    while (gcvFALSE);

    /* Return status. */
    return status;
}

gceSTATUS CDECL GalLoadYUV2Surface2(
    IN gcoOS Os,
    IN gcoHAL Hal,
    IN gctCONST_STRING filename,
    OUT gcoSURF *surface
    )
{
    gctFILE file = gcvNULL;
    gceSTATUS status;
    gctPOINTER memory[3] = {0, 0, 0};
    gcoSURF surf = gcvNULL;

    do {
        YUV_HEADER head = {0, 0, 0};
        gctUINT alignedWidth, alignedHeight;
        gctINT alignedStride;
        gctUINT32 lineSize, aStride;
        gctUINT32 address[3] = {0, 0, 0};
        gctUINT32 width[3] = {0, 0, 0};
        gctUINT32 height[3] = {0, 0, 0};
        gctUINT32 bpp[3] = {0, 0, 0};
        gctUINT nPlane, n, i;
        gcoSURF surf;
        gceSURF_FORMAT format;
        gctBOOL swap = gcvFALSE;

        gcmONERROR(gcoOS_Open(Os, filename, gcvFILE_READ, &file));

        head.Width  = read_dword(file);
        head.Height = read_dword(file);
        head.Type   = read_byte(file);

        if (head.Width == 0 || head.Height == 0)
        {
            gcmONERROR(gcvSTATUS_INVALID_DATA);
        }

        /* Check the type. */
        switch (head.Type)
        {
        case YUV_HEADER_TYPE_YUY2:
            format = gcvSURF_YUY2;
            nPlane = 1;
            width[0] = head.Width;
            height[0] = head.Height;
            bpp[0] = 2;
            break;

        case YUV_HEADER_TYPE_UYVY:
            format = gcvSURF_UYVY;
            nPlane = 1;
            width[0] = head.Width;
            height[0] = head.Height;
            bpp[0] = 2;
            break;

        case YUV_HEADER_TYPE_I420:
            format = gcvSURF_I420;
            nPlane = 3;
            width[0] = head.Width;
            height[0] = head.Height;
            bpp[0] = 1;
            width[1] = width[2] = head.Width / 2;
            height[1] = height[2] = head.Height / 2;
            bpp[1] = bpp[2] = 1;
            break;

        case YUV_HEADER_TYPE_YV12:
            format = gcvSURF_YV12;
            nPlane = 3;
            width[0] = head.Width;
            height[0] = head.Height;
            bpp[0] = 1;
            width[1] = width[2] = head.Width / 2;
            height[1] = height[2] = head.Height / 2;
            bpp[1] = bpp[2] = 1;
            swap = gcvTRUE;
            break;

        case YUV_HEADER_TYPE_NV16:
            format = gcvSURF_NV16;
            nPlane = 2;
            width[0] = head.Width;
            height[0] = head.Height;
            bpp[0] = 1;
            width[1] = head.Width / 2;
            height[1] = head.Height;
            bpp[1] = 2;
            break;

        case YUV_HEADER_TYPE_NV12:
            format = gcvSURF_NV12;
            nPlane = 2;
            width[0] = head.Width;
            height[0] = head.Height;
            bpp[0] = 1;
            width[1] = head.Width / 2;
            height[1] = head.Height / 2;
            bpp[1] = 2;
            break;

        case YUV_HEADER_TYPE_NV21:
            format = gcvSURF_NV21;
            nPlane = 2;
            width[0] = head.Width;
            height[0] = head.Height;
            bpp[0] = 1;
            width[1] = head.Width / 2;
            height[1] = head.Height / 2;
            bpp[1] = 2;
            break;

        case YUV_HEADER_TYPE_NV61:
            format = gcvSURF_NV61;
            nPlane = 2;
            width[0] = head.Width;
            height[0] = head.Height;
            bpp[0] = 1;
            width[1] = head.Width / 2;
            height[1] = head.Height;
            bpp[1] = 2;
            break;

        default:
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
            break;
        }

        gcmONERROR(gcoSURF_Construct(Hal, head.Width, head.Height, 1,
            gcvSURF_BITMAP, format, gcvPOOL_DEFAULT, &surf));

        gcmONERROR(gcoSURF_GetAlignedSize(surf, &alignedWidth,
            &alignedHeight, &alignedStride));

        gcmONERROR(gcoSURF_Lock(surf, address, memory));

        if (memory[0] == gcvNULL)
        {
            gcmONERROR(gcvSTATUS_INVALID_DATA);
        }

        aStride = alignedWidth * bpp[0];
        lineSize = width[0] * bpp[0];
        for (n = 0; n < height[0]; n++)
        {
            /* Fill plane 1. */
            gctUINT8_PTR p = (gctUINT8_PTR)memory[0] + n * aStride;

            gcmONERROR(gcoOS_Read(
                    Os,
                    file,
                    lineSize,
                    p,
                    gcvNULL
                    ));

            if (GalIsBigEndian() && (bpp[0] == 2))
            {
                gctUINT16 *s = (gctUINT16 *)p;

                for (i = 0; i < lineSize; i += 2)
                {
                    *s = swap_word(*s);
                    s++;
                }
            }
        }

        if (nPlane > 1)
        {
            gctUINT aStride1 = (alignedWidth/2) * bpp[1];
            gctUINT lineSize1 = width[1] * bpp[1];

            if (memory[1] == gcvNULL)
            {
                gcmONERROR(gcvSTATUS_INVALID_DATA);
            }

            for (n = 0; n < height[1]; n++)
            {
                /* Fill plane 2. */
                gctUINT8_PTR p = (gctUINT8_PTR)memory[swap ? 2 : 1] + n * aStride1;

                gcmONERROR(gcoOS_Read(
                        Os,
                        file,
                        lineSize1,
                        p,
                        gcvNULL
                        ));

                if (GalIsBigEndian() && (bpp[1] == 2))
                {
                    gctUINT16 *s = (gctUINT16 *)p;

                    for (i = 0; i < lineSize1; i += 2)
                    {
                        *s = swap_word(*s);
                        s++;
                    }
                }
            }

            if (nPlane > 2)
            {
                gctUINT aStride2;
                gctUINT lineSize2;

                if (nPlane != 3)
                {
                    gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
                }

                if (memory[2] == gcvNULL)
                {
                    gcmONERROR(gcvSTATUS_INVALID_DATA);
                }

                aStride2 = (alignedWidth/2) * bpp[2];
                lineSize2 = width[2] * bpp[2];
                for (n = 0; n < height[2]; n++)
                {
                    /* Fill plane 3. */
                    gctUINT8_PTR p = (gctUINT8_PTR)memory[swap ? 1 : 2] + n * aStride2;

                    gcmONERROR(gcoOS_Read(
                            Os,
                            file,
                            lineSize2,
                            p,
                            gcvNULL
                            ));

                    if (GalIsBigEndian() && (bpp[2] == 2))
                    {
                        gctUINT16 *s = (gctUINT16 *)p;

                        for (i = 0; i < lineSize2; i += 2)
                        {
                            *s = swap_word(*s);
                            s++;
                        }
                    }
                }
            }
        }

        gcmONERROR(gcoSURF_Unlock(surf, memory));
        memory[0] = gcvNULL;

        gcmONERROR(gcoOS_Close(Os, file));

        gcmONERROR(gcoSURF_CPUCacheOperation(surf, gcvCACHE_FLUSH));

        *surface = surf;

        return gcvSTATUS_OK;

    } while (gcvFALSE);

OnError:
    if (surf && memory[0])
    {
        gcmVERIFY_OK(gcoSURF_Unlock(surf, memory));
    }

    if (surf)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(surf));
    }

    if (file)
    {
        gcmVERIFY_OK(gcoOS_Close(Os, file));
    }

    return status;
}

static gceSTATUS LockVideoNode(
    IN gcoHAL Hal,
    IN gctUINT32 Node,
    OUT gctUINT32 *Address,
    OUT gctPOINTER *Memory)
{
    gceSTATUS status;
    gcsHAL_INTERFACE iface;

    gcmASSERT(Address != gcvNULL);
    gcmASSERT(Memory != gcvNULL);
    gcmASSERT(Node != 0);

    iface.command     = gcvHAL_LOCK_VIDEO_MEMORY;
    iface.u.LockVideoMemory.node = Node;
    iface.u.LockVideoMemory.cacheable = gcvFALSE;

    /* Call kernel API. */
    gcmONERROR(gcoHAL_Call(Hal, &iface));

    /* Get allocated node in video memory. */
    *Address = iface.u.LockVideoMemory.address;
    *Memory = (gctPOINTER)(gctSIZE_T)iface.u.LockVideoMemory.memory;

OnError:

    return status;
}

static gceSTATUS UnlockVideoNode(
    IN gcoHAL Hal,
    IN gctUINT32 Node)
{
    gceSTATUS status;
    gcsHAL_INTERFACE iface;

    gcmASSERT(Node != 0);

    iface.command = gcvHAL_UNLOCK_VIDEO_MEMORY;
    iface.u.UnlockVideoMemory.node = Node;
    iface.u.UnlockVideoMemory.type = gcvSURF_BITMAP;

#if gcvVERSION_MAJOR>5 || (gcvVERSION_MAJOR==5 && gcvVERSION_MINOR==0 && gcvVERSION_PATCH==0)
    iface.engine = gcvENGINE_RENDER;
#endif

    /* Call kernel API. */
    gcmONERROR(gcoHAL_Call(Hal, &iface));
    gcmONERROR(gcoHAL_ScheduleEvent(Hal, &iface));

OnError:

    return status;
}

gceSTATUS AllocVideoNode(
    IN gcoHAL Hal,
    IN OUT gctUINT_PTR Size,
    IN OUT gcePOOL *Pool,
    IN OUT gctINT32 *Alignment,
    OUT gctUINT32 *Node,
    OUT gctUINT32 *Address,
    OUT gctPOINTER *Memory)
{
    gcsHAL_INTERFACE iface;
    gceSTATUS status;

    gcmASSERT(Pool != gcvNULL);
    gcmASSERT(Size != gcvNULL);
    gcmASSERT(Node != gcvNULL);

    *Size += 256;

    iface.u.AllocateLinearVideoMemory.node = 0;
    iface.command     = gcvHAL_ALLOCATE_LINEAR_VIDEO_MEMORY;
    iface.u.AllocateLinearVideoMemory.bytes     = *Size;
    iface.u.AllocateLinearVideoMemory.alignment = Alignment ? *Alignment : 64;
    iface.u.AllocateLinearVideoMemory.pool      = *Pool;
    iface.u.AllocateLinearVideoMemory.flag      = gcvALLOC_FLAG_NONE;
    iface.u.AllocateLinearVideoMemory.type      = gcvSURF_BITMAP;

    /* Call kernel API. */
    gcmONERROR(gcoHAL_Call(Hal, &iface));

    gcmONERROR(LockVideoNode(Hal,(gctUINT32)iface.u.AllocateLinearVideoMemory.node, Address, Memory));

    /* Get allocated node in video memory. */
    *Node = (gctUINT32)iface.u.AllocateLinearVideoMemory.node;
    *Pool = iface.u.AllocateLinearVideoMemory.pool;
    *Size = iface.u.AllocateLinearVideoMemory.bytes;

    if (Alignment)
    {
        *Alignment = iface.u.AllocateLinearVideoMemory.alignment;
    }

    return gcvSTATUS_OK;

OnError:

    if (!iface.u.AllocateLinearVideoMemory.node)
    {
#if gcvVERSION_MAJOR >= 5
        iface.command     = gcvHAL_RELEASE_VIDEO_MEMORY;
        iface.u.ReleaseVideoMemory.node = iface.u.AllocateLinearVideoMemory.node;
#else
        iface.command     = gcvHAL_FREE_VIDEO_MEMORY;
        iface.u.FreeVideoMemory.node = iface.u.AllocateLinearVideoMemory.node;
#endif
        gcoHAL_Call(Hal, &iface);
    }

    return status;
}

gceSTATUS FreeVideoNode(
    IN gcoHAL Hal,
    IN gctUINT32 Node)
{
    gcsHAL_INTERFACE iface;

    gcmASSERT(Node != 0);

    gcmVERIFY_OK(UnlockVideoNode(Hal, Node));

#if gcvVERSION_MAJOR >= 5
        iface.command     = gcvHAL_RELEASE_VIDEO_MEMORY;
        iface.u.ReleaseVideoMemory.node = Node;
#else
        iface.command     = gcvHAL_FREE_VIDEO_MEMORY;
        iface.u.FreeVideoMemory.node = Node;
#endif

    return gcoHAL_Call(Hal, &iface);
}

#if gcdTEST_DEC200
gceSTATUS Dec200Compression(
    IN gcoHAL Hal,
    IN gctUINT32 Format,
    IN gctUINT32 Phys,
    IN gctUINT32 PhysTileStatus)
{
    gcsHAL_INTERFACE iface;

    iface.command               = gcvHAL_DEC200_TEST;
    iface.u.Dec200Test.command  = gcvDEC200_COMPRESSION;
    iface.u.Dec200Test.format   = Format;
    iface.u.Dec200Test.physBase = Phys;
    iface.u.Dec200Test.physTile = PhysTileStatus;

    return gcoHAL_Call(Hal, &iface);
}

gceSTATUS Dec200Decompression(
    IN gcoHAL Hal,
    IN gctUINT32 Format,
    IN gctUINT32 Phys,
    IN gctUINT32 PhysTileStatus)
{
    gcsHAL_INTERFACE iface;

    iface.command               = gcvHAL_DEC200_TEST;
    iface.u.Dec200Test.command  = gcvDEC200_DECOMPRESSION;
    iface.u.Dec200Test.format   = Format;
    iface.u.Dec200Test.physBase = Phys;
    iface.u.Dec200Test.physTile = PhysTileStatus;

    return gcoHAL_Call(Hal, &iface);
}

gceSTATUS Dec200Flush(
    IN gcoHAL Hal)
{
    gcsHAL_INTERFACE iface;

    iface.command               = gcvHAL_DEC200_TEST;
    iface.u.Dec200Test.command  = gcvDEC200_FLUSH;

    return gcoHAL_Call(Hal, &iface);
}

gceSTATUS Dec200FlushDone(
    IN gcoHAL Hal)
{
    gcsHAL_INTERFACE iface;
    gctUINT32 i;

    iface.command               = gcvHAL_DEC200_TEST;
    iface.u.Dec200Test.command  = gcvDEC200_FLUSH_DONE;

    for (i=0; i< 5; i++)
    {
        gcoHAL_Call(Hal, &iface);
        if (iface.u.Dec200Test.flushDone)
            return gcvSTATUS_OK;

        gcoOS_Delay(gcvNULL, 100);
    }

    return gcvSTATUS_TIMEOUT;
}

gceSTATUS Dec200CmdStart(
    IN gcoHAL Hal)
{
    gcsHAL_INTERFACE iface;

    iface.command               = gcvHAL_DEC200_TEST;
    iface.u.Dec200Test.command  = gcvDEC200_CMD_START;

    return gcoHAL_Call(Hal, &iface);
}

gceSTATUS Dec200CmdStop(
    IN gcoHAL Hal)
{
    gcsHAL_INTERFACE iface;

    iface.command               = gcvHAL_DEC200_TEST;
    iface.u.Dec200Test.command  = gcvDEC200_CMD_STOP;

    return gcoHAL_Call(Hal, &iface);
}

#endif

gceSTATUS GalQueryBpp(gceSURF_FORMAT Format, gctUINT *Bpp)
{
    switch (Format)
    {
    case gcvSURF_A8      :
    case gcvSURF_R8      :
    case gcvSURF_INDEX8  :
        *Bpp = 1;
        break;

    case gcvSURF_X4R4G4B4:
    case gcvSURF_A4R4G4B4:
    case gcvSURF_R4G4B4X4:
    case gcvSURF_R4G4B4A4:
    case gcvSURF_X4B4G4R4:
    case gcvSURF_A4B4G4R4:
    case gcvSURF_B4G4R4X4:
    case gcvSURF_B4G4R4A4:

    case gcvSURF_X1R5G5B5:
    case gcvSURF_A1R5G5B5:
    case gcvSURF_R5G5B5X1:
    case gcvSURF_R5G5B5A1:
    case gcvSURF_B5G5R5X1:
    case gcvSURF_B5G5R5A1:
    case gcvSURF_X1B5G5R5:
    case gcvSURF_A1B5G5R5:
    case gcvSURF_R5G6B5  :
    case gcvSURF_B5G6R5  :
    case gcvSURF_RG16    :
    case gcvSURF_YUY2:
    case gcvSURF_UYVY:
    case gcvSURF_YVYU:
    case gcvSURF_VYUY:
    case gcvSURF_I420:
    case gcvSURF_YV12:
    case gcvSURF_NV16:
    case gcvSURF_NV12:
    case gcvSURF_NV61:
    case gcvSURF_NV21:
    case gcvSURF_NV12_10BIT:
    case gcvSURF_NV21_10BIT:
    case gcvSURF_NV16_10BIT:
    case gcvSURF_NV61_10BIT:
        *Bpp = 2;
        break;

    case gcvSURF_X8R8G8B8:
    case gcvSURF_A8R8G8B8:
    case gcvSURF_R8G8B8X8:
    case gcvSURF_R8G8B8A8:
    case gcvSURF_B8G8R8X8:
    case gcvSURF_B8G8R8A8:
    case gcvSURF_X8B8G8R8:
    case gcvSURF_A8B8G8R8:
    case gcvSURF_A2B10G10R10:
    case gcvSURF_A2R10G10B10:
    case gcvSURF_R10G10B10A2:
    case gcvSURF_B10G10R10A2:
    case gcvSURF_P010:
        *Bpp = 4;
        break;

    default:
        return gcvSTATUS_NOT_SUPPORTED;
    }

    return gcvSTATUS_OK;
}

gceSTATUS GalCreateTSurf(
    IN gcoHAL Hal,
    IN gceSURF_FORMAT Format,
    IN gceTILING Tiling,
    IN gce2D_TILE_STATUS_CONFIG TileStatusConfig,
    IN gctUINT32 Width,
    IN gctUINT32 Height,
    OUT T2D_SURF_PTR *Surface
    )
{
    return GalCreateTSurfWithPool(Hal, Format, Tiling, TileStatusConfig, Width, Height, gcvPOOL_DEFAULT, Surface);
}

gceSTATUS GalCreateTSurfWithPool(
    IN gcoHAL Hal,
    IN gceSURF_FORMAT Format,
    IN gceTILING Tiling,
    IN gce2D_TILE_STATUS_CONFIG TileStatusConfig,
    IN gctUINT32 Width,
    IN gctUINT32 Height,
    IN gcePOOL Pool,
    OUT T2D_SURF_PTR *Surface
    )
{
    gceSTATUS status;
    T2D_SURF_PTR surf = gcvNULL;
    gctUINT32 bpp, offset = 0, aligned = 1;
    gctUINT alignedWidth, alignedHeight, alignedStride, alignedBase = 64, alignedTSBase = 64;

    gcmONERROR(GalQueryBpp(Format, &bpp));

    alignedWidth = Width;
    alignedHeight = Height;

    /* Align width and height. */
    switch (Tiling)
    {
    case gcvTILED:
        alignedWidth = gcmALIGN(Width, 8);
        alignedHeight = gcmALIGN(Height, 8);
        break;

    case gcvMULTI_TILED:
        alignedWidth = gcmALIGN(Width, 16);
        alignedHeight = gcmALIGN(Height, 16);
        break;

    case gcvSUPERTILED:
    case gcvSUPERTILED_128B:
    case gcvSUPERTILED_256B:
    case gcvYMAJOR_SUPERTILED:
        alignedWidth = gcmALIGN(Width, 64);
        alignedHeight = gcmALIGN(Height, 64);
        break;

    case gcvMULTI_SUPERTILED:
        alignedWidth = gcmALIGN(Width, 64);
        alignedHeight = gcmALIGN(Height, 128);
        break;

    case gcvTILED_8X8_XMAJOR:
        alignedWidth = gcmALIGN(Width, 16);
        alignedHeight = gcmALIGN(Height, 8);
        break;

    case gcvTILED_8X8_YMAJOR:
        if (Format == gcvSURF_NV12)
        {
            alignedWidth = gcmALIGN(Width, 16);
            alignedHeight = gcmALIGN(Height, 64);
        }
        else if (Format == gcvSURF_P010)
        {
            alignedWidth = gcmALIGN(Width, 8);
            alignedHeight = gcmALIGN(Height, 64);
        }
        break;

    case gcvTILED_8X4:
    case gcvTILED_4X8:
        alignedWidth = gcmALIGN(Width, 8);
        alignedHeight = gcmALIGN(Height, 8);
        break;

    case gcvTILED_32X4:
        alignedWidth = gcmALIGN(Width, 32);
        alignedHeight = gcmALIGN(Height, 8);
        break;

    case gcvTILED_64X4:
        alignedWidth = gcmALIGN(Width, 64);
        alignedHeight = gcmALIGN(Height, 8);
        break;

    default:
        alignedWidth = gcmALIGN(Width, 16);
        alignedHeight = gcmALIGN(Height, 1);
        break;
    }

    if (TileStatusConfig == gcv2D_TSC_2D_COMPRESSED ||
        (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_DEC_COMPRESSION) &&
         (TileStatusConfig & gcv2D_TSC_DEC_COMPRESSED)) ||
        (TileStatusConfig & gcv2D_TSC_V4_COMPRESSED))
    {
        if (TileStatusConfig == gcv2D_TSC_2D_COMPRESSED &&
            (Tiling != gcvLINEAR ||
             (Format != gcvSURF_A8R8G8B8 &&
              Format != gcvSURF_X8R8G8B8)))
        {
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }

        if (TileStatusConfig & gcv2D_TSC_V4_COMPRESSED)
        {
            if (Tiling == gcvTILED)
            {
                if (TileStatusConfig & gcv2D_TSC_V4_COMPRESSED_256B)
                {
                    alignedWidth = gcmALIGN(Width, 256);
                    alignedHeight = gcmALIGN(Height, 256);
                }
                else
                {
                    alignedWidth = gcmALIGN(Width, 128);
                    alignedHeight = gcmALIGN(Height, 128);
                }
            }
            else if ((Tiling & gcvSUPERTILED) || Tiling == gcvYMAJOR_SUPERTILED)
            {
                alignedWidth = gcmALIGN(Width, 128);
                alignedHeight = gcmALIGN(Height, 64);
            }
        }
        else if (TileStatusConfig & gcv2D_TSC_DEC_TPC)
        {
            alignedWidth = gcmALIGN(Width, 8);
            alignedHeight = gcmALIGN(Height, 8);
        }
        else
        {
            alignedWidth = gcmALIGN(Width, 64);

            if (gcoHAL_IsFeatureAvailable(Hal, gcvFEATURE_DEC400_COMPRESSION) &&
                TileStatusConfig == gcv2D_TSC_DEC_COMPRESSED &&
                Tiling == gcvTILED_64X4 &&
                (Format == gcvSURF_NV12 || Format == gcvSURF_P010))
            {
                alignedHeight = gcmALIGN(alignedHeight, 8);
            }
        }
        alignedBase = 256;
    }
    else if (TileStatusConfig & gcv2D_TSC_TPC_COMPRESSED)
    {
        if ((Tiling != gcvLINEAR) && (Tiling != gcvTILED))
        {
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }

        if (TileStatusConfig == gcv2D_TSC_TPC_COMPRESSED_V10)
        {
            alignedWidth = gcmALIGN(Width, 16);
            alignedHeight = gcmALIGN(Height, 16);
            alignedBase = 1024;
            bpp = 4;
        }
        else if (TileStatusConfig == gcv2D_TSC_TPC_COMPRESSED_V11)
        {
            alignedBase = 32 * 8 * bpp;

            alignedWidth = gcmALIGN(Width, 32);
            alignedHeight = gcmALIGN(Height, 8);

            if (Format == gcvSURF_NV12 ||
                Format == gcvSURF_NV21)
            {
                alignedBase = 256;
            }
            else if (Format == gcvSURF_P010)
            {
                alignedBase = 512;
            }
        }
    }
    else if (gcoHAL_IsFeatureAvailable(Hal, gcvFEATURE_TPCV11_COMPRESSION) ||
             gcoHAL_IsFeatureAvailable(Hal, gcvFEATURE_DEC400_COMPRESSION))
    {
        if (Format == gcvSURF_NV12)
        {
            if (!gcoHAL_IsFeatureAvailable(Hal, gcvFEATURE_DEC400_COMPRESSION))
            {
                alignedWidth = gcmALIGN(alignedWidth, 256);
            }
            alignedBase = 256;
        }
        else if (Format == gcvSURF_NV12_10BIT || Format == gcvSURF_NV21_10BIT)
        {
            alignedWidth = gcmALIGN(alignedWidth, 256);
            alignedBase = 320;
        }
        else if (Format == gcvSURF_P010)
        {
            alignedWidth = gcmALIGN(alignedWidth, 32);
            alignedHeight = gcmALIGN(alignedHeight, 8);
            alignedBase = 512;
        }
    }
    else if (TileStatusConfig != gcv2D_TSC_DISABLE)
    {
        alignedBase = 256;
    }

    alignedStride = alignedWidth * bpp;

    if (TileStatusConfig == gcv2D_TSC_TPC_COMPRESSED_V11)
    {
        switch (Format)
        {
            case gcvSURF_A8R8G8B8:
            case gcvSURF_X8R8G8B8:
            case gcvSURF_A2R10G10B10:
                alignedStride = gcmALIGN(alignedStride, 128);
                aligned = 1024;
                break;

            case gcvSURF_NV12:
                alignedStride = gcmALIGN(alignedStride, 32);
                aligned = 256;
                break;

            case gcvSURF_P010:
                alignedStride = gcmALIGN(alignedStride, 64);
                aligned = 512;
                break;
        }
    }
    else if (gcoHAL_IsFeatureAvailable(Hal, gcvFEATURE_TPCV11_COMPRESSION) ||
             gcoHAL_IsFeatureAvailable(Hal, gcvFEATURE_DEC400_COMPRESSION))
    {
        if (Format == gcvSURF_P010)
        {
            alignedStride = gcmALIGN(alignedStride, 64);
            aligned = 512;
        }
        else if (Format == gcvSURF_NV12 || Format == gcvSURF_NV21)
        {
            if (gcoHAL_IsFeatureAvailable(Hal, gcvFEATURE_TPCV11_COMPRESSION))
            {
                alignedStride = gcmALIGN(alignedStride, 256);
                aligned = 256;
            }
            else
            {
                alignedStride = gcmALIGN(alignedStride, 64);
                aligned = 64;
            }
        }
        else if (Format == gcvSURF_NV12_10BIT || Format == gcvSURF_NV21_10BIT)
        {
            alignedStride = gcmALIGN_NP2(alignedStride, 320);
            aligned = 320;
        }
    }

    if (Format == gcvSURF_NV12_10BIT ||
        Format == gcvSURF_NV21_10BIT ||
        Format == gcvSURF_NV16_10BIT ||
        Format == gcvSURF_NV61_10BIT)
    {
        alignedStride = gcmALIGN_NP2(alignedStride, 80);
        aligned = gcmALIGN_NP2(aligned, 80);
    }
    else if (gcoHAL_IsFeatureAvailable(Hal, gcvFEATURE_2D_YUV420_OUTPUT_LINEAR) &&
             (Format == gcvSURF_NV12 || Format == gcvSURF_NV21 || Format == gcvSURF_YV12 ||
              Format == gcvSURF_NV16 || Format == gcvSURF_NV61 || Format == gcvSURF_I420))
    {
        alignedStride = gcmALIGN(alignedStride, 64);
        aligned = gcmALIGN(aligned, 64);
    }

    gcmONERROR(gcoOS_Allocate(gcvNULL, sizeof(T2D_SURF), &surf));
    memset(surf, 0, sizeof(T2D_SURF));

    if (TileStatusConfig == gcv2D_TSC_2D_COMPRESSED ||
        (TileStatusConfig & gcv2D_TSC_V4_COMPRESSED) ||
        (TileStatusConfig & gcv2D_TSC_TPC_COMPRESSED) ||
        (gcoHAL_IsFeatureAvailable(Hal, gcvFEATURE_DEC_COMPRESSION) &&
         (TileStatusConfig & gcv2D_TSC_DEC_COMPRESSED)) ||
        Tiling != gcvLINEAR)
    {
        if (TileStatusConfig & gcv2D_TSC_DEC_TPC)
        {
            surf->vNode.size = gcmALIGN(alignedWidth * bpp, 16) * alignedHeight;
        }
        else
        {
            surf->vNode.size = alignedStride * alignedHeight;
        }
    }
    else
    {
        surf->vNode.size = alignedStride * alignedHeight + 128;
    }
    surf->vNode.pool = Pool;

    if ((TileStatusConfig & gcv2D_TSC_DOWN_SAMPLER) &&
        !(TileStatusConfig & gcv2D_TSC_V4_COMPRESSED))
    {
        alignedBase = 1024;
        surf->vNode.size *= 4;
    }

    if (TileStatusConfig & gcv2D_TSC_TPC_COMPRESSED)
    {
        if (TileStatusConfig == gcv2D_TSC_TPC_COMPRESSED_V10)
        {
            offset = gcmALIGN(alignedWidth * alignedHeight / 16, 1024);
        }
        else if (TileStatusConfig == gcv2D_TSC_TPC_COMPRESSED_V11)
        {
            offset = gcmALIGN(surf->vNode.size / 16, aligned);
        }
        surf->vNode.size += offset;
    }

    surf->vNode.size += aligned == 1 ? 0 : aligned;

    surf->vNode.allocatedSize = surf->vNode.size;
    gcmONERROR(AllocVideoNode(
        Hal,
        &surf->vNode.allocatedSize,
        &surf->vNode.pool,
        &alignedBase,
        &surf->vNode.node,
        &surf->vNode.address,
        &surf->vNode.memory));

    memset(surf->vNode.memory, 0, surf->vNode.allocatedSize);

    gcmONERROR(gcoOS_CacheFlush(
        gcvNULL,
        surf->vNode.node,
        surf->vNode.memory,
        surf->vNode.size));

#if gcvVERSION_MAJOR >= 5
    gcfAddMemoryInfo(
        surf->vNode.address,
        surf->vNode.memory,
        gcvINVALID_ADDRESS,
        surf->vNode.size);
#endif

    surf->format = Format;
    surf->tiling = Tiling;
    surf->width = Width;
    surf->height = Height;
    surf->aWidth = alignedWidth;
    surf->aHeight = alignedHeight;
    surf->stride[0] = alignedStride;
    surf->validStrideNum = 1;
    surf->address[0] = surf->vNode.address + offset;
    surf->address[0] = (aligned & (aligned-1)) ? gcmALIGN_NP2(surf->address[0], aligned) : gcmALIGN(surf->address[0], aligned);
    surf->logical[0] = GAL_POINTER_OFFSET(surf->vNode.memory, surf->address[0] - surf->vNode.address);
    surf->rotation = gcvSURF_0_DEGREE;
    surf->tileStatusConfig = gcoHAL_IsFeatureAvailable(Hal, gcvFEATURE_DEC_COMPRESSION) ?
                             TileStatusConfig : TileStatusConfig & ~gcv2D_TSC_DEC_COMPRESSED;
    surf->tileStatusFormat = Format;
    surf->superTileVersion = Tiling & gcvSUPERTILED ? gcv2D_SUPER_TILE_VERSION_V3 : -1;

    if (TileStatusConfig & gcv2D_TSC_TPC_COMPRESSED)
    {
        surf->tileStatusNode.size = offset;
        surf->tileStatusNode.pool = surf->vNode.pool;
        surf->tileStatusNode.address = surf->vNode.address;
        surf->tileStatusNode.memory = surf->vNode.memory;
        surf->tileStatusAddress = surf->tileStatusNode.address;
        surf->tileStatuslogical = surf->tileStatusNode.memory;

#if gcvVERSION_MAJOR >= 5
        gcfAddMemoryInfo(
            surf->tileStatusNode.address,
            surf->tileStatusNode.memory,
            gcvINVALID_ADDRESS,
            surf->tileStatusNode.size);
#endif
    }
    else if (TileStatusConfig != gcv2D_TSC_DISABLE && TileStatusConfig != gcv2D_TSC_DEC_TPC_TILED)
    {
        if (TileStatusConfig & gcv2D_TSC_DEC_TPC_COMPRESSED)
        {
            surf->tileStatusNode.size = gcmMIN(surf->vNode.size, 150 * 1024);
        }
        else if (gcoHAL_IsFeatureAvailable(Hal, gcvFEATURE_DEC400_COMPRESSION) &&
                 (TileStatusConfig & gcv2D_TSC_DEC_COMPRESSED))
        {
            gctUINT32 total = 0, tsbit = 0;

            if (!GalIsYUVFormat(Format) ||
                Format == gcvSURF_YUY2  ||
                Format == gcvSURF_UYVY)
            {
                static gctUINT tileBitsArray[3][5] =
                {
               /* 16x4 8x8 8x4 4x8 4x4 */
                    {8, 8, 8, 8, 8}, /* 16 aligned */
                    {4, 4, 4, 4, 4}, /* 32 aligned */
                    {4, 4, 4, 4, 4}, /* 64 aligned */
                };

                switch (surf->tiling)
                {
                    case gcvTILED_8X8_XMAJOR:
                    case gcvSUPERTILED_256B:
                        total = 64;
                        tsbit = tileBitsArray[gcd2D_COMPRESSION_DEC400_ALIGN_MODE][1];
                        break;

                    case gcvSUPERTILED_128B:
                        if (bpp == 4)
                        {
                            total = 32;
                            tsbit = tileBitsArray[gcd2D_COMPRESSION_DEC400_ALIGN_MODE][2];
                        }
                        else
                        {
                            total = 64;
                            tsbit = tileBitsArray[gcd2D_COMPRESSION_DEC400_ALIGN_MODE][1];
                        }
                        break;

                    case gcvTILED_8X4:
                        total = 32;
                        tsbit = tileBitsArray[gcd2D_COMPRESSION_DEC400_ALIGN_MODE][2];
                        break;

                    case gcvTILED_4X8:
                        total = 32;
                        tsbit = tileBitsArray[gcd2D_COMPRESSION_DEC400_ALIGN_MODE][3];
                        break;

                    default:
                        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
                        break;
                }

                if (total && tsbit)
                {
                    surf->tileStatusNode.size = alignedWidth * alignedHeight / total * tsbit / 8;
                }
            }
            else
            {
                static gctUINT tileBitsArray[3][3] =
                {
                /* 256 128 64 */
                    {4, 4, 2}, /* 16 aligned */
                    {4, 2, 1}, /* 32 aligned */
                    {2, 1, 1}, /* 64 aligned */
                };

                if (Format == gcvSURF_P010 && surf->tiling == gcvTILED_32X4)
                {
                    total = 32 * 4;
                    /* Y tile status size = 32 * 4 * 2 = 256 */
                    tsbit = tileBitsArray[gcd2D_COMPRESSION_DEC400_ALIGN_MODE][0];
                }
                else if (Format == gcvSURF_NV12 && surf->tiling == gcvTILED_64X4)
                {
                    total = 64 * 4;
                    /* Y tile status size = 64 * 4 * 1 = 256 */
                    tsbit = tileBitsArray[gcd2D_COMPRESSION_DEC400_ALIGN_MODE][0];
                }
                else if (surf->tiling == gcvTILED_8X8_XMAJOR)
                {
                    total = 8 * 8;
                    if (Format == gcvSURF_NV12)
                    {
                        /* Y tile status size = 8 * 8 * 1 = 64 */
                        tsbit = tileBitsArray[gcd2D_COMPRESSION_DEC400_ALIGN_MODE][2];
                    }
                    else if (Format == gcvSURF_NV12_10BIT)
                    {
                        /* Y tile status size = 8 * 8 * 2 = 128 */
                        tsbit = tileBitsArray[gcd2D_COMPRESSION_DEC400_ALIGN_MODE][1];
                    }
                }

                if (total && tsbit)
                {
                    surf->tileStatusNode.size = alignedWidth * alignedHeight / total * tsbit / 8 + 1024;
                }
            }
        }
        else
        {
            surf->tileStatusNode.size = surf->vNode.size / 64;
        }
        surf->tileStatusNode.pool = Pool;

        surf->tileStatusNode.allocatedSize = surf->tileStatusNode.size;
        gcmONERROR(AllocVideoNode(
            Hal,
            &surf->tileStatusNode.allocatedSize,
            &surf->tileStatusNode.pool,
            &alignedTSBase,
            &surf->tileStatusNode.node,
            &surf->tileStatusNode.address,
            &surf->tileStatusNode.memory));

        surf->tileStatusAddress = surf->tileStatusNode.address;
        surf->tileStatuslogical = surf->tileStatusNode.memory;

#if gcvVERSION_MAJOR >= 5
        gcfAddMemoryInfo(
            surf->tileStatusNode.address,
            surf->tileStatusNode.memory,
            gcvINVALID_ADDRESS,
            surf->tileStatusNode.size);
#endif

        if ((TileStatusConfig & gcv2D_TSC_DOWN_SAMPLER) &&
            !(TileStatusConfig & gcv2D_TSC_V4_COMPRESSED))
        {
            surf->stride[0] *= 2;
        }

        if (TileStatusConfig == gcv2D_TSC_2D_COMPRESSED)
        {
            if (Format == gcvSURF_A8R8G8B8)
            {
                memset(surf->tileStatusNode.memory, 0x88, surf->tileStatusNode.allocatedSize);
            }
            else
            {
                memset(surf->tileStatusNode.memory, 0x66, surf->tileStatusNode.allocatedSize);
            }
        }
        else if (TileStatusConfig & gcv2D_TSC_V4_COMPRESSED)
        {
            memset(surf->tileStatusNode.memory, 0x11, surf->tileStatusNode.allocatedSize);
        }
        else
        {
            memset(surf->tileStatusNode.memory, 0, surf->tileStatusNode.allocatedSize);
        }

        gcmONERROR(gcoOS_CacheFlush(
            gcvNULL,
            surf->tileStatusNode.node,
            surf->tileStatusNode.memory,
            surf->tileStatusNode.size));
    }

    if (GalIsYUVFormat(Format))
    {
        switch (Format)
        {
        case gcvSURF_YUY2:
        case gcvSURF_UYVY:
        case gcvSURF_YVYU:
        case gcvSURF_VYUY:
            surf->validAddressNum = surf->validStrideNum = 1;
            break;

        case gcvSURF_I420:
        case gcvSURF_YV12:
            {
                gctUINT32 size, delta;

                surf->validAddressNum = surf->validStrideNum = 3;

                surf->stride[0] = gcmALIGN(alignedWidth, aligned);
                size = surf->stride[0] * alignedHeight;

                surf->address[1] = gcmALIGN(surf->address[0] + size, aligned);
                delta = surf->address[1] - surf->address[0] - size;
                surf->logical[1] = GAL_POINTER_OFFSET(surf->logical[0], size + delta);
                if(gcoHAL_IsFeatureAvailable(Hal, gcvFEATURE_2D_YUV420_OUTPUT_LINEAR))
                {
                    /*I420 and YV12 UV stride must be 32 bytes alignment*/
                    surf->stride[1] = gcmALIGN(alignedWidth >> 1, 32);
                    surf->stride[2] = gcmALIGN(alignedWidth >> 1, 32);
                } else
                {
                    surf->stride[1] = alignedWidth >> 1;
                    surf->stride[2] = alignedWidth >> 1;
                }
                size >>= 2;
                surf->address[2] = gcmALIGN(surf->address[1] + size, aligned);
                delta = surf->address[2] - surf->address[1] - size;
                surf->logical[2] = GAL_POINTER_OFFSET(surf->logical[1], size + delta);
            }
            break;

        case gcvSURF_NV16:
        case gcvSURF_NV61:
        case gcvSURF_NV12:
        case gcvSURF_NV21:
            {
                gctUINT32 size, delta;

                surf->validAddressNum = surf->validStrideNum = 2;

                if (TileStatusConfig & gcv2D_TSC_DEC_TPC)
                {
                    surf->stride[0] = gcmALIGN(alignedWidth, 16);
                    surf->stride[1] = gcmALIGN(alignedWidth, 16);
                }
                else
                {
                    surf->stride[0] = gcmALIGN(alignedWidth, aligned);
                    surf->stride[1] = gcmALIGN(alignedWidth, aligned);
                }

                size = surf->stride[0] * alignedHeight;
                surf->address[1] = gcmALIGN(surf->address[0] + size, aligned);
                delta = surf->address[1] - surf->address[0] - size;
                surf->logical[1] = GAL_POINTER_OFFSET(surf->logical[0], size + delta);
            }
            break;

        case gcvSURF_NV12_10BIT:
        case gcvSURF_NV21_10BIT:
            {
                gctUINT32 size, delta;

                surf->validAddressNum = surf->validStrideNum = 2;

                if (gcoHAL_IsFeatureAvailable(Hal, gcvFEATURE_DEC400_COMPRESSION))
                {
                    aligned = 80;
                }

                surf->stride[0] = surf->stride[1] = gcmALIGN_NP2((gctUINT32)((float)alignedWidth * 1.25), aligned);

                size = surf->stride[0] * alignedHeight;
                surf->address[1] = gcmALIGN_NP2(surf->address[0] + size, aligned);
                delta = surf->address[1] - surf->address[0] - size;
                surf->logical[1] = GAL_POINTER_OFFSET(surf->logical[0], size + delta);
            }
            break;

        case gcvSURF_NV16_10BIT:
        case gcvSURF_NV61_10BIT:
            {
                gctUINT32 size, delta;

                surf->validAddressNum = surf->validStrideNum = 2;
                surf->stride[0] = surf->stride[1] = gcmALIGN_NP2((gctUINT32)((float)alignedWidth * 1.25), aligned);

                size = surf->stride[0] * alignedHeight;
                surf->address[1] = gcmALIGN_NP2(surf->address[0] + size, aligned);
                delta = surf->address[1] - surf->address[0] - size;
                surf->logical[1] = GAL_POINTER_OFFSET(surf->logical[0], size + delta);
            }
            break;

        case gcvSURF_P010:
            {
                gctUINT32 size, delta;

                surf->validAddressNum = surf->validStrideNum = 2;
                surf->stride[0] = surf->stride[1] = gcmALIGN(alignedWidth * 2, aligned);

                size = surf->stride[0] * alignedHeight;
                surf->address[1] = gcmALIGN(surf->address[0] + size, aligned);
                delta = surf->address[1] - surf->address[0] - size;
                surf->logical[1] = GAL_POINTER_OFFSET(surf->logical[0], size + delta);
            }
            break;

        default:
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }

        if ((TileStatusConfig & gcv2D_TSC_DEC_TPC_COMPRESSED) ||
            (gcoHAL_IsFeatureAvailable(Hal, gcvFEATURE_DEC400_COMPRESSION) &&
             (TileStatusConfig & gcv2D_TSC_DEC_COMPRESSED)))
        {
            if (surf->validAddressNum > 1)
            {
                if (TileStatusConfig & gcv2D_TSC_DEC_TPC_COMPRESSED)
                {
                    surf->tileStatusNodeEx[0].size = gcmMIN(surf->stride[1] * alignedHeight, 150 * 1024);
                }
                else if (gcoHAL_IsFeatureAvailable(Hal, gcvFEATURE_DEC400_COMPRESSION))
                {
                    gctUINT total = 0, tsbit = 0;

                    static gctUINT tileBitsArray[3][3] =
                    {
                    /* 256 128 64 */
                        {4, 4, 2}, /* 16 aligned */
                        {4, 2, 1}, /* 32 aligned */
                        {2, 1, 1}, /* 64 aligned */
                    };

                    if (Format == gcvSURF_P010 && surf->tiling == gcvTILED_32X4)
                    {
                        total = 16 * 4;
                        /* UV tile status size = 16 * 4 * 4 = 256 */
                        tsbit = tileBitsArray[gcd2D_COMPRESSION_DEC400_ALIGN_MODE][0];
                    }
                    else if (Format == gcvSURF_NV12 && surf->tiling == gcvTILED_64X4)
                    {
                        total = 32 * 4;
                        /* UV tile status size = 32 * 4 * 2 = 256 */
                        tsbit = tileBitsArray[gcd2D_COMPRESSION_DEC400_ALIGN_MODE][0];
                    }
                    else if (surf->tiling == gcvTILED_8X8_XMAJOR)
                    {
                        total = 8 * 4;
                        if (Format == gcvSURF_NV12)
                        {
                            /* UV tile status size = 8 * 4 * 2 = 64 */
                            tsbit = tileBitsArray[gcd2D_COMPRESSION_DEC400_ALIGN_MODE][2];
                        }
                        else if (Format == gcvSURF_NV12_10BIT)
                        {
                            /* Y tile status size = 8 * 4 * 4 = 128 */
                            tsbit = tileBitsArray[gcd2D_COMPRESSION_DEC400_ALIGN_MODE][1];
                        }
                    }

                    if (total && tsbit)
                    {
                        surf->tileStatusNodeEx[0].size = alignedWidth * alignedHeight / 4 / total * tsbit / 8;
                    }
                }

                surf->tileStatusNodeEx[0].pool = Pool;

                surf->tileStatusNodeEx[0].allocatedSize = surf->tileStatusNodeEx[0].size;
                gcmONERROR(AllocVideoNode(
                    Hal,
                    &surf->tileStatusNodeEx[0].allocatedSize,
                    &surf->tileStatusNodeEx[0].pool,
                    &alignedTSBase,
                    &surf->tileStatusNodeEx[0].node,
                    &surf->tileStatusNodeEx[0].address,
                    &surf->tileStatusNodeEx[0].memory));

                surf->tileStatusAddressEx[0] = surf->tileStatusNodeEx[0].address;
                surf->tileStatuslogicalEx[0] = surf->tileStatusNodeEx[0].memory;

#if gcvVERSION_MAJOR >= 5
                gcfAddMemoryInfo(
                    surf->tileStatusNodeEx[0].address,
                    surf->tileStatusNodeEx[0].memory,
                    gcvINVALID_ADDRESS,
                    surf->tileStatusNodeEx[0].size);
#endif

                if (gcoHAL_IsFeatureAvailable(Hal, gcvFEATURE_DEC400_COMPRESSION))
                {
                    memset(surf->tileStatusNodeEx[0].memory, 0, surf->tileStatusNodeEx[0].allocatedSize);
                }

                gcmONERROR(gcoOS_CacheFlush(
                    gcvNULL,
                    surf->tileStatusNodeEx[0].node,
                    surf->tileStatusNodeEx[0].memory,
                    surf->tileStatusNodeEx[0].size));

                if (surf->validAddressNum > 2)
                {
                    surf->tileStatusNodeEx[1].size = surf->stride[2] * alignedHeight / 32;
                    surf->tileStatusNodeEx[1].pool = Pool;

                    surf->tileStatusNodeEx[1].allocatedSize = surf->tileStatusNodeEx[1].size;
                    gcmONERROR(AllocVideoNode(
                        Hal,
                        &surf->tileStatusNodeEx[1].allocatedSize,
                        &surf->tileStatusNodeEx[1].pool,
                        &alignedTSBase,
                        &surf->tileStatusNodeEx[1].node,
                        &surf->tileStatusNodeEx[1].address,
                        &surf->tileStatusNodeEx[1].memory));

                    surf->tileStatusAddressEx[1] = surf->tileStatusNodeEx[0].address;
                    surf->tileStatuslogicalEx[1] = surf->tileStatusNodeEx[0].memory;

    #if gcvVERSION_MAJOR >= 5
                    gcfAddMemoryInfo(
                        surf->tileStatusNodeEx[1].address,
                        surf->tileStatusNodeEx[1].memory,
                        gcvINVALID_ADDRESS,
                        surf->tileStatusNodeEx[1].size);
    #endif

                    gcmONERROR(gcoOS_CacheFlush(
                        gcvNULL,
                        surf->tileStatusNodeEx[1].node,
                        surf->tileStatusNodeEx[1].memory,
                        surf->tileStatusNodeEx[1].size));
                }
            }
        }
    }
    else
    {
        if (Tiling == gcvMULTI_TILED || Tiling == gcvMULTI_SUPERTILED)
        {
            surf->address[1] = surf->address[0] + surf->vNode.size / 2 + offset;
            surf->logical[1] = GAL_POINTER_OFFSET(surf->logical[0], surf->vNode.size / 2 + offset);
            surf->validAddressNum = 2;
        }
        else
        {
            surf->validAddressNum = 1;
        }
    }

    *Surface = surf;

OnError:
    if (status != gcvSTATUS_OK)
    {
        /* roll back. */
        if (surf != gcvNULL)
        {
            if (surf->vNode.node != 0)
            {
#if gcvVERSION_MAJOR >= 5
                gcfDelMemoryInfo(surf->vNode.address);
#endif
                FreeVideoNode(Hal, surf->vNode.node);
            }

            gcoOS_Free(gcvNULL, surf);
        }
    }

    return status;
}

gceSTATUS GalDeleteTSurf(
    IN gcoHAL Hal,
    IN T2D_SURF_PTR Surface
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    if (Surface != gcvNULL)
    {
        if (Surface->vNode.node)
        {
#if gcvVERSION_MAJOR >= 5
            gcfDelMemoryInfo(Surface->vNode.address);
#endif
            gcmONERROR(FreeVideoNode(Hal, Surface->vNode.node));
        }

        if (Surface->tileStatusNode.node)
        {
#if gcvVERSION_MAJOR >= 5
            gcfDelMemoryInfo(Surface->tileStatusNode.address);
#endif
            if (!(Surface->tileStatusConfig & gcv2D_TSC_TPC_COMPRESSED))
            {
                gcmONERROR(FreeVideoNode(Hal, Surface->tileStatusNode.node));
            }
        }

        if (Surface->tileStatusNodeEx[0].node)
        {
#if gcvVERSION_MAJOR >= 5
            gcfDelMemoryInfo(Surface->tileStatusNodeEx[0].address);
#endif
            gcmONERROR(FreeVideoNode(Hal, Surface->tileStatusNodeEx[0].node));
        }

        if (Surface->tileStatusNodeEx[1].node)
        {
#if gcvVERSION_MAJOR >= 5
            gcfDelMemoryInfo(Surface->tileStatusNodeEx[1].address);
#endif
            gcmONERROR(FreeVideoNode(Hal, Surface->tileStatusNodeEx[1].node));
        }

        gcmONERROR(gcoOS_Free(gcvNULL, Surface));
    }

OnError:

    return status;
}

static gctUINT32 GETBITS(gctUINT32 Data, gctUINT32 Start, gctUINT32 End)
{
    gctUINT32 start = gcmMIN(Start, End);
    gctUINT32 end = gcmMAX(Start, End);

    return (Data >> start) & (~0U >> (31 - end + start));
}

gceSTATUS GalLoadVimgToTSurf(
    IN gctCONST_STRING Filename,
    OUT T2D_SURF_PTR *TSurf
    )
{
    return GalLoadVimgToTSurfWithPool(Filename, gcvPOOL_DEFAULT, TSurf);
}

gceSTATUS GalLoadVimgToTSurfWithPool(
    IN gctCONST_STRING Filename,
    IN gcePOOL Pool,
    OUT T2D_SURF_PTR *TSurf
    )
{
    gctFILE file = gcvNULL;
    gceSTATUS status;
    T2D_SURF_PTR surf = gcvNULL;

    do {
        VIMG_FILEHEADER head;
        VIMG_V1 *v1 = gcvNULL;
        VIMG_V2 *v2 = gcvNULL;
        VIMG_V2 vhead;

        gctUINT32 width[3] = {0, 0, 0};
        gctUINT32 height[3] = {0, 0, 0};
        gctUINT32 lineSize;
        gctUINT32 bpp[3] = {0, 0, 0};
        gctUINT nPlane, i;
        gctBOOL swap = gcvFALSE;
        gceSURF_FORMAT format;

        gceTILING tiling;
        gce2D_TILE_STATUS_CONFIG tsc = gcv2D_TSC_DISABLE;
        gctUINT n;
        gctUINT32 vsupertile = ~0u;

        gcmONERROR(gcoOS_Open(NULL, Filename, gcvFILE_READ, &file));

        gcmONERROR(gcoOS_Read(
            NULL,
            file,
            sizeof(head),
            &head,
            gcvNULL
            ));

        if (head.magic[0] != 'V' || head.magic[1] != 'I'
                || head.magic[2] != 'V')
        {
            gcmONERROR(gcvSTATUS_INVALID_DATA);
        }

    if (head.version == 1 || head.version == 2)
    {
        vhead.v1.format = read_dword(file);
        vhead.v1.tiling = read_dword(file);
        vhead.v1.imageStride = read_dword(file);
        vhead.v1.imageWidth = read_dword(file);
        vhead.v1.imageHeight = read_dword(file);
        vhead.v1.bitsOffset = read_dword(file);
            v1 = &vhead.v1;

            if (head.version == 2)
            {
                vhead.tileStatusConfig = read_dword(file);
                vhead.tileStatusSize = read_dword(file);
                vhead.compressedFormat = read_dword(file);
                vhead.clearValue = read_dword(file);
                vhead.tileStatusOffset = read_dword(file);
                vhead.tileStatusOffsetEx[0] = read_dword(file);
                vhead.tileStatusOffsetEx[1] = read_dword(file);
                vhead.tileStatusSizeEx[0] = read_dword(file);
                vhead.tileStatusSizeEx[1] = read_dword(file);
                v2 = &vhead;
            }
        }
        else
        {
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }
        format = v1->format;
        switch (format)
        {
        case gcvSURF_I420:
            nPlane = 3;
            width[0] = v1->imageWidth;
            height[0] = v1->imageHeight;
            bpp[0] = 1;
            width[1] = width[2] = v1->imageWidth / 2;
            height[1] = height[2] = v1->imageHeight / 2;
            bpp[1] = bpp[2] = 1;
            break;

        case gcvSURF_YV12:
            nPlane = 3;
            width[0] = v1->imageWidth;
            height[0] = v1->imageHeight;
            bpp[0] = 1;
            width[1] = width[2] = v1->imageWidth / 2;
            height[1] = height[2] = v1->imageHeight / 2;
            bpp[1] = bpp[2] = 1;
            swap = gcvTRUE;
            break;

        case gcvSURF_NV16:
            nPlane = 2;
            width[0] = v1->imageWidth;
            height[0] = v1->imageHeight;
            bpp[0] = 1;
            width[1] = v1->imageWidth / 2;
            height[1] = v1->imageHeight;
            bpp[1] = 2;
            break;

        case gcvSURF_NV12:
            format = gcvSURF_NV12;
            nPlane = 2;
            width[0] = v1->imageWidth;
            height[0] = v1->imageHeight;
            bpp[0] = 1;
            width[1] = v1->imageWidth / 2;
            height[1] = v1->imageHeight / 2;
            bpp[1] = 2;
            break;

        case gcvSURF_NV21:
            nPlane = 2;
            width[0] = v1->imageWidth;
            height[0] = v1->imageHeight;
            bpp[0] = 1;
            width[1] = v1->imageWidth / 2;
            height[1] = v1->imageHeight / 2;
            bpp[1] = 2;
            break;

        case gcvSURF_NV61:
            nPlane = 2;
            width[0] = v1->imageWidth;
            height[0] = v1->imageHeight;
            bpp[0] = 1;
            width[1] = v1->imageWidth / 2;
            height[1] = v1->imageHeight;
            bpp[1] = 2;
            break;

        default:
            /*do nothing*/
            break;
        }

        gcmONERROR(gcoOS_Seek(
            NULL,
            file,
            0,
            gcvFILE_SEEK_END));

        gcmONERROR(gcoOS_GetPos(
            NULL,
            file,
            &n));

        if (n - v1->bitsOffset < v1->imageHeight * v1->imageStride)
        {
            gcmONERROR(gcvSTATUS_OUT_OF_RESOURCES);
        }

        if (v2 != gcvNULL)
        {
            if ((v2->tileStatusOffset - v1->bitsOffset < v1->imageHeight * v1->imageStride)
                || (n - v2->tileStatusOffset < v2->tileStatusSize))
            {
                gcmONERROR(gcvSTATUS_OUT_OF_RESOURCES);
            }

            tsc = v2->tileStatusConfig;
        }

        gcmONERROR(ConvertToGalTiling(v1->tiling, (gctUINT32_PTR)&tiling, &vsupertile));

        gcmONERROR(GalCreateTSurfWithPool(gcvNULL, v1->format, tiling, tsc, v1->imageWidth, v1->imageHeight, Pool, &surf));

        gcmONERROR(gcoOS_Seek(
                gcvNULL,
                file,
                v1->bitsOffset,
                gcvFILE_SEEK_SET
                ));
        /*write YUV planar format data*/
        if(format >= gcvSURF_YV12 && format <= gcvSURF_NV61)
        {

            lineSize = width[0] * bpp[0];
            for (n = 0; n < height[0]; n++)
            {
                /* Fill plane 1. */
                gctUINT8_PTR p = (gctUINT8_PTR)surf->logical[0] + n * surf->stride[0];

                gcmONERROR(gcoOS_Read(
                        gcvNULL,
                        file,
                        lineSize,
                        p,
                        gcvNULL
                        ));

                if (GalIsBigEndian() && (bpp[0] == 2))
                {
                    gctUINT16 *s = (gctUINT16 *)p;

                    for (i = 0; i < lineSize; i += 2)
                    {
                        *s = swap_word(*s);
                        s++;
                    }
                }
            }


            if (nPlane > 1)
            {
                gctUINT aStride1 = surf->stride[1];
                gctUINT lineSize1 = width[1] * bpp[1];

                if (surf->logical[1] == gcvNULL)
                {
                    gcmONERROR(gcvSTATUS_INVALID_DATA);
                }

                for (n = 0; n < height[1]; n++)
                {
                    /* Fill plane 2. */
                    gctUINT8_PTR p = (gctUINT8_PTR)surf->logical[swap ? 2 : 1] + n * aStride1;

                    gcmONERROR(gcoOS_Read(
                            gcvNULL,
                            file,
                            lineSize1,
                            p,
                            gcvNULL
                            ));

                    if (GalIsBigEndian() && (bpp[1] == 2))
                    {
                        gctUINT16 *s = (gctUINT16 *)p;

                        for (i = 0; i < lineSize1; i += 2)
                        {
                            *s = swap_word(*s);
                            s++;
                        }
                    }
                }

                if (nPlane > 2)
                {
                    gctUINT aStride2;
                    gctUINT lineSize2;

                    if (nPlane != 3)
                    {
                        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
                    }

                    if (surf->logical[2] == gcvNULL)
                    {
                        gcmONERROR(gcvSTATUS_INVALID_DATA);
                    }

                    aStride2 = surf->stride[2];
                    lineSize2 = width[2] * bpp[2];
                    for (n = 0; n < height[2]; n++)
                    {
                        /* Fill plane 3. */
                        gctUINT8_PTR p = (gctUINT8_PTR)surf->logical[swap ? 1 : 2] + n * aStride2;

                        gcmONERROR(gcoOS_Read(
                                gcvNULL,
                                file,
                                lineSize2,
                                p,
                                gcvNULL
                                ));

                        if (GalIsBigEndian() && (bpp[2] == 2))
                        {
                            gctUINT16 *s = (gctUINT16 *)p;

                            for (i = 0; i < lineSize2; i += 2)
                            {
                                *s = swap_word(*s);
                                s++;
                            }
                        }
                    }
                }
            }
    }
    else
    /* non-YUV_planar format */
    {
        if (!GalIsBigEndian() || v2 != gcvNULL)
        {
            gcmONERROR(gcoOS_Read(
                gcvNULL,
                file,
                surf->vNode.size,
                surf->logical[0],
                gcvNULL
                ));
        }
        else
        {
            switch (v1->imageStride / v1->imageWidth)
            {
            case 1:
                gcmONERROR(gcoOS_Read(
                    gcvNULL,
                    file,
                    surf->vNode.size,
                    surf->vNode.memory,
                    gcvNULL
                       ));
                break;

            case 2:
                {
                unsigned short* p = (unsigned short*) surf->vNode.memory;
                int n = surf->vNode.size / 2;
                while (n > 0)
                {
                    *p = read_word(file);
                    p++;
                    n--;
                }
                }
                break;

            case 3:
                {
                unsigned char* p = (unsigned char*) surf->vNode.memory;
                int n = surf->vNode.size / 3;
                while (n > 0)
                {
                    *(p + 2) = read_byte(file);
                    *(p + 1) = read_byte(file);
                    *(p)     = read_byte(file);
                    p += 3;
                    n--;
                }
                }
                break;

            case 4:
                {
                unsigned int* p = (unsigned int*) surf->vNode.memory;
                int n = surf->vNode.size / 4;
                while (n > 0)
                {
                    *p = read_dword(file);
                    p++;
                    n--;
                }
                }
                break;
            }
        }

        }

        if (v2 != gcvNULL)
        {
            gcmONERROR(gcoOS_Seek(
                    gcvNULL,
                    file,
                    v2->tileStatusOffset,
                    gcvFILE_SEEK_SET
                    ));

            gcmONERROR(gcoOS_Read(
                    gcvNULL,
                    file,
                    v2->tileStatusSize,
                    surf->tileStatuslogical,
                    gcvNULL
                    ));

            surf->tileStatusClear = v2->clearValue;
            surf->tileStatusConfig = v2->tileStatusConfig;
            surf->tileStatusConfig = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_DEC_COMPRESSION) ?
                         v2->tileStatusConfig : v2->tileStatusConfig & ~gcv2D_TSC_DEC_COMPRESSED;

            surf->tileStatusFormat = v2->compressedFormat;
            if (surf->tileStatusFormat == gcvSURF_R8G8B8)
                surf->tileStatusFormat = gcvSURF_X8R8G8B8;

            if (v2->tileStatusOffsetEx[0])
            {
                gcmONERROR(gcoOS_Seek(
                    gcvNULL,
                    file,
                    v2->tileStatusOffsetEx[0],
                    gcvFILE_SEEK_SET
                    ));

                gcmONERROR(gcoOS_Read(
                    gcvNULL,
                    file,
                    v2->tileStatusSizeEx[0],
                    surf->tileStatuslogicalEx[0],
                    gcvNULL
                    ));
            }
        }

        surf->superTileVersion = vsupertile;

        gcmONERROR(gcoOS_Close(gcvNULL, file));

        gcmONERROR(gcoOS_CacheFlush(gcvNULL, surf->vNode.node, surf->vNode.memory, surf->vNode.size));

        *TSurf = surf;

        return gcvSTATUS_OK;

    } while (gcvFALSE);

OnError:

    if (surf)
    {
        gcmVERIFY_OK(GalDeleteTSurf(gcvNULL, surf));
    }

    if (file)
    {
        gcmVERIFY_OK(gcoOS_Close(gcvNULL, file));
    }

    return status;
}


gceSTATUS CDECL GalLoadVimgToSurface(
    IN gctCONST_STRING filename,
    OUT gcoSURF *surface
    )
{
    gctFILE file = gcvNULL;
    gceSTATUS status;
    gctPOINTER memory[3] = {0, 0, 0};
    gcoSURF surf = gcvNULL;

    do {
        VIMG_FILEHEADER fileHead;
        VIMG_V1 head;
        gctUINT alignedWidth, alignedHeight, widthT;
        gctINT alignedStride;
        gctUINT32 lineSize, aStride;
        gctUINT32 address[3] = {0, 0, 0};
        gctUINT32 width[3] = {0, 0, 0};
        gctUINT32 height[3] = {0, 0, 0};
        gctFLOAT bpp[3] = {0.0, 0.0, 0.0};
        gctUINT nPlane, n, i;
        gcoSURF surf;
        gceSURF_FORMAT format;
        gctBOOL swap = gcvFALSE;

        gcmONERROR(gcoOS_Open(NULL, filename, gcvFILE_READ, &file));

        gcmONERROR(gcoOS_Read(
                NULL,
                file,
                sizeof(VIMG_FILEHEADER),
                &fileHead,
                gcvNULL
                ));

        if (fileHead.magic[0] != 'V' || fileHead.magic[1] != 'I'
                || fileHead.magic[2] != 'V')
        {
            gcmONERROR(gcvSTATUS_INVALID_DATA);
        }

        if (fileHead.version == 1)
        {
            head.format      = read_dword(file);
            head.tiling      = read_dword(file);
            head.imageStride = read_dword(file);
            head.imageWidth  = read_dword(file);
            head.imageHeight = read_dword(file);
            head.bitsOffset  = read_dword(file);

            /* translate the tiling. */
            gcmONERROR(ConvertToGalTiling(head.tiling, &head.tiling, gcvNULL));
        }
        else
        {
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }

        gcmONERROR(gcoOS_Seek(
            NULL,
            file,
            0,
            gcvFILE_SEEK_END));

        gcmONERROR(gcoOS_GetPos(
            NULL,
            file,
            &n));

        if (n - head.bitsOffset < head.imageHeight * head.imageStride)
        {
            gcmONERROR(gcvSTATUS_OUT_OF_RESOURCES);
        }

        if ((head.imageWidth == 0) || (head.imageHeight == 0) || (head.tiling != gcvLINEAR))
        {
            gcmONERROR(gcvSTATUS_INVALID_DATA);
        }

        widthT = head.imageWidth;

        /* Check the type. */
        switch (head.format)
        {
        case gcvSURF_A8:
            format = gcvSURF_A8;
            nPlane = 1;
            width[0] = head.imageWidth;
            height[0] = head.imageHeight;
            bpp[0] = 1;
            break;

        case gcvSURF_YUY2:
            format = gcvSURF_YUY2;
            nPlane = 1;
            width[0] = head.imageWidth;
            height[0] = head.imageHeight;
            bpp[0] = 2;
            break;

        case gcvSURF_YVYU:
            format = gcvSURF_YVYU;
            nPlane = 1;
            width[0] = head.imageWidth;
            height[0] = head.imageHeight;
            bpp[0] = 2;
            break;

        case gcvSURF_UYVY:
            format = gcvSURF_UYVY;
            nPlane = 1;
            width[0] = head.imageWidth;
            height[0] = head.imageHeight;
            bpp[0] = 2;
            break;

        case gcvSURF_VYUY:
            format = gcvSURF_VYUY;
            nPlane = 1;
            width[0] = head.imageWidth;
            height[0] = head.imageHeight;
            bpp[0] = 2;
            break;

        case gcvSURF_I420:
            format = gcvSURF_I420;
            nPlane = 3;
            width[0] = head.imageWidth;
            height[0] = head.imageHeight;
            bpp[0] = 1;
            width[1] = width[2] = head.imageWidth / 2;
            height[1] = height[2] = head.imageHeight / 2;
            bpp[1] = bpp[2] = 1;
            break;

        case gcvSURF_YV12:
            format = gcvSURF_YV12;
            nPlane = 3;
            width[0] = head.imageWidth;
            height[0] = head.imageHeight;
            bpp[0] = 1;
            width[1] = width[2] = head.imageWidth / 2;
            height[1] = height[2] = head.imageHeight / 2;
            bpp[1] = bpp[2] = 1;
            swap = gcvTRUE;
            break;

        case gcvSURF_NV16:
            format = gcvSURF_NV16;
            nPlane = 2;
            width[0] = head.imageWidth;
            height[0] = head.imageHeight;
            bpp[0] = 1;
            width[1] = head.imageWidth / 2;
            height[1] = head.imageHeight;
            bpp[1] = 2;
            break;

        case gcvSURF_NV61:
            format = gcvSURF_NV61;
            nPlane = 2;
            width[0] = head.imageWidth;
            height[0] = head.imageHeight;
            bpp[0] = 1;
            width[1] = head.imageWidth / 2;
            height[1] = head.imageHeight;
            bpp[1] = 2;
            break;

        case gcvSURF_NV12:
            format = gcvSURF_NV12;
            nPlane = 2;
            width[0] = head.imageWidth;
            height[0] = head.imageHeight;
            bpp[0] = 1;
            width[1] = head.imageWidth / 2;
            height[1] = head.imageHeight / 2;
            bpp[1] = 2;
            if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TPCV11_COMPRESSION) ||
                gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_DEC400_COMPRESSION))
                widthT = gcmALIGN(head.imageWidth, 256);
            break;

        case gcvSURF_NV21:
            format = gcvSURF_NV21;
            nPlane = 2;
            width[0] = head.imageWidth;
            height[0] = head.imageHeight;
            bpp[0] = 1;
            width[1] = head.imageWidth / 2;
            height[1] = head.imageHeight / 2;
            bpp[1] = 2;
            if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TPCV11_COMPRESSION) ||
                gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_DEC400_COMPRESSION))
                widthT = gcmALIGN(head.imageWidth, 256);
            break;

        case gcvSURF_NV16_10BIT:
            format = gcvSURF_NV16_10BIT;
            nPlane = 2;
            width[0] = head.imageWidth;
            height[0] = head.imageHeight;
            bpp[0] = 1.25;
            width[1] = head.imageWidth / 2;
            height[1] = head.imageHeight;
            bpp[1] = 2.5;
            break;

        case gcvSURF_NV61_10BIT:
            format = gcvSURF_NV61_10BIT;
            nPlane = 2;
            width[0] = head.imageWidth;
            height[0] = head.imageHeight;
            bpp[0] = 1.25;
            width[1] = head.imageWidth / 2;
            height[1] = head.imageHeight;
            bpp[1] = 2.5;
            break;

        case gcvSURF_NV12_10BIT:
            format = gcvSURF_NV12_10BIT;
            nPlane = 2;
            width[0] = head.imageWidth;
            height[0] = head.imageHeight;
            bpp[0] = 1.25;
            width[1] = head.imageWidth / 2;
            height[1] = head.imageHeight / 2;
            bpp[1] = 2.5;
            if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TPCV11_COMPRESSION) ||
                gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_DEC400_COMPRESSION))
                widthT = gcmALIGN(head.imageWidth, 256);
            break;

        case gcvSURF_NV21_10BIT:
            format = gcvSURF_NV21_10BIT;
            nPlane = 2;
            width[0] = head.imageWidth;
            height[0] = head.imageHeight;
            bpp[0] = 1.25;
            width[1] = head.imageWidth / 2;
            height[1] = head.imageHeight / 2;
            bpp[1] = 2.5;
            if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TPCV11_COMPRESSION) ||
                gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_DEC400_COMPRESSION))
                widthT = gcmALIGN(head.imageWidth, 256);
            break;

        default:
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
            break;
        }

        gcmONERROR(gcoSURF_Construct(gcvNULL, widthT, head.imageHeight, 1,
            gcvSURF_BITMAP, format, gcvPOOL_DEFAULT, &surf));

        gcmONERROR(gcoSURF_GetAlignedSize(surf, &alignedWidth,
            &alignedHeight, &alignedStride));

        aStride = (gctUINT32)((gctFLOAT)alignedWidth * bpp[0]);
        lineSize = (gctUINT32)((gctFLOAT)width[0] * bpp[0]);
        gcmONERROR(gcoSURF_Lock(surf, address, memory));

        {
            gctUINT32 addressT, aligned;

            if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TPCV11_COMPRESSION) ||
                gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_DEC400_COMPRESSION))
            {
                memset(memory[0], 0, alignedStride * alignedHeight * 2);
            }

            switch (head.format)
            {
                case gcvSURF_NV16_10BIT:
                case gcvSURF_NV61_10BIT:
                    addressT = gcmALIGN_NP2(address[0], 80);

                    memory[0] = GAL_POINTER_OFFSET(memory[0], addressT - address[0]);
                    memory[1] = GAL_POINTER_OFFSET(memory[1], addressT - address[0]);

                    address[1] += addressT - address[0];
                    address[0] += addressT - address[0];
                    break;

                case gcvSURF_NV12_10BIT:
                case gcvSURF_NV21_10BIT:
                    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TPCV11_COMPRESSION) ||
                        gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_DEC400_COMPRESSION))
                    {
                        aligned = 320;
                    }
                    else
                    {
                        aligned = 80;
                    }

                    addressT = gcmALIGN_NP2(address[0], aligned);

                    memory[0] = GAL_POINTER_OFFSET(memory[0], addressT - address[0]);
                    memory[1] = GAL_POINTER_OFFSET(memory[1], addressT - address[0]);

                    address[1] += addressT - address[0];
                    address[0] += addressT - address[0];
                break;

                default:
                    break;
            }
        }

        if (memory[0] == gcvNULL)
        {
            gcmONERROR(gcvSTATUS_INVALID_DATA);
        }
        if(gcoHAL_IsFeatureAvailable(NULL, gcvFEATURE_2D_YUV420_OUTPUT_LINEAR))
        {
            if(head.format == gcvSURF_YV12 || head.format == gcvSURF_I420 || head.format == gcvSURF_NV12 ||
                 head.format == gcvSURF_NV21 || head.format == gcvSURF_NV16 || head.format == gcvSURF_NV61)
                aStride = alignedStride;
        }


        gcmONERROR(gcoOS_Seek(
                gcvNULL,
                file,
                head.bitsOffset,
                gcvFILE_SEEK_SET
                ));

        for (n = 0; n < height[0]; n++)
        {
            /* Fill plane 1. */
            gctUINT8_PTR p = (gctUINT8_PTR)memory[0] + n * aStride;

            gcmONERROR(gcoOS_Read(
                    gcvNULL,
                    file,
                    lineSize,
                    p,
                    gcvNULL
                    ));

            if (GalIsBigEndian() && (bpp[0] == 2))
            {
                gctUINT16 *s = (gctUINT16 *)p;

                for (i = 0; i < lineSize; i += 2)
                {
                    *s = swap_word(*s);
                    s++;
                }
            }
        }

        if (nPlane > 1)
        {
            gctUINT aStride1 = (gctUINT32)((gctFLOAT)(alignedWidth/2) * bpp[1]);
            gctUINT lineSize1 = (gctUINT32)((gctFLOAT)width[1] * bpp[1]);

            if(gcoHAL_IsFeatureAvailable(NULL, gcvFEATURE_2D_YUV420_OUTPUT_LINEAR))
            {

                if(head.format == gcvSURF_YV12 || head.format == gcvSURF_I420)
                    aStride1 = alignedStride / 2;
                else if(head.format == gcvSURF_NV12 || head.format == gcvSURF_NV21 ||
                    head.format == gcvSURF_NV16 || head.format == gcvSURF_NV61)
                    aStride1 = alignedStride;
            }

            if (memory[1] == gcvNULL)
            {
                gcmONERROR(gcvSTATUS_INVALID_DATA);
            }

            for (n = 0; n < height[1]; n++)
            {
                /* Fill plane 2. */
                gctUINT8_PTR p = (gctUINT8_PTR)memory[swap ? 2 : 1] + n * aStride1;

                gcmONERROR(gcoOS_Read(
                        gcvNULL,
                        file,
                        lineSize1,
                        p,
                        gcvNULL
                        ));

                if (GalIsBigEndian() && (bpp[1] == 2))
                {
                    gctUINT16 *s = (gctUINT16 *)p;

                    for (i = 0; i < lineSize1; i += 2)
                    {
                        *s = swap_word(*s);
                        s++;
                    }
                }
            }

            if (nPlane > 2)
            {
                gctUINT aStride2;
                gctUINT lineSize2;

                if (nPlane != 3)
                {
                    gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
                }

                if (memory[2] == gcvNULL)
                {
                    gcmONERROR(gcvSTATUS_INVALID_DATA);
                }

                aStride2 = (gctUINT32)((alignedWidth/2) * bpp[2]);
                lineSize2 = (gctUINT32)(width[2] * bpp[2]);
                if(gcoHAL_IsFeatureAvailable(NULL, gcvFEATURE_2D_YUV420_OUTPUT_LINEAR))
                {
                    if(head.format == gcvSURF_YV12 || head.format == gcvSURF_I420)
                        aStride2 = alignedStride / 2;
                    else if(head.format == gcvSURF_NV12 || head.format == gcvSURF_NV21 ||
                        head.format == gcvSURF_NV16 || head.format == gcvSURF_NV61)
                        aStride2 = alignedStride;
                }
                for (n = 0; n < height[2]; n++)
                {
                    /* Fill plane 3. */
                    gctUINT8_PTR p = (gctUINT8_PTR)memory[swap ? 1 : 2] + n * aStride2;

                    gcmONERROR(gcoOS_Read(
                            gcvNULL,
                            file,
                            lineSize2,
                            p,
                            gcvNULL
                            ));

                    if (GalIsBigEndian() && (bpp[2] == 2))
                    {
                        gctUINT16 *s = (gctUINT16 *)p;

                        for (i = 0; i < lineSize2; i += 2)
                        {
                            *s = swap_word(*s);
                            s++;
                        }
                    }
                }
            }
        }

        gcmONERROR(gcoSURF_Unlock(surf, memory));
        memory[0] = gcvNULL;

        gcmONERROR(gcoOS_Close(gcvNULL, file));

        gcmONERROR(gcoSURF_CPUCacheOperation(surf, gcvCACHE_FLUSH));

        *surface = surf;

        return gcvSTATUS_OK;

    } while (gcvFALSE);

OnError:
    if (surf && memory[0])
    {
        gcmVERIFY_OK(gcoSURF_Unlock(surf, memory));
    }

    if (surf)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(surf));
    }

    if (file)
    {
        gcmVERIFY_OK(gcoOS_Close(gcvNULL, file));
    }

    return status;
}

gceSTATUS CDECL GalLoadVimgToSurfaceWithPool(
    IN gctCONST_STRING filename,
    IN gcePOOL pool,
    OUT gcoSURF *surface
    )
{
    gctFILE file = gcvNULL;
    gceSTATUS status;
    gctPOINTER memory[3] = {0, 0, 0};
    gcoSURF surf = gcvNULL;

    do {
        VIMG_FILEHEADER fileHead;
        VIMG_V1 head;
        gctUINT alignedWidth, alignedHeight;
        gctINT alignedStride;
        gctUINT32 lineSize, aStride;
        gctUINT32 address[3] = {0, 0, 0};
        gctUINT32 width[3] = {0, 0, 0};
        gctUINT32 height[3] = {0, 0, 0};
        gctUINT32 bpp[3] = {0, 0, 0};
        gctUINT nPlane, n, i;
        gcoSURF surf;
        gceSURF_FORMAT format;
        gctBOOL swap = gcvFALSE;

        gcmONERROR(gcoOS_Open(NULL, filename, gcvFILE_READ, &file));

        gcmONERROR(gcoOS_Read(
                NULL,
                file,
                sizeof(VIMG_FILEHEADER),
                &fileHead,
                gcvNULL
                ));

        if (fileHead.magic[0] != 'V' || fileHead.magic[1] != 'I'
                || fileHead.magic[2] != 'V')
        {
            gcmONERROR(gcvSTATUS_INVALID_DATA);
        }

        if (fileHead.version == 1)
        {
            head.format      = read_dword(file);
            head.tiling      = read_dword(file);
            head.imageStride = read_dword(file);
            head.imageWidth  = read_dword(file);
            head.imageHeight = read_dword(file);
            head.bitsOffset  = read_dword(file);

            /* translate the tiling. */
            gcmONERROR(ConvertToGalTiling(head.tiling, &head.tiling, gcvNULL));
        }
        else
        {
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }

        gcmONERROR(gcoOS_Seek(
            NULL,
            file,
            0,
            gcvFILE_SEEK_END));

        gcmONERROR(gcoOS_GetPos(
            NULL,
            file,
            &n));

        if (n - head.bitsOffset < head.imageHeight * head.imageStride)
        {
            gcmONERROR(gcvSTATUS_OUT_OF_RESOURCES);
        }

        if ((head.imageWidth == 0) || (head.imageHeight == 0) || (head.tiling != gcvLINEAR))
        {
            gcmONERROR(gcvSTATUS_INVALID_DATA);
        }

        /* Check the type. */
        switch (head.format)
        {
        case gcvSURF_A8:
            format = gcvSURF_A8;
            nPlane = 1;
            width[0] = head.imageWidth;
            height[0] = head.imageHeight;
            bpp[0] = 1;
            break;

        case gcvSURF_YUY2:
            format = gcvSURF_YUY2;
            nPlane = 1;
            width[0] = head.imageWidth;
            height[0] = head.imageHeight;
            bpp[0] = 2;
            break;

        case gcvSURF_YVYU:
            format = gcvSURF_YVYU;
            nPlane = 1;
            width[0] = head.imageWidth;
            height[0] = head.imageHeight;
            bpp[0] = 2;
            break;

        case gcvSURF_UYVY:
            format = gcvSURF_UYVY;
            nPlane = 1;
            width[0] = head.imageWidth;
            height[0] = head.imageHeight;
            bpp[0] = 2;
            break;

        case gcvSURF_VYUY:
            format = gcvSURF_VYUY;
            nPlane = 1;
            width[0] = head.imageWidth;
            height[0] = head.imageHeight;
            bpp[0] = 2;
            break;

        case gcvSURF_I420:
            format = gcvSURF_I420;
            nPlane = 3;
            width[0] = head.imageWidth;
            height[0] = head.imageHeight;
            bpp[0] = 1;
            width[1] = width[2] = head.imageWidth / 2;
            height[1] = height[2] = head.imageHeight / 2;
            bpp[1] = bpp[2] = 1;
            break;

        case gcvSURF_YV12:
            format = gcvSURF_YV12;
            nPlane = 3;
            width[0] = head.imageWidth;
            height[0] = head.imageHeight;
            bpp[0] = 1;
            width[1] = width[2] = head.imageWidth / 2;
            height[1] = height[2] = head.imageHeight / 2;
            bpp[1] = bpp[2] = 1;
            swap = gcvTRUE;
            break;

        case gcvSURF_NV16:
            format = gcvSURF_NV16;
            nPlane = 2;
            width[0] = head.imageWidth;
            height[0] = head.imageHeight;
            bpp[0] = 1;
            width[1] = head.imageWidth / 2;
            height[1] = head.imageHeight;
            bpp[1] = 2;
            break;

        case gcvSURF_NV12:
            format = gcvSURF_NV12;
            nPlane = 2;
            width[0] = head.imageWidth;
            height[0] = head.imageHeight;
            bpp[0] = 1;
            width[1] = head.imageWidth / 2;
            height[1] = head.imageHeight / 2;
            bpp[1] = 2;
            break;

        case gcvSURF_NV21:
            format = gcvSURF_NV21;
            nPlane = 2;
            width[0] = head.imageWidth;
            height[0] = head.imageHeight;
            bpp[0] = 1;
            width[1] = head.imageWidth / 2;
            height[1] = head.imageHeight / 2;
            bpp[1] = 2;
            break;

        case gcvSURF_NV61:
            format = gcvSURF_NV61;
            nPlane = 2;
            width[0] = head.imageWidth;
            height[0] = head.imageHeight;
            bpp[0] = 1;
            width[1] = head.imageWidth / 2;
            height[1] = head.imageHeight;
            bpp[1] = 2;
            break;

        default:
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
            break;
        }

        gcmONERROR(gcoSURF_Construct(gcvNULL, head.imageWidth+64, head.imageHeight+64, 1,
            gcvSURF_BITMAP, format, pool, &surf));

        gcmONERROR(gcoSURF_GetAlignedSize(surf, &alignedWidth,
            &alignedHeight, &alignedStride));

        gcmONERROR(gcoSURF_Lock(surf, address, memory));

        if (memory[0] == gcvNULL)
        {
            gcmONERROR(gcvSTATUS_INVALID_DATA);
        }

        aStride = alignedWidth * bpp[0];
        lineSize = width[0] * bpp[0];

        gcmONERROR(gcoOS_Seek(
                gcvNULL,
                file,
                head.bitsOffset,
                gcvFILE_SEEK_SET
                ));

        for (n = 0; n < height[0]; n++)
        {
            /* Fill plane 1. */
            gctUINT8_PTR p = (gctUINT8_PTR)memory[0] + n * aStride;

            gcmONERROR(gcoOS_Read(
                    gcvNULL,
                    file,
                    lineSize,
                    p,
                    gcvNULL
                    ));

            if (GalIsBigEndian() && (bpp[0] == 2))
            {
                gctUINT16 *s = (gctUINT16 *)p;

                for (i = 0; i < lineSize; i += 2)
                {
                    *s = swap_word(*s);
                    s++;
                }
            }
        }

        if (nPlane > 1)
        {
            gctUINT aStride1 = (alignedWidth/2) * bpp[1];
            gctUINT lineSize1 = width[1] * bpp[1];

            if (memory[1] == gcvNULL)
            {
                gcmONERROR(gcvSTATUS_INVALID_DATA);
            }

            for (n = 0; n < height[1]; n++)
            {
                /* Fill plane 2. */
                gctUINT8_PTR p = (gctUINT8_PTR)memory[swap ? 2 : 1] + n * aStride1;

                gcmONERROR(gcoOS_Read(
                        gcvNULL,
                        file,
                        lineSize1,
                        p,
                        gcvNULL
                        ));

                if (GalIsBigEndian() && (bpp[1] == 2))
                {
                    gctUINT16 *s = (gctUINT16 *)p;

                    for (i = 0; i < lineSize1; i += 2)
                    {
                        *s = swap_word(*s);
                        s++;
                    }
                }
            }

            if (nPlane > 2)
            {
                gctUINT aStride2;
                gctUINT lineSize2;

                if (nPlane != 3)
                {
                    gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
                }

                if (memory[2] == gcvNULL)
                {
                    gcmONERROR(gcvSTATUS_INVALID_DATA);
                }

                aStride2 = (alignedWidth/2) * bpp[2];
                lineSize2 = width[2] * bpp[2];
                for (n = 0; n < height[2]; n++)
                {
                    /* Fill plane 3. */
                    gctUINT8_PTR p = (gctUINT8_PTR)memory[swap ? 1 : 2] + n * aStride2;

                    gcmONERROR(gcoOS_Read(
                            gcvNULL,
                            file,
                            lineSize2,
                            p,
                            gcvNULL
                            ));

                    if (GalIsBigEndian() && (bpp[2] == 2))
                    {
                        gctUINT16 *s = (gctUINT16 *)p;

                        for (i = 0; i < lineSize2; i += 2)
                        {
                            *s = swap_word(*s);
                            s++;
                        }
                    }
                }
            }
        }

        gcmONERROR(gcoSURF_Unlock(surf, memory));
        memory[0] = gcvNULL;

        gcmONERROR(gcoOS_Close(gcvNULL, file));

        gcmONERROR(gcoSURF_CPUCacheOperation(surf, gcvCACHE_FLUSH));

        *surface = surf;

        return gcvSTATUS_OK;

    } while (gcvFALSE);

OnError:
    if (surf && memory[0])
    {
        gcmVERIFY_OK(gcoSURF_Unlock(surf, memory));
    }

    if (surf)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(surf));
    }

    if (file)
    {
        gcmVERIFY_OK(gcoOS_Close(gcvNULL, file));
    }

    return status;
}

gceSTATUS CDECL GalLoadRawToSurface(
    IN gctCONST_STRING filename,
    IN gctUINT32 imageWidth,
    IN gctUINT32 imageHeight,
    IN gctUINT32 stride,
    IN gceSURF_FORMAT format,
    OUT gcoSURF *surface
    )
{
    gctFILE file = gcvNULL;
    gceSTATUS status;
    gctPOINTER memory[3] = {0, 0, 0};
    gcoSURF surf = gcvNULL;

    do {
        gctUINT alignedWidth, alignedHeight;
        gctINT alignedStride;
        gctUINT32 lineSize, aStride;
        gctUINT32 address[3] = {0, 0, 0};
        gctUINT32 width[3] = {0, 0, 0};
        gctUINT32 height[3] = {0, 0, 0};
        gctUINT32 bpp[3] = {0, 0, 0};
        gctUINT nPlane, n, i;
        gcoSURF surf;
        gctBOOL swap = gcvFALSE;

        gcmONERROR(gcoOS_Open(NULL, filename, gcvFILE_READ, &file));

        /* Check the type. */
        switch (format)
        {
        case gcvSURF_A8:
            nPlane = 1;
            width[0] = imageWidth;
            height[0] = imageHeight;
            bpp[0] = 1;
            break;

        case gcvSURF_YUY2:
        case gcvSURF_YVYU:
            nPlane = 1;
            width[0] = imageWidth;
            height[0] = imageHeight;
            bpp[0] = 2;
            break;

        case gcvSURF_UYVY:
        case gcvSURF_VYUY:
            nPlane = 1;
            width[0] = imageWidth;
            height[0] = imageHeight;
            bpp[0] = 2;
            break;

        case gcvSURF_I420:
            nPlane = 3;
            width[0] = imageWidth;
            height[0] = imageHeight;
            bpp[0] = 1;
            width[1] = width[2] = imageWidth / 2;
            height[1] = height[2] = imageHeight / 2;
            bpp[1] = bpp[2] = 1;
            break;

        case gcvSURF_YV12:
            nPlane = 3;
            width[0] = imageWidth;
            height[0] = imageHeight;
            bpp[0] = 1;
            width[1] = width[2] = imageWidth / 2;
            height[1] = height[2] = imageHeight / 2;
            bpp[1] = bpp[2] = 1;
            swap = gcvTRUE;
            break;

        case gcvSURF_NV16:
            nPlane = 2;
            width[0] = imageWidth;
            height[0] = imageHeight;
            bpp[0] = 1;
            width[1] = imageWidth / 2;
            height[1] = imageHeight;
            bpp[1] = 2;
            break;

        case gcvSURF_NV12:
            nPlane = 2;
            width[0] = imageWidth;
            height[0] = imageHeight;
            bpp[0] = 1;
            width[1] = imageWidth / 2;
            height[1] = imageHeight / 2;
            bpp[1] = 2;
            break;

        case gcvSURF_NV21:
            nPlane = 2;
            width[0] = imageWidth;
            height[0] = imageHeight;
            bpp[0] = 1;
            width[1] = imageWidth / 2;
            height[1] = imageHeight / 2;
            bpp[1] = 2;
            break;

        case gcvSURF_NV61:
            nPlane = 2;
            width[0] = imageWidth;
            height[0] = imageHeight;
            bpp[0] = 1;
            width[1] = imageWidth / 2;
            height[1] = imageHeight;
            bpp[1] = 2;
            break;

        default:
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
            break;
        }

        gcmONERROR(gcoSURF_Construct(gcvNULL, imageWidth, imageHeight, 1,
            gcvSURF_BITMAP, format, gcvPOOL_DEFAULT, &surf));

        gcmONERROR(gcoSURF_GetAlignedSize(surf, &alignedWidth,
            &alignedHeight, &alignedStride));

        gcmONERROR(gcoSURF_Lock(surf, address, memory));

        if (memory[0] == gcvNULL)
        {
            gcmONERROR(gcvSTATUS_INVALID_DATA);
        }

        aStride = alignedWidth * bpp[0];
        lineSize = width[0] * bpp[0];

        for (n = 0; n < height[0]; n++)
        {
            /* Fill plane 1. */
            gctUINT8_PTR p = (gctUINT8_PTR)memory[0] + n * aStride;

            gcmONERROR(gcoOS_Read(
                    gcvNULL,
                    file,
                    lineSize,
                    p,
                    gcvNULL
                    ));

            if (GalIsBigEndian() && (bpp[0] == 2))
            {
                gctUINT16 *s = (gctUINT16 *)p;

                for (i = 0; i < lineSize; i += 2)
                {
                    *s = swap_word(*s);
                    s++;
                }
            }
        }

        if (nPlane > 1)
        {
            gctUINT aStride1 = (alignedWidth/2) * bpp[1];
            gctUINT lineSize1 = width[1] * bpp[1];

            if (memory[1] == gcvNULL)
            {
                gcmONERROR(gcvSTATUS_INVALID_DATA);
            }

            for (n = 0; n < height[1]; n++)
            {
                /* Fill plane 2. */
                gctUINT8_PTR p = (gctUINT8_PTR)memory[swap ? 2 : 1] + n * aStride1;

                gcmONERROR(gcoOS_Read(
                        gcvNULL,
                        file,
                        lineSize1,
                        p,
                        gcvNULL
                        ));

                if (GalIsBigEndian() && (bpp[1] == 2))
                {
                    gctUINT16 *s = (gctUINT16 *)p;

                    for (i = 0; i < lineSize1; i += 2)
                    {
                        *s = swap_word(*s);
                        s++;
                    }
                }
            }

            if (nPlane > 2)
            {
                gctUINT aStride2;
                gctUINT lineSize2;

                if (nPlane != 3)
                {
                    gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
                }

                if (memory[2] == gcvNULL)
                {
                    gcmONERROR(gcvSTATUS_INVALID_DATA);
                }

                aStride2 = (alignedWidth/2) * bpp[2];
                lineSize2 = width[2] * bpp[2];
                for (n = 0; n < height[2]; n++)
                {
                    /* Fill plane 3. */
                    gctUINT8_PTR p = (gctUINT8_PTR)memory[swap ? 1 : 2] + n * aStride2;

                    gcmONERROR(gcoOS_Read(
                            gcvNULL,
                            file,
                            lineSize2,
                            p,
                            gcvNULL
                            ));

                    if (GalIsBigEndian() && (bpp[2] == 2))
                    {
                        gctUINT16 *s = (gctUINT16 *)p;

                        for (i = 0; i < lineSize2; i += 2)
                        {
                            *s = swap_word(*s);
                            s++;
                        }
                    }
                }
            }
        }

        gcmONERROR(gcoSURF_Unlock(surf, memory));
        memory[0] = gcvNULL;

        gcmONERROR(gcoOS_Close(gcvNULL, file));

        gcmONERROR(gcoSURF_CPUCacheOperation(surf, gcvCACHE_FLUSH));

        *surface = surf;

        return gcvSTATUS_OK;

    } while (gcvFALSE);

OnError:
    if (surf && memory[0])
    {
        gcmVERIFY_OK(gcoSURF_Unlock(surf, memory));
    }

    if (surf)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(surf));
    }

    if (file)
    {
        gcmVERIFY_OK(gcoOS_Close(gcvNULL, file));
    }

    return status;
}

gctUINT32 CDECL GalGetStretchFactor(
    gctINT32 SrcSize,
    gctINT32 DestSize
    )
{
    gctUINT stretchFactor;

    if ( (SrcSize > 0) && (DestSize > 1) )
    {
        stretchFactor = ((SrcSize - 1) << 16) / (DestSize - 1);
    }
    else
    {
        stretchFactor = 0;
    }

    return stretchFactor;
}

gceSTATUS CDECL GalGetStretchFactors(
    IN gcsRECT_PTR SrcRect,
    IN gcsRECT_PTR DestRect,
    OUT gctUINT32 * HorFactor,
    OUT gctUINT32 * VerFactor
    )
{
    if (HorFactor != gcvNULL)
    {
        gctINT32 src, dest;

        /* Compute width of rectangles. */
        gcmVERIFY_OK(gcsRECT_Width(SrcRect, &src));
        gcmVERIFY_OK(gcsRECT_Width(DestRect, &dest));

        /* Compute and return horizontal stretch factor. */
        *HorFactor = GalGetStretchFactor(src, dest);
    }

    if (VerFactor != gcvNULL)
    {
        gctINT32 src, dest;

        /* Compute height of rectangles. */
        gcmVERIFY_OK(gcsRECT_Height(SrcRect, &src));
        gcmVERIFY_OK(gcsRECT_Height(DestRect, &dest));

        /* Compute and return vertical stretch factor. */
        *VerFactor = GalGetStretchFactor(src, dest);
    }

    /* Success. */
    return gcvSTATUS_OK;
}

// Generate randam rects in the surface size
gctBOOL CDECL GalRandRect(gcsRECT* rect, gctUINT rectNum, gctUINT rectWidth, gctUINT rectHeight,
                 gctUINT surfWidth, gctUINT surfHeight)
{
    gctUINT RANGE_WIDTH;
    gctUINT RANGE_HIGHT;
    gctUINT i;

    if (!rect || rectNum == 0 || rectWidth > surfWidth || rectHeight > surfHeight)
        return gcvFALSE;

    RANGE_WIDTH = surfWidth - rectWidth;
    RANGE_HIGHT = surfHeight - rectHeight;
    srand( (unsigned)time( NULL ));

    for (i = 0; i < rectNum; i++ )
    {
        rect[i].left = (gctINT32)(( (double)rand() / (double)RAND_MAX) * RANGE_WIDTH);
        rect[i].top = (gctINT32)(( (double)rand() / (double)RAND_MAX) * RANGE_HIGHT);
        rect[i].right = rect[i].left + rectWidth;
        rect[i].bottom = rect[i].top + rectHeight;

        /*printf( "rect[%d]:Left, Top, Right, Bottom = %d,%d, %d,%d\n",
            i, rect[i].left, rect[i].top, rect[i].right, rect[i].bottom);*/
    }

    return gcvTRUE;
}

// Gal2DStartPixelEfficiencyCount
// start the 2D pixel count and return the current cycles
// Efficiency = (pixels/cycle)
gceSTATUS CDECL Gal2DStartPixelEfficiencyCount(gcoOS os, gctUINT32* startClock)
{
    gceSTATUS status = gcvSTATUS_OK;
    if (!startClock)
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    do {
        gcmERR_BREAK(gcoOS_WriteRegister(os, 0x0470, 0xF << 16));
        gcmERR_BREAK(gcoOS_WriteRegister(os, 0x0470, 0xB << 16));
        gcmERR_BREAK(gcoOS_ReadRegister(os, 0x0438, startClock));
    } while (gcvFALSE);

    return status;
}

// Gal2DStopPixelEfficiencyCount
// return the 2D pixel count and the current cycles
// Efficiency = (pixels/cycle)
gctUINT32 CDECL Gal2DStopPixelEfficiencyCount(gcoOS os, gctUINT32* endClock, gctUINT32* pixelCount)
{
    gceSTATUS status = gcvSTATUS_OK;
    if (!endClock || !pixelCount)
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }
    do {
        gcmERR_BREAK(gcoOS_ReadRegister(os, 0x0454, pixelCount));
        gcmERR_BREAK(gcoOS_ReadRegister(os, 0x0438, endClock));
    } while (gcvFALSE);

    return status;
}

gceSTATUS CDECL GalGetCurrentCycles(gcoOS os, gctUINT32* cycle)
{
    if (!cycle)
    {
        /* clear the cycles */
        return gcoOS_WriteRegister(os, 0x0438, 0);
    }
    else
    {
        return gcoOS_ReadRegister(os, 0x0438, cycle);
    }
}

gceSTATUS CDECL GalQueryUVStride(
    IN gceSURF_FORMAT Format,
    IN gctUINT32 yStride,
    OUT gctUINT32_PTR uStride,
    OUT gctUINT32_PTR vStride
    )
{
    switch (Format)
    {
    case gcvSURF_YUY2:
    case gcvSURF_UYVY:
    case gcvSURF_YVYU:
    case gcvSURF_VYUY:
        *uStride = *vStride = 0;
        break;

    case gcvSURF_I420:
    case gcvSURF_YV12:
        *uStride = *vStride = yStride / 2;
        break;

    case gcvSURF_NV16:
    case gcvSURF_NV12:
    case gcvSURF_NV61:
    case gcvSURF_NV21:
    case gcvSURF_NV16_10BIT:
    case gcvSURF_NV61_10BIT:
    case gcvSURF_NV12_10BIT:
    case gcvSURF_NV21_10BIT:
        *uStride = yStride;
        *vStride = 0;
        break;

    default:
        return gcvSTATUS_NOT_SUPPORTED;
    }

    return gcvSTATUS_OK;
}

gceSTATUS CDECL
GalIsYUVFormat(IN gceSURF_FORMAT Format)
{
    switch (Format)
    {
    case gcvSURF_YUY2:
    case gcvSURF_UYVY:
    case gcvSURF_YVYU:
    case gcvSURF_VYUY:
    case gcvSURF_I420:
    case gcvSURF_YV12:
    case gcvSURF_NV16:
    case gcvSURF_NV12:
    case gcvSURF_NV61:
    case gcvSURF_NV21:
    case gcvSURF_NV16_10BIT:
    case gcvSURF_NV61_10BIT:
    case gcvSURF_NV12_10BIT:
    case gcvSURF_NV21_10BIT:
    case gcvSURF_P010:
        return gcvSTATUS_TRUE;

    default:
        return gcvSTATUS_FALSE;
    }
}

gceSTATUS CDECL
GalStrSearch(
    IN gctCONST_STRING String,
    IN gctCONST_STRING SubString,
    OUT gctSTRING * Output
    )
{
    /* Verify the arguments. */
    if ((String == gcvNULL)
        || (SubString == gcvNULL)
        || (Output == gcvNULL))
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    *Output = strstr(String, SubString);

    /* Success. */
    return gcvSTATUS_OK;
}

gceSTATUS GalColorConvertFromARGB8(
    IN gceSURF_FORMAT Format,
    IN gctUINT32 NumColors,
    IN gctUINT32_PTR Color32,
    OUT gctUINT32_PTR Color
    )
{
    gctUINT32 i;

    for (i = 0; i < NumColors; i++)
    {
        gctUINT32 color32 = Color32[i];
        gctUINT32 colorR, colorG, colorB, colorA;
        gctUINT32_PTR color = &Color[i];

        /* Extract colors. */
        colorB = (color32 & 0xFF);
        colorG = (color32 & 0xFF00) >>  8;
        colorR = (color32 & 0xFF0000) >> 16;
        colorA = (color32 & 0xFF000000) >> 24;

        switch(Format)
        {
        case gcvSURF_X8R8G8B8:
        case gcvSURF_A8R8G8B8:
            /* No color conversion needed. */
            *color = color32;
            break;

        case gcvSURF_R8G8B8X8:
        case gcvSURF_R8G8B8A8:
            *color =
                (colorA) |
                (colorR << 24) |
                (colorG << 16) |
                (colorB <<  8);
            break;

        case gcvSURF_B8G8R8X8:
        case gcvSURF_B8G8R8A8:
            *color =
                (colorA) |
                (colorR <<  8) |
                (colorG << 16) |
                (colorB << 24);
            break;

        case gcvSURF_X8B8G8R8:
        case gcvSURF_A8B8G8R8:
            *color =
                (colorA << 24) |
                (colorR) |
                (colorG <<  8) |
                (colorB << 16);
            break;

        case gcvSURF_A1R5G5B5:
        case gcvSURF_X1R5G5B5:
            *color =
                ((colorA >> 7) << 15) |
                ((colorR >> 3) << 10) |
                ((colorG >> 3) <<  5) |
                ((colorB >> 3));
            /* Expand to 32bit. */
            *color = (*color << 16) | *color;
            break;

        case gcvSURF_R5G5B5A1:
        case gcvSURF_R5G5B5X1:
            *color =
                ((colorA >> 7)) |
                ((colorR >> 3) << 11) |
                ((colorG >> 3) <<  6) |
                ((colorB >> 3) <<  1);
            /* Expand to 32bit. */
            *color = (*color << 16) | *color;
            break;

        case gcvSURF_A1B5G5R5:
        case gcvSURF_X1B5G5R5:
            *color =
                ((colorA >> 7) << 15) |
                ((colorR >> 3)) |
                ((colorG >> 3) <<  5) |
                ((colorB >> 3) << 10);
            /* Expand to 32bit. */
            *color = (*color << 16) | *color;
            break;

        case gcvSURF_B5G5R5A1:
        case gcvSURF_B5G5R5X1:
            *color =
                ((colorA >> 7)) |
                ((colorR >> 3) <<  1) |
                ((colorG >> 3) <<  6) |
                ((colorB >> 3) << 11);
            /* Expand to 32bit. */
            *color = (*color << 16) | *color;
            break;

        case gcvSURF_A4R4G4B4:
        case gcvSURF_X4R4G4B4:
            *color =
                ((colorA >> 4) << 12) |
                ((colorR >> 4) <<  8) |
                ((colorG >> 4) <<  4) |
                ((colorB >> 4));
            /* Expand to 32bit. */
            *color = (*color << 16) | *color;
            break;

        case gcvSURF_R4G4B4A4:
        case gcvSURF_R4G4B4X4:
            *color =
                ((colorA >> 4)) |
                ((colorR >> 4) << 12) |
                ((colorG >> 4) <<  8) |
                ((colorB >> 4) <<  4);
            /* Expand to 32bit. */
            *color = (*color << 16) | *color;
            break;

        case gcvSURF_A4B4G4R4:
        case gcvSURF_X4B4G4R4:
            *color =
                ((colorA >> 4) << 12) |
                ((colorR >> 4)) |
                ((colorG >> 4) <<  4) |
                ((colorB >> 4) <<  8);
            /* Expand to 32bit. */
            *color = (*color << 16) | *color;
            break;

        case gcvSURF_B4G4R4A4:
        case gcvSURF_B4G4R4X4:
            *color =
                ((colorA >> 4)) |
                ((colorR >> 4) <<  4) |
                ((colorG >> 4) <<  8) |
                ((colorB >> 4) << 12);
            /* Expand to 32bit. */
            *color = (*color << 16) | *color;
            break;

        case gcvSURF_R5G6B5:
            *color =
                ((colorR >> 3) << 11) |
                ((colorG >> 2) <<  5) |
                ((colorB >> 3));
            /* Expand to 32bit. */
            *color = (*color << 16) | *color;
            break;

        case gcvSURF_B5G6R5:
            *color =
                ((colorR >> 3) |
                ((colorG >> 2) <<  5) |
                ((colorB >> 3) << 11));
            /* Expand to 32bit. */
            *color = (*color << 16) | *color;
            break;

        default:
            return gcvSTATUS_NOT_SUPPORTED;
        }
    }

    return gcvSTATUS_OK;
}

gceSTATUS CDECL GalCreateMonoStream(
    IN gcoOS Os,
    IN gctCONST_STRING filename,
    OUT GalMonoStream_PTR *MonoStream
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    BMPINFO *pInfo = gcvNULL;
    GalMonoStream *stream = gcvNULL;

    do {
        gcmERR_BREAK(gcoOS_Allocate(Os, sizeof(GalMonoStream), &stream));

        memset(stream, 0, sizeof(GalMonoStream));

        // create mask surface
         stream->data = GalLoadDIBitmap(filename, &pInfo);
        if (stream->data == gcvNULL)
        {
            gcmERR_BREAK(gcvSTATUS_GENERIC_IO);
        }

        if ((pInfo->bmiHeader.biWidth <= 0) ||
            (pInfo->bmiHeader.biWidth & 0x1F))
        {
            gcmERR_BREAK(gcvSTATUS_INVALID_OBJECT);
        }

        stream->width = pInfo->bmiHeader.biWidth;
        stream->type = gcvSURF_UNPACKED;

        if (pInfo->bmiHeader.biHeight > 0)
        {
            gctINT i;
            gctINT32 Stride
                = pInfo->bmiHeader.biWidth
                * pInfo->bmiHeader.biBitCount
                / 8;
            gctPOINTER temp;
            gctSTRING bits = stream->data;

            Stride = gcmALIGN(Stride, 4);
            temp = malloc(Stride);
            stream->height = pInfo->bmiHeader.biHeight;

            for (i = 0; i < pInfo->bmiHeader.biHeight/2; i++)
            {
                memcpy(temp, bits + i * Stride, Stride);
                memcpy(bits + i * Stride, bits + (pInfo->bmiHeader.biHeight - 1 - i) * Stride, Stride);
                memcpy(bits + (pInfo->bmiHeader.biHeight - 1 - i) * Stride, temp, Stride);
            }
            free(temp);
        }
        else
        {
            stream->height = -pInfo->bmiHeader.biHeight;
        }

        *MonoStream = stream;

    } while (gcvFALSE);

    if (pInfo)
    {
        free(pInfo);
    }

    if (gcmIS_ERROR(status) && stream)
    {
        if (stream->data != gcvNULL)
        {
            free(stream->data);
        }

        gcmVERIFY_OK(gcoOS_Free(Os, stream));
    }

    return status;
}

gceSTATUS CDECL GalDestroyMonoStream(
    IN gcoOS Os,
    IN GalMonoStream_PTR MonoStream
    )
{
    if (MonoStream)
    {
        if (MonoStream->data)
        {
            free(MonoStream->data);
        }

        return gcoOS_Free(Os, MonoStream);
    }
    else
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }
}

gceSTATUS CDECL GalLoadImageToSurface(
    IN gcoOS Os,
    IN gcoHAL Hal,
    IN gco2D egn2D,
    IN gctCONST_STRING sourcefile,
    IN gcoSURF surface)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 address[3];
    gctPOINTER memory[3] = {0, 0, 0};
    gctSTRING pos = gcvNULL;
    gcoSURF tmpSurf = gcvNULL;
    gctUINT32 tmpStride, tmpWidth, tmpHeight, tmpUStride, tmpVStride;
    gctUINT32 dstStride, dstWidth, dstHeight;
    gceSURF_FORMAT tmpFormat, dstFormat;
    gctUINT32 dstPhyAddr = 0;
    gctPOINTER dstLgcAddr;
    gcsRECT srcRect, dstRect;

    do {
        // create source surface
        gcmERR_BREAK(GalStrSearch(sourcefile, ".bmp", &pos));
        if (pos)
        {
            tmpSurf = GalLoadDIB2Surface(Hal,
                sourcefile);
            if (tmpSurf == NULL)
            {
                gcmERR_BREAK(gcvSTATUS_NOT_FOUND);
            }
        }
        else
        {
            gcmERR_BREAK(GalLoadYUV2Surface2(Os, Hal,
                sourcefile, &tmpSurf));
        }

        gcmERR_BREAK(gcoSURF_GetAlignedSize(tmpSurf,
                                            gcvNULL,
                                            gcvNULL,
                                            &tmpStride));

        gcmERR_BREAK(gcoSURF_GetSize(tmpSurf,
                                    &tmpWidth,
                                    &tmpHeight,
                                    gcvNULL));

        gcmERR_BREAK(gcoSURF_GetFormat(tmpSurf, gcvNULL, &tmpFormat));

        gcmERR_BREAK(gcoSURF_Lock(tmpSurf, address, memory));

        if (GalIsYUVFormat(tmpFormat))
        {
            gcmERR_BREAK(GalQueryUVStride(tmpFormat, tmpStride,
                &tmpUStride, &tmpVStride));
        }

        gcmERR_BREAK(gcoSURF_GetAlignedSize(surface,
                                            gcvNULL,
                                            gcvNULL,
                                            &dstStride));

        gcmERR_BREAK(gcoSURF_GetSize(surface,
                                    &dstWidth,
                                    &dstHeight,
                                    gcvNULL));

        gcmERR_BREAK(gcoSURF_GetFormat(surface, gcvNULL, &dstFormat));

        gcmERR_BREAK(gcoSURF_Lock(surface, &dstPhyAddr, &dstLgcAddr));

        gcmERR_BREAK(gco2D_SetKernelSize(egn2D, 3, 3));

        srcRect.left = 0;
        srcRect.right = tmpWidth;
        srcRect.top = 0;
        srcRect.bottom = tmpHeight;

        dstRect.left = 0;
        dstRect.right = dstWidth;
        dstRect.top = 0;
        dstRect.bottom = dstHeight;

        gcmERR_BREAK(gco2D_SetClipping(egn2D, &dstRect));

        gcmERR_BREAK(gco2D_FilterBlitEx(egn2D,
            address[0], tmpStride,
            address[1], tmpUStride,
            address[2], tmpVStride,
            tmpFormat, gcvSURF_0_DEGREE, tmpWidth, tmpHeight, &srcRect,
            dstPhyAddr, dstStride, dstFormat,
            gcvSURF_0_DEGREE, dstWidth, dstHeight, &dstRect, &dstRect));

        gcmERR_BREAK(gcoHAL_Commit(Hal, gcvTRUE));

    } while (gcvFALSE);

    if (dstLgcAddr)
    {
        gcmVERIFY_OK(gcoSURF_Unlock(surface, dstLgcAddr));
    }

    if (tmpSurf)
    {
        if (memory[0])
        {
            gcmVERIFY_OK(gcoSURF_Unlock(tmpSurf, memory));
        }

        gcmVERIFY_OK(gcoSURF_Destroy(tmpSurf));
    }

    return status;
}

const struct FormatInfo
{
    const char      *name;
    gceSURF_FORMAT  format;
} c_formatInfos[] =
{
    {"X4R4G4B4" , gcvSURF_X4R4G4B4},

    {"A4R4G4B4" , gcvSURF_A4R4G4B4},

    {"R4G4B4X4" , gcvSURF_R4G4B4X4},

    {"R4G4B4A4" , gcvSURF_R4G4B4A4},

    {"X4B4G4R4" , gcvSURF_X4B4G4R4},

    {"A4B4G4R4" , gcvSURF_A4B4G4R4},

    {"B4G4R4X4" , gcvSURF_B4G4R4X4},

    {"B4G4R4A4" , gcvSURF_B4G4R4A4},



    {"X1R5G5B5" , gcvSURF_X1R5G5B5},

    {"A1R5G5B5" , gcvSURF_A1R5G5B5},

    {"R5G5B5X1" , gcvSURF_R5G5B5X1},

    {"R5G5B5A1" , gcvSURF_R5G5B5A1},

    {"B5G5R5X1" , gcvSURF_B5G5R5X1},

    {"B5G5R5A1" , gcvSURF_B5G5R5A1},

    {"X1B5G5R5" , gcvSURF_X1B5G5R5},

    {"A1B5G5R5" , gcvSURF_A1B5G5R5},



    {"X8R8G8B8" , gcvSURF_X8R8G8B8},

    {"A8R8G8B8" , gcvSURF_A8R8G8B8},

    {"R8G8B8X8" , gcvSURF_R8G8B8X8},

    {"R8G8B8A8" , gcvSURF_R8G8B8A8},

    {"B8G8R8X8" , gcvSURF_B8G8R8X8},

    {"B8G8R8A8" , gcvSURF_B8G8R8A8},

    {"X8B8G8R8" , gcvSURF_X8B8G8R8},

    {"A8B8G8R8" , gcvSURF_A8B8G8R8},



    {"R5G6B5"   , gcvSURF_R5G6B5  },

    {"B5G6R5"   , gcvSURF_B5G6R5  },

    {"YUY2",    gcvSURF_YUY2},
    {"UYVY",    gcvSURF_UYVY},
    {"YVYU",    gcvSURF_YVYU},
    {"VYUY",    gcvSURF_VYUY},
    {"YV12",    gcvSURF_YV12},
    {"I420",    gcvSURF_I420},
    {"NV12",    gcvSURF_NV12},
    {"NV21",    gcvSURF_NV21},
    {"NV16",    gcvSURF_NV16},
    {"NV61",    gcvSURF_NV61},

    {"R10G10B10A2", gcvSURF_R10G10B10A2},
    {"B10G10R10A2", gcvSURF_B10G10R10A2},
    {"A2R10G10B10", gcvSURF_A2R10G10B10},
    {"A2B10G10R10", gcvSURF_A2B10G10R10},
    {"NV12_10BIT",  gcvSURF_NV12_10BIT},
    {"NV21_10BIT",  gcvSURF_NV21_10BIT},
    {"NV16_10BIT",  gcvSURF_NV16_10BIT},
    {"NV61_10BIT",  gcvSURF_NV61_10BIT},
};

gceSURF_FORMAT GalQueryFormat(const char *name)
{
    int i;

    for (i = 0; i < sizeof(c_formatInfos) / sizeof(c_formatInfos[0]); i++)
    {
        if (strcmp(name, c_formatInfos[i].name) == 0) {
            return c_formatInfos[i].format;
        }
    }

    return gcvSURF_UNKNOWN;
}

const char *GalQueryFormatStr(gceSURF_FORMAT format)
{
    int i;

    for (i = 0; i < sizeof(c_formatInfos) / sizeof(c_formatInfos[0]); i++)
    {
        if (c_formatInfos[i].format == format) {
            return c_formatInfos[i].name;
        }
    }

    return "Unknown Surface Format";
}

/*
 * Translate the local defined tiling values into the system defined values.
 */
static gceSTATUS
ConvertToGalTiling(
    IN gctUINT32 Value,
    OUT gctUINT32_PTR Tiling,
    OUT gctUINT32_PTR Vsupertile
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 tiling;
    gctUINT32 vsupertile;

    if ((Tiling == gcvNULL) && (Vsupertile == gcvNULL))
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    switch (GETBITS(Value, G2D_TILING_TYPE_START, G2D_TILING_TYPE_END))
    {
        case G2D_TILING_LINEAR:
            tiling = gcvLINEAR;
            break;

        case G2D_TILING_TILE:
            tiling = gcvTILED;
            break;

        case G2D_TILING_SUPERTILE:
            tiling = gcvSUPERTILED;
            break;

        case G2D_TILING_MINORTILE:
            tiling = gcvMINORTILED;
            break;

        case G2D_TILING_YMAJORSUPERTILE:
            tiling = gcvYMAJOR_SUPERTILED;
            break;

        case G2D_TILING_XMAJORSUPERTILE:
            tiling = gcvSUPERTILED;
            break;

        case G2D_TILING_SUPERTILE128B:
            tiling = gcvSUPERTILED_128B;
            break;

        case G2D_TILING_SUPERTILE256B:
            tiling = gcvSUPERTILED_256B;
            break;

        case G2D_TILING_XMAJOR8X8:
            tiling = gcvTILED_8X8_XMAJOR;
            break;

        case G2D_TILING_Y:
            tiling = gcvTILED_8X8_YMAJOR;
            break;

        case G2D_TILING_8X4:
            tiling = gcvTILED_8X4;
            break;

        case G2D_TILING_4X8:
            tiling = gcvTILED_4X8;
            break;

        case G2D_TILING_32X4:
            tiling = gcvTILED_32X4;
            break;

        case G2D_TILING_64X4:
            tiling = gcvTILED_64X4;
            break;

        default:
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    if (tiling & gcvSUPERTILED)
    {
        switch (GETBITS(Value, G2D_TILING_SUPERTILE_VERSION_START,
                    G2D_TILING_SUPERTILE_VERSION_END))
        {
            case G2D_TILING_SUPERTILE_VERSION_1:
                vsupertile = gcv2D_SUPER_TILE_VERSION_V1;
                break;

            case G2D_TILING_SUPERTILE_VERSION_2:
                vsupertile = gcv2D_SUPER_TILE_VERSION_V2;
                break;

            case G2D_TILING_SUPERTILE_VERSION_3:
                vsupertile = gcv2D_SUPER_TILE_VERSION_V3;
                break;

            default:
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        if (Vsupertile != gcvNULL)
        {
            *Vsupertile = vsupertile;
        }
    }

    if (GETBITS(Value, G2D_TILING_MULTI_START, G2D_TILING_MULTI_END)
            == G2D_TILING_MULTI_ON)
    {
#if gcvVERSION_MAJOR >= 5
        tiling |= gcvTILING_SPLIT_BUFFER;
#else
        if (tiling == gcvTILED)
        {
            tiling = gcvMULTI_TILED;
        }
        else if (tiling == gcvSUPERTILED)
        {
            tiling = gcvMULTI_SUPERTILED;
        }
        else
        {
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }
#endif
    }

    if (Tiling != gcvNULL)
    {
        *Tiling = tiling;
    }

OnError:
    return status;
}

gctBOOL CDECL GalIsBigEndian()
{
    static gctUINT16 data = 0xff00;
    return (*(gctUINT8 *)&data == 0xff);
}

gceSTATUS CDECL GalLoadFileToTSurf(
    IN gctCONST_STRING FileName,
    IN T2D_SURF_PTR *TSurf)
{
    return GalLoadFileToTSurfWithPool(FileName, gcvPOOL_DEFAULT, TSurf);
}

gceSTATUS CDECL GalLoadFileToTSurfWithPool(
    IN gctCONST_STRING FileName,
    IN gcePOOL Pool,
    IN T2D_SURF_PTR *TSurf)
{
    gceSTATUS status = gcvSTATUS_OK;

    if (strstr(FileName, ".bmp"))
    {
        gctUINT8_PTR bits = gcvNULL;
        BMPINFO *info = gcvNULL;
        T2D_SURF_PTR surf = gcvNULL;

        do {
            gctUINT32 width, srcWidth;
            gctUINT32 height, srcHeight;
            gctUINT32 i, j;
            gctUINT surfDepth, biDepth;
            gceSURF_FORMAT format;
            gctUINT8_PTR logical;

            bits = GalLoadDIBitmap(FileName, &info);
            if (!bits) {
                gcmERR_BREAK(gcvSTATUS_NOT_FOUND); ;
            }

            srcWidth  = abs(info->bmiHeader.biWidth);
            srcHeight = abs(info->bmiHeader.biHeight);

            width = gcmALIGN(srcWidth, 4);
            height = srcHeight;

            if (info->bmiHeader.biCompression == BIT_RGB)
            {
                if (info->bmiHeader.biBitCount == 32)
                {
                    format = gcvSURF_A8R8G8B8;
                    surfDepth = 4;
                    biDepth = 4;
                }
                else if (info->bmiHeader.biBitCount == 24)
                {
                    format = gcvSURF_A8R8G8B8;
                    surfDepth = 4;
                    biDepth = 3;
                }
                else if (info->bmiHeader.biBitCount == 16)
                {
                    format = gcvSURF_R5G6B5;
                    surfDepth = 2;
                    biDepth = 2;
                }
                else if (info->bmiHeader.biBitCount == 8)
                {
                    gco2D engn2d;
                    format = gcvSURF_INDEX8;
                    surfDepth = 1;
                    biDepth = 1;
                    gcmERR_BREAK(gcoHAL_Get2DEngine(gcvNULL, &engn2d));
                    gcmERR_BREAK(gco2D_LoadPalette(engn2d, 0, 256, info->bmiColors, gcvTRUE));
                }
                else {
                    GalOutput(GalOutputType_Error, "*ERROR* Unknown bmiHeader.biBitCount: %d", info->bmiHeader.biBitCount);
                    gcmERR_BREAK(gcvSTATUS_NOT_SUPPORTED);
                }
            }
            else if (info->bmiHeader.biCompression == BIT_BITFIELDS)
            {
                gctINT sizeR, sizeG, sizeB, sizeA;
                sizeR = GetBitsSize(info->mask[0]);
                sizeG = GetBitsSize(info->mask[1]);
                sizeB = GetBitsSize(info->mask[2]);
                sizeA =  - sizeR - sizeG - sizeB;

                switch (info->bmiHeader.biBitCount)
                {
                case 32:

                    if (info->mask[0] == 0x00FF0000
                        && info->mask[1] == 0x0000FF00
                        && info->mask[2] == 0x000000FF)
                    {
                        format = gcvSURF_A8R8G8B8;
                        surfDepth = 4;
                        biDepth = 4;
                    }
                    else if (info->mask[0] == 0xFF000000
                        && info->mask[1] == 0x00FF0000
                        && info->mask[2] == 0x0000FF00)
                    {
                        format = gcvSURF_R8G8B8A8;
                        surfDepth = 4;
                        biDepth = 4;
                    }
                    else if (info->mask[0] == 0x000000FF
                        && info->mask[1] == 0x0000FF00
                        && info->mask[2] == 0x00FF0000)
                    {
                        format = gcvSURF_A8B8G8R8;
                        surfDepth = 4;
                        biDepth = 4;
                    }
                    else if (info->mask[0] == 0x0000FF00
                        && info->mask[1] == 0x00FF0000
                        && info->mask[2] == 0xFF000000)
                    {
                        format = gcvSURF_B8G8R8A8;
                        surfDepth = 4;
                        biDepth = 4;
                    }
                    else if (info->mask[0] == 0xFFC00000
                        && info->mask[1] == 0x003FF000
                        && info->mask[2] == 0x00000FFC)
                    {
                        format = gcvSURF_R10G10B10A2;
                        surfDepth = 4;
                        biDepth = 4;
                    }
                    else if (info->mask[0] == 0x00000FFC
                        && info->mask[1] == 0x003FF000
                        && info->mask[2] == 0xFFC00000)
                    {
                        format = gcvSURF_B10G10R10A2;
                        surfDepth = 4;
                        biDepth = 4;
                    }
                    else if (info->mask[0] == 0x3FF00000
                        && info->mask[1] == 0x000FFC00
                        && info->mask[2] == 0x000003FF)
                    {
                        format = gcvSURF_A2R10G10B10;
                        surfDepth = 4;
                        biDepth = 4;
                    }
                    else if (info->mask[0] == 0x000003FF
                        && info->mask[1] == 0x000FFC00
                        && info->mask[2] == 0x3FF00000)
                    {
                        format = gcvSURF_A2B10G10R10;
                        surfDepth = 4;
                        biDepth = 4;
                    }
                    else
                    {
                        GalOutput(GalOutputType_Error, "*ERROR* unsupported format");
                        status = gcvSTATUS_NOT_SUPPORTED;
                    }

                    break;

                case 16:
                    if (info->mask[0] == 0x00000F00
                        && info->mask[1] == 0x000000F0
                        && info->mask[2] == 0x0000000F)
                    {
                        format = gcvSURF_A4R4G4B4;
                        surfDepth = 2;
                        biDepth = 2;
                    }
                    else if (info->mask[0] == 0x0000F000
                        && info->mask[1] == 0x00000F00
                        && info->mask[2] == 0x000000F0)
                    {
                        format = gcvSURF_R4G4B4A4;
                        surfDepth = 2;
                        biDepth = 2;
                    }
                    else if (info->mask[0] == 0x0000000F
                        && info->mask[1] == 0x000000F0
                        && info->mask[2] == 0x00000F00)
                    {
                        format = gcvSURF_A4B4G4R4;
                        surfDepth = 2;
                        biDepth = 2;
                    }
                    else if (info->mask[0] == 0x000000F0
                        && info->mask[1] == 0x00000F00
                        && info->mask[2] == 0x0000F000)
                    {
                        format = gcvSURF_B4G4R4A4;
                        surfDepth = 2;
                        biDepth = 2;
                    }
                    else if (info->mask[0] == 0x0000F800
                        && info->mask[1] == 0x000007E0
                        && info->mask[2] == 0x0000001F)
                    {
                        format = gcvSURF_R5G6B5;
                        surfDepth = 2;
                        biDepth = 2;
                    }
                    else if (info->mask[0] == 0x0000001F
                        && info->mask[1] == 0x000007E0
                        && info->mask[2] == 0x0000F800)
                    {
                        format = gcvSURF_B5G6R5;
                        surfDepth = 2;
                        biDepth = 2;
                    }
                    else if (info->mask[0] == 0x00007C00
                        && info->mask[1] == 0x000003E0
                        && info->mask[2] == 0x0000001F)
                    {
                        format = gcvSURF_A1R5G5B5;
                        surfDepth = 2;
                        biDepth = 2;
                    }
                    else if (info->mask[0] == 0x0000F800
                        && info->mask[1] == 0x000007C0
                        && info->mask[2] == 0x0000003E)
                    {
                        format = gcvSURF_R5G5B5A1;
                        surfDepth = 2;
                        biDepth = 2;
                    }
                    else if (info->mask[0] == 0x0000003E
                        && info->mask[1] == 0x000007C0
                        && info->mask[2] == 0x0000F800)
                    {
                        format = gcvSURF_B5G5R5A1;
                        surfDepth = 2;
                        biDepth = 2;
                    }
                    else if (info->mask[0] == 0x0000001F
                        && info->mask[1] == 0x000003E0
                        && info->mask[2] == 0x00007C00)
                    {
                        format = gcvSURF_A1B5G5R5;
                        surfDepth = 2;
                        biDepth = 2;
                    }
                    else
                    {
                        GalOutput(GalOutputType_Error, "*ERROR* unsupported format");
                        status = gcvSTATUS_NOT_SUPPORTED;
                    }

                    break;

                default:
                    GalOutput(GalOutputType_Error, "*ERROR* unsupported bpp:%d",
                        info->bmiHeader.biBitCount);
                        status = gcvSTATUS_NOT_SUPPORTED;
                }

                gcmERR_BREAK(status);

            }
            else {
                GalOutput(GalOutputType_Error, "*ERROR* unknown bmiHeader");
                gcmERR_BREAK(gcvSTATUS_NOT_SUPPORTED);
            }

            gcmERR_BREAK(GalCreateTSurfWithPool(gcvNULL, format, gcvLINEAR,
                gcv2D_TSC_DISABLE, width, height, Pool, &surf));

            logical = surf->logical[0];
            for (i = 0; i < srcHeight; i++)
            {
                for (j = 0; j < srcWidth; j++)
                {
                    gctUINT dstOff = i * surf->aWidth+ j;
                    gctUINT srcOff = (info->bmiHeader.biHeight >= 0) ?
                        ((srcHeight - 1 - i) * srcWidth + j) : (i * srcWidth + j);

                    if (GalIsBigEndian() && (biDepth == 3))
                    {
                        memcpy(logical + dstOff * surfDepth + 1, bits + srcOff * biDepth, biDepth);
                    }
                    else
                    {
                        memcpy(logical + dstOff * surfDepth, bits + srcOff * biDepth, biDepth);
                    }
                }
            }

            gcmERR_BREAK(gcoOS_CacheFlush(gcvNULL, surf->vNode.node, surf->vNode.memory, surf->vNode.size));

        } while(gcvFALSE);

        if (bits)
        {
            free(bits);
        }

        if (info)
        {
            free(info);
        }

        if (status == gcvSTATUS_OK)
        {
            *TSurf = surf;
        }
        else if (surf)
        {
            gcmVERIFY_OK(GalDeleteTSurf(gcvNULL, surf));
        }
    }
    else if (strstr(FileName, ".vimg"))
    {
        status = GalLoadVimgToTSurfWithPool(FileName, Pool, TSurf);
    }
    else
    {
        status = gcvSTATUS_NOT_SUPPORTED;
    }

    return status;
}

static
gceSTATUS CDECL GalLoadSpecialRawToTSurfWithPool(
    IN gctCONST_STRING DataName0,
    IN gctCONST_STRING DataName1,
    IN gctCONST_STRING DataName2,
    IN gctCONST_STRING TableName0,
    IN gctCONST_STRING TableName1,
    IN gctCONST_STRING TableName2,
    IN gctUINT32 ImageWidth,
    IN gctUINT32 ImageHeight,
    IN gceTILING Tiling,
    IN gceSURF_FORMAT Format,
    IN gce2D_TILE_STATUS_CONFIG TileStatusConfig,
    IN gcePOOL Pool,
    OUT T2D_SURF_PTR *TSurf
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctFILE file = gcvNULL;
    T2D_SURF_PTR surf = gcvNULL;

    gctSIZE_T read;
    gctUINT32 lineSize, aStride;
    gctUINT32 width[3] = {0, 0, 0};
    gctUINT32 height[3] = {0, 0, 0};
    gctUINT32 bpp[3] = {0, 0, 0};
    gctUINT nPlane, n, i;
    gctBOOL swap = gcvFALSE;

    /* Check the type. */
    switch (Format)
    {
        case gcvSURF_NV12:
            nPlane = 2;
            width[0] = ImageWidth;
            height[0] = ImageHeight;
            bpp[0] = 1;
            width[1] = ImageWidth / 2;
            height[1] = ImageHeight / 2;
            bpp[1] = 2;
            break;

        case gcvSURF_R5G6B5:
            nPlane = 1;
            width[0] = ImageWidth;
            height[0] = ImageHeight;
            bpp[0] = 2;
            break;

        case gcvSURF_A8R8G8B8:
        case gcvSURF_X8R8G8B8:
            nPlane = 1;
            width[0] = ImageWidth;
            height[0] = ImageHeight;
            bpp[0] = 4;
            break;

        default:
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
            break;
    }

    gcmONERROR(GalCreateTSurfWithPool(
        gcvNULL,
        Format, Tiling,
        TileStatusConfig,
        ImageWidth, ImageHeight,
        Pool, &surf));

    gcmONERROR(gcoOS_Open(NULL, DataName0, gcvFILE_READ, &file));

    aStride = surf->aWidth * bpp[0];
    lineSize = width[0] * bpp[0];

    /* Copy data */
    for (n = 0; n < height[0]; n++)
    {
        /* Fill plane 1. */
        gctUINT8_PTR p = (gctUINT8_PTR)surf->logical[0] + n * aStride;

        gcmONERROR(gcoOS_Read(
                gcvNULL,
                file,
                lineSize,
                p,
                &read
                ));

        if (GalIsBigEndian() && (bpp[0] == 2))
        {
            gctUINT16 *s = (gctUINT16 *)p;

            for (i = 0; i < lineSize; i += 2)
            {
                *s = swap_word(*s);
                s++;
            }
        }

        if (read != lineSize)
            break;
    }

    if (nPlane > 1)
    {
        gctUINT aStride1 = (surf->aWidth/2) * bpp[1];
        gctUINT lineSize1 = width[1] * bpp[1];

        if (surf->logical[1] == gcvNULL)
        {
            gcmONERROR(gcvSTATUS_INVALID_DATA);
        }

        if (DataName1 != gcvNULL)
        {
            if (file != gcvNULL)
            {
                gcmONERROR(gcoOS_Close(gcvNULL, file));
            }

            gcmONERROR(gcoOS_Open(NULL, DataName1, gcvFILE_READ, &file));
        }

        /* Copy data */
        for (n = 0; n < height[1]; n++)
        {
            /* Fill plane 2. */
            gctUINT8_PTR p = (gctUINT8_PTR)surf->logical[swap ? 2 : 1] + n * aStride1;

            gcmONERROR(gcoOS_Read(
                    gcvNULL,
                    file,
                    lineSize1,
                    p,
                    &read
                    ));

            if (GalIsBigEndian() && (bpp[1] == 2))
            {
                gctUINT16 *s = (gctUINT16 *)p;

                for (i = 0; i < lineSize1; i += 2)
                {
                    *s = swap_word(*s);
                    s++;
                }
            }

            if (read != lineSize1)
                break;
        }

        if (nPlane > 2)
        {
            gctUINT aStride2;
            gctUINT lineSize2;

            if (nPlane != 3)
            {
                gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
            }

            if (surf->logical[2] == gcvNULL)
            {
                gcmONERROR(gcvSTATUS_INVALID_DATA);
            }

            aStride2 = (surf->aWidth/2) * bpp[2];
            lineSize2 = width[2] * bpp[2];

            if (DataName2 != gcvNULL)
            {
                if (file != gcvNULL)
                {
                    gcmONERROR(gcoOS_Close(gcvNULL, file));
                }

                gcmONERROR(gcoOS_Open(NULL, DataName2, gcvFILE_READ, &file));
            }

            /* Copy data */
            for (n = 0; n < height[2]; n++)
            {
                /* Fill plane 3. */
                gctUINT8_PTR p = (gctUINT8_PTR)surf->logical[swap ? 1 : 2] + n * aStride2;

                gcmONERROR(gcoOS_Read(
                            gcvNULL,
                            file,
                            lineSize2,
                            p,
                            &read
                            ));

                if (GalIsBigEndian() && (bpp[2] == 2))
                {
                    gctUINT16 *s = (gctUINT16 *)p;

                    for (i = 0; i < lineSize2; i += 2)
                    {
                        *s = swap_word(*s);
                        s++;
                    }
                }

                if (read != lineSize2)
                    break;
            }
        }
    }

    if (file != gcvNULL)
    {
        gcmONERROR(gcoOS_Close(gcvNULL, file));
    }

    gcmONERROR(gcoOS_CacheFlush(gcvNULL, surf->vNode.node, surf->vNode.memory, surf->vNode.size));

    /* Tile status buffer */
    if (TableName0 != gcvNULL && surf->tileStatuslogical != gcvNULL)
    {
        gcmONERROR(gcoOS_Open(NULL, TableName0, gcvFILE_READ, &file));

        /* Copy tile status data */
        for (n = 0; n < height[0]; n++)
        {
            /* Fill plane 1. */
            gctUINT8_PTR p = (gctUINT8_PTR)surf->tileStatuslogical + n * aStride;

            gcmONERROR(gcoOS_Read(
                        gcvNULL,
                        file,
                        lineSize,
                        p,
                        &read
                        ));

            if (GalIsBigEndian() && (bpp[0] == 2))
            {
                gctUINT16 *s = (gctUINT16 *)p;

                for (i = 0; i < lineSize; i += 2)
                {
                    *s = swap_word(*s);
                    s++;
                }
            }

            if (read != lineSize)
                break;
        }

        gcmONERROR(gcoOS_Close(gcvNULL, file));
        gcmONERROR(gcoOS_CacheFlush(
            gcvNULL,
            surf->tileStatusNode.node,
            surf->tileStatusNode.memory,
            surf->tileStatusNode.size));

        if (nPlane > 1 && TableName1 != gcvNULL && surf->tileStatuslogicalEx[0] != gcvNULL)
        {
            gctUINT aStride1 = (surf->aWidth/2) * bpp[1];
            gctUINT lineSize1 = width[1] * bpp[1];

            gcmONERROR(gcoOS_Open(NULL, TableName1, gcvFILE_READ, &file));

            /* Copy tile status data */
            for (n = 0; n < height[1]; n++)
            {
                /* Fill plane 2. */
                gctUINT8_PTR p = (gctUINT8_PTR)surf->tileStatuslogicalEx[swap ? 1 : 0] + n * aStride1;

                gcmONERROR(gcoOS_Read(
                            gcvNULL,
                            file,
                            lineSize1,
                            p,
                            &read
                            ));

                if (GalIsBigEndian() && (bpp[1] == 2))
                {
                    gctUINT16 *s = (gctUINT16 *)p;

                    for (i = 0; i < lineSize1; i += 2)
                    {
                            *s = swap_word(*s);
                            s++;
                    }
                }

                if (read != lineSize1)
                        break;
            }

            gcmONERROR(gcoOS_Close(gcvNULL, file));

            gcmONERROR(gcoOS_CacheFlush(
                    gcvNULL,
                    surf->tileStatusNodeEx[0].node,
                    surf->tileStatusNodeEx[0].memory,
                    surf->tileStatusNodeEx[0].size));

            if (nPlane > 2 && TableName2 != gcvNULL && surf->tileStatuslogicalEx[1] != gcvNULL)
            {
                gctUINT aStride2;
                gctUINT lineSize2;

                if (nPlane != 3)
                {
                    gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
                }

                aStride2 = (surf->aWidth/2) * bpp[2];
                lineSize2 = width[2] * bpp[2];

                gcmONERROR(gcoOS_Open(NULL, TableName2, gcvFILE_READ, &file));

                /* Copy tile status data */
                for (n = 0; n < height[1]; n++)
                {
                    /* Fill plane 2. */
                    gctUINT8_PTR p = (gctUINT8_PTR)surf->tileStatuslogicalEx[swap ? 0 : 1] + n * aStride2;

                    gcmONERROR(gcoOS_Read(
                                gcvNULL,
                                file,
                                lineSize1,
                                p,
                                &read
                                ));

                    if (GalIsBigEndian() && (bpp[2] == 2))
                    {
                        gctUINT16 *s = (gctUINT16 *)p;

                        for (i = 0; i < lineSize2; i += 2)
                        {
                                *s = swap_word(*s);
                                s++;
                        }
                    }

                    if (read != lineSize2)
                            break;
                }

                gcmONERROR(gcoOS_Close(gcvNULL, file));

                gcmONERROR(gcoOS_CacheFlush(
                        gcvNULL,
                        surf->tileStatusNodeEx[1].node,
                        surf->tileStatusNodeEx[1].memory,
                        surf->tileStatusNodeEx[1].size));
            }
        }
    }

    *TSurf = surf;

    return gcvSTATUS_OK;


OnError:
    if (surf != gcvNULL)
    {
        GalDeleteTSurf(gcvNULL, surf);
    }

    if (file)
    {
        gcmVERIFY_OK(gcoOS_Close(gcvNULL, file));
    }

    return status;
}


gceSTATUS CDECL GalLoadDECTPCRawToTSurf(
    IN gctCONST_STRING DataName0,
    IN gctCONST_STRING DataName1,
    IN gctCONST_STRING DataName2,
    IN gctCONST_STRING TableName0,
    IN gctCONST_STRING TableName1,
    IN gctCONST_STRING TableName2,
    IN gctUINT32 ImageWidth,
    IN gctUINT32 ImageHeight,
    IN gceTILING Tiling,
    IN gceSURF_FORMAT Format,
    IN gctBOOL Compressed,
    OUT T2D_SURF_PTR *TSurf
    )
{
    return GalLoadSpecialRawToTSurfWithPool(
        DataName0, DataName1, DataName2,
        TableName0, TableName1, TableName2,
        ImageWidth, ImageHeight,
        Tiling, Format,
        Compressed ? gcv2D_TSC_DEC_TPC_TILED_COMPRESSED : gcv2D_TSC_DEC_TPC_TILED,
        gcvPOOL_DEFAULT,
        TSurf);
}

gceSTATUS CDECL GalLoadVMSAARawToTSurf(
    IN gctCONST_STRING DataName,
    IN gctCONST_STRING TileStatusName,
    IN gctUINT32 ImageWidth,
    IN gctUINT32 ImageHeight,
    IN gceTILING Tiling,
    IN gceSURF_FORMAT Format,
    IN gce2D_TILE_STATUS_CONFIG TileStatusConfig,
    OUT T2D_SURF_PTR *TSurf
    )
{
    gceSTATUS status;

    status = GalLoadSpecialRawToTSurfWithPool(
        DataName, gcvNULL, gcvNULL,
        TileStatusName, gcvNULL, gcvNULL,
        ImageWidth, ImageHeight,
        Tiling, Format,
        TileStatusConfig,
        gcvPOOL_DEFAULT,
        TSurf);

    (*TSurf)->tileStatusClear = 0xDEADDEAD;

    return status;
}

#define POOL_SIZE 1024

T2DArea * CDECL
GalAllocateArea(
    IN T2DAreaContext * Context,
    IN T2DArea * Slibing,
    IN gcsRECT * Rect,
    IN gctUINT32 Owner
    )
{
    T2DArea * area;
    T2DAreaPool * pool  = Context->areaPool;

    if (pool == NULL)
    {
        /* First pool. */
        pool = (T2DAreaPool *) malloc(sizeof (T2DAreaPool));
        Context->areaPool = pool;

        /* Clear fields. */
        pool->areas     = NULL;
        pool->freeNodes = NULL;
        pool->next      = NULL;
    }

    for (;;)
    {
        if (pool->areas == NULL)
        {
            /* No areas allocated, allocate now. */
            pool->areas = (T2DArea *) malloc(sizeof (T2DArea) * POOL_SIZE);

            /* Get area. */
            area = pool->areas;

            /* Update freeNodes. */
            pool->freeNodes = area + 1;

            break;
        }

        else if (pool->freeNodes - pool->areas >= POOL_SIZE)
        {
            /* This pool is full. */
            if (pool->next == NULL)
            {
                /* No more pools, allocate one. */
                pool->next = (T2DAreaPool *) malloc(sizeof (T2DAreaPool));

                /* Point to the new pool. */
                pool = pool->next;

                /* Clear fields. */
                pool->areas     = NULL;
                pool->freeNodes = NULL;
                pool->next      = NULL;
            }

            else
            {
                /* Advance to next pool. */
                pool = pool->next;
            }
        }

        else
        {
            /* Get area and update freeNodes. */
            area = pool->freeNodes++;

            break;
        }
    }

    /* Update area fields. */
    area->rect   = *Rect;
    area->owners = Owner;

    if (Slibing == NULL)
    {
        area->next = NULL;
    }

    else
    {
        area->next = Slibing->next;
        Slibing->next = area;
    }

    return area;
}

void CDECL
GalSplitArea(
    IN T2DAreaContext * Context,
    IN T2DArea * Area,
    IN gcsRECT * Rect,
    IN gctUINT32 Owner
    )
{
    gcsRECT r0[4];
    gcsRECT r1[4];
    gctUINT32 c0 = 0;
    gctUINT32 c1 = 0;
    gctUINT32 i;

    gcsRECT * rect;

    for (;;)
    {
        rect = &Area->rect;

        if ((Rect->left   < rect->right)
        &&  (Rect->top    < rect->bottom)
        &&  (Rect->right  > rect->left)
        &&  (Rect->bottom > rect->top)
        )
        {
            /* Overlapped. */
            break;
        }

        if (Area->next == NULL)
        {
            /* This rectangle is not overlapped with any area. */
            GalAllocateArea(Context, Area, Rect, Owner);
            return;
        }

        Area = Area->next;
    }

    /* OK, the rectangle is overlapped with 'rect' area. */
    if ((Rect->left <= rect->left)
    &&  (Rect->right >= rect->right)
    )
    {
        /* |-><-| */
        /* +---+---+---+
         * | X | X | X |
         * +---+---+---+
         * | X | X | X |
         * +---+---+---+
         * | X | X | X |
         * +---+---+---+
         */

        if (Rect->left < rect->left)
        {
            r1[c1].left   = Rect->left;
            r1[c1].top    = Rect->top;
            r1[c1].right  = rect->left;
            r1[c1].bottom = Rect->bottom;

            c1++;
        }

        if (Rect->top < rect->top)
        {
            r1[c1].left   = rect->left;
            r1[c1].top    = Rect->top;
            r1[c1].right  = rect->right;
            r1[c1].bottom = rect->top;

            c1++;
        }

        else if (rect->top < Rect->top)
        {
            r0[c0].left   = rect->left;
            r0[c0].top    = rect->top;
            r0[c0].right  = rect->right;
            r0[c0].bottom = Rect->top;

            c0++;
        }

        if (Rect->right > rect->right)
        {
            r1[c1].left   = rect->right;
            r1[c1].top    = Rect->top;
            r1[c1].right  = Rect->right;
            r1[c1].bottom = Rect->bottom;

            c1++;
        }

        if (Rect->bottom > rect->bottom)
        {
            r1[c1].left   = rect->left;
            r1[c1].top    = rect->bottom;
            r1[c1].right  = rect->right;
            r1[c1].bottom = Rect->bottom;

            c1++;
        }

        else if (rect->bottom > Rect->bottom)
        {
            r0[c0].left   = rect->left;
            r0[c0].top    = Rect->bottom;
            r0[c0].right  = rect->right;
            r0[c0].bottom = rect->bottom;

            c0++;
        }
    }

    else if (Rect->left <= rect->left)
    {
        /* |-> */
        /* +---+---+---+
         * | X | X |   |
         * +---+---+---+
         * | X | X |   |
         * +---+---+---+
         * | X | X |   |
         * +---+---+---+
         */

        if (Rect->left < rect->left)
        {
            r1[c1].left   = Rect->left;
            r1[c1].top    = Rect->top;
            r1[c1].right  = rect->left;
            r1[c1].bottom = Rect->bottom;

            c1++;
        }

        if (Rect->top < rect->top)
        {
            r1[c1].left   = rect->left;
            r1[c1].top    = Rect->top;
            r1[c1].right  = Rect->right;
            r1[c1].bottom = rect->top;

            c1++;
        }

        else if (rect->top < Rect->top)
        {
            r0[c0].left   = rect->left;
            r0[c0].top    = rect->top;
            r0[c0].right  = Rect->right;
            r0[c0].bottom = Rect->top;

            c0++;
        }

        /* if (rect->right > Rect->right) */
        {
            r0[c0].left   = Rect->right;
            r0[c0].top    = rect->top;
            r0[c0].right  = rect->right;
            r0[c0].bottom = rect->bottom;

            c0++;
        }

        if (Rect->bottom > rect->bottom)
        {
            r1[c1].left   = rect->left;
            r1[c1].top    = rect->bottom;
            r1[c1].right  = Rect->right;
            r1[c1].bottom = Rect->bottom;

            c1++;
        }

        else if (rect->bottom > Rect->bottom)
        {
            r0[c0].left   = rect->left;
            r0[c0].top    = Rect->bottom;
            r0[c0].right  = Rect->right;
            r0[c0].bottom = rect->bottom;

            c0++;
        }
    }

    else if (Rect->right >= rect->right)
    {
        /*    <-| */
        /* +---+---+---+
         * |   | X | X |
         * +---+---+---+
         * |   | X | X |
         * +---+---+---+
         * |   | X | X |
         * +---+---+---+
         */

        /* if (rect->left < Rect->left) */
        {
            r0[c0].left   = rect->left;
            r0[c0].top    = rect->top;
            r0[c0].right  = Rect->left;
            r0[c0].bottom = rect->bottom;

            c0++;
        }

        if (Rect->top < rect->top)
        {
            r1[c1].left   = Rect->left;
            r1[c1].top    = Rect->top;
            r1[c1].right  = rect->right;
            r1[c1].bottom = rect->top;

            c1++;
        }

        else if (rect->top < Rect->top)
        {
            r0[c0].left   = Rect->left;
            r0[c0].top    = rect->top;
            r0[c0].right  = rect->right;
            r0[c0].bottom = Rect->top;

            c0++;
        }

        if (Rect->right > rect->right)
        {
            r1[c1].left   = rect->right;
            r1[c1].top    = Rect->top;
            r1[c1].right  = Rect->right;
            r1[c1].bottom = Rect->bottom;

            c1++;
        }

        if (Rect->bottom > rect->bottom)
        {
            r1[c1].left   = Rect->left;
            r1[c1].top    = rect->bottom;
            r1[c1].right  = rect->right;
            r1[c1].bottom = Rect->bottom;

            c1++;
        }

        else if (rect->bottom > Rect->bottom)
        {
            r0[c0].left   = Rect->left;
            r0[c0].top    = Rect->bottom;
            r0[c0].right  = rect->right;
            r0[c0].bottom = rect->bottom;

            c0++;
        }
    }

    else
    {
        /* | */
        /* +---+---+---+
         * |   | X |   |
         * +---+---+---+
         * |   | X |   |
         * +---+---+---+
         * |   | X |   |
         * +---+---+---+
         */

        /* if (rect->left < Rect->left) */
        {
            r0[c0].left   = rect->left;
            r0[c0].top    = rect->top;
            r0[c0].right  = Rect->left;
            r0[c0].bottom = rect->bottom;

            c0++;
        }

        if (Rect->top < rect->top)
        {
            r1[c1].left   = Rect->left;
            r1[c1].top    = Rect->top;
            r1[c1].right  = Rect->right;
            r1[c1].bottom = rect->top;

            c1++;
        }

        else if (rect->top < Rect->top)
        {
            r0[c0].left   = Rect->left;
            r0[c0].top    = rect->top;
            r0[c0].right  = Rect->right;
            r0[c0].bottom = Rect->top;

            c0++;
        }

        /* if (rect->right > Rect->right) */
        {
            r0[c0].left   = Rect->right;
            r0[c0].top    = rect->top;
            r0[c0].right  = rect->right;
            r0[c0].bottom = rect->bottom;

            c0++;
        }

        if (Rect->bottom > rect->bottom)
        {
            r1[c1].left   = Rect->left;
            r1[c1].top    = rect->bottom;
            r1[c1].right  = Rect->right;
            r1[c1].bottom = Rect->bottom;

            c1++;
        }

        else if (rect->bottom > Rect->bottom)
        {
            r0[c0].left   = Rect->left;
            r0[c0].top    = Rect->bottom;
            r0[c0].right  = Rect->right;
            r0[c0].bottom = rect->bottom;

            c0++;
        }
    }

    if (c1 > 0)
    {
        /* Process rects outside area. */
        if (Area->next == NULL)
        {
            /* Save rects outside area. */
            for (i = 0; i < c1; i++)
            {
                GalAllocateArea(Context, Area, &r1[i], Owner);
            }
        }

        else
        {
            /* Rects outside area. */
            for (i = 0; i < c1; i++)
            {
                GalSplitArea(Context, Area, &r1[i], Owner);
            }
        }
    }

    if (c0 > 0)
    {
        /* Save rects inside area but not overlapped. */
        for (i = 0; i < c0; i++)
        {
            GalAllocateArea(Context, Area, &r0[i], Area->owners);
        }

        /* Update overlapped area. */
        if (rect->left   < Rect->left)   { rect->left   = Rect->left;   }
        if (rect->top    < Rect->top)    { rect->top    = Rect->top;    }
        if (rect->right  > Rect->right)  { rect->right  = Rect->right;  }
        if (rect->bottom > Rect->bottom) { rect->bottom = Rect->bottom; }
    }

    /* The area is owned by the new owner as well. */
    Area->owners |= Owner;
}

void CDECL GalFreeArea(
    IN T2DAreaContext *Context
    )
{
    T2DAreaPool *pPool, *pNext;

    pPool = Context->areaPool;

    while (pPool != gcvNULL)
    {
        pNext = pPool->next;

        if (pPool->areas != gcvNULL)
        {
            free(pPool->areas);
        }

        free(pPool);

        pPool = pNext;
    }
}

void CDECL
GalFillAlphaBySW(
    IN gctPOINTER       VirtAddr,
    IN gctUINT32        Width,
    IN gctUINT32        Height,
    IN gctINT           Stride,
    IN gceSURF_ROTATION Rotation,
    IN gcsRECT *        Rect,
    IN gcsRECT *        ClipRect,
    IN gctUINT8         Alpha,
    IN gctUINT8         InitAlpha
    )
{
    gctUINT32_PTR pos, start = (gctUINT32_PTR)VirtAddr;
    gctUINT i, j, w, h, x, y;

    // TODO: fix later
    if (ClipRect &&
        (ClipRect->left != Rect->left ||
         ClipRect->top != Rect->top ||
         ClipRect->right != Rect->right ||
         ClipRect->bottom != Rect->bottom))
    {
        for (i = 0; i < (gctUINT)(ClipRect->bottom - ClipRect->top); i++)
        {
            pos = (gctUINT32_PTR)((gctUINT8_PTR)start + i * Stride + ClipRect->left * 4);
            for (j = 0; j < (gctUINT)(ClipRect->right - ClipRect->left); j++, pos++)
            {
                *pos = *pos | COLOR_ARGB8(InitAlpha, 0x0, 0x0, 0x0);
            }
        }
    }

    switch(Rotation)
    {
        case gcvSURF_0_DEGREE:
            w = Rect->right - Rect->left;
            h = Rect->bottom - Rect->top;
            x = Rect->left;
            y = Rect->top;
            break;

        case gcvSURF_90_DEGREE:
            w = Rect->bottom - Rect->top;
            h = Rect->right - Rect->left;
            x = Height - Rect->bottom;
            y = Rect->left;
            break;

        case gcvSURF_180_DEGREE:
            w = Rect->right - Rect->left;
            h = Rect->bottom - Rect->top;
            x = Width - Rect->left - w;
            y = Height - Rect->top - h;
            break;

        case gcvSURF_270_DEGREE:
            w = Rect->bottom - Rect->top;
            h = Rect->right - Rect->left;
            x = Rect->top;
            y = Height - Rect->left - h;
            break;

        case gcvSURF_FLIP_X:
            w = Rect->right - Rect->left;
            h = Rect->bottom - Rect->top;
            x = Width - Rect->left - w;
            y = Rect->top;
            break;

        case gcvSURF_FLIP_Y:
            w = Rect->right - Rect->left;
            h = Rect->bottom - Rect->top;
            x = Rect->left;
            y = Height - Rect->top - h;
            break;
    }

    for (i = 0; i < h; i++)
    {
        pos = (gctUINT32_PTR)((gctUINT8_PTR)start + (y + i) * Stride + x * 4);
        for (j = 0; j < w; j++, pos++)
        {
            *pos = (*pos | 0xFF000000) & COLOR_ARGB8(Alpha, 0xFF, 0xFF, 0xFF);
        }
    }
}

const struct FeatureInfo
{
    const char* name;
    const char* msg;
    gceFEATURE  feature;
    gctBOOL     status;
} c_featureInfos[] =
{
    {"Unknown feature" , "Unknown feature ", gcvFEATURE_PIPE_2D, gcvFALSE},
    {"gcvFEATURE_2D_NO_COLORBRUSH_INDEX8 " , "ColorBursh or index8", gcvFEATURE_2D_NO_COLORBRUSH_INDEX8, gcvTRUE},
    {"gcvFEATURE_ANDROID_ONLY " , "Android only feature", gcvFEATURE_ANDROID_ONLY, gcvTRUE},
    {"gcvFEATURE_2DPE20 " , "PE2.0 and related function", gcvFEATURE_2DPE20, gcvFALSE},
    {"gcvFEATURE_2D_BITBLIT_FULLROTATION " , "Flip_Y or Full rotation", gcvFEATURE_2D_BITBLIT_FULLROTATION, gcvFALSE},
    {"gcvFEATURE_2D_DITHER " , "Dither", gcvFEATURE_2D_DITHER, gcvFALSE},
    {"gcvFEATURE_YUV420_SCALER " , "YUV420 scaler", gcvFEATURE_YUV420_SCALER, gcvFALSE},
    {"gcvFEATURE_2D_OPF_YUV_OUTPUT " , "YUV output of opf", gcvFEATURE_2D_OPF_YUV_OUTPUT, gcvFALSE},
    {"gcvFEATURE_2D_TILING " , "Tilling", gcvFEATURE_2D_TILING, gcvFALSE},
    {"gcvFEATURE_2D_FILTERBLIT_FULLROTATION " , "Filterblit full rotation", gcvFEATURE_2D_FILTERBLIT_FULLROTATION, gcvFALSE},
    {"gcvFEATURE_2D_FILTERBLIT_A8_ALPHA " , "Filterblit with A8 alphablend", gcvFEATURE_2D_FILTERBLIT_A8_ALPHA, gcvFALSE},
    {"gcvFEATURE_2D_FILTERBLIT_PLUS_ALPHABLEND " , "Filter with alphablend", gcvFEATURE_2D_FILTERBLIT_PLUS_ALPHABLEND, gcvFALSE},
    {"gcvFEATURE_SCALER " , "2D two pass filterblit", gcvFEATURE_SCALER, gcvFALSE},
    {"gcvFEATURE_2D_ONE_PASS_FILTER " , "2D one pass filterblit", gcvFEATURE_2D_ONE_PASS_FILTER, gcvFALSE},
    {"gcvFEATURE_2D_ONE_PASS_FILTER_TAP " , "2D one pass filterblit with 7/9 tap", gcvFEATURE_2D_ONE_PASS_FILTER_TAP, gcvFALSE},
    {"gcvFEATURE_2D_MULTI_SOURCE_BLT " , "2D MultisrcBlit v1 or some YUV output", gcvFEATURE_2D_MULTI_SOURCE_BLT, gcvFALSE},
    {"gcvFEATURE_2D_MULTI_SOURCE_BLT_EX " , "2D MultisrcBlit v1.5 (8 source)", gcvFEATURE_2D_MULTI_SOURCE_BLT_EX, gcvFALSE},
    {"gcvFEATURE_2D_MULTI_SRC_BLT_TO_UNIFIED_DST_RECT " , "2D MultiSourceBlit v1.5 (Dest Rect)", gcvFEATURE_2D_MULTI_SRC_BLT_TO_UNIFIED_DST_RECT, gcvFALSE},
    {"gcvFEATURE_2D_MULTI_SOURCE_BLT_EX2 " , "2D MultisrcBlit v2", gcvFEATURE_2D_MULTI_SOURCE_BLT_EX2, gcvFALSE},
    {"gcvFEATURE_2D_YUV_BLIT " , "YUV blit", gcvFEATURE_2D_YUV_BLIT, gcvFALSE},
    {"gcvFEATURE_2D_A8_TARGET " , "Format A8 output", gcvFEATURE_2D_A8_TARGET, gcvFALSE},
    {"gcvFEATURE_SEPARATE_SRC_DST " , "Separate src and dst address", gcvFEATURE_SEPARATE_SRC_DST, gcvTRUE},
    {"gcvFEATURE_2D_COMPRESSION " , "2D COMPRESSION", gcvFEATURE_2D_COMPRESSION, gcvFALSE},
    {"gcvFEATURE_2D_YUV_SEPARATE_STRIDE " , "Seperate U/V stride", gcvFEATURE_2D_YUV_SEPARATE_STRIDE, gcvFALSE},
    {"gcvFEATURE_2D_COLOR_SPACE_CONVERSION " , "Color space conversion", gcvFEATURE_2D_COLOR_SPACE_CONVERSION, gcvFALSE},
    {"gcvFEATURE_NO_USER_CSC " , "User defined color space conversion", gcvFEATURE_NO_USER_CSC, gcvTRUE},
    {"gcvFEATURE_2D_TILING " , "2D tiling", gcvFEATURE_2D_TILING, gcvFALSE},
    {"gcvFEATURE_2D_SUPER_TILE_V1 " , "2D super tiling v1", gcvFEATURE_2D_SUPER_TILE_V1, gcvFALSE},
    {"gcvFEATURE_2D_SUPER_TILE_V2 " , "2D super tiling v2", gcvFEATURE_2D_SUPER_TILE_V2, gcvFALSE},
    {"gcvFEATURE_2D_SUPER_TILE_V3 " , "2D super tiling v3", gcvFEATURE_2D_SUPER_TILE_V3, gcvFALSE},
    {"gcvFEATURE_2D_MINOR_TILING " , "2D minor tiling", gcvFEATURE_2D_MINOR_TILING, gcvFALSE},
    {"gcvFEATURE_2D_SUPER_TILE_VERSION " , "2D super tile version", gcvFEATURE_2D_SUPER_TILE_VERSION, gcvFALSE},
    {"gcvFEATURE_2D_GAMMA " , "Gamma", gcvFEATURE_2D_GAMMA, gcvFALSE},
    {"gcvFEATURE_2D_A8_NO_ALPHA " , "A8 alphablend", gcvFEATURE_2D_A8_NO_ALPHA, gcvTRUE},
    {"gcvFEATURE_2D_FC_SOURCE " , "FC surface", gcvFEATURE_2D_FC_SOURCE, gcvFALSE},
    {"gcvFEATURE_TPC_COMPRESSION " , "TPC compression", gcvFEATURE_TPC_COMPRESSION, gcvFALSE},
    {"gcvFEATURE_DEC_COMPRESSION " , "DEC300 compression", gcvFEATURE_DEC_COMPRESSION, gcvFALSE},
    {"gcvFEATURE_DEC_TPC_COMPRESSION " , "DEC300 TPC compression", gcvFEATURE_DEC_TPC_COMPRESSION, gcvFALSE},
    {"gcvFEATURE_DEC_COMPRESSION_TILE_NV12_8BIT " , "DEC300 TPC compression 8bit", gcvFEATURE_DEC_COMPRESSION_TILE_NV12_8BIT, gcvFALSE},
    {"gcvFEATURE_DEC_COMPRESSION_TILE_NV12_10BIT " , "DEC300 TPC compression 10bit", gcvFEATURE_DEC_COMPRESSION_TILE_NV12_10BIT, gcvFALSE},
    {"gcvFEATURE_FULL_DIRECTFB " , "DFB dest color key mode", gcvFEATURE_FULL_DIRECTFB, gcvFALSE},
    {"gcvFEATURE_2D_MIRROR_EXTENSION " , "2D mirror extension", gcvFEATURE_2D_MIRROR_EXTENSION, gcvFALSE},
    {"gcvFEATURE_2D_PIXEL_ALIGNMENT " , "Alignment improvement", gcvFEATURE_2D_PIXEL_ALIGNMENT, gcvFALSE},
    {"gcvFEATURE_2D_POST_FLIP " , "2D post flip", gcvFEATURE_2D_POST_FLIP, gcvFALSE},
    {"gcvFEATURE_2D_ROTATION_STALL_FIX " , "Fix rotation stall", gcvFEATURE_2D_ROTATION_STALL_FIX, gcvFALSE},
    {"gcvFEATURE_2D_YUV_MODE " , "YUV standard", gcvFEATURE_2D_YUV_MODE, gcvFALSE},
    {"gcvFEATURE_2D_MULTI_SRC_BLT_BILINEAR_FILTER " , "MultiSrc bilinear filter", gcvFEATURE_2D_MULTI_SRC_BLT_BILINEAR_FILTER, gcvFALSE},
    {"gcvFEATURE_2D_MAJOR_SUPER_TILE " , "supertile output and ymajor supertile", gcvFEATURE_2D_MAJOR_SUPER_TILE, gcvFALSE},
    {"gcvFEATURE_2D_V4COMPRESSION " , "2D V4 compression", gcvFEATURE_2D_V4COMPRESSION, gcvFALSE},
    {"gcvFEATURE_2D_VMSAA " , "2D VMSAA", gcvFEATURE_2D_VMSAA, gcvFALSE},
    {"gcvFEATURE_2D_10BIT_OUTPUT_LINEAR " , "2D 10bit output", gcvFEATURE_2D_10BIT_OUTPUT_LINEAR, gcvFALSE},
    {"gcvFEATURE_2D_YUV420_OUTPUT_LINEAR " , "2D YUV420 output", gcvFEATURE_2D_YUV420_OUTPUT_LINEAR, gcvFALSE},
    {"gcvFEATURE_TPCV11_COMPRESSION " , "TPCv11 compression", gcvFEATURE_TPCV11_COMPRESSION, gcvFALSE},
    {"gcvFEATURE_DEC400_COMPRESSION " , "DEC400 compression", gcvFEATURE_DEC400_COMPRESSION, gcvFALSE},
    {"gcvFEATURE_2D_FAST_CLEAR " , "2D tilestatus Fast Clear", gcvFEATURE_2D_FAST_CLEAR, gcvFALSE},
};

gceSTATUS GalQueryFeatureStr(
    gceFEATURE feature,
    char* name,
    char* message,
    gctBOOL* status)
{
    gctUINT32 i;
    gctBOOL found = gcvFALSE;

    if (name == gcvNULL || message == gcvNULL || status == gcvNULL)
        return gcvSTATUS_INVALID_ARGUMENT;

    for (i = 0; i < sizeof(c_featureInfos) / sizeof(c_featureInfos[0]); i++)
    {
        if (c_featureInfos[i].feature == feature) {
            memcpy(name, c_featureInfos[i].name, strlen(c_featureInfos[i].name)+1);
            memcpy(message, c_featureInfos[i].msg, strlen(c_featureInfos[i].msg)+1);
            *status = c_featureInfos[i].status;
            found = gcvTRUE;
            break;
        }
    }

    if (!found)
    {
        memcpy(name, c_featureInfos[0].name, strlen(c_featureInfos[0].name)+1);
        memcpy(message, c_featureInfos[0].msg, strlen(c_featureInfos[0].msg)+1);
        *status = gcvFALSE;
    }

    return gcvSTATUS_OK;
}

gceSTATUS GalQueryFeatureByIndex(
    gctUINT32 index,
    gceFEATURE* feature,
    char* name,
    char* message,
    gctBOOL* status)
{
    if (index > 0 && index < gcmCOUNTOF(c_featureInfos))
    {
        if (feature != gcvNULL)
            *feature = c_featureInfos[index].feature;

        if (name != gcvNULL)
            memcpy(name, c_featureInfos[index].name, strlen(c_featureInfos[index].name)+1);

        if (message != gcvNULL)
            memcpy(message, c_featureInfos[index].msg, strlen(c_featureInfos[index].msg)+1);

        if (status != gcvNULL)
            *status = c_featureInfos[index].status;

        return gcvSTATUS_OK;
    }

    return gcvSTATUS_INVALID_ARGUMENT;
}

