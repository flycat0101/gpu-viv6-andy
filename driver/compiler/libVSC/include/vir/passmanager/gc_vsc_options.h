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


#ifndef __gc_vsc_options_h_
#define __gc_vsc_options_h_

BEGIN_EXTERN_C()

struct _VIR_DUMPER;

typedef struct _VSC_OPTN_BASE
{
    gctBOOL   switch_on;
    gctUINT   passId;
    gctTRACE  trace;
} VSC_OPTN_BASE;

#define VSC_OPTN_GetSwitchOn(baseOptn)                    ((baseOptn)->switch_on)
#define VSC_OPTN_SetSwitchOn(baseOptn, s)                 ((baseOptn)->switch_on = (s))
#define VSC_OPTN_GetPassId(baseOptn)                      ((baseOptn)->passId)
#define VSC_OPTN_SetPassId(baseOptn, p)                   ((baseOptn)->passId = (p))
#define VSC_OPTN_GetTrace(baseOptn)                       ((baseOptn)->trace)
#define VSC_OPTN_SetTrace(baseOptn, t)                    ((baseOptn)->trace = (t))

/* Simple Copy Propagation Options */
typedef struct _VSC_OPTN_SCPPOPTIONS
{
    /* Must be first element */
    VSC_OPTN_BASE optnBase;

    gctUINT32 before_shader;
    gctUINT32 after_shader;
} VSC_OPTN_SCPPOptions;

#define VSC_OPTN_SCPPOptions_GetSwitchOn(option)                VSC_OPTN_GetSwitchOn(&(option)->optnBase)
#define VSC_OPTN_SCPPOptions_SetSwitchOn(option, s)             VSC_OPTN_SetSwitchOn(&(option)->optnBase, (s))
#define VSC_OPTN_SCPPOptions_GetPassId(option)                  VSC_OPTN_GetPassId(&(option)->optnBase)
#define VSC_OPTN_SCPPOptions_SetPassId(option, s)               VSC_OPTN_SetPassId(&(option)->optnBase, (s))
#define VSC_OPTN_SCPPOptions_GetTrace(option)                   VSC_OPTN_GetTrace(&(option)->optnBase)
#define VSC_OPTN_SCPPOptions_SetTrace(option, t)                VSC_OPTN_SetTrace(&(option)->optnBase, (t))

#define VSC_OPTN_SCPPOptions_TRACE_INPUT_SHADER                 0x1
#define VSC_OPTN_SCPPOptions_TRACE_INPUT_FUNC                   0x2
#define VSC_OPTN_SCPPOptions_TRACE_INPUT_BB                     0x4
#define VSC_OPTN_SCPPOptions_TRACE_DETAIL                       0x8
#define VSC_OPTN_SCPPOptions_TRACE_OUTPUT_BB                    0x10
#define VSC_OPTN_SCPPOptions_TRACE_OUTPUT_FUNC                  0x20
#define VSC_OPTN_SCPPOptions_TRACE_OUTPUT_SHADER                0x40

#define VSC_OPTN_SCPPOptions_GetBeforeShader(option)            ((option)->before_shader)
#define VSC_OPTN_SCPPOptions_SetBeforeShader(option, b)         ((option)->before_shader = (b))
#define VSC_OPTN_SCPPOptions_GetAfterShader(option)             ((option)->after_shader)
#define VSC_OPTN_SCPPOptions_SetAfterShader(option, a)          ((option)->after_shader = (a))

void VSC_OPTN_SCPPOptions_SetDefault(
    IN OUT VSC_OPTN_SCPPOptions* options,
    IN gctUINT optLevel
    );

void VSC_OPTN_SCPPOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_SCPPOptions* options
    );

void VSC_OPTN_SCPPOptions_Dump(
    IN VSC_OPTN_SCPPOptions* options,
    IN struct _VIR_DUMPER* dumper
    );

void VSC_OPTN_SCPPOptions_Usage(
    IN struct _VIR_DUMPER* dumper
    );

/* Long Parameter Optimization Options */
typedef struct _VSC_OPTN_PARAMOPTOPTIONS
{
    /* Must be first element */
    VSC_OPTN_BASE optnBase;
    gctUINT longArrayThreshold;
    gctUINT32 before_shader;
    gctUINT32 after_shader;
} VSC_OPTN_ParamOptOptions;

#define VSC_OPTN_ParamOptOptions_GetSwitchOn(option)                VSC_OPTN_GetSwitchOn(&(option)->optnBase)
#define VSC_OPTN_ParamOptOptions_SetSwitchOn(option, s)             VSC_OPTN_SetSwitchOn(&(option)->optnBase, (s))
#define VSC_OPTN_ParamOptOptions_GetPassId(option)                  VSC_OPTN_GetPassId(&(option)->optnBase)
#define VSC_OPTN_ParamOptOptions_SetPassId(option, s)               VSC_OPTN_SetPassId(&(option)->optnBase, (s))
#define VSC_OPTN_ParamOptOptions_GetTrace(option)                   VSC_OPTN_GetTrace(&(option)->optnBase)
#define VSC_OPTN_ParamOptOptions_SetTrace(option, t)                VSC_OPTN_SetTrace(&(option)->optnBase, (t))

#define VSC_OPTN_ParamOptOptions_TRACE_INPUT_SHADER                 0x1
#define VSC_OPTN_ParamOptOptions_TRACE_INPUT_FUNC                   0x2
#define VSC_OPTN_ParamOptOptions_TRACE_INPUT_BB                     0x4
#define VSC_OPTN_ParamOptOptions_TRACE_DETAIL                       0x8
#define VSC_OPTN_ParamOptOptions_TRACE_OUTPUT_BB                    0x10
#define VSC_OPTN_ParamOptOptions_TRACE_OUTPUT_FUNC                  0x20
#define VSC_OPTN_ParamOptOptions_TRACE_OUTPUT_SHADER                0x40

#define VSC_OPTN_ParamOptOptions_GetLongArrayThreshold(option)            ((option)->longArrayThreshold)
#define VSC_OPTN_ParamOptOptions_SetLongArrayThreshold(option, l)         ((option)->longArrayThreshold = (l))
#define VSC_OPTN_ParamOptOptions_GetBeforeShader(option)            ((option)->before_shader)
#define VSC_OPTN_ParamOptOptions_SetBeforeShader(option, b)         ((option)->before_shader = (b))
#define VSC_OPTN_ParamOptOptions_GetAfterShader(option)             ((option)->after_shader)
#define VSC_OPTN_ParamOptOptions_SetAfterShader(option, a)          ((option)->after_shader = (a))

#define VSC_OPTN_ParamOptOptions_TRACE 0x1

void VSC_OPTN_ParamOptOptions_SetDefault(
    IN OUT VSC_OPTN_ParamOptOptions* options,
    IN gctUINT longArrayThreshold
    );

void VSC_OPTN_ParamOptOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_ParamOptOptions* options
    );

void VSC_OPTN_ParamOptOptions_Dump(
    IN VSC_OPTN_ParamOptOptions* options,
    IN struct _VIR_DUMPER* dumper
    );

void VSC_OPTN_ParamOptOptions_Usage(
    IN struct _VIR_DUMPER* dumper
    );


/* Loop Optimizations Options */
typedef struct _VSC_OPTN_LOOPOPTSOPTIONS
{
    /* Must be first element */
    VSC_OPTN_BASE optnBase;

    gctUINT32 opts;
    gctINT32 totalUnrollingFactor;
    gctINT32 fullUnrollingFactor;
    gctINT32 partialUnrollingFactor;
    gctINT32 licmFactor;
    gctUINT32 before_shader;
    gctUINT32 after_shader;
} VSC_OPTN_LoopOptsOptions;

#define VSC_OPTN_LoopOptsOptions_GetSwitchOn(option)                    VSC_OPTN_GetSwitchOn(&(option)->optnBase)
#define VSC_OPTN_LoopOptsOptions_SetSwitchOn(option, s)                 VSC_OPTN_SetSwitchOn(&(option)->optnBase, (s))

#define VSC_OPTN_LoopOptsOptions_OPTS_NONE                              0x0
#define VSC_OPTN_LoopOptsOptions_OPTS_LOOP_INVERSION                    0x1
#define VSC_OPTN_LoopOptsOptions_OPTS_LOOP_INVARIANT                    0x2
#define VSC_OPTN_LoopOptsOptions_OPTS_LOOP_UNROLLING                    0x4

#define VSC_OPTN_LoopOptsOptions_GetOpts(option)                        ((option)->opts)
#define VSC_OPTN_LoopOptsOptions_SetOpts(option, o)                     ((option)->opts = (o))
#define VSC_OPTN_LoopOptsOptions_GetTotalUnrollingFactor(option)        ((option)->totalUnrollingFactor)
#define VSC_OPTN_LoopOptsOptions_SetTotalUnrollingFactor(option, f)     ((option)->totalUnrollingFactor = (f))
#define VSC_OPTN_LoopOptsOptions_GetFullUnrollingFactor(option)         ((option)->fullUnrollingFactor)
#define VSC_OPTN_LoopOptsOptions_SetFullUnrollingFactor(option, f)      ((option)->fullUnrollingFactor = (f))
#define VSC_OPTN_LoopOptsOptions_GetPartialUnrollingFactor(option)      ((option)->partialUnrollingFactor)
#define VSC_OPTN_LoopOptsOptions_SetPartialUnrollingFactor(option, p)   ((option)->partialUnrollingFactor = (p))
#define VSC_OPTN_LoopOptsOptions_GetLICMFactor(option)                  ((option)->licmFactor)
#define VSC_OPTN_LoopOptsOptions_SetLICMFactor(option, p)               ((option)->licmFactor = (p))
#define VSC_OPTN_LoopOptsOptions_GetTrace(option)                       VSC_OPTN_GetTrace(&(option)->optnBase)
#define VSC_OPTN_LoopOptsOptions_SetTrace(option, t)                    VSC_OPTN_SetTrace(&(option)->optnBase, (t))

#define VSC_OPTN_LoopOptsOptions_TRACE_SHADER_INPUT                     0x1
#define VSC_OPTN_LoopOptsOptions_TRACE_FUNC_INPUT                       0x2
#define VSC_OPTN_LoopOptsOptions_TRACE_FUNC_LOOP_ANALYSIS               0x4
#define VSC_OPTN_LoopOptsOptions_TRACE_INVERSION_FUNC_INPUT             0x8
#define VSC_OPTN_LoopOptsOptions_TRACE_INVERSION                        0x10
#define VSC_OPTN_LoopOptsOptions_TRACE_INVERSION_FUNC_OUTPUT            0x20
#define VSC_OPTN_LoopOptsOptions_TRACE_INVARIANT_FUNC_INPUT             0x40
#define VSC_OPTN_LoopOptsOptions_TRACE_INVARIANT                        0x80
#define VSC_OPTN_LoopOptsOptions_TRACE_INVARIANT_FUNC_OUTPUT            0x100
#define VSC_OPTN_LoopOptsOptions_TRACE_UNROLL_FUNC_INPUT                0x200
#define VSC_OPTN_LoopOptsOptions_TRACE_UNROLL                           0x400
#define VSC_OPTN_LoopOptsOptions_TRACE_UNROLL_FUNC_OUTPUT               0x800
#define VSC_OPTN_LoopOptsOptions_TRACE_FUNC_OUTPUT                      0x1000
#define VSC_OPTN_LoopOptsOptions_TRACE_SHADER_OUTPUT                    0x2000

#define VSC_OPTN_LoopOptsOptions_GetBeforeShader(option)                ((option)->before_shader)
#define VSC_OPTN_LoopOptsOptions_SetBeforeShader(option, b)             ((option)->before_shader = (b))
#define VSC_OPTN_LoopOptsOptions_GetAfterShader(option)                 ((option)->after_shader)
#define VSC_OPTN_LoopOptsOptions_SetAfterShader(option, a)              ((option)->after_shader = (a))

