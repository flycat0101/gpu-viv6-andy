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


#include <gc_vx_common.h>
#include <gc_hal_user_precomp.h>
#include <gc_hal_vx.h>

VX_INTERNAL_CALLBACK_API void vxoProgram_Destructor(vx_reference ref)
{
    vx_program program = (vx_program)ref;

    if (program->buildOptions) gcoOS_Free(gcvNULL, program->buildOptions);
    if (program->buildLog) gcoOS_Free(gcvNULL, program->buildLog);
    if (program->source) gcoOS_Free(gcvNULL, program->source);
    if (program->binary) gcSHADER_Destroy((gcSHADER)program->binary);
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


    if (!vxoContext_IsValid(context)) return VX_NULL;

    program = (vx_program)vxoReference_Create(context, (vx_type_e)VX_TYPE_PROGRAM, VX_REF_EXTERNAL, &context->base);

    if (vxoReference_GetStatus((vx_reference)program) != VX_SUCCESS) return program;



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


    return program;

OnError:
    if (sizes) gcoOS_Free(gcvNULL, sizes);
    vxReleaseProgram(&program);

    return VX_NULL;
}

VX_API_ENTRY vx_program VX_API_CALL vxCreateProgramWithBinary(
        vx_context context, const vx_uint8 * binary, vx_size size)
{
    vx_program program;
    gceSTATUS   status;
    gcSHADER    shaderBinary;

    if (!vxoContext_IsValid(context)) return VX_NULL;

    program = (vx_program)vxoReference_Create(context, (vx_type_e)VX_TYPE_PROGRAM, VX_REF_EXTERNAL, &context->base);

    if (vxoReference_GetStatus((vx_reference)program) != VX_SUCCESS) return program;

    /* Construct binary. */
    gcmONERROR(gcSHADER_Construct(gcSHADER_TYPE_CL, &shaderBinary));

    /* Load binary */
    gcmONERROR(gcSHADER_LoadEx(shaderBinary, (gctPOINTER)binary, (gctUINT32)size));

    program->binary = (gctUINT8_PTR) shaderBinary;

    return program;

OnError:
    return VX_NULL;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseProgram(vx_program *program)
{
    return vxoReference_Release((vx_reference_ptr)program, (vx_type_e)VX_TYPE_PROGRAM, VX_REF_EXTERNAL);
}


VX_API_ENTRY vx_status VX_API_CALL vxBuildProgram(vx_program program, vx_const_string options)
{
    gctPOINTER          pointer;
    gctSIZE_T           length;
    gcSHADER            binary;
    gctUINT             binarySize;
    gceSTATUS           status = gcvSTATUS_OK;
    vx_status           vStatus = VX_FAILURE;

    if (!vxoReference_IsValidAndSpecific(&program->base, (vx_type_e)VX_TYPE_PROGRAM)) return VX_ERROR_INVALID_REFERENCE;

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

    if (program->binary == gcvNULL)
    {
        status = gcCompileKernel(gcvNULL,
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
            binary = gcvNULL;

            if (program->buildLog != gcvNULL)
            {
                fprintf(stderr, "%s\n", program->buildLog);
            }

            fprintf(stderr, "ERROR: Failed to compile vx shader. (error: %X)\n", status);
        }


    if(program != gcvNULL)
    {
        program->buildStatus = (vStatus == VX_SUCCESS) ? VX_BUILD_SUCCESS : VX_BUILD_ERROR;
    }

    return vStatus;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryProgram(vx_program program, vx_enum attribute, void *ptr, vx_size size)
{
    if (!vxoReference_IsValidAndSpecific(&program->base, (vx_type_e)VX_TYPE_PROGRAM)) return VX_ERROR_INVALID_REFERENCE;

    switch (attribute)
    {
        case VX_PROGRAM_ATTRIBUTE_BUILD_LOG:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_ptr, 0x3);
            *(vx_ptr *)ptr = program->buildLog;
            break;

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            return VX_ERROR_NOT_SUPPORTED;
    }

    return VX_SUCCESS;
}

