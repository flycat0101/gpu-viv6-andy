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


#ifndef __gc_gl_texture_h_
#define __gc_gl_texture_h_

#include "gc_gl_context.h"

/*
** Maximum linear table size for texture object.
*/
#define __GL_MAX_TEXOBJ_LINEAR_TABLE_SIZE       8192

/*
** Default linear table size for texture object.
*/
#define __GL_DEFAULT_TEXOBJ_LINEAR_TABLE_SIZE   1024

/*
** Maximum hash table size for texture object. Must be 2^n.
*/
#define __GL_TEXOBJ_HASH_TABLE_SIZE                8192

/*texture binding index*/
enum __GL_TEXTURE_BINDING_INDEX
{
    __GL_TEXTURE_1D_INDEX = 0,
    __GL_TEXTURE_2D_INDEX,
    __GL_TEXTURE_3D_INDEX,
    __GL_TEXTURE_CUBEMAP_INDEX,
    __GL_TEXTURE_RECTANGLE_INDEX,
    __GL_TEXTURE_1D_ARRAY_INDEX,
    __GL_TEXTURE_2D_ARRAY_INDEX,
    __GL_TEXTURE_BUFFER_INDEX,

    /*number of texture bindings*/
    __GL_MAX_TEXTURE_BINDINGS,
};


#define __GL_TEXTURE_ENABLED_MASK    0xff

#define __GL_NULL_COLORBUFFER        0xffffffff

/* Bit masks for "flag" variable in __GLtextureObject.
** Note: Bit 0~7 are reserved for generic object management.
*/
#define __GL_TEXTURE_IS_CHECKED                    (1 << 8)
#define __GL_TEXTURE_IS_CONSISTENT                 (1 << 9)
#define __GL_TEXTURE_FOR_DELETE                    (1 << 10)

/*macro to enable/disable texture consistency check*/
#define ENABLE_TEXTURE_CONSISTENCY_CHECK(tex) \
    (tex)->flag &= ~(__GL_TEXTURE_IS_CHECKED | __GL_TEXTURE_IS_CONSISTENT)
#define DISABLE_TEXTURE_CONSISTENCY_CHECK(tex) \
    (tex)->flag |= (__GL_TEXTURE_IS_CHECKED | __GL_TEXTURE_IS_CONSISTENT);

/*macro for texture_image_uptodate*/
#define GL_TEX_IMAGE_UPTODATE(tex, level) \
    tex->imageUpToDate |= (1<<(level))

#define GL_TEX_IMAGE_OUTDATED(tex, level) \
    tex->imageUpToDate &= ~(1<<(level))

#define GL_TEX_IMAGE_IS_UPTODATE(tex, level) \
    (tex->imageUpToDate & (1<<(level)))

#define GL_TEX_IMAGE_IS_ANY_LEVEL_DIRTY( tex, baseLevel, maxLevel) \
    ((~(tex->imageUpToDate)) & ( (1 << (maxLevel + 1)) - (1 << baseLevel)))


typedef GLubyte __GLtextureBuffer;

struct __GLtexelRec
{
    GLubyte r, g, b, a;
};


/*
** Client state set with glTexGen
*/
typedef struct __GLtextureCoordStateRec
{
    /* How coordinates are being generated */
    GLenum mode;

    /* eye plane equation (used iff mode == GL_EYE_LINEAR) */
    __GLcoord eyePlaneEquation;

    /* object plane equation (used iff mode == GL_OBJECT_LINEAR) */
    __GLcoord objectPlaneEquation;
} __GLtextureCoordState;

/*
** Client state set with glTexEnv
*/
typedef struct __GL_texture_env_params
{
    GLenum rgb;
    GLenum alpha;
} __GLtextureEnvParams;


typedef struct __GLtextureEnvStateRec
{
    /* environment "blend" function */
    GLenum mode;

    /* environment color */
    __GLcolor color;

    /* environment "combine" function */
    __GLtextureEnvParams function;
    __GLtextureEnvParams source[3];
    __GLtextureEnvParams operand[3];
    GLfloat rgbScale, alphaScale;
    GLboolean coordReplace;

} __GLtextureEnvState;