void VSC_OPTN_LoopOptsOptions_SetDefault(
    IN OUT VSC_OPTN_LoopOptsOptions* options,
    IN gctUINT optLevel
    );

void VSC_OPTN_LoopOptsOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_LoopOptsOptions* options
    );

void VSC_OPTN_LoopOptsOptions_Dump(
    IN VSC_OPTN_LoopOptsOptions* options,
    IN struct _VIR_DUMPER* dumper
    );

void VSC_OPTN_LoopOptsOptions_Usage(
    IN struct _VIR_DUMPER* dumper
    );

/* Control Flow Optimizations Options */
typedef struct _VSC_OPTN_CFOOPTIONS
{
    /* Must be first element */
    VSC_OPTN_BASE optnBase;

    gctUINT32 opts;
    gctUINT32 before_shader;
    gctUINT32 after_shader;
} VSC_OPTN_CFOOptions;

#define VSC_OPTN_CFOOptions_GetSwitchOn(option)            VSC_OPTN_GetSwitchOn(&(option)->optnBase)
#define VSC_OPTN_CFOOptions_SetSwitchOn(option, s)         VSC_OPTN_SetSwitchOn(&(option)->optnBase, (s))
#define VSC_OPTN_CFOOptions_GetPassId(option)              VSC_OPTN_GetPassId(&(option)->optnBase)
#define VSC_OPTN_CFOOptions_SetPassId(option, p)           VSC_OPTN_SetPassId(&(option)->optnBase, (p))

#define VSC_OPTN_CFOOptions_OPTS_NONE                      0x0
#define VSC_OPTN_CFOOptions_OPTS_PATTERN                   0x1
#define VSC_OPTN_CFOOptions_OPTS_GEN_SELECT                0x2

#define VSC_OPTN_CFOOptions_GetOpts(option)                ((option)->opts)
#define VSC_OPTN_CFOOptions_SetOpts(option, o)             ((option)->opts = (o))
#define VSC_OPTN_CFOOptions_GetTrace(option)               VSC_OPTN_GetTrace(&(option)->optnBase)
#define VSC_OPTN_CFOOptions_SetTrace(option, t)            VSC_OPTN_SetTrace(&(option)->optnBase, (t))

#define VSC_OPTN_CFOOptions_TRACE_SHADER_INPUT             0x1
#define VSC_OPTN_CFOOptions_TRACE_FUNC_INPUT               0x2
#define VSC_OPTN_CFOOptions_TRACE_PATTERN_INPUT            0x4
#define VSC_OPTN_CFOOptions_TRACE_PATTERN_DETAIL           0x8
#define VSC_OPTN_CFOOptions_TRACE_PATTERN_OUTPUT           0x10
#define VSC_OPTN_CFOOptions_TRACE_GEN_SELECT_INPUT         0x20
#define VSC_OPTN_CFOOptions_TRACE_GEN_SELECT_DETAIL        0x40
#define VSC_OPTN_CFOOptions_TRACE_GEN_SELECT_OUTPUT        0x80
#define VSC_OPTN_CFOOptions_TRACE_FUNC_OUTPUT              0x100
#define VSC_OPTN_CFOOptions_TRACE_SHADER_OUTPUT            0x200

#define VSC_OPTN_CFOOptions_GetBeforeShader(option)      ((option)->before_shader)
#define VSC_OPTN_CFOOptions_SetBeforeShader(option, b)   ((option)->before_shader = (b))
#define VSC_OPTN_CFOOptions_GetAfterShader(option)       ((option)->after_shader)
#define VSC_OPTN_CFOOptions_SetAfterShader(option, a)    ((option)->after_shader = (a))

void VSC_OPTN_CFOOptions_SetDefault(
    IN OUT VSC_OPTN_CFOOptions* options,
    IN gctUINT optLevel
    );

void VSC_OPTN_CFOOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_CFOOptions* options
    );

void VSC_OPTN_CFOOptions_Dump(
    IN VSC_OPTN_CFOOptions* options,
    IN struct _VIR_DUMPER* dumper
    );

void VSC_OPTN_CFOOptions_Usage(
    IN struct _VIR_DUMPER* dumper
    );

/* Uniform Rearrange Options */
typedef struct _VSC_OPTN_UF_AUBO_OPTIONS
{
    /* Must be first element */
    VSC_OPTN_BASE optnBase;

    gctUINT32 heuristics;
    gctUINT32 const_reg_reservation;
    gctUINT32 opt;
    gctUINT32 before_shader;
    gctUINT32 after_shader;
} VSC_OPTN_UF_AUBOOptions;

#define VSC_OPTN_UF_AUBOOptions_GetSwitchOn(option)                    VSC_OPTN_GetSwitchOn(&(option)->optnBase)
#define VSC_OPTN_UF_AUBOOptions_SetSwitchOn(option, s)                 VSC_OPTN_SetSwitchOn(&(option)->optnBase, (s))

#define VSC_OPTN_UF_AUBOOptions_HEUR_FORCE_ALL               0x1
#define VSC_OPTN_UF_AUBOOptions_HEUR_INDIRECT_IMM            0x2
#define VSC_OPTN_UF_AUBOOptions_HEUR_INDIRECT_UD             0x4
#define VSC_OPTN_UF_AUBOOptions_HEUR_INDIRECT_D              0x6
#define VSC_OPTN_UF_AUBOOptions_HEUR_INDIRECT_MASK           0x6
#define VSC_OPTN_UF_AUBOOptions_HEUR_ORDERLY                 0x8
#define VSC_OPTN_UF_AUBOOptions_HEUR_SKIP_DUBO               0x10
#define VSC_OPTN_UF_AUBOOptions_HEUR_SKIP_CUBO               0x20

#define VSC_OPTN_UF_AUBOOptions_GetHeuristics(option)                  ((option)->heuristics)
#define VSC_OPTN_UF_AUBOOptions_SetHeuristics(option, h)               ((option)->heuristics = (h))
#define VSC_OPTN_UF_AUBOOptions_GetIndirectAccessLevel(option)         (((option)->heuristics & VSC_OPTN_UF_AUBOOptions_HEUR_INDIRECT_MASK) >> 1)
#define VSC_OPTN_UF_AUBOOptions_SetIndirectAccessLevel(option, ial)    ((option)->heuristics |= ((ial) << 1))
#define VSC_OPTN_UF_AUBOOptions_GetConstRegReservation(option)         ((option)->const_reg_reservation)
#define VSC_OPTN_UF_AUBOOptions_SetConstRegReservation(option, c)      ((option)->const_reg_reservation = (c))
#define VSC_OPTN_UF_AUBOOptions_GetOpt(option)                         ((option)->opt)
#define VSC_OPTN_UF_AUBOOptions_SetOpt(option, o)                      ((option)->opt = (o))

#define VSC_OPTN_UF_AUBOOptions_GetTrace(option)             VSC_OPTN_GetTrace(&(option)->optnBase)
#define VSC_OPTN_UF_AUBOOptions_SetTrace(option, t)          VSC_OPTN_SetTrace(&(option)->optnBase, (t))

#define VSC_OPTN_UF_AUBOOptions_TRACE_INPUT                  0x1
#define VSC_OPTN_UF_AUBOOptions_TRACE_INITIALIZE             0x2
#define VSC_OPTN_UF_AUBOOptions_TRACE_MARKACTIVE             0x4
#define VSC_OPTN_UF_AUBOOptions_TRACE_INDIRECT               0x8
#define VSC_OPTN_UF_AUBOOptions_TRACE_DUBSIZE                0x10
#define VSC_OPTN_UF_AUBOOptions_TRACE_PICK                   0x20
#define VSC_OPTN_UF_AUBOOptions_TRACE_CONSTRUCTION           0x40
#define VSC_OPTN_UF_AUBOOptions_TRACE_FILL                   0x80
#define VSC_OPTN_UF_AUBOOptions_TRACE_TRANSFORM              0x100
#define VSC_OPTN_UF_AUBOOptions_TRACE_TRANSFORM_BUILD        0x200
#define VSC_OPTN_UF_AUBOOptions_TRACE_TRANSFORM_REORG        0x400
#define VSC_OPTN_UF_AUBOOptions_TRACE_TRANSFORM_INSERT       0x800
#define VSC_OPTN_UF_AUBOOptions_TRACE_OUTPUT                 0x1000

#define VSC_OPTN_UF_AUBOOptions_GetBeforeShader(option)      ((option)->before_shader)
#define VSC_OPTN_UF_AUBOOptions_SetBeforeShader(option, b)   ((option)->before_shader = (b))
#define VSC_OPTN_UF_AUBOOptions_GetAfterShader(option)       ((option)->after_shader)
#define VSC_OPTN_UF_AUBOOptions_SetAfterShader(option, a)    ((option)->after_shader = (a))

void VSC_OPTN_UF_AUBOOptions_SetDefault(
    IN OUT VSC_OPTN_UF_AUBOOptions* options,
    IN gctUINT optLevel
    );

void VSC_OPTN_UF_AUBOOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_UF_AUBOOptions* options
    );

void VSC_OPTN_UF_AUBOOptions_Dump(
    IN VSC_OPTN_UF_AUBOOptions* options,
    IN struct _VIR_DUMPER* dumper
    );

void VSC_OPTN_UF_AUBOOptions_Usage(
    IN struct _VIR_DUMPER* dumper
    );

/* Inliner options */
typedef struct _VSC_OPTN_ILOPTIONS
{
    /* Must be first element */
    VSC_OPTN_BASE optnBase;

    gctUINT32 heuristics;

    /* inline level */
    gctUINT32 level;

} VSC_OPTN_ILOptions;

#define VSC_OPTN_ILOptions_GetSwitchOn(option)          VSC_OPTN_GetSwitchOn(&(option)->optnBase)
#define VSC_OPTN_ILOptions_SetSwitchOn(option, s)       VSC_OPTN_SetSwitchOn(&(option)->optnBase, (s))

#define VSC_OPTN_ILOptions_GetHeuristics(option)        ((option)->heuristics)
#define VSC_OPTN_ILOptions_SetHeuristics(option, h)     ((option)->heuristics = (h))

#define VSC_OPTN_ILOptions_CALL_DEPTH                    0x1
#define VSC_OPTN_ILOptions_TOP_DOWN                      0x2

#define VSC_OPTN_ILOptions_GetInlineLevel(option)        ((option)->level)
#define VSC_OPTN_ILOptions_SetInlineLevel(option, h)     ((option)->level = (h))

#define VSC_OPTN_ILOptions_LEVEL0                        0x0    /* disable inline */
#define VSC_OPTN_ILOptions_LEVEL1                        0x1    /* inline ALWAYSINLINE function only. */
#define VSC_OPTN_ILOptions_LEVEL2                        0x2    /* minimal inline */
#define VSC_OPTN_ILOptions_LEVEL3                        0x3    /* default inline */
#define VSC_OPTN_ILOptions_LEVEL4                        0x4    /* maximal inline */

#define VSC_OPTN_ILOptions_GetTrace(option)             VSC_OPTN_GetTrace(&(option)->optnBase)
#define VSC_OPTN_ILOptions_SetTrace(option, t)          VSC_OPTN_SetTrace(&(option)->optnBase, (t))

#define VSC_OPTN_ILOptions_TRACE                        0x1

void VSC_OPTN_ILOptions_SetDefault(
    IN OUT VSC_OPTN_ILOptions* options,
    IN gctUINT optLevel
    );

void VSC_OPTN_ILOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_ILOptions* options
    );

