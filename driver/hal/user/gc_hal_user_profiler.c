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


/*******************************************************************************
**    Profiler for Vivante HAL.
*/
#include "gc_hal_user_precomp.h"

#define _GC_OBJ_ZONE            gcvZONE_HAL

#if gcdENABLE_3D
#if VIVANTE_PROFILER

#ifdef ANDROID
#define DEFAULT_PROFILE_FILE_NAME   "/sdcard/vprofiler.vpd"

#if VIVANTE_PROFILER_PROBE
#define DEFAULT_PROBE_FILE_NAME   "/sdcard/vprobe.xml"
#endif

#else
#define DEFAULT_PROFILE_FILE_NAME   "vprofiler.vpd"

#if VIVANTE_PROFILER_PROBE
#define DEFAULT_PROBE_FILE_NAME   "vprobe.xml"
#endif

#endif

#ifdef ANDROID
#define DEFAULT_PROFILE_NEW_FILE_NAME   "/sdcard/vprofiler.vpd"
#else
#define DEFAULT_PROFILE_NEW_FILE_NAME   "vprofiler.vpd"
#endif

#if gcdNEW_PROFILER_FILE
#if VIVANTE_PROFILER_CONTEXT
#define _HAL Hal
#else
#define _HAL gcPLS.hal
#endif

#if VIVANTE_PROFILER_CONTEXT
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
#else
static void glhal_map_create(gcoHAL Hal){}
static void glhal_map_delete(gcoHAL Hal){}
#endif

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

#if VIVANTE_PROFILER_PROBE
#define gcmWRITE_XML_STRING(String) \
    do \
    { \
    gceSTATUS status; \
    gctINT32 length; \
    length = (gctINT32) gcoOS_StrLen((gctSTRING) String, gcvNULL); \
    gcmERR_BREAK(gcoPROFILER_Write(_HAL, length, String)); \
    } \
    while (gcvFALSE)


#define gcmWRITE_XML_COUNTER(Counter,Value) \
    do \
    { \
    char buffer[256]; \
    gctUINT tempOffset = 0; \
    gceSTATUS status; \
    if (Value == 0xDEADDEAD) \
    { \
        gcmERR_BREAK(gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), \
        &tempOffset, \
        "<%s> %s </%s>\n", \
        Counter, \
        "invalid", \
        Counter)); \
        gcmWRITE_XML_STRING(buffer); \
    } \
    else \
    { \
        gcmERR_BREAK(gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), \
        &tempOffset, \
        "<%s> %d </%s>\n", \
        Counter, \
        Value, \
        Counter)); \
        gcmWRITE_XML_STRING(buffer); \
    } \
    } \
    while (gcvFALSE)

#define gcmWRITE_XML_Value(Counter,Value,Dec) \
    do \
    { \
    char buffer[256]; \
    gctUINT offset = 0; \
    gceSTATUS status; \
    gcmERR_BREAK(gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), \
    &offset, \
    Dec?"<%s>%d</%s>\n":"<%s>%x</%s>\n", \
    Counter, \
    Value, \
    Counter)); \
    gcmWRITE_XML_STRING(buffer); \
    } \
    while (gcvFALSE)
#endif

