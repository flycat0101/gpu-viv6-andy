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


#ifndef __gc_gl_transform_h_
#define __gc_gl_transform_h_

/*
** Note:
**
** Other code assumes that all types >= __GL_MT_IS2D are also 2D
** Other code assumes that all types >= __GL_MT_W0001 are also W0001
** Other code assumes that all types >= __GL_MT_IS2DNR are also 2DNR
**
*/
#define __GL_MT_GENERAL     0   /* No information */
#define __GL_MT_W0001       1   /* W row looks like 0 0 0 1 */
#define __GL_MT_IS2D        2   /* 2D matrix */
#define __GL_MT_IS2DNR      3   /* 2D non-rotational matrix */
#define __GL_MT_IDENTITY    4   /* Identity */
#define __GL_MT_IS2DNRSC    5   /* Screen coords, subset of 2DNR */

typedef struct __GLmatrixRec {
    GLfloat matrix[4][4];

    /*
    ** matrixType set to general if nothing is known about this matrix.
    **
    ** matrixType set to __GL_MT_W0001 if it looks like this:
    ** | . . . 0 |
    ** | . . . 0 |
    ** | . . . 0 |
    ** | . . . 1 |
    **
    ** matrixType set to __GL_MT_IS2D if it looks like this:
    ** | . . 0 0 |
    ** | . . 0 0 |
    ** | 0 0 . 0 |
    ** | . . . 1 |
    **
    ** matrixType set to __GL_MT_IS2DNR if it looks like this:
    ** | . 0 0 0 |
    ** | 0 . 0 0 |
    ** | 0 0 . 0 |
    ** | . . . 1 |
    **
    ** 2DNRSC matrixes are difficult to deal with properly, but they
    ** are nicely efficient, so we try anyway.  If a matrix is marked as
    ** 2DNRSC, it must be verified before it can be believed.  In order to
    ** verify it, you must check that the viewport starts at (0,0), and that
    ** the viewport width and height matches the width and height associated
    ** (below) with the matrix.
    **
    ** matrixType set to __GL_MT_IS2DNRSC if it looks like this:
    ** | 2/W   0   0   0 |
    ** |   0 2/H   0   0 |
    ** |   0   0   .   0 |
    ** |  -1  -1   .   1 |
    **
    ** Note that the matrix type pickers are incremental.  The matrix
    ** may be marked as __GL_MT_W001, for example, even if it also
    ** happens to be __GL_MT_IS2D (as long as the user does not attempt
    ** to confuse OpenGL, everything is picked quickly and efficiently).
    **
    ** 2DNRSC matrixes also guarantee that a z of zero will not get clipped.
    */
    GLenum matrixType;

} __GLmatrix;

/*
** Transform struct.  This structure is what the matrix stacks are
** composed of.  inverseTranspose contains the inverse transpose of matrix.
** For the modelView stack, "mvp" will contain the concatenation of
** the modelView and current projection matrix (i.e. the multiplication of
** the two matricies).
*/
typedef struct __GLtransformRec {
    __GLmatrix matrix;
    __GLmatrix inverseTranspose;
    __GLmatrix mvp;

    /* Used to reverse xform coordinates. */
    __GLmatrix inverse;         /* reverse xforms positional coords */
    __GLmatrix transpose;       /* reverse xforms directional coords */

    /* Sequence number tag for mvp */
    GLuint sequence;
    GLboolean updateInverse;
} __GLtransform;

/*
** Matrix handling function pointers. These functions are used to compute a
** matrix, not to use a matrix for computations.
*/
typedef struct __GLmatrixProcsRec {
    GLvoid (*copy)(__GLmatrix *d, const __GLmatrix *s);
    GLvoid (*invertTranspose)(__GLmatrix *d, const __GLmatrix *s);
    GLvoid (*makeIdentity)(__GLmatrix *d);
    GLvoid (*mult)(__GLmatrix *d, const __GLmatrix *a, const __GLmatrix *b);
} __GLmatrixProcs;

/*
** Transformation machinery state.  Contains the state needed to transform
** user coordinates into eye & window coordinates.
*/
typedef struct __GLtransformMachineRec {
    /*
    ** Transformation stack.  "modelView" points to the active element in
    ** the stack.
    */
    __GLtransform *modelViewStack;
    __GLtransform *modelView;

    /*
    ** Current projection matrix.  Used to transform eye coordinates into
    ** NTVP (or clip) coordinates.
    */
    __GLtransform *projectionStack;
    __GLtransform *projection;
    GLuint projectionSequence;

    /*
    ** Texture matrix stack.
    */
    __GLtransform *textureStack[__GL_MAX_TEXTURE_UNITS];
    __GLtransform *texture[__GL_MAX_TEXTURE_UNITS];

    /*
    ** Program matrix stack.
    */
    __GLtransform *programStack[__GL_MAX_PROGRAM_MATRICES];
    __GLtransform *program[__GL_MAX_PROGRAM_MATRICES];

    /*
    ** The smallest rectangle that is the intersection of the window clip
    ** and the scissor clip.  If the scissor box is disabled then this
    ** is just the window box. Note that the x0,y0 point is inside the
    ** box but that the x1,y1 point is just outside the box.
    */
    GLint clipX0;
    GLint clipY0;
    GLint clipX1;
    GLint clipY1;
    /* Seq Number used for sync clipbox from drawable */
    GLuint clipSeqNum;

    /* Proces used to operate on individual matricies */
    __GLmatrixProcs matrix;

    /* Matrix stack operations */
    GLvoid (*pushMatrix)(__GLcontext *gc);
    GLvoid (*popMatrix)(__GLcontext *gc);
    GLvoid (*loadIdentity)(__GLcontext *gc);

} __GLtransformMachine;

#ifdef __cplusplus
extern "C" {
#endif

extern GLvoid __glNormalize(GLfloat vout[3], const GLfloat v[3]);
extern GLvoid __glNormalizeTwo(const GLfloat v[3], GLfloat voa[3], GLfloat vob[3]);
extern GLvoid __glMultMatrix(__GLmatrix *r, const __GLmatrix *a, const __GLmatrix *b);
extern GLvoid __glCopyMatrix(__GLmatrix *dst, const __GLmatrix *src);
extern GLvoid __glMakeIdentity(__GLmatrix *m);
extern GLvoid __glInvertTransposeMatrix(__GLmatrix *inverse, const __GLmatrix *src);
extern GLvoid __glTransposeMatrix(__GLmatrix *transpose, const __GLmatrix *src);
extern GLvoid __glTransformCoord(__GLcoord *res, __GLcoord *v, __GLmatrix *m);
extern GLvoid __glTransformVector(__GLcontext *gc, __GLcoord *eyeDir, __GLcoord *mvDir, __GLtransform *tr, GLboolean);
extern GLvoid __glComputeClipBox(__GLcontext *gc);
#ifdef __cplusplus
}
#endif

#endif /* __gc_gl_transform_h_ */
