/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_vsc.h"

VSC_ErrCode
VSC_IO_AllocateMem(
    gctUINT Sz,
    void ** Ptr
    )
{
    /* now we use system allocator to allocate memory */
    gctPOINTER pointer;
    gceSTATUS status;

    /* Allocate a new buffer to store the names */
    status = gcoOS_Allocate(gcvNULL,
                            gcmSIZEOF(gctCHAR) * Sz,
                            &pointer);
    if (gcmIS_ERROR(status)) {
        /* Error. */
        gcmFATAL("VSC_IO_AllocateMem: gcoOS_Allocate failed status=%d(%s)", status, gcoOS_DebugStatus2Name(status));
        return VSC_ERR_OUT_OF_MEMORY;
    }
    *Ptr  = pointer;
    return VSC_ERR_NONE;
}

VSC_ErrCode
VSC_IO_ReallocateMem(
    VSC_IO_BUFFER       * Buf,
    gctUINT               Sz
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    if (Sz > Buf->allocatedBytes)
    {
        gctUINT newSz;
        if (Buf->buffer)
        {
            /* no enough space, need to grow the buffer */
            gctPOINTER pointer;
            newSz = (gctUINT)(((Sz > 1) ? Sz : 2) * 1.6);
            errCode = VSC_IO_AllocateMem(newSz, (void **)&pointer);
            ON_ERROR(errCode, "Failed to reserveBytes %ud", newSz);
            gcoOS_MemCopy(pointer, Buf->buffer, Buf->curPos);
            gcmOS_SAFE_FREE(gcvNULL, Buf->buffer);
            Buf->buffer = (gctCHAR *)pointer;
        }
        else
        {
            newSz = (Sz + 16);
        }
        Buf->allocatedBytes = newSz;
    }
OnError:
    return errCode;
}

VSC_ErrCode
VSC_IO_Init(VSC_IO_BUFFER *Buf, gctUINT Sz)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    errCode = VSC_IO_AllocateMem(Sz, (void **)&Buf->buffer);
    Buf->allocatedBytes = Sz;
    Buf->curPos = 0;

    return errCode;
}

void
VSC_IO_Finalize(VSC_IO_BUFFER *Buf)
{
    if (Buf->buffer)
    {
        gcmOS_SAFE_FREE(gcvNULL, Buf->buffer);
    }
    Buf->allocatedBytes = 0;
    Buf->curPos = 0;
}

#define VSC_IO_ReserveBytes(Buf, Size) (((Buf)->curPos + Size > (Buf)->allocatedBytes) ?  \
                                        VSC_IO_ReallocateMem(Buf, Buf->curPos + Size) : VSC_ERR_NONE)

#define VSC_IO_CheckBounds(Buf, Size) (((Buf)->curPos + Size > (Buf)->allocatedBytes) ?  \
                                        VSC_ERR_OUT_OF_BOUNDS : VSC_ERR_NONE)


#define VSC_IO_WRITEBYTE(Buf, Byte)                             \
    do {                                                        \
        if ((Buf)->buffer)                                      \
        {                                                       \
            (Buf)->buffer[(Buf)->curPos++] = (Byte);            \
        }                                                       \
        else                                                    \
        {                                                       \
            (Buf)->curPos++;                                    \
        }                                                       \
    } while(0)

VSC_ErrCode
VSC_IO_writeInt(VSC_IO_BUFFER *Buf, gctINT Val)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VSC_IO_ReserveBytes(Buf, sizeof(gctINT));
    if (errCode == VSC_ERR_NONE)
    {
        gctINT i;
        for (i = 0; i < sizeof(gctINT); i++)
        {
            VSC_IO_WRITEBYTE(Buf, ((gctCHAR*)&Val)[i]);
        }
    }
    return errCode;
}

VSC_ErrCode
VSC_IO_writeUint(VSC_IO_BUFFER *Buf, gctUINT Val)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VSC_IO_ReserveBytes(Buf, sizeof(gctUINT));
    if (errCode == VSC_ERR_NONE)
    {
        gctINT i;
        for (i = 0; i < sizeof(gctUINT); i++)
        {
            VSC_IO_WRITEBYTE(Buf, ((gctCHAR*)&Val)[i]);
        }
    }
    return errCode;
}

VSC_ErrCode
VSC_IO_writeShort(VSC_IO_BUFFER *Buf, gctINT16 Val)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VSC_IO_ReserveBytes(Buf, sizeof(gctINT16));
    if (errCode == VSC_ERR_NONE)
    {
        gctINT i;
        for (i = 0; i < sizeof(gctINT16); i++)
        {
            VSC_IO_WRITEBYTE(Buf, ((gctCHAR*)&Val)[i]);
        }
    }
    return errCode;
}

VSC_ErrCode
VSC_IO_writeUshort(VSC_IO_BUFFER *Buf, gctUINT16 Val)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VSC_IO_ReserveBytes(Buf, sizeof(gctUINT16));
    if (errCode == VSC_ERR_NONE)
    {
        gctINT i;
        for (i = 0; i < sizeof(gctUINT16); i++)
        {
            VSC_IO_WRITEBYTE(Buf, ((gctCHAR*)&Val)[i]);
        }
    }
    return errCode;
}

