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


#include "gc_vsc.h"

VSC_OPTN_Options theVSCOption;

VSC_OPTN_Options *
VSC_OPTN_Get_Options()
{
    if (!theVSCOption.initialized)
    {
        /* initialize the option */
        VSC_OPTN_Options_SetDefault(&theVSCOption);
        VSC_OPTN_Options_GetOptionFromEnv(&theVSCOption);
        theVSCOption.initialized = gcvTRUE;
    }
    return &theVSCOption;
}

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

/* HW configuration option */
void VSC_OPTN_HWOptions_SetDefault(
    IN OUT VSC_OPTN_HWOptions* options
    )
{
    VSC_OPTN_HWOptions_SetRegFilesPerCore(options, 4);
    VSC_OPTN_HWOptions_SetRegFileLen(options, 128);
    VSC_OPTN_HWOptions_SetGroupDispatchCycles(options, 4);
    VSC_OPTN_HWOptions_SetPipelineCycles(options, 28);
    VSC_OPTN_HWOptions_SetTexldCycles(options, 384);
    VSC_OPTN_HWOptions_SetMemldCycles(options, 384);
}

void VSC_OPTN_HWOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_HWOptions* options
    )
{
    while (str[0] == ':')
    {
        ++str;
        if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "reg_files_per_core:", sizeof("reg_files_per_core:") - 1))
        {
            gctUINT32 reg_files_per_core;
            gctUINT32 len;

            str += sizeof("reg_files_per_core:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            reg_files_per_core = vscSTR_StrToUint32(str, len);
            VSC_OPTN_HWOptions_SetRegFilesPerCore(options, reg_files_per_core);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "reg_file_len:", sizeof("reg_file_len:") - 1))
        {
            gctUINT32 reg_file_len;
            gctUINT32 len;

            str += sizeof("reg_file_len:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            reg_file_len = vscSTR_StrToUint32(str, len);
            VSC_OPTN_HWOptions_SetRegFileLen(options, reg_file_len);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "group_dispatch_cycles:", sizeof("group_dispatch_cycles:") - 1))
        {
            gctUINT32 group_dispatch_cycles;
            gctUINT32 len;

            str += sizeof("group_dispatch_cycles:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            group_dispatch_cycles = vscSTR_StrToUint32(str, len);
            VSC_OPTN_HWOptions_SetGroupDispatchCycles(options, group_dispatch_cycles);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "pipeline_cycles:", sizeof("pipeline_cycles:") - 1))
        {
            gctUINT32 pipeline_cycles;
            gctUINT32 len;

            str += sizeof("pipeline_cycles:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            pipeline_cycles = vscSTR_StrToUint32(str, len);
            VSC_OPTN_HWOptions_SetPipelineCycles(options, pipeline_cycles);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "texld_cycles:", sizeof("texld_cycles:") - 1))
        {
            gctUINT32 texld_cycles;
            gctUINT32 len;

            str += sizeof("texld_cycles:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            texld_cycles = vscSTR_StrToUint32(str, len);
            VSC_OPTN_HWOptions_SetTexldCycles(options, texld_cycles);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "memld_cycles:", sizeof("memld_cycles:") - 1))
        {
            gctUINT32 memld_cycles;
            gctUINT32 len;

            str += sizeof("memld_cycles:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            memld_cycles = vscSTR_StrToUint32(str, len);
            VSC_OPTN_HWOptions_SetMemldCycles(options, memld_cycles);
            str += len;
        }
    }
}

