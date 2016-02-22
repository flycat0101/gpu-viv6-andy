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


#ifndef __gc_vsc_vir_uniform_h_
#define __gc_vsc_vir_uniform_h_

#include "gc_vsc.h"

BEGIN_EXTERN_C()

typedef struct VSC_ALLSHADERS VSC_AllShaders;

typedef struct VSC_GLOBALUNIFORMITEM
{
    VSC_UNI_LIST_NODE node;
    VIR_Id id;
    VSC_AllShaders* all_shaders;
    VIR_SymId uniforms[VSC_MAX_LINKABLE_SHADER_STAGE_COUNT];
    VIR_SymFlag flags;
    gctINT location;
    gctINT range;
    gctUINT reg_count;
    gctUINT byte_size;
    gctINT offset;
} VSC_GlobalUniformItem;

#define VSC_GlobalUniformItem_GetID(gui)                        ((gui)->id)
#define VSC_GlobalUniformItem_SetID(gui, i)                     ((gui)->id = (i))
#define VSC_GlobalUniformItem_GetShader(gui, i)                 ((gui)->all_shaders->shaders[i])
#define VSC_GlobalUniformItem_SetAllShaders(gui, a)             ((gui)->all_shaders = (a))
#define VSC_GlobalUniformItem_GetGlobalUniformTable(gui)        (&(gui)->all_shaders->global_uniform_table)
#define VSC_GlobalUniformItem_GetUniform(gui, i)                ((gui)->uniforms[i])
#define VSC_GlobalUniformItem_SetUniform(gui, i, u)             ((gui)->uniforms[i] = (u))
#define VSC_GlobalUniformItem_IsInShader(gui, i)                VIR_Id_isValid(VSC_GlobalUniformItem_GetUniform(item, i))
#define VSC_GlobalUniformItem_GetFlags(gui)                     ((gui)->flags)
#define VSC_GlobalUniformItem_SetFlag(gui, f)                   ((gui)->flags = (VIR_SymFlag)((gctUINT)(gui)->flags | (gctUINT)(f)))
#define VSC_GlobalUniformItem_ResetFlag(gui, f)                 ((gui)->flags = (VIR_SymFlag)((gctUINT)(gui)->flags & ~(gctUINT)(f)))
#define VSC_GlobalUniformItem_HasFlag(gui, f)                   ((gctUINT)(gui)->flags & (gctUINT)(f))
#define VSC_GlobalUniformItem_ClearFlags(gui)                   ((gui)->flags = VIR_SYMFLAG_NONE)
#define VSC_GlobalUniformItem_GetLocation(gui)                  ((gui)->location)
#define VSC_GlobalUniformItem_SetLocation(gui, i)               ((gui)->location = (i))
#define VSC_GlobalUniformItem_GetRange(gui)                     ((gui)->range)
#define VSC_GlobalUniformItem_SetRange(gui, r)                  ((gui)->range = (r))
#define VSC_GlobalUniformItem_GetRegCount(gui)                  ((gui)->reg_count)
#define VSC_GlobalUniformItem_SetRegCount(gui, r)               ((gui)->reg_count = (r))
#define VSC_GlobalUniformItem_GetByteSize(gui)                  ((gui)->byte_size)
#define VSC_GlobalUniformItem_SetByteSize(gui, b)               ((gui)->byte_size = (b))
#define VSC_GlobalUniformItem_GetOffset(gui)                    ((gui)->offset)
#define VSC_GlobalUniformItem_SetOffset(gui, o)                 ((gui)->offset = (o))
#define VSC_GlobalUniformItem_GetDumper(gui)                    ((gui)->all_shaders->dumper)

void
VSC_GlobalUniformItem_Initialize(
    IN OUT VSC_GlobalUniformItem* global_uniform_item,
    IN VSC_AllShaders* all_shaders,
    IN VIR_Id id
    );