void VSC_OPTN_ILOptions_Dump(
    IN VSC_OPTN_ILOptions* options,
    IN struct _VIR_DUMPER* dumper
    );

void VSC_OPTN_ILOptions_Usage(
    IN struct _VIR_DUMPER* dumper
    );

/* Precision Updater options */
typedef struct _VSC_OPTN_PUOPTIONS
{
    /* Must be first element */
    VSC_OPTN_BASE optnBase;
} VSC_OPTN_PUOptions;

#define VSC_OPTN_PUOptions_GetSwitchOn(option)          VSC_OPTN_GetSwitchOn(&(option)->optnBase)
#define VSC_OPTN_PUOptions_SetSwitchOn(option, s)       VSC_OPTN_SetSwitchOn(&(option)->optnBase, (s))
#define VSC_OPTN_PUOptions_GetTrace(option)             VSC_OPTN_GetTrace(&(option)->optnBase)
#define VSC_OPTN_PUOptions_SetTrace(option, t)          VSC_OPTN_SetTrace(&(option)->optnBase, (t))

#define VSC_OPTN_PUOptions_TRACE                        0x1

void VSC_OPTN_PUOptions_SetDefault(
    IN OUT VSC_OPTN_PUOptions* options,
    IN gctUINT optLevel
    );

void VSC_OPTN_PUOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_PUOptions* options
    );

void VSC_OPTN_PUOptions_Dump(
    IN VSC_OPTN_PUOptions* options,
    IN struct _VIR_DUMPER* dumper
    );

void VSC_OPTN_PUOptions_Usage(
    IN struct _VIR_DUMPER* dumper
    );

/* Simplification options */
typedef struct _VSC_OPTN_SIMPOPTIONS
{
    /* Must be first element */
    VSC_OPTN_BASE optnBase;

    gctUINT32 before_shader;
    gctUINT32 after_shader;
    gctUINT32 before_func;
    gctUINT32 after_func;
    gctUINT32 before_inst;
    gctUINT32 after_inst;
} VSC_OPTN_SIMPOptions;

#define VSC_OPTN_SIMPOptions_GetSwitchOn(option)            VSC_OPTN_GetSwitchOn(&(option)->optnBase)
#define VSC_OPTN_SIMPOptions_SetSwitchOn(option, s)         VSC_OPTN_SetSwitchOn(&(option)->optnBase, (s))
#define VSC_OPTN_SIMPOptions_GetBeforeShader(option)        ((option)->before_shader)
#define VSC_OPTN_SIMPOptions_SetBeforeShader(option, b)     ((option)->before_shader = (b))
#define VSC_OPTN_SIMPOptions_GetAfterShader(option)         ((option)->after_shader)
#define VSC_OPTN_SIMPOptions_SetAfterShader(option, a)      ((option)->after_shader = (a))
#define VSC_OPTN_SIMPOptions_GetBeforeFunc(option)          ((option)->before_func)
#define VSC_OPTN_SIMPOptions_SetBeforeFunc(option, b)       ((option)->before_func = (b))
#define VSC_OPTN_SIMPOptions_GetAfterFunc(option)           ((option)->after_func)
#define VSC_OPTN_SIMPOptions_SetAfterFunc(option, a)        ((option)->after_func = (a))
#define VSC_OPTN_SIMPOptions_GetBeforeInst(option)          ((option)->before_inst)
#define VSC_OPTN_SIMPOptions_SetBeforeInst(option, b)       ((option)->before_inst = (b))
#define VSC_OPTN_SIMPOptions_GetAfterInst(option)           ((option)->after_inst)
#define VSC_OPTN_SIMPOptions_SetAfterInst(option, a)        ((option)->after_inst = (a))
#define VSC_OPTN_SIMPOptions_GetTrace(option)               VSC_OPTN_GetTrace(&(option)->optnBase)
#define VSC_OPTN_SIMPOptions_SetTrace(option, t)            VSC_OPTN_SetTrace(&(option)->optnBase, (t))

#define VSC_OPTN_SIMPOptions_TRACE_INPUT_SHADER             0x1
#define VSC_OPTN_SIMPOptions_TRACE_OUTPUT_SHADER            0x2
#define VSC_OPTN_SIMPOptions_TRACE_INPUT_FUNCTION           0x4
#define VSC_OPTN_SIMPOptions_TRACE_OUTPUT_FUNCTION          0x8
#define VSC_OPTN_SIMPOptions_TRACE_INPUT_BB                 0x10
#define VSC_OPTN_SIMPOptions_TRACE_OUTPUT_BB                0x20
#define VSC_OPTN_SIMPOptions_TRACE_TRANSFORMATION           0x40

void VSC_OPTN_SIMPOptions_SetDefault(
    IN OUT VSC_OPTN_SIMPOptions* options,
    IN gctUINT optLevel
    );

void VSC_OPTN_SIMPOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_SIMPOptions* options
    );

void VSC_OPTN_SIMPOptions_Dump(
    IN VSC_OPTN_SIMPOptions* options,
    IN struct _VIR_DUMPER* dumper
    );

void VSC_OPTN_SIMPOptions_Usage(
    IN struct _VIR_DUMPER* dumper
    );

/* Lowering from Mid Level to Low Level options */
typedef struct _VSC_OPTN_LOWERM2L
{
    /* Must be first element */
    VSC_OPTN_BASE optnBase;
} VSC_OPTN_LowerM2LOptions;

#define VSC_OPTN_LowerM2LOptions_GetSwitchOn(option)        VSC_OPTN_GetSwitchOn(&(option)->optnBase)
#define VSC_OPTN_LowerM2LOptions_SetSwitchOn(option, s)     VSC_OPTN_SetSwitchOn(&(option)->optnBase, (s))
#define VSC_OPTN_LowerM2LOptions_GetTrace(option)           VSC_OPTN_GetTrace(&(option)->optnBase)
#define VSC_OPTN_LowerM2LOptions_SetTrace(option, t)        VSC_OPTN_SetTrace(&(option)->optnBase, (t))

void VSC_OPTN_LowerM2LOptions_SetDefault(
    IN OUT VSC_OPTN_LowerM2LOptions* options,
    IN gctUINT optLevel
    );

void VSC_OPTN_LowerM2LOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_LowerM2LOptions* options
    );

void VSC_OPTN_LowerM2LOptions_Dump(
    IN VSC_OPTN_LowerM2LOptions* options_is,
    IN struct _VIR_DUMPER* dumper
    );

void VSC_OPTN_LowerM2LOptions_Usage(
    IN struct _VIR_DUMPER* dumper
    );

/* Scalarization options */
typedef struct _VSC_OPTN_SCLOPTIONS
{
    /* Must be first element */
    VSC_OPTN_BASE optnBase;
} VSC_OPTN_SCLOptions;

#define VSC_OPTN_SCLOptions_GetSwitchOn(option)         VSC_OPTN_GetSwitchOn(&(option)->optnBase)
#define VSC_OPTN_SCLOptions_SetSwitchOn(option, s)      VSC_OPTN_SetSwitchOn(&(option)->optnBase, (s))
#define VSC_OPTN_SCLOptions_GetTrace(option)            VSC_OPTN_GetTrace(&(option)->optnBase)
#define VSC_OPTN_SCLOptions_SetTrace(option, t)         VSC_OPTN_SetTrace(&(option)->optnBase, (t))
#define VSC_OPTN_SCLOptions_TRACE_INPUT_SHADER          0x1
#define VSC_OPTN_SCLOptions_TRACE_OUTPUT_SHADER         0x2
#define VSC_OPTN_SCLOptions_TRACE_INPUT_FUNCTIONS       0x4
#define VSC_OPTN_SCLOptions_TRACE_OUTPUT_FUNCTIONS      0x8
#define VSC_OPTN_SCLOptions_TRACE_COLLECTED_INFO        0x10
#define VSC_OPTN_SCLOptions_TRACE_DUMP_NEW_SYMBOLS      0x20

void VSC_OPTN_SCLOptions_SetDefault(
    IN OUT VSC_OPTN_SCLOptions* options,
    IN gctUINT optLevel
    );

void VSC_OPTN_SCLOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_SCLOptions* options
    );

void VSC_OPTN_SCLOptions_Dump(
    IN VSC_OPTN_SCLOptions* options_is,
    IN struct _VIR_DUMPER* dumper
    );

void VSC_OPTN_SCLOptions_Usage(
    IN struct _VIR_DUMPER* dumper
    );

/* Peephole options */
typedef struct _VSC_OPTN_PHOPTIONS
{
    /* Must be first element */
    VSC_OPTN_BASE optnBase;

    gctUINT32 opts;
    gctUINT32 modifiers;
    gctUINT32 before_shader;
    gctUINT32 after_shader;
    gctUINT32 before_func;
    gctUINT32 after_func;
    gctUINT32 before_bb;
    gctUINT32 after_bb;
    gctUINT32 before_inst;
    gctUINT32 after_inst;
} VSC_OPTN_PHOptions;

#define VSC_OPTN_PHOptions_GetSwitchOn(option)          VSC_OPTN_GetSwitchOn(&(option)->optnBase)
#define VSC_OPTN_PHOptions_SetSwitchOn(option, s)       VSC_OPTN_SetSwitchOn(&(option)->optnBase, (s))

#define VSC_OPTN_PHOptions_GetOPTS(option)              ((option)->opts)
#define VSC_OPTN_PHOptions_SetOPTS(option, o)           ((option)->opts = (o))
#define VSC_OPTN_PHOptions_OPTS_MODIFIER                0x1
#define VSC_OPTN_PHOptions_OPTS_MAD                     0x2
#define VSC_OPTN_PHOptions_OPTS_RSQ                     0x4
#define VSC_OPTN_PHOptions_OPTS_VEC                     0x8
/* remove unused checking instruction, i.e., change JMPC to JMP */
#define VSC_OPTN_PHOptions_OPTS_RUC                     0x10
#define VSC_OPTN_PHOptions_OPTS_MOV_ATOM                0x20
#define VSC_OPTN_PHOptions_OPTS_MOV_LDARR               0x40
#define VSC_OPTN_PHOptions_OPTS_MOV_DEF                 (VSC_OPTN_PHOptions_OPTS_MOV_ATOM | VSC_OPTN_PHOptions_OPTS_MOV_LDARR)
#define VSC_OPTN_PHOptions_OPTS_LSHIFT_LS               0x80
#define VSC_OPTN_PHOptions_OPTS_LOC_MEM                 0x100
#define VSC_OPTN_PHOptions_OPTS_ADD_MEM_ADDR            0x200
#define VSC_OPTN_PHOptions_OPTS_REDUNDANT_MOV_DEF       0x400

#define VSC_OPTN_PHOptions_GetModifiers(option)         ((option)->modifiers)
#define VSC_OPTN_PHOptions_SetModifiers(option, m)      ((option)->modifiers = (m))
#define VSC_OPTN_PHOptions_MODIFIERS_SAT                0x1
#define VSC_OPTN_PHOptions_MODIFIERS_NEG                0x2
#define VSC_OPTN_PHOptions_MODIFIERS_ABS                0x4
#define VSC_OPTN_PHOptions_MODIFIERS_CONJ               0x8

