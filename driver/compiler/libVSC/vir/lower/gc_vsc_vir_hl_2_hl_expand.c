/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "vir/lower/gc_vsc_vir_hl_2_hl.h"

#include "vir/lower/gc_vsc_vir_lower_common_func.h"

#define __MAX_SYM_NAME_LENGTH__     128

static void
_Lower_Initialize(
    IN VIR_Shader               *Shader,
    IN VSC_MM                   *pMM,
    IN VIR_PatternHL2HLContext  *Context
    )
{
    gcoOS_ZeroMemory(Context, sizeof(VIR_PatternHL2HLContext));

    Context->header.shader = Shader;

    /* Save the PMM. */
    Context->pMM = pMM;
}

static void
_Lower_Finalize(
    IN VIR_Shader               *Shader,
    IN VIR_PatternHL2HLContext  *Context
    )
{
}

static VSC_ErrCode
_SplitStructVariable(
    IN  VIR_Shader              *Shader,
    IN  VIR_Symbol              *ParentSym,
    IN  VIR_Symbol              *StructSym,
    IN  VIR_SymbolKind           SymbolKind,
    IN  VIR_StorageClass         StorageClass,
    IN  gctSTRING                PrefixName,
    IN  VIR_Type                *Type,
    IN  VIR_SymId                BlockIndex,
    IN  VIR_SymFlag              SymFlag,
    IN  gctBOOL                  SplitArray,
    IN  gctBOOL                  AllocateSym,
    IN  gctBOOL                  AllocateReg,
    IN  gctUINT                 *UpcomingRegCount,
    OUT gctINT                  *Location,
    OUT VIR_SymId               *FirstElementId,
    OUT VIR_IdList              *IdList
    );

static VSC_ErrCode
_SplitArrayVariable(
    IN  VIR_Shader              *Shader,
    IN  VIR_Symbol              *ParentSym,
    IN  VIR_Symbol              *ArraySym,
    IN  VIR_Type                *Type,
    IN  VIR_SymbolKind           SymbolKind,
    IN  VIR_StorageClass         StorageClass,
    IN  gctSTRING                PrefixName,
    IN  VIR_SymId                BlockIndex,
    IN  VIR_SymFlag              SymFlag,
    IN  gctBOOL                  SplitArray,
    IN  gctBOOL                  AllocateSym,
    IN  gctBOOL                  AllocateReg,
    IN  gctUINT                 *UpcomingRegCount,
    OUT gctINT                  *Location,
    OUT VIR_SymId               *FirstElementId,
    OUT VIR_IdList              *IdList
    );

static gctINT
_EvaluateLocation(
    IN  VIR_Shader              *Shader,
    IN  VIR_Symbol              *Symbol,
    IN  VIR_Type                *Type,
    IN  VIR_SymbolKind           VariableKind,
    IN  VIR_StorageClass         StorageClass,
    OUT gctINT                  *Location
    )
{
    gctINT                       location = *Location;

    /* TODO: we may need to handle different variable kind. */
    location += 1;

    *Location = location;

    return location;
}

static VSC_ErrCode
_AddGeneralVariable(
    IN  VIR_Shader              *Shader,
    IN  VIR_Symbol              *ParentSym,
    IN  VIR_Symbol              *Symbol,
    IN  VIR_Type                *Type,
    IN  VIR_SymbolKind           VariableKind,
    IN  VIR_StorageClass         StorageClass,
    IN  VIR_SymId                BlockIndex,
    IN  VIR_SymFlag              SymFlag,
    IN  gctBOOL                  AllocateSym,
    IN  gctBOOL                  AllocateReg,
    IN  gctUINT                 *UpcomingRegCount,
    IN  gctSTRING                Name,
    OUT gctINT                  *Location,
    OUT VIR_SymId               *SymId,
    OUT VIR_IdList              *IdList
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_SymId                    symId, regSymId;
    VIR_Function                *func = gcvNULL;
    VIR_Symbol                  *sym = gcvNULL;
    VIR_Symbol                  *regSym = gcvNULL;
    VIR_Type                    *type = (Type != gcvNULL) ? Type : VIR_Symbol_GetType(Symbol);
    VIR_NameId                   nameId;
    VIR_VirRegId                 regId;
    VIR_SymFlag                  flags = VIR_Symbol_GetFlag(Symbol);
    gctSTRING                    name = Name;
    gctINT                       location = -1;
    gctUINT                      i;
    gctUINT                      regCount = VIR_Type_GetVirRegCount(Shader, type, -1);

    /* The parent symbol is a parameter. */
    if (VIR_Id_isFunctionScope(VIR_Symbol_GetIndex(ParentSym)))
    {
        func = VIR_Symbol_GetHostFunction(ParentSym);
    }

    /* Allocate symbol for this variable if needed. */
    if (AllocateSym)
    {
        if (gcmIS_SUCCESS(gcoOS_StrNCmp(Name, "gl_out", 6)) ||
            gcmIS_SUCCESS(gcoOS_StrNCmp(Name, "gl_PerVertex", 12)))
        {
            gcoOS_StrStr(Name, ".", &name);
            gcmASSERT(name != gcvNULL);
            name = name + 1;
        }
        /*
        ** If a shader is generated from a SPIR-V assembly, then the builtin names could be hidden by a non-builtin block name,
        ** so we may need to ignore the block name.
        ** TODO: We need to move those checks to SPIR-V converter.
        */
        else if (!gcmIS_SUCCESS(gcoOS_StrNCmp(Name, "gl_", 3))
                 &&
                 gcoOS_StrStr(Name, "gl_Position", &name))
        {
            if (StorageClass == VIR_STORAGE_INPUT)
            {
                name = "gl_in.gl_Position";
            }
            gcmASSERT(name != gcvNULL);
        }
        else if (!gcmIS_SUCCESS(gcoOS_StrNCmp(Name, "gl_", 3))
                 &&
                 gcoOS_StrStr(Name, "gl_PointSize", &name))
        {
            if (StorageClass == VIR_STORAGE_INPUT)
            {
                name = "gl_in.gl_PointSize";
            }
            gcmASSERT(name != gcvNULL);
        }
        else
        {
            name = Name;
        }

        if (func)
        {
            errCode = VIR_Function_AddParameter(func,
                                                name,
                                                VIR_Type_GetIndex(type),
                                                StorageClass,
                                                &symId);
            CHECK_ERROR(errCode, "VIR_Function_AddParameter failed.");
            sym = VIR_Function_GetSymFromId(func, symId);
        }
        else
        {
            /* Add name string. */
            errCode = VIR_Shader_AddString(Shader,
                                           name,
                                           &nameId);
            CHECK_ERROR(errCode, "VIR_Shader_AddString failed.");

            /* Add variable symbol. */
            errCode = VIR_Shader_AddSymbol(Shader,
                                           VariableKind,
                                           nameId,
                                           type,
                                           StorageClass,
                                           &symId);
            CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed.");
            sym = VIR_Shader_GetSymFromId(Shader, symId);
        }
        gcmASSERT(sym);

        if (Location)
        {
            location = *Location;
        }
        VIR_Symbol_SetLocation(sym, location);
        if (Location)
        {
            _EvaluateLocation(Shader,
                Symbol,
                Type,
                VariableKind,
                StorageClass,
                Location);
        }
        VIR_Symbol_SetPrecision(sym, VIR_Symbol_GetPrecision(Symbol));

        switch (VariableKind)
        {
        case VIR_SYM_UNIFORM:
        case VIR_SYM_SAMPLER:
        case VIR_SYM_IMAGE:
            {
                VIR_Uniform             *virUniform;
                gctUINT storageClass = (gctUINT)StorageClass;
                /* Set the sym info. */
                VIR_Symbol_SetPrecision(sym, VIR_Symbol_GetPrecision(Symbol));
                VIR_Symbol_SetAddrSpace(sym, VIR_AS_CONSTANT);
                VIR_Symbol_SetTyQualifier(sym, VIR_TYQUAL_CONST);
                if (BlockIndex != VIR_INVALID_ID)
                {
                    VIR_Symbol_SetUniformKind(sym, VIR_UNIFORM_BLOCK_MEMBER);
                }
                else if(storageClass == VIR_UNIFORM_PUSH_CONSTANT)
                {
                    VIR_FieldInfo *fInfo = VIR_Symbol_GetFieldInfo(Symbol);
                    VIR_Symbol_SetUniformKind(sym, VIR_UNIFORM_PUSH_CONSTANT);
                    VIR_Symbol_SetLayoutOffset(sym, VIR_FieldInfo_GetOffset(fInfo));
                    flags = flags | VIR_SYMUNIFORMFLAG_USED_IN_SHADER;
                }
                else
                {
                    VIR_Symbol_SetUniformKind(sym, VIR_UNIFORM_NORMAL);
                }
                VIR_Symbol_SetFlag(sym, flags | SymFlag);
                VIR_Symbol_ClrFlag(sym, VIR_SYMFLAG_WITHOUT_REG);

                /* Set the uniform sym info.*/
                virUniform = sym->u2.uniform;
                gcmASSERT(virUniform);

                virUniform->sym = symId;
                /* TODO: we need to set the correct value for those -1. */
                if (BlockIndex != VIR_INVALID_ID)
                {
                    virUniform->blockIndex = (gctINT16)BlockIndex;
                }
                else
                {
                    virUniform->blockIndex = -1;
                }
                virUniform->gcslIndex = -1;
                virUniform->lastIndexingIndex = -1;
                virUniform->glUniformIndex = (gctUINT16)-1;
                VIR_Uniform_SetOrigPhysical(virUniform, -1);
                VIR_Uniform_SetPhysical(virUniform, -1);
                VIR_Uniform_SetSamplerPhysical(virUniform, -1);
                virUniform->swizzle = (gctUINT8)-1;
                virUniform->address = (gctUINT32)-1;
                virUniform->imageSamplerIndex = (gctUINT16)-1;
                virUniform->offset = 0;
                virUniform->baseBindingUniform = VIR_INVALID_ID;
                /* TODO: we need to set layout info. */
                break;
            }

        case VIR_SYM_VARIABLE:
            {
                VIR_Symbol_SetFlag(sym, flags | SymFlag);
                VIR_Symbol_ClrFlag(sym, VIR_SYMFLAG_WITHOUT_REG);
                if (BlockIndex != VIR_INVALID_ID)
                {
                    sym->ioBlockIndex = BlockIndex;
                }
                break;
            }

        default:
            break;
        }
    }
    else
    {
        sym = Symbol;
        symId = VIR_Symbol_GetIndex(sym);
    }

    /* Allocate temp register for this variable if needed. */
    if (AllocateReg)
    {
        regId = VIR_Shader_NewVirRegId(Shader, regCount);
        VIR_Symbol_SetVariableVregIndex(sym, regId);
        if(*UpcomingRegCount)
        {
            VIR_Symbol_SetIndexRange(sym, regId + *UpcomingRegCount);
            (*UpcomingRegCount) -= regCount;
        }

        for (i = 0; i < regCount; i++)
        {
            errCode = VIR_Shader_AddSymbol(Shader,
                                           VIR_SYM_VIRREG,
                                           regId + i,
                                           VIR_Type_GetRegIndexType(Shader, type, regId),
                                           VIR_STORAGE_UNKNOWN,
                                           &regSymId);
            CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed.");

            regSym = VIR_Shader_GetSymFromId(Shader, regSymId);
            gcmASSERT(regSym);

            VIR_Symbol_SetVregVariable(regSym, sym);
            if (func)
            {
                VIR_Symbol_SetStorageClass(regSym, StorageClass);
                VIR_Symbol_SetParamFuncSymId(regSym, VIR_Function_GetSymId(func));
            }
            VIR_Symbol_SetPrecision(regSym, VIR_Symbol_GetPrecision(sym));
        }
    }

    if (SymId)
    {
        *SymId = symId;
    }

    if (IdList)
    {
        VIR_IdList_Add(IdList, symId);
    }

    return errCode;
}

static VSC_ErrCode
_SplitStructVariable(
    IN  VIR_Shader              *Shader,
    IN  VIR_Symbol              *ParentSym,
    IN  VIR_Symbol              *StructSym,
    IN  VIR_SymbolKind           SymbolKind,
    IN  VIR_StorageClass         StorageClass,
    IN  gctSTRING                PrefixName,
    IN  VIR_Type                *Type,
    IN  VIR_SymId                BlockIndex,
    IN  VIR_SymFlag              SymFlag,
    IN  gctBOOL                  SplitArray,
    IN  gctBOOL                  AllocateSym,
    IN  gctBOOL                  AllocateReg,
    IN  gctUINT                 *UpcomingRegCount,
    OUT gctINT                  *Location,
    OUT VIR_SymId               *FirstElementId,
    OUT VIR_IdList              *IdList
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_SymIdList               *fields = VIR_Type_GetFields(Type);
    VIR_Id                       id;
    VIR_SymId                    firstElementId = VIR_INVALID_ID;
    VIR_Symbol                  *fieldSym;
    VIR_Type                    *fieldType;
    VIR_SymbolKind               fieldKind;
    VIR_TypeId                   baseTypeId;
    VIR_SymId                    symId;
    gctUINT                      i;
    gctSTRING                    fieldName;
    gctCHAR                      prevName[__MAX_SYM_NAME_LENGTH__];
    gctCHAR                      mixName[__MAX_SYM_NAME_LENGTH__];

    gcmASSERT(PrefixName);

    /* Generate name: 'prefixName.' */
    gcmASSERT(gcoOS_StrLen(PrefixName, gcvNULL) < __MAX_SYM_NAME_LENGTH__);
    gcoOS_StrCopySafe(prevName, __MAX_SYM_NAME_LENGTH__, PrefixName);
    gcoOS_StrCatSafe(prevName, __MAX_SYM_NAME_LENGTH__, ".");

    if (fields == gcvNULL)
    {
        return errCode;
    }

    for (i = 0; i < VIR_IdList_Count(fields); i++)
    {
        VIR_Precision    precision;

        id = VIR_IdList_GetId(VIR_Type_GetFields(Type), i);
        fieldSym = VIR_Shader_GetSymFromId(Shader, id);
        precision = VIR_Symbol_GetPrecision(fieldSym);
        if(precision == VIR_PRECISION_DEFAULT)
        {
            VIR_Symbol_SetPrecision(fieldSym, VIR_Symbol_GetPrecision(StructSym));
        }
        fieldName = VIR_Shader_GetSymNameString(Shader, fieldSym);
        fieldType = VIR_Symbol_GetType(fieldSym);
        baseTypeId = VIR_Type_GetBaseTypeId(fieldType);

        /* Cat field name: 'prefixName.fieldName'. */
        gcoOS_StrCopySafe(mixName, __MAX_SYM_NAME_LENGTH__, prevName);
        gcoOS_StrCatSafe(mixName, __MAX_SYM_NAME_LENGTH__, fieldName);

        /* If this field is not a normal variable, split it. */
        if (VIR_Type_IsBaseTypeStruct(Shader, fieldType))
        {
            if (VIR_Type_GetKind(fieldType) == VIR_TY_STRUCT)
            {
                errCode = _SplitStructVariable(Shader,
                                               ParentSym,
                                               fieldSym,
                                               SymbolKind,
                                               StorageClass,
                                               mixName,
                                               fieldType,
                                               BlockIndex,
                                               SymFlag,
                                               SplitArray,
                                               AllocateSym,
                                               AllocateReg,
                                               UpcomingRegCount,
                                               Location,
                                               (firstElementId == VIR_INVALID_ID) ? FirstElementId : gcvNULL,
                                               IdList);
                CHECK_ERROR(errCode, "_SplitStructVariable failed.");
                continue;
            }
            else
            {
                gctUINT upcomingRegCount = 0;
                if(*UpcomingRegCount == 0)
                {
                    upcomingRegCount = VIR_Type_GetVirRegCount(Shader, fieldType, -1);
                }
                gcmASSERT(VIR_Type_GetKind(fieldType) == VIR_TY_ARRAY);
                errCode = _SplitArrayVariable(Shader,
                                              ParentSym,
                                              fieldSym,
                                              fieldType,
                                              SymbolKind,
                                              StorageClass,
                                              mixName,
                                              BlockIndex,
                                              SymFlag,
                                              SplitArray,
                                              AllocateSym,
                                              AllocateReg,
                                              *UpcomingRegCount ? UpcomingRegCount : &upcomingRegCount,
                                              Location,
                                              (firstElementId == VIR_INVALID_ID) ? FirstElementId : gcvNULL,
                                              IdList);

                gcmASSERT(!AllocateReg || (AllocateReg && upcomingRegCount == 0));
                CHECK_ERROR(errCode, "_SplitArrayVariable failed.");
                continue;
            }
        }

        /* Generate the symbol kind. */
        if (VIR_TypeId_isImage(baseTypeId))
        {
            fieldKind = VIR_SYM_IMAGE;
        }
        else if (VIR_TypeId_isSampler(baseTypeId))
        {
            fieldKind = VIR_SYM_SAMPLER;
        }
        else
        {
            fieldKind = SymbolKind;
        }

        errCode = _AddGeneralVariable(Shader,
                                      ParentSym,
                                      fieldSym,
                                      gcvNULL,
                                      fieldKind,
                                      StorageClass,
                                      BlockIndex,
                                      SymFlag,
                                      AllocateSym,
                                      AllocateReg,
                                      UpcomingRegCount,
                                      mixName,
                                      Location,
                                      &symId,
                                      IdList);
        CHECK_ERROR(errCode, "_AddGeneralVariable failed.");

        if (firstElementId == VIR_INVALID_ID)
        {
            firstElementId = symId;
        }
    }

    /* Save the first element ID if needed. */
    if (FirstElementId && *FirstElementId == VIR_INVALID_ID)
    {
        *FirstElementId = firstElementId;
    }

    return errCode;
}

static VSC_ErrCode
_SplitArrayVariable(
    IN  VIR_Shader              *Shader,
    IN  VIR_Symbol              *ParentSym,
    IN  VIR_Symbol              *ArraySym,
    IN  VIR_Type                *Type,
    IN  VIR_SymbolKind           SymbolKind,
    IN  VIR_StorageClass         StorageClass,
    IN  gctSTRING                PrefixName,
    IN  VIR_SymId                BlockIndex,
    IN  VIR_SymFlag              SymFlag,
    IN  gctBOOL                  SplitArray,
    IN  gctBOOL                  AllocateSym,
    IN  gctBOOL                  AllocateReg,
    IN  gctUINT                 *UpcomingRegCount,
    OUT gctINT                  *Location,
    OUT VIR_SymId               *FirstElementId,
    OUT VIR_IdList              *IdList
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_SymId                    firstElementId = VIR_INVALID_ID;
    VIR_Type                    *type = Type;
    VIR_TypeId                   baseTypeId = VIR_Type_GetBaseTypeId(type);
    VIR_Type                    *baseType = VIR_Shader_GetTypeFromId(Shader, baseTypeId);
    VIR_SymId                    symId;
    VIR_SymbolKind               symbolKind;
    gctUINT                      i, arrayLength = VIR_Type_GetArrayLength(type);
    gctUINT                      offset = 0;
    gctCHAR                      mixName[__MAX_SYM_NAME_LENGTH__];

    if (VIR_Type_IsBaseTypeStruct(Shader, type) || SplitArray)
    {
        for (i = 0; i < arrayLength; i++)
        {
            offset = 0;
            /* Generate array index name: 'symName[index]'. */
            gcoOS_PrintStrSafe(mixName,
                               __MAX_SYM_NAME_LENGTH__,
                               &offset,
                               "%s[%d]",
                               PrefixName,
                               i);

            if (VIR_Type_IsBaseTypeStruct(Shader, baseType))
            {
                if (VIR_Type_GetKind(baseType) == VIR_TY_STRUCT)
                {
                    errCode = _SplitStructVariable(Shader,
                                                   ParentSym,
                                                   ArraySym,
                                                   SymbolKind,
                                                   StorageClass,
                                                   mixName,
                                                   baseType,
                                                   BlockIndex,
                                                   SymFlag,
                                                   SplitArray,
                                                   AllocateSym,
                                                   AllocateReg,
                                                   UpcomingRegCount,
                                                   Location,
                                                   (firstElementId == VIR_INVALID_ID) ? FirstElementId : gcvNULL,
                                                   IdList);
                    CHECK_ERROR(errCode, "_SplitStructVariable failed.");
                }
                else
                {
                    gcmASSERT(VIR_Type_GetKind(baseType) == VIR_TY_ARRAY);
                    errCode = _SplitArrayVariable(Shader,
                                                  ParentSym,
                                                  ArraySym,
                                                  baseType,
                                                  SymbolKind,
                                                  StorageClass,
                                                  mixName,
                                                  BlockIndex,
                                                  SymFlag,
                                                  SplitArray,
                                                  AllocateSym,
                                                  AllocateReg,
                                                  UpcomingRegCount,
                                                  Location,
                                                  (firstElementId == VIR_INVALID_ID) ? FirstElementId : gcvNULL,
                                                  IdList);
                    CHECK_ERROR(errCode, "_SplitArrayVariable failed.");
                }
            }
            else
            {
                /* Generate the symbol kind. */
                if (VIR_TypeId_isImage(baseTypeId))
                {
                    symbolKind = VIR_SYM_IMAGE;
                }
                else if (VIR_TypeId_isSampler(baseTypeId))
                {
                    symbolKind = VIR_SYM_SAMPLER;
                }
                else
                {
                    symbolKind = SymbolKind;
                }

                errCode = _AddGeneralVariable(Shader,
                                              ParentSym,
                                              ArraySym,
                                              baseType,
                                              symbolKind,
                                              StorageClass,
                                              BlockIndex,
                                              SymFlag,
                                              AllocateSym,
                                              AllocateReg,
                                              UpcomingRegCount,
                                              mixName,
                                              Location,
                                              &symId,
                                              IdList);
                CHECK_ERROR(errCode, "_AddGeneralVariable failed.");

                if (firstElementId == VIR_INVALID_ID)
                {
                    firstElementId = symId;
                }
            }
        }
    }
    else
    {
        /* Generate the symbol kind. */
        if (VIR_TypeId_isImage(baseTypeId))
        {
            symbolKind = VIR_SYM_IMAGE;
        }
        else if (VIR_TypeId_isSampler(baseTypeId))
        {
            symbolKind = VIR_SYM_SAMPLER;
        }
        else
        {
            symbolKind = SymbolKind;
        }

        errCode = _AddGeneralVariable(Shader,
                                      ParentSym,
                                      ArraySym,
                                      gcvNULL,
                                      symbolKind,
                                      StorageClass,
                                      BlockIndex,
                                      SymFlag,
                                      AllocateSym,
                                      AllocateReg,
                                      UpcomingRegCount,
                                      PrefixName,
                                      Location,
                                      &symId,
                                      IdList);
        CHECK_ERROR(errCode, "_AddGeneralVariable failed.");

        if (firstElementId == VIR_INVALID_ID)
        {
            firstElementId = symId;
        }
    }

    /* Save the first element ID if needed. */
    if (FirstElementId && *FirstElementId == VIR_INVALID_ID)
    {
        *FirstElementId = firstElementId;
    }

    return errCode;
}

