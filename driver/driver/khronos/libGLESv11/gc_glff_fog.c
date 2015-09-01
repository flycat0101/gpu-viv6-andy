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


#include "gc_glff_precomp.h"

#define _GC_OBJ_ZONE glvZONE_TRACE

/******************************************************************************\
********************************* GL Name Arrays *******************************
\******************************************************************************/

static GLenum _FogModeNames[] =
{
    GL_LINEAR,          /* glvLINEARFOG */
    GL_EXP,             /* glvEXPFOG */
    GL_EXP2,            /* glvEXP2FOG */
};


/******************************************************************************\
********************** Individual State Setting Functions **********************
\******************************************************************************/

static GLenum _SetFogMode(
    glsCONTEXT_PTR Context,
    const GLfloat* FogMode
    )
{
    GLuint fogMode;
    gcmHEADER_ARG("Context=0x%x FogMode=0x%x", Context, FogMode);

    if (!glfConvertGLEnum(
            _FogModeNames,
            gcmCOUNTOF(_FogModeNames),
            FogMode,
            glvFLOAT,
            &fogMode
            ))
    {
        gcmFOOTER_ARG("return=%d", GL_INVALID_ENUM);
        return GL_INVALID_ENUM;
    }

    Context->hashKey.hashFogMode = fogMode;
    Context->fogStates.mode = (gleFOGMODES) fogMode;

    gcmFOOTER_ARG("return=%d", GL_NO_ERROR);

    return GL_NO_ERROR;
}

static GLenum _SetFogDensity(
    glsCONTEXT_PTR Context,
    const GLfloat* FogDensity
    )
{
    gcmHEADER_ARG("Context=0x%x FogDensity=0x%x", Context, FogDensity);


    /* Density cannot be negative. */
    if (*FogDensity < glvFLOATZERO)
    {
        gcmFOOTER_ARG("return=%d", GL_INVALID_VALUE);
        return GL_INVALID_VALUE;
    }

    Context->fogStates.density = *FogDensity;

    Context->fogStates.expDirty  = GL_TRUE;
    Context->fogStates.exp2Dirty = GL_TRUE;

    /* Set uFogFactors dirty. */
    Context->fsUniformDirty.uFogFactorsDirty = gcvTRUE;

    gcmFOOTER_ARG("return=%d", GL_NO_ERROR);
    return GL_NO_ERROR;
}

static GLenum _SetFogStart(
    glsCONTEXT_PTR Context,
    const GLfloat* FogStart
    )
{
    gcmHEADER_ARG("Context=0x%x FogStart=0x%x", Context, FogStart);

    Context->fogStates.start =  *FogStart;

    Context->fogStates.linearDirty = GL_TRUE;

    /* Set uFogFactors dirty. */
    Context->fsUniformDirty.uFogFactorsDirty = gcvTRUE;

    gcmFOOTER_ARG("return=%d", GL_NO_ERROR);
    return GL_NO_ERROR;
}

static GLenum _SetFogEnd(
    glsCONTEXT_PTR Context,
    const GLfloat* FogEnd
    )
{
    gcmHEADER_ARG("Context=0x%x FogEnd=0x%x", Context, FogEnd);

    Context->fogStates.end = *FogEnd;

    Context->fogStates.linearDirty = GL_TRUE;

    /* Set uFogFactors dirty. */
    Context->fsUniformDirty.uFogFactorsDirty = gcvTRUE;

    gcmFOOTER_ARG("return=%d", GL_NO_ERROR);
    return GL_NO_ERROR;
}

static GLenum _SetFogColor(
    glsCONTEXT_PTR Context,
    const GLfloat* Value
    )
{
    gcmHEADER_ARG("Context=0x%x Value=0x%x", Context, Value);

    glfSetClampedVector4(&Context->fogStates.color, Value);

    /* Set uFogColor dirty. */
    Context->fsUniformDirty.uFogColorDirty = gcvTRUE;

    gcmFOOTER_ARG("return=%d", GL_NO_ERROR);
    return GL_NO_ERROR;
}


/******************************************************************************\
*********************** Support Functions and Definitions **********************
\******************************************************************************/

/*******************************************************************************
**
**  _SetFog
**
**  Specify fog parameters. If fog is enabled, fog affects rasterized geometry,
**  bitmaps, and pixel blocks, but not buffer clear operations. To enable and
**  disable fog, call glEnable and glDisable with argument GL_FOG.
**  Fog is initially disabled.
**
**  INPUT:
**
**      Context
**          Pointer to the current context object.
**
**      Name
**          Specifies a single-valued fog parameter.
**          GL_FOG_MODE, GL_FOG_DENSITY, GL_FOG_START, and GL_FOG_END
**          are accepted.
**
**      Value
**          Specifies the value that Name will be set to.
**
**      Type
**          Specifies the type of Value.
**
**      ValueArraySize
**          Specifies the size of Value.
**
**  OUTPUT:
**
**      Nothing.
*/