#define VSC_OPTN_PHOptions_GetTrace(option)             VSC_OPTN_GetTrace(&(option)->optnBase)
#define VSC_OPTN_PHOptions_SetTrace(option, t)          VSC_OPTN_SetTrace(&(option)->optnBase, (t))
#define VSC_OPTN_PHOptions_TRACE_INPUT_CFG              0x1
#define VSC_OPTN_PHOptions_TRACE_INPUT_BB               0x2
#define VSC_OPTN_PHOptions_TRACE_MODIFIER               0x4
#define VSC_OPTN_PHOptions_TRACE_MAD                    0x8
#define VSC_OPTN_PHOptions_TRACE_OUTPUT_BB              0x10
#define VSC_OPTN_PHOptions_TRACE_OUTPUT_CFG             0x20
#define VSC_OPTN_PHOptions_TRACE_VEC                    0x40
#define VSC_OPTN_PHOptions_TRACE_RSQ                    0x80
#define VSC_OPTN_PHOptions_TRACE_RUC                    0x100
#define VSC_OPTN_PHOptions_TRACE_MOV_ATOM               0x200
#define VSC_OPTN_PHOptions_TRACE_MOV_LDARR              0x400
#define VSC_OPTN_PHOptions_TRACE_MOV_DEF                (VSC_OPTN_PHOptions_TRACE_MOV_ATOM | VSC_OPTN_PHOptions_TRACE_MOV_LDARR)
#define VSC_OPTN_PHOptions_TRACE_LSHIFT_LS              0x800
#define VSC_OPTN_PHOptions_TRACE_DP                     0x1000
#define VSC_OPTN_PHOptions_TRACE_INPUT_INST             0x2000
#define VSC_OPTN_PHOptions_TRACE_BUILT_TREE             0x4000
#define VSC_OPTN_PHOptions_TRACE_REQUIREMENTS           0x8000
#define VSC_OPTN_PHOptions_TRACE_TRANSFORMS             0x10000
#define VSC_OPTN_PHOptions_TRACE_MERGING                0x20000
#define VSC_OPTN_PHOptions_TRACE_OUTPUT_INST            0x40000

#define VSC_OPTN_PHOptions_GetBeforeShader(option)      ((option)->before_shader)
#define VSC_OPTN_PHOptions_SetBeforeShader(option, b)   ((option)->before_shader = (b))
#define VSC_OPTN_PHOptions_GetAfterShader(option)       ((option)->after_shader)
#define VSC_OPTN_PHOptions_SetAfterShader(option, a)    ((option)->after_shader = (a))
#define VSC_OPTN_PHOptions_GetBeforeFunc(option)        ((option)->before_func)
#define VSC_OPTN_PHOptions_SetBeforeFunc(option, b)     ((option)->before_func = (b))
#define VSC_OPTN_PHOptions_GetAfterFunc(option)         ((option)->after_func)
#define VSC_OPTN_PHOptions_SetAfterFunc(option, a)      ((option)->after_func = (a))
#define VSC_OPTN_PHOptions_GetBeforeBB(option)          ((option)->before_bb)
#define VSC_OPTN_PHOptions_SetBeforeBB(option, b)       ((option)->before_bb = (b))
#define VSC_OPTN_PHOptions_GetAfterBB(option)           ((option)->after_bb)
#define VSC_OPTN_PHOptions_SetAfterBB(option, a)        ((option)->after_bb = (a))
#define VSC_OPTN_PHOptions_GetBeforeInst(option)        ((option)->before_inst)
#define VSC_OPTN_PHOptions_SetBeforeInst(option, b)     ((option)->before_inst = (b))
#define VSC_OPTN_PHOptions_GetAfterInst(option)         ((option)->after_inst)
#define VSC_OPTN_PHOptions_SetAfterInst(option, a)      ((option)->after_inst = (a))

void VSC_OPTN_PHOptions_SetDefault(
    IN OUT VSC_OPTN_PHOptions* options,
    IN gctUINT optLevel
    );

void VSC_OPTN_PHOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_PHOptions* options
    );

void VSC_OPTN_PHOptions_Dump(
    IN VSC_OPTN_PHOptions* options_is,
    IN struct _VIR_DUMPER* dumper
    );

void VSC_OPTN_PHOptions_Usage(
    IN struct _VIR_DUMPER* dumper
    );

/* IS options */
typedef struct _VSC_OPTN_ISOPTIONS
{
    /* Must be first element */
    VSC_OPTN_BASE optnBase;

    gctBOOL is_forward;
    gctBOOL lli_only;
    gctBOOL bandwidth_only;
    gctBOOL vx_on;
    gctBOOL re_issue;
    gctUINT32 reg_count;                /* 0 is invalid here */
    gctUINT32 texld_cycles;             /* 0 is invalid here */
    gctUINT32 memld_cycles;             /* 0 is invalid here */
    gctUINT32 memst_cycles;             /* 0 is invalid here */
    gctUINT32 cacheld_cycles;           /* 0 is invalid here */
    gctUINT32 cachest_cycles;           /* 0 is invalid here */
    gctUINT32 bb_ceiling;
    gctUINT32 algorithm;
    gctUINT32 fw_heuristics;
    gctUINT32 bw_heuristics;
    gctUINT32 before_shader;
    gctUINT32 after_shader;
    gctUINT32 before_func;
    gctUINT32 after_func;
    gctUINT32 before_bb;
    gctUINT32 after_bb;
    gctUINT32 before_inst;
    gctUINT32 after_inst;
} VSC_OPTN_ISOptions;

#define VSC_OPTN_ISOptions_GetSwitchOn(option)              VSC_OPTN_GetSwitchOn(&(option)->optnBase)
#define VSC_OPTN_ISOptions_SetSwitchOn(option, s)           VSC_OPTN_SetSwitchOn(&(option)->optnBase, (s))
#define VSC_OPTN_ISOptions_GetPassId(option)                VSC_OPTN_GetPassId(&(option)->optnBase)
#define VSC_OPTN_ISOptions_SetPassId(option, p)             VSC_OPTN_SetPassId(&(option)->optnBase, (p))
#define VSC_OPTN_ISOptions_GetIsForward(option)             ((option)->is_forward)
#define VSC_OPTN_ISOptions_SetIsForward(option, i)          ((option)->is_forward = (i))
#define VSC_OPTN_ISOptions_GetLLIOnly(option)               ((option)->lli_only)
#define VSC_OPTN_ISOptions_SetLLIOnly(option, o)            ((option)->lli_only = (o))
#define VSC_OPTN_ISOptions_GetBandwidthOnly(option)         ((option)->bandwidth_only)
#define VSC_OPTN_ISOptions_SetBandwidthOnly(option, o)      ((option)->bandwidth_only = (o))
#define VSC_OPTN_ISOptions_GetVXOn(option)                  ((option)->vx_on)
#define VSC_OPTN_ISOptions_SetVXOn(option, v)               ((option)->vx_on = (v))
#define VSC_OPTN_ISOptions_GetReissue(option)               ((option)->re_issue)
#define VSC_OPTN_ISOptions_SetReissue(option, r)            ((option)->re_issue = (r))
#define VSC_OPTN_ISOptions_GetRegCount(option)              ((option)->reg_count)
#define VSC_OPTN_ISOptions_SetRegCount(option, r)           ((option)->reg_count = (r))
#define VSC_OPTN_ISOptions_GetTexldCycles(option)           ((option)->texld_cycles)
#define VSC_OPTN_ISOptions_SetTexldCycles(option, t)        ((option)->texld_cycles = (t))
#define VSC_OPTN_ISOptions_GetMemldCycles(option)           ((option)->memld_cycles)
#define VSC_OPTN_ISOptions_SetMemldCycles(option, m)        ((option)->memld_cycles = (m))
#define VSC_OPTN_ISOptions_GetMemstCycles(option)           ((option)->memst_cycles)
#define VSC_OPTN_ISOptions_SetMemstCycles(option, m)        ((option)->memst_cycles = (m))
#define VSC_OPTN_ISOptions_GetCacheldCycles(option)         ((option)->cacheld_cycles)
#define VSC_OPTN_ISOptions_SetCacheldCycles(option, c)      ((option)->cacheld_cycles = (c))
#define VSC_OPTN_ISOptions_GetCachestCycles(option)         ((option)->cachest_cycles)
#define VSC_OPTN_ISOptions_SetCachestCycles(option, c)      ((option)->cachest_cycles = (c))
#define VSC_OPTN_ISOptions_GetBBCeiling(option)             ((option)->bb_ceiling)
#define VSC_OPTN_ISOptions_SetBBCeiling(option, b)          ((option)->bb_ceiling = (b))
#define VSC_OPTN_ISOptions_GetAlgorithm(option)             ((option)->algorithm)
#define VSC_OPTN_ISOptions_SetAlgorithm(option, d)          ((option)->algorithm = (d))
#define VSC_OPTN_ISOptions_GetFwHeuristics(option)          ((option)->fw_heuristics)
#define VSC_OPTN_ISOptions_SetFwHeuristics(option, f)       ((option)->fw_heuristics = (f))
#define VSC_OPTN_ISOptions_GetBwHeuristics(option)          ((option)->bw_heuristics)
#define VSC_OPTN_ISOptions_SetBwHeuristics(option, b)       ((option)->bw_heuristics = (b))

#define VSC_OPTN_ISOptions_ALGORITHM_LISTSCHEDULING         0
#define VSC_OPTN_ISOptions_ALGORITHM_BUBBLESCHEDULING       1

#define VSC_OPTN_ISOptions_FW_HEUR_PREFER_RANGE                 0x1
#define VSC_OPTN_ISOptions_FW_HEUR_PRE_PREFER_BINDING           0x2
#define VSC_OPTN_ISOptions_FW_HEUR_PRE_PREFER_KILLDEP           0x4
#define VSC_OPTN_ISOptions_FW_HEUR_PREFER_KILL                  0x8
#define VSC_OPTN_ISOptions_FW_HEUR_PRE_PREFER_TEXLD             0x10
#define VSC_OPTN_ISOptions_FW_HEUR_PRE_PREFER_MEMLD             0x20
#define VSC_OPTN_ISOptions_FW_HEUR_PREFER_ANTI_BUBBLE           0x40
#define VSC_OPTN_ISOptions_FW_HEUR_POST_PREFER_TEXLD            0x80
#define VSC_OPTN_ISOptions_FW_HEUR_POST_PREFER_MEMLD            0x100
#define VSC_OPTN_ISOptions_FW_HEUR_DELAY_TEX_LD                 0x200
#define VSC_OPTN_ISOptions_FW_HEUR_DELAY_MEM_LD                 0x400
#define VSC_OPTN_ISOptions_FW_HEUR_POST_PREFER_TEXLDMEMLD       0x800
#define VSC_OPTN_ISOptions_FW_HEUR_POST_PREFER_KILLDEP          0x1000
#define VSC_OPTN_ISOptions_FW_HEUR_PREFER_ORDER                 0x2000

#define VSC_OPTN_ISOptions_FW_HEUR_PREFER_RANGE_ID              0
#define VSC_OPTN_ISOptions_FW_HEUR_PRE_PREFER_BINDING_ID        1
#define VSC_OPTN_ISOptions_FW_HEUR_PRE_PREFER_KILLDEP_ID        2
#define VSC_OPTN_ISOptions_FW_HEUR_PREFER_KILL_ID               3
#define VSC_OPTN_ISOptions_FW_HEUR_PRE_PREFER_TEXLD_ID          4
#define VSC_OPTN_ISOptions_FW_HEUR_PRE_PREFER_MEMLD_ID          5
#define VSC_OPTN_ISOptions_FW_HEUR_PREFER_ANTI_BUBBLE_ID        6
#define VSC_OPTN_ISOptions_FW_HEUR_POST_PREFER_TEXLD_ID         7
#define VSC_OPTN_ISOptions_FW_HEUR_POST_PREFER_MEMLD_ID         8
#define VSC_OPTN_ISOptions_FW_HEUR_DELAY_TEX_LD_ID              9
#define VSC_OPTN_ISOptions_FW_HEUR_DELAY_MEM_LD_ID              10
#define VSC_OPTN_ISOptions_FW_HEUR_POST_PREFER_TEXLDMEMLD_ID    11
#define VSC_OPTN_ISOptions_FW_HEUR_POST_PREFER_KILLDEP_ID       12
#define VSC_OPTN_ISOptions_FW_HEUR_PREFER_ORDER_ID              13
#define VSC_OPTN_ISOptions_FW_HEUR_LAST                         14

