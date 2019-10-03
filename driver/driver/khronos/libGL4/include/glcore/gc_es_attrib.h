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


#ifndef __gc_es_attrib_h__
#define __gc_es_attrib_h__

#ifdef OPENGL40
/*
** Current raster / window position state.
*/
typedef struct __GLRasterPosStateRec {
    __GLvertex  rPos;
    GLboolean    validRasterPos;
} __GLRasterPosState;

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
** Polygon stipple state.
*/
typedef struct __GLpolygonStippleStateRec {
    GLubyte stipple[4*32];
} __GLpolygonStippleState;

typedef struct __GLfogStateRec {
    GLenum mode;
    __GLcolor color;
    GLfloat density, start, end;
    GLfloat oneOverEMinusS;
    GLfloat index;
#ifdef __GL_LITTLE_ENDIAN
    union {
        GLuint composite;
        struct {
            GLubyte r, g, b, i; /* scaled ubyte copies of color, index */
        };
    };
#else
    union byteColor {
        GLuint composite;
        struct {
            GLubyte i, b, g, r; /* scaled ubyte copies of color, index */
        };
    };
#endif
    GLenum coordSource; /* FRAGMENT_DEPTH or FOG_COORDINATE */
} __GLfogState;

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
** All stackable list state.
*/
typedef struct __GLdlistStateRec {
    GLuint listBase;
} __GLdlistState;

/************************************************************************/

#define __GL_ATTRIB_POLYGON_OFFSET_FILL_BIT     (1 << 0)
#define __GL_ATTRIB_DEPTH_TEST_BIT              (1 << 1)
#define __GL_ATTRIB_DEPTH_MASK_BIT              (1 << 2)
#define __GL_ATTRIB_LINE_STIPPLE_BIT            (1 << 3)

#define __GL_ATTRIB_COLOR_MASK_R_BIT            (1 << 0)
#define __GL_ATTRIB_COLOR_MASK_G_BIT            (1 << 1)
#define __GL_ATTRIB_COLOR_MASK_B_BIT            (1 << 2)
#define __GL_ATTRIB_COLOR_MASK_A_BIT            (1 << 3)

/************************************************************************/

#endif
/*
** The current state for Vertex.
*/
typedef struct __GLcurrentStateRec
{
#ifdef OPENGL40
    union
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
                __GLcoord attribute[__GL_MAX_VERTEX_ATTRIBUTES];
            };

            __GLcoord currentState[__GL_TOTAL_VERTEX_ATTRIBUTES];
        };
    };
    GLfloat colorIndex;
#else
    __GLcoord attribute[__GL_MAX_VERTEX_ATTRIBUTES];
#endif
} __GLcurrentState;

/*
** Line state.  Contains all the client controllable line state.
*/
typedef struct __GLlineStateRec
{
    GLfloat  requestedWidth;
    GLint    aliasedWidth;
#ifdef OPENGL40
    GLfloat  smoothWidth;
    GLushort   stipple;
    GLshort    stippleRepeat;
#endif
} __GLlineState;

/*
** Polygon state.  Contains all the user controllable polygon state
*/
typedef struct __GLpolygonStateRec
{
    GLenum cullFace;
    GLenum frontFace;

    /* Polygon offset */
    GLfloat factor;
    GLfloat units;
#ifdef OPENGL40
    GLenum frontMode;
    GLenum backMode;
    GLuint bothFaceFill;
    GLfloat bias;
#endif
} __GLpolygonState;

/*
** Depth state.  Contains all the user settable depth state.
*/
typedef struct __GLdepthStateRec
{
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

    /*
    ** Depthrange parameters from user.
    */
    GLfloat zNear, zFar;
} __GLdepthState;

typedef struct __GLstencilFaceStateRec
{
    GLenum testFunc;
    GLint  reference;
    GLuint mask;
    GLenum fail;
    GLenum depthFail;
    GLenum depthPass;
    GLint writeMask;
} __GLstencilFaceState;


typedef struct __GLstencilStateRec
{
    __GLstencilFaceState front;
    __GLstencilFaceState back;
    GLint clear;
#ifdef OPENGL40
    __GLstencilFaceState StencilArbFront; /* For ARB spec 2.0 */
    __GLstencilFaceState StencilArbBACK; /* For ARB spec 2.0 */
    __GLstencilFaceState StencilEXTFront; /* For GL_EXT_stencil_two_side */
    __GLstencilFaceState StencilEXTBACK; /* For GL_EXT_stencil_two_side */
    /* Active stencil face (for GL_EXT_stencil_two_side only) */
    GLenum activeStencilFace;
#endif
}__GLstencilState;