static VSC_ErrCode
_SplitUniforms(
    IN  VIR_Shader              *Shader,
    IN  VIR_IdList              *IdList,
    IN  gctUINT                  IdIndex,
    IN  VIR_Symbol              *UniformSym,
    IN  VIR_StorageClass         StorageClass,
    OUT gctINT                  *Location
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_Type                    *type = VIR_Symbol_GetType(UniformSym);
    gctSTRING                    uniformName = VIR_Shader_GetSymNameString(Shader, UniformSym);
    VIR_SymId                    firstElementId = VIR_INVALID_ID;
    gctUINT                      upcomingRegCount = 0;

    gcmASSERT(!VIR_Type_isPrimitive(type));

    if (VIR_Type_isArray(type))
    {
        /*upcomingRegCount = VIR_Type_GetVirRegCount(Shader, type, -1);*/

        errCode = _SplitArrayVariable(Shader,
                                      UniformSym,
                                      UniformSym,
                                      type,
                                      VIR_SYM_UNIFORM,
                                      StorageClass,
                                      uniformName,
                                      VIR_INVALID_ID,
                                      VIR_SYMFLAG_NONE,
                                      gcvFALSE,
                                      gcvTRUE,
                                      gcvFALSE,
                                      &upcomingRegCount,
                                      Location,
                                      &firstElementId,
                                      gcvNULL);
        CHECK_ERROR(errCode, "_SplitArrayVariable failed.");
    }
    else
    {
        gcmASSERT(VIR_Type_GetKind(type) == VIR_TY_STRUCT);

        errCode = _SplitStructVariable(Shader,
                                       UniformSym,
                                       UniformSym,
                                       VIR_SYM_UNIFORM,
                                       StorageClass,
                                       uniformName,
                                       type,
                                       VIR_INVALID_ID,
                                       VIR_SYMFLAG_NONE,
                                       gcvFALSE,
                                       gcvTRUE,
                                       gcvFALSE,
                                       &upcomingRegCount,
                                       Location,
                                       &firstElementId,
                                       gcvNULL);
        CHECK_ERROR(errCode, "_SplitStructVariable failed.");
    }

    /* Set the first element ID. */
    gcmASSERT(firstElementId != VIR_INVALID_ID);
    VIR_Symbol_SetFirstElementId(UniformSym, firstElementId);

    return errCode;
}

static VSC_ErrCode
_GenInvocationIndex(
    IN  VIR_Shader              *Shader,
    IN  VIR_Symbol              *VariableSym
    )
{
    VSC_ErrCode     errCode  = VSC_ERR_NONE;
    VIR_Function    *pFunc = VIR_Shader_GetMainFunction(Shader);
    VIR_Instruction *mul1Inst = gcvNULL, *mul2Inst = gcvNULL;
    VIR_Instruction *add1Inst = gcvNULL, *add2Inst = gcvNULL;
    VIR_Operand     *src = gcvNULL;
    VIR_SymId        newVarSymId, tmpSymId1, tmpSymId2, tmpSymId3;
    VIR_SymId        tmpSymId = VIR_INVALID_ID, IndexSymId = VIR_INVALID_ID;
    VIR_Symbol       *newVarSym = gcvNULL;
    VIR_VirRegId     regId = VIR_INVALID_ID;
    gctUINT         i;
    VIR_AttributeIdList *attIdList = VIR_Shader_GetAttributes(Shader);

    gcmASSERT(VIR_Symbol_GetName(VariableSym) == VIR_NAME_LOCALINVOCATIONINDEX);

    /* create a temp for invocation index */
    regId = VIR_Shader_NewVirRegId(Shader, 1);
    errCode = VIR_Shader_AddSymbol(Shader,
                VIR_SYM_VIRREG,
                regId,
                VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT32),
                VIR_STORAGE_UNKNOWN,
                &IndexSymId);
    VIR_Symbol_ClrFlag(VariableSym, VIR_SYMFLAG_ENABLED | VIR_SYMFLAG_STATICALLY_USED);
    VIR_Symbol_SetFlag(VariableSym, VIR_SYMFLAG_UNUSED);
    VIR_Symbol_SetVariableVregIndex(VariableSym, regId);

    /* add an attribute if not found - LocalInvocationID */
    for (i = 0;  i< VIR_IdList_Count(attIdList); i++)
    {
        VIR_Symbol*attr = VIR_Shader_GetSymFromId(Shader, VIR_IdList_GetId(attIdList, i));
        if (VIR_Symbol_GetName(attr) == VIR_NAME_LOCAL_INVOCATION_ID)
        {
            newVarSym = attr;
            break;
        }
    }

    if (i == VIR_IdList_Count(attIdList))
    {
        errCode = VIR_Shader_AddSymbol(Shader,
                                       VIR_SYM_VARIABLE,
                                       VIR_NAME_LOCAL_INVOCATION_ID,
                                       VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT_X3),
                                       VIR_Symbol_GetStorageClass(VariableSym),
                                       &newVarSymId);
        CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed.");
        newVarSym = VIR_Shader_GetSymFromId(Shader, newVarSymId);
        VIR_Symbol_SetFlag(newVarSym, VIR_SYMFLAG_ENABLED | VIR_SYMFLAG_STATICALLY_USED);

        /* create a temp for invocation id */
        regId = VIR_Shader_NewVirRegId(Shader, 1);
        errCode = VIR_Shader_AddSymbol(Shader,
                    VIR_SYM_VIRREG,
                    regId,
                    VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT_X3),
                    VIR_STORAGE_UNKNOWN,
                    &tmpSymId);

        VIR_Symbol_SetVariableVregIndex(newVarSym, regId);
        VIR_Symbol_SetVregVariable(VIR_Shader_GetSymFromId(Shader, tmpSymId), newVarSym);
    }

    /* Compute local invocation index :
           Z * I * J + Y * I + X
           where local Id = (X, Y, Z) and
                 work group size = (I, J, K)  */

    /* (Y, Z) * I */
    errCode = VIR_Function_PrependInstruction(pFunc,
                        VIR_OP_MUL,
                        VIR_TYPE_UINT_X2,
                        &mul1Inst);

    CHECK_ERROR(errCode, "VIR_Function_PrependInstruction failed.");

    /* src0 - (Y, Z) */
    src = VIR_Inst_GetSource(mul1Inst, 0);
    VIR_Operand_SetOpKind(src, VIR_OPND_SYMBOL);
    VIR_Operand_SetTypeId(src, VIR_TYPE_UINT_X2);
    VIR_Operand_SetSym(src, newVarSym);
    VIR_Operand_SetSwizzle(src, VIR_SWIZZLE_YZZZ);

    /* src1 - workGroupSize[0] */
    src = VIR_Inst_GetSource(mul1Inst, 1);
    VIR_Operand_SetImmediateUint(src, Shader->shaderLayout.compute.workGroupSize[0]);

    /* dest */
    errCode = VIR_Shader_AddSymbol(Shader,
            VIR_SYM_VIRREG,
            VIR_Shader_NewVirRegId(Shader, 1),
            VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT_X2),
            VIR_STORAGE_UNKNOWN,
            &tmpSymId1);

    VIR_Operand_SetTempRegister(VIR_Inst_GetDest(mul1Inst),
                                pFunc,
                                tmpSymId1,
                                VIR_TYPE_UINT_X2);
    VIR_Operand_SetEnable(VIR_Inst_GetDest(mul1Inst), VIR_ENABLE_XY);

    /* (Z * I) * J */
    errCode = VIR_Function_AddInstructionAfter(pFunc,
                        VIR_OP_MUL,
                        VIR_TYPE_UINT32,
                        mul1Inst,
                        gcvTRUE,
                        &mul2Inst);

    CHECK_ERROR(errCode, "VIR_Function_PrependInstruction failed.");

    /* src0 */
    src = VIR_Inst_GetSource(mul2Inst, 0);
    VIR_Operand_SetTempRegister(src,
                                pFunc,
                                tmpSymId1,
                                VIR_TYPE_UINT32);
    VIR_Operand_SetSwizzle(src, VIR_SWIZZLE_YYYY);

    /* src1 - workGroupSize[1] */
    src = VIR_Inst_GetSource(mul2Inst, 1);
    VIR_Operand_SetImmediateUint(src, Shader->shaderLayout.compute.workGroupSize[1]);

    /* dest */
    errCode = VIR_Shader_AddSymbol(Shader,
            VIR_SYM_VIRREG,
            VIR_Shader_NewVirRegId(Shader, 1),
            VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT32),
            VIR_STORAGE_UNKNOWN,
            &tmpSymId2);

    VIR_Operand_SetTempRegister(VIR_Inst_GetDest(mul2Inst),
                                pFunc,
                                tmpSymId2,
                                VIR_TYPE_UINT32);
    VIR_Operand_SetEnable(VIR_Inst_GetDest(mul2Inst), VIR_ENABLE_X);

    /* (Z * I) * J + (Y * I) */
    errCode = VIR_Function_AddInstructionAfter(pFunc,
                        VIR_OP_ADD,
                        VIR_TYPE_UINT32,
                        mul2Inst,
                        gcvTRUE,
                        &add1Inst);

    CHECK_ERROR(errCode, "VIR_Function_PrependInstruction failed.");

    /* src0 */
    src = VIR_Inst_GetSource(add1Inst, 0);
    VIR_Operand_SetTempRegister(src,
                                pFunc,
                                tmpSymId2,
                                VIR_TYPE_UINT32);
    VIR_Operand_SetSwizzle(src, VIR_SWIZZLE_XXXX);

    /* src1 */
    src = VIR_Inst_GetSource(add1Inst, 1);
    VIR_Operand_SetTempRegister(src,
                                pFunc,
                                tmpSymId1,
                                VIR_TYPE_UINT32);
    VIR_Operand_SetSwizzle(src, VIR_SWIZZLE_XXXX);

    /* dest */
    errCode = VIR_Shader_AddSymbol(Shader,
            VIR_SYM_VIRREG,
            VIR_Shader_NewVirRegId(Shader, 1),
            VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT32),
            VIR_STORAGE_UNKNOWN,
            &tmpSymId3);

    VIR_Operand_SetTempRegister(VIR_Inst_GetDest(add1Inst),
                                pFunc,
                                tmpSymId3,
                                VIR_TYPE_UINT32);
    VIR_Operand_SetEnable(VIR_Inst_GetDest(add1Inst), VIR_ENABLE_X);

    /* (Z * I) * J + (Y * I)  + X*/
    errCode = VIR_Function_AddInstructionAfter(pFunc,
                        VIR_OP_ADD,
                        VIR_TYPE_UINT32,
                        add1Inst,
                        gcvTRUE,
                        &add2Inst);

    CHECK_ERROR(errCode, "VIR_Function_PrependInstruction failed.");

    /* src0 */
    src = VIR_Inst_GetSource(add2Inst, 0);
    VIR_Operand_SetTempRegister(src,
                                pFunc,
                                tmpSymId3,
                                VIR_TYPE_UINT32);
    VIR_Operand_SetSwizzle(src, VIR_SWIZZLE_XXXX);

    /* src1 */
    src = VIR_Inst_GetSource(add2Inst, 1);
    VIR_Operand_SetOpKind(src, VIR_OPND_SYMBOL);
    VIR_Operand_SetTypeId(src, VIR_TYPE_UINT32);
    VIR_Operand_SetSym(src, newVarSym);
    VIR_Operand_SetSwizzle(src, VIR_SWIZZLE_XXXX);

    /* dest */
    VIR_Operand_SetTempRegister(VIR_Inst_GetDest(add2Inst),
                                pFunc,
                                IndexSymId,
                                VIR_TYPE_UINT32);
    VIR_Operand_SetEnable(VIR_Inst_GetDest(add2Inst), VIR_ENABLE_X);

    return errCode;
}

static VSC_ErrCode
_GenWorkGroupIndex(
    IN  VIR_Shader              *Shader,
    IN  VIR_Symbol              *VariableSym
    )
{
    VSC_ErrCode     errCode  = VSC_ERR_NONE;
    VIR_Function    *pFunc = VIR_Shader_GetMainFunction(Shader);
    VIR_Instruction *mul1Inst = gcvNULL, *mul2Inst = gcvNULL;
    VIR_Instruction *add1Inst = gcvNULL, *add2Inst = gcvNULL;
    VIR_Operand     *src = gcvNULL;
    VIR_SymId        newVarSymId, tmpSymId1, tmpSymId2, tmpSymId3;
    VIR_SymId        tmpSymId = VIR_INVALID_ID, IndexSymId = VIR_INVALID_ID;
    VIR_Symbol       *newVarSym = gcvNULL;
    VIR_VirRegId     regId = VIR_INVALID_ID;
    gctUINT         i;
    VIR_AttributeIdList *attIdList = VIR_Shader_GetAttributes(Shader);
    VIR_Symbol       *numVarSym = gcvNULL;
    VIR_SymId        numSymId = VIR_INVALID_ID;

    gcmASSERT(VIR_Symbol_GetName(VariableSym) == VIR_NAME_WORK_GROUP_INDEX);

    /* create a temp for groupIndex */
    regId = VIR_Shader_NewVirRegId(Shader, 1);
    errCode = VIR_Shader_AddSymbol(Shader,
                VIR_SYM_VIRREG,
                regId,
                VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT_X3),
                VIR_STORAGE_UNKNOWN,
                &IndexSymId);
    VIR_Symbol_ClrFlag(VariableSym, VIR_SYMFLAG_ENABLED | VIR_SYMFLAG_STATICALLY_USED);
    VIR_Symbol_SetFlag(VariableSym, VIR_SYMFLAG_UNUSED);
    VIR_Symbol_SetVariableVregIndex(VariableSym, regId);

    /* add an attribute if not found - groupId */
    for (i = 0;  i< VIR_IdList_Count(attIdList); i++)
    {
        VIR_Symbol*attr = VIR_Shader_GetSymFromId(Shader, VIR_IdList_GetId(attIdList, i));
        if (VIR_Symbol_GetName(attr) == VIR_NAME_WORK_GROUP_ID)
        {
            newVarSym = attr;
            break;
        }
    }

    if (i == VIR_IdList_Count(attIdList))
    {
        errCode = VIR_Shader_AddSymbol(Shader,
                                       VIR_SYM_VARIABLE,
                                       VIR_NAME_WORK_GROUP_ID,
                                       VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT_X3),
                                       VIR_Symbol_GetStorageClass(VariableSym),
                                       &newVarSymId);
        CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed.");
        newVarSym = VIR_Shader_GetSymFromId(Shader, newVarSymId);
        VIR_Symbol_SetFlag(newVarSym, VIR_SYMFLAG_ENABLED | VIR_SYMFLAG_STATICALLY_USED);

        /* create a temp */
        regId = VIR_Shader_NewVirRegId(Shader, 1);
        errCode = VIR_Shader_AddSymbol(Shader,
                    VIR_SYM_VIRREG,
                    regId,
                    VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT_X3),
                    VIR_STORAGE_UNKNOWN,
                    &tmpSymId);
        VIR_Symbol_SetVariableVregIndex(newVarSym, regId);
        VIR_Symbol_SetVregVariable(VIR_Shader_GetSymFromId(Shader, tmpSymId), newVarSym);
    }

    /* find gl_NumWorkGroups */
    numVarSym = VIR_Shader_FindSymbolByName(Shader,
                                      VIR_SYM_UNIFORM,
                                      "gl_NumWorkGroups");

    if (numVarSym == gcvNULL)
    {
        VIR_NameId nameId;

        errCode = VIR_Shader_AddString(Shader,
                                       "gl_NumWorkGroups",
                                       &nameId);
        if (errCode != VSC_ERR_NONE) return errCode;

        errCode = VIR_Shader_AddSymbol(Shader,
                                       VIR_SYM_UNIFORM,
                                       nameId,
                                       VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT_X3),
                                       VIR_STORAGE_UNKNOWN,
                                       &numSymId);
        CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed.");

        numVarSym = VIR_Shader_GetSymFromId(Shader, numSymId);
        VIR_Symbol_SetLocation(numVarSym, -1);
    }
    else
    {
        numSymId = VIR_Symbol_GetIndex(numVarSym);
    }

    /* Compute workgroup index :
           Z * I * J + Y * I + X
           where workgroupId = (X, Y, Z) and
                 #numWorkGroup = (I, J, K)  */

    /* (Y, Z) * I */
    errCode = VIR_Function_PrependInstruction(pFunc,
                        VIR_OP_MUL,
                        VIR_TYPE_UINT32,
                        &mul1Inst);

    CHECK_ERROR(errCode, "VIR_Function_PrependInstruction failed.");

    /* src0 - (Y, Z) */
    src = VIR_Inst_GetSource(mul1Inst, 0);
    VIR_Operand_SetOpKind(src, VIR_OPND_SYMBOL);
    VIR_Operand_SetTypeId(src, VIR_TYPE_UINT_X2);
    VIR_Operand_SetSym(src, newVarSym);
    VIR_Operand_SetSwizzle(src, VIR_SWIZZLE_YZZZ);

    /* src1 - numWorkGroup.x */
    src = VIR_Inst_GetSource(mul1Inst, 1);
    VIR_Operand_SetSymbol(src, Shader->mainFunction, numSymId);
    VIR_Operand_SetSwizzle(src, VIR_SWIZZLE_X);

    /* dest */
    errCode = VIR_Shader_AddSymbol(Shader,
            VIR_SYM_VIRREG,
            VIR_Shader_NewVirRegId(Shader, 1),
            VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT_X2),
            VIR_STORAGE_UNKNOWN,
            &tmpSymId1);

    VIR_Operand_SetTempRegister(VIR_Inst_GetDest(mul1Inst),
                                pFunc,
                                tmpSymId1,
                                VIR_TYPE_UINT_X2);
    VIR_Operand_SetEnable(VIR_Inst_GetDest(mul1Inst), VIR_ENABLE_XY);

    /* (Z * I) * J */
    errCode = VIR_Function_AddInstructionAfter(pFunc,
                        VIR_OP_MUL,
                        VIR_TYPE_UINT32,
                        mul1Inst,
                        gcvTRUE,
                        &mul2Inst);

    CHECK_ERROR(errCode, "VIR_Function_PrependInstruction failed.");

    /* src0 */
    src = VIR_Inst_GetSource(mul2Inst, 0);
    VIR_Operand_SetTempRegister(src,
                                pFunc,
                                tmpSymId1,
                                VIR_TYPE_UINT32);
    VIR_Operand_SetSwizzle(src, VIR_SWIZZLE_YYYY);

    /* src1 - numWorkgroup.y */
    src = VIR_Inst_GetSource(mul2Inst, 1);
    VIR_Operand_SetSymbol(src, Shader->mainFunction, numSymId);
    VIR_Operand_SetSwizzle(src, VIR_SWIZZLE_Y);

    /* dest */
    errCode = VIR_Shader_AddSymbol(Shader,
            VIR_SYM_VIRREG,
            VIR_Shader_NewVirRegId(Shader, 1),
            VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT32),
            VIR_STORAGE_UNKNOWN,
            &tmpSymId2);

    VIR_Operand_SetTempRegister(VIR_Inst_GetDest(mul2Inst),
                                pFunc,
                                tmpSymId2,
                                VIR_TYPE_UINT32);
    VIR_Operand_SetEnable(VIR_Inst_GetDest(mul2Inst), VIR_ENABLE_X);

    /* (Z * I) * J + (Y * I) */
    errCode = VIR_Function_AddInstructionAfter(pFunc,
                        VIR_OP_ADD,
                        VIR_TYPE_UINT32,
                        mul2Inst,
                        gcvTRUE,
                        &add1Inst);

    CHECK_ERROR(errCode, "VIR_Function_PrependInstruction failed.");

    /* src0 */
    src = VIR_Inst_GetSource(add1Inst, 0);
    VIR_Operand_SetTempRegister(src,
                                pFunc,
                                tmpSymId2,
                                VIR_TYPE_UINT32);
    VIR_Operand_SetSwizzle(src, VIR_SWIZZLE_XXXX);

    /* src1 */
    src = VIR_Inst_GetSource(add1Inst, 1);
    VIR_Operand_SetTempRegister(src,
                                pFunc,
                                tmpSymId1,
                                VIR_TYPE_UINT32);
    VIR_Operand_SetSwizzle(src, VIR_SWIZZLE_XXXX);

    /* dest */
    errCode = VIR_Shader_AddSymbol(Shader,
            VIR_SYM_VIRREG,
            VIR_Shader_NewVirRegId(Shader, 1),
            VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_FLOAT_X4),
            VIR_STORAGE_UNKNOWN,
            &tmpSymId3);

    VIR_Operand_SetTempRegister(VIR_Inst_GetDest(add1Inst),
                                pFunc,
                                tmpSymId3,
                                VIR_TYPE_UINT32);
    VIR_Operand_SetEnable(VIR_Inst_GetDest(add1Inst), VIR_ENABLE_X);

    /* (Z * I) * J + (Y * I)  + X*/
    errCode = VIR_Function_AddInstructionAfter(pFunc,
                        VIR_OP_ADD,
                        VIR_TYPE_UINT32,
                        add1Inst,
                        gcvTRUE,
                        &add2Inst);

    CHECK_ERROR(errCode, "VIR_Function_PrependInstruction failed.");

    /* src0 */
    src = VIR_Inst_GetSource(add2Inst, 0);
    VIR_Operand_SetTempRegister(src,
                                pFunc,
                                tmpSymId3,
                                VIR_TYPE_UINT32);
    VIR_Operand_SetSwizzle(src, VIR_SWIZZLE_XXXX);

    /* src1 */
    src = VIR_Inst_GetSource(add2Inst, 1);
    VIR_Operand_SetOpKind(src, VIR_OPND_SYMBOL);
    VIR_Operand_SetTypeId(src, VIR_TYPE_UINT32);
    VIR_Operand_SetSym(src, newVarSym);
    VIR_Operand_SetSwizzle(src, VIR_SWIZZLE_XXXX);

    /* dest */
    VIR_Operand_SetTempRegister(VIR_Inst_GetDest(add2Inst),
                                pFunc,
                                IndexSymId,
                                VIR_TYPE_UINT32);
    VIR_Operand_SetEnable(VIR_Inst_GetDest(add2Inst), VIR_ENABLE_X);

    return errCode;
}

