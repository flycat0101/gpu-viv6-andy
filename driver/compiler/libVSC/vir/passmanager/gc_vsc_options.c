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


#include "gc_vsc.h"

gctUINT32 _VSC_OPTN_GetSubOptionLength(
    IN gctSTRING str
    )
{
    gctSTRING pos = str;
    while(*pos != ':' && *pos != ' ' && *pos != '\0')
    {
        pos++;
    }

    return (gctUINT32)(pos - str);
}

/* Simple Copy Propagation options */
void VSC_OPTN_SCPPOptions_SetDefault(
    IN OUT VSC_OPTN_SCPPOptions* options,
    IN gctUINT optLevel
    )
{
    if(optLevel >= 2)
    {
        VSC_OPTN_SCPPOptions_SetSwitchOn(options, gcvTRUE);
    }
    else
    {
        VSC_OPTN_SCPPOptions_SetSwitchOn(options, gcvFALSE);
    }
    VSC_OPTN_SCPPOptions_SetPassId(options, 0);
    VSC_OPTN_SCPPOptions_SetTrace(options, 0);
}

void VSC_OPTN_SCPPOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_SCPPOptions* options
    )
{
    while (str[0] == ':')
    {
        ++str;
        if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "on", sizeof("on") - 1))
        {
            VSC_OPTN_SCPPOptions_SetSwitchOn(options, gcvTRUE);
            str += sizeof("on") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "off", sizeof("off") - 1))
        {
            VSC_OPTN_SCPPOptions_SetSwitchOn(options, gcvFALSE);
            str += sizeof("off") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "trace:", sizeof("trace:")-1))
        {
            gctUINT32 trace;
            gctUINT32 len;

            str += sizeof("trace:") -1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            trace = vscSTR_StrToUint32(str, len);
            VSC_OPTN_SCPPOptions_SetTrace(options, trace);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "bs:", sizeof("bs:") - 1))
        {
            gctUINT32 before_shader;
            gctUINT32 len;

            str += sizeof("bs:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            before_shader = vscSTR_StrToUint32(str, len);
            VSC_OPTN_SCPPOptions_SetBeforeShader(options, before_shader);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "as:", sizeof("as:") - 1))
        {
            gctUINT32 after_shader;
            gctUINT32 len;

            str += sizeof("as:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            after_shader = vscSTR_StrToUint32(str, len);
            VSC_OPTN_SCPPOptions_SetAfterShader(options, after_shader);
            str += len;
        }
    }
}

void VSC_OPTN_SCPPOptions_Dump(
    IN VSC_OPTN_SCPPOptions* options,
    IN VIR_Dumper* dumper
    )
{
    VIR_LOG(dumper, "SCPP options:\n");
    VIR_LOG(dumper, "    on: %s\n", VSC_OPTN_SCPPOptions_GetSwitchOn(options) ? "true" : "false");
    VIR_LOG(dumper, "    trace: %x\n", VSC_OPTN_SCPPOptions_GetTrace(options));
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_SCPPOptions_Usage(
    IN VIR_Dumper* dumper
    )
{
    gctSTRING usage =
        "-SCPP:\n"
        "    on                         turn on VIR Simple Copy Propagation\n"
        "    off                        turn off VIR Simple Copy Propagation\n"
        "    opts:                0x1    \n"
        "    trace:                     0x1    \n"
        "                               0x2    \n";

    VIR_LOG(dumper, usage);
    VIR_LOG_FLUSH(dumper);
}

/* Long Parameter Optimization options */
void VSC_OPTN_ParamOptOptions_SetDefault(
    IN OUT VSC_OPTN_ParamOptOptions* options,
    IN gctUINT optLevel
    )
{
    VSC_OPTN_ParamOptOptions_SetLongArrayThreshold(options, 64);
    VSC_OPTN_ParamOptOptions_SetSwitchOn(options, gcvTRUE);
    VSC_OPTN_ParamOptOptions_SetPassId(options, 0);
    VSC_OPTN_ParamOptOptions_SetTrace(options, 0);
}

void VSC_OPTN_ParamOptOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_ParamOptOptions* options
    )
{
    while (str[0] == ':')
    {
        ++str;
        if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "on", sizeof("on") - 1))
        {
            VSC_OPTN_ParamOptOptions_SetSwitchOn(options, gcvTRUE);
            str += sizeof("on") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "off", sizeof("off") - 1))
        {
            VSC_OPTN_ParamOptOptions_SetSwitchOn(options, gcvFALSE);
            str += sizeof("off") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "threshold:", sizeof("threshold:") - 1))
        {
            gctINT32 threshold;
            gctUINT32 len;

            str += sizeof("threshold:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            threshold = (gctINT32)vscSTR_StrToUint32(str, len);
            VSC_OPTN_ParamOptOptions_SetLongArrayThreshold(options, threshold);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "trace:", sizeof("trace:")-1))
        {
            gctUINT32 trace;
            gctUINT32 len;

            str += sizeof("trace:") -1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            trace = vscSTR_StrToUint32(str, len);
            VSC_OPTN_ParamOptOptions_SetTrace(options, trace);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "bs:", sizeof("bs:") - 1))
        {
            gctUINT32 before_shader;
            gctUINT32 len;

            str += sizeof("bs:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            before_shader = vscSTR_StrToUint32(str, len);
            VSC_OPTN_ParamOptOptions_SetBeforeShader(options, before_shader);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "as:", sizeof("as:") - 1))
        {
            gctUINT32 after_shader;
            gctUINT32 len;

            str += sizeof("as:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            after_shader = vscSTR_StrToUint32(str, len);
            VSC_OPTN_ParamOptOptions_SetAfterShader(options, after_shader);
            str += len;
        }
    }
}

void VSC_OPTN_ParamOptOptions_Dump(
    IN VSC_OPTN_ParamOptOptions* options,
    IN VIR_Dumper* dumper
    )
{
    VIR_LOG(dumper, "PARAMOPT options:\n");
    VIR_LOG(dumper, "    on: %s\n", VSC_OPTN_ParamOptOptions_GetSwitchOn(options) ? "true" : "false");
    VIR_LOG(dumper, "    trace: %x\n", VSC_OPTN_ParamOptOptions_GetTrace(options));
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_ParamOptOptions_Usage(
    IN VIR_Dumper* dumper
    )
{
    gctSTRING usage =
        "-PARAMOPT:\n"
        "    on                         turn on Long Parameter Optimizer\n"
        "    off                        turn off Long Parameter Optimizer\n"
        "    threshold:                0x1    \n"
        "    trace:                     0x1    \n"
        "                               0x2    \n";

    VIR_LOG(dumper, usage);
    VIR_LOG_FLUSH(dumper);
}

/* Loop Optimization options */
void VSC_OPTN_LoopOptsOptions_SetDefault(
    IN OUT VSC_OPTN_LoopOptsOptions* options,
    IN gctUINT optLevel
    )
{
    if(optLevel >=2)
    {
        VSC_OPTN_LoopOptsOptions_SetSwitchOn(options, gcvTRUE);
    }
    else
    {
        VSC_OPTN_LoopOptsOptions_SetSwitchOn(options, gcvFALSE);
    }
    VSC_OPTN_LoopOptsOptions_SetOpts(options,
                                     VSC_OPTN_LoopOptsOptions_OPTS_NONE |
                                     VSC_OPTN_LoopOptsOptions_OPTS_LOOP_INVERSION |
                                     VSC_OPTN_LoopOptsOptions_OPTS_LOOP_INVARIANT |
                                     VSC_OPTN_LoopOptsOptions_OPTS_LOOP_UNROLLING);
    VSC_OPTN_LoopOptsOptions_SetFullUnrollingFactor(options, 16);
    VSC_OPTN_LoopOptsOptions_SetTotalUnrollingFactor(options, 64);
    VSC_OPTN_LoopOptsOptions_SetPartialUnrollingFactor(options, 4);
    VSC_OPTN_LoopOptsOptions_SetLICMFactor(options, 16);
    VSC_OPTN_LoopOptsOptions_SetTrace(options, 0);

    VSC_OPTN_LoopOptsOptions_SetBeforeShader(options, gcvMAXUINT32);
    VSC_OPTN_LoopOptsOptions_SetAfterShader(options, gcvMAXUINT32);
}

void VSC_OPTN_LoopOptsOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_LoopOptsOptions* options
    )
{
    while (str[0] == ':')
    {
        ++str;
        if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "on", sizeof("on") - 1))
        {
            VSC_OPTN_LoopOptsOptions_SetSwitchOn(options, gcvTRUE);
            str += sizeof("on") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "off", sizeof("off") - 1))
        {
            VSC_OPTN_LoopOptsOptions_SetSwitchOn(options, gcvFALSE);
            str += sizeof("off") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "opts:", sizeof("opts:") - 1))
        {
            gctUINT32 opts;
            gctUINT32 len;

            str += sizeof("opts:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            opts = vscSTR_StrToUint32(str, len);
            VSC_OPTN_LoopOptsOptions_SetOpts(options, opts);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "tof:", sizeof("tof:") - 1))
        {
            gctINT32 tof;
            gctUINT32 len;

            str += sizeof("tof:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            tof = (gctINT32)vscSTR_StrToUint32(str, len);
            VSC_OPTN_LoopOptsOptions_SetTotalUnrollingFactor(options, tof);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "fuf:", sizeof("fuf:") - 1))
        {
            gctINT32 fuf;
            gctUINT32 len;

            str += sizeof("fuf:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            fuf = (gctINT32)vscSTR_StrToUint32(str, len);
            VSC_OPTN_LoopOptsOptions_SetFullUnrollingFactor(options, fuf);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "puf:", sizeof("puf:") - 1))
        {
            gctINT32 puf;
            gctUINT32 len;

            str += sizeof("puf:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            puf = (gctINT32)vscSTR_StrToUint32(str, len);
            VSC_OPTN_LoopOptsOptions_SetPartialUnrollingFactor(options, puf);
            str += len;
        }else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "licm:", sizeof("licm:") - 1))
        {
            gctINT32 licm;
            gctUINT32 len;

            str += sizeof("licm:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            licm = (gctINT32)vscSTR_StrToUint32(str, len);
            VSC_OPTN_LoopOptsOptions_SetLICMFactor(options, licm);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "trace:", sizeof("trace:")-1))
        {
            gctUINT32 trace;
            gctUINT32 len;

            str += sizeof("trace:") -1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            trace = vscSTR_StrToUint32(str, len);
            VSC_OPTN_LoopOptsOptions_SetTrace(options, trace);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "bs:", sizeof("bs:") - 1))
        {
            gctUINT32 before_shader;
            gctUINT32 len;

            str += sizeof("bs:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            before_shader = vscSTR_StrToUint32(str, len);
            VSC_OPTN_LoopOptsOptions_SetBeforeShader(options, before_shader);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "as:", sizeof("as:") - 1))
        {
            gctUINT32 after_shader;
            gctUINT32 len;

            str += sizeof("as:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            after_shader = vscSTR_StrToUint32(str, len);
            VSC_OPTN_LoopOptsOptions_SetAfterShader(options, after_shader);
            str += len;
        }
    }
}

void VSC_OPTN_LoopOptsOptions_Dump(
    IN VSC_OPTN_LoopOptsOptions* options,
    IN VIR_Dumper* dumper
    )
{
    VIR_LOG(dumper, "LOOP options:\n");
    VIR_LOG(dumper, "    on: %s\n", VSC_OPTN_LoopOptsOptions_GetSwitchOn(options) ? "true" : "false");
    VIR_LOG(dumper, "    opts: %x\n", VSC_OPTN_LoopOptsOptions_GetOpts(options));
    VIR_LOG(dumper, "    trace: %x\n", VSC_OPTN_LoopOptsOptions_GetTrace(options));
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_LoopOptsOptions_Usage(
    IN VIR_Dumper* dumper
    )
{
    gctSTRING usage =
        "-LOOP:\n"
        "    on                         turn on VIR loop optimizer\n"
        "    off                        turn off VIR loop optimizer\n"
        "    opts:                      0x1    \n"
        "    trace:                     0x1    \n"
        "                               0x2    \n";

    VIR_LOG(dumper, usage);
    VIR_LOG_FLUSH(dumper);
}

/* Control Flow Optimization options */
void VSC_OPTN_CFOOptions_SetDefault(
    IN OUT VSC_OPTN_CFOOptions* options,
    IN gctUINT optLevel
    )
{
    gctUINT i;

    for(i = 0; i < VSC_OPTN_CFO_COUNT; i++)
    {
        if(optLevel >=2)
        {
            VSC_OPTN_CFOOptions_SetSwitchOn(options + i, gcvTRUE);
        }
        else
        {
            VSC_OPTN_CFOOptions_SetSwitchOn(options + i, gcvFALSE);
        }
        VSC_OPTN_CFOOptions_SetOpts(options + i,
                                         VSC_OPTN_CFOOptions_OPTS_NONE |
                                         VSC_OPTN_CFOOptions_OPTS_PATTERN /*|
                                         VSC_OPTN_CFOOptions_OPTS_GEN_SELECT*/);
        VSC_OPTN_CFOOptions_SetTrace(options + i, 0);

        VSC_OPTN_CFOOptions_SetBeforeShader(options + i, gcvMAXUINT32);
        VSC_OPTN_CFOOptions_SetAfterShader(options + i, gcvMAXUINT32);
    }
}

void VSC_OPTN_CFOOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_CFOOptions* options
    )
{
    while (str[0] == ':')
    {
        ++str;
        if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "on", sizeof("on") - 1))
        {
            VSC_OPTN_CFOOptions_SetSwitchOn(options, gcvTRUE);
            str += sizeof("on") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "off", sizeof("off") - 1))
        {
            VSC_OPTN_CFOOptions_SetSwitchOn(options, gcvFALSE);
            str += sizeof("off") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "opts:", sizeof("opts:") - 1))
        {
            gctUINT32 opts;
            gctUINT32 len;

            str += sizeof("opts:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            opts = vscSTR_StrToUint32(str, len);
            VSC_OPTN_CFOOptions_SetOpts(options, opts);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "trace:", sizeof("trace:")-1))
        {
            gctUINT32 trace;
            gctUINT32 len;

            str += sizeof("trace:") -1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            trace = vscSTR_StrToUint32(str, len);
            VSC_OPTN_CFOOptions_SetTrace(options, trace);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "bs:", sizeof("bs:") - 1))
        {
            gctUINT32 before_shader;
            gctUINT32 len;

            str += sizeof("bs:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            before_shader = vscSTR_StrToUint32(str, len);
            VSC_OPTN_CFOOptions_SetBeforeShader(options, before_shader);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "as:", sizeof("as:") - 1))
        {
            gctUINT32 after_shader;
            gctUINT32 len;

            str += sizeof("as:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            after_shader = vscSTR_StrToUint32(str, len);
            VSC_OPTN_CFOOptions_SetAfterShader(options, after_shader);
            str += len;
        }
    }
}

void VSC_OPTN_CFOOptions_Dump(
    IN VSC_OPTN_CFOOptions* options,
    IN VIR_Dumper* dumper
    )
{
    VIR_LOG(dumper, "CFO options:\n");
    VIR_LOG(dumper, "    on: %s\n", VSC_OPTN_CFOOptions_GetSwitchOn(options) ? "true" : "false");
    VIR_LOG(dumper, "    opts: %x\n", VSC_OPTN_CFOOptions_GetOpts(options));
    VIR_LOG(dumper, "    trace: %x\n", VSC_OPTN_CFOOptions_GetTrace(options));
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_CFOOptions_Usage(
    IN VIR_Dumper* dumper
    )
{
    gctSTRING usage =
        "-CFO:\n"
        "    on                         turn on VIR control flow optimizer\n"
        "    off                        turn off VIR control flow optimizer\n"
        "    opts:                      0x1    \n"
        "    trace:                     0x1    \n"
        "                               0x2    \n";

    VIR_LOG(dumper, usage);
    VIR_LOG_FLUSH(dumper);
}

/* Default UBO options */
void VSC_OPTN_UF_AUBOOptions_SetDefault(
    IN OUT VSC_OPTN_UF_AUBOOptions* options,
    IN gctUINT optLevel
    )
{
    VSC_OPTN_UF_AUBOOptions_SetSwitchOn(options, gcvTRUE);
    VSC_OPTN_UF_AUBOOptions_SetHeuristics(options,
                                           /* VSC_OPTN_UF_AUBOOptions_HEUR_FORCE_ALL |
                                           VSC_OPTN_UF_AUBOOptions_HEUR_INDIRECT_IMM |
                                           VSC_OPTN_UF_AUBOOptions_HEUR_INDIRECT_UD |
                                           VSC_OPTN_UF_AUBOOptions_HEUR_INDIRECT_D | */
                                           VSC_OPTN_UF_AUBOOptions_HEUR_ORDERLY /* |
                                           VSC_OPTN_UF_AUBOOptions_HEUR_SKIP_DUBO |
                                           VSC_OPTN_UF_AUBOOptions_HEUR_SKIP_CUBO*/);
    VSC_OPTN_UF_AUBOOptions_SetConstRegReservation(options, 0);
    VSC_OPTN_UF_AUBOOptions_SetOpt(options, 1);
    VSC_OPTN_UF_AUBOOptions_SetTrace(options, 0);

    VSC_OPTN_PHOptions_SetBeforeShader(options, gcvMAXUINT32);
    VSC_OPTN_PHOptions_SetAfterShader(options, gcvMAXUINT32);
}

void VSC_OPTN_UF_AUBOOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_UF_AUBOOptions* options
    )
{
    while (str[0] == ':')
    {
        ++str;
        if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "on", sizeof("on") - 1))
        {
            VSC_OPTN_UF_AUBOOptions_SetSwitchOn(options, gcvTRUE);
            str += sizeof("on") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "off", sizeof("off") - 1))
        {
            VSC_OPTN_UF_AUBOOptions_SetSwitchOn(options, gcvFALSE);
            str += sizeof("off") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "heuristics:", sizeof("heuristics:") - 1))
        {
            gctUINT32 heuristics;
            gctUINT32 len;

            str += sizeof("heuristics:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            heuristics = vscSTR_StrToUint32(str, len);
            VSC_OPTN_UF_AUBOOptions_SetHeuristics(options, heuristics);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "const_reg_reservation:", sizeof("const_reg_reservation:") - 1))
        {
            gctUINT32 const_reg_reservation;
            gctUINT32 len;

            str += sizeof("const_reg_reservation:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            const_reg_reservation = vscSTR_StrToUint32(str, len);
            VSC_OPTN_UF_AUBOOptions_SetConstRegReservation(options, const_reg_reservation);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "opt:", sizeof("opt:") - 1))
        {
            gctUINT32 opt;
            gctUINT32 len;

            str += sizeof("opt:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            opt = vscSTR_StrToUint32(str, len);
            VSC_OPTN_UF_AUBOOptions_SetOpt(options, opt);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "trace:", sizeof("trace:")-1))
        {
            gctUINT32 trace;
            gctUINT32 len;

            str += sizeof("trace:") -1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            trace = vscSTR_StrToUint32(str, len);
            VSC_OPTN_UF_AUBOOptions_SetTrace(options, trace);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "bs:", sizeof("bs:") - 1))
        {
            gctUINT32 before_shader;
            gctUINT32 len;

            str += sizeof("bs:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            before_shader = vscSTR_StrToUint32(str, len);
            VSC_OPTN_UF_AUBOOptions_SetBeforeShader(options, before_shader);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "as:", sizeof("as:") - 1))
        {
            gctUINT32 after_shader;
            gctUINT32 len;

            str += sizeof("as:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            after_shader = vscSTR_StrToUint32(str, len);
            VSC_OPTN_UF_AUBOOptions_SetAfterShader(options, after_shader);
            str += len;
        }
    }
}

