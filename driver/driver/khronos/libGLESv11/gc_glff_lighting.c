/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_glff_precomp.h"

#define _GC_OBJ_ZONE gcdZONE_ES11_TRACE

/******************************************************************************\
********************** Individual State Setting Functions **********************
\******************************************************************************/

static GLenum _SetShadeModel(
    glsCONTEXT_PTR Context,
    GLenum ShadeModel
    )
{
    GLenum result = GL_NO_ERROR;
    gcmHEADER_ARG("Context=0x%x ShadeModel=0x%04x", Context, ShadeModel);

    switch (ShadeModel)
    {
    case GL_FLAT:
        /* Set flat shading. */
        Context->lightingStates.shadeModel = ShadeModel;
        /* Set hash key for current program. */
        Context->hashKey.hashShadingModel  = 1;
        gcmVERIFY_OK(gco3D_SetShading(Context->hw, gcvSHADING_FLAT_OPENGL));
        break;

    case GL_SMOOTH:
        /* Set smooth shading. */
        Context->lightingStates.shadeModel = ShadeModel;
        Context->hashKey.hashShadingModel  = 0;
        gcmVERIFY_OK(gco3D_SetShading(Context->hw, gcvSHADING_SMOOTH));
        break;

    default:
        result = GL_INVALID_ENUM;
    }

    gcmFOOTER_ARG("return=%d", result);

    return result;
}

static GLenum _SetLightModelAmbient(
    glsCONTEXT_PTR Context,
    const GLfloat* Value
    )
{
    gcmHEADER_ARG("Context=0x%x Value=0x%x", Context, Value);

    glfSetVector4(
        &Context->lightingStates.Acs,
        Value
        );

    Context->hashKey.hashZeroAcs = Context->lightingStates.Acs.zero3;

    /* Set uAcs dirty. */
    Context->vsUniformDirty.uAcsDirty = gcvTRUE;

    gcmFOOTER_ARG("return=%d", GL_NO_ERROR);

    return GL_NO_ERROR;
}

static GLenum _SetTwoSidedLighting(
    glsCONTEXT_PTR Context,
    const GLfloat* Value
    )
{
    gcmHEADER_ARG("Context=0x%x Value=0x%x", Context, Value);

    Context->lightingStates.twoSidedLighting =
        *Value
            ? GL_TRUE
            : GL_FALSE;

    gcmFOOTER_ARG("return=%d", GL_NO_ERROR);

    return GL_NO_ERROR;
}

static GLenum _SetLightAmbient(
    glsCONTEXT_PTR Context,
    GLint Light,
    const GLfloat* Value
    )
{
    gcmHEADER_ARG("Context=0x%x Light=%d Value=0x%x", Context, Light, Value);

    glfSetVector4(
        &Context->lightingStates.Acli[Light],
        Value
        );

    glmSETHASH_1BIT(
        hashZeroAcl,
        Context->lightingStates.Acli[Light].zero3,
        Light
        );

    /* Set uAcli and uAcmAcli dirty. */
    Context->vsUniformDirty.uAcliDirty    = gcvTRUE;
    Context->vsUniformDirty.uAcmAcliDirty = gcvTRUE;

    gcmFOOTER_ARG("return=%d", GL_NO_ERROR);
    return GL_NO_ERROR;
}

static GLenum _SetLightDiffuse(
    glsCONTEXT_PTR Context,
    GLint Light,
    const GLfloat* Value
    )
{
    gcmHEADER_ARG("Context=0x%x Light=%d Value=0x%x", Context, Light, Value);
    glfSetVector4(
        &Context->lightingStates.Dcli[Light],
        Value
        );

    glmSETHASH_1BIT(
        hashZeroDcl,
        Context->lightingStates.Dcli[Light].zero3,
        Light
        );

    /* Set uDcli and uDcmDcli dirty. */
    Context->vsUniformDirty.uDcliDirty    = gcvTRUE;
    Context->vsUniformDirty.uDcmDcliDirty = gcvTRUE;

    gcmFOOTER_ARG("return=%d", GL_NO_ERROR);
    return GL_NO_ERROR;
}

static GLenum _SetLightSpecular(
    glsCONTEXT_PTR Context,
    GLint Light,
    const GLfloat* Value
    )
{
    gcmHEADER_ARG("Context=0x%x Light=%d Value=0x%x", Context, Light, Value);

    glfSetVector4(
        &Context->lightingStates.Scli[Light],
        Value
        );

    glmSETHASH_1BIT(
        hashZeroScl,
        Context->lightingStates.Scli[Light].zero3,
        Light
        );

    /* Set uScli dirty. */
    Context->vsUniformDirty.uScliDirty = gcvTRUE;

    gcmFOOTER_ARG("return=%d", GL_NO_ERROR);
    return GL_NO_ERROR;
}

static GLenum _SetLightPosition(
    glsCONTEXT_PTR Context,
    GLint Light,
    const GLfloat* Value
    )
{
    glsVECTOR Ppli;
    gcmHEADER_ARG("Context=0x%x Light=%d Value=0x%x", Context, Light, Value);

    /*
        GLES1.1 spec specifically says that the light position is to be
        transformed to eye space at the time when it is specified by
        the current model view matrix.
    */

    /* Normalize positional light by dividing by the W component. */
    glfSetHomogeneousVector4(
        &Ppli, Value
        );

    /* Transfrom the current light position to the eye space. */
    glfMultiplyVector4ByMatrix4x4(
        &Ppli, Context->modelViewMatrix, &Context->lightingStates.Ppli[Light]
        );

    /* Set directional light flag. */
    Context->lightingStates.Directional[Light]
        = (Context->lightingStates.Ppli[Light].value[3] == glvFLOATZERO);

    /* Update hash key. */
    glmSETHASH_1BIT(
        hashDirectionalLight,
        Context->lightingStates.Directional[Light],
        Light
        );

    /* Set uPpli and uVPpli dirty. */
    Context->vsUniformDirty.uPpliDirty  = gcvTRUE;
    Context->vsUniformDirty.uVPpliDirty = gcvTRUE;

    gcmFOOTER_ARG("return=%d", GL_NO_ERROR);
    return GL_NO_ERROR;
}

static GLenum _SetLightSpotDirection(
    glsCONTEXT_PTR Context,
    GLint Light,
    const GLfloat* Value
    )
{
    /* Make a shortcut to the light position vector. */
    glsVECTOR_PTR Sdli = &Context->lightingStates.Sdli[Light];

    gcmHEADER_ARG("Context=0x%x Light=%d Value=0x%x", Context, Light, Value);

    /* Set spot light direction. */
    glfSetVector3(Sdli, Value);

    /* Transform direction of spotlight for the current
       light (Sdli) to the eye space. */
    glfMultiplyVector3ByMatrix3x3(Sdli, Context->modelViewMatrix, Sdli);

    /* Set uNormedSdli dirty. */
    Context->vsUniformDirty.uNormedSdliDirty = gcvTRUE;

    gcmFOOTER_ARG("return=%d", GL_NO_ERROR);

    return GL_NO_ERROR;
}

static GLenum _SetLightSpotExponent(
    glsCONTEXT_PTR Context,
    GLint Light,
    const GLfloat* Value
    )
{
    GLenum result;
    GLfloat Srli = *Value;

    gcmHEADER_ARG("Context=0x%x Light=%d Value=0x%x", Context, Light, Value);

    if ((Srli < glvFLOATZERO) || (Srli > glvFLOAT128))
    {
        result = GL_INVALID_VALUE;
    }
    else
    {
        Context->lightingStates.Srli[Light] = *Value;

        /* Set uSrli dirty. */
        Context->vsUniformDirty.uSrliDirty = gcvTRUE;

        result = GL_NO_ERROR;
    }

    gcmFOOTER_ARG("return=%d", result);
    return result;
}

