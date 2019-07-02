/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_es_context.h"
#include "gc_es_object_inline.c"
#include "gc_es_bindtable.h"

#define __GLSL_SET_STATE_USAGE(usage, index, bit)   \
    (usage)->globalAttrState[(index)] |= (bit);     \
    (usage)->globalAttrState[__GLSL_USAGE_ALL_ATTRS] |= (__GL_ONE_32 << (index))

#define __GLSL_SET_TEXTURE_STATE_USAGE(usage, texi, bit)                    \
    (usage)->texAttrState[(texi)] |= (bit);                                 \
    (usage)->globalAttrState[__GLSL_USAGE_TEXTURE_ATTRS] |= (__GL_ONE_32 << (texi));  \
    (usage)->globalAttrState[__GLSL_USAGE_ALL_ATTRS] |= (__GL_ONE_32 << __GLSL_USAGE_TEXTURE_ATTRS)

GLboolean __glInitShaderObject(__GLcontext *gc, __GLshaderObject *shaderObject, GLuint shaderType, GLuint id);
GLboolean __glDeleteShaderObject(__GLcontext *gc, __GLshaderObject * shaderObject);
GLboolean __glDeleteProgramObject(__GLcontext *gc, __GLprogramObject * programObject);
GLboolean __glDeleteShaderProgramObject(__GLcontext *gc, GLvoid * obj);
GLvoid    __glInitXfbObject(__GLcontext *gc, __GLxfbObject *xfbObj, GLuint name);
GLboolean __glDeleteXfbObj(__GLcontext *gc, __GLxfbObject *xfbObj);
GLvoid    __glBindProgramPipeline(__GLcontext *gc, GLuint pipeline);
GLboolean __glDeleteProgramPipelineObj(__GLcontext *gc, __GLprogramPipelineObject *ppObj);
extern GLvoid __glDispatchCompute(__GLcontext *gc);

#define _GC_OBJ_ZONE gcdZONE_ES30_CORE

__GL_INLINE __GLSLStage __glGetShaderStage(GLenum type)
{
    __GLSLStage stageIdx = __GLSL_STAGE_LAST;

    switch (type)
    {
    case GL_VERTEX_SHADER:
        stageIdx = __GLSL_STAGE_VS;
        break;
    case GL_FRAGMENT_SHADER:
        stageIdx = __GLSL_STAGE_FS;
        break;
    case GL_COMPUTE_SHADER:
        stageIdx = __GLSL_STAGE_CS;
        break;
    case GL_TESS_CONTROL_SHADER_EXT:
        stageIdx = __GLSL_STAGE_TCS;
        break;
    case GL_TESS_EVALUATION_SHADER_EXT:
        stageIdx = __GLSL_STAGE_TES;
        break;
    case GL_GEOMETRY_SHADER_EXT:
        stageIdx = __GLSL_STAGE_GS;
        break;
    default:
        GL_ASSERT(0);
    }
    return stageIdx;
}


/* Internal functions for object management */
GLboolean __glInitShaderObject(__GLcontext *gc, __GLshaderObject *shaderObject, GLuint shaderType, GLuint id)
{
    shaderObject->objectInfo.bindCount = 0;

    shaderObject->objectInfo.id = id;
    shaderObject->objectInfo.objectType = __GL_SHADER_OBJECT_TYPE;

    shaderObject->shaderInfo.shaderType = shaderType;

    return GL_TRUE;
}

GLboolean __glInitProgramObject(__GLcontext *gc, __GLprogramObject *programObject, GLuint id, GLuint uniqueId)
{
    GLuint i;
    programObject->objectInfo.bindCount = 0;

    programObject->objectInfo.id = id;
    programObject->objectInfo.objectType = __GL_PROGRAM_OBJECT_TYPE;

    programObject->programInfo.deleteStatus = GL_FALSE;
    programObject->programInfo.linkedStatus = GL_FALSE;
    programObject->programInfo.validateStatus = GL_FALSE;
    programObject->programInfo.invalidFlags = __GL_INVALID_LINK_BIT;
    for (i = 0; i < __GLSL_STAGE_LAST; i++)
    {
        programObject->programInfo.attachedShader[i] = gcvNULL;
    }
    programObject->programInfo.separable = GL_FALSE;
    programObject->programInfo.retrievable = GL_FALSE;
    programObject->bindingInfo.isSeparable = GL_FALSE;
    programObject->bindingInfo.isRetrievable = GL_FALSE;
    programObject->bindingInfo.xfbMode = GL_INTERLEAVED_ATTRIBS;
    programObject->programInfo.uniqueId = uniqueId;

    programObject->samplerSeq = 0;

    /* xfb */
    programObject->xfbMode = GL_INTERLEAVED_ATTRIBS;
    programObject->xfbVaryingNum = 0;
    programObject->ppXfbVaryingNames = gcvNULL;
    programObject->xfbRefCount = 0;

    programObject->maxSampler = 0;
    programObject->maxUnit = 0;

    __GL_MEMZERO(programObject->bindingInfo.workGroupSize, 3 * sizeof(GLuint));

    programObject->programInfo.infoLog = (GLchar*)(*gc->imports.calloc)(gc, __GLSL_LOG_INFO_SIZE, sizeof(GLchar));

    return GL_TRUE;
}

GLvoid __glDetachShader(__GLcontext *gc, __GLprogramObject * programObject, __GLshaderObject * shaderObject)
{
    __GLshaderObject **pAttachedShader = gcvNULL;
    __GLSLStage  stage;

    GL_ASSERT(programObject);
    GL_ASSERT(shaderObject);

    stage = __glGetShaderStage(shaderObject->shaderInfo.shaderType);

    pAttachedShader = &programObject->programInfo.attachedShader[stage];

    /* The shader was not attached to the program */
    if (*pAttachedShader != shaderObject)
    {
        __GL_ERROR_RET(GL_INVALID_OPERATION);
    }
    *pAttachedShader = gcvNULL;

    if ((--shaderObject->objectInfo.bindCount) == 0 && shaderObject->shaderInfo.deleteStatus)
    {
        __glDeleteObject(gc, gc->shaderProgram.spShared, shaderObject->objectInfo.id);
    }
}

GLboolean __glDeleteShaderObject(__GLcontext *gc, __GLshaderObject * shaderObject)
{
    GL_ASSERT(shaderObject);

    /* Check if the object can be deleted */
    if (shaderObject->objectInfo.bindCount > 0)
    {
        shaderObject->shaderInfo.deleteStatus = GL_TRUE;
        return GL_FALSE;
    }

    if (shaderObject->objectInfo.label)
    {
        gc->imports.free(gc, shaderObject->objectInfo.label);
    }

    /* Delete shader source */
    if (shaderObject->shaderInfo.source)
    {
        (*gc->imports.free)(gc, shaderObject->shaderInfo.source);
        shaderObject->shaderInfo.source = gcvNULL;
    }

    (*gc->dp.deleteShader)(gc, shaderObject);

    /* Delete shader object */
    (*gc->imports.free)(gc, shaderObject);

    return GL_TRUE;
}

GLboolean __glDeleteProgramObject(__GLcontext *gc, __GLprogramObject * programObject)
{
    GLuint i;
    __GLSLStage stage;

    GL_ASSERT(programObject);

    for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
    {
        if (gc->shaderProgram.lastProgObjs[stage] == programObject)
        {
            /* Add this logic to avoid mis-detection in the attribute evaluation function */
            gc->shaderProgram.lastProgObjs[stage] = gcvNULL;
            gc->shaderProgram.lastCodeSeqs[stage] = 0xFFFFFFFF;
        }
    }

    if (programObject->objectInfo.bindCount > 0)
    {
        programObject->programInfo.deleteStatus = GL_TRUE;
        return GL_FALSE;
    }

    (*gc->dp.deleteProgram)(gc, programObject);

    /* Detach all shaders */
    for (i = 0; i < __GLSL_STAGE_LAST; ++i)
    {
        if (programObject->programInfo.attachedShader[i])
        {
            __glDetachShader(gc, programObject, programObject->programInfo.attachedShader[i]);
        }
    }

    if (programObject->objectInfo.label)
    {
        gc->imports.free(gc, programObject->objectInfo.label);
    }

    /* Free info log buffer */
    if (programObject->programInfo.infoLog != gcvNULL)
    {
        (*gc->imports.free)(gc, programObject->programInfo.infoLog);
        programObject->programInfo.infoLog = gcvNULL;
    }

    for (i = 0; i < programObject->xfbVaryingNum; i++)
    {
        GL_ASSERT(programObject->ppXfbVaryingNames[i]);
        (*gc->imports.free)(gc, programObject->ppXfbVaryingNames[i]);
    }

    if (programObject->ppXfbVaryingNames)
    {
        (*gc->imports.free)(gc, programObject->ppXfbVaryingNames);
    }

    (*gc->imports.free)(gc, programObject);

    return GL_TRUE;
}

GLboolean __glDeleteShaderProgramObject(__GLcontext *gc, GLvoid * obj)
{
    __GLshPrgObjInfo *objectInfo = (__GLshPrgObjInfo *)obj;

    switch (objectInfo->objectType)
    {
    case __GL_SHADER_OBJECT_TYPE:
        return __glDeleteShaderObject(gc, (__GLshaderObject*)obj);
        break;

    case __GL_PROGRAM_OBJECT_TYPE:
        return __glDeleteProgramObject(gc, (__GLprogramObject*)obj);
        break;

    default:
        return GL_FALSE;
    }
}

GLvoid __glBindTransformFeedback(__GLcontext *gc, GLuint id)
{
    __GLxfbObject *xfbObj = gcvNULL;
    __GLxfbObject *boundObj = gcvNULL;

    boundObj = gc->xfb.boundXfbObj;

    if (boundObj->active && !boundObj->paused)
    {
        __GL_ERROR_RET(GL_INVALID_OPERATION);
    }

    if (boundObj->name == id)
    {
        return;
    }

    GL_ASSERT(gc->xfb.noShare);

    if (id == 0)
    {
        /* Retrieve the default object in __GLcontext.
        */
        xfbObj = &gc->xfb.defaultXfbObj;
        GL_ASSERT(xfbObj->name == 0);
    }
    else
    {
        if (!__glIsNameDefined(gc, gc->xfb.noShare, id))
        {
            __GL_ERROR_RET(GL_INVALID_OPERATION);
        }

        xfbObj = (__GLxfbObject *)__glGetObject(gc, gc->xfb.noShare, id);
    }

    if (gcvNULL == xfbObj)
    {
        xfbObj = (__GLxfbObject *)(*gc->imports.calloc)(gc, 1, sizeof(__GLxfbObject));
        __glInitXfbObject(gc, xfbObj, id);
        __glAddObject(gc, gc->xfb.noShare, id, xfbObj);
    }

    gc->xfb.boundXfbObj = xfbObj;

    /* Call the dp interface */
    (*gc->dp.bindXFB)(gc, xfbObj);

    gc->xfb.dirtyState |= __GL_XFB_DIRTY_OBJECT;
}


GLvoid __glInitXfbObject(__GLcontext *gc, __GLxfbObject *xfbObj, GLuint name)
{
    xfbObj->name = name;
    xfbObj->active = GL_FALSE;
    xfbObj->paused = GL_FALSE;
    xfbObj->primMode = 0;
    xfbObj->offset = 0;
    xfbObj->vertices = 0;
    xfbObj->programObj = NULL;
    xfbObj->flags = 0;
    xfbObj->boundBufObj = NULL;
    xfbObj->boundBufName = 0;
    xfbObj->privateData = NULL;
    __GL_MEMZERO(xfbObj->boundBufBinding, sizeof(__GLBufBindPoint)* __GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS);
}

GLboolean __glDeleteXfbObj(__GLcontext *gc, __GLxfbObject *xfbObj)
{
    if (xfbObj->active)
    {
        /* For any object named by ids is currently active. */
        __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
    }

    /* Decrease bound buffer object bindCount */
    if (xfbObj->boundBufObj)
    {
        __GLbufferObject *oldBufObj = xfbObj->boundBufObj;

        oldBufObj->bindCount--;

        if (!oldBufObj->bindCount && !oldBufObj->vaoList &&
            !oldBufObj->texList && (oldBufObj->flag & __GL_OBJECT_IS_DELETED))
        {
            __glDeleteBufferObject(gc, oldBufObj);
        }
    }

    if (xfbObj == gc->xfb.boundXfbObj)
    {
        __glBindTransformFeedback(gc, 0);
    }

    if (xfbObj->label)
    {
        gc->imports.free(gc, xfbObj->label);
    }

    (*gc->dp.deleteXFB)(gc, xfbObj);

    (*gc->imports.free)(gc, xfbObj);

    return GL_TRUE;
}


GLvoid __glInitShaderProgramState(__GLcontext *gc)
{
    __GLSLStage stage;
    GLuint unit;
    GLuint sampler;

    /* Shader and program objects can be shared across contexts */
    if (gc->shareCtx)
    {
        GL_ASSERT(gc->shareCtx->shaderProgram.spShared);
        gc->shaderProgram.spShared = gc->shareCtx->shaderProgram.spShared;
        gcoOS_LockPLS();
        gc->shaderProgram.spShared->refcount++;

        /* Allocate VEGL lock */
        if (gcvNULL == gc->shaderProgram.spShared->lock)
        {
            gc->shaderProgram.spShared->lock = (*gc->imports.calloc)(gc, 1, sizeof(VEGLLock));
           (*gc->imports.createMutex)(gc->shaderProgram.spShared->lock);
        }
        gcoOS_UnLockPLS();
    }
    else
    {
        GL_ASSERT(gcvNULL == gc->shaderProgram.spShared);

        gc->shaderProgram.spShared = (__GLsharedObjectMachine*)(*gc->imports.calloc)(gc, 1, sizeof(__GLsharedObjectMachine));
        /* Initialize a linear lookup table for program objects */
        gc->shaderProgram.spShared->maxLinearTableSize = __GL_MAX_PRGOBJ_LINEAR_TABLE_SIZE;
        gc->shaderProgram.spShared->linearTableSize = __GL_DEFAULT_PRGOBJ_LINEAR_TABLE_SIZE;
        gc->shaderProgram.spShared->linearTable = (GLvoid **)
            (*gc->imports.calloc)(gc, 1, gc->shaderProgram.spShared->linearTableSize * sizeof(GLvoid *));

        gc->shaderProgram.spShared->hashSize = __GL_PRGOBJ_HASH_TABLE_SIZE;
        gc->shaderProgram.spShared->hashMask = __GL_PRGOBJ_HASH_TABLE_SIZE - 1;
        gc->shaderProgram.spShared->refcount = 1;
        gc->shaderProgram.spShared->deleteObject = (__GLdeleteObjectFunc)__glDeleteShaderProgramObject;
        gc->shaderProgram.spShared->immediateInvalid = GL_FALSE;
    }
    gc->shaderProgram.currentProgram = gcvNULL;

    /* Program Pipeline Objects cannot be shared across contexts */
    if (gc->shaderProgram.ppNoShare == gcvNULL)
    {
        gc->shaderProgram.ppNoShare = (__GLsharedObjectMachine *)
            (*gc->imports.calloc)(gc, 1, sizeof(__GLsharedObjectMachine));

        /* Initialize a linear lookup table for program pipeline Object */
        gc->shaderProgram.ppNoShare->maxLinearTableSize = __GL_MAX_PRGOBJ_LINEAR_TABLE_SIZE;
        gc->shaderProgram.ppNoShare->linearTableSize = __GL_DEFAULT_PRGOBJ_LINEAR_TABLE_SIZE;
        gc->shaderProgram.ppNoShare->linearTable = (GLvoid **)
            (*gc->imports.calloc)(gc, 1, gc->shaderProgram.ppNoShare->linearTableSize * sizeof(GLvoid *));

        gc->shaderProgram.ppNoShare->hashSize = __GL_PRGOBJ_HASH_TABLE_SIZE;
        gc->shaderProgram.ppNoShare->hashMask = __GL_PRGOBJ_HASH_TABLE_SIZE - 1;
        gc->shaderProgram.ppNoShare->refcount = 1;
        gc->shaderProgram.ppNoShare->deleteObject = (__GLdeleteObjectFunc)__glDeleteProgramPipelineObj;
        gc->shaderProgram.ppNoShare->immediateInvalid = GL_FALSE;
    }
    gc->shaderProgram.boundPPO = gcvNULL;

    __glBitmaskInitAllZero(&gc->shaderProgram.samplerMapDirty, gc->constants.shaderCaps.maxTextureSamplers);
    __glBitmaskInitAllZero(&gc->shaderProgram.samplerStateDirty, gc->constants.shaderCaps.maxTextureSamplers);
    __glBitmaskInitAllZero(&gc->shaderProgram.samplerStateKeepDirty ,gc->constants.shaderCaps.maxTextureSamplers);

    __glBitmaskInitAllZero(&gc->shaderProgram.samplerTexelFetchDirty, gc->constants.shaderCaps.maxTextureSamplers);
    __glBitmaskInitAllZero(&gc->shaderProgram.samplerPrevTexelFetchDirty, gc->constants.shaderCaps.maxTextureSamplers);


    gc->shaderProgram.samplerSeq = 0;
    gc->shaderProgram.mode = __GLSL_MODE_GRAPHICS;
    gc->shaderProgram.patchVertices = 3;

    for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
    {
        gc->shaderProgram.lastProgObjs[stage] = gcvNULL;
        gc->shaderProgram.lastCodeSeqs[stage] = 0xFFFFFFFF;
    }

    for (unit = 0; unit < gc->constants.shaderCaps.maxCombinedTextureImageUnits; unit++)
    {
        gc->state.texture.texUnits[unit].enableDim = __GL_MAX_TEXTURE_UNITS;
        gc->shaderProgram.texUnit2Sampler[unit].numSamplers = 0;
    }

    for (sampler = 0; sampler < gc->constants.shaderCaps.maxTextureSamplers; sampler++)
    {
        gc->state.program.sampler2TexUnit[sampler] = __GL_MAX_TEXTURE_UNITS;
    }

    gc->shaderProgram.maxSampler = 0;
}