#define VSC_OPTN_ISOptions_BW_HEUR_PREFER_RANGE                 0x1
#define VSC_OPTN_ISOptions_BW_HEUR_PREFER_ORDER                 0x2

#define VSC_OPTN_ISOptions_BW_HEUR_PREFER_RANGE_ID              0
#define VSC_OPTN_ISOptions_BW_HEUR_PREFER_ORDER_ID              1
#define VSC_OPTN_ISOptions_BW_HEUR_LAST                         2

#define VSC_OPTN_ISOptions_GetTrace(option)                 VSC_OPTN_GetTrace(&(option)->optnBase)
#define VSC_OPTN_ISOptions_SetTrace(option, t)              VSC_OPTN_SetTrace(&(option)->optnBase, (t))

#define VSC_OPTN_ISOptions_TRACE_INITIALIZATION             0x1
#define VSC_OPTN_ISOptions_TRACE_INPUT_CFG                  0x2
#define VSC_OPTN_ISOptions_TRACE_INPUT_BB                   0x4
#define VSC_OPTN_ISOptions_TRACE_DAG                        0x8
#define VSC_OPTN_ISOptions_TRACE_HEURISTIC                  0x10
#define VSC_OPTN_ISOptions_TRACE_BUBBLESCHED                0x20
#define VSC_OPTN_ISOptions_TRACE_BUBBLESCHED_INCLUDE_RECUR  0x40
#define VSC_OPTN_ISOptions_TRACE_OUTPUT_BB                  0x80
#define VSC_OPTN_ISOptions_TRACE_BB_BUBBLE_SUM              0x100
#define VSC_OPTN_ISOptions_TRACE_OUTPUT_CFG                 0x200

#define VSC_OPTN_ISOptions_GetBeforeShader(option)      ((option)->before_shader)
#define VSC_OPTN_ISOptions_SetBeforeShader(option, b)   ((option)->before_shader = (b))
#define VSC_OPTN_ISOptions_GetAfterShader(option)       ((option)->after_shader)
#define VSC_OPTN_ISOptions_SetAfterShader(option, a)    ((option)->after_shader = (a))
#define VSC_OPTN_ISOptions_GetBeforeFunc(option)        ((option)->before_func)
#define VSC_OPTN_ISOptions_SetBeforeFunc(option, b)     ((option)->before_func = (b))
#define VSC_OPTN_ISOptions_GetAfterFunc(option)         ((option)->after_func)
#define VSC_OPTN_ISOptions_SetAfterFunc(option, a)      ((option)->after_func = (a))
#define VSC_OPTN_ISOptions_GetBeforeBB(option)          ((option)->before_bb)
#define VSC_OPTN_ISOptions_SetBeforeBB(option, b)       ((option)->before_bb = (b))
#define VSC_OPTN_ISOptions_GetAfterBB(option)           ((option)->after_bb)
#define VSC_OPTN_ISOptions_SetAfterBB(option, a)        ((option)->after_bb = (a))
#define VSC_OPTN_ISOptions_GetBeforeInst(option)        ((option)->before_inst)
#define VSC_OPTN_ISOptions_SetBeforeInst(option, b)     ((option)->before_inst = (b))
#define VSC_OPTN_ISOptions_GetAfterInst(option)         ((option)->after_inst)
#define VSC_OPTN_ISOptions_SetAfterInst(option, a)      ((option)->after_inst = (a))

void VSC_OPTN_ISOptions_SetDefault(
    IN OUT VSC_OPTN_ISOptions* options,
    IN gctUINT optLevel
    );

void VSC_OPTN_ISOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_ISOptions* options
    );

void VSC_OPTN_ISOptions_Dump(
    IN VSC_OPTN_ISOptions* options_is,
    IN struct _VIR_DUMPER* dumper
    );

void VSC_OPTN_ISOptions_Usage(
    IN struct _VIR_DUMPER* dumper
    );

/* RA options */
typedef struct _VSC_OPTN_RAOPTIONS
{
    /* Must be first element */
    VSC_OPTN_BASE optnBase;

    gctUINT32 heuristics;
    gctUINT32 opts;
    gctUINT32 registerCount;        /* set maxReg */
    gctUINT32 regWaterMark;         /* set register Water Mark */
    gctUINT32 stBubbleSize;         /* set the bubble size for st instructions */
    gctUINT32 spillBeforeShader;
    gctUINT32 spillAfterShader;

} VSC_OPTN_RAOptions;

#define VSC_OPTN_RAOptions_GetSwitchOn(option)          VSC_OPTN_GetSwitchOn(&(option)->optnBase)
#define VSC_OPTN_RAOptions_SetSwitchOn(option, s)       VSC_OPTN_SetSwitchOn(&(option)->optnBase, (s))
#define VSC_OPTN_RAOptions_GetHeuristics(option)        ((option)->heuristics)
#define VSC_OPTN_RAOptions_SetHeuristics(option, h)     ((option)->heuristics = (h))

#define VSC_OPTN_RAOptions_SPILL_FIRST                  0x1

#define VSC_OPTN_RAOptions_GetOPTS(option)              ((option)->opts)
#define VSC_OPTN_RAOptions_SetOPTS(option, s)           ((option)->opts = (s))

#define VSC_OPTN_RAOptions_ALLOC_REG                    0x1
#define VSC_OPTN_RAOptions_ALLOC_UNIFORM                0x2
#define VSC_OPTN_RAOptions_MAX_LS_EXTENED_END_POINT     0x4
#define VSC_OPTN_RAOptions_SPILL_DEST_OPT               0x8

#define VSC_OPTN_RAOptions_GetTrace(option)             VSC_OPTN_GetTrace(&(option)->optnBase)
#define VSC_OPTN_RAOptions_SetTrace(option, t)          VSC_OPTN_SetTrace(&(option)->optnBase, (t))

#define VSC_OPTN_RAOptions_TRACE_BUILD_LR               0x1
#define VSC_OPTN_RAOptions_TRACE_SORT_LR                0x2
#define VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR           0x4
#define VSC_OPTN_RAOptions_TRACE_FINAL                  0x8
#define VSC_OPTN_RAOptions_TRACE_INPUT                  0x10
#define VSC_OPTN_RAOptions_TRACE_WATERMARK              0x20
#define VSC_OPTN_RAOptions_TRACE    VSC_OPTN_RAOptions_TRACE_BUILD_LR | \
                                    VSC_OPTN_RAOptions_TRACE_SORT_LR | \
                                    VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR | \
                                    VSC_OPTN_RAOptions_TRACE_FINAL

#define VSC_OPTN_RAOptions_SetRegisterCount(option, s)  ((option)->registerCount = (s))
#define VSC_OPTN_RAOptions_GetRegisterCount(option)     ((option)->registerCount)

#define VSC_OPTN_RAOptions_SetRegWaterMark(option, s)  ((option)->regWaterMark = (s))
#define VSC_OPTN_RAOptions_GetRegWaterMark(option)     ((option)->regWaterMark)

#define VSC_OPTN_RAOptions_SetSTBubbleSize(option, s)  ((option)->stBubbleSize = (s))
#define VSC_OPTN_RAOptions_GetSTBubbleSize(option)     ((option)->stBubbleSize)

#define VSC_OPTN_RAOptions_GetSpillBeforeShader(option)      ((option)->spillBeforeShader)
#define VSC_OPTN_RAOptions_SetSpillBeforeShader(option, b)   ((option)->spillBeforeShader = (b))
#define VSC_OPTN_RAOptions_GetSpillAfterShader(option)       ((option)->spillAfterShader)
#define VSC_OPTN_RAOptions_SetSpillAfterShader(option, a)    ((option)->spillAfterShader = (a))

void VSC_OPTN_RAOptions_SetDefault(
    IN OUT VSC_OPTN_RAOptions* options,
    IN gctUINT optLevel
    );

void VSC_OPTN_RAOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_RAOptions* options
    );

void VSC_OPTN_RAOptions_Dump(
    IN VSC_OPTN_RAOptions* options,
    IN struct _VIR_DUMPER* dumper
    );

void VSC_OPTN_RAOptions_Usage(
    IN struct _VIR_DUMPER* dumper
    );

/* CPP options */
typedef struct _VSC_OPTN_CPPOPTIONS
{
    /* Must be first element */
    VSC_OPTN_BASE optnBase;

    gctUINT32   opts;
} VSC_OPTN_CPPOptions;

#define VSC_OPTN_CPPOptions_GetSwitchOn(option)             VSC_OPTN_GetSwitchOn(&(option)->optnBase)
#define VSC_OPTN_CPPOptions_SetSwitchOn(option, s)          VSC_OPTN_SetSwitchOn(&(option)->optnBase, (s))
#define VSC_OPTN_CPPOptions_GetPassId(option)               VSC_OPTN_GetPassId(&(option)->optnBase)
#define VSC_OPTN_CPPOptions_SetPassId(option, p)            VSC_OPTN_SetPassId(&(option)->optnBase, (p))
#define VSC_OPTN_CPPOptions_GetOPTS(option)                 ((option)->opts)
#define VSC_OPTN_CPPOptions_SetOPTS(option, s)              ((option)->opts = (s))
#define VSC_OPTN_CPPOptions_GetTrace(option)                VSC_OPTN_GetTrace(&(option)->optnBase)
#define VSC_OPTN_CPPOptions_SetTrace(option, t)             VSC_OPTN_SetTrace(&(option)->optnBase, (t))

#define VSC_OPTN_CPPOptions_FORWARD_OPT                     0x1
#define VSC_OPTN_CPPOptions_BACKWARD_OPT                    0x2

#define VSC_OPTN_CPPOptions_TRACE_INPUT                     0x1
#define VSC_OPTN_CPPOptions_TRACE_OUTPUT                    0x2
#define VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT               0x4
#define VSC_OPTN_CPPOptions_TRACE_BACKWARD_OPT              0x8

void VSC_OPTN_CPPOptions_SetDefault(
    IN OUT VSC_OPTN_CPPOptions* options,
    IN gctUINT optLevel
    );

void VSC_OPTN_CPPOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_CPPOptions* options
    );

void VSC_OPTN_CPPOptions_Dump(
    IN VSC_OPTN_CPPOptions* options,
    IN struct _VIR_DUMPER* dumper
    );

void VSC_OPTN_CPPOptions_Usage(
    IN struct _VIR_DUMPER* dumper
    );

/* Constant propagation and folding options */
typedef struct _VSC_OPTN_CPFOPTIONS
{
    /* Must be first element */
    VSC_OPTN_BASE optnBase;

    gctUINT32   before_shader;
    gctUINT32   after_shader;
    gctUINT32   before_func;
    gctUINT32   after_func;
} VSC_OPTN_CPFOptions;

