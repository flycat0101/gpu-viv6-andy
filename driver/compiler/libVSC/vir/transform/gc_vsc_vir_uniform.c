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


#include "vir/transform/gc_vsc_vir_uniform.h"

/* VSC_GlobalUniformItem methods */
void
VSC_GlobalUniformItem_Initialize(
    IN OUT VSC_GlobalUniformItem* global_uniform_item,
    IN VSC_AllShaders* all_shaders,
    IN VIR_Id id
    )
{
    gctUINT i;

    VSC_GlobalUniformItem_SetID(global_uniform_item, id);
    VSC_GlobalUniformItem_SetAllShaders(global_uniform_item, all_shaders);
    for(i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
    {
        VSC_GlobalUniformItem_SetUniform(global_uniform_item, i, VIR_INVALID_ID);
    }
    VSC_GlobalUniformItem_ClearFlags(global_uniform_item);
    VSC_GlobalUniformItem_SetLocation(global_uniform_item, -1);
    VSC_GlobalUniformItem_SetRange(global_uniform_item, 0);
    VSC_GlobalUniformItem_SetRegCount(global_uniform_item, 0);
    VSC_GlobalUniformItem_SetByteSize(global_uniform_item, 0);
    VSC_GlobalUniformItem_SetOffset(global_uniform_item, -1);
}

void
VSC_GlobalUniformItem_Update(
    IN OUT VSC_GlobalUniformItem* item,
    IN VIR_Shader* shader,
    IN VIR_SymId uniform_symid
    )
{
    VSC_GlobalUniformTable* global_uniform_table = VSC_GlobalUniformItem_GetGlobalUniformTable(item);
    gctUINT shader_kind_id = VIR_ShaderKind_Map2KindId(VIR_Shader_GetKind(shader));
    VIR_Symbol* uniform_sym = VIR_Shader_GetSymFromId(shader, uniform_symid);
    VIR_Type* uniform_type = VIR_Symbol_GetType(uniform_sym);
    gctINT location = VIR_Symbol_GetLocation(uniform_sym);
    gctSTRING name = VIR_Shader_GetSymNameString(shader, uniform_sym);

    VSC_GlobalUniformItem_SetUniform(item, shader_kind_id, uniform_symid);
    VSC_GlobalUniformItem_SetFlag(item, VIR_Symbol_GetFlag(uniform_sym));
    /* mark the uniform item inactive by default. Following pass will go over
       instructions and mark those active ones */
    VSC_GlobalUniformItem_SetFlag(item, VIR_SYMFLAG_INACTIVE);
    if(location != -1)
    {
        VSC_GlobalUniformItem_SetLocation(item, location);
        VSC_GlobalUniformItem_SetRange(item, VIR_Shader_GetLogicalCount(shader, uniform_type));
    }
    VSC_GlobalUniformItem_SetRegCount(item, VIR_Type_GetVirRegCount(shader, uniform_type));
    VSC_GlobalUniformItem_SetByteSize(item, VIR_Shader_GetTypeByteSize(shader, uniform_type));
    vscHTBL_DirectSet(VSC_GlobalUniformTable_GetNameMap(global_uniform_table), name, item);
}

gctBOOL
VSC_GlobalUniformItem_UniformTypeMatch(
    IN VSC_GlobalUniformItem* item,
    IN VIR_Shader* shader,
    IN VIR_SymId uniform_symid
    )
{
    return gcvTRUE;
}

gctBOOL
VSC_GlobalUniformItem_SuitableForPickingIntoAuxUBO(
    IN VSC_GlobalUniformItem* item,
    IN gctBOOL useDUBO,
    IN gctBOOL useCUBO
    )
{
    gctUINT i;

    if(VSC_GlobalUniformItem_HasFlag(item, VIR_SYMFLAG_INACTIVE) ||
       VSC_GlobalUniformItem_HasFlag(item, VIR_SYMUNIFORMFLAG_ALWAYS_IN_DUB) ||
       VSC_GlobalUniformItem_HasFlag(item, VIR_SYMUNIFORMFLAG_MOVED_TO_DUBO) ||
       VSC_GlobalUniformItem_HasFlag(item, VIR_SYMUNIFORMFLAG_MOVED_TO_CUBO))
    {
        return gcvFALSE;
    }

    for(i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
    {
        VIR_SymId uniform_symid = VSC_GlobalUniformItem_GetUniform(item, i);
        if(VIR_Id_isValid(uniform_symid))
        {
            VIR_Shader* shader = VSC_GlobalUniformItem_GetShader(item, i);
            gcmASSERT(shader);
            {
                VIR_Symbol* uniform_sym = VIR_Shader_GetSymFromId(shader, uniform_symid);
                if(!VIR_Symbol_isUniform(uniform_sym) ||
                   VIR_Symbol_GetUniformKind(uniform_sym) != VIR_UNIFORM_NORMAL ||
                   VIR_Symbol_HasFlag(uniform_sym, VIR_SYMUNIFORMFLAG_ATOMIC_COUNTER) ||
                   VIR_Symbol_HasFlag(uniform_sym, VIR_SYMUNIFORMFLAG_USED_IN_LTC) ||      /* this condition may be optimized */
                   VIR_Symbol_HasFlag(uniform_sym, VIR_SYMFLAG_BUILTIN) ||
                   (VIR_Symbol_HasFlag(uniform_sym, VIR_SYMFLAG_COMPILER_GEN) && !VIR_Symbol_HasFlag(uniform_sym, VIR_SYMUNIFORMFLAG_COMPILETIME_INITIALIZED)))
                {
                    return gcvFALSE;
                }
                if(!useCUBO && VIR_Symbol_HasFlag(uniform_sym, VIR_SYMUNIFORMFLAG_COMPILETIME_INITIALIZED))
                {
                    return gcvFALSE;
                }
                if(!useDUBO && !VIR_Symbol_HasFlag(uniform_sym, VIR_SYMUNIFORMFLAG_COMPILETIME_INITIALIZED))
                {
                    return gcvFALSE;
                }
            }
        }
    }

    return gcvTRUE;
}

void
VSC_GlobalUniformItem_SetInDUBO(
    IN OUT VSC_GlobalUniformItem* item
    )
{
    gctUINT i;

    gcmASSERT(!VSC_GlobalUniformItem_HasFlag(item, VIR_SYMUNIFORMFLAG_MOVED_TO_DUBO));

    for(i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
    {
        VIR_SymId uniform_symid = VSC_GlobalUniformItem_GetUniform(item, i);
        if(VIR_Id_isValid(uniform_symid))
        {
            VIR_Shader* shader = VSC_GlobalUniformItem_GetShader(item, i);
            VIR_Symbol* uniform_sym = VIR_Shader_GetSymFromId(shader, uniform_symid);
            VIR_Symbol_AddFlag(uniform_sym, VIR_SYMUNIFORMFLAG_MOVED_TO_DUBO);
        }
    }
    VSC_GlobalUniformItem_SetFlag(item, VIR_SYMUNIFORMFLAG_MOVED_TO_DUBO);
}

void
VSC_GlobalUniformItem_SetInCUBO(
    IN OUT VSC_GlobalUniformItem* item
    )
{
    gctUINT i;

    gcmASSERT(!VSC_GlobalUniformItem_HasFlag(item, VIR_SYMUNIFORMFLAG_MOVED_TO_CUBO));

    for(i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
    {
        VIR_SymId uniform_symid = VSC_GlobalUniformItem_GetUniform(item, i);
        if(VIR_Id_isValid(uniform_symid))
        {
            VIR_Shader* shader = VSC_GlobalUniformItem_GetShader(item, i);
            VIR_Symbol* uniform_sym = VIR_Shader_GetSymFromId(shader, uniform_symid);
            VIR_Symbol_AddFlag(uniform_sym, VIR_SYMUNIFORMFLAG_MOVED_TO_CUBO);
        }
    }
    VSC_GlobalUniformItem_SetFlag(item, VIR_SYMUNIFORMFLAG_MOVED_TO_CUBO);
}

void
VSC_GlobalUniformItem_SetOffsetByAll(
    IN OUT VSC_GlobalUniformItem* item,
    IN gctUINT offset
    )
{
    gctUINT i;

    for(i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
    {
        VIR_SymId uniform_symid = VSC_GlobalUniformItem_GetUniform(item, i);
        if(VIR_Id_isValid(uniform_symid))
        {
            VIR_Shader* shader = VSC_GlobalUniformItem_GetShader(item, i);
            VIR_Symbol* uniform_sym = VIR_Shader_GetSymFromId(shader, uniform_symid);
            VIR_Uniform* uniform = VIR_Symbol_GetUniform(uniform_sym);
            VIR_Uniform_SetOffset(uniform, offset);
        }
    }
    VSC_GlobalUniformItem_SetOffset(item, offset);
}

void
VSC_GlobalUniformItem_Dump(
    VSC_GlobalUniformItem* global_uniform_item
    )
{
    gctUINT i;
    VIR_Dumper* dumper = VSC_GlobalUniformItem_GetDumper(global_uniform_item);

    VIR_LOG(dumper, "global uniform item(id:%d):\n", VSC_GlobalUniformItem_GetID(global_uniform_item));
    VIR_LOG_FLUSH(dumper);
    for(i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
    {
        VIR_SymId uniform_symid = VSC_GlobalUniformItem_GetUniform(global_uniform_item, i);
        if(VIR_Id_isValid(uniform_symid))
        {
            VIR_Shader* shader = VSC_GlobalUniformItem_GetShader(global_uniform_item, i);
            VIR_Symbol* uniform_sym = VIR_Shader_GetSymFromId(shader, uniform_symid);
            VIR_Uniform* uniform = gcvNULL;

            VIR_LOG(dumper, "shader(id:%d)\n", VIR_Shader_GetId(shader));
            VIR_LOG_FLUSH(dumper);
            switch(VIR_Symbol_GetKind(uniform_sym))
            {
                case VIR_SYM_UNIFORM:
                {
                    uniform = VIR_Symbol_GetUniform(uniform_sym);
                    break;
                }
                case VIR_SYM_SAMPLER:
                {
                    uniform = VIR_Symbol_GetSampler(uniform_sym);
                    break;
                }
                case VIR_SYM_IMAGE:
                {
                    uniform = VIR_Symbol_GetImage(uniform_sym);
                    break;
                }
                default:
                    gcmASSERT(0);
            }
            VIR_Uniform_Dump(shader->dumper, uniform);
        }
    }
    if(VSC_GlobalUniformItem_GetLocation(global_uniform_item) != -1)
    {
        VIR_LOG(dumper, "location: %d\n", VSC_GlobalUniformItem_GetLocation(global_uniform_item));
        VIR_LOG(dumper, "range: %d\n", VSC_GlobalUniformItem_GetRange(global_uniform_item));
    }
    if(VSC_GlobalUniformItem_GetRegCount(global_uniform_item))
    {
        VIR_LOG(dumper, "reg count: %d\n", VSC_GlobalUniformItem_GetRegCount(global_uniform_item));
    }
    if(VSC_GlobalUniformItem_GetByteSize(global_uniform_item))
    {
        VIR_LOG(dumper, "byte size: %d\n", VSC_GlobalUniformItem_GetByteSize(global_uniform_item));
    }
    if(VSC_GlobalUniformItem_GetOffset(global_uniform_item) != -1)
    {
        VIR_LOG(dumper, "offset: %d\n", VSC_GlobalUniformItem_GetOffset(global_uniform_item));
    }
    VIR_LOG(dumper, "\n");
    VIR_LOG_FLUSH(dumper);
}

/* VSC_GlobalUniformTable methods */
void
VSC_GlobalUniformTable_Initialize(
    IN OUT VSC_GlobalUniformTable* global_uniform_table,
    IN VSC_AllShaders* all_shaders,
    IN VSC_MM* mm_wrapper
    )
{
    VSC_GlobalUniformTable_SetAllShaders(global_uniform_table, all_shaders);
    vscUNILST_Initialize(VSC_GlobalUniformTable_GetItemList(global_uniform_table), gcvFALSE);
    VSC_GlobalUniformTable_SetNameMap(global_uniform_table, vscHTBL_Create(mm_wrapper, vscHFUNC_String, vcsHKCMP_String, 512));
    VSC_GlobalUniformTable_ResetHasActiveUniform(global_uniform_table);
    VSC_GlobalUniformTable_SetItemCount(global_uniform_table, 0);
    VSC_GlobalUniformTable_SetMM(global_uniform_table, mm_wrapper);
}

VSC_GlobalUniformItem*
VSC_GlobalUniformTable_NewItem(
    IN OUT VSC_GlobalUniformTable* global_uniform_table
    )
{
    VSC_GlobalUniformItem* item = (VSC_GlobalUniformItem*)vscMM_Alloc(VSC_GlobalUniformTable_GetMM(global_uniform_table), sizeof(VSC_GlobalUniformItem));

    VSC_GlobalUniformItem_Initialize(item, VSC_GlobalUniformTable_GetAllShaders(global_uniform_table), VSC_GlobalUniformTable_GetItemCount(global_uniform_table));
    VSC_GlobalUniformTable_IncItemCount(global_uniform_table);
    vscUNILST_Append(VSC_GlobalUniformTable_GetItemList(global_uniform_table), (VSC_UNI_LIST_NODE*)item);

    return item;
}

VSC_GlobalUniformItem*
VSC_GlobalUniformTable_FindUniformWithLocation(
    IN VSC_GlobalUniformTable* global_uniform_table,
    IN gctINT location,
    OUT gctBOOL* from_head
    )
{
    VSC_UL_ITERATOR iter;
    VSC_UNI_LIST_NODE* node;

    gcmASSERT(location >= 0);

    vscULIterator_Init(&iter, VSC_GlobalUniformTable_GetItemList(global_uniform_table));
    for(node = vscULIterator_First(&iter);
        node != gcvNULL; node = vscULIterator_Next(&iter))
    {
        VSC_GlobalUniformItem* item = (VSC_GlobalUniformItem*)node;
        gctINT item_location = VSC_GlobalUniformItem_GetLocation(item);
        if(item_location != -1)
        {
            /*if(item_location > location)
            {
                return gcvNULL;
            }
            else*/
            {
                gctUINT item_range = VSC_GlobalUniformItem_GetRange(item);
                if(location == item_location)
                {
                    if(from_head)
                    {
                        *from_head = gcvTRUE;
                    }
                    return item;
                }
                else if(location > item_location && location < (item_location + (gctINT)item_range))
                {
                    if(from_head)
                    {
                        *from_head = gcvFALSE;
                    }
                    return item;
                }
            }
        }
    }
    if(from_head)
    {
        *from_head = gcvFALSE;
    }
    return gcvNULL;
}

VSC_GlobalUniformItem*
VSC_GlobalUniformTable_FindUniformWithName(
    IN VSC_GlobalUniformTable* global_uniform_table,
    IN gctSTRING name
    )
{
    VSC_GlobalUniformItem* item = gcvNULL;
    vscHTBL_DirectTestAndGet(VSC_GlobalUniformTable_GetNameMap(global_uniform_table), name, (void**)&item);
    return item;
}


VSC_ErrCode
VSC_GlobalUniformTable_FindUniformWithShaderUniform(
    IN VSC_GlobalUniformTable* global_uniform_table,
    IN VIR_Shader* shader,
    IN VIR_SymId uniform_symid,
    OUT VSC_GlobalUniformItem** item
    )
{
    VSC_ErrCode error_code = VSC_ERR_NONE;
    VIR_Symbol* uniform_sym;
    gctINT location;
    gctSTRING name;
    VSC_GlobalUniformItem* loc_result = gcvNULL;
    VSC_GlobalUniformItem* name_result = gcvNULL;
    gctBOOL from_head = gcvTRUE;

    gcmASSERT(shader == VSC_GlobalUniformTable_GetShader(global_uniform_table, VIR_ShaderKind_Map2KindId(VIR_Shader_GetKind(shader))));
    gcmASSERT(item);

    uniform_sym = VIR_Shader_GetSymFromId(shader, uniform_symid);
    location = VIR_Symbol_GetLocation(uniform_sym);
    if(location != -1)
    {
        loc_result = VSC_GlobalUniformTable_FindUniformWithLocation(global_uniform_table, location, &from_head);
    }

    name = VIR_Shader_GetSymNameString(shader, uniform_sym);
    name_result = VSC_GlobalUniformTable_FindUniformWithName(global_uniform_table, name);

    if(loc_result)
    {
        if(!from_head)
        {
            return VSC_ERR_LOCATION_ALIASED;
        }

        if(loc_result != name_result)
        {
            return VSC_ERR_LOCATION_MISMATCH;
        }
        *item = loc_result;
    }
    else if(name_result)
    {
        if(VSC_GlobalUniformItem_GetLocation(name_result) != -1
            && location != -1
            && VSC_GlobalUniformItem_GetLocation(name_result) != location)
        {
            return VSC_ERR_LOCATION_MISMATCH;
        }
        *item = name_result;
    }
    else
    {
        *item = gcvNULL;
    }

    if(*item)
    {
        if(!VSC_GlobalUniformItem_UniformTypeMatch(*item, shader, uniform_symid))
        {
            return VSC_ERR_UNIFORM_TYPE_MISMATCH;
        }
    }
    return error_code;
}

VSC_ErrCode
VSC_GlobalUniformTable_InsertShaderUniform(
    IN OUT VSC_GlobalUniformTable* global_uniform_table,
    IN VIR_Shader* shader,
    IN VIR_SymId uniform
    )
{
    VSC_ErrCode error_code = VSC_ERR_NONE;
    VSC_GlobalUniformItem* item;

    gcmASSERT(shader == VSC_GlobalUniformTable_GetShader(global_uniform_table, VIR_ShaderKind_Map2KindId(VIR_Shader_GetKind(shader))));

    error_code = VSC_GlobalUniformTable_FindUniformWithShaderUniform(global_uniform_table, shader, uniform, &item);
    if(error_code != VSC_ERR_NONE)
    {
        return error_code;
    }

    if(!item)
    {
        item = VSC_GlobalUniformTable_NewItem(global_uniform_table);
    }
    VSC_GlobalUniformItem_Update(item, shader, uniform);

    return error_code;
}

void
VSC_GlobalUniformTable_Iterator_Init(
    IN OUT VSC_GlobalUniformTable_Iterator* iter,
    IN VSC_GlobalUniformTable* gut
    )
{
    vscULIterator_Init(iter, VSC_GlobalUniformTable_GetItemList(gut));
}

VSC_GlobalUniformItem*
VSC_GlobalUniformTable_Iterator_First(
    IN VSC_GlobalUniformTable_Iterator* iter
    )
{
    return (VSC_GlobalUniformItem*)vscULIterator_First(iter);
}

VSC_GlobalUniformItem*
VSC_GlobalUniformTable_Iterator_Next(
    IN VSC_GlobalUniformTable_Iterator* iter
    )
{
    return (VSC_GlobalUniformItem*)vscULIterator_Next(iter);
}

VSC_GlobalUniformItem*
VSC_GlobalUniformTable_Iterator_Last(
    IN VSC_GlobalUniformTable_Iterator* iter
    )
{
    return (VSC_GlobalUniformItem*)vscULIterator_Last(iter);
}

void
VSC_GlobalUniformTable_Dump(
    IN VSC_GlobalUniformTable* global_uniform_table
    )
{
    VSC_GlobalUniformTable_Iterator iter;
    VSC_GlobalUniformItem* item;
    gctUINT i;
    VIR_Dumper* dumper = VSC_GlobalUniformTable_GetDumper(global_uniform_table);

    VIR_LOG(dumper, "global uniform table(%x)\n", global_uniform_table);
    VIR_LOG_FLUSH(dumper);
    VSC_GlobalUniformTable_Iterator_Init(&iter, global_uniform_table);
    for(item = VSC_GlobalUniformTable_Iterator_First(&iter), i = 0; item != gcvNULL;
        item = VSC_GlobalUniformTable_Iterator_Next(&iter), i++)
    {
        VSC_GlobalUniformItem_Dump(item);
    }
    VIR_LOG(dumper, "\n");
    VIR_LOG_FLUSH(dumper);
}

/* VSC_AllShaders methods */
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
    )
{
    if(cs_shader)
    {
        VSC_AllShaders_SetShader(all_shaders, VSC_CPT_SHADER_STAGE_CS, cs_shader);
        VSC_AllShaders_SetShader(all_shaders, VSC_GFX_SHADER_STAGE_HS, gcvNULL);
        VSC_AllShaders_SetShader(all_shaders, VSC_GFX_SHADER_STAGE_DS, gcvNULL);
        VSC_AllShaders_SetShader(all_shaders, VSC_GFX_SHADER_STAGE_GS, gcvNULL);
        VSC_AllShaders_SetShader(all_shaders, VSC_GFX_SHADER_STAGE_PS, gcvNULL);
    }
    else
    {
        VSC_AllShaders_SetShader(all_shaders, VSC_GFX_SHADER_STAGE_VS, vs_shader);
        VSC_AllShaders_SetShader(all_shaders, VSC_GFX_SHADER_STAGE_HS, hs_shader);
        VSC_AllShaders_SetShader(all_shaders, VSC_GFX_SHADER_STAGE_DS, ds_shader);
        VSC_AllShaders_SetShader(all_shaders, VSC_GFX_SHADER_STAGE_GS, gs_shader);
        VSC_AllShaders_SetShader(all_shaders, VSC_GFX_SHADER_STAGE_PS, ps_shader);
    }
    VSC_GlobalUniformTable_Initialize(VSC_AllShaders_GetGlobalUniformTable(all_shaders), all_shaders, mem_pool);
    VSC_AllShaders_SetDumper(all_shaders, dumper);
    VSC_AllShaders_SetMM(all_shaders, mem_pool);
    VSC_AllShaders_SetHwCFG(all_shaders, hwCfg);
}

void
VSC_AllShaders_Finalize(
    IN OUT VSC_AllShaders* all_shaders
    )
{
    return;
}

VSC_ErrCode
VSC_AllShaders_LinkUniforms(
    IN OUT VSC_AllShaders* all_shaders
    )
{
    VSC_ErrCode error_code = VSC_ERR_NONE;
    VSC_GlobalUniformTable* global_uniform_table = VSC_AllShaders_GetGlobalUniformTable(all_shaders);
    gctUINT i;

    for(i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
    {
        VIR_Shader* shader = VSC_AllShaders_GetShader(all_shaders, i);
        if(shader)
        {
            VIR_UniformIdList* uniforms = VIR_Shader_GetUniforms(shader);
            gctUINT j;

            VSC_CheckUniformUsage(shader);

            for(j = 0; j < VIR_IdList_Count(uniforms); j++)
            {
                error_code = VSC_GlobalUniformTable_InsertShaderUniform(global_uniform_table, shader, VIR_IdList_GetId(uniforms, j));
                if(error_code != VSC_ERR_NONE)
                {
                    return error_code;
                }
            }
        }
    }
    return error_code;
}

typedef struct VSC_UF_AUBO
{
    VSC_AllShaders* all_shaders;
    gctUINT dub_reg_count[VSC_MAX_LINKABLE_SHADER_STAGE_COUNT];
    gctUINT max_reg_count[VSC_MAX_LINKABLE_SHADER_STAGE_COUNT];
    VIR_SymId aubo[VSC_MAX_LINKABLE_SHADER_STAGE_COUNT];
    VIR_SymId dubo_addr[VSC_MAX_LINKABLE_SHADER_STAGE_COUNT];
    VIR_SymId cubo[VSC_MAX_LINKABLE_SHADER_STAGE_COUNT];
    VIR_SymId cubo_addr[VSC_MAX_LINKABLE_SHADER_STAGE_COUNT];
    gctUINT dubo_item_count[VSC_MAX_LINKABLE_SHADER_STAGE_COUNT];
    gctUINT cubo_item_count[VSC_MAX_LINKABLE_SHADER_STAGE_COUNT];
    gctUINT dubo_byte_size;
    gctUINT cubo_byte_size;
    VSC_OPTN_UF_AUBO_Options* options;
} VSC_UF_AUBO;

#define VSC_UF_AUBO_GetAllShaders(aubo)             ((aubo)->all_shaders)
#define VSC_UF_AUBO_SetAllShaders(aubo, a)          ((aubo)->all_shaders = (a))
#define VSC_UF_AUBO_GetShader(aubo, i)              ((aubo)->all_shaders->shaders[i])
#define VSC_UF_AUBO_GetDUBRegCount(aubo, i)         ((aubo)->dub_reg_count[i])
#define VSC_UF_AUBO_SetDUBRegCount(aubo, i, d)      ((aubo)->dub_reg_count[i] = (d))
#define VSC_UF_AUBO_IncDUBRegCount(aubo, i, d)      ((aubo)->dub_reg_count[i] = (aubo)->dub_reg_count[i] + (d))
#define VSC_UF_AUBO_GetMaxRegCount(aubo, i)         ((aubo)->max_reg_count[i])
#define VSC_UF_AUBO_SetMaxRegCount(aubo, i, m)      ((aubo)->max_reg_count[i] = (m))
#define VSC_UF_AUBO_GetDUBO(aubo, i)                ((aubo)->aubo[i])
#define VSC_UF_AUBO_SetDUBO(aubo, i, d)             ((aubo)->aubo[i] = (d))
#define VSC_UF_AUBO_GetDUBOAddr(aubo, i)            ((aubo)->dubo_addr[i])
#define VSC_UF_AUBO_SetDUBOAddr(aubo, i, d)         ((aubo)->dubo_addr[i] = (d))
#define VSC_UF_AUBO_GetCUBO(aubo, i)                ((aubo)->cubo[i])
#define VSC_UF_AUBO_SetCUBO(aubo, i, d)             ((aubo)->cubo[i] = (d))
#define VSC_UF_AUBO_GetCUBOAddr(aubo, i)            ((aubo)->cubo_addr[i])
#define VSC_UF_AUBO_SetCUBOAddr(aubo, i, d)         ((aubo)->cubo_addr[i] = (d))
#define VSC_UF_AUBO_GetDUBOItemCount(aubo, i)       ((aubo)->dubo_item_count[i])
#define VSC_UF_AUBO_SetDUBOItemCount(aubo, i, c)    ((aubo)->dubo_item_count[i] = (c))
#define VSC_UF_AUBO_IncDUBOItemCount(aubo, i)       ((aubo)->dubo_item_count[i]++)
#define VSC_UF_AUBO_GetCUBOItemCount(aubo, i)       ((aubo)->cubo_item_count[i])
#define VSC_UF_AUBO_SetCUBOItemCount(aubo, i, c)    ((aubo)->cubo_item_count[i] = (c))
#define VSC_UF_AUBO_IncCUBOItemCount(aubo, i)       ((aubo)->cubo_item_count[i]++)
#define VSC_UF_AUBO_GetDUBOByteSize(aubo)           ((aubo)->dubo_byte_size)
#define VSC_UF_AUBO_SetDUBOByteSize(aubo, d)        ((aubo)->dubo_byte_size = (d))
#define VSC_UF_AUBO_GetCUBOByteSize(aubo)           ((aubo)->cubo_byte_size)
#define VSC_UF_AUBO_SetCUBOByteSize(aubo, d)        ((aubo)->cubo_byte_size = (d))
#define VSC_UF_AUBO_GetOptions(aubo)                ((aubo)->options)
#define VSC_UF_AUBO_SetOptions(aubo, o)             ((aubo)->options = (o))
#define VSC_UF_AUBO_GetDumper(aubo)                 ((aubo)->all_shaders->dumper)

/* get the maxmium capability of default uniform block */
static gctUINT
_VSC_UF_AUBO_GetCapability(
    IN VIR_ShaderKind kind,
    IN VSC_HW_CONFIG* HwCfg
    )
{
    switch (kind)
    {
    case VIR_SHADER_VERTEX:
        return HwCfg->maxVSConstRegCount;

    case VIR_SHADER_TESSELLATION_CONTROL:
        return HwCfg->maxTCSConstRegCount;

    case VIR_SHADER_TESSELLATION_EVALUATION:
        return HwCfg->maxTESConstRegCount;

    case VIR_SHADER_GEOMETRY:
        return HwCfg->maxGSConstRegCount;

    case VIR_SHADER_FRAGMENT:
    case VIR_SHADER_COMPUTE:
        return HwCfg->maxPSConstRegCount;

    default:
        gcmASSERT(0);
    }

    return 0;
}

static void
_VSC_UF_AUBO_DumpDUBRegCount(
    IN VSC_UF_AUBO* aubo
    )
{
    VIR_Dumper* dumper = VSC_UF_AUBO_GetDumper(aubo);
    gctUINT i;

    VIR_LOG(dumper, "Default Uniform Block Reg Count: ");
    for(i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
    {
        VIR_LOG(dumper, "%d ", VSC_UF_AUBO_GetDUBRegCount(aubo, i));
    }
    VIR_LOG(dumper, "\n");
    VIR_LOG_FLUSH(dumper);
}

static void
_VSC_UF_AUBO_DumpMaxRegCount(
    IN VSC_UF_AUBO* aubo
    )
{
    VIR_Dumper* dumper = VSC_UF_AUBO_GetDumper(aubo);
    gctUINT i;

    VIR_LOG(dumper, "Max Default Uniform Block Reg Count: ");
    for(i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
    {
        VIR_LOG(dumper, "%d ", VSC_UF_AUBO_GetMaxRegCount(aubo, i));
    }
    VIR_LOG(dumper, "\n");
    VIR_LOG_FLUSH(dumper);
}

static void
_VSC_UF_AUBO_DumpDUBOs(
    IN VSC_UF_AUBO* aubo
    )
{
    VIR_Dumper* dumper = VSC_UF_AUBO_GetDumper(aubo);
    gctUINT i;

    VIR_LOG(dumper, "All Default UBOs:\n");
    VIR_LOG_FLUSH(dumper);
    for(i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
    {
        VIR_SymId dubo_id = VSC_UF_AUBO_GetDUBO(aubo, i);
        if(VIR_Id_isValid(dubo_id))
        {
            VIR_Shader* shader = VSC_UF_AUBO_GetShader(aubo, i);
            VIR_Symbol* dubo_sym = VIR_Shader_GetSymFromId(shader, dubo_id);
            VIR_UniformBlock* dubo = VIR_Symbol_GetUBO(dubo_sym);
            VIR_LOG(dumper, "shader(id:%d):\n", VIR_Shader_GetId(shader));
            VIR_LOG_FLUSH(dumper);
            VIR_UniformBlock_Dump(shader->dumper, dubo);
        }
    }
    VIR_LOG(dumper, "\n");
    VIR_LOG_FLUSH(dumper);
}

static void
_VSC_UF_AUBO_DumpDUBOAddrs(
    IN VSC_UF_AUBO* aubo
    )
{
    VIR_Dumper* dumper = VSC_UF_AUBO_GetDumper(aubo);
    gctUINT i;

    VIR_LOG(dumper, "All Default UBO Addresses:\n");
    VIR_LOG_FLUSH(dumper);
    for(i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
    {
        VIR_SymId duboaddr_id = VSC_UF_AUBO_GetDUBOAddr(aubo, i);
        if(VIR_Id_isValid(duboaddr_id))
        {
            VIR_Shader* shader = VSC_UF_AUBO_GetShader(aubo, i);
            VIR_Symbol* duboaddr_sym = VIR_Shader_GetSymFromId(shader, duboaddr_id);
            VIR_Uniform* duboaddr = VIR_Symbol_GetUniform(duboaddr_sym);
            VIR_LOG(dumper, "shader(id:%d):\n", VIR_Shader_GetId(shader));
            VIR_LOG_FLUSH(dumper);
            VIR_Uniform_Dump(shader->dumper, duboaddr);
        }
    }
    VIR_LOG(dumper, "\n");
    VIR_LOG_FLUSH(dumper);
}

static void
_VSC_UF_AUBO_DumpCUBOs(
    IN VSC_UF_AUBO* aubo
    )
{
    VIR_Dumper* dumper = VSC_UF_AUBO_GetDumper(aubo);
    gctUINT i;

    VIR_LOG(dumper, "All Constant UBOs:\n");
    VIR_LOG_FLUSH(dumper);
    for(i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
    {
        VIR_SymId cubo_id = VSC_UF_AUBO_GetCUBO(aubo, i);
        if(VIR_Id_isValid(cubo_id))
        {
            VIR_Shader* shader = VSC_UF_AUBO_GetShader(aubo, i);
            VIR_Symbol* cubo_sym = VIR_Shader_GetSymFromId(shader, cubo_id);
            VIR_UniformBlock* cubo = VIR_Symbol_GetUBO(cubo_sym);
            VIR_LOG(dumper, "shader(id:%d):\n", VIR_Shader_GetId(shader));
            VIR_LOG_FLUSH(dumper);
            VIR_UniformBlock_Dump(shader->dumper, cubo);
        }
    }
    VIR_LOG(dumper, "\n");
    VIR_LOG_FLUSH(dumper);
}

static void
_VSC_UF_AUBO_DumpCUBOAddrs(
    IN VSC_UF_AUBO* aubo
    )
{
    VIR_Dumper* dumper = VSC_UF_AUBO_GetDumper(aubo);
    gctUINT i;

    VIR_LOG(dumper, "All Constant UBO Addresses:\n");
    VIR_LOG_FLUSH(dumper);
    for(i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
    {
        VIR_SymId cuboaddr_id = VSC_UF_AUBO_GetCUBOAddr(aubo, i);
        if(VIR_Id_isValid(cuboaddr_id))
        {
            VIR_Shader* shader = VSC_UF_AUBO_GetShader(aubo, i);
            VIR_Symbol* cuboaddr_sym = VIR_Shader_GetSymFromId(shader, cuboaddr_id);
            VIR_Uniform* cuboaddr = VIR_Symbol_GetUniform(cuboaddr_sym);
            VIR_LOG(dumper, "shader(id:%d):\n", VIR_Shader_GetId(shader));
            VIR_LOG_FLUSH(dumper);
            VIR_Uniform_Dump(shader->dumper, cuboaddr);
        }
    }
    VIR_LOG(dumper, "\n");
    VIR_LOG_FLUSH(dumper);
}

/* initialize struct VSC_UF_AUBO */
static void
_VSC_UF_AUBO_Initialize(
    IN OUT VSC_UF_AUBO* aubo,
    IN VSC_AllShaders* all_shaders,
    IN VSC_OPTN_UF_AUBO_Options* options
    )
{
    gctUINT i;

    memset(aubo, 0, sizeof(VSC_UF_AUBO));
    VSC_UF_AUBO_SetAllShaders(aubo, all_shaders);

    for(i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
    {
        VIR_Shader* shader = VSC_AllShaders_GetShader(all_shaders, i);
        if(shader)
        {
            VSC_UF_AUBO_SetMaxRegCount(aubo, i,
                _VSC_UF_AUBO_GetCapability(VIR_Shader_GetKind(shader), all_shaders->hwCfg));
        }
        else
        {
            VSC_UF_AUBO_SetMaxRegCount(aubo, i, 0);
        }
        VSC_UF_AUBO_SetDUBRegCount(aubo, i, 0);
        VSC_UF_AUBO_SetDUBO(aubo, i, VIR_INVALID_ID);
        VSC_UF_AUBO_SetDUBOAddr(aubo, i, VIR_INVALID_ID);
        VSC_UF_AUBO_SetDUBOItemCount(aubo, i, 0);
        VSC_UF_AUBO_SetCUBO(aubo, i, VIR_INVALID_ID);
        VSC_UF_AUBO_SetCUBOAddr(aubo, i, VIR_INVALID_ID);
        VSC_UF_AUBO_SetCUBOItemCount(aubo, i, 0);
    }
    VSC_UF_AUBO_SetDUBOByteSize(aubo, 0);
    VSC_UF_AUBO_SetCUBOByteSize(aubo, 0);
    VSC_UF_AUBO_SetOptions(aubo, options);

    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBO_Options_GetTrace(options), VSC_OPTN_UF_AUBO_Options_TRACE_INITIALIZE))
    {
        VIR_Dumper* dumper = VSC_UF_AUBO_GetDumper(aubo);
        VIR_LOG(dumper, "DUBO Initialization\n");
        _VSC_UF_AUBO_DumpMaxRegCount(aubo);
        VIR_LOG_FLUSH(dumper);
    }
}

/* finalize struct VSC_UF_AUBO */
static void
_VSC_UF_AUBO_Finalize(
    IN OUT VSC_UF_AUBO* aubo
    )
{
    return;
}

/* construct default UBO, add uniform for default UBO addreess, move uniforms in workset into default UBO */
static VSC_ErrCode
_VSC_UF_AUBO_CollectUniformsInfo(
    IN OUT VSC_UF_AUBO* aubo,
    gctBOOL* has_active
    )
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    VSC_AllShaders* all_shaders = VSC_UF_AUBO_GetAllShaders(aubo);
    VSC_GlobalUniformTable* global_uniform_table = VSC_AllShaders_GetGlobalUniformTable(all_shaders);
    VSC_OPTN_UF_AUBO_Options* options = VSC_UF_AUBO_GetOptions(aubo);
    VIR_Dumper* dumper = VSC_UF_AUBO_GetDumper(aubo);
    gctUINT i;

    gcmASSERT(has_active);
    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBO_Options_GetTrace(options), VSC_OPTN_UF_AUBO_Options_TRACE_MARKACTIVE))
    {
        VIR_LOG(dumper, "Mark active uniforms:\n");
        VIR_LOG_FLUSH(dumper);
    }

    for(i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
    {
        VIR_Shader* shader = VSC_AllShaders_GetShader(all_shaders, i);
        if(shader)
        {
            VIR_FuncIterator func_iter;
            VIR_FunctionNode* func_node;

            VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(shader));
            /* iterate over functions */
            for(func_node = VIR_FuncIterator_First(&func_iter); func_node != gcvNULL;
                func_node = VIR_FuncIterator_Next(&func_iter))
            {
                VIR_Function* func = func_node->function;
                VIR_Instruction* inst;

                /* iterate over instructions */
                for(inst = VIR_Function_GetInstList(func)->pHead;
                    inst != gcvNULL;inst = VIR_Inst_GetNext(inst))
                {
                    gctUINT j;


                    /* iterate over source operands */
                    for(j = 0; j < VIR_Inst_GetSrcNum(inst); j++)
                    {
                        VIR_Operand* src = VIR_Inst_GetSource(inst, j);
                        gcmASSERT(src);

                        if(VIR_Operand_isSymbol(src))
                        {
                            VIR_Symbol* sym = VIR_Operand_GetSymbol(src);
                            if(VIR_Symbol_isUniform(sym))
                            {
                                VSC_GlobalUniformItem* item;

                                VSC_GlobalUniformTable_FindUniformWithShaderUniform(global_uniform_table, shader, VIR_Symbol_GetIndex(sym), &item);
                                gcmASSERT(item);

                                if((VIR_Operand_GetRelAddrMode(src) || (VIR_Inst_GetOpcode(inst) == VIR_OP_LDARR && j == 0)) && VIR_Symbol_HasFlag(sym, VIR_SYMFLAG_IS_FIELD))
                                {
                                    VSC_GlobalUniformItem_SetFlag(item, VIR_SYMUNIFORMFLAG_ALWAYS_IN_DUB);
                                }

                                if(VSC_GlobalUniformItem_HasFlag(item, VIR_SYMFLAG_INACTIVE))
                                {
                                    VSC_GlobalUniformItem_ResetFlag(item, VIR_SYMFLAG_INACTIVE);
                                    /*VIR_Symbol_RemoveFlag(sym, VIR_SYMFLAG_INACTIVE);*/
                                    VSC_GlobalUniformTable_SetHasActiveUniform(global_uniform_table);
                                    *has_active = gcvTRUE;

                                    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBO_Options_GetTrace(options), VSC_OPTN_UF_AUBO_Options_TRACE_MARKACTIVE))
                                    {
                                        VSC_GlobalUniformItem_Dump(item);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return virErrCode;
}

static void
_VSC_UF_AUBO_CollectIndirectlyAccessedUniforms(
    IN OUT VSC_UF_AUBO* aubo
    )
{
    VSC_AllShaders* all_shaders = VSC_UF_AUBO_GetAllShaders(aubo);
    VSC_GlobalUniformTable* global_uniform_table = VSC_AllShaders_GetGlobalUniformTable(all_shaders);
    VSC_OPTN_UF_AUBO_Options* options = VSC_UF_AUBO_GetOptions(aubo);
    VIR_Dumper* dumper = VSC_UF_AUBO_GetDumper(aubo);
    gctUINT i;

    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBO_Options_GetTrace(options), VSC_OPTN_UF_AUBO_Options_TRACE_INDIRECT))
    {
        VIR_LOG(dumper, "Collect indirectly accessed uniforms:\n");
        VIR_LOG_FLUSH(dumper);
    }

    for(i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
    {
        VIR_Shader* shader = VSC_AllShaders_GetShader(all_shaders, i);
        if(shader)
        {
            /*VSC_HASH_TABLE* workset = VSC_UF_AUBO_GetWorkSet(aubo);*/

            VIR_FuncIterator func_iter;
            VIR_FunctionNode* func_node;

            VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(shader));
            /* iterate over functions */
            for(func_node = VIR_FuncIterator_First(&func_iter); func_node != gcvNULL;
                func_node = VIR_FuncIterator_Next(&func_iter))
            {
                VIR_Function* func = func_node->function;
                VIR_Instruction* inst = VIR_Function_GetInstList(func)->pHead;

                if(inst == gcvNULL)
                {
                    continue;
                }

                /* iterate over instructions */
                do
                {
                    gctUINT i;
                    /* iterate over source operands */
                    for(i = 0; i < 3; i++)
                    {
                        VIR_Operand* src = VIR_Inst_GetSource(inst, i);
                        if(src && VIR_Operand_isSymbol(src))
                        {
                            VIR_Symbol* sym = VIR_Operand_GetSymbol(src);
                            if(VIR_Symbol_isUniform(sym) && VIR_Operand_GetIsConstIndexing(src))        /* you wen ti */
                            {
                                if(!VIR_Symbol_HasFlag(sym, VIR_SYMUNIFORMFLAG_MOVED_TO_DUBO))
                                {
                                    VSC_GlobalUniformItem* item;
                                    VSC_GlobalUniformTable_FindUniformWithShaderUniform(global_uniform_table, shader, VIR_Symbol_GetIndex(sym), &item);
                                    gcmASSERT(item);
                                    if(!VSC_GlobalUniformItem_HasFlag(item, VIR_SYMUNIFORMFLAG_ALWAYS_IN_DUB))
                                    {
                                        VSC_GlobalUniformItem_SetFlag(item, VIR_SYMUNIFORMFLAG_MOVING_TO_DUBO);

                                        if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBO_Options_GetTrace(options), VSC_OPTN_UF_AUBO_Options_TRACE_INDIRECT))
                                        {
                                            VSC_GlobalUniformItem_Dump(item);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    inst = VIR_Inst_GetNext(inst);
                }
                while(inst && inst != VIR_Function_GetInstList(func)->pTail);
            }
        }
    }
}

/* calculate the size of default uniform block */
static void
_VSC_UF_AUBO_CalculateDUBRegCount(
    IN OUT VSC_UF_AUBO* aubo
    )
{
    VSC_AllShaders* all_shaders = VSC_UF_AUBO_GetAllShaders(aubo);
    VSC_OPTN_UF_AUBO_Options* options = VSC_UF_AUBO_GetOptions(aubo);

    VSC_GlobalUniformTable* global_uniform_table = VSC_AllShaders_GetGlobalUniformTable(all_shaders);
    VSC_GlobalUniformTable_Iterator iter;
    VSC_GlobalUniformItem* item;

    VSC_GlobalUniformTable_Iterator_Init(&iter, global_uniform_table);

    for(item = VSC_GlobalUniformTable_Iterator_First(&iter); item != gcvNULL;
        item = VSC_GlobalUniformTable_Iterator_Next(&iter))
    {
        gctUINT i;
        for(i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
        {
            if(VSC_GlobalUniformItem_IsInShader(item, i) && !VSC_GlobalUniformItem_HasFlag(item, VIR_SYMFLAG_INACTIVE))
            {
                VSC_UF_AUBO_IncDUBRegCount(aubo, i, VSC_GlobalUniformItem_GetRegCount(item));
            }
        }
    }

    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBO_Options_GetTrace(options), VSC_OPTN_UF_AUBO_Options_TRACE_DUBSIZE))
    {
        _VSC_UF_AUBO_DumpDUBRegCount(aubo);
    }
}

static gctBOOL
_VSC_UF_AUBO_DUBIsAffordable(
    IN OUT VSC_UF_AUBO* aubo,
    OUT gctUINT* fattest_shader
    )
{
    gctUINT i;
    VSC_OPTN_UF_AUBO_Options* options = VSC_UF_AUBO_GetOptions(aubo);
    gctUINT fattest = 0;
    gctUINT max_gap = 0;
    gctUINT totalRegCount = 0;
    gctUINT maxRegShaderIdx = 0;
    gctBOOL result = gcvTRUE;

    for(i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
    {
        totalRegCount += VSC_UF_AUBO_GetDUBRegCount(aubo, i);
        if(i != 0 && (VSC_UF_AUBO_GetDUBRegCount(aubo, i) > (VSC_UF_AUBO_GetDUBRegCount(aubo, i - 1))))
        {
            maxRegShaderIdx = i;
        }

        if(VSC_UF_AUBO_GetDUBRegCount(aubo, i) > (VSC_UF_AUBO_GetMaxRegCount(aubo, i) - VSC_OPTN_UF_AUBO_Options_GetConstRegReservation(options)))
        {
            result = gcvFALSE;
            if(VSC_UF_AUBO_GetDUBRegCount(aubo, i) - (VSC_UF_AUBO_GetMaxRegCount(aubo, i) - VSC_OPTN_UF_AUBO_Options_GetConstRegReservation(options))> max_gap)
            {
                max_gap = VSC_UF_AUBO_GetDUBRegCount(aubo, i) - VSC_UF_AUBO_GetMaxRegCount(aubo, i);
                fattest = i;
            }
        }
    }

    if(!result && fattest_shader)
    {
        *fattest_shader = fattest;
    }

    if(result &&
       aubo->all_shaders->hwCfg->hwFeatureFlags.constRegFileUnified &&
       totalRegCount > aubo->all_shaders->hwCfg->maxTotalConstRegCount)
    {
        result = gcvFALSE;
        if (fattest_shader)
        {
           *fattest_shader = maxRegShaderIdx;
        }
    }

    return result;
}

static void
_VSC_UF_AUBO_DecreaseDUB(
    IN OUT VSC_UF_AUBO* aubo,
    IN VSC_GlobalUniformItem* item
    )
{
    gctUINT i;

    for(i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
    {
        if(VSC_GlobalUniformItem_IsInShader(item, i))
        {
            VSC_UF_AUBO_SetDUBRegCount(aubo, i, VSC_UF_AUBO_GetDUBRegCount(aubo, i) - VSC_GlobalUniformItem_GetRegCount(item));
        }
    }
}

static void
_VSC_UF_AUBO_UpdateDUBOItemCount(
    IN OUT VSC_UF_AUBO* aubo,
    IN VSC_GlobalUniformItem* item
    )
{
    gctUINT i;
    for(i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
    {
        if(VSC_GlobalUniformItem_IsInShader(item, i))
        {
            VSC_UF_AUBO_IncDUBOItemCount(aubo, i);
        }
    }
}

static void
_VSC_UF_AUBO_UpdateCUBOItemCount(
    IN OUT VSC_UF_AUBO* aubo,
    IN VSC_GlobalUniformItem* item
    )
{
    gctUINT i;
    for(i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
    {
        if(VSC_GlobalUniformItem_IsInShader(item, i))
        {
            VSC_UF_AUBO_IncCUBOItemCount(aubo, i);
        }
    }
}

static void
_VSC_UF_AUBO_PickItem(
    IN OUT VSC_UF_AUBO* aubo,
    IN VSC_GlobalUniformItem* item,
    IN gctUINT* dubo_byte_size,
    IN gctUINT* cubo_byte_size
    )
{
    VSC_OPTN_UF_AUBO_Options* options = VSC_UF_AUBO_GetOptions(aubo);

    gcmASSERT(dubo_byte_size);

    _VSC_UF_AUBO_DecreaseDUB(aubo, item);
    if(VSC_GlobalUniformItem_HasFlag(item, VIR_SYMUNIFORMFLAG_COMPILETIME_INITIALIZED))
    {
        VSC_GlobalUniformItem_SetInCUBO(item);
        VSC_GlobalUniformItem_SetOffsetByAll(item, *cubo_byte_size);
        *cubo_byte_size += VSC_GlobalUniformItem_GetByteSize(item);
        _VSC_UF_AUBO_UpdateCUBOItemCount(aubo, item);
    }
    else
    {
        VSC_GlobalUniformItem_SetInDUBO(item);
        VSC_GlobalUniformItem_SetOffsetByAll(item, *dubo_byte_size);
        *dubo_byte_size += VSC_GlobalUniformItem_GetByteSize(item);
        _VSC_UF_AUBO_UpdateDUBOItemCount(aubo, item);
    }
    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBO_Options_GetTrace(options), VSC_OPTN_UF_AUBO_Options_TRACE_PICK))
    {
        VSC_GlobalUniformItem_Dump(item);
    }
}

/* pick uniforms from default uniform block to default UBO */
static void
_VSC_UF_AUBO_PickUniforms(
    IN OUT VSC_UF_AUBO* aubo
    )
{
    VSC_AllShaders* all_shaders = VSC_UF_AUBO_GetAllShaders(aubo);
    VSC_OPTN_UF_AUBO_Options* options = VSC_UF_AUBO_GetOptions(aubo);
    VIR_Dumper* dumper = VSC_UF_AUBO_GetDumper(aubo);
    gctUINT dubo_byte_size = 0, cubo_byte_size = 0;
    VSC_GlobalUniformTable* global_uniform_table = VSC_AllShaders_GetGlobalUniformTable(all_shaders);
    VSC_GlobalUniformTable_Iterator iter;
    VSC_GlobalUniformItem* item;
    gctBOOL usbDUBO = VSC_UTILS_MASK(VSC_OPTN_UF_AUBO_Options_GetHeuristics(options), VSC_OPTN_UF_AUBO_Options_HEUR_USE_DUBO);
    gctBOOL usbCUBO = VSC_UTILS_MASK(VSC_OPTN_UF_AUBO_Options_GetHeuristics(options), VSC_OPTN_UF_AUBO_Options_HEUR_USE_CUBO);

    VSC_GlobalUniformTable_Iterator_Init(&iter, global_uniform_table);

    /* pick those pre-decided uniform items */
    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBO_Options_GetTrace(options), VSC_OPTN_UF_AUBO_Options_TRACE_PICK))
    {
        VIR_LOG(dumper, "Picked pre-decided uniforms:\n");
        VIR_LOG_FLUSH(dumper);
    }
    for(item = VSC_GlobalUniformTable_Iterator_First(&iter); item != gcvNULL;
        item = VSC_GlobalUniformTable_Iterator_Next(&iter))
    {
        if(VSC_GlobalUniformItem_HasFlag(item, VIR_SYMUNIFORMFLAG_MOVING_TO_DUBO))
        {
            gcmASSERT(VSC_GlobalUniformItem_SuitableForPickingIntoAuxUBO(item, usbDUBO, usbCUBO));
            _VSC_UF_AUBO_PickItem(aubo, item, &dubo_byte_size, &cubo_byte_size);
        }
    }

    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBO_Options_GetTrace(options), VSC_OPTN_UF_AUBO_Options_TRACE_PICK))
    {
        VIR_LOG(dumper, "Pick uniforms with heuristic %x:\n", VSC_OPTN_UF_AUBO_Options_GetHeuristics(options));
        VIR_LOG_FLUSH(dumper);
    }
    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBO_Options_GetHeuristics(options), VSC_OPTN_UF_AUBO_Options_HEUR_FORCE_ALL))
    {
        VSC_GlobalUniformTable_Iterator_Init(&iter, global_uniform_table);

        for(item = VSC_GlobalUniformTable_Iterator_First(&iter); item != gcvNULL;
            item = VSC_GlobalUniformTable_Iterator_Next(&iter))
        {
            if(VSC_GlobalUniformItem_SuitableForPickingIntoAuxUBO(item, usbDUBO, usbCUBO))
            {
                _VSC_UF_AUBO_PickItem(aubo, item, &dubo_byte_size, &cubo_byte_size);
            }
        }
    }
    else
    {
        gctUINT fattest_shader = VSC_MAX_LINKABLE_SHADER_STAGE_COUNT;
        while(!_VSC_UF_AUBO_DUBIsAffordable(aubo, &fattest_shader))
        {
            if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBO_Options_GetHeuristics(options), VSC_OPTN_UF_AUBO_Options_HEUR_ORDERLY))
            {
                VSC_GlobalUniformTable_Iterator_Init(&iter, global_uniform_table);

                for(item = VSC_GlobalUniformTable_Iterator_First(&iter); item != gcvNULL;
                    item = VSC_GlobalUniformTable_Iterator_Next(&iter))
                {
                    if(VSC_GlobalUniformItem_IsInShader(item, fattest_shader) && VSC_GlobalUniformItem_SuitableForPickingIntoAuxUBO(item, usbDUBO, usbCUBO))
                    {
                        _VSC_UF_AUBO_PickItem(aubo, item, &dubo_byte_size, &cubo_byte_size);
                        break;
                    }
                }
                if(item == gcvNULL)
                {
                    /* no more item could be put into DUBO ??!! */
                    break;
                }
            }
        }
    }

    VSC_UF_AUBO_SetDUBOByteSize(aubo, dubo_byte_size);
    VSC_UF_AUBO_SetCUBOByteSize(aubo, cubo_byte_size);

    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBO_Options_GetTrace(options), VSC_OPTN_UF_AUBO_Options_TRACE_PICK))
    {
        VIR_LOG(dumper, "DUBO byte size: %d\n", VSC_UF_AUBO_GetDUBOByteSize(aubo));
        VIR_LOG(dumper, "CUBO byte size: %d\n", VSC_UF_AUBO_GetCUBOByteSize(aubo));
        VIR_LOG_FLUSH(dumper);
    }
}

/* construct default UBO and add uniform for default UBO addreess */
static VSC_ErrCode
_VSC_UF_AUBO_ConstructDefaultUBO(
    IN OUT VSC_UF_AUBO* aubo
    )
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    VSC_AllShaders* all_shaders = VSC_UF_AUBO_GetAllShaders(aubo);
    VSC_OPTN_UF_AUBO_Options* options = VSC_UF_AUBO_GetOptions(aubo);
    gctUINT i;

    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBO_Options_GetTrace(options), VSC_OPTN_UF_AUBO_Options_TRACE_CONSTRUCTION))
    {
        VIR_Dumper* dumper = VSC_UF_AUBO_GetDumper(aubo);
        VIR_LOG(dumper, "Construct Default UBOs:\n");
        VIR_LOG_FLUSH(dumper);
    }

    for(i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
    {
        gctUINT count = VSC_UF_AUBO_GetDUBOItemCount(aubo, i);
        if(count > 0)
        {
            VIR_Shader* shader = VSC_AllShaders_GetShader(all_shaders, i);
            VIR_Symbol* dubo_sym = gcvNULL;
            VIR_UniformBlock* dubo_ub;
            VIR_Symbol* dubo_addr_sym = gcvNULL;

            VIR_Shader_GetDUBO(shader, &dubo_sym, &dubo_addr_sym);
            dubo_ub = VIR_Symbol_GetUBO(dubo_sym);
            VSC_UF_AUBO_SetDUBO(aubo, i, VIR_Symbol_GetIndex(dubo_sym));
            dubo_ub->blockSize = VSC_UF_AUBO_GetDUBOByteSize(aubo);
            dubo_ub->uniforms = (VIR_Uniform **)vscMM_Alloc(&shader->mempool, sizeof(VIR_Uniform*) * count);
            VIR_Symbol_SetFlag(dubo_addr_sym, VIR_SYMUNIFORMFLAG_USED_IN_SHADER);
            VSC_UF_AUBO_SetDUBOAddr(aubo, i, VIR_Symbol_GetIndex(dubo_addr_sym));
        }
    }

    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBO_Options_GetTrace(options), VSC_OPTN_UF_AUBO_Options_TRACE_CONSTRUCTION))
    {
        _VSC_UF_AUBO_DumpDUBOs(aubo);
        _VSC_UF_AUBO_DumpDUBOAddrs(aubo);
    }
    return virErrCode;
}

/* construct default UBO and add uniform for default UBO addreess */
static VSC_ErrCode
_VSC_UF_AUBO_ConstructConstantUBO(
    IN OUT VSC_UF_AUBO* aubo
    )
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    VSC_AllShaders* all_shaders = VSC_UF_AUBO_GetAllShaders(aubo);
    VSC_OPTN_UF_AUBO_Options* options = VSC_UF_AUBO_GetOptions(aubo);
    gctUINT i;

    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBO_Options_GetTrace(options), VSC_OPTN_UF_AUBO_Options_TRACE_CONSTRUCTION))
    {
        VIR_Dumper* dumper = VSC_UF_AUBO_GetDumper(aubo);
        VIR_LOG(dumper, "Construct Constant UBOs:\n");
        VIR_LOG_FLUSH(dumper);
    }

    for(i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
    {
        gctUINT count = VSC_UF_AUBO_GetCUBOItemCount(aubo, i);
        if(count > 0)
        {
            VIR_Shader* shader = VSC_AllShaders_GetShader(all_shaders, i);
            /* varables used for creating default ubo */
            VIR_NameId cubo_nameId;
            VIR_TypeId cubo_typeId;
            VIR_SymId cubo_symId;
            VIR_Symbol* cubo_sym;
            VIR_UniformBlock* cubo_ub;
            /* varables used for creating default ubo address */
            VIR_NameId cubo_addr_nameId;
            VIR_SymId cubo_addr_symId;
            VIR_Symbol* cubo_addr_sym;
            VIR_Uniform* cubo_addr_uniform;

            /* create default ubo */
            {
                /* default ubo name */
                virErrCode = VIR_Shader_AddString(shader,
                                                    "#ConstantUBO",
                                                    &cubo_nameId);
                if(virErrCode != VSC_ERR_NONE) return virErrCode;

                /* default ubo type */
                virErrCode = VIR_Shader_AddStructType(shader,
                                                        gcvFALSE,
                                                        cubo_nameId,
                                                        &cubo_typeId);
                if(virErrCode != VSC_ERR_NONE) return virErrCode;

                /* default ubo symbol */
                virErrCode = VIR_Shader_AddSymbol(shader,
                                                    VIR_SYM_UBO,
                                                    cubo_nameId,
                                                    VIR_Shader_GetTypeFromId(shader, cubo_typeId),
                                                    VIR_STORAGE_UNKNOWN,
                                                    &cubo_symId);
                if(virErrCode != VSC_ERR_NONE) return virErrCode;

                cubo_sym = VIR_Shader_GetSymFromId(shader, cubo_symId);
                VIR_Symbol_SetPrecision(cubo_sym, VIR_PRECISION_DEFAULT);
                VIR_Symbol_SetAddrSpace(cubo_sym, VIR_AS_CONSTANT);
                VIR_Symbol_SetTyQualifier(cubo_sym, VIR_TYQUAL_CONST);
                VIR_Symbol_SetLayoutQualifier(cubo_sym, VIR_LAYQUAL_PACKED);
                VIR_Symbol_SetFlag(cubo_sym, VIR_SYMFLAG_COMPILER_GEN);

                cubo_ub = VIR_Symbol_GetUBO(cubo_sym);
                cubo_ub->blockIndex = (gctINT16)VIR_IdList_Count(VIR_Shader_GetUniformBlocks(shader)) - 1;
                VIR_Shader_SetConstantUBOIndex(shader, cubo_ub->blockIndex);

                VSC_UF_AUBO_SetCUBO(aubo, i, cubo_symId);
                cubo_ub->blockSize = VSC_UF_AUBO_GetCUBOByteSize(aubo);
                cubo_ub->uniforms = (VIR_Uniform **)vscMM_Alloc(&shader->mempool, sizeof(VIR_Uniform*) * count);
            }

            /* create default ubo address */
            {
                virErrCode = VIR_Shader_AddString(shader,
                                                    "#ConstantUBO",
                                                    &cubo_addr_nameId);
                if(virErrCode != VSC_ERR_NONE) return virErrCode;

                /* default ubo symbol */
                virErrCode = VIR_Shader_AddSymbol(shader,
                                                    VIR_SYM_UNIFORM,
                                                    cubo_addr_nameId,
                                                    VIR_Shader_GetTypeFromId(shader, VIR_TYPE_UINT32),
                                                    VIR_STORAGE_UNKNOWN,
                                                    &cubo_addr_symId);

                cubo_addr_sym = VIR_Shader_GetSymFromId(shader, cubo_addr_symId);
                VIR_Symbol_SetUniformKind(cubo_addr_sym, VIR_UNIFORM_UNIFORM_BLOCK_ADDRESS);
                VIR_Symbol_SetPrecision(cubo_addr_sym, VIR_PRECISION_HIGH);
                VIR_Symbol_SetFlag(cubo_addr_sym, VIR_SYMUNIFORMFLAG_USED_IN_SHADER);
                VIR_Symbol_SetFlag(cubo_addr_sym, VIR_SYMFLAG_COMPILER_GEN);

                cubo_addr_uniform = VIR_Symbol_GetUniform(cubo_addr_sym);
                cubo_addr_uniform->index = shader->uniformCount - 1;
                cubo_addr_uniform->blockIndex = cubo_ub->blockIndex;

                VSC_UF_AUBO_SetCUBOAddr(aubo, i, cubo_addr_symId);
            }

            cubo_ub->baseAddr = cubo_addr_symId;
        }
    }

    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBO_Options_GetTrace(options), VSC_OPTN_UF_AUBO_Options_TRACE_CONSTRUCTION))
    {
        _VSC_UF_AUBO_DumpCUBOs(aubo);
        _VSC_UF_AUBO_DumpCUBOAddrs(aubo);
    }
    return virErrCode;
}