static VSC_ErrCode _AppendBaseInstanceUniform(
    VIR_Shader *pShader,
    VIR_Uniform **baseInstanceUniform)
{
    VSC_ErrCode  virErrCode;
    VIR_NameId   baseInstanceName;
    VIR_SymId    baseInstanceSymId;
    VIR_Symbol*  baseInstanceSym;
    VIR_Uniform *virUniform = gcvNULL;

    virErrCode = VIR_Shader_AddString(pShader, "#BaseInstance", &baseInstanceName);
    ON_ERROR(virErrCode, "Failed to VIR_Shader_AddString");

    /* default ubo symbol */
    virErrCode = VIR_Shader_AddSymbol(pShader,
                                      VIR_SYM_UNIFORM,
                                      baseInstanceName,
                                      VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_UINT32),
                                      VIR_STORAGE_UNKNOWN,
                                      &baseInstanceSymId);

    baseInstanceSym = VIR_Shader_GetSymFromId(pShader, baseInstanceSymId);
    VIR_Symbol_SetUniformKind(baseInstanceSym, VIR_UNIFORM_BASE_INSTANCE);
    VIR_Symbol_SetFlag(baseInstanceSym, VIR_SYMUNIFORMFLAG_USED_IN_SHADER);
    VIR_Symbol_SetFlag(baseInstanceSym, VIR_SYMFLAG_COMPILER_GEN);
    VIR_Symbol_SetLocation(baseInstanceSym, -1);

    virUniform = VIR_Symbol_GetUniform(baseInstanceSym);
    virUniform->index = VIR_IdList_Count(VIR_Shader_GetUniforms(pShader)) - 1;

    if (baseInstanceUniform)
    {
        *baseInstanceUniform = virUniform;
    }

OnError:
    return virErrCode;
}
static VSC_ErrCode
_GenInstanceIndex(
    IN  VIR_Shader              *Shader,
    IN  VIR_Symbol              *VariableSym
    )
{
    VSC_ErrCode     errCode  = VSC_ERR_NONE;
    VIR_Function    *pFunc = VIR_Shader_GetMainFunction(Shader);
    VIR_Instruction *addInst = gcvNULL;
    VIR_Operand     *src = gcvNULL;
    VIR_SymId        newVarSymId;
    VIR_SymId        tmpSymId = VIR_INVALID_ID, IndexSymId = VIR_INVALID_ID;
    VIR_Symbol       *newVarSym = gcvNULL;
    VIR_VirRegId     regId = VIR_INVALID_ID;
    gctUINT         i;
    VIR_AttributeIdList *attIdList = VIR_Shader_GetAttributes(Shader);
    VIR_Uniform      *baseInstanceUniform = gcvNULL;

    gcmASSERT(VIR_Symbol_GetName(VariableSym) == VIR_NAME_INSTANCE_INDEX);

    /* create a temp for instance index */
    regId = VIR_Shader_NewVirRegId(Shader, 1);
    errCode = VIR_Shader_AddSymbol(Shader,
                VIR_SYM_VIRREG,
                regId,
                VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT32),
                VIR_STORAGE_UNKNOWN,
                &IndexSymId);
    VIR_Symbol_ClrFlag(VariableSym, VIR_SYMFLAG_ENABLED | VIR_SYMFLAG_STATICALLY_USED);
    VIR_Symbol_SetFlag(VariableSym, VIR_SYMFLAG_UNUSED);
    VIR_Symbol_SetVariableVregIndex(VariableSym, regId);

    /* add an attribute if not found - LocalInvocationID */
    for (i = 0;  i< VIR_IdList_Count(attIdList); i++)
    {
        VIR_Symbol*attr = VIR_Shader_GetSymFromId(Shader, VIR_IdList_GetId(attIdList, i));
        if (VIR_Symbol_GetName(attr) == VIR_NAME_INSTANCE_ID)
        {
            newVarSym = attr;
            break;
        }
    }

    if (i == VIR_IdList_Count(attIdList))
    {
        errCode = VIR_Shader_AddSymbol(Shader,
                                       VIR_SYM_VARIABLE,
                                       VIR_NAME_INSTANCE_ID,
                                       VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT32),
                                       VIR_Symbol_GetStorageClass(VariableSym),
                                       &newVarSymId);
        CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed.");
        newVarSym = VIR_Shader_GetSymFromId(Shader, newVarSymId);
        VIR_Symbol_SetFlag(newVarSym, VIR_SYMFLAG_ENABLED | VIR_SYMFLAG_STATICALLY_USED);

        /* create a temp for instance id */
        regId = VIR_Shader_NewVirRegId(Shader, 1);
        errCode = VIR_Shader_AddSymbol(Shader,
                    VIR_SYM_VIRREG,
                    regId,
                    VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT32),
                    VIR_STORAGE_UNKNOWN,
                    &tmpSymId);

        VIR_Symbol_SetVariableVregIndex(newVarSym, regId);
        VIR_Symbol_SetVregVariable(VIR_Shader_GetSymFromId(Shader, tmpSymId), newVarSym);
    }

    /* instanceId +  baseInstance */
    errCode = VIR_Function_PrependInstruction(pFunc,
                        VIR_OP_ADD,
                        VIR_TYPE_UINT32,
                        &addInst);

    CHECK_ERROR(errCode, "VIR_Function_PrependInstruction failed.");

    /* src0 - instanceId */
    src = VIR_Inst_GetSource(addInst, 0);
    VIR_Operand_SetOpKind(src, VIR_OPND_SYMBOL);
    VIR_Operand_SetTypeId(src, VIR_TYPE_UINT32);
    VIR_Operand_SetSym(src, newVarSym);
    VIR_Operand_SetSwizzle(src, VIR_SWIZZLE_X);

    /* src1 - baseInstance */
    src = VIR_Inst_GetSource(addInst, 1);
    _AppendBaseInstanceUniform(Shader, &baseInstanceUniform);
    VIR_Operand_SetUniform(src, baseInstanceUniform, Shader);

    /* dest */
    VIR_Operand_SetTempRegister(VIR_Inst_GetDest(addInst),
                                pFunc,
                                IndexSymId,
                                VIR_TYPE_UINT32);
    VIR_Operand_SetEnable(VIR_Inst_GetDest(addInst), VIR_ENABLE_X);

    return errCode;
}

static VSC_ErrCode
_SplitVariables(
    IN  VIR_Shader              *Shader,
    IN  VIR_IdList              *IdList,
    IN  gctUINT                  IdIndex,
    IN  VIR_Symbol              *VariableSym,
    IN  VIR_StorageClass         StorageClass,
    OUT gctINT                  *Location
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_SymId                    symId;
    VIR_Type                    *type = VIR_Symbol_GetType(VariableSym);
    VIR_SymId                    firstElementId = VIR_INVALID_ID;
    gctSTRING                    name = VIR_Shader_GetSymNameString(Shader, VariableSym);
    gctUINT                      upcomingRegCount = 0;

    if (VIR_Type_IsBaseTypeStruct(Shader, type))
    {
        if (VIR_Type_isArray(type))
        {
            upcomingRegCount = VIR_Type_GetVirRegCount(Shader, type, -1);
            errCode = _SplitArrayVariable(Shader,
                                          VariableSym,
                                          VariableSym,
                                          type,
                                          VIR_SYM_VARIABLE,
                                          StorageClass,
                                          name,
                                          VIR_INVALID_ID,
                                          VIR_Symbol_GetFlag(VariableSym),
                                          gcvFALSE,
                                          gcvTRUE,
                                          gcvTRUE,
                                          &upcomingRegCount,
                                          Location,
                                          &firstElementId,
                                          gcvNULL);
            CHECK_ERROR(errCode, "_SplitArrayVariable failed.");
        }
        else
        {
            gcmASSERT(VIR_Type_GetKind(type) == VIR_TY_STRUCT);

            errCode = _SplitStructVariable(Shader,
                                           VariableSym,
                                           VariableSym,
                                           VIR_SYM_VARIABLE,
                                           StorageClass,
                                           name,
                                           type,
                                           VIR_INVALID_ID,
                                           VIR_Symbol_GetFlag(VariableSym),
                                           gcvFALSE,
                                           gcvTRUE,
                                           gcvTRUE,
                                           &upcomingRegCount,
                                           Location,
                                           &firstElementId,
                                           gcvNULL);
            CHECK_ERROR(errCode, "_SplitStructVariable failed.");
        }

        /* Set the first element ID. */
        gcmASSERT(firstElementId != VIR_INVALID_ID);
        VIR_Symbol_SetFirstElementId(VariableSym, firstElementId);
    }
    else
    {
        /* Allocate temp register for this variable. */
        errCode = _AddGeneralVariable(Shader,
                                      VariableSym,
                                      VariableSym,
                                      gcvNULL,
                                      VIR_SYM_VARIABLE,
                                      StorageClass,
                                      VIR_INVALID_ID,
                                      VIR_SYMFLAG_NONE,
                                      gcvFALSE,
                                      gcvTRUE,
                                      &upcomingRegCount,
                                      name,
                                      Location,
                                      &symId,
                                      gcvNULL);
        CHECK_ERROR(errCode, "_AddGeneralVariable failed.");
    }

    gcmASSERT(upcomingRegCount == 0);
    return errCode;
}

static VSC_ErrCode
_SplitOutputs(
    IN  VIR_Shader              *Shader,
    IN  VIR_IdList              *IdList,
    IN  gctUINT                  IdIndex,
    IN  VIR_Symbol              *OutputSym,
    IN  VIR_StorageClass         StorageClass,
    OUT gctINT                  *Location
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_SymId                    symId;
    VIR_Type                    *type = VIR_Symbol_GetType(OutputSym);
    VIR_SymId                    firstElementId = VIR_INVALID_ID;
    gctSTRING                    name = VIR_Shader_GetSymNameString(Shader, OutputSym);
    gctBOOL                      splitArray = gcvFALSE;
    gctUINT                      upcomingRegCount = 0;

    if (VIR_Type_IsBaseTypeStruct(Shader, type))
    {
        if (VIR_Type_isArray(type))
        {
            upcomingRegCount = VIR_Type_GetVirRegCount(Shader, type, -1);

            errCode = _SplitArrayVariable(Shader,
                                          OutputSym,
                                          OutputSym,
                                          type,
                                          VIR_SYM_VARIABLE,
                                          StorageClass,
                                          name,
                                          VIR_INVALID_ID,
                                          VIR_Symbol_GetFlag(OutputSym),
                                          splitArray,
                                          gcvTRUE,
                                          gcvTRUE,
                                          &upcomingRegCount,
                                          Location,
                                          &firstElementId,
                                          gcvNULL);
            CHECK_ERROR(errCode, "_SplitArrayVariable failed.");
        }
        else
        {
            gcmASSERT(VIR_Type_GetKind(type) == VIR_TY_STRUCT);

            errCode = _SplitStructVariable(Shader,
                                           OutputSym,
                                           OutputSym,
                                           VIR_SYM_VARIABLE,
                                           StorageClass,
                                           name,
                                           type,
                                           VIR_INVALID_ID,
                                           VIR_Symbol_GetFlag(OutputSym),
                                           splitArray,
                                           gcvTRUE,
                                           gcvTRUE,
                                           &upcomingRegCount,
                                           Location,
                                           &firstElementId,
                                           gcvNULL);
            CHECK_ERROR(errCode, "_SplitStructVariable failed.");
        }

        /* Set the first element ID. */
        gcmASSERT(firstElementId != VIR_INVALID_ID);
        VIR_Symbol_SetFirstElementId(OutputSym, firstElementId);
    }
    else
    {
        errCode = _AddGeneralVariable(Shader,
                                      OutputSym,
                                      OutputSym,
                                      gcvNULL,
                                      VIR_SYM_VARIABLE,
                                      StorageClass,
                                      VIR_INVALID_ID,
                                      VIR_SYMFLAG_NONE,
                                      gcvFALSE,
                                      gcvTRUE,
                                      &upcomingRegCount,
                                      name,
                                      Location,
                                      &symId,
                                      gcvNULL);
        CHECK_ERROR(errCode, "_AddGeneralVariable failed.");
    }

    gcmASSERT(upcomingRegCount == 0);

    return errCode;
}

static VSC_ErrCode
_AllocateBaseAddrUniformForIB(
    IN  VIR_Shader              *Shader,
    IN  VIR_Symbol              *IBSymbol,
    IN  VIR_SymbolKind           IBSymbolKind,
    IN  VIR_NameId               NameId,
    IN  gctUINT                  ArrayLength,
    OUT VIR_SymId               *BaseAddrSymId)
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_SymId                    baseAddrSymId = VIR_INVALID_ID;
    VIR_Symbol                  *baseAddrSym = gcvNULL;
    VIR_Uniform                 *baseAddrUniform = gcvNULL;
    VIR_TypeId                   baseAddrTypeId = VIR_TYPE_UINT32;
    VIR_SymFlag                  symFlag = VIR_SYMFLAG_NONE;

    /* For SBO, the base address uniform is a uvec2. */
    if (IBSymbolKind == VIR_SYM_UBO)
    {
        baseAddrTypeId = VIR_TYPE_UINT32;
    }
    else if (IBSymbolKind == VIR_SYM_SBO)
    {
        baseAddrTypeId = VIR_TYPE_UINT_X2;
    }

    /* Create the array type if needed. */
    if (ArrayLength > 1)
    {
        errCode = VIR_Shader_AddArrayType(Shader,
                                          baseAddrTypeId,
                                          ArrayLength,
                                          0,
                                          &baseAddrTypeId);
        CHECK_ERROR(errCode, "VIR_Shader_AddArrayType failed.");
    }

    /* Create the base address uniform. */
    errCode = VIR_Shader_AddSymbol(Shader,
                                   VIR_SYM_UNIFORM,
                                   NameId,
                                   VIR_Shader_GetTypeFromId(Shader, baseAddrTypeId),
                                   VIR_STORAGE_UNKNOWN,
                                   &baseAddrSymId);
    CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed.");
    gcmASSERT(baseAddrSymId != VIR_INVALID_ID);

    /* Fill the symbol info. */
    baseAddrSym = VIR_Shader_GetSymFromId(Shader, baseAddrSymId);
    VIR_Symbol_SetAddrSpace(baseAddrSym, VIR_AS_CONSTANT);
    VIR_Symbol_SetTyQualifier(baseAddrSym, VIR_TYQUAL_CONST);
    VIR_Symbol_SetPrecision(baseAddrSym, VIR_PRECISION_HIGH);
    VIR_Symbol_SetLocation(baseAddrSym, -1);
    VIR_Symbol_SetBinding(baseAddrSym, VIR_Symbol_GetBinding(IBSymbol));
    VIR_Symbol_SetDescriptorSet(baseAddrSym, VIR_Symbol_GetDescriptorSet(IBSymbol));
    symFlag |= (VIR_SYMFLAG_STATICALLY_USED | VIR_SYMUNIFORMFLAG_USED_IN_SHADER);
    VIR_Symbol_SetFlag(baseAddrSym, symFlag);
    if (isSymCompilerGen(IBSymbol))
    {
        VIR_Symbol_SetFlag(baseAddrSym, VIR_SYMFLAG_COMPILER_GEN);
    }
    if (isSymSkipNameCheck(IBSymbol))
    {
        VIR_Symbol_SetFlag(baseAddrSym, VIR_SYMFLAG_SKIP_NAME_CHECK);
    }
    VIR_Symbol_ClrFlag(baseAddrSym, VIR_SYMFLAG_WITHOUT_REG);
    if (IBSymbolKind == VIR_SYM_UBO)
    {
        VIR_Symbol_SetUniformKind(baseAddrSym, VIR_UNIFORM_UNIFORM_BLOCK_ADDRESS);
    }
    else if (IBSymbolKind == VIR_SYM_SBO)
    {
        VIR_Symbol_SetUniformKind(baseAddrSym, VIR_UNIFORM_STORAGE_BLOCK_ADDRESS);
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }

    baseAddrUniform = baseAddrSym->u2.uniform;
    baseAddrUniform->sym = VIR_Symbol_GetIndex(baseAddrSym);
    baseAddrUniform->_varCategory = gcSHADER_VAR_CATEGORY_BLOCK_ADDRESS;
    baseAddrUniform->gcslIndex = 0;
    baseAddrUniform->lastIndexingIndex = -1;
    baseAddrUniform->glUniformIndex = 0;

    /* Save the symbol ID. */
    if (BaseAddrSymId)
    {
        *BaseAddrSymId = baseAddrSymId;
    }

    return errCode;
}

