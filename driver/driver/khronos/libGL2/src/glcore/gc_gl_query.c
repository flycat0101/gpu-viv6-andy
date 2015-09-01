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


#include "gc_gl_context.h"
#include "gc_gl_names_inline.c"
#include "gc_gl_debug.h"
#include "dri/viv_lock.h"

GLvoid APIENTRY __glim_GenQueries(GLsizei n, GLuint *ids)
{
    GLint start, i;

    __GL_SETUP_NOT_IN_BEGIN();
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GenQueries", DT_GLsizei, n, DT_GLuint_ptr, ids, DT_GLnull);
#endif

    if (NULL == ids)
        return;

    if (n < 0)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if (gc->query.currQuery[__GL_QUERY_OCCLUSION] ||
        gc->query.currQuery[__GL_QUERY_TIME_ELAPSED])
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    start = __glGenerateNames(gc, gc->query.noShare, n);

    for (i = 0; i < n; i++) {
        ids[i] = start + i;
    }
}

GLvoid APIENTRY __glim_DeleteQueries(GLsizei n, const GLuint *ids)
{
    GLint i;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_DeleteQueries", DT_GLsizei, n, DT_GLuint_ptr, ids, DT_GLnull);
#endif

    if (gc->query.currQuery[__GL_QUERY_OCCLUSION] ||
        gc->query.currQuery[__GL_QUERY_TIME_ELAPSED])
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    for (i = 0; i < n; i++) {
        __glDeleteNamesFrList(gc, gc->query.noShare, ids[i], 1);
        __glDeleteObject(gc, gc->query.noShare, ids[i]);
    }
}

GLboolean APIENTRY __glim_IsQuery(GLuint id)
{
    GLboolean retVal;
    __GL_SETUP_NOT_IN_BEGIN_RET(0);

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_IsQuery", DT_GLuint, id, DT_GLnull);
#endif

    retVal = __glIsNameDefined(gc, gc->query.noShare, id);

    return retVal;
}

