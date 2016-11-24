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


#ifndef __gc_vsc_options_h_
#define __gc_vsc_options_h_

BEGIN_EXTERN_C()

struct _VIR_DUMPER;

typedef struct _VSC_OPTN_BASE
{
    gctBOOL   switch_on;
    gctTRACE  trace;
} VSC_OPTN_BASE;

#define VSC_OPTN_GetSwitchOn(baseOptn)                    ((baseOptn)->switch_on)
#define VSC_OPTN_SetSwitchOn(baseOptn, s)                 ((baseOptn)->switch_on = (s))
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
#define VSC_OPTN_SCPPOptions_GetTrace(option)                   VSC_OPTN_GetTrace(&(option)->optnBase)
#define VSC_OPTN_SCPPOptions_SetTrace(option, t)                VSC_OPTN_SetTrace(&(option)->optnBase, (t))

#define VSC_OPTN_SCPPOptions_TRACE_INPUT_SHADER                 0x1
#define VSC_OPTN_SCPPOptions_TRACE_INPUT_FUNC                   0x2
#define VSC_OPTN_SCPPOptions_TRACE_INPUT_BB                     0x4
#define VSC_OPTN_SCPPOptions_TRACE_DETAIL                       0x8
#define VSC_OPTN_SCPPOptions_TRACE_OUTPUT_BB                    0x10
#define VSC_OPTN_SCPPOptions_TRACE_OUTPUT_FUNC                  0x20
#define VSC_OPTN_SCPPOptions_TRACE_OUTPUT_SHADER                0x40

