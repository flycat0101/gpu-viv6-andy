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


#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "gc_hal_options.h"
#include"ovx12_vxc_binary_interface.h"
#if gcdUSE_VXC_BINARY
#include"ovx12_vxc_binaries.h"
#endif

void * GetOvx12KernelBinaryPtr(ovx12_vxc_kernel_enum type, unsigned int* len)
{
    switch(type)
    {
    default:
        printf("ERROR: Invalid ovx1.2 vxc kernel binary type!\n");
    }
    *len = 0;
    return NULL;
}

#if defined(_WINDOWS) && !gcdSTATIC_LINK
#include <windows.h>

int WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved)
{
    hInst = hInst;
    reason = reason;
    reserved = reserved;

    switch (reason)
    {
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_DETACH:
        break;

    default:
        break;

    }
    return TRUE;
}
#endif