static GLenum _SetFog(
    glsCONTEXT_PTR Context,
    GLenum Name,
    const GLfloat* Value,
    gctUINT32 ValueArraySize
    )
{
    GLenum result;
    gcmHEADER_ARG("Context=0x%x Name=0x%x Value=0x%x ValueArraySize=%d",
                  Context, Name, Value, ValueArraySize);
    if (ValueArraySize > 1)
    {
        switch (Name)
        {
        case GL_FOG_COLOR:
            result = _SetFogColor(Context, Value);
            gcmFOOTER_ARG("0x%x", result);
            return result;
        }
    }

    switch (Name)
    {
    case GL_FOG_MODE:
        result = _SetFogMode(Context, Value);
        break;

    case GL_FOG_DENSITY:
        result = _SetFogDensity(Context, Value);
        break;

    case GL_FOG_START:
        result = _SetFogStart(Context, Value);
        break;

    case GL_FOG_END:
        result = _SetFogEnd(Context, Value);
        break;
    default:
        result = GL_INVALID_ENUM;
        break;
    }

    gcmFOOTER_ARG("0x%x", result);
    return result;
}


/*******************************************************************************
**
**  glfSetDefaultFogStates
**
**  Set default fog states.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**  OUTPUT:
**
**      Nothing.
*/

gceSTATUS glfSetDefaultFogStates(
    glsCONTEXT_PTR Context
    )
{
    GLenum result;
    gceSTATUS status;
    gcmHEADER_ARG("Context=0x%x", Context);

    do
    {
        static GLfloat fogMode = GL_EXP;
        static GLfloat value0 = glvFLOATZERO;
        static GLfloat value1 = glvFLOATONE;

        static GLfloat vec0000[] =
            { glvFLOATZERO, glvFLOATZERO, glvFLOATZERO, glvFLOATZERO };

        /* Default hint. */
        Context->fogStates.hint = GL_DONT_CARE;

        /* Set default fog mode. */
        glmERR_BREAK(_SetFogMode(Context, &fogMode));

        /* Set default fog density. */
        glmERR_BREAK(_SetFogDensity(Context, &value1));

        /* Set default fog start and end values. */
        glmERR_BREAK(_SetFogStart(Context, &value0));
        glmERR_BREAK(_SetFogEnd(Context, &value1));

        /* Set default fog color. */
        glmERR_BREAK(_SetFogColor(Context, vec0000));
    }
    while (gcvFALSE);

    status = glmTRANSLATEGLRESULT(result);
    gcmFOOTER();

    return status;
}


/*******************************************************************************
**
**  glfFlushFogStates
**
**  Flush fog states.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**  OUTPUT:
**
**      Nothing.
*/

gceSTATUS glfFlushFogStates(
    glsCONTEXT_PTR Context
    )
{
    gcmHEADER_ARG("Context=0x%x", Context);

    do
    {
        /* Set dirty flag for fog density. */
        Context->fogStates.expDirty  = GL_TRUE;
        Context->fogStates.exp2Dirty = GL_TRUE;

        /* Set dirty flag for fog start and end values. */
        Context->fogStates.linearDirty = GL_TRUE;

        /* Set dirty flag for uFogFactors. */
        Context->fsUniformDirty.uFogFactorsDirty = gcvTRUE;

        /* Set dirty flag for uFogColor. */
        Context->fsUniformDirty.uFogColorDirty = gcvTRUE;
    }
    while (gcvFALSE);

    gcmFOOTER_NO();

    return gcvSTATUS_OK;
}


/*******************************************************************************
**
**  glfEnableFog
**
**  Enable fog.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      Enable
**          Enable flag.
**
**  OUTPUT:
**
**      Nothing.
*/

GLenum glfEnableFog(
    glsCONTEXT_PTR Context,
    GLboolean Enable
    )
{
    gcmHEADER_ARG("Context=0x%x Enable=%d", Context, Enable);
    Context->hashKey.hashFogEnabled =
    Context->fogStates.enabled = Enable;
    gcmFOOTER_ARG("result=0x%04x", GL_NO_ERROR);
    return GL_NO_ERROR;
}


/*******************************************************************************
**
**  glfUpdateLinearFactors
**
**  Update GL_LINEAR factors for the shaders.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**  OUTPUT:
**
**      Nothing.
*/