/* construct default UBO, add uniform for default UBO addreess, move uniforms in workset into default UBO */
static VSC_ErrCode
_VSC_UF_AUBO_FillAuxiliaryUBOs(
    IN OUT VSC_UF_AUBO* aubo
    )
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    VSC_AllShaders* all_shaders = VSC_UF_AUBO_GetAllShaders(aubo);
    VSC_OPTN_UF_AUBO_Options* options = VSC_UF_AUBO_GetOptions(aubo);
    /*VIR_Dumper* dumper = VSC_UF_AUBO_GetDumper(aubo);*/
    VSC_GlobalUniformTable* global_uniform_table = VSC_AllShaders_GetGlobalUniformTable(all_shaders);
    VSC_GlobalUniformTable_Iterator iter;
    VSC_GlobalUniformItem* item;

    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBO_Options_GetTrace(options), VSC_OPTN_UF_AUBO_Options_TRACE_FILL))
    {
        VIR_Dumper* dumper = VSC_UF_AUBO_GetDumper(aubo);
        VIR_LOG(dumper, "Fill Auxiliary UBOs:\n");
        VIR_LOG_FLUSH(dumper);
    }

    VSC_GlobalUniformTable_Iterator_Init(&iter, global_uniform_table);

    for(item = VSC_GlobalUniformTable_Iterator_First(&iter); item != gcvNULL;
        item = VSC_GlobalUniformTable_Iterator_Next(&iter))
    {
        if(VSC_GlobalUniformItem_HasFlag(item, VIR_SYMUNIFORMFLAG_MOVED_TO_DUBO) ||
           VSC_GlobalUniformItem_HasFlag(item, VIR_SYMUNIFORMFLAG_MOVED_TO_CUBO))
        {
            gctUINT i;
            for(i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
            {
                VIR_SymId uniform_symid = VSC_GlobalUniformItem_GetUniform(item, i);
                if(VIR_Id_isValid(uniform_symid))
                {
                    VIR_Shader* shader = VSC_GlobalUniformItem_GetShader(item, i);
                    VIR_Symbol* sym = VIR_Shader_GetSymFromId(shader, uniform_symid);
                    VIR_Uniform* uniform = VIR_Symbol_GetUniform(sym);

                    if(VSC_GlobalUniformItem_HasFlag(item, VIR_SYMUNIFORMFLAG_MOVED_TO_CUBO))
                    {
                        VIR_SymId cubo_ub_id = VSC_UF_AUBO_GetCUBO(aubo, i);
                        VIR_Symbol* cubo_ub_sym = VIR_Shader_GetSymFromId(shader, cubo_ub_id);
                        VIR_UniformBlock* cubo_ub = VIR_Symbol_GetUBO(cubo_ub_sym);

                        uniform->blockIndex = cubo_ub->blockIndex;
                        cubo_ub->uniforms[cubo_ub->uniformCount++] = uniform;
                    }
                    else
                    {
                        VIR_SymId dubo_ub_id;
                        VIR_Symbol* dubo_ub_sym;
                        VIR_UniformBlock* dubo_ub;

                        gcmASSERT(VSC_GlobalUniformItem_HasFlag(item, VIR_SYMUNIFORMFLAG_MOVED_TO_DUBO));

                        dubo_ub_id = VSC_UF_AUBO_GetDUBO(aubo, i);
                        dubo_ub_sym = VIR_Shader_GetSymFromId(shader, dubo_ub_id);
                        dubo_ub = VIR_Symbol_GetUBO(dubo_ub_sym);
                        uniform->blockIndex = dubo_ub->blockIndex;
                        dubo_ub->uniforms[dubo_ub->uniformCount++] = uniform;
                    }
                }
            }
        }
    }

    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBO_Options_GetTrace(options), VSC_OPTN_UF_AUBO_Options_TRACE_FILL))
    {
        _VSC_UF_AUBO_DumpDUBOs(aubo);
        _VSC_UF_AUBO_DumpCUBOs(aubo);
    }
    return virErrCode;
}