void VSC_OPTN_UF_AUBOOptions_Dump(
    IN VSC_OPTN_UF_AUBOOptions* options,
    IN VIR_Dumper* dumper
    )
{
    VIR_LOG(dumper, "default UBO options:\n");
    VIR_LOG(dumper, "    on: %s\n", VSC_OPTN_UF_AUBOOptions_GetSwitchOn(options) ? "true" : "false");
    VIR_LOG(dumper, "    heuristics: %x\n", VSC_OPTN_UF_AUBOOptions_GetHeuristics(options));
    VIR_LOG(dumper, "    const_reg_reservation: %x\n", VSC_OPTN_UF_AUBOOptions_GetConstRegReservation(options));
    VIR_LOG(dumper, "    opt: %s\n", VSC_OPTN_UF_AUBOOptions_GetOpt(options) ? "true" : "false");
    VIR_LOG(dumper, "    trace: %x\n", VSC_OPTN_UF_AUBOOptions_GetTrace(options));
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_UF_AUBOOptions_Usage(
    IN VIR_Dumper* dumper
    )
{
    gctSTRING usage =
        "-DUBO:\n"
        "    on                         turn on VIR DUBO phase\n"
        "    off                        turn off VIR  DUBO phase\n"
        "    heuristics:                0x1    \n"
        "    const_reg_reservation:     number of constant register to reserve\n"
        "    opt_on                     turn on optimization\n"
        "    opt_on                     turn off optimization\n"
        "    trace:                     0x1    \n"
        "                               0x2    \n";

    VIR_LOG(dumper, usage);
    VIR_LOG_FLUSH(dumper);
}

/* Inliner options */
void VSC_OPTN_ILOptions_SetDefault(
    IN OUT VSC_OPTN_ILOptions* options,
    IN gctUINT optLevel
    )
{
    if(optLevel >= 1)
    {
        VSC_OPTN_ILOptions_SetSwitchOn(options, gcvFALSE);
    }
    else
    {
        VSC_OPTN_ILOptions_SetSwitchOn(options, gcvFALSE);
    }
    VSC_OPTN_ILOptions_SetHeuristics(options, 0xffffffff);
    VSC_OPTN_ILOptions_SetTrace(options, 0);
    VSC_OPTN_ILOptions_SetInlineLevel(options, VSC_OPTN_ILOptions_LEVEL3);
}

void VSC_OPTN_ILOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_ILOptions* options
    )
{
    while (str[0] == ':')
    {
        ++str;
        if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "on", sizeof("on") - 1))
        {
            VSC_OPTN_ILOptions_SetSwitchOn(options, gcvTRUE);
            str += sizeof("on") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "off", sizeof("off") - 1))
        {
            VSC_OPTN_ILOptions_SetSwitchOn(options, gcvFALSE);
            str += sizeof("off") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "heuristics:", sizeof("heuristics:") - 1))
        {
            gctUINT32 heuristics;
            gctUINT32 len;

            str += sizeof("heuristics:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            heuristics = vscSTR_StrToUint32(str, len);
            VSC_OPTN_ILOptions_SetHeuristics(options, heuristics);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "level:", sizeof("level:") - 1))
        {
            gctUINT32 level;
            gctUINT32 len;

            str += sizeof("level:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            level = vscSTR_StrToUint32(str, len);
            /* inline level:
                0: disable inline
                1: inline ALWAYSINLINE function only
                2: minimal inline
                3: default inline
                4: maximal inline
            */
            if (level >= VSC_OPTN_ILOptions_LEVEL4)
            {
                VSC_OPTN_ILOptions_SetInlineLevel(options, VSC_OPTN_ILOptions_LEVEL4);
            }
            else
            {
                VSC_OPTN_ILOptions_SetInlineLevel(options, level);
            }

            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "trace:", sizeof("trace:")-1))
        {
            gctUINT32 trace;
            gctUINT32 len;

            str += sizeof("trace:") -1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            trace = vscSTR_StrToUint32(str, len);
            VSC_OPTN_ILOptions_SetTrace(options, trace);
            str += len;
        }
    }

    if (VSC_OPTN_ILOptions_GetInlineLevel(options) == VSC_OPTN_ILOptions_LEVEL1)
    {
        VSC_OPTN_ILOptions_SetSwitchOn(options, gcvTRUE);
    }
}

void VSC_OPTN_ILOptions_Dump(
    IN VSC_OPTN_ILOptions* options,
    IN VIR_Dumper* dumper
    )
{
    VIR_LOG(dumper, "inliner options:\n");
    VIR_LOG(dumper, "    on: %s\n", VSC_OPTN_ILOptions_GetSwitchOn(options) ? "true" : "false");
    VIR_LOG(dumper, "    heuristics: %x\n", VSC_OPTN_ILOptions_GetHeuristics(options));
    VIR_LOG(dumper, "    trace: %x\n", VSC_OPTN_ILOptions_GetTrace(options));
    VIR_LOG(dumper, "    level: %x\n", VSC_OPTN_ILOptions_GetInlineLevel(options));
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_ILOptions_Usage(
    IN VIR_Dumper* dumper
    )
{
    gctSTRING usage =
        "-IL:\n"
        "    on                  turn on VIR inliner\n"
        "    off                 turn off VIR inliner\n"
        "    heuristics:         0x1    top down inline\n"
        "    trace:              0x1    trace inliner\n"
        "                        0x2    trace output\n"
        "    level:              0x0    disable inline\n"
        "                        0x1    minimal inline\n"
        "                        0x2    default inline\n"
        "                        0x3    miximal inline\n";

    VIR_LOG(dumper, usage);
    VIR_LOG_FLUSH(dumper);
}

/* Precision Update options */
void VSC_OPTN_PUOptions_SetDefault(
    IN OUT VSC_OPTN_PUOptions* options,
    IN gctUINT optLevel
    )
{
    VSC_OPTN_PUOptions_SetSwitchOn(options, gcvTRUE);
    VSC_OPTN_PUOptions_SetTrace(options, 0);
}

void VSC_OPTN_PUOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_PUOptions* options
    )
{
    while (str[0] == ':')
    {
        ++str;
        if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "on", sizeof("on") - 1))
        {
            VSC_OPTN_PUOptions_SetSwitchOn(options, gcvTRUE);
            str += sizeof("on") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "off", sizeof("off") - 1))
        {
            VSC_OPTN_PUOptions_SetSwitchOn(options, gcvFALSE);
            str += sizeof("off") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "trace:", sizeof("trace:")-1))
        {
            gctUINT32 trace;
            gctUINT32 len;

            str += sizeof("trace:") -1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            trace = vscSTR_StrToUint32(str, len);
            VSC_OPTN_PUOptions_SetTrace(options, trace);
            str += len;
        }
    }
}

void VSC_OPTN_PUOptions_Dump(
    IN VSC_OPTN_PUOptions* options,
    IN VIR_Dumper* dumper
    )
{
    VIR_LOG(dumper, "precision updater options:\n");
    VIR_LOG(dumper, "    on: %s\n", VSC_OPTN_PUOptions_GetSwitchOn(options) ? "true" : "false");
    VIR_LOG(dumper, "    trace: %x\n", VSC_OPTN_PUOptions_GetTrace(options));
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_PUOptions_Usage(
    IN VIR_Dumper* dumper
    )
{
    gctSTRING usage =
        "-PU:\n"
        "    on                  turn on VIR precision updater\n"
        "    off                 turn off VIR precision updater\n"
        "    trace:              0x1    trace precision updater\n"
        "                        0x2    trace output\n";

    VIR_LOG(dumper, usage);
    VIR_LOG_FLUSH(dumper);
}

/* M2L lower option */
void VSC_OPTN_LowerM2LOptions_SetDefault(
    IN OUT VSC_OPTN_LowerM2LOptions* options,
    IN gctUINT optLevel
    )
{
    VSC_OPTN_LowerM2LOptions_SetSwitchOn(options, gcvTRUE);
    VSC_OPTN_LowerM2LOptions_SetTrace(options, 0);
}

void VSC_OPTN_LowerM2LOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_LowerM2LOptions* options
    )
{
    while (str[0] == ':')
    {
        ++str;
        if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "on", sizeof("on") - 1))
        {
            VSC_OPTN_LowerM2LOptions_SetSwitchOn(options, gcvTRUE);
            str += sizeof("on") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "off", sizeof("off") - 1))
        {
            VSC_OPTN_LowerM2LOptions_SetSwitchOn(options, gcvFALSE);
            str += sizeof("off") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "trace:", sizeof("trace:") - 1))
        {
            gctUINT32 trace;
            gctUINT32 len;

            str += sizeof("trace:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            trace = vscSTR_StrToUint32(str, len);
            VSC_OPTN_LowerM2LOptions_SetTrace(options, trace);
            str += len;
        }
    }
}

void VSC_OPTN_LowerM2LOptions_Dump(
    IN VSC_OPTN_LowerM2LOptions* options,
    IN VIR_Dumper* dumper
    )
{
    VIR_LOG(dumper, "lowering from mid level to low level options:\n");
    VIR_LOG(dumper, "    on: %s\n", VSC_OPTN_LowerM2LOptions_GetSwitchOn(options) ? "true" : "false");
    VIR_LOG(dumper, "    trace: %x\n", VSC_OPTN_LowerM2LOptions_GetTrace(options));
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_LowerM2LOptions_Usage(
    IN VIR_Dumper* dumper
    )
{
    gctSTRING usage =
        "-LOWERM2L:\n"
        "    on              turn on lowering from mid level to low level\n"
        "    off             turn off lowering from mid level to low level\n"
        "    trace:          0x1     input function\n"
        "                    0x2     input cfg\n"
        "                    0x4     input bb\n"
        "                    0x8     dag\n"
        "                    0x10    heuristic\n"
        "                    0x20    output bb\n"
        "                    0x40    output cfg\n";
    VIR_LOG(dumper, usage);
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_SCLOptions_SetDefault(
    IN OUT VSC_OPTN_SCLOptions* options,
    IN gctUINT optLevel
    )
{
    if(optLevel >= 1)
    {
        VSC_OPTN_SCLOptions_SetSwitchOn(options, gcvFALSE);
    }
    else
    {
        VSC_OPTN_SCLOptions_SetSwitchOn(options, gcvFALSE);
    }
    VSC_OPTN_SCLOptions_SetTrace(options, 0);
}

void VSC_OPTN_SCLOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_SCLOptions* options
    )
{
    while (str[0] == ':')
    {
        ++str;
        if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "on", sizeof("on") - 1))
        {
            VSC_OPTN_SCLOptions_SetSwitchOn(options, gcvTRUE);
            str += sizeof("on") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "off", sizeof("off") - 1))
        {
            VSC_OPTN_SCLOptions_SetSwitchOn(options, gcvFALSE);
            str += sizeof("off") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "trace:", sizeof("trace:") - 1))
        {
            gctUINT32 trace;
            gctUINT32 len;

            str += sizeof("trace:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            trace = vscSTR_StrToUint32(str, len);
            VSC_OPTN_SCLOptions_SetTrace(options, trace);
            str += len;
        }
    }
}

void VSC_OPTN_SCLOptions_Dump(
    IN VSC_OPTN_SCLOptions* options,
    IN VIR_Dumper* dumper
    )
{
    VIR_LOG(dumper, "scalarization options:\n");
    VIR_LOG(dumper, "    on: %s\n", VSC_OPTN_SCLOptions_GetSwitchOn(options) ? "true" : "false");
    VIR_LOG(dumper, "    trace: %x\n", VSC_OPTN_SCLOptions_GetTrace(options));
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_SCLOptions_Usage(
    IN VIR_Dumper* dumper
    )
{
    gctSTRING usage =
        "-SCL:\n"
        "    on                  turn on scalarization\n"
        "    off                 turn off scalarization\n"
        "    trace:              0x1     input function\n"
        "                        0x2     input cfg\n"
        "                        0x4     input bb\n"
        "                        0x8     dag\n"
        "                        0x10    heuristic\n"
        "                        0x20    output bb\n"
        "                        0x40    output cfg\n";
    VIR_LOG(dumper, usage);
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_PHOptions_SetDefault(
    IN OUT VSC_OPTN_PHOptions* options,
    IN gctUINT optLevel
    )
{
    gctUINT  option = VSC_OPTN_PHOptions_OPTS_MODIFIER |
                      VSC_OPTN_PHOptions_OPTS_MAD |
                      VSC_OPTN_PHOptions_OPTS_RSQ |
                      VSC_OPTN_PHOptions_OPTS_RUC |
                      VSC_OPTN_PHOptions_OPTS_MOV_LDARR |
                      VSC_OPTN_PHOptions_OPTS_LSHIFT_LS;

    if(optLevel >= 2)
    {
        VSC_OPTN_PHOptions_SetSwitchOn(options, gcvTRUE);
    }
    else
    {
        VSC_OPTN_PHOptions_SetSwitchOn(options, gcvFALSE);
    }
    VSC_OPTN_PHOptions_SetOPTS(options, option);
    VSC_OPTN_PHOptions_SetModifiers(options, 0xff);
    VSC_OPTN_PHOptions_SetTrace(options, 0);
    VSC_OPTN_PHOptions_SetBeforeShader(options, gcvMAXUINT32);
    VSC_OPTN_PHOptions_SetAfterShader(options, gcvMAXUINT32);
    VSC_OPTN_PHOptions_SetBeforeFunc(options, gcvMAXUINT32);
    VSC_OPTN_PHOptions_SetAfterFunc(options, gcvMAXUINT32);
    VSC_OPTN_PHOptions_SetBeforeBB(options, gcvMAXUINT32);
    VSC_OPTN_PHOptions_SetAfterBB(options, gcvMAXUINT32);
    VSC_OPTN_PHOptions_SetBeforeInst(options, gcvMAXUINT32);
    VSC_OPTN_PHOptions_SetAfterInst(options, gcvMAXUINT32);
}

void VSC_OPTN_PHOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_PHOptions* options
    )
{
    while (str[0] == ':')
    {
        ++str;
        if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "on", sizeof("on") - 1))
        {
            VSC_OPTN_PHOptions_SetSwitchOn(options, gcvTRUE);
            str += sizeof("on") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "off", sizeof("off") - 1))
        {
            VSC_OPTN_PHOptions_SetSwitchOn(options, gcvFALSE);
            str += sizeof("off") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "opts:", sizeof("opts:") - 1))
        {
            gctUINT32 opts;
            gctUINT32 len;

            str += sizeof("opts:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            opts = vscSTR_StrToUint32(str, len);
            VSC_OPTN_PHOptions_SetOPTS(options, opts);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "modifiers:", sizeof("modifiers:") - 1))
        {
            gctUINT32 modifiers;
            gctUINT32 len;

            str += sizeof("modifiers:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            modifiers = vscSTR_StrToUint32(str, len);
            VSC_OPTN_PHOptions_SetModifiers(options, modifiers);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "trace:", sizeof("trace:") - 1))
        {
            gctUINT32 trace;
            gctUINT32 len;

            str += sizeof("trace:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            trace = vscSTR_StrToUint32(str, len);
            VSC_OPTN_PHOptions_SetTrace(options, trace);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "bs:", sizeof("bs:") - 1))
        {
            gctUINT32 before_shader;
            gctUINT32 len;

            str += sizeof("bs:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            before_shader = vscSTR_StrToUint32(str, len);
            VSC_OPTN_PHOptions_SetBeforeShader(options, before_shader);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "as:", sizeof("as:") - 1))
        {
            gctUINT32 after_shader;
            gctUINT32 len;

            str += sizeof("as:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            after_shader = vscSTR_StrToUint32(str, len);
            VSC_OPTN_PHOptions_SetAfterShader(options, after_shader);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "bf:", sizeof("bf:") - 1))
        {
            gctUINT32 before_func;
            gctUINT32 len;

            str += sizeof("bf:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            before_func = vscSTR_StrToUint32(str, len);
            VSC_OPTN_PHOptions_SetBeforeFunc(options, before_func);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "af:", sizeof("af:") - 1))
        {
            gctUINT32 after_func;
            gctUINT32 len;

            str += sizeof("af:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            after_func = vscSTR_StrToUint32(str, len);
            VSC_OPTN_PHOptions_SetAfterFunc(options, after_func);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "bb:", sizeof("bb:") - 1))
        {
            gctUINT32 before_bb;
            gctUINT32 len;

            str += sizeof("bb:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            before_bb = vscSTR_StrToUint32(str, len);
            VSC_OPTN_PHOptions_SetBeforeBB(options, before_bb);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "ab:", sizeof("ab:") - 1))
        {
            gctUINT32 after_bb;
            gctUINT32 len;

            str += sizeof("ab:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            after_bb = vscSTR_StrToUint32(str, len);
            VSC_OPTN_PHOptions_SetAfterBB(options, after_bb);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "bi:", sizeof("bi:") - 1))
        {
            gctUINT32 before_inst;
            gctUINT32 len;

            str += sizeof("bi:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            before_inst = vscSTR_StrToUint32(str, len);
            VSC_OPTN_PHOptions_SetBeforeInst(options, before_inst);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "ai:", sizeof("ai:") - 1))
        {
            gctUINT32 after_inst;
            gctUINT32 len;

            str += sizeof("ai:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            after_inst = vscSTR_StrToUint32(str, len);
            VSC_OPTN_PHOptions_SetAfterInst(options, after_inst);
            str += len;
        }
    }
}