static GLenum _SetLightSpotCutoff(
    glsCONTEXT_PTR Context,
    GLint Light,
    const GLfloat* Value
    )
{
    GLenum result;

    GLfloat Crli = *Value;

    gcmHEADER_ARG("Context=0x%x Light=%d Value=0x%x", Context, Light, Value);

    if (((Crli < glvFLOATZERO) || (Crli > glvFLOAT90)) && (Crli != glvFLOAT180))
    {
        gcmFOOTER_ARG("return=%d", GL_INVALID_VALUE);
        result = GL_INVALID_VALUE;
    }
    else
    {
        GLboolean is180 = (Crli == glvFLOAT180);
        GLfloat   flag  = is180 ? 1.0f : 0.0f;

        Context->lightingStates.Crli[Light] = *Value;
        Context->lightingStates.uCrli180[Light] = flag;

        Context->lightingStates.RadCrli[Light] =
            Context->lightingStates.Crli[Light] * glvFLOATPIOVER180;

        Context->lightingStates.Crli180[Light] = is180;

        glmSETHASH_1BIT(
            hashCrl_180,
            is180,
            Light
            );

        /* Set uCosCrli dirty. */
        Context->vsUniformDirty.uCosCrliDirty = gcvTRUE;
        Context->vsUniformDirty.uCrli180Dirty = gcvTRUE;

        result = GL_NO_ERROR;
    }

    gcmFOOTER_ARG("return=%d", result);
    return result;
}

static GLenum _SetLightConstantAttenuation(
    glsCONTEXT_PTR Context,
    GLint Light,
    const GLfloat* Value
    )
{
    gcmHEADER_ARG("Context=0x%x Light=%d Value=0x%x", Context, Light, Value);

    if(*Value < glvFLOATZERO)

    {

        gcmFOOTER_ARG("return=%d", GL_INVALID_VALUE);

        return GL_INVALID_VALUE;

    }

    Context->lightingStates.K0i[Light] = *Value;

    glmSETHASH_1BIT(
        hashOneK0,
        glfISONE(Context->lightingStates.K0i[Light]),
        Light
        );

    /* Set uKi dirty. */
    Context->vsUniformDirty.uKiDirty = gcvTRUE;

    gcmFOOTER_ARG("return=%d", GL_NO_ERROR);
    return GL_NO_ERROR;
}

static GLenum _SetLightLinearAttenuation(
    glsCONTEXT_PTR Context,
    GLint Light,
    const GLfloat* Value
    )
{
    gcmHEADER_ARG("Context=0x%x Light=%d Value=0x%x", Context, Light, Value);

    if(*Value < glvFLOATZERO)

    {

        gcmFOOTER_ARG("return=%d", GL_INVALID_VALUE);

        return GL_INVALID_VALUE;

    }

    Context->lightingStates.K1i[Light] = *Value;

    glmSETHASH_1BIT(
        hashZeroK1,
        glfISZERO(Context->lightingStates.K1i[Light]),
        Light
        );

    /* Set uKi dirty. */
    Context->vsUniformDirty.uKiDirty = gcvTRUE;

    gcmFOOTER_ARG("return=%d", GL_NO_ERROR);
    return GL_NO_ERROR;
}

static GLenum _SetLightQuadraticAttenuation(
    glsCONTEXT_PTR Context,
    GLint Light,
    const GLfloat* Value
    )
{
    gcmHEADER_ARG("Context=0x%x Light=%d Value=0x%x", Context, Light, Value);

    if(*Value < glvFLOATZERO)

    {

        gcmFOOTER_ARG("return=%d", GL_INVALID_VALUE);

        return GL_INVALID_VALUE;

    }

    Context->lightingStates.K2i[Light] = *Value;

    glmSETHASH_1BIT(
        hashZeroK2,
        glfISZERO(Context->lightingStates.K2i[Light]),
        Light
        );

    /* Set uKi dirty. */
    Context->vsUniformDirty.uKiDirty = gcvTRUE;

    gcmFOOTER_ARG("return=%d", GL_NO_ERROR);
    return GL_NO_ERROR;
}

static GLenum _SetAmbient(
    glsCONTEXT_PTR Context,
    const GLfloat* Value
    )
{
    gcmHEADER_ARG("Context=0x%x Value=0x%x", Context, Value);

    glfSetVector4(
        &Context->lightingStates.Acm,
        Value
        );

    Context->hashKey.hashZeroAcm = Context->lightingStates.Acm.zero3;

    /* Set uAcm and uAcmAcli dirty. */
    Context->vsUniformDirty.uAcmDirty     = gcvTRUE;
    Context->vsUniformDirty.uAcmAcliDirty = gcvTRUE;

    gcmFOOTER_ARG("return=%d", GL_NO_ERROR);
    return GL_NO_ERROR;
}

static GLenum _SetDiffuse(
    glsCONTEXT_PTR Context,
    const GLfloat* Value
    )
{
    gcmHEADER_ARG("Context=0x%x Value=0x%x", Context, Value);

    glfSetVector4(
        &Context->lightingStates.Dcm,
        Value
        );

    Context->hashKey.hashZeroDcm = Context->lightingStates.Dcm.zero3;

    /* Set uDcm and uDcmDcli dirty. */
    Context->vsUniformDirty.uDcmDirty     = gcvTRUE;
    Context->vsUniformDirty.uDcmDcliDirty = gcvTRUE;

    gcmFOOTER_ARG("return=%d", GL_NO_ERROR);

    return GL_NO_ERROR;
}

static GLenum _SetAmbientAndDiffuse(
    glsCONTEXT_PTR Context,
    const GLvoid* Value
    )
{
    GLenum result;

    gcmHEADER_ARG("Context=0x%x Value=0x%x", Context, Value);

    result = _SetAmbient(Context, Value);
    if (result == GL_NO_ERROR)
    {
        result = _SetDiffuse(Context, Value);
    }

    gcmFOOTER_ARG("return=%d", result);
    return result;
}

static GLenum _SetSpecular(
    glsCONTEXT_PTR Context,
    const GLfloat* Value
    )
{
    gcmHEADER_ARG("Context=0x%x Value=0x%x", Context, Value);

    glfSetVector4(
        &Context->lightingStates.Scm,
        Value
        );

    Context->hashKey.hashZeroScm = Context->lightingStates.Scm.zero3;

    /* Set uScm dirty. */
    Context->vsUniformDirty.uScmDirty = gcvTRUE;

    gcmFOOTER_ARG("return=%d", GL_NO_ERROR);
    return GL_NO_ERROR;
}

static GLenum _SetEmission(
    glsCONTEXT_PTR Context,
    const GLfloat* Value
    )
{
    gcmHEADER_ARG("Context=0x%x Value=0x%x", Context, Value);

    glfSetVector4(
        &Context->lightingStates.Ecm,
        Value
        );

    Context->hashKey.hashZeroEcm = Context->lightingStates.Ecm.zero3;

    /* Set uEcm dirty. */
    Context->vsUniformDirty.uEcmDirty = gcvTRUE;

    gcmFOOTER_ARG("return=%d", GL_NO_ERROR);
    return GL_NO_ERROR;
}

static GLenum _SetShininess(
    glsCONTEXT_PTR Context,
    const GLfloat* Value
    )
{
    GLenum result;
    GLfloat Srm = *Value;

    gcmHEADER_ARG("Context=0x%x Value=0x%x", Context, Value);

    if ((Srm < glvFLOATZERO) || (Srm > glvFLOAT128))
    {
        result = GL_INVALID_VALUE;
    }
    else
    {
        Context->lightingStates.Srm = *Value;

        Context->hashKey.hashZeroSrm = glfISZERO(Context->lightingStates.Srm);

        /* Set uSrm dirty. */
        Context->vsUniformDirty.uSrmDirty = gcvTRUE;

        result = GL_NO_ERROR;
    }

    gcmFOOTER_ARG("return=%d", result);
    return result;
}


/******************************************************************************\
*********************** Support Functions and Definitions **********************
\******************************************************************************/

