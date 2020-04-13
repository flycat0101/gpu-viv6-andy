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


#ifndef __gc_vsc_vir_linker_h_
#define __gc_vsc_vir_linker_h_

BEGIN_EXTERN_C()

#define TEXLDTYPE_NORMAL        0
#define TEXLDTYPE_PROJ          1
#define TEXLDTYPE_GATHER        2
#define TEXLDTYPE_GATHERPCF     3
#define TEXLDTYPE_FETCHMS       4
#define TEXLDTYPE_U             5

#define TEXLDMOD_NONE           0
#define TEXLDMOD_BIAS           1
#define TEXLDMOD_LOD            2
#define TEXLDMOD_GATHER         3

typedef VSC_SIMPLE_QUEUE  VIR_LIB_WORKLIST;
typedef VSC_SIMPLE_QUEUE  VIR_LIB_CALLSITES;

typedef struct _VIR_LinkLib_CONTEXT                 VIR_LinkLibContext;

typedef struct _VIR_LINKER_CALL_INST_NODE           VIR_LINKER_CALL_INST_NODE;

struct _VIR_LINKER_CALL_INST_NODE
{
    VIR_Instruction             *inst;           /* the INTRINSIC/EXTCALL instruction to be converted to CALL,
                                                  * or the CALL inst from lib func to be recursively linked in */
    union {
        VIR_IntrinsicsKind       libIntrinsicKind;  /* intrinsic kind for VIR_OP_INTRINSIC */
        VIR_NameId               extFuncName;       /* extern function name for EXTCALL */
    } u;
};

void VIR_LIB_WorkListQueue(
    IN VSC_MM                   *pMM,
    IN VIR_LIB_WORKLIST         *WorkList,
    IN VIR_Function             *Func
    );

void VIR_LIB_WorkListDequeue(
    IN VSC_MM                   *pMM,
    IN VIR_LIB_WORKLIST         *WorkList,
    OUT VIR_Function            **Func
    );

void VIR_LIB_CallSitesQueue(
    IN VSC_MM                   *pMM,
    IN VIR_LIB_CALLSITES        *pCallSites,
    IN VIR_LINKER_CALL_INST_NODE*InstNode
    );

void VIR_LIB_CallSitesDequeue(
    IN VSC_MM                   *pMM,
    IN VIR_LIB_CALLSITES        *pCallSites,
    OUT VIR_LINKER_CALL_INST_NODE**InstNode
    );

VIR_TypeId VIR_LinkLib_TypeConv(
    IN VIR_Shader   *pShader,
    IN VIR_Type     *inType,
    IN gctBOOL       ConvertSampler
    );

VSC_ErrCode
VIR_Lib_LinkFunctions(
    IN  VIR_LinkLibContext      *Context,
    IN  VIR_Shader              *pShader,
    IN  VIR_Shader              *pLibShader,
    IN  VSC_MM                  *pMM,
    OUT VSC_HASH_TABLE          *pAddLibFuncSet,
    OUT VIR_LIB_WORKLIST        *pWorkList,
    OUT VIR_LIB_CALLSITES       *pCallSites
    );

VSC_ErrCode
VIR_Lib_UpdateCallSites(
    IN  VIR_LinkLibContext      *Context,
    IN  VIR_Shader              *pShader,
    IN  VIR_Shader              *pLibShader,
    IN  VSC_HW_CONFIG           *pHwCfg,
    IN  VSC_MM                  *pMM,
    IN  VSC_HASH_TABLE          *pAddLibFuncSet,
    OUT VIR_LIB_CALLSITES       *pCallSites
    );

/* Intrinsic library list. */
typedef struct _VIR_INTRINSIC_LIBLIST
{
    VSC_UNI_LIST                intrinsicLibList;
    VSC_MM                      *pMM;
} VIR_Intrinsic_LibList;

#define VIR_Intrinsic_LibList_GetList(il)                   ((VSC_UNI_LIST*)&((il)->intrinsicLibList))
#define VIR_Intrinsic_LibList_GetMM(il)                     ((il)->pMM)
#define VIR_Intrinsic_LibList_SetMM(il, mm)                 ((il)->pMM = (mm))

typedef enum _VIR_INTRINSIC_LIBKIND
{
    /* CL lib. */
    VIR_INTRINSIC_LIB_CL                = 0x0,
    /* GL lib. */
    VIR_INTRINSIC_LIB_GL_COMMON_FUNC    = 0x1,
    VIR_INTRINSIC_LIB_GL_IMAGE_FUNC     = 0x2,
    VIR_INTRINSIC_LIB_GL_TEXLD_FUNC     = 0x3,

    VIR_INTRINSIC_LIB_MAX               = 0x4,
} VIR_Intrinsic_LibKind;

typedef struct _VIR_INTRINSIC_LIB_SOURCE
{
    VIR_Intrinsic_LibKind       libKind;
    gctSTRING                   header;
    gctSTRING                   extension;
    gctSTRING                   *body;
} VIR_Intrinsic_LibSource;

typedef struct _VIR_INTRINSIC_LIB_NODE
{
    VSC_UNI_LIST_NODE           node;
    VIR_Shader                  *pIntrinsicLib;
    VIR_Intrinsic_LibKind       libKind;
} VIR_Intrinsic_LibNode;

