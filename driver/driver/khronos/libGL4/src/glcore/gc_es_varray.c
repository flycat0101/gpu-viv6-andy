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

#define _GC_OBJ_ZONE gcdZONE_GL40_CORE


extern GLvoid __glDrawPrimitive(__GLcontext *gc, GLenum mode);
extern GLvoid __glInitBufferObject(__GLcontext *gc, __GLbufferObject *bufObj, GLuint name);
#ifdef OPENGL40
extern GLvoid __glConfigArrayVertexStream(__GLcontext *gc, GLenum mode);
#endif


__GL_INLINE GLvoid __glVertexAttrib4f(__GLcontext *gc, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    if (index >= gc->constants.shaderCaps.maxUserVertAttributes)
    {
        __GL_ERROR_RET(GL_INVALID_VALUE);
    }


    gc->state.current.attribute[index].f.x = x;
    gc->state.current.attribute[index].f.y = y;
    gc->state.current.attribute[index].f.z = z;
    gc->state.current.attribute[index].f.w = w;

}

__GL_INLINE GLvoid __glVertexAttribI4ui(__GLcontext *gc, GLuint index, GLuint ix, GLuint iy, GLuint iz, GLuint iw)
{
    if (index >= gc->constants.shaderCaps.maxUserVertAttributes)
    {
        __GL_ERROR_RET(GL_INVALID_VALUE);
    }

    gc->state.current.attribute[index].i.ix = ix;
    gc->state.current.attribute[index].i.iy = iy;
    gc->state.current.attribute[index].i.iz = iz;
    gc->state.current.attribute[index].i.iw = iw;

}

