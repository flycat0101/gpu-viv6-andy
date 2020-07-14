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



#define _GC_OBJ_ZONE gcdZONE_ES11_MATRIX



/******************************************************************************\

*********************** Support Functions and Definitions **********************

\******************************************************************************/

/*******************************************************************************

**

**  glfUpdateMatrixStates

**

**  Update matrix states.

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



void glfUpdateMatrixStates(

    glsCONTEXT_PTR Context

    )

{

    gcmHEADER_ARG("Context=0x%x", Context);

    /* Make sure everything is computed and up to date. */

    glfGetModelViewInverse3x3TransposedMatrix(Context);

    glfGetModelViewProjectionMatrix(Context);



    gcmFOOTER_NO();

}





/*******************************************************************************

**

**  _UpdateMatrixFlags

**

**  Updates special value flags of the specified matrix.

**

**  INPUT:

**

**      Matrix

**          Matrix to be updated.

**

**  OUTPUT:

**

**      Nothing.

*/

static gctBOOL _UpdateMatrixFlags(

    glsMATRIX_PTR Matrix

    )

{

    GLint x, y;



    gcmHEADER_ARG("Matrix=0x%x", Matrix);



    Matrix->identity = GL_TRUE;



    for (y = 0; y < 4; y++)

    {

        for (x = 0; x < 4; x++)

        {

            if (x == y)

            {

                if (glmMATFLOAT(Matrix, x, y) != 1.0f)

                {

                    Matrix->identity = GL_FALSE;

                    gcmFOOTER_NO();

                    return gcvTRUE;

                }

            }

            else

            {

                if (glmMATFLOAT(Matrix, x, y) != 0.0f)

                {

                    Matrix->identity = GL_FALSE;

                    gcmFOOTER_NO();

                    return gcvTRUE;

                }

            }

        }

    }



    gcmFOOTER_NO();

    return gcvTRUE;

}





/*******************************************************************************

**

**  _LoadIdentityMatrix

**

**  Initializes the matrix to "identity".

**

**  INPUT:

**

**      Matrix

**          Pointer to the matrix to be reset.

**

**      Type

**          Type of the matrix to be produced.

**

**  OUTPUT:

**

**      Matrix

**          Identity matrix.

*/

static void _LoadIdentityMatrix(

    glsMATRIX_PTR Matrix

    )

{

    GLint x, y;



    gcmHEADER_ARG("Matrix=0x%x", Matrix);



    for (y = 0; y < 4; y++)

    {

        for (x = 0; x < 4; x++)

        {

            glmMATFLOAT(Matrix, x, y) = (x == y)

                ? 1.0f

                : 0.0f;

        }

    }



    Matrix->identity = GL_TRUE;



    gcmFOOTER_NO();

}





/*******************************************************************************

**

**  glfConvertToVivanteMatrix

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

void glfConvertToVivanteMatrix(

    glsCONTEXT_PTR Context,

    const glsMATRIX_PTR Matrix,

    glsMATRIX_PTR Result

    )

{

    gcmHEADER();



    *Result = *Matrix;



    if (Context->chipModel >= gcv1000 || Context->chipModel == gcv880 || (Context->chipModel == gcv900 && Context->chipRevision == 0x5250))

    {

        gcmFOOTER_NO();

        return;

    }



    Result->identity = GL_FALSE;



    {

        GLfloat z0 = glmMATFLOAT(Result, 0, 2) + glmMATFLOAT(Result, 0, 3);

        GLfloat z1 = glmMATFLOAT(Result, 1, 2) + glmMATFLOAT(Result, 1, 3);

        GLfloat z2 = glmMATFLOAT(Result, 2, 2) + glmMATFLOAT(Result, 2, 3);

        GLfloat z3 = glmMATFLOAT(Result, 3, 2) + glmMATFLOAT(Result, 3, 3);



        glmMATFLOAT(Result, 0, 2) = z0 * 0.5f;

        glmMATFLOAT(Result, 1, 2) = z1 * 0.5f;

        glmMATFLOAT(Result, 2, 2) = z2 * 0.5f;

        glmMATFLOAT(Result, 3, 2) = z3 * 0.5f;

    }



    gcmFOOTER_NO();

}





/*******************************************************************************

**

**  _MultiplyMatrix4x4

**

**  Multiplies two 4x4 matrices.

**

**  INPUT:

**

**      Matrix1

**          Points to the first matrix.

**

**      Matrix2

**          Points to the second matrix.

**

**  OUTPUT:

**

**      Result

**          Product of multiplication.

*/

static gctBOOL _MultiplyMatrix4x4(

    const glsMATRIX_PTR Matrix1,

    const glsMATRIX_PTR Matrix2,

    glsMATRIX_PTR Result

    )

{

    gcmHEADER_ARG("Matrix1=0x%x Matrix2=0x%x Result=0x%x", Matrix1, Matrix2, Result);



    if (Matrix1->identity)

    {

        *Result = *Matrix2;

    }

    else if (Matrix2->identity)

    {

        *Result = *Matrix1;

    }

    else

    {

        GLint x, y;



        Result->identity = GL_FALSE;



        for (y = 0; y < 4; y++)

        {

            for (x = 0; x < 4; x++)

            {

                glmMATFLOAT(Result, x, y) =

                    (glmMATFLOAT(Matrix1, 0, y)

                     * glmMATFLOAT(Matrix2, x, 0))

                  + (glmMATFLOAT(Matrix1, 1, y)

                     * glmMATFLOAT(Matrix2, x, 1))

                  + (glmMATFLOAT(Matrix1, 2, y)

                     * glmMATFLOAT(Matrix2, x, 2))

                  +    (glmMATFLOAT(Matrix1, 3, y)

                     * glmMATFLOAT(Matrix2, x, 3));

            }

        }

    }

    gcmFOOTER_NO();

    return gcvTRUE;

}





/*******************************************************************************

**

**  _TransposeMatrix

**

**  Transposed matrix is the original matrix flipped along its main diagonal.

**

**  INPUT:

**

**      Matrix

**          Points to the matrix to be transposed.

**

**  OUTPUT:

**

**      Result

**          Transposed matrix.

*/

static void _TransposeMatrix(

    const glsMATRIX_PTR Matrix,

    glsMATRIX_PTR Result

    )

{

    GLint y, x;



    gcmHEADER_ARG("Matrix=0x%x Result=0x%x", Matrix, Result);



    for (y = 0; y < 4; y++)

    {

        for (x = 0; x < 4; x++)

        {

            glmMAT(Result, x, y) = glmMAT(Matrix, y, x);

        }

    }



    Result->identity = Matrix->identity;



    gcmFOOTER_NO();

}



static GLboolean _InverseFloatMatrix3x3(

    GLfloat Row0[8],

    GLfloat Row1[8],

    GLfloat Row2[8],

    glsMATRIX_PTR Result

    )

{

    GLint i;

    gcmHEADER_ARG("Row0=0x%x Row1=0x%x Row2=0x%x Result=0x%x",

                    Row0, Row1, Row2, Result);



    /***************************************************************************

    ** Transform the first column to (x, 0, 0).

    */

    /* Make sure [0][0] is not a zero. */

    if (glmABS(Row2[0]) > glmABS(Row1[0])) glmSWAP(GLfloat*, Row2, Row1);

    if (glmABS(Row1[0]) > glmABS(Row0[0])) glmSWAP(GLfloat*, Row1, Row0);

    if (Row0[0] == 0.0f)

    {

        gcmFOOTER_ARG("%d", GL_FALSE);

        return GL_FALSE;

    }



    for (i = 1; i < 6; i++)

    {

        if (Row0[i] != 0.0f)

        {

            if (Row1[0] != 0.0f)

            {

                Row1[i] -= Row0[i] * Row1[0] / Row0[0];

            }



            if (Row2[0] != 0.0f)

            {

                Row2[i] -= Row0[i] * Row2[0] / Row0[0];

            }

        }

    }





    /***************************************************************************

    ** Transform the second column to (0, x, 0).

    */

    /* Make sure [1][1] is not a zero. */

    if (glmABS(Row2[1]) > glmABS(Row1[1])) glmSWAP(GLfloat*, Row2, Row1);

    if (Row1[1] == 0.0f)

    {

        gcmFOOTER_ARG("%d", GL_FALSE);

        return GL_FALSE;

    }



    for (i = 2; i < 6; i++)

    {

        if (Row1[i] != 0.0f)

        {

            if (Row0[1] != 0.0f)

            {

                Row0[i] -= Row1[i] * Row0[1] / Row1[1];

            }



            if (Row2[1] != 0.0f)

            {

                Row2[i] -= Row1[i] * Row2[1] / Row1[1];

            }

        }

    }





    /***************************************************************************

    ** Transform the third column to (0, 0, x).

    */

    /* Make sure [2][2] is not a zero. */

    if (Row2[2] == 0.0f)

    {

        gcmFOOTER_ARG("%d", GL_FALSE);

        return GL_FALSE;

    }



    for (i = 3; i < 6; i++)

    {

        if (Row2[i] != 0.0f)

        {

            if (Row0[2] != 0.0f)

            {

                Row0[i] -= Row2[i] * Row0[2] / Row2[2];

            }



            if (Row1[2] != 0.0f)

            {

                Row1[i] -= Row2[i] * Row1[2] / Row2[2];

            }

        }

    }





    /***************************************************************************

    ** Normalize the result.

    */

    for (i = 0; i < 3; i++)

    {

        glmMATFLOAT(Result, i, 0) = Row0[i + 3] / Row0[0];

        glmMATFLOAT(Result, i, 1) = Row1[i + 3] / Row1[1];

        glmMATFLOAT(Result, i, 2) = Row2[i + 3] / Row2[2];

    }



    gcmFOOTER_ARG("%d", GL_TRUE);

    /* Success. */

    return GL_TRUE;

}





/*******************************************************************************

**

**  _InverseMatrix3x3

**

**  Get an inversed 3x3 matrix using Gaussian-Jordan Elimination.

**

**  INPUT:

**

**      Matrix

**          Points to the matrix to be inversed.

**

**  OUTPUT:

**

**      Result

**          Result of the inversed matrix.

*/

static GLboolean _InverseMatrix3x3(

    const glsMATRIX_PTR Matrix,

    glsMATRIX_PTR Result

    )

{

    GLboolean result;

    gcmHEADER_ARG("Matrix=0x%x Result=0x%x", Matrix, Result);



    if (Matrix->identity)

    {

        *Result = *Matrix;

        result = GL_TRUE;

    }

    else

    {

        GLfloat row0[6];

        GLfloat row1[6];

        GLfloat row2[6];



        GLint i;



        /* Initialize the augmented matrix. */

        for (i = 0; i < 3; i++)

        {

            row0[i] = glmMAT(Matrix, i, 0);

            row1[i] = glmMAT(Matrix, i, 1);

            row2[i] = glmMAT(Matrix, i, 2);



            row0[i + 3] = glvFLOATZERO;

            row1[i + 3] = glvFLOATZERO;

            row2[i + 3] = glvFLOATZERO;

        }



        row0[3] = glvFLOATONE;

        row1[4] = glvFLOATONE;

        row2[5] = glvFLOATONE;



        result = _InverseFloatMatrix3x3(row0, row1, row2, Result);



        if (result)

        {

            Result->identity = GL_FALSE;



            for (i = 0; i < 4; i++)

            {

                glmMAT(Result, i, 3) = glvFLOATZERO;

                glmMAT(Result, 3, i) = glvFLOATZERO;

            }

        }

    }



    gcmFOOTER_ARG("%d", result);

    return result;

}



static GLboolean _InverseFloatMatrix4x4(

    GLfloat Row0[8],

    GLfloat Row1[8],

    GLfloat Row2[8],

    GLfloat Row3[8],

    glsMATRIX_PTR Result

    )

