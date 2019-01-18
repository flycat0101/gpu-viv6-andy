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
    VSC_GlobalUniformItem_SetFlag(item, VIR_Symbol_GetFlags(uniform_sym));
    /* mark the uniform item inactive by default. Following pass will go over
       instructions and mark those active ones */
    /* #TempRegSpillMemAddr is not used in the shader now (for reg spill),
       but need to be active */
    if (VIR_Symbol_GetUniformKind(uniform_sym) != VIR_UNIFORM_TEMP_REG_SPILL_MEM_ADDRESS)
    {
        VSC_GlobalUniformItem_SetFlag(item, VIR_SYMFLAG_INACTIVE);
    }
    if(location != -1)
    {
        VSC_GlobalUniformItem_SetLocation(item, location);
        VSC_GlobalUniformItem_SetRange(item, VIR_Shader_GetLogicalCount(shader, uniform_type));
    }
    VSC_GlobalUniformItem_SetRegCount(item, VIR_Type_GetVirRegCount(shader, uniform_type, -1));
    VSC_GlobalUniformItem_SetByteSize(item, VIR_Type_GetTypeByteSize(shader, uniform_type));
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
    IN gctBOOL skipDUBO,
    IN gctBOOL skipCUBO
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
                if(skipCUBO && VIR_Symbol_HasFlag(uniform_sym, VIR_SYMUNIFORMFLAG_COMPILETIME_INITIALIZED))
                {
                    return gcvFALSE;
                }
                if(skipDUBO && !VIR_Symbol_HasFlag(uniform_sym, VIR_SYMUNIFORMFLAG_COMPILETIME_INITIALIZED))
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
            VIR_Uniform* uniform = VIR_Symbol_GetUniformPointer(shader, uniform_sym);

            VIR_LOG(dumper, "shader(id:%d)\n", VIR_Shader_GetId(shader));
            VIR_LOG_FLUSH(dumper);
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
    if(location != -1 && (shader->clientApiVersion != gcvAPI_OPENCL)) /*opencl shader location of uniform variable is always 0 (need fixed by frontend) */
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
    IN VIR_Dumper* dumper,
    IN VSC_MM* mem_pool,
    IN VSC_COMPILER_CONFIG* compilerCfg
    )
{
    gctBOOL    needBoundsCheck = (compilerCfg->cFlags & VSC_COMPILER_FLAG_NEED_OOB_CHECK) != 0;
    gctINT     i;

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
    /* set bounds check flag */
    for (i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
    {
        if (all_shaders->shaders[i])
        {
            if (needBoundsCheck)
            {
                    VIR_Shader_SetFlagsExt1(all_shaders->shaders[i], VIR_SHFLAG_EXT1_ENABLE_ROBUST_CHECK);
            }
            else
            {
                    VIR_Shader_ClrFlagExt1(all_shaders->shaders[i], VIR_SHFLAG_EXT1_ENABLE_ROBUST_CHECK);
            }

        }
    }
    VSC_GlobalUniformTable_Initialize(VSC_AllShaders_GetGlobalUniformTable(all_shaders), all_shaders, mem_pool);
    VSC_AllShaders_SetDumper(all_shaders, dumper);
    VSC_AllShaders_SetMM(all_shaders, mem_pool);
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
    gctUINT perShaderReserved[VSC_MAX_LINKABLE_SHADER_STAGE_COUNT];
    gctUINT max_reg_count[VSC_MAX_LINKABLE_SHADER_STAGE_COUNT];
    VIR_SymId aubo[VSC_MAX_LINKABLE_SHADER_STAGE_COUNT];
    VIR_SymId dubo_addr[VSC_MAX_LINKABLE_SHADER_STAGE_COUNT];
    VIR_SymId cubo[VSC_MAX_LINKABLE_SHADER_STAGE_COUNT];
    VIR_SymId cubo_addr[VSC_MAX_LINKABLE_SHADER_STAGE_COUNT];
    gctUINT dubo_item_count[VSC_MAX_LINKABLE_SHADER_STAGE_COUNT];
    gctUINT cubo_item_count[VSC_MAX_LINKABLE_SHADER_STAGE_COUNT];
    gctUINT dubo_byte_size;
    gctUINT cubo_byte_size;
    VSC_HASH_TABLE* aux_addresses;
    gctUINT aux_addr_count;
    VSC_HW_CONFIG* hwCfg;
    VSC_OPTN_UF_AUBOOptions* options;
    VSC_MM* mm;
} VSC_UF_AUBO;

#define VSC_UF_AUBO_GetAllShaders(aubo)             ((aubo)->all_shaders)
#define VSC_UF_AUBO_SetAllShaders(aubo, a)          ((aubo)->all_shaders = (a))
#define VSC_UF_AUBO_GetShader(aubo, i)              ((aubo)->all_shaders->shaders[i])
#define VSC_UF_AUBO_GetDUBRegCount(aubo, i)         ((aubo)->dub_reg_count[i])
#define VSC_UF_AUBO_SetDUBRegCount(aubo, i, d)      ((aubo)->dub_reg_count[i] = (d))
#define VSC_UF_AUBO_IncDUBRegCount(aubo, i, d)      ((aubo)->dub_reg_count[i] = (aubo)->dub_reg_count[i] + (d))
#define VSC_UF_AUBO_GetPerShRsvedCount(aubo, i)     ((aubo)->perShaderReserved[i])
#define VSC_UF_AUBO_SetPerShRsvedCount(aubo, i, d)  ((aubo)->perShaderReserved[i] = (d))
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
#define VSC_UF_AUBO_GetAuxAddresses(aubo)           ((aubo)->aux_addresses)
#define VSC_UF_AUBO_SetAuxAddresses(aubo, a)        ((aubo)->aux_addresses = (a))
#define VSC_UF_AUBO_GetAuxAddrCount(aubo)           ((aubo)->aux_addr_count)
#define VSC_UF_AUBO_SetAuxAddrCount(aubo, a)        ((aubo)->aux_addr_count = (a))
#define VSC_UF_AUBO_IncAuxAddrCount(aubo)           ((aubo)->aux_addr_count++)
#define VSC_UF_AUBO_GetHwCfg(aubo)                  ((aubo)->hwCfg)
#define VSC_UF_AUBO_SetHwCfg(aubo, hc)              ((aubo)->hwCfg = (hc))
#define VSC_UF_AUBO_GetOptions(aubo)                ((aubo)->options)
#define VSC_UF_AUBO_SetOptions(aubo, o)             ((aubo)->options = (o))
#define VSC_UF_AUBO_GetDumper(aubo)                 ((aubo)->all_shaders->dumper)
#define VSC_UF_AUBO_GetMM(aubo)                     ((aubo)->mm)
#define VSC_UF_AUBO_SetMM(aubo, m)                  ((aubo)->mm = (m))

/* get the maxmium capability of default uniform block */
static gctUINT
_VSC_UF_AUBO_GetCapability(
    IN VSC_UF_AUBO* aubo,
    IN VIR_ShaderKind kind
    )
{
    VSC_HW_CONFIG* HwCfg = VSC_UF_AUBO_GetHwCfg(aubo);

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
        return HwCfg->maxPSConstRegCount;

    case VIR_SHADER_COMPUTE:
        return (HwCfg->hwFeatureFlags.hasThreadWalkerInPS ?
                HwCfg->maxVSConstRegCount : HwCfg->maxPSConstRegCount);

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
    IN VSC_HW_CONFIG* hwCfg,
    IN VSC_OPTN_UF_AUBOOptions* options
    )
{
    gctUINT i;

    gcoOS_ZeroMemory(aubo, sizeof(VSC_UF_AUBO));
    VSC_UF_AUBO_SetAllShaders(aubo, all_shaders);
    VSC_UF_AUBO_SetHwCfg(aubo, hwCfg);

    for(i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
    {
        VIR_Shader* shader = VSC_AllShaders_GetShader(all_shaders, i);

        if(shader)
        {
            VSC_UF_AUBO_SetMaxRegCount(aubo, i,
                _VSC_UF_AUBO_GetCapability(aubo, VIR_Shader_GetKind(shader)));
            if (VIR_Shader_IsEnableRobustCheck(shader))
            {
                /* reserve vec4 register for aubo base address if OOB check is enabled */
                VSC_UF_AUBO_SetPerShRsvedCount(aubo, i, 4);
            }
        }

        VSC_UF_AUBO_SetDUBO(aubo, i, VIR_INVALID_ID);
        VSC_UF_AUBO_SetDUBOAddr(aubo, i, VIR_INVALID_ID);
        VSC_UF_AUBO_SetCUBO(aubo, i, VIR_INVALID_ID);
        VSC_UF_AUBO_SetCUBOAddr(aubo, i, VIR_INVALID_ID);
    }
    VSC_UF_AUBO_SetOptions(aubo, options);
    VSC_UF_AUBO_SetMM(aubo, VSC_AllShaders_GetMM(all_shaders));

    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_INITIALIZE))
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
    VSC_OPTN_UF_AUBOOptions* options = VSC_UF_AUBO_GetOptions(aubo);
    VIR_Dumper* dumper = VSC_UF_AUBO_GetDumper(aubo);
    gctUINT i;

    gcmASSERT(has_active);
    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_MARKACTIVE))
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

                                if (VIR_Symbol_GetUniformKind(sym) == VIR_UNIFORM_PUSH_CONSTANT ||
                                    VIR_Symbol_GetDescriptorSet(sym) != -1)
                                {
                                    VSC_GlobalUniformItem_SetFlag(item, VIR_SYMUNIFORMFLAG_ALWAYS_IN_DUB);
                                }

                                if(VSC_GlobalUniformItem_HasFlag(item, VIR_SYMFLAG_INACTIVE))
                                {
                                    VSC_GlobalUniformItem_ResetFlag(item, VIR_SYMFLAG_INACTIVE);
                                    /*VIR_Symbol_RemoveFlag(sym, VIR_SYMFLAG_INACTIVE);*/
                                    VSC_GlobalUniformTable_SetHasActiveUniform(global_uniform_table);
                                    *has_active = gcvTRUE;

                                    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_MARKACTIVE))
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

typedef enum _VSC_INDIRECT_ACCESS_LEVEL
{
    VSC_IndirectAccessLevel_NONE,
    VSC_IndirectAccessLevel_IMM,
    VSC_IndirectAccessLevel_UNIFORM_DYNAMICALLY,
    VSC_IndirectAccessLevel_DYNAMICALLY,
} VSC_IndirectAccessLevel;

static void
_VSC_UF_AUBO_CollectIndirectlyAccessedUniforms(
    IN OUT VSC_UF_AUBO* aubo,
    IN VSC_IndirectAccessLevel level
    )
{
    VSC_AllShaders* all_shaders = VSC_UF_AUBO_GetAllShaders(aubo);
    VSC_GlobalUniformTable* global_uniform_table = VSC_AllShaders_GetGlobalUniformTable(all_shaders);
    VSC_OPTN_UF_AUBOOptions* options = VSC_UF_AUBO_GetOptions(aubo);
    VIR_Dumper* dumper = VSC_UF_AUBO_GetDumper(aubo);
    gctUINT i;

    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_INDIRECT))
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

                while(inst)
                {
                    VIR_Symbol* uniform_sym = gcvNULL;
                    if(VIR_Inst_GetOpcode(inst) == VIR_OP_LDARR)
                    {
                        VIR_Operand* src0 = VIR_Inst_GetSource(inst, 0);
                        if(VIR_Operand_isSymbol(src0))
                        {
                            VIR_Symbol* sym = VIR_Operand_GetSymbol(src0);
                            if(VIR_Symbol_isUniform(sym))
                            {
                                switch(level)
                                {
                                    case VSC_IndirectAccessLevel_UNIFORM_DYNAMICALLY:
                                    {
                                        VIR_Operand* src1 = VIR_Inst_GetSource(inst, 1);
                                        if(!VIR_Operand_HasFlag(src1, VIR_OPNDFLAG_UNIFORM_INDEX) || VIR_TypeId_isFloat(VIR_Operand_GetTypeId(src1)))
                                        {
                                            uniform_sym = sym;
                                        }
                                        break;
                                    }
                                    default:
                                        gcmASSERT(0);
                                }
                            }
                        }
                    }
                    else
                    {
                        gctUINT i;
                        /* iterate over source operands */
                        for(i = 0; i < VIR_Inst_GetSrcNum(inst); i++)
                        {
                            VIR_Operand* src = VIR_Inst_GetSource(inst, i);

                            gcmASSERT(src);
                            if(VIR_Operand_isSymbol(src))
                            {
                                VIR_Symbol* sym = VIR_Operand_GetSymbol(src);
                                if(VIR_Symbol_isUniform(sym) && !VIR_Operand_GetIsConstIndexing(src) && VIR_Operand_GetRelAddrMode(src) != VIR_INDEXED_NONE)
                                {
                                    uniform_sym = sym;
                                }
                            }
                        }
                    }
                    if(uniform_sym && !VIR_Symbol_HasFlag(uniform_sym, VIR_SYMUNIFORMFLAG_MOVED_TO_DUBO))
                    {
                        VSC_GlobalUniformItem* item;
                        VSC_GlobalUniformTable_FindUniformWithShaderUniform(global_uniform_table, shader, VIR_Symbol_GetIndex(uniform_sym), &item);
                        gcmASSERT(item);
                        if(!VSC_GlobalUniformItem_HasFlag(item, VIR_SYMUNIFORMFLAG_ALWAYS_IN_DUB))
                        {
                            VSC_GlobalUniformItem_SetFlag(item, VIR_SYMUNIFORMFLAG_MOVING_TO_DUBO);

                            if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_INDIRECT))
                            {
                                VSC_GlobalUniformItem_Dump(item);
                            }
                        }
                    }
                    inst = VIR_Inst_GetNext(inst);
                }
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
    VSC_OPTN_UF_AUBOOptions* options = VSC_UF_AUBO_GetOptions(aubo);

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
            if (VSC_GlobalUniformItem_IsInShader(item, i) &&
                (!VSC_GlobalUniformItem_HasFlag(item, VIR_SYMFLAG_INACTIVE)))
            {
                VSC_UF_AUBO_IncDUBRegCount(aubo, i, VSC_GlobalUniformItem_GetRegCount(item));
            }
        }
    }

    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_DUBSIZE))
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
    VSC_OPTN_UF_AUBOOptions* options = VSC_UF_AUBO_GetOptions(aubo);
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

        if(VSC_UF_AUBO_GetDUBRegCount(aubo, i) > (VSC_UF_AUBO_GetMaxRegCount(aubo, i) -
                              VSC_OPTN_UF_AUBOOptions_GetConstRegReservation(options) - VSC_UF_AUBO_GetPerShRsvedCount(aubo, i)))
        {
            result = gcvFALSE;
            if(VSC_UF_AUBO_GetDUBRegCount(aubo, i) - (VSC_UF_AUBO_GetMaxRegCount(aubo, i) -
                              VSC_OPTN_UF_AUBOOptions_GetConstRegReservation(options) - VSC_UF_AUBO_GetPerShRsvedCount(aubo, i))> max_gap)
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
       VSC_UF_AUBO_GetHwCfg(aubo)->hwFeatureFlags.constRegFileUnified &&
       totalRegCount > VSC_UF_AUBO_GetHwCfg(aubo)->maxTotalConstRegCount)
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
    VSC_OPTN_UF_AUBOOptions* options = VSC_UF_AUBO_GetOptions(aubo);

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
    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_PICK))
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
    VSC_OPTN_UF_AUBOOptions* options = VSC_UF_AUBO_GetOptions(aubo);
    VIR_Dumper* dumper = VSC_UF_AUBO_GetDumper(aubo);
    gctUINT dubo_byte_size = 0, cubo_byte_size = 0;
    VSC_GlobalUniformTable* global_uniform_table = VSC_AllShaders_GetGlobalUniformTable(all_shaders);
    VSC_GlobalUniformTable_Iterator iter;
    VSC_GlobalUniformItem* item;
    gctBOOL skipDUBO = VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetHeuristics(options), VSC_OPTN_UF_AUBOOptions_HEUR_SKIP_DUBO);
    gctBOOL skipCUBO = VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetHeuristics(options), VSC_OPTN_UF_AUBOOptions_HEUR_SKIP_CUBO);

    VSC_GlobalUniformTable_Iterator_Init(&iter, global_uniform_table);

    /* pick those pre-decided uniform items */
    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_PICK))
    {
        VIR_LOG(dumper, "Picked pre-decided uniforms:\n");
        VIR_LOG_FLUSH(dumper);
    }
    for(item = VSC_GlobalUniformTable_Iterator_First(&iter); item != gcvNULL;
        item = VSC_GlobalUniformTable_Iterator_Next(&iter))
    {
        if(VSC_GlobalUniformItem_HasFlag(item, VIR_SYMUNIFORMFLAG_MOVING_TO_DUBO))
        {
            _VSC_UF_AUBO_PickItem(aubo, item, &dubo_byte_size, &cubo_byte_size);
        }
    }

    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_PICK))
    {
        VIR_LOG(dumper, "Pick uniforms with heuristic %x:\n", VSC_OPTN_UF_AUBOOptions_GetHeuristics(options));
        VIR_LOG_FLUSH(dumper);
    }
    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetHeuristics(options), VSC_OPTN_UF_AUBOOptions_HEUR_FORCE_ALL))
    {
        VSC_GlobalUniformTable_Iterator_Init(&iter, global_uniform_table);

        for(item = VSC_GlobalUniformTable_Iterator_First(&iter); item != gcvNULL;
            item = VSC_GlobalUniformTable_Iterator_Next(&iter))
        {
            if(VSC_GlobalUniformItem_SuitableForPickingIntoAuxUBO(item, skipDUBO, skipCUBO))
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
            if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetHeuristics(options), VSC_OPTN_UF_AUBOOptions_HEUR_ORDERLY))
            {
                VSC_GlobalUniformTable_Iterator_Init(&iter, global_uniform_table);

                for(item = VSC_GlobalUniformTable_Iterator_First(&iter); item != gcvNULL;
                    item = VSC_GlobalUniformTable_Iterator_Next(&iter))
                {
                    if(VSC_GlobalUniformItem_IsInShader(item, fattest_shader) && VSC_GlobalUniformItem_SuitableForPickingIntoAuxUBO(item, skipDUBO, skipCUBO))
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

    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_PICK))
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
    VSC_OPTN_UF_AUBOOptions* options = VSC_UF_AUBO_GetOptions(aubo);
    gctUINT i;

    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_CONSTRUCTION))
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

            VIR_Shader_GetDUBO(shader, gcvTRUE, &dubo_sym, &dubo_addr_sym);
            dubo_ub = VIR_Symbol_GetUBO(dubo_sym);
            VSC_UF_AUBO_SetDUBO(aubo, i, VIR_Symbol_GetIndex(dubo_sym));
            dubo_ub->blockSize = VSC_UF_AUBO_GetDUBOByteSize(aubo);
            dubo_ub->uniforms = (VIR_Uniform **)vscMM_Alloc(&shader->pmp.mmWrapper, sizeof(VIR_Uniform*) * count);
            VIR_Symbol_SetFlag(dubo_addr_sym, VIR_SYMUNIFORMFLAG_USED_IN_SHADER);
            VSC_UF_AUBO_SetDUBOAddr(aubo, i, VIR_Symbol_GetIndex(dubo_addr_sym));
        }
    }

    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_CONSTRUCTION))
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
    VSC_OPTN_UF_AUBOOptions* options = VSC_UF_AUBO_GetOptions(aubo);
    gctUINT i;

    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_CONSTRUCTION))
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
            VIR_Symbol* cubo_sym = gcvNULL;
            VIR_UniformBlock* cubo_ub;
            VIR_Symbol* cubo_addr_sym = gcvNULL;

            VIR_Shader_GetCUBO(shader, &cubo_sym, &cubo_addr_sym);
            cubo_ub = VIR_Symbol_GetUBO(cubo_sym);
            VSC_UF_AUBO_SetCUBO(aubo, i, VIR_Symbol_GetIndex(cubo_sym));
            cubo_ub->blockSize = VSC_UF_AUBO_GetCUBOByteSize(aubo);
            cubo_ub->uniforms = (VIR_Uniform **)vscMM_Alloc(&shader->pmp.mmWrapper, sizeof(VIR_Uniform*) * count);
            VIR_Symbol_SetFlag(cubo_addr_sym, VIR_SYMUNIFORMFLAG_USED_IN_SHADER);
            VSC_UF_AUBO_SetCUBOAddr(aubo, i, VIR_Symbol_GetIndex(cubo_addr_sym));
        }
    }

    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_CONSTRUCTION))
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
    VSC_OPTN_UF_AUBOOptions* options = VSC_UF_AUBO_GetOptions(aubo);
    /*VIR_Dumper* dumper = VSC_UF_AUBO_GetDumper(aubo);*/
    VSC_GlobalUniformTable* global_uniform_table = VSC_AllShaders_GetGlobalUniformTable(all_shaders);
    VSC_GlobalUniformTable_Iterator iter;
    VSC_GlobalUniformItem* item;

    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_FILL))
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

    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_FILL))
    {
        _VSC_UF_AUBO_DumpDUBOs(aubo);
        _VSC_UF_AUBO_DumpCUBOs(aubo);
    }
    return virErrCode;
}