/*
** Viewport state.
*/
typedef struct __GLviewportRec
{
    /* Viewport parameters from user, as integers. */
    GLint x, y;
    GLsizei width, height;
#ifdef OPENGL40
    /*
    ** Depthrange parameters from user.
    */
    GLfloat zNear, zFar;
    GLfloat farMinusNear, pack;
#endif
} __GLviewport;

/*
** Raster state.
*/
typedef struct __GLrasterStateRec
{
    /* Alpha blending source and destination factors, blend op,
    ** and blend color.
    */
    GLenum    blendEquationRGB[__GL_MAX_DRAW_BUFFERS];
    GLenum    blendEquationAlpha[__GL_MAX_DRAW_BUFFERS];

    GLenum    blendSrcRGB[__GL_MAX_DRAW_BUFFERS];
    GLenum    blendDstRGB[__GL_MAX_DRAW_BUFFERS];
    GLenum    blendSrcAlpha[__GL_MAX_DRAW_BUFFERS];
    GLenum    blendDstAlpha[__GL_MAX_DRAW_BUFFERS];

    __GLcolor blendColor;

    /*
    ** Color to fill the color portion of the framebuffer when clear
    ** is called.
    */
    union __GLclearColorUnion
    {
        __GLcolor    clear;
        __GLcolorIui clearIui;
        __GLcolorIi  clearIi;
    } clearColor;

    /*
    ** RGB write masks.  These booleans enable or disable writing of
    ** the r, g, b, and a components.
    */
    struct __GLcolorMaskRec
    {
        GLboolean redMask;
        GLboolean greenMask;
        GLboolean blueMask;
        GLboolean alphaMask;
    } colorMask[__GL_MAX_DRAW_BUFFERS];

    /* This state variable tracks which buffers are drawn into. */
    GLenum    drawBuffers[__GL_MAX_DRAW_BUFFERS];
    GLboolean mrtEnable;

    GLenum    readBuffer;
#ifdef OPENGL40
     /*
    ** Alpha function.  The alpha function is applied to the alpha color
    ** value and the reference value.  If it fails then the pixel is
    ** not rendered.
    */
    GLenum alphaFunction;
    GLfloat alphaReference;
    GLenum logicOp;

    GLfloat clearIndex;
    /*
    ** Color index write mask.  The color values are masked with this
    ** value when writing to the frame buffer so that only the bits set
    ** in the mask are changed in the frame buffer.
    */
    GLint writeMask;

    /*
    ** Color buffer clamp color control
    ** GL_CLAMP_FRAGMENT_COLOR_ARB
    ** GL_CLAMP_READ_COLOR_ARB
    */
    GLenum clampFragColor;
    GLenum clampReadColor;
#endif
} __GLrasterState;

/*
** Pixel state
*/
typedef struct __GLpixelPackModeRec
{
    GLuint alignment;
    GLuint lineLength;
    GLuint skipLines;
    GLuint skipPixels;
    GLuint skipImages;
    GLuint imageHeight;
#ifdef OPENGL40
    GLuint swapEndian;
    GLuint lsbFirst;
#endif
} __GLpixelPackMode;

typedef struct __GLclientPixelStateRec
{
    __GLpixelPackMode packModes;
    __GLpixelPackMode unpackModes;
#ifdef OPENGL40
    /* Buffer object binding points for PIXEL_PACK_BUFFER and PIXEL_UNPACK_BUFFER */
    GLuint  packBufBinding;
    GLuint  unpackBufBinding;
#endif
} __GLclientPixelState;

/*
** Hint state.  Contains all the user controllable hint state.
*/
typedef struct __GLhitStateRec
{
    GLenum generateMipmap;
    GLenum fsDerivative;
#ifdef OPENGL40
    GLenum perspectiveCorrection;
    GLenum pointSmooth;
    GLenum lineSmooth;
    GLenum polygonSmooth;
    GLenum fog;
    GLenum textureCompressionHint;
#endif
} __GLhintState;

/*
** Scissor state from user.
*/
typedef struct __GLscissorRec
{
    GLint scissorX;
    GLint scissorY;
    GLint scissorWidth;
    GLint scissorHeight;
} __GLscissor;

/*
** Multisample state from user.
*/
typedef struct __GLmultisampleStateRec {
    GLboolean coverageInvert;
    GLfloat   coverageValue;

    GLbitfield sampleMaskValue;
    GLfloat   minSampleShadingValue;

#ifdef OPENGL40
    GLboolean alphaToCoverage;
    GLboolean alphaToOne;
    GLboolean coverage;
#endif
} __GLmultisampleState;

