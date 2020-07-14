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


#ifndef __gc_es_texture_h__
#define __gc_es_texture_h__


/*
** Maximum linear table size for texture object.
*/
#define __GL_MAX_TEXOBJ_LINEAR_TABLE_SIZE           8192

/*
** Default linear table size for texture object.
*/
#define __GL_DEFAULT_TEXOBJ_LINEAR_TABLE_SIZE       1024

/*
** Maximum hash table size for texture object. Must be 2^n.
*/
#define __GL_TEXOBJ_HASH_TABLE_SIZE                 8192

/*
** Texture binding index
*/
enum __GL_TEXTURE_BINDING_INDEX
{
    __GL_TEXTURE_2D_INDEX = 0,
    __GL_TEXTURE_3D_INDEX,
    __GL_TEXTURE_CUBEMAP_INDEX,
    __GL_TEXTURE_2D_ARRAY_INDEX,
    __GL_TEXTURE_EXTERNAL_INDEX,
    __GL_TEXTURE_2D_MS_INDEX,
    __GL_TEXTURE_2D_MS_ARRAY_INDEX,
    __GL_TEXTURE_CUBEMAP_ARRAY_INDEX,
    __GL_TEXTURE_BINDING_BUFFER_EXT,

    /* Number of texture bindings*/
    __GL_MAX_TEXTURE_BINDINGS,
};

enum __GL_TEX_COMPONENT
{
    __GL_TEX_COMPONENT_R,
    __GL_TEX_COMPONENT_G,
    __GL_TEX_COMPONENT_B,
    __GL_TEX_COMPONENT_A,

    __GL_TEX_COMPONENT_NUM,
};

enum
{
    __GL_TEX_MIP_HINT_AUTO = 0,
    __GL_TEX_MIP_HINT_FORCE_ON,
    __GL_TEX_MIP_HINT_FORCE_OFF,
    __GL_TEX_MIP_HINT_AUTO_MIP
};

#define __GL_HALVE_SIZE(size)   \
    size >>= 1;                 \
    if (size == 0)              \
    {                           \
        size = 1;               \
    }

#define __GL_IS_2D_TEXTURE_ARRAY(targetIdx) \
    ((targetIdx == __GL_TEXTURE_2D_ARRAY_INDEX) || (targetIdx == __GL_TEXTURE_2D_MS_ARRAY_INDEX))

#define __GL_IS_TEXTURE_MSAA(targetIdx) \
    ((targetIdx == __GL_TEXTURE_2D_MS_INDEX) || (targetIdx == __GL_TEXTURE_2D_MS_ARRAY_INDEX))

#define __GL_IS_TEXTURE_ARRAY(targetIdx) \
    ((__GL_IS_2D_TEXTURE_ARRAY(targetIdx)) || \
     (targetIdx == __GL_TEXTURE_CUBEMAP_ARRAY_INDEX))

#define __GL_IS_TEXTURE_CUBE(targetIdx) \
    ((targetIdx == __GL_TEXTURE_CUBEMAP_INDEX) || (targetIdx == __GL_TEXTURE_CUBEMAP_ARRAY_INDEX))

/*
** Macros for texture image
*/
#define GL_TEX_IMAGE_UPTODATE(tex, level) \
    tex->imageUpToDate |= (__GL_ONE_32 << (level))

#define GL_TEX_IMAGE_OUTDATED(tex, level) \
    tex->imageUpToDate &= ~(__GL_ONE_32 << (level))

#define GL_TEX_IMAGE_IS_UPTODATE(tex, level) \
    (tex->imageUpToDate & (__GL_ONE_32 << (level)))

#define GL_TEX_IMAGE_IS_ANY_LEVEL_DIRTY( tex, baseLevel, maxLevel) \
    ((~(tex->imageUpToDate)) & ( (__GL_ONE_32 << (maxLevel + 1)) - (__GL_ONE_32 << baseLevel)))


typedef struct __GLsamplerObjectRec __GLsamplerObject;

typedef struct __GLsamplerParamStateRec
{
    /* Wrap modes */
    GLenum sWrapMode;
    GLenum tWrapMode;
    GLenum rWrapMode;

    /* Min and mag filter */
    GLenum minFilter;
    GLenum magFilter;

    /* Texture lods */
    GLfloat minLod;
    GLfloat maxLod;

    /* Shadow texture */
    GLenum compareMode;
    GLenum compareFunc;

    GLfloat maxAnistropy;

    GLenum sRGB;

    union {
        GLfloat  fv[4];
        GLint    iv[4];
        GLuint  uiv[4];
    } borderColor;

} __GLsamplerParamState;


