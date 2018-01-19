/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
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

gctCONST_STRING
gcmShaderName(
    IN gctINT ShaderKind
    )
{
    gcmASSERT(ShaderKind >= gcSHADER_TYPE_UNKNOWN &&  ShaderKind < gcSHADER_KIND_COUNT);
    return gcSL_GetShaderKindString(ShaderKind);
}

/* Get the compiler options */
sltOPTIMIZATION_OPTIONS _GetOptions(sleSHADER_TYPE shaderType)
{
    sltOPTIMIZATION_OPTIONS     opt = slvOPTIMIZATION_ALL;
    gcOPTIMIZER_OPTION *        optimizer_opt = gcGetOptimizerOption();

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

#define DUMP_BUFFER_SIZE    512

static void
_DumpShaderSource(
    IN gcSHADER Shader,
    IN gctCONST_STRING Source
    )
{
    if (gcSHADER_DumpSource(Shader))
    {
        gctCHAR *buffer;
        gctUINT offset = 0;
        gctSIZE_T n;
        gcoOS_Print("===== [ Incoming %s shader source (id:%d) %s] =====",
                    gcmShaderName(_convertShaderType(Shader->type)), Shader->_id,
                    (Source != Shader->source) ? "(replaced)" : "");

        gcoOS_Allocate(gcvNULL, DUMP_BUFFER_SIZE, (gctPOINTER *)&buffer) ;

        for (n = 0; n < Shader->sourceLength ; ++n)
        {
            if ((Shader->source[n] == '\n')
            ||  (offset == DUMP_BUFFER_SIZE - 2)
            )
            {
                if (Shader->source[n] != '\n')
                {
                    /* take care the last char */
                    buffer[offset++] = Shader->source[n];
                }
                buffer[offset] = '\0';
                gcoOS_Print("%s", buffer);
                offset = 0;
            }
            else
            {
                /* don't print \r, which may be from a DOS format file */
                if (Shader->source[n] != '\r')
                    buffer[offset++] = Shader->source[n];
            }
        }
        if (offset != 0)
        {
            buffer[offset] = '\0';
            gcoOS_Print("%s", buffer);
        }

        gcoOS_Free(gcvNULL, buffer);
    }
}

#if gcdUSE_PRECOMPILED_SHADERS
static gceSTATUS
_UsePreCompiledShader(
    IN gcSHADER Shader,
    OUT gcSHADER * Binary,
    OUT gctSTRING * Log
)
{
    gceSTATUS status;

    if (gcSHADER_DoPatch(Shader) && Shader->sourceLength >= 604)
    {
        const char *p, *q;
        lookup * lookup;
        gctBOOL useSource1;

        /* Look up any hand compiled shader. */
        for (lookup = compiledShaders; lookup->function != gcvNULL; ++lookup)
        {
            if (Shader->sourceLength < lookup->sourceSize)
            {
                continue;
            }

            p = Shader->source;
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
                gcmONERROR(gcSHADER_Construct(_convertShaderType(Shader->type),
                                              Binary));

                compilerVersion[0] = _SHADER_GL_LANGUAGE_TYPE | (_convertShaderType(Shader->type) << 16);
                compilerVersion[1] = _SHADER_ES11_VERSION;

                /* Set GLSL version. */
                gcmONERROR(gcSHADER_SetCompilerVersion(*Binary, compilerVersion));

                gcmONERROR(gcSHADER_SetClientApiVersion(*Binary, Shader->clientApiVersion));

                gcmONERROR(gcSHADER_SetShaderID(*Binary, Shader->_stringId));

                /* Create the shader dynamically. */
                gcmONERROR((*lookup->function)(*Binary));

                return gcvSTATUS_TRUE;
            }
        }
    }

    return gcvSTATUS_FALSE;

OnError:
    return status;
}
#endif /* gcdUSE_PRECOMPILED_SHADERS */

void _ReplaceShaderSource(gcSHADER Shader)
{
    /* check if the current shader source is replaced by
       VC_OPTION=-SHADER option
    */
    if (gcmOPT_ShaderSourceList() != gcvNULL &&
        Shader->type != gcSHADER_TYPE_PRECOMPILED)
    {
        ShaderSourceList * srcList  = gcmOPT_ShaderSourceList();
        /* find if the shader id is in the list */
        while (srcList != gcvNULL)
        {
            if ((gctUINT)srcList->shaderId == Shader->_id)
            {
                Shader->source       = srcList->src;
                Shader->sourceLength = srcList->sourceSize;
                break;
            }
            srcList = srcList->next;
        }
    }
}

