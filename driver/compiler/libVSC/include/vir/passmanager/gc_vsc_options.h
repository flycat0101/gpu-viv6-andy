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

/* Uniform Rearrange Options */
typedef struct _VSC_OPTN_UF_AUBO_OPTIONS
{
    gctBOOL switch_on;
    gctUINT32 heuristics;
    gctUINT32 const_reg_reservation;
    gctTRACE trace;
    gctUINT32 before_shader;
    gctUINT32 after_shader;
} VSC_OPTN_UF_AUBO_Options;

#define VSC_OPTN_UF_AUBO_Options_GetSwitchOn(option)                    ((option)->switch_on)
#define VSC_OPTN_UF_AUBO_Options_SetSwitchOn(option, s)                 ((option)->switch_on = (s))
#define VSC_OPTN_UF_AUBO_Options_GetHeuristics(option)                  ((option)->heuristics)
#define VSC_OPTN_UF_AUBO_Options_SetHeuristics(option, h)               ((option)->heuristics = (h))
#define VSC_OPTN_UF_AUBO_Options_GetConstRegReservation(option)         ((option)->const_reg_reservation)
#define VSC_OPTN_UF_AUBO_Options_SetConstRegReservation(option, c)      ((option)->const_reg_reservation = (c))

#define VSC_OPTN_UF_AUBO_Options_HEUR_FORCE_ALL               0x1
#define VSC_OPTN_UF_AUBO_Options_HEUR_INDIRECT_MUST           0x2
#define VSC_OPTN_UF_AUBO_Options_HEUR_ORDERLY                 0x4
#define VSC_OPTN_UF_AUBO_Options_HEUR_USE_DUBO                0x8
#define VSC_OPTN_UF_AUBO_Options_HEUR_USE_CUBO                0x10

#define VSC_OPTN_UF_AUBO_Options_GetTrace(option)             ((option)->trace)
#define VSC_OPTN_UF_AUBO_Options_SetTrace(option, t)          ((option)->trace = (t))

#define VSC_OPTN_UF_AUBO_Options_TRACE_INPUT                  0x1
#define VSC_OPTN_UF_AUBO_Options_TRACE_INITIALIZE             0x2
#define VSC_OPTN_UF_AUBO_Options_TRACE_MARKACTIVE             0x4
#define VSC_OPTN_UF_AUBO_Options_TRACE_INDIRECT               0x8
#define VSC_OPTN_UF_AUBO_Options_TRACE_DUBSIZE                0x10
#define VSC_OPTN_UF_AUBO_Options_TRACE_PICK                   0x20
#define VSC_OPTN_UF_AUBO_Options_TRACE_CONSTRUCTION           0x40
#define VSC_OPTN_UF_AUBO_Options_TRACE_FILL                   0x80
#define VSC_OPTN_UF_AUBO_Options_TRACE_TRANSFORM              0x100
#define VSC_OPTN_UF_AUBO_Options_TRACE_OUTPUT                 0x200

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
    gctBOOL switch_on;
    gctUINT32 heuristics;
    gctTRACE trace;
} VSC_OPTN_ILOptions;

#define VSC_OPTN_ILOptions_GetSwitchOn(option)          ((option)->switch_on)
#define VSC_OPTN_ILOptions_SetSwitchOn(option, s)       ((option)->switch_on = (s))

#define VSC_OPTN_ILOptions_GetHeuristics(option)        ((option)->heuristics)
#define VSC_OPTN_ILOptions_SetHeuristics(option, h)     ((option)->heuristics = (h))

#define VSC_OPTN_ILOptions_CALL_DEPTH                    0x1
#define VSC_OPTN_ILOptions_TOP_DOWN                      0x2

#define VSC_OPTN_ILOptions_GetTrace(option)             ((option)->trace)
#define VSC_OPTN_ILOptions_SetTrace(option, t)          ((option)->trace = (t))

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
    gctBOOL switch_on;
    gctTRACE trace;
} VSC_OPTN_PUOptions;

#define VSC_OPTN_PUOptions_GetSwitchOn(option)          ((option)->switch_on)
#define VSC_OPTN_PUOptions_SetSwitchOn(option, s)       ((option)->switch_on = (s))
#define VSC_OPTN_PUOptions_GetTrace(option)             ((option)->trace)
#define VSC_OPTN_PUOptions_SetTrace(option, t)          ((option)->trace = (t))

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
    gctBOOL switch_on;
    gctUINT32 before_shader;
    gctUINT32 after_shader;
    gctUINT32 before_func;
    gctUINT32 after_func;
    gctUINT32 before_inst;
    gctUINT32 after_inst;
    gctTRACE trace;
} VSC_OPTN_SIMPOptions;

