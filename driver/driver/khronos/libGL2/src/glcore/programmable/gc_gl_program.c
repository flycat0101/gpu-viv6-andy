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


#include "gc_gl_context.h"
#include "../gc_gl_names_inline.c"
#include "gc_gl_debug.h"

/*****************************************************************
*              Program env and local parameters functions.
*/
#define __GL_PROGRAM_SET_ENV_DIRTY(target,index)\
    gc->globalDirtyState[__GL_PROGRAM_ATTRS] |= (__GL_DIRTY_VERTEX_PROGRAM_ENV << (target));\
    gc->globalDirtyState[__GL_ALL_ATTRS] |= (1 << __GL_PROGRAM_ATTRS);\
    gc->program.envDirty[target][(index) >> 5] |= (1 << ((index) &0x0000001F))

#define __GL_PROGRAM_SET_LOCAL_DIRTY(target,index)\
    gc->globalDirtyState[__GL_PROGRAM_ATTRS] |= (__GL_DIRTY_VERTEX_PROGRAM_LOCAL << (target));\
    gc->globalDirtyState[__GL_ALL_ATTRS] |= (1 << __GL_PROGRAM_ATTRS);\
    gc->program.localDirty[target][(index) >> 5] |= (1 << ((index) &0x0000001F))

__GL_INLINE GLvoid __glProgramEnvParameter4fv(
    __GLcontext *gc,
    GLenum target,
    GLuint index,
    __GLcoord *params)
{
    __GLcoord   *envParams;

    __GL_PROGRAM_CHECK_TARGET_INDEX(target,index,Env,target);

    envParams = &(gc->program.envParameter[target][index]);
    if(__GL_MEMCMP(envParams, params, sizeof(__GLcoord)))
    {
        __GL_VERTEX_BUFFER_FLUSH(gc);

        __GL_MEMCOPY(envParams, params, sizeof(__GLcoord));

        __GL_PROGRAM_SET_ENV_DIRTY(target,index);
    }
}

GLvoid APIENTRY __glim_ProgramEnvParameter4dARB(GLenum target, GLuint index,
                                                GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    __GLcoord params;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ProgramEnvParameter4dARB", DT_GLenum, target, DT_GLuint, index,
        DT_GLdouble, x, DT_GLdouble, y, DT_GLdouble, z, DT_GLdouble, w, DT_GLnull);
#endif

    params.x = (GLfloat)x;
    params.y = (GLfloat)y;
    params.z = (GLfloat)z;
    params.w = (GLfloat)w;

    __glProgramEnvParameter4fv(gc,target,index,&params);
}

GLvoid APIENTRY __glim_ProgramEnvParameter4dvARB(GLenum target, GLuint index,
                                               const GLdouble *params)
{
    __GLcoord params_f;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ProgramEnvParameter4dvARB", DT_GLenum, target, DT_GLuint, index,
        DT_GLdouble_ptr, params, DT_GLnull);
#endif

    params_f.x = (GLfloat)*params++;
    params_f.y = (GLfloat)*params++;
    params_f.z = (GLfloat)*params++;
    params_f.w = (GLfloat)*params;

    __glProgramEnvParameter4fv(gc,target,index,&params_f);
}

GLvoid APIENTRY __glim_ProgramEnvParameter4fARB(GLenum target, GLuint index,
                                              GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GLcoord params;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ProgramEnvParameter4fARB", DT_GLenum, target, DT_GLuint, index,
        DT_GLfloat, x, DT_GLfloat, y, DT_GLfloat, z, DT_GLfloat, w, DT_GLnull);
#endif

    params.x = (GLfloat)x;
    params.y = (GLfloat)y;
    params.z = (GLfloat)z;
    params.w = (GLfloat)w;

    __glProgramEnvParameter4fv(gc,target,index,&params);
}

GLvoid APIENTRY __glim_ProgramEnvParameter4fvARB(GLenum target, GLuint index,
                                               const GLfloat *params)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ProgramEnvParameter4fvARB", DT_GLenum, target, DT_GLuint, index,
        DT_GLfloat_ptr, params, DT_GLnull);
#endif


    __glProgramEnvParameter4fv(gc,target,index,(__GLcoord *)params);
}