static VSC_ErrCode
_AllocateInterfaceBlock(
    IN  VIR_PatternHL2HLContext *Context,
    IN  VIR_Shader              *Shader,
    IN  VIR_Symbol              *IBSymbol,
    IN  VIR_SymFlag              SymFlag,
    IN  gctBOOL                  SplitArray,
    IN  gctBOOL                  GenerateBlockMember,
    IN  gctBOOL                  CreateBaseAddr,
    IN  gctBOOL                  IsBaseAddrArray,
    IN  gctBOOL                  AllocMemberSym,
    IN  gctBOOL                  AllocMemberReg,
    IN OUT VIR_IdList           *IdList
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_Type                    *type = VIR_Symbol_GetType(IBSymbol);
    VIR_Type                    *baseType;
    VIR_SymId                    baseAddrSymId = VIR_INVALID_ID, symId = VIR_INVALID_ID, blockIndex = VIR_INVALID_ID;
    VIR_SymId                   *baseAddrArraySymId = gcvNULL;
    VIR_Symbol                  *symbol;
    VIR_UniformBlock            *uniformBlock = gcvNULL, *arrayMemberUniformBlock = gcvNULL;
    VIR_StorageBlock            *storageBlock = gcvNULL, *arrayMemberStorageBlock = gcvNULL;
    VIR_IOBlock                 *ioBlock = gcvNULL, *arrayMemberIoBlock = gcvNULL;
    VIR_SymbolKind               symbolKind = VIR_Symbol_GetKind(IBSymbol);
    VIR_SymbolKind               blockMemberSymbolKind = VIR_SYM_UNKNOWN;
    VIR_StorageClass             blockMemberStorageClass = VIR_STORAGE_UNKNOWN;
    VIR_NameId                   nameId;
    VIR_SymId                    firstElementId = VIR_INVALID_ID;
    gctSTRING                    blockName = VIR_Shader_GetSymNameString(Shader, IBSymbol);
    gctSTRING                    instanceName = gcvNULL;
    gctINT                       location = VIR_Symbol_GetLocation(IBSymbol);
    gctUINT                      i, arrayLength, offset;
    gctCHAR                      mixName[__MAX_SYM_NAME_LENGTH__];
    VIR_Uniform                  *symUniform = gcvNULL;
    gctUINT                      upcomingRegCount = 0;

    /* Clear the ID list first. */
    if (IdList)
    {
        VIR_IdList_Count(IdList) = 0;
    }

    /* Get the array length. */
    if (VIR_Type_isArray(type))
    {
        arrayLength = VIR_Type_GetArrayLength(type);
        if(AllocMemberReg)
        {
            upcomingRegCount = VIR_Type_GetVirRegCount(Shader, type, -1);
        }
    }
    else
    {
        arrayLength = 1;
    }

    gcmASSERT(arrayLength > 0);

    /* Check UBO/SBO/IOB. */
    if (symbolKind == VIR_SYM_UBO)
    {
        uniformBlock = IBSymbol->u2.ubo;
        blockIndex = uniformBlock->blockIndex;
        blockMemberSymbolKind = VIR_SYM_UNIFORM;
    }
    else if (symbolKind == VIR_SYM_SBO)
    {
        storageBlock = IBSymbol->u2.sbo;
        if (VIR_IB_GetFlags(storageBlock) & VIR_IB_UNSIZED)
        {
            arrayLength = 1;
        }
        blockIndex = storageBlock->blockIndex;
        blockMemberSymbolKind = VIR_SYM_VARIABLE;
    }
    else if (symbolKind == VIR_SYM_IOBLOCK)
    {
        ioBlock = IBSymbol->u2.ioBlock;
        blockIndex = ioBlock->blockIndex;
        blockMemberSymbolKind = VIR_SYM_VARIABLE;
        blockMemberStorageClass = VIR_IOBLOCK_GetStorage(ioBlock);
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }

    /* Get the instance name if existed. */
    gcoOS_StrStr(blockName, ".", &instanceName);

    if (instanceName)
    {
        blockName = instanceName + 1;
    }

    /* Create the base address uniform if needed.*/
    if (CreateBaseAddr)
    {
        if (IsBaseAddrArray)
        {
            baseAddrArraySymId = (VIR_SymId *)vscMM_Alloc(Context->pMM,
                arrayLength * sizeof(VIR_SymId));

            /* Make sure that the sym IDs are consecutive so that we can get the correct
            ** symbol ID in function _ReplaceInterfaceBlockWithBaseSymbol.
            */
            for (i = 0; i < arrayLength; i++)
            {
                /* Generate array index name: 'symName_BaseAddr[%d]'. */
                offset = 0;
                gcoOS_PrintStrSafe(mixName,
                                   __MAX_SYM_NAME_LENGTH__,
                                   &offset,
                                   "%s_BaseAddr[%d]",
                                   blockName,
                                   i);
                errCode = VIR_Shader_AddString(Shader,
                                               mixName,
                                               &nameId);
                CHECK_ERROR(errCode, "VIR_Shader_AddString failed.");

                errCode = _AllocateBaseAddrUniformForIB(Shader,
                                                        IBSymbol,
                                                        symbolKind,
                                                        nameId,
                                                        1,
                                                        &baseAddrArraySymId[i]);
                CHECK_ERROR(errCode, "_AllocateBaseAddrUniformForIB failed.");

                if (baseAddrSymId == VIR_INVALID_ID)
                {
                    baseAddrSymId = baseAddrArraySymId[i];
                }
            }
        }
        else
        {
            /* Generate array index name: 'symName_BaseAddr'. */
            offset = 0;
            gcoOS_PrintStrSafe(mixName,
                               __MAX_SYM_NAME_LENGTH__,
                               &offset,
                               "%s_BaseAddr",
                               blockName);
            errCode = VIR_Shader_AddString(Shader,
                                           mixName,
                                           &nameId);
            CHECK_ERROR(errCode, "VIR_Shader_AddString failed.");

            errCode = _AllocateBaseAddrUniformForIB(Shader,
                                                    IBSymbol,
                                                    symbolKind,
                                                    nameId,
                                                    arrayLength,
                                                    &baseAddrSymId);
            CHECK_ERROR(errCode, "_AllocateBaseAddrUniformForIB failed.");
        }

        /* Save the base address symbol ID. */
        if (symbolKind == VIR_SYM_UBO)
        {
            VIR_UBO_SetBaseAddress(uniformBlock, baseAddrSymId);
        }
        else
        {
            gcmASSERT(symbolKind == VIR_SYM_SBO);
            VIR_SBO_SetBaseAddress(storageBlock, baseAddrSymId);

            symbol = VIR_Shader_GetSymFromId(Shader, baseAddrSymId);
            symUniform = VIR_Symbol_GetUniform(symbol);
            symUniform->u.parentSSBO = storageBlock->sym;
        }
    }

    symbol = IBSymbol;
    baseType = type;
    if (VIR_Type_isArray(baseType))
    {
        /* Get the array base type. */
        baseType = VIR_Shader_GetTypeFromId(Shader, VIR_Type_GetBaseTypeId(baseType));
    }
    for (i = 0; i < arrayLength; i++)
    {
        /* Generate array entries if needed.*/
        if (SplitArray && VIR_Type_isArray(type))
        {
            /* Generate array index name: 'symName[index]'. */
            offset = 0;
            gcoOS_PrintStrSafe(mixName,
                               __MAX_SYM_NAME_LENGTH__,
                               &offset,
                               "%s[%d]",
                               blockName,
                               i);
            errCode = VIR_Shader_AddString(Shader,
                                           mixName,
                                           &nameId);
            CHECK_ERROR(errCode, "VIR_Shader_AddString failed.");

            /* Create the IB. */
            errCode = VIR_Shader_AddSymbol(Shader,
                                           symbolKind,
                                           nameId,
                                           baseType,
                                           VIR_STORAGE_UNKNOWN,
                                           &symId);
            CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed");
            gcmASSERT(symId != VIR_INVALID_ID);
            symbol = VIR_Shader_GetSymFromId(Shader, symId);

            if (isSymSkipNameCheck(IBSymbol))
            {
                VIR_Symbol_SetFlag(symbol, VIR_SYMFLAG_SKIP_NAME_CHECK);
            }

            /* Set general info. */
            VIR_Symbol_SetPrecision(symbol, VIR_Symbol_GetPrecision(IBSymbol));
            if (symbolKind == VIR_SYM_UBO)
            {
                VIR_Symbol_SetAddrSpace(symbol, VIR_AS_CONSTANT);
                VIR_Symbol_SetTyQualifier(symbol, VIR_TYQUAL_CONST);
            }
            else if (symbolKind == VIR_SYM_SBO)
            {
            }
            else if (symbolKind == VIR_SYM_IOBLOCK)
            {
            }
            /* Set layout info */
            VIR_Symbol_SetLayoutQualifier(symbol, VIR_Symbol_GetLayoutQualifier(IBSymbol));
            VIR_Symbol_SetLocation(symbol, 0);
            VIR_Symbol_SetBinding(symbol, VIR_Symbol_GetBinding(IBSymbol));

            if (symbolKind == VIR_SYM_UBO)
            {
                arrayMemberUniformBlock = symbol->u2.ubo;
                arrayMemberUniformBlock->sym = symId;
                arrayMemberUniformBlock->blockSize = VIR_UBO_GetBlockSize(uniformBlock);
                VIR_UBO_SetFlag(arrayMemberUniformBlock, VIR_UBO_GetFlags(uniformBlock));
                if (IsBaseAddrArray)
                {
                    VIR_UBO_SetBaseAddress(arrayMemberUniformBlock, baseAddrArraySymId[i]);
                }
                else
                {
                    VIR_UBO_SetBaseAddress(arrayMemberUniformBlock, baseAddrSymId);
                }
                blockIndex = arrayMemberUniformBlock->blockIndex;
            }
            else if (symbolKind == VIR_SYM_SBO)
            {
                arrayMemberStorageBlock = symbol->u2.sbo;
                arrayMemberStorageBlock->sym = symId;
                arrayMemberStorageBlock->blockSize = VIR_SBO_GetBlockSize(storageBlock);
                VIR_SBO_SetFlag(arrayMemberStorageBlock, VIR_SBO_GetFlags(storageBlock));
                if (IsBaseAddrArray)
                {
                    VIR_SBO_SetBaseAddress(arrayMemberStorageBlock, baseAddrArraySymId[i]);
                }
                else
                {
                    VIR_SBO_SetBaseAddress(arrayMemberStorageBlock, baseAddrSymId);
                }
                blockIndex = arrayMemberStorageBlock->blockIndex;
            }
            else if (symbolKind == VIR_SYM_IOBLOCK)
            {
                arrayMemberIoBlock = symbol->u2.ioBlock;
                arrayMemberIoBlock->sym = symId;
                VIR_IOBLOCK_SetBlockNameLength(arrayMemberIoBlock, gcoOS_StrLen(mixName, gcvNULL));
                VIR_IOBLOCK_SetInstanceNameLength(arrayMemberIoBlock, VIR_IOBLOCK_GetInstanceNameLength(ioBlock));
                VIR_IB_SetFlag(arrayMemberIoBlock, VIR_IB_GetFlags(ioBlock));
                blockIndex = arrayMemberIoBlock->blockIndex;
            }
        }
        else
        {
            offset = 0;
            if (i == 0)
            {
                gcoOS_PrintStrSafe(mixName,
                    __MAX_SYM_NAME_LENGTH__,
                    &offset,
                    "%s",
                    blockName);
            }
            else
            {
                gcoOS_PrintStrSafe(mixName,
                    __MAX_SYM_NAME_LENGTH__,
                    &offset,
                    "%s[%d]",
                    blockName,
                    i);
            }
        }

        /* Generate block member if needed. */
        if (GenerateBlockMember)
        {
            errCode = _SplitStructVariable(Shader,
                                           symbol,
                                           symbol,
                                           blockMemberSymbolKind,
                                           blockMemberStorageClass,
                                           mixName,
                                           baseType,
                                           blockIndex,
                                           SymFlag,
                                           gcvFALSE,
                                           AllocMemberSym,
                                           AllocMemberReg,
                                           &upcomingRegCount,
                                           (location == -1) ? gcvNULL : &location,
                                           &firstElementId,
                                           IdList);
            CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed");

            if (AllocMemberReg)
            {
                VIR_Symbol_SetFirstElementId(symbol, firstElementId);
            }
        }
    }

    return errCode;
}

static VSC_ErrCode _VIR_HL_Reg_Alloc(
    IN  VIR_Shader              *Shader,
    IN  VIR_PatternHL2HLContext *Context
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_UniformIdList           *uniformList = VIR_Shader_GetUniforms(Shader);
    VIR_UBOIdList               *uboList = VIR_Shader_GetUniformBlocks(Shader);
    VIR_VariableIdList          *variableList = VIR_Shader_GetVaribles(Shader);
    VIR_OutputIdList            *outputList = VIR_Shader_GetOutputs(Shader);
    VIR_InputIdList             *inputList = VIR_Shader_GetAttributes(Shader);
    VIR_InputIdList             *perPatchInputList = VIR_Shader_GetPerpatchAttributes(Shader);
    VIR_OutputIdList            *perPatchOutputList = VIR_Shader_GetPerpatchOutputs(Shader);
    VIR_SBOIdList               *sboList = VIR_Shader_GetSSBlocks(Shader);
    VIR_IOBIdList               *iobList = VIR_Shader_GetIOBlocks(Shader);
    VIR_IdList                  *idList = gcvNULL;
    VIR_FuncIterator             func_iter;
    VIR_FunctionNode            *func_node;
    VIR_Symbol                  *symbol = gcvNULL;
    VIR_SymId                    symId;
    VIR_VirRegId                 regId;
    VIR_StorageClass             storageClass;
    VIR_SymFlag                  symFlag;
    gctINT                       location;
    gctUINT                      listLength;
    gctUINT                      i;
    gctBOOL                      hasNumWorkGroups = gcvFALSE;

    VIR_Shader_UpdateVirRegCount(Shader, 256);

    /* the first three are reserved for temp virtual register for the operands
       when used in array indexing */
    regId = VIR_Shader_NewVirRegId(Shader, 3);
    for(i = 0; i < 3; i++)
    {
        errCode = VIR_Shader_AddSymbol(Shader,
                                       VIR_SYM_VIRREG,
                                       regId + i,
                                       VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UNKNOWN),
                                       VIR_STORAGE_UNKNOWN,
                                       &symId);
        CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed.");
    }

    /* Initialize a ID list to save the symbol ID. */
    errCode = VIR_IdList_Init(&Shader->pmp.mmWrapper, 32, &idList);
    CHECK_ERROR(errCode, "InitIdList");

    /* Uniforms:
    ** Split struct uniforms.
    */
    listLength = VIR_IdList_Count(uniformList);
    for (i = 0; i < listLength; i++)
    {
        symbol = VIR_Shader_GetSymFromId(Shader, VIR_IdList_GetId(uniformList, i));
        storageClass = VIR_Symbol_GetStorageClass(symbol);

        if (VIR_Symbol_GetName(symbol) == VIR_NAME_NUM_GROUPS)
        {
            hasNumWorkGroups = gcvTRUE;
        }

        if (VIR_Type_IsBaseTypeStruct(Shader, VIR_Symbol_GetType(symbol)))
        {
            location = VIR_Symbol_GetLocation(symbol);
            errCode = _SplitUniforms(Shader,
                                     uniformList,
                                     i,
                                     symbol,
                                     storageClass,
                                     (location == -1) ? gcvNULL : &location);
            CHECK_ERROR(errCode, "_SplitUniforms failed.");
        }
    }
    /* always generate numWorkGroups uniform */
    if (!hasNumWorkGroups &&
        VIR_Shader_GetKind(Shader) == VIR_SHADER_COMPUTE)
    {
        VIR_SymId   uniformSymId;
        VIR_Symbol  *uniformSym;
        VIR_Uniform *virUniform = gcvNULL;

        /* default uniform symbol */
        errCode = VIR_Shader_AddSymbol(Shader,
                                       VIR_SYM_UNIFORM,
                                       VIR_NAME_NUM_GROUPS,
                                       VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT_X3),
                                       VIR_STORAGE_UNKNOWN,
                                       &uniformSymId);

        uniformSym = VIR_Shader_GetSymFromId(Shader, uniformSymId);
        VIR_Symbol_SetUniformKind(uniformSym, VIR_UNIFORM_NUM_GROUPS);
        VIR_Symbol_SetFlag(uniformSym, VIR_SYMUNIFORMFLAG_USED_IN_SHADER);
        VIR_Symbol_SetFlag(uniformSym, VIR_SYMFLAG_COMPILER_GEN);
        VIR_Symbol_SetLocation(uniformSym, -1);

        virUniform = VIR_Symbol_GetUniform(uniformSym);
        virUniform->index = VIR_IdList_Count(VIR_Shader_GetUniforms(Shader)) - 1;
    }

    /* UBO:
    **  Split array UBO, and generate base address uniform and all UBO members.
    */
    listLength = VIR_IdList_Count(uboList);
    for (i = 0; i < listLength; i++)
    {
        symbol = VIR_Shader_GetSymFromId(Shader, VIR_IdList_GetId(uboList, i));

        if (!VIR_Symbol_HasFlag(symbol, VIR_SYMFLAG_WITHOUT_REG))
        {
            continue;
        }

        symFlag = VIR_SYMFLAG_NONE;

        errCode = _AllocateInterfaceBlock(Context,
                                          Shader,
                                          symbol,
                                          symFlag,
                                          gcvTRUE,
                                          gcvFALSE,
                                          gcvTRUE,
                                          gcvFALSE,
                                          gcvFALSE,
                                          gcvFALSE,
                                          idList);
        CHECK_ERROR(errCode, "_AllocateInterfaceBlock failed.");

        /* Mark symbol as reg allocated. */
        VIR_Symbol_ClrFlag(symbol, VIR_SYMFLAG_WITHOUT_REG);
    }

    /* Variables:
    **  Split struct variables and allocate regs for all variables.
    */
    listLength = VIR_IdList_Count(variableList);
    for (i = 0; i < listLength; i++)
    {
        symbol = VIR_Shader_GetSymFromId(Shader, VIR_IdList_GetId(variableList, i));
        storageClass = VIR_Symbol_GetStorageClass(symbol);

        if (!VIR_Symbol_HasFlag(symbol, VIR_SYMFLAG_WITHOUT_REG))
        {
            continue;
        }

        location = VIR_Symbol_GetLocation(symbol);

        errCode = _SplitVariables(Shader,
                                  variableList,
                                  i,
                                  symbol,
                                  storageClass,
                                  (location == -1) ? gcvNULL : &location);
        CHECK_ERROR(errCode, "_SplitVariables failed.");

        /* Mark symbol as reg allocated. */
        VIR_Symbol_ClrFlag(symbol, VIR_SYMFLAG_WITHOUT_REG);
    }

    /* Inputs:
    **  Split struct inputs, and allocate regs for all inputs.
    */
    listLength = VIR_IdList_Count(inputList);
    for (i = 0; i < listLength; i++)
    {
        symbol = VIR_Shader_GetSymFromId(Shader, VIR_IdList_GetId(inputList, i));
        storageClass = VIR_Symbol_GetStorageClass(symbol);

        if (!VIR_Symbol_HasFlag(symbol, VIR_SYMFLAG_WITHOUT_REG))
        {
            continue;
        }

        location = VIR_Symbol_GetLocation(symbol);

        /* need to generate LocatInvocationIndex */
        if (VIR_Symbol_GetName(symbol) == VIR_NAME_LOCALINVOCATIONINDEX)
        {
            errCode = _GenInvocationIndex(Shader, symbol);
            CHECK_ERROR(errCode, "_SplitVariables failed.");
        }
        else if (VIR_Symbol_GetName(symbol) == VIR_NAME_WORK_GROUP_INDEX)
        {
            errCode = _GenWorkGroupIndex(Shader, symbol);
            CHECK_ERROR(errCode, "_SplitVariables failed.");
        }
        else if (VIR_Symbol_GetName(symbol) == VIR_NAME_INSTANCE_INDEX)
        {
            errCode = _GenInstanceIndex(Shader, symbol);
            CHECK_ERROR(errCode, "_SplitVariables failed.");
        }
        else
        {
            errCode = _SplitVariables(Shader,
                                      inputList,
                                      i,
                                      symbol,
                                      storageClass,
                                      (location == -1) ? gcvNULL : &location);
            CHECK_ERROR(errCode, "_SplitVariables failed.");
        }

        /* Mark symbol as reg allocated. */
        VIR_Symbol_ClrFlag(symbol, VIR_SYMFLAG_WITHOUT_REG);
    }

    /* perPatchInputs:
    **  Split struct inputs, and allocate regs for all inputs.
    */
    listLength = VIR_IdList_Count(perPatchInputList);
    for (i = 0; i < listLength; i++)
    {
        symbol = VIR_Shader_GetSymFromId(Shader, VIR_IdList_GetId(perPatchInputList, i));
        storageClass = VIR_Symbol_GetStorageClass(symbol);

        if (!VIR_Symbol_HasFlag(symbol, VIR_SYMFLAG_WITHOUT_REG))
        {
            continue;
        }

        location = VIR_Symbol_GetLocation(symbol);

        /* need to generate LocatInvocationIndex */
        if (VIR_Symbol_GetName(symbol) == VIR_NAME_LOCALINVOCATIONINDEX)
        {
            errCode = _GenInvocationIndex(Shader, symbol);
            CHECK_ERROR(errCode, "_SplitVariables failed.");
        }
        else if (VIR_Symbol_GetName(symbol) == VIR_NAME_WORK_GROUP_INDEX)
        {
            errCode = _GenWorkGroupIndex(Shader, symbol);
            CHECK_ERROR(errCode, "_SplitVariables failed.");
        }
        else
        {
            errCode = _SplitVariables(Shader,
                                      perPatchInputList,
                                      i,
                                      symbol,
                                      storageClass,
                                      (location == -1) ? gcvNULL : &location);
            CHECK_ERROR(errCode, "_SplitVariables failed.");
        }

        /* Mark symbol as reg allocated. */
        VIR_Symbol_ClrFlag(symbol, VIR_SYMFLAG_WITHOUT_REG);
    }

    /* Outputs:
    **  Split struct and array outputs, and allocate regs for all outputs.
    */
    listLength = VIR_IdList_Count(outputList);
    for (i = 0; i < listLength; i++)
    {
        symbol = VIR_Shader_GetSymFromId(Shader, VIR_IdList_GetId(outputList, i));
        storageClass = VIR_Symbol_GetStorageClass(symbol);

        if (!VIR_Symbol_HasFlag(symbol, VIR_SYMFLAG_WITHOUT_REG))
        {
            continue;
        }

        location = VIR_Symbol_GetLocation(symbol);

        errCode = _SplitOutputs(Shader,
                                outputList,
                                i,
                                symbol,
                                storageClass,
                                (location == -1) ? gcvNULL : &location);
        CHECK_ERROR(errCode, "_SplitOutputs failed.");

        /* Mark symbol as reg allocated. */
        VIR_Symbol_ClrFlag(symbol, VIR_SYMFLAG_WITHOUT_REG);
    }

    /* perPatchOutput:
    **  Split struct outputs, and allocate regs for all outputs.
    */
    listLength = VIR_IdList_Count(perPatchOutputList);
    for (i = 0; i < listLength; i++)
    {
        symbol = VIR_Shader_GetSymFromId(Shader, VIR_IdList_GetId(perPatchOutputList, i));
        storageClass = VIR_Symbol_GetStorageClass(symbol);

        if (!VIR_Symbol_HasFlag(symbol, VIR_SYMFLAG_WITHOUT_REG))
        {
            continue;
        }

        location = VIR_Symbol_GetLocation(symbol);

        errCode = _SplitOutputs(Shader,
                                perPatchOutputList,
                                i,
                                symbol,
                                storageClass,
                                (location == -1) ? gcvNULL : &location);
        CHECK_ERROR(errCode, "_SplitOutputs failed.");

        /* Mark symbol as reg allocated. */
        VIR_Symbol_ClrFlag(symbol, VIR_SYMFLAG_WITHOUT_REG);
    }

    /* SBO:
    **  Split array SBO, generate base address.
    */
    listLength = VIR_IdList_Count(sboList);
    for (i = 0; i < listLength; i++)
    {
        symbol = VIR_Shader_GetSymFromId(Shader, VIR_IdList_GetId(sboList, i));

        if (!VIR_Symbol_HasFlag(symbol, VIR_SYMFLAG_WITHOUT_REG))
        {
            continue;
        }

        symFlag = VIR_SYMFLAG_NONE;

        errCode = _AllocateInterfaceBlock(Context,
                                          Shader,
                                          symbol,
                                          symFlag,
                                          gcvTRUE,
                                          gcvTRUE,
                                          gcvTRUE,
                                          gcvFALSE,
                                          gcvTRUE,
                                          gcvFALSE,
                                          idList);
        CHECK_ERROR(errCode, "_AllocateInterfaceBlock failed.");

        /* Mark symbol as reg allocated. */
        VIR_Symbol_ClrFlag(symbol, VIR_SYMFLAG_WITHOUT_REG);
    }

    /* IOB:
    **  Allocate reg/sym for IOB members.
    */
    listLength = VIR_IdList_Count(iobList);
    for (i = 0; i < listLength; i++)
    {
        symbol = VIR_Shader_GetSymFromId(Shader, VIR_IdList_GetId(iobList, i));

        if (!VIR_Symbol_HasFlag(symbol, VIR_SYMFLAG_WITHOUT_REG))
        {
            continue;
        }

        symFlag = VIR_SYMFLAG_IS_IOBLOCK_MEMBER;

        if (VIR_IOBLOCK_GetInstanceNameLength(symbol->u2.ioBlock) > 0)
        {
            symFlag |= VIR_SYMFLAG_IS_INSTANCE_MEMBER;
        }

        /* If this IO block is per-vertex array, all its member are per-vertex array. */
        if (isSymArrayedPerVertex(symbol))
        {
            symFlag |= VIR_SYMFLAG_ARRAYED_PER_VERTEX;
        }

        errCode = _AllocateInterfaceBlock(Context,
                                          Shader,
                                          symbol,
                                          symFlag,
                                          gcvFALSE,
                                          gcvTRUE,
                                          gcvFALSE,
                                          gcvTRUE,
                                          gcvTRUE,
                                          gcvTRUE,
                                          idList);
        CHECK_ERROR(errCode, "_AllocateInterfaceBlock failed.");

        /* Mark symbol as reg allocated. */
        VIR_Symbol_ClrFlag(symbol, VIR_SYMFLAG_WITHOUT_REG);
    }

    /* Allocate register for the variables belong to the function only. */
    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(Shader));

    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL;
         func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function        *func = func_node->function;
        VIR_VariableIdList  *paramList = VIR_Function_GetParameters(func);
        VIR_VariableIdList  *localVarList = VIR_Function_GetLocalVar(func);

        /* Allocate register for parameters. */
        listLength = VIR_IdList_Count(paramList);
        for (i = 0; i < listLength; i++)
        {
            symbol = VIR_Function_GetSymFromId(func, VIR_IdList_GetId(paramList, i));
            storageClass = VIR_Symbol_GetStorageClass(symbol);

            if (!VIR_Symbol_HasFlag(symbol, VIR_SYMFLAG_WITHOUT_REG))
            {
                continue;
            }

            location = VIR_Symbol_GetLocation(symbol);

            errCode = _SplitVariables(Shader,
                                      paramList,
                                      i,
                                      symbol,
                                      storageClass,
                                      (location == -1) ? gcvNULL : &location);
            CHECK_ERROR(errCode, "_SplitVariables failed.");

            /* Mark symbol as reg allocated. */
            VIR_Symbol_ClrFlag(symbol, VIR_SYMFLAG_WITHOUT_REG);
        }

        /* Allocate register for local variables. */
        listLength = VIR_IdList_Count(localVarList);
        for (i = 0; i < listLength; i++)
        {
            symbol = VIR_Function_GetSymFromId(func, VIR_IdList_GetId(localVarList, i));
            storageClass = VIR_Symbol_GetStorageClass(symbol);

            if (!VIR_Symbol_HasFlag(symbol, VIR_SYMFLAG_WITHOUT_REG))
            {
                continue;
            }

            location = VIR_Symbol_GetLocation(symbol);

            errCode = _SplitVariables(Shader,
                                      localVarList,
                                      i,
                                      symbol,
                                      storageClass,
                                      (location == -1) ? gcvNULL : &location);
            CHECK_ERROR(errCode, "_SplitVariables failed.");

            /* Mark symbol as reg allocated. */
            VIR_Symbol_ClrFlag(symbol, VIR_SYMFLAG_WITHOUT_REG);
        }
    }

    return errCode;
}

