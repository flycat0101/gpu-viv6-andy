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


#ifndef __gc_gl_attrib_h_
#define __gc_gl_attrib_h_

/*
** Current raster / window position state.
*/
typedef struct __GLRasterPosStateRec {
    __GLvertex  rPos;
    GLboolean    validRasterPos;
} __GLRasterPosState;


/*
** The Current state for Vertex.
*/
typedef struct __GLcurrentStateRec
{
    union
    {
        struct
        {
            __GLcoord vertex;
            GLfloat   weight[__GL_MAX_WEIGHT_SUPPORT];
            __GLcoord normal;
            __GLcolor color;
            __GLcolor color2;
            GLfloat   fog;
            GLfloat   padding1[3];
            GLboolean edgeflag;
            GLubyte   padding2[15];
            __GLcoord filler;
            __GLcoord texture[__GL_MAX_TEXTURE_COORDS];
            __GLcoord attribute[__GL_MAX_PROGRAM_VERTEX_ATTRIBUTES];
        };
        __GLcoord currentState[__GL_TOTAL_VERTEX_ATTRIBUTES];
    };
    GLfloat colorIndex;

} __GLcurrentState;

/*
** Point state.  Contains all the client controllable point state.
*/
typedef struct __GLpointStateRec {
    GLfloat requestedSize;
    GLfloat sizeMin;
    GLfloat sizeMax;
    GLfloat fadeThresholdSize;
    GLfloat distanceAttenuation[3];
    /* point sprite coordinate origin */
    GLenum coordOrigin;
} __GLpointState;

/*
** Line state.  Contains all the client controllable line state.
*/
typedef struct __GLlineStateRec {
    GLfloat  requestedWidth;
    GLfloat  smoothWidth;
    GLint    aliasedWidth;
    GLushort   stipple;
    GLshort    stippleRepeat;
} __GLlineState;


/*
** Polygon state.  Contains all the user controllable polygon state
** except for the stipple state.
*/
typedef struct __GLpolygonStateRec {
    GLenum frontMode;
    GLenum backMode;
    GLuint bothFaceFill;

    /*
    ** Culling state.  Culling can be enabled/disabled and set to cull
    ** front or back faces.  The FrontFace call determines whether clockwise
    ** or counter-clockwise oriented vertices are front facing.
    */
    GLenum cullFace;
    GLenum frontFace;

    /*
    ** Polygon offset state.
    */
    GLfloat factor;
    GLfloat units;
    GLfloat bias;
} __GLpolygonState;


/*
** Polygon stipple state.
*/
typedef struct __GLpolygonStippleStateRec {
    GLubyte stipple[4*32];
} __GLpolygonStippleState;


/*
** Fog state.
*/
typedef struct __GLfogStateRec {
    GLenum mode;
    __GLcolor color;
    GLfloat density, start, end;
    GLfloat oneOverEMinusS;
    GLfloat index;
#if defined(USE_LENDIAN)
    union {
        GLuint composite;
        struct {
            GLubyte r, g, b, i; /* scaled ubyte copies of color, index */
        };
    };
#else
    union {
        GLuint composite;
        struct {
            GLubyte i, b, g, r; /* scaled ubyte copies of color, index */
        };
    };
#endif
    GLenum coordSource; /* FRAGMENT_DEPTH or FOG_COORDINATE */
} __GLfogState;


/*
** Depth state.  Contains all the user settable depth state.
*/
typedef struct __GLdepthStateRec {
    /*
    ** Depth buffer test function.  The z value is compared using zFunction
    ** against the current value in the zbuffer.  If the comparison
    ** succeeds the new z value is written into the z buffer masked
    ** by the z write mask.
    */
    GLenum testFunc;

    /*
    ** Writemask enable.  When GL_TRUE writing to the depth buffer is
    ** allowed.
    */
    GLboolean writeEnable;

    /*
    ** Value used to clear the z buffer when glClear is called.
    */
    GLfloat clear;
} __GLdepthState;


/*
** Depth bound test state
*/
typedef struct __GLdepthBoundTestStateRec {
    GLfloat zMin;
    GLfloat zMax;
} __GLdepthBoundTestState;

/*
** Accum state.
*/
typedef struct __GLaccumStateRec {
    __GLcolor clear;
} __GLaccumState;


typedef struct __GLstencilFaceStateRec {
    /*
    ** Stencil test function.  When the stencil is enabled this
    ** function is applied to the reference value and the stored stencil
    ** value as follows:
    **          result = ref comparision (mask & stencilBuffer[x][y])
    ** If the test fails then the fail op is applied and rendering of
    ** the pixel stops.
    */
    GLenum testFunc;

    /*
    ** Reference stencil value.
    */
    GLint  reference;

    /*
    ** Stencil mask.  This is anded against the contents of the stencil
    ** buffer during comparisons.
    */
    GLuint mask;

    /*
    ** When the stencil comparison fails this operation is applied to
    ** the stencil buffer.
    */
    GLenum fail;

    /*
    ** When the stencil comparison passes and the depth test
    ** fails this operation is applied to the stencil buffer.
    */
    GLenum depthFail;

    /*
    ** When both the stencil comparison passes and the depth test
    ** passes this operation is applied to the stencil buffer.
    */
    GLenum depthPass;

    /*
    ** Stencil write mask
    */
    GLint writeMask;

} __GLstencilFaceState;

