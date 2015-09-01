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

#if GL_ATI_vertex_array_object
extern GLvoid __glBindBuffer(__GLcontext *gc, GLuint targetIndex, GLuint buffer);

GLint __glGetObjectBuffer(__GLcontext *gc,GLuint buffer, GLenum pname)
{
    __GLbufferObject * bufObj;
    GLenum mapARBUsageToATI[2] = {GL_STATIC_ATI, GL_DYNAMIC_ATI};

    bufObj = (__GLbufferObject *)__glGetObject(gc, gc->bufferObject.shared, buffer);

    GL_ASSERT(bufObj);

    switch (pname)
    {
    case GL_OBJECT_BUFFER_SIZE_ATI:
        return (GLint)bufObj->size;

    case GL_OBJECT_BUFFER_USAGE_ATI:
        return mapARBUsageToATI[bufObj->usage - GL_STATIC_DRAW];

    default:
        return -1;
    }
}

GLint __glGetArrayObject(__GLcontext *gc,GLenum array, GLenum pname)
{
    GLuint buffer=0;
    GLint offset=0;
    GLuint arrayIdx=0;
    __GLbufferObject * bufObj;

    switch (array)
    {
    case GL_VERTEX_ARRAY:
        arrayIdx = __GL_VARRAY_VERTEX_INDEX;
        break;
    case GL_NORMAL_ARRAY:
        arrayIdx = __GL_VARRAY_NORMAL_INDEX;
        break;
    case GL_COLOR_ARRAY:
        arrayIdx = __GL_VARRAY_DIFFUSE_INDEX;
        break;
    case GL_TEXTURE_COORD_ARRAY:
        arrayIdx = __GL_VARRAY_TEX0_INDEX + gc->clientState.vertexArray.clientActiveUnit;
        break;
    case GL_SECONDARY_COLOR_ARRAY_EXT:
        arrayIdx = __GL_VARRAY_SPECULAR_INDEX;
        break;
    case GL_FOG_COORDINATE_ARRAY_EXT:
        arrayIdx = __GL_VARRAY_FOGCOORD_INDEX;
        break;

    case GL_ELEMENT_ARRAY_ATI:
        break;

    case GL_WEIGHT_ARRAY_ARB:
        break;

    case GL_INDEX_ARRAY:
        break;

    case GL_EDGE_FLAG_ARRAY:
        arrayIdx = __GL_VARRAY_EDGEFLAG_INDEX;
        break;

    default:
        //result is zero if no array object in buffer;
        return 0;
    }

    if(array == GL_ELEMENT_ARRAY_ATI)
    {
        bufObj = gc->bufferObject.boundTarget[__GL_ELEMENT_ARRAY_BUFFER_INDEX];

        if(bufObj)
            return 0;

        buffer = gc->bufferObject.boundBuffer[__GL_ELEMENT_ARRAY_BUFFER_INDEX];

        switch(pname)
        {
        case GL_ARRAY_OBJECT_BUFFER_ATI:
            return buffer;

        case GL_ARRAY_OBJECT_OFFSET_ATI:
            offset = (GLint)glALL_TO_UINT32(bufObj->mapPointer);
            return offset;

        default:
            return -1;
        }
    }
    else
    {
        bufObj = gc->bufferObject.boundArrays[arrayIdx];

        if(bufObj)
            return 0;

        buffer = bufObj->name;

        switch(pname)
        {
        case GL_ARRAY_OBJECT_BUFFER_ATI:
            return buffer;

        case GL_ARRAY_OBJECT_OFFSET_ATI:
            offset = (GLint)glALL_TO_UINT32(gc->clientState.vertexArray.currentArrays[arrayIdx].pointer);
            return offset;
        default:
            return -1;
        }
    }
}

/* OpenGL ATI_vertex_array_object APIs*/