GLvoid APIENTRY __glim_BeginQuery(GLenum target, GLuint id)
{
    __GLqueryObject *queryObj;

    GLuint targetIndex;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_BeginQuery", DT_GLenum, target, DT_GLuint, id, DT_GLnull);
#endif

    switch (target) {
      case GL_SAMPLES_PASSED:
        targetIndex = __GL_QUERY_OCCLUSION;
        break;
      case GL_TIME_ELAPSED_EXT:
        if (!__glExtension[INDEX_EXT_timer_query].bEnabled)
        {
            __glSetError(GL_INVALID_ENUM);
            return;
        }
        targetIndex = __GL_QUERY_TIME_ELAPSED;
        break;
      default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    /* another query is already in progress with the same target. */
    if (gc->query.currQuery[targetIndex]) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    /* where id is the name of a query currently in progress */
    if ((gc->query.currQuery[__GL_QUERY_OCCLUSION] &&
        (gc->query.currQuery[__GL_QUERY_OCCLUSION]->name == id)) ||
        (gc->query.currQuery[__GL_QUERY_TIME_ELAPSED] &&
        (gc->query.currQuery[__GL_QUERY_TIME_ELAPSED]->name == id)))
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /*
    ** Retrieve the __GLqueryObject from the "gc->query.noShare" structure.
    */
    queryObj = (__GLqueryObject *)__glGetObject(gc, gc->query.noShare, id);

    if (queryObj == NULL) {
        /*
        ** If this is the first time this name has been bound,
        ** then create a new texture object and initialize it.
        */
        queryObj = (__GLqueryObject *)(*gc->imports.calloc)(gc, 1, sizeof(__GLqueryObject) );
        if (queryObj == NULL) {
            __glSetError(GL_OUT_OF_MEMORY);
            return;
        }

        queryObj->name = id;

        /* Add this __GLoccluQueryObject to the "gc->occluQuery.noShare" structure. */
        __glAddObject(gc, gc->query.noShare, id, queryObj);

        /* Mark the name "id" used in the nameArray. */
        __glMarkNameUsed(gc, gc->query.noShare, id);
    }

    queryObj->target = target;
    queryObj->count = 0;
    queryObj->resultAvailable = GL_FALSE;
    queryObj->status = __GL_QUERY_BEGIN;
    gc->query.currQuery[targetIndex] = queryObj;

    /* Notify DP to begin query on the query object */
    (*gc->dp.beginQuery)(gc, queryObj);
}

GLvoid APIENTRY __glim_EndQuery(GLenum target)
{
    __GLqueryObject *queryObj;

    GLuint targetIndex;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_EndQuery", DT_GLenum, target, DT_GLnull);
#endif

    switch (target) {
      case GL_SAMPLES_PASSED:
        targetIndex = __GL_QUERY_OCCLUSION;
        break;

      case GL_TIME_ELAPSED_EXT:
        if (!__glExtension[INDEX_EXT_timer_query].bEnabled)
        {
            __glSetError(GL_INVALID_ENUM);
            return;
        }
        targetIndex = __GL_QUERY_TIME_ELAPSED;
        break;
      default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    /* If there is no active query */
    if (gc->query.currQuery[targetIndex] == NULL) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    queryObj = gc->query.currQuery[targetIndex];
    if (queryObj->status != __GL_QUERY_BEGIN) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    LINUX_LOCK_FRAMEBUFFER(gc);
    /* Notify DP to end query on the query object */
    (*gc->dp.endQuery)(gc, queryObj);
    LINUX_UNLOCK_FRAMEBUFFER(gc);
    queryObj->status = __GL_QUERY_END;
    gc->query.currQuery[targetIndex] = NULL;
}

#if GL_NV_occlusion_query
GLvoid APIENTRY __glim_BeginQueryNV(GLuint id)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_BeginQueryNV", DT_GLenum, GL_SAMPLES_PASSED, DT_GLuint, id, DT_GLnull);
#endif
    __glim_BeginQuery(GL_SAMPLES_PASSED,id);

}

GLvoid APIENTRY __glim_EndQueryNV(GLvoid)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_EndQueryNV", DT_GLenum, GL_SAMPLES_PASSED, DT_GLnull);
#endif
    __glim_EndQuery(GL_SAMPLES_PASSED);
}
#endif /* GL_NV_occlusion_query */

