/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __chip_device_h_
#define __chip_device_h_
#ifdef _LINUX_
#else
#include "vvtpfdtypes.h"
#endif
#include "chip_context.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _LINUX_
typedef unsigned char       BYTE;
#endif

typedef struct _glsCONFIGDEPTH {
    GLint depthSize;
    GLint stencilSize;
} glsCONFIGDEPTH;

typedef struct _glsCONFIGCOLORALPHA {
    BYTE  cColorBits;
    BYTE  cRedBits;
    BYTE  cRedShift;
    BYTE  cGreenBits;
    BYTE  cGreenShift;
    BYTE  cBlueBits;
    BYTE  cBlueShift;
    BYTE  cAlphaBits;
    BYTE  cAlphaShift;
} glsCONFIGCOLORALPHA;

typedef struct _glsCONFIGACCUM {
    BYTE  cAccumBits;
    BYTE  cAccumRedBits;
    BYTE  cAccumGreenBits;
    BYTE  cAccumBlueBits;
    BYTE  cAccumAlphaBits;
} glsCONFIGACCUM;

typedef struct _glsCONFIGMULTISAMPLE {
    GLuint                extFlags;
    GLint                 sampleQuality;
} glsCONFIGMULTISAMPLE;

typedef struct _glsDEVICEPIPELINEGLOBAL {
    GLuint                  processID;

    GLbyte                  *__glPFDTable;      /* Table of pixel format */
    LONG                    dPFDSize;           /*Displayable size*/
    LONG                    dPFDSizeNonDisplay; /*Non-Displayable size*/

    /* Desktop information */
    GLuint                  width;
    GLuint                  height;
    GLuint                  bpp;
    GLuint                  stride;
    /* Frame buffer base physical address for on screen */
    GLvoid *logicalAddress;
    GLvoid *basePhyAddress;
    GLuint                  numContext;

    /* Mapped gpu address and info */
    gctUINT32 gpuAddress;

    /* Worker thread for copying data. */
    gctHANDLE      workerThread;
    gctSIGNAL      startSignal;
    gctSIGNAL      stopSignal;
    gctSIGNAL      doneSignal;
    gctPOINTER     suspendMutex;

    /* Sentinel for a list of active workers that are queued up. */
    struct glsWorkerInfo        workerSentinel;
} glsDEVICEPIPELINEGLOBAL;

typedef glsDEVICEPIPELINEGLOBAL    *glsDEVICEPIPELINEGLOBAL_PTR;

#ifdef __cplusplus
}
#endif

#endif /* _chip_device_h_ */
