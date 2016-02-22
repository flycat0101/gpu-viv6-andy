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


#ifndef __gl_sharedobj_h_
#define __gl_sharedobj_h_

/* Bit masks for "flag" variable in __GL*Object.
** Note: Bit 0~7 are reserved for generic object management.
*/
#define __GL_OBJECT_IS_DELETED                    (1 << 0)


typedef GLvoid (* CleanupFunc)(__GLcontext *, GLvoid *);


/*
** A linked list of named objects attached to each hashBuck
*/
typedef struct __GLobjItemRec {
    struct __GLobjItemRec *next;
    GLuint name;
    GLvoid *obj;
} __GLobjItem;

/*
** A linked list of contiguous name allocation nodes
*/
typedef struct __GLnameAllocationRec {
    struct __GLnameAllocationRec *next;
    GLuint start;
    GLuint number;
} __GLnameAllocation;

/*
** This is the generic data structure for all GL sharable objects
** (display list, texture object, buffer object, etc.)
*/
typedef struct __GLsharedObjectMachineRec {

    /* A linear lookup table for fast object name to object structure pointer lookup */
    GLvoid **linearTable;

    /* A pre-allocated array of pointers to __GLobjItem */
    __GLobjItem **hashBuckets;

    /* Pointer to the linked list of name allocation nodes */
    __GLnameAllocation *nameArray;

    /* Number of GL contexts that are using this structure */
    GLint refcount;

    /* The size of current linear table */
    GLuint linearTableSize;
    GLuint maxLinearTableSize;

    /* The size of the hashBuckets must be power of 2 (1024, 2048, 4096, etc.)
    ** so that we can use a simple bitmask (hashSize-1) to get the hash ID.
    */
    GLuint hashSize;
    GLuint hashMask;

    /* Object specific destroy function */
    GLboolean (*deleteObject)(__GLcontext *gc, GLvoid *obj);

} __GLsharedObjectMachine;

/*
** A linked list of  container objects that share an object.
**Exp: list of FBO a renderbuffer object attached to.
*/
typedef struct __GLimageUserRec {
    /* Pointer to a container object that this object attached with.
    ** exp: a Framebuffer object a renderbuffer image attached to.
    */
    GLvoid *imageUser;
    CleanupFunc cleanUp;
    struct __GLimageUserRec *next;
} __GLimageUser;

#ifdef __cplusplus
extern "C" {
#endif

extern GLuint __glGenerateNames(__GLcontext *gc, __GLsharedObjectMachine *shared, GLsizei range);
extern GLboolean __glIsNameDefined(__GLcontext *gc, __GLsharedObjectMachine *shared, GLuint id);
extern GLvoid __glDeleteNamesFrList(__GLcontext *gc, __GLsharedObjectMachine *shared, GLuint id, GLsizei n);
extern __GLobjItem **__glLookupObjectItem(__GLcontext *gc, __GLsharedObjectMachine *shared, GLuint id);
extern __GLobjItem * __glFindObjItemNode(__GLcontext *gc, __GLsharedObjectMachine *shared, GLuint id);
extern GLvoid __glFreeSharedObjectState(__GLcontext *gc, __GLsharedObjectMachine *shared);
extern GLvoid __glCheckLinearTableSize(__GLcontext *gc, __GLsharedObjectMachine *shared, GLuint size);

#ifdef __cplusplus
}
#endif

#endif /* __gl_sharedobj_h_ */
