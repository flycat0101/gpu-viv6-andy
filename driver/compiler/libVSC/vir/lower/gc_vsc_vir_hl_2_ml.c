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


#include "gc_vsc.h"
#include "vir/lower/gc_vsc_vir_hl_2_ml.h"
#include "vir/lower/gc_vsc_vir_lower_common_func.h"
#include "vir/transform/gc_vsc_vir_misc_opts.h"

#define __MAX_SYM_NAME_LENGTH__     128

static void
_Lower_Initialize(
    IN VIR_Shader               *Shader,
    IN VIR_PatternLowerContext  *Context,
    IN VSC_HW_CONFIG            *HwCfg
    )
{
    Context->hwCfg = HwCfg;
}

static VSC_ErrCode
_SplitStructVariable(
    IN  VIR_Shader              *Shader,
    IN  VIR_IdList              *IdList,
    IN  VIR_Symbol              *StructSym,
    IN  VIR_SymbolKind           SymbolKind,
    IN  VIR_StorageClass         StorageClass,
    IN  gctSTRING                PrefixName,
    IN  VIR_Type                *Type,
    IN  gctBOOL                  SplitArray,
    IN  gctBOOL                  AllocateSym,
    IN  gctBOOL                  AllocateReg,
    OUT gctINT                  *Location,
    OUT VIR_SymId               *FirstElementId
    );