__GL_INLINE GLvoid __glProgramLocalParameter4fv(
    __GLcontext *gc,
    GLenum target,
    GLuint index,
    __GLcoord *params)
{
    __GLProgramObject   *proObj;
    __GLcoord           *localParams;

    __GL_PROGRAM_CHECK_TARGET_INDEX(target,index,Local,target);

    localParams = &(gc->program.localParameter[target][index]);
    if(__GL_MEMCMP(localParams, params, sizeof(__GLcoord)))
    {
        __GL_VERTEX_BUFFER_FLUSH(gc);

        /* Update gc->program.localParameter */
        __GL_MEMCOPY(localParams, params, sizeof(__GLcoord));

        /* Copy gc->program.localParameter to program object localParameter */
        proObj = gc->program.currentProgram[target];
        proObj->localParameter[index] = *params;

        /* Increment local parameter sequence number */
        gc->program.locParamSeqNum[target]++;
        proObj->locParamSeqNum++;

        __GL_PROGRAM_SET_LOCAL_DIRTY(target,index);
    }
}

GLvoid APIENTRY __glim_ProgramLocalParameter4dARB(GLenum target, GLuint index,
                                                GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    __GLcoord params;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ProgramLocalParameter4dARB", DT_GLenum, target, DT_GLuint, index,
        DT_GLdouble, x, DT_GLdouble, y, DT_GLdouble, z, DT_GLdouble, w, DT_GLnull);
#endif

    params.x = (GLfloat)x;
    params.y = (GLfloat)y;
    params.z = (GLfloat)z;
    params.w = (GLfloat)w;

    __glProgramLocalParameter4fv(gc,target,index,&params);
}

GLvoid APIENTRY __glim_ProgramLocalParameter4dvARB(GLenum target, GLuint index,
                                                 const GLdouble *params)
{
    __GLcoord params_f;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ProgramLocalParameter4dvARB", DT_GLenum, target, DT_GLuint, index,
        DT_GLdouble_ptr, params, DT_GLnull);
#endif

    params_f.x = (GLfloat)*params++;
    params_f.y = (GLfloat)*params++;
    params_f.z = (GLfloat)*params++;
    params_f.w = (GLfloat)*params;

    __glProgramLocalParameter4fv(gc,target,index,&params_f);
}

GLvoid APIENTRY __glim_ProgramLocalParameter4fARB(GLenum target, GLuint index,
                                                GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GLcoord params_f;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ProgramLocalParameter4fARB", DT_GLenum, target, DT_GLuint, index,
        DT_GLfloat, x, DT_GLfloat, y, DT_GLfloat, z, DT_GLfloat, w, DT_GLnull);
#endif

    params_f.x = x;
    params_f.y = y;
    params_f.z = z;
    params_f.w = w;

    __glProgramLocalParameter4fv(gc,target,index,&params_f);
}

GLvoid APIENTRY __glim_ProgramLocalParameter4fvARB(GLenum target, GLuint index,
                                                 const GLfloat *params)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ProgramLocalParameter4fvARB", DT_GLenum, target, DT_GLuint, index,
        DT_GLfloat_ptr, params, DT_GLnull);
#endif

    __glProgramLocalParameter4fv(gc,target,index,(__GLcoord *)params);
}

GLvoid APIENTRY __glim_GetProgramEnvParameterdvARB(GLenum target, GLuint index,
                                                 GLdouble *params)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetProgramEnvParameterdvARB", DT_GLenum, target, DT_GLuint, index,
        DT_GLdouble_ptr, params, DT_GLnull);
#endif

    __GL_PROGRAM_CHECK_TARGET_INDEX(target,index,Env,target);

    *params++ = (GLdouble)gc->program.envParameter[target][index].x;
    *params++ = (GLdouble)gc->program.envParameter[target][index].y;
    *params++ = (GLdouble)gc->program.envParameter[target][index].z;
    *params = (GLdouble)gc->program.envParameter[target][index].w;
}

GLvoid APIENTRY __glim_GetProgramEnvParameterfvARB(GLenum target, GLuint index,
                                                 GLfloat *params)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetProgramEnvParameterfvARB", DT_GLenum, target, DT_GLuint, index,
        DT_GLfloat_ptr, params, DT_GLnull);