void VSC_OPTN_HWOptions_Dump(
    IN VSC_OPTN_HWOptions* options,
    IN VIR_Dumper* dumper
    )
{
    VIR_LOG(dumper, "hardware options:\n");
    VIR_LOG(dumper, "    reg_files_per_core: %d\n", VSC_OPTN_HWOptions_GetRegFilesPerCore(options));
    VIR_LOG(dumper, "    reg_file_len: %d\n", VSC_OPTN_HWOptions_GetRegFileLen(options));
    VIR_LOG(dumper, "    group_dispatch_cycles: %d\n", VSC_OPTN_HWOptions_GetGroupDispatchCycles(options));
    VIR_LOG(dumper, "    pipeline_cycles: %d\n", VSC_OPTN_HWOptions_GetPipelineCycles(options));
    VIR_LOG(dumper, "    texld_cycles: %d\n", VSC_OPTN_HWOptions_GetTexldCycles(options));
    VIR_LOG(dumper, "    memld_cycles: %d\n", VSC_OPTN_HWOptions_GetMemldCycles(options));
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_HWOptions_Usage(
    IN VIR_Dumper* dumper
    )
{
    gctSTRING usage =
        "-HW:\n"
        "    reg_files_per_core      register file count per core\n"
        "    reg_file_len            register file length\n"
        "    group_dispatch_cycles   group dispatching cycles\n"
        "    pipeline_cycles         pipeline cycles\n"
        "    texld_cycles            texture load cycles\n"
        "    memld_cycles            memory load cycles\n";

    VIR_LOG(dumper, usage);
    VIR_LOG_FLUSH(dumper);
}

/* Default UBO options */
void VSC_OPTN_UF_AUBO_Options_SetDefault(
    IN OUT VSC_OPTN_UF_AUBO_Options* options
    )
{
    VSC_OPTN_UF_AUBO_Options_SetSwitchOn(options, gcvTRUE);
    VSC_OPTN_UF_AUBO_Options_SetHeuristics(options,
                                           /*VSC_OPTN_UF_AUBO_Options_HEUR_FORCE_ALL |
                                           VSC_OPTN_UF_AUBO_Options_HEUR_INDIRECT_MUST |*/
                                           VSC_OPTN_UF_AUBO_Options_HEUR_ORDERLY |
                                           VSC_OPTN_UF_AUBO_Options_HEUR_USE_DUBO |
                                           VSC_OPTN_UF_AUBO_Options_HEUR_USE_CUBO);
    VSC_OPTN_UF_AUBO_Options_SetConstRegReservation(options, 0);
    VSC_OPTN_UF_AUBO_Options_SetTrace(options, 0);
}

void VSC_OPTN_UF_AUBO_Options_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_UF_AUBO_Options* options
    )
{
    while (str[0] == ':')
    {
        ++str;
        if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "on", sizeof("on") - 1))
        {
            VSC_OPTN_UF_AUBO_Options_SetSwitchOn(options, gcvTRUE);
            str += sizeof("on") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "off", sizeof("off") - 1))
        {
            VSC_OPTN_UF_AUBO_Options_SetSwitchOn(options, gcvFALSE);
            str += sizeof("off") - 1;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "heuristics:", sizeof("heuristics:") - 1))
        {
            gctUINT32 heuristics;
            gctUINT32 len;

            str += sizeof("heuristics:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            heuristics = vscSTR_StrToUint32(str, len);
            VSC_OPTN_UF_AUBO_Options_SetHeuristics(options, heuristics);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "const_reg_reservation:", sizeof("const_reg_reservation:") - 1))
        {
            gctUINT32 const_reg_reservation;
            gctUINT32 len;

            str += sizeof("const_reg_reservation:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            const_reg_reservation = vscSTR_StrToUint32(str, len);
            VSC_OPTN_UF_AUBO_Options_SetConstRegReservation(options, const_reg_reservation);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "trace:", sizeof("trace:")-1))
        {
            gctUINT32 trace;
            gctUINT32 len;

            str += sizeof("trace:") -1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            trace = vscSTR_StrToUint32(str, len);
            VSC_OPTN_UF_AUBO_Options_SetTrace(options, trace);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "bs:", sizeof("bs:") - 1))
        {
            gctUINT32 before_shader;
            gctUINT32 len;

            str += sizeof("bs:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            before_shader = vscSTR_StrToUint32(str, len);
            VSC_OPTN_UF_AUBO_Options_SetBeforeShader(options, before_shader);
            str += len;
        }
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "as:", sizeof("as:") - 1))
        {
            gctUINT32 after_shader;
            gctUINT32 len;

            str += sizeof("as:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            after_shader = vscSTR_StrToUint32(str, len);
            VSC_OPTN_UF_AUBO_Options_SetAfterShader(options, after_shader);
            str += len;
        }
    }
}

