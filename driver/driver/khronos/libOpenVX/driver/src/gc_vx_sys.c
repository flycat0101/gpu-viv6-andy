/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <gc_vx_common.h>

VX_API_ENTRY vx_status VX_API_CALL vxSysSetVipFrequency(
    vx_uint32 coreIndex,
    vx_uint32 vipFscaleValue,
    vx_uint32 shaderFscaleValue
    )
{
    gceSTATUS status;

    if ((vipFscaleValue < 1) || (vipFscaleValue > 64) ||
        (shaderFscaleValue < 1) || (shaderFscaleValue > 64))
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    status = gcoHAL_SetFscaleValue(gcvNULL, coreIndex, vipFscaleValue, shaderFscaleValue);

    return (status == gcvSTATUS_OK) ? VX_SUCCESS : VX_FAILURE;
}