{

    GLint i;

    gcmHEADER_ARG("Row0=0x%x Row1=0x%x Row2=0x%x Row3=0x%x Result=0x%x",

                    Row0, Row1, Row2, Row3, Result);



    /***************************************************************************

    ** Transform the first column to (x, 0, 0, 0).

    */

    /* Make sure [0][0] is not a zero. */

    if (glmABS(Row3[0]) > glmABS(Row2[0])) glmSWAP(GLfloat *, Row3, Row2);

    if (glmABS(Row2[0]) > glmABS(Row1[0])) glmSWAP(GLfloat *, Row2, Row1);

    if (glmABS(Row1[0]) > glmABS(Row0[0])) glmSWAP(GLfloat *, Row1, Row0);

    if (Row0[0] == glvFLOATZERO)

    {

        gcmFOOTER_ARG("%d", GL_FALSE);

        return GL_FALSE;

    }



    for (i = 1; i < 8; i++)

    {

        if (Row0[i] != 0.0f)

        {

            if (Row1[0] != 0.0f)

            {

                Row1[i] -= Row0[i] * Row1[0] / Row0[0];

            }



            if (Row2[0] != 0.0f)

            {

                Row2[i] -= Row0[i] * Row2[0] / Row0[0];

            }



            if (Row3[0] != 0.0f)

            {

                Row3[i] -= Row0[i] * Row3[0] / Row0[0];

            }

        }

    }





    /***************************************************************************

    ** Transform the second column to (0, x, 0, 0).

    */

    /* Make sure [1][1] is not a zero. */

    if (glmABS(Row3[1]) > glmABS(Row2[1])) glmSWAP(GLfloat*, Row3, Row2);

    if (glmABS(Row2[1]) > glmABS(Row1[1])) glmSWAP(GLfloat*, Row2, Row1);

    if (Row1[1] == 0.0f)

    {

        gcmFOOTER_ARG("%d", GL_FALSE);

        return GL_FALSE;

    }



    for (i = 2; i < 8; i++)

    {

        if (Row1[i] != 0.0f)

        {

            if (Row0[1] != 0.0f)

            {

                Row0[i] -= Row1[i] * Row0[1] / Row1[1];

            }



            if (Row2[1] != 0.0f)

            {

                Row2[i] -= Row1[i] * Row2[1] / Row1[1];

            }



            if (Row3[1] != 0.0f)

            {

                Row3[i] -= Row1[i] * Row3[1] / Row1[1];

            }

        }

    }





    /***************************************************************************

    ** Transform the third column to (0, 0, x, 0).

    */

    /* Make sure [2][2] is not a zero. */

    if (glmABS(Row3[2]) > glmABS(Row2[2])) glmSWAP(GLfloat*, Row3, Row2);

    if (Row2[2] == 0.0f)

    {

        gcmFOOTER_ARG("%d", GL_FALSE);

        return GL_FALSE;

    }



    for (i = 3; i < 8; i++)

    {

        if (Row2[i] != 0.0f)

        {

            if (Row0[2] != 0.0f)

            {

                Row0[i] -= Row2[i] * Row0[2] / Row2[2];

            }



            if (Row1[2] != 0.0f)

            {

                Row1[i] -= Row2[i] * Row1[2] / Row2[2];

            }



            if (Row3[2] != 0.0f)

            {

                Row3[i] -= Row2[i] * Row3[2] / Row2[2];

            }

        }

    }





    /***************************************************************************

    ** Transform the fourth column to (0, 0, 0, x).

    */

    if (Row3[3] == 0.0f)

    {

        gcmFOOTER_ARG("%d", GL_FALSE);

        return GL_FALSE;

    }



    for (i = 4; i < 8; i++)

    {

        if (Row3[i] != 0.0f)

        {

            if (Row0[3] != 0.0f)

            {

                Row0[i] -= Row3[i] * Row0[3] / Row3[3];

            }



            if (Row1[3] != 0.0f)

            {

                Row1[i] -= Row3[i] * Row1[3] / Row3[3];

            }



            if (Row2[3] != 0.0f)

            {

                Row2[i] -= Row3[i] * Row2[3] / Row3[3];

            }

        }

    }





    /***************************************************************************

    ** Normalize the result.

    */

    for (i = 0; i < 4; i++)

    {

        glmMATFLOAT(Result, i, 0) = Row0[i + 4] / Row0[0];

        glmMATFLOAT(Result, i, 1) = Row1[i + 4] / Row1[1];

        glmMATFLOAT(Result, i, 2) = Row2[i + 4] / Row2[2];

        glmMATFLOAT(Result, i, 3) = Row3[i + 4] / Row3[3];

    }



    gcmFOOTER_ARG("%d", GL_TRUE);

    /* Success. */

    return GL_TRUE;

}





/*******************************************************************************

**

**  _InverseMatrix4x4

**

**  Get an inversed 4x4 matrix using Gaussian-Jordan Elimination.

**

**  INPUT:

**

**      Matrix

**          Points to the matrix to be inversed.

**

**  OUTPUT:

**

**      Result

**          Result of the inversed matrix.

*/

static GLboolean _InverseMatrix4x4(

    const glsMATRIX_PTR Matrix,

    glsMATRIX_PTR Result

    )

{

    GLboolean result;



    GLfloat row0[8];

    GLfloat row1[8];

    GLfloat row2[8];

    GLfloat row3[8];



    GLint i;



    gcmHEADER_ARG("Matrix=0x%x Result=0x%x", Matrix, Result);



    /* Initialize the augmented matrix. */

    for (i = 0; i < 4; i++)

    {

        row0[i] = glmMAT(Matrix, i, 0);

        row1[i] = glmMAT(Matrix, i, 1);

        row2[i] = glmMAT(Matrix, i, 2);

        row3[i] = glmMAT(Matrix, i, 3);



        row0[i + 4] = glvFLOATZERO;

        row1[i + 4] = glvFLOATZERO;

        row2[i + 4] = glvFLOATZERO;

        row3[i + 4] = glvFLOATZERO;

    }



    row0[4] = glvFLOATONE;

    row1[5] = glvFLOATONE;

    row2[6] = glvFLOATONE;

    row3[7] = glvFLOATONE;



    result = _InverseFloatMatrix4x4(row0, row1, row2, row3, Result);



    Result->identity = GL_FALSE;



    gcmFOOTER_ARG("%d", result);

    return result;

}





/*******************************************************************************

**

**  _LoadMatrix

**

**  Loads matrix from an array of raw values:

**

**      m[0]        m[4]        m[8]        m[12]

**      m[1]        m[5]        m[9]        m[13]

**      m[2]        m[6]        m[10]       m[14]

**      m[3]        m[7]        m[11]       m[15]

**

**  INPUT:

**

**      Matrix

**          Pointer to the matrix to be reset.

**

**      Type

**          Type of the matrix to be produced.

**

**      Values

**          Points to the array of values.

**

**  OUTPUT:

**

**      Matrix

**          Identity matrix.

*/

static gctBOOL _LoadMatrix(

    glsMATRIX_PTR Matrix,

    const GLfloat* Values

    )

{

    gctBOOL result;



    gcmHEADER_ARG("Matrix=0x%x Values=0x%x", Matrix, Values);



    gcoOS_MemCopy(&Matrix->value, Values, 4 * 4 * gcmSIZEOF(GLfloat));



    /* Update matrix flags. */

    result = _UpdateMatrixFlags(Matrix);

    gcmFOOTER_NO();

    return result;

}





/******************************************************************************\

******************** Model View matrix modification handlers *******************

\******************************************************************************/

/* Current model view matrix has changed, update pointers. */

static void _ModelViewMatrixCurrentChanged(

    glsCONTEXT_PTR Context,

    GLuint MatrixID

    )

{

    gcmHEADER_ARG("Context=0x%x", Context);

    Context->modelViewMatrix

        = Context->matrixStackArray[glvMODEL_VIEW_MATRIX].topMatrix;



    /* Set uModelView, uModeViewInversed uModelViewProjection dirty. */

    Context->vsUniformDirty.uModelViewDirty                     = gcvTRUE;

    Context->vsUniformDirty.uModelViewInverse3x3TransposedDirty = gcvTRUE;

    Context->vsUniformDirty.uModeViewProjectionDirty            = gcvTRUE;



    gcmFOOTER_NO();

}



/* Value of the current model view matrix has changed, invalidate dependets. */

static void _ModelViewMatrixDataChanged(

    glsCONTEXT_PTR Context,

    GLuint MatrixID

    )

{

    gcmHEADER_ARG("Context=0x%x", Context);



    Context->hashKey.hashModelViewIdentity =

        Context->modelViewMatrix->identity;



    Context->modelViewInverse3x3TransposedMatrix.recompute = GL_TRUE;

    Context->modelViewInverse3x3TransposedMatrix.dirty     = GL_TRUE;

    Context->modelViewInverse4x4TransposedMatrix.recompute = GL_TRUE;

    Context->modelViewProjectionMatrix.recompute = GL_TRUE;

    Context->modelViewProjectionMatrix.dirty     = GL_TRUE;



    /* Set uModelView, uModeViewInversed uModelViewProjection dirty. */

    Context->vsUniformDirty.uModelViewDirty                     = gcvTRUE;

    Context->vsUniformDirty.uModelViewInverse3x3TransposedDirty = gcvTRUE;

    Context->vsUniformDirty.uModeViewProjectionDirty            = gcvTRUE;



    gcmFOOTER_NO();

}





/******************************************************************************\

******************** Projection matrix modification handlers *******************

\******************************************************************************/

/* Current projection matrix has changed, update pointers. */

static void _ProjectionMatrixCurrentChanged(

    glsCONTEXT_PTR Context,

    GLuint MatrixID

    )

{

    gcmHEADER_ARG("Context=0x%x", Context);

    Context->projectionMatrix

        = Context->matrixStackArray[glvPROJECTION_MATRIX].topMatrix;



    /* Set uProjection and uModelViewProjection dirty. */

    Context->vsUniformDirty.uProjectionDirty         = gcvTRUE;

    Context->vsUniformDirty.uModeViewProjectionDirty = gcvTRUE;



    gcmFOOTER_NO();

}



/* Value of the current projection matrix has changed, invalidate dependets. */

static void _ProjectionMatrixDataChanged(

    glsCONTEXT_PTR Context,

    GLuint MatrixID

    )

{

    gcmHEADER_ARG("Context=0x%x", Context);

    Context->hashKey.hashProjectionIdentity =

        Context->projectionMatrix->identity;



    Context->convertedProjectionMatrix.recompute = GL_TRUE;

    Context->modelViewProjectionMatrix.recompute = GL_TRUE;

    Context->modelViewProjectionMatrix.dirty     = GL_TRUE;



    /* Set uProjection and uModelViewProjection dirty. */

    Context->vsUniformDirty.uProjectionDirty         = gcvTRUE;

    Context->vsUniformDirty.uModeViewProjectionDirty = gcvTRUE;



    gcmFOOTER_NO();

}





/******************************************************************************\

********************* Palette matrix modification handlers *********************

\******************************************************************************/

/* Current palette matrix has changed, update pointers. */

static void _PaletteMatrixCurrentChanged(

    glsCONTEXT_PTR Context,

    GLuint MatrixID

    )

{

    gcmHEADER_ARG("Context=0x%x", Context);



    /* Set uMatrixPalette dirty. */

    Context->vsUniformDirty.uMatrixPaletteDirty        = gcvTRUE;

    Context->vsUniformDirty.uMatrixPaletteInverseDirty = gcvTRUE;



    gcmFOOTER_NO();

}



/* Value of the current palette matrix has changed, invalidate dependets. */

static void _PaletteMatrixDataChanged(

    glsCONTEXT_PTR Context,

    GLuint MatrixID

    )

{

    gcmHEADER_ARG("Context=0x%x", Context);

    gcmASSERT(MatrixID < glvMAX_PALETTE_MATRICES);



    Context->paletteMatrixInverse3x3[MatrixID].recompute = GL_TRUE;

    Context->paletteMatrixInverse3x3Recompute = GL_TRUE;



    /* Set uMatrixPalette dirty. */

    Context->vsUniformDirty.uMatrixPaletteDirty        = gcvTRUE;

    Context->vsUniformDirty.uMatrixPaletteInverseDirty = gcvTRUE;



    gcmFOOTER_NO();

}





/******************************************************************************\

********************* Texture matrix modification handlers *********************

\******************************************************************************/

/* Current texture matrix has changed, update pointers. */

static void _TextureMatrixCurrentChanged(

    glsCONTEXT_PTR Context,

    GLuint MatrixID

    )