GLuint APIENTRY __glim_NewObjectBufferATI (GLsizei size, const GLvoid *pointer, GLenum usage)
{
    GLuint name;
    GLenum mapATIUsageToARB[2] = {GL_STATIC_DRAW, GL_DYNAMIC_DRAW};
    GLuint oldArrayBufferBinding;
    __GL_SETUP();

    oldArrayBufferBinding =  gc->bufferObject.boundBuffer[__GL_ARRAY_BUFFER_INDEX];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_NewObjectBufferATI", DT_GLsizei, size, DT_GLvoid_ptr, pointer, DT_GLenum, usage, DT_GLnull);
#endif

    if (size <=0 ||(usage != GL_STATIC_ATI && usage != GL_DYNAMIC_ATI))
    {
        __glSetError(GL_INVALID_VALUE);
        return 0;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    name = __glGenerateNames(gc, gc->bufferObject.shared, 1);

    GL_ASSERT(name);

    /* Call ARB binding function to allocate buffer object and add to shared list */
    __glBindBuffer(gc, __GL_ARRAY_BUFFER_INDEX, name);

    /* Call ARB data transfer function to allocate local memory for this object */
    __glim_BufferData(GL_ARRAY_BUFFER, size, pointer, mapATIUsageToARB[usage - GL_STATIC_ATI]);

    /* Reset to default buffer object */
    __glBindBuffer(gc, __GL_ARRAY_BUFFER_INDEX, oldArrayBufferBinding);

    return name;
}


GLboolean APIENTRY __glim_IsObjectBufferATI (GLuint buffer)
{

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_IsObjectBufferATI", DT_GLuint, buffer, DT_GLnull);
#endif

    return __glim_IsBuffer(buffer);
}

GLvoid APIENTRY __glim_UpdateObjectBufferATI (GLuint buffer, GLuint offset,
                                              GLsizei size, const GLvoid *pointer,
                                              GLenum preserve)
{
    GLuint oldArrayBufferBinding;
    __GL_SETUP_NOT_IN_BEGIN();

    oldArrayBufferBinding =  gc->bufferObject.boundBuffer[__GL_ARRAY_BUFFER_INDEX];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_UpdateObjectBufferATI", DT_GLuint, buffer, DT_GLuint, offset, DT_GLsizei, size, DT_GLvoid_ptr, pointer, DT_GLenum, preserve, DT_GLnull);
#endif

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Call ARB binding function to allocate buffer object and add to shared list */
    __glBindBuffer(gc, __GL_ARRAY_BUFFER_INDEX, buffer);

    __glim_BufferSubData(GL_ARRAY_BUFFER, offset, size, pointer);

    /* Reset to default buffer object */
    __glBindBuffer(gc, __GL_ARRAY_BUFFER_INDEX, oldArrayBufferBinding);
}


GLvoid APIENTRY __glim_GetObjectBufferfvATI (GLuint buffer, GLenum pname, GLfloat *params)
{
    GLint result;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetObjectBufferfvATI", DT_GLuint, buffer, DT_GLenum, pname, DT_GLfloat_ptr, params, DT_GLnull);
#endif

    result = __glGetObjectBuffer(gc,buffer,pname);

    if(result == -1)
    {
        __glSetError(GL_INVALID_VALUE);
        return ;
    }

    *params = (GLfloat)result;

}
GLvoid APIENTRY __glim_GetObjectBufferivATI (GLuint buffer, GLenum pname, GLint *params)
{
    GLint result;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetObjectBufferivATI", DT_GLuint, buffer, DT_GLenum, pname, DT_GLint_ptr, params, DT_GLnull);
#endif

    result = __glGetObjectBuffer(gc,buffer,pname);
    if(result == -1)
    {
        __glSetError(GL_INVALID_VALUE);
        return ;
    }

    *params = result;
}

GLvoid APIENTRY __glim_FreeObjectBufferATI (GLuint buffer)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_FreeObjectBufferATI", DT_GLuint, buffer, DT_GLnull);
#endif

    __glim_DeleteBuffers(1, &buffer);
}