static VIR_TypeId
_VSC_UF_AUBO_GetUniformDataTypeID(
    IN VIR_Shader* shader,
    IN VIR_Symbol* uniformSym
    )
{
    VIR_Type* sym_type;
    VIR_TypeId result = VIR_TYPE_UNKNOWN;
    gctBOOL base_find = gcvFALSE;

    gcmASSERT(VIR_Symbol_isUniform(uniformSym));

    sym_type = VIR_Symbol_GetType(uniformSym);

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
    VIR_Instruction *movInst = gcvNULL;
    VIR_Operand* src0 = VIR_Inst_GetSource(inst, 0);
    VIR_Operand* indexing_src = gcvNULL;
    VIR_Swizzle indexing_src_swizzle = VIR_SWIZZLE_X;
    VIR_Symbol* uniform_sym = VIR_Operand_GetSymbol(src);
    VIR_Uniform* uniform = VIR_Symbol_GetUniform(uniform_sym);
    VIR_Type* src_sym_type = VIR_Symbol_GetType(VIR_Operand_GetSymbol(src));
    VIR_TypeId uniform_data_type_id = _VSC_UF_AUBO_GetUniformDataTypeID(shader, uniform_sym);
    VIR_TypeId base_typeid = VIR_Type_GetBaseTypeId(src_sym_type);
    VIR_Operand *mad_dest, *mad_src0, *mad_src1, *mad_src2, *load_dest, *load_src0, *load_src1;
    VIR_VirRegId mad_regid, load_regid;
    VIR_SymId mad_symid, load_symid;
    VIR_Symbol* mad_sym;
    gctUINT const_offset = 0;
    VSC_OPTN_UF_AUBOOptions* options = VSC_UF_AUBO_GetOptions(aubo);
    VIR_Dumper* dumper = VSC_UF_AUBO_GetDumper(aubo);
    VIR_Type* base_type = VIR_Shader_GetTypeFromId(shader, base_typeid);
    VIR_Symbol* ubo_addr_sym = gcvNULL;
    gctBOOL  robustCheck = VIR_Shader_IsEnableRobustCheck(shader);

    if (VIR_Inst_GetOpcode(inst) == VIR_OP_LDARR)
    {
        VIR_Operand* src1 = VIR_Inst_GetSource(inst, 1);
        VIR_Swizzle src1_swizzle = VIR_Operand_GetSwizzle(src1);

        if(VIR_Shader_isRAEnabled(shader))
        {
            VIR_Instruction *movaInst = VIR_Inst_GetPrev(inst);

            while(movaInst)
            {
                if(VIR_Inst_GetOpcode(movaInst) == VIR_OP_MOVA &&
                   VIR_Operand_Defines(VIR_Inst_GetDest(movaInst), src1))
                {
                    break;
                }
                movaInst = VIR_Inst_GetPrev(movaInst);
            }

            gcmASSERT(movaInst);

            indexing_src = VIR_Inst_GetSource(movaInst, 0);
        }
        else
        {
            indexing_src = VIR_Inst_GetSource(inst, 1);
        }
        indexing_src_swizzle = VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(indexing_src), VIR_Swizzle_GetChannel(src1_swizzle, 0));

        if(VIR_TypeId_isFloat(VIR_Operand_GetTypeId(indexing_src)))
        {
            VIR_Instruction *f2i_inst;
            VIR_Operand *f2i_dest;
            VIR_VirRegId f2i_regid;
            VIR_SymId f2i_symid;
            VIR_Symbol* f2i_sym;
            virErrCode = VIR_Function_AddInstructionBefore(func, VIR_OP_F2I, VIR_TYPE_UINT32, insert_before, gcvTRUE, &f2i_inst);

            f2i_regid = VIR_Shader_NewVirRegId(shader, 1);
            virErrCode = VIR_Shader_AddSymbol(shader,
                                              VIR_SYM_VIRREG,
                                              f2i_regid,
                                              VIR_Shader_GetTypeFromId(shader, VIR_TYPE_UINT32),
                                              VIR_STORAGE_UNKNOWN,
                                              &f2i_symid);
            if(virErrCode != VSC_ERR_NONE) return virErrCode;

            f2i_sym = VIR_Shader_GetSymFromId(shader, f2i_symid);
            VIR_Symbol_SetPrecision(f2i_sym, VIR_PRECISION_HIGH);
            f2i_dest = VIR_Inst_GetDest(f2i_inst);
            VIR_Operand_SetTempRegister(f2i_dest, func, f2i_symid, VIR_TYPE_UINT32);
            VIR_Operand_SetEnable(f2i_dest, VIR_ENABLE_X);

            VIR_Operand_Copy(VIR_Inst_GetSource(f2i_inst, 0), indexing_src);
            VIR_Operand_SetSwizzle(VIR_Inst_GetSource(f2i_inst, 0), indexing_src_swizzle);
            indexing_src = f2i_dest;
            indexing_src_swizzle = VIR_SWIZZLE_X;
        }
    }

    /* add a MAD instruction to compute the offset */
    virErrCode = VIR_Function_AddInstructionBefore(func, VIR_OP_MAD, VIR_TYPE_UINT32, insert_before, gcvTRUE, &mad_inst);
    if(virErrCode != VSC_ERR_NONE) return virErrCode;

    /* Get operands from this MAD instruction */
    mad_dest = VIR_Inst_GetDest(mad_inst);
    mad_src0 = VIR_Inst_GetSource(mad_inst, 0);
    mad_src1 = VIR_Inst_GetSource(mad_inst, 1);
    mad_src2 = VIR_Inst_GetSource(mad_inst, 2);

    if(VIR_Inst_GetOpcode(inst) == VIR_OP_LDARR)
    {
            /* set mad_src0 to src1 */
            gcmASSERT(indexing_src);
            VIR_Operand_Copy(mad_src0, indexing_src);
            VIR_Operand_Change2Src(mad_src0);
            VIR_Operand_SetSwizzle(mad_src0, indexing_src_swizzle);
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
        ubo_addr_sym = VIR_Shader_GetSymFromId(shader, cubo_addr_symid);
        VIR_Operand_SetUniform(mad_src2, VIR_Symbol_GetUniform(ubo_addr_sym), shader);
    }
    else
    {
        VIR_SymId dubo_addr_symid;

        gcmASSERT(isSymUniformMovedToDUBO(uniform_sym));

        dubo_addr_symid = VSC_UF_AUBO_GetDUBOAddr(aubo, shader_kind_id);
        ubo_addr_sym = VIR_Shader_GetSymFromId(shader, dubo_addr_symid);

        VIR_Operand_SetUniform(mad_src2, VIR_Symbol_GetUniform(ubo_addr_sym), shader);
    }
    VIR_Operand_SetSwizzle(mad_src2, VIR_SWIZZLE_X);

    if (robustCheck)
    {
        /* MOV  baseRegister.xyz, dubo_addr.xyz */
        virErrCode = VIR_Function_AddInstructionAfter(func,
                                                      VIR_OP_MOV,
                                                      VIR_TYPE_UINT_X3,
                                                      mad_inst,
                                                      gcvTRUE,
                                                      &movInst);
        if (virErrCode != VSC_ERR_NONE) return virErrCode;

        /* src0 - dubo_addr.xyz */
        VIR_Operand_SetOpKind(movInst->src[VIR_Operand_Src0], VIR_OPND_SYMBOL);
        VIR_Operand_SetSym(movInst->src[VIR_Operand_Src0], ubo_addr_sym);
        VIR_Operand_SetTypeId(movInst->src[VIR_Operand_Src0], VIR_TYPE_UINT_X3);
        VIR_Operand_SetSwizzle(movInst->src[VIR_Operand_Src0], VIR_SWIZZLE_XYZZ);

        /* dest */
        VIR_Operand_SetTempRegister(movInst->dest,
                                    func,
                                    mad_symid,
                                    VIR_TYPE_UINT_X3);
        VIR_Operand_SetEnable(movInst->dest, VIR_ENABLE_XYZ);
    }


    /* add a LOAD instruction to load the uniform data from Default UBO */
    virErrCode = VIR_Function_AddInstructionBefore(func, VIR_OP_LOAD, VIR_TYPE_UINT32, insert_before, gcvTRUE, &load_inst);
    if(virErrCode != VSC_ERR_NONE) return virErrCode;

    load_dest = VIR_Inst_GetDest(load_inst);
    load_src0 = VIR_Inst_GetSource(load_inst, 0);
    load_src1 = VIR_Inst_GetSource(load_inst, 1);

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
    VIR_Operand_SetEnable(load_dest, VIR_TypeId_Conv2Enable(uniform_data_type_id));

    /* set src0 of load to dubo_addr */
    VIR_Operand_SetTempRegister(load_src0, func, mad_symid, VIR_TYPE_UINT32);
    VIR_Operand_SetSwizzle(load_src0, robustCheck ? VIR_SWIZZLE_XYZZ : VIR_SWIZZLE_X);

    /* set src1 of load to mad_symid */
    if(VIR_Operand_GetIsConstIndexing(src))
    {
        gctUINT stride;

        if(VIR_Type_isMatrix(base_type))
        {
            VIR_TypeId row_typeid = VIR_GetTypeRowType(VIR_Type_GetIndex(base_type));
            VIR_Type* row_type = VIR_Shader_GetTypeFromId(shader, row_typeid);
            stride = VIR_Type_GetTypeByteSize(shader, row_type);
        }
        else
        {
            stride = _VSC_UF_AUBO_GetArrayStride(VIR_Symbol_GetType(uniform_sym));
        }
        const_offset = stride * VIR_Operand_GetMatrixConstIndex(src);
    }
    VIR_Operand_SetImmediateUint(load_src1, VIR_Uniform_GetOffset(uniform) + const_offset);

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
        VIR_Operand_SetTempRegister(src, func, load_symid, VIR_Operand_GetTypeId(src));
        VIR_Operand_SetMatrixConstIndex(src, 0);
        VIR_Operand_SetRelIndexing(src, 0);
        VIR_Operand_SetRelAddrMode(src, 0);
    }
    else
    {
        gcmASSERT(0);
    }

    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_TRANSFORM))
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
    VIR_TypeId src_type_id = VIR_Operand_GetTypeId(src);
    /*VIR_Type* src_type = VIR_Shader_GetTypeFromId(shader, src_type_id);*/
    VIR_VirRegId load_regid, conv_regid;
    VIR_SymId load_symid, conv_symid, replacing_symid;
    VIR_Operand *new_dest, *new_src0, *new_src1;
    VIR_Enable new_enable;
    VIR_Symbol* src_sym = VIR_Operand_GetSymbol(src);
    VIR_Type* src_sym_type = VIR_Symbol_GetType(src_sym);
    VIR_TypeId src_data_type_id = _VSC_UF_AUBO_GetUniformDataTypeID(shader, src_sym);
    VIR_Type* src_data_type = VIR_Shader_GetTypeFromId(shader, src_data_type_id);
    VIR_Uniform* src_uniform = VIR_Symbol_GetUniform(src_sym);
    VSC_OPTN_UF_AUBOOptions* options = VSC_UF_AUBO_GetOptions(aubo);
    VIR_Dumper* dumper = VSC_UF_AUBO_GetDumper(aubo);

    virErrCode = VIR_Function_AddInstructionBefore(func, VIR_OP_LOAD, src_data_type_id, insert_before, gcvTRUE, &new_inst);
    if(virErrCode != VSC_ERR_NONE) return virErrCode;

    new_dest = VIR_Inst_GetDest(new_inst);
    new_src0 = VIR_Inst_GetSource(new_inst, 0);
    new_src1 = VIR_Inst_GetSource(new_inst, 1);

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
    VIR_Operand_SetSwizzle(new_src0, VIR_Shader_IsEnableRobustCheck(shader) ? VIR_SWIZZLE_XYZZ : VIR_SWIZZLE_X);

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
                    stride = VIR_Type_GetTypeByteSize(shader, row_type);
                }
                else
                {
                    stride = VIR_Type_GetTypeByteSize(shader, base_type);
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

    /* The type of src may be different with the type of uniform data. Add
       conv instruction here if needed*/
    if(VIR_GetTypeComponentType(src_data_type_id) != VIR_GetTypeComponentType(src_type_id) &&
       (VIR_GetTypeFlag(src_data_type_id) & VIR_TYFLAG_ISFLOAT) && (VIR_GetTypeFlag(src_type_id) & VIR_TYFLAG_ISINTEGER))
    {
        VIR_TypeId converted_typeid = VIR_TypeId_ComposeNonOpaqueType(VIR_GetTypeComponentType(src_type_id), VIR_GetTypeComponents(src_data_type_id), 1);
        virErrCode = VIR_Function_AddInstructionBefore(func, VIR_OP_F2I, converted_typeid, insert_before, gcvTRUE, &conv_inst);
        if(virErrCode != VSC_ERR_NONE) return virErrCode;

        new_dest = VIR_Inst_GetDest(conv_inst);
        new_src0 = VIR_Inst_GetSource(conv_inst, 0);

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
    }

    VIR_Operand_SetTempRegister(src, func, replacing_symid, src_type_id);
    VIR_Operand_SetMatrixConstIndex(src, 0);
    VIR_Operand_SetRelIndexing(src, 0);
    /*VIR_Operand_SetSwizzle(src, VIR_Enable_2_Swizzle(new_enable));*/

    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_TRANSFORM))
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
    VSC_OPTN_UF_AUBOOptions* options = VSC_UF_AUBO_GetOptions(aubo);
    VIR_Dumper* dumper = VSC_UF_AUBO_GetDumper(aubo);
    gctUINT i;

    for(i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
    {
        if(VSC_UF_AUBO_GetDUBOItemCount(aubo, i) || VSC_UF_AUBO_GetCUBOItemCount(aubo, i))
        {
            VIR_Shader* shader = VSC_AllShaders_GetShader(all_shaders, i);
            VIR_FuncIterator func_iter;
            VIR_FunctionNode* func_node;

            if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_INPUT))
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


                                if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_TRANSFORM))
                                {
                                    VIR_LOG(dumper, "To be transformed instruction:\n");
                                    VIR_LOG_FLUSH(dumper);
                                    VIR_Inst_Dump(shader->dumper, inst);
                                }

                                if((VIR_OPCODE_isAtom(VIR_Inst_GetOpcode(inst)) || VIR_Inst_GetOpcode(inst) == VIR_OP_COMPARE)
                                    && VIR_Inst_GetPrev(inst) && VIR_Inst_GetOpcode(VIR_Inst_GetPrev(inst)) == VIR_OP_LOAD)
                                {
                                    insert_before = VIR_Inst_GetPrev(inst);
                                }
                                if(VIR_OPCODE_isAtomCmpxChg(VIR_Inst_GetOpcode(inst)) && VIR_Inst_GetOpcode(VIR_Inst_GetPrev(inst)) == VIR_OP_COMPARE
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
                            VIR_Operand* tms = (VIR_Operand*)src;
                            for(k = 0; k < VIR_TEXLDMODIFIER_COUNT; ++k)
                            {
                                VIR_Operand* tm = VIR_Operand_GetTexldModifier(tms, k);
                                if(tm != gcvNULL)
                                {
                                    if(VIR_Operand_isSymbol(tm))
                                    {
                                        VIR_Symbol* sym = VIR_Operand_GetSymbol(tm);
                                        if(VIR_Symbol_HasFlag(sym, VIR_SYMUNIFORMFLAG_MOVED_TO_DUBO))
                                        {
                                            VIR_Symbol_SetFlag(sym, VIR_SYMUNIFORMFLAG_USED_IN_SHADER);
                                            if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_TRANSFORM))
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

            if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_OUTPUT))
            {
                VIR_Shader_Dump(gcvNULL, "CreateDefaultUBO Output", shader, gcvTRUE);
            }
        }
    }

    return virErrCode;
}