typedef struct __GLcolorBufferStateRec
{
    GLboolean dither;
    GLboolean blend[__GL_MAX_DRAW_BUFFERS];
#ifdef OPENGL40
    GLboolean alphaTest;
    GLboolean logicOp;
    GLboolean colorLogicOp, indexLogicOp;
#endif
} __GLColorBufferEnableState;

typedef struct __GLpolygonEnableStateRec
{
    GLboolean cullFace;
    GLboolean polygonOffsetFill;
#ifdef OPENGL40
    GLboolean smooth;
    GLboolean stipple;
    GLboolean polygonOffsetPoint, polygonOffsetLine;
#endif
} __GLPolygonEnableState;

typedef struct __GLmultisampleEnableStateRec
{
    GLboolean alphaToCoverage;
    GLboolean coverage;
    GLboolean sampleMask;
    GLboolean sampleShading;
#ifdef OPENGL40
    GLboolean multisampleOn;
    GLboolean alphaToOne;
#endif
} __GLMultisampleEnableState;

typedef struct __GLenableStateRec
{
    __GLColorBufferEnableState colorBuffer;
    __GLPolygonEnableState     polygon;
    __GLMultisampleEnableState multisample;
    GLboolean                  scissorTest;
    GLboolean                  depthTest;
    GLboolean                  stencilTest;
    GLboolean                  primitiveRestart;
    GLboolean                  rasterizerDiscard;
#ifdef OPENGL40
    __GLTransformEnableState   transform;
    __GLLightingEnableState    lighting;
    __GLEvalEnableState        eval;
    __GLTextureEnableState     texUnits[__GL_MAX_TEXTURE_UNITS];
    __GLDepthEnableState       depthBuffer;
    __GLLineEnableState        line;
    __GLProgramEnableState     program;
    GLboolean                  pointSmooth;
    GLboolean                  fog;
    GLboolean                  scissor; //adopt the same name as OPENGLES
    GLboolean                  stencilTestTwoSideExt;
    GLboolean                  colorSum;
    GLboolean                  depthBoundTest; //adopt the same name as OPENGLES
    GLboolean                  pointSprite;
#endif
} __GLenableState;

typedef struct __GLprimBoundStateRec
{
    GLfloat minX;
    GLfloat minY;
    GLfloat minZ;
    GLfloat minW;
    GLfloat maxX;
    GLfloat maxY;
    GLfloat maxZ;
    GLfloat maxW;
} __GLprimBoundState;

typedef struct __GLPrimRestartStateRec
{
    GLuint restartElement;
}
__GLPrimRestartState;


/* Record the last samper2TexUnit mapping */
typedef struct __GLprogramStateRec
{
    GLuint sampler2TexUnit[__GL_MAX_GLSL_SAMPLERS];
} __GLprogramState;

/*
***********************************************************************
*/
typedef struct __GLattributeRec {
    __GLcurrentState current;
    __GLlineState line;
    __GLpolygonState polygon;
    __GLdepthState depth;
    __GLstencilState stencil;
    __GLviewport viewport;
    __GLenableState enables;
    __GLrasterState raster;
    __GLhintState hints;
    __GLscissor scissor;
    __GLmultisampleState multisample;
    __GLtextureState texture;
    __GLprogramState program;
    __GLimageState image;
    __GLprimBoundState primBound;
    __GLPrimRestartState primRestart;
#ifdef OPENGL40
    __GLRasterPosState rasterPos;
    __GLpointState point;
    __GLpolygonStippleState polygonStipple;
    __GLpixelState pixel;
    __GLlightState light;
    __GLfogState fog;
    __GLdepthBoundTestState depthBoundTest;
    __GLaccumState accum;
    __GLtransformState transform;
    __GLevaluatorState evaluator;
    __GLdlistState list;
#endif
} __GLattribute;

typedef struct __GLclientAttributeRec
{
    __GLclientPixelState pixel;
#ifdef OPENGL40
    __GLvertexArrayState vertexArray;
#endif
} __GLclientAttribute;

#ifdef OPENGL40
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
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern GLvoid __glOverturnCommitStates(__GLcontext *gc);
extern GLvoid __glSetAttributeStatesDirty(__GLcontext *gc);

#ifdef OPENGL40
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
extern GLvoid __glEvaluateAttributeChange(__GLcontext *gc);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __gc_es_attrib_h__ */