#define VSC_OPTN_SIMPOptions_GetSwitchOn(option)            ((option)->switch_on)
#define VSC_OPTN_SIMPOptions_SetSwitchOn(option, s)         ((option)->switch_on = (s))
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
#define VSC_OPTN_SIMPOptions_GetTrace(option)               ((option)->trace)
#define VSC_OPTN_SIMPOptions_SetTrace(option, t)            ((option)->trace = (t))

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
    gctBOOL     switch_on;
    gctTRACE    trace;
} VSC_OPTN_LowerM2LOptions;

#define VSC_OPTN_LowerM2LOptions_GetSwitchOn(option)        ((option)->switch_on)
#define VSC_OPTN_LowerM2LOptions_SetSwitchOn(option, s)     ((option)->switch_on = (s))
#define VSC_OPTN_LowerM2LOptions_GetTrace(option)           ((option)->trace)
#define VSC_OPTN_LowerM2LOptions_SetTrace(option, t)        ((option)->trace = (t))

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
    gctBOOL  switch_on;
    gctTRACE trace;
} VSC_OPTN_SCLOptions;

#define VSC_OPTN_SCLOptions_GetSwitchOn(option)         ((option)->switch_on)
#define VSC_OPTN_SCLOptions_SetSwitchOn(option, s)      ((option)->switch_on = (s))
#define VSC_OPTN_SCLOptions_GetTrace(option)            ((option)->trace)
#define VSC_OPTN_SCLOptions_SetTrace(option, t)         ((option)->trace = (t))
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
    gctBOOL  switch_on;
    gctUINT32 opts;
    gctUINT32 modifiers;
    gctTRACE trace;
    gctUINT32 before_shader;
    gctUINT32 after_shader;
    gctUINT32 before_func;
    gctUINT32 after_func;
    gctUINT32 before_bb;
    gctUINT32 after_bb;
    gctUINT32 before_inst;
    gctUINT32 after_inst;
} VSC_OPTN_PHOptions;

#define VSC_OPTN_PHOptions_GetSwitchOn(option)          ((option)->switch_on)
#define VSC_OPTN_PHOptions_SetSwitchOn(option, s)       ((option)->switch_on = (s))

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
#define VSC_OPTN_PHOptions_OPTS_ICAST                   0x80
#define VSC_OPTN_PHOptions_OPTS_LSHIFT_LS               0x100
#define VSC_OPTN_PHOptions_OPTS_DP                      0x200

#define VSC_OPTN_PHOptions_GetModifiers(option)         ((option)->modifiers)
#define VSC_OPTN_PHOptions_SetModifiers(option, m)      ((option)->modifiers = (m))
#define VSC_OPTN_PHOptions_MODIFIERS_SAT                0x1
#define VSC_OPTN_PHOptions_MODIFIERS_NEG                0x2
#define VSC_OPTN_PHOptions_MODIFIERS_ABS                0x4

#define VSC_OPTN_PHOptions_GetTrace(option)             ((option)->trace)
#define VSC_OPTN_PHOptions_SetTrace(option, t)          ((option)->trace = (t))
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
#define VSC_OPTN_PHOptions_TRACE_ICAST                  0x800
#define VSC_OPTN_PHOptions_TRACE_LSHIFT_LS              0x1000
#define VSC_OPTN_PHOptions_TRACE_DP                     0x2000
#define VSC_OPTN_PHOptions_TRACE_INPUT_INST             0x4000
#define VSC_OPTN_PHOptions_TRACE_BUILT_TREE             0x8000
#define VSC_OPTN_PHOptions_TRACE_REQUIREMENTS           0x10000
#define VSC_OPTN_PHOptions_TRACE_TRANSFORMS             0x20000
#define VSC_OPTN_PHOptions_TRACE_MERGING                0x40000
#define VSC_OPTN_PHOptions_TRACE_OUTPUT_INST            0x80000

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
    gctBOOL switch_on;
    gctBOOL is_forward;
    gctBOOL lli_only;
    gctBOOL bandwidth_only;
    gctUINT32 reg_count;                /* 0 is invalid here */
    gctUINT32 dep_gran;
    gctUINT32 bb_ceiling;
    gctUINT32 algorithm;
    gctUINT32 fw_heuristics;
    gctUINT32 bw_heuristics;
    gctTRACE trace;
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