void VSC_OPTN_PHOptions_Dump(
    IN VSC_OPTN_PHOptions* options,
    IN VIR_Dumper* dumper
    )
{
    VIR_LOG(dumper, "peephole options:\n");
    VIR_LOG(dumper, "    on: %s\n", VSC_OPTN_PHOptions_GetSwitchOn(options) ? "true" : "false");
    VIR_LOG(dumper, "    opts: 0x%x\n", VSC_OPTN_PHOptions_GetOPTS(options));
    VIR_LOG(dumper, "    modifiers: 0x%x\n", VSC_OPTN_PHOptions_GetModifiers(options));
    VIR_LOG(dumper, "    trace: 0x%x\n", VSC_OPTN_PHOptions_GetTrace(options));
    VIR_LOG(dumper, "    bs: %d\n", VSC_OPTN_PHOptions_GetBeforeShader(options));
    VIR_LOG(dumper, "    as: %d\n", VSC_OPTN_PHOptions_GetAfterShader(options));
    VIR_LOG(dumper, "    bf: %d\n", VSC_OPTN_PHOptions_GetBeforeFunc(options));
    VIR_LOG(dumper, "    af: %d\n", VSC_OPTN_PHOptions_GetAfterFunc(options));
    VIR_LOG(dumper, "    bb: %d\n", VSC_OPTN_PHOptions_GetBeforeBB(options));
    VIR_LOG(dumper, "    ab: %d\n", VSC_OPTN_PHOptions_GetAfterBB(options));
    VIR_LOG(dumper, "    bi: %d\n", VSC_OPTN_PHOptions_GetBeforeInst(options));
    VIR_LOG(dumper, "    ai: %d\n", VSC_OPTN_PHOptions_GetAfterInst(options));
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_PHOptions_Usage(
    IN VIR_Dumper* dumper
    )
{
    gctSTRING usage =
        "-PH:\n"
        "    on                  turn on peephole opt\n"
        "    off                 turn off peephole opt\n"
        "    opts:               0x1     generate operand modifiers\n"
        "                        0x2     merge MUL/ADD instructions and generate MAD instruction\n"
        "                        0x4     vectorization\n"
        "                        0x8     remove instruction whose def is not used\n"
        "    modifiers:          0x1     generate SAT modifer\n"
        "                        0x2     generate NEG modifer\n"
        "                        0x4     generate ABS modifer\n"
        "    trace:              0x1     input cfg\n"
        "                        0x2     input bb\n"
        "                        0x4     modifiers\n"
        "                        0x8     MAD\n"
        "                        0x20    output bb\n"
        "                        0x40    output cfg\n"
        "    bs:                 before shader\n"
        "    as:                 after shader\n"
        "    bf:                 before function\n"
        "    af:                 after function\n"
        "    bb:                 before basic block\n"
        "    ab:                 after basic block\n"
        "    bi:                 before instruction\n"
        "    ai:                 after instruction\n";
    VIR_LOG(dumper, usage);
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_SIMPOptions_SetDefault(
    IN OUT VSC_OPTN_SIMPOptions* options,
    IN gctUINT optLevel
    )
{
    if(optLevel >= 2)
    {
        VSC_OPTN_SIMPOptions_SetSwitchOn(options, gcvTRUE);
    }
    else
    {
        VSC_OPTN_SIMPOptions_SetSwitchOn(options, gcvFALSE);
    }
    VSC_OPTN_SIMPOptions_SetTrace(options, 0);
    VSC_OPTN_SIMPOptions_SetBeforeShader(options, gcvMAXUINT32);
    VSC_OPTN_SIMPOptions_SetAfterShader(options, gcvMAXUINT32);
    VSC_OPTN_SIMPOptions_SetBeforeFunc(options, gcvMAXUINT32);
    VSC_OPTN_SIMPOptions_SetAfterFunc(options, gcvMAXUINT32);
    VSC_OPTN_SIMPOptions_SetBeforeInst(options, gcvMAXUINT32);
    VSC_OPTN_SIMPOptions_SetAfterInst(options, gcvMAXUINT32);
}

void VSC_OPTN_SIMPOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_SIMPOptions* options
    )
{
    while (str[0] == ':')
    {
        ++str;
        if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "on", sizeof("on") - 1))
        {
            VSC_OPTN_SIMPOptions_SetSwitchOn(options, gcvTRUE);
            str += sizeof("on") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "off", sizeof("off") - 1))
        {
            VSC_OPTN_SIMPOptions_SetSwitchOn(options, gcvFALSE);
            str += sizeof("off") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "trace:", sizeof("trace:") - 1))
        {
            gctUINT32 trace;
            gctUINT32 len;

            str += sizeof("trace:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            trace = vscSTR_StrToUint32(str, len);
            VSC_OPTN_SIMPOptions_SetTrace(options, trace);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "before_shader:", sizeof("before_shader:") - 1))
        {
            gctUINT32 before_shader;
            gctUINT32 len;

            str += sizeof("before_shader:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            before_shader = vscSTR_StrToUint32(str, len);
            VSC_OPTN_SIMPOptions_SetBeforeShader(options, before_shader);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "after_shader:", sizeof("after_shader:") - 1))
        {
            gctUINT32 after_shader;
            gctUINT32 len;

            str += sizeof("after_shader:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            after_shader = vscSTR_StrToUint32(str, len);
            VSC_OPTN_SIMPOptions_SetAfterShader(options, after_shader);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "before_func:", sizeof("before_func:") - 1))
        {
            gctUINT32 before_func;
            gctUINT32 len;

            str += sizeof("before_func:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            before_func = vscSTR_StrToUint32(str, len);
            VSC_OPTN_SIMPOptions_SetBeforeFunc(options, before_func);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "after_func:", sizeof("after_func:") - 1))
        {
            gctUINT32 after_func;
            gctUINT32 len;

            str += sizeof("after_func:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            after_func = vscSTR_StrToUint32(str, len);
            VSC_OPTN_SIMPOptions_SetAfterFunc(options, after_func);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "before_inst:", sizeof("before_inst:") - 1))
        {
            gctUINT32 before_inst;
            gctUINT32 len;

            str += sizeof("before_inst:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            before_inst = vscSTR_StrToUint32(str, len);
            VSC_OPTN_SIMPOptions_SetBeforeInst(options, before_inst);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "after_inst:", sizeof("after_inst:") - 1))
        {
            gctUINT32 after_inst;
            gctUINT32 len;

            str += sizeof("after_inst:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            after_inst = vscSTR_StrToUint32(str, len);
            VSC_OPTN_SIMPOptions_SetAfterInst(options, after_inst);
            str += len;
        }
    }
}

void VSC_OPTN_SIMPOptions_Dump(
    IN VSC_OPTN_SIMPOptions* options,
    IN struct _VIR_DUMPER* dumper
    )
{}

void VSC_OPTN_SIMPOptions_Usage(
    IN struct _VIR_DUMPER* dumper
    )
{}

void VSC_OPTN_ISOptions_SetDefault(
    IN OUT VSC_OPTN_ISOptions* options,
    IN gctUINT optLevel
    )
{
    gctUINT i;

    for(i = 0; i < VSC_OPTN_IS_COUNT; i++)
    {
        VSC_OPTN_ISOptions_SetSwitchOn(options + i, gcvFALSE);
        VSC_OPTN_ISOptions_SetPassId(options + i, i);
        VSC_OPTN_ISOptions_SetIsForward(options + i, gcvTRUE);
        VSC_OPTN_ISOptions_SetLLIOnly(options + i, gcvTRUE);
        VSC_OPTN_ISOptions_SetBandwidthOnly(options + i, gcvFALSE);
        VSC_OPTN_ISOptions_SetVXOn(options + i, gcvFALSE);
        VSC_OPTN_ISOptions_SetReissue(options + i, gcvFALSE);
        VSC_OPTN_ISOptions_SetRegCount(options + i, 0);             /* 0 is invalid number here */
        VSC_OPTN_ISOptions_SetTexldCycles(options + i, 0);          /* 0 is invalid number here */
        VSC_OPTN_ISOptions_SetMemldCycles(options + i, 0);          /* 0 is invalid number here */
        VSC_OPTN_ISOptions_SetMemstCycles(options + i, 0);          /* 0 is invalid number here */
        VSC_OPTN_ISOptions_SetCacheldCycles(options + i, 0);        /* 0 is invalid number here */
        VSC_OPTN_ISOptions_SetCachestCycles(options + i, 0);        /* 0 is invalid number here */
        VSC_OPTN_ISOptions_SetBBCeiling(options + i, 512);
        VSC_OPTN_ISOptions_SetAlgorithm(options + i, /*VSC_OPTN_ISOptions_ALGORITHM_LISTSCHEDULING
                                               ,*/ VSC_OPTN_ISOptions_ALGORITHM_BUBBLESCHEDULING);
        /*VSC_OPTN_ISOptions_SetDepGran(options + i, VSC_OPTN_ISOptions_DEPGRAN_DU_PER_CHANNAL);*/
        VSC_OPTN_ISOptions_SetFwHeuristics(options + i, (
            /*VSC_OPTN_ISOptions_FW_HEUR_PREFER_RANGE |*/
            VSC_OPTN_ISOptions_FW_HEUR_PRE_PREFER_BINDING |
            VSC_OPTN_ISOptions_FW_HEUR_PRE_PREFER_KILLDEP |
            /*VSC_OPTN_ISOptions_FW_HEUR_PREFER_KILL |*/
            VSC_OPTN_ISOptions_FW_HEUR_PRE_PREFER_TEXLD |
            VSC_OPTN_ISOptions_FW_HEUR_PRE_PREFER_MEMLD |
            VSC_OPTN_ISOptions_FW_HEUR_PREFER_ANTI_BUBBLE |
            /*VSC_OPTN_ISOptions_FW_HEUR_POST_PREFER_TEXLD |
            VSC_OPTN_ISOptions_FW_HEUR_POST_PREFER_MEMLD |
            VSC_OPTN_ISOptions_FW_HEUR_DELAY_TEX_LD |
            VSC_OPTN_ISOptions_FW_HEUR_DELAY_MEM_LD |*/
            VSC_OPTN_ISOptions_FW_HEUR_POST_PREFER_TEXLDMEMLD |
            /*VSC_OPTN_ISOptions_FW_HEUR_POST_PREFER_KILLDEP |*/
            VSC_OPTN_ISOptions_FW_HEUR_PREFER_ORDER
            ));
        VSC_OPTN_ISOptions_SetBwHeuristics(options + i, (
            /*VSC_OPTN_ISOptions_BW_HEUR_PREFER_RANGE |*/
            VSC_OPTN_ISOptions_BW_HEUR_PREFER_ORDER
            ));
        VSC_OPTN_ISOptions_SetTrace(options + i, 0);
        VSC_OPTN_ISOptions_SetBeforeShader(options + i, gcvMAXUINT32);
        VSC_OPTN_ISOptions_SetAfterShader(options + i, gcvMAXUINT32);
        VSC_OPTN_ISOptions_SetBeforeFunc(options + i, gcvMAXUINT32);
        VSC_OPTN_ISOptions_SetAfterFunc(options + i, gcvMAXUINT32);
        VSC_OPTN_ISOptions_SetBeforeBB(options + i, gcvMAXUINT32);
        VSC_OPTN_ISOptions_SetAfterBB(options + i, gcvMAXUINT32);
        VSC_OPTN_ISOptions_SetBeforeInst(options + i, gcvMAXUINT32);
        VSC_OPTN_ISOptions_SetAfterInst(options + i, gcvMAXUINT32);
    }
}

void VSC_OPTN_ISOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_ISOptions* options
    )
{
    while (str[0] == ':')
    {
        ++str;
        if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "on", sizeof("on") - 1))
        {
            VSC_OPTN_ISOptions_SetSwitchOn(options, gcvTRUE);
            str += sizeof("on") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "off", sizeof("off") - 1))
        {
            VSC_OPTN_ISOptions_SetSwitchOn(options, gcvFALSE);
            str += sizeof("off") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "is_forward:on", sizeof("is_forward:on") - 1))
        {
            VSC_OPTN_ISOptions_SetIsForward(options, gcvTRUE);
            str += sizeof("is_forward:on") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "is_forward:off", sizeof("is_forward:off") - 1))
        {
            VSC_OPTN_ISOptions_SetIsForward(options, gcvFALSE);
            str += sizeof("is_forward:off") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "lli_only:on", sizeof("lli_only:on") - 1))
        {
            VSC_OPTN_ISOptions_SetLLIOnly(options, gcvTRUE);
            str += sizeof("lli_only:on") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "lli_only:off", sizeof("lli_only:off") - 1))
        {
            VSC_OPTN_ISOptions_SetLLIOnly(options, gcvFALSE);
            str += sizeof("lli_only:off") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "bandwidth_only:on", sizeof("bandwidth_only:on") - 1))
        {
            VSC_OPTN_ISOptions_SetBandwidthOnly(options, gcvTRUE);
            str += sizeof("bandwidth_only:on") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "bandwidth_only:off", sizeof("bandwidth_only:off") - 1))
        {
            VSC_OPTN_ISOptions_SetBandwidthOnly(options, gcvFALSE);
            str += sizeof("bandwidth_only:off") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "vx_on:on", sizeof("vx_on:on") - 1))
        {
            VSC_OPTN_ISOptions_SetVXOn(options, gcvTRUE);
            str += sizeof("vx_on:on") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "vx_on:off", sizeof("vx_on:off") - 1))
        {
            VSC_OPTN_ISOptions_SetVXOn(options, gcvFALSE);
            str += sizeof("vx_on:off") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "re_issue:on", sizeof("re_issue:on") - 1))
        {
            VSC_OPTN_ISOptions_SetReissue(options, gcvTRUE);
            str += sizeof("re_issue:on") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "re_issue:off", sizeof("re_issue:off") - 1))
        {
            VSC_OPTN_ISOptions_SetReissue(options, gcvFALSE);
            str += sizeof("re_issue:off") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "reg_count:", sizeof("reg_count:") - 1))
        {
            gctUINT32 reg_count;
            gctUINT32 len;

            str += sizeof("reg_count:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            reg_count = vscSTR_StrToUint32(str, len);
            VSC_OPTN_ISOptions_SetRegCount(options, reg_count);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "texld_cycles:", sizeof("texld_cycles:") - 1))
        {
            gctUINT32 texld_cycles;
            gctUINT32 len;

            str += sizeof("texld_cycles:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            texld_cycles = vscSTR_StrToUint32(str, len);
            VSC_OPTN_ISOptions_SetTexldCycles(options, texld_cycles);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "memld_cycles:", sizeof("memld_cycles:") - 1))
        {
            gctUINT32 memld_cycles;
            gctUINT32 len;

            str += sizeof("memld_cycles:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            memld_cycles = vscSTR_StrToUint32(str, len);
            VSC_OPTN_ISOptions_SetMemldCycles(options, memld_cycles);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "memst_cycles:", sizeof("memst_cycles:") - 1))
        {
            gctUINT32 memst_cycles;
            gctUINT32 len;

            str += sizeof("memst_cycles:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            memst_cycles = vscSTR_StrToUint32(str, len);
            VSC_OPTN_ISOptions_SetMemstCycles(options, memst_cycles);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "cacheld_cycles:", sizeof("cacheld_cycles:") - 1))
        {
            gctUINT32 cacheld_cycles;
            gctUINT32 len;

            str += sizeof("cacheld_cycles:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            cacheld_cycles = vscSTR_StrToUint32(str, len);
            VSC_OPTN_ISOptions_SetCacheldCycles(options, cacheld_cycles);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "cachest_cycles:", sizeof("cachest_cycles:") - 1))
        {
            gctUINT32 cachest_cycles;
            gctUINT32 len;

            str += sizeof("cachest_cycles:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            cachest_cycles = vscSTR_StrToUint32(str, len);
            VSC_OPTN_ISOptions_SetCachestCycles(options, cachest_cycles);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "bb_ceiling:", sizeof("bb_ceiling:") - 1))
        {
            gctUINT32 bb_ceiling;
            gctUINT32 len;

            str += sizeof("bb_ceiling:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            bb_ceiling = vscSTR_StrToUint32(str, len);
            VSC_OPTN_ISOptions_SetBBCeiling(options, bb_ceiling);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "algorithm:", sizeof("algorithm:") - 1))
        {
            gctUINT32 algorithm;
            gctUINT32 len;

            str += sizeof("algorithm:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            algorithm = vscSTR_StrToUint32(str, len);
            VSC_OPTN_ISOptions_SetAlgorithm(options, algorithm);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "fw_heuristics:", sizeof("fw_heuristics:") - 1))
        {
            gctUINT32 fw_heuristics;
            gctUINT32 len;

            str += sizeof("fw_heuristics:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            fw_heuristics = vscSTR_StrToUint32(str, len);
            VSC_OPTN_ISOptions_SetFwHeuristics(options, fw_heuristics);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "bw_heuristics:", sizeof("bw_heuristics:") - 1))
        {
            gctUINT32 bw_heuristics;
            gctUINT32 len;

            str += sizeof("bw_heuristics:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            bw_heuristics = vscSTR_StrToUint32(str, len);
            VSC_OPTN_ISOptions_SetBwHeuristics(options, bw_heuristics);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "trace:", sizeof("trace:")-1))
        {
            gctUINT32 trace;
            gctUINT32 len;

            str += sizeof("trace:") -1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            trace = vscSTR_StrToUint32(str, len);
            VSC_OPTN_ISOptions_SetTrace(options, trace);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "before_shader:", sizeof("before_shader:") - 1))
        {
            gctUINT32 before_shader;
            gctUINT32 len;

            str += sizeof("before_shader:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            before_shader = vscSTR_StrToUint32(str, len);
            VSC_OPTN_ISOptions_SetBeforeShader(options, before_shader);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "bs:", sizeof("bs:") - 1))
        {
            gctUINT32 before_shader;
            gctUINT32 len;

            str += sizeof("bs:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            before_shader = vscSTR_StrToUint32(str, len);
            VSC_OPTN_ISOptions_SetBeforeShader(options, before_shader);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "after_shader:", sizeof("after_shader:") - 1))
        {
            gctUINT32 after_shader;
            gctUINT32 len;

            str += sizeof("after_shader:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            after_shader = vscSTR_StrToUint32(str, len);
            VSC_OPTN_ISOptions_SetAfterShader(options, after_shader);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "as:", sizeof("as:") - 1))
        {
            gctUINT32 after_shader;
            gctUINT32 len;

            str += sizeof("as:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            after_shader = vscSTR_StrToUint32(str, len);
            VSC_OPTN_ISOptions_SetAfterShader(options, after_shader);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "before_func:", sizeof("before_func:") - 1))
        {
            gctUINT32 before_func;
            gctUINT32 len;

            str += sizeof("before_func:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            before_func = vscSTR_StrToUint32(str, len);
            VSC_OPTN_ISOptions_SetBeforeFunc(options, before_func);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "bf:", sizeof("bf:") - 1))
        {
            gctUINT32 before_func;
            gctUINT32 len;

            str += sizeof("bf:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            before_func = vscSTR_StrToUint32(str, len);
            VSC_OPTN_ISOptions_SetBeforeFunc(options, before_func);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "after_func:", sizeof("after_func:") - 1))
        {
            gctUINT32 after_func;
            gctUINT32 len;

            str += sizeof("after_func:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            after_func = vscSTR_StrToUint32(str, len);
            VSC_OPTN_ISOptions_SetAfterFunc(options, after_func);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "af:", sizeof("af:") - 1))
        {
            gctUINT32 after_func;
            gctUINT32 len;

            str += sizeof("af:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            after_func = vscSTR_StrToUint32(str, len);
            VSC_OPTN_ISOptions_SetAfterFunc(options, after_func);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "before_bb:", sizeof("before_bb:") - 1))
        {
            gctUINT32 before_bb;
            gctUINT32 len;

            str += sizeof("before_bb:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            before_bb = vscSTR_StrToUint32(str, len);
            VSC_OPTN_ISOptions_SetBeforeBB(options, before_bb);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "bb:", sizeof("bb:") - 1))
        {
            gctUINT32 before_bb;
            gctUINT32 len;

            str += sizeof("bb:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            before_bb = vscSTR_StrToUint32(str, len);
            VSC_OPTN_ISOptions_SetBeforeBB(options, before_bb);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "after_bb:", sizeof("after_bb:") - 1))
        {
            gctUINT32 after_bb;
            gctUINT32 len;

            str += sizeof("after_bb:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            after_bb = vscSTR_StrToUint32(str, len);
            VSC_OPTN_ISOptions_SetAfterBB(options, after_bb);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "ab:", sizeof("ab:") - 1))
        {
            gctUINT32 after_bb;
            gctUINT32 len;

            str += sizeof("ab:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            after_bb = vscSTR_StrToUint32(str, len);
            VSC_OPTN_ISOptions_SetAfterBB(options, after_bb);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "before_inst:", sizeof("before_inst:") - 1))
        {
            gctUINT32 before_inst;
            gctUINT32 len;

            str += sizeof("before_inst:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            before_inst = vscSTR_StrToUint32(str, len);
            VSC_OPTN_ISOptions_SetBeforeInst(options, before_inst);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "bi:", sizeof("bi:") - 1))
        {
            gctUINT32 before_inst;
            gctUINT32 len;

            str += sizeof("bi:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            before_inst = vscSTR_StrToUint32(str, len);
            VSC_OPTN_ISOptions_SetBeforeInst(options, before_inst);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "after_inst:", sizeof("after_inst:") - 1))
        {
            gctUINT32 after_inst;
            gctUINT32 len;

            str += sizeof("after_inst:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            after_inst = vscSTR_StrToUint32(str, len);
            VSC_OPTN_ISOptions_SetAfterInst(options, after_inst);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "ai:", sizeof("ai:") - 1))
        {
            gctUINT32 after_inst;
            gctUINT32 len;

            str += sizeof("ai:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            after_inst = vscSTR_StrToUint32(str, len);
            VSC_OPTN_ISOptions_SetAfterInst(options, after_inst);
            str += len;
        }
    }
}

