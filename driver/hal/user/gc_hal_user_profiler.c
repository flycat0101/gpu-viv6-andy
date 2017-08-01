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


/*******************************************************************************
**    Profiler for Vivante HAL.
*/
#include "gc_hal_user_precomp.h"

#define _GC_OBJ_ZONE            gcvZONE_HAL

#if gcdENABLE_3D
#if VIVANTE_PROFILER

#ifdef ANDROID
#define DEFAULT_PROFILE_FILE_NAME   "/sdcard/vprofiler.vpd"
#else
#define DEFAULT_PROFILE_FILE_NAME   "vprofiler.vpd"
#endif

#ifdef ANDROID
#define DEFAULT_PROFILE_NEW_FILE_NAME   "/sdcard/vprofiler.vpd"
#else
#define DEFAULT_PROFILE_NEW_FILE_NAME   "vprofiler.vpd"
#endif

#define _HAL Hal

typedef struct _glhal_map
{
    gctUINT hardwareContext;
    gcoHAL  profilerHal;
    struct _glhal_map * next;
}glhal_map;
glhal_map* halprofilermap = gcvNULL;
static gcoHAL glhal_map_current()
{
    gceSTATUS status;
    gcoHARDWARE hardware = gcvNULL;
    gctUINT32 context;
    glhal_map* p = halprofilermap;
    if(p == gcvNULL) return gcvNULL;
    gcmGETHARDWARE(hardware);
    if (hardware)
    {
        gcmVERIFY_OK(gcoHARDWARE_GetContext(hardware, &context));
    }
    else
    {
        return gcvNULL;
    }
    while(p != gcvNULL)
    {
        if(p->hardwareContext == context) return p->profilerHal;
        p=p->next;
    }
OnError:
    return gcvNULL;
}
static void glhal_map_create(gcoHAL Hal)
{
    gceSTATUS status;
    gcoHARDWARE hardware = gcvNULL;
    gctUINT32 context;
    glhal_map* p = halprofilermap;
    gctPOINTER    pointer;

    gcmGETHARDWARE(hardware);
    if (hardware)
    {
        gcmVERIFY_OK(gcoHARDWARE_GetContext(hardware, &context));
    }
    else
    {
        gcmASSERT(-1);
        return;
    }

    gcmONERROR( gcoOS_Allocate(gcvNULL,
        gcmSIZEOF(struct _glhal_map),
        &pointer));
    p = (glhal_map*) pointer;
    p->hardwareContext = context;
    p->profilerHal = _HAL;
    p->next = gcvNULL;

    p = halprofilermap;

    if(p == gcvNULL) halprofilermap = (glhal_map*) pointer;
    else
    {
        while(p->next != gcvNULL)
        {
            p=p->next;
        }
        p->next = pointer;

    }
    return;
OnError:
    gcmASSERT(-1);
    return;
}
static void glhal_map_delete(gcoHAL Hal)
{
    glhal_map* p = halprofilermap;

    if(p->profilerHal == _HAL)
    {
        halprofilermap = halprofilermap->next;
    }
    else
    {
        glhal_map* pre = p;
        p = p->next;
        while(p != gcvNULL)
        {
            if(p->profilerHal == _HAL)
            {
                pre->next = p->next;
                break;
            }
            pre = p;
            p=p->next;
        }
        if(p == gcvNULL)
        {
            /*Must match*/
            gcmASSERT(-1);
            return;
        }
    }
    gcoOS_Free(gcvNULL, p);

}

/* Write a data value. */
#define gcmWRITE_VALUE_NEW(IntData) \
    do \
    { \
    gceSTATUS status; \
    gctINT32 value = IntData; \
    gcmERR_BREAK(gcoPROFILER_NEW_Write(Profiler, gcmSIZEOF(value), &value)); \
    } \
        while (gcvFALSE)

#define gcmWRITE_CONST_NEW(Const) \
    do \
    { \
    gceSTATUS status; \
    gctINT32 data = Const; \
    gcmERR_BREAK(gcoPROFILER_NEW_Write(Profiler, gcmSIZEOF(data), &data)); \
    } \
        while (gcvFALSE)

#define gcmWRITE_COUNTER_NEW(Counter, Value) \
    gcmWRITE_CONST_NEW(Counter); \
    gcmWRITE_VALUE_NEW(Value)

/* Write a string value (char*). */
#define gcmWRITE_STRING_NEW(String) \
    do \
    { \
    gceSTATUS status; \
    gctINT32 length; \
    length = (gctINT32) gcoOS_StrLen((gctSTRING)String, gcvNULL); \
    gcmERR_BREAK(gcoPROFILER_NEW_Write(Profiler, gcmSIZEOF(length), &length)); \
    gcmERR_BREAK(gcoPROFILER_NEW_Write(Profiler, length, String)); \
    } \
        while (gcvFALSE)

#define gcmWRITE_BUFFER_NEW(Size, Buffer) \
    do \
    { \
    gceSTATUS status; \
    gcmERR_BREAK(gcoPROFILER_NEW_Write(Profiler, Size, Buffer)); \
    } \
        while (gcvFALSE)

/* Write a data value. */
#define gcmWRITE_VALUE(IntData) \
    do \
    { \
        gceSTATUS status; \
        gctINT32 value = IntData; \
        gcmERR_BREAK(gcoPROFILER_Write(_HAL, gcmSIZEOF(value), &value)); \
    } \
    while (gcvFALSE)

#define gcmWRITE_CONST(Const) \
    do \
    { \
        gceSTATUS status; \
        gctINT32 data = Const; \
        gcmERR_BREAK(gcoPROFILER_Write(_HAL, gcmSIZEOF(data), &data)); \
    } \
    while (gcvFALSE)

#define gcmWRITE_COUNTER(Counter, Value) \
    gcmWRITE_CONST(Counter); \
    gcmWRITE_VALUE(Value)

/* Write a string value (char*). */
#define gcmWRITE_STRING(String) \
    do \
    { \
        gceSTATUS status; \
        gctINT32 length; \
        length = (gctINT32) gcoOS_StrLen((gctSTRING)String, gcvNULL); \
        gcmERR_BREAK(gcoPROFILER_Write(_HAL, gcmSIZEOF(length), &length)); \
        gcmERR_BREAK(gcoPROFILER_Write(_HAL, length, String)); \
    } \
    while (gcvFALSE)

#define gcmWRITE_BUFFER(Size, Buffer) \
    do \
    { \
        gceSTATUS status; \
        gcmERR_BREAK(gcoPROFILER_Write(_HAL, Size, Buffer)); \
    } \
    while (gcvFALSE)

#define gcmGET_COUNTER(counter, counterId) \
    do \
    { \
        if ((gctUINT32)*(memory + counterId + offset) == 0xdeaddead) \
        { \
            counter = 0xdeaddead; \
        } \
        else \
        { \
            gctUINT32 i; \
            gctUINT64_PTR Memory = memory; \
            counter = 0; \
            for (i = 0; i < coreCount; i++) \
            { \
                Memory += TOTAL_PROBE_NUMBER * i; \
                counter += (gctUINT32)*(Memory + counterId + offset); \
            } \
        } \
    } \
    while (gcvFALSE)

#define gcmGET_LATENCY_COUNTER(minLatency, maxLatency, counterId) \
    do \
    { \
        if ((gctUINT32)*(memory + counterId + offset) == 0xdeaddead) \
        { \
            minLatency = maxLatency = 0xdeaddead; \
        } \
        else \
        { \
            gctUINT32 i; \
            gctUINT64_PTR Memory = memory; \
            maxLatency = minLatency = 0; \
            for (i = 0; i < coreCount; i++) \
            { \
                Memory += TOTAL_PROBE_NUMBER * i; \
                maxLatency += (((gctUINT32)*(Memory + counterId + offset) & 0xfff000) >> 12); \
                minLatency += ((gctUINT32)*(Memory + counterId + offset) & 0x000fff); \
                if (minLatency == 4095) \
                    minLatency = 0; \
            } \
        } \
    } \
    while (gcvFALSE)

gceSTATUS
gcoPROFILER_NEW_Construct(
    OUT gcoPROFILER * Profiler
)
{
    gceSTATUS status=gcvSTATUS_OK;
    gcoPROFILER profiler = gcvNULL;
    gctPOINTER pointer = gcvNULL;
    gctINT32 bufId = 0;

    gcmHEADER();

    /* Verify the arguments. */
    gcmVERIFY_ARGUMENT(Profiler != gcvNULL);

    /* Allocate the gcoPROFILER object structure. */
    gcmONERROR(gcoOS_Allocate(gcvNULL,
        gcmSIZEOF(struct _gcoPROFILER),
        &pointer));


    profiler = pointer;

    profiler->enable = gcvFALSE;
    profiler->isSyncMode = gcvTRUE;
    profiler->file = gcvNULL;
    profiler->perDrawMode = gcvFALSE;
    profiler->needDump = gcvFALSE;
    profiler->counterEnable = gcvFALSE;
    profiler->fileName = DEFAULT_PROFILE_NEW_FILE_NAME;

    for (; bufId < NumOfDrawBuf; bufId++)
    {
        profiler->counterBuf[bufId].couterBufobj = gcvNULL;
        gcoOS_ZeroMemory(&profiler->counterBuf[bufId].counters, gcmSIZEOF(gcsPROFILER_NEW_COUNTERS));
        profiler->counterBuf[bufId].opType = gcvCOUNTER_OP_NONE;
    }

    profiler->curBufId = -1;
    /* Return the gcoPROFILER object. */
    *Profiler = profiler;

 OnError:
    /* Success. */
    gcmFOOTER_ARG("*Profiler=0x%x", *Profiler);
    return status;
}

gceSTATUS
gcoPROFILER_NEW_Destroy(
    IN gcoPROFILER Profiler
)
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsHAL_INTERFACE iface;
    gctUINT32 i = 0;

    gcmHEADER_ARG("Profiler=0x%x", Profiler);

    if (Profiler->file != gcvNULL)
    {
        gcmONERROR(gcoOS_Close(gcvNULL, Profiler->file));
    }

    for (i = 0; i < NumOfDrawBuf; i++)
    {
        if (Profiler->counterBuf[i].couterBufobj)
            gcoBUFOBJ_Destroy(Profiler->counterBuf[i].couterBufobj);
    }

    /* disable profiler in kernel. */
    iface.ignoreTLS = gcvFALSE;
    iface.command = gcvHAL_SET_PROFILE_SETTING;
    iface.u.SetProfileSetting.enable = gcvFALSE;

    /* Call the kernel. */
    gcoOS_DeviceControl(gcvNULL,
        IOCTL_GCHAL_INTERFACE,
        &iface, gcmSIZEOF(iface),
        &iface, gcmSIZEOF(iface));

    /* Free the gcoPROFILER structure. */
    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, Profiler));

OnError:
    /* Success. */
    gcmFOOTER();
    return gcvSTATUS_OK;
}

gceSTATUS
gcoPROFILER_NEW_Enable(
    IN gcoPROFILER Profiler
)
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsHAL_INTERFACE iface;
    gctCHAR* fileName = Profiler->fileName;
    gctCHAR inputFileName[256] = { '\0' };
    gctCHAR profilerName[256] = { '\0' };
    gctHANDLE pid = gcoOS_GetCurrentProcessID();
    static gctUINT8 num = 1;
    gctUINT offset = 0;
    gctCHAR* pos = gcvNULL;

    gcmHEADER_ARG("Profiler=0x%x", Profiler);

    /*generate file name for each context*/
    if (fileName) gcoOS_StrCatSafe(inputFileName, 256, fileName);

    gcmONERROR(gcoOS_StrStr(inputFileName, ".vpd", &pos));
    if (pos) pos[0] = '\0';
    gcmONERROR(gcoOS_PrintStrSafe(profilerName, gcmSIZEOF(profilerName), &offset, "%s_%d_%d.vpd", inputFileName, (gctUINTPTR_T)pid, num++));

    gcmONERROR(gcoOS_Open(gcvNULL, profilerName, gcvFILE_CREATE, &Profiler->file));

    /*do profile in new way by probe*/
    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_PROBE))
    {
        gctINT32 bufId = 0;
        gctUINT32 address;
        gctUINT32 size;
        gctUINT32 coreCount = 1;
        gcoBUFOBJ counterBuf;

        gcoHAL_ConfigPowerManagement(gcvTRUE);

        /* disable old profiler in kernel. */
        iface.ignoreTLS = gcvFALSE;
        iface.command = gcvHAL_SET_PROFILE_SETTING;
        iface.u.SetProfileSetting.enable = gcvFALSE;

        /* Call the kernel. */
        gcmONERROR(gcoOS_DeviceControl(gcvNULL,
            IOCTL_GCHAL_INTERFACE,
            &iface, gcmSIZEOF(iface),
            &iface, gcmSIZEOF(iface)));

        gcmONERROR(gcoHARDWARE_Query3DCoreCount(gcvNULL, &coreCount));

        size = gcmSIZEOF(gctUINT64) * TOTAL_PROBE_NUMBER * coreCount;
        for (; bufId < NumOfDrawBuf; bufId++)
        {
            gcmONERROR(gcoBUFOBJ_Construct(gcvNULL, gcvBUFOBJ_TYPE_GENERIC_BUFFER, &counterBuf));
            gcmONERROR(gcoBUFOBJ_Upload(counterBuf, gcvNULL, 0, size, gcvBUFOBJ_USAGE_STATIC_DRAW));
            gcoBUFOBJ_Lock(counterBuf, &address, gcvNULL);
            Profiler->counterBuf[bufId].probeAddress = address;
            Profiler->counterBuf[bufId].couterBufobj = (gctHANDLE)counterBuf;
            Profiler->counterBuf[bufId].opType = gcvCOUNTER_OP_NONE;
        }

        Profiler->curBufId = 0;
    }
    /* do profiling in old way*/
    else
    {
        gctUINT32 coreCount = 1;
        gctUINT32 coreId;
        gctUINT32 originalCoreIndex;
        gctINT32 bufId = 0;

        gcoHAL_ConfigPowerManagement(gcvFALSE);

        /* enable profiler in kernel. */
        iface.ignoreTLS = gcvFALSE;
        iface.command = gcvHAL_SET_PROFILE_SETTING;
        iface.u.SetProfileSetting.enable = gcvTRUE;

        iface.u.SetProfileSetting.syncMode = Profiler->isSyncMode;

        gcmONERROR(gcoHARDWARE_Query3DCoreCount(gcvNULL, &coreCount));
        gcmONERROR(gcoHAL_GetCurrentCoreIndex(gcvNULL, &originalCoreIndex));

        for (coreId = 0; coreId < coreCount; coreId++)
        {
            gctUINT32 coreIndex;
            /* Convert coreID in this hardware to global core index. */
            gcmONERROR(gcoHARDWARE_QueryCoreIndex(gcvNULL, coreId, &coreIndex));
            /* Set it to TLS to find correct command queue. */
            gcmONERROR(gcoHAL_SetCoreIndex(gcvNULL, coreIndex));

            /* Call the kernel. */
            gcmONERROR(gcoOS_DeviceControl(gcvNULL,
                IOCTL_GCHAL_INTERFACE,
                &iface, gcmSIZEOF(iface),
                &iface, gcmSIZEOF(iface)));
        }
        /* Restore core index in TLS. */
        gcmONERROR(gcoHAL_SetCoreIndex(gcvNULL, originalCoreIndex));

        for (; bufId < NumOfDrawBuf; bufId++)
        {
            gcoOS_ZeroMemory(&Profiler->counterBuf[bufId].counters, gcmSIZEOF(gcsPROFILER_NEW_COUNTERS));
            Profiler->counterBuf[bufId].opType = gcvCOUNTER_OP_NONE;
        }
        Profiler->curBufId = 0;
    }

    Profiler->perDrawMode = gcvTRUE;
    Profiler->needDump = gcvTRUE;
    Profiler->enable = gcvTRUE;

    gcmWRITE_CONST_NEW(VPHEADER);
    gcmWRITE_BUFFER_NEW(4, "VP12");

    /* Success. */
    gcmFOOTER_ARG("Profiler=0x%x", Profiler);
    return status;

OnError:

    Profiler->enable = gcvFALSE;
    /* Success. */
    gcmFOOTER_ARG("Profiler=0x%x", Profiler);
    return status;
}

gceSTATUS
gcoPROFILER_NEW_Disable(
)
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsHAL_INTERFACE iface;
    gcmHEADER();

    gcoHAL_ConfigPowerManagement(gcvTRUE);
    /* disable profiler in kernel. */
    iface.ignoreTLS = gcvFALSE;
    iface.command = gcvHAL_SET_PROFILE_SETTING;
    iface.u.SetProfileSetting.enable = gcvFALSE;

    /* Call the kernel. */
    status = gcoOS_DeviceControl(gcvNULL,
        IOCTL_GCHAL_INTERFACE,
        &iface, gcmSIZEOF(iface),
        &iface, gcmSIZEOF(iface));

    gcmFOOTER();
    return status;
}

