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


#include <gc_vx_common.h>
#include <gc_vx_profiler.h>

gctINT
vxoProfiler_Initialize(
    vx_context context
    )
{
    gctINT status = gcvSTATUS_OK;
    gctINT profileMode = 0;
    char *env = gcvNULL;

    if (!vxoContext_IsValid(context)) return VX_ERROR_INVALID_REFERENCE;

    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_PROFILE", &env)) && env)
    {
        if gcmIS_SUCCESS(gcoOS_StrCmp(env, "1"))
        {
            profileMode = 1;
        }
    }

    if (profileMode == 0)
    {
        context->profiler.enable = gcvFALSE;
        context->profiler.perVxfinish = gcvFALSE;
        return status;
    }

    if(context->phal == gcvNULL)
    {
        gctPOINTER pointer = gcvNULL;
        if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL,
                       gcmSIZEOF(struct _gcoHAL),
                       &pointer)))
        {
            gcmFATAL("%s(%d): gcoOS_Allocate failed", __FUNCTION__, __LINE__);
            return status;
        }
        gcoOS_MemFill(pointer, 0, gcmSIZEOF(struct _gcoHAL));
        context->phal = (gcoHAL) pointer;
    }

    status = gcoPROFILER_Initialize(context->phal, gcvNULL, gcvTRUE);
    switch (status)
    {
        case gcvSTATUS_OK:
            break;
        case gcvSTATUS_MISMATCH: /*fall through*/
        case gcvSTATUS_NOT_SUPPORTED:
        default:
            context->profiler.enable = gcvFALSE;
            if(context->phal != gcvNULL)
                gcoOS_Free(gcvNULL, context->phal);
            return status;
    }

    /* Clear the profiler. */
    gcoOS_ZeroMemory(&context->profiler, gcmSIZEOF(context->profiler));
    context->profiler.enable = gcvTRUE;

    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VP_FRAME_NUM", &env)))
    {
        if ((env != gcvNULL) && (env[0] !=0))
        {
            int frameNum;
            gcoOS_StrToInt(env, &frameNum);
            if (frameNum > 1)
                context->profiler.frameMaxNum = (gctUINT32)frameNum;
        }
    }

    if (profileMode == 2)
    {
        context->profiler.perVxfinish = gcvTRUE;
    }

    {
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

        gcoHAL_GetProductName(gcvNULL, &productName);
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

    return status;
}

void
vxoProfiler_Destroy(
    vx_context context
    )
{
    gcsHAL_INTERFACE iface;

    if (!vxoContext_IsValid(context)) return;

    if (context->profiler.enable)
    {
        /* disable profiler in kernel. */
        iface.ignoreTLS = gcvFALSE;
        iface.command = gcvHAL_SET_PROFILE_SETTING;
        iface.u.SetProfileSetting.enable = gcvFALSE;

        /* Call the kernel. */
        gcoOS_DeviceControl(gcvNULL,
            IOCTL_GCHAL_INTERFACE,
            &iface, gcmSIZEOF(iface),
            &iface, gcmSIZEOF(iface));

        context->profiler.enable = gcvFALSE;
        gcmVERIFY_OK(gcoPROFILER_Destroy(context->phal));
        if(context->phal != gcvNULL)
            gcoOS_Free(gcvNULL, context->phal);
    }
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

    gcoPROFILER_Begin(context->phal, 0);
    gcoOS_GetTime(&context->profiler.frameStartTimeusec);

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

    context = vxoContext_GetFromReference(reference);

    if (!vxoContext_IsValid(context)) return VX_ERROR_INVALID_REFERENCE;

    if (!context->profiler.enable)
    {
        return status;
    }

    gcmWRITE_COUNTER(VPG_FRAME, context->profiler.frameNumber);
    gcoPROFILER_EndFrame(context->phal);

    gcoOS_GetTime(&context->profiler.frameEndTimeusec);

    gcmWRITE_CONST(VPG_TIME);
    gcmWRITE_COUNTER(VPC_ELAPSETIME, (gctINT32) (context->profiler.frameEndTimeusec
                     - context->profiler.frameStartTimeusec));
    gcmWRITE_CONST(VPG_END);
    gcmWRITE_CONST(VPG_END);

    gcoPROFILER_Flush(context->phal);

    gcmPRINT("VPC_ELAPSETIME: %d\n", (gctINT32) (context->profiler.frameEndTimeusec
                     - context->profiler.frameStartTimeusec));
    gcmPRINT("*********\n");
    context->profiler.frameNumber++;

    /* Return the status. */
    return status;
}

