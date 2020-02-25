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


#include "gc_hal.h"
#include "gc_hal_kernel.h"
#include "gc_hal_kernel_hardware.h"

#define _GC_OBJ_ZONE    gcvZONE_HARDWARE

#define gcmSEMAPHORESTALL(buffer) \
        do \
        { \
            /* Arm the PE-FE Semaphore. */ \
            *buffer++ \
                = gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE, LOAD_STATE) \
                | gcmSETFIELD     (0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT, 1) \
                | gcmSETFIELD     (0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, 0x0E02); \
            \
            *buffer++ \
                = gcmSETFIELDVALUE(0, AQ_SEMAPHORE, SOURCE, FRONT_END) \
                | gcmSETFIELDVALUE(0, AQ_SEMAPHORE, DESTINATION, PIXEL_ENGINE);\
            \
            /* STALL FE until PE is done flushing. */ \
            *buffer++ \
                = gcmSETFIELDVALUE(0, STALL_COMMAND, OPCODE, STALL); \
            \
            *buffer++ \
                = gcmSETFIELDVALUE(0, STALL_STALL, SOURCE, FRONT_END) \
                | gcmSETFIELDVALUE(0, STALL_STALL, DESTINATION, PIXEL_ENGINE); \
        } while(0)

static gceSTATUS
_FuncExecute(IN gcsFUNCTION_EXECUTION_PTR Execution)
{
    return gckHARDWARE_ExecuteFunctions(Execution);
}