/*
** Client state set with glTexParameter
*/
typedef struct __GLtextureParamStateRec
{
    /* wrap modes */
    GLenum sWrapMode;
    GLenum tWrapMode;
    GLenum rWrapMode;

    /* min and mag filter */
    GLenum minFilter;
    GLenum magFilter;

    /* border color */
    union{
        __GLcolor    borderColor;
        __GLcolorIi  borderColorIi;
        __GLcolorIui borderColorIui;
    };

    /*
    ** Border Color which has been converted according to InternalFormat
    */
    union{
        __GLcolor     InternalBorderColor;
        __GLcolorIi   InternalBorderColorIi;
        __GLcolorIui  InternalBorderColorIui;
    };

    /* priority of the texture */
    GLfloat priority;

    /* texture lods */
    GLfloat    minLod;
    GLfloat    maxLod;

    /* texture levels */
    GLint baseLevel;
    GLint maxLevel;

    /* anisotropic extension */
    GLfloat    anisotropicLimit;

    /* mipmap generation */
    GLboolean generateMipmap;

    /* depth texture */
    GLenum depthTexMode;

    /* shadow extension */
    GLenum compareMode;
    GLenum compareFunc;

    /* shadow extension */
    GLfloat compareFailValue;

    /* texture LOD bias */
    GLfloat    lodBias;

    /* shadowAmbient */
      GLfloat shadowAmbient;

} __GLtextureParamState;

/*
** Mipmap level information for texture object
*/
struct __GLmipMapLevelRec
{
    /* Image dimensions, including border */
    GLint width, height, depth;

    //GLint imageSize;/*used by 3D texture sw rasterization*/

    /* Image dimensions, doesn't include border */
    GLint   width2, height2, depth2;
    GLfloat width2f, height2f, depth2f;

    /* log2 of width2 & height2 */
    GLint widthLog2, heightLog2, depthLog2;

    /* Border size */
    GLint border;

    /* Array size */
    GLint arrays;

    GLboolean compressed;   /* this is just a flag telling whether the requestedFormat is compressed,
                               we could choose not to compress the texture image */
    GLsizei compressedSize; /* the buffer size of image data of compressed texture */

    /* Requested internal format */
    GLint requestedFormat;

    /* Base internal format */
    GLenum baseFormat;

    /* parameters from the texImage */
    GLenum format;
    GLenum type;

    const __GLdeviceFormatInfo * deviceFormat;/*dp texture format*/

    /* The seqNumber is increased by 1 whenever this mipmap information is changed.
    ** DP must update its internal mipmap information if its internal copy of
    ** seqNumber is different from this seqNumber.
    */
    GLuint seqNumber;
};

/*
** Texture object structure.
*/
struct __GLtextureObjectRec
{
    /* indicate how many targets the object is currently bound to
    */
    GLuint bindCount;


    /* List of FBO this texture object attached to
    */
    __GLimageUser *fboList;

    /* List of texture units this texture object has EVER or NOW been bound to, that is,
    ** once this texture object is bound to an active texture unit, no matter whether it
    ** is unbound from the active texture unit in the later time, this active texture
    ** unit is always in the texUnitBoundList.
    */
    __GLimageUser *texUnitBoundList;

    /* The seqNumber is increased by 1 whenever object states are changed.
    ** DP must update its internal object states if its internal copy of
    ** savedSeqNum is different from this seqNumber.
    */
    GLuint seqNumber;

    /* Internal flag for generic object management and texture object consistency check. */
    GLbitfield flag;

    /* This is the object privateData that can be shared by different contexts
    ** that are bound to the object.
    */
    GLvoid *privateData;

#if SWP_PIPELINE_ENABLED
    /* Pointer to swp specific private data */
    GLvoid *swpPrivateData;
#endif

    GLuint name;
    GLuint targetIndex;

    /* Texture parameter state (set with glTexParameter) */
    __GLtextureParamState params;

    /* ppMipMap could be seen as a 2-dimensional array of mipmap.The usage of this member to get a
    ** __GLmipMapLevel is: faceMipmap[face][level]. for none-cubemap textures, face will always be 0.
    */
    __GLmipMapLevel **faceMipmap;

    /* Bit fields telling if the texture image is uptodate. one bit controls one level.
    ** if sw rasterizer is going to use this texture, we must make sure that the texture image is uptodate.
    */
    GLbitfield imageUpToDate;

    /* texture age, this is used to do texture swap using LRU algorithm */
    GLuint age;

    /*
    ** The max level that will be sampled by HW.
    ** This value is the "q" in the spec ( refer to "Texture Completeness" part in the spec)
    */
    GLint maxLevelUsed;

    /* Only sample from base level when it is true */
    GLboolean forceBaseLeve;

    /*
    ** Following fields are used by render texture.
    ** They will be set in __glBindTexImageARB.
    */
    GLuint pBufferNumLevels;/* How many levels */
    GLenum colorBuffer;/* The colorbuffer bound to the texture */
    GLvoid *hPbuffer;/* handle for this pbuffer */
    /* End of fields that are used by render texture*/

    /*
    ** For pixel_buffer_object
    */
    GLuint unpackBuffer;
    GLvoid *offsetInPBO;

    /* the number of arrays which is used for texture array extensions
    ** cube map is 6, 1d, 2d and 3d texture are 1, texture_1d_array and texture_2d_array
    ** are variable
    */
    GLint arrays;

    /* maxFaces and maxLevels
    */
    GLuint maxFaces;
    GLuint maxLevels;