typedef struct VSC_UF_AUBO_GetAuxAddress_Key
{
    VIR_Symbol* uniformSym;
    gctUINT constOffset;
} VSC_UF_AUBO_GetAuxAddress_Key;

static gctUINT
_VSC_UF_AUBO_GetAuxAddress_HashFunc(const void* pHashKey)
{
    VSC_UF_AUBO_GetAuxAddress_Key* key = (VSC_UF_AUBO_GetAuxAddress_Key*)pHashKey;

    return (gctUINT)(gctUINTPTR_T)key->uniformSym;
}

static gctBOOL
_VSC_UF_AUBO_GetAuxAddress_KeyCmp(const void* pHashKey1, const void* pHashKey2)
{
    VSC_UF_AUBO_GetAuxAddress_Key* key1 = (VSC_UF_AUBO_GetAuxAddress_Key*)pHashKey1;
    VSC_UF_AUBO_GetAuxAddress_Key* key2 = (VSC_UF_AUBO_GetAuxAddress_Key*)pHashKey2;

    return key1->uniformSym == key2->uniformSym &&
           key1->constOffset == key2->constOffset;
}

static VIR_SymId
_VSC_UF_AUBO_GetAuxAddress(
    IN OUT VSC_UF_AUBO* aubo,
    IN VIR_Shader* shader,
    IN VIR_Symbol* uniformSym,
    IN gctUINT constOffset
    )
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    VSC_UF_AUBO_GetAuxAddress_Key* key = (VSC_UF_AUBO_GetAuxAddress_Key*)vscMM_Alloc(VSC_UF_AUBO_GetMM(aubo), sizeof(VSC_UF_AUBO_GetAuxAddress_Key));
    VSC_HASH_TABLE* auxAddressTable = VSC_UF_AUBO_GetAuxAddresses(aubo);
    VIR_Symbol* auxAddrSym;
    VIR_SymId auxAddrSymId;

    if(auxAddressTable == gcvNULL)
    {
        auxAddressTable = vscHTBL_Create(VSC_UF_AUBO_GetMM(aubo), _VSC_UF_AUBO_GetAuxAddress_HashFunc, _VSC_UF_AUBO_GetAuxAddress_KeyCmp, 16);
        VSC_UF_AUBO_SetAuxAddresses(aubo, auxAddressTable);
    }

    key->uniformSym = uniformSym;
    key->constOffset = constOffset;
    if(!vscHTBL_DirectTestAndGet(auxAddressTable, (void*)key, (void**)&auxAddrSym))
    {
        VIR_Uniform* uniform = VIR_Symbol_GetUniform(uniformSym);
        VIR_Symbol* defaultUBOSym;
        VIR_Symbol* defaultUBOAddr;

        VIR_Shader_GetDUBO(shader, gcvTRUE, &defaultUBOSym, &defaultUBOAddr);
        if(VIR_Uniform_GetOffset(uniform) + constOffset == 0)
        {
            auxAddrSym = defaultUBOAddr;
        }
        else
        {
            gctCHAR name[64];
            VIR_NameId auxAddrNameId;
            VIR_Uniform* auxAddrUniform;
            VIR_UniformBlock* defaultUBO;
            VIR_Uniform* defaultUBOAddrUniform;

            gcoOS_PrintStrSafe(name, 64, gcvNULL, "#DefaultUBO_%d", VSC_UF_AUBO_GetAuxAddrCount(aubo));
            VSC_UF_AUBO_IncAuxAddrCount(aubo);

            virErrCode = VIR_Shader_AddString(shader,
                                              name,
                                              &auxAddrNameId);
            if(virErrCode != VSC_ERR_NONE) return VIR_INVALID_ID;

            defaultUBO = VIR_Symbol_GetUBO(defaultUBOSym);
            defaultUBOAddrUniform = VIR_Symbol_GetUniform(defaultUBOAddr);

            virErrCode = VIR_Shader_AddSymbol(shader,
                VIR_SYM_UNIFORM,
                auxAddrNameId,
                VIR_Shader_GetTypeFromId(shader, VIR_TYPE_UINT32),
                VIR_STORAGE_UNKNOWN,
                &auxAddrSymId);

            auxAddrSym = VIR_Shader_GetSymFromId(shader, auxAddrSymId);
            VIR_Symbol_SetUniformKind(auxAddrSym, VIR_UNIFORM_UNIFORM_BLOCK_ADDRESS);
            VIR_Symbol_SetPrecision(auxAddrSym, VIR_PRECISION_HIGH);
            VIR_Symbol_SetFlag(auxAddrSym, VIR_SYMFLAG_COMPILER_GEN);

            auxAddrUniform = VIR_Symbol_GetUniform(auxAddrSym);
            auxAddrUniform->blockIndex = defaultUBO->blockIndex;
            VIR_Uniform_SetOffset(auxAddrUniform, VIR_Uniform_GetOffset(uniform) + constOffset);

            while(VIR_Uniform_GetAuxAddrSymId(defaultUBOAddrUniform) != VIR_INVALID_ID)
            {
                VIR_Symbol* auxAddr = VIR_Shader_GetSymFromId(shader, VIR_Uniform_GetAuxAddrSymId(defaultUBOAddrUniform));
                defaultUBOAddrUniform = VIR_Symbol_GetUniform(auxAddr);
            }

            VIR_Uniform_SetAuxAddrSymId(defaultUBOAddrUniform, auxAddrSymId);
        }

        vscHTBL_DirectSet(auxAddressTable, (void*)key, (void*)auxAddrSym);
    }

    auxAddrSymId = VIR_Symbol_GetIndex(auxAddrSym);
    return auxAddrSymId;
}

typedef struct VSC_UF_AUBO_UNIFORMINFONODE
{
    VSC_UNI_LIST_NODE node;
    VIR_Instruction* inst;      /* inst containing the uniform */
    VIR_Operand* operand;       /* operand containing the uniform */
    VIR_Symbol* uniformSym;
    VIR_SymId indexSymId;
    VIR_Operand* indexOperand;
    VIR_Swizzle indexSwizzle;
    gctUINT constOffset;
    gctUINT stride;
    struct VSC_UF_AUBO_UNIFORMINFONODE* sameIndexList;
    struct VSC_UF_AUBO_UNIFORMINFONODE* sameIndexSameSwizzleList;
    struct VSC_UF_AUBO_UNIFORMINFONODE* sameIndexSameSwizzleSameStrideList;
    struct VSC_UF_AUBO_UNIFORMINFONODE* identicalList;
    struct VSC_UF_AUBO_UNIFORMINFONODE* identical;
    struct VSC_UF_AUBO_UNIFORMINFONODE* sameIndexSameSwizzleSameStride;
    struct VSC_UF_AUBO_UNIFORMINFONODE* sameIndexSameSwizzle;
    struct VSC_UF_AUBO_UNIFORMINFONODE* sameIndex;
    gctUINT sameIndexId;
    VIR_Instruction* f2iInst;
    VIR_Instruction* madInst;
    VIR_Instruction* loadInst;
} VSC_UF_AUBO_UniformInfoNode;

#define VSC_UF_AUBO_UniformInfoNode_GetInst(ui)                                         ((ui)->inst)
#define VSC_UF_AUBO_UniformInfoNode_SetInst(ui, i)                                      ((ui)->inst = (i))
#define VSC_UF_AUBO_UniformInfoNode_GetOperand(ui)                                      ((ui)->operand)
#define VSC_UF_AUBO_UniformInfoNode_SetOperand(ui, o)                                   ((ui)->operand = (o))
#define VSC_UF_AUBO_UniformInfoNode_GetUniformSym(ui)                                   ((ui)->uniformSym)
#define VSC_UF_AUBO_UniformInfoNode_SetUniformSym(ui, us)                               ((ui)->uniformSym = (us))
#define VSC_UF_AUBO_UniformInfoNode_GetIndexSymId(ui)                                   ((ui)->indexSymId)
#define VSC_UF_AUBO_UniformInfoNode_SetIndexSymId(ui, i)                                ((ui)->indexSymId = (i))
#define VSC_UF_AUBO_UniformInfoNode_GetIndexOperand(ui)                                 ((ui)->indexOperand)
#define VSC_UF_AUBO_UniformInfoNode_SetIndexOperand(ui, i)                              ((ui)->indexOperand = (i))
#define VSC_UF_AUBO_UniformInfoNode_GetIndexSwizzle(ui)                                 ((ui)->indexSwizzle)
#define VSC_UF_AUBO_UniformInfoNode_SetIndexSwizzle(ui, is)                             ((ui)->indexSwizzle = (is))
#define VSC_UF_AUBO_UniformInfoNode_GetConstOffset(ui)                                  ((ui)->constOffset)
#define VSC_UF_AUBO_UniformInfoNode_SetConstOffset(ui, c)                               ((ui)->constOffset = (c))
#define VSC_UF_AUBO_UniformInfoNode_GetStride(ui)                                       ((ui)->stride)
#define VSC_UF_AUBO_UniformInfoNode_SetStride(ui, s)                                    ((ui)->stride = (s))
#define VSC_UF_AUBO_UniformInfoNode_GetSameIndexList(ui)                                ((ui)->sameIndexList)
#define VSC_UF_AUBO_UniformInfoNode_SetSameIndexList(ui, s)                             ((ui)->sameIndexList = (s))
#define VSC_UF_AUBO_UniformInfoNode_GetSameIndexSameSwizzleList(ui)                     ((ui)->sameIndexSameSwizzleList)
#define VSC_UF_AUBO_UniformInfoNode_SetSameIndexSameSwizzleList(ui, s)                  ((ui)->sameIndexSameSwizzleList = (s))
#define VSC_UF_AUBO_UniformInfoNode_GetSameIndexSameSwizzleSameStrideList(ui)           ((ui)->sameIndexSameSwizzleSameStrideList)
#define VSC_UF_AUBO_UniformInfoNode_SetSameIndexSameSwizzleSameStrideList(ui, s)        ((ui)->sameIndexSameSwizzleSameStrideList = (s))
#define VSC_UF_AUBO_UniformInfoNode_GetIdenticalList(ui)                                ((ui)->identicalList)
#define VSC_UF_AUBO_UniformInfoNode_SetIdenticalList(ui, i)                             ((ui)->identicalList = (i))
#define VSC_UF_AUBO_UniformInfoNode_GetIdentical(ui)                                    ((ui)->identical)
#define VSC_UF_AUBO_UniformInfoNode_SetIdentical(ui, i)                                 ((ui)->identical = (i))
#define VSC_UF_AUBO_UniformInfoNode_GetSameIndexSameSwizzleSameStride(ui)               ((ui)->sameIndexSameSwizzleSameStride)
#define VSC_UF_AUBO_UniformInfoNode_SetSameIndexSameSwizzleSameStride(ui, s)            ((ui)->sameIndexSameSwizzleSameStride = (s))
#define VSC_UF_AUBO_UniformInfoNode_GetSameIndexSameSwizzle(ui)                         ((ui)->sameIndexSameSwizzle)
#define VSC_UF_AUBO_UniformInfoNode_SetSameIndexSameSwizzle(ui, s)                      ((ui)->sameIndexSameSwizzle = (s))
#define VSC_UF_AUBO_UniformInfoNode_GetSameIndex(ui)                                    ((ui)->sameIndex)
#define VSC_UF_AUBO_UniformInfoNode_SetSameIndex(ui, s)                                 ((ui)->sameIndex = (s))
#define VSC_UF_AUBO_UniformInfoNode_GetSameIndexId(ui)                                  ((ui)->sameIndexId)
#define VSC_UF_AUBO_UniformInfoNode_SetSameIndexId(ui, s)                               ((ui)->sameIndexId = (s))
#define VSC_UF_AUBO_UniformInfoNode_GetF2IInst(ui)                                      ((ui)->f2iInst)
#define VSC_UF_AUBO_UniformInfoNode_SetF2IInst(ui, f)                                   ((ui)->f2iInst = (f))
#define VSC_UF_AUBO_UniformInfoNode_GetMADInst(ui)                                      ((ui)->madInst)
#define VSC_UF_AUBO_UniformInfoNode_SetMADInst(ui, m)                                   ((ui)->madInst = (m))
#define VSC_UF_AUBO_UniformInfoNode_GetLOADInst(ui)                                     ((ui)->loadInst)
#define VSC_UF_AUBO_UniformInfoNode_SetLOADInst(ui, l)                                  ((ui)->loadInst = (l))

static void
_VSC_UF_AUBO_UniformInfoNode_Init(
    IN OUT VSC_UF_AUBO_UniformInfoNode* uin,
    IN VIR_Instruction* inst,
    IN VIR_Operand* opnd,
    IN VIR_Symbol* uniformSym
    )
{
    gcoOS_ZeroMemory(uin, sizeof(VSC_UF_AUBO_UniformInfoNode));
    VSC_UF_AUBO_UniformInfoNode_SetInst(uin, inst);
    VSC_UF_AUBO_UniformInfoNode_SetOperand(uin, opnd);
    VSC_UF_AUBO_UniformInfoNode_SetUniformSym(uin, uniformSym);
    VSC_UF_AUBO_UniformInfoNode_SetIndexSymId(uin, VIR_INVALID_ID);
}

static void
_VSC_UF_AUBO_UniformInfoNode_AppendSameIndexList(
    IN OUT VSC_UF_AUBO_UniformInfoNode* uin0,
    IN OUT VSC_UF_AUBO_UniformInfoNode* uin1
    )
{
    VSC_UF_AUBO_UniformInfoNode* list = VSC_UF_AUBO_UniformInfoNode_GetSameIndexList(uin0);

    VSC_UF_AUBO_UniformInfoNode_SetSameIndex(uin1, uin0);
    while(list != gcvNULL)
    {
        uin0 = list;
        list = VSC_UF_AUBO_UniformInfoNode_GetSameIndexList(uin0);
    }

    VSC_UF_AUBO_UniformInfoNode_SetSameIndexList(uin0, uin1);
}

static void
_VSC_UF_AUBO_UniformInfoNode_RemoveFromSameIndexList(
    IN OUT VSC_UF_AUBO_UniformInfoNode* uin0,
    IN OUT VSC_UF_AUBO_UniformInfoNode* uin1
    )
{
    VSC_UF_AUBO_UniformInfoNode* list = VSC_UF_AUBO_UniformInfoNode_GetSameIndexList(uin0);

    if(list == uin1)
    {
        VSC_UF_AUBO_UniformInfoNode_SetSameIndexList(uin0, VSC_UF_AUBO_UniformInfoNode_GetSameIndexList(uin1));
    }
    else
    {
        while(list != gcvNULL)
        {
            VSC_UF_AUBO_UniformInfoNode* listNext = VSC_UF_AUBO_UniformInfoNode_GetSameIndexList(list);

            if(listNext == uin1)
            {
                VSC_UF_AUBO_UniformInfoNode_SetSameIndexList(list, VSC_UF_AUBO_UniformInfoNode_GetSameIndexList(listNext));
                break;
            }
            else
            {
                list = listNext;
            }
        }
        gcmASSERT(list);
    }
    VSC_UF_AUBO_UniformInfoNode_SetSameIndexList(uin1, gcvNULL);
}

static void
_VSC_UF_AUBO_UniformInfoNode_AppendSameIndexSameSwizzleList(
    IN OUT VSC_UF_AUBO_UniformInfoNode* uin0,
    IN OUT VSC_UF_AUBO_UniformInfoNode* uin1
    )
{
    VSC_UF_AUBO_UniformInfoNode* list = VSC_UF_AUBO_UniformInfoNode_GetSameIndexSameSwizzleList(uin0);

    VSC_UF_AUBO_UniformInfoNode_SetSameIndexSameSwizzle(uin1, uin0);
    while(list != gcvNULL)
    {
        uin0 = list;
        list = VSC_UF_AUBO_UniformInfoNode_GetSameIndexSameSwizzleList(uin0);
    }

    VSC_UF_AUBO_UniformInfoNode_SetSameIndexSameSwizzleList(uin0, uin1);
}

static void
_VSC_UF_AUBO_UniformInfoNode_AppendSameIndexSameSwizzleSameStrideList(
    IN OUT VSC_UF_AUBO_UniformInfoNode* uin0,
    IN OUT VSC_UF_AUBO_UniformInfoNode* uin1
    )
{
    VSC_UF_AUBO_UniformInfoNode* list = VSC_UF_AUBO_UniformInfoNode_GetSameIndexSameSwizzleSameStrideList(uin0);

    VSC_UF_AUBO_UniformInfoNode_SetSameIndexSameSwizzleSameStride(uin1, uin0);
    while(list != gcvNULL)
    {
        uin0 = list;
        list = VSC_UF_AUBO_UniformInfoNode_GetSameIndexSameSwizzleSameStrideList(uin0);
    }

    VSC_UF_AUBO_UniformInfoNode_SetSameIndexSameSwizzleSameStrideList(uin0, uin1);
}

static void
_VSC_UF_AUBO_UniformInfoNode_LowerSameIndex2SameIndexSameSwizzle(
    IN OUT VSC_UF_AUBO_UniformInfoNode* uin0,
    IN OUT VSC_UF_AUBO_UniformInfoNode* uin1
    )
{
    _VSC_UF_AUBO_UniformInfoNode_RemoveFromSameIndexList(uin0, uin1);
    _VSC_UF_AUBO_UniformInfoNode_AppendSameIndexSameSwizzleList(uin0, uin1);
}

static void
_VSC_UF_AUBO_UniformInfoNode_LowerSameIndex2SameIndexSameSwizzleSameStride(
    IN OUT VSC_UF_AUBO_UniformInfoNode* uin0,
    IN OUT VSC_UF_AUBO_UniformInfoNode* uin1
    )
{
    _VSC_UF_AUBO_UniformInfoNode_RemoveFromSameIndexList(uin0, uin1);
    _VSC_UF_AUBO_UniformInfoNode_AppendSameIndexSameSwizzleSameStrideList(uin0, uin1);
}

