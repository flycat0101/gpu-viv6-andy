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
#include "../gc_gl_names_inline.c"
#include "gc_gl_bindtable.h"
#include "GL/gl.h"
#include "gc_gl_debug.h"
#include <ctype.h>

#ifndef GL_VERTEX_ATTRIB_ARRAY_INTEGER_EXT
#define GL_VERTEX_ATTRIB_ARRAY_INTEGER_EXT 0x88FD
#endif
/************************************************************************/
/* Section for extern declaration                                       */
/************************************************************************/
extern GLvoid __glUpdateProgramEnableDimension(__GLcontext * gc);


/************************************************************************/
/* Section for internal declaration                                     */
/************************************************************************/

/*
** Lookup table from sampler data type to texture enable dimension.
*/

#define __GLSL_SET_STATE_USAGE(usage, index, bit)   \
    (usage)->globalAttrState[(index)] |= (bit);     \
    (usage)->globalAttrState[__GLSL_USAGE_ALL_ATTRS] |= (1 << (index))

#define __GLSL_SET_LIGHT_STATE_USAGE(usage, lighti, bit)                    \
    (usage)->lightAttrState[(lighti)] |= (bit);                             \
    (usage)->globalAttrState[__GLSL_USAGE_LIGHT_ATTRS] |= (1 << (lighti));  \
    (usage)->globalAttrState[__GLSL_USAGE_ALL_ATTRS] |= (1 << __GLSL_USAGE_LIGHT_ATTRS)

#define __GLSL_SET_TEXTURE_STATE_USAGE(usage, texi, bit)                    \
    (usage)->texAttrState[(texi)] |= (bit);                                 \
    (usage)->globalAttrState[__GLSL_USAGE_TEXTURE_ATTRS] |= (1 << (texi));  \
    (usage)->globalAttrState[__GLSL_USAGE_ALL_ATTRS] |= (1 << __GLSL_USAGE_TEXTURE_ATTRS)

GLboolean __glInitShaderObject(__GLcontext *gc, __GLshaderObject *shaderObject, GLuint shaderType, GLuint id);
GLboolean __glInitShaderProgramObject(__GLcontext *gc, __GLshaderProgramObject *programObject, GLuint id);
GLboolean __glDeleteShaderObject(__GLcontext *gc, __GLshaderObject * shaderObject);
GLboolean __glDeleteProgram2Object(__GLcontext *gc, __GLshaderProgramObject * programObject);
GLboolean __glDeleteShaderProgramObject(__GLcontext *gc, GLvoid * obj);
GLuint __glFindEmptySlot(__GLshaderProgramObject * programObject);
GLuint __glFindAttachSlot(__GLshaderProgramObject * programObject, __GLshaderObject * shaderObject);
GLvoid __glAttachShader(__GLcontext *gc, __GLshaderProgramObject * programObject, __GLshaderObject * shaderObject);
GLvoid __glDetachShader(__GLcontext *gc, __GLshaderProgramObject * programObject, __GLshaderObject * shaderObject);
GLboolean __glIsShaderAttached(__GLshaderProgramObject * programObject, __GLshaderObject * shaderObject);

GLboolean __glParseUnifromName(const GLchar* name, GLuint* nameLen, GLuint* arrayIdx, GLboolean* bArray);
GLvoid __glSLangBuildTextureEnableDim(__GLcontext *gc);


/* Internal functions for object management */
GLboolean __glInitShaderObject(__GLcontext *gc, __GLshaderObject *shaderObject, GLuint shaderType, GLuint id)
{
    shaderObject->objectInfo.bindCount = 0;

    shaderObject->objectInfo.id = id;
    shaderObject->objectInfo.objectType = __GL_SHADER_OBJECT_TYPE;

    shaderObject->shaderInfo.shaderType = shaderType;

    return GL_TRUE;
}

GLboolean __glInitShaderProgramObject(__GLcontext *gc, __GLshaderProgramObject *programObject, GLuint id)
{
    GLuint i;

    programObject->objectInfo.bindCount = 0;

    programObject->objectInfo.id = id;
    programObject->objectInfo.objectType = __GL_PROGRAM_OBJECT_TYPE;

    programObject->programInfo.geomVerticesOut = 0;
    programObject->programInfo.geomInputType = GL_TRIANGLES;
    programObject->programInfo.geomOutputType = GL_TRIANGLE_STRIP;

    /* create the attach list for the program object */
    programObject->programInfo.attachedShaders =
        (__GLshaderObject **)(*gc->imports.calloc)(gc, 1, sizeof(__GLshaderObject *) * __GL_MAX_ATTACHED_SHADERS );
    if(!programObject->programInfo.attachedShaders) {
        __glSetError(GL_OUT_OF_MEMORY);
        return GL_FALSE;
    }
    programObject->programInfo.attachedShadersTableSize = __GL_MAX_ATTACHED_SHADERS;

    for (i=0; i<__GL_MAX_GLSL_SAMPLERS; i++)
    {
        programObject->bindingInfo.sampler2TexUnit[i] = __GL_MAX_TEXTURE_UNITS;
    }

    programObject->bindingInfo.samplerSeq = 0;

    return GL_TRUE;
}

GLuint __glFindEmptySlot(__GLshaderProgramObject * programObject)
{
    GLuint i;

    for(i = 0; i < programObject->programInfo.attachedShadersTableSize; i++)
    {
        if(programObject->programInfo.attachedShaders[i] == NULL)
        {
            break;
        }
    }

    return i;
}


GLuint __glFindAttachSlot(__GLshaderProgramObject * programObject, __GLshaderObject * shaderObject)
{
    GLuint i;

    for(i = 0; i < programObject->programInfo.attachedShadersTableSize; i++)
    {
        if(programObject->programInfo.attachedShaders[i] == shaderObject)
        {
            break;
        }
    }

    return i;
}

GLvoid __glAttachShader(__GLcontext *gc, __GLshaderProgramObject * programObject, __GLshaderObject * shaderObject)
{
    GLuint index;
    __GLshaderObject **table;

    GL_ASSERT(programObject);
    GL_ASSERT(shaderObject);

    if(__glIsShaderAttached(programObject, shaderObject))
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    index = __glFindEmptySlot(programObject);
    if(index < programObject->programInfo.attachedShadersTableSize)
    {
        programObject->programInfo.attachedShaders[index] = shaderObject;
        programObject->programInfo.count++;
    }
    else
    {
        GLuint oldSize = programObject->programInfo.attachedShadersTableSize;
        GLuint newSize = oldSize + __GL_MAX_ATTACHED_SHADERS;
        table = (__GLshaderObject **)(*gc->imports.malloc)(gc,
            sizeof(__GLshaderObject*) * newSize );

        if(!table)
        {
            __glSetError(GL_OUT_OF_MEMORY);
            return;
        }

        /* Copy previous table */
        if(oldSize)
        {
            __GL_MEMCOPY(table,programObject->programInfo.attachedShaders, oldSize * sizeof(__GLshaderObject*));
        }
        (*gc->imports.free)(gc, programObject->programInfo.attachedShaders);
        programObject->programInfo.attachedShaders = table;
        programObject->programInfo.attachedShadersTableSize = newSize;

        programObject->programInfo.attachedShaders[programObject->programInfo.count] = shaderObject;
        programObject->programInfo.count++;

    }

    shaderObject->attachCount++;
}

GLvoid __glDetachShader(__GLcontext *gc, __GLshaderProgramObject * programObject, __GLshaderObject * shaderObject)
{
    GLuint index;

    GL_ASSERT(programObject);
    GL_ASSERT(shaderObject);

    index = __glFindAttachSlot(programObject, shaderObject);

    if(index < programObject->programInfo.attachedShadersTableSize)
    {
        programObject->programInfo.attachedShaders[index] = NULL;
        programObject->programInfo.count--;

        shaderObject->attachCount--;
        if(shaderObject->shaderInfo.deleteStatus)
        {
            __glDeleteObject(gc, gc->shaderProgram.shared, shaderObject->objectInfo.id);
        }
    }
    else
    {
        __glSetError(GL_INVALID_OPERATION);
    }
}

GLboolean __glIsShaderAttached(__GLshaderProgramObject * programObject, __GLshaderObject * shaderObject)
{
    if(__glFindAttachSlot(programObject, shaderObject) < programObject->programInfo.attachedShadersTableSize)
    {
        return GL_TRUE;
    }
    else
    {
        return GL_FALSE;
    }
}

GLboolean __glDeleteShaderObject(__GLcontext *gc, __GLshaderObject * shaderObject)
{
    GL_ASSERT(shaderObject);

    /* Check whether could be deleted*/
    if(shaderObject->attachCount)
    {
        shaderObject->shaderInfo.deleteStatus = GL_TRUE;
        return GL_FALSE;
    }

    /* Delete name */
    __glDeleteNamesFrList(gc, gc->shaderProgram.shared, shaderObject->objectInfo.id, 1);

    /* delete shader source */
    if (shaderObject->shaderInfo.source) {
        (*gc->imports.free)(gc, shaderObject->shaderInfo.source);
        shaderObject->shaderInfo.source = NULL;
    }

    (*gc->dp.deleteShader)(gc, shaderObject);

    /* Delete shader object */
    (*gc->imports.free)(gc, shaderObject);

    return GL_TRUE;
}

GLboolean __glDeleteProgram2Object(__GLcontext *gc, __GLshaderProgramObject * programObject)
{
    GLuint i = 0;

    GL_ASSERT(programObject);

    if( programObject->objectInfo.id == gc->shaderProgram.lastProgram)
    {
        /* Add this logic to avoid that evaluate function miss detect */
        gc->shaderProgram.lastProgram = 0xFFFFFFFF;
        gc->shaderProgram.lastCodeSeq = 0xFFFFFFFF;
    }

    if (programObject->objectInfo.bindCount != 0)
    {
        programObject->programInfo.deletedStatus = GL_TRUE;
        return GL_FALSE;
    }

    (*gc->dp.deleteShaderProgram)(gc, &programObject->privateData);

    /* Detach all shaders */
    for(i = 0; i < programObject->programInfo.attachedShadersTableSize; i++)
    {
        if(programObject->programInfo.attachedShaders[i])
        {
            __glDetachShader(gc, programObject, programObject->programInfo.attachedShaders[i]);
        }
    }

    /* Delete name */
    __glDeleteNamesFrList(gc, gc->shaderProgram.shared, programObject->objectInfo.id, 1);

    /* Delete program object */

    if(programObject->programInfo.attachedShaders)
    {
        (*gc->imports.free)(gc, programObject->programInfo.attachedShaders);
    }

    if(programObject->bindingInfo.pFragmentVaringOutTable)
    {
        (*gc->imports.free)(gc, programObject->bindingInfo.pFragmentVaringOutTable);
    }

    (*gc->imports.free)(gc, programObject);

    return GL_TRUE;

}

GLboolean __glDeleteShaderProgramObject(__GLcontext *gc, GLvoid * obj)
{
    __GLobjectInfo * objectInfo = (__GLobjectInfo *)obj;

    switch(objectInfo->objectType)
    {
    case __GL_SHADER_OBJECT_TYPE:
        return __glDeleteShaderObject(gc, (__GLshaderObject *)obj);
        break;

    case __GL_PROGRAM_OBJECT_TYPE:
        return __glDeleteProgram2Object(gc, (__GLshaderProgramObject*)obj);
        break;

    default :
        return GL_FALSE;
    }
}

