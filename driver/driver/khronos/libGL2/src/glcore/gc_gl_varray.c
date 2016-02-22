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
#include "gc_gl_debug.h"

extern GLvoid __glDrawArrayPrimitive(__GLcontext *gc, GLenum mode);
extern GLvoid __glSelectImmedDrawArraysFn(__GLcontext *gc);


/******utility functions*********/

GLvoid __glGetIndexRange(__GLcontext *gc, const GLvoid *indices, GLuint count, GLuint indextype, GLuint* start, GLuint *end )
{
    GLuint i;

    switch(indextype)
    {
    case GL_UNSIGNED_BYTE:
        {
            GLubyte *ubp=(GLubyte*)indices, ubMax=0, ubMin=0xFF;
            for(i=0;i<count;i++)
            {if(ubMax<ubp[i])
            ubMax = ubp[i];
            if(ubMin>ubp[i])
                ubMin = ubp[i];}
            *start = (GLuint)ubMin;
            *end = (GLuint)ubMax;
            break;
        }
    case GL_UNSIGNED_SHORT:
        {
            GLushort *usp=(GLushort*)indices, usMax=0, usMin=0xFFFF;
            for(i=0;i<count;i++)
            {if(usMax<usp[i])
            usMax = usp[i];
            if(usMin>usp[i])
                usMin = usp[i];}
            *start = (GLuint)usMin;
            *end =  (GLuint)usMax;
            break;
        }
    case GL_UNSIGNED_INT:
        {
            GLuint * uip=(GLuint*)indices, uiMax=0, uiMin=0xFFFFFFFF;
            for(i=0;i<count;i++)
            {if(uiMax<uip[i])
            uiMax = uip[i];
            if(uiMin>uip[i])
                uiMin = uip[i];}
            *start = (GLuint)uiMin;
            *end = (GLuint)uiMax;
            break;
        }
    default:
        GL_ASSERT(0);
    }
    *end += 1;
}


/*
** validate vertex array
** select immedia mode draw array function pointers
*/
__GL_INLINE GLvoid __glValidateVertexArrays(__GLcontext *gc)
{
    GLuint currentEnabled;

    /* Filter out the vertex arrays that are not really needed */
    currentEnabled = (gc->clientState.vertexArray.arrayEnabled & gc->input.requiredInputMask);
    /* requiredInputMask  set only bit 0 on for vertexinput, while vertexarray enable bit are saparate for vertex and attrib 0.
    Case attribarray 0 enable, vertexarray disabled. attribarray 0 bit will be cleared.
    Solution: set on attrib 0 when it's enabled and vertex input is required. */
    if(gc->input.requiredInputMask & __GL_VARRAY_VERTEX && gc->clientState.vertexArray.arrayEnabled & __GL_VARRAY_ATT0)
        currentEnabled |= __GL_VARRAY_ATT0;

    if (currentEnabled != gc->clientState.vertexArray.currentEnabled)
    {
        __GL_SET_VARRAY_ENABLE_BIT(gc);
        gc->clientState.vertexArray.currentEnabled = currentEnabled;
    }
    else
    {
        gc->vertexArray.globalDirty &= (~__GL_DIRTY_VARRAY_ENABLE_BIT);
    }

    if (gc->vertexArray.globalDirty & (__GL_DIRTY_VARRAY_FORMAT_BIT |
        __GL_DIRTY_VARRAY_ENABLE_BIT |
        __GL_DIRTY_VARRAY_STOP_CACHE_BIT))
    {
        /* Validate DrawArrays/ArrayElement function */
        gc->vertexArray.drawArraysFunc = __glim_DrawArrays;
        gc->vertexArray.drawElementsFunc = __glim_DrawElements;
        gc->vertexArray.arrayElementFunc = __glim_ArrayElement;

        gc->vertexArray.globalDirty &= (~__GL_DIRTY_VARRAY_STOP_CACHE_BIT);
    }

    /* Finally, Clear the vertex array dirty bits except __GL_DIRTY_VARRAY_STOP_CACHE_BIT */
    gc->vertexArray.globalDirtyBackup = gc->vertexArray.globalDirty;
    gc->vertexArray.globalDirty &= __GL_DIRTY_VARRAY_STOP_CACHE_BIT;
}

/*
** validate vertex array, bypass cache logic
*/
__GL_INLINE GLvoid __glValidateVertexArraysInstancedEXT(__GLcontext *gc)
{
    GLuint currentEnabled;

    /* Filter out the vertex arrays that are not really needed */
    currentEnabled = (gc->clientState.vertexArray.arrayEnabled & gc->input.requiredInputMask);
    /* requiredInputMask  set only bit 0 on for vertexinput, while vertexarray enable bit are saparate for vertex and attrib 0.
    Case attribarray 0 enable, vertexarray disabled. attribarray 0 bit will be cleared.
    Solution: set on attrib 0 when it's enabled and vertex input is required. */
    if(gc->input.requiredInputMask & __GL_VARRAY_VERTEX && gc->clientState.vertexArray.arrayEnabled & __GL_VARRAY_ATT0)
        currentEnabled |= __GL_VARRAY_ATT0;

    if (currentEnabled != gc->clientState.vertexArray.currentEnabled)
    {
        __GL_SET_VARRAY_ENABLE_BIT(gc);
        gc->clientState.vertexArray.currentEnabled = currentEnabled;
    }
    else
    {
        gc->vertexArray.globalDirty &= (~__GL_DIRTY_VARRAY_ENABLE_BIT);
    }

    /* Finally, Clear the vertex array dirty bits */
    gc->vertexArray.globalDirtyBackup = gc->vertexArray.globalDirty;
    gc->vertexArray.globalDirty &= __GL_DIRTY_VARRAY_STOP_CACHE_BIT;
}

