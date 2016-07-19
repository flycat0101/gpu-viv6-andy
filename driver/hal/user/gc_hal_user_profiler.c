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

#else

#define gcmWRITE_STRING(String) \
    do \
    { \
    gceSTATUS status; \
    gctINT32 length; \
    length = (gctINT32) gcoOS_StrLen((gctSTRING) String, gcvNULL); \
    gcmERR_BREAK(gcoPROFILER_Write(_HAL, length, String)); \
    } \
    while (gcvFALSE)

#define gcmWRITE_BUFFER(Buffer, ByteCount) \
    do \
    { \
    gceSTATUS status; \
    gcmERR_BREAK(gcoPROFILER_Write(_HAL, ByteCount, String)); \
    } \
    while (gcvFALSE)

#define gcmPRINT_XML_COUNTER(Counter) \
    do \
    { \
    char buffer[256]; \
    gctUINT offset = 0; \
    gceSTATUS status; \
    gcmERR_BREAK(gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), \
    &offset, \
    "<%s value=\"%d\"/>\n", \
# Counter, \
    _HAL->profiler.Counter)); \
    gcmWRITE_STRING(buffer); \
    } \
    while (gcvFALSE)

#define gcmPRINT_XML(Format, Value) \
    do \
    { \
    char buffer[256]; \
    gctUINT offset = 0; \
    gceSTATUS status; \
    gcmERR_BREAK(gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), \
    &offset, \
    Format, \
    Value)); \
    gcmWRITE_STRING(buffer); \
    } \
    while (gcvFALSE)
#endif

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