GLvoid __glInitShaderProgramState(__GLcontext *gc)
{
    GLuint i;

    if (gc->shaderProgram.shared == NULL)
    {
        gc->shaderProgram.shared = (__GLsharedObjectMachine *)
            (*gc->imports.calloc)(gc, 1, sizeof(__GLsharedObjectMachine) );
        /* Initialize a linear lookup table for program objects */
        gc->shaderProgram.shared->maxLinearTableSize = __GL_MAX_PRGOBJ_LINEAR_TABLE_SIZE;
        gc->shaderProgram.shared->linearTableSize = __GL_DEFAULT_PRGOBJ_LINEAR_TABLE_SIZE;
        gc->shaderProgram.shared->linearTable = (GLvoid **)
            (*gc->imports.calloc)(gc, 1, gc->shaderProgram.shared->linearTableSize * sizeof(GLvoid *) );

        gc->shaderProgram.shared->hashSize = __GL_PRGOBJ_HASH_TABLE_SIZE;
        gc->shaderProgram.shared->hashMask = __GL_PRGOBJ_HASH_TABLE_SIZE - 1;
        gc->shaderProgram.shared->refcount = 1;
        gc->shaderProgram.shared->deleteObject = __glDeleteShaderProgramObject;

        /*Intialization of deleteCtxPrivData will delay to the final of create context*/
    }

    /* Init the current program */
    gc->shaderProgram.currentShaderProgram = NULL;

    gc->shaderProgram.vertShaderEnable = GL_FALSE;
    gc->shaderProgram.geomShaderEnable = GL_FALSE;
    gc->shaderProgram.fragShaderEnable = GL_FALSE;

    gc->shaderProgram.vertShaderRealEnable = GL_FALSE;
    gc->shaderProgram.geomShaderRealEnable = GL_FALSE;
    gc->shaderProgram.fragShaderRealEnable = GL_FALSE;

    gc->shaderProgram.samplerDirtyState = 0;
    gc->shaderProgram.samplerSeq = 0;

    gc->shaderProgram.lastProgram = 0xFFFFFFFF;
    gc->shaderProgram.lastCodeSeq = 0xFFFFFFFF;

    gc->shaderProgram.lastVertShaderEnable = GL_FALSE;
    gc->shaderProgram.lastFragShaderEnable = GL_FALSE;

    for (i=0; i<__GL_MAX_GLSL_SAMPLERS; i++)
    {
        gc->shaderProgram.prevSampler2TexUnit[i] = __GL_MAX_TEXTURE_UNITS;
    }
}

GLvoid __glFreeShaderProgramState(__GLcontext * gc)
{
    __GLshaderProgramObject *shaderProgram = gc->shaderProgram.currentShaderProgram;

    if(shaderProgram)
    {
        shaderProgram->objectInfo.bindCount--;
        if (shaderProgram->objectInfo.bindCount == 0 && shaderProgram->programInfo.deletedStatus)
        {
            __glDeleteObject(gc, gc->shaderProgram.shared, shaderProgram->objectInfo.id);
        }

        gc->shaderProgram.currentShaderProgram = NULL;
    }

    /* Free shared program object table */
    __glFreeSharedObjectState(gc, gc->shaderProgram.shared);

}

GLvoid __glShareShaderProgramObjects(__GLcontext *dst, __GLcontext *src)
{
    if (dst->shaderProgram.shared)
    {
        __glFreeSharedObjectState(dst, dst->shaderProgram.shared);
    }

    dst->shaderProgram.shared = src->shaderProgram.shared;
    dst->shaderProgram.shared->refcount++;
}

GLboolean __glParseUnifromName(const GLchar* name, GLuint* nameLen, GLuint* arrayIdx, GLboolean* bArray)
{
    GLuint origLen = 0;
    const GLchar* p = NULL;
    const GLchar* start = NULL;
    const GLchar* end = NULL;
    GLuint index =0;
    GL_ASSERT(name);
    GL_ASSERT(nameLen);
    GL_ASSERT(arrayIdx);

    origLen = (GLuint)strlen((const char *)name);

    if( (origLen >= 4) && (name[origLen-1] == ']') )
    {
        start = (const GLchar*)strrchr((const char *)name, '[');
        if(start)
        {
            end = name + origLen - 1;
            for(p = start+1; p < end; p++)
            {
                if(!isdigit(*p))
                {
                    return GL_FALSE;
                }
                index = index*10 + ((*p)-'0');
            }

            *nameLen = (GLuint)(start - name);
            *arrayIdx = index;
            *bArray = GL_TRUE;
            return GL_TRUE;
        }
        else
        {
            return GL_FALSE;
        }
    }

    *nameLen = origLen;
    *arrayIdx = 0;
    *bArray = GL_FALSE;
    return GL_TRUE;
}

GLvoid __glSLangBuildTextureEnableDim(__GLcontext *gc)
{
    (*gc->dp.buildTextureEnableDim)(gc);
}

GLboolean __glSLangCheckTextureConflict(__GLcontext* gc, __GLshaderProgramObject *programObject)
{
    return (*gc->dp.checkTextureConflict)(gc, programObject);
}

