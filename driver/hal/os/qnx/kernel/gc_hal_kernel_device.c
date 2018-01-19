/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_hal_kernel_qnx.h"
#include <sys/mman.h>
#include <sys/siginfo.h>
#include <errno.h>

#ifndef _NTO_TCTL_IO_PRIV
#define _NTO_TCTL_IO_PRIV _NTO_TCTL_IO
#endif

#define GC_HAL_QNX_PULSEVAL_INTR    (_PULSE_CODE_MINAVAIL)
#define GC_HAL_QNX_PULSEVAL_MAJOR   (GC_HAL_QNX_PULSEVAL_INTR + gcvCORE_MAJOR)
#define GC_HAL_QNX_PULSEVAL_3D1     (GC_HAL_QNX_PULSEVAL_INTR + gcvCORE_3D1)
#define GC_HAL_QNX_PULSEVAL_3D2     (GC_HAL_QNX_PULSEVAL_INTR + gcvCORE_3D2)
#define GC_HAL_QNX_PULSEVAL_3D3     (GC_HAL_QNX_PULSEVAL_INTR + gcvCORE_3D3)
#define GC_HAL_QNX_PULSEVAL_2D      (GC_HAL_QNX_PULSEVAL_INTR + gcvCORE_2D)
#define GC_HAL_QNX_PULSEVAL_VG      (GC_HAL_QNX_PULSEVAL_INTR + gcvCORE_VG)

#define _GC_OBJ_ZONE    gcvZONE_DEVICE

/******************************************************************************\
*************************** Memory Allocation Wrappers *************************
\******************************************************************************/

gceSTATUS
_AllocateMemory(
    IN gckGALDEVICE Device,
    IN gctSIZE_T Bytes,
    OUT gctPOINTER *Logical,
    OUT gctPHYS_ADDR *Physical,
    OUT gctUINT32 *PhysAddr
    )
{
    gceSTATUS status;
    gcsPHYSICAL_PTR node;

    gcmkHEADER_ARG("Bytes=%u", Bytes);

    gcmkVERIFY_ARGUMENT(Device != NULL);
    gcmkVERIFY_ARGUMENT(Logical != NULL);
    gcmkVERIFY_ARGUMENT(Physical != NULL);
    gcmkVERIFY_ARGUMENT(PhysAddr != NULL);

    status = gckOS_AllocateContiguous(Device->os,
                      gcvFALSE,
                      &Bytes,
                      Physical,
                      Logical);

    if (gcmIS_ERROR(status))
    {
        gcmkTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_DRIVER,
                       "_AllocateMemory: error status->0x%x",
                       status);
        gcmkFOOTER();
        return status;
    }

    node = (gcsPHYSICAL_PTR) *Physical;

    gcmkASSERT(node->type == gcvPHYSICAL_TYPE_MEMPOOL);
    gcmkASSERT(node->physicalAddress != 0);

    *PhysAddr = node->physicalAddress;

    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_DRIVER,
                   "_AllocateMemory: phys_addr->0x%x phsical->0x%x Logical->0x%x",
                   (gctUINT32)*Physical,
                   (gctUINT32)*PhysAddr,
                   (gctUINT32)*Logical);

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_FreeMemory(
    IN gckGALDEVICE Device,
    IN gctPOINTER Logical,
    IN gctPHYS_ADDR Physical)
{
    gceSTATUS status;

    gcmkHEADER_ARG("Device=0x%x Logical=0x%x Physical=0x%x",
                   Device, Logical, Physical);

    gcmkVERIFY_ARGUMENT(Device != NULL);

    status = gckOS_FreeContiguous(Device->os,
                                Physical,
                                Logical,
                                0);

    gcmkFOOTER();
    return status;
}

/******************************************************************************\
******************************* Interrupt Handler ******************************
\******************************************************************************/

struct sigevent gIntrEvents[gcdMAX_GPU_COUNT];
int gIntrChids[gcdMAX_GPU_COUNT];
int gIntrCoids[gcdMAX_GPU_COUNT];
volatile unsigned g_nQnxInIsrs = 0;

const struct sigevent* isrRoutine(void* arg, int id)
{
    gckGALDEVICE device = (gckGALDEVICE)arg;

    atomic_add(&g_nQnxInIsrs, 1);

    if (gckKERNEL_Notify(device->kernels[gcvCORE_MAJOR],
                         gcvNOTIFY_INTERRUPT,
                         gcvTRUE) == gcvSTATUS_OK)
    {
        atomic_add((volatile unsigned *)&device->interrupts[gcvCORE_MAJOR], 1);

        atomic_sub(&g_nQnxInIsrs, 1);

        return &gIntrEvents[gcvCORE_MAJOR];
    }

    atomic_sub(&g_nQnxInIsrs, 1);

    return gcvNULL;
}

static void* threadRoutine(void *ctxt)
{
    int rc;
    struct _pulse pulse;

    gckGALDEVICE device = (gckGALDEVICE) ctxt;

    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_DRIVER,
                "Starting ISR Thread with irq:%d\n",
                device->irqLines[gcvCORE_MAJOR]);

    device->isrInitializeds[gcvCORE_MAJOR] = gcvFALSE;

    if ((gIntrChids[gcvCORE_MAJOR] = ChannelCreate(_NTO_CHF_DISCONNECT | _NTO_CHF_UNBLOCK)) == -1) {
        gcmkTRACE_ZONE(gcvLEVEL_ERROR,
                        gcvZONE_DRIVER,
                        "[galcore] gckGALDEVICE_Setup_ISR: "
                        "Could not create channel (%s)\n",
                        strerror(errno));
        return (void *)1;
    }

    if ((gIntrCoids[gcvCORE_MAJOR] = ConnectAttach(0, 0, gIntrChids[gcvCORE_MAJOR], _NTO_SIDE_CHANNEL, 0)) == -1) {
        gcmkTRACE_ZONE(gcvLEVEL_ERROR,
                        gcvZONE_DRIVER,
                        "[galcore] gckGALDEVICE_Setup_ISR: "
                        "Could not create connection (%s)\n",
                        strerror(errno));
        goto fail1;
    }

    SIGEV_PULSE_INIT(&gIntrEvents[gcvCORE_MAJOR], gIntrCoids[gcvCORE_MAJOR],
            21, GC_HAL_QNX_PULSEVAL_MAJOR, gcvNULL);

    /* Obtain I/O privileges */
    ThreadCtl( _NTO_TCTL_IO_PRIV, 0 );

    device->irqIds[gcvCORE_MAJOR] = InterruptAttach(device->irqLines[gcvCORE_MAJOR],
                                    isrRoutine,
                                    (void*)device,
                                    gcmSIZEOF(struct _gckGALDEVICE),
                                    _NTO_INTR_FLAGS_TRK_MSK);

    if (device->irqIds[gcvCORE_MAJOR] < 0) {
        gcmkTRACE_ZONE(gcvLEVEL_ERROR,
                gcvZONE_DRIVER,
                "[galcore] gckGALDEVICE_Setup_ISR: "
                "Could not register irq line->%d\n",
                device->irqLines[gcvCORE_MAJOR]);

        goto fail2;
    }

    gcmkTRACE_ZONE(gcvLEVEL_INFO,
                    gcvZONE_DRIVER,
                    "irqId:%d\n",
                    device->irqIds[gcvCORE_MAJOR]);

    gcmkPRINT("[Interrupt] Attached irqLine %d with id %d.\n",
        device->irqLines[gcvCORE_MAJOR], device->irqIds[gcvCORE_MAJOR]);

    device->isrInitializeds[gcvCORE_MAJOR] = gcvTRUE;

    while (1)
    {
        rc = MsgReceivePulse_r(gIntrChids[gcvCORE_MAJOR], &pulse, gcmSIZEOF(pulse), gcvNULL);

        if (rc == (-1 * EINTR))
        {
            continue;
        }

        if (rc == (-1 * ESRCH))
        {
            break;
        }

        if (rc != EOK)
        {
            continue;
        }

        if (pulse.code == GC_HAL_QNX_PULSEVAL_MAJOR)
        {
            gckKERNEL_Notify(device->kernels[gcvCORE_MAJOR], gcvNOTIFY_INTERRUPT, gcvFALSE);
        }
    }

    return (void *)0;