static VSC_ErrCode
_SplitArrayVariable(
    IN  VIR_Shader              *Shader,
    IN  VIR_IdList              *IdList,
    IN  VIR_Symbol              *ArraySym,
    IN  VIR_Type                *Type,
    IN  VIR_SymbolKind           SymbolKind,
    IN  VIR_StorageClass         StorageClass,
    IN  gctSTRING                PrefixName,
    IN  gctBOOL                  SplitArray,
    IN  gctBOOL                  AllocateSym,
    IN  gctBOOL                  AllocateReg,
    OUT gctINT                  *Location,
    OUT VIR_SymId               *FirstElementId
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
    IN  VIR_Symbol              *Symbol,
    IN  VIR_Type                *Type,
    IN  VIR_SymbolKind           VariableKind,
    IN  VIR_StorageClass         StorageClass,
    IN  gctBOOL                  AllocateSym,
    IN  gctBOOL                  AllocateReg,
    IN  gctSTRING                Name,
    OUT gctINT                  *Location,
    OUT VIR_SymId               *SymId
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_SymId                    symId, regSymId;
    VIR_Symbol                  *sym = gcvNULL;
    VIR_Symbol                  *regSym = gcvNULL;
    VIR_Type                    *type = (Type != gcvNULL) ? Type : VIR_Symbol_GetType(Symbol);
    VIR_NameId                   nameId;
    VIR_VirRegId                 regId;
    VIR_GeneralSymFlag           generalFlags = VIR_Symbol_GetGeneralFlag(Symbol);
    VIR_SymFlag                  flags = VIR_Symbol_GetFlag(Symbol);
    gctINT                       location = -1;
    gctUINT                      i;
    gctUINT                      regCount = VIR_Type_GetVirRegCount(Shader, type);

    /* Allocate symbol for this variable if needed. */
    if (AllocateSym)
    {
        /* Add name string. */
        errCode = VIR_Shader_AddString(Shader,
                                       Name,
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
        gcmASSERT(sym);

        if (Location)
        {
            location = _EvaluateLocation(Shader,
                                         Symbol,
                                         Type,
                                         VariableKind,
                                         StorageClass,
                                         Location);
        }
        VIR_Symbol_SetLocation(sym, location);

        switch (VariableKind)
        {
        case VIR_SYM_UNIFORM:
        case VIR_SYM_SAMPLER:
        case VIR_SYM_IMAGE:
            {
                VIR_Uniform             *virUniform;

                /* Set the sym info. */
                VIR_Symbol_SetPrecision(sym, VIR_Symbol_GetPrecision(Symbol));
                VIR_Symbol_SetAddrSpace(sym, VIR_AS_CONSTANT);
                VIR_Symbol_SetTyQualifier(sym, VIR_TYQUAL_CONST);
                VIR_Symbol_SetUniformKind(sym, VIR_UNIFORM_NORMAL);
                VIR_Symbol_SetGeneralFlag(sym, generalFlags);
                VIR_Symbol_SetFlag(sym, flags);

                /* Set the uniform sym info.*/
                virUniform = sym->u2.uniform;
                gcmASSERT(virUniform);

                virUniform->sym = symId;
                /* TODO: we need to set the correct value for those -1. */
                virUniform->blockIndex = -1;
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
                virUniform->baseBindingUniform = gcvNULL;
                /* TODO: we need to set layout info. */
                break;
            }

        case VIR_SYM_VARIABLE:
            {
                VIR_Symbol_SetGeneralFlag(sym, generalFlags);
                VIR_Symbol_SetFlag(sym, flags);
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

        for (i = 0; i < regCount; i++)
        {
            errCode = VIR_Shader_AddSymbol(Shader,
                                           VIR_SYM_VIRREG,
                                           regId + i,
                                           VIR_Shader_GetTypeFromId(Shader, VIR_Type_GetBaseTypeId(type)),
                                           VIR_STORAGE_UNKNOWN,
                                           &regSymId);
            CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed.");

            regSym = VIR_Shader_GetSymFromId(Shader, regSymId);
            gcmASSERT(regSym);

            VIR_Symbol_SetVregVariable(regSym, sym);
            VIR_Symbol_SetPrecision(regSym, VIR_Symbol_GetPrecision(sym));
        }
    }

    if (SymId)
    {
        *SymId = symId;
    }

    return errCode;
}

static VSC_ErrCode
_SplitStructVariable(
    IN  VIR_Shader              *Shader,
    IN  VIR_IdList              *IdList,
    IN  VIR_Symbol              *StructSym,
    IN  VIR_SymbolKind           SymbolKind,
    IN  VIR_StorageClass         StorageClass,
    IN  gctSTRING                PrefixName,
    IN  VIR_Type                *Type,
    IN  gctBOOL                  SplitArray,
    IN  gctBOOL                  AllocateSym,
    IN  gctBOOL                  AllocateReg,
    OUT gctINT                  *Location,
    OUT VIR_SymId               *FirstElementId
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

    for (i = 0; i < VIR_IdList_Count(fields); i++)
    {
        id = VIR_IdList_GetId(VIR_Type_GetFields(Type), i);
        fieldSym = VIR_Shader_GetSymFromId(Shader, id);
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
                                               IdList,
                                               fieldSym,
                                               SymbolKind,
                                               StorageClass,
                                               mixName,
                                               fieldType,
                                               SplitArray,
                                               AllocateSym,
                                               AllocateReg,
                                               Location,
                                               FirstElementId);
                CHECK_ERROR(errCode, "_SplitStructVariable failed.");
                continue;
            }
            else
            {
                gcmASSERT(VIR_Type_GetKind(fieldType) == VIR_TY_ARRAY);
                errCode = _SplitArrayVariable(Shader,
                                              IdList,
                                              fieldSym,
                                              fieldType,
                                              SymbolKind,
                                              StorageClass,
                                              mixName,
                                              SplitArray,
                                              AllocateSym,
                                              AllocateReg,
                                              Location,
                                              FirstElementId);
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
                                      fieldSym,
                                      gcvNULL,
                                      fieldKind,
                                      StorageClass,
                                      AllocateSym,
                                      AllocateReg,
                                      mixName,
                                      Location,
                                      &symId);
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
    IN  VIR_IdList              *IdList,
    IN  VIR_Symbol              *ArraySym,
    IN  VIR_Type                *Type,
    IN  VIR_SymbolKind           SymbolKind,
    IN  VIR_StorageClass         StorageClass,
    IN  gctSTRING                PrefixName,
    IN  gctBOOL                  SplitArray,
    IN  gctBOOL                  AllocateSym,
    IN  gctBOOL                  AllocateReg,
    OUT gctINT                  *Location,
    OUT VIR_SymId               *FirstElementId
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
                                                   IdList,
                                                   ArraySym,
                                                   SymbolKind,
                                                   StorageClass,
                                                   mixName,
                                                   baseType,
                                                   SplitArray,
                                                   AllocateSym,
                                                   AllocateReg,
                                                   Location,
                                                   FirstElementId);
                    CHECK_ERROR(errCode, "_SplitStructVariable failed.");
                }
                else
                {
                    gcmASSERT(VIR_Type_GetKind(baseType) == VIR_TY_ARRAY);
                    errCode = _SplitArrayVariable(Shader,
                                                  IdList,
                                                  ArraySym,
                                                  baseType,
                                                  SymbolKind,
                                                  StorageClass,
                                                  mixName,
                                                  SplitArray,
                                                  AllocateSym,
                                                  AllocateReg,
                                                  Location,
                                                  FirstElementId);
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
                                              ArraySym,
                                              baseType,
                                              symbolKind,
                                              StorageClass,
                                              AllocateSym,
                                              AllocateReg,
                                              mixName,
                                              Location,
                                              &symId);
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
                                      ArraySym,
                                      gcvNULL,
                                      symbolKind,
                                      StorageClass,
                                      AllocateSym,
                                      AllocateReg,
                                      PrefixName,
                                      Location,
                                      &symId);
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

    gcmASSERT(!VIR_Type_isPrimitive(type));

    if (VIR_Type_GetKind(type) == VIR_TY_ARRAY)
    {
        errCode = _SplitArrayVariable(Shader,
                                      IdList,
                                      UniformSym,
                                      type,
                                      VIR_SYM_UNIFORM,
                                      StorageClass,
                                      uniformName,
                                      gcvFALSE,
                                      gcvTRUE,
                                      gcvFALSE,
                                      Location,
                                      &firstElementId);
        CHECK_ERROR(errCode, "_SplitArrayVariable failed.");
    }
    else
    {
        gcmASSERT(VIR_Type_GetKind(type) == VIR_TY_STRUCT);

        errCode = _SplitStructVariable(Shader,
                                       IdList,
                                       UniformSym,
                                       VIR_SYM_UNIFORM,
                                       StorageClass,
                                       uniformName,
                                       type,
                                       gcvFALSE,
                                       gcvTRUE,
                                       gcvFALSE,
                                       Location,
                                       &firstElementId);
        CHECK_ERROR(errCode, "_SplitStructVariable failed.");
    }

    /* Set the first element ID. */
    gcmASSERT(firstElementId != VIR_INVALID_ID);
    VIR_Symbol_SetFirstElementId(UniformSym, firstElementId);

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

    if (VIR_Type_IsBaseTypeStruct(Shader, type))
    {
        if (VIR_Type_GetKind(type) == VIR_TY_ARRAY)
        {
            errCode = _SplitArrayVariable(Shader,
                                          IdList,
                                          VariableSym,
                                          type,
                                          VIR_SYM_VARIABLE,
                                          StorageClass,
                                          name,
                                          gcvFALSE,
                                          gcvTRUE,
                                          gcvTRUE,
                                          Location,
                                          &firstElementId);
            CHECK_ERROR(errCode, "_SplitArrayVariable failed.");
        }
        else
        {
            gcmASSERT(VIR_Type_GetKind(type) == VIR_TY_STRUCT);

            errCode = _SplitStructVariable(Shader,
                                           IdList,
                                           VariableSym,
                                           VIR_SYM_VARIABLE,
                                           StorageClass,
                                           name,
                                           type,
                                           gcvFALSE,
                                           gcvTRUE,
                                           gcvTRUE,
                                           Location,
                                           &firstElementId);
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
                                      gcvNULL,
                                      VIR_SYM_VARIABLE,
                                      StorageClass,
                                      gcvFALSE,
                                      gcvTRUE,
                                      name,
                                      Location,
                                      &symId);
        CHECK_ERROR(errCode, "_AddGeneralVariable failed.");
    }

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

    if (VIR_Type_IsBaseTypeStruct(Shader, type))
    {
        if (VIR_Type_GetKind(type) == VIR_TY_ARRAY)
        {
            errCode = _SplitArrayVariable(Shader,
                                          IdList,
                                          OutputSym,
                                          type,
                                          VIR_SYM_VARIABLE,
                                          StorageClass,
                                          name,
                                          splitArray,
                                          gcvTRUE,
                                          gcvTRUE,
                                          Location,
                                          &firstElementId);
            CHECK_ERROR(errCode, "_SplitArrayVariable failed.");
        }
        else
        {
            gcmASSERT(VIR_Type_GetKind(type) == VIR_TY_STRUCT);

            errCode = _SplitStructVariable(Shader,
                                           IdList,
                                           OutputSym,
                                           VIR_SYM_VARIABLE,
                                           StorageClass,
                                           name,
                                           type,
                                           splitArray,
                                           gcvTRUE,
                                           gcvTRUE,
                                           Location,
                                           &firstElementId);
            CHECK_ERROR(errCode, "_SplitStructVariable failed.");
        }

        /* Set the first element ID. */
        gcmASSERT(firstElementId != VIR_INVALID_ID);
        VIR_Symbol_SetFirstElementId(OutputSym, firstElementId);
    }
    else
    {
        {
            errCode = _AddGeneralVariable(Shader,
                                          OutputSym,
                                          gcvNULL,
                                          VIR_SYM_VARIABLE,
                                          StorageClass,
                                          gcvFALSE,
                                          gcvTRUE,
                                          name,
                                          Location,
                                          &symId);
            CHECK_ERROR(errCode, "_AddGeneralVariable failed.");
        }
    }

    return errCode;
}