/************************************************************************/
/* Section for implantation of API fucntions                           */
/************************************************************************/
GLuint APIENTRY __glim_CreateShader( GLenum type)
{
    GLuint shader = 0;
    __GLshaderObject* shaderObject = NULL;

    __GL_SETUP_NOT_IN_BEGIN_RET(0);

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_CreateShader", DT_GLenum, type, DT_GLnull);
#endif

    if((type != GL_VERTEX_SHADER) && (type != GL_FRAGMENT_SHADER) && (type != GL_GEOMETRY_SHADER_EXT))
    {
        __glSetError(GL_INVALID_ENUM);
        return 0;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    GL_ASSERT(gc->shaderProgram.shared);
    shader = __glGenerateNames(gc, gc->shaderProgram.shared, 1);
    __glMarkNameUsed(gc, gc->shaderProgram.shared, shader);

    shaderObject = (__GLshaderObject *)(*gc->imports.calloc)(gc, 1, sizeof(__GLshaderObject) );
    if(NULL == shaderObject){
        __glSetError(GL_OUT_OF_MEMORY);
        return 0;
    }

    __glInitShaderObject(gc, shaderObject, type, shader);
    __glAddObject(gc, gc->shaderProgram.shared, shader, shaderObject);

    return shader;
}

GLvoid APIENTRY __glim_ShaderSource( GLuint shader, GLsizei count, const GLchar ** string, const GLint *length)
{
    __GLshaderObject * shaderObject = NULL;
    GLubyte *source = NULL;
    GLuint sourceLen = 0;
    GLsizei i = 0;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ShaderSource", DT_GLuint, shader, DT_GLsizei, count, DT_GLubyte_ptr, string, DT_GLint_ptr, length, DT_GLnull);
#endif


    if((count < 0) || (string == NULL))
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if(shader <= 0)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    GL_ASSERT(gc->shaderProgram.shared);
    shaderObject = (__GLshaderObject *)__glGetObject(gc, gc->shaderProgram.shared, shader);
    if(!shaderObject)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    else if(shaderObject->objectInfo.objectType != __GL_SHADER_OBJECT_TYPE)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    /* Calculate source size */
    if(length)
    {
        for(i = 0; i < count; i++)
        {
            sourceLen += (length[i] < 0) ? (GLuint)strlen((const char *)string[i]) : length[i];
        }
    }
    else
    {
        for(i = 0; i < count; i++)
        {
            sourceLen += (GLuint)strlen((const char *)string[i]);
        }
    }

    /* Assemble all sources into one string*/
    source =(GLubyte *)(*gc->imports.malloc)(gc, sourceLen+1 );
    if(NULL == source)
    {
        __glSetError(GL_OUT_OF_MEMORY);
        return;
    }
    source[0] = '\0';
    for(i = 0; i < count; i++)
    {
        GLuint len = 0;
        if(length)
        {
            len = (length[i] < 0) ? (GLuint)strlen((const char *)string[i]) : length[i];
        }
        else
        {
            len = (GLuint)strlen((const char *)string[i]);
        }
        if(string[i])
        {
            strncat((char *)source, (const char *)string[i], len);
        }
    }

    shaderObject->shaderInfo.source = source;
    shaderObject->shaderInfo.sourceSize = sourceLen;

    return;
}


GLvoid APIENTRY __glim_CompileShader( GLuint shader)
{
    __GLshaderObject * shaderObject;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_CompileShader", DT_GLuint, shader, DT_GLnull);
#endif

    GL_ASSERT(gc->shaderProgram.shared);
    shaderObject = (__GLshaderObject *)__glGetObject(gc, gc->shaderProgram.shared, shader);

    if(!shaderObject)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    else if(shaderObject->objectInfo.objectType != __GL_SHADER_OBJECT_TYPE)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    shaderObject->shaderInfo.compiledStatus = (*gc->dp.compileShader)(gc, shaderObject);
}

GLvoid APIENTRY __glim_DeleteShader( GLuint shader)
{
    __GLshaderObject * shaderObject;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_DeleteShader", DT_GLuint, shader, DT_GLnull);
#endif

    if(shader <= 0)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    GL_ASSERT(gc->shaderProgram.shared);
    shaderObject = (__GLshaderObject *)__glGetObject(gc, gc->shaderProgram.shared, shader);
    if(!shaderObject)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    if(shaderObject->objectInfo.objectType != __GL_SHADER_OBJECT_TYPE)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    __glDeleteObject(gc, gc->shaderProgram.shared, shaderObject->objectInfo.id);

}

GLuint APIENTRY __glim_CreateProgram (GLvoid)
{
    GLuint program;
    __GLshaderProgramObject * programObject;

    __GL_SETUP_NOT_IN_BEGIN_RET(0);

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_CreateProgram", DT_GLnull);
#endif

    GL_ASSERT(gc->shaderProgram.shared);
    program = __glGenerateNames(gc, gc->shaderProgram.shared, 1);
    __glMarkNameUsed(gc, gc->shaderProgram.shared, program);

    programObject = (__GLshaderProgramObject *)(*gc->imports.calloc)(gc, 1, sizeof(__GLshaderProgramObject) );
    if(!programObject)
    {
        __glSetError(GL_OUT_OF_MEMORY);
        return 0;
    }

    if(__glInitShaderProgramObject(gc, programObject, program))
    {
        /* Add this shader object to the "gc->shaderProgram.shared" structure.
        */
        __glAddObject(gc, gc->shaderProgram.shared, program, programObject);
    }
    else
    {
        (*gc->imports.free)(gc, programObject);
        __glDeleteNamesFrList(gc, gc->shaderProgram.shared, program, 1);
        program = 0;
    }

    return program;
}


GLvoid APIENTRY __glim_AttachShader( GLuint program, GLuint shader)
{
    __GLshaderObject * shaderObject;
    __GLshaderProgramObject * programObject;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_AttachShader", DT_GLuint, program, DT_GLuint, shader, DT_GLnull);
#endif

    if((program <= 0) || (shader <= 0)) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    GL_ASSERT(gc->shaderProgram.shared);
    shaderObject = (__GLshaderObject *)__glGetObject(gc, gc->shaderProgram.shared, shader);
    if(!shaderObject) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    else if(shaderObject->objectInfo.objectType != __GL_SHADER_OBJECT_TYPE) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = (__GLshaderProgramObject *)__glGetObject(gc, gc->shaderProgram.shared, program);
    if(!programObject) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    else if(programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    __glAttachShader(gc, programObject, shaderObject);
}

GLvoid APIENTRY __glim_DetachShader( GLuint program, GLuint shader)
{
    __GLshaderObject * shaderObject;
    __GLshaderProgramObject * programObject;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_DetachShader", DT_GLuint, program, DT_GLuint, shader, DT_GLnull);
#endif

    if((program <= 0) || (shader <= 0)) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    GL_ASSERT(gc->shaderProgram.shared);
    shaderObject = (__GLshaderObject *)__glGetObject(gc, gc->shaderProgram.shared, shader);
    if(!shaderObject) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    else if(shaderObject->objectInfo.objectType != __GL_SHADER_OBJECT_TYPE) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = (__GLshaderProgramObject *)__glGetObject(gc, gc->shaderProgram.shared, program);
    if(!programObject) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    else if(programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    __glDetachShader(gc, programObject, shaderObject);
}

GLvoid APIENTRY __glim_LinkProgram( GLuint program)
{
    __GLshaderProgramObject * programObject = NULL;
    GLuint i = 0;
    GLboolean vertShaderEnable = GL_FALSE;
    GLboolean geomShaderEnable = GL_FALSE;
    GLboolean fragShaderEnable = GL_FALSE;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_LinkProgram", DT_GLuint, program, DT_GLnull);
#endif

    if(program <= 0)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    GL_ASSERT(gc->shaderProgram.shared);
    programObject = (__GLshaderProgramObject *)__glGetObject(gc, gc->shaderProgram.shared, program);
    if(!programObject)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    else if(programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject->programInfo.codeSeq++;

    programObject->programInfo.linkedStatus = (*gc->dp.linkProgram)(gc, programObject);

    for(i = 0; i < programObject->programInfo.attachedShadersTableSize; i++)
    {
        if(programObject->programInfo.attachedShaders[i])
        {
            switch(programObject->programInfo.attachedShaders[i]->shaderInfo.shaderType){
            case GL_VERTEX_SHADER:
                vertShaderEnable = GL_TRUE;
                break;
            case GL_FRAGMENT_SHADER:
                fragShaderEnable = GL_TRUE;
                break;
            case GL_GEOMETRY_SHADER_EXT:
                geomShaderEnable = GL_TRUE;
                break;
            }
        }
    }

    programObject->programInfo.vertShaderEnable = vertShaderEnable;
    programObject->programInfo.geomShaderEnable = geomShaderEnable;
    programObject->programInfo.fragShaderEnable = fragShaderEnable;


    if( programObject->programInfo.linkedStatus )
    {
        /* Save output type of current link */
        programObject->programInfo.geomRealOutputType = programObject->programInfo.geomOutputType;
    }

    /* If program is currently in use and re-linked sucessfully, it should be reload to the
    ** vertex processor, so should flush the previous draw first.
    */
    if((gc->shaderProgram.currentShaderProgram == programObject) &&
        programObject->programInfo.linkedStatus)
    {
        __GL_VERTEX_BUFFER_FLUSH(gc);
        (*gc->dp.useProgram)(gc, programObject, NULL);


        /* Repick enable/disable flags because they may be changed after relink. */
        gc->shaderProgram.vertShaderEnable = programObject->programInfo.vertShaderEnable;
        gc->shaderProgram.geomShaderEnable = programObject->programInfo.geomShaderEnable;
        gc->shaderProgram.fragShaderEnable = programObject->programInfo.fragShaderEnable;
        gc->shaderProgram.geomOutputType   = programObject->programInfo.geomRealOutputType;

        __GL_SET_ATTR_DIRTY_BIT(gc,__GL_PROGRAM_ATTRS,__GL_DIRTY_GLSL_PROGRAM_SWITCH);
    }
}


GLvoid APIENTRY __glim_UseProgram( GLuint program)
{
    __GLshaderProgramObject * programObject = NULL;
    __GLshaderProgramObject * oldProgramObject = NULL;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_UseProgram", DT_GLuint, program, DT_GLnull);
#endif

    GL_ASSERT(gc->shaderProgram.shared);
    if(program != 0)
    {
        programObject = (__GLshaderProgramObject *)__glGetObject(gc, gc->shaderProgram.shared, program);
        if(!programObject)
        {
            __glSetError(GL_INVALID_VALUE);
            return;
        }

        if(programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
        {
            __glSetError(GL_INVALID_OPERATION);
            return;
        }

        if(!programObject->programInfo.linkedStatus)
        {
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
    }
    else
    {
        /* Disable programmable path */
        programObject = NULL;
    }

    if (programObject)
    {
        if (gc->shaderProgram.samplerSeq != programObject->bindingInfo.samplerSeq)
        {
            /* There may be other gc modifying the program object's samplers, but we have no way to know
            ** which samplers are modified. so here we have to dirty them all.
            */
            if(gc->shaderProgram.currentShaderProgram == programObject)
            {
                gc->shaderProgram.samplerDirtyState = __GL_GLSL_VS_SAMPLER_MASK |
                                                      __GL_GLSL_PS_SAMPLER_MASK |
                                                      __GL_GLSL_GS_SAMPLER_MASK;
                gc->globalDirtyState[__GL_PROGRAM_ATTRS] |= __GL_DIRTY_GLSL_SAMPLER;
                gc->globalDirtyState[__GL_ALL_ATTRS] |= (1 << __GL_PROGRAM_ATTRS);
            }

            gc->shaderProgram.samplerSeq = programObject->bindingInfo.samplerSeq;
        }
    }

    /* If the program is currently in use, do not call dp layer to do further compile,
    ** because if it wasn't relinked before, the farther compiled result is unchanged,
    ** and if it was, the further compile is done when LinkProgram()
    */
    if(gc->shaderProgram.currentShaderProgram == programObject)
    {
        if(programObject)
        {
            GLboolean recompiled = GL_FALSE;

            (*gc->dp.useProgram)(gc, programObject, &recompiled);

            gc->shaderProgram.vertShaderEnable = programObject->programInfo.vertShaderEnable;
            gc->shaderProgram.geomShaderEnable = programObject->programInfo.geomShaderEnable;
            gc->shaderProgram.fragShaderEnable = programObject->programInfo.fragShaderEnable;
            gc->shaderProgram.geomOutputType   = programObject->programInfo.geomRealOutputType;

            if(recompiled)
            {
                __GL_SET_ATTR_DIRTY_BIT(gc,__GL_PROGRAM_ATTRS,__GL_DIRTY_GLSL_PROGRAM_SWITCH);
            }
        }
        else
        {
            gc->shaderProgram.vertShaderEnable = GL_FALSE;
            gc->shaderProgram.geomShaderEnable = GL_FALSE;
            gc->shaderProgram.fragShaderEnable = GL_FALSE;
            gc->shaderProgram.geomOutputType   = GL_TRIANGLE_STRIP;
            (*gc->dp.useProgram)(gc, NULL, NULL);
        }
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);
    __GL_INPUTMASK_CHANGED(gc);

    /* vertex or fragment shader endisable will be determined here */
    oldProgramObject = gc->shaderProgram.currentShaderProgram;

    if(oldProgramObject)
    {
        oldProgramObject->objectInfo.bindCount--;

        if(oldProgramObject->objectInfo.bindCount == 0 && oldProgramObject->programInfo.deletedStatus)
        {
            __glDeleteObject(gc, gc->shaderProgram.shared, oldProgramObject->objectInfo.id);
        }
    }

    gc->shaderProgram.currentShaderProgram = programObject;

    if(programObject)
    {
        programObject->objectInfo.bindCount++;

        gc->shaderProgram.vertShaderEnable = programObject->programInfo.vertShaderEnable;
        gc->shaderProgram.geomShaderEnable = programObject->programInfo.geomShaderEnable;
        gc->shaderProgram.fragShaderEnable = programObject->programInfo.fragShaderEnable;
        gc->shaderProgram.geomOutputType   = programObject->programInfo.geomRealOutputType;

        (*gc->dp.useProgram)(gc, programObject, NULL);
    }

    else
    {
        gc->shaderProgram.vertShaderEnable = GL_FALSE;
        gc->shaderProgram.geomShaderEnable = GL_FALSE;
        gc->shaderProgram.fragShaderEnable = GL_FALSE;
        gc->shaderProgram.geomOutputType   = GL_TRIANGLE_STRIP;

        (*gc->dp.useProgram)(gc, NULL, NULL);

        __GL_SET_ATTR_DIRTY_BIT(gc,__GL_PROGRAM_ATTRS,__GL_DIRTY_VERTEX_PROGRAM_SWITCH);
        __GL_SET_ATTR_DIRTY_BIT(gc,__GL_PROGRAM_ATTRS,__GL_DIRTY_FRAGMENT_PROGRAM_SWITCH);
    }

    __GL_SET_ATTR_DIRTY_BIT(gc,__GL_PROGRAM_ATTRS,__GL_DIRTY_GLSL_PROGRAM_SWITCH);
}

GLvoid APIENTRY __glim_ValidateProgram( GLuint program)
{
    __GLshaderProgramObject * programObject = NULL;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ValidateProgram", DT_GLuint, program, DT_GLnull);
#endif

    if(program <= 0) {
        return;
    }

    GL_ASSERT(gc->shaderProgram.shared);
    programObject = (__GLshaderProgramObject *)__glGetObject(gc, gc->shaderProgram.shared, program);
    if(!programObject) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    else if(programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    (*gc->dp.validateShaderProgram)(gc, programObject);

    /* Check texture enable dimension conflict */
    if(__glSLangCheckTextureConflict(gc, programObject))
    {
        programObject->programInfo.invalidFlag |= GLSL_INVALID_TEX_BIT;
        return;
    }
    else
    {
        programObject->programInfo.invalidFlag &= ~GLSL_INVALID_TEX_BIT;
    }
}

GLvoid APIENTRY __glim_DeleteProgram( GLuint program)
{
    __GLshaderProgramObject * programObject;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_DeleteProgram", DT_GLuint, program, DT_GLnull);
#endif

    if(program <= 0) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    GL_ASSERT(gc->shaderProgram.shared);
    programObject = (__GLshaderProgramObject *)__glGetObject(gc, gc->shaderProgram.shared, program);
    if(!programObject) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    if(programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    __glDeleteObject(gc, gc->shaderProgram.shared, programObject->objectInfo.id);
}

GLboolean APIENTRY __glim_IsShader( GLuint shader)
{
    __GLobjectInfo * object;

    __GL_SETUP_NOT_IN_BEGIN_RET(0);

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_IsShader", DT_GLuint, shader, DT_GLnull);
#endif

    if(shader <= 0) {
        return GL_FALSE;
    }

    GL_ASSERT(gc->shaderProgram.shared);
    object = (__GLobjectInfo *)__glGetObject(gc, gc->shaderProgram.shared, shader);
    if(!object) {
        return GL_FALSE;
    }

    if(object->objectType == __GL_SHADER_OBJECT_TYPE) {
        return GL_TRUE;
    }
    else {
        return GL_FALSE;
    }
}

GLvoid APIENTRY __glim_GetShaderiv( GLuint shader, GLenum pname, GLint *params)
{
    __GLshaderObject * shaderObject;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetShaderiv", DT_GLuint, shader, DT_GLenum, pname, DT_GLint_ptr, params, DT_GLnull);
#endif

    if((shader <= 0) || (params == NULL)) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    shaderObject = (__GLshaderObject *)__glGetObject(gc, gc->shaderProgram.shared, shader);
    if(!shaderObject) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    else if(shaderObject->objectInfo.objectType != __GL_SHADER_OBJECT_TYPE) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    switch(pname)
    {
    case GL_SHADER_TYPE:
        *params = shaderObject->shaderInfo.shaderType;
        break;

    case GL_DELETE_STATUS:
        *params = shaderObject->shaderInfo.deleteStatus;
        break;

    case GL_COMPILE_STATUS:
        *params = shaderObject->shaderInfo.compiledStatus;
        break;

    case GL_INFO_LOG_LENGTH:
        *params = (GLint)strlen((const char *)shaderObject->shaderInfo.compiledLog);
        break;

    case GL_SHADER_SOURCE_LENGTH:
        *params = (GLint)shaderObject->shaderInfo.sourceSize + 1;
        break;

    default:
        __glSetError(GL_INVALID_ENUM);
        break;
    }
}

GLvoid APIENTRY __glim_GetShaderSource( GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source)
{
    __GLshaderObject * shaderObject = NULL;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetShaderSource", DT_GLuint, shader, DT_GLsizei, bufSize, DT_GLsizei_ptr, length, DT_GLbyte_ptr, source, DT_GLnull);
#endif

    if((shader <= 0) || (source == NULL))
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    GL_ASSERT(gc->shaderProgram.shared);

    shaderObject = (__GLshaderObject *)__glGetObject(gc, gc->shaderProgram.shared, shader);
    if(!shaderObject)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if(shaderObject->objectInfo.objectType != __GL_SHADER_OBJECT_TYPE)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    strncpy((char *)source, (const char *)shaderObject->shaderInfo.source, shaderObject->shaderInfo.sourceSize);
}

GLvoid APIENTRY __glim_GetShaderInfoLog( GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
    __GLshaderObject * shaderObject = NULL;
    GLint len = 0;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetShaderInfoLog", DT_GLuint, shader, DT_GLsizei, bufSize, DT_GLsizei_ptr, length, DT_GLbyte_ptr, infoLog, DT_GLnull);
#endif

    if((shader <= 0) || (infoLog == NULL)) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if(bufSize <= 0)
        return;

    GL_ASSERT(gc->shaderProgram.shared);

    shaderObject = (__GLshaderObject *)__glGetObject(gc, gc->shaderProgram.shared, shader);
    if(!shaderObject) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    else if(shaderObject->objectInfo.objectType != __GL_SHADER_OBJECT_TYPE) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    /* According to spec, the returned string should be null-terminated, but "length" returned
    excluded null-terminator */
    len = __GL_MIN((GLsizei)strlen((const char *)shaderObject->shaderInfo.compiledLog), bufSize-1);
    if(length)
    {
        *length = len;
    }

    strncpy((char *)infoLog, (const char *)shaderObject->shaderInfo.compiledLog, len + 1);
}


GLboolean APIENTRY __glim_IsProgram( GLuint program)
{
    __GLobjectInfo *object;

    __GL_SETUP_NOT_IN_BEGIN_RET(0);

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_IsProgram", DT_GLuint, program, DT_GLnull);
#endif

    if(program <= 0) {
        return GL_FALSE;
    }

    GL_ASSERT(gc->shaderProgram.shared);
    object = (__GLobjectInfo *)__glGetObject(gc, gc->shaderProgram.shared, program);
    if(!object) {
        return __glIsNameDefined(gc,gc->shaderProgram.shared,program);
    }

    if(object->objectType == __GL_PROGRAM_OBJECT_TYPE) {
        return GL_TRUE;
    }
    else {
        return GL_FALSE;
    }
}

GLvoid APIENTRY __glim_GetProgramiv( GLuint program, GLenum pname, GLint *params)
{
    __GLshaderProgramObject * programObject;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetProgramiv", DT_GLuint, program, DT_GLenum, pname, DT_GLint_ptr, params, DT_GLnull);
#endif

    if((program <= 0) || (params == NULL)) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    GL_ASSERT(gc->shaderProgram.shared);

    programObject = (__GLshaderProgramObject *)__glGetObject(gc, gc->shaderProgram.shared, program);
    if(!programObject) {
        __glim_GetProgramivARB(program, pname, params);
        return;
    }
    else if(programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    switch(pname)
    {
    case GL_DELETE_STATUS:
        *params = programObject->programInfo.deletedStatus;
        break;

    case GL_LINK_STATUS:
        *params = programObject->programInfo.linkedStatus;
        break;

    case GL_VALIDATE_STATUS:
        *params = programObject->programInfo.invalidFlag ? 0 : 1;
        break;

    case GL_INFO_LOG_LENGTH:
        if (programObject->programInfo.infoLog)
        {
            *params = strlen((const char *)programObject->programInfo.infoLog);
        }
        break;

    case GL_ATTACHED_SHADERS:
        *params = programObject->programInfo.count;
        break;

    case GL_ACTIVE_ATTRIBUTES:
        *params = programObject->bindingInfo.numActiveAttrib;
        break;

    case GL_ACTIVE_ATTRIBUTE_MAX_LENGTH:
        *params = programObject->bindingInfo.maxActiveAttribNameLength;
        break;

    case GL_ACTIVE_UNIFORMS:
        *params = programObject->bindingInfo.numActiveUniform;
        break;

    case GL_ACTIVE_UNIFORM_MAX_LENGTH:
        *params = programObject->bindingInfo.maxActiveUniformLength;
        break;

#if GL_EXT_geometry_shader4
    case GL_GEOMETRY_VERTICES_OUT_EXT:
        *params = programObject->programInfo.geomVerticesOut;
        break;
    case GL_GEOMETRY_INPUT_TYPE_EXT:
        *params = programObject->programInfo.geomInputType;
        break;
    case GL_GEOMETRY_OUTPUT_TYPE_EXT:
        *params = programObject->programInfo.geomOutputType;
        break;
#endif

    default:
        __glSetError(GL_INVALID_ENUM);
        break;
    }
}

GLvoid APIENTRY __glim_GetProgramInfoLog( GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
    __GLshaderProgramObject * programObject = NULL;
    GLint len = 0;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetProgramInfoLog", DT_GLuint, program, DT_GLsizei, bufSize, DT_GLsizei_ptr, length, DT_GLbyte_ptr,infoLog, DT_GLnull);
#endif

    if((program <= 0) || (infoLog == NULL)) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    GL_ASSERT(gc->shaderProgram.shared);
    programObject = (__GLshaderProgramObject *)__glGetObject(gc, gc->shaderProgram.shared, program);
    if(!programObject) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    else if(programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    if(bufSize <= 0)
        return;

    /* According to spec, the returned string should be null-terminated, but "length" returned
    excluded null-terminator */
    if(programObject->programInfo.infoLog)
    {
        len = __GL_MIN((GLsizei)strlen((const char *)programObject->programInfo.infoLog), bufSize-1);
    }
    else
    {
        len = 0;
    }
    if (len > 0 && infoLog)
    {
        strncpy((char *)infoLog, (const char *)programObject->programInfo.infoLog, len+1);
    }
    if(length)
    {
        *length = len;
    }
}

GLvoid APIENTRY __glim_GetAttachedShaders( GLuint program, GLsizei maxCount, GLsizei * count, GLuint *shader)
{
    __GLshaderProgramObject * programObject;
    GLint i;
    GLint activeCount;
    GLint index;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetAttachedShaders", DT_GLuint, program, DT_GLsizei, maxCount, DT_GLsizei_ptr, count, DT_GLuint_ptr, shader, DT_GLnull);
#endif

    if((program <= 0) || (!shader)) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    GL_ASSERT(gc->shaderProgram.shared);

    programObject = (__GLshaderProgramObject *)__glGetObject(gc, gc->shaderProgram.shared, program);
    if(!programObject) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    else if(programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    activeCount = __GL_MIN((GLint)programObject->programInfo.count, maxCount);

    index = 0;
    i = 0;

    while(index < activeCount)
    {
        if(programObject->programInfo.attachedShaders[i])
        {
            *shader = programObject->programInfo.attachedShaders[i]->objectInfo.id;
            shader++;
            index++;
        }
        i++;
    }

    if(count) {
        *count = activeCount;
    }
}


GLvoid APIENTRY __glim_GetActiveAttrib( GLuint program, GLuint index, GLsizei bufSize,
                                       GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
    __GLshaderProgramObject * programObject;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetActiveAttrib", DT_GLuint, program, DT_GLuint, index, DT_GLsizei, bufSize,
        DT_GLsizei_ptr, length, DT_GLint_ptr, size, DT_GLenum_ptr, type, DT_GLbyte_ptr,name, DT_GLnull);
#endif

    if((program <= 0) || (size == NULL) || (type == NULL) || (name == NULL) || (bufSize <= 0))
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    GL_ASSERT(gc->shaderProgram.shared);

    programObject = (__GLshaderProgramObject *)__glGetObject(gc, gc->shaderProgram.shared, program);
    if(!programObject) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    else if(programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    if (index >= programObject->bindingInfo.numActiveAttrib) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    if(!programObject->programInfo.linkedStatus)
    {
        if(length)
        {
            *length = 0;
        }

        if(name)
        {
            *name = '\0';
        }

        return;
    }
    (*gc->dp.getActiveAttribute)(gc, programObject, index, bufSize, length, size, type, (char *)name);
}

GLint APIENTRY __glim_GetAttribLocation( GLuint program, const GLchar * name)
{
    __GLshaderProgramObject * programObject;

    __GL_SETUP_NOT_IN_BEGIN_RET(-1);

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetAttribLocation",DT_GLuint, program, DT_GLbyte_ptr, name, DT_GLnull);
#endif

    if((program <= 0) || (name == NULL)) {
        __glSetError(GL_INVALID_VALUE);
        return -1;
    }
    else if(!strncmp((const char *)name, "gl_", 3)) {
        __glSetError(GL_INVALID_OPERATION);
        return -1;
    }

    GL_ASSERT(gc->shaderProgram.shared);

    programObject = (__GLshaderProgramObject *)__glGetObject(gc, gc->shaderProgram.shared, program);
    if(!programObject) {
        __glSetError(GL_INVALID_VALUE);
        return -1;
    }
    else if(programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE) {
        __glSetError(GL_INVALID_OPERATION);
        return -1;
    }
    else if(!programObject->programInfo.linkedStatus) {
        __glSetError(GL_INVALID_VALUE);
        return -1;
    }

    return (*gc->dp.getAttributeLocation)(gc, programObject, name);
}

GLvoid APIENTRY __glim_BindAttribLocation( GLuint program, GLuint index, const GLchar * name)
{
    __GLshaderProgramObject * programObject;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_BindAttribLocation", DT_GLuint, program, DT_GLuint, index,DT_GLbyte_ptr ,name, DT_GLnull);
#endif

    if((program <= 0) || (name == NULL) || (index > __GL_MAX_PROGRAM_VERTEX_ATTRIBUTES)) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    else if(!strncmp((const char *)name, "gl_", 3)) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    GL_ASSERT(gc->shaderProgram.shared);

    programObject = (__GLshaderProgramObject *)__glGetObject(gc, gc->shaderProgram.shared, program);
    if(!programObject) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    else if(programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    if (!(*gc->dp.bindAttributeLocation)(gc, programObject, index, name))
    {
        __glSetError(GL_INVALID_VALUE);
    }
}

GLvoid APIENTRY __glim_GetActiveUniform(GLuint program, GLuint index, GLsizei bufSize,
                                        GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
    __GLshaderProgramObject* programObject;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetActiveUniform", DT_GLuint, program, DT_GLuint, index, DT_GLsizei, bufSize,
        DT_GLsizei_ptr, length, DT_GLint_ptr, size, DT_GLenum_ptr, type, DT_GLbyte_ptr , name, DT_GLnull);
#endif

    if((program <= 0) || (name == NULL) || (size == NULL) || (type == NULL) || (bufSize <= 0))
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    GL_ASSERT(gc->shaderProgram.shared);
    programObject = (__GLshaderProgramObject *)__glGetObject(gc, gc->shaderProgram.shared, program);
    if(!programObject) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    else if(programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    if(!programObject->programInfo.linkedStatus)
    {
        if(length)
        {
            *length = 0;
        }

        if(name)
        {
            *name = '\0';
        }

        return;
    }

    if(index >= programObject->bindingInfo.numActiveUniform) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    (*gc->dp.getActiveUniform)(gc, programObject, index, bufSize, length, size, type, (char *)name);
}


GLint APIENTRY __glim_GetUniformLocation( GLuint program, const GLchar *name)
{
    __GLshaderProgramObject * programObject;
    GLint location;
    GLuint nameLen = 0;
    GLuint arrayIdx = 0 ;
    GLboolean bArray = GL_TRUE;

    __GL_SETUP_NOT_IN_BEGIN_RET(-1);

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetUniformLocation", DT_GLuint, program, DT_GLbyte_ptr, name, DT_GLnull);
#endif

    if((program <= 0) || (name == NULL)) {
        __glSetError(GL_INVALID_VALUE);
        return -1;
    }
    else if(!strncmp((const char *)name, "gl_", 3)) {
        __glSetError(GL_INVALID_VALUE);
        return -1;
    }

    GL_ASSERT(gc->shaderProgram.shared);
    programObject = (__GLshaderProgramObject *)__glGetObject(gc, gc->shaderProgram.shared, program);
    if (!programObject->programInfo.linkedStatus) {
        __glSetError(GL_INVALID_OPERATION);
        return -1;
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE) {
        __glSetError(GL_INVALID_OPERATION);
        return -1;
    }

    if(!programObject->programInfo.linkedStatus)
    {
        return -1;
    }

    if(!__glParseUnifromName(name, &nameLen, &arrayIdx, &bArray))
    {
        /* illegal name */
        return -1;
    }

    (*gc->dp.getUniformLocation)(gc, programObject, name, nameLen, arrayIdx, bArray, &location);
    return location;
}

GLvoid APIENTRY __glim_Uniform1f( GLint location, GLfloat x)
{
    __GLshaderProgramObject* programObject = NULL;
    GLint error;


    GLuint itemIndex = 0;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Uniform1f", DT_GLint, location, DT_GLfloat, x, DT_GLnull);
#endif

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = gc->shaderProgram.currentShaderProgram;
    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( location & __GL_LOCATION_SAMPLER_MASK)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    error = (*gc->dp.uniforms)(gc, programObject, location, GL_FLOAT, 1, &x, GL_FALSE);

    if (error != 0) {
        __glSetError(error);
    }
}

GLvoid APIENTRY __glim_Uniform2f( GLint location, GLfloat x, GLfloat y)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;
    GLint error;
    GLfloat v[2];


    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Uniform2f", DT_GLint, location, DT_GLfloat, x, DT_GLfloat, y, DT_GLnull);
#endif

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = gc->shaderProgram.currentShaderProgram;
    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( location & __GL_LOCATION_SAMPLER_MASK)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    v[0] = x;
    v[1] = y;

    error = (*gc->dp.uniforms)(gc, programObject, location, GL_FLOAT_VEC2, 1, v, GL_FALSE);

    if (error != 0) {
        __glSetError(error);
    }
}

GLvoid APIENTRY __glim_Uniform3f( GLint location, GLfloat x, GLfloat y, GLfloat z)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;
    GLint error;
    GLfloat v[3];


    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Uniform3f", DT_GLint, location, DT_GLfloat, x, DT_GLfloat, y, DT_GLfloat, z, DT_GLnull);
#endif

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = gc->shaderProgram.currentShaderProgram;
    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( location & __GL_LOCATION_SAMPLER_MASK)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    v[0] = x;
    v[1] = y;
    v[2] = z;

    error = (*gc->dp.uniforms)(gc, programObject, location, GL_FLOAT_VEC3, 1, v, GL_FALSE);

    if (error != 0) {
        __glSetError(error);
    }
}


GLvoid APIENTRY __glim_Uniform4f( GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;
    GLint error;
    GLfloat v[4];

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Uniform4f", DT_GLint, location, DT_GLfloat, x, DT_GLfloat, y, DT_GLfloat, z, DT_GLfloat, w, DT_GLnull);
#endif

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = gc->shaderProgram.currentShaderProgram;
    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( location & __GL_LOCATION_SAMPLER_MASK)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    v[0] = x;
    v[1] = y;
    v[2] = z;
    v[3] = w;

    error = (*gc->dp.uniforms)(gc, programObject, location, GL_FLOAT_VEC4, 1, v, GL_FALSE);

    if (error != 0) {
        __glSetError(error);
    }
}


GLvoid APIENTRY __glim_Uniform1i( GLint location, GLint x)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;
    GLint error;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Uniform1i", DT_GLint, location, DT_GLint, x, DT_GLnull);
#endif

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = gc->shaderProgram.currentShaderProgram;
    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( location & __GL_LOCATION_SAMPLER_MASK)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    error = (*gc->dp.uniforms)(gc, programObject, location, GL_INT, 1, &x, GL_FALSE);

    if (error != 0) {
        __glSetError(error);
    }
}

GLvoid APIENTRY __glim_Uniform2i( GLint location, GLint x, GLint y)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;
    GLint iv[2];
    GLint error;


    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Uniform2i", DT_GLint, location, DT_GLint, x, DT_GLint, y, DT_GLnull);
#endif

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = gc->shaderProgram.currentShaderProgram;
    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( location & __GL_LOCATION_SAMPLER_MASK)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    iv[0] = x;
    iv[1] = y;

    error = (*gc->dp.uniforms)(gc, programObject, location, GL_INT_VEC2, 1, iv, GL_FALSE);

    if (error != 0) {
        __glSetError(error);
    }
}

GLvoid APIENTRY __glim_Uniform3i( GLint location, GLint x, GLint y, GLint z)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;
    GLint iv[3];
    GLint error;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Uniform3i", DT_GLint, location, DT_GLint, x, DT_GLint, y, DT_GLint, z, DT_GLnull);
#endif

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = gc->shaderProgram.currentShaderProgram;
    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( location & __GL_LOCATION_SAMPLER_MASK)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    iv[0] = x;
    iv[1] = y;
    iv[2] = z;

    error = (*gc->dp.uniforms)(gc, programObject, location, GL_INT_VEC3, 1, iv, GL_FALSE);

    if (error != 0) {
        __glSetError(error);
    }
}

GLvoid APIENTRY __glim_Uniform4i( GLint location, GLint x, GLint y, GLint z, GLint w)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;
    GLint iv[4];
    GLint error;


    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Uniform4i", DT_GLint, location, DT_GLint, x, DT_GLint, y, DT_GLint, z, DT_GLint, w, DT_GLnull);
#endif

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = gc->shaderProgram.currentShaderProgram;
    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( location & __GL_LOCATION_SAMPLER_MASK)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    iv[0] = x;
    iv[1] = y;
    iv[2] = z;
    iv[3] = w;

    error = (*gc->dp.uniforms)(gc, programObject, location, GL_INT_VEC4, 1, iv, GL_FALSE);

    if (error != 0) {
        __glSetError(error);
    }
}

GLvoid APIENTRY __glim_Uniform1fv( GLint location, GLsizei count, const GLfloat * v)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;
    GLint error;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Uniform1fv", DT_GLint, location, DT_GLsizei, count, DT_GLfloat_ptr, v, DT_GLnull);
#endif

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = gc->shaderProgram.currentShaderProgram;
    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( location & __GL_LOCATION_SAMPLER_MASK)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    error = (*gc->dp.uniforms)(gc, programObject, location, GL_FLOAT, count, v, GL_FALSE);

    if (error != 0) {
        __glSetError(error);
    }
}

GLvoid APIENTRY __glim_Uniform2fv( GLint location, GLsizei count, const GLfloat *v)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;
    GLint error;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Uniform2fv", DT_GLint, location, DT_GLsizei, count, DT_GLfloat_ptr, v, DT_GLnull);
#endif

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = gc->shaderProgram.currentShaderProgram;
    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( location & __GL_LOCATION_SAMPLER_MASK)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    error = (*gc->dp.uniforms)(gc, programObject, location, GL_FLOAT_VEC2, count, v, GL_FALSE);

    if (error != 0) {
        __glSetError(error);
    }
}

GLvoid APIENTRY __glim_Uniform3fv( GLint location, GLsizei count, const GLfloat *v)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;
    GLint error;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Uniform3fv", DT_GLint, location, DT_GLsizei, count, DT_GLfloat_ptr, v, DT_GLnull);
#endif

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = gc->shaderProgram.currentShaderProgram;
    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( location & __GL_LOCATION_SAMPLER_MASK)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    error = (*gc->dp.uniforms)(gc, programObject, location, GL_FLOAT_VEC3, count, v, GL_FALSE);

    if (error != 0) {
        __glSetError(error);
    }
}

GLvoid APIENTRY __glim_Uniform4fv( GLint location, GLsizei count, const GLfloat *v)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;
    GLint error;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Uniform4fv", DT_GLint, location, DT_GLsizei, count, DT_GLfloat_ptr, v, DT_GLnull);
#endif

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = gc->shaderProgram.currentShaderProgram;
    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( location & __GL_LOCATION_SAMPLER_MASK)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    error = (*gc->dp.uniforms)(gc, programObject, location, GL_FLOAT_VEC4, count, v, GL_FALSE);

    if (error != 0) {
        __glSetError(error);
    }
}

GLvoid APIENTRY __glim_Uniform1iv( GLint location, GLsizei count, const GLint *v)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;
    GLint error;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Uniform1iv", DT_GLint, location, DT_GLsizei, count, DT_GLint_ptr, v, DT_GLnull);
#endif

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = gc->shaderProgram.currentShaderProgram;
    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    error = (*gc->dp.uniforms)(gc, programObject, location, GL_INT, count, v, GL_FALSE);

    if (error != 0) {
        __glSetError(error);
    }
}

GLvoid APIENTRY __glim_Uniform2iv( GLint location, GLsizei count, const GLint *v)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;
    GLint error;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Uniform2iv", DT_GLint, location, DT_GLsizei, count, DT_GLint_ptr, v, DT_GLnull);
#endif

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = gc->shaderProgram.currentShaderProgram;
    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( location & __GL_LOCATION_SAMPLER_MASK)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    error = (*gc->dp.uniforms)(gc, programObject, location, GL_INT_VEC2, count, v, GL_FALSE);

    if (error != 0) {
        __glSetError(error);
    }
}

