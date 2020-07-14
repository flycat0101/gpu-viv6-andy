/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_vsc_utils_io_h_
#define __gc_vsc_utils_io_h_

BEGIN_EXTERN_C()

typedef struct _VSC_IO_BUFFER
{
    gctUINT              curPos;           /* current position in the buffer */
    gctUINT              allocatedBytes;   /* size of buff allocated */
    gctCHAR *            buffer;
} VSC_IO_BUFFER;

VSC_ErrCode
VSC_IO_AllocateMem(
    gctUINT Sz,
    void ** Ptr
    );

VSC_ErrCode
VSC_IO_ReallocateMem(
    VSC_IO_BUFFER       * Buf,
    gctUINT               Sz
    );

VSC_ErrCode
VSC_IO_Init(
    VSC_IO_BUFFER       * Buf,
    gctUINT               Sz
    );

void
VSC_IO_Finalize(
    VSC_IO_BUFFER       * Buf
    );

VSC_ErrCode
VSC_IO_writeInt(VSC_IO_BUFFER *Buf, gctINT Val);

VSC_ErrCode
VSC_IO_writeUint(VSC_IO_BUFFER *Buf, gctUINT Val);

VSC_ErrCode
VSC_IO_writeShort(VSC_IO_BUFFER *Buf, gctINT16 Val);

VSC_ErrCode
VSC_IO_writeUshort(VSC_IO_BUFFER *Buf, gctUINT16 Val);

VSC_ErrCode
VSC_IO_writeLong(VSC_IO_BUFFER *Buf, gctUINT64 Val);

VSC_ErrCode
VSC_IO_writeFloat(VSC_IO_BUFFER *Buf, gctFLOAT Val);

VSC_ErrCode
VSC_IO_writeChar(VSC_IO_BUFFER *Buf, gctCHAR Val);

VSC_ErrCode
VSC_IO_writeBlock(VSC_IO_BUFFER *Buf, gctCHAR *Val, gctUINT Sz);

VSC_ErrCode
VSC_IO_readInt(VSC_IO_BUFFER *Buf, gctINT * Val);

VSC_ErrCode
VSC_IO_readUint(VSC_IO_BUFFER *Buf, gctUINT * Val);

VSC_ErrCode
VSC_IO_readShort(VSC_IO_BUFFER *Buf, gctINT16 * Val);

VSC_ErrCode
VSC_IO_readUshort(VSC_IO_BUFFER *Buf, gctUINT16 * Val);

VSC_ErrCode
VSC_IO_readLong(VSC_IO_BUFFER *Buf, gctUINT64 * Val);

VSC_ErrCode
VSC_IO_readFloat(VSC_IO_BUFFER *Buf, gctFLOAT * Val);

VSC_ErrCode
VSC_IO_readChar(VSC_IO_BUFFER *Buf, gctCHAR * Val);

VSC_ErrCode
VSC_IO_readBlock(VSC_IO_BUFFER *Buf, gctCHAR *Val, gctUINT Sz);

END_EXTERN_C()

#endif /* __gc_vsc_utils_io_h_ */

