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


#ifndef __gc_vsc_chip_desc_h_
#define __gc_vsc_chip_desc_h_

#include "gc_vsc.h"

/*
  This file ONLY includes the chip info that can not be retrieved from feature bit. So these
  info may be experience data or data from super micro-arch. For those who can be retrieved
  from feature bit are defined in VSC_HW_CONFIG for driver side uses.
*/

BEGIN_EXTERN_C()

extern gctUINT32 GetGroupDispatchCycles();
extern gctUINT32 GetPipelineCycles();
extern gctUINT32 GetTexLoadCycles();
extern gctUINT32 GetMemLoadCycles();
extern gctUINT32 GetPortCountBetweenGPUAndTexInterface();
extern gctUINT32 GetPortCountBetweenTexInterfaceAndTexUnit();
extern gctUINT32 GetPortCountBetweenGPUAndL1Interface();
extern gctUINT32 GetPortCountBetweenL1InterfaceAndL1Cache();
extern gctUINT32 GetRegisterFileCountPerCore();
extern gctUINT32 GetRegisterFileLength();

END_EXTERN_C()

#endif /* __gc_vsc_chip_desc_h_ */