#endif


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

        for (; bufId < NumOfDrawBuf; bufId++)
        {
            gcmONERROR(gcoBUFOBJ_Construct(gcvNULL, gcvBUFOBJ_TYPE_GENERIC_BUFFER, &counterBuf));
            gcmONERROR(gcoBUFOBJ_Upload(counterBuf, gcvNULL, 0, gcmSIZEOF(gcsPROFILER_NEW_COUNTERS), gcvBUFOBJ_USAGE_STATIC_DRAW));
            gcoBUFOBJ_Lock(counterBuf, &address, gcvNULL);
            Profiler->counterBuf[bufId].probeAddress = address;
            Profiler->counterBuf[bufId].couterBufobj = (gctHANDLE)counterBuf;
            Profiler->counterBuf[bufId].opType = gcvCOUNTER_OP_NONE;
        }

        gcmONERROR(gcoHARDWARE_EnableCounters(gcvNULL));

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
        /* reset probe counter */
        if (Profiler->curBufId == 0)
        {
            gcmONERROR(gcoHARDWARE_SetProbeCmd(gcvNULL, gcvPROBECMD_BEGIN, Profiler->counterBuf[0].probeAddress, gcvNULL));
        }

        Profiler->counterBuf[Profiler->curBufId].opType = operationType;

        /* if the BufId beyond the limit, upload a new buffer */
        if (Profiler->curBufId >= NumOfDrawBuf)
        {
            gctINT32 i;
            gctPOINTER memory;

            gcmONERROR(gcoBUFOBJ_WaitFence(Profiler->counterBuf[NumOfDrawBuf-1].couterBufobj, gcvFENCE_TYPE_READ));

            for (i = 0; i <= Profiler->curBufId; i++)
            {
                gcmONERROR(gcoBUFOBJ_Lock(Profiler->counterBuf[i].couterBufobj, gcvNULL, &memory));

                gcmONERROR(gcoPROFILER_NEW_RecordCounters(memory, &Profiler->counterBuf[i].counters));

                gcmONERROR(gcoPROFILER_NEW_WriteCounters(Profiler, Profiler->counterBuf[i].counters, gcvFALSE));
            }

            Profiler->curBufId = 0;
            for (i = 0; i < NumOfDrawBuf; i++)
            {
                gcmONERROR(gcoBUFOBJ_Upload(Profiler->counterBuf[i].couterBufobj, gcvNULL, 0, gcmSIZEOF(gcsPROFILER_NEW_COUNTERS), gcvBUFOBJ_USAGE_STATIC_DRAW));
                Profiler->counterBuf[i].opType = gcvCOUNTER_OP_NONE;
            }
        }
    }
    else
    {
        /* reset profiler counter */
        if (Profiler->curBufId == 0)
        {
            gcsHAL_INTERFACE iface;
            gctUINT32 context;
            gctUINT32 coreCount = 1;
            gctUINT32 coreId;
            gctUINT32 originalCoreIndex;

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
        }

        Profiler->counterBuf[Profiler->curBufId].opType = operationType;

        /* if the BufId beyond the limit, upload a new buffer */
        if (Profiler->curBufId >= NumOfDrawBuf - 1)
        {
            gctINT32 i;

            for (i = 0; i <= Profiler->curBufId; i++)
            {
                gcmONERROR(gcoPROFILER_NEW_WriteCounters(Profiler, Profiler->counterBuf[i].counters, gcvFALSE));
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
    IN gctUINT32 DrawID
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

        Profiler->counterBuf[Profiler->curBufId].counters.drawID = DrawID;
        Profiler->curBufId++;
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
    IN gcoPROFILER Profiler
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
        gctPOINTER  memory;

        gcmONERROR(gcoBUFOBJ_GetFence(Profiler->counterBuf[Profiler->curBufId].couterBufobj, gcvFENCE_TYPE_READ));

        gcmONERROR(gcoHARDWARE_SetProbeCmd(gcvNULL, gcvPROBECMD_END, Profiler->counterBuf[Profiler->curBufId].probeAddress, gcvNULL));

        /*add a fence here, no need commit true*/
        gcmONERROR(gcoBUFOBJ_WaitFence(Profiler->counterBuf[Profiler->curBufId].couterBufobj, gcvFENCE_TYPE_READ));

        /*write this frame perdraw counter to file*/
        for (i = 0; i < Profiler->curBufId; i++)
        {
            gcmONERROR(gcoBUFOBJ_Lock(Profiler->counterBuf[i].couterBufobj, gcvNULL, &memory));

            gcmONERROR(gcoPROFILER_NEW_RecordCounters(memory, &Profiler->counterBuf[i].counters));

            gcmONERROR(gcoPROFILER_NEW_WriteCounters(Profiler, Profiler->counterBuf[i].counters, gcvFALSE));
        }

        /*write this frame counter to file*/
        gcmONERROR(gcoBUFOBJ_Lock(Profiler->counterBuf[Profiler->curBufId].couterBufobj, gcvNULL, &memory));

        gcmONERROR(gcoPROFILER_NEW_RecordCounters(memory, &Profiler->counterBuf[Profiler->curBufId].counters));

        gcmONERROR(gcoPROFILER_NEW_WriteCounters(Profiler, Profiler->counterBuf[Profiler->curBufId].counters, gcvTRUE));

        /*reset probe counters*/
        gcmONERROR(gcoHARDWARE_SetProbeCmd(gcvNULL, gcvPROBECMD_BEGIN, Profiler->counterBuf[0].probeAddress, gcvNULL));

        Profiler->curBufId = 0;
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
        iface.u.SetProfilerRegisterClear.bclear = gcvTRUE;

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
        Profiler->curBufId++;
        /* Restore core index in TLS. */
        gcmONERROR(gcoHAL_SetCoreIndex(gcvNULL, originalCoreIndex));

        Profiler->curBufId--;
        gcmONERROR(gcoPROFILER_NEW_WriteCounters(Profiler, Profiler->counterBuf[Profiler->curBufId].counters, gcvTRUE));

        Profiler->curBufId = 0;

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
    gctUINT32_PTR memory = gcvNULL;
    gctUINT32 min_latency = 0;
    gctUINT32 max_latency = 0;
    gctUINT32 offset = 0;

    gcmHEADER_ARG("Logical=0x%x, Counters=0x%x", Logical, Counters);

    gcmASSERT(gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_PROBE));

    memory = (gctUINT32_PTR)Logical;

    /* module FE */
    Counters->counters_part1.fe_out_vertex_count = *(memory + 0 + offset);
    Counters->counters_part1.fe_cache_miss_count = *(memory + 1 + offset);
    Counters->counters_part1.fe_cache_lk_count = *(memory + 2 + offset);
    Counters->counters_part1.fe_stall_count = *(memory + 3 + offset);
    Counters->counters_part1.fe_process_count = *(memory + 4 + offset);
    Counters->counters_part1.fe_starve_count = 0xDEADDEAD;
    offset += MODULE_FRONT_END_COUNTER_NUM;

    /* module VS */
    /*todo*/
    Counters->counters_part1.vs_shader_cycle_count = *(memory + 0 + offset);
    Counters->counters_part1.vs_inst_counter = *(memory + 3 + offset);
    Counters->counters_part1.vs_rendered_vertice_counter = *(memory + 4 + offset);
    Counters->counters_part1.vs_branch_inst_counter = *(memory + 5 + offset);
    Counters->counters_part1.vs_texld_inst_counter = *(memory + 6 + offset);
    offset += MODULE_VERTEX_SHADER_COUNTER_NUM;

    /* module PA */
    Counters->counters_part1.pa_input_vtx_counter = *(memory + 0 + offset);
    Counters->counters_part1.pa_input_prim_counter = *(memory + 1 + offset);
    Counters->counters_part1.pa_output_prim_counter = *(memory + 2 + offset);
    Counters->counters_part1.pa_trivial_rejected_counter = *(memory + 3 + offset);
    Counters->counters_part1.pa_culled_prim_counter = *(memory + 4 + offset);
    Counters->counters_part1.pa_droped_prim_counter = *(memory + 5 + offset);
    Counters->counters_part1.pa_frustum_clipped_prim_counter = *(memory + 6 + offset);
    Counters->counters_part1.pa_frustum_clipdroped_prim_counter = *(memory + 7 + offset);
    Counters->counters_part1.pa_non_idle_starve_count = *(memory + 8 + offset);
    Counters->counters_part1.pa_starve_count = *(memory + 9 + offset);
    Counters->counters_part1.pa_stall_count = *(memory + 10 + offset);
    Counters->counters_part1.pa_process_count = *(memory + 11 + offset);
    offset += MODULE_PRIMITIVE_ASSEMBLY_COUNTER_NUM;

    /* module SE */
    Counters->counters_part1.se_starve_count = *(memory + 0 + offset);
    Counters->counters_part1.se_stall_count = *(memory + 1 + offset);
    Counters->counters_part1.se_receive_triangle_count = *(memory + 2 + offset);
    Counters->counters_part1.se_send_triangle_count = *(memory + 3 + offset);
    Counters->counters_part1.se_receive_lines_count = *(memory + 4 + offset);
    Counters->counters_part1.se_send_lines_count = *(memory + 5 + offset);
    Counters->counters_part1.se_process_count = *(memory + 6 + offset);
    Counters->counters_part1.se_clipped_triangle_count = *(memory + 7 + offset);
    Counters->counters_part1.se_clipped_line_count = *(memory + 8 + offset);
    Counters->counters_part1.se_culled_lines_count = *(memory + 9 + offset);
    Counters->counters_part1.se_culled_triangle_count = *(memory + 10 + offset);
    Counters->counters_part1.se_trivial_rejected_line_count = *(memory + 11 + offset);
    Counters->counters_part1.se_non_idle_starve_count = *(memory + 12 + offset);
    offset += MODULE_SETUP_COUNTER_NUM;

    /* module RA */
    Counters->counters_part1.ra_input_prim_count = *(memory + 0 + offset);
    Counters->counters_part1.ra_total_quad_count = *(memory + 1 + offset);
    Counters->counters_part1.ra_prefetch_cache_miss_counter = *(memory + 2 + offset);
    Counters->counters_part1.ra_prefetch_hz_cache_miss_counter = *(memory + 3 + offset);
    Counters->counters_part1.ra_valid_quad_count_after_early_z = *(memory + 4 + offset);
    Counters->counters_part1.ra_valid_pixel_count_to_render = *(memory + 5 + offset);
    Counters->counters_part1.ra_output_valid_quad_count = *(memory + 6 + offset);
    Counters->counters_part1.ra_output_valid_pixel_count = *(memory + 7 + offset);
    /*need confirm with hw*/
    Counters->counters_part1.ra_pipe_cache_miss_counter = *(memory + 8 + offset);
    Counters->counters_part1.ra_pipe_hz_cache_miss_counter = *(memory + 9 + offset);
    Counters->counters_part1.ra_non_idle_starve_count = *(memory + 10 + offset);
    Counters->counters_part1.ra_starve_count = *(memory + 11 + offset);
    Counters->counters_part1.ra_stall_count = *(memory + 12 + offset);
    Counters->counters_part1.ra_process_count = *(memory + 12 + offset);
    offset += MODULE_RASTERIZER_COUNTER_NUM;

    /* module PS */
    Counters->counters_part1.ps_shader_cycle_count = *(memory + 0 + offset);
    Counters->counters_part1.ps_inst_counter = *(memory + 1 + offset);
    Counters->counters_part1.ps_rendered_pixel_counter = *(memory + 2 + offset);
    Counters->counters_part1.ps_branch_inst_counter = *(memory + 7 + offset);
    Counters->counters_part1.ps_texld_inst_counter = *(memory + 8 + offset);
    offset += MODULE_PIXEL_SHADER_COUNTER_NUM;

    /* module TX */
    Counters->counters_part1.tx_total_bilinear_requests = *(memory + 0 + offset);
    Counters->counters_part1.tx_total_trilinear_requests = *(memory + 1 + offset);
    Counters->counters_part1.tx_total_discarded_texture_requests = *(memory + 2 + offset);
    Counters->counters_part1.tx_total_texture_requests = *(memory + 3 + offset);
    Counters->counters_part1.tx_mc0_miss_count = *(memory + 4 + offset);
    Counters->counters_part1.tx_mc0_request_byte_count = *(memory + 5 + offset);
    Counters->counters_part1.tx_mc1_miss_count = *(memory + 6 + offset);
    Counters->counters_part1.tx_mc1_request_byte_count = *(memory + 7 + offset);
    offset += MODULE_TEXTURE_COUNTER_NUM;

    /* module PE */
    Counters->counters_part1.pe0_pixel_count_killed_by_color_pipe = *(memory + 0 + offset);
    Counters->counters_part1.pe0_pixel_count_killed_by_depth_pipe = *(memory + 1 + offset);
    Counters->counters_part1.pe0_pixel_count_drawn_by_color_pipe = *(memory + 2 + offset);
    Counters->counters_part1.pe0_pixel_count_drawn_by_depth_pipe = *(memory + 3 + offset);
    Counters->counters_part1.pe1_pixel_count_killed_by_color_pipe = *(memory + 4 + offset);
    Counters->counters_part1.pe1_pixel_count_killed_by_depth_pipe = *(memory + 5 + offset);
    Counters->counters_part1.pe1_pixel_count_drawn_by_color_pipe = *(memory + 6 + offset);
    Counters->counters_part1.pe1_pixel_count_drawn_by_depth_pipe = *(memory + 7 + offset);
    offset += MODULE_PIXEL_ENGINE_COUNTER_NUM;

    /* module MCC */
    max_latency = ((*(memory + 0 + offset) & 0x0fff0000) >> 16);
    min_latency = (*(memory + 0 + offset) & 0x00000fff);
    if (min_latency == 4095)
        min_latency = 0;
    Counters->counters_part2.mc_axi_max_latency = max_latency;
    Counters->counters_part2.mc_axi_max_latency = min_latency;
    Counters->counters_part2.mc_axi_total_latency = *(memory + 1 + offset);
    Counters->counters_part2.mc_axi_sample_count = *(memory + 2 + offset);
    Counters->counters_part2.mc_total_read_req_8B_from_colorpipe = *(memory + 3 + offset);
    Counters->counters_part2.mc_total_read_req_8B_sentout_from_colorpipe = *(memory + 4 + offset);
    Counters->counters_part2.mc_total_write_req_8B_from_colorpipe = *(memory + 5 + offset);
    Counters->counters_part2.mc_total_read_req_sentout_from_colorpipe = *(memory + 6 + offset);
    Counters->counters_part2.mc_total_write_req_from_colorpipe = *(memory + 7 + offset);
    Counters->counters_part2.mc_total_read_req_8B_from_others = *(memory + 8 + offset);
    Counters->counters_part2.mc_total_write_req_8B_from_others = *(memory + 9 + offset);
    Counters->counters_part2.mc_total_read_req_from_others = *(memory + 10 + offset);
    Counters->counters_part2.mc_total_write_req_from_others = *(memory + 11 + offset);
    offset += MODULE_MEMORY_CONTROLLER_COLOR_COUNTER_NUM;

    /* module MCZ */
    Counters->counters_part2.mc_total_read_req_8B_from_depthpipe = *(memory + 3 + offset);
    Counters->counters_part2.mc_total_read_req_8B_sentout_from_depthpipe = *(memory + 4 + offset);
    Counters->counters_part2.mc_total_write_req_8B_from_depthpipe = *(memory + 5 + offset);
    Counters->counters_part2.mc_total_read_req_sentout_from_depthpipe = *(memory + 6 + offset);
    Counters->counters_part2.mc_total_write_req_from_depthpipe = *(memory + 7 + offset);
    offset += MODULE_MEMORY_CONTROLLER_DEPTH_COUNTER_NUM;

    /* module HI0 */
    Counters->counters_part2.hi0_total_read_8B_count = *(memory + 0 + offset);
    Counters->counters_part2.hi0_total_write_8B_count = *(memory + 1 + offset);
    Counters->counters_part2.hi0_total_write_request_count = *(memory + 2 + offset);
    Counters->counters_part2.hi0_total_read_request_count = *(memory + 3 + offset);
    Counters->counters_part2.hi0_axi_cycles_read_request_stalled = *(memory + 4 + offset);
    Counters->counters_part2.hi0_axi_cycles_write_request_stalled = *(memory + 5 + offset);
    Counters->counters_part2.hi0_axi_cycles_write_data_stalled = *(memory + 6 + offset);
    Counters->counters_part2.hi_total_cycle_count = *(memory + 7 + offset);
    Counters->counters_part2.hi_total_idle_cycle_count = *(memory + 8 + offset);
    offset += MODULE_HOST_INTERFACE0_COUNTER_NUM;

    /* module HI1 */
    Counters->counters_part2.hi1_total_read_8B_count = *(memory + 0 + offset);
    Counters->counters_part2.hi1_total_write_8B_count = *(memory + 1 + offset);
    Counters->counters_part2.hi1_total_write_request_count = *(memory + 2 + offset);
    Counters->counters_part2.hi1_total_read_request_count = *(memory + 3 + offset);
    Counters->counters_part2.hi1_axi_cycles_read_request_stalled = *(memory + 4 + offset);
    Counters->counters_part2.hi1_axi_cycles_write_request_stalled = *(memory + 5 + offset);
    Counters->counters_part2.hi1_axi_cycles_write_data_stalled = *(memory + 6 + offset);
    offset += MODULE_HOST_INTERFACE1_COUNTER_NUM;

    /* module L2 */
    Counters->counters_part2.l2_total_axi0_read_request_count = *(memory + 0 + offset);
    Counters->counters_part2.l2_total_axi1_read_request_count = *(memory + 1 + offset);
    Counters->counters_part2.l2_total_axi0_write_request_count = *(memory + 2 + offset);
    Counters->counters_part2.l2_total_axi1_write_request_count = *(memory + 3 + offset);
    Counters->counters_part2.l2_total_read_transactions_request_by_axi0 = *(memory + 4 + offset);
    Counters->counters_part2.l2_total_read_transactions_request_by_axi1 = *(memory + 5 + offset);
    Counters->counters_part2.l2_total_write_transactions_request_by_axi0 = *(memory + 6 + offset);
    Counters->counters_part2.l2_total_write_transactions_request_by_axi1 = *(memory + 7 + offset);
    max_latency = ((*(memory + 8 + offset) & 0xffff0000) >> 16);
    min_latency = (*(memory + 8 + offset) & 0x0000ffff);
    if (min_latency == 4095)
        min_latency = 0;
    Counters->counters_part2.l2_axi0_min_latency = max_latency;
    Counters->counters_part2.l2_axi0_max_latency = min_latency;
    Counters->counters_part2.l2_axi0_total_latency = *(memory + 9 + offset);
    Counters->counters_part2.l2_axi0_total_request_count = *(memory + 10 + offset);
    max_latency = ((*(memory + 11 + offset) & 0xffff0000) >> 16);
    min_latency = (*(memory + 11 + offset) & 0x0000ffff);
    if (min_latency == 4095)
        min_latency = 0;
    Counters->counters_part2.l2_axi1_min_latency = max_latency;
    Counters->counters_part2.l2_axi1_max_latency = min_latency;
    Counters->counters_part2.l2_axi1_total_latency = *(memory + 12 + offset);
    Counters->counters_part2.l2_axi1_total_request_count = *(memory + 13 + offset);

    gcmFOOTER_NO();
    return status;
}

gcmINLINE static gctUINT32
CalcDelta(
IN gctUINT32 new,
IN gctUINT32 old
)
{
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
    IN gctBOOL IsFrameEnd
)
{
    gceSTATUS status = gcvSTATUS_OK;
    static gcsPROFILER_NEW_COUNTERS precounters;
    gctBOOL bSupportProbe = (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_PROBE) == gcvSTATUS_TRUE);

    gcmHEADER_ARG("Profiler=0x%x Counters=0x%x", Profiler, Counters);

    if (!Profiler)
    {
        gcmFOOTER();
        return gcvSTATUS_NOT_SUPPORTED;
    }

    gcmASSERT(Profiler->enable);

#define gcmGETCOUNTER(name) Profiler->perDrawMode && !IsFrameEnd? CalcDelta((Counters.name), (precounters.name)) : (Counters.name)

    if (Profiler->curBufId == 0 && Profiler->perDrawMode)
    {
        gcoOS_ZeroMemory(&precounters, gcmSIZEOF(precounters));
    }

    if (Profiler->needDump)
    {
        if (IsFrameEnd)
        {
            gcmWRITE_CONST_NEW(VPG_HW);
        }
        else
        {
            gcmWRITE_CONST_NEW(VPG_ES30_DRAW);
            gcmWRITE_COUNTER_NEW(VPC_ES30_DRAW_NO, Counters.drawID);
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
        gcmWRITE_COUNTER_NEW(VPNC_PSINSTCOUNT, gcmGETCOUNTER(counters_part1.ps_inst_counter));
        gcmWRITE_COUNTER_NEW(VPNC_PSBRANCHINSTCOUNT, gcmGETCOUNTER(counters_part1.ps_branch_inst_counter));
        gcmWRITE_COUNTER_NEW(VPNC_PSTEXLDINSTCOUNT, gcmGETCOUNTER(counters_part1.ps_texld_inst_counter));
        gcmWRITE_COUNTER_NEW(VPNC_PSRENDEREDPIXCOUNT, gcmGETCOUNTER(counters_part1.ps_rendered_pixel_counter));
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

        gcmWRITE_CONST_NEW(VPNG_MC);
        gcmWRITE_COUNTER_NEW(VPNC_MCREADREQ8BCOLORPIPE, gcmGETCOUNTER(counters_part2.mc_total_read_req_8B_from_colorpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCREADREQ8BSOCOLORPIPE, gcmGETCOUNTER(counters_part2.mc_total_read_req_8B_sentout_from_colorpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCWRITEREQ8BCOLORPIPE, gcmGETCOUNTER(counters_part2.mc_total_write_req_8B_from_colorpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCREADREQSOCOLORPIPE, gcmGETCOUNTER(counters_part2.mc_total_read_req_8B_from_colorpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCWRITEREQCOLORPIPE, gcmGETCOUNTER(counters_part2.mc_total_read_req_8B_sentout_from_colorpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCREADREQ8BDEPTHPIPE, gcmGETCOUNTER(counters_part2.mc_total_read_req_8B_from_colorpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCREADREQ8BSFDEPTHPIPE, gcmGETCOUNTER(counters_part2.mc_total_read_req_8B_sentout_from_colorpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCWRITEREQ8BDEPTHPIPE, gcmGETCOUNTER(counters_part2.mc_total_write_req_8B_from_colorpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCREADREQSFDEPTHPIPE, gcmGETCOUNTER(counters_part2.mc_total_read_req_8B_from_colorpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCWRITEREQDEPTHPIPE, gcmGETCOUNTER(counters_part2.mc_total_read_req_8B_sentout_from_colorpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCREADREQ8BOTHERPIPE, gcmGETCOUNTER(counters_part2.mc_total_read_req_8B_from_colorpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCREADREQ8BSFOTHERPIPE, gcmGETCOUNTER(counters_part2.mc_total_read_req_8B_sentout_from_colorpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCWRITEREQ8BOTHERPIPE, gcmGETCOUNTER(counters_part2.mc_total_write_req_8B_from_colorpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCREADREQSFOTHERPIPE, gcmGETCOUNTER(counters_part2.mc_total_read_req_8B_from_colorpipe));
        gcmWRITE_COUNTER_NEW(VPNC_MCAXIMINLATENCY, gcmGETCOUNTER(counters_part2.mc_axi_min_latency));
        gcmWRITE_COUNTER_NEW(VPNC_MCAXIMAXLATENCY, gcmGETCOUNTER(counters_part2.mc_axi_max_latency));
        gcmWRITE_COUNTER_NEW(VPNC_MCAXIMAXLATENCY, gcmGETCOUNTER(counters_part2.mc_axi_total_latency));
        gcmWRITE_COUNTER_NEW(VPNC_MCAXISAMPLECOUNT, gcmGETCOUNTER(counters_part2.mc_axi_sample_count));
        gcmWRITE_CONST_NEW(VPG_END);

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
        if (bSupportProbe)
        {
            gcmWRITE_COUNTER_NEW(VPNC_HIREAD8BYTE, gcmGETCOUNTER(counters_part2.hi0_total_read_8B_count) + gcmGETCOUNTER(counters_part2.hi1_total_read_8B_count));
            gcmWRITE_COUNTER_NEW(VPNC_HIWRITE8BYTE, gcmGETCOUNTER(counters_part2.hi0_total_write_8B_count) + gcmGETCOUNTER(counters_part2.hi1_total_write_8B_count));
        }
        else
        {
            gcmWRITE_COUNTER_NEW(VPNC_HIREAD8BYTE, gcmGETCOUNTER(counters_part2.hi_total_read_8B_count));
            gcmWRITE_COUNTER_NEW(VPNC_HIWRITE8BYTE, gcmGETCOUNTER(counters_part2.hi_total_write_8B_count));
        }
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
        gcmWRITE_COUNTER_NEW(VPNC_L2AXI0MINLATENCY, gcmGETCOUNTER(counters_part2.l2_axi0_min_latency));
        gcmWRITE_COUNTER_NEW(VPNC_L2AXI0MAXLATENCY, gcmGETCOUNTER(counters_part2.l2_axi0_max_latency));
        gcmWRITE_COUNTER_NEW(VPNC_L2AXI0TOTLATENCY, gcmGETCOUNTER(counters_part2.l2_axi0_total_latency));
        gcmWRITE_COUNTER_NEW(VPNC_L2AXI0TOTREQCOUNT, gcmGETCOUNTER(counters_part2.l2_axi0_total_request_count));
        gcmWRITE_COUNTER_NEW(VPNC_L2AXI1MINLATENCY, gcmGETCOUNTER(counters_part2.l2_axi1_min_latency));
        gcmWRITE_COUNTER_NEW(VPNC_L2AXI1MAXLATENCY, gcmGETCOUNTER(counters_part2.l2_axi1_max_latency));
        gcmWRITE_COUNTER_NEW(VPNC_L2AXI1TOTLATENCY, gcmGETCOUNTER(counters_part2.l2_axi1_total_latency));
        gcmWRITE_COUNTER_NEW(VPNC_L2AXI1TOTREQCOUNT, gcmGETCOUNTER(counters_part2.l2_axi1_total_request_count));
        gcmWRITE_CONST_NEW(VPG_END);

        gcmWRITE_CONST_NEW(VPG_END);
    }

    if (!IsFrameEnd && Profiler->perDrawMode)
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

#if VIVANTE_PROFILER_PM
    if (Enable)
    {
        gcoHAL_ConfigPowerManagement(gcvFALSE);
    }
    else
    {
#if VIVANTE_PROFILER_PROBE
        if (_HAL)
        {
            int bufId = 0;
            gcoBUFOBJ counterBuf;
            gctHANDLE pid = gcoOS_GetCurrentProcessID();
            static gctUINT8 num = 1;
            gctUINT offset = 0;

            char* pos = gcvNULL;
#ifdef ANDROID
            gcoOS_GetEnv(gcvNULL, "VP_PROCESS_NAME", &env);
            if ((env != gcvNULL) && (env[0] !=0)) matchResult = (gcoOS_DetectProcessByName(env) ? gcvTRUE : gcvFALSE);
            if(matchResult != gcvTRUE) {
                return gcvSTATUS_MISMATCH;
            }
#endif
            gcoHAL_ConfigPowerManagement(gcvFALSE);

            /* enable profiler in kernel. */
            iface.ignoreTLS = gcvFALSE;
            iface.command = gcvHAL_SET_PROFILE_SETTING;
            iface.u.SetProfileSetting.enable = gcvFALSE;

            /* Call the kernel. */
            status = gcoOS_DeviceControl(gcvNULL,
                IOCTL_GCHAL_INTERFACE,
                &iface, gcmSIZEOF(iface),
                &iface, gcmSIZEOF(iface));

            gcoHARDWARE_EnableCounters(gcvNULL, &_HAL->profiler.probeBuffer);
            for (; bufId < NumOfDrawBuf;bufId++)
            {
                gcoBUFOBJ_Construct(_HAL, gcvBUFOBJ_TYPE_GENERIC_BUFFER, &counterBuf);
                gcoBUFOBJ_Upload(counterBuf, gcvNULL, 0, 1024*sizeof(gctUINT32), gcvBUFOBJ_USAGE_STATIC_DRAW);
                _HAL->profiler.probeBuffer.newCounterBuf[bufId] = (gctHANDLE)counterBuf;
                _HAL->profiler.probeBuffer.opType[bufId] = gcvCOUNTER_OP_NONE;
            }
            _HAL->profiler.probeBuffer.curBufId = 0;

            /*generate file name for each context*/
            fileName = DEFAULT_PROBE_FILE_NAME;
            if(fileName) gcoOS_StrCatSafe(inputFileName,256,fileName);

            gcoOS_StrStr (inputFileName, ".xml",&pos);
            if(pos) pos[0] = '\0';
            gcoOS_PrintStrSafe(profilerName, gcmSIZEOF(profilerName), &offset, "%s_%d_%d.xml",inputFileName,(gctUINTPTR_T)pid, num);

            num++;
            gcoOS_Open(gcvNULL, profilerName, gcvFILE_CREATETEXT, &_HAL->profiler.probeFile);
#if !VIVANTE_PROFILER_PROBE_PERDRAW
            gcoPROFILER_Begin(Hal, gcvCOUNTER_OP_NONE);
#endif
        }
#else
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

#endif
        gcmFOOTER();
        return status;
    }
#endif

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
#if VIVANTE_PROFILER_CONTEXT
        /*generate file name for each context*/
        gctHANDLE pid = gcoOS_GetCurrentProcessID();
        static gctUINT8 num = 1;
        gctUINT offset = 0;
        char* pos = gcvNULL;

        gcoOS_StrStr (inputFileName, ".vpd",&pos);
        if(pos) pos[0] = '\0';
        gcoOS_PrintStrSafe(profilerName, gcmSIZEOF(profilerName), &offset, "%s_%d_%d.vpd",inputFileName,(gctUINTPTR_T)pid, num);

        num++;
#endif
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

#if VIVANTE_PROFILER_PROBE
    {
    int bufId = 0;
    gcoBUFOBJ counterBuf;
    for (; bufId < NumOfDrawBuf; bufId++)
    {
        gcoBUFOBJ_Construct(_HAL, gcvBUFOBJ_TYPE_GENERIC_BUFFER, &counterBuf);
        gcoBUFOBJ_Upload(counterBuf, gcvNULL, 0, 1024 * sizeof(gctUINT32), gcvBUFOBJ_USAGE_STATIC_DRAW);
        _HAL->profiler.probeBuffer.newCounterBuf[bufId] = (gctHANDLE)counterBuf;
    }
    _HAL->profiler.probeBuffer.curBufId = 0;
    }
#endif
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
    else
    {
#if VIVANTE_PROFILER_PROBE
        gctUINT32 i = 0;
        for (i = 0;  i < NumOfDrawBuf; i++)
            gcoBUFOBJ_Destroy(_HAL->profiler.probeBuffer.newCounterBuf[i]);

        gcmWRITE_XML_STRING("</DrawCounter>\n");
        gcoPROFILER_Flush(gcvNULL);
        gcoOS_Close(gcvNULL, _HAL->profiler.probeFile);
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
#endif
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
    else
    {
#if VIVANTE_PROFILER_PROBE
        {
            status = gcoOS_Write(gcvNULL,
                _HAL->profiler.probeFile,
                ByteCount, Data);
        }
#endif
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
    else
    {
#if VIVANTE_PROFILER_PROBE
        {
            status = gcoOS_Flush(gcvNULL,
                _HAL->profiler.probeFile);
        }
#endif
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
    #if VIVANTE_PROFILER_CONTEXT
    if(!_HAL) _HAL = glhal_map_current();
    #endif
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
#if VIVANTE_PROFILER_PROBE
    static gctUINT32 frameNo = 0;
    gctUINT32   address;
    gctPOINTER  memory;
    gctUINT32 i, j, offset;
#endif

    gcmHEADER();
    if(!_HAL)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_NOT_SUPPORTED;
    }

    if (!_HAL->profiler.enable)
    {
#if VIVANTE_PROFILER_PROBE

#if !VIVANTE_PROFILER_PROBE_PERDRAW
        gcoPROFILER_End(Hal, gcvFALSE);
#endif
        if (_HAL->profiler.probeBuffer.curBufId > 0)
        {
            gcoHAL_Commit(_HAL, gcvTRUE);
            gcmWRITE_XML_STRING("\t<FrameID>\n");
            gcmWRITE_XML_STRING("\t\t"); gcmWRITE_XML_Value("FrameNUM", frameNo, 1);

            for (j = 0; j < _HAL->profiler.probeBuffer.curBufId; j++)
            {
                gctUINT32 min_latency = 0;
                gctUINT32 max_latency = 0;
                gceCOUNTER_OPTYPE operationType = _HAL->profiler.probeBuffer.opType[j];

                gcmWRITE_XML_STRING("\t\t<Operation>\n");
                gcmWRITE_XML_STRING("\t\t\t"); gcmWRITE_XML_Value("OperationType", operationType, 1);
                gcmWRITE_XML_STRING("\t\t\t"); gcmWRITE_XML_Value("OperationNumber", j, 1);

                gcoBUFOBJ_Lock(_HAL->profiler.probeBuffer.newCounterBuf[j], &address, &memory);
                offset = 0;
                /* module FE */

                gcmWRITE_XML_STRING("\t\t\t<FECounters>\n");
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("VtxOuputCount", *((gctUINT32_PTR)memory + 0 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("VtxCacheMissCount", *((gctUINT32_PTR)memory + 1 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("VtxCacheLookupCount", *((gctUINT32_PTR)memory + 2 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("StallCount", *((gctUINT32_PTR)memory + 3 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("ProcessCount", *((gctUINT32_PTR)memory + 4 + offset));
                gcmWRITE_XML_STRING("\t\t\t</FECounters>\n");
                offset += MODULE_FRONT_END_COUNTER_NUM;

                /* module VS */
                gcmWRITE_XML_STRING("\t\t\t<VSCounters>\n");
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("CycleCounter", *((gctUINT32_PTR)memory + 0 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("PixelInstrPerCore", *((gctUINT32_PTR)memory + 1 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalPixelShaded", *((gctUINT32_PTR)memory + 2 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("VertexInstrPerCore", *((gctUINT32_PTR)memory + 3 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("VertexExecute", *((gctUINT32_PTR)memory + 4 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("VertexBranchInstrExecute", *((gctUINT32_PTR)memory + 5 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("VertexTextureInstrExecute", *((gctUINT32_PTR)memory + 6 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("PixelBranchInstrExecute", *((gctUINT32_PTR)memory + 7 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("PixelTextureInstrExecute", *((gctUINT32_PTR)memory + 8 + offset));
                gcmWRITE_XML_STRING("\t\t\t</VSCounters>\n");
                offset += MODULE_VERTEX_SHADER_COUNTER_NUM;

                /* module PA */
                gcmWRITE_XML_STRING("\t\t\t<PACounters>\n");
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("VTXCount", *((gctUINT32_PTR)memory + 0 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("InPrimCount", *((gctUINT32_PTR)memory + 1 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("OutPrimCount", *((gctUINT32_PTR)memory + 2 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("tRejCount", *((gctUINT32_PTR)memory + 3 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("CullPrimCount", *((gctUINT32_PTR)memory + 4 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("DropPrimCount", *((gctUINT32_PTR)memory + 5 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("FrstClipPrimCount", *((gctUINT32_PTR)memory + 6 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("FrstClipDropPrimCount", *((gctUINT32_PTR)memory + 7 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("NonIdleStarveCount", *((gctUINT32_PTR)memory + 8 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("StarveCount", *((gctUINT32_PTR)memory + 9 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("StallCount", *((gctUINT32_PTR)memory + 10 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("ProcessCount", *((gctUINT32_PTR)memory + 11 + offset));
                gcmWRITE_XML_STRING("\t\t\t</PACounters>\n");
                offset += MODULE_PRIMITIVE_ASSEMBLY_COUNTER_NUM;

                /* module SE */
                gcmWRITE_XML_STRING("\t\t\t<SECounters>\n");
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("StarveCount", *((gctUINT32_PTR)memory + 0 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("StallCount", *((gctUINT32_PTR)memory + 1 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("InputTriCount", *((gctUINT32_PTR)memory + 2 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("OutputTriCount", *((gctUINT32_PTR)memory + 3 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("InputLineCount", *((gctUINT32_PTR)memory + 4 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("OutputLineCount", *((gctUINT32_PTR)memory + 5 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("ProcessCount", *((gctUINT32_PTR)memory + 6 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("ClippedTriCount", *((gctUINT32_PTR)memory + 7 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("ClippedLineCount", *((gctUINT32_PTR)memory + 8 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("CulledTriCount", *((gctUINT32_PTR)memory + 9 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("CulledLineCount", *((gctUINT32_PTR)memory + 10 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TrivialRejLineCount", *((gctUINT32_PTR)memory + 11 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("NonIdleStartveCount", *((gctUINT32_PTR)memory + 12 + offset));
                gcmWRITE_XML_STRING("\t\t\t</SECounters>\n");
                offset += MODULE_SETUP_COUNTER_NUM;

                /* module RA */
                gcmWRITE_XML_STRING("\t\t\t<RACounters>\n");
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("PrimInputCount", *((gctUINT32_PTR)memory + 0 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("QuadCountFromScan", *((gctUINT32_PTR)memory + 1 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("ZCacheMiss", *((gctUINT32_PTR)memory + 2 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("HZCacheMiss", *((gctUINT32_PTR)memory + 3 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("QuadCountAfterHZorEEZ", *((gctUINT32_PTR)memory + 4 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("ValidPixelCountToRender", *((gctUINT32_PTR)memory + 5 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("OutputValidQuadCount", *((gctUINT32_PTR)memory + 6 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("OutputValidPixelCount", *((gctUINT32_PTR)memory + 7 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("HZPipeCacheMiss", *((gctUINT32_PTR)memory + 8 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("RARenderPipe1CacheMiss", *((gctUINT32_PTR)memory + 9 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("NonIdleStarveCount", *((gctUINT32_PTR)memory + 10 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("StarveCount", *((gctUINT32_PTR)memory + 11 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("StallCount", *((gctUINT32_PTR)memory + 12 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("ProcessCount", *((gctUINT32_PTR)memory + 13 + offset));
                gcmWRITE_XML_STRING("\t\t\t</RACounters>\n");
                offset += MODULE_RASTERIZER_COUNTER_NUM;

                /* module PS */
                gcmWRITE_XML_STRING("\t\t\t<PSCounters>\n");
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("CycleCounter", *((gctUINT32_PTR)memory + 0 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("PixelInstrPerCore", *((gctUINT32_PTR)memory + 1 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalPixelShaded", *((gctUINT32_PTR)memory + 2 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("VertexInstrPerCore", *((gctUINT32_PTR)memory + 3 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("VertexExecute", *((gctUINT32_PTR)memory + 4 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("VertexBranchInstrExecute", *((gctUINT32_PTR)memory + 5 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("VertexTextureInstrExecute", *((gctUINT32_PTR)memory + 6 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("PixelBranchInstrExecute", *((gctUINT32_PTR)memory + 7 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("PixelTextureInstrExecute", *((gctUINT32_PTR)memory + 8 + offset));
                gcmWRITE_XML_STRING("\t\t\t</PSCounters>\n");
                offset += MODULE_PIXEL_SHADER_COUNTER_NUM;

                /* module TX */
                gcmWRITE_XML_STRING("\t\t\t<TXCounters>\n");
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("BiLinearIncomingPixel", *((gctUINT32_PTR)memory + 0 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TriLinearIncomingPixel", *((gctUINT32_PTR)memory + 1 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("DiscardIncomingPixel", *((gctUINT32_PTR)memory + 2 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalIncomingPixel", *((gctUINT32_PTR)memory + 3 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("MC0MissCount", *((gctUINT32_PTR)memory + 4 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("MC0MemoryRequestByte", *((gctUINT32_PTR)memory + 5 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("MC1MissCount", *((gctUINT32_PTR)memory + 6 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("MC1MemoryRequestByte", *((gctUINT32_PTR)memory + 7 + offset));
                gcmWRITE_XML_STRING("\t\t\t</TXCounters>\n");
                offset += MODULE_TEXTURE_COUNTER_NUM;

                /* module PE */
                gcmWRITE_XML_STRING("\t\t\t<PECounters>\n");
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("PE0ColorKilledCount", *((gctUINT32_PTR)memory + 0 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("PE0DepthKilledCount", *((gctUINT32_PTR)memory + 1 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("PE0ColorDrawnCount", *((gctUINT32_PTR)memory + 2 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("PE0DepthDrawnCount", *((gctUINT32_PTR)memory + 3 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("PE1ColorKilledCount", *((gctUINT32_PTR)memory + 4 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("PE1DepthKilledCount", *((gctUINT32_PTR)memory + 5 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("PE1ColorDrawnCount", *((gctUINT32_PTR)memory + 6 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("PE1DepthDrawnCount", *((gctUINT32_PTR)memory + 7 + offset));
                gcmWRITE_XML_STRING("\t\t\t</PECounters>\n");
                offset += MODULE_PIXEL_ENGINE_COUNTER_NUM;

                /* module MCC */
                gcmWRITE_XML_STRING("\t\t\t<MCCCounters>\n");
                if (*((gctUINT32_PTR)memory + offset) != 0xDEADDEAD)
                {
                    max_latency = ((*((gctUINT32_PTR)memory + offset) & 0xfff000) >> 12);
                    min_latency = (*((gctUINT32_PTR)memory + offset) & 0x000fff);
                    if (min_latency == 4095)
                        min_latency = 0;
                    gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("MaxLatency", max_latency);
                    gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("MinLatency", min_latency);
                }
                else
                {
                    gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("MinMaxLatency", *((gctUINT32_PTR)memory + 0 + offset));
                }
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("Total_Latency", *((gctUINT32_PTR)memory + 1 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalSampleCount", *((gctUINT32_PTR)memory + 2 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalReadin8BFromPE", *((gctUINT32_PTR)memory + 3 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalReadin8BFromComp", *((gctUINT32_PTR)memory + 4 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalWritein8BFromPE", *((gctUINT32_PTR)memory + 5 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalReadReqFromPE", *((gctUINT32_PTR)memory + 6 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalWriteReqFromPE", *((gctUINT32_PTR)memory + 7 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalReadRin8BFromOthers", *((gctUINT32_PTR)memory + 8 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalWritein8BFromOthers", *((gctUINT32_PTR)memory + 9 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalReadReqFromOthers", *((gctUINT32_PTR)memory + 10 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalWriteReqFromOthers", *((gctUINT32_PTR)memory + 11 + offset));
                gcmWRITE_XML_STRING("\t\t\t</MCCCounters>\n");
                offset += MODULE_MEMORY_CONTROLLER_COLOR_COUNTER_NUM;

                /* module MCZ */
                gcmWRITE_XML_STRING("\t\t\t<MCZCounters>\n");
                if (*((gctUINT32_PTR)memory + offset) != 0xDEADDEAD)
                {
                    max_latency = ((*((gctUINT32_PTR)memory + offset) & 0xfff000) >> 12);
                    min_latency = (*((gctUINT32_PTR)memory + offset) & 0x000fff);
                    if (min_latency == 4095)
                        min_latency = 0;
                    gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("MaxLatency", max_latency);
                    gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("MinLatency", min_latency);
                }
                else
                {
                    gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("MinMaxLatency", *((gctUINT32_PTR)memory + 0 + offset));
                }
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("Total_Latency", *((gctUINT32_PTR)memory + 1 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalSampleCount", *((gctUINT32_PTR)memory + 2 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalReadin8BFromPE", *((gctUINT32_PTR)memory + 3 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalReadin8BFromComp", *((gctUINT32_PTR)memory + 4 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalWritein8BFromPE", *((gctUINT32_PTR)memory + 5 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalReadReqFromPE", *((gctUINT32_PTR)memory + 6 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalWriteReqFromPE", *((gctUINT32_PTR)memory + 7 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalReadRin8BFromOthers", *((gctUINT32_PTR)memory + 8 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalWritein8BFromOthers", *((gctUINT32_PTR)memory + 9 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalReadReqFromOthers", *((gctUINT32_PTR)memory + 10 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalWriteReqFromOthers", *((gctUINT32_PTR)memory + 11 + offset));
                gcmWRITE_XML_STRING("\t\t\t</MCZCounters>\n");
                offset += MODULE_MEMORY_CONTROLLER_DEPTH_COUNTER_NUM;

                /* module HI0 */
                gcmWRITE_XML_STRING("\t\t\t<HI0Counters>\n");
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalReadsin64bits", *((gctUINT32_PTR)memory + 0 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalWritesin64bits", *((gctUINT32_PTR)memory + 1 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalWriteReqCount", *((gctUINT32_PTR)memory + 2 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalReadReqCount", *((gctUINT32_PTR)memory + 3 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("NumberOfCyclesAXIReadReqisStalled", *((gctUINT32_PTR)memory + 4 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("NumberOfCyclesAXIWriteReqisStalled", *((gctUINT32_PTR)memory + 5 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("NumberOfCyclesAXIWriteDataisStalled", *((gctUINT32_PTR)memory + 6 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalCycle", *((gctUINT32_PTR)memory + 7 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalIdleCycle", *((gctUINT32_PTR)memory + 8 + offset));
                gcmWRITE_XML_STRING("\t\t\t</HI0Counters>\n");
                offset += MODULE_HOST_INTERFACE0_COUNTER_NUM;

                /* module HI1 */
                gcmWRITE_XML_STRING("\t\t\t<HI1Counters>\n");
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalReadsin64bits", *((gctUINT32_PTR)memory + 0 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalWritesin64bits", *((gctUINT32_PTR)memory + 1 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalWriteReqCount", *((gctUINT32_PTR)memory + 2 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalReadReqCount", *((gctUINT32_PTR)memory + 3 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("NumberOfCyclesAXIReadReqisStalled", *((gctUINT32_PTR)memory + 4 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("NumberOfCyclesAXIWriteReqisStalled", *((gctUINT32_PTR)memory + 5 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("NumberOfCyclesAXIWriteDataisStalled", *((gctUINT32_PTR)memory + 6 + offset));
                gcmWRITE_XML_STRING("\t\t\t</HI1Counters>\n");
                offset += MODULE_HOST_INTERFACE1_COUNTER_NUM;

                /* module L2 */
                gcmWRITE_XML_STRING("\t\t\t<GPUL2Counters>\n");
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalAXI0ReadReq", *((gctUINT32_PTR)memory + 0 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalAXI1ReadReq", *((gctUINT32_PTR)memory + 1 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalAXI0WriteReq", *((gctUINT32_PTR)memory + 2 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalAXI1WriteReq", *((gctUINT32_PTR)memory + 3 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalAXI0ReadTransReq", *((gctUINT32_PTR)memory + 4 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalAXI1ReadTransReq", *((gctUINT32_PTR)memory + 5 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalAXI0WriteTransReq", *((gctUINT32_PTR)memory + 6 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("TotalAXI1WriteTransReq", *((gctUINT32_PTR)memory + 7 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("AXI0MinMaxLatency", *((gctUINT32_PTR)memory + 8 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("AXI0TotalLatency", *((gctUINT32_PTR)memory + 9 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("AXI0TotalReq", *((gctUINT32_PTR)memory + 10 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("AXI1MinMaxLatency", *((gctUINT32_PTR)memory + 11 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("AXI1TotalLatency", *((gctUINT32_PTR)memory + 12 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t"); gcmWRITE_XML_COUNTER("AXI1TotalReq", *((gctUINT32_PTR)memory + 13 + offset));
                gcmWRITE_XML_STRING("\t\t\t</GPUL2Counters>\n");

                gcmWRITE_XML_STRING("\t\t</Operation>\n");
            }
            gcmWRITE_XML_STRING("\t</FrameID>\n");
            frameNo++;
            _HAL->profiler.probeBuffer.curBufId = 0;
        }
#endif
#if !VIVANTE_PROFILER_PROBE_PERDRAW
        gcoPROFILER_Begin(Hal, gcvCOUNTER_OP_NONE);
#endif
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

#if VIVANTE_PROFILER_PROBE
        gctUINT32 mc_axi_min_latency, mc_axi_max_latency, total_reads, total_writes;

        gcoHAL_Commit(_HAL, gcvTRUE); /* TODO: Calling this function on multi-GPU board will cause GPU hang. */

        /* write per draw counter */
        for (j = 0; j < _HAL->profiler.probeBuffer.curBufId; j++)
        {
            gcmWRITE_CONST(VPG_ES30_DRAW);
            gcmWRITE_COUNTER(VPC_ES30_DRAW_NO, j);
            offset = 0;

            gcoBUFOBJ_Lock(_HAL->profiler.probeBuffer.newCounterBuf[j], &address, &memory);

            /* module FE */
            gcmWRITE_CONST(VPG_FE);
            gcmWRITE_COUNTER(VPC_FEOUTVERTEXCOUNT, *((gctUINT32_PTR)memory + offset));
            gcmWRITE_COUNTER(VPC_FESTALLCOUNT, *((gctUINT32_PTR)memory + 3 + offset));
            gcmWRITE_CONST(VPG_END);

            offset += MODULE_FRONT_END_COUNTER_NUM;

            /* module VS */
            gcmWRITE_CONST(VPG_VS);
            gcmWRITE_COUNTER(VPC_VSINSTCOUNT, *((gctUINT32_PTR)memory + 3 + offset));
            gcmWRITE_COUNTER(VPC_VSRENDEREDVERTCOUNT, *((gctUINT32_PTR)memory + 4 + offset));
            gcmWRITE_COUNTER(VPC_VSBRANCHINSTCOUNT, *((gctUINT32_PTR)memory + 5 + offset));
            gcmWRITE_COUNTER(VPC_VSTEXLDINSTCOUNT, *((gctUINT32_PTR)memory + 6 + offset));
            gcmWRITE_CONST(VPG_END);
            offset += MODULE_VERTEX_SHADER_COUNTER_NUM;

            /* module PA */
            gcmWRITE_CONST(VPG_PA);
            gcmWRITE_COUNTER(VPC_PAINPRIMCOUNT, *((gctUINT32_PTR)memory + offset));
            gcmWRITE_COUNTER(VPC_PAOUTPRIMCOUNT, *((gctUINT32_PTR)memory + 1 + offset));
            gcmWRITE_COUNTER(VPC_PADEPTHCLIPCOUNT, *((gctUINT32_PTR)memory + 2 + offset));
            gcmWRITE_COUNTER(VPC_PATRIVIALREJCOUNT, *((gctUINT32_PTR)memory + 3 + offset));
            gcmWRITE_COUNTER(VPC_PACULLCOUNT, *((gctUINT32_PTR)memory + 4 + offset));
            gcmWRITE_COUNTER(VPC_PANONIDLESTARVECOUNT, *((gctUINT32_PTR)memory + 8 + offset));
            gcmWRITE_COUNTER(VPC_PASTARVELCOUNT, *((gctUINT32_PTR)memory + 9 + offset));
            gcmWRITE_COUNTER(VPC_PASTALLCOUNT, *((gctUINT32_PTR)memory + 10 + offset));
            gcmWRITE_COUNTER(VPC_PAPROCESSCOUNT, *((gctUINT32_PTR)memory + 11 + offset));
            gcmWRITE_CONST(VPG_END);
            offset += MODULE_PRIMITIVE_ASSEMBLY_COUNTER_NUM;

            /* module SE */
            gcmWRITE_CONST(VPG_SETUP);
            gcmWRITE_COUNTER(VPC_SETRIANGLECOUNT, *((gctUINT32_PTR)memory + 9 + offset));
            gcmWRITE_COUNTER(VPC_SELINECOUNT, *((gctUINT32_PTR)memory + 10 + offset));
            gcmWRITE_COUNTER(VPC_SESTARVECOUNT, *((gctUINT32_PTR)memory + offset));
            gcmWRITE_COUNTER(VPC_SESTALLCOUNT, *((gctUINT32_PTR)memory + 1 + offset));
            gcmWRITE_COUNTER(VPC_SERECEIVETRIANGLECOUNT, *((gctUINT32_PTR)memory + 2 + offset));
            gcmWRITE_COUNTER(VPC_SESENDTRIANGLECOUNT, *((gctUINT32_PTR)memory + 3 + offset));
            gcmWRITE_COUNTER(VPC_SERECEIVELINESCOUNT, *((gctUINT32_PTR)memory + 4 + offset));
            gcmWRITE_COUNTER(VPC_SESENDLINESCOUNT, *((gctUINT32_PTR)memory + 5 + offset));
            gcmWRITE_COUNTER(VPC_SEPROCESSCOUNT, *((gctUINT32_PTR)memory + 6 + offset));
            gcmWRITE_COUNTER(VPC_SENONIDLESTARVECOUNT, *((gctUINT32_PTR)memory + 12 + offset));
            gcmWRITE_CONST(VPG_END);
            offset += MODULE_SETUP_COUNTER_NUM;

            /* module RA */
            gcmWRITE_CONST(VPG_RA);
            gcmWRITE_COUNTER(VPC_RAVALIDPIXCOUNT, *((gctUINT32_PTR)memory + 7 + offset));
            gcmWRITE_COUNTER(VPC_RATOTALQUADCOUNT, *((gctUINT32_PTR)memory + 1 + offset));
            gcmWRITE_COUNTER(VPC_RAVALIDQUADCOUNTEZ, *((gctUINT32_PTR)memory + 4 + offset));
            gcmWRITE_COUNTER(VPC_RATOTALPRIMCOUNT, *((gctUINT32_PTR)memory + offset));
            gcmWRITE_COUNTER(VPC_RANONIDLESTARVECOUNT, *((gctUINT32_PTR)memory + 10 + offset));
            gcmWRITE_COUNTER(VPC_RASTARVELCOUNT, *((gctUINT32_PTR)memory + 11 + offset));
            gcmWRITE_COUNTER(VPC_RASTALLCOUNT, *((gctUINT32_PTR)memory + 12 + offset));
            gcmWRITE_COUNTER(VPC_RAPROCESSCOUNT, *((gctUINT32_PTR)memory + 13 + offset));
            gcmWRITE_CONST(VPG_END);
            offset += MODULE_RASTERIZER_COUNTER_NUM;

            /* module PS */
            gcmWRITE_CONST(VPG_PS);
            gcmWRITE_COUNTER(VPC_PSINSTCOUNT,  *((gctUINT32_PTR)memory + 1 + offset));
            gcmWRITE_COUNTER(VPC_PSBRANCHINSTCOUNT,  *((gctUINT32_PTR)memory + 7 + offset));
            gcmWRITE_COUNTER(VPC_PSTEXLDINSTCOUNT,  *((gctUINT32_PTR)memory + 8 + offset));
            gcmWRITE_COUNTER(VPC_PSRENDEREDPIXCOUNT,  *((gctUINT32_PTR)memory + 2 + offset));
            gcmWRITE_COUNTER(VPC_PSSHADERCYCLECOUNT,  *((gctUINT32_PTR)memory + offset));
            gcmWRITE_CONST(VPG_END);
            offset += MODULE_PIXEL_SHADER_COUNTER_NUM;

            /* module TX */
            gcmWRITE_CONST(VPG_TX);
            gcmWRITE_COUNTER(VPC_TXTOTBILINEARREQ,  *((gctUINT32_PTR)memory + offset));
            gcmWRITE_COUNTER(VPC_TXTOTTRILINEARREQ,  *((gctUINT32_PTR)memory + 1 + offset));
            gcmWRITE_COUNTER(VPC_TXTOTTEXREQ,  *((gctUINT32_PTR)memory + 3 + offset));
            gcmWRITE_CONST(VPG_END);
            offset += MODULE_TEXTURE_COUNTER_NUM;

            /* module PE */
            gcmWRITE_CONST(VPG_PE);
            gcmWRITE_COUNTER(VPC_PEKILLEDBYCOLOR, *((gctUINT32_PTR)memory + offset));
            gcmWRITE_COUNTER(VPC_PEKILLEDBYDEPTH, *((gctUINT32_PTR)memory + 1 + offset));
            gcmWRITE_COUNTER(VPC_PEDRAWNBYCOLOR, *((gctUINT32_PTR)memory + 2 + offset));
            gcmWRITE_COUNTER(VPC_PEDRAWNBYDEPTH, *((gctUINT32_PTR)memory + 3 + offset));
            gcmWRITE_CONST(VPG_END);
            offset += MODULE_PIXEL_ENGINE_COUNTER_NUM;
            offset += MODULE_MEMORY_CONTROLLER_COLOR_COUNTER_NUM;

            /* module MCZ */
            gcmWRITE_CONST(VPG_MC);
            mc_axi_min_latency = (*((gctUINT32_PTR)memory + offset) & 0x0fff0000) >> 16;
            mc_axi_max_latency = (*((gctUINT32_PTR)memory + offset) & 0x00000fff);
            if (mc_axi_min_latency == 4095)
                mc_axi_min_latency = 0;
            gcmWRITE_COUNTER(VPC_MCAXIMINLATENCY, mc_axi_min_latency);
            gcmWRITE_COUNTER(VPC_MCAXIMAXLATENCY, mc_axi_max_latency);
            gcmWRITE_COUNTER(VPC_MCAXIMAXLATENCY, *((gctUINT32_PTR)memory + 1 + offset));
            gcmWRITE_COUNTER(VPC_MCAXISAMPLECOUNT, *((gctUINT32_PTR)memory + 2 + offset));
            gcmWRITE_CONST(VPG_END);
            offset += MODULE_MEMORY_CONTROLLER_DEPTH_COUNTER_NUM;

            /* module HI0 */
            total_reads = *((gctUINT32_PTR)memory + offset);
            total_writes = *((gctUINT32_PTR)memory + 1 + offset);
            gcmWRITE_COUNTER(VPC_GPUCYCLES, *((gctUINT32_PTR)memory + 7 + offset));
            gcmWRITE_COUNTER(VPC_GPUIDLECYCLES, *((gctUINT32_PTR)memory + 8 + offset));
            offset += MODULE_HOST_INTERFACE0_COUNTER_NUM;

            /* module HI1 */
            total_reads += *((gctUINT32_PTR)memory + offset);
            total_writes += *((gctUINT32_PTR)memory + 1 + offset);

            gcmWRITE_CONST(VPG_GPU);
            gcmWRITE_COUNTER(VPC_GPUREAD64BYTE, total_reads);
            gcmWRITE_COUNTER(VPC_GPUWRITE64BYTE, total_writes);
            gcmWRITE_CONST(VPG_END);

            gcmWRITE_CONST(VPG_END);
        }

#endif

#if VIVANTE_PROFILER_PERDRAW
        /* Set Register clear Flag. */
        iface.ignoreTLS = gcvFALSE;
        iface.command = gcvHAL_READ_PROFILER_REGISTER_SETTING;
        iface.u.SetProfilerRegisterClear.bclear = gcvTRUE;

        /* Call the kernel. */
        status = gcoOS_DeviceControl(gcvNULL,
                                    IOCTL_GCHAL_INTERFACE,
                                    &iface, gcmSIZEOF(iface),
                                    &iface, gcmSIZEOF(iface));
        /* Verify result. */
        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_NO();
            return gcvSTATUS_GENERIC_IO;
        }
#endif
        if (_HAL->profiler.isCLMode)
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

#if VIVANTE_PROFILER_PROBE

                gcmPRINT("Frame %d:\n", frameNo);
                gcmPRINT("************\n");

                for (j = 0; j < _HAL->profiler.probeBuffer.curBufId; j++)
                {
                    gcmPRINT("draw #%d\n", j);
                    gcmPRINT("************\n");
                    gcoBUFOBJ_Lock(_HAL->profiler.probeBuffer.newCounterBuf[j], &address, &memory);
                    offset = 0;
                    /* module FE */
                    for (i = 0; i < MODULE_FRONT_END_COUNTER_NUM; i++)
                    {
                        gcmPRINT("FE counter #%d: %d\n", i, *((gctUINT32_PTR)memory + i + offset));
                    }
                    offset += MODULE_FRONT_END_COUNTER_NUM;
                    /* module VS */
                    for (i = 0; i < MODULE_VERTEX_SHADER_COUNTER_NUM; i++)
                    {
                        gcmPRINT("VS counter #%d: %d\n", i, *((gctUINT32_PTR)memory + i + offset));
                    }
                    offset += MODULE_VERTEX_SHADER_COUNTER_NUM;
                    /* module PA */
                    for (i = 0; i < MODULE_PRIMITIVE_ASSEMBLY_COUNTER_NUM; i++)
                    {
                        gcmPRINT("PA counter #%d: %d\n", i, *((gctUINT32_PTR)memory + i + offset));
                    }
                    offset += MODULE_PRIMITIVE_ASSEMBLY_COUNTER_NUM;
                    /* module SE */
                    for (i = 0; i < MODULE_SETUP_COUNTER_NUM; i++)
                    {
                        gcmPRINT("SE counter #%d: %d\n", i, *((gctUINT32_PTR)memory + i + offset));
                    }
                    offset += MODULE_SETUP_COUNTER_NUM;
                    /* module RA */
                    for (i = 0; i < MODULE_RASTERIZER_COUNTER_NUM; i++)
                    {
                        gcmPRINT("RA counter #%d: %d\n", i, *((gctUINT32_PTR)memory + i + offset));
                    }
                    offset += MODULE_RASTERIZER_COUNTER_NUM;
                    /* module PS */
                    for (i = 0; i < MODULE_PIXEL_SHADER_COUNTER_NUM; i++)
                    {
                        gcmPRINT("PS counter #%d: %d\n", i, *((gctUINT32_PTR)memory + i + offset));
                    }
                    offset += MODULE_PIXEL_SHADER_COUNTER_NUM;
                    /* module TX */
                    for (i = 0; i < MODULE_TEXTURE_COUNTER_NUM; i++)
                    {
                        gcmPRINT("TX counter #%d: %d\n", i, *((gctUINT32_PTR)memory + i + offset));
                    }
                    offset += MODULE_TEXTURE_COUNTER_NUM;
                    /* module PE */
                    for (i = 0; i < MODULE_PIXEL_ENGINE_COUNTER_NUM; i++)
                    {
                        gcmPRINT("PE counter #%d: %d\n", i, *((gctUINT32_PTR)memory + i + offset));
                    }
                    offset += MODULE_PIXEL_ENGINE_COUNTER_NUM;
                    /* module MCC */
                    for (i = 0; i < MODULE_MEMORY_CONTROLLER_COLOR_COUNTER_NUM; i++)
                    {
                        gcmPRINT("MCC counter #%d: %d\n", i, *((gctUINT32_PTR)memory + i + offset));
                    }
                    offset += MODULE_MEMORY_CONTROLLER_COLOR_COUNTER_NUM;
                    /* module MCZ */
                    for (i = 0; i < MODULE_MEMORY_CONTROLLER_DEPTH_COUNTER_NUM; i++)
                    {
                        gcmPRINT("MCZ counter #%d: %d\n", i, *((gctUINT32_PTR)memory + i + offset));
                    }
                    offset += MODULE_MEMORY_CONTROLLER_DEPTH_COUNTER_NUM;
                    /* module HI0 */
                    for (i = 0; i < MODULE_HOST_INTERFACE0_COUNTER_NUM; i++)
                    {
                        gcmPRINT("HI0 counter #%d: %d\n", i, *((gctUINT32_PTR)memory + i + offset));
                    }
                    offset += MODULE_HOST_INTERFACE0_COUNTER_NUM;
                    /* module HI1 */
                    for (i = 0; i < MODULE_HOST_INTERFACE1_COUNTER_NUM; i++)
                    {
                        gcmPRINT("HI1 counter #%d: %d\n", i, *((gctUINT32_PTR)memory + i + offset));
                    }
                    offset += MODULE_HOST_INTERFACE1_COUNTER_NUM;
                    /* module L2 */
                    for (i = 0; i < MODULE_GPUL2_CACHE_COUNTER_NUM; i++)
                    {
                        gcmPRINT("L2 counter #%d: %d\n", i, *((gctUINT32_PTR)memory + i + offset));
                    }
                    gcmPRINT("************\n");
                }
                frameNo++;
                _HAL->profiler.probeBuffer.curBufId = 0;

#else

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
#endif
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

#if VIVANTE_PROFILER_PERDRAW
static gctUINT32
CalcDelta(
    IN gctUINT32 new,
    IN gctUINT32 old
    )
{
    if (new >= old)
    {
        return new - old;
    }
    else
    {
        return (gctUINT32)((gctUINT64)new + 0x100000000ll - old);
    }
}
#endif

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
#if VIVANTE_PROFILER_PROBE
    if(!_HAL)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_NOT_SUPPORTED;
    }

    /* TODO */
    if (_HAL->profiler.probeBuffer.curBufId >= NumOfDrawBuf)
    {
        int bufId = 0;
        _HAL->profiler.probeBuffer.curBufId = 0;
        for (; bufId < NumOfDrawBuf;bufId++)
        {
            gcoBUFOBJ_Upload(_HAL->profiler.probeBuffer.newCounterBuf[bufId], gcvNULL, 0, 1024 * sizeof(gctUINT32), gcvBUFOBJ_USAGE_STATIC_DRAW);
            _HAL->profiler.probeBuffer.opType[bufId] = gcvCOUNTER_OP_NONE;
        }
    }

    gcmGETHARDWARE(hardware);
    if (hardware)
    {
        gctUINT32   address;
        gctINT32   module;

        gcoBUFOBJ_Lock(_HAL->profiler.probeBuffer.newCounterBuf[_HAL->profiler.probeBuffer.curBufId], &address, gcvNULL);

        for (module = 0; module < gcvCOUNTER_COUNT; module++)
        {
            gcoHARDWARE_ProbeCounter(hardware, address, (gceCOUNTER)module, gcvTRUE, gcvNULL);
        }
        _HAL->profiler.probeBuffer.opType[_HAL->profiler.probeBuffer.curBufId] = operationType;
    }

#else
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

#endif
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
#if VIVANTE_PROFILER_PROBE
    gceSTATUS status;
    gcoHARDWARE hardware = gcvNULL;
    gcmHEADER();
    if(!_HAL)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_NOT_SUPPORTED;
    }

    /* TODO */
    if (_HAL->profiler.probeBuffer.curBufId >= NumOfDrawBuf)
        return gcvSTATUS_NOT_SUPPORTED;

    gcmGETHARDWARE(hardware);
    if (hardware)
    {
        gctUINT32   address;
        gctINT32    module;

        gcoBUFOBJ_Lock(_HAL->profiler.probeBuffer.newCounterBuf[_HAL->profiler.probeBuffer.curBufId], &address, gcvNULL);

        for (module = 0; module < gcvCOUNTER_COUNT; module++)
        {
            gcoHARDWARE_ProbeCounter(hardware, address, (gceCOUNTER)module, gcvFALSE, gcvNULL);
        }

        _HAL->profiler.probeBuffer.curBufId++;
    }

OnError:
    gcmFOOTER_NO();
#endif

#if VIVANTE_PROFILER_PERDRAW
    gcsHAL_INTERFACE iface;
    gceSTATUS status;
    gcoHARDWARE hardware = gcvNULL;
    gctUINT32 context;
    static gcsPROFILER_COUNTERS precounters;

    gcmHEADER();
    if(!_HAL)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_NOT_SUPPORTED;
    }

    if (!_HAL->profiler.enable)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    /*#if PROFILE_HW_COUNTERS*/
    /* gcvHAL_READ_ALL_PROFILE_REGISTERS. */
    if (_HAL->profiler.enableHW)
    {

        /* Set Register clear Flag. */
        iface.ignoreTLS = gcvFALSE;
        iface.command = gcvHAL_READ_PROFILER_REGISTER_SETTING;
        iface.u.SetProfilerRegisterClear.bclear = gcvFALSE;

        /* Call the kernel. */
        status = gcoOS_DeviceControl(gcvNULL,
                                    IOCTL_GCHAL_INTERFACE,
                                    &iface, gcmSIZEOF(iface),
                                    &iface, gcmSIZEOF(iface));
        /* Verify result. */
        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_NO();
            return gcvSTATUS_GENERIC_IO;
        }

        iface.command = gcvHAL_READ_ALL_PROFILE_REGISTERS;
        gcmGETHARDWARE(hardware);
        if (hardware)
        {
            gcmVERIFY_OK(gcoHARDWARE_GetContext(hardware, &context));
            if (context != 0)
                iface.u.RegisterProfileData.context = context;
        }

        /* Call the kernel. */
        status = gcoOS_DeviceControl(gcvNULL,
            IOCTL_GCHAL_INTERFACE,
            &iface, gcmSIZEOF(iface),
            &iface, gcmSIZEOF(iface));

        /* Verify result. */
        if (gcmNO_ERROR(status) && !_HAL->profiler.disableOutputCounter)
        {
#define gcmCOUNTERCOMPARE(name)    CalcDelta((iface.u.RegisterProfileData.counters.name), (precounters.name))

            if(FirstDraw == gcvTRUE)
            {
                gcoOS_ZeroMemory(&precounters, gcmSIZEOF(precounters));
            }

            gcmWRITE_CONST(VPG_GPU);
            gcmWRITE_COUNTER(VPC_GPUREAD64BYTE, gcmCOUNTERCOMPARE(gpuTotalRead64BytesPerFrame));
            gcmWRITE_COUNTER(VPC_GPUWRITE64BYTE, gcmCOUNTERCOMPARE(gpuTotalWrite64BytesPerFrame));
            gcmWRITE_COUNTER(VPC_GPUCYCLES, gcmCOUNTERCOMPARE(gpuCyclesCounter));
            gcmWRITE_COUNTER(VPC_GPUTOTALCYCLES, gcmCOUNTERCOMPARE(gpuTotalCyclesCounter));
            gcmWRITE_COUNTER(VPC_GPUIDLECYCLES, gcmCOUNTERCOMPARE(gpuIdleCyclesCounter));
            gcmWRITE_CONST(VPG_END);

            gcmWRITE_CONST(VPG_FE);
            gcmWRITE_COUNTER(VPC_FEDRAWCOUNT, gcmCOUNTERCOMPARE(fe_draw_count) );
            gcmWRITE_COUNTER(VPC_FEOUTVERTEXCOUNT, gcmCOUNTERCOMPARE(fe_out_vertex_count));
            gcmWRITE_COUNTER(VPC_FESTALLCOUNT, gcmCOUNTERCOMPARE(fe_stall_count));
            gcmWRITE_COUNTER(VPC_FESTARVECOUNT, gcmCOUNTERCOMPARE(fe_starve_count));
            gcmWRITE_CONST(VPG_END);

            gcmWRITE_CONST(VPG_VS);
            gcmWRITE_COUNTER(VPC_VSINSTCOUNT, gcmCOUNTERCOMPARE(vs_inst_counter));
            gcmWRITE_COUNTER(VPC_VSBRANCHINSTCOUNT, gcmCOUNTERCOMPARE(vtx_branch_inst_counter));
            gcmWRITE_COUNTER(VPC_VSTEXLDINSTCOUNT, gcmCOUNTERCOMPARE(vtx_texld_inst_counter));
            gcmWRITE_COUNTER(VPC_VSRENDEREDVERTCOUNT, gcmCOUNTERCOMPARE(rendered_vertice_counter));
            gcmWRITE_COUNTER(VPC_VSNONIDLESTARVECOUNT, gcmCOUNTERCOMPARE(vs_non_idle_starve_count));
            gcmWRITE_COUNTER(VPC_VSSTARVELCOUNT, gcmCOUNTERCOMPARE(vs_starve_count));
            gcmWRITE_COUNTER(VPC_VSSTALLCOUNT, gcmCOUNTERCOMPARE(vs_stall_count));
            gcmWRITE_COUNTER(VPC_VSPROCESSCOUNT, gcmCOUNTERCOMPARE(vs_process_count));
            gcmWRITE_CONST(VPG_END);

            gcmWRITE_CONST(VPG_PA);
            gcmWRITE_COUNTER(VPC_PAINVERTCOUNT, gcmCOUNTERCOMPARE(pa_input_vtx_counter));
            gcmWRITE_COUNTER(VPC_PAINPRIMCOUNT, gcmCOUNTERCOMPARE(pa_input_prim_counter));
            gcmWRITE_COUNTER(VPC_PAOUTPRIMCOUNT, gcmCOUNTERCOMPARE(pa_output_prim_counter));
            gcmWRITE_COUNTER(VPC_PADEPTHCLIPCOUNT, gcmCOUNTERCOMPARE(pa_depth_clipped_counter));
            gcmWRITE_COUNTER(VPC_PATRIVIALREJCOUNT, gcmCOUNTERCOMPARE(pa_trivial_rejected_counter));
            gcmWRITE_COUNTER(VPC_PACULLCOUNT, gcmCOUNTERCOMPARE(pa_culled_counter));
            gcmWRITE_COUNTER(VPC_PANONIDLESTARVECOUNT, gcmCOUNTERCOMPARE(pa_non_idle_starve_count));
            gcmWRITE_COUNTER(VPC_PASTARVELCOUNT, gcmCOUNTERCOMPARE(pa_starve_count));
            gcmWRITE_COUNTER(VPC_PASTALLCOUNT, gcmCOUNTERCOMPARE(pa_stall_count));
            gcmWRITE_COUNTER(VPC_PAPROCESSCOUNT, gcmCOUNTERCOMPARE(pa_process_count));
            gcmWRITE_CONST(VPG_END);

            gcmWRITE_CONST(VPG_SETUP);
            gcmWRITE_COUNTER(VPC_SETRIANGLECOUNT, gcmCOUNTERCOMPARE(se_culled_triangle_count));
            gcmWRITE_COUNTER(VPC_SELINECOUNT, gcmCOUNTERCOMPARE(se_culled_lines_count));
            gcmWRITE_COUNTER(VPC_SESTARVECOUNT, gcmCOUNTERCOMPARE(se_starve_count));
            gcmWRITE_COUNTER(VPC_SESTALLCOUNT, gcmCOUNTERCOMPARE(se_stall_count));
            gcmWRITE_COUNTER(VPC_SERECEIVETRIANGLECOUNT, gcmCOUNTERCOMPARE(se_receive_triangle_count));
            gcmWRITE_COUNTER(VPC_SESENDTRIANGLECOUNT, gcmCOUNTERCOMPARE(se_send_triangle_count));
            gcmWRITE_COUNTER(VPC_SERECEIVELINESCOUNT, gcmCOUNTERCOMPARE(se_receive_lines_count));
            gcmWRITE_COUNTER(VPC_SESENDLINESCOUNT, gcmCOUNTERCOMPARE(se_send_lines_count));
            gcmWRITE_COUNTER(VPC_SEPROCESSCOUNT, gcmCOUNTERCOMPARE(se_process_count));
            gcmWRITE_COUNTER(VPC_SENONIDLESTARVECOUNT, gcmCOUNTERCOMPARE(se_non_idle_starve_count));
            gcmWRITE_CONST(VPG_END);

            gcmWRITE_CONST(VPG_RA);
            gcmWRITE_COUNTER(VPC_RAVALIDPIXCOUNT, gcmCOUNTERCOMPARE(ra_valid_pixel_count));
            gcmWRITE_COUNTER(VPC_RATOTALQUADCOUNT, gcmCOUNTERCOMPARE(ra_total_quad_count));
            gcmWRITE_COUNTER(VPC_RAVALIDQUADCOUNTEZ, gcmCOUNTERCOMPARE(ra_valid_quad_count_after_early_z));
            gcmWRITE_COUNTER(VPC_RATOTALPRIMCOUNT, gcmCOUNTERCOMPARE(ra_total_primitive_count));
            gcmWRITE_COUNTER(VPC_RAPIPECACHEMISSCOUNT, gcmCOUNTERCOMPARE(ra_pipe_cache_miss_counter));
            gcmWRITE_COUNTER(VPC_RAPREFCACHEMISSCOUNT, gcmCOUNTERCOMPARE(ra_prefetch_cache_miss_counter));
            gcmWRITE_COUNTER(VPC_RAEEZCULLCOUNT, gcmCOUNTERCOMPARE(ra_eez_culled_counter));
            gcmWRITE_COUNTER(VPC_RANONIDLESTARVECOUNT, gcmCOUNTERCOMPARE(ra_non_idle_starve_count));
            gcmWRITE_COUNTER(VPC_RASTARVELCOUNT, gcmCOUNTERCOMPARE(ra_starve_count));
            gcmWRITE_COUNTER(VPC_RASTALLCOUNT, gcmCOUNTERCOMPARE(ra_stall_count));
            gcmWRITE_COUNTER(VPC_RAPROCESSCOUNT, gcmCOUNTERCOMPARE(ra_process_count));
            gcmWRITE_CONST(VPG_END);

            gcmWRITE_CONST(VPG_TX);
            gcmWRITE_COUNTER(VPC_TXTOTBILINEARREQ, gcmCOUNTERCOMPARE(tx_total_bilinear_requests));
            gcmWRITE_COUNTER(VPC_TXTOTTRILINEARREQ, gcmCOUNTERCOMPARE(tx_total_trilinear_requests));
            gcmWRITE_COUNTER(VPC_TXTOTTEXREQ, gcmCOUNTERCOMPARE(tx_total_texture_requests));
            gcmWRITE_COUNTER(VPC_TXMEMREADCOUNT, gcmCOUNTERCOMPARE(tx_mem_read_count));
            gcmWRITE_COUNTER(VPC_TXMEMREADIN8BCOUNT, gcmCOUNTERCOMPARE(tx_mem_read_in_8B_count));
            gcmWRITE_COUNTER(VPC_TXCACHEMISSCOUNT, gcmCOUNTERCOMPARE(tx_cache_miss_count));
            gcmWRITE_COUNTER(VPC_TXCACHEHITTEXELCOUNT, gcmCOUNTERCOMPARE(tx_cache_hit_texel_count));
            gcmWRITE_COUNTER(VPC_TXCACHEMISSTEXELCOUNT, gcmCOUNTERCOMPARE(tx_cache_miss_texel_count));
            gcmWRITE_COUNTER(VPC_TXNONIDLESTARVECOUNT, gcmCOUNTERCOMPARE(tx_non_idle_starve_count));
            gcmWRITE_COUNTER(VPC_TXSTARVELCOUNT, gcmCOUNTERCOMPARE(tx_starve_count));
            gcmWRITE_COUNTER(VPC_TXSTALLCOUNT, gcmCOUNTERCOMPARE(tx_stall_count));
            gcmWRITE_COUNTER(VPC_TXPROCESSCOUNT, gcmCOUNTERCOMPARE(tx_process_count));
            gcmWRITE_CONST(VPG_END);

            gcmWRITE_CONST(VPG_PS);
            gcmWRITE_COUNTER(VPC_PSINSTCOUNT, gcmCOUNTERCOMPARE(ps_inst_counter) );
            gcmWRITE_COUNTER(VPC_PSBRANCHINSTCOUNT, gcmCOUNTERCOMPARE(pxl_branch_inst_counter));
            gcmWRITE_COUNTER(VPC_PSTEXLDINSTCOUNT, gcmCOUNTERCOMPARE(pxl_texld_inst_counter));
            gcmWRITE_COUNTER(VPC_PSRENDEREDPIXCOUNT, gcmCOUNTERCOMPARE(rendered_pixel_counter));
            gcmWRITE_COUNTER(VPC_PSNONIDLESTARVECOUNT, gcmCOUNTERCOMPARE(ps_non_idle_starve_count));
            gcmWRITE_COUNTER(VPC_PSSTARVELCOUNT, gcmCOUNTERCOMPARE(ps_starve_count));
            gcmWRITE_COUNTER(VPC_PSSTALLCOUNT, gcmCOUNTERCOMPARE(ps_stall_count));
            gcmWRITE_COUNTER(VPC_PSPROCESSCOUNT, gcmCOUNTERCOMPARE(ps_process_count));
            gcmWRITE_COUNTER(VPC_PSSHADERCYCLECOUNT, gcmCOUNTERCOMPARE(shader_cycle_count));
            gcmWRITE_CONST(VPG_END);

            gcmWRITE_CONST(VPG_PE);
            gcmWRITE_COUNTER(VPC_PEKILLEDBYCOLOR, gcmCOUNTERCOMPARE(pe_pixel_count_killed_by_color_pipe));
            gcmWRITE_COUNTER(VPC_PEKILLEDBYDEPTH, gcmCOUNTERCOMPARE(pe_pixel_count_killed_by_depth_pipe));
            gcmWRITE_COUNTER(VPC_PEDRAWNBYCOLOR, gcmCOUNTERCOMPARE(pe_pixel_count_drawn_by_color_pipe));
            gcmWRITE_COUNTER(VPC_PEDRAWNBYDEPTH, gcmCOUNTERCOMPARE(pe_pixel_count_drawn_by_depth_pipe));
            gcmWRITE_CONST(VPG_END);

            gcmWRITE_CONST(VPG_MC);
            gcmWRITE_COUNTER(VPC_MCREADREQ8BPIPE, gcmCOUNTERCOMPARE(mc_total_read_req_8B_from_pipeline));
            gcmWRITE_COUNTER(VPC_MCREADREQ8BIP, gcmCOUNTERCOMPARE(mc_total_read_req_8B_from_IP));
            gcmWRITE_COUNTER(VPC_MCWRITEREQ8BPIPE, gcmCOUNTERCOMPARE(mc_total_write_req_8B_from_pipeline));
            gcmWRITE_COUNTER(VPC_MCAXIMINLATENCY, gcmCOUNTERCOMPARE(mc_axi_min_latency));
            gcmWRITE_COUNTER(VPC_MCAXIMAXLATENCY, gcmCOUNTERCOMPARE(mc_axi_max_latency));
            gcmWRITE_COUNTER(VPC_MCAXIMAXLATENCY, gcmCOUNTERCOMPARE(mc_axi_total_latency));
            gcmWRITE_COUNTER(VPC_MCAXISAMPLECOUNT, gcmCOUNTERCOMPARE(mc_axi_sample_count));
            gcmWRITE_CONST(VPG_END);

            gcmWRITE_CONST(VPG_AXI);
            gcmWRITE_COUNTER(VPC_AXIREADREQSTALLED, gcmCOUNTERCOMPARE(hi_axi_cycles_read_request_stalled));
            gcmWRITE_COUNTER(VPC_AXIWRITEREQSTALLED, gcmCOUNTERCOMPARE(hi_axi_cycles_write_request_stalled));
            gcmWRITE_COUNTER(VPC_AXIWRITEDATASTALLED, gcmCOUNTERCOMPARE(hi_axi_cycles_write_data_stalled));
            gcmWRITE_CONST(VPG_END);

            gcmWRITE_CONST(VPG_END);

            gcoOS_MemCopy(&precounters,&iface.u.RegisterProfileData.counters,gcmSIZEOF(precounters));

        }
    }

OnError:
    gcmFOOTER_NO();

#endif

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
