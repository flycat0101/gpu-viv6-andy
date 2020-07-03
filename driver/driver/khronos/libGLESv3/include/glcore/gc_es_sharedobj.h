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


#ifndef __gl_es_sharedobj_h__
#define __gl_es_sharedobj_h__

/*
** Bit masks for "flag" variable in __GL*Object.
** Note: Bit 0~7 are reserved for generic object management.
*/
#define __GL_OBJECT_IS_DELETED                    (1 << 0)

typedef GLboolean (*__GLdeleteObjectFunc)(__GLcontext *gc, GLvoid *obj);

/*
** A linked list of named objects attached to each hashBuck
*/
typedef struct __GLobjItemRec
{
    struct __GLobjItemRec *next;
    GLuint name;
    GLvoid *obj;
} __GLobjItem;

/*
** A linked list of contiguous name allocation nodes
*/
typedef struct __GLnameAllocationRec
{
    struct __GLnameAllocationRec *next;
    GLuint start;
    GLuint number;
} __GLnameAllocation;

/*
** This is the generic data structure for all GL sharable objects
** (texture object, buffer object, shader program object, render objects etc.)
*/
typedef struct __GLsharedObjectMachineRec
{
    /* A linear lookup table for fast object name to object structure pointer lookup */
    GLvoid **linearTable;

    /* A pre-allocated array of pointers to __GLobjItem */
    __GLobjItem **hashBuckets;

    /* Increased unique id for each used object */
    GLuint uniqueId;

    /* Pointer to the linked list of name allocation nodes */
    __GLnameAllocation *nameArray;

    /* Number of GL contexts that are using this structure */
    GLint refcount;

    /* The size of current linear table */
    GLuint linearTableSize;
    GLuint maxLinearTableSize;

    /* For some objects, names immediately becomes invalid after deletion. */
    GLboolean immediateInvalid;

    /* The size of the hashBuckets must be power of 2 (1024, 2048, 4096, etc.)
    ** so that we can use a simple bitmask (hashSize-1) to get the hash ID.
    */
    GLuint hashSize;
    GLuint hashMask;

    /* pointer to VEGLLock for exclusively access */
    GLvoid *lock;

    /* Object specific destroy function */
    __GLdeleteObjectFunc deleteObject;
} __GLsharedObjectMachine;


typedef struct __GLimageUserRec
{
    GLvoid *imageUser;
    GLuint  refCount;
    struct __GLimageUserRec *next;
} __GLimageUser;

#ifdef __cplusplus
extern "C" {
#endif

extern GLint __glGenerateNames(__GLcontext *gc, __GLsharedObjectMachine *shared, GLsizei range);
extern GLboolean __glIsNameDefined(__GLcontext *gc, __GLsharedObjectMachine *shared, GLuint id);
extern GLvoid __glDeleteNamesFrList(__GLcontext *gc, __GLsharedObjectMachine *shared, GLuint id, GLsizei n);
extern __GLobjItem **__glLookupObjectItem(__GLcontext *gc, __GLsharedObjectMachine *shared, GLuint id);
extern __GLobjItem * __glFindObjItemNode(__GLcontext *gc, __GLsharedObjectMachine *shared, GLuint id);
extern GLvoid __glFreeSharedObjectState(__GLcontext *gc, __GLsharedObjectMachine *shared);
extern GLvoid __glCheckLinearTableSize(__GLcontext *gc, __GLsharedObjectMachine *shared, GLuint size);

#ifdef __cplusplus
}
#endif

#endif /* __gl_es_sharedobj_h__ */