GLvoid __glValidateArrayStreamConfigPath(__GLcontext *gc)
{
    GLbitfield globalDirty = (gc->vertexArray.globalDirtyBackup | gc->vertexArray.globalDirty );
    /*
    ** If inputMaskChanged is TRUE, __glValidateDrawArrays must be called before
    */
    GL_ASSERT(gc->input.inputMaskChanged == GL_FALSE);
    gc->vertexArray.fastStreamSetup = GL_TRUE;
    if (globalDirty & (__GL_DIRTY_VARRAY_FORMAT_BIT|__GL_DIRTY_VARRAY_ENABLE_BIT))
    {
        /* If ENABLE/FORMAT changed, we need slow stream setup*/
        gc->vertexArray.fastStreamSetup = GL_FALSE;
    }
    else if( globalDirty & __GL_DIRTY_VARRAY_OFFSET_BIT )
    {
        GLuint streamIdx, elementIdx, arrayIdx, bufBinding;
        __GLstreamDecl *stream;
        /*
        ** gc->vertexStreams has the stream information of last draw, which will be used
        ** to determine if the "layout" of the stream could change. If the stream layout will
        ** not change, we could do fast stream setup. __glConfigArrayVertexStream() will
        ** make sure that if the last draw is not vertex array mode, the fast stream setup
        ** will not be entered.
        */
        for (streamIdx = 0; streamIdx < gc->vertexStreams.numStreams; streamIdx++ )
        {
            stream = &gc->vertexStreams.streams[streamIdx];
            arrayIdx = stream->streamElement[0].inputIndex;
            bufBinding = gc->clientState.vertexArray.currentArrays[arrayIdx].bufBinding;
            for ( elementIdx = 0; elementIdx < stream->numElements; elementIdx++ )
            {
                arrayIdx = stream->streamElement[elementIdx].inputIndex;
                /*
                ** If the elements are in the same buffer object, and the offset is also the same,
                ** we could do fast stream setup
                */
                if ( (bufBinding != gc->clientState.vertexArray.currentArrays[arrayIdx].bufBinding) ||
                     (stream->streamElement[elementIdx].offset != gc->clientState.vertexArray.currentArrays[arrayIdx].offset) )
                {
                    gc->vertexArray.fastStreamSetup = GL_FALSE;
                    goto finish;
                }
            }
        }
    }

finish:
    gc->vertexArray.globalDirtyBackup = 0;
}


/******End utility functions**************/

/*
** Update the state of a vertex array, and set gc->vertexArray.globalDirty if state changes
** gc->vertexArray.globalDirty will triger vertex array validation
**
*/
static GLvoid  __glUpdateVertexArray(__GLcontext *gc,
                                     GLuint arrayIdx,
                                     GLint size,
                                     GLenum type,
                                     GLboolean normalized,
                                     GLboolean integer,
                                     GLsizei stride,
                                     const GLvoid *pointer)
{
    __GLvertexArray *pArray = &gc->clientState.vertexArray.currentArrays[arrayIdx];
    GLuint currentBuffer = gc->bufferObject.boundBuffer[__GL_ARRAY_BUFFER_INDEX];

    if (pArray->bufBinding != currentBuffer)
    {
        if (pArray->bufBinding == 0 || currentBuffer == 0)
        {
            /*
            ** if there is switch between buffer objects and conventional vertex array,
            ** set the format and offset bits dirty
            */
            __GL_SET_VARRAY_FORMAT_BIT(gc);
            __GL_SET_VARRAY_OFFSET_BIT(gc);
        }
        pArray->bufBinding = currentBuffer;
        if (currentBuffer)
        {
            gc->clientState.vertexArray.arrayInBufObj |= (1 << arrayIdx);
        }
        else
        {
            gc->clientState.vertexArray.arrayInBufObj &= ~(1 << arrayIdx);
        }
        gc->bufferObject.boundArrays[arrayIdx] = gc->bufferObject.boundTarget[__GL_ARRAY_BUFFER_INDEX];
        __GL_SET_VARRAY_BINDING_BIT(gc);
    }
    else if (gc->bufferObject.boundArrays[arrayIdx] != gc->bufferObject.boundTarget[__GL_ARRAY_BUFFER_INDEX])
    {
        /* Check the bound object because the app may delete the buffer object and bind again with the same name. */
        gc->bufferObject.boundArrays[arrayIdx] = gc->bufferObject.boundTarget[__GL_ARRAY_BUFFER_INDEX];
        __GL_SET_VARRAY_BINDING_BIT(gc);
    }

    if (pArray->size != size || pArray->type != type ||
        pArray->normalized != normalized || pArray->usr_stride != stride || pArray->integer != integer)
    {
        pArray->size = size;
        pArray->type = type;
        pArray->normalized = normalized;
        pArray->usr_stride = stride;
        pArray->stride = stride ? stride : size * __glSizeOfType( type);
        pArray->integer = integer;
        __GL_SET_VARRAY_FORMAT_BIT(gc);
    }

    /*no matter whether the "pointer" is a real pointer or an offset, we could just assign the value*/
    if (pArray->pointer != pointer)
    {
        pArray->pointer = pointer;
        __GL_SET_VARRAY_OFFSET_BIT(gc);
    }
}

GLvoid APIENTRY __glim_VertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexPointer", DT_GLint, size, DT_GLenum, type, DT_GLsizei, stride, DT_GLvoid_ptr, pointer, DT_GLnull);
#endif

    if (stride < 0 || size < 2 || size > 4)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    switch (type)
    {
    case GL_SHORT:
    case GL_INT:
    case GL_FLOAT:
    case GL_DOUBLE:
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    /* Note: Do not add __GL_VERTEX_BUFFER_FLUSH(gc) in this function */

    __glUpdateVertexArray(gc, __GL_VARRAY_VERTEX_INDEX,
        size, type, GL_FALSE, GL_FALSE, stride, pointer);
    if (gc->vertexArray.globalDirty & __GL_DIRTY_VARRAY_FORMAT_BIT)
    {
        gc->vertexArray.interleaved = GL_FALSE;
        __GL_VALIDATE_VERTEX_ARRAYS(gc);
    }
}