/*
** Stencil state.  Contains all the user settable stencil state.
*/
typedef struct __GLstencilFuncStateRec {

    /* Front stencil state.
    */
    __GLstencilFaceState front;

    /* Back stencil state.
    */
    __GLstencilFaceState back;
} __GLstencilFuncState;

typedef struct __GLstencilStateRec {
    __GLstencilFuncState current;    /* The current stencil state */
    __GLstencilFuncState StencilArb; /* For ARB spec 2.0 */
    __GLstencilFuncState stencilExt; /* For GL_EXT_stencil_two_side */

    /*
    ** Stencil clear value.  Used by glClear.
    */
    GLint clear;

    /* Active stencil face (for GL_EXT_stencil_two_side only) */
    GLenum activeStencilFace;
}__GLstencilState;

/*
** Viewport state.
*/
typedef struct __GLviewportRec {
    /*
    ** Viewport parameters from user, as integers.
    */
    GLint x, y;
    GLsizei width, height;

    /*
    ** Depthrange parameters from user.
    */
    GLfloat zNear, zFar;

    GLfloat farMinusNear, pack;
} __GLviewport;


/*
** Transform state.
*/
typedef struct __GLtransformStateRec {
    /*
    ** Current mode of the matrix stack.  This determines what effect
    ** the various matrix operations (load, mult, scale) apply to.
    */
    GLenum matrixMode;

    /*
    ** User clipping planes in eye space.  These are the user clip planes
    ** projected into eye space.
    */
   __GLcoord eyeClipPlanes[__GL_MAX_CLIPPLANE_NUMBER];
} __GLtransformState;

/*
** Raster state.
*/
typedef struct __GLrasterStateRec {
    /*
    ** Alpha function.  The alpha function is applied to the alpha color
    ** value and the reference value.  If it fails then the pixel is
    ** not rendered.
    */
    GLenum alphaFunction;
    GLfloat alphaReference;

    /*
    ** Alpha blending source and destination factors, blend op,
    ** and blend color.
    */
    GLenum blendEquationRGB;
      GLenum blendEquationAlpha;

      GLenum blendSrcRGB;
    GLenum blendDstRGB;
      GLenum blendSrcAlpha;
      GLenum blendDstAlpha;

      __GLcolor blendColor;

    /*
    ** Index function.  This is part of the GL_SGI_index_func extension.
    ** It works just like the alpha test but for color indexes instead.
    **/
    GLenum indexFunction;
    GLfloat indexReference;

    /*
    ** Logic op.
    */
    GLenum logicOp;

    /*
    ** Color to fill the color portion of the framebuffer when clear
    ** is called.
    */
    union {
        __GLcolor    clear;
        __GLcolorIui clearIui;
        __GLcolorIi  clearIi;
    };

    GLfloat clearIndex;

    /*
    ** Color index write mask.  The color values are masked with this
    ** value when writing to the frame buffer so that only the bits set
    ** in the mask are changed in the frame buffer.
    */
    GLint writeMask;

    /*
    ** RGB write masks.  These booleans enable or disable writing of
    ** the r, g, b, and a components.
    */
#if defined(USE_LENDIAN)
    union
    {
        GLuint compositeMask;
        struct __colorMask
        {
            GLboolean redMask;
            GLboolean greenMask;
            GLboolean blueMask;
            GLboolean alphaMask;
        }colorMask[__GL_MAX_DRAW_BUFFERS];
    };
#else
    union
    {
        GLuint compositeMask;
        struct __colorMask
        {
            GLboolean alphaMask;
            GLboolean blueMask;
            GLboolean greenMask;
            GLboolean redMask;
        }colorMask[__GL_MAX_DRAW_BUFFERS];
    };
#endif

    /*
    ** Color buffer clamp color control
    ** GL_CLAMP_FRAGMENT_COLOR_ARB
    ** GL_CLAMP_READ_COLOR_ARB
    */
    GLenum clampFragColor;
    GLenum clampReadColor;

    /*
    ** This state variable tracks which buffer(s) is being drawn into.
    */
    GLenum drawBuffer[__GL_MAX_DRAW_BUFFERS];
    GLboolean  mrtEnable;

    /*
    ** Draw buffer specified by user.  May be different from drawBuffer
    ** above.  If the user specifies GL_FRONT_LEFT, for example, then
    ** drawBuffer is set to GL_FRONT, and drawBufferReturn to
    ** GL_FRONT_LEFT.
    */
    GLenum drawBufferReturn;
} __GLrasterState;


/*
** Hint state.  Contains all the user controllable hint state.
*/
typedef struct {
    GLenum perspectiveCorrection;
    GLenum pointSmooth;
    GLenum lineSmooth;
    GLenum polygonSmooth;
    GLenum fog;
    GLenum generateMipmap;
    GLenum textureCompressionHint;
} __GLhintState;