static VSC_ErrCode
_ReplaceSymWithFirstElementSym(
    IN  VIR_Shader              *Shader,
    IN  VIR_Function            *Func,
    IN  VIR_Operand             *Operand
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_Function                *func = gcvNULL;
    VIR_Symbol                  *operandSym = VIR_Operand_GetSymbol(Operand);
    VIR_Symbol                  *elementSym;
    VIR_Symbol                  *virRegSym;
    VIR_SymId                    elementSymId, virRegSymId;
    VIR_VirRegId                 virRegId;

    gcmASSERT(!isSymCombinedSampler(operandSym));

    elementSymId = VIR_Symbol_GetFirstElementId(operandSym);
    if (VIR_Id_isFunctionScope(VIR_Symbol_GetIndex(operandSym)))
    {
        func = VIR_Symbol_GetHostFunction(operandSym);
        elementSym = VIR_Function_GetSymFromId(func, elementSymId);
    }
    else
    {
        elementSym = VIR_Shader_GetSymFromId(Shader, elementSymId);

        if (VIR_Symbol_isUniform(elementSym) &&
            VIR_Symbol_GetUniformKind(elementSym) == VIR_UNIFORM_PUSH_CONSTANT)
        {
            /* SPIV generates constIndexing for field access(will change) */
            if (VIR_Operand_GetIsConstIndexing(Operand) &&
                !VIR_Type_isArray(VIR_Symbol_GetType(elementSym)))
            {
                elementSymId += VIR_Operand_GetConstIndexingImmed(Operand);
                VIR_Operand_SetIsConstIndexing(Operand, gcvFALSE);
                VIR_Operand_SetRelIndex(Operand, 0);
                elementSym = VIR_Shader_GetSymFromId(Shader, elementSymId);
                gcmASSERT(VIR_Symbol_isUniform(elementSym) &&
                          VIR_Symbol_GetUniformKind(elementSym) == VIR_UNIFORM_PUSH_CONSTANT);
            }
        }
    }

    if (!VIR_Symbol_NeedReplaceSymWithReg(elementSym))
    {
        VIR_Operand_SetSym(Operand, elementSym);
    }
    else
    {
        gcmASSERT(VIR_Symbol_GetKind(elementSym) == VIR_SYM_VARIABLE);

        virRegId = VIR_Symbol_GetVregIndex(elementSym);

        if (VIR_Operand_GetIsConstIndexing(Operand))
        {
            virRegId += VIR_Operand_GetConstIndexingImmed(Operand);
            VIR_Operand_SetIsConstIndexing(Operand, gcvFALSE);
            VIR_Operand_SetRelIndex(Operand, 0);
        }

        errCode = VIR_Shader_GetVirRegSymByVirRegId(Shader,
                                                    virRegId,
                                                    &virRegSymId);
        CHECK_ERROR(errCode, "VIR_Shader_GetVirRegSymByVirRegId failed.");
        virRegSym = VIR_Shader_GetSymFromId(Shader, virRegSymId);
        gcmASSERT(virRegSym);

        VIR_Operand_SetSym(Operand, virRegSym);
        /* TODO: do we need to change kind to VIR_OPND_VIRREG? */
        VIR_Operand_SetOpKind(Operand, VIR_OPND_SYMBOL);
    }

    return errCode;
}

static VSC_ErrCode
_ReplaceInterfaceBlockWithBaseSymbol(
    IN  VIR_Shader              *Shader,
    IN  VIR_Function            *Func,
    IN  VIR_Operand             *Operand
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_Symbol                  *opndSym = VIR_Operand_GetSymbol(Operand);
    VIR_Symbol                  *replaceSym = gcvNULL;
    VIR_SymId                    replaceSymId = VIR_INVALID_ID;
    VIR_Type                    *type;

    /* For UBO/SBO, use base address uniform to replace the interface block. */
    if (VIR_Symbol_isUBO(opndSym))
    {
        replaceSymId = VIR_UBO_GetBaseAddress(opndSym->u2.ubo);
    }
    else if (VIR_Symbol_isSBO(opndSym))
    {
        replaceSymId = VIR_UBO_GetBaseAddress(opndSym->u2.sbo);

    }
    /* IOB should be handled in _ReplaceSymWithFirstElementSym. */
    else if (VIR_Symbol_isIOB(opndSym))
    {
        gcmASSERT(gcvFALSE);
        return errCode;
    }
    else
    {
        gcmASSERT(gcvFALSE);
        return errCode;
    }

    /* Get the replace symbol. */
    gcmASSERT(replaceSymId != VIR_INVALID_ID);
    replaceSym = VIR_Shader_GetSymFromId(Shader, replaceSymId);

    /* Get the replace symbol type, if it is an array, use the base type. */
    type = VIR_Symbol_GetType(replaceSym);
    if (VIR_Type_isArray(type))
    {
        type = VIR_Shader_GetTypeFromId(Shader, VIR_Type_GetBaseTypeId(type));
    }

    /* Update the symbol and operand type. */
    VIR_Operand_SetTypeId(Operand, VIR_Type_GetIndex(type));
    VIR_Operand_SetSym(Operand, replaceSym);
    VIR_Operand_SetPrecision(Operand, VIR_Symbol_GetPrecision(replaceSym));

    return errCode;
}

static VSC_ErrCode
_ReplaceVariableWithVirReg(
    IN  VIR_Shader              *Shader,
    IN  VIR_Function            *Func,
    IN  VIR_Operand             *Operand
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_Symbol                  *opndSym = VIR_Operand_GetSymbol(Operand);
    VIR_VirRegId                 virRegId = VIR_Symbol_GetVregIndex(opndSym);
    VIR_SymId                    virRegSymId;
    VIR_Symbol                  *virRegSym;
    gctBOOL                      isInput = VIR_Symbol_isPerPatchInput(opndSym) || VIR_Symbol_isAttribute(opndSym);
    gctBOOL                      needReplaceSymWithReg = VIR_Symbol_NeedReplaceSymWithReg(opndSym);
    gctBOOL                      isConstIndex = gcvFALSE;

    if (!isInput && !needReplaceSymWithReg)
    {
        return errCode;
    }

    /* Add the constant indexing to vir reg. */
    if (VIR_Operand_GetIsConstIndexing(Operand))
    {
        virRegId += VIR_Operand_GetConstIndexingImmed(Operand);
        isConstIndex = gcvTRUE;
    }

    /* Add the matrix const index if needed. */
    virRegId += VIR_Operand_GetMatrixConstIndex(Operand);

    errCode = VIR_Shader_GetVirRegSymByVirRegId(Shader,
                                                virRegId,
                                                &virRegSymId);
    CHECK_ERROR(errCode, "VIR_Shader_GetVirRegSymByVirRegId failed.");
    virRegSym = VIR_Shader_GetSymFromId(Shader, virRegSymId);
    gcmASSERT(virRegSym);

    if (needReplaceSymWithReg)
    {
        /* Clean the array indexing. */
        if (isConstIndex)
        {
            VIR_Operand_SetIsConstIndexing(Operand, gcvFALSE);
            VIR_Operand_SetRelIndex(Operand, 0);
        }

        /* Clean the matrix index. */
        VIR_Operand_SetMatrixConstIndex(Operand, 0);

        VIR_Operand_SetSym(Operand, virRegSym);
    }
    else
    {
        gcmASSERT(isInput);
        if (VIR_Symbol_GetVregVariable(virRegSym) != opndSym)
        {
            /* Clean the array indexing. */
            if (isConstIndex)
            {
                VIR_Operand_SetIsConstIndexing(Operand, gcvFALSE);
                VIR_Operand_SetRelIndex(Operand, 0);
            }

            VIR_Operand_SetSym(Operand, VIR_Symbol_GetVregVariable(virRegSym));
        }
    }


    return errCode;
}

static VSC_ErrCode
_ReplaceOperandSymbol(
    IN  VIR_Shader              *Shader,
    IN  VIR_Function            *Func,
    IN  VIR_Operand             *Operand
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_Symbol                  *opndSym;
    VIR_Symbol                  *indexSym = gcvNULL;
    VIR_SymId                    indexSymId;
    gctUINT                      i;

    if (Operand == gcvNULL)
    {
        return errCode;
    }

    if ((VIR_Operand_isSymbol(Operand) || VIR_Operand_isVirReg(Operand)) &&
        !VIR_Operand_GetIsConstIndexing(Operand) &&
        VIR_Operand_GetRelAddrMode(Operand) != VIR_INDEXED_NONE)
    {
        indexSymId = VIR_Operand_GetRelIndexing(Operand);
        indexSym = VIR_Shader_GetSymFromId(Shader, indexSymId);

        if (VIR_Symbol_NeedReplaceSymWithReg(indexSym))
        {
            VIR_VirRegId          virRegId;
            VIR_SymId             virRegSymId;

            virRegId = VIR_Symbol_GetVregIndex(indexSym);
            errCode = VIR_Shader_GetVirRegSymByVirRegId(Shader,
                                                        virRegId,
                                                        &virRegSymId);
            CHECK_ERROR(errCode, "VIR_Shader_GetVirRegSymByVirRegId failed.");

            VIR_Operand_SetRelIndex(Operand, virRegSymId);
        }
    }

    if (VIR_Operand_isParameters(Operand))
    {
        VIR_ParmPassing *parm = VIR_Operand_GetParameters(Operand);
        for (i = 0; i < parm->argNum; i++)
        {
            _ReplaceOperandSymbol(Shader, Func, parm->args[i]);
        }
    }
    else if (VIR_Operand_isTexldParm(Operand))
    {
        VIR_Operand *texldOperand = (VIR_Operand*)Operand;

        for (i = 0; i < VIR_TEXLDMODIFIER_COUNT; ++i)
        {
            _ReplaceOperandSymbol(Shader, Func, VIR_Operand_GetTexldModifier(texldOperand,i));
        }
    }

    /* Ignore non-symbol operands. */
    if (!VIR_Operand_isSymbol(Operand))
    {
        return errCode;
    }

    /* Ignore simple symbol operands. */
    opndSym = VIR_Operand_GetSymbol(Operand);
    gcmASSERT(!VIR_Symbol_HasFlag(opndSym, VIR_SYMFLAG_WITHOUT_REG));

    /* Ignore combined sampler sym */
    if (isSymCombinedSampler(opndSym))
    {
        return errCode;
    }

    /* I:
    ** If the firstElementId of a symbol is not INVALID, replace this symbol by its first element.
    */
    if (VIR_Symbol_GetFirstElementId(opndSym) != VIR_INVALID_ID)
    {
        errCode = _ReplaceSymWithFirstElementSym(Shader, Func, Operand);
        CHECK_ERROR(errCode, "_ReplaceSymWithFirstElementSym failed.");
    }
    else if (VIR_Symbol_isIB(opndSym))
    {
        errCode = _ReplaceInterfaceBlockWithBaseSymbol(Shader, Func, Operand);
        CHECK_ERROR(errCode, "_ReplaceInterfaceBlockWithBaseSymbol failed.");
    }
    /* Replace the variable symbol with vir reg symbol if needed. */
    else
    {
        errCode = _ReplaceVariableWithVirReg(Shader, Func, Operand);
        CHECK_ERROR(errCode, "_ReplaceVariableWithVirReg failed.");
    }

    return errCode;
}

static VSC_ErrCode _VIR_HL_Expand_FuncArguments(
    IN  VIR_Shader              *Shader
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_FuncIterator             func_iter;
    VIR_FunctionNode            *func_node;

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(Shader));

    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL;
         func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function        *func = func_node->function;
        VIR_Instruction     *inst = func->instList.pHead;
        VIR_Instruction     *nextInst;

        for(; inst != gcvNULL; inst = VIR_Inst_GetNext(inst))
        {
            VIR_Function    *pCalleeFunc;
            VIR_Operand     *pFuncOpnd;
            VIR_Operand     *pArgOpnd;
            VIR_ParmPassing *opndParm = gcvNULL;
            VIR_SymId        parmSymId;
            VIR_Symbol      *parmSym;
            VIR_Type        *parmType = gcvNULL;
            VIR_Enable       movEnable = VIR_ENABLE_NONE;
            VIR_Instruction *newInst = gcvNULL;
            VIR_Operand     *newOperand = gcvNULL;
            gctUINT          i, parmCount;

            if (VIR_Inst_GetOpcode(inst) != VIR_OP_PARM)
            {
                continue;
            }
            nextInst = VIR_Inst_GetNext(inst);
            gcmASSERT(VIR_Inst_GetOpcode(nextInst) == VIR_OP_CALL);

            /* Get the argument list. */
            opndParm = VIR_Operand_GetParameters(VIR_Inst_GetSource(inst, 0));

            /* Get the callee function. */
            pFuncOpnd = VIR_Inst_GetDest(inst);
            pCalleeFunc = VIR_Operand_GetFunction(pFuncOpnd);
            parmCount = VIR_IdList_Count(&pCalleeFunc->paramters);
            gcmASSERT(parmCount == opndParm->argNum);

            /* Generate assignments for in/out parameter */
            for (i = 0; i < parmCount; i++)
            {
                /* Get function parameter. */
                parmSymId = VIR_IdList_GetId(&pCalleeFunc->paramters, i);
                parmSym = VIR_Function_GetSymFromId(pCalleeFunc, parmSymId);
                parmType = VIR_Symbol_GetType(parmSym);
                movEnable = VIR_TypeId_isPrimitive(VIR_Type_GetIndex(parmType)) ? VIR_Type_Conv2Enable(parmType) : VIR_ENABLE_XYZW;

                /* Get argument operand. */
                pArgOpnd = opndParm->args[i];

                /* Add a MOV instruction before/after the VIR_OP_CALL instruction. */
                if (VIR_Symbol_GetStorageClass(parmSym) == VIR_STORAGE_INPARM ||
                    VIR_Symbol_GetStorageClass(parmSym) == VIR_STORAGE_INOUTPARM)
                {
                    VIR_Function_AddInstructionBefore(func,
                                                      VIR_OP_MOV,
                                                      VIR_TYPE_UNKNOWN,
                                                      nextInst,
                                                      gcvTRUE,
                                                      &newInst);
                    /* Set DEST. */
                    newOperand = VIR_Inst_GetDest(newInst);
                    VIR_Operand_SetSymbol(newOperand, pCalleeFunc, parmSymId);
                    VIR_Operand_SetEnable(newOperand, movEnable);

                    /* Set SOURCE. */
                    newOperand = VIR_Inst_GetSource(newInst, 0);
                    VIR_Operand_Copy(newOperand, pArgOpnd);
                }
                if (VIR_Symbol_GetStorageClass(parmSym) == VIR_STORAGE_OUTPARM ||
                         VIR_Symbol_GetStorageClass(parmSym) == VIR_STORAGE_INOUTPARM)
                {
                    VIR_Function_AddInstructionAfter(func,
                                                     VIR_OP_MOV,
                                                     VIR_TYPE_UNKNOWN,
                                                     nextInst,
                                                     gcvTRUE,
                                                     &newInst);
                    /* Set DEST. */
                    newOperand = VIR_Inst_GetDest(newInst);
                    VIR_Operand_Copy(newOperand, pArgOpnd);
                    VIR_Operand_SetLvalue(newOperand, 1);
                    VIR_Operand_SetEnable(newOperand, VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(newOperand)));

                    /* Set SOURCE. */
                    newOperand = VIR_Inst_GetSource(newInst, 0);
                    VIR_Operand_SetSymbol(newOperand, pCalleeFunc, parmSymId);
                    VIR_Operand_SetSwizzle(newOperand, VIR_Enable_2_Swizzle_WShift(movEnable));
                }
            }

            /* Change current instruction to NOP. */
            VIR_Inst_SetOpcode(inst, VIR_OP_NOP);
            VIR_Inst_SetConditionOp(inst, VIR_COP_ALWAYS);
            VIR_Inst_SetSrcNum(inst, 0);
            VIR_Inst_SetDest(inst, gcvNULL);
        }
    }

    return errCode;
}

static VSC_ErrCode _VIR_HL_Sym_Replace(
    IN  VIR_Shader              *Shader,
    IN  VIR_PatternHL2HLContext *Context
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_FuncIterator             func_iter;
    VIR_FunctionNode            *func_node;
    gctUINT                      i;

    /*-------------------I: Update symbols-------------------*/
    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(Shader));

    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL;
         func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function        *func = func_node->function;
        VIR_Instruction     *inst = func->instList.pHead;

        for(; inst != gcvNULL; inst = VIR_Inst_GetNext(inst))
        {
            errCode = _ReplaceOperandSymbol(Shader, func, VIR_Inst_GetDest(inst));
            CHECK_ERROR(errCode, "_ReplaceOperandSymbol failed.");

            for (i = 0; i < VIR_Inst_GetSrcNum(inst); i++)
            {
                errCode = _ReplaceOperandSymbol(Shader, func, VIR_Inst_GetSource(inst, i));
                CHECK_ERROR(errCode, "_ReplaceOperandSymbol failed.");
            }
        }
    }

    return errCode;
}

static VSC_ErrCode _VIR_HL_Sym_Delete(
    IN  VIR_Shader              *Shader
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_IdList                  *list;
    VIR_Symbol                  *symbol;
    VIR_Type                    *type;
    gctUINT                      i;

    /*-------------------I: Remove some symbols from ID list-------------------*/
    /* Remove struct outputs. */
    list = VIR_Shader_GetOutputs(Shader);
    for (i = 0; i < VIR_IdList_Count(list);)
    {
        symbol = VIR_Shader_GetSymFromId(Shader, VIR_IdList_GetId(list, i));
        if (!symbol)
        {
            i++;
            continue;
        }

        if (VIR_Type_IsBaseTypeStruct(Shader, VIR_Symbol_GetType(symbol)))
        {
            VIR_IdList_DeleteByIndex(list, i);
        }
        else
        {
            i++;
        }
    }
    VIR_IdList_RenumberIndex(Shader, list);

    /* Remove struct perpatch outputs. */
    list = VIR_Shader_GetPerpatchOutputs(Shader);
    for (i = 0; i < VIR_IdList_Count(list);)
    {
        symbol = VIR_Shader_GetSymFromId(Shader, VIR_IdList_GetId(list, i));
        if (!symbol)
        {
            i++;
            continue;
        }

        if (VIR_Type_IsBaseTypeStruct(Shader, VIR_Symbol_GetType(symbol)))
        {
            VIR_IdList_DeleteByIndex(list, i);
        }
        else
        {
            i++;
        }
    }
    VIR_IdList_RenumberIndex(Shader, list);

    /* Remove struct inputs. */
    list = VIR_Shader_GetAttributes(Shader);
    for (i = 0; i < VIR_IdList_Count(list);)
    {
        symbol = VIR_Shader_GetSymFromId(Shader, VIR_IdList_GetId(list, i));
        if (!symbol)
        {
            i++;
            continue;
        }

        if (VIR_Type_IsBaseTypeStruct(Shader, VIR_Symbol_GetType(symbol)))
        {
            VIR_IdList_DeleteByIndex(list, i);
        }
        else
        {
            i++;
        }
    }
    VIR_IdList_RenumberIndex(Shader, list);

    /* Remove struct perpatch inputs. */
    list = VIR_Shader_GetPerpatchAttributes(Shader);
    for (i = 0; i < VIR_IdList_Count(list);)
    {
        symbol = VIR_Shader_GetSymFromId(Shader, VIR_IdList_GetId(list, i));
        if (!symbol)
        {
            i++;
            continue;
        }

        if (VIR_Type_IsBaseTypeStruct(Shader, VIR_Symbol_GetType(symbol)))
        {
            VIR_IdList_DeleteByIndex(list, i);
        }
        else
        {
            i++;
        }
    }
    VIR_IdList_RenumberIndex(Shader, list);

    /* Remove struct uniforms. */
    list = VIR_Shader_GetUniforms(Shader);
    for (i = 0; i < VIR_IdList_Count(list);)
    {
        symbol = VIR_Shader_GetSymFromId(Shader, VIR_IdList_GetId(list, i));
        if (!symbol)
        {
            i++;
            continue;
        }

        if (VIR_Type_IsBaseTypeStruct(Shader, VIR_Symbol_GetType(symbol)))
        {
            VIR_IdList_DeleteByIndex(list, i);
        }
        else
        {
            i++;
        }
    }
    VIR_IdList_RenumberIndex(Shader, list);

    /* Remove array UBOs. */
    list = VIR_Shader_GetUniformBlocks(Shader);
    for (i = 0; i < VIR_IdList_Count(list);)
    {
        symbol = VIR_Shader_GetSymFromId(Shader, VIR_IdList_GetId(list, i));
        if (!symbol)
        {
            i++;
            continue;
        }

        type = VIR_Symbol_GetType(symbol);

        if (VIR_Type_isArray(type))
        {
            VIR_IdList_DeleteByIndex(list, i);
        }
        else
        {
            i++;
        }
    }
    VIR_IdList_RenumberIndex(Shader, list);

    /* Remove array SBO. */
    list = VIR_Shader_GetSSBlocks(Shader);
    for (i = 0; i < VIR_IdList_Count(list);)
    {
        symbol = VIR_Shader_GetSymFromId(Shader, VIR_IdList_GetId(list, i));
        if (!symbol)
        {
            i++;
            continue;
        }

        type = VIR_Symbol_GetType(symbol);

        if (VIR_Type_isArray(type))
        {
            VIR_IdList_DeleteByIndex(list, i);
        }
        else
        {
            i++;
        }
    }
    VIR_IdList_RenumberIndex(Shader, list);

    /* TODO: may need to support IBO, SSBO and so on. */
    return errCode;
}