GLvoid APIENTRY __glim_NormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_NormalPointer", DT_GLenum, type, DT_GLsizei, stride, DT_GLvoid_ptr, pointer, DT_GLnull);
#endif

    if (stride < 0)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    switch (type)
    {
    case GL_BYTE:
    case GL_SHORT:
    case GL_INT:
    case GL_FLOAT:
    case GL_DOUBLE:
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    /* Note: Do not add __GL_VERTEX_BUFFER_FLUSH(gc) in this function */

    /*normal should set the normalized flag*/
    __glUpdateVertexArray(gc, __GL_VARRAY_NORMAL_INDEX,
        3, type, GL_TRUE, GL_FALSE, stride, pointer);
    if (gc->vertexArray.globalDirty & __GL_DIRTY_VARRAY_FORMAT_BIT)
    {
        gc->vertexArray.interleaved = GL_FALSE;
        __GL_VALIDATE_VERTEX_ARRAYS(gc);
    }
}

GLvoid APIENTRY __glim_ColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ColorPointer", DT_GLint, size, DT_GLenum, type, DT_GLsizei, stride, DT_GLvoid_ptr, pointer, DT_GLnull);
#endif

    if (stride < 0 || size < 3 || size > 4)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    switch (type)
    {
    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
    case GL_INT:
    case GL_UNSIGNED_INT:
    case GL_FLOAT:
    case GL_DOUBLE:
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    /* Note: Do not add __GL_VERTEX_BUFFER_FLUSH(gc) in this function */

    /*color should set the normalized flag*/
    __glUpdateVertexArray(gc, __GL_VARRAY_DIFFUSE_INDEX,
        size, type, GL_TRUE, GL_FALSE, stride, pointer);
    if (gc->vertexArray.globalDirty & __GL_DIRTY_VARRAY_FORMAT_BIT)
    {
        gc->vertexArray.interleaved = GL_FALSE;
        __GL_VALIDATE_VERTEX_ARRAYS(gc);
    }
}

GLvoid APIENTRY __glim_SecondaryColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_SecondaryColorPointer", DT_GLint, size, DT_GLenum, type, DT_GLsizei, stride, DT_GLvoid_ptr, pointer, DT_GLnull);
#endif

    if (stride < 0 || size != 3)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    switch (type)
    {
    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
    case GL_INT:
    case GL_UNSIGNED_INT:
    case GL_FLOAT:
    case GL_DOUBLE:
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    /* Note: Do not add __GL_VERTEX_BUFFER_FLUSH(gc) in this function */

    /*color should set the normalized flag*/
    __glUpdateVertexArray(gc, __GL_VARRAY_SPECULAR_INDEX,
        size, type, GL_TRUE, GL_FALSE, stride, pointer);
    if (gc->vertexArray.globalDirty & __GL_DIRTY_VARRAY_FORMAT_BIT)
    {
        gc->vertexArray.interleaved = GL_FALSE;
        __GL_VALIDATE_VERTEX_ARRAYS(gc);
    }
}

GLvoid APIENTRY __glim_FogCoordPointer(GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_FogCoordPointer", DT_GLenum, type, DT_GLsizei, stride, DT_GLvoid_ptr, pointer, DT_GLnull);
#endif

    if (stride < 0)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    switch (type)
    {
    case GL_FLOAT:
    case GL_DOUBLE:
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    /* Note: Do not add __GL_VERTEX_BUFFER_FLUSH(gc) in this function */

    __glUpdateVertexArray(gc, __GL_VARRAY_FOGCOORD_INDEX,
        1, type, GL_FALSE, GL_FALSE, stride, pointer);
    if (gc->vertexArray.globalDirty & __GL_DIRTY_VARRAY_FORMAT_BIT)
    {
        gc->vertexArray.interleaved = GL_FALSE;
        __GL_VALIDATE_VERTEX_ARRAYS(gc);
    }
}

GLvoid APIENTRY __glim_EdgeFlagPointer(GLint stride, const GLboolean *pointer)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_EdgeFlagPointer", DT_GLint, stride, DT_GLboolean_ptr, pointer, DT_GLnull);
#endif

    if (stride < 0)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    /* Note: Do not add __GL_VERTEX_BUFFER_FLUSH(gc) in this function */

    __glUpdateVertexArray(gc, __GL_VARRAY_EDGEFLAG_INDEX,
        1, GL_UNSIGNED_BYTE, GL_FALSE, GL_FALSE, stride, pointer);
    if (gc->vertexArray.globalDirty & __GL_DIRTY_VARRAY_FORMAT_BIT)
    {
        gc->vertexArray.interleaved = GL_FALSE;
        __GL_VALIDATE_VERTEX_ARRAYS(gc);
    }
}

GLvoid APIENTRY __glim_IndexPointer(GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_IndexPointer", DT_GLenum, type, DT_GLsizei, stride, DT_GLvoid_ptr, pointer, DT_GLnull);
#endif

    if (stride < 0)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    switch (type)
    {
    case GL_UNSIGNED_BYTE:
    case GL_SHORT:
    case GL_INT:
    case GL_FLOAT:
    case GL_DOUBLE:
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    /* Note: Do not add __GL_VERTEX_BUFFER_FLUSH(gc) in this function */

    __glUpdateVertexArray(gc, __GL_VARRAY_COLORINDEX_INDEX,
        1, type, GL_FALSE, GL_FALSE, stride, pointer);
    if (gc->vertexArray.globalDirty & __GL_DIRTY_VARRAY_FORMAT_BIT)
    {
        gc->vertexArray.interleaved = GL_FALSE;
        __GL_VALIDATE_VERTEX_ARRAYS(gc);
    }
}

GLvoid APIENTRY __glim_TexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_TexCoordPointer", DT_GLint, size, DT_GLenum, type, DT_GLsizei, stride, DT_GLvoid_ptr, pointer, DT_GLnull);
#endif

    if (stride < 0 || size < 1 || size > 4)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    switch (type)
    {
    case GL_SHORT:
    case GL_INT:
    case GL_FLOAT:
    case GL_DOUBLE:
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    /* Note: Do not add __GL_VERTEX_BUFFER_FLUSH(gc) in this function */

    __glUpdateVertexArray(gc,
        __GL_VARRAY_TEX0_INDEX + gc->clientState.vertexArray.clientActiveUnit,
        size, type, GL_FALSE, GL_FALSE, stride, pointer);
    if (gc->vertexArray.globalDirty & __GL_DIRTY_VARRAY_FORMAT_BIT)
    {
        gc->vertexArray.interleaved = GL_FALSE;
        __GL_VALIDATE_VERTEX_ARRAYS(gc);
    }
}

GLvoid APIENTRY __glim_VertexAttribPointer(GLuint index, GLint size, GLenum type,
                                           GLboolean normalized, GLsizei stride, const GLvoid *pointer)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttribPointer", DT_GLuint, index, DT_GLint, size, DT_GLenum, type,
        DT_GLboolean, normalized, DT_GLsizei, stride, DT_GLvoid_ptr, pointer, DT_GLnull);
