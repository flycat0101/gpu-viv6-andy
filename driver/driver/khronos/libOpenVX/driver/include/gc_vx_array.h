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


#ifndef __GC_VX_ARRAY_H__
#define __GC_VX_ARRAY_H__

EXTERN_C_BEGIN

VX_INTERNAL_API void vxoArray_Dump(vx_array array);

VX_INTERNAL_API vx_array vxoArray_Create(
        vx_context context, vx_enum itemType, vx_size capacity, vx_bool isVirtual, vx_enum type);

VX_INTERNAL_CALLBACK_API void vxoArray_Destructor(vx_reference reference);

VX_INTERNAL_API vx_bool vxoArray_InitializeAsVirtual(vx_array array, vx_enum itemType, vx_size capacity);

VX_INTERNAL_API vx_bool vxoArray_CheckItemTypeAndCapacity(vx_array array, vx_enum itemType, vx_size capacity);

VX_INTERNAL_API vx_bool vxoArray_AllocateMemory(vx_array array);

VX_INTERNAL_API vx_status vxoArray_AccessRange(vx_array array, vx_size start, vx_size end, void **ptr, vx_enum usage);

VX_INTERNAL_API vx_status vxoArray_CommitRange(vx_array array, vx_size start, vx_size end, void *ptr);

VX_INTERNAL_API vx_status vxSetArrayAttribute(vx_array array, vx_enum attribute, void *ptr, vx_size size);

EXTERN_C_END

#endif /* __GC_VX_ARRAY_H__*/