static gceSTATUS _loadPreCompiledShader(
    IN gcSHADER Shader,
    OUT gcSHADER * Binary
)
{
    gceSTATUS status;

    /* Construct a new gcSHADER object. */
    gcmONERROR(gcSHADER_Construct(Shader->type,
                                  Binary));

    /* A precompiled shader */
    gcmONERROR(gcSHADER_SetCompilerVersion(*Binary, sloCOMPILER_GetVersion(gcvNULL, (sleSHADER_TYPE)Shader->type)));

    gcmONERROR(gcSHADER_SetClientApiVersion(*Binary, Shader->clientApiVersion));

    gcmONERROR(gcSHADER_Load(*Binary,
                             (gctPOINTER) Shader->source,
                             Shader->sourceLength));

OnError:
        return status;
}

static gctUINT _convertShaderTypeToGcCompilerIndex(
    sleSHADER_TYPE ShaderType
    )
{
    gctUINT compilerIndex = 0;

    switch (ShaderType)
    {
    case slvSHADER_TYPE_VERTEX:
    case slvSHADER_TYPE_PRECOMPILED:
    case slvSHADER_TYPE_VERTEX_DEFAULT_UBO:
        compilerIndex = 0;
        break;

    case slvSHADER_TYPE_FRAGMENT:
    case slvSHADER_TYPE_FRAGMENT_DEFAULT_UBO:
        compilerIndex = 1;
        break;

    case slvSHADER_TYPE_COMPUTE:
        compilerIndex = 2;
        break;

    case slvSHADER_TYPE_TCS:
        compilerIndex = 3;
        break;

    case slvSHADER_TYPE_TES:
        compilerIndex = 4;
        break;

    case slvSHADER_TYPE_GS:
        compilerIndex = 5;
        break;

    case slvSHADER_TYPE_LIBRARY:
        compilerIndex = 6;
        break;

    default:
        gcmASSERT(gcvFALSE);
        compilerIndex = 0;
        break;
    }

    return compilerIndex;
}

struct gcsATOM CompilerLockRef = gcmATOM_INITIALIZER;

static gceSTATUS
_LockCompiler(void)
{
    gceSTATUS status;

    gcmHEADER();

    status = gcoOS_LockGLFECompiler();

    gcmFOOTER();
    return status;
}