#endif

    __GL_PROGRAM_CHECK_TARGET_INDEX(target,index,Env,target);

    *params++ = gc->program.envParameter[target][index].x;
    *params++ = gc->program.envParameter[target][index].y;
    *params++ = gc->program.envParameter[target][index].z;
    *params = gc->program.envParameter[target][index].w;
}

GLvoid APIENTRY __glim_GetProgramLocalParameterdvARB(GLenum target, GLuint index,
                                                   GLdouble *params)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetProgramLocalParameterdvARB", DT_GLenum, target, DT_GLuint, index,
        DT_GLdouble_ptr, params, DT_GLnull);
#endif

    __GL_PROGRAM_CHECK_TARGET_INDEX(target,index,Local,target);

    *params++ = (GLdouble)gc->program.localParameter[target][index].x;
    *params++ = (GLdouble)gc->program.localParameter[target][index].y;
    *params++ = (GLdouble)gc->program.localParameter[target][index].z;
    *params = (GLdouble)gc->program.localParameter[target][index].w;
}

GLvoid APIENTRY __glim_GetProgramLocalParameterfvARB(GLenum target, GLuint index,
                                                   GLfloat *params)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetProgramLocalParameterfvARB", DT_GLenum, target, DT_GLuint, index,
        DT_GLfloat_ptr, params, DT_GLnull);
#endif

    __GL_PROGRAM_CHECK_TARGET_INDEX(target,index,Local,target);

    *params++ = gc->program.localParameter[target][index].x;
    *params++ = gc->program.localParameter[target][index].y;
    *params++ = gc->program.localParameter[target][index].z;
    *params = gc->program.localParameter[target][index].w;
}


/***********************************************************************
*   Dp will store the result of compiler in progObj->compiledResult
*   Includes:
*       1. Vertex input mask.
*       2. Texture samplers.
*************************************************************************/

#if (defined(_DEBUG) || defined(DEBUG))
/* Output the program string to c:\program.log */

#ifdef _LINUX_
#define ARB_PRG_LOG_NAME "/var/log/oglprogram.log"
#else
#define ARB_PRG_LOG_NAME "c:\\oglprogram.log"
#endif
static FILE *programFile=NULL;

GLvoid __gl_PrintPrgStr(__GLProgramObject * progObj)
{
    if (programFile)
    {
        fprintf(programFile, "Program Name = %d;\n Target = %s;\t String Length = %d; Valid? %c\n String:\n%s\n",
            progObj->name,
            (__GL_VERTEX_PROGRAM_INDEX==progObj->targetIndex)?"VERTEX_PROGRAM":"FRAGMENT_PROGRAM",
            progObj->programLen,
            (progObj->flags & __GL_PRGOBJFLAG_VALID)?'y':'n',
            progObj->programString);
        fflush(programFile);
    }
}
#endif

GLvoid APIENTRY __glim_ProgramStringARB(GLenum target, GLenum format, GLsizei len,
                                      const GLvoid *string)
{
    __GLProgramObject * progObj;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ProgramStringARB", DT_GLenum, target, DT_GLenum, format, DT_GLsizei, len,
        DT_GLvoid_ptr, string, DT_GLnull);
