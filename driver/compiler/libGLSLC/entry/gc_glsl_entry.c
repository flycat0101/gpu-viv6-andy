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


#include "gc_glsl_compiler.h"

/*******************************************************************************
***** Version Signature *******************************************************/

#define _gcmTXT2STR(t) #t
#define gcmTXT2STR(t) _gcmTXT2STR(t)
const char * _GLESV2SC_VERSION = "\n\0$VERSION$"
                                 gcmTXT2STR(gcvVERSION_MAJOR) "."
                                 gcmTXT2STR(gcvVERSION_MINOR) "."
                                 gcmTXT2STR(gcvVERSION_PATCH) ":"
                                 gcmTXT2STR(gcvVERSION_BUILD)
                                 "$\n";

#define gcdUSE_PRECOMPILED_SHADERS     1

#if gcdUSE_PRECOMPILED_SHADERS
typedef struct _lookup
{
    const gctUINT sourceSize;
    const char * source1;
    const char * source2;
    gceSTATUS (*function)(gcSHADER Shader);
}
lookup;

#include "gc_glsl_util.h"
#include "gc_glsl_precompiled_shaders.h"
#endif

#define gcdDUMP_SHADER  0  /* use VC_DUMP_SHADER_SOURCE=1 env variable to dump shader source */

gctCONST_STRING
gcmShaderName(
    IN gctINT ShaderKind
    )
{
    gcmASSERT(ShaderKind >= gcSHADER_TYPE_UNKNOWN &&  ShaderKind < gcSHADER_KIND_COUNT);
    return gcSL_GetShaderKindString(ShaderKind);
}

/* Get the compiler options */
sltOPTIMIZATION_OPTIONS sloCOMPILER_GetOptions(sloCOMPILER Compiler)
{
    sltOPTIMIZATION_OPTIONS     opt = slvOPTIMIZATION_ALL;
    gcOPTIMIZER_OPTION *        optimizer_opt = gcGetOptimizerOption();
    sleSHADER_TYPE              shaderType = slvSHADER_TYPE_VERTEX;

    sloCOMPILER_GetShaderType(Compiler, &shaderType);

    if ((optimizer_opt->optFlags & gcvOPTIMIZATION_POWER_OPTIMIZATION) != 0 &&
         optimizer_opt->splitVec == gcvTRUE)
    {
        opt |= slvOPTIMIZATION_EXPAND_NORM;
    }

    if (shaderType == slvSHADER_TYPE_LIBRARY)
    {
        opt &= ~slvOPTIMIZATION_SHARE_VEC_CONSTANTS;
    }

    return opt;
}

gctBOOL sloCOMPILER_ExpandNorm(sloCOMPILER Compiler)
{
    return (sloCOMPILER_GetOptions(Compiler) & slvOPTIMIZATION_EXPAND_NORM) != 0;
}

gctINT
_convertShaderType(
    IN gctINT ShaderType
    )
{
    if (ShaderType == gcSHADER_TYPE_VERTEX_DEFAULT_UBO)
        return gcSHADER_TYPE_VERTEX;

    if (ShaderType == gcSHADER_TYPE_FRAGMENT_DEFAULT_UBO)
        return gcSHADER_TYPE_FRAGMENT;

    return ShaderType;
}