void VSC_OPTN_ISOptions_Dump(
    IN VSC_OPTN_ISOptions* options,
    IN VIR_Dumper* dumper
    )
{
    VIR_LOG(dumper, "%s instruction scheduling options:\n", VSC_OPTN_ISOptions_GetPassId(options) ? "Post RA" : "Pre RA");
    VIR_LOG(dumper, "    on: %s\n", VSC_OPTN_ISOptions_GetSwitchOn(options) ? "true" : "false");
    VIR_LOG(dumper, "    lli_only: %s\n", VSC_OPTN_ISOptions_GetLLIOnly(options) ? "true" : "false");
    VIR_LOG(dumper, "    bandwidth_only: %s\n", VSC_OPTN_ISOptions_GetBandwidthOnly(options) ? "true" : "false");
    VIR_LOG(dumper, "    reg_count: %d\n", VSC_OPTN_ISOptions_GetRegCount(options));
    VIR_LOG(dumper, "    bb ceiling: %d\n", VSC_OPTN_ISOptions_GetBBCeiling(options));
    VIR_LOG(dumper, "    fw_heuristics: 0x%x\n", VSC_OPTN_ISOptions_GetFwHeuristics(options));
    VIR_LOG(dumper, "    bw_heuristics: 0x%x\n", VSC_OPTN_ISOptions_GetBwHeuristics(options));
    VIR_LOG(dumper, "    trace: 0x%x\n", VSC_OPTN_ISOptions_GetTrace(options));
    VIR_LOG(dumper, "    before_shader: %d\n", VSC_OPTN_ISOptions_GetBeforeShader(options));
    VIR_LOG(dumper, "    after_shader: %d\n", VSC_OPTN_ISOptions_GetAfterShader(options));
    VIR_LOG(dumper, "    before_func: %d\n", VSC_OPTN_ISOptions_GetBeforeFunc(options));
    VIR_LOG(dumper, "    after_func: %d\n", VSC_OPTN_ISOptions_GetAfterFunc(options));
    VIR_LOG(dumper, "    before_bb: %d\n", VSC_OPTN_ISOptions_GetBeforeBB(options));
    VIR_LOG(dumper, "    after_bb: %d\n", VSC_OPTN_ISOptions_GetAfterBB(options));
    VIR_LOG(dumper, "    before_inst: %d\n", VSC_OPTN_ISOptions_GetBeforeInst(options));
    VIR_LOG(dumper, "    after_inst: %d\n", VSC_OPTN_ISOptions_GetAfterInst(options));
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_ISOptions_Usage(
    IN VIR_Dumper* dumper
    )
{
    gctSTRING usage =
        "-[IS|POSTRAIS]:\n"
        "    on                  turn on instruction scheduling\n"
        "    off                 turn off instruction scheduling\n"
        "    lli_only:           on      perform instruction scheduling only on BBs with Long Latency Instructions\n"
        "                        off     perform instruction scheduling on BBs with or without Long Latency Instructions\n"
        "    bandwidth_only:     on      perform instruction scheduling only on BBs which have enough instructions to fill the long latency instruction bandwidth\n"
        "                        off     perform instruction scheduling on BBs no matter the bandwidth is met or not\n"
        "    reg_count           force the count of used registers. Numbers greater than 0 are valid\n"
        "    dep_gran:           0       gross dependency, only compares symbol name or temp register number\n"
        "                        1       uses DU info and compares per channel\n"
        "    bb_ceiling:         n       when bb has more other n instructions, skip it\n"
        "    heuristics:         0x1     prefer range\n"
        "                        0x2     prefer texld\n"
        "                        0x4     prefer memld\n"
        "                        0x8     perfer anti bubble\n"
        "                        0x10    delay texld\n"
        "                        0x20    delay memld\n"
        "                        0x40    prefer kill\n"
        "                        0x80    relieve RA\n"
        "                        0x100   prefer order\n"
        "                        0x200   mcand\n"
        "                        0x400   ecnad\n"
        "                        0x800   minimum etime\n"
        "    trace:              0x1     initialization\n"
        "                        0x2     input cfg\n"
        "                        0x4     input bb\n"
        "                        0x8     dag\n"
        "                        0x10    heuristic\n"
        "                        0x20    output bb\n"
        "                        0x40    output cfg\n";

    VIR_LOG(dumper, usage);
    VIR_LOG_FLUSH(dumper);
}

/* register allocation options*/
void VSC_OPTN_RAOptions_SetDefault(
    IN OUT VSC_OPTN_RAOptions* options,
    IN gctUINT optLevel
    )
{
    VSC_OPTN_RAOptions_SetSwitchOn(options, gcvFALSE);
    VSC_OPTN_RAOptions_SetHeuristics(options, 0xffffffff);
    VSC_OPTN_RAOptions_SetTrace(options, 0);
    VSC_OPTN_RAOptions_SetOPTS(options, VSC_OPTN_RAOptions_ALLOC_REG                |
                                        VSC_OPTN_RAOptions_ALLOC_UNIFORM            |
                                        /*
                                        ** By default, use MIN LS extened.
                                        VSC_OPTN_RAOptions_MAX_LS_EXTENED_END_POINT |
                                        */
                                        VSC_OPTN_RAOptions_SPILL_DEST_OPT);
    VSC_OPTN_RAOptions_SetRegisterCount(options, 0);
    VSC_OPTN_RAOptions_SetRegWaterMark(options, 0);
    VSC_OPTN_RAOptions_SetSTBubbleSize(options, 8);
    VSC_OPTN_RAOptions_SetSpillBeforeShader(options, gcvMAXUINT32);
    VSC_OPTN_RAOptions_SetSpillAfterShader(options, gcvMAXUINT32);
}

void VSC_OPTN_RAOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_RAOptions* options
    )
{
    while (str[0] == ':')
    {
        ++str;
        if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "on", sizeof("on") - 1))
        {
            VSC_OPTN_RAOptions_SetSwitchOn(options, gcvTRUE);
            str += sizeof("on") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "off", sizeof("off") - 1))
        {
            VSC_OPTN_RAOptions_SetSwitchOn(options, gcvFALSE);
            str += sizeof("off") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "heuristics:", sizeof("heuristics:") - 1))
        {
            gctUINT32 heuristics;
            gctUINT32 len;

            str += sizeof("heuristics:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            heuristics = vscSTR_StrToUint32(str, len);
            VSC_OPTN_RAOptions_SetHeuristics(options, heuristics);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "opts:", sizeof("opts:") - 1))
        {
            gctUINT32 opts;
            gctUINT32 len;

            str += sizeof("opts:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            opts = vscSTR_StrToUint32(str, len);
            VSC_OPTN_RAOptions_SetOPTS(options, opts);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "trace:", sizeof("trace:")-1))
        {
            gctUINT32 trace;
            gctUINT32 len;

            str += sizeof("trace:") -1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            trace = vscSTR_StrToUint32(str, len);
            VSC_OPTN_RAOptions_SetTrace(options, trace);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "regs:", sizeof("regs:") - 1))
        {
            gctUINT32 registerCount;
            gctUINT32 len;

            str += sizeof("regs:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            registerCount = vscSTR_StrToUint32(str, len);
            VSC_OPTN_RAOptions_SetRegisterCount(options, registerCount);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "wm:", sizeof("wm:") - 1))
        {
            gctUINT32 registerWaterMark;
            gctUINT32 len;

            str += sizeof("wm:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            registerWaterMark = vscSTR_StrToUint32(str, len);
            VSC_OPTN_RAOptions_SetRegWaterMark(options, registerWaterMark);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "bubble:", sizeof("bubble:") - 1))
        {
            gctUINT32 stBubbleSize;
            gctUINT32 len;

            str += sizeof("bubble:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            stBubbleSize = vscSTR_StrToUint32(str, len);
            VSC_OPTN_RAOptions_SetSTBubbleSize(options, stBubbleSize);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "bs:", sizeof("bs:") - 1))
        {
            gctUINT32 beforeShader;
            gctUINT32 len;

            str += sizeof("bs:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            beforeShader = vscSTR_StrToUint32(str, len);
            VSC_OPTN_RAOptions_SetSpillBeforeShader(options, beforeShader);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "as:", sizeof("as:") - 1))
        {
            gctUINT32 afterShader;
            gctUINT32 len;

            str += sizeof("as:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            afterShader = vscSTR_StrToUint32(str, len);
            VSC_OPTN_RAOptions_SetSpillAfterShader(options, afterShader);
            str += len;
        }
    }
}

void VSC_OPTN_RAOptions_Dump(
    IN VSC_OPTN_RAOptions* options,
    IN VIR_Dumper* dumper
    )
{
    VIR_LOG(dumper, "register allocation options:\n");
    VIR_LOG(dumper, "    on: %s\n", VSC_OPTN_RAOptions_GetSwitchOn(options) ? "true" : "false");
    VIR_LOG(dumper, "    heuristics: %x\n", VSC_OPTN_RAOptions_GetHeuristics(options));
    VIR_LOG(dumper, "    opts: %x\n", VSC_OPTN_RAOptions_GetOPTS(options));
    VIR_LOG(dumper, "    trace: %x\n", VSC_OPTN_RAOptions_GetTrace(options));
    VIR_LOG(dumper, "    registerCount: %d\n", VSC_OPTN_RAOptions_GetRegisterCount(options));
    VIR_LOG(dumper, "    register water mark: %d\n", VSC_OPTN_RAOptions_GetRegWaterMark(options));
    VIR_LOG(dumper, "    st bubble size: %d\n", VSC_OPTN_RAOptions_GetSTBubbleSize(options));
    VIR_LOG(dumper, "    bs: %d\n", VSC_OPTN_RAOptions_GetSpillBeforeShader(options));
    VIR_LOG(dumper, "    as: %d\n", VSC_OPTN_RAOptions_GetSpillAfterShader(options));
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_RAOptions_Usage(
    IN VIR_Dumper* dumper
    )
{
    gctSTRING usage =
        "-RA:\n"
        "    on                  turn on register allocation\n"
        "    off                 turn off register allocation\n"
        "    heuristics:         0x1    spill the first liverange\n"
        "    opts:               0x1    allocate virtual registers\n"
        "                        0x2    allocate const registers\n"
        "    trace:              0x1    build LR\n"
        "                        0x2    sort LR\n"
        "                        0x4    assign color\n"
        "                        0x8    final vir\n"
        "    regs:               set the number of registers\n"
        "    wm:                 set register water mark\n"
        "    bubble:             set bubble size for st instruction\n"
        "    bs:                 set spill shader before\n"
        "    as:                 set spill shader after\n";
    VIR_LOG(dumper, usage);
    VIR_LOG_FLUSH(dumper);
}

/* copy propagation options*/
void VSC_OPTN_CPPOptions_SetDefault(
    IN OUT VSC_OPTN_CPPOptions* options,
    IN gctUINT optLevel
    )
{
    gctUINT i;

    for(i = 0; i < VSC_OPTN_CPP_COUNT; i++)
    {
        if(optLevel >= 2)
        {
            VSC_OPTN_CPPOptions_SetSwitchOn(options + i, gcvTRUE);
        }
        else
        {
            VSC_OPTN_CPPOptions_SetSwitchOn(options + i, gcvFALSE);
        }
        VSC_OPTN_CPPOptions_SetPassId(options + i, i);
        VSC_OPTN_CPPOptions_SetOPTS(options + i, 0xff);
        VSC_OPTN_CPPOptions_SetTrace(options + i, 0);
    }
}

void VSC_OPTN_CPPOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_CPPOptions  *options
    )
{
    while (str[0] == ':')
    {
        ++str;
        if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "on", sizeof("on") - 1))
        {
            VSC_OPTN_CPPOptions_SetSwitchOn(options, gcvTRUE);
            str += sizeof("on") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "off", sizeof("off") - 1))
        {
            VSC_OPTN_CPPOptions_SetSwitchOn(options, gcvFALSE);
            str += sizeof("off") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "opts:", sizeof("opts:") - 1))
        {
            gctUINT32 opts;
            gctUINT32 len;

            str += sizeof("opts:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            opts = vscSTR_StrToUint32(str, len);
            VSC_OPTN_CPPOptions_SetOPTS(options, opts);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "trace:", sizeof("trace:")-1))
        {
            gctUINT32 trace;
            gctUINT32 len;

            str += sizeof("trace:") -1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            trace = vscSTR_StrToUint32(str, len);
            VSC_OPTN_CPPOptions_SetTrace(options, trace);
            str += len;
        }
    }
}