fail2:
    ConnectDetach(gIntrCoids[gcvCORE_MAJOR]);

fail1:
    ChannelDestroy(gIntrChids[gcvCORE_MAJOR]);

    return (void *)1;
}

#ifdef gcdDUAL_CORE
const struct sigevent* isrRoutine3D1(void* arg, int id)
{
    gckGALDEVICE device = (gckGALDEVICE)arg;

    atomic_add(&g_nQnxInIsrs, 1);

    if (gckKERNEL_Notify(device->kernels[gcvCORE_3D1],
                         gcvNOTIFY_INTERRUPT,
                         gcvTRUE) == gcvSTATUS_OK)
    {
        atomic_add((volatile unsigned *)&device->interrupts[gcvCORE_3D1], 1);
        atomic_sub(&g_nQnxInIsrs, 1);

        return &gIntrEvents[gcvCORE_3D1];
    }
    atomic_sub(&g_nQnxInIsrs, 1);
    return gcvNULL;
}

static void* threadRoutine3D1(void *ctxt)
{
    int rc;
    struct _pulse pulse;

    gckGALDEVICE device = (gckGALDEVICE) ctxt;

    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_DRIVER,
                "Starting ISR Thread for 3D core 1 with irq:%d\n",
                device->irqLines[gcvCORE_3D1]);

    device->isrInitializeds[gcvCORE_3D1] = gcvFALSE;

    if ((gIntrChids[gcvCORE_3D1] = ChannelCreate(_NTO_CHF_DISCONNECT | _NTO_CHF_UNBLOCK)) == -1) {
        gcmkTRACE_ZONE(gcvLEVEL_ERROR,
                        gcvZONE_DRIVER,
                        "[galcore] gckGALDEVICE_Setup_ISR for 3D core 1: "
                        "Could not create channel (%s)\n",
                        strerror(errno));
        return (void *)1;
    }

    if ((gIntrCoids[gcvCORE_3D1] = ConnectAttach(0, 0, gIntrChids[gcvCORE_3D1], _NTO_SIDE_CHANNEL, 0)) == -1) {
        gcmkTRACE_ZONE(gcvLEVEL_ERROR,
                        gcvZONE_DRIVER,
                        "[galcore] gckGALDEVICE_Setup_ISR for 3D core 1: "
                        "Could not create connection (%s)\n",
                        strerror(errno));
        goto fail1;
    }

    SIGEV_PULSE_INIT(&gIntrEvents[gcvCORE_3D1], gIntrCoids[gcvCORE_3D1],
            21, GC_HAL_QNX_PULSEVAL_3D1, gcvNULL);

    /* Obtain I/O privileges */
    ThreadCtl( _NTO_TCTL_IO_PRIV, 0 );

    device->irqIds[gcvCORE_3D1] = InterruptAttach(device->irqLines[gcvCORE_3D1],
                                    isrRoutine3D1,
                                    (void*)device,
                                    gcmSIZEOF(struct _gckGALDEVICE),
                                    _NTO_INTR_FLAGS_TRK_MSK);

    if (device->irqIds[gcvCORE_3D1] < 0) {
        gcmkTRACE_ZONE(gcvLEVEL_ERROR,
                gcvZONE_DRIVER,
                "[galcore] gckGALDEVICE_Setup_ISR for 3D core 1: "
                "Could not register irq line->%d\n",
                device->irqLines[gcvCORE_3D1]);

        goto fail2;
    }

    gcmkTRACE_ZONE(gcvLEVEL_INFO,
                    gcvZONE_DRIVER,
                    "irqId:%d\n",
                    device->irqIds[gcvCORE_3D1]);

    gcmkPRINT("[Interrupt] Attached irqLine %d with id %d.\n",
        device->irqLines[gcvCORE_3D1], device->irqIds[gcvCORE_3D1]);

    device->isrInitializeds[gcvCORE_3D1] = gcvTRUE;

    while (1)
    {
        rc = MsgReceivePulse_r(gIntrChids[gcvCORE_3D1], &pulse, gcmSIZEOF(pulse), gcvNULL);

        if (rc == (-1 * EINTR))
        {
            continue;
        }

        if (rc == (-1 * ESRCH))
        {
            break;
        }

        if (rc != EOK)
        {
            continue;
        }

        if (pulse.code == GC_HAL_QNX_PULSEVAL_3D1)
        {
            gckKERNEL_Notify(device->kernels[gcvCORE_3D1], gcvNOTIFY_INTERRUPT, gcvFALSE);
        }
    }

    return (void *)0;

fail2:
    ConnectDetach(gIntrCoids[gcvCORE_3D1]);

fail1:
    ChannelDestroy(gIntrChids[gcvCORE_3D1]);

    return (void *)1;
}
#endif

