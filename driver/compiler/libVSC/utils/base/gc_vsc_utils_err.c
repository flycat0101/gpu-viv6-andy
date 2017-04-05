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


#include "gc_vsc.h"
#include <stdio.h>

#define VSC_MAX_ERR_WARNING_MSG_SIZE    512

#define PrintMsg        gcoOS_Print

void vscERR_ReportError(
    const char* file,
    gctUINT line,
    VSC_ErrCode errCode,
    const char* format,
    ...
    )
{
    char message[VSC_MAX_ERR_WARNING_MSG_SIZE];
    va_list arguments;

    gcmASSERT(format != gcvNULL);

    /* Get the pointer to the variable arguments. */
    va_start(arguments, format);

#if defined(_WINDOWS) || (defined(_WIN32) || defined(WIN32))
    /* Format the string. */
    vsprintf_s(message, VSC_MAX_ERR_WARNING_MSG_SIZE, format, arguments);
#else
    vsnprintf(message, VSC_MAX_ERR_WARNING_MSG_SIZE, format, arguments);
#endif

    PrintMsg("%s:%d Error %d: %s\n", file, line, (gctUINT)errCode, message);

    /* Delete the pointer to the variable arguments. */
    va_end(arguments);
}

void vscERR_ReportWarning(
    const char* file,
    gctUINT line,
    VSC_ErrCode errCode,
    const char* format,
    ...
    )
{
    char message[VSC_MAX_ERR_WARNING_MSG_SIZE];
    va_list arguments;

    gcmASSERT(format != gcvNULL);

    /* Get the pointer to the variable arguments. */
    va_start(arguments, format);

#if defined(_WINDOWS) || (defined(_WIN32) || defined(WIN32))
    /* Format the string. */
    vsprintf_s(message, VSC_MAX_ERR_WARNING_MSG_SIZE, format, arguments);
#else
    vsnprintf(message, VSC_MAX_ERR_WARNING_MSG_SIZE, format, arguments);
#endif

    PrintMsg("%s:%d Warning %d: %s\n", file, line, (gctUINT)errCode, message);

    /* Delete the pointer to the variable arguments. */
    va_end(arguments);
}

gceSTATUS vscERR_CastErrCode2GcStatus(VSC_ErrCode errCode)
{
    switch (errCode)
    {
    case VSC_ERR_NONE:
        return gcvSTATUS_OK;

    /* General errors */
    case VSC_ERR_INVALID_ARGUMENT:
    case VSC_ERR_REDEFINITION:
    case VSC_ERR_CG_NOT_BUILT:
        return gcvSTATUS_INVALID_ARGUMENT;
    case VSC_ERR_NOT_SUPPORTED:
        return gcvSTATUS_NOT_SUPPORTED;
    case VSC_ERR_INVALID_DATA:
        return gcvSTATUS_INVALID_DATA;
    case VSC_ERR_OUT_OF_MEMORY:
        return gcvSTATUS_OUT_OF_MEMORY;
    case VSC_ERR_OUT_OF_RESOURCE:
        return gcvSTATUS_OUT_OF_RESOURCES;
    case VSC_ERR_OUT_OF_SAMPLER:
        return gcvSTATUS_OUT_OF_SAMPLER;
    case VSC_ERR_VERSION_MISMATCH:
        return gcvSTATUS_VERSION_MISMATCH;

    /* Link errors */
    case VSC_ERR_GLOBAL_TYPE_MISMATCH:
        return gcvSTATUS_GLOBAL_TYPE_MISMATCH;
    case VSC_ERR_TOO_MANY_ATTRIBUTES:
        return gcvSTATUS_TOO_MANY_ATTRIBUTES;
    case VSC_ERR_TOO_MANY_VARYINGS:
        return gcvSTATUS_TOO_MANY_VARYINGS;
    case VSC_ERR_TOO_MANY_OUTPUTS:
        return gcvSTATUS_TOO_MANY_OUTPUT;
    case VSC_ERR_UNDECLARED_VARYING:
        return gcvSTATUS_UNDECLARED_VARYING;
    case VSC_ERR_VARYING_TYPE_MISMATCH:
        return gcvSTATUS_VARYING_TYPE_MISMATCH;
    case VSC_ERR_MISSING_MAIN:
        return gcvSTATUS_MISSING_MAIN;
    case VSC_ERR_NAME_MISMATCH:
        return gcvSTATUS_NAME_MISMATCH;
    case VSC_ERR_INVALID_INDEX:
        return gcvSTATUS_INVALID_INDEX;
    case VSC_ERR_UNIFORMS_TOO_MANY:
        return gcvSTATUS_TOO_MANY_UNIFORMS;
    case VSC_ERR_UNIFORM_TYPE_MISMATCH:
        return gcvSTATUS_UNIFORM_TYPE_MISMATCH;
    case VSC_ERR_LOCATION_MISMATCH:
        return gcvSTATUS_INVALID_ARGUMENT;
    case VSC_ERR_LOCATION_ALIASED:
        return gcvSTATUS_LOCATION_ALIASED;

    default:
        return (gceSTATUS)(-errCode);
    };
}

