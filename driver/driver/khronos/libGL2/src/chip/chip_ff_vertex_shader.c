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
#include "chip_context.h"


#define _GC_OBJ_ZONE glvZONE_SHADER


/*******************************************************************************
** Vertex Shader generation defines.
*/

#define glmLIGHTING_UNIFORM(Shader, Name, Swizzle, LightIndex) \
    if (LightIndex == -1) \
    { \
        glmUNIFORM_DYNAMIC(Shader, Name, Swizzle, ShaderControl->rLightIndex); \
    } \
    else \
    { \
        glmUNIFORM_STATIC(Shader, Name, Swizzle, LightIndex); \
    }

#define setColor(color, value) {                               \
    *value++ = color.r;                                        \
    *value++ = color.g;                                        \
    *value++ = color.b;                                        \
    *value++ = color.a;                                        \
}

/*******************************************************************************
** Indexing and swizzle sets used in loops.
*/

/*******************************************************************************
** Vertex Shader internal parameters.
*/

typedef struct _glsVSCONTROL * glsVSCONTROL_PTR;
typedef struct _glsVSCONTROL
{
    /* Pointer to the exposed shader interface. */
    glsSHADERCONTROL_PTR i;

    /* Temporary register and label allocation indices. */
    glmDEFINE_TEMP(rLastAllocated);
    glmDEFINE_LABEL(lLastAllocated);

    /* Temporary result registers that are used in more then one place. */
    glmDEFINE_TEMP(rVtxInEyeSpace);
    glmDEFINE_TEMP(rNrmInEyeSpace)[2];

    /* Lighting. */
    gctINT outputCount;

    glmDEFINE_TEMP(rLighting)[2];
    glmDEFINE_TEMP(rLighting2)[2];
    glmDEFINE_TEMP(rLightIndex);
    gcFUNCTION funcLighting;

    glmDEFINE_TEMP(rVPpli);
    glmDEFINE_TEMP(rVPpliLength);
    glmDEFINE_TEMP(rNdotVPpli)[2];
    glmDEFINE_TEMP(rAttenuation);
    glmDEFINE_TEMP(rSpot);
    glmDEFINE_TEMP(rAmbient)[2];
    glmDEFINE_TEMP(rDiffuse)[2];
    glmDEFINE_TEMP(rSpecular)[2];

    /* Uniforms. */
    glsUNIFORMWRAP_PTR uniforms[glvUNIFORM_VS_COUNT];

    /* Attributes. */
    glsATTRIBUTEWRAP_PTR attributes[glvATTRIBUTE_VS_COUNT];

    /* Varyings. */
    glmDEFINE_TEMP(vPosition);
    glmDEFINE_TEMP(vEyePosition);
    glmDEFINE_TEMP(vColor)[2];     /* primary front and back */
    glmDEFINE_TEMP(vColor2)[2];    /* secondary front and back */
    glmDEFINE_TEMP(vTexCoord)[glvMAX_TEXTURES];
    glmDEFINE_TEMP(vClipPlane)[glvMAX_CLIP_PLANES];
    glmDEFINE_TEMP(vPointSize);
    glmDEFINE_TEMP(vPointFade);
    glmDEFINE_TEMP(vPointSmooth);
}
glsVSCONTROL;

GLboolean colorMaterialEnabled( __GLcontext * gc, GLenum colorMaterialFace, GLenum colorMaterialParam)
{
    if (gc->state.enables.lighting.colorMaterial) {
        switch (colorMaterialFace)
        {
            case 0:
                if (((gc->state.light.colorMaterialFace == GL_FRONT_AND_BACK) ||
                    (gc->state.light.colorMaterialFace == GL_FRONT)) &&
                    (gc->state.light.colorMaterialParam == colorMaterialParam))
                {
                    return GL_TRUE;
                }
                break;
            case 1:
                if (((gc->state.light.colorMaterialFace == GL_FRONT_AND_BACK) ||
                    (gc->state.light.colorMaterialFace == GL_BACK)) &&
                    (gc->state.light.colorMaterialParam == colorMaterialParam))
                {
                    return GL_TRUE;
                }
                break;
        }
    }
    return GL_FALSE;
}

/*******************************************************************************
**
**  convertToVivanteMatrix
**
**      Z'c = (Zc + Wc) / 2
**
**      0 < Z'c <= Wc.
**
**  INPUT:
**
**      Matrix
**          Pointer to the matrix to be converted.
**
**  OUTPUT:
**
**      Result
**          Converted matrix.
*/

static void
convertToVivanteMatrix(
    __GLcontext * gc,
    GLfloat matrix[]
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    GLfloat z0, z1, z2, z3;

    if (chipCtx->chipModel >= gcv1000)
    {
        return;
    }

    z0 = matrix[8] + matrix[12];
    z1 = matrix[9] + matrix[13];
    z2 = matrix[10] + matrix[14];
    z3 = matrix[11] + matrix[15];

    matrix[8] = z0 * 0.5f;
    matrix[9] = z1 * 0.5f;
    matrix[10] = z2 * 0.5f;
    matrix[11] = z3 * 0.5f;
}


/*******************************************************************************
** Temporary allocation helper.
*/

static gctUINT16 allocateTemp(
    glsVSCONTROL_PTR ShaderControl
    )
{
    gctUINT16 result;
    gcmHEADER_ARG("ShaderControl=0x%x", ShaderControl);
    gcmASSERT(ShaderControl->rLastAllocated < 65535);
    result = ++ShaderControl->rLastAllocated;
    gcmFOOTER_ARG("%u", result);
    return result;
}


/*******************************************************************************
** Label allocation helper.
*/

static gctUINT allocateLabel(
    glsVSCONTROL_PTR ShaderControl
    )
{
    gctUINT result;
    gcmHEADER_ARG("ShaderControl=0x%x", ShaderControl);
    result = ++ShaderControl->lLastAllocated;
    gcmFOOTER_ARG("%u", result);
    return result;
}


/*******************************************************************************
** Constant setting callbacks.
*/

static gceSTATUS set_uFogCoord(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    gceSTATUS status;
    gltFRACTYPE valueArray[4];
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);
    valueArray[0] = gc->state.current.fog;
    status = gcUNIFORM_SetValueF_Ex(Uniform, 1, chipCtx->currProgram->hints, valueArray);
    gcmFOOTER_NO();
    return status;
}


static gceSTATUS set_uColor(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    gceSTATUS status;
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);
    status = gcUNIFORM_SetValueF_Ex(Uniform, 1, chipCtx->currProgram->hints, &gc->state.current.color.r);
    gcmFOOTER_NO();
    return status;
}


static gceSTATUS set_uNormal(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    gceSTATUS status;
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);
    status = gcUNIFORM_SetValueF_Ex(Uniform, 1, chipCtx->currProgram->hints, gc->state.current.normal.v);
    gcmFOOTER();
    return status;
}

gceSTATUS set_uModelView(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    gltFRACTYPE valueArray[4 * 4];
    gltFRACTYPE * value = valueArray;
    GLuint x, y;
    gceSTATUS status;
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);
    for (y = 0; y < 4; y++) {
        for (x = 0; x < 4; x++) {
            *value++ = gc->transform.modelView->matrix.matrix[x][y];
        }
    }

    status = gcUNIFORM_SetValueF_Ex(Uniform, 4, chipCtx->currProgram->hints, valueArray);

    gcmFOOTER();
    return status;
}

gceSTATUS set_uModelViewInverse3x3Transposed(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    gltFRACTYPE valueArray[3 * 3];
    gltFRACTYPE * value = valueArray;
    gceSTATUS status;
    GLuint x, y;
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);

    for (y = 0; y < 3; y++) {
        for (x = 0; x < 3; x++) {
            *value++ = gc->transform.modelView->inverseTranspose.matrix[x][y];
        }
    }
    status = gcUNIFORM_SetValueF_Ex(Uniform, 3, chipCtx->currProgram->hints, valueArray);
    gcmFOOTER();
    return status;
}

gceSTATUS set_uModelViewProjection(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    gltFRACTYPE valueArray[4 * 4];
    gltFRACTYPE * value = valueArray;
    GLuint x, y;
    gceSTATUS status = gcvSTATUS_OK;
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);

    for (y = 0; y < 4; y++) {
        for (x = 0; x < 4; x++) {
            *value++ = gc->transform.modelView->mvp.matrix[x][y];
        }
    }

    convertToVivanteMatrix(gc, valueArray);
    status = gcUNIFORM_SetValueF_Ex(Uniform, 4, chipCtx->currProgram->hints, valueArray);

    gcmFOOTER();
    return status;
}

gceSTATUS set_uProjection(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    gceSTATUS status;
    gltFRACTYPE valueArray[4 * 4];
    gltFRACTYPE * value = valueArray;
    GLuint x, y;
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);
    for (y = 0; y < 4; y++) {
        for (x = 0; x < 4; x++) {
            *value++ = gc->transform.projection->matrix.matrix[x][y];
        }
    }
    convertToVivanteMatrix(gc, valueArray);
    status = gcUNIFORM_SetValueF_Ex(Uniform, 4, chipCtx->currProgram->hints, valueArray);
    gcmFOOTER();
    return status;
}

static gceSTATUS set_uEcm(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    gceSTATUS status;
    gltFRACTYPE valueArray[2 * 4];
    gltFRACTYPE * value = valueArray;
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);
    setColor(gc->state.light.front.emissive, value);
    setColor(gc->state.light.back.emissive, value);
    status = gcUNIFORM_SetValueF_Ex(Uniform, 2, chipCtx->currProgram->hints, valueArray);
    gcmFOOTER();
    return status;
}

static gceSTATUS set_uAcm(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    gceSTATUS status;
    gltFRACTYPE valueArray[2 * 4];
    gltFRACTYPE * value = valueArray;
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);
    setColor(gc->state.light.front.ambient, value);
    setColor(gc->state.light.back.ambient, value);
    status = gcUNIFORM_SetValueF_Ex(Uniform, 2, chipCtx->currProgram->hints, valueArray);
    gcmFOOTER_NO();
    return status;
}

static gceSTATUS set_uDcm(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    gceSTATUS status;
    gltFRACTYPE valueArray[2 * 4];
    gltFRACTYPE * value = valueArray;
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);
    setColor(gc->state.light.front.diffuse, value);
    setColor(gc->state.light.back.diffuse, value);
    status = gcUNIFORM_SetValueF_Ex(Uniform, 2, chipCtx->currProgram->hints, valueArray);
    gcmFOOTER();
    return status;
}

static gceSTATUS set_uAcs(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    gceSTATUS status;
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);
    status = gcUNIFORM_SetValueF_Ex(Uniform, 1, chipCtx->currProgram->hints, &gc->state.light.model.ambient.r);
    gcmFOOTER();
    return status;
}

static gceSTATUS set_uSrm(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    gceSTATUS status;
    gltFRACTYPE valueArray[2 * 4];
    gltFRACTYPE * value = valueArray;
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);
    *value++ = gc->state.light.front.specularExponent;
    *value = gc->state.light.back.specularExponent;
    status = gcUNIFORM_SetValueF_Ex(Uniform, 2, chipCtx->currProgram->hints, valueArray);
    gcmFOOTER();
    return status;
}

static gceSTATUS set_uScm(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    gceSTATUS status;
    gltFRACTYPE valueArray[2 * 4];
    gltFRACTYPE * value = valueArray;
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);
    setColor(gc->state.light.front.specular, value);
    setColor(gc->state.light.back.specular, value);
    status = gcUNIFORM_SetValueF_Ex(Uniform, 2, chipCtx->currProgram->hints, valueArray);
    gcmFOOTER();
    return status;
}

static gceSTATUS set_uPpli(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gltFRACTYPE valueArray[4 * glvMAX_LIGHTS];
    GLuint i = 0, index = 0;
    gceSTATUS status = gcvSTATUS_OK;
    GLuint enableMask = chipCtx->lightingStates.lightEnabled;
    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);
    while((i < glvMAX_LIGHTS) && enableMask) {
        if (enableMask & 1) {
            valueArray[index++] = gc->state.light.source[i].positionEye.x;
            valueArray[index++] = gc->state.light.source[i].positionEye.y;
            valueArray[index++] = gc->state.light.source[i].positionEye.z;
            valueArray[index++] = gc->state.light.source[i].positionEye.w;
        } else {
            index += 4;
        }
        i++;
        enableMask >>= 1;
    }
    if (i > 0) {
        status = gcUNIFORM_SetValueF_Ex(Uniform, i, chipCtx->currProgram->hints, valueArray);
    }
    gcmFOOTER();
    return status;
}

static gceSTATUS set_uKi(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gltFRACTYPE valueArray[3 * glvMAX_LIGHTS];
    GLuint i = 0, index = 0;
    gceSTATUS status = gcvSTATUS_OK;
    GLuint enableMask = chipCtx->lightingStates.lightEnabled;
    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);
    while((i < glvMAX_LIGHTS) && enableMask) {
        if (enableMask & 1) {
            valueArray[index++] = gc->state.light.source[i].constantAttenuation;
            valueArray[index++] = gc->state.light.source[i].linearAttenuation;;
            valueArray[index++] = gc->state.light.source[i].quadraticAttenuation;
        } else {
            index += 3;
        }
        i++;
        enableMask >>= 1;
    }
    if (i > 0) {
        status = gcUNIFORM_SetValueF_Ex(Uniform, i, chipCtx->currProgram->hints, valueArray);
    }
    gcmFOOTER();
    return status;
}

static gceSTATUS set_uSrli(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    GLuint i = 0;
    gltFRACTYPE valueArray[glvMAX_LIGHTS];
    gceSTATUS status = gcvSTATUS_OK;
    GLuint enableMask = chipCtx->lightingStates.lightEnabled;
    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);
    while((i < glvMAX_LIGHTS) && enableMask) {
        if (enableMask & 1) {
            valueArray[i] = gc->state.light.source[i].spotLightExponent;
        }
        i++;
        enableMask >>= 1;
    }
    if (i > 0) {
        status = gcUNIFORM_SetValueF_Ex(Uniform, i, chipCtx->currProgram->hints, valueArray);
    }

    gcmFOOTER();
    return status;
}

static gceSTATUS set_uAcli(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    GLuint i = 0, index = 0;
    gltFRACTYPE valueArray[4 * glvMAX_LIGHTS];
    gceSTATUS status = gcvSTATUS_OK;
    GLuint enableMask = chipCtx->lightingStates.lightEnabled;
    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);
    while((i < glvMAX_LIGHTS) && enableMask) {
        if (enableMask & 1) {
            valueArray[index++] = gc->state.light.source[i].ambient.r;
            valueArray[index++] = gc->state.light.source[i].ambient.g;
            valueArray[index++] = gc->state.light.source[i].ambient.b;
            valueArray[index++] = gc->state.light.source[i].ambient.a;
        } else {
            index += 4;
        }
        i++;
        enableMask >>= 1;
    }
    if (i > 0) {
        status = gcUNIFORM_SetValueF_Ex(Uniform, i, chipCtx->currProgram->hints, valueArray);
    }
    gcmFOOTER();
    return status;
}

static gceSTATUS set_uDcli(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    GLuint i = 0, index = 0;
    gltFRACTYPE valueArray[4 * glvMAX_LIGHTS];
    gceSTATUS status = gcvSTATUS_OK;
    GLuint enableMask = chipCtx->lightingStates.lightEnabled;
    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);
    while((i < glvMAX_LIGHTS) && enableMask) {
        if (enableMask & 1) {
            valueArray[index++] = gc->state.light.source[i].diffuse.r;
            valueArray[index++] = gc->state.light.source[i].diffuse.g;
            valueArray[index++] = gc->state.light.source[i].diffuse.b;
            valueArray[index++] = gc->state.light.source[i].diffuse.a;
        } else {
            index += 4;
        }
        i++;
        enableMask >>= 1;
    }
    if (i > 0) {
        status = gcUNIFORM_SetValueF_Ex(Uniform, i, chipCtx->currProgram->hints, valueArray);
    }
    gcmFOOTER();
    return status;
}

static gceSTATUS set_uScli(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gltFRACTYPE valueArray[4 * glvMAX_LIGHTS];
    GLuint i = 0, index = 0;
    gceSTATUS status = gcvSTATUS_OK;
    GLuint enableMask = chipCtx->lightingStates.lightEnabled;
    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);
    while((i < glvMAX_LIGHTS) && enableMask) {
        if (enableMask & 1) {
            valueArray[index++] = gc->state.light.source[i].specular.r;
            valueArray[index++] = gc->state.light.source[i].specular.g;
            valueArray[index++] = gc->state.light.source[i].specular.b;
            valueArray[index++] = gc->state.light.source[i].specular.a;
        } else {
            index += 4;
        }
        i++;
        enableMask >>= 1;
    }
    if (i > 0) {
        status = gcUNIFORM_SetValueF_Ex(Uniform, i, chipCtx->currProgram->hints, valueArray);
    }
    gcmFOOTER();
    return status;
}

static gceSTATUS set_uTexMatrix(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    GLint i = 0, x, y;
    gltFRACTYPE valueArray[4 * 4 * glvMAX_TEXTURES];
    gltFRACTYPE* value = valueArray;
    gceSTATUS status = gcvSTATUS_OK;
    GLuint stageEnableMask;

    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);


    stageEnableMask = chipCtx->texture.stageEnabledMask;
    while ((i < gc->constants.numberOfTextureUnits) && stageEnableMask)
    {
        if (stageEnableMask & 1) {
            for (y = 0; y < 4; y++)
            {
                for (x = 0; x < 4; x++)
                {
                    *value++ = gc->transform.texture[i]->matrix.matrix[x][y];
                }
            }
        } else {
            value += 16;
        }
        i++;
        stageEnableMask >>= 1;
    }

    if (i > 0) {
        status = gcUNIFORM_SetValueF_Ex(Uniform,
            4 * i, chipCtx->currProgram->hints, valueArray);
    }

    gcmFOOTER();
    return status;
}

static gceSTATUS set_uClipPlane(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    GLuint i, index = 0;
    gltFRACTYPE valueArray[4 * glvMAX_CLIP_PLANES];
    gceSTATUS status;
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);
    for (i = 0; i < glvMAX_CLIP_PLANES; i++) {
        valueArray[index++] = gc->state.transform.eyeClipPlanes[i].x;
        valueArray[index++] = gc->state.transform.eyeClipPlanes[i].y;
        valueArray[index++] = gc->state.transform.eyeClipPlanes[i].z;
        valueArray[index++] = gc->state.transform.eyeClipPlanes[i].w;
    }
    status = gcUNIFORM_SetValueF_Ex(Uniform, glvMAX_CLIP_PLANES, chipCtx->currProgram->hints, valueArray);

    gcmFOOTER_NO();
    return status;
}

static gceSTATUS set_uPointAttenuation(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    gceSTATUS status;
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);
    status = gcUNIFORM_SetValueF_Ex(Uniform, 1, chipCtx->currProgram->hints, gc->state.point.distanceAttenuation);
    gcmFOOTER_NO();
    return status;
}

static gceSTATUS set_uPointSize(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    gceSTATUS status;
    gltFRACTYPE valueArray[4];
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);
    valueArray[0] = gc->state.point.requestedSize;
    valueArray[1] = gc->state.point.sizeMin;
    valueArray[2] = gc->state.point.sizeMax;
    valueArray[3] = gc->state.point.fadeThresholdSize;
    status = gcUNIFORM_SetValueF_Ex(Uniform, 1, chipCtx->currProgram->hints, valueArray);
    gcmFOOTER();
    return status;
}

static gceSTATUS set_uViewport(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{

    gltFRACTYPE valueArray[4];
    gceSTATUS status = gcvSTATUS_OK;
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);
    /* ViewportScaleX */
    valueArray[0] = glmINT2FRAC(gc->state.viewport.width / 2);

    /* ViewportOriginX */
    valueArray[1] = glmINT2FRAC(gc->state.viewport.x + gc->state.viewport.width / 2);

    /* ViewportScaleY */
    valueArray[2] = glmINT2FRAC(gc->state.viewport.height / 2);

    /* ViewportOriginY */
    valueArray[3] = glmINT2FRAC(gc->state.viewport.y + gc->state.viewport.height / 2);

    status = gcUNIFORM_SetValueF_Ex(Uniform, 1, chipCtx->currProgram->hints, valueArray);

    gcmFOOTER();
    return status;
}

static gceSTATUS set_uAcmAcli(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gltFRACTYPE vAcmAcli[4 * glvMAX_LIGHTS];
    GLfloat     vec[] = {0.0f, 0.0f, 0.0f, 1.0f};
    GLint       i = 0, index = 0;
    gceSTATUS status = gcvSTATUS_OK;
    GLuint enableMask = chipCtx->lightingStates.lightEnabled;

    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);

    gcoOS_ZeroMemory(vAcmAcli, 4 * glvMAX_LIGHTS * sizeof(gltFRACTYPE));
    while((i < glvMAX_LIGHTS) && enableMask) {
        if (enableMask & 1) {
            if ((gc->state.enables.lighting.colorMaterial) &&
                !chipCtx->attributeInfo[__GL_INPUT_DIFFUSE_INDEX].streamEnabled)
            {
                vAcmAcli[index++] = gc->state.current.color.r *
                    gc->state.light.source[i].ambient.r;
                vAcmAcli[index++] = gc->state.current.color.g *
                    gc->state.light.source[i].ambient.g;
                vAcmAcli[index++] = gc->state.current.color.b *
                    gc->state.light.source[i].ambient.b;
                vAcmAcli[index++] = gc->state.current.color.a *
                    gc->state.light.source[i].ambient.a;
            }
            else if (!gc->state.enables.lighting.colorMaterial)
            {
                vAcmAcli[index++] = gc->state.light.front.ambient.r *
                    gc->state.light.source[i].ambient.r;
                vAcmAcli[index++] = gc->state.light.front.ambient.g *
                    gc->state.light.source[i].ambient.g;
                vAcmAcli[index++] = gc->state.light.front.ambient.b *
                    gc->state.light.source[i].ambient.b;
                vAcmAcli[index++] = gc->state.light.front.ambient.a *
                    gc->state.light.source[i].ambient.a;
            } else {
                vAcmAcli[index++] = vec[0];
                vAcmAcli[index++] = vec[1];
                vAcmAcli[index++] = vec[2];
                vAcmAcli[index++] = vec[3];
            }
        } else {
            index += 4;
        }
        i++;
        enableMask >>= 1;
    }

    if (i > 0) {
        status = gcUNIFORM_SetValueF_Ex(Uniform, i, chipCtx->currProgram->hints, vAcmAcli);
    }

    gcmFOOTER();

    return status;
}