/*******************************************************************************
**
**  _SetMaterial
**
**  _SetMaterial assigns values to material parameters. There are two matched
**  sets of material parameters. One, the front-facing set, is used to shade
**  points, lines, and all polygons (when two-sided lighting is disabled), or
**  just front-facing polygons (when two-sided lighting is enabled). The other
**  set, back-facing, is used to shade back-facing polygons only when two-sided
**  lighting is enabled. Refer to the glLightModel reference page for details
**  concerning one- and two-sided lighting calculations.
**
**  _SetMaterial takes three arguments. The first, face, must be
**  GL_FRONT_AND_BACK and specifies that both front and back materials will be
**  modified. The second, Name, specifies which of several parameters in one or
**  both sets will be modified. The third, Value, specifies what value or values
**  will be assigned to the specified parameter.
**
**  Material parameters are used in the lighting equation that is optionally
**  applied to each vertex. The equation is discussed in the glLightModel
**  reference page.
**
**  INPUT:
**
**      Context
**          Pointer to the current context object.
**
**      Face
**          Specifies which face or faces are being updated.
**          Must be GL_FRONT_AND_BACK.
**
**      Name
**          Specifies the material parameter of the face or faces that is being
**          updated. Must be one of GL_AMBIENT, GL_DIFFUSE, GL_SPECULAR,
**          GL_EMISSION, GL_SHININESS, or GL_AMBIENT_AND_DIFFUSE.
**
**      Value
**          Specifies a pointer to the value or values that Name will be set to.
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

static GLenum _SetMaterial(
    glsCONTEXT_PTR Context,
    GLenum Face,
    GLenum Name,
    const GLfloat* Value,
    gctUINT32 ValueArraySize
    )
{
    GLenum result = GL_INVALID_ENUM;

    gcmHEADER_ARG("Context=0x%x Face=0x%x Name=0x%x Value=0x%x "
                  "ValueArraySize=%d",
                  Context, Face, Name, Value, ValueArraySize);

    /* Verify the face. */
    if (Face != GL_FRONT_AND_BACK)
    {
        gcmFOOTER_ARG("return=%d", result);
        return result;
    }

    if (ValueArraySize > 1)
    {
        switch (Name)
        {
        case GL_AMBIENT:
            result = _SetAmbient(Context, Value);
            gcmFOOTER_ARG("%d", result);
            return result;
            break;

        case GL_DIFFUSE:
            result = _SetDiffuse(Context, Value);
            gcmFOOTER_ARG("%d", result);
            return result;
            break;

        case GL_AMBIENT_AND_DIFFUSE:
            result = _SetAmbientAndDiffuse(Context, Value);
            gcmFOOTER_ARG("%d", result);
            return result;
            break;

        case GL_SPECULAR:
            result = _SetSpecular(Context, Value);
            gcmFOOTER_ARG("%d", result);
            return result;
            break;

        case GL_EMISSION:
            result = _SetEmission(Context, Value);
            gcmFOOTER_ARG("%d", result);
            return result;
            break;
        }
    }

    /* Dispatch based on the name. */
    switch (Name)
    {
    case GL_SHININESS:
        result = _SetShininess(Context, Value);
        break;
    default:
        result = GL_INVALID_ENUM;
        break;
    }

    /* Return result. */
    gcmFOOTER_ARG("%d", result);
    return result;
}


/*******************************************************************************
**
**  _GetMaterial
**
**  _GetMaterial returns in Value the value or values of parameter Name
**  of material Face.
**
**  INPUT:
**
**      Context
**          Pointer to the current context object.
**
**      Face
**          Specifies which face or faces are being updated.
**          Must be GL_FRONT_AND_BACK.
**
**      Name
**          Specifies the material parameter of the face or faces that is being
**          updated. Must be one of GL_AMBIENT, GL_DIFFUSE, GL_SPECULAR,
**          GL_EMISSION, GL_SHININESS, or GL_AMBIENT_AND_DIFFUSE.
**
**      Type
**          Specifies the type of Value.
**
**  OUTPUT:
**
**      Value
**          Specifies a pointer to the value to be returned.
*/

static GLenum _GetMaterial(
    glsCONTEXT_PTR Context,
    GLenum Face,
    GLenum Name,
    GLvoid* Value,
    gleTYPE Type
    )
{
    GLenum result;
    gcmHEADER_ARG("Context=0x%x Face=0x%04x Name=0x%04x Value=0x%x Type=0x%04x",
                    Context, Face, Name, Value, Type);

    /* Verify the face. */
    if ((Face != GL_FRONT) && (Face != GL_BACK))
    {
        gcmFOOTER_ARG("return=%d", GL_INVALID_ENUM);
        return GL_INVALID_ENUM;
    }

    /* Assume no error. */
    result = GL_NO_ERROR;

    /* Dispatch based on the name. */
    switch (Name)
    {
    case GL_AMBIENT:
        if (Context->lightingStates.materialEnabled)
            glfGetFromVector4(&Context->aColorInfo.currValue, Value, Type);
        else
            glfGetFromVector4(&Context->lightingStates.Acm, Value, Type);
        break;

    case GL_DIFFUSE:
        if (Context->lightingStates.materialEnabled)
            glfGetFromVector4(&Context->aColorInfo.currValue, Value, Type);
        else
            glfGetFromVector4(&Context->lightingStates.Dcm, Value, Type);
        break;

    case GL_SPECULAR:
        glfGetFromVector4(&Context->lightingStates.Scm, Value, Type);
        break;

    case GL_EMISSION:
        glfGetFromVector4(&Context->lightingStates.Ecm, Value, Type);
        break;

    case GL_SHININESS:
        glfGetFromFloat(Context->lightingStates.Srm, Value, Type);
        break;

    default:
        result = GL_INVALID_ENUM;
    }

    gcmFOOTER_ARG("return=%d", result);

    /* Return result. */
    return result;
}


/*******************************************************************************
**
**  _SetLight
**
**  _SetLight sets the values of individual light source parameters.
**  Light names the light and is a symbolic name of the form GL_LIGHTi, where
**  0 <= i < GL_MAX_LIGHTS. Name specifies one of ten light source parameters,
**  again by symbolic name. Value is either a single value or a pointer to an
**  array that contains the new values.
**
**  To enable and disable lighting calculation, call glEnable and glDisable
**  with argument GL_LIGHTING. Lighting is initially disabled. When it is
**  enabled, light sources that are enabled contribute to the lighting
**  calculation. Light source i is enabled and disabled using glEnable
**  and glDisable with argument GL_LIGHTi.
**
**  INPUT:
**
**      Context
**          Pointer to the current context object.
**
**      Light
**          Specifies a light. The number of lights depends on the
**          implementation, but at least eight lights are supported.
**          They are identified by symbolic names of the form GL_LIGHTi
**          where 0 <= i < GL_MAX_LIGHTS.
**
**      Name
**          Specifies a light source parameter for light. GL_AMBIENT,
**          GL_DIFFUSE, GL_SPECULAR, GL_POSITION, GL_SPOT_CUTOFF,
**          GL_SPOT_DIRECTION, GL_SPOT_EXPONENT, GL_CONSTANT_ATTENUATION,
**          GL_LINEAR_ATTENUATION, and GL_QUADRATIC_ATTENUATION are accepted.
**
**      Value
**          Specifies a pointer to the value or values that parameter Name
**          of light source Light will be set to.
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

static GLenum _SetLight(
    glsCONTEXT_PTR Context,
    GLenum Light,
    GLenum Name,
    const GLfloat* Value,
    gctUINT32 ValueArraySize
    )
{
    /* Determine the light. */
    GLint light = Light - GL_LIGHT0;

    GLenum status;

    gcmHEADER_ARG("Context=0x%x Light=0x%x Name=0x%x Value=0x%x "
                  "ValueArraySize=%d",
                  Context, Light, Name, Value, ValueArraySize);

    if ((light < 0) || (light >= glvMAX_LIGHTS))
    {
        /* Invalid light. */
        gcmFOOTER_ARG("status=%d", GL_INVALID_ENUM);
        return GL_INVALID_ENUM;
    }

    if (ValueArraySize > 1)
    {
        switch (Name)
        {
        case GL_AMBIENT:
            status = _SetLightAmbient(Context, light, Value);
            gcmFOOTER_ARG("status=%d", status);
            return status;

        case GL_DIFFUSE:
            status = _SetLightDiffuse(Context, light, Value);
            gcmFOOTER_ARG("status=%d", status);
            return status;

        case GL_SPECULAR:
            status = _SetLightSpecular(Context, light, Value);
            gcmFOOTER_ARG("status=%d", status);
            return status;

        case GL_POSITION:
            status = _SetLightPosition(Context, light, Value);
            gcmFOOTER_ARG("status=%d", status);
            return status;

        case GL_SPOT_DIRECTION:
            status = _SetLightSpotDirection(Context, light, Value);
            gcmFOOTER_ARG("status=%d", status);
            return status;
        }

    }

    switch (Name)
    {
    case GL_SPOT_EXPONENT:
        status = _SetLightSpotExponent(Context, light, Value);
        gcmFOOTER_ARG("status=%d", status);
        return status;

    case GL_SPOT_CUTOFF:
        status = _SetLightSpotCutoff(Context, light, Value);
        gcmFOOTER_ARG("status=%d", status);
        return status;

    case GL_CONSTANT_ATTENUATION:
        status = _SetLightConstantAttenuation(Context, light, Value);
        gcmFOOTER_ARG("status=%d", status);
        return status;

    case GL_LINEAR_ATTENUATION:
        status = _SetLightLinearAttenuation(Context, light, Value);
        gcmFOOTER_ARG("status=%d", status);
        return status;

    case GL_QUADRATIC_ATTENUATION:
        status = _SetLightQuadraticAttenuation(Context, light, Value);
        gcmFOOTER_ARG("status=%d", status);
        return status;
    }

    gcmFOOTER_ARG("status=%d", GL_INVALID_ENUM);
    return GL_INVALID_ENUM;
}