static gceSTATUS
_UnLockCompiler(void)
{
    gceSTATUS status;

    gcmHEADER();

    status = gcoOS_UnLockGLFECompiler();

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**                              gcCompileShader
********************************************************************************
**
**  Compile a shader.
**
**  INPUT:
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
    IN gctINT ShaderType,
    IN gctUINT SourceSize,
    IN gctCONST_STRING Source,
    OUT gcSHADER * Binary,
    OUT gctSTRING * Log
    )
{
    gceSTATUS status;
    sloCOMPILER compiler = gcvNULL;
    struct _gcSHADER shader_;
    gco3D engine;
    gctBOOL locked = gcvFALSE;

    gcmHEADER_ARG("ShaderType=%d SourceSize=%lu Source=0x%x",
                  ShaderType, SourceSize, Source);

    shader_.object.type = gcvOBJ_SHADER;
    shader_._id = gcSHADER_NextId();
    shader_.type = ShaderType;
    shader_.source = (gctSTRING)Source;
    shader_.sourceLength = SourceSize;

    if (ShaderType != gcSHADER_TYPE_PRECOMPILED)
    {
        shader_._stringId = gcEvaluateCRC32ForShaderString(Source, (gctUINT32)SourceSize);
    }
    else
    {
        shader_._stringId = 0;
    }

    status = gco3D_Get3DEngine(&engine);
    if (status == gcvSTATUS_OK)
    {
        gcmONERROR(gco3D_GetAPI(engine, &shader_.clientApiVersion));
    }
    else
    {
        shader_.clientApiVersion = gcvAPI_OPENGL_ES20;
    }

    if (*Binary)
    {
        gcmONERROR(gcSHADER_Destroy(*Binary));
        *Binary = gcvNULL;
    }

    _ReplaceShaderSource(&shader_);

    _DumpShaderSource(&shader_, Source);

    if(ShaderType == gcSHADER_TYPE_PRECOMPILED)
    {
        gcmONERROR(_loadPreCompiledShader(&shader_, Binary));
    }
    else
    {
#if gcdUSE_PRECOMPILED_SHADERS
        gcmONERROR(_UsePreCompiledShader(&shader_, Binary, Log));

        if (status == gcvSTATUS_TRUE)
        {
            /* Success. */
            gcmFOOTER_ARG("*Binary=0x%x *Log=0x%x", *Binary, *Log);
            return gcvSTATUS_OK;
        }
#endif
        {
            sltDUMP_OPTIONS dumpOption = slmDUMP_OPTIONS;
            char* p = gcvNULL;
            sleSHADER_TYPE shaderType = slvSHADER_TYPE_VERTEX;
            sltOPTIMIZATION_OPTIONS optOption;
            gctUINT compilerIndex = _convertShaderTypeToGcCompilerIndex((sleSHADER_TYPE)ShaderType);

            gcmONERROR(_LockCompiler());
            locked = gcvTRUE;

            compiler = *gcGetCompiler(compilerIndex);

            gcoOS_GetEnv(gcvNULL, "VIV_GLSL_DUMP", &p);

            if (compiler == gcvNULL)
            {
                sloCOMPILER_Construct_General((sleSHADER_TYPE)ShaderType,
                                              gcGetCompiler(compilerIndex));
                compiler = *gcGetCompiler(compilerIndex);
            }

            gcmONERROR(sloCOMPILER_Construct((sleSHADER_TYPE)ShaderType,
                                             shader_.clientApiVersion,
                                             compiler));

            sloCOMPILER_GetShaderType(compiler, &shaderType);

            optOption = _GetOptions(shaderType);

            if ((p && p[0] == '1') && (shaderType != slvSHADER_TYPE_LIBRARY))
            {
                dumpOption = slvDUMP_ALL;
                optOption &=~slvOPTIMIZATION_DATA_FLOW;
                dumpOption &=~slvDUMP_CODE_GENERATOR;
            }

            gcmONERROR(sloCOMPILER_Compile(compiler,
                                           optOption,
                                           dumpOption,
                                           1,
                                           (gctCONST_STRING *)(&shader_.source),
                                           Binary,
                                           Log));

            gcmONERROR(gcSHADER_SetShaderID(*Binary, shader_._stringId));

            gcmONERROR(sloCOMPILER_Destroy(compiler));

            locked = gcvFALSE;
            gcmONERROR(_UnLockCompiler());
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

    if (locked)
    {
        gcmVERIFY_OK(_UnLockCompiler());
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************************
**  Initialize compiler global variables.
**
**  Input:
**      gcePATCH_ID PatchId,
**      patch ID
**
**      gcsHWCaps HWCaps,
**      HW capabilities filled in by driver and passed to compiler
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
    IN gcePATCH_ID PatchId,
    IN gcsHWCaps *HWCaps,
    IN gcsGLSLCaps *Caps
    )
{
    gctINT32 reference;
    gceSTATUS status;
    gcsGLSLCaps *glslCaps = gcGetGLSLCaps();

    gcmHEADER_ARG("Hal=0x%x", Hal);

    /* Increment the reference counter */
    gcmONERROR(gcoOS_AtomIncrement(gcvNULL, &CompilerLockRef, &reference));

    *gcGetPatchId() = PatchId;

    if (HWCaps)
    {
        *gcGetHWCaps() = *HWCaps;
    }
    else
    {
        gcQueryShaderCompilerHwCfg(gcvNULL, gcGetHWCaps());
    }

    if (Caps)
    {
        *glslCaps = *Caps;

        if (glslCaps->extensions == gcvNULL)
        {
            glslCaps->extensions = __DEFAULT_GLSL_EXTENSION_STRING__;
        }
    }
    else
    {
        gcmVERIFY_OK(gcInitGLSLCaps(glslCaps));
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
gcFinalizeCompiler(void)
{
    gctINT32 reference = 0;
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT i;

    gcmHEADER();

    /* Decrement the reference counter */
    gcmVERIFY_OK(gcoOS_AtomDecrement(gcvNULL, &CompilerLockRef, &reference));

    if (reference == 1)
    {
        for (i = 0; i < __GC_COMPILER_NUMBER__; i++)
        {
            if (*gcGetCompiler(i) != gcvNULL)
            {
                sloCOMPILER_Destroy_General(*gcGetCompiler(i));
                *gcGetCompiler(i) = gcvNULL;
            }
        }
    }

    gcmFOOTER_ARG("status=%d", status);
    return status;
}