static gceSTATUS
_FuncValidate_MMU(IN gcsFUNCTION_EXECUTION_PTR Execution)
{
    gckHARDWARE hardware = (gckHARDWARE)Execution->hardware;

    if ((hardware->mmuVersion > 0) &&
        hardware->options.enableMMU &&
        (hardware->options.secureMode != gcvSECURE_IN_TA))
    {
        Execution->valid = gcvTRUE;
    }
    else
    {
        Execution->valid = gcvFALSE;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_FuncRelease_MMU(IN gcsFUNCTION_EXECUTION_PTR Execution)
{
    gckHARDWARE hardware = (gckHARDWARE)Execution->hardware;

    if (Execution->funcVidMem)
    {
        if (Execution->logical)
        {
            gcmkVERIFY_OK(gckVIDMEM_NODE_UnlockCPU(
                        hardware->kernel,
                        Execution->funcVidMem,
                        0,
                        gcvFALSE,
                        gcvFALSE
                        ));
            Execution->logical = gcvNULL;
        }

        gcmkVERIFY_OK(gckVIDMEM_NODE_Dereference(
                    hardware->kernel,
                    Execution->funcVidMem
                    ));
        Execution->funcVidMem = gcvNULL;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_ProgramMMUStates(
    IN gckHARDWARE Hardware,
    IN gckMMU Mmu,
    IN gceMMU_MODE Mode,
    IN gctPOINTER Logical,
    IN OUT gctUINT32 * Bytes
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 config, address;
    gctUINT32 extMtlb, extSafeAddress, configEx = 0;
    gctPHYS_ADDR_T physical;
    gctUINT32_PTR buffer;
    gctBOOL ace;
    gctUINT32 reserveBytes = 0;

    gctBOOL config2D;

    gcmkHEADER_ARG("Hardware=0x%x", Hardware);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmkVERIFY_ARGUMENT(Hardware->mmuVersion != 0);

    ace = gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_ACE);

    switch (Hardware->options.secureMode)
    {
    case gcvSECURE_IN_NORMAL:
        reserveBytes = 8 + 4 * 4;
        break;
    case gcvSECURE_NONE:
        reserveBytes = 16 + 4 * 4;
        if (ace)
        {
            reserveBytes += 8;
        }
        break;
    case gcvSECURE_IN_TA:
    default:
        gcmkASSERT(gcvFALSE);
        gcmkPRINT("%s(%d): secureMode is wrong", __FUNCTION__, __LINE__);
        break;
    }

    config2D =  gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_PIPE_3D)
             && gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_PIPE_2D);

    if (config2D)
    {
        reserveBytes +=
            /* Pipe Select. */
            4 * 4
            /* Configure MMU States. */
          + 4 * 4
            /* Semaphore stall */
          + 4 * 8;

        if (ace)
        {
            reserveBytes += 8;
        }
    }

    reserveBytes += 8;

    physical = Mmu->mtlbPhysical;

    config  = (gctUINT32)(physical & 0xFFFFFFFF);
    extMtlb = (gctUINT32)(physical >> 32);

    /* more than 40bit physical address */
    if (extMtlb & 0xFFFFFF00)
    {
        gcmkONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    physical = Mmu->safePagePhysical;

    address = (gctUINT32)(physical & 0xFFFFFFFF);
    extSafeAddress = (gctUINT32)(physical >> 32);

    if (address & 0x3F)
    {
        gcmkONERROR(gcvSTATUS_NOT_ALIGNED);
    }

    /* more than 40bit physical address */
    if (extSafeAddress & 0xFFFFFF00)
    {
        gcmkONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    if (ace)
    {
        configEx = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:0) - (0 ?
 7:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:0) - (0 ?
 7:0) + 1))))))) << (0 ?
 7:0))) | (((gctUINT32) ((gctUINT32) (extSafeAddress) & ((gctUINT32) ((((1 ?
 7:0) - (0 ?
 7:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ? 7:0)))
                 | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:16) - (0 ?
 23:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:16) - (0 ?
 23:16) + 1))))))) << (0 ?
 23:16))) | (((gctUINT32) ((gctUINT32) (extMtlb) & ((gctUINT32) ((((1 ?
 23:16) - (0 ?
 23:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:16) - (0 ? 23:16) + 1))))))) << (0 ? 23:16)));
    }

    switch (Mode)
    {
    case gcvMMU_MODE_1K:
        if (config & 0x3FF)
        {
            gcmkONERROR(gcvSTATUS_NOT_ALIGNED);
        }

        config |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

        break;

    case gcvMMU_MODE_4K:
        if (config & 0xFFF)
        {
            gcmkONERROR(gcvSTATUS_NOT_ALIGNED);
        }

        config |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

        break;

    default:
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (Logical != gcvNULL)
    {
        buffer = (gctUINT32_PTR)Logical;

        if (Hardware->options.secureMode == gcvSECURE_IN_NORMAL)
        {
            gcsMMU_TABLE_ARRAY_ENTRY * entry;
            entry = (gcsMMU_TABLE_ARRAY_ENTRY *) Hardware->pagetableArray.logical;

            /* Setup page table array entry. */
            if (Hardware->bigEndian)
            {
                entry->low = gcmBSWAP32(config);
                entry->high = gcmBSWAP32(extMtlb);
            }
            else
            {
                entry->low = config;
                entry->high = extMtlb;
            }

            gcmkDUMP(Mmu->os, "#[mmu: page table array]");

            gcmkDUMP(Mmu->os, "@[physical.fill 0x%010llX 0x%08X 0x%08X]",
                     (unsigned long long)Hardware->pagetableArray.address, entry->low, 4);

            gcmkDUMP(Mmu->os, "@[physical.fill 0x%010llX 0x%08X 0x%08X]",
                     (unsigned long long)Hardware->pagetableArray.address + 4, entry->high, 4);

            gcmkVERIFY_OK(gckVIDMEM_NODE_CleanCache(
                Hardware->kernel,
                Hardware->pagetableArray.videoMem,
                0,
                entry,
                8
                ));

            /* Setup command buffer to load index 0 of page table array. */
            *buffer++
                = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x006B) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));

            *buffer++
                = (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16))));
        }
        else
        {
            gcmkASSERT(Hardware->options.secureMode == gcvSECURE_NONE);

            *buffer++
                = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0061) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));

            *buffer++ = config;

            *buffer++
                = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0060) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));

            *buffer++ = address;

            if (ace)
            {
                *buffer++
                    = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0068) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));

                *buffer++
                    = configEx;
            }
        }

        *buffer++
            = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E12) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));

        *buffer++
            = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)));

        do{*buffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E02) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))); *buffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))); *buffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))); *buffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)));} while(0);
;


        if (config2D)
        {
            /* LoadState(AQPipeSelect, 1), pipe. */
            *buffer++
                = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E00) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));

            *buffer++ = 0x1;

            *buffer++
                = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0061) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));

            *buffer++ = config;

            *buffer++
                = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0060) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));

            *buffer++ = address;

            if (ace)
            {
                *buffer++
                    = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0068) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));

                *buffer++
                    = configEx;
            }

            do{*buffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E02) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))); *buffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))); *buffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))); *buffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)));} while(0);
