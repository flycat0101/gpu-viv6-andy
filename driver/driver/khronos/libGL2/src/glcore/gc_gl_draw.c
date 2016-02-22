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
#include "gl/gl_device.h"
#include "dri/viv_lock.h"
#include "gc_gl_debug.h"

/* For fix function path, the first 8 samplers are used for every texture unit */
GLuint fixFuncSampler2TexUnit[__GL_MAX_GLSL_SAMPLERS] = {
    0, 1, 2, 3, 4, 5, 6, 7,
    __GL_MAX_TEXTURE_UNITS, __GL_MAX_TEXTURE_UNITS, __GL_MAX_TEXTURE_UNITS, __GL_MAX_TEXTURE_UNITS,
    __GL_MAX_TEXTURE_UNITS, __GL_MAX_TEXTURE_UNITS, __GL_MAX_TEXTURE_UNITS, __GL_MAX_TEXTURE_UNITS,
    __GL_MAX_TEXTURE_UNITS, __GL_MAX_TEXTURE_UNITS, __GL_MAX_TEXTURE_UNITS, __GL_MAX_TEXTURE_UNITS,
    __GL_MAX_TEXTURE_UNITS, __GL_MAX_TEXTURE_UNITS, __GL_MAX_TEXTURE_UNITS, __GL_MAX_TEXTURE_UNITS,
    __GL_MAX_TEXTURE_UNITS, __GL_MAX_TEXTURE_UNITS, __GL_MAX_TEXTURE_UNITS, __GL_MAX_TEXTURE_UNITS,
    __GL_MAX_TEXTURE_UNITS, __GL_MAX_TEXTURE_UNITS, __GL_MAX_TEXTURE_UNITS, __GL_MAX_TEXTURE_UNITS,
    __GL_MAX_TEXTURE_UNITS, __GL_MAX_TEXTURE_UNITS, __GL_MAX_TEXTURE_UNITS, __GL_MAX_TEXTURE_UNITS,
    __GL_MAX_TEXTURE_UNITS, __GL_MAX_TEXTURE_UNITS, __GL_MAX_TEXTURE_UNITS, __GL_MAX_TEXTURE_UNITS,
    __GL_MAX_TEXTURE_UNITS, __GL_MAX_TEXTURE_UNITS, __GL_MAX_TEXTURE_UNITS, __GL_MAX_TEXTURE_UNITS,
    __GL_MAX_TEXTURE_UNITS, __GL_MAX_TEXTURE_UNITS, __GL_MAX_TEXTURE_UNITS, __GL_MAX_TEXTURE_UNITS
};

extern GLboolean __glIsTextureConsistent(__GLcontext *gc, __GLtextureObject *tex);

/* s_program.c */
extern GLvoid __glVertexProgramRealEnabled(__GLcontext * gc);
extern GLvoid __glFragmentProgramRealEnabled(__GLcontext * gc);

/* s_shader.c*/
extern GLvoid __glSLangBuildTextureEnableDim(__GLcontext *gc);

#define __GL_CHECK_ATTR1(bit, attr1)                \
    if (localMask & bit) {                          \
        if (ds->attr1 != cs->attr1)                 \
        {                                           \
            ds->attr1 = cs->attr1;                  \
        }                                           \
        else {                                      \
            localMask &= ~bit;                      \
        }                                           \
    }

#define __GL_CHECK_ATTR2(bit, attr1, attr2)         \
    if (localMask & bit) {                          \
        if (ds->attr1 != cs->attr1 ||               \
            ds->attr2 != cs->attr2)                 \
        {                                           \
            ds->attr1 = cs->attr1;                  \
            ds->attr2 = cs->attr2;                  \
        }                                           \
        else {                                      \
            localMask &= ~bit;                      \
        }                                           \
    }

#define __GL_CHECK_ATTR3(bit, attr1, attr2, attr3)  \
    if (localMask & bit) {                          \
        if (ds->attr1 != cs->attr1 ||               \
            ds->attr2 != cs->attr2 ||               \
            ds->attr3 != cs->attr3)                 \
        {                                           \
            ds->attr1 = cs->attr1;                  \
            ds->attr2 = cs->attr2;                  \
            ds->attr3 = cs->attr3;                  \
        }                                           \
        else {                                      \
            localMask &= ~bit;                      \
        }                                           \
    }

#define __GL_CHECK_ATTR4(bit, attr1, attr2, attr3, attr4) \
    if (localMask & bit) {                          \
        if (ds->attr1 != cs->attr1 ||               \
            ds->attr2 != cs->attr2 ||               \
            ds->attr3 != cs->attr3 ||               \
            ds->attr4 != cs->attr4)                 \
        {                                           \
            ds->attr1 = cs->attr1;                  \
            ds->attr2 = cs->attr2;                  \
            ds->attr3 = cs->attr3;                  \
            ds->attr4 = cs->attr4;                  \
        }                                           \
        else {                                      \
            localMask &= ~bit;                      \
        }                                           \
    }

#define __GL_CHECK_ATTR5(bit, attr1, attr2, attr3, attr4, attr5) \
    if (localMask & bit) {                          \
        if (ds->attr1 != cs->attr1 ||               \
            ds->attr2 != cs->attr2 ||               \
            ds->attr3 != cs->attr3 ||               \
            ds->attr4 != cs->attr4 ||               \
            ds->attr5 != cs->attr5)                 \
        {                                           \
            ds->attr1 = cs->attr1;                  \
            ds->attr2 = cs->attr2;                  \
            ds->attr3 = cs->attr3;                  \
            ds->attr4 = cs->attr4;                  \
            ds->attr5 = cs->attr5;                  \
        }                                           \
        else {                                      \
            localMask &= ~bit;                      \
        }                                           \
    }

#define __GL_CHECK_TEX_PARAM1(bit, attr)            \
    if (localMask & bit) {                          \
        if (ds_params->attr != cs_params->attr) {   \
            ds_params->attr = cs_params->attr;      \
        }                                           \
        else {                                      \
            localMask &= ~bit;                      \
        }                                           \
    }

#define __GL_CHECK_TEX_PARAM_BORDERCOLOR()                              \
    if (localMask & __GL_TEXPARAM_BORDER_COLOR_BIT) {                   \
        if (ds_params->borderColor.r != cs_params->borderColor.r ||     \
            ds_params->borderColor.g != cs_params->borderColor.g ||     \
            ds_params->borderColor.b != cs_params->borderColor.b ||     \
            ds_params->borderColor.a != cs_params->borderColor.a)       \
        {                                                               \
            ds_params->borderColor = cs_params->borderColor;            \
        }                                                               \
        else {                                                          \
            localMask &= ~__GL_TEXPARAM_BORDER_COLOR_BIT;               \
        }                                                               \
    }


/* Set all attribute dirty bits.
 */
GLvoid __glSetAttributeStatesDirty(__GLcontext *gc)
{
    GLuint index,unit;
    GLuint planes;

    /* Initialize global dirty attribute bits */
    for (index = 0; index < __GL_DIRTY_ATTRS_END; index++) {
        gc->globalDirtyState[index] = (GLbitfield)(-1);
        gc->swpDirtyState[index] = (GLbitfield)(-1);
    }

    /* The upper 16 bits are for clipPlane enable/disable.
     * The lower 16 bits are for clipPlane parameters.
     */
    planes = (1 << gc->constants.numberOfClipPlanes) - 1;
    gc->globalDirtyState[__GL_CLIP_ATTRS] =
    gc->swpDirtyState[__GL_CLIP_ATTRS] = planes | (planes << 16);

    gc->texUnitAttrDirtyMask = gc->swpTexUnitAttrDirtyMask =
        (__GL_ONE_64 << __GL_MAX_TEXTURE_UNITS) - 1;

    for (unit = 0; unit < __GL_MAX_TEXTURE_UNITS; unit++) {
        gc->texUnitAttrState[unit] =
        gc->swpTexUnitAttrState[unit] = (GLuint64)(-1);
    }

    gc->globalDirtyState[__GL_LIGHT_SRC_ATTRS] = 0;
    gc->swpDirtyState[__GL_LIGHT_SRC_ATTRS] = 0;
    for (index = 0; index < gc->constants.numberOfLights; index++) {
        gc->lightAttrState[index] =
        gc->swpLightAttrState[index] = (GLbitfield)(-1);
        gc->globalDirtyState[__GL_LIGHT_SRC_ATTRS] |= (1 << index);
        gc->swpDirtyState[__GL_LIGHT_SRC_ATTRS] |= (1 << index);
    }
}

/* Clear all attribute dirty bits for DP or SWP.
 */
__GL_INLINE GLvoid __glClearAttributeStates(__GLcontext *gc)
{
    gc->texUnitAttrDirtyMask = 0;
    __GL_MEMZERO(gc->texUnitAttrState, __GL_MAX_TEXTURE_UNITS*sizeof(GLuint64));
    __GL_MEMZERO(gc->lightAttrState, __GL_MAX_LIGHT_NUMBER*sizeof(GLbitfield));
    __GL_MEMZERO(gc->globalDirtyState, __GL_DIRTY_ATTRS_END*sizeof(GLbitfield));

    gc->shaderProgram.samplerDirtyState = 0;
}

#define PRINT_ATTRIB  0