VSC_ErrCode
VIR_Lower_HighLevel_To_MiddleLevel_Reg_Alloc(
    IN  VIR_Shader              *Shader
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_UniformIdList           *uniformList = VIR_Shader_GetUniforms(Shader);
    VIR_UBOIdList               *uboList = VIR_Shader_GetUniformBlocks(Shader);
    VIR_VariableIdList          *variableList = VIR_Shader_GetVaribles(Shader);
    VIR_OutputIdList            *outputList = VIR_Shader_GetOutputs(Shader);
    VIR_InputIdList             *inputList = VIR_Shader_GetAttributes(Shader);
    VIR_SBOIdList               *sboList = VIR_Shader_GetSSBlocks(Shader);
    VIR_IOBIdList               *iobList = VIR_Shader_GetIOBlocks(Shader);
    VIR_Symbol                  *symbol = gcvNULL;
    VIR_SymId                    symId;
    VIR_VirRegId                 regId;
    VIR_StorageClass             storageClass;
    gctINT                       location;
    gctUINT                      listLength;
    gctUINT                      i;

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

    /* Uniforms:
    ** Split struct uniforms.
    */
    listLength = VIR_IdList_Count(uniformList);
    for (i = 0; i < listLength; i++)
    {
        symbol = VIR_Shader_GetSymFromId(Shader, VIR_IdList_GetId(uniformList, i));
        storageClass = VIR_Symbol_GetStorageClass(symbol);

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

    /* UBO:
    ** Split UBOs.
    */
    listLength = VIR_IdList_Count(uboList);
    for (i = 0; i < listLength; i++)
    {
        symbol = VIR_Shader_GetSymFromId(Shader, VIR_IdList_GetId(uboList, i));
    }

    /* Variables:
    ** Split struct variables and allocate regs for all variables.
    */
    listLength = VIR_IdList_Count(variableList);
    for (i = 0; i < listLength; i++)
    {
        symbol = VIR_Shader_GetSymFromId(Shader, VIR_IdList_GetId(variableList, i));
        storageClass = VIR_Symbol_GetStorageClass(symbol);

        if (!VIR_Symbol_HasGeneralFlag(symbol, VIR_GENERAL_SYMFLAG_WITHOUT_REG))
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
        VIR_Symbol_ClrGeneralFlag(symbol, VIR_GENERAL_SYMFLAG_WITHOUT_REG);
    }

    /* Inputs:
    ** 1) Split struct inputs, and allocate regs for all inputs.
    ** 2) Delete struct inputs if needed.
    */
    listLength = VIR_IdList_Count(inputList);
    for (i = 0; i < listLength; i++)
    {
        symbol = VIR_Shader_GetSymFromId(Shader, VIR_IdList_GetId(inputList, i));
        storageClass = VIR_Symbol_GetStorageClass(symbol);

        if (!VIR_Symbol_HasGeneralFlag(symbol, VIR_GENERAL_SYMFLAG_WITHOUT_REG))
        {
            continue;
        }

        location = VIR_Symbol_GetLocation(symbol);

        errCode = _SplitVariables(Shader,
                                  inputList,
                                  i,
                                  symbol,
                                  storageClass,
                                  (location == -1) ? gcvNULL : &location);
        CHECK_ERROR(errCode, "_SplitVariables failed.");

        /* Mark symbol as reg allocated. */
        VIR_Symbol_ClrGeneralFlag(symbol, VIR_GENERAL_SYMFLAG_WITHOUT_REG);
    }

    /* Outputs:
    ** 1) Split struct and array outputs, and allocate regs for all outputs.
    ** 2) Delete struct and array output if needed.
    */
    listLength = VIR_IdList_Count(outputList);
    for (i = 0; i < listLength; i++)
    {
        symbol = VIR_Shader_GetSymFromId(Shader, VIR_IdList_GetId(outputList, i));
        storageClass = VIR_Symbol_GetStorageClass(symbol);

        if (!VIR_Symbol_HasGeneralFlag(symbol, VIR_GENERAL_SYMFLAG_WITHOUT_REG))
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
        VIR_Symbol_ClrGeneralFlag(symbol, VIR_GENERAL_SYMFLAG_WITHOUT_REG);
    }

    /* SBO:
    */
    listLength = VIR_IdList_Count(sboList);
    for (i = 0; i < listLength; i++)
    {
    }

    /* IOB:
    */
    listLength = VIR_IdList_Count(iobList);
    for (i = 0; i < listLength; i++)
    {
    }

    return errCode;
}

static VSC_ErrCode
_ReplaceSymWithFirstElementSym(
    IN  VIR_Shader              *Shader,
    IN  VIR_Operand             *Operand
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_Symbol                  *opndSym;
    VIR_Symbol                  *elementSym;
    VIR_Symbol                  *virRegSym;
    VIR_SymId                    elementSymId, virRegSymId;
    VIR_SymbolKind               elementSymKind;

    /* Ignore non-symbol operands. */
    if (Operand == gcvNULL || !VIR_Operand_isSymbol(Operand))
    {
        return errCode;
    }

    /* Ignore simple symbol operands. */
    opndSym = VIR_Operand_GetSymbol(Operand);
    gcmASSERT(!VIR_Symbol_HasGeneralFlag(opndSym, VIR_GENERAL_SYMFLAG_WITHOUT_REG));

    elementSymId = VIR_Symbol_GetFirstElementId(opndSym);
    if (elementSymId == VIR_INVALID_ID)
    {
        return errCode;
    }

    elementSym = VIR_Shader_GetSymFromId(Shader, elementSymId);
    elementSymKind = VIR_Symbol_GetKind(elementSym);

    /*
    ** If this symbol is uniform, just replace it by element symbol.
    ** For the rest, replace it by reg symbol.
    */
    if (elementSymKind == VIR_SYM_UNIFORM)
    {
        VIR_Operand_SetSym(Operand, elementSym);
    }
    else
    {
        gcmASSERT(elementSymKind == VIR_SYM_VARIABLE);

        errCode = VIR_Shader_GetVirRegSymByVirRegId(Shader,
                                                    VIR_Symbol_GetVariableVregIndex(elementSym),
                                                    &virRegSymId);
        CHECK_ERROR(errCode, "VIR_Shader_GetVirRegSymByVirRegId failed.")
        virRegSym = VIR_Shader_GetSymFromId(Shader, virRegSymId);
        gcmASSERT(virRegSym);

        VIR_Operand_SetSym(Operand, virRegSym);
        /* TODO: do we need to change kind to VIR_OPND_VIRREG? */
        VIR_Operand_SetOpKind(Operand, VIR_OPND_SYMBOL);
    }

    return errCode;
}

VSC_ErrCode
VIR_Lower_HighLevel_To_MiddleLevel_Sym_Update(
    IN  VIR_Shader              *Shader
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_FuncIterator             func_iter;
    VIR_FunctionNode            *func_node;
    VIR_OutputIdList            *outputList = VIR_Shader_GetOutputs(Shader);
    VIR_InputIdList             *inputList = VIR_Shader_GetAttributes(Shader);
    VIR_Symbol                  *symbol;
    VIR_TypeKind                 typeKind;
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
            /* I:
            ** If the firstElementId of a symbol is not INVALID,
            ** replace this symbol by its first element.
            */
            errCode = _ReplaceSymWithFirstElementSym(Shader,
                                                     VIR_Inst_GetDest(inst));
            CHECK_ERROR(errCode, "_ReplaceSymWithFirstElementSym failed.")

            for (i = 0; i < VIR_Inst_GetSrcNum(inst); i++)
            {
                errCode = _ReplaceSymWithFirstElementSym(Shader,
                                                         VIR_Inst_GetSource(inst, i));
                CHECK_ERROR(errCode, "_ReplaceSymWithFirstElementSym failed.")
            }
            /* TODO: Do we need to handle texld parameters here? */
        }
    }

    /*-------------------II: Remove some symbols from ID list-------------------*/
    /* Remove struct outputs. */
    for (i = 0; i < VIR_IdList_Count(outputList); i++)
    {
        symbol = VIR_Shader_GetSymFromId(Shader, VIR_IdList_GetId(outputList, i));
        if (!symbol)
        {
            continue;
        }

        typeKind = VIR_Type_GetKind(VIR_Symbol_GetType(symbol));
        if (/*typeKind == VIR_TY_ARRAY || */typeKind == VIR_TY_STRUCT)
        {
            VIR_IdList_DeleteByIndex(outputList, i);
        }
    }

    /* Remove struct inputs. */
    for (i = 0; i < VIR_IdList_Count(inputList); i++)
    {
        symbol = VIR_Shader_GetSymFromId(Shader, VIR_IdList_GetId(inputList, i));
        if (!symbol)
        {
            continue;
        }

        typeKind = VIR_Type_GetKind(VIR_Symbol_GetType(symbol));
        if (typeKind == VIR_TY_STRUCT)
        {
            VIR_IdList_DeleteByIndex(inputList, i);
        }
    }
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
    VIR_TypeId                   matrixTypeId = VIR_Operand_GetType(MatrixOperand);
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
                VIR_Function_DupOperand(Func, MatrixOperand, &vectorOperand);
                VIR_Operand_SetMatrixConstIndex(vectorOperand, MatrixIndex);
                VIR_Operand_SetType(vectorOperand, rowTypeId);
            }
            else
            {
                errCode = VIR_Shader_GetVirRegSymByVirRegId(Shader,
                                                            VIR_Symbol_GetVregIndex(symbol) + MatrixIndex,
                                                            &symId);
                CHECK_ERROR(errCode, "VIR_Shader_GetVirRegSymByVirRegId failed.")
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
            CHECK_ERROR(errCode, "VIR_Shader_GetVirRegSymByVirRegId failed.")
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
    VIR_TypeId                   src0TypeId = VIR_Operand_GetType(VIR_Inst_GetSource(Inst, 0));
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
                                                    &newInst);
        CHECK_ERROR(errCode, "VIR_Function_AddInstructionBefore failed.")

        /* Set DEST. */
        newOperand = VIR_Inst_GetDest(newInst);
        errCode = _ConvMatrixOperandToVectorOperand(Shader,
                                                    Func,
                                                    VIR_Inst_GetDest(Inst),
                                                    gcvTRUE,
                                                    i,
                                                    &newOperand);
        CHECK_ERROR(errCode, "_ConvMatrixOperandToVectorOperand failed.")
        VIR_Inst_SetDest(newInst, newOperand);

        /* Set SOURCE0. */
        newOperand = VIR_Inst_GetSource(newInst, 0);
        errCode = _ConvMatrixOperandToVectorOperand(Shader,
                                                    Func,
                                                    VIR_Inst_GetSource(Inst, 0),
                                                    gcvFALSE,
                                                    i,
                                                    &newOperand);
        CHECK_ERROR(errCode, "_ConvMatrixOperandToVectorOperand failed.")
        VIR_Inst_SetSource(newInst, 0, newOperand);

        /* Set SOURCE1. */
        VIR_Function_DupOperand(Func, VIR_Inst_GetSource(Inst, 1), &newOperand);
        VIR_Inst_SetSource(newInst, 1, newOperand);
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

    src0TypeId = VIR_Operand_GetType(Source0);
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
                                                    &newInst);
        CHECK_ERROR(errCode, "VIR_Function_AddInstructionBefore failed.")

        /* Create a symbol to save DEST. */
        virRegId = VIR_Shader_NewVirRegId(Shader, 1);
        errCode = VIR_Shader_AddSymbol(Shader,
                                       VIR_SYM_VIRREG,
                                       virRegId,
                                       VIR_Shader_GetTypeFromId(Shader, vectorTypeId),
                                       VIR_STORAGE_UNKNOWN,
                                       &currentSymId);
        CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed.")

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
        CHECK_ERROR(errCode, "_ConvMatrixOperandToVectorOperand failed.")
        src0Swizzle = VIR_Operand_GetSwizzle(newOperand);
        VIR_Inst_SetSource(newInst, 0, newOperand);

        /* Set SOURCE1. */
        VIR_Function_DupOperand(Func, Source1, &newOperand);
        VIR_Operand_SetSwizzle(newOperand,
            VIR_Swizzle_Trim(VIR_Operand_GetSwizzle(Source1), (VIR_Enable)(1 << i)));
        VIR_Inst_SetSource(newInst, 1, newOperand);

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
    VIR_TypeId                   src0TypeId = VIR_Operand_GetType(src0);
    VIR_TypeId                   vectorTypeId = VIR_GetTypeRowType(src0TypeId);
    VIR_SymId                    returnValueSymId[4] = {VIR_INVALID_ID, VIR_INVALID_ID, VIR_INVALID_ID, VIR_INVALID_ID};
    VIR_Instruction             *newInst;
    VIR_Swizzle                  src0Swizzle;
    gctINT                       src0ComponetCount = VIR_GetTypeComponents(src0TypeId);
    gctINT                       i;

    gcmASSERT(src0ComponetCount <= 4 && src0ComponetCount > 1);
    src0Swizzle = VIR_Swizzle_GenSwizzleByComponentCount(src0ComponetCount);

    /* Split MatrixTimesMatrix to several MatrixTimesVector. */
    for (i = 0; i < src0ComponetCount; i++)
    {
        /* Convert source1 from matrix to matrix[i]. */
        VIR_Function_DupOperand(Func, src1, &vectorOperand);
        errCode = _ConvMatrixOperandToVectorOperand(Shader,
                                                    Func,
                                                    src1,
                                                    gcvFALSE,
                                                    i,
                                                    &vectorOperand);
        CHECK_ERROR(errCode, "_ConvMatrixOperandToVectorOperand failed.")

        errCode = _SplitMatrixMulVector(Shader,
                                        Func,
                                        Inst,
                                        src0,
                                        vectorOperand,
                                        gcvFALSE,
                                        &returnValueSymId[i]);
        CHECK_ERROR(errCode, "_SplitMatrixMulVector failed.")
        gcmASSERT(returnValueSymId[i] != VIR_INVALID_ID);

        /* Save the return value. */
        errCode = VIR_Function_AddInstructionBefore(Func,
                                                    VIR_OP_MOV,
                                                    vectorTypeId,
                                                    Inst,
                                                    &newInst);
        CHECK_ERROR(errCode, "VIR_Function_AddInstructionBefore failed.")

        /* Set DEST. */
        newOperand = VIR_Inst_GetDest(newInst);
        errCode = _ConvMatrixOperandToVectorOperand(Shader,
                                                    Func,
                                                    VIR_Inst_GetDest(Inst),
                                                    gcvTRUE,
                                                    i,
                                                    &newOperand);
        CHECK_ERROR(errCode, "_ConvMatrixOperandToVectorOperand failed.")
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
    VIR_TypeId                   src1TypeId = VIR_Operand_GetType(src1);
    VIR_TypeId                   componentTypeId = VIR_GetTypeComponentType(src1TypeId);
    VIR_TypeId                   vectorTypeId = VIR_Operand_GetType(VIR_Inst_GetDest(Inst));
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
    CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed.")
    gcmASSERT(symId != VIR_INVALID_ID);

    /* General several DPs. */
    for (i = 0; i < rowCount; i++)
    {
        errCode = VIR_Function_AddInstructionBefore(Func,
                                                    opcode,
                                                    componentTypeId,
                                                    Inst,
                                                    &newInst);
        CHECK_ERROR(errCode, "VIR_Function_AddInstructionBefore failed.")

        /* Set DEST. */
        newOperand = VIR_Inst_GetDest(newInst);
        VIR_Operand_SetTempRegister(newOperand,
                                    Func,
                                    symId,
                                    componentTypeId);
        VIR_Operand_SetEnable(newOperand, VIR_ENABLE_X << i);
        VIR_Inst_SetDest(newInst, newOperand);

        /* Set Source0. */
        VIR_Function_DupOperand(Func, src0, &newOperand);
        VIR_Inst_SetSource(newInst, 0, newOperand);

        /* Set SOURCE1. */
        newOperand = VIR_Inst_GetSource(newInst, 1);
        errCode = _ConvMatrixOperandToVectorOperand(Shader,
                                                    Func,
                                                    src1,
                                                    gcvFALSE,
                                                    i,
                                                    &newOperand);
        CHECK_ERROR(errCode, "_ConvMatrixOperandToVectorOperand failed.")
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
    gctBOOL                      isSrc1Vector = gcvFALSE;

    src0TypeId = VIR_Operand_GetType(src0);
    src1TypeId = VIR_Operand_GetType(src1);

    gcmASSERT(VIR_TypeId_isPrimitive(VIR_Operand_GetType(VIR_Inst_GetDest(Inst))) &&
              VIR_TypeId_isPrimitive(src0TypeId) &&
              VIR_TypeId_isPrimitive(src1TypeId));

    isSrc0Matrix = (VIR_GetTypeRows(src0TypeId) > 1);
    isSrc1Matrix = (VIR_GetTypeRows(src1TypeId) > 1);

    isSrc1Vector = (VIR_GetTypeComponents(src1TypeId) > 1);

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
        CHECK_ERROR(errCode, "_SplitMatrixMulVector failed.")
    }
    /* Matrix times Scalar */
    else if (isSrc0Matrix && !isSrc1Matrix && !isSrc1Vector)
    {
        errCode = _SplitMatrixMulScalar(Shader, Func, Inst);
        CHECK_ERROR(errCode, "_SplitMatrixMulScalar failed.")
    }
    /* Vector times Matrix */
    else
    {
        gcmASSERT((VIR_GetTypeComponents(src0TypeId) > 1) && isSrc1Matrix);

        errCode = _SplitVectorMulMatrix(Shader, Func, Inst);
        CHECK_ERROR(errCode, "_SplitVectorMulMatrix failed.")
    }

    return errCode;
}

