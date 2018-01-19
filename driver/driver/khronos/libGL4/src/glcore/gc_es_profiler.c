/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_es_context.h"


#define _GC_OBJ_ZONE __GLES3_ZONE_CORE

#if VIVANTE_PROFILER

gctBOOL
__glProfiler(
    IN gctPOINTER Profiler,
    IN gctUINT32 Enum,
    IN gctHANDLE Value
    )
{
    __GLcontext *gc = __glGetGLcontext();
    GL_ASSERT(gc);

    if(gcvNULL == gc)
    {
        gcmFATAL("Get context failed");
        return GL_FALSE;
    }

    return (*gc->dp.profiler)(Profiler, Enum, Value);
}


#endif