/*******************************************************************************
**
**  _GetLight
**
**  glGetLight returns in params the value or values of a light source
**  parameter. light names the light and is a symbolic name of the form
**  GL_LIGHTi for 0 <= i < GL_MAX_LIGHTS where GL_MAX_LIGHTS is an
**  implementation dependent constant that is greater than or equal to eight.
**  Name specifies one of ten light source parameters, again by symbolic name.
**
**  INPUT:
**
**      Context
**          Pointer to the current context object.
**
**      Light
**          Specifies a light source. The number of possible lights depends on
**          the implementation, but at least eight lights are supported.
**          They are identified by symbolic names of the form GL_LIGHTi
**          where 0 <= i < GL_MAX_LIGHTS.
**
**      Name
**          Specifies a light source parameter for light. Accepted symbolic
**          names are GL_AMBIENT, GL_DIFFUSE, GL_SPECULAR, GL_EMISSION,
**          GL_SPOT_DIRECTION, GL_SPOT_EXPONENT, GL_SPOT_CUTOFF,
**          GL_CONSTANT_ATTENUATION, GL_LINEAR_ATTENUATION,
**          and GL_QUADRATIC_ATTENUATION.
**
**      Type
**          Specifies the type of Value.
**
**  OUTPUT:
**
**      Value
**          Returns the requested data.
*/

static GLenum _GetLight(
    glsCONTEXT_PTR Context,
    GLenum Light,
    GLenum Name,
    GLvoid* Value,
    gleTYPE Type
    )
{
    GLenum result;
    GLint light;

    gcmHEADER_ARG("Context=0x%x Light=0x%04x Name=0x%04x Value=0x%x Type=0x%04x",
                    Context, Light, Name, Value, Type);

    /* Determine the light. */
    light = Light - GL_LIGHT0;

    if ((light < 0) || (light >= glvMAX_LIGHTS))
    {
        gcmFOOTER_ARG("result=%d", GL_INVALID_ENUM);
        /* Invalid light. */
        return GL_INVALID_ENUM;
    }

    /* Assume no error. */
    result = GL_NO_ERROR;

    /* Dispatch based on the name. */
    switch (Name)
    {
    case GL_AMBIENT:
        glfGetFromVector4(&Context->lightingStates.Acli[light], Value, Type);
        break;

    case GL_DIFFUSE:
        glfGetFromVector4(&Context->lightingStates.Dcli[light], Value, Type);
        break;

    case GL_SPECULAR:
        glfGetFromVector4(&Context->lightingStates.Scli[light], Value, Type);
        break;

    case GL_POSITION:
        glfGetFromVector4(&Context->lightingStates.Ppli[light], Value, Type);
        break;

    case GL_SPOT_DIRECTION:
        glfGetFromVector3(&Context->lightingStates.Sdli[light], Value, Type);
        break;

    case GL_SPOT_EXPONENT:
        glfGetFromFloat(Context->lightingStates.Srli[light], Value, Type);
        break;

    case GL_SPOT_CUTOFF:
        glfGetFromFloat(Context->lightingStates.Crli[light], Value, Type);
        break;

    case GL_CONSTANT_ATTENUATION:
        glfGetFromFloat(Context->lightingStates.K0i[light], Value, Type);
        break;

    case GL_LINEAR_ATTENUATION:
        glfGetFromFloat(Context->lightingStates.K1i[light], Value, Type);
        break;

    case GL_QUADRATIC_ATTENUATION:
        glfGetFromFloat(Context->lightingStates.K2i[light], Value, Type);
        break;

    default:
        result = GL_INVALID_ENUM;
    }

    gcmFOOTER_ARG("result=%d", result);
    /* Return result. */
    return result;
}


/*******************************************************************************
**
**  _SetLightModel
**
**  _SetLightModel sets the lighting model parameter. Name names a parameter
**  and Value gives the new value. There are two lighting model parameters:
**
**      GL_LIGHT_MODEL_AMBIENT
**          Value contains four fixed-point or floating-point values that
**          specify the ambient intensity of the entire scene. The values are
**          not clamped. The initial value is (0.2, 0.2, 0.2, 1.0).
**
**      GL_LIGHT_MODEL_TWO_SIDE
**          Value is a single fixed-point or floating-point value that specifies
**          whether one- or two-sided lighting calculations are done for
**          polygons. It has no effect on the lighting calculations for points,
**          lines, or bitmaps. If params is 0, one-sided lighting is specified,
**          and only the front material parameters are used in the lighting
**          equation. Otherwise, two-sided lighting is specified. In this case,
**          vertices of back-facing polygons are lighted using the back material
**          parameters, and have their normals reversed before the lighting
**          equation is evaluated. Vertices of front-facing polygons are always
**          lighted using the front material parameters, with no change to their
**          normals. The initial value is 0.
**
**  The lighted color of a vertex is the sum of the material emission intensity,
**  the product of the material ambient reflectance and the lighting model
**  full-scene ambient intensity, and the contribution of each enabled light
**  source. Each light source contributes the sum of three terms: ambient,
**  diffuse, and specular. The ambient light source contribution is the product
**  of the material ambient reflectance and the light's ambient intensity. The
**  diffuse light source contribution is the product of the material diffuse
**  reflectance, the light's diffuse intensity, and the dot product of the
**  vertex's normal with the normalized vector from the vertex to the light
**  source. The specular light source contribution is the product of the
**  material specular reflectance, the light's specular intensity, and the dot
**  product of the normalized vertex-to-eye and vertex-to-light vectors, raised
**  to the power of the Srm of the material. All three light source
**  contributions are attenuated equally based on the distance from the vertex
**  to the light source and on light source direction, spread exponent, and
**  spread cutoff angle. All dot products are replaced with 0 if they evaluate
**  to a negative value.
**
**  The alpha component of the resulting lighted color is set to the alpha value
**  of the material diffuse reflectance.
**
**  INPUT:
**
**      Context
**          Pointer to the current context object.
**
**      Name
**          Specifies a lighting model parameter. GL_LIGHT_MODEL_AMBIENT and
**          GL_LIGHT_MODEL_TWO_SIDE are accepted.
**
**      Value
**          Specifies a pointer to the value to be set.
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

static GLenum _SetLightModel(
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
        case GL_LIGHT_MODEL_AMBIENT:
            result = _SetLightModelAmbient(Context, Value);
            gcmFOOTER_ARG("result=%d", result);
            return result;
        }
    }

    /* Dispatch based on the name. */
    switch (Name)
    {
    case GL_LIGHT_MODEL_TWO_SIDE:
        result = _SetTwoSidedLighting(Context, Value);
        break;
    default:
        result = GL_INVALID_ENUM;
    }

    gcmFOOTER_ARG("result=%d", result);
    /* Return result. */
    return result;
}


/*******************************************************************************
**
**  glfSetDefaultLightingStates
**
**  Set default lighting states.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**  OUTPUT:
**
**      Nothing.
**
*/