#define VSC_OPTN_CPFOptions_GetSwitchOn(option)             VSC_OPTN_GetSwitchOn(&(option)->optnBase)
#define VSC_OPTN_CPFOptions_SetSwitchOn(option, s)          VSC_OPTN_SetSwitchOn(&(option)->optnBase, (s))
#define VSC_OPTN_CPFOptions_GetBeforeShader(option)         ((option)->before_shader)
#define VSC_OPTN_CPFOptions_SetBeforeShader(option, b)      ((option)->before_shader = (b))
#define VSC_OPTN_CPFOptions_GetAfterShader(option)          ((option)->after_shader)
#define VSC_OPTN_CPFOptions_SetAfterShader(option, a)       ((option)->after_shader = (a))
#define VSC_OPTN_CPFOptions_GetBeforeFunc(option)           ((option)->before_func)
#define VSC_OPTN_CPFOptions_SetBeforeFunc(option, b)        ((option)->before_func = (b))
#define VSC_OPTN_CPFOptions_GetAfterFunc(option)            ((option)->after_func)
#define VSC_OPTN_CPFOptions_SetAfterFunc(option, a)         ((option)->after_func = (a))
#define VSC_OPTN_CPFOptions_GetTrace(option)                VSC_OPTN_GetTrace(&(option)->optnBase)
#define VSC_OPTN_CPFOptions_SetTrace(option, t)             VSC_OPTN_SetTrace(&(option)->optnBase, (t))

#define VSC_OPTN_CPFOptions_TRACE_INPUT                     0x1
#define VSC_OPTN_CPFOptions_TRACE_OUTPUT                    0x2
#define VSC_OPTN_CPFOptions_TRACE_ALGORITHM                 0x4

void VSC_OPTN_CPFOptions_SetDefault(
    IN OUT VSC_OPTN_CPFOptions* options,
    IN gctUINT optLevel
    );

void VSC_OPTN_CPFOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_CPFOptions* options
    );

void VSC_OPTN_CPFOptions_Dump(
    IN VSC_OPTN_CPFOptions* options,
    IN struct _VIR_DUMPER* dumper
    );

void VSC_OPTN_CPFOptions_Usage(
    IN struct _VIR_DUMPER* dumper
    );

/* Vectorization options */
typedef struct _VSC_OPTN_VECOPTIONS
{
    /* Must be first element */
    VSC_OPTN_BASE optnBase;

    gctUINT32   before_shader;
    gctUINT32   after_shader;
} VSC_OPTN_VECOptions;

#define VSC_OPTN_VECOptions_GetSwitchOn(option)             VSC_OPTN_GetSwitchOn(&(option)->optnBase)
#define VSC_OPTN_VECOptions_SetSwitchOn(option, s)          VSC_OPTN_SetSwitchOn(&(option)->optnBase, (s))
#define VSC_OPTN_VECOptions_GetBeforeShader(option)         ((option)->before_shader)
#define VSC_OPTN_VECOptions_SetBeforeShader(option, b)      ((option)->before_shader = (b))
#define VSC_OPTN_VECOptions_GetAfterShader(option)          ((option)->after_shader)
#define VSC_OPTN_VECOptions_SetAfterShader(option, a)       ((option)->after_shader = (a))

void VSC_OPTN_VECOptions_SetDefault(
    IN OUT VSC_OPTN_VECOptions* options,
    IN gctUINT optLevel
    );

void VSC_OPTN_VECOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_VECOptions* options
    );

/* Common Subexpression Elimination options */
typedef struct _VSC_OPTN_LCSEOPTIONS
{
    /* Must be first element */
    VSC_OPTN_BASE optnBase;

    gctUINT32   opts;
    gctUINT32   before_shader;
    gctUINT32   after_shader;
    gctUINT32   before_func;
    gctUINT32   after_func;
} VSC_OPTN_LCSEOptions;

#define VSC_OPTN_LCSEOptions_GetSwitchOn(option)            VSC_OPTN_GetSwitchOn(&(option)->optnBase)
#define VSC_OPTN_LCSEOptions_SetSwitchOn(option, s)         VSC_OPTN_SetSwitchOn(&(option)->optnBase, (s))
#define VSC_OPTN_LCSEOptions_GetOpts(option)                ((option)->opts)
#define VSC_OPTN_LCSEOptions_SetOpts(option, h)             ((option)->opts = (h))

#define VSC_OPTN_LCSEOptions_OPT_LOAD                       0x1
#define VSC_OPTN_LCSEOptions_OPT_ATTR_LD                    0x2
#define VSC_OPTN_LCSEOptions_OPT_OTHERS                     0x4

#define VSC_OPTN_LCSEOptions_GetBeforeShader(option)        ((option)->before_shader)
#define VSC_OPTN_LCSEOptions_SetBeforeShader(option, b)     ((option)->before_shader = (b))
#define VSC_OPTN_LCSEOptions_GetAfterShader(option)         ((option)->after_shader)
#define VSC_OPTN_LCSEOptions_SetAfterShader(option, a)      ((option)->after_shader = (a))
#define VSC_OPTN_LCSEOptions_GetBeforeFunc(option)          ((option)->before_func)
#define VSC_OPTN_LCSEOptions_SetBeforeFunc(option, b)       ((option)->before_func = (b))
#define VSC_OPTN_LCSEOptions_GetAfterFunc(option)           ((option)->after_func)
#define VSC_OPTN_LCSEOptions_SetAfterFunc(option, a)        ((option)->after_func = (a))
#define VSC_OPTN_LCSEOptions_GetTrace(option)               VSC_OPTN_GetTrace(&(option)->optnBase)
#define VSC_OPTN_LCSEOptions_SetTrace(option, t)            VSC_OPTN_SetTrace(&(option)->optnBase, (t))

#define VSC_OPTN_LCSEOptions_TRACE_INPUT_CFG                0x1
#define VSC_OPTN_LCSEOptions_TRACE_OUTPUT_CFG               0x2
#define VSC_OPTN_LCSEOptions_TRACE_INPUT_BB                 0x4
#define VSC_OPTN_LCSEOptions_TRACE_OUTPUT_BB                0x8
#define VSC_OPTN_LCSEOptions_TRACE_GENERATING               0x10
#define VSC_OPTN_LCSEOptions_TRACE_INSERTING                0x20
#define VSC_OPTN_LCSEOptions_TRACE_REPLACING                0x40
#define VSC_OPTN_LCSEOptions_TRACE_KILLING                  0x80

void VSC_OPTN_LCSEOptions_SetDefault(
    IN OUT VSC_OPTN_LCSEOptions* options,
    IN gctUINT optLevel
    );

void VSC_OPTN_LCSEOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_LCSEOptions* options
    );

void VSC_OPTN_LCSEOptions_Dump(
    IN VSC_OPTN_LCSEOptions* options,
    IN struct _VIR_DUMPER* dumper
    );

void VSC_OPTN_LCSEOptions_Usage(
    IN struct _VIR_DUMPER* dumper
    );

/* DCE options */
typedef struct _VSC_OPTN_DCEOPTIONS
{
    /* Must be first element */
    VSC_OPTN_BASE optnBase;

    gctUINT32 opts;
} VSC_OPTN_DCEOptions;

#define VSC_OPTN_DCEOptions_GetSwitchOn(option)             VSC_OPTN_GetSwitchOn(&(option)->optnBase)
#define VSC_OPTN_DCEOptions_SetSwitchOn(option, s)          VSC_OPTN_SetSwitchOn(&(option)->optnBase, (s))
#define VSC_OPTN_DCEOptions_GetPassId(option)               VSC_OPTN_GetPassId(&(option)->optnBase)
#define VSC_OPTN_DCEOptions_SetPassId(option, p)            VSC_OPTN_SetPassId(&(option)->optnBase, (p))
#define VSC_OPTN_DCEOptions_GetOPTS(option)                 ((option)->opts)
#define VSC_OPTN_DCEOptions_SetOPTS(option, s)              ((option)->opts = (s))

#define VSC_OPTN_DCEOptions_OPTS_CONTROL                    (0x01)

#define VSC_OPTN_DCEOptions_GetTrace(option)                VSC_OPTN_GetTrace(&(option)->optnBase)
#define VSC_OPTN_DCEOptions_SetTrace(option, t)             VSC_OPTN_SetTrace(&(option)->optnBase, (t))

#define VSC_OPTN_DCEOptions_TRACE_INPUT                     0x1
#define VSC_OPTN_DCEOptions_TRACE_WORKLIST                  0x2
#define VSC_OPTN_DCEOptions_TRACE_OUTPUT                    0x4
#define VSC_OPTN_DCEOptions_TRACE_ANALYSIS                  0x8
#define VSC_OPTN_DCEOptions_TRACE_REMOVED                   0x10
#define VSC_OPTN_DCEOptions_TRACE_ADDED                     0x20
#define VSC_OPTN_DCEOptions_TRACE_CONTROLFLOW               0x40

void VSC_OPTN_DCEOptions_SetDefault(
    IN OUT VSC_OPTN_DCEOptions* options,
    IN gctUINT optLevel
    );

void VSC_OPTN_DCEOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_DCEOptions* options
    );

void VSC_OPTN_DCEOptions_Dump(
    IN VSC_OPTN_DCEOptions* options,
    IN struct _VIR_DUMPER* dumper
    );

void VSC_OPTN_DCEOptions_Usage(
    IN struct _VIR_DUMPER* dumper
    );

/* IO-packing options */
typedef struct _VSC_OPTN_IOPOPTIONS
{
    /* Must be first element */
    VSC_OPTN_BASE optnBase;

    gctUINT32 opts;
} VSC_OPTN_IOPOptions;

#define VSC_OPTN_IOPOptions_GetSwitchOn(option)             VSC_OPTN_GetSwitchOn(&(option)->optnBase)
#define VSC_OPTN_IOPOptions_SetSwitchOn(option, s)          VSC_OPTN_SetSwitchOn(&(option)->optnBase, (s))
#define VSC_OPTN_IOPOptions_GetOPTS(option)                 ((option)->opts)
#define VSC_OPTN_IOPOptions_SetOPTS(option, s)              ((option)->opts = (s))

#define VSC_OPTN_IOPOptions_GetTrace(option)                VSC_OPTN_GetTrace(&(option)->optnBase)
#define VSC_OPTN_IOPOptions_SetTrace(option, t)             VSC_OPTN_SetTrace(&(option)->optnBase, (t))

void VSC_OPTN_IOPOptions_SetDefault(
    IN OUT VSC_OPTN_IOPOptions* options,
    IN gctUINT optLevel
    );

void VSC_OPTN_IOPOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_IOPOptions* options
    );

void VSC_OPTN_IOPOptions_Dump(
    IN VSC_OPTN_IOPOptions* options,
    IN struct _VIR_DUMPER* dumper
    );

void VSC_OPTN_IOPOptions_Usage(
    IN struct _VIR_DUMPER* dumper
    );

/* full active IO options */
typedef struct _VSC_OPTN_FAIOOPTIONS
{
    /* Must be first element */
    VSC_OPTN_BASE optnBase;

    gctUINT32 opts;
} VSC_OPTN_FAIOOptions;

#define VSC_OPTN_FAIOOptions_GetSwitchOn(option)             VSC_OPTN_GetSwitchOn(&(option)->optnBase)
#define VSC_OPTN_FAIOOptions_SetSwitchOn(option, s)          VSC_OPTN_SetSwitchOn(&(option)->optnBase, (s))
#define VSC_OPTN_FAIOOptions_GetOPTS(option)                 ((option)->opts)
#define VSC_OPTN_FAIOOptions_SetOPTS(option, s)              ((option)->opts = (s))

#define VSC_OPTN_FAIOOptions_GetTrace(option)                VSC_OPTN_GetTrace(&(option)->optnBase)
#define VSC_OPTN_FAIOOptions_SetTrace(option, t)             VSC_OPTN_SetTrace(&(option)->optnBase, (t))

void VSC_OPTN_FAIOOptions_SetDefault(
    IN OUT VSC_OPTN_FAIOOptions* options,
    IN gctUINT optLevel
    );

void VSC_OPTN_FAIOOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_FAIOOptions* options
    );

void VSC_OPTN_FAIOOptions_Dump(
    IN VSC_OPTN_FAIOOptions* options,
    IN struct _VIR_DUMPER* dumper
    );

