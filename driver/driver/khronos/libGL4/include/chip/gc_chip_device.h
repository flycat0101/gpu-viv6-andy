/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __chip_device_h__
#define __chip_device_h__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __GLchipGlobalRec
{
    GLuint  processID;
    GLuint  numContext;
    GLbyte                  *__glPFDTable;      /* Table of pixel format */
    LONG                    dPFDSize;           /*Displayable size*/
    LONG                    dPFDSizeNonDisplay; /*Non-Displayable size*/

    /* Desktop information */
    GLuint                  width;
    GLuint                  height;
    GLuint                  bpp;
    GLuint                  stride;
#ifdef _LINUX_
    /* Frame buffer base physical address for on screen */
    GLvoid *logicalAddress;
    GLvoid *basePhyAddress;
    /* Mapped gpu address and info */
    gctUINT32 gpuAddress;
#endif
} __GLchipGlobal;

#ifdef __cplusplus
}
#endif

#endif /* __chip_device_h__ */