void VSC_OPTN_UF_AUBO_Options_Dump(
    IN VSC_OPTN_UF_AUBO_Options* options,
    IN VIR_Dumper* dumper
    )
{
    VIR_LOG(dumper, "default UBO options:\n");
    VIR_LOG(dumper, "    on: %s\n", VSC_OPTN_UF_AUBO_Options_GetSwitchOn(options) ? "true" : "false");
    VIR_LOG(dumper, "    heuristics: %x\n", VSC_OPTN_UF_AUBO_Options_GetHeuristics(options));
    VIR_LOG(dumper, "    const_reg_reservation: %x\n", VSC_OPTN_UF_AUBO_Options_GetConstRegReservation(options));
    VIR_LOG(dumper, "    trace: %x\n", VSC_OPTN_UF_AUBO_Options_GetTrace(options));
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_UF_AUBO_Options_Usage(
    IN VIR_Dumper* dumper
    )
{
    gctSTRING usage =
        "-DUBO:\n"
        "    on                         turn on VIR inliner\n"
        "    off                        turn off VIR inliner\n"
        "    heuristics:                0x1    \n"
        "    const_reg_reservation:     number of constant register to reserve\n"
        "    trace:                     0x1    \n"
        "                               0x2    \n";

    VIR_LOG(dumper, usage);
    VIR_LOG_FLUSH(dumper);
}

/* Inliner options */
void VSC_OPTN_ILOptions_SetDefault(
    IN OUT VSC_OPTN_ILOptions* options
    )
{
    VSC_OPTN_ILOptions_SetSwitchOn(options, gcvFALSE);
    VSC_OPTN_ILOptions_SetHeuristics(options, 0xffffffff);
    VSC_OPTN_ILOptions_SetTrace(options, 0);
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
        "                        0x2    trace output\n";

    VIR_LOG(dumper, usage);
    VIR_LOG_FLUSH(dumper);
}

/* M2L lower option */
void VSC_OPTN_LowerM2LOptions_SetDefault(
    IN OUT VSC_OPTN_LowerM2LOptions* options
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
    IN OUT VSC_OPTN_SCLOptions* options
    )
{
    VSC_OPTN_SCLOptions_SetSwitchOn(options, gcvFALSE);
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
    IN OUT VSC_OPTN_PHOptions* options
    )
{
    gctUINT  option = VSC_OPTN_PHOptions_OPTS_MODIFIER |
                      VSC_OPTN_PHOptions_OPTS_MAD |
                      VSC_OPTN_PHOptions_OPTS_RSQ |
                      VSC_OPTN_PHOptions_OPTS_RUC |
                      VSC_OPTN_PHOptions_OPTS_MOV_LDARR;

    if (!ENABLE_FULL_NEW_LINKER)
    {
        option |= VSC_OPTN_PHOptions_OPTS_VEC;
    }

    VSC_OPTN_PHOptions_SetSwitchOn(options, gcvTRUE);
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
    IN OUT VSC_OPTN_SIMPOptions* options
    )
{
    VSC_OPTN_SIMPOptions_SetSwitchOn(options, gcvFALSE);
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
    IN OUT VSC_OPTN_ISOptions* options
    )
{
    gcoHARDWARE hardware;

    gcoHAL_GetHardware(gcvNULL, &hardware);
    gcmASSERT(hardware);
    if(hardware->config->chipModel == gcv3000)
    {
        VSC_OPTN_ISOptions_SetSwitchOn(options, gcvTRUE);
    }
    else
    {
        VSC_OPTN_ISOptions_SetSwitchOn(options, gcvFALSE);
    }
    VSC_OPTN_ISOptions_SetIsForward(options, gcvTRUE);
    VSC_OPTN_ISOptions_SetLLIOnly(options, gcvTRUE);
    VSC_OPTN_ISOptions_SetBandwidthOnly(options, gcvFALSE);
    VSC_OPTN_ISOptions_SetRegCount(options, 0);         /* 0 is invalid number here */
    VSC_OPTN_ISOptions_SetDepGran(options, VSC_OPTN_ISOptions_DEPGRAN_GROSS);
    VSC_OPTN_ISOptions_SetBBCeiling(options, 500);
    VSC_OPTN_ISOptions_SetAlgorithm(options, VSC_OPTN_ISOptions_ALGORITHM_LISTSCHEDULING
                                           /*, VSC_OPTN_ISOptions_ALGORITHM_BUBBLESCHEDULING*/);
    /*VSC_OPTN_ISOptions_SetDepGran(options, VSC_OPTN_ISOptions_DEPGRAN_DU_PER_CHANNAL);*/
    VSC_OPTN_ISOptions_SetFwHeuristics(options, (
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
    VSC_OPTN_ISOptions_SetBwHeuristics(options, (
        /*VSC_OPTN_ISOptions_BW_HEUR_PREFER_RANGE |*/
        VSC_OPTN_ISOptions_BW_HEUR_PREFER_ORDER
        ));
    VSC_OPTN_ISOptions_SetTrace(options, 0);
    VSC_OPTN_ISOptions_SetBeforeShader(options, gcvMAXUINT32);
    VSC_OPTN_ISOptions_SetAfterShader(options, gcvMAXUINT32);
    VSC_OPTN_ISOptions_SetBeforeFunc(options, gcvMAXUINT32);
    VSC_OPTN_ISOptions_SetAfterFunc(options, gcvMAXUINT32);
    VSC_OPTN_ISOptions_SetBeforeBB(options, gcvMAXUINT32);
    VSC_OPTN_ISOptions_SetAfterBB(options, gcvMAXUINT32);
    VSC_OPTN_ISOptions_SetBeforeInst(options, gcvMAXUINT32);
    VSC_OPTN_ISOptions_SetAfterInst(options, gcvMAXUINT32);
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
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "dep_gran:", sizeof("dep_gran:") - 1))
        {
            gctUINT32 dep_gran;
            gctUINT32 len;

            str += sizeof("dep_gran:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            dep_gran = vscSTR_StrToUint32(str, len);
            VSC_OPTN_ISOptions_SetDepGran(options, dep_gran);
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
    VIR_LOG(dumper, "%s instruction scheduling options:\n", VSC_OPTN_ISOptions_GetPreRA(options) ? "Pre RA" : "Post RA");
    VIR_LOG(dumper, "    on: %s\n", VSC_OPTN_ISOptions_GetSwitchOn(options) ? "true" : "false");
    VIR_LOG(dumper, "    lli_only: %s\n", VSC_OPTN_ISOptions_GetLLIOnly(options) ? "true" : "false");
    VIR_LOG(dumper, "    bandwidth_only: %s\n", VSC_OPTN_ISOptions_GetBandwidthOnly(options) ? "true" : "false");
    VIR_LOG(dumper, "    reg_count: %d\n", VSC_OPTN_ISOptions_GetRegCount(options));
    VIR_LOG(dumper, "    depandency granularity: %d\n", VSC_OPTN_ISOptions_GetDepGran(options));
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
    IN OUT VSC_OPTN_RAOptions* options
    )
{
    VSC_OPTN_RAOptions_SetSwitchOn(options, ENABLE_FULL_NEW_LINKER);
    VSC_OPTN_RAOptions_SetHeuristics(options, 0xffffffff);
    VSC_OPTN_RAOptions_SetTrace(options, 0);
    VSC_OPTN_RAOptions_SetOPTS(options, 0x3);
    VSC_OPTN_RAOptions_SetRegisterCount(options, 0);
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
        "    bs:                 set spill shader before\n"
        "    as:                 set spill shader after\n";
    VIR_LOG(dumper, usage);
    VIR_LOG_FLUSH(dumper);
}

/* copy propagation options*/
void VSC_OPTN_CPPOptions_SetDefault(
    IN OUT VSC_OPTN_CPPOptions* options
    )
{
    VSC_OPTN_CPPOptions_SetSwitchOn(options, gcvTRUE);
    VSC_OPTN_CPPOptions_SetOPTS(options, 0xff);
    VSC_OPTN_CPPOptions_SetTrace(options, 0);
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
    IN OUT VSC_OPTN_CPFOptions* options
    )
{
    VSC_OPTN_CPFOptions_SetSwitchOn(options, gcvTRUE);
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

/* constant propagation and folding options*/
void VSC_OPTN_LCSEOptions_SetDefault(
    IN OUT VSC_OPTN_LCSEOptions* options
    )
{
    VSC_OPTN_LCSEOptions_SetSwitchOn(options, gcvTRUE);
    VSC_OPTN_LCSEOptions_SetHeuristics(options, VSC_OPTN_LCSEOptions_HEUR_NONE ||
                                                VSC_OPTN_LCSEOptions_HEUR_LOAD_ONLY ||
                                                VSC_OPTN_LCSEOptions_HEUR_EXCLUDE_MUL);
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
        else if (gcvSTATUS_OK == gcoOS_StrNCmp(str, "heuristics:", sizeof("heuristics:") - 1))
        {
            gctUINT32 heuristics;
            gctUINT32 len;

            str += sizeof("heuristics:") - 1;
            len = _VSC_OPTN_GetSubOptionLength(str);
            heuristics = vscSTR_StrToUint32(str, len);
            VSC_OPTN_LCSEOptions_SetHeuristics(options, heuristics);
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
    VIR_LOG(dumper, "copy propagation options:\n");
    VIR_LOG(dumper, "    on: %s\n", VSC_OPTN_LCSEOptions_GetSwitchOn(options) ? "true" : "false");
    VIR_LOG(dumper, "    bs: %d\n", VSC_OPTN_LCSEOptions_GetBeforeShader(options));
    VIR_LOG(dumper, "    as: %d\n", VSC_OPTN_LCSEOptions_GetAfterShader(options));
    VIR_LOG(dumper, "    bf: %d\n", VSC_OPTN_LCSEOptions_GetBeforeFunc(options));
    VIR_LOG(dumper, "    af: %d\n", VSC_OPTN_LCSEOptions_GetAfterFunc(options));
    VIR_LOG(dumper, "    trace: %x\n", VSC_OPTN_LCSEOptions_GetTrace(options));
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_LCSEOptions_Usage(
    IN VIR_Dumper   *dumper
    )
{
    gctSTRING usage =
        "-LCSE:\n"
        "    on                  turn on constant propagation and folding\n"
        "    off                 turn off constant propagation and folding\n"
        "    trace:              0x1    trace constant propagation and folding input\n"
        "                        0x2    trace constant propagation and folding output\n"
        "                        0x4    trace constant propagation and folding algorithm\n\n";

    VIR_LOG(dumper, usage);
    VIR_LOG_FLUSH(dumper);
}

/* dead code elimination */
void VSC_OPTN_DCEOptions_SetDefault(
    IN OUT VSC_OPTN_DCEOptions* options
    )
{
    VSC_OPTN_DCEOptions_SetSwitchOn(options, gcvTRUE);
    VSC_OPTN_DCEOptions_SetOPTS(options, 0x1);
    VSC_OPTN_DCEOptions_SetTrace(options, 0);
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


/* final clean up phase options*/
void VSC_OPTN_FCPOptions_SetDefault(
    IN OUT VSC_OPTN_FCPOptions* options
    )
{
    VSC_OPTN_FCPOptions_SetSwitchOn(options, gcvTRUE);
    VSC_OPTN_FCPOptions_SetOPTS(options, 0x3);
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
    IN OUT VSC_OPTN_MCGenOptions* options
    )
{
    VSC_OPTN_MCGenOptions_SetSwitchOn(options, ENABLE_FULL_NEW_LINKER);
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
    IN OUT VSC_OPTN_SEPGenOptions* options
    )
{
    VSC_OPTN_SEPGenOptions_SetTrace(options, 0);
}

void VSC_OPTN_Options_SetDefault(
    IN OUT VSC_OPTN_Options* options
    )
{
    VSC_OPTN_HWOptions_SetDefault(VSC_OPTN_Options_GetHWOptions(options));
    VSC_OPTN_UF_AUBO_Options_SetDefault(VSC_OPTN_Options_GetAUBOOptions(options));
    VSC_OPTN_ILOptions_SetDefault(VSC_OPTN_Options_GetInlinerOptions(options));
    VSC_OPTN_LowerM2LOptions_SetDefault(VSC_OPTN_Options_GetLowerM2LOptions(options));
    VSC_OPTN_SCLOptions_SetDefault(VSC_OPTN_Options_GetSCLOptions(options));
    VSC_OPTN_PHOptions_SetDefault(VSC_OPTN_Options_GetPHOptions(options));
    VSC_OPTN_SIMPOptions_SetDefault(VSC_OPTN_Options_GetSIMPOptions(options));
    VSC_OPTN_ISOptions_SetDefault(VSC_OPTN_Options_GetPreRAISOptions(options));
    VSC_OPTN_ISOptions_SetPreRA(VSC_OPTN_Options_GetPreRAISOptions(options), gcvTRUE);
    VSC_OPTN_RAOptions_SetDefault(VSC_OPTN_Options_GetRAOptions(options));
    VSC_OPTN_ISOptions_SetDefault(VSC_OPTN_Options_GetPostRAISOptions(options));
    VSC_OPTN_CPPOptions_SetDefault(VSC_OPTN_Options_GetCPPOptions(options));
    VSC_OPTN_CPFOptions_SetDefault(VSC_OPTN_Options_GetCPFOptions(options));
    VSC_OPTN_LCSEOptions_SetDefault(VSC_OPTN_Options_GetLCSEOptions(options));
    VSC_OPTN_DCEOptions_SetDefault(VSC_OPTN_Options_GetDCEOptions(options));
    VSC_OPTN_FCPOptions_SetDefault(VSC_OPTN_Options_GetFCPOptions(options));
    VSC_OPTN_MCGenOptions_SetDefault(VSC_OPTN_Options_GetMCGenOptions(options));
    VSC_OPTN_SEPGenOptions_SetDefault(VSC_OPTN_Options_GetSEPGenOptions(options));
    /* temperarily switch of Post RA IS */
    VSC_OPTN_ISOptions_SetSwitchOn(VSC_OPTN_Options_GetPostRAISOptions(options), gcvFALSE);
    VSC_OPTN_ISOptions_SetPreRA(VSC_OPTN_Options_GetPostRAISOptions(options), gcvFALSE);
    VSC_OPTN_Options_SetDumpOptions(options, gcvFALSE);
    VSC_OPTN_Options_SetOptionsUsage(options, gcvFALSE);
}

void VSC_OPTN_Options_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_Options* options
    )
{
    gctSTRING pos = gcvNULL;

    /* hardware configuration: */
    gcoOS_StrStr(str, "-HW", &pos);
    if (pos)
    {
        pos += sizeof("-HW") - 1;
        VSC_OPTN_HWOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetHWOptions(options));
    }

    /* default UBO options */
    gcoOS_StrStr(str, "-DUBO", &pos);
    if (pos)
    {
        pos += sizeof("-DUBO") - 1;
        VSC_OPTN_UF_AUBO_Options_GetOptionFromString(pos, VSC_OPTN_Options_GetAUBOOptions(options));
    }

    /* inliner options */
    gcoOS_StrStr(str, "-IL", &pos);
    if (pos)
    {
        pos += sizeof("-IL") - 1;
        VSC_OPTN_ILOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetInlinerOptions(options));
    }

    /* lowering from mid level to low level Options: */
    gcoOS_StrStr(str, "-LOWERM2L", &pos);
    if (pos)
    {
        pos += sizeof("-LOWERM2L") - 1;
        VSC_OPTN_LowerM2LOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetLowerM2LOptions(options));
    }

    /* scalarization Options: */
    gcoOS_StrStr(str, "-SCL", &pos);
    if (pos)
    {
        pos += sizeof("-SCL") - 1;
        VSC_OPTN_SCLOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetSCLOptions(options));
    }

    /* copy propagation Options: */
    gcoOS_StrStr(str, "-CPP", &pos);
    if (pos)
    {
        pos += sizeof("-CPP") - 1;
        VSC_OPTN_CPPOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetCPPOptions(options));
    }

    /* constant propagation and folding Options: */
    gcoOS_StrStr(str, "-CPF", &pos);
    if (pos)
    {
        pos += sizeof("-CPF") - 1;
        VSC_OPTN_CPFOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetCPFOptions(options));
    }

    /* common subexpression elimination Options: */
    gcoOS_StrStr(str, "-LCSE", &pos);
    if (pos)
    {
        pos += sizeof("-LCSE") - 1;
        VSC_OPTN_LCSEOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetLCSEOptions(options));
    }

    /* dead code elimination Options: */
    gcoOS_StrStr(str, "-DCE", &pos);
    if (pos)
    {
        pos += sizeof("-DCE") - 1;
        VSC_OPTN_DCEOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetDCEOptions(options));
    }

    /* peephole Options: */
    gcoOS_StrStr(str, "-PH", &pos);
    if (pos)
    {
        pos += sizeof("-PH") - 1;
        VSC_OPTN_PHOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetPHOptions(options));
    }

    /* simplification Options: */
    gcoOS_StrStr(str, "-SIMP", &pos);
    if (pos)
    {
        pos += sizeof("-SIMP") - 1;
        VSC_OPTN_SIMPOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetSIMPOptions(options));
    }

    /* Pre RA instruction scheduling Options: */
    gcoOS_StrStr(str, "-IS", &pos);
    if (pos)
    {
        pos += sizeof("-IS") - 1;
        VSC_OPTN_ISOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetPreRAISOptions(options));
    }

    /* register allocation Options: */
    gcoOS_StrStr(str, "-RA", &pos);
    if (pos)
    {
        pos += sizeof("-RA") - 1;
        VSC_OPTN_RAOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetRAOptions(options));
    }

    /* final clean up phase Options: */
    gcoOS_StrStr(str, "-FCP", &pos);
    if (pos)
    {
        pos += sizeof("-FCP") - 1;
        VSC_OPTN_FCPOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetFCPOptions(options));
    }

    /* machine code generator: */
    gcoOS_StrStr(str, "-GEN", &pos);
    if (pos)
    {
        pos += sizeof("-GEN") - 1;
        VSC_OPTN_MCGenOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetMCGenOptions(options));
    }

    /* Post RA instruction scheduling Options: */
    gcoOS_StrStr(str, "-POSTRAIS", &pos);
    if (pos)
    {
        pos += sizeof("-POSTRAIS") - 1;
        VSC_OPTN_ISOptions_GetOptionFromString(pos, VSC_OPTN_Options_GetPostRAISOptions(options));
    }

    /* dump options */
    gcoOS_StrStr(str, "-DUMP_OPTIONS", &pos);
    if (pos)
    {
        VSC_OPTN_Options_SetDumpOptions(options, gcvTRUE);
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
    VSC_OPTN_HWOptions_Dump(VSC_OPTN_Options_GetHWOptions(options), dumper);
    VSC_OPTN_UF_AUBO_Options_Dump(VSC_OPTN_Options_GetAUBOOptions(options), dumper);
    VSC_OPTN_ILOptions_Dump(VSC_OPTN_Options_GetInlinerOptions(options), dumper);
    VSC_OPTN_LowerM2LOptions_Dump(VSC_OPTN_Options_GetLowerM2LOptions(options), dumper);
    VSC_OPTN_SCLOptions_Dump(VSC_OPTN_Options_GetSCLOptions(options), dumper);
    VSC_OPTN_CPPOptions_Dump(VSC_OPTN_Options_GetCPPOptions(options), dumper);
    VSC_OPTN_CPFOptions_Dump(VSC_OPTN_Options_GetCPFOptions(options), dumper);
    VSC_OPTN_LCSEOptions_Dump(VSC_OPTN_Options_GetLCSEOptions(options), dumper);
    VSC_OPTN_PHOptions_Dump(VSC_OPTN_Options_GetPHOptions(options), dumper);
    VSC_OPTN_SIMPOptions_Dump(VSC_OPTN_Options_GetSIMPOptions(options), dumper);
    VSC_OPTN_ISOptions_Dump(VSC_OPTN_Options_GetPreRAISOptions(options), dumper);
    VSC_OPTN_RAOptions_Dump(VSC_OPTN_Options_GetRAOptions(options), dumper);
    VSC_OPTN_ISOptions_Dump(VSC_OPTN_Options_GetPostRAISOptions(options), dumper);
    VSC_OPTN_FCPOptions_Dump(VSC_OPTN_Options_GetFCPOptions(options), dumper);
    VIR_LOG(dumper, "dump_options: %s\n", VSC_OPTN_Options_GetDumpOptions(options) ? "true" : "false");
    VIR_LOG(dumper, "options usage: %s\n", VSC_OPTN_Options_GetOptionsUsage(options) ? "true" : "false");
    VIR_LOG_FLUSH(dumper);
}