#define VSC_OPTN_ISOptions_GetSwitchOn(option)              ((option)->switch_on)
#define VSC_OPTN_ISOptions_SetSwitchOn(option, s)           ((option)->switch_on = (s))
#define VSC_OPTN_ISOptions_GetIsForward(option)             ((option)->is_forward)
#define VSC_OPTN_ISOptions_SetIsForward(option, i)          ((option)->is_forward = (i))
#define VSC_OPTN_ISOptions_GetLLIOnly(option)               ((option)->lli_only)
#define VSC_OPTN_ISOptions_SetLLIOnly(option, o)            ((option)->lli_only = (o))
#define VSC_OPTN_ISOptions_GetBandwidthOnly(option)         ((option)->bandwidth_only)
#define VSC_OPTN_ISOptions_SetBandwidthOnly(option, o)      ((option)->bandwidth_only = (o))
#define VSC_OPTN_ISOptions_GetRegCount(option)              ((option)->reg_count)
#define VSC_OPTN_ISOptions_SetRegCount(option, r)           ((option)->reg_count = (r))
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

#define VSC_OPTN_ISOptions_GetTrace(option)                 ((option)->trace)
#define VSC_OPTN_ISOptions_SetTrace(option, t)              ((option)->trace = (t))

#define VSC_OPTN_ISOptions_TRACE_INITIALIZATION             0x1
#define VSC_OPTN_ISOptions_TRACE_INPUT_CFG                  0x2
#define VSC_OPTN_ISOptions_TRACE_INPUT_BB                   0x4
#define VSC_OPTN_ISOptions_TRACE_DAG                        0x8
#define VSC_OPTN_ISOptions_TRACE_HEURISTIC                  0x10
#define VSC_OPTN_ISOptions_TRACE_BUBBLESCHED                0x20
#define VSC_OPTN_ISOptions_TRACE_OUTPUT_BB                  0x40
#define VSC_OPTN_ISOptions_TRACE_OUTPUT_CFG                 0x80

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
    gctBOOL switch_on;
    gctUINT32 heuristics;
    gctUINT32 opts;
    gctTRACE trace;
    gctUINT32 registerCount;
    gctUINT32 stBubbleSize;         /* set the bubble size for st instructions */
    gctUINT32 spillBeforeShader;
    gctUINT32 spillAfterShader;

} VSC_OPTN_RAOptions;

#define VSC_OPTN_RAOptions_GetSwitchOn(option)          ((option)->switch_on)
#define VSC_OPTN_RAOptions_SetSwitchOn(option, s)       ((option)->switch_on = (s))
#define VSC_OPTN_RAOptions_GetHeuristics(option)        ((option)->heuristics)
#define VSC_OPTN_RAOptions_SetHeuristics(option, h)     ((option)->heuristics = (h))

#define VSC_OPTN_RAOptions_SPILL_FIRST                  0x1

#define VSC_OPTN_RAOptions_GetOPTS(option)              ((option)->opts)
#define VSC_OPTN_RAOptions_SetOPTS(option, s)           ((option)->opts = (s))

#define VSC_OPTN_RAOptions_ALLOC_REG                      0x1
#define VSC_OPTN_RAOptions_ALLOC_UNIFORM                  0x2

#define VSC_OPTN_RAOptions_GetTrace(option)             ((option)->trace)
#define VSC_OPTN_RAOptions_SetTrace(option, t)          ((option)->trace = (t))

#define VSC_OPTN_RAOptions_TRACE_BUILD_LR               0x1
#define VSC_OPTN_RAOptions_TRACE_SORT_LR                0x2
#define VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR           0x4
#define VSC_OPTN_RAOptions_TRACE_FINAL                  0x8
#define VSC_OPTN_RAOptions_TRACE_INPUT                  0x10
#define VSC_OPTN_RAOptions_TRACE    VSC_OPTN_RAOptions_TRACE_BUILD_LR | \
                                    VSC_OPTN_RAOptions_TRACE_SORT_LR | \
                                    VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR | \
                                    VSC_OPTN_RAOptions_TRACE_FINAL

#define VSC_OPTN_RAOptions_SetRegisterCount(option, s)  ((option)->registerCount = (s))
#define VSC_OPTN_RAOptions_GetRegisterCount(option)     ((option)->registerCount)

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
    gctBOOL     switch_on;
    gctUINT32   opts;
    gctTRACE    trace;
} VSC_OPTN_CPPOptions;