#if PRINT_ATTRIB
GLvoid __glPrintOutAttributeChanges(__GLcontext *gc)
{
    __GLattribute *cs = &gc->state;
    GLbitfield bitMask;
    GLuint i;

    /* Check the dirty bits in globalDirtyState[__GL_DIRTY_ATTRS_1]
     */
    if (gc->globalDirtyState[__GL_DIRTY_ATTRS_1]) {
        GLbitfield localMask = gc->globalDirtyState[__GL_DIRTY_ATTRS_1];

        if (localMask & __GL_VIEWPORT_BIT)
            printf("    __GL_VIEWPORT_BIT\n");

        if (localMask & __GL_DEPTHRANGE_BIT)
            printf("    __GL_DEPTHRANGE_BIT\n");

        if (localMask & __GL_ALPHAFUNC_BIT)
            printf("    __GL_ALPHAFUNC_BIT\n");

        if (localMask & __GL_ALPHATEST_ENDISABLE_BIT)
            printf("    __GL_ALPHATEST_ENDISABLE_BIT\n");

        if (localMask & __GL_BLENDCOLOR_BIT)
            printf("    __GL_BLENDCOLOR_BIT\n");

        if (localMask & __GL_BLENDFUNC_BIT)
            printf("    __GL_BLENDFUNC_BIT\n");

        if (localMask & __GL_BLENDEQUATION_BIT)
            printf("    __GL_BLENDEQUATION_BIT\n");

        if (localMask & __GL_BLEND_ENDISABLE_BIT)
            printf("    __GL_BLEND_ENDISABLE_BIT\n");

        if (localMask & __GL_LOGICOP_BIT)
            printf("    __GL_LOGICOP_BIT\n");

        if (localMask & __GL_LOGICOP_ENDISABLE_BIT)
            printf("    __GL_LOGICOP_ENDISABLE_BIT\n");

        if (localMask & __GL_CLEARCOLOR_BIT)
            printf("    __GL_CLEARCOLOR_BIT\n");

        if (localMask & __GL_COLORMASK_BIT)
            printf("    __GL_COLORMASK_BIT\n");

        if (localMask & __GL_DITHER_ENDISABLE_BIT)
            printf("    __GL_DITHER_ENDISABLE_BIT\n");

        if (localMask & __GL_DEPTHFUNC_BIT)
            printf("    __GL_DEPTHFUNC_BIT\n");

        if (localMask & __GL_DEPTHTEST_ENDISABLE_BIT)
            printf("    __GL_DEPTHTEST_ENDISABLE_BIT\n");

        if (localMask & __GL_CLEARDEPTH_BIT)
            printf("    __GL_CLEARDEPTH_BIT\n");

        if (localMask & __GL_DEPTHMASK_BIT)
            printf("    __GL_DEPTHMASK_BIT\n");

        if (localMask & __GL_DEPTHBOUNDTEST_BIT)
            printf("    __GL_DEPTHBOUNDTEST_BIT\n");

        if (localMask & __GL_DEPTHBOUNDTESTENABLE_BIT)
            printf("    __GL_DEPTHBOUNDTESTENABLE_BIT\n");

        if (localMask & __GL_STENCILFUNC_FRONT_BIT)
            printf("    __GL_STENCILFUNC_FRONT_BIT\n");

        if (localMask & __GL_STENCILOP_FRONT_BIT)
            printf("    __GL_STENCILOP_FRONT_BIT\n");

        if (localMask & __GL_STENCILFUNC_BACK_BIT)
            printf("    __GL_STENCILFUNC_BACK_BIT\n");

        if (localMask & __GL_STENCILOP_BACK_BIT)
            printf("    __GL_STENCILOP_BACK_BIT\n");

        if (localMask & __GL_STENCILMASK_FRONT_BIT)
            printf("    __GL_STENCILMASK_FRONT_BIT\n");

        if (localMask & __GL_STENCILMASK_BACK_BIT)
            printf("    __GL_STENCILMASK_BACK_BIT\n");

        if (localMask & __GL_STENCILTEST_ENDISABLE_BIT)
            printf("    __GL_STENCILTEST_ENDISABLE_BIT\n");

        if (localMask & __GL_CLEARSTENCIL_BIT)
            printf("    __GL_CLEARSTENCIL_BIT\n");

        if (localMask & __GL_SCISSOR_BIT)
            printf("    __GL_SCISSOR_BIT\n");

        if (localMask & __GL_SCISSORTEST_ENDISABLE_BIT)
            printf("    __GL_SCISSORTEST_ENDISABLE_BIT\n");

        if (localMask & __GL_CLEARACCUM_BIT)
            printf("    __GL_CLEARACCUM_BIT\n");
    }

    /* Check the dirty bits in globalDirtyState[__GL_DIRTY_ATTRS_2]
     */
    if (gc->globalDirtyState[__GL_DIRTY_ATTRS_2]) {
        GLbitfield localMask = gc->globalDirtyState[__GL_DIRTY_ATTRS_2];

        if (localMask & __GL_FOGCOLOR_BIT)
            printf("    __GL_FOGCOLOR_BIT\n");

        if (localMask & __GL_FOGINDEX_BIT)
            printf("    __GL_FOGINDEX_BIT\n");

        if (localMask & __GL_FOGDENSITY_BIT)
            printf("    __GL_FOGDENSITY_BIT\n");

        if (localMask & __GL_FOGSTART_BIT)
            printf("    __GL_FOGSTART_BIT\n");

        if (localMask & __GL_FOGEND_BIT)
            printf("    __GL_FOGEND_BIT\n");

        if (localMask & __GL_FOGMODE_BIT)
            printf("    __GL_FOGMODE_BIT\n");

        if (localMask & __GL_FOG_ENDISABLE_BIT)
            printf("    __GL_FOG_ENDISABLE_BIT\n");

        if (localMask & __GL_FRONTFACE_BIT)
            printf("    __GL_FRONTFACE_BIT\n");

        if (localMask & __GL_CULLFACE_BIT)
            printf("    __GL_CULLFACE_BIT\n");

        if (localMask & __GL_CULLFACE_ENDISABLE_BIT)
            printf("    __GL_CULLFACE_ENDISABLE_BIT\n");

        if (localMask & __GL_POLYGONMODE_BIT)
            printf("    __GL_POLYGONMODE_BIT\n");

        if (localMask & __GL_POLYGONOFFSET_BIT)
            printf("    __GL_POLYGONOFFSET_BIT\n");

        if (localMask & __GL_POLYGONOFFSET_POINT_ENDISABLE_BIT)
            printf("    __GL_POLYGONOFFSET_POINT_ENDISABLE_BIT\n");

        if (localMask & __GL_POLYGONOFFSET_LINE_ENDISABLE_BIT)
            printf("    __GL_POLYGONOFFSET_LINE_ENDISABLE_BIT\n");

        if (localMask & __GL_POLYGONOFFSET_FILL_ENDISABLE_BIT)
            printf("    __GL_POLYGONOFFSET_FILL_ENDISABLE_BIT\n");

        if (localMask & __GL_POLYGONSMOOTH_ENDISABLE_BIT)
            printf("    __GL_POLYGONSMOOTH_ENDISABLE_BIT\n");

        if (localMask & __GL_POLYGONSTIPPLE_ENDISABLE_BIT)
            printf("    __GL_POLYGONSTIPPLE_ENDISABLE_BIT\n");

        if (localMask & __GL_LINEWIDTH_BIT)
            printf("    __GL_LINEWIDTH_BIT\n");

        if (localMask & __GL_LINESMOOTH_ENDISABLE_BIT)
            printf("    __GL_LINESMOOTH_ENDISABLE_BIT\n");

        if (localMask & __GL_LINESTIPPLE_BIT)
            printf("    __GL_LINESTIPPLE_BIT\n");

        if (localMask & __GL_LINESTIPPLE_ENDISABLE_BIT)
            printf("    __GL_LINESTIPPLE_ENDISABLE_BIT\n");

        if (localMask & __GL_POINTSIZE_BIT)
            printf("    __GL_POINTSIZE_BIT\n");

        if (localMask & __GL_POINTSMOOTH_ENDISABLE_BIT)
            printf("    __GL_POINTSMOOTH_ENDISABLE_BIT\n");

        if (localMask & __GL_POINT_SIZE_MIN_BIT)
            printf("    __GL_POINT_SIZE_MIN_BIT\n");

        if (localMask & __GL_POINT_SIZE_MAX_BIT)
            printf("    __GL_POINT_SIZE_MAX_BIT\n");

        if (localMask & __GL_POINT_FADE_THRESHOLD_SIZE_BIT)
            printf("    __GL_POINT_FADE_THRESHOLD_SIZE_BIT\n");

        if (localMask & __GL_POINT_DISTANCE_ATTENUATION_BIT)
            printf("    __GL_POINT_DISTANCE_ATTENUATION_BIT\n");
    }

    /* Check the dirty bits in globalDirtyState[__GL_DIRTY_ATTRS_3]
     */
    if (gc->globalDirtyState[__GL_DIRTY_ATTRS_3]) {
        GLbitfield localMask = gc->globalDirtyState[__GL_DIRTY_ATTRS_3];

        if (localMask & __GL_NORMALIZE_ENDISABLE_BIT)
            printf("    __GL_NORMALIZE_ENDISABLE_BIT\n");

        if (localMask & __GL_RESCALENORMAL_ENDISABLE_BIT)
            printf("    __GL_RESCALENORMAL_ENDISABLE_BIT\n");

        if (localMask & __GL_SAMPLECOVERAGE_BIT)
            printf("    __GL_SAMPLECOVERAGE_BIT\n");

        if (localMask & __GL_MULTISAMPLE_ENDISABLE_BIT)
            printf("    __GL_MULTISAMPLE_ENDISABLE_BIT\n");

        if (localMask & __GL_SAMPLE_ALPHA_TO_COVERAGE_ENDISABLE_BIT)
            printf("    __GL_SAMPLE_ALPHA_TO_COVERAGE_ENDISABLE_BIT\n");

        if (localMask & __GL_SAMPLE_ALPHA_TO_ONE_ENDISABLE_BIT)
            printf("    __GL_SAMPLE_ALPHA_TO_ONE_ENDISABLE_BIT\n");

        if (localMask & __GL_SAMPLE_COVERAGE_ENDISABLE_BIT)
            printf("    __GL_SAMPLE_COVERAGE_ENDISABLE_BIT\n");

        if (localMask & __GL_COLORSUM_ENDISABLE_BIT)
            printf("    __GL_COLORSUM_ENDISABLE_BIT\n");
    }

    /* Check the dirty bits in globalDirtyState[__GL_LIGHTING_ATTRS]
     */
    if (gc->globalDirtyState[__GL_LIGHTING_ATTRS]) {
        GLbitfield localMask = gc->globalDirtyState[__GL_LIGHTING_ATTRS];

        if (localMask & __GL_SHADEMODEL_BIT)
            printf("    __GL_SHADEMODEL_BIT\n");

        if (localMask & __GL_LIGHTING_ENDISABLE_BIT)
            printf("    __GL_LIGHTING_ENDISABLE_BIT\n");

        if (localMask & __GL_LIGHTMODEL_AMBIENT_BIT)
            printf("    __GL_LIGHTMODEL_AMBIENT_BIT\n");

        if (localMask & __GL_LIGHTMODEL_LOCALVIEWER_BIT)
            printf("    __GL_LIGHTMODEL_LOCALVIEWER_BIT\n");

        if (localMask & __GL_LIGHTMODEL_TWOSIDE_BIT)
            printf("    __GL_LIGHTMODEL_TWOSIDE_BIT\n");

        if (localMask & __GL_LIGHTMODEL_COLORCONTROL_BIT)
            printf("    __GL_LIGHTMODEL_COLORCONTROL_BIT\n");

        if (localMask & __GL_MATERIAL_COLORINDEX_FRONT_BIT)
            printf("    __GL_MATERIAL_COLORINDEX_FRONT_BIT\n");

        if (localMask & __GL_MATERIAL_EMISSION_FRONT_BIT)
            printf("    __GL_MATERIAL_EMISSION_FRONT_BIT\n");

        if (localMask & __GL_MATERIAL_SPECULAR_FRONT_BIT)
            printf("    __GL_MATERIAL_SPECULAR_FRONT_BIT\n");

        if (localMask & __GL_MATERIAL_SHININESS_FRONT_BIT)
            printf("    __GL_MATERIAL_SHININESS_FRONT_BIT\n");

        if (localMask & __GL_MATERIAL_AMBIENT_FRONT_BIT)
            printf("    __GL_MATERIAL_AMBIENT_FRONT_BIT\n");

        if (localMask & __GL_MATERIAL_DIFFUSE_FRONT_BIT)
            printf("    __GL_MATERIAL_DIFFUSE_FRONT_BIT\n");

        if (localMask & __GL_MATERIAL_COLORINDEX_BACK_BIT)
            printf("    __GL_MATERIAL_COLORINDEX_BACK_BIT\n");

        if (localMask & __GL_MATERIAL_EMISSION_BACK_BIT)
            printf("    __GL_MATERIAL_EMISSION_BACK_BIT\n");

        if (localMask & __GL_MATERIAL_SPECULAR_BACK_BIT)
            printf("    __GL_MATERIAL_SPECULAR_BACK_BIT\n");

        if (localMask & __GL_MATERIAL_SHININESS_BACK_BIT)
            printf("    __GL_MATERIAL_SHININESS_BACK_BIT\n");

        if (localMask & __GL_MATERIAL_AMBIENT_BACK_BIT)
            printf("    __GL_MATERIAL_AMBIENT_BACK_BIT\n");

        if (localMask & __GL_MATERIAL_DIFFUSE_BACK_BIT)
            printf("    __GL_MATERIAL_DIFFUSE_BACK_BIT\n");

        if (localMask & __GL_COLORMATERIAL_BIT)
            printf("    __GL_COLORMATERIAL_BIT\n");

        if (localMask & __GL_COLORMATERIAL_ENDISABLE_BIT)
            printf("    __GL_COLORMATERIAL_ENDISABLE_BIT\n");
    }

    /* Check the dirty bits in lightAttrState[0 .. __GL_MAX_LIGHT_NUM]
     */
    if (gc->globalDirtyState[__GL_LIGHT_SRC_ATTRS]) {
        i = 0;
        bitMask = gc->globalDirtyState[__GL_LIGHT_SRC_ATTRS];

        while (bitMask) {
            if (bitMask & 0x1) {
                GLbitfield localMask = gc->lightAttrState[i];

                if (localMask & __GL_LIGHT_ENDISABLE_BIT)
                    printf("    __GL_LIGHT_ENDISABLE_BIT, light %d\n", i);

                if (localMask & __GL_LIGHT_AMBIENT_BIT)
                    printf("    __GL_LIGHT_AMBIENT_BIT, light %d\n", i);

                if (localMask & __GL_LIGHT_DIFFUSE_BIT)
                    printf("    __GL_LIGHT_DIFFUSE_BIT, light %d\n", i);

                if (localMask & __GL_LIGHT_SPECULAR_BIT)
                    printf("    __GL_LIGHT_SPECULAR_BIT, light %d\n", i);

                if (localMask & __GL_LIGHT_POSITION_BIT)
                    printf("    __GL_LIGHT_POSITION_BIT, light %d\n", i);

                if (localMask & __GL_LIGHT_CONSTANTATT_BIT)
                    printf("    __GL_LIGHT_CONSTANTATT_BIT, light %d\n", i);

                if (localMask & __GL_LIGHT_LINEARATT_BIT)
                    printf("    __GL_LIGHT_LINEARATT_BIT, light %d\n", i);

                if (localMask & __GL_LIGHT_QUADRATICATT_BIT)
                    printf("    __GL_LIGHT_QUADRATICATT_BIT, light %d\n", i);

                if (localMask & __GL_LIGHT_SPOTDIRECTION_BIT)
                    printf("    __GL_LIGHT_SPOTDIRECTION_BIT, light %d\n", i);

                if (localMask & __GL_LIGHT_SPOTEXPONENT_BIT)
                    printf("    __GL_LIGHT_SPOTEXPONENT_BIT, light %d\n", i);

                if (localMask & __GL_LIGHT_SPOTCUTOFF_BIT)
                    printf("    __GL_LIGHT_SPOTCUTOFF_BIT, light %d\n", i);
            }

            bitMask = (bitMask >> 1);
            i++;
        }
    }

    /* Check the dirty bits in globalDirtyState[__GL_CLIP_ATTRS]
     */
    if (gc->globalDirtyState[__GL_CLIP_ATTRS]) {
        GLbitfield localMask = gc->globalDirtyState[__GL_CLIP_ATTRS];

        for (i = 0; i < gc->constants.numberOfClipPlanes; i++) {
            GLuint planeBit = (1 << i);
            GLuint enableBit = (1 << (i + 16));

            if (localMask & planeBit)
                printf("    __GL_CLIPPLANE, clipplane %d\n", i);

            if (localMask & enableBit)
                printf("    __GL_CLIPPLANE_ENDISABLE_BIT, clipplane %d\n", i);
        }
    }

    /* Check the dirty bits in texUnitAttrState[0 .. __GL_TEXTURE_MAX_UNITS]
     */
    if (gc->texUnitAttrDirtyMask) {
        GLuint64 localMask, unitMask;
        i = 0;
        unitMask = gc->texUnitAttrDirtyMask;
        while (unitMask) {
            if (unitMask & 0x1) {
                localMask = gc->texUnitAttrState[i];

                if (localMask & __GL_TEX_ENABLE_DIM_CHANGED_BIT)
                    printf("    __GL_TEX_ENABLE_DIM_CHANGED_BIT, unit %d\n", i);

                if (localMask & __GL_TEXGEN_S_ENDISABLE_BIT)
                    printf("    __GL_TEXGEN_S_ENDISABLE_BIT, unit %d\n", i);

                if (localMask & __GL_TEXGEN_T_ENDISABLE_BIT)
                    printf("    __GL_TEXGEN_T_ENDISABLE_BIT, unit %d\n", i);

                if (localMask & __GL_TEXGEN_R_ENDISABLE_BIT)
                    printf("    __GL_TEXGEN_R_ENDISABLE_BIT, unit %d\n", i);

                if (localMask & __GL_TEXGEN_Q_ENDISABLE_BIT)
                    printf("    __GL_TEXGEN_Q_ENDISABLE_BIT, unit %d\n", i);

                if (localMask & __GL_TEXGEN_S_BIT)
                    printf("    __GL_TEXGEN_S_BIT, unit %d\n", i);

                if (localMask & __GL_TEXGEN_T_BIT)
                    printf("    __GL_TEXGEN_T_BIT, unit %d\n", i);

                if (localMask & __GL_TEXGEN_R_BIT)
                    printf("    __GL_TEXGEN_R_BIT, unit %d\n", i);

                if (localMask & __GL_TEXGEN_Q_BIT)
                    printf("    __GL_TEXGEN_Q_BIT, unit %d\n", i);

                if (localMask & __GL_TEXENV_MODE_BIT)
                    printf("    __GL_TEXENV_MODE_BIT, unit %d\n", i);

                if (localMask & __GL_TEXENV_COLOR_BIT)
                    printf("    __GL_TEXENV_COLOR_BIT, unit %d\n", i);

                if (localMask & __GL_TEXENV_COMBINE_ALPHA_BIT)
                    printf("    __GL_TEXENV_COMBINE_ALPHA_BIT, unit %d\n", i);

                if (localMask & __GL_TEXENV_COMBINE_RGB_BIT)
                    printf("    __GL_TEXENV_COMBINE_RGB_BIT, unit %d\n", i);

                if (localMask & __GL_TEXENV_SOURCE0_RGB_BIT)
                    printf("    __GL_TEXENV_SOURCE0_RGB_BIT, unit %d\n", i);

                if (localMask & __GL_TEXENV_SOURCE1_RGB_BIT)
                    printf("    __GL_TEXENV_SOURCE1_RGB_BIT, unit %d\n", i);

                if (localMask & __GL_TEXENV_SOURCE2_RGB_BIT)
                    printf("    __GL_TEXENV_SOURCE2_RGB_BIT, unit %d\n", i);

                if (localMask & __GL_TEXENV_SOURCE0_ALPHA_BIT)
                    printf("    __GL_TEXENV_SOURCE0_ALPHA_BIT, unit %d\n", i);

                if (localMask & __GL_TEXENV_SOURCE1_ALPHA_BIT)
                    printf("    __GL_TEXENV_SOURCE1_ALPHA_BIT, unit %d\n", i);

                if (localMask & __GL_TEXENV_SOURCE2_ALPHA_BIT)
                    printf("    __GL_TEXENV_SOURCE2_ALPHA_BIT, unit %d\n", i);

                if (localMask & __GL_TEXENV_OPERAND0_RGB_BIT)
                    printf("    __GL_TEXENV_OPERAND0_RGB_BIT, unit %d\n", i);

                if (localMask & __GL_TEXENV_OPERAND1_RGB_BIT)
                    printf("    __GL_TEXENV_OPERAND1_RGB_BIT, unit %d\n", i);

                if (localMask & __GL_TEXENV_OPERAND2_RGB_BIT)
                    printf("    __GL_TEXENV_OPERAND2_RGB_BIT, unit %d\n", i);

                if (localMask & __GL_TEXENV_OPERAND0_ALPHA_BIT)
                    printf("    __GL_TEXENV_OPERAND0_ALPHA_BIT, unit %d\n", i);

                if (localMask & __GL_TEXENV_OPERAND1_ALPHA_BIT)
                    printf("    __GL_TEXENV_OPERAND1_ALPHA_BIT, unit %d\n", i);

                if (localMask & __GL_TEXENV_OPERAND2_ALPHA_BIT)
                    printf("    __GL_TEXENV_OPERAND2_ALPHA_BIT, unit %d\n", i);

                if (localMask & __GL_TEXENV_RGB_SCALE_BIT)
                    printf("    __GL_TEXENV_RGB_SCALE_BIT, unit %d\n", i);

                if (localMask & __GL_TEXENV_ALPHA_SCALE_BIT)
                    printf("    __GL_TEXENV_ALPHA_SCALE_BIT, unit %d\n", i);

                if (localMask & __GL_TEXPARAM_WRAP_S_BIT)
                    printf("    __GL_TEXPARAM_WRAP_S_BIT, unit %d\n", i);

                if (localMask & __GL_TEXPARAM_WRAP_T_BIT)
                    printf("    __GL_TEXPARAM_WRAP_T_BIT, unit %d\n", i);

                if (localMask & __GL_TEXPARAM_WRAP_R_BIT)
                    printf("    __GL_TEXPARAM_WRAP_R_BIT, unit %d\n", i);

                if (localMask & __GL_TEXPARAM_MIN_FILTER_BIT)
                    printf("    __GL_TEXPARAM_MIN_FILTER_BIT, unit %d\n", i);

                if (localMask & __GL_TEXPARAM_MAG_FILTER_BIT)
                    printf("    __GL_TEXPARAM_MAG_FILTER_BIT, unit %d\n", i);

                if (localMask & __GL_TEXPARAM_PRIORITY_BIT)
                    printf("    __GL_TEXPARAM_PRIORITY_BIT, unit %d\n", i);

                if (localMask & __GL_TEXPARAM_MIN_LOD_BIT)
                    printf("    __GL_TEXPARAM_MIN_LOD_BIT, unit %d\n", i);

                if (localMask & __GL_TEXPARAM_MAX_LOD_BIT)
                    printf("    __GL_TEXPARAM_MAX_LOD_BIT, unit %d\n", i);

                if (localMask & __GL_TEXPARAM_BASE_LEVEL_BIT)
                    printf("    __GL_TEXPARAM_BASE_LEVEL_BIT, unit %d\n", i);

                if (localMask & __GL_TEXPARAM_MAX_LEVEL_BIT)
                    printf("    __GL_TEXPARAM_MAX_LEVEL_BIT, unit %d\n", i);

                if (localMask & __GL_TEXPARAM_LOD_BIAS_BIT)
                    printf("    __GL_TEXPARAM_LOD_BIAS_BIT, unit %d\n", i);

                if (localMask & __GL_TEXPARAM_DEPTH_TEX_MODE_BIT)
                    printf("    __GL_TEXPARAM_DEPTH_TEX_MODE_BIT, unit %d\n", i);

                if (localMask & __GL_TEXPARAM_COMPARE_MODE_BIT)
                    printf("    __GL_TEXPARAM_COMPARE_MODE_BIT, unit %d\n", i);

                if (localMask & __GL_TEXPARAM_COMPARE_FUNC_BIT)
                    printf("    __GL_TEXPARAM_COMPARE_FUNC_BIT, unit %d\n", i);

                if (localMask & __GL_TEXPARAM_GENERATE_MIPMAP_BIT)
                    printf("    __GL_TEXPARAM_GENERATE_MIPMAP_BIT, unit %d\n", i);
            }

            unitMask = (unitMask >> 1);
            i++;
        }
    }
}
#endif

