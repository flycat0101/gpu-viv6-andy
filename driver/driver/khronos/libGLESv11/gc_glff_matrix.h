/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_glff_matrix_h_
#define __gc_glff_matrix_h_

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************\
***************************** Matric stack depths. *****************************
\******************************************************************************/

#define glvMAX_STACK_NUM_MODELVIEW          32
#define glvMAX_STACK_NUM_PROJECTION         2
#define glvMAX_STACK_NUM_PALETTE            2
#define glvMAX_STACK_NUM_TEXTURES           2

/* Supported matrix modes. */
typedef enum _gleMATRIXMODE
{
    glvMODEL_VIEW_MATRIX,
    glvPROJECTION_MATRIX,
    glvPALETTE_MATRIX_0,
    glvPALETTE_MATRIX_LAST = glvPALETTE_MATRIX_0 + glvMAX_PALETTE_MATRICES - 1,
    glvTEXTURE_MATRIX_0,
    glvTEXTURE_MATRIX_LAST = glvTEXTURE_MATRIX_0 + glvMAX_TEXTURES - 1,
    glvSTACKCOUNT
}
gleMATRIXMODE, * gleMATRIXMODE_PTR;


/******************************************************************************\
*************************** Matric stack structures. ***************************
\******************************************************************************/

typedef void (* glfMATRIXCHANGEEVENT) (
    glsCONTEXT_PTR Context,
    GLuint MatrixID
    );

typedef struct _glsMATRIXSTACK * glsMATRIXSTACK_PTR;
typedef struct _glsMATRIXSTACK
{
    GLuint                  count;
    GLuint                  index;
    glsMATRIX_PTR           stack;
    glsMATRIX_PTR           topMatrix;
    glfMATRIXCHANGEEVENT    currChanged;
    glfMATRIXCHANGEEVENT    dataChanged;
    GLuint                  matrixID;
}
glsMATRIXSTACK;

typedef struct _glsDEPENDENTMATRIX * glsDEPENDENTMATRIX_PTR;
typedef struct _glsDEPENDENTMATRIX
{
    glsMATRIX               matrix;
    GLboolean               recompute;
    GLboolean               dirty;
}
glsDEPENDENTMATRIX;


/******************************************************************************\
******************************* Matrix Stack API *******************************
\******************************************************************************/

gceSTATUS glfInitializeMatrixStack(
    glsCONTEXT_PTR Context
    );

gceSTATUS glfFlushMatrixStack(
    glsCONTEXT_PTR Context
    );

gceSTATUS glfFreeMatrixStack(
    glsCONTEXT_PTR Context
    );

GLboolean glfSetMatrixStackIndex(
    glsCONTEXT_PTR Context,
    glsMATRIXSTACK_PTR MatrixStack,
    GLuint Index
    );

void glfUpdateMatrixStates(
    glsCONTEXT_PTR Context
    );


/******************************************************************************\
******************************** Matrix Access *********************************
\******************************************************************************/

void glfConvertToVivanteMatrix(
    glsCONTEXT_PTR Context,
    const glsMATRIX_PTR Matrix,
    glsMATRIX_PTR Result
    );

GLenum glfSetMatrixMode(
    glsCONTEXT_PTR Context,
    GLenum MatrixMode
    );

glsMATRIX_PTR glfGetModelViewInverse3x3TransposedMatrix(
    glsCONTEXT_PTR Context
    );

glsMATRIX_PTR glfGetModelViewInverse4x4TransposedMatrix(
    glsCONTEXT_PTR Context
    );

glsMATRIX_PTR glfGetModelViewProjectionMatrix(
    glsCONTEXT_PTR Context
    );

glsMATRIX_PTR glfGetConvertedProjectionMatrix(
    glsCONTEXT_PTR Context
    );

glsDEPENDENTMATRIX_PTR glfGetMatrixPaletteInverse(
    glsCONTEXT_PTR Context
    );


/******************************************************************************\
****************************** Matrix State Query ******************************
\******************************************************************************/

GLboolean glfQueryMatrixState(
    glsCONTEXT_PTR Context,
    GLenum Name,
    GLvoid* Value,
    gleTYPE Type
    );


/******************************************************************************\
******************************* Matrix Operations ******************************
\******************************************************************************/

void glfMultiplyVector3ByMatrix3x3(
    const glsVECTOR_PTR Vector,
    const glsMATRIX_PTR Matrix,
    glsVECTOR_PTR Result
    );

void glfMultiplyVector4ByMatrix4x4(
    const glsVECTOR_PTR Vector,
    const glsMATRIX_PTR Matrix,
    glsVECTOR_PTR Result
    );

GLfixed glfSinX(
     GLfixed Angle
     );

GLfixed glfCosX(
     GLfixed Angle
     );

GLfixed glfRSQX(
    GLfixed X
    );

#ifdef __cplusplus
}
#endif

#endif /* __gc_glff_matrix_h_ */