gceSTATUS glfSetDefaultLightingStates(
    glsCONTEXT_PTR Context
    )
{
    GLenum result;

    gcmHEADER_ARG("Context=0x%x", Context);

    do
    {
        GLint i;

        static GLfloat value0 = glvFLOATZERO;

        static GLfloat value1 = glvFLOATONE;

        static GLfloat value180 = glvFLOAT180;

        static GLfloat vecP2P2P21[] =
            { glvFLOATPOINT2, glvFLOATPOINT2, glvFLOATPOINT2, glvFLOATONE };

        static GLfloat vecP8P8P81[] =
            { glvFLOATPOINT8, glvFLOATPOINT8, glvFLOATPOINT8, glvFLOATONE };

        static GLfloat vec0001[] =
            { glvFLOATZERO, glvFLOATZERO, glvFLOATZERO, glvFLOATONE };

        static GLfloat vec1111[] =
            { glvFLOATONE, glvFLOATONE, glvFLOATONE, glvFLOATONE };

        static GLfloat vec0010[] =
            { glvFLOATZERO, glvFLOATZERO, glvFLOATONE, glvFLOATZERO };

        static GLfloat vec00n10[] =
            { glvFLOATZERO, glvFLOATZERO, glvFLOATNEGONE, glvFLOATZERO };

        /* Default shading mode. */
        glmERR_BREAK(_SetShadeModel(
            Context, GL_SMOOTH
            ));

        /* Two-sided lighting. */
        glmERR_BREAK(_SetTwoSidedLighting(
            Context, &value0
            ));

        /* Ambient color of scene (Acs). */
        glmERR_BREAK(_SetLightModelAmbient(
            Context, vecP2P2P21
            ));

        /* Ambient color of material (Acm). */
        glmERR_BREAK(_SetAmbient(
            Context, vecP2P2P21
            ));

        /* Diffuse color of material (Dcm). */
        glmERR_BREAK(_SetDiffuse(
            Context, vecP8P8P81
            ));

        /* Specular color (Scm). */
        glmERR_BREAK(_SetSpecular(
            Context, vec0001
            ));

        /* Emissive color (Ecm). */
        glmERR_BREAK(_SetEmission(
            Context, vec0001
            ));

        /* Specular exponent (Srm). */
        glmERR_BREAK(_SetShininess(
            Context, &value0
            ));

        for (i = 0; i < glvMAX_LIGHTS; i++)
        {
            /* Ambient intensity (Acli). */
            glmERR_BREAK(_SetLightAmbient(
                Context, i, vec0001
                ));

            if (i == 0)
            {
                /* Diffuse intensity (Dcli). */
                glmERR_BREAK(_SetLightDiffuse(
                    Context, i, vec1111
                    ));

                /* Specular intensity (Scli). */
                glmERR_BREAK(_SetLightSpecular(
                    Context, i, vec1111
                    ));
            }
            else
            {
                /* Diffuse intensity (Dcli). */
                glmERR_BREAK(_SetLightDiffuse(
                    Context, i, vec0001
                    ));

                /* Specular intensity (Scli). */
                glmERR_BREAK(_SetLightSpecular(
                    Context, i, vec0001
                    ));
            }

            /* Position (Ppli). */
            glmERR_BREAK(_SetLightPosition(
                Context, i, vec0010
                ));

            /* Direction of spotlight (Sdli). */
            glmERR_BREAK(_SetLightSpotDirection(
                Context, i, vec00n10
                ));

            /* Spotlight exponent (Srli). */
            glmERR_BREAK(_SetLightSpotExponent(
                Context, i, &value0
                ));

            /* Spotlight cutoff angle (Crli). */
            glmERR_BREAK(_SetLightSpotCutoff(
                Context, i, &value180
                ));

            /* Attenualtion factors (K0i, K1i, K2i). */
            glmERR_BREAK(_SetLightConstantAttenuation(
                Context, i, &value1
                ));

            glmERR_BREAK(_SetLightLinearAttenuation(
                Context, i, &value0
                ));

            glmERR_BREAK(_SetLightQuadraticAttenuation(
                Context, i, &value0
                ));
        }
    }
    while (gcvFALSE);

    gcmFOOTER_ARG("result=%d", (result == GL_NO_ERROR) ? gcvSTATUS_OK : gcvSTATUS_INVALID_ARGUMENT);

    return (result == GL_NO_ERROR)
        ? gcvSTATUS_OK
        : gcvSTATUS_INVALID_ARGUMENT;
}


/*******************************************************************************
**
**  glfFlushLightingStates
**
**  Flush lighting states.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**  OUTPUT:
**
**      Nothing.
**
*/

gceSTATUS glfFlushLightingStates(
    glsCONTEXT_PTR Context
    )
{
    GLenum result;

    gcmHEADER_ARG("Context=0x%x", Context);

    do
    {
        /* Flush shading mode. */
        glmERR_BREAK(_SetShadeModel(
            Context, Context->lightingStates.shadeModel
            ));

        /* Set dirty flag for Ambient color of scene (Acs). */
        Context->vsUniformDirty.uAcsDirty = gcvTRUE;

        /* Set dirty flag for Ambient color of material (Acm). */
        Context->vsUniformDirty.uAcmDirty     = gcvTRUE;
        Context->vsUniformDirty.uAcmAcliDirty = gcvTRUE;

        /* Set dirty flag for Diffuse color of material (Dcm). */
        Context->vsUniformDirty.uDcmDirty     = gcvTRUE;
        Context->vsUniformDirty.uDcmDcliDirty = gcvTRUE;

        /* Set dirty flag for Specular color (Scm). */
        Context->vsUniformDirty.uScmDirty = gcvTRUE;

        /* Set dirty flag for Emissive color (Ecm). */
        Context->vsUniformDirty.uEcmDirty = gcvTRUE;

        /* Set dirty flag for Specular exponent (Srm). */
        Context->vsUniformDirty.uSrmDirty = gcvTRUE;

        /* Set dirty flag for Ambient intensity (Acli). */
        Context->vsUniformDirty.uAcliDirty    = gcvTRUE;
        Context->vsUniformDirty.uAcmAcliDirty = gcvTRUE;

        /* Set dirty flag for Diffuse intensity (Dcli). */
        Context->vsUniformDirty.uDcliDirty    = gcvTRUE;
        Context->vsUniformDirty.uDcmDcliDirty = gcvTRUE;

        /* Set dirty flag for Specular intensity (Scli). */
        Context->vsUniformDirty.uScliDirty = gcvTRUE;

        /* Set dirty flag for Diffuse intensity (Dcli). */
        Context->vsUniformDirty.uDcliDirty    = gcvTRUE;
        Context->vsUniformDirty.uDcmDcliDirty = gcvTRUE;

        /* Set dirty flag for Specular intensity (Scli). */
        Context->vsUniformDirty.uScliDirty = gcvTRUE;

        /* Set dirty flag for Position (Ppli). */
        Context->vsUniformDirty.uPpliDirty  = gcvTRUE;
        Context->vsUniformDirty.uVPpliDirty = gcvTRUE;

        /* Set dirty flag for Direction of spotlight (Sdli). */
        Context->vsUniformDirty.uNormedSdliDirty = gcvTRUE;

        /* Set dirty flag for Spotlight exponent (Srli). */
        Context->vsUniformDirty.uSrliDirty = gcvTRUE;

        /* Set dirty flag for Spotlight cutoff angle (Crli). */
        Context->vsUniformDirty.uCosCrliDirty = gcvTRUE;
        Context->vsUniformDirty.uCrli180Dirty = gcvTRUE;

        /* Set dirt flag for Attenualtion factors (K0i, K1i, K2i). */
        Context->vsUniformDirty.uKiDirty = gcvTRUE;
    }
    while (gcvFALSE);

    gcmFOOTER_ARG("result=%d", (result == GL_NO_ERROR) ? gcvSTATUS_OK : gcvSTATUS_INVALID_ARGUMENT);

    return (result == GL_NO_ERROR)
        ? gcvSTATUS_OK
        : gcvSTATUS_INVALID_ARGUMENT;
}


/*******************************************************************************
**
*   glfQueryLightingState
**
**  Queries lighting state values.
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

GLboolean glfQueryLightingState(
    glsCONTEXT_PTR Context,
    GLenum Name,
    GLvoid* Value,
    gleTYPE Type
    )
{
    GLboolean result = GL_TRUE;

    gcmHEADER_ARG("Context=0x%x Name=0x%04x Value=%x Type=0x%04x",
                    Context, Name, Value, Type);

    switch (Name)
    {
    case GL_SHADE_MODEL:
        glfGetFromEnum(
            Context->lightingStates.shadeModel,
            Value,
            Type
            );
        break;

    case GL_COLOR_MATERIAL:
        glfGetFromInt(
            Context->lightingStates.materialEnabled,
            Value,
            Type
            );
        break;

    case GL_LIGHTING:
        glfGetFromInt(
            Context->lightingStates.lightingEnabled,
            Value,
            Type
            );
        break;

    case GL_LIGHT_MODEL_AMBIENT:
        glfGetFromVector4(
            &Context->lightingStates.Acs,
            Value,
            Type
            );
        break;

    case GL_LIGHT_MODEL_TWO_SIDE:
        glfGetFromInt(
            Context->lightingStates.twoSidedLighting,
            Value,
            Type
            );
        break;

    case GL_MAX_LIGHTS:
        glfGetFromInt(
            glvMAX_LIGHTS,
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
************************** Light State Management Code *************************
\******************************************************************************/