static VSC_ErrCode
_SplitMatrixDiv(
    IN  VIR_Shader              *Shader,
    IN  VIR_Function            *Func,
    IN  VIR_Instruction         *Inst
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;

    return errCode;
}

static VSC_ErrCode
_SplitMatrixMov(
    IN  VIR_Shader              *Shader,
    IN  VIR_Function            *Func,
    IN  VIR_Instruction         *Inst
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_TypeId                   src0TypeId = VIR_Operand_GetType(VIR_Inst_GetSource(Inst, 0));
    VIR_TypeId                   newTypeId = VIR_GetTypeRowType(src0TypeId);
    VIR_Instruction             *newInst = gcvNULL;
    VIR_Operand                 *newOperand;
    gctINT                       rowCount = VIR_GetTypeRows(src0TypeId);
    gctINT                       i;

    /* Split matrix MOV to vector MOV. */
    for (i = 0; i < rowCount; i++)
    {
        errCode = VIR_Function_AddInstructionBefore(Func,
                                                    VIR_OP_MOV,
                                                    newTypeId,
                                                    Inst,
                                                    &newInst);
        CHECK_ERROR(errCode, "VIR_Function_AddInstructionBefore failed.")

        /* Set DEST. */
        newOperand = VIR_Inst_GetDest(newInst);
        errCode = _ConvMatrixOperandToVectorOperand(Shader,
                                                    Func,
                                                    VIR_Inst_GetDest(Inst),
                                                    gcvTRUE,
                                                    i,
                                                    &newOperand);
        CHECK_ERROR(errCode, "_ConvMatrixOperandToVectorOperand failed.")
        VIR_Inst_SetDest(newInst, newOperand);

        /* Set SOURCE0. */
        newOperand = VIR_Inst_GetSource(newInst, 0);
        errCode = _ConvMatrixOperandToVectorOperand(Shader,
                                                    Func,
                                                    VIR_Inst_GetSource(Inst, 0),
                                                    gcvFALSE,
                                                    i,
                                                    &newOperand);
        CHECK_ERROR(errCode, "_ConvMatrixOperandToVectorOperand failed.")
        VIR_Inst_SetSource(newInst, 0, newOperand);
    }

    /* Change current MOV to NOP. */
    VIR_Inst_SetOpcode(Inst, VIR_OP_NOP);
    VIR_Inst_SetConditionOp(Inst, VIR_COP_ALWAYS);
    VIR_Inst_SetSrcNum(Inst, 0);
    VIR_Inst_SetDest(Inst, gcvNULL);

    return errCode;
}