GLvoid GL_APIENTRY __glim_VertexAttrib1f(__GLcontext *gc, GLuint index, GLfloat x)
{
    __GL_HEADER();

    __glVertexAttrib4f(gc, index, x, 0.0f, 0.0f, 1.0f);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_VertexAttrib1fv(__GLcontext *gc, GLuint index, const GLfloat *v)
{
    __GL_HEADER();

    __glVertexAttrib4f(gc, index, v[0], 0.0f, 0.0f, 1.0f);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_VertexAttrib2f(__GLcontext *gc, GLuint index, GLfloat x, GLfloat y)
{
    __GL_HEADER();

    __glVertexAttrib4f(gc, index, x, y, 0.0f, 1.0f);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_VertexAttrib2fv(__GLcontext *gc, GLuint index, const GLfloat *v)
{
    __GL_HEADER();

    __glVertexAttrib4f(gc, index, v[0], v[1], 0.0f, 1.0f);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_VertexAttrib3f(__GLcontext *gc, GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
    __GL_HEADER();

    __glVertexAttrib4f(gc, index, x, y, z, 1.0f);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_VertexAttrib3fv(__GLcontext *gc, GLuint index, const GLfloat *v)
{
    __GL_HEADER();

    __glVertexAttrib4f(gc, index, v[0], v[1], v[2], 1.0f);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_VertexAttrib4f(__GLcontext *gc, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GL_HEADER();

    __glVertexAttrib4f(gc, index, x, y, z, w);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_VertexAttrib4fv(__GLcontext *gc, GLuint index, const GLfloat *v)
{
    __GL_HEADER();

    __glVertexAttrib4f(gc, index, v[0], v[1], v[2], v[3]);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_VertexAttribI4i(__GLcontext *gc, GLuint index, GLint x, GLint y, GLint z, GLint w)
{
    __GL_HEADER();

    __glVertexAttribI4ui(gc, index, (GLuint)x, (GLuint)y, (GLuint)z, (GLuint)w);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_VertexAttribI4ui(__GLcontext *gc, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
    __GL_HEADER();

    __glVertexAttribI4ui(gc, index, x, y, z, w);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_VertexAttribI4iv(__GLcontext *gc, GLuint index, const GLint *v)
{
    __GL_HEADER();

    __glVertexAttribI4ui(gc, index, (GLuint)v[0], (GLuint)v[1], (GLuint)v[2], (GLuint)v[3]);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_VertexAttribI4uiv(__GLcontext *gc, GLuint index, const GLuint *v)
{
    __GL_HEADER();

    __glVertexAttribI4ui(gc, index, v[0], v[1], v[2], v[3]);

    __GL_FOOTER();
}

#ifdef OPENGL40
GLvoid APIENTRY __glim_VertexAttrib1s(__GLcontext *gc, GLuint index, GLshort x)
{
    __glVertexAttrib4f(gc, index, x, 0.0f, 0.0f, 1.0f);
}


GLvoid APIENTRY __glim_VertexAttrib1d(__GLcontext *gc, GLuint index, GLdouble x)
{
    __glVertexAttrib4f(gc, index, (GLfloat)x, 0.0f, 0.0f, 1.0f);
}

GLvoid APIENTRY __glim_VertexAttrib2s(__GLcontext *gc, GLuint index, GLshort x, GLshort y)
{
    __glVertexAttrib4f(gc, index, x, y, 0.0f, 1.0f);
}


GLvoid APIENTRY __glim_VertexAttrib2d(__GLcontext *gc, GLuint index, GLdouble x, GLdouble y)
{
    __glVertexAttrib4f(gc, index, (GLfloat)x, (GLfloat)y, 0.0f, 1.0f);
}

GLvoid APIENTRY __glim_VertexAttrib3s(__GLcontext *gc, GLuint index, GLshort x, GLshort y, GLshort z)
{
    __glVertexAttrib4f(gc, index, x, y, z, 1.0f);
}


GLvoid APIENTRY __glim_VertexAttrib3d(__GLcontext *gc, GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
    __glVertexAttrib4f(gc, index, (GLfloat)x, (GLfloat)y, (GLfloat)z, 1.0f);
}

GLvoid APIENTRY __glim_VertexAttrib4s(__GLcontext *gc, GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)
{
    __glVertexAttrib4f(gc, index, x, y, z, w);
}


GLvoid APIENTRY __glim_VertexAttrib4d(__GLcontext *gc, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    __glVertexAttrib4f(gc, index, (GLfloat)x, (GLfloat)y, (GLfloat)z, (GLfloat)w);
}

GLvoid APIENTRY __glim_VertexAttrib1sv(__GLcontext *gc, GLuint index, const GLshort *v)
{
    __glVertexAttrib4f(gc, index, v[0], 0.0f, 0.0f, 1.0f);
}


GLvoid APIENTRY __glim_VertexAttrib1dv(__GLcontext *gc, GLuint index, const GLdouble *v)
{
    __glVertexAttrib4f(gc, index, (GLfloat)v[0], 0.0f, 0.0f, 1.0f);
}

GLvoid APIENTRY __glim_VertexAttrib2sv(__GLcontext *gc, GLuint index, const GLshort *v)
{
    __glVertexAttrib4f(gc, index, v[0], v[1], 0.0f, 1.0f);
}


GLvoid APIENTRY __glim_VertexAttrib2dv(__GLcontext *gc, GLuint index, const GLdouble *v)
{
    __glVertexAttrib4f(gc,index, (GLfloat)v[0], (GLfloat)v[1], 0.0f, 1.0f);
}

GLvoid APIENTRY __glim_VertexAttrib3sv(__GLcontext *gc, GLuint index, const GLshort *v)
{
    __glVertexAttrib4f(gc,index, v[0], v[1], v[2], 1.0f);
}

GLvoid APIENTRY __glim_VertexAttrib3dv(__GLcontext *gc, GLuint index, const GLdouble *v)
{
    __glVertexAttrib4f(gc, index, (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], 1.0f);
}

GLvoid APIENTRY __glim_VertexAttrib4bv(__GLcontext *gc, GLuint index, const GLbyte *v)
{
    __glVertexAttrib4f(gc, index, (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3]);
}

GLvoid APIENTRY __glim_VertexAttrib4sv(__GLcontext *gc, GLuint index, const GLshort *v)
{
    __glVertexAttrib4f(gc, index, (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3]);
}

GLvoid APIENTRY __glim_VertexAttrib4iv(__GLcontext *gc, GLuint index, const GLint *v)
{
    __glVertexAttrib4f(gc, index, (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3]);
}

GLvoid APIENTRY __glim_VertexAttrib4ubv(__GLcontext *gc, GLuint index, const GLubyte *v)
{
    __glVertexAttrib4f(gc, index, v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3]);
}

GLvoid APIENTRY __glim_VertexAttrib4usv(__GLcontext *gc, GLuint index, const GLushort *v)
{
    __glVertexAttrib4f(gc, index, v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3]);
}

GLvoid APIENTRY __glim_VertexAttrib4uiv(__GLcontext *gc, GLuint index, const GLuint *v)
{
    __glVertexAttrib4f(gc, index, (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3]);
}


GLvoid APIENTRY __glim_VertexAttrib4dv(__GLcontext *gc, GLuint index, const GLdouble *v)
{
    __glVertexAttrib4f(gc, index, (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3]);
}

GLvoid APIENTRY __glim_VertexAttrib4Nub(__GLcontext *gc, GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)
{
    __glVertexAttrib4f(gc, index, __GL_UB_TO_FLOAT(x), __GL_UB_TO_FLOAT(y),
        __GL_UB_TO_FLOAT(z), __GL_UB_TO_FLOAT(w));
}

GLvoid APIENTRY __glim_VertexAttrib4Nbv(__GLcontext *gc, GLuint index, const GLbyte *v)
{
    __glVertexAttrib4f(gc, index, __GL_B_TO_FLOAT(v[0]), __GL_B_TO_FLOAT(v[1]),
        __GL_B_TO_FLOAT(v[2]), __GL_B_TO_FLOAT(v[3]));
}

GLvoid APIENTRY __glim_VertexAttrib4Nsv(__GLcontext *gc, GLuint index, const GLshort *v)
{
    __glVertexAttrib4f(gc, index, __GL_S_TO_FLOAT(v[0]), __GL_S_TO_FLOAT(v[1]),
        __GL_S_TO_FLOAT(v[2]), __GL_S_TO_FLOAT(v[3]));
}

GLvoid APIENTRY __glim_VertexAttrib4Niv(__GLcontext *gc, GLuint index, const GLint *v)
{
    __glVertexAttrib4f(gc, index, __GL_I_TO_FLOAT(v[0]), __GL_I_TO_FLOAT(v[1]),
        __GL_I_TO_FLOAT(v[2]), __GL_I_TO_FLOAT(v[3]));
}

GLvoid APIENTRY __glim_VertexAttrib4Nubv(__GLcontext *gc, GLuint index, const GLubyte *v)
{
    __glVertexAttrib4f(gc, index, __GL_UB_TO_FLOAT(v[0]), __GL_UB_TO_FLOAT(v[1]),
        __GL_UB_TO_FLOAT(v[2]), __GL_UB_TO_FLOAT(v[3]));
}

GLvoid APIENTRY __glim_VertexAttrib4Nusv(__GLcontext *gc, GLuint index, const GLushort *v)
{
    __glVertexAttrib4f(gc, index, __GL_US_TO_FLOAT(v[0]), __GL_US_TO_FLOAT(v[1]),
        __GL_US_TO_FLOAT(v[2]), __GL_US_TO_FLOAT(v[3]));
}

GLvoid APIENTRY __glim_VertexAttrib4Nuiv(__GLcontext *gc, GLuint index, const GLuint *v)
{
    __glVertexAttrib4f(gc, index, __GL_UI_TO_FLOAT(v[0]), __GL_UI_TO_FLOAT(v[1]),
        __GL_UI_TO_FLOAT(v[2]), __GL_UI_TO_FLOAT(v[3]));
}

GLvoid GL_APIENTRY __glim_VertexAttribI1i(__GLcontext *gc, GLuint index, GLint x)
{
    __GL_HEADER();

    __glVertexAttribI4ui(gc, index, (GLuint)x, 0, 0, 0);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_VertexAttribI2i(__GLcontext *gc, GLuint index, GLint x, GLint y)
{
    __GL_HEADER();

    __glVertexAttribI4ui(gc, index, (GLuint)x, (GLuint)y, 0, 0);

    __GL_FOOTER();
}


GLvoid GL_APIENTRY __glim_VertexAttribI3i(__GLcontext *gc, GLuint index, GLint x, GLint y, GLint z)
{
    __GL_HEADER();

    __glVertexAttribI4ui(gc, index, (GLuint)x, (GLuint)y, (GLuint)z, 0);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_VertexAttribI1ui(__GLcontext *gc, GLuint index, GLuint x)
{
    __GL_HEADER();

    __glVertexAttribI4ui(gc, index, (GLuint)x, 0, 0, 0);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_VertexAttribI2ui(__GLcontext *gc, GLuint index, GLuint x, GLuint y)
{
    __GL_HEADER();

    __glVertexAttribI4ui(gc, index, (GLuint)x, (GLuint)y, 0, 0);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_VertexAttribI3ui(__GLcontext *gc, GLuint index, GLuint x, GLuint y, GLuint z)
{
    __GL_HEADER();

    __glVertexAttribI4ui(gc, index, (GLuint)x, (GLuint)y, (GLuint)z, 0);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_VertexAttribI1iv(__GLcontext *gc, GLuint index, const GLint *v)
{
    __GL_HEADER();

    __glVertexAttribI4ui(gc, index, (GLuint)v[0], 0, 0, 0);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_VertexAttribI2iv(__GLcontext *gc, GLuint index, const GLint *v)
{
    __GL_HEADER();

    __glVertexAttribI4ui(gc, index, (GLuint)v[0], (GLuint)v[1], 0, 0);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_VertexAttribI3iv(__GLcontext *gc, GLuint index, const GLint *v)
{
    __GL_HEADER();

    __glVertexAttribI4ui(gc, index, (GLuint)v[0], (GLuint)v[1], (GLuint)v[2], 0);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_VertexAttribI1uiv(__GLcontext *gc, GLuint index, const GLuint *v)
{
    __GL_HEADER();

    __glVertexAttribI4ui(gc, index, v[0], 0, 0, 0);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_VertexAttribI2uiv(__GLcontext *gc, GLuint index, const GLuint *v)
{
    __GL_HEADER();

    __glVertexAttribI4ui(gc, index, v[0], v[1], 0, 0);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_VertexAttribI3uiv(__GLcontext *gc, GLuint index, const GLuint *v)
{
    __GL_HEADER();

    __glVertexAttribI4ui(gc, index, v[0], v[1], v[2], 0);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_VertexAttribI4bv(__GLcontext *gc, GLuint index, const GLbyte *v)
{
#ifdef VIV_GL4_TODO_API
    gcoOS_Print(" VIV Warning: [TODO] File:%s, Line:%d Not Implemented API! \n",__FILE__,__LINE__);
#endif
}

GLvoid GL_APIENTRY __glim_VertexAttribI4sv(__GLcontext *gc, GLuint index, const GLshort *v)
{
#ifdef VIV_GL4_TODO_API
    gcoOS_Print(" VIV Warning: [TODO] File:%s, Line:%d Not Implemented API! \n",__FILE__,__LINE__);
#endif
}

GLvoid GL_APIENTRY __glim_VertexAttribI4ubv(__GLcontext *gc, GLuint index, const GLubyte *v)
{
#ifdef VIV_GL4_TODO_API
    gcoOS_Print(" VIV Warning: [TODO] File:%s, Line:%d Not Implemented API! \n",__FILE__,__LINE__);
#endif
}

GLvoid GL_APIENTRY __glim_VertexAttribI4usv(__GLcontext *gc, GLuint index, const GLushort *v)
{
#ifdef VIV_GL4_TODO_API
    gcoOS_Print(" VIV Warning: [TODO] File:%s, Line:%d Not Implemented API! \n",__FILE__,__LINE__);
#endif
}

GLvoid APIENTRY __glim_GetVertexAttribdv(__GLcontext *gc,  GLuint index, GLenum pname, GLdouble *params)
{
    __GLvertexAttrib *pAttrib;
    __GLvertexArrayState * pVertexArrayState = &gc->vertexArray.boundVAO->vertexArray;
    GLuint bit;

    __GL_SETUP_NOT_IN_BEGIN(gc);


    if ((index >= __GL_MAX_PROGRAM_VERTEX_ATTRIBUTES) || (params == NULL))
    {
        __glSetError(gc, GL_INVALID_VALUE);
        return;
    }

#ifdef OPENGL40
    if (gc->imports.conformGLSpec)
    {
        index = index + __GL_VARRAY_ATT0_INDEX;
    }
#endif

    pVertexArrayState = &gc->clientState.vertexArray;
    pAttrib = &pVertexArrayState->attribute[index];

    switch(pname)
    {
    case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
        bit = __GL_VARRAY_ATT0 << index;
        *params = (pVertexArrayState->attribEnabled & bit) ? 1 : 0;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_SIZE:
        *params = pAttrib->size;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
        *params = pAttrib->stride;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_TYPE:
        *params = pAttrib->type;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
        *params = pAttrib->normalized;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_INTEGER_EXT:
        *params = pAttrib->integer;
        break;

    case GL_CURRENT_VERTEX_ATTRIB:
        if (index > 0)
        {
#ifdef OPENGL40
            if (gc->imports.conformGLSpec)
            {
                *params++ = gc->state.current.currentState[index].f.x;
                *params++ = gc->state.current.currentState[index].f.y;
                *params++ = gc->state.current.currentState[index].f.z;
                *params++ = gc->state.current.currentState[index].f.w;
            }
            else  /* Running OES api */
#endif
            {
                *params++ = gc->state.current.attribute[index].f.x;
                *params++ = gc->state.current.attribute[index].f.y;
                *params++ = gc->state.current.attribute[index].f.z;
                *params++ = gc->state.current.attribute[index].f.w;
            }
        }
        else
            __glSetError(gc, GL_INVALID_OPERATION);
        break;

    default:
        __glSetError(gc, GL_INVALID_VALUE);
        break;
    }
}

#endif

/*
** Minimum vertex numbers for each primitive type.
*/
GLsizei g_minVertexNumber[] =
{
    1,  /* GL_POINTS */
    2,  /* GL_LINES */
    2,  /* GL_LINE_LOOP */
    2,  /* GL_LINE_STRIP */
    3,  /* GL_TRIANGLES */
    3,  /* GL_TRIANGLE_STRIP */
    3,  /* GL_TRIANGLE_FAN */
    ~0U,  /* 0x7 */
    ~0U,  /* 0x8 */
    ~0U,  /* 0x9 */
    4,  /* GL_LINES_ADJACENCY_EXT */
    4,  /* GL_LINE_STRIP_ADJACENCY_EXT */
    6,  /* GL_TRIANGLES_ADJACENCY_EXT */
    6,  /* GL_TRIANGLE_STRIP_ADJACENCY_EXT */
    0,  /* GL_PATCHES_EXT */

};

#define __GL_CHECK_VERTEX_COUNT(gc, mode, count)    \
    if (count < g_minVertexNumber[mode])            \
    {                                               \
        gc->flags |= __GL_CONTEXT_SKIP_DRAW_INSUFFICIENT_VERTEXCOUNT;   \
    }                                                                   \
    else                                                                \
    {                                                                   \
        gc->flags &= ~__GL_CONTEXT_SKIP_DRAW_INSUFFICIENT_VERTEXCOUNT;  \
    }


#define __GL_CHECK_INSTANCE_COUNT(primCount) \
    if (primCount < 1)                       \
    {                                        \
        return;                              \
    }


static GLboolean __glCheckVAOState(__GLcontext *gc, GLboolean attribMustFromVBO, GLboolean IdxMustFromVBO)
{
    GLuint index = 0;
    GLuint64 attribEnabled;
    __GLbufferObject *boundIdxObj = __glGetBoundBufObj(gc, __GL_ELEMENT_ARRAY_BUFFER_INDEX);
    __GLvertexArrayState *curVertexArray = &gc->vertexArray.boundVAO->vertexArray;

    GL_ASSERT(curVertexArray);

    if (IdxMustFromVBO && !boundIdxObj)
    {
        __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
    }

    if (boundIdxObj && boundIdxObj->bufferMapped)
    {
        __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
    }

    attribEnabled = curVertexArray->attribEnabled;
    while (attribEnabled)
    {
        if (attribEnabled & 0x1)
        {
            if (gc->imports.conformGLSpec)
            {
                __GLvertexAttrib *pAttrib = &curVertexArray->attribute[index];
                __GLvertexAttribBinding *pAttribBinding = &curVertexArray->attributeBinding[pAttrib->attribBinding];
                if (attribMustFromVBO)
                {
                    if (pAttribBinding->boundArrayName == 0)
                    {
                        __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
                    }
                }

                if (pAttribBinding->boundArrayObj && pAttribBinding->boundArrayObj->name != 0 &&
                    pAttribBinding->boundArrayObj->bufferMapped)
                {
                    __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
                }
            }
            else
            {
                GLuint binding = curVertexArray->attribute[index].attribBinding;
                __GLbufferObject *boundVBObj = __glGetCurrentVertexArrayBufObj(gc, binding);

                if (attribMustFromVBO)
                {
                    if (!boundVBObj)
                    {
                        __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
                    }
                }

                if (boundVBObj && boundVBObj->bufferMapped)
                {
                    __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
                }
            }
        }
        index++;
        attribEnabled >>= 1;
    }

    return GL_TRUE;
}

/*
** Update the state of a vertex array and set gc->vertexArray.varrayDirty if state changes
** gc->vertexArray.varrayDirty will trigger vertex array validation
**
*/
static GLvoid  __glUpdateVertexArray(__GLcontext *gc,
                                     GLuint attribIdx,
                                     GLuint bindingIdx,
                                     GLint size,
                                     GLenum type,
                                     GLboolean normalized,
                                     GLboolean integer,
                                     GLsizei stride,
                                     const GLvoid *pointer)
{
    __GLvertexArrayState *vertexArrayState = &gc->vertexArray.boundVAO->vertexArray;
    __GLvertexAttrib *pAttrib = &vertexArrayState->attribute[attribIdx];
    __GLvertexAttribBinding *pAttribBinding = &vertexArrayState->attributeBinding[bindingIdx];
    __GLbufferObject *newBufObj = gc->bufferObject.generalBindingPoint[__GL_ARRAY_BUFFER_INDEX].boundBufObj;
    __GLbufferObject *oldBufObj = gc->imports.conformGLSpec ? pAttribBinding->boundArrayObj
                                            :__glGetCurrentVertexArrayBufObj(gc, bindingIdx);
    GLsizei actualStride = stride ? stride : __glUtilCalculateStride(size, type);

#ifdef OPENGL40
    if(gc->imports.conformGLSpec)
    {
        pAttrib->stride = actualStride;
    }
#endif

    /* if (oldBufObj != newBufObj) */
    {
        if (!oldBufObj || !newBufObj)
        {
            /* If there is switch between buffer objects and conventional vertex array,
            ** set the format and offset bits dirty
            */
            __GL_SET_VARRAY_FORMAT_BIT(gc);
            __GL_SET_VARRAY_OFFSET_BIT(gc);
        }

        if (gc->vertexArray.boundVertexArray)
        {
            /* Remove current VAO from old buffer object's vaoList */
            if (oldBufObj)
            {
                __glRemoveImageUser(gc, &oldBufObj->vaoList, gc->vertexArray.boundVAO);
                if (!oldBufObj->bindCount && !oldBufObj->vaoList &&
                    !oldBufObj->texList && (oldBufObj->flag & __GL_OBJECT_IS_DELETED))
                {
                    __glDeleteBufferObject(gc, oldBufObj);
                }
            }

            /* Add current VAO into new buffer object's vaoList */
            if (newBufObj)
            {
                __glAddImageUser(gc, &newBufObj->vaoList, gc->vertexArray.boundVAO);
            }
        }

        pAttribBinding->boundArrayName = gc->bufferObject.generalBindingPoint[__GL_ARRAY_BUFFER_INDEX].boundBufName;
        pAttribBinding->boundArrayObj = newBufObj;

        __GL_SET_VARRAY_BINDING_BIT(gc);
    }

    if (pAttrib->size != size || pAttrib->type != type || pAttrib->normalized != normalized ||
        pAttrib->usr_stride != stride || pAttrib->integer != integer)
    {
        pAttrib->size = size;
        pAttrib->type = type;
        pAttrib->normalized = normalized;
        pAttrib->usr_stride = stride;
        pAttrib->integer = integer;
        __GL_SET_VARRAY_FORMAT_BIT(gc);
    }

    if (pAttribBinding->stride != actualStride)
    {
        pAttribBinding->stride = actualStride;
    }

    if (pAttrib->attribBinding != bindingIdx)
    {
        pAttrib->attribBinding = bindingIdx;
        __GL_SET_VARRAY_BINDING_BIT(gc);
    }

    if (newBufObj)
    {
        /* "pointer" is an offset */
        if (pAttribBinding->offset != __GL_PTR2INT(pointer))
        {
            pAttribBinding->offset = __GL_PTR2INT(pointer);
            __GL_SET_VARRAY_OFFSET_BIT(gc);
        }
    }

    /* "pointer" is a real pointer */
    if ((pAttrib->pointer != pointer) || (pAttrib->relativeOffset != 0))
    {
        pAttrib->pointer = pointer;
        pAttrib->relativeOffset = 0;
        __GL_SET_VARRAY_OFFSET_BIT(gc);
    }
}

static GLboolean __glCheckXFBState(__GLcontext *gc, GLboolean allowXFB, GLenum mode, GLsizei vertexCount, GLsizei instanceCount)
{
    __GLxfbObject *xfbObj;

    xfbObj = gc->xfb.boundXfbObj;

    if (allowXFB)
    {
        /* For GS extension enable case, the check is deferred to __glChipDrawBegin */
        if ((!__glExtension[__GL_EXTID_EXT_geometry_shader].bEnabled) &&
            (!__glExtension[__GL_EXTID_EXT_tessellation_shader].bEnabled))
        {
            if (xfbObj->active && !xfbObj->paused && xfbObj->primMode != mode)
            {
                __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
            }

            if (xfbObj->active && !xfbObj->paused)
            {
                GLuint numPrims = gc->imports.conformGLSpec ? vertexCount : 0;
                GLuint numVerts = gc->imports.conformGLSpec ? vertexCount : 0;

                /* VIV TODO: Stream index */
                __GLqueryObject *queryObj = gc->query.currQuery[__GL_QUERY_XFB_PRIMITIVES_WRITTEN][0];

                switch (mode)
                {
                case GL_TRIANGLES:
                    numPrims = (vertexCount / 3) * instanceCount;
                    numVerts = numPrims * 3;
                    break;
                case GL_LINES:
                    numPrims = (vertexCount / 2) * instanceCount;
                    numVerts = numPrims * 2;
                    break;
                case GL_POINTS:
                    numPrims = vertexCount * instanceCount;
                    numVerts = numPrims;
                    break;
                }

                if (!(*gc->dp.checkXFBBufSizes)(gc, xfbObj, numVerts))
                {
                    __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
                }

                xfbObj->vertices = numVerts;

                /* Update query object for xfb. Handled by the CPU for now. */
                if (queryObj && queryObj->active)
                {
                    queryObj->count += numPrims;
                }
            }
        }
    }
    else
    {
        if (xfbObj->active && !xfbObj->paused)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
        }
    }

    return GL_TRUE;

}

__GL_INLINE GLvoid __glDrawRangeElements(__GLcontext *gc, GLenum mode, GLsizei count,
                                         GLenum type, const GLvoid *indices)
{
    __GLvertexArrayMachine *vertexArray = &gc->vertexArray;

    /* The indices might be used as an offset */
    vertexArray->indexCount = count;
    vertexArray->indices = indices;
    vertexArray->indexType = type;
    vertexArray->drawIndirect = GL_FALSE;
    gc->vertexArray.multidrawIndirect = GL_FALSE;

    __GL_CHECK_VERTEX_COUNT(gc, mode, count);

#ifdef OPENGL40
     /* Config __GLvertexStreamMachine */
    if (gc->imports.conformGLSpec &&
        gc->input.beginMode != __GL_IN_BEGIN && gc->input.beginMode != __GL_SMALL_LIST_BATCH)
    {
        __glConfigArrayVertexStream(gc, mode);
    }
#endif

    __glDrawPrimitive(gc, mode);
}

GLvoid GL_APIENTRY __glim_VertexAttribPointer(__GLcontext *gc, GLuint index, GLint size, GLenum type,
                                              GLboolean normalized, GLsizei stride, const GLvoid *pointer)
{
    __GL_HEADER();

    if (index >= gc->constants.shaderCaps.maxUserVertAttributes)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

#ifdef OPENGL40
    if (gc->imports.conformGLSpec)
    {
        index = index + __GL_VARRAY_ATT0_INDEX;
    }
#endif

    if ((stride < 0) || (stride > (GLsizei)gc->constants.maxVertexAttribStride)|| (size < 1) || (size > 4))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    switch (type)
    {
    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
    case GL_FIXED:
    case GL_FLOAT:
    case GL_HALF_FLOAT_OES:
#ifdef OPENGL40
    case GL_DOUBLE:
#endif
        break;

    /* GL_OES_vertex_type_10_10_10_2 requires INVALID value error */
    case GL_INT_10_10_10_2_OES:
    case GL_UNSIGNED_INT_10_10_10_2_OES:
        if (4 != size && 3 != size)
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
        break;

    case GL_INT:
    case GL_UNSIGNED_INT:
    case GL_HALF_FLOAT:
        break;

    case GL_INT_2_10_10_10_REV:
    case GL_UNSIGNED_INT_2_10_10_10_REV:
        if (4 != size)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (0 != gc->vertexArray.boundVertexArray &&
        0 == gc->bufferObject.generalBindingPoint[__GL_ARRAY_BUFFER_INDEX].boundBufName &&
        pointer)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

     __glUpdateVertexArray(gc, index, index, size, type, normalized, GL_FALSE, stride, pointer);

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_VertexAttribIPointer(__GLcontext *gc, GLuint index, GLint size, GLenum type,
                                               GLsizei stride, const GLvoid *pointer)
{
    __GL_HEADER();

    if (index >= gc->constants.shaderCaps.maxUserVertAttributes)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

#ifdef OPENGL40
    if (gc->imports.conformGLSpec)
    {
        index = index + __GL_VARRAY_ATT0_INDEX;
    }
#endif

    if ((stride < 0) || (stride > (GLsizei)gc->constants.maxVertexAttribStride) || (size <= 0) || (size > 4))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
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
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (0 != gc->vertexArray.boundVertexArray &&
        0 == gc->bufferObject.generalBindingPoint[__GL_ARRAY_BUFFER_INDEX].boundBufName &&
        pointer)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    __glUpdateVertexArray(gc, index, index, size, type, GL_FALSE, GL_TRUE, stride, pointer);

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_EnableVertexAttribArray(__GLcontext *gc, GLuint index)
{
    __GLvertexArrayState * pVertexArrayState = &gc->vertexArray.boundVAO->vertexArray;

    __GL_HEADER();

    if (index >= gc->constants.shaderCaps.maxUserVertAttributes)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

#ifdef OPENGL40
    if (gc->imports.conformGLSpec)
    {
        index = index + __GL_VARRAY_ATT0_INDEX;
    }
#endif

    if ((pVertexArrayState->attribEnabled & (__GL_ONE_64 << index)) == 0)
    {
        pVertexArrayState->attribEnabled |= (__GL_ONE_64 << index);
        __GL_SET_VARRAY_ENABLE_BIT(gc);
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_DisableVertexAttribArray(__GLcontext *gc, GLuint index)
{
    __GLvertexArrayState * pVertexArrayState = &gc->vertexArray.boundVAO->vertexArray;

    __GL_HEADER();

    if (index >= gc->constants.shaderCaps.maxUserVertAttributes)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

#ifdef OPENGL40
    if (gc->imports.conformGLSpec)
    {
        index = index + __GL_VARRAY_ATT0_INDEX;
    }
#endif

    if (pVertexArrayState->attribEnabled & (__GL_ONE_64 << index))
    {
        pVertexArrayState->attribEnabled &= ~(__GL_ONE_64 << index);
        __GL_SET_VARRAY_ENABLE_BIT(gc);
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_DrawElements(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type,
                                       const GLvoid *indices)
{
    __GL_HEADER();

    switch (type)
    {
    case GL_UNSIGNED_BYTE:
    case GL_UNSIGNED_SHORT:
    case GL_UNSIGNED_INT:
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (count < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

#ifdef OPENGL40
    if (gc->imports.conformGLSpec &&
        mode > GL_TRIANGLE_STRIP_ADJACENCY_EXT)
    {
        __glSetError(gc, GL_INVALID_ENUM);
        return;
    }
    else
#endif
    if ((mode > GL_TRIANGLE_FAN) &&
        (mode < GL_LINES_ADJACENCY_EXT || mode > GL_PATCHES_EXT))
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (!__glCheckVAOState(gc, GL_FALSE, GL_FALSE))
    {
        __GL_EXIT();
    }

    if (!__glCheckXFBState(gc,
            __glExtension[__GL_EXTID_EXT_geometry_shader].bEnabled,
            mode,
            count,
            1))
    {
        __GL_EXIT();
    }

    gc->vertexArray.start = 0;
    gc->vertexArray.end = 0;
    gc->vertexArray.baseVertex = 0;
    gc->vertexArray.instanceCount = 1;

    __glDrawRangeElements(gc, mode, count, type, indices);

OnExit:
OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_VertexAttribDivisor(__GLcontext *gc, GLuint index, GLuint divisor)
{
    __GLvertexArrayState *vertexArrayState = &gc->vertexArray.boundVAO->vertexArray;
    __GLvertexAttrib *pAttrib;
    __GLvertexAttribBinding *pAttribBinding;

    __GL_HEADER();

    if (index >= gc->constants.shaderCaps.maxUserVertAttributes)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

#ifdef OPENGL40
    if (gc->imports.conformGLSpec)
    {
        index = index + __GL_VARRAY_ATT0_INDEX;
    }
#endif

    pAttrib = &vertexArrayState->attribute[index];
    pAttribBinding = &vertexArrayState->attributeBinding[index];
    pAttrib->attribBinding = index;

    if (pAttribBinding->divisor != divisor)
    {
        pAttribBinding->divisor = divisor;
        __GL_SET_VARRAY_DIVISOR_BIT(gc);
    }

OnError:
    __GL_FOOTER();
}

GLvoid __glDrawArraysInstanced(__GLcontext *gc, GLenum mode, GLint first, GLsizei count,
                                           GLsizei instanceCount, GLboolean fromDrawXFB)
{
    __GLvertexArrayMachine *vertexArray = &gc->vertexArray;

    if (first < 0 || count < 0 || instanceCount < 0)
    {
        __GL_ERROR_RET(GL_INVALID_VALUE);
    }

    if ((mode > GL_TRIANGLE_FAN) &&
        ((mode < GL_LINES_ADJACENCY_EXT) ||
         (mode > GL_PATCHES_EXT)))
    {
        if (gc->imports.coreProfile)
        {
            __GL_ERROR_RET(GL_INVALID_ENUM);
        }
        else if (mode != GL_QUADS && mode != GL_QUAD_STRIP && mode != GL_POLYGON)
        {
            __GL_ERROR_RET(GL_INVALID_ENUM);
        }
    }

    __GL_CHECK_INSTANCE_COUNT(instanceCount);

    if (!__glCheckVAOState(gc, GL_FALSE, GL_FALSE))
    {
        return;
    }

    if (!__glCheckXFBState(gc, GL_TRUE, mode, count, instanceCount))
    {
        return;
    }

    vertexArray->indexCount = 0;
    vertexArray->instanceCount  = instanceCount;
    vertexArray->start = first;
    vertexArray->end = first + count;
    vertexArray->baseVertex = first;
    vertexArray->fromDrawXFB = fromDrawXFB;
    gc->vertexArray.drawIndirect = GL_FALSE;
    gc->vertexArray.multidrawIndirect = GL_FALSE;

    __GL_CHECK_VERTEX_COUNT(gc, mode, count)

#ifdef OPENGL40
     /* Config __GLvertexStreamMachine */
    if (gc->imports.conformGLSpec &&
        gc->input.beginMode != __GL_IN_BEGIN && gc->input.beginMode != __GL_SMALL_LIST_BATCH)
    {
        __glConfigArrayVertexStream(gc, mode);
    }
#endif

    __glDrawPrimitive(gc, mode);
}


GLvoid GL_APIENTRY __glim_DrawArraysInstanced(__GLcontext *gc, GLenum mode, GLint first, GLsizei count,
                                              GLsizei instanceCount)
{
    __GL_HEADER();

    __glDrawArraysInstanced(gc, mode, first, count, instanceCount, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_DrawArrays(__GLcontext *gc, GLenum mode, GLint first, GLsizei count)
{
    __GL_HEADER();

    __glDrawArraysInstanced(gc, mode, first, count, 1, GL_FALSE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_DrawElementsInstanced(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type,
                                                const GLvoid *indices, GLsizei instanceCount)
{
    __GL_HEADER();

    switch (type)
    {
    case GL_UNSIGNED_BYTE:
    case GL_UNSIGNED_SHORT:
    case GL_UNSIGNED_INT:
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (count < 0 || instanceCount < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if ((mode > GL_TRIANGLE_FAN) &&
        ((mode < GL_LINES_ADJACENCY_EXT) ||
         (mode > GL_PATCHES_EXT)))
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    __GL_CHECK_INSTANCE_COUNT(instanceCount);

    if (!__glCheckVAOState(gc, GL_FALSE, GL_FALSE))
    {
        __GL_EXIT();
    }

    if (!__glCheckXFBState(gc,
            __glExtension[__GL_EXTID_EXT_geometry_shader].bEnabled,
            mode,
            count,
            instanceCount))
    {
        __GL_EXIT();
    }


    gc->vertexArray.start = 0;
    gc->vertexArray.end = 0;
    gc->vertexArray.baseVertex = 0;
    gc->vertexArray.instanceCount = instanceCount;

    __glDrawRangeElements(gc, mode, count, type, indices);

OnExit:
OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_DrawRangeElements(__GLcontext *gc, GLenum mode, GLuint start, GLuint end,
                                            GLsizei count, GLenum type, const GLvoid *indices)
{
    __GL_HEADER();

    switch (type)
    {
    case GL_UNSIGNED_BYTE:
    case GL_UNSIGNED_SHORT:
    case GL_UNSIGNED_INT:
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (count < 0 || start > end)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if ((mode > GL_TRIANGLE_FAN) &&
        ((mode < GL_LINES_ADJACENCY_EXT) ||
         (mode > GL_PATCHES_EXT)))
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (!__glCheckVAOState(gc, GL_FALSE, GL_FALSE))
    {
        __GL_EXIT();
    }

    if (!__glCheckXFBState(gc,
            __glExtension[__GL_EXTID_EXT_geometry_shader].bEnabled,
            mode,
            count,
            1))
    {
        __GL_EXIT();
    }

    gc->vertexArray.instanceCount = 1;
    gc->vertexArray.start = start;
    gc->vertexArray.end = end + 1;
    gc->vertexArray.baseVertex = 0;

    __glDrawRangeElements(gc, mode, count, type, indices);

OnExit:
OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_DrawArraysIndirect(__GLcontext *gc, GLenum mode, const void *indirect)
{
    GLsizeiptr offset = (GLsizeiptr)indirect;
    __GLbufferObject *indirectObj;

    __GL_HEADER();

    if ((mode > GL_TRIANGLE_FAN) &&
        ((mode < GL_LINES_ADJACENCY_EXT) ||
         (mode > GL_PATCHES_EXT)))
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    indirectObj = gc->bufferObject.generalBindingPoint[__GL_DRAW_INDIRECT_BUFFER_INDEX].boundBufObj;

    if (!gc->vertexArray.boundVertexArray ||
        !indirectObj || indirectObj->bufferMapped)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (offset & 0x3)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (offset < 0 || offset >= indirectObj->size ||
        (GLsizeiptr)(offset + sizeof(__GLdrawArraysIndirectCommand)) > indirectObj->size)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (!__glCheckVAOState(gc, GL_TRUE, GL_FALSE))
    {
        __GL_EXIT();
    }

    if (!__glCheckXFBState(gc,
            __glExtension[__GL_EXTID_EXT_geometry_shader].bEnabled,
            mode,
            0,
            1))
    {
        __GL_EXIT();
    }

    /* Assign fake start, end values to make primcount > 0,
    ** 32(max patch vertices)is to pass ts vertex count check */
    gc->vertexArray.start = 0xdeadbeef;
    gc->vertexArray.end = 0xdeadbeef + 32;
    gc->vertexArray.instanceCount = 1;
    gc->vertexArray.indexCount = 0;
    gc->vertexArray.indices = 0;
    gc->vertexArray.indirectOffset = indirect;
    gc->vertexArray.drawIndirect = GL_TRUE;
    gc->vertexArray.baseVertex = 0;
    gc->vertexArray.multidrawIndirect = GL_FALSE;
    gc->flags &= ~__GL_CONTEXT_SKIP_DRAW_INSUFFICIENT_VERTEXCOUNT;

    __glDrawPrimitive(gc, mode);

OnExit:
OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_DrawElementsIndirect(__GLcontext *gc, GLenum mode, GLenum type, const void *indirect)
{
    GLsizeiptr offset = (GLsizeiptr)indirect;
    __GLbufferObject *indirectObj;

    __GL_HEADER();

    if ((mode > GL_TRIANGLE_FAN) &&
        ((mode < GL_LINES_ADJACENCY_EXT) ||
         (mode > GL_PATCHES_EXT)))
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    switch (type)
    {
    case GL_UNSIGNED_BYTE:
    case GL_UNSIGNED_SHORT:
    case GL_UNSIGNED_INT:
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    indirectObj = gc->bufferObject.generalBindingPoint[__GL_DRAW_INDIRECT_BUFFER_INDEX].boundBufObj;

    if (!gc->vertexArray.boundVertexArray ||
        !__glGetBoundBufObj(gc, __GL_ELEMENT_ARRAY_BUFFER_INDEX) ||
        !indirectObj || indirectObj->bufferMapped)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (offset & 0x3)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (offset < 0 || offset >= indirectObj->size ||
        (GLsizeiptr)(offset + sizeof(__GLdrawElementsIndirectCommand)) > indirectObj->size)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (!__glCheckVAOState(gc, GL_TRUE, GL_TRUE))
    {
        __GL_EXIT();
    }

    if (!__glCheckXFBState(gc,
            __glExtension[__GL_EXTID_EXT_geometry_shader].bEnabled,
            mode,
            0,
            1))
    {
        __GL_EXIT();
    }

    gc->vertexArray.start = 0;
    gc->vertexArray.end = 0;
    gc->vertexArray.instanceCount = 1;
    gc->vertexArray.indexCount = 0xdeadbeef;
    gc->vertexArray.indexType = type;
    gc->vertexArray.indices = 0;
    gc->vertexArray.indirectOffset = indirect;
    gc->vertexArray.drawIndirect = GL_TRUE;
    gc->vertexArray.baseVertex = 0;
    gc->vertexArray.multidrawIndirect = GL_FALSE;
    gc->flags &= ~__GL_CONTEXT_SKIP_DRAW_INSUFFICIENT_VERTEXCOUNT;

    __glDrawPrimitive(gc, mode);

OnExit:
OnError:
    __GL_FOOTER();
}

#if GL_EXT_multi_draw_indirect
GLvoid GL_APIENTRY __glim_MultiDrawArraysIndirect(__GLcontext *gc, GLenum mode, const void *indirect,
                                                  GLsizei drawcount, GLsizei stride)
{
    GLsizeiptr offset = (GLsizeiptr)indirect;
    __GLbufferObject *indirectObj;

    __GL_HEADER();

    if ((mode > GL_TRIANGLE_FAN) &&
        ((mode < GL_LINES_ADJACENCY_EXT) ||
         (mode > GL_PATCHES_EXT)))
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if ((stride != 0) && ((stride % 4) != 0))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (drawcount < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    indirectObj = gc->bufferObject.generalBindingPoint[__GL_DRAW_INDIRECT_BUFFER_INDEX].boundBufObj;

    if (!gc->vertexArray.boundVertexArray ||
        !indirectObj || indirectObj->bufferMapped)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (offset & 0x3)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (stride == 0)
    {
        stride = sizeof(__GLdrawArraysIndirectCommand);
    }

    if (offset < 0 || (offset + stride * drawcount) > indirectObj->size)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (!__glCheckVAOState(gc, GL_TRUE, GL_FALSE))
    {
        __GL_EXIT();
    }

    if (!__glCheckXFBState(gc,
            __glExtension[__GL_EXTID_EXT_geometry_shader].bEnabled,
            mode,
            0,
            1))
    {
        __GL_EXIT();
    }

    /* Assign fake start, end values to make primcount > 0,
    ** 32(max patch vertices)is to pass ts vertex count check */
    gc->vertexArray.start = 0xdeadbeef;
    gc->vertexArray.end = 0xdeadbeef + 32;
    gc->vertexArray.instanceCount = 1;
    gc->vertexArray.indexCount = 0;
    gc->vertexArray.indices = 0;
    gc->vertexArray.indirectOffset = indirect;
    gc->vertexArray.baseVertex = 0;
    gc->vertexArray.drawcount = drawcount;
    gc->vertexArray.stride = stride;
    gc->vertexArray.multidrawIndirect = GL_TRUE;
    gc->vertexArray.drawIndirect = GL_FALSE;
    gc->flags &= ~__GL_CONTEXT_SKIP_DRAW_INSUFFICIENT_VERTEXCOUNT;

    __glDrawPrimitive(gc, mode);

OnExit:
OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_MultiDrawElementsIndirect(__GLcontext *gc, GLenum mode, GLenum type, const void *indirect,
                                                    GLsizei drawcount, GLsizei stride)
{
    GLsizeiptr offset = (GLsizeiptr)indirect;
    __GLbufferObject *indirectObj;

    __GL_HEADER();

    if ((mode > GL_TRIANGLE_FAN) &&
        ((mode < GL_LINES_ADJACENCY_EXT) ||
         (mode > GL_PATCHES_EXT)))
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if ((stride != 0) && ((stride % 4) != 0))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (drawcount < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    switch (type)
    {
    case GL_UNSIGNED_BYTE:
    case GL_UNSIGNED_SHORT:
    case GL_UNSIGNED_INT:
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    indirectObj = gc->bufferObject.generalBindingPoint[__GL_DRAW_INDIRECT_BUFFER_INDEX].boundBufObj;

    if (!gc->vertexArray.boundVertexArray ||
        !__glGetBoundBufObj(gc, __GL_ELEMENT_ARRAY_BUFFER_INDEX) ||
        !indirectObj || indirectObj->bufferMapped)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (offset & 0x3)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (stride == 0)
    {
        stride = sizeof(__GLdrawElementsIndirectCommand);
    }

    if (offset < 0 || (offset + stride * drawcount) > indirectObj->size)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (!__glCheckVAOState(gc, GL_TRUE, GL_TRUE))
    {
        __GL_EXIT();
    }

    if (!__glCheckXFBState(gc,
            __glExtension[__GL_EXTID_EXT_geometry_shader].bEnabled,
            mode,
            0,
            1))
    {
        __GL_EXIT();
    }

    gc->vertexArray.start = 0;
    gc->vertexArray.end = 0;
    gc->vertexArray.instanceCount = 1;
    gc->vertexArray.indexCount = 0xdeadbeef;
    gc->vertexArray.indexType = type;
    gc->vertexArray.indices = 0;
    gc->vertexArray.indirectOffset = indirect;
    gc->vertexArray.multidrawIndirect = GL_TRUE;
    gc->vertexArray.baseVertex = 0;
    gc->vertexArray.drawcount = drawcount;
    gc->vertexArray.stride = stride;
    gc->vertexArray.drawIndirect = GL_FALSE;
    gc->flags &= ~__GL_CONTEXT_SKIP_DRAW_INSUFFICIENT_VERTEXCOUNT;

    __glDrawPrimitive(gc, mode);

OnExit:
OnError:
    __GL_FOOTER();
}
#endif

GLvoid GL_APIENTRY __glim_DrawElementsBaseVertex(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex)
{
    __GL_HEADER();

    switch (type)
    {
    case GL_UNSIGNED_BYTE:
    case GL_UNSIGNED_SHORT:
    case GL_UNSIGNED_INT:
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (count < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if ((mode > GL_TRIANGLE_FAN) &&
        ((mode < GL_LINES_ADJACENCY_EXT) ||
         (mode > GL_PATCHES_EXT)))
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (!__glCheckVAOState(gc, GL_FALSE, GL_FALSE))
    {
        __GL_EXIT();
    }

    if (!__glCheckXFBState(gc,
            __glExtension[__GL_EXTID_EXT_geometry_shader].bEnabled,
            mode,
            count,
            1))
    {
        __GL_EXIT();
    }


    gc->vertexArray.start = 0;
    gc->vertexArray.end = 0;
    gc->vertexArray.baseVertex = basevertex;
    gc->vertexArray.instanceCount = 1;

    __glDrawRangeElements(gc, mode, count, type, indices);

OnExit:
OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_DrawRangeElementsBaseVertex(__GLcontext *gc, GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex)
{
    __GL_HEADER();

    switch (type)
    {
    case GL_UNSIGNED_BYTE:
    case GL_UNSIGNED_SHORT:
    case GL_UNSIGNED_INT:
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (count < 0 || start > end)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if ((mode > GL_TRIANGLE_FAN) &&
        ((mode < GL_LINES_ADJACENCY_EXT) ||
         (mode > GL_PATCHES_EXT)))
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (!__glCheckVAOState(gc, GL_FALSE, GL_FALSE))
    {
        __GL_EXIT();
    }

    if (!__glCheckXFBState(gc,
            __glExtension[__GL_EXTID_EXT_geometry_shader].bEnabled,
            mode,
            count,
            1))
    {
        __GL_EXIT();
    }
    gc->vertexArray.instanceCount = 1;
    gc->vertexArray.start = start;
    gc->vertexArray.end = end + 1;
    gc->vertexArray.baseVertex = basevertex;

    __glDrawRangeElements(gc, mode, count, type, indices);

OnExit:
OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_DrawElementsInstancedBaseVertex(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instanceCount, GLint basevertex)
{
    __GL_HEADER();

    switch (type)
    {
    case GL_UNSIGNED_BYTE:
    case GL_UNSIGNED_SHORT:
    case GL_UNSIGNED_INT:
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (count < 0 || instanceCount < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if ((mode > GL_TRIANGLE_FAN) &&
        ((mode < GL_LINES_ADJACENCY_EXT) ||
         (mode > GL_PATCHES_EXT)))
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    __GL_CHECK_INSTANCE_COUNT(instanceCount);

    if (!__glCheckVAOState(gc, GL_FALSE, GL_FALSE))
    {
        __GL_EXIT();
    }

    if (!__glCheckXFBState(gc,
            __glExtension[__GL_EXTID_EXT_geometry_shader].bEnabled,
            mode,
            count,
            instanceCount))
    {
        __GL_EXIT();
    }

    gc->vertexArray.start = 0;
    gc->vertexArray.end = 0;
    gc->vertexArray.baseVertex = basevertex;
    gc->vertexArray.instanceCount = instanceCount;

    __glDrawRangeElements(gc, mode, count, type, indices);

OnExit:
OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_MultiDrawElementsBaseVertex(__GLcontext *gc, GLenum mode, const GLsizei * count, GLenum type, const void *const *indices, GLsizei drawCount, const GLint * baseVertex)
{
    GLsizei i;

    __GL_HEADER();

    if (drawCount < 0 || !count || !indices)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    for (i = 0; i < drawCount; ++i)
    {
        __glim_DrawElementsBaseVertex(gc, mode, count[i], type, indices[i], baseVertex ? baseVertex[i] : 0);
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_GetVertexAttribfv(__GLcontext *gc,  GLuint index, GLenum pname, GLfloat *params)
{
    __GLvertexAttrib *pAttrib;
    __GLvertexAttribBinding *pAttribBinding;
    __GLvertexArrayState * pVertexArrayState = &gc->vertexArray.boundVAO->vertexArray;

    __GL_HEADER();

    if ((index >= gc->constants.shaderCaps.maxUserVertAttributes) || (params == gcvNULL))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

#ifdef OPENGL40
    if (gc->imports.conformGLSpec)
    {
        index = index + __GL_VARRAY_ATT0_INDEX;
    }
#endif

    pAttrib = &pVertexArrayState->attribute[index];
    pAttribBinding = &pVertexArrayState->attributeBinding[pAttrib->attribBinding];

    switch (pname)
    {
    case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:
        *params = (GLfloat)pAttribBinding->boundArrayName;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
        *params = (GLfloat)((pVertexArrayState->attribEnabled & (__GL_ONE_64 << index)) ? 1 : 0);
        break;

    case GL_VERTEX_ATTRIB_ARRAY_SIZE:
        *params = (GLfloat)pAttrib->size;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
        *params = (GLfloat)pAttrib->usr_stride;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_TYPE:
        *params = (GLfloat)pAttrib->type;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
        *params = (GLfloat)pAttrib->normalized;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_INTEGER:
        *params = (GLfloat)pAttrib->integer;
        break;

    case GL_CURRENT_VERTEX_ATTRIB:
#ifdef OPENGL40
        if (gc->imports.conformGLSpec)
        {
            *params++ = gc->state.current.currentState[index].f.x;
            *params++ = gc->state.current.currentState[index].f.y;
            *params++ = gc->state.current.currentState[index].f.z;
            *params++ = gc->state.current.currentState[index].f.w;
        }
        else  /* Running OES api */
#endif
        {
            *params++ = gc->state.current.attribute[index].f.x;
            *params++ = gc->state.current.attribute[index].f.y;
            *params++ = gc->state.current.attribute[index].f.z;
            *params++ = gc->state.current.attribute[index].f.w;
        }
        break;

    case GL_VERTEX_ATTRIB_ARRAY_DIVISOR:
        *params   = (GLfloat)pAttribBinding->divisor;
        break;

    case GL_VERTEX_ATTRIB_BINDING:
        *params = (GLfloat) pAttrib->attribBinding;
        break;

    case GL_VERTEX_ATTRIB_RELATIVE_OFFSET:
        *params = (GLfloat)pAttrib->relativeOffset;
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_GetVertexAttribiv(__GLcontext *gc,  GLuint index, GLenum pname, GLint *params)
{
    __GLvertexAttrib *pAttrib;
    __GLvertexAttribBinding *pAttribBinding;
    __GLvertexArrayState * pVertexArrayState = &gc->vertexArray.boundVAO->vertexArray;

    __GL_HEADER();

    if ((index >= gc->constants.shaderCaps.maxUserVertAttributes) || (params == gcvNULL))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

#ifdef OPENGL40
    if (gc->imports.conformGLSpec)
    {
        index = index + __GL_VARRAY_ATT0_INDEX;
    }
#endif

    pAttrib = &pVertexArrayState->attribute[index];
    pAttribBinding = &pVertexArrayState->attributeBinding[pAttrib->attribBinding];

    switch (pname)
    {
    case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:
        *params = pAttribBinding->boundArrayName;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
        *params = (pVertexArrayState->attribEnabled & (__GL_ONE_64 << index)) ? 1 : 0;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_SIZE:
        *params = pAttrib->size;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
        *params = pAttrib->usr_stride;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_TYPE:
        *params = pAttrib->type;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
        *params = pAttrib->normalized;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_INTEGER:
        *params = pAttrib->integer;
        break;

    case GL_CURRENT_VERTEX_ATTRIB:
#ifdef OPENGL40
        if (gc->imports.conformGLSpec)
        {
            *params++ = (GLint) gc->state.current.currentState[index].f.x;
            *params++ = (GLint) gc->state.current.currentState[index].f.y;
            *params++ = (GLint) gc->state.current.currentState[index].f.z;
            *params++ = (GLint) gc->state.current.currentState[index].f.w;
        }
        else  /* Running OES api */
#endif
        {
            *params++ = (GLint) gc->state.current.attribute[index].f.x;
            *params++ = (GLint) gc->state.current.attribute[index].f.y;
            *params++ = (GLint) gc->state.current.attribute[index].f.z;
            *params++ = (GLint) gc->state.current.attribute[index].f.w;
        }
        break;

    case GL_VERTEX_ATTRIB_ARRAY_DIVISOR:
        *params   = pAttribBinding->divisor;
        break;

    case GL_VERTEX_ATTRIB_BINDING:
        *params = pAttrib->attribBinding;
        break;

    case GL_VERTEX_ATTRIB_RELATIVE_OFFSET:
        *params = pAttrib->relativeOffset;
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_GetVertexAttribPointerv(__GLcontext *gc, GLuint index, GLenum pname, GLvoid**pointer)
{
    __GLvertexAttrib *pAttrib;

    __GL_HEADER();

    if ((index >= gc->constants.shaderCaps.maxUserVertAttributes) || (pointer == gcvNULL))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

#ifdef OPENGL40
    if (gc->imports.conformGLSpec)
    {
        index = index + __GL_VARRAY_ATT0_INDEX;
    }
#endif

    pAttrib = &gc->vertexArray.boundVAO->vertexArray.attribute[index];
    switch (pname)
    {
    case GL_VERTEX_ATTRIB_ARRAY_POINTER:
        *pointer = (GLvoid *)pAttrib->pointer;
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_GetVertexAttribIiv(__GLcontext *gc, GLuint index, GLenum pname, GLint *params)
{
    __GLvertexArrayState * pVertexArrayState = &gc->vertexArray.boundVAO->vertexArray;
    __GLvertexAttrib *pAttrib;
    __GLvertexAttribBinding *pAttribBinding;
    GLuint *uparams = (GLuint*)params;

    __GL_HEADER();

    if ((index >= gc->constants.shaderCaps.maxUserVertAttributes) || (params == gcvNULL))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

#ifdef OPENGL40
    if (gc->imports.conformGLSpec)
    {
        index = index + __GL_VARRAY_ATT0_INDEX;
    }
#endif

    pAttrib = &pVertexArrayState->attribute[index];
    pAttribBinding = &pVertexArrayState->attributeBinding[pAttrib->attribBinding];

    switch (pname)
    {
    case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
        *params = (pVertexArrayState->attribEnabled & (__GL_ONE_64 << index)) ? 1 : 0;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_SIZE:
        *params = pAttrib->size;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
        *params = pAttrib->usr_stride;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_TYPE:
        *params = pAttrib->type;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
        *params = pAttrib->normalized;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_INTEGER:
        *params = pAttrib->integer;
        break;

    case GL_CURRENT_VERTEX_ATTRIB:
#ifdef OPENGL40
        if (gc->imports.conformGLSpec)
        {
            *uparams++ = gc->state.current.currentState[index].i.ix;
            *uparams++ = gc->state.current.currentState[index].i.iy;
            *uparams++ = gc->state.current.currentState[index].i.iz;
            *uparams++ = gc->state.current.currentState[index].i.iw;
        }
        else  /* Running OES api */
#endif
        {
            *uparams++ = gc->state.current.attribute[index].i.ix;
            *uparams++ = gc->state.current.attribute[index].i.iy;
            *uparams++ = gc->state.current.attribute[index].i.iz;
            *uparams++ = gc->state.current.attribute[index].i.iw;
        }
        break;

    case GL_VERTEX_ATTRIB_ARRAY_DIVISOR:
        *params   = pAttribBinding->divisor;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:
        *params = pAttribBinding->boundArrayName;
        break;

    case GL_VERTEX_ATTRIB_BINDING:
        *params = pAttrib->attribBinding;
        break;

    case GL_VERTEX_ATTRIB_RELATIVE_OFFSET:
        *params = pAttrib->relativeOffset;
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
        break;
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_GetVertexAttribIuiv(__GLcontext *gc, GLuint index, GLenum pname, GLuint *params)
{
    __GLvertexAttrib *pAttrib;
    __GLvertexAttribBinding *pAttribBinding;
    __GLvertexArrayState * pVertexArrayState = &gc->vertexArray.boundVAO->vertexArray;

    __GL_HEADER();

    if ((index >= gc->constants.shaderCaps.maxUserVertAttributes) || (params == gcvNULL))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

#ifdef OPENGL40
    if (gc->imports.conformGLSpec)
    {
        index = index + __GL_VARRAY_ATT0_INDEX;
    }
#endif

    pAttrib = &pVertexArrayState->attribute[index];
    pAttribBinding = &pVertexArrayState->attributeBinding[pAttrib->attribBinding];

    switch (pname)
    {
    case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
        *params = (pVertexArrayState->attribEnabled & (__GL_ONE_64 << index)) ? 1 : 0;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_SIZE:
        *params = pAttrib->size;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
        *params = pAttrib->usr_stride;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_TYPE:
        *params = pAttrib->type;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
        *params = pAttrib->normalized;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_INTEGER:
        *params = pAttrib->integer;
        break;

    case GL_CURRENT_VERTEX_ATTRIB:
#ifdef OPENGL40
        if (gc->imports.conformGLSpec)
        {
            *params++ = gc->state.current.currentState[index].i.ix;
            *params++ = gc->state.current.currentState[index].i.iy;
            *params++ = gc->state.current.currentState[index].i.iz;
            *params++ = gc->state.current.currentState[index].i.iw;
        }
        else  /* Running OES api */
#endif
        {
            *params++ = gc->state.current.attribute[index].i.ix;
            *params++ = gc->state.current.attribute[index].i.iy;
            *params++ = gc->state.current.attribute[index].i.iz;
            *params++ = gc->state.current.attribute[index].i.iw;
        }
        break;

    case GL_VERTEX_ATTRIB_ARRAY_DIVISOR:
        *params   = pAttribBinding->divisor;
        break;

    case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:
        *params = pAttribBinding->boundArrayName;
        break;

    case GL_VERTEX_ATTRIB_BINDING:
        *params = pAttrib->attribBinding;
        break;

    case GL_VERTEX_ATTRIB_RELATIVE_OFFSET:
        *params = pAttrib->relativeOffset;
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

OnError:
    __GL_FOOTER();
}

/*
** VAO
*/
GLvoid __glInitVertexArrayObject(__GLcontext *gc, __GLvertexArrayObject *vertexArrayObj, GLuint array)
{
    GLint i;
    __GLvertexArrayState *pArrayState;

    __GL_HEADER();

    GL_ASSERT(vertexArrayObj);

    vertexArrayObj->name = array;
    pArrayState = &vertexArrayObj->vertexArray;

    for (i = 0; i < __GL_MAX_VERTEX_ATTRIBUTES; i++)
    {
        pArrayState->attribute[i].normalized = GL_FALSE;
        pArrayState->attribute[i].size       = 4;
        pArrayState->attribute[i].type       = GL_FLOAT;
        pArrayState->attribute[i].usr_stride = 0;
        pArrayState->attribute[i].integer    = GL_FALSE;
        pArrayState->attribute[i].relativeOffset = 0;
        pArrayState->attribute[i].pointer    = NULL;
        pArrayState->attribute[i].attribBinding = i;
    }

    for (i = 0; i < __GL_MAX_VERTEX_ATTRIBUTE_BINDINGS; i++)
    {
        pArrayState->attributeBinding[i].boundArrayName = 0;
        pArrayState->attributeBinding[i].boundArrayObj = NULL;
        pArrayState->attributeBinding[i].divisor = 0;
        pArrayState->attributeBinding[i].offset = 0;
        pArrayState->attributeBinding[i].stride = 16;
    }

    pArrayState->attribEnabled = 0;

#ifdef OPENGL40
    pArrayState->clientActiveUnit = 0;
#endif
    __GL_FOOTER();
}

GLvoid __glBindVertexArray(__GLcontext *gc, GLuint array)
{
    __GLvertexArrayObject *vertexArrayObj;

    __GL_HEADER();

    if (gc->vertexArray.boundVertexArray == array)
    {
        __GL_EXIT();
    }

    if (array)
    {
        if (!__glIsNameDefined(gc, gc->vertexArray.noShare, array))
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }

        vertexArrayObj = (__GLvertexArrayObject *)__glGetObject(gc, gc->vertexArray.noShare, array);
        if (vertexArrayObj == gcvNULL)
        {
            vertexArrayObj = (__GLvertexArrayObject*)(*gc->imports.calloc)(gc, 1, sizeof(__GLvertexArrayObject));

            __glInitVertexArrayObject(gc,  vertexArrayObj, array);
            __glAddObject(gc, gc->vertexArray.noShare, array, vertexArrayObj);
            __glMarkNameUsed(gc, gc->vertexArray.noShare, array);
        }
    }
    else
    {
        /* Bind vertex array object to 0 which means client vertex array */
        vertexArrayObj = &gc->vertexArray.defaultVAO;
    }

    /* Bind to new vertex array object */
    gc->vertexArray.boundVertexArray = array;
    gc->vertexArray.boundVAO = vertexArrayObj;

    /* Set all the vertex array dirty bits */
    __GL_SET_VARRAY_ENABLE_BIT(gc);
    __GL_SET_VARRAY_FORMAT_BIT(gc);
    __GL_SET_VARRAY_BINDING_BIT(gc);

OnExit:
OnError:
    __GL_FOOTER();
}

GLboolean __glDeleteVertexArrayObject(__GLcontext *gc, __GLvertexArrayObject *vertexArrayObj)
{
    GLint i;
    __GLbufferObject *bufObj;

    __GL_HEADER();

    GL_ASSERT(vertexArrayObj->name);

    for (i = 0; i < __GL_MAX_VERTEX_ATTRIBUTE_BINDINGS; i++)
    {
        bufObj = vertexArrayObj->vertexArray.attributeBinding[i].boundArrayObj;
        if (bufObj)
        {
            /* Remove the VAO from the buffer object's VAO list */
            __glRemoveImageUser(gc, &bufObj->vaoList, vertexArrayObj);

            if (!bufObj->bindCount && !bufObj->vaoList &&
                !bufObj->texList && (bufObj->flag & __GL_OBJECT_IS_DELETED))
            {
                __glDeleteBufferObject(gc, bufObj);
            }
        }
    }

    bufObj = vertexArrayObj->vertexArray.boundIdxObj;
    if (bufObj)
    {
        __glRemoveImageUser(gc, &bufObj->vaoList, vertexArrayObj);
        if (!bufObj->bindCount && !bufObj->vaoList &&
            !bufObj->texList && (bufObj->flag & __GL_OBJECT_IS_DELETED))
        {
            __glDeleteBufferObject(gc, bufObj);
        }
    }

    /*
    ** If a currently bound object is deleted, the binding point reverts to 0
    ** which means the default vertex array becomes current.
    */
    if (gc->vertexArray.boundVAO == vertexArrayObj)
    {
        __glBindVertexArray(gc, 0);
    }

    if (vertexArrayObj->label)
    {
        gc->imports.free(gc, vertexArrayObj->label);
    }

    /* Delete the vertex array object structure */
    (*gc->imports.free)(gc, vertexArrayObj);

    __GL_FOOTER();

    return GL_TRUE;
}

void __glInitVertexArrayState(__GLcontext *gc)
{
    __GL_HEADER();

    /* Vertex array object cannot be shared between contexts */
    if (gc->vertexArray.noShare == gcvNULL)
    {
        gc->vertexArray.noShare = (__GLsharedObjectMachine*)(*gc->imports.calloc)(gc, 1, sizeof(__GLsharedObjectMachine));

        /* Initialize a linear lookup table for vertex array object */
        gc->vertexArray.noShare->maxLinearTableSize = __GL_MAX_VAO_LINEAR_TABLE_SIZE;
        gc->vertexArray.noShare->linearTableSize = __GL_DEFAULT_VAO_LINEAR_TABLE_SIZE;
        gc->vertexArray.noShare->linearTable = (GLvoid **)
            (*gc->imports.calloc)(gc, 1, gc->vertexArray.noShare->linearTableSize * sizeof(GLvoid *));

        gc->vertexArray.noShare->hashSize = __GL_VAO_HASH_TABLE_SIZE;
        gc->vertexArray.noShare->hashMask = __GL_VAO_HASH_TABLE_SIZE - 1;
        gc->vertexArray.noShare->refcount = 1;
        gc->vertexArray.noShare->deleteObject = (__GLdeleteObjectFunc)__glDeleteVertexArrayObject;
        gc->vertexArray.noShare->immediateInvalid = GL_FALSE;
    }

    __glInitVertexArrayObject(gc, &gc->vertexArray.defaultVAO, 0);

    /* VAO 0 is the default client vertex array state */
    gc->vertexArray.boundVertexArray = (GLuint)-1;
    __glBindVertexArray(gc, 0);

    /* Initialize the primMode to an invalid primitive type */
    gc->vertexArray.primMode = (GLenum)-1;

    /* Init vertex array dirty bits */
    gc->vertexArray.varrayDirty = (GLbitfield)(-1);

#ifdef OPENGL40
    gc->vertexArray.interleaved = GL_FALSE;
    gc->vertexArray.formatChanged =  GL_TRUE;
    gc->vertexArray.fromDrawXFB =  GL_FALSE;
#endif
    __GL_FOOTER();
}

void __glFreeVertexArrayState(__GLcontext *gc)
{
    __GL_HEADER();

    __glFreeSharedObjectState(gc, gc->vertexArray.noShare);

    __GL_FOOTER();
}

__GLbufferObject* __glGetCurrentVertexArrayBufObj(__GLcontext *gc, GLuint binding)
{
    __GLbufferObject *bufObj = gcvNULL;
    __GLvertexAttribBinding *pAttribBinding = gcvNULL;

    GL_ASSERT(binding < __GL_MAX_VERTEX_ATTRIBUTE_BINDINGS);

    pAttribBinding = &gc->vertexArray.boundVAO->vertexArray.attributeBinding[binding];
    if (gc->vertexArray.boundVertexArray)
    {
        /* For named vao, get the bound bufobj of binding time. */
        bufObj = pAttribBinding->boundArrayObj;
    }
    else if (pAttribBinding->boundArrayName)
    {
        /* For default vao, since bufobj will not be kept when it was bound to default vao,
        ** need to retrieve bufobj from its name.
        */
        bufObj = (__GLbufferObject*)__glGetObject(gc, gc->bufferObject.shared, pAttribBinding->boundArrayName);
    }

    return bufObj;
}

GLvoid GL_APIENTRY __glim_BindVertexArray(__GLcontext *gc, GLuint array)
{
    __GL_HEADER();

    __glBindVertexArray(gc, array);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_DeleteVertexArrays(__GLcontext *gc, GLsizei n, const GLuint* arrays)
{
    GLint i;

    __GL_HEADER();

    if (n < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    for (i = 0; i < n; i++)
    {
        __glDeleteObject(gc, gc->vertexArray.noShare, arrays[i]);
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_GenVertexArrays(__GLcontext *gc, GLsizei n, GLuint* arrays)
{
    GLint start, i;

    __GL_HEADER();

    if (n < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (gcvNULL == arrays)
    {
        __GL_EXIT();
    }

    GL_ASSERT(gcvNULL != gc->vertexArray.noShare);

    start = __glGenerateNames(gc, gc->vertexArray.noShare, n);

    for (i = 0; i < n; i++)
    {
        arrays[i] = start + i;
    }

    if (gc->vertexArray.noShare->linearTable)
    {
        __glCheckLinearTableSize(gc, gc->vertexArray.noShare, (start + n));
    }

OnError:
OnExit:
    __GL_FOOTER();
}

GLboolean GL_APIENTRY __glim_IsVertexArray(__GLcontext *gc, GLuint array)
{
    return (gcvNULL != __glGetObject(gc, gc->vertexArray.noShare, array));
}


GLvoid GL_APIENTRY __glim_MultiDrawArrays(__GLcontext *gc, GLenum mode, const GLint *first,
                                             const GLsizei *count, GLsizei primcount)
{
    GLsizei i;

    __GL_HEADER();

    if (primcount < 0 || !first || !count)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    for (i = 0; i < primcount; ++i)
    {
        __glim_DrawArrays(gc, mode, first[i], count[i]);
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_MultiDrawElements(__GLcontext *gc, GLenum mode, const GLsizei *count,
                                            GLenum type, const GLvoid*const*indices, GLsizei primcount)
{
    GLsizei i;

    __GL_HEADER();

    if (primcount < 0 || !count || !indices)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    for (i = 0; i < primcount; ++i)
    {
        __glim_DrawElements(gc, mode, count[i], type, indices[i]);
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_BindVertexBuffer(__GLcontext *gc, GLuint bindingindex, GLuint buffer,
                                           GLintptr offset, GLsizei stride)
{
    __GLbufferObject *bufObj, *oldBufObj;
    __GLvertexAttribBinding *pAttribBinding;

    __GL_HEADER();

    if (bindingindex >= gc->constants.maxVertexAttribBindings)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (stride < 0 || stride > (GLsizei)gc->constants.maxVertexAttribStride || offset < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (gc->vertexArray.boundVertexArray == 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (0 != buffer)
    {
        if (GL_FALSE == __glIsNameDefined(gc, gc->bufferObject.shared, buffer))
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }

        bufObj = (__GLbufferObject*)__glGetObject(gc, gc->bufferObject.shared, buffer);

        if (NULL == bufObj)
        {
            bufObj = (__GLbufferObject *)(*gc->imports.calloc)(gc, 1, sizeof(__GLbufferObject));

            if (!bufObj)
            {
                __GL_ERROR_EXIT(GL_OUT_OF_MEMORY);
            }

            __glInitBufferObject(gc, bufObj, buffer);

            /* Add this buffer object to the "gc->bufferObject.shared" structure. */
            __glAddObject(gc, gc->bufferObject.shared, buffer, bufObj);

            /* Mark the name "buffer" used in the buffer object nameArray.*/
            __glMarkNameUsed(gc, gc->bufferObject.shared, buffer);

            if (!(*gc->dp.bindBuffer)(gc,bufObj,gcvBUFOBJ_TYPE_ARRAY_BUFFER))
            {
                __GL_ERROR((*gc->dp.getError)(gc));
            }
        }
        GL_ASSERT(bufObj);
    }
    else
    {
        bufObj = NULL;
    }

    pAttribBinding = &gc->vertexArray.boundVAO->vertexArray.attributeBinding[bindingindex];
    oldBufObj = pAttribBinding->boundArrayObj;

    if (pAttribBinding->boundArrayName != buffer || (oldBufObj && oldBufObj != bufObj))
    {
        /* Remove current VAO from old buffer object's vaoList */
        if (oldBufObj)
        {
            __glRemoveImageUser(gc, &oldBufObj->vaoList, gc->vertexArray.boundVAO);
            if (!oldBufObj->bindCount && !oldBufObj->vaoList &&
                !oldBufObj->texList && (oldBufObj->flag & __GL_OBJECT_IS_DELETED))
            {
                __glDeleteBufferObject(gc, oldBufObj);
            }
        }

        /* Add current VAO into new buffer object's vaoList */
        if (bufObj)
        {
            __glAddImageUser(gc, &bufObj->vaoList, gc->vertexArray.boundVAO);
        }

        pAttribBinding->boundArrayName = buffer;
        pAttribBinding->boundArrayObj = bufObj;
    }

    pAttribBinding->offset = offset;
    pAttribBinding->stride = stride;

    __GL_SET_VARRAY_BINDING_BIT(gc);

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_VertexAttribFormat(__GLcontext *gc, GLuint attribindex, GLint size,
                                             GLenum type, GLboolean normalized, GLuint relativeoffset)
{
    __GLvertexAttrib *pAttrib;

    __GL_HEADER();

    if (attribindex >= gc->constants.shaderCaps.maxUserVertAttributes)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

#ifdef OPENGL40
    if (gc->imports.conformGLSpec)
    {
        attribindex = attribindex + __GL_VARRAY_ATT0_INDEX;
    }
#endif

    if ((size < 1) || (size > 4))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (relativeoffset > gc->constants.maxVertexAttribRelativeOffset)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    switch (type)
    {
    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
    case GL_FIXED:
    case GL_FLOAT:
    case GL_HALF_FLOAT_OES:
        break;

    case GL_INT:
    case GL_UNSIGNED_INT:
    case GL_HALF_FLOAT:
        break;

    case GL_INT_2_10_10_10_REV:
    case GL_UNSIGNED_INT_2_10_10_10_REV:
        if (4 != size)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (gc->vertexArray.boundVertexArray == 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    pAttrib = &gc->vertexArray.boundVAO->vertexArray.attribute[attribindex];

    pAttrib->size = size;
    pAttrib->integer = GL_FALSE;
    pAttrib->normalized = normalized;
    pAttrib->relativeOffset = relativeoffset;
    pAttrib->type = type;

    __GL_SET_VARRAY_FORMAT_BIT(gc);
    __GL_SET_VARRAY_OFFSET_BIT(gc);

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_VertexAttribIFormat(__GLcontext *gc, GLuint attribindex, GLint size,
                                              GLenum type, GLuint relativeoffset)
{
    __GLvertexAttrib *pAttrib;

    __GL_HEADER();

    if (attribindex >= gc->constants.shaderCaps.maxUserVertAttributes)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

#ifdef OPENGL40
    if (gc->imports.conformGLSpec)
    {
        attribindex = attribindex + __GL_VARRAY_ATT0_INDEX;
    }
#endif

    if ((size < 1) || (size > 4))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (relativeoffset > gc->constants.maxVertexAttribRelativeOffset)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
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
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (gc->vertexArray.boundVertexArray == 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    pAttrib = &gc->vertexArray.boundVAO->vertexArray.attribute[attribindex];

    pAttrib->size = size;
    pAttrib->integer = GL_TRUE;
    pAttrib->normalized = GL_FALSE;
    pAttrib->relativeOffset = relativeoffset;
    pAttrib->type = type;

    __GL_SET_VARRAY_FORMAT_BIT(gc);
    __GL_SET_VARRAY_OFFSET_BIT(gc);

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_VertexAttribBinding(__GLcontext *gc, GLuint attribindex, GLuint bindingindex)
{
    __GLvertexAttrib *pAttrib;

    __GL_HEADER();

    if ((attribindex >= gc->constants.shaderCaps.maxUserVertAttributes) ||
        (bindingindex >= gc->constants.maxVertexAttribBindings))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

#ifdef OPENGL40
    if (gc->imports.conformGLSpec)
    {
        attribindex = attribindex + __GL_VARRAY_ATT0_INDEX;
    }
#endif

    if (gc->vertexArray.boundVertexArray == 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    pAttrib = &gc->vertexArray.boundVAO->vertexArray.attribute[attribindex];
    pAttrib->attribBinding = bindingindex;

    __GL_SET_VARRAY_BINDING_BIT(gc);

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_VertexBindingDivisor(__GLcontext *gc, GLuint bindingindex, GLuint divisor)
{
    __GLvertexAttribBinding *pAttribBinding;

    __GL_HEADER();

    if (bindingindex >= gc->constants.maxVertexAttribBindings)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

#ifdef OPENGL40
    if (gc->imports.conformGLSpec)
    {
        bindingindex = bindingindex + __GL_VARRAY_ATT0_INDEX;
    }
#endif

    if (gc->vertexArray.boundVertexArray == 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    pAttribBinding = &gc->vertexArray.boundVAO->vertexArray.attributeBinding[bindingindex];
    pAttribBinding->divisor = divisor;

    __GL_SET_VARRAY_DIVISOR_BIT(gc);

OnError:
    __GL_FOOTER();
}


#ifdef OPENGL40
GLvoid APIENTRY __glim_VertexPointer(__GLcontext *gc, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    if (stride < 0 || size < 2 || size > 4)
    {
        __glSetError(gc, GL_INVALID_VALUE);
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
        __glSetError(gc, GL_INVALID_ENUM);
        return;
    }

    /* Note: Do not add __GL_VERTEX_BUFFER_FLUSH(gc) in this function */

    __glUpdateVertexArray(gc, __GL_VARRAY_VERTEX_INDEX, __GL_VARRAY_VERTEX_INDEX,
        size, type, GL_FALSE, GL_FALSE, stride, pointer);
    if (gc->vertexArray.varrayDirty & __GL_DIRTY_VARRAY_FORMAT_BIT)
    {
        gc->vertexArray.interleaved = GL_FALSE;
        __GL_VALIDATE_VERTEX_ARRAYS(gc);
    }
}

GLvoid APIENTRY __glim_NormalPointer(__GLcontext *gc, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    if (stride < 0)
    {
        __glSetError(gc, GL_INVALID_VALUE);
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
        __glSetError(gc, GL_INVALID_ENUM);
        return;
    }

    /* Note: Do not add __GL_VERTEX_BUFFER_FLUSH(gc) in this function */

    /*normal should set the normalized flag*/
    __glUpdateVertexArray(gc, __GL_VARRAY_NORMAL_INDEX, __GL_VARRAY_NORMAL_INDEX,
        3, type, GL_TRUE, GL_FALSE, stride, pointer);
    if (gc->vertexArray.varrayDirty & __GL_DIRTY_VARRAY_FORMAT_BIT)
    {
        gc->vertexArray.interleaved = GL_FALSE;
        __GL_VALIDATE_VERTEX_ARRAYS(gc);
    }
}

GLvoid APIENTRY __glim_ColorPointer(__GLcontext *gc, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    if (stride < 0 || size < 3 || size > 4)
    {
        __glSetError(gc, GL_INVALID_VALUE);
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
        __glSetError(gc, GL_INVALID_ENUM);
        return;
    }

    /* Note: Do not add __GL_VERTEX_BUFFER_FLUSH(gc) in this function */

    /*color should set the normalized flag*/
    __glUpdateVertexArray(gc, __GL_VARRAY_DIFFUSE_INDEX, __GL_VARRAY_DIFFUSE_INDEX,
        size, type, GL_TRUE, GL_FALSE, stride, pointer);
    if (gc->vertexArray.varrayDirty & __GL_DIRTY_VARRAY_FORMAT_BIT)
    {
        gc->vertexArray.interleaved = GL_FALSE;
        __GL_VALIDATE_VERTEX_ARRAYS(gc);
    }
}

GLvoid APIENTRY __glim_SecondaryColorPointer(__GLcontext *gc, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    if (stride < 0 || size != 3)
    {
        __glSetError(gc, GL_INVALID_VALUE);
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
        __glSetError(gc, GL_INVALID_ENUM);
        return;
    }

    /* Note: Do not add __GL_VERTEX_BUFFER_FLUSH(gc) in this function */

    /*color should set the normalized flag*/
    __glUpdateVertexArray(gc, __GL_VARRAY_SPECULAR_INDEX, __GL_VARRAY_SPECULAR_INDEX,
        size, type, GL_TRUE, GL_FALSE, stride, pointer);
    if (gc->vertexArray.varrayDirty & __GL_DIRTY_VARRAY_FORMAT_BIT)
    {
        gc->vertexArray.interleaved = GL_FALSE;
        __GL_VALIDATE_VERTEX_ARRAYS(gc);
    }
}

GLvoid APIENTRY __glim_FogCoordPointer(__GLcontext *gc, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    if (stride < 0)
    {
        __glSetError(gc, GL_INVALID_VALUE);
        return;
    }
    switch (type)
    {
    case GL_FLOAT:
    case GL_DOUBLE:
        break;
    default:
        __glSetError(gc, GL_INVALID_ENUM);
        return;
    }

    /* Note: Do not add __GL_VERTEX_BUFFER_FLUSH(gc) in this function */

    __glUpdateVertexArray(gc, __GL_VARRAY_FOGCOORD_INDEX, __GL_VARRAY_FOGCOORD_INDEX,
        1, type, GL_FALSE, GL_FALSE, stride, pointer);
    if (gc->vertexArray.varrayDirty & __GL_DIRTY_VARRAY_FORMAT_BIT)
    {
        gc->vertexArray.interleaved = GL_FALSE;
        __GL_VALIDATE_VERTEX_ARRAYS(gc);
    }
}


GLvoid APIENTRY __glim_EdgeFlagPointer(__GLcontext *gc,  GLsizei stride, const GLvoid *ptr)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    if (stride < 0)
    {
        __glSetError(gc, GL_INVALID_VALUE);
        return;
    }

    /* Note: Do not add __GL_VERTEX_BUFFER_FLUSH(gc) in this function */

    __glUpdateVertexArray(gc, __GL_VARRAY_EDGEFLAG_INDEX, __GL_VARRAY_EDGEFLAG_INDEX,
        1, GL_UNSIGNED_BYTE, GL_FALSE, GL_FALSE, stride, ptr);
    if (gc->vertexArray.varrayDirty & __GL_DIRTY_VARRAY_FORMAT_BIT)
    {
        gc->vertexArray.interleaved = GL_FALSE;
        __GL_VALIDATE_VERTEX_ARRAYS(gc);
    }
}

GLvoid APIENTRY __glim_IndexPointer(__GLcontext *gc, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    if (stride < 0)
    {
        __glSetError(gc, GL_INVALID_VALUE);
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
        __glSetError(gc, GL_INVALID_ENUM);
        return;
    }

    /* Note: Do not add __GL_VERTEX_BUFFER_FLUSH(gc) in this function */

    __glUpdateVertexArray(gc, __GL_VARRAY_COLORINDEX_INDEX, __GL_VARRAY_COLORINDEX_INDEX,
        1, type, GL_FALSE, GL_FALSE, stride, pointer);
    if (gc->vertexArray.varrayDirty & __GL_DIRTY_VARRAY_FORMAT_BIT)
    {
        gc->vertexArray.interleaved = GL_FALSE;
        __GL_VALIDATE_VERTEX_ARRAYS(gc);
    }
}

GLvoid APIENTRY __glim_TexCoordPointer(__GLcontext *gc, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    if (stride < 0 || size < 1 || size > 4)
    {
        __glSetError(gc, GL_INVALID_VALUE);
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
        __glSetError(gc, GL_INVALID_ENUM);
        return;
    }

    /* Note: Do not add __GL_VERTEX_BUFFER_FLUSH(gc) in this function */

    __glUpdateVertexArray(gc,
        __GL_VARRAY_TEX0_INDEX + gc->vertexArray.boundVAO->vertexArray.clientActiveUnit,
        __GL_VARRAY_TEX0_INDEX + gc->vertexArray.boundVAO->vertexArray.clientActiveUnit,
        size, type, GL_FALSE, GL_FALSE, stride, pointer);
    if (gc->vertexArray.varrayDirty & __GL_DIRTY_VARRAY_FORMAT_BIT)
    {
        gc->vertexArray.interleaved = GL_FALSE;
        __GL_VALIDATE_VERTEX_ARRAYS(gc);
    }
}

/*
** validate vertex array
** select immedia mode draw array function pointers
*/
__GL_INLINE GLvoid __glValidateVertexArrays(__GLcontext *gc)
{
    GLuint64 currentEnabled;

    /* Filter out the vertex arrays that are not really needed */
    currentEnabled = ( gc->vertexArray.boundVAO->vertexArray.attribEnabled & gc->input.requiredInputMask);

    /* requiredInputMask  set only bit 0 on for vertexinput, while vertexarray enable bit are saparate for vertex and attrib 0.
    Case attribarray 0 enable, vertexarray disabled. attribarray 0 bit will be cleared.
    Solution: set on attrib 0 when it's enabled and vertex input is required. */
    if (gc->input.requiredInputMask & __GL_VARRAY_VERTEX && gc->vertexArray.boundVAO->vertexArray.attribEnabled & __GL_VARRAY_ATT0)
        currentEnabled |= __GL_VARRAY_ATT0;

    if (currentEnabled != gc->vertexArray.boundVAO->vertexArray.attribEnabled)
    {
        __GL_SET_VARRAY_ENABLE_BIT(gc);
        gc->vertexArray.boundVAO->vertexArray.attribEnabled = currentEnabled;
    }
    else
    {
        gc->vertexArray.varrayDirty &= (~__GL_DIRTY_VARRAY_ENABLE_BIT);
    }

    if (gc->vertexArray.varrayDirty & (__GL_DIRTY_VARRAY_FORMAT_BIT | __GL_DIRTY_VARRAY_ENABLE_BIT))
    {
        /* Validate DrawArrays/ArrayElement function */
        gc->vertexArray.drawArraysFunc = __glim_DrawArrays;
        gc->vertexArray.drawElementsFunc = __glim_DrawElements;
        gc->vertexArray.arrayElementFunc = __glim_ArrayElement;
    }

    /* Finally, Clear the vertex array dirty bits */
    gc->vertexArray.globalDirtyBackup = gc->vertexArray.varrayDirty;
}

#define __GL_SIZE_F \
    sizeof(GLfloat)

#define __GL_SIZE_C \
    (((4*sizeof(GLubyte) + (__GL_SIZE_F-1)) / __GL_SIZE_F) * __GL_SIZE_F)

GLvoid APIENTRY __glim_InterleavedArrays(__GLcontext *gc, GLenum format, GLsizei stride, const GLvoid *pointer)
{
    GLboolean tflag, cflag, nflag;  /* enable/disable flags */
    GLint tcomps, ccomps, vcomps;   /* components per texcoord, color, vertex */
    GLenum ctype = 0;               /* color type */
    GLint coffset = 0, noffset = 0, voffset;/* color, normal, vertex offsets */
    const GLint toffset = 0;        /* always zero */
    GLint defstride;                /* default stride */
    GLint c, f;

    __GL_SETUP_NOT_IN_BEGIN(gc);

    f = __GL_SIZE_F;
    c = __GL_SIZE_C;

    if (stride < 0)
    {
        __glSetError(gc, GL_INVALID_VALUE);
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
          __glSetError(gc, GL_INVALID_ENUM);
          return;
    }

    if (stride==0) {
        stride = defstride;
    }

    __glim_DisableClientState(gc, GL_EDGE_FLAG_ARRAY );
    __glim_DisableClientState(gc, GL_INDEX_ARRAY );
    __glim_DisableClientState(gc, GL_FOG_COORDINATE_ARRAY);
    __glim_DisableClientState(gc, GL_SECONDARY_COLOR_ARRAY );

    /* Texcoords */
    if (tflag) {
        __glim_EnableClientState(gc, GL_TEXTURE_COORD_ARRAY );
        __glUpdateVertexArray(gc,
            __GL_VARRAY_TEX0_INDEX + gc->vertexArray.boundVAO->vertexArray.clientActiveUnit,
            __GL_VARRAY_TEX0_INDEX + gc->vertexArray.boundVAO->vertexArray.clientActiveUnit,
            tcomps, GL_FLOAT, GL_TRUE,
            GL_FALSE, stride, (GLubyte *) pointer + toffset);
    }
    else {
        __glim_DisableClientState(gc, GL_TEXTURE_COORD_ARRAY );
    }

    /* Color */
    if (cflag) {
        __glim_EnableClientState(gc, GL_COLOR_ARRAY );
        __glUpdateVertexArray(gc, __GL_VARRAY_DIFFUSE_INDEX,
            __GL_VARRAY_DIFFUSE_INDEX,
            ccomps, ctype, GL_TRUE,
            GL_FALSE, stride, (GLubyte *) pointer + coffset);
    }
    else {
        __glim_DisableClientState(gc, GL_COLOR_ARRAY );
    }


    /* Normals */
    if (nflag) {
        __glim_EnableClientState(gc, GL_NORMAL_ARRAY );
        __glUpdateVertexArray(gc, __GL_VARRAY_NORMAL_INDEX,
            __GL_VARRAY_NORMAL_INDEX,
            3, GL_FLOAT, GL_TRUE,
            GL_FALSE, stride, (GLubyte *) pointer + noffset);
    }
    else {
        __glim_DisableClientState(gc, GL_NORMAL_ARRAY );
    }

    /* Vertices */
    __glim_EnableClientState(gc, GL_VERTEX_ARRAY );
    __glUpdateVertexArray(gc, __GL_VARRAY_VERTEX_INDEX,
        __GL_VARRAY_VERTEX_INDEX,
        vcomps, GL_FLOAT,
        GL_FALSE, GL_FALSE, stride,
        (GLubyte *) pointer + voffset);

    /*set the interleaved flag*/
    gc->vertexArray.interleaved = GL_TRUE;
}



GLvoid APIENTRY __glim_DrawArrays_Validate(__GLcontext *gc, GLenum mode, GLint first, GLsizei count)
{
    /* Compute the required primitive input mask */
    if (gc->input.inputMaskChanged)
    {
        __glComputeRequiredInputMask(gc);
        gc->input.inputMaskChanged = GL_FALSE;
    }
    gc->input.requiredInputMask = gc->input.currentInputMask & edgeFlagInputMask[gc->input.primMode];

    __glValidateVertexArrays(gc);

    gc->immedModeDispatch.ArrayElement = gc->vertexArray.arrayElementFunc;
    gc->immedModeDispatch.DrawArrays   = gc->vertexArray.drawArraysFunc;
    gc->immedModeDispatch.DrawElements = gc->vertexArray.drawElementsFunc;

    (*gc->immedModeDispatch.DrawArrays)(gc, mode, first, count);
}

GLvoid APIENTRY __glim_DrawElements_Validate(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    /* Compute the required primitive input mask */
    if (gc->input.inputMaskChanged)
    {
        __glComputeRequiredInputMask(gc);
        gc->input.inputMaskChanged = GL_FALSE;
    }
    gc->input.requiredInputMask = gc->input.currentInputMask & edgeFlagInputMask[mode];

    __glValidateVertexArrays(gc);

    gc->immedModeDispatch.ArrayElement = gc->vertexArray.arrayElementFunc;
    gc->immedModeDispatch.DrawArrays   = gc->vertexArray.drawArraysFunc;
    gc->immedModeDispatch.DrawElements = gc->vertexArray.drawElementsFunc;

    (*gc->immedModeDispatch.DrawElements)(gc, mode, count, type, indices);
}

GLvoid APIENTRY __glim_ArrayElement_Validate(__GLcontext *gc, GLint element)
{
    /* Compute the required primitive input mask */
    if (gc->input.inputMaskChanged)
    {
        __glComputeRequiredInputMask(gc);
        gc->input.inputMaskChanged = GL_FALSE;
    }
    gc->input.requiredInputMask = gc->input.currentInputMask & edgeFlagInputMask[gc->input.primMode];

    __glValidateVertexArrays(gc);

    gc->immedModeDispatch.ArrayElement = gc->vertexArray.arrayElementFunc;
    gc->immedModeDispatch.DrawArrays   = gc->vertexArray.drawArraysFunc;
    gc->immedModeDispatch.DrawElements = gc->vertexArray.drawElementsFunc;

    (*gc->immedModeDispatch.ArrayElement)(gc, element);
}

GLvoid APIENTRY __glim_ArrayElement(__GLcontext *gc, GLint element)
{
    GLfloat dataBuf[__GL_TOTAL_VERTEX_ATTRIBUTES * 4];
    GLuint tagBuf[__GL_TOTAL_VERTEX_ATTRIBUTES];
    GLboolean edgeflag;
    GLfloat *bufptr = (GLfloat *)&dataBuf[0];
    GLubyte *edgeptr = (GLubyte *)&edgeflag;
    GLint i, index, loop;
    GLenum error;

    __GL_MEMZERO(tagBuf, __GL_TOTAL_VERTEX_ATTRIBUTES * sizeof(GLuint));

    /* Copy the arrayElement data into dataBuf[] */

    error = __glArrayElement_Generic(gc, element, &bufptr, &edgeptr, tagBuf);
    if (error != GL_NO_ERROR) {
        __glSetError(gc, error);
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
            (*gc->immedModeDispatch.Vertex2fv)(gc, bufptr);
            bufptr += 2;
            loop = GL_FALSE;
            break;
        case __GL_V3F_TAG:
            (*gc->immedModeDispatch.Vertex3fv)(gc, bufptr);
            bufptr += 3;
            loop = GL_FALSE;
            break;
        case __GL_V4F_TAG:
            (*gc->immedModeDispatch.Vertex4fv)(gc, bufptr);
            bufptr += 4;
            loop = GL_FALSE;
            break;
        case __GL_C3F_TAG:
            (*gc->immedModeDispatch.Color3fv)(gc, bufptr);
            bufptr += 3;
            break;
        case __GL_C4F_TAG:
            (*gc->immedModeDispatch.Color4fv)(gc, bufptr);
            bufptr += 4;
            break;
        case __GL_C4UB_TAG:
            (*gc->immedModeDispatch.Color4ubv)(gc, (GLubyte *)bufptr);
            bufptr += 1;
            break;
        case __GL_N3F_TAG:
            (*gc->immedModeDispatch.Normal3fv)(gc, bufptr);
            bufptr += 3;
            break;
        case __GL_TC2F_TAG:
            (*gc->immedModeDispatch.TexCoord2fv)(gc, bufptr);
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
            (*gc->immedModeDispatch.MultiTexCoord2fv)(gc, index, bufptr);
            bufptr += 2;
            break;
        case __GL_TC3F_TAG:
            (*gc->immedModeDispatch.TexCoord3fv)(gc, bufptr);
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
            (*gc->immedModeDispatch.MultiTexCoord3fv)(gc, index, bufptr);
            bufptr += 3;
            break;
        case __GL_TC4F_TAG:
            (*gc->immedModeDispatch.TexCoord4fv)(gc, bufptr);
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
            (*gc->immedModeDispatch.MultiTexCoord4fv)(gc, index, bufptr);
            bufptr += 4;
            break;
        case __GL_EDGEFLAG_TAG:
            (*gc->immedModeDispatch.EdgeFlag)(gc, edgeflag);
            break;
        case __GL_SC3F_TAG:
            (*gc->immedModeDispatch.SecondaryColor3fv)(gc, bufptr);
            bufptr += 3;
            break;
        case __GL_FOG1F_TAG:
            (*gc->immedModeDispatch.FogCoordfv)(gc, bufptr);
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
            (*gc->immedModeDispatch.VertexAttrib4fv)(gc, index, bufptr);
            bufptr += 4;
            break;

        default:
            loop = GL_FALSE;
            break;
        }
        i++;
    }
}
#endif
