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


#ifndef __gc_vsc_chip_state_programming_h_
#define __gc_vsc_chip_state_programming_h_

BEGIN_EXTERN_C()

/* Current OGL driver will use first programming info for seperated program to do something,
   but when seperated programs composing into another executable program pipeline (which must
   be un-seperated), a second programing info will be generated (this could be avoidable by
   directly using PEP/SEP), however, driver does not use this new one. So we need assure we
   can generate states for seperated program, although it is not reasonable! */
#define PROGRAMING_STATES_FOR_SEPERATED_PROGRAM      1

typedef struct _VSC_CHIP_STATES_PROGRAMMER
{
    VSC_PRIMARY_MEM_POOL  pmp;

    PVSC_SYS_CONTEXT      pSysCtx;

    gctUINT*              pStartStateBuffer;

    /* Followings are at UINT size granularity */
    gctUINT               allocSize;
    gctUINT               nextStateAddr;

    struct _gcsHINT*      pHints;
}VSC_CHIP_STATES_PROGRAMMER;

VSC_ErrCode vscInitializeChipStatesProgrammer(VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer,
                                              PVSC_SYS_CONTEXT pSysCtx,
                                              struct _gcsHINT* pHints);
VSC_ErrCode vscFinalizeChipStatesProgrammer(VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer);

VSC_ErrCode vscProgramShaderStates(SHADER_HW_INFO* pShHwInfo, VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer);

VSC_ErrCode vscVerifyShaderStates(VSC_CHIP_STATES_PROGRAMMER* pStatesPgmer);

END_EXTERN_C()

#endif /* __gc_vsc_chip_state_programming_h_ */