    /*
    ** pointer to the buffer object which is attached to texture buffer object
    */
    __GLbufferObject *bufferObj;

} ;

/*
** Encapsulate texture object attribute for push/pop purposes.
*/
typedef struct __GLtextureObjAttrRec
{
    GLuint name;
    __GLtextureParamState params;
} __GLtextureObjAttr;

/*
** Stackable client texture state. This does not include
** the mipmaps, or level dependent state.  Only state which is
** stackable via glPushAttrib/glPopAttrib is here.  The rest of the
** state is in the machine structure below.
*/
typedef struct __GLtextureUnitStateRec
{
    /* Per coordinate texture state (set with glTexGen) */
    __GLtextureCoordState s;
    __GLtextureCoordState t;
    __GLtextureCoordState r;
    __GLtextureCoordState q;

    /* Texture object state */
    __GLtextureObjAttr texObj[__GL_MAX_TEXTURE_BINDINGS];

    /* Texture environment state */
    __GLtextureEnvState env;

    /* Per texture unit GL_TEXTURE_FILTER_CONTROL texture lodBias value */
    GLfloat lodBias;

    /* Current texture enable dimension based on "enabledDimension" and texture
     * consistency check result. a value of zero means nothing is enabled.
     */
    GLuint currentEnableDim;

} __GLtextureUnitState;

typedef struct __GLtextureStateRec
{
    __GLtextureUnitState texUnits[__GL_MAX_TEXTURE_UNITS];

    GLuint activeTexIndex;

    /* This is the user-controlled GL_TEXTURE_BUFFER_EXT binding point which is
    ** set by glBindBuffer(GL_TEXTURE_BUFFER_EXT, buffer). The same buffer binding
    ** is also stored in gc->bufferObject.boundBuffer[__GL_TEXTURE_BUFFER_EXT_INDEX].
    */
    GLuint textureBufBinding;

} __GLtextureState;


typedef struct __GLtextureUnitRec
{
    __GLtextureObject *boundTextures[__GL_MAX_TEXTURE_BINDINGS];

    __GLtextureObject *currentTexture;

} __GLtextureUnit;

typedef struct __GLtextureMachineRec
{
    __GLsharedObjectMachine *shared;

    __GLtextureUnit units[__GL_MAX_TEXTURE_UNITS];

    /* Array of dummy texture objects for the default textures */
    __GLtextureObject defaultTextures[__GL_MAX_TEXTURE_BINDINGS];

    /* Texture objects for proxy textures */
    __GLtextureObject proxyTextures[__GL_MAX_TEXTURE_BINDINGS];

    /* If a stage has any texture target enabled
    ** (gc->state.enables.texUnits[unit].enabledDimension > 0)
    ** the corresponding bit will be set.
    */
    GLbitfield enabledMask;

    /* If a stage has any texture target ACTUALLY enabled after __glEvaluateAttributeChange
    ** (gc->state.texture.texUnits[i].currentEnableDim > 0), the corresponding bit will be set.
    */
    GLuint64 currentEnableMask;

    /* This stage is used to simulate DrawPixels with depth component
    ** currently we always use the biggest texture stage, and will fall back to SW
    ** Path if the texture stage is already enabled
    */
    GLint drawDepthStage;

    /* A general texture object Template used for initial a new texture object*/
    __GLtextureObject texObjTemplate;

    /* Texture conflict mask */
    GLuint texConflit;

} __GLtextureMachine;


extern GLboolean __glCheckTexImageArgs(__GLcontext *gc,
                               GLenum target,
                               GLint lod,
                               GLint internalFormat,
                               GLsizei width,
                               GLsizei height,
                               GLsizei depth,
                               GLint border,
                               GLenum format,
                               GLenum type);

extern GLboolean __glCheckTexSubImageArgs(__GLcontext *gc,
                               __GLtextureObject *tex,
                               GLuint face,
                               GLint lod,
                               GLint xoffset,
                               GLint yoffset,
                               GLint zoffset,
                               GLsizei width,
                               GLsizei height,
                               GLsizei depth,
                               GLenum format,
                               GLenum type);

/* Bind texture object function */
extern GLvoid __glBindTexture(__GLcontext *gc, GLuint unitIdx, GLuint targetIndex, GLuint texture);

/* Compute the number of levels given width,height,depth and level */
__GL_INLINE GLuint __glComputeNumLevels(GLsizei width,GLsizei height,GLsizei depth,GLint lod)
{
    while((width != 1) || (height != 1) || (depth != 1))
    {
        width >>= 1;
        height >>= 1;
        depth >>= 1;

        lod ++;
        if (width == 0)
        {
            width = 1;
        }
        if (height == 0)
        {
            height = 1;
        }
        if (depth == 0)
        {
            depth = 1;
        }
    }

    return (lod+1);
}

#endif /* __gc_gl_texture_h_ */