static gceSTATUS set_uAcmAcli2(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gltFRACTYPE vAcmAcli[4 * glvMAX_LIGHTS];
    GLfloat     vec[] = {0.0f, 0.0f, 0.0f, 1.0f};
    GLint       i = 0, index = 0;
    gceSTATUS status = gcvSTATUS_OK;
    GLuint enableMask = chipCtx->lightingStates.lightEnabled;

    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);

    gcoOS_ZeroMemory(vAcmAcli, 4 * glvMAX_LIGHTS * sizeof(gltFRACTYPE));
    while((i < glvMAX_LIGHTS) && enableMask) {
        if (enableMask & 1) {
            if ((gc->state.enables.lighting.colorMaterial) &&
                !chipCtx->attributeInfo[__GL_INPUT_DIFFUSE_INDEX].streamEnabled)
            {
                vAcmAcli[index++] = gc->state.current.color.r *
                    gc->state.light.source[i].ambient.r;
                vAcmAcli[index++] = gc->state.current.color.g *
                    gc->state.light.source[i].ambient.g;
                vAcmAcli[index++] = gc->state.current.color.b *
                    gc->state.light.source[i].ambient.b;
                vAcmAcli[index++] = gc->state.current.color.a *
                    gc->state.light.source[i].ambient.a;
            }
            else if (!gc->state.enables.lighting.colorMaterial)
            {
                vAcmAcli[index++] = gc->state.light.back.ambient.r *
                    gc->state.light.source[i].ambient.r;
                vAcmAcli[index++] = gc->state.light.back.ambient.g *
                    gc->state.light.source[i].ambient.g;
                vAcmAcli[index++] = gc->state.light.back.ambient.b *
                    gc->state.light.source[i].ambient.b;
                vAcmAcli[index++] = gc->state.light.back.ambient.a *
                    gc->state.light.source[i].ambient.a;
            } else {
                vAcmAcli[index++] = vec[0];
                vAcmAcli[index++] = vec[1];
                vAcmAcli[index++] = vec[2];
                vAcmAcli[index++] = vec[3];
            }
        } else {
            index += 4;
        }
        i++;
        enableMask >>= 1;
    }

    if (i > 0) {
        status = gcUNIFORM_SetValueF_Ex(Uniform, i, chipCtx->currProgram->hints, vAcmAcli);
    }

    gcmFOOTER();

    return status;
}

static gceSTATUS set_uVPpli(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    GLint       i = 0, index = 0;
    gltFRACTYPE vPpli[4 * glvMAX_LIGHTS];
    gceSTATUS status = gcvSTATUS_OK;
    GLuint enableMask = chipCtx->lightingStates.lightEnabled;

    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);

    while((i < glvMAX_LIGHTS) && enableMask) {
        if (enableMask & 1) {
            vPpli[index++] = gc->state.light.source[i].positionEye.x;
            vPpli[index++] = gc->state.light.source[i].positionEye.y;
            vPpli[index++] = gc->state.light.source[i].positionEye.z;
            vPpli[index++] = gc->state.light.source[i].positionEye.w;
        } else {
            index += 4;
        }
        i++;
        enableMask >>= 1;
    }

    if (i > 0) {
        status = gcUNIFORM_SetValueF_Ex(Uniform, i, chipCtx->currProgram->hints, vPpli);
    }

    gcmFOOTER();

    return status;
}

static gceSTATUS set_uDcmDcli(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gltFRACTYPE vDcmDcli[4 * glvMAX_LIGHTS];
    GLfloat     vec[] = {0.0f, 0.0f, 0.0f, 1.0f};
    GLint       i = 0, index = 0;
    gceSTATUS status;
    GLuint enableMask = chipCtx->lightingStates.lightEnabled;

    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);

    gcoOS_ZeroMemory(vDcmDcli, 4 * glvMAX_LIGHTS * sizeof(gltFRACTYPE));

    while((i < glvMAX_LIGHTS) && enableMask) {
        if (enableMask & 1) {
            if (gc->state.enables.lighting.colorMaterial &&
                !chipCtx->attributeInfo[__GL_INPUT_DIFFUSE_INDEX].streamEnabled)
            {
                vDcmDcli[index++] = gc->state.current.color.r *
                    gc->state.light.source[i].diffuse.r;
                vDcmDcli[index++] = gc->state.current.color.g *
                    gc->state.light.source[i].diffuse.g;
                vDcmDcli[index++] = gc->state.current.color.b *
                    gc->state.light.source[i].diffuse.b;
                vDcmDcli[index++] = gc->state.current.color.a *
                    gc->state.light.source[i].diffuse.a;
            }
            else if (!gc->state.enables.lighting.colorMaterial)
            {
                vDcmDcli[index++] = gc->state.light.front.diffuse.r *
                    gc->state.light.source[i].diffuse.r;
                vDcmDcli[index++] = gc->state.light.front.diffuse.g *
                    gc->state.light.source[i].diffuse.g;
                vDcmDcli[index++] = gc->state.light.front.diffuse.b *
                    gc->state.light.source[i].diffuse.b;
                vDcmDcli[index++] = gc->state.light.front.diffuse.a *
                    gc->state.light.source[i].diffuse.a;
            } else {
                vDcmDcli[index++] = vec[0];
                vDcmDcli[index++] = vec[1];
                vDcmDcli[index++] = vec[2];
                vDcmDcli[index++] = vec[3];
            }
        } else {
            index += 4;
        }
        i++;
        enableMask >>= 1;
    }
    status = gcUNIFORM_SetValueF_Ex(Uniform, glvMAX_LIGHTS, chipCtx->currProgram->hints, vDcmDcli);

    gcmFOOTER();

    return status;
}

static gceSTATUS set_uDcmDcli2(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gltFRACTYPE vDcmDcli[4 * glvMAX_LIGHTS];
    GLfloat     vec[] = {0.0f, 0.0f, 0.0f, 1.0f};
    GLint       i = 0, index = 0;
    gceSTATUS status = gcvSTATUS_OK;
    GLuint enableMask = chipCtx->lightingStates.lightEnabled;

    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);

    gcoOS_ZeroMemory(vDcmDcli, 4 * glvMAX_LIGHTS * sizeof(gltFRACTYPE));

    while((i < glvMAX_LIGHTS) && enableMask) {
        if (enableMask & 1) {
            if (gc->state.enables.lighting.colorMaterial &&
                !chipCtx->attributeInfo[__GL_INPUT_DIFFUSE_INDEX].streamEnabled)
            {
                vDcmDcli[index++] = gc->state.current.color.r *
                    gc->state.light.source[i].diffuse.r;
                vDcmDcli[index++] = gc->state.current.color.g *
                    gc->state.light.source[i].diffuse.g;
                vDcmDcli[index++] = gc->state.current.color.b *
                    gc->state.light.source[i].diffuse.b;
                vDcmDcli[index++] = gc->state.current.color.a *
                    gc->state.light.source[i].diffuse.a;
            }
            else if (!gc->state.enables.lighting.colorMaterial)
            {
                vDcmDcli[index++] = gc->state.light.back.diffuse.r *
                    gc->state.light.source[i].diffuse.r;
                vDcmDcli[index++] = gc->state.light.back.diffuse.g *
                    gc->state.light.source[i].diffuse.g;
                vDcmDcli[index++] = gc->state.light.back.diffuse.b *
                    gc->state.light.source[i].diffuse.b;
                vDcmDcli[index++] = gc->state.light.back.diffuse.a *
                    gc->state.light.source[i].diffuse.a;
            } else {
                vDcmDcli[index++] = vec[0];
                vDcmDcli[index++] = vec[1];
                vDcmDcli[index++] = vec[2];
                vDcmDcli[index++] = vec[3];
            }
        } else {
            index += 4;
        }
        i++;
        enableMask >>= 1;
    }

    if (i > 0) {
        status = gcUNIFORM_SetValueF_Ex(Uniform, glvMAX_LIGHTS, chipCtx->currProgram->hints, vDcmDcli);
    }

    gcmFOOTER();

    return status;
}

static gceSTATUS set_uCrli(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gltFRACTYPE  uCrli[glvMAX_LIGHTS];
    GLint       i = 0;
    gceSTATUS status = gcvSTATUS_OK;
    GLuint enableMask = chipCtx->lightingStates.lightEnabled;

    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);

    while((i < glvMAX_LIGHTS) && enableMask) {
        if (enableMask & 1) {
            uCrli[i] = gc->state.light.source[i].spotLightCutOffAngle;
        }
        i++;
        enableMask >>= 1;
    }

    if (i > 0) {
        status = gcUNIFORM_SetValueF_Ex(Uniform, glvMAX_LIGHTS, chipCtx->currProgram->hints, uCrli);
    }

    gcmFOOTER();
    return status;
}

static gceSTATUS set_uCosCrli(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gltFRACTYPE  mCosCrli[glvMAX_LIGHTS];
    GLint       i = 0;
    gceSTATUS status = gcvSTATUS_OK;
    GLuint enableMask = chipCtx->lightingStates.lightEnabled;

    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);

    while((i < glvMAX_LIGHTS) && enableMask) {
        if (enableMask & 1) {
            /* Convert to randians. */
            GLfloat radians = gc->state.light.source[i].spotLightCutOffAngle * glvFLOATPIOVER180;
            /* Compute cos. */
            mCosCrli[i] = __GL_COSF(radians);
        }
        i++;
        enableMask >>= 1;
    }

    if (i > 0) {
        status = gcUNIFORM_SetValueF_Ex(Uniform, glvMAX_LIGHTS, chipCtx->currProgram->hints, mCosCrli);
    }

    gcmFOOTER();

    return status;
}

static gceSTATUS set_uNormedSdli(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gltFRACTYPE vNormedSdli[4 * glvMAX_LIGHTS];
    GLint       i = 0;
    gceSTATUS status = gcvSTATUS_OK;
    GLuint enableMask = chipCtx->lightingStates.lightEnabled;

    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);

    while((i < glvMAX_LIGHTS) && enableMask) {
        if (enableMask & 1) {
            glfNorm3Vector4f(gc->state.light.source[i].direction.v,
                            &vNormedSdli[i * 4]);
        }

        i++;
        enableMask >>= 1;
    }

    if (i > 0) {
        status = gcUNIFORM_SetValueF_Ex(Uniform, i, chipCtx->currProgram->hints, vNormedSdli);
    }

    gcmFOOTER();

    return status;
}


/*******************************************************************************
** Uniform access helpers.
*/

static gceSTATUS using_uFogCoord(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingUniform(
          ShaderControl->i,
          "uFogCoord",
          gcSHADER_FRAC_X1,
          1,
          set_uFogCoord,
          &glmUNIFORM_WRAP(VS, uFogCoord)
          );
    gcmFOOTER();
    return status;
}