{

    gcmHEADER_ARG("Context=0x%x", Context);

    gcmASSERT(MatrixID < glvMAX_TEXTURES);



    if (Context->texture.activeSamplerIndex == MatrixID)

    {

        Context->textureMatrix

            = Context->matrixStackArray[glvTEXTURE_MATRIX_0 + MatrixID].topMatrix;

    }



    /* Set uTexMatrix dirty. */

    Context->vsUniformDirty.uTexMatrixDirty = gcvTRUE;



    gcmFOOTER_NO();

}



/* Value of the current texture matrix has changed, invalidate dependets. */

static void _TextureMatrixDataChanged(

    glsCONTEXT_PTR Context,

    GLuint MatrixID

    )

{

    gcmHEADER_ARG("Context=0x%x", Context);

    gcmASSERT(MatrixID < glvMAX_TEXTURES);



    /* Update the hash key. */

    glmSETHASH_1BIT(hashTextureIdentity, Context->textureMatrix->identity, MatrixID);



    /* Invalidate current texture coordinate. */

    Context->texture.sampler[MatrixID].recomputeCoord = GL_TRUE;

    Context->texture.matrixDirty = GL_TRUE;



    /* Set uTexMatrix dirty. */

    Context->vsUniformDirty.uTexMatrixDirty = gcvTRUE;



    gcmFOOTER_NO();

}



/*******************************************************************************

**

**  _InitializeMatrixStack

**

**  Allocate matrix staack and initialize all matrices to identity.

**

**  INPUT:

**

**      Context

**          Pointer to the current context.

**

**      MatrixStack

**          Pointer to the matrix stack container.

**

**      Count

**          Number of matrixes in the stack.

**

**      NotifyCallback

**          Function to call when there is a change on the stack.

**

**  OUTPUT:

**

**      Nothing.

*/

static gceSTATUS _InitializeMatrixStack(

    glsCONTEXT_PTR Context,

    glsMATRIXSTACK_PTR MatrixStack,

    GLuint Count,

    glfMATRIXCHANGEEVENT CurrentChanged,

    glfMATRIXCHANGEEVENT DataChanged,

    GLuint MatrixID

    )

{

    GLuint i;

    gceSTATUS status;



    gcmHEADER_ARG("Context=0x%x MatrixStack=0x%x Count=%u CurrentChanged=0x%x DataChanged=0x%x",

                    Context, MatrixStack, Count, CurrentChanged, DataChanged);



    do

    {

        GLuint stackSize = sizeof(glsMATRIX) * Count;

        gctPOINTER pointer = gcvNULL;



        gcmERR_BREAK(gcoOS_Allocate(

            gcvNULL,

            stackSize,

            &pointer

            ));



        MatrixStack->stack = pointer;



        gcoOS_ZeroMemory(MatrixStack->stack, stackSize);



        MatrixStack->count = Count;

        MatrixStack->index = 0;

        MatrixStack->topMatrix = MatrixStack->stack;

        MatrixStack->currChanged = CurrentChanged;

        MatrixStack->dataChanged = DataChanged;

        MatrixStack->matrixID    = MatrixID;



        for (i = 0; i < MatrixStack->count; i++)

        {

            glsMATRIX_PTR matrix = &MatrixStack->stack[i];

            _LoadIdentityMatrix(matrix);

        }



        (*MatrixStack->currChanged) (Context, MatrixID);

        (*MatrixStack->dataChanged) (Context, MatrixID);

    }

    while (gcvFALSE);



    gcmFOOTER();

    return status;

}





/*******************************************************************************

**

**  _FreeMatrixStack

**

**  Free allocated matrix stack.

**

**  INPUT:

**

**      Context

**          Pointer to the current context.

**

**      MatrixStack

**          Pointer to the matrix stack container.

**

**  OUTPUT:

**

**      Nothing.

*/

static gceSTATUS _FreeMatrixStack(

    glsCONTEXT_PTR Context,

    glsMATRIXSTACK_PTR MatrixStack

    )

{

    gceSTATUS status;

    gcmHEADER_ARG("Context=0x%x MatrixStack=0x%x", Context, MatrixStack);



    if (MatrixStack->stack != gcvNULL)

    {

        status = gcmOS_SAFE_FREE(gcvNULL, MatrixStack->stack);

    }

    else

    {

        status = gcvSTATUS_OK;

    }



    gcmFOOTER();

    return status;

}





/*******************************************************************************

**

**  glfInitializeMatrixStack

**

**  Initialize matrix stacks.

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

gceSTATUS glfInitializeMatrixStack(

    glsCONTEXT_PTR Context

    )

{

    gctINT i;

    gceSTATUS status = gcvSTATUS_OK;



    gcmHEADER_ARG("Context=0x%x", Context);



    gcmONERROR(_InitializeMatrixStack(

        Context,

        &Context->matrixStackArray[glvMODEL_VIEW_MATRIX],

        glvMAX_STACK_NUM_MODELVIEW,

        _ModelViewMatrixCurrentChanged,

        _ModelViewMatrixDataChanged,

        0

        ));



    gcmONERROR(_InitializeMatrixStack(

        Context,

        &Context->matrixStackArray[glvPROJECTION_MATRIX],

        glvMAX_STACK_NUM_PROJECTION,

        _ProjectionMatrixCurrentChanged,

        _ProjectionMatrixDataChanged,

        0

        ));



    for (i = 0; i < glvMAX_PALETTE_MATRICES; i++)

    {

        gcmONERROR(_InitializeMatrixStack(

            Context,

            &Context->matrixStackArray[glvPALETTE_MATRIX_0 + i],

            glvMAX_STACK_NUM_PALETTE,

            _PaletteMatrixCurrentChanged,

            _PaletteMatrixDataChanged,

            i

            ));

    }



    for (i = 0; i < glvMAX_TEXTURES; i++)

    {

        gcmONERROR(_InitializeMatrixStack(

            Context,

            &Context->matrixStackArray[glvTEXTURE_MATRIX_0 + i],

            glvMAX_STACK_NUM_TEXTURES,

            _TextureMatrixCurrentChanged,

            _TextureMatrixDataChanged,

            i

            ));

    }



    Context->modelViewProjectionMatrix.dirty = GL_TRUE;



    status = glmTRANSLATEGLRESULT(glfSetMatrixMode(

        Context,

        GL_MODELVIEW

        ));



    gcmONERROR(status);



    gcmFOOTER_NO();

    return gcvSTATUS_OK;



OnError:

    gcmFOOTER();

    return status;

}





/*******************************************************************************

**

**  glfFlushMatrixStack

**

**  Flush matrix stacks.

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

gceSTATUS glfFlushMatrixStack(

    glsCONTEXT_PTR Context

    )

{

    gctINT i;

    gcmHEADER_ARG("Context=0x%x", Context);



    Context->modelViewInverse3x3TransposedMatrix.dirty = gcvTRUE;

    Context->modelViewInverse4x4TransposedMatrix.dirty = gcvTRUE;

    Context->modelViewProjectionMatrix.dirty = gcvTRUE;

    Context->convertedProjectionMatrix.dirty = gcvTRUE;



    for (i = 0; i < glvMAX_PALETTE_MATRICES; i++)

    {

        Context->paletteMatrixInverse3x3[i].dirty = gcvTRUE;

    }



    gcmFOOTER_NO();

    return gcvSTATUS_OK;

}





/*******************************************************************************

**

**  glfFreeMatrixStack

**

**  Free allocated matrix stack.

**

**  INPUT:

**

**      Context

**          Pointer to the current context.

**

**      MatrixStack

**          Pointer to the matrix stack container.

**

**  OUTPUT:

**

**      Nothing.

*/

gceSTATUS glfFreeMatrixStack(

    glsCONTEXT_PTR Context

    )

{

    gceSTATUS status = gcvSTATUS_OK;



    gcmHEADER();



    do

    {

        gctINT i;



        for (i = 0; i < glvSTACKCOUNT; i++)

        {

            gceSTATUS last = _FreeMatrixStack(

                Context,

                &Context->matrixStackArray[i]

                );



            if (gcmIS_ERROR(last))

            {

                status = last;

            }

        }

    }

    while (gcvFALSE);



    gcmFOOTER();

    return status;

}





/*******************************************************************************

**

**  glfSetMatrixMode

**

**  Set current matrix mode.

**

**  INPUT:

**

**      Context

**          Pointer to the current context.

**

**      MatrixMode

**          Matrix mode to be set.

**

**  OUTPUT:

**

**      Nothing.

**

*/

GLenum glfSetMatrixMode(

    glsCONTEXT_PTR Context,

    GLenum MatrixMode

    )

{

    GLenum result = GL_NO_ERROR;



    gcmHEADER();



    switch (MatrixMode)

    {

    case GL_MODELVIEW:

        Context->matrixMode = glvMODEL_VIEW_MATRIX;

        break;



    case GL_PROJECTION:

        Context->matrixMode = glvPROJECTION_MATRIX;

        break;



    case GL_TEXTURE:

        Context->matrixMode

            = (gleMATRIXMODE) (glvTEXTURE_MATRIX_0

            + Context->texture.activeSamplerIndex);

        break;



    case GL_MATRIX_PALETTE_OES:

        Context->matrixMode

            = (gleMATRIXMODE) (glvPALETTE_MATRIX_0

            + Context->currentPalette);

        break;



    default:

        result = GL_INVALID_ENUM;

    }



    if (result == GL_NO_ERROR)

    {

        Context->currentStack = &Context->matrixStackArray[Context->matrixMode];

        Context->currentMatrix = Context->currentStack->topMatrix;

    }



    gcmFOOTER_ARG("result=0x%04x", result);

    return result;

}





/*******************************************************************************

**

**  glfGetModelViewInverse3x3TransposedMatrix

**

**  Get the current model view inverse transposed 3x3 matrix.

**

**  INPUT:

**

**      Context

**          Pointer to the current context.

**

**  OUTPUT:

**

**      Inversed Model View matrix.

*/

glsMATRIX_PTR glfGetModelViewInverse3x3TransposedMatrix(

    glsCONTEXT_PTR Context

    )

{

    gcmHEADER_ARG("Context=0x%x", Context);



    if (Context->modelViewInverse3x3TransposedMatrix.recompute

    &&  (Context->modelViewMatrix != gcvNULL)

    )

    {

        glsMATRIX modelViewInverse3x3Matrix;



        if (Context->modelViewMatrix->identity ||

            !_InverseMatrix3x3(

                Context->modelViewMatrix,

                &modelViewInverse3x3Matrix))

        {

            _LoadIdentityMatrix(

                &Context->modelViewInverse3x3TransposedMatrix.matrix

                );

        }

        else

        {

            _TransposeMatrix(

                &modelViewInverse3x3Matrix,

                &Context->modelViewInverse3x3TransposedMatrix.matrix

                );

        }



        Context->hashKey.hashModelViewInverse3x3TransIdentity =

            Context->modelViewInverse3x3TransposedMatrix.matrix.identity;



        Context->modelViewInverse3x3TransposedMatrix.recompute = GL_FALSE;

    }



    gcmFOOTER_ARG("result=0x%x", &Context->modelViewInverse3x3TransposedMatrix.matrix);

    return &Context->modelViewInverse3x3TransposedMatrix.matrix;

}





/*******************************************************************************

**

**  glfGetModelViewInverse4x4TransposedMatrix

**

**  Get the current model view inverse transposed matrix.

**

**  INPUT:

**

**      Context

**          Pointer to the current context.

**

**  OUTPUT:

**

**      Model View Projection matrix.

*/

glsMATRIX_PTR glfGetModelViewInverse4x4TransposedMatrix(

    glsCONTEXT_PTR Context

    )

{

    gcmHEADER_ARG("Context=0x%x", Context);

    if (Context->modelViewInverse4x4TransposedMatrix.recompute)

    {

        glsMATRIX modelViewInverseMatrix;



        if (Context->modelViewMatrix->identity ||

            !_InverseMatrix4x4(

                Context->modelViewMatrix,

                &modelViewInverseMatrix))

        {

            _LoadIdentityMatrix(

                &Context->modelViewInverse4x4TransposedMatrix.matrix

                );

        }

        else

        {

            _TransposeMatrix(

                &modelViewInverseMatrix,

                &Context->modelViewInverse4x4TransposedMatrix.matrix

                );

        }



        Context->modelViewInverse4x4TransposedMatrix.recompute = GL_FALSE;

    }



    gcmFOOTER_ARG("result=0x%x", &Context->modelViewInverse4x4TransposedMatrix.matrix);

    return &Context->modelViewInverse4x4TransposedMatrix.matrix;

}