VSC_ErrCode
VIR_Lower_HighLevel_To_MiddleLevel_Preprocess(
    IN  VIR_Shader              *Shader
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    gctUINT                      i;

    /* Split matrix-related instructions. */
    {
        VIR_FuncIterator         func_iter;
        VIR_FunctionNode        *func_node;

        VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(Shader));

        for (func_node = VIR_FuncIterator_First(&func_iter);
             func_node != gcvNULL;
             func_node = VIR_FuncIterator_Next(&func_iter))
        {
            VIR_Function        *func = func_node->function;
            VIR_Instruction     *inst = func->instList.pHead;
            VIR_Operand         *srcOpnd;
            VIR_TypeId           typeId;
            gctBOOL              useMatrix = gcvFALSE;

            for(; inst != gcvNULL; inst = VIR_Inst_GetNext(inst))
            {
                useMatrix = gcvFALSE;

                for (i = 0; i < VIR_Inst_GetSrcNum(inst); i++)
                {
                    srcOpnd = VIR_Inst_GetSource(inst, i);
                    typeId = VIR_Operand_GetType(srcOpnd);

                    if (VIR_TypeId_isPrimitive(typeId) &&
                        (VIR_GetTypeRows(typeId) > 1))
                    {
                        useMatrix = gcvTRUE;
                        break;
                    }
                }

                if (!useMatrix)
                {
                    continue;
                }

                switch (VIR_Inst_GetOpcode(inst))
                {
                case VIR_OP_ADD:
                    /* SPIRV can only support scalar or vector ADD now. */
                    gcmASSERT(gcvFALSE);
                    errCode = _SplitMatrixAdd(Shader,
                                              func,
                                              inst);
                    CHECK_ERROR(errCode, "_SplitMatrixAdd failed.");
                    break;

                case VIR_OP_MUL:
                    errCode = _SplitMatrixMul(Shader,
                                              func,
                                              inst);
                    CHECK_ERROR(errCode, "_SplitMatrixAdd failed.");
                    break;

                case VIR_OP_DIV:
                    /* SPIRV can only support scalar or vector DIV now. */
                     gcmASSERT(gcvFALSE);
                    errCode = _SplitMatrixDiv(Shader,
                                              func,
                                              inst);
                    CHECK_ERROR(errCode, "_SplitMatrixAdd failed.");
                    break;

                case VIR_OP_MOV:
                    errCode = _SplitMatrixMov(Shader,
                                              func,
                                              inst);
                    CHECK_ERROR(errCode, "_SplitMatrixMov failed.");
                    break;

                default:
                    gcmASSERT(gcvFALSE);
                    break;
                }
            }
        }
    }

    return errCode;
}