#endif

    __GL_PROGRAM_CHECK_TARGET(target,target);

    if(format != GL_PROGRAM_FORMAT_ASCII_ARB)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    if((len <= 0) || (!string))
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    progObj = gc->program.currentProgram[target];
    if (progObj == NULL)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* free the previous string  */
    if(NULL!=progObj->programString)
    {
        (*gc->imports.free)(gc, progObj->programString);
    }

    progObj->programString = (*gc->imports.malloc)(gc, len + 1 );
    if(!progObj->programString)
    {
        __glSetError(GL_OUT_OF_MEMORY);
        return;
    }

    __GL_MEMCOPY(progObj->programString, string, len);
    progObj->programString[len] = '\0';
    progObj->format = format;
    progObj->programLen = len;
    progObj->seqNumber++;

    /* call dp program string to compile the string. */
    if ((*gc->dp.ProgramStringARB)(gc,progObj) == GL_TRUE)
    {
        gc->program.errorPos = -1;/* no error  */
        gc->program.errStr[0] = '\0'; /* empty error string  */
        progObj->flags |= __GL_PRGOBJFLAG_VALID;
    }
    else
    {
        gc->program.errorPos = progObj->compiledResult.errorPos;/* we can get the exact error position at now  */
        __GL_MEMCOPY(gc->program.errStr,progObj->compiledResult.errStr,__GL_MAX_PROGRAM_ERRORSTR_LENGTH);
        progObj->flags &= ~__GL_PRGOBJFLAG_VALID;
        __glSetError(GL_INVALID_OPERATION);
    }

    progObj->flags |= __GL_PRGOBJFLAG_COMPILED;

    /* Program switch may cause inputMask to be changed. */
    __GL_INPUTMASK_CHANGED(gc);

    /* program string triggers a program switch also */
    __GL_SET_ATTR_DIRTY_BIT(gc,__GL_PROGRAM_ATTRS,__GL_DIRTY_VERTEX_PROGRAM_SWITCH << target);

#if (defined(_DEBUG) || defined(DEBUG))
    __gl_PrintPrgStr(progObj);
#endif

    return;
}




/************************************************************************/
/* program object management                                            */
/************************************************************************/
GLvoid __glInitProgramObject(__GLcontext *gc, __GLProgramObject *progObj, GLuint targetIndex, GLuint name)
{
    progObj->bindCount = 0;

    /* Init the state in program object */
    progObj->targetIndex = targetIndex;
    progObj->name        = name;
    progObj->seqNumber   = 0;
}



GLvoid __glBindProgram(__GLcontext *gc, GLuint targetIndex, GLuint program)
{
    __GLProgramObject *progObj, *boundProgObj;

    __GL_VERTEX_BUFFER_FLUSH(gc);

#if (defined(DEBUG) || defined(_DEBUG))
    if (program == gdbg_nameTarget)
        gdbg_nameTarget = gdbg_nameTarget;
#endif

    boundProgObj = gc->program.currentProgram[targetIndex];

    /* bind the same program object again */
    if(boundProgObj->name == program)
    {
        /* Copy local parameters from object into gc->program if locParamSeqNums are different */
        if (gc->program.locParamSeqNum[targetIndex] != boundProgObj->locParamSeqNum)
        {
            __GL_MEMCOPY(&gc->program.localParameter[targetIndex], boundProgObj->localParameter,
                __GL_MAX_PROGRAM_LOCAL_PARAMETERS * sizeof(__GLcoord));
            gc->program.locParamSeqNum[targetIndex] = boundProgObj->locParamSeqNum;

            /* Set this dirty bit to force a full update of program constants */
            __GL_SET_ATTR_DIRTY_BIT(gc,__GL_PROGRAM_ATTRS,__GL_DIRTY_VERTEX_PROGRAM_SWITCH << targetIndex);
        }

        if ((*gc->dp.BindProgramARB)(gc, boundProgObj, NULL))
        {
            /* Set this dirty bit to force a full update of program constants  when program string is changed */
            __GL_SET_ATTR_DIRTY_BIT(gc,__GL_PROGRAM_ATTRS,__GL_DIRTY_VERTEX_PROGRAM_SWITCH << targetIndex);

            /* Program switch may cause inputMask to be changed. */
            __GL_INPUTMASK_CHANGED(gc);

            __GL_VERTEX_BUFFER_FLUSH(gc);
        }
        return;
    }

    if(program == 0)
    {
        progObj = &gc->program.defaultProgram[targetIndex];
        GL_ASSERT(progObj != NULL);
    }
    else
    {
        progObj = __glGetObject(gc, gc->program.shared, program);
    }

    if(progObj == NULL)
    {
        /* create a new object  */
        progObj = (__GLProgramObject*)(*gc->imports.calloc)(gc, 1, sizeof(__GLProgramObject) );
        if(!progObj)
        {
            __glSetError(GL_OUT_OF_MEMORY);
            return;
        }
        __glInitProgramObject(gc,progObj,targetIndex,program);

        /* add to the program share machine */
        __glAddObject(gc,gc->program.shared,program,progObj);

        /* mark the name used */
        __glMarkNameUsed(gc,gc->program.shared,program);
    }
    else if (progObj->targetIndex != targetIndex)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    if (boundProgObj->name != 0)
    {
        boundProgObj->bindCount--;

        if (boundProgObj->bindCount == 0 && (boundProgObj->flag & __GL_OBJECT_IS_DELETED))
        {
            __glDeleteObject(gc, gc->program.shared, boundProgObj->name);
        }
    }

    if (progObj->name != 0)
    {
        progObj->bindCount++;
    }

    /* set default program object */
    gc->program.currentProgram[targetIndex] = progObj;

    /* Copy the local parameters into gc->program and save progObj->locParamSeqNum */
    __GL_MEMCOPY(&gc->program.localParameter[targetIndex], progObj->localParameter,
        __GL_MAX_PROGRAM_LOCAL_PARAMETERS * sizeof(__GLcoord));
    gc->program.locParamSeqNum[targetIndex] = progObj->locParamSeqNum;


    /* Set this dirty bit to force a full update of program constants */
    __GL_SET_ATTR_DIRTY_BIT(gc,__GL_PROGRAM_ATTRS,__GL_DIRTY_VERTEX_PROGRAM_SWITCH << targetIndex);

    /* Program switch may cause inputMask to be changed. */
    __GL_INPUTMASK_CHANGED(gc);

    (*gc->dp.BindProgramARB)(gc, progObj, NULL);
}

