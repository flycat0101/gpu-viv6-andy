/****************************************************************************
*
*    Copyright 2012 - 2020 Vivante Corporation, Santa Clara, California.
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




#ifndef __bitmap_h_
#define __bitmap_h_

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************\
******************************* gcoBITMAP Object *******************************
\******************************************************************************/

typedef struct _gcoBITMAP * gcoBITMAP;

/* Construct bitmap object. */
gceSTATUS
gcoBITMAP_Construct(
    IN gcoOS Os,
    IN OUT gcoBITMAP * Bitmap
    );

/* Destroy bitmap object. */
gceSTATUS
gcoBITMAP_Destroy(
    IN gcoBITMAP Bitmap
    );

/* Free resources held by bitmap object. */
gceSTATUS
gcoBITMAP_Free(
    IN gcoBITMAP Bitmap
    );

/* Load a bitmap file. */
gceSTATUS
gcoBITMAP_Load(
    IN gcoBITMAP Bitmap,
    IN char * FileName
    );

/* Return the size of the bitmap. */
gceSTATUS
gcoBITMAP_GetSize(
    IN gcoBITMAP Bitmap,
    IN OUT gctUINT32 * Width,
    IN OUT gctUINT32 * Height
    );

/* Upload the bitmap to the specified destination. */
gceSTATUS
gcoBITMAP_Upload(
    IN gcoBITMAP Bitmap,
    IN gctPOINTER Destination
    );

/* Save the bitmap to a file. */
gceSTATUS
gcoBITMAP_Save(
    IN gctPOINTER source,
    IN gctUINT stride,
    IN gctINT left,
    IN gctINT top,
    IN gctINT right,
    IN gctINT bottom,
    IN char * fileName
    );

/* Load bitmap into a new surface. */
gceSTATUS
gcoBITMAP_LoadSurface(
    IN gcoHAL Hal,
    IN gcoOS Os,
    IN char * FileName,
    IN OUT gcoSURF * Surface
    );

/* Save surface into a bitmap. */
gceSTATUS
gcoBITMAP_SaveSurface(
    IN gcoSURF Surface,
    IN char * FileName
    );


#ifdef __cplusplus
}
#endif

#endif
