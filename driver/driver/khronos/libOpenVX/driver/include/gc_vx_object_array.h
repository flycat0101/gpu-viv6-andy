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


#ifndef __GC_VX_OBJECT_ARRAY_H__
#define __GC_VX_OBJECT_ARRAY_H__

EXTERN_C_BEGIN

VX_INTERNAL_CALLBACK_API vx_bool vxoOA_IsValidObjectArray(vx_object_array array);

VX_INTERNAL_CALLBACK_API vx_status vxoOA_InitObjectArrayInt(vx_object_array arr, vx_reference exemplar, vx_size num_items);

VX_INTERNAL_CALLBACK_API vx_object_array vxoOA_CreateObjectArrayInt(vx_reference scope, vx_reference exemplar, vx_size count, vx_bool is_virtual);

VX_INTERNAL_CALLBACK_API void vxoOA_DestructObjectArray(vx_reference ref);

VX_INTERNAL_CALLBACK_API vx_bool vxoOA_ValidateObjectArray(vx_object_array objarr, vx_enum item_type, vx_size num_items);

VX_INTERNAL_CALLBACK_API vx_object_array vxoOA_CreateObjectArrayEmpty(vx_reference scope, vx_enum item_type, vx_size count);

VX_INTERNAL_CALLBACK_API vx_bool vxoOA_SetObjectArrayItem(vx_object_array arr, vx_reference item);

EXTERN_C_END

#endif /* __GC_VX_OBJECT_ARRAY_H__*/