GLvoid APIENTRY __glim_ArrayObjectATI (GLenum array,
                                       GLint size,
                                       GLenum type,
                                       GLsizei stride,
                                       GLuint buffer,
                                       GLuint offset)
{

    GLuint oldArrayBufferBinding;
#if GL_ATI_element_array
    GLuint oldElementArrayBinding;
#endif

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ArrayObjectATI", DT_GLenum, array, DT_GLint, size, DT_GLenum, type, DT_GLsizei, stride, DT_GLuint, buffer, DT_GLuint, offset, DT_GLnull);
#endif

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Make the buffer the current buffer object */
#if GL_ATI_element_array
    if( array == GL_ELEMENT_ARRAY_ATI )
    {
        oldElementArrayBinding =  gc->bufferObject.boundBuffer[__GL_ELEMENT_ARRAY_BUFFER_INDEX];
        __glBindBuffer(gc, __GL_ELEMENT_ARRAY_BUFFER_INDEX, buffer);
    }
    else
#endif
    {
        oldArrayBufferBinding =  gc->bufferObject.boundBuffer[__GL_ARRAY_BUFFER_INDEX];
        __glBindBuffer(gc, __GL_ARRAY_BUFFER_INDEX, buffer);
    }

    switch (array)
    {
    case GL_VERTEX_ARRAY:

        __glim_VertexPointer(size, type, stride, (GLvoid *)(UINT_PTR)offset);
        break;

    case GL_NORMAL_ARRAY:
        __glim_NormalPointer(type, stride, (GLvoid *)(UINT_PTR)offset);
        break;

    case GL_COLOR_ARRAY:
        __glim_ColorPointer(size, type, stride, (GLvoid *)(UINT_PTR)offset);
        break;

    case GL_TEXTURE_COORD_ARRAY:
        __glim_TexCoordPointer(size, type, stride, (GLvoid *)(UINT_PTR)offset);
        break;

    case GL_SECONDARY_COLOR_ARRAY_EXT:
        __glim_SecondaryColorPointer(size, type, stride, (GLvoid *)(UINT_PTR)offset);
        break;

    case GL_FOG_COORDINATE_ARRAY_EXT:
        __glim_FogCoordPointer(type, stride, (GLvoid *)(UINT_PTR)offset);
        break;

    case GL_WEIGHT_ARRAY_ARB:
        GL_ASSERT(0);
        /* Add later */
        break;

#if GL_ATI_element_array
    case GL_ELEMENT_ARRAY_ATI:
        __glim_ElementPointerATI(type, (const GLvoid *)(UINT_PTR)offset);
        break;
#endif

    case GL_EDGE_FLAG_ARRAY:
        __glim_EdgeFlagPointer(stride, (GLvoid *)(UINT_PTR)offset);
        break;

    case GL_INDEX_ARRAY:
        break;

    default:
        __glSetError(GL_INVALID_ENUM);
        break;
    }

    /* Restore the buffer binding */
#if GL_ATI_element_array
    if( array == GL_ELEMENT_ARRAY_ATI )
    {
        __glBindBuffer(gc, __GL_ELEMENT_ARRAY_BUFFER_INDEX, oldElementArrayBinding);
    }
    else
#endif
    {
        __glBindBuffer(gc, __GL_ARRAY_BUFFER_INDEX, oldArrayBufferBinding);
    }
}

GLvoid APIENTRY __glim_GetArrayObjectfvATI (GLenum array, GLenum pname, GLfloat * params)
{
    GLint result;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetArrayObjectfvATI", DT_GLenum, array, DT_GLenum, pname, DT_GLfloat_ptr, params, DT_GLnull);
#endif

    result = __glGetArrayObject(gc,array,pname);
    if (result == -1)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    *params = (GLfloat)result;
}

GLvoid APIENTRY __glim_GetArrayObjectivATI (GLenum array, GLenum pname, GLint * params)
{
    GLint result;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetArrayObjectivATI", DT_GLenum, array, DT_GLenum, pname, DT_GLint_ptr, params, DT_GLnull);
#endif

    result = __glGetArrayObject(gc,array,pname);
    if (result == -1)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    *params = result;
}

