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


#ifndef __chip_utils_h__
#define __chip_utils_h__

#ifdef __cplusplus
extern "C" {
#endif

/* A general object who may be used in chip layer. It is a wrapper
   of user data. This object can be used in any data structure, such
   as array, link, hash, etc */
typedef struct __GLchipUtilsObjectRec
{
    /* Pointer to user defined data */
    GLvoid                         *pUserData;

    /* A key uniquely indicating this object */
    GLuint                          key;

    /* How many references are this object being referenced currently?
       This is used to saftely delete object if object does not need to
       be maintained anymore */
    GLint                          refCount;

    /* Recent referenced year of this object for LRU evict */
    GLuint                          year;

    GLboolean                       perpetual;

    struct __GLchipUtilsObjectRec  *next;
} __GLchipUtilsObject;

typedef GLvoid (*__GLchipDeleteUserDataFunc)(__GLcontext *gc, GLvoid *pUserData);

/* Hash definition */
typedef struct __GLchipUtilsHashRec
{
    __GLchipUtilsObject           **ppHashTable;

    /* How many objects of each entry */
    GLuint                         *pEntryCounts;

    /* Hash table entry number must be power of 2 (32, 64, etc.), so that
       we can use a simple bitmask (entryNum-1) to get the hash entry */
    GLuint                          tbEntryNum;

    /* Max objects count that each hash table entry can hold. For memory
       footprint consideration, we don't hope too many objects are hold in
       each entry. */
    GLuint                          maxEntryObjs;

    GLuint                          year;

    __GLchipDeleteUserDataFunc      pfnDeleteUserData;
} __GLchipUtilsHash;

/* Convert a segment of data to CRC32 tag */
GLuint
gcChipUtilsEvaluateCRC32(
    GLvoid *pData,
    GLuint dataSizeInByte
    );


/* Object operations */
GLvoid
gcChipUtilsObjectAddRef(
    __GLchipUtilsObject *pObj
    );

GLvoid
gcChipUtilsObjectReleaseRef(
    __GLchipUtilsObject *pObj
    );

/* Hash operations */
__GLchipUtilsHash*
gcChipUtilsHashCreate(
    __GLcontext *gc,
    GLuint tbEntryNum,
    GLuint maxEntryObjs,
    __GLchipDeleteUserDataFunc pfnDeleteUserData
    );

GLvoid
gcChipUtilsHashDestory(
    __GLcontext *gc,
    __GLchipUtilsHash *pHash
    );

__GLchipUtilsObject*
gcChipUtilsHashAddObject(
    __GLcontext *gc,
    __GLchipUtilsHash *pHash,
    GLvoid *pUserData,
    GLuint key,
    GLboolean bPerpetual
    );

GLvoid
gcChipUtilsHashDeleteObject(
    __GLcontext *gc,
    __GLchipUtilsHash* pHash,
    __GLchipUtilsObject* pObj
    );

GLvoid
gcChipUtilsHashDeleteAllObjects(
    __GLcontext *gc,
    __GLchipUtilsHash *pHash
    );

__GLchipUtilsObject*
gcChipUtilsHashFindObjectByKey(
    __GLcontext *gc,
    __GLchipUtilsHash *pHash,
    GLuint key
    );

/* Debug util function */
gceSTATUS
gcChipUtilsDumpSurface(
    __GLcontext *gc,
    gcsSURF_VIEW *surfView,
    gctCONST_STRING fileName,
    GLboolean yInverted
);

gceSTATUS
gcChipUtilsVerifyRT(
    __GLcontext *gc
    );

gceSTATUS
gcChipUtilsVerifyImages(
    __GLcontext *gc
    );

#if gcdFRAMEINFO_STATISTIC
gceSTATUS
gcChipUtilsDumpTexture(
    __GLcontext *gc,
    __GLtextureObject *tex
    );

gceSTATUS
gcChipUtilsDumpRT(
    __GLcontext *gc,
    GLuint flag
    );

#endif

__GL_INLINE gctUINT
gcChipGetSurfOffset(
    gcsSURF_VIEW *surfView
)
{
    gctUINT offset = 0;

    if (surfView && surfView->surf)
    {
        gctINT32 sliceSize = 0;
        gcoSURF_GetInfo(surfView->surf, gcvSURF_INFO_SLICESIZE, &sliceSize);
        offset = surfView->firstSlice * (gctUINT)sliceSize;
    }

    return offset;
}

#ifdef __cplusplus
}
#endif

#endif /* __chip_utils_h__ */