;


            /* LoadState(AQPipeSelect, 1), pipe. */
            *buffer++
                = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E00) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));

            *buffer++ = 0x0;

            do{*buffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E02) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))); *buffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))); *buffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))); *buffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)));} while(0);
;

        }

    }

    if (Bytes != gcvNULL)
    {
        *Bytes = reserveBytes;
    }

    /* Return the status. */
    gcmkFOOTER_NO();
    return status;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

static gceSTATUS
_ProgramMMUStatesMCFE(
    IN gckHARDWARE Hardware,
    IN gckMMU Mmu,
    IN gceMMU_MODE Mode,
    IN gctPOINTER Logical,
    IN OUT gctUINT32 * Bytes
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 config, address;
    gctUINT32 extMtlb, extSafeAddress, configEx = 0;
    gctPHYS_ADDR_T physical;
    gctUINT32_PTR buffer;
    gctBOOL ace;
    gctUINT32 reserveBytes = 0;

    gcmkHEADER_ARG("Hardware=0x%x", Hardware);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmkVERIFY_ARGUMENT(Hardware->mmuVersion != 0);

    ace = gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_ACE);

    switch (Hardware->options.secureMode)
    {
    case gcvSECURE_IN_NORMAL:
        reserveBytes = 8;
        reserveBytes += 8;
        break;
    case gcvSECURE_NONE:
        reserveBytes = 16;;
        if (ace)
        {
            reserveBytes += 8;
            reserveBytes += 8;
        }
        break;
    case gcvSECURE_IN_TA:
    default:
        gcmkASSERT(gcvFALSE);
        gcmkPRINT("%s(%d): secureMode is wrong", __FUNCTION__, __LINE__);
        break;
    }

    physical = Mmu->mtlbPhysical;

    config  = (gctUINT32)(physical & 0xFFFFFFFF);
    extMtlb = (gctUINT32)(physical >> 32);

    /* more than 40bit physical address */
    if (extMtlb & 0xFFFFFF00)
    {
        gcmkONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    physical = Mmu->safePagePhysical;

    address = (gctUINT32)(physical & 0xFFFFFFFF);
    extSafeAddress = (gctUINT32)(physical >> 32);

    if (address & 0x3F)
    {
        gcmkONERROR(gcvSTATUS_NOT_ALIGNED);
    }

    /* more than 40bit physical address */
    if (extSafeAddress & 0xFFFFFF00)
    {
        gcmkONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    if (ace)
    {
        configEx = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:0) - (0 ?
 7:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:0) - (0 ?
 7:0) + 1))))))) << (0 ?
 7:0))) | (((gctUINT32) ((gctUINT32) (extSafeAddress) & ((gctUINT32) ((((1 ?
 7:0) - (0 ?
 7:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ? 7:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:16) - (0 ?
 23:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:16) - (0 ?
 23:16) + 1))))))) << (0 ?
 23:16))) | (((gctUINT32) ((gctUINT32) (extMtlb) & ((gctUINT32) ((((1 ?
 23:16) - (0 ?
 23:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:16) - (0 ? 23:16) + 1))))))) << (0 ? 23:16)));
    }

    switch (Mode)
    {
    case gcvMMU_MODE_1K:
        if (config & 0x3FF)
        {
            gcmkONERROR(gcvSTATUS_NOT_ALIGNED);
        }

        config |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

        break;

    case gcvMMU_MODE_4K:
        if (config & 0xFFF)
        {
            gcmkONERROR(gcvSTATUS_NOT_ALIGNED);
        }

        config |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

        break;

    default:
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (Logical != gcvNULL)
    {
        buffer = Logical;

        if (Hardware->options.secureMode == gcvSECURE_IN_NORMAL)
        {
            gcsMMU_TABLE_ARRAY_ENTRY * entry;
            entry = (gcsMMU_TABLE_ARRAY_ENTRY *) Hardware->pagetableArray.logical;

            /* Setup page table array entry. */
            if (Hardware->bigEndian)
            {
                entry->low = gcmBSWAP32(config);
                entry->high = gcmBSWAP32(extMtlb);
            }
            else
            {
                entry->low = config;
                entry->high = extMtlb;
            }

            gcmkDUMP(Mmu->os, "#[mmu: page table array]");

            gcmkDUMP(Mmu->os, "@[physical.fill 0x%010llX 0x%08X 0x%08X]",
                     (unsigned long long)Hardware->pagetableArray.address, entry->low, 4);

            gcmkDUMP(Mmu->os, "@[physical.fill 0x%010llX 0x%08X 0x%08X]",
                     (unsigned long long)Hardware->pagetableArray.address + 4, entry->high, 4);

            gcmkVERIFY_OK(gckVIDMEM_NODE_CleanCache(
                Hardware->kernel,
                Hardware->pagetableArray.videoMem,
                0,
                entry,
                8
                ));

            /* Setup command buffer to load index 0 of page table array. */
            *buffer++
                = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x006B) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));

            *buffer++
                = (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16))));

            *buffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x03 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));
            *buffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x03 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));
        }
        else
        {
            gcmkASSERT(Hardware->options.secureMode == gcvSECURE_NONE);

            *buffer++
                = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0061) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));

            *buffer++ = config;

            *buffer++
                = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0060) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));

            *buffer++ = address;

            if (ace)
            {
                *buffer++
                    = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0068) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));

                *buffer++ = configEx;

                *buffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x03 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));
                *buffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x03 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));
            }
        }
    }

    if (Bytes != gcvNULL)
    {
        *Bytes = reserveBytes;
    }

    /* Return the status. */
    gcmkFOOTER_NO();
    return status;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