GLvoid APIENTRY __glim_Uniform3iv( GLint location, GLsizei count, const GLint *v)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;
    GLint error;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Uniform3iv", DT_GLint, location, DT_GLsizei, count, DT_GLint_ptr, v, DT_GLnull);
#endif

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = gc->shaderProgram.currentShaderProgram;
    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( location & __GL_LOCATION_SAMPLER_MASK)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    error = (*gc->dp.uniforms)(gc, programObject, location, GL_INT_VEC3, count, v, GL_FALSE);

    if (error != 0) {
        __glSetError(error);
    }
}

GLvoid APIENTRY __glim_Uniform4iv( GLint location, GLsizei count, const GLint *v)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;
    GLint error;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Uniform4iv", DT_GLint, location, DT_GLsizei, count, DT_GLint_ptr, v, DT_GLnull);
#endif

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = gc->shaderProgram.currentShaderProgram;
    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( location & __GL_LOCATION_SAMPLER_MASK)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    error = (*gc->dp.uniforms)(gc, programObject, location, GL_INT_VEC4, count, v, GL_FALSE);

    if (error != 0) {
        __glSetError(error);
    }
}

GLvoid APIENTRY __glim_UniformMatrix2fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat *v)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;
    GLint error;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_UniformMatrix2fv",DT_GLint, location, DT_GLsizei, count, DT_GLboolean, transpose, DT_GLfloat_ptr, v, DT_GLnull);