void VSC_OPTN_CPPOptions_Dump(
    IN VSC_OPTN_CPPOptions  *options,
    IN VIR_Dumper           *dumper
    )
{
    VIR_LOG(dumper, "copy propagation options:\n");
    VIR_LOG(dumper, "    on: %s\n", VSC_OPTN_CPPOptions_GetSwitchOn(options) ? "true" : "false");
    VIR_LOG(dumper, "    opts: 0x%x\n", VSC_OPTN_CPPOptions_GetOPTS(options));
    VIR_LOG(dumper, "    trace: %x\n", VSC_OPTN_CPPOptions_GetTrace(options));
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_CPPOptions_Usage(
    IN VIR_Dumper   *dumper
    )
{
    gctSTRING usage =
        "-CPP:\n"
        "    on                  turn on copy propagation\n"
        "    off                 turn off copy propagation\n"
        "    opts:               0x1    forward copy propagation\n"
        "                        0x2    backward copy propagation\n"
        "    trace:              0x1    trace copy propagation input\n"
        "                        0x2    trace copy propagation output\n"
        "                        0x4    trace forward copy propagation\n"
        "                        0x8    trace backward copy propagation\n\n";

    VIR_LOG(dumper, usage);
    VIR_LOG_FLUSH(dumper);
}

/* constant propagation and folding options*/
void VSC_OPTN_CPFOptions_SetDefault(
    IN OUT VSC_OPTN_CPFOptions* options,
    IN gctUINT optLevel
    )
{
    if(optLevel >= 2)
    {
        VSC_OPTN_CPFOptions_SetSwitchOn(options, gcvTRUE);
    }
    else
    {
        VSC_OPTN_CPFOptions_SetSwitchOn(options, gcvFALSE);
    }
    VSC_OPTN_CPFOptions_SetBeforeShader(options, gcvMAXUINT32);
    VSC_OPTN_CPFOptions_SetAfterShader(options, gcvMAXUINT32);
    VSC_OPTN_CPFOptions_SetBeforeFunc(options, gcvMAXUINT32);
    VSC_OPTN_CPFOptions_SetAfterFunc(options, gcvMAXUINT32);
    VSC_OPTN_CPFOptions_SetTrace(options, 0);
}

void VSC_OPTN_CPFOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_CPFOptions  *options
    )
{
    while (str[0] == ':')
    {
        ++str;
        if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "on", sizeof("on") - 1))
        {
            VSC_OPTN_CPFOptions_SetSwitchOn(options, gcvTRUE);
            str += sizeof("on") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "off", sizeof("off") - 1))
        {
            VSC_OPTN_CPFOptions_SetSwitchOn(options, gcvFALSE);
            str += sizeof("off") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "bs:", sizeof("bs:") - 1))
        {
            gctUINT32 before_shader;
            gctUINT32 len;

            str += sizeof("bs:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            before_shader = vscSTR_StrToUint32(str, len);
            VSC_OPTN_CPFOptions_SetBeforeShader(options, before_shader);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "as:", sizeof("as:") - 1))
        {
            gctUINT32 after_shader;
            gctUINT32 len;

            str += sizeof("as:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            after_shader = vscSTR_StrToUint32(str, len);
            VSC_OPTN_CPFOptions_SetAfterShader(options, after_shader);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "bf:", sizeof("bf:") - 1))
        {
            gctUINT32 before_shader;
            gctUINT32 len;

            str += sizeof("bf:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            before_shader = vscSTR_StrToUint32(str, len);
            VSC_OPTN_CPFOptions_SetBeforeFunc(options, before_shader);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "af:", sizeof("af:") - 1))
        {
            gctUINT32 after_shader;
            gctUINT32 len;

            str += sizeof("af:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            after_shader = vscSTR_StrToUint32(str, len);
            VSC_OPTN_CPFOptions_SetAfterFunc(options, after_shader);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "trace:", sizeof("trace:")-1))
        {
            gctUINT32 trace;
            gctUINT32 len;

            str += sizeof("trace:") -1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            trace = vscSTR_StrToUint32(str, len);
            VSC_OPTN_CPFOptions_SetTrace(options, trace);
            str += len;
        }
    }
}

void VSC_OPTN_CPFOptions_Dump(
    IN VSC_OPTN_CPFOptions  *options,
    IN VIR_Dumper           *dumper
    )
{
    VIR_LOG(dumper, "copy propagation options:\n");
    VIR_LOG(dumper, "    on: %s\n", VSC_OPTN_CPFOptions_GetSwitchOn(options) ? "true" : "false");
    VIR_LOG(dumper, "    bs: %d\n", VSC_OPTN_CPFOptions_GetBeforeShader(options));
    VIR_LOG(dumper, "    as: %d\n", VSC_OPTN_CPFOptions_GetAfterShader(options));
    VIR_LOG(dumper, "    bf: %d\n", VSC_OPTN_CPFOptions_GetBeforeFunc(options));
    VIR_LOG(dumper, "    af: %d\n", VSC_OPTN_CPFOptions_GetAfterFunc(options));
    VIR_LOG(dumper, "    trace: %x\n", VSC_OPTN_CPFOptions_GetTrace(options));
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_CPFOptions_Usage(
    IN VIR_Dumper   *dumper
    )
{
    gctSTRING usage =
        "-CPF:\n"
        "    on                  turn on constant propagation and folding\n"
        "    off                 turn off constant propagation and folding\n"
        "    trace:              0x1    trace constant propagation and folding input\n"
        "                        0x2    trace constant propagation and folding output\n"
        "                        0x4    trace constant propagation and folding algorithm\n\n";

    VIR_LOG(dumper, usage);
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_VECOptions_SetDefault(
    IN OUT VSC_OPTN_VECOptions* options,
    IN gctUINT optLevel
    )
{
    if(optLevel >= 2)
    {
        VSC_OPTN_VECOptions_SetSwitchOn(options, gcvTRUE);
    }
    else
    {
        VSC_OPTN_VECOptions_SetSwitchOn(options, gcvFALSE);
    }
    VSC_OPTN_VECOptions_SetBeforeShader(options, gcvMAXUINT32);
    VSC_OPTN_VECOptions_SetAfterShader(options, gcvMAXUINT32);
}

void VSC_OPTN_VECOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_VECOptions* options
    )
{
    while (str[0] == ':')
    {
        ++str;
        if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "on", sizeof("on") - 1))
        {
            VSC_OPTN_VECOptions_SetSwitchOn(options, gcvTRUE);
            str += sizeof("on") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "off", sizeof("off") - 1))
        {
            VSC_OPTN_VECOptions_SetSwitchOn(options, gcvFALSE);
            str += sizeof("off") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "bs:", sizeof("bs:") - 1))
        {
            gctUINT32 before_shader;
            gctUINT32 len;

            str += sizeof("bs:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            before_shader = vscSTR_StrToUint32(str, len);
            VSC_OPTN_VECOptions_SetBeforeShader(options, before_shader);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "as:", sizeof("as:") - 1))
        {
            gctUINT32 after_shader;
            gctUINT32 len;

            str += sizeof("as:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            after_shader = vscSTR_StrToUint32(str, len);
            VSC_OPTN_VECOptions_SetAfterShader(options, after_shader);
            str += len;
        }
    }
}

/* constant propagation and folding options*/
void VSC_OPTN_LCSEOptions_SetDefault(
    IN OUT VSC_OPTN_LCSEOptions* options,
    IN gctUINT optLevel
    )
{
    if(optLevel >= 2)
    {
        VSC_OPTN_LCSEOptions_SetSwitchOn(options, gcvTRUE);
    }
    else
    {
        VSC_OPTN_LCSEOptions_SetSwitchOn(options, gcvFALSE);
    }
    VSC_OPTN_LCSEOptions_SetOpts(options, VSC_OPTN_LCSEOptions_OPT_LOAD | VSC_OPTN_LCSEOptions_OPT_OTHERS);
    VSC_OPTN_LCSEOptions_SetBeforeShader(options, gcvMAXUINT32);
    VSC_OPTN_LCSEOptions_SetAfterShader(options, gcvMAXUINT32);
    VSC_OPTN_LCSEOptions_SetBeforeFunc(options, gcvMAXUINT32);
    VSC_OPTN_LCSEOptions_SetAfterFunc(options, gcvMAXUINT32);
    VSC_OPTN_LCSEOptions_SetTrace(options, 0);
}

void VSC_OPTN_LCSEOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_LCSEOptions  *options
    )
{
    while (str[0] == ':')
    {
        ++str;
        if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "on", sizeof("on") - 1))
        {
            VSC_OPTN_LCSEOptions_SetSwitchOn(options, gcvTRUE);
            str += sizeof("on") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "off", sizeof("off") - 1))
        {
            VSC_OPTN_LCSEOptions_SetSwitchOn(options, gcvFALSE);
            str += sizeof("off") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "opts:", sizeof("opts:") - 1))
        {
            gctUINT32 opts;
            gctUINT32 len;

            str += sizeof("opts:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            opts = vscSTR_StrToUint32(str, len);
            VSC_OPTN_LCSEOptions_SetOpts(options, opts);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "bs:", sizeof("bs:") - 1))
        {
            gctUINT32 before_shader;
            gctUINT32 len;

            str += sizeof("bs:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            before_shader = vscSTR_StrToUint32(str, len);
            VSC_OPTN_LCSEOptions_SetBeforeShader(options, before_shader);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "as:", sizeof("as:") - 1))
        {
            gctUINT32 after_shader;
            gctUINT32 len;

            str += sizeof("as:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            after_shader = vscSTR_StrToUint32(str, len);
            VSC_OPTN_LCSEOptions_SetAfterShader(options, after_shader);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "bf:", sizeof("bf:") - 1))
        {
            gctUINT32 before_shader;
            gctUINT32 len;

            str += sizeof("bf:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            before_shader = vscSTR_StrToUint32(str, len);
            VSC_OPTN_LCSEOptions_SetBeforeFunc(options, before_shader);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "af:", sizeof("af:") - 1))
        {
            gctUINT32 after_shader;
            gctUINT32 len;

            str += sizeof("af:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            after_shader = vscSTR_StrToUint32(str, len);
            VSC_OPTN_LCSEOptions_SetAfterFunc(options, after_shader);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "trace:", sizeof("trace:")-1))
        {
            gctUINT32 trace;
            gctUINT32 len;

            str += sizeof("trace:") -1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            trace = vscSTR_StrToUint32(str, len);
            VSC_OPTN_LCSEOptions_SetTrace(options, trace);
            str += len;
        }
    }
}

void VSC_OPTN_LCSEOptions_Dump(
    IN VSC_OPTN_LCSEOptions *options,
    IN VIR_Dumper           *dumper
    )
{
    VIR_LOG(dumper, "LCSE options:\n");
    VIR_LOG(dumper, "    on:   %s\n", VSC_OPTN_LCSEOptions_GetSwitchOn(options) ? "true" : "false");
    VIR_LOG(dumper, "    opts: %x\n", VSC_OPTN_LCSEOptions_GetOpts(options));
    VIR_LOG(dumper, "    bs:   %d\n", VSC_OPTN_LCSEOptions_GetBeforeShader(options));
    VIR_LOG(dumper, "    as:   %d\n", VSC_OPTN_LCSEOptions_GetAfterShader(options));
    VIR_LOG(dumper, "    bf:   %d\n", VSC_OPTN_LCSEOptions_GetBeforeFunc(options));
    VIR_LOG(dumper, "    af:   %d\n", VSC_OPTN_LCSEOptions_GetAfterFunc(options));
    VIR_LOG(dumper, "    trace: %x\n", VSC_OPTN_LCSEOptions_GetTrace(options));
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_LCSEOptions_Usage(
    IN VIR_Dumper   *dumper
    )
{
    gctSTRING usage =
        "-LCSE:\n"
        "    on                  turn on local common subexpression elimination\n"
        "    off                 turn off local common subexpression elimination\n"
        "    opts                0x1    ld lcse\n"
        "                        0x2    attr_ld lcse\n"
        "                        0x4    other expressions lcse\n"
        "    trace:              0x1    trace local common subexpression elimination input\n"
        "                        0x2    trace local common subexpression elimination output\n"
        "                        0x4    trace local common subexpression elimination algorithm\n\n";

    VIR_LOG(dumper, usage);
    VIR_LOG_FLUSH(dumper);
}

/* dead code elimination */
void VSC_OPTN_DCEOptions_SetDefault(
    IN OUT VSC_OPTN_DCEOptions* options,
    IN gctUINT optLevel
    )
{
    gctUINT i;

    for(i = 0; i < VSC_OPTN_DCE_COUNT; i++)
    {
        if(optLevel >= 2)
        {
            VSC_OPTN_DCEOptions_SetSwitchOn(options + i, gcvTRUE);
        }
        else
        {
            VSC_OPTN_DCEOptions_SetSwitchOn(options + i, gcvFALSE);
        }
        VSC_OPTN_DCEOptions_SetPassId(options + i, gcvTRUE);
        VSC_OPTN_DCEOptions_SetOPTS(options + i, 0x1);
        VSC_OPTN_DCEOptions_SetTrace(options + i, 0);
    }
}

void VSC_OPTN_DCEOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_DCEOptions  *options
    )
{
    while (str[0] == ':')
    {
        ++str;
        if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "on", sizeof("on") - 1))
        {
            VSC_OPTN_CPPOptions_SetSwitchOn(options, gcvTRUE);
            str += sizeof("on") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "off", sizeof("off") - 1))
        {
            VSC_OPTN_CPPOptions_SetSwitchOn(options, gcvFALSE);
            str += sizeof("off") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "opts:", sizeof("opts:") - 1))
        {
            gctUINT32 opts;
            gctUINT32 len;

            str += sizeof("opts:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            opts = vscSTR_StrToUint32(str, len);
            VSC_OPTN_CPPOptions_SetOPTS(options, opts);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "trace:", sizeof("trace:")-1))
        {
            gctUINT32 trace;
            gctUINT32 len;

            str += sizeof("trace:") -1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            trace = vscSTR_StrToUint32(str, len);
            VSC_OPTN_CPPOptions_SetTrace(options, trace);
            str += len;
        }
    }
}

void VSC_OPTN_DCEOptions_Dump(
    IN VSC_OPTN_DCEOptions  *options,
    IN VIR_Dumper           *dumper
    )
{
    VIR_LOG(dumper, "dead code elimination options:\n");
    VIR_LOG(dumper, "    on: %s\n", VSC_OPTN_DCEOptions_GetSwitchOn(options) ? "true" : "false");
    VIR_LOG(dumper, "    opts: 0x%x\n", VSC_OPTN_DCEOptions_GetOPTS(options));
    VIR_LOG(dumper, "    trace: %x\n", VSC_OPTN_DCEOptions_GetTrace(options));
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_DCEOptions_Usage(
    IN VIR_Dumper   *dumper
    )
{
    gctSTRING usage =
        "-DCE:\n"
        "    on                  turn on dead code elimination\n"
        "    off                 turn off dead code elimination\n"
        "    opts:               0x1    control dead code elimination\n"
        "    trace:              0x1    trace input\n"
        "                        0x2    trace worklist\n"
        "                        0x4    trace output\n"
        "                        0x8    trace analysis inst\n"
        "                        0x10   trace removed inst\n"
        "                        0x20   trace added inst\n\n";

    VIR_LOG(dumper, usage);
    VIR_LOG_FLUSH(dumper);
}

/* IO-packing */
void VSC_OPTN_IOPOptions_SetDefault(
    IN OUT VSC_OPTN_IOPOptions* options,
    IN gctUINT optLevel
    )
{
    VSC_OPTN_IOPOptions_SetSwitchOn(options, gcvFALSE);
    VSC_OPTN_IOPOptions_SetTrace(options, 0);
}

void VSC_OPTN_IOPOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_IOPOptions  *options
    )
{
    while (str[0] == ':')
    {
        ++str;
        if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "on", sizeof("on") - 1))
        {
            VSC_OPTN_IOPOptions_SetSwitchOn(options, gcvTRUE);
            str += sizeof("on") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "off", sizeof("off") - 1))
        {
            VSC_OPTN_IOPOptions_SetSwitchOn(options, gcvFALSE);
            str += sizeof("off") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "trace:", sizeof("trace:")-1))
        {
            gctUINT32 trace;
            gctUINT32 len;

            str += sizeof("trace:") -1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            trace = vscSTR_StrToUint32(str, len);
            VSC_OPTN_IOPOptions_SetTrace(options, trace);
            str += len;
        }
    }
}

void VSC_OPTN_IOPOptions_Dump(
    IN VSC_OPTN_IOPOptions  *options,
    IN VIR_Dumper           *dumper
    )
{
    VIR_LOG(dumper, "IO-packing options:\n");
    VIR_LOG(dumper, "    on: %s\n", VSC_OPTN_IOPOptions_GetSwitchOn(options) ? "true" : "false");
    VIR_LOG(dumper, "    trace: %x\n", VSC_OPTN_IOPOptions_GetTrace(options));
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_IOPOptions_Usage(
    IN VIR_Dumper   *dumper
    )
{
    VIR_LOG_FLUSH(dumper);
}

/* full active IO*/
void VSC_OPTN_FAIOOptions_SetDefault(
    IN OUT VSC_OPTN_FAIOOptions* options,
    IN gctUINT optLevel
    )
{
    VSC_OPTN_FAIOOptions_SetSwitchOn(options, gcvFALSE);
    VSC_OPTN_FAIOOptions_SetTrace(options, 0);
}

void VSC_OPTN_FAIOOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_FAIOOptions  *options
    )
{
    while (str[0] == ':')
    {
        ++str;
        if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "on", sizeof("on") - 1))
        {
            VSC_OPTN_FAIOOptions_SetSwitchOn(options, gcvTRUE);
            str += sizeof("on") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "off", sizeof("off") - 1))
        {
            VSC_OPTN_FAIOOptions_SetSwitchOn(options, gcvFALSE);
            str += sizeof("off") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "trace:", sizeof("trace:")-1))
        {
            gctUINT32 trace;
            gctUINT32 len;

            str += sizeof("trace:") -1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            trace = vscSTR_StrToUint32(str, len);
            VSC_OPTN_FAIOOptions_SetTrace(options, trace);
            str += len;
        }
    }
}