/*--------------------------Matrix-related functions--------------------------*/
static VSC_ErrCode
_ConvMatrixOperandToVectorOperand(
    IN  VIR_Shader              *Shader,
    IN  VIR_Function            *Func,
    IN  VIR_Operand             *MatrixOperand,
    IN  gctBOOL                  IsDest,
    IN  gctINT                   MatrixIndex,
    OUT VIR_Operand            **VectorOperand
    )
{
    VSC_ErrCode                  errCode = VSC_ERR_NONE;
    VIR_SymId                    symId;
    VIR_Symbol                  *symbol;
    VIR_OperandKind              opKind = VIR_Operand_GetOpKind(MatrixOperand);
    VIR_TypeId                   matrixTypeId = VIR_Operand_GetTypeId(MatrixOperand);
    VIR_TypeId                   rowTypeId = VIR_GetTypeRowType(matrixTypeId);
    VIR_Enable                   enable = VIR_TypeId_Conv2Enable(rowTypeId);
    VIR_Swizzle                  swizzle = VIR_Enable_2_Swizzle_WShift(enable);
    VIR_Operand                 *vectorOperand = *VectorOperand;

    /* If this matrix is:
    ** 1) uniform, use "uniform sym" + "_matrixConstIndex" to describe this operand.
    ** 2) other types, use "base temp register" + "offset" to describe this operand.
    */
    if (opKind == VIR_OPND_SYMBOL)
    {
        symbol = VIR_Operand_GetSymbol(MatrixOperand);

        /* TODO: handle const index and rel addr. */
        if (VIR_Operand_GetIsConstIndexing(MatrixOperand))
        {
            gcmASSERT(gcvFALSE);
        }
        else if (VIR_Operand_GetRelAddrMode(MatrixOperand) != VIR_INDEXED_NONE)
        {
            gcmASSERT(gcvFALSE);
        }
        else
        {
            if (VIR_Symbol_GetKind(symbol) == VIR_SYM_UNIFORM)
            {
                VIR_Operand_Copy(vectorOperand, MatrixOperand);
                VIR_Operand_SetMatrixConstIndex(vectorOperand, MatrixIndex);
                VIR_Operand_SetTypeId(vectorOperand, rowTypeId);
            }
            else
            {
                errCode = VIR_Shader_GetVirRegSymByVirRegId(Shader,
                                                            VIR_Symbol_GetVregIndex(symbol) + MatrixIndex,
                                                            &symId);
                CHECK_ERROR(errCode, "VIR_Shader_GetVirRegSymByVirRegId failed.");
                gcmASSERT(symId != VIR_INVALID_ID);

                VIR_Operand_SetTempRegister(vectorOperand,
                                            Func,
                                            symId,
                                            rowTypeId);
            }
        }
    }
    else
    {
        gcmASSERT(opKind == VIR_OPND_VIRREG);
        symbol = VIR_Operand_GetSymbol(MatrixOperand);

        /* TODO: handle const index and rel addr. */
        if (VIR_Operand_GetIsConstIndexing(MatrixOperand))
        {
            gcmASSERT(gcvFALSE);
        }
        else if (VIR_Operand_GetRelAddrMode(MatrixOperand) != VIR_INDEXED_NONE)
        {
            gcmASSERT(gcvFALSE);
        }
        else
        {
            errCode = VIR_Shader_GetVirRegSymByVirRegId(Shader,
                                                        VIR_Symbol_GetVregIndex(symbol) + MatrixIndex,
                                                        &symId);
            CHECK_ERROR(errCode, "VIR_Shader_GetVirRegSymByVirRegId failed.");
            gcmASSERT(symId != VIR_INVALID_ID);

            VIR_Operand_SetTempRegister(vectorOperand,
                                        Func,
                                        symId,
                                        rowTypeId);
        }
    }

    /* Update the enable/swizzle. */
    if (IsDest)
    {
        VIR_Operand_SetEnable(vectorOperand, enable);
    }
    else
    {
        VIR_Operand_SetSwizzle(vectorOperand, swizzle);
    }

    if (VectorOperand)
    {
        *VectorOperand = vectorOperand;
    }

    return errCode;
}

static VSC_ErrCode
_SplitMatrixAdd(
    IN  VIR_Shader              *Shader,
    IN  VIR_Function            *Func,
    IN  VIR_Instruction         *Inst
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;

    return errCode;
}

static VSC_ErrCode
_SplitMatrixMulScalar(
    IN  VIR_Shader              *Shader,
    IN  VIR_Function            *Func,
    IN  VIR_Instruction         *Inst
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_TypeId                   src0TypeId = VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 0));
    VIR_TypeId                   newTypeId = VIR_GetTypeRowType(src0TypeId);
    VIR_Instruction             *newInst = gcvNULL;
    VIR_Operand                 *newOperand;
    gctINT                       rowCount = VIR_GetTypeRows(src0TypeId);
    gctINT                       i;

    for (i = 0; i < rowCount; i++)
    {
        errCode = VIR_Function_AddInstructionBefore(Func,
                                                    VIR_OP_MUL,
                                                    newTypeId,
                                                    Inst,
                                                    gcvTRUE,
                                                    &newInst);
        CHECK_ERROR(errCode, "VIR_Function_AddInstructionBefore failed.");

        /* Set DEST. */
        newOperand = VIR_Inst_GetDest(newInst);
        errCode = _ConvMatrixOperandToVectorOperand(Shader,
                                                    Func,
                                                    VIR_Inst_GetDest(Inst),
                                                    gcvTRUE,
                                                    i,
                                                    &newOperand);
        CHECK_ERROR(errCode, "_ConvMatrixOperandToVectorOperand failed.");
        VIR_Inst_SetDest(newInst, newOperand);

        /* Set SOURCE0. */
        newOperand = VIR_Inst_GetSource(newInst, 0);
        errCode = _ConvMatrixOperandToVectorOperand(Shader,
                                                    Func,
                                                    VIR_Inst_GetSource(Inst, 0),
                                                    gcvFALSE,
                                                    i,
                                                    &newOperand);
        CHECK_ERROR(errCode, "_ConvMatrixOperandToVectorOperand failed.");
        VIR_Inst_SetSource(newInst, 0, newOperand);

        /* Set SOURCE1. */
        newOperand = VIR_Inst_GetSource(newInst, 1);
        VIR_Operand_Copy(newOperand, VIR_Inst_GetSource(Inst, 1));
    }

    /* Change current MUL to NOP. */
    VIR_Inst_SetOpcode(Inst, VIR_OP_NOP);
    VIR_Inst_SetConditionOp(Inst, VIR_COP_ALWAYS);
    VIR_Inst_SetSrcNum(Inst, 0);
    VIR_Inst_SetDest(Inst, gcvNULL);

    return errCode;
}

static VSC_ErrCode
_SplitMatrixMulVector(
    IN  VIR_Shader              *Shader,
    IN  VIR_Function            *Func,
    IN  VIR_Instruction         *Inst,
    IN  VIR_Operand             *Source0,
    IN  VIR_Operand             *Source1,
    IN  gctBOOL                  ChangeMulToMov,
    OUT VIR_SymId               *ReturnSymId
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_TypeId                   src0TypeId, vectorTypeId;
    VIR_SymId                    currentSymId = VIR_INVALID_ID, prevSymId = VIR_INVALID_ID;
    VIR_Instruction             *newInst = gcvNULL;
    VIR_Operand                 *newOperand = gcvNULL;
    VIR_OpCode                   opcode;
    VIR_Swizzle                  src0Swizzle;
    VIR_VirRegId                 virRegId;
    gctINT                       i, rowCount;

    src0TypeId = VIR_Operand_GetTypeId(Source0);
    vectorTypeId = VIR_GetTypeRowType(src0TypeId);

    rowCount = VIR_GetTypeRows(src0TypeId);

    for (i = 0; i < rowCount; i++)
    {
        /* Generate MUL for first component, generate MAD for the rest components. */
        if (i == 0)
        {
            opcode = VIR_OP_MUL;
        }
        else
        {
            opcode = VIR_OP_MAD;
        }

        errCode = VIR_Function_AddInstructionBefore(Func,
                                                    opcode,
                                                    vectorTypeId,
                                                    Inst,
                                                    gcvTRUE,
                                                    &newInst);
        CHECK_ERROR(errCode, "VIR_Function_AddInstructionBefore failed.");

        /* Create a symbol to save DEST. */
        virRegId = VIR_Shader_NewVirRegId(Shader, 1);
        errCode = VIR_Shader_AddSymbol(Shader,
                                       VIR_SYM_VIRREG,
                                       virRegId,
                                       VIR_Shader_GetTypeFromId(Shader, vectorTypeId),
                                       VIR_STORAGE_UNKNOWN,
                                       &currentSymId);
        CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed.");

        /* Set DEST. */
        newOperand = VIR_Inst_GetDest(newInst);
        VIR_Operand_SetTempRegister(newOperand,
                                    Func,
                                    currentSymId,
                                    vectorTypeId);
        VIR_Operand_SetEnable(newOperand, VIR_TypeId_Conv2Enable(vectorTypeId));
        VIR_Inst_SetDest(newInst, newOperand);

        /* Set SOURCE0. */
        newOperand = VIR_Inst_GetSource(newInst, 0);
        errCode = _ConvMatrixOperandToVectorOperand(Shader,
                                                    Func,
                                                    Source0,
                                                    gcvFALSE,
                                                    i,
                                                    &newOperand);
        CHECK_ERROR(errCode, "_ConvMatrixOperandToVectorOperand failed.");
        src0Swizzle = VIR_Operand_GetSwizzle(newOperand);
        VIR_Inst_SetSource(newInst, 0, newOperand);

        /* Set SOURCE1. */
        newOperand = VIR_Inst_GetSource(newInst, 1);
        VIR_Operand_Copy(newOperand, Source1);
        VIR_Operand_SetSwizzle(newOperand,
            VIR_Swizzle_Trim(VIR_Operand_GetSwizzle(Source1), (VIR_Enable)(1 << i)));

        /* Set SOURCE2 for MAD. */
        if (i != 0)
        {
            newOperand = VIR_Inst_GetSource(newInst, 2);
            VIR_Operand_SetTempRegister(newOperand,
                                        Func,
                                        prevSymId,
                                        vectorTypeId);
            VIR_Operand_SetSwizzle(newOperand, src0Swizzle);
            VIR_Inst_SetSource(newInst, 2, newOperand);
        }

        /* Save the DEST of current MUL/MAD for the SOURCE2 of next MAD. */
        prevSymId = currentSymId;
    }

    /* Change current MUL to MOV if needed. */
    if (ChangeMulToMov)
    {
        VIR_Inst_SetOpcode(Inst, VIR_OP_MOV);
        VIR_Inst_SetConditionOp(Inst, VIR_COP_ALWAYS);
        VIR_Inst_SetSrcNum(Inst, 1);
        newOperand = VIR_Inst_GetSource(Inst, 0);
        VIR_Operand_SetTempRegister(newOperand,
                                    Func,
                                    prevSymId,
                                    vectorTypeId);
        VIR_Inst_SetSource(Inst, 0, newOperand);
    }

    if (ReturnSymId)
    {
        *ReturnSymId = prevSymId;
    }

    return errCode;
}

static VSC_ErrCode
_SplitMatrixMulMatrix(
    IN  VIR_Shader              *Shader,
    IN  VIR_Function            *Func,
    IN  VIR_Instruction         *Inst
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_Operand                 *src0 = VIR_Inst_GetSource(Inst, 0);
    VIR_Operand                 *src1 = VIR_Inst_GetSource(Inst, 1);
    VIR_Operand                 *vectorOperand;
    VIR_Operand                 *newOperand;
    VIR_TypeId                   src0TypeId = VIR_Operand_GetTypeId(src0);
    VIR_TypeId                   src1TypeId = VIR_Operand_GetTypeId(src1);
    VIR_TypeId                   vectorTypeId = VIR_GetTypeRowType(src1TypeId);
    VIR_SymId                    returnValueSymId[4] = {VIR_INVALID_ID, VIR_INVALID_ID, VIR_INVALID_ID, VIR_INVALID_ID};
    VIR_Instruction             *newInst;
    VIR_Swizzle                  src0Swizzle;
    gctINT                       src0ComponetCount = VIR_GetTypeComponents(src0TypeId);
    gctINT                       src1RowCount = VIR_GetTypeRows(src1TypeId);
    gctINT                       i;

    gcmASSERT(src0ComponetCount <= 4 && src0ComponetCount > 1);
    gcmASSERT(src1RowCount <= 4 && src1RowCount > 1);
    src0Swizzle = VIR_Swizzle_GenSwizzleByComponentCount(src0ComponetCount);

    /* Split MatrixTimesMatrix to several MatrixTimesVector. */
    for (i = 0; i < src1RowCount; i++)
    {
        /* Convert source1 from matrix to matrix[i]. */
        VIR_Function_DupOperand(Func, src1, &vectorOperand);
        errCode = _ConvMatrixOperandToVectorOperand(Shader,
                                                    Func,
                                                    src1,
                                                    gcvFALSE,
                                                    i,
                                                    &vectorOperand);
        CHECK_ERROR(errCode, "_ConvMatrixOperandToVectorOperand failed.");

        errCode = _SplitMatrixMulVector(Shader,
                                        Func,
                                        Inst,
                                        src0,
                                        vectorOperand,
                                        gcvFALSE,
                                        &returnValueSymId[i]);
        CHECK_ERROR(errCode, "_SplitMatrixMulVector failed.");
        gcmASSERT(returnValueSymId[i] != VIR_INVALID_ID);

        VIR_Function_FreeOperand(Func, vectorOperand);

        /* Save the return value. */
        errCode = VIR_Function_AddInstructionBefore(Func,
                                                    VIR_OP_MOV,
                                                    vectorTypeId,
                                                    Inst,
                                                    gcvTRUE,
                                                    &newInst);
        CHECK_ERROR(errCode, "VIR_Function_AddInstructionBefore failed.");

        /* Set DEST. */
        newOperand = VIR_Inst_GetDest(newInst);
        errCode = _ConvMatrixOperandToVectorOperand(Shader,
                                                    Func,
                                                    VIR_Inst_GetDest(Inst),
                                                    gcvTRUE,
                                                    i,
                                                    &newOperand);
        CHECK_ERROR(errCode, "_ConvMatrixOperandToVectorOperand failed.");
        VIR_Inst_SetDest(newInst, newOperand);

        /* Set SOURCE0. */
        newOperand = VIR_Inst_GetSource(newInst, 0);
        VIR_Operand_SetTempRegister(newOperand,
                                    Func,
                                    returnValueSymId[i],
                                    vectorTypeId);
        VIR_Operand_SetSwizzle(newOperand, src0Swizzle);
        VIR_Inst_SetSource(newInst, 0, newOperand);
    }

    /* Change current MOV to NOP. */
    VIR_Inst_SetOpcode(Inst, VIR_OP_NOP);
    VIR_Inst_SetConditionOp(Inst, VIR_COP_ALWAYS);
    VIR_Inst_SetSrcNum(Inst, 0);
    VIR_Inst_SetDest(Inst, gcvNULL);

    return errCode;
}

static VSC_ErrCode
_SplitVectorMulVector(
    IN  VIR_Shader              *Shader,
    IN  VIR_Function            *Func,
    IN  VIR_Instruction         *Inst
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_Operand                 *dest = VIR_Inst_GetDest(Inst);
    VIR_Operand                 *src0 = VIR_Inst_GetSource(Inst, 0);
    VIR_Operand                 *src1 = VIR_Inst_GetSource(Inst, 1);
    VIR_TypeId                   rowTypeId = VIR_Operand_GetTypeId(src0);
    VIR_TypeId                   columnTypeId = VIR_Operand_GetTypeId(src1);
    VIR_TypeId                   scalarTypeId = VIR_GetTypeComponentType(rowTypeId);
    VIR_OpCode                   opcode = VIR_OP_MUL;
    VIR_Instruction             *newInst = gcvNULL;
    VIR_Operand                 *newOperand = gcvNULL;
    gctUINT                      columnCount;
    gctUINT                      i;

    columnCount = VIR_GetTypeComponents(columnTypeId);

    for (i = 0; i < columnCount; i++)
    {
        errCode = VIR_Function_AddInstructionBefore(Func,
                                                    opcode,
                                                    rowTypeId,
                                                    Inst,
                                                    gcvTRUE,
                                                    &newInst);
        CHECK_ERROR(errCode, "VIR_Function_AddInstructionBefore failed.");

        /* Set DEST. */
        newOperand = VIR_Inst_GetDest(newInst);
        errCode = _ConvMatrixOperandToVectorOperand(Shader,
                                                    Func,
                                                    dest,
                                                    gcvTRUE,
                                                    i,
                                                    &newOperand);
        CHECK_ERROR(errCode, "_ConvMatrixOperandToVectorOperand failed.");
        VIR_Inst_SetDest(newInst, newOperand);

        /* Set SOURCE0. */
        newOperand = VIR_Inst_GetSource(newInst, 0);
        VIR_Operand_Copy(newOperand, src0);

        /* Set SOURCE1. */
        newOperand = VIR_Inst_GetSource(newInst, 1);
        VIR_Operand_Copy(newOperand, src1);
        VIR_Operand_SetTypeId(newOperand, scalarTypeId);
        VIR_Operand_SetSwizzle(newOperand,
            VIR_Swizzle_Trim(VIR_Operand_GetSwizzle(src1), (VIR_Enable)(1 << i)));
    }

    /* Change current MOV to NOP. */
    VIR_Inst_SetOpcode(Inst, VIR_OP_NOP);
    VIR_Inst_SetConditionOp(Inst, VIR_COP_ALWAYS);
    VIR_Inst_SetSrcNum(Inst, 0);
    VIR_Inst_SetDest(Inst, gcvNULL);

    return errCode;
}

static VSC_ErrCode
_SplitVectorMulMatrix(
    IN  VIR_Shader              *Shader,
    IN  VIR_Function            *Func,
    IN  VIR_Instruction         *Inst
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_Operand                 *src0 = VIR_Inst_GetSource(Inst, 0);
    VIR_Operand                 *src1 = VIR_Inst_GetSource(Inst, 1);
    VIR_Operand                 *newOperand;
    VIR_TypeId                   src1TypeId = VIR_Operand_GetTypeId(src1);
    VIR_TypeId                   componentTypeId = VIR_GetTypeComponentType(src1TypeId);
    VIR_TypeId                   vectorTypeId = VIR_Operand_GetTypeId(VIR_Inst_GetDest(Inst));
    VIR_OpCode                   opcode;
    VIR_VirRegId                 virRegId;
    VIR_SymId                    symId;
    VIR_Swizzle                  src0Swizzle;
    VIR_Instruction             *newInst = gcvNULL;
    gctINT                       componentCount = VIR_GetTypeComponents(src1TypeId);
    gctINT                       rowCount = VIR_GetTypeRows(src1TypeId);
    gctINT                       i;

    src0Swizzle = VIR_Swizzle_GenSwizzleByComponentCount(rowCount);

    /* Use DP(componentcount)*/
    if (componentCount == 2)
    {
        opcode = VIR_OP_DP2;
    }
    else if (componentCount == 3)
    {
        opcode = VIR_OP_DP3;
    }
    else
    {
        gcmASSERT(componentCount == 4);
        opcode = VIR_OP_DP4;
    }

    /* Allocate a new temp register to save the result. */
    virRegId = VIR_Shader_NewVirRegId(Shader, 1);
    errCode = VIR_Shader_AddSymbol(Shader,
                                   VIR_SYM_VIRREG,
                                   virRegId,
                                   VIR_Shader_GetTypeFromId(Shader, vectorTypeId),
                                   VIR_STORAGE_UNKNOWN,
                                   &symId);
    CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed.");
    gcmASSERT(symId != VIR_INVALID_ID);

    /* General several DPs. */
    for (i = 0; i < rowCount; i++)
    {
        errCode = VIR_Function_AddInstructionBefore(Func,
                                                    opcode,
                                                    componentTypeId,
                                                    Inst,
                                                    gcvTRUE,
                                                    &newInst);
        CHECK_ERROR(errCode, "VIR_Function_AddInstructionBefore failed.");

        /* Set DEST. */
        newOperand = VIR_Inst_GetDest(newInst);
        VIR_Operand_SetTempRegister(newOperand,
                                    Func,
                                    symId,
                                    componentTypeId);
        VIR_Operand_SetEnable(newOperand, VIR_ENABLE_X << i);
        VIR_Inst_SetDest(newInst, newOperand);

        /* Set Source0. */
        newOperand = VIR_Inst_GetSource(newInst, 0);
        VIR_Operand_Copy(newOperand, src0);

        /* Set SOURCE1. */
        newOperand = VIR_Inst_GetSource(newInst, 1);
        errCode = _ConvMatrixOperandToVectorOperand(Shader,
                                                    Func,
                                                    src1,
                                                    gcvFALSE,
                                                    i,
                                                    &newOperand);
        CHECK_ERROR(errCode, "_ConvMatrixOperandToVectorOperand failed.");
        VIR_Inst_SetSource(newInst, 1, newOperand);
    }

    /* Change current MUL to MOV. */
    VIR_Inst_SetOpcode(Inst, VIR_OP_MOV);
    VIR_Inst_SetConditionOp(Inst, VIR_COP_ALWAYS);
    VIR_Inst_SetSrcNum(Inst, 1);
    newOperand = VIR_Inst_GetSource(Inst, 0);
    VIR_Operand_SetTempRegister(newOperand,
                                Func,
                                symId,
                                vectorTypeId);
    VIR_Operand_SetSwizzle(newOperand, src0Swizzle);
    VIR_Inst_SetSource(Inst, 0, newOperand);

    return errCode;
}