#endif

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = gc->shaderProgram.currentShaderProgram;
    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( location & __GL_LOCATION_SAMPLER_MASK)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }


    error = (*gc->dp.uniforms)(gc, programObject, location, GL_FLOAT_MAT2, count, v, transpose);

    if (error != 0) {
        __glSetError(error);
    }
}

GLvoid APIENTRY __glim_UniformMatrix2x3fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat *v)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;
    GLint error;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_UniformMatrix2x3fv",DT_GLint, location, DT_GLsizei, count, DT_GLboolean, transpose, DT_GLfloat_ptr, v, DT_GLnull);
#endif

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = gc->shaderProgram.currentShaderProgram;
    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( location & __GL_LOCATION_SAMPLER_MASK)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    error = (*gc->dp.uniforms)(gc, programObject, location, GL_FLOAT_MAT2x3, count, v, transpose);

    if (error != 0) {
        __glSetError(error);
    }
}

GLvoid APIENTRY __glim_UniformMatrix2x4fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat *v)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;
    GLint error;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_UniformMatrix2x4fv",DT_GLint, location, DT_GLsizei, count, DT_GLboolean, transpose, DT_GLfloat_ptr, v, DT_GLnull);
#endif

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = gc->shaderProgram.currentShaderProgram;
    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( location & __GL_LOCATION_SAMPLER_MASK)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    error = (*gc->dp.uniforms)(gc, programObject, location, GL_FLOAT_MAT2x4, count, v, transpose);

    if (error != 0) {
        __glSetError(error);
    }
}


GLvoid APIENTRY __glim_UniformMatrix3fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat *v)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;
    GLint error;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_UniformMatrix3fv",DT_GLint, location, DT_GLsizei, count, DT_GLboolean, transpose, DT_GLfloat_ptr, v, DT_GLnull);
#endif

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = gc->shaderProgram.currentShaderProgram;
    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( location & __GL_LOCATION_SAMPLER_MASK)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    error = (*gc->dp.uniforms)(gc, programObject, location, GL_FLOAT_MAT3, count, v, transpose);

    if (error != 0) {
        __glSetError(error);
    }
}


GLvoid APIENTRY __glim_UniformMatrix3x2fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat *v)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;
    GLint error;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_UniformMatrix3x2fv",DT_GLint, location, DT_GLsizei, count, DT_GLboolean, transpose, DT_GLfloat_ptr, v, DT_GLnull);
#endif

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = gc->shaderProgram.currentShaderProgram;
    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( location & __GL_LOCATION_SAMPLER_MASK)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    error = (*gc->dp.uniforms)(gc, programObject, location, GL_FLOAT_MAT3x2, count, v, transpose);

    if (error != 0) {
        __glSetError(error);
    }
}

GLvoid APIENTRY __glim_UniformMatrix3x4fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat *v)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;
    GLint error;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_UniformMatrix3x4fv",DT_GLint, location, DT_GLsizei, count, DT_GLboolean, transpose, DT_GLfloat_ptr, v, DT_GLnull);
#endif

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = gc->shaderProgram.currentShaderProgram;
    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( location & __GL_LOCATION_SAMPLER_MASK)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    error = (*gc->dp.uniforms)(gc, programObject, location, GL_FLOAT_MAT3x2, count, v, transpose);

    if (error != 0) {
        __glSetError(error);
    }
}


GLvoid APIENTRY __glim_UniformMatrix4fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat * v)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;
    GLint error;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_UniformMatrix4fv",DT_GLint, location, DT_GLsizei, count, DT_GLboolean, transpose, DT_GLfloat_ptr, v, DT_GLnull);
#endif

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = gc->shaderProgram.currentShaderProgram;
    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( location & __GL_LOCATION_SAMPLER_MASK)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    error = (*gc->dp.uniforms)(gc, programObject, location, GL_FLOAT_MAT4, count, v, transpose);

    if (error != 0) {
        __glSetError(error);
    }
}

GLvoid APIENTRY __glim_UniformMatrix4x2fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat * v)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;
    GLint error;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_UniformMatrix4x2fv",DT_GLint, location, DT_GLsizei, count, DT_GLboolean, transpose, DT_GLfloat_ptr, v, DT_GLnull);
#endif

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = gc->shaderProgram.currentShaderProgram;
    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( location & __GL_LOCATION_SAMPLER_MASK)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    error = (*gc->dp.uniforms)(gc, programObject, location, GL_FLOAT_MAT4x2, count, v, transpose);

    if (error != 0) {
        __glSetError(error);
    }
}

GLvoid APIENTRY __glim_UniformMatrix4x3fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat * v)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;
    GLint error;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_UniformMatrix4x3fv",DT_GLint, location, DT_GLsizei, count, DT_GLboolean, transpose, DT_GLfloat_ptr, v, DT_GLnull);
