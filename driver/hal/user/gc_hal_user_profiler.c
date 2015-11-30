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


/*******************************************************************************
**    Profiler for Vivante HAL.
*/
#include "gc_hal_user_precomp.h"

#define _GC_OBJ_ZONE            gcvZONE_HAL

#if gcdENABLE_3D
#if VIVANTE_PROFILER

#ifdef ANDROID
#if gcdNEW_PROFILER_FILE
#define DEFAULT_PROFILE_FILE_NAME   "/sdcard/vprofiler.vpd"
#else
#define DEFAULT_PROFILE_FILE_NAME   "/sdcard/vprofiler.xml"
#endif
#else
#if gcdNEW_PROFILER_FILE
#define DEFAULT_PROFILE_FILE_NAME   "vprofiler.vpd"
#else
#define DEFAULT_PROFILE_FILE_NAME   "vprofiler.xml"
#endif
#endif

#if gcdNEW_PROFILER_FILE
#if VIVANTE_PROFILER_CONTEXT
#define _HAL Hal
#else
#define _HAL gcPLS.hal
#endif

#if VIVANTE_PROFILER_PROBE
#define   MODULE_FRONT_END                                0x0
#define   MODULE_VERTEX_SHADER                            0x1
#define   MODULE_PRIMITIVE_ASSEMBLY                       0x2
#define   MODULE_SETUP                                    0x3
#define   MODULE_RASTERIZER                               0x4
#define   MODULE_PIXEL_SHADER                             0x5
#define   MODULE_TEXTURE                                  0x6
#define   MODULE_PIXEL_ENGINE                             0x7
#define   MODULE_MEMORY_CONTROLLER_COLOR                  0x8
#define   MODULE_MEMORY_CONTROLLER_DEPTH                  0x9
#define   MODULE_HOST_INTERFACE0                          0xA
#define   MODULE_HOST_INTERFACE1                          0xB
#define   MODULE_GPUL2_CACHE                              0xC

#define   MODULE_FRONT_END_COUNTER_NUM                    0x5
#define   MODULE_VERTEX_SHADER_COUNTER_NUM                0x9
#define   MODULE_PRIMITIVE_ASSEMBLY_COUNTER_NUM           0xC
#define   MODULE_SETUP_COUNTER_NUM                        0xD
#define   MODULE_RASTERIZER_COUNTER_NUM                   0xE
#define   MODULE_PIXEL_SHADER_COUNTER_NUM                 0x9
#define   MODULE_TEXTURE_COUNTER_NUM                      0x8
#define   MODULE_PIXEL_ENGINE_COUNTER_NUM                 0x8
#define   MODULE_MEMORY_CONTROLLER_COLOR_COUNTER_NUM      0xC
#define   MODULE_MEMORY_CONTROLLER_DEPTH_COUNTER_NUM      0xC
#define   MODULE_HOST_INTERFACE0_COUNTER_NUM              0x7
#define   MODULE_HOST_INTERFACE1_COUNTER_NUM              0x7
#define   MODULE_GPUL2_CACHE_COUNTER_NUM                  0xE

#define PROBE_MODULE_COUNTERS(module_id, counter_num) \
        for (i = 0; i < counter_num; i++) \
        { \
            gcoHARDWARE_ProbeCounter(hardware, address + (i + offset) * sizeof(gctUINT32), module_id, i); \
        } \
        offset += counter_num; \

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
#if (gcdENABLE_3D || gcdENABLE_2D)
    gceCHIPMODEL chipModel;
    gctUINT32 chipRevision;
#endif
    gcsHAL_INTERFACE iface;
#ifdef ANDROID
    gctBOOL matchResult = gcvFALSE;
