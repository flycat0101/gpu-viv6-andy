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


#include <gc_vx_common.h>
#include <gc_hal_user_precomp.h>
#include <gc_hal_vx.h>

#define _GC_OBJ_ZONE            gcdZONE_VX_PROGRAM

#if gcdSTATIC_LINK

gceSTATUS
gcCompileKernel(
    IN gcoHAL Hal,
    IN gctUINT SourceSize,
    IN gctCONST_STRING Source,
    IN gctCONST_STRING Options,
    OUT gcSHADER * Binary,
    OUT gctSTRING * Log
    );

gceSTATUS
gcLoadKernelCompiler(
    IN gcsHWCaps *HWCaps,
    IN gcePATCH_ID PatchId
    );

gceSTATUS
gcUnloadKernelCompiler(
void
);
#endif

static gceSTATUS _UpdateCompileOption(gctSTRING *options)
{
    gctSIZE_T extraOptionLength = 0;
    gctSIZE_T originalLengh = 0;
    gctSIZE_T totalLength;
    gceSTATUS status;
    gctPOINTER pointer = gcvNULL;

    gcmHEADER_ARG("options=%p", options);

    extraOptionLength = gcoOS_StrLen(" -cl-viv-gcsl-driver-image", gcvNULL);

    if (*options)
    {
        originalLengh = gcoOS_StrLen(*options, gcvNULL);
    }
    totalLength = originalLengh + extraOptionLength + 1;
    gcmONERROR(gcoOS_Allocate(gcvNULL, totalLength, &pointer));

    gcoOS_ZeroMemory(pointer, totalLength);

    if (*options)
    {
        gcmVERIFY_OK(gcoOS_StrCopySafe((gctSTRING)pointer, totalLength, *options));
        gcmOS_SAFE_FREE(gcvNULL, *options);
    }

    gcmASSERT(extraOptionLength);
    gcmVERIFY_OK(gcoOS_StrCatSafe((gctSTRING)pointer, totalLength, " -cl-viv-gcsl-driver-image"));


    *options = (gctSTRING)pointer;

OnError:
    gcmFOOTER_NO();
    return status;

}


VX_INTERNAL_CALLBACK_API void vxoProgram_Destructor(vx_reference ref)
{
    vx_program program = (vx_program)ref;

    if (program->buildOptions) gcoOS_Free(gcvNULL, program->buildOptions);
    if (program->buildLog) gcoOS_Free(gcvNULL, program->buildLog);
    if (program->source) gcoOS_Free(gcvNULL, program->source);
    if (program->linked)
    {
        if (program->binary) gcoOS_Free(gcvNULL, program->binary);
    }
    else
    {
        if (program->binary) gcSHADER_Destroy((gcSHADER)program->binary);
    }
}

VX_API_ENTRY vx_program VX_API_CALL vxCreateProgramWithSource(
        vx_context context, vx_uint32 count, const vx_char * strings[], vx_size lengths[])
{
    vx_program  program;
    gceSTATUS   status;
    gctPOINTER  pointer = gcvNULL;
    gctUINT *   sizes = gcvNULL;
    gctUINT     length;
    gctUINT     size = 0;
    gctSTRING   source;
    gctUINT     i;

    gcmHEADER_ARG("context=%p, count=0x%x, strings=%p, lengths=%p", context, count, strings, lengths);
    gcmDUMP_API("$VX vxCreateProgramWithSource: context=%p, count=0x%x, strings=%p, lengths=%p", context, count, strings, lengths);

    if (!vxoContext_IsValid(context))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    program = (vx_program)vxoReference_Create(context, (vx_type_e)VX_TYPE_PROGRAM, VX_REF_EXTERNAL, &context->base);

    if (vxoReference_GetStatus((vx_reference)program) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return program;
    }


    if (count == 0 || strings == gcvNULL)
    {
        goto OnError;
    }

    /* Allocate an array for lengths of strings. */
    gcmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctUINT) * count, &pointer));
    sizes = (gctUINT *) pointer;

    for (i = 0; i < count; i++)
    {
        if (strings[i] == gcvNULL)
        {
            goto OnError;
        }

        if (lengths == gcvNULL || lengths[i] == 0)
        {
            length = (gctUINT) gcoOS_StrLen(strings[i], gcvNULL);
            sizes[i] = length;
            size += length;
        }
        else
        {
            sizes[i] = (gctUINT)lengths[i];
            size += (gctINT)lengths[i];
        }
    }

    /* Allocate source. */
    gcmONERROR(gcoOS_Allocate(gcvNULL, size + 1, &pointer));
    source = (gctSTRING) pointer;

    program->source          = source;
    program->binarySize      = 0;
    program->binary          = gcvNULL;
    program->buildOptions    = gcvNULL;
    program->buildLog        = gcvNULL;
    program->buildStatus     = VX_BUILD_NONE;

    /* Copy source. */
    for (i = 0; i < count; i++)
    {
        if (sizes[i] > 0) {
            gcoOS_MemCopy(source, strings[i], sizes[i]);
            source += sizes[i];
        }
    }
    source[0] = '\0';


    gcoOS_Free(gcvNULL, sizes);

    gcmFOOTER_NO();
    return program;