static VIR_TypeId
_VSC_UF_AUBO_GetUniformDataTypeID(
    IN VIR_Shader* shader,
    IN VIR_Operand* src
    )
{
    VIR_Symbol* sym;
    VIR_Type* sym_type;
    VIR_TypeId result = VIR_TYPE_UNKNOWN;
    gctBOOL base_find = gcvFALSE;

    sym = VIR_Operand_GetSymbol(src);
    gcmASSERT(VIR_Symbol_isUniform(sym));

    sym_type = VIR_Symbol_GetType(sym);

    while(!base_find)
    {
        switch(VIR_Type_GetKind(sym_type))
        {
            case VIR_TY_ARRAY:
            {
                VIR_TypeId base_type_id = VIR_Type_GetBaseTypeId(sym_type);
                sym_type = VIR_Shader_GetTypeFromId(shader, base_type_id);
                break;
            }
            case VIR_TY_STRUCT:
            {
                gcmASSERT(0);
                break;
            }
            default:
                base_find = gcvTRUE;
        }
    }

    switch(VIR_Type_GetKind(sym_type))
    {
        case VIR_TY_SCALAR:
        case VIR_TY_VECTOR:
            result = VIR_Type_GetIndex(sym_type);
            break;
        case VIR_TY_MATRIX:
            result = VIR_GetTypeRowType(VIR_Type_GetIndex(sym_type));
            break;
        default:
            gcmASSERT(0);
    }

    return result;
}