GLboolean __glDeleteProgramObject(__GLcontext *gc, GLvoid *obj)
{
    __GLProgramObject *progObj = obj;

    if (progObj == NULL)
        return GL_FALSE;

    GL_ASSERT(progObj->name);

    /* bind default program if the program object is currently bound to gc */
    if (gc->program.currentProgram[progObj->targetIndex] == progObj)
    {
        /*
        ** __GL_OBJECT_IS_DELETED is cleared because we do not want this object
        ** deleted in the following function, otherwise there will be recursion:
        ** __glDeleteObject-->__glBindObject-->glDeleteObject, and the object will
        ** be deleted twice.
        */
        progObj->flag &= ~__GL_OBJECT_IS_DELETED;
        __glBindProgram(gc,progObj->targetIndex,0);
    }

    /* Do not delete the progObj if there are other contexts bound to it. */
    if (progObj->bindCount != 0)
    {
        /* Set the flag to indicate the object is marked for delete */
        progObj->flag |= __GL_OBJECT_IS_DELETED;
        return GL_FALSE;
    }

    (*gc->dp.DeleteProgramARB)(gc, &progObj->privateData);

    /* The object is truly deleted here, so delete the object name from name list. */
    __glDeleteNamesFrList(gc, gc->program.shared, progObj->name, 1);

    /*Free the program string if exist.*/
    if (progObj->programString != NULL)
    {
        (*gc->imports.free)(gc,progObj->programString);
        progObj->programString = NULL;
    }

    /*Free itself*/
    (*gc->imports.free)(gc,progObj);

    return GL_TRUE;
}

/************************************************************************/
/*                  Program names functions.                            */
/************************************************************************/

GLvoid APIENTRY __glim_GenProgramsARB(GLsizei n, GLuint *programs)
{
    GLuint start;
    GLsizei i;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GenProgramsARB", DT_GLsizei, n, DT_GLuint_ptr, programs, DT_GLnull);
#endif

    if(n <= 0)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if(programs == NULL)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    GL_ASSERT(NULL != gc->program.shared);

    start = __glGenerateNames(gc, gc->program.shared, n);

    for (i = 0; i < n; i++) {
        programs[i] = start + i;
    }

    if (gc->program.shared->linearTable)
    {
        __glCheckLinearTableSize(gc, gc->program.shared, (start + n));
    }
}

GLvoid APIENTRY __glim_DeleteProgramsARB(GLsizei n, const GLuint *programs)
{
    GLint i;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_DeleteProgramsARB", DT_GLsizei, n, DT_GLuint_ptr, programs, DT_GLnull);
#endif

    if(n <= 0)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if(programs == NULL)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    for(i=0;i<n;i++)
    {
        /* skip default program */
        if (programs[i])
        {
            __glDeleteObject(gc, gc->program.shared, programs[i]);
        }
    }
}