VSC_ErrCode
VSC_IO_writeLong(VSC_IO_BUFFER *Buf, gctUINT64 Val)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VSC_IO_ReserveBytes(Buf, sizeof(gctUINT64));
    if (errCode == VSC_ERR_NONE)
    {
        gctINT i;
        for (i = 0; i < sizeof(gctUINT64); i++)
        {
            VSC_IO_WRITEBYTE(Buf, ((gctCHAR*)&Val)[i]);
        }
    }
    return errCode;
}

VSC_ErrCode
VSC_IO_writeFloat(VSC_IO_BUFFER *Buf, gctFLOAT Val)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VSC_IO_ReserveBytes(Buf, sizeof(gctFLOAT));
    if (errCode == VSC_ERR_NONE)
    {
        gctINT i;
        for (i = 0; i < sizeof(gctFLOAT); i++)
        {
            VSC_IO_WRITEBYTE(Buf, ((gctCHAR*)&Val)[i]);
        }
    }
    return errCode;
}

VSC_ErrCode
VSC_IO_writeChar(VSC_IO_BUFFER *Buf, gctCHAR Val)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VSC_IO_ReserveBytes(Buf, sizeof(Val));
    if (errCode == VSC_ERR_NONE)
    {
            VSC_IO_WRITEBYTE(Buf, Val);
    }
    return errCode;
}

VSC_ErrCode
VSC_IO_writeBlock(VSC_IO_BUFFER *Buf, gctCHAR *Val, gctUINT Sz)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VSC_IO_ReserveBytes(Buf, Sz);
    if (errCode == VSC_ERR_NONE)
    {
        if (Buf->buffer)
        {
            gcoOS_MemCopy(Buf->buffer + Buf->curPos, Val, Sz);
        }
        Buf->curPos += Sz;
    }
    return errCode;
}

VSC_ErrCode
VSC_IO_readInt(VSC_IO_BUFFER *Buf, gctINT * Val)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VSC_IO_CheckBounds(Buf, sizeof(gctINT));
    if (errCode == VSC_ERR_NONE)
    {
        gctINT i;
        for (i = 0; i < sizeof(gctINT); i++)
        {
            ((gctCHAR*)Val)[i] = Buf->buffer[Buf->curPos++];
        }
    }
    return errCode;
}

VSC_ErrCode
VSC_IO_readUint(VSC_IO_BUFFER *Buf, gctUINT * Val)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VSC_IO_CheckBounds(Buf, sizeof(gctUINT));
    if (errCode == VSC_ERR_NONE)
    {
        gctINT i;
        for (i = 0; i < sizeof(gctUINT); i++)
        {
            ((gctCHAR*)Val)[i] = Buf->buffer[Buf->curPos++];
        }
    }
    return errCode;
}

VSC_ErrCode
VSC_IO_readShort(VSC_IO_BUFFER *Buf, gctINT16 * Val)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VSC_IO_CheckBounds(Buf, sizeof(gctINT16));
    if (errCode == VSC_ERR_NONE)
    {
        gctINT i;
        for (i = 0; i < sizeof(gctINT16); i++)
        {
            ((gctCHAR*)Val)[i] = Buf->buffer[Buf->curPos++];
        }
    }
    return errCode;
}

VSC_ErrCode
VSC_IO_readUshort(VSC_IO_BUFFER *Buf, gctUINT16 * Val)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VSC_IO_CheckBounds(Buf, sizeof(gctUINT16));
    if (errCode == VSC_ERR_NONE)
    {
        gctINT i;
        for (i = 0; i < sizeof(gctUINT16); i++)
        {
            ((gctCHAR*)Val)[i] = Buf->buffer[Buf->curPos++];
        }
    }
    return errCode;
}

VSC_ErrCode
VSC_IO_readLong(VSC_IO_BUFFER *Buf, gctUINT64 * Val)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VSC_IO_CheckBounds(Buf, sizeof(gctUINT64));
    if (errCode == VSC_ERR_NONE)
    {
        gctINT i;
        for (i = 0; i < sizeof(gctUINT64); i++)
        {
            ((gctCHAR*)Val)[i] = Buf->buffer[Buf->curPos++];
        }
    }
    return errCode;
}

VSC_ErrCode
VSC_IO_readFloat(VSC_IO_BUFFER *Buf, gctFLOAT * Val)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VSC_IO_CheckBounds(Buf, sizeof(gctFLOAT));
    if (errCode == VSC_ERR_NONE)
    {
        gctINT i;
        for (i = 0; i < sizeof(gctFLOAT); i++)
        {
            ((gctCHAR*)Val)[i] = Buf->buffer[Buf->curPos++];
        }
    }
    return errCode;
}

VSC_ErrCode
VSC_IO_readChar(VSC_IO_BUFFER *Buf, gctCHAR * Val)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VSC_IO_CheckBounds(Buf, sizeof(Val));
    if (errCode == VSC_ERR_NONE)
    {
       *Val = Buf->buffer[Buf->curPos++];
    }
    return errCode;
}

VSC_ErrCode
VSC_IO_readBlock(VSC_IO_BUFFER *Buf, gctCHAR *Val, gctUINT Sz)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VSC_IO_CheckBounds(Buf, Sz);
    if (errCode == VSC_ERR_NONE)
    {
        gcoOS_MemCopy(Val, Buf->buffer + Buf->curPos, Sz);
        Buf->curPos += Sz;
    }
    return errCode;
}