static gctUINT
_VSC_UF_AUBO_GetArrayStride(
    IN VIR_Type* type
    )
{
    VIR_TypeId base_type_id = VIR_TYPE_UNKNOWN;

    gcmASSERT(VIR_Type_isArray(type) || VIR_Type_isMatrix(type));

    if(VIR_Type_isArray(type))
    {
        base_type_id = VIR_Type_GetBaseTypeId(type);
    }
    if(VIR_Type_isMatrix(type))
    {
        base_type_id = VIR_GetTypeRowType(VIR_Type_GetIndex(type));
    }
    if(VIR_TypeId_isPrimitive(base_type_id))
    {
        return VIR_GetTypeSize(base_type_id);
    }
    else
    {
        gcmASSERT(0);
    }

    return 0;
}

static VSC_ErrCode
_VSC_UF_AUBO_TransformLdarrInstruction(
    IN OUT VSC_UF_AUBO* aubo,
    IN OUT VIR_Shader* shader,
    IN OUT VIR_Function* func,
    IN OUT VIR_Instruction* inst,
    IN VIR_Instruction* insert_before,
    IN OUT VIR_Operand* src
    )
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    gctUINT shader_kind_id = VIR_ShaderKind_Map2KindId(VIR_Shader_GetKind(shader));
    VIR_Instruction *mad_inst, *load_inst;
    VIR_Operand* src0 = VIR_Inst_GetSource(inst, 0);
    VIR_Operand* indexing_src = VIR_Inst_GetSource(inst, 1);
    VIR_Symbol* uniform_sym = VIR_Operand_GetSymbol(src);
    VIR_Uniform* uniform = VIR_Symbol_GetUniform(uniform_sym);
    VIR_Type* src_sym_type = VIR_Symbol_GetType(VIR_Operand_GetSymbol(src));
    VIR_TypeId uniform_data_type_id = _VSC_UF_AUBO_GetUniformDataTypeID(shader, src);
    VIR_TypeId base_typeid = VIR_Type_GetBaseTypeId(src_sym_type);
    VIR_Operand *mad_dest, *mad_src0, *mad_src1, *mad_src2, *load_dest, *load_src0, *load_src1;
    VIR_VirRegId mad_regid, load_regid;
    VIR_SymId mad_symid, load_symid;
    VIR_Symbol* mad_sym;
    gctUINT const_offset = 0;
    VSC_OPTN_UF_AUBO_Options* options = VSC_UF_AUBO_GetOptions(aubo);
    VIR_Dumper* dumper = VSC_UF_AUBO_GetDumper(aubo);
    VIR_Type* base_type = VIR_Shader_GetTypeFromId(shader, base_typeid);

    if (VIR_Inst_GetOpcode(inst) == VIR_OP_LDARR)
    {
        VIR_Instruction *prevInst = VIR_Inst_GetPrev(insert_before);
        if (prevInst &&
            VIR_Inst_GetOpcode(prevInst) == VIR_OP_MOVA)
        {
            VIR_Symbol* movaSym = VIR_Operand_GetSymbol(VIR_Inst_GetDest(prevInst));

            VIR_Inst_SetOpcode(prevInst, VIR_OP_MOV);
            VIR_Symbol_SetStorageClass(movaSym, VIR_STORAGE_UNKNOWN);
            indexing_src = VIR_Inst_GetSource(prevInst, 0);
        }
    }

    /* add a MAD instruction to compute the offset */
    virErrCode = VIR_Function_AddInstructionBefore(func, VIR_OP_MAD, VIR_TYPE_UINT32, insert_before, &mad_inst);
    if(virErrCode != VSC_ERR_NONE) return virErrCode;

    /* Get operands from this MAD instruction */
    mad_dest = VIR_Inst_GetDest(mad_inst);
    mad_src0 = VIR_Inst_GetSource(mad_inst, 0);
    mad_src1 = VIR_Inst_GetSource(mad_inst, 1);
    mad_src2 = VIR_Inst_GetSource(mad_inst, 2);

    if(VIR_Inst_GetOpcode(inst) == VIR_OP_LDARR)
    {
            /* set mad_src0 to src1 */
            VIR_Operand_Copy(mad_src0, indexing_src);
    }
    else if(VIR_Operand_GetRelAddrMode(src))
    {
        VIR_SymId index_symid = VIR_Operand_GetRelIndexing(src);
        VIR_Operand_SetSymbol(mad_src0, func, index_symid);
    }
    else
    {
        gcmASSERT(0);
    }

    /* allocate a new vir_reg which performs as the dest of MAD */
    mad_regid = VIR_Shader_NewVirRegId(shader, 1);
    virErrCode = VIR_Shader_AddSymbol(shader,
                                      VIR_SYM_VIRREG,
                                      mad_regid,
                                      VIR_Shader_GetTypeFromId(shader, VIR_TYPE_UINT32),
                                      VIR_STORAGE_UNKNOWN,
                                      &mad_symid);
    if(virErrCode != VSC_ERR_NONE) return virErrCode;

    mad_sym = VIR_Shader_GetSymFromId(shader, mad_symid);
    VIR_Symbol_SetPrecision(mad_sym, VIR_PRECISION_HIGH);
    VIR_Operand_SetTempRegister(mad_dest, func, mad_symid, VIR_TYPE_UINT32);
    VIR_Operand_SetLvalue(mad_dest, gcvTRUE);
    VIR_Operand_SetEnable(mad_dest, VIR_ENABLE_X);

    /* set mad_src1 to the array stride */
    if (VIR_Type_isMatrix(base_type) && VIR_Type_isArray(src_sym_type))
    {
        VIR_Operand_SetImmediateUint(mad_src1, _VSC_UF_AUBO_GetArrayStride(VIR_Symbol_GetType(uniform_sym)) / VIR_GetTypeRows(VIR_Type_GetIndex(base_type)));
    }
    else
    {
        VIR_Operand_SetImmediateUint(mad_src1, _VSC_UF_AUBO_GetArrayStride(VIR_Symbol_GetType(uniform_sym)));
    }

    /* set mad_src2 to the offset of uniform */
    if(isSymUniformMovedToCUBO(uniform_sym))
    {
        VIR_SymId cubo_addr_symid = VSC_UF_AUBO_GetCUBOAddr(aubo, shader_kind_id);
        VIR_Symbol* cubo_addr_sym = VIR_Shader_GetSymFromId(shader, cubo_addr_symid);
        VIR_Operand_SetUniform(mad_src2, VIR_Symbol_GetUniform(cubo_addr_sym), shader);
    }
    else
    {
        VIR_SymId dubo_addr_symid;
        VIR_Symbol* dubo_addr_sym;

        gcmASSERT(isSymUniformMovedToDUBO(uniform_sym));

        dubo_addr_symid = VSC_UF_AUBO_GetDUBOAddr(aubo, shader_kind_id);
        dubo_addr_sym = VIR_Shader_GetSymFromId(shader, dubo_addr_symid);

        VIR_Operand_SetUniform(mad_src2, VIR_Symbol_GetUniform(dubo_addr_sym), shader);
    }
    VIR_Operand_SetSwizzle(mad_src2, VIR_SWIZZLE_X);

    /* update for dual16 */
    if(VIR_Shader_isDual16Mode(shader))
    {
        gctBOOL needRunSingleT = gcvFALSE;
        gctBOOL dual16NotSupported = gcvFALSE;
        VIR_Inst_Check4Dual16(mad_inst, &needRunSingleT, &dual16NotSupported, gcvNULL, gcvNULL);
        if (needRunSingleT)
        {
            VIR_Inst_SetThreadMode(mad_inst, VIR_THREAD_D16_DUAL_32);
        }
    }

    /* add a LOAD instruction to load the uniform data from Default UBO */
    virErrCode = VIR_Function_AddInstructionBefore(func, VIR_OP_LOAD, VIR_TYPE_UINT32, insert_before, &load_inst);
    if(virErrCode != VSC_ERR_NONE) return virErrCode;

    /* allocate operands for this LOAD instruction */
    virErrCode = VIR_Function_NewOperand(func, &load_dest);
    if(virErrCode != VSC_ERR_NONE) return virErrCode;
    virErrCode = VIR_Function_NewOperand(func, &load_src0);
    if(virErrCode != VSC_ERR_NONE) return virErrCode;
    virErrCode = VIR_Function_NewOperand(func, &load_src1);
    if(virErrCode != VSC_ERR_NONE) return virErrCode;
    VIR_Inst_SetDest(load_inst, load_dest);
    VIR_Inst_SetSource(load_inst, 0, load_src0);
    VIR_Inst_SetSource(load_inst, 1, load_src1);

    /* allocate a new vir_reg which performs as the dest of LOAD */
    load_regid = VIR_Shader_NewVirRegId(shader, 1);
    virErrCode = VIR_Shader_AddSymbol(shader,
                                      VIR_SYM_VIRREG,
                                      load_regid,
                                      VIR_Shader_GetTypeFromId(shader, uniform_data_type_id),
                                      VIR_STORAGE_UNKNOWN,
                                      &load_symid);
    if(virErrCode != VSC_ERR_NONE) return virErrCode;

    VIR_Operand_SetTempRegister(load_dest, func, load_symid, uniform_data_type_id);
    VIR_Operand_SetLvalue(load_dest, gcvTRUE);
    VIR_Operand_SetEnable(load_dest, VIR_Type_Conv2Enable(VIR_Shader_GetTypeFromId(shader, uniform_data_type_id)));

    /* set src0 of load to dubo_addr */
    VIR_Operand_SetTempRegister(load_src0, func, mad_symid, VIR_TYPE_UINT32);
    VIR_Operand_SetSwizzle(load_src0, VIR_SWIZZLE_X);

    /* set src1 of load to mad_symid */
    if(VIR_Operand_GetIsConstIndexing(src))
    {
        gctUINT stride;

        if(VIR_Type_isMatrix(base_type))
        {
            VIR_TypeId row_typeid = VIR_GetTypeRowType(VIR_Type_GetIndex(base_type));
            VIR_Type* row_type = VIR_Shader_GetTypeFromId(shader, row_typeid);
            stride = VIR_Shader_GetTypeByteSize(shader, row_type);
        }
        else
        {
            stride = _VSC_UF_AUBO_GetArrayStride(VIR_Symbol_GetType(uniform_sym));
        }
        const_offset = stride * VIR_Operand_GetMatrixConstIndex(src);
    }
    VIR_Operand_SetImmediateUint(load_src1, VIR_Uniform_GetOffset(uniform) + const_offset);

    /* update for dual16 */
    if(VIR_Shader_isDual16Mode(shader))
    {
        gctBOOL needRunSingleT = gcvFALSE;
        gctBOOL dual16NotSupported = gcvFALSE;
        VIR_Inst_Check4Dual16(load_inst, &needRunSingleT, &dual16NotSupported, gcvNULL, gcvNULL);
        if (needRunSingleT)
        {
            VIR_Inst_SetThreadMode(load_inst, VIR_THREAD_D16_DUAL_32);
        }
    }

    if(VIR_Inst_GetOpcode(inst) == VIR_OP_LDARR)
    {
        /* set opcode of inst from ldarr to mov */
        VIR_Inst_SetOpcode(inst, VIR_OP_MOV);
        VIR_Inst_SetSrcNum(inst, 1);

        /* set src0 of inst to load_symid */
        VIR_Operand_SetTempRegister(src0, func, load_symid, uniform_data_type_id);
        /* clear the indexing */
        VIR_Operand_SetIsConstIndexing(src0, 0);
        VIR_Operand_SetMatrixConstIndex(src0, 0);

        /* set src1 of inst to NULL */
        VIR_Inst_SetSource(inst, 1, gcvNULL);
    }
    else if(VIR_Operand_GetRelAddrMode(src))
    {
        VIR_Operand_SetTempRegister(src, func, load_symid, VIR_Operand_GetType(src));
        VIR_Operand_SetMatrixConstIndex(src, 0);
        VIR_Operand_SetRelIndexing(src, 0);
        VIR_Operand_SetRelAddrMode(src, 0);
    }
    else
    {
        gcmASSERT(0);
    }

    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBO_Options_GetTrace(options), VSC_OPTN_UF_AUBO_Options_TRACE_TRANSFORM))
    {
        VIR_LOG(dumper, "MAD instruction:\n");
        VIR_LOG_FLUSH(dumper);
        VIR_Inst_Dump(shader->dumper, mad_inst);
        VIR_LOG(dumper, "LOAD instruction:\n");
        VIR_LOG_FLUSH(dumper);
        VIR_Inst_Dump(shader->dumper, load_inst);
        VIR_LOG(dumper, "Transformed instruction:\n");
        VIR_LOG_FLUSH(dumper);
        VIR_Inst_Dump(shader->dumper, inst);
    }

    return virErrCode;
}