void VSC_OPTN_FAIOOptions_Dump(
    IN VSC_OPTN_FAIOOptions  *options,
    IN VIR_Dumper           *dumper
    )
{
    VIR_LOG(dumper, "full active IO options:\n");
    VIR_LOG(dumper, "    on: %s\n", VSC_OPTN_IOPOptions_GetSwitchOn(options) ? "true" : "false");
    VIR_LOG(dumper, "    trace: %x\n", VSC_OPTN_IOPOptions_GetTrace(options));
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_FAIOOptions_Usage(
    IN VIR_Dumper   *dumper
    )
{
    VIR_LOG_FLUSH(dumper);
}

/* dual16 phase options*/
void VSC_OPTN_DUAL16Options_SetDefault(
    IN OUT VSC_OPTN_DUAL16Options* options,
    IN gctUINT optLevel
    )
{
    if(optLevel >= 2)
    {
        VSC_OPTN_DUAL16Options_SetSwitchOn(options, gcvTRUE);
    }
    else
    {
        VSC_OPTN_DUAL16Options_SetSwitchOn(options, gcvFALSE);
    }
    VSC_OPTN_DUAL16Options_SetPercentage(options, (gctFLOAT)0.67);
    VSC_OPTN_DUAL16Options_SetHalfDepPercentage(options, (gctFLOAT)0.33);
    VSC_OPTN_DUAL16Options_SetTrace(options, 0);
}

void VSC_OPTN_DUAL16Options_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_DUAL16Options  *options
    )
{
    while (str[0] == ':')
    {
        ++str;
        if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "percentage:", sizeof("percentage:")-1))
        {
            gctUINT32 percentage;
            gctUINT32 len;

            str += sizeof("percentage:") -1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            percentage = vscSTR_StrToUint32(str, len);
            VSC_OPTN_DUAL16Options_SetPercentage(options, (gctFLOAT)((gctFLOAT)percentage / 100.0));
            str += len;
        }
        if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "halfDep:", sizeof("halfDep:") - 1))
        {
            gctUINT32 percentage;
            gctUINT32 len;

            str += sizeof("halfDep:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            percentage = vscSTR_StrToUint32(str, len);
            VSC_OPTN_DUAL16Options_SetHalfDepPercentage(options, (gctFLOAT)((gctFLOAT)percentage / 100.0));
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "trace:", sizeof("trace:")-1))
        {
            gctUINT32 trace;
            gctUINT32 len;

            str += sizeof("trace:") -1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            trace = vscSTR_StrToUint32(str, len);
            VSC_OPTN_DUAL16Options_SetTrace(options, trace);
            str += len;
        }
    }
}

void VSC_OPTN_DUAL16Options_Dump(
    IN VSC_OPTN_DUAL16Options  *options,
    IN VIR_Dumper           *dumper
    )
{
    VIR_LOG(dumper, "dual16 phase options:\n");
    VIR_LOG(dumper, "    trace: %x\n", VSC_OPTN_DUAL16Options_GetTrace(options));
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_DUAL16Options_Usage(
    IN VIR_Dumper   *dumper
    )
{
    gctSTRING usage =
        "-DUAL16:\n"
        "    trace:              0x1    trace input\n"
        "                        0x2    trace output\n"
        "                        0x4    trace operands\n"
        "                        0x8    trace instructions\n\n";

    VIR_LOG(dumper, usage);
    VIR_LOG_FLUSH(dumper);
}

/* final clean up phase options*/
void VSC_OPTN_FCPOptions_SetDefault(
    IN OUT VSC_OPTN_FCPOptions* options,
    IN gctUINT optLevel
    )
{
    gctUINT  option = VSC_OPTN_FCPOptions_REPL_LDARR    |
                      VSC_OPTN_FCPOptions_SPLIT_DUAL32  |
                      VSC_OPTN_FCPOptions_OPTS_ICAST;

    VSC_OPTN_FCPOptions_SetSwitchOn(options, gcvTRUE);
    VSC_OPTN_FCPOptions_SetOPTS(options, option);
    VSC_OPTN_FCPOptions_SetTrace(options, 0);
}

void VSC_OPTN_FCPOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_FCPOptions  *options
    )
{
    while (str[0] == ':')
    {
        ++str;
        if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "on", sizeof("on") - 1))
        {
            VSC_OPTN_FCPOptions_SetSwitchOn(options, gcvTRUE);
            str += sizeof("on") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "off", sizeof("off") - 1))
        {
            VSC_OPTN_FCPOptions_SetSwitchOn(options, gcvFALSE);
            str += sizeof("off") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "opts:", sizeof("opts:") - 1))
        {
            gctUINT32 opts;
            gctUINT32 len;

            str += sizeof("opts:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            opts = vscSTR_StrToUint32(str, len);
            VSC_OPTN_FCPOptions_SetOPTS(options, opts);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "trace:", sizeof("trace:")-1))
        {
            gctUINT32 trace;
            gctUINT32 len;

            str += sizeof("trace:") -1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            trace = vscSTR_StrToUint32(str, len);
            VSC_OPTN_FCPOptions_SetTrace(options, trace);
            str += len;
        }
    }
}

void VSC_OPTN_ATOMPatchOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_ATOMPatchOptions  *options
    )
{
    while (str[0] == ':')
    {
        ++str;
        if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "on", sizeof("on") - 1))
        {
            VSC_OPTN_ATOMPatchOptions_SetSwitchOn(options, gcvTRUE);
            str += sizeof("on") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "off", sizeof("off") - 1))
        {
            VSC_OPTN_ATOMPatchOptions_SetSwitchOn(options, gcvFALSE);
            str += sizeof("off") - 1;
        }
    }
}

void VSC_OPTN_ATOMPatchOptions_Usage(
    IN VIR_Dumper   *dumper
    )
{
    gctSTRING usage =
        "-ATOMPATCH:\n"
        "    on                  turn on atomic patch phase\n"
        "    off                 turn off atomic patch phase\n";

    VIR_LOG(dumper, usage);
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_FCPOptions_Dump(
    IN VSC_OPTN_FCPOptions  *options,
    IN VIR_Dumper           *dumper
    )
{
    VIR_LOG(dumper, "final clean up phase options:\n");
    VIR_LOG(dumper, "    on: %s\n", VSC_OPTN_FCPOptions_GetSwitchOn(options) ? "true" : "false");
    VIR_LOG(dumper, "    opts: 0x%x\n", VSC_OPTN_FCPOptions_GetOPTS(options));
    VIR_LOG(dumper, "    trace: %x\n", VSC_OPTN_FCPOptions_GetTrace(options));
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_FCPOptions_Usage(
    IN VIR_Dumper   *dumper
    )
{
    gctSTRING usage =
        "-FCP:\n"
        "    on                  turn on final clean up phase\n"
        "    off                 turn off final clean up phase\n"
        "    opts:               0x1    replace LDARR\n"
        "    trace:              0x1    trace input\n"
        "                        0x2    trace output\n\n";

    VIR_LOG(dumper, usage);
    VIR_LOG_FLUSH(dumper);
}

/* machine code generator */
void VSC_OPTN_MCGenOptions_SetDefault(
    IN OUT VSC_OPTN_MCGenOptions* options,
    IN gctUINT optLevel
    )
{
    VSC_OPTN_MCGenOptions_SetSwitchOn(options, gcvFALSE);
    VSC_OPTN_MCGenOptions_SetOPTS(options, 0x00);
    VSC_OPTN_MCGenOptions_SetTrace(options, 0);
}

void VSC_OPTN_MCGenOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_MCGenOptions  *options
    )
{
    while (str[0] == ':')
    {
        ++str;
        if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "on", sizeof("on") - 1))
        {
            VSC_OPTN_CPPOptions_SetSwitchOn(options, gcvTRUE);
            str += sizeof("on") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "off", sizeof("off") - 1))
        {
            VSC_OPTN_CPPOptions_SetSwitchOn(options, gcvFALSE);
            str += sizeof("off") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "opts:", sizeof("opts:") - 1))
        {
            gctUINT32 opts;
            gctUINT32 len;

            str += sizeof("opts:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            opts = vscSTR_StrToUint32(str, len);
            VSC_OPTN_CPPOptions_SetOPTS(options, opts);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "trace:", sizeof("trace:")-1))
        {
            gctUINT32 trace;
            gctUINT32 len;

            str += sizeof("trace:") -1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            trace = vscSTR_StrToUint32(str, len);
            VSC_OPTN_CPPOptions_SetTrace(options, trace);
            str += len;
        }
    }
}

void VSC_OPTN_MCGenOptions_Dump(
    IN VSC_OPTN_MCGenOptions  *options,
    IN VIR_Dumper           *dumper
    )
{
    VIR_LOG(dumper, "machine code generator options:\n");
    VIR_LOG(dumper, "    on: %s\n", VSC_OPTN_MCGenOptions_GetSwitchOn(options) ? "true" : "false");
    VIR_LOG(dumper, "    opts: 0x%x\n", VSC_OPTN_MCGenOptions_GetOPTS(options));
    VIR_LOG(dumper, "    trace: %x\n", VSC_OPTN_MCGenOptions_GetTrace(options));
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_MCGenOptions_Usage(
    IN VIR_Dumper   *dumper
    )
{
    gctSTRING usage =
    "-GEN:\n"
    "    on                  turn on machine code generator\n"
    "    off                 turn off machine code generator\n"
    "    trace:              0x1    trace output\n\n";

    VIR_LOG(dumper, usage);
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_SEPGenOptions_SetDefault(
    IN OUT VSC_OPTN_SEPGenOptions* options,
    IN gctUINT optLevel
    )
{
    VSC_OPTN_SEPGenOptions_SetTrace(options, 0);
    VSC_OPTN_SEPGenOptions_SetSwitchOn(options, gcvTRUE);
}

void VSC_OPTN_DumpOptions_SetDefault(
    IN OUT VSC_OPTN_DumpOptions* options
    )
{
    VSC_OPTN_DumpOptions_SetSwitchOn(options, gcvFALSE);
    VSC_OPTN_DumpOptions_SetDumpStart(options, 0);
    VSC_OPTN_DumpOptions_SetDumpEnd(options, 0x7fffffff);
    VSC_OPTN_DumpOptions_SetOPTS(options, VSC_OPTN_DumpOptions_DUMP_NONE);
}

void VSC_OPTN_DumpOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_DumpOptions* options
    )
{
    gctUINT32 opt = VSC_OPTN_DumpOptions_DUMP_NONE;

    while (str[0] == ':')
    {
        ++str;
        if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "SHADER", sizeof("SHADER") - 1))
        {
            opt |= VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE;
            str += sizeof("SHADER") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "OPTION", sizeof("OPTION") - 1))
        {
            opt |= VSC_OPTN_DumpOptions_DUMP_OPTION;
            str += sizeof("OPTION") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "CG", sizeof("CG") - 1))
        {
            opt |= VSC_OPTN_DumpOptions_DUMP_CG;
            str += sizeof("CG") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "ALLV", sizeof("ALLV") - 1))
        {
            opt |= VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE| VSC_OPTN_DumpOptions_DUMP_CG;
            str += sizeof("ALLV") - 1;
        }
    }
    VSC_OPTN_DumpOptions_SetOPTS(options, opt);
}

void VSC_OPTN_DumpOptions_Dump(
    IN VSC_OPTN_DumpOptions  *options,
    IN VIR_Dumper           *dumper
    )
{
    VIR_LOG(dumper, "dump options:\n");
    VIR_LOG(dumper, "    on: %s\n", VSC_OPTN_DumpOptions_GetSwitchOn(options) ? "true" : "false");
    VIR_LOG(dumper, "    opts: 0x%x\n", VSC_OPTN_DumpOptions_GetOPTS(options));
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_ILFLinkOptions_SetDefault(
    IN OUT VSC_OPTN_ILFLinkOptions* options
    )
{
    VSC_OPTN_ILFLinkOptions_SetSwitchOn(options, gcvTRUE);
}

void VSC_OPTN_UnifiedUniformOptions_SetDefault(
    IN OUT VSC_OPTN_UnifiedUniformOptions* options
    )
{
    VSC_OPTN_UnifiedUniformOptions_SetSwitchOn(options, gcvFALSE);
}

void VSC_OPTN_UnifiedUniformOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_UnifiedUniformOptions* options
    )
{
    while (str[0] == ':')
    {
        ++str;
        if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "on", sizeof("on") - 1))
        {
            VSC_OPTN_UnifiedUniformOptions_SetSwitchOn(options, gcvTRUE);
            str += sizeof("on") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "off", sizeof("off") - 1))
        {
            VSC_OPTN_UnifiedUniformOptions_SetSwitchOn(options, gcvFALSE);
            str += sizeof("off") - 1;
        }
    }
}

void VSC_OPTN_UnifiedUniformOptions_Dump(
    IN VSC_OPTN_UnifiedUniformOptions  *options,
    IN VIR_Dumper           *dumper
    )
{
    VIR_LOG(dumper, "unified uniform options:\n");
    VIR_LOG(dumper, "    on: %s\n", VSC_OPTN_UnifiedUniformOptions_GetSwitchOn(options) ? "true" : "false");
}

void VSC_OPTN_ATOMPatchOptions_SetDefault(
    IN OUT VSC_OPTN_ATOMPatchOptions* options
    )
{
    /* set default value as VSC_OPTN_ILFLinkOptions_SetDefault */
    VSC_OPTN_ATOMPatchOptions_SetSwitchOn(options, gcvTRUE);
}


static gctBOOL
_IsTriageForShaderId(
    IN gctINT           ShaderId,
    IN gctINT           StartId,
    IN gctINT           EndId
    )
{
    if ((StartId == 0 && EndId == 0) || ShaderId == 0)
        return gcvTRUE;

    if (StartId >= 0)
    {
        gcmASSERT(StartId <= EndId);  /* [21, 60 ] */
        return ShaderId >= StartId && ShaderId <= EndId;
    }
    else
    {
        gcmASSERT(EndId < 0 && StartId >= EndId);  /* [-45, -100] */
        return ShaderId < -StartId || ShaderId > -EndId;
    }
}

gctBOOL VSC_OPTN_DumpOptions_CheckDumpFlag(
    IN VSC_OPTN_DumpOptions* options,
    IN gctINT ShaderId,
    IN gctUINT DumpFlag
    )
{
    gctBOOL hasFlag = gcvFALSE;

    hasFlag = VSC_UTILS_MASK(VSC_OPTN_DumpOptions_GetOPTS(options), DumpFlag);

    if (hasFlag)
    {
        hasFlag = _IsTriageForShaderId(ShaderId,
                                       VSC_OPTN_DumpOptions_GetDumpStart(options),
                                       VSC_OPTN_DumpOptions_GetDumpEnd(options));
    }

    return hasFlag;
}

void VSC_OPTN_Options_SetDefault(
    IN OUT VSC_OPTN_Options* options,
    IN gctUINT optLevel
    )
{
    VSC_OPTN_SCPPOptions_SetDefault(VSC_OPTN_Options_GetSCPPOptions(options, 0), optLevel);
    VSC_OPTN_ParamOptOptions_SetDefault(VSC_OPTN_Options_GetPARAMOPTOptions(options, 0), optLevel);
    VSC_OPTN_LoopOptsOptions_SetDefault(VSC_OPTN_Options_GetLoopOptsOptions(options, 0), optLevel);
    VSC_OPTN_CFOOptions_SetDefault(VSC_OPTN_Options_GetCFOOptions(options, 0), optLevel);
    VSC_OPTN_UF_AUBOOptions_SetDefault(VSC_OPTN_Options_GetAUBOOptions(options, 0), optLevel);
    VSC_OPTN_ILOptions_SetDefault(VSC_OPTN_Options_GetInlinerOptions(options, 0), optLevel);
    VSC_OPTN_PUOptions_SetDefault(VSC_OPTN_Options_GetPUOptions(options, 0), optLevel);
    VSC_OPTN_LowerM2LOptions_SetDefault(VSC_OPTN_Options_GetLowerM2LOptions(options, 0), optLevel);
    VSC_OPTN_SCLOptions_SetDefault(VSC_OPTN_Options_GetSCLOptions(options, 0), optLevel);
    VSC_OPTN_PHOptions_SetDefault(VSC_OPTN_Options_GetPHOptions(options, 0), optLevel);
    VSC_OPTN_SIMPOptions_SetDefault(VSC_OPTN_Options_GetSIMPOptions(options, 0), optLevel);
    VSC_OPTN_ISOptions_SetDefault(VSC_OPTN_Options_GetISOptions(options, 0), optLevel);
    VSC_OPTN_RAOptions_SetDefault(VSC_OPTN_Options_GetRAOptions(options, 0), optLevel);
    VSC_OPTN_CPPOptions_SetDefault(VSC_OPTN_Options_GetCPPOptions(options, 0), optLevel);
    VSC_OPTN_CPFOptions_SetDefault(VSC_OPTN_Options_GetCPFOptions(options, 0), optLevel);
    VSC_OPTN_VECOptions_SetDefault(VSC_OPTN_Options_GetVECOptions(options, 0), optLevel);
    VSC_OPTN_LCSEOptions_SetDefault(VSC_OPTN_Options_GetLCSEOptions(options, 0), optLevel);
    VSC_OPTN_DCEOptions_SetDefault(VSC_OPTN_Options_GetDCEOptions(options, 0), optLevel);
    VSC_OPTN_IOPOptions_SetDefault(VSC_OPTN_Options_GetIOPOptions(options, 0), optLevel);
    VSC_OPTN_FAIOOptions_SetDefault(VSC_OPTN_Options_GetFAIOOptions(options, 0), optLevel);
    VSC_OPTN_DUAL16Options_SetDefault(VSC_OPTN_Options_GetDUAL16Options(options, 0), optLevel);
    VSC_OPTN_FCPOptions_SetDefault(VSC_OPTN_Options_GetFCPOptions(options, 0), optLevel);
    VSC_OPTN_MCGenOptions_SetDefault(VSC_OPTN_Options_GetMCGenOptions(options, 0), optLevel);
    VSC_OPTN_SEPGenOptions_SetDefault(VSC_OPTN_Options_GetSEPGenOptions(options, 0), optLevel);
    VSC_OPTN_DumpOptions_SetDefault(VSC_OPTN_Options_GetDumpOptions(options));
    VSC_OPTN_ILFLinkOptions_SetDefault(VSC_OPTN_Options_GetILFLinkOptions(options));
    VSC_OPTN_ATOMPatchOptions_SetDefault(VSC_OPTN_Options_GetATOMPatchOptions(options));
    VSC_OPTN_Options_SetOptionsUsage(options, gcvFALSE);
}

