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


#ifndef __gc_vsc_debug_extern_h_
#define __gc_vsc_debug_extern_h_

int vscDIGetSrcLineByPC(
    void * ptr,
    unsigned int pc,
    unsigned int * line
    );

#define VSC_DI_INVALID_PC    0xffff

void vscDIGetPCBySrcLine(
    void * ptr,
    unsigned int src,
    unsigned int * start,
    unsigned int * end
    );

void vscDIGetNearPCBySrcLine(
    void * ptr,
    unsigned int src,
    unsigned int * newSrc,
    unsigned int * start,
    unsigned int * end
    );

unsigned int
vscDIGetPCByFuncName(
    void * ptr,
    char * name
    );

#endif

