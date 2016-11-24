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


#ifdef OPENGL40
#ifndef __gc_gl_lighting_h_
#define __gc_gl_lighting_h_

typedef struct __GLmaterialStateRec {
    __GLcolor ambient;                  /* unscaled */
    __GLcolor diffuse;                  /* unscaled */
    __GLcolor specular;                 /* unscaled */
    __GLcolor emissive;                 /* scaled */
    GLfloat   specularExponent;
    GLfloat   cmapa, cmapd, cmaps;
} __GLmaterialState;

typedef struct __GLlightModelStateRec {
    __GLcolor ambient;                  /* scaled */
    GLuint    localViewer;
    GLuint    twoSided;
    GLenum    colorControl;     /* SINGLE_COLOR or SEPARATE_SPECULAR_COLOR */
} __GLlightModelState;

typedef struct __GLlightSourceStateRec {
    __GLcolor ambient;                  /* scaled */
    __GLcolor diffuse;                  /* scaled */
    __GLcolor specular;                 /* scaled */
    __GLcoord position;
    __GLcoord positionEye;
    __GLcoord direction;
    GLfloat   spotLightExponent;
    GLfloat   spotLightCutOffAngle;
    GLfloat   constantAttenuation;
    GLfloat   linearAttenuation;
    GLfloat   quadraticAttenuation;
} __GLlightSourceState;

typedef struct {
    GLenum colorMaterialFace;
    GLenum colorMaterialParam;
    GLenum indexMaterialFace;
    GLenum indexMaterialParam;
    GLenum shadingModel;
    GLenum clampVertexColor;
    __GLlightModelState model;
    __GLmaterialState front;
    __GLmaterialState back;
    __GLlightSourceState source[__GL_MAX_LIGHT_NUMBER];
} __GLlightState;


extern GLvoid __glUpdateMaterialfv(__GLcontext *gc, GLenum face, GLenum pname, GLfloat *pv);

#endif /* __gc_gl_lighting_h_ */
#endif