GLboolean APIENTRY __glim_IsProgramARB(GLuint program)
{
    __GL_SETUP();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_IsProgramARB", DT_GLuint, program, DT_GLnull);
#endif

    if (program == 0)
    {
        return GL_FALSE;
    }
    return __glIsNameDefined(gc,gc->program.shared,program);
}


GLvoid APIENTRY __glim_BindProgramARB(GLenum target, GLuint program)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_BindProgramARB", DT_GLenum, target, DT_GLuint, program, DT_GLnull);
#endif

    __GL_PROGRAM_CHECK_TARGET(target,target);

    __glBindProgram(gc,target,program);
}


/************************************************************************/
/* Program machine management functions                                 */
/************************************************************************/

GLvoid __glInitProgramState(__GLcontext * gc)
{
    __GLProgramObject * progObj;

    if (gc->program.shared == NULL) {
        gc->program.shared = (__GLsharedObjectMachine *)
            (*gc->imports.calloc)(gc, 1, sizeof(__GLsharedObjectMachine) );

        /* Initialize a linear lookup table for program objects */
        gc->program.shared->maxLinearTableSize = __GL_MAX_PRGOBJ_LINEAR_TABLE_SIZE;
        gc->program.shared->linearTableSize = __GL_DEFAULT_PRGOBJ_LINEAR_TABLE_SIZE;
        gc->program.shared->linearTable = (GLvoid **)
            (*gc->imports.calloc)(gc, 1, gc->program.shared->linearTableSize * sizeof(GLvoid *) );

        gc->program.shared->hashSize = __GL_PRGOBJ_HASH_TABLE_SIZE;
        gc->program.shared->hashMask = __GL_PRGOBJ_HASH_TABLE_SIZE - 1;
        gc->program.shared->refcount = 1;
        gc->program.shared->deleteObject = __glDeleteProgramObject;
    }

    gc->program.errorPos = -1;

    /* Init the default vertex program. */
    progObj = &gc->program.defaultProgram[__GL_VERTEX_PROGRAM_INDEX];
    __glInitProgramObject(gc,progObj,__GL_VERTEX_PROGRAM_INDEX,0);

    /* Set current vertex program to the default vertex program. */
    gc->program.currentProgram[__GL_VERTEX_PROGRAM_INDEX] = progObj;

    /* Init the default fragment program. */
    progObj = &gc->program.defaultProgram[__GL_FRAGMENT_PROGRAM_INDEX];
    __glInitProgramObject(gc,progObj,__GL_FRAGMENT_PROGRAM_INDEX,0);

    /* Set current fragment program to the default fragment program. */
    gc->program.currentProgram[__GL_FRAGMENT_PROGRAM_INDEX] = progObj;

    /* Clear the env dirty  */
    __GL_MEMZERO(gc->program.envDirty,sizeof(gc->program.envDirty));

#if (defined(_DEBUG) || defined(DEBUG))
    /* Open the program string output file. */
    if (programFile == NULL)
    {
        programFile = fopen(ARB_PRG_LOG_NAME,"w");
    }
#endif
}

GLvoid __glFreeProgramState(__GLcontext * gc)
{
    __GLProgramObject *progObj;
    GLuint i;

    /* Free default program objects */
    for (i=0; i<__GL_NUMBER_OF_PROGRAM_TARGET; i++)
    {
        /* Unbind from non-default program object and bind to default program object */
        if (gc->program.currentProgram[i]->name != 0)
        {
            __glBindProgram(gc, gc->program.currentProgram[i]->targetIndex, 0);
        }

        /* Delete default program object's ctxPrivData */
        progObj = gc->program.currentProgram[i];

        (*gc->dp.DeleteProgramARB)(gc, &progObj->privateData);

        /* Free the program string if exist */
        if (progObj->programString)
        {
            (*gc->imports.free)(gc, progObj->programString);
            progObj->programString = NULL;
        }
    }

    /* Free shared program object table */
    __glFreeSharedObjectState(gc, gc->program.shared);

#if (defined(_DEBUG) || defined(DEBUG))
    if (programFile != NULL)
    {
        fclose(programFile);
        programFile = NULL;
    }
#endif
}