#define VSC_OPTN_CPPOptions_GetSwitchOn(option)             ((option)->switch_on)
#define VSC_OPTN_CPPOptions_SetSwitchOn(option, s)          ((option)->switch_on = (s))
#define VSC_OPTN_CPPOptions_GetOPTS(option)                 ((option)->opts)
#define VSC_OPTN_CPPOptions_SetOPTS(option, s)              ((option)->opts = (s))
#define VSC_OPTN_CPPOptions_GetTrace(option)                ((option)->trace)
#define VSC_OPTN_CPPOptions_SetTrace(option, t)             ((option)->trace = (t))

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
    gctBOOL     switch_on;
    gctUINT32   before_shader;
    gctUINT32   after_shader;
    gctUINT32   before_func;
    gctUINT32   after_func;
    gctTRACE    trace;
} VSC_OPTN_CPFOptions;

#define VSC_OPTN_CPFOptions_GetSwitchOn(option)             ((option)->switch_on)
#define VSC_OPTN_CPFOptions_SetSwitchOn(option, s)          ((option)->switch_on = (s))
#define VSC_OPTN_CPFOptions_GetBeforeShader(option)         ((option)->before_shader)
#define VSC_OPTN_CPFOptions_SetBeforeShader(option, b)      ((option)->before_shader = (b))
#define VSC_OPTN_CPFOptions_GetAfterShader(option)          ((option)->after_shader)
#define VSC_OPTN_CPFOptions_SetAfterShader(option, a)       ((option)->after_shader = (a))
#define VSC_OPTN_CPFOptions_GetBeforeFunc(option)           ((option)->before_func)
#define VSC_OPTN_CPFOptions_SetBeforeFunc(option, b)        ((option)->before_func = (b))
#define VSC_OPTN_CPFOptions_GetAfterFunc(option)            ((option)->after_func)
#define VSC_OPTN_CPFOptions_SetAfterFunc(option, a)         ((option)->after_func = (a))
#define VSC_OPTN_CPFOptions_GetTrace(option)                ((option)->trace)
#define VSC_OPTN_CPFOptions_SetTrace(option, t)             ((option)->trace = (t))

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

/* Common Subexpression Elimination options */
typedef struct _VSC_OPTN_LCSEOPTIONS
{
    gctBOOL     switch_on;
    gctUINT32   opts;
    gctUINT32   before_shader;
    gctUINT32   after_shader;
    gctUINT32   before_func;
    gctUINT32   after_func;
    gctTRACE    trace;
} VSC_OPTN_LCSEOptions;

#define VSC_OPTN_LCSEOptions_GetSwitchOn(option)            ((option)->switch_on)
#define VSC_OPTN_LCSEOptions_SetSwitchOn(option, s)         ((option)->switch_on = (s))
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
#define VSC_OPTN_LCSEOptions_GetTrace(option)               ((option)->trace)
#define VSC_OPTN_LCSEOptions_SetTrace(option, t)            ((option)->trace = (t))

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
    gctBOOL  switch_on;
    gctUINT32 opts;
    gctTRACE  trace;
} VSC_OPTN_DCEOptions;

#define VSC_OPTN_DCEOptions_GetSwitchOn(option)             ((option)->switch_on)
#define VSC_OPTN_DCEOptions_SetSwitchOn(option, s)          ((option)->switch_on = (s))
#define VSC_OPTN_DCEOptions_GetOPTS(option)                 ((option)->opts)
#define VSC_OPTN_DCEOptions_SetOPTS(option, s)              ((option)->opts = (s))

#define VSC_OPTN_DCEOptions_OPTS_CONTROL                    (0x01)

#define VSC_OPTN_DCEOptions_GetTrace(option)                ((option)->trace)
#define VSC_OPTN_DCEOptions_SetTrace(option, t)             ((option)->trace = (t))

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

/* Dual16 options */
typedef struct _VSC_OPTN_DUAL16OPTIONS
{
    gctFLOAT percentage;
    gctTRACE trace;
} VSC_OPTN_DUAL16Options;

#define VSC_OPTN_DUAL16Options_GetPercentage(option)        ((option)->percentage)
#define VSC_OPTN_DUAL16Options_SetPercentage(option, p)     ((option)->percentage = (p))
#define VSC_OPTN_DUAL16Options_GetTrace(option)             ((option)->trace)
#define VSC_OPTN_DUAL16Options_SetTrace(option, t)          ((option)->trace = (t))

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
    gctBOOL switch_on;
    gctUINT32 opts;
    gctTRACE trace;
} VSC_OPTN_FCPOptions;