static void
_VSC_UF_AUBO_UniformInfoNode_AppendIdenticalList(
    IN OUT VSC_UF_AUBO_UniformInfoNode* uin0,
    IN OUT VSC_UF_AUBO_UniformInfoNode* uin1
    )
{
    VSC_UF_AUBO_UniformInfoNode* list = VSC_UF_AUBO_UniformInfoNode_GetIdenticalList(uin0);

    VSC_UF_AUBO_UniformInfoNode_SetIdentical(uin1, uin0);
    while(list != gcvNULL)
    {
        uin0 = list;
        list = VSC_UF_AUBO_UniformInfoNode_GetIdenticalList(uin0);
    }

    VSC_UF_AUBO_UniformInfoNode_SetIdenticalList(uin0, uin1);
}

typedef enum VSC_UF_AUBO_UNIFORMINFONODE_DUMPFLAG
{
    VSC_UF_AUBO_UniformInfoNode_DumpFlag_None = 0x0,
    VSC_UF_AUBO_UniformInfoNode_DumpFlag_Inst = 0x1,
    VSC_UF_AUBO_UniformInfoNode_DumpFlag_Misc = 0x2,
    VSC_UF_AUBO_UniformInfoNode_DumpFlag_SameIndexList = 0x4,
    VSC_UF_AUBO_UniformInfoNode_DumpFlag_SameIndexSameSwizzleList = 0x8,
    VSC_UF_AUBO_UniformInfoNode_DumpFlag_SameIndexSameSwizzleSameStrideList = 0x10,
    VSC_UF_AUBO_UniformInfoNode_DumpFlag_IdenticalList = 0x20,
    VSC_UF_AUBO_UniformInfoNode_DumpFlag_Identical = 0x40,
    VSC_UF_AUBO_UniformInfoNode_DumpFlag_SameIndexSameSwizzleSameStride = 0x80,
    VSC_UF_AUBO_UniformInfoNode_DumpFlag_SameIndexSameSwizzle = 0x100,
    VSC_UF_AUBO_UniformInfoNode_DumpFlag_SameIndex = 0x200,

    VSC_UF_AUBO_UniformInfoNode_DumpFlag_All = 0xff
} VSC_UF_AUBO_UniformInfoNode_DumpFlag;

static void
_VSC_UF_AUBO_UniformInfoNode_Dump(
    IN VSC_UF_AUBO_UniformInfoNode* uin,
    IN VIR_Dumper* dumper,
    VSC_UF_AUBO_UniformInfoNode_DumpFlag flags
    )
{
    VSC_UF_AUBO_UniformInfoNode* sameIndexList = VSC_UF_AUBO_UniformInfoNode_GetSameIndexList(uin);
    VSC_UF_AUBO_UniformInfoNode* sameIndexSameSwizzleList = VSC_UF_AUBO_UniformInfoNode_GetSameIndexSameSwizzleList(uin);
    VSC_UF_AUBO_UniformInfoNode* sameIndexSameSwizzleSameStrideList = VSC_UF_AUBO_UniformInfoNode_GetSameIndexSameSwizzleSameStrideList(uin);
    VSC_UF_AUBO_UniformInfoNode* identicalList = VSC_UF_AUBO_UniformInfoNode_GetIdenticalList(uin);
    VSC_UF_AUBO_UniformInfoNode* identical = VSC_UF_AUBO_UniformInfoNode_GetIdentical(uin);
    VSC_UF_AUBO_UniformInfoNode* sameIndexSameSwizzleSameStride = VSC_UF_AUBO_UniformInfoNode_GetSameIndexSameSwizzleSameStride(uin);
    VSC_UF_AUBO_UniformInfoNode* sameIndexSameSwizzle = VSC_UF_AUBO_UniformInfoNode_GetSameIndexSameSwizzle(uin);
    VSC_UF_AUBO_UniformInfoNode* sameIndex = VSC_UF_AUBO_UniformInfoNode_GetSameIndex(uin);

    if(flags & VSC_UF_AUBO_UniformInfoNode_DumpFlag_Inst)
    {
        VIR_Inst_Dump(dumper, VSC_UF_AUBO_UniformInfoNode_GetInst(uin));
    }
    if(flags & VSC_UF_AUBO_UniformInfoNode_DumpFlag_Misc)
    {
        VIR_LOG(dumper, "indexSymId %x \n", VSC_UF_AUBO_UniformInfoNode_GetIndexSymId(uin));
        VIR_LOG(dumper, "indexSwizzle %d \n", (gctUINT)VSC_UF_AUBO_UniformInfoNode_GetIndexSwizzle(uin));
        VIR_LOG(dumper, "constOffset %d \n", VSC_UF_AUBO_UniformInfoNode_GetConstOffset(uin));
        VIR_LOG(dumper, "stride %d \n", VSC_UF_AUBO_UniformInfoNode_GetStride(uin));
        if(VSC_UF_AUBO_UniformInfoNode_GetSameIndexId(uin))
        {
            VIR_LOG(dumper, "sameIndexId %d \n", VSC_UF_AUBO_UniformInfoNode_GetSameIndexId(uin));
        }
    }
    if(sameIndexList && (flags & VSC_UF_AUBO_UniformInfoNode_DumpFlag_SameIndexList))
    {
        VIR_LOG(dumper, "sameIndexList:\n");
        _VSC_UF_AUBO_UniformInfoNode_Dump(sameIndexList, dumper, (VSC_UF_AUBO_UniformInfoNode_DumpFlag)(VSC_UF_AUBO_UniformInfoNode_DumpFlag_Inst | VSC_UF_AUBO_UniformInfoNode_DumpFlag_SameIndexList));
    }
    if(sameIndexSameSwizzleList && (flags & VSC_UF_AUBO_UniformInfoNode_DumpFlag_SameIndexSameSwizzleList))
    {
        VIR_LOG(dumper, "sameIndexSameSwizzleList:\n");
        _VSC_UF_AUBO_UniformInfoNode_Dump(sameIndexSameSwizzleList, dumper, (VSC_UF_AUBO_UniformInfoNode_DumpFlag)(VSC_UF_AUBO_UniformInfoNode_DumpFlag_Inst | VSC_UF_AUBO_UniformInfoNode_DumpFlag_SameIndexSameSwizzleList));
    }
    if(sameIndexSameSwizzleSameStrideList && (flags & VSC_UF_AUBO_UniformInfoNode_DumpFlag_SameIndexSameSwizzleSameStrideList))
    {
        VIR_LOG(dumper, "sameIndexSameSwizzleSameStrideList:\n");
        _VSC_UF_AUBO_UniformInfoNode_Dump(sameIndexSameSwizzleSameStrideList, dumper, (VSC_UF_AUBO_UniformInfoNode_DumpFlag)(VSC_UF_AUBO_UniformInfoNode_DumpFlag_Inst | VSC_UF_AUBO_UniformInfoNode_DumpFlag_SameIndexSameSwizzleSameStrideList));
    }
    if(identicalList && (flags & VSC_UF_AUBO_UniformInfoNode_DumpFlag_IdenticalList))
    {
        VIR_LOG(dumper, "identicalList:\n");
        _VSC_UF_AUBO_UniformInfoNode_Dump(identicalList, dumper, (VSC_UF_AUBO_UniformInfoNode_DumpFlag)(VSC_UF_AUBO_UniformInfoNode_DumpFlag_Inst | VSC_UF_AUBO_UniformInfoNode_DumpFlag_IdenticalList));
    }
    if(identical && (flags & VSC_UF_AUBO_UniformInfoNode_DumpFlag_Identical))
    {
        VIR_LOG(dumper, "identical:\n");
        _VSC_UF_AUBO_UniformInfoNode_Dump(identical, dumper, VSC_UF_AUBO_UniformInfoNode_DumpFlag_Inst);
    }
    if(sameIndexSameSwizzleSameStride && (flags & VSC_UF_AUBO_UniformInfoNode_DumpFlag_SameIndexSameSwizzleSameStride))
    {
        VIR_LOG(dumper, "sameIndexSameSwizzleSameStride:\n");
        _VSC_UF_AUBO_UniformInfoNode_Dump(sameIndexSameSwizzleSameStride, dumper, VSC_UF_AUBO_UniformInfoNode_DumpFlag_Inst);
    }
    if(sameIndexSameSwizzle && (flags & VSC_UF_AUBO_UniformInfoNode_DumpFlag_SameIndexSameSwizzle))
    {
        VIR_LOG(dumper, "sameIndexSameSwizzle:\n");
        _VSC_UF_AUBO_UniformInfoNode_Dump(sameIndexSameSwizzle, dumper, VSC_UF_AUBO_UniformInfoNode_DumpFlag_Inst);
    }
    if(sameIndex && (flags & VSC_UF_AUBO_UniformInfoNode_DumpFlag_SameIndex))
    {
        VIR_LOG(dumper, "sameIndex:\n");
        _VSC_UF_AUBO_UniformInfoNode_Dump(sameIndex, dumper, VSC_UF_AUBO_UniformInfoNode_DumpFlag_Inst);
    }
    if(flags == VSC_UF_AUBO_UniformInfoNode_DumpFlag_All)
    {
        VIR_LOG(dumper, "\n");
    }
    VIR_LOG_FLUSH(dumper);
}

typedef struct VSC_UF_AUBO_UNIFORMINFOLIST
{
    VSC_UNI_LIST list;
    VSC_MM* mm;
} VSC_UF_AUBO_UniformInfoList;

#define VSC_UF_AUBO_UniformInfoList_GetList(uil)                (&(uil)->list)
#define VSC_UF_AUBO_UniformInfoList_GetLength(uil)              (vscUNILST_GetNodeCount(VSC_UF_AUBO_UniformInfoList_GetList(uil)))
#define VSC_UF_AUBO_UniformInfoList_GetMM(uil)                  ((uil)->mm)
#define VSC_UF_AUBO_UniformInfoList_SetMM(uil, m)               ((uil)->mm = (m))

static void
_VSC_UF_AUBO_UniformInfoList_Initialize(
    IN OUT VSC_UF_AUBO_UniformInfoList* uil,
    IN VSC_MM* mm
    )
{
    vscUNILST_Initialize(VSC_UF_AUBO_UniformInfoList_GetList(uil), gcvFALSE);
    VSC_UF_AUBO_UniformInfoList_SetMM(uil, mm);
}

static void
_VSC_UF_AUBO_UniformInfoList_Finalize(
    IN OUT VSC_UF_AUBO_UniformInfoList* uil
    )
{
    VSC_UL_ITERATOR iter;
    VSC_UF_AUBO_UniformInfoNode *uin, *lastUIN = gcvNULL;

    vscULIterator_Init(&iter, VSC_UF_AUBO_UniformInfoList_GetList(uil));
    for(uin = (VSC_UF_AUBO_UniformInfoNode*)vscULIterator_First(&iter);
        uin != gcvNULL;
        uin = (VSC_UF_AUBO_UniformInfoNode*)vscULIterator_Next(&iter))
    {
        if(lastUIN == gcvNULL)
        {
            lastUIN = uin;
        }
        else
        {
            vscMM_Free(VSC_UF_AUBO_UniformInfoList_GetMM(uil), lastUIN);
            lastUIN = uin;
        }
    }

    if(lastUIN)
    {
        vscMM_Free(VSC_UF_AUBO_UniformInfoList_GetMM(uil), lastUIN);
    }
}

static VSC_UF_AUBO_UniformInfoNode*
_VSC_UF_AUBO_UniformInfoList_NewNode(
    IN OUT VSC_UF_AUBO_UniformInfoList* uil
    )
{
    VSC_UF_AUBO_UniformInfoNode* uin = (VSC_UF_AUBO_UniformInfoNode*)vscMM_Alloc(VSC_UF_AUBO_UniformInfoList_GetMM(uil), sizeof(VSC_UF_AUBO_UniformInfoNode));
    vscUNILST_Append(VSC_UF_AUBO_UniformInfoList_GetList(uil), (VSC_UNI_LIST_NODE*)uin);

    return uin;
}

static void
_VSC_UF_AUBO_UniformInfoList_Dump(
    IN VSC_UF_AUBO_UniformInfoList* uil,
    IN VIR_Dumper* dumper
    )
{
    VSC_UL_ITERATOR iter;
    VSC_UF_AUBO_UniformInfoNode* uin;

    vscULIterator_Init(&iter, VSC_UF_AUBO_UniformInfoList_GetList(uil));
    for(uin = (VSC_UF_AUBO_UniformInfoNode*)vscULIterator_First(&iter);
        uin != gcvNULL;
        uin = (VSC_UF_AUBO_UniformInfoNode*)vscULIterator_Next(&iter))
    {
        _VSC_UF_AUBO_UniformInfoNode_Dump(uin, dumper, VSC_UF_AUBO_UniformInfoNode_DumpFlag_All);
    }
}

static VSC_ErrCode
_VSC_UF_AUBO_BuildUpInfoList(
    IN OUT VSC_UF_AUBO* aubo,
    IN VIR_Shader* shader,
    IN VIR_Function* func,
    IN VIR_BB* bb,
    OUT VSC_UF_AUBO_UniformInfoList* uil
    )
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    VIR_Instruction* inst = BB_GET_START_INST(bb);
    VSC_OPTN_UF_AUBOOptions* options = VSC_UF_AUBO_GetOptions(aubo);

    gcmASSERT(inst);

    while(gcvTRUE)
    {
        gctUINT i;

        for(i = 0; i < VIR_Inst_GetSrcNum(inst); i++)
        {
            VIR_Operand* src = VIR_Inst_GetSource(inst, i);
            gcmASSERT(src);
            if(VIR_Operand_isSymbol(src))
            {
                VIR_Symbol* uniformSym = VIR_Operand_GetSymbol(src);
                if(isSymUniformMovedToAUBO(uniformSym))
                {
                    VIR_Type* uniformType = VIR_Symbol_GetType(uniformSym);
                    VIR_TypeId uniformBaseTypeId = VIR_Type_GetBaseTypeId(uniformType);
                    VIR_Type* uniformBaseType = VIR_Shader_GetTypeFromId(shader, uniformBaseTypeId);
                    VSC_UF_AUBO_UniformInfoNode* uin = _VSC_UF_AUBO_UniformInfoList_NewNode(uil);
                    _VSC_UF_AUBO_UniformInfoNode_Init(uin, inst, src, uniformSym);

                    if(VIR_Inst_GetOpcode(inst) == VIR_OP_LDARR)
                    {
                        VIR_Operand* src0 = VIR_Inst_GetSource(inst, 0);
                        VIR_Operand* src1 = VIR_Inst_GetSource(inst, 1);
                        VIR_Swizzle src1Swizzle = VIR_Operand_GetSwizzle(src1);
                        VIR_Operand* indexingSrc;

                        gcmASSERT(VIR_Type_isArray(uniformType) || VIR_Type_isMatrix(uniformType));

                        if(VIR_Shader_isRAEnabled(shader))
                        {
                            VIR_Instruction *movaInst = VIR_Inst_GetPrev(inst);

                            while(movaInst)
                            {
                                if(VIR_Inst_GetOpcode(movaInst) == VIR_OP_MOVA &&
                                    VIR_Operand_Defines(VIR_Inst_GetDest(movaInst), src1))
                                {
                                    break;
                                }
                                movaInst = VIR_Inst_GetPrev(movaInst);
                            }

                            gcmASSERT(movaInst);

                            indexingSrc = VIR_Inst_GetSource(movaInst, 0);
                        }
                        else
                        {
                            indexingSrc = VIR_Inst_GetSource(inst, 1);
                        }
                        VSC_UF_AUBO_UniformInfoNode_SetIndexSymId(uin, VIR_Operand_GetSymbolId_(indexingSrc));
                        VSC_UF_AUBO_UniformInfoNode_SetIndexOperand(uin, indexingSrc);
                        VSC_UF_AUBO_UniformInfoNode_SetIndexSwizzle(uin, VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(indexingSrc), VIR_Swizzle_GetChannel(src1Swizzle, 0)));

                        if(VIR_Type_isArray(uniformType))
                        {
                            if(VIR_Type_isMatrix(uniformBaseType))
                            {
                                VSC_UF_AUBO_UniformInfoNode_SetStride(uin, _VSC_UF_AUBO_GetArrayStride(uniformType) / VIR_GetTypeRows(uniformBaseTypeId));
                                VSC_UF_AUBO_UniformInfoNode_SetConstOffset(uin, VIR_Operand_GetMatrixConstIndex(src0) * VIR_GetTypeSize(VIR_GetTypeRowType(VIR_Type_GetIndex(uniformBaseType))));
                            }
                            else
                            {
                                VSC_UF_AUBO_UniformInfoNode_SetStride(uin, _VSC_UF_AUBO_GetArrayStride(uniformType));
                                VSC_UF_AUBO_UniformInfoNode_SetConstOffset(uin, VIR_Operand_GetIsConstIndexing(src0) ?
                                    (VIR_Operand_GetConstIndexingImmed(src0) + VIR_Operand_GetMatrixConstIndex(src0)) * VIR_GetTypeSize(VIR_Type_GetIndex(uniformBaseType)) : 0);
                            }
                        }
                        else
                        {
                            gcmASSERT(VIR_Type_isMatrix(uniformType));
                            VSC_UF_AUBO_UniformInfoNode_SetStride(uin, VIR_GetTypeSize(VIR_GetTypeRowType(VIR_Type_GetIndex(uniformBaseType))));
                        }
                    }
                    else if(VIR_Operand_GetRelAddrMode(src))
                    {
                        gcmASSERT(VIR_Operand_GetRelAddrMode(src) >= VIR_INDEXED_X && VIR_Operand_GetRelAddrMode(src) <= VIR_INDEXED_W);

                        VSC_UF_AUBO_UniformInfoNode_SetIndexSymId(uin, VIR_Operand_GetRelIndexing(src));
                        VSC_UF_AUBO_UniformInfoNode_SetIndexSwizzle(uin, (VIR_Swizzle)(VIR_Operand_GetRelAddrMode(src) - 1));
                        if(VIR_Type_isMatrix(uniformBaseType))
                        {
                            VSC_UF_AUBO_UniformInfoNode_SetStride(uin, _VSC_UF_AUBO_GetArrayStride(uniformType) / VIR_GetTypeRows(uniformBaseTypeId));
                        }
                        else
                        {
                            VSC_UF_AUBO_UniformInfoNode_SetStride(uin, _VSC_UF_AUBO_GetArrayStride(uniformType));
                        }
                    }
                    else
                    {
                        VSC_UF_AUBO_UniformInfoNode_SetConstOffset(uin, VIR_Operand_GetMatrixConstIndex(src) * VIR_GetTypeSize(VIR_GetTypeRowType(VIR_Type_GetIndex(uniformBaseType))));
                    }
                }
            }
            else if(VIR_Operand_isTexldParm(src))
            {
                gctUINT k;
                VIR_Operand* tms = (VIR_Operand*)src;
                for(k = 0; k < VIR_TEXLDMODIFIER_COUNT; ++k)
                {
                    VIR_Operand* tm = VIR_Operand_GetTexldModifier(tms, k);
                    if(tm != gcvNULL)
                    {
                        if(VIR_Operand_isSymbol(tm))
                        {
                            VIR_Symbol* uniformSym = VIR_Operand_GetSymbol(tm);
                            if(VIR_Symbol_HasFlag(uniformSym, VIR_SYMUNIFORMFLAG_MOVED_TO_DUBO))
                            {
                                VSC_UF_AUBO_UniformInfoNode* uin = _VSC_UF_AUBO_UniformInfoList_NewNode(uil);

                                _VSC_UF_AUBO_UniformInfoNode_Init(uin, inst, tm, uniformSym);
                            }
                        }
                    }
                }
            }
        }

        if(inst == BB_GET_END_INST(bb))
        {
            break;
        }
        else
        {
            inst = VIR_Inst_GetNext(inst);
        }
    }

    if(VSC_UF_AUBO_UniformInfoList_GetLength(uil))
    {
        /* update bb flags. set this bb having load instructions */
        BB_FLAGS_SET_LLI(bb);
    }

    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_TRANSFORM_BUILD))
    {
        VIR_Dumper* dumper = VIR_Shader_GetDumper(shader);
        VIR_LOG(dumper, "after building up info list:\n");
        _VSC_UF_AUBO_UniformInfoList_Dump(uil, VIR_Shader_GetDumper(shader));
    }

    return virErrCode;
}