const struct sigevent* isrRoutine2D(void* arg, int id)
{
    gckGALDEVICE device = (gckGALDEVICE)arg;

    atomic_add(&g_nQnxInIsrs, 1);

    if (gckKERNEL_Notify(device->kernels[gcvCORE_2D],
                         gcvNOTIFY_INTERRUPT,
                         gcvTRUE) == gcvSTATUS_OK)
    {
        atomic_add((volatile unsigned *)&device->interrupts[gcvCORE_2D], 1);

        atomic_sub(&g_nQnxInIsrs, 1);

        return &gIntrEvents[gcvCORE_2D];
    }

    atomic_sub(&g_nQnxInIsrs, 1);

    return gcvNULL;
}

static void* threadRoutine2D(void *ctxt)
{
    int rc;
    struct _pulse pulse;

    gckGALDEVICE device = (gckGALDEVICE) ctxt;

    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_DRIVER,
                "Starting ISR Thread with irq:%d\n",
                device->irqLines[gcvCORE_2D]);

    device->isrInitializeds[gcvCORE_2D] = gcvFALSE;

    if ((gIntrChids[gcvCORE_2D] = ChannelCreate(_NTO_CHF_DISCONNECT | _NTO_CHF_UNBLOCK)) == -1) {
        gcmkTRACE_ZONE(gcvLEVEL_ERROR,
                        gcvZONE_DRIVER,
                        "[galcore] gckGALDEVICE_Setup_ISR: "
                        "Could not create channel (%s)\n",
                        strerror(errno));
        return (void *)1;
    }

    if ((gIntrCoids[gcvCORE_2D] = ConnectAttach(0, 0, gIntrChids[gcvCORE_2D], _NTO_SIDE_CHANNEL, 0)) == -1) {
        gcmkTRACE_ZONE(gcvLEVEL_ERROR,
                        gcvZONE_DRIVER,
                        "[galcore] gckGALDEVICE_Setup_ISR: "
                        "Could not create connection (%s)\n",
                        strerror(errno));
        goto fail1;
    }

    SIGEV_PULSE_INIT(&gIntrEvents[gcvCORE_2D], gIntrCoids[gcvCORE_2D],
            21, GC_HAL_QNX_PULSEVAL_2D, gcvNULL);

    /* Obtain I/O privileges */
    ThreadCtl( _NTO_TCTL_IO_PRIV, 0 );

    device->irqIds[gcvCORE_2D] = InterruptAttach(device->irqLines[gcvCORE_2D],
                                    isrRoutine2D,
                                    (void*)device,
                                    gcmSIZEOF(struct _gckGALDEVICE),
                                    _NTO_INTR_FLAGS_TRK_MSK);

    if (device->irqIds[gcvCORE_2D] < 0) {
        gcmkTRACE_ZONE(gcvLEVEL_ERROR,
                gcvZONE_DRIVER,
                "[galcore] gckGALDEVICE_Setup_ISR: "
                "Could not register irq line->%d\n",
                device->irqLines[gcvCORE_2D]);

        goto fail2;
    }

    gcmkTRACE_ZONE(gcvLEVEL_INFO,
                    gcvZONE_DRIVER,
                    "irqId:%d\n",
                    device->irqIds[gcvCORE_2D]);

    gcmkPRINT("[Interrupt] Attached irqLine %d with id %d.\n",
        device->irqLines[gcvCORE_2D], device->irqIds[gcvCORE_2D]);

    device->isrInitializeds[gcvCORE_2D] = gcvTRUE;

    while (1)
    {
        rc = MsgReceivePulse_r(gIntrChids[gcvCORE_2D], &pulse, gcmSIZEOF(pulse), gcvNULL);

        if (rc == (-1 * EINTR))
        {
            continue;
        }

        if (rc == (-1 * ESRCH))
        {
            break;
        }

        if (rc != EOK)
        {
            continue;
        }

        if (pulse.code == GC_HAL_QNX_PULSEVAL_2D)
        {
            gckKERNEL_Notify(device->kernels[gcvCORE_2D],
                             gcvNOTIFY_INTERRUPT,
                             gcvFALSE);
        }
    }

    return (void *)0;

fail2:
    ConnectDetach(gIntrCoids[gcvCORE_2D]);

fail1:
    ChannelDestroy(gIntrChids[gcvCORE_2D]);

    return (void *)1;
}

#if gcdENABLE_VG

static struct
{
    gckOS           os;
    gctSEMAPHORE    semaphore;
} s_VgIsrNotify[1024];

volatile unsigned s_nVgIsrIndex = 0;
volatile unsigned s_nVgIstIndex = 0;

const struct sigevent* isrRoutineVG(void* arg, int id)
{
    gckGALDEVICE device = (gckGALDEVICE)arg;

    gckOS os = gcvNULL;
    gctSEMAPHORE semaphore = gcvNULL;

    atomic_add(&g_nQnxInIsrs, 1);

    gckVGINTERRUPT_Enque(
            device->kernels[gcvCORE_VG]->vg->interrupt,
            &os, &semaphore);

    atomic_add((volatile unsigned *)&device->interrupts[gcvCORE_VG], 1);

    s_VgIsrNotify[s_nVgIsrIndex].os = os;
    s_VgIsrNotify[s_nVgIsrIndex].semaphore = semaphore;

    s_nVgIsrIndex = (s_nVgIsrIndex + 1) % gcmCOUNTOF(s_VgIsrNotify);

    atomic_sub(&g_nQnxInIsrs, 1);

    return &gIntrEvents[gcvCORE_VG];
}

