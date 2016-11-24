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


#ifndef __GC_VX_REFERENCE_H__
#define __GC_VX_REFERENCE_H__

EXTERN_C_BEGIN

VX_INTERNAL_API vx_status vxoReference_GetStatus(vx_reference ref);

VX_INTERNAL_API vx_reference vxoReference_Create(
        vx_context context, vx_type_e type, vx_reference_kind_e kind, vx_reference scope);

VX_INTERNAL_API void vxoReference_Initialize(
        vx_reference ref, vx_context context, vx_type_e type, vx_reference scope);

VX_INTERNAL_API void vxoReference_InitializeForDelay(
        vx_reference ref, vx_delay delay, vx_int32 slotIndex);

VX_INTERNAL_API vx_type_e vxoReference_GetType(vx_reference ref);

VX_INTERNAL_API void vxoReference_Dump(vx_reference ref);

VX_INTERNAL_API vx_bool vxoReference_IsValidAndNoncontext(vx_reference ref);

VX_INTERNAL_API vx_bool vxoReference_IsValidAndSpecific(vx_reference ref, vx_type_e type);

VX_INTERNAL_API vx_uint32 vxoReference_Increment(vx_reference ref, vx_reference_kind_e kind);

VX_INTERNAL_API vx_uint32 vxoReference_Decrement(vx_reference ref, vx_reference_kind_e kind);

VX_INTERNAL_API void vxoReference_Extract(vx_reference ref);

VX_INTERNAL_API vx_uint32 vxoReference_GetExternalCount(vx_reference ref);

VX_INTERNAL_API vx_uint32 vxoReference_GetInternalCount(vx_reference ref);

VX_INTERNAL_API vx_uint32 vxoReference_GetTotalCount(vx_reference ref);

VX_INTERNAL_API vx_status vxoReference_Release(
        INOUT vx_reference_ptr refPtr, vx_type_e type, vx_reference_kind_e kind);

VX_INTERNAL_API void vxoReference_IncrementWriteCount(vx_reference ref);

VX_INTERNAL_API void vxoReference_IncrementReadCount(vx_reference ref);


VX_INTERNAL_API vx_status vxQuerySurfaceNode(vx_reference reference, vx_uint32 plane, void **ptr);

VX_INTERNAL_API vx_status vxCommitSurfaceNode(vx_reference reference);


EXTERN_C_END

#endif /* __GC_VX_REFERENCE_H__ */