void VSC_OPTN_FAIOOptions_Usage(
    IN struct _VIR_DUMPER* dumper
    );

/* Dual16 options */
typedef struct _VSC_OPTN_DUAL16OPTIONS
{
    /* Must be first element */
    VSC_OPTN_BASE optnBase;

    gctFLOAT percentage;
    gctFLOAT halfDepPercentage;

} VSC_OPTN_DUAL16Options;

#define VSC_OPTN_DUAL16Options_GetPercentage(option)        ((option)->percentage)
#define VSC_OPTN_DUAL16Options_SetPercentage(option, p)     ((option)->percentage = (p))

#define VSC_OPTN_DUAL16Options_GetHalfDepPercentage(option)        ((option)->halfDepPercentage)
#define VSC_OPTN_DUAL16Options_SetHalfDepPercentage(option, p)     ((option)->halfDepPercentage = (p))

#define VSC_OPTN_DUAL16Options_GetTrace(option)             VSC_OPTN_GetTrace(&(option)->optnBase)
#define VSC_OPTN_DUAL16Options_SetTrace(option, t)          VSC_OPTN_SetTrace(&(option)->optnBase, (t))
#define VSC_OPTN_DUAL16Options_GetSwitchOn(option)          VSC_OPTN_GetSwitchOn(&(option)->optnBase)
#define VSC_OPTN_DUAL16Options_SetSwitchOn(option, s)       VSC_OPTN_SetSwitchOn(&(option)->optnBase, (s))

#define VSC_OPTN_DUAL16Options_TRACE_INPUT                  0x1
#define VSC_OPTN_DUAL16Options_TRACE_OUTPUT                 0x2
#define VSC_OPTN_DUAL16Options_TRACE_DETAIL                 0x4

void VSC_OPTN_DUAL16Options_SetDefault(
    IN OUT VSC_OPTN_DUAL16Options* options,
    IN gctUINT optLevel
    );

void VSC_OPTN_DUAL16Options_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_DUAL16Options* options
    );

void VSC_OPTN_DUAL16Options_Dump(
    IN VSC_OPTN_DUAL16Options* options,
    IN struct _VIR_DUMPER* dumper
    );

void VSC_OPTN_DUAL16Options_Usage(
    IN struct _VIR_DUMPER* dumper
    );

/* Final Cleanup Phase options */
typedef struct _VSC_OPTN_FCPOPTIONS
{
    /* Must be first element */
    VSC_OPTN_BASE optnBase;

    gctUINT32 opts;
} VSC_OPTN_FCPOptions;

#define VSC_OPTN_FCPOptions_GetSwitchOn(option)          VSC_OPTN_GetSwitchOn(&(option)->optnBase)
#define VSC_OPTN_FCPOptions_SetSwitchOn(option, s)       VSC_OPTN_SetSwitchOn(&(option)->optnBase, (s))

#define VSC_OPTN_FCPOptions_GetOPTS(option)              ((option)->opts)
#define VSC_OPTN_FCPOptions_SetOPTS(option, s)           ((option)->opts = (s))

#define VSC_OPTN_FCPOptions_REPL_LDARR                   0x1
#define VSC_OPTN_FCPOptions_SPLIT_DUAL32                 0x2
#define VSC_OPTN_FCPOptions_OPTS_ICAST                   0x4

#define VSC_OPTN_FCPOptions_GetTrace(option)             VSC_OPTN_GetTrace(&(option)->optnBase)
#define VSC_OPTN_FCPOptions_SetTrace(option, t)          VSC_OPTN_SetTrace(&(option)->optnBase, (t))

#define VSC_OPTN_FCPOptions_TRACE_INPUT                  0x1
#define VSC_OPTN_FCPOptions_TRACE_OUTPUT                 0x2
#define VSC_OPTN_FCPOptions_TRACE_ICAST                  0x4

void VSC_OPTN_FCPOptions_SetDefault(
    IN OUT VSC_OPTN_FCPOptions* options,
    IN gctUINT optLevel
    );

void VSC_OPTN_FCPOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_FCPOptions* options
    );

void VSC_OPTN_FCPOptions_Dump(
    IN VSC_OPTN_FCPOptions* options,
    IN struct _VIR_DUMPER* dumper
    );

void VSC_OPTN_FCPOptions_Usage(
    IN struct _VIR_DUMPER* dumper
    );

/* MC Gen options */
typedef struct _VSC_OPTN_MCGENOPTIONS
{
    /* Must be first element */
    VSC_OPTN_BASE optnBase;

    gctUINT32 opts;
} VSC_OPTN_MCGenOptions;

#define VSC_OPTN_MCGenOptions_GetSwitchOn(option)             VSC_OPTN_GetSwitchOn(&(option)->optnBase)
#define VSC_OPTN_MCGenOptions_SetSwitchOn(option, s)          VSC_OPTN_SetSwitchOn(&(option)->optnBase, (s))
#define VSC_OPTN_MCGenOptions_GetOPTS(option)                 ((option)->opts)
#define VSC_OPTN_MCGenOptions_SetOPTS(option, s)              ((option)->opts = (s))

#define VSC_OPTN_MCGenOptions_GetTrace(option)                VSC_OPTN_GetTrace(&(option)->optnBase)
#define VSC_OPTN_MCGenOptions_SetTrace(option, t)             VSC_OPTN_SetTrace(&(option)->optnBase, (t))

#define VSC_OPTN_MCGenOptions_TRACE_OUTPUT                    (0x1)

void VSC_OPTN_MCGenOptions_SetDefault(
    IN OUT VSC_OPTN_MCGenOptions* options,
    IN gctUINT optLevel
    );

void VSC_OPTN_MCGenOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_MCGenOptions* options
    );

void VSC_OPTN_MCGenOptions_Dump(
    IN VSC_OPTN_MCGenOptions* options,
    IN struct _VIR_DUMPER* dumper
    );

void VSC_OPTN_MCGenOptions_Usage(
    IN struct _VIR_DUMPER* dumper
    );

/* SEP Gen options */
typedef struct _VSC_OPTN_SEPGENOPTIONS
{
    /* Must be first element */
    VSC_OPTN_BASE optnBase;

} VSC_OPTN_SEPGenOptions;

#define VSC_OPTN_SEPGenOptions_GetSwitchOn(option)             VSC_OPTN_GetSwitchOn(&(option)->optnBase)
#define VSC_OPTN_SEPGenOptions_SetSwitchOn(option, s)          VSC_OPTN_SetSwitchOn(&(option)->optnBase, (s))
#define VSC_OPTN_SEPGenOptions_GetTrace(option)                VSC_OPTN_GetTrace(&(option)->optnBase)
#define VSC_OPTN_SEPGenOptions_SetTrace(option, t)             VSC_OPTN_SetTrace(&(option)->optnBase, (t))

void VSC_OPTN_SEPGenOptions_SetDefault(
    IN OUT VSC_OPTN_SEPGenOptions* options,
    IN gctUINT optLevel
    );

/* Dump options */
typedef struct _VSC_OPTN_DUMPOPTIONS
{
    /* Must be first element */
    VSC_OPTN_BASE optnBase;

    gctINT dumpStart;
    gctINT dumpEnd;
    gctUINT32 opts;
} VSC_OPTN_DumpOptions;

#define VSC_OPTN_DumpOptions_GetSwitchOn(option)            VSC_OPTN_GetSwitchOn(&(option)->optnBase)
#define VSC_OPTN_DumpOptions_SetSwitchOn(option, s)         VSC_OPTN_SetSwitchOn(&(option)->optnBase, (s))

#define VSC_OPTN_DumpOptions_GetDumpStart(option)           ((option)->dumpStart)
#define VSC_OPTN_DumpOptions_SetDumpStart(option, start)    ((option)->dumpStart = (start))
#define VSC_OPTN_DumpOptions_GetDumpEnd(option)             ((option)->dumpEnd)
#define VSC_OPTN_DumpOptions_SetDumpEnd(option, end)        ((option)->dumpEnd = (end))
#define VSC_OPTN_DumpOptions_GetOPTS(option)                ((option)->opts)
#define VSC_OPTN_DumpOptions_SetOPTS(option, o)             ((option)->opts = (o))
#define VSC_OPTN_DumpOptions_DUMP_NONE                      0x0
#define VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE               0x1
#define VSC_OPTN_DumpOptions_DUMP_OPTION                    0x2
#define VSC_OPTN_DumpOptions_DUMP_CG                        0x4
#define VSC_OPTN_DumpOptions_DUMP_FINALIR                   0x8

void VSC_OPTN_DumpOptions_SetDefault(
    IN OUT VSC_OPTN_DumpOptions* options
    );

void VSC_OPTN_DumpOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_DumpOptions* options
    );

void VSC_OPTN_DumpOptions_Dump(
    IN VSC_OPTN_DumpOptions* options,
    IN struct _VIR_DUMPER* dumper
    );

gctBOOL VSC_OPTN_DumpOptions_CheckDumpFlag(
    IN VSC_OPTN_DumpOptions* options,
    IN gctINT ShaderId,
    IN gctUINT DumpFlag
    );

/* internal lib functions link */
typedef struct _VSC_OPTN_ILFLINKOPTIONS
{
    /* Must be first element */
    VSC_OPTN_BASE optnBase;
} VSC_OPTN_ILFLinkOptions;

#define VSC_OPTN_ILFLinkOptions_GetSwitchOn(option)         VSC_OPTN_GetSwitchOn(&(option)->optnBase)
#define VSC_OPTN_ILFLinkOptions_SetSwitchOn(option, s)      VSC_OPTN_SetSwitchOn(&(option)->optnBase, (s))

void VSC_OPTN_ILFLinkOptions_SetDefault(
    IN OUT VSC_OPTN_ILFLinkOptions* options
    );

/* option to control atomic patch */
typedef struct _VSC_OPTN_ATOMICPATCHOPTIONS
{
    /* Must be first element */
    VSC_OPTN_BASE optnBase;
} VSC_OPTN_ATOMPatchOptions;

#define VSC_OPTN_ATOMPatchOptions_GetSwitchOn(option)         VSC_OPTN_GetSwitchOn(&(option)->optnBase)
#define VSC_OPTN_ATOMPatchOptions_SetSwitchOn(option, s)      VSC_OPTN_SetSwitchOn(&(option)->optnBase, (s))

void VSC_OPTN_ATOMPatchOptions_SetDefault(
    IN OUT VSC_OPTN_ATOMPatchOptions* options
    );

/* Unified uniform allocation options */
typedef struct _VSC_OPTN_UNIFIEDUNIFORMOPTIONS
{
    /* Must be first element */
    VSC_OPTN_BASE optnBase;
} VSC_OPTN_UnifiedUniformOptions;

#define VSC_OPTN_UnifiedUniformOptions_GetSwitchOn(option)            VSC_OPTN_GetSwitchOn(&(option)->optnBase)
#define VSC_OPTN_UnifiedUniformOptions_SetSwitchOn(option, s)         VSC_OPTN_SetSwitchOn(&(option)->optnBase, (s))

void VSC_OPTN_UnifiedUniformOptions_SetDefault(
    IN OUT VSC_OPTN_UnifiedUniformOptions* options
    );

void VSC_OPTN_UnifiedUniformOptions_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_UnifiedUniformOptions* options
    );

void VSC_OPTN_UnifiedUniformOptions_Dump(
    IN VSC_OPTN_UnifiedUniformOptions* options,
    IN struct _VIR_DUMPER* dumper
    );