#endif
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
        if (_HAL && hardware)
        {
            int bufId = 0;
            gcoBUFOBJ counterBuf;
            gcoHARDWARE_EnableCounters(hardware);
            for (; bufId < NumOfDrawBuf;bufId++)
            {
                gcoBUFOBJ_Construct(_HAL, gcvBUFOBJ_TYPE_GENERIC_BUFFER, &counterBuf);
                gcoBUFOBJ_Upload(counterBuf, gcvNULL, 0, 1024*sizeof(gctUINT32), gcvBUFOBJ_USAGE_STATIC_DRAW);
                _HAL->profiler.newCounterBuf[bufId] = (gctHANDLE)counterBuf;
            }
            _HAL->profiler.curBufId = 0;
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
    if (fileName && *fileName != '\0' && *fileName != ' ')
    {
    }
    else
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
#if gcdNEW_PROFILER_FILE
            gcvFILE_CREATE,
#else
            gcvFILE_CREATETEXT,
#endif
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

    /* Call the kernel. */
    status = gcoOS_DeviceControl(gcvNULL,
        IOCTL_GCHAL_INTERFACE,
        &iface, gcmSIZEOF(iface),
        &iface, gcmSIZEOF(iface));

    if (gcmIS_ERROR(status))
    {
        _HAL->profiler.enable = 0;

        gcmFOOTER();
        return status;
    }

#if (gcdENABLE_3D || gcdENABLE_2D)
    gcmVERIFY_OK(gcoHAL_QueryChipIdentity(gcvNULL, &chipModel, &chipRevision, gcvNULL, gcvNULL));

    if ((chipModel == gcv1500  && chipRevision == 0x5246) ||
        (chipModel == gcv3000  && chipRevision == 0x5450))
        gcoHARDWARE_EnableCounters(hardware);
#endif

#if VIVANTE_PROFILER_PROBE
    if (hardware)
    {
        int bufId = 0;
        gcoBUFOBJ counterBuf;
        gcoHARDWARE_EnableCounters(hardware);
        for (; bufId < NumOfDrawBuf;bufId++)
        {
            gcoBUFOBJ_Construct(_HAL, gcvBUFOBJ_TYPE_GENERIC_BUFFER, &counterBuf);
            gcoBUFOBJ_Upload(counterBuf, gcvNULL, 0, 1024*sizeof(gctUINT32), gcvBUFOBJ_USAGE_STATIC_DRAW);
            _HAL->profiler.newCounterBuf[bufId] = (gctHANDLE)counterBuf;
        }
        _HAL->profiler.curBufId = 0;
    }
#endif

    _HAL->profiler.enable = 1;
    gcoOS_GetTime(&_HAL->profiler.frameStart);
    _HAL->profiler.frameStartTimeusec = _HAL->profiler.frameStart;

#if gcdNEW_PROFILER_FILE
    gcmWRITE_CONST(VPHEADER);
    gcmWRITE_BUFFER(4, "VP12");
#else
    gcmWRITE_STRING("<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n<VProfile>\n");
#endif

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
#if gcdNEW_PROFILER_FILE
    gcmWRITE_CONST(VPG_END);
#else
    gcmWRITE_STRING("</VProfile>\n");
#endif

    gcoPROFILER_Flush(gcvNULL);
    if (_HAL->profiler.enable )
    {
        {
            /* Close the profiler file. */
            gcmVERIFY_OK(gcoOS_Close(gcvNULL, _HAL->profiler.file));
        }
    }
    glhal_map_delete(_HAL);
#if VIVANTE_PROFILER_PROBE
    {
        gctUINT32 i = 0;
        for (i = 0;  i < NumOfDrawBuf; i++)
            gcoBUFOBJ_Destroy(_HAL->profiler.newCounterBuf[i]);
    }
#endif

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

#if gcdENABLE_3D
#if !gcdNEW_PROFILER_FILE
gceSTATUS
gcoPROFILER_Shader(
                   IN gcoHAL Hal,
                   IN gcSHADER Shader
                   )
{
    /*#if PROFILE_SHADER_COUNTERS*/

    gcmHEADER_ARG("Shader=0x%x", Shader);
    if(!_HAL)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_NOT_SUPPORTED;
    }
    if (_HAL->profiler.enableSH)
    {
        gctUINT16 alu = 0, tex = 0, i;

        if (_HAL->profiler.enable)
        {
            /* Profile shader */
            for (i = 0; i < Shader->codeCount; i++ )
            {
                switch (gcmSL_OPCODE_GET(Shader->code[i].opcode, Opcode))
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

            gcmPRINT_XML("<InstructionCount value=\"%d\"/>\n", tex + alu);
            gcmPRINT_XML("<ALUInstructionCount value=\"%d\"/>\n", alu);
            gcmPRINT_XML("<TextureInstructionCount value=\"%d\"/>\n", tex);
            gcmPRINT_XML("<Attributes value=\"%lu\"/>\n", Shader->attributeCount);
            gcmPRINT_XML("<Uniforms value=\"%lu\"/>\n", Shader->uniformCount);
            gcmPRINT_XML("<Functions value=\"%lu\"/>\n", Shader->functionCount);
        }
        /*#endif*/
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}
#endif

gceSTATUS
gcoPROFILER_ShaderVS(
    IN gcoHAL Hal,
    IN gctPOINTER Vs
    )
{
    /*#if PROFILE_SHADER_COUNTERS*/
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
#if gcdNEW_PROFILER_FILE
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
#else
            gcmWRITE_STRING("<VS>\n");
            gcoPROFILER_Shader(gcvNULL, (gcSHADER) Vs);
            gcmWRITE_STRING("</VS>\n");
#endif
        }
    }
    /*#endif*/

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
gcoPROFILER_ShaderFS(
    IN gcoHAL Hal,
    IN void* Fs
    )
{
    /*#if PROFILE_SHADER_COUNTERS*/
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
#if gcdNEW_PROFILER_FILE
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
#else
            gcmWRITE_STRING("<FS>\n");
            gcoPROFILER_Shader(gcvNULL, (gcSHADER) Fs);
            gcmWRITE_STRING("</FS>\n");
#endif
        }
    }
    /*#endif*/

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