/*******************************************************************************

**

**  glfGetMatrixPaletteInverse

**

**  Get the array of current inversed 3x3 palette matrices.

**

**  INPUT:

**

**      Context

**          Pointer to the current context.

**

**  OUTPUT:

**

**      Array of inversed 3x3 palette matrices.

*/

glsDEPENDENTMATRIX_PTR glfGetMatrixPaletteInverse(

    glsCONTEXT_PTR Context

    )

{

    gcmHEADER_ARG("Context=0x%x", Context);

    if (Context->paletteMatrixInverse3x3Recompute)

    {

        gctUINT i;

        glsMATRIX_PTR paletteMatrix;



        for (i = 0; i < glvMAX_PALETTE_MATRICES; i += 1)

        {

            if (!Context->paletteMatrixInverse3x3[i].recompute)

            {

                continue;

            }



            /* Get the current palette matrix. */

            paletteMatrix

                = Context->matrixStackArray[glvPALETTE_MATRIX_0 + i].topMatrix;



            if (paletteMatrix->identity ||

                !_InverseMatrix3x3(

                    paletteMatrix,

                    &Context->paletteMatrixInverse3x3[i].matrix))

            {

                _LoadIdentityMatrix(

                    &Context->paletteMatrixInverse3x3[i].matrix

                    );

            }



            Context->paletteMatrixInverse3x3[i].recompute = GL_FALSE;

        }



        Context->paletteMatrixInverse3x3Recompute = GL_FALSE;

    }



    gcmFOOTER_ARG("result=0x%x", Context->paletteMatrixInverse3x3);

    return Context->paletteMatrixInverse3x3;

}





/*******************************************************************************

**

**  glfGetProjectionMatrix

**

**  Get the current converted projection matrix.

**

**  INPUT:

**

**      Context

**          Pointer to the current context.

**

**  OUTPUT:

**

**          Projection matrix.

*/

glsMATRIX_PTR glfGetConvertedProjectionMatrix(

    glsCONTEXT_PTR Context

    )

{

    gcmHEADER_ARG("Context=0x%x", Context);

    if (Context->convertedProjectionMatrix.recompute)

    {

        glfConvertToVivanteMatrix(

            Context,

            Context->projectionMatrix,

            &Context->convertedProjectionMatrix.matrix

            );



        Context->convertedProjectionMatrix.recompute = GL_FALSE;

    }



    gcmFOOTER_ARG("result=0x%x", &Context->convertedProjectionMatrix.matrix);

    return &Context->convertedProjectionMatrix.matrix;

}





/*******************************************************************************

**

**  glfGetModelViewProjectionMatrix

**

**  Get the current model view projection matrix.

**

**  INPUT:

**

**      Context

**          Pointer to the current context.

**

**  OUTPUT:

**

**      Model View Projection matrix.

*/

glsMATRIX_PTR glfGetModelViewProjectionMatrix(

    glsCONTEXT_PTR Context

    )

{

    gcmHEADER_ARG("Context=0x%x", Context);

    if (Context->modelViewProjectionMatrix.recompute)

    {

        glsMATRIX_PTR temp;

        glsMATRIX product;



        if (Context->projectionMatrix->identity)

        {

            temp = Context->modelViewMatrix;

        }

        else if (Context->modelViewMatrix->identity)

        {

            temp = Context->projectionMatrix;

        }

        else

        {

            temp = &product;



            _MultiplyMatrix4x4(

                Context->projectionMatrix,

                Context->modelViewMatrix,

                temp

                );

        }



        glfConvertToVivanteMatrix(

            Context,

            temp,

            &Context->modelViewProjectionMatrix.matrix

            );



        Context->hashKey.hashModelViewProjectionIdentity =

            Context->modelViewProjectionMatrix.matrix.identity;



        Context->modelViewProjectionMatrix.recompute = GL_FALSE;

    }



    gcmFOOTER_ARG("result=0x%x", &Context->modelViewProjectionMatrix.matrix);

    return &Context->modelViewProjectionMatrix.matrix;

}





/*******************************************************************************

**

**  glfQueryMatrixState

**

**  Queries matrix state values.

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

**

*/

GLboolean glfQueryMatrixState(

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

    case GL_MATRIX_MODE:

        {

            GLuint mode;



            if (Context->matrixMode == glvMODEL_VIEW_MATRIX)

            {

                mode = GL_MODELVIEW;

            }

            else if (Context->matrixMode == glvPROJECTION_MATRIX)

            {

                mode = GL_PROJECTION;

            }

            else if ((Context->matrixMode >= glvPALETTE_MATRIX_0) &&

                (Context->matrixMode <= glvPALETTE_MATRIX_LAST))

            {

                mode = GL_MATRIX_PALETTE_OES;

            }

            else

            {

                mode = GL_TEXTURE;

            }



            glfGetFromEnum(

                mode,

                Value,

                Type

                );

        }

        break;



    case GL_MODELVIEW_MATRIX:

        glfGetFromMatrix(

            Context->modelViewMatrix,

            Value,

            Type

            );

        break;



    case GL_PROJECTION_MATRIX:

        glfGetFromMatrix(

            Context->projectionMatrix,

            Value,

            Type

            );

        break;



    case GL_TEXTURE_MATRIX:

        glfGetFromMatrix(

            Context->textureMatrix,

            Value,

            Type

            );

        break;



    case GL_MAX_MODELVIEW_STACK_DEPTH:

        glfGetFromInt(

            glvMAX_STACK_NUM_MODELVIEW,

            Value,

            Type

            );

        break;



    case GL_MAX_PROJECTION_STACK_DEPTH:

        glfGetFromInt(

            glvMAX_STACK_NUM_PROJECTION,

            Value,

            Type

            );

        break;



    case GL_MAX_TEXTURE_STACK_DEPTH:

        glfGetFromInt(

            glvMAX_STACK_NUM_TEXTURES,

            Value,

            Type

            );

        break;



    case GL_MODELVIEW_STACK_DEPTH:

        glfGetFromInt(

            Context->matrixStackArray[glvMODEL_VIEW_MATRIX].index + 1,

            Value,

            Type

            );

        break;



    case GL_PROJECTION_STACK_DEPTH:

        glfGetFromInt(

            Context->matrixStackArray[glvPROJECTION_MATRIX].index + 1,

            Value,

            Type

            );

        break;



    case GL_TEXTURE_STACK_DEPTH:

        {

            GLint stackIndex

                = glvTEXTURE_MATRIX_0

                + Context->texture.activeSamplerIndex;



            glfGetFromInt(

                Context->matrixStackArray[stackIndex].index + 1,

                Value,

                Type

                );

        }

        break;



    /* Supported under GL_OES_matrix_get extension. */

    case GL_MODELVIEW_MATRIX_FLOAT_AS_INT_BITS_OES:

        glfGetFloatFromMatrix(

            Context->modelViewMatrix,

            Value

            );

        break;



    case GL_PROJECTION_MATRIX_FLOAT_AS_INT_BITS_OES:

        glfGetFloatFromMatrix(

            Context->projectionMatrix,

            Value

            );

        break;



    case GL_TEXTURE_MATRIX_FLOAT_AS_INT_BITS_OES:

        glfGetFloatFromMatrix(

            Context->textureMatrix,

            Value

            );

        break;



    case GL_CURRENT_PALETTE_MATRIX_OES:

        glfGetFromInt(

            Context->currentPalette,

            Value,

            glvINT

            );

        break;



    default:

        result = GL_FALSE;

    }



    gcmFOOTER_ARG("result=%d", result);

    /* Return result. */

    return result;

}





/*******************************************************************************

**

**  glfMultiplyVector3ByMatrix3x3

**

**  Multiplies a 3-component (x, y, z) vector by a 3x3 matrix.

**

**  INPUT:

**

**      Vector

**          Points to the input vector.

**

**      Matrix

**          Points to the input matrix.

**

**  OUTPUT:

**

**      Result

**          Points to the resulting vector.

*/

void glfMultiplyVector3ByMatrix3x3(

    const glsVECTOR_PTR Vector,

    const glsMATRIX_PTR Matrix,

    glsVECTOR_PTR Result

    )

{

    GLint x, y;

    GLfloat result[3];



    gcmHEADER_ARG("Vector=0x%x Matrix=0x%04x Result=0x%x", Vector, Matrix, Result);

    /* Fast case. */

    if (Matrix->identity)

    {

        if (Result != Vector)

        {

            *Result = *Vector;

        }



        gcmFOOTER_NO();

        return;

    }



    /* Multiply if not identity. */

    for (y = 0; y < 3; y++)

    {

        result[y] = 0;



        for (x = 0; x < 3; x++)

        {

            result[y]

                += glmVECFLOAT(Vector, x)

                *  glmMATFLOAT(Matrix, x, y);

        }

    }



    glfSetVector3(Result, result);



    gcmFOOTER_NO();

}





/*******************************************************************************

**

**  glfMultiplyVector4ByMatrix4x4

**

**  Multiplies a 4-component (x, y, z, w) vector by a 4x4 matrix.

**

**  INPUT:

**

**      Matrix

**          Points to a matrix.

**

**      Vector

**          Points to a column vector.

**

**  OUTPUT:

**

**       Result

**          A vector, product of multiplication.

*/

void glfMultiplyVector4ByMatrix4x4(

    const glsVECTOR_PTR Vector,

    const glsMATRIX_PTR Matrix,

    glsVECTOR_PTR Result

    )

{

    GLint x, y;

    GLfloat result[4];



    gcmHEADER_ARG("Vector=0x%x Matrix=0x%04x Result=0x%x", Vector, Matrix, Result);

    /* Fast case. */

    if (Matrix->identity)

    {

        if (Result != Vector)

        {

            *Result = *Vector;

        }



        gcmFOOTER_NO();

        return;

    }



    for (y = 0; y < 4; y++)

    {

        result[y] = 0;



        for (x = 0; x < 4; x++)

        {

            result[y]

                += glmVECFLOAT(Vector, x)

                *  glmMATFLOAT(Matrix, x, y);

        }

    }



    glfSetVector4(Result, result);



    gcmFOOTER_NO();

}



/*******************************************************************************

**

**  glfRSQX

**

**  Reciprocal square root in fixed point.

**

**  INPUT:

**

**      X

**          Input value.

**

**  OUTPUT:

**

**      1 / sqrt(X).

*/

GLfixed glfRSQX(

    GLfixed X

    )

{

    static const GLushort rsqrtx_table[8] =

    {

        0x6A0A, 0x5555, 0x43D1, 0x34BF, 0x279A, 0x1C02, 0x11AD, 0x0865

    };



    GLfixed r;

    int exp, i;



    gcmHEADER_ARG("X=0x%08x", X);



    gcmASSERT(X >= 0);



    if (X == gcvONE_X)

    {

        gcmFOOTER_ARG("0x%x", gcvONE_X);

        return gcvONE_X;

    }



    r   = X;

    exp = 31;



    if (r & 0xFFFF0000)

    {

        exp -= 16;

        r >>= 16;

    }



    if (r & 0xFF00)

    {

        exp -= 8;

        r >>= 8;

    }



    if (r & 0xF0)

    {

        exp -= 4;

        r >>= 4;

    }



    if (r & 0xC)

    {

        exp -= 2;

        r >>= 2;

    }



    if (r & 0x2)

    {

        exp -= 1;

    }



    if (exp > 28)

    {

        static const GLfixed low_value_result[] =

        {

            0x7FFFFFFF, 0x01000000, 0x00B504F3, 0x0093CD3A,

            0x00800000, 0x00727C97, 0x006882F5, 0x0060C247

        };



        gcmFOOTER_ARG("0x%x", low_value_result[X & 7]);

        return low_value_result[X & 7];

    }



    r    = gcvONE_X + rsqrtx_table[(X >> (28 - exp)) & 0x7];

    exp -= 16;



    if (exp <= 0)

    {

        r >>= -exp >> 1;

    }

    else

    {

        r <<= (exp >> 1) + (exp & 1);

    }



    if (exp & 1)

    {

        r = gcmXMultiply(r, rsqrtx_table[0]);

    }



    for (i = 0; i < 3; i++)

    {

        r = gcmXMultiply(

            (r >> 1),

            (3 << 16) - gcmXMultiply(gcmXMultiply(r, r), X)

            );

    }



    gcmFOOTER_ARG("0x%x", r);

    return r;

}



