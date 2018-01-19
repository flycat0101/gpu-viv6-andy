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


#ifndef __gc_vsc_chip_mc_dump_h_
#define __gc_vsc_chip_mc_dump_h_

BEGIN_EXTERN_C()

void vscMC_DisassembleInst(VSC_MC_CODEC* pMcCodec,
                           VSC_MC_RAW_INST* pMcInst,
                           gctUINT instIdx,
                           VSC_DUMPER* pDumper);

void vscMC_DumpInst(VSC_MC_CODEC* pMcCodec,
                    VSC_MC_RAW_INST* pMcInst,
                    gctUINT instIdx,
                    VSC_DUMPER* pDumper);

void vscMC_DumpInsts(VSC_MC_RAW_INST* pMcInsts,
                     gctUINT countOfMCInst,
                     VSC_HW_CONFIG* pHwCfg, /* Target HW MC runs */
                     gctBOOL bUnderDual16Mode,
                     VSC_DUMPER* pDumper);

END_EXTERN_C()

#endif /* __gc_vsc_chip_mc_dump_h_ */