GLvoid __glClearProgramVSEnabledDimension(__GLcontext * gc)
{
    GLuint unit;
    for (unit=0; unit<__GL_MAX_TEXTURE_UNITS; unit++)
    {
        if(gc->state.enables.texUnits[unit].programVSEnabledDimension != 0)
        {
            gc->state.enables.texUnits[unit].programVSEnabledDimension = 0;
            __GL_SET_TEX_UNIT_BIT(gc, unit, __GL_TEX_ENABLE_DIM_CHANGED_BIT);
        }
    }
}

GLvoid __glSetProgramVSEnabledDimension(__GLcontext * gc)
{
    GLuint unit;
    if (gc->shaderProgram.vertShaderEnable)
    {
        if(gc->shaderProgram.vertShaderRealEnable)
        {
            __GLshaderProgramObject *progGLSLObj = gc->shaderProgram.currentShaderProgram;
            GL_ASSERT(progGLSLObj);
            for(unit=0; unit<__GL_MAX_TEXTURE_UNITS; unit++)
            {
                if (gc->state.enables.texUnits[unit].programVSEnabledDimension !=
                        progGLSLObj->bindingInfo.vsTexEnableDim[unit])
                {
                    gc->state.enables.texUnits[unit].programVSEnabledDimension =
                        progGLSLObj->bindingInfo.vsTexEnableDim[unit];
                    __GL_SET_TEX_UNIT_BIT(gc, unit, __GL_TEX_ENABLE_DIM_CHANGED_BIT);
                    /*
                    ** If the texture enable dimension is really changed then
                    ** force set all dirty bits for texture parameters and texture image.
                    */
                    if (gc->state.enables.texUnits[unit].programVSEnabledDimension)
                    {
                        __GL_SET_TEX_UNIT_BIT(gc, unit, (__GL_TEXPARAMETER_BITS | __GL_TEXIMAGE_BITS));
                    }
                }
            }
        }
        else
        {
            __glClearProgramVSEnabledDimension(gc);
        }
    }
    else
    {
        __glClearProgramVSEnabledDimension(gc);
    }
}

GLvoid __glClearProgramGSEnabledDimension(__GLcontext * gc)
{
    GLuint unit;
    for (unit=0; unit<__GL_MAX_TEXTURE_UNITS; unit++)
    {
        if(gc->state.enables.texUnits[unit].programGSEnabledDimension != 0)
        {
            gc->state.enables.texUnits[unit].programGSEnabledDimension = 0;
            __GL_SET_TEX_UNIT_BIT(gc, unit, __GL_TEX_ENABLE_DIM_CHANGED_BIT);
        }
    }
}

GLvoid __glSetProgramGSEnabledDimension(__GLcontext * gc)
{
    GLuint unit;
    if (gc->shaderProgram.geomShaderEnable)
    {
        if(gc->shaderProgram.geomShaderRealEnable)
        {
            __GLshaderProgramObject *progGLSLObj = gc->shaderProgram.currentShaderProgram;
            GL_ASSERT(progGLSLObj);
            for(unit=0; unit<__GL_MAX_TEXTURE_UNITS; unit++)
            {
                if (gc->state.enables.texUnits[unit].programGSEnabledDimension !=
                        progGLSLObj->bindingInfo.gsTexEnableDim[unit])
                {
                    gc->state.enables.texUnits[unit].programGSEnabledDimension =
                        progGLSLObj->bindingInfo.gsTexEnableDim[unit];
                    __GL_SET_TEX_UNIT_BIT(gc, unit, __GL_TEX_ENABLE_DIM_CHANGED_BIT);
                    /*
                    ** If the texture enable dimension is really changed then
                    ** force set all dirty bits for texture parameters and texture image.
                    */
                    if (gc->state.enables.texUnits[unit].programGSEnabledDimension)
                    {
                        __GL_SET_TEX_UNIT_BIT(gc, unit, (__GL_TEXPARAMETER_BITS | __GL_TEXIMAGE_BITS));
                    }
                }
            }
        }
        else
        {
            __glClearProgramGSEnabledDimension(gc);
        }
    }
    else
    {
        __glClearProgramGSEnabledDimension(gc);
    }
}

GLvoid __glClearProgramPSEnabledDimension(__GLcontext * gc)
{
    GLuint unit;
    for (unit=0; unit<__GL_MAX_TEXTURE_UNITS; unit++)
    {
        if(gc->state.enables.texUnits[unit].programPSEnabledDimension != 0)
        {
            gc->state.enables.texUnits[unit].programPSEnabledDimension = 0;
            __GL_SET_TEX_UNIT_BIT(gc, unit, __GL_TEX_ENABLE_DIM_CHANGED_BIT);
        }
    }
}

GLvoid __glSetProgramPSEnabledDimension(__GLcontext * gc)
{
    GLuint unit;
    __GLshaderProgramMachine *shaderProgramMachine = &gc->shaderProgram;
    if(shaderProgramMachine->fragShaderEnable)
    {
        if(shaderProgramMachine->fragShaderRealEnable)
        {
            __GLshaderProgramObject *progGLSLObj = shaderProgramMachine->currentShaderProgram;
            GL_ASSERT(progGLSLObj);

            for(unit=0; unit<__GL_MAX_TEXTURE_UNITS; unit++)
            {
                if (gc->state.enables.texUnits[unit].programPSEnabledDimension !=
                        progGLSLObj->bindingInfo.psTexEnableDim[unit])
                {
                    gc->state.enables.texUnits[unit].programPSEnabledDimension =
                        progGLSLObj->bindingInfo.psTexEnableDim[unit];
                    __GL_SET_TEX_UNIT_BIT(gc, unit, __GL_TEX_ENABLE_DIM_CHANGED_BIT);
                    /*
                    ** If the texture enable dimension is really changed then
                    ** force set all dirty bits for texture parameters and texture image.
                    */
                    if (gc->state.enables.texUnits[unit].programPSEnabledDimension)
                    {
                        __GL_SET_TEX_UNIT_BIT(gc, unit, (__GL_TEXPARAMETER_BITS | __GL_TEXIMAGE_BITS));
                    }
                }
            }
        }
        else
        {
            __glClearProgramPSEnabledDimension(gc);
        }
    }
    else if(gc->state.enables.program.fragmentProgram)
    {
        if(gc->program.realEnabled[__GL_FRAGMENT_PROGRAM_INDEX])
        {
            __GLProgramObject *progObj = gc->program.currentProgram[__GL_FRAGMENT_PROGRAM_INDEX];

            GL_ASSERT(progObj);
            for (unit=0; unit<__GL_MAX_TEXTURE_UNITS; unit++)
            {
                if(gc->state.enables.texUnits[unit].programPSEnabledDimension !=
                    progObj->compiledResult.enabledDimension[unit])
                {
                    gc->state.enables.texUnits[unit].programPSEnabledDimension =
                        progObj->compiledResult.enabledDimension[unit];
                    __GL_SET_TEX_UNIT_BIT(gc, unit, __GL_TEX_ENABLE_DIM_CHANGED_BIT);

                    /*
                    ** If the texture enable dimension is really changed then
                    ** force set all dirty bits for texture parameters and texture image.
                    */
                    if (gc->state.enables.texUnits[unit].programPSEnabledDimension)
                    {
                        __GL_SET_TEX_UNIT_BIT(gc, unit, (__GL_TEXPARAMETER_BITS | __GL_TEXIMAGE_BITS));
                    }
                }
            }
        }
        else
        {
            __glClearProgramPSEnabledDimension(gc);
        }
    }
    else
    {
        __glClearProgramPSEnabledDimension(gc);
    }
}


GLvoid __glUpdateProgramEnableDimension(__GLcontext * gc)
{
    __GLshaderProgramMachine *shaderProgramMachine = &gc->shaderProgram;

    /* VS */
    if(shaderProgramMachine->vertShaderEnable)
    {
        GL_ASSERT(shaderProgramMachine->currentShaderProgram);
        shaderProgramMachine->vertShaderRealEnable =
            gc->dp.validateShaderProgram(gc, shaderProgramMachine->currentShaderProgram);
    }
    __glSetProgramVSEnabledDimension(gc);

    /* GS */
    if(shaderProgramMachine->geomShaderEnable)
    {
        GL_ASSERT(shaderProgramMachine->currentShaderProgram);
        shaderProgramMachine->geomShaderRealEnable =
            gc->dp.validateShaderProgram(gc, shaderProgramMachine->currentShaderProgram);
    }
    __glSetProgramGSEnabledDimension(gc);

    /* PS */
   if(shaderProgramMachine->fragShaderEnable)
   {
        GL_ASSERT(shaderProgramMachine->currentShaderProgram);
        shaderProgramMachine->fragShaderRealEnable =
            gc->dp.validateShaderProgram(gc, shaderProgramMachine->currentShaderProgram);
    }

    if (gc->state.enables.program.fragmentProgram)
    {
        __glFragmentProgramRealEnabled(gc);
    }

    __glSetProgramPSEnabledDimension(gc);

}

__GL_INLINE GLvoid __glEvaluateAttribGroup1(__GLcontext* gc, __GLattribute* cs, __GLattribute* ds)
{
    GLbitfield localMask = gc->globalDirtyState[__GL_DIRTY_ATTRS_1];

    __GL_CHECK_ATTR4(__GL_VIEWPORT_BIT,
        viewport.x, viewport.y,
        viewport.width, viewport.height);

    __GL_CHECK_ATTR2(__GL_DEPTHRANGE_BIT,
        viewport.zNear, viewport.zFar);

    if (localMask & __GL_COLORBUF_ATTR_BITS) {

        __GL_CHECK_ATTR2(__GL_ALPHAFUNC_BIT,
            raster.alphaFunction, raster.alphaReference);

        __GL_CHECK_ATTR1(__GL_ALPHATEST_ENDISABLE_BIT,
            enables.colorBuffer.alphaTest);

        __GL_CHECK_ATTR4(__GL_BLENDCOLOR_BIT,
            raster.blendColor.r, raster.blendColor.g,
            raster.blendColor.b, raster.blendColor.a);

        __GL_CHECK_ATTR4(__GL_BLENDFUNC_BIT,
            raster.blendSrcRGB, raster.blendSrcAlpha,
            raster.blendDstRGB, raster.blendDstAlpha);

        __GL_CHECK_ATTR2(__GL_BLENDEQUATION_BIT,
            raster.blendEquationRGB, raster.blendEquationAlpha);

        if(localMask & __GL_BLEND_ENDISABLE_BIT)
        {
            if(__GL_MEMCMP(ds->enables.colorBuffer.blend, cs->enables.colorBuffer.blend, sizeof(GLboolean)*__GL_MAX_DRAW_BUFFERS))
            {
                __GL_MEMCOPY(ds->enables.colorBuffer.blend, cs->enables.colorBuffer.blend, sizeof(GLboolean)*__GL_MAX_DRAW_BUFFERS);
            }else
            {
                localMask &= ~(__GL_BLEND_ENDISABLE_BIT);
            }
        }

        __GL_CHECK_ATTR1(__GL_LOGICOP_BIT,
            raster.logicOp);

        __GL_CHECK_ATTR1(__GL_LOGICOP_ENDISABLE_BIT,
            enables.colorBuffer.colorLogicOp);

        __GL_CHECK_ATTR4(__GL_CLEARCOLOR_BIT,
            raster.clear.r, raster.clear.g, raster.clear.b, raster.clear.a);

        if(localMask & __GL_COLORMASK_BIT)
        {
            if(__GL_MEMCMP(ds->raster.colorMask, cs->raster.colorMask, sizeof(GLboolean)*4*__GL_MAX_DRAW_BUFFERS))
            {
                __GL_MEMCOPY(ds->raster.colorMask, cs->raster.colorMask, sizeof(GLboolean)*4*__GL_MAX_DRAW_BUFFERS);
            }else
            {
                localMask &= ~( __GL_COLORMASK_BIT);
            }
        }

        __GL_CHECK_ATTR1(__GL_DITHER_ENDISABLE_BIT,
            enables.colorBuffer.dither);
    }

    if (localMask & __GL_DEPTHBUF_ATTR_BITS) {

        __GL_CHECK_ATTR1(__GL_DEPTHFUNC_BIT,
            depth.testFunc);

        __GL_CHECK_ATTR1(__GL_DEPTHTEST_ENDISABLE_BIT,
            enables.depthBuffer.test);

        __GL_CHECK_ATTR1(__GL_CLEARDEPTH_BIT,
            depth.clear);

        __GL_CHECK_ATTR1(__GL_DEPTHMASK_BIT,
            depth.writeEnable);

        __GL_CHECK_ATTR2(__GL_DEPTHBOUNDTEST_BIT,
            depthBoundTest.zMin, depthBoundTest.zMax);

        __GL_CHECK_ATTR1(__GL_DEPTHBOUNDTESTENABLE_BIT,
            enables.depthBoundTest);
    }

    if (localMask & __GL_STENCIL_ATTR_BITS) {

        /* Copy one stencil state to current stencil state base on the current stencil enables.
        */
        if (cs->enables.stencilTest && cs->enables.stencilTestTwoSideExt) {
            cs->stencil.current = cs->stencil.stencilExt;
        } else {
            cs->stencil.current = cs->stencil.StencilArb;
        }

        __GL_CHECK_ATTR3(__GL_STENCILFUNC_FRONT_BIT,
            stencil.current.front.testFunc, stencil.current.front.reference, stencil.current.front.mask);

        __GL_CHECK_ATTR3(__GL_STENCILOP_FRONT_BIT,
            stencil.current.front.fail, stencil.current.front.depthFail, stencil.current.front.depthPass);

        __GL_CHECK_ATTR3(__GL_STENCILFUNC_BACK_BIT,
            stencil.current.back.testFunc, stencil.current.back.reference, stencil.current.back.mask);

        __GL_CHECK_ATTR3(__GL_STENCILOP_BACK_BIT,
            stencil.current.back.fail, stencil.current.back.depthFail, stencil.current.back.depthPass);

        __GL_CHECK_ATTR1(__GL_STENCILMASK_FRONT_BIT,
            stencil.current.front.writeMask);

        __GL_CHECK_ATTR1(__GL_STENCILMASK_BACK_BIT,
            stencil.current.back.writeMask);

        __GL_CHECK_ATTR1(__GL_STENCILTEST_ENDISABLE_BIT,
            enables.stencilTest);

        __GL_CHECK_ATTR1(__GL_CLEARSTENCIL_BIT,
            stencil.clear);
    }

    __GL_CHECK_ATTR4(__GL_SCISSOR_BIT,
        scissor.scissorX, scissor.scissorY, scissor.scissorWidth, scissor.scissorHeight);

    __GL_CHECK_ATTR1(__GL_SCISSORTEST_ENDISABLE_BIT,
        enables.scissor);

    __GL_CHECK_ATTR4(__GL_CLEARACCUM_BIT,
        accum.clear.r, accum.clear.g, accum.clear.b, accum.clear.a);

    __GL_CHECK_ATTR1(__GL_CLAMP_VERTEX_COLOR_BIT, light.clampVertexColor);

    __GL_CHECK_ATTR1(__GL_CLAMP_FRAG_COLOR_BIT, raster.clampFragColor);

    /* Reassign localMask back to globalDirtyState[__GL_DIRTY_ATTRS_1]
    */
    gc->globalDirtyState[__GL_DIRTY_ATTRS_1] = localMask;
    if (localMask == 0) {
        gc->globalDirtyState[__GL_ALL_ATTRS] &= ~(1 << __GL_DIRTY_ATTRS_1);
    }
    else {
        /* Copy the dirty bits to swpDirtyState[__GL_DIRTY_ATTRS_1]
        */
        gc->swpDirtyState[__GL_DIRTY_ATTRS_1] |= localMask;
    }
}

