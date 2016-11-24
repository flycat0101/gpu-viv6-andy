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


#ifndef __gc_vsc_chip_uarch_caps_h_
#define __gc_vsc_chip_uarch_caps_h_

/*
  This file ONLY includes the chip info that can not be retrieved from feature bit. So these
  info may be experience data or data from super micro-arch. For those who can be retrieved
  from feature bit are defined in VSC_HW_CONFIG for driver side uses.
*/

BEGIN_EXTERN_C()

typedef struct _VSC_HW_UARCH_CAPS
{
    gctUINT           hwShGrpDispatchCycles;
    gctUINT           hwShPipelineCycles;
    gctUINT           texldPerCycle;
    gctUINT           texldCycles;
    gctUINT           cacheLdCycles;
    gctUINT           cacheStCycles;
    gctUINT           memLdCycles;
    gctUINT           memStCycles;
    gctUINT           rqPortCountOfShCore2TUFifo;
    gctUINT           rqPortCountOfTUFifo2TU;
    gctUINT           rqPortCountOfShCore2L1Fifo;
    gctUINT           rqPortCountOfL1Fifo2L1;
    gctUINT           hwThreadNumPerHwGrpPerCore;
} VSC_HW_UARCH_CAPS;

void vscQueryHwMicroArchCaps(VSC_HW_CONFIG* pHwCfg, VSC_HW_UARCH_CAPS* pHwUArchCaps);

END_EXTERN_C()

#endif /* __gc_vsc_chip_uarch_caps_h_ */