VSC_OPTN_BASE* VSC_OPTN_Options_GetOption(VSC_OPTN_Options* pOptions, VSC_PASS_OPTN_TYPE optnType, gctUINT passId)
{
    switch (optnType)
    {
    case VSC_PASS_OPTN_TYPE_SCPP:
        return &pOptions->scpp_options[passId].optnBase;
    case VSC_PASS_OPTN_TYPE_PAOPT:
        return &pOptions->paopt_options[passId].optnBase;
    case VSC_PASS_OPTN_TYPE_LOOPOPTS:
        return &pOptions->loopopts_options[passId].optnBase;
    case VSC_PASS_OPTN_TYPE_CFO:
        return &pOptions->cfo_options[passId].optnBase;
    case VSC_PASS_OPTN_TYPE_AUBO:
        return &pOptions->aubo_options[passId].optnBase;
    case VSC_PASS_OPTN_TYPE_INLINER:
        return &pOptions->inliner_options[passId].optnBase;
    case VSC_PASS_OPTN_TYPE_PU:
        return &pOptions->pu_options[passId].optnBase;
    case VSC_PASS_OPTN_TYPE_M2LLOWER:
        return &pOptions->lowerM2L_options[passId].optnBase;
    case VSC_PASS_OPTN_TYPE_SCALARIZE:
        return &pOptions->scalarization_options[passId].optnBase;
    case VSC_PASS_OPTN_TYPE_PH:
        return &pOptions->ph_options[passId].optnBase;
    case VSC_PASS_OPTN_TYPE_SIMP:
        return &pOptions->simp_options[passId].optnBase;
    case VSC_PASS_OPTN_TYPE_IS:
        return &pOptions->is_options[passId].optnBase;
    case VSC_PASS_OPTN_TYPE_RA:
        return &pOptions->ra_options[passId].optnBase;
    case VSC_PASS_OPTN_TYPE_CPP:
        return &pOptions->cpp_options[passId].optnBase;
    case VSC_PASS_OPTN_TYPE_CPF:
        return &pOptions->cpf_options[passId].optnBase;
    case VSC_PASS_OPTN_TYPE_VEC:
        return &pOptions->vec_options[passId].optnBase;
    case VSC_PASS_OPTN_TYPE_LCSE:
        return &pOptions->cse_options[passId].optnBase;
    case VSC_PASS_OPTN_TYPE_DCE:
        return &pOptions->dce_options[passId].optnBase;
    case VSC_PASS_OPTN_TYPE_IOP:
        return &pOptions->iopacking_options[passId].optnBase;
    case VSC_PASS_OPTN_TYPE_FAIO:
        return &pOptions->fullActiveIO_options[passId].optnBase;
    case VSC_PASS_OPTN_TYPE_DUAL16:
        return &pOptions->dual16_options[passId].optnBase;
    case VSC_PASS_OPTN_TYPE_FCP:
        return &pOptions->fcp_options[passId].optnBase;
    case VSC_PASS_OPTN_TYPE_MC_GEN:
        return &pOptions->mcgen_options[passId].optnBase;
    case VSC_PASS_OPTN_TYPE_SEP_GEN:
        return &pOptions->sepgen_options[passId].optnBase;
    case VSC_PASS_OPTN_TYPE_DUMP:
        return &pOptions->dump_options.optnBase;
    case VSC_PASS_OPTN_TYPE_ILF_LINK:
        return &pOptions->ilflink_options.optnBase;
    case VSC_PASS_OPTN_TYPE_UNIFIED_UNIFORM:
        return &pOptions->unifiedUniform_options.optnBase;
    case VSC_PASS_OPTN_TYPE_ATOM_PATCH:
        return &pOptions->atompatch_options.optnBase;
    default:
        return gcvNULL;
    }
}

void VSC_OPTN_Options_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_Options* options
    )
{
    gctSTRING pos = gcvNULL;

    /* Simple Copy Propagation options */
    gcoOS_StrStr(str, "-SCPP", &pos);
    if (pos)
    {
        pos += sizeof("-SCPP") - 1;
        VSC_OPTN_SCPPOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetSCPPOptions(options, 0));
    }

    /* Long Parameter Optimization options */
    gcoOS_StrStr(str, "-PAOPT", &pos);
    if (pos)
    {
        pos += sizeof("-PAOPT") - 1;
        VSC_OPTN_ParamOptOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetPARAMOPTOptions(options, 0));

    }

    /* loop optimizations options */
    gcoOS_StrStr(str, "-LOOP", &pos);
    if (pos)
    {
        pos += sizeof("-LOOP") - 1;
        VSC_OPTN_LoopOptsOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetLoopOptsOptions(options, 0));
    }

    /* control flow optimizations options */
    gcoOS_StrStr(str, "-CFO", &pos);
    if (pos)
    {
        gctUINT i;

        pos += sizeof("-CFO") - 1;
        for(i = 0; i < VSC_OPTN_CFO_COUNT; i++)
        {
            VSC_OPTN_CFOOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetCFOOptions(options, i));
        }
    }
    gcoOS_StrStr(str, "-CFO0", &pos);
    if (pos)
    {
        pos += sizeof("-CFO0") - 1;
        VSC_OPTN_CFOOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetCFOOptions(options, 0));
    }
    gcoOS_StrStr(str, "-CFO1", &pos);
    if (pos)
    {
        pos += sizeof("-CFO1") - 1;
        VSC_OPTN_CFOOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetCFOOptions(options, 1));
    }

    /* default UBO options */
    gcoOS_StrStr(str, "-DUBO", &pos);
    if (pos)
    {
        pos += sizeof("-DUBO") - 1;
        VSC_OPTN_UF_AUBOOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetAUBOOptions(options, 0));
    }

    /* inliner options */
    gcoOS_StrStr(str, "-IL", &pos);
    if (pos)
    {
        pos += sizeof("-IL") - 1;
        VSC_OPTN_ILOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetInlinerOptions(options, 0));
    }

    /* precision updater options */
    gcoOS_StrStr(str, "-PU", &pos);
    if (pos)
    {
        pos += sizeof("-PU") - 1;
        VSC_OPTN_PUOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetPUOptions(options, 0));
    }

    /* lowering from mid level to low level Options: */
    gcoOS_StrStr(str, "-LOWERM2L", &pos);
    if (pos)
    {
        pos += sizeof("-LOWERM2L") - 1;
        VSC_OPTN_LowerM2LOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetLowerM2LOptions(options, 0));
    }

    /* scalarization Options: */
    gcoOS_StrStr(str, "-SCL", &pos);
    if (pos)
    {
        pos += sizeof("-SCL") - 1;
        VSC_OPTN_SCLOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetSCLOptions(options, 0));
    }

    /* copy propagation Options: */
    gcoOS_StrStr(str, "-CPP:", &pos);
    if (pos)
    {
        gctUINT i;

        pos += sizeof("-CPP") - 1;
        for(i = 0; i < VSC_OPTN_CPP_COUNT; i++)
        {
            VSC_OPTN_CPPOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetCPPOptions(options, i));
        }
    }
    gcoOS_StrStr(str, "-CPP0", &pos);
    if (pos)
    {
        pos += sizeof("-CPP0") - 1;
        VSC_OPTN_CPPOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetCPPOptions(options, 0));
    }
    gcoOS_StrStr(str, "-CPP1", &pos);
    if (pos)
    {
        pos += sizeof("-CPP1") - 1;
        VSC_OPTN_CPPOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetCPPOptions(options, 0));
    }

    /* constant propagation and folding Options: */
    gcoOS_StrStr(str, "-CPF", &pos);
    if (pos)
    {
        pos += sizeof("-CPF") - 1;
        VSC_OPTN_CPFOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetCPFOptions(options, 0));
    }

    /* vectorization Options: */
    gcoOS_StrStr(str, "-VEC", &pos);
    if (pos)
    {
        pos += sizeof("-VEC") - 1;
        VSC_OPTN_VECOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetVECOptions(options, 0));
    }

    /* common subexpression elimination Options: */
    gcoOS_StrStr(str, "-LCSE", &pos);
    if (pos)
    {
        pos += sizeof("-LCSE") - 1;
        VSC_OPTN_LCSEOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetLCSEOptions(options, 0));
    }

    /* dead code elimination Options: */
    gcoOS_StrStr(str, "-DCE:", &pos);
    if (pos)
    {
        gctUINT i;

        pos += sizeof("-DCE") - 1;
        for(i = 0; i < VSC_OPTN_DCE_COUNT; i++)
        {
            VSC_OPTN_DCEOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetDCEOptions(options, i));
        }
    }
    gcoOS_StrStr(str, "-DCE0", &pos);
    if (pos)
    {
        pos += sizeof("-DCE0") - 1;
        VSC_OPTN_DCEOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetDCEOptions(options, 0));
    }
    gcoOS_StrStr(str, "-DCE1", &pos);
    if (pos)
    {
        pos += sizeof("-DCE1") - 1;
        VSC_OPTN_DCEOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetDCEOptions(options, 1));
    }

    /* peephole Options: */
    gcoOS_StrStr(str, "-PH", &pos);
    if (pos)
    {
        pos += sizeof("-PH") - 1;
        VSC_OPTN_PHOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetPHOptions(options, 0));
    }

    /* simplification Options: */
    gcoOS_StrStr(str, "-SIMP", &pos);
    if (pos)
    {
        pos += sizeof("-SIMP") - 1;
        VSC_OPTN_SIMPOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetSIMPOptions(options, 0));
    }

    /* Pre RA instruction scheduling Options: */
    gcoOS_StrStr(str, "-IS:", &pos);
    if (pos)
    {
        gctUINT i;

        pos += sizeof("-IS") - 1;
        for(i = 0; i < VSC_OPTN_IS_COUNT; i++)
        {
            VSC_OPTN_ISOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetISOptions(options, i));
        }
    }
    gcoOS_StrStr(str, "-IS0", &pos);
    if (pos)
    {
        pos += sizeof("-IS0") - 1;
        VSC_OPTN_ISOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetISOptions(options, 0));
    }
    gcoOS_StrStr(str, "-IS1", &pos);
    if (pos)
    {
        pos += sizeof("-IS1") - 1;
        VSC_OPTN_ISOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetISOptions(options, 1));
    }

    /* register allocation Options: */
    gcoOS_StrStr(str, "-RA", &pos);
    if (pos)
    {
        pos += sizeof("-RA") - 1;
        VSC_OPTN_RAOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetRAOptions(options, 0));
    }

    /* dual16 phase Options: */
    gcoOS_StrStr(str, "-DUAL16", &pos);
    if (pos)
    {
        pos += sizeof("-DUAL16") - 1;
        VSC_OPTN_DUAL16Options_GetOptionFromString(pos, VSC_OPTN_Options_GetDUAL16Options(options, 0));
    }

    /* final clean up phase Options: */
    gcoOS_StrStr(str, "-FCP", &pos);
    if (pos)
    {
        pos += sizeof("-FCP") - 1;
        VSC_OPTN_FCPOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetFCPOptions(options, 0));
    }

    /* machine code generator: */
    gcoOS_StrStr(str, "-GEN", &pos);
    if (pos)
    {
        pos += sizeof("-GEN") - 1;
        VSC_OPTN_MCGenOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetMCGenOptions(options, 0));
    }

    /* dump options */
    gcoOS_StrStr(str, "-DUMP_OPTIONS", &pos);
    if (pos)
    {
        pos += sizeof("-DUMP_OPTIONS") - 1;
        VSC_OPTN_DumpOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetDumpOptions(options));
    }

    /* unified uniform options */
    gcoOS_StrStr(str, "-UNIFIEDUNIFORM", &pos);
    if (pos)
    {
        pos += sizeof("-UNIFIEDUNIFORM") - 1;
        VSC_OPTN_UnifiedUniformOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetUnifiedUniformOptions(options));
    }

    gcoOS_StrStr(str, "-ATOMPATCH", &pos);
    if (pos)
    {
        pos += sizeof("-ATOMPATCH") - 1;
        VSC_OPTN_ATOMPatchOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetATOMPatchOptions(options));
    }

    /* print options usage*/
    gcoOS_StrStr(str, "-USAGE", &pos);
    if (pos)
    {
        VSC_OPTN_Options_SetOptionsUsage(options, gcvTRUE);
    }
}