static void* threadRoutineVG(void *ctxt)
{
    int rc;
    struct _pulse pulse;

    gckGALDEVICE device = (gckGALDEVICE) ctxt;

    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_DRIVER,
                "Starting ISR Thread with irq:%d\n",
                device->irqLines[gcvCORE_VG]);

    device->isrInitializeds[gcvCORE_VG] = gcvFALSE;

    if ((gIntrChids[gcvCORE_VG] = ChannelCreate(_NTO_CHF_DISCONNECT | _NTO_CHF_UNBLOCK)) == -1) {
        gcmkTRACE_ZONE(gcvLEVEL_ERROR,
                        gcvZONE_DRIVER,
                        "[galcore] gckGALDEVICE_Setup_ISR: "
                        "Could not create channel (%s)\n",
                        strerror(errno));
        return (void *)1;
    }

    if ((gIntrCoids[gcvCORE_VG] = ConnectAttach(0, 0, gIntrChids[gcvCORE_VG], _NTO_SIDE_CHANNEL, 0)) == -1) {
        gcmkTRACE_ZONE(gcvLEVEL_ERROR,
                        gcvZONE_DRIVER,
                        "[galcore] gckGALDEVICE_Setup_ISR: "
                        "Could not create connection (%s)\n",
                        strerror(errno));
        goto fail1;
    }

    SIGEV_PULSE_INIT(&gIntrEvents[gcvCORE_VG], gIntrCoids[gcvCORE_VG],
            21, GC_HAL_QNX_PULSEVAL_VG, gcvNULL);

    /* Obtain I/O privileges */
    ThreadCtl( _NTO_TCTL_IO_PRIV, 0 );

    device->irqIds[gcvCORE_VG] = InterruptAttach(device->irqLines[gcvCORE_VG],
                                    isrRoutineVG,
                                    (void*)device,
                                    gcmSIZEOF(struct _gckGALDEVICE),
                                    _NTO_INTR_FLAGS_TRK_MSK);

    if (device->irqIds[gcvCORE_VG] < 0) {
        gcmkTRACE_ZONE(gcvLEVEL_ERROR,
                gcvZONE_DRIVER,
                "[galcore] gckGALDEVICE_Setup_ISR: "
                "Could not register irq line->%d\n",
                device->irqLines[gcvCORE_VG]);

        goto fail2;
    }

    gcmkTRACE_ZONE(gcvLEVEL_INFO,
                    gcvZONE_DRIVER,
                    "irqId:%d\n",
                    device->irqIds[gcvCORE_VG]);

    gcmkPRINT("[Interrupt] Attached irqLine %d with id %d.\n",
        device->irqLines[gcvCORE_VG], device->irqIds[gcvCORE_VG]);

    device->isrInitializeds[gcvCORE_VG] = gcvTRUE;

    while (1)
    {
        rc = MsgReceivePulse_r(gIntrChids[gcvCORE_VG], &pulse, gcmSIZEOF(pulse), gcvNULL);

        if (rc == (-1 * EINTR))
        {
            continue;
        }

        if (rc == (-1 * ESRCH))
        {
            break;
        }

        if (rc != EOK)
        {
            continue;
        }

        if (pulse.code == GC_HAL_QNX_PULSEVAL_VG)
        {
            if (s_VgIsrNotify[s_nVgIstIndex].semaphore != gcvNULL)
            {
                gckOS_IncrementSemaphore(
                        s_VgIsrNotify[s_nVgIstIndex].os,
                        s_VgIsrNotify[s_nVgIstIndex].semaphore);
            }

            s_nVgIstIndex = (s_nVgIstIndex + 1) % gcmCOUNTOF(s_VgIsrNotify);
        }
    }

    return (void *)0;

fail2:
    ConnectDetach(gIntrCoids[gcvCORE_VG]);

fail1:
    ChannelDestroy(gIntrChids[gcvCORE_VG]);

    return (void *)1;
}

#endif /* gcdENABLE_VG */

int
gsl_free_interrupts()
{
    return 0;
}