gceSTATUS
gcoPROFILER_Initialize(
    IN gcoHAL Hal,
    IN gctBOOL Enable
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    char profilerName[256] = {'\0'};
    char inputFileName[256] = {'\0'};
    char* fileName = gcvNULL;
    char* filter = gcvNULL;
    char* env = gcvNULL;
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
        gcoHAL_ConfigPowerManagement(gcvTRUE);
        /* disable profiler in kernel. */
        iface.command = gcvHAL_SET_PROFILE_SETTING;
        iface.u.SetProfileSetting.enable = gcvFALSE;

        /* Call the kernel. */
        status = gcoOS_DeviceControl(gcvNULL,
            IOCTL_GCHAL_INTERFACE,
            &iface, gcmSIZEOF(iface),
            &iface, gcmSIZEOF(iface));


#if VIVANTE_PROFILER_PROBE
        if (_HAL)
        {
            int bufId = 0;
            gcoBUFOBJ counterBuf;
            gctHANDLE pid = gcoOS_GetCurrentProcessID();
            static gctUINT8 num = 1;
            gctUINT offset = 0;

            char* pos = gcvNULL;

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
    if(fileName) gcoOS_StrCatSafe(inputFileName,256,fileName);

    _HAL->profiler.enablePrint = gcvFALSE;
    gcoOS_GetEnv(gcvNULL,
        "VP_ENABLE_PRINT",
        &env);
    if ((env != gcvNULL) && gcmIS_SUCCESS(gcoOS_StrCmp(env, "1")))
    {
        _HAL->profiler.enablePrint = gcvTRUE;
    }

    _HAL->profiler.useSocket = gcvFALSE;
    _HAL->profiler.disableOutputCounter = gcvFALSE;
    if (!fileName || *fileName == '\0' || *fileName == ' ')
    {
        fileName = DEFAULT_PROFILE_FILE_NAME;
        if(fileName) gcoOS_StrCatSafe(inputFileName,256,fileName);
    }
#if VIVANTE_PROFILER_CONTEXT
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
    }
#endif
    if (! _HAL->profiler.useSocket)
    {
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
    iface.command = gcvHAL_SET_PROFILE_SETTING;
    iface.u.SetProfileSetting.enable = gcvTRUE;
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
    int bufId = 0;
    gcoBUFOBJ counterBuf;
    for (; bufId < NumOfDrawBuf; bufId++)
    {
        gcoBUFOBJ_Construct(_HAL, gcvBUFOBJ_TYPE_GENERIC_BUFFER, &counterBuf);
        gcoBUFOBJ_Upload(counterBuf, gcvNULL, 0, 1024 * sizeof(gctUINT32), gcvBUFOBJ_USAGE_STATIC_DRAW);
        _HAL->profiler.probeBuffer.newCounterBuf[bufId] = (gctHANDLE)counterBuf;
    }
    _HAL->profiler.probeBuffer.curBufId = 0;
#endif
    gcoHARDWARE_EnableCounters(gcvNULL, gcvNULL);

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
        gcmVERIFY_OK(gcoOS_Close(gcvNULL, _HAL->profiler.file));
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
        status = gcoOS_Write(gcvNULL,
            _HAL->profiler.file,
            ByteCount, Data);
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
        status = gcoOS_Flush(gcvNULL,
            _HAL->profiler.file);
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
    if (!_HAL)
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
            gcmWRITE_XML_STRING("\t\t");gcmWRITE_XML_Value("FrameNUM", frameNo, 1);

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
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("VtxOuputCount",*((gctUINT32_PTR)memory + 0 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("VtxCacheMissCount",*((gctUINT32_PTR)memory + 1 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("VtxCacheLookupCount",*((gctUINT32_PTR)memory + 2 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("StallCount",*((gctUINT32_PTR)memory + 3 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("ProcessCount",*((gctUINT32_PTR)memory + 4 + offset));
                gcmWRITE_XML_STRING("\t\t\t</FECounters>\n");
                offset += MODULE_FRONT_END_COUNTER_NUM;

                /* module VS */
                gcmWRITE_XML_STRING("\t\t\t<VSCounters>\n");
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("CycleCounter",*((gctUINT32_PTR)memory + 0 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("PixelInstrPerCore",*((gctUINT32_PTR)memory + 1 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalPixelShaded",*((gctUINT32_PTR)memory + 2 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("VertexInstrPerCore",*((gctUINT32_PTR)memory + 3 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("VertexExecute",*((gctUINT32_PTR)memory + 4 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("VertexBranchInstrExecute",*((gctUINT32_PTR)memory + 5 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("VertexTextureInstrExecute",*((gctUINT32_PTR)memory + 6 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("PixelBranchInstrExecute",*((gctUINT32_PTR)memory + 7 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("PixelTextureInstrExecute",*((gctUINT32_PTR)memory + 8 + offset));
                gcmWRITE_XML_STRING("\t\t\t</VSCounters>\n");
                offset += MODULE_VERTEX_SHADER_COUNTER_NUM;

                /* module PA */
                gcmWRITE_XML_STRING("\t\t\t<PACounters>\n");
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("VTXCount",*((gctUINT32_PTR)memory + 0 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("InPrimCount",*((gctUINT32_PTR)memory + 1 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("OutPrimCount",*((gctUINT32_PTR)memory + 2 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("tRejCount",*((gctUINT32_PTR)memory + 3 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("CullPrimCount",*((gctUINT32_PTR)memory + 4 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("DropPrimCount",*((gctUINT32_PTR)memory + 5 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("FrstClipPrimCount",*((gctUINT32_PTR)memory + 6 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("FrstClipDropPrimCount",*((gctUINT32_PTR)memory + 7 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("NonIdleStarveCount",*((gctUINT32_PTR)memory + 8 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("StarveCount",*((gctUINT32_PTR)memory + 9 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("StallCount",*((gctUINT32_PTR)memory + 10 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("ProcessCount",*((gctUINT32_PTR)memory + 11 + offset));
                gcmWRITE_XML_STRING("\t\t\t</PACounters>\n");
                offset += MODULE_PRIMITIVE_ASSEMBLY_COUNTER_NUM;

                /* module SE */
                gcmWRITE_XML_STRING("\t\t\t<SECounters>\n");
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("StarveCount",*((gctUINT32_PTR)memory + 0 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("StallCount",*((gctUINT32_PTR)memory + 1 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("InputTriCount",*((gctUINT32_PTR)memory + 2 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("OutputTriCount",*((gctUINT32_PTR)memory + 3 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("InputLineCount",*((gctUINT32_PTR)memory + 4 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("OutputLineCount",*((gctUINT32_PTR)memory + 5 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("ProcessCount",*((gctUINT32_PTR)memory + 6 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("ClippedTriCount",*((gctUINT32_PTR)memory + 7 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("ClippedLineCount",*((gctUINT32_PTR)memory + 8 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("CulledTriCount",*((gctUINT32_PTR)memory + 9 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("CulledLineCount",*((gctUINT32_PTR)memory + 10 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TrivialRejLineCount",*((gctUINT32_PTR)memory + 11 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("NonIdleStartveCount",*((gctUINT32_PTR)memory + 12 + offset));
                gcmWRITE_XML_STRING("\t\t\t</SECounters>\n");
                offset += MODULE_SETUP_COUNTER_NUM;

                /* module RA */
                gcmWRITE_XML_STRING("\t\t\t<RACounters>\n");
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("PrimInputCount",*((gctUINT32_PTR)memory + 0 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("QuadCountFromScan",*((gctUINT32_PTR)memory + 1 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("ZCacheMiss",*((gctUINT32_PTR)memory + 2 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("HZCacheMiss",*((gctUINT32_PTR)memory + 3 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("QuadCountAfterHZorEEZ",*((gctUINT32_PTR)memory + 4 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("ValidPixelCountToRender",*((gctUINT32_PTR)memory + 5 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("OutputValidQuadCount",*((gctUINT32_PTR)memory + 6 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("OutputValidPixelCount",*((gctUINT32_PTR)memory + 7 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("HZPipeCacheMiss",*((gctUINT32_PTR)memory + 8 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("RARenderPipe1CacheMiss",*((gctUINT32_PTR)memory + 9 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("NonIdleStarveCount",*((gctUINT32_PTR)memory + 10 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("StarveCount",*((gctUINT32_PTR)memory + 11 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("StallCount",*((gctUINT32_PTR)memory + 12 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("ProcessCount",*((gctUINT32_PTR)memory + 13 + offset));
                gcmWRITE_XML_STRING("\t\t\t</RACounters>\n");
                offset += MODULE_RASTERIZER_COUNTER_NUM;

                /* module PS */
                gcmWRITE_XML_STRING("\t\t\t<PSCounters>\n");
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("CycleCounter",*((gctUINT32_PTR)memory + 0 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("PixelInstrPerCore",*((gctUINT32_PTR)memory + 1 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalPixelShaded",*((gctUINT32_PTR)memory + 2 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("VertexInstrPerCore",*((gctUINT32_PTR)memory + 3 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("VertexExecute",*((gctUINT32_PTR)memory + 4 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("VertexBranchInstrExecute",*((gctUINT32_PTR)memory + 5 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("VertexTextureInstrExecute",*((gctUINT32_PTR)memory + 6 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("PixelBranchInstrExecute",*((gctUINT32_PTR)memory + 7 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("PixelTextureInstrExecute",*((gctUINT32_PTR)memory + 8 + offset));
                gcmWRITE_XML_STRING("\t\t\t</PSCounters>\n");
                offset += MODULE_PIXEL_SHADER_COUNTER_NUM;

                /* module TX */
                gcmWRITE_XML_STRING("\t\t\t<TXCounters>\n");
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("BiLinearIncomingPixel",*((gctUINT32_PTR)memory + 0 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TriLinearIncomingPixel",*((gctUINT32_PTR)memory + 1 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("DiscardIncomingPixel",*((gctUINT32_PTR)memory + 2 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalIncomingPixel",*((gctUINT32_PTR)memory + 3 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("MC0MissCount",*((gctUINT32_PTR)memory + 4 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("MC0MemoryRequestByte",*((gctUINT32_PTR)memory + 5 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("MC1MissCount",*((gctUINT32_PTR)memory + 6 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("MC1MemoryRequestByte",*((gctUINT32_PTR)memory + 7 + offset));
                gcmWRITE_XML_STRING("\t\t\t</TXCounters>\n");
                offset += MODULE_TEXTURE_COUNTER_NUM;

                /* module PE */
                gcmWRITE_XML_STRING("\t\t\t<PECounters>\n");
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("PE0ColorKilledCount",*((gctUINT32_PTR)memory + 0 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("PE0DepthKilledCount",*((gctUINT32_PTR)memory + 1 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("PE0ColorDrawnCount",*((gctUINT32_PTR)memory + 2 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("PE0DepthDrawnCount",*((gctUINT32_PTR)memory + 3 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("PE1ColorKilledCount",*((gctUINT32_PTR)memory + 4 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("PE1DepthKilledCount",*((gctUINT32_PTR)memory + 5 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("PE1ColorDrawnCount",*((gctUINT32_PTR)memory + 6 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("PE1DepthDrawnCount",*((gctUINT32_PTR)memory + 7 + offset));
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
                    gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("MaxLatency",max_latency);
                    gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("MinLatency",min_latency);
                }
                else
                {
                    gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("MinMaxLatency",*((gctUINT32_PTR)memory + 0 + offset));
                }
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("Total_Latency",*((gctUINT32_PTR)memory + 1 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalSampleCount",*((gctUINT32_PTR)memory + 2 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalReadin8BFromPE",*((gctUINT32_PTR)memory + 3 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalReadin8BFromComp",*((gctUINT32_PTR)memory + 4 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalWritein8BFromPE",*((gctUINT32_PTR)memory + 5 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalReadReqFromPE",*((gctUINT32_PTR)memory + 6 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalWriteReqFromPE",*((gctUINT32_PTR)memory + 7 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalReadRin8BFromOthers",*((gctUINT32_PTR)memory + 8 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalWritein8BFromOthers",*((gctUINT32_PTR)memory + 9 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalReadReqFromOthers",*((gctUINT32_PTR)memory + 10 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalWriteReqFromOthers",*((gctUINT32_PTR)memory + 11 + offset));
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
                    gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("MaxLatency",max_latency);
                    gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("MinLatency",min_latency);
                }
                else
                {
                    gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("MinMaxLatency",*((gctUINT32_PTR)memory + 0 + offset));
                }
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("Total_Latency",*((gctUINT32_PTR)memory + 1 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalSampleCount",*((gctUINT32_PTR)memory + 2 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalReadin8BFromPE",*((gctUINT32_PTR)memory + 3 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalReadin8BFromComp",*((gctUINT32_PTR)memory + 4 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalWritein8BFromPE",*((gctUINT32_PTR)memory + 5 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalReadReqFromPE",*((gctUINT32_PTR)memory + 6 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalWriteReqFromPE",*((gctUINT32_PTR)memory + 7 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalReadRin8BFromOthers",*((gctUINT32_PTR)memory + 8 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalWritein8BFromOthers",*((gctUINT32_PTR)memory + 9 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalReadReqFromOthers",*((gctUINT32_PTR)memory + 10 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalWriteReqFromOthers",*((gctUINT32_PTR)memory + 11 + offset));
                gcmWRITE_XML_STRING("\t\t\t</MCZCounters>\n");
                offset += MODULE_MEMORY_CONTROLLER_DEPTH_COUNTER_NUM;

                /* module HI0 */
                gcmWRITE_XML_STRING("\t\t\t<HI0Counters>\n");
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalReadsin64bits",*((gctUINT32_PTR)memory + 0 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalWritesin64bits",*((gctUINT32_PTR)memory + 1 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalWriteReqCount",*((gctUINT32_PTR)memory + 2 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalReadReqCount",*((gctUINT32_PTR)memory + 3 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("NumberOfCyclesAXIReadReqisStalled",*((gctUINT32_PTR)memory + 4 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("NumberOfCyclesAXIWriteReqisStalled",*((gctUINT32_PTR)memory + 5 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("NumberOfCyclesAXIWriteDataisStalled",*((gctUINT32_PTR)memory + 6 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalCycle",*((gctUINT32_PTR)memory + 7 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalIdleCycle",*((gctUINT32_PTR)memory + 8 + offset));
                gcmWRITE_XML_STRING("\t\t\t</HI0Counters>\n");
                offset += MODULE_HOST_INTERFACE0_COUNTER_NUM;

                /* module HI1 */
                gcmWRITE_XML_STRING("\t\t\t<HI1Counters>\n");
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalReadsin64bits",*((gctUINT32_PTR)memory + 0 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalWritesin64bits",*((gctUINT32_PTR)memory + 1 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalWriteReqCount",*((gctUINT32_PTR)memory + 2 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalReadReqCount",*((gctUINT32_PTR)memory + 3 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("NumberOfCyclesAXIReadReqisStalled",*((gctUINT32_PTR)memory + 4 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("NumberOfCyclesAXIWriteReqisStalled",*((gctUINT32_PTR)memory + 5 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("NumberOfCyclesAXIWriteDataisStalled",*((gctUINT32_PTR)memory + 6 + offset));
                gcmWRITE_XML_STRING("\t\t\t</HI1Counters>\n");
                offset += MODULE_HOST_INTERFACE1_COUNTER_NUM;

                /* module L2 */
                gcmWRITE_XML_STRING("\t\t\t<GPUL2Counters>\n");
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalAXI0ReadReq",*((gctUINT32_PTR)memory + 0 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalAXI1ReadReq",*((gctUINT32_PTR)memory + 1 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalAXI0WriteReq",*((gctUINT32_PTR)memory + 2 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalAXI1WriteReq",*((gctUINT32_PTR)memory + 3 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalAXI0ReadTransReq",*((gctUINT32_PTR)memory + 4 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalAXI1ReadTransReq",*((gctUINT32_PTR)memory + 5 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalAXI0WriteTransReq",*((gctUINT32_PTR)memory + 6 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("TotalAXI1WriteTransReq",*((gctUINT32_PTR)memory + 7 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("AXI0MinMaxLatency",*((gctUINT32_PTR)memory + 8 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("AXI0TotalLatency",*((gctUINT32_PTR)memory + 9 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("AXI0TotalReq",*((gctUINT32_PTR)memory + 10 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("AXI1MinMaxLatency",*((gctUINT32_PTR)memory + 11 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("AXI1TotalLatency",*((gctUINT32_PTR)memory + 12 + offset));
                gcmWRITE_XML_STRING("\t\t\t\t");gcmWRITE_XML_COUNTER("AXI1TotalReq",*((gctUINT32_PTR)memory + 13 + offset));
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

        iface.command = gcvHAL_READ_ALL_PROFILE_REGISTERS;

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
                    gcmPRINT("frame %d\n", frameNo);
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
                    gcmPRINT("VPC_GPUTOTALCYCLES: %d\n", gcmCOUNTER(gpuTotalCyclesCounter));
                    gcmPRINT("VPC_GPUIDLECYCLES: %d\n", gcmCOUNTER(gpuIdleCyclesCounter));
                    gcmPRINT("*********\n");

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

                    gcmPRINT("VPG_PE\n");
                    gcmPRINT("VPC_PEKILLEDBYCOLOR: %d\n", gcmCOUNTER(pe_pixel_count_killed_by_color_pipe));
                    gcmPRINT("VPC_PEKILLEDBYDEPTH: %d\n", gcmCOUNTER(pe_pixel_count_killed_by_depth_pipe));
                    gcmPRINT("VPC_PEDRAWNBYCOLOR: %d\n", gcmCOUNTER(pe_pixel_count_drawn_by_color_pipe));
                    gcmPRINT("VPC_PEDRAWNBYDEPTH: %d\n", gcmCOUNTER(pe_pixel_count_drawn_by_depth_pipe));
                    gcmPRINT("*********\n");

                    gcmPRINT("VPG_MC\n");
                    gcmPRINT("VPC_MCREADREQ8BPIPE: %d\n", gcmCOUNTER(mc_total_read_req_8B_from_pipeline));
                    gcmPRINT("VPC_MCREADREQ8BIP: %d\n", gcmCOUNTER(mc_total_read_req_8B_from_IP));
                    gcmPRINT("VPC_MCWRITEREQ8BPIPE: %d\n", gcmCOUNTER(mc_total_write_req_8B_from_pipeline));
                    gcmPRINT("VPC_MCAXIMINLATENCY: %d\n", gcmCOUNTER(mc_axi_min_latency));
                    gcmPRINT("VPC_MCAXIMAXLATENCY: %d\n", gcmCOUNTER(mc_axi_max_latency));
                    gcmPRINT("VPC_MCAXITOTALLATENCY: %d\n", gcmCOUNTER(mc_axi_total_latency));
                    gcmPRINT("VPC_MCAXISAMPLECOUNT: %d\n", gcmCOUNTER(mc_axi_sample_count));
                    gcmPRINT("*********\n");;

                    gcmPRINT("VPG_AXI\n");
                    gcmPRINT("VPC_AXIREADREQSTALLED: %d\n", gcmCOUNTER(hi_axi_cycles_read_request_stalled));
                    gcmPRINT("VPC_AXIWRITEREQSTALLED: %d\n", gcmCOUNTER(hi_axi_cycles_write_request_stalled));
                    gcmPRINT("VPC_AXIWRITEDATASTALLED: %d\n", gcmCOUNTER(hi_axi_cycles_write_data_stalled));
                    gcmPRINT("*********\n");

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

OnError:
    gcmFOOTER_NO();
#endif
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
