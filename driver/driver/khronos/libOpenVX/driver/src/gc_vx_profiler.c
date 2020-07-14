/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <gc_vx_common.h>
#include <gc_vx_profiler.h>

#define _GC_OBJ_ZONE            gcdZONE_VX_OTHERS

#if VIVANTE_PROFILER
gctINT
vxoProfiler_Initialize(
    vx_context context
    )
{
    gctINT status = gcvSTATUS_OK;
    gctINT profileMode = 0;
    char *env = gcvNULL;


    gcmHEADER_ARG("context=0x%x ", context);

    if (!vxoContext_IsValid(context)) {
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_REFERENCE;
    }
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_PROFILE", &env)) && env)
    {
        if gcmIS_SUCCESS(gcoOS_StrCmp(env, "0"))
        {
            gcoPROFILER_Disable();
            context->profiler.enable = gcvFALSE;
            context->profiler.perGraphProcess = gcvFALSE;
            gcmFOOTER_ARG("%d", status);
            return status;
        }
        if gcmIS_SUCCESS(gcoOS_StrCmp(env, "1"))
        {
            profileMode = 1;
        }
        if gcmIS_SUCCESS(gcoOS_StrCmp(env, "2"))
        {
            profileMode = 2;
            context->profiler.perGraphProcess = gcvTRUE;
        }
    }

    if (profileMode == 0)
    {
        context->profiler.enable = gcvFALSE;
        context->profiler.perGraphProcess = gcvFALSE;
        gcmFOOTER_ARG("%d", status);
        return status;
    }

    if(context->halProfile == gcvNULL)
    {
        status = gcoPROFILER_Construct(&context->halProfile);
        if(status < 0)
        {
            gcmUSER_DEBUG_ERROR_MSG(
            "OVX: (Vivante Profile) Unable to create profile object.\n");
            goto OnError;
        }
    }

    /* Clear the profiler. */
    gcoOS_ZeroMemory(&context->profiler, gcmSIZEOF(context->profiler));
    context->profiler.enable = gcvTRUE;
    context->halProfile->profilerClient = gcvCLIENT_OPENVX;

    if (gcoPROFILER_Initialize(context->halProfile) != gcvSTATUS_OK)
    {
        context->profiler.enable = gcvFALSE;
        goto OnError;
    }

    {
        gcoPROFILER Profiler = context->halProfile;
        /* Write Generic Info. */
        char* infoCompany = "Vivante Corporation";
        char* infoVersion = "1.3";
        char  infoRevision[255] = {'\0'};   /* read from hw */
        char  infoRenderer[255] = {'\0'};
        char* infoDriver = "OpenVX 1.0.1";
        gceCHIPMODEL chipModel;
        gctUINT32 chipRevision;
        gctUINT offset = 0;
        gctSTRING productName = gcvNULL;

        gcoHAL_QueryChipIdentity(gcvNULL,&chipModel, &chipRevision,gcvNULL,gcvNULL);

#define BCD(digit)      ((chipRevision >> (digit * 4)) & 0xF)
        gcoOS_MemFill(infoRevision, 0, gcmSIZEOF(infoRevision));
        if (BCD(3) == 0)
        {
            /* Old format. */
            gcoOS_PrintStrSafe(infoRevision, gcmSIZEOF(infoRevision),
                &offset, "revision=\"%d.%d\" ", BCD(1), BCD(0));
        }
        else
        {
            /* New format. */
            gcoOS_PrintStrSafe(infoRevision, gcmSIZEOF(infoRevision),
                &offset, "revision=\"%d.%d.%d_rc%d\" ",
                BCD(3), BCD(2), BCD(1), BCD(0));
        }

        gcoHAL_GetProductName(gcvNULL, &productName, gcvNULL);
        gcoOS_StrCatSafe(infoRenderer, 9, "Vivante ");
        gcoOS_StrCatSafe(infoRenderer, 23, productName);
        gcmOS_SAFE_FREE(gcvNULL, productName);

        gcmWRITE_CONST(VPG_INFO);

        gcmWRITE_CONST(VPC_INFOCOMPANY);
        gcmWRITE_STRING(infoCompany);
        gcmWRITE_CONST(VPC_INFOVERSION);
        gcmWRITE_STRING(infoVersion);
        gcmWRITE_CONST(VPC_INFORENDERER);
        gcmWRITE_STRING(infoRenderer);
        gcmWRITE_CONST(VPC_INFOREVISION);
        gcmWRITE_STRING(infoRevision);
        gcmWRITE_CONST(VPC_INFODRIVER);
        gcmWRITE_STRING(infoDriver);

        gcmWRITE_CONST(VPG_END);
    }

    gcoOS_GetTime(&context->profiler.frameStartTimeusec);

OnError:
    /* Return the status. */
    gcmFOOTER_ARG("%d", status);
    return status;
}

void
vxoProfiler_Destroy(
    vx_context context
    )
{
    gcmHEADER_ARG("context=0x%x", context);

    if (!vxoContext_IsValid(context)) return;

    if (context->profiler.enable)
    {
        gcoPROFILER_Destroy(context->halProfile);
        context->profiler.enable = gcvFALSE;

    }

    gcmFOOTER_NO();
    return;
}

gctINT
vxoProfiler_Begin(
    vx_reference reference
    )
{
    gctINT status = gcvSTATUS_OK;
    vx_context context;

    context = vxoContext_GetFromReference(reference);

    if (!vxoContext_IsValid(context)) return VX_ERROR_INVALID_REFERENCE;

    if (!context->profiler.enable)
    {
        return status;
    }

    gcoOS_GetTime(&context->profiler.frameStartTimeusec);
    gcoPROFILER_EnableCounters(context->halProfile, gcvCOUNTER_OP_FRAME);

    /* Return the status. */
    return status;
}

gctINT
vxoProfiler_End(
    vx_reference reference
    )
{
    gctINT status = gcvSTATUS_OK;
    vx_context context;
    gcoPROFILER Profiler;
    gcmHEADER_ARG("reference=%p", reference);

    context = vxoContext_GetFromReference(reference);

    if (!vxoContext_IsValid(context))
    {
        gcmFOOTER_ARG("%d", status);
        return VX_ERROR_INVALID_REFERENCE;
    }
    if (!context->profiler.enable)
    {
        gcmFOOTER_ARG("%d", status);
        return status;
    }

    Profiler = context->halProfile;

    gcmWRITE_COUNTER(VPG_FRAME, context->profiler.frameNumber);
    gcoPROFILER_End(context->halProfile, gcvCOUNTER_OP_FRAME, context->profiler.frameNumber);

    gcoOS_GetTime(&context->profiler.frameEndTimeusec);

    gcmWRITE_CONST(VPG_TIME);
    gcmWRITE_COUNTER(VPC_ELAPSETIME, (gctINT32) (context->profiler.frameEndTimeusec
                     - context->profiler.frameStartTimeusec));
    gcmWRITE_CONST(VPG_END);
    gcmWRITE_CONST(VPG_END);

    gcoPROFILER_Flush(context->halProfile);

    vxInfo("VPC_ELAPSETIME: %d\n", (gctINT32) (context->profiler.frameEndTimeusec
                     - context->profiler.frameStartTimeusec));
    vxInfo("*********\n");
    context->profiler.frameNumber++;

    gcmFOOTER_ARG("%d", status);
    /* Return the status. */
    return status;
}
#endif

