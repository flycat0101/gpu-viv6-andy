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


#ifndef __gc_hal_kernel_device_h_
#define __gc_hal_kernel_device_h_

/******************************************************************************\
|*************************** gckGALDEVICE Structure ***************************|
\******************************************************************************/

typedef struct _gcsDEVICE_CONSTRUCT_ARGS
{
    gctBOOL             recovery;
    gctUINT             stuckDump;
    gctBOOL             powerManagement;
    gctBOOL             mmu;
    gctBOOL             gpuProfiler;

    gctINT              irqs[gcvCORE_COUNT];
    gctUINT             registerBases[gcvCORE_COUNT];
    gctUINT             registerSizes[gcvCORE_COUNT];
    gctUINT             chipIDs[gcvCORE_COUNT];

}
gcsDEVICE_CONSTRUCT_ARGS;

typedef struct _gckGALDEVICE
{
    /* Objects. */
    gckOS               os;
    gckKERNEL           kernels[gcdMAX_GPU_COUNT];

    /* Attributes. */
    gctSIZE_T           internalSize;
    gctPHYS_ADDR        internalPhysical;
    gctPOINTER          internalLogical;
    gckVIDMEM           internalVidMem;
    gctSIZE_T           externalSize;
    gctPHYS_ADDR        externalPhysical;
    gctPOINTER          externalLogical;
    gckVIDMEM           externalVidMem;
    gckVIDMEM           contiguousVidMem;
    gctPOINTER          contiguousBase;
    gctPHYS_ADDR        contiguousPhysical;
    gctSIZE_T           contiguousSize;
    gctBOOL             contiguousMapped;
    gctPOINTER          contiguousMappedUser;
    gctSIZE_T           systemMemorySize;
    gctUINT32           systemMemoryBaseAddress;
    gctPOINTER          registerBases[gcdMAX_GPU_COUNT];
    gctSIZE_T           registerSizes[gcdMAX_GPU_COUNT];
    gctUINT32           baseAddress;
    gctUINT32           physBase;
    gctUINT32           physSize;
    gctUINT32           requestedRegisterMemBases[gcdMAX_GPU_COUNT];
    gctSIZE_T           requestedRegisterMemSizes[gcdMAX_GPU_COUNT];
    gctUINT32           requestedContiguousBase;
    gctSIZE_T           requestedContiguousSize;

    /* IRQ management. */
    gctINT              irqLines[gcdMAX_GPU_COUNT];
    gctINT              irqIds[gcdMAX_GPU_COUNT];
    gctBOOL             isrInitializeds[gcdMAX_GPU_COUNT];
    int                 chids[gcdMAX_GPU_COUNT];
    int                 coids[gcdMAX_GPU_COUNT];
    unsigned            interrupts[gcdMAX_GPU_COUNT];
    intrspin_t          isrLock;
    struct sigevent     event;

    /* Thread management. */
    pthread_t           threadCtxts[gcdMAX_GPU_COUNT];
    gctBOOL             threadInitializeds[gcdMAX_GPU_COUNT];
    gctBOOL             killThread;

    gckDEVICE           device;

    gcsDEVICE_CONSTRUCT_ARGS args;
}
* gckGALDEVICE;

typedef struct _gcsHAL_PRIVATE_DATA
{
    gckGALDEVICE        device;
    gctPOINTER          mappedMemory;
    gctPOINTER          contiguousLogical;
}
gcsHAL_PRIVATE_DATA, * gcsHAL_PRIVATE_DATA_PTR;

gceSTATUS gckGALDEVICE_Setup_ISR(
    IN gckGALDEVICE Device
    );

gceSTATUS gckGALDEVICE_Setup_ISR_2D(
    IN gckGALDEVICE Device
    );

gceSTATUS gckGALDEVICE_Setup_ISR_VG(
    IN gckGALDEVICE Device
    );

gceSTATUS gckGALDEVICE_Release_ISR(
    IN gckGALDEVICE Device
    );

gceSTATUS gckGALDEVICE_Release_ISR_2D(
    IN gckGALDEVICE Device
    );

gceSTATUS gckGALDEVICE_Release_ISR_VG(
    IN gckGALDEVICE Device
    );

gceSTATUS gckGALDEVICE_Start_Threads(
    IN gckGALDEVICE Device
    );

gceSTATUS gckGALDEVICE_Stop_Threads(
    gckGALDEVICE Device
    );

gceSTATUS gckGALDEVICE_Start(
    IN gckGALDEVICE Device
    );

gceSTATUS gckGALDEVICE_Stop(
    gckGALDEVICE Device
    );

gceSTATUS gckGALDEVICE_Construct(
    IN gctINT IrqLine,
    IN gctUINT32 RegisterMemBase,
    IN gctSIZE_T RegisterMemSize,
    IN gctINT IrqLine2D,
    IN gctUINT32 RegisterMemBase2D,
    IN gctSIZE_T RegisterMemSize2D,
    IN gctINT IrqLineVG,
    IN gctUINT32 RegisterMemBaseVG,
    IN gctSIZE_T RegisterMemSizeVG,
    IN gctUINT32 ContiguousBase,
    IN gctSIZE_T ContiguousSize,
    IN gctSIZE_T BankSize,
    IN gctINT FastClear,
    IN gctINT Compression,
    IN gctUINT32 BaseAddress,
    IN gctUINT32 PhysBaseAddr,
    IN gctUINT32 PhysSize,
    IN gctINT PowerManagement,
    IN gctINT GpuProfiler,
    IN gcsDEVICE_CONSTRUCT_ARGS * Args,
    OUT gckGALDEVICE *Device
    );

gceSTATUS gckGALDEVICE_Destroy(
    IN gckGALDEVICE Device
    );

#endif /* __gc_hal_kernel_device_h_ */