void VSC_OPTN_Options_Dump(
    IN VSC_OPTN_Options* options,
    IN VIR_Dumper* dumper
    )
{
    VIR_LOG(dumper, "%s\nDUMP OPTIONS\n%s\n", VSC_TRACE_BAR_LINE, VSC_TRACE_BAR_LINE);
    VSC_OPTN_SCPPOptions_Dump(VSC_OPTN_Options_GetSCPPOptions(options, 0), dumper);
    VSC_OPTN_ParamOptOptions_Dump(VSC_OPTN_Options_GetPARAMOPTOptions(options, 0), dumper);
    VSC_OPTN_LoopOptsOptions_Dump(VSC_OPTN_Options_GetLoopOptsOptions(options, 0), dumper);
    VSC_OPTN_CFOOptions_Dump(VSC_OPTN_Options_GetCFOOptions(options, 0), dumper);
    VSC_OPTN_UF_AUBOOptions_Dump(VSC_OPTN_Options_GetAUBOOptions(options, 0), dumper);
    VSC_OPTN_ILOptions_Dump(VSC_OPTN_Options_GetInlinerOptions(options, 0), dumper);
    VSC_OPTN_PUOptions_Dump(VSC_OPTN_Options_GetPUOptions(options, 0), dumper);
    VSC_OPTN_LowerM2LOptions_Dump(VSC_OPTN_Options_GetLowerM2LOptions(options, 0), dumper);
    VSC_OPTN_SCLOptions_Dump(VSC_OPTN_Options_GetSCLOptions(options, 0), dumper);
    VSC_OPTN_CPPOptions_Dump(VSC_OPTN_Options_GetCPPOptions(options, 0), dumper);
    VSC_OPTN_CPPOptions_Dump(VSC_OPTN_Options_GetCPPOptions(options, 1), dumper);
    VSC_OPTN_CPFOptions_Dump(VSC_OPTN_Options_GetCPFOptions(options, 0), dumper);
    VSC_OPTN_LCSEOptions_Dump(VSC_OPTN_Options_GetLCSEOptions(options, 0), dumper);
    VSC_OPTN_PHOptions_Dump(VSC_OPTN_Options_GetPHOptions(options, 0), dumper);
    VSC_OPTN_SIMPOptions_Dump(VSC_OPTN_Options_GetSIMPOptions(options, 0), dumper);
    VSC_OPTN_ISOptions_Dump(VSC_OPTN_Options_GetISOptions(options, 0), dumper);
    VSC_OPTN_ISOptions_Dump(VSC_OPTN_Options_GetISOptions(options, 1), dumper);
    VSC_OPTN_RAOptions_Dump(VSC_OPTN_Options_GetRAOptions(options, 0), dumper);
    VSC_OPTN_DUAL16Options_Dump(VSC_OPTN_Options_GetDUAL16Options(options, 0), dumper);
    VSC_OPTN_FCPOptions_Dump(VSC_OPTN_Options_GetFCPOptions(options, 0), dumper);
    VSC_OPTN_DumpOptions_Dump(VSC_OPTN_Options_GetDumpOptions(options), dumper);
    VSC_OPTN_UnifiedUniformOptions_Dump(VSC_OPTN_Options_GetUnifiedUniformOptions(options), dumper);
    VIR_LOG(dumper, "options usage: %s\n", VSC_OPTN_Options_GetOptionsUsage(options) ? "true" : "false");
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_Options_Usage(
    IN VIR_Dumper* dumper
    )
{
    VIR_LOG(dumper, "%s\nOPTIONS USAGE\n%s\n", VSC_TRACE_BAR_LINE, VSC_TRACE_BAR_LINE);
    VSC_OPTN_SCPPOptions_Usage(dumper);
    VSC_OPTN_ParamOptOptions_Usage(dumper);
    VSC_OPTN_LoopOptsOptions_Usage(dumper);
    VSC_OPTN_CFOOptions_Usage(dumper);
    VSC_OPTN_UF_AUBOOptions_Usage(dumper);
    VSC_OPTN_ILOptions_Usage(dumper);
    VSC_OPTN_PUOptions_Usage(dumper);
    VSC_OPTN_LowerM2LOptions_Usage(dumper);
    VSC_OPTN_SCLOptions_Usage(dumper);
    VSC_OPTN_PHOptions_Usage(dumper);
    VSC_OPTN_CPPOptions_Usage(dumper);
    VSC_OPTN_CPFOptions_Usage(dumper);
    VSC_OPTN_LCSEOptions_Usage(dumper);
    VSC_OPTN_SIMPOptions_Usage(dumper);
    VSC_OPTN_ISOptions_Usage(dumper);
    VSC_OPTN_RAOptions_Usage(dumper);
    VSC_OPTN_DUAL16Options_Usage(dumper);
    VSC_OPTN_FCPOptions_Usage(dumper);
    VIR_LOG(dumper, "-DUMP_OPTIONS        dump options\n");
    VIR_LOG(dumper, "-USAGE               print options usage\n");
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_Options_SetOptionsByCompileFlags(
    IN OUT VSC_OPTN_Options* Options,
    IN  gctUINT CompileFlags
    )
{
    Options->cFlags = CompileFlags;
}

void VSC_OPTN_Options_SetOptionsByOptFlags(
    IN OUT VSC_OPTN_Options* Options,
    IN  gctUINT64 OptFlags
    )
{
    if(OptFlags & VSC_COMPILER_OPT_CONSTANT_REG_SPILLABLE)
    {
        gcmASSERT(!(OptFlags & VSC_COMPILER_OPT_NO_CONSTANT_REG_SPILLABLE));
        VSC_OPTN_UF_AUBOOptions_SetSwitchOn(VSC_OPTN_Options_GetAUBOOptions(Options, 0), gcvTRUE);
    }
    else if(OptFlags & VSC_COMPILER_OPT_NO_CONSTANT_REG_SPILLABLE)
    {
        VSC_OPTN_UF_AUBOOptions_SetSwitchOn(VSC_OPTN_Options_GetAUBOOptions(Options, 0), gcvFALSE);
    }

    if(OptFlags & VSC_COMPILER_OPT_DCE)
    {
        gcmASSERT(!(OptFlags & VSC_COMPILER_OPT_NO_DCE));
        VSC_OPTN_DCEOptions_SetSwitchOn(VSC_OPTN_Options_GetDCEOptions(Options, 0), gcvTRUE);
        VSC_OPTN_DCEOptions_SetSwitchOn(VSC_OPTN_Options_GetDCEOptions(Options, 1), gcvTRUE);
    }
    else if(OptFlags & VSC_COMPILER_OPT_NO_DCE)
    {
        VSC_OPTN_DCEOptions_SetSwitchOn(VSC_OPTN_Options_GetDCEOptions(Options, 0), gcvFALSE);
        VSC_OPTN_DCEOptions_SetSwitchOn(VSC_OPTN_Options_GetDCEOptions(Options, 1), gcvFALSE);
    }

    if(OptFlags & VSC_COMPILER_OPT_PEEPHOLE)
    {
        gcmASSERT(!(OptFlags & VSC_COMPILER_OPT_NO_PEEPHOLE));
        VSC_OPTN_PHOptions_SetSwitchOn(VSC_OPTN_Options_GetPHOptions(Options, 0), gcvTRUE);
    }
    else if(OptFlags & VSC_COMPILER_OPT_NO_PEEPHOLE)
    {
        VSC_OPTN_PHOptions_SetSwitchOn(VSC_OPTN_Options_GetPHOptions(Options, 0), gcvFALSE);
    }

    if(OptFlags & VSC_COMPILER_OPT_LCSE)
    {
        gcmASSERT(!(OptFlags & VSC_COMPILER_OPT_NO_LCSE));
        VSC_OPTN_LCSEOptions_SetSwitchOn(VSC_OPTN_Options_GetLCSEOptions(Options, 0), gcvTRUE);
    }
    else if(OptFlags & VSC_COMPILER_OPT_NO_LCSE)
    {
        VSC_OPTN_LCSEOptions_SetSwitchOn(VSC_OPTN_Options_GetLCSEOptions(Options, 0), gcvFALSE);
    }

    if(OptFlags & VSC_COMPILER_OPT_CONSTANT_PROPOGATION)
    {
        gcmASSERT(!(OptFlags & VSC_COMPILER_OPT_NO_CONSTANT_PROPOGATION));
        VSC_OPTN_CPPOptions_SetSwitchOn(VSC_OPTN_Options_GetCPPOptions(Options, 0), gcvTRUE);
        VSC_OPTN_CPPOptions_SetSwitchOn(VSC_OPTN_Options_GetCPPOptions(Options, 1), gcvTRUE);
    }
    else if(OptFlags & VSC_COMPILER_OPT_NO_CONSTANT_PROPOGATION)
    {
        VSC_OPTN_CPPOptions_SetSwitchOn(VSC_OPTN_Options_GetCPPOptions(Options, 0), gcvFALSE);
        VSC_OPTN_CPPOptions_SetSwitchOn(VSC_OPTN_Options_GetCPPOptions(Options, 1), gcvFALSE);
    }

    if(OptFlags & VSC_COMPILER_OPT_CONSTANT_FOLDING)
    {
        gcmASSERT(!(OptFlags & VSC_COMPILER_OPT_NO_CONSTANT_FOLDING));
        VSC_OPTN_CPFOptions_SetSwitchOn(VSC_OPTN_Options_GetCPFOptions(Options, 0), gcvTRUE);
    }
    else if(OptFlags & VSC_COMPILER_OPT_NO_CONSTANT_FOLDING)
    {
        VSC_OPTN_CPFOptions_SetSwitchOn(VSC_OPTN_Options_GetCPFOptions(Options, 0), gcvFALSE);
    }

    if(OptFlags & VSC_COMPILER_OPT_VEC)
    {
        gcmASSERT(!(OptFlags & VSC_COMPILER_OPT_NO_VEC));
        VSC_OPTN_VECOptions_SetSwitchOn(VSC_OPTN_Options_GetVECOptions(Options, 0), gcvTRUE);
    }
    else if(OptFlags & VSC_COMPILER_OPT_NO_VEC)
    {
        VSC_OPTN_VECOptions_SetSwitchOn(VSC_OPTN_Options_GetVECOptions(Options, 0), gcvFALSE);
    }

    if(OptFlags & VSC_COMPILER_OPT_IO_PACKING)
    {
        gcmASSERT(!(OptFlags & VSC_COMPILER_OPT_NO_IO_PACKING));
        VSC_OPTN_IOPOptions_SetSwitchOn(VSC_OPTN_Options_GetIOPOptions(Options, 0), gcvTRUE);
    }
    else if(OptFlags & VSC_COMPILER_OPT_NO_IO_PACKING)
    {
        VSC_OPTN_IOPOptions_SetSwitchOn(VSC_OPTN_Options_GetIOPOptions(Options, 0), gcvFALSE);
    }

    if(OptFlags & VSC_COMPILER_OPT_FULL_ACTIVE_IO)
    {
        gcmASSERT(!(OptFlags & VSC_COMPILER_OPT_NO_FULL_ACTIVE_IO));
        VSC_OPTN_FAIOOptions_SetSwitchOn(VSC_OPTN_Options_GetFAIOOptions(Options, 0), gcvTRUE);
    }
    else if(OptFlags & VSC_COMPILER_OPT_NO_FULL_ACTIVE_IO)
    {
        VSC_OPTN_FAIOOptions_SetSwitchOn(VSC_OPTN_Options_GetFAIOOptions(Options, 0), gcvFALSE);
    }

    if(OptFlags & VSC_COMPILER_OPT_DUAL16)
    {
        gcmASSERT(!(OptFlags & VSC_COMPILER_OPT_NO_DUAL16));
        VSC_OPTN_DUAL16Options_SetSwitchOn(VSC_OPTN_Options_GetDUAL16Options(Options, 0), gcvTRUE);
    }
    else if(OptFlags & VSC_COMPILER_OPT_NO_DUAL16)
    {
        VSC_OPTN_DUAL16Options_SetSwitchOn(VSC_OPTN_Options_GetDUAL16Options(Options, 0), gcvFALSE);
    }

    if(OptFlags & VSC_COMPILER_OPT_ILF_LINK)
    {
        gcmASSERT(!(OptFlags & VSC_COMPILER_OPT_NO_ILF_LINK));
        VSC_OPTN_ILFLinkOptions_SetSwitchOn(VSC_OPTN_Options_GetILFLinkOptions(Options), gcvTRUE);
        /* atomic patch pass should follow with VIR_LinkInternalLibFunc by default */
        VSC_OPTN_ATOMPatchOptions_SetSwitchOn(VSC_OPTN_Options_GetATOMPatchOptions(Options), gcvTRUE);
    }
    else if(OptFlags & VSC_COMPILER_OPT_NO_ILF_LINK)
    {
        VSC_OPTN_ILFLinkOptions_SetSwitchOn(VSC_OPTN_Options_GetILFLinkOptions(Options), gcvFALSE);
        VSC_OPTN_ATOMPatchOptions_SetSwitchOn(VSC_OPTN_Options_GetATOMPatchOptions(Options), gcvFALSE);
    }

    if(OptFlags & VSC_COMPILER_OPT_FUNC_INLINE)
    {
        gcmASSERT(!(OptFlags & VSC_COMPILER_OPT_NO_FUNC_INLINE));
        VSC_OPTN_ILOptions_SetSwitchOn(VSC_OPTN_Options_GetInlinerOptions(Options, 0), gcvTRUE);
    }
    else if(OptFlags & VSC_COMPILER_OPT_NO_FUNC_INLINE)
    {
        VSC_OPTN_ILOptions_SetSwitchOn(VSC_OPTN_Options_GetInlinerOptions(Options, 0), gcvFALSE);
    }

    if(OptFlags & VSC_COMPILER_OPT_ALGE_SIMP)
    {
        gcmASSERT(!(OptFlags & VSC_COMPILER_OPT_NO_ALGE_SIMP));
        VSC_OPTN_SIMPOptions_SetSwitchOn(VSC_OPTN_Options_GetSIMPOptions(Options, 0), gcvTRUE);
    }
    else if(OptFlags & VSC_COMPILER_OPT_NO_ALGE_SIMP)
    {
        VSC_OPTN_SIMPOptions_SetSwitchOn(VSC_OPTN_Options_GetSIMPOptions(Options, 0), gcvFALSE);
    }

    if(OptFlags & VSC_COMPILER_OPT_INST_SKED)
    {
        gcmASSERT(!(OptFlags & VSC_COMPILER_OPT_NO_INST_SKED));
        VSC_OPTN_ISOptions_SetSwitchOn(VSC_OPTN_Options_GetISOptions(Options, 0), gcvTRUE);
    }
    else if(OptFlags & VSC_COMPILER_OPT_NO_INST_SKED)
    {
        VSC_OPTN_ISOptions_SetSwitchOn(VSC_OPTN_Options_GetISOptions(Options, 0), gcvFALSE);
        VSC_OPTN_ISOptions_SetSwitchOn(VSC_OPTN_Options_GetISOptions(Options, 1), gcvFALSE);
    }
}

void VSC_OPTN_Options_SetSpecialOptions(
    IN OUT VSC_OPTN_Options* options,
    IN VSC_HW_CONFIG* pHwCfg
    )
{
    gctBOOL useNewCG = gcUseFullNewLinker(pHwCfg->hwFeatureFlags.hasHalti2);

    if (useNewCG)
    {
        VSC_OPTN_RAOptions* ra_options = VSC_OPTN_Options_GetRAOptions(options, 0);
        VSC_OPTN_MCGenOptions* mc_options = VSC_OPTN_Options_GetMCGenOptions(options, 0);
        VSC_OPTN_ISOptions* prera_is_options = VSC_OPTN_Options_GetISOptions(options, 0);
        VSC_OPTN_ISOptions* postra_is_options = VSC_OPTN_Options_GetISOptions(options, 1);

        VSC_OPTN_RAOptions_SetSwitchOn(ra_options, gcvTRUE);
        VSC_OPTN_MCGenOptions_SetSwitchOn(mc_options, gcvTRUE);
        VSC_OPTN_ISOptions_SetSwitchOn(prera_is_options, gcvTRUE);
        VSC_OPTN_ISOptions_SetSwitchOn(postra_is_options, gcvTRUE);
    }
    else
    {
        VSC_OPTN_PHOptions* ph_options = VSC_OPTN_Options_GetPHOptions(options, 0);
        VSC_OPTN_ILOptions* il_options = VSC_OPTN_Options_GetInlinerOptions(options, 0);

        VSC_OPTN_PHOptions_SetOPTS(ph_options, VSC_OPTN_PHOptions_GetOPTS(ph_options) | VSC_OPTN_PHOptions_OPTS_VEC);
        VSC_OPTN_ILOptions_SetSwitchOn(il_options, gcvFALSE);
    }

    if (pHwCfg->hwFeatureFlags.supportUnifiedConstant && pHwCfg->hwFeatureFlags.supportUnifiedSampler)
    {
        VSC_OPTN_UnifiedUniformOptions* unifiedUniform_options = VSC_OPTN_Options_GetUnifiedUniformOptions(options);

        VSC_OPTN_UnifiedUniformOptions_SetSwitchOn(unifiedUniform_options, gcvTRUE);
    }

    {
        if(gcPatchId == gcvPATCH_OCLCTS)
        {
            VSC_OPTN_ISOptions_SetSwitchOn(VSC_OPTN_Options_GetISOptions(options, 0), gcvFALSE);
        }
    }

    if (pHwCfg->hwFeatureFlags.supportPSCSThrottle || pHwCfg->hwFeatureFlags.supportHWManagedLS)
    {
        VSC_OPTN_PHOptions* ph_options = VSC_OPTN_Options_GetPHOptions(options, 0);
        gctUINT ph_option = VSC_OPTN_PHOptions_GetOPTS(ph_options);

        VSC_OPTN_PHOptions_SetOPTS(ph_options, (ph_option | VSC_OPTN_PHOptions_OPTS_LOC_MEM));
    }

    /* Enable HW local memory for new CG only. */
    if (!useNewCG)
    {
        VSC_OPTN_PHOptions* ph_options = VSC_OPTN_Options_GetPHOptions(options, 0);
        gctUINT ph_option = VSC_OPTN_PHOptions_GetOPTS(ph_options);

        VSC_OPTN_PHOptions_SetOPTS(ph_options, (ph_option & (~VSC_OPTN_PHOptions_OPTS_LOC_MEM)));
    }

    /* temperarily switch off Post RA IS */
    VSC_OPTN_ISOptions_SetSwitchOn(VSC_OPTN_Options_GetISOptions(options, 1), gcvFALSE);

    /* disable atomic patch for compbenchcl for performance */
    if (gcPatchId == gcvPATCH_COMPUTBENCH_CL)
    {
        VSC_OPTN_ATOMPatchOptions_SetSwitchOn(VSC_OPTN_Options_GetATOMPatchOptions(options), gcvFALSE);
    }
}

gctBOOL VSC_OPTN_Options_GetOptLevelFromEnv(
    OUT gctUINT* optLevel
    )
{
    char* p = gcvNULL;
    gcoOS_GetEnv(gcvNULL, "VSC_OPTION", &p);
    if (p)
    {
        gctSTRING pos = gcvNULL;

        gcoOS_StrStr(p, "-O:", &pos);

        if(pos)
        {
            gctUINT32 ol;
            gctUINT32 len;

            pos += 3;
            len = _VSC_OPTN_GetSubOptionLength(pos);
            ol = vscSTR_StrToUint32(pos, len);
            *optLevel = ol;

            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

void VSC_OPTN_Options_GetOptionFromEnv(
    IN OUT VSC_OPTN_Options* options
    )
{
    char* p = gcvNULL;
    gcoOS_GetEnv(gcvNULL, "VSC_OPTION", &p);
    if (p)
    {
        VSC_OPTN_Options_GetOptionFromString(p, options);
    }
}

void VSC_OPTN_Options_MergeVCEnvOption(
    IN OUT VSC_OPTN_Options* options
    )
{
    {
        VSC_OPTN_DUAL16Options* dual16_options = VSC_OPTN_Options_GetDUAL16Options(options, 0);
        if (VSC_OPTN_DUAL16Options_GetSwitchOn(dual16_options) && gcmOPT_DualFP16Mode() == DUAL16_FORCE_OFF)
        {
            VSC_OPTN_DUAL16Options_SetSwitchOn(dual16_options, gcvFALSE);
        }
    }

    /* Dump options. */
    {
        VSC_OPTN_DumpOptions* dump_options = VSC_OPTN_Options_GetDumpOptions(options);
        gctUINT opt = VSC_OPTN_DumpOptions_DUMP_NONE;
        if (!(options->cFlags & VSC_COMPILER_FLAG_DISABLE_IR_DUMP))
        {
            if (gcmOPT_DUMP_OPTIMIZER_VERBOSE())
            {
                opt |= VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE;
            }
            if (gcmOPT_DUMP_CODEGEN())
            {
                opt |= VSC_OPTN_DumpOptions_DUMP_CG;
            }
            if (gcmOPT_DUMP_FINAL_IR())
            {
                opt |= VSC_OPTN_DumpOptions_DUMP_FINALIR;
            }
        }
        VSC_OPTN_DumpOptions_SetDumpStart(dump_options, gcmOPT_DUMP_Start());
        VSC_OPTN_DumpOptions_SetDumpEnd(dump_options, gcmOPT_DUMP_End());
        VSC_OPTN_DumpOptions_SetOPTS(dump_options, opt);
    }

    {
        /* turn off most opts when debugging mode is turned on */
        if (gcmOPT_EnableDebugMode())
        {
            VSC_OPTN_ISOptions_SetSwitchOn(VSC_OPTN_Options_GetISOptions(options, 0), gcvFALSE);
            VSC_OPTN_ISOptions_SetSwitchOn(VSC_OPTN_Options_GetISOptions(options, 1), gcvFALSE);
            VSC_OPTN_SCPPOptions_SetSwitchOn(VSC_OPTN_Options_GetSCPPOptions(options, 0), gcvFALSE);
            VSC_OPTN_LoopOptsOptions_SetSwitchOn(VSC_OPTN_Options_GetLoopOptsOptions(options, 0), gcvFALSE);
            VSC_OPTN_CFOOptions_SetSwitchOn(VSC_OPTN_Options_GetCFOOptions(options, 0), gcvFALSE);
            VSC_OPTN_CFOOptions_SetSwitchOn(VSC_OPTN_Options_GetCFOOptions(options, 1), gcvFALSE);
            VSC_OPTN_ILOptions_SetSwitchOn(VSC_OPTN_Options_GetInlinerOptions(options, 0), gcvFALSE);
            VSC_OPTN_SCLOptions_SetSwitchOn(VSC_OPTN_Options_GetSCLOptions(options, 0), gcvFALSE);
            VSC_OPTN_PHOptions_SetSwitchOn(VSC_OPTN_Options_GetPHOptions(options, 0), gcvFALSE);
            VSC_OPTN_SIMPOptions_SetSwitchOn(VSC_OPTN_Options_GetSIMPOptions(options, 0), gcvFALSE);
            VSC_OPTN_CPPOptions_SetSwitchOn(VSC_OPTN_Options_GetCPPOptions(options, 0), gcvFALSE);
            VSC_OPTN_CPPOptions_SetSwitchOn(VSC_OPTN_Options_GetCPPOptions(options, 1), gcvFALSE);
            VSC_OPTN_CPFOptions_SetSwitchOn(VSC_OPTN_Options_GetCPFOptions(options, 0), gcvFALSE);
            VSC_OPTN_VECOptions_SetSwitchOn(VSC_OPTN_Options_GetVECOptions(options, 0), gcvFALSE);
            VSC_OPTN_LCSEOptions_SetSwitchOn(VSC_OPTN_Options_GetLCSEOptions(options, 0), gcvFALSE);
            VSC_OPTN_DCEOptions_SetSwitchOn(VSC_OPTN_Options_GetDCEOptions(options, 0), gcvFALSE);
            VSC_OPTN_DCEOptions_SetSwitchOn(VSC_OPTN_Options_GetDCEOptions(options, 1), gcvFALSE);
            VSC_OPTN_DUAL16Options_SetSwitchOn(VSC_OPTN_Options_GetDUAL16Options(options, 0), gcvFALSE);
        }
    }
}

gctBOOL VSC_OPTN_InRange(
    IN gctUINT32 id,
    IN gctUINT32 before,
    IN gctUINT32 after
    )
{
    if(before == gcvMAXUINT32 && after == gcvMAXUINT32)
    {
        return gcvTRUE;
    }
    if(before == gcvMAXUINT32)
    {
        return id > after;
    }
    if(after == gcvMAXUINT32)
    {
        return id < before;
    }
    if(before > after)
    {
        return id > after && id < before;
    }
    else
    {
        return id < before || id > after;
    }
}