static VSC_ErrCode
_SplitMatrixMul(
    IN  VIR_Shader              *Shader,
    IN  VIR_Function            *Func,
    IN  VIR_Instruction         *Inst
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_Operand                 *src0 = VIR_Inst_GetSource(Inst, 0);
    VIR_Operand                 *src1 = VIR_Inst_GetSource(Inst, 1);
    VIR_TypeId                   src0TypeId, src1TypeId;
    gctBOOL                      isSrc0Matrix = gcvFALSE, isSrc1Matrix = gcvFALSE;
    gctBOOL                      isSrc0Vector = gcvFALSE, isSrc1Vector = gcvFALSE;

    src0TypeId = VIR_Operand_GetTypeId(src0);
    src1TypeId = VIR_Operand_GetTypeId(src1);

    gcmASSERT(VIR_TypeId_isPrimitive(VIR_Operand_GetTypeId(VIR_Inst_GetDest(Inst))) &&
              VIR_TypeId_isPrimitive(src0TypeId) &&
              VIR_TypeId_isPrimitive(src1TypeId));

    isSrc0Matrix = (VIR_GetTypeTypeKind(src0TypeId) == VIR_TY_MATRIX);
    isSrc1Matrix = (VIR_GetTypeTypeKind(src1TypeId) == VIR_TY_MATRIX);

    isSrc0Vector = (VIR_GetTypeTypeKind(src0TypeId) == VIR_TY_VECTOR);
    isSrc1Vector = (VIR_GetTypeTypeKind(src1TypeId) == VIR_TY_VECTOR);

    /* Matrix times Matrix */
    if (isSrc0Matrix && isSrc1Matrix)
    {
        errCode = _SplitMatrixMulMatrix(Shader, Func, Inst);
        CHECK_ERROR(errCode, "_SplitMatrixMulMatrix failed.");
    }
    /* Matrix times Vector */
    else if (isSrc0Matrix && isSrc1Vector)
    {
        errCode = _SplitMatrixMulVector(Shader,
                                        Func,
                                        Inst,
                                        VIR_Inst_GetSource(Inst, 0),
                                        VIR_Inst_GetSource(Inst, 1),
                                        gcvTRUE,
                                        gcvNULL);
        CHECK_ERROR(errCode, "_SplitMatrixMulVector failed.");
    }
    /* Matrix times Scalar */
    else if (isSrc0Matrix && !isSrc1Matrix && !isSrc1Vector)
    {
        errCode = _SplitMatrixMulScalar(Shader, Func, Inst);
        CHECK_ERROR(errCode, "_SplitMatrixMulScalar failed.");
    }
    /* Vector times vector */
    else if (isSrc0Vector && isSrc1Vector)
    {
        errCode = _SplitVectorMulVector(Shader, Func, Inst);
        CHECK_ERROR(errCode, "_SplitMatrixMulVector failed.");
    }
    /* Vector times Matrix */
    else if (isSrc0Vector && isSrc1Matrix)
    {
        errCode = _SplitVectorMulMatrix(Shader, Func, Inst);
        CHECK_ERROR(errCode, "_SplitVectorMulMatrix failed.");
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }

    return errCode;
}

static VSC_ErrCode
_SplitMatrixLoadStore(
    IN  VIR_Shader              *Shader,
    IN  VIR_Function            *Func,
    IN  VIR_Instruction         *Inst
    )
{
    /* If it is a LOAD/STORE, then it must load a matrix from a SBO/UBO. */
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_Operand                 *origDest = VIR_Inst_GetDest(Inst);
    VIR_Operand                 *baseAddr = VIR_Inst_GetSource(Inst, 0);
    VIR_Operand                 *origOffset = VIR_Inst_GetSource(Inst, 1);
    VIR_TypeId                   destTypeId = VIR_Operand_GetTypeId(origDest);
    VIR_Type                    *destType = VIR_Shader_GetTypeFromId(Shader, destTypeId);
    VIR_TypeId                   componentTypeId = VIR_GetTypeComponentType(destTypeId);
    VIR_Instruction             *newInst = gcvNULL;
    VIR_Operand                 *newOperand = gcvNULL;
    VIR_ScalarConstVal           newOffset;
    VIR_VirRegId                 regId;
    VIR_SymId                    regSymId, regSymId2;
    VIR_OpCode                   opCode = VIR_Inst_GetOpcode(Inst);
    gctINT                       rowCount = VIR_GetTypeRows(destTypeId);
    gctINT                       componentCount = VIR_GetTypeComponents(destTypeId);
    gctINT                       matrixStride = VIR_Operand_GetMatrixStride(baseAddr);
    gctINT                       rowMatrixStride;
    VIR_LayoutQual               layoutQual = VIR_Operand_GetLayoutQual(baseAddr);
    gctBOOL                      isRowMajor = VIR_LAYQUAL_ROW_MAJOR & layoutQual;
    gctINT                       i, j;

    gcmASSERT(VIR_Type_isMatrix(VIR_Shader_GetTypeFromId(Shader, destTypeId)));
    gcmASSERT(matrixStride > 0);

    /* Allocate a new reg to save the offset. */
    regId = VIR_Shader_NewVirRegId(Shader, 1);
    errCode = VIR_Shader_AddSymbol(Shader,
                                   VIR_SYM_VIRREG,
                                   regId,
                                   VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT32),
                                   VIR_STORAGE_UNKNOWN,
                                   &regSymId);
    CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed.");

    /* Do we need to calculate the matrix stride for row_major here?
    ** If the decoration for matrix stride is already for row_major, then we don't need this.
    */
    if (isRowMajor)
    {
        VIR_Type_CalcByteOffset(Shader,
            destType, VIR_Type_isArray(destType), layoutQual, 0, gcvNULL, gcvNULL, &rowMatrixStride, gcvNULL);
    }

    for (i = 0; i < rowCount; i++)
    {
        /* Insert a ADD to update the offset. */
        errCode = VIR_Function_AddInstructionBefore(Func,
                                                    VIR_OP_ADD,
                                                    VIR_TYPE_UINT32,
                                                    Inst,
                                                    gcvTRUE,
                                                    &newInst);
        CHECK_ERROR(errCode, "VIR_Function_AddInstructionBefore failed.");

        /* Set DEST. */
        newOperand = VIR_Inst_GetDest(newInst);
        VIR_Operand_SetTempRegister(newOperand, Func, regSymId, VIR_TYPE_UINT32);
        VIR_Operand_SetEnable(newOperand, VIR_ENABLE_X);
        VIR_Operand_SetPrecision(newOperand, VIR_PRECISION_HIGH);

        /* Set SOURCE0. */
        newOperand = VIR_Inst_GetSource(newInst, 0);
        VIR_Operand_Copy(newOperand, origOffset);

        /* Set SOURCE1. */
        newOperand = VIR_Inst_GetSource(newInst, 1);
        if (isRowMajor)
        {
            newOffset.uValue = i * 4;
        }
        else
        {
            /* The matrix stride is saved in base address operand. */
            newOffset.uValue = i * matrixStride;
        }

        VIR_Operand_SetImmediate(newOperand, VIR_TYPE_UINT32, newOffset);

        if (isRowMajor)
        {
            for (j = 0; j < componentCount; j++)
            {
                /* Add the matrix stride. */
                regId = VIR_Shader_NewVirRegId(Shader, 1);
                errCode = VIR_Shader_AddSymbol(Shader,
                                               VIR_SYM_VIRREG,
                                               regId,
                                               VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT32),
                                               VIR_STORAGE_UNKNOWN,
                                               &regSymId2);
                CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed.");

                /* Insert a ADD to update the offset. */
                errCode = VIR_Function_AddInstructionBefore(Func,
                                                            VIR_OP_ADD,
                                                            VIR_TYPE_UINT32,
                                                            Inst,
                                                            gcvTRUE,
                                                            &newInst);
                CHECK_ERROR(errCode, "VIR_Function_AddInstructionBefore failed.");

                /* Set DEST. */
                newOperand = VIR_Inst_GetDest(newInst);
                VIR_Operand_SetTempRegister(newOperand, Func, regSymId2, VIR_TYPE_UINT32);
                VIR_Operand_SetEnable(newOperand, VIR_ENABLE_X);
                VIR_Operand_SetPrecision(newOperand, VIR_PRECISION_HIGH);

                /* Set SOURCE0. */
                newOperand = VIR_Inst_GetSource(newInst, 0);
                VIR_Operand_SetTempRegister(newOperand, Func, regSymId, VIR_TYPE_UINT32);
                VIR_Operand_SetSwizzle(newOperand, VIR_SWIZZLE_XXXX);

                /* Set SOURCE1. */
                newOperand = VIR_Inst_GetSource(newInst, 1);
                /* The matrix stride is saved in base address operand. */
                newOffset.uValue = j * matrixStride;
                VIR_Operand_SetImmediate(newOperand, VIR_TYPE_UINT32, newOffset);

                /* Load with the updated offset. */
                errCode = VIR_Function_AddInstructionBefore(Func,
                                                            opCode,
                                                            VIR_TYPE_UINT32,
                                                            Inst,
                                                            gcvTRUE,
                                                            &newInst);
                CHECK_ERROR(errCode, "VIR_Function_AddInstructionBefore failed.");

                /* Set DEST. */
                newOperand = VIR_Inst_GetDest(newInst);
                errCode = _ConvMatrixOperandToVectorOperand(Shader,
                                                            Func,
                                                            origDest,
                                                            gcvTRUE,
                                                            i,
                                                            &newOperand);
                CHECK_ERROR(errCode, "_ConvMatrixOperandToVectorOperand failed.");
                VIR_Operand_SetEnable(newOperand, VIR_ENABLE_X << j);
                VIR_Operand_SetTypeId(newOperand, componentTypeId);
                VIR_Inst_SetDest(newInst, newOperand);

                /* Set SOURCE0. */
                newOperand = VIR_Inst_GetSource(newInst, 0);
                VIR_Operand_Copy(newOperand, baseAddr);

                /* Set SOURCE1. */
                newOperand = VIR_Inst_GetSource(newInst, 1);
                VIR_Operand_SetTempRegister(newOperand, Func, regSymId2, VIR_TYPE_UINT32);
                VIR_Operand_SetSwizzle(newOperand, VIR_SWIZZLE_XXXX);

                if (opCode == VIR_OP_STORE)
                {
                    newOperand = VIR_Inst_GetSource(newInst, 2);
                    errCode = _ConvMatrixOperandToVectorOperand(Shader,
                                                                Func,
                                                                VIR_Inst_GetSource(Inst, 2),
                                                                gcvFALSE,
                                                                i,
                                                                &newOperand);
                    CHECK_ERROR(errCode, "_ConvMatrixOperandToVectorOperand failed.");
                    VIR_Operand_SetSwizzle(newOperand, VIR_Enable_2_Swizzle_WShift(VIR_ENABLE_X << j));
                    VIR_Operand_SetTypeId(newOperand, componentTypeId);
                    VIR_Inst_SetSource(newInst, 2, newOperand);
                }
            }
        }
        else
        {
            /* Load with the updated offset. */
            errCode = VIR_Function_AddInstructionBefore(Func,
                                                        opCode,
                                                        VIR_TYPE_UINT32,
                                                        Inst,
                                                        gcvTRUE,
                                                        &newInst);
            CHECK_ERROR(errCode, "VIR_Function_AddInstructionBefore failed.");

            /* Set DEST. */
            newOperand = VIR_Inst_GetDest(newInst);
            errCode = _ConvMatrixOperandToVectorOperand(Shader,
                                                        Func,
                                                        origDest,
                                                        gcvTRUE,
                                                        i,
                                                        &newOperand);
            CHECK_ERROR(errCode, "_ConvMatrixOperandToVectorOperand failed.");
            VIR_Inst_SetDest(newInst, newOperand);

            /* Set SOURCE0. */
            newOperand = VIR_Inst_GetSource(newInst, 0);
            VIR_Operand_Copy(newOperand, baseAddr);

            /* Set SOURCE1. */
            newOperand = VIR_Inst_GetSource(newInst, 1);
            VIR_Operand_SetTempRegister(newOperand, Func, regSymId, VIR_TYPE_UINT32);
            VIR_Operand_SetSwizzle(newOperand, VIR_SWIZZLE_XXXX);
            VIR_Inst_SetSource(newInst, 1, newOperand);

            if (opCode == VIR_OP_STORE)
            {
                newOperand = VIR_Inst_GetSource(newInst, 2);
                errCode = _ConvMatrixOperandToVectorOperand(Shader,
                                                            Func,
                                                            VIR_Inst_GetSource(Inst, 2),
                                                            gcvFALSE,
                                                            i,
                                                            &newOperand);
                CHECK_ERROR(errCode, "_ConvMatrixOperandToVectorOperand failed.");
                VIR_Inst_SetSource(newInst, 2, newOperand);
            }
        }
    }

    /* Change current MOV to NOP. */
    VIR_Inst_SetOpcode(Inst, VIR_OP_NOP);
    VIR_Inst_SetConditionOp(Inst, VIR_COP_ALWAYS);
    VIR_Inst_SetSrcNum(Inst, 0);
    VIR_Inst_SetDest(Inst, gcvNULL);

    return errCode;
}

static VSC_ErrCode
_SplitIntrinsicMatrixSrcOpnd(
    IN  VIR_Shader              *Shader,
    IN  VIR_Function            *Func,
    IN  VIR_Instruction         *Inst
    )
{
    VSC_ErrCode                 errCode  = VSC_ERR_NONE;
    VIR_Operand                 *paramOperand = VIR_Inst_GetSource(Inst, 1);
    VIR_ParmPassing             *opndParm = VIR_Operand_GetParameters(paramOperand);
    gctUINT                     argNum = opndParm->argNum;
    gctUINT                     i, totalRow = 0, newOpndIdx;
    VIR_Operand                 *srcOpnd, *newOpnd;

    for (i = 0; i < argNum; i++)
    {
        VIR_TypeId typeId;
        srcOpnd = opndParm->args[i];
        typeId = VIR_Operand_GetTypeId(srcOpnd);

        if (VIR_TypeId_isPrimitive(typeId) && (VIR_GetTypeRows(typeId) > 1))
        {
            totalRow += VIR_GetTypeRows(typeId);
        }
        else
        {
            totalRow ++;
        }
    }

    if (totalRow > argNum)
    {
        VIR_ParmPassing *newParm = gcvNULL;

        VIR_Function_NewParameters(Func, totalRow, &newParm);
        newOpndIdx = 0;
        for (i = 0; i < argNum; i++)
        {
            VIR_TypeId typeId;
            srcOpnd = opndParm->args[i];
            typeId = VIR_Operand_GetTypeId(srcOpnd);

            if (VIR_TypeId_isPrimitive(typeId) && (VIR_GetTypeRows(typeId) > 1))
            {
                gctINT  rowCount = VIR_GetTypeRows(typeId);
                gctINT  j;

                for (j = 0; j < rowCount; j++)
                {
                   VIR_Function_NewOperand(Func, &newOpnd);
                   errCode = _ConvMatrixOperandToVectorOperand(Shader,
                                                               Func,
                                                               srcOpnd,
                                                               gcvFALSE,
                                                               j,
                                                               &newOpnd);
                   newParm->args[newOpndIdx++] = newOpnd;
                }
            }
            else
            {
                newParm->args[newOpndIdx++] = srcOpnd;
            }
        }
        gcmASSERT(newOpndIdx == totalRow);
        VIR_Operand_SetParameters(paramOperand, newParm);
    }

    return errCode;
}
static VSC_ErrCode
_SplitMatrixRelatedInst(
    IN  VIR_Shader              *Shader,
    IN  VIR_Function            *Func,
    IN  VIR_Instruction         *Inst
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;

    switch (VIR_Inst_GetOpcode(Inst))
    {
    case VIR_OP_ADD:
        /* SPIRV can only support scalar or vector ADD now. */
        gcmASSERT(gcvFALSE);
        errCode = _SplitMatrixAdd(Shader,
                                  Func,
                                  Inst);
        CHECK_ERROR(errCode, "_SplitMatrixAdd failed.");
        break;

    case VIR_OP_MUL:
        errCode = _SplitMatrixMul(Shader,
                                  Func,
                                  Inst);
        CHECK_ERROR(errCode, "_SplitMatrixAdd failed.");
        break;

    case VIR_OP_LOAD:
    case VIR_OP_STORE:
        errCode = _SplitMatrixLoadStore(Shader,
                                        Func,
                                        Inst);
        CHECK_ERROR(errCode, "_SplitMatrixLoadStore failed.");
        break;

    case VIR_OP_INTRINSIC:
        errCode = _SplitIntrinsicMatrixSrcOpnd(Shader,
                                               Func,
                                               Inst);
        CHECK_ERROR(errCode, "_SplitMatrixOpnd failed.");
        break;

    default:
        break;
    }

    return errCode;
}

static VSC_ErrCode
_MoveOffsetIntoVirReg(
    IN  VIR_Shader              *Shader,
    IN  VIR_Function            *Func,
    IN  VIR_Instruction         *Inst,
    IN  VIR_Operand             *Operand,
    OUT VIR_SymbolKind          *OffsetSymKind,
    OUT VIR_SymId               *OffsetSymId
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_SymbolKind               offsetSymKind;
    VIR_SymId                    offsetSymId, regSymId;
    VIR_Symbol                  *offsetSym = gcvNULL;
    VIR_Instruction             *newInst;
    VIR_Operand                 *newOperand;
    VIR_VirRegId                 regId;

    if (VIR_Operand_GetIsConstIndexing(Operand) ||
        VIR_Operand_GetRelAddrMode(Operand) == VIR_INDEXED_NONE)
    {
        offsetSymKind = VIR_SYM_CONST;
        offsetSymId = VIR_Operand_GetConstIndexingImmed(Operand);
    }
    else
    {
        offsetSymId = VIR_Operand_GetRelIndexing(Operand);
        offsetSym = VIR_Shader_GetSymFromId(Shader, offsetSymId);
        offsetSymKind = VIR_Symbol_GetKind(offsetSym);

        if (offsetSymKind != VIR_SYM_VIRREG)
        {
            regId = VIR_Shader_NewVirRegId(Shader, 1);

            errCode = VIR_Shader_AddSymbol(Shader,
                                           VIR_SYM_VIRREG,
                                           regId,
                                           VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT32),
                                           VIR_STORAGE_UNKNOWN,
                                           &regSymId);
            CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed.");

            /* Insert a MOV. */
            errCode = VIR_Function_AddInstructionBefore(Func,
                                                        VIR_OP_MOV,
                                                        VIR_TYPE_UINT32,
                                                        Inst,
                                                        gcvTRUE,
                                                        &newInst);
            CHECK_ERROR(errCode, "VIR_Function_AddInstructionBefore failed.");

            /* Set DEST. */
            newOperand = VIR_Inst_GetDest(newInst);
            VIR_Operand_SetTempRegister(newOperand, Func, regSymId, VIR_TYPE_UINT32);
            VIR_Operand_SetEnable(newOperand, VIR_ENABLE_X);
            VIR_Inst_SetDest(newInst, newOperand);

            /* Set SOURCE0. */
            newOperand = VIR_Inst_GetSource(newInst, 0);
            VIR_Operand_SetSymbol(newOperand, Func, offsetSymId);
            VIR_Operand_SetSwizzle(newOperand, VIR_SWIZZLE_XXXX);
            VIR_Inst_SetSource(newInst, 0, newOperand);

            /* Update the offset. */
            offsetSymKind = VIR_SYM_VIRREG;
            offsetSymId = regSymId;
        }
    }

    /* Save the offset sym type and ID. */
    if (OffsetSymKind)
    {
        *OffsetSymKind = offsetSymKind;
    }
    if (OffsetSymId)
    {
        *OffsetSymId = offsetSymId;
    }

    return errCode;
}

static VSC_ErrCode
_SplitCompositeRegAssignment(
    IN  VIR_Shader              *Shader,
    IN  VIR_Function            *Func,
    IN  VIR_Instruction         *Inst
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_Operand                 *dest = VIR_Inst_GetDest(Inst);
    VIR_Symbol                  *destSymbol = VIR_Operand_GetSymbol(dest);
    VIR_Operand                 *source = VIR_Inst_GetSource(Inst, 0);
    VIR_Symbol                  *sourceSymbol = VIR_Operand_GetSymbol(source);
    VIR_Type                    *destType = VIR_Shader_GetTypeFromId(Shader,
                                                VIR_Operand_GetTypeId(dest));
    VIR_SymbolKind               destOffsetKind = VIR_SYM_UNKNOWN, sourceOffsetKind = VIR_SYM_UNKNOWN;
    VIR_SymId                    destOffset = VIR_INVALID_ID, sourceOffset = VIR_INVALID_ID;

    gcmASSERT(VIR_Inst_GetOpcode(Inst) == VIR_OP_MOV);
    gcmASSERT(destSymbol && sourceSymbol);
    gcmASSERT(destType);

    /* Make sure that DEST and SOURCE offset is always saved by a VIR reg or a constant. */
    errCode = _MoveOffsetIntoVirReg(Shader,
                                    Func,
                                    Inst,
                                    dest,
                                    &destOffsetKind,
                                    &destOffset);
    CHECK_ERROR(errCode, "_MoveOffsetIntoVirReg failed.");

    errCode = _MoveOffsetIntoVirReg(Shader,
                                    Func,
                                    Inst,
                                    source,
                                    &sourceOffsetKind,
                                    &sourceOffset);
    CHECK_ERROR(errCode, "_MoveOffsetIntoVirReg failed.");

    if (VIR_Type_isStruct(destType))
    {
        errCode = VIR_Shader_GenStructAssignment(Shader,
                                                 Func,
                                                 Inst,
                                                 destType,
                                                 VIR_Symbol_GetIndex(destSymbol),
                                                 VIR_Symbol_GetIndex(sourceSymbol),
                                                 destOffsetKind,
                                                 destOffset,
                                                 sourceOffsetKind,
                                                 sourceOffset);
        CHECK_ERROR(errCode, "VIR_Shader_GenStructAssignment failed.");
    }
    else if (VIR_Type_isMatrix(destType))
    {
        errCode = VIR_Shader_GenMatrixAssignment(Shader,
                                                 Func,
                                                 Inst,
                                                 destType,
                                                 VIR_Symbol_GetIndex(destSymbol),
                                                 VIR_Symbol_GetIndex(sourceSymbol),
                                                 destOffsetKind,
                                                 destOffset,
                                                 sourceOffsetKind,
                                                 sourceOffset);
        CHECK_ERROR(errCode, "VIR_Shader_GenMatrixAssignment failed.");
    }
    else if (VIR_Type_isArray(destType))
    {
        errCode = VIR_Shader_GenArrayAssignment(Shader,
                                                Func,
                                                Inst,
                                                destType,
                                                VIR_Symbol_GetIndex(destSymbol),
                                                VIR_Symbol_GetIndex(sourceSymbol),
                                                destOffsetKind,
                                                destOffset,
                                                sourceOffsetKind,
                                                sourceOffset);
        CHECK_ERROR(errCode, "VIR_Shader_GenArrayAssignment failed.");
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }

    /* Change current MOV to NOP. */
    VIR_Inst_SetOpcode(Inst, VIR_OP_NOP);
    VIR_Inst_SetConditionOp(Inst, VIR_COP_ALWAYS);
    VIR_Inst_SetSrcNum(Inst, 0);
    VIR_Inst_SetDest(Inst, gcvNULL);

    return errCode;
}