void
VSC_GlobalUniformItem_Update(
    IN OUT VSC_GlobalUniformItem* item,
    IN VIR_Shader* shader,
    IN VIR_SymId uniform_symid
    );

gctBOOL
VSC_GlobalUniformItem_UniformTypeMatch(
    IN VSC_GlobalUniformItem* item,
    IN VIR_Shader* shader,
    IN VIR_SymId uniform_symid
    );

gctBOOL
VSC_GlobalUniformItem_SuitableForPickingIntoAuxUBO(
    IN VSC_GlobalUniformItem* item,
    IN gctBOOL useDUBO,
    IN gctBOOL useCUBO
    );

void
VSC_GlobalUniformItem_SetInDUBO(
    IN OUT VSC_GlobalUniformItem* item
    );

void
VSC_GlobalUniformItem_SetOffsetByAll(
    IN OUT VSC_GlobalUniformItem* item,
    IN gctUINT offset
    );

void
VSC_GlobalUniformItem_Dump(
    VSC_GlobalUniformItem* global_uniform_item
    );

typedef struct VSC_GLOBALUNIFORMTABLE
{
    VSC_AllShaders* all_shaders;
    VSC_UNI_LIST item_list;
    VSC_HASH_TABLE* name_map;
    gctUINT item_count;
    gctBOOL has_active_uniform;
    VSC_MM mem_pool;
} VSC_GlobalUniformTable;

#define VSC_GlobalUniformTable_GetAllShaders(gut)                   ((gut)->all_shaders)
#define VSC_GlobalUniformTable_SetAllShaders(gut, a)                ((gut)->all_shaders = (a))
#define VSC_GlobalUniformTable_GetShader(gut, i)                    ((gut)->all_shaders->shaders[i])
#define VSC_GlobalUniformTable_GetItemList(gut)                     (&(gut)->item_list)
#define VSC_GlobalUniformTable_GetNameMap(gut)                      ((gut)->name_map)
#define VSC_GlobalUniformTable_SetNameMap(gut, n)                   ((gut)->name_map = (n))
#define VSC_GlobalUniformTable_GetItemCount(gut)                    ((gut)->item_count)
#define VSC_GlobalUniformTable_SetItemCount(gut, i)                 ((gut)->item_count = (i))
#define VSC_GlobalUniformTable_IncItemCount(gut)                    (++(gut)->item_count)
#define VSC_GlobalUniformTable_GetHasActiveUniform(gut)             ((gut)->has_active_uniform)
#define VSC_GlobalUniformTable_SetHasActiveUniform(gut)             ((gut)->has_active_uniform = gcvTRUE)
#define VSC_GlobalUniformTable_ResetHasActiveUniform(gut)           ((gut)->has_active_uniform = gcvFALSE)
#define VSC_GlobalUniformTable_GetDumper(gut)                       ((gut)->all_shaders->dumper)
#define VSC_GlobalUniformTable_GetMM(gut)                           (&(gut)->mem_pool)
#define VSC_GlobalUniformTable_SetMM(gut, m)                        ((gut)->mem_pool = *(m))

void
VSC_GlobalUniformTable_Initialize(
    IN OUT VSC_GlobalUniformTable* global_uniform_table,
    IN VSC_AllShaders* all_shaders,
    IN VSC_MM* mm_wrapper
    );

VSC_GlobalUniformItem*
VSC_GlobalUniformTable_NewItem(
    IN OUT VSC_GlobalUniformTable* global_uniform_table
    );

VSC_GlobalUniformItem*
VSC_GlobalUniformTable_FindUniformWithLocation(
    IN VSC_GlobalUniformTable* global_uniform_table,
    IN gctINT location,
    OUT gctBOOL* from_head
    );

VSC_GlobalUniformItem*
VSC_GlobalUniformTable_FindUniformWithName(
    IN VSC_GlobalUniformTable* global_uniform_table,
    IN gctSTRING name
    );