/*******************************************************************************
**
**  glLightModel
**
**  glLightModel sets the lighting model parameter. Name names a parameter and
**  Value gives the new value. There are two lighting model parameters:
**
**      GL_LIGHT_MODEL_AMBIENT
**          Value contains four fixed-point or floating-point values that
**          specify the ambient intensity of the entire scene. The values are
**          not clamped. The initial value is (0.2, 0.2, 0.2, 1.0).
**
**      GL_LIGHT_MODEL_TWO_SIDE
**          Value is a single fixed-point or floating-point value that specifies
**          whether one- or two-sided lighting calculations are done for
**          polygons. It has no effect on the lighting calculations for points,
**          lines, or bitmaps. If params is 0, one-sided lighting is specified,
**          and only the front material parameters are used in the lighting
**          equation. Otherwise, two-sided lighting is specified. In this case,
**          vertices of back-facing polygons are lighted using the back material
**          parameters, and have their normals reversed before the lighting
**          equation is evaluated. Vertices of front-facing polygons are always
**          lighted using the front material parameters, with no change to their
**          normals. The initial value is 0.
**
**  The lighted color of a vertex is the sum of the material emission intensity,
**  the product of the material ambient reflectance and the lighting model
**  full-scene ambient intensity, and the contribution of each enabled light
**  source. Each light source contributes the sum of three terms: ambient,
**  diffuse, and specular. The ambient light source contribution is the product
**  of the material ambient reflectance and the light's ambient intensity. The
**  diffuse light source contribution is the product of the material diffuse
**  reflectance, the light's diffuse intensity, and the dot product of the
**  vertex's normal with the normalized vector from the vertex to the light
**  source. The specular light source contribution is the product of the
**  material specular reflectance, the light's specular intensity, and the dot
**  product of the normalized vertex-to-eye and vertex-to-light vectors, raised
**  to the power of the Srm of the material. All three light source
**  contributions are attenuated equally based on the distance from the vertex
**  to the light source and on light source direction, spread exponent, and
**  spread cutoff angle. All dot products are replaced with 0 if they evaluate
**  to a negative value.
**
**  The alpha component of the resulting lighted color is set to the alpha value
**  of the material diffuse reflectance.
**
**  INPUT:
**
**      Name
**          Specifies a lighting model parameter. GL_LIGHT_MODEL_AMBIENT and
**          GL_LIGHT_MODEL_TWO_SIDE are accepted.
**
**      Value
**          Specifies a pointer to the value to be set.
**
**  OUTPUT:
**
**      Nothing.
*/
#ifdef _GC_OBJ_ZONE
#undef _GC_OBJ_ZONE
#endif
#define _GC_OBJ_ZONE    gcdZONE_ES11_LIGHT