VSC_ErrCode
VIR_Lower_HighLevel_To_MiddleLevel(
    IN  VIR_Shader              *Shader,
    IN  VSC_HW_CONFIG           *HwCfg
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_PatternLowerContext      context;

    gcmASSERT(VIR_Shader_GetLevel(Shader) == VIR_SHLEVEL_Post_High);

    if (gcSHADER_DumpCodeGenVerbose(Shader))
    {
        VIR_Shader_Dump(gcvNULL, "Before HighLevel to MiddleLevel.", Shader, gcvTRUE);
    }

    _Lower_Initialize(Shader, &context, HwCfg);

    /* Allocate regs for all symbols. */
    errCode = VIR_Lower_HighLevel_To_MiddleLevel_Reg_Alloc(Shader);
    CHECK_ERROR(errCode, "VIR_Lower_HighLevel_To_MiddleLevel_Reg_Alloc failed.");

    /* Update symbol in operands. */
    errCode = VIR_Lower_HighLevel_To_MiddleLevel_Sym_Update(Shader);
    CHECK_ERROR(errCode, "VIR_Lower_HighLevel_To_MiddleLevel_Sym_Update failed.");

    /* Do some preprocesses before expand HL. */
    errCode = VIR_Lower_HighLevel_To_MiddleLevel_Preprocess(Shader);
    CHECK_ERROR(errCode, "VIR_Lower_HighLevel_To_MiddleLevel_Preprocess failed.");

    /* Expand HL instructions to ML opcodes. */
    errCode = VIR_Lower_HighLevel_To_MiddleLevel_Expand(Shader, HwCfg, &context);
    CHECK_ERROR(errCode, "VIR_Lower_HighLevel_To_MiddleLevel_Expand failed.");

    /* Renumber instruction ID. */
    vscVIR_RemoveNop(Shader);
    VIR_Shader_RenumberInstId(Shader);

    if (gcSHADER_DumpCodeGenVerbose(Shader))
    {
        VIR_Shader_Dump(gcvNULL, "After HighLevel to MiddleLevel.", Shader, gcvTRUE);
    }

    return errCode;
}

