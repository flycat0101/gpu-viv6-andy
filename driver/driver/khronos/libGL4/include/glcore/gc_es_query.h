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


#ifndef __gc_es_query_h__
#define __gc_es_query_h__

/*
** Maximum linear table size for Query object.
*/
#define __GL_MAX_QUERYOBJ_LINEAR_TABLE_SIZE         1024
#define __GL_DEFAULT_QUERYOBJ_LINEAR_TABLE_SIZE     256
#define __GL_QUERY_HASH_TABLE_SIZE                  512


/* Status for query object */
#define __GL_QUERY_QUERYED     3
#define __GL_QUERY_END         2
#define __GL_QUERY_BEGIN       1
#define __GL_QUERY_CREATE      0

enum
{
    __GL_QUERY_ANY_SAMPLES_PASSED = 0,
    __GL_QUERY_ANY_SAMPLES_PASSED_CONSERVATIVE,
    __GL_QUERY_XFB_PRIMITIVES_WRITTEN,
    __GL_QUERY_PRIMITIVES_GENERATED,
#ifdef OPENGL40
    __GL_QUERY_OCCLUSION,
    __GL_QUERY_TIME_ELAPSED,
#endif
    __GL_QUERY_LAST
};

typedef struct __GLqueryObjectRec
{
    GLenum    target;
    GLuint    name;
    GLint64   count;
    GLboolean active;
    GLboolean resultAvailable;

    /* Internal flag for generic object management  */
    GLbitfield flag;

    /* This points to DP specific data */
    GLvoid  *privateData;

    GLchar *label;
} __GLqueryObject;


typedef struct __GLqueryMachineRec
{
    /* Query objects are not shared between contexts according to the Spec.
    ** we just use the __GLsharedObjectMachine to manage the query objects.
    */
    __GLsharedObjectMachine *noShare;
    __GLqueryObject *currQuery[__GL_QUERY_LAST];

} __GLqueryMachine;


/*
** Sync Object
*/

#define __GL_MAX_SYNCOBJ_LINEAR_TABLE_SIZE          1024
#define __GL_DEFAULT_SYNCOBJ_LINEAR_TABLE_SIZE      256
#define __GL_SYNC_HASH_TABLE_SIZE                   512

typedef struct __GLsyncObjectRec
{
    GLuint name;
    GLint type;
    GLint status;
    GLint condition;
    GLbitfield flags;

    /* Number of clients waitings */
    GLuint waitCount;

    /* Internal flag for generic object management. */
    GLbitfield objFlag;

    GLvoid  *privateData;

    GLchar *label;
} __GLsyncObject;

typedef struct __GLsyncMachineRec
{
    /* Sync objects can be shared between contexts according to the Spec.
    */
    __GLsharedObjectMachine *shared;

} __GLsyncMachine;

#endif /* __gc_es_query_h__ */