#endif

    if(index >= __GL_MAX_PROGRAM_VERTEX_ATTRIBUTES)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if((stride < 0) || (size < 1) || (size > 4))
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    switch (type)
    {
    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
    case GL_INT:
    case GL_UNSIGNED_INT:
        break;
    case GL_FLOAT:
    case GL_DOUBLE:
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    /* Note: Do not add __GL_VERTEX_BUFFER_FLUSH(gc) in this function */

    __glUpdateVertexArray(gc, (__GL_VARRAY_ATT0_INDEX + index),
        size, type, normalized, GL_FALSE, stride, pointer);
    if (gc->vertexArray.globalDirty & __GL_DIRTY_VARRAY_FORMAT_BIT)
    {
        gc->vertexArray.interleaved = GL_FALSE;
        __GL_VALIDATE_VERTEX_ARRAYS(gc);
    }
}

#if GL_EXT_gpu_shader4
GLvoid APIENTRY __glim_VertexAttribIPointerEXT(GLuint index, GLint size, GLenum type,
                                GLsizei stride, const GLvoid *pointer)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttribIPointerEXT", DT_GLuint, index, DT_GLint, size, DT_GLenum, type,
        DT_GLsizei, stride, DT_GLvoid_ptr, pointer, DT_GLnull);
#endif

    if(index >= __GL_MAX_PROGRAM_VERTEX_ATTRIBUTES)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if((stride < 0) || (size <= 0) || (size > 4))
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    switch (type)
    {
    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
    case GL_INT:
    case GL_UNSIGNED_INT:
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    /* Note: Do not add __GL_VERTEX_BUFFER_FLUSH(gc) in this function */

    __glUpdateVertexArray(gc, (__GL_VARRAY_ATT0_INDEX + index),
        size, type, GL_FALSE, GL_TRUE, stride, pointer);
    if (gc->vertexArray.globalDirty & __GL_DIRTY_VARRAY_FORMAT_BIT)
    {
        gc->vertexArray.interleaved = GL_FALSE;
        __GL_VALIDATE_VERTEX_ARRAYS(gc);
    }
}
#endif

#define __GL_SIZE_F \
    sizeof(GLfloat)

#define __GL_SIZE_C \
    (((4*sizeof(GLubyte) + (__GL_SIZE_F-1)) / __GL_SIZE_F) * __GL_SIZE_F)