void glfUpdateLinearFactors(
    glsCONTEXT_PTR Context
    )
{
    gcmHEADER_ARG("Context=0x%x", Context);
    if (Context->fogStates.linearDirty)
    {
        /* Get the current start and end values. */
        GLfloat start = Context->fogStates.start;
        GLfloat end   = Context->fogStates.end;

        /* factor0 = 1 / (s - e) */
        Context->fogStates.linearFactor0 = 1.0f / (start - end);

        /* factor1 = e / (e - s) */
        Context->fogStates.linearFactor1 = end / (end - start);

        /* Validate the factors. */
        Context->fogStates.linearDirty = GL_FALSE;
    }
    gcmFOOTER_NO();
}


/*******************************************************************************
**
**  glfUpdateExpFactors
**
**  Update GL_EXP factors for the shaders.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**  OUTPUT:
**
**      Nothing.
*/

void glfUpdateExpFactors(
    glsCONTEXT_PTR Context
    )
{
    gcmHEADER_ARG("Context=0x%x", Context);
    if (Context->fogStates.expDirty)
    {
        /***********************************************************************
        ** Fog internal definitions.
        */

        /*
            Hardware only has an exponent function with the base of 2.
            Convert the exponent here to be able to substitute the required
            e-based exponent function with available 2-based exponent
            function:

                EXP mode:
                    1 / log(2) = 1.44269504088896
        */

        static const GLfloat _expFogDensityAdjustment  = 1.44269504088896f;

        /* Get fog density. */
        GLfloat density = Context->fogStates.density;

        /* Compute adjusted density. */
        Context->fogStates.expFactor = density * _expFogDensityAdjustment;

        /* Validate the factors. */
        Context->fogStates.expDirty = GL_FALSE;
    }
    gcmFOOTER_NO();
}


/*******************************************************************************
**
**  glfUpdateExpFactors
**
**  Update GL_EXP2 factors for the shaders.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**  OUTPUT:
**
**      Nothing.
*/

void glfUpdateExp2Factors(
    glsCONTEXT_PTR Context
    )
{
    gcmHEADER_ARG("Context=0x%x", Context);
    if (Context->fogStates.exp2Dirty)
    {
        /***********************************************************************
        ** Fog internal definitions.
        */

        /*
            Hardware only has an exponent function with the base of 2.
            Convert the exponent here to be able to substitute the required
            e-based exponent function with available 2-based exponent
            function:

                EXP2 mode:
                    sqrt(1 / log(2)) = 1.2011224088
        */

        static const GLfloat _exp2FogDensityAdjustment = 1.2011224088f;

        /* Get fog density. */
        GLfloat density = Context->fogStates.density;

        /* Compute adjusted density. */
        Context->fogStates.exp2Factor = density * _exp2FogDensityAdjustment;

        /* Validate the factors. */
        Context->fogStates.exp2Dirty = GL_FALSE;
    }
    gcmFOOTER_NO();
}


/*******************************************************************************
**
**  glfQueryFogState
**
**  Queries fog state values.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      Name
**          Specifies the symbolic name of the state to get.
**
**      Type
**          Data format.
**
**  OUTPUT:
**
**      Value
**          Points to the data.
*/

GLboolean glfQueryFogState(
    glsCONTEXT_PTR Context,
    GLenum Name,
    GLvoid* Value,
    gleTYPE Type
    )
{
    GLboolean result = GL_TRUE;

    gcmHEADER_ARG("Context=0x%x Name=0x%04x Value=0x%x Type=0x%04x",
                    Context, Name, Value, Type);

    switch (Name)
    {
    case GL_FOG:
        glfGetFromInt(
            Context->fogStates.enabled,
            Value,
            Type
            );
        break;

    case GL_FOG_COLOR:
        glfGetFromVector4(
            &Context->fogStates.color,
            Value,
            Type
            );
        break;

    case GL_FOG_MODE:
        glfGetFromEnum(
            _FogModeNames[Context->fogStates.mode],
            Value,
            Type
            );
        break;

    case GL_FOG_DENSITY:
        glfGetFromFloat(
            Context->fogStates.density,
            Value,
            Type
            );
        break;

    case GL_FOG_START:
        glfGetFromFloat(
            Context->fogStates.start,
            Value,
            Type
            );
        break;

    case GL_FOG_END:
        glfGetFromFloat(
            Context->fogStates.end,
            Value,
            Type
            );
        break;

    case GL_FOG_HINT:
        glfGetFromEnum(
            Context->fogStates.hint,
            Value,
            Type
            );
        break;

    default:
        result = GL_FALSE;
    }

    gcmFOOTER_ARG("result=%d", result);
    /* Return result. */
    return result;
}


/******************************************************************************\
****************************** Fog Management Code *****************************
\******************************************************************************/