VSC_ErrCode
VSC_GlobalUniformTable_FindUniformWithShaderUniform(
    IN VSC_GlobalUniformTable* global_uniform_table,
    IN VIR_Shader* shader,
    IN VIR_SymId uniform_symid,
    OUT VSC_GlobalUniformItem** item
    );

VSC_ErrCode
VSC_GlobalUniformTable_InsertShaderUniform(
    IN OUT VSC_GlobalUniformTable* global_uniform_table,
    IN VIR_Shader* shader,
    IN VIR_SymId uniform
    );

typedef VSC_UL_ITERATOR VSC_GlobalUniformTable_Iterator;

void
VSC_GlobalUniformTable_Iterator_Init(
    IN OUT VSC_GlobalUniformTable_Iterator* iter,
    IN VSC_GlobalUniformTable* gut
    );

VSC_GlobalUniformItem*
VSC_GlobalUniformTable_Iterator_First(
    IN VSC_GlobalUniformTable_Iterator* iter
    );

VSC_GlobalUniformItem*
VSC_GlobalUniformTable_Iterator_Next(
    VSC_GlobalUniformTable_Iterator* iter
    );

VSC_GlobalUniformItem*
VSC_GlobalUniformTable_Iterator_Last(
    VSC_GlobalUniformTable_Iterator* iter
    );

void
VSC_GlobalUniformTable_Dump(
    IN VSC_GlobalUniformTable* global_uniform_table
    );

struct VSC_ALLSHADERS
{
    VIR_Shader* shaders[VSC_MAX_LINKABLE_SHADER_STAGE_COUNT];
    VSC_GlobalUniformTable global_uniform_table;
    VSC_HW_CONFIG* hwCfg;
    VIR_Dumper* dumper;
    VSC_MM mem_pool;
};

#define VSC_AllShaders_GetShader(as, i)                 ((as)->shaders[i])
#define VSC_AllShaders_SetShader(as, i, s)              ((as)->shaders[i] = (s))
#define VSC_AllShaders_GetGlobalUniformTable(as)        (&(as)->global_uniform_table)
#define VSC_AllShaders_GetGlobalUniformTable(as)        (&(as)->global_uniform_table)
#define VSC_AllShaders_GetDumper(as)                    ((as)->dumper)
#define VSC_AllShaders_SetDumper(as, d)                 ((as)->dumper = (d))
#define VSC_AllShaders_GetMM(as)                        (&(as)->mem_pool)
#define VSC_AllShaders_SetMM(as, m)                     ((as)->mem_pool = *(m))
#define VSC_AllShaders_SetHwCFG(as, hc)                    ((as)->hwCfg = (hc))

void
VSC_AllShaders_Initialize(
    IN OUT VSC_AllShaders* all_shaders,
    IN VIR_Shader* vs_shader,
    IN VIR_Shader* hs_shader,
    IN VIR_Shader* ds_shader,
    IN VIR_Shader* gs_shader,
    IN VIR_Shader* ps_shader,
    IN VIR_Shader* cs_shader,
    IN VSC_HW_CONFIG* hwCfg,
    IN VIR_Dumper* dumper,
    IN VSC_MM* mem_pool
    );

void
VSC_AllShaders_Finalize(
    IN OUT VSC_AllShaders* all_shaders
    );

VSC_ErrCode
VSC_AllShaders_LinkUniforms(
    IN OUT VSC_AllShaders* all_shaders
    );

extern VSC_ErrCode
VSC_UF_UtilizeAuxUBO(
    IN OUT VSC_AllShaders           *all_shaders,
    IN VSC_OPTN_UF_AUBO_Options     *options
    );

VSC_ErrCode
VSC_GetUniformIndexingRange(
    IN OUT VIR_Shader       *pShader,
    IN gctINT               uniformIndex,
    OUT gctINT              *LastUniformIndex
    );

VSC_ErrCode
VSC_CheckUniformUsage(
    IN OUT VIR_Shader       *pShader
    );

END_EXTERN_C()

#endif