static void _Norm3F(

    GLfloat X,

    GLfloat Y,

    GLfloat Z,

    glsVECTOR_PTR Normal

    )

{

    /* Compute normal. */

    GLfloat sum;

    GLfloat norm;

    gcmHEADER_ARG("X=0x%08x Y=0x%08x Z=0x%08x Normal=0x%x", X, Y, Z, Normal);



    /* Compute normal. */

    sum = X * X + Y * Y + Z * Z;



    if (sum == 0.0f)

    {

        norm = 1.0f;

    }

    else

    {

        norm = 1.0f / gcoMATH_SquareRoot(sum);

    }



    /* Multiply vector by normal. */

    X = X * norm;

    Y = Y * norm;

    Z = Z * norm;



    /* Set the result. */

    glfSetFloatVector4(Normal, X, Y, Z, 0);

    gcmFOOTER_NO();

}



/******************************************************************************\

************************* OpenGL Matrix Management Code ************************

\******************************************************************************/

/*******************************************************************************

**

**  glMatrixMode

**

**  glMatrixMode sets the current matrix mode. Mode can assume one of four

**  values:

**

**     GL_MODELVIEW

**        Applies subsequent matrix operations to the modelview matrix stack.

**

**     GL_PROJECTION

**        Applies subsequent matrix operations to the projection matrix stack.

**

**     GL_TEXTURE

**        Applies subsequent matrix operations to the texture matrix stack.

**

**     GL_MATRIX_PALETTE_OES

**        Enables the matrix palette stack extension, and applies subsequent

**        matrix operations to the matrix palette stack.

**

**  INPUT:

**

**      Mode

**          Specifies which matrix stack is the target for subsequent matrix

**          operations. The initial value is GL_MODELVIEW.

**

**  OUTPUT:

**

**      Nothing.

*/

#ifdef _GC_OBJ_ZONE

#undef _GC_OBJ_ZONE

#endif

#define _GC_OBJ_ZONE    gcdZONE_ES11_MATRIX



GL_API void GL_APIENTRY glMatrixMode(

    GLenum Mode

    )

{

    glmENTER1(glmARGENUM, Mode)

    {

        gcmDUMP_API("${ES11 glMatrixMode 0x%08X}", Mode);



        glmPROFILE(context, GLES1_MATRIXMODE, 0);

        glmERROR(glfSetMatrixMode(context, Mode));

    }

    glmLEAVE();

}





/*******************************************************************************

**

**  glLoadIdentity

**

**  glLoadIdentity replaces the current matrix with the identity matrix. It is

**  semantically equivalent to calling glLoadMatrix with the identity matrix:

**

**      1   0   0   0

**      0   1   0   0

**      0   0   1   0

**      0   0   0   1

**

**  INPUT:

**

**      Nothing.

**

**  OUTPUT:

**

**      Nothing.

*/

GL_API void GL_APIENTRY glLoadIdentity(

    void

    )

{

    glmENTER()

    {

        GLuint matrixID;

        gcmDUMP_API("${ES11 glLoadIdentity}");



        glmPROFILE(context, GLES1_LOADIDENTITY, 0);

        /* Initialize identity matrix. */

        _LoadIdentityMatrix(context->currentMatrix);



        /* Notify dependents of the change. */

        matrixID = context->currentStack->matrixID;

        (*context->currentStack->dataChanged) (context, matrixID);

    }

    glmLEAVE();

}





/*******************************************************************************

**

**  glLoadMatrix

**

**  glLoadMatrix replaces the current matrix with the one whose elements are

**  specified by Matrix. The current matrix is the projection matrix, modelview

**  matrix, or texture matrix, depending on the current matrix mode

**  (see glMatrixMode).

**

**  INPUT:

**

**      Matrix

**          Specifies a pointer to 16 consecutive values, which are used as the

**          elements of a 4 ? 4 column-major matrix.

**

**  OUTPUT:

**

**      Nothing.

*/

GL_API void GL_APIENTRY glLoadMatrixx(

    const GLfixed* Matrix

    )

{

    glmENTER1(glmARGPTR, Matrix)

    {

        GLfloat matrix[16];

        GLint i;



        gcmDUMP_API("${ES11 glLoadMatrixx (0x%08X)", Matrix);

        gcmDUMP_API_ARRAY(Matrix, 16);

        gcmDUMP_API("$}");



        glmPROFILE(context, GLES1_LOADMATRIXX, 0);



        /* convert to Float */

        for (i = 0; i < 16; i++)

        {

            matrix[i] = glmFIXED2FLOAT(Matrix[i]);

        }



        /* Load the matrix. */

        if(_LoadMatrix(context->currentMatrix, matrix))

        {

            /* Notify dependents of the change. */

            GLuint matrixID = context->currentStack->matrixID;

            (*context->currentStack->dataChanged) (context, matrixID);

        }

    }

    glmLEAVE();

}



GL_API void GL_APIENTRY glLoadMatrixxOES(

    const GLfixed* Matrix

    )

{

    glmENTER1(glmARGPTR, Matrix)

    {

        GLfloat matrix[16];

        GLint i;



        gcmDUMP_API("${ES11 glLoadMatrixxOES (0x%08X)", Matrix);

        gcmDUMP_API_ARRAY(Matrix, 16);

        gcmDUMP_API("$}");



        glmPROFILE(context, GLES1_LOADMATRIXX, 0);



        /* convert to Float */

        for (i = 0; i < 16; i++)

        {

            matrix[i] = glmFIXED2FLOAT(Matrix[i]);

        }



        /* Load the matrix. */

        if(_LoadMatrix(context->currentMatrix, matrix))

        {

            /* Notify dependents of the change. */

            GLuint matrixID = context->currentStack->matrixID;

            (*context->currentStack->dataChanged) (context, matrixID);

        }

    }

    glmLEAVE();

}



GL_API void GL_APIENTRY glLoadMatrixf(

    const GLfloat* Matrix

    )

{

    glmENTER1(glmARGPTR, Matrix)

    {

        gcmDUMP_API("${ES11 glLoadMatrixf (0x%08X)", Matrix);

        gcmDUMP_API_ARRAY(Matrix, 16);

        gcmDUMP_API("$}");



        glmPROFILE(context, GLES1_LOADMATRIXF, 0);



        /* Load the matrix. */

        if(_LoadMatrix(context->currentMatrix, Matrix))

        {

            /* Notify dependents of the change. */

            GLuint matrixID = context->currentStack->matrixID;

            (*context->currentStack->dataChanged) (context, matrixID);

        }

    }

    glmLEAVE();

}



static void _Orthof(

    GLfloat Left,

    GLfloat Right,

    GLfloat Bottom,

    GLfloat Top,

    GLfloat zNear,

    GLfloat zFar

    )

{

    glmENTER6(glmARGFLOAT, Left, glmARGFLOAT, Right, glmARGFLOAT, Bottom,

              glmARGFLOAT, Top, glmARGFLOAT, zNear, glmARGFLOAT, zFar)

    {

        glsMATRIX ortho, result;

        GLuint matrixID;



        glmPROFILE(context, GLES1_ORTHOF, 0);



        /* Verify arguments. */

        if ((Left == Right) ||

            (Bottom == Top) ||

            (zNear == zFar))

        {

            /* Invalid value. */

            glmERROR(GL_INVALID_VALUE);

            break;

        }



        /* Fill in ortho matrix. */

        gcoOS_ZeroMemory(&ortho, sizeof(ortho));

        glmMATFLOAT(&ortho, 0, 0) =  2.0f          / (Right  - Left);

        glmMATFLOAT(&ortho, 1, 1) =  2.0f          / (Top    - Bottom);

        glmMATFLOAT(&ortho, 2, 2) =  2.0f          / (zNear  - zFar);

        glmMATFLOAT(&ortho, 3, 0) = (Right + Left) / (Left   - Right);

        glmMATFLOAT(&ortho, 3, 1) = (Top + Bottom) / (Bottom - Top);

        glmMATFLOAT(&ortho, 3, 2) = (zFar + zNear) / (zNear  - zFar);

        glmMATFLOAT(&ortho, 3, 3) =  1.0f;



        /* Multipy the current matrix by the ortho matrix. */

        _MultiplyMatrix4x4(context->currentMatrix, &ortho, &result);



        /* Replace the current matrix. */

        *context->currentMatrix = result;



        /* Notify dependents of the change. */

        matrixID = context->currentStack->matrixID;

        (*context->currentStack->dataChanged) (context, matrixID);

    }

    glmLEAVE();

}



/*******************************************************************************

**

**  glOrtho

**

**  glOrtho describes a transformation that produces a parallel projection.

**  The current matrix (see glMatrixMode) is multiplied by this matrix and

**  the result replaces the current matrix.

**

**  INPUT:

**

**      Left

**      Right

**          Specify the coordinates for the left and right vertical

**          clipping planes.

**

**      Bottom

**      Top

**          Specify the coordinates for the bottom and top horizontal

**          clipping planes.

**

**      zNear

**      zFar

**          Specify the distances to the nearer and farther depth clipping

**          planes. These values are negative if the plane is to be behind

**          the viewer.

**

**  OUTPUT:

**

**      Nothing.

*/

GL_API void GL_APIENTRY glOrthox(

    GLfixed Left,

    GLfixed Right,

    GLfixed Bottom,

    GLfixed Top,

    GLfixed zNear,

    GLfixed zFar

    )

{

    GLfloat left, right, bottom, top, znear, zfar;



    gcmDUMP_API("${ES11 glOrthox 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X}",

        Left, Right, Bottom, Top, zNear, zFar);



    left   = glmFIXED2FLOAT(Left);

    right  = glmFIXED2FLOAT(Right);

    bottom = glmFIXED2FLOAT(Bottom);

    top    = glmFIXED2FLOAT(Top);

    znear  = glmFIXED2FLOAT(zNear);

    zfar   = glmFIXED2FLOAT(zFar);



    _Orthof(left, right, bottom, top, znear, zfar);

}



GL_API void GL_APIENTRY glOrthoxOES(

    GLfixed Left,

    GLfixed Right,

    GLfixed Bottom,

    GLfixed Top,

    GLfixed zNear,

    GLfixed zFar

    )

{

    GLfloat left, right, bottom, top, znear, zfar;



    gcmDUMP_API("${ES11 glOrthoxOES 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X}",

        Left, Right, Bottom, Top, zNear, zFar);



    left   = glmFIXED2FLOAT(Left);

    right  = glmFIXED2FLOAT(Right);

    bottom = glmFIXED2FLOAT(Bottom);

    top    = glmFIXED2FLOAT(Top);

    znear  = glmFIXED2FLOAT(zNear);

    zfar   = glmFIXED2FLOAT(zFar);



    _Orthof(left, right, bottom, top, znear, zfar);

}



GL_API void GL_APIENTRY glOrthof(

    GLfloat Left,

    GLfloat Right,

    GLfloat Bottom,

    GLfloat Top,

    GLfloat zNear,

    GLfloat zFar

    )

{

    gcmDUMP_API("${ES11 glOrthof 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X}",

        *(GLuint*)&Left, *(GLuint*)&Right, *(GLuint*)&Bottom,

        *(GLuint*)&Top, *(GLuint*)&zNear, *(GLuint*)&zFar);



    _Orthof(Left, Right, Bottom, Top, zNear, zFar);

}



GL_API void GL_APIENTRY glOrthofOES(

    GLfloat Left,

    GLfloat Right,

    GLfloat Bottom,

    GLfloat Top,

    GLfloat zNear,

    GLfloat zFar

    )

{

    gcmDUMP_API("${ES11 glOrthofOES 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X}",

        *(GLuint*)&Left, *(GLuint*)&Right, *(GLuint*)&Bottom,

        *(GLuint*)&Top, *(GLuint*)&zNear, *(GLuint*)&zFar);



    _Orthof(Left, Right, Bottom, Top, zNear, zFar);

}



void _Translatef(

    GLfloat X,

    GLfloat Y,

    GLfloat Z

    )