GLvoid APIENTRY __glim_InterleavedArrays(GLenum format, GLsizei stride, const GLvoid *pointer)
{
    GLboolean tflag, cflag, nflag;  /* enable/disable flags */
    GLint tcomps, ccomps, vcomps;   /* components per texcoord, color, vertex */
    GLenum ctype = 0;               /* color type */
    GLint coffset = 0, noffset = 0, voffset;/* color, normal, vertex offsets */
    const GLint toffset = 0;        /* always zero */
    GLint defstride;                /* default stride */
    GLint c, f;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_InterleavedArrays", DT_GLenum, format, DT_GLsizei, stride, DT_GLvoid_ptr, pointer, DT_GLnull);
#endif

    f = __GL_SIZE_F;
    c = __GL_SIZE_C;

    if (stride < 0)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    /* Note: Do not add __GL_VERTEX_BUFFER_FLUSH(gc) in this function */

    switch (format) {
      case GL_V2F:
          tflag = GL_FALSE;  cflag = GL_FALSE;  nflag = GL_FALSE;
          tcomps = 0;  ccomps = 0;  vcomps = 2;
          voffset = 0;
          defstride = 2*f;
          break;
      case GL_V3F:
          tflag = GL_FALSE;  cflag = GL_FALSE;  nflag = GL_FALSE;
          tcomps = 0;  ccomps = 0;  vcomps = 3;
          voffset = 0;
          defstride = 3*f;
          break;
      case GL_C4UB_V2F:
          tflag = GL_FALSE;  cflag = GL_TRUE;  nflag = GL_FALSE;
          tcomps = 0;  ccomps = 4;  vcomps = 2;
          ctype = GL_UNSIGNED_BYTE;
          coffset = 0;
          voffset = c;
          defstride = c + 2*f;
          break;
      case GL_C4UB_V3F:
          tflag = GL_FALSE;  cflag = GL_TRUE;  nflag = GL_FALSE;
          tcomps = 0;  ccomps = 4;  vcomps = 3;
          ctype = GL_UNSIGNED_BYTE;
          coffset = 0;
          voffset = c;
          defstride = c + 3*f;
          break;
      case GL_C3F_V3F:
          tflag = GL_FALSE;  cflag = GL_TRUE;  nflag = GL_FALSE;
          tcomps = 0;  ccomps = 3;  vcomps = 3;
          ctype = GL_FLOAT;
          coffset = 0;
          voffset = 3*f;
          defstride = 6*f;
          break;
      case GL_N3F_V3F:
          tflag = GL_FALSE;  cflag = GL_FALSE;  nflag = GL_TRUE;
          tcomps = 0;  ccomps = 0;  vcomps = 3;
          noffset = 0;
          voffset = 3*f;
          defstride = 6*f;
          break;
      case GL_C4F_N3F_V3F:
          tflag = GL_FALSE;  cflag = GL_TRUE;  nflag = GL_TRUE;
          tcomps = 0;  ccomps = 4;  vcomps = 3;
          ctype = GL_FLOAT;
          coffset = 0;
          noffset = 4*f;
          voffset = 7*f;
          defstride = 10*f;
          break;
      case GL_T2F_V3F:
          tflag = GL_TRUE;  cflag = GL_FALSE;  nflag = GL_FALSE;
          tcomps = 2;  ccomps = 0;  vcomps = 3;
          voffset = 2*f;
          defstride = 5*f;
          break;
      case GL_T4F_V4F:
          tflag = GL_TRUE;  cflag = GL_FALSE;  nflag = GL_FALSE;
          tcomps = 4;  ccomps = 0;  vcomps = 4;
          voffset = 4*f;
          defstride = 8*f;
          break;
      case GL_T2F_C4UB_V3F:
          tflag = GL_TRUE;  cflag = GL_TRUE;  nflag = GL_FALSE;
          tcomps = 2;  ccomps = 4;  vcomps = 3;
          ctype = GL_UNSIGNED_BYTE;
          coffset = 2*f;
          voffset = c+2*f;
          defstride = c+5*f;
          break;
      case GL_T2F_C3F_V3F:
          tflag = GL_TRUE;  cflag = GL_TRUE;  nflag = GL_FALSE;
          tcomps = 2;  ccomps = 3;  vcomps = 3;
          ctype = GL_FLOAT;
          coffset = 2*f;
          voffset = 5*f;
          defstride = 8*f;
          break;
      case GL_T2F_N3F_V3F:
          tflag = GL_TRUE;  cflag = GL_FALSE;  nflag = GL_TRUE;
          tcomps = 2;  ccomps = 0;  vcomps = 3;
          noffset = 2*f;
          voffset = 5*f;
          defstride = 8*f;
          break;
      case GL_T2F_C4F_N3F_V3F:
          tflag = GL_TRUE;  cflag = GL_TRUE;  nflag = GL_TRUE;
          tcomps = 2;  ccomps = 4;  vcomps = 3;
          ctype = GL_FLOAT;
          coffset = 2*f;
          noffset = 6*f;
          voffset = 9*f;
          defstride = 12*f;
          break;
      case GL_T4F_C4F_N3F_V4F:
          tflag = GL_TRUE;  cflag = GL_TRUE;  nflag = GL_TRUE;
          tcomps = 4;  ccomps = 4;  vcomps = 4;
          ctype = GL_FLOAT;
          coffset = 4*f;
          noffset = 8*f;
          voffset = 11*f;
          defstride = 15*f;
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    if (stride==0) {
        stride = defstride;
    }

    __glim_DisableClientState( GL_EDGE_FLAG_ARRAY );
    __glim_DisableClientState( GL_INDEX_ARRAY );
    __glim_DisableClientState( GL_FOG_COORDINATE_ARRAY);
    __glim_DisableClientState( GL_SECONDARY_COLOR_ARRAY );

    /* Texcoords */
    if (tflag) {
        __glim_EnableClientState( GL_TEXTURE_COORD_ARRAY );
        __glUpdateVertexArray(gc,
            __GL_VARRAY_TEX0_INDEX + gc->clientState.vertexArray.clientActiveUnit,
            tcomps, GL_FLOAT, GL_TRUE,
            GL_FALSE, stride, (GLubyte *) pointer + toffset);
    }
    else {
        __glim_DisableClientState( GL_TEXTURE_COORD_ARRAY );
    }

    /* Color */
    if (cflag) {
        __glim_EnableClientState( GL_COLOR_ARRAY );
        __glUpdateVertexArray(gc, __GL_VARRAY_DIFFUSE_INDEX,
            ccomps, ctype, GL_TRUE,
            GL_FALSE, stride, (GLubyte *) pointer + coffset);
    }
    else {
        __glim_DisableClientState( GL_COLOR_ARRAY );
    }


    /* Normals */
    if (nflag) {
        __glim_EnableClientState( GL_NORMAL_ARRAY );
        __glUpdateVertexArray(gc, __GL_VARRAY_NORMAL_INDEX,
            3, GL_FLOAT, GL_TRUE,
            GL_FALSE, stride, (GLubyte *) pointer + noffset);
    }
    else {
        __glim_DisableClientState( GL_NORMAL_ARRAY );
    }

    /* Vertices */
    __glim_EnableClientState( GL_VERTEX_ARRAY );
    __glUpdateVertexArray(gc, __GL_VARRAY_VERTEX_INDEX,
        vcomps, GL_FLOAT,
        GL_FALSE, GL_FALSE, stride,
        (GLubyte *) pointer + voffset);

    /*set the interleaved flag*/
    gc->vertexArray.interleaved = GL_TRUE;
}

GLvoid APIENTRY __glim_ArrayElement_Validate(GLint element)
{
    __GL_SETUP();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ArrayElement_Validate", DT_GLint, element, DT_GLnull);
#endif

    /* Compute the required primitive input mask */
    if (gc->input.inputMaskChanged)
    {
        __glComputeRequiredInputMask(gc);
        gc->input.inputMaskChanged = GL_FALSE;
    }
    gc->input.requiredInputMask = gc->input.currentInputMask & edgeFlagInputMask[gc->input.primMode];

    __glValidateVertexArrays(gc);

    gc->currentImmediateTable->dispatch.ArrayElement = gc->vertexArray.arrayElementFunc;

    (*gc->currentImmediateTable->dispatch.ArrayElement)(element);
}

GLvoid APIENTRY __glim_ArrayElement(GLint element)
{
    __GL_SETUP();
    GLfloat dataBuf[__GL_TOTAL_VERTEX_ATTRIBUTES * 4];
    GLuint tagBuf[__GL_TOTAL_VERTEX_ATTRIBUTES];
    GLboolean edgeflag;
    GLfloat *bufptr = (GLfloat *)&dataBuf[0];
    GLubyte *edgeptr = (GLubyte *)&edgeflag;
    GLint i, index, loop;
    GLenum error;

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ArrayElement", DT_GLint, element, DT_GLnull);
#endif

    __GL_MEMZERO(tagBuf, __GL_TOTAL_VERTEX_ATTRIBUTES * sizeof(GLuint));

    /* Copy the arrayElement data into dataBuf[] */

    error = __glArrayElement_Generic(gc, element, &bufptr, &edgeptr, tagBuf);
    if (error != GL_NO_ERROR) {
        __glSetError(error);
        return;
    }

    /* Dispatch the arrayElement data to the right API functions */

    i = 0;
    loop = GL_TRUE;
    bufptr = (GLfloat *)&dataBuf[0];
    while (loop) {
        switch (tagBuf[i])
        {
        case __GL_V2F_TAG:
            (*gc->currentImmediateTable->dispatch.Vertex2fv)(bufptr);
            bufptr += 2;
            loop = GL_FALSE;
            break;
        case __GL_V3F_TAG:
            (*gc->currentImmediateTable->dispatch.Vertex3fv)(bufptr);
            bufptr += 3;
            loop = GL_FALSE;
            break;
        case __GL_V4F_TAG:
            (*gc->currentImmediateTable->dispatch.Vertex4fv)(bufptr);
            bufptr += 4;
            loop = GL_FALSE;
            break;
        case __GL_C3F_TAG:
            (*gc->currentImmediateTable->dispatch.Color3fv)(bufptr);
            bufptr += 3;
            break;
        case __GL_C4F_TAG:
            (*gc->currentImmediateTable->dispatch.Color4fv)(bufptr);
            bufptr += 4;
            break;
        case __GL_C4UB_TAG:
            (*gc->currentImmediateTable->dispatch.Color4ubv)((GLubyte *)bufptr);
            bufptr += 1;
            break;
        case __GL_N3F_TAG:
            (*gc->currentImmediateTable->dispatch.Normal3fv)(bufptr);
            bufptr += 3;
            break;
        case __GL_TC2F_TAG:
            (*gc->currentImmediateTable->dispatch.TexCoord2fv)(bufptr);
            bufptr += 2;
            break;
        case __GL_TC2F_U1_TAG:
        case __GL_TC2F_U2_TAG:
        case __GL_TC2F_U3_TAG:
        case __GL_TC2F_U4_TAG:
        case __GL_TC2F_U5_TAG:
        case __GL_TC2F_U6_TAG:
        case __GL_TC2F_U7_TAG:
            index = GL_TEXTURE0 + (tagBuf[i] - __GL_TC2F_TAG);
            (*gc->currentImmediateTable->dispatch.MultiTexCoord2fv)(index, bufptr);
            bufptr += 2;
            break;
        case __GL_TC3F_TAG:
            (*gc->currentImmediateTable->dispatch.TexCoord3fv)(bufptr);
            bufptr += 3;
            break;
        case __GL_TC3F_U1_TAG:
        case __GL_TC3F_U2_TAG:
        case __GL_TC3F_U3_TAG:
        case __GL_TC3F_U4_TAG:
        case __GL_TC3F_U5_TAG:
        case __GL_TC3F_U6_TAG:
        case __GL_TC3F_U7_TAG:
            index = GL_TEXTURE0 + (tagBuf[i] - __GL_TC3F_TAG);
            (*gc->currentImmediateTable->dispatch.MultiTexCoord3fv)(index, bufptr);
            bufptr += 3;
            break;
        case __GL_TC4F_TAG:
            (*gc->currentImmediateTable->dispatch.TexCoord4fv)(bufptr);
            bufptr += 4;
            break;
        case __GL_TC4F_U1_TAG:
        case __GL_TC4F_U2_TAG:
        case __GL_TC4F_U3_TAG:
        case __GL_TC4F_U4_TAG:
        case __GL_TC4F_U5_TAG:
        case __GL_TC4F_U6_TAG:
        case __GL_TC4F_U7_TAG:
            index = GL_TEXTURE0 + (tagBuf[i] - __GL_TC4F_TAG);
            (*gc->currentImmediateTable->dispatch.MultiTexCoord4fv)(index, bufptr);
            bufptr += 4;
            break;
        case __GL_EDGEFLAG_TAG:
            (*gc->currentImmediateTable->dispatch.EdgeFlag)(edgeflag);
            break;
        case __GL_SC3F_TAG:
            (*gc->currentImmediateTable->dispatch.SecondaryColor3fv)(bufptr);
            bufptr += 3;
            break;
        case __GL_FOG1F_TAG:
            (*gc->currentImmediateTable->dispatch.FogCoordfv)(bufptr);
            bufptr += 1;
            break;
        case __GL_AT4F_I0_TAG:
        case __GL_AT4F_I1_TAG:
        case __GL_AT4F_I2_TAG:
        case __GL_AT4F_I3_TAG:
        case __GL_AT4F_I4_TAG:
        case __GL_AT4F_I5_TAG:
        case __GL_AT4F_I6_TAG:
        case __GL_AT4F_I7_TAG:
        case __GL_AT4F_I8_TAG:
        case __GL_AT4F_I9_TAG:
        case __GL_AT4F_I10_TAG:
        case __GL_AT4F_I11_TAG:
        case __GL_AT4F_I12_TAG:
        case __GL_AT4F_I13_TAG:
        case __GL_AT4F_I14_TAG:
        case __GL_AT4F_I15_TAG:
            index = (tagBuf[i] - __GL_AT4F_I0_TAG);
            (*gc->currentImmediateTable->dispatch.VertexAttrib4fv)(index, bufptr);
            bufptr += 4;
            break;

        default:
            loop = GL_FALSE;
            break;
        }
        i++;
    }
}

GLvoid APIENTRY __glim_DrawArrays_Validate(GLenum mode, GLint first, GLsizei count)
{
    __GL_SETUP();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_DrawArrays_Validate", DT_GLenum, mode, DT_GLint, first, DT_GLsizei, count, DT_GLnull);
#endif

    /* Compute the required primitive input mask */
    if (gc->input.inputMaskChanged)
    {
        __glComputeRequiredInputMask(gc);
        gc->input.inputMaskChanged = GL_FALSE;
    }
    gc->input.requiredInputMask = gc->input.currentInputMask & edgeFlagInputMask[gc->input.primMode];

    __glValidateVertexArrays(gc);

    gc->immediateDispatchTable.dispatch.DrawArrays = gc->vertexArray.drawArraysFunc;

    (*gc->immediateDispatchTable.dispatch.DrawArrays)(mode, first, count);
}

GLvoid APIENTRY __glim_DrawArrays(GLenum mode, GLint first, GLsizei count)
{
    __GLvertexArrayMachine *vertexArray;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_DrawArrays", DT_GLenum, mode, DT_GLint, first, DT_GLsizei, count, DT_GLnull);
#endif

    if (first < 0 || count < 0)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if (mode > GL_TRIANGLE_STRIP_ADJACENCY_EXT)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    CHECK_VERTEX_COUNT();

    vertexArray = &gc->vertexArray;
    vertexArray->indexCount = 0;
    vertexArray->start = first;
    vertexArray->end = first + count;

    __glValidateArrayStreamConfigPath(gc);

    __glDrawArrayPrimitive(gc, mode);

    if (gc->vertexArray.immedFallback)
    {
        /*Fall back to immediate mode*/
        GLint i;

        gc->vertexArray.immedFallback = GL_FALSE;

        __glim_Begin(mode);
        for( i = 0; i < count; i++ )
            __glim_ArrayElement(first + i );
        __glim_End();


    }
}

GLvoid APIENTRY __glim_DrawArraysInstancedEXT(
    GLenum mode, GLint first, GLsizei count, GLsizei primCount)
{
    __GLvertexArrayMachine *vertexArray;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_DrawArraysInstancedEXT", DT_GLenum, mode, DT_GLint, first, DT_GLsizei, count, DT_GLsizei, primCount, DT_GLnull);
#endif

    /* Compute the required primitive input mask */
    if (gc->input.inputMaskChanged)
    {
        __glComputeRequiredInputMaskInstancedEXT(gc);
        gc->input.inputMaskChanged = GL_FALSE;
    }

    gc->input.requiredInputMask = gc->input.currentInputMask & edgeFlagInputMask[gc->input.primMode];
    /* validate vertex array */
    __glValidateVertexArraysInstancedEXT(gc);

    if (first < 0 || count < 0 || primCount < 0)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if (mode > GL_TRIANGLE_STRIP_ADJACENCY_EXT)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    CHECK_VERTEX_COUNT();
    CHECK_INSTANCE_COUNT();

    vertexArray = &gc->vertexArray;
    vertexArray->indexCount = 0;
    vertexArray->start = first;
    vertexArray->end = first + count;

    gc->input.primCount = primCount;
    __glValidateArrayStreamConfigPath(gc);
    __glDrawArrayPrimitive(gc, mode);

    if (gc->vertexArray.immedFallback)
    {
        /*Fall back to immediate mode*/
        GLint i;
        gc->vertexArray.immedFallback = GL_FALSE;
        __glim_Begin(mode);
        for( i = 0; i < count; i++ )
            __glim_ArrayElement(first + i );
        __glim_End();
    }
}

__GL_INLINE GLvoid __glDrawRangeElements(__GLcontext *gc,
                                         GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    __GLvertexArrayMachine *vertexArray = &gc->vertexArray;
    GLint i;

    vertexArray->indexCount = count;
    /* This indices might be treated as an offset. */
    vertexArray->indices = indices;
    vertexArray->indexType = type;

    __glDrawArrayPrimitive(gc, mode);

    if (gc->vertexArray.immedFallback)
    {
        gc->vertexArray.immedFallback = GL_FALSE;

        /*Fall back to immediate mode*/
        __glim_Begin(mode);
        switch(type)
        {
        case GL_UNSIGNED_BYTE:
            for( i = 0; i < count; i++ )
                __glim_ArrayElement( ((GLubyte *)indices)[i] );
            break;

        case GL_UNSIGNED_SHORT:
            for( i = 0; i < count; i++ )
                __glim_ArrayElement( ((GLushort *)indices)[i] );
            break;

        case GL_UNSIGNED_INT:
            for( i = 0; i < count; i++ )
                __glim_ArrayElement( ((GLuint *)indices)[i] );
            break;
        }
        __glim_End();
    }
}

GLvoid APIENTRY __glim_DrawElements_Validate(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    __GL_SETUP();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_DrawElements_Validate", DT_GLenum, mode, DT_GLsizei, count, DT_GLenum, type, DT_GLvoid_ptr, indices, DT_GLnull);
#endif

    /* Compute the required primitive input mask */
    if (gc->input.inputMaskChanged)
    {
        __glComputeRequiredInputMask(gc);
        gc->input.inputMaskChanged = GL_FALSE;
    }
    gc->input.requiredInputMask = gc->input.currentInputMask & edgeFlagInputMask[mode];

    __glValidateVertexArrays(gc);

    gc->immediateDispatchTable.dispatch.DrawElements = gc->vertexArray.drawElementsFunc;

    (*gc->immediateDispatchTable.dispatch.DrawElements)(mode, count, type, indices);
}

GLvoid APIENTRY __glim_DrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_DrawElements", DT_GLenum, mode, DT_GLsizei, count, DT_GLenum, type, DT_GLvoid_ptr, indices, DT_GLnull);
#endif

    switch (type)
    {
    case GL_UNSIGNED_BYTE:
    case GL_UNSIGNED_SHORT:
    case GL_UNSIGNED_INT:
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    if (count < 0)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    if (mode > GL_TRIANGLE_STRIP_ADJACENCY_EXT)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    CHECK_VERTEX_COUNT();

    /*
    ** DP know exactly whether data need to be copied, so move those check for index scanning
    ** to DP and only do them when they are really needed.
    */

    __glValidateArrayStreamConfigPath(gc);

    if(gc->vertexArray.lockData.lockValid)
    {
        gc->vertexArray.start = gc->vertexArray.lockData.first;
        gc->vertexArray.end = gc->vertexArray.lockData.first + gc->vertexArray.lockData.count;
    }
    else
    {
        gc->vertexArray.start = 0;
        gc->vertexArray.end = 0;
    }

    __glDrawRangeElements(gc, mode, count, type, indices);
}

GLvoid APIENTRY __glim_DrawElementsInstancedEXT(
    GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei primCount)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_DrawElementsInstancedEXT", DT_GLenum, mode, DT_GLsizei, count, DT_GLenum, type, DT_GLsizei, primCount, DT_GLvoid_ptr, indices, DT_GLnull);
#endif

    /* Compute the required primitive input mask */
    if (gc->input.inputMaskChanged)
    {
        __glComputeRequiredInputMaskInstancedEXT(gc);
        gc->input.inputMaskChanged = GL_FALSE;
    }
    gc->input.requiredInputMask = gc->input.currentInputMask & edgeFlagInputMask[mode];
    __glValidateVertexArraysInstancedEXT(gc);

    switch (type)
    {
    case GL_UNSIGNED_BYTE:
    case GL_UNSIGNED_SHORT:
    case GL_UNSIGNED_INT:
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    if (count < 0 || primCount < 0)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    if (mode > GL_TRIANGLE_STRIP_ADJACENCY_EXT)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    CHECK_VERTEX_COUNT();
    CHECK_INSTANCE_COUNT();
    gc->input.primCount = primCount;

    /*
    ** DP know exactly whether data need to be copied, so move those check for index scanning
    ** to DP and only do them when they are really needed.
    */
    __glValidateArrayStreamConfigPath(gc);

    if(gc->vertexArray.lockData.lockValid)
    {
        gc->vertexArray.start = gc->vertexArray.lockData.first;
        gc->vertexArray.end = gc->vertexArray.lockData.first + gc->vertexArray.lockData.count;
    }
    else
    {
        gc->vertexArray.start = 0;
        gc->vertexArray.end = 0;
    }

    __glDrawRangeElements(gc, mode, count, type, indices);
}

GLvoid APIENTRY __glim_DrawRangeElements(GLenum mode,
    GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_DrawRangeElements", DT_GLenum, mode,
        DT_GLuint, start, DT_GLuint, end, DT_GLsizei, count, DT_GLenum, type, DT_GLvoid_ptr, indices, DT_GLnull);
#endif

    switch (type)
    {
    case GL_UNSIGNED_BYTE:
    case GL_UNSIGNED_SHORT:
    case GL_UNSIGNED_INT:
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    if (count < 0 || start > end)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    if (mode > GL_TRIANGLE_STRIP_ADJACENCY_EXT)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    CHECK_VERTEX_COUNT();

    if (gc->immediateDispatchTable.dispatch.DrawElements == __glim_DrawElements_Validate)
    {
        /* Compute the required primitive input mask */
        if (gc->input.inputMaskChanged)
        {
            __glComputeRequiredInputMask(gc);
            gc->input.inputMaskChanged = GL_FALSE;
        }
        gc->input.requiredInputMask = gc->input.currentInputMask & edgeFlagInputMask[mode];

        __glValidateVertexArrays(gc);

        gc->immediateDispatchTable.dispatch.DrawElements = gc->vertexArray.drawElementsFunc;
    }

    /*
    ** If drawElements could go to the immediate mode path, drawRangeElements also could.
    */
    if (gc->vertexArray.drawElementsFunc != __glim_DrawElements)
    {
        (*gc->vertexArray.drawElementsFunc)(mode, count, type, indices);
        return;
    }

    __glValidateArrayStreamConfigPath(gc);

    gc->vertexArray.start = start;
    gc->vertexArray.end = end + 1;

    __glDrawRangeElements(gc, mode, count, type, indices);
}

GLvoid APIENTRY __glim_MultiDrawArrays(GLenum mode, GLint *first, GLsizei *count, GLsizei primcount)
{
    GLint i;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_MultiDrawArrays", DT_GLenum, mode, DT_GLint_ptr, first, DT_GLsizei_ptr, count, DT_GLsizei, primcount, DT_GLnull);
#endif

    for (i = 0; i < primcount; i++)
    {
        if (count[i] >0) {
            (*gc->immediateDispatchTable.dispatch.DrawArrays)(mode, first[i], count[i]);
        }
    }
}

GLvoid APIENTRY __glim_MultiDrawElements(GLenum mode,
                                         const GLsizei *count, GLenum type, const GLvoid **indices, GLsizei primcount)
{
    GLint i;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_MultiDrawElements", DT_GLenum, mode,
        DT_GLsizei_ptr, count, DT_GLenum, type, DT_GLvoid_ptr, indices, DT_GLsizei, primcount, DT_GLnull);
#endif

    for (i = 0; i < primcount; i++)
    {
        if (count[i] >0) {
            (*gc->immediateDispatchTable.dispatch.DrawElements)(mode, count[i], type, indices[i]);
        }
    }
}