static gceSTATUS
_FuncInit_MMU(IN gcsFUNCTION_EXECUTION_PTR Execution)
{
    gceSTATUS status;
    gctUINT32 mmuBytes = 0;
    gctUINT32 tailBytes;
    gctUINT32 flags = gcvALLOC_FLAG_CONTIGUOUS;
    gceMMU_MODE mode;
    gcePOOL pool;
    gctPHYS_ADDR_T physical;
    gckHARDWARE hardware = (gckHARDWARE)Execution->hardware;

#if gcdENABLE_MMU_1KMODE
    mode = gcvMMU_MODE_1K;
#else
    mode = gcvMMU_MODE_4K;
#endif

    flags |= gcvALLOC_FLAG_4GB_ADDR;

#if gcdENABLE_CACHEABLE_COMMAND_BUFFER
    flags |= gcvALLOC_FLAG_CACHEABLE;
#endif

    pool = gcvPOOL_DEFAULT;

    Execution->funcVidMemBytes = 1024;
    /* Allocate mmu command buffer within 32bit space */
    gcmkONERROR(gckKERNEL_AllocateVideoMemory(
                hardware->kernel,
                64,
                gcvVIDMEM_TYPE_COMMAND,
                flags,
                &Execution->funcVidMemBytes,
                &pool,
                &Execution->funcVidMem
                ));

    /* Lock for kernel side CPU access. */
    gcmkONERROR(gckVIDMEM_NODE_LockCPU(
        hardware->kernel,
        Execution->funcVidMem,
        gcvFALSE,
        gcvFALSE,
        &Execution->logical
        ));

    /* Get CPU physical address. */
    gcmkONERROR(gckVIDMEM_NODE_GetPhysical(
        hardware->kernel,
        Execution->funcVidMem,
        0,
        &physical
        ));

    /* Convert to GPU physical address. */
    gcmkVERIFY_OK(gckOS_CPUPhysicalToGPUPhysical(
        hardware->os,
        physical,
        &physical
        ));

    if (!(flags & gcvALLOC_FLAG_4GB_ADDR) && (physical & 0xFFFFFFFF00000000ULL))
    {
        gcmkFATAL("%s(%d): Command buffer physical address (0x%llx) for MMU setup exceeds 32bits, "
                  "please rebuild kernel with CONFIG_ZONE_DMA32=y.",
                  __FUNCTION__, __LINE__, physical);
    }

    gcmkSAFECASTPHYSADDRT(Execution->address, physical);

    if (hardware->mcFE)
    {
        gcmkONERROR(_ProgramMMUStatesMCFE(
            hardware,
            hardware->kernel->mmu,
            mode,
            Execution->logical,
            &mmuBytes
            ));
    }
    else
    {
        gcmkONERROR(_ProgramMMUStates(
            hardware,
            hardware->kernel->mmu,
            mode,
            Execution->logical,
            &mmuBytes
            ));
    }

    Execution->endAddress = Execution->address + mmuBytes;
    Execution->endLogical = (gctUINT8_PTR)Execution->logical + mmuBytes;

    if (hardware->wlFE)
    {
        tailBytes = (gctUINT32)(Execution->funcVidMemBytes - mmuBytes);
        gcmkONERROR(gckWLFE_End(hardware, Execution->endLogical, Execution->endAddress, &tailBytes));
    }
    else
    {
        tailBytes = 0;
    }

    Execution->bytes = mmuBytes + tailBytes;

    gcmkONERROR(gckVIDMEM_NODE_CleanCache(
        hardware->kernel,
        Execution->funcVidMem,
        0,
        Execution->logical,
        Execution->bytes
        ));

    return gcvSTATUS_OK;

OnError:
    _FuncRelease_MMU(Execution);

    return status;
}