/* Check whether the program is enabled. A program is really enabled when
1. The Vertex_program or Fragment_program is enabled.
2. No compile error.
*/
GLvoid __glVertexProgramRealEnabled(__GLcontext * gc)
{
    __GLProgramObject * progObj;
    GLboolean enabled = GL_FALSE;

    if(gc->state.enables.program.vertexProgram)
    {
        progObj = gc->program.currentProgram[__GL_VERTEX_PROGRAM_INDEX];
        if (progObj != NULL)
        {
            if (progObj->flags & __GL_PRGOBJFLAG_VALID)
                enabled = GL_TRUE;
        }
    }

    gc->program.realEnabled[__GL_VERTEX_PROGRAM_INDEX] = enabled;
}


GLvoid __glFragmentProgramRealEnabled(__GLcontext * gc)
{
    __GLProgramObject * progObj;

    GLboolean enabled = GL_FALSE;

    if(gc->state.enables.program.fragmentProgram)
    {
        progObj = gc->program.currentProgram[__GL_FRAGMENT_PROGRAM_INDEX];
        if (progObj != NULL)
        {
            if (progObj->flags & __GL_PRGOBJFLAG_VALID)
                enabled = GL_TRUE;
        }
    }

    gc->program.realEnabled[__GL_FRAGMENT_PROGRAM_INDEX] = enabled;

}

/*
** Used to share program objects between two different contexts.
*/
GLvoid __glShareProgramObjects(__GLcontext *dst, __GLcontext *src)
{
    if (dst->program.shared)
    {
        __glFreeSharedObjectState(dst, dst->program.shared);
    }

    dst->program.shared = src->program.shared;
    dst->program.shared->refcount++;
}

/************************************************************************/
/* program get function. many things are empty                          */
/************************************************************************/
GLvoid APIENTRY __glim_GetProgramivARB(GLenum target, GLenum pname, GLint *params)
{
    __GLProgramObject *progObj;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetProgramivARB", DT_GLenum, target, DT_GLenum, pname, DT_GLint_ptr, params, DT_GLnull);
#endif

    __GL_PROGRAM_CHECK_TARGET(target,target);

    progObj = gc->program.currentProgram[target];

    if(params == NULL)
        return;

    switch(pname)
    {
    case GL_PROGRAM_LENGTH_ARB:
        *params = progObj->programLen;
        break;

    case GL_PROGRAM_FORMAT_ARB:
        *params = progObj->format;
        break;

    case GL_PROGRAM_BINDING_ARB:
        *params = progObj->name;
        break;

    case GL_PROGRAM_INSTRUCTIONS_ARB:
        break;

    case GL_MAX_PROGRAM_INSTRUCTIONS_ARB:
        *params = __GL_MAX_PROGRAM_INSTRUCTIONS;
        break;

    case GL_PROGRAM_NATIVE_INSTRUCTIONS_ARB:
        *params = progObj->compiledResult.nativeInstruction;
        break;

    case GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB:
        *params = gc->constants.maxInstruction[target];
        break;

    case GL_PROGRAM_TEMPORARIES_ARB:
        break;

    case GL_MAX_PROGRAM_TEMPORARIES_ARB:
        *params = gc->constants.maxTemp[target];
        break;

    case GL_PROGRAM_NATIVE_TEMPORARIES_ARB:
        break;

    case GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB:
        *params = gc->constants.maxTemp[target];
        break;

    case GL_PROGRAM_PARAMETERS_ARB:
        break;

    case GL_MAX_PROGRAM_PARAMETERS_ARB:
        *params = gc->constants.maxParameter[target];
        break;

    case GL_PROGRAM_NATIVE_PARAMETERS_ARB:
        break;

    case GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB:
        *params = gc->constants.maxParameter[target];
        break;

    case GL_PROGRAM_ATTRIBS_ARB:
        break;

    case GL_MAX_PROGRAM_ATTRIBS_ARB:
        *params = __GL_MAX_PROGRAM_VERTEX_ATTRIBUTES;
        break;

    case GL_PROGRAM_NATIVE_ATTRIBS_ARB:
        break;

    case GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB:
        *params = __GL_MAX_PROGRAM_VERTEX_ATTRIBUTES;
        break;

    case GL_PROGRAM_ADDRESS_REGISTERS_ARB:
        break;

    case GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB:
        *params = gc->constants.maxAddressRegister[target];
        break;

    case GL_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB:
        break;

    case GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB:
        *params = gc->constants.maxAddressRegister[target];
        break;

    case GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB:
        *params = gc->constants.maxLocalParameter[target];
        break;

    case GL_MAX_PROGRAM_ENV_PARAMETERS_ARB:
        *params = gc->constants.maxEnvParameter[target];
        break;

    case GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB:
        *params = progObj->compiledResult.nativeLimit;
        break;
    case GL_MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB:
        *params = gc->constants.maxPSTexInstr;
        break;
    case GL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB:
        *params = gc->constants.maxPSALUInstr;
        break;
    case GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB:
        *params = gc->constants.maxPSTexIndirectionInstr;
        break;
    case GL_MAX_PROGRAM_ALU_INSTRUCTIONS_ARB:
        *params = gc->constants.maxPSALUInstr;
        break;
    case GL_MAX_PROGRAM_TEX_INSTRUCTIONS_ARB:
        *params = gc->constants.maxPSTexInstr;
        break;
    case GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB:
        *params = gc->constants.maxPSTexIndirectionInstr;
        break;
    case GL_PROGRAM_ALU_INSTRUCTIONS_ARB:
        break;
    case GL_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB:
        break;
    case GL_PROGRAM_TEX_INSTRUCTIONS_ARB:
        break;
    case GL_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB:
        break;
    case GL_PROGRAM_TEX_INDIRECTIONS_ARB:
        break;
    case GL_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB:
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        break;
    }
}