/*
** Client state set with glTexParameter
*/
typedef struct __GLtextureParamStateRec
{
    __GLsamplerParamState sampler;

    /* mipHint is a VIV private tex parameter.
    ** When a tex generates mipmap and those mipmaps, either the range or image of any levels,
    ** were not changed until draw time, we will force mipmap OFF if the generation failed,
    ** and force the mipmap ON if generation successed (benefit for Antutu).
    ** For DirectVIV tex, we also force the mipmap always OFF.
    */
    GLenum mipHint;

    /* Texture levels */
    GLint baseLevel;
    GLint maxLevel;

    /* RGBA swizzle */
    GLenum swizzle[__GL_TEX_COMPONENT_NUM];

    /* Depth stencil texture mode */
    GLenum dsTexMode;

    /* VIV private property */
    GLboolean contentProtected;

} __GLtextureParamState;

/*
** Mipmap level information for texture object
*/
struct __GLmipMapLevelRec
{
    /* Image dimensions, including border */
    GLint width, height, depth;

    /* Array size, for cubemap array, it's layer-face*/
    GLint arrays;

    GLboolean compressed;
    GLsizei compressedSize;

    /* Requested internal format */
    GLint requestedFormat;

    /* internal format */
    GLint interalFormat;

    /* Base internal format */
    GLenum baseFormat;

    /* Parameters from the texImage */
    GLenum format;
    GLenum type;

    /* Texture format info */
    __GLformatInfo *formatInfo;

    /* Texture Buffer info */
    GLint bufObjName;
    GLint offset;
    GLint size;
};

/*
** Texture object structure.
*/
struct __GLtextureObjectRec
{
    /* Number of texture units (could be from different contexts) that bound to the object.
    */
    GLuint bindCount;

    /* The seqNumber is increased by 1 whenever object states are changed.
    ** DP must update its internal object states if its internal copy of
    ** savedSeqNum is different from this seqNumber.
    */
    GLuint seqNumber;

    /* List of texture unit numbers (could be from different contexts) the texture object
    ** has EVER been bound to, will never remove from the list.
    */
    __GLimageUser *texUnitBoundList;

    /* List of FBO this texture object attached to
    */
    __GLimageUser *fboList;

    /* List of image unit which bind with this texture object
    */
    __GLimageUser *imageList;

    /* Internal flag for generic object management.
    */
    GLbitfield flag;

    /* Texture object privateData pointer.
    */
    GLvoid *privateData;

    GLuint name;
    GLuint targetIndex;

    /* Texture parameter state (set with glTexParameter) */
    __GLtextureParamState params;

    GLboolean immutable;
    GLint immutableLevels;

    /* faceMipmap can be seen as a 2-dimensional array of mipmap. The usage of this member to get a
    ** __GLmipMapLevel is: faceMipmap[face][level]. for none-cubemap textures, face will always be 0.
    */
    __GLmipMapLevel **faceMipmap;

    /* Bit fields indicating if the texture image is uptodate. one bit controls one level.
    */
    GLbitfield imageUpToDate;

    /* The baseLevel/maxLevel when glGenerateMipmap */
    GLint  mipBaseLevel;
    GLint  mipMaxLevel;

    /* The number of arrays which is used for texture array extensions
    ** cube map is 6, 1d, 2d and 3d texture are 1, texture_1d_array and texture_2d_array
    ** are variable, for cubemap arary, it's a layer-face value.
    */
    GLint arrays;

    /*
    ** maxFaces:   it's 6 for cubemap, 1 otherwise.
    ** maxDepths:  3d texture: maxDepthSize;
    **             2d array  : maxArraySize;
    **             cube array: 6 * maxArraySize
    ** maxSlices:  equals to maxFaces if cube, and maxDepths otherwise
    */
    GLuint maxFaces;
    GLuint maxDepths;
    GLuint maxSlices;
    GLuint maxLevels;

    /* Below states belong to this object only. Use for texture descriptor only.
    ** It won't compare with the previous object on the same texture unit.
    */
    GLuint maxLevelUsed;
    union {
        GLfloat  fv[4];
        GLint    iv[4];
        GLuint  uiv[4];
    } borderColorUsed;

    union
    {
        struct
        {
            GLuint maxLevelUsedDirty : 1;
            GLuint swizzleDirty      : 1;
            GLuint dsModeDirty       : 1;
            GLuint baseLevelDirty    : 1;
            GLuint reserved          : 28;
        } s;
        GLuint objStateDirty;
    } uObjStateDirty;

    /*
    ** Flag to indicate whether or not the texture is specified with sized internal format.
    ** Extension GL_OES_depth_texture defined unsized internal format, which produce (d,d,d,1.0) color data
    ** OES3.0 core spec defined sized internal format which produce (d,0,0,1.0) color data
    */
    GLboolean unsizedTexture;