__GL_INLINE GLvoid __glEvaluateAttribGroup2(__GLcontext* gc, __GLattribute* cs, __GLattribute* ds)
{
    GLbitfield localMask = gc->globalDirtyState[__GL_DIRTY_ATTRS_2];

    if (localMask & __GL_FOG_ATTR_BITS) {

        __GL_CHECK_ATTR4(__GL_FOGCOLOR_BIT,
            fog.color.r, fog.color.g, fog.color.b, fog.color.a);

        __GL_CHECK_ATTR1(__GL_FOGINDEX_BIT,
            fog.index);

        __GL_CHECK_ATTR1(__GL_FOGDENSITY_BIT,
            fog.density);

        __GL_CHECK_ATTR1(__GL_FOGSTART_BIT,
            fog.start);

        __GL_CHECK_ATTR1(__GL_FOGEND_BIT,
            fog.end);

        __GL_CHECK_ATTR1(__GL_FOGMODE_BIT,
            fog.mode);

        __GL_CHECK_ATTR1(__GL_FOG_ENDISABLE_BIT,
            enables.fog);
    }

    if (localMask & __GL_POLYGON_ATTR_BITS) {

        __GL_CHECK_ATTR1(__GL_FRONTFACE_BIT,
            polygon.frontFace);

        __GL_CHECK_ATTR1(__GL_CULLFACE_BIT,
            polygon.cullFace);

        __GL_CHECK_ATTR1(__GL_CULLFACE_ENDISABLE_BIT,
            enables.polygon.cullFace);

        __GL_CHECK_ATTR2(__GL_POLYGONMODE_BIT,
            polygon.frontMode, polygon.backMode);

        __GL_CHECK_ATTR2(__GL_POLYGONOFFSET_BIT,
            polygon.factor, polygon.units);

        __GL_CHECK_ATTR1(__GL_POLYGONOFFSET_POINT_ENDISABLE_BIT,
            enables.polygon.polygonOffsetPoint);

        __GL_CHECK_ATTR1(__GL_POLYGONOFFSET_LINE_ENDISABLE_BIT,
            enables.polygon.polygonOffsetLine);

        __GL_CHECK_ATTR1(__GL_POLYGONOFFSET_FILL_ENDISABLE_BIT,
            enables.polygon.polygonOffsetFill);

        __GL_CHECK_ATTR1(__GL_POLYGONSMOOTH_ENDISABLE_BIT,
            enables.polygon.smooth);

        __GL_CHECK_ATTR1(__GL_POLYGONSTIPPLE_ENDISABLE_BIT,
            enables.polygon.stipple);
    }

    if (localMask & __GL_LINE_ATTR_BITS) {

        __GL_CHECK_ATTR1(__GL_LINEWIDTH_BIT,
            line.requestedWidth);

        __GL_CHECK_ATTR1(__GL_LINESMOOTH_ENDISABLE_BIT,
            enables.line.smooth);

        __GL_CHECK_ATTR2(__GL_LINESTIPPLE_BIT,
            line.stippleRepeat, line.stipple);

        __GL_CHECK_ATTR1(__GL_LINESTIPPLE_ENDISABLE_BIT,
            enables.line.stipple);
    }

    if (localMask & __GL_POINT_ATTR_BITS) {

        __GL_CHECK_ATTR1(__GL_POINTSIZE_BIT,
            point.requestedSize);

        __GL_CHECK_ATTR1(__GL_POINTSMOOTH_ENDISABLE_BIT,
            enables.pointSmooth);

        __GL_CHECK_ATTR1(__GL_POINT_SIZE_MIN_BIT,
            point.sizeMin);

        __GL_CHECK_ATTR1(__GL_POINT_SIZE_MAX_BIT,
            point.sizeMax);

        __GL_CHECK_ATTR1(__GL_POINT_FADE_THRESHOLD_SIZE_BIT,
            point.fadeThresholdSize);

        __GL_CHECK_ATTR3(__GL_POINT_DISTANCE_ATTENUATION_BIT,
            point.distanceAttenuation[0],
            point.distanceAttenuation[1],
            point.distanceAttenuation[2]);

        __GL_CHECK_ATTR1(__GL_POINTSPRITE_ENDISABLE_BIT,
            enables.pointSprite);

        __GL_CHECK_ATTR1(__GL_POINTSPRITE_COORD_ORIGIN_BIT,
            point.coordOrigin);

    }

    /* Reassign localMask back to globalDirtyState[__GL_DIRTY_ATTRS_2]
    */
    gc->globalDirtyState[__GL_DIRTY_ATTRS_2] = localMask;
    if (localMask == 0) {
        gc->globalDirtyState[__GL_ALL_ATTRS] &= ~(1 << __GL_DIRTY_ATTRS_2);
    }
    else {
        /* Copy the dirty bits to swpDirtyState[__GL_DIRTY_ATTRS_2]
        */
        gc->swpDirtyState[__GL_DIRTY_ATTRS_2] |= localMask;
    }
}

__GL_INLINE GLvoid __glEvaluateAttribGroup3(__GLcontext* gc, __GLattribute* cs, __GLattribute* ds)
{
    GLbitfield localMask = gc->globalDirtyState[__GL_DIRTY_ATTRS_3];

    __GL_CHECK_ATTR1(__GL_NORMALIZE_ENDISABLE_BIT,
        enables.transform.normalize);

    __GL_CHECK_ATTR1(__GL_RESCALENORMAL_ENDISABLE_BIT,
        enables.transform.rescaleNormal);

    __GL_CHECK_ATTR2(__GL_SAMPLECOVERAGE_BIT,
        multisample.coverageValue, multisample.coverageInvert);

    __GL_CHECK_ATTR1(__GL_MULTISAMPLE_ENDISABLE_BIT,
        enables.multisample.multisampleOn);

    __GL_CHECK_ATTR1(__GL_SAMPLE_ALPHA_TO_COVERAGE_ENDISABLE_BIT,
        enables.multisample.alphaToCoverage);

    __GL_CHECK_ATTR1(__GL_SAMPLE_ALPHA_TO_ONE_ENDISABLE_BIT,
        enables.multisample.alphaToOne);

    __GL_CHECK_ATTR1(__GL_SAMPLE_COVERAGE_ENDISABLE_BIT,
        enables.multisample.coverage);

    __GL_CHECK_ATTR1(__GL_COLORSUM_ENDISABLE_BIT,
        enables.colorSum);

    /* Reassign localMask back to globalDirtyState[__GL_DIRTY_ATTRS_3]
    */
    gc->globalDirtyState[__GL_DIRTY_ATTRS_3] = localMask;
    if (localMask == 0) {
        gc->globalDirtyState[__GL_ALL_ATTRS] &= ~(1 << __GL_DIRTY_ATTRS_3);
    }
    else {
        /* Copy the dirty bits to swpDirtyState[__GL_DIRTY_ATTRS_3]
        */
        gc->swpDirtyState[__GL_DIRTY_ATTRS_3] |= localMask;
    }
}

__GL_INLINE GLvoid __glEvaluateLightingAttrib(__GLcontext* gc, __GLattribute* cs, __GLattribute* ds)
{
    GLbitfield localMask = gc->globalDirtyState[__GL_LIGHTING_ATTRS];

    __GL_CHECK_ATTR1(__GL_SHADEMODEL_BIT,
        light.shadingModel);

    __GL_CHECK_ATTR1(__GL_LIGHTING_ENDISABLE_BIT,
        enables.lighting.lighting);

    if (localMask & __GL_LIGHTMODEL_ATTR_BITS) {

        __GL_CHECK_ATTR4(__GL_LIGHTMODEL_AMBIENT_BIT,
            light.model.ambient.r, light.model.ambient.g,
            light.model.ambient.b, light.model.ambient.a);

        __GL_CHECK_ATTR1(__GL_LIGHTMODEL_LOCALVIEWER_BIT,
            light.model.localViewer);

        __GL_CHECK_ATTR1(__GL_LIGHTMODEL_TWOSIDE_BIT,
            light.model.twoSided);

        __GL_CHECK_ATTR1(__GL_LIGHTMODEL_COLORCONTROL_BIT,
            light.model.colorControl);
    }

    if (localMask & __GL_FRONT_MATERIAL_BITS) {
        __GL_CHECK_ATTR3(__GL_MATERIAL_COLORINDEX_FRONT_BIT,
            light.front.cmapa, light.front.cmapd, light.front.cmaps);

        __GL_CHECK_ATTR4(__GL_MATERIAL_EMISSION_FRONT_BIT,
            light.front.emissive.r, light.front.emissive.g,
            light.front.emissive.b, light.front.emissive.a);

        __GL_CHECK_ATTR4(__GL_MATERIAL_SPECULAR_FRONT_BIT,
            light.front.specular.r, light.front.specular.g,
            light.front.specular.b, light.front.specular.a);

        __GL_CHECK_ATTR1(__GL_MATERIAL_SHININESS_FRONT_BIT,
            light.front.specularExponent);

        __GL_CHECK_ATTR4(__GL_MATERIAL_AMBIENT_FRONT_BIT,
            light.front.ambient.r, light.front.ambient.g,
            light.front.ambient.b, light.front.ambient.a);

        __GL_CHECK_ATTR4(__GL_MATERIAL_DIFFUSE_FRONT_BIT,
            light.front.diffuse.r, light.front.diffuse.g,
            light.front.diffuse.b, light.front.diffuse.a);
    }

    if (localMask & __GL_BACK_MATERIAL_BITS) {
        __GL_CHECK_ATTR3(__GL_MATERIAL_COLORINDEX_BACK_BIT,
            light.back.cmapa, light.back.cmapd, light.back.cmaps);

        __GL_CHECK_ATTR4(__GL_MATERIAL_EMISSION_BACK_BIT,
            light.back.emissive.r, light.back.emissive.g,
            light.back.emissive.b, light.back.emissive.a);

        __GL_CHECK_ATTR4(__GL_MATERIAL_SPECULAR_BACK_BIT,
            light.back.specular.r, light.back.specular.g,
            light.back.specular.b, light.back.specular.a);

        __GL_CHECK_ATTR1(__GL_MATERIAL_SHININESS_BACK_BIT,
            light.back.specularExponent);

        __GL_CHECK_ATTR4(__GL_MATERIAL_AMBIENT_BACK_BIT,
            light.back.ambient.r, light.back.ambient.g,
            light.back.ambient.b, light.back.ambient.a);

        __GL_CHECK_ATTR4(__GL_MATERIAL_DIFFUSE_BACK_BIT,
            light.back.diffuse.r, light.back.diffuse.g,
            light.back.diffuse.b, light.back.diffuse.a);
    }

    __GL_CHECK_ATTR2(__GL_COLORMATERIAL_BIT,
        light.colorMaterialFace, light.colorMaterialParam);

    __GL_CHECK_ATTR1(__GL_COLORMATERIAL_ENDISABLE_BIT,
        enables.lighting.colorMaterial);

    /* Reassign localMask back to globalDirtyState[__GL_LIGHTING_ATTRS]
    */
    gc->globalDirtyState[__GL_LIGHTING_ATTRS] = localMask;
    if (localMask == 0) {
        gc->globalDirtyState[__GL_ALL_ATTRS] &= ~(1 << __GL_LIGHTING_ATTRS);
    }
    else {
        /* Copy the dirty bits to swpDirtyState[__GL_LIGHTING_ATTRS]
        */
        gc->swpDirtyState[__GL_LIGHTING_ATTRS] |= localMask;
    }
}

__GL_INLINE GLvoid __glEvaluateLightSrcAttrib(__GLcontext* gc, __GLattribute* cs, __GLattribute* ds)
{
    GLbitfield bitMask = gc->globalDirtyState[__GL_LIGHT_SRC_ATTRS];
    GLuint i = 0;

    while (bitMask) {
        if (bitMask & 0x1) {
            GLbitfield localMask = gc->lightAttrState[i];

            __GL_CHECK_ATTR1(__GL_LIGHT_ENDISABLE_BIT,
                enables.lighting.light[i]);

            __GL_CHECK_ATTR4(__GL_LIGHT_AMBIENT_BIT,
                light.source[i].ambient.r, light.source[i].ambient.g,
                light.source[i].ambient.b, light.source[i].ambient.a);

            __GL_CHECK_ATTR4(__GL_LIGHT_DIFFUSE_BIT,
                light.source[i].diffuse.r, light.source[i].diffuse.g,
                light.source[i].diffuse.b, light.source[i].diffuse.a);

            __GL_CHECK_ATTR4(__GL_LIGHT_SPECULAR_BIT,
                light.source[i].specular.r, light.source[i].specular.g,
                light.source[i].specular.b, light.source[i].specular.a);

            __GL_CHECK_ATTR4(__GL_LIGHT_POSITION_BIT,
                light.source[i].positionEye.x,
                light.source[i].positionEye.y,
                light.source[i].positionEye.z,
                light.source[i].positionEye.w);

            __GL_CHECK_ATTR1(__GL_LIGHT_CONSTANTATT_BIT,
                light.source[i].constantAttenuation);

            __GL_CHECK_ATTR1(__GL_LIGHT_LINEARATT_BIT,
                light.source[i].linearAttenuation);

            __GL_CHECK_ATTR1(__GL_LIGHT_QUADRATICATT_BIT,
                light.source[i].quadraticAttenuation);

            __GL_CHECK_ATTR4(__GL_LIGHT_SPOTDIRECTION_BIT,
                light.source[i].direction.x, light.source[i].direction.y,
                light.source[i].direction.z, light.source[i].direction.w);

            __GL_CHECK_ATTR1(__GL_LIGHT_SPOTEXPONENT_BIT,
                light.source[i].spotLightExponent);

            __GL_CHECK_ATTR1(__GL_LIGHT_SPOTCUTOFF_BIT,
                light.source[i].spotLightCutOffAngle);

            /* Reassign localMask back to gc->lightAttrState[i]
            */
            gc->lightAttrState[i] = localMask;
            if (localMask == 0) {
                gc->globalDirtyState[__GL_LIGHT_SRC_ATTRS] &= ~(1 << i);
            }
            else {
                /* Copy the dirty bits to swpLightAttrState[i].
                */
                gc->swpLightAttrState[i] |= localMask;
            }
        }

        bitMask = (bitMask >> 1);
        i++;
    }

    if (gc->globalDirtyState[__GL_LIGHT_SRC_ATTRS] == 0) {
        gc->globalDirtyState[__GL_ALL_ATTRS] &= ~(1 << __GL_LIGHT_SRC_ATTRS);
    }
    else {
        /* Copy the dirty bits to swpDirtyState[__GL_LIGHT_SRC_ATTRS]
        */
        gc->swpDirtyState[__GL_LIGHT_SRC_ATTRS] |=
            gc->globalDirtyState[__GL_LIGHT_SRC_ATTRS];
    }
}