static VSC_ErrCode
_VSC_UF_AUBO_TransformNormalInstruction(
    IN OUT VSC_UF_AUBO* aubo,
    IN OUT VIR_Shader* shader,
    IN OUT VIR_Function* func,
    IN OUT VIR_Instruction* inst,
    IN VIR_Instruction* insert_before,
    IN OUT VIR_Operand* src
    )
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    gctUINT shader_kind_id = VIR_ShaderKind_Map2KindId(VIR_Shader_GetKind(shader));
    VIR_Instruction* new_inst, *conv_inst = gcvNULL;
    VIR_TypeId src_type_id = VIR_Operand_GetType(src);
    /*VIR_Type* src_type = VIR_Shader_GetTypeFromId(shader, src_type_id);*/
    VIR_VirRegId load_regid, conv_regid;
    VIR_SymId load_symid, conv_symid, replacing_symid;
    VIR_Operand *new_dest, *new_src0, *new_src1;
    VIR_Enable new_enable;
    VIR_Symbol* src_sym = VIR_Operand_GetSymbol(src);
    VIR_Type* src_sym_type = VIR_Symbol_GetType(src_sym);
    VIR_TypeId src_data_type_id = _VSC_UF_AUBO_GetUniformDataTypeID(shader, src);
    VIR_Type* src_data_type = VIR_Shader_GetTypeFromId(shader, src_data_type_id);
    VIR_Uniform* src_uniform = VIR_Symbol_GetUniform(src_sym);
    VSC_OPTN_UF_AUBO_Options* options = VSC_UF_AUBO_GetOptions(aubo);
    VIR_Dumper* dumper = VSC_UF_AUBO_GetDumper(aubo);

    virErrCode = VIR_Function_AddInstructionBefore(func, VIR_OP_LOAD, src_data_type_id, insert_before, &new_inst);
    if(virErrCode != VSC_ERR_NONE) return virErrCode;

    virErrCode = VIR_Function_NewOperand(func, &new_dest);
    if(virErrCode != VSC_ERR_NONE) return virErrCode;
    virErrCode = VIR_Function_NewOperand(func, &new_src0);
    if(virErrCode != VSC_ERR_NONE) return virErrCode;
    virErrCode = VIR_Function_NewOperand(func, &new_src1);
    if(virErrCode != VSC_ERR_NONE) return virErrCode;
    VIR_Inst_SetDest(new_inst, new_dest);
    VIR_Inst_SetSource(new_inst, 0, new_src0);
    VIR_Inst_SetSource(new_inst, 1, new_src1);

    /* The type of src may be different with the type of uniform data. Use
       the type of uniform data to construct load instruction here */
    load_regid = VIR_Shader_NewVirRegId(shader, 1);
    virErrCode = VIR_Shader_AddSymbol(shader,
                                      VIR_SYM_VIRREG,
                                      load_regid,
                                      src_data_type,
                                      VIR_STORAGE_UNKNOWN,
                                      &load_symid);
    if(virErrCode != VSC_ERR_NONE) return virErrCode;

    VIR_Operand_SetTempRegister(new_dest, func, load_symid, src_data_type_id);
    replacing_symid = load_symid;
    new_enable = VIR_Type_Conv2Enable(src_data_type);
    VIR_Operand_SetLvalue(new_dest, gcvTRUE);
    VIR_Operand_SetEnable(new_dest, new_enable);

    if(isSymUniformMovedToCUBO(src_sym))
    {
        VIR_SymId cubo_addr_symid = VSC_UF_AUBO_GetCUBOAddr(aubo, shader_kind_id);
        VIR_Symbol* cubo_addr_sym = VIR_Shader_GetSymFromId(shader, cubo_addr_symid);
        VIR_Uniform* cubo_addr_uniform = VIR_Symbol_GetUniform(cubo_addr_sym);
        VIR_Operand_SetUniform(new_src0, cubo_addr_uniform, shader);
    }
    else
    {
        VIR_SymId dubo_addr_symid;
        VIR_Symbol* dubo_addr_sym;
        VIR_Uniform* dubo_addr_uniform;

        gcmASSERT(isSymUniformMovedToDUBO(src_sym));

        dubo_addr_symid = VSC_UF_AUBO_GetDUBOAddr(aubo, shader_kind_id);
        dubo_addr_sym = VIR_Shader_GetSymFromId(shader, dubo_addr_symid);
        dubo_addr_uniform = VIR_Symbol_GetUniform(dubo_addr_sym);
        VIR_Operand_SetUniform(new_src0, dubo_addr_uniform, shader);
    }
    switch(VIR_Type_GetKind(src_sym_type))
    {
        case VIR_TY_INVALID:
        case VIR_TY_SCALAR:
        case VIR_TY_VECTOR:
            VIR_Operand_SetImmediateUint(new_src1, src_uniform->offset);
            break;
        case VIR_TY_MATRIX:
        {
            if(VIR_Operand_GetRelAddrMode(src) == VIR_INDEXED_NONE)
            {
                VIR_TypeId base_typeid = VIR_Type_GetBaseTypeId(src_sym_type);
                gctUINT const_index = VIR_Operand_GetMatrixConstIndex(src);
                VIR_TypeId row_typeid = VIR_GetTypeRowType(base_typeid);
                gctUINT stride = VIR_GetTypeSize(row_typeid);
                VIR_Operand_SetImmediateUint(new_src1, src_uniform->offset + stride * const_index);
            }
            break;
        }
        case VIR_TY_ARRAY:
        {
            if(VIR_Operand_GetRelAddrMode(src) == VIR_INDEXED_NONE)
            {
                VIR_TypeId base_typeid = VIR_Type_GetBaseTypeId(src_sym_type);
                VIR_Type* base_type = VIR_Shader_GetTypeFromId(shader, base_typeid);
                gctUINT const_index = VIR_Operand_GetRelIndexing(src);
                gctUINT mat_index = VIR_Operand_GetMatrixConstIndex(src);
                gctUINT stride;
                if(VIR_Type_isMatrix(base_type))
                {
                    VIR_TypeId row_typeid = VIR_GetTypeRowType(VIR_Type_GetIndex(base_type));
                    VIR_Type* row_type = VIR_Shader_GetTypeFromId(shader, row_typeid);
                    stride = VIR_Shader_GetTypeByteSize(shader, row_type);
                }
                else
                {
                    stride = VIR_Shader_GetTypeByteSize(shader, base_type);
                }
                VIR_Operand_SetImmediateUint(new_src1, src_uniform->offset + stride * (const_index + mat_index));
            }
            break;
        }
        default:
        {
            gcmASSERT(0);
        }
    }
    /* update for dual16 */
    if(VIR_Shader_isDual16Mode(shader))
    {
        gctBOOL needRunSingleT = gcvFALSE;
        gctBOOL dual16NotSupported = gcvFALSE;
        VIR_Inst_Check4Dual16(new_inst, &needRunSingleT, &dual16NotSupported, gcvNULL, gcvNULL);
        if (needRunSingleT)
        {
            VIR_Inst_SetThreadMode(new_inst, VIR_THREAD_D16_DUAL_32);
        }
    }

    /* The type of src may be different with the type of uniform data. Add
       conv instruction here if needed*/
    if(VIR_GetTypeComponentType(src_data_type_id) != VIR_GetTypeComponentType(src_type_id) &&
       (VIR_GetTypeFlag(src_data_type_id) & VIR_TYFLAG_ISFLOAT) && (VIR_GetTypeFlag(src_type_id) & VIR_TYFLAG_ISINTEGER))
    {
        VIR_TypeId converted_typeid = VIR_TypeId_ComposeNonOpaqueType(VIR_GetTypeComponentType(src_type_id), VIR_GetTypeComponents(src_data_type_id), 1);
        virErrCode = VIR_Function_AddInstructionBefore(func, VIR_OP_AQ_F2I, converted_typeid, insert_before, &conv_inst);
        if(virErrCode != VSC_ERR_NONE) return virErrCode;

        virErrCode = VIR_Function_NewOperand(func, &new_dest);
        if(virErrCode != VSC_ERR_NONE) return virErrCode;
        virErrCode = VIR_Function_NewOperand(func, &new_src0);
        if(virErrCode != VSC_ERR_NONE) return virErrCode;
        VIR_Inst_SetDest(conv_inst, new_dest);
        VIR_Inst_SetSource(conv_inst, 0, new_src0);

        conv_regid = VIR_Shader_NewVirRegId(shader, 1);
        virErrCode = VIR_Shader_AddSymbol(shader,
                                            VIR_SYM_VIRREG,
                                            conv_regid,
                                            VIR_Shader_GetTypeFromId(shader, converted_typeid),
                                            VIR_STORAGE_UNKNOWN,
                                            &conv_symid);
        if(virErrCode != VSC_ERR_NONE) return virErrCode;

        VIR_Operand_SetTempRegister(new_dest, func, conv_symid, converted_typeid);
        replacing_symid = conv_symid;
        new_enable = VIR_TypeId_Conv2Enable(converted_typeid);
        VIR_Operand_SetLvalue(new_dest, gcvTRUE);
        VIR_Operand_SetEnable(new_dest, new_enable);

        VIR_Operand_SetTempRegister(new_src0, func, load_symid, src_data_type_id);
        VIR_Operand_SetSwizzle(new_src0, VIR_Enable_2_Swizzle(new_enable));
        /* update for dual16 */
        if(VIR_Shader_isDual16Mode(shader))
        {
            gctBOOL needRunSingleT = gcvFALSE;
            gctBOOL dual16NotSupported = gcvFALSE;
            VIR_Inst_Check4Dual16(conv_inst, &needRunSingleT, &dual16NotSupported, gcvNULL, gcvNULL);
            if (needRunSingleT)
            {
                VIR_Inst_SetThreadMode(conv_inst, VIR_THREAD_D16_DUAL_32);
            }
        }
    }

    VIR_Operand_SetTempRegister(src, func, replacing_symid, src_type_id);
    VIR_Operand_SetMatrixConstIndex(src, 0);
    VIR_Operand_SetRelIndexing(src, 0);
    /*VIR_Operand_SetSwizzle(src, VIR_Enable_2_Swizzle(new_enable));*/

    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBO_Options_GetTrace(options), VSC_OPTN_UF_AUBO_Options_TRACE_TRANSFORM))
    {
        VIR_LOG(dumper, "Load instruction:\n");
        VIR_LOG_FLUSH(dumper);
        VIR_Inst_Dump(shader->dumper, new_inst);

        if(conv_inst)
        {
            VIR_LOG(dumper, "Conv instruction:\n");
            VIR_LOG_FLUSH(dumper);
            VIR_Inst_Dump(shader->dumper, conv_inst);
        }
        VIR_LOG(dumper, "Transformed instruction:\n");
        VIR_LOG_FLUSH(dumper);
        VIR_Inst_Dump(shader->dumper, inst);
    }

    return virErrCode;
}