#if GL_ATI_vertex_array_object
GLvoid APIENTRY __glim_VariantArrayObjectATI (GLuint id, GLenum type, GLsizei stride, GLuint buffer, GLuint offset)
{
    GLuint oldArrayBufferBinding;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VariantArrayObjectATI", DT_GLuint, id, DT_GLenum, type, DT_GLsizei, stride, DT_GLuint, buffer, DT_GLuint, offset, DT_GLnull);
#endif

    if (type < GL_BYTE || type > GL_DOUBLE)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    if (stride < 0)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Make the buffer the current buffer object */
    oldArrayBufferBinding =  gc->bufferObject.boundBuffer[__GL_ARRAY_BUFFER_INDEX];
    __glBindBuffer(gc, __GL_ARRAY_BUFFER_INDEX, buffer);

    /* __glim_VariantPointerEXT (id, type, stride, (GLvoid *)(UINT_PTR)offset); */

    __glBindBuffer(gc, __GL_ARRAY_BUFFER_INDEX, oldArrayBufferBinding);
}

GLvoid APIENTRY __glim_GetVariantArrayObjectfvATI (GLuint id, GLenum pname, GLfloat *params)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetVariantArrayObjectfvATI", DT_GLuint, id, DT_GLenum, pname, DT_GLfloat_ptr, params, DT_GLnull);
#endif

}

GLvoid APIENTRY __glim_GetVariantArrayObjectivATI (GLuint id, GLenum pname, GLint *params)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetVariantArrayObjectivATI", DT_GLuint, id, DT_GLenum, pname, DT_GLint_ptr, params, DT_GLnull);
#endif

}
#endif

#if GL_ATI_vertex_attrib_array_object
GLvoid APIENTRY __glim_VertexAttribArrayObjectATI (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLuint buffer, GLuint offset)
{
    GLuint oldArrayBufferBinding;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttribArrayObjectATI", DT_GLuint, index, DT_GLint, size, DT_GLenum, type, DT_GLboolean, normalized, DT_GLsizei, stride, DT_GLuint, buffer, DT_GLuint, offset, DT_GLnull);
#endif

    if (type < GL_BYTE || type > GL_DOUBLE)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    if (stride < 0)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

     /* Make the buffer the current buffer object */
    oldArrayBufferBinding =  gc->bufferObject.boundBuffer[__GL_ARRAY_BUFFER_INDEX];
    __glBindBuffer(gc, __GL_ARRAY_BUFFER_INDEX, buffer);

    __glim_VertexAttribPointer (index, size, type, normalized, stride, (GLvoid *)(UINT_PTR)offset);

    __glBindBuffer(gc, __GL_ARRAY_BUFFER_INDEX, oldArrayBufferBinding);
}