__GL_INLINE GLvoid __glEvaluateClipAttrib(__GLcontext* gc, __GLattribute* cs, __GLattribute* ds)
{
    GLuint i = 0 ;
    GLbitfield localMask = gc->globalDirtyState[__GL_CLIP_ATTRS];

    for (i = 0; i < gc->constants.numberOfClipPlanes; i++) {
        GLuint planeBit = (1 << i);
        GLuint enableBit = (1 << (i + 16));

        /* The lower 16 bits of localMask are for clipPlane parameters.
        */
        __GL_CHECK_ATTR4(planeBit,
            transform.eyeClipPlanes[i].x,
            transform.eyeClipPlanes[i].y,
            transform.eyeClipPlanes[i].z,
            transform.eyeClipPlanes[i].w);

        /* The upper 16 bits of localMask are for clipPlane enable/disable.
        */
        if (localMask & enableBit) {
            if ((ds->enables.transform.clipPlanesMask & planeBit) !=
                (cs->enables.transform.clipPlanesMask & planeBit))
            {
                if (cs->enables.transform.clipPlanesMask & planeBit) {
                    ds->enables.transform.clipPlanesMask |= planeBit;
                }
                else {
                    ds->enables.transform.clipPlanesMask &= ~planeBit;
                }
            }
            else {
                localMask &= ~(enableBit);
            }
        }
    }

    /* Reassign localMask back to globalDirtyState[__GL_CLIP_ATTRS]
    */
    gc->globalDirtyState[__GL_CLIP_ATTRS] = localMask;
    if (gc->globalDirtyState[__GL_CLIP_ATTRS] == 0) {
        gc->globalDirtyState[__GL_ALL_ATTRS] &= ~(1 << __GL_CLIP_ATTRS);
    }
    else {
        /* Copy the dirty bits to swpDirtyState[__GL_CLIP_ATTRS]
        */
        gc->swpDirtyState[__GL_CLIP_ATTRS] |= localMask;
    }
}

__GL_INLINE GLvoid __glEvaluateTextureAttrib(__GLcontext* gc, __GLattribute* cs, __GLattribute* ds)
{
    GLuint64 unitMask = gc->texUnitAttrDirtyMask;
    GLuint i;
    GLuint64 localMask;
    GLuint enabledDim, lastEnableDim;
    GLuint vsEnableDim, psEnableDim;

    while (unitMask) {
        __GLtextureObject * lastTexObj;

        __GL_BitScanForward64(&i, unitMask);
        __GL_BitTestAndReset64(&unitMask, i);

        localMask = gc->texUnitAttrState[i];

        /* Save the last enabled texture dimension and last texture object.
        */
        lastEnableDim = ds->texture.texUnits[i].currentEnableDim;
        lastTexObj = gc->texture.units[i].currentTexture;

        /*
        ** Pick really enabled dimension by information of fix function and programmable path.
        */
        vsEnableDim = cs->enables.texUnits[i].programVSEnabledDimension;
        if (cs->enables.texUnits[i].programPSEnabledDimension)
        {
            psEnableDim = cs->enables.texUnits[i].programPSEnabledDimension;
        }
        else
        {
            psEnableDim = cs->enables.texUnits[i].enabledDimension;
        }

        if(vsEnableDim && psEnableDim)
        {
            if(vsEnableDim != psEnableDim)
            {
                enabledDim = 0;
                __glSetError(GL_INVALID_OPERATION);
                gc->texture.texConflit |= (1 << i);
            }
            else
            {
                enabledDim = psEnableDim;
                gc->texture.texConflit &= ~(1 << i);
            }
        }
        else
        {
            enabledDim = psEnableDim ? psEnableDim : vsEnableDim;
            gc->texture.texConflit &= ~(1 << i);
        }


#if (defined(DEBUG) || defined(_DEBUG))
        {
            static GLuint uForceNoTexture = 0;
            if (uForceNoTexture)
                enabledDim = 0;
        }
#endif
        /*
        ** Set the "currentTexture" pointer and the "currentEnableDim" variable
        ** based on the result of texture consistency (completeness) check.
        ** NOTE: The currentEnableDim is accurate texture dimension.
        ** So don't check gc->state.enables.texUnits[i].enabledDimension or
        ** gc->state.enables.texUnits[i].programVS/PSEnabledDimension after __glEvaluateAttributeChange.
        */
        gc->texture.units[i].currentTexture = NULL;
        cs->texture.texUnits[i].currentEnableDim = 0;
        gc->texture.currentEnableMask &= ~(__GL_ONE_64 << i);
        if (enabledDim)
        {
            __GLtextureObject *tex = gc->texture.units[i].boundTextures[enabledDim-1];
            GLenum baseFormat = tex->faceMipmap[0][tex->params.baseLevel].baseFormat;
            GLboolean undefined = GL_FALSE;

            /* TexEnv GL_DECAL mode doesn't matches texture image baseFormat (Spec2.0 p184) */
            if ((gc->state.texture.texUnits[i].env.mode == GL_DECAL) &&
                !(baseFormat == GL_RGB || baseFormat == GL_RGBA))
            {
                undefined = GL_TRUE;
            }
            if (__glIsTextureConsistent(gc, tex) && !undefined)
            {
                gc->texture.units[i].currentTexture = tex;
                cs->texture.texUnits[i].currentEnableDim = enabledDim;
                gc->texture.currentEnableMask |= (1 << i);
            }
        }

        /* Check if "currentEnableDim" is really changed.
        */
        if (lastEnableDim != cs->texture.texUnits[i].currentEnableDim)
        {
            ds->texture.texUnits[i].currentEnableDim = cs->texture.texUnits[i].currentEnableDim;
            localMask |= __GL_TEX_ENABLE_DIM_CHANGED_BIT;
        }
        else {
            localMask &= ~__GL_TEX_ENABLE_DIM_CHANGED_BIT;
        }

        if (localMask & __GL_TEX_ENABLE_DIM_CHANGED_BIT)
        {
            /* force set all dirty bits for texture parameters if texture is enabled.
            */
            if (cs->texture.texUnits[i].currentEnableDim > 0) {
                localMask |= __GL_TEXPARAMETER_BITS;
            }

            /* Texture image dirty bits need to be set whenever currentEnableDim is changed.
            */
            localMask |= __GL_TEXIMAGE_BITS;
        }

        if (localMask & __GL_TEXGEN_BITS) {
            __GL_CHECK_ATTR1(__GL_TEXGEN_S_ENDISABLE_BIT,
                enables.texUnits[i].texGen[0]);

            __GL_CHECK_ATTR1(__GL_TEXGEN_T_ENDISABLE_BIT,
                enables.texUnits[i].texGen[1]);

            __GL_CHECK_ATTR1(__GL_TEXGEN_R_ENDISABLE_BIT,
                enables.texUnits[i].texGen[2]);

            __GL_CHECK_ATTR1(__GL_TEXGEN_Q_ENDISABLE_BIT,
                enables.texUnits[i].texGen[3]);

            if (localMask & __GL_TEXGEN_S_BIT) {
                switch(cs->texture.texUnits[i].s.mode) {
                case GL_OBJECT_LINEAR:
                    __GL_CHECK_ATTR5(__GL_TEXGEN_S_BIT,
                        texture.texUnits[i].s.mode,
                        texture.texUnits[i].s.objectPlaneEquation.x,
                        texture.texUnits[i].s.objectPlaneEquation.y,
                        texture.texUnits[i].s.objectPlaneEquation.z,
                        texture.texUnits[i].s.objectPlaneEquation.w);
                    break;
                case GL_EYE_LINEAR:
                    __GL_CHECK_ATTR5(__GL_TEXGEN_S_BIT,
                        texture.texUnits[i].s.mode,
                        texture.texUnits[i].s.eyePlaneEquation.x,
                        texture.texUnits[i].s.eyePlaneEquation.y,
                        texture.texUnits[i].s.eyePlaneEquation.z,
                        texture.texUnits[i].s.eyePlaneEquation.w);
                    break;
                default:
                    __GL_CHECK_ATTR1(__GL_TEXGEN_S_BIT,
                        texture.texUnits[i].s.mode);
                    break;
                }
            }

            if (localMask & __GL_TEXGEN_T_BIT) {
                switch(cs->texture.texUnits[i].t.mode) {
                case GL_OBJECT_LINEAR:
                    __GL_CHECK_ATTR5(__GL_TEXGEN_T_BIT,
                        texture.texUnits[i].t.mode,
                        texture.texUnits[i].t.objectPlaneEquation.x,
                        texture.texUnits[i].t.objectPlaneEquation.y,
                        texture.texUnits[i].t.objectPlaneEquation.z,
                        texture.texUnits[i].t.objectPlaneEquation.w);
                    break;
                case GL_EYE_LINEAR:
                    __GL_CHECK_ATTR5(__GL_TEXGEN_T_BIT,
                        texture.texUnits[i].t.mode,
                        texture.texUnits[i].t.eyePlaneEquation.x,
                        texture.texUnits[i].t.eyePlaneEquation.y,
                        texture.texUnits[i].t.eyePlaneEquation.z,
                        texture.texUnits[i].t.eyePlaneEquation.w);
                    break;
                default:
                    __GL_CHECK_ATTR1(__GL_TEXGEN_T_BIT,
                        texture.texUnits[i].t.mode);
                    break;
                }
            }

            if (localMask & __GL_TEXGEN_R_BIT) {
                switch(cs->texture.texUnits[i].r.mode) {
                case GL_OBJECT_LINEAR:
                    __GL_CHECK_ATTR5(__GL_TEXGEN_R_BIT,
                        texture.texUnits[i].r.mode,
                        texture.texUnits[i].r.objectPlaneEquation.x,
                        texture.texUnits[i].r.objectPlaneEquation.y,
                        texture.texUnits[i].r.objectPlaneEquation.z,
                        texture.texUnits[i].r.objectPlaneEquation.w);
                    break;
                case GL_EYE_LINEAR:
                    __GL_CHECK_ATTR5(__GL_TEXGEN_R_BIT,
                        texture.texUnits[i].r.mode,
                        texture.texUnits[i].r.eyePlaneEquation.x,
                        texture.texUnits[i].r.eyePlaneEquation.y,
                        texture.texUnits[i].r.eyePlaneEquation.z,
                        texture.texUnits[i].r.eyePlaneEquation.w);
                    break;
                default:
                    __GL_CHECK_ATTR1(__GL_TEXGEN_R_BIT,
                        texture.texUnits[i].r.mode);
                    break;
                }
            }

            if (localMask & __GL_TEXGEN_Q_BIT) {
                switch(cs->texture.texUnits[i].q.mode) {
                case GL_OBJECT_LINEAR:
                    __GL_CHECK_ATTR5(__GL_TEXGEN_Q_BIT,
                        texture.texUnits[i].q.mode,
                        texture.texUnits[i].q.objectPlaneEquation.x,
                        texture.texUnits[i].q.objectPlaneEquation.y,
                        texture.texUnits[i].q.objectPlaneEquation.z,
                        texture.texUnits[i].q.objectPlaneEquation.w);
                    break;
                case GL_EYE_LINEAR:
                    __GL_CHECK_ATTR5(__GL_TEXGEN_Q_BIT,
                        texture.texUnits[i].q.mode,
                        texture.texUnits[i].q.eyePlaneEquation.x,
                        texture.texUnits[i].q.eyePlaneEquation.y,
                        texture.texUnits[i].q.eyePlaneEquation.z,
                        texture.texUnits[i].q.eyePlaneEquation.w);
                    break;
                default:
                    __GL_CHECK_ATTR1(__GL_TEXGEN_Q_BIT,
                        texture.texUnits[i].q.mode);
                    break;
                }
            }
        }

        if (localMask & __GL_TEXENV_BITS) {
            __GL_CHECK_ATTR1(__GL_TEXENV_MODE_BIT,
                texture.texUnits[i].env.mode);

            __GL_CHECK_ATTR4(__GL_TEXENV_COLOR_BIT,
                texture.texUnits[i].env.color.r,
                texture.texUnits[i].env.color.g,
                texture.texUnits[i].env.color.b,
                texture.texUnits[i].env.color.a);

            __GL_CHECK_ATTR1(__GL_TEXENV_COMBINE_ALPHA_BIT,
                texture.texUnits[i].env.function.alpha);

            __GL_CHECK_ATTR1(__GL_TEXENV_COMBINE_RGB_BIT,
                texture.texUnits[i].env.function.rgb);

            __GL_CHECK_ATTR1(__GL_TEXENV_SOURCE0_RGB_BIT,
                texture.texUnits[i].env.source[0].rgb);

            __GL_CHECK_ATTR1(__GL_TEXENV_SOURCE1_RGB_BIT,
                texture.texUnits[i].env.source[1].rgb);

            __GL_CHECK_ATTR1(__GL_TEXENV_SOURCE2_RGB_BIT,
                texture.texUnits[i].env.source[2].rgb);

            __GL_CHECK_ATTR1(__GL_TEXENV_SOURCE0_ALPHA_BIT,
                texture.texUnits[i].env.source[0].alpha);

            __GL_CHECK_ATTR1(__GL_TEXENV_SOURCE1_ALPHA_BIT,
                texture.texUnits[i].env.source[1].alpha);

            __GL_CHECK_ATTR1(__GL_TEXENV_SOURCE2_ALPHA_BIT,
                texture.texUnits[i].env.source[2].alpha);

            __GL_CHECK_ATTR1(__GL_TEXENV_OPERAND0_RGB_BIT,
                texture.texUnits[i].env.operand[0].rgb);

            __GL_CHECK_ATTR1(__GL_TEXENV_OPERAND1_RGB_BIT,
                texture.texUnits[i].env.operand[1].rgb);

            __GL_CHECK_ATTR1(__GL_TEXENV_OPERAND2_RGB_BIT,
                texture.texUnits[i].env.operand[2].rgb);

            __GL_CHECK_ATTR1(__GL_TEXENV_OPERAND0_ALPHA_BIT,
                texture.texUnits[i].env.operand[0].alpha);

            __GL_CHECK_ATTR1(__GL_TEXENV_OPERAND1_ALPHA_BIT,
                texture.texUnits[i].env.operand[1].alpha);

            __GL_CHECK_ATTR1(__GL_TEXENV_OPERAND2_ALPHA_BIT,
                texture.texUnits[i].env.operand[2].alpha);

            __GL_CHECK_ATTR1(__GL_TEXENV_RGB_SCALE_BIT,
                texture.texUnits[i].env.rgbScale);

            __GL_CHECK_ATTR1(__GL_TEXENV_ALPHA_SCALE_BIT,
                texture.texUnits[i].env.alphaScale);
        }

        /* There is no need to check texParameter redundancy if texture is disabled.
        */
        if (localMask & __GL_TEXPARAMETER_BITS)
        {
            if (enabledDim && lastEnableDim)
            {
                __GLtextureParamState *cs_params = &cs->texture.texUnits[i].texObj[enabledDim-1].params;
                __GLtextureParamState *ds_params = &ds->texture.texUnits[i].texObj[0].params;
                if(__GL_MEMCMP(ds_params, cs_params, sizeof(__GLtextureParamState)))
                {
                    __GL_CHECK_TEX_PARAM1(__GL_TEXPARAM_WRAP_S_BIT, sWrapMode);
                    __GL_CHECK_TEX_PARAM1(__GL_TEXPARAM_WRAP_T_BIT, tWrapMode);
                    __GL_CHECK_TEX_PARAM1(__GL_TEXPARAM_WRAP_R_BIT, rWrapMode);
                    __GL_CHECK_TEX_PARAM1(__GL_TEXPARAM_MIN_FILTER_BIT, minFilter);
                    __GL_CHECK_TEX_PARAM1(__GL_TEXPARAM_MAG_FILTER_BIT, magFilter);
                    __GL_CHECK_TEX_PARAM1(__GL_TEXPARAM_PRIORITY_BIT, priority);
                    __GL_CHECK_TEX_PARAM1(__GL_TEXPARAM_MIN_LOD_BIT, minLod);
                    __GL_CHECK_TEX_PARAM1(__GL_TEXPARAM_MAX_LOD_BIT, maxLod);
                    __GL_CHECK_TEX_PARAM1(__GL_TEXPARAM_BASE_LEVEL_BIT, baseLevel);
                    __GL_CHECK_TEX_PARAM1(__GL_TEXPARAM_MAX_LEVEL_BIT, maxLevel);
                    __GL_CHECK_TEX_PARAM1(__GL_TEXPARAM_LOD_BIAS_BIT, lodBias);
                    __GL_CHECK_TEX_PARAM1(__GL_TEXPARAM_DEPTH_TEX_MODE_BIT, depthTexMode);
                    __GL_CHECK_TEX_PARAM1(__GL_TEXPARAM_COMPARE_MODE_BIT, compareMode);
                    __GL_CHECK_TEX_PARAM1(__GL_TEXPARAM_COMPARE_FUNC_BIT, compareFunc);
                    __GL_CHECK_TEX_PARAM1(__GL_TEXPARAM_GENERATE_MIPMAP_BIT, generateMipmap);
                    __GL_CHECK_TEX_PARAM1(__GL_TEXPARAM_COMPARE_FAIL_VALUE_BIT, compareFailValue);
                    __GL_CHECK_TEX_PARAM1(__GL_TEXPARAM_MAX_ANISOTROPY_BIT, anisotropicLimit);
                    __GL_CHECK_TEX_PARAM_BORDERCOLOR();

                }
                else
                {
                    localMask &= ~__GL_TEXPARAMETER_BITS;
                }
            }
            else if (enabledDim && !lastEnableDim)
            {
                ds->texture.texUnits[i].texObj[0].params = cs->texture.texUnits[i].texObj[enabledDim-1].params;
            }
        }

        if(localMask & __GL_TEX_IMAGE_FORMAT_CHANGED_BIT)
        {
            __GLtextureObject * currentTexObj = gc->texture.units[i].currentTexture;

            /* If lastTexObj == currentTexObj, baseFormat may have been changed.
            ** We shouldn't clear __GL_TEX_IMAGE_FORMAT_CHANGED_BIT for such case.
            */
            if(lastTexObj &&  currentTexObj && (lastTexObj != currentTexObj) &&
                lastTexObj->faceMipmap[0][lastTexObj->params.baseLevel].baseFormat ==
                currentTexObj->faceMipmap[0][currentTexObj->params.baseLevel].baseFormat)
            {
                localMask &= ~(__GL_TEX_IMAGE_FORMAT_CHANGED_BIT);
            }
            else
            {
                /* InternalBorderColor depends on base format */
                localMask |= __GL_TEXPARAM_BORDER_COLOR_BIT;
            }
        }

        /* As to depth texture, InternalBorderColor also depends on depthTexMode */
        if (localMask & __GL_TEXPARAM_DEPTH_TEX_MODE_BIT)
        {
            __GLtextureObject *currentTexObj = gc->texture.units[i].currentTexture;
            if (currentTexObj && currentTexObj->faceMipmap[0][currentTexObj->params.baseLevel].baseFormat == GL_DEPTH_COMPONENT)
            {
                localMask |= __GL_TEXPARAM_BORDER_COLOR_BIT;
            }
        }

        __GL_CHECK_ATTR1(__GL_TEX_UNIT_LODBIAS_BIT, texture.texUnits[i].lodBias);

        /* Reassign localMask back to texUnitAttrState[i]
        */
        gc->texUnitAttrState[i] = localMask;
        if (localMask == 0) {
            gc->texUnitAttrDirtyMask &= ~(__GL_ONE_64 << i);
        }
        else {
            /* Copy the dirty bits to swpTexUnitAttrState[i].
            */
            gc->swpTexUnitAttrState[i] |= localMask;
        }
    }

    if(gc->shaderProgram.currentShaderProgram)
    {
        __GLshaderProgramObject* programObject = gc->shaderProgram.currentShaderProgram;

        if(gc->texture.texConflit || programObject->bindingInfo.texConflict)
        {
            gc->flags |= __GL_DISCARD_FOLLOWING_DRAWS_INVALID_TEXDIM;
            programObject->programInfo.invalidFlag |= GLSL_INVALID_TEX_BIT;

        }
        else
        {
            gc->flags &= ~__GL_DISCARD_FOLLOWING_DRAWS_INVALID_TEXDIM;
            programObject->programInfo.invalidFlag &= ~GLSL_INVALID_TEX_BIT;

        }
    }
    else
    {
        gc->flags &= ~__GL_DISCARD_FOLLOWING_DRAWS_INVALID_TEXDIM;
    }

    if (gc->texUnitAttrDirtyMask == 0) {
        gc->globalDirtyState[__GL_ALL_ATTRS] &= ~(1 << __GL_TEX_UNIT_ATTRS);
    }
    else {
        /* Copy the dirty bits to gc->swpTexUnitAttrDirtyMask
        */
        gc->swpTexUnitAttrDirtyMask |= gc->texUnitAttrDirtyMask;
    }
}

