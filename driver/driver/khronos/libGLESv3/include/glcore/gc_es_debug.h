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


#ifndef __gl_es_debug_h__
#define __gl_es_debug_h__


typedef enum
{
    __GL_DEBUG_SRC_API = 0,
    __GL_DEBUG_SRC_WIN,
    __GL_DEBUG_SRC_COMPILER,
    __GL_DEBUG_SRC_3RDPARTY,
    __GL_DEBUG_SRC_APP,
    __GL_DEBUG_SRC_OTHER,

    __GL_DEBUG_SRC_NUM
} __GLdbgSrc;

typedef enum
{
    __GL_DEBUG_TYPE_ERROR = 0,
    __GL_DEBUG_TYPE_DEPRECATED,
    __GL_DEBUG_TYPE_UNDEFINED,
    __GL_DEBUG_TYPE_PORTABILITY,
    __GL_DEBUG_TYPE_PERFORMANCE,
    __GL_DEBUG_TYPE_OTHER,
    __GL_DEBUG_TYPE_MARKER,
    __GL_DEBUG_TYPE_PUSH,
    __GL_DEBUG_TYPE_POP,

    __GL_DEBUG_TYPE_NUM
} __GLdbgType;

typedef enum
{
    __GL_DEBUG_SEVERITY_HIGH = 0,
    __GL_DEBUG_SEVERITY_MEDIUM,
    __GL_DEBUG_SEVERITY_LOW,
    __GL_DEBUG_SEVERITY_NOTICE,

    __GL_DEBUG_SEVERITY_NUM
} __GLdbgSeverity;

typedef struct __GLdbgMsgLogRec
{
    GLenum src;
    GLenum type;
    GLenum severity;
    GLuint id;

    GLchar* message;
    GLsizei length; /* message length including null-terminator */

    struct __GLdbgMsgLogRec *next;
} __GLdbgMsgLog;


typedef struct __GLdbgMsgCtrlRec
{
    GLenum src;
    GLenum type;
    GLuint id;

    GLboolean enables[__GL_DEBUG_SEVERITY_NUM];

    struct __GLdbgMsgCtrlRec *next;
} __GLdbgMsgCtrl;


typedef struct __GLdbgSpaceCtrlRec
{
    GLboolean enables[__GL_DEBUG_SEVERITY_NUM];
    __GLdbgMsgCtrl *msgs; /* known message ctrl list */
} __GLdbgSpaceCtrl;


typedef struct __GLdbgGroupCtrlRec
{
    __GLdbgSpaceCtrl spaces[__GL_DEBUG_SRC_NUM][__GL_DEBUG_TYPE_NUM];

    /* Record source/id/message when PushGroup */
    GLenum source;
    GLuint id;
    GLchar *message;
} __GLdbgGroupCtrl;

typedef struct __GLdebugMachineRec
{
    GLint               maxStackDepth;
    GLsizei             maxMsgLen;
    GLuint              maxLogMsgs;

    GLboolean           dbgOut;
    GLboolean           dbgOutSync;

    GLDEBUGPROCKHR      callback;
    const GLvoid*       userParam;

    GLint               current;    /* current position in the stack */
    __GLdbgGroupCtrl  **msgCtrlStack;

    GLuint              loggedMsgs;
    __GLdbgMsgLog      *msgLogHead;
    __GLdbgMsgLog      *msgLogTail;
} __GLdebugMachine;

extern GLvoid __glDebugPrintLogMessage(__GLcontext *gc, GLenum source, GLenum type, GLuint id,
                                       GLenum severity, const GLchar *format, ...);

#endif /*__gl_es_debug_h__*/