/*******************************************************************************
**
**  gckGALDEVICE_Construct
**
**  Constructor.
**
**  INPUT:
**
**  OUTPUT:
**
**      gckGALDEVICE * Device
**          Pointer to a variable receiving the gckGALDEVICE object pointer on
**          success.
*/
gceSTATUS
gckGALDEVICE_Construct(
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
)
{
    gctUINT32 internalBaseAddress = 0, internalAlignment = 0;
    gctUINT32 externalBaseAddress = 0, externalAlignment = 0;
    gctUINT32 horizontalTileSize, verticalTileSize;
    gctUINT32 physAddr;
    gctUINT32 physical;
    gckGALDEVICE device;
    gceSTATUS status;
    gctINT32 i;
    gceHARDWARE_TYPE type;

    gcmkTRACE(gcvLEVEL_VERBOSE, "[galcore] Enter gckGALDEVICE_Construct");

    /* Allocate device structure. */
    device = calloc(1, gcmSIZEOF(struct _gckGALDEVICE));
    if (!device)
    {
        gcmkTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_DRIVER,
                      "[galcore] gckGALDEVICE_Construct: Can't allocate memory.");

        return gcvSTATUS_OUT_OF_MEMORY;
    }

    memset(device, 0, gcmSIZEOF(struct _gckGALDEVICE));

    device->args = *Args;

    gckDEBUGFS_Initialize(device);

    if (IrqLine != -1)
    {
        device->requestedRegisterMemBases[gcvCORE_MAJOR]    = RegisterMemBase;
        device->requestedRegisterMemSizes[gcvCORE_MAJOR]    = RegisterMemSize;
    }

    if (IrqLine2D != -1)
    {
        device->requestedRegisterMemBases[gcvCORE_2D]       = RegisterMemBase2D;
        device->requestedRegisterMemSizes[gcvCORE_2D]       = RegisterMemSize2D;
    }

    if (IrqLineVG != -1)
    {
        device->requestedRegisterMemBases[gcvCORE_VG]       = RegisterMemBaseVG;
        device->requestedRegisterMemSizes[gcvCORE_VG]       = RegisterMemSizeVG;
    }

    for (i = gcvCORE_MAJOR; i < gcvCORE_COUNT; i++)
    {
        if (Args->irqs[i] != -1)
        {
            device->requestedRegisterMemBases[i] = Args->registerBases[i];
            device->requestedRegisterMemSizes[i] = Args->registerSizes[i];
            device->irqLines[i] = Args->irqs[i];

            gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_DEVICE,
                           "%s(%d): Core = %d, RegiseterBase = %x, IRQ Line = %d",
                           __FUNCTION__, __LINE__,
                           i, Args->registerBases[i], device->irqLines[i]
                           );
        }
    }

    device->requestedContiguousBase  = 0;
    device->requestedContiguousSize  = 0;

    for (i = 0; i < gcdMAX_GPU_COUNT; i++)
    {
        physical = device->requestedRegisterMemBases[i];

        /* Set up register memory region */
        if (physical != 0)
        {
            /* Request a region. */
            device->registerBases[i] = (gctPOINTER)mmap_device_io(
                    device->requestedRegisterMemSizes[i],
                    device->requestedRegisterMemBases[i]);

            if ((uintptr_t)device->registerBases[i] == MAP_DEVICE_FAILED)
            {
                gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_DRIVER,
                        "[galcore] gckGALDEVICE_Construct: Unable to map location->0x%lX for size->%ld\n",
                        (unsigned long)device->requestedRegisterMemBases[i],
                        device->requestedRegisterMemSizes[i]);

                return gcvSTATUS_OUT_OF_RESOURCES;
            }

            physical += device->requestedRegisterMemSizes[i];

            device->registerSizes[i] = device->requestedRegisterMemSizes[i];

            gcmkTRACE_ZONE(gcvLEVEL_INFO,
                    gcvZONE_DRIVER,
                    "[galcore] gckGALDEVICE_Construct: "
                    "RegisterBase after mapping Address->0x%x is 0x%x",
                    (gctUINT32)device->requestedRegisterMemBases[i],
                    (gctUINT32)device->registerBases[i]);
        }
        else
        {
            device->registerBases[i] = gcvNULL;
        }
    }

    /* Set the base address */
    device->baseAddress = BaseAddress;
    device->physBase    = PhysBaseAddr;
    device->physSize    = PhysSize;

    /* construct the gckOS object */
    gcmkONERROR(gckOS_Construct(device, &device->os));

    /* Construct the gckDEVICE object for os independent core management. */
    gcmkONERROR(gckDEVICE_Construct(device->os, &device->device));

    /* set up the contiguous memory */
    device->contiguousBase = ContiguousBase;
    device->contiguousSize = ContiguousSize;

    if ((ContiguousSize > 0) && (ContiguousBase == 0))
    {
        status = gcvSTATUS_OUT_OF_MEMORY;

        while (device->contiguousSize > 0)
        {
            gcmkTRACE_ZONE(
                gcvLEVEL_INFO, gcvZONE_DRIVER,
                "[galcore] gckGALDEVICE_Construct: Will be trying to allocate contiguous memory of %ld bytes",
                device->contiguousSize
                );

            /* Allocate contiguous memory */
            status = _AllocateMemory(
                device,
                device->contiguousSize,
                &device->contiguousLogical,
                &device->contiguousPhysical,
                &physAddr
                );

            if (gcmIS_SUCCESS(status))
            {
                gcmkTRACE_ZONE(
                    gcvLEVEL_INFO, gcvZONE_DRIVER,
                    "[galcore] gckGALDEVICE_Construct: Contiguous allocated size->0x%08X Virt->0x%08lX physAddr->0x%08llX",
                    device->contiguousSize,
                    device->contiguousBase,
                    physAddr
                    );

                status = gckVIDMEM_Construct(
                    device->os,
                    physAddr | device->systemMemoryBaseAddress,
                    device->contiguousSize,
                    64,
                    BankSize,
                    &device->contiguousVidMem
                    );

                if (gcmIS_SUCCESS(status))
                {
                    device->contiguousMapped = gcvFALSE;
                    device->contiguousBase = physAddr;

                    /* success, abort loop */
                    gcmkTRACE_ZONE(
                        gcvLEVEL_INFO, gcvZONE_DRIVER,
                        "Using %u bytes of contiguous memory.",
                        device->contiguousSize
                        );

                    break;
                }

                gcmkVERIFY_OK(_FreeMemory(
                    device,
                    device->contiguousLogical,
                    device->contiguousPhysical
                    ));

                device->contiguousLogical   = gcvNULL;
                device->contiguousPhysical  = gcvNULL;
            }

            if (device->contiguousSize <= (4 << 20))
            {
                device->contiguousSize = 0;
            }
            else
            {
                device->contiguousSize -= (4 << 20);
            }
        }
    }
    else if (ContiguousSize > 0)
    {
        /* Create the contiguous memory heap. */
        status = gckVIDMEM_Construct(
            device->os,
            ContiguousBase | device->systemMemoryBaseAddress,
            ContiguousSize,
            64,
            BankSize,
            &device->contiguousVidMem
            );

        if (gcmIS_ERROR(status))
        {
            /* Error, roll back. */
            device->contiguousVidMem = gcvNULL;
            device->contiguousSize   = 0;
        }
        else
        {
            /* Map the contiguous memory. */
            device->contiguousPhysical = gcmUINT64_TO_PTR(ContiguousBase);
            device->contiguousSize     = ContiguousSize;
            device->contiguousLogical  = (gctPOINTER) mmap_device_io(ContiguousSize, ContiguousBase);
            device->contiguousMapped   = gcvTRUE;

            if (device->contiguousLogical == gcvNULL)
            {
                /* Error, roll back. */
                gcmkVERIFY_OK(gckVIDMEM_Destroy(device->contiguousVidMem));
                device->contiguousVidMem = gcvNULL;
                device->contiguousSize   = 0;

                status = gcvSTATUS_OUT_OF_RESOURCES;
            }
        }
    }

    for (i = gcvCORE_MAJOR; i <= gcvCORE_3D3; i++)
    {
        if (Args->irqs[i] != -1)
        {
            gckDEVICE_AddCore(device->device, i, Args->chipIDs[i], device, &device->kernels[i]);

            gcmkONERROR(
                gckHARDWARE_SetFastClear(device->kernels[i]->hardware,
                    FastClear,
                    Compression));

            gcmkONERROR(gckHARDWARE_SetPowerManagement(
                device->kernels[i]->hardware, PowerManagement
                ));

            gcmkONERROR(gckHARDWARE_SetGpuProfiler(
                device->kernels[i]->hardware, GpuProfiler
                ));
        }
        else
        {
            device->kernels[i] = gcvNULL;
        }
    }

    if (IrqLine2D != -1)
    {
        gckDEVICE_AddCore(device->device, gcvCORE_2D, gcvCHIP_ID_DEFAULT,device, &device->kernels[gcvCORE_2D]);

        /* Verify the hardware type */
        gcmkONERROR(gckHARDWARE_GetType(device->kernels[gcvCORE_2D]->hardware, &type));

        if (type != gcvHARDWARE_2D)
        {
            gcmkTRACE_ZONE(
                    gcvLEVEL_ERROR, gcvZONE_DRIVER,
                    "%s(%d): Unexpected hardware type: %d\n",
                    __FUNCTION__, __LINE__,
                    type
                    );

            gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        gcmkONERROR(gckHARDWARE_SetPowerManagement(
            device->kernels[gcvCORE_2D]->hardware, PowerManagement
            ));
    }
    else
    {
        device->kernels[gcvCORE_2D] = gcvNULL;
    }

    if (IrqLineVG != -1)
    {
#if gcdENABLE_VG
        gckDEVICE_AddCore(device->device, gcvCORE_VG, gcvCHIP_ID_DEFAULT, device, &device->kernels[gcvCORE_VG]);

        gcmkONERROR(gckVGHARDWARE_SetPowerManagement(
            device->kernels[gcvCORE_VG]->vg->hardware,
            PowerManagement
            ));
#endif /* gcdENABLE_VG */
    }
    else
    {
        device->kernels[gcvCORE_VG] = gcvNULL;
    }

    /* Initialize the ISR. */
    device->irqLines[gcvCORE_MAJOR]    = IrqLine;
    device->irqLines[gcvCORE_2D]       = IrqLine2D;
    device->irqLines[gcvCORE_VG]       = IrqLineVG;

    for (i = 0; i < gcdMAX_GPU_COUNT; i++)
    {
        if (device->kernels[i] != gcvNULL) break;
    }

    if (i == gcdMAX_GPU_COUNT)
    {
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

#if gcdENABLE_VG
    if (i == gcvCORE_VG)
    {
        /* query the ceiling of the system memory */
        gcmkONERROR(gckVGHARDWARE_QuerySystemMemory(device->kernels[i]->vg->hardware,
                    &device->systemMemorySize,
                    &device->systemMemoryBaseAddress));

        /* query the amount of video memory */
        gcmkVERIFY_OK(gckVGHARDWARE_QueryMemory(device->kernels[i]->vg->hardware,
                    &device->internalSize,
                    &internalBaseAddress,
                    &internalAlignment,
                    &device->externalSize,
                    &externalBaseAddress,
                    &externalAlignment,
                    &horizontalTileSize,
                    &verticalTileSize));
    }
    else
#endif /* gcdENABLE_VG */
    {
        /* query the ceiling of the system memory */
        gcmkONERROR(gckHARDWARE_QuerySystemMemory(device->kernels[i]->hardware,
                    &device->systemMemorySize,
                    &device->systemMemoryBaseAddress));

        /* query the amount of video memory */
        gcmkVERIFY_OK(gckHARDWARE_QueryMemory(device->kernels[i]->hardware,
                    &device->internalSize,
                    &internalBaseAddress,
                    &internalAlignment,
                    &device->externalSize,
                    &externalBaseAddress,
                    &externalAlignment,
                    &horizontalTileSize,
                    &verticalTileSize));
    }

    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_DRIVER,
            "[galcore] gckGALDEVICE_Construct: "
            "Will be trying to allocate contiguous memory of 0x%x bytes",
            (gctUINT32)device->systemMemoryBaseAddress);

    /* initialize the thread daemon */
    memset(&device->isrLock, 0, sizeof(device->isrLock));

    /* set up the internal memory region */
    if (device->internalSize > 0)
    {
        gceSTATUS status = gckVIDMEM_Construct(device->os,
                    internalBaseAddress,
                    device->internalSize,
                    internalAlignment,
                    0,
                    &device->internalVidMem);

        if (gcmIS_ERROR(status))
        {
            /* error, remove internal heap */
            device->internalSize = 0;
        }
        else
        {
            /* map internal memory */
            device->internalPhysical  = gcmUINT64_TO_PTR(physical);
            device->internalLogical = (gctPOINTER)mmap_device_io(device->internalSize, physical);

            gcmkASSERT(device->internalLogical != NULL);

            physical += device->internalSize;
        }
    }

    if (device->externalSize > 0)
    {
        /* create the external memory heap */
        gceSTATUS status = gckVIDMEM_Construct(device->os,
                    externalBaseAddress,
                    device->externalSize,
                    externalAlignment,
                    0,
                    &device->externalVidMem);

        if (gcmIS_ERROR(status))
        {
            /* error, remove internal heap */
            device->externalSize = 0;
        }
        else
        {
            /* map internal memory */
            device->externalPhysical = gcmUINT64_TO_PTR(physical);
            device->externalLogical = (gctPOINTER)mmap_device_io(device->externalSize, physical);

            gcmkASSERT(device->externalLogical != NULL);

            physical += device->externalSize;
        }
    }

    *Device = device;

    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_DRIVER,
                  "[galcore] gckGALDEVICE_Construct: Initialized device->%p contiguous->%lu @ %p (0x%08X)",
                  device,
                  device->contiguousSize,
                  device->contiguousBase,
                  device->contiguousPhysical);

    return gcvSTATUS_OK;
