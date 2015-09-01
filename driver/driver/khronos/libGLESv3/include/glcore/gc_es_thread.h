/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gl_es_thread_h__
#define __gl_es_thread_h__


typedef struct __GLthreadPrivRec
{
    void *p3DBlitState;
    void (*destroy3DBlitState)(void*);
} __GLthreadPriv;

#endif /* __gl_es_thread_h__ */
