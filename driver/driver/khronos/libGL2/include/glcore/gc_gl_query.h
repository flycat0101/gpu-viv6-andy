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


#ifndef __gc_gl_query_h_
#define __gc_gl_query_h_

#include "gc_gl_context.h"

/*
** Maximum linear table size for query object.
*/
#define __GL_MAX_QUERYOBJ_LINEAR_TABLE_SIZE       1024

/*
** Default linear table size for query object.
*/
#define __GL_DEFAULT_QUERYOBJ_LINEAR_TABLE_SIZE   256

/*
** Maximum hash table size for query. Must be 2^n.
*/
#define __GL_QUERY_HASH_TABLE_SIZE                  512


/* Status for query object */
#define __GL_QUERY_QUERYED     3
#define __GL_QUERY_END         2
#define __GL_QUERY_BEGIN       1
#define __GL_QUERY_CREATE      0

enum {
    __GL_QUERY_OCCLUSION  = 0,
    __GL_QUERY_TIME_ELAPSED,
    __GL_QUERY_LAST
};


typedef struct __GLqueryObjectRec
{
    GLenum  target;
    GLuint  name;
    GLint64 count;
    GLuint  status;
    GLboolean resultAvailable;

    /* This points to DP specific data */
    GLvoid  *privateData;

} __GLqueryObject;


typedef struct __GLqueryMachineRec
{
    /* query objects are not shared between contexts according to the Spec.
    ** we just use the __GLsharedObjectMachine to manage the query objects.
    */
    __GLsharedObjectMachine *noShare;
    __GLqueryObject *currQuery[__GL_QUERY_LAST];

} __GLqueryMachine;

#endif /* __gc_gl_query_h_ */