/*
** For LOAD, we need to update the dest and the offset.
** For STORE, we need to udpate the dest, the offset and the data.
*/
static VSC_ErrCode
_SplitArrayMemoryAssignment(
    IN  VIR_Shader              *Shader,
    IN  VIR_Function            *Func,
    IN  VIR_Instruction         *Inst,
    IN  VIR_Type                *Type,
    IN  gctUINT                  ArrayStride,
    IN  VIR_Operand             *Dest,
    IN  VIR_Operand             *BaseAddr,
    IN  VIR_Operand             *BaseOffset,
    IN  VIR_Operand             *Data
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_TypeId                   baseTypeId = VIR_Type_GetBaseTypeId(Type);
    VIR_Type                    *baseType = VIR_Shader_GetTypeFromId(Shader, baseTypeId);
    VIR_OpCode                   opCode = VIR_Inst_GetOpcode(Inst);
    VIR_VirRegId                 regId;
    VIR_SymId                    offsetRegSymId;
    VIR_Instruction             *newInst = gcvNULL;
    VIR_Operand                 *newOperand = gcvNULL;
    VIR_ScalarConstVal           newOffset;
    VIR_Enable                   enable;
    gctUINT                      i, arrayLength = VIR_Type_GetArrayLength(Type);
    gctUINT                      destRegOffset;
    gctINT                       destStride = VIR_Type_GetRegCount(Shader, baseType, gcvFALSE);

    /* Allocate a new reg to save the data offset. */
    regId = VIR_Shader_NewVirRegId(Shader, 1);
    errCode = VIR_Shader_AddSymbol(Shader,
                                   VIR_SYM_VIRREG,
                                   regId,
                                   VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT32),
                                   VIR_STORAGE_UNKNOWN,
                                   &offsetRegSymId);
    CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed.");

    for (i = 0; i < arrayLength; i++)
    {
        destRegOffset = i * destStride;
        /* Insert a ADD to update the offset. */
        errCode = VIR_Function_AddInstructionBefore(Func,
                                                    VIR_OP_ADD,
                                                    VIR_TYPE_UINT32,
                                                    Inst,
                                                    gcvTRUE,
                                                    &newInst);
        CHECK_ERROR(errCode, "VIR_Function_AddInstructionBefore failed.");

        /* Set DEST. */
        newOperand = VIR_Inst_GetDest(newInst);
        VIR_Operand_SetTempRegister(newOperand, Func, offsetRegSymId, VIR_TYPE_UINT32);
        VIR_Operand_SetEnable(newOperand, VIR_ENABLE_X);
        VIR_Operand_SetPrecision(newOperand, VIR_PRECISION_HIGH);

        /* Set SOURCE0. */
        newOperand = VIR_Inst_GetSource(newInst, 0);
        VIR_Operand_Copy(newOperand, BaseOffset);

        /* Set SOURCE1. */
        newOperand = VIR_Inst_GetSource(newInst, 1);
        /* The array stride is saved in base address operand. */
        newOffset.uValue = i * ArrayStride;
        VIR_Operand_SetImmediate(newOperand, VIR_TYPE_UINT32, newOffset);

        /* Split instructions. */
        errCode = VIR_Function_AddInstructionBefore(Func,
                                                    opCode,
                                                    baseTypeId,
                                                    Inst,
                                                    gcvTRUE,
                                                    &newInst);
        CHECK_ERROR(errCode, "VIR_Function_AddInstructionBefore failed.");

        /* Update the dest. */
        newOperand = VIR_Inst_GetDest(newInst);
        VIR_Operand_Copy(newOperand, Dest);
        enable = VIR_TypeId_isPrimitive(baseTypeId) ? VIR_TypeId_Conv2Enable(baseTypeId) : VIR_ENABLE_NONE;
        VIR_Operand_SetTypeId(newOperand, baseTypeId);
        VIR_Operand_SetEnable(newOperand, enable);
        VIR_Operand_SetRelIndexingImmed(newOperand, destRegOffset);

        /* Copy the base addr. */
        newOperand = VIR_Inst_GetSource(newInst, 0);
        VIR_Operand_Copy(newOperand, BaseAddr);

        /* Use the updated offset.*/
        newOperand = VIR_Inst_GetSource(newInst, 1);
        VIR_Operand_SetTempRegister(newOperand, Func, offsetRegSymId, VIR_TYPE_UINT32);
        VIR_Operand_SetSwizzle(newOperand, VIR_SWIZZLE_XXXX);

        /* Update the data if needed. */
        if (Data != gcvNULL)
        {
            newOperand = VIR_Inst_GetSource(newInst, 2);
            VIR_Operand_Copy(newOperand, Data);
            VIR_Operand_SetTypeId(newOperand, baseTypeId);
            VIR_Operand_SetSwizzle(newOperand, VIR_Enable_2_Swizzle_WShift(enable));
            VIR_Operand_SetRelIndexingImmed(newOperand, destRegOffset);
        }
    }
    return errCode;
}

static VSC_ErrCode
_SplitStructMemoryAssignment(
    IN  VIR_Shader              *Shader,
    IN  VIR_Function            *Func,
    IN  VIR_Instruction         *Inst,
    IN  VIR_Type                *Type,
    IN  VIR_Operand             *Dest,
    IN  VIR_Operand             *BaseAddr,
    IN  VIR_Operand             *BaseOffset,
    IN  VIR_Operand             *Data
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_OpCode                   opCode = VIR_Inst_GetOpcode(Inst);
    VIR_VirRegId                 regId;
    VIR_SymId                    offsetRegSymId;
    VIR_Instruction             *newInst = gcvNULL;
    VIR_Operand                 *newOperand = gcvNULL;
    VIR_ScalarConstVal           newOffset;
    VIR_SymIdList               *fields = VIR_Type_GetFields(Type);
    VIR_Symbol                  *fieldSymbol;
    VIR_Id                       fieldId;
    VIR_Type                    *fieldType;
    VIR_TypeId                   fieldTypeId;
    VIR_Enable                   enable;
    gctUINT                      destRegOffset = 0;
    gctUINT                      i;

    /* Allocate a new reg to save the data offset. */
    regId = VIR_Shader_NewVirRegId(Shader, 1);
    errCode = VIR_Shader_AddSymbol(Shader,
                                   VIR_SYM_VIRREG,
                                   regId,
                                   VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT32),
                                   VIR_STORAGE_UNKNOWN,
                                   &offsetRegSymId);
    CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed.");

    for (i = 0; i < VIR_IdList_Count(fields); i++)
    {
        fieldId= VIR_IdList_GetId(VIR_Type_GetFields(Type), i);
        fieldSymbol = VIR_Shader_GetSymFromId(Shader, fieldId);
        fieldType = VIR_Symbol_GetType(fieldSymbol);
        fieldTypeId = VIR_Type_GetIndex(fieldType);

        /* Insert a ADD to update the offset. */
        errCode = VIR_Function_AddInstructionBefore(Func,
                                                    VIR_OP_ADD,
                                                    VIR_TYPE_UINT32,
                                                    Inst,
                                                    gcvTRUE,
                                                    &newInst);
        CHECK_ERROR(errCode, "VIR_Function_AddInstructionBefore failed.");

        /* Set DEST. */
        newOperand = VIR_Inst_GetDest(newInst);
        VIR_Operand_SetTempRegister(newOperand, Func, offsetRegSymId, VIR_TYPE_UINT32);
        VIR_Operand_SetEnable(newOperand, VIR_ENABLE_X);
        VIR_Operand_SetPrecision(newOperand, VIR_PRECISION_HIGH);

        /* Set SOURCE0. */
        newOperand = VIR_Inst_GetSource(newInst, 0);
        VIR_Operand_Copy(newOperand, BaseOffset);

        /* Set SOURCE1. */
        newOperand = VIR_Inst_GetSource(newInst, 1);
        newOffset.uValue = VIR_FieldInfo_GetOffset(VIR_Symbol_GetFieldInfo(fieldSymbol));
        VIR_Operand_SetImmediate(newOperand, VIR_TYPE_UINT32, newOffset);

        /* Split instructions. */
        errCode = VIR_Function_AddInstructionBefore(Func,
                                                    opCode,
                                                    fieldTypeId,
                                                    Inst,
                                                    gcvTRUE,
                                                    &newInst);
        CHECK_ERROR(errCode, "VIR_Function_AddInstructionBefore failed.");

        /* Update the dest. */
        newOperand = VIR_Inst_GetDest(newInst);
        VIR_Operand_Copy(newOperand, Dest);
        enable = VIR_TypeId_isPrimitive(fieldTypeId) ? VIR_TypeId_Conv2Enable(fieldTypeId) : VIR_ENABLE_NONE;
        VIR_Operand_SetTypeId(newOperand, fieldTypeId);
        VIR_Operand_SetEnable(newOperand, enable);
        VIR_Operand_SetRelIndexingImmed(newOperand, destRegOffset);

        /* Copy the base addr. */
        newOperand = VIR_Inst_GetSource(newInst, 0);
        VIR_Operand_Copy(newOperand, BaseAddr);

        /* Use the updated offset.*/
        newOperand = VIR_Inst_GetSource(newInst, 1);
        VIR_Operand_SetTempRegister(newOperand, Func, offsetRegSymId, VIR_TYPE_UINT32);
        VIR_Operand_SetSwizzle(newOperand, VIR_SWIZZLE_XXXX);

        /* Update the data if needed. */
        if (Data != gcvNULL)
        {
            newOperand = VIR_Inst_GetSource(newInst, 2);
            VIR_Operand_Copy(newOperand, Data);
            VIR_Operand_SetTypeId(newOperand, fieldTypeId);
            VIR_Operand_SetSwizzle(newOperand, VIR_Enable_2_Swizzle_WShift(enable));
            VIR_Operand_SetRelIndexingImmed(newOperand, destRegOffset);
        }

        destRegOffset += VIR_Type_GetRegCount(Shader, fieldType, gcvFALSE);
    }

    return errCode;
}

static VSC_ErrCode
_SplitAggregateMemoryAssignment(
    IN  VIR_Shader              *Shader,
    IN  VIR_Function            *Func,
    IN  VIR_Instruction         *Inst
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_OpCode                   opCode = VIR_Inst_GetOpcode(Inst);
    VIR_Operand                 *dest = VIR_Inst_GetDest(Inst);
    VIR_Operand                 *baseAddr = VIR_Inst_GetSource(Inst, 0);
    VIR_Operand                 *baseOffset = VIR_Inst_GetSource(Inst, 1);
    VIR_Operand                 *data = gcvNULL;
    VIR_Type                    *destType = VIR_Shader_GetTypeFromId(Shader,
                                                VIR_Operand_GetTypeId(dest));

    if (VIR_OPCODE_isMemSt(opCode))
    {
        data = VIR_Inst_GetSource(Inst, 2);
    }

    if (VIR_Type_isArray(destType))
    {
        errCode = _SplitArrayMemoryAssignment(Shader,
                                              Func,
                                              Inst,
                                              destType,
                                              VIR_Type_GetArrayStride(destType),
                                              dest,
                                              baseAddr,
                                              baseOffset,
                                              data);
        CHECK_ERROR(errCode, "_SplitArrayMemoryAssignment failed.");
    }
    else if (VIR_Type_isStruct(destType))
    {
        errCode = _SplitStructMemoryAssignment(Shader,
                                               Func,
                                               Inst,
                                               destType,
                                               dest,
                                               baseAddr,
                                               baseOffset,
                                               data);
        CHECK_ERROR(errCode, "_SplitStructMemoryAssignment failed.");
    }

    /* Change current MOV to NOP. */
    VIR_Inst_SetOpcode(Inst, VIR_OP_NOP);
    VIR_Inst_SetConditionOp(Inst, VIR_COP_ALWAYS);
    VIR_Inst_SetSrcNum(Inst, 0);
    VIR_Inst_SetDest(Inst, gcvNULL);
    return errCode;
}

static gctBOOL
_IsMemoryAssignment(
    IN  VIR_OpCode               OpCode
    )
{
    if (OpCode == VIR_OP_LOAD ||
        OpCode == VIR_OP_STORE)
    {
        return gcvTRUE;
    }
    return gcvFALSE;
}

static gctBOOL
_IsRegAssignment(
    IN  VIR_OpCode               OpCode
    )
{
    if (OpCode == VIR_OP_MOV)
    {
        return gcvTRUE;
    }
    return gcvFALSE;
}

static VSC_ErrCode _VIR_HL_Preprocess(
    IN  VIR_Shader              *Shader
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_Function                *mainFunction = VIR_Shader_GetMainFunction(Shader);
    VIR_Function                *initFunction = gcvNULL;
    VIR_FuncIterator             func_iter;
    VIR_FunctionNode            *func_node;
    VIR_Instruction             *callInst = gcvNULL;
    gctBOOL                      callInitFunc = gcvTRUE;
    gctUINT                      i;

    /*-------------------I: Update instructions. -------------------*/
    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(Shader));

    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL;
         func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function            *func = func_node->function;
        VIR_Instruction         *inst = func->instList.pHead;
        VIR_Operand             *dest;
        VIR_Type                *destType;
        VIR_Operand             *srcOpnd;
        VIR_OpCode               opcode;
        VIR_TypeId               typeId;
        gctBOOL                  useMatrix, compositeRegAssignment, aggregateMemoryAssignment;

        if (VIR_Function_HasFlag(func, VIR_FUNCFLAG_INITIALIZE_FUNC))
        {
            initFunction = func;
        }

        for(; inst != gcvNULL; inst = VIR_Inst_GetNext(inst))
        {
            useMatrix = compositeRegAssignment = aggregateMemoryAssignment = gcvFALSE;
            opcode = VIR_Inst_GetOpcode(inst);

            /* Check DEST. */
            dest = VIR_Inst_GetDest(inst);
            if (dest && VIR_Operand_GetOpKind(dest) == VIR_OPND_SYMBOL)
            {
                destType = VIR_Shader_GetTypeFromId(Shader, VIR_Operand_GetTypeId(dest));

                if ((VIR_Type_isArray(destType) || VIR_Type_isStruct(destType))
                    &&
                    _IsMemoryAssignment(opcode))
                {
                    aggregateMemoryAssignment = gcvTRUE;
                }

                if ((VIR_Type_isMatrix(destType) || VIR_Type_isArray(destType)  || VIR_Type_isStruct(destType))
                    &&
                    _IsRegAssignment(opcode))
                {
                    compositeRegAssignment = gcvTRUE;
                }

                if (VIR_Type_isMatrix(destType))
                {
                    useMatrix = gcvTRUE;
                }
            }

            if ((VIR_OPCODE_isMemSt(opcode) ||
                 VIR_OPCODE_isImgSt(opcode) ||
                 VIR_OPCODE_isAttrSt(opcode)) &&
                 VIR_Inst_Store_Have_Dst(inst))
            {
                if (VIR_Operand_GetOpKind(dest) == VIR_OPND_CONST)
                {
                    VIR_VirRegId regId = VIR_Shader_NewVirRegId(Shader, 1);
                    VIR_SymId    regSymId;

                    errCode = VIR_Shader_AddSymbol(Shader,
                                                   VIR_SYM_VIRREG,
                                                   regId,
                                                   VIR_Shader_GetTypeFromId(Shader, VIR_Operand_GetTypeId(dest)),
                                                   VIR_STORAGE_UNKNOWN,
                                                   &regSymId);

                    CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed.");

                    /* Set DEST. */
                    VIR_Operand_SetTempRegister(dest, func, regSymId, VIR_Operand_GetTypeId(dest));
                }
            }

            /* Check SOURCEs. */
            if (VIR_Inst_GetOpcode(inst) == VIR_OP_INTRINSIC)
            {
                VIR_Operand                 *paramOperand = VIR_Inst_GetSource(inst, 1);
                VIR_ParmPassing             *opndParm = VIR_Operand_GetParameters(paramOperand);
                gctUINT                      argNum = opndParm->argNum;
                gctUINT                      i;

                /* clean useMatrix, since we only expand the src matrix operands */
                useMatrix = gcvFALSE;
                for (i = 0; i < argNum; i++)
                {
                    srcOpnd = opndParm->args[i];
                    typeId = VIR_Operand_GetTypeId(srcOpnd);

                    if (VIR_TypeId_isPrimitive(typeId) && (VIR_GetTypeRows(typeId) > 1))
                    {
                        useMatrix = gcvTRUE;
                        break;
                    }
                }
            }
            else
            {
                for (i = 0; i < VIR_Inst_GetSrcNum(inst); i++)
                {
                    srcOpnd = VIR_Inst_GetSource(inst, i);
                    typeId = VIR_Operand_GetTypeId(srcOpnd);

                    if (VIR_TypeId_isPrimitive(typeId) && (VIR_GetTypeRows(typeId) > 1))
                    {
                        useMatrix = gcvTRUE;
                        break;
                    }
                }
            }

            /* Split complex assignment. */
            if (compositeRegAssignment)
            {
                errCode = _SplitCompositeRegAssignment(Shader,
                                                       func,
                                                       inst);
                CHECK_ERROR(errCode, "_SplitCompositeRegAssignment failed.");
                continue;
            }
            else if (aggregateMemoryAssignment)
            {
                errCode = _SplitAggregateMemoryAssignment(Shader,
                                                          func,
                                                          inst);
                CHECK_ERROR(errCode, "_SplitAggregateMemoryAssignment failed.");
                continue;
            }

            /* Split matrix-related instructions. */
            if (useMatrix)
            {
                errCode = _SplitMatrixRelatedInst(Shader,
                                                  func,
                                                  inst);
                CHECK_ERROR(errCode, "_SplitMatrixRelatedInst failed.");
            }

            /* spiv generate immediate as const:
               change src 1 component const to immediate */
            for (i = 0; i < VIR_Inst_GetSrcNum(inst); i++)
            {
                VIR_Const* srcConst = gcvNULL;
                srcOpnd = VIR_Inst_GetSource(inst, i);

                if (VIR_Operand_isConst(srcOpnd) &&
                    VIR_Type_isScalar(VIR_Shader_GetTypeFromId(Shader, VIR_Operand_GetTypeId(srcOpnd))))
                {
                    VIR_ScalarConstVal scalarConstVal;

                    srcConst = VIR_Shader_GetConstFromId(Shader, VIR_Operand_GetConstId(srcOpnd));

                    if (VIR_Type_isScalar(VIR_Shader_GetTypeFromId(Shader, srcConst->type)))
                    {
                        scalarConstVal = srcConst->value.scalarVal;
                    }
                    else
                    {
                        gctUINT channel = VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(srcOpnd), 1);
                        scalarConstVal.fValue = srcConst->value.vecVal.f32Value[channel];
                    }

                    VIR_Operand_SetImmediate(srcOpnd, VIR_Operand_GetTypeId(srcOpnd), scalarConstVal);
                }
            }
        }
    }

    /*-------------------II: Call the initialize function if it is no empty. -------------------*/
    gcmASSERT(initFunction);

    if (VIR_Function_GetInstCount(initFunction) == 0)
    {
        callInitFunc = gcvFALSE;
    }
    else if (VIR_Function_GetInstCount(initFunction) == 1)
    {
        if (VIR_Function_GetInstStart(initFunction) &&
            VIR_Inst_GetOpcode(VIR_Function_GetInstStart(initFunction)) == VIR_OP_RET)
        {
            callInitFunc = gcvFALSE;
        }
    }

    if (callInitFunc)
    {
        /* Call initialize function in the very beginning of main function. */
        VIR_Function_AddInstructionBefore(mainFunction,
                                          VIR_OP_CALL,
                                          VIR_TYPE_UNKNOWN,
                                          mainFunction->instList.pHead,
                                          gcvTRUE,
                                          &callInst);
        VIR_Operand_SetFunction(VIR_Inst_GetDest(callInst), initFunction);
    }

    return errCode;
}

DEF_QUERY_PASS_PROP(VIR_Lower_HighLevel_To_HighLevel_Expand)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_HL;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;
}

VSC_ErrCode
VIR_Lower_HighLevel_To_HighLevel_Expand(
    IN VSC_SH_PASS_WORKER* pPassWorker
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_PatternHL2HLContext      context;
    VIR_Shader*                  shader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VSC_MM                      *pmm = pPassWorker->basePassWorker.pMM;

    /* So far only enable this for VK. */
    if (VIR_Shader_GetClientApiVersion(shader) != gcvAPI_OPENVK)
    {
        return errCode;
    }

    if (VIR_Shader_GetLevel(shader) != VIR_SHLEVEL_Pre_High)
    {
        return errCode;
    }

    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(shader), VIR_Shader_GetId(shader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "Before HighLevel to HighLevel Expand.", shader, gcvTRUE);
    }

    _Lower_Initialize(shader, pmm, &context);

    /* Expand function arguments by expanding the argument list in VIR_OP_PARM.
    ** We need to do this before symbol allocation.
    */
    errCode = _VIR_HL_Expand_FuncArguments(shader);
    CHECK_ERROR(errCode, "_VIR_HL_Expand_FuncArguments failed.");

    /* Allocate regs for all symbols. */
    errCode = _VIR_HL_Reg_Alloc(shader, &context);
    CHECK_ERROR(errCode, "_VIR_HL_Reg_Alloc failed.");

    /* Replace symbol in operands. */
    errCode = _VIR_HL_Sym_Replace(shader, &context);
    CHECK_ERROR(errCode, "_VIR_HL_Sym_Replace failed.");

    /* Do some preprocesses before expand HL. */
    errCode = _VIR_HL_Preprocess(shader);
    CHECK_ERROR(errCode, "_VIR_HL_Preprocess failed.");

    /* Replace symbol in operands one more time. */
    errCode = _VIR_HL_Sym_Replace(shader, &context);
    CHECK_ERROR(errCode, "_VIR_HL_Sym_Replace failed.");

    /* Destroy some symbols if needed. */
    errCode = _VIR_HL_Sym_Delete(shader);
    CHECK_ERROR(errCode, "_VIR_HL_Sym_Delete failed.");

    /* Renumber instruction ID. */
    VIR_Shader_RenumberInstId(shader);

    _Lower_Finalize(shader, &context);

    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(shader), VIR_Shader_GetId(shader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "After HighLevel to HighLevel Expand.", shader, gcvTRUE);
    }

    return errCode;
}