/* construct default UBO, add uniform for default UBO addreess, move uniforms in workset into default UBO */
static VSC_ErrCode
_VSC_UF_AUBO_TransformInstructions(
    IN OUT VSC_UF_AUBO* aubo
    )
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    VSC_AllShaders* all_shaders = VSC_UF_AUBO_GetAllShaders(aubo);
    VSC_OPTN_UF_AUBO_Options* options = VSC_UF_AUBO_GetOptions(aubo);
    VIR_Dumper* dumper = VSC_UF_AUBO_GetDumper(aubo);
    gctUINT i;

    for(i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
    {
        if(VSC_UF_AUBO_GetDUBOItemCount(aubo, i) || VSC_UF_AUBO_GetCUBOItemCount(aubo, i))
        {
            VIR_Shader* shader = VSC_AllShaders_GetShader(all_shaders, i);
            VIR_FuncIterator func_iter;
            VIR_FunctionNode* func_node;

            if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBO_Options_GetTrace(options), VSC_OPTN_UF_AUBO_Options_TRACE_INPUT))
            {
                VIR_Shader_Dump(gcvNULL, "CreateAUBO Input", shader, gcvTRUE);
            }

            VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(shader));
            /* iterate over functions */
            for(func_node = VIR_FuncIterator_First(&func_iter); func_node != gcvNULL;
                func_node = VIR_FuncIterator_Next(&func_iter))
            {
                VIR_Function* func = func_node->function;
                VIR_Instruction* inst;

                /* iterate over instructions */
                for(inst = VIR_Function_GetInstList(func)->pHead;
                    inst != gcvNULL;inst = VIR_Inst_GetNext(inst))
                {
                    gctUINT j;
                    /* iterate over source operands */
                    for(j = 0; j < VIR_Inst_GetSrcNum(inst); j++)
                    {
                        VIR_Operand* src = VIR_Inst_GetSource(inst, j);
                        gcmASSERT(src);
                        if(VIR_Operand_isSymbol(src))
                        {
                            VIR_Symbol* sym = VIR_Operand_GetSymbol(src);
                            if(isSymUniformMovedToAUBO(sym))
                            {
                                VIR_Instruction* insert_before = inst;


                                if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBO_Options_GetTrace(options), VSC_OPTN_UF_AUBO_Options_TRACE_TRANSFORM))
                                {
                                    VIR_LOG(dumper, "To be transformed instruction:\n");
                                    VIR_LOG_FLUSH(dumper);
                                    VIR_Inst_Dump(shader->dumper, inst);
                                }

                                if((VIR_OPCODE_isAtom(VIR_Inst_GetOpcode(inst)) || VIR_Inst_GetOpcode(inst) == VIR_OP_CMP)
                                    && VIR_Inst_GetPrev(inst) && VIR_Inst_GetOpcode(VIR_Inst_GetPrev(inst)) == VIR_OP_LOAD)
                                {
                                    insert_before = VIR_Inst_GetPrev(inst);
                                }
                                if(VIR_Inst_GetOpcode(inst) == VIR_OP_ATOMCMPXCHG && VIR_Inst_GetOpcode(VIR_Inst_GetPrev(inst)) == VIR_OP_CMP
                                    && VIR_Inst_GetPrev(inst) && VIR_Inst_GetPrev(VIR_Inst_GetPrev(inst))
                                    && VIR_Inst_GetOpcode(VIR_Inst_GetPrev(VIR_Inst_GetPrev(inst))) == VIR_OP_LOAD)
                                {
                                    insert_before = VIR_Inst_GetPrev(VIR_Inst_GetPrev(inst));
                                }
                                if(VIR_Inst_GetOpcode(inst) == VIR_OP_LDARR || VIR_Operand_GetRelAddrMode(src))
                                {
                                    virErrCode = _VSC_UF_AUBO_TransformLdarrInstruction(aubo, shader, func, inst, insert_before, src);
                                }
                                else
                                {
                                    virErrCode = _VSC_UF_AUBO_TransformNormalInstruction(aubo, shader, func, inst, insert_before, src);
                                }
                            }
                        }
                        if(VIR_Operand_isTexldParm(src))
                        {
                            gctUINT k;
                            VIR_Operand_TexldModifier* tms = (VIR_Operand_TexldModifier*)src;
                            for(k = 0; k < VIR_TEXLDMODIFIER_COUNT; ++k)
                            {
                                VIR_Operand* tm = tms->tmodifier[k];
                                if(tm != gcvNULL)
                                {
                                    if(VIR_Operand_isSymbol(tm))
                                    {
                                        VIR_Symbol* sym = VIR_Operand_GetSymbol(tm);
                                        if(VIR_Symbol_HasFlag(sym, VIR_SYMUNIFORMFLAG_MOVED_TO_DUBO))
                                        {
                                            VIR_Symbol_SetFlag(sym, VIR_SYMUNIFORMFLAG_USED_IN_SHADER);
                                            if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBO_Options_GetTrace(options), VSC_OPTN_UF_AUBO_Options_TRACE_TRANSFORM))
                                            {
                                                VIR_LOG(dumper, "To be transformed instruction:\n");
                                                VIR_LOG_FLUSH(dumper);
                                                VIR_Inst_Dump(shader->dumper, inst);
                                            }

                                            virErrCode = _VSC_UF_AUBO_TransformNormalInstruction(aubo, shader, func, inst, inst, tm);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBO_Options_GetTrace(options), VSC_OPTN_UF_AUBO_Options_TRACE_OUTPUT))
            {
                VIR_Shader_Dump(gcvNULL, "CreateDefaultUBO Output", shader, gcvTRUE);
            }
        }
    }

    return virErrCode;
}