gceSTATUS
gcoPROFILER_NEW_Begin(
    IN gcoPROFILER Profiler,
    IN gceCOUNTER_OPTYPE operationType
)
{
    gceSTATUS status;

    gcmHEADER_ARG("Profiler=0x%x operationType=%d", Profiler, operationType);

    if (!Profiler || !Profiler->perDrawMode)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_NOT_SUPPORTED;
    }

    gcmASSERT(Profiler->enable);

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_PROBE))
    {
        /* enable hw profiler counter here because of cl maybe do begin in another context which hw counter did not enabled */
        if (Profiler->counterEnable == gcvFALSE)
        {
            gcmONERROR(gcoHARDWARE_EnableCounters(gcvNULL));

            gcmONERROR(gcoHARDWARE_SetProbeCmd(gcvNULL, gcvPROBECMD_BEGIN, Profiler->counterBuf[0].probeAddress, gcvNULL));

            Profiler->counterEnable = gcvTRUE;
        }

        Profiler->counterBuf[Profiler->curBufId].opType = operationType;

        /* if the BufId beyond the limit, upload a new buffer */
        if (Profiler->curBufId >= NumOfDrawBuf)
        {
            gctINT32 i;
            gctUINT32 size;
            gctUINT32 coreCount = 1;
            gctPOINTER memory;

            gcmONERROR(gcoBUFOBJ_WaitFence(Profiler->counterBuf[NumOfDrawBuf-1].couterBufobj, gcvFENCE_TYPE_READ));

            for (i = 0; i <= Profiler->curBufId; i++)
            {
                gcmONERROR(gcoBUFOBJ_Lock(Profiler->counterBuf[i].couterBufobj, gcvNULL, &memory));

                gcmONERROR(gcoPROFILER_NEW_RecordCounters(memory, &Profiler->counterBuf[i].counters));

                gcmONERROR(gcoPROFILER_NEW_WriteCounters(Profiler, Profiler->counterBuf[i].counters, Profiler->counterBuf[i].opType));
            }

            Profiler->curBufId = 0;

            gcmONERROR(gcoHARDWARE_Query3DCoreCount(gcvNULL, &coreCount));

            size = gcmSIZEOF(gctUINT64) * TOTAL_PROBE_NUMBER * coreCount;

            for (i = 0; i < NumOfDrawBuf; i++)
            {
                gcmONERROR(gcoBUFOBJ_Upload(Profiler->counterBuf[i].couterBufobj, gcvNULL, 0, size, gcvBUFOBJ_USAGE_STATIC_DRAW));
                Profiler->counterBuf[i].opType = gcvCOUNTER_OP_NONE;
            }
        }
    }
    else
    {
        /* reset profiler counter */
        if (Profiler->counterEnable == gcvFALSE)
        {
            gcsHAL_INTERFACE iface;
            gctUINT32 context;
            gctUINT32 coreCount = 1;
            gctUINT32 coreId;
            gctUINT32 originalCoreIndex;

            gcmONERROR(gcoHAL_Commit(gcvNULL, gcvTRUE));

            gcmONERROR(gcoHARDWARE_Query3DCoreCount(gcvNULL, &coreCount));
            gcmONERROR(gcoHAL_GetCurrentCoreIndex(gcvNULL, &originalCoreIndex));

            iface.ignoreTLS = gcvFALSE;
            iface.command = gcvHAL_READ_ALL_PROFILE_NEW_REGISTERS_PART1;

            /* Call kernel service. */
            gcmVERIFY_OK(gcoHARDWARE_GetContext(gcvNULL, &context));
            if (context != 0)
                iface.u.RegisterProfileNewData_part1.context = context;

            for (coreId = 0; coreId < coreCount; coreId++)
            {
                gctUINT32 coreIndex;
                /* Convert coreID in this hardware to global core index. */
                gcmONERROR(gcoHARDWARE_QueryCoreIndex(gcvNULL, coreId, &coreIndex));
                /* Set it to TLS to find correct command queue. */
                gcmONERROR(gcoHAL_SetCoreIndex(gcvNULL, coreIndex));

                /* Call the kernel. */
                gcmONERROR(gcoOS_DeviceControl(gcvNULL,
                    IOCTL_GCHAL_INTERFACE,
                    &iface, gcmSIZEOF(iface),
                    &iface, gcmSIZEOF(iface)));
            }

            iface.ignoreTLS = gcvFALSE;
            iface.command = gcvHAL_READ_ALL_PROFILE_NEW_REGISTERS_PART2;

            /* Call kernel service. */
            gcmVERIFY_OK(gcoHARDWARE_GetContext(gcvNULL, &context));
            if (context != 0)
                iface.u.RegisterProfileNewData_part2.context = context;

            for (coreId = 0; coreId < coreCount; coreId++)
            {
                gctUINT32 coreIndex;
                /* Convert coreID in this hardware to global core index. */
                gcmONERROR(gcoHARDWARE_QueryCoreIndex(gcvNULL, coreId, &coreIndex));
                /* Set it to TLS to find correct command queue. */
                gcmONERROR(gcoHAL_SetCoreIndex(gcvNULL, coreIndex));

                /* Call the kernel. */
                gcmONERROR(gcoOS_DeviceControl(gcvNULL,
                    IOCTL_GCHAL_INTERFACE,
                    &iface, gcmSIZEOF(iface),
                    &iface, gcmSIZEOF(iface)));
            }
            /* Restore core index in TLS. */
            gcmONERROR(gcoHAL_SetCoreIndex(gcvNULL, originalCoreIndex));

            Profiler->counterEnable = gcvTRUE;
        }

        Profiler->counterBuf[Profiler->curBufId].opType = operationType;

        /* if the BufId beyond the limit, upload a new buffer */
        if (Profiler->curBufId >= NumOfDrawBuf - 1)
        {
            gctINT32 i;

            for (i = 0; i <= Profiler->curBufId; i++)
            {
                gcmONERROR(gcoPROFILER_NEW_WriteCounters(Profiler, Profiler->counterBuf[i].counters, Profiler->counterBuf[i].opType));
            }
            Profiler->curBufId = 0;

            for (i = 0 ; i < NumOfDrawBuf; i++)
            {
                gcoOS_ZeroMemory(&Profiler->counterBuf[i].counters, gcmSIZEOF(gcsPROFILER_NEW_COUNTERS));
                Profiler->counterBuf[i].opType = gcvCOUNTER_OP_NONE;
            }
        }
    }

OnError:

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
gcoPROFILER_NEW_End(
    IN gcoPROFILER Profiler,
    IN gctUINT32 DrawID,
    IN gceCOUNTER_OPTYPE Type
)
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Profiler=0x%x", Profiler);

    if (!Profiler)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_NOT_SUPPORTED;
    }

    gcmASSERT(Profiler->enable);

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_PROBE))
    {
        gcmONERROR(gcoHARDWARE_SetProbeCmd(gcvNULL, gcvPROBECMD_END, Profiler->counterBuf[Profiler->curBufId].probeAddress, gcvNULL));

        gcmONERROR(gcoBUFOBJ_GetFence(Profiler->counterBuf[Profiler->curBufId].couterBufobj, gcvFENCE_TYPE_READ));

        Profiler->counterBuf[Profiler->curBufId].opType = Type;

        if (Type == gcvCOUNTER_OP_DRAW)
        {
            Profiler->counterBuf[Profiler->curBufId].counters.drawID = DrawID;
            Profiler->curBufId++;
        }
    }
    else
    {
        gcsHAL_INTERFACE iface;
        gctUINT32 context;
        gctUINT32 coreCount = 1;
        gctUINT32 coreId;
        gctUINT32 originalCoreIndex;

        if (Type == gcvCOUNTER_OP_FINISH)
        {
            gcmFOOTER_NO();
            return status;
        }
        gcoHAL_Commit(gcvNULL, gcvTRUE);

        /* Set Register clear Flag. */
        iface.ignoreTLS = gcvFALSE;
        iface.command = gcvHAL_READ_PROFILER_NEW_REGISTER_SETTING;
        iface.u.SetProfilerRegisterClear.bclear = gcvFALSE;

        /* Call the kernel. */
        gcmONERROR(gcoHARDWARE_Query3DCoreCount(gcvNULL, &coreCount));
        gcmONERROR(gcoHAL_GetCurrentCoreIndex(gcvNULL, &originalCoreIndex));

        for (coreId = 0; coreId < coreCount; coreId++)
        {
            gctUINT32 coreIndex;
            /* Convert coreID in this hardware to global core index. */
            gcmONERROR(gcoHARDWARE_QueryCoreIndex(gcvNULL, coreId, &coreIndex));
            /* Set it to TLS to find correct command queue. */
            gcmONERROR(gcoHAL_SetCoreIndex(gcvNULL, coreIndex));

            /* Call the kernel. */
            gcmONERROR(gcoOS_DeviceControl(gcvNULL,
                IOCTL_GCHAL_INTERFACE,
                &iface, gcmSIZEOF(iface),
                &iface, gcmSIZEOF(iface)));
        }

        iface.ignoreTLS = gcvFALSE;
        iface.command = gcvHAL_READ_ALL_PROFILE_NEW_REGISTERS_PART1;

        gcmVERIFY_OK(gcoHARDWARE_GetContext(gcvNULL, &context));
        if (context != 0)
            iface.u.RegisterProfileNewData_part1.context = context;

        /* Call the kernel. */
        for (coreId = 0; coreId < coreCount; coreId++)
        {
            gctUINT32 coreIndex;
            /* Convert coreID in this hardware to global core index. */
            gcmONERROR(gcoHARDWARE_QueryCoreIndex(gcvNULL, coreId, &coreIndex));
            /* Set it to TLS to find correct command queue. */
            gcmONERROR(gcoHAL_SetCoreIndex(gcvNULL, coreIndex));

            /* Call the kernel. */
            gcmONERROR(gcoOS_DeviceControl(gcvNULL,
                IOCTL_GCHAL_INTERFACE,
                &iface, gcmSIZEOF(iface),
                &iface, gcmSIZEOF(iface)));
            Profiler->counterBuf[Profiler->curBufId].counters.counters_part1 = iface.u.RegisterProfileNewData_part1.newCounters;
        }

        iface.ignoreTLS = gcvFALSE;
        iface.command = gcvHAL_READ_ALL_PROFILE_NEW_REGISTERS_PART2;

        gcmVERIFY_OK(gcoHARDWARE_GetContext(gcvNULL, &context));
        if (context != 0)
            iface.u.RegisterProfileNewData_part2.context = context;

        /* Call the kernel. */
        for (coreId = 0; coreId < coreCount; coreId++)
        {
            gctUINT32 coreIndex;
            /* Convert coreID in this hardware to global core index. */
            gcmONERROR(gcoHARDWARE_QueryCoreIndex(gcvNULL, coreId, &coreIndex));
            /* Set it to TLS to find correct command queue. */
            gcmONERROR(gcoHAL_SetCoreIndex(gcvNULL, coreIndex));

            /* Call the kernel. */
            gcmONERROR(gcoOS_DeviceControl(gcvNULL,
                IOCTL_GCHAL_INTERFACE,
                &iface, gcmSIZEOF(iface),
                &iface, gcmSIZEOF(iface)));
            Profiler->counterBuf[Profiler->curBufId].counters.counters_part2 = iface.u.RegisterProfileNewData_part2.newCounters;
        }

        Profiler->counterBuf[Profiler->curBufId].opType = Type;
        Profiler->counterBuf[Profiler->curBufId].counters.drawID = DrawID;
        Profiler->curBufId++;
        /* Restore core index in TLS. */
        gcmONERROR(gcoHAL_SetCoreIndex(gcvNULL, originalCoreIndex));
    }

OnError:

    gcmFOOTER_NO();
    return status;
}

gceSTATUS
gcoPROFILER_NEW_EndFrame(
    IN gcoPROFILER Profiler,
    IN gceCOUNTER_OPTYPE Type
)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctINT32 i;

    gcmHEADER_ARG("Profiler=0x%x", Profiler);

    if (!Profiler)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_NOT_SUPPORTED;
    }

    gcmASSERT(Profiler->enable);

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_PROBE))
    {
        gctPOINTER memory;

        if (Type == gcvCOUNTER_OP_FINISH)
        {
            gcmONERROR(gcoBUFOBJ_WaitFence(Profiler->counterBuf[Profiler->curBufId].couterBufobj, gcvFENCE_TYPE_READ));

            /*write this finish counter to file*/
            gcmONERROR(gcoBUFOBJ_Lock(Profiler->counterBuf[Profiler->curBufId].couterBufobj, gcvNULL, &memory));

            gcmONERROR(gcoPROFILER_NEW_RecordCounters(memory, &Profiler->counterBuf[Profiler->curBufId].counters));

            gcmONERROR(gcoPROFILER_NEW_WriteCounters(Profiler, Profiler->counterBuf[Profiler->curBufId].counters, Profiler->counterBuf[Profiler->curBufId].opType));

            Profiler->curBufId++;
        }
        else
        {
            Profiler->counterBuf[Profiler->curBufId].opType = Type;

            gcmONERROR(gcoBUFOBJ_GetFence(Profiler->counterBuf[Profiler->curBufId].couterBufobj, gcvFENCE_TYPE_READ));

            gcmONERROR(gcoHARDWARE_SetProbeCmd(gcvNULL, gcvPROBECMD_END, Profiler->counterBuf[Profiler->curBufId].probeAddress, gcvNULL));

            /*add a fence here, no need commit true*/
            gcmONERROR(gcoBUFOBJ_WaitFence(Profiler->counterBuf[Profiler->curBufId].couterBufobj, gcvFENCE_TYPE_READ));

            /*write this frame perdraw counter to file*/
            for (i = 0; i < Profiler->curBufId + 1; i++)
            {
                if (Profiler->counterBuf[i].opType == gcvCOUNTER_OP_FINISH)
                {
                    continue;
                }
                gcmONERROR(gcoBUFOBJ_Lock(Profiler->counterBuf[i].couterBufobj, gcvNULL, &memory));

                gcmONERROR(gcoPROFILER_NEW_RecordCounters(memory, &Profiler->counterBuf[i].counters));

                gcmONERROR(gcoPROFILER_NEW_WriteCounters(Profiler, Profiler->counterBuf[i].counters, Profiler->counterBuf[i].opType));
            }
            gcmONERROR(gcoHARDWARE_SetProbeCmd(gcvNULL, gcvPROBECMD_BEGIN, Profiler->counterBuf[0].probeAddress, gcvNULL));
            Profiler->curBufId = 0;
        }
    }
    else
    {
        gcsHAL_INTERFACE iface;
        gctUINT32 context;
        gctUINT32 coreCount = 1;
        gctUINT32 coreId;
        gctUINT32 originalCoreIndex;

        gcoHAL_Commit(gcvNULL, gcvTRUE);

        /* Set Register clear Flag. */
        iface.ignoreTLS = gcvFALSE;
        iface.command = gcvHAL_READ_PROFILER_NEW_REGISTER_SETTING;
        if (Type == gcvCOUNTER_OP_FINISH)
        {
            iface.u.SetProfilerRegisterClear.bclear = gcvFALSE;
        }
        else
        {
            iface.u.SetProfilerRegisterClear.bclear = gcvTRUE;
        }

        /* Call the kernel. */
        gcmONERROR(gcoHARDWARE_Query3DCoreCount(gcvNULL, &coreCount));
        gcmONERROR(gcoHAL_GetCurrentCoreIndex(gcvNULL, &originalCoreIndex));

        for (coreId = 0; coreId < coreCount; coreId++)
        {
            gctUINT32 coreIndex;
            /* Convert coreID in this hardware to global core index. */
            gcmONERROR(gcoHARDWARE_QueryCoreIndex(gcvNULL, coreId, &coreIndex));
            /* Set it to TLS to find correct command queue. */
            gcmONERROR(gcoHAL_SetCoreIndex(gcvNULL, coreIndex));

            /* Call the kernel. */
            gcmONERROR(gcoOS_DeviceControl(gcvNULL,
                IOCTL_GCHAL_INTERFACE,
                &iface, gcmSIZEOF(iface),
                &iface, gcmSIZEOF(iface)));
        }

        iface.ignoreTLS = gcvFALSE;
        iface.command = gcvHAL_READ_ALL_PROFILE_NEW_REGISTERS_PART1;

        gcmVERIFY_OK(gcoHARDWARE_GetContext(gcvNULL, &context));
        if (context != 0)
            iface.u.RegisterProfileNewData_part1.context = context;

        /* Call the kernel. */
        for (coreId = 0; coreId < coreCount; coreId++)
        {
            gctUINT32 coreIndex;
            /* Convert coreID in this hardware to global core index. */
            gcmONERROR(gcoHARDWARE_QueryCoreIndex(gcvNULL, coreId, &coreIndex));
            /* Set it to TLS to find correct command queue. */
            gcmONERROR(gcoHAL_SetCoreIndex(gcvNULL, coreIndex));

            /* Call the kernel. */
            gcmONERROR(gcoOS_DeviceControl(gcvNULL,
                IOCTL_GCHAL_INTERFACE,
                &iface, gcmSIZEOF(iface),
                &iface, gcmSIZEOF(iface)));
            Profiler->counterBuf[Profiler->curBufId].counters.counters_part1 = iface.u.RegisterProfileNewData_part1.newCounters;
        }

        iface.ignoreTLS = gcvFALSE;
        iface.command = gcvHAL_READ_ALL_PROFILE_NEW_REGISTERS_PART2;

        gcmVERIFY_OK(gcoHARDWARE_GetContext(gcvNULL, &context));
        if (context != 0)
            iface.u.RegisterProfileNewData_part2.context = context;

        /* Call the kernel. */
        for (coreId = 0; coreId < coreCount; coreId++)
        {
            gctUINT32 coreIndex;
            /* Convert coreID in this hardware to global core index. */
            gcmONERROR(gcoHARDWARE_QueryCoreIndex(gcvNULL, coreId, &coreIndex));
            /* Set it to TLS to find correct command queue. */
            gcmONERROR(gcoHAL_SetCoreIndex(gcvNULL, coreIndex));

            /* Call the kernel. */
            gcmONERROR(gcoOS_DeviceControl(gcvNULL,
                IOCTL_GCHAL_INTERFACE,
                &iface, gcmSIZEOF(iface),
                &iface, gcmSIZEOF(iface)));
            Profiler->counterBuf[Profiler->curBufId].counters.counters_part2 = iface.u.RegisterProfileNewData_part2.newCounters;
        }
        /* Restore core index in TLS. */
        gcmONERROR(gcoHAL_SetCoreIndex(gcvNULL, originalCoreIndex));

        if (Type == gcvCOUNTER_OP_FINISH)
        {
            /*write this finish counter to file*/
            gcmONERROR(gcoPROFILER_NEW_WriteCounters(Profiler, Profiler->counterBuf[Profiler->curBufId].counters, Profiler->counterBuf[Profiler->curBufId].opType));

            Profiler->curBufId++;
        }
        else
        {
            /*write this frame perdraw counter to file*/
            for (i = 0; i < Profiler->curBufId + 1; i++)
            {
                if (Profiler->counterBuf[i].opType == gcvCOUNTER_OP_FINISH)
                {
                    continue;
                }
                gcmONERROR(gcoPROFILER_NEW_WriteCounters(Profiler, Profiler->counterBuf[i].counters, Profiler->counterBuf[i].opType));
            }
            Profiler->curBufId = 0;
        }
    }