{

    glmENTER3(glmARGFLOAT, X, glmARGFLOAT, Y, glmARGFLOAT, Z)

    {

        GLuint matrixID;

        glmPROFILE(context, GLES1_TRANSLATEF, 0);

        /* Optimization: do nothing when translation is zero. */

        if ((X == 0.0f) && (Y == 0.0f) && (Z == 0.0f))

        {

            break;

        }



        if (context->currentMatrix->identity)

        {

            /*  Optimized result with identity:



                [3,0] = X

                [3,1] = Y

                [3,2] = Z

            */

            glmMATFLOAT(context->currentMatrix, 3, 0) = X;

            glmMATFLOAT(context->currentMatrix, 3, 1) = Y;

            glmMATFLOAT(context->currentMatrix, 3, 2) = Z;

        }

        else

        {

            /*  Optimized result:



            [3,0] = [0,0] * X + [1,0] * Y + [2,0] * Z + [3,0]

            [3,1] = [0,1] * X + [1,1] * Y + [2,1] * Z + [3,1]

            [3,2] = [0,2] * X + [1,2] * Y + [2,2] * Z + [3,2]

            [3,3] = [0,3] * X + [1,3] * Y + [2,3] * Z + [3,3]

            */

            glmMATFLOAT(context->currentMatrix, 3, 0) +=

                glmMATFLOAT(context->currentMatrix, 0, 0) * X +

                glmMATFLOAT(context->currentMatrix, 1, 0) * Y +

                glmMATFLOAT(context->currentMatrix, 2, 0) * Z ;

            glmMATFLOAT(context->currentMatrix, 3, 1) +=

                glmMATFLOAT(context->currentMatrix, 0, 1) * X +

                glmMATFLOAT(context->currentMatrix, 1, 1) * Y +

                glmMATFLOAT(context->currentMatrix, 2, 1) * Z ;

            glmMATFLOAT(context->currentMatrix, 3, 2) +=

                glmMATFLOAT(context->currentMatrix, 0, 2) * X +

                glmMATFLOAT(context->currentMatrix, 1, 2) * Y +

                glmMATFLOAT(context->currentMatrix, 2, 2) * Z ;

            glmMATFLOAT(context->currentMatrix, 3, 3) +=

                glmMATFLOAT(context->currentMatrix, 0, 3) * X +

                glmMATFLOAT(context->currentMatrix, 1, 3) * Y +

                glmMATFLOAT(context->currentMatrix, 2, 3) * Z ;

        }



        context->currentMatrix->identity = GL_FALSE;



        /* Notify dependents of the change. */

        matrixID = context->currentStack->matrixID;

        (*context->currentStack->dataChanged) (context, matrixID);

    }

    glmLEAVE();

}











/*******************************************************************************

**

**  glTranslate

**

**  glTranslate produces a translation by (x, y, z). The current matrix

**  (see glMatrixMode) is multiplied by this translation matrix, with

**  the product replacing the current matrix, as if glMultMatrix were called

**  with the following matrix for its argument:

**

**      1   0   0   x

**      0   1   0   y

**      0   0   1   z

**      0   0   0   1

**

**  If the matrix mode is either GL_MODELVIEW or GL_PROJECTION, all objects

**  drawn after a call to glTranslate are translated.

**

**  Use glPushMatrix and glPopMatrix to save and restore the untranslated

**  coordinate system.

**

**  INPUT:

**

**      X

**      Y

**      Z

**          Specify the x, y, and z coordinates of a translation vector.

**

**  OUTPUT:

**

**      Nothing.

*/

GL_API void GL_APIENTRY glTranslatex(

    GLfixed X,

    GLfixed Y,

    GLfixed Z

    )

{

    GLfloat x, y, z;



    gcmDUMP_API("${ES11 glTranslatex 0x%08X 0x%08X 0x%08X}", X, Y, Z);



    x = glmFIXED2FLOAT(X);

    y = glmFIXED2FLOAT(Y);

    z = glmFIXED2FLOAT(Z);



    _Translatef(x, y, z);

}



GL_API void GL_APIENTRY glTranslatexOES(

    GLfixed X,

    GLfixed Y,

    GLfixed Z

    )

{

    GLfloat x, y, z;



    gcmDUMP_API("${ES11 glTranslatexOES 0x%08X 0x%08X 0x%08X}", X, Y, Z);



    x = glmFIXED2FLOAT(X);

    y = glmFIXED2FLOAT(Y);

    z = glmFIXED2FLOAT(Z);



    _Translatef(x, y, z);

}



GL_API void GL_APIENTRY glTranslatef(

    GLfloat X,

    GLfloat Y,

    GLfloat Z

    )

{

    gcmDUMP_API("${ES11 glTranslatef 0x%08X 0x%08X 0x%08X}", *(GLuint*)&X, *(GLuint*)&Y, *(GLuint*)&Z);



    _Translatef(X,Y,Z);

}



void _Rotatef(

    GLfloat Angle,

    GLfloat X,

    GLfloat Y,

    GLfloat Z

    )

{

    glmENTER4(glmARGFLOAT, Angle, glmARGFLOAT, X, glmARGFLOAT, Y,

              glmARGFLOAT, Z)

    {

        GLfloat rad;

        GLfloat s, c, c1;

        glsVECTOR normal;

        GLfloat nx, ny, nz;

        GLfloat xx_1c, xy_1c, xz_1c, yy_1c, yz_1c, zz_1c;

        GLfloat xs, ys, zs;

        GLuint matrixID;



        glmPROFILE(context, GLES1_ROTATEF, 0);

        /* Optimization: do nothing when rotation is 0 or axis is origin. */

        if (Angle == 0.0f)

        {

            break;

        }



        rad = glvFLOATPIOVER180 * Angle;



        s  = gcoMATH_Sine(rad);

        c  = gcoMATH_Cosine(rad);

        c1 = 1.0f - c;



        _Norm3F(X, Y, Z, &normal);



        nx = glmVECFLOAT(&normal, 0);

        ny = glmVECFLOAT(&normal, 1);

        nz = glmVECFLOAT(&normal, 2);



        xx_1c = nx * nx * c1;

        xy_1c = nx * ny * c1;

        xz_1c = nx * nz * c1;

        yy_1c = ny * ny * c1;

        yz_1c = ny * nz * c1;

        zz_1c = nz * nz * c1;



        xs = nx * s;

        ys = ny * s;

        zs = nz * s;



        if (context->currentMatrix->identity)

        {

            /*  Optimized result with identity:



                [0,0] = xx_1c + c

                [0,1] = xy_1c + zs

                [0,2] = xz_1c - ys

                [1,0] = xy_1c - zs

                [1,1] = yy_1c + c

                [1,2] = yz_1c + xs

                [2,0] = xz_1c + ys

                [2,1] = yz_1c - xs

                [2,2] = zz_1c + c

            */

            glmMATFLOAT(context->currentMatrix, 0, 0) = xx_1c + c;

            glmMATFLOAT(context->currentMatrix, 0, 1) = xy_1c + zs;

            glmMATFLOAT(context->currentMatrix, 0, 2) = xz_1c - ys;

            glmMATFLOAT(context->currentMatrix, 1, 0) = xy_1c - zs;

            glmMATFLOAT(context->currentMatrix, 1, 1) = yy_1c + c;

            glmMATFLOAT(context->currentMatrix, 1, 2) = yz_1c + xs;

            glmMATFLOAT(context->currentMatrix, 2, 0) = xz_1c + ys;

            glmMATFLOAT(context->currentMatrix, 2, 1) = yz_1c - xs;

            glmMATFLOAT(context->currentMatrix, 2, 2) = zz_1c + c;

        }

        else

        {

            /*  Optimized result:

                [0,0] = [0,0] * (xx_1c + c)  + [1,0] * (xy_1c + zs) + [2,0] * (xz_1c - ys)

                [0,1] = [0,1] * (xx_1c + c)  + [1,1] * (xy_1c + zs) + [2,1] * (xz_1c - ys)

                [0,2] = [0,2] * (xx_1c + c)  + [1,2] * (xy_1c + zs) + [2,2] * (xz_1c - ys)

                [0,3] = [0,3] * (xx_1c + c)  + [1,3] * (xy_1c + zs) + [2,3] * (xz_1c - ys)

                [1,0] = [0,0] * (xy_1c - zs) + [1,0] * (yy_1c + c)  + [2,0] * (yz_1c + xs)

                [1,1] = [0,1] * (xy_1c - zs) + [1,1] * (yy_1c + c)  + [2,1] * (yz_1c + xs)

                [1,2] = [0,2] * (xy_1c - zs) + [1,2] * (yy_1c + c)  + [2,2] * (yz_1c + xs)

                [1,3] = [0,3] * (xy_1c - zs) + [1,3] * (yy_1c + c)  + [2,3] * (yz_1c + xs)

                [2,0] = [0,0] * (xz_1c + ys) + [1,0] * (yz_1c - xs) + [2,0] * (zz_1c + c)

                [2,1] = [0,1] * (xz_1c + ys) + [1,1] * (yz_1c - xs) + [2,1] * (zz_1c + c)

                [2,2] = [0,2] * (xz_1c + ys) + [1,2] * (yz_1c - xs) + [2,2] * (zz_1c + c)

                [2,3] = [0,3] * (xz_1c + ys) + [1,3] * (yz_1c - xs) + [2,3] * (zz_1c + c)

            */

            GLint x, y;

            GLfloat rotation[3][3];

            glsMATRIX source = *context->currentMatrix;



            rotation[0][0] = xx_1c + c;

            rotation[0][1] = xy_1c + zs;

            rotation[0][2] = xz_1c - ys;

            rotation[1][0] = xy_1c - zs;

            rotation[1][1] = yy_1c + c;

            rotation[1][2] = yz_1c + xs;

            rotation[2][0] = xz_1c + ys;

            rotation[2][1] = yz_1c - xs;

            rotation[2][2] = zz_1c + c;



            for (x = 0; x < 3; ++x)

            {

                for (y = 0; y < 4; ++y)

                {

                    glmMATFLOAT(context->currentMatrix, x, y) =

                        glmMATFLOAT(&source, 0, y) * rotation[x][0] +

                        glmMATFLOAT(&source, 1, y) * rotation[x][1] +

                        glmMATFLOAT(&source, 2, y) * rotation[x][2] ;

                }

            }

        }



        context->currentMatrix->identity = GL_FALSE;



        /* Notify dependents of the change. */

        matrixID = context->currentStack->matrixID;

        (*context->currentStack->dataChanged) (context, matrixID);

    }

    glmLEAVE();

}







/*******************************************************************************

**

**  glRotate

**

**  glRotate produces a rotation of Angle degrees around the vector (x, y, z).

**  The current matrix (see glMatrixMode) is multiplied by a rotation matrix

**  with the product replacing the current matrix, as if glMultMatrix were

**  called with the following matrix as its argument:

**

**      xx (1 - c) + c      xy (1 - c) - zs     xz (1 - c) + ys     0

**      xy (1 - c) + zs     yy (1 - c) + c      yz (1 - c) - xs     0

**      xz (1 - c) - ys     yz (1 - c) + xs     zz (1 - c) + c      0

**      0                   0                   0                   1

**

**  Where c = cos(Angle), s = sin(Angle), and || (x, y, z) || = 1, (if not,

**  the GL will normalize this vector).

**

**  If the matrix mode is either GL_MODELVIEW or GL_PROJECTION, all objects

**  drawn after glRotate is called are rotated. Use glPushMatrix and

**  glPopMatrix to save and restore the unrotated coordinate system.

**

**  INPUT:

**

**      Angle

**          Specifies the angle of rotation, in degrees.

**

**      X

**      Y

**      Z

**          Specify the x, y, and z coordinates of a vector, respectively.

**

**  OUTPUT:

**

**      Nothing.

*/

GL_API void GL_APIENTRY glRotatex(

    GLfixed Angle,

    GLfixed X,

    GLfixed Y,

    GLfixed Z

    )

{

    GLfloat angle, x, y, z;



    gcmDUMP_API("${ES11 glRotatex 0x%08X 0x%08X 0x%08X 0x%08X}", Angle, X, Y, Z);



    /* Convert to float */

    angle = glmFIXED2FLOAT(Angle);

    x = glmFIXED2FLOAT(X);

    y = glmFIXED2FLOAT(Y);

    z = glmFIXED2FLOAT(Z);



    _Rotatef(angle, x, y, z);

}



