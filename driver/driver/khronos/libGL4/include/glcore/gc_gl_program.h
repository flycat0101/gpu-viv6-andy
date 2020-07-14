/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_gl_program_h_
#define __gc_gl_program_h_

#include "gc_gl_consts.h"
#include "gc_gl_sharedobj.h"

/*
** Maximum linear table size for program object.
*/
#define __GL_MAX_PRGOBJ_LINEAR_TABLE_SIZE       1024

/*
** Default linear table size for program object.
*/
#define __GL_DEFAULT_PRGOBJ_LINEAR_TABLE_SIZE   256

/*
** Maximum hash table size for program object. Must be 2^n.
*/
#define __GL_PRGOBJ_HASH_TABLE_SIZE             512

/*
**
*/
#define __GL_NUMBER_OF_PROGRAM_TARGET   2
#define __GL_VERTEX_PROGRAM_INDEX   0
#define __GL_FRAGMENT_PROGRAM_INDEX 1
#define __GL_PROGRAM_CHECK_TARGET(target,targetIndex)     \
    switch(target) {                                      \
        case GL_VERTEX_PROGRAM_ARB:                       \
            targetIndex = __GL_VERTEX_PROGRAM_INDEX;      \
            break;                                        \
        case GL_FRAGMENT_PROGRAM_ARB:                     \
            targetIndex = __GL_FRAGMENT_PROGRAM_INDEX;    \
            break;                                        \
        default:                                          \
            __glSetError(GL_INVALID_ENUM);                \
            return;                                       \
    }

#define __GL_PROGRAM_CHECK_TARGET_INDEX(target,index,envLocal,targetIndex)\
    switch(target) {                                                      \
        case GL_VERTEX_PROGRAM_ARB:                                       \
            targetIndex = __GL_VERTEX_PROGRAM_INDEX;                      \
            break;                                                        \
        case GL_FRAGMENT_PROGRAM_ARB:                                     \
            targetIndex = __GL_FRAGMENT_PROGRAM_INDEX;                    \
            break;                                                        \
        default:                                                          \
            __glSetError(GL_INVALID_ENUM);                                \
            return;                                                       \
    }                                                                     \
    if (index > gc->constants.max##envLocal##Parameter[targetIndex])      \
    {                                                                     \
        __glSetError(GL_INVALID_VALUE);                                   \
        return;                                                           \
    }


/* To transfer the result from compiler to glcore */
typedef struct __GLProgramStringResultRec {
    GLbyte              errStr[__GL_MAX_PROGRAM_ERRORSTR_LENGTH];
    GLuint              errorNo;
    GLint  errorPos;

    /* vertex program */
    GLuint inputMask;   /* vertex attributes used by this program */
    GLuint texCoorDimension; /* The texture coordinates outputed. */

    /* fragment program */
    GLuint enabledDimension[__GL_MAX_TEXTURE_UNITS]; /* Texture samplers used by this program */

    GLuint  nativeLimit;
    GLuint  nativeInstruction;
    GLuint  nativeTemporaries;
    GLuint  nativeParameters;
    GLuint  nativeAttribs;
    GLuint  nativeAddrRegisters;
}__GLProgramStringResult;


typedef struct __GLProgramObjectRec {
    /* indicate how many targets the object is currently bound to
    */
    GLuint bindCount;

      /* The seqNumber is increased by 1 whenever program string is changed.
      ** DP must recompile the program string if its internal copy of
      ** savedSeqNum is different from this seqNumber.
      */
      GLuint seqNumber;

    /* Internal flag for generic object management. */
    GLbitfield flag;

      /* The locParamSeqNum is increased by 1 whenever local parameters are changed. */
      GLuint locParamSeqNum;

    /* This is the object privateData that can be shared by different contexts
    ** that are bound to the object.
    */
    GLvoid *privateData;

    GLuint          name;
    GLuint          format;
    GLuint          target;
    GLuint          targetIndex;
    GLuint          flags;
    GLubyte         *programString;
    GLuint          programLen;
    __GLcoord       localParameter[__GL_MAX_PROGRAM_LOCAL_PARAMETERS];
    GLuint          programIndex;
    GLuint          bTexBindCopied;

    __GLProgramStringResult compiledResult;

} __GLProgramObject;

 /*Program object flags:*/
#define __GL_PRGOBJFLAG_COMPILED        0x00000001
#define __GL_PRGOBJFLAG_BOUND           0x00000002
#define __GL_PRGOBJFLAG_VALID           0x00000004

typedef struct __GLProgramMachineRec {
    __GLsharedObjectMachine *shared;

    __GLProgramObject *currentProgram[__GL_NUMBER_OF_PROGRAM_TARGET];

    /* dirty bits for env parameters */
    __GLcoord         envParameter[__GL_NUMBER_OF_PROGRAM_TARGET][__GL_MAX_PROGRAM_ENV_PARAMETERS];
    GLbitfield        envDirty[__GL_NUMBER_OF_PROGRAM_TARGET][__GL_MAX_PROGRAM_ENV_PARAMETERS/(sizeof(GLbitfield) * 8)];

    __GLcoord         localParameter[__GL_NUMBER_OF_PROGRAM_TARGET][__GL_MAX_PROGRAM_LOCAL_PARAMETERS];
    GLbitfield        localDirty[__GL_NUMBER_OF_PROGRAM_TARGET][__GL_MAX_PROGRAM_LOCAL_PARAMETERS/(sizeof(GLbitfield) * 8)];
    GLuint            locParamSeqNum[__GL_NUMBER_OF_PROGRAM_TARGET];

    /* dirty bits for program matrix */
    GLbitfield        programMatrix;

    __GLProgramObject defaultProgram[__GL_NUMBER_OF_PROGRAM_TARGET];
    GLubyte           errStr[__GL_MAX_PROGRAM_ERRORSTR_LENGTH];
    GLint             errorPos;
    GLint             errorNo;

    GLboolean         realEnabled[__GL_NUMBER_OF_PROGRAM_TARGET];
} __GLProgramMachine;

#define __GL_PROGRAM_SET_PROGRAMMATRIX_DIRTY(gc,index) \
    gc->globalDirtyState[__GL_PROGRAM_ATTRS] |= (__GL_DIRTY_PROGRAM_MATRIX);\
    gc->globalDirtyState[__GL_ALL_ATTRS] |= (1 << __GL_PROGRAM_ATTRS);\
    gc->program.programMatrix |= (index);

#endif   /* __gc_gl_program_h_ */