static VSC_ErrCode
_VSC_UF_AUBO_ReorgUniformInfoList(
    IN OUT VSC_UF_AUBO* aubo,
    IN VIR_Shader* shader,
    IN VIR_Function* func,
    IN VIR_BB* bb,
    OUT VSC_UF_AUBO_UniformInfoList* uil
    )
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    VSC_UL_ITERATOR iter0;
    VSC_UF_AUBO_UniformInfoNode* uin0;
    gctBOOL changed = gcvFALSE;
    VSC_OPTN_UF_AUBOOptions* options = VSC_UF_AUBO_GetOptions(aubo);
    VIR_Dumper* dumper = VIR_Shader_GetDumper(shader);

    gcmASSERT(VSC_OPTN_UF_AUBOOptions_GetOpt(options));

    /* find out identicals */
    vscULIterator_Init(&iter0, VSC_UF_AUBO_UniformInfoList_GetList(uil));
    for(uin0 = (VSC_UF_AUBO_UniformInfoNode*)vscULIterator_First(&iter0);
        uin0 != gcvNULL;
        uin0 = (VSC_UF_AUBO_UniformInfoNode*)vscULIterator_Next(&iter0))
    {
        VSC_UL_ITERATOR iter1 = iter0;
        VSC_UF_AUBO_UniformInfoNode* uin1;

        for(uin1 = (VSC_UF_AUBO_UniformInfoNode*)vscULIterator_Next(&iter1);
            uin1 != gcvNULL;
            uin1 = (VSC_UF_AUBO_UniformInfoNode*)vscULIterator_Next(&iter1))
        {
            if(VSC_UF_AUBO_UniformInfoNode_GetIdentical(uin1) == gcvNULL &&
               VSC_UF_AUBO_UniformInfoNode_GetUniformSym(uin0) == VSC_UF_AUBO_UniformInfoNode_GetUniformSym(uin1) &&
               VSC_UF_AUBO_UniformInfoNode_GetIndexSymId(uin0) == VSC_UF_AUBO_UniformInfoNode_GetIndexSymId(uin1) &&
               VSC_UF_AUBO_UniformInfoNode_GetIndexSwizzle(uin0) == VSC_UF_AUBO_UniformInfoNode_GetIndexSwizzle(uin1) &&
               VSC_UF_AUBO_UniformInfoNode_GetConstOffset(uin0) == VSC_UF_AUBO_UniformInfoNode_GetConstOffset(uin1) &&
               VSC_UF_AUBO_UniformInfoNode_GetStride(uin0) == VSC_UF_AUBO_UniformInfoNode_GetStride(uin1))
            {
                _VSC_UF_AUBO_UniformInfoNode_AppendIdenticalList(uin0, uin1);
                changed = gcvTRUE;
            }
        }
    }
    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_TRANSFORM_REORG))
    {
        if(changed)
        {
            VIR_LOG(dumper, "after identicals found out:\n");
            _VSC_UF_AUBO_UniformInfoList_Dump(uil, dumper);
        }
        else
        {
            VIR_LOG(dumper, "no identicals found out.\n");
            VIR_LOG_FLUSH(dumper);
        }
    }

    /* find out same-indexes */
    changed = gcvFALSE;
    vscULIterator_Init(&iter0, VSC_UF_AUBO_UniformInfoList_GetList(uil));
    for(uin0 = (VSC_UF_AUBO_UniformInfoNode*)vscULIterator_First(&iter0);
        uin0 != gcvNULL;
        uin0 = (VSC_UF_AUBO_UniformInfoNode*)vscULIterator_Next(&iter0))
    {
        if(VSC_UF_AUBO_UniformInfoNode_GetIndexSymId(uin0) != VIR_INVALID_ID &&
           VSC_UF_AUBO_UniformInfoNode_GetSameIndex(uin0) == gcvNULL &&
           VSC_UF_AUBO_UniformInfoNode_GetIdentical(uin0) == gcvNULL)
        {
            VSC_UL_ITERATOR iter1 = iter0;
            VSC_UF_AUBO_UniformInfoNode* uin1;

            for(uin1 = (VSC_UF_AUBO_UniformInfoNode*)vscULIterator_Next(&iter1);
                uin1 != gcvNULL;
                uin1 = (VSC_UF_AUBO_UniformInfoNode*)vscULIterator_Next(&iter1))
            {
                if(VSC_UF_AUBO_UniformInfoNode_GetIdentical(uin1) == gcvNULL &&
                   VSC_UF_AUBO_UniformInfoNode_GetIndexSymId(uin1) == VSC_UF_AUBO_UniformInfoNode_GetIndexSymId(uin0) &&
                   VSC_UF_AUBO_UniformInfoNode_GetIndexSwizzle(uin1) == VSC_UF_AUBO_UniformInfoNode_GetIndexSwizzle(uin0) &&
                   VSC_UF_AUBO_UniformInfoNode_GetStride(uin1) == VSC_UF_AUBO_UniformInfoNode_GetStride(uin0))
                {
                    _VSC_UF_AUBO_UniformInfoNode_AppendSameIndexList(uin0, uin1);
                    changed = gcvTRUE;
                }
            }
        }
    }
    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_TRANSFORM_REORG))
    {
        if(changed)
        {
            VIR_LOG(dumper, "after same-indexes found out:\n");
            _VSC_UF_AUBO_UniformInfoList_Dump(uil, dumper);
        }
        else
        {
            VIR_LOG(dumper, "no same-indexes found out.\n");
            VIR_LOG_FLUSH(dumper);
        }
    }

    if(VSC_OPTN_UF_AUBOOptions_GetOpt(options) == 2)
    {
        /* find out same-index-same-swizzles */
        changed = gcvFALSE;
        vscULIterator_Init(&iter0, VSC_UF_AUBO_UniformInfoList_GetList(uil));
        for(uin0 = (VSC_UF_AUBO_UniformInfoNode*)vscULIterator_First(&iter0);
            uin0 != gcvNULL;
            uin0 = (VSC_UF_AUBO_UniformInfoNode*)vscULIterator_Next(&iter0))
        {
            if(VSC_UF_AUBO_UniformInfoNode_GetIndexOperand(uin0))
            {
                VIR_SymId indexSymId = VIR_Operand_GetSymbolId_(VSC_UF_AUBO_UniformInfoNode_GetIndexOperand(uin0));
                VIR_Symbol* indexSym = VIR_Shader_GetSymFromId(shader, indexSymId);
                VIR_TypeId indexSymTypeId = VIR_Symbol_GetTypeId(indexSym);

                if(VSC_UF_AUBO_UniformInfoNode_GetSameIndexSameSwizzle(uin0) == gcvNULL &&
                   VIR_TypeId_isFloat(indexSymTypeId))
                {
                    VSC_UF_AUBO_UniformInfoNode* sameIndex = VSC_UF_AUBO_UniformInfoNode_GetSameIndex(uin0);
                    gctUINT sameIndexId = 0;

                    if(sameIndex)
                    {
                        while(sameIndex != uin0)
                        {
                            sameIndexId++;
                            sameIndex = VSC_UF_AUBO_UniformInfoNode_GetSameIndexList(sameIndex);
                        }

                        gcmASSERT(sameIndexId < 4);
                        VSC_UF_AUBO_UniformInfoNode_SetSameIndexId(uin0, sameIndexId);
                    }

                    sameIndex = VSC_UF_AUBO_UniformInfoNode_GetSameIndexList(uin0);
                    while(sameIndex)
                    {
                        VSC_UF_AUBO_UniformInfoNode* sameIndexNext = VSC_UF_AUBO_UniformInfoNode_GetSameIndexList(sameIndex);
                        if(VSC_UF_AUBO_UniformInfoNode_GetIndexSwizzle(sameIndex) == VSC_UF_AUBO_UniformInfoNode_GetIndexSwizzle(uin0))
                        {
                            _VSC_UF_AUBO_UniformInfoNode_LowerSameIndex2SameIndexSameSwizzle(uin0, sameIndex);
                            VSC_UF_AUBO_UniformInfoNode_SetSameIndexId(sameIndex, VSC_UF_AUBO_UniformInfoNode_GetSameIndexId(uin0));
                            changed = gcvTRUE;
                        }
                        sameIndex = sameIndexNext;
                    }
                }
            }
        }
        if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_TRANSFORM_REORG))
        {
            if(changed)
            {
                VIR_LOG(dumper, "after same-index-same-swizzles found out:\n");
                _VSC_UF_AUBO_UniformInfoList_Dump(uil, dumper);
            }
            else
            {
                VIR_LOG(dumper, "no same-index-same-swizzles found out.\n");
                VIR_LOG_FLUSH(dumper);
            }
        }
    }
    else
    {
        /* find out same-index-same-stride-same-swizzles */
        changed = gcvFALSE;
        vscULIterator_Init(&iter0, VSC_UF_AUBO_UniformInfoList_GetList(uil));
        for(uin0 = (VSC_UF_AUBO_UniformInfoNode*)vscULIterator_First(&iter0);
            uin0 != gcvNULL;
            uin0 = (VSC_UF_AUBO_UniformInfoNode*)vscULIterator_Next(&iter0))
        {
            if(VSC_UF_AUBO_UniformInfoNode_GetSameIndexList(uin0))
            {
                VSC_UF_AUBO_UniformInfoNode* sameIndex = VSC_UF_AUBO_UniformInfoNode_GetSameIndexList(uin0);

                while(sameIndex)
                {
                    VSC_UF_AUBO_UniformInfoNode* sameIndexNext = VSC_UF_AUBO_UniformInfoNode_GetSameIndexList(sameIndex);
                    if(VSC_UF_AUBO_UniformInfoNode_GetStride(sameIndex) == VSC_UF_AUBO_UniformInfoNode_GetStride(uin0) &&
                       VSC_UF_AUBO_UniformInfoNode_GetIndexSwizzle(sameIndex) == VSC_UF_AUBO_UniformInfoNode_GetIndexSwizzle(uin0))
                    {
                        _VSC_UF_AUBO_UniformInfoNode_LowerSameIndex2SameIndexSameSwizzleSameStride(uin0, sameIndex);
                        changed = gcvTRUE;
                    }
                    sameIndex = sameIndexNext;
                }
            }
        }
        if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_TRANSFORM_REORG))
        {
            if(changed)
            {
                VIR_LOG(dumper, "after same-index-same-stride-same-swizzles found out:\n");
                _VSC_UF_AUBO_UniformInfoList_Dump(uil, dumper);
            }
            else
            {
                VIR_LOG(dumper, "no same-index-same-stride-same-swizzles found out.\n");
                VIR_LOG_FLUSH(dumper);
            }
        }

        /* cut same index list for vectorization vec4 restriction, set sameIndexId at the same time */
        changed = gcvFALSE;
        vscULIterator_Init(&iter0, VSC_UF_AUBO_UniformInfoList_GetList(uil));
        for(uin0 = (VSC_UF_AUBO_UniformInfoNode*)vscULIterator_First(&iter0);
            uin0 != gcvNULL;
            uin0 = (VSC_UF_AUBO_UniformInfoNode*)vscULIterator_Next(&iter0))
        {
            if(VSC_UF_AUBO_UniformInfoNode_GetSameIndexList(uin0) &&
               VSC_UF_AUBO_UniformInfoNode_GetSameIndex(uin0) == gcvNULL)
            {
                VSC_UF_AUBO_UniformInfoNode* sameIndex = VSC_UF_AUBO_UniformInfoNode_GetSameIndexList(uin0);
                gctUINT sameIndexId = 1;

                while(sameIndex)
                {
                    VSC_UF_AUBO_UniformInfoNode_SetSameIndexId(sameIndex, sameIndexId);
                    sameIndex = VSC_UF_AUBO_UniformInfoNode_GetSameIndexList(sameIndex);
                    ++sameIndexId;
                    if(sameIndexId == 3)
                    {
                        VSC_UF_AUBO_UniformInfoNode_SetSameIndexId(sameIndex, sameIndexId);
                        break;
                    }
                }

                if(sameIndexId == 3 && VSC_UF_AUBO_UniformInfoNode_GetSameIndexList(sameIndex))
                {
                    VSC_UF_AUBO_UniformInfoNode* sameIndexNext = VSC_UF_AUBO_UniformInfoNode_GetSameIndexList(sameIndex);
                    VSC_UF_AUBO_UniformInfoNode* sameIndexIter;

                    VSC_UF_AUBO_UniformInfoNode_SetSameIndexList(sameIndex, gcvNULL);
                    VSC_UF_AUBO_UniformInfoNode_SetSameIndex(sameIndexNext, gcvNULL);
                    for(sameIndexIter = VSC_UF_AUBO_UniformInfoNode_GetSameIndexList(sameIndexNext);
                        sameIndexIter != gcvNULL;
                        sameIndexIter = VSC_UF_AUBO_UniformInfoNode_GetSameIndexList(sameIndexIter))
                    {
                        VSC_UF_AUBO_UniformInfoNode_SetSameIndex(sameIndexIter, sameIndexNext);
                    }
                    changed = gcvTRUE;
                }
            }
        }
        if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_TRANSFORM_REORG))
        {
            if(changed)
            {
                VIR_LOG(dumper, "after cut same index list:\n");
                _VSC_UF_AUBO_UniformInfoList_Dump(uil, dumper);
            }
            else
            {
                VIR_LOG(dumper, "no cut same index list.\n");
                VIR_LOG_FLUSH(dumper);
            }
        }
    }

    return virErrCode;
}

static void
_Extract_LShift_Mul3(
    IN gctUINT stride,
    OUT gctUINT* lshift,
    OUT gctUINT* mul3
    )
{
    gctUINT s = stride;
    gctUINT ls = 0;
    gctUINT m3 = 0;

    if(stride % 3 == 0)
    {
        m3 = 1;
        s = stride / 3;
    }

    while(s != 1)
    {
        ls++;
        s >>= 1;
    }

    gcmASSERT(stride == (gctUINT)((1 << ls) * (m3 ? 3 : 1)));
    gcmASSERT(lshift && mul3);

    *lshift = ls;
    *mul3 = m3;
}