static gceSTATUS
_FuncExecute_MMU(IN gcsFUNCTION_EXECUTION_PTR Execution)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 address = 0;
    gctUINT32 idle;
    gctUINT32 timer = 0, delay = 1;
    gckHARDWARE hardware = (gckHARDWARE)Execution->hardware;
    gckMMU mmu = hardware->kernel->mmu;

    /* Prepared command sequence contains an END,
    ** so update lastEnd and store executeCount to END command.
    */
    gctUINT32_PTR endLogical = (gctUINT32_PTR)Execution->endLogical;

    hardware->lastEnd = Execution->endAddress;

    if (hardware->wlFE)
    {
        /* Append a executeCount in End command, MCFE does not support such End command. */
        *(endLogical + 1) = hardware->executeCount + 1;
    }

    if (hardware->options.secureMode == gcvSECURE_IN_NORMAL)
    {
        gctUINT32 extSafeAddress;
        /* Set up base address of page table array. */
        gcmkONERROR(gckOS_WriteRegisterEx(
            hardware->os,
            hardware->core,
            0x0038C,
            (gctUINT32)(hardware->pagetableArray.address & 0xFFFFFFFF)
            ));

        gcmkONERROR(gckOS_WriteRegisterEx(
            hardware->os,
            hardware->core,
            0x00390,
            (gctUINT32)((hardware->pagetableArray.address >> 32) & 0xFFFFFFFF)
            ));

        gcmkONERROR(gckOS_WriteRegisterEx(
            hardware->os,
            hardware->core,
            0x00394,
            1
            ));

        address = (gctUINT32)(mmu->safePagePhysical & 0xFFFFFFFF);
        extSafeAddress = (gctUINT32)(mmu->safePagePhysical >> 32);

        if (address & 0x3F)
        {
            gcmkONERROR(gcvSTATUS_NOT_ALIGNED);
        }

        /* more than 40bit physical address */
        if (extSafeAddress & 0xFFFFFF00)
        {
            gcmkONERROR(gcvSTATUS_NOT_SUPPORTED);
        }

        gcmkONERROR(gckOS_WriteRegisterEx(
            hardware->os,
            hardware->core,
            0x0039C,
            address
            ));

        gcmkONERROR(gckOS_WriteRegisterEx(
            hardware->os,
            hardware->core,
            0x00398,
            address
            ));

        gcmkONERROR(gckOS_WriteRegisterEx(
            hardware->os,
            hardware->core,
            0x003A0,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:16) - (0 ?
 23:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:16) - (0 ?
 23:16) + 1))))))) << (0 ?
 23:16))) | (((gctUINT32) ((gctUINT32) ((gctUINT32)extSafeAddress) & ((gctUINT32) ((((1 ?
 23:16) - (0 ?
 23:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:16) - (0 ? 23:16) + 1))))))) << (0 ? 23:16)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:31) - (0 ?
 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:0) - (0 ?
 7:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:0) - (0 ?
 7:0) + 1))))))) << (0 ?
 7:0))) | (((gctUINT32) ((gctUINT32) ((gctUINT32)extSafeAddress) & ((gctUINT32) ((((1 ?
 7:0) - (0 ?
 7:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ? 7:0)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:15) - (0 ?
 15:15) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:15) - (0 ?
 15:15) + 1))))))) << (0 ?
 15:15))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 15:15) - (0 ?
 15:15) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:15) - (0 ? 15:15) + 1))))))) << (0 ? 15:15)))
            ));
    }

    gckFUNCTION_Dump(Execution);

    /* Execute prepared command sequence. */
    if (hardware->mcFE)
    {
        gcmkONERROR(gckMCFE_Execute(
            hardware,
            gcvFALSE,
            0,
            Execution->address,
            Execution->bytes
            ));
    }
    else
    {
        gcmkONERROR(gckWLFE_Execute(
            hardware,
            Execution->address,
            Execution->bytes
            ));
    }