GLvoid APIENTRY __glim_GetProgramStringARB(GLenum target, GLenum pname, GLvoid *string)
{
    __GLProgramObject *progObj;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetProgramStringARB", DT_GLenum, target, DT_GLenum, pname, DT_GLvoid_ptr, string, DT_GLnull);
#endif

    __GL_PROGRAM_CHECK_TARGET(target,target);

    progObj = gc->program.currentProgram[target];

    if(pname != GL_PROGRAM_STRING_ARB)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    if(progObj)
    {
        if(progObj->programString)
            __GL_MEMCOPY((GLubyte *)string, progObj->programString,progObj->programLen);
        else
            ((GLubyte *)string)[0] = 0;
    }
    else
    {
        __glSetError(GL_INVALID_OPERATION);
    }
}


#if GL_EXT_gpu_program_parameters
GLvoid APIENTRY __glim_ProgramEnvParameters4fvEXT(GLenum target, GLuint index, GLsizei count,
                                               const GLfloat *params)
{
    GLsizei i;
    __GLcoord *param;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ProgramEnvParameter4fvEXT", DT_GLenum, target, DT_GLuint, index, DT_GLsizei, count,
        DT_GLfloat_ptr, params, DT_GLnull);
#endif

    /* index check was done in __glProgramEnvParameter4fv() */
    if((count <= 0))
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    param = (__GLcoord *)params;

    for(i = 0; i < count; i++)
    {
        __glProgramEnvParameter4fv(gc,target,index,param);
        param++;
        index++;
    }
}

GLvoid APIENTRY __glim_ProgramLocalParameters4fvEXT(GLenum target, GLuint index, GLsizei count,
                                               const GLfloat *params)
{
    GLsizei i;
    __GLcoord *param;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ProgramLocalParameter4fvEXT", DT_GLenum, target, DT_GLuint, index, DT_GLsizei, count,
        DT_GLfloat_ptr, params, DT_GLnull);
#endif

    /* index check was done in __glProgramEnvParameter4fv() */
    if((count <= 0))
    {
        __glSetError(GL_INVALID_VALUE );
        return;
    }


    param = (__GLcoord *)params;

    for(i = 0; i < count; i++)
    {
        __glProgramLocalParameter4fv(gc,target,index,param);
        param++;
        index++;
    }
}
#endif