static VSC_ErrCode
_VSC_UF_AUBO_InsertInstructions(
    IN OUT VSC_UF_AUBO* aubo,
    IN OUT VIR_Shader* shader,
    IN OUT VIR_Function* func,
    IN OUT VIR_BB* bb,
    IN VSC_UF_AUBO_UniformInfoList* uil
    )
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    VSC_UL_ITERATOR iter;
    VSC_UF_AUBO_UniformInfoNode* uin;
    VSC_OPTN_UF_AUBOOptions* options = VSC_UF_AUBO_GetOptions(aubo);
    VIR_Dumper* dumper = VIR_Shader_GetDumper(shader);

    vscULIterator_Init(&iter, VSC_UF_AUBO_UniformInfoList_GetList(uil));
    for(uin = (VSC_UF_AUBO_UniformInfoNode*)vscULIterator_First(&iter);
        uin != gcvNULL;
        uin = (VSC_UF_AUBO_UniformInfoNode*)vscULIterator_Next(&iter))
    {
        VIR_Instruction* inst = VSC_UF_AUBO_UniformInfoNode_GetInst(uin);
        VIR_OpCode opcode = VIR_Inst_GetOpcode(inst);
        VIR_Operand* operand = VSC_UF_AUBO_UniformInfoNode_GetOperand(uin);
        VIR_TypeId operandTypeId = VIR_Operand_GetTypeId(operand);
        VIR_Type* operandType = VIR_Shader_GetTypeFromId(shader, operandTypeId);
        VIR_Swizzle operandSwizzle = VIR_Operand_GetSwizzle(operand);
        VIR_Symbol* uniformSym = VSC_UF_AUBO_UniformInfoNode_GetUniformSym(uin);
        VIR_Type* uniformSymType = VIR_Symbol_GetType(uniformSym);
        VIR_TypeId uniformSymRegTypeId = _VSC_UF_AUBO_GetUniformDataTypeID(shader, uniformSym);
        VIR_Type* uniformSymRegType = VIR_Shader_GetTypeFromId(shader, uniformSymRegTypeId);
        VIR_Uniform* uniform = VIR_Symbol_GetUniform(uniformSym);
        VIR_Instruction* insertBefore = inst;

        if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_TRANSFORM_INSERT))
        {
            VIR_LOG(dumper, "inst:\n");
            VIR_LOG_FLUSH(dumper);
            VIR_Inst_Dump(dumper, inst);
        }

        if((VIR_OPCODE_isAtom(VIR_Inst_GetOpcode(inst)) || opcode == VIR_OP_COMPARE)
            && VIR_Inst_GetPrev(inst) && VIR_Inst_GetOpcode(VIR_Inst_GetPrev(inst)) == VIR_OP_LOAD)
        {
            insertBefore = VIR_Inst_GetPrev(inst);
        }
        if(VIR_OPCODE_isAtomCmpxChg(opcode) && VIR_Inst_GetOpcode(VIR_Inst_GetPrev(inst)) == VIR_OP_COMPARE
            && VIR_Inst_GetPrev(inst) && VIR_Inst_GetPrev(VIR_Inst_GetPrev(inst))
            && VIR_Inst_GetOpcode(VIR_Inst_GetPrev(VIR_Inst_GetPrev(inst))) == VIR_OP_LOAD)
        {
            insertBefore = VIR_Inst_GetPrev(VIR_Inst_GetPrev(inst));
        }

        if(VSC_UF_AUBO_UniformInfoNode_GetIdentical(uin))
        {
            /* handle uniform usage having identical */
            VSC_UF_AUBO_UniformInfoNode* identical = VSC_UF_AUBO_UniformInfoNode_GetIdentical(uin);
            VIR_Instruction* load = VSC_UF_AUBO_UniformInfoNode_GetLOADInst(identical);
            VIR_Operand* loadDest = VIR_Inst_GetDest(load);

            if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_TRANSFORM_INSERT))
            {
                VIR_LOG(dumper, "identical to inst:\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Inst_Dump(dumper, VSC_UF_AUBO_UniformInfoNode_GetInst(identical));
                VIR_LOG(dumper, "use its load:\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Inst_Dump(dumper, load);
            }

            if(opcode == VIR_OP_LDARR)
            {
                VIR_Inst_SetOpcode(inst, VIR_OP_MOV);
                VIR_Inst_SetSrcNum(inst, 1);
                VIR_Operand_Copy(VIR_Inst_GetSource(inst, 0), loadDest);
                VIR_Operand_Change2Src(VIR_Inst_GetSource(inst, 0));
                VIR_Operand_SetSwizzle(operand, operandSwizzle);
                if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_TRANSFORM_INSERT))
                {
                    VIR_LOG(dumper, "propagate load and change ldarr to mov:\n");
                    VIR_LOG_FLUSH(dumper);
                    VIR_Inst_Dump(dumper, inst);
                }
            }
            else
            {
                VIR_Operand* operand = VSC_UF_AUBO_UniformInfoNode_GetOperand(uin);

                VIR_Operand_Copy(operand, loadDest);
                VIR_Operand_Change2Src(operand);
                VIR_Operand_SetSwizzle(operand, operandSwizzle);
                VIR_Operand_SetRelIndex(operand, 0);
                VIR_Operand_SetRelAddrMode(operand, 0);
                VIR_Operand_SetMatrixConstIndex(operand, 0);
                if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_TRANSFORM_INSERT))
                {
                    VIR_LOG(dumper, "propagate as:\n");
                    VIR_LOG_FLUSH(dumper);
                    VIR_Inst_Dump(dumper, inst);
                }
            }
        }
        else if((VSC_UF_AUBO_UniformInfoNode_GetSameIndexSameSwizzle(uin) ||
                 VSC_UF_AUBO_UniformInfoNode_GetSameIndex(uin)) && VSC_OPTN_UF_AUBOOptions_GetOpt(options) == 2)
        {
            /* handle uniform usage having same index */
            VSC_UF_AUBO_UniformInfoNode* sameIndex = VSC_UF_AUBO_UniformInfoNode_GetSameIndex(uin);
            gctUINT sameIndexId = VSC_UF_AUBO_UniformInfoNode_GetSameIndexId(uin);
            VIR_Instruction* f2i = VSC_UF_AUBO_UniformInfoNode_GetF2IInst(sameIndex);
            VIR_Operand* f2iDest;
            VIR_Operand *loadDest;
            VIR_SymId loadSymId;
            gctUINT lshift, mul3;

            gcmASSERT(opcode == VIR_OP_LDARR && f2i);
            f2iDest = VIR_Inst_GetDest(f2i);

            if(VSC_UF_AUBO_UniformInfoNode_GetSameIndexSameSwizzle(uin))
            {
                VSC_UF_AUBO_UniformInfoNode* sameIndexSameSwizzle = VSC_UF_AUBO_UniformInfoNode_GetSameIndexSameSwizzle(uin);
                sameIndexId = VSC_UF_AUBO_UniformInfoNode_GetSameIndexId(sameIndexSameSwizzle);
            }

            if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_TRANSFORM_INSERT))
            {
                VIR_LOG(dumper, "same index as inst:\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Inst_Dump(dumper, VSC_UF_AUBO_UniformInfoNode_GetInst(sameIndex));
                VIR_LOG(dumper, "use the f2i:\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Inst_Dump(dumper, f2i);
            }

            /* insert load */
            {
                VIR_Instruction* load;
                VIR_Operand *loadSrc0, *loadSrc1;
                VIR_VirRegId loadRegId;
                VIR_SymId addressSymId;

                VIR_Function_AddInstructionBefore(func, VIR_OP_LOAD, uniformSymRegTypeId, insertBefore, gcvTRUE, &load);
                loadDest = VIR_Inst_GetDest(load);
                loadSrc0 = VIR_Inst_GetSource(load, 0);
                loadSrc1 = VIR_Inst_GetSource(load, 1);

                /* set loadDest */
                loadRegId = VIR_Shader_NewVirRegId(shader, 1);
                virErrCode = VIR_Shader_AddSymbol(shader,
                                                  VIR_SYM_VIRREG,
                                                  loadRegId,
                                                  uniformSymRegType,
                                                  VIR_STORAGE_UNKNOWN,
                                                  &loadSymId);
                if(virErrCode != VSC_ERR_NONE) return virErrCode;

                VIR_Operand_SetTempRegister(loadDest, func, loadSymId, operandTypeId);
                VIR_Operand_SetEnable(loadDest, VIR_TypeId_Conv2Enable(uniformSymRegTypeId));

                addressSymId = _VSC_UF_AUBO_GetAuxAddress(aubo, shader, uniformSym, VSC_UF_AUBO_UniformInfoNode_GetConstOffset(uin));
                VIR_Operand_SetSymbol(loadSrc0, func, addressSymId);
                VIR_Operand_SetSwizzle(loadSrc0, VIR_Shader_IsEnableRobustCheck(shader) ? VIR_SWIZZLE_XYZZ : VIR_SWIZZLE_X);

                _Extract_LShift_Mul3(VSC_UF_AUBO_UniformInfoNode_GetStride(uin), &lshift, &mul3);
                VIR_Operand_Copy(loadSrc1, f2iDest);
                VIR_Operand_Change2Src(loadSrc1);
                VIR_Operand_SetSwizzle(loadSrc1, (VIR_Swizzle)sameIndexId);
                VIR_Operand_SetLShift(loadSrc1, lshift);
                if(mul3)
                {
                    VIR_Operand_SetModifier(loadSrc1, VIR_MOD_X3);
                }

                VSC_UF_AUBO_UniformInfoNode_SetLOADInst(uin, load);

                if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_TRANSFORM_INSERT))
                {
                    VIR_LOG(dumper, "insert load:\n");
                    VIR_LOG_FLUSH(dumper);
                    VIR_Inst_Dump(dumper, load);
                }
            }

            /* propagate load and change ldarr to mov */
            {
                VIR_Inst_SetOpcode(inst, VIR_OP_MOV);
                VIR_Inst_SetSrcNum(inst, 1);
                VIR_Operand_Copy(VIR_Inst_GetSource(inst, 0), loadDest);
                VIR_Operand_Change2Src(VIR_Inst_GetSource(inst, 0));
                VIR_Operand_SetSwizzle(operand, operandSwizzle);
                if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_TRANSFORM_INSERT))
                {
                    VIR_LOG(dumper, "propagate load and change ldarr to mov:\n");
                    VIR_LOG_FLUSH(dumper);
                    VIR_Inst_Dump(dumper, inst);
                }
            }
        }
        else if((VSC_UF_AUBO_UniformInfoNode_GetSameIndexSameSwizzleSameStride(uin) ||
                 VSC_UF_AUBO_UniformInfoNode_GetSameIndex(uin)) && VSC_OPTN_UF_AUBOOptions_GetOpt(options) == 1)
        {
            /* handle uniform usage having same index */
            VSC_UF_AUBO_UniformInfoNode* sameIndex = VSC_UF_AUBO_UniformInfoNode_GetSameIndex(uin);
            gctUINT sameIndexId = VSC_UF_AUBO_UniformInfoNode_GetSameIndexId(uin);
            VIR_Instruction* mad = VSC_UF_AUBO_UniformInfoNode_GetMADInst(sameIndex);
            VIR_Operand* madDest = VIR_Inst_GetDest(mad);
            VIR_Operand *loadDest;
            VIR_SymId loadSymId;

            gcmASSERT(opcode == VIR_OP_LDARR);

            if(VSC_UF_AUBO_UniformInfoNode_GetSameIndexSameSwizzleSameStride(uin))
            {
                VSC_UF_AUBO_UniformInfoNode* sameIndexSameSwizzleSameStride = VSC_UF_AUBO_UniformInfoNode_GetSameIndexSameSwizzleSameStride(uin);
                sameIndexId = VSC_UF_AUBO_UniformInfoNode_GetSameIndexId(sameIndexSameSwizzleSameStride);
            }

            gcmASSERT(opcode == VIR_OP_LDARR);
            if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_TRANSFORM_INSERT))
            {
                VIR_LOG(dumper, "same index as inst:\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Inst_Dump(dumper, VSC_UF_AUBO_UniformInfoNode_GetInst(sameIndex));
                VIR_LOG(dumper, "use the mad:\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Inst_Dump(dumper, mad);
            }

            /* insert load */
            {
                VIR_Instruction* load;
                VIR_Operand *loadSrc0, *loadSrc1;
                VIR_VirRegId loadRegId;

                VIR_Function_AddInstructionBefore(func, VIR_OP_LOAD, uniformSymRegTypeId, insertBefore, gcvTRUE, &load);
                loadDest = VIR_Inst_GetDest(load);
                loadSrc0 = VIR_Inst_GetSource(load, 0);
                loadSrc1 = VIR_Inst_GetSource(load, 1);

                /* set loadDest */
                loadRegId = VIR_Shader_NewVirRegId(shader, 1);
                virErrCode = VIR_Shader_AddSymbol(shader,
                                                    VIR_SYM_VIRREG,
                                                    loadRegId,
                                                    uniformSymRegType,
                                                    VIR_STORAGE_UNKNOWN,
                                                    &loadSymId);
                if(virErrCode != VSC_ERR_NONE) return virErrCode;

                VIR_Operand_SetTempRegister(loadDest, func, loadSymId, operandTypeId);
                VIR_Operand_SetEnable(loadDest, VIR_TypeId_Conv2Enable(uniformSymRegTypeId));

                VIR_Operand_Copy(loadSrc0, madDest);
                VIR_Operand_Change2Src(loadSrc0);
                VIR_Operand_SetSwizzle(loadSrc0, (VIR_Swizzle)sameIndexId);
                VIR_Operand_SetImmediateUint(loadSrc1, VSC_UF_AUBO_UniformInfoNode_GetConstOffset(uin) + VIR_Uniform_GetOffset(uniform));

                VSC_UF_AUBO_UniformInfoNode_SetLOADInst(uin, load);

                if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_TRANSFORM_INSERT))
                {
                    VIR_LOG(dumper, "insert load:\n");
                    VIR_LOG_FLUSH(dumper);
                    VIR_Inst_Dump(dumper, load);
                }
            }

            /* propagate load and change ldarr to mov */
            {
                VIR_Inst_SetOpcode(inst, VIR_OP_MOV);
                VIR_Inst_SetSrcNum(inst, 1);
                VIR_Operand_Copy(VIR_Inst_GetSource(inst, 0), loadDest);
                VIR_Operand_Change2Src(VIR_Inst_GetSource(inst, 0));
                VIR_Operand_SetSwizzle(operand, operandSwizzle);
                if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_TRANSFORM_INSERT))
                {
                    VIR_LOG(dumper, "propagate load and change ldarr to mov:\n");
                    VIR_LOG_FLUSH(dumper);
                    VIR_Inst_Dump(dumper, inst);
                }
            }
        }
        else
        {
            /* handle uniform usage different from others */
            gctUINT shaderKindId = VIR_ShaderKind_Map2KindId(VIR_Shader_GetKind(shader));

            if(VSC_UF_AUBO_UniformInfoNode_GetIndexSymId(uin) != VIR_INVALID_ID)
            {
                /* handle ldarr uniform usage different from others */
                VIR_SymId indexSymId = VIR_Operand_GetSymbolId_(VSC_UF_AUBO_UniformInfoNode_GetIndexOperand(uin));  /* because indexSymId may be a uniform, which is moved to DUBO already */
                VIR_Symbol* indexSym = VIR_Shader_GetSymFromId(shader, indexSymId);
                VIR_TypeId indexSymTypeId = VIR_Symbol_GetTypeId(indexSym);

                VSC_UF_AUBO_UniformInfoNode* sameIndex = VSC_UF_AUBO_UniformInfoNode_GetSameIndexList(uin);
                gctUINT strides[4];
                VIR_Swizzle swizzles[4];
                VIR_VirRegId lastSymId = indexSymId;
                VIR_Swizzle lastSwizzle;
                gctUINT i = 0;
                VIR_Enable lastEnable = VIR_ENABLE_X;
                gctBOOL needConst = gcvFALSE;
                VIR_Operand *loadDest;

                strides[0] = VSC_UF_AUBO_UniformInfoNode_GetStride(uin);
                swizzles[0] = VSC_UF_AUBO_UniformInfoNode_GetIndexSwizzle(uin);
                lastSwizzle = swizzles[0];

                if(sameIndex)
                {
                    for(i = 1; i < 4; i++)
                    {
                        gctUINT j;

                        strides[i] = VSC_UF_AUBO_UniformInfoNode_GetStride(sameIndex);
                        if(!needConst)
                        {
                            for(j = 0; j < i; j++)
                            {
                                if(strides[j] != strides[i])
                                {
                                    needConst = gcvTRUE;
                                    break;
                                }
                            }
                        }
                        swizzles[i] = VSC_UF_AUBO_UniformInfoNode_GetIndexSwizzle(sameIndex);
                        VIR_Swizzle_SetChannel(lastSwizzle, i, swizzles[i]);
                        lastEnable = (VIR_Enable)((gctUINT)lastEnable | 1 << i);
                        sameIndex = VSC_UF_AUBO_UniformInfoNode_GetSameIndexList(sameIndex);
                        if(sameIndex == gcvNULL)
                        {
                            break;
                        }
                    }
                    gcmASSERT(sameIndex == gcvNULL);
                }

                if(VIR_TypeId_isFloat(indexSymTypeId))
                {
                    VIR_Instruction* f2i;
                    VIR_Operand *f2iDest, *f2iSrc;
                    VIR_TypeId f2iDestTypeId = VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_UINT32, i + 1, 1);
                    VIR_VirRegId f2iRegId;
                    VIR_SymId f2iSymId;
                    VIR_Symbol* f2iSym;

                    VIR_Function_AddInstructionBefore(func, VIR_OP_F2I, f2iDestTypeId, insertBefore, gcvTRUE, &f2i);
                    f2iDest = VIR_Inst_GetDest(f2i);
                    f2iSrc = VIR_Inst_GetSource(f2i, 0);

                    f2iRegId = VIR_Shader_NewVirRegId(shader, 1);
                    virErrCode = VIR_Shader_AddSymbol(shader,
                                                        VIR_SYM_VIRREG,
                                                        f2iRegId,
                                                        VIR_Shader_GetTypeFromId(shader, f2iDestTypeId),
                                                        VIR_STORAGE_UNKNOWN,
                                                        &f2iSymId);
                    if(virErrCode != VSC_ERR_NONE) return virErrCode;

                    f2iSym = VIR_Shader_GetSymFromId(shader, f2iSymId);
                    VIR_Symbol_SetPrecision(f2iSym, VIR_PRECISION_HIGH);
                    VIR_Operand_SetTempRegister(f2iDest, func, f2iSymId, f2iDestTypeId);
                    VIR_Operand_SetEnable(f2iDest, lastEnable);

                    VIR_Operand_SetTempRegister(f2iSrc, func, lastSymId, f2iDestTypeId);
                    VIR_Operand_SetSwizzle(f2iSrc, lastSwizzle);
                    VSC_UF_AUBO_UniformInfoNode_SetF2IInst(uin, f2i);
                    lastSymId = f2iSymId;
                    lastSwizzle = VIR_Enable_2_Swizzle(lastEnable);

                    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_TRANSFORM_INSERT))
                    {
                        VIR_LOG(dumper, "insert f2i:\n");
                        VIR_LOG_FLUSH(dumper);
                        VIR_Inst_Dump(dumper, f2i);
                    }
                }

                if(VSC_OPTN_UF_AUBOOptions_GetOpt(options) == 2)
                {
                    /* insert load */
                    {
                        VIR_Instruction* load;
                        VIR_Operand *loadSrc0, *loadSrc1;
                        VIR_VirRegId loadRegId;
                        VIR_SymId loadSymId;
                        VIR_SymId addressSymId;
                        gctUINT lshift, mul3;

                        VIR_Function_AddInstructionBefore(func, VIR_OP_LOAD, uniformSymRegTypeId, insertBefore, gcvTRUE, &load);
                        loadDest = VIR_Inst_GetDest(load);
                        loadSrc0 = VIR_Inst_GetSource(load, 0);
                        loadSrc1 = VIR_Inst_GetSource(load, 1);

                        /* set loadDest */
                        loadRegId = VIR_Shader_NewVirRegId(shader, 1);
                        virErrCode = VIR_Shader_AddSymbol(shader,
                                                          VIR_SYM_VIRREG,
                                                          loadRegId,
                                                          uniformSymRegType,
                                                          VIR_STORAGE_UNKNOWN,
                                                          &loadSymId);
                        if(virErrCode != VSC_ERR_NONE) return virErrCode;

                        VIR_Operand_SetTempRegister(loadDest, func, loadSymId, operandTypeId);
                        VIR_Operand_SetEnable(loadDest, VIR_TypeId_Conv2Enable(uniformSymRegTypeId));

                        addressSymId = _VSC_UF_AUBO_GetAuxAddress(aubo, shader, uniformSym, VSC_UF_AUBO_UniformInfoNode_GetConstOffset(uin));
                        VIR_Operand_SetTempRegister(loadSrc0, func, addressSymId, VIR_TYPE_UINT32);
                        VIR_Operand_SetSwizzle(loadSrc0, VIR_Shader_IsEnableRobustCheck(shader) ? VIR_SWIZZLE_XYZZ : VIR_SWIZZLE_X);

                        _Extract_LShift_Mul3(VSC_UF_AUBO_UniformInfoNode_GetStride(uin), &lshift, &mul3);
                        VIR_Operand_SetTempRegister(loadSrc1, func, lastSymId, VIR_TYPE_UINT32);
                        VIR_Operand_SetSwizzle(loadSrc1, lastSwizzle);
                        VIR_Operand_SetLShift(loadSrc1, lshift);
                        VIR_Operand_SetModifier(loadSrc1, VIR_MOD_X3);

                        VSC_UF_AUBO_UniformInfoNode_SetLOADInst(uin, load);

                        if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_TRANSFORM_INSERT))
                        {
                            VIR_LOG(dumper, "insert load:\n");
                            VIR_LOG_FLUSH(dumper);
                            VIR_Inst_Dump(dumper, load);
                        }
                    }

                    /* propagate load and change ldarr to mov */
                    {
                        VIR_Inst_SetOpcode(inst, VIR_OP_MOV);
                        VIR_Inst_SetSrcNum(inst, 1);
                        VIR_Operand_Copy(VIR_Inst_GetSource(inst, 0), loadDest);
                        VIR_Operand_Change2Src(VIR_Inst_GetSource(inst, 0));
                        VIR_Operand_SetSwizzle(operand, operandSwizzle);
                        if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_TRANSFORM_INSERT))
                        {
                            VIR_LOG(dumper, "propagate load and change ldarr to mov:\n");
                            VIR_LOG_FLUSH(dumper);
                            VIR_Inst_Dump(dumper, inst);
                        }
                    }
                }
                else
                {
                    /* insert mad */
                    {
                        VIR_Instruction* mad;
                        VIR_Operand* madDest;
                        VIR_Operand* madSrc0;
                        VIR_Operand* madSrc1;
                        VIR_Operand* madSrc2;
                        VIR_TypeId madDestTypeId = VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_UINT32, i + 1, 1);
                        VIR_VirRegId madRegId;
                        VIR_SymId madSymId;
                        VIR_Symbol* madSym;

                        VIR_Function_AddInstructionBefore(func, VIR_OP_MAD, madDestTypeId, insertBefore, gcvTRUE, &mad);
                        madDest = VIR_Inst_GetDest(mad);
                        madSrc0 = VIR_Inst_GetSource(mad, 0);
                        madSrc1 = VIR_Inst_GetSource(mad, 1);
                        madSrc2 = VIR_Inst_GetSource(mad, 2);

                        /* set madDest */
                        madRegId = VIR_Shader_NewVirRegId(shader, 1);
                        virErrCode = VIR_Shader_AddSymbol(shader,
                                                            VIR_SYM_VIRREG,
                                                            madRegId,
                                                            VIR_Shader_GetTypeFromId(shader, madDestTypeId),
                                                            VIR_STORAGE_UNKNOWN,
                                                            &madSymId);
                        if(virErrCode != VSC_ERR_NONE) return virErrCode;

                        madSym = VIR_Shader_GetSymFromId(shader, madSymId);
                        VIR_Symbol_SetPrecision(madSym, VIR_PRECISION_HIGH);
                        VIR_Operand_SetTempRegister(madDest, func, madSymId, madDestTypeId);
                        VIR_Operand_SetEnable(madDest, lastEnable);

                        /* set madSrc0 */
                        VIR_Operand_SetTempRegister(madSrc0, func, lastSymId, madDestTypeId);
                        VIR_Operand_SetSwizzle(madSrc0, lastSwizzle);

                        /* set madSrc1 */
                        if(needConst)
                        {
                            VIR_ConstVal constVal;
                            VIR_ConstId constId;
                            gctUINT j;

                            for(j = 0; j <= i; j++)
                            {
                                constVal.vecVal.u32Value[j] = strides[j];
                            }

                            VIR_Shader_AddConstant(shader, madDestTypeId, &constVal, &constId);
                            VIR_Operand_SetConst(madSrc1, madDestTypeId, constId);
                        }
                        else
                        {
                            VIR_Operand_SetImmediateUint(madSrc1, strides[0]);
                        }

                        /* set madSrc2 */
                        if(isSymUniformMovedToCUBO(uniformSym))
                        {
                            VIR_SymId cuboAddrSymId = VSC_UF_AUBO_GetCUBOAddr(aubo, shaderKindId);
                            VIR_Symbol* cuboAddrSym = VIR_Shader_GetSymFromId(shader, cuboAddrSymId);
                            VIR_Operand_SetUniform(madSrc2, VIR_Symbol_GetUniform(cuboAddrSym), shader);
                        }
                        else
                        {
                            VIR_SymId duboAddrSymId;
                            VIR_Symbol* duboAddrSym;

                            gcmASSERT(isSymUniformMovedToDUBO(uniformSym));

                            duboAddrSymId = VSC_UF_AUBO_GetDUBOAddr(aubo, shaderKindId);
                            duboAddrSym = VIR_Shader_GetSymFromId(shader, duboAddrSymId);

                            VIR_Operand_SetUniform(madSrc2, VIR_Symbol_GetUniform(duboAddrSym), shader);
                        }
                        VIR_Operand_SetSwizzle(madSrc2, VIR_SWIZZLE_X);

                        lastSymId = madSymId;
                        lastSwizzle = VIR_Enable_2_Swizzle(lastEnable);
                        VSC_UF_AUBO_UniformInfoNode_SetMADInst(uin, mad);

                        if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_TRANSFORM_INSERT))
                        {
                            VIR_LOG(dumper, "insert mad:\n");
                            VIR_LOG_FLUSH(dumper);
                            VIR_Inst_Dump(dumper, mad);
                        }
                    }

                    /* insert load */
                    {
                        VIR_Instruction* load;
                        VIR_Operand *loadSrc0, *loadSrc1;
                        VIR_VirRegId loadRegId;
                        VIR_SymId loadSymId;

                        VIR_Function_AddInstructionBefore(func, VIR_OP_LOAD, uniformSymRegTypeId, insertBefore, gcvTRUE, &load);
                        loadDest = VIR_Inst_GetDest(load);
                        loadSrc0 = VIR_Inst_GetSource(load, 0);
                        loadSrc1 = VIR_Inst_GetSource(load, 1);

                        /* set loadDest */
                        loadRegId = VIR_Shader_NewVirRegId(shader, 1);
                        virErrCode = VIR_Shader_AddSymbol(shader,
                                                          VIR_SYM_VIRREG,
                                                          loadRegId,
                                                          uniformSymRegType,
                                                          VIR_STORAGE_UNKNOWN,
                                                          &loadSymId);
                        if(virErrCode != VSC_ERR_NONE) return virErrCode;

                        VIR_Operand_SetTempRegister(loadDest, func, loadSymId, operandTypeId);
                        VIR_Operand_SetEnable(loadDest, VIR_TypeId_Conv2Enable(uniformSymRegTypeId));

                        VIR_Operand_SetTempRegister(loadSrc0, func, lastSymId, VIR_TYPE_UINT32);
                        VIR_Operand_SetImmediateUint(loadSrc1, VSC_UF_AUBO_UniformInfoNode_GetConstOffset(uin) + VIR_Uniform_GetOffset(uniform));

                        VSC_UF_AUBO_UniformInfoNode_SetLOADInst(uin, load);

                        if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_TRANSFORM_INSERT))
                        {
                            VIR_LOG(dumper, "insert load:\n");
                            VIR_LOG_FLUSH(dumper);
                            VIR_Inst_Dump(dumper, load);
                        }
                    }

                    /* propagate load and change ldarr to mov */
                    {
                        VIR_Inst_SetOpcode(inst, VIR_OP_MOV);
                        VIR_Inst_SetSrcNum(inst, 1);
                        VIR_Operand_Copy(VIR_Inst_GetSource(inst, 0), loadDest);
                        VIR_Operand_Change2Src(VIR_Inst_GetSource(inst, 0));
                        VIR_Operand_SetSwizzle(operand, operandSwizzle);
                        if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_TRANSFORM_INSERT))
                        {
                            VIR_LOG(dumper, "propagate load and change ldarr to mov:\n");
                            VIR_LOG_FLUSH(dumper);
                            VIR_Inst_Dump(dumper, inst);
                        }
                    }
                }
            }
            else
            {
                /* handle normal uniform usage different from others */
                VIR_Instruction* load;
                VIR_Operand *loadDest, *loadSrc0, *loadSrc1;
                VIR_VirRegId loadRegId;
                VIR_SymId loadSymId;

                VIR_Function_AddInstructionBefore(func, VIR_OP_LOAD, VIR_Operand_GetTypeId(operand), insertBefore, gcvTRUE, &load);
                loadDest = VIR_Inst_GetDest(load);
                loadSrc0 = VIR_Inst_GetSource(load, 0);
                loadSrc1 = VIR_Inst_GetSource(load, 1);

                /* set loadDest */
                loadRegId = VIR_Shader_NewVirRegId(shader, 1);
                virErrCode = VIR_Shader_AddSymbol(shader,
                                                  VIR_SYM_VIRREG,
                                                  loadRegId,
                                                  operandType,
                                                  VIR_STORAGE_UNKNOWN,
                                                  &loadSymId);
                if(virErrCode != VSC_ERR_NONE) return virErrCode;

                VIR_Operand_SetTempRegister(loadDest, func, loadSymId, operandTypeId);
                VIR_Operand_SetEnable(loadDest, VIR_TypeId_Conv2Enable(uniformSymRegTypeId));

                /* set loadSrc0 */
                if(isSymUniformMovedToCUBO(uniformSym))
                {
                    VIR_SymId cuboAddrSymId = VSC_UF_AUBO_GetCUBOAddr(aubo, shaderKindId);
                    VIR_Symbol* cuboAddrSym = VIR_Shader_GetSymFromId(shader, cuboAddrSymId);
                    VIR_Operand_SetUniform(loadSrc0, VIR_Symbol_GetUniform(cuboAddrSym), shader);
                }
                else
                {
                    VIR_SymId duboAddrSymId;
                    VIR_Symbol* duboAddrSym;

                    gcmASSERT(isSymUniformMovedToDUBO(uniformSym));

                    duboAddrSymId = VSC_UF_AUBO_GetDUBOAddr(aubo, shaderKindId);
                    duboAddrSym = VIR_Shader_GetSymFromId(shader, duboAddrSymId);

                    VIR_Operand_SetUniform(loadSrc0, VIR_Symbol_GetUniform(duboAddrSym), shader);
                }
                VIR_Operand_SetSwizzle(loadSrc0, VIR_Shader_IsEnableRobustCheck(shader) ? VIR_SWIZZLE_XYZZ : VIR_SWIZZLE_X);

                /* set loadSrc1 */
                switch(VIR_Type_GetKind(uniformSymType))
                {
                    case VIR_TY_INVALID:
                    case VIR_TY_SCALAR:
                    case VIR_TY_VECTOR:
                        VIR_Operand_SetImmediateUint(loadSrc1, VIR_Uniform_GetOffset(uniform));
                        break;
                    case VIR_TY_MATRIX:
                    {
                        if(VIR_Operand_GetRelAddrMode(operand) == VIR_INDEXED_NONE)
                        {
                            VIR_TypeId base_typeid = VIR_Type_GetBaseTypeId(uniformSymType);
                            gctUINT const_index = VIR_Operand_GetMatrixConstIndex(operand);
                            VIR_TypeId row_typeid = VIR_GetTypeRowType(base_typeid);
                            gctUINT stride = VIR_GetTypeSize(row_typeid);
                            VIR_Operand_SetImmediateUint(loadSrc1, VIR_Uniform_GetOffset(uniform) + stride * const_index);
                        }
                        break;
                    }
                    case VIR_TY_ARRAY:
                    {
                        if(VIR_Operand_GetRelAddrMode(operand) == VIR_INDEXED_NONE)
                        {
                            VIR_TypeId base_typeid = VIR_Type_GetBaseTypeId(uniformSymType);
                            VIR_Type* base_type = VIR_Shader_GetTypeFromId(shader, base_typeid);
                            gctUINT const_index = VIR_Operand_GetRelIndexing(operand);
                            gctUINT mat_index = VIR_Operand_GetMatrixConstIndex(operand);
                            gctUINT stride;
                            if(VIR_Type_isMatrix(base_type))
                            {
                                VIR_TypeId row_typeid = VIR_GetTypeRowType(VIR_Type_GetIndex(base_type));
                                VIR_Type* row_type = VIR_Shader_GetTypeFromId(shader, row_typeid);
                                stride = VIR_Type_GetTypeByteSize(shader, row_type);
                            }
                            else
                            {
                                stride = VIR_Type_GetTypeByteSize(shader, base_type);
                            }
                            VIR_Operand_SetImmediateUint(loadSrc1, VIR_Uniform_GetOffset(uniform) + stride * (const_index + mat_index));
                        }
                        break;
                    }
                    default:
                    {
                        gcmASSERT(0);
                    }
                }

                {
                    VIR_Operand_Copy(operand, loadDest);
                    VIR_Operand_Change2Src(operand);
                    VIR_Operand_SetSwizzle(operand, operandSwizzle);
                }

                VSC_UF_AUBO_UniformInfoNode_SetLOADInst(uin, load);

                if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_TRANSFORM_INSERT))
                {
                    VIR_LOG(dumper, "insert load:\n");
                    VIR_LOG_FLUSH(dumper);
                    VIR_Inst_Dump(dumper, load);
                    VIR_LOG(dumper, "propagate load to inst:\n");
                    VIR_LOG_FLUSH(dumper);
                    VIR_Inst_Dump(dumper, inst);
                }
            }
        }
    }

    return virErrCode;
}

