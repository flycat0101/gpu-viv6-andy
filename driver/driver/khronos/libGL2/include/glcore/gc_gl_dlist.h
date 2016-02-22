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


#ifndef __gc_gl_dlist_h_
#define __gc_gl_dlist_h_

#define __GL_DLIST_CACHE_ENABLE                 1

/*
** Maximum linear table size for display lists.
**
*/
#define __GL_MAX_DLIST_LINEAR_TABLE_SIZE        60000

/*
** Default linear table size for display lists.
*/
#define __GL_DEFAULT_DLIST_LINEAR_TABLE_SIZE    1024

/*
** Maximum hash table size for display lists. Must be 2^n.
*/
#define __GL_MAX_DLIST_HASH_TABLE_SIZE          16384

/*
** Maximum vertex number in a single __glop_Primitive. Must be 3*n.
*/
#define __GL_MAX_DLIST_VERTEX_NUMBER            65529

/*
** Maximum concatenate list numbers.
*/
#define __GL_MAX_CONCAT_DLIST_COUNT             1000

/*
** Maximum concatenate Dlist cache table size. Must be 2^n.
*/
#define __GL_MAX_CONCAT_DLIST_CACHE_SIZE        4096
#define __GL_CONCAT_DLIST_CACHE_INDEX(name)  ((name) & (__GL_MAX_CONCAT_DLIST_CACHE_SIZE - 1))

/*
** Stub macros that can later be filled in to allow multi-threaded
** applications to share the same display list space.  This problem needs
** to be thought out carefully, because we don't really want to
** slow an application down to perform semaphore locking if it isn't
** sharing any display lists (maybe it isn't too bad, only a couple locks
** are required per glCallLists() call).
*/
#define __GL_DLIST_SEMAPHORE_LOCK()
#define __GL_DLIST_SEMAPHORE_UNLOCK()


/*
*** Data types are used by display list
*/
typedef struct __GLdlistRec __GLdlist;
typedef struct __GLdlistOpRec __GLdlistOp;
typedef const GLubyte *__GLlistExecFunc(const GLubyte *);

/*
** Implementation-specific function called to free display list
*/
typedef GLvoid (*__GLDlistFreeFn)(__GLcontext *gc, __GLdlist *dlist);


typedef struct __GLPrimBeginRec {
    GLfloat **primStartAddr;
    GLint *primVertCount;
    GLushort *indexBuffer;
    GLvoid *ibPrivateData;
    GLboolean *edgeflagBuffer;
    GLvoid *privateData;
    GLvoid *privStreamInfo;
    GLint indexCount;
    GLuint64 primElemSequence;
    GLuint64 primitiveFormat;
    GLuint primInputMask;
    GLuint primType;
    GLint elementCount;
    GLint vertexCount;
    GLint primCount;
    GLint totalStrideDW;
    GLint elemOffsetDW[__GL_TOTAL_VERTEX_ATTRIBUTES];
    GLint elemSizeDW[__GL_TOTAL_VERTEX_ATTRIBUTES];
} __GLPrimBegin;


/*
** A compiled, unoptimized dlist.  Conceptually a linked list of operations.
** An optimizer may work through the operations and delete, add, or change
** them.
**
** These are only stored transiently.  They are created, optimized, and
** converted into optimized dlists.
**
** This structure *MUST* be set up so that data is doubleword aligned!
*/

struct __GLdlistOpRec
{
    __GLdlistOp *next;          /* Linked list chain */

    /* This dlist free function is called when
    ** the entire dlist is freed.  It is passed
    ** a pointer to data.  It should *not* free
    ** data, but only any memory that has been
    ** allocated and is pointed to by the
    ** structure contained in data (which will
    ** be freed after this function returns).
    */
    GLvoid (*dlistFree)(__GLcontext *gc, GLubyte *);
    GLvoid (*dlistFreePrivateData)(__GLcontext *gc, GLubyte *);

    GLuint size;                /* Actual size of data */
    GLshort opcode;             /* Opcode for this operation */
    union
    {
        GLuint aligned;            /* Data require 64-bit aligned */
        GLuint primType;        /* Primitive type in glBegin */
    };
};