    /*
    ** Is the texture specified with canonical format combination?
    */
    GLboolean canonicalFormat;
    /* For MSAA texture */
    GLint samples;
    GLboolean fixedSampleLocations;

    GLuint samplesUsed;

    GLchar *label;

    __GLbufferObject* bufObj;
    GLint bufSize;
    GLint bufOffset;
    GLint bppPerTexel;
};

/*
** States of a texture unit in context.
*/
typedef struct __GLtextureUnitStateRec
{
    /* Texture parameters that have been committed to a HW texture unit */
    __GLtextureParamState commitParams;

    /* Enable dimension was build from program and sampler mapping at draw time */
    GLuint enableDim;

    /* The previous enableDim combined with texture complete check */
    GLuint realEnableDim;

} __GLtextureUnitState;

typedef struct __GLtextureStateRec
{
    __GLtextureUnitState texUnits[__GL_MAX_TEXTURE_UNITS];
    GLuint activeTexIndex;

} __GLtextureState;

typedef struct __GLtextureUnitRec
{
    __GLsamplerObject *boundSampler;
    __GLtextureObject *boundTextures[__GL_MAX_TEXTURE_BINDINGS];

    __GLtextureObject *currentTexture;

    /*
    ** The max level that will be sampled by HW, was evaluated at draw time.
    ** This value is the "q" in the spec ( refer to "Texture Completeness" part in the spec)
    ** The value cannot be put in __GLtextureObject, because a texture can be bound to different
    ** unit and each unit may use different levels.
    */
    GLuint maxLevelUsed;

} __GLtextureUnit;

typedef struct __GLtextureMachineRec
{
    __GLsharedObjectMachine *shared;

    __GLtextureUnit units[__GL_MAX_TEXTURE_UNITS];

    /* Array of default texture objects embedded in context */
    __GLtextureObject defaultTextures[__GL_MAX_TEXTURE_BINDINGS];

    /* If a stage has any texture target ACTUALLY enabled after __glEvaluateAttributeChange
    ** (gc->state.texture.texUnits[i].currentEnableDim > 0), the corresponding bit will be set.
    */
    __GLbitmask currentEnableMask;

    /* Texture conflict mask */
    __GLbitmask texConflict;

} __GLtextureMachine;


/*
** Sampler Object
*/

#define __GL_MAX_SAMPLEROBJ_LINEAR_TABLE_SIZE           1024
#define __GL_DEFAULT_SAMPLEROBJ_LINEAR_TABLE_SIZE       256
#define __GL_SAMPLER_HASH_TABLE_SIZE                    512

struct __GLsamplerObjectRec
{
    GLuint name;

    GLuint bindCount;
    GLbitfield flags;

    /* List of texture units this sampler object has ever been bound to.
    ** Even later unbound, the unit will not be removed from the list.
    */
    __GLimageUser *texUnitBoundList;

    __GLsamplerParamState params;

    GLchar *label;
};

typedef struct __GLsamplerMachineRec
{
    __GLsharedObjectMachine *shared;

} __GLsamplerMachine;

/* Bind texture object function */
extern GLvoid __glBindTexture(__GLcontext *gc, GLuint unitIdx, GLuint targetIndex, GLuint texture);
extern GLvoid __glSetFBOAttachedTexDirty(__GLcontext *gc, GLbitfield mask, GLint drawbuffer);

enum __GL_IMAGE_TYPE
{
    __GL_IMAGE_2D         = 0,
    __GL_IMAGE_3D         = 1,
    __GL_IMAGE_CUBE       = 2,
    __GL_IMAGE_2D_ARRAY   = 3,
    __GL_IMAGE_CUBE_ARRAY = 4,
    __GL_IMAGE_BUFFER     = 5,
    __GL_IMAGE_TYPE_NUM
};

typedef struct __GLimageUnitStateRec
{
    /* State defined by spec
    */
    __GLtextureObject *texObj;
    GLint level;
    GLboolean layered;
    GLint requestLayer;
    GLenum access;
    GLenum format;

    /* Internal flag */
    __GLformatInfo *formatInfo;
    GLint     actualLayer;
    GLuint    type;
    GLboolean invalid;
    GLboolean singleLayered;
    GLuint width;
    GLuint height;
    GLuint depth;
} __GLimageUnitState;

typedef struct __GLimageStateRec
{
    __GLimageUnitState  imageUnit[__GL_MAX_IMAGE_UNITS];
} __GLimageState;

extern GLvoid __glUnBindTextureBuffer(__GLcontext *gc, __GLtextureObject *tex, __GLbufferObject *bufObj);

#endif /* __gc_es_texture_h__ */