__GL_INLINE GLvoid __glEvaluateProgramAttrib(__GLcontext* gc, __GLattribute* cs, __GLattribute* ds)
{
    GLbitfield localMask = gc->globalDirtyState[__GL_PROGRAM_ATTRS];
    __GLshaderProgramMachine *shaderProgramMachine = &gc->shaderProgram;
    GLboolean bVsSamplerChanged = GL_FALSE, bPsSamplerChanged = GL_FALSE, bGsSamplerChanged = GL_FALSE;
    GLboolean bTextureEnableDimChanged = GL_FALSE;

    __GL_CHECK_ATTR1(__GL_DIRTY_VERTEX_PROGRAM_ENABLE,
        enables.program.vertexProgram);
    __GL_CHECK_ATTR1(__GL_DIRTY_VP_POINT_SIZE_ENABLE,
        enables.program.vpPointSize);
    __GL_CHECK_ATTR1(__GL_DIRTY_VP_TWO_SIDE_ENABLE,
        enables.program.vpTwoSize);
    __GL_CHECK_ATTR1(__GL_DIRTY_FRAGMENT_PROGRAM_ENABLE,
        enables.program.fragmentProgram);

    if(localMask & __GL_DIRTY_GLSL_PROGRAM_SWITCH)
    {
        GLuint currentProgram = 0;
        GLuint currentCodeSeq = 0;
        if(shaderProgramMachine->currentShaderProgram)
        {
            currentProgram = shaderProgramMachine->currentShaderProgram->objectInfo.id;
            currentCodeSeq = shaderProgramMachine->currentShaderProgram->programInfo.codeSeq;
        }

        if( (shaderProgramMachine->lastProgram == currentProgram) &&
            (shaderProgramMachine->lastCodeSeq == currentCodeSeq) )
        {
            localMask &= ~__GL_DIRTY_GLSL_PROGRAM_SWITCH;
        }
        else
        {
            if(shaderProgramMachine->lastVertShaderEnable != shaderProgramMachine->vertShaderEnable)
            {
                localMask |= __GL_DIRTY_GLSL_VS_ENABLE;
            }

            if(shaderProgramMachine->lastGeomShaderEnable != shaderProgramMachine->geomShaderEnable)
            {
                localMask |= __GL_DIRTY_GLSL_GS_ENABLE;
            }

            if(shaderProgramMachine->lastFragShaderEnable != shaderProgramMachine->fragShaderEnable)
            {
                localMask |= __GL_DIRTY_GLSL_FS_ENABLE;
            }

            if(shaderProgramMachine->lastVertShaderEnable && shaderProgramMachine->vertShaderEnable)
            {
                localMask |= __GL_DIRTY_GLSL_VS_SWITCH;
            }

            if(shaderProgramMachine->lastGeomShaderEnable && shaderProgramMachine->geomShaderEnable)
            {
                localMask |= __GL_DIRTY_GLSL_GS_SWITCH;
            }

            if(shaderProgramMachine->lastFragShaderEnable && shaderProgramMachine->fragShaderEnable)
            {
                localMask |= __GL_DIRTY_GLSL_FS_SWITCH;
            }

            localMask |= __GL_DIRTY_GLSL_SAMPLER;
            localMask |= __GL_DIRTY_GLSL_FS_OUTPUT;


            shaderProgramMachine->lastProgram = currentProgram;
            shaderProgramMachine->lastCodeSeq = currentCodeSeq;
            shaderProgramMachine->lastVertShaderEnable = shaderProgramMachine->vertShaderEnable;
            shaderProgramMachine->lastGeomShaderEnable = shaderProgramMachine->geomShaderEnable;
            shaderProgramMachine->lastFragShaderEnable = shaderProgramMachine->fragShaderEnable;
        }
    }

    /* If there's sampler bound change in the program object, the texEnableDim and texUnit2Sampler array should
    ** be rebuilt. If there's only program switch, they do not need to be rebuilt.
    */
    if (localMask & __GL_DIRTY_GLSL_SAMPLER)
    {
        GLuint *prevSampler2TexUnit = gc->shaderProgram.prevSampler2TexUnit;
        GLuint *sampler2TexUnit, i;

        if (gc->shaderProgram.currentShaderProgram)
        {
            sampler2TexUnit = gc->shaderProgram.currentShaderProgram->bindingInfo.sampler2TexUnit;
        }
        else
        {
            sampler2TexUnit = fixFuncSampler2TexUnit;
        }

        /* If GLSL is disabled and bit __GL_DIRTY_GLSL_SAMPLER is TRUE,
        ** bit __GL_DIRTY_GLSL_PROGRAM_SWITCH must be TURE.
        */
        if (localMask & __GL_DIRTY_GLSL_PROGRAM_SWITCH)
        {
            /* build gc->shaderProgram.samplerDirtyState */
            for (i = 0; i < __GL_MAX_GLSL_SAMPLERS; i++)
            {
                if (sampler2TexUnit[i] != prevSampler2TexUnit[i])
                {
                    gc->shaderProgram.samplerDirtyState |= (__GL_ONE_64 << i);
                }
            }
            /* If Program has switch,need buildTextureEnableDim */
            bTextureEnableDimChanged = GL_TRUE;
        }

        if (gc->shaderProgram.currentShaderProgram)
        {
            if (gc->shaderProgram.samplerDirtyState)
            {
                if (gc->shaderProgram.samplerDirtyState & __GL_GLSL_VS_SAMPLER_MASK)
                {
                    bVsSamplerChanged = GL_TRUE;
                }
                if (gc->shaderProgram.samplerDirtyState & __GL_GLSL_PS_SAMPLER_MASK)
                {
                    bPsSamplerChanged = GL_TRUE;
                }
                if (gc->shaderProgram.samplerDirtyState & __GL_GLSL_GS_SAMPLER_MASK)
                {
                    bGsSamplerChanged = GL_TRUE;
                }

                /*If Sampler is dirty,need buildTextureEnableDim*/
                bTextureEnableDimChanged = GL_TRUE;
            }

            if (bTextureEnableDimChanged)
            {
                __glSLangBuildTextureEnableDim(gc);
                bTextureEnableDimChanged = GL_FALSE;
            }
        }

        __GL_MEMCOPY(prevSampler2TexUnit, sampler2TexUnit, sizeof(GLuint) * __GL_MAX_GLSL_SAMPLERS);
    }


    /* VS */
    if(localMask & __GL_DIRTY_PROGRAM_VS_SWITCH )
    {
        /* Check program vs real enable */
        if( localMask & (__GL_DIRTY_GLSL_VS_ENABLE | __GL_DIRTY_GLSL_VS_SWITCH) )
        {
            if(shaderProgramMachine->vertShaderEnable)
            {
                __GLshaderProgramObject *programObject = shaderProgramMachine->currentShaderProgram;
                GL_ASSERT(programObject);
                if(gc->dp.validateShaderProgram(gc, programObject))
                {
                    shaderProgramMachine->vertShaderRealEnable = GL_TRUE;
                    programObject->programInfo.invalidFlag &= ~GLSL_INVALID_CODE_BIT;
                }
                else
                {
                    shaderProgramMachine->vertShaderRealEnable = GL_FALSE;
                    programObject->programInfo.invalidFlag |= GLSL_INVALID_CODE_BIT;
                }
            }
        }

        if (localMask & (__GL_DIRTY_VERTEX_PROGRAM_ENABLE | __GL_DIRTY_VERTEX_PROGRAM_SWITCH))
        {
            if (gc->state.enables.program.vertexProgram)
            {
                __glVertexProgramRealEnabled(gc);
            }
        }

        /* If program vs is enabled but invalid, discard this draw. */
        if(shaderProgramMachine->vertShaderEnable)
        {
            if(shaderProgramMachine->vertShaderRealEnable)
            {
                gc->flags &= ~__GL_DISCARD_FOLLOWING_DRAWS_INVALID_PROGRAM_VS;
            }
            else
            {
                gc->flags |= __GL_DISCARD_FOLLOWING_DRAWS_INVALID_PROGRAM_VS;
            }
        }
        else if(gc->state.enables.program.vertexProgram)
        {
            if(gc->program.realEnabled[__GL_VERTEX_PROGRAM_INDEX])
            {
                gc->flags &= ~__GL_DISCARD_FOLLOWING_DRAWS_INVALID_PROGRAM_VS;
            }
            else
            {
                gc->flags |= __GL_DISCARD_FOLLOWING_DRAWS_INVALID_PROGRAM_VS;
            }
        }
        else
        {
            gc->flags &= ~__GL_DISCARD_FOLLOWING_DRAWS_INVALID_PROGRAM_VS;
        }
    }

    if ((localMask & (__GL_DIRTY_GLSL_VS_ENABLE | __GL_DIRTY_GLSL_VS_SWITCH)) || bVsSamplerChanged)
    {
        __glSetProgramVSEnabledDimension(gc);
    }

    /* GS */
    if(localMask & __GL_DIRTY_PROGRAM_GS_SWITCH )
    {
        /* Check program gs real enable */
        if(shaderProgramMachine->geomShaderEnable)
        {
            __GLshaderProgramObject *programObject = shaderProgramMachine->currentShaderProgram;
            GL_ASSERT(programObject);
            if(gc->dp.validateShaderProgram(gc, programObject))
            {
                shaderProgramMachine->geomShaderRealEnable = GL_TRUE;
                programObject->programInfo.invalidFlag &= ~GLSL_INVALID_CODE_BIT;
            }
            else
            {
                shaderProgramMachine->geomShaderRealEnable = GL_FALSE;
                programObject->programInfo.invalidFlag |= GLSL_INVALID_CODE_BIT;
            }
        }

        /* If program vs is enabled but invalid, discard this draw. */
        if(shaderProgramMachine->geomShaderEnable)
        {
            if(shaderProgramMachine->geomShaderRealEnable)
            {
                gc->flags &= ~__GL_DISCARD_FOLLOWING_DRAWS_INVALID_PROGRAM_GS;
            }
            else
            {
                gc->flags |= __GL_DISCARD_FOLLOWING_DRAWS_INVALID_PROGRAM_GS;
            }
        }
        else
        {
            gc->flags &= ~__GL_DISCARD_FOLLOWING_DRAWS_INVALID_PROGRAM_GS;
        }
    }

    if ((localMask & __GL_DIRTY_PROGRAM_GS_SWITCH) || bGsSamplerChanged)
    {
        __glSetProgramGSEnabledDimension(gc);
    }

    /* PS */
    if (localMask & __GL_DIRTY_PROGRAM_PS_SWITCH)
    {
        /* Check program ps real enable */
        if( localMask & (__GL_DIRTY_GLSL_FS_ENABLE | __GL_DIRTY_GLSL_FS_SWITCH) )
        {
            if(shaderProgramMachine->fragShaderEnable)
            {
                __GLshaderProgramObject *programObject = shaderProgramMachine->currentShaderProgram;
                GL_ASSERT(programObject);
                if(gc->dp.validateShaderProgram(gc, programObject))
                {
                    shaderProgramMachine->fragShaderRealEnable = GL_TRUE;
                    programObject->programInfo.invalidFlag &= ~GLSL_INVALID_CODE_BIT;
                }
                else
                {
                    shaderProgramMachine->fragShaderRealEnable = GL_FALSE;
                    programObject->programInfo.invalidFlag |= GLSL_INVALID_CODE_BIT;
                }
            }
        }

        if (localMask & (__GL_DIRTY_FRAGMENT_PROGRAM_ENABLE | __GL_DIRTY_FRAGMENT_PROGRAM_SWITCH))
        {
            if (gc->state.enables.program.fragmentProgram)
            {
                __glFragmentProgramRealEnabled(gc);
            }
        }

        /* If program ps is enabled but invalid, discard this draw. */
        if(shaderProgramMachine->fragShaderEnable)
        {
            if(shaderProgramMachine->fragShaderRealEnable)
            {
                gc->flags &= ~__GL_DISCARD_FOLLOWING_DRAWS_INVALID_PROGRAM_PS;
            }
            else
            {
                gc->flags |= __GL_DISCARD_FOLLOWING_DRAWS_INVALID_PROGRAM_PS;
            }
        }
        else if(gc->state.enables.program.fragmentProgram)
        {
            if(gc->program.realEnabled[__GL_FRAGMENT_PROGRAM_INDEX])
            {
                gc->flags &= ~__GL_DISCARD_FOLLOWING_DRAWS_INVALID_PROGRAM_PS;
            }
            else
            {
                gc->flags |= __GL_DISCARD_FOLLOWING_DRAWS_INVALID_PROGRAM_PS;
            }
        }
        else
        {
            /* For ATI FS, if it is invalid, we choose render following draw by fix function */
            gc->flags &= ~__GL_DISCARD_FOLLOWING_DRAWS_INVALID_PROGRAM_PS;
        }

    }

    if ((localMask & __GL_DIRTY_PROGRAM_PS_SWITCH) || bPsSamplerChanged)
    {
        __glSetProgramPSEnabledDimension(gc);
    }

    gc->globalDirtyState[__GL_PROGRAM_ATTRS] = localMask;
}