/*******************************************************************************
**                              gcCompileShader
********************************************************************************
**
**  Compile a shader.
**
**  INPUT:
**
**      gcoOS Hal
**          Pointer to an gcoHAL object.
**
**      gctINT ShaderType
**          Shader type to compile.  Can be one of the following values:
**
**              gcSHADER_TYPE_VERTEX
**                  Compile a vertex shader.
**
**              gcSHADER_TYPE_FRAGMENT
**                  Compile a fragment shader.
**
**              gcSHADER_TYPE_COMPUTE
**                  Compile a compute shader.
**
**        gcSHADER_TYPE_PRECOMPILED
**                  Precompiled shader
**
**      gctUINT SourceSize
**          Size of the source buffer in bytes.
**
**      gctCONST_STRING Source
**          Pointer to the buffer containing the shader source code.
**
**  OUTPUT:
**
**      gcSHADER * Binary
**          Pointer to a variable receiving the pointer to a gcSHADER object
**          containg the compiled shader code.
**
**      gctSTRING * Log
**          Pointer to a variable receiving a string pointer containging the
**          compile log.
*/
gceSTATUS
gcCompileShader(
    IN gcoHAL Hal,
    IN gctINT ShaderType,
    IN gctUINT SourceSize,
    IN gctCONST_STRING Source,
    OUT gcSHADER * Binary,
    OUT gctSTRING * Log
    )
{
    gceSTATUS status;
    sloCOMPILER compiler = gcvNULL;
    gctSIZE_T theSourceSize = SourceSize;
    gctCONST_STRING theSource = Source;
    gctINT shaderId = (gctINT)gcSHADER_NextId();
    gctUINT stringId = 0;

    gctBOOL dumpShader =
#if gcdDUMP_SHADER
                                 gcvTRUE;
#else
                                 gcvFALSE;
#endif
    static gctBOOL firstTime = 1;
    gceAPI apiVersion = gcvAPI_OPENGL_ES20;
    /* fake a shader to have desired shaderId */
    struct _gcSHADER shader_;

    gcmHEADER_ARG("Hal=0x%x ShaderType=%d SourceSize=%lu Source=0x%x",
                  Hal, ShaderType, SourceSize, Source);

    shader_.object.type = gcvOBJ_SHADER;
    shader_._id = shaderId;

    if (ShaderType != gcSHADER_TYPE_PRECOMPILED)
    {
        stringId = gcEvaluateCRC32ForShaderString(Source, (gctUINT32)SourceSize);
    }
    {
        /* Get the current client API type. */
        gco3D engine;
        status = gco3D_Get3DEngine(&engine);
        if (status == gcvSTATUS_OK)
        {
            gcmONERROR(gco3D_GetAPI(engine, &apiVersion));
        }
    }
    if (*Binary)
    {
        gcmONERROR(gcSHADER_Destroy(*Binary));
    }

    if (firstTime)
    {
        /* check environment variable VC_DUMP_SHADER_SOURCE */
        char* p = gcvNULL;
        gcoOS_GetEnv(gcvNULL, "VC_DUMP_SHADER_SOURCE", &p);
        if (p)
        {
            if (gcmIS_SUCCESS(gcoOS_StrCmp(p, "1")) ||
                gcmIS_SUCCESS(gcoOS_StrCmp(p, "on")) ||
                gcmIS_SUCCESS(gcoOS_StrCmp(p, "ON")) )
            {
                gcmOPT_SET_DUMP_SHADER_SRC(gcvTRUE);
            }
        }
        firstTime = 0;
    }

    /* check if the current shader source is replaced by
       VC_OPTION=-SHADER option
    */
    if (gcmOPT_ShaderSourceList() != gcvNULL &&
        ShaderType != gcSHADER_TYPE_PRECOMPILED)
    {
        ShaderSourceList * srcList  = gcmOPT_ShaderSourceList();
        /* find if the shader id is in the list */
        while (srcList != gcvNULL)
        {
            if (srcList->shaderId == shaderId)
            {
                theSource     = srcList->src;
                theSourceSize = srcList->sourceSize;
                break;
            }
            srcList = srcList->next;
        }
    }
    if (dumpShader || gcSHADER_DumpSource(&shader_))
    {
        gctCHAR buffer[512];
        gctUINT offset = 0;
        gctSIZE_T n;
        gcoOS_Print("===== [ Incoming %s shader source (id:%d) %s] =====",
                    gcmShaderName(_convertShaderType(ShaderType)), shaderId,
                    (theSource != Source) ? "(replaced)" : "");
        for (n = 0; n < theSourceSize; ++n)
        {
            if ((theSource[n] == '\n')
            ||  (offset == gcmSIZEOF(buffer) - 2)
            )
            {
                if (theSource[n] != '\n')
                {
                    /* take care the last char */
                    buffer[offset++] = theSource[n];
                }
                buffer[offset] = '\0';
                gcoOS_Print("%s", buffer);
                offset = 0;
            }
            else
            {
                /* don't print \r, which may be from a DOS format file */
                if (theSource[n] != '\r')
                    buffer[offset++] = theSource[n];
            }
        }
        if (offset != 0)
        {
            buffer[offset] = '\0';
            gcoOS_Print("%s", buffer);
        }
    }

    if(ShaderType == gcSHADER_TYPE_PRECOMPILED)
    {
        /* Construct a new gcSHADER object. */
        gcmONERROR(gcSHADER_Construct(Hal,
                                      ShaderType,
                                      Binary));

        /* A precompiled shader */
        gcmONERROR(gcSHADER_SetCompilerVersion(*Binary,
                                               sloCOMPILER_GetVersion(gcvNULL, ShaderType)));

        gcmONERROR(gcSHADER_SetClientApiVersion(*Binary, apiVersion));

        gcmASSERT(theSource == Source);
        gcmONERROR(gcSHADER_Load(*Binary,
                                 (gctPOINTER) Source,
                                 SourceSize));
    }
    else
    {
#if gcdUSE_PRECOMPILED_SHADERS
        if (gcSHADER_DoPatch(&shader_) && theSourceSize >= 604)
        {
            const char *p, *q;
            lookup * lookup;
            gctBOOL useSource1;

            /* Look up any hand compiled shader. */
            for (lookup = compiledShaders; lookup->function != gcvNULL; ++lookup)
            {
                if (theSourceSize < lookup->sourceSize)
                {
                    continue;
                }

                p = theSource;
                q = lookup->source1;
                useSource1 = gcvTRUE;

                while (*p)
                {
                    if (*p == *q)
                    {
                        p++;
                        q++;
                    }
                    else if (*p == ' ' || *p == '\r' || *p == '\n' || *p == '\t')
                    {
                        p++;
                    }
                    else if (*q == ' ' || *q == '\r' || *q == '\n' || *q == '\t')
                    {
                        q++;
                    }
                    else if (*q == '\0' && useSource1 && lookup->source2)
                    {
                        q = lookup->source2;
                        useSource1 = gcvFALSE;
                    }
                    else
                    {
                        break;
                    }
                }

                if (*p == '\0' && *q == '\0')
                {
                    gctUINT32 compilerVersion[2];

                    /* Construct a new gcSHADER object. */
                    gcmONERROR(gcSHADER_Construct(Hal,
                                                  _convertShaderType(ShaderType),
                                                  Binary));

                    compilerVersion[0] = gcmCC('E', 'S', '\0', '\0') | (_convertShaderType(ShaderType) << 16);
                    compilerVersion[1] = gcmCC('\0', '\0', '\1', '\1');

                    /* Set GLSL version. */
                    gcmONERROR(gcSHADER_SetCompilerVersion(*Binary, compilerVersion));

                    gcmONERROR(gcSHADER_SetClientApiVersion(*Binary, apiVersion));

                    gcmONERROR(gcSHADER_SetShaderID(*Binary, stringId));

                    /* Create the shader dynamically. */
                    gcmONERROR((*lookup->function)(*Binary));

                    /* Success. */
                    gcmFOOTER_ARG("*Binary=0x%x *Log=0x%x", *Binary, *Log);
                    return gcvSTATUS_OK;
                }
            }
        }
#endif /* gcdUSE_PRECOMPILED_SHADERS */
        {
            gcmONERROR(sloCOMPILER_Construct(Hal,
                                             (sleSHADER_TYPE) ShaderType,
                                             apiVersion,
                                             &compiler));

            gcmONERROR(sloCOMPILER_Compile(compiler,
                                           sloCOMPILER_GetOptions(compiler),
                                           slmDUMP_OPTIONS,
                                           1,
                                           &theSource,
                                           Binary,
                                           Log));

            gcmONERROR(gcSHADER_SetShaderID(*Binary, stringId));

            gcmONERROR(sloCOMPILER_Destroy(compiler));
        }
    }

    /* Success. */
    gcmFOOTER_ARG("*Binary=0x%x *Log=0x%x", *Binary, *Log);
    return gcvSTATUS_OK;

OnError:

    if (gcmOPT_DUMP_FELOG() && *Log != gcvNULL && (*Log)[0] != '\0')
    {
        if (gcoOS_StrLen(*Log, gcvNULL) > 2047)
        {
            gctSTRING log = gcvNULL;
            gcmVERIFY_OK(gcoOS_Allocate(gcvNULL, 2048, (gctPOINTER *)&log));
            gcoOS_StrCopySafe(log, 2047, *Log);
            log[2047] = '\0';
            gcoOS_Print("%s", log);
            gcmOS_SAFE_FREE(gcvNULL, log);
        }
        else
        {
            gcoOS_Print("%s", *Log);
        }
    }

    if (compiler != gcvNULL)
    {
        /* Delete sloCOMPILER object. */
       gcmVERIFY_OK(sloCOMPILER_Destroy(compiler));
    }

    if (*Binary)
    {
        /* Destroy bad binary. */
        gcmVERIFY_OK(gcSHADER_Destroy(*Binary));
        *Binary = gcvNULL;
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
}


static gcsATOM_PTR  CompilerLockRef = gcvNULL;
gctPOINTER   CompilerLock = gcvNULL;

/*******************************************************************************************
**  Initialize compiler global variables.
**
**  Input:
**      gcoHAL Hal
**      Pointer to HAL object
**
**      gcsGLSLCaps *Caps
**      Min/Max capabilities filled in by driver and passed to compiler
**
**  Output:
**      Nothing
**
*/
gceSTATUS
gcInitializeCompiler(
    IN gcoHAL Hal,
    IN gcsGLSLCaps *Caps
    )
{
    gctINT32 reference;
    gceSTATUS status;

    gcmHEADER_ARG("Hal=0x%x", Hal);

    if (CompilerLockRef == gcvNULL)
    {
        /* Create a new reference counter. */
        gcmONERROR(gcoOS_AtomConstruct(gcvNULL, &CompilerLockRef));
    }

    /* Increment the reference counter */
    gcmONERROR(gcoOS_AtomIncrement(gcvNULL, CompilerLockRef, &reference));

    if (reference == 0)
    {
        /* Create a global lock. */
        status = gcoOS_CreateMutex(gcvNULL, &CompilerLock);

        if (gcmIS_ERROR(status))
        {
            CompilerLock = gcvNULL;
        }
    }

    if(Caps)
    {
        *gcGetGLSLCaps() = *Caps;
    }
    else
    {
        gcmVERIFY_OK(gcInitGLSLCaps(Hal, gcGetGLSLCaps()));
    }

OnError:
    gcmFOOTER_ARG("status=%d", status);
    return status;

}

/*******************************************************************************************
**  Finalize compiler global variables.
**
**  Input:
**      gcoHAL Hal
**      Pointer to HAL object
**
**  Output:
**      Nothing
**
*/
gceSTATUS
gcFinalizeCompiler(
    IN gcoHAL Hal
    )
{
    gctINT32 reference = 0;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hal=0x%x", Hal);

    /* CompilerLockRef could be NULL when Construction failed. */
    if(CompilerLockRef != gcvNULL)
    {
        /* Decrement the reference counter */
        gcmVERIFY_OK(gcoOS_AtomDecrement(gcvNULL, CompilerLockRef, &reference));
    }

    if (reference == 1)
    {
        /* Delete the global lock */
        gcmVERIFY_OK(gcoOS_DeleteMutex(gcvNULL, CompilerLock));

        /* Destroy the reference counter */
        gcmVERIFY_OK(gcoOS_AtomDestroy(gcvNULL, CompilerLockRef));

        CompilerLockRef = gcvNULL;
    }

    gcmFOOTER_ARG("status=%d", status);
    return status;
}