OnError:
    /* Roll back. */

    /* Destroy the gckKERNEL object. */
    if (Device != gcvNULL)
    {
        gcmkVERIFY_OK(gckGALDEVICE_Destroy(*Device));
    }

    /* Return the status. */
    return status;
}

/*******************************************************************************
**
**  gckGALDEVICE_Destroy
**
**  Class destructor.
**
**  INPUT:
**
**      Nothing.
**
**  OUTPUT:
**
**      Nothing.
**
**  RETURNS:
**
**      Nothing.
*/
gceSTATUS
gckGALDEVICE_Destroy(
    gckGALDEVICE Device)
{
    gctINT i;

    gcmkHEADER_ARG("Device=0x%x", Device);

    gcmkTRACE(gcvLEVEL_VERBOSE, "[ENTER] gckGALDEVICE_Destroy");

    if (Device != gcvNULL)
    {
        for (i = 0; i < gcdMAX_GPU_COUNT; i++)
        {
            if (Device->kernels[i] != gcvNULL)
            {
                Device->kernels[i] = gcvNULL;
            }
        }

        if (Device->internalVidMem != gcvNULL)
        {
            /* destroy the internal heap */
            gcmkVERIFY_OK(gckVIDMEM_Destroy(Device->internalVidMem));

            /* unmap the internal memory */
            munmap_device_io((uintptr_t)Device->internalLogical, Device->internalSize);
        }

        if (Device->externalVidMem != gcvNULL)
        {
            /* destroy the internal heap */
            gcmkVERIFY_OK(gckVIDMEM_Destroy(Device->externalVidMem));

            /* unmap the external memory */
            munmap_device_io((uintptr_t)Device->externalLogical, Device->externalSize);
        }

        if (Device->contiguousVidMem != gcvNULL)
        {
            /* Destroy the contiguous heap */
            gcmkVERIFY_OK(gckVIDMEM_Destroy(Device->contiguousVidMem));

            if (Device->contiguousMapped)
            {
                gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_DRIVER,
                        "[galcore] gckGALDEVICE_Destroy: "
                        "Unmapping contiguous memory->0x%08lX",
                        Device->contiguousLogical);

                munmap_device_io((uintptr_t)Device->contiguousLogical, Device->contiguousSize);
            }
            else if (Device->contiguousLogical != gcvNULL)
            {
                gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_DRIVER,
                        "[galcore] gckGALDEVICE_Destroy: "
                        "Freeing contiguous memory->0x%08lX",
                        Device->contiguousLogical);

                gcmkVERIFY_OK(_FreeMemory(Device,
                            Device->contiguousLogical,
                            Device->contiguousPhysical));
            }
        }

        for (i = 0; i < gcdMAX_GPU_COUNT; i++)
        {
            if (Device->registerBases[i] != gcvNULL)
            {
                munmap_device_io((uintptr_t)Device->registerBases[i], Device->registerSizes[i]);
            }

            Device->registerBases[i] = 0;
            Device->registerSizes[i] = 0;
            Device->requestedRegisterMemBases[i] = 0;
            Device->requestedRegisterMemSizes[i] = 0;
        }

        if (Device->device)
        {
            gcmkVERIFY_OK(gckDEVICE_Destroy(Device->os, Device->device));
            Device->device = gcvNULL;
        }

        /* Destroy the gckOS object. */
        gcmkVERIFY_OK(gckOS_Destroy(Device->os));

        gckDEBUGFS_Terminate();

        free(Device);
    }

    gcmkTRACE(gcvLEVEL_VERBOSE, "[galcore] Leave gckGALDEVICE_Destroy");

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}
/*******************************************************************************
**
**  gckGALDEVICE_Setup_ISR
**
**  Start the ISR routine.
**
**  INPUT:
**
**      gckGALDEVICE Device
**          Pointer to an gckGALDEVICE object.
**
**  OUTPUT:
**
**      Nothing.
**
**  RETURNS:
**
**      gcvSTATUS_OK
**          Setup successfully.
**      gcvSTATUS_GENERIC_IO
**          Setup failed.
*/
gceSTATUS
gckGALDEVICE_Setup_ISR(
    IN gckGALDEVICE Device
    )
{
    return gcvSTATUS_OK;
}