GLvoid APIENTRY __glim_GetVertexAttribArrayObjectfvATI (GLuint index, GLenum pname, GLfloat *params)
{
    __GLbufferObject * bufObj;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetVertexAttribArrayObjectfvATI", DT_GLuint, index, DT_GLenum, pname, DT_GLfloat_ptr, params, DT_GLnull);
#endif

    if (index >= __GL_MAX_PROGRAM_VERTEX_ATTRIBUTES)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    bufObj = gc->bufferObject.boundArrays[index + __GL_VARRAY_ATT0_INDEX];

    switch(pname)
    {
    case GL_ARRAY_OBJECT_BUFFER_ATI:
        *params = (GLfloat)bufObj->name;
        return;

    case GL_ARRAY_OBJECT_OFFSET_ATI:
        *params = (GLfloat)glALL_TO_UINT32(gc->clientState.vertexArray.currentArrays[index + __GL_VARRAY_ATT0_INDEX].pointer);
        return;

    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}

GLvoid APIENTRY __glim_GetVertexAttribArrayObjectivATI (GLuint index, GLenum pname, GLint *params)
{
    __GLbufferObject * bufObj;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetVertexAttribArrayObjectivATI", DT_GLuint, index, DT_GLenum, pname, DT_GLint_ptr, params, DT_GLnull);
#endif

    if (index >= __GL_MAX_PROGRAM_VERTEX_ATTRIBUTES)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    bufObj = gc->bufferObject.boundArrays[index + __GL_VARRAY_ATT0_INDEX];

    switch(pname)
    {
    case GL_ARRAY_OBJECT_BUFFER_ATI:
        *params = bufObj->name;
        return;

    case GL_ARRAY_OBJECT_OFFSET_ATI:
        *params = (GLint)glALL_TO_UINT32(gc->clientState.vertexArray.currentArrays[index + __GL_VARRAY_ATT0_INDEX].pointer);
        return;

    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}
#endif
#endif

#if GL_ATI_element_array
/* ATI_element_array APIs */
GLvoid  APIENTRY __glim_ElementPointerATI(GLenum type, const GLvoid *pointer)
{
    __GLvertexArrayState *pVertexArrayState;
    __GL_SETUP_NOT_IN_BEGIN();

    pVertexArrayState = &gc->clientState.vertexArray;

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ElementPointerATI" ,DT_GLenum, type, DT_GLvoid_ptr, pointer, DT_GLnull);
#endif

    switch (type)
    {
    case GL_UNSIGNED_SHORT:
    case GL_UNSIGNED_INT:
    case GL_UNSIGNED_BYTE:
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    pVertexArrayState->elementPointer = pointer;
    pVertexArrayState->elementType = type;
    pVertexArrayState->elementArrayBindingATI = gc->bufferObject.boundBuffer[__GL_ELEMENT_ARRAY_BUFFER_INDEX];
}

GLvoid  APIENTRY __glim_DrawElementArrayATI(GLenum mode, GLsizei count)
{
     __GLvertexArrayState *pVertexArrayState;
     GLuint oldElementArrayBinding;
    __GL_SETUP_NOT_IN_BEGIN();

    pVertexArrayState = &gc->clientState.vertexArray;

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_DrawElementArrayATI", DT_GLenum, mode, DT_GLsizei, count, DT_GLnull);
#endif

    if(pVertexArrayState->elementArrayATI == GL_FALSE)
    {
        __glSetError(GL_INVALID_OPERATION );
        return;
    }

    oldElementArrayBinding = gc->bufferObject.boundBuffer[__GL_ELEMENT_ARRAY_BUFFER_INDEX];

    /* Bind the current element array to the element array binding */
    __glBindBuffer(gc, __GL_ELEMENT_ARRAY_BUFFER_INDEX, pVertexArrayState->elementArrayBindingATI);

    gc->immediateDispatchTable.dispatch.DrawElements(mode,
            count,
            pVertexArrayState->elementType,
            pVertexArrayState->elementPointer);

    /* Restore the current element array binding */
    __glBindBuffer(gc, __GL_ELEMENT_ARRAY_BUFFER_INDEX, oldElementArrayBinding);
}

GLvoid  APIENTRY __glim_DrawRangeElementArrayATI(GLenum mode, GLuint start,
                                                 GLuint end, GLsizei count)
{
     __GLvertexArrayState *pVertexArrayState;
     GLuint oldElementArrayBinding;
    __GL_SETUP_NOT_IN_BEGIN();

    pVertexArrayState = &gc->clientState.vertexArray;

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_DrawRangeElementArrayATI", DT_GLenum, mode, DT_GLuint, start, DT_GLuint, end, DT_GLsizei, count, DT_GLnull);
#endif

    if(pVertexArrayState->elementArrayATI == GL_FALSE)
    {
        __glSetError(GL_INVALID_OPERATION );
        return;
    }

    oldElementArrayBinding = gc->bufferObject.boundBuffer[__GL_ELEMENT_ARRAY_BUFFER_INDEX];

    /* Bind the current element array to the element array binding */
    __glBindBuffer(gc, __GL_ELEMENT_ARRAY_BUFFER_INDEX, pVertexArrayState->elementArrayBindingATI);

    __glim_DrawRangeElements(mode, start, end, count,
        pVertexArrayState->elementType,
        pVertexArrayState->elementPointer);

    /* Restore the current element array binding */
    __glBindBuffer(gc, __GL_ELEMENT_ARRAY_BUFFER_INDEX, oldElementArrayBinding);

}
#endif

