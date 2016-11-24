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


#ifndef __GC_VX_META_FORMAT_H__
#define __GC_VX_META_FORMAT_H__

EXTERN_C_BEGIN

VX_INTERNAL_API void vxoMetaFormat_Release(vx_meta_format *pmeta);

VX_INTERNAL_API vx_meta_format vxoMetaFormat_Create(vx_context context);

EXTERN_C_END

#endif /* __GC_VX_META_FORMAT_H__ */