GL_API void GL_APIENTRY glLightModelx(
    GLenum Name,
    GLfixed Value
    )
{
    glmENTER2(glmARGENUM, Name, glmARGFIXED, Value)
    {
        GLfloat value;

        gcmDUMP_API("${ES11 glLightModelx 0x%08X 0x%08X}", Name, Value);
        glmPROFILE(context, GLES1_LIGHTMODELX, 0);

        /* Convert to float */
        value = glmFIXED2FLOAT(Value);

        glmERROR(_SetLightModel(context, Name, &value, 1));
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glLightModelxOES(
    GLenum Name,
    GLfixed Value
    )
{
    glmENTER2(glmARGENUM, Name, glmARGFIXED, Value)
    {
        GLfloat value;

        gcmDUMP_API("${ES11 glLightModelxOES 0x%08X 0x%08X}", Name, Value);

        /* Convert to float */
        value = glmFIXED2FLOAT(Value);

        glmERROR(_SetLightModel(context, Name, &value, 1));
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glLightModelxv(
    GLenum Name,
    const GLfixed* Values
    )
{
    glmENTER2(glmARGENUM, Name, glmARGPTR, Values)
    {
        GLfloat values[4];

        gcmDUMP_API("${ES11 glLightModelxv 0x%08X (0x%08X)", Name, Values);
        gcmDUMP_API_ARRAY(Values, 4);
        gcmDUMP_API("$}");

        glmPROFILE(context, GLES1_LIGHTMODELXV, 0);

        /* Convert to float */
        values[0] = glmFIXED2FLOAT(Values[0]);
        values[1] = glmFIXED2FLOAT(Values[1]);
        values[2] = glmFIXED2FLOAT(Values[2]);
        values[3] = glmFIXED2FLOAT(Values[3]);

        glmERROR(_SetLightModel(context, Name, values, 4));
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glLightModelxvOES(
    GLenum Name,
    const GLfixed* Values
    )
{
    glmENTER2(glmARGENUM, Name, glmARGPTR, Values)
    {
        GLfloat values[4];

        gcmDUMP_API("${ES11 glLightModelxvOES 0x%08X (0x%08X)", Name, Values);
        gcmDUMP_API_ARRAY(Values, 4);
        gcmDUMP_API("$}");

        /* Convert to float */
        values[0] = glmFIXED2FLOAT(Values[0]);
        values[1] = glmFIXED2FLOAT(Values[1]);
        values[2] = glmFIXED2FLOAT(Values[2]);
        values[3] = glmFIXED2FLOAT(Values[3]);

        glmERROR(_SetLightModel(context, Name, values, 4));
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glLightModelf(
    GLenum Name,
    GLfloat Value
    )
{
    glmENTER2(glmARGENUM, Name, glmARGFLOAT, Value)
    {
        gcmDUMP_API("${ES11 glLightModelf 0x%08X 0x%08X}", Name, *(GLuint*)&Value);

        glmPROFILE(context, GLES1_LIGHTMODELF, 0);
        glmERROR(_SetLightModel(context, Name, &Value, 1));
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glLightModelfv(
    GLenum Name,
    const GLfloat* Value
    )
{
    glmENTER2(glmARGENUM, Name, glmARGPTR, Value)
    {
        gcmDUMP_API("${ES11 glLightModelfv 0x%08X (0x%08X)", Name, Value);
        gcmDUMP_API_ARRAY(Value, 4);
        gcmDUMP_API("$}");

        glmPROFILE(context, GLES1_LIGHTMODELFV, 0);
        glmERROR(_SetLightModel(context, Name, Value, 4));
    }
    glmLEAVE();
}

/*******************************************************************************
**
**  glLight
**
**  glLight sets the values of individual light source parameters. Light names
**  the light and is a symbolic name of the form GL_LIGHTi, where
**  0 <= i < GL_MAX_LIGHTS. Name specifies one of ten light source parameters,
**  again by symbolic name. Value is either a single value or a pointer to an
**  array that contains the new values.
**
**  To enable and disable lighting calculation, call glEnable and glDisable
**  with argument GL_LIGHTING. Lighting is initially disabled. When it is
**  enabled, light sources that are enabled contribute to the lighting
**  calculation. Light source i is enabled and disabled using glEnable
**  and glDisable with argument GL_LIGHTi.
**
**  INPUT:
**
**      Light
**          Specifies a light. The number of lights depends on the
**          implementation, but at least eight lights are supported.
**          They are identified by symbolic names of the form GL_LIGHTi
**          where 0 <= i < GL_MAX_LIGHTS.
**
**      Name
**          Specifies a light source parameter for light. GL_AMBIENT,
**          GL_DIFFUSE, GL_SPECULAR, GL_POSITION, GL_SPOT_CUTOFF,
**          GL_SPOT_DIRECTION, GL_SPOT_EXPONENT, GL_CONSTANT_ATTENUATION,
**          GL_LINEAR_ATTENUATION, and GL_QUADRATIC_ATTENUATION are accepted.
**
**      Value
**          Specifies a pointer to the value or values that parameter Name
**          of light source Light will be set to.
**
**  OUTPUT:
**
**      Nothing.
*/

GL_API void GL_APIENTRY glLightx(
    GLenum Light,
    GLenum Name,
    GLfixed Value
    )
{
    glmENTER3(glmARGENUM, Light, glmARGENUM, Name, glmARGFIXED, Value)
    {
        GLfloat value;

        gcmDUMP_API("${ES11 glLightx 0x%08X 0x%08X 0x%08X}", Light, Name, Value);
        glmPROFILE(context, GLES1_LIGHTX, 0);

        /* Convert to float */
        value = glmFIXED2FLOAT(Value);

        glmERROR(_SetLight(context, Light, Name, &value, 1));
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glLightxOES(
    GLenum Light,
    GLenum Name,
    GLfixed Value
    )
{
    glmENTER3(glmARGENUM, Light, glmARGENUM, Name, glmARGFIXED, Value)
    {
        GLfloat value;

        gcmDUMP_API("${ES11 glLightxOES 0x%08X 0x%08X 0x%08X}", Light, Name, Value);

        /* Convert to Float */
        value = glmFIXED2FLOAT(Value);
        glmERROR(_SetLight(context, Light, Name, &value, 1));
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glLightxv(
    GLenum Light,
    GLenum Name,
    const GLfixed* Value
    )
{
    glmENTER3(glmARGENUM, Light, glmARGENUM, Name, glmARGPTR, Value)
    {
        GLfloat values[4];

        gcmDUMP_API("${ES11 glLightxv 0x%08X 0x%08X (0x%08X)", Light, Name, Value);
        gcmDUMP_API_ARRAY(Value, 4);
        gcmDUMP_API("$}");

        glmPROFILE(context, GLES1_LIGHTXV, 0);

        /* Convert to Float */
        values[0] = glmFIXED2FLOAT(Value[0]);
        values[1] = glmFIXED2FLOAT(Value[1]);
        values[2] = glmFIXED2FLOAT(Value[2]);
        values[3] = glmFIXED2FLOAT(Value[3]);

        glmERROR(_SetLight(context, Light, Name, values, 4));
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glLightxvOES(
    GLenum Light,
    GLenum Name,
    const GLfixed* Value
    )
{
    glmENTER3(glmARGENUM, Light, glmARGENUM, Name, glmARGPTR, Value)
    {
        GLfloat values[4];

        gcmDUMP_API("${ES11 glLightxvOES 0x%08X 0x%08X (0x%08X)", Light, Name, Value);
        gcmDUMP_API_ARRAY(Value, 4);
        gcmDUMP_API("$}");

        /* Convert to Float */
        values[0] = glmFIXED2FLOAT(Value[0]);
        values[1] = glmFIXED2FLOAT(Value[1]);
        values[2] = glmFIXED2FLOAT(Value[2]);
        values[3] = glmFIXED2FLOAT(Value[3]);

        glmERROR(_SetLight(context, Light, Name, values, 4));
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glLightf(
    GLenum Light,
    GLenum Name,
    GLfloat Value
    )
{
    glmENTER3(glmARGENUM, Light, glmARGENUM, Name, glmARGFLOAT, Value)
    {
        gcmDUMP_API("${ES11 glLightf 0x%08X 0x%08X 0x%08X}", Light, Name, *(GLuint*) &Value);

        glmPROFILE(context, GLES1_LIGHTF, 0);
        glmERROR(_SetLight(context, Light, Name, &Value, 1));
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glLightfv(
    GLenum Light,
    GLenum Name,
    const GLfloat* Value
    )
{
    glmENTER3(glmARGENUM, Light, glmARGENUM, Name, glmARGPTR, Value)
    {
        gcmDUMP_API("${ES11 glLightfv 0x%08X 0x%08X (0x%08X)", Light, Name, Value);
        gcmDUMP_API_ARRAY(Value, 4);
        gcmDUMP_API("$}");

        glmPROFILE(context, GLES1_LIGHTFV, 0);
        glmERROR(_SetLight(context, Light, Name, Value, 4));
    }
    glmLEAVE();
}


/*******************************************************************************
**
**  glGetMaterial
**
**  glGetMaterial returns in Value the value or values of parameter Name
**  of material Face.
**
**  INPUT:
**
**      Face
**          Specifies which face or faces are being updated.
**          Must be GL_FRONT_AND_BACK.
**
**      Name
**          Specifies the material parameter of the face or faces that is being
**          updated. Must be one of GL_AMBIENT, GL_DIFFUSE, GL_SPECULAR,
**          GL_EMISSION, GL_SHININESS, or GL_AMBIENT_AND_DIFFUSE.
**
**  OUTPUT:
**
**      Value
**          Specifies a pointer to the value to be returned.
*/

GL_API void GL_APIENTRY glGetMaterialxv(
    GLenum Face,
    GLenum Name,
    GLfixed* Value
    )
{
    glmENTER3(glmARGENUM, Face, glmARGENUM, Name, glmARGPTR, Value)
    {
        glmPROFILE(context, GLES1_GETMATERIALXV, 0);
        glmERROR(_GetMaterial(context, Face, Name, Value, glvFIXED));

        gcmDUMP_API("${ES11 glGetMaterialxv 0x%08X 0x%08X (0x%08X)", Face, Name, Value);
        gcmDUMP_API_ARRAY(Value, 4);
        gcmDUMP_API("$}");
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glGetMaterialxvOES(
    GLenum Face,
    GLenum Name,
    GLfixed* Value
    )
{
    glmENTER3(glmARGENUM, Face, glmARGENUM, Name, glmARGPTR, Value)
    {
        glmERROR(_GetMaterial(context, Face, Name, Value, glvFIXED));

        gcmDUMP_API("${ES11 glGetMaterialxvOES 0x%08X 0x%08X (0x%08X)", Face, Name, Value);
        gcmDUMP_API_ARRAY(Value, 4);
        gcmDUMP_API("$}");
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glGetMaterialfv(
    GLenum Face,
    GLenum Name,
    GLfloat* Value
    )
{
    glmENTER3(glmARGENUM, Face, glmARGENUM, Name, glmARGPTR, Value)
    {
        glmPROFILE(context, GLES1_GETMATERIALFV, 0);
        glmERROR(_GetMaterial(context, Face, Name, Value, glvFLOAT));

        gcmDUMP_API("${ES11 glGetMaterialfv 0x%08X 0x%08X (0x%08X)", Face, Name, Value);
        gcmDUMP_API_ARRAY(Value, 4);
        gcmDUMP_API("$}");
    }
    glmLEAVE();
}

/*******************************************************************************
**
**  glMaterial
**
**  glMaterial assigns values to material parameters. There are two matched sets
**  of material parameters. One, the front-facing set, is used to shade points,
**  lines, and all polygons (when two-sided lighting is disabled), or just
**  front-facing polygons (when two-sided lighting is enabled). The other set,
**  back-facing, is used to shade back-facing polygons only when two-sided
**  lighting is enabled. Refer to the glLightModel reference page for details
**  concerning one- and two-sided lighting calculations.
**
**  glMaterial takes three arguments. The first, face, must be GL_FRONT_AND_BACK
**  and specifies that both front and back materials will be modified.
**  The second, pname, specifies which of several parameters in one or both sets
**  will be modified. The third, params, specifies what value or values will be
**  assigned to the specified parameter.
**
**  Material parameters are used in the lighting equation that is optionally
**  applied to each vertex. The equation is discussed in the glLightModel
**  reference page.
**
**  INPUT:
**
**      Face
**          Specifies which face or faces are being updated.
**          Must be GL_FRONT_AND_BACK.
**
**      Name
**          Specifies the material parameter of the face or faces that is being
**          updated. Must be one of GL_AMBIENT, GL_DIFFUSE, GL_SPECULAR,
**          GL_EMISSION, GL_SHININESS, or GL_AMBIENT_AND_DIFFUSE.
**
**      Value
**          Specifies a pointer to the value or values that Name will be set to.
**
**  OUTPUT:
**
**      Nothing.
*/

GL_API void GL_APIENTRY glMaterialx(
    GLenum Face,
    GLenum Name,
    GLfixed Value
    )
{
    glmENTER3(glmARGENUM, Face, glmARGENUM, Name, glmARGFIXED, Value)
    {
        GLfloat value;

        gcmDUMP_API("${ES11 glMaterialx 0x%08X 0x%08X 0x%08X}", Face, Name, Value);
        glmPROFILE(context, GLES1_MATERIALX, 0);

        /* Convert to float */
        value = glmFIXED2FLOAT(Value);

        glmERROR(_SetMaterial(context, Face, Name, &value, 1));
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glMaterialxOES(
    GLenum Face,
    GLenum Name,
    GLfixed Value
    )
{
    glmENTER3(glmARGENUM, Face, glmARGENUM, Name, glmARGFIXED, Value)
    {
        GLfloat value;

        gcmDUMP_API("${ES11 glMaterialxOES 0x%08X 0x%08X 0x%08X}", Face, Name, Value);

        /* Convert to float */
        value = glmFIXED2FLOAT(Value);

        glmERROR(_SetMaterial(context, Face, Name, &value, 1));
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glMaterialxv(
    GLenum Face,
    GLenum Name,
    const GLfixed* Value
    )
{
    glmENTER3(glmARGENUM, Face, glmARGENUM, Name, glmARGPTR, Value)
    {
        GLfloat values[4];

        gcmDUMP_API("${ES11 glMaterialxv 0x%08X 0x%08X (0x%08X)", Face, Name, Value);
        gcmDUMP_API_ARRAY(Value, 4);
        gcmDUMP_API("$}");

        /* Convert to float */
        values[0] = glmFIXED2FLOAT(Value[0]);
        values[1] = glmFIXED2FLOAT(Value[1]);
        values[2] = glmFIXED2FLOAT(Value[2]);
        values[3] = glmFIXED2FLOAT(Value[3]);

        glmPROFILE(context, GLES1_MATERIALXV, 0);
        glmERROR(_SetMaterial(context, Face, Name, values, 4));
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glMaterialxvOES(
    GLenum Face,
    GLenum Name,
    const GLfixed* Value
    )
{
    glmENTER3(glmARGENUM, Face, glmARGENUM, Name, glmARGPTR, Value)
    {
        GLfloat values[4];

        gcmDUMP_API("${ES11 glMaterialxvOES 0x%08X 0x%08X (0x%08X)", Face, Name, Value);
        gcmDUMP_API_ARRAY(Value, 4);
        gcmDUMP_API("$}");

        /* Convert to float */
        values[0] = glmFIXED2FLOAT(Value[0]);
        values[1] = glmFIXED2FLOAT(Value[1]);
        values[2] = glmFIXED2FLOAT(Value[2]);
        values[3] = glmFIXED2FLOAT(Value[3]);

        glmERROR(_SetMaterial(context, Face, Name, values, 4));
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glMaterialf(
    GLenum Face,
    GLenum Name,
    GLfloat Value
    )
{
    glmENTER3(glmARGENUM, Face, glmARGENUM, Name, glmARGFLOAT, Value)
    {
        gcmDUMP_API("${ES11 glMaterialf 0x%08X 0x%08X 0x%08X}", Face, Name, *(GLuint*) &Value);

        glmPROFILE(context, GLES1_MATERIALF, 0);
        glmERROR(_SetMaterial(context, Face, Name, &Value, 1));
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glMaterialfv(
    GLenum Face,
    GLenum Name,
    const GLfloat* Value
    )
{
    glmENTER3(glmARGENUM, Face, glmARGENUM, Name, glmARGPTR, Value)
    {
        gcmDUMP_API("${ES11 glMaterialfv 0x%08X 0x%08X (0x%08X)", Face, Name, Value);
        gcmDUMP_API_ARRAY(Value, 4);
        gcmDUMP_API("$}");

        glmPROFILE(context, GLES1_MATERIALFV, 0);
        glmERROR(_SetMaterial(context, Face, Name, Value, 4));
    }
    glmLEAVE();
}


/*******************************************************************************
**
**  glGetLight
**
**  glGetLight returns in params the value or values of a light source
**  parameter. light names the light and is a symbolic name of the form
**  GL_LIGHTi for 0 <= i < GL_MAX_LIGHTS where GL_MAX_LIGHTS is an
**  implementation dependent constant that is greater than or equal to eight.
**  Name specifies one of ten light source parameters, again by symbolic name.
**
**  INPUT:
**
**      Light
**          Specifies a light source. The number of possible lights depends on
**          the implementation, but at least eight lights are supported.
**          They are identified by symbolic names of the form GL_LIGHTi
**          where 0 <= i < GL_MAX_LIGHTS.
**
**      Name
**          Specifies a light source parameter for light. Accepted symbolic
**          names are GL_AMBIENT, GL_DIFFUSE, GL_SPECULAR, GL_EMISSION,
**          GL_SPOT_DIRECTION, GL_SPOT_EXPONENT, GL_SPOT_CUTOFF,
**          GL_CONSTANT_ATTENUATION, GL_LINEAR_ATTENUATION,
**          and GL_QUADRATIC_ATTENUATION.
**
**  OUTPUT:
**
**      Value
**          Returns the requested data.
*/

GL_API void GL_APIENTRY glGetLightxv(
    GLenum Light,
    GLenum Name,
    GLfixed* Value
    )
{
    glmENTER3(glmARGENUM, Light, glmARGENUM, Name, glmARGPTR, Value)
    {
        glmPROFILE(context, GLES1_GETLIGHTXV, 0);
        glmERROR(_GetLight(context, Light, Name, Value, glvFIXED));

        gcmDUMP_API("${ES11 glGetLightxv 0x%08X 0x%08X (0x%08X)", Light, Name, Value);
        gcmDUMP_API_ARRAY(Value, 4);
        gcmDUMP_API("$}");
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glGetLightxvOES(
    GLenum Light,
    GLenum Name,
    GLfixed* Value
    )
{
    glmENTER3(glmARGENUM, Light, glmARGENUM, Name, glmARGPTR, Value)
    {
        gcmDUMP_API("${ES11 glGetLightxvOES 0x%08X 0x%08X (0x%08X)", Light, Name, Value);
        gcmDUMP_API_ARRAY(Value, 4);
        gcmDUMP_API("$}");

        glmERROR(_GetLight(context, Light, Name, Value, glvFIXED));
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glGetLightfv(
    GLenum Light,
    GLenum Name,
    GLfloat* Value
    )
{
    glmENTER3(glmARGENUM, Light, glmARGENUM, Name, glmARGPTR, Value)
    {
        glmPROFILE(context, GLES1_GETLIGHTFV, 0);
        glmERROR(_GetLight(context, Light, Name, Value, glvFLOAT));

        gcmDUMP_API("${ES11 glGetLightfv 0x%08X 0x%08X (0x%08X)", Light, Name, Value);
        gcmDUMP_API_ARRAY(Value, 4);
        gcmDUMP_API("$}");
    }
    glmLEAVE();
}


/*******************************************************************************
**
**  glShadeModel
**
**  GL primitives can have either flat or smooth shading. Smooth shading,
**  the default, causes the computed colors of vertices to be interpolated
**  as the primitive is rasterized, typically assigning different colors to
**  each resulting pixel fragment. Flat shading selects the computed color of
**  just one vertex and assigns it to all the pixel fragments generated by
**  rasterizing a single primitive. In either case, the computed color of
**  a vertex is the result of lighting if lighting is enabled, or it is the
**  current color at the time the vertex was specified if lighting is disabled.
**
**  Flat and smooth shading are indistinguishable for points. Starting at the
**  beginning of the vertex array and counting vertices and primitives from 1,
**  the GL gives each flat-shaded line segment i the computed color of vertex
**  i + 1, its second vertex. Counting similarly from 1, the GL gives each
**  flat-shaded polygon the computed color of vertex i + 2, which is the last
**  vertex to specify the polygon.
**
**  Flat and smooth shading are specified by glShadeModel with mode set to
**  GL_FLAT and GL_SMOOTH, respectively.
**
**  INPUT:
**
**      Mode
**          Specifies a symbolic value representing a shading technique.
**          Accepted values are GL_FLAT and GL_SMOOTH.
**          The initial value is GL_SMOOTH.
**
**  OUTPUT:
**
**      Nothing.
*/

GL_API void GL_APIENTRY glShadeModel(
    GLenum Mode
    )
{
    glmENTER1(glmARGENUM, Mode)
    {
        gcmDUMP_API("${ES11 glShadeModel 0x%08X}", Mode);

        glmPROFILE(context, GLES1_SHADEMODEL, 0);
        glmERROR(_SetShadeModel(context, Mode));
    }
    glmLEAVE();
}
