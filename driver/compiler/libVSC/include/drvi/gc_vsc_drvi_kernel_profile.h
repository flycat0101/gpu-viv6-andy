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


/*********************************************************************************
   Following definitions are ONLY used for openCL(ES) to communicate with client
   query/binding request. NOTE that ONLY active parts are collected into each high
   level table.
**********************************************************************************/

#ifndef __gc_vsc_drvi_kernel_profile_h_
#define __gc_vsc_drvi_kernel_profile_h_

BEGIN_EXTERN_C()

/* Executable kernel profile definition. It is generated only when clCreateKernel calling.
   Each client kernel only has one profile like this. */
typedef struct KERNEL_EXECUTABLE_PROFILE
{
    /* The shaders that this kernel contains. */
    SHADER_EXECUTABLE_PROFILE                   ExecutableKernel;

    /* High level mapping tables from high level variables to # */
}
KERNEL_EXECUTABLE_PROFILE;

gceSTATUS vscInitializeKEP(KERNEL_EXECUTABLE_PROFILE* pKEP);
gceSTATUS vscFinalizeKEP(KERNEL_EXECUTABLE_PROFILE* pKEP);

END_EXTERN_C();

#endif /* __gc_vsc_drvi_kernel_profile_h_ */