static gceSTATUS using_uColor(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingUniform(
          ShaderControl->i,
          "uColor",
          gcSHADER_FRAC_X4,
          1,
          set_uColor,
          &glmUNIFORM_WRAP(VS, uColor)
          );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_uNormal(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingUniform(
        ShaderControl->i,
        "uNormal",
        gcSHADER_FRAC_X3,
        1,
        set_uNormal,
        &glmUNIFORM_WRAP(VS, uNormal)
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_uModelView(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingUniform(
        ShaderControl->i,
        "uModelView",
        gcSHADER_FRAC_X4,
        4,
        set_uModelView,
        &glmUNIFORM_WRAP(VS, uModelView)
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_uModelViewInverse3x3Transposed(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingUniform(
        ShaderControl->i,
        "uModelViewInverse3x3Transposed",
        gcSHADER_FRAC_X3,
        3,
        set_uModelViewInverse3x3Transposed,
        &glmUNIFORM_WRAP(VS, uModelViewInverse3x3Transposed)
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_uModelViewProjection(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingUniform(
        ShaderControl->i,
        "uModelViewProjection",
        gcSHADER_FRAC_X4,
        4,
        set_uModelViewProjection,
        &glmUNIFORM_WRAP(VS, uModelViewProjection)
        );
    gcmFOOTER();
    return status;
}
static gceSTATUS using_uEcm(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingUniform(
        ShaderControl->i,
        "uEcm",
        gcSHADER_FRAC_X4,
        2,
        set_uEcm,
        &glmUNIFORM_WRAP(VS, uEcm)
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_uAcm(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingUniform(
        ShaderControl->i,
        "uAcm",
        gcSHADER_FRAC_X4,
        2,
        set_uAcm,
        &glmUNIFORM_WRAP(VS, uAcm)
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_uDcm(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingUniform(
        ShaderControl->i,
        "uDcm",
        gcSHADER_FRAC_X4,
        2,
        set_uDcm,
        &glmUNIFORM_WRAP(VS, uDcm)
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_uAcs(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingUniform(
        ShaderControl->i,
        "uAcs",
        gcSHADER_FRAC_X4,
        2,
        set_uAcs,
        &glmUNIFORM_WRAP(VS, uAcs)
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_uSrm(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingUniform(
        ShaderControl->i,
        "uSrm",
        gcSHADER_FRAC_X1,
        2,
        set_uSrm,
        &glmUNIFORM_WRAP(VS, uSrm)
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_uScm(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingUniform(
        ShaderControl->i,
        "uScm",
        gcSHADER_FRAC_X4,
        2,
        set_uScm,
        &glmUNIFORM_WRAP(VS, uScm)
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_uPpli(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingUniform(
        ShaderControl->i,
        "uPpli",
        gcSHADER_FRAC_X4,
        glvMAX_LIGHTS,
        set_uPpli,
        &glmUNIFORM_WRAP(VS, uPpli)
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_uKi(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingUniform(
        ShaderControl->i,
        "uKi",
        gcSHADER_FRAC_X3,
        glvMAX_LIGHTS,
        set_uKi,
        &glmUNIFORM_WRAP(VS, uKi)
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_uSrli(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingUniform(
        ShaderControl->i,
        "uSrli",
        gcSHADER_FRAC_X1,
        glvMAX_LIGHTS,
        set_uSrli,
        &glmUNIFORM_WRAP(VS, uSrli)
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_uAcli(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingUniform(
        ShaderControl->i,
        "uAcli",
        gcSHADER_FRAC_X4,
        glvMAX_LIGHTS,
        set_uAcli,
        &glmUNIFORM_WRAP(VS, uAcli)
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_uDcli(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingUniform(
        ShaderControl->i,
        "uDcli",
        gcSHADER_FRAC_X4,
        glvMAX_LIGHTS,
        set_uDcli,
        &glmUNIFORM_WRAP(VS, uDcli)
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_uScli(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingUniform(
        ShaderControl->i,
        "uScli",
        gcSHADER_FRAC_X4,
        glvMAX_LIGHTS,
        set_uScli,
        &glmUNIFORM_WRAP(VS, uScli)
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_uAcmAcli(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingUniform(
        ShaderControl->i,
        "uAcmAcli",
        gcSHADER_FRAC_X4,
        glvMAX_LIGHTS,
        set_uAcmAcli,
        &glmUNIFORM_WRAP(VS, uAcmAcli)
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_uAcmAcli2(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingUniform(
        ShaderControl->i,
        "uAcmAcli2",
        gcSHADER_FRAC_X4,
        glvMAX_LIGHTS,
        set_uAcmAcli2,
        &glmUNIFORM_WRAP(VS, uAcmAcli2)
        );
    gcmFOOTER();
    return status;
}


static gceSTATUS using_uVPpli(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingUniform(
        ShaderControl->i,
        "uVPpli",
        gcSHADER_FRAC_X4,
        glvMAX_LIGHTS,
        set_uVPpli,
        &glmUNIFORM_WRAP(VS, uVPpli)
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_uDcmDcli(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingUniform(
        ShaderControl->i,
        "uDcmDcli",
        gcSHADER_FRAC_X4,
        glvMAX_LIGHTS,
        set_uDcmDcli,
        &glmUNIFORM_WRAP(VS, uDcmDcli)
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_uDcmDcli2(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingUniform(
        ShaderControl->i,
        "uDcmDcli2",
        gcSHADER_FRAC_X4,
        glvMAX_LIGHTS,
        set_uDcmDcli2,
        &glmUNIFORM_WRAP(VS, uDcmDcli2)
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_uCrli(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingUniform(
        ShaderControl->i,
        "uCrli",
        gcSHADER_FRAC_X1,
        glvMAX_LIGHTS,
        set_uCrli,
        &glmUNIFORM_WRAP(VS, uCrli)
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_uCosCrli(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingUniform(
        ShaderControl->i,
        "uCosCrli",
        gcSHADER_FRAC_X1,
        glvMAX_LIGHTS,
        set_uCosCrli,
        &glmUNIFORM_WRAP(VS, uCosCrli)
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_uNormedSdli(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingUniform(
        ShaderControl->i,
        "uNormedSdli",
        gcSHADER_FRAC_X4,
        glvMAX_LIGHTS,
        set_uNormedSdli,
        &glmUNIFORM_WRAP(VS, uNormedSdli)
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_uTexCoord(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsing_uTexCoord(
        ShaderControl->i,
        &glmUNIFORM_WRAP(VS, uTexCoord)
        );
    gcmFOOTER();
    return status;
}

extern gceSTATUS glfUsing_uTexGenObjectPlane(
    IN glsSHADERCONTROL_PTR ShaderControl,
    OUT glsUNIFORMWRAP_PTR* UniformWrap
    );

static gceSTATUS using_uTexGenObjectPlane(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsing_uTexGenObjectPlane(
        ShaderControl->i,
        &glmUNIFORM_WRAP(VS, uTexGenObjectPlane)
        );
    gcmFOOTER();
    return status;
}

extern gceSTATUS glfUsing_uTexGenEyePlane(
    IN glsSHADERCONTROL_PTR ShaderControl,
    OUT glsUNIFORMWRAP_PTR* UniformWrap
    );

static gceSTATUS using_uTexGenEyePlane(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsing_uTexGenEyePlane(
        ShaderControl->i,
        &glmUNIFORM_WRAP(VS, uTexGenEyePlane)
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_uTexMatrix(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingUniform(
        ShaderControl->i,
        "uTexMatrix",
        gcSHADER_FRAC_X4,
        4 * gc->constants.numberOfTextureUnits,
        set_uTexMatrix,
        &glmUNIFORM_WRAP(VS, uTexMatrix)
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_uClipPlane(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingUniform(
        ShaderControl->i,
        "uClipPlane",
        gcSHADER_FRAC_X4,
        glvMAX_CLIP_PLANES,
        set_uClipPlane,
        &glmUNIFORM_WRAP(VS, uClipPlane)
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_uPointAttenuation(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingUniform(
        ShaderControl->i,
        "uPointAttenuation",
        gcSHADER_FRAC_X3,
        1,
        set_uPointAttenuation,
        &glmUNIFORM_WRAP(VS, uPointAttenuation)
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_uPointSize(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingUniform(
        ShaderControl->i,
        "#PointSize",
        gcSHADER_FRAC_X4,
        1,
        set_uPointSize,
        &glmUNIFORM_WRAP(VS, uPointSize)
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_uViewport(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingUniform(
        ShaderControl->i,
        "uViewport",
        gcSHADER_FRAC_X4,
        1,
        set_uViewport,
        &glmUNIFORM_WRAP(VS, uViewport)
        );
    gcmFOOTER();
    return status;
}


/*******************************************************************************
** Attribute access helpers.
*/

static gceSTATUS using_aPosition(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    glsATTRIBUTEINFO_PTR info;
    gctINT binding;
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    /* Define a shortcut to the attribute descriptor. */
    if (chipCtx->drawClearRectEnabled)
    {
        info = &chipCtx->attributeInfo[gldATTRIBUTE_DRAWCLEAR_POSITION];
        binding = gldATTRIBUTE_DRAWCLEAR_POSITION;
    }
    else if (chipCtx->drawTexOESEnabled)
    {
        info = &chipCtx->attributeInfo[gldATTRIBUTE_DRAWTEX_POSITION];
        binding = gldATTRIBUTE_DRAWTEX_POSITION;
    }
    else
    {
        info = &chipCtx->attributeInfo[__GL_INPUT_VERTEX_INDEX];
        binding = __GL_INPUT_VERTEX_INDEX;
    }

    status = glfUsingAttribute(
        ShaderControl->i,
        "aPosition",
        info->attributeType,
        1,
        gcvFALSE,
        info,
        &glmATTRIBUTE_WRAP(VS, aPosition),
        binding,
        gcSHADER_SHADER_DEFAULT
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_aNormal(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    glsATTRIBUTEINFO_PTR info = &chipCtx->attributeInfo[__GL_INPUT_NORMAL_INDEX];

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingAttribute(
        ShaderControl->i,
        "aNormal",
        info->attributeType,
        1,
        gcvFALSE,
        info,
        &glmATTRIBUTE_WRAP(VS, aNormal),
        __GL_INPUT_NORMAL_INDEX,
        gcSHADER_SHADER_DEFAULT
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_aColor(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    glsATTRIBUTEINFO_PTR info = &chipCtx->attributeInfo[__GL_INPUT_DIFFUSE_INDEX];

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingAttribute(
        ShaderControl->i,
        "aColor",
        info->attributeType,
        1,
        gcvFALSE,
        info,
        &glmATTRIBUTE_WRAP(VS, aColor),
        __GL_INPUT_DIFFUSE_INDEX,
        gcSHADER_SHADER_DEFAULT
        );
    gcmFOOTER();
    return status;
}



static gceSTATUS using_aTexCoord(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl,
    gctINT Sampler
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    static gctCONST_STRING aName[] =
    {
        "aTexCoord0",
        "aTexCoord1",
        "aTexCoord2",
        "aTexCoord3",
        "aTexCoord4",
        "aTexCoord5",
        "aTexCoord6",
        "aTexCoord7"
    };

    /* Make a shortcut to the attribute descriptor. */
    glsATTRIBUTEINFO_PTR info =  &chipCtx->attributeInfo[Sampler + __GL_INPUT_TEX0_INDEX];

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x Sampler=%d", gc, ShaderControl, Sampler);

    status = glfUsingAttribute(
        ShaderControl->i,
        aName[Sampler],
        info->attributeType,
        1,
        gcvTRUE,
        info,
        &glmATTRIBUTE_WRAP_INDEXED(VS, aTexCoord0, Sampler),
        __GL_INPUT_TEX0_INDEX + Sampler,
        gcSHADER_SHADER_DEFAULT
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_aPointSize(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    glsATTRIBUTEINFO_PTR info = &chipCtx->attributeInfo[gldATTRIBUTE_POINT_SIZE];
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    chipCtx = CHIP_CTXINFO(gc);

    status = glfUsingAttribute(
        ShaderControl->i,
        "aPointSize",
        info->attributeType,
        1,
        gcvFALSE,
        info,
        &glmATTRIBUTE_WRAP(VS, aPointSize),
        gldATTRIBUTE_POINT_SIZE,
        gcSHADER_SHADER_DEFAULT
        );
    gcmFOOTER();
    return status;
}

/*******************************************************************************
** Output access helpers.
*/

static gceSTATUS assign_vPosition(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl,
    gctUINT16 TempRegister
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x TempRegister=%u", gc, ShaderControl, TempRegister);
    status = gcSHADER_AddOutput(
        ShaderControl->i->shader,
        "#Position",
        gcSHADER_FLOAT_X4,
        1,
        TempRegister,
        gcSHADER_PRECISION_DEFAULT
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS assign_vEyePosition(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl,
    gctUINT16 TempRegister
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x TempRegister=%u", ShaderControl, TempRegister);
    status = gcSHADER_AddOutput(
        ShaderControl->i->shader,
        "#FogFragCoord",
        gcSHADER_FLOAT_X1,
        1,
        TempRegister,
        gcSHADER_PRECISION_DEFAULT
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS assign_vColor(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl,
    gctINT OutputIndex
    )
{
    static gctCONST_STRING vName[] =
    {
        "#FrontColor",
        "#BackColor"
    };
    gcSHADER_SHADERMODE shadingMode = gcSHADER_SHADER_DEFAULT;
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;

    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x OutputIndex=%d", gc, ShaderControl, OutputIndex);

    if (chipCtx->hashKey.hashShadingMode == 1)
        shadingMode = gcSHADER_SHADER_FLAT;

    status = gcSHADER_AddOutputEx(
        ShaderControl->i->shader,
        vName[OutputIndex],
        gcSHADER_FLOAT_X4,
        gcSHADER_PRECISION_DEFAULT,
        gcvFALSE,
        1,
        ShaderControl->vColor[OutputIndex],
        -1,
        gcvFALSE,
        gcvFALSE,
        shadingMode,
        gcvNULL
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS assign_vColor2(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl,
    gctINT OutputIndex
    )
{
    static gctCONST_STRING vName[] =
    {
        "#FrontSecondaryColor",
        "#BackSecondaryColor"
    };
    gcSHADER_SHADERMODE shadingMode = gcSHADER_SHADER_DEFAULT;
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;

    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x OutputIndex=%d", gc, ShaderControl, OutputIndex);

    if (chipCtx->hashKey.hashShadingMode == 1)
        shadingMode = gcSHADER_SHADER_FLAT;

    status = gcSHADER_AddOutputEx(
        ShaderControl->i->shader,
        vName[OutputIndex],
        gcSHADER_FLOAT_X4,
        gcSHADER_PRECISION_DEFAULT,
        gcvFALSE,
        1,
        ShaderControl->vColor2[OutputIndex],
        -1,
        gcvFALSE,
        gcvFALSE,
        shadingMode,
        gcvNULL
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS assign_vTexCoord(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl,
    gctINT Sampler
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    static gctCONST_STRING vName[] =
    {
        "#TexCoord0",
        "#TexCoord1",
        "#TexCoord2",
        "#TexCoord3",
        "#TexCoord4",
        "#TexCoord5",
        "#TexCoord6",
        "#TexCoord7"
    };
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x Sampler=%d", gc, ShaderControl, Sampler);

    status = gcSHADER_AddOutput(
        ShaderControl->i->shader,
        vName[Sampler],
        chipCtx->texture.sampler[Sampler].coordType,
        1,
        ShaderControl->vTexCoord[Sampler],
        gcSHADER_PRECISION_DEFAULT
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS assign_vClipPlane(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl,
    gctINT ClipPlane
    )
{
    static gctCONST_STRING vName[] =
    {
        "#ClipPlane0",
        "#ClipPlane1",
        "#ClipPlane2",
        "#ClipPlane3",
        "#ClipPlane4",
        "#ClipPlane5"
    };
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x ClipPlane=%d", gc, ShaderControl, ClipPlane);

    status = gcSHADER_AddOutput(
        ShaderControl->i->shader,
        vName[ClipPlane],
        gcSHADER_FLOAT_X1,
        1,
        ShaderControl->vClipPlane[ClipPlane],
        gcSHADER_PRECISION_DEFAULT
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS assign_vPointSize(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl,
    gctUINT16 TempRegister
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x TempRegister=%u", gc, ShaderControl, TempRegister);

    status = gcSHADER_AddOutput(
        ShaderControl->i->shader,
        "#PointSize",
        gcSHADER_FLOAT_X1,
        1,
        TempRegister,
        gcSHADER_PRECISION_DEFAULT
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS assign_vPointFade(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl,
    gctUINT16 TempRegister
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x TempRegister=%u", gc, ShaderControl, TempRegister);

    status = gcSHADER_AddOutput(
        ShaderControl->i->shader,
        "vPointFade",
        gcSHADER_FLOAT_X1,
        1,
        TempRegister,
        gcSHADER_PRECISION_DEFAULT
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS assign_vPointSmooth(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl,
    gctUINT16 TempRegister
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x TempRegister=%u", gc, ShaderControl, TempRegister);

    status = gcSHADER_AddOutput(
        ShaderControl->i->shader,
        "vPointSmooth",
        gcSHADER_FLOAT_X3,
        1,
        TempRegister,
        gcSHADER_PRECISION_DEFAULT
        );

    gcmFOOTER();
    return status;
}


/*******************************************************************************
*   Check special case to expand VS shader to compute lighting
*   even if light source number greater than 4.
*/
static gctBOOL checkUseLightingFunction(
    __GLcontext * gc
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    GLuint   lightEnable, i = 0;
    gctBOOL bUseFunction = gcvFALSE;
    GLuint lightCount = 0;

    gcmHEADER_ARG("gc=0x%x", gc);

    if (chipCtx->lightingStates.prevLightEnabled != chipCtx->lightingStates.lightEnabled) {
        lightEnable = chipCtx->lightingStates.lightEnabled;
        while ((i < glvMAX_LIGHTS) && lightEnable)
        {
            if (lightEnable & 1) {
                lightCount++;
            }
            i++;
            lightEnable >>= 1;
        }
        chipCtx->lightingStates.useFunction = (lightCount > 4);
    }

    lightEnable = chipCtx->lightingStates.lightEnabled;
    bUseFunction = chipCtx->lightingStates.useFunction;

    gcmFOOTER_ARG("%d", bUseFunction);
    return bUseFunction;
}


/*******************************************************************************
**
**  _Pos2Eye
**
**  Transform the incoming position to the eye space coordinates using matrix
**  palette.
**
**  INPUT:
**
**      gc
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS pos2Eye(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;

    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    do
    {
        /* Already computed? */
        if (ShaderControl->rVtxInEyeSpace != 0)
        {
            status = gcvSTATUS_OK;
            break;
        }

        {
            /* Allocate a temporary for the vertex in eye space. */
            glmALLOCATE_TEMP(ShaderControl->rVtxInEyeSpace);

            /* Allocate position attribute. */
            glmUSING_ATTRIBUTE(aPosition);

            if (gc->transform.modelView->matrix.matrixType == __GL_MT_IDENTITY)
            {
                /* rVtxInEyeSpace = aPosition */
                glmOPCODE(MOV, ShaderControl->rVtxInEyeSpace, XYZW);
                    glmATTRIBUTE(VS, aPosition, XYZW);
            }
            else
            {
                /* Allocate resources. */
                glmUSING_UNIFORM(uModelView);

                /* dp4 rVtxInEyeSpace.x, aPosition, uModelView[0] */
                glmOPCODE(DP4, ShaderControl->rVtxInEyeSpace, X);
                    glmATTRIBUTE(VS, aPosition, XYZW);
                    glmUNIFORM_STATIC(VS, uModelView, XYZW, 0);

                /* dp4 rVtxInEyeSpace.y, aPosition, uModelView[1] */
                glmOPCODE(DP4, ShaderControl->rVtxInEyeSpace, Y);
                    glmATTRIBUTE(VS, aPosition, XYZW);
                    glmUNIFORM_STATIC(VS, uModelView, XYZW, 1);

                /* dp4 rVtxInEyeSpace.z, aPosition, uModelView[2] */
                glmOPCODE(DP4, ShaderControl->rVtxInEyeSpace, Z);
                    glmATTRIBUTE(VS, aPosition, XYZW);
                    glmUNIFORM_STATIC(VS, uModelView, XYZW, 2);

                /* dp4 rVtxInEyeSpace.w, aPosition, uModelView[3] */
                glmOPCODE(DP4, ShaderControl->rVtxInEyeSpace, W);
                    glmATTRIBUTE(VS, aPosition, XYZW);
                    glmUNIFORM_STATIC(VS, uModelView, XYZW, 3);
            }
        }
    }
    while (gcvFALSE);
    gcmFOOTER();
    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _Pos2ClipWithPalette
**
**  Transform the incoming position to the clip coordinate space using matrix
**  palette.
**
**  INPUT:
**
**      gc
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**  OUTPUT:
**
**      Nothing.
*/

/*******************************************************************************
**
**  _Pos2ClipWithModelViewProjection
**
**  Transform the incoming position to the clip coordinate space using the
**  product of model view and projection matrices.
**
**  INPUT:
**
**      gc
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS pos2ClipWithModelViewProjection(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    do
    {
        /* Already computed? */
        if (ShaderControl->vPosition != 0)
        {
            status = gcvSTATUS_OK;
            break;
        }

        /* Allocate a temporary for the position output. */
        glmALLOCATE_TEMP(ShaderControl->vPosition);

        /* Allocate position attribute. */
        glmUSING_ATTRIBUTE(aPosition);

        /* Allocate projection matrix. */
        glmUSING_UNIFORM(uModelViewProjection);

        /* dp4 vPosition.x, aPosition, uModelViewProjection[0] */
        glmOPCODE(DP4, ShaderControl->vPosition, X);
            glmATTRIBUTE(VS, aPosition, XYZW);
            glmUNIFORM_STATIC(VS, uModelViewProjection, XYZW, 0);

        /* dp4 vPosition.y, aPosition, uModelViewProjection[1] */
        glmOPCODE(DP4, ShaderControl->vPosition, Y);
            glmATTRIBUTE(VS, aPosition, XYZW);
            glmUNIFORM_STATIC(VS, uModelViewProjection, XYZW, 1);

        /* dp4 vPosition.z, aPosition, uModelViewProjection[2] */
        glmOPCODE(DP4, ShaderControl->vPosition, Z);
            glmATTRIBUTE(VS, aPosition, XYZW);
            glmUNIFORM_STATIC(VS, uModelViewProjection, XYZW, 2);

        /* dp4 vPosition.w, aPosition, uModelViewProjection[3] */
        glmOPCODE(DP4, ShaderControl->vPosition, W);
            glmATTRIBUTE(VS, aPosition, XYZW);
            glmUNIFORM_STATIC(VS, uModelViewProjection, XYZW, 3);
    }
    while (gcvFALSE);

    gcmFOOTER();
    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _Pos2Clip
**
**  Transform the incoming position to the clip coordinate space.
**
**  INPUT:
**
**      gc
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS pos2Clip(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    do
    {
        /* Already computed? */
        if (ShaderControl->vPosition != 0)
        {
            status = gcvSTATUS_OK;
            break;
        }

        if (chipCtx->drawTexOESEnabled || chipCtx->drawClearRectEnabled)
        {
            /* Allocate a temporary for the position output. */
            glmALLOCATE_TEMP(ShaderControl->vPosition);

            /* Allocate position attribute. */
            glmUSING_ATTRIBUTE(aPosition);

            /* vPosition = aPosition */
            glmOPCODE(MOV, ShaderControl->vPosition, XYZW);
                glmATTRIBUTE(VS, aPosition, XYZW);
        }
        else
        {
            status = pos2ClipWithModelViewProjection(gc, ShaderControl);
        }
    }
    while (gcvFALSE);

    gcmFOOTER();
    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _Normal2EyeWithPalette
**
**  Transform the incoming normal to the eye space coordinates using matrix
**  palette.
**
**  INPUT:
**
**      gc
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**  OUTPUT:
**
**      Nothing.
*/

/*******************************************************************************
**
**  _Normal2EyeWithModelViewInv
**
*   Transform normal to eye space using model-view inverse matrix.
**
**  INPUT:
**
**      gc
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS normal2EyeWithModelViewInv(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    do
    {
        /* Allocate a temp. */
        glmALLOCATE_LOCAL_TEMP(temp);

        /* Normal comes from the stream? */
        if (chipCtx->attributeInfo[__GL_INPUT_NORMAL_INDEX].streamEnabled)
        {
            /* Allocate normal attribute. */
            glmUSING_ATTRIBUTE(aNormal);

            /* temp = aNormal */
            glmOPCODE(MOV, temp, XYZ);
                glmATTRIBUTE(VS, aNormal, XYZZ);
        }

        /* Normal comes from the constant. */
        else
        {
            /* Allocate normal uniform. */
            glmUSING_UNIFORM(uNormal);

            /* temp = uNormal */
            glmOPCODE(MOV, temp, XYZ);
                glmUNIFORM(VS, uNormal, XYZZ);
        }

        if (gc->transform.modelView->inverseTranspose.matrixType == __GL_MT_IDENTITY)
        {
            /* Set the result. */
            ShaderControl->rNrmInEyeSpace[0] = temp;
        }
        else
        {
            /* Allocate the result. */
            glmALLOCATE_TEMP(ShaderControl->rNrmInEyeSpace[0]);

            /* Allocate matrix uniform. */
            glmUSING_UNIFORM(uModelViewInverse3x3Transposed);

            /* rNrmInEyeSpace[0].x = dot(temp, uModelViewInverse3x3Transposed[0]) */
            glmOPCODE(DP3, ShaderControl->rNrmInEyeSpace[0], X);
                glmTEMP(temp, XYZZ);
                glmUNIFORM_STATIC(VS, uModelViewInverse3x3Transposed, XYZZ, 0);

            /* rNrmInEyeSpace[0].y = dot(temp, uModelViewInverse3x3Transposed[1]) */
            glmOPCODE(DP3, ShaderControl->rNrmInEyeSpace[0], Y);
                glmTEMP(temp, XYZZ);
                glmUNIFORM_STATIC(VS, uModelViewInverse3x3Transposed, XYZZ, 1);

            /* rNrmInEyeSpace[0].z = dot(temp, uModelViewInverse3x3Transposed[2]) */
            glmOPCODE(DP3, ShaderControl->rNrmInEyeSpace[0], Z);
                glmTEMP(temp, XYZZ);
                glmUNIFORM_STATIC(VS, uModelViewInverse3x3Transposed, XYZZ, 2);
        }
    }
    while (gcvFALSE);

    gcmFOOTER();
    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _Normal2Eye
**
**  Transform normal to eye space.
**
**  INPUT:
**
**      gc
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS normal2Eye(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    do
    {
        /* Already computed? */
        if (ShaderControl->rNrmInEyeSpace[0])
        {
            status = gcvSTATUS_OK;
            break;
        }

        /* Matrix palette enabled? */
        {
            gcmERR_BREAK(normal2EyeWithModelViewInv(gc, ShaderControl));
        }

        /* Rescale normal. */
        if ((gc->state.enables.transform.rescaleNormal) &&
            (gc->transform.modelView->inverseTranspose.matrixType != __GL_MT_IDENTITY))
        {
            /* Allocate temporaries. */
            glmALLOCATE_LOCAL_TEMP(temp1);
            glmALLOCATE_LOCAL_TEMP(temp2);

            /* Save current normal value. */
            glmDEFINE_TEMP(prevNrmInEyeSpace) = ShaderControl->rNrmInEyeSpace[0];

            /* Allocate new normal register. */
            glmALLOCATE_TEMP(ShaderControl->rNrmInEyeSpace[0]);

            /* Allocate resources. */
            glmUSING_UNIFORM(uModelViewInverse3x3Transposed);

            /* temp1.x = m31^2 + m32^2 + m33^2 */
            glmOPCODE(DP3, temp1, X);
            glmUNIFORM_STATIC(VS, uModelViewInverse3x3Transposed, XYZZ, 2);
            glmUNIFORM_STATIC(VS, uModelViewInverse3x3Transposed, XYZZ, 2);

            /* temp2.x = scale factor */
            glmOPCODE(RSQ, temp2, X);
                glmTEMP(temp1, XXXX);

            /* Rescale the normal. */
            glmOPCODE(MUL, ShaderControl->rNrmInEyeSpace[0], XYZ);
                glmTEMP(prevNrmInEyeSpace, XYZZ);
                glmTEMP(temp2, XXXX);
        }

        /* Normalize the normal. */
        if (gc->state.enables.transform.normalize)
        {
            /* Save current normal value. */
            glmDEFINE_TEMP(prevNrmInEyeSpace) = ShaderControl->rNrmInEyeSpace[0];

            /* Allocate new normal register. */
            glmALLOCATE_LOCAL_TEMP(temp1);
            glmALLOCATE_LOCAL_LABEL(lblZero);

            /* rNrmInEyeSpace[0] = norm(rNrmInEyeSpace[0]) */
            glmOPCODE(MOV, temp1, XYZW);
                glmTEMP(ShaderControl->rNrmInEyeSpace[0], XYZW);

            glmOPCODE_BRANCH(JMP, EQUAL, lblZero);
                glmTEMP(prevNrmInEyeSpace, XYZZ);
                glmCONST(0);
            {
                /* rNrmInEyeSpace[0] = norm(rNrmInEyeSpace[0]) */
                glmOPCODE(NORM, ShaderControl->rNrmInEyeSpace[0], XYZ);
                    glmTEMP(temp1, XYZZ);
            }

            glmLABEL(lblZero);
        }

        /* Compute the negated normal for two-sided lighting. */
        if (ShaderControl->outputCount == 2)
        {
            glmALLOCATE_TEMP(ShaderControl->rNrmInEyeSpace[1]);

            /* Compute the reversed normal. */
            glmOPCODE(SUB, ShaderControl->rNrmInEyeSpace[1], XYZ);
                glmCONST(0);
                glmTEMP(ShaderControl->rNrmInEyeSpace[0], XYZZ);
        }
    }
    while (gcvFALSE);

    gcmFOOTER();
    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _ComputePointSize
**
**  Compute point size. Per OpenGL ES spec:
**
**                                     1
**  derived_size' = size * ------------------------- = size * scale_factor
**                         SQRT(a + b * d + c * d^2)
**
**  derived_size = clamp(derived_size')
**
**  where a, b, and c - distance attenuation function coefficients;
**        d           - eye-coordinate distance from the eye to the vertex;
**
**  INPUT:
**
**      gc
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS computePointSize(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    do
    {
        /* Define a label. */
        glmDEFINE_LABEL(lblZero);

        /* Define temporaries. */
        glmDEFINE_TEMP(temp1);
        glmDEFINE_TEMP(temp2);
        glmDEFINE_TEMP(temp3);
        glmDEFINE_TEMP(temp4);
        glmDEFINE_TEMP(temp5);
        glmDEFINE_TEMP(temp6);

        /* Allocate the label. */
        glmALLOCATE_LABEL(lblZero);

        /* Compute current position in eye space. */
        gcmERR_BREAK(pos2Eye(gc, ShaderControl));

        /* Allocate temporaries. */
        glmALLOCATE_TEMP(temp1);
        glmALLOCATE_TEMP(temp2);
        glmALLOCATE_TEMP(temp3);
        glmALLOCATE_TEMP(temp4);
        glmALLOCATE_TEMP(temp5);
        glmALLOCATE_TEMP(temp6);

        /* Allocate resources. */
        glmUSING_UNIFORM(uPointAttenuation);
        glmUSING_UNIFORM(uPointSize);

        /* temp1 = (1, d, d^2) */
        {
            /* temp1.z = d^2 */
            glmOPCODE(DP3, temp1, YZ);
                glmTEMP(ShaderControl->rVtxInEyeSpace, XYZZ);
                glmTEMP(ShaderControl->rVtxInEyeSpace, XYZZ);

            /* Make sure Z is not zero, otherwize RSQ will produce an INF. */
            glmOPCODE_BRANCH(JMP, EQUAL, lblZero);
                glmTEMP(temp1, ZZZZ);
                glmCONST(0);

            {
                /* temp2.y = 1 / sqrt(d^2) */
                glmOPCODE(RSQ, temp2, Y);
                    glmTEMP(temp1, ZZZZ);

                /* temp2.z = temp1.z */
                glmOPCODE(MOV, temp2, Z);
                    glmTEMP(temp1, ZZZZ);

                /* temp1.y = d */
                glmOPCODE(MUL, temp1, Y);
                    glmTEMP(temp2, YYYY);
                    glmTEMP(temp2, ZZZZ);
            }

            /* Define label. */
            glmLABEL(lblZero);

            /* temp1.x = 1 */
            glmOPCODE(MOV, temp1, X);
                glmCONST(1);
        }

        /* temp2.x = a + b * d + c * d^2 */
        glmOPCODE(DP3, temp2, X);
            glmUNIFORM(VS, uPointAttenuation, XYZZ);
            glmTEMP(temp1, XYZZ);

        /* temp3.x = scale_factor */
        glmOPCODE(RSQ, temp3, X);
            glmTEMP(temp2, XXXX);

        /* Point size is comming from the stream. */
        if (chipCtx->attributeInfo[gldATTRIBUTE_POINT_SIZE].streamEnabled)
        {
            glmUSING_ATTRIBUTE(aPointSize);

            /* temp4.x = derived_size' */
            glmOPCODE(MUL, temp4, X);
                glmATTRIBUTE(VS, aPointSize, XXXX);
                glmTEMP(temp3, XXXX);
        }

        /* Point size is a constant value. */
        else
        {
            /* temp4.x = derived_size' */
            glmOPCODE(MUL, temp4, X);
                glmUNIFORM(VS, uPointSize, XXXX);
                glmTEMP(temp3, XXXX);
        }

        /* Clamp by the lower boundary (uPointSize.y). */
        glmOPCODE(MAX, temp5, X);
            glmUNIFORM(VS, uPointSize, YYYY);
            glmTEMP(temp4, XXXX);

        /* Clamp by the upper boundary (uPointSize.z) = derived_size. */
        glmOPCODE(MIN, temp6, X);
            glmUNIFORM(VS, uPointSize, ZZZZ);
            glmTEMP(temp5, XXXX);

        /* Special patch for GC500 with super-sampling enabled:
           multiply point size by 2. */
        if (chipCtx->chipModel == gcv500)
        {
            gctUINT samples;

            if (gcmIS_SUCCESS(gcoSURF_GetSamples(chipCtx->drawRT[0], &samples))
            &&  (samples > 1)
            )
            {
                /* temp6 *= 2 */
                glmOPCODE(MUL, temp6, X);
                    glmTEMP(temp6, XXXX);
                    glmCONST(2);
            }
        }

        if (gc->state.enables.multisample.multisampleOn)
        {
            /* Allocate temporaries. */
            glmALLOCATE_LOCAL_TEMP(temp7);
            glmALLOCATE_LOCAL_TEMP(temp8);
            glmALLOCATE_LOCAL_LABEL(lblDontFade);

            /* Allocate varyings. */
            glmALLOCATE_TEMP(ShaderControl->vPointSize);
            glmALLOCATE_TEMP(ShaderControl->vPointFade);

            /* Compare against the threshold. */
            glmOPCODE(MAX, ShaderControl->vPointSize, X);
                glmUNIFORM(VS, uPointSize, WWWW);
                glmTEMP(temp6, XXXX);

            /* Initialize fade factor to 1. */
            glmOPCODE(MOV, ShaderControl->vPointFade, X);
                glmCONST(1);

            /* if (fadeThrdshold <= derived_size) goto lblDontFade. */
            glmOPCODE_BRANCH(JMP, LESS_OR_EQUAL, lblDontFade);
                glmUNIFORM(VS, uPointSize, WWWW);
                glmTEMP(temp6, XXXX);

            {
                /* temp7.x = 1 / fadeThrdshold. */
                glmOPCODE(RCP, temp7, X);
                    glmUNIFORM(VS, uPointSize, WWWW);

                /* temp8.x = derived_size / fadeThrdshold. */
                glmOPCODE(MUL, temp8, X);
                    glmTEMP(temp6, XXXX);
                    glmTEMP(temp7, XXXX);

                /* vPointFade.x = (derived_size / fadeThrdshold) ^ 2. */
                glmOPCODE(MUL, ShaderControl->vPointFade, X);
                    glmTEMP(temp8, XXXX);
                    glmTEMP(temp8, XXXX);
            }

            /* Define label. */
            glmLABEL(lblDontFade);
        }
        else
        {
            /* Assign point size varying. */
            ShaderControl->vPointSize = temp6;
        }
    }
    while (gcvFALSE);

    gcmFOOTER();
    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _ComputePointSmooth
**
**  Compute point smooth.
**
**  INPUT:
**
**      gc
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS computePointSmooth(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    do
    {
        /* Allocate temporaries. */
        glmALLOCATE_LOCAL_TEMP(temp1);
        glmALLOCATE_LOCAL_TEMP(temp2);
        glmALLOCATE_LOCAL_TEMP(temp3);

        glmALLOCATE_LOCAL_TEMP(temp4);
        glmALLOCATE_LOCAL_TEMP(temp5);
        /* Allocate point smooth varying. */
        glmALLOCATE_TEMP(ShaderControl->vPointSmooth);

        /* Allocate resources. */
        glmUSING_UNIFORM(uViewport);

        /* Do W correction for position. */
        glmOPCODE(RCP, temp1, X);
            glmTEMP(ShaderControl->vPosition, WWWW);

        glmOPCODE(MUL, temp2, XY);
            glmTEMP(temp1, XXXX);
            glmTEMP(ShaderControl->vPosition, XYYY);

        /* PointScreenX (vPointSmooth.x)
              = PositionX * ViewportScaleX + ViewportOriginX */
        glmOPCODE(MUL, temp3, X);
            glmTEMP(temp2, XXXX);
            glmUNIFORM(VS, uViewport, XXXX);

        glmOPCODE(ADD, ShaderControl->vPointSmooth, X);
            glmTEMP(temp3, XXXX);
            glmUNIFORM(VS, uViewport, YYYY);

        /* PointScreenY (vPointSmooth.y)
              = PositionY * ViewportScaleY + ViewportOriginY */
        glmOPCODE(MUL, temp4, X);
            glmTEMP(temp2, YYYY);
            glmUNIFORM(VS, uViewport, ZZZZ);

        glmOPCODE(ADD, ShaderControl->vPointSmooth, Y);
            glmTEMP(temp4, XXXX);
            glmUNIFORM(VS, uViewport, WWWW);

        /* temp3.x = the radius of the point. */
        glmOPCODE(MUL, temp5, X);
            glmTEMP(ShaderControl->vPointSize, XXXX);
            glmCONST(0.5f);

        /* vPointSmooth.z = radius^2. */
        glmOPCODE(MUL, ShaderControl->vPointSmooth, Z);
            glmTEMP(temp5, XXXX);
            glmTEMP(temp5, XXXX);
    }
    while (gcvFALSE);

    gcmFOOTER();
    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _lightGeneral
**
**  Generate general lighting component of the lighting formula: object emission
**  and general ambient lighting. Per OpenGL ES spec:
**
**      c = Ecm + Acm * Acs,
**
**  where Ecm - emissive color of material;
**        Acm - ambient color of material;
**        Acs - ambient color of scene.
**
**  INPUT:
**
**      gc
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
*/

static gceSTATUS lightGeneral(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl,
    GLuint OutputIndex
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    do
    {
        /***************************************************************************
        ** Per OpenGL spec:
        **   When color material tracking is enabled, by glEnable with
        **   COLOR_MATERIAL, the ambient (Acm) property of both the front and back
        **   material are immediately set to the value of the current color, and
        **   will track changes to the current color resulting from either the
        **   glColor commands or drawing vertex arrays with the color array enabled.
        */

        /* Allocate function result register. */
        glmALLOCATE_TEMP(ShaderControl->rLighting[OutputIndex]);
        glmALLOCATE_TEMP(ShaderControl->rLighting2[OutputIndex]);

        /* Initialize specular color */
        glmOPCODE(MOV, ShaderControl->rLighting2[OutputIndex], XYZ);
                 glmCONST(0);
        glmOPCODE(MOV, ShaderControl->rLighting2[OutputIndex], W);
                 glmCONST(1);
        /***********************************************************************
        ** BLOCK1: Acs == 0
        */
        if (chipCtx->hashKey.hashZeroAcs)
        {
            /*******************************************************************
            ** BLOCK2: Ecm == 0, (Acs == 0)
            */
            if (colorMaterialEnabled(gc, OutputIndex, GL_EMISSION) &&
                (chipCtx->attributeInfo[__GL_INPUT_DIFFUSE_INDEX].streamEnabled))
            {
                /* Allocate resources. */
                glmUSING_ATTRIBUTE(aColor);

                /* rLighting[0] = aColor */
                glmOPCODE(MOV, ShaderControl->rLighting[OutputIndex], XYZ);
                    glmATTRIBUTE(VS, aColor, XYZZ);
            } else {
                if (chipCtx->hashKey.hashZeroEcm & (1 << OutputIndex))
                {
                    /* rLighting[0] = 0 */
                    glmOPCODE(MOV, ShaderControl->rLighting[OutputIndex], XYZ);
                        glmCONST(0);
                }
                /*
                ** BLOCK2: Ecm == 0, (Acs == 0)
                *******************************************************************/

                /*******************************************************************
                ** BLOCK3: Ecm != 0, (Acs == 0)
                */
                else
                {
                    /* Allocate resources. */
                    glmUSING_UNIFORM(uEcm);

                    /* rLighting[0] = Ecm */
                    glmOPCODE(MOV, ShaderControl->rLighting[OutputIndex], XYZ);
                        glmLIGHTING_UNIFORM(VS, uEcm, XYZZ, OutputIndex);
                }
            }
            /*
                ** BLOCK3: Ecm != 0, (Acs == 0)
                *******************************************************************/
        }
        /*
        ** BLOCK1: Acs == 0
        ***********************************************************************/

        /***********************************************************************
        ** BLOCK4: Acs != 0
        */
        else
        {
            /*******************************************************************
            ** BLOCK5: Ecm == 0, (Acs != 0)
            */
            if (chipCtx->hashKey.hashZeroEcm & (1 << OutputIndex))
            {
                /***************************************************************
                ** BLOCK6: Acm = Color, ((Ecm == 0) and (Acs != 0)).
                */
                if (colorMaterialEnabled(gc, OutputIndex, GL_AMBIENT) ||
                    colorMaterialEnabled(gc, OutputIndex, GL_AMBIENT_AND_DIFFUSE))
                {
                    /* Color from stream? */
                    if (chipCtx->attributeInfo[__GL_INPUT_DIFFUSE_INDEX].streamEnabled)
                    {
                        /* Allocate resources. */
                        glmUSING_UNIFORM(uAcs);
                        glmUSING_ATTRIBUTE(aColor);

                        /* rLighting[0] = aColor * Acs */
                        glmOPCODE(MUL, ShaderControl->rLighting[OutputIndex], XYZ);
                            glmATTRIBUTE(VS, aColor, XYZZ);
                            glmUNIFORM(VS, uAcs, XYZZ);
                    }

                    /* Color from a constant. */
                    else
                    {
                        /* Color == 0 ? */
                        if (chipCtx->hashKey.hashZeroAcm & (1 << OutputIndex))
                        {
                            /* rLighting[0] = 0 */
                            glmOPCODE(MOV, ShaderControl->rLighting[OutputIndex], XYZ);
                                glmCONST(0);
                        }
                        else
                        {
                            /* Allocate a temp. */
                            glmALLOCATE_LOCAL_TEMP(temp);

                            /* Allocate resources. */
                            glmUSING_UNIFORM(uAcs);
                            glmUSING_UNIFORM(uColor);

                            /* temp = uColor */
                            glmOPCODE(MOV, temp, XYZ);
                                glmUNIFORM(VS, uColor, XYZZ);

                            /* rLighting[0] = temp * Acs */
                            glmOPCODE(MUL, ShaderControl->rLighting[OutputIndex], XYZ);
                                glmTEMP(temp, XYZZ);
                                glmUNIFORM(VS, uAcs, XYZZ);
                        }
                    }
                }
                /*
                ** BLOCK6: Acm = Color, ((Ecm == 0) and (Acs != 0)).
                ***************************************************************/

                /***************************************************************
                ** BLOCK7: Acm = as is, ((Ecm == 0) and (Acs != 0)).
                */
                else
                {
                    /* (Ecm == 0) and (Acm == 0) and (Acs != 0) */
                    if (chipCtx->hashKey.hashZeroAcm & (1 << OutputIndex))
                    {
                        /* rLighting[0] = 0 */
                        glmOPCODE(MOV, ShaderControl->rLighting[OutputIndex], XYZ);
                            glmCONST(0);
                    }

                    /* (Ecm == 0) and (Acm != 0) and (Acs != 0) */
                    else
                    {
                        /* Allocate a temp. */
                        glmALLOCATE_LOCAL_TEMP(temp);

                        /* Allocate resources. */
                        glmUSING_UNIFORM(uAcm);
                        glmUSING_UNIFORM(uAcs);

                        /* temp = uAcm */
                        glmOPCODE(MOV, temp, XYZ);
                            glmLIGHTING_UNIFORM(VS, uAcm, XYZZ, OutputIndex);

                        /* rLighting[0] = temp * Acs */
                        glmOPCODE(MUL, ShaderControl->rLighting[OutputIndex], XYZ);
                            glmTEMP(temp, XYZZ);
                            glmUNIFORM(VS, uAcs, XYZZ);
                    }
                }
                /*
                ** BLOCK7: Acm = as is, ((Ecm == 0) and (Acs != 0)).
                ***************************************************************/
            }
            /*
            ** BLOCK5: Ecm == 0, (Acs != 0)
            *******************************************************************/

            /*******************************************************************
            ** BLOCK8: Ecm != 0, (Acs != 0)
            */
            else
            {
                /***************************************************************
                ** BLOCK9: Acm = Color, ((Ecm != 0) and (Acs != 0)).
                */
                if (colorMaterialEnabled(gc, OutputIndex, GL_AMBIENT) ||
                    colorMaterialEnabled(gc, OutputIndex, GL_AMBIENT_AND_DIFFUSE))
                {
                    /* Color from stream? */
                    if (chipCtx->attributeInfo[__GL_INPUT_DIFFUSE_INDEX].streamEnabled)
                    {
                        /* Allocate a temp. */
                        glmALLOCATE_LOCAL_TEMP(temp);

                        /* Allocate resources. */
                        glmUSING_UNIFORM(uEcm);
                        glmUSING_UNIFORM(uAcs);
                        glmUSING_ATTRIBUTE(aColor);

                        /* temp = aColor * Acs */
                        glmOPCODE(MUL, temp, XYZ);
                            glmATTRIBUTE(VS, aColor, XYZZ);
                            glmUNIFORM(VS, uAcs, XYZZ);

                        /* rLighting[0] = Ecm + temp */
                        glmOPCODE(ADD, ShaderControl->rLighting[OutputIndex], XYZ);
                            glmLIGHTING_UNIFORM(VS, uEcm, XYZZ, OutputIndex);
                            glmTEMP(temp, XYZZ);
                    }

                    /* Color from a constant. */
                    else
                    {
                        /* Color == 0 ? */
                        if (chipCtx->hashKey.hashZeroAcm & (1 << OutputIndex))
                        {
                            /* Allocate resources. */
                            glmUSING_UNIFORM(uEcm);

                            /* rLighting[0] = Ecm */
                            glmOPCODE(MOV, ShaderControl->rLighting[OutputIndex], XYZ);
                                glmLIGHTING_UNIFORM(VS, uEcm, XYZZ, OutputIndex);
                        }
                        else
                        {
                            /* Allocate temporaries. */
                            glmALLOCATE_LOCAL_TEMP(temp1);
                            glmALLOCATE_LOCAL_TEMP(temp2);

                            /* Allocate resources. */
                            glmUSING_UNIFORM(uEcm);
                            glmUSING_UNIFORM(uAcs);
                            glmUSING_UNIFORM(uColor);

                            /* temp1 = uColor */
                            glmOPCODE(MOV, temp1, XYZ);
                                glmUNIFORM(VS, uColor, XYZZ);

                            /* temp2 = temp1 * Acs */
                            glmOPCODE(MUL, temp2, XYZ);
                                glmTEMP(temp1, XYZZ);
                                glmUNIFORM(VS, uAcs, XYZZ);

                            /* rLighting[0] = Ecm + temp2 */
                            glmOPCODE(ADD, ShaderControl->rLighting[OutputIndex], XYZ);
                                glmLIGHTING_UNIFORM(VS, uEcm, XYZZ, OutputIndex);
                                glmTEMP(temp2, XYZZ);
                        }
                    }
                }
                /*
                ** BLOCK9: Acm = Color, ((Ecm != 0) and (Acs != 0)).
                ***************************************************************/

                /***************************************************************
                ** BLOCK10: Acm = as is, ((Ecm != 0) and (Acs != 0)).
                */
                else
                {
                    /* (Ecm != 0) and (Acm == 0) and (Acs != 0) */
                    if (chipCtx->hashKey.hashZeroAcm & (1 << OutputIndex))
                    {
                        /* Allocate resources. */
                        glmUSING_UNIFORM(uEcm);

                        /* rLighting[0] = Ecm */
                        glmOPCODE(MOV, ShaderControl->rLighting[OutputIndex], XYZ);
                            glmLIGHTING_UNIFORM(VS, uEcm, XYZZ, OutputIndex);
                    }

                    /* (Ecm != 0) and (Acm != 0) and (Acs != 0) */
                    else
                    {
                        /* Allocate temporaries. */
                        glmALLOCATE_LOCAL_TEMP(temp1);
                        glmALLOCATE_LOCAL_TEMP(temp2);

                        /* Allocate resources. */
                        glmUSING_UNIFORM(uAcm);
                        glmUSING_UNIFORM(uAcs);
                        glmUSING_UNIFORM(uEcm);

                        /* temp1 = uAcm */
                        glmOPCODE(MOV, temp1, XYZ);
                            glmUNIFORM(VS, uAcm, XYZZ);

                        /* temp2 = temp1 * Acs */
                        glmOPCODE(MUL, temp2, XYZ);
                            glmTEMP(temp1, XYZZ);
                            glmUNIFORM(VS, uAcs, XYZZ);

                        /* rLighting[0] = Ecm + temp2 */
                        glmOPCODE(ADD, ShaderControl->rLighting[OutputIndex], XYZ);
                            glmLIGHTING_UNIFORM(VS, uEcm, XYZZ, OutputIndex);
                            glmTEMP(temp2, XYZZ);
                    }
                }
                /*
                ** BLOCK10: Acm = as is, ((Ecm != 0) and (Acs != 0)).
                ***************************************************************/
            }
            /*
            ** BLOCK8: Ecm != 0, (Acs != 0)
            *******************************************************************/
        }
        /*
        ** BLOCK4: (Acs != 0)
        ***********************************************************************/

        /* Allocate resources. */
        glmUSING_UNIFORM(uDcm);

        /* Accordingly to OpenGL ES spec, alpha component comes from alpha value
           associated with Dcm. */
        glmOPCODE(MOV, ShaderControl->rLighting[OutputIndex], W);
            glmLIGHTING_UNIFORM(VS, uDcm, WWWW, OutputIndex);
    }
    while (gcvFALSE);

    gcmFOOTER();
    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _lightDetermineVPpli
**
**  Generate VPpli vector as it is used in multiple places within the
**  lighting formula. Per OpenGL ES spec:
**
**      Let VPpli be the unit vector that points from point V to point Ppli.
**      Note that:
**          - if V has a zero w coordinate, results of lighting are
**            undefined accordingly to the OpenGL ES spec;
**          - if V has a non-zero w coordinate and Ppli has a zero w
**            coordinate, then VPpli is the unit vector corresponding to the
**            direction specified by the x, y, and z coordinates of Ppli;
**          - if V and Ppli both have non-zero w coordinates, then VPpli
**            is the unit vector obtained by normalizing the direction
**            corresponding to (Ppli - V).
**
**  The notes can be interpreted as:
**
**      if (V.w == 0)
**      {
**          // Results are undefined.
**      }
**
**      else
**      {
**          if (P.w == 0)
**          {
**              VPpli = (Ppli.x, Ppli.y, Ppli.z)
**          }
**          else
**          {
**              VPpli = (Ppli.x - V.x, Ppli.y - V.y, Ppli.z - V.z)
**          }
**      }
**
**  The pseudo-code can be rewritten as:
**
**      VPpli = Ppli - V * Ppli.w'
**      Ppli.w' = (Ppli.w == 0)? 0 : 1
**
**  INPUT:
**
**      gc
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**      LightIndex
**          Current light.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS lightDetermineVPpli(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl,
    gctINT LightIndex
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x LightIndex=%d", gc, ShaderControl, LightIndex);

    do
    {
        if (ShaderControl->rVPpli == 0)
        {
            /* Allocate local resources. */
            glmALLOCATE_LOCAL_TEMP(temp1);
            glmALLOCATE_LOCAL_TEMP(temp2);
            glmALLOCATE_LOCAL_TEMP(temp3);

            /* Allocate the result registers. */
            glmALLOCATE_TEMP(ShaderControl->rVPpli);
            glmALLOCATE_TEMP(ShaderControl->rVPpliLength);

            /* Allocate resources. */
            glmUSING_UNIFORM(uPpli);
            glmUSING_UNIFORM(uVPpli);

            /*******************************************************************
            ** Determine VPpli vector.
            */

            /* Set preliminary VPpli vector value:
               temp1 = Ppli[LightIndex]. */
            glmOPCODE(MOV, temp1, XYZW);
                glmLIGHTING_UNIFORM(VS, uPpli, XYZW, LightIndex);

            /* For generic function we have to put condition in place. */
            if (LightIndex < 0)
            {
                /* Define a label. */
                glmDEFINE_LABEL(lblDirectional);

                /* Compute current position in eye space. */
                gcmERR_BREAK(pos2Eye(gc, ShaderControl));

                /* Allocate a label. */
                glmALLOCATE_LABEL(lblDirectional);

                glmOPCODE(MOV, ShaderControl->rVPpli, XYZ);
                    glmLIGHTING_UNIFORM(VS, uVPpli, XYZZ, LightIndex);

                /* if (temp1.w == 0) goto lblDirectional. */
                glmOPCODE_BRANCH(JMP, EQUAL, lblDirectional);
                    glmTEMP(temp1, WWWW);
                    glmCONST(0);

                {
                    /* Compute VPpli:
                       temp1 = temp1 - V. */
                    glmOPCODE(MOV, temp2, XYZW);
                        glmTEMP(temp1, XYZW);
                    glmOPCODE(SUB, temp1, XYZ);
                        glmTEMP(temp2, XYZZ);
                        glmTEMP(ShaderControl->rVtxInEyeSpace, XYZZ);


                    /***************************************************************
                    ** Normalize VPpli vector (make it into a unit vector).
                    **
                    **                VPpli
                    ** VPpli.norm = ---------
                    **              ||VPpli||
                    **
                    */

                    /* Dot product of the same vector gives a square of its length. */
                    glmOPCODE(DP3, temp2, X);
                        glmTEMP(temp1, XYZZ);
                        glmTEMP(temp1, XYZZ);

                    /* Compute 1 / ||VPpli|| */
                    glmOPCODE(RSQ, temp3, X);
                        glmTEMP(temp2, XXXX);

                    /* Normalize the vector. */
                    glmOPCODE(MUL, ShaderControl->rVPpli, XYZ);
                        glmTEMP(temp1, XYZZ);
                        glmTEMP(temp3, XXXX);

                    /* Compute the length of the vector. */
                    glmOPCODE(MUL, ShaderControl->rVPpliLength, X);
                        glmTEMP(temp2, XXXX);
                        glmTEMP(temp3, XXXX);
                }

                /* Define label. */
                glmLABEL(lblDirectional);
            }
            else
            {
                if (!(gc->state.light.source[LightIndex].positionEye.w == 0.0f))
                {
                    /* Compute current position in eye space. */
                    gcmERR_BREAK(pos2Eye(gc, ShaderControl));

                    /* Compute VPpli:
                       temp1 = temp1 - V. */
                    glmOPCODE(MOV, temp2, XYZW);
                        glmTEMP(temp1, XYZW);
                    glmOPCODE(SUB, temp1, XYZ);
                        glmTEMP(temp2, XYZZ);
                        glmTEMP(ShaderControl->rVtxInEyeSpace, XYZZ);
                }
            }

            if ((LightIndex >= 0) &&
                (gc->state.light.source[LightIndex].positionEye.w == 0.0f))
            {
                    /* Compute current position in eye space. Call this function to init some data even if positionEye.w=0 */
                    gcmERR_BREAK(pos2Eye(gc, ShaderControl));
                    glmOPCODE(MOV, ShaderControl->rVPpli, XYZ);
                        glmLIGHTING_UNIFORM(VS, uVPpli, XYZZ, LightIndex);
            }
            else if(LightIndex >= 0)
            {
                /***************************************************************
                ** Normalize VPpli vector (make it into a unit vector).
                **
                **                VPpli
                ** VPpli.norm = ---------
                **              ||VPpli||
                **
                */

                /* Dot product of the same vector gives a square of its length. */
                glmOPCODE(DP3, temp2, X);
                    glmTEMP(temp1, XYZZ);
                    glmTEMP(temp1, XYZZ);

                /* Compute 1 / ||VPpli|| */
                glmOPCODE(RSQ, temp3, X);
                    glmTEMP(temp2, XXXX);

                /* Normalize the vector. */
                glmOPCODE(MUL, ShaderControl->rVPpli, XYZ);
                    glmTEMP(temp1, XYZZ);
                    glmTEMP(temp3, XXXX);

                /* Compute the length of the vector. */
                glmOPCODE(MUL, ShaderControl->rVPpliLength, X);
                    glmTEMP(temp2, XXXX);
                    glmTEMP(temp3, XXXX);
            }
        }
        else
        {
            status = gcvSTATUS_OK;
        }
    }
    while (gcvFALSE);

    gcmFOOTER();
    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _lightNormDotVPpli
**
**  Generate the dot product between the norm vector and VPpli vector as it is
**  used in multiple places within the lighting formula.
**
**  INPUT:
**
**      gc
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**      LightIndex
**          Current light.
**
**      OutputIndex
**          Color output index (0 or 1).
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS lightNormDotVPpli(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl,
    gctINT LightIndex,
    gctINT OutputIndex
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x LightIndex=%d OutputIndex=%d",
                    gc, ShaderControl, LightIndex, OutputIndex);

    do
    {
        if (ShaderControl->rNdotVPpli[OutputIndex] == 0)
        {
            /* Define a temporary. */
            glmDEFINE_TEMP(temp);

            /* Transform normal to eye space. */
            gcmERR_BREAK(normal2Eye(
                gc, ShaderControl
                ));

            /* Determnine VPpli unit vector. */
            gcmERR_BREAK(lightDetermineVPpli(
                gc, ShaderControl, LightIndex
                ));

            /* Allocate temporaries. */
            glmALLOCATE_TEMP(temp);
            glmALLOCATE_TEMP(ShaderControl->rNdotVPpli[OutputIndex]);

            /* temp.x = dot(n, VPpli) */
            glmOPCODE(DP3, temp, X);
                glmTEMP(ShaderControl->rNrmInEyeSpace[OutputIndex], XYZZ);
                glmTEMP(ShaderControl->rVPpli, XYZZ);

            /* If normal is normalized, "dp3 normal, vp" will be in [-1, 1]
               Use SAT to replace MAX, and expect back-end optimise it. */
            if (gc->state.enables.transform.normalize)
            {
                /* rNdotVPpli[OutputIndex] = sat(dot(n, VPpli), 0). */
                glmOPCODE(SAT, ShaderControl->rNdotVPpli[OutputIndex], X);
                    glmTEMP(temp, XXXX);
            }
            else
            {
                /* rNdotVPpli[OutputIndex] = max(dot(n, VPpli), 0). */
                glmOPCODE(MAX, ShaderControl->rNdotVPpli[OutputIndex], X);
                    glmTEMP(temp, XXXX);
                    glmCONST(0);
            }
        }
        else
        {
            status = gcvSTATUS_OK;
        }
    }
    while (gcvFALSE);

    gcmFOOTER();
    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _lightAttenuation
**
**  Generate light attenuation factor. Per OpenGL ES spec:
**
**  If Ppli's w != 0:
**
**                                1
**      ATTi = --------------------------------------------
**             K0i + K1i * ||VPpli|| + K2i * ||VPpli|| ** 2
**
**  If Ppli's w == 0:
**
**      ATTi = 1
**
**  where K0i, K1i, K2i - user-defined constants associated with the light;
**        V             - current vertex;
**        Ppli          - position of light i;
**        ||VPpli||     - distance between the vertex and the light.
**
**  INPUT:
**
**      gc
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**      LightIndex
**          Current light.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS lightAttenuation(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl,
    gctINT LightIndex
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x LightIndex=%d",
                    gc, ShaderControl, LightIndex);

    do
    {
        /* Per-light optimization. */
        if (
             (LightIndex >= 0) &&
                (
                    (
                        /* Attenuation = 1 for directional lights. */
                        (gc->state.light.source[LightIndex].positionEye.w == 0.0f) ||

                        /* Attenuation = 1 if K0i == 1 and K1i == 0 and K2i == 0. */
                        ((gc->state.light.source[LightIndex].constantAttenuation == 1.0f) &&
                        (gc->state.light.source[LightIndex].linearAttenuation == 0.0f) &&
                        (gc->state.light.source[LightIndex].quadraticAttenuation == 0.0f))
                    )
                    /* Already computed. */
                    || (ShaderControl->rAttenuation != 0)
                )
            )
        {
            status = gcvSTATUS_OK;
        }
        else
        {
            /* For generic function we have to put condition in place. */
            if (LightIndex < 0)
            {
                glmDEFINE_TEMP(temp1);
                /* Define a label. */
                glmDEFINE_LABEL(lblDirectional);

                glmALLOCATE_TEMP(temp1);
                /* Allocate resources. */
                glmUSING_UNIFORM(uPpli);

                glmALLOCATE_TEMP(ShaderControl->rAttenuation);

                /*   ShaderControl->rAttenuation = 1.0. */
                glmOPCODE(MOV, ShaderControl->rAttenuation, XYZW);
                    glmCONST(1.0);

                /*   temp1 = Ppli[LightIndex]. */
                glmOPCODE(MOV, temp1, XYZW);
                    glmLIGHTING_UNIFORM(VS, uPpli, XYZW, LightIndex);

                /* Allocate a label. */
                glmALLOCATE_LABEL(lblDirectional);

                /* if (temp1.w == 0) goto lblDirectional. */
                glmOPCODE_BRANCH(JMP, EQUAL, lblDirectional);
                    glmTEMP(temp1, WWWW);
                    glmCONST(0);

                {
                    glmDEFINE_TEMP(temp1);
                    glmDEFINE_TEMP(temp2);

                    /* Determnine VPpli unit vector. */
                    gcmERR_BREAK(lightDetermineVPpli(gc, ShaderControl, LightIndex));

                    /* Allocate temporaries. */
                    glmALLOCATE_TEMP(temp1);
                    glmALLOCATE_TEMP(temp2);

                    /* Allocate resources. */
                    glmUSING_UNIFORM(uKi);

                    /* temp1.x = 1 */
                    glmOPCODE(MOV, temp1, X);
                        glmCONST(1);

                    /* temp1.y = len(VPpli) */
                    glmOPCODE(MOV, temp1, Y);
                        glmTEMP(ShaderControl->rVPpliLength, XXXX);

                    /* temp1.z = len(VPpli)^2 */
                    glmOPCODE(MUL, temp1, Z);
                        glmTEMP(ShaderControl->rVPpliLength, XXXX);
                        glmTEMP(ShaderControl->rVPpliLength, XXXX);

                    /* temp2.x = K0i + K1i * ||VPpli|| + K2i * ||VPpli||^2 */
                    glmOPCODE(DP3, temp2, X);
                        glmLIGHTING_UNIFORM(VS, uKi, XYZZ, LightIndex);
                        glmTEMP(temp1, XYZZ);

                    /* rAttenuation = rcp(temp2.x) */
                    glmOPCODE(RCP, ShaderControl->rAttenuation, X);
                        glmTEMP(temp2, XXXX);
                }
                /* Define label. */
                glmLABEL(lblDirectional);
            }
            else
            {
                glmDEFINE_TEMP(temp1);
                glmDEFINE_TEMP(temp2);

                /* Determnine VPpli unit vector. */
                gcmERR_BREAK(lightDetermineVPpli(gc, ShaderControl, LightIndex));

                glmALLOCATE_TEMP(ShaderControl->rAttenuation);

                /* Allocate temporaries. */
                glmALLOCATE_TEMP(temp1);
                glmALLOCATE_TEMP(temp2);

                /* Allocate resources. */
                glmUSING_UNIFORM(uKi);

                /* temp1.x = 1 */
                glmOPCODE(MOV, temp1, X);
                    glmCONST(1);

                /* temp1.y = len(VPpli) */
                glmOPCODE(MOV, temp1, Y);
                    glmTEMP(ShaderControl->rVPpliLength, XXXX);

                /* temp1.z = len(VPpli)^2 */
                glmOPCODE(MUL, temp1, Z);
                    glmTEMP(ShaderControl->rVPpliLength, XXXX);
                    glmTEMP(ShaderControl->rVPpliLength, XXXX);

                /* temp2.x = K0i + K1i * ||VPpli|| + K2i * ||VPpli||^2 */
                glmOPCODE(DP3, temp2, X);
                    glmLIGHTING_UNIFORM(VS, uKi, XYZZ, LightIndex);
                    glmTEMP(temp1, XYZZ);

                /* rAttenuation = rcp(temp2.x) */
                glmOPCODE(RCP, ShaderControl->rAttenuation, X);
                    glmTEMP(temp2, XXXX);
            }

        }
    }
    while (gcvFALSE);

    gcmFOOTER();
    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _lightSpot
**
**  Generate light spot factor. Per OpenGL ES spec:
**
**  If (Crli != 180) and (PpliV dot S'dli >= cos(Crli)):
**
**      SPOTi = (PpliV dot S'dli) ** Srli
**
**  If (Crli != 180) and (PpliV dot S'dli < cos(Crli)):
**
**      SPOTi = 0
**
**  If (Crli == 180):
**
**      SPOTi = 1
**
**  INPUT:
**
**      gc
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**      LightIndex
**          Current light.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS lightSpot(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl,
    gctINT LightIndex
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x LightIndex=%d",
                    gc, ShaderControl, LightIndex);

    do
    {
        /* Per-light optimization. */
        if (
                (
                    (LightIndex >= 0) &&

                    /* Spot = 1, if Crli == 180. */
                    (gc->state.light.source[LightIndex].spotLightCutOffAngle == 180.0f)
                )

                /* Already computed. */
                || (ShaderControl->rSpot != 0)
           )
        {
            status = gcvSTATUS_OK;
        }
        else
        {
            /* Define a label. */
            glmDEFINE_LABEL(lblZero1);
            /* Define a label. */
            glmDEFINE_LABEL(lblZero);

            /* Define temporaries. */
            glmDEFINE_TEMP(temp1);
            glmDEFINE_TEMP(temp2);
            glmDEFINE_TEMP(temp3);
            glmDEFINE_TEMP(temp4);
            glmDEFINE_TEMP(temp5);

            /* Allocate the temporary registers. */
            glmALLOCATE_TEMP(temp1);
            glmALLOCATE_TEMP(temp2);

            glmUSING_UNIFORM(uCrli);
            glmOPCODE(MOV, temp1, X);
                glmLIGHTING_UNIFORM(VS, uCrli, XXXX, LightIndex);

            glmOPCODE(SUB, temp2, X);
                glmCONST(180);
                glmTEMP(temp1, XXXX);

            glmALLOCATE_TEMP(ShaderControl->rSpot);

            glmOPCODE(MOV, ShaderControl->rSpot, X);
                glmCONST(1.0);

            glmALLOCATE_LABEL(lblZero1);

            /* if (max(dot(PpliV, S'dli), 0) < cos(Crli)) goto lblZero. */
            glmOPCODE_BRANCH(JMP, EQUAL, lblZero1);
                glmTEMP(temp2, XXXX);
                glmCONST(0);

            {
                /* Determnine VPpli unit vector. */
                gcmERR_BREAK(lightDetermineVPpli(gc, ShaderControl, LightIndex));

                /* Allocate the label. */
                glmALLOCATE_LABEL(lblZero);
                glmALLOCATE_TEMP(temp3);
                glmALLOCATE_TEMP(temp4);
                glmALLOCATE_TEMP(temp5);

                /* Allocate resources. */
                glmUSING_UNIFORM(uNormedSdli);
                glmUSING_UNIFORM(uCosCrli);
                glmUSING_UNIFORM(uSrli);

                /***************************************************************************
                ** Hardware cannot compute exponents with an arbitrary base. We have to do
                ** a transformation to be able to compute the formula. By definition:
                **           logB(A)
                **      A = B
                ** substitute B with 2, we get:
                **           log2(A)
                **      A = 2
                ** raise both parts of the equation to the power of Srli, we get:
                **       Srli    Srli * log2(A)
                **      A     = 2
                ** where:
                **      A = PpliV dot S'dli.
                ** to summarize:
                **                       Srli    Srli * log2(PpliV dot S'dli)
                **      (PpliV dot S'dli)     = 2
                */

                /* Get normalized S'dli.  */
                glmOPCODE(MOV, temp1, XYZ);
                    glmLIGHTING_UNIFORM(VS, uNormedSdli, XYZZ, LightIndex);

                /*
                **  Compute dot(PpliV, S'dli) and cos(Crli).
                */

                /* Negate VPli. */
                glmOPCODE(SUB, temp2, XYZ);
                    glmCONST(0);
                    glmTEMP(ShaderControl->rVPpli, XYZZ);

                /* dot(PpliV, S'dli). */
                glmOPCODE(DP3, temp3, X);
                    glmTEMP(temp1, XYZZ);
                    glmTEMP(temp2, XYZZ);

                /* max(dot(PpliV, S'dli), 0). */
                glmOPCODE(MAX, temp4, X);
                    glmTEMP(temp3, XXXX);
                    glmCONST(0);

                /* Get cos(Crli). */
                glmOPCODE(MOV, temp5, X);
                    glmLIGHTING_UNIFORM(VS, uCosCrli, XXXX, LightIndex);

                /* rSpot = 0 */
                glmOPCODE(MOV, ShaderControl->rSpot, X);
                    glmCONST(0);

                /* if (max(dot(PpliV, S'dli), 0) < cos(Crli)) goto lblZero. */
                glmOPCODE_BRANCH(JMP, LESS, lblZero);
                    glmTEMP(temp4, XXXX);
                    glmTEMP(temp5, XXXX);

                {
                    /*
                    **  CASE:   max(dot(PpliV, S'dli), 0) >= cos(Crli)
                    **
                    **                                   Srli
                    **  rSpot = max(dot(PpliV, S'dli), 0)
                    */
                    glmOPCODE(POW, ShaderControl->rSpot, X);
                        glmTEMP(temp4, XXXX);
                        glmLIGHTING_UNIFORM(VS, uSrli, XXXX, LightIndex);
                }

                glmLABEL(lblZero);
            }
            glmLABEL(lblZero1);
        }
    }
    while (gcvFALSE);

    gcmFOOTER();
    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _lightAmbient
**
**  Generate ambient factor for the current light. Per OpenGL ES spec:
**
**      AMBIENTi = Acm * Acli
**
**  INPUT:
**
**      gc
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**      LightIndex
**          Current light.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS lightAmbient(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl,
    gctINT LightIndex,
    gctINT OutputIndex
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x LightIndex=%d",
                    gc, ShaderControl, LightIndex);

    do
    {
        /* Per-light optimization. */
        if (
                (
                    (LightIndex >= 0) &&

                    /* Ambient = 0, if Acli == 0 */
                    (chipCtx->hashKey.hashZeroAcl & (1 << LightIndex))
                )

                /* Already computed. */
                || (ShaderControl->rAmbient[OutputIndex] != 0)
           )
        {
            status = gcvSTATUS_OK;
        }
        else
        {
            /*******************************************************************
            ** Per OpenGL ES spec:
            **   When color material tracking is enabled, by glEnable with
            **   COLOR_MATERIAL, the ambient (Acm) property of both the front
            **   and back material are immediately set to the value of the
            **   current color, and will track changes to the current color
            **   resulting from either the glColor commands or drawing vertex
            **   arrays with the color array enabled.
            */

            if (colorMaterialEnabled(gc, OutputIndex, GL_AMBIENT) ||
                colorMaterialEnabled(gc, OutputIndex, GL_AMBIENT_AND_DIFFUSE))
            {
                /* Color from stream? */
                if (chipCtx->attributeInfo[__GL_INPUT_DIFFUSE_INDEX].streamEnabled)
                {
                    /* Allocate resources. */
                    glmUSING_UNIFORM(uAcli);
                    glmUSING_ATTRIBUTE(aColor);

                    /* Allocate the result. */
                    glmALLOCATE_TEMP(ShaderControl->rAmbient[OutputIndex]);

                    /* rAmbient = aColor * Acli */
                    glmOPCODE(MUL, ShaderControl->rAmbient[OutputIndex], XYZ);
                        glmATTRIBUTE(VS, aColor, XYZZ);
                        glmLIGHTING_UNIFORM(VS, uAcli, XYZZ, LightIndex);
                }

                /* Color from a constant. */
                else
                {
                    /* Color == 0 ? */
                    if (chipCtx->hashKey.hashZeroAcm & (1 << OutputIndex))
                    {
                        /* rAmbient = 0 */
                        status = gcvSTATUS_OK;
                    }
                    else
                    {
                        /* Allocate temporaries. */
                        glmALLOCATE_TEMP(ShaderControl->rAmbient[OutputIndex]);

                        if (OutputIndex == 0) {
                            /* For front face case */
                            /* Allocate resources. */
                            glmUSING_UNIFORM(uAcmAcli);

                            /* rAmbient = pre-calculated (aColor * Acli) */
                            glmOPCODE(MOV, ShaderControl->rAmbient[OutputIndex], XYZ);
                                glmLIGHTING_UNIFORM(VS, uAcmAcli, XYZZ, LightIndex);
                        } else {
                            /* For back face case */
                            /* Allocate resources. */
                            glmUSING_UNIFORM(uAcmAcli2);

                            /* rAmbient = pre-calculated (aColor * Acli) */
                            glmOPCODE(MOV, ShaderControl->rAmbient[OutputIndex], XYZ);
                                glmLIGHTING_UNIFORM(VS, uAcmAcli2, XYZZ, LightIndex);
                        }
                    }
                }
            }
            else
            {
                if (chipCtx->hashKey.hashZeroAcm & (1 << OutputIndex))
                {
                    /* rAmbient = 0 */
                    status = gcvSTATUS_OK;
                }
                else
                {
                    /* Allocate temporaries. */
                    glmALLOCATE_TEMP(ShaderControl->rAmbient[OutputIndex]);

                    if (OutputIndex == 0) {
                        /* For front face case */
                        /* Allocate resources. */
                        glmUSING_UNIFORM(uAcmAcli);

                        /* rAmbient = pre-calculated (uAcm * Acli) */
                        glmOPCODE(MOV, ShaderControl->rAmbient[OutputIndex], XYZ);
                            glmLIGHTING_UNIFORM(VS, uAcmAcli, XYZZ, LightIndex);
                    } else {
                        /* For back face case */
                        /* Allocate resources. */
                        glmUSING_UNIFORM(uAcmAcli2);

                        /* rAmbient = pre-calculated (uAcm * Acli) */
                        glmOPCODE(MOV, ShaderControl->rAmbient[OutputIndex], XYZ);
                            glmLIGHTING_UNIFORM(VS, uAcmAcli2, XYZZ, LightIndex);
                    }
                }
            }
        }
    }
    while (gcvFALSE);

    gcmFOOTER();
    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _lightDiffuse
**
**  Compute diffuse component of the formula. Per OpenGL ES spec:
**
**      DIFFUSEi = (n dot VPpli) * (Dcm * Dcli),
**
**  INPUT:
**
**      gc
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**      LightIndex
**          Current light.
**
**      OutputIndex
**          Color output index (0 or 1).
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS lightDiffuse(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl,
    gctINT LightIndex,
    gctINT OutputIndex
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x LightIndex=%d OutputIndex=%d",
                    gc, ShaderControl, LightIndex, OutputIndex);

    do
    {
        /* Per-light optimization. */
        if (
                (LightIndex >= 0) &&

                /* Diffuse = 0, if Dcli == 0 */
                (chipCtx->hashKey.hashZeroDcl & (1 << LightIndex))
           )
        {
            status = gcvSTATUS_OK;
        }
        else
        {
            /*******************************************************************
            ** Per OpenGL ES spec:
            **   When color material tracking is enabled, by glEnable with
            **   COLOR_MATERIAL, the diffuse (Dcm) property of both the front
            **   and back material are immediately set to the value of the
            **   current color, and will track changes to the current color
            **   resulting from either the glColor commands or drawing vertex
            **   arrays with the color array enabled.
            */

            /* Allocate temporary registers. */
            if (colorMaterialEnabled(gc, OutputIndex, GL_DIFFUSE) ||
                colorMaterialEnabled(gc, OutputIndex, GL_AMBIENT_AND_DIFFUSE))
            {
                /* Color from stream? */
                if (chipCtx->attributeInfo[__GL_INPUT_DIFFUSE_INDEX].streamEnabled)
                {
                    glmDEFINE_TEMP(temp);

                    /* Determine dot product between norm vector and VPpli. */
                    gcmERR_BREAK(lightNormDotVPpli(
                        gc, ShaderControl, LightIndex, OutputIndex
                        ));

                    /* Allocate temporaries. */
                    glmALLOCATE_TEMP(temp);
                    glmALLOCATE_TEMP(ShaderControl->rDiffuse[OutputIndex]);

                    /* Allocate resources. */
                    glmUSING_UNIFORM(uDcli);
                    glmUSING_ATTRIBUTE(aColor);

                    /* temp = aColor * Dcli */
                    glmOPCODE(MUL, temp, XYZ);
                        glmATTRIBUTE(VS, aColor, XYZZ);
                        glmLIGHTING_UNIFORM(VS, uDcli, XYZZ, LightIndex);

                    /* rDiffuse[OutputIndex] = rNdotVPpli * temp */
                    glmOPCODE(MUL, ShaderControl->rDiffuse[OutputIndex], XYZ);
                        glmTEMP(ShaderControl->rNdotVPpli[OutputIndex], XXXX);
                        glmTEMP(temp, XYZZ);
                }

                /* Color from a constant. */
                else
                {
                    /* Color == 0 ? */
                    if (chipCtx->hashKey.hashZeroDcm & (1 << OutputIndex))
                    {
                        /* rDiffuse[OutputIndex] = 0 */
                        status = gcvSTATUS_OK;
                    }
                    else
                    {
                        /* Determine dot product between norm vector and VPpli. */
                        gcmERR_BREAK(lightNormDotVPpli(
                            gc, ShaderControl, LightIndex, OutputIndex
                            ));

                        /* Allocate temporaries. */
                        glmALLOCATE_TEMP(ShaderControl->rDiffuse[OutputIndex]);
                        if (OutputIndex == 0) {
                            /* Allocate resources. */
                            glmUSING_UNIFORM(uDcmDcli);

                            /* rDiffuse[OutputIndex] = rNdotVPpli[OutputIndex] * uColor * Dcli */
                            glmOPCODE(MUL, ShaderControl->rDiffuse[OutputIndex], XYZ);
                                glmTEMP(ShaderControl->rNdotVPpli[OutputIndex], XXXX);
                                glmLIGHTING_UNIFORM(VS, uDcmDcli, XYZZ, LightIndex);
                        } else {
                            /* Allocate resources. */
                            glmUSING_UNIFORM(uDcmDcli2);

                            /* rDiffuse[OutputIndex] = rNdotVPpli[OutputIndex] * uColor * Dcli */
                            glmOPCODE(MUL, ShaderControl->rDiffuse[OutputIndex], XYZ);
                                glmTEMP(ShaderControl->rNdotVPpli[OutputIndex], XXXX);
                                glmLIGHTING_UNIFORM(VS, uDcmDcli2, XYZZ, LightIndex);
                        }
                    }
                }
            }
            else
            {
                if (chipCtx->hashKey.hashZeroDcm & (1 << OutputIndex))
                {
                    /* rDiffuse[OutputIndex] = 0 */
                    status = gcvSTATUS_OK;
                }
                else
                {
                    /* Determine dot product between norm vector and VPpli. */
                    gcmERR_BREAK(lightNormDotVPpli(
                        gc, ShaderControl, LightIndex, OutputIndex
                        ));

                    /* Allocate temporaries. */
                    glmALLOCATE_TEMP(ShaderControl->rDiffuse[OutputIndex]);

                    if (OutputIndex == 0) {
                        /* Allocate resources. */
                        glmUSING_UNIFORM(uDcmDcli);

                        /* rDiffuse[OutputIndex] = rNdotVPpli[OutputIndex] * uDcm * Dcli */
                        glmOPCODE(MUL, ShaderControl->rDiffuse[OutputIndex], XYZ);
                            glmTEMP(ShaderControl->rNdotVPpli[OutputIndex], XXXX);
                            glmLIGHTING_UNIFORM(VS, uDcmDcli, XYZZ, LightIndex);
                    } else {
                        /* Allocate resources. */
                        glmUSING_UNIFORM(uDcmDcli2);

                        /* rDiffuse[OutputIndex] = rNdotVPpli[OutputIndex] * uDcm * Dcli */
                        glmOPCODE(MUL, ShaderControl->rDiffuse[OutputIndex], XYZ);
                            glmTEMP(ShaderControl->rNdotVPpli[OutputIndex], XXXX);
                            glmLIGHTING_UNIFORM(VS, uDcmDcli2, XYZZ, LightIndex);
                    }
                }
            }
        }
    }
    while (gcvFALSE);

    gcmFOOTER();
    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _lightSpecular
**
**  Compute specular component of the formula. Per OpenGL ES spec:
**
**      SPECULARi = (Fi) * ((n dot H'i) ** Srm) * (Scm * Scli),
**
**  where
**
**      Fi = 1, if (n dot VPpli) != 0
**         = 0 otherwise
**
**      Hi = halfway vector = VPpli + (0, 0, 1)
**
**      H'i = normalized(Hi)
**
**  OpenGL spec defines:
**
**      Hi = VPpli + VPe, where VPe is a unit vector between the vertex and the
**      eye position.
**
**  OpenGL ES always assumes an infinite viewer, which in unit vector space
**  means (0, 0, 1).
**
**  INPUT:
**
**      gc
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**      LightIndex
**          Current light.
**
**      OutputIndex
**          Color output index (0 or 1).
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS lightSpecular(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl,
    gctINT LightIndex,
    gctINT OutputIndex
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x LightIndex=%d OutputIndex=%d",
                    gc, ShaderControl, LightIndex, OutputIndex);

    do
    {
        /* Per-light optimization. */
        if (
                (LightIndex >= 0) &&
                /* Specular = 0, if Scli == 0 */
                ((chipCtx->hashKey.hashZeroScl & (1 << LightIndex))
                /* Already computed. */
                || (ShaderControl->rSpecular[OutputIndex] != 0)
                )
           )
        {
            status = gcvSTATUS_OK;
        }
        else
        {
            glmDEFINE_LABEL(lblZero);

            /* Determine dot product between norm vector and VPpli. */
            gcmERR_BREAK(lightNormDotVPpli(
                gc, ShaderControl, LightIndex, OutputIndex
                ));

            /* Allocate resources. */
            glmALLOCATE_LABEL(lblZero);
            glmALLOCATE_TEMP(ShaderControl->rSpecular[OutputIndex]);

            /* rSpecular[OutputIndex] = 0 */
            glmOPCODE(MOV, ShaderControl->rSpecular[OutputIndex], XYZ);
                glmCONST(0);

            /* Function Fi: if (max(dot(n, VPpli), 0) == 0) */
            glmOPCODE_BRANCH(JMP, EQUAL, lblZero);
                glmTEMP(ShaderControl->rNdotVPpli[OutputIndex], XXXX);
                glmCONST(0);

            {
                /*
                **  CASE:   max(dot(n, VPpli), 0) != 0
                **
                **                                     Srm
                **  rSpecular = Scm * Scli * (n dot H'i)
                */

                /* Srm == 0 --> no need to compute the dot product. */
                if (chipCtx->hashKey.hashZeroSrm & (1 << OutputIndex))
                {
                    glmALLOCATE_LOCAL_TEMP(temp);

                    /* Allocate resources. */
                    glmUSING_UNIFORM(uScli);

                    if (colorMaterialEnabled(gc, OutputIndex, GL_SPECULAR) &&
                        chipCtx->attributeInfo[__GL_INPUT_DIFFUSE_INDEX].streamEnabled)
                    {
                        glmUSING_ATTRIBUTE(aColor);

                        glmOPCODE(MUL, ShaderControl->rSpecular[OutputIndex], XYZ);
                            glmATTRIBUTE(VS, aColor, XYZZ);
                            glmLIGHTING_UNIFORM(VS, uScli, XYZZ, LightIndex);

                    } else {
                        glmUSING_UNIFORM(uScm);

                        glmOPCODE(MOV, temp, XYZ);
                            glmLIGHTING_UNIFORM(VS, uScm, XYZZ, OutputIndex);

                        glmOPCODE(MUL, ShaderControl->rSpecular[OutputIndex], XYZ);
                            glmTEMP(temp, XYZZ);
                            glmLIGHTING_UNIFORM(VS, uScli, XYZZ, LightIndex);
                    }
                }

                /* Srm != 0 */
                else
                {
                    /* Define temporaries. */
                    glmDEFINE_TEMP(temp1);
                    glmDEFINE_TEMP(temp2);
                    glmDEFINE_TEMP(temp3);
                    glmDEFINE_TEMP(temp4);
                    glmDEFINE_TEMP(temp5);
                    glmDEFINE_TEMP(temp6);

                    /* Transform normal to eye space. */
                    gcmERR_BREAK(normal2Eye(gc, ShaderControl));

                    /* Transform position to eye space. */
                    gcmERR_BREAK(pos2Eye(gc, ShaderControl));

                    /* Determnine VPpli unit vector. */
                    gcmERR_BREAK(lightDetermineVPpli(gc, ShaderControl, LightIndex));

                    /* Allocate temporaries. */
                    glmALLOCATE_TEMP(temp1);
                    glmALLOCATE_TEMP(temp2);
                    glmALLOCATE_TEMP(temp3);
                    glmALLOCATE_TEMP(temp4);
                    glmALLOCATE_TEMP(temp5);
                    glmALLOCATE_TEMP(temp6);

                    /* Allocate resources. */
                    glmUSING_UNIFORM(uSrm);
                    glmUSING_UNIFORM(uScm);
                    glmUSING_UNIFORM(uScli);

                    if (chipCtx->hashKey.hasLocalViewer)
                    {
                        /* Define a label. */
                        /* view point is in (0, 0, 0) */
                        /*
                        **  Compute vector:
                        **      temp6 = VPpli + VPe.
                        **      Pe = (0, 0, 0)
                        */
                        glmDEFINE_LABEL(lblZero1);

                        /* Allocate the label. */
                        glmALLOCATE_LABEL(lblZero1);

                        /* temp6.x = 1 */
                        glmOPCODE(MOV, temp5, X);
                             glmCONST(1);

                        glmOPCODE(DP3, temp1, Z);
                        glmTEMP(ShaderControl->rVtxInEyeSpace, XYZZ);
                        glmTEMP(ShaderControl->rVtxInEyeSpace, XYZZ);

                        /* Make sure Z is not zero, otherwize RSQ will produce an INF. */
                        glmOPCODE_BRANCH(JMP, EQUAL, lblZero1);
                        glmTEMP(temp1, ZZZZ);
                            glmCONST(0);

                        {
                            /* temp2 = viewpoint - V */
                            glmOPCODE(SUB, temp2, XYZ);
                                glmCONST(0);
                                glmTEMP(ShaderControl->rVtxInEyeSpace, XYZZ);

                            /* temp2.w = temp1.w */
                            glmOPCODE(MOV, temp2, W);
                                glmTEMP(temp1, ZZZZ);

                            /* temp1.x = 1 / sqrt (temp2.x); 1 / sqrt(x^2 + y^2 + (z+1)^2) */
                            glmOPCODE(RSQ, temp1, X);
                                glmTEMP(temp2, WWWW);

                            glmOPCODE(MUL, temp6, XYZ);
                                glmTEMP(temp2, XYZZ);
                                glmTEMP(temp1, XXXX);

                            /* VPpli + VPe */
                            glmOPCODE(ADD, temp1, XYZ);
                                glmTEMP(ShaderControl->rVPpli, XYZZ);
                                glmTEMP(temp6, XYZZ);

                            /* Normalize */
                            glmOPCODE(NORM, temp6, XYZ);
                                glmTEMP(temp1, XYZZ);

                            /* temp2.x = max(dot(n, H'i), 0). */
                            glmOPCODE(DP3, temp1, X);
                                glmTEMP(ShaderControl->rNrmInEyeSpace[OutputIndex], XYZZ);
                                glmTEMP(temp6, XYZZ);

                            glmOPCODE(MAX, temp6, X);
                                glmTEMP(temp1, XXXX);
                                glmCONST(0);

                            /* temp5.x = max(dot(n, H'i), 0) ^ Srm */
                            glmOPCODE(POW, temp5, X);
                                glmTEMP(temp6, XXXX);
                                glmLIGHTING_UNIFORM(VS, uSrm, XXXX, OutputIndex);
                        }
                        /* Define label. */
                        glmLABEL(lblZero1);
                    } else {
                        /*
                        **  Compute halfway vector:
                        **      temp1 = VPpli + (0, 0, 1).
                        */
                        glmDEFINE_LABEL(lblZero2);
                        glmALLOCATE_LABEL(lblZero2);

                        glmOPCODE(MOV, temp1, XYZ);
                            glmTEMP(ShaderControl->rVPpli, XYZZ);

                        glmOPCODE(ADD, temp1, Z);
                            glmTEMP(ShaderControl->rVPpli, ZZZZ);
                            glmCONST(1);

                        /* Normalize halfway vector. */
                        glmOPCODE(NORM, temp2, XYZ);
                            glmTEMP(temp1, XYZZ);

                        /* temp3.x = max(dot(n, H'i), 0). */
                        glmOPCODE(DP3, temp3, X);
                            glmTEMP(ShaderControl->rNrmInEyeSpace[OutputIndex], XYZZ);
                            glmTEMP(temp2, XYZZ);

                        glmOPCODE(MAX, temp4, X);
                            glmTEMP(temp3, XXXX);
                            glmCONST(0);

                        /* temp5.x = max(dot(n, H'i), 0) ^ Srm
                            0^0 perhaps can't be calculated by HW, next code to handle this.
                            0^0 should be 1.
                        */
                        glmOPCODE(MOV, temp5, X);
                          glmCONST(1);

                        glmOPCODE(ADD, temp6, X);
                            glmCONST(0);
                            glmLIGHTING_UNIFORM(VS, uSrm, XXXX, OutputIndex);

                        glmOPCODE_BRANCH(JMP, EQUAL, lblZero2);
                        glmTEMP(temp6, X);
                                glmCONST(0);
                        {
                            glmOPCODE(POW, temp5, X);
                                glmTEMP(temp4, XXXX);
                                glmTEMP(temp6, XXXX);
                         }
                         glmLABEL(lblZero2);
                    }

                    if (colorMaterialEnabled(gc, OutputIndex, GL_SPECULAR) &&
                        chipCtx->attributeInfo[__GL_INPUT_DIFFUSE_INDEX].streamEnabled)
                    {
                        glmUSING_ATTRIBUTE(aColor);
                        glmOPCODE(MUL, temp6, XYZ);
                            glmTEMP(temp5, XXXX);
                            glmATTRIBUTE(VS, aColor, XYZZ);
                    }
                    else
                    {
                        /* Multuply by Scm. */
                        glmOPCODE(MUL, temp6, XYZ);
                            glmTEMP(temp5, XXXX);
                            glmLIGHTING_UNIFORM(VS, uScm, XYZZ, OutputIndex);
                    }

                    /* Multuply by Scli. */
                    glmOPCODE(MUL, ShaderControl->rSpecular[OutputIndex], XYZ);
                        glmTEMP(temp6, XYZZ);
                        glmLIGHTING_UNIFORM(VS, uScli, XYZZ, LightIndex);
                }
            }

            glmLABEL(lblZero);
        }
    }
    while (gcvFALSE);

    gcmFOOTER();
    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _lightGetPerLightResult
**
**  Complete the result by putting all components together.
**
**  INPUT:
**
**      gc
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**      LightIndex
**          Current light.
**
**      OutputIndex
**          Color output index (0 or 1).
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS lightGetPerLightResult(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl,
    gctINT LightIndex,
    gctINT OutputIndex
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x LightIndex=%d OutputIndex=%d",
                    gc, ShaderControl, LightIndex, OutputIndex);

    do
    {
        /* Define the accumulator. */
        glmDEFINE_TEMP(accumulator) = 0;
        glmDEFINE_TEMP(accumulator2) = 0;
        glmDEFINE_TEMP(temp) = 0;
        glmDEFINE_TEMP(tempOut) = 0;
        glmDEFINE_TEMP(tempOut2) = 0;

        glmALLOCATE_TEMP(temp);
        glmALLOCATE_TEMP(tempOut);
        glmALLOCATE_TEMP(tempOut2);
        /* Diffuse color */
        glmOPCODE(MOV, tempOut, XYZW);
            glmTEMP(ShaderControl->rLighting[OutputIndex], XYZW);

        /* specular color */
        glmOPCODE(MOV, tempOut2, XYZW);
            glmTEMP(ShaderControl->rLighting2[OutputIndex], XYZW);

        /* Initialize with ambient component. */
        if (ShaderControl->rAmbient[OutputIndex])
        {
            glmALLOCATE_TEMP(accumulator);
            glmOPCODE(MOV, accumulator, XYZ);
                glmTEMP(ShaderControl->rAmbient[OutputIndex], XYZZ);
        }

        /* Add diffuse component. */
        if (ShaderControl->rDiffuse[OutputIndex])
        {
            if (accumulator)
            {
                glmOPCODE(MOV, temp, XYZW);
                    glmTEMP(accumulator, XYZW);
                glmOPCODE(ADD, accumulator, XYZ);
                    glmTEMP(temp, XYZZ);
                    glmTEMP(ShaderControl->rDiffuse[OutputIndex], XYZZ);
            }
            else
            {
                glmALLOCATE_TEMP(accumulator);
                glmOPCODE(MOV, accumulator, XYZ);
                    glmTEMP(ShaderControl->rDiffuse[OutputIndex], XYZZ);
            }
        }

        /* Add specular component. */
        if (ShaderControl->rSpecular[OutputIndex])
        {
            glmALLOCATE_TEMP(accumulator2);
            glmOPCODE(MOV, accumulator2, XYZ);
                glmTEMP(ShaderControl->rSpecular[OutputIndex], XYZZ);
        }

        /* Multiply by the spot factor. */
        if (ShaderControl->rSpot)
        {
            /* Multiply diffuse by the spot factor */
            if (accumulator) {
            glmOPCODE(MOV, temp, XYZW);
                glmTEMP(accumulator, XYZW);
            glmOPCODE(MUL, accumulator, XYZ);
                glmTEMP(ShaderControl->rSpot, XXXX);
                glmTEMP(temp, XYZZ);
            }

            if (ShaderControl->rSpecular[OutputIndex])
            {
                /* Multiply specular by the spot factor */
                glmOPCODE(MOV, temp, XYZW);
                    glmTEMP(accumulator2, XYZW);

                glmOPCODE(MUL, accumulator2, XYZ);
                    glmTEMP(ShaderControl->rSpot, XXXX);
                    glmTEMP(temp, XYZZ);
            }
        }

        /* Multiply by the attenuation factor. */
        if (ShaderControl->rAttenuation)
        {
            /* Multiply diffuse by the attenuation factor. */
            if (accumulator) {
            glmOPCODE(MOV, temp, XYZW);
                glmTEMP(accumulator, XYZW);
            glmOPCODE(MUL, accumulator, XYZ);
                glmTEMP(ShaderControl->rAttenuation, XXXX);
                glmTEMP(temp, XYZZ);
            }

            if (ShaderControl->rSpecular[OutputIndex])
            {
                /* Multiply specular by the attenuation factor. */
                glmOPCODE(MOV, temp, XYZW);
                    glmTEMP(accumulator2, XYZW);

                glmOPCODE(MUL, accumulator2, XYZ);
                    glmTEMP(ShaderControl->rAttenuation, XXXX);
                    glmTEMP(temp, XYZZ);
            }
        }

        /* Add to the final diffuse result. */
        if (accumulator) {
        glmOPCODE(ADD, ShaderControl->rLighting[OutputIndex], XYZ);
            glmTEMP(tempOut, XYZZ);
            glmTEMP(accumulator, XYZZ);
        }

        if (ShaderControl->rSpecular[OutputIndex])
        {
            /* Add to the final specular result. */
            glmOPCODE(ADD, ShaderControl->rLighting2[OutputIndex], XYZ);
                glmTEMP(tempOut2, XYZZ);
                glmTEMP(accumulator2, XYZZ);
        }
    }
    while (gcvFALSE);

    gcmFOOTER();
    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _GenerateLightingFormula
**
**  Generate light-dependent part of the formula.
**
**  INPUT:
**
**      gc
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**      LightIndex
**          Light in interest. For a generic version set to -1.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS generateLightingFormula(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl,
    gctINT LightIndex
    )
{

    gceSTATUS status = gcvSTATUS_OK;
    gctINT i;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x LightIndex=%d",
                    gc, ShaderControl, LightIndex);

    /* Reset temporary registers. */
    ShaderControl->rVPpli        = 0;
    ShaderControl->rVPpliLength  = 0;
    ShaderControl->rNdotVPpli[0] = 0;
    ShaderControl->rNdotVPpli[1] = 0;
    ShaderControl->rAttenuation  = 0;
    ShaderControl->rSpot         = 0;
    ShaderControl->rAmbient[0]   = 0;
    ShaderControl->rAmbient[1]   = 0;
    ShaderControl->rDiffuse[0]   = 0;
    ShaderControl->rDiffuse[1]   = 0;
    ShaderControl->rSpecular[0]  = 0;
    ShaderControl->rSpecular[1]  = 0;

    /* Compute the lighting color. */
    for (i = 0; i < ShaderControl->outputCount; i++)
    {
        /* Compute light ambient component of the formula. */
        gcmERR_BREAK(lightAmbient(
            gc, ShaderControl, LightIndex, i
            ));

        /* Compute diffuse component of the formula. */
        gcmERR_BREAK(lightDiffuse(
            gc, ShaderControl, LightIndex, i
            ));

        /* Compute specular component of the formula. */
        gcmERR_BREAK(lightSpecular(
            gc, ShaderControl, LightIndex, i
            ));

        /* Verify the state of ambient, diffuse and specular factors. */
        if ((ShaderControl->rAmbient[i] == 0) &&
            (ShaderControl->rDiffuse[i] == 0) &&
            (ShaderControl->rSpecular[i] == 0))
        {
            /* Ambient, diffuse and specular factors are all zeros,
               the result for the current light becomes a zero as well. */
            continue;
        }
        /* Compute light attenuation factor (ATTi). */
        gcmERR_BREAK(lightAttenuation(
            gc, ShaderControl, LightIndex
            ));

        /* Compute light spot factor (SPOTi). */
        gcmERR_BREAK(lightSpot(
            gc, ShaderControl, LightIndex
            ));
        /* Finalize the result for the current light. */
        gcmERR_BREAK(lightGetPerLightResult(
            gc, ShaderControl, LightIndex, i
            ));
    }

    gcmFOOTER();
    /* Return status. */
    return status;
}

static gceSTATUS callGenericLightingFunction(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl,
    gctINT LightIndex
    )
{

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x LightIndex=%d",
                    gc, ShaderControl, LightIndex);

    do
    {
        gctUINT label;

        /* Set the index of the light to be worked on. */
        glmOPCODE(MOV, ShaderControl->rLightIndex, X);
            glmCONST(LightIndex);

        /* Generate the call instruction. */
        gcmERR_BREAK(gcFUNCTION_GetLabel(
            ShaderControl->funcLighting,
            &label
            ));

        gcmERR_BREAK(gcSHADER_AddOpcodeConditional(
            ShaderControl->i->shader,
            gcSL_CALL,
            gcSL_ALWAYS,
            label
            ));
    }
    while (gcvFALSE);

    gcmFOOTER();
    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _GetOuputColorFromLighting
**
**  Compute vertex color based on lighting equation.
**
**  INPUT:
**
**      gc
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS getOuputColorFromLighting(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    do
    {
        gctINT i;
        gctUINT lightIndex;
        gctBOOL bUseLightingFunction = checkUseLightingFunction(gc);

        for (i = 0; i < ShaderControl->outputCount; i++)
        {
            /* Generate general lighting component of the lighting formula. */
            gcmERR_BREAK(lightGeneral(gc, ShaderControl, i));
        }

        /* Use generic lighting function when there are more then 4 lights. */
        if (bUseLightingFunction)
        {
            /* Allocate light index register. */
            glmALLOCATE_TEMP(ShaderControl->rLightIndex);

            /* Create lighting function. */
            glmADD_FUNCTION(funcLighting);

            /* Begin function. */
            glmBEGIN_FUNCTION(funcLighting);

            /* Generate generic light formula. */
            gcmERR_BREAK(generateLightingFormula(gc, ShaderControl, -1));

            /* Add the return instruction. */
            glmRET();

            /* End function. */
            glmEND_FUNCTION(funcLighting);

            /* Iterate through lights. */
            for (lightIndex = 0; lightIndex < glvMAX_LIGHTS; lightIndex++)
            {
                if (gc->state.enables.lighting.light[lightIndex])
                {
                    gcmERR_BREAK(callGenericLightingFunction(
                        gc,
                        ShaderControl,
                        lightIndex
                        ));
                }
            }
        }
        else
        {
            /* Iterate through lights. */
            for (lightIndex = 0; lightIndex < glvMAX_LIGHTS; lightIndex++)
            {
                if (gc->state.enables.lighting.light[lightIndex])
                {
                    /* Inline an optimized version. */
                    gcmERR_BREAK(generateLightingFormula(
                        gc,
                        ShaderControl,
                        lightIndex
                        ));
                }
            }
        }

        /* Assign varying. */
        if (gcmIS_SUCCESS(status))
        {
            /* Allocate a temp. */
            glmALLOCATE_LOCAL_TEMP(temp);

            for (i = 0; i < ShaderControl->outputCount; i++)
            {
                if (chipCtx->hashKey.hasSecondaryColorOutput) {
                    /* Allocate color varying. */
                    glmALLOCATE_TEMP(ShaderControl->vColor[i]);

                    /* Clamp. diffuse color */
                    glmOPCODE(SAT, ShaderControl->vColor[i], XYZW);
                        glmTEMP(ShaderControl->rLighting[i], XYZW);

                    glmALLOCATE_TEMP(ShaderControl->vColor2[i]);

                    /* Clamp. specular color */
                    glmOPCODE(SAT, ShaderControl->vColor2[i], XYZW);
                        glmTEMP(ShaderControl->rLighting2[i], XYZW);
                } else {
                    /* Allocate color varying. */
                    glmALLOCATE_TEMP(ShaderControl->vColor[i]);

                    /* Diffuse + Specular */
                    glmOPCODE(ADD, temp, XYZ);
                        glmTEMP(ShaderControl->rLighting[i], XYZZ);
                        glmTEMP(ShaderControl->rLighting2[i], XYZZ);

                    glmOPCODE(MOV, temp, W);
                        glmTEMP(ShaderControl->rLighting[i], WWWW);

                    /* Clamp. */
                    glmOPCODE(SAT, ShaderControl->vColor[i], XYZW);
                        glmTEMP(temp, XYZW);
                }
            }
        }
    }
    while (gcvFALSE);

    gcmFOOTER();
    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _GetOuputColorFromInput
**
**  Determine the proper input color source and set it to the output.
**
**  INPUT:
**
**      gc
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS getOuputColorFromInput(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    do
    {
        /* Get the color from the stream if enabled. */
        if ((chipCtx->attributeInfo[__GL_INPUT_DIFFUSE_INDEX].streamEnabled) &&
            (!chipCtx->drawClearRectEnabled)
            )
        {
            /* Allocate a temp. */
            glmALLOCATE_LOCAL_TEMP(temp);

            /* Allocate color varying. */
            glmALLOCATE_TEMP(ShaderControl->vColor[0]);

            /* Allocate color attribute. */
            glmUSING_ATTRIBUTE(aColor);

            /* Move to the output. */
            glmOPCODE(MOV, temp, XYZW);
                glmATTRIBUTE(VS, aColor, XYZW);

            /* Clamp. */
            glmOPCODE(SAT, ShaderControl->vColor[0], XYZW);
                glmTEMP(temp, XYZW);
        }

        /* get secondary color from the stream */
        if (chipCtx->attributeInfo[__GL_INPUT_SPECULAR_INDEX].streamEnabled)
        {
            /* Allocate a temp. */
            glmALLOCATE_LOCAL_TEMP(temp);

            /* Allocate color varying. */
            glmALLOCATE_TEMP(ShaderControl->vColor2[0]);

            /* Allocate color attribute. */
            glmUSING_ATTRIBUTE(aColor);

            /* Move to the output. */
            glmOPCODE(MOV, temp, XYZW);
                glmATTRIBUTE(VS, aColor, XYZW);

            /* Clamp. */
            glmOPCODE(SAT, ShaderControl->vColor2[0], XYZW);
                glmTEMP(temp, XYZW);
        }
    }
    while (gcvFALSE);

    gcmFOOTER();
    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _PrepareFog
**
**  Compute the eye space vertex position and pass it to the fragment shader.
**
**  INPUT:
**
**      gc
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS processFog(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    do
    {
        if (gc->state.fog.coordSource == GL_FOG_COORDINATE) {
            /* Allocate eye position register. */
            glmALLOCATE_TEMP(ShaderControl->vEyePosition);

            if (chipCtx->attributeInfo[__GL_INPUT_FOGCOORD_INDEX].streamEnabled) {
                glmOPCODE(MOV, ShaderControl->vEyePosition, X);
                    glmATTRIBUTE(VS, aFogCoord, XXXX);
            } else {
                glmUSING_UNIFORM(uFogCoord);

                glmOPCODE(MOV, ShaderControl->vEyePosition, X);
                    glmUNIFORM_STATIC(VS, uFogCoord, XXXX, 0);
            }
        } else {
            /* Compute current position in eye space. */
            gcmERR_BREAK(pos2Eye(gc, ShaderControl));

            /* Allocate eye position register. */
            glmALLOCATE_TEMP(ShaderControl->vEyePosition);

            /* We take the "easy way out" per spec, only need Z. */
            glmOPCODE(MOV, ShaderControl->vEyePosition, X);
                glmTEMP(ShaderControl->rVtxInEyeSpace, ZZZZ);
        }
    }
    while (gcvFALSE);

    gcmFOOTER();
    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _TransformTextureCoordinates
**
**  Transform texture coordinates as necessary.
**
**  INPUT:
**
**      gc
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS transformTextureCoordinates(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;
    GLuint stageEnableMask;
    GLuint genEnable;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    do
    {
        glmDEFINE_TEMP(reflection) = 0;
        glmDEFINE_TEMP(eyeLinear) = 0;
        glmDEFINE_TEMP(objectLinear) = 0;
        glmDEFINE_TEMP(sphereLinear) = 0;
        /* Define a label. */
        glmDEFINE_LABEL(lblZero);


        gctBOOL needNormal;
        gctBOOL needReflection;
        gctBOOL needObj;
        gctBOOL needEye;
        gctBOOL needSphere;

        GLint i = 0;
        GLuint j;

        if (chipCtx->drawClearRectEnabled)
        {
            status = gcvSTATUS_OK;
            break;
        }

        if (chipCtx->hashKey.accumMode != gccACCUM_UNKNOWN)
        {
            stageEnableMask = 3;
        }
        else
        {
            stageEnableMask = chipCtx->texture.stageEnabledMask;
        }
        /* Scan all stages and determine control flags. */
        while ((i < gc->constants.numberOfTextureUnits) && stageEnableMask)
        {
            glsTEXTURESAMPLER_PTR sampler = &chipCtx->texture.sampler[i];

            /* Skip the stage if disabled. */
            if (!(stageEnableMask & 1)) {
                stageEnableMask >>= 1;
                i++;
                continue;
            }

            genEnable = sampler->genEnable;
            needNormal = GL_FALSE;
            needReflection = GL_FALSE;
            needObj = GL_FALSE;
            needEye = GL_FALSE;
            needSphere = GL_FALSE;
            j = 0;
            while (genEnable && (j < 4)) {
                if (genEnable & 1) {
                    switch(sampler->genMode[j]) {
                        case glvTEXNORMAL:
                            needNormal = GL_TRUE;
                            break;
                        case glvREFLECTION:
                            needNormal = GL_TRUE;
                            needReflection = GL_TRUE;
                            break;
                        case glvSPHERE:
                            needNormal = GL_TRUE;
                            needSphere = GL_TRUE;
                            break;
                        case glvOBJECTLINEAR:
                            needObj = GL_TRUE;
                            break;
                        case glvEYELINEAR:
                            needEye = GL_TRUE;
                            break;
                    }
                }
                j++;
                genEnable >>= 1;
            }

            if (needObj)
            {
                /* Optimize later */
                /* Allocate temporaries. */
                glmALLOCATE_TEMP(objectLinear);
                /* Allocate resources. */
                glmUSING_UNIFORM(uTexGenObjectPlane);

                /* dp4 s, aPosition, s.ObjectPlane */
                glmOPCODE(DP4, objectLinear, X);
                    glmATTRIBUTE(VS, aPosition, XYZW);
                    glmUNIFORM_STATIC(VS, uTexGenObjectPlane, XYZW, i * 4);

                /* dp4 t, aPosition, t.ObjectPlane */
                glmOPCODE(DP4, objectLinear, Y);
                     glmATTRIBUTE(VS, aPosition, XYZW);
                     glmUNIFORM_STATIC(VS, uTexGenObjectPlane, XYZW, i * 4 + 1);

                 /* dp4 r, aPosition, r.ObjectPlane */
                 glmOPCODE(DP4, objectLinear, Z);
                     glmATTRIBUTE(VS, aPosition, XYZW);
                 glmUNIFORM_STATIC(VS, uTexGenObjectPlane, XYZW, i * 4 + 2);

                 /* dp4 q, aPosition, q.ObjectPlane */
                 glmOPCODE(DP4, objectLinear, W);
                     glmATTRIBUTE(VS, aPosition, XYZW);
                     glmUNIFORM_STATIC(VS, uTexGenObjectPlane, XYZW, i * 4 + 3);
            }

            if (needEye)
            {
                /* Optimize later */
                pos2Eye(gc, ShaderControl);
                /* Allocate temporaries. */
                glmALLOCATE_TEMP(eyeLinear);
                /* Allocate resources. */
                glmUSING_UNIFORM(uTexGenEyePlane);

                /* dp4 s, aPosition, s.ObjectPlane */
                glmOPCODE(DP4, eyeLinear, X);
                    glmTEMP(ShaderControl->rVtxInEyeSpace, XYZW);
                    glmUNIFORM_STATIC(VS, uTexGenEyePlane, XYZW, i * 4);

                /* dp4 t, aPosition, t.ObjectPlane */
                glmOPCODE(DP4, eyeLinear, Y);
                    glmTEMP(ShaderControl->rVtxInEyeSpace, XYZW);
                    glmUNIFORM_STATIC(VS, uTexGenEyePlane, XYZW, i * 4 + 1);

                /* dp4 r, aPosition, r.ObjectPlane */
                glmOPCODE(DP4, eyeLinear, Z);
                    glmTEMP(ShaderControl->rVtxInEyeSpace, XYZW);
                    glmUNIFORM_STATIC(VS, uTexGenEyePlane, XYZW, i * 4 + 2);

                /* dp4 q, aPosition, q.ObjectPlane */
                glmOPCODE(DP4, eyeLinear, W);
                    glmTEMP(ShaderControl->rVtxInEyeSpace, XYZW);
                    glmUNIFORM_STATIC(VS, uTexGenEyePlane, XYZW, i * 4 + 3);
            }

            /* Generate the current normal if needed. */
            if (needNormal)
            {
                /* Transform normal to eye space. */
                gcmERR_BREAK(normal2Eye(
                    gc, ShaderControl
                    ));
            }

            /* Generate the reflection vector if needed. */
            if (needReflection || needSphere)
            {
                /* Define unit vectors. */
                glmDEFINE_TEMP(unitNormal);
                glmDEFINE_TEMP(unitPosition);

                /* Allocate temporaries. */
                glmALLOCATE_LOCAL_TEMP(temp1);
                glmALLOCATE_LOCAL_TEMP(temp2);
                glmALLOCATE_LOCAL_TEMP(temp3);

                /**********************************************************
                ** Reflection vector is computed using the following      **
                ** formula:                                               **
                **                                                        **
                ** r = u - 2 * n * (n DOT u)                              **
                **                                                        **
                ** where u is a unit vector pointing from the origin to   **
                **         the vertex (in eye coordinates);               **
                **       n is the current transformed normal in eye       **
                **         coordinates.                                   **
                **                                                        **
                 **********************************************************/

                /***********************************************************
                ** Normalize the normal vector.
                */

                if (gc->state.enables.transform.normalize)
                {
                    /* Already normalized. */
                    unitNormal = ShaderControl->rNrmInEyeSpace[0];
                }
                else
                {
                    /* Allocate new temp. */
                    glmALLOCATE_TEMP(unitNormal);

                    /* unitNormal = norm(rNrmInEyeSpace[0]) */
                    glmOPCODE(NORM, unitNormal, XYZ);
                        glmTEMP(ShaderControl->rNrmInEyeSpace[0], XYZZ);
                }

                /***********************************************************
                ** Normalize the position vector.
                */

                /* Allocate new temp. */
                glmALLOCATE_TEMP(unitPosition);
                /* Transform position to eye space. */
                gcmERR_BREAK(pos2Eye(
                    gc, ShaderControl
                    ));

                if (chipCtx->chipModel >= gcv1000 || chipCtx->chipModel == gcv880)
                {
                    /* Allocate temporaries. */
                    glmALLOCATE_LOCAL_TEMP(temp4);
                    glmALLOCATE_LOCAL_TEMP(temp5);

                    /* mov postion to temp. */
                    glmOPCODE(MOV, temp5, XYZW);
                        glmTEMP(ShaderControl->rVtxInEyeSpace, XYZW);

                    /* get position ( Z + W ) / 2. */
                    glmOPCODE(ADD, temp4, Z);
                        glmTEMP(ShaderControl->rVtxInEyeSpace, ZZZZ);
                            glmTEMP(ShaderControl->rVtxInEyeSpace, WWWW);

                    glmOPCODE(MUL, temp5, Z);
                        glmTEMP(temp4, ZZZZ);
                        glmCONST(0.5);

                    /* Normalize the position vector. */
                    glmOPCODE(NORM, unitPosition, XYZ);
                        glmTEMP(temp5, XYZZ);
                }
                else
                {
                    /* Normalize the position vector. */
                    glmOPCODE(NORM, unitPosition, XYZ);
                        glmTEMP(ShaderControl->rVtxInEyeSpace, XYZZ);
                }
                /***********************************************************
                ** Find (n DOT u) = (normal DOT vPosition.norm).
                */

                glmOPCODE(DP3, temp1, X);
                    glmTEMP(unitNormal, XYZZ);
                    glmTEMP(unitPosition, XYZZ);

                /***********************************************************
                ** Find (2 * normal * (normal DOT vPosition.norm)).
                */

                glmOPCODE(MUL, temp2, XYZ);
                    glmTEMP(unitNormal, XYZZ);
                    glmTEMP(temp1, XXXX);

                glmOPCODE(MUL, temp3, XYZ);
                    glmTEMP(temp2, XYZZ);
                    glmCONST(2);

                /***********************************************************
                ** Find the reflection vector.
                */

                /* Allocate the vector. */
                glmALLOCATE_TEMP(reflection);

                /* Compute reflection. */
                glmOPCODE(SUB, reflection, XYZ);
                    glmTEMP(unitPosition, XYZZ);
                    glmTEMP(temp3, XYZZ);
            }

            if (needSphere)
            {
                /* Allocate the vector. */
                glmALLOCATE_LOCAL_TEMP(temp1);
                glmALLOCATE_LOCAL_TEMP(temp2);
                /* Allocate the label. */
                glmALLOCATE_LABEL(lblZero);

                glmALLOCATE_TEMP(sphereLinear);

                ///* temp1 = reflection */
                glmOPCODE(MOV, temp1, XYZ);
                     glmTEMP(reflection, XYZZ);

                /* temp2.z = (z+1) */
                glmOPCODE(ADD, temp2, Z);
                     glmTEMP(temp1, ZZZZ);
                     glmCONST(1);

                /* temp1 = (x, y, z+1) */
                glmOPCODE(MOV, temp1, Z);
                     glmTEMP(temp2, ZZZZ);

                /* temp2.x = x^2 + y^2 + (z+1)^2 */
                glmOPCODE(DP3, temp2, X);
                    glmTEMP(temp1, XYZZ);
                    glmTEMP(temp1, XYZZ);

                glmOPCODE(MOV, temp1, X);
                   glmCONST(1);

                /* Make sure Z is not zero, otherwize RSQ will produce an INF. */
                glmOPCODE_BRANCH(JMP, EQUAL, lblZero);
                   glmTEMP(temp2, XXXX);
                   glmCONST(0);
                {

                    /* temp1.x = 1 / sqrt (temp2.x); 1 / sqrt(x^2 + y^2 + (z+1)^2) */
                    glmOPCODE(RSQ, temp1, X);
                        glmTEMP(temp2, XXXX);

                    /* temp2.x = 0.5 * temp1.x */
                    glmOPCODE(MUL, temp2, X);
                        glmTEMP(temp1, XXXX);
                        glmCONST(0.5f);

                    glmOPCODE(MOV, temp1, X);
                        glmTEMP(temp2, XXXX);
                }

                /* Define label. */
                glmLABEL(lblZero);

                glmOPCODE(MUL, temp2, XY);
                    glmTEMP(reflection, XYYY);
                    glmTEMP(temp1, XXXX);

                /* sphereLinear = temp1.x */
                glmOPCODE(ADD, sphereLinear, XY);
                    glmTEMP(temp2, XYYY);
                    glmCONST(0.5f);
            }

            /* Coordinate generation enabled (GL_OES_texture_cube_map) ? */
            if (sampler->genEnable)
            {
                genEnable = sampler->genEnable;
                /* Allocate texture coordinate register. */
                glmALLOCATE_TEMP(ShaderControl->vTexCoord[i]);
                /* Set X to 0.0. */
                glmOPCODE(MOV, ShaderControl->vTexCoord[i], X);
                    glmCONST(0);
                /* Set Y to 0.0. */
                glmOPCODE(MOV, ShaderControl->vTexCoord[i], Y);
                    glmCONST(0);
                /* Set Z to 0.0. */
                glmOPCODE(MOV, ShaderControl->vTexCoord[i], Z);
                    glmCONST(0);
                /* Set Q to 1.0. */
                glmOPCODE(MOV, ShaderControl->vTexCoord[i], W);
                    glmCONST(1);

                j = 0;
                while (genEnable && (j < 4)) {
                    /* Normal vector based coordinates? */
                    if (genEnable & 1) {
                        switch(sampler->genMode[j]) {
                            case glvTEXNORMAL:
                                switch (j) {
                                    case 0:
                                        glmOPCODE(MOV, ShaderControl->vTexCoord[i], X);
                                            glmTEMP(ShaderControl->rNrmInEyeSpace[0], XYZZ);
                                        break;
                                    case 1:
                                        glmOPCODE(MOV, ShaderControl->vTexCoord[i], Y);
                                            glmTEMP(ShaderControl->rNrmInEyeSpace[0], XYZZ);
                                        break;
                                    case 2:
                                        glmOPCODE(MOV, ShaderControl->vTexCoord[i], Z);
                                            glmTEMP(ShaderControl->rNrmInEyeSpace[0], XYZZ);
                                        break;
                                }
                                break;
                            case glvREFLECTION:
                                switch (j) {
                                    case 0:
                                        glmOPCODE(MOV, ShaderControl->vTexCoord[i], X);
                                            glmTEMP(reflection, XYZZ);
                                        break;
                                    case 1:
                                        glmOPCODE(MOV, ShaderControl->vTexCoord[i], Y);
                                            glmTEMP(reflection, XYZZ);
                                        break;
                                    case 2:
                                        glmOPCODE(MOV, ShaderControl->vTexCoord[i], Z);
                                            glmTEMP(reflection, XYZZ);
                                        break;
                                }
                                break;
                            case glvOBJECTLINEAR:
                                switch (j) {
                                    case 0:
                                        glmOPCODE(MOV, ShaderControl->vTexCoord[i], X);
                                            glmTEMP(objectLinear, XYZW);
                                        break;
                                    case 1:
                                        glmOPCODE(MOV, ShaderControl->vTexCoord[i], Y);
                                            glmTEMP(objectLinear, XYZW);
                                        break;
                                    case 2:
                                        glmOPCODE(MOV, ShaderControl->vTexCoord[i], Z);
                                            glmTEMP(objectLinear, XYZW);
                                        break;
                                    case 3:
                                        glmOPCODE(MOV, ShaderControl->vTexCoord[i], W);
                                            glmTEMP(objectLinear, XYZW);
                                        break;
                                }
                                break;
                            case glvEYELINEAR:
                                switch (j) {
                                    case 0:
                                        glmOPCODE(MOV, ShaderControl->vTexCoord[i], X);
                                            glmTEMP(eyeLinear, XYZW);
                                        break;
                                    case 1:
                                        glmOPCODE(MOV, ShaderControl->vTexCoord[i], Y);
                                            glmTEMP(eyeLinear, XYZW);
                                        break;
                                    case 2:
                                        glmOPCODE(MOV, ShaderControl->vTexCoord[i], Z);
                                            glmTEMP(eyeLinear, XYZW);
                                        break;
                                    case 3:
                                        glmOPCODE(MOV, ShaderControl->vTexCoord[i], W);
                                            glmTEMP(eyeLinear, XYZW);
                                        break;
                                }
                                break;
                            case glvSPHERE:
                                switch (j) {
                                    case 0:
                                        glmOPCODE(MOV, ShaderControl->vTexCoord[i], X);
                                            glmTEMP(sphereLinear, XYYY);
                                        break;
                                    case 1:
                                        glmOPCODE(MOV, ShaderControl->vTexCoord[i], Y);
                                            glmTEMP(sphereLinear, XYYY);
                                        break;
                                }
                                break;
                        }
                    }
                    j++;
                    genEnable >>= 1;
                }
                /* Transform texture coordinates. */
                if (gc->transform.texture[i]->matrix.matrixType != __GL_MT_IDENTITY)
                {
                    /* Define a pre-transform texture coordinate temp register. */
                    glmDEFINE_TEMP(preXformTexCoord);

                    /* Allocate texture transformation matrix. */
                    glmUSING_UNIFORM(uTexMatrix);

                    /* Set the pre-transform register. */
                    preXformTexCoord = ShaderControl->vTexCoord[i];

                    /* Allocate transformed texture coordinate register. */
                    glmALLOCATE_TEMP(ShaderControl->vTexCoord[i]);

                    /* vTexCoord[i].x = dp4(preXformTexCoord, uTexMatrix[i * 4 + 0]) */
                    glmOPCODE(DP4, ShaderControl->vTexCoord[i], X);
                        glmTEMP(preXformTexCoord, XYZW);
                        glmUNIFORM_STATIC(VS, uTexMatrix, XYZW, i * 4 + 0);

                    /* vTexCoord[i].y = dp4(preXformTexCoord, uTexMatrix[i * 4 + 1]) */
                    glmOPCODE(DP4, ShaderControl->vTexCoord[i], Y);
                        glmTEMP(preXformTexCoord, XYZW);
                        glmUNIFORM_STATIC(VS, uTexMatrix, XYZW, i * 4 + 1);

                    /* vTexCoord[i].z = dp4(preXformTexCoord, uTexMatrix[i * 4 + 2]) */
                    glmOPCODE(DP4, ShaderControl->vTexCoord[i], Z);
                        glmTEMP(preXformTexCoord, XYZW);
                        glmUNIFORM_STATIC(VS, uTexMatrix, XYZW, i * 4 + 2);

                    glmOPCODE(DP4, ShaderControl->vTexCoord[i], W);
                        glmTEMP(preXformTexCoord, XYZW);
                        glmUNIFORM_STATIC(VS, uTexMatrix, XYZW, i * 4 + 3);
                }
            }
            /* Retrieve texture coordinate from the coordinate stream. */
            else if (chipCtx->attributeInfo[i + __GL_INPUT_TEX0_INDEX].streamEnabled)
            {
                /* Define texture stream. */
                glmUSING_INDEXED_ATTRIBUTE(aTexCoord, i);

                /* Transform texture coordinates. */
                if (gc->transform.texture[i]->matrix.matrixType == __GL_MT_IDENTITY)
                {
                    /* Allocate texture coordinate register. */
                    glmALLOCATE_TEMP(ShaderControl->vTexCoord[i]);

                    /* Copy two components form input to output. */
                    if (sampler->coordType == gcSHADER_FLOAT_X2)
                    {
                        glmOPCODE(MOV, ShaderControl->vTexCoord[i], XY);
                            glmATTRIBUTE_INDEXED(VS, aTexCoord0, i, XYYY);
                    }

                    /* Copy three components form input to output. */
                    else if (sampler->coordType == gcSHADER_FLOAT_X3)
                    {
                        glmOPCODE(MOV, ShaderControl->vTexCoord[i], XYZ);
                            glmATTRIBUTE_INDEXED(VS, aTexCoord0, i, XYZZ);
                    }

                    /* Copy four components form input to output. */
                    else
                    {
                        /* Allocate a temp. */
                        //glmALLOCATE_LOCAL_TEMP(temp);
                        //glmALLOCATE_LOCAL_TEMP(temp1);

                        /* Make sure it's four. */
                        gcmASSERT(sampler->coordType == gcSHADER_FLOAT_X4);

                        /* Get the stream value. */
                        glmOPCODE(MOV, ShaderControl->vTexCoord[i], XYZW);
                            glmATTRIBUTE_INDEXED(VS, aTexCoord0, i, XYZW);

                        ///* Make homogeneous vector. */
                        //glmOPCODE(RCP, temp, X);
                        //    glmTEMP(ShaderControl->vTexCoord[i], WWWW);

                        //glmOPCODE(MOV, temp1, XYZW);
                        //    glmTEMP(ShaderControl->vTexCoord[i], XYZW);

                        //glmOPCODE(MUL, ShaderControl->vTexCoord[i], XYZW);
                        //    glmTEMP(temp1, XYZW);
                        //    glmTEMP(temp, XXXX);
                    }
                }
                else
                {
                    /* Allocate a temp. */
                    glmALLOCATE_LOCAL_TEMP(temp1);

                    /* Allocate texture coordinate varying. */
                    glmALLOCATE_TEMP(ShaderControl->vTexCoord[i]);

                    /* Allocate texture transformation matrix. */
                    glmUSING_UNIFORM(uTexMatrix);


                    /*sampler->coordType = gcSHADER_FLOAT_X4;*/
                    {

                        if (sampler->coordType == gcSHADER_FLOAT_X2)
                        {
                            /* Get the stream value. */
                            glmOPCODE(MOV, temp1, XY);
                                glmATTRIBUTE_INDEXED(VS, aTexCoord0, i, XYYY);

                            glmOPCODE(MOV, temp1, Z);
                                glmCONST(0.0);
                            glmOPCODE(MOV, temp1, W);
                                glmCONST(1.0);
                        }
                        else if (sampler->coordType == gcSHADER_FLOAT_X3)
                        {
                            glmOPCODE(MOV, temp1, XYZ);
                                glmATTRIBUTE_INDEXED(VS, aTexCoord0, i, XYZZ);
                        }
                        else if (sampler->coordType == gcSHADER_FLOAT_X4)
                        {
                            glmOPCODE(MOV, temp1, XYZW);
                                glmATTRIBUTE_INDEXED(VS, aTexCoord0, i, XYZW);
                        }

                        /* vTexCoord[i].x = dp4(temp1, uTexMatrix[i * 4 + 0]) */
                        glmOPCODE(DP4, ShaderControl->vTexCoord[i], X);
                            glmTEMP(temp1, XYZW);
                            glmUNIFORM_STATIC(VS, uTexMatrix, XYZW, i * 4 + 0);

                        /* vTexCoord[i].y = dp4(temp1, uTexMatrix[i * 4 + 1]) */
                        glmOPCODE(DP4, ShaderControl->vTexCoord[i], Y);
                            glmTEMP(temp1, XYZW);
                            glmUNIFORM_STATIC(VS, uTexMatrix, XYZW, i * 4 + 1);

                        /* vTexCoord[i].z = dp4(temp1, uTexMatrix[i * 4 + 2]) */
                        glmOPCODE(DP4, ShaderControl->vTexCoord[i], Z);
                            glmTEMP(temp1, XYZW);
                            glmUNIFORM_STATIC(VS, uTexMatrix, XYZW, i * 4 + 2);

                        /* vTexCoord[i].w = dp4(temp1, uTexMatrix[i * 4 + 3]) */
                        glmOPCODE(DP4, ShaderControl->vTexCoord[i], W);
                            glmTEMP(temp1, XYZW);
                            glmUNIFORM_STATIC(VS, uTexMatrix, XYZW, i * 4 + 3);
                    }
                }
            }

            /* Retrieve texture coordinate from the constant. */
            else
            {
                /* We only send constant texture coordinates through if
                   point sprite is enabled because in this case the
                   coordinate has to be properly interpolated. */
                if (chipCtx->pointStates.spriteActive)
                {
                    /* Allocate uniform. */
                    glmUSING_UNIFORM(uTexCoord);

                    /* Allocate texture coordinate register. */
                    glmALLOCATE_TEMP(ShaderControl->vTexCoord[i]);

                    /* Copy four components form input to output. */
                    glmOPCODE(MOV, ShaderControl->vTexCoord[i], XYZW);
                        glmUNIFORM_STATIC(VS, uTexCoord, XYZW, i);
                }
            }
            stageEnableMask >>= 1;
            i++;
        }
    }
    while (gcvFALSE);

    gcmFOOTER();
    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _TransformClipPlane
**
**  Transform clip plane values.
**
**  INPUT:
**
**      gc
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS transformClipPlane(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    do
    {
        GLint i = 0;
        GLuint clipPlanEnableMask;

        /* Ignore clip planes if DrawTex extension is in use. */
        if (chipCtx->drawTexOESEnabled)
        {
            break;
        }

        /* Ignore clip planes if DrawClearRect is in use. */
        if (chipCtx->drawClearRectEnabled)
        {
            break;
        }

        clipPlanEnableMask = gc->state.enables.transform.clipPlanesMask;
        while ((i < glvMAX_CLIP_PLANES) && clipPlanEnableMask)
        {
            if (clipPlanEnableMask & 1)
            {
                /* Compute current position in eye space. */
                gcmERR_BREAK(pos2Eye(gc, ShaderControl));

                /* Allocate clip plane varyinga temporary for the input. */
                glmALLOCATE_TEMP(ShaderControl->vClipPlane[i]);

                /* Allocate resources. */
                glmUSING_UNIFORM(uClipPlane);

                /* Transform clip plane. */
                glmOPCODE(DP4, ShaderControl->vClipPlane[i], X);
                    glmTEMP(ShaderControl->rVtxInEyeSpace, XYZW);
                    glmUNIFORM_STATIC(VS, uClipPlane, XYZW, i);
            }
            i++;
            clipPlanEnableMask >>= 1;
        }
    }
    while (gcvFALSE);

    gcmFOOTER();
    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _MapVaryings
**
**  Map all used varyings.
**
**  INPUT:
**
**      gc
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS mapVaryings(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status = gcvSTATUS_OK;
    GLint i;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    do
    {
        glmASSIGN_OUTPUT(vPosition);
        glmASSIGN_OUTPUT(vEyePosition);
        glmASSIGN_INDEXED_OUTPUT(vColor, 0);
        glmASSIGN_INDEXED_OUTPUT(vColor, 1);
        glmASSIGN_INDEXED_OUTPUT(vColor2, 0);
        glmASSIGN_INDEXED_OUTPUT(vColor2, 1);
        glmASSIGN_OUTPUT(vPointSize);
        glmASSIGN_OUTPUT(vPointFade);
        glmASSIGN_OUTPUT(vPointSmooth);

        for (i = 0; i < gc->constants.numberOfTextureUnits; i++)
        {
            if (ShaderControl->vTexCoord[i] != 0)
            {
                glmASSIGN_INDEXED_OUTPUT(vTexCoord, i);
            }
        }

        if (gcmIS_ERROR(status))
        {
            break;
        }

        for (i = 0; i < glvMAX_CLIP_PLANES; i++)
        {
            if (ShaderControl->vClipPlane[i] != 0)
            {
                glmASSIGN_INDEXED_OUTPUT(vClipPlane, i);
            }
        }

        if (gcmIS_ERROR(status))
        {
            break;
        }
    }
    while (gcvFALSE);
    gcmFOOTER();
    /* Return status. */
    return status;
}

/*******************************************************************************
**
**  _InitializeVS
**
**  INitialize common things for a vertex shader.
**
**  INPUT:
**
**      gc
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS initializeVS(
    __GLcontext * gc,
    glsVSCONTROL_PTR ShaderControl
    )
{

    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    do
    {
        /* Allocate the ModelViewProjection uniform. */
        glmUSING_UNIFORM(uModelViewProjection);

        /* Allocate the TexMatrix uniform. */
        glmUSING_UNIFORM(uTexMatrix);
    }
    while (gcvFALSE);
    gcmFOOTER();
    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  glfGenerateVSFixedFunction
**
**  Generate Vertex Shader code.
**
**  INPUT:
**
**      gc
**          Pointer to the current context.
**
**  OUTPUT:
**
**      Nothing.
*/

gceSTATUS glfGenerateVSFixedFunction(
    __GLcontext * gc
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;
    /* Define internal shader structure. */
    glsVSCONTROL vsControl;
    gcmHEADER_ARG("gc=0x%x", gc);

    do
    {
        gcoOS_ZeroMemory(&vsControl, sizeof(glsVSCONTROL));
        vsControl.i = &chipCtx->currProgram->vs;

        if (vsControl.i->shader == gcvNULL)
        {
            /* Shader may have been deleted. Construct again. */
            gcmONERROR(gcSHADER_Construct(
                chipCtx->hal,
                gcSHADER_TYPE_VERTEX,
                &vsControl.i->shader
                ));
        }

        /* Determine the number of color outputs. */
        vsControl.outputCount
            = (gc->state.enables.lighting.lighting
            && gc->state.light.model.twoSided)
                ? 2
                : 1;

        /* Initialize the vertex shader. */
        gcmONERROR(initializeVS(gc, &vsControl));

        /* Determine the output position. */
        gcmONERROR(pos2Clip(gc, &vsControl));

        /* Determine point output parameters. */
        if (gc->vertexStreams.primMode == GL_POINTS)
        {
            /*******************************************************************
            ** The points can be rendered in three different ways:
            **
            ** 1. Point sprites (point.sprite).
            **    When enabled, the antialiased enable state (point.smooth)
            **    is ignored. The exact vertex position is used as the center
            **    and the point size as the side length of the square. Texture
            **    coordinates are replaced for units with GL_COORD_REPLACE_OES
            **    enabled. The result is a square.
            **
            **    The hardware computes and substitutes the texture coordinates,
            **    therefore there is nothing special left for the shader to do.
            **
            ** 2. Antialiased point (point.smooth).
            **    Use exact vertex position as the center and point size
            **    as the diameter of the smooth point with blended edge.
            **
            **    Shader needs to compute alpha values to make a circle out
            **    of the rasterized square.
            **
            ** 3. Non-antialiased point (not sprite and not smooth).
            **    Use integer vertex position as the center and point size
            **    as the side length of the resulting square.
            **
            **    The hardware adjusts the center, nothing special for the
            **    shader to do.
            **
            */

            /* Compute point size. */
            gcmONERROR(computePointSize(gc, &vsControl));

            /* Compute point smooth. */
            if (chipCtx->pointStates.smooth
                && !chipCtx->pointStates.spriteEnable)
            {
                gcmONERROR(computePointSmooth(gc, &vsControl));
            }
        }

        /* Determine output color. */
        if (gc->state.enables.lighting.lighting)
        {
            gcmONERROR(getOuputColorFromLighting(gc, &vsControl));
        }
        else
        {
            gcmONERROR(getOuputColorFromInput(gc, &vsControl));
        }

        /* Pass the eye space position if fog is enabled. */
        if (gc->state.enables.fog)
        {
            gcmONERROR(processFog(gc, &vsControl));
        }

        /* Determine texture coordinates. */
        gcmONERROR(transformTextureCoordinates(gc, &vsControl));

        /* Calculate clip plane equation. */
        gcmONERROR(transformClipPlane(gc, &vsControl));

        /* Map varyings. */
        gcmONERROR(mapVaryings(gc, &vsControl));

        /* Pack the shader. */
        gcmONERROR(gcSHADER_Pack(vsControl.i->shader));

        /* Optimize the shader. */
#if !GC_ENABLE_LOADTIME_OPT
        gcmONERROR(gcSHADER_SetOptimizationOption(vsControl.i->shader, gcvOPTIMIZATION_FULL & ~gcvOPTIMIZATION_LOADTIME_CONSTANT));
        gcmONERROR(gcOptimizeShader(vsControl.i->shader, gcvNULL));
#endif
    }
    while (gcvFALSE);

    gcmFOOTER();
    /* Return status. */
    return status;

OnError:
    /* Destroy partially generated shader. */
    if (vsControl.i->shader)
    {
        gcmVERIFY_OK(gcSHADER_Destroy(vsControl.i->shader));
        vsControl.i->shader = gcvNULL;
    }

    gcmFOOTER();
    /* Return status. */
    return status;
}