OnError:

    gcmFOOTER_NO();
    return status;
}

gceSTATUS
gcoPROFILER_NEW_RecordCounters(
    IN gctPOINTER Logical,
    OUT gcsPROFILER_NEW_COUNTERS * Counters
)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT64_PTR memory = gcvNULL;
    gctUINT32 offset = 0;
    gctUINT32 coreCount = 1;

    gcmHEADER_ARG("Logical=0x%x, Counters=0x%x", Logical, Counters);

    gcmASSERT(gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_PROBE));

    gcmONERROR(gcoHARDWARE_Query3DCoreCount(gcvNULL, &coreCount));

    memory = (gctUINT64_PTR)Logical;

    gcoOS_ZeroMemory(&Counters->counters_part1, gcmSIZEOF(gcsPROFILER_NEW_COUNTERS_PART1));
    gcoOS_ZeroMemory(&Counters->counters_part2, gcmSIZEOF(gcsPROFILER_NEW_COUNTERS_PART2));
    /* module FE */
    gcmGET_COUNTER(Counters->counters_part1.fe_out_vertex_count, 0);
    gcmGET_COUNTER(Counters->counters_part1.fe_cache_miss_count, 1);
    gcmGET_COUNTER(Counters->counters_part1.fe_cache_lk_count, 2);
    gcmGET_COUNTER(Counters->counters_part1.fe_stall_count, 3);
    gcmGET_COUNTER(Counters->counters_part1.fe_process_count, 4);
    Counters->counters_part1.fe_starve_count = 0xDEADDEAD;
    offset += MODULE_FRONT_END_COUNTER_NUM;

    /* module VS */
    /*todo*/
    gcmGET_COUNTER(Counters->counters_part1.vs_shader_cycle_count, 0);
    gcmGET_COUNTER(Counters->counters_part1.vs_inst_counter, 3);
    gcmGET_COUNTER(Counters->counters_part1.vs_rendered_vertice_counter, 4);
    gcmGET_COUNTER(Counters->counters_part1.vs_branch_inst_counter, 5);
    gcmGET_COUNTER(Counters->counters_part1.vs_texld_inst_counter, 6);
    offset += MODULE_VERTEX_SHADER_COUNTER_NUM;

    /* module PA */
    gcmGET_COUNTER(Counters->counters_part1.pa_input_vtx_counter, 0);
    gcmGET_COUNTER(Counters->counters_part1.pa_input_prim_counter, 1);
    gcmGET_COUNTER(Counters->counters_part1.pa_output_prim_counter, 2);
    gcmGET_COUNTER(Counters->counters_part1.pa_trivial_rejected_counter, 3);
    gcmGET_COUNTER(Counters->counters_part1.pa_culled_prim_counter, 4);
    gcmGET_COUNTER(Counters->counters_part1.pa_droped_prim_counter, 5);
    gcmGET_COUNTER(Counters->counters_part1.pa_frustum_clipped_prim_counter, 6);
    gcmGET_COUNTER(Counters->counters_part1.pa_frustum_clipdroped_prim_counter, 7);
    gcmGET_COUNTER(Counters->counters_part1.pa_non_idle_starve_count, 8);
    gcmGET_COUNTER(Counters->counters_part1.pa_starve_count, 9);
    gcmGET_COUNTER(Counters->counters_part1.pa_stall_count, 10);
    gcmGET_COUNTER(Counters->counters_part1.pa_process_count, 11);
    offset += MODULE_PRIMITIVE_ASSEMBLY_COUNTER_NUM;

    /* module SE */
    gcmGET_COUNTER(Counters->counters_part1.se_starve_count, 0);
    gcmGET_COUNTER(Counters->counters_part1.se_stall_count, 1);
    gcmGET_COUNTER(Counters->counters_part1.se_receive_triangle_count, 2);
    gcmGET_COUNTER(Counters->counters_part1.se_send_triangle_count, 3);
    gcmGET_COUNTER(Counters->counters_part1.se_receive_lines_count, 4);
    gcmGET_COUNTER(Counters->counters_part1.se_send_lines_count, 5);
    gcmGET_COUNTER(Counters->counters_part1.se_process_count, 6);
    gcmGET_COUNTER(Counters->counters_part1.se_clipped_triangle_count, 7);
    gcmGET_COUNTER(Counters->counters_part1.se_clipped_line_count, 8);
    gcmGET_COUNTER(Counters->counters_part1.se_culled_lines_count, 9);
    gcmGET_COUNTER(Counters->counters_part1.se_culled_triangle_count, 10);
    gcmGET_COUNTER(Counters->counters_part1.se_trivial_rejected_line_count, 11);
    gcmGET_COUNTER(Counters->counters_part1.se_non_idle_starve_count, 12);
    offset += MODULE_SETUP_COUNTER_NUM;

    /* module RA */
    gcmGET_COUNTER(Counters->counters_part1.ra_input_prim_count, 0);
    gcmGET_COUNTER(Counters->counters_part1.ra_total_quad_count, 1);
    gcmGET_COUNTER(Counters->counters_part1.ra_prefetch_cache_miss_counter, 2);
    gcmGET_COUNTER(Counters->counters_part1.ra_prefetch_hz_cache_miss_counter, 3);
    gcmGET_COUNTER(Counters->counters_part1.ra_valid_quad_count_after_early_z, 4);
    gcmGET_COUNTER(Counters->counters_part1.ra_valid_pixel_count_to_render, 5);
    gcmGET_COUNTER(Counters->counters_part1.ra_output_valid_quad_count, 6);
    gcmGET_COUNTER(Counters->counters_part1.ra_output_valid_pixel_count, 7);
    Counters->counters_part1.ra_eez_culled_counter = 0xDEADDEAD;
    gcmGET_COUNTER(Counters->counters_part1.ra_pipe_cache_miss_counter, 8);
    gcmGET_COUNTER(Counters->counters_part1.ra_pipe_hz_cache_miss_counter, 9);
    gcmGET_COUNTER(Counters->counters_part1.ra_non_idle_starve_count, 10);
    gcmGET_COUNTER(Counters->counters_part1.ra_starve_count, 11);
    gcmGET_COUNTER(Counters->counters_part1.ra_stall_count, 12);
    gcmGET_COUNTER(Counters->counters_part1.ra_process_count, 13);
    offset += MODULE_RASTERIZER_COUNTER_NUM;

    /* module PS */
    gcmGET_COUNTER(Counters->counters_part1.ps_shader_cycle_count, 0);
    gcmGET_COUNTER(Counters->counters_part1.ps_inst_counter, 1);
    gcmGET_COUNTER(Counters->counters_part1.ps_rendered_pixel_counter, 2);
    gcmGET_COUNTER(Counters->counters_part1.ps_branch_inst_counter, 7);
    gcmGET_COUNTER(Counters->counters_part1.ps_texld_inst_counter, 8);
    offset += MODULE_PIXEL_SHADER_COUNTER_NUM;

    /* module TX */
    gcmGET_COUNTER(Counters->counters_part1.tx_total_bilinear_requests, 0);
    gcmGET_COUNTER(Counters->counters_part1.tx_total_trilinear_requests, 1);
    gcmGET_COUNTER(Counters->counters_part1.tx_total_discarded_texture_requests, 2);
    gcmGET_COUNTER(Counters->counters_part1.tx_total_texture_requests, 3);
    gcmGET_COUNTER(Counters->counters_part1.tx_mc0_miss_count, 4);
    gcmGET_COUNTER(Counters->counters_part1.tx_mc0_request_byte_count, 5);
    gcmGET_COUNTER(Counters->counters_part1.tx_mc1_miss_count, 6);
    gcmGET_COUNTER(Counters->counters_part1.tx_mc1_request_byte_count, 7);
    offset += MODULE_TEXTURE_COUNTER_NUM;

    /* module PE */
    gcmGET_COUNTER(Counters->counters_part1.pe0_pixel_count_killed_by_color_pipe, 0);
    gcmGET_COUNTER(Counters->counters_part1.pe0_pixel_count_killed_by_depth_pipe, 1);
    gcmGET_COUNTER(Counters->counters_part1.pe0_pixel_count_drawn_by_color_pipe, 2);
    gcmGET_COUNTER(Counters->counters_part1.pe0_pixel_count_drawn_by_depth_pipe, 3);
    gcmGET_COUNTER(Counters->counters_part1.pe1_pixel_count_killed_by_color_pipe, 4);
    gcmGET_COUNTER(Counters->counters_part1.pe1_pixel_count_killed_by_depth_pipe, 5);
    gcmGET_COUNTER(Counters->counters_part1.pe1_pixel_count_drawn_by_color_pipe, 6);
    gcmGET_COUNTER(Counters->counters_part1.pe1_pixel_count_drawn_by_depth_pipe, 7);
    offset += MODULE_PIXEL_ENGINE_COUNTER_NUM;

    /* module MCC */
    gcmGET_LATENCY_COUNTER(Counters->counters_part2.mcc_axi_min_latency, Counters->counters_part2.mcc_axi_max_latency, 0);
    gcmGET_COUNTER(Counters->counters_part2.mcc_axi_total_latency, 1);
    gcmGET_COUNTER(Counters->counters_part2.mcc_axi_sample_count, 2);
    gcmGET_COUNTER(Counters->counters_part2.mcc_total_read_req_8B_from_colorpipe, 3);
    gcmGET_COUNTER(Counters->counters_part2.mcc_total_read_req_8B_sentout_from_colorpipe, 4);
    gcmGET_COUNTER(Counters->counters_part2.mcc_total_write_req_8B_from_colorpipe, 5);
    gcmGET_COUNTER(Counters->counters_part2.mcc_total_read_req_sentout_from_colorpipe, 6);
    gcmGET_COUNTER(Counters->counters_part2.mcc_total_write_req_from_colorpipe, 7);
    gcmGET_COUNTER(Counters->counters_part2.mcc_total_read_req_8B_from_others, 8);
    gcmGET_COUNTER(Counters->counters_part2.mcc_total_write_req_8B_from_others, 9);
    gcmGET_COUNTER(Counters->counters_part2.mcc_total_read_req_from_others, 10);
    gcmGET_COUNTER(Counters->counters_part2.mcc_total_write_req_from_others, 11);
    offset += MODULE_MEMORY_CONTROLLER_COLOR_COUNTER_NUM;

    /* module MCZ */
    gcmGET_LATENCY_COUNTER(Counters->counters_part2.mcz_axi_min_latency, Counters->counters_part2.mcz_axi_max_latency, 0);
    gcmGET_COUNTER(Counters->counters_part2.mcz_axi_total_latency, 1);
    gcmGET_COUNTER(Counters->counters_part2.mcz_axi_sample_count, 2);
    gcmGET_COUNTER(Counters->counters_part2.mcz_total_read_req_8B_from_colorpipe, 3);
    gcmGET_COUNTER(Counters->counters_part2.mcz_total_read_req_8B_sentout_from_colorpipe, 4);
    gcmGET_COUNTER(Counters->counters_part2.mcz_total_write_req_8B_from_colorpipe, 5);
    gcmGET_COUNTER(Counters->counters_part2.mcz_total_read_req_sentout_from_colorpipe, 6);
    gcmGET_COUNTER(Counters->counters_part2.mcz_total_write_req_from_colorpipe, 7);
    gcmGET_COUNTER(Counters->counters_part2.mcz_total_read_req_8B_from_others, 8);
    gcmGET_COUNTER(Counters->counters_part2.mcz_total_write_req_8B_from_others, 9);
    gcmGET_COUNTER(Counters->counters_part2.mcz_total_read_req_from_others, 10);
    gcmGET_COUNTER(Counters->counters_part2.mcz_total_write_req_from_others, 11);
    offset += MODULE_MEMORY_CONTROLLER_DEPTH_COUNTER_NUM;

    /* module HI0 */
    gcmGET_COUNTER(Counters->counters_part2.hi0_total_read_8B_count, 0);
    gcmGET_COUNTER(Counters->counters_part2.hi0_total_write_8B_count, 1);
    gcmGET_COUNTER(Counters->counters_part2.hi0_total_write_request_count, 2);
    gcmGET_COUNTER(Counters->counters_part2.hi0_total_read_request_count, 3);
    gcmGET_COUNTER(Counters->counters_part2.hi0_axi_cycles_read_request_stalled, 4);
    gcmGET_COUNTER(Counters->counters_part2.hi0_axi_cycles_write_request_stalled, 5);
    gcmGET_COUNTER(Counters->counters_part2.hi0_axi_cycles_write_data_stalled, 6);
    gcmGET_COUNTER(Counters->counters_part2.hi_total_cycle_count, 7);
    gcmGET_COUNTER(Counters->counters_part2.hi_total_idle_cycle_count, 8);
    offset += MODULE_HOST_INTERFACE0_COUNTER_NUM;

    /* module HI1 */
    gcmGET_COUNTER(Counters->counters_part2.hi1_total_read_8B_count, 0);
    gcmGET_COUNTER(Counters->counters_part2.hi1_total_write_8B_count, 1);
    gcmGET_COUNTER(Counters->counters_part2.hi1_total_write_request_count, 2);
    gcmGET_COUNTER(Counters->counters_part2.hi1_total_read_request_count, 3);
    gcmGET_COUNTER(Counters->counters_part2.hi1_axi_cycles_read_request_stalled, 4);
    gcmGET_COUNTER(Counters->counters_part2.hi1_axi_cycles_write_request_stalled, 5);
    gcmGET_COUNTER(Counters->counters_part2.hi1_axi_cycles_write_data_stalled, 6);
    offset += MODULE_HOST_INTERFACE1_COUNTER_NUM;

    /* module L2 */
    gcmGET_COUNTER(Counters->counters_part2.l2_total_axi0_read_request_count, 0);
    gcmGET_COUNTER(Counters->counters_part2.l2_total_axi1_read_request_count, 1);
    gcmGET_COUNTER(Counters->counters_part2.l2_total_axi0_write_request_count, 2);
    gcmGET_COUNTER(Counters->counters_part2.l2_total_axi1_write_request_count, 3);
    gcmGET_COUNTER(Counters->counters_part2.l2_total_read_transactions_request_by_axi0, 4);
    gcmGET_COUNTER(Counters->counters_part2.l2_total_read_transactions_request_by_axi1, 5);
    gcmGET_COUNTER(Counters->counters_part2.l2_total_write_transactions_request_by_axi0, 6);
    gcmGET_COUNTER(Counters->counters_part2.l2_total_write_transactions_request_by_axi1, 7);
    gcmGET_LATENCY_COUNTER(Counters->counters_part2.l2_axi0_min_latency, Counters->counters_part2.l2_axi0_max_latency, 8);
    gcmGET_COUNTER(Counters->counters_part2.l2_axi0_total_latency, 9);
    gcmGET_COUNTER(Counters->counters_part2.l2_axi0_total_request_count, 10);
    gcmGET_LATENCY_COUNTER(Counters->counters_part2.l2_axi1_min_latency, Counters->counters_part2.l2_axi1_max_latency, 11);
    gcmGET_COUNTER(Counters->counters_part2.l2_axi1_total_latency, 12);
    gcmGET_COUNTER(Counters->counters_part2.l2_axi1_total_request_count, 13);

OnError:
    gcmFOOTER_NO();
    return status;
}

gcmINLINE static gctUINT32
CalcDelta(
IN gctUINT32 new,
IN gctUINT32 old
)
{
    if (new == 0xdeaddead || new == 0xdead)
    {
        return new;
    }
    if (new >= old)
    {
        return new - old;
    }
    else
    {
        return (gctUINT32)((gctUINT64)new + 0x100000000ll - old);
    }
}

/* Write counter to profile. */
gceSTATUS
gcoPROFILER_NEW_WriteCounters(
    IN gcoPROFILER Profiler,
    IN gcsPROFILER_NEW_COUNTERS Counters,
    IN gceCOUNTER_OPTYPE OpType
)
{
    gceSTATUS status = gcvSTATUS_OK;
    static gcsPROFILER_NEW_COUNTERS precounters;
    gctUINT shaderCoreCount = 0;
    gctBOOL bSupportProbe = (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_PROBE) == gcvSTATUS_TRUE);
    gctBOOL bHalti4 = (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_HALTI4) == gcvSTATUS_TRUE);
    gceCHIPMODEL chipModel;
    gctUINT32 chipRevision;
    gctBOOL axiBus128bits;

    gcmHEADER_ARG("Profiler=0x%x Counters=0x%x", Profiler, Counters);

    if (!Profiler)
    {
        gcmFOOTER();
        return gcvSTATUS_NOT_SUPPORTED;
    }

    gcmASSERT(Profiler->enable);