#define VSC_OPTN_FCPOptions_GetSwitchOn(option)          ((option)->switch_on)
#define VSC_OPTN_FCPOptions_SetSwitchOn(option, s)       ((option)->switch_on = (s))

#define VSC_OPTN_FCPOptions_GetOPTS(option)              ((option)->opts)
#define VSC_OPTN_FCPOptions_SetOPTS(option, s)           ((option)->opts = (s))

#define VSC_OPTN_FCPOptions_REPL_LDARR                   0x1
#define VSC_OPTN_FCPOptions_SPLIT_DUAL32                 0x2

#define VSC_OPTN_FCPOptions_GetTrace(option)             ((option)->trace)
#define VSC_OPTN_FCPOptions_SetTrace(option, t)          ((option)->trace = (t))

#define VSC_OPTN_FCPOptions_TRACE_INPUT                  0x1
#define VSC_OPTN_FCPOptions_TRACE_OUTPUT                 0x2

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
    gctBOOL   switch_on;
    gctUINT32 opts;
    gctTRACE  trace;
} VSC_OPTN_MCGenOptions;

#define VSC_OPTN_MCGenOptions_GetSwitchOn(option)             ((option)->switch_on)
#define VSC_OPTN_MCGenOptions_SetSwitchOn(option, s)          ((option)->switch_on = (s))
#define VSC_OPTN_MCGenOptions_GetOPTS(option)                 ((option)->opts)
#define VSC_OPTN_MCGenOptions_SetOPTS(option, s)              ((option)->opts = (s))

#define VSC_OPTN_MCGenOptions_GetTrace(option)                ((option)->trace)
#define VSC_OPTN_MCGenOptions_SetTrace(option, t)             ((option)->trace = (t))

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
    gctTRACE  trace;
} VSC_OPTN_SEPGenOptions;

#define VSC_OPTN_SEPGenOptions_GetTrace(option)                ((option)->trace)
#define VSC_OPTN_SEPGenOptions_SetTrace(option, t)             ((option)->trace = (t))

void VSC_OPTN_SEPGenOptions_SetDefault(
    IN OUT VSC_OPTN_SEPGenOptions* options
    );

typedef struct _VSC_OPTN_OPTIONS
{
    gctBOOL                     initialized;
    gctBOOL                     reset;
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
    VSC_OPTN_LCSEOptions        cse_options;
    VSC_OPTN_DCEOptions         dce_options;
    VSC_OPTN_ISOptions          postra_is_options;
    VSC_OPTN_DUAL16Options      dual16_options;
    VSC_OPTN_FCPOptions         fcp_options;
    VSC_OPTN_MCGenOptions       mcgen_options;
    VSC_OPTN_SEPGenOptions      sepgen_options;
    gctBOOL                     dump_options;
    gctBOOL                     options_usage;
} VSC_OPTN_Options;

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
#define VSC_OPTN_Options_GetLCSEOptions(option)             (&((option)->cse_options))
#define VSC_OPTN_Options_GetDCEOptions(option)              (&((option)->dce_options))
#define VSC_OPTN_Options_GetMCGenOptions(option)            (&((option)->mcgen_options))
#define VSC_OPTN_Options_GetSEPGenOptions(option)           (&((option)->sepgen_options))
#define VSC_OPTN_Options_GetPostRAISOptions(option)         (&((option)->postra_is_options))
#define VSC_OPTN_Options_GetDUAL16Options(option)           (&((option)->dual16_options))
#define VSC_OPTN_Options_GetFCPOptions(option)              (&((option)->fcp_options))
#define VSC_OPTN_Options_GetDumpOptions(option)             ((option)->dump_options)
#define VSC_OPTN_Options_SetDumpOptions(option, d)          ((option)->dump_options = (d))
#define VSC_OPTN_Options_GetOptionsUsage(option)            ((option)->options_usage)
#define VSC_OPTN_Options_SetOptionsUsage(option, u)         ((option)->options_usage = (u))
#define VSC_OPTN_Options_SetReset(option, u)                ((option)->reset = (u))

extern VSC_OPTN_Options theVSCOption;

VSC_OPTN_Options *
VSC_OPTN_Get_Options(void);

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


void VSC_OPTN_Options_GetOptionFromEnv(
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