GLvoid APIENTRY __glim_GetQueryiv(GLenum target, GLenum pname, GLint *params)
{
    __GLqueryObject *queryObj;

    GLuint targetIndex ;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetQueryiv" ,DT_GLenum, target, DT_GLenum, pname, DT_GLint_ptr, params, DT_GLnull);
#endif

    switch (target) {
      case GL_SAMPLES_PASSED:
        targetIndex = __GL_QUERY_OCCLUSION;
        break;
      case GL_TIME_ELAPSED_EXT:
        if (!__glExtension[INDEX_EXT_timer_query].bEnabled)
        {
            __glSetError(GL_INVALID_ENUM);
            return;
        }
        targetIndex = __GL_QUERY_TIME_ELAPSED;
        break;
      default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    queryObj = gc->query.currQuery[targetIndex];

    switch (pname) {
      case GL_QUERY_COUNTER_BITS:
        *params = gc->constants.numberofQueryCounterBits;
        return;
      case GL_CURRENT_QUERY:
        if (queryObj && queryObj->status == __GL_QUERY_BEGIN) {
            *params = queryObj->name;
        }
        else {
            *params = 0;
        }
        return;
      default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}

GLvoid __glGetQueryObjecti64v(GLuint id, GLenum pname, GLint64* params)
{
    __GLqueryObject *queryObj;

    __GL_SETUP_NOT_IN_BEGIN();

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /*
    ** Retrieve the __GLqueryObject from the "gc->query.noShare" structure.
    */
    queryObj = (__GLqueryObject *)__glGetObject(gc, gc->query.noShare, id);
    if (queryObj == NULL || queryObj->status == __GL_QUERY_BEGIN) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    switch(pname) {
      case GL_QUERY_RESULT:
          LINUX_LOCK_FRAMEBUFFER(gc);
          while (!queryObj->resultAvailable)
          {
              /* Query DP for the results of the query object */
              (*gc->dp.getQueryObject)(gc, pname, queryObj);
          }
          LINUX_UNLOCK_FRAMEBUFFER(gc);
          *params = queryObj->count;
          break;

      case GL_QUERY_RESULT_AVAILABLE:
          LINUX_LOCK_FRAMEBUFFER(gc);
          if (!queryObj->resultAvailable)
          {
              /* Query DP for the results of the query object */
              (*gc->dp.getQueryObject)(gc, pname, queryObj);
          }
          LINUX_UNLOCK_FRAMEBUFFER(gc);
          *params = queryObj->resultAvailable;
          break;

      default:
          __glSetError(GL_INVALID_VALUE);
          return;
    }
}

GLvoid APIENTRY __glim_GetQueryObjectiv(GLuint id, GLenum pname, GLint *params)
{
    GLint64 result = 0;

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetQueryObjectiv", DT_GLuint, id, DT_GLenum, pname, DT_GLint_ptr, params, DT_GLnull);
#endif

    __glGetQueryObjecti64v(id, pname, &result);

    /* make sure result can convert from INT64 to INT safely */
    GL_ASSERT( (result >= (-__glMaxInt-1) ) && (result <= __glMaxInt) );

    *params = (GLint)result;
}

GLvoid APIENTRY __glim_GetQueryObjectuiv(GLuint id, GLenum pname, GLuint *params)
{
    GLint64 result = 0;

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetQueryObjectuiv" ,DT_GLuint, id, DT_GLenum, pname, DT_GLuint_ptr, params, DT_GLnull);
#endif

    __glGetQueryObjecti64v(id, pname, &result);

    /* make sure result can convert from INT64 to UINT safely */
    GL_ASSERT( (result >= 0) && (result <= __glMaxUint) );

    *params = (GLuint)result;
}

GLvoid APIENTRY __glim_GetQueryObjecti64vEXT(GLuint id, GLenum pname, GLint64EXT * params)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetQueryObjectuiv" ,DT_GLuint, id, DT_GLenum, pname, DT_GLint_ptr, params, DT_GLnull);
#endif

    __glGetQueryObjecti64v(id, pname, (GLint64 *)params);
}

GLvoid APIENTRY __glim_GetQueryObjectui64vEXT(GLuint id, GLenum pname, GLuint64EXT * params)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetQueryObjectuiv" ,DT_GLuint, id, DT_GLenum, pname, DT_GLuint_ptr, params, DT_GLnull);
#endif

    __glGetQueryObjecti64v(id, pname, (GLint64 *)params);
}

GLboolean __glDeleteQueryObj(__GLcontext *gc, GLvoid *obj)
{
    __GLqueryObject *queryObj = obj;

    (*gc->dp.deleteQuery)(gc, queryObj);

    /* Delete the __GLqueryObject structure */
    (*gc->imports.free)(gc, queryObj);

    return GL_TRUE;
}

GLvoid __glInitQueryState(__GLcontext *gc)
{
    if (gc->query.noShare == NULL) {
        gc->query.noShare = (__GLsharedObjectMachine *)
            (*gc->imports.calloc)(gc, 1, sizeof(__GLsharedObjectMachine) );

        /* Initialize a linear lookup table for query object */
        gc->query.noShare->maxLinearTableSize = __GL_MAX_QUERYOBJ_LINEAR_TABLE_SIZE;
        gc->query.noShare->linearTableSize = __GL_DEFAULT_QUERYOBJ_LINEAR_TABLE_SIZE;
        gc->query.noShare->linearTable = (GLvoid **)
            (*gc->imports.calloc)(gc, 1, gc->query.noShare->linearTableSize * sizeof(GLvoid *) );

        gc->query.noShare->hashSize = __GL_QUERY_HASH_TABLE_SIZE;
        gc->query.noShare->hashMask = __GL_QUERY_HASH_TABLE_SIZE - 1;
        gc->query.noShare->refcount = 1;
        gc->query.noShare->deleteObject = __glDeleteQueryObj;
    }
}

GLvoid __glFreeQueryState(__GLcontext *gc)
{
    /* Free shared query object table */
    __glFreeSharedObjectState(gc, gc->query.noShare);
}