/*******************************************************************************
**
**  glFog
**
**  Specify fog parameters. If fog is enabled, fog affects rasterized geometry,
**  bitmaps, and pixel blocks, but not buffer clear operations. To enable and
**  disable fog, call glEnable and glDisable with argument GL_FOG.
**  Fog is initially disabled.
**
**  INPUT:
**
**      Name
**          Specifies a single-valued fog parameter.
**          GL_FOG_MODE, GL_FOG_DENSITY, GL_FOG_START, and GL_FOG_END
**          are accepted.
**
**      Value
**          Specifies the value that Name will be set to.
**
**  OUTPUT:
**
**      Nothing.
*/
#ifdef _GC_OBJ_ZONE
#undef _GC_OBJ_ZONE
#endif
#define _GC_OBJ_ZONE    glvZONE_FOG

GL_API void GL_APIENTRY glFogf(
    GLenum Name,
    GLfloat Value
    )
{
    glmENTER2(glmARGENUM, Name, glmARGFLOAT, Value)
    {
        gcmDUMP_API("${ES11 glFogf 0x%08X 0x%08X}", Name, *(GLuint*)&Value);
        glmPROFILE(context, GLES1_FOGF, 0);

        glmERROR(_SetFog(context, Name, &Value, 1));
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glFogfv(
    GLenum Name,
    const GLfloat* Values
    )
{
    glmENTER2(glmARGENUM, Name, glmARGPTR, Values)
    {
        gcmDUMP_API("${ES11 glFogfv 0x%08X (0x%08X)", Name, Values);
        gcmDUMP_API_ARRAY(Values, 4);
        gcmDUMP_API("$}");

        glmPROFILE(context, GLES1_FOGFV, 0);
        glmERROR(_SetFog(context, Name, Values, 4));
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glFogx(
    GLenum Name,
    GLfixed Value
    )
{
    glmENTER2(glmARGENUM, Name, glmARGFIXED, Value)
    {
        GLfloat value;

        gcmDUMP_API("${ES11 glFogx 0x%08X 0x%08X}", Name, Value);
        glmPROFILE(context, GLES1_FOGX, 0);

        /* Convert to float */
        if( Name == GL_FOG_MODE)
        {
            value = (GLfloat)Value;
        }
        else
        {
            value = glmFIXED2FLOAT(Value);
        }

        glmERROR(_SetFog(context, Name, &value, 1));
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glFogxOES(
    GLenum Name,
    GLfixed Value
    )
{
    glmENTER2(glmARGENUM, Name, glmARGFIXED, Value)
    {
        GLfloat value;

        gcmDUMP_API("${ES11 glFogxOES 0x%08X 0x%08X}", Name, Value);

        /* Convert to float */
        if( Name == GL_FOG_MODE)
        {
            value = (GLfloat)Value;
        }
        else
        {
            value = glmFIXED2FLOAT(Value);
        }

        glmERROR(_SetFog(context, Name, &value, 1));

    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glFogxv(
    GLenum Name,
    const GLfixed* Values
    )
{
    glmENTER2(glmARGENUM, Name, glmARGPTR, Values)
    {
        GLfloat values[4];

        gcmDUMP_API("${ES11 glFogxv 0x%08X (0x%08X)", Name, Values);
        gcmDUMP_API_ARRAY(Values, 4);
        gcmDUMP_API("$}");

        glmPROFILE(context, GLES1_FOGXV, 0);

        /* Convert to float */
        if( Name == GL_FOG_MODE)
        {
            values[0] = (GLfloat)Values[0];
            values[1] = (GLfloat)Values[1];
            values[2] = (GLfloat)Values[2];
            values[3] = (GLfloat)Values[3];
        }
        else
        {
            values[0] = glmFIXED2FLOAT(Values[0]);
            values[1] = glmFIXED2FLOAT(Values[1]);
            values[2] = glmFIXED2FLOAT(Values[2]);
            values[3] = glmFIXED2FLOAT(Values[3]);
        }

        glmERROR(_SetFog(context, Name, values, 4));
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glFogxvOES(
    GLenum Name,
    const GLfixed* Values
    )
{
    glmENTER2(glmARGENUM, Name, glmARGPTR, Values)
    {
        GLfloat values[1];

        gcmDUMP_API("${ES11 glFogxvOES 0x%08X (0x%08X)", Name, Values);
        gcmDUMP_API_ARRAY(Values, 4);
        gcmDUMP_API("$}");

        /* Convert to float */
        if( Name == GL_FOG_MODE)
        {
            values[0] = (GLfloat)Values[0];
        }
        else
        {
            values[0] = glmFIXED2FLOAT(Values[0]);
        }

        glmERROR(_SetFog(context, Name, values, 1));
    }
    glmLEAVE();
}