OnError:
    if (sizes) gcoOS_Free(gcvNULL, sizes);
    vxReleaseProgram(&program);
    gcmFOOTER_NO();
    return VX_NULL;
}

VX_API_ENTRY vx_program VX_API_CALL vxCreateProgramWithBinary(
        vx_context context, const vx_uint8 * binary, vx_size size)
{
    vx_program program;
    gceSTATUS   status;
    gcSHADER    shaderBinary;
    gctUINT32   *pBinary = (gctUINT32*)binary;

    gcmHEADER_ARG("context=%p, binary=%p, size=0x%lx", context, binary, size);
    gcmDUMP_API("$VX vxCreateProgramWithBinary: context=%p, binary=%p, size=0x%lx", context, binary, size);

    if (!vxoContext_IsValid(context))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    program = (vx_program)vxoReference_Create(context, (vx_type_e)VX_TYPE_PROGRAM, VX_REF_EXTERNAL, &context->base);

    if (vxoReference_GetStatus((vx_reference)program) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return program;
    }
    gcQueryShaderCompilerHwCfg(gcvNULL, gcGetHWCaps());

    if ((*pBinary == FULL_PROGRAM_BINARY_SIG_1) &&  (*(pBinary+1) == FULL_PROGRAM_BINARY_SIG_2))
    {
        program->linked = gcvTRUE;

        gcmONERROR(gcoOS_Allocate(gcvNULL, size, (gctPOINTER*)&program->binary));
        gcoOS_MemCopy(program->binary, binary, size);
        program->binarySize = (gctUINT)size;
    }
    else
    {
        /* Construct binary. */
        gcmONERROR(gcSHADER_Construct(gcSHADER_TYPE_CL, &shaderBinary));

        /* Load binary */
        gcmONERROR(gcSHADER_LoadEx(shaderBinary, (gctPOINTER)binary, (gctUINT32)size));

        program->binary = (gctUINT8_PTR) shaderBinary;
        program->binarySize = (gctUINT)size;
    }
    gcmFOOTER_NO();
    return program;

OnError:
    vxReleaseProgram(&program);
    gcmFOOTER_NO();
    return VX_NULL;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseProgram(vx_program *program)
{
    gcmDUMP_API("$VX vxReleaseProgram: program=%p", program);
    return vxoReference_Release((vx_reference_ptr)program, (vx_type_e)VX_TYPE_PROGRAM, VX_REF_EXTERNAL);
}

static gceSTATUS _LoadCompiler(vx_context context)
{
    gceSTATUS status = gcvSTATUS_OK;
    VSC_HW_CONFIG hwCfg;
    gcePATCH_ID patchId;

    gcmHEADER();
    {
        if (context->compileKernel == gcvNULL)
        {
#if !gcdSTATIC_LINK
            status = gcoOS_LoadLibrary(gcvNULL,
#if defined(__APPLE__)
                                       "libCLC.dylib",
#elif  defined(_WIN32) || defined(_WIN32_WCE)
                                       "libCLC.dll",
#else
                                       "libCLC.so",
#endif
                                       &context->libCLC);

            if (gcmIS_ERROR(status))
            {
                goto OnError;
            }

             gcmONERROR(gcoOS_GetProcAddress(gcvNULL,
                                          context->libCLC,
                                          "gcCompileKernel",
                                          (gctPOINTER*)&context->compileKernel));

            gcmONERROR(gcoOS_GetProcAddress(gcvNULL,
                                          context->libCLC,
                                          "gcLoadKernelCompiler",
                                          (gctPOINTER*)&context->loadCompiler));

            gcmONERROR(gcoOS_GetProcAddress(gcvNULL,
                                          context->libCLC,
                                          "gcUnloadKernelCompiler",
                                          (gctPOINTER*)&context->unloadCompiler));
#else
            context->unloadCompiler = gcUnloadKernelCompiler;
            context->loadCompiler = gcLoadKernelCompiler;
            context->compileKernel = gcCompileKernel;
#endif
            gcmONERROR(gcQueryShaderCompilerHwCfg(gcvNULL, &hwCfg));
            gcmONERROR(gcoHAL_GetPatchID(gcvNULL, &patchId));

            gcmONERROR((*context->loadCompiler)(&hwCfg, patchId));
        }
    }

OnError:

    gcmFOOTER_NO();
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxBuildProgram(vx_program program, vx_const_string options)
{
    gctPOINTER          pointer;
    gctSIZE_T           length;
    gcSHADER            binary;
    gctUINT             binarySize;
    gceSTATUS           status = gcvSTATUS_OK;
    vx_status           vStatus = VX_FAILURE;

    gcmHEADER_ARG("program=%p, options=%s", program, options);
    gcmDUMP_API("$VX vxBuildProgram: program=%p, options=%s", program, options);

    if (!vxoReference_IsValidAndSpecific(&program->base, (vx_type_e)VX_TYPE_PROGRAM))
    {
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_REFERENCE;
    }
    if ((program->binary != gcvNULL) && (program->source != gcvNULL))
    {
        /* This program was built before
         * Clean up before building again
         */
        gcSHADER_Destroy((gcSHADER)program->binary);

        if (program->buildOptions) gcoOS_Free(gcvNULL, program->buildOptions);
        if (program->buildLog) gcoOS_Free(gcvNULL, program->buildLog);

        program->binary          = gcvNULL;
        program->buildOptions    = gcvNULL;
        program->buildLog        = gcvNULL;
        program->buildStatus     = VX_BUILD_NONE;
    }

    /* Copy build options */
    if (options)
    {
        length = gcoOS_StrLen(options, gcvNULL) + 1;
        gcmONERROR(gcoOS_Allocate(gcvNULL, length, &pointer));

        gcoOS_StrCopySafe((gctSTRING)pointer, length, options);
        program->buildOptions = (gctSTRING) pointer;
    }
    else
    {
        program->buildOptions = gcvNULL;
    }

    program->buildStatus = VX_BUILD_IN_PROGRESS;

    if (program->binary == gcvNULL || !program->linked)
    {
        gcmONERROR(_LoadCompiler(program->base.context));
    }

    if (program->binary == gcvNULL)
    {

        gcmONERROR(_UpdateCompileOption(&program->buildOptions));
        vscSetDriverVIRPath(gcvFALSE);  /* change to true if vx driver changed to program with VIR shader */

        status = (*program->base.context->compileKernel) (
                                        gcvNULL,
                                        0,
                                        program->source,
                                        program->buildOptions,
                                        &binary,
                                        &program->buildLog);
        if (gcmIS_ERROR(status))
        {
            goto OnError;
        }

        program->binary = (unsigned char *) binary;
        gcmONERROR(gcSHADER_SaveEx(binary, gcvNULL, &binarySize));
        program->binarySize = binarySize;
    }

    vStatus = VX_SUCCESS;

OnError:
    if (gcmIS_ERROR(status))
    {
        if (program->buildLog != gcvNULL)
        {
            fprintf(stderr, "%s\n", program->buildLog);
        }

        fprintf(stderr, "ERROR: Failed to compile vx shader. (error: %X)\n", status);
    }


    gcmASSERT(program != gcvNULL);

    {
        program->buildStatus = (vStatus == VX_SUCCESS) ? VX_BUILD_SUCCESS : VX_BUILD_ERROR;
    }

    gcmFOOTER_NO();
    return vStatus;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryProgram(vx_program program, vx_enum attribute, void *ptr, vx_size size)
{
    gcmHEADER_ARG("program=%p, attribute=0x%x, ptr=%p, size=0x%lx", program, attribute, ptr, size);
    gcmDUMP_API("$VX vxQueryProgram: program=%p, attribute=0x%x, ptr=%p, size=0x%lx", program, attribute, ptr, size);

    if (!vxoReference_IsValidAndSpecific(&program->base, (vx_type_e)VX_TYPE_PROGRAM))
    {
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_REFERENCE;
    }
    switch (attribute)
    {
        case VX_PROGRAM_ATTRIBUTE_BUILD_LOG:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_ptr, 0x3);
            *(vx_ptr *)ptr = program->buildLog;
            break;

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            gcmFOOTER_NO();
            return VX_ERROR_NOT_SUPPORTED;
    }

    gcmFOOTER_NO();
    return VX_SUCCESS;
}

