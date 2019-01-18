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


#ifndef __gl_asmoff_h_
#define __gl_asmoff_h_

#ifndef offsetof
#define offsetof(s,m)   (size_t)( (GLint64)&(((s *)0)->m) )
#endif

enum
{
  //  __GL_CURRENT_DISPATCH_OFFSET    = (offsetof(__GLcontext,currentDispatchOffset)),
  //  __GL_IMMEDIATE_TABLE_OFFSET     = (offsetof(__GLcontext,immediateDispatchTable) + sizeof(GLvoid *)),
  //  __GL_LISTCOMPILE_TABLE_OFFSET   = (offsetof(__GLcontext,listCompileDispatchTable) + sizeof(GLvoid *)),
    __GL_INPUT_CURRENT_PRIMMODE     = (offsetof(__GLcontext,input.currentPrimMode)),
    __GL_INPUT_BEGINMODE            = (offsetof(__GLcontext,input.beginMode))
};

#endif /*__gl_asmoff_h_ */