#define VIR_Intrinsic_LibNode_GetNode(ld)          ((VSC_UNI_LIST_NODE*)&(ld)->node)
#define VIR_Intrinsic_LibNode_GetLib(ld)           ((ld)->pIntrinsicLib)
#define VIR_Intrinsic_LibNode_SetLib(ld, lib)      ((ld)->pIntrinsicLib = (lib))
#define VIR_Intrinsic_LibNode_GetLibKind(ld)       ((ld)->libKind)
#define VIR_Intrinsic_LibNode_SetLibKind(ld, kind) ((ld)->libKind = (kind))

void
VIR_Intrinsic_LibList_Initialize(
    IN  VSC_MM                   *pMM,
    IN  VIR_Intrinsic_LibList    *pIntrinsicLibList
    );

void
VIR_Intrinsic_LibList_Finalize(
    IN  VIR_Intrinsic_LibList    *pIntrinsicLibList
    );

void
VIR_Intrinsic_LibList_AppendNode(
    IN  VIR_Intrinsic_LibList    *pIntrinsicLibList,
    IN  VIR_Shader               *pIntrinsicLib,
    IN  VIR_Intrinsic_LibKind    libKind
    );

VIR_Intrinsic_LibNode*
VIR_Intrinsic_LibList_GetNodeByLibKind(
    IN  VIR_Intrinsic_LibList    *pIntrinsicLibList,
    IN  VIR_Intrinsic_LibKind    libKind
    );

/* functions for link lib library (e.g., recompile) */
typedef VSC_SIMPLE_QUEUE  VIR_TRANS_WORKLIST;

typedef struct _VIR_LinkLib_TRANSPOINT
{
    /* for output cvt */
    VIR_Symbol  *output;

} VIR_LinkLib_Transpoint;

typedef void (*VIR_LinkLib_GET_TRANSPOINT_PTR)(
    IN  VIR_LinkLibContext      *Context,
    OUT VIR_TRANS_WORKLIST      *Transpoints
    );

typedef VSC_ErrCode (*VIR_LinkLib_GET_LIB_FUNC_NAME_PTR) (
    IN  VIR_LinkLibContext          *Context,
    IN void                         *TransPoint,
    IN  gctSTRING                   *LibFuncName
    );

typedef VSC_ErrCode (*VIR_LinkLib_INSERT_CALL_PTR) (
    IN  VIR_LinkLibContext          *Context,
    IN void                         *TransPoint,
    IN  VIR_Function                *LibFunc
    );

#define LIB_NUM 2
struct _VIR_LinkLib_CONTEXT
{
    VIR_Shader                          *shader;
    VIR_Shader                          *libShader;
    VIR_Shader                          *libShaders[LIB_NUM];
    VSC_HASH_TABLE                      *pTempHashTable;
    VIR_ShaderKind                      shaderKind;
    VSC_LIB_LINK_POINT                  *linkPoint;
    gctUINT                             libSpecializationConstantCount;
    VSC_LIB_SPECIALIZATION_CONSTANT     *libSpecializationConsts;

    gctBOOL                             changed;

    VIR_LinkLib_GET_TRANSPOINT_PTR      getTranspoint;
    VIR_LinkLib_GET_LIB_FUNC_NAME_PTR   getLibFuncName;
    VIR_LinkLib_INSERT_CALL_PTR         insertCall;
    VSC_HW_CONFIG                       *pHwCfg;
    VSC_MM                              *pMM;
};

void
_LinkLibContext_Finalize(
    IN OUT VIR_LinkLibContext      *Context
    );

VSC_ErrCode
_LinkLib_Transform(
    IN VIR_LinkLibContext *Context
    );

VSC_ErrCode
VIR_GetIntrinsicLib(
    IN VSC_HW_CONFIG            *pHwCfg,
    IN VSC_MM                   *pMM,
    IN gctBOOL                  forOCL,
    IN gctBOOL                  forGraphics,
    IN gctBOOL                  forDesktopGL,
    IN gctBOOL                   DumpShader,
    OUT VIR_Shader              **pOutLib
    );

VSC_ErrCode
VIR_DestroyIntrinsicLib(
    IN VIR_Shader              *pLib
    );

/* link the lib library */
VSC_ErrCode
VIR_LinkLibLibrary(
    IN VSC_HW_CONFIG            *pHwCfg,
    IN VSC_MM                   *pMM,
    IN VIR_Shader               *pShader,
    IN VSC_SHADER_LIB_LINK_TABLE*pLibLinkTable,
    INOUT gctBOOL               *pChanged
    );

typedef struct _VSC_EXTERNAL_LINK_PASS_DATA
{
    gctBOOL                     bChanged;
} VSC_EXTERNAL_LINK_PASS_DATA;

VSC_ErrCode
VIR_LinkExternalLibFunc(IN VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(VIR_LinkExternalLibFunc);
DECLARE_SH_NECESSITY_CHECK(VIR_LinkExternalLibFunc);

VSC_ErrCode
VIR_LinkInternalLibFunc(IN VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(VIR_LinkInternalLibFunc);
DECLARE_SH_NECESSITY_CHECK(VIR_LinkInternalLibFunc);

END_EXTERN_C()

#endif /* __gc_vsc_vir_linker_h_ */


