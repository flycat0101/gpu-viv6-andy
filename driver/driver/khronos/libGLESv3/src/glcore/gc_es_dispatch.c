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

/* ES 3.1 + Extension API Function Dispatch Table */

#define __gles(func) __gles_##func

__GLesDispatchTable __glesApiFuncDispatchTable = {
    __GLES_API_ENTRIES(__gles)
};