#if gcdLINK_QUEUE_SIZE
    {
        gcuQUEUEDATA data;

        gcmkVERIFY_OK(gckOS_GetProcessID(&data.linkData.pid));

        data.linkData.start    = Execution->address;
        data.linkData.end      = Execution->address + Execution->bytes;
        data.linkData.linkLow  = 0;
        data.linkData.linkHigh = 0;

        gckQUEUE_Enqueue(&hardware->linkQueue, &data);
    }
#endif

    /* Wait until MMU configure finishes. */
    do
    {
        gckOS_Delay(hardware->os, delay);

        gcmkONERROR(gckOS_ReadRegisterEx(
            hardware->os,
            hardware->core,
            0x00004,
            &idle));

        timer += delay;
        delay *= 2;

#if gcdGPU_TIMEOUT
        if (timer >= hardware->kernel->timeOut)
        {
            gckHARDWARE_DumpGPUState(hardware);

            if (hardware->kernel->command)
            {
                gckCOMMAND_DumpExecutingBuffer(hardware->kernel->command);
            }

            /* Even if hardware is not reset correctly, let software
            ** continue to avoid software stuck. Software will timeout again
            ** and try to recover GPU in next timeout.
            */
            gcmkONERROR(gcvSTATUS_DEVICE);
        }
#endif
    }
    while (!(((((gctUINT32) (idle)) >> (0 ? 0:0)) & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1)))))) ));

    gcmkDUMP(hardware->os, "@[register.wait 0x%05X 0x%08X 0x%08X]",
             0x00004,
             ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (~0U) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))),
             idle);

OnError:
    return status;
}

static gceSTATUS
_FuncValidate_Flush(IN gcsFUNCTION_EXECUTION_PTR Execution)
{
    Execution->valid = gcvTRUE;

    return gcvSTATUS_OK;
}