#endif

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = gc->shaderProgram.currentShaderProgram;
    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( location & __GL_LOCATION_SAMPLER_MASK)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    error = (*gc->dp.uniforms)(gc, programObject, location, GL_FLOAT_MAT4x3, count, v, transpose);

    if (error != 0) {
        __glSetError(error);
    }
}


GLvoid APIENTRY __glim_GetUniformfv( GLuint program, GLint location, GLfloat *params)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetUniformfv",DT_GLuint, program, DT_GLint, location, DT_GLfloat_ptr, params, DT_GLnull);
#endif

    if((program <= 0) || (params == NULL)) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    GL_ASSERT(gc->shaderProgram.shared);
    programObject = (__GLshaderProgramObject *)__glGetObject(gc, gc->shaderProgram.shared, program);
    if(!programObject) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    else if(programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }
    else if(!programObject->programInfo.linkedStatus)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    if ( !(location & __GL_LOCATION_UNIFORM_MASK) )
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    if (!(*gc->dp.getUniforms)(gc, programObject, location, GL_FLOAT, params))
    {
        __glSetError(GL_INVALID_OPERATION);
    }
}

GLvoid APIENTRY __glim_GetUniformiv( GLuint program, GLint location, GLint *params)
{
    __GLshaderProgramObject* programObject = NULL;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetUniformiv",DT_GLuint, program, DT_GLint, location, DT_GLint_ptr, params, DT_GLnull);
#endif

    if((program <= 0) || (params == NULL)) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    GL_ASSERT(gc->shaderProgram.shared);

    programObject = (__GLshaderProgramObject *)__glGetObject(gc, gc->shaderProgram.shared, program);
    if(!programObject) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    else if(programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }
    else if(!programObject->programInfo.linkedStatus)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    if (!(*gc->dp.getUniforms)(gc, programObject, location, GL_INT, params))
    {
        __glSetError(GL_INVALID_OPERATION);
    }
}

GLvoid APIENTRY __glim_GetVertexAttribdv( GLuint index, GLenum pname, GLdouble *params)
{
    __GLvertexArray *pArray;
    __GLvertexArrayState * pVertexArrayState;
    GLuint bit;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetVertexAttribdv",DT_GLuint, index, DT_GLenum, pname, DT_GLdouble_ptr, params, DT_GLnull);
#endif

    if((index >= __GL_MAX_PROGRAM_VERTEX_ATTRIBUTES) || (params == NULL))
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    pArray = &gc->clientState.vertexArray.currentArrays[__GL_VARRAY_ATT0_INDEX + index];
    pVertexArrayState = &gc->clientState.vertexArray;

    switch(pname)
    {
    case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
        bit = __GL_VARRAY_ATT0 << index;
        *params = (pVertexArrayState->arrayEnabled & bit) ? 1 : 0;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_SIZE:
        *params = pArray->size;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
        *params = pArray->stride;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_TYPE:
        *params = pArray->type;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
        *params = pArray->normalized;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_INTEGER_EXT:
        *params = pArray->integer;
        break;

    case GL_CURRENT_VERTEX_ATTRIB:
        if(index > 0)
        {
            *params++ = gc->state.current.attribute[index].x;
            *params++ = gc->state.current.attribute[index].y;
            *params++ = gc->state.current.attribute[index].z;
            *params++ = gc->state.current.attribute[index].w;
        }
        else
            __glSetError(GL_INVALID_OPERATION);
        break;

    default:
        __glSetError(GL_INVALID_VALUE);
        break;
    }
}

GLvoid APIENTRY __glim_GetVertexAttribfv( GLuint index, GLenum pname, GLfloat *params)
{
    __GLvertexArray *pArray;
    __GLvertexArrayState * pVertexArrayState;
    GLuint bit;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetVertexAttribfv",DT_GLuint, index, DT_GLenum, pname, DT_GLfloat_ptr, params, DT_GLnull);
#endif

    if((index >= __GL_MAX_PROGRAM_VERTEX_ATTRIBUTES) || (params == NULL))
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    pArray = &gc->clientState.vertexArray.currentArrays[__GL_VARRAY_ATT0_INDEX + index];
    pVertexArrayState = &gc->clientState.vertexArray;

    switch(pname)
    {
    case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
        bit = __GL_VARRAY_ATT0 << index;
        *params = (GLfloat)((pVertexArrayState->arrayEnabled & bit) ? 1 : 0);
        break;

    case GL_VERTEX_ATTRIB_ARRAY_SIZE:
        *params = (GLfloat)pArray->size;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
        *params = (GLfloat)pArray->stride;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_TYPE:
        *params = (GLfloat)pArray->type;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
        *params = (GLfloat)pArray->normalized;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_INTEGER_EXT:
        *params = (GLfloat)pArray->integer;
        break;

    case GL_CURRENT_VERTEX_ATTRIB:
        if(index > 0)
        {
            *params++ = gc->state.current.attribute[index].x;
            *params++ = gc->state.current.attribute[index].y;
            *params++ = gc->state.current.attribute[index].z;
            *params++ = gc->state.current.attribute[index].w;
        }
        else
            __glSetError(GL_INVALID_OPERATION);
        break;

    default:
        __glSetError(GL_INVALID_VALUE);
        break;
    }
}

GLvoid APIENTRY __glim_GetVertexAttribiv( GLuint index, GLenum pname, GLint *params)
{
    __GLvertexArray *pArray;
    __GLvertexArrayState * pVertexArrayState;
    GLuint bit;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetVertexAttribiv",DT_GLuint, index, DT_GLenum, pname, DT_GLint_ptr, params, DT_GLnull);
#endif

    if((index >= __GL_MAX_PROGRAM_VERTEX_ATTRIBUTES) || (params == NULL))
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    pArray = &gc->clientState.vertexArray.currentArrays[__GL_VARRAY_ATT0_INDEX + index];
    pVertexArrayState = &gc->clientState.vertexArray;

    switch(pname)
    {
    case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
        bit = __GL_VARRAY_ATT0 << index;
        *params = (pVertexArrayState->arrayEnabled & bit) ? 1 : 0;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_SIZE:
        *params = pArray->size;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
        *params = pArray->stride;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_TYPE:
        *params = pArray->type;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
        *params = pArray->normalized;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_INTEGER_EXT:
        *params = pArray->integer;
        break;

    case GL_CURRENT_VERTEX_ATTRIB:
        if(index > 0)
        {
            *params++ = (GLint) gc->state.current.attribute[index].x;
            *params++ = (GLint) gc->state.current.attribute[index].y;
            *params++ = (GLint) gc->state.current.attribute[index].z;
            *params++ = (GLint) gc->state.current.attribute[index].w;
        }
        else
            __glSetError(GL_INVALID_OPERATION);
        break;

    default:
        __glSetError(GL_INVALID_VALUE);
        break;
    }
}

#if GL_EXT_gpu_shader4

GLvoid APIENTRY __glim_Uniform1uiEXT(GLint location, GLuint x)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Uniform1uiEXT", DT_GLint, location, DT_GLuint, x, DT_GLnull);
#endif

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = gc->shaderProgram.currentShaderProgram;
    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }
}

GLvoid APIENTRY __glim_Uniform2uiEXT(GLint location, GLuint x, GLuint y)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;


    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Uniform2uiEXT", DT_GLint, location, DT_GLuint, x, DT_GLuint, y, DT_GLnull);
#endif

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = gc->shaderProgram.currentShaderProgram;
    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( location & __GL_LOCATION_SAMPLER_MASK)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }
}

GLvoid APIENTRY __glim_Uniform3uiEXT(GLint location, GLuint x, GLuint y, GLuint z)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Uniform3uiEXT", DT_GLint, location, DT_GLuint, x, DT_GLuint, y, DT_GLuint, z, DT_GLnull);
#endif

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = gc->shaderProgram.currentShaderProgram;
    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( location & __GL_LOCATION_SAMPLER_MASK)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }
}

GLvoid APIENTRY __glim_Uniform4uiEXT(GLint location, GLuint x, GLuint y, GLuint z,
                      GLuint w)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Uniform4uiEXT", DT_GLint, location, DT_GLuint, x, DT_GLuint, y, DT_GLuint, z, DT_GLuint, w, DT_GLnull);
#endif

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = gc->shaderProgram.currentShaderProgram;
    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( location & __GL_LOCATION_SAMPLER_MASK)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }
}

GLvoid APIENTRY __glim_Uniform1uivEXT(GLint location, GLsizei count, const GLuint *v)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Uniform1uivEXT", DT_GLint, location, DT_GLsizei, count, DT_GLuint_ptr, v, DT_GLnull);
#endif

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = gc->shaderProgram.currentShaderProgram;
    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }
}

GLvoid APIENTRY __glim_Uniform2uivEXT(GLint location, GLsizei count, const GLuint *v)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Uniform2uivEXT", DT_GLint, location, DT_GLsizei, count, DT_GLuint_ptr, v, DT_GLnull);
#endif

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = gc->shaderProgram.currentShaderProgram;
    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( location & __GL_LOCATION_SAMPLER_MASK)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }
}

GLvoid APIENTRY __glim_Uniform3uivEXT(GLint location, GLsizei count, const GLuint *v)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Uniform3uivEXT", DT_GLint, location, DT_GLsizei, count, DT_GLuint_ptr, v, DT_GLnull);
#endif

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = gc->shaderProgram.currentShaderProgram;
    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( location & __GL_LOCATION_SAMPLER_MASK)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }
}

GLvoid APIENTRY __glim_Uniform4uivEXT(GLint location, GLsizei count, const GLuint *v)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Uniform4iv", DT_GLint, location, DT_GLsizei, count, DT_GLuint_ptr, v, DT_GLnull);
#endif

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    programObject = gc->shaderProgram.currentShaderProgram;
    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( location & __GL_LOCATION_SAMPLER_MASK)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }
}

GLvoid APIENTRY __glim_GetUniformuivEXT(GLuint program, GLint location, GLuint *params)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetUniformiv",DT_GLuint, program, DT_GLint, location, DT_GLint_ptr, params, DT_GLnull);
#endif

    if((program <= 0) || (params == NULL)) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if(location == -1)
    {
        return;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    GL_ASSERT(gc->shaderProgram.shared);

    programObject = (__GLshaderProgramObject *)__glGetObject(gc, gc->shaderProgram.shared, program);
    if(!programObject) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    else if(programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }
    else if(!programObject->programInfo.linkedStatus)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }


    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }
}