GL_API void GL_APIENTRY glRotatexOES(

    GLfixed Angle,

    GLfixed X,

    GLfixed Y,

    GLfixed Z

    )

{

    GLfloat angle, x, y, z;



    gcmDUMP_API("${ES11 glRotatexOES 0x%08X 0x%08X 0x%08X 0x%08X}", Angle, X, Y, Z);



    /* Convert to float */

    angle = glmFIXED2FLOAT(Angle);

    x = glmFIXED2FLOAT(X);

    y = glmFIXED2FLOAT(Y);

    z = glmFIXED2FLOAT(Z);



    _Rotatef(angle, x, y, z);

}



GL_API void GL_APIENTRY glRotatef(

    GLfloat Angle,

    GLfloat X,

    GLfloat Y,

    GLfloat Z

    )

{

    gcmDUMP_API("${ES11 glRotatef 0x%08X 0x%08X 0x%08X 0x%08X}",

        *(GLuint*)&Angle, *(GLuint*)&X, *(GLuint*)&Y, *(GLuint*)&Z);



    _Rotatef(Angle, X, Y, Z);

}



static void _Frustumf(

    GLfloat Left,

    GLfloat Right,

    GLfloat Bottom,

    GLfloat Top,

    GLfloat zNear,

    GLfloat zFar

    )

{

    glmENTER6(glmARGFLOAT, Left, glmARGFLOAT, Right, glmARGFLOAT, Bottom,

              glmARGFLOAT, Top, glmARGFLOAT, zNear, glmARGFLOAT, zFar)

    {

        glsMATRIX frustum, result;

        GLuint matrixID;



        glmPROFILE(context, GLES1_FRUSTUMF, 0);



        /* Verify arguments. */

        if ((Left == Right) ||

            (Bottom == Top) ||

            (zNear <= 0.0f) ||

            (zFar <= 0.0f)  ||

            (zNear == zFar))

        {

            /* Invalid value. */

            glmERROR(GL_INVALID_VALUE);

            break;

        }



        /* Fill in frustum matrix. */

        gcoOS_ZeroMemory(&frustum, sizeof(frustum));

        glmMATFLOAT(&frustum, 0, 0) =  2.0f * zNear        / (Right - Left);

        glmMATFLOAT(&frustum, 1, 1) =  2.0f * zNear        / (Top   - Bottom);

        glmMATFLOAT(&frustum, 2, 0) = (Right + Left)       / (Right - Left);

        glmMATFLOAT(&frustum, 2, 1) = (Top   + Bottom)     / (Top   - Bottom);

        glmMATFLOAT(&frustum, 2, 2) = (zNear + zFar)       / (zNear - zFar);

        glmMATFLOAT(&frustum, 2, 3) = -1.0f;

        glmMATFLOAT(&frustum, 3, 2) =  2.0f * zNear * zFar / (zNear - zFar);



        /* Multipy the current matrix by the frustum matrix. */

        _MultiplyMatrix4x4(context->currentMatrix, &frustum, &result);



        /* Replace the current matrix. */

        *context->currentMatrix = result;



        /* Notify dependents of the change. */

        matrixID = context->currentStack->matrixID;

        (*context->currentStack->dataChanged) (context, matrixID);

    }

    glmLEAVE();

}



/*******************************************************************************

**

**  glFrustum

**

**  glFrustum describes a perspective matrix that produces a perspective

**  projection. The current matrix (see glMatrixMode) is multiplied by this

**  matrix and the result replaces the current matrix, as if glMultMatrix were

**  called with the following matrix as its argument:

**

**      2 * near / (right - left)    0                            A    0

**      0                            2 * near / (top - bottom)    B    0

**      0                            0                            C    D

**      0                            0                           -1    0

**

**  where

**

**      A = (right + left)   / (right - left)

**      B = (top   + bottom) / (top   - bottom)

**      C = (near  + far)    / (near  - far)

**      D = 2 * near * far   / (near  - far)

**

**  Typically, the matrix mode is GL_PROJECTION, and (left, bottom, -near) and

**  (right, top, -near) specify the points on the near clipping plane that are

**  mapped to the lower left and upper right corners of the window, assuming

**  that the eye is located at (0, 0, 0). -far specifies the location of the

**  far clipping plane. Both near and far must be positive.

**

**  Use glPushMatrix and glPopMatrix to save and restore the current

**  matrix stack.

**

**  INPUT:

**

**      Left

**      Right

**          Specify the coordinates for the left and right vertical

**          clipping planes.

**

**      Bottom

**      Right

**          Specify the coordinates for the bottom and top horizontal

**          clipping planes.

**

**      zNear

**      zFar

**          Specify the distances to the near and far depth clipping planes.

**          Both distances must be positive.

**

**  OUTPUT:

**

**      Nothing.

*/

GL_API void GL_APIENTRY glFrustumx(

    GLfixed Left,

    GLfixed Right,

    GLfixed Bottom,

    GLfixed Top,

    GLfixed zNear,

    GLfixed zFar

    )

{

    GLfloat left, right, bottom, top, znear, zfar;



    gcmDUMP_API("${ES11 glFrustumx 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X}",

        Left, Right, Bottom, Top, zNear, zFar);



    left   = glmFIXED2FLOAT(Left);

    right  = glmFIXED2FLOAT(Right);

    bottom = glmFIXED2FLOAT(Bottom);

    top    = glmFIXED2FLOAT(Top);

    znear  = glmFIXED2FLOAT(zNear);

    zfar   = glmFIXED2FLOAT(zFar);



    _Frustumf(left, right, bottom, top, znear, zfar);

}



GL_API void GL_APIENTRY glFrustumxOES(

    GLfixed Left,

    GLfixed Right,

    GLfixed Bottom,

    GLfixed Top,

    GLfixed zNear,

    GLfixed zFar

    )

{

    GLfloat left, right, bottom, top, znear, zfar;



    gcmDUMP_API("${ES11 glFrustumxOES 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X}",

        Left, Right, Bottom, Top, zNear, zFar);



    left   = glmFIXED2FLOAT(Left);

    right  = glmFIXED2FLOAT(Right);

    bottom = glmFIXED2FLOAT(Bottom);

    top    = glmFIXED2FLOAT(Top);

    znear  = glmFIXED2FLOAT(zNear);

    zfar   = glmFIXED2FLOAT(zFar);



    _Frustumf(left, right, bottom, top, znear, zfar);

}



GL_API void GL_APIENTRY glFrustumf(

    GLfloat Left,

    GLfloat Right,

    GLfloat Bottom,

    GLfloat Top,

    GLfloat zNear,

    GLfloat zFar

    )

{

    gcmDUMP_API("${ES11 glFrustumf 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X}",

        *(GLuint*)&Left, *(GLuint*)&Right, *(GLuint*)&Bottom,

        *(GLuint*)&Top, *(GLuint*)&zNear, *(GLuint*)&zFar);



    _Frustumf(Left, Right, Bottom, Top, zNear, zFar);

}



GL_API void GL_APIENTRY glFrustumfOES(

    GLfloat Left,

    GLfloat Right,

    GLfloat Bottom,

    GLfloat Top,

    GLfloat zNear,

    GLfloat zFar

    )

{

    gcmDUMP_API("${ES11 glFrustumfOES 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X}",

        *(GLuint*)&Left, *(GLuint*)&Right, *(GLuint*)&Bottom,

        *(GLuint*)&Top, *(GLuint*)&zNear, *(GLuint*)&zFar);



    _Frustumf(Left, Right, Bottom, Top, zNear, zFar);

}



void _Scalef(

    GLfloat X,

    GLfloat Y,

    GLfloat Z

    )

{

    glmENTER3(glmARGFLOAT, X, glmARGFLOAT, Y, glmARGFLOAT, Z)

    {

        GLuint matrixID;



        glmPROFILE(context, GLES1_SCALEF, 0);

        /* Optimization: do nothing when scale is one. */

        if ((X == 1.0f) && (Y == 1.0f) && (Z == 1.0f))

        {

            break;

        }



        if (context->currentMatrix->identity)

        {

            /*  Optimized result with identity:



                [0,0] = X

                [1,1] = Y

                [2,2] = Z

            */

            glmMATFLOAT(context->currentMatrix, 0, 0) = X;

            glmMATFLOAT(context->currentMatrix, 1, 1) = Y;

            glmMATFLOAT(context->currentMatrix, 2, 2) = Z;

        }

        else

        {

            /*  Optimized result:

            [0,0] = [0,0] * X

            [1,0] = [1,0] * Y

            [2,0] = [2,0] * Z

            [0,1] = [0,1] * X

            [1,1] = [1,1] * Y

            [2,1] = [2,1] * Z

            [0,2] = [0,2] * X

            [1,2] = [1,2] * Y

            [2,2] = [2,2] * Z

            [0,3] = [0,3] * X

            [1,3] = [1,3] * Y

            [2,3] = [2,3] * Z

            */

            glmMATFLOAT(context->currentMatrix, 0, 0) *= X;

            glmMATFLOAT(context->currentMatrix, 0, 1) *= X;

            glmMATFLOAT(context->currentMatrix, 0, 2) *= X;

            glmMATFLOAT(context->currentMatrix, 0, 3) *= X;

            glmMATFLOAT(context->currentMatrix, 1, 0) *= Y;

            glmMATFLOAT(context->currentMatrix, 1, 1) *= Y;

            glmMATFLOAT(context->currentMatrix, 1, 2) *= Y;

            glmMATFLOAT(context->currentMatrix, 1, 3) *= Y;

            glmMATFLOAT(context->currentMatrix, 2, 0) *= Z;

            glmMATFLOAT(context->currentMatrix, 2, 1) *= Z;

            glmMATFLOAT(context->currentMatrix, 2, 2) *= Z;

            glmMATFLOAT(context->currentMatrix, 2, 3) *= Z;

        }



        context->currentMatrix->identity = GL_FALSE;



        /* Notify dependents of the change. */

        matrixID = context->currentStack->matrixID;

        (*context->currentStack->dataChanged) (context, matrixID);

    }

    glmLEAVE();

}





/*******************************************************************************

**

**  glScale

**

**  glScale produces a nonuniform scaling along the x, y, and z axes. The three

**  parameters indicate the desired scale factor along each of the three axes.

**

**  The current matrix (see glMatrixMode) is multiplied by this scale matrix,

**  and the product replaces the current matrix as if glScale were called with

**  the following matrix as its argument:

**

**      x   0   0   0

**      0   y   0   0

**      0   0   z   0

**      0   0   0   1

**

**  If the matrix mode is either GL_MODELVIEW or GL_PROJECTION, all objects

**  drawn after glScale is called are scaled.

**

**  Use glPushMatrix and glPopMatrix to save and restore the unscaled

**  coordinate system.

**

**  INPUT:

**

**      X

**      Y

**      Z

**          Specify scale factors along the x, y, and z axes, respectively.

**

**  OUTPUT:

**

**      Nothing.

*/

GL_API void GL_APIENTRY glScalex(

    GLfixed X,

    GLfixed Y,

    GLfixed Z

    )

{

    GLfloat x, y, z;



    gcmDUMP_API("${ES11 glScalex 0x%08X 0x%08X 0x%08X}", X, Y, Z);



    x = glmFIXED2FLOAT(X);

    y = glmFIXED2FLOAT(Y);

    z = glmFIXED2FLOAT(Z);



    _Scalef(x, y, z);

}



GL_API void GL_APIENTRY glScalexOES(

    GLfixed X,

    GLfixed Y,

    GLfixed Z

    )

{

    GLfloat x, y, z;



    gcmDUMP_API("${ES11 glScalexOES 0x%08X 0x%08X 0x%08X}", X, Y, Z);



    x = glmFIXED2FLOAT(X);

    y = glmFIXED2FLOAT(Y);

    z = glmFIXED2FLOAT(Z);



    _Scalef(x, y, z);

}



GL_API void GL_APIENTRY glScalef(

    GLfloat X,

    GLfloat Y,

    GLfloat Z

    )

{

    gcmDUMP_API("${ES11 glScalef 0x%08X 0x%08X 0x%08X}",

        *(GLuint*)&X, *(GLuint*)&Y, *(GLuint*)&Z);



    _Scalef(X, Y, Z);

}