GLvoid __glInitVertexArrayState(__GLcontext *gc)
{
    GLuint i;

    for (i = 0; i < __GL_TOTAL_VERTEX_ATTRIBUTES; i++)
    {
        gc->clientState.vertexArray.currentArrays[i].bufBinding = 0;
        gc->clientState.vertexArray.currentArrays[i].normalized = GL_FALSE;
        gc->clientState.vertexArray.currentArrays[i].size = 0;
        gc->clientState.vertexArray.currentArrays[i].offset = 0;
        gc->clientState.vertexArray.currentArrays[i].pointer = 0;
        gc->clientState.vertexArray.currentArrays[i].stride = 0;
        gc->clientState.vertexArray.currentArrays[i].type = GL_FLOAT;
        gc->clientState.vertexArray.currentArrays[i].usr_stride = 0;
        gc->clientState.vertexArray.currentArrays[i].integer = GL_FALSE;
    }
    gc->clientState.vertexArray.arrayEnabled = 0;
    gc->clientState.vertexArray.currentEnabled = 0;
    gc->clientState.vertexArray.arrayInBufObj = 0;
    gc->clientState.vertexArray.clientActiveUnit = 0;
    gc->clientState.vertexArray.arrayBufBinding = 0;
    gc->clientState.vertexArray.elementBufBinding = 0;

#if GL_ATI_element_array
    gc->clientState.vertexArray.elementArrayATI = GL_FALSE;
    gc->clientState.vertexArray.elementPointer = NULL;
    gc->clientState.vertexArray.elementType = GL_UNSIGNED_INT;
    gc->clientState.vertexArray.elementArrayBindingATI = 0;
#endif

    /*init vertex array dirty bits*/
    gc->vertexArray.globalDirty = (GLbitfield)(-1);
    gc->vertexArray.immedFallback = GL_FALSE;
    gc->vertexArray.interleaved = GL_FALSE;
    gc->vertexArray.formatChanged = GL_TRUE;
    gc->vertexArray.fastStreamSetup = GL_FALSE;
    gc->vertexArray.lockData.lockValid = GL_FALSE;
}