GLvoid APIENTRY __glim_BindFragDataLocationEXT(GLuint program, GLuint colorNumber,
                                const GLbyte *name)
{
    __GLshaderProgramObject * programObject;
    GLuint i;
    __GLSLFragVaringOutItem *pFragmentVaringOutTable;
    GLuint fragmentVaringOutCount;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_BindFragDataLocationEXT", DT_GLuint, program, DT_GLuint, colorNumber,DT_GLbyte_ptr ,name, DT_GLnull);
#endif

    if((program <= 0) || (name == NULL) || (colorNumber > __GL_MAX_DRAW_BUFFERS)) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    else if(!strncmp((const char *)name, "gl_", 3)) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    GL_ASSERT(gc->shaderProgram.shared);

    programObject = (__GLshaderProgramObject *)__glGetObject(gc, gc->shaderProgram.shared, program);
    if(!programObject) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    else if(programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    pFragmentVaringOutTable = (__GLSLFragVaringOutItem*)programObject->bindingInfo.pFragmentVaringOutTable;
    fragmentVaringOutCount = programObject->bindingInfo.fragmentVaringOutTableSize;

    for (i=0; i<fragmentVaringOutCount; i++)
    {
        if (strcmp((const char *)pFragmentVaringOutTable[i].lpszFragDataName, (const char *)name) == 0)
        {
            if (pFragmentVaringOutTable[i].dwColorIdx != colorNumber)
            {
            }
        }
    }
}

GLint APIENTRY __glim_GetFragDataLocationEXT(GLuint program, const GLbyte *name)
{
    __GLshaderProgramObject * programObject;
    GLuint i;
    __GLSLFragVaringOutItem *pFragmentVaringOutTable;

    __GL_SETUP_NOT_IN_BEGIN_RET(-1);

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetFragDataLocationEXT",DT_GLuint, program, DT_GLbyte_ptr, name, DT_GLnull);
#endif

    if((program <= 0) || (name == NULL)) {
        __glSetError(GL_INVALID_VALUE);
        return -1;
    }
    else if(!strncmp((const char *)name, "gl_", 3)) {
        __glSetError(GL_INVALID_OPERATION);
        return -1;
    }

    GL_ASSERT(gc->shaderProgram.shared);

    programObject = (__GLshaderProgramObject *)__glGetObject(gc, gc->shaderProgram.shared, program);
    if(!programObject) {
        __glSetError(GL_INVALID_VALUE);
        return -1;
    }
    else if(programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE) {
        __glSetError(GL_INVALID_OPERATION);
        return -1;
    }
    else if(!programObject->programInfo.linkedStatus) {
        __glSetError(GL_INVALID_VALUE);
        return -1;
    }

    pFragmentVaringOutTable = (__GLSLFragVaringOutItem*)programObject->bindingInfo.pFragmentVaringOutTable;

    for (i=0; i<programObject->bindingInfo.fragmentVaringOutTableSize; i++)
    {
        if (strcmp((const char *)pFragmentVaringOutTable[i].lpszFragDataName, (const char *)name) == 0)
        {
            return pFragmentVaringOutTable[i].dwColorIdx;
        }
    }

    return -1;
}

GLvoid APIENTRY __glim_GetVertexAttribIivEXT(GLuint index, GLenum pname, GLint *params)
{
    __GLvertexArray *pArray;
    __GLvertexArrayState * pVertexArrayState;
    GLuint bit;
    GLuint *uparams = (GLuint *)params;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetVertexAttribIivEXT",DT_GLuint, index, DT_GLenum, pname, DT_GLint_ptr, params, DT_GLnull);
#endif

    if((index >= __GL_MAX_PROGRAM_VERTEX_ATTRIBUTES) || (params == NULL))
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    pArray = &gc->clientState.vertexArray.currentArrays[__GL_VARRAY_ATT0_INDEX + index];
    pVertexArrayState = &gc->clientState.vertexArray;

    switch(pname)
    {
    case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
        bit = __GL_VARRAY_VERTEX << index;
        *params = (pVertexArrayState->arrayEnabled & bit) ? 1 : 0;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_SIZE:
        *params = pArray->size;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
        *params = pArray->stride;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_TYPE:
        *params = pArray->type;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
        *params = pArray->normalized;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_INTEGER_EXT:
        *params = pArray->integer;
        break;

    case GL_CURRENT_VERTEX_ATTRIB:
        if(index > 0)
        {
            *uparams++ = gc->state.current.attribute[index].ix;
            *uparams++ = gc->state.current.attribute[index].iy;
            *uparams++ = gc->state.current.attribute[index].iz;
            *uparams++ = gc->state.current.attribute[index].iw;
        }
        else
            __glSetError(GL_INVALID_OPERATION);
        break;

    default:
        __glSetError(GL_INVALID_VALUE);
        break;
    }
}

GLvoid APIENTRY __glim_GetVertexAttribIuivEXT(GLuint index, GLenum pname, GLuint *params)
{
    __GLvertexArray *pArray;
    __GLvertexArrayState * pVertexArrayState;
    GLuint bit;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetVertexAttribIuivEXT",DT_GLuint, index, DT_GLenum, pname, DT_GLint_ptr, params, DT_GLnull);
#endif

    if((index >= __GL_MAX_PROGRAM_VERTEX_ATTRIBUTES) || (params == NULL))
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    pArray = &gc->clientState.vertexArray.currentArrays[__GL_VARRAY_ATT0_INDEX + index];
    pVertexArrayState = &gc->clientState.vertexArray;

    switch(pname)
    {
    case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
        bit = __GL_VARRAY_VERTEX << index;
        *params = (pVertexArrayState->arrayEnabled & bit) ? 1 : 0;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_SIZE:
        *params = pArray->size;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
        *params = pArray->stride;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_TYPE:
        *params = pArray->type;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
        *params = pArray->normalized;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_INTEGER_EXT:
        *params = pArray->integer;
        break;

    case GL_CURRENT_VERTEX_ATTRIB:
        if(index > 0)
        {
            *params++ = gc->state.current.attribute[index].ix;
            *params++ = gc->state.current.attribute[index].iy;
            *params++ = gc->state.current.attribute[index].iz;
            *params++ = gc->state.current.attribute[index].iw;
        }
        else
            __glSetError(GL_INVALID_OPERATION);
        break;

    default:
        __glSetError(GL_INVALID_VALUE);
        break;
    }
}
#endif
GLvoid APIENTRY __glim_GetVertexAttribPointerv( GLuint index, GLenum pname, GLvoid**pointer)
{
    __GLvertexArray *pArray;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetVertexAttribPointerv", DT_GLuint, index, DT_GLenum, pname, DT_GLvoid_ptr, pointer, DT_GLnull);
#endif

    if((index >= __GL_MAX_PROGRAM_VERTEX_ATTRIBUTES) || (pointer == NULL))
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    pArray = &gc->clientState.vertexArray.currentArrays[__GL_VARRAY_ATT0_INDEX +index];

    switch(pname)
    {
    case GL_VERTEX_ATTRIB_ARRAY_POINTER:
        *pointer = (GLvoid *)pArray->pointer;
        break;

    default:
        __glSetError(GL_INVALID_VALUE);
        break;
    }
}



/************************************************************************/
/* GL_ARB_shader_object APIs                                            */
/************************************************************************/
GLhandleARB APIENTRY __glim_GetHandleARB(GLenum pname)
{
    __GL_SETUP_NOT_IN_BEGIN_RET(0);

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetHandleARB", DT_GLenum, pname, DT_GLnull);
#endif

    if(pname != GL_PROGRAM_OBJECT_ARB) {
        return 0;
    }

    if(!gc->shaderProgram.currentShaderProgram) {
        return 0;
    }

    return gc->shaderProgram.currentShaderProgram->objectInfo.id;
}


GLvoid APIENTRY __glim_GetInfoLogARB(GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog)
{
    __GLobjectInfo * object = NULL;
    GLuint len = 0;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetInfoLogARB", DT_GLuint, obj, DT_GLsizei, maxLength, DT_GLsizei_ptr, length, DT_GLbyte_ptr, infoLog, DT_GLnull);
#endif

    if(obj <= 0) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    object = (__GLobjectInfo *)__glGetObject(gc, gc->shaderProgram.shared, obj);

    if(!object) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if(object->objectType == __GL_SHADER_OBJECT_TYPE)
    {
        __GLshaderObject *shaderObject = (__GLshaderObject *)object;
        /* According to spec, the returned string should be null-terminated, but "length" returned
        excluded null-terminator */
        len = __GL_MIN((GLsizei)strlen((const char *)shaderObject->shaderInfo.compiledLog), maxLength-1);
        if(length)
        {
            *length = len;
        }
        strncpy((char *)infoLog, (const char *)shaderObject->shaderInfo.compiledLog, len+1);
    }
    else if(object->objectType == __GL_PROGRAM_OBJECT_TYPE)
    {
        __GLshaderProgramObject *programObject = (__GLshaderProgramObject *)object;
        /* According to spec, the returned string should be null-terminated, but "length" returned
        excluded null-terminator */
        if(programObject->programInfo.infoLog)
        {
            len = __GL_MIN((GLsizei)strlen((const char *)programObject->programInfo.infoLog), maxLength-1);
        }
        else
        {
            len = 0;
        }
        if (len > 0 && infoLog)
        {
            strncpy((char *)infoLog, (const char *)programObject->programInfo.infoLog, len+1);
        }
        if(length)
        {
            *length = len;
        }
    }
    else
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }
}

GLvoid APIENTRY __glim_GetObjectParameterfvARB(GLhandleARB obj, GLenum pname, GLfloat *params)
{
    __GLobjectInfo * object;
    __GLshaderObject *shaderObject = NULL;
    __GLshaderProgramObject *programObject = NULL;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetObjectParameterfvARB", DT_GLuint, obj, DT_GLenum, pname, DT_GLfloat_ptr, params, DT_GLnull);
#endif

    if(obj <= 0) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    object = (__GLobjectInfo *)__glGetObject(gc, gc->shaderProgram.shared, obj);

    if(!object) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    switch (pname)
    {
    case GL_OBJECT_TYPE_ARB:
        if(object->objectType == __GL_SHADER_OBJECT_TYPE) {
            *params = (GLfloat)GL_SHADER_OBJECT_ARB;
        }
        else if(object->objectType == __GL_PROGRAM_OBJECT_TYPE) {
            *params = (GLfloat)GL_PROGRAM_OBJECT_ARB;
        }
        else {
            __glSetError(GL_INVALID_OPERATION);
        }
        break;

    case GL_OBJECT_SUBTYPE_ARB:
        if(object->objectType == __GL_SHADER_OBJECT_TYPE) {
            shaderObject = (__GLshaderObject *)object;
            *params = (GLfloat)(shaderObject->shaderInfo.shaderType);
        }
        else {
            __glSetError(GL_INVALID_OPERATION);
        }
        break;

    case GL_OBJECT_DELETE_STATUS_ARB:
        if(object->objectType == __GL_SHADER_OBJECT_TYPE) {
            shaderObject = (__GLshaderObject *)object;
            *params = shaderObject->shaderInfo.deleteStatus?1.0f:0.0f;
        }
        else if(object->objectType == __GL_PROGRAM_OBJECT_TYPE) {
            programObject = (__GLshaderProgramObject *)object;
            *params = programObject->programInfo.deletedStatus?1.0f:0.0f;
        }
        else {
            __glSetError(GL_INVALID_OPERATION);
        }
        break;

    case GL_OBJECT_LINK_STATUS_ARB:
        if(object->objectType == __GL_PROGRAM_OBJECT_TYPE) {
            programObject = (__GLshaderProgramObject *)object;
            *params = programObject->programInfo.linkedStatus?1.0f:0.0f;
        }
        else {
            __glSetError(GL_INVALID_OPERATION);
        }
        break;

    case GL_OBJECT_VALIDATE_STATUS_ARB:
        if(object->objectType == __GL_PROGRAM_OBJECT_TYPE) {
            programObject = (__GLshaderProgramObject *)object;
            *params = programObject->programInfo.invalidFlag ? 0.0f : 1.0f;
        }
        else {
            __glSetError(GL_INVALID_OPERATION);
        }
        break;

    case GL_OBJECT_COMPILE_STATUS_ARB:
        if(object->objectType == __GL_SHADER_OBJECT_TYPE) {
            shaderObject = (__GLshaderObject *)object;
            *params = shaderObject->shaderInfo.compiledStatus?1.0f:0.0f;
        }
        else {
            __glSetError(GL_INVALID_OPERATION);
        }
        break;

    case GL_OBJECT_INFO_LOG_LENGTH_ARB:
        if(object->objectType == __GL_SHADER_OBJECT_TYPE) {
            shaderObject = (__GLshaderObject *)object;
            *params = (GLfloat)strlen((const char *)shaderObject->shaderInfo.compiledLog) + 1;
        }
        else if(object->objectType == __GL_PROGRAM_OBJECT_TYPE) {
            programObject = (__GLshaderProgramObject *)object;
            if (programObject->programInfo.infoLog)
            {
                *params = (GLfloat)strlen((const char *)programObject->programInfo.infoLog) + 1;
            }
        }
        else {
            __glSetError(GL_INVALID_OPERATION);
        }
        break;

    case GL_OBJECT_ATTACHED_OBJECTS_ARB:
        if(object->objectType == __GL_PROGRAM_OBJECT_TYPE) {
            programObject = (__GLshaderProgramObject *)object;
            *params = (GLfloat)(programObject->programInfo.count);
        }
        else {
            __glSetError(GL_INVALID_OPERATION);
        }
        break;

    case GL_OBJECT_ACTIVE_UNIFORMS_ARB:
        if(object->objectType == __GL_PROGRAM_OBJECT_TYPE) {
            programObject = (__GLshaderProgramObject *)object;
            *params = (GLfloat)(programObject->bindingInfo.numActiveUniform);
        }
        else {
            __glSetError(GL_INVALID_OPERATION);
        }
        break;

    case GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB:
        if(object->objectType == __GL_PROGRAM_OBJECT_TYPE) {
            programObject = (__GLshaderProgramObject *)object;
            *params = (GLfloat)(programObject->bindingInfo.maxActiveUniformLength);
        }
        else {
            __glSetError(GL_INVALID_OPERATION);
        }
        break;

    case GL_OBJECT_SHADER_SOURCE_LENGTH_ARB:
        if(object->objectType == __GL_SHADER_OBJECT_TYPE) {
            shaderObject = (__GLshaderObject *)object;
            *params = (GLfloat)strlen((const char *)shaderObject->shaderInfo.compiledLog) + 1;
        }
        else {
            __glSetError(GL_INVALID_OPERATION);
        }
        break;

        /* In OpenGL 1.5 Extensions, there are no requirement to get GL_OBJECT_ACTIVE_ATTRIBUTES
        ** or GL_ACTIVE_ATTRIBUTE_MAX_LENGTH
        */

    default:
        __glSetError(GL_INVALID_ENUM);
        break;
    }
}