/*******************************************************************************

**

**  glMultMatrix

**

**  glMultMatrix multiplies the current matrix with the one specified using

**  Matrix, and replaces the current matrix with the product.

**

**  The current matrix is determined by the current matrix mode

**  (see glMatrixMode). It is either the projection matrix, modelview matrix,

**  or the texture matrix.

**

**  INPUT:

**

**      Matrix

**          Points to 16 consecutive values that are used as the elements of

**          a 4 ? 4 column-major matrix.

**

**  OUTPUT:

**

**      Nothing.

*/

GL_API void GL_APIENTRY glMultMatrixx(

    const GLfixed* Matrix

    )

{

    glmENTER1(glmARGPTR, Matrix)

    {

        GLfloat matrixf[16];

        GLint i;

        glsMATRIX matrix, result;



        gcmDUMP_API("${ES11 glMultMatrixx (0x%08X)", Matrix);

        gcmDUMP_API_ARRAY(Matrix, 16);

        gcmDUMP_API("$}");



        glmPROFILE(context, GLES1_MULTMATRIXX, 0);



        /* convert to Float */

        for (i = 0; i < 16; i++)

        {

            matrixf[i] = glmFIXED2FLOAT(Matrix[i]);

        }



        /* Load the matrix. */

        if(_LoadMatrix(&matrix, matrixf))

        {

            /* Multipy the current matrix by the matrix. */

            if(_MultiplyMatrix4x4(context->currentMatrix, &matrix, &result))

            {

                GLuint matrixID;



                /* Replace the current matrix. */

                *context->currentMatrix = result;



                /* Notify dependents of the change. */

                matrixID = context->currentStack->matrixID;

                (*context->currentStack->dataChanged) (context, matrixID);

            }

        }

    }

    glmLEAVE();

}



GL_API void GL_APIENTRY glMultMatrixxOES(

    const GLfixed* Matrix

    )

{

    glmENTER1(glmARGPTR, Matrix)

    {

        GLfloat matrixf[16];

        GLint i;

        glsMATRIX matrix, result;



        gcmDUMP_API("${ES11 glMultMatrixxOES (0x%08X)", Matrix);

        gcmDUMP_API_ARRAY(Matrix, 16);

        gcmDUMP_API("$}");



        glmPROFILE(context, GLES1_MULTMATRIXX, 0);



        /* convert to Float */

        for (i = 0; i < 16; i++)

        {

            matrixf[i] = glmFIXED2FLOAT(Matrix[i]);

        }



        /* Load the matrix. */

        if(_LoadMatrix(&matrix, matrixf))

        {

            /* Multipy the current matrix by the matrix. */

            if(_MultiplyMatrix4x4(context->currentMatrix, &matrix, &result))

            {

                GLuint matrixID;



                /* Replace the current matrix. */

                *context->currentMatrix = result;



                /* Notify dependents of the change. */

                matrixID = context->currentStack->matrixID;

                (*context->currentStack->dataChanged) (context, matrixID);

            }

        }

    }

    glmLEAVE();

}



GL_API void GL_APIENTRY glMultMatrixf(

    const GLfloat* Matrix

    )

{

    glmENTER1(glmARGPTR, Matrix)

    {

        glsMATRIX matrix, result;



        gcmDUMP_API("${ES11 glMultMatrixf (0x%08X)", Matrix);

        gcmDUMP_API_ARRAY(Matrix, 16);

        gcmDUMP_API("$}");



        glmPROFILE(context, GLES1_MULTMATRIXF, 0);



        /* Load the matrix. */

        if(_LoadMatrix(&matrix, Matrix))

        {

            /* Multipy the current matrix by the matrix. */

            if(_MultiplyMatrix4x4(context->currentMatrix, &matrix, &result))

            {

                GLuint matrixID;



                /* Replace the current matrix. */

                *context->currentMatrix = result;



                /* Notify dependents of the change. */

                matrixID = context->currentStack->matrixID;

                (*context->currentStack->dataChanged) (context, matrixID);

            }

        }

    }

    glmLEAVE();

}





/*******************************************************************************

**

**  glPushMatrix/glPopMatrix

**

**  There is a stack of matrices for each of the matrix modes. In GL_MODELVIEW

**  mode, the stack depth is at least 16. In the other modes, GL_PROJECTION,

**  and GL_TEXTURE, the depth is at least 2. The current matrix in any mode

**  is the matrix on the top of the stack for that mode.

**

**  glPushMatrix pushes the current matrix stack down by one, duplicating

**  the current matrix. That is, after a glPushMatrix call, the matrix on top

**  of the stack is identical to the one below it.

**

**  glPopMatrix pops the current matrix stack, replacing the current matrix

**  with the one below it on the stack.

**

**  Initially, each of the stacks contains one matrix, an identity matrix.

**

**  It is an error to push a full matrix stack, or to pop a matrix stack that

**  contains only a single matrix. In either case, the error flag is set and

**  no other change is made to GL state.

**

**  INPUT:

**

**      Nothing.

**

**  OUTPUT:

**

**      Nothing.

*/

GL_API void GL_APIENTRY glPushMatrix(

    void

    )

{

    glmENTER()

    {

        gcmDUMP_API("${ES11 glPushMatrix}");



        glmPROFILE(context, GLES1_PUSHMATRIX, 0);

        /* Stack overflow? */

        if (context->currentStack->index == context->currentStack->count - 1)

        {

            /* Set error. */

            if (glmIS_SUCCESS(context->error))

            {

                glmERROR(GL_STACK_OVERFLOW);

            }

        }

        else

        {

            GLuint matrixID;



            /* Copy the previous matrix. */

            context->currentStack->topMatrix[1]

                = context->currentStack->topMatrix[0];



            /* Increment the stack pointer. */

            context->currentStack->index++;

            context->currentStack->topMatrix++;

            context->currentMatrix++;



            /* Notify dependents of the change. */

            matrixID = context->currentStack->matrixID;

            (*context->currentStack->currChanged) (context, matrixID);

        }

    }

    glmLEAVE();

}



GL_API void GL_APIENTRY glPopMatrix(

    void

    )

{

    glmENTER()

    {

        gcmDUMP_API("${ES11 glPopMatrix}");



        glmPROFILE(context, GLES1_POPMATRIX, 0);

        /* Already on the first matrix? */

        if (context->currentStack->index == 0)

        {

            /* Set error. */

            if (glmIS_SUCCESS(context->error))

            {

                glmERROR(GL_STACK_UNDERFLOW);

            }

        }

        else

        {

            GLuint matrixID;



            /* Decrement the stack pointer. */

            context->currentStack->index--;

            context->currentStack->topMatrix--;

            context->currentMatrix--;



            /* Notify dependents of the change. */

            matrixID = context->currentStack->matrixID;

            (*context->currentStack->currChanged) (context, matrixID);

            (*context->currentStack->dataChanged) (context, matrixID);

        }

    }

    glmLEAVE();

}





/*******************************************************************************

**

**  glCurrentPaletteMatrixOES (GL_OES_matrix_palette)

**

**  glCurrentPaletteMatrixOES defines which of the palette's matrices is

**  affected by subsequent matrix operations when the current matrix mode is

**  MATRIX_PALETTE_OES. glCurrentPaletteMatrixOES generates the error

**  INVALID_VALUE if the <MatrixIndex> parameter is not between 0 and

**  MAX_PALETTE_MATRICES_OES - 1.

**

**  INPUT:

**

**      MatrixIndex

**          Index of the palette matrix to set as current.

**

**  OUTPUT:

**

**      Nothing.

*/

#undef  _GC_OBJ_ZONE

#define _GC_OBJ_ZONE    gcdZONE_ES11_EXTENTION



GL_API void GL_APIENTRY glCurrentPaletteMatrixOES(

    GLuint MatrixIndex

    )

{

    glmENTER1(glmARGUINT, MatrixIndex)

    {

        gcmDUMP_API("${ES11 glCurrentPaletteMatrixOES 0x%08X}", MatrixIndex);



        if (MatrixIndex >= glvMAX_PALETTE_MATRICES)

        {

            glmERROR(GL_INVALID_VALUE);

            break;

        }



        /* Set the new current. */

        context->currentPalette = MatrixIndex;



        /* Are we currently in palette matrix mode? */

        if ((context->matrixMode >= glvPALETTE_MATRIX_0) &&

            (context->matrixMode <= glvPALETTE_MATRIX_LAST))

        {

            /* Update matrix mode. */

            context->matrixMode

                = (gleMATRIXMODE) (glvPALETTE_MATRIX_0

                + context->currentPalette);



            /* Update the stack. */

            context->currentStack = &context->matrixStackArray[context->matrixMode];

            context->currentMatrix = context->currentStack->topMatrix;

        }

    }

    glmLEAVE();

}





/*******************************************************************************

**

**  glLoadPaletteFromModelViewMatrixOES (GL_OES_matrix_palette)

**

**  glLoadPaletteFromModelViewMatrixOES copies the current model view matrix

**  to a matrix in the matrix palette, specified by CurrentPaletteMatrixOES.

**

**  INPUT:

**

**      Nothing.

**

**  OUTPUT:

**

**      Nothing.

*/

GL_API void GL_APIENTRY glLoadPaletteFromModelViewMatrixOES(

    void

    )

{

    glmENTER()

    {

        gctUINT index;

        GLuint matrixID;

        glsMATRIX_PTR matrix;



        gcmDUMP_API("${ES11 glLoadPaletteFromModelViewMatrixOES}");



        /* Determine the stack index. */

        index = glvPALETTE_MATRIX_0 + context->currentPalette;



        /* Get the top matrix. */

        matrix = context->matrixStackArray[index].topMatrix;



        /* Copy the current Model View. */

        gcoOS_MemCopy(

            matrix, context->modelViewMatrix, gcmSIZEOF(glsMATRIX)

            );



        /* Notify of the change. */

        matrixID = context->matrixStackArray[index].matrixID;

        (*context->matrixStackArray[index].dataChanged) (context, matrixID);

    }

    glmLEAVE();

}





/*******************************************************************************

**

**  glQueryMatrixxOES (GL_OES_query_matrix)

**

**  glQueryMatrixxOES returns the values of the current matrix. Mantissa returns

**  the 16 mantissa values of the current matrix, and Exponent returns the

**  correspnding 16 exponent values. The matrix value i is then close to

**  Mantissa[i] * 2 ** Exponent[i].

**

**  Use glMatrixMode and glActiveTexture, to select the desired matrix to

**  return.

**

**  If all are valid (not NaN or Inf), glQueryMatrixxOES returns the status

**  value 0. Otherwise, for every component i which is not valid, the ith bit

**  is set.

**

**  The implementation is not required to keep track of overflows. If overflows

**  are not tracked, the returned status value is always 0.

**

**  INPUT:

**

**      Mantissa

**          Returns the mantissi of the current matrix.

**

**      Exponent

**          Returns the exponents of the current matrix.

**

**  OUTPUT:

**

**      Nothing.

*/

GL_API GLbitfield GL_APIENTRY glQueryMatrixxOES(

    GLfixed* Mantissa,

    GLint* Exponent

    )

{

    GLbitfield valid = 0;

    int i;



    glmENTER2(glmARGPTR, Mantissa, glmARGPTR, Exponent)

    {

        for (i = 0; i < 16; i++)

        {

            GLbitfield bits = ((GLbitfield*) (context->currentMatrix->value))[i];

            GLfloat m = context->currentMatrix->value[i];



            if ((bits & 0x7f800000) == 0x7f800000)

            {

                /* IEEE754: INF, NaN, non-regular float value */

                valid |= (1 << i);



                continue;

            }



            Exponent[i] = 0;



            while (gcmABS(m) >= 32768.0f)

            {

                Exponent[i]++;

                m /= 2.0f;

            }



            Mantissa[i] = (GLfixed) (m * 65536.0f);

        }



        gcmDUMP_API("${ES11 glQueryMatrixxOES (0x%08X) (0x%08X)", Mantissa, Exponent);

        gcmDUMP_API_ARRAY(Mantissa, 16);

        gcmDUMP_API_ARRAY(Exponent, 16);

        gcmDUMP_API("$}");

    }

    glmLEAVE();



    return valid;

}