typedef struct __GLcompiledDlistRec
{
    GLint freeCount;              /* Number of free functions defined */
    __GLdlistOp *dlist;           /* The linked list of operations */
    __GLdlistOp *lastDlist;       /* For quick appends */
    __GLdlistOp *lastPrimNode;    /* For quick merge of primitive nodes */
} __GLcompiledDlist;

typedef struct __GLdlistNameNodeRec {
    struct __GLdlistNameNodeRec *next;
    GLuint name;
} __GLdlistNameNode;


enum {
    __GL_LIST_NOT_USED_IN_BATCH = 0,
    __GL_LIST_USED_IN_SINGLE_BATCH,
    __GL_LIST_USED_IN_MULTI_BATCHES
};

/*
** A fully optimized dlist. One of these is stored for every permanent dlist.
*/
struct __GLdlistRec
{
    GLuint name;
    GLint refcount; /* number of references; created with 1; delete when 0 */
    GLubyte *segment; /* points to first segment of display list */
    GLubyte *freefunc; /* free function array (variable size) used by deleteList */
    __GLdlistNameNode *parent;
    __GLdlistNameNode *child;
    __GLdlistNameNode *batchNameList; /* Link list for multiple concatenate list batch names */
    GLuint listBatchName;             /* Variable for single concatenate list batch name */
    GLint vertexCount;
    GLint primitiveCount;
    GLboolean concatenatable;
    GLubyte batchUsageFlag; /* flag to indicate list is referenced in multiple concat batches */
};

/*
*** Free local data function
*/
typedef struct __GLDlistFreeFnsRec
{
    GLvoid (*freeFn)(__GLcontext *gc, GLubyte *);
    /* This function is only used in mode change */
    GLvoid (*freePrivateDataFn)(__GLcontext *gc, GLubyte *);
    GLubyte *data;
} __GLDlistFreeFns;

typedef struct __GLDlistConcatDrawRec
{
    struct __GLDlistConcatDrawRec *next;
    __GLPrimBegin* primBegin;
    __GLdlist **concatDlistPtrs;
    GLint *concatVertexCount;
    GLint *concatIndexCount;
    GLuint listBatchName;
    GLint  concatListCount;
} __GLDlistConcatDraw;

typedef struct __GLdlistMachineRec
{
    __GLsharedObjectMachine *shared;

    /*
    ** If a list is being executed (glCallList or glCallLists) then this
    ** is the current nesting of calls.  It is constrained by the limit
    ** gc->constants.maxListNesting (this prevents infinite recursion).
    */
    GLuint nesting;

    /*
    ** GL_COMPILE or GL_COMPILE_AND_EXECUTE.
    */
    GLenum mode;

    /*
    ** List being compiled - 0 means none.
    */
    GLuint currentList;

    /*
    ** Data for the current list being compiled.
    */
    __GLcompiledDlist listData;

    /*
    ** Current display list.
    */
    __GLdlist *pCurrentDlist;

    /*
    ** For fast memory manipulation.  Check out soft/so_memmgr for details.
    ** Memory managers for dlist objects.  All dlist operations will use
    ** these to acquire the final block of memory which will store the display
    ** list.
    */
    __GLarena *arena;

    __GLdlist *concatDlistPtrs[__GL_MAX_CONCAT_DLIST_COUNT];
    GLint  concatVertexCount[__GL_MAX_CONCAT_DLIST_COUNT];
    GLint  concatIndexCount[__GL_MAX_CONCAT_DLIST_COUNT];

    __GLDlistConcatDraw *concatListCache[__GL_MAX_CONCAT_DLIST_CACHE_SIZE];
    __GLDlistConcatDraw *currentConcatDraw;
    /* the maximum index used in the concatListCache array */
    GLint maxConcatListCacheIdx;

    GLint  concatListCount;
    GLboolean enableConcatListCache;
    GLboolean origConcatListCacheFlag;

} __GLdlistMachine;


/*
** Create and destroy display list ops.  __glDlistAllocOp() sets an
** out of memory error before returning NULL if there is no memory left.
*/
extern __GLdlistOp *__glDlistAllocOp(__GLcontext *gc, GLuint size);

/*
** Append the given op to the currently under construction list.
*/
extern GLvoid __glDlistAppendOp(__GLcontext *gc, __GLdlistOp *newop);

extern GLvoid __glDisplayListBatchEnd(__GLcontext *gc);

#endif /* __gc_gl_dlist_h_ */