static VSC_ErrCode
_VSC_UF_AUBO_TransformInstructionsForBB(
    IN OUT VSC_UF_AUBO* aubo,
    IN OUT VIR_Shader* shader,
    IN OUT VIR_Function* func,
    IN OUT VIR_BB* bb
    )
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    VSC_UF_AUBO_UniformInfoList uil;

    if(BB_GET_LENGTH(bb))
    {
        _VSC_UF_AUBO_UniformInfoList_Initialize(&uil, VSC_UF_AUBO_GetMM(aubo));

        virErrCode = _VSC_UF_AUBO_BuildUpInfoList(aubo, shader, func, bb, &uil);
        if(virErrCode != VSC_ERR_NONE)
        {
            return virErrCode;
        }
        virErrCode = _VSC_UF_AUBO_ReorgUniformInfoList(aubo, shader, func, bb, &uil);
        if(virErrCode != VSC_ERR_NONE)
        {
            return virErrCode;
        }
        virErrCode = _VSC_UF_AUBO_InsertInstructions(aubo, shader, func, bb, &uil);
        if(virErrCode != VSC_ERR_NONE)
        {
            return virErrCode;
        }

        _VSC_UF_AUBO_UniformInfoList_Finalize(&uil);
    }

    return virErrCode;
}

static VSC_ErrCode
_VSC_UF_AUBO_TransformInstructionsForFunctionLocally(
    IN OUT VSC_UF_AUBO* aubo,
    IN OUT VIR_Shader* shader,
    IN OUT VIR_Function* func
    )
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    VIR_CONTROL_FLOW_GRAPH* cfg = VIR_Function_GetCFG(func);
    CFG_ITERATOR cfgIter;
    VIR_BASIC_BLOCK* bb;

    gcmASSERT(cfg);
    CFG_ITERATOR_INIT(&cfgIter, cfg);
    bb = CFG_ITERATOR_FIRST(&cfgIter);
    gcmASSERT(bb);
    for(; bb != gcvNULL; bb = CFG_ITERATOR_NEXT(&cfgIter))
    {
        virErrCode = _VSC_UF_AUBO_TransformInstructionsForBB(aubo, shader, func, bb);
        if(virErrCode != VSC_ERR_NONE)
        {
            break;
        }
    }

    return virErrCode;
}

static VSC_ErrCode
_VSC_UF_AUBO_TransformInstructionsForShader(
    IN OUT VSC_UF_AUBO* aubo,
    IN OUT VIR_Shader* shader
    )
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    VIR_FuncIterator func_iter;
    VIR_FunctionNode* func_node;
    VSC_OPTN_UF_AUBOOptions* options = VSC_UF_AUBO_GetOptions(aubo);

    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_INPUT))
    {
        VIR_Shader_Dump(gcvNULL, "CreateAUBO Input", shader, gcvTRUE);
    }

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(shader));
    /* iterate over functions */
    for(func_node = VIR_FuncIterator_First(&func_iter); func_node != gcvNULL;
        func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function* func = func_node->function;

        virErrCode = _VSC_UF_AUBO_TransformInstructionsForFunctionLocally(aubo, shader, func);
        if(virErrCode != VSC_ERR_NONE)
        {
            break;
        }
    }

    if(VSC_UF_AUBO_GetAuxAddresses(aubo))
    {
        VSC_HASH_TABLE* auxAddrTable = VSC_UF_AUBO_GetAuxAddresses(aubo);
        VSC_HASH_ITERATOR iter;
        VSC_UF_AUBO_GetAuxAddress_Key* key;

        vscHTBLIterator_Init(&iter, auxAddrTable);
        for(key = (VSC_UF_AUBO_GetAuxAddress_Key*)vscHTBLIterator_First(&iter);
            key != gcvNULL;
            key = (VSC_UF_AUBO_GetAuxAddress_Key*)vscHTBLIterator_Next(&iter))
        {
            vscMM_Free(VSC_UF_AUBO_GetMM(aubo), key);
        }
        vscMM_Free(VSC_UF_AUBO_GetMM(aubo), auxAddrTable);
        VSC_UF_AUBO_SetAuxAddresses(aubo, gcvNULL);
    }

    if(VSC_UTILS_MASK(VSC_OPTN_UF_AUBOOptions_GetTrace(options), VSC_OPTN_UF_AUBOOptions_TRACE_OUTPUT))
    {
        VIR_Shader_Dump(gcvNULL, "CreateAUBO Output", shader, gcvTRUE);
    }

    return virErrCode;
}

