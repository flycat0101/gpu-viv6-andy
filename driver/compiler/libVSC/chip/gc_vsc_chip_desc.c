/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_vsc.h"
#include "chip/gc_vsc_chip_desc.h"

VSC_OPTN_HWOptions* hw_options;

gctUINT32 GetRegisterFileCountPerCore()
{
    return VSC_OPTN_HWOptions_GetRegFilesPerCore(hw_options);
}
gctUINT32 GetRegisterFileLength()
{
    return VSC_OPTN_HWOptions_GetRegFileLen(hw_options);
}
gctUINT32 GetGroupDispatchCycles()
{
    return VSC_OPTN_HWOptions_GetGroupDispatchCycles(hw_options);
}
gctUINT32 GetPipelineCycles()
{
    return VSC_OPTN_HWOptions_GetPipelineCycles(hw_options);
}
gctUINT32 GetTexLoadCycles()
{
    return VSC_OPTN_HWOptions_GetTexldCycles(hw_options);
}
gctUINT32 GetMemLoadCycles()
{
    return VSC_OPTN_HWOptions_GetMemldCycles(hw_options);
}
gctUINT32 GetPortCountBetweenGPUAndTexInterface()
{
    return 8;
}
gctUINT32 GetPortCountBetweenTexInterfaceAndTexUnit()
{
    return 2;
}
gctUINT32 GetPortCountBetweenGPUAndL1Interface()
{
    return 8;
}
gctUINT32 GetPortCountBetweenL1InterfaceAndL1Cache()
{
    return 2;
}