/*
** All stackable list state.
*/
typedef struct __GLdlistStateRec {
    GLuint listBase;
} __GLdlistState;


/*
** Scissor state from user.
*/
typedef struct __GLscissorRec {
    GLint scissorX, scissorY, scissorWidth, scissorHeight;
} __GLscissor;


/*
** Multisample state from user.
*/
typedef struct __GLmultisampleStateRec {
    GLboolean alphaToCoverage;
    GLboolean alphaToOne;
    GLboolean coverage;
    GLboolean coverageInvert;
    GLfloat   coverageValue;
} __GLmultisampleState;


/************************************************************************/

#define __GL_ATTRIB_POLYGON_OFFSET_FILL_BIT     (1 << 0)
#define __GL_ATTRIB_DEPTH_TEST_BIT              (1 << 1)
#define __GL_ATTRIB_DEPTH_MASK_BIT              (1 << 2)
#define __GL_ATTRIB_LINE_STIPPLE_BIT            (1 << 3)

#define __GL_ATTRIB_COLOR_MASK_R_BIT            (1 << 0)
#define __GL_ATTRIB_COLOR_MASK_G_BIT            (1 << 1)
#define __GL_ATTRIB_COLOR_MASK_B_BIT            (1 << 2)
#define __GL_ATTRIB_COLOR_MASK_A_BIT            (1 << 3)

typedef struct __GLattributeRec {
    __GLcurrentState current;
    __GLRasterPosState rasterPos;
    __GLpointState point;
    __GLlineState line;
    __GLpolygonState polygon;
    __GLpolygonStippleState polygonStipple;
    __GLpixelState pixel;
    __GLlightState light;
    __GLfogState fog;
    __GLdepthState depth;
    __GLdepthBoundTestState depthBoundTest;
    __GLaccumState accum;
      __GLstencilState stencil;
    __GLviewport viewport;
    __GLtransformState transform;
    __GLenableState enables;
    __GLrasterState raster;
    __GLhintState hints;
    __GLevaluatorState evaluator;
    __GLdlistState list;
    __GLscissor scissor;
    __GLmultisampleState multisample;
    __GLtextureState texture;
} __GLattribute;

typedef struct __GLclientAttributeRec {
    __GLclientPixelState pixel;
    __GLvertexArrayState vertexArray;
} __GLclientAttribute;

/************************************************************************/

typedef struct __GLattributeStackRec
{
    __GLattribute state;
    GLuint mask;
} __GLattributeStack;

typedef struct __GLclientAttribStackRec {
    __GLclientAttribute clientState;
    GLuint mask;
} __GLclientAttribStack;


/*
** Attribute machine state.  This manages the stack of attributes.
*/
typedef struct {
    /*
    ** Attribute stack.  The attribute stack keeps track of the
    ** attributes that have been pushed.
    */
    __GLattributeStack **stack;
    __GLclientAttribStack **clientStack;

    /*
    ** Attribute stack pointer.
    */
    __GLattributeStack **stackPointer;
    __GLclientAttribStack **clientStackPointer;
} __GLattributeMachine;

#ifdef __cplusplus
extern "C" {
#endif

extern GLvoid __glInitCurrentState(__GLcontext *gc);
extern GLvoid __glInitAttribStackState(__GLcontext *gc);
extern GLvoid __glInitHintState(__GLcontext *gc);
extern GLvoid __glInitRasterState(__GLcontext *gc);
extern GLvoid __glInitStencilState(__GLcontext *gc);
extern GLvoid __glInitDepthState(__GLcontext *gc);
extern GLvoid __glInitTransformState(__GLcontext *gc);
extern GLvoid __glInitFeedback(__GLcontext *gc);
extern GLvoid __glInitSelect(__GLcontext *gc);
extern GLvoid __glInitFogState(__GLcontext *gc);
extern GLvoid __glInitLightState(__GLcontext *gc);
extern GLvoid __glInitPointState(__GLcontext *gc);
extern GLvoid __glInitLineState(__GLcontext *gc);
extern GLvoid __glInitPolygonState(__GLcontext *gc);
extern GLvoid __glInitEvaluatorState(__GLcontext *gc);
extern GLvoid __glInitVertexArrayState(__GLcontext *gc);
extern GLvoid __glInitPixelState(__GLcontext *gc);
extern GLvoid __glInitAccumState(__GLcontext *gc);
extern GLvoid __glInitMultisampleState(__GLcontext *gc);
extern GLvoid __glInitDlistState(__GLcontext *gc);
extern GLvoid __glInitTextureState(__GLcontext *gc);
extern GLvoid __glInitBufferObjectState(__GLcontext *gc);
extern GLvoid __glInitProgramState(__GLcontext *gc);
extern GLvoid __glInitShaderProgramState(__GLcontext *gc);
extern GLvoid __glInitQueryState(__GLcontext *gc);
extern GLvoid __glSetAttributeStatesDirty(__GLcontext *gc);
extern GLvoid __glEvaluateAttributeChange(__GLcontext *gc);
#ifdef __cplusplus
}
#endif

#endif /* __gc_gl_attrib_h_ */