static VSC_ErrCode
_VSC_UF_AUBO_TransformInstructionsForShaders(
    IN OUT VSC_UF_AUBO* aubo
    )
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    VSC_AllShaders* all_shaders = VSC_UF_AUBO_GetAllShaders(aubo);
    gctUINT i;

    for(i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
    {
        if(VSC_UF_AUBO_GetDUBOItemCount(aubo, i) || VSC_UF_AUBO_GetCUBOItemCount(aubo, i))
        {
            VIR_Shader* shader = VSC_AllShaders_GetShader(all_shaders, i);

            virErrCode = _VSC_UF_AUBO_TransformInstructionsForShader(aubo, shader);
            if(virErrCode != VSC_ERR_NONE)
            {
                break;
            }
        }
    }

    return virErrCode;
}


extern VIR_UniformKind _VIR_CG_ResType2UniformKind(VSC_SHADER_RESOURCE_TYPE    resType);
extern VIR_Uniform* _VIR_CG_FindResUniform(IN VIR_Shader           *pShader,
                                           IN VIR_UniformKind      uniformKind,
                                           IN gctUINT              setOrOffset,
                                           IN gctUINT              binding,
                                           IN gctUINT              arraySize);

static void _CheckPerShaderReservation(IN VSC_AllShaders           *all_shaders,
                                       IN VSC_PROGRAM_RESOURCE_LAYOUT  *pgResourceLayout,
                                       IN OUT VSC_UF_AUBO* aubo)
{
    gctUINT i, k, j, newReservedCount, stage;

    for (k = 0; k < pgResourceLayout->resourceSetCount; k++)
    {
        VSC_PROGRAM_RESOURCE_SET* set = &pgResourceLayout->pResourceSets[k];

        for (j = 0; j < set->resourceBindingCount; j ++)
        {
            VSC_SHADER_RESOURCE_BINDING resBinding = set->pResouceBindings[j].shResBinding;

            for(i = 0; i < VSC_MAX_LINKABLE_SHADER_STAGE_COUNT; i++)
            {
                VIR_Shader* shader = VSC_AllShaders_GetShader(all_shaders, i);

                if (shader)
                {
                    stage = i;

                    if (shader->shaderKind == VIR_SHADER_COMPUTE)
                    {
                        stage = VSC_SHADER_STAGE_CS;
                    }

                    if (set->pResouceBindings[j].stageBits & VSC_SHADER_STAGE_2_STAGE_BIT(stage))
                    {
                        if (_VIR_CG_FindResUniform(shader,
                                                   _VIR_CG_ResType2UniformKind(resBinding.type),
                                                   resBinding.set,
                                                   resBinding.binding,
                                                   resBinding.arraySize) == gcvNULL)
                        {
                            newReservedCount = VSC_UF_AUBO_GetPerShRsvedCount(aubo, i) + resBinding.arraySize;
                            VSC_UF_AUBO_SetPerShRsvedCount(aubo, i, newReservedCount);
                        }
                    }
                }
            }
        }
    }
}

VSC_ErrCode VSC_UF_UtilizeAuxUBO(
    IN OUT VSC_AllShaders           *all_shaders,
    IN VSC_HW_CONFIG                *hwCfg,
    IN VSC_PROGRAM_RESOURCE_LAYOUT  *pgResourceLayout,
    IN VSC_OPTN_UF_AUBOOptions      *options,
    IN OUT gctBOOL                  *trans
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctUINT i;
    VSC_UF_AUBO aubo;
    VIR_Dumper *dumper = VSC_AllShaders_GetDumper(all_shaders);

    /* verify whether hardware configuration supports default UBO */
    if(!hwCfg->hwFeatureFlags.hasHalti1)
    {
        if(VSC_OPTN_UF_AUBOOptions_GetTrace(options))
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
            if (!VSC_OPTN_InRange(VIR_Shader_GetId(shader), VSC_OPTN_UF_AUBOOptions_GetBeforeShader(options),
                                  VSC_OPTN_UF_AUBOOptions_GetAfterShader(options)))
            {
                if(VSC_OPTN_UF_AUBOOptions_GetTrace(options))
                {
                    VIR_LOG(dumper, "Default UBO skips shader(id=%d)\n", VIR_Shader_GetId(shader));
                    VIR_LOG_FLUSH(dumper);
                }
                return errCode;
            }
            if(VIR_Shader_GetClientApiVersion(shader) < gcvAPI_OPENGL_ES20)
            {
                if(VSC_OPTN_UF_AUBOOptions_GetTrace(options))
                {
                    VIR_LOG(dumper, "Default UBO skips shader(id=%d) because it's not an es20 or above shader\n", VIR_Shader_GetId(shader));
                    VIR_LOG_FLUSH(dumper);
                }
                return errCode;
            }
            if(VIR_Shader_IsOpenVG(shader))
            {
                if(VSC_OPTN_UF_AUBOOptions_GetTrace(options))
                {
                    VIR_LOG(dumper, "Default UBO skips shader(id=%d) because it's an OpenVG shader\n", VIR_Shader_GetId(shader));
                    VIR_LOG_FLUSH(dumper);
                }
                return errCode;
            }
        }
    }

    if(VSC_OPTN_UF_AUBOOptions_GetTrace(options))
    {
        VIR_LOG(dumper, "Default UBO starts for program\n");
        VIR_LOG_FLUSH(dumper);
    }

    _VSC_UF_AUBO_Initialize(&aubo, all_shaders, hwCfg, options);

    if (pgResourceLayout)
    {
        _CheckPerShaderReservation(all_shaders, pgResourceLayout, &aubo);
    }

    {
        gctBOOL has_active = gcvFALSE;
        _VSC_UF_AUBO_CollectUniformsInfo(&aubo, &has_active);
        if(!has_active)
        {
            if(VSC_OPTN_UF_AUBOOptions_GetTrace(options))
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
    if(hwCfg->hwFeatureFlags.hasUniformB0)
    {
        VSC_OPTN_UF_AUBOOptions_SetIndirectAccessLevel(options, VSC_IndirectAccessLevel_UNIFORM_DYNAMICALLY);
    }

    if(VSC_OPTN_UF_AUBOOptions_GetIndirectAccessLevel(options))
    {
        _VSC_UF_AUBO_CollectIndirectlyAccessedUniforms(&aubo, (VSC_IndirectAccessLevel)VSC_OPTN_UF_AUBOOptions_GetIndirectAccessLevel(options));
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
        if(VSC_OPTN_UF_AUBOOptions_GetOpt(options))
        {
            _VSC_UF_AUBO_TransformInstructionsForShaders(&aubo);
        }
        else
        {
            _VSC_UF_AUBO_TransformInstructions(&aubo);
        }
        if (trans)
        {
            *trans = gcvTRUE; /*invalid cfg if code changed*/
        }
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

    symUniform = VIR_Symbol_GetUniformPointer(pShader, sym);

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
        symUniform = VIR_Symbol_GetUniformPointer(pShader, sym);

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
    VIR_Symbol         *sym;
    VIR_Uniform        *uniform = gcvNULL;

    if (!opnd) return;

    if (VIR_Operand_GetOpKind(opnd) == VIR_OPND_SYMBOL)
    {
        sym = VIR_Operand_GetSymbol(opnd);

        uniform = VIR_Symbol_GetUniformPointer(pShader, sym);

        if (uniform)
        {
            VIR_Type    *symType = VIR_Symbol_GetType(sym);

            if (InLTC)
            {
                VIR_Symbol_SetFlag(sym, VIR_SYMUNIFORMFLAG_USED_IN_LTC);
            }
            else
            {
                VIR_Symbol_SetFlag(sym, VIR_SYMUNIFORMFLAG_USED_IN_SHADER);
            }

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

                    if (VIR_Symbol_isSampler(sym))
                    {
                        rows = 1;
                    }
                    else
                    {
                        rows = VIR_Type_GetVirRegCount(pShader, baseType, -1);
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
                    if (thisArraySize > VIR_Uniform_GetRealUseArraySize(uniform))
                    {
                        VIR_Uniform_SetRealUseArraySize(uniform, thisArraySize);
                    }
                }
            }
        }
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
    for(i = 0; i < VIR_IdList_Count(VIR_Shader_GetUniforms(pShader)); ++i)
    {
        /* need to fix here */
        VIR_Id      id  = VIR_IdList_GetId(VIR_Shader_GetUniforms(pShader), i);
        VIR_Symbol *sym = VIR_Shader_GetSymFromId(pShader, id);

        if (isSymUniformCompiletimeInitialized(sym) &&
            VIR_Symbol_GetUniformKind(sym) != VIR_UNIFORM_TEMP_REG_SPILL_MEM_ADDRESS)
        {
            VIR_Symbol_ClrFlag(sym, VIR_SYMUNIFORMFLAG_USED_IN_SHADER);
        }

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
            VIR_SrcOperand_Iterator srcOpndIter;
            VIR_Operand*            srcOpnd;

            VIR_SrcOperand_Iterator_Init(inst, &srcOpndIter);
            srcOpnd = VIR_SrcOperand_Iterator_First(&srcOpndIter);

            for (; srcOpnd != gcvNULL; srcOpnd = VIR_SrcOperand_Iterator_Next(&srcOpndIter))
            {
                VSC_CheckOpndUniformUsage(pShader, inst, srcOpnd, gcvFALSE);
            }
        }
    }

    /* check usage in LTC: VIV:TODO: LTC is not in VIR yet */

    for(i = 0; i < VIR_IdList_Count(VIR_Shader_GetUniforms(pShader)); ++i)
    {
        VIR_Id      id  = VIR_IdList_GetId(VIR_Shader_GetUniforms(pShader), i);
        VIR_Symbol *sym = VIR_Shader_GetSymFromId(pShader, id);
        VIR_Uniform*symUniform = VIR_Symbol_GetUniformPointer(pShader, sym);
        VIR_Type   *symType = VIR_Symbol_GetType(sym);

        if (VIR_Shader_IsEnableRobustCheck(pShader))
        {
            VIR_Shader_ChangeAddressUniformTypeToFatPointer(pShader, sym);
        }
        if (VIR_Uniform_GetRealUseArraySize(symUniform) == -1)
        {
            if (VIR_Type_GetKind(symType) == VIR_TY_ARRAY)
            {
                VIR_Uniform_SetRealUseArraySize(symUniform, VIR_Type_GetArrayLength(symType));
            }
            else
            {
                VIR_Uniform_SetRealUseArraySize(symUniform, 1);
            }
        }
    }

    return retValue;
}

DEF_QUERY_PASS_PROP(VSC_UF_CreateAUBO)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_MC;
    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_AUBO;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;
    pPassProp->passFlag.resCreationReq.s.bNeedCfg = gcvTRUE;
}

DEF_GPG_NECESSITY_CHECK(VSC_UF_CreateAUBO)
{
    return gcvTRUE;
}

VSC_ErrCode
VSC_UF_CreateAUBO(
    IN VSC_GPG_PASS_WORKER* pPassWorker
    )
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VSC_AllShaders             all_shaders;
    VSC_OPTN_UF_AUBOOptions*   aubo_options = (VSC_OPTN_UF_AUBOOptions*)pPassWorker->basePassWorker.pBaseOption;
    gctBOOL                    trans = gcvFALSE;

    VSC_AllShaders_Initialize(&all_shaders,
                              (VIR_Shader*)pPassWorker->pPgmLinkerParam->hShaderArray[VSC_SHADER_STAGE_VS],
                              (VIR_Shader*)pPassWorker->pPgmLinkerParam->hShaderArray[VSC_SHADER_STAGE_HS],
                              (VIR_Shader*)pPassWorker->pPgmLinkerParam->hShaderArray[VSC_SHADER_STAGE_DS],
                              (VIR_Shader*)pPassWorker->pPgmLinkerParam->hShaderArray[VSC_SHADER_STAGE_GS],
                              (VIR_Shader*)pPassWorker->pPgmLinkerParam->hShaderArray[VSC_SHADER_STAGE_PS],
                              (VIR_Shader*)pPassWorker->pPgmLinkerParam->hShaderArray[VSC_SHADER_STAGE_CS],
                              pPassWorker->basePassWorker.pDumper,
                              pPassWorker->basePassWorker.pMM,
                              &pPassWorker->pPgmLinkerParam->cfg
                              );

    /* Create Default UBO now */
    errCode = VSC_AllShaders_LinkUniforms(&all_shaders);
    ON_ERROR(errCode, "Link uniforms");

    errCode = VSC_UF_UtilizeAuxUBO(&all_shaders,
                                   &pPassWorker->pPgmLinkerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg,
                                   pPassWorker->pPgmLinkerParam->pPgResourceLayout,
                                   aubo_options, &trans);

    ON_ERROR(errCode, "utilize default UBO");
    if (trans)
    {
        gctUINT i;
        for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
        {
            pPassWorker->resDestroyReqArray[i].s.bInvalidateCfg = gcvTRUE;
        }
    }
OnError:
    return errCode;
}

extern VSC_ErrCode VIR_CG_Unified_MapUniforms(
    IN OUT VSC_AllShaders   *all_shaders,
    IN VSC_HW_CONFIG        *pHwConfig
    );

DEF_QUERY_PASS_PROP(VSC_UF_UnifiedUniformAlloc)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_MC;
    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_UNIFIED_UNIFORM;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;
    pPassProp->passFlag.resCreationReq.s.bNeedCfg = gcvTRUE;
}

DEF_GPG_NECESSITY_CHECK(VSC_UF_UnifiedUniformAlloc)
{
    /*
    ** Don't do unified uniform allocation for PPO, it may change the uniform physical address.
    */
    if (pPassWorker->pPgmLinkerParam->cfg.cFlags & VSC_COMPILER_FLAG_LINK_PROGRAM_PIPELINE_OBJ)
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

VSC_ErrCode
VSC_UF_UnifiedUniformAlloc(
    IN VSC_GPG_PASS_WORKER* pPassWorker
    )
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VSC_AllShaders             all_shaders;

    VSC_AllShaders_Initialize(&all_shaders,
                              (VIR_Shader*)pPassWorker->pPgmLinkerParam->hShaderArray[VSC_SHADER_STAGE_VS],
                              (VIR_Shader*)pPassWorker->pPgmLinkerParam->hShaderArray[VSC_SHADER_STAGE_HS],
                              (VIR_Shader*)pPassWorker->pPgmLinkerParam->hShaderArray[VSC_SHADER_STAGE_DS],
                              (VIR_Shader*)pPassWorker->pPgmLinkerParam->hShaderArray[VSC_SHADER_STAGE_GS],
                              (VIR_Shader*)pPassWorker->pPgmLinkerParam->hShaderArray[VSC_SHADER_STAGE_PS],
                              (VIR_Shader*)pPassWorker->pPgmLinkerParam->hShaderArray[VSC_SHADER_STAGE_CS],
                              pPassWorker->basePassWorker.pDumper,
                              pPassWorker->basePassWorker.pMM,
                              &pPassWorker->pPgmLinkerParam->cfg
                              );

    /* Link uniforms. */
    errCode = VSC_AllShaders_LinkUniforms(&all_shaders);
    ON_ERROR(errCode, "Link uniforms");

    /* For unified uniform (i.e., multi-cluster), use all_shaders to allocate uniforms. */
    if (!pPassWorker->pPgmLinkerParam->pPgResourceLayout)
    {
        errCode = VIR_CG_Unified_MapUniforms(&all_shaders,
                                             &pPassWorker->pPgmLinkerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg);
        ON_ERROR(errCode, "VIR_CG_Unified_MapUniforms");
    }
    else
    {
    }
    ON_ERROR(errCode, "unified allocate uniform");

OnError:
    return errCode;
}

DEF_QUERY_PASS_PROP(VSC_UF_CreateAUBOForCLShader)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_MC;
    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_AUBO;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;

    pPassProp->passFlag.resCreationReq.s.bNeedCfg = gcvTRUE;
}

DEF_SH_NECESSITY_CHECK(VSC_UF_CreateAUBOForCLShader)
{
    VIR_Shader*    pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    if ((pShader->clientApiVersion == gcvAPI_OPENCL) &&
        (VIR_Shader_GetKind(pShader) == VIR_SHADER_COMPUTE) &&
        (pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasUniformB0))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

/* AUBO pass for openCL Compute shader */
VSC_ErrCode VSC_UF_CreateAUBOForCLShader(
    IN VSC_SH_PASS_WORKER* pPassWorker
    )
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;
    VIR_Shader      *pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VSC_AllShaders  all_shaders;
    VSC_OPTN_UF_AUBOOptions* aubo_options = (VSC_OPTN_UF_AUBOOptions*)pPassWorker->basePassWorker.pBaseOption;
    gctBOOL         trans = gcvFALSE;

    gcmASSERT((pShader->clientApiVersion == gcvAPI_OPENCL) &&
              (VIR_Shader_GetKind(pShader) == VIR_SHADER_COMPUTE));

    VSC_AllShaders_Initialize(&all_shaders, gcvNULL, gcvNULL, gcvNULL, gcvNULL, gcvNULL,
                             pShader, pPassWorker->basePassWorker.pDumper, pPassWorker->basePassWorker.pMM,
                             &pPassWorker->pCompilerParam->cfg);

    VSC_AllShaders_LinkUniforms(&all_shaders);

    /* Create Default UBO*/
    if (VSC_OPTN_UF_AUBOOptions_GetSwitchOn(aubo_options))
    {
        VSC_UF_UtilizeAuxUBO(&all_shaders, &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg, gcvNULL, aubo_options, &trans);
    }

    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(pShader), VIR_Shader_GetId(pShader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "After VSC_UF_CreateAUBOForCLShader", pShader, gcvTRUE);
    }

    pPassWorker->pResDestroyReq->s.bInvalidateCfg = trans;
    return errCode;
}

