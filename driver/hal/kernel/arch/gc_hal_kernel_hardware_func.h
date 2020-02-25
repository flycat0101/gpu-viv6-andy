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


#ifndef __gc_hal_kernel_hardware_func_h_
#define __gc_hal_kernel_hardware_func_h_
#include "gc_hal.h"
#ifdef __cplusplus
extern "C" {
#endif

#define gcdINITIALIZE_PPU       0
#define gcdRESET_USC            0

typedef struct _gcsFUNCTION_EXECUTION * gcsFUNCTION_EXECUTION_PTR;

typedef enum {
    gcvFUNCTION_EXECUTION_MMU,
    gcvFUNCTION_EXECUTION_FLUSH,
#if gcdRESET_USC
    gcvFUNCTION_EXECUTION_RESET_USC,
    gcvFUNCTION_EXECUTION_RESET_USC2,
#endif
#if gcdINITIALIZE_PPU
    gcvFUNCTION_EXECUTION_INITIALIZE_PPU,
#endif

    gcvFUNCTION_EXECUTION_NUM
}
gceFUNCTION_EXECUTION;

typedef struct {
    gckVIDMEM_NODE      bufVidMem;
    gctUINT32           address;
    /* CPU address of the function. */
    gctPOINTER          logical;
}
gcsFUNCTION_EXECUTION_DATA, *gcsFUNCTION_EXECUTION_DATA_PTR;

typedef struct {
    gceSTATUS (*validate)(IN gcsFUNCTION_EXECUTION_PTR Execution);
    gceSTATUS (*init)(IN gcsFUNCTION_EXECUTION_PTR Execution);
    gceSTATUS (*execute)(IN gcsFUNCTION_EXECUTION_PTR Execution);
    gceSTATUS (*release)(IN gcsFUNCTION_EXECUTION_PTR Execution);
}
gcsFUNCTION_API, *gcsFUNCTION_API_PTR;


typedef struct _gcsFUNCTION_EXECUTION
{
    gctPOINTER                  hardware;

    /* Function name */
    gctCHAR                     funcName[16];

    /* Function ID */
    gceFUNCTION_EXECUTION       funcId;

    /* Function Vidmem object */
    gckVIDMEM_NODE              funcVidMem;

    /* Total bytes of the function. */
    gctSIZE_T                   funcVidMemBytes;

    /* Entry of the function. */
    gctUINT32                   address;

    /* CPU address of the function. */
    gctPOINTER                  logical;

    /* Actually bytes of the function. */
    gctUINT32                   bytes;

    /* Hardware address of END in this function. */
    gctUINT32                   endAddress;

    /* Logical of END in this function. */
    gctUINT8_PTR                endLogical;

    /* Function private data */
    gcsFUNCTION_EXECUTION_DATA_PTR data;

    /* API of functions */
    gcsFUNCTION_API funcExecution;

    /* Need this function or not */
    gctBOOL valid;

    /* Function has inited */
    gctBOOL inited;
}
gcsFUNCTION_EXECUTION;

gceSTATUS gckFUNCTION_Construct(IN         gctPOINTER Hardware);
gceSTATUS gckFUNCTION_Destory(IN    gctPOINTER Hardware);

gceSTATUS gckFUNCTION_Validate(IN gcsFUNCTION_EXECUTION_PTR Execution,
                                       IN OUT gctBOOL_PTR Valid);
gceSTATUS gckFUNCTION_Init(IN gcsFUNCTION_EXECUTION_PTR Execution);
gceSTATUS gckFUNCTION_Execute(IN gcsFUNCTION_EXECUTION_PTR Execution);
gceSTATUS gckFUNCTION_Release(IN gcsFUNCTION_EXECUTION_PTR Execution);
void gckFUNCTION_Dump(IN gcsFUNCTION_EXECUTION_PTR Execution);
#ifdef __cplusplus
}
#endif
#endif