VSC_ErrCode VSC_UF_UtilizeAuxUBO(
    IN OUT VSC_AllShaders           *all_shaders,
    IN VSC_OPTN_UF_AUBO_Options     *options
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctUINT i;
    VSC_UF_AUBO aubo;
    VIR_Dumper *dumper = VSC_AllShaders_GetDumper(all_shaders);

    /* verify whether hardware configuration supports default UBO */
    if(!all_shaders->hwCfg->hwFeatureFlags.hasHalti1)
    {
        if(VSC_OPTN_UF_AUBO_Options_GetTrace(options))
        {
            VIR_LOG(dumper, "DefaultUBO is skipped because HALTI1 is not available:\n");
            VIR_LOG_FLUSH(dumper);
        }
        return VSC_ERR_NONE;
    }

    /* verify whether there is shader marked as skipping default UBO or not es20 or above*/
    for(i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
    {
        VIR_Shader* shader = VSC_AllShaders_GetShader(all_shaders, i);
        if(shader)
        {
            if (!VSC_OPTN_InRange(VIR_Shader_GetId(shader), VSC_OPTN_UF_AUBO_Options_GetBeforeShader(options),
                                  VSC_OPTN_UF_AUBO_Options_GetAfterShader(options)))
            {
                if(VSC_OPTN_UF_AUBO_Options_GetTrace(options))
                {
                    VIR_LOG(dumper, "Default UBO skips shader(id=%d)\n", VIR_Shader_GetId(shader));
                    VIR_LOG_FLUSH(dumper);
                }
                return errCode;
            }
            if(VIR_Shader_GetClientApiVersion(shader) < gcvAPI_OPENGL_ES20)
            {
                if(VSC_OPTN_UF_AUBO_Options_GetTrace(options))
                {
                    VIR_LOG(dumper, "Default UBO skips shader(id=%d) because it's not an es20 or above shader\n", VIR_Shader_GetId(shader));
                    VIR_LOG_FLUSH(dumper);
                }
                return errCode;
            }
        }
    }

    if(VSC_OPTN_UF_AUBO_Options_GetTrace(options))
    {
        VIR_LOG(dumper, "Default UBO starts for program\n");
        VIR_LOG_FLUSH(dumper);
    }

    _VSC_UF_AUBO_Initialize(&aubo, all_shaders, options);

    {
        gctBOOL has_active = gcvFALSE;
        _VSC_UF_AUBO_CollectUniformsInfo(&aubo, &has_active);
        if(!has_active)
        {
            if(VSC_OPTN_UF_AUBO_Options_GetTrace(options))
            {
                VIR_LOG(dumper, "Default UBO ends for program because there is no active uniform\n");
                VIR_LOG_FLUSH(dumper);
            }
            return errCode;
        }
    }

    /* calculate the size of default uniform block */
    _VSC_UF_AUBO_CalculateDUBRegCount(&aubo);

    /* HW may not support dynamic uniform indexing, so put indirectly accessed uniforms into workset if needed */
    if(all_shaders->hwCfg->hwFeatureFlags.noUniformA0 ||
       VSC_UTILS_MASK(VSC_OPTN_UF_AUBO_Options_GetHeuristics(options), VSC_OPTN_UF_AUBO_Options_HEUR_INDIRECT_MUST))
    {
        _VSC_UF_AUBO_CollectIndirectlyAccessedUniforms(&aubo);
    }

    /* pick uniforms to put into default UBO */
    _VSC_UF_AUBO_PickUniforms(&aubo);

    if(VSC_UF_AUBO_GetDUBOByteSize(&aubo) || VSC_UF_AUBO_GetCUBOByteSize(&aubo))
    {
        /* construct default UBO */
        if(VSC_UF_AUBO_GetDUBOByteSize(&aubo))
        {
            _VSC_UF_AUBO_ConstructDefaultUBO(&aubo);
        }
        /* construct const UBO */
        if(VSC_UF_AUBO_GetCUBOByteSize(&aubo))
        {
            _VSC_UF_AUBO_ConstructConstantUBO(&aubo);
        }
        /* fill default UBO */
        _VSC_UF_AUBO_FillAuxiliaryUBOs(&aubo);
        /* transform corresponding MOV into LOAD */
        _VSC_UF_AUBO_TransformInstructions(&aubo);
    }

    _VSC_UF_AUBO_Finalize(&aubo);

    return errCode;
}

VSC_ErrCode
VSC_GetUniformIndexingRange(
    IN OUT VIR_Shader       *pShader,
    IN gctINT               uniformIndex,
    OUT gctINT              *LastUniformIndex)
{
    VIR_Id      id  = VIR_IdList_GetId(&pShader->uniforms, uniformIndex);
    VIR_Symbol  *sym = VIR_Shader_GetSymFromId(pShader, id);
    VIR_Uniform *symUniform;
    gctUINT     i;
    gctINT      lastIndexingIndex = 0;

    *LastUniformIndex = uniformIndex;

    if (VIR_Symbol_isImage(sym))
    {
        symUniform = VIR_Symbol_GetImage(sym);
    }
    else
    {
        symUniform = VIR_Symbol_GetUniform(sym);
    }

    if (!symUniform)
    {
        return VSC_ERR_NONE;
    }

    if ((gctINT16)symUniform->index == symUniform->lastIndexingIndex ||
        symUniform->lastIndexingIndex == -1)
    {
        return VSC_ERR_NONE;
    }
    else
    {
        lastIndexingIndex = symUniform->lastIndexingIndex;
    }

    for (i = 0; i < (gctINT) VIR_IdList_Count(&pShader->uniforms); i ++)
    {
        /* Get uniform. */
        id  = VIR_IdList_GetId(&pShader->uniforms, i);
        sym = VIR_Shader_GetSymFromId(pShader, id);

        if (VIR_Symbol_isImage(sym))
        {
            symUniform = VIR_Symbol_GetImage(sym);
        }
        else
        {
            symUniform = VIR_Symbol_GetUniform(sym);
        }

        if (!symUniform)
        {
            continue;
        }

        if (lastIndexingIndex == symUniform->gcslIndex)
        {
            *LastUniformIndex = i;
            break;
        }
    }

    return VSC_ERR_NONE;
}

static void
VSC_CheckOpndUniformUsage(
    IN OUT VIR_Shader       *pShader,
    IN     VIR_Instruction  *inst,
    IN     VIR_Operand      *opnd,
    IN     gctBOOL          InLTC
    )
{
    gctINT             i, j;
    gctINT             lastUniformIdx;
    VIR_Symbol         *sym, *symTemp;
    VIR_Uniform        *uniform = gcvNULL;

    if (!opnd) return;

    if (VIR_Operand_GetOpKind(opnd) == VIR_OPND_SYMBOL)
    {
        sym = VIR_Operand_GetSymbol(opnd);

        if (sym)
        {
            for (i = 0; i < (gctINT) VIR_IdList_Count(&pShader->uniforms); i ++)
            {
                /* Get uniform. */
                VIR_Id id  = VIR_IdList_GetId(&pShader->uniforms, i);
                symTemp = VIR_Shader_GetSymFromId(pShader, id);

                if (symTemp == sym)
                {
                    if (VIR_Operand_GetRelAddrLevel(opnd) == VIR_NODE_INDEXED ||
                        VIR_Operand_GetRelAddrLevel(opnd) == VIR_LEAF_AND_NODE_INDEXED)
                    {
                        VSC_GetUniformIndexingRange(pShader,
                                                    i,
                                                    &lastUniformIdx);
                    }
                    else
                    {
                        lastUniformIdx = i;
                    }

                    for (j = i; j <= lastUniformIdx; j++)
                    {
                        id  = VIR_IdList_GetId(&pShader->uniforms, j);
                        sym = VIR_Shader_GetSymFromId(pShader, id);
                        uniform = gcvNULL;
                        if (!sym) continue;

                        if (VIR_Symbol_isUniform(sym))
                        {
                            uniform = VIR_Symbol_GetUniform(sym);
                        }
                        else if (VIR_Symbol_isSampler(sym))
                        {
                            uniform = VIR_Symbol_GetSampler(sym);
                        }

                        if (uniform == gcvNULL)
                        {
                            continue;
                        }

                        if (InLTC)
                        {
                            VIR_Symbol_SetFlag(sym, VIR_SYMUNIFORMFLAG_USED_IN_LTC);
                        }
                        else
                        {
                            VIR_Symbol_SetFlag(sym, VIR_SYMUNIFORMFLAG_USED_IN_SHADER);
                        }

                        if (VIR_Symbol_GetUniformKind(sym) == VIR_UNIFORM_UNIFORM_BLOCK_ADDRESS &&
                            VIR_Inst_GetOpcode(inst) == VIR_OP_LOAD)
                        {
                            VIR_Symbol_SetFlag(sym, VIR_SYMUNIFORMFLAG_USED_IN_SHADER);
                        }

                        /* If this uniform belongs to a UBO, mark this UBO as used. */
                        if (uniform->blockIndex != -1)
                        {
                        }
                    }
                }
            }
        }

        /* Below is temp conservative uniform usage analyze */

        sym = VIR_Operand_GetSymbol(opnd);

        uniform = gcvNULL;
        if (VIR_Symbol_isUniform(sym))
        {
            uniform = VIR_Symbol_GetUniform(sym);
        }
        else if (VIR_Symbol_isSampler(sym))
        {
            uniform = VIR_Symbol_GetSampler(sym);
        }

        if (uniform)
        {
            VIR_Type    *symType = VIR_Symbol_GetType(sym);

            if (VIR_Type_GetKind(symType) == VIR_TY_ARRAY)
            {
                if ((VIR_Inst_GetOpcode(inst) == VIR_OP_LDARR &&
                     opnd == VIR_Inst_GetSource(inst, 0)) ||
                     VIR_Operand_GetRelAddrMode(opnd) != VIR_INDEXED_NONE)
                {
                    uniform->realUseArraySize = VIR_Type_GetArrayLength(symType);
                }
                else
                {
                    gctINT immIdxNo = VIR_Operand_GetMatrixConstIndex(opnd);
                    gctINT indexingRange = VIR_Type_GetIndexingRange(pShader, symType);
                    VIR_Type * baseType = VIR_Shader_GetTypeFromId(pShader,
                                                                VIR_Type_GetBaseTypeId(symType));
                    gctUINT rows;
                    gctINT  thisArraySize;

                    if (VIR_Symbol_isSampler(sym) || VIR_Symbol_isImage(sym))
                    {
                        rows = 1;
                    }
                    else
                    {
                        rows = VIR_Type_GetVirRegCount(pShader, baseType);
                    }

                    if (VIR_Operand_GetIsConstIndexing(opnd))
                    {
                        immIdxNo += VIR_Operand_GetConstIndexingImmed(opnd);
                    }

                    if (immIdxNo > indexingRange)
                    {
                        gcmASSERT(gcvFALSE);
                    }

                    thisArraySize = immIdxNo / rows + 1;
                    if (thisArraySize > uniform->realUseArraySize)
                    {
                        uniform->realUseArraySize = thisArraySize;
                    }
                }
            }
        }
    }
}

static void
VSC_CheckInstUniformUsage(
    IN OUT VIR_Shader       *pShader,
    IN     VIR_Instruction  *inst,
    IN     gctBOOL          InLTC
    )
{
    gctINT             i;

    for (i = 0; i < (gctINT) VIR_Inst_GetSrcNum(inst); i++)
    {
        VIR_Operand *srcOpnd = VIR_Inst_GetSource(inst, i);

        if (srcOpnd == gcvNULL) continue;

        /* VIR_OPND_TEXLDPARM need to check its tmmodifier */
        if (VIR_Operand_GetOpKind(srcOpnd) == VIR_OPND_TEXLDPARM)
        {
            VIR_Operand_TexldModifier *TexldOperand = (VIR_Operand_TexldModifier*) srcOpnd;

            for(i = 0; i < VIR_TEXLDMODIFIER_COUNT; ++i)
            {
                if(TexldOperand->tmodifier[i] != gcvNULL)
                {
                    VSC_CheckOpndUniformUsage(
                        pShader,
                        inst,
                        TexldOperand->tmodifier[i],
                        InLTC);
                }
            }
        }

        VSC_CheckOpndUniformUsage(pShader, inst, srcOpnd, InLTC);
    }
}

VSC_ErrCode
VSC_CheckUniformUsage(
    IN OUT VIR_Shader       *pShader
    )
{
    VSC_ErrCode             retValue = VSC_ERR_NONE;

    VIR_FuncIterator  func_iter;
    VIR_FunctionNode* func_node;

    gctSIZE_T      i;
    for(i = 0; i < VIR_IdList_Count(&pShader->uniformBlocks); ++i)
    {
        VIR_Id      id  = VIR_IdList_GetId(&pShader->uniformBlocks, i);
        VIR_Symbol *sym = VIR_Shader_GetSymFromId(pShader, id);

        VIR_Symbol_ClrFlag(sym, VIR_SYMUNIFORMFLAG_USED_IN_SHADER);
        VIR_Symbol_ClrFlag(sym, VIR_SYMUNIFORMFLAG_USED_IN_LTC);
    }

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function     *func = func_node->function;
        VIR_InstIterator inst_iter;
        VIR_Instruction  *inst;

        VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(func));
        for (inst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
             inst != gcvNULL; inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
        {
            VSC_CheckInstUniformUsage(pShader, inst, gcvFALSE);
        }
    }

    /* to-do need to check usage in LTC */

    return retValue;
}