#endif /* vivante_no_3d */

gceSTATUS
gcoPROFILER_EndFrame(
                     IN gcoHAL Hal
                     )
{
    gcsHAL_INTERFACE iface;
    gceSTATUS status;
#if VIVANTE_PROFILER_CONTEXT
    gcoHARDWARE hardware = gcvNULL;
    gctUINT32 context;
#endif
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

#if VIVANTE_PROFILER_PROBE
        /* write per draw counter */
        gcmWRITE_CONST(VPG_ES30_DRAW);

        for (j = 0; j < _HAL->profiler.curBufId; j++)
        {
            gcmWRITE_COUNTER(VPC_ES30_DRAW_NO, j);
            gcmWRITE_CONST(VPG_HW);
            offset = 0;

            gcoBUFOBJ_Lock(_HAL->profiler.newCounterBuf[j], &address, &memory);

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

            gcmWRITE_CONST(VPG_END);
        }

        gcmWRITE_CONST(VPG_END);

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
#if VIVANTE_PROFILER_CONTEXT
        gcmGETHARDWARE(hardware);
        if (hardware)
        {
            gcmVERIFY_OK(gcoHARDWARE_GetContext(hardware, &context));
            if (context != 0)
                iface.u.RegisterProfileData.context = context;
        }
#endif
        /* Call the kernel. */
        status = gcoOS_DeviceControl(gcvNULL,
            IOCTL_GCHAL_INTERFACE,
            &iface, gcmSIZEOF(iface),
            &iface, gcmSIZEOF(iface));

        /* Verify result. */
        if (gcmNO_ERROR(status) && !_HAL->profiler.disableOutputCounter)
        {
#define gcmCOUNTER(name)    iface.u.RegisterProfileData.counters.name

            gcmWRITE_CONST(VPG_HW);
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
            if (hardware)
            {
                gcoHAL_Commit(_HAL, gcvTRUE);

                gcmPRINT("Frame %d:\n", frameNo);
                gcmPRINT("************\n");

                for (j = 0; j < _HAL->profiler.curBufId; j++)
                {
                    gcmPRINT("draw #%d\n", j);
                    gcmPRINT("************\n");
                    gcoBUFOBJ_Lock(_HAL->profiler.newCounterBuf[j], &address, &memory);
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
                _HAL->profiler.curBufId = 0;
            }

#else

            if (_HAL->profiler.enablePrint)
            {
                static gctUINT32 frameNo = 0;
                gcmPRINT("frame %d\n", frameNo);
                gcmPRINT("*********\n");
                frameNo++;

                gcmPRINT("VPG_GPU\n");
                gcmPRINT("VPC_GPUREAD64BYTE: %d\n", gcmCOUNTER(gpuTotalRead64BytesPerFrame));
                gcmPRINT("VPC_GPUWRITE64BYTE: %d\n", gcmCOUNTER(gpuTotalWrite64BytesPerFrame));
                gcmPRINT("VPC_GPUCYCLES: %d\n", gcmCOUNTER(gpuCyclesCounter));
                gcmPRINT("VPC_GPUTOTALCYCLES: %d\n", gcmCOUNTER(gpuTotalCyclesCounter));
                gcmPRINT("VPC_GPUIDLECYCLES: %d\n", gcmCOUNTER(gpuIdleCyclesCounter));
                gcmPRINT("*********\n");

                gcmPRINT("VPG_VS\n");
                gcmPRINT("VPC_VSINSTCOUNT: %d\n", gcmCOUNTER(vs_inst_counter));
                gcmPRINT("VPC_VSBRANCHINSTCOUNT: %d\n", gcmCOUNTER(vtx_branch_inst_counter));
                gcmPRINT("VPC_VSTEXLDINSTCOUNT: %d\n", gcmCOUNTER(vtx_texld_inst_counter));
                gcmPRINT("VPC_VSRENDEREDVERTCOUNT: %d\n", gcmCOUNTER(rendered_vertice_counter));
                gcmPRINT("*********\n");

                gcmPRINT("VPG_PA\n");
                gcmPRINT("VPC_PAINVERTCOUNT: %d\n", gcmCOUNTER(pa_input_vtx_counter));
                gcmPRINT("VPC_PAINPRIMCOUNT: %d\n", gcmCOUNTER(pa_input_prim_counter));
                gcmPRINT("VPC_PAOUTPRIMCOUNT: %d\n", gcmCOUNTER(pa_output_prim_counter));
                gcmPRINT("VPC_PADEPTHCLIPCOUNT: %d\n", gcmCOUNTER(pa_depth_clipped_counter));
                gcmPRINT("VPC_PATRIVIALREJCOUNT: %d\n", gcmCOUNTER(pa_trivial_rejected_counter));
                gcmPRINT("VPC_PACULLCOUNT: %d\n", gcmCOUNTER(pa_culled_counter));
                gcmPRINT("*********\n");

                gcmPRINT("VPG_SETUP\n");
                gcmPRINT("VPC_SETRIANGLECOUNT: %d\n", gcmCOUNTER(se_culled_triangle_count));
                gcmPRINT("VPC_SELINECOUNT: %d\n", gcmCOUNTER(se_culled_lines_count));
                gcmPRINT("*********\n");

                gcmPRINT("VPG_RA\n");
                gcmPRINT("VPC_RAVALIDPIXCOUNT: %d\n", gcmCOUNTER(ra_valid_pixel_count));
                gcmPRINT("VPC_RATOTALQUADCOUNT: %d\n", gcmCOUNTER(ra_total_quad_count));
                gcmPRINT("VPC_RAVALIDQUADCOUNTEZ: %d\n", gcmCOUNTER(ra_valid_quad_count_after_early_z));
                gcmPRINT("VPC_RATOTALPRIMCOUNT: %d\n", gcmCOUNTER(ra_total_primitive_count));
                gcmPRINT("VPC_RAPIPECACHEMISSCOUNT: %d\n", gcmCOUNTER(ra_pipe_cache_miss_counter));
                gcmPRINT("VPC_RAPREFCACHEMISSCOUNT: %d\n", gcmCOUNTER(ra_prefetch_cache_miss_counter));
                gcmPRINT("VPC_RAEEZCULLCOUNT: %d\n", gcmCOUNTER(ra_eez_culled_counter));
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
                gcmPRINT("*********\n");

                gcmPRINT("VPG_PS\n");
                gcmPRINT("VPC_PSINSTCOUNT: %d\n", gcmCOUNTER(ps_inst_counter) );
                gcmPRINT("VPC_PSBRANCHINSTCOUNT: %d\n", gcmCOUNTER(pxl_branch_inst_counter));
                gcmPRINT("VPC_PSTEXLDINSTCOUNT: %d\n", gcmCOUNTER(pxl_texld_inst_counter));
                gcmPRINT("VPC_PSRENDEREDPIXCOUNT: %d\n", gcmCOUNTER(rendered_pixel_counter));
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
                gcmPRINT("*********\n");;

                gcmPRINT("VPG_AXI\n");
                gcmPRINT("VPC_AXIREADREQSTALLED: %d\n", gcmCOUNTER(hi_axi_cycles_read_request_stalled));
                gcmPRINT("VPC_AXIWRITEREQSTALLED: %d\n", gcmCOUNTER(hi_axi_cycles_write_request_stalled));
                gcmPRINT("VPC_AXIWRITEDATASTALLED: %d\n", gcmCOUNTER(hi_axi_cycles_write_data_stalled));
                gcmPRINT("*********\n");

            }
#endif
        }
    }


    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

#if VIVANTE_PROFILER_CONTEXT
OnError:
    gcmFOOTER_NO();
    return status;
#endif
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
gcoPROFILER_EndDraw(
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
    if (_HAL->profiler.curBufId >= NumOfDrawBuf)
        return gcvSTATUS_NOT_SUPPORTED;

    gcmGETHARDWARE(hardware);
    if (hardware)
    {
        gctUINT32   address;
        gctUINT32   offset = 0;
        gctPOINTER  memory;
        gctUINT32 i;

        gcoBUFOBJ_Lock(_HAL->profiler.newCounterBuf[_HAL->profiler.curBufId], &address, &memory);
        PROBE_MODULE_COUNTERS(MODULE_FRONT_END, MODULE_FRONT_END_COUNTER_NUM);
        PROBE_MODULE_COUNTERS(MODULE_VERTEX_SHADER, MODULE_VERTEX_SHADER_COUNTER_NUM);
        PROBE_MODULE_COUNTERS(MODULE_PRIMITIVE_ASSEMBLY, MODULE_PRIMITIVE_ASSEMBLY_COUNTER_NUM);
        PROBE_MODULE_COUNTERS(MODULE_SETUP, MODULE_SETUP_COUNTER_NUM);
        PROBE_MODULE_COUNTERS(MODULE_RASTERIZER, MODULE_RASTERIZER_COUNTER_NUM);
        PROBE_MODULE_COUNTERS(MODULE_PIXEL_SHADER, MODULE_PIXEL_SHADER_COUNTER_NUM);
        PROBE_MODULE_COUNTERS(MODULE_TEXTURE, MODULE_TEXTURE_COUNTER_NUM);
        PROBE_MODULE_COUNTERS(MODULE_PIXEL_ENGINE, MODULE_PIXEL_ENGINE_COUNTER_NUM);
        PROBE_MODULE_COUNTERS(MODULE_MEMORY_CONTROLLER_COLOR, MODULE_MEMORY_CONTROLLER_COLOR_COUNTER_NUM);
        PROBE_MODULE_COUNTERS(MODULE_MEMORY_CONTROLLER_DEPTH, MODULE_MEMORY_CONTROLLER_DEPTH_COUNTER_NUM);
        PROBE_MODULE_COUNTERS(MODULE_HOST_INTERFACE0, MODULE_HOST_INTERFACE0_COUNTER_NUM);
        PROBE_MODULE_COUNTERS(MODULE_HOST_INTERFACE1, MODULE_HOST_INTERFACE1_COUNTER_NUM);
        PROBE_MODULE_COUNTERS(MODULE_GPUL2_CACHE, MODULE_GPUL2_CACHE_COUNTER_NUM);
        _HAL->profiler.curBufId++;
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

            gcmWRITE_CONST(VPG_HW);
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
            gcmWRITE_COUNTER(VPC_MCAXITOTALLATENCY, gcmCOUNTERCOMPARE(mc_axi_total_latency));
            gcmWRITE_COUNTER(VPC_MCAXISAMPLECOUNT, gcmCOUNTERCOMPARE(mc_axi_sample_count));
            gcmWRITE_CONST(VPG_END);

            gcmWRITE_CONST(VPG_AXI);
            gcmWRITE_COUNTER(VPC_AXIREADREQSTALLED, gcmCOUNTERCOMPARE(hi_axi_cycles_read_request_stalled));
            gcmWRITE_COUNTER(VPC_AXIWRITEREQSTALLED, gcmCOUNTERCOMPARE(hi_axi_cycles_write_request_stalled));
            gcmWRITE_COUNTER(VPC_AXIWRITEDATASTALLED, gcmCOUNTERCOMPARE(hi_axi_cycles_write_data_stalled));
            gcmWRITE_CONST(VPG_END);

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

/* Call to signal end of draw. */
gceSTATUS
gcoPROFILER_EndDraw(
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