GLvoid __glFreeShaderProgramState(__GLcontext * gc)
{
    __GLprogramObject *progObj = gc->shaderProgram.currentProgram;

    /* unbind ppo */
    __glBindProgramPipeline(gc, 0);

    /* Free ppo table */
    __glFreeSharedObjectState(gc, gc->shaderProgram.ppNoShare);

    /* unbind program */
    if (progObj)
    {
        if ((--progObj->objectInfo.bindCount) == 0 && progObj->programInfo.deleteStatus)
        {
            __glDeleteObject(gc, gc->shaderProgram.spShared, progObj->objectInfo.id);
        }

        gc->shaderProgram.currentProgram = gcvNULL;
    }
    /* Free shared program object table */
    __glFreeSharedObjectState(gc, gc->shaderProgram.spShared);
}

/************************************************************************/
/* Section for implantation of API functions                            */
/************************************************************************/

GLuint GL_APIENTRY __gles_CreateShader(__GLcontext *gc,  GLenum type)
{
    GLuint shader = 0;
    __GLshaderObject* shaderObject = gcvNULL;

    __GL_HEADER();

    switch (type)
    {
    case GL_VERTEX_SHADER:
    case GL_FRAGMENT_SHADER:
    case GL_COMPUTE_SHADER:
    case GL_TESS_CONTROL_SHADER_EXT:
    case GL_TESS_EVALUATION_SHADER_EXT:
    case GL_GEOMETRY_SHADER_EXT:
        break;
    default:
        shader = 0;
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    GL_ASSERT(gc->shaderProgram.spShared);
    shader = __glGenerateNames(gc, gc->shaderProgram.spShared, 1);
    __glMarkNameUsed(gc, gc->shaderProgram.spShared, shader);

    shaderObject = (__GLshaderObject *)(*gc->imports.calloc)(gc, 1, sizeof(__GLshaderObject));
    if (gcvNULL == shaderObject)
    {
        shader = 0;
        __GL_ERROR_EXIT(GL_OUT_OF_MEMORY);
    }

    __glInitShaderObject(gc, shaderObject, type, shader);
    __glAddObject(gc, gc->shaderProgram.spShared, shader, shaderObject);

OnError:
    __GL_FOOTER();

    return shader;
}

GLvoid GL_APIENTRY __gles_ShaderSource(__GLcontext *gc, GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length)
{
    __GLshaderObject * shaderObject = gcvNULL;
    GLchar *source = gcvNULL;
    GLsizei sourceLen = 0;
    GLsizei i = 0;

    __GL_HEADER();

    if (shader <= 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    GL_ASSERT(gc->shaderProgram.spShared);
    shaderObject = (__GLshaderObject *)__glGetObject(gc, gc->shaderProgram.spShared, shader);
    if (!shaderObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (shaderObject->objectInfo.objectType != __GL_SHADER_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if ((count < 0) || (string == gcvNULL))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    /* Calculate source size */
    for (i = 0; i < count; i++)
    {
        if (!string[i])
        {
            if (length && length[i] > 0)
            {
                /* length[i] is positive value while corresponding string[i] is gcvNULL
                ** Although Spec didn't describe it, we treat it as INVALID_VALUE error
                */
                __GL_ERROR_EXIT(GL_INVALID_VALUE);
            }
            continue;
        }

        sourceLen += (length && length[i] >= 0) ? (GLsizei)length[i] : (GLsizei)strlen(string[i]);
    }

    /* Assemble all sources into one string*/
    source = (GLchar *)(*gc->imports.malloc)(gc, sourceLen + 1);
    if (gcvNULL == source)
    {
        __GL_ERROR_EXIT(GL_OUT_OF_MEMORY);
    }

    source[0] = '\0';
    for (i = 0; i < count; i++)
    {
        GLuint len = 0;

        if (!string[i])
        {
            continue;
        }

        len = (length && length[i] >= 0) ? (GLsizei)length[i] : (GLsizei)strlen(string[i]);

        strncat(source, string[i], len);
    }

    /* Free the previsou shader source. */
    if (shaderObject->shaderInfo.source != gcvNULL)
    {
        (*gc->imports.free)(gc, shaderObject->shaderInfo.source);
    }

    shaderObject->shaderInfo.source     = source;
    shaderObject->shaderInfo.sourceSize = sourceLen;

OnError:
    __GL_FOOTER();

    return;
}

GLvoid GL_APIENTRY __gles_ShaderBinary(__GLcontext *gc, GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length)
{
    __GLshaderObject **shaderObjects = gcvNULL;
    GLboolean noerr;
    GLsizei i;
    GLsizei vertexCount = 0;
    GLsizei fragmentCount = 0;

    __GL_HEADER();

    /* Validate parameters. */
    if ((binaryformat != GL_SHADER_BINARY_VIV) && (binaryformat != GL_PROGRAM_BINARY_VIV))
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }
    if ((n <= 0) || (shaders == gcvNULL))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    /* Set shader binary. */
    shaderObjects = (__GLshaderObject**)gc->imports.calloc(gc, n, sizeof(__GLshaderObject*));
    if (shaderObjects == gcvNULL)
    {
        __GL_ERROR_EXIT(GL_OUT_OF_MEMORY);
    }

    for (i = 0; i < n; ++i)
    {
        shaderObjects[i] = (__GLshaderObject*)__glGetObject(gc, gc->shaderProgram.spShared, shaders[i]);
        if ((shaderObjects[i] == gcvNULL) || (shaderObjects[i]->objectInfo.objectType != __GL_SHADER_OBJECT_TYPE))
        {
            if (shaderObjects[i] && shaderObjects[i]->objectInfo.objectType == __GL_PROGRAM_OBJECT_TYPE)
            {
                gc->imports.free(gc, (gctPOINTER)shaderObjects);
                __GL_ERROR_EXIT(GL_INVALID_OPERATION);
            }
            else
            {
                gc->imports.free(gc, (gctPOINTER)shaderObjects);
                __GL_ERROR_EXIT(GL_INVALID_VALUE);
            }
        }

        switch (shaderObjects[i]->shaderInfo.shaderType)
        {
        case GL_VERTEX_SHADER:
            if (++vertexCount > 1)
            {
                __GL_ERROR(GL_INVALID_OPERATION);
            }
            break;
        case GL_FRAGMENT_SHADER:
            if (++fragmentCount > 1)
            {
                __GL_ERROR(GL_INVALID_OPERATION);
            }
            break;
        }
    }

    if ((binary == gcvNULL) || (length <= 0))
    {
        gc->imports.free(gc, (gctPOINTER)shaderObjects);
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    noerr = gc->dp.shaderBinary(gc, n, shaderObjects, binaryformat, binary, length);

    gc->imports.free(gc, (gctPOINTER)shaderObjects);    /* Free the resource. */
    if (!noerr)
    {
        __GL_ERROR(GL_INVALID_VALUE);
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_CompileShader(__GLcontext *gc, GLuint shader)
{
    __GLshaderObject * shaderObject;

    __GL_HEADER();

    GL_ASSERT(gc->shaderProgram.spShared);
    shaderObject = (__GLshaderObject *)__glGetObject(gc, gc->shaderProgram.spShared, shader);

    if (!shaderObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (shaderObject->objectInfo.objectType != __GL_SHADER_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    shaderObject->shaderInfo.compiledStatus = (*gc->dp.compileShader)(gc, shaderObject);

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_DeleteShader(__GLcontext *gc, GLuint shader)
{
    __GLshaderObject * shaderObject;

    __GL_HEADER();

    if (0 == shader)
    {
        __GL_EXIT();
    }

    GL_ASSERT(gc->shaderProgram.spShared);
    shaderObject = (__GLshaderObject *)__glGetObject(gc, gc->shaderProgram.spShared, shader);
    if (!shaderObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    if (shaderObject->objectInfo.objectType != __GL_SHADER_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    __glDeleteObject(gc, gc->shaderProgram.spShared, shaderObject->objectInfo.id);

OnExit:
OnError:
    __GL_FOOTER();
}

GLuint GL_APIENTRY __gles_CreateProgram(__GLcontext *gc)
{
    GLuint program;
    GLuint uniqueId;
    __GLprogramObject *programObject;

    __GL_HEADER();

    GL_ASSERT(gc->shaderProgram.spShared);
    program = __glGenerateNames(gc, gc->shaderProgram.spShared, 1);
    uniqueId = __glMarkNameUsed(gc, gc->shaderProgram.spShared, program);

    programObject = (__GLprogramObject *)(*gc->imports.calloc)(gc, 1, sizeof(__GLprogramObject));
    if (!programObject)
    {
        program = 0;
        __GL_ERROR_EXIT(GL_OUT_OF_MEMORY);
    }

    __glInitProgramObject(gc, programObject, program, uniqueId);
    __glAddObject(gc, gc->shaderProgram.spShared, program, programObject);

    if (!(*gc->dp.createProgram)(gc, programObject))
    {
        __GL_ERROR((*gc->dp.getError)(gc));
    }

OnError:
    __GL_FOOTER();

    return program;
}

GLvoid GL_APIENTRY __gles_AttachShader(__GLcontext *gc,  GLuint program, GLuint shader)
{
    __GLshaderObject  *shaderObject    = gcvNULL;
    __GLprogramObject *programObject   = gcvNULL;
    __GLshaderObject **pAttachedShader = gcvNULL;
    __GLSLStage stage;

    __GL_HEADER();

    if ((program <= 0) || (shader <= 0))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    GL_ASSERT(gc->shaderProgram.spShared);
    shaderObject = (__GLshaderObject *)__glGetObject(gc, gc->shaderProgram.spShared, shader);
    if (!shaderObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (shaderObject->objectInfo.objectType != __GL_SHADER_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    stage = __glGetShaderStage(shaderObject->shaderInfo.shaderType);

    pAttachedShader = &programObject->programInfo.attachedShader[stage];

    /* There is shader of the same type already be attached */
    if (*pAttachedShader)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    *pAttachedShader = shaderObject;

    /* bindCount includes bindings to programObjects both in single context and in shared contexts.
    */
    shaderObject->objectInfo.bindCount++;

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_DetachShader(__GLcontext *gc,  GLuint program, GLuint shader)
{
    __GLshaderObject * shaderObject;
    __GLprogramObject * programObject;

    __GL_HEADER();

    if ((program <= 0) || (shader <= 0))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    GL_ASSERT(gc->shaderProgram.spShared);
    shaderObject = (__GLshaderObject *)__glGetObject(gc, gc->shaderProgram.spShared, shader);
    if (!shaderObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (shaderObject->objectInfo.objectType != __GL_SHADER_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    __glDetachShader(gc, programObject, shaderObject);

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_LinkProgram(__GLcontext *gc,  GLuint program)
{
    __GLprogramObject * programObject = gcvNULL;

    __GL_HEADER();

    if (program <= 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    GL_ASSERT(gc->shaderProgram.spShared);
    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    /* Used by one or more (active) transform feedback objects */
    if (programObject->xfbRefCount)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    /* Clear the log buffer*/
    programObject->programInfo.infoLog[0] = '\0';

    /* Compute shader can't co-exist with other shader types */
    if ((programObject->programInfo.attachedShader[__GLSL_STAGE_CS])   &&
        (programObject->programInfo.attachedShader[__GLSL_STAGE_VS]  ||
         programObject->programInfo.attachedShader[__GLSL_STAGE_FS]  ||
         programObject->programInfo.attachedShader[__GLSL_STAGE_TCS] ||
         programObject->programInfo.attachedShader[__GLSL_STAGE_TES] ||
         programObject->programInfo.attachedShader[__GLSL_STAGE_GS]))
    {
        strncpy(programObject->programInfo.infoLog, "Other shaderType exist with compute shader", __GLSL_LOG_INFO_SIZE);
        programObject->programInfo.linkedStatus = GL_FALSE;
        __GL_EXIT();
    }

    if (programObject->programInfo.separable)
    {
        GLuint i;
        GLboolean hasShaderObj = GL_FALSE;
        for (i = 0; i < __GLSL_STAGE_LAST; i++)
        {
            if (programObject->programInfo.attachedShader[i])
            {
               if (!programObject->programInfo.attachedShader[i]->shaderInfo.compiledStatus)
               {
                   strncpy(programObject->programInfo.infoLog, "one attached shader in program is bad", __GLSL_LOG_INFO_SIZE);
                   programObject->programInfo.linkedStatus = GL_FALSE;
                   __GL_EXIT();
               }
               hasShaderObj = GL_TRUE;
            }
        }

        if (!hasShaderObj)
        {
            strncpy(programObject->programInfo.infoLog, "no shader is attached in program", __GLSL_LOG_INFO_SIZE);
            programObject->programInfo.linkedStatus = GL_FALSE;
            __GL_EXIT();
        }
    }
    else
    {
        /* (tbr)Check VS and PS shader. */
        if ((!programObject->programInfo.attachedShader[__GLSL_STAGE_VS] ||
             !programObject->programInfo.attachedShader[__GLSL_STAGE_VS]->shaderInfo.compiledStatus ||
             !programObject->programInfo.attachedShader[__GLSL_STAGE_FS] ||
             !programObject->programInfo.attachedShader[__GLSL_STAGE_FS]->shaderInfo.compiledStatus)
          /* Check CS shader. */
          &&(!programObject->programInfo.attachedShader[__GLSL_STAGE_CS] ||
             !programObject->programInfo.attachedShader[__GLSL_STAGE_CS]->shaderInfo.compiledStatus)
          /* Check VS, TCS and TES shader. */
          &&(!programObject->programInfo.attachedShader[__GLSL_STAGE_VS] ||
             !programObject->programInfo.attachedShader[__GLSL_STAGE_VS]->shaderInfo.compiledStatus ||
             !programObject->programInfo.attachedShader[__GLSL_STAGE_TCS] ||
             !programObject->programInfo.attachedShader[__GLSL_STAGE_TCS]->shaderInfo.compiledStatus ||
             !programObject->programInfo.attachedShader[__GLSL_STAGE_TES] ||
             !programObject->programInfo.attachedShader[__GLSL_STAGE_TES]->shaderInfo.compiledStatus)
           )
        {
            strncpy(programObject->programInfo.infoLog, "either vs or ps or cs in program is missed or bad", __GLSL_LOG_INFO_SIZE);
            programObject->programInfo.linkedStatus = GL_FALSE;
            __GL_EXIT();
        }
    }

    /* Exception handling for too many varyings of xfb GL_INTERLEAVED_ATTRIBS buffermode*/
    if(programObject->xfbVaryingNum > gc->constants.shaderCaps.maxXfbInterleavedComponents && programObject->xfbMode == GL_INTERLEAVED_ATTRIBS){
        strncpy(programObject->programInfo.infoLog, "too many varyings for xfb GL_INTERLEAVED_ATTRIBS buffermode", __GLSL_LOG_INFO_SIZE);
        programObject->programInfo.linkedStatus = GL_FALSE;
        __GL_EXIT();
    }

    /* Exception handling for too many varyings of xfb GL_SEPARATE_ATTRIBS buffermode*/
    if(programObject->xfbVaryingNum > gc->constants.shaderCaps.maxXfbSeparateComponents && programObject->xfbMode == GL_SEPARATE_ATTRIBS){
        strncpy(programObject->programInfo.infoLog, "too many varyings for xfb GL_SEPARATE_ATTRIBS buffermode", __GLSL_LOG_INFO_SIZE);
        programObject->programInfo.linkedStatus = GL_FALSE;
        __GL_EXIT();
    }

    /* Exception handling for repeated varying of xfb*/
    if(programObject->xfbVaryingNum > 1){
        GLuint i,j;
        for(i = 0; i < programObject->xfbVaryingNum; i++){
            for(j = i + 1; j < programObject->xfbVaryingNum; j++){
                if(!strcmp(programObject->ppXfbVaryingNames[i], programObject->ppXfbVaryingNames[j])){
                    strncpy(programObject->programInfo.infoLog, "repeated varying of xfb", __GLSL_LOG_INFO_SIZE);
                    programObject->programInfo.linkedStatus = GL_FALSE;
                    __GL_EXIT();
                }
            }
        }
    }

    programObject->programInfo.codeSeq++;
    programObject->programInfo.linkedStatus = (*gc->dp.linkProgram)(gc, programObject);

    if (programObject->programInfo.linkedStatus)
    {
        __GLSLStage stage;

        /* Record link time info. */
        programObject->bindingInfo.isSeparable = programObject->programInfo.separable;
        programObject->bindingInfo.isRetrievable = programObject->programInfo.retrievable;
        for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
        {
            programObject->bindingInfo.activeShaderID[stage] = programObject->programInfo.attachedShader[stage]
                                                             ? programObject->programInfo.attachedShader[stage]->objectInfo.id
                                                             : 0;
        }

        /* If program is currently in use and re-linked successfully, should be reloaded */
        if (gc->shaderProgram.currentProgram == programObject)
        {
            (*gc->dp.useProgram)(gc, programObject, gcvNULL);
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_GLSL_PROGRAM_SWITCH);
        }
        else if (!gc->shaderProgram.currentProgram && gc->shaderProgram.boundPPO)
        {
            __GLprogramPipelineObject *boundPPO = gc->shaderProgram.boundPPO;

            if (boundPPO->stageProgs[__GLSL_STAGE_VS] == programObject)
            {
                __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_GLSL_VS_SWITCH);
            }

            if (boundPPO->stageProgs[__GLSL_STAGE_FS] == programObject)
            {
                __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_GLSL_FS_SWITCH);
            }

            if (boundPPO->stageProgs[__GLSL_STAGE_CS] == programObject)
            {
                __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_GLSL_CS_SWITCH);
            }

            if (boundPPO->stageProgs[__GLSL_STAGE_TCS] == programObject)
            {
                __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_GLSL_TCS_SWITCH);
            }

            if (boundPPO->stageProgs[__GLSL_STAGE_TES] == programObject)
            {
                __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_GLSL_TES_SWITCH);
            }

            if (boundPPO->stageProgs[__GLSL_STAGE_GS] == programObject)
            {
                __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_GLSL_GS_SWITCH);
            }
        }
    }

OnExit:
OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_UseProgram(__GLcontext *gc, GLuint program)
{
    __GLxfbObject *xfbObj;
    __GLprogramObject * programObject = gcvNULL;

    __GL_HEADER();

    xfbObj = gc->xfb.boundXfbObj;
    if (xfbObj->active && !xfbObj->paused)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    GL_ASSERT(gc->shaderProgram.spShared);
    if (program != 0)
    {
        programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
        if (!programObject)
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
        else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }

        if (!programObject->programInfo.linkedStatus)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
    }
    else
    {
        /* Disable programmable path */
        programObject = gcvNULL;
    }

    if (programObject)
    {
        if (gc->shaderProgram.samplerSeq != programObject->samplerSeq)
        {
            /* There may be other gc modifying the program object's samplers, but we have no way to know
            ** which samplers are modified. so here we have to dirty them all.
            */
            if (gc->shaderProgram.currentProgram == programObject)
            {
                __glBitmaskSetAll(&gc->shaderProgram.samplerMapDirty, GL_TRUE);
                gc->globalDirtyState[__GL_PROGRAM_ATTRS] |= __GL_DIRTY_GLSL_SAMPLER;
                gc->globalDirtyState[__GL_ALL_ATTRS] |= (__GL_ONE_32 << __GL_PROGRAM_ATTRS);
            }

            gc->shaderProgram.samplerSeq = programObject->samplerSeq;
        }
    }

    if (gc->shaderProgram.currentProgram != programObject)
    {
        __GLprogramObject *oldProgObj = gc->shaderProgram.currentProgram;
        if (oldProgObj)
        {
            /* Remove the old program object if possible */
            if ((--oldProgObj->objectInfo.bindCount) == 0 && oldProgObj->programInfo.deleteStatus)
            {
                __glDeleteObject(gc, gc->shaderProgram.spShared, oldProgObj->objectInfo.id);
            }
        }

        /* Set the new program object as the current program */
        gc->shaderProgram.currentProgram = programObject;

        /* bindCount includes both single context and shared context bindings.
        */
        if (programObject)
        {
            programObject->objectInfo.bindCount++;
        }

        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_GLSL_PROGRAM_SWITCH);
    }

    (*gc->dp.useProgram)(gc, programObject, gcvNULL);

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ValidateProgram(__GLcontext *gc,  GLuint program)
{
    __GLprogramObject *programObject = gcvNULL;

    __GL_HEADER();

    GL_ASSERT(gc->shaderProgram.spShared);
    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    /* Clear the log buffer*/
    programObject->programInfo.infoLog[0] = '\0';

    programObject->programInfo.validateStatus = (*gc->dp.validateProgram)(gc, programObject, GL_FALSE);

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_DeleteProgram(__GLcontext *gc,  GLuint program)
{
    __GLprogramObject * programObject;

    __GL_HEADER();

    if (0 == program)
    {
        __GL_EXIT();
    }

    GL_ASSERT(gc->shaderProgram.spShared);
    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    __glDeleteObject(gc, gc->shaderProgram.spShared, programObject->objectInfo.id);

OnExit:
OnError:
    __GL_FOOTER();
}

GLboolean GL_APIENTRY __gles_IsShader(__GLcontext *gc,  GLuint shader)
{
    __GLshPrgObjInfo *object;

    if (shader <= 0)
    {
        return GL_FALSE;
    }

    GL_ASSERT(gc->shaderProgram.spShared);
    object = (__GLshPrgObjInfo *)__glGetObject(gc, gc->shaderProgram.spShared, shader);

    if (object && object->objectType == __GL_SHADER_OBJECT_TYPE)
    {
        return GL_TRUE;
    }
    else
    {
        return GL_FALSE;
    }
}

GLvoid GL_APIENTRY __gles_GetShaderiv(__GLcontext *gc,  GLuint shader, GLenum pname, GLint *params)
{
    __GLshaderObject * shaderObject;

    __GL_HEADER();

    if ((shader <= 0) || (params == gcvNULL))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    shaderObject = (__GLshaderObject *)__glGetObject(gc, gc->shaderProgram.spShared, shader);
    if (!shaderObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (shaderObject->objectInfo.objectType != __GL_SHADER_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    switch (pname)
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
        {
            GLchar *pLog = shaderObject->shaderInfo.compiledLog;
            *params = (pLog && pLog[0] != '\0') ? (GLint)(strlen(pLog) + 1) : 0;
        }
        break;

    case GL_SHADER_SOURCE_LENGTH:
        *params = shaderObject->shaderInfo.sourceSize
                ? (GLint)(shaderObject->shaderInfo.sourceSize + 1)
                : 0;
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetShaderSource(__GLcontext *gc, GLuint shader, GLsizei bufSize,
                                          GLsizei *length, GLchar *source)
{
    __GLshaderObject * shaderObject = gcvNULL;
    GLsizei len = 0;

    __GL_HEADER();

    if (bufSize < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    GL_ASSERT(gc->shaderProgram.spShared);
    shaderObject = (__GLshaderObject *)__glGetObject(gc, gc->shaderProgram.spShared, shader);
    if (!shaderObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (shaderObject->objectInfo.objectType != __GL_SHADER_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    /* According to spec, the returned string should be null-terminated,
    ** but "length" returned should exclude null-terminator.
    */
    if (source && bufSize > 0)
    {
        len = __GL_MIN(shaderObject->shaderInfo.sourceSize, bufSize - 1);
        if (len > 0)
        {
            strncpy(source, shaderObject->shaderInfo.source, len);
        }
        source[len] = '\0';
    }

    if (length)
    {
        *length = len;
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetShaderInfoLog(__GLcontext *gc, GLuint shader, GLsizei bufSize,
                                           GLsizei *length, GLchar *infoLog)
{
    __GLshaderObject * shaderObject = gcvNULL;
    GLsizei len = 0;

    __GL_HEADER();

    if (bufSize < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    GL_ASSERT(gc->shaderProgram.spShared);
    shaderObject = (__GLshaderObject *)__glGetObject(gc, gc->shaderProgram.spShared, shader);
    if (!shaderObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (shaderObject->objectInfo.objectType != __GL_SHADER_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    /* According to spec, the returned string should be null-terminated,
    ** but "length" returned should exclude null-terminator.
    */
    if (infoLog && bufSize > 0)
    {
        if (shaderObject->shaderInfo.compiledLog)
        {
            GLsizei length = (GLsizei)strlen(shaderObject->shaderInfo.compiledLog);
            len = __GL_MIN(length, bufSize - 1);
        }
        else
        {
            len = 0;
        }

        if (len > 0)
        {
            memcpy(infoLog, shaderObject->shaderInfo.compiledLog, len);
        }
        infoLog[len] = '\0';
    }

    if (length)
    {
        *length = len;
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetShaderPrecisionFormat(__GLcontext *gc, GLenum shadertype, GLenum precisiontype,
                                                   GLint* range, GLint* precision)
{
    __GLSLStage stageIndex = __GLSL_STAGE_LAST;
    GLuint typeIndex = __GLSL_TYPE_LAST;

    __GL_HEADER();

    switch (shadertype)
    {
    case GL_VERTEX_SHADER:
        stageIndex = __GLSL_STAGE_VS;
        break;
    case GL_FRAGMENT_SHADER:
        stageIndex = __GLSL_STAGE_FS;
        break;
    case GL_COMPUTE_SHADER:
        stageIndex = __GLSL_STAGE_CS;
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (precisiontype < GL_LOW_FLOAT || precisiontype > GL_HIGH_INT)
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }
    typeIndex = precisiontype - GL_LOW_FLOAT;

    if (range)
    {
        range[0] = gc->constants.shaderPrecision[stageIndex][typeIndex].rangeLow;
        range[1] = gc->constants.shaderPrecision[stageIndex][typeIndex].rangeHigh;
    }

    if (precision)
    {
        *precision = gc->constants.shaderPrecision[stageIndex][typeIndex].precision;
    }

OnError:
    __GL_FOOTER();
}

GLboolean GL_APIENTRY __gles_IsProgram(__GLcontext *gc, GLuint program)
{
    __GLshPrgObjInfo *object;

    if (program <= 0)
    {
        return GL_FALSE;
    }

    GL_ASSERT(gc->shaderProgram.spShared);
    object = (__GLshPrgObjInfo *)__glGetObject(gc, gc->shaderProgram.spShared, program);

    if (object && object->objectType == __GL_PROGRAM_OBJECT_TYPE)
    {
        return GL_TRUE;
    }
    else
    {
        return GL_FALSE;
    }
}

GLvoid GL_APIENTRY __gles_GetProgramiv(__GLcontext *gc, GLuint program, GLenum pname, GLint *params)
{
    __GLprogramObject * programObject;

    __GL_HEADER();

    if ((program <= 0) || (params == gcvNULL))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    GL_ASSERT(gc->shaderProgram.spShared);

    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    switch (pname)
    {
    case GL_DELETE_STATUS:
        *params = programObject->programInfo.deleteStatus;
        break;

    case GL_LINK_STATUS:
        *params = programObject->programInfo.linkedStatus;
        break;

    case GL_VALIDATE_STATUS:
        *params = programObject->programInfo.validateStatus;
        break;

    case GL_INFO_LOG_LENGTH:
        {
            GLchar *pLog = programObject->programInfo.infoLog;
            *params = (pLog && pLog[0] != '\0') ? (GLint)(strlen(pLog) + 1) : 0;
        }
        break;

    case GL_ATTACHED_SHADERS:
        {
            GLint count = 0;
            GLuint i;
            for (i = 0; i < __GLSL_STAGE_LAST; i++)
            {
                if (programObject->programInfo.attachedShader[i] != NULL)
                {
                    count++;
                }
            }
            *params = count;
        }
        break;

    case GL_ACTIVE_ATTRIBUTES:
        *params = programObject->bindingInfo.numActiveInput;
        break;

    case GL_ACTIVE_ATTRIBUTE_MAX_LENGTH:
        *params = programObject->bindingInfo.maxInputNameLen;
        break;

    case GL_ACTIVE_UNIFORMS:
        *params = programObject->bindingInfo.numActiveUniform;
        break;

    case GL_ACTIVE_UNIFORM_MAX_LENGTH:
        *params = programObject->bindingInfo.maxUniformNameLen;
        break;

    case GL_ACTIVE_UNIFORM_BLOCKS:
        *params = programObject->bindingInfo.numActiveUB;
        break;

    case GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH:
        *params = programObject->bindingInfo.maxUBNameLen;
        break;

    case GL_PROGRAM_SEPARABLE:
        *params = programObject->bindingInfo.isSeparable;
        break;

    case GL_PROGRAM_BINARY_RETRIEVABLE_HINT:
        *params = programObject->bindingInfo.isRetrievable;
        break;

    case GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH:
        *params = programObject->bindingInfo.maxXFBNameLen;
        break;

    case GL_TRANSFORM_FEEDBACK_BUFFER_MODE:
        *params = programObject->bindingInfo.xfbMode;
        break;

    case GL_TRANSFORM_FEEDBACK_VARYINGS:
        *params = programObject->bindingInfo.numActiveXFB;
        break;

    case GL_PROGRAM_BINARY_LENGTH:
        if (programObject->programInfo.linkedStatus)
        {
            GLsizei bplen = 0;
            gc->dp.getProgramBinary(gc, programObject, (GLsizei)(0x7ffffff), &bplen, gcvNULL, gcvNULL);
            *params = *((GLint *)&bplen);
        }
        else
        {
            *params = 0;
        }
        break;

    case GL_COMPUTE_WORK_GROUP_SIZE:
        if (programObject->bindingInfo.activeShaderID[__GLSL_STAGE_CS])
        {
            __GL_MEMCOPY(params, programObject->bindingInfo.workGroupSize, 3 * sizeof(GLuint));
        }
        else
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        break;

    case GL_ACTIVE_ATOMIC_COUNTER_BUFFERS:
        *params = (GLint)programObject->bindingInfo.numActiveACBs;
        break;

    case GL_TESS_CONTROL_OUTPUT_VERTICES_EXT:
        if (programObject->programInfo.linkedStatus &&
            programObject->bindingInfo.activeShaderID[__GLSL_STAGE_TCS])
        {
            *params = programObject->bindingInfo.tessOutPatchSize;
        }
        else
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        break;

    case GL_TESS_GEN_MODE_EXT:
        if (programObject->programInfo.linkedStatus &&
            programObject->bindingInfo.activeShaderID[__GLSL_STAGE_TES])
        {
            *params = (GLint)programObject->bindingInfo.tessGenMode;
        }
        else
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        break;

    case GL_TESS_GEN_SPACING_EXT:
        if (programObject->programInfo.linkedStatus &&
            programObject->bindingInfo.activeShaderID[__GLSL_STAGE_TES])
        {
            *params = (GLint)programObject->bindingInfo.tessSpacing;
        }
        else
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        break;

    case GL_TESS_GEN_VERTEX_ORDER_EXT:
        if (programObject->programInfo.linkedStatus &&
            programObject->bindingInfo.activeShaderID[__GLSL_STAGE_TES])
        {
            *params = (GLint)programObject->bindingInfo.tessVertexOrder;
        }
        else
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        break;

    case GL_TESS_GEN_POINT_MODE_EXT:
        if (programObject->programInfo.linkedStatus &&
            programObject->bindingInfo.activeShaderID[__GLSL_STAGE_TES])
        {
            *params = (GLint)programObject->bindingInfo.tessPointMode;
        }
        else
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        break;


    case GL_GEOMETRY_LINKED_VERTICES_OUT_EXT:
        if (programObject->programInfo.linkedStatus &&
            programObject->bindingInfo.activeShaderID[__GLSL_STAGE_GS])
        {
            *params = (GLuint)programObject->bindingInfo.gsOutVertices;
        }
        else
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        break;

    case GL_GEOMETRY_LINKED_INPUT_TYPE_EXT:
        if (programObject->programInfo.linkedStatus &&
            programObject->bindingInfo.activeShaderID[__GLSL_STAGE_GS])
        {
            *params = (GLuint)programObject->bindingInfo.gsInputType;
        }
        else
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        break;

    case GL_GEOMETRY_LINKED_OUTPUT_TYPE_EXT:
        if (programObject->programInfo.linkedStatus &&
            programObject->bindingInfo.activeShaderID[__GLSL_STAGE_GS])
        {
            *params = (GLuint)programObject->bindingInfo.gsOutputType;
        }
        else
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        break;

    case GL_GEOMETRY_SHADER_INVOCATIONS_EXT:
        if (programObject->programInfo.linkedStatus &&
            programObject->bindingInfo.activeShaderID[__GLSL_STAGE_GS])
        {
            *params = (GLuint)programObject->bindingInfo.gsInvocationCount;
        }
        else
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetProgramInfoLog(__GLcontext *gc, GLuint program, GLsizei bufSize,
                                            GLsizei *length, GLchar *infoLog)
{
    __GLprogramObject * programObject = gcvNULL;
    GLsizei len = 0;

    __GL_HEADER();

    if (bufSize < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    GL_ASSERT(gc->shaderProgram.spShared);
    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    /* According to spec, the returned string should be null-terminated,
    ** but "length" returned should exclude null-terminator.
    */
    if (infoLog && bufSize > 0)
    {
        if(programObject->programInfo.infoLog)
        {
            GLsizei length = (GLsizei)strlen(programObject->programInfo.infoLog);
            len = __GL_MIN(length, bufSize - 1);
        }
        else
        {
            len = 0;
        }

        if (len > 0)
        {
            memcpy(infoLog, programObject->programInfo.infoLog, len);
        }
        infoLog[len] = '\0';
    }

    if (length)
    {
        *length = len;
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetAttachedShaders(__GLcontext *gc,  GLuint program, GLsizei maxCount,
                                             GLsizei * count, GLuint *shader)
{
    __GLprogramObject * programObject;
    GLuint i;
    GLint number = 0;

    __GL_HEADER();

    if ((program <= 0) || (!shader))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (maxCount < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    GL_ASSERT(gc->shaderProgram.spShared);

    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    for (i = 0; (i < __GLSL_STAGE_LAST) && (number < maxCount); i++)
    {
        if (programObject->programInfo.attachedShader[i] != NULL)
        {
            shader[number++] = programObject->programInfo.attachedShader[i]->objectInfo.id;
        }
    }

    if (count)
    {
        *count = number;
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetActiveAttrib(__GLcontext *gc,  GLuint program, GLuint index, GLsizei bufSize,
                                          GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
    __GLprogramObject * programObject;

    __GL_HEADER();

    if (bufSize < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    GL_ASSERT(gc->shaderProgram.spShared);
    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (index >= programObject->bindingInfo.numActiveInput)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    (*gc->dp.getActiveAttribute)(gc, programObject, index, bufSize, length, size, type, name);

OnError:
    __GL_FOOTER();
}

GLint GL_APIENTRY __gles_GetAttribLocation(__GLcontext *gc, GLuint program, const GLchar * name)
{
    __GLprogramObject * programObject;
    GLint ret = -1;

    __GL_HEADER();

    if ((program <= 0) || (name == gcvNULL))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (!strncmp(name, "gl_", 3))
    {
        __GL_EXIT();
    }

    GL_ASSERT(gc->shaderProgram.spShared);

    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }
    else if (!programObject->programInfo.linkedStatus)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    ret = (*gc->dp.getAttributeLocation)(gc, programObject, name);

OnError:
OnExit:
    __GL_FOOTER();

    return ret;
}

GLvoid GL_APIENTRY __gles_BindAttribLocation(__GLcontext *gc,  GLuint program, GLuint index, const GLchar * name)
{
    __GLprogramObject * programObject;

    __GL_HEADER();

    if ((program <= 0) || (name == gcvNULL) || (index > gc->constants.shaderCaps.maxUserVertAttributes))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (!strncmp(name, "gl_", 3))
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    GL_ASSERT(gc->shaderProgram.spShared);

    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (!(*gc->dp.bindAttributeLocation)(gc, programObject, index, name))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetActiveUniform(__GLcontext *gc, GLuint program, GLuint index, GLsizei bufSize,
                                           GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
    __GLprogramObject* programObject;

    __GL_HEADER();

    if (bufSize < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    GL_ASSERT(gc->shaderProgram.spShared);
    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (index >= programObject->bindingInfo.numActiveUniform)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    (*gc->dp.getActiveUniform)(gc, programObject, index, bufSize, length, size, type, name);

OnError:
    __GL_FOOTER();
}


GLint GL_APIENTRY __gles_GetUniformLocation(__GLcontext *gc,  GLuint program, const GLchar *name)
{
    __GLprogramObject * programObject;
    GLint ret = -1;

    __GL_HEADER();

    if ((program <= 0) || (name == gcvNULL))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (!strncmp(name, "gl_", 3))
    {
        __GL_EXIT();
    }

    GL_ASSERT(gc->shaderProgram.spShared);
    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }
    else if (!programObject->programInfo.linkedStatus)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    ret = (*gc->dp.getUniformLocation)(gc, programObject, name);

OnExit:
OnError:
    __GL_FOOTER();

    return ret;
}

GLvoid __glUniform(__GLcontext *gc, GLint location, GLint type, GLsizei count,
                   const GLvoid * values, GLboolean transpose)
{
    __GLprogramObject* activeProgObj = gcvNULL;

    activeProgObj = gc->shaderProgram.currentProgram;
    if (!activeProgObj)
    {
        activeProgObj = gc->shaderProgram.boundPPO ? gc->shaderProgram.boundPPO->activeProg : gcvNULL;
        if (!activeProgObj)
        {
            __GL_ERROR_RET(GL_INVALID_OPERATION);
        }
    }

    if (gc->apiVersion == __GL_API_VERSION_ES20 &&
        transpose)
    {
        __GL_ERROR_RET(GL_INVALID_VALUE);
    }

    if (location == -1)
    {
        return;
    }
    if (location < 0)
    {
        __GL_ERROR_RET(GL_INVALID_OPERATION);
    }

    if (count == 0)
    {
        return;
    }

    (*gc->dp.setUniformData)(gc, activeProgObj, location, type, count, values, transpose);
}

GLvoid GL_APIENTRY __gles_Uniform1f(__GLcontext *gc, GLint location, GLfloat x)
{
    __GL_HEADER();

    __glUniform(gc, location, GL_FLOAT, 1, &x, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_Uniform2f(__GLcontext *gc, GLint location, GLfloat x, GLfloat y)
{
    GLfloat v[2] = {x, y};

    __GL_HEADER();

    __glUniform(gc, location, GL_FLOAT_VEC2, 1, v, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_Uniform3f(__GLcontext *gc, GLint location, GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat v[3] = {x, y, z};

    __GL_HEADER();

    __glUniform(gc, location, GL_FLOAT_VEC3, 1, v, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_Uniform4f(__GLcontext *gc, GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    GLfloat v[4] = {x, y, z, w};

    __GL_HEADER();

    __glUniform(gc, location, GL_FLOAT_VEC4, 1, v, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_Uniform1i(__GLcontext *gc, GLint location, GLint x)
{
    __GL_HEADER();

    __glUniform(gc, location, GL_INT, 1, &x, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_Uniform2i(__GLcontext *gc, GLint location, GLint x, GLint y)
{
    GLint iv[2] = {x, y};

    __GL_HEADER();

    __glUniform(gc, location, GL_INT_VEC2, 1, iv, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_Uniform3i(__GLcontext *gc, GLint location, GLint x, GLint y, GLint z)
{
    GLint iv[3]= {x, y, z};

    __GL_HEADER();

    __glUniform(gc, location, GL_INT_VEC3, 1, iv, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_Uniform4i(__GLcontext *gc, GLint location, GLint x, GLint y, GLint z, GLint w)
{
    GLint iv[4] = {x, y, z, w};

    __GL_HEADER();

    __glUniform(gc, location, GL_INT_VEC4, 1, iv, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_Uniform1ui(__GLcontext *gc, GLint location, GLuint x)
{
    __GL_HEADER();

    __glUniform(gc, location, GL_UNSIGNED_INT, 1, &x, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_Uniform2ui(__GLcontext *gc, GLint location, GLuint x, GLuint y)
{
    GLuint iv[2] = {x, y};

    __GL_HEADER();

    __glUniform(gc, location, GL_UNSIGNED_INT_VEC2, 1, iv, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_Uniform3ui(__GLcontext *gc, GLint location, GLuint x, GLuint y, GLuint z)
{
    GLuint iv[3] = {x, y, z};

    __GL_HEADER();

    __glUniform(gc, location, GL_UNSIGNED_INT_VEC3, 1, iv, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_Uniform4ui(__GLcontext *gc, GLint location, GLuint x, GLuint y, GLuint z, GLuint w)
{
    GLuint iv[4] = {x, y, z, w};

    __GL_HEADER();

    __glUniform(gc, location, GL_UNSIGNED_INT_VEC4, 1, iv, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_Uniform1fv(__GLcontext *gc, GLint location, GLsizei count, const GLfloat * v)
{
    __GL_HEADER();

    __glUniform(gc, location, GL_FLOAT, count, v, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_Uniform2fv(__GLcontext *gc, GLint location, GLsizei count, const GLfloat *v)
{
    __GL_HEADER();

    __glUniform(gc, location, GL_FLOAT_VEC2, count, v, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_Uniform3fv(__GLcontext *gc, GLint location, GLsizei count, const GLfloat *v)
{
    __GL_HEADER();

    __glUniform(gc, location, GL_FLOAT_VEC3, count, v, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_Uniform4fv(__GLcontext *gc, GLint location, GLsizei count, const GLfloat *v)
{
    __GL_HEADER();

    __glUniform(gc, location, GL_FLOAT_VEC4, count, v, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_Uniform1iv(__GLcontext *gc, GLint location, GLsizei count, const GLint *v)
{
    __GL_HEADER();

    __glUniform(gc, location, GL_INT, count, v, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_Uniform2iv(__GLcontext *gc, GLint location, GLsizei count, const GLint *v)
{
    __GL_HEADER();

    __glUniform(gc, location, GL_INT_VEC2, count, v, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_Uniform3iv(__GLcontext *gc, GLint location, GLsizei count, const GLint *v)
{
    __GL_HEADER();

    __glUniform(gc, location, GL_INT_VEC3, count, v, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_Uniform4iv(__GLcontext *gc, GLint location, GLsizei count, const GLint *v)
{
    __GL_HEADER();

    __glUniform(gc, location, GL_INT_VEC4, count, v, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_Uniform1uiv(__GLcontext *gc, GLint location, GLsizei count, const GLuint *v)
{
    __GL_HEADER();

    __glUniform(gc, location, GL_UNSIGNED_INT, count, v, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_Uniform2uiv(__GLcontext *gc, GLint location, GLsizei count, const GLuint *v)
{
    __GL_HEADER();

    __glUniform(gc, location, GL_UNSIGNED_INT_VEC2, count, v, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_Uniform3uiv(__GLcontext *gc, GLint location, GLsizei count, const GLuint *v)
{
    __GL_HEADER();

    __glUniform(gc, location, GL_UNSIGNED_INT_VEC3, count, v, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_Uniform4uiv(__GLcontext *gc, GLint location, GLsizei count, const GLuint *v)
{
    __GL_HEADER();

    __glUniform(gc, location, GL_UNSIGNED_INT_VEC4, count, v, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_UniformMatrix2fv(__GLcontext *gc, GLint location, GLsizei count,
                                           GLboolean transpose, const GLfloat *v)
{
    __GL_HEADER();

    __glUniform(gc, location, GL_FLOAT_MAT2, count, v, transpose);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_UniformMatrix3fv(__GLcontext *gc, GLint location, GLsizei count,
                                           GLboolean transpose, const GLfloat *v)
{
    __GL_HEADER();

    __glUniform(gc, location, GL_FLOAT_MAT3, count, v, transpose);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_UniformMatrix4fv(__GLcontext *gc, GLint location, GLsizei count,
                                           GLboolean transpose, const GLfloat * v)
{
    __GL_HEADER();

    __glUniform(gc, location, GL_FLOAT_MAT4, count, v, transpose);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_UniformMatrix2x3fv(__GLcontext *gc, GLint location, GLsizei count,
                                             GLboolean transpose, const GLfloat *v)
{
    __GL_HEADER();

    __glUniform(gc, location, GL_FLOAT_MAT2x3, count, v, transpose);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_UniformMatrix2x4fv(__GLcontext *gc, GLint location, GLsizei count,
                                             GLboolean transpose, const GLfloat *v)
{
    __GL_HEADER();

    __glUniform(gc, location, GL_FLOAT_MAT2x4, count, v, transpose);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_UniformMatrix3x2fv(__GLcontext *gc, GLint location, GLsizei count,
                                             GLboolean transpose, const GLfloat *v)
{
    __GL_HEADER();

    __glUniform(gc, location, GL_FLOAT_MAT3x2, count, v, transpose);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_UniformMatrix3x4fv(__GLcontext *gc, GLint location, GLsizei count,
                                             GLboolean transpose, const GLfloat *v)
{
    __GL_HEADER();

    __glUniform(gc, location, GL_FLOAT_MAT3x4, count, v, transpose);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_UniformMatrix4x2fv(__GLcontext *gc, GLint location, GLsizei count,
                                             GLboolean transpose, const GLfloat * v)
{
    __GL_HEADER();

    __glUniform(gc, location, GL_FLOAT_MAT4x2, count, v, transpose);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_UniformMatrix4x3fv(__GLcontext *gc, GLint location, GLsizei count,
                                             GLboolean transpose, const GLfloat * v)
{
    __GL_HEADER();

    __glUniform(gc, location, GL_FLOAT_MAT4x3, count, v, transpose);

    __GL_FOOTER();
}


GLvoid __glProgramUniform(__GLcontext *gc, GLuint program, GLint location, GLint type,
                          GLsizei count, const GLvoid * values, GLboolean transpose)
{
    __GLprogramObject* programObject = gcvNULL;

    GL_ASSERT(gc->shaderProgram.spShared);
    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_RET(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_RET(GL_INVALID_OPERATION);
    }

    if (location == -1)
    {
        return;
    }
    if (location < 0)
    {
        __GL_ERROR_RET(GL_INVALID_OPERATION);
    }

    if (count == 0)
    {
        return;
    }

    (*gc->dp.setUniformData)(gc, programObject, location, type, count, values, transpose);
}

GLvoid GL_APIENTRY __gles_ProgramUniform1f(__GLcontext *gc, GLuint program, GLint location, GLfloat x)
{
    __GL_HEADER();

    __glProgramUniform(gc, program, location, GL_FLOAT, 1, &x, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ProgramUniform2f(__GLcontext *gc, GLuint program, GLint location, GLfloat x, GLfloat y)
{
    GLfloat v[2] = {x, y};

    __GL_HEADER();

    __glProgramUniform(gc, program, location, GL_FLOAT_VEC2, 1, v, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ProgramUniform3f(__GLcontext *gc, GLuint program, GLint location, GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat v[3] = {x, y, z};

    __GL_HEADER();

    __glProgramUniform(gc, program, location, GL_FLOAT_VEC3, 1, v, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ProgramUniform4f(__GLcontext *gc, GLuint program, GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    GLfloat v[4] = {x, y, z, w};

    __GL_HEADER();

    __glProgramUniform(gc, program, location, GL_FLOAT_VEC4, 1, v, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ProgramUniform1i(__GLcontext *gc, GLuint program, GLint location, GLint x)
{
    __GL_HEADER();

    __glProgramUniform(gc, program, location, GL_INT, 1, &x, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ProgramUniform2i(__GLcontext *gc, GLuint program, GLint location, GLint x, GLint y)
{
    GLint iv[2] = {x, y};

    __GL_HEADER();

    __glProgramUniform(gc, program, location, GL_INT_VEC2, 1, iv, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ProgramUniform3i(__GLcontext *gc, GLuint program, GLint location, GLint x, GLint y, GLint z)
{
    GLint iv[3]= {x, y, z};

    __GL_HEADER();

    __glProgramUniform(gc, program, location, GL_INT_VEC3, 1, iv, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ProgramUniform4i(__GLcontext *gc, GLuint program, GLint location, GLint x, GLint y, GLint z, GLint w)
{
    GLint iv[4] = {x, y, z, w};

    __GL_HEADER();

    __glProgramUniform(gc, program, location, GL_INT_VEC4, 1, iv, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ProgramUniform1ui(__GLcontext *gc, GLuint program, GLint location, GLuint x)
{
    __GL_HEADER();

    __glProgramUniform(gc, program, location, GL_UNSIGNED_INT, 1, &x, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ProgramUniform2ui(__GLcontext *gc, GLuint program, GLint location, GLuint x, GLuint y)
{
    GLuint iv[2] = {x, y};

    __GL_HEADER();

    __glProgramUniform(gc, program, location, GL_UNSIGNED_INT_VEC2, 1, iv, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ProgramUniform3ui(__GLcontext *gc, GLuint program, GLint location, GLuint x, GLuint y, GLuint z)
{
    GLuint iv[3] = {x, y, z};

    __GL_HEADER();

    __glProgramUniform(gc, program, location, GL_UNSIGNED_INT_VEC3, 1, iv, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ProgramUniform4ui(__GLcontext *gc, GLuint program, GLint location, GLuint x, GLuint y, GLuint z, GLuint w)
{
    GLuint iv[4] = {x, y, z, w};

    __GL_HEADER();

    __glProgramUniform(gc, program, location, GL_UNSIGNED_INT_VEC4, 1, iv, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ProgramUniform1fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLfloat * v)
{
    __GL_HEADER();

    __glProgramUniform(gc, program, location, GL_FLOAT, count, v, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ProgramUniform2fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLfloat *v)
{
    __GL_HEADER();

    __glProgramUniform(gc, program, location, GL_FLOAT_VEC2, count, v, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ProgramUniform3fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLfloat *v)
{
    __GL_HEADER();

    __glProgramUniform(gc, program, location, GL_FLOAT_VEC3, count, v, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ProgramUniform4fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLfloat *v)
{
    __GL_HEADER();

    __glProgramUniform(gc, program, location, GL_FLOAT_VEC4, count, v, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ProgramUniform1iv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLint *v)
{
    __GL_HEADER();

    __glProgramUniform(gc, program, location, GL_INT, count, v, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ProgramUniform2iv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLint *v)
{
    __GL_HEADER();

    __glProgramUniform(gc, program, location, GL_INT_VEC2, count, v, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ProgramUniform3iv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLint *v)
{
    __GL_HEADER();

    __glProgramUniform(gc, program, location, GL_INT_VEC3, count, v, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ProgramUniform4iv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLint *v)
{
    __GL_HEADER();

    __glProgramUniform(gc, program, location, GL_INT_VEC4, count, v, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ProgramUniform1uiv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLuint *v)
{
    __GL_HEADER();

    __glProgramUniform(gc, program, location, GL_UNSIGNED_INT, count, v, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ProgramUniform2uiv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLuint *v)
{
    __GL_HEADER();

    __glProgramUniform(gc, program, location, GL_UNSIGNED_INT_VEC2, count, v, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ProgramUniform3uiv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLuint *v)
{
    __GL_HEADER();

    __glProgramUniform(gc, program, location, GL_UNSIGNED_INT_VEC3, count, v, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ProgramUniform4uiv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLuint *v)
{
    __GL_HEADER();

    __glProgramUniform(gc, program, location, GL_UNSIGNED_INT_VEC4, count, v, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ProgramUniformMatrix2fv(__GLcontext *gc, GLuint program, GLint location,
                                                  GLsizei count, GLboolean transpose, const GLfloat *v)
{
    __GL_HEADER();

    __glProgramUniform(gc, program, location, GL_FLOAT_MAT2, count, v, transpose);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ProgramUniformMatrix3fv(__GLcontext *gc, GLuint program, GLint location,
                                                  GLsizei count, GLboolean transpose, const GLfloat *v)
{
    __GL_HEADER();

    __glProgramUniform(gc, program, location, GL_FLOAT_MAT3, count, v, transpose);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ProgramUniformMatrix4fv(__GLcontext *gc, GLuint program, GLint location,
                                                  GLsizei count, GLboolean transpose, const GLfloat * v)
{
    __GL_HEADER();

    __glProgramUniform(gc, program, location, GL_FLOAT_MAT4, count, v, transpose);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ProgramUniformMatrix2x3fv(__GLcontext *gc, GLuint program, GLint location,
                                                    GLsizei count, GLboolean transpose, const GLfloat *v)
{
    __GL_HEADER();

    __glProgramUniform(gc, program, location, GL_FLOAT_MAT2x3, count, v, transpose);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ProgramUniformMatrix2x4fv(__GLcontext *gc, GLuint program, GLint location,
                                                    GLsizei count, GLboolean transpose, const GLfloat *v)
{
    __GL_HEADER();

    __glProgramUniform(gc, program, location, GL_FLOAT_MAT2x4, count, v, transpose);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ProgramUniformMatrix3x2fv(__GLcontext *gc, GLuint program, GLint location,
                                                    GLsizei count, GLboolean transpose, const GLfloat *v)
{
    __GL_HEADER();

    __glProgramUniform(gc, program, location, GL_FLOAT_MAT3x2, count, v, transpose);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ProgramUniformMatrix3x4fv(__GLcontext *gc, GLuint program, GLint location,
                                                    GLsizei count, GLboolean transpose, const GLfloat *v)
{
    __GL_HEADER();

    __glProgramUniform(gc, program, location, GL_FLOAT_MAT3x4, count, v, transpose);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ProgramUniformMatrix4x2fv(__GLcontext *gc, GLuint program, GLint location,
                                                    GLsizei count, GLboolean transpose, const GLfloat * v)
{
    __GL_HEADER();

    __glProgramUniform(gc, program, location, GL_FLOAT_MAT4x2, count, v, transpose);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ProgramUniformMatrix4x3fv(__GLcontext *gc, GLuint program, GLint location,
                                                    GLsizei count, GLboolean transpose, const GLfloat * v)
{
    __GL_HEADER();

    __glProgramUniform(gc, program, location, GL_FLOAT_MAT4x3, count, v, transpose);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetUniformfv(__GLcontext *gc, GLuint program, GLint location, GLfloat *params)
{
    __GLprogramObject* programObject = gcvNULL;

    __GL_HEADER();

    if ((program <= 0) || !params)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    if (location < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    GL_ASSERT(gc->shaderProgram.spShared);
    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }
    else if (!programObject->programInfo.linkedStatus)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (!(*gc->dp.getUniformData)(gc, programObject, location, GL_FLOAT, params))
    {
        __GL_ERROR_EXIT((*gc->dp.getError)(gc));
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetUniformiv(__GLcontext *gc,  GLuint program, GLint location, GLint *params)
{
    __GLprogramObject* programObject = gcvNULL;

    __GL_HEADER();

    if ((program <= 0) || (params == gcvNULL))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    if (location < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    GL_ASSERT(gc->shaderProgram.spShared);

    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }
    else if (!programObject->programInfo.linkedStatus)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (!(*gc->dp.getUniformData)(gc, programObject, location, GL_INT, params))
    {
        __GL_ERROR_EXIT((*gc->dp.getError)(gc));
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetUniformuiv(__GLcontext *gc, GLuint program, GLint location, GLuint *params)
{
    __GLprogramObject* programObject = gcvNULL;

    __GL_HEADER();

    if ((program <= 0) || (params == gcvNULL))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    if (location < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    GL_ASSERT(gc->shaderProgram.spShared);

    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }
    else if (!programObject->programInfo.linkedStatus)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (!(*gc->dp.getUniformData)(gc, programObject, location, GL_UNSIGNED_INT, params))
    {
        __GL_ERROR_EXIT((*gc->dp.getError)(gc));
    }

OnError:
    __GL_FOOTER();
}


GLvoid GL_APIENTRY __gles_GetnUniformfv(__GLcontext *gc, GLuint program, GLint location, GLsizei bufSize, GLfloat *params)
{
    __GLprogramObject* programObject = gcvNULL;
    GLsizei uniformSize;

    __GL_HEADER();

    if ((program <= 0) || !params)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    if (location < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    GL_ASSERT(gc->shaderProgram.spShared);
    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }
    else if (!programObject->programInfo.linkedStatus)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    uniformSize = (*gc->dp.getUniformSize)(gc, programObject, location);

    if (uniformSize <= bufSize)
    {
        if (!(*gc->dp.getUniformData)(gc, programObject, location, GL_FLOAT, params))
        {
            __GL_ERROR_EXIT((*gc->dp.getError)(gc));
        }
    }
    else
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetnUniformiv(__GLcontext *gc, GLuint program, GLint location, GLsizei bufSize, GLint *params)
{
    __GLprogramObject* programObject = gcvNULL;
    GLsizei uniformSize;

    __GL_HEADER();

    if ((program <= 0) || (params == gcvNULL))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    if (location < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    GL_ASSERT(gc->shaderProgram.spShared);

    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }
    else if (!programObject->programInfo.linkedStatus)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    uniformSize = (*gc->dp.getUniformSize)(gc, programObject, location);

    if (uniformSize <= bufSize)
    {
        if (!(*gc->dp.getUniformData)(gc, programObject, location, GL_INT, params))
        {
            __GL_ERROR_EXIT((*gc->dp.getError)(gc));
        }
    }
    else
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetnUniformuiv(__GLcontext *gc, GLuint program, GLint location, GLsizei bufSize, GLuint *params)
{
    __GLprogramObject* programObject = gcvNULL;
    GLsizei uniformSize;

    __GL_HEADER();

    if ((program <= 0) || (params == gcvNULL))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    if (location < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    GL_ASSERT(gc->shaderProgram.spShared);

    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }
    else if (!programObject->programInfo.linkedStatus)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    uniformSize = (*gc->dp.getUniformSize)(gc, programObject, location);

    if (uniformSize <= bufSize)
    {
        if (!(*gc->dp.getUniformData)(gc, programObject, location, GL_UNSIGNED_INT, params))
        {
            __GL_ERROR_EXIT((*gc->dp.getError)(gc));
        }
    }
    else
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

OnError:
    __GL_FOOTER();
}


GLvoid GL_APIENTRY __gles_ReleaseShaderCompiler(__GLcontext *gc)
{
}

GLvoid GL_APIENTRY __gles_ProgramBinary(__GLcontext *gc, GLuint program, GLenum binaryFormat,
                                        const GLvoid* binary, GLsizei length)
{
    __GLprogramObject * programObject = gcvNULL;
    GLint i = 0;

    __GL_HEADER();

    if (program <= 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    for (i = 0; i < gc->constants.numProgramBinaryFormats; ++i)
    {
        if (gc->constants.pProgramBinaryFormats[i] == (GLint)binaryFormat)
        {
            break;
        }
    }

    if (i == gc->constants.numProgramBinaryFormats)
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    GL_ASSERT(gc->shaderProgram.spShared);
    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    /* Used by one or more (active) transform feedback objects */
    if (programObject->xfbRefCount)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    /* Clear the log buffer*/
    programObject->programInfo.infoLog[0] = '\0';

    programObject->programInfo.codeSeq++;

    /* Set program binary data. */
    programObject->programInfo.linkedStatus = (*gc->dp.programBinary)(gc, programObject, binary, length);

    if (programObject->programInfo.linkedStatus)
    {
        /* If program is currently in use and re-linked successfully, should be reloaded */
        if (gc->shaderProgram.currentProgram == programObject)
        {
            (*gc->dp.useProgram)(gc, programObject, gcvNULL);
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_GLSL_PROGRAM_SWITCH);
        }
        else if (!gc->shaderProgram.currentProgram && gc->shaderProgram.boundPPO)
        {
            __GLprogramPipelineObject *boundPPO = gc->shaderProgram.boundPPO;

            if (boundPPO->stageProgs[__GLSL_STAGE_VS] == programObject)
            {
                __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_GLSL_VS_SWITCH);
            }

            if (boundPPO->stageProgs[__GLSL_STAGE_FS] == programObject)
            {
                __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_GLSL_FS_SWITCH);
            }

            if (boundPPO->stageProgs[__GLSL_STAGE_CS] == programObject)
            {
                __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_GLSL_CS_SWITCH);
            }

            if (boundPPO->stageProgs[__GLSL_STAGE_TCS] == programObject)
            {
                __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_GLSL_TCS_SWITCH);
            }

            if (boundPPO->stageProgs[__GLSL_STAGE_TES] == programObject)
            {
                __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_GLSL_TES_SWITCH);
            }

            if (boundPPO->stageProgs[__GLSL_STAGE_GS] == programObject)
            {
                __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_GLSL_GS_SWITCH);
            }
        }
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ProgramParameteri(__GLcontext *gc, GLuint program, GLenum pname, GLint value)
{
    __GLprogramObject * programObject = gcvNULL;

    __GL_HEADER();

    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (value != GL_TRUE && value != GL_FALSE)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    switch (pname)
    {
    case GL_PROGRAM_SEPARABLE:
        programObject->programInfo.separable = (GLboolean)value;
        break;
    case GL_PROGRAM_BINARY_RETRIEVABLE_HINT:
        programObject->programInfo.retrievable = (GLboolean)value;
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetProgramBinary(__GLcontext *gc, GLuint program, GLsizei bufSize,
                                           GLsizei* length, GLenum* binaryFormat, GLvoid* binary)
{
    __GLprogramObject * programObject = gcvNULL;
    GLboolean result = GL_FALSE;

    __GL_HEADER();

    /* Validate the parameters */
    if (binaryFormat == gcvNULL)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    /* Find the program object */
    programObject = (__GLprogramObject*)__glGetObject(gc, gc->shaderProgram.spShared, program);

    /* Check for errors */
    if (programObject == gcvNULL)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    if ((programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE) || !programObject->programInfo.linkedStatus)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    /* Get the binary */
    result = gc->dp.getProgramBinary(gc, programObject, bufSize, length, binaryFormat, binary);
    if (!result)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

OnError:
    __GL_FOOTER();
}


GLint GL_APIENTRY __gles_GetFragDataLocation(__GLcontext *gc, GLuint program, const GLchar *name)
{
    __GLprogramObject * programObject;
    GLint ret = -1;

    __GL_HEADER();

    if (((GLint)program <= 0) || (name == gcvNULL))
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    GL_ASSERT(gc->shaderProgram.spShared);

    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }
    else if (!programObject->programInfo.linkedStatus)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (!strncmp(name, "gl_", 3))
    {
        __GL_EXIT();
    }

    ret = (*gc->dp.getFragDataLocation)(gc, programObject, name);

OnExit:
OnError:
    __GL_FOOTER();

    return ret;
}

GLvoid GL_APIENTRY __gles_GetUniformIndices(__GLcontext *gc, GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices)
{
    __GLprogramObject* programObject = gcvNULL;

    __GL_HEADER();

    if (program <= 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    GL_ASSERT(gc->shaderProgram.spShared);
    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }
    else if (!programObject->programInfo.linkedStatus)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (uniformCount < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    /* Just return if app does not really get anything */
    if (0 == uniformCount || !uniformIndices)
    {
        __GL_EXIT();
    }

    if (!uniformNames)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    (*gc->dp.getUniformIndices)(gc, programObject, uniformCount, uniformNames, uniformIndices);

OnExit:
OnError:
    __GL_FOOTER();

}

GLvoid GL_APIENTRY __gles_GetActiveUniformsiv(__GLcontext *gc, GLuint program, GLsizei uniformCount,
                                              const GLuint* uniformIndices, GLenum pname, GLint* params)
{
    GLsizei i;
    __GLprogramObject* programObject = gcvNULL;

    __GL_HEADER();

    if (program <= 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    GL_ASSERT(gc->shaderProgram.spShared);
    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }
    else if (!programObject->programInfo.linkedStatus)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (uniformCount < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    /* Just return if app does not really get anything */
    if (0 == uniformCount || !params)
    {
        __GL_EXIT();
    }

    switch (pname)
    {
    case GL_UNIFORM_TYPE:
    case GL_UNIFORM_SIZE:
    case GL_UNIFORM_NAME_LENGTH:
    case GL_UNIFORM_BLOCK_INDEX:
    case GL_UNIFORM_OFFSET:
    case GL_UNIFORM_ARRAY_STRIDE:
    case GL_UNIFORM_MATRIX_STRIDE:
    case GL_UNIFORM_IS_ROW_MAJOR:
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (!uniformIndices)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    for (i = 0; i < uniformCount; ++i)
    {
        if (uniformIndices[i] >= programObject->bindingInfo.numActiveUniform)
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
    }

    (*gc->dp.getActiveUniformsiv)(gc, programObject, uniformCount, uniformIndices, pname, params);

OnExit:
OnError:
    __GL_FOOTER();
}

GLuint GL_APIENTRY __gles_GetUniformBlockIndex(__GLcontext *gc, GLuint program, const GLchar* uniformBlockName)
{
    __GLprogramObject* programObject = gcvNULL;
    GLuint ret = GL_INVALID_INDEX;

    __GL_HEADER();

    if (program <= 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    GL_ASSERT(gc->shaderProgram.spShared);
    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (!programObject->programInfo.linkedStatus)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (!uniformBlockName)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    ret = (*gc->dp.getUniformBlockIndex)(gc, programObject, uniformBlockName);

OnError:
    __GL_FOOTER();

    return ret;
}

GLvoid GL_APIENTRY __gles_GetActiveUniformBlockiv(__GLcontext *gc, GLuint program, GLuint uniformBlockIndex,
                                                  GLenum pname, GLint* params)
{
    __GLprogramObject* programObject = gcvNULL;

    __GL_HEADER();

    if (program <= 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    GL_ASSERT(gc->shaderProgram.spShared);
    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }
    else if (!programObject->programInfo.linkedStatus)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (uniformBlockIndex >= programObject->bindingInfo.numActiveUB)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    switch (pname)
    {
    case GL_UNIFORM_BLOCK_BINDING:
    case GL_UNIFORM_BLOCK_DATA_SIZE:
    case GL_UNIFORM_BLOCK_NAME_LENGTH:
    case GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS:
    case GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES:
    case GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER:
    case GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER:
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    /* Just return if app does not really get anything */
    if (!params)
    {
        __GL_EXIT();
    }

    (*gc->dp.getActiveUniformBlockiv)(gc, programObject, uniformBlockIndex, pname, params);

OnExit:
OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetActiveUniformBlockName(__GLcontext *gc, GLuint program, GLuint uniformBlockIndex,
                                                    GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName)
{
    __GLprogramObject* programObject = gcvNULL;

    __GL_HEADER();

    if (program <= 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    GL_ASSERT(gc->shaderProgram.spShared);
    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }
    else if (!programObject->programInfo.linkedStatus)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (uniformBlockIndex >= programObject->bindingInfo.numActiveUB)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    (*gc->dp.getActiveUniformBlockName)(gc, programObject, uniformBlockIndex, bufSize, length, uniformBlockName);

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_UniformBlockBinding(__GLcontext *gc, GLuint program, GLuint uniformBlockIndex,
                                              GLuint uniformBlockBinding)
{
    __GLprogramObject* programObject = gcvNULL;

    __GL_HEADER();

    if (program <= 0 || uniformBlockBinding >= gc->constants.shaderCaps.maxUniformBufferBindings)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    GL_ASSERT(gc->shaderProgram.spShared);
    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (uniformBlockIndex >= programObject->bindingInfo.numActiveUB)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    (*gc->dp.uniformBlockBinding)(gc, programObject, uniformBlockIndex, uniformBlockBinding);

OnError:
    __GL_FOOTER();
}

/*
** XFB
*/

GLvoid __glInitXfbState(__GLcontext *gc)
{
    /* Transform feedback objects cannot be shared across contexts */
    if (gc->xfb.noShare == gcvNULL)
    {
        gc->xfb.noShare = (__GLsharedObjectMachine *)
            (*gc->imports.calloc)(gc, 1, sizeof(__GLsharedObjectMachine));

        /* Initialize a linear lookup table for XFB object */
        gc->xfb.noShare->maxLinearTableSize = __GL_MAX_XFBOBJ_LINEAR_TABLE_SIZE;
        gc->xfb.noShare->linearTableSize = __GL_DEFAULT_XFBOBJ_LINEAR_TABLE_SIZE;
        gc->xfb.noShare->linearTable = (GLvoid **)
            (*gc->imports.calloc)(gc, 1, gc->xfb.noShare->linearTableSize * sizeof(GLvoid *));

        gc->xfb.noShare->hashSize = __GL_XFB_HASH_TABLE_SIZE;
        gc->xfb.noShare->hashMask = __GL_XFB_HASH_TABLE_SIZE - 1;
        gc->xfb.noShare->refcount = 1;
        gc->xfb.noShare->deleteObject = (__GLdeleteObjectFunc)__glDeleteXfbObj;
        gc->xfb.noShare->immediateInvalid = GL_FALSE;
    }

    /* Initialize and bind the default XFB object */
    __glInitXfbObject(gc, &gc->xfb.defaultXfbObj, 0);
    gc->xfb.boundXfbObj = &gc->xfb.defaultXfbObj;
    gc->xfb.dirtyState = __GL_XFB_DIRTY_OBJECT;
}

GLvoid __glFreeXfbState(__GLcontext *gc)
{
    (*gc->dp.deleteXFB)(gc, &gc->xfb.defaultXfbObj);
    /* Free Transform feedback object table */
    __glFreeSharedObjectState(gc, gc->xfb.noShare);
}

GLvoid GL_APIENTRY __gles_GenTransformFeedbacks(__GLcontext *gc, GLsizei n, GLuint* ids)
{
    GLint start, i;

    __GL_HEADER();

    if (gcvNULL == ids || 0 == n)
    {
        __GL_EXIT();
    }

    if (n < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    start = __glGenerateNames(gc, gc->xfb.noShare, n);

    for (i = 0; i < n; i++)
    {
        ids[i] = start + i;
    }

    if (gc->xfb.noShare->linearTable)
    {
        __glCheckLinearTableSize(gc, gc->xfb.noShare, start + n);
    }

OnExit:
OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_DeleteTransformFeedbacks(__GLcontext *gc, GLsizei n, const GLuint* ids)
{
    GLint i;

    __GL_HEADER();

    if (n < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    for (i = 0; i < n; i++)
    {
        __glDeleteObject(gc, gc->xfb.noShare, ids[i]);
    }

OnError:
    __GL_FOOTER();
}

GLboolean GL_APIENTRY __gles_IsTransformFeedback(__GLcontext *gc, GLuint id)
{
    return (gcvNULL != __glGetObject(gc, gc->xfb.noShare, id));
}

GLvoid GL_APIENTRY __gles_BindTransformFeedback(__GLcontext *gc, GLenum target, GLuint id)
{
    __GL_HEADER();

    if (target != GL_TRANSFORM_FEEDBACK)
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    __glBindTransformFeedback(gc, id);

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_BeginTransformFeedback(__GLcontext *gc, GLenum primitiveMode)
{
    __GLxfbObject *xfbObj;
    __GLprogramObject *programObj;
    __GLBufBindPoint *pXfbBindingPoints;

    __GL_HEADER();

    switch (primitiveMode)
    {
    case GL_POINTS:
    case GL_LINES:
    case GL_TRIANGLES:
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    xfbObj = gc->xfb.boundXfbObj;
    if (xfbObj->active)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    programObj = __glGetLastNonFragProgram(gc);
    if (!programObj || !programObj->bindingInfo.numActiveXFB)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    pXfbBindingPoints = gc->xfb.boundXfbObj->boundBufBinding;
    if (programObj->bindingInfo.xfbMode == GL_INTERLEAVED_ATTRIBS)
    {
        if (!pXfbBindingPoints[0].boundBufName)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
    }
    else
    {
        GLuint index;
        for (index = 0; index < programObj->bindingInfo.numActiveXFB; ++index)
        {
            if (!pXfbBindingPoints[index].boundBufName)
            {
                __GL_ERROR_EXIT(GL_INVALID_OPERATION);
            }
        }
    }

    ++programObj->xfbRefCount;
    xfbObj->primMode = primitiveMode;
    xfbObj->active = GL_TRUE;
    xfbObj->offset = 0;
    xfbObj->programObj = programObj;

    (*gc->dp.beginXFB)(gc, xfbObj);

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_EndTransformFeedback(__GLcontext *gc)
{
    __GLxfbObject *xfbObj = gc->xfb.boundXfbObj;

    __GL_HEADER();

    if (!xfbObj->active)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (xfbObj->paused)
    {
        /* Implicit resume */
        xfbObj->paused = GL_FALSE;
    }
    else
    {
        GL_ASSERT(xfbObj->programObj == __glGetLastNonFragProgram(gc));
    }

    /* Needed in endXFB */
    xfbObj->active = GL_FALSE;

    (*gc->dp.endXFB)(gc, xfbObj);

    --xfbObj->programObj->xfbRefCount;
    xfbObj->programObj = gcvNULL;
    xfbObj->primMode = 0;

    if (xfbObj->flags & __GL_OBJECT_IS_DELETED)
    {
        __glDeleteXfbObj(gc, xfbObj);
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_PauseTransformFeedback(__GLcontext *gc)
{
    __GLxfbObject *xfbObj;

    __GL_HEADER();

    xfbObj = gc->xfb.boundXfbObj;
    if (!xfbObj->active || xfbObj->paused)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    (*gc->dp.pauseXFB)(gc);

    xfbObj->paused = GL_TRUE;

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ResumeTransformFeedback(__GLcontext *gc)
{
    __GLxfbObject *xfbObj;
    __GLprogramObject *programObj;

    __GL_HEADER();

    programObj = __glGetLastNonFragProgram(gc);
    xfbObj = gc->xfb.boundXfbObj;
    if (!xfbObj->active || !xfbObj->paused || xfbObj->programObj != programObj)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    (*gc->dp.resumeXFB)(gc);

    xfbObj->paused = GL_FALSE;

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_TransformFeedbackVaryings(__GLcontext *gc, GLuint program, GLsizei count,
                                                    const GLchar* const* varyings, GLenum bufferMode)
{
    GLuint i;
    __GLprogramObject *programObject;

    __GL_HEADER();

    if (count < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    switch (bufferMode)
    {
    case GL_INTERLEAVED_ATTRIBS:
        break;
    case GL_SEPARATE_ATTRIBS:
        if (count > (GLint)gc->constants.shaderCaps.maxXfbSeparateAttribs)
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    programObject = (__GLprogramObject*)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        if(programObject->objectInfo.objectType == __GL_SHADER_OBJECT_TYPE)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        else
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
    }

    /* Free previous name records */
    for (i = 0; i < programObject->xfbVaryingNum; i++)
    {
        GL_ASSERT(programObject->ppXfbVaryingNames[i]);
        (*gc->imports.free)(gc, programObject->ppXfbVaryingNames[i]);
    }

    if (programObject->ppXfbVaryingNames)
    {
        (*gc->imports.free)(gc, programObject->ppXfbVaryingNames);
    }
    programObject->ppXfbVaryingNames = gcvNULL;

    programObject->xfbMode       = bufferMode;
    programObject->xfbVaryingNum = count;

    if (count > 0)
    {
        programObject->ppXfbVaryingNames = (GLchar**)(*gc->imports.malloc)(gc, count*sizeof(GLchar*));
    }

    for (i = 0; i < (GLuint)count; i++)
    {
        GLuint nameLen = (GLuint)strlen(varyings[i]) + 1;
        programObject->ppXfbVaryingNames[i] = (GLchar*)(*gc->imports.malloc)(gc, nameLen);
        strcpy(programObject->ppXfbVaryingNames[i], varyings[i]);
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetTransformFeedbackVarying(__GLcontext *gc, GLuint program, GLuint index,
                                                      GLsizei bufSize, GLsizei* length, GLsizei* size,
                                                      GLenum* type, GLchar* name)
{
    __GLprogramObject *programObject;

    __GL_HEADER();

    programObject = (__GLprogramObject*)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (index >= programObject->bindingInfo.numActiveXFB)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    (*gc->dp.getXfbVarying)(gc, programObject, index, bufSize, length, size, type, name);

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetProgramInterfaceiv(__GLcontext *gc, GLuint program, GLenum programInterface, GLenum pname, GLint *params)
{
    __GLprogramObject * programObject;

    __GL_HEADER();

    if (!params)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    GL_ASSERT(gc->shaderProgram.spShared);

    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    switch (pname)
    {
    case GL_ACTIVE_RESOURCES:
    case GL_MAX_NAME_LENGTH:
    case GL_MAX_NUM_ACTIVE_VARIABLES:
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    switch (programInterface)
    {
    case GL_UNIFORM:
        switch (pname)
        {
        case GL_ACTIVE_RESOURCES:
            *params = programObject->bindingInfo.numActiveUniform;
            break;
        case GL_MAX_NAME_LENGTH:
            *params = programObject->bindingInfo.maxUniformNameLen;
            break;
        default:
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        break;

    case GL_UNIFORM_BLOCK:
        switch (pname)
        {
        case GL_ACTIVE_RESOURCES:
            *params = programObject->bindingInfo.numActiveUB;
            break;
        case GL_MAX_NAME_LENGTH:
            *params = programObject->bindingInfo.maxUBNameLen;
            break;
        case GL_MAX_NUM_ACTIVE_VARIABLES:
            *params = programObject->bindingInfo.maxActiveUniforms;
            break;
        default:
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        break;

    case GL_PROGRAM_INPUT:
        switch (pname)
        {
        case GL_ACTIVE_RESOURCES:
            *params = programObject->bindingInfo.numActiveInput;
            break;
        case GL_MAX_NAME_LENGTH:
            *params = programObject->bindingInfo.maxInputNameLen;
            break;
        default:
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        break;

    case GL_PROGRAM_OUTPUT:
        switch (pname)
        {
        case GL_ACTIVE_RESOURCES:
            *params = programObject->bindingInfo.numActiveOutput;
            break;
        case GL_MAX_NAME_LENGTH:
            *params = programObject->bindingInfo.maxOutputNameLen;
            break;
            break;
        default:
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        break;

    case GL_TRANSFORM_FEEDBACK_VARYING:
        switch (pname)
        {
        case GL_ACTIVE_RESOURCES:
            *params = programObject->bindingInfo.numActiveXFB;
            break;
        case GL_MAX_NAME_LENGTH:
            *params = programObject->bindingInfo.maxXFBNameLen;
            break;
        default:
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        break;

    case GL_ATOMIC_COUNTER_BUFFER:
        switch (pname)
        {
        case GL_ACTIVE_RESOURCES:
            *params = programObject->bindingInfo.numActiveACBs;
            break;
        case GL_MAX_NUM_ACTIVE_VARIABLES:
            *params = programObject->bindingInfo.maxActiveACs;
            break;
        default:
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        break;

    case GL_BUFFER_VARIABLE:
        switch (pname)
        {
        case GL_ACTIVE_RESOURCES:
            *params = programObject->bindingInfo.numActiveBV;
            break;
        case GL_MAX_NAME_LENGTH:
            *params = programObject->bindingInfo.maxBVNameLen;
            break;
        default:
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        break;

    case GL_SHADER_STORAGE_BLOCK:
        switch (pname)
        {
        case GL_ACTIVE_RESOURCES:
            *params = programObject->bindingInfo.numActiveSSB;
            break;
        case GL_MAX_NAME_LENGTH:
            *params = programObject->bindingInfo.maxSSBNameLen;
            break;
        case GL_MAX_NUM_ACTIVE_VARIABLES:
            *params = programObject->bindingInfo.maxActiveBVs;
            break;
        default:
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

OnError:
    __GL_FOOTER();
}

GLuint GL_APIENTRY __gles_GetProgramResourceIndex(__GLcontext *gc, GLuint program, GLenum programInterface, const GLchar *name)
{
    GLuint resIndex = 0;
    __GLprogramObject * programObject;

    __GL_HEADER();

    GL_ASSERT(gc->shaderProgram.spShared);

    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    switch (programInterface)
    {
    case GL_UNIFORM:
        (*gc->dp.getUniformIndices)(gc, programObject, 1, &name, &resIndex);
        break;
    case GL_UNIFORM_BLOCK:
        resIndex = (*gc->dp.getUniformBlockIndex)(gc, programObject, name);
        break;
    case GL_PROGRAM_INPUT:
    case GL_PROGRAM_OUTPUT:
    case GL_TRANSFORM_FEEDBACK_VARYING:
    case GL_BUFFER_VARIABLE:
    case GL_SHADER_STORAGE_BLOCK:
        resIndex = (*gc->dp.getProgramResourceIndex)(gc, programObject, programInterface, name);
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

OnError:
    __GL_FOOTER();

    return resIndex;
}

GLvoid GL_APIENTRY __gles_GetProgramResourceName(__GLcontext *gc, GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name)
{
    __GLprogramObject * programObject;

    __GL_HEADER();

    if (bufSize < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    GL_ASSERT(gc->shaderProgram.spShared);

    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    switch (programInterface)
    {
    case GL_UNIFORM:
        if (index >= programObject->bindingInfo.numActiveUniform)
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
        (*gc->dp.getActiveUniform)(gc, programObject, index, bufSize, length, gcvNULL, gcvNULL, name);
        break;
    case GL_UNIFORM_BLOCK:
        if (index >= programObject->bindingInfo.numActiveUB)
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
        (*gc->dp.getActiveUniformBlockName)(gc, programObject, index, bufSize, length, name);
        break;
    case GL_TRANSFORM_FEEDBACK_VARYING:
        if (index >= programObject->bindingInfo.numActiveXFB)
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
        (*gc->dp.getXfbVarying)(gc, programObject, index, bufSize, length, gcvNULL, gcvNULL, name);
        break;
    case GL_PROGRAM_INPUT:
    case GL_PROGRAM_OUTPUT:
    case GL_BUFFER_VARIABLE:
    case GL_SHADER_STORAGE_BLOCK:
        (*gc->dp.getProgramResourceName)(gc, programObject, programInterface, index, bufSize, length, name);
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetProgramResourceiv(__GLcontext *gc, GLuint program, GLenum programInterface,
                                               GLuint index, GLsizei propCount, const GLenum *props,
                                               GLsizei bufSize, GLsizei *length, GLint *params)
{
    GLsizei i;
    __GLprogramObject * programObject;

    __GL_HEADER();

    GL_ASSERT(gc->shaderProgram.spShared);

    if (propCount <= 0 || bufSize <0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    /* Check disallowed props and report GL_INVALID_ENUM */
    for (i = 0; i < propCount; ++i)
    {
        switch (props[i])
        {
        case GL_LOCATION:
        case GL_TYPE:
        case GL_ARRAY_SIZE:
        case GL_OFFSET:
        case GL_NAME_LENGTH:
        case GL_BLOCK_INDEX:
        case GL_ARRAY_STRIDE:
        case GL_MATRIX_STRIDE:
        case GL_IS_ROW_MAJOR:
        case GL_ACTIVE_VARIABLES:
        case GL_BUFFER_BINDING:
        case GL_NUM_ACTIVE_VARIABLES:
        case GL_BUFFER_DATA_SIZE:
        case GL_REFERENCED_BY_VERTEX_SHADER:
        case GL_REFERENCED_BY_FRAGMENT_SHADER:
        case GL_REFERENCED_BY_COMPUTE_SHADER:
        case GL_ATOMIC_COUNTER_BUFFER_INDEX:
        case GL_TOP_LEVEL_ARRAY_SIZE:
        case GL_TOP_LEVEL_ARRAY_STRIDE:
        case GL_REFERENCED_BY_TESS_CONTROL_SHADER_EXT:
        case GL_REFERENCED_BY_TESS_EVALUATION_SHADER_EXT:
        case GL_REFERENCED_BY_GEOMETRY_SHADER_EXT:
        case GL_IS_PER_PATCH_OES:
            break;
        default:
            __GL_ERROR_EXIT(GL_INVALID_ENUM);
        }
    }

    (*gc->dp.getProgramResourceiv)(gc, programObject, programInterface, index,
                                   propCount, props, bufSize, length, params);

OnError:
    __GL_FOOTER();
}

GLint GL_APIENTRY __gles_GetProgramResourceLocation(__GLcontext *gc, GLuint program, GLenum programInterface, const GLchar *name)
{
    GLint location = -1;
    __GLprogramObject * programObject;

    __GL_HEADER();

    GL_ASSERT(gc->shaderProgram.spShared);

    programObject = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
    if (!programObject)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    else if (programObject->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }
    else if (!programObject->programInfo.linkedStatus)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    switch (programInterface)
    {
    case GL_UNIFORM:
        location = (*gc->dp.getUniformLocation)(gc, programObject, name);
        break;
    case GL_PROGRAM_INPUT:
        location = (*gc->dp.getAttributeLocation)(gc, programObject, name);
        break;
    case GL_PROGRAM_OUTPUT:
        location = (*gc->dp.getFragDataLocation)(gc, programObject, name);
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

OnError:
    __GL_FOOTER();

    return location;
}

GLvoid GL_APIENTRY __gles_DispatchCompute(__GLcontext *gc, GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z)
{
    __GL_HEADER();

    if (num_groups_x > gc->constants.shaderCaps.maxWorkGroupCount[0] ||
        num_groups_y > gc->constants.shaderCaps.maxWorkGroupCount[1] ||
        num_groups_z > gc->constants.shaderCaps.maxWorkGroupCount[2])
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    gc->compute.num_groups_x = num_groups_x;
    gc->compute.num_groups_y = num_groups_y;
    gc->compute.num_groups_z = num_groups_z;

    gc->compute.indirect = GL_FALSE;

    __glDispatchCompute(gc);

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_DispatchComputeIndirect(__GLcontext *gc, GLintptr indirect)
{
    __GLbufferObject *indirectObj = gc->bufferObject.generalBindingPoint[__GL_DISPATCH_INDIRECT_BUFFER_INDEX].boundBufObj;

    __GL_HEADER();

    if (indirect < 0 || (indirect & 0x3))
    {
        /* ES_SPEC 3.1 Chapter 17 Compute Shaders
         * An INVALID_VALUE error is generated if indirect is negative or
         * is not a multiple of the size, in basic machine units, of uint.
         */
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (!indirectObj || indirectObj->bufferMapped)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (indirect >= indirectObj->size ||
        (GLsizeiptr)(indirect + sizeof(__GLcomputeIndirectCmd)) >  indirectObj->size)
    {
        /* An INVALID_OPERATION error is generated if the command would
         * source data beyond the end of the buffer object.
         */
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    gc->compute.indirect = GL_TRUE;
    gc->compute.offset = indirect;

    __glDispatchCompute(gc);

OnError:
    __GL_FOOTER();
}

GLuint GL_APIENTRY __gles_CreateShaderProgramv(__GLcontext *gc, GLenum type, GLsizei count, const GLchar *const*strings)
{
    GLsizei i = 0;
    GLuint shader = 0, program = 0;
    GLuint uniqueId;
    __GLshaderObject *shaderObj = gcvNULL;
    __GLprogramObject *progObj  = gcvNULL;
    GLboolean success = GL_FALSE;

    __GL_HEADER();

    switch (type)
    {
    case GL_VERTEX_SHADER:
    case GL_FRAGMENT_SHADER:
    case GL_COMPUTE_SHADER:
        break;
    case GL_TESS_CONTROL_SHADER_EXT:
    case GL_TESS_EVALUATION_SHADER_EXT:
        if (__glExtension[__GL_EXTID_EXT_tessellation_shader].bEnabled)
        {
            break;
        }
    case GL_GEOMETRY_SHADER_EXT:
        if (__glExtension[__GL_EXTID_EXT_geometry_shader].bEnabled)
        {
            break;
        }
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (count < 0 || !strings)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    /* Create shader object */
    GL_ASSERT(gc->shaderProgram.spShared);
    shader = __glGenerateNames(gc, gc->shaderProgram.spShared, 1);
    __glMarkNameUsed(gc, gc->shaderProgram.spShared, shader);
    shaderObj = (__GLshaderObject *)(*gc->imports.calloc)(gc, 1, sizeof(__GLshaderObject));
    if (!shaderObj)
    {
        __GL_ERROR_EXIT(GL_OUT_OF_MEMORY);
    }
    __glInitShaderObject(gc, shaderObj, type, shader);
    __glAddObject(gc, gc->shaderProgram.spShared, shader, shaderObj);

    /* Create program object */
    program = __glGenerateNames(gc, gc->shaderProgram.spShared, 1);
    uniqueId = __glMarkNameUsed(gc, gc->shaderProgram.spShared, program);
    progObj = (__GLprogramObject *)(*gc->imports.calloc)(gc, 1, sizeof(__GLprogramObject));
    if (!progObj)
    {
        __GL_ERROR_EXIT(GL_OUT_OF_MEMORY);
    }
    __glInitProgramObject(gc, progObj, program, uniqueId);
    __glAddObject(gc, gc->shaderProgram.spShared, program, progObj);
    if (!(*gc->dp.createProgram)(gc, progObj))
    {
        __GL_ERROR_EXIT((*gc->dp.getError)(gc));
    }

    /* Calculate source length */
    for (i = 0; i < count; ++i)
    {
        if (strings[i])
        {
            shaderObj->shaderInfo.sourceSize += (GLsizei)strlen(strings[i]);
        }
    }

    /* Assemble all sources into one string*/
    shaderObj->shaderInfo.source = (GLchar *)(*gc->imports.malloc)(gc, shaderObj->shaderInfo.sourceSize + 1);
    if (!shaderObj->shaderInfo.source)
    {
        __GL_ERROR_EXIT(GL_OUT_OF_MEMORY);
    }

    shaderObj->shaderInfo.source[0] = '\0';
    for (i = 0; i < count; ++i)
    {
        if (strings[i])
        {
            strcat(shaderObj->shaderInfo.source, strings[i]);
        }
    }

    /* Compile shader */
    if ((*gc->dp.compileShader)(gc, shaderObj))
    {
        __GLSLStage stage = __glGetShaderStage(type);

        /* Attach Shader */
        progObj->programInfo.attachedShader[stage] = shaderObj;

        /* Set separable */
        progObj->programInfo.separable = GL_TRUE;

        /* Link program */
        progObj->programInfo.codeSeq++;
        progObj->programInfo.linkedStatus = (*gc->dp.linkProgram)(gc, progObj);
        if (progObj->programInfo.linkedStatus)
        {
            progObj->bindingInfo.isSeparable = progObj->programInfo.separable;
            progObj->bindingInfo.activeShaderID[stage] = shader;
        }

        /* Detach Shader */
        progObj->programInfo.attachedShader[stage] = gcvNULL;
    }

    success = GL_TRUE;

OnError:
    /* Append shader infoLog to program infoLog */
    if (shaderObj && progObj)
    {
        if (shaderObj->shaderInfo.compiledLog && progObj->programInfo.infoLog)
        {
            gcoOS_StrCatSafe(progObj->programInfo.infoLog, __GLSL_LOG_INFO_SIZE, shaderObj->shaderInfo.compiledLog);
        }
    }

    /* Delete shader always */
    if (shader)
    {
        __glDeleteObject(gc, gc->shaderProgram.spShared, shader);
    }

    /* Delete program if failed */
    if (!success && program)
    {
        __glDeleteObject(gc, gc->shaderProgram.spShared, program);
        program = 0;
    }

    __GL_FOOTER();

    return program;
}


/*
** Program Pipeline Object
*/

GLvoid __glInitProgramPipelineObject(__GLcontext *gc, __GLprogramPipelineObject *ppObj, GLuint pipeline)
{
    ppObj->name = pipeline;
    ppObj->infoLog = (GLchar*)(*gc->imports.calloc)(gc, __GLSL_LOG_INFO_SIZE, sizeof(GLchar));
}

__GLprogramPipelineObject* __glGetProgramPipelineObject(__GLcontext *gc, GLuint pipeline)
{
    __GLprogramPipelineObject *ppObj = gcvNULL;

    if (!__glIsNameDefined(gc, gc->shaderProgram.ppNoShare, pipeline))
    {
        __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, gcvNULL);
    }

    ppObj = (__GLprogramPipelineObject *)__glGetObject(gc, gc->shaderProgram.ppNoShare, pipeline);
    if (ppObj == gcvNULL)
    {
        ppObj = (__GLprogramPipelineObject*)(*gc->imports.calloc)(gc, 1, sizeof(__GLprogramPipelineObject));

        __glInitProgramPipelineObject(gc, ppObj, pipeline);
        __glAddObject(gc, gc->shaderProgram.ppNoShare, pipeline, ppObj);
    }

    return ppObj;
}

GLvoid __glBindProgramPipeline(__GLcontext *gc, GLuint pipeline)
{
    __GLprogramPipelineObject *ppObj;
    GLuint boundPPName = gc->shaderProgram.boundPPO ? gc->shaderProgram.boundPPO->name : 0;

    if (boundPPName == pipeline)
    {
        return;
    }

    if (pipeline > 0)
    {
        ppObj = __glGetProgramPipelineObject(gc, pipeline);
        if (ppObj == gcvNULL)
        {
            return;
        }
    }
    else
    {
        ppObj = gcvNULL;
    }

    gc->shaderProgram.boundPPO = ppObj;

    if (!gc->shaderProgram.currentProgram)
    {
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_GLSL_PROGRAM_SWITCH);
    }
}

GLvoid __glUseProgramStages(__GLcontext *gc, __GLprogramPipelineObject* ppObj, __GLSLStage stage, __GLprogramObject* progObj, GLenum dirty)
{
    __GLprogramObject *oldProgObj;

    GL_ASSERT(ppObj);
    GL_ASSERT(stage < __GLSL_STAGE_LAST);

    oldProgObj = ppObj->stageProgs[stage];
    if (oldProgObj == progObj)
    {
        return;
    }

    if (oldProgObj)
    {
        if ((--oldProgObj->objectInfo.bindCount) == 0 && oldProgObj->programInfo.deleteStatus)
        {
            __glDeleteObject(gc, gc->shaderProgram.spShared, oldProgObj->objectInfo.id);
        }
    }

    /* If this programObject do have this stage, install it */
    if (progObj && progObj->bindingInfo.activeShaderID[stage])
    {
        ppObj->stageProgs[stage] = progObj;
        progObj->objectInfo.bindCount++;
    }
    else
    {
        ppObj->stageProgs[stage]  = NULL;
    }

    /* If the PPO is currently in use */
    if ((!gc->shaderProgram.currentProgram) && (gc->shaderProgram.boundPPO == ppObj))
    {
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, dirty);
    }
}

GLvoid __glActiveShaderProgram(__GLcontext *gc, __GLprogramPipelineObject* ppObj, __GLprogramObject* progObj)
{
    __GLprogramObject *oldProgObj;

    GL_ASSERT(ppObj);

    oldProgObj = ppObj->activeProg;
    if (oldProgObj == progObj)
    {
        return;
    }

    if (oldProgObj)
    {
        if ((--oldProgObj->objectInfo.bindCount) == 0 && oldProgObj->programInfo.deleteStatus)
        {
            __glDeleteObject(gc, gc->shaderProgram.spShared, oldProgObj->objectInfo.id);
        }
    }

    if (progObj)
    {
        progObj->objectInfo.bindCount++;
    }

    ppObj->activeProg = progObj;
}

GLboolean __glDeleteProgramPipelineObj(__GLcontext *gc, __GLprogramPipelineObject *ppObj)
{
    GL_ASSERT(ppObj);

    /* unbound first if current */
    if (gc->shaderProgram.boundPPO == ppObj)
    {
        __glBindProgramPipeline(gc, 0);
    }

    __glUseProgramStages(gc, ppObj, __GLSL_STAGE_VS, gcvNULL, __GL_DIRTY_GLSL_VS_SWITCH);
    __glUseProgramStages(gc, ppObj, __GLSL_STAGE_FS, gcvNULL, __GL_DIRTY_GLSL_FS_SWITCH);
    __glUseProgramStages(gc, ppObj, __GLSL_STAGE_CS, gcvNULL, __GL_DIRTY_GLSL_CS_SWITCH);
    __glUseProgramStages(gc, ppObj, __GLSL_STAGE_TCS, gcvNULL, __GL_DIRTY_GLSL_TCS_SWITCH);
    __glUseProgramStages(gc, ppObj, __GLSL_STAGE_TES, gcvNULL, __GL_DIRTY_GLSL_TCS_SWITCH);
    __glUseProgramStages(gc, ppObj, __GLSL_STAGE_GS, gcvNULL, __GL_DIRTY_GLSL_GS_SWITCH);

    __glActiveShaderProgram(gc, ppObj, gcvNULL);

    if (ppObj->label)
    {
        gc->imports.free(gc, ppObj->label);
    }

    if (ppObj->infoLog)
    {
        (*gc->imports.free)(gc, ppObj->infoLog);
        ppObj->infoLog = gcvNULL;
    }

    (*gc->imports.free)(gc, ppObj);

    return GL_TRUE;
}

GLvoid GL_APIENTRY __gles_GenProgramPipelines(__GLcontext *gc, GLsizei n, GLuint *pipelines)
{
    GLint start, i;

    __GL_HEADER();

    if (n < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (NULL == pipelines)
    {
        __GL_EXIT();
    }

    GL_ASSERT(NULL != gc->shaderProgram.ppNoShare);

    start = __glGenerateNames(gc, gc->shaderProgram.ppNoShare, n);

    for (i = 0; i < n; i++)
    {
        pipelines[i] = start + i;
    }

    if (gc->shaderProgram.ppNoShare->linearTable)
    {
        __glCheckLinearTableSize(gc, gc->shaderProgram.ppNoShare, (start + n));
    }

OnExit:
OnError:
    __GL_FOOTER();
}

GLboolean GL_APIENTRY __gles_IsProgramPipeline(__GLcontext *gc, GLuint pipeline)
{
    return (gcvNULL != __glGetObject(gc, gc->shaderProgram.ppNoShare, pipeline));
}

GLvoid GL_APIENTRY __gles_DeleteProgramPipelines(__GLcontext *gc, GLsizei n, const GLuint *pipelines)
{
    GLsizei i;

    __GL_HEADER();

    if (n < 0 )
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    for (i = 0; i < n; i++)
    {
        if (pipelines[i])
        {
            __glDeleteObject(gc, gc->shaderProgram.ppNoShare, pipelines[i]);
        }
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_BindProgramPipeline(__GLcontext *gc, GLuint pipeline)
{
    __GL_HEADER();

    __glBindProgramPipeline(gc, pipeline);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetProgramPipelineiv(__GLcontext *gc, GLuint pipeline, GLenum pname, GLint *params)
{
    __GLprogramPipelineObject *ppObj = __glGetProgramPipelineObject(gc, pipeline);

    __GL_HEADER();

    if (!ppObj)
    {
        __GL_EXIT();
    }

    switch (pname)
    {
    case GL_ACTIVE_PROGRAM:
        *params = ppObj->activeProg ? ppObj->activeProg->objectInfo.id : 0;
        break;
    case GL_VERTEX_SHADER:
        *params = ppObj->stageProgs[__GLSL_STAGE_VS] ? ppObj->stageProgs[__GLSL_STAGE_VS]->objectInfo.id : 0;
        break;
    case GL_FRAGMENT_SHADER:
        *params = ppObj->stageProgs[__GLSL_STAGE_FS] ? ppObj->stageProgs[__GLSL_STAGE_FS]->objectInfo.id : 0;
        break;
    case GL_COMPUTE_SHADER:
        *params = ppObj->stageProgs[__GLSL_STAGE_CS] ? ppObj->stageProgs[__GLSL_STAGE_CS]->objectInfo.id : 0;
        break;
    case GL_VALIDATE_STATUS:
        *params = ppObj->validateStatus;
        break;
    case GL_INFO_LOG_LENGTH:
        {
            GLchar *pLog = ppObj->infoLog;
            *params = (pLog && pLog[0] != '\0') ? (GLint)(strlen(pLog) + 1) : 0;
        }
        break;
    case GL_TESS_CONTROL_SHADER_EXT:
        *params = ppObj->stageProgs[__GLSL_STAGE_TCS] ? ppObj->stageProgs[__GLSL_STAGE_TCS]->objectInfo.id : 0;
        break;
    case GL_TESS_EVALUATION_SHADER_EXT:
        *params = ppObj->stageProgs[__GLSL_STAGE_TES] ? ppObj->stageProgs[__GLSL_STAGE_TES]->objectInfo.id : 0;
        break;
    case GL_GEOMETRY_SHADER_EXT:
        *params = ppObj->stageProgs[__GLSL_STAGE_GS] ? ppObj->stageProgs[__GLSL_STAGE_GS]->objectInfo.id : 0;
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

OnExit:
OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetProgramPipelineInfoLog(__GLcontext *gc, GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
    GLsizei len = 0;
    __GLprogramPipelineObject *ppObj;

    __GL_HEADER();

    if (bufSize < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    ppObj = (__GLprogramPipelineObject *)__glGetObject(gc, gc->shaderProgram.ppNoShare, pipeline);
    if (ppObj == gcvNULL)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    /* According to spec, the returned string should be null-terminated,
    ** but "length" returned should exclude null-terminator.
    */
    if (infoLog && bufSize > 0)
    {
        if (ppObj->infoLog)
        {
            GLsizei length = (GLsizei)strlen(ppObj->infoLog);
            len = __GL_MIN(length, bufSize - 1);
        }
        else
        {
            len = 0;
        }

        if (len > 0)
        {
            memcpy(infoLog, ppObj->infoLog, len);
        }
        infoLog[len] = '\0';
    }

    if (length)
    {
        *length = len;
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ValidateProgramPipeline(__GLcontext *gc, GLuint pipeline)
{
    __GLprogramPipelineObject *ppObj = __glGetProgramPipelineObject(gc, pipeline);

    __GL_HEADER();

    if (!ppObj)
    {
        __GL_EXIT();
    }

    /* Clear the log buffer*/
    ppObj->infoLog[0] = '\0';

    (*gc->dp.validateProgramPipeline)(gc, ppObj, GL_FALSE);

OnExit:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_UseProgramStages(__GLcontext *gc, GLuint pipeline, GLbitfield stages, GLuint program)
{
    __GLprogramObject * progObj;
    __GLprogramPipelineObject *ppObj;
    GLbitfield allowedStages = GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT | GL_COMPUTE_SHADER_BIT;

    __GL_HEADER();

    if (__glExtension[__GL_EXTID_EXT_tessellation_shader].bEnabled)
    {
        allowedStages |= GL_TESS_CONTROL_SHADER_BIT_EXT | GL_TESS_EVALUATION_SHADER_BIT_EXT;
    }

    if (__glExtension[__GL_EXTID_EXT_geometry_shader].bEnabled)
    {
        allowedStages |= GL_GEOMETRY_SHADER_BIT_EXT;
    }

    if (stages != GL_ALL_SHADER_BITS && (stages & ~(allowedStages)))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (program)
    {
        GL_ASSERT(gc->shaderProgram.spShared);
        progObj = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
        if (!progObj)
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
        else if (progObj->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        else if (!progObj->bindingInfo.isSeparable || !progObj->programInfo.linkedStatus)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
    }
    else
    {
        progObj = gcvNULL;
    }

    ppObj = __glGetProgramPipelineObject(gc, pipeline);
    if (!ppObj)
    {
        __GL_EXIT();
    }

    if (stages & GL_VERTEX_SHADER_BIT)
    {
        __glUseProgramStages(gc, ppObj, __GLSL_STAGE_VS, progObj, __GL_DIRTY_GLSL_VS_SWITCH);
    }

    if (stages & GL_TESS_CONTROL_SHADER_BIT_EXT)
    {
        __glUseProgramStages(gc, ppObj, __GLSL_STAGE_TCS, progObj, __GL_DIRTY_GLSL_TCS_SWITCH);
    }

    if (stages & GL_TESS_EVALUATION_SHADER_BIT_EXT)
    {
        __glUseProgramStages(gc, ppObj, __GLSL_STAGE_TES, progObj, __GL_DIRTY_GLSL_TES_SWITCH);
    }

    if (stages & GL_GEOMETRY_SHADER_BIT_EXT)
    {
        __glUseProgramStages(gc, ppObj, __GLSL_STAGE_GS, progObj, __GL_DIRTY_GLSL_GS_SWITCH);
    }

    if (stages & GL_FRAGMENT_SHADER_BIT)
    {
        __glUseProgramStages(gc, ppObj, __GLSL_STAGE_FS, progObj, __GL_DIRTY_GLSL_FS_SWITCH);
    }

    if (stages & GL_COMPUTE_SHADER_BIT)
    {
        __glUseProgramStages(gc, ppObj, __GLSL_STAGE_CS, progObj, __GL_DIRTY_GLSL_CS_SWITCH);
    }

OnExit:
OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ActiveShaderProgram(__GLcontext *gc, GLuint pipeline, GLuint program)
{
    __GLprogramObject *progObj;
    __GLprogramPipelineObject *ppObj;

    __GL_HEADER();

    if (program)
    {
        GL_ASSERT(gc->shaderProgram.spShared);
        progObj = (__GLprogramObject *)__glGetObject(gc, gc->shaderProgram.spShared, program);
        if (!progObj)
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
        else if (progObj->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        else if (!progObj->programInfo.linkedStatus)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
    }
    else
    {
        progObj = gcvNULL;
    }

    ppObj = __glGetProgramPipelineObject(gc, pipeline);
    if (!ppObj)
    {
        __GL_EXIT();
    }

    __glActiveShaderProgram(gc, ppObj, progObj);

OnExit:
OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_PatchParameteri(__GLcontext *gc, GLenum pname, GLint value)
{
    __GL_HEADER();

    if (GL_PATCH_VERTICES_EXT != pname)
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (value <= 0 || value > (GLint)gc->constants.shaderCaps.maxTessPatchVertices)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (gc->shaderProgram.patchVertices != value)
    {
        gc->shaderProgram.patchVertices = value;
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_GLSL_PATCH_VERTICES);
    }

OnError:
    __GL_FOOTER();
}