GLvoid APIENTRY __glim_GetObjectParameterivARB(GLhandleARB obj, GLenum pname, GLint *params)
{
    __GLobjectInfo * object;
    __GLshaderObject *shaderObject = NULL;
    __GLshaderProgramObject *programObject = NULL;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetObjectParameterivARB", DT_GLuint, obj, DT_GLenum, pname, DT_GLint_ptr, params, DT_GLnull);
#endif

    if(obj <= 0) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    object = (__GLobjectInfo *)__glGetObject(gc, gc->shaderProgram.shared, obj);

    if(!object) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    switch (pname)
    {
    case GL_OBJECT_TYPE_ARB:
        if(object->objectType== __GL_SHADER_OBJECT_TYPE) {
            *params = GL_SHADER_OBJECT_ARB;
        }
        else if(object->objectType == __GL_PROGRAM_OBJECT_TYPE) {
            *params = GL_PROGRAM_OBJECT_ARB;
        }
        else {
            __glSetError(GL_INVALID_OPERATION);
        }
        break;

    case GL_OBJECT_SUBTYPE_ARB:
        if(object->objectType == __GL_SHADER_OBJECT_TYPE) {
            shaderObject = (__GLshaderObject *)object;
            *params = shaderObject->shaderInfo.shaderType;
        }
        else {
            __glSetError(GL_INVALID_OPERATION);
        }
        break;

    case GL_OBJECT_DELETE_STATUS_ARB:
        if(object->objectType == __GL_SHADER_OBJECT_TYPE) {
            shaderObject = (__GLshaderObject *)object;
            *params = shaderObject->shaderInfo.deleteStatus? 1:0;
        }
        else if(object->objectType == __GL_PROGRAM_OBJECT_TYPE) {
            programObject = (__GLshaderProgramObject *)object;
            *params = programObject->programInfo.deletedStatus?1:0;
        }
        else {
            __glSetError(GL_INVALID_OPERATION);
        }
        break;

    case GL_OBJECT_LINK_STATUS_ARB:
        if(object->objectType == __GL_PROGRAM_OBJECT_TYPE) {
            programObject = (__GLshaderProgramObject *)object;
            *params = programObject->programInfo.linkedStatus?1:0;
        }
        else {
            __glSetError(GL_INVALID_OPERATION);
        }
        break;

    case GL_OBJECT_VALIDATE_STATUS_ARB:
        if(object->objectType == __GL_PROGRAM_OBJECT_TYPE) {
            programObject = (__GLshaderProgramObject *)object;
            *params = programObject->programInfo.invalidFlag ? 0 : 1;
        }
        else {
            __glSetError(GL_INVALID_OPERATION);
        }
        break;

    case GL_OBJECT_COMPILE_STATUS_ARB:
        if(object->objectType == __GL_SHADER_OBJECT_TYPE) {
            shaderObject = (__GLshaderObject *)object;
            *params = shaderObject->shaderInfo.compiledStatus?1:0;
        }
        else {
            __glSetError(GL_INVALID_OPERATION);
        }
        break;

    case GL_OBJECT_INFO_LOG_LENGTH_ARB:
        if(object->objectType == __GL_SHADER_OBJECT_TYPE) {
            shaderObject = (__GLshaderObject *)object;
            *params = shaderObject->shaderInfo.deleteStatus ? 1 : 0;
        }
        else if(object->objectType == __GL_PROGRAM_OBJECT_TYPE) {
            programObject = (__GLshaderProgramObject *)object;
            *params = programObject->programInfo.deletedStatus ? 1 : 0;
        }
        else {
            __glSetError(GL_INVALID_OPERATION);
        }
        break;

    case GL_OBJECT_ATTACHED_OBJECTS_ARB:
        if(object->objectType == __GL_PROGRAM_OBJECT_TYPE) {
            programObject = (__GLshaderProgramObject *)object;
            *params = programObject->programInfo.count;
        }
        else {
            __glSetError(GL_INVALID_OPERATION);
        }
        break;

    case GL_OBJECT_ACTIVE_UNIFORMS_ARB:
        if(object->objectType == __GL_PROGRAM_OBJECT_TYPE) {
            programObject = (__GLshaderProgramObject *)object;
            *params = programObject->bindingInfo.numActiveUniform;
        }
        else {
            __glSetError(GL_INVALID_OPERATION);
        }
        break;

    case GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB:
        if(object->objectType == __GL_PROGRAM_OBJECT_TYPE) {
            programObject = (__GLshaderProgramObject *)object;
            *params = programObject->bindingInfo.maxActiveUniformLength;
        }
        else {
            __glSetError(GL_INVALID_OPERATION);
        }
        break;

    case GL_OBJECT_SHADER_SOURCE_LENGTH_ARB:
        if(object->objectType == GL_VERTEX_SHADER || object->objectType == GL_FRAGMENT_SHADER) {
            shaderObject = (__GLshaderObject *)object;
            *params = shaderObject->shaderInfo.sourceSize + 1;
        }
        else {
            __glSetError(GL_INVALID_OPERATION);
        }
        break;

        /* In OpenGL 1.5 Extensions, there are no requirement to get GL_OBJECT_ACTIVE_ATTRIBUTES
        ** or GL_ACTIVE_ATTRIBUTE_MAX_LENGTH
        */

    default:
        __glSetError(GL_INVALID_ENUM);
        break;
    }
}

GLvoid APIENTRY __glim_DeleteObjectARB(GLhandleARB obj)
{
    __GLobjectInfo * object;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_DeleteObjectARB", DT_GLuint, obj, DT_GLnull);
#endif

    if(obj <= 0) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    object = (__GLobjectInfo *)__glGetObject(gc, gc->shaderProgram.shared, obj);

    if(!object)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    switch(object->objectType){
    case __GL_SHADER_OBJECT_TYPE:
    case __GL_PROGRAM_OBJECT_TYPE:
        __glDeleteObject(gc, gc->shaderProgram.shared, object->id);
        break;
    default :
        __glSetError(GL_INVALID_OPERATION);
        break;
    }

}

/***************************************************************************/
/* GL_EXT_bindable_uniform APIs                                            */
/***************************************************************************/

GLvoid APIENTRY __glim_UniformBufferEXT(GLuint program, GLint location, GLuint buffer)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_UniformBuffer", DT_GLuint, program, DT_GLint, location, DT_GLuint, buffer, DT_GLnull);
#endif

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    if(program <= 0)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    GL_ASSERT(gc->shaderProgram.shared);
    programObject = (__GLshaderProgramObject *)__glGetObject(gc, gc->shaderProgram.shared, program);

    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( location & __GL_LOCATION_SAMPLER_MASK)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    if(!__glIsNameDefined(gc, gc->bufferObject.shared, buffer))
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    if (!programObject->programInfo.linkedStatus)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }
    /* Add code later */
}


GLint APIENTRY __glim_GetUniformBufferSizeEXT(GLuint program, GLint location)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;
    GLsizei size = 0;

    __GL_SETUP_NOT_IN_BEGIN_RET(-1);

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetUniformBufferSize", DT_GLuint, program, DT_GLint, location, DT_GLnull);
#endif

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return -1;
    }

    if(program <= 0)
    {
        __glSetError(GL_INVALID_VALUE);
        return -1;
    }

    GL_ASSERT(gc->shaderProgram.shared);
    programObject = (__GLshaderProgramObject *)__glGetObject(gc, gc->shaderProgram.shared, program);

    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return -1;
    }


    if( location & __GL_LOCATION_SAMPLER_MASK)
    {
        __glSetError(GL_INVALID_VALUE);
        return -1;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return -1;
    }

    if(!programObject->programInfo.linkedStatus)
    {
        __glSetError(GL_INVALID_VALUE);
        return -1;
    }

    /* Add code later */

    return size;
}

GLintptr APIENTRY __glim_GetUniformOffsetEXT(GLuint program, GLint location)
{
    __GLshaderProgramObject* programObject = NULL;
    GLuint itemIndex = 0;

    __GL_SETUP_NOT_IN_BEGIN_RET(-1);

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetUniformOffset", DT_GLuint, program, DT_GLint, location, DT_GLnull);
#endif

    if(location == -1)
    {
        return -1;
    }

    if(location < 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return -1;
    }

    if(program <= 0)
    {
        __glSetError(GL_INVALID_VALUE);
        return -1;
    }

    GL_ASSERT(gc->shaderProgram.shared);
    programObject = (__GLshaderProgramObject *)__glGetObject(gc, gc->shaderProgram.shared, program);

    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return -1;
    }

    if (!programObject->programInfo.linkedStatus) {
        __glSetError(GL_INVALID_OPERATION);
        return -1;
    }

    if( location & __GL_LOCATION_SAMPLER_MASK)
    {
        __glSetError(GL_INVALID_VALUE);
        return -1;
    }

    itemIndex = __GET_UNIFORM_ITEM_INDEX(location);

    if(itemIndex >= programObject->bindingInfo.numUserUnifrom)
    {
        __glSetError(GL_INVALID_OPERATION);
        return -1;
    }

    /* Add code later */
    return 0;
}

/***************************************************************************/
/*  GL_EXT_geometry_shader4 APIs                                           */
/***************************************************************************/

GLvoid APIENTRY __glim_ProgramParameteriEXT(GLuint program, GLenum pname, GLint value)
{
    __GLshaderProgramObject* programObject = NULL;

    __GL_SETUP_NOT_IN_BEGIN();


#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ProgramParameteriEXT", DT_GLuint, program, DT_GLenum, pname, DT_GLint, value, DT_GLnull);
#endif

    if(program <= 0)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    GL_ASSERT(gc->shaderProgram.shared);
    programObject = (__GLshaderProgramObject *)__glGetObject(gc, gc->shaderProgram.shared, program);

    if(!programObject )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

#if ENABLE_GLSL
    {
        EGLSLProgramParameterType etype;
        GLint ivalue;

        if(pname == GL_GEOMETRY_VERTICES_OUT_EXT)
        {
            if (value > (GLint)gc->constants.maxGeometryOutputVertices)
            {
                __glSetError(GL_INVALID_VALUE);
                return;
            }
            programObject->programInfo.geomVerticesOut = value;
            etype = E_GLSL_GEOMETRY_VERTICES_OUT;
            ivalue = value;
        }
        else if(pname == GL_GEOMETRY_INPUT_TYPE_EXT)
        {
            switch(value)
            {
            case GL_POINTS:
                ivalue = E_GLSL_POINTS;
                break;
            case GL_LINES:
                ivalue = E_GLSL_LINES;
                break;
            case GL_LINES_ADJACENCY_EXT:
                ivalue = E_GLSL_LINES_ADJ;
                break;
            case GL_TRIANGLES:
                ivalue = E_GLSL_TRIANGLES;
                break;
            case GL_TRIANGLES_ADJACENCY_EXT:
                ivalue = E_GLSL_TRIANGLES_ADJ;
                break;
            default:
                __glSetError(GL_INVALID_VALUE);
                return;
            }
            programObject->programInfo.geomInputType = value;
            etype = E_GLSL_GEOMETRY_INPUT_TYPE;
        }
        else if(pname == GL_GEOMETRY_OUTPUT_TYPE_EXT)
        {
            switch(value)
            {
            case GL_POINTS:
                ivalue = E_GLSL_POINTS;
                break;
            case GL_LINE_STRIP:
                ivalue = E_GLSL_LINE_STRIP;
                break;
            case GL_TRIANGLE_STRIP:
                ivalue = E_GLSL_TRIANGLE_STRIP;
                break;
            default:
                __glSetError(GL_INVALID_VALUE);
                 return;
            }
            programObject->programInfo.geomOutputType = value;
            etype = E_GLSL_GEOMETRY_OUTPUT_TYPE;
        }
        else
        {
           __glSetError(GL_INVALID_VALUE);
           return;
        }

        if(!OGL_Compiler_ProgramParameteri(programObject->programInfo.hProgram, etype, ivalue))
        {
            __glSetError(GL_INVALID_VALUE);
            return;
        }
    }
#else
    switch(pname)
    {
    case GL_GEOMETRY_VERTICES_OUT_EXT:
        programObject->programInfo.geomVerticesOut = value;
        break;
    case GL_GEOMETRY_INPUT_TYPE_EXT:
        programObject->programInfo.geomInputType = value;
        break;
    case GL_GEOMETRY_OUTPUT_TYPE_EXT:
        programObject->programInfo.geomOutputType = value;
        break;
    }

#endif

}