void VSC_OPTN_Options_Usage(
    IN VIR_Dumper* dumper
    )
{
    VIR_LOG(dumper, "%s\nOPTIONS USAGE\n%s\n", VSC_TRACE_BAR_LINE, VSC_TRACE_BAR_LINE);
    VSC_OPTN_HWOptions_Usage(dumper);
    VSC_OPTN_UF_AUBO_Options_Usage(dumper);
    VSC_OPTN_ILOptions_Usage(dumper);
    VSC_OPTN_LowerM2LOptions_Usage(dumper);
    VSC_OPTN_SCLOptions_Usage(dumper);
    VSC_OPTN_PHOptions_Usage(dumper);
    VSC_OPTN_CPPOptions_Usage(dumper);
    VSC_OPTN_CPFOptions_Usage(dumper);
    VSC_OPTN_LCSEOptions_Usage(dumper);
    VSC_OPTN_SIMPOptions_Usage(dumper);
    VSC_OPTN_ISOptions_Usage(dumper);
    VSC_OPTN_RAOptions_Usage(dumper);
    VSC_OPTN_FCPOptions_Usage(dumper);
    VIR_LOG(dumper, "-DUMP_OPTIONS        dump options\n");
    VIR_LOG(dumper, "-USAGE               print options usage\n");
    VIR_LOG_FLUSH(dumper);
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

