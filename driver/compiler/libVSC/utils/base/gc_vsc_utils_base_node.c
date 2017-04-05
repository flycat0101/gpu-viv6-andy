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


#include "gc_vsc.h"

void vscBSNODE_Initialize(VSC_BASE_NODE* pBaseNode, void* pUserData)
{
    pBaseNode->pUserData = pUserData;
}

void* vscBSNODE_GetContainedUserData(VSC_BASE_NODE* pBaseNode)
{
    return pBaseNode->pUserData;
}