GLvoid __glEvaluateAttributeChange(__GLcontext *gc)
{
    __GLattribute *cs = &gc->state;
    __GLattribute *ds = &gc->commitState;

#if PRINT_ATTRIB
    printf("\n\n__glEvaluateAttributeChange ---- Before Evaluation: \n");
    __glPrintOutAttributeChanges(gc);
#endif

    /* Check the dirty bits in globalDirtyState[__GL_DIRTY_ATTRS_1]
     */
    if (gc->globalDirtyState[__GL_DIRTY_ATTRS_1]) {
        __glEvaluateAttribGroup1(gc, cs, ds);
    }

    /* Check the dirty bits in globalDirtyState[__GL_DIRTY_ATTRS_2]
     */
    if (gc->globalDirtyState[__GL_DIRTY_ATTRS_2]) {
        __glEvaluateAttribGroup2(gc, cs, ds);
    }

    /* Check the dirty bits in globalDirtyState[__GL_DIRTY_ATTRS_3]
     */
    if (gc->globalDirtyState[__GL_DIRTY_ATTRS_3]) {
        __glEvaluateAttribGroup3(gc, cs, ds);
    }

    /* Check the dirty bits in globalDirtyState[__GL_LIGHTING_ATTRS]
     */
    if (gc->globalDirtyState[__GL_LIGHTING_ATTRS]) {
        __glEvaluateLightingAttrib(gc, cs, ds);
    }

    /* Check the dirty bits in lightAttrState[0 .. __GL_MAX_LIGHT_NUM]
     */
    if (gc->globalDirtyState[__GL_LIGHT_SRC_ATTRS]) {
        __glEvaluateLightSrcAttrib(gc, cs, ds);
    }

    /* Check the dirty bits in globalDirtyState[__GL_CLIP_ATTRS]
     */
    if (gc->globalDirtyState[__GL_CLIP_ATTRS]) {
        __glEvaluateClipAttrib(gc, cs, ds);
    }

    /* Program state must be checked before texture.
     */
    if (gc->globalDirtyState[__GL_PROGRAM_ATTRS]) {
        __glEvaluateProgramAttrib(gc, cs, ds);
    }

    /* Check the dirty bits in texUnitAttrState[0 .. __GL_TEXTURE_MAX_UNITS]
     */
    if (gc->texUnitAttrDirtyMask) {
        __glEvaluateTextureAttrib(gc, cs, ds);
    }

#if PRINT_ATTRIB
    printf("\n__glEvaluateAttributeChange ---- After Evaluation: \n");
    __glPrintOutAttributeChanges(gc);
#endif

    if (gc->globalDirtyState[__GL_ALL_ATTRS]) {

        /* Copy the dirty bits to swpDirtyState[__GL_ALL_ATTRS]
        */
        gc->swpDirtyState[__GL_ALL_ATTRS] |= gc->globalDirtyState[__GL_ALL_ATTRS];

        /* Notify DP all the attribute changes.
        */
        (*gc->dp.attributeChanged)(gc);
        /* Clear all the attribute dirty bits.
        */
        __glClearAttributeStates(gc);
    }
}

/***** Vertex stream config functions ********/

GLvoid __glConfigImmedVertexStream(__GLcontext *gc, GLenum mode)
{
    __GLvertexInput *input = NULL;
    __GLstreamDecl *stream;
    __GLvertexElement *element;
    GLint offset;
    GLubyte inputIdx;
    GLuint mask, i;

    /* Immediate mode vertex input always use single stream */
    gc->vertexStreams.numStreams = 1;
    gc->vertexStreams.endVertex = gc->input.vertex.index;
    gc->vertexStreams.indexCount = gc->input.indexCount;
    gc->vertexStreams.startVertex = 0;
    gc->vertexStreams.primElemSequence = gc->input.primElemSequence;
    gc->vertexStreams.primElementMask = gc->input.primInputMask;
    gc->vertexStreams.missingAttribs = gc->input.requiredInputMask & (~gc->input.primInputMask) & (~__GL_INPUT_EDGEFLAG) & (~__GL_INPUT_VERTEX);
    gc->vertexStreams.edgeflagStream =
        (gc->input.primInputMask & __GL_INPUT_EDGEFLAG) ? gc->input.edgeflag.pointer : NULL;

    stream = &gc->vertexStreams.streams[0];
    stream->numElements = gc->input.numberOfElements;
    stream->streamAddr = (GLuint *)gc->input.vertexDataBuffer;
    stream->stride = (gc->input.vertTotalStrideDW << 2);
    stream->privPtrAddr = NULL;

    offset = 0;
    for (i = 0; i < gc->input.numberOfElements; i++)
    {
        element = &stream->streamElement[i];

        inputIdx = 0;
        mask = gc->input.primInputMask & (~__GL_INPUT_EDGEFLAG);
        while (mask)
        {
            if ((mask & 0x1) && (gc->input.currentInput[inputIdx].offsetDW == offset))
            {
                input = &gc->input.currentInput[inputIdx];
                offset += input->sizeDW;
                break;
            }
            mask >>= 1;
            inputIdx += 1;
        }

        GL_ASSERT( ( 1 << inputIdx ) & gc->input.primInputMask );

        element->inputIndex = inputIdx;
        element->streamIndex = 0;
        element->offset = (input->offsetDW << 2);
        element->size = input->sizeDW;
        element->type = GL_FLOAT;
        element->normalized = GL_FALSE;
        element->integer = GL_FALSE;
        if (element->inputIndex == __GL_INPUT_DIFFUSE_INDEX && element->size == 1)
        {
            /* Special case for RGBA ubyte format */
            element->size = 4;
            element->type = GL_UNSIGNED_BYTE;
            element->normalized = GL_TRUE;
        }
    }

    gc->vertexStreams.streamMode = IMMEDIATE_STREAMMODE;
}

GLvoid __glConfigArrayVertexStream(__GLcontext *gc, GLenum mode)
{
    GLuint indexBuffer = gc->bufferObject.boundBuffer[__GL_ELEMENT_ARRAY_BUFFER_INDEX];
    GLuint arrayEnabled = gc->clientState.vertexArray.currentEnabled & (~(__GL_VARRAY_EDGEFLAG));
    __GLstreamDecl *stream;
    __GLvertexElement *element;
    __GLvertexArray *array;
    GLubyte arrayIdx, streamIdx, numStreams;
    GLuint mask, i;
    GLboolean newStream;
    __GLbufferObject *bufObj;

    gc->vertexStreams.primElemSequence = 0;
    gc->vertexStreams.primElementMask = arrayEnabled;
    gc->vertexStreams.missingAttribs = gc->input.requiredInputMask & (~arrayEnabled) & (~__GL_VARRAY_EDGEFLAG) & (~__GL_INPUT_VERTEX);
    gc->vertexStreams.edgeflagStream = NULL;

    arrayEnabled = gc->clientState.vertexArray.currentEnabled;
    if(arrayEnabled & (__GL_VARRAY_EDGEFLAG))
    {
        array = &gc->clientState.vertexArray.currentArrays[__GL_VARRAY_EDGEFLAG_INDEX];
        if( array->bufBinding == 0 )
        {
            gc->vertexStreams.edgeflagStream = (GLubyte *)array->pointer;
        }
        else
        {
            /* Else for buffer object todo later */
            GL_ASSERT(0);
        }
    }

    arrayEnabled &= ~(__GL_VARRAY_EDGEFLAG);
    gc->vertexStreams.indexCount = gc->vertexArray.indexCount;
    if(gc->vertexStreams.indexCount)
    {
        /*configure the index stream*/
        gc->vertexStreams.indexStream.type = gc->vertexArray.indexType;
        if(indexBuffer == 0 )
        {
            gc->vertexStreams.indexStream.ppIndexBufPriv = NULL;
            gc->vertexStreams.indexStream.streamAddr = (GLvoid *)gc->vertexArray.indices;
            gc->vertexStreams.indexStream.offset = 0;
        }
        else
        {
            gc->vertexStreams.indexStream.ppIndexBufPriv = &gc->bufferObject.boundTarget[__GL_ELEMENT_ARRAY_BUFFER_INDEX]->privateData;
            gc->vertexStreams.indexStream.streamAddr = NULL;
            gc->vertexStreams.indexStream.offset = gc->vertexArray.indexOffset;
        }
    }

    gc->vertexStreams.startVertex = gc->vertexArray.start;
    gc->vertexStreams.endVertex = gc->vertexArray.end;

    /*
    ** Now gc->vertexStreams.streamMode is the stream mode of last draw,
    ** and will be set to VERTEXARRAY_STREAMMODE at the end of this function.
    */
    if (gc->vertexArray.fastStreamSetup && gc->vertexStreams.streamMode == VERTEXARRAY_STREAMMODE)
    {
#if defined (_DEBUG) || defined (DEBUG)
        GLuint elementIdx;
        GLuint arrayIdx_2;
#endif
        for (streamIdx = 0; streamIdx < gc->vertexStreams.numStreams; streamIdx++ )
        {
            stream = &gc->vertexStreams.streams[streamIdx];
            arrayIdx = stream->streamElement[0].inputIndex;
            bufObj = gc->bufferObject.boundArrays[arrayIdx];
            if (bufObj && bufObj->size > 0)
            {
                GL_ASSERT(stream->privPtrAddr);
                GL_ASSERT(gc->clientState.vertexArray.currentArrays[arrayIdx].bufBinding);
                stream->privPtrAddr = &bufObj->privateData;
                stream->streamAddr = NULL;
            }
            else
            {
                GL_ASSERT(stream->privPtrAddr == NULL);
                GL_ASSERT(gc->clientState.vertexArray.currentArrays[arrayIdx].bufBinding == 0 ||(bufObj && bufObj->size == 0) );
                stream->streamAddr = (GLvoid *)gc->clientState.vertexArray.currentArrays[arrayIdx].pointer;
#if defined (_DEBUG) || defined (DEBUG)
                for ( elementIdx = 0; elementIdx < stream->numElements; elementIdx++ )
                {
                    arrayIdx_2 = stream->streamElement[elementIdx].inputIndex;
                    GL_ASSERT(stream->streamElement[elementIdx].offset == \
                        gc->clientState.vertexArray.currentArrays[arrayIdx_2].offset - \
                        gc->clientState.vertexArray.currentArrays[arrayIdx].offset);
                }
#endif
            }
        }

        return;
    }

    gc->vertexArray.fastStreamSetup = GL_FALSE;

    /* If both __GL_VARRAY_ATT0 and __GL_VARRAY_VERTEX enabled, according
    ** to spec, we prefer to send down the data of __GL_VARRAY_ATT0
    */
    if (arrayEnabled & __GL_VARRAY_ATT0)
    {
        mask = arrayEnabled & ~__GL_VARRAY_VERTEX;
    }else
    {
        mask = arrayEnabled;
    }

    arrayIdx = 0;
    numStreams = 0;

    while(mask)
    {
        if(mask & 0x1)
        {
            array = &gc->clientState.vertexArray.currentArrays[arrayIdx];
            bufObj = gc->bufferObject.boundArrays[arrayIdx];

            /* Corner case: FarCry call SecondaryColorPointer with size 4. currentArrays will not updated.*/
            if(array->stride == 0)
            {
                gc->vertexStreams.missingAttribs |= (1<<arrayIdx);
                arrayIdx++;
                mask >>= 1;
                continue;
            }

            if(array->stride < array->size * __glSizeOfType(array->type))
            {
                gc->vertexArray.immedFallback = GL_TRUE;
                return;
            }

            /*the first stage of checking whether we should configure another stream for this element*/
            newStream = GL_TRUE;/*default: YES, another stream*/
            for(streamIdx = 0; streamIdx < numStreams; streamIdx++)
            {
                stream = &gc->vertexStreams.streams[streamIdx];
                if( bufObj == NULL || bufObj->size == 0 )/*normal vertex array*/
                {
                    if(stream->privPtrAddr == NULL)/*stream source from conventional vertex array*/
                    {
                        newStream = GL_FALSE;
                        break;
                    }
                    else/*stream source from buffer object*/
                    {
                        continue;
                    }
                }
                else/*from buffer object*/
                {
                    if(stream->privPtrAddr == NULL)/*stream source from conventional vertex array*/
                    {
                        continue;
                    }
                    else/*stream source from buffer object*/
                    {
                        GL_ASSERT(bufObj);
                        if(&bufObj->privateData == stream->privPtrAddr)
                        {
                            newStream = GL_FALSE;
                            break;
                        }
                        else
                        {
                            continue;
                        }
                    }
                }
            }

            /*the second stage of checking whether we should configure another stream for this element*/
            if(!newStream)
            {
                __GLvertexElement *prevElement, tempElement;
                __GLvertexArray *prevArray;
                stream = &gc->vertexStreams.streams[streamIdx];

                prevElement = &stream->streamElement[stream->numElements-1];
                prevArray = &gc->clientState.vertexArray.currentArrays[prevElement->inputIndex];

                GL_ASSERT(prevArray->stride == stream->stride);
                GL_ASSERT(streamIdx == prevElement->streamIndex);

                /*array->offset may be an offset or a pointer. No matter what it is,
                (array->offset - prevArray->offset) is always what we want here*/
                if( (array->stride == stream->stride) && ( (GLuint)abs((GLint)glALL_TO_UINT32(array->offset - prevArray->offset)) < stream->stride ))
                {
                    /*configure the next element in the stream*/
                    element = &stream->streamElement[stream->numElements];
                    element->streamIndex = streamIdx;
                    element->inputIndex = arrayIdx;
                    element->size = array->size;
                    element->type = array->type;
                    element->normalized = array->normalized;
                    element->integer = array->integer;
                    element->offset = array->offset;
                    /*for conventional vertex arrays, this offset could be seen as an offset from NULL*/

                    /*insert the element to the stream according to offset*/
                    while( (element > stream->streamElement) && (element->offset < prevElement->offset) )
                    {
                        GL_ASSERT( element == prevElement + 1 );
                        tempElement = *prevElement;
                        *prevElement = *element;
                        *element = tempElement;
                        element--;
                        prevElement--;
                    }

                    /*streamAddr may have changed*/
                    stream->streamAddr = __GL_OFFSET_TO_POINTER(stream->streamElement[0].offset);

                    /*incease the numElements of the stream*/
                    stream->numElements++;
                }
                else
                {
                    /*we need another stream for this element*/
                    newStream = GL_TRUE;
                }
            }

            if(newStream)
            {
                /*get a new stream*/
                stream = &gc->vertexStreams.streams[numStreams];

                /*configure the first element in the new stream*/
                element = &stream->streamElement[0];
                element->streamIndex = numStreams;
                element->inputIndex = arrayIdx;
                element->size = array->size;
                element->type = array->type;
                element->normalized = array->normalized;
                element->integer = array->integer;
                element->offset = array->offset;/*for normal vertex arrays, this offset could be seen as an offset from
                                                                NULL pointer*/

                /*configure the new stream*/
                stream->privPtrAddr = (bufObj && bufObj->size > 0) ? &bufObj->privateData: 0;
                stream->numElements = 1;
                stream->stride = array->stride;
                stream->streamAddr = __GL_OFFSET_TO_POINTER(element->offset);

                /* increase the numStream */
                numStreams++;
            }
        }

        arrayIdx++;
        mask >>= 1;
    }

    /*
    **For streams that do not source data from buffer objects, calculate the offsets
    **for all elements. For Streams that source data from buffer objects, calculate
    ** the streamAddr.
    */
    for(streamIdx = 0; streamIdx < numStreams; streamIdx++ )
    {
        stream = &gc->vertexStreams.streams[streamIdx];
        /*for streams in buffer object, offset is already an offset*/
        if(stream->privPtrAddr == NULL)
        {
            GL_ASSERT(stream->streamAddr == __GL_OFFSET_TO_POINTER(stream->streamElement[0].offset));
            for(i = 1; i < stream->numElements; i ++ )
            {
                stream->streamElement[i].offset -= stream->streamElement[0].offset;
            }
            stream->streamElement[0].offset = 0;
        }
        else
        {
            /*Map the buffer object, and set the streamAddr*/
            arrayIdx = stream->streamElement[0].inputIndex;
            bufObj = gc->bufferObject.boundArrays[arrayIdx];
            GL_ASSERT(&bufObj->privateData == stream->privPtrAddr);
            stream->streamAddr = NULL;
        }
    }

    gc->vertexStreams.numStreams = numStreams;
    gc->vertexStreams.streamMode = VERTEXARRAY_STREAMMODE;
}


