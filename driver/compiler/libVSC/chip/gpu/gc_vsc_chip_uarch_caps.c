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

void vscQueryHwMicroArchCaps(VSC_HW_CONFIG* pHwCfg, VSC_HW_UARCH_CAPS* pHwUArchCaps)
{
    pHwUArchCaps->hwShGrpDispatchCycles = 4;
    pHwUArchCaps->hwShPipelineCycles = 28;
    pHwUArchCaps->texldCycles = 384;
    pHwUArchCaps->cacheLdCycles = 64;
    pHwUArchCaps->cacheStCycles = 64;
    pHwUArchCaps->memLdCycles = 384;
    pHwUArchCaps->memStCycles = 384;
    pHwUArchCaps->rqPortCountOfShCore2TUFifo = 8;
    pHwUArchCaps->rqPortCountOfTUFifo2TU = 2;
    pHwUArchCaps->rqPortCountOfShCore2L1Fifo = 8;
    pHwUArchCaps->rqPortCountOfL1Fifo2L1 = 2;
    pHwUArchCaps->hwThreadNumPerHwGrpPerCore = 4;

    switch (pHwCfg->maxCoreCount)
    {
    case 1:
    case 2:
        pHwUArchCaps->texldPerCycle = 1;
        break;

    case 4:
        pHwUArchCaps->texldPerCycle = 2;
        break;

    case 8:
        if (pHwCfg->chipRevision < 0x5420)
        {
            pHwUArchCaps->texldPerCycle = 2;
        }
        else
        {
            pHwUArchCaps->texldPerCycle = 4;
        }
        break;

    default:
        pHwUArchCaps->texldPerCycle = 1;
        break;
    }
}