#define gcmGETCOUNTER(name) OpType == gcvCOUNTER_OP_DRAW ? CalcDelta((Counters.name), (precounters.name)) : (Counters.name)

    if (Counters.drawID == 0 && OpType == gcvCOUNTER_OP_DRAW)
    {
        gcoOS_ZeroMemory(&precounters, gcmSIZEOF(precounters));
    }

    gcoHAL_QueryShaderCaps(gcvNULL,
                           gcvNULL,
                           gcvNULL,
                           gcvNULL,
                           gcvNULL,
                           &shaderCoreCount,
                           gcvNULL,
                           gcvNULL,
                           gcvNULL);

    gcoHAL_QueryChipIdentity(gcvNULL, &chipModel, &chipRevision, gcvNULL, gcvNULL);

    gcoHAL_QueryChipAxiBusWidth(&axiBus128bits);

    if (Profiler->needDump)
    {
        if (OpType == gcvCOUNTER_OP_DRAW)
        {
            gcmWRITE_CONST_NEW(VPG_ES30_DRAW);
            gcmWRITE_COUNTER_NEW(VPC_ES30_DRAW_NO, Counters.drawID);
        }
        else
        {
            gcmWRITE_CONST_NEW(VPG_HW);
        }
        gcmWRITE_CONST_NEW(VPNG_FE);
        gcmWRITE_COUNTER_NEW(VPNC_FEDRAWCOUNT, gcmGETCOUNTER(counters_part1.fe_draw_count));
        gcmWRITE_COUNTER_NEW(VPNC_FEOUTVERTEXCOUNT, gcmGETCOUNTER(counters_part1.fe_out_vertex_count));
        gcmWRITE_COUNTER_NEW(VPNC_FECACHEMISSCOUNT, gcmGETCOUNTER(counters_part1.fe_cache_miss_count));
        gcmWRITE_COUNTER_NEW(VPNC_FECACHELKCOUNT, gcmGETCOUNTER(counters_part1.fe_cache_lk_count));
        gcmWRITE_COUNTER_NEW(VPNC_FESTALLCOUNT, gcmGETCOUNTER(counters_part1.fe_stall_count));
        gcmWRITE_COUNTER_NEW(VPNC_FESTARVECOUNT, gcmGETCOUNTER(counters_part1.fe_starve_count));
        gcmWRITE_COUNTER_NEW(VPNC_FEPROCESSCOUNT, gcmGETCOUNTER(counters_part1.fe_stall_count));
        gcmWRITE_CONST_NEW(VPG_END);

        gcmWRITE_CONST_NEW(VPNG_VS);
        gcmWRITE_COUNTER_NEW(VPNC_VSINSTCOUNT, gcmGETCOUNTER(counters_part1.vs_inst_counter));
        gcmWRITE_COUNTER_NEW(VPNC_VSBRANCHINSTCOUNT, gcmGETCOUNTER(counters_part1.vs_branch_inst_counter));
        gcmWRITE_COUNTER_NEW(VPNC_VSTEXLDINSTCOUNT, gcmGETCOUNTER(counters_part1.vs_texld_inst_counter));
        gcmWRITE_COUNTER_NEW(VPNC_VSRENDEREDVERTCOUNT, gcmGETCOUNTER(counters_part1.vs_rendered_vertice_counter));
        gcmWRITE_COUNTER_NEW(VPNC_VSNONIDLESTARVECOUNT, gcmGETCOUNTER(counters_part1.vs_non_idle_starve_count));
        gcmWRITE_COUNTER_NEW(VPNC_VSSTARVELCOUNT, gcmGETCOUNTER(counters_part1.vs_starve_count));
        gcmWRITE_COUNTER_NEW(VPNC_VSSTALLCOUNT, gcmGETCOUNTER(counters_part1.vs_stall_count));
        gcmWRITE_COUNTER_NEW(VPNC_VSPROCESSCOUNT, gcmGETCOUNTER(counters_part1.vs_process_count));
        gcmWRITE_COUNTER_NEW(VPNC_VSSHADERCYCLECOUNT, gcmGETCOUNTER(counters_part1.vs_shader_cycle_count));
        gcmWRITE_CONST_NEW(VPG_END);

        gcmWRITE_CONST_NEW(VPNG_PA);
        gcmWRITE_COUNTER_NEW(VPNC_PAINVERTCOUNT, gcmGETCOUNTER(counters_part1.pa_input_vtx_counter));
        gcmWRITE_COUNTER_NEW(VPNC_PAINPRIMCOUNT, gcmGETCOUNTER(counters_part1.pa_input_prim_counter));
        gcmWRITE_COUNTER_NEW(VPNC_PAOUTPRIMCOUNT, gcmGETCOUNTER(counters_part1.pa_output_prim_counter));
        gcmWRITE_COUNTER_NEW(VPNC_PADEPTHCLIPCOUNT, gcmGETCOUNTER(counters_part1.pa_depth_clipped_counter));
        gcmWRITE_COUNTER_NEW(VPNC_PATRIVIALREJCOUNT, gcmGETCOUNTER(counters_part1.pa_trivial_rejected_counter));
        gcmWRITE_COUNTER_NEW(VPNC_PACULLPRIMCOUNT, gcmGETCOUNTER(counters_part1.pa_culled_prim_counter));
        gcmWRITE_COUNTER_NEW(VPNC_PADROPPRIMCOUNT, gcmGETCOUNTER(counters_part1.pa_droped_prim_counter));
        gcmWRITE_COUNTER_NEW(VPNC_PAFRCLIPPRIMCOUNT, gcmGETCOUNTER(counters_part1.pa_frustum_clipped_prim_counter));
        gcmWRITE_COUNTER_NEW(VPNC_PAFRCLIPDROPPRIMCOUNT, gcmGETCOUNTER(counters_part1.pa_frustum_clipdroped_prim_counter));
        gcmWRITE_COUNTER_NEW(VPNC_PANONIDLESTARVECOUNT, gcmGETCOUNTER(counters_part1.pa_non_idle_starve_count));
        gcmWRITE_COUNTER_NEW(VPNC_PASTARVELCOUNT, gcmGETCOUNTER(counters_part1.pa_starve_count));
        gcmWRITE_COUNTER_NEW(VPNC_PASTALLCOUNT, gcmGETCOUNTER(counters_part1.pa_stall_count));
        gcmWRITE_COUNTER_NEW(VPNC_PAPROCESSCOUNT, gcmGETCOUNTER(counters_part1.pa_process_count));
        gcmWRITE_CONST_NEW(VPG_END);

        gcmWRITE_CONST_NEW(VPNG_SETUP);
        gcmWRITE_COUNTER_NEW(VPNC_SECULLTRIANGLECOUNT, gcmGETCOUNTER(counters_part1.se_culled_triangle_count));
        gcmWRITE_COUNTER_NEW(VPNC_SECULLLINECOUNT, gcmGETCOUNTER(counters_part1.se_culled_lines_count));
        gcmWRITE_COUNTER_NEW(VPNC_SECLIPTRIANGLECOUNT, gcmGETCOUNTER(counters_part1.se_clipped_triangle_count));
        gcmWRITE_COUNTER_NEW(VPNC_SECLIPLINECOUNT, gcmGETCOUNTER(counters_part1.se_clipped_line_count));
        gcmWRITE_COUNTER_NEW(VPNC_SESTARVECOUNT, gcmGETCOUNTER(counters_part1.se_starve_count));
        gcmWRITE_COUNTER_NEW(VPNC_SESTALLCOUNT, gcmGETCOUNTER(counters_part1.se_stall_count));
        gcmWRITE_COUNTER_NEW(VPNC_SERECEIVETRIANGLECOUNT, gcmGETCOUNTER(counters_part1.se_receive_triangle_count));
        gcmWRITE_COUNTER_NEW(VPNC_SESENDTRIANGLECOUNT, gcmGETCOUNTER(counters_part1.se_send_triangle_count));
        gcmWRITE_COUNTER_NEW(VPNC_SERECEIVELINESCOUNT, gcmGETCOUNTER(counters_part1.se_receive_lines_count));
        gcmWRITE_COUNTER_NEW(VPNC_SESENDLINESCOUNT, gcmGETCOUNTER(counters_part1.se_send_lines_count));
        gcmWRITE_COUNTER_NEW(VPNC_SEPROCESSCOUNT, gcmGETCOUNTER(counters_part1.se_process_count));
        gcmWRITE_COUNTER_NEW(VPNC_SETRIVIALREJLINECOUNT, gcmGETCOUNTER(counters_part1.se_trivial_rejected_line_count));
        gcmWRITE_COUNTER_NEW(VPNC_SENONIDLESTARVECOUNT, gcmGETCOUNTER(counters_part1.se_non_idle_starve_count));
        gcmWRITE_CONST_NEW(VPG_END);

        gcmWRITE_CONST_NEW(VPNG_RA);
        gcmWRITE_COUNTER_NEW(VPNC_RAINPUTPRIMCOUNT, gcmGETCOUNTER(counters_part1.ra_input_prim_count));
        gcmWRITE_COUNTER_NEW(VPNC_RATOTALQUADCOUNT, gcmGETCOUNTER(counters_part1.ra_total_quad_count));
        gcmWRITE_COUNTER_NEW(VPNC_RAPIPECACHEMISSCOUNT, gcmGETCOUNTER(counters_part1.ra_pipe_cache_miss_counter));
        gcmWRITE_COUNTER_NEW(VPNC_RAPIPEHZCACHEMISSCOUNT, gcmGETCOUNTER(counters_part1.ra_pipe_hz_cache_miss_counter));
        gcmWRITE_COUNTER_NEW(VPNC_RAVALIDQUADCOUNTEZ, gcmGETCOUNTER(counters_part1.ra_valid_quad_count_after_early_z));
        gcmWRITE_COUNTER_NEW(VPNC_RAVALIDPIXCOUNT, gcmGETCOUNTER(counters_part1.ra_valid_pixel_count_to_render));
        gcmWRITE_COUNTER_NEW(VPNC_RAOUTPUTQUADCOUNT, gcmGETCOUNTER(counters_part1.ra_output_valid_quad_count));
        gcmWRITE_COUNTER_NEW(VPNC_RAOUTPUTPIXELCOUNT, gcmGETCOUNTER(counters_part1.ra_output_valid_pixel_count));
        gcmWRITE_COUNTER_NEW(VPNC_RAPREFCACHEMISSCOUNT, gcmGETCOUNTER(counters_part1.ra_prefetch_cache_miss_counter));
        gcmWRITE_COUNTER_NEW(VPNC_RAPREFHZCACHEMISSCOUNT, gcmGETCOUNTER(counters_part1.ra_prefetch_hz_cache_miss_counter));
        gcmWRITE_COUNTER_NEW(VPNC_RAEEZCULLCOUNT, gcmGETCOUNTER(counters_part1.ra_eez_culled_counter));
        gcmWRITE_COUNTER_NEW(VPNC_RANONIDLESTARVECOUNT, gcmGETCOUNTER(counters_part1.ra_non_idle_starve_count));
        gcmWRITE_COUNTER_NEW(VPNC_RASTARVELCOUNT, gcmGETCOUNTER(counters_part1.ra_starve_count));
        gcmWRITE_COUNTER_NEW(VPNC_RASTALLCOUNT, gcmGETCOUNTER(counters_part1.ra_stall_count));
        gcmWRITE_COUNTER_NEW(VPNC_RAPROCESSCOUNT, gcmGETCOUNTER(counters_part1.ra_process_count));
        gcmWRITE_CONST_NEW(VPG_END);

        gcmWRITE_CONST_NEW(VPNG_TX);
        gcmWRITE_COUNTER_NEW(VPNC_TXTOTBILINEARREQ, gcmGETCOUNTER(counters_part1.tx_total_bilinear_requests));
        gcmWRITE_COUNTER_NEW(VPNC_TXTOTTRILINEARREQ, gcmGETCOUNTER(counters_part1.tx_total_trilinear_requests));
        gcmWRITE_COUNTER_NEW(VPNC_TXTOTDISCARDTEXREQ, gcmGETCOUNTER(counters_part1.tx_total_discarded_texture_requests));
        gcmWRITE_COUNTER_NEW(VPNC_TXTOTTEXREQ, gcmGETCOUNTER(counters_part1.tx_total_texture_requests));
        gcmWRITE_COUNTER_NEW(VPNC_TXMC0MISSCOUNT, gcmGETCOUNTER(counters_part1.tx_mc0_miss_count));
        gcmWRITE_COUNTER_NEW(VPNC_TXMC0REQCOUNT, gcmGETCOUNTER(counters_part1.tx_mc0_request_byte_count));
        gcmWRITE_COUNTER_NEW(VPNC_TXMC1MISSCOUNT, gcmGETCOUNTER(counters_part1.tx_mc1_miss_count));
        gcmWRITE_COUNTER_NEW(VPNC_TXMC1REQCOUNT, gcmGETCOUNTER(counters_part1.tx_mc1_request_byte_count));
        gcmWRITE_CONST_NEW(VPG_END);

        gcmWRITE_CONST_NEW(VPNG_PS);
        if (!bSupportProbe && !bHalti4)
        {
            /*this counter only recode on the first shader core, so just multi shaderCoreCount here*/
            gcmWRITE_COUNTER_NEW(VPNC_PSINSTCOUNT, gcmGETCOUNTER(counters_part1.ps_inst_counter) * shaderCoreCount);
            if (chipModel == gcv2000 && chipRevision == 0x5108)
            {
                /* this counter is not correct on gc2000 510_rc8, so set the value invalid here*/
                gcmWRITE_COUNTER_NEW(VPNC_PSRENDEREDPIXCOUNT, 0xdeaddead);
            }
            else
            {
                /*this counter will caculate twice on each shader core, so need divide by 2 for each shader core*/
                Counters.counters_part1.ps_rendered_pixel_counter = Counters.counters_part1.ps_rendered_pixel_counter * (shaderCoreCount / 2);
                gcmWRITE_COUNTER_NEW(VPNC_PSRENDEREDPIXCOUNT, gcmGETCOUNTER(counters_part1.ps_rendered_pixel_counter));
            }
        }
        gcmWRITE_COUNTER_NEW(VPNC_PSBRANCHINSTCOUNT, gcmGETCOUNTER(counters_part1.ps_branch_inst_counter));
        gcmWRITE_COUNTER_NEW(VPNC_PSTEXLDINSTCOUNT, gcmGETCOUNTER(counters_part1.ps_texld_inst_counter));
        gcmWRITE_COUNTER_NEW(VPNC_PSNONIDLESTARVECOUNT, gcmGETCOUNTER(counters_part1.ps_non_idle_starve_count));
        gcmWRITE_COUNTER_NEW(VPNC_PSSTARVELCOUNT, gcmGETCOUNTER(counters_part1.ps_starve_count));
        gcmWRITE_COUNTER_NEW(VPNC_PSSTALLCOUNT, gcmGETCOUNTER(counters_part1.ps_stall_count));
        gcmWRITE_COUNTER_NEW(VPNC_PSPROCESSCOUNT, gcmGETCOUNTER(counters_part1.ps_process_count));
        gcmWRITE_COUNTER_NEW(VPNC_PSSHADERCYCLECOUNT, gcmGETCOUNTER(counters_part1.ps_shader_cycle_count));
        gcmWRITE_CONST_NEW(VPG_END);

        gcmWRITE_CONST_NEW(VPNG_PE);
        gcmWRITE_COUNTER_NEW(VPNC_PE0KILLEDBYCOLOR, gcmGETCOUNTER(counters_part1.pe0_pixel_count_killed_by_color_pipe));
        gcmWRITE_COUNTER_NEW(VPNC_PE0KILLEDBYDEPTH, gcmGETCOUNTER(counters_part1.pe0_pixel_count_killed_by_depth_pipe));
        gcmWRITE_COUNTER_NEW(VPNC_PE0DRAWNBYCOLOR, gcmGETCOUNTER(counters_part1.pe0_pixel_count_drawn_by_color_pipe));
        gcmWRITE_COUNTER_NEW(VPNC_PE0DRAWNBYDEPTH, gcmGETCOUNTER(counters_part1.pe0_pixel_count_drawn_by_depth_pipe));
        gcmWRITE_COUNTER_NEW(VPNC_PE1KILLEDBYCOLOR, gcmGETCOUNTER(counters_part1.pe1_pixel_count_killed_by_color_pipe));
        gcmWRITE_COUNTER_NEW(VPNC_PE1KILLEDBYDEPTH, gcmGETCOUNTER(counters_part1.pe1_pixel_count_killed_by_depth_pipe));
        gcmWRITE_COUNTER_NEW(VPNC_PE1DRAWNBYCOLOR, gcmGETCOUNTER(counters_part1.pe1_pixel_count_drawn_by_color_pipe));
        gcmWRITE_COUNTER_NEW(VPNC_PE1DRAWNBYDEPTH, gcmGETCOUNTER(counters_part1.pe1_pixel_count_drawn_by_depth_pipe));
        gcmWRITE_CONST_NEW(VPG_END);

        gcmWRITE_CONST_NEW(VPNG_MCC);
        gcmWRITE_COUNTER_NEW(VPNC_MCCREADREQ8BCOLORPIPE, gcmGETCOUNTER(counters_part2.mcc_total_read_req_8B_from_colorpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCCREADREQ8BSOCOLORPIPE, gcmGETCOUNTER(counters_part2.mcc_total_read_req_8B_sentout_from_colorpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCCWRITEREQ8BCOLORPIPE, gcmGETCOUNTER(counters_part2.mcc_total_write_req_8B_from_colorpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCCREADREQSOCOLORPIPE, gcmGETCOUNTER(counters_part2.mcc_total_read_req_sentout_from_colorpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCCWRITEREQCOLORPIPE, gcmGETCOUNTER(counters_part2.mcc_total_write_req_from_colorpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCCREADREQ8BDEPTHPIPE, gcmGETCOUNTER(counters_part2.mcc_total_read_req_8B_from_depthpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCCREADREQ8BSFDEPTHPIPE, gcmGETCOUNTER(counters_part2.mcc_total_read_req_8B_sentout_from_depthpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCCWRITEREQ8BDEPTHPIPE, gcmGETCOUNTER(counters_part2.mcc_total_write_req_8B_from_depthpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCCREADREQSFDEPTHPIPE, gcmGETCOUNTER(counters_part2.mcc_total_read_req_sentout_from_depthpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCCWRITEREQDEPTHPIPE, gcmGETCOUNTER(counters_part2.mcc_total_write_req_from_depthpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCCREADREQ8BOTHERPIPE, gcmGETCOUNTER(counters_part2.mcc_total_read_req_8B_from_others));
        gcmWRITE_COUNTER_NEW(VPNC_MCCWRITEREQ8BOTHERPIPE, gcmGETCOUNTER(counters_part2.mcc_total_write_req_8B_from_others));
        gcmWRITE_COUNTER_NEW(VPNC_MCCREADREQOTHERPIPE, gcmGETCOUNTER(counters_part2.mcc_total_read_req_from_others));
        gcmWRITE_COUNTER_NEW(VPNC_MCCWRITEREQOTHERPIPE, gcmGETCOUNTER(counters_part2.mcc_total_write_req_from_others));
        gcmWRITE_COUNTER_NEW(VPNC_MCCAXIMINLATENCY, Counters.counters_part2.mcc_axi_min_latency);
        gcmWRITE_COUNTER_NEW(VPNC_MCCAXIMAXLATENCY, Counters.counters_part2.mcc_axi_max_latency);
        gcmWRITE_COUNTER_NEW(VPNC_MCCAXITOTALLATENCY, gcmGETCOUNTER(counters_part2.mcc_axi_total_latency));
        gcmWRITE_COUNTER_NEW(VPNC_MCCAXISAMPLECOUNT, gcmGETCOUNTER(counters_part2.mcc_axi_sample_count));
        gcmWRITE_CONST_NEW(VPG_END);

        gcmWRITE_CONST_NEW(VPNG_MCZ);
        gcmWRITE_COUNTER_NEW(VPNC_MCZREADREQ8BCOLORPIPE, gcmGETCOUNTER(counters_part2.mcz_total_read_req_8B_from_colorpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCZREADREQ8BSOCOLORPIPE, gcmGETCOUNTER(counters_part2.mcz_total_read_req_8B_sentout_from_colorpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCZWRITEREQ8BCOLORPIPE, gcmGETCOUNTER(counters_part2.mcz_total_write_req_8B_from_colorpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCZREADREQSOCOLORPIPE, gcmGETCOUNTER(counters_part2.mcz_total_read_req_sentout_from_colorpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCZWRITEREQCOLORPIPE, gcmGETCOUNTER(counters_part2.mcz_total_write_req_from_colorpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCZREADREQ8BDEPTHPIPE, gcmGETCOUNTER(counters_part2.mcz_total_read_req_8B_from_depthpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCZREADREQ8BSFDEPTHPIPE, gcmGETCOUNTER(counters_part2.mcz_total_read_req_8B_sentout_from_depthpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCZWRITEREQ8BDEPTHPIPE, gcmGETCOUNTER(counters_part2.mcz_total_write_req_8B_from_depthpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCZREADREQSFDEPTHPIPE, gcmGETCOUNTER(counters_part2.mcz_total_read_req_sentout_from_depthpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCZWRITEREQDEPTHPIPE, gcmGETCOUNTER(counters_part2.mcz_total_write_req_from_depthpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCZREADREQ8BOTHERPIPE, gcmGETCOUNTER(counters_part2.mcz_total_read_req_8B_from_others));
        gcmWRITE_COUNTER_NEW(VPNC_MCZWRITEREQ8BOTHERPIPE, gcmGETCOUNTER(counters_part2.mcz_total_write_req_8B_from_others));
        gcmWRITE_COUNTER_NEW(VPNC_MCZREADREQOTHERPIPE, gcmGETCOUNTER(counters_part2.mcz_total_read_req_from_others));
        gcmWRITE_COUNTER_NEW(VPNC_MCZWRITEREQOTHERPIPE, gcmGETCOUNTER(counters_part2.mcz_total_write_req_from_others));
        gcmWRITE_COUNTER_NEW(VPNC_MCZAXIMINLATENCY, Counters.counters_part2.mcz_axi_min_latency);
        gcmWRITE_COUNTER_NEW(VPNC_MCZAXIMAXLATENCY, Counters.counters_part2.mcz_axi_max_latency);
        gcmWRITE_COUNTER_NEW(VPNC_MCZAXITOTALLATENCY, gcmGETCOUNTER(counters_part2.mcz_axi_total_latency));
        gcmWRITE_COUNTER_NEW(VPNC_MCZAXISAMPLECOUNT, gcmGETCOUNTER(counters_part2.mcz_axi_sample_count));
        gcmWRITE_CONST_NEW(VPG_END);

        if (bSupportProbe)
        {
            /*the bandwidth counter need multiply by 2 when AXI bus is 128 bits*/
            if (axiBus128bits)
            {
                Counters.counters_part2.hi0_total_read_8B_count *= 2;
                Counters.counters_part2.hi0_total_write_8B_count *= 2;
                Counters.counters_part2.hi1_total_read_8B_count *= 2;
                Counters.counters_part2.hi1_total_write_8B_count *= 2;
            }
            Counters.counters_part2.hi_total_read_8B_count = Counters.counters_part2.hi0_total_read_8B_count + Counters.counters_part2.hi1_total_read_8B_count;
            Counters.counters_part2.hi_total_write_8B_count = Counters.counters_part2.hi0_total_write_8B_count + Counters.counters_part2.hi1_total_write_8B_count;
        }
        /*non probe mode */
        else if (axiBus128bits)
        {
            Counters.counters_part2.hi_total_read_8B_count *= 2;
            Counters.counters_part2.hi_total_write_8B_count *= 2;
        }
        gcmWRITE_CONST_NEW(VPNG_HI);
        gcmWRITE_COUNTER_NEW(VPNC_HI0READ8BYTE, gcmGETCOUNTER(counters_part2.hi0_total_read_8B_count));
        gcmWRITE_COUNTER_NEW(VPNC_HI0WRITE8BYTE, gcmGETCOUNTER(counters_part2.hi0_total_write_8B_count));
        gcmWRITE_COUNTER_NEW(VPNC_HI0READREQ, gcmGETCOUNTER(counters_part2.hi0_total_read_request_count));
        gcmWRITE_COUNTER_NEW(VPNC_HI0WRITEREQ, gcmGETCOUNTER(counters_part2.hi0_total_write_request_count));
        gcmWRITE_COUNTER_NEW(VPNC_HI0AXIREADREQSTALL, gcmGETCOUNTER(counters_part2.hi0_axi_cycles_read_request_stalled));
        gcmWRITE_COUNTER_NEW(VPNC_HI0AXIWRITEREQSTALL, gcmGETCOUNTER(counters_part2.hi0_axi_cycles_write_request_stalled));
        gcmWRITE_COUNTER_NEW(VPNC_HI0AXIWRITEDATASTALL, gcmGETCOUNTER(counters_part2.hi0_axi_cycles_write_data_stalled));
        gcmWRITE_COUNTER_NEW(VPNC_HI1READ8BYTE, gcmGETCOUNTER(counters_part2.hi1_total_read_8B_count));
        gcmWRITE_COUNTER_NEW(VPNC_HI1WRITE8BYTE, gcmGETCOUNTER(counters_part2.hi1_total_write_8B_count));
        gcmWRITE_COUNTER_NEW(VPNC_HI1READREQ, gcmGETCOUNTER(counters_part2.hi1_total_read_request_count));
        gcmWRITE_COUNTER_NEW(VPNC_HI1WRITEREQ, gcmGETCOUNTER(counters_part2.hi1_total_write_request_count));
        gcmWRITE_COUNTER_NEW(VPNC_HI1AXIREADREQSTALL, gcmGETCOUNTER(counters_part2.hi1_axi_cycles_read_request_stalled));
        gcmWRITE_COUNTER_NEW(VPNC_HI1AXIWRITEREQSTALL, gcmGETCOUNTER(counters_part2.hi1_axi_cycles_write_request_stalled));
        gcmWRITE_COUNTER_NEW(VPNC_HI1AXIWRITEDATASTALL, gcmGETCOUNTER(counters_part2.hi1_axi_cycles_write_data_stalled));
        gcmWRITE_COUNTER_NEW(VPNC_HITOTALCYCLES, gcmGETCOUNTER(counters_part2.hi_total_cycle_count));
        gcmWRITE_COUNTER_NEW(VPNC_HIIDLECYCLES, gcmGETCOUNTER(counters_part2.hi_total_idle_cycle_count));
        gcmWRITE_COUNTER_NEW(VPNC_HIREAD8BYTE, gcmGETCOUNTER(counters_part2.hi_total_read_8B_count));
        gcmWRITE_COUNTER_NEW(VPNC_HIWRITE8BYTE, gcmGETCOUNTER(counters_part2.hi_total_write_8B_count));
        gcmWRITE_CONST_NEW(VPG_END);

        gcmWRITE_CONST_NEW(VPNG_L2);
        gcmWRITE_COUNTER_NEW(VPNC_L2AXI0READREQCOUNT, gcmGETCOUNTER(counters_part2.l2_total_axi0_read_request_count));
        gcmWRITE_COUNTER_NEW(VPNC_L2AXI1READREQCOUNT, gcmGETCOUNTER(counters_part2.l2_total_axi1_read_request_count));
        gcmWRITE_COUNTER_NEW(VPNC_L2AXI0WRITEREQCOUNT, gcmGETCOUNTER(counters_part2.l2_total_axi0_write_request_count));
        gcmWRITE_COUNTER_NEW(VPNC_L2AXI1WRITEREQCOUNT, gcmGETCOUNTER(counters_part2.l2_total_axi1_write_request_count));
        gcmWRITE_COUNTER_NEW(VPNC_L2READTRANSREQBYAXI0, gcmGETCOUNTER(counters_part2.l2_total_read_transactions_request_by_axi0));
        gcmWRITE_COUNTER_NEW(VPNC_L2READTRANSREQBYAXI1, gcmGETCOUNTER(counters_part2.l2_total_read_transactions_request_by_axi1));
        gcmWRITE_COUNTER_NEW(VPNC_L2WRITETRANSREQBYAXI0, gcmGETCOUNTER(counters_part2.l2_total_write_transactions_request_by_axi0));
        gcmWRITE_COUNTER_NEW(VPNC_L2WRITETRANSREQBYAXI1, gcmGETCOUNTER(counters_part2.l2_total_write_transactions_request_by_axi1));
        gcmWRITE_COUNTER_NEW(VPNC_L2AXI0MINLATENCY, Counters.counters_part2.l2_axi0_min_latency);
        gcmWRITE_COUNTER_NEW(VPNC_L2AXI0MAXLATENCY, Counters.counters_part2.l2_axi0_max_latency);
        gcmWRITE_COUNTER_NEW(VPNC_L2AXI0TOTLATENCY, gcmGETCOUNTER(counters_part2.l2_axi0_total_latency));
        gcmWRITE_COUNTER_NEW(VPNC_L2AXI0TOTREQCOUNT, gcmGETCOUNTER(counters_part2.l2_axi0_total_request_count));
        gcmWRITE_COUNTER_NEW(VPNC_L2AXI1MINLATENCY, Counters.counters_part2.l2_axi1_min_latency);
        gcmWRITE_COUNTER_NEW(VPNC_L2AXI1MAXLATENCY, Counters.counters_part2.l2_axi1_max_latency);
        gcmWRITE_COUNTER_NEW(VPNC_L2AXI1TOTLATENCY, gcmGETCOUNTER(counters_part2.l2_axi1_total_latency));
        gcmWRITE_COUNTER_NEW(VPNC_L2AXI1TOTREQCOUNT, gcmGETCOUNTER(counters_part2.l2_axi1_total_request_count));
        gcmWRITE_CONST_NEW(VPG_END);

        gcmWRITE_CONST_NEW(VPG_END);
    }

    if (OpType == gcvCOUNTER_OP_DRAW && Profiler->perDrawMode)
    {
        gcoOS_MemCopy(&precounters, &Counters, gcmSIZEOF(precounters));
    }


    gcmFOOTER();
    return status;
}

/* Write data to profile file. */
gceSTATUS
gcoPROFILER_NEW_Write(
    IN gcoPROFILER Profiler,
    IN gctSIZE_T ByteCount,
    IN gctCONST_POINTER Data
)
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Profiler=0x%x ByteCount=%lu Data=0x%x", Profiler, ByteCount, Data);

    if (!Profiler)
    {
        gcmFOOTER();
        return gcvSTATUS_NOT_SUPPORTED;
    }
    /* Check if already destroyed. */
    if (Profiler->enable)
    {
        status = gcoOS_Write(gcvNULL,
            Profiler->file,
            ByteCount, Data);
    }

    gcmFOOTER();
    return status;
}

gceSTATUS
gcoPROFILER_NEW_GetPos(
    IN gcoPROFILER Profiler,
    OUT gctUINT32 * Position
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Profiler=0x%x ", Profiler);

    if (!Profiler)
    {
        gcmFOOTER();
        return gcvSTATUS_NOT_SUPPORTED;
    }
    /* Check if already destroyed. */
    if (Profiler->enable)
    {
        status = gcoOS_GetPos(gcvNULL,
            Profiler->file,
            Position);
    }

    gcmFOOTER();
    return status;
}

gceSTATUS
gcoPROFILER_NEW_Seek(
    IN gcoPROFILER Profiler,
    IN gctUINT32 Offset,
    IN gceFILE_WHENCE Whence
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Profiler=0x%x ", Profiler);

    if (!Profiler)
    {
        gcmFOOTER();
        return gcvSTATUS_NOT_SUPPORTED;
    }
    /* Check if already destroyed. */
    if (Profiler->enable)
    {
        status = gcoOS_Seek(gcvNULL,
            Profiler->file,
            Offset, Whence);
    }

    gcmFOOTER();
    return status;
}

/* Flush data out. */
gceSTATUS
gcoPROFILER_NEW_Flush(
    IN gcoPROFILER Profiler
)
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Profiler=0x%x", Profiler);

    if (!Profiler)
    {
        gcmFOOTER();
        return gcvSTATUS_NOT_SUPPORTED;
    }
    if (Profiler->enable)
    {
        status = gcoOS_Flush(gcvNULL, Profiler->file);
    }

    gcmFOOTER();
    return status;
}

gceSTATUS
gcoPROFILER_Initialize(
    IN gcoHAL Hal,
    IN gco3D Engine,
    IN gctBOOL Enable
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctCHAR profilerName[256] = { '\0' };
    gctCHAR inputFileName[256] = { '\0' };
    gctCHAR* fileName = gcvNULL;
    gctCHAR* filter = gcvNULL;
    gctCHAR* env = gcvNULL;
    gctSTRING portName;
    gctINT port;
    gcoHARDWARE hardware = gcvNULL;
    gceCHIPMODEL chipModel;
    gctUINT32 chipRevision;
    gcsHAL_INTERFACE iface;
#ifdef ANDROID
    gctBOOL matchResult = gcvFALSE;
#endif
    gctUINT32 coreCount = 1;
    gctUINT32 coreId;
    gctUINT32 originalCoreIndex;

    gcmHEADER();

    gcmGETHARDWARE(hardware);

    if (Enable)
    {
        gcoHAL_ConfigPowerManagement(gcvFALSE);
    }
    else
    {
        gcoHAL_ConfigPowerManagement(gcvTRUE);
        /* disable profiler in kernel. */
        iface.ignoreTLS = gcvFALSE;
        iface.command = gcvHAL_SET_PROFILE_SETTING;
        iface.u.SetProfileSetting.enable = gcvFALSE;

        /* Call the kernel. */
        status = gcoOS_DeviceControl(gcvNULL,
            IOCTL_GCHAL_INTERFACE,
            &iface, gcmSIZEOF(iface),
            &iface, gcmSIZEOF(iface));

        gcmFOOTER();
        return status;
    }

    if(!_HAL)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_NOT_SUPPORTED;
    }
    glhal_map_create(_HAL);

#ifdef ANDROID
    gcoOS_GetEnv(gcvNULL, "VP_PROCESS_NAME", &env);
    if ((env != gcvNULL) && (env[0] !=0)) matchResult = (gcoOS_DetectProcessByName(env) ? gcvTRUE : gcvFALSE);
    if(matchResult != gcvTRUE) {
        return gcvSTATUS_MISMATCH;
    }
    env= gcvNULL;
#endif

    gcoOS_ZeroMemory(&_HAL->profiler, gcmSIZEOF(_HAL->profiler));

    gcoOS_GetEnv(gcvNULL,
        "VP_COUNTER_FILTER",
        &filter);

    /* Enable/Disable specific counters. */
    if ((filter != gcvNULL))
    {
        gctUINT bitsLen = (gctUINT) gcoOS_StrLen(filter, gcvNULL);
        if (bitsLen > 2)
        {
            _HAL->profiler.enableHal = (filter[2] == '1');
        }
        else
        {
            _HAL->profiler.enableHal = gcvTRUE;
        }

        if (bitsLen > 3)
        {
            _HAL->profiler.enableHW = (filter[3] == '1');
        }
        else
        {
            _HAL->profiler.enableHW = gcvTRUE;
        }

        if (bitsLen > 8)
        {
            _HAL->profiler.enableSH = (filter[8] == '1');
        }
        else
        {
            _HAL->profiler.enableSH = gcvTRUE;
        }
    }
    else
    {
        _HAL->profiler.enableHal = gcvTRUE;
        _HAL->profiler.enableHW = gcvTRUE;
        _HAL->profiler.enableSH = gcvTRUE;
    }

    gcoOS_GetEnv(gcvNULL,
        "VP_OUTPUT",
        &fileName);
    _HAL->profiler.useSocket = gcvFALSE;
    if (fileName && *fileName != '\0' && *fileName != ' ')
    {
        /* Extract port info. */
        gcoOS_StrFindReverse(fileName, ':', &portName);

        if (portName)
        {
            gcoOS_StrToInt(portName + 1, &port);

            if (port > 0)
            {
                /*status = gcoOS_Socket(gcvNULL, AF_INET, SOCK_STREAM, 0, &gcPLS.hal->profiler.sockFd);*/
                status = gcoOS_Socket(gcvNULL, 2, 1, 0, &_HAL->profiler.sockFd);

                if (gcmIS_SUCCESS(status))
                {
                    *portName = '\0';
                    status = gcoOS_Connect(gcvNULL,
                            _HAL->profiler.sockFd, fileName, port);
                    *portName = ':';

                    if (gcmIS_SUCCESS(status))
                    {
                        _HAL->profiler.useSocket = gcvTRUE;
                    }
                }
            }
        }
    }
    if(fileName && !_HAL->profiler.useSocket) gcoOS_StrCatSafe(inputFileName,256,fileName);

    _HAL->profiler.enablePrint = gcvFALSE;
    gcoOS_GetEnv(gcvNULL,
        "VP_ENABLE_PRINT",
        &env);
    if ((env != gcvNULL) && gcmIS_SUCCESS(gcoOS_StrCmp(env, "1")))
    {
        _HAL->profiler.enablePrint = gcvTRUE;
    }

    _HAL->profiler.isCLMode = gcvFALSE;
    gcoOS_GetEnv(gcvNULL,
        "VIV_CL_PROFILE",
        &env);
    if ((env != gcvNULL) && gcmIS_SUCCESS(gcoOS_StrCmp(env, "1")))
    {
        _HAL->profiler.isCLMode = gcvTRUE;
        _HAL->profiler.enablePrint = gcvTRUE;
        _HAL->profiler.enableHal = gcvFALSE;
    }

    gcoOS_GetEnv(gcvNULL,
        "VIV_VX_PROFILE",
        &env);
    if ((env != gcvNULL) && gcmIS_SUCCESS(gcoOS_StrCmp(env, "1")))
    {
        _HAL->profiler.isCLMode = gcvTRUE;
        _HAL->profiler.enablePrint = gcvTRUE;
        _HAL->profiler.enableHal = gcvFALSE;
    }

    _HAL->profiler.disableOutputCounter = gcvFALSE;
    if ((!fileName || *fileName == '\0' || *fileName == ' ') && !_HAL->profiler.useSocket)
    {
        fileName = DEFAULT_PROFILE_FILE_NAME;
        if(fileName) gcoOS_StrCatSafe(inputFileName,256,fileName);
    }

    if (! _HAL->profiler.useSocket)
    {
        /*generate file name for each context*/
        gctHANDLE pid = gcoOS_GetCurrentProcessID();
        static gctUINT8 num = 1;
        gctUINT offset = 0;
        char* pos = gcvNULL;

        gcoOS_StrStr (inputFileName, ".vpd",&pos);
        if(pos) pos[0] = '\0';
        gcoOS_PrintStrSafe(profilerName, gcmSIZEOF(profilerName), &offset, "%s_%d_%d.vpd",inputFileName,(gctUINTPTR_T)pid, num);

        num++;

        status = gcoOS_Open(gcvNULL,
            profilerName,
            gcvFILE_CREATE,
            &_HAL->profiler.file);
    }

    if (gcmIS_ERROR(status))
    {
        _HAL->profiler.enable = 0;
        status = gcvSTATUS_GENERIC_IO;

        gcmFOOTER();
        return status;
    }

    _HAL->profiler.isSyncMode = gcvTRUE;
    gcoOS_GetEnv(gcvNULL,
        "VP_SYNC_MODE",
        &env);

    if ((env != gcvNULL) && gcmIS_SUCCESS(gcoOS_StrCmp(env, "0")))
    {
        _HAL->profiler.isSyncMode = gcvFALSE;
    }

    /* enable profiler in kernel. */
    iface.ignoreTLS = gcvFALSE;
    iface.command = gcvHAL_SET_PROFILE_SETTING;
    iface.u.SetProfileSetting.enable = gcvTRUE;
    if (_HAL->profiler.isCLMode)
        iface.u.SetProfileSetting.syncMode = gcvFALSE;
    else
        iface.u.SetProfileSetting.syncMode = _HAL->profiler.isSyncMode;

    gcoHARDWARE_Query3DCoreCount(gcvNULL, &coreCount);
    gcmONERROR(gcoHAL_GetCurrentCoreIndex(gcvNULL, &originalCoreIndex));
    for (coreId = 0; coreId < coreCount; coreId++)
    {

        gctUINT32 coreIndex;
        /* Convert coreID in this hardware to global core index. */
        gcmONERROR(gcoHARDWARE_QueryCoreIndex(gcvNULL, coreId, &coreIndex));
        /* Set it to TLS to find correct command queue. */
        gcmONERROR(gcoHAL_SetCoreIndex(gcvNULL, coreIndex));

        /* Call the kernel. */
        status = gcoOS_DeviceControl(gcvNULL,
        IOCTL_GCHAL_INTERFACE,
        &iface, gcmSIZEOF(iface),
        &iface, gcmSIZEOF(iface));
    }
    /* Restore core index in TLS. */
    gcmONERROR(gcoHAL_SetCoreIndex(gcvNULL, originalCoreIndex));

    if (gcmIS_ERROR(status))
    {
        _HAL->profiler.enable = 0;

        gcmFOOTER();
        return status;
    }

    gcmVERIFY_OK(gcoHAL_QueryChipIdentity(gcvNULL, &chipModel, &chipRevision, gcvNULL, gcvNULL));

    gcoHARDWARE_EnableCounters(gcvNULL);

    _HAL->profiler.enable = 1;
    gcoOS_GetTime(&_HAL->profiler.frameStart);
    _HAL->profiler.frameStartTimeusec = _HAL->profiler.frameStart;

    gcmWRITE_CONST(VPHEADER);
    gcmWRITE_BUFFER(4, "VP12");

OnError:
    /* Success. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoPROFILER_Destroy(
    IN gcoHAL Hal
    )
{
    gcmHEADER();
    if(!_HAL)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_NOT_SUPPORTED;
    }

    if (_HAL->profiler.enable)
    {
        /* Close the profiler file. */
        gcmWRITE_CONST(VPG_END);

        gcoPROFILER_Flush(gcvNULL);
        if (_HAL->profiler.useSocket)
        {
            /* Close the socket. */
            gcmVERIFY_OK(gcoOS_CloseSocket(gcvNULL, _HAL->profiler.sockFd));
        }
        else
        {
            /* Close the profiler file. */
            gcmVERIFY_OK(gcoOS_Close(gcvNULL, _HAL->profiler.file));
        }
    }

    glhal_map_delete(_HAL);

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/* Write data to profile. */
gceSTATUS
gcoPROFILER_Write(
    IN gcoHAL Hal,
    IN gctSIZE_T ByteCount,
    IN gctCONST_POINTER Data
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("ByteCount=%lu Data=0x%x", ByteCount, Data);
    if(!_HAL)
    {
        gcmFOOTER();
        return gcvSTATUS_NOT_SUPPORTED;
    }
    /* Check if already destroyed. */
    if (_HAL->profiler.enable)
    {
        if (_HAL->profiler.useSocket)
        {
            status = gcoOS_Send(gcvNULL,
                                _HAL->profiler.sockFd,
                                ByteCount, Data, 0);
        }
        else
        {
            status = gcoOS_Write(gcvNULL,
                                 _HAL->profiler.file,
                                 ByteCount, Data);
        }
    }

    gcmFOOTER();
    return status;
}

/* Flush data out. */
gceSTATUS
gcoPROFILER_Flush(
    IN gcoHAL Hal
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER();
    if(!_HAL)
    {
        gcmFOOTER();
        return gcvSTATUS_NOT_SUPPORTED;
    }
    if (_HAL->profiler.enable)
    {
        if (_HAL->profiler.useSocket)
        {
            status = gcoOS_WaitForSend(gcvNULL,
                                       _HAL->profiler.sockFd,
                                       0, 50000);
        }
        else
        {
            status = gcoOS_Flush(gcvNULL,
                                 _HAL->profiler.file);
        }
    }

    gcmFOOTER();
    return status;
}

gceSTATUS
gcoPROFILER_Count(
    IN gcoHAL Hal,
    IN gctUINT32 Enum,
    IN gctINT Value
    )
{
#if PROFILE_HAL_COUNTERS
    gcmHEADER_ARG("Enum=%lu Value=%d", Enum, Value);

    if(!_HAL) _HAL = glhal_map_current();

    if(!_HAL)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_NOT_SUPPORTED;
    }
    if (_HAL->profiler.enable)
    {
        switch (Enum)
        {
        case GLINDEX_OBJECT:
            _HAL->profiler.indexBufferNewObjectsAlloc   += Value;
            _HAL->profiler.indexBufferTotalObjectsAlloc += Value;
            break;

        case GLINDEX_OBJECT_BYTES:
            _HAL->profiler.indexBufferNewBytesAlloc   += Value;
            _HAL->profiler.indexBufferTotalBytesAlloc += Value;
            break;

        case GLVERTEX_OBJECT:
            _HAL->profiler.vertexBufferNewObjectsAlloc   += Value;
            _HAL->profiler.vertexBufferTotalObjectsAlloc += Value;
            break;

        case GLVERTEX_OBJECT_BYTES:
            _HAL->profiler.vertexBufferNewBytesAlloc   += Value;
            _HAL->profiler.vertexBufferTotalBytesAlloc += Value;
            break;

        case GLTEXTURE_OBJECT:
            _HAL->profiler.textureBufferNewObjectsAlloc   += Value;
            _HAL->profiler.textureBufferTotalObjectsAlloc += Value;
            break;

        case GLTEXTURE_OBJECT_BYTES:
            _HAL->profiler.textureBufferNewBytesAlloc   += Value;
            _HAL->profiler.textureBufferTotalBytesAlloc += Value;
            break;

        default:
            break;
        }
    }
#endif

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}


gceSTATUS
gcoPROFILER_ShaderVS(
    IN gcoHAL Hal,
    IN gctPOINTER Vs
    )
{
    gcmHEADER_ARG("Vs=0x%x", Vs);
    if(!_HAL)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_NOT_SUPPORTED;
    }
    if (_HAL->profiler.enable)
    {
        if (_HAL->profiler.enableSH)
        {
            gctUINT16 alu = 0, tex = 0, i;
            gcSHADER Shader = (gcSHADER)Vs;

            /* Profile shader */
            for (i = 0; i < GetShaderCodeCount(Shader); i++ )
            {
                switch (gcmSL_OPCODE_GET(GetInstOpcode(GetShaderInstruction(Shader, i)), Opcode))
                {
                case gcSL_NOP:
                    break;

                case gcSL_TEXLD:
                    tex++;
                    break;

                default:
                    alu++;
                    break;
                }
            }

            gcmWRITE_CONST(VPG_PVS);

            gcmWRITE_COUNTER(VPC_PVSINSTRCOUNT, (tex + alu));
            gcmWRITE_COUNTER(VPC_PVSALUINSTRCOUNT, alu);
            gcmWRITE_COUNTER(VPC_PVSTEXINSTRCOUNT, tex);
            gcmWRITE_COUNTER(VPC_PVSATTRIBCOUNT, (GetShaderAttributeCount(Shader)));
            gcmWRITE_COUNTER(VPC_PVSUNIFORMCOUNT, (GetShaderUniformCount(Shader)));
            gcmWRITE_COUNTER(VPC_PVSFUNCTIONCOUNT, (GetShaderFunctionCount(Shader)));
            if (GetShaderSourceCode(Shader))
            {
                gcmWRITE_CONST(VPC_PVSSOURCE);
                gcmWRITE_STRING(GetShaderSourceCode(Shader));
            }
            gcmWRITE_CONST(VPG_END);
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
gcoPROFILER_ShaderFS(
    IN gcoHAL Hal,
    IN void* Fs
    )
{
    gcmHEADER_ARG("Fs=0x%x", Fs);
    if(!_HAL)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_NOT_SUPPORTED;
    }
    if (_HAL->profiler.enable)
    {
        if (_HAL->profiler.enableSH)
        {
            gctUINT16 alu = 0, tex = 0, i;
            gcSHADER Shader = (gcSHADER)Fs;

            /* Profile shader */
            for (i = 0; i < GetShaderCodeCount(Shader); i++ )
            {
                switch (gcmSL_OPCODE_GET(GetInstOpcode(GetShaderInstruction(Shader, i)), Opcode))
                {
                case gcSL_NOP:
                    break;

                case gcSL_TEXLD:
                    tex++;
                    break;

                default:
                    alu++;
                    break;
                }
            }

            gcmWRITE_CONST(VPG_PPS);
            gcmWRITE_COUNTER(VPC_PPSINSTRCOUNT, (tex + alu));
            gcmWRITE_COUNTER(VPC_PPSALUINSTRCOUNT, alu);
            gcmWRITE_COUNTER(VPC_PPSTEXINSTRCOUNT, tex);
            gcmWRITE_COUNTER(VPC_PPSATTRIBCOUNT, (GetShaderAttributeCount(Shader)));
            gcmWRITE_COUNTER(VPC_PPSUNIFORMCOUNT, (GetShaderUniformCount(Shader)));
            gcmWRITE_COUNTER(VPC_PPSFUNCTIONCOUNT, (GetShaderFunctionCount(Shader)));
            if (GetShaderSourceCode(Shader))
            {
                gcmWRITE_CONST(VPC_PPSSOURCE);
                gcmWRITE_STRING(GetShaderSourceCode(Shader));
            }
            gcmWRITE_CONST(VPG_END);
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}


gceSTATUS
gcoPROFILER_EndFrame(
                     IN gcoHAL Hal
                     )
{
    gcsHAL_INTERFACE iface;
    gceSTATUS status;
    gcoHARDWARE hardware = gcvNULL;
    gctUINT32 context;
    gctUINT32 coreCount = 1;
    gctUINT32 coreId;
    gctUINT32 originalCoreIndex;

    gcmHEADER();
    if(!_HAL)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_NOT_SUPPORTED;
    }

    if (!_HAL->profiler.enable)
    {
        gcoPROFILER_Begin(Hal, gcvCOUNTER_OP_NONE);

        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    /*#if PROFILE_HAL_COUNTERS*/
    if (_HAL->profiler.enableHal)
    {
        if (!_HAL->profiler.disableOutputCounter)
        {
            gcmWRITE_CONST(VPG_HAL);

            gcmWRITE_COUNTER(VPC_HALVERTBUFNEWBYTEALLOC, _HAL->profiler.vertexBufferNewBytesAlloc);
            gcmWRITE_COUNTER(VPC_HALVERTBUFTOTALBYTEALLOC, _HAL->profiler.vertexBufferTotalBytesAlloc);
            gcmWRITE_COUNTER(VPC_HALVERTBUFNEWOBJALLOC, _HAL->profiler.vertexBufferNewObjectsAlloc);
            gcmWRITE_COUNTER(VPC_HALVERTBUFTOTALOBJALLOC, _HAL->profiler.vertexBufferTotalObjectsAlloc);

            gcmWRITE_COUNTER(VPC_HALINDBUFNEWBYTEALLOC, _HAL->profiler.indexBufferNewBytesAlloc);
            gcmWRITE_COUNTER(VPC_HALINDBUFTOTALBYTEALLOC, _HAL->profiler.indexBufferTotalBytesAlloc);
            gcmWRITE_COUNTER(VPC_HALINDBUFNEWOBJALLOC, _HAL->profiler.indexBufferNewObjectsAlloc);
            gcmWRITE_COUNTER(VPC_HALINDBUFTOTALOBJALLOC, _HAL->profiler.indexBufferTotalObjectsAlloc);

            gcmWRITE_COUNTER(VPC_HALTEXBUFNEWBYTEALLOC, _HAL->profiler.textureBufferNewBytesAlloc);
            gcmWRITE_COUNTER(VPC_HALTEXBUFTOTALBYTEALLOC, _HAL->profiler.textureBufferTotalBytesAlloc);
            gcmWRITE_COUNTER(VPC_HALTEXBUFNEWOBJALLOC, _HAL->profiler.textureBufferNewObjectsAlloc);
            gcmWRITE_COUNTER(VPC_HALTEXBUFTOTALOBJALLOC, _HAL->profiler.textureBufferTotalObjectsAlloc);

            gcmWRITE_CONST(VPG_END);
        }

        /* Reset per-frame counters. */
        _HAL->profiler.vertexBufferNewBytesAlloc   = 0;
        _HAL->profiler.vertexBufferNewObjectsAlloc = 0;

        _HAL->profiler.indexBufferNewBytesAlloc   = 0;
        _HAL->profiler.indexBufferNewObjectsAlloc = 0;

        _HAL->profiler.textureBufferNewBytesAlloc   = 0;
        _HAL->profiler.textureBufferNewObjectsAlloc = 0;
    }
    /*#endif*/

    /*#if PROFILE_HW_COUNTERS*/
    /* gcvHAL_READ_ALL_PROFILE_REGISTERS. */
    if (_HAL->profiler.enableHW)
    {
#define gcmCOUNTER(name)    iface.u.RegisterProfileData.counters.name

        if (_HAL->profiler.isCLMode)
            gcoHAL_Commit(gcvNULL, gcvTRUE);
        iface.command = gcvHAL_READ_ALL_PROFILE_REGISTERS;
        iface.ignoreTLS = gcvFALSE;

        gcmGETHARDWARE(hardware);
        if (hardware)
        {
            gctBOOL axiBus128bits;

            gcoHAL_QueryChipAxiBusWidth(&axiBus128bits);
            gcoHARDWARE_Query3DCoreCount(gcvNULL, &coreCount);
            gcmONERROR(gcoHAL_GetCurrentCoreIndex(gcvNULL, &originalCoreIndex));

            for (coreId = 0; coreId < coreCount; coreId++)
            {
                gctUINT32 coreIndex;

                /* Convert coreID in this hardware to global core index. */
                gcmONERROR(gcoHARDWARE_QueryCoreIndex(gcvNULL, coreId, &coreIndex));

                /* Set it to TLS to find correct command queue. */
                gcmONERROR(gcoHAL_SetCoreIndex(gcvNULL, coreIndex));

                /* Call kernel service. */
                gcmVERIFY_OK(gcoHARDWARE_GetContext(hardware, &context));
                if (context != 0)
                    iface.u.RegisterProfileData.context = context;

                status = gcoOS_DeviceControl(gcvNULL,
                    IOCTL_GCHAL_INTERFACE,
                    &iface, gcmSIZEOF(iface),
                    &iface, gcmSIZEOF(iface));

                if (!gcmNO_ERROR(status) || _HAL->profiler.disableOutputCounter)
                    continue;

                if (coreCount == 1)
                {
                    gcmWRITE_CONST(VPG_HW);
                }
                else
                {
                    gcmWRITE_CONST(VPG_MULTI_GPU);
                }

                gcmWRITE_CONST(VPG_GPU);
                gcmWRITE_COUNTER(VPC_GPUREAD64BYTE, gcmCOUNTER(gpuTotalRead64BytesPerFrame));
                gcmWRITE_COUNTER(VPC_GPUWRITE64BYTE, gcmCOUNTER(gpuTotalWrite64BytesPerFrame));
                gcmWRITE_COUNTER(VPC_GPUCYCLES, gcmCOUNTER(gpuCyclesCounter));
                gcmWRITE_COUNTER(VPC_GPUTOTALCYCLES, gcmCOUNTER(gpuTotalCyclesCounter));
                gcmWRITE_COUNTER(VPC_GPUIDLECYCLES, gcmCOUNTER(gpuIdleCyclesCounter));
                gcmWRITE_CONST(VPG_END);

                gcmWRITE_CONST(VPG_FE);
                gcmWRITE_COUNTER(VPC_FEDRAWCOUNT, gcmCOUNTER(fe_draw_count) );
                gcmWRITE_COUNTER(VPC_FEOUTVERTEXCOUNT, gcmCOUNTER(fe_out_vertex_count));
                gcmWRITE_COUNTER(VPC_FESTALLCOUNT, gcmCOUNTER(fe_stall_count));
                gcmWRITE_COUNTER(VPC_FESTARVECOUNT, gcmCOUNTER(fe_starve_count));
                gcmWRITE_CONST(VPG_END);

                gcmWRITE_CONST(VPG_VS);
                gcmWRITE_COUNTER(VPC_VSINSTCOUNT, gcmCOUNTER(vs_inst_counter));
                gcmWRITE_COUNTER(VPC_VSBRANCHINSTCOUNT, gcmCOUNTER(vtx_branch_inst_counter));
                gcmWRITE_COUNTER(VPC_VSTEXLDINSTCOUNT, gcmCOUNTER(vtx_texld_inst_counter));
                gcmWRITE_COUNTER(VPC_VSRENDEREDVERTCOUNT, gcmCOUNTER(rendered_vertice_counter));
                gcmWRITE_COUNTER(VPC_VSNONIDLESTARVECOUNT, gcmCOUNTER(vs_non_idle_starve_count));
                gcmWRITE_COUNTER(VPC_VSSTARVELCOUNT, gcmCOUNTER(vs_starve_count));
                gcmWRITE_COUNTER(VPC_VSSTALLCOUNT, gcmCOUNTER(vs_stall_count));
                gcmWRITE_COUNTER(VPC_VSPROCESSCOUNT, gcmCOUNTER(vs_process_count));
                gcmWRITE_CONST(VPG_END);

                gcmWRITE_CONST(VPG_PA);
                gcmWRITE_COUNTER(VPC_PAINVERTCOUNT, gcmCOUNTER(pa_input_vtx_counter));
                gcmWRITE_COUNTER(VPC_PAINPRIMCOUNT, gcmCOUNTER(pa_input_prim_counter));
                gcmWRITE_COUNTER(VPC_PAOUTPRIMCOUNT, gcmCOUNTER(pa_output_prim_counter));
                gcmWRITE_COUNTER(VPC_PADEPTHCLIPCOUNT, gcmCOUNTER(pa_depth_clipped_counter));
                gcmWRITE_COUNTER(VPC_PATRIVIALREJCOUNT, gcmCOUNTER(pa_trivial_rejected_counter));
                gcmWRITE_COUNTER(VPC_PACULLCOUNT, gcmCOUNTER(pa_culled_counter));
                gcmWRITE_COUNTER(VPC_PANONIDLESTARVECOUNT, gcmCOUNTER(pa_non_idle_starve_count));
                gcmWRITE_COUNTER(VPC_PASTARVELCOUNT, gcmCOUNTER(pa_starve_count));
                gcmWRITE_COUNTER(VPC_PASTALLCOUNT, gcmCOUNTER(pa_stall_count));
                gcmWRITE_COUNTER(VPC_PAPROCESSCOUNT, gcmCOUNTER(pa_process_count));
                gcmWRITE_CONST(VPG_END);

                gcmWRITE_CONST(VPG_SETUP);
                gcmWRITE_COUNTER(VPC_SETRIANGLECOUNT, gcmCOUNTER(se_culled_triangle_count));
                gcmWRITE_COUNTER(VPC_SELINECOUNT, gcmCOUNTER(se_culled_lines_count));
                gcmWRITE_COUNTER(VPC_SESTARVECOUNT, gcmCOUNTER(se_starve_count));
                gcmWRITE_COUNTER(VPC_SESTALLCOUNT, gcmCOUNTER(se_stall_count));
                gcmWRITE_COUNTER(VPC_SERECEIVETRIANGLECOUNT, gcmCOUNTER(se_receive_triangle_count));
                gcmWRITE_COUNTER(VPC_SESENDTRIANGLECOUNT, gcmCOUNTER(se_send_triangle_count));
                gcmWRITE_COUNTER(VPC_SERECEIVELINESCOUNT, gcmCOUNTER(se_receive_lines_count));
                gcmWRITE_COUNTER(VPC_SESENDLINESCOUNT, gcmCOUNTER(se_send_lines_count));
                gcmWRITE_COUNTER(VPC_SEPROCESSCOUNT, gcmCOUNTER(se_process_count));
                gcmWRITE_COUNTER(VPC_SENONIDLESTARVECOUNT, gcmCOUNTER(se_non_idle_starve_count));
                gcmWRITE_CONST(VPG_END);

                gcmWRITE_CONST(VPG_RA);
                gcmWRITE_COUNTER(VPC_RAVALIDPIXCOUNT, gcmCOUNTER(ra_valid_pixel_count));
                gcmWRITE_COUNTER(VPC_RATOTALQUADCOUNT, gcmCOUNTER(ra_total_quad_count));
                gcmWRITE_COUNTER(VPC_RAVALIDQUADCOUNTEZ, gcmCOUNTER(ra_valid_quad_count_after_early_z));
                gcmWRITE_COUNTER(VPC_RATOTALPRIMCOUNT, gcmCOUNTER(ra_total_primitive_count));
                gcmWRITE_COUNTER(VPC_RAPIPECACHEMISSCOUNT, gcmCOUNTER(ra_pipe_cache_miss_counter));
                gcmWRITE_COUNTER(VPC_RAPREFCACHEMISSCOUNT, gcmCOUNTER(ra_prefetch_cache_miss_counter));
                gcmWRITE_COUNTER(VPC_RAEEZCULLCOUNT, gcmCOUNTER(ra_eez_culled_counter));
                gcmWRITE_COUNTER(VPC_RANONIDLESTARVECOUNT, gcmCOUNTER(ra_non_idle_starve_count));
                gcmWRITE_COUNTER(VPC_RASTARVELCOUNT, gcmCOUNTER(ra_starve_count));
                gcmWRITE_COUNTER(VPC_RASTALLCOUNT, gcmCOUNTER(ra_stall_count));
                gcmWRITE_COUNTER(VPC_RAPROCESSCOUNT, gcmCOUNTER(ra_process_count));
                gcmWRITE_CONST(VPG_END);

                gcmWRITE_CONST(VPG_TX);
                gcmWRITE_COUNTER(VPC_TXTOTBILINEARREQ, gcmCOUNTER(tx_total_bilinear_requests));
                gcmWRITE_COUNTER(VPC_TXTOTTRILINEARREQ, gcmCOUNTER(tx_total_trilinear_requests));
                gcmWRITE_COUNTER(VPC_TXTOTTEXREQ, gcmCOUNTER(tx_total_texture_requests));
                gcmWRITE_COUNTER(VPC_TXMEMREADCOUNT, gcmCOUNTER(tx_mem_read_count));
                gcmWRITE_COUNTER(VPC_TXMEMREADIN8BCOUNT, gcmCOUNTER(tx_mem_read_in_8B_count));
                gcmWRITE_COUNTER(VPC_TXCACHEMISSCOUNT, gcmCOUNTER(tx_cache_miss_count));
                gcmWRITE_COUNTER(VPC_TXCACHEHITTEXELCOUNT, gcmCOUNTER(tx_cache_hit_texel_count));
                gcmWRITE_COUNTER(VPC_TXCACHEMISSTEXELCOUNT, gcmCOUNTER(tx_cache_miss_texel_count));
                gcmWRITE_COUNTER(VPC_TXNONIDLESTARVECOUNT, gcmCOUNTER(tx_non_idle_starve_count));
                gcmWRITE_COUNTER(VPC_TXSTARVELCOUNT, gcmCOUNTER(tx_starve_count));
                gcmWRITE_COUNTER(VPC_TXSTALLCOUNT, gcmCOUNTER(tx_stall_count));
                gcmWRITE_COUNTER(VPC_TXPROCESSCOUNT, gcmCOUNTER(tx_process_count));
                gcmWRITE_CONST(VPG_END);

                gcmWRITE_CONST(VPG_PS);
                gcmWRITE_COUNTER(VPC_PSINSTCOUNT, gcmCOUNTER(ps_inst_counter) );
                gcmWRITE_COUNTER(VPC_PSBRANCHINSTCOUNT, gcmCOUNTER(pxl_branch_inst_counter));
                gcmWRITE_COUNTER(VPC_PSTEXLDINSTCOUNT, gcmCOUNTER(pxl_texld_inst_counter));
                gcmWRITE_COUNTER(VPC_PSRENDEREDPIXCOUNT, gcmCOUNTER(rendered_pixel_counter));
                gcmWRITE_COUNTER(VPC_PSNONIDLESTARVECOUNT, gcmCOUNTER(ps_non_idle_starve_count));
                gcmWRITE_COUNTER(VPC_PSSTARVELCOUNT, gcmCOUNTER(ps_starve_count));
                gcmWRITE_COUNTER(VPC_PSSTALLCOUNT, gcmCOUNTER(ps_stall_count));
                gcmWRITE_COUNTER(VPC_PSPROCESSCOUNT, gcmCOUNTER(ps_process_count));
                gcmWRITE_COUNTER(VPC_PSSHADERCYCLECOUNT, gcmCOUNTER(shader_cycle_count));
                gcmWRITE_CONST(VPG_END);

                gcmWRITE_CONST(VPG_PE);
                gcmWRITE_COUNTER(VPC_PEKILLEDBYCOLOR, gcmCOUNTER(pe_pixel_count_killed_by_color_pipe));
                gcmWRITE_COUNTER(VPC_PEKILLEDBYDEPTH, gcmCOUNTER(pe_pixel_count_killed_by_depth_pipe));
                gcmWRITE_COUNTER(VPC_PEDRAWNBYCOLOR, gcmCOUNTER(pe_pixel_count_drawn_by_color_pipe));
                gcmWRITE_COUNTER(VPC_PEDRAWNBYDEPTH, gcmCOUNTER(pe_pixel_count_drawn_by_depth_pipe));
                gcmWRITE_CONST(VPG_END);

                gcmWRITE_CONST(VPG_MC);
                gcmWRITE_COUNTER(VPC_MCREADREQ8BPIPE, gcmCOUNTER(mc_total_read_req_8B_from_pipeline));
                gcmWRITE_COUNTER(VPC_MCREADREQ8BIP, gcmCOUNTER(mc_total_read_req_8B_from_IP));
                gcmWRITE_COUNTER(VPC_MCWRITEREQ8BPIPE, gcmCOUNTER(mc_total_write_req_8B_from_pipeline));
                gcmWRITE_COUNTER(VPC_MCAXIMINLATENCY, gcmCOUNTER(mc_axi_min_latency));
                gcmWRITE_COUNTER(VPC_MCAXIMAXLATENCY, gcmCOUNTER(mc_axi_max_latency));
                gcmWRITE_COUNTER(VPC_MCAXITOTALLATENCY, gcmCOUNTER(mc_axi_total_latency));
                gcmWRITE_COUNTER(VPC_MCAXISAMPLECOUNT, gcmCOUNTER(mc_axi_sample_count));
                gcmWRITE_CONST(VPG_END);

                gcmWRITE_CONST(VPG_AXI);
                gcmWRITE_COUNTER(VPC_AXIREADREQSTALLED, gcmCOUNTER(hi_axi_cycles_read_request_stalled));
                gcmWRITE_COUNTER(VPC_AXIWRITEREQSTALLED, gcmCOUNTER(hi_axi_cycles_write_request_stalled));
                gcmWRITE_COUNTER(VPC_AXIWRITEDATASTALLED, gcmCOUNTER(hi_axi_cycles_write_data_stalled));
                gcmWRITE_CONST(VPG_END);

                gcmWRITE_CONST(VPG_END);

                if (_HAL->profiler.enablePrint)
                {
                    static gctUINT32 frameNo = 0;
                    if (_HAL->profiler.isCLMode)
                    {
                        gcmPRINT("VPC_Kernel %d\n", frameNo);
                    }
                    else
                    {
                        gcmPRINT("frame %d\n", frameNo);
                    }
                    gcmPRINT("*********\n");
                    if (coreCount > 1)
                    {
                        gcmPRINT("GPU #%d\n", coreId);
                        gcmPRINT("*********\n");
                        if (coreId == coreCount - 1)
                            frameNo++;
                    }
                    else
                    {
                        frameNo++;
                    }

                    if (_HAL->profiler.isCLMode)
                    {
                        /* simplify the messages for vx demo */
                        /* 0.00000095367 = 1 / 1024 / 1024*/
                        if (axiBus128bits)
                        {
                            gcmPRINT("READ_BANDWIDTH  (MByte): %f\n", gcmCOUNTER(gpuTotalRead64BytesPerFrame) * 16 * 0.00000095367);
                            gcmPRINT("WRITE_BANDWIDTH (MByte): %f\n", gcmCOUNTER(gpuTotalWrite64BytesPerFrame) * 16 * 0.00000095367);
                        }
                        else
                        {
                            gcmPRINT("READ_BANDWIDTH  (MByte): %f\n", gcmCOUNTER(gpuTotalRead64BytesPerFrame) * 8 * 0.00000095367);
                            gcmPRINT("WRITE_BANDWIDTH (MByte): %f\n", gcmCOUNTER(gpuTotalWrite64BytesPerFrame) * 8 * 0.00000095367);
                        }
                    }
                    gcmPRINT("VPG_GPU\n");
                    gcmPRINT("VPC_GPUREAD64BYTE: %d\n", gcmCOUNTER(gpuTotalRead64BytesPerFrame));
                    gcmPRINT("VPC_GPUWRITE64BYTE: %d\n", gcmCOUNTER(gpuTotalWrite64BytesPerFrame));
                    gcmPRINT("VPC_GPUCYCLES: %d\n", gcmCOUNTER(gpuCyclesCounter));
                    /* This counter should be equal to gpuCyclesCounter. Currently it's not displayed in vAnalyzer
                    gcmPRINT("VPC_GPUTOTALCYCLES: %d\n", gcmCOUNTER(gpuTotalCyclesCounter)); */
                    gcmPRINT("VPC_GPUIDLECYCLES: %d\n", gcmCOUNTER(gpuIdleCyclesCounter));
                    gcmPRINT("*********\n");

                    /* these counters are not needed in CL profiler */
                    if (!_HAL->profiler.isCLMode)
                    {
                    gcmPRINT("VPG_FE\n");
                    gcmPRINT("VPC_FEDRAWCOUNT: %d\n", gcmCOUNTER(fe_draw_count));
                    gcmPRINT("VPC_FEOUTVERTEXCOUNT: %d\n", gcmCOUNTER(fe_out_vertex_count));
                    gcmPRINT("VPC_FESTALLCOUNT: %d\n", gcmCOUNTER(fe_stall_count));
                    gcmPRINT("VPC_FESTARVECOUNT: %d\n", gcmCOUNTER(fe_starve_count));
                    gcmPRINT("*********\n");

                    gcmPRINT("VPG_VS\n");
                    gcmPRINT("VPC_VSINSTCOUNT: %d\n", gcmCOUNTER(vs_inst_counter));
                    gcmPRINT("VPC_VSBRANCHINSTCOUNT: %d\n", gcmCOUNTER(vtx_branch_inst_counter));
                    gcmPRINT("VPC_VSTEXLDINSTCOUNT: %d\n", gcmCOUNTER(vtx_texld_inst_counter));
                    gcmPRINT("VPC_VSRENDEREDVERTCOUNT: %d\n", gcmCOUNTER(rendered_vertice_counter));
                    gcmPRINT("VPC_VSNONIDLESTARVECOUNT: %d\n", gcmCOUNTER(vs_non_idle_starve_count));
                    gcmPRINT("VPC_VSSTARVELCOUNT: %d\n", gcmCOUNTER(vs_starve_count));
                    gcmPRINT("VPC_VSSTALLCOUNT: %d\n", gcmCOUNTER(vs_stall_count));
                    gcmPRINT("VPC_VSPROCESSCOUNT: %d\n", gcmCOUNTER(vs_process_count));
                    gcmPRINT("*********\n");

                    gcmPRINT("VPG_PA\n");
                    gcmPRINT("VPC_PAINVERTCOUNT: %d\n", gcmCOUNTER(pa_input_vtx_counter));
                    gcmPRINT("VPC_PAINPRIMCOUNT: %d\n", gcmCOUNTER(pa_input_prim_counter));
                    gcmPRINT("VPC_PAOUTPRIMCOUNT: %d\n", gcmCOUNTER(pa_output_prim_counter));
                    gcmPRINT("VPC_PADEPTHCLIPCOUNT: %d\n", gcmCOUNTER(pa_depth_clipped_counter));
                    gcmPRINT("VPC_PATRIVIALREJCOUNT: %d\n", gcmCOUNTER(pa_trivial_rejected_counter));
                    gcmPRINT("VPC_PACULLCOUNT: %d\n", gcmCOUNTER(pa_culled_counter));
                    gcmPRINT("VPC_PANONIDLESTARVECOUNT: %d\n", gcmCOUNTER(pa_non_idle_starve_count));
                    gcmPRINT("VPC_PASTARVELCOUNT: %d\n", gcmCOUNTER(pa_starve_count));
                    gcmPRINT("VPC_PASTALLCOUNT: %d\n", gcmCOUNTER(pa_stall_count));
                    gcmPRINT("VPC_PAPROCESSCOUNT: %d\n", gcmCOUNTER(pa_process_count));
                    gcmPRINT("*********\n");

                    gcmPRINT("VPG_SETUP\n");
                    gcmPRINT("VPC_SETRIANGLECOUNT: %d\n", gcmCOUNTER(se_culled_triangle_count));
                    gcmPRINT("VPC_SELINECOUNT: %d\n", gcmCOUNTER(se_culled_lines_count));
                    gcmPRINT("VPC_SESTARVECOUNT: %d\n", gcmCOUNTER(se_starve_count));
                    gcmPRINT("VPC_SESTALLCOUNT: %d\n", gcmCOUNTER(se_stall_count));
                    gcmPRINT("VPC_SERECEIVETRIANGLECOUNT: %d\n", gcmCOUNTER(se_receive_triangle_count));
                    gcmPRINT("VPC_SESENDTRIANGLECOUNT: %d\n", gcmCOUNTER(se_send_triangle_count));
                    gcmPRINT("VPC_SERECEIVELINESCOUNT: %d\n", gcmCOUNTER(se_receive_lines_count));
                    gcmPRINT("VPC_SESENDLINESCOUNT: %d\n", gcmCOUNTER(se_send_lines_count));
                    gcmPRINT("VPC_SEPROCESSCOUNT: %d\n", gcmCOUNTER(se_process_count));
                    gcmPRINT("VPC_SENONIDLESTARVECOUNT: %d\n", gcmCOUNTER(se_non_idle_starve_count));
                    gcmPRINT("*********\n");

                    gcmPRINT("VPG_RA\n");
                    gcmPRINT("VPC_RAVALIDPIXCOUNT: %d\n", gcmCOUNTER(ra_valid_pixel_count));
                    gcmPRINT("VPC_RATOTALQUADCOUNT: %d\n", gcmCOUNTER(ra_total_quad_count));
                    gcmPRINT("VPC_RAVALIDQUADCOUNTEZ: %d\n", gcmCOUNTER(ra_valid_quad_count_after_early_z));
                    gcmPRINT("VPC_RATOTALPRIMCOUNT: %d\n", gcmCOUNTER(ra_total_primitive_count));
                    gcmPRINT("VPC_RAPIPECACHEMISSCOUNT: %d\n", gcmCOUNTER(ra_pipe_cache_miss_counter));
                    gcmPRINT("VPC_RAPREFCACHEMISSCOUNT: %d\n", gcmCOUNTER(ra_prefetch_cache_miss_counter));
                    gcmPRINT("VPC_RAEEZCULLCOUNT: %d\n", gcmCOUNTER(ra_eez_culled_counter));
                    gcmPRINT("VPC_RANONIDLESTARVECOUNT: %d\n", gcmCOUNTER(ra_non_idle_starve_count));
                    gcmPRINT("VPC_RASTARVELCOUNT: %d\n", gcmCOUNTER(ra_starve_count));
                    gcmPRINT("VPC_RASTALLCOUNT: %d\n", gcmCOUNTER(ra_stall_count));
                    gcmPRINT("VPC_RAPROCESSCOUNT: %d\n", gcmCOUNTER(ra_process_count));
                    gcmPRINT("*********\n");

                    gcmPRINT("VPG_TX\n");
                    gcmPRINT("VPC_TXTOTBILINEARREQ: %d\n", gcmCOUNTER(tx_total_bilinear_requests));
                    gcmPRINT("VPC_TXTOTTRILINEARREQ: %d\n", gcmCOUNTER(tx_total_trilinear_requests));
                    gcmPRINT("VPC_TXTOTTEXREQ: %d\n", gcmCOUNTER(tx_total_texture_requests));
                    gcmPRINT("VPC_TXMEMREADCOUNT: %d\n", gcmCOUNTER(tx_mem_read_count));
                    gcmPRINT("VPC_TXMEMREADIN8BCOUNT: %d\n", gcmCOUNTER(tx_mem_read_in_8B_count));
                    gcmPRINT("VPC_TXCACHEMISSCOUNT: %d\n", gcmCOUNTER(tx_cache_miss_count));
                    gcmPRINT("VPC_TXCACHEHITTEXELCOUNT: %d\n", gcmCOUNTER(tx_cache_hit_texel_count));
                    gcmPRINT("VPC_TXCACHEMISSTEXELCOUNT: %d\n", gcmCOUNTER(tx_cache_miss_texel_count));
                    gcmPRINT("VPC_TXNONIDLESTARVECOUNT: %d\n", gcmCOUNTER(tx_non_idle_starve_count));
                    gcmPRINT("VPC_TXSTARVELCOUNT: %d\n", gcmCOUNTER(tx_starve_count));
                    gcmPRINT("VPC_TXSTALLCOUNT: %d\n", gcmCOUNTER(tx_stall_count));
                    gcmPRINT("VPC_TXPROCESSCOUNT: %d\n", gcmCOUNTER(tx_process_count));
                    gcmPRINT("*********\n");
                    }

                    gcmPRINT("VPG_PS\n");
                    gcmPRINT("VPC_PSINSTCOUNT: %d\n", gcmCOUNTER(ps_inst_counter) );
                    gcmPRINT("VPC_PSBRANCHINSTCOUNT: %d\n", gcmCOUNTER(pxl_branch_inst_counter));
                    gcmPRINT("VPC_PSTEXLDINSTCOUNT: %d\n", gcmCOUNTER(pxl_texld_inst_counter));
                    gcmPRINT("VPC_PSRENDEREDPIXCOUNT: %d\n", gcmCOUNTER(rendered_pixel_counter));
                    gcmPRINT("VPC_PSNONIDLESTARVECOUNT: %d\n", gcmCOUNTER(ps_non_idle_starve_count) );
                    gcmPRINT("VPC_PSSTARVELCOUNT: %d\n", gcmCOUNTER(ps_starve_count));
                    gcmPRINT("VPC_PSSTALLCOUNT: %d\n", gcmCOUNTER(ps_stall_count));
                    gcmPRINT("VPC_PSPROCESSCOUNT: %d\n", gcmCOUNTER(ps_process_count));;
                    gcmPRINT("VPC_PSSHADERCYCLECOUNT: %d\n", gcmCOUNTER(shader_cycle_count));
                    gcmPRINT("*********\n");

                    if (!_HAL->profiler.isCLMode)
                    {
                    gcmPRINT("VPG_PE\n");
                    gcmPRINT("VPC_PEKILLEDBYCOLOR: %d\n", gcmCOUNTER(pe_pixel_count_killed_by_color_pipe));
                    gcmPRINT("VPC_PEKILLEDBYDEPTH: %d\n", gcmCOUNTER(pe_pixel_count_killed_by_depth_pipe));
                    gcmPRINT("VPC_PEDRAWNBYCOLOR: %d\n", gcmCOUNTER(pe_pixel_count_drawn_by_color_pipe));
                    gcmPRINT("VPC_PEDRAWNBYDEPTH: %d\n", gcmCOUNTER(pe_pixel_count_drawn_by_depth_pipe));
                    gcmPRINT("*********\n");
                    }

                    gcmPRINT("VPG_MC\n");
                    if (!_HAL->profiler.isCLMode)
                    {
                    gcmPRINT("VPC_MCREADREQ8BPIPE: %d\n", gcmCOUNTER(mc_total_read_req_8B_from_pipeline));
                    gcmPRINT("VPC_MCREADREQ8BIP: %d\n", gcmCOUNTER(mc_total_read_req_8B_from_IP));
                    gcmPRINT("VPC_MCWRITEREQ8BPIPE: %d\n", gcmCOUNTER(mc_total_write_req_8B_from_pipeline));
                    }
                    gcmPRINT("VPC_MCAXIMINLATENCY: %d\n", gcmCOUNTER(mc_axi_min_latency));
                    gcmPRINT("VPC_MCAXIMAXLATENCY: %d\n", gcmCOUNTER(mc_axi_max_latency));
                    gcmPRINT("VPC_MCAXITOTALLATENCY: %d\n", gcmCOUNTER(mc_axi_total_latency));
                    gcmPRINT("VPC_MCAXISAMPLECOUNT: %d\n", gcmCOUNTER(mc_axi_sample_count));
                    gcmPRINT("*********\n");

                    if (!_HAL->profiler.isCLMode)
                    {
                    gcmPRINT("VPG_AXI\n");
                    gcmPRINT("VPC_AXIREADREQSTALLED: %d\n", gcmCOUNTER(hi_axi_cycles_read_request_stalled));
                    gcmPRINT("VPC_AXIWRITEREQSTALLED: %d\n", gcmCOUNTER(hi_axi_cycles_write_request_stalled));
                    gcmPRINT("VPC_AXIWRITEDATASTALLED: %d\n", gcmCOUNTER(hi_axi_cycles_write_data_stalled));
                    gcmPRINT("*********\n");
                    }
                }
            }
            /* Restore core index in TLS. */
            gcmONERROR(gcoHAL_SetCoreIndex(gcvNULL, originalCoreIndex));
        }
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmFOOTER_NO();
    return status;
}

gceSTATUS
gcoPROFILER_Begin(
    IN gcoHAL Hal,
    IN gceCOUNTER_OPTYPE operationType
                    )
{
    gceSTATUS status;
    gcoHARDWARE hardware = gcvNULL;
    gcsTLS_PTR __tls;
    gcmHEADER();
    gcmONERROR(gcoOS_GetTLS(&__tls));

   if (!Hal->profiler.isCLMode)
   {
       gcmFOOTER_NO();
       return gcvSTATUS_OK;
   }

    /* reset gpu counters */
    if (_HAL->profiler.enableHW)
    {
        gcsHAL_INTERFACE iface;
        gctUINT32 context;
        gctUINT32 coreCount = 1;
        gctUINT32 coreId;
        gctUINT32 originalCoreIndex;

        gcoHAL_Commit(gcvNULL, gcvTRUE);
        iface.command = gcvHAL_READ_ALL_PROFILE_REGISTERS;
        iface.ignoreTLS = gcvFALSE;

        gcmGETHARDWARE(hardware);
        if (hardware)
        {
            gcoHARDWARE_Query3DCoreCount(gcvNULL, &coreCount);
            gcmONERROR(gcoHAL_GetCurrentCoreIndex(gcvNULL, &originalCoreIndex));

            for (coreId = 0; coreId < coreCount; coreId++)
            {
                gctUINT32 coreIndex;

                /* Convert coreID in this hardware to global core index. */
                gcmONERROR(gcoHARDWARE_QueryCoreIndex(gcvNULL, coreId, &coreIndex));

                /* Set it to TLS to find correct command queue. */
                gcmONERROR(gcoHAL_SetCoreIndex(gcvNULL, coreIndex));

                /* Call kernel service. */
                gcmVERIFY_OK(gcoHARDWARE_GetContext(hardware, &context));
                if (context != 0)
                    iface.u.RegisterProfileData.context = context;

                status = gcoOS_DeviceControl(gcvNULL,
                    IOCTL_GCHAL_INTERFACE,
                    &iface, gcmSIZEOF(iface),
                    &iface, gcmSIZEOF(iface));
            }
            /* Restore core index in TLS. */
            gcmONERROR(gcoHAL_SetCoreIndex(gcvNULL, originalCoreIndex));
        }
    }

OnError:
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
gcoPROFILER_End(
                    IN gcoHAL Hal,
                    IN gctBOOL FirstDraw
                    )
{
    return gcvSTATUS_OK;
}

#else
/* Stubs for Profiler functions. */

/* Initialize the gcsProfiler. */
gceSTATUS
gcoPROFILER_Initialize(
                       IN gcoHAL Hal,
                       IN gco3D Engine,
                       IN gctBOOL Enable
                       )
{
    gcmHEADER();
    gcmFOOTER_NO();
    return gcvSTATUS_INVALID_REQUEST;
}

/* Destroy the gcProfiler. */
gceSTATUS
gcoPROFILER_Destroy(
                    IN gcoHAL Hal
                    )
{
    gcmHEADER();
    gcmFOOTER_NO();
    return gcvSTATUS_INVALID_REQUEST;
}

/* Write data to profile. */
gceSTATUS
gcoPROFILER_Write(
                  IN gcoHAL Hal,
                  IN gctSIZE_T ByteCount,
                  IN gctCONST_POINTER Data
                  )
{
    gcmHEADER_ARG("ByteCount=%lu Data=0x%x", ByteCount, Data);

    gcmFOOTER_NO();
    return gcvSTATUS_INVALID_REQUEST;
}

/* Flush data out. */
gceSTATUS
gcoPROFILER_Flush(
                  IN gcoHAL Hal
                  )
{
    gcmHEADER();
    gcmFOOTER_NO();
    return gcvSTATUS_INVALID_REQUEST;
}

/* Call to signal end of frame. */
gceSTATUS
gcoPROFILER_EndFrame(
                     IN gcoHAL Hal
                     )
{
    gcmHEADER();
    gcmFOOTER_NO();
    return gcvSTATUS_INVALID_REQUEST;
}

gceSTATUS
gcoPROFILER_Begin(
    IN gcoHAL Hal,
    IN gceCOUNTER_OPTYPE operationType
                    )
{
    gcmHEADER();
    gcmFOOTER_NO();
    return gcvSTATUS_INVALID_REQUEST;
}

/* Call to signal end of draw. */
gceSTATUS
gcoPROFILER_End(
    IN gcoHAL Hal,
    IN gctBOOL FirstDraw
                    )
{
    gcmHEADER();
    gcmFOOTER_NO();
    return gcvSTATUS_INVALID_REQUEST;
}

gceSTATUS
gcoPROFILER_Count(
                  IN gcoHAL Hal,
                  IN gctUINT32 Enum,
                  IN gctINT Value
                  )
{
    gcmHEADER_ARG("Enum=%lu Value=%d", Enum, Value);

    gcmFOOTER_NO();
    return gcvSTATUS_INVALID_REQUEST;
}

/* Profile input vertex shader. */
gceSTATUS
gcoPROFILER_ShaderVS(
                     IN gcoHAL Hal,
                     IN gctPOINTER Vs
                     )
{
    gcmHEADER_ARG("Vs=0x%x", Vs);
    gcmFOOTER_NO();
    return gcvSTATUS_INVALID_REQUEST;
}

/* Profile input fragment shader. */
gceSTATUS
gcoPROFILER_ShaderFS(
                     IN gcoHAL Hal,
                     IN gctPOINTER Fs
                     )
{
    gcmHEADER_ARG("Fs=0x%x", Fs);
    gcmFOOTER_NO();
    return gcvSTATUS_INVALID_REQUEST;
}

#endif /* VIVANTE_PROFILER */
#endif /* gcdENABLE_3D */