gceSTATUS
gckGALDEVICE_Setup_ISR_2D(
    IN gckGALDEVICE Device
    )
{
    return gcvSTATUS_OK;
}

gceSTATUS
gckGALDEVICE_Setup_ISR_VG(
    IN gckGALDEVICE Device
    )
{
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckGALDEVICE_Release_ISR
**
**  Release the irq line.
**
**  INPUT:
**
**      gckGALDEVICE Device
**          Pointer to an gckGALDEVICE object.
**
**  OUTPUT:
**
**      Nothing.
**
**  RETURNS:
**
**      Nothing.
*/
gceSTATUS
gckGALDEVICE_Release_ISR(
    IN gckGALDEVICE Device
    )
{
    gcmkHEADER_ARG("Device=0x%x", Device);

    gcmkVERIFY_ARGUMENT(Device != NULL);

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
gckGALDEVICE_Release_ISR_2D(
    IN gckGALDEVICE Device
    )
{
    gcmkHEADER_ARG("Device=0x%x", Device);

    gcmkVERIFY_ARGUMENT(Device != NULL);

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
gckGALDEVICE_Release_ISR_VG(
    IN gckGALDEVICE Device
    )
{
    gcmkHEADER_ARG("Device=0x%x", Device);

    gcmkVERIFY_ARGUMENT(Device != NULL);

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckGALDEVICE_Start_Threads
**
**  Start the daemon threads.
**
**  INPUT:
**
**      gckGALDEVICE Device
**          Pointer to an gckGALDEVICE object.
**
**  OUTPUT:
**
**      Nothing.
**
**  RETURNS:
**
**      gcvSTATUS_OK
**          Start successfully.
**      gcvSTATUS_GENERIC_IO
**          Start failed.
*/
gceSTATUS
gckGALDEVICE_Start_Threads(
    IN gckGALDEVICE Device
    )
{
    gceSTATUS status;
    gctINT ret;

    gcmkHEADER_ARG("Device=0x%x", Device);

    gcmkVERIFY_ARGUMENT(Device != NULL);

    gcmkTRACE_ZONE(gcvLEVEL_INFO,
                  gcvZONE_DRIVER,
                  "[galcore] gckGALDEVICE_Start_Threads: Creating threadRoutine\n");

    if (Device->kernels[gcvCORE_MAJOR] != gcvNULL)
    {
        /* Start the interrupt service thread */
        if ((ret = pthread_create(&Device->threadCtxts[gcvCORE_MAJOR], gcvNULL, threadRoutine, Device)) != 0)
        {
            gcmkTRACE_ZONE(gcvLEVEL_ERROR,
                    gcvZONE_DRIVER,
                    "[galcore] gckGALDEVICE_Start_Threads: Failed with code %d\n",
                    ret);

            gcmkONERROR(gcvSTATUS_OUT_OF_RESOURCES);
        }

        pthread_setname_np(Device->threadCtxts[gcvCORE_MAJOR], "galcore-IST");

        Device->threadInitializeds[gcvCORE_MAJOR] = gcvTRUE;
        gcmkTRACE_ZONE(gcvLEVEL_INFO,
                gcvZONE_DRIVER,
                "[galcore] gckGALDEVICE_Start_Threads: "
                "Start the daemon thread.");
    }

#ifdef gcdDUAL_CORE
    if (Device->kernels[gcvCORE_3D1] != gcvNULL)
    {
        /* Start the interrupt service thread */
        if ((ret = pthread_create(&Device->threadCtxts[gcvCORE_3D1], gcvNULL, threadRoutine3D1, Device)) != 0)
        {
            gcmkTRACE_ZONE(gcvLEVEL_ERROR,
                    gcvZONE_DRIVER,
                    "[galcore] gckGALDEVICE_Start_Threads: Core 3D1: Failed with code %d\n",
                    ret);

            gcmkONERROR(gcvSTATUS_OUT_OF_RESOURCES);
        }

        pthread_setname_np(Device->threadCtxts[gcvCORE_3D1], "galcore-IST-3D1");

        Device->threadInitializeds[gcvCORE_3D1] = gcvTRUE;
        gcmkTRACE_ZONE(gcvLEVEL_INFO,
                gcvZONE_DRIVER,
                "[galcore] gckGALDEVICE_Start_Threads: Core 3D1:"
                "Start the daemon thread.");
    }
#endif

    if (Device->kernels[gcvCORE_2D] != gcvNULL)
    {
        /* Start the interrupt service thread */
        if ((ret = pthread_create(&Device->threadCtxts[gcvCORE_2D], gcvNULL, threadRoutine2D, Device)) != 0)
        {
            gcmkTRACE_ZONE(gcvLEVEL_ERROR,
                    gcvZONE_DRIVER,
                    "[galcore] gckGALDEVICE_Start_Threads: Failed with code %d\n",
                    ret);

            gcmkONERROR(gcvSTATUS_OUT_OF_RESOURCES);
        }

        pthread_setname_np(Device->threadCtxts[gcvCORE_2D], "galcore-IST-2D");

        Device->threadInitializeds[gcvCORE_2D] = gcvTRUE;
        gcmkTRACE_ZONE(gcvLEVEL_INFO,
                gcvZONE_DRIVER,
                "[galcore] gckGALDEVICE_Start_Threads: "
                "Start the daemon thread.");
    }
    else
    {
        Device->threadInitializeds[gcvCORE_2D]  = gcvFALSE;
    }

#if gcdENABLE_VG
    if (Device->kernels[gcvCORE_VG] != gcvNULL)
    {
        /* Start the interrupt service thread */
        if ((ret = pthread_create(&Device->threadCtxts[gcvCORE_VG], gcvNULL, threadRoutineVG, Device)) != 0)
        {
            gcmkTRACE_ZONE(gcvLEVEL_ERROR,
                    gcvZONE_DRIVER,
                    "[galcore] gckGALDEVICE_Start_Threads: Failed with code %d\n",
                    ret);

            gcmkONERROR(gcvSTATUS_OUT_OF_RESOURCES);
        }

        pthread_setname_np(Device->threadCtxts[gcvCORE_VG], "galcore-IST-VG");

        Device->threadInitializeds[gcvCORE_VG] = gcvTRUE;
        gcmkTRACE_ZONE(gcvLEVEL_INFO,
                gcvZONE_DRIVER,
                "[galcore] gckGALDEVICE_Start_Threads: "
                "Start the daemon thread.");
    }
    else
#endif /* gcdENABLE_VG */
    {
        Device->threadInitializeds[gcvCORE_VG]  = gcvFALSE;
    }

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckGALDEVICE_Stop_Threads
**
**  Stop the gal device, including the following actions: stop the daemon
**  thread, release the irq.
**
**  INPUT:
**
**      gckGALDEVICE Device
**          Pointer to an gckGALDEVICE object.
**
**  OUTPUT:
**
**      Nothing.
**
**  RETURNS:
**
**      Nothing.
*/
gceSTATUS
gckGALDEVICE_Stop_Threads(
    gckGALDEVICE Device
    )
{
    gctINT i;

    gcmkHEADER_ARG("Device=0x%x", Device);

    gcmkVERIFY_ARGUMENT(Device != NULL);

    /* Stop the kernel threads. */
    for (i = 0; i < gcdMAX_GPU_COUNT; i++)
    {
        if (Device->threadInitializeds[i])
        {
            InterruptDetach(Device->irqIds[i]);
            Device->irqIds[i] = 0;
            ConnectDetach(Device->coids[i]);
            Device->coids[i] = 0;
            ChannelDestroy(Device->chids[i]);
            Device->chids[i] = 0;

            pthread_cancel(Device->threadCtxts[i]);
            pthread_join(Device->threadCtxts[i], NULL);
            Device->threadCtxts[i] = 0;

            Device->threadInitializeds[i] = gcvFALSE;
            Device->isrInitializeds[i] = gcvFALSE;
        }
    }

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckGALDEVICE_Start
**
**  Start the gal device, including the following actions: setup the isr routine
**  and start the daemon thread.
**
**  INPUT:
**
**      gckGALDEVICE Device
**          Pointer to an gckGALDEVICE object.
**
**  OUTPUT:
**
**      Nothing.
**
**  RETURNS:
**
**      gcvSTATUS_OK
**          Start successfully.
*/
gceSTATUS
gckGALDEVICE_Start(
    IN gckGALDEVICE Device
    )
{
    gceSTATUS status;

    gcmkHEADER_ARG("Device=0x%x", Device);

    /* Start the daemon thread. */
    gcmkVERIFY_OK((status = gckGALDEVICE_Start_Threads(Device)));

    if (Device->kernels[gcvCORE_MAJOR] != gcvNULL)
    {
        /* Switch to SUSPEND power state. */
        gcmkONERROR(gckHARDWARE_SetPowerManagementState(
            Device->kernels[gcvCORE_MAJOR]->hardware, gcvPOWER_OFF_BROADCAST
            ));
    }
#ifdef gcdDUAL_CORE
    if (Device->kernels[gcvCORE_3D1] != gcvNULL)
    {
        /* Switch to SUSPEND power state. */
        gcmkONERROR(gckHARDWARE_SetPowerManagementState(
            Device->kernels[gcvCORE_3D1]->hardware, gcvPOWER_OFF_BROADCAST
            ));
    }
#endif

    if (Device->kernels[gcvCORE_2D] != gcvNULL)
    {
        /* Switch to SUSPEND power state. */
        gcmkONERROR(gckHARDWARE_SetPowerManagementState(
            Device->kernels[gcvCORE_2D]->hardware, gcvPOWER_OFF_BROADCAST
            ));
    }

    if (Device->kernels[gcvCORE_VG] != gcvNULL)
    {
#if gcdENABLE_VG
        /* Switch to SUSPEND power state. */
        gcmkONERROR(gckVGHARDWARE_SetPowerManagementState(
            Device->kernels[gcvCORE_VG]->vg->hardware, gcvPOWER_OFF_BROADCAST
            ));
#endif
    }

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckGALDEVICE_Stop
**
**  Stop the gal device, including the following actions: stop the daemon
**  thread, release the irq.
**
**  INPUT:
**
**      gckGALDEVICE Device
**          Pointer to an gckGALDEVICE object.
**
**  OUTPUT:
**
**      Nothing.
**
**  RETURNS:
**
**      Nothing.
*/
gceSTATUS
gckGALDEVICE_Stop(
    gckGALDEVICE Device
    )
{
    gceSTATUS status;

    gcmkHEADER_ARG("Device=0x%x", Device);

    gcmkVERIFY_ARGUMENT(Device != NULL);

    if (Device->kernels[gcvCORE_MAJOR] != gcvNULL)
    {
        gcmkONERROR(gckHARDWARE_SetPowerManagement(
            Device->kernels[gcvCORE_MAJOR]->hardware, gcvTRUE
            ));

        /* Switch to OFF power state. */
        gcmkONERROR(gckHARDWARE_SetPowerManagementState(
            Device->kernels[gcvCORE_MAJOR]->hardware, gcvPOWER_OFF
            ));
    }

#ifdef gcdDUAL_CORE
    if (Device->kernels[gcvCORE_3D1] != gcvNULL)
    {
        gcmkONERROR(gckHARDWARE_SetPowerManagement(
            Device->kernels[gcvCORE_3D1]->hardware, gcvTRUE
            ));

        /* Switch to OFF power state. */
        gcmkONERROR(gckHARDWARE_SetPowerManagementState(
            Device->kernels[gcvCORE_3D1]->hardware, gcvPOWER_OFF
            ));
    }
#endif

    if (Device->kernels[gcvCORE_2D] != gcvNULL)
    {
        /* Switch to OFF power state. */
        gcmkONERROR(gckHARDWARE_SetPowerManagementState(
            Device->kernels[gcvCORE_2D]->hardware, gcvPOWER_OFF
            ));
    }

    if (Device->kernels[gcvCORE_VG] != gcvNULL)
    {
#if gcdENABLE_VG
        /* Switch to OFF power state. */
        gcmkONERROR(gckVGHARDWARE_SetPowerManagementState(
            Device->kernels[gcvCORE_VG]->vg->hardware, gcvPOWER_OFF
            ));
#endif
    }

    gckGALDEVICE_Stop_Threads(Device);

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmkFOOTER();
    return status;
}