void VSC_OPTN_SCPPOptions_SetDefault(
    IN OUT VSC_OPTN_SCPPOptions* options
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

/* Loop Optimizations Options */
typedef struct _VSC_OPTN_LOOPOPTSOPTIONS
{
    /* Must be first element */
    VSC_OPTN_BASE optnBase;

    gctUINT32 opts;
    gctUINT32 before_shader;
    gctUINT32 after_shader;
} VSC_OPTN_LoopOptsOptions;

#define VSC_OPTN_LoopOptsOptions_GetSwitchOn(option)            VSC_OPTN_GetSwitchOn(&(option)->optnBase)
#define VSC_OPTN_LoopOptsOptions_SetSwitchOn(option, s)         VSC_OPTN_SetSwitchOn(&(option)->optnBase, (s))

#define VSC_OPTN_LoopOptsOptions_OPTS_NONE                      0x0
#define VSC_OPTN_LoopOptsOptions_OPTS_LOOP_INVERSION            0x1
#define VSC_OPTN_LoopOptsOptions_OPTS_LOOP_UNROLLING            0x2

#define VSC_OPTN_LoopOptsOptions_GetOpts(option)                ((option)->opts)
#define VSC_OPTN_LoopOptsOptions_SetOpts(option, o)             ((option)->opts = (o))
#define VSC_OPTN_LoopOptsOptions_GetTrace(option)               VSC_OPTN_GetTrace(&(option)->optnBase)
#define VSC_OPTN_LoopOptsOptions_SetTrace(option, t)            VSC_OPTN_SetTrace(&(option)->optnBase, (t))

#define VSC_OPTN_LoopOptsOptions_TRACE_SHADER_INPUT             0x1
#define VSC_OPTN_LoopOptsOptions_TRACE_FUNC_INPUT               0x2
#define VSC_OPTN_LoopOptsOptions_TRACE_INV_FUNC_INPUT           0x4
#define VSC_OPTN_LoopOptsOptions_TRACE_INV                      0x8
#define VSC_OPTN_LoopOptsOptions_TRACE_INV_FUNC_OUTPUT          0x10
#define VSC_OPTN_LoopOptsOptions_TRACE_UNROLL_FUNC_INPUT        0x20
#define VSC_OPTN_LoopOptsOptions_TRACE_UNROLL                   0x40
#define VSC_OPTN_LoopOptsOptions_TRACE_UNROLL_FUNC_OUTPUT       0x80
#define VSC_OPTN_LoopOptsOptions_TRACE_FUNC_OUTPUT              0x100
#define VSC_OPTN_LoopOptsOptions_TRACE_SHADER_OUTPUT            0x200

#define VSC_OPTN_LoopOptsOptions_GetBeforeShader(option)      ((option)->before_shader)
#define VSC_OPTN_LoopOptsOptions_SetBeforeShader(option, b)   ((option)->before_shader = (b))
#define VSC_OPTN_LoopOptsOptions_GetAfterShader(option)       ((option)->after_shader)
#define VSC_OPTN_LoopOptsOptions_SetAfterShader(option, a)    ((option)->after_shader = (a))

void VSC_OPTN_LoopOptsOptions_SetDefault(
    IN OUT VSC_OPTN_LoopOptsOptions* options
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
} VSC_OPTN_UF_AUBO_Options;

#define VSC_OPTN_UF_AUBO_Options_GetSwitchOn(option)                    VSC_OPTN_GetSwitchOn(&(option)->optnBase)
#define VSC_OPTN_UF_AUBO_Options_SetSwitchOn(option, s)                 VSC_OPTN_SetSwitchOn(&(option)->optnBase, (s))

#define VSC_OPTN_UF_AUBO_Options_HEUR_FORCE_ALL               0x1
#define VSC_OPTN_UF_AUBO_Options_HEUR_INDIRECT_IMM            0x2
#define VSC_OPTN_UF_AUBO_Options_HEUR_INDIRECT_UD             0x4
#define VSC_OPTN_UF_AUBO_Options_HEUR_INDIRECT_D              0x6
#define VSC_OPTN_UF_AUBO_Options_HEUR_INDIRECT_MASK           0x6
#define VSC_OPTN_UF_AUBO_Options_HEUR_ORDERLY                 0x8
#define VSC_OPTN_UF_AUBO_Options_HEUR_SKIP_DUBO               0x10
#define VSC_OPTN_UF_AUBO_Options_HEUR_SKIP_CUBO               0x20

#define VSC_OPTN_UF_AUBO_Options_GetHeuristics(option)                  ((option)->heuristics)
#define VSC_OPTN_UF_AUBO_Options_SetHeuristics(option, h)               ((option)->heuristics = (h))
#define VSC_OPTN_UF_AUBO_Options_GetIndirectAccessLevel(option)         (((option)->heuristics & VSC_OPTN_UF_AUBO_Options_HEUR_INDIRECT_MASK) >> 1)
#define VSC_OPTN_UF_AUBO_Options_SetIndirectAccessLevel(option, ial)    ((option)->heuristics |= ((ial) << 1))
#define VSC_OPTN_UF_AUBO_Options_GetConstRegReservation(option)         ((option)->const_reg_reservation)
#define VSC_OPTN_UF_AUBO_Options_SetConstRegReservation(option, c)      ((option)->const_reg_reservation = (c))
#define VSC_OPTN_UF_AUBO_Options_GetOpt(option)                         ((option)->opt)
#define VSC_OPTN_UF_AUBO_Options_SetOpt(option, o)                      ((option)->opt = (o))

#define VSC_OPTN_UF_AUBO_Options_GetTrace(option)             VSC_OPTN_GetTrace(&(option)->optnBase)
#define VSC_OPTN_UF_AUBO_Options_SetTrace(option, t)          VSC_OPTN_SetTrace(&(option)->optnBase, (t))

#define VSC_OPTN_UF_AUBO_Options_TRACE_INPUT                  0x1
#define VSC_OPTN_UF_AUBO_Options_TRACE_INITIALIZE             0x2
#define VSC_OPTN_UF_AUBO_Options_TRACE_MARKACTIVE             0x4
#define VSC_OPTN_UF_AUBO_Options_TRACE_INDIRECT               0x8
#define VSC_OPTN_UF_AUBO_Options_TRACE_DUBSIZE                0x10
#define VSC_OPTN_UF_AUBO_Options_TRACE_PICK                   0x20
#define VSC_OPTN_UF_AUBO_Options_TRACE_CONSTRUCTION           0x40
#define VSC_OPTN_UF_AUBO_Options_TRACE_FILL                   0x80
#define VSC_OPTN_UF_AUBO_Options_TRACE_TRANSFORM              0x100
#define VSC_OPTN_UF_AUBO_Options_TRACE_TRANSFORM_BUILD        0x200
#define VSC_OPTN_UF_AUBO_Options_TRACE_TRANSFORM_REORG        0x400
#define VSC_OPTN_UF_AUBO_Options_TRACE_TRANSFORM_INSERT       0x800
#define VSC_OPTN_UF_AUBO_Options_TRACE_OUTPUT                 0x1000

#define VSC_OPTN_UF_AUBO_Options_GetBeforeShader(option)      ((option)->before_shader)
#define VSC_OPTN_UF_AUBO_Options_SetBeforeShader(option, b)   ((option)->before_shader = (b))
#define VSC_OPTN_UF_AUBO_Options_GetAfterShader(option)       ((option)->after_shader)
#define VSC_OPTN_UF_AUBO_Options_SetAfterShader(option, a)    ((option)->after_shader = (a))

void VSC_OPTN_UF_AUBO_Options_SetDefault(
    IN OUT VSC_OPTN_UF_AUBO_Options* options
    );

void VSC_OPTN_UF_AUBO_Options_GetOptionFromString(
    IN gctSTRING str,
    IN OUT VSC_OPTN_UF_AUBO_Options* options
    );

void VSC_OPTN_UF_AUBO_Options_Dump(
    IN VSC_OPTN_UF_AUBO_Options* options,
    IN struct _VIR_DUMPER* dumper
    );

void VSC_OPTN_UF_AUBO_Options_Usage(
    IN struct _VIR_DUMPER* dumper
    );

/* Inliner options */
typedef struct _VSC_OPTN_ILOPTIONS
{
    /* Must be first element */
    VSC_OPTN_BASE optnBase;

    gctUINT32 heuristics;
} VSC_OPTN_ILOptions;

#define VSC_OPTN_ILOptions_GetSwitchOn(option)          VSC_OPTN_GetSwitchOn(&(option)->optnBase)
#define VSC_OPTN_ILOptions_SetSwitchOn(option, s)       VSC_OPTN_SetSwitchOn(&(option)->optnBase, (s))

#define VSC_OPTN_ILOptions_GetHeuristics(option)        ((option)->heuristics)
#define VSC_OPTN_ILOptions_SetHeuristics(option, h)     ((option)->heuristics = (h))

#define VSC_OPTN_ILOptions_CALL_DEPTH                    0x1
#define VSC_OPTN_ILOptions_TOP_DOWN                      0x2

#define VSC_OPTN_ILOptions_GetTrace(option)             VSC_OPTN_GetTrace(&(option)->optnBase)
#define VSC_OPTN_ILOptions_SetTrace(option, t)          VSC_OPTN_SetTrace(&(option)->optnBase, (t))

#define VSC_OPTN_ILOptions_TRACE                        0x1

void VSC_OPTN_ILOptions_SetDefault(
    IN OUT VSC_OPTN_ILOptions* options
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
    IN OUT VSC_OPTN_PUOptions* options
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
    IN OUT VSC_OPTN_SIMPOptions* options
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
    IN OUT VSC_OPTN_LowerM2LOptions* options
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
    IN OUT VSC_OPTN_SCLOptions* options
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

#define VSC_OPTN_PHOptions_GetModifiers(option)         ((option)->modifiers)
#define VSC_OPTN_PHOptions_SetModifiers(option, m)      ((option)->modifiers = (m))
#define VSC_OPTN_PHOptions_MODIFIERS_SAT                0x1
#define VSC_OPTN_PHOptions_MODIFIERS_NEG                0x2
#define VSC_OPTN_PHOptions_MODIFIERS_ABS                0x4

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
    IN OUT VSC_OPTN_PHOptions* options
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
    gctUINT32 reg_count;                /* 0 is invalid here */
    gctUINT32 texld_cycles;             /* 0 is invalid here */
    gctUINT32 memld_cycles;             /* 0 is invalid here */
    gctUINT32 memst_cycles;             /* 0 is invalid here */
    gctUINT32 cacheld_cycles;           /* 0 is invalid here */
    gctUINT32 cachest_cycles;           /* 0 is invalid here */
    gctUINT32 dep_gran;
    gctUINT32 bb_ceiling;
    gctUINT32 algorithm;
    gctUINT32 fw_heuristics;
    gctUINT32 bw_heuristics;
    gctBOOL preRA;
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
#define VSC_OPTN_ISOptions_GetIsForward(option)             ((option)->is_forward)
#define VSC_OPTN_ISOptions_SetIsForward(option, i)          ((option)->is_forward = (i))
#define VSC_OPTN_ISOptions_GetLLIOnly(option)               ((option)->lli_only)
#define VSC_OPTN_ISOptions_SetLLIOnly(option, o)            ((option)->lli_only = (o))
#define VSC_OPTN_ISOptions_GetBandwidthOnly(option)         ((option)->bandwidth_only)
#define VSC_OPTN_ISOptions_SetBandwidthOnly(option, o)      ((option)->bandwidth_only = (o))
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
#define VSC_OPTN_ISOptions_GetDepGran(option)               ((option)->dep_gran)
#define VSC_OPTN_ISOptions_SetDepGran(option, d)            ((option)->dep_gran = (d))
#define VSC_OPTN_ISOptions_GetBBCeiling(option)             ((option)->bb_ceiling)
#define VSC_OPTN_ISOptions_SetBBCeiling(option, b)          ((option)->bb_ceiling = (b))
#define VSC_OPTN_ISOptions_GetAlgorithm(option)             ((option)->algorithm)
#define VSC_OPTN_ISOptions_SetAlgorithm(option, d)          ((option)->algorithm = (d))
#define VSC_OPTN_ISOptions_GetFwHeuristics(option)          ((option)->fw_heuristics)
#define VSC_OPTN_ISOptions_SetFwHeuristics(option, f)       ((option)->fw_heuristics = (f))
#define VSC_OPTN_ISOptions_GetBwHeuristics(option)          ((option)->bw_heuristics)
#define VSC_OPTN_ISOptions_SetBwHeuristics(option, b)       ((option)->bw_heuristics = (b))

#define VSC_OPTN_ISOptions_DEPGRAN_GROSS                    0
#define VSC_OPTN_ISOptions_DEPGRAN_DU_PER_CHANNAL           1

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

#define VSC_OPTN_ISOptions_GetPreRA(option)                 ((option)->preRA)
#define VSC_OPTN_ISOptions_SetPreRA(option, p)              ((option)->preRA = (p))

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
    IN OUT VSC_OPTN_ISOptions* options
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

#define VSC_OPTN_RAOptions_ALLOC_REG                      0x1
#define VSC_OPTN_RAOptions_ALLOC_UNIFORM                  0x2

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
    IN OUT VSC_OPTN_RAOptions* options
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
    IN OUT VSC_OPTN_CPPOptions* options
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
    IN OUT VSC_OPTN_CPFOptions* options
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
    IN OUT VSC_OPTN_VECOptions* options
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
    IN OUT VSC_OPTN_LCSEOptions* options
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
    IN OUT VSC_OPTN_DCEOptions* options
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
    IN OUT VSC_OPTN_IOPOptions* options
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
    IN OUT VSC_OPTN_FAIOOptions* options
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
    IN OUT VSC_OPTN_DUAL16Options* options
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
    IN OUT VSC_OPTN_FCPOptions* options
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
    IN OUT VSC_OPTN_MCGenOptions* options
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
    IN OUT VSC_OPTN_SEPGenOptions* options
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

/* When add a new option, we need to add the corresponding type for VSC_PASS_OPTN_TYPE and VSC_OPTN_Options_GetOption. */
typedef struct _VSC_OPTN_OPTIONS
{
    gctBOOL                     initialized;
    gctBOOL                     reset;
    VSC_OPTN_SCPPOptions        scpp_options;
    VSC_OPTN_LoopOptsOptions    loopopts_options;
    VSC_OPTN_UF_AUBO_Options    aubo_options;
    VSC_OPTN_ILOptions          inliner_options;
    VSC_OPTN_PUOptions          pu_options;
    VSC_OPTN_LowerM2LOptions    lowerM2L_options;
    VSC_OPTN_SCLOptions         scalarization_options;
    VSC_OPTN_PHOptions          ph_options;
    VSC_OPTN_SIMPOptions        simp_options;
    VSC_OPTN_ISOptions          prera_is_options;
    VSC_OPTN_RAOptions          ra_options;
    VSC_OPTN_CPPOptions         cpp_options;
    VSC_OPTN_CPFOptions         cpf_options;
    VSC_OPTN_VECOptions         vec_options;
    VSC_OPTN_LCSEOptions        cse_options;
    VSC_OPTN_DCEOptions         dce_options;
    VSC_OPTN_IOPOptions         iopacking_options;
    VSC_OPTN_FAIOOptions        fullActiveIO_options;
    VSC_OPTN_ISOptions          postra_is_options;
    VSC_OPTN_DUAL16Options      dual16_options;
    VSC_OPTN_FCPOptions         fcp_options;
    VSC_OPTN_MCGenOptions       mcgen_options;
    VSC_OPTN_SEPGenOptions      sepgen_options;
    VSC_OPTN_DumpOptions        dump_options;
    gctBOOL                     options_usage;
} VSC_OPTN_Options;

typedef enum _VSC_PASS_OPTN_TYPE
{
    VSC_PASS_OPTN_TYPE_NONE   = 0,
    VSC_PASS_OPTN_TYPE_SCPP,
    VSC_PASS_OPTN_TYPE_LOOPOPTS,
    VSC_PASS_OPTN_TYPE_AUBO,
    VSC_PASS_OPTN_TYPE_INLINER,
    VSC_PASS_OPTN_TYPE_PU,
    VSC_PASS_OPTN_TYPE_M2LLOWER,
    VSC_PASS_OPTN_TYPE_SCALARIZE,
    VSC_PASS_OPTN_TYPE_PH,
    VSC_PASS_OPTN_TYPE_SIMP,
    VSC_PASS_OPTN_TYPE_PRE_RA_IS,
    VSC_PASS_OPTN_TYPE_RA,
    VSC_PASS_OPTN_TYPE_CPP,
    VSC_PASS_OPTN_TYPE_CPF,
    VSC_PASS_OPTN_TYPE_VEC,
    VSC_PASS_OPTN_TYPE_LCSE,
    VSC_PASS_OPTN_TYPE_DCE,
    VSC_PASS_OPTN_TYPE_IOP,
    VSC_PASS_OPTN_TYPE_FAIO,
    VSC_PASS_OPTN_TYPE_POST_RA_IS,
    VSC_PASS_OPTN_TYPE_DUAL16,
    VSC_PASS_OPTN_TYPE_FCP,
    VSC_PASS_OPTN_TYPE_MC_GEN,
    VSC_PASS_OPTN_TYPE_SEP_GEN,
    VSC_PASS_OPTN_TYPE_DUMP,
    VSC_PASS_OPTN_TYPE_MAX,
} VSC_PASS_OPTN_TYPE;

#define VSC_OPTN_Options_GetSCPPOptions(option)             (&((option)->scpp_options))
#define VSC_OPTN_Options_GetLoopOptsOptions(option)         (&((option)->loopopts_options))
#define VSC_OPTN_Options_GetAUBOOptions(option)             (&((option)->aubo_options))
#define VSC_OPTN_Options_GetInlinerOptions(option)          (&((option)->inliner_options))
#define VSC_OPTN_Options_GetPUOptions(option)               (&((option)->pu_options))
#define VSC_OPTN_Options_GetLowerM2LOptions(option)         (&((option)->lowerM2L_options))
#define VSC_OPTN_Options_GetSCLOptions(option)              (&((option)->scalarization_options))
#define VSC_OPTN_Options_GetPHOptions(option)               (&((option)->ph_options))
#define VSC_OPTN_Options_GetSIMPOptions(option)             (&((option)->simp_options))
#define VSC_OPTN_Options_GetPreRAISOptions(option)          (&((option)->prera_is_options))
#define VSC_OPTN_Options_GetRAOptions(option)               (&((option)->ra_options))
#define VSC_OPTN_Options_GetCPPOptions(option)              (&((option)->cpp_options))
#define VSC_OPTN_Options_GetCPFOptions(option)              (&((option)->cpf_options))
#define VSC_OPTN_Options_GetVECOptions(option)              (&((option)->vec_options))
#define VSC_OPTN_Options_GetLCSEOptions(option)             (&((option)->cse_options))
#define VSC_OPTN_Options_GetDCEOptions(option)              (&((option)->dce_options))
#define VSC_OPTN_Options_GetVECTORIZEOptions(option)        (&((option)->vectorize_options))
#define VSC_OPTN_Options_GetIOPOptions(option)              (&((option)->iopacking_options))
#define VSC_OPTN_Options_GetFAIOOptions(option)             (&((option)->fullActiveIO_options))
#define VSC_OPTN_Options_GetMCGenOptions(option)            (&((option)->mcgen_options))
#define VSC_OPTN_Options_GetSEPGenOptions(option)           (&((option)->sepgen_options))
#define VSC_OPTN_Options_GetPostRAISOptions(option)         (&((option)->postra_is_options))
#define VSC_OPTN_Options_GetDUAL16Options(option)           (&((option)->dual16_options))
#define VSC_OPTN_Options_GetFCPOptions(option)              (&((option)->fcp_options))
#define VSC_OPTN_Options_GetDumpOptions(option)             (&((option)->dump_options))
#define VSC_OPTN_Options_GetOptionsUsage(option)            ((option)->options_usage)
#define VSC_OPTN_Options_SetOptionsUsage(option, u)         ((option)->options_usage = (u))
#define VSC_OPTN_Options_SetReset(option, u)                ((option)->reset = (u))

VSC_OPTN_BASE* VSC_OPTN_Options_GetOption(
    IN OUT VSC_OPTN_Options* pOptions,
    IN  VSC_PASS_OPTN_TYPE optnType
    );

void VSC_OPTN_Options_SetDefault(
    IN OUT VSC_OPTN_Options* options
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