#define VSC_OPTN_SCPP_COUNT 1
#define VSC_OPTN_PARAMOPT_COUNT 1
#define VSC_OPTN_LoopOpts_COUNT 1
#define VSC_OPTN_CFO_COUNT 2
#define VSC_OPTN_UF_AUBO_COUNT 1
#define VSC_OPTN_IL_COUNT 1
#define VSC_OPTN_PU_COUNT 1
#define VSC_OPTN_LowerM2L_COUNT 1
#define VSC_OPTN_SCL_COUNT 1
#define VSC_OPTN_PH_COUNT 1
#define VSC_OPTN_SIMP_COUNT 1
#define VSC_OPTN_IS_COUNT 2
#define VSC_OPTN_RA_COUNT 1
#define VSC_OPTN_CPP_COUNT 2
#define VSC_OPTN_CPF_COUNT 1
#define VSC_OPTN_VEC_COUNT 1
#define VSC_OPTN_LCSE_COUNT 1
#define VSC_OPTN_DCE_COUNT 2
#define VSC_OPTN_IOP_COUNT 1
#define VSC_OPTN_FAIO_COUNT 1
#define VSC_OPTN_DUAL16_COUNT 1
#define VSC_OPTN_FCP_COUNT 1
#define VSC_OPTN_MCGen_COUNT 1
#define VSC_OPTN_SEPGen_COUNT 1


/* When add a new option, we need to add the corresponding type for VSC_PASS_OPTN_TYPE and VSC_OPTN_Options_GetOption. */
typedef struct _VSC_OPTN_OPTIONS
{
    gctBOOL                     initialized;
    gctBOOL                     reset;
    VSC_OPTN_SCPPOptions        scpp_options[VSC_OPTN_SCPP_COUNT];
    VSC_OPTN_ParamOptOptions    paopt_options[VSC_OPTN_PARAMOPT_COUNT];
    VSC_OPTN_LoopOptsOptions    loopopts_options[VSC_OPTN_LoopOpts_COUNT];
    VSC_OPTN_CFOOptions         cfo_options[VSC_OPTN_CFO_COUNT];
    VSC_OPTN_UF_AUBOOptions     aubo_options[VSC_OPTN_UF_AUBO_COUNT];
    VSC_OPTN_ILOptions          inliner_options[VSC_OPTN_IL_COUNT];
    VSC_OPTN_PUOptions          pu_options[VSC_OPTN_PU_COUNT];
    VSC_OPTN_LowerM2LOptions    lowerM2L_options[VSC_OPTN_LowerM2L_COUNT];
    VSC_OPTN_SCLOptions         scalarization_options[VSC_OPTN_SCL_COUNT];
    VSC_OPTN_PHOptions          ph_options[VSC_OPTN_PH_COUNT];
    VSC_OPTN_SIMPOptions        simp_options[VSC_OPTN_SIMP_COUNT];
    VSC_OPTN_ISOptions          is_options[VSC_OPTN_IS_COUNT];
    VSC_OPTN_RAOptions          ra_options[VSC_OPTN_RA_COUNT];
    VSC_OPTN_CPPOptions         cpp_options[VSC_OPTN_CPP_COUNT];
    VSC_OPTN_CPFOptions         cpf_options[VSC_OPTN_CPF_COUNT];
    VSC_OPTN_VECOptions         vec_options[VSC_OPTN_VEC_COUNT];
    VSC_OPTN_LCSEOptions        cse_options[VSC_OPTN_LCSE_COUNT];
    VSC_OPTN_DCEOptions         dce_options[VSC_OPTN_DCE_COUNT];
    VSC_OPTN_IOPOptions         iopacking_options[VSC_OPTN_IOP_COUNT];
    VSC_OPTN_FAIOOptions        fullActiveIO_options[VSC_OPTN_FAIO_COUNT];
    VSC_OPTN_DUAL16Options      dual16_options[VSC_OPTN_DUAL16_COUNT];
    VSC_OPTN_FCPOptions         fcp_options[VSC_OPTN_FCP_COUNT];
    VSC_OPTN_MCGenOptions       mcgen_options[VSC_OPTN_MCGen_COUNT];
    VSC_OPTN_SEPGenOptions      sepgen_options[VSC_OPTN_SEPGen_COUNT];
    VSC_OPTN_DumpOptions        dump_options;
    VSC_OPTN_ILFLinkOptions     ilflink_options;
    VSC_OPTN_UnifiedUniformOptions unifiedUniform_options;
    VSC_OPTN_ATOMPatchOptions   atompatch_options;
    gctBOOL                     options_usage;
    gctUINT                     cFlags;
} VSC_OPTN_Options;

typedef enum _VSC_PASS_OPTN_TYPE
{
    VSC_PASS_OPTN_TYPE_NONE   = 0,
    VSC_PASS_OPTN_TYPE_SCPP,
    VSC_PASS_OPTN_TYPE_PAOPT,
    VSC_PASS_OPTN_TYPE_LOOPOPTS,
    VSC_PASS_OPTN_TYPE_CFO,
    VSC_PASS_OPTN_TYPE_AUBO,
    VSC_PASS_OPTN_TYPE_INLINER,
    VSC_PASS_OPTN_TYPE_PU,
    VSC_PASS_OPTN_TYPE_M2LLOWER,
    VSC_PASS_OPTN_TYPE_SCALARIZE,
    VSC_PASS_OPTN_TYPE_PH,
    VSC_PASS_OPTN_TYPE_SIMP,
    VSC_PASS_OPTN_TYPE_IS,
    VSC_PASS_OPTN_TYPE_RA,
    VSC_PASS_OPTN_TYPE_CPP,
    VSC_PASS_OPTN_TYPE_CPF,
    VSC_PASS_OPTN_TYPE_VEC,
    VSC_PASS_OPTN_TYPE_LCSE,
    VSC_PASS_OPTN_TYPE_DCE,
    VSC_PASS_OPTN_TYPE_IOP,
    VSC_PASS_OPTN_TYPE_FAIO,
    VSC_PASS_OPTN_TYPE_DUAL16,
    VSC_PASS_OPTN_TYPE_FCP,
    VSC_PASS_OPTN_TYPE_MC_GEN,
    VSC_PASS_OPTN_TYPE_SEP_GEN,
    VSC_PASS_OPTN_TYPE_DUMP,
    VSC_PASS_OPTN_TYPE_ILF_LINK,
    VSC_PASS_OPTN_TYPE_UNIFIED_UNIFORM,
    VSC_PASS_OPTN_TYPE_ATOM_PATCH,
    VSC_PASS_OPTN_TYPE_MAX,
} VSC_PASS_OPTN_TYPE;

#define VSC_OPTN_Options_GetSCPPOptions(option, i)         (&((option)->scpp_options[i]))
#define VSC_OPTN_Options_GetPARAMOPTOptions(option, i)     (&((option)->paopt_options[i]))
#define VSC_OPTN_Options_GetLoopOptsOptions(option, i)     (&((option)->loopopts_options[i]))
#define VSC_OPTN_Options_GetCFOOptions(option, i)          (&((option)->cfo_options[i]))
#define VSC_OPTN_Options_GetAUBOOptions(option, i)         (&((option)->aubo_options[i]))
#define VSC_OPTN_Options_GetInlinerOptions(option, i)      (&((option)->inliner_options[i]))
#define VSC_OPTN_Options_GetPUOptions(option, i)           (&((option)->pu_options[i]))
#define VSC_OPTN_Options_GetLowerM2LOptions(option, i)     (&((option)->lowerM2L_options[i]))
#define VSC_OPTN_Options_GetSCLOptions(option, i)          (&((option)->scalarization_options[i]))
#define VSC_OPTN_Options_GetPHOptions(option, i)           (&((option)->ph_options[i]))
#define VSC_OPTN_Options_GetSIMPOptions(option, i)         (&((option)->simp_options[i]))
#define VSC_OPTN_Options_GetISOptions(option, i)           (&((option)->is_options[i]))
#define VSC_OPTN_Options_GetRAOptions(option, i)           (&((option)->ra_options[i]))
#define VSC_OPTN_Options_GetCPPOptions(option, i)          (&((option)->cpp_options[i]))
#define VSC_OPTN_Options_GetCPFOptions(option, i)          (&((option)->cpf_options[i]))
#define VSC_OPTN_Options_GetVECOptions(option, i)          (&((option)->vec_options[i]))
#define VSC_OPTN_Options_GetLCSEOptions(option, i)         (&((option)->cse_options[i]))
#define VSC_OPTN_Options_GetDCEOptions(option, i)          (&((option)->dce_options[i]))
#define VSC_OPTN_Options_GetVECTORIZEOptions(option, i)    (&((option)->vectorize_options[i]))
#define VSC_OPTN_Options_GetIOPOptions(option, i)          (&((option)->iopacking_options[i]))
#define VSC_OPTN_Options_GetFAIOOptions(option, i)         (&((option)->fullActiveIO_options[i]))
#define VSC_OPTN_Options_GetMCGenOptions(option, i)        (&((option)->mcgen_options[i]))
#define VSC_OPTN_Options_GetSEPGenOptions(option, i)       (&((option)->sepgen_options[i]))
#define VSC_OPTN_Options_GetDUAL16Options(option, i)       (&((option)->dual16_options[i]))
#define VSC_OPTN_Options_GetFCPOptions(option, i)          (&((option)->fcp_options[i]))
#define VSC_OPTN_Options_GetDumpOptions(option)            (&((option)->dump_options))
#define VSC_OPTN_Options_GetILFLinkOptions(option)         (&((option)->ilflink_options))
#define VSC_OPTN_Options_GetUnifiedUniformOptions(option)  (&((option)->unifiedUniform_options))
#define VSC_OPTN_Options_GetATOMPatchOptions(option)       (&((option)->atompatch_options))
#define VSC_OPTN_Options_GetOptionsUsage(option)           ((option)->options_usage)
#define VSC_OPTN_Options_SetOptionsUsage(option, u)        ((option)->options_usage = (u))
#define VSC_OPTN_Options_SetReset(option, u)               ((option)->reset = (u))

VSC_OPTN_BASE* VSC_OPTN_Options_GetOption(
    IN OUT VSC_OPTN_Options* pOptions,
    IN  VSC_PASS_OPTN_TYPE optnType,
    IN  gctUINT passId
    );

void VSC_OPTN_Options_SetDefault(
    IN OUT VSC_OPTN_Options* options,
    IN gctUINT optLevel
    );

void VSC_OPTN_Options_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_Options* options
    );

void VSC_OPTN_Options_Dump(
    IN VSC_OPTN_Options* options,
    IN struct _VIR_DUMPER* dumper
    );

void VSC_OPTN_Options_Usage(
    IN struct _VIR_DUMPER* dumper
    );

void VSC_OPTN_Options_SetOptionsByCompileFlags(
    IN OUT VSC_OPTN_Options* Options,
    IN  gctUINT CompileFlags
    );

void VSC_OPTN_Options_SetOptionsByOptFlags(
    IN OUT VSC_OPTN_Options* Options,
    IN  gctUINT64 OptFlags
    );

void VSC_OPTN_Options_SetSpecialOptions(
    IN OUT VSC_OPTN_Options* options,
    IN VSC_HW_CONFIG* pHwCfg
    );

gctBOOL VSC_OPTN_Options_GetOptLevelFromEnv(
    OUT gctUINT* optLevel
    );

void VSC_OPTN_Options_GetOptionFromEnv(
    IN OUT VSC_OPTN_Options* options
    );

void VSC_OPTN_Options_MergeVCEnvOption(
    IN OUT VSC_OPTN_Options* options
    );

gctBOOL VSC_OPTN_InRange(
    IN gctUINT32 id,
    IN gctUINT32 before,
    IN gctUINT32 after
    );
/*
   Only call stack requirement of HW arch must make it disabled.
*/
#define SUPPORT_IPA_DFA        1

END_EXTERN_C()

#endif /* __gc_vsc_options_h_ */