static gceSTATUS
_FuncRelease_Flush(IN gcsFUNCTION_EXECUTION_PTR Execution)
{
    gckHARDWARE hardware = (gckHARDWARE)Execution->hardware;

    if (Execution->funcVidMem)
    {
        if (Execution->address)
        {
            /* Synchroneous unlock. */
            gcmkVERIFY_OK(gckVIDMEM_NODE_Unlock(
                        hardware->kernel,
                        Execution->funcVidMem,
                        0,
                        gcvNULL
                        ));
             Execution->address = 0;
        }

        if (Execution->logical)
        {
            gcmkVERIFY_OK(gckVIDMEM_NODE_UnlockCPU(
                        hardware->kernel,
                        Execution->funcVidMem,
                        0,
                        gcvFALSE,
                        gcvFALSE
                        ));
            Execution->logical = gcvNULL;
        }

        gcmkVERIFY_OK(gckVIDMEM_NODE_Dereference(
                    hardware->kernel,
                    Execution->funcVidMem
                    ));

        Execution->funcVidMem = gcvNULL;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_FuncInit_Flush(IN gcsFUNCTION_EXECUTION_PTR Execution)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 flushBytes = 0;
    gctUINT32 offset = 0;
    gctUINT32 endBytes = 0;
    gctUINT32 address;
    gctUINT32 allocFlag = 0;
    gcePOOL pool;
    gctUINT8_PTR logical;
    gckHARDWARE hardware = (gckHARDWARE)Execution->hardware;

    pool = gcvPOOL_DEFAULT;

#if gcdENABLE_CACHEABLE_COMMAND_BUFFER
    allocFlag = gcvALLOC_FLAG_CACHEABLE;
#endif

    Execution->funcVidMemBytes = 1024;
    /* Allocate video memory node for aux functions. */
    gcmkONERROR(gckKERNEL_AllocateVideoMemory(
                hardware->kernel,
                64,
                gcvVIDMEM_TYPE_COMMAND,
                allocFlag,
                &Execution->funcVidMemBytes,
                &pool,
                &Execution->funcVidMem
                ));

    /* Lock for GPU access. */
    gcmkONERROR(gckVIDMEM_NODE_Lock(
                hardware->kernel,
                Execution->funcVidMem,
                &Execution->address
                ));

    /* Lock for kernel side CPU access. */
    gcmkONERROR(gckVIDMEM_NODE_LockCPU(
                hardware->kernel,
                Execution->funcVidMem,
                gcvFALSE,
                gcvFALSE,
                &Execution->logical
                ));
    /*
    ** All cache flush command sequence.
    */
    logical = (gctUINT8_PTR)Execution->logical;
    address = Execution->address;

    /* Get the size of the flush command. */
    gcmkONERROR(gckHARDWARE_Flush(hardware, gcvFLUSH_ALL, gcvNULL, &flushBytes));

    /* Append a flush. */
    gcmkONERROR(gckHARDWARE_Flush(hardware, gcvFLUSH_ALL, logical, &flushBytes));

    offset += flushBytes;
    logical += offset;
    address += offset;

    if (hardware->wlFE)
    {
        gcmkONERROR(gckWLFE_End(hardware, gcvNULL, ~0U, &endBytes));
        gcmkONERROR(gckWLFE_End(hardware, logical, address, &endBytes));
    }

    Execution->bytes = flushBytes + endBytes;
    Execution->endAddress = Execution->address + flushBytes;
    Execution->endLogical = (gctUINT8_PTR)Execution->logical + flushBytes;

    return gcvSTATUS_OK;
OnError:
    _FuncRelease_Flush(Execution);

    return status;
}

/*******************************************************************************
**
**  gckFUNCTION_Construct
**
**  Generate command buffer snippets which will be used by gckHARDWARE, by which
**  gckHARDWARE can manipulate GPU by FE command without using gckCOMMAND to avoid
**  race condition and deadlock.
**
**  Notice:
**  1. Each snippet can only be executed when GPU is idle.
**  2. Execution is triggered by AHB (0x658)
**  3. Each snippet followed by END so software can sync with GPU by checking GPU
**     idle
**  4. It is transparent to gckCOMMAND command buffer.
**
**  Existing Snippets:
**  1. MMU Configure
**     For new MMU, after GPU is reset, FE execute this command sequence to enable MMU.
*/
gceSTATUS gckFUNCTION_Construct(IN         gctPOINTER Hardware)
{
    gceSTATUS status = gcvSTATUS_OK;
    gckHARDWARE hardware = (gckHARDWARE)Hardware;
    gctPOINTER pointer = gcvNULL;
    gctUINT i;

    gcmkHEADER_ARG("Hardware=0x%x", Hardware);
    /* Verify the arguments. */
    gcmkVERIFY_ARGUMENT(Hardware != gcvNULL);

    /* Allocate the gcsFUNCTION_EXECUTION object. */
    gcmkONERROR(gckOS_Allocate(hardware->os,
                               gcmSIZEOF(gcsFUNCTION_EXECUTION) * gcvFUNCTION_EXECUTION_NUM,
                               &pointer));

    gckOS_ZeroMemory(pointer, gcmSIZEOF(gcsFUNCTION_EXECUTION) * gcvFUNCTION_EXECUTION_NUM);

    hardware->functions = (gcsFUNCTION_EXECUTION_PTR)pointer;

    for (i = 0; i < gcvFUNCTION_EXECUTION_NUM; i++)
    {
        gcsFUNCTION_EXECUTION_PTR func = &hardware->functions[i];

        func->hardware = hardware;
        func->funcId = (gceFUNCTION_EXECUTION)i;

        /* Init functions API pointer */
        switch (i)
        {
        case gcvFUNCTION_EXECUTION_MMU:
            gckOS_MemCopy(func->funcName, "set mmu", 8);
            func->funcExecution.init = _FuncInit_MMU;
            func->funcExecution.validate = _FuncValidate_MMU;
            func->funcExecution.execute = _FuncExecute_MMU;
            func->funcExecution.release = _FuncRelease_MMU;
            break;

        case gcvFUNCTION_EXECUTION_FLUSH:
            gckOS_MemCopy(func->funcName, "flush", 6);
            func->funcExecution.init = _FuncInit_Flush;
            func->funcExecution.validate = _FuncValidate_Flush;
            func->funcExecution.execute = _FuncExecute;
            func->funcExecution.release = _FuncRelease_Flush;
            break;

        default:
            gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }

    return gcvSTATUS_OK;
OnError:
    if (pointer)
    {
        gcmkVERIFY_OK(gckOS_Free(hardware->os, pointer));
    }

    /* Return the status. */
    gcmkFOOTER();
    return status;
}
gceSTATUS gckFUNCTION_Destory(IN    gctPOINTER Hardware)
{
    gceSTATUS status = gcvSTATUS_OK;
    gckHARDWARE hardware = (gckHARDWARE)Hardware;
    gctUINT i;

    gcmkHEADER_ARG("Hardware=0x%x", Hardware);
    /* Verify the arguments. */
    gcmkVERIFY_ARGUMENT(Hardware != gcvNULL);

    for (i = 0; i < gcvFUNCTION_EXECUTION_NUM; i++)
    {
        gckFUNCTION_Release(&hardware->functions[i]);
    }

    gcmkVERIFY_OK(gckOS_Free(hardware->os, hardware->functions));
    hardware->functions = gcvNULL;

    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS gckFUNCTION_Validate(IN gcsFUNCTION_EXECUTION_PTR Execution,
                                       IN OUT gctBOOL_PTR Valid)
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

    gcmkHEADER_ARG("Execution=0x%x", Execution);
    /* Verify the arguments. */
    gcmkVERIFY_ARGUMENT(Execution != gcvNULL);
    gcmkVERIFY_ARGUMENT(Valid != gcvNULL);

    if (Execution->funcExecution.validate)
    {
        status = Execution->funcExecution.validate(Execution);
        *Valid = Execution->valid;
    }

    gcmkFOOTER();
    return status;
}

gceSTATUS gckFUNCTION_Init(IN gcsFUNCTION_EXECUTION_PTR Execution)
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

    gcmkHEADER_ARG("Execution=0x%x", Execution);
    /* Verify the arguments. */
    gcmkVERIFY_ARGUMENT(Execution != gcvNULL);

    Execution->inited = gcvTRUE;
    if (Execution->funcExecution.init)
    {
        status = Execution->funcExecution.init(Execution);
    }

    if (status != gcvSTATUS_OK)
    {
        Execution->inited = gcvFALSE;
    }

    gcmkFOOTER();
    return status;
}

gceSTATUS gckFUNCTION_Execute(IN gcsFUNCTION_EXECUTION_PTR Execution)
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

    gcmkHEADER_ARG("Execution=0x%x", Execution);
    /* Verify the arguments. */
    gcmkVERIFY_ARGUMENT(Execution != gcvNULL);

    if (Execution->inited && Execution->funcExecution.execute)
    {
        status = Execution->funcExecution.execute(Execution);
    }

    gcmkFOOTER();
    return status;
}

gceSTATUS gckFUNCTION_Release(IN gcsFUNCTION_EXECUTION_PTR Execution)
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

    gcmkHEADER_ARG("Execution=0x%x", Execution);
    /* Verify the arguments. */
    gcmkVERIFY_ARGUMENT(Execution != gcvNULL);

    if (Execution->inited && Execution->funcExecution.release)
    {
        status = Execution->funcExecution.release(Execution);
    }

    Execution->inited = gcvFALSE;

    gcmkFOOTER();
    return status;
}

void
gckFUNCTION_Dump(IN gcsFUNCTION_EXECUTION_PTR Execution)
{
#if gcdDUMP_IN_KERNEL
    gckHARDWARE hardware = (gckHARDWARE)Execution->hardware;

    gcmkDUMP(hardware->os, "#[function: %s]", Execution->funcName);
    gcmkDUMP_BUFFER(
        hardware->os,
        gcvDUMP_BUFFER_KERNEL_COMMAND,
        Execution->logical,
        Execution->address,
        Execution->bytes);
#endif
}