GLvoid __glConfigDlistVertexStream(__GLcontext *gc, __GLPrimBegin *primBegin, GLfloat *beginAddr,
        GLuint vertexCount, GLuint indexCount, GLushort *indexBuffer, GLvoid **privPtrAddr, GLvoid **ibPrivPtrAddr)
{
    __GLstreamDecl *stream;
    __GLvertexElement *element;
    GLint i, offset;
    GLubyte inputIdx;
    GLuint mask;

    /* Display list consistent primitive always use single stream */
    gc->vertexStreams.numStreams = 1;
    gc->vertexStreams.endVertex = vertexCount;
    gc->vertexStreams.indexCount = indexCount;
    gc->vertexStreams.startVertex = 0;
    gc->vertexStreams.primElemSequence = primBegin->primElemSequence;
    gc->vertexStreams.primElementMask = primBegin->primInputMask;
    gc->vertexStreams.missingAttribs = gc->input.requiredInputMask & (~primBegin->primInputMask) & (~__GL_INPUT_EDGEFLAG) & (~__GL_INPUT_VERTEX);
    gc->vertexStreams.edgeflagStream = primBegin->edgeflagBuffer;
    if (indexCount)
    {   /* Configure the index stream */
        gc->vertexStreams.indexStream.streamAddr = (GLvoid *)indexBuffer;
        gc->vertexStreams.indexStream.type = GL_UNSIGNED_SHORT;
        gc->vertexStreams.indexStream.ppIndexBufPriv = ibPrivPtrAddr;
        gc->vertexStreams.indexStream.offset = 0;
    }

    stream = &gc->vertexStreams.streams[0];
    stream->numElements = primBegin->elementCount;
    stream->streamAddr = (GLuint *)beginAddr;
    stream->stride = (primBegin->totalStrideDW << 2);
    stream->privPtrAddr = privPtrAddr;

    offset = 0;
    for (i = 0; i < (GLint)stream->numElements; i++)
    {
        element = &stream->streamElement[i];
        inputIdx = 0;
        mask = primBegin->primInputMask & (~__GL_INPUT_EDGEFLAG);
        while (mask)
        {
            if ((mask & 0x1) && (primBegin->elemOffsetDW[inputIdx] == offset))
            {
                offset += primBegin->elemSizeDW[inputIdx];
                break;
            }

            mask >>= 1;
            inputIdx += 1;
        }

        element->inputIndex = inputIdx;
        element->streamIndex = 0;
        element->offset = (primBegin->elemOffsetDW[inputIdx] << 2);
        element->size = primBegin->elemSizeDW[inputIdx];
        element->type = GL_FLOAT;
        element->normalized = GL_FALSE;
        element->integer = GL_FALSE;
        if (element->inputIndex == __GL_INPUT_DIFFUSE_INDEX && element->size == 1)
        {
            /* Special case for RGBA ubyte format */
            element->size = 4;
            element->type = GL_UNSIGNED_BYTE;
            element->normalized = GL_TRUE;
        }
    }

    gc->vertexStreams.streamMode = DLIST_STREAMMODE;
}

/***** End of vertex stream config functions ********/
GLenum indexPrimModeDL[] = {
    GL_POINTS,            /* GL_POINTS */
    GL_LINES,            /* GL_LINES */
    GL_LINES,            /* GL_LINE_LOOP */
    GL_LINES,            /* GL_LINE_STRIP */
    GL_TRIANGLES,        /* GL_TRIANGLES */
    GL_TRIANGLES,        /* GL_TRIANGLE_STRIP */
    GL_TRIANGLES,        /* GL_TRIANGLE_FAN */
    GL_QUADS,            /* GL_QUADS */
    GL_TRIANGLES,        /* GL_QUAD_STRIP */
    GL_TRIANGLES,        /* GL_POLYGON */
    GL_LINES_ADJACENCY_EXT,             /* GL_LINES_ADJACENCY_EXT */
    GL_LINE_STRIP_ADJACENCY_EXT,        /* GL_LINE_STRIP_ADJACENCY_EXT */
    GL_TRIANGLES_ADJACENCY_EXT,         /* GL_TRIANGLES_ADJACENCY_EXT */
    GL_TRIANGLE_STRIP_ADJACENCY_EXT,    /* GL_TRIANGLE_STRIP_ADJACENCY_EXT */
};

GLenum indexPrimMode[] = {
    GL_POINTS,                          /* GL_POINTS */
    GL_LINES,                           /* GL_LINES */
    GL_LINE_LOOP,                       /* GL_LINE_LOOP */
    GL_LINE_STRIP,                      /* GL_LINE_STRIP */
    GL_TRIANGLES,                       /* GL_TRIANGLES */
    GL_TRIANGLE_STRIP,                  /* GL_TRIANGLE_STRIP */
    GL_TRIANGLE_FAN,                    /* GL_TRIANGLE_FAN */
    GL_QUADS,                           /* GL_QUADS */
    GL_QUAD_STRIP,                      /* GL_QUAD_STRIP */
    GL_POLYGON,                         /* GL_POLYGON */
    GL_LINES_ADJACENCY_EXT,             /* GL_LINES_ADJACENCY_EXT */
    GL_LINE_STRIP_ADJACENCY_EXT,        /* GL_LINE_STRIP_ADJACENCY_EXT */
    GL_TRIANGLES_ADJACENCY_EXT,         /* GL_TRIANGLES_ADJACENCY_EXT */
    GL_TRIANGLE_STRIP_ADJACENCY_EXT,    /* GL_TRIANGLE_STRIP_ADJACENCY_EXT */
};


/******** Draw functions *****************/
#if __GL_ENABLE_HW_NULL
extern GLuint drawCount;
#endif

#if __GL_TIMEING_DRAW
extern BOOLEAN logTimeEnable;
extern GLuint64 tTick;
extern FILE*  timeingLogFile ;
extern GLbyte *printBuffer ;
extern GLbyte* curOutput, *endSlot;

__GL_INLINE GLuint64 GetCycleCount()
{
__asm _emit 0x0F
__asm _emit 0x31
}

#define ENTERFUNC_TM()\
    if(logTimeEnable) tTick = GetCycleCount();

#define LEAVEFUNC_TM()\
    if(logTimeEnable) \
    {           \
        tTick = GetCycleCount() - tTick;   \
        curOutput += sprintf(curOutput, "%d\t", tTick);  \
        if(curOutput > endSlot ){           \
               if(timeingLogFile)                 \
               {                                              \
                    sprintf(curOutput, "\0");           \
                    fprintf(timeingLogFile, printBuffer);           \
                    curOutput = printBuffer;                            \
               }                                                      \
         }                                     \
    }

#else
#define    ENTERFUNC_TM()
#define    LEAVEFUNC_TM()
#endif

__GL_INLINE GLvoid __glDrawPrimitive(__GLcontext *gc)
{
#if __GL_ENABLE_HW_NULL
    if( hwNULL && skipDrawPrimitive )
        return;

    drawCount++;
#endif

#if (defined(DEBUG) || defined(_DEBUG))
    gdbg_drawCount ++;
    gc->drawablePrivate->drawCount++;
#endif

    /* Check consistency between primMode & gs input type
    */

    if(gc->shaderProgram.geomShaderEnable)
    {
        GL_ASSERT(gc->shaderProgram.currentShaderProgram);
        switch(gc->shaderProgram.currentShaderProgram->programInfo.geomInputType)
        {
        case GL_POINTS:
            if(!(gc->vertexStreams.primMode == GL_POINTS))
            {
                __glSetError(GL_INVALID_OPERATION);
                return;
            }
            break;
        case GL_LINES:
            if(!((gc->vertexStreams.primMode == GL_LINES) ||
                (gc->vertexStreams.primMode == GL_LINE_STRIP) ||
                (gc->vertexStreams.primMode == GL_LINE_LOOP)) )
            {
                __glSetError(GL_INVALID_OPERATION);
                return;
            }
            break;
        case GL_LINES_ADJACENCY_EXT:
            if(!((gc->vertexStreams.primMode == GL_LINES_ADJACENCY_EXT) ||
                (gc->vertexStreams.primMode == GL_LINE_STRIP_ADJACENCY_EXT)) )
            {
                __glSetError(GL_INVALID_OPERATION);
                return;
            }
            break;
        case GL_TRIANGLES:
            if(!((gc->vertexStreams.primMode == GL_TRIANGLES) ||
                (gc->vertexStreams.primMode == GL_TRIANGLE_STRIP) ||
                (gc->vertexStreams.primMode == GL_TRIANGLE_FAN)) )
            {
                __glSetError(GL_INVALID_OPERATION);
                return;
            }
            break;
        case GL_TRIANGLES_ADJACENCY_EXT:
            if(!((gc->vertexStreams.primMode == GL_TRIANGLES_ADJACENCY_EXT) ||
                (gc->vertexStreams.primMode == GL_TRIANGLE_STRIP_ADJACENCY_EXT)) )
            {
                __glSetError(GL_INVALID_OPERATION);
                return;
            }
            break;
        default:
            GL_ASSERT(0);
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
    }

    if ((gc->flags & __GL_DISCARD_DRAWS_MASK) == 0)
    {
        /* DP can set gc->pipeline to SW pipeline if it can't render the primitive.
        */
        (*gc->dp.begin)(gc, gc->vertexStreams.primMode);

        /* Draw primitive through either device pipeline or SW pipeline.
        */
        if (!gc->vertexArray.immedFallback && gc->pipeline->drawPrimitive)
            (*gc->pipeline->drawPrimitive)(gc);

        /* DP can perform clean-up work here.
        */
        (*gc->dp.end)(gc);
    }
    else
    {
        gc->flags &= ~(__GL_DISCARD_ONE_DRAW_MASK);
    }
}

/* Vertex array mode draw function */
GLvoid  __glDrawArrayPrimitive(__GLcontext *gc, GLenum mode)
{
    ENTERFUNC_TM()

#if (defined(DEBUG) || defined(_DEBUG))
    gdbg_drawArrayCount ++;
#endif

    if (mode != gc->vertexStreams.primMode)
    {
        gc->vertexStreams.primMode = mode;
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_PRIMMODE_BIT);
    }

    /* Get the latest drawable information */
    LINUX_LOCK_FRAMEBUFFER(gc);

    /* Check all GL attributes for redundancy before notify device pipeline.
    */
    __glEvaluateAttribDrawableChange(gc);

    /* Config __GLvertexStreamMachine */
    __glConfigArrayVertexStream(gc, mode);

    if(gc->vertexArray.immedFallback == GL_TRUE)
    {
        LINUX_UNLOCK_FRAMEBUFFER(gc);

        LEAVEFUNC_TM();
        return;
    }

    gc->dp.beginIndex = __GL_DP_GENERIC_PATH;

    __glDrawPrimitive(gc);

    LINUX_UNLOCK_FRAMEBUFFER(gc);

    LEAVEFUNC_TM();
}

/* Immediate mode draw function */
GLvoid  __glDrawImmedPrimitive(__GLcontext *gc)
{
    GLenum mode;

    ENTERFUNC_TM();

#if (defined(DEBUG) || defined(_DEBUG))
    gdbg_drawImmeCount ++;
#endif

    mode = (gc->input.indexCount ? indexPrimMode[gc->input.primMode] : gc->input.primMode);

    if (mode != gc->vertexStreams.primMode)
    {
        gc->vertexStreams.primMode = mode;
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_PRIMMODE_BIT);
    }

    /* Get the latest drawable information */
    LINUX_LOCK_FRAMEBUFFER(gc);

    __glEvaluateAttribDrawableChange(gc);

    __glConfigImmedVertexStream(gc, mode);

    __glDrawPrimitive(gc);

    LINUX_UNLOCK_FRAMEBUFFER(gc);
    LEAVEFUNC_TM();
}

GLvoid __glDrawDlistPrimitive(__GLcontext *gc, __GLPrimBegin *primBegin)
{
    GLfloat *beginAddr;
    GLint vertexCount, indexCount, i;
    GLboolean bothFaceFill, indexedPrim;
    GLenum mode;

    ENTERFUNC_TM();

#if (defined(DEBUG) || defined(_DEBUG))
    gdbg_drawDlistCount ++;
#endif

    /* Compute the required attribute mask */
    if (gc->input.inputMaskChanged)
    {
        __glComputeRequiredInputMask(gc);
        gc->input.inputMaskChanged = GL_FALSE;
    }
    gc->input.requiredInputMask = gc->input.currentInputMask & edgeFlagInputMask[primBegin->primType];

    bothFaceFill = (primBegin->primType >= GL_TRIANGLES && gc->state.polygon.bothFaceFill);
    indexedPrim = ((bothFaceFill || primBegin->primType <= GL_LINE_STRIP) && primBegin->indexCount > 0);

    mode = (indexedPrim) ? indexPrimModeDL[primBegin->primType] : primBegin->primType;
    if (mode != gc->vertexStreams.primMode)
    {
        gc->vertexStreams.primMode = mode;
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_PRIMMODE_BIT);
    }

    /* Get the latest drawable information */
    LINUX_LOCK_FRAMEBUFFER(gc);

    __glEvaluateAttribDrawableChange(gc);

    if (indexedPrim || primBegin->primCount == 1)
    {
        indexCount = (indexedPrim || bothFaceFill) ? primBegin->indexCount : 0;

        beginAddr = (GLfloat *)(primBegin + 1);
         __glConfigDlistVertexStream(gc, primBegin, beginAddr, primBegin->vertexCount,
                indexCount, primBegin->indexBuffer, &primBegin->privateData, &primBegin->ibPrivateData);

        /* Draw indexed primitive */
        __glDrawPrimitive(gc);
    }
    else
    {
        for (i = 0; i < primBegin->primCount; i++) {
            beginAddr = primBegin->primStartAddr[i];
            vertexCount = primBegin->primVertCount[i];
            __glConfigDlistVertexStream(gc, primBegin, beginAddr, vertexCount, 0, NULL, NULL, NULL);

            /* Draw individual primtives */
            gc->dp.beginIndex = __GL_DP_GENERIC_PATH;
            __glDrawPrimitive(gc);
        }
    }

    LINUX_UNLOCK_FRAMEBUFFER(gc);

    LEAVEFUNC_TM();
}

/***** End of draw functions *****************/


