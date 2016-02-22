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


#include "gc_glff_precomp.h"
#define _GC_OBJ_ZONE    glvZONE_TEXTURE

#if defined(ANDROID)
#if ANDROID_SDK_VERSION >= 16
#      include <ui/ANativeObjectBase.h>
#   else
#      include <private/ui/android_natives_priv.h>
#   endif

#   include <gc_gralloc_priv.h>
#endif

/******************************************************************************\
***************************** Texture GL Name Arrays ***************************
\******************************************************************************/

/* Possible texture min filters. */
static GLenum _TextureMinFilterNames[] =
{
    GL_NEAREST,                 /* glvNEAREST */
    GL_LINEAR,                  /* glvLINEAR */
    GL_NEAREST_MIPMAP_NEAREST,  /* glvNEAREST_MIPMAP_NEAREST */
    GL_LINEAR_MIPMAP_NEAREST,   /* glvLINEAR_MIPMAP_NEAREST */
    GL_NEAREST_MIPMAP_LINEAR,   /* glvNEAREST_MIPMAP_LINEAR */
    GL_LINEAR_MIPMAP_LINEAR,    /* glvLINEAR_MIPMAP_LINEAR */
};

/* Possible texture mag filters. */
static GLenum _TextureMagFilterNames[] =
{
    GL_NEAREST,                 /* glvNEAREST */
    GL_LINEAR,                  /* glvLINEAR */
};

/* Possible texture wraps. */
static GLenum _TextureWrapNames[] =
{
    GL_CLAMP_TO_EDGE,           /* glvCLAMP */
    GL_REPEAT,                  /* glvREPEAT */
    GL_MIRRORED_REPEAT_OES,     /* gcvMIRROR */
};

/* Possible texture functions. */
static GLenum _TextureFunctionNames[] =
{
    GL_REPLACE,                 /* glvTEXREPLACE */
    GL_MODULATE,                /* glvTEXMODULATE */
    GL_DECAL,                   /* glvTEXDECAL */
    GL_BLEND,                   /* glvTEXBLEND */
    GL_ADD,                     /* glvTEXADD */
    GL_COMBINE,                 /* glvTEXCOMBINE */
};

/* Possible combine color texture functions. */
static GLenum _CombineColorTextureFunctionNames[] =
{
    GL_REPLACE,                 /* glvCOMBINEREPLACE */
    GL_MODULATE,                /* glvCOMBINEMODULATE */
    GL_ADD,                     /* glvCOMBINEADD */
    GL_ADD_SIGNED,              /* glvCOMBINEADDSIGNED */
    GL_INTERPOLATE,             /* glvCOMBINEINTERPOLATE */
    GL_SUBTRACT,                /* glvCOMBINESUBTRACT */
    GL_DOT3_RGB,                /* glvCOMBINEDOT3RGB */
    GL_DOT3_RGBA,               /* glvCOMBINEDOT3RGBA */
};

/* Possible combine alpha texture functions. */
static GLenum _CombineAlphaTextureFunctionNames[] =
{
    GL_REPLACE,                 /* glvCOMBINEREPLACE */
    GL_MODULATE,                /* glvCOMBINEMODULATE */
    GL_ADD,                     /* glvCOMBINEADD */
    GL_ADD_SIGNED,              /* glvCOMBINEADDSIGNED */
    GL_INTERPOLATE,             /* glvCOMBINEINTERPOLATE */
    GL_SUBTRACT,                /* glvCOMBINESUBTRACT */
};

/* Possible combine texture function sources. */
static GLenum _CombineFunctionSourceNames[] =
{
    GL_TEXTURE,                 /* glvTEXTURE */
    GL_CONSTANT,                /* glvCONSTANT */
    GL_PRIMARY_COLOR,           /* glvCOLOR */
    GL_PREVIOUS,                /* glvPREVIOUS */
};

/* Possible combine texture function RGB operands. */
static GLenum _CombineFunctionColorOperandNames[] =
{
    GL_SRC_ALPHA,               /* glvSRCALPHA */
    GL_ONE_MINUS_SRC_ALPHA,     /* glvSRCALPHAINV */
    GL_SRC_COLOR,               /* glvSRCCOLOR */
    GL_ONE_MINUS_SRC_COLOR,     /* glvSRCCOLORINV */
};

/* Possible combine texture function alpha operands. */
static GLenum _CombineFunctionAlphaOperandNames[] =
{
    GL_SRC_ALPHA,               /* glvSRCALPHA */
    GL_ONE_MINUS_SRC_ALPHA,     /* glvSRCALPHAINV */
};

/* Possible texture coordinate generation modes. */
static GLenum _TextureGenModes[] =
{
    GL_NORMAL_MAP_OES,          /* glvTEXNORMAL */
    GL_REFLECTION_MAP_OES,      /* glvREFLECTION */
};

static gceTEXTURE_TYPE _HALtexType[] =
{
    gcvTEXTURE_2D,              /* glvTEXTURE2D */
    gcvTEXTURE_CUBEMAP,         /* glvCUBEMAP   */
    gcvTEXTURE_EXTERNAL,        /* glvTEXTUREEXTERNAL */
};

/******************************************************************************\
******************************* Texture List Code *****************************
\******************************************************************************/

/*******************************************************************************
**
**  _glffFindTexture
**
**  Finds a texture wrapper object in the hash list
**
**  INPUT:
**
**      List
**          Pointer to the texture list
**
**      Name
**          Name of the texture wrapper object.
**
**  OUTPUT:
**
**      Pointer to the texture wrapper object.
*/
glsTEXTUREWRAPPER_PTR
_glffFindTexture(
    glsTEXTURELIST * List,
    GLuint Name
    )
{
    GLuint index;
    glsTEXTUREWRAPPER_PTR texture;

#if gldSUPPORT_SHARED_CONTEXT
    if (List == gcvNULL)
    {
        return gcvNULL;
    }
    if(List->sharedLock != gcvNULL)
    {
        gcmVERIFY_OK(gcoOS_AcquireMutex(gcvNULL, List->sharedLock, gcvINFINITE));
    }

#endif

    index = Name % NAMED_TEXTURE_HASH;
    for (texture = List->objects[index];
         texture != gcvNULL;
         texture = texture->next)
    {
        if (texture->name == Name)
        {
#if gldSUPPORT_SHARED_CONTEXT
             if(List->sharedLock != gcvNULL)
             {
                gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, List->sharedLock));
             }
#endif
            return texture;
        }
    }

#if gldSUPPORT_SHARED_CONTEXT
     if(List->sharedLock != gcvNULL)
     {
        gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, List->sharedLock));
     }
#endif

    return gcvNULL;
}

/*******************************************************************************
**
**  _glffRemoveTexture
**
**  Removes a texture wrapper object from the hash list
**
**  INPUT:
**
**      List
**          Pointer to the texture list
**
**      Name
**          Name of the texture wrapper object.
**
**  OUTPUT:
**          None
*/
void
_glffRemoveTexture(
    glsTEXTURELIST * List,
    glsTEXTUREWRAPPER_PTR Texture
    )
{
    GLuint index;

#if gldSUPPORT_SHARED_CONTEXT
    if (List == gcvNULL)
    {
        return;
    }

    if(List->sharedLock  != gcvNULL)
    {
        gcmVERIFY_OK(gcoOS_AcquireMutex(gcvNULL, List->sharedLock, gcvINFINITE));
    }
#endif

    index = Texture->name % NAMED_TEXTURE_HASH;

    if (Texture->prev != gcvNULL)
    {
        Texture->prev->next = Texture->next;
    }
    else
    {
        List->objects[index] = Texture->next;
    }

    if (Texture->next != gcvNULL)
    {
        Texture->next->prev = Texture->prev;
    }

#if gldSUPPORT_SHARED_CONTEXT
     if(List->sharedLock  != gcvNULL)
     {
        gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, List->sharedLock));
     }
#endif
}


/*******************************************************************************
**
**  _glffInsertTexture
**
**  Inserts a texture wrapper object to the hash list
**
**  INPUT:
**
**      List
**          Pointer to the texture list
**
**      Texture
**          Pointer to the texture wrapper object.
**
**  OUTPUT:
**          None
*/
void
_glffInsertTexture(
    glsTEXTURELIST * List,
    glsTEXTUREWRAPPER_PTR Texture
    )
{

    GLuint index;

#if gldSUPPORT_SHARED_CONTEXT
        if (List == gcvNULL)
    {
        return;
    }

    if(List->sharedLock  != gcvNULL)
    {
        gcmVERIFY_OK(gcoOS_AcquireMutex(gcvNULL, List->sharedLock, gcvINFINITE));
    }
#endif

    index = Texture->name % NAMED_TEXTURE_HASH;

    Texture->prev = gcvNULL;
    Texture->next = List->objects[index];

    if (Texture->next != gcvNULL)
    {
        Texture->next->prev = Texture;
    }

    List->objects[index] = Texture;

#if gldSUPPORT_SHARED_CONTEXT
    if(List->sharedLock  != gcvNULL)
    {
        gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, List->sharedLock));
    }
#endif
}


/*******************************************************************************
**
**  _glffGetNextAvailableName
**
**  Returns the next available name for a new texture wrapper object
**
**  INPUT:
**
**      List
**          Pointer to the texture list
**
**
**  OUTPUT:
**          Next available name for a new texture wrapper object
*/
GLuint
_glffGetNextAvailableName(
    glsTEXTURELIST * List
    )
{
    GLuint name;
    glsTEXTUREWRAPPER_PTR texture;

#if gldSUPPORT_SHARED_CONTEXT
        if (List == gcvNULL)
    {
        return 0;
    }

    if(List->sharedLock  != gcvNULL)
    {
        gcmVERIFY_OK(gcoOS_AcquireMutex(gcvNULL, List->sharedLock, gcvINFINITE));
    }
#endif

    while(GL_TRUE)
    {
        /* get next proposal for name */
        name = List->lastTexture + 1;

        /* see if the name has been used or not.*/
        texture = _glffFindTexture(List, name);

        if (texture == gcvNULL)
        {
            /* this name is not used before. So we can use it */
            List->lastTexture = name;
            break;
        }
        else
        {
            /* this is used. Go to next */
            List->lastTexture += 1;
        }
    }

#if gldSUPPORT_SHARED_CONTEXT
     if(List->sharedLock  != gcvNULL)
     {
        gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, List->sharedLock));
     }
#endif

    return name;
}


/******************************************************************************\
********************** Individual State Setting Functions **********************
\******************************************************************************/

static GLboolean _SetTextureFunction(
    glsCONTEXT_PTR Context,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLfloat* Value
    )
{
    GLboolean result;
    GLuint value;
    gcmHEADER_ARG("Context=0x%x Sampler=0x%x Value=0x%x",
                    Context, Sampler, Value);

    result = glfConvertGLEnum(
        _TextureFunctionNames,
        gcmCOUNTOF(_TextureFunctionNames),
        Value,
        glvFLOAT,
        &value
        );

    if (result)
    {
        glmSETHASH_3BITS(hashTextureFunction, value, Sampler->index);
        Sampler->function = (gleTEXTUREFUNCTION) value;
    }

    gcmFOOTER_ARG("return=%d", result);
    return result;
}

static GLboolean _SetCombineColorFunction(
    glsCONTEXT_PTR Context,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLfloat* Value
    )
{
    GLboolean result;
    GLuint value;
    gcmHEADER_ARG("Context=0x%x Sampler=0x%x Value=0x%x",
                    Context, Sampler, Value);

    result = glfConvertGLEnum(
        _CombineColorTextureFunctionNames,
        gcmCOUNTOF(_CombineColorTextureFunctionNames),
        Value,
        glvFLOAT,
        &value
        );

    if (result)
    {
        glmSETHASH_4BITS(hashTextureCombColorFunction, value, Sampler->index);
        Sampler->combColor.function = (gleTEXCOMBINEFUNCTION) value;

        if (value == glvCOMBINEDOT3RGBA)
        {
            Sampler->colorDataFlow.targetEnable = gcSL_ENABLE_XYZW;
            Sampler->colorDataFlow.tempEnable   = gcSL_ENABLE_XYZW;
            Sampler->colorDataFlow.tempSwizzle  = gcSL_SWIZZLE_XYZW;
            Sampler->colorDataFlow.argSwizzle   = gcSL_SWIZZLE_XYZW;
        }
        else
        {
            Sampler->colorDataFlow.targetEnable = gcSL_ENABLE_XYZ;
            Sampler->colorDataFlow.tempEnable   = gcSL_ENABLE_XYZ;
            Sampler->colorDataFlow.tempSwizzle  = gcSL_SWIZZLE_XYZZ;
            Sampler->colorDataFlow.argSwizzle   = gcSL_SWIZZLE_XYZZ;
        }
    }

    gcmFOOTER_ARG("return=%d", result);
    return result;
}

static GLboolean _SetCombineAlphaFunction(
    glsCONTEXT_PTR Context,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLfloat* Value
    )
{
    GLboolean result;
    GLuint value;
    gcmHEADER_ARG("Context=0x%x Sampler=0x%x Value=0x%x",
                    Context, Sampler, Value);

    result = glfConvertGLEnum(
        _CombineAlphaTextureFunctionNames,
        gcmCOUNTOF(_CombineAlphaTextureFunctionNames),
        Value,
        glvFLOAT,
        &value
        );

    if (result)
    {
        glmSETHASH_3BITS(hashTextureCombAlphaFunction, value, Sampler->index);
        Sampler->combAlpha.function = (gleTEXCOMBINEFUNCTION) value;
    }

    gcmFOOTER_ARG("return=%d", result);
    return result;
}

static GLboolean _SetCombineColorSource(
    glsCONTEXT_PTR Context,
    GLenum Source,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLfloat* Value
    )
{
    GLboolean result;
    GLuint value;
    gcmHEADER_ARG("Context=0x%x Source=0x%04x Sampler=0x%x Value=0x%x",
                    Context, Source, Sampler, Value);

    result = glfConvertGLEnum(
        _CombineFunctionSourceNames,
        gcmCOUNTOF(_CombineFunctionSourceNames),
        Value,
        glvFLOAT,
        &value
        );

    if (result)
    {
        gctUINT source = Source - GL_SRC0_RGB;

        switch (source)
        {
        case 0:
            glmSETHASH_2BITS(hashTextureCombColorSource0, value, Sampler->index);
            Sampler->combColor.source[0] = (gleCOMBINESOURCE) value;
            break;

        case 1:
            glmSETHASH_2BITS(hashTextureCombColorSource1, value, Sampler->index);
            Sampler->combColor.source[1] = (gleCOMBINESOURCE) value;
            break;

        case 2:
            glmSETHASH_2BITS(hashTextureCombColorSource2, value, Sampler->index);
            Sampler->combColor.source[2] = (gleCOMBINESOURCE) value;
            break;
        }
    }

    gcmFOOTER_ARG("return=%d", result);
    return result;
}

static GLboolean _SetCombineAlphaSource(
    glsCONTEXT_PTR Context,
    GLenum Source,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLfloat* Value
    )
{
    GLboolean result;
    GLuint value;
    gcmHEADER_ARG("Context=0x%x Source=0x%04x Sampler=0x%x Value=0x%x",
                    Context, Source, Sampler, Value);

    result = glfConvertGLEnum(
        _CombineFunctionSourceNames,
        gcmCOUNTOF(_CombineFunctionSourceNames),
        Value,
        glvFLOAT,
        &value
        );

    if (result)
    {
        gctUINT source = Source - GL_SRC0_ALPHA;

        switch (source)
        {
        case 0:
            glmSETHASH_2BITS(hashTextureCombAlphaSource0, value, Sampler->index);
            Sampler->combAlpha.source[0] = (gleCOMBINESOURCE) value;
            break;

        case 1:
            glmSETHASH_2BITS(hashTextureCombAlphaSource1, value, Sampler->index);
            Sampler->combAlpha.source[1] = (gleCOMBINESOURCE) value;
            break;

        case 2:
            glmSETHASH_2BITS(hashTextureCombAlphaSource2, value, Sampler->index);
            Sampler->combAlpha.source[2] = (gleCOMBINESOURCE) value;
            break;
        }
    }

    gcmFOOTER_ARG("return=%d", result);
    return result;
}

static GLboolean _SetCombineColorOperand(
    glsCONTEXT_PTR Context,
    GLenum Operand,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLfloat* Value
    )
{
    GLboolean result;
    GLuint value;
    gcmHEADER_ARG("Context=0x%x Operand=0x%04x Sampler=0x%x Value=0x%x",
                    Context, Operand, Sampler, Value);

    result = glfConvertGLEnum(
        _CombineFunctionColorOperandNames,
        gcmCOUNTOF(_CombineFunctionColorOperandNames),
        Value,
        glvFLOAT,
        &value
        );

    if (result)
    {
        GLuint operand = Operand - GL_OPERAND0_RGB;

        switch (operand)
        {
        case 0:
            glmSETHASH_2BITS(hashTextureCombColorOperand0, value, Sampler->index);
            Sampler->combColor.operand[0] = (gleCOMBINEOPERAND) value;
            break;

        case 1:
            glmSETHASH_2BITS(hashTextureCombColorOperand1, value, Sampler->index);
            Sampler->combColor.operand[1] = (gleCOMBINEOPERAND) value;
            break;

        case 2:
            glmSETHASH_2BITS(hashTextureCombColorOperand2, value, Sampler->index);
            Sampler->combColor.operand[2] = (gleCOMBINEOPERAND) value;
            break;
        }
    }

    gcmFOOTER_ARG("return=%d", result);
    return result;
}

static GLboolean _SetCombineAlphaOperand(
    glsCONTEXT_PTR Context,
    GLenum Operand,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLfloat* Value
    )
{
    GLboolean result;
    GLuint value;
    gcmHEADER_ARG("Context=0x%x Operand=0x%04x Sampler=0x%x Value=0x%x",
                    Context, Operand, Sampler, Value);

    result = glfConvertGLEnum(
        _CombineFunctionAlphaOperandNames,
        gcmCOUNTOF(_CombineFunctionAlphaOperandNames),
        Value,
        glvFLOAT,
        &value
        );

    if (result)
    {
        GLuint operand = Operand - GL_OPERAND0_ALPHA;

        switch (operand)
        {
        case 0:
            glmSETHASH_2BITS(hashTextureCombAlphaOperand0, value, Sampler->index);
            Sampler->combAlpha.operand[0] = (gleCOMBINEOPERAND) value;
            break;

        case 1:
            glmSETHASH_2BITS(hashTextureCombAlphaOperand1, value, Sampler->index);
            Sampler->combAlpha.operand[1] = (gleCOMBINEOPERAND) value;
            break;

        case 2:
            glmSETHASH_2BITS(hashTextureCombAlphaOperand2, value, Sampler->index);
            Sampler->combAlpha.operand[2] = (gleCOMBINEOPERAND) value;
            break;
        }
    }

    gcmFOOTER_ARG("return=%d", result);
    return result;
}

static GLboolean _SetCurrentColor(
    glsCONTEXT_PTR Context,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLfloat* Value
    )
{
    gcmHEADER_ARG("Context=0x%x Sampler=0x%x Value=0x%x",
                    Context, Sampler, Value);

    glfSetVector4(
        &Sampler->constColor,
        Value
        );

    gcmFOOTER_ARG("return=%d", GL_TRUE);

    return GL_TRUE;
}

static GLboolean _SetColorScale(
    glsCONTEXT_PTR Context,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLfloat* Value
    )
{
    GLfloat scale;
    gcmHEADER_ARG("Context=0x%x Sampler=0x%x Value=0x%x",
                    Context, Sampler, Value);

    scale = *Value;

    if ((scale != 1.0f) && (scale != 2.0f) && (scale != 4.0f))
    {
        gcmFOOTER_ARG("return=%d", GL_FALSE);
        return GL_FALSE;
    }

    Sampler->combColor.scale = *Value;

    glmSETHASH_1BIT(
        hashTexCombColorScaleOne,
        glfISONE(Sampler->combColor.scale),
        Sampler->index
        );

    /* Set uTexCombScale dirty. */
    Context->fsUniformDirty.uTexCombScaleDirty = gcvTRUE;

    gcmFOOTER_ARG("return=%d", GL_TRUE);

    return GL_TRUE;
}

static GLboolean _SetAlphaScale(
    glsCONTEXT_PTR Context,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLfloat* Value
    )
{
    GLfloat scale;
    gcmHEADER_ARG("Context=0x%x Sampler=0x%x Value=0x%x",
                    Context, Sampler, Value);

    scale = *Value;

    if ((scale != 1.0f) && (scale != 2.0f) && (scale != 4.0f))
    {
        gcmFOOTER_ARG("return=%d", GL_FALSE);
        return GL_FALSE;
    }

    Sampler->combAlpha.scale = *Value;

    glmSETHASH_1BIT(
        hashTexCombAlphaScaleOne,
        glfISONE(Sampler->combAlpha.scale),
        Sampler->index
        );

    /* Set uTexCombScale dirty. */
    Context->fsUniformDirty.uTexCombScaleDirty = gcvTRUE;

    gcmFOOTER_ARG("return=%d", GL_TRUE);

    return GL_TRUE;
}

static GLboolean _SetTexCoordGenMode(
    glsCONTEXT_PTR Context,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLfloat* Value
    )
{
    GLboolean result;
    gcmHEADER_ARG("Context=0x%x Sampler=0x%x Value=0x%x",
                    Context, Sampler, Value);

    do
    {
        GLuint value;

        /* Convert the value. */
        result = glfConvertGLEnum(
            _TextureGenModes,
            gcmCOUNTOF(_TextureGenModes),
            Value,
            glvFLOAT,
            &value
            );

        /* Error? */
        if (result)
        {
            /* Update the generation mode. */
            Sampler->genMode = (gleTEXTUREGEN) value;

            /* Update the hash key. */
            glmSETHASH_1BIT(hashTexCubeCoordGenMode, value, Sampler->index);
        }
    }
    while (gcvFALSE);

    gcmFOOTER_ARG("return=%d", result);

    /* Return result. */
    return result;
}


/******************************************************************************\
*********************** Support Functions and Definitions **********************
\******************************************************************************/

#define gcSENTINELTEXTURE ( (gcoTEXTURE) ~0 )

typedef struct _glsCOMPRESSEDTEXTURE * glsCOMPRESSEDTEXTURE_PTR;
typedef struct _glsCOMPRESSEDTEXTURE
{
    GLint       bits;
    GLint       bytes;
    GLenum      format;
    GLenum      type;
}
glsCOMPRESSEDTEXTURE;

static GLenum _compressedTextures[] =
{
    GL_PALETTE4_RGB8_OES,
    GL_PALETTE4_RGBA8_OES,
    GL_PALETTE4_R5_G6_B5_OES,
    GL_PALETTE4_RGBA4_OES,
    GL_PALETTE4_RGB5_A1_OES,
    GL_PALETTE8_RGB8_OES,
    GL_PALETTE8_RGBA8_OES,
    GL_PALETTE8_R5_G6_B5_OES,
    GL_PALETTE8_RGBA4_OES,
    GL_PALETTE8_RGB5_A1_OES,

    GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
    GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
    GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,
    GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,

    GL_ETC1_RGB8_OES,
};


/*******************************************************************************
**
**  _GetCompressedPalettedTextureDetails
**
**  Return Find the closest power of two to the specified value.
**
**  INPUT:
**
**      Name
**          Enumerated compressed paletted texture name.
**
**  OUTPUT:
**
**      Pointer to the format information.
*/

static glsCOMPRESSEDTEXTURE_PTR _GetCompressedPalettedTextureDetails(
    GLenum Name
    )
{
    static glsCOMPRESSEDTEXTURE _compressedTextureDetails[] =
    {
        { 4, 3, GL_RGB,  GL_UNSIGNED_BYTE },            /* GL_PALETTE4_RGB8_OES  */
        { 4, 4, GL_RGBA, GL_UNSIGNED_BYTE },            /* GL_PALETTE4_RGBA8_OES */
        { 4, 2, GL_RGB,  GL_UNSIGNED_SHORT_5_6_5 },     /* GL_PALETTE4_R5_G6_B5_OES */
        { 4, 2, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4 },   /* GL_PALETTE4_RGBA4_OES */
        { 4, 2, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1 },   /* GL_PALETTE4_RGB5_A1_OES */
        { 8, 3, GL_RGB,  GL_UNSIGNED_BYTE },            /* GL_PALETTE8_RGB8_OES */
        { 8, 4, GL_RGBA, GL_UNSIGNED_BYTE },            /* GL_PALETTE8_RGBA8_OES */
        { 8, 2, GL_RGB,  GL_UNSIGNED_SHORT_5_6_5 },     /* GL_PALETTE8_R5_G6_B5_OES */
        { 8, 2, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4 },   /* GL_PALETTE8_RGBA4_OES */
        { 8, 2, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1 },   /* GL_PALETTE8_RGB5_A1_OES */
    };

    gctINT index;

    gcmHEADER_ARG("Name=0x%04x", Name);

    /* Determine the info index. */
    index = Name - GL_PALETTE4_RGB8_OES;

    /* Out of ranage? */
    if ((index < 0) || (index > (gctINT) gcmCOUNTOF(_compressedTextureDetails)))
    {
        gcmFOOTER_ARG("return=0x%x", gcvNULL);
        return gcvNULL;
    }
    else
    {
        gcmFOOTER_ARG("return=0x%x", &_compressedTextureDetails[index]);
        return &_compressedTextureDetails[index];
    }
}


static void
_DecodeETC1Block(
    GLubyte * Output,
    GLsizei Stride,
    GLsizei Width,
    GLsizei Height,
    const GLubyte * Data
    )
{
    GLubyte base[2][3];
    GLboolean flip, diff;
    GLbyte index[2];
    GLint i, j, x, y, offset;
    static GLubyte table[][2] =
    {
        {  2,   8 },
        {  5,  17 },
        {  9,  29 },
        { 13,  42 },
        { 18,  60 },
        { 24,  80 },
        { 33, 106 },
        { 47, 183 },
    };

    diff = Data[3] & 0x2;
    flip = Data[3] & 0x1;

    if (diff)
    {
        GLbyte delta[3];

        base[0][0] = (Data[0] & 0xF8) | (Data[0] >> 5);
        base[0][1] = (Data[1] & 0xF8) | (Data[1] >> 5);
        base[0][2] = (Data[2] & 0xF8) | (Data[1] >> 5);

        delta[0] = (GLbyte) ((Data[0] & 0x7) << 5) >> 2;
        delta[1] = (GLbyte) ((Data[1] & 0x7) << 5) >> 2;
        delta[2] = (GLbyte) ((Data[2] & 0x7) << 5) >> 2;
        base[1][0] = base[0][0] + delta[0];
        base[1][1] = base[0][1] + delta[1];
        base[1][2] = base[0][2] + delta[2];
        base[1][0] |= base[1][0] >> 5;
        base[1][1] |= base[1][1] >> 5;
        base[1][2] |= base[1][2] >> 5;
    }
    else
    {
        base[0][0] = (Data[0] & 0xF0) | (Data[0] >> 4  );
        base[0][1] = (Data[1] & 0xF0) | (Data[1] >> 4  );
        base[0][2] = (Data[2] & 0xF0) | (Data[2] >> 4  );
        base[1][0] = (Data[0] << 4  ) | (Data[0] & 0x0F);
        base[1][1] = (Data[1] << 4  ) | (Data[1] & 0x0F);
        base[1][2] = (Data[2] << 4  ) | (Data[2] & 0x0F);
    }

    index[0] = (Data[3] & 0xE0) >> 5;
    index[1] = (Data[3] & 0x1C) >> 2;

    for (i = x = y = offset = 0; i < 2; ++i)
    {
        GLubyte msb = Data[5 - i];
        GLubyte lsb = Data[7 - i];

        for (j = 0; j < 8; ++j)
        {
            GLuint delta = 0;
            GLint r, g, b;
            GLint block = flip
                        ? (y < 2) ? 0 : 1
                        : (x < 2) ? 0 : 1;

            switch (((msb & 1) << 1) | (lsb & 1))
            {
            case 0x3: delta = -table[index[block]][1]; break;
            case 0x2: delta = -table[index[block]][0]; break;
            case 0x0: delta =  table[index[block]][0]; break;
            case 0x1: delta =  table[index[block]][1]; break;
            }

            r = base[block][0] + delta; r = gcmMAX(0x00, gcmMIN(r, 0xFF));
            g = base[block][1] + delta; g = gcmMAX(0x00, gcmMIN(g, 0xFF));
            b = base[block][2] + delta; b = gcmMAX(0x00, gcmMIN(b, 0xFF));

            if ((x < Width) && (y < Height))
            {
                Output[offset + 0] = (GLubyte) r;
                Output[offset + 1] = (GLubyte) g;
                Output[offset + 2] = (GLubyte) b;
            }

            offset += Stride;
            if (++y == 4)
            {
                y = 0;
                ++x;

                offset += 3 - 4 * Stride;
            }

            msb >>= 1;
            lsb >>= 1;
        }
    }
}

static void *
_DecompressETC1(
    IN GLsizei Width,
    IN GLsizei Height,
    IN GLsizei ImageSize,
    IN const void * Data,
    OUT gctUINT32 * Type
    )
{
    GLubyte * pixels, * line;
    const GLubyte * data;
    GLsizei x, y, stride;
    gctPOINTER pointer = gcvNULL;

    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL,
                                   Width * Height * 3,
                                   &pointer)))
    {
        glmERROR(GL_OUT_OF_MEMORY);
        return gcvNULL;
    }

    pixels = pointer;

    stride = Width * 3;
    data   = Data;

    for (y = 0, line = pixels; y < Height; y += 4, line += stride * 4)
    {
        GLubyte * p = line;

        for (x = 0; x < Width; x += 4)
        {
            gcmASSERT(ImageSize >= 8);

            _DecodeETC1Block(p,
                             stride,
                             gcmMIN(Width - x, 4),
                             gcmMIN(Height - y, 4),
                             data);

            p         += 4 * 3;
            data      += 8;
            ImageSize -= 8;
        }
    }

    *Type = GL_UNSIGNED_BYTE;

    return pixels;
}


#define glmRED(c)   ( (c) >> 11 )
#define glmGREEN(c) ( ((c) >> 5) & 0x3F )
#define glmBLUE(c)  ( (c) & 0x1F )

/* Decode 64-bits of color information. */
static void
_DecodeDXTColor16(
    IN GLsizei Width,
    IN GLsizei Height,
    IN GLsizei Stride,
    IN const GLubyte * Data,
    OUT GLubyte * Output
    )
{
    GLushort c0, c1;
    GLushort color[4];
    GLushort r, g, b;
    GLint x, y;

    /* Decode color 0. */
    c0 = *(GLushort*)Data;
    color[0] = 0x8000 | (c0 & 0x001F) | ((c0 & 0xFFC0) >> 1);

    /* Decode color 1. */
    c1 = *(GLushort*)(Data + 2);
    color[1] = 0x8000 | (c1 & 0x001F) | ((c1 & 0xFFC0) >> 1);

    if (c0 > c1)
    {
        /* Compute color 2: (c0 * 2 + c1) / 3. */
        r = (2 * glmRED  (c0) + glmRED  (c1)) / 3;
        g = (2 * glmGREEN(c0) + glmGREEN(c1)) / 3;
        b = (2 * glmBLUE (c0) + glmBLUE (c1)) / 3;
        color[2] = 0x8000 | (r << 10) | ((g & 0x3E) << 4) | b;

        /* Compute color 3: (c0 + 2 * c1) / 3. */
        r = (glmRED  (c0) + 2 * glmRED  (c1)) / 3;
        g = (glmGREEN(c0) + 2 * glmGREEN(c1)) / 3;
        b = (glmBLUE (c0) + 2 * glmBLUE (c1)) / 3;
        color[3] = 0x8000 | (r << 10) | ((g & 0x3E) << 4) | b;
    }
    else
    {
        /* Compute color 2: (c0 + c1) / 2. */
        r = (glmRED  (c0) + glmRED  (c1)) / 2;
        g = (glmGREEN(c0) + glmGREEN(c1)) / 2;
        b = (glmBLUE (c0) + glmBLUE (c1)) / 2;
        color[2] = 0x8000 | (r << 10) | ((g & 0x3E) << 4) | b;

        /* Color 3 is opaque black. */
        color[3] = 0;
    }
    /* Walk all lines. */
    for (y = 0; y < Height; y++)
    {
        /* Get lookup for line. */
        GLubyte bits = Data[4 + y];

        /* Walk all pixels. */
        for (x = 0; x < Width; x++, bits >>= 2, Output += 2)
        {
            /* Copy the color. */
            *(GLshort *) Output = color[bits & 3];
        }

        /* Next line. */
        Output += Stride - Width * 2;
    }
}

#define glmEXPAND_RED(c)   ( (((c) & 0xF800) << 8) | (((c) & 0xE000) << 3) )
#define glmEXPAND_GREEN(c) ( (((c) & 0x07E0) << 5) | (((c) & 0x0600) >> 1) )
#define glmEXPAND_BLUE(c)  ( (((c) & 0x001F) << 3) | (((c) & 0x001C) >> 2) )

/* Decode 64-bits of color information. */
static void
_DecodeDXTColor32(
    IN GLsizei Width,
    IN GLsizei Height,
    IN GLsizei Stride,
    IN const GLubyte * Data,
    IN const GLubyte *Alpha,
    OUT GLubyte * Output
    )
{
    GLuint color[4];
    GLushort c0, c1;
    GLint x, y;
    GLuint r, g, b;

    /* Decode color 0. */
    c0       = Data[0] | (Data[1] << 8);
    color[0] = glmEXPAND_RED(c0) | glmEXPAND_GREEN(c0) | glmEXPAND_BLUE(c0);

    /* Decode color 1. */
    c1       = Data[2] | (Data[3] << 8);
    color[1] = glmEXPAND_RED(c1) | glmEXPAND_GREEN(c1) | glmEXPAND_BLUE(c1);

    /* Compute color 2: (c0 * 2 + c1) / 3. */
    r = (2 * (color[0] & 0xFF0000) + (color[1] & 0xFF0000)) / 3;
    g = (2 * (color[0] & 0x00FF00) + (color[1] & 0x00FF00)) / 3;
    b = (2 * (color[0] & 0x0000FF) + (color[1] & 0x0000FF)) / 3;
    color[2] = (r & 0xFF0000) | (g & 0x00FF00) | (b & 0x0000FF);

    /* Compute color 3: (c0 + 2 * c1) / 3. */
    r = ((color[0] & 0xFF0000) + 2 * (color[1] & 0xFF0000)) / 3;
    g = ((color[0] & 0x00FF00) + 2 * (color[1] & 0x00FF00)) / 3;
    b = ((color[0] & 0x0000FF) + 2 * (color[1] & 0x0000FF)) / 3;
    color[3] = (r & 0xFF0000) | (g & 0x00FF00) | (b & 0x0000FF);

    /* Walk all lines. */
    for (y = 0; y < Height; y++)
    {
        /* Get lookup for line. */
        GLubyte bits = Data[4 + y];
        GLint   a    = y << 2;

        /* Walk all pixels. */
        for (x = 0; x < Width; x++, bits >>= 2, Output += 4)
        {
            /* Copmbine the lookup color with the alpha value. */
            GLuint c = color[bits & 3] | (Alpha[a++] << 24);
            *(GLuint *) Output = c;
        }

        /* Next line. */
        Output += Stride - Width * 4;
    }
}

/* Decode 64-bits of alpha information. */
static void
_DecodeDXT3Alpha(
    IN const GLubyte * Data,
    OUT GLubyte * Output
    )
{
    GLint i;
    GLubyte a;

    /* Walk all alpha pixels. */
    for (i = 0; i < 8; i++, Data++)
    {
        /* Get even alpha and expand into 8-bit. */
        a = *Data & 0x0F;
        *Output++ = a | (a << 4);

        /* Get odd alpha and expand into 8-bit. */
        a = *Data >> 4;
        *Output++ = a | (a << 4);
    }
}

/* Decode 64-bits of alpha information. */
static void
_DecodeDXT5Alpha(
    IN const GLubyte * Data,
    OUT GLubyte * Output
    )
{
    GLint i, j, n;
    GLubyte a[8];
    GLushort bits = 0;

    /* Load alphas 0 and 1. */
    a[0] = Data[0];
    a[1] = Data[1];

    if (a[0] > a[1])
    {
        /* Interpolate alphas 2 through 7. */
        a[2] = (GLubyte) ((6 * a[0] + 1 * a[1]) / 7);
        a[3] = (GLubyte) ((5 * a[0] + 2 * a[1]) / 7);
        a[4] = (GLubyte) ((4 * a[0] + 3 * a[1]) / 7);
        a[5] = (GLubyte) ((3 * a[0] + 4 * a[1]) / 7);
        a[6] = (GLubyte) ((2 * a[0] + 5 * a[1]) / 7);
        a[7] = (GLubyte) ((1 * a[0] + 6 * a[1]) / 7);
    }
    else
    {
        /* Interpolate alphas 2 through 5. */
        a[2] = (GLubyte) ((4 * a[0] + 1 * a[1]) / 5);
        a[3] = (GLubyte) ((3 * a[0] + 2 * a[1]) / 5);
        a[4] = (GLubyte) ((2 * a[0] + 3 * a[1]) / 5);
        a[5] = (GLubyte) ((1 * a[0] + 4 * a[1]) / 5);

        /* Set alphas 6 and 7. */
        a[6] = 0;
        a[7] = 255;
    }

    /* Walk all pixels. */
    for (i = 0, j = 2, n = 0; i < 16; i++, bits >>= 3, n -= 3)
    {
        /* Test if we have enough bits in the accumulator. */
        if (n < 3)
        {
            /* Load another chunk of bits in the accumulator. */
            bits |= Data[j++] << n;
            n += 8;
        }

        /* Copy decoded alpha value. */
        Output[i] = a[bits & 0x7];
    }
}

/* Decompress a DXT texture. */
static void *
_DecompressDXT(
    IN GLsizei Width,
    IN GLsizei Height,
    IN GLsizei ImageSize,
    IN const void * Data,
    IN GLenum InternalFormat,
    IN gceSURF_FORMAT Format
    )
{
    GLubyte * pixels, * line;
    const GLubyte * data;
    GLsizei x, y, stride;
    gctPOINTER pointer = gcvNULL;
    GLubyte alpha[16];
    GLint bpp;

    /* Determine bytes per pixel. */
    bpp = (Format == gcvSURF_R5G6B5) ? 2 : 4;

    /* Allocate the decompressed memory. */
    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, Width * Height * bpp, &pointer)))
    {
        /* Error. */
        glmERROR(GL_OUT_OF_MEMORY);
        return gcvNULL;
    }

    /* Initialize the variables. */
    pixels = pointer;
    stride = Width * bpp;
    data   = Data;

    /* Walk all lines, 4 lines per block. */
    for (y = 0, line = pixels; y < Height; y += 4, line += stride * 4)
    {
        GLubyte * p = line;

        /* Walk all pixels, 4 pixels per block. */
        for (x = 0; x < Width; x += 4)
        {
            /* Dispatch on format. */
            switch (InternalFormat)
            {
            default:
                gcmASSERT(ImageSize >= 8);

                /* Decompress one color block. */
                _DecodeDXTColor16(gcmMIN(Width - x, 4),
                                  gcmMIN(Height - y, 4),
                                  stride,
                                  data,
                                  p);

                /* 8 bytes per block. */
                data      += 8;
                ImageSize -= 8;
                break;

            case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
                gcmASSERT(ImageSize >= 16);

                /* Decode DXT3 alpha. */
                _DecodeDXT3Alpha(data, alpha);

                /* Decompress one color block. */
                _DecodeDXTColor32(gcmMIN(Width - x, 4),
                                  gcmMIN(Height - y, 4),
                                  stride,
                                  data + 8,
                                  alpha,
                                  p);

                /* 16 bytes per block. */
                data      += 16;
                ImageSize -= 16;
                break;

            case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
                gcmASSERT(ImageSize >= 16);

                /* Decode DXT5 alpha. */
                _DecodeDXT5Alpha(data, alpha);

                /* Decompress one color block. */
                _DecodeDXTColor32(gcmMIN(Width - x, 4),
                                  gcmMIN(Height - y, 4),
                                  stride,
                                  data + 8,
                                  alpha,
                                  p);

                /* 16 bytes per block. */
                data      += 16;
                ImageSize -= 16;
                break;
            }

            /* Next block. */
            p += 4 * bpp;
        }
    }

    /* Return pointer to decompressed data. */
    return pixels;
}

/*******************************************************************************
**
**  _UpdateStageEnable
**
**  Update stage enable state.
**
**  INPUT:
**
**      Context
**          Pointer to the glsCONTEXT structure.
**
**      Sampler
**          Pointer to the texture sampler descriptor.
**
**  OUTPUT:
**
**      Nothing.
*/

static void _UpdateStageEnable(
    glsCONTEXT_PTR Context,
    glsTEXTURESAMPLER_PTR Sampler
    )
{
    gctUINT formatIndex;
    gctBOOL mipmapRequired;
    gctUINT index;

    gcmHEADER_ARG("Context=0x%x Sampler=0x%x", Context, Sampler);

    Sampler->stageEnabled = GL_FALSE;

    if ((Sampler->enableTexturing
      || Sampler->enableCubeTexturing
      || Sampler->enableExternalTexturing)
    &&  (Sampler->binding->object != gcvNULL)
    )
    {
        /* Determine whether mipmaps are required. */
        mipmapRequired
            =  (Sampler->binding->minFilter != glvNEAREST)
            && (Sampler->binding->minFilter != glvLINEAR);

        /* Determine maximum LOD. */
        if(!( gcoHAL_IsFeatureAvailable(gcvNULL,gcvFEATURE_HALTI2) && gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_BUG_FIXES18)))
        {
            Sampler->binding->maxLOD = mipmapRequired
                ? Sampler->binding->maxLevel
                : 0;
        }

        /* Determine the current stage enable flag. */
        Sampler->stageEnabled = gcmIS_SUCCESS(gcoTEXTURE_IsComplete(
                                        Sampler->binding->object,
                                        gcvNULL,
                                        0,
                                        (mipmapRequired ? Sampler->binding->maxLevel: 0)));
    }

    /* Get the sampler index. */
    index = Sampler->index;

    if (Sampler->stageEnabled)
    {
        /* Determine the format. */
        switch (Sampler->binding->format)
        {
        case GL_ALPHA:
        case GL_RGB:
        case GL_RGBA:
        case GL_LUMINANCE:
        case GL_LUMINANCE_ALPHA:
            formatIndex = Sampler->binding->format - GL_ALPHA;
            break;
        case GL_BGRA_EXT:
            formatIndex = 5;
            break;
        default:
            /* Invalid request. */
            gcmFOOTER_NO();
            gcmASSERT(gcvFALSE);
            return;
        }

        /* Update the hash key. */
        glmSETHASH_1BIT (hashStageEnabled,  Sampler->stageEnabled, index);
        glmSETHASH_3BITS(hashTextureFormat, formatIndex,  index);
    }
    else
    {
        /* Update the hash key. */
        glmSETHASH_1BIT(hashStageEnabled,  Sampler->stageEnabled, index);

        /* Set to an invalid format. */
        glmCLEARHASH_3BITS(hashTextureFormat, index);
    }
    gcmFOOTER_NO();
}

static gceSTATUS _AddMipMap(
    IN glsCONTEXT_PTR Context,
    IN gcoTEXTURE Texture,
    IN gceSURF_FORMAT Format,
    IN GLint Level,
    IN GLsizei Width,
    IN GLsizei Height,
    IN GLuint Faces
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("Context=0x%x Texture=0x%x Format=0x%04x Level=%d Width=%d Height=%d Faces=%u",
                    Context, Texture, Format, Level, Width, Height, Faces);

    do
    {
        gcoSURF lod0, lod1 = gcvNULL;
        GLsizei newWidth, newHeight;

        /* Compute the new LOD size. */
        newWidth  = Width / 2;
        newHeight = Height / 2;

        /* Reached the smallest level? */
        if ((newWidth == Width) && (newHeight == Height))
        {
            status = gcvSTATUS_MIPMAP_TOO_SMALL;
            break;
        }

        if ((newWidth == 0) && (newHeight == 0))
        {
            status = gcvSTATUS_MIPMAP_TOO_SMALL;
            break;
        }

        if (newWidth == 0)
            newWidth = 1;
        else if (newHeight == 0)
            newHeight = 1;

        /* Get the texture surface. */
        gcmERR_BREAK(gcoTEXTURE_GetMipMap(
            Texture, Level, &lod0
            ));

        /* Create a new level. */
        gcmERR_BREAK(gcoTEXTURE_AddMipMap(
            Texture,
            Level + 1,
            gcvUNKNOWN_MIPMAP_IMAGE_FORMAT,
            Format,
            newWidth, newHeight,
            0, Faces,
            gcvPOOL_DEFAULT,
            &lod1
            ));

#if gcdSYNC
        gcmVERIFY_OK(gcoSURF_SetSharedLock(lod1, Context->texture.textureList->sharedLock));
        /* Why get fence here? */
        gcoSURF_GetFence(lod1, gcvFENCE_TYPE_ALL);
#endif

    }
    while (gcvFALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}

static gceSTATUS _BltMipMap(
    IN glsCONTEXT_PTR Context,
    IN gcoTEXTURE Texture,
    IN gctINT MaxLevel
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("Context=0x%x Texture=0x%x MaxLevel=%d", Context, Texture, MaxLevel);

    gcmONERROR(gcoTEXTURE_GenerateMipMap(Texture, 0, MaxLevel));

    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  _GenerateMipMap
**
**  Allocate the specified mipmap level and generate the image for it.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      Texture
**          Texture object handle.
**
**      Format
**          Format of the mipmap to add.
**
**      Level
**          The level number after which the new level is to be added.
**
**      Width, Height
**          The size of the mipmap level speified by Level parameter.
**
**      Faces
**          Number of faces in the texture.
**
**  OUTPUT:
**
**      Pointer to the format information.
*/

static gceSTATUS _GenerateMipMap(
    IN glsCONTEXT_PTR Context,
    IN gcoTEXTURE Texture,
    IN gceSURF_FORMAT Format,
    IN GLint Level,
    IN GLsizei Width,
    IN GLsizei Height,
    IN GLuint Faces
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("Context=0x%x Texture=0x%x Format=0x%04x Level=%d Width=%d Height=%d Faces=%u",
                    Context, Texture, Format, Level, Width, Height, Faces);

    do
    {
        gcoSURF lod0, lod1 = gcvNULL;
        GLsizei newWidth, newHeight;

        /* Compute the new LOD size. */
        newWidth  = Width / 2;
        newHeight = Height / 2;

        /* Reached the smallest level? */
        if ((newWidth == Width) && (newHeight == Height))
        {
            status = gcvSTATUS_MIPMAP_TOO_SMALL;
            break;
        }

        if ((newWidth == 0) && (newHeight == 0))
        {
            status = gcvSTATUS_MIPMAP_TOO_SMALL;
            break;
        }

        if (newWidth == 0)
            newWidth = 1;
        else if (newHeight == 0)
            newHeight = 1;

        /* Get the texture surface. */
        gcmERR_BREAK(gcoTEXTURE_GetMipMap(
            Texture, Level, &lod0
            ));

        /* Create a new level. */
        gcmERR_BREAK(gcoTEXTURE_AddMipMap(
            Texture,
            Level + 1,
            gcvUNKNOWN_MIPMAP_IMAGE_FORMAT,
            Format,
            newWidth, newHeight,
            0, Faces,
            gcvPOOL_DEFAULT,
            &lod1
            ));

        gcmERR_BREAK(gcoSURF_Resample(lod0, lod1));

#if gcdSYNC
        gcmVERIFY_OK(gcoSURF_SetSharedLock(lod1, Context->texture.textureList->sharedLock));
        gcoSURF_GetFence(lod1, gcvFENCE_TYPE_ALL);
#endif

#ifdef _DUMP_FILE
        {
            gcoDUMP dump;
            gctUINT width, height;
            gctINT stride;
            gctPOINTER memory[3] = {0};
            gctUINT32 address[3] = {gcvNULL};

            gcmVERIFY_OK(gcoSURF_Lock(
                lod1, address, memory
                ));

            gcmVERIFY_OK(gcoSURF_GetAlignedSize(
                lod1, &width, &height, &stride
                ));

            gcmVERIFY_OK(gcoHAL_GetDump(
                Context->hal, &dump
                ));

            gcmVERIFY_OK(gcoDUMP_DumpData(
                dump, gcvTAG_TEXTURE, address[0], height * stride, (char *) memory[0]
                ));

            gcmVERIFY_OK(gcoSURF_Unlock(
                lod1, memory[0]
                ));
        }
#endif
    }
    while (gcvFALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  glfGenerateMipMaps
**
**  Generate mipmap levels for the specified texture.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      Texture
**          Pointer to the texture wrapper object.
**
**      Format
**          Format of the mipmap to add.
**
**      Level
**          The level number after which the new level is to be added.
**
**      Width, Height
**          The size of the mipmap level speified by Level parameter.
**
**      Faces
**          Number of faces in the texture.
**
**  OUTPUT:
**
**      Pointer to the format information.
*/

gceSTATUS glfGenerateMipMaps(
    IN glsCONTEXT_PTR Context,
    IN glsTEXTUREWRAPPER_PTR Texture,
    IN gceSURF_FORMAT Format,
    IN GLint BaseLevel,
    IN GLsizei Width,
    IN GLsizei Height,
    IN GLuint Faces
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("Context=0x%x Texture=0x%x Format=0x%04x Level=%d MaxLeves=%d Width=%d Height=%d Faces=%u",
                    Context, Texture, Format, BaseLevel, Width, Height, Faces);

    do
    {
        GLint level = BaseLevel;

        /* Disable BLT_ENGINE mipmap temporarily. */
        if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_BLT_ENGINE))
        {
            while (gcvTRUE)
            {
                gcmERR_BREAK(_AddMipMap(
                    Context, Texture->object, Format,
                    level, Width, Height, Faces
                    ));

                if (status == gcvSTATUS_MIPMAP_TOO_SMALL)
                {
                    break;
                }

                level += 1;

                Width  = Width / 2;
                Height = Height / 2;
            }

            _BltMipMap(Context, Texture->object, level);
        }
        else
        {
            while (gcvTRUE)
            {
                gcmERR_BREAK(_GenerateMipMap(
                    Context, Texture->object, Format,
                    level, Width, Height, Faces
                    ));

                if (status == gcvSTATUS_MIPMAP_TOO_SMALL)
                {
                    break;
                }

                level += 1;

                Width  = Width / 2;
                Height = Height / 2;
            }
        }

        /* Mark texture as dirty. */
        if (BaseLevel != level)
        {
            /* Invalidate the texture. */
            Texture->dirty = gcvTRUE;
        }
    }
    while (gcvFALSE);

    gcmFOOTER();
    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  glfGetMaxLOD
**
**  Determine the maximum LOD number.
**
**  INPUT:
**
**      Width, Height
**          The size of LOD 0.
**
**  OUTPUT:
**
**      Maximum level number.
*/

gctINT glfGetMaxLOD(
    gctINT Width,
    gctINT Height
    )
{
    gctINT lod = 0;
    gcmHEADER_ARG("Width=%d Height=%d", Width, Height);

    while ((Width > 1) || (Height > 1))
    {
        /* Update the maximum LOD. */
        lod += 1;

        /* Compute the size of the next level. */
        Width  = gcmMAX(Width  / 2, 1);
        Height = gcmMAX(Height / 2, 1);
    }

    gcmFOOTER_ARG("return=%d", lod);
    /* Return the maximum LOD. */
    return lod;
}


/*******************************************************************************
**
**  _SetTextureWrapperFormat
**
**  Set texture format and associated fields.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      Texture
**          Pointer to the texture wrapper object.
**
**      Format
**          Specifies the number of color components in the texture. Must be
**          GL_ALPHA, GL_RGB, GL_RGBA, GL_LUMINANCE, or GL_LUMINANCE_ALPHA.
**
**  OUTPUT:
**
**      Nothing.
*/

static void _SetTextureWrapperFormat(
    glsCONTEXT_PTR Context,
    glsTEXTUREWRAPPER_PTR Texture,
    GLenum Format
    )
{
    gcmHEADER_ARG("Context=0x%x Texture=0x%x Format=0x%04x", Context, Texture, Format);

    /* Set the format. */
    Texture->format = Format;

    /* Set target enable for FS. */
    switch (Format)
    {
    case GL_ALPHA:
        Texture->combineFlow.targetEnable = gcSL_ENABLE_W;
        Texture->combineFlow.tempEnable   = gcSL_ENABLE_X;
        Texture->combineFlow.tempSwizzle  = gcSL_SWIZZLE_XXXX;
        Texture->combineFlow.argSwizzle   = gcSL_SWIZZLE_WWWW;
        break;

    case GL_LUMINANCE:
    case GL_RGB:
        Texture->combineFlow.targetEnable = gcSL_ENABLE_XYZ;
        Texture->combineFlow.tempEnable   = gcSL_ENABLE_XYZ;
        Texture->combineFlow.tempSwizzle  = gcSL_SWIZZLE_XYZZ;
        Texture->combineFlow.argSwizzle   = gcSL_SWIZZLE_XYZZ;
        break;

    case GL_LUMINANCE_ALPHA:
    case GL_RGBA:
    case GL_BGRA_EXT:
        Texture->combineFlow.targetEnable = gcSL_ENABLE_XYZW;
        Texture->combineFlow.tempEnable   = gcSL_ENABLE_XYZW;
        Texture->combineFlow.tempSwizzle  = gcSL_SWIZZLE_XYZW;
        Texture->combineFlow.argSwizzle   = gcSL_SWIZZLE_XYZW;
        break;

    default:
        /* Invalid request. */
        gcmASSERT(gcvFALSE);
    }
    gcmFOOTER_NO();
    return;
}

/*******************************************************************************
**
**  _InitTextureWrapper
**
**  Sets default values to specified texture wrapper object.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      Texture
**          Pointer to the texture wrapper object.
**
**  OUTPUT:
**
**      Nothing.
*/

static void _InitTextureWrapper(
    glsCONTEXT_PTR Context,
    glsTEXTUREWRAPPER_PTR Texture
    )
{
    gcmHEADER_ARG("Context=0x%x Texture=0x%x", Context, Texture);
    /*
    ** Init everything to zero.
    */
    gcoOS_ZeroMemory(Texture, gcmSIZEOF(glsTEXTUREWRAPPER));

    /*
    ** Set binding.
    */
    Texture->binding          = gcvNULL;
    Texture->boundAtLeastOnce = gcvFALSE;

    /*
    ** Default format.
    */
    _SetTextureWrapperFormat(Context, Texture, GL_RGBA);

    /*
    ** Init states.
    */
    Texture->maxLevel  = 1000;
    Texture->maxLOD    = 1000;
    Texture->minFilter = glvNEAREST_MIPMAP_LINEAR;
    Texture->magFilter = glvLINEAR;
    Texture->wrapS     = glvREPEAT;
    Texture->wrapT     = glvREPEAT;
    Texture->genMipmap = GL_FALSE;
    Texture->anisoFilter = 1;

    /* init bind count.*/
    Texture->bindCount = 0;
    Texture->flag = 0;

    gcmFOOTER_NO();
}

/*******************************************************************************
**
**  _ResetTextureWrapper
**
**  Delete objects associated with the wrapper.
**
**  INPUT:
**
**      Context
**          Pointer to the glsCONTEXT structure.
**
**      Texture
**          Pointer to the glsTEXTUREWRAPPER texture wrapper.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS _ResetTextureWrapper(
    glsCONTEXT_PTR Context,
    glsTEXTUREWRAPPER_PTR Texture
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Context=0x%x Texture=0x%x", Context, Texture);

    do
    {
        /* Reset the number of levels. */
        Texture->maxLevel = 1000;
        Texture->maxLOD   = 1000;

        Texture->dirty = gcvFALSE;

        Texture->direct.dirty = gcvFALSE;
        Texture->image.dirty  = gcvFALSE;

        /* Destroy the texture object. */
        if (Texture->object != gcvNULL)
        {
            gcmERR_BREAK(gcoTEXTURE_Destroy(Texture->object));
            Texture->object = gcvNULL;
        }

        /* Remove existing YUV direct structure. */
        if (Texture->direct.source != gcvNULL)
        {
            /* Unlock the source surface. */
            gcmERR_BREAK(gcoSURF_Unlock(
                Texture->direct.source,
                gcvNULL
                ));

            /* Destroy the source surface. */
            gcmERR_BREAK(gcoSURF_Destroy(
                Texture->direct.source
                ));

            Texture->direct.source = gcvNULL;
        }

        /* Dereference EGL image source. */
        if (Texture->image.source != gcvNULL)
        {
            /* Destroy the source surface. */
            gcmERR_BREAK(gcoSURF_Destroy(
                Texture->image.source
                ));

            Texture->image.source = gcvNULL;
        }

        /* Deference EGLImageKHR. */
        if (Texture->image.image != gcvNULL)
        {
            Context->imports.dereferenceImage(Texture->image.image);
            Texture->image.image = gcvNULL;
        }

#if defined(ANDROID)
        /* Dereference android native buffer. */
        if (Texture->image.nativeBuffer != gcvNULL)
        {
            android_native_buffer_t * nativeBuffer;

            /* Cast to android native buffer. */
            nativeBuffer = (android_native_buffer_t *) Texture->image.nativeBuffer;

            /* Decrease native buffer reference count. */
            nativeBuffer->common.decRef(&nativeBuffer->common);
            Texture->image.nativeBuffer = gcvNULL;
        }
#endif
    }
    while (gcvFALSE);

    gcmFOOTER();
    /* Return status. */
    return status;
}

#if gldSUPPORT_SHARED_CONTEXT
/*******************************************************************************
**
**  glfPointTexture
**
**  Set texture list to pointer.
**
**  INPUT:
**
**      Pointer
**      List
**
**  OUTPUT:
**
**      Nothing.
*/

gceSTATUS glfPointTexture(
    glsTEXTURELIST_PTR * Pointer,
    glsTEXTURELIST_PTR List
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("Pointer=0x%x List=0x%x", Pointer, List);

    if (List == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER_ARG("return=%s", status);
        return status;
    }

    if(List->sharedLock == gcvNULL)
    {
           gcoOS_CreateMutex(
                gcvNULL,
                &List->sharedLock
                );
    }
    do
    {
        /* lock */
        gcmERR_BREAK(gcoOS_AcquireMutex(
            gcvNULL, List->sharedLock, gcvINFINITE));

        *Pointer = List;

        List->reference++;

        /* Unlock */
        gcmERR_BREAK(gcoOS_ReleaseMutex(
            gcvNULL,
            List->sharedLock));
    }
    while (GL_FALSE);

    gcmFOOTER_ARG("return=%s", status);
    return status;
}
#endif

/*******************************************************************************
**
**  glfInitializeTexture
**
**  Constructs texture management object.
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

gceSTATUS glfInitializeTexture(
    glsCONTEXT_PTR Context
    )
{
    gceSTATUS status;

    static GLfloat textureFunction   = GL_MODULATE;
    static GLfloat combColorFunction = GL_MODULATE;
    static GLfloat combAlphaFunction = GL_MODULATE;

    static GLfloat combColorSource0 = GL_TEXTURE;
    static GLfloat combColorSource1 = GL_PREVIOUS;
    static GLfloat combColorSource2 = GL_CONSTANT;

    static GLfloat combColorOperand0 = GL_SRC_COLOR;
    static GLfloat combColorOperand1 = GL_SRC_COLOR;
    static GLfloat combColorOperand2 = GL_SRC_ALPHA;

    static GLfloat combAlphaSource0 = GL_TEXTURE;
    static GLfloat combAlphaSource1 = GL_PREVIOUS;
    static GLfloat combAlphaSource2 = GL_CONSTANT;

    static GLfloat combAlphaOperand0 = GL_SRC_ALPHA;
    static GLfloat combAlphaOperand1 = GL_SRC_ALPHA;
    static GLfloat combAlphaOperand2 = GL_SRC_ALPHA;

    static GLfloat textureCoordGenMode = GL_REFLECTION_MAP_OES;

    static GLfloat vec0000[] =
    {
        glvFLOATZERO, glvFLOATZERO, glvFLOATZERO, glvFLOATZERO
    };

    static GLfloat value1 = glvFLOATONE;

    gcmHEADER_ARG("Context=0x%x", Context);

    do
    {
        gctUINT texMaxWidth;
        gctUINT texMaxHeight;
        gctUINT texMaxDepth;
        gctBOOL texCubic;
        gctBOOL texNonPowerOfTwo;
        gctUINT texPixelSamplers;
        GLuint samplerSize, j;
        GLint i;
        gctPOINTER pointer = gcvNULL;

        /* Query the texture caps. */
        gcmERR_BREAK(gcoTEXTURE_QueryCaps(
            Context->hal,
            &texMaxWidth,
            &texMaxHeight,
            &texMaxDepth,
            &texCubic,
            &texNonPowerOfTwo,
            gcvNULL,
            &texPixelSamplers
            ));

        /* Limit to API defined value. */
        if (texPixelSamplers > glvMAX_TEXTURES)
        {
            texPixelSamplers = glvMAX_TEXTURES;
        }

        /* Make sure we have samplers. */
        if (texPixelSamplers == 0)
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

        /* Enable DrawTex position stream. */
        Context->aPositionDrawTexInfo.streamEnabled = GL_TRUE;

        Context->hwPointSprite = gcvFALSE;

#if !gldSUPPORT_SHARED_CONTEXT
        if (Context->texture.textureList == gcvNULL)
        {
            gcmERR_BREAK(gcoOS_Allocate(
                gcvNULL,
                sizeof(glsTEXTURELIST),
                (gctPOINTER *)&Context->texture.textureList
                ));

            /* Zero buffers. */
            gcoOS_ZeroMemory(Context->texture.textureList,
                sizeof(glsTEXTURELIST));
        }
#endif

        /* Allocate and init texture sampler structures. */
        samplerSize = texPixelSamplers * gcmSIZEOF(glsTEXTURESAMPLER);
        gcmERR_BREAK(gcoOS_Allocate(
            gcvNULL,
            samplerSize,
            &pointer
            ));

        Context->texture.sampler = pointer;

        /* Reset to zero. */
        gcoOS_ZeroMemory(Context->texture.sampler, samplerSize);

        /* Init default textures. */
        for (j = 0; j < gcmCOUNTOF(Context->texture.defaultTexture); j += 1)
        {
            _InitTextureWrapper(Context, &Context->texture.defaultTexture[j]);
        }

        /* Init active samplers. */
        Context->texture.activeSampler = &Context->texture.sampler[0];
        Context->texture.activeSamplerIndex = 0;
        Context->texture.activeClientSampler = &Context->texture.sampler[0];
        Context->texture.activeClientSamplerIndex = 0;

        /* Init texture caps. */
        Context->texture.maxWidth           = (GLuint) texMaxWidth;
        Context->texture.maxHeight          = (GLuint) texMaxHeight;
        Context->texture.maxDepth           = (GLuint) texMaxDepth;
        Context->texture.cubic              = (GLboolean) texCubic;
        Context->texture.nonPowerOfTwo      = (GLboolean) texNonPowerOfTwo;
        Context->texture.pixelSamplers      = (GLint) texPixelSamplers;
        Context->texture.generateMipmapHint = GL_DONT_CARE;
        Context->texture.matrixDirty        = GL_FALSE;

        /* Init the samplers. */
        for (i = 0; i < Context->texture.pixelSamplers; i++)
        {
            /* Get a shortcut to the current sampler. */
            glsTEXTURESAMPLER_PTR sampler = &Context->texture.sampler[i];

            /* Set the index. */
            sampler->index = i;

            /* Initialize default bindings. */
            for (j = 0; j < gcmCOUNTOF(Context->texture.defaultTexture); j += 1)
            {
                sampler->bindings[j] = &Context->texture.defaultTexture[j];
            }

            /* Bind to default 2D texture. */
            sampler->binding = sampler->bindings[glvTEXTURE2D];

            /* Set data flow structute pointers. */
            sampler->combColor.combineFlow = &sampler->colorDataFlow;
            sampler->combAlpha.combineFlow = &sampler->alphaDataFlow;

            /* Set defaults for alapha data flow. */
            sampler->alphaDataFlow.targetEnable = gcSL_ENABLE_W;
            sampler->alphaDataFlow.tempEnable   = gcSL_ENABLE_X;
            sampler->alphaDataFlow.tempSwizzle  = gcSL_SWIZZLE_XXXX;
            sampler->alphaDataFlow.argSwizzle   = gcSL_SWIZZLE_WWWW;

            /* Set default states. */
            sampler->coordReplace = GL_FALSE;

            /* Enable DrawTex texture coordinate stream. */
            sampler->aTexCoordDrawTexInfo.streamEnabled = GL_TRUE;

            gcmVERIFY(_SetTextureFunction(
                Context, sampler,
                &textureFunction
                ));

            gcmVERIFY(_SetCombineColorFunction(
                Context, sampler,
                &combColorFunction
                ));

            gcmVERIFY(_SetCombineAlphaFunction(
                Context, sampler,
                &combAlphaFunction
                ));

            gcmVERIFY(_SetCombineColorSource(
                Context, GL_SRC0_RGB, sampler,
                &combColorSource0
                ));

            gcmVERIFY(_SetCombineColorSource(
                Context, GL_SRC1_RGB, sampler,
                &combColorSource1
                ));

            gcmVERIFY(_SetCombineColorSource(
                Context, GL_SRC2_RGB, sampler,
                &combColorSource2
                ));

            gcmVERIFY(_SetCombineAlphaSource(
                Context, GL_SRC0_ALPHA, sampler,
                &combAlphaSource0
                ));

            gcmVERIFY(_SetCombineAlphaSource(
                Context, GL_SRC1_ALPHA, sampler,
                &combAlphaSource1
                ));

            gcmVERIFY(_SetCombineAlphaSource(
                Context, GL_SRC2_ALPHA, sampler,
                &combAlphaSource2
                ));

            gcmVERIFY(_SetCombineColorOperand(
                Context, GL_OPERAND0_RGB, sampler,
                &combColorOperand0
                ));

            gcmVERIFY(_SetCombineColorOperand(
                Context, GL_OPERAND1_RGB, sampler,
                &combColorOperand1
                ));

            gcmVERIFY(_SetCombineColorOperand(
                Context, GL_OPERAND2_RGB, sampler,
                &combColorOperand2
                ));

            gcmVERIFY(_SetCombineAlphaOperand(
                Context, GL_OPERAND0_ALPHA, sampler,
                &combAlphaOperand0
                ));

            gcmVERIFY(_SetCombineAlphaOperand(
                Context, GL_OPERAND1_ALPHA, sampler,
                &combAlphaOperand1
                ));

            gcmVERIFY(_SetCombineAlphaOperand(
                Context, GL_OPERAND2_ALPHA, sampler,
                &combAlphaOperand2
                ));

            gcmVERIFY(_SetTexCoordGenMode(
                Context, sampler,
                &textureCoordGenMode
                ));

            gcmVERIFY(_SetCurrentColor(
                Context, sampler,
                vec0000
                ));

            gcmVERIFY(_SetColorScale(
                Context, sampler,
                &value1
                ));

            gcmVERIFY(_SetAlphaScale(
                Context, sampler,
                &value1
                ));
        }
    }
    while (GL_FALSE);

    gcmFOOTER();
    /* Return result. */
    return status;
}


/*******************************************************************************
**
**  glfFlushTexture
**
**  Flush texture states.
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
gceSTATUS glfFlushTexture(
    IN glsCONTEXT_PTR Context
    )
{
    gctINT i;

    gcmHEADER_ARG("Context=0x%x", Context);
    Context->texture.matrixDirty = gcvTRUE;

    for (i = 0; i < Context->texture.pixelSamplers; i++)
    {
        /* Get a shortcut to the current sampler. */
        glsTEXTURESAMPLER_PTR sampler = &Context->texture.sampler[i];

        if (sampler->binding != gcvNULL)
        {
            sampler->binding->dirty = gcvTRUE;
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}


/*******************************************************************************
**
**  glfDestroyTexture
**
**  Destructs texture management object.
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

gceSTATUS glfDestroyTexture(
    glsCONTEXT_PTR Context
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Context=0x%x", Context);

    do
    {
        GLuint i;
        glsTEXTUREWRAPPER_PTR texture;

        if (gcvNULL == Context->texture.textureList)
        {
            status = gcvSTATUS_INVALID_OBJECT;
            return status;
        }
        /* Destroy the default textures. */
        for (i = 0; i < gcmCOUNTOF(Context->texture.defaultTexture); i += 1)
        {
            gcmERR_BREAK(_ResetTextureWrapper(
                Context,
                &Context->texture.defaultTexture[i]
            ));
        }

        /* Failed? */
        if (gcmIS_ERROR(status))
        {
            break;
        }

        /* Delete samplers. */
        if (Context->texture.sampler != gcvNULL)
        {
            gcmERR_BREAK(gcmOS_SAFE_FREE(
                gcvNULL, Context->texture.sampler
                ));
        }

#if gldSUPPORT_SHARED_CONTEXT
        if(Context->texture.textureList->sharedLock != gcvNULL)
        {
            gcmERR_BREAK(gcoOS_AcquireMutex(
                gcvNULL,
                Context->texture.textureList->sharedLock,
                gcvINFINITE
                ));
        }

        if (--Context->texture.textureList->reference == 0)
#endif
        {
            /* Delete textures. */
            for (i=0; i<NAMED_TEXTURE_HASH; i++)
            {
                for (texture = Context->texture.textureList->objects[i]; texture != gcvNULL;)
                {
                    /* Save the curent node. */
                    glsTEXTUREWRAPPER_PTR node = texture;

                    /* Advance to the next node. */
                    texture = texture->next;

                    /* Destroy the texture. */
                    gcmERR_BREAK(_ResetTextureWrapper(Context, node));

                    /* Destroy the wrapper. */
                    gcmERR_BREAK(gcmOS_SAFE_FREE(gcvNULL, node));
                }
            }
        }
    }
    while (gcvFALSE);

#if gldSUPPORT_SHARED_CONTEXT
    if(Context->texture.textureList->sharedLock != gcvNULL)
    {
        gcmVERIFY_OK(gcoOS_ReleaseMutex(
            gcvNULL,
            Context->texture.textureList->sharedLock
            ));
    }

    if ((Context->texture.textureList != gcvNULL) && (Context->texture.textureList->reference == 0))
    {
        if(Context->texture.textureList->sharedLock != gcvNULL)
        {
            gcmVERIFY_OK(gcoOS_DeleteMutex(
                gcvNULL,
                Context->texture.textureList->sharedLock));

            Context->texture.textureList->sharedLock = gcvNULL;
        }
    }

    if ((Context->texture.textureList != gcvNULL) && (Context->texture.textureList->reference == 0))
#endif
    {
        gcoOS_Free(gcvNULL,Context->texture.textureList);
        Context->texture.textureList = gcvNULL;
    }


    gcmFOOTER();

    /* Return status. */
    return status;
}

/*******************************************************************************
**
**  glfFindTexture
**
**  Returns a pointer to the specified texture.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      Texture
**          The name of the texture to find.
**
**  OUTPUT:
**
**      Pointer to the texture if found.
*/

glsTEXTUREWRAPPER_PTR glfFindTexture(
    glsCONTEXT_PTR Context,
    GLuint Texture
    )
{
    glsTEXTUREWRAPPER_PTR texture;
    glsCONTEXT_PTR shared;

    gcmHEADER_ARG("Context=0x%x Texture=0x%08x", Context, Texture);

    /* Map shared context. */
#if gcdRENDER_THREADS
    shared = (Context->shared != gcvNULL) ? Context->shared : Context;
#else
    shared = Context;
#endif

    texture = _glffFindTexture(shared->texture.textureList, Texture);
    if (texture != gcvNULL)
    {
        gcmFOOTER_ARG("texture=0x%08x", texture);
        return texture;
    }

    gcmFOOTER_ARG("texture=0x%x", gcvNULL);
    /* No such texture. */
    return gcvNULL;
}


/*******************************************************************************
**
**  glfInitializeTempBitmap
**
**  Initialize the temporary bitmap image.
**
**  INPUT:
**
**      Context
**          Pointer to the context.
**
**      Format
**          Format of the image.
**
**      Width, Height
**          The size of the image.
**
** OUTPUT:
**
**      Nothing.
*/

gceSTATUS glfInitializeTempBitmap(
    IN glsCONTEXT_PTR Context,
    IN gceSURF_FORMAT Format,
    IN gctUINT Width,
    IN gctUINT Height
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcoSURF bitmap = gcvNULL;
    gcmHEADER_ARG("Context=0x%x Format=0x%04x Width=%u Height=%u",
                    Context, Format, Width, Height);

    do
    {
        /* See if the existing surface can be reused. */
        if ((Context->tempWidth  < Width)  ||
            (Context->tempHeight < Height) ||
            (Context->tempFormat != Format))
        {
            gctUINT width;
            gctUINT height;
            gctINT stride;
            gctPOINTER bits[3];
            gcsSURF_FORMAT_INFO_PTR info[2];

            /* Is there a surface allocated? */
            if (Context->tempBitmap != gcvNULL)
            {
                /* Unlock the surface. */
                if (Context->tempBits != gcvNULL)
                {
                    gcmERR_BREAK(gcoSURF_Unlock(
                        Context->tempBitmap, Context->tempBits
                        ));

                    Context->tempBits = gcvNULL;
                }

                /* Destroy the surface. */
                gcmERR_BREAK(gcoSURF_Destroy(Context->tempBitmap));

                /* Reset temporary surface. */
                Context->tempBitmap       = gcvNULL;
                Context->tempFormat       = gcvSURF_UNKNOWN;
                Context->tempBitsPerPixel = 0;
                Context->tempWidth        = 0;
                Context->tempHeight       = 0;
                Context->tempStride       = 0;
            }

            /* Valid surface requested? */
            if (Format != gcvSURF_UNKNOWN)
            {
                /* Round up the size. */
                width  = gcmALIGN(Width,  256);
                height = gcmALIGN(Height, 256);

                /* Allocate a new surface. */
                gcmONERROR(gcoSURF_Construct(
                    Context->hal,
                    width, height, 1,
                    gcvSURF_BITMAP, Format,
                    gcvPOOL_UNIFIED,
                    &bitmap
                    ));

                /* Get the pointer to the bits. */
                gcmONERROR(gcoSURF_Lock(
                    bitmap, gcvNULL, bits
                    ));

                /* Query the parameters back. */
                gcmONERROR(gcoSURF_GetAlignedSize(
                    bitmap, &width, &height, &stride
                    ));

                /* Query format specifics. */
                gcmONERROR(gcoSURF_QueryFormat(
                    Format, info
                    ));

                /* Set information. */
                Context->tempBitmap       = bitmap;
                Context->tempBits         = bits[0];
                Context->tempFormat       = Format;
                Context->tempBitsPerPixel = info[0]->bitsPerPixel;
                Context->tempWidth        = width;
                Context->tempHeight       = height;
                Context->tempStride       = stride;
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

OnError:
    if (bitmap != gcvNULL)
    {
        gcoSURF_Destroy(bitmap);
        bitmap = gcvNULL;
    }
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  glfResolveDrawToTempBitmap
**
**  Resolve specified area of the drawing surface to the temporary bitmap.
**
**  INPUT:
**
**      Context
**          Pointer to the context.
**
**      SourceX, SourceY, Width, Height
**          The origin and the size of the image.
**
** OUTPUT:
**
**      Nothing.
*/

gceSTATUS glfResolveDrawToTempBitmap(
    IN glsCONTEXT_PTR Context,
    IN gctINT SourceX,
    IN gctINT SourceY,
    IN gctINT Width,
    IN gctINT Height
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Context=0x%x SourceX=%d SourceY=%d Width=%d Height=%d",
                    Context, SourceX, SourceY, Width, Height);

    do
    {
        gctUINT resX       = 0;
        gctUINT resY       = 0;
        gctUINT resW       = 0;
        gctUINT resH       = 0;

        gctUINT sourceX    = 0;
        gctUINT sourceY    = 0;

        /* Clamp coordinates. */
        gctINT left        = gcmMAX(SourceX, 0);
        gctINT top         = gcmMAX(SourceY, 0);
        gctINT right       = gcmMIN(SourceX + Width,  (GLint) Context->drawWidth);
        gctINT bottom      = gcmMIN(SourceY + Height, (GLint) Context->drawHeight);

        gcsSURF_VIEW srcView = {Context->draw, 0, 1};
        gcsSURF_VIEW tmpView = {gcvNULL, 0, 1};
        gcsSURF_RESOLVE_ARGS rlvArgs = {0};

        if ((right <= 0) || (bottom <= 0))
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            gcmFOOTER();
            return status;
        }

        gcmERR_BREAK(gcoSURF_GetResolveAlignment(Context->draw,
                                                 &resX,
                                                 &resY,
                                                 &resW,
                                                 &resH));

        /* Convert GL coordinates. */
        sourceX = left;
        sourceY = top;

        if(Context->drawYInverted)
        {
            sourceY = Context->drawHeight - bottom;
        }
        else
        {
            sourceY = top;
        }

        rlvArgs.version = gcvHAL_ARG_VERSION_V2;
        rlvArgs.uArgs.v2.yInverted = Context->drawYInverted;
        rlvArgs.uArgs.v2.numSlices  = 1;

        /* Determine the aligned source origin. */
        rlvArgs.uArgs.v2.srcOrigin.x = sourceX & ~(resX - 1);
        rlvArgs.uArgs.v2.srcOrigin.y = sourceY & ~(resY - 1);
        if ((rlvArgs.uArgs.v2.srcOrigin.x + (gctINT) resW > (GLint) Context->drawWidth)
        &&  (rlvArgs.uArgs.v2.srcOrigin.x > 0)
        )
        {
            rlvArgs.uArgs.v2.srcOrigin.x = (Context->drawWidth - resW) & ~(resX - 1);
        }

        /* Determine the origin adjustment. */
        Context->tempX = sourceX - rlvArgs.uArgs.v2.srcOrigin.x;
        Context->tempY = sourceY - rlvArgs.uArgs.v2.srcOrigin.y;

        /* Determine the aligned area size. */
        rlvArgs.uArgs.v2.rectSize.x = (gctUINT) gcmALIGN(right  - left + Context->tempX, resW);
        rlvArgs.uArgs.v2.rectSize.y = (gctUINT) gcmALIGN(bottom - top  + Context->tempY, resH);

        /* Initialize the temporary surface. */
        /* FIXME: Bug #4219. */
        gcmERR_BREAK(glfInitializeTempBitmap(
            Context,
            Context->drawFormatInfo[0]->format,
            Width,
            Height
            ));

        tmpView.surf = Context->tempBitmap;
        gcmERR_BREAK(gcoSURF_ResolveRect(&srcView, &tmpView, &rlvArgs));

        /* Make sure the operation is complete. */
        gcmERR_BREAK(gcoHAL_Commit(Context->hal, gcvTRUE));

        if (Context->drawYInverted)
        {
            /* Compute the pointer to the last line. */
            Context->tempLastLine
                =  Context->tempBits
                +  Context->tempStride *  (rlvArgs.uArgs.v2.rectSize.y - (Context->tempY + (bottom - top)))
                + (Context->tempX      *  Context->tempBitsPerPixel) / 8;
        }
        else
        {
            /* Compute the pointer to the last line. */
            Context->tempLastLine
                =  Context->tempBits
                +  Context->tempStride *  Context->tempY
                + (Context->tempX      *  Context->tempBitsPerPixel) / 8;

        }
    }
    while (gcvFALSE);

    gcmFOOTER();
    /* Return status. */
    return status;
}

/*******************************************************************************
**
**  glfLoadTexture / glfUnloadTexture
**
**  Texture state and binding/unbinding functions.
**
**  INPUT:
**
**      Context
**          Pointer to the glsCONTEXT structure.
**
**  OUTPUT:
**
**      Nothing.
*/

gceSTATUS glfLoadTexture(
    glsCONTEXT_PTR Context
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    static gceTEXTURE_FILTER halMipFilter[] =
    {
        gcvTEXTURE_NONE,    /* glvNEAREST */
        gcvTEXTURE_NONE,    /* glvLINEAR */
        gcvTEXTURE_POINT,   /* glvNEAREST_MIPMAP_NEAREST */
        gcvTEXTURE_POINT,   /* glvLINEAR_MIPMAP_NEAREST */
        gcvTEXTURE_LINEAR,  /* glvNEAREST_MIPMAP_LINEAR */
        gcvTEXTURE_LINEAR,  /* glvLINEAR_MIPMAP_LINEAR */
    };

    static gceTEXTURE_FILTER halMinFilter[] =
    {
        gcvTEXTURE_POINT,   /* glvNEAREST */
        gcvTEXTURE_LINEAR,  /* glvLINEAR */
        gcvTEXTURE_POINT,   /* glvNEAREST_MIPMAP_NEAREST */
        gcvTEXTURE_LINEAR,  /* glvLINEAR_MIPMAP_NEAREST */
        gcvTEXTURE_POINT,   /* glvNEAREST_MIPMAP_LINEAR */
        gcvTEXTURE_LINEAR,  /* glvLINEAR_MIPMAP_LINEAR */
    };

    static gceTEXTURE_FILTER halMagFilter[] =
    {
        gcvTEXTURE_POINT,   /* glvNEAREST */
        gcvTEXTURE_LINEAR,  /* glvLINEAR */
    };

    static gceTEXTURE_ADDRESSING halWrap[] =
    {
        gcvTEXTURE_CLAMP,   /* glvCLAMP */
        gcvTEXTURE_WRAP,    /* glvREPEAT */
        gcvTEXTURE_MIRROR,  /* glvMIRROR */
    };

    gcmHEADER_ARG("Context=0x%x", Context);

    do
    {
        gctINT i;

        /* Make a shortcut to the texture attribute array. */
        glsUNIFORMWRAP_PTR* attrTexture = Context->currProgram->fs.texture;

        /* Iterate though the attributes. */
        for (i = 0; i < glvMAX_TEXTURES; i++)
        {
            gctUINT32 samplerNumber;
            gctUINT32 samplerBase;
            glsTEXTURESAMPLER_PTR sampler;
            glsTEXTUREWRAPPER_PTR texture;
            gcsTEXTURE halTexture;

            /* Skip if the texture is not used. */
            if (attrTexture[i] == gcvNULL)
            {
                continue;
            }

            /* Get a shortcut to the current sampler. */
            sampler = &Context->texture.sampler[i];

            /* Make sure the stage is valid. */
            gcmASSERT(sampler->stageEnabled);
            gcmASSERT(sampler->binding != gcvNULL);
            gcmASSERT(sampler->binding->object != gcvNULL);

            /* Get a shortcut to the current sampler's bound texture. */
            texture = sampler->binding;

            /* Flush texture cache. */
            if (texture->dirty)
            {
                gcmERR_BREAK(gcoTEXTURE_Flush(texture->object));
                texture->dirty = gcvFALSE;
            }

            /* Program states. */
            gcoTEXTURE_InitParams(Context->hal, &halTexture);
            halTexture.s = halWrap[texture->wrapS];
            halTexture.t = halWrap[texture->wrapT];

            /* Use same logic for all platforms*/
            if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NON_POWER_OF_TWO) != gcvSTATUS_TRUE)
            {
                /* Set addressing mode to clamp for npot texture temp. */
                if ( (texture->width  & (texture->width  - 1))
                ||   (texture->height & (texture->height - 1))
                )
                {
                    halTexture.s = gcvTEXTURE_CLAMP;
                    halTexture.t = gcvTEXTURE_CLAMP;
                }
            }

            halTexture.minFilter = halMinFilter[texture->minFilter];
            halTexture.magFilter = halMagFilter[texture->magFilter];
            halTexture.mipFilter = halMipFilter[texture->minFilter];
            halTexture.anisoFilter = texture->anisoFilter;

            halTexture.lodMax  = (gctFLOAT)texture->maxLOD;
            halTexture.lodBias = Context->texture.activeSampler->lodBias;

            halTexture.maxLevel = texture->maxLevel;
            halTexture.baseLevel = 0;

            /* Get the sampler number. */
            gcmERR_BREAK(gcUNIFORM_GetSampler(
                attrTexture[i]->uniform,
                &samplerNumber
                ));

            samplerBase = GetShaderSamplerBaseOffset(Context->currProgram->fs.shader);
            samplerNumber += samplerBase;

            if (Context->hasTxDescriptor)
            {
                /* Bind tescriptor to the sampler. */
                gcmERR_BREAK(gcoTEXTURE_BindTextureDesc(
                    texture->object,
                    samplerNumber,
                    &halTexture,
                    0
                    ));

            }
            else
            {
                /* Bind to the sampler. */
                gcmERR_BREAK(gcoTEXTURE_BindTexture(
                    texture->object,
                    0,
                    samplerNumber,
                    &halTexture
                    ));
            }
#if gcdSYNC
            {
                /* Only Mip 0 now, latter I may add other mipmap base on the filter */
                gcoSURF map = gcvNULL;
                gcoTEXTURE_GetMipMap(texture->object,0,&map);

                gcoSURF_GetFence(map, gcvFENCE_TYPE_READ);
            }
#endif
        }
    }
    while (GL_FALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}

/*******************************************************************************
**
**  _SetTextureParameter/_GetTextureParameter
**
**  Texture parameter access.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      Target
**          Specifies the target texture.
**
**      Name
**          Specifies the symbolic name of a single-valued texture parameter.
**
**      Value
**          Points to the value of 'Name'.
**
**      Type
**          Type of the value.
**
**  OUTPUT:
**
**      Nothing.
*/

static GLboolean _SetTextureParameter(
    glsCONTEXT_PTR Context,
    GLenum Target,
    GLenum Name,
    const GLfloat* Value
    )
{
    GLboolean result;
    gcmHEADER_ARG("Context=0x%x Target=0x%04x Name=0x%04x Value=0x%x",
                    Context, Target, Name, Value);

    do
    {
        gleTARGETTYPE target;
        glsTEXTUREWRAPPER_PTR texture;
        GLuint value;

        /* Determine the target. */
        if (Target == GL_TEXTURE_2D)
        {
            /* 2D target. */
            target = glvTEXTURE2D;
        }
        else if (Target == GL_TEXTURE_CUBE_MAP_OES)
        {
            /* Cubemap target. */
            target = glvCUBEMAP;
        }
        else if (Target == GL_TEXTURE_EXTERNAL_OES)
        {
            target = glvTEXTUREEXTERNAL;
        }
        else
        {
            /* Invalid target. */
            result = GL_FALSE;
            break;
        }

        /* Get a shortcut to the active texture. */
        texture = Context->texture.activeSampler->bindings[target];
        gcmASSERT(texture != gcvNULL);

        /* Dispatch. */
        switch (Name)
        {
        case GL_TEXTURE_MIN_FILTER:
            if ((result = glfConvertGLEnum(
                    _TextureMinFilterNames,
                    gcmCOUNTOF(_TextureMinFilterNames),
                    Value,
                    glvFLOAT,
                    &value
                    )) != GL_FALSE)
            {
                if ((target == glvTEXTUREEXTERNAL) &&
                    (value  != glvLINEAR) &&
                    (value  != glvNEAREST))
                {
                    /* Only accept GL_NEAREST/GL_LINEAR for min filter. */
                    result = GL_FALSE;
                    break;
                }

                texture->minFilter = (gleTEXTUREFILTER) value;
            }
            break;

        case GL_TEXTURE_MAG_FILTER:
            if ((result = glfConvertGLEnum(
                    _TextureMagFilterNames,
                    gcmCOUNTOF(_TextureMagFilterNames),
                    Value,
                    glvFLOAT,
                    &value
                    )) != GL_FALSE)
            {
                texture->magFilter = (gleTEXTUREFILTER) value;
            }
            break;

        case GL_TEXTURE_MAX_ANISOTROPY_EXT:
            {
                /* Convert the enum. */
                GLint value = glmFLOAT2INT(*Value);

                if (value < 1)
                {
                    result = GL_FALSE;
                }
                else
                {
                    /* Clamp the value to maxAniso supported. */
                    value = gcmMIN(value, (GLint)Context->maxAniso);

                    texture->anisoFilter = value;

                    result = GL_TRUE;
                }
            }
            break;

        case GL_TEXTURE_WRAP_S:
            if ((result = glfConvertGLEnum(
                    _TextureWrapNames,
                    gcmCOUNTOF(_TextureWrapNames),
                    Value,
                    glvFLOAT,
                    &value
                    )) != GL_FALSE)
            {
                if ((target == glvTEXTUREEXTERNAL) &&
                    (value  != glvCLAMP))
                {
                    /* Only accept GL_CLAMP_TO_EDGE for wrap method. */
                    result = GL_FALSE;
                    break;
                }

                texture->wrapS = (gleTEXTUREWRAP) value;
            }
            break;

        case GL_TEXTURE_WRAP_T:
            if ((result = glfConvertGLEnum(
                    _TextureWrapNames,
                    gcmCOUNTOF(_TextureWrapNames),
                    Value,
                    glvFLOAT,
                    &value
                    )) != GL_FALSE)
            {
                if ((target == glvTEXTUREEXTERNAL) &&
                    (value  != glvCLAMP))
                {
                    /* Only accept GL_CLAMP_TO_EDGE for wrap method. */
                    result = GL_FALSE;
                    break;
                }

                texture->wrapT = (gleTEXTUREWRAP) value;
            }
            break;

        case GL_GENERATE_MIPMAP:
            if ((result = glfConvertGLboolean(
                    Value,
                    glvFLOAT,
                    &value
                    )) != GL_FALSE)
            {
                if ((target == glvTEXTUREEXTERNAL) &&
                    (value  != 0))
                {
                    /* Only accept 'FALSE' for mipmap generation. */
                    result = GL_FALSE;
                    break;
                }

                texture->genMipmap = value != 0;
            }
            break;

        case GL_TEXTURE_CROP_RECT_OES:
            glfGetFromFloatArray(
                Value,
                4,
                &texture->cropRect,
                glvINT
                );

            /* Invalidate normalized crop rectangle. */
            texture->dirtyCropRect = GL_TRUE;

            /* Success. */
            result = GL_TRUE;
            break;

        case GL_TEXTURE_MAX_LEVEL_APPLE:
            {
                GLint maxLevel;

                glfGetFromFloatArray(
                    Value,
                    1,
                    &maxLevel,
                    glvINT
                );

                if (maxLevel > 0)
                {
                    GLint tValue;
                    tValue = glfGetMaxLOD(texture->width, texture->height);
                    texture->maxLevel = gcmMIN(tValue, maxLevel);
                    result = GL_TRUE;
                }
                else
                {
                    result = GL_FALSE;
                }
                break;
            }

        default:
            result = GL_FALSE;
        }
    }
    while (gcvFALSE);

    gcmFOOTER_ARG("result=%d", result);
    /* Return result. */
    return result;
}

static GLboolean _GetTextureParameter(
    glsCONTEXT_PTR Context,
    GLenum Target,
    GLenum Name,
    GLvoid* Value,
    gleTYPE Type
    )
{
    GLboolean result = GL_TRUE;
    gcmHEADER_ARG("Context=0x%x Target=0x%04x Name=0x%04x Value=0x%x Type=0x%04x",
                    Context, Target, Name, Value, Type);

    do
    {
        gleTARGETTYPE target;
        glsTEXTUREWRAPPER_PTR texture;

        /* Determine the target. */
        if (Target == GL_TEXTURE_2D)
        {
            /* 2D target. */
            target = glvTEXTURE2D;
        }
        else if (Target == GL_TEXTURE_CUBE_MAP_OES)
        {
            /* Cubemap target. */
            target = glvCUBEMAP;
        }
        else if (Target == GL_TEXTURE_EXTERNAL_OES)
        {
            target = glvTEXTUREEXTERNAL;
        }
        else
        {
            /* Invalid target. */
            result = GL_FALSE;
            break;
        }

        /* Get a shortcut to the active texture. */
        texture = Context->texture.activeSampler->bindings[target];
        gcmASSERT(texture != gcvNULL);

        /* Dispatch. */
        switch (Name)
        {
        case GL_TEXTURE_MIN_FILTER:
            glfGetFromEnum(
                _TextureMinFilterNames[texture->minFilter],
                Value,
                Type
                );
            break;

        case GL_TEXTURE_MAG_FILTER:
            glfGetFromEnum(
                _TextureMagFilterNames[texture->magFilter],
                Value,
                Type
                );
            break;

        case GL_TEXTURE_WRAP_S:
            glfGetFromEnum(
                _TextureWrapNames[texture->wrapS],
                Value,
                Type
                );
            break;

        case GL_TEXTURE_WRAP_T:
            glfGetFromEnum(
                _TextureWrapNames[texture->wrapT],
                Value,
                Type
                );
            break;

        case GL_GENERATE_MIPMAP:
            glfGetFromInt(
                texture->genMipmap,
                Value,
                Type
                );
            break;

        case GL_TEXTURE_CROP_RECT_OES:
            glfGetFromIntArray(
                (const GLint*) &texture->cropRect,
                4,
                Value,
                Type
                );
            break;

        case GL_TEXTURE_MAX_LEVEL_APPLE:
            glfGetFromInt(
                texture->maxLevel,
                Value,
                Type
                );
            break;

        case GL_TEXTURE_MAX_ANISOTROPY_EXT:
            glfGetFromInt(
                (GLint)texture->anisoFilter,
                Value,
                Type
                );
            break;

        case GL_REQUIRED_TEXTURE_IMAGE_UNITS_OES:
            glfGetFromInt(
                1,
                Value,
                Type
                );
            break;

        default:
            result = GL_FALSE;
        }
    }
    while (gcvFALSE);

    gcmFOOTER_ARG("result=%d", result);
    /* Return result. */
    return result;
}


/*******************************************************************************
**
**  _SetTextureEnvironment/_GetTextureEnvironment
**
**  Texture environment state access.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      Name
**          Specifies the symbolic name of the state to set/get.
**
**      Value
**          Points to the data.
**
**      Type
**          Data format.
**
**  OUTPUT:
**
**      Nothing.
*/

static GLboolean _SetTextureEnvironment(
    glsCONTEXT_PTR Context,
    GLenum Name,
    const GLfloat* Value
    )
{
    /* Get a shortcut to the active sampler. */
    glsTEXTURESAMPLER_PTR sampler = Context->texture.activeSampler;
    GLboolean result;
    gcmHEADER_ARG("Context=0x%x Name=0x%04x Value=0x%x",
                    Context, Name, Value);
    gcmASSERT(sampler != gcvNULL);

    /* Set parameter. */
    switch (Name)
    {
    case GL_TEXTURE_ENV_MODE:
        result = _SetTextureFunction(Context, sampler, Value);
        break;

    case GL_COMBINE_RGB:
        result = _SetCombineColorFunction(Context, sampler, Value);
        break;

    case GL_COMBINE_ALPHA:
        result = _SetCombineAlphaFunction(Context, sampler, Value);
        break;

    case GL_SRC0_RGB:
    case GL_SRC1_RGB:
    case GL_SRC2_RGB:
        result = _SetCombineColorSource(Context, Name, sampler, Value);
        break;

    case GL_SRC0_ALPHA:
    case GL_SRC1_ALPHA:
    case GL_SRC2_ALPHA:
        result = _SetCombineAlphaSource(Context, Name, sampler, Value);
        break;

    case GL_OPERAND0_RGB:
    case GL_OPERAND1_RGB:
    case GL_OPERAND2_RGB:
        result = _SetCombineColorOperand(Context, Name, sampler, Value);
        break;

    case GL_OPERAND0_ALPHA:
    case GL_OPERAND1_ALPHA:
    case GL_OPERAND2_ALPHA:
        result = _SetCombineAlphaOperand(Context, Name, sampler, Value);
        break;

    case GL_TEXTURE_ENV_COLOR:
        result = _SetCurrentColor(Context, sampler, Value);
        Context->fsUniformDirty.uTexColorDirty = gcvTRUE;
        break;

    case GL_RGB_SCALE:
        result = _SetColorScale(Context, sampler, Value);
        break;

    case GL_ALPHA_SCALE:
        result = _SetAlphaScale(Context, sampler, Value);
        break;
    default:
        result = GL_FALSE;
        break;
    }

    gcmFOOTER_ARG("result=%d", result);
    /* Invalid enum. */
    return result;
}

static GLboolean _GetTextureEnvironment(
    glsCONTEXT_PTR Context,
    GLenum Name,
    GLvoid* Value,
    gleTYPE Type
    )
{
    GLboolean result = GL_TRUE;

    /* Get a shortcut to the active sampler. */
    glsTEXTURESAMPLER_PTR sampler = Context->texture.activeSampler;
    gcmHEADER_ARG("Context=0x%x Name=0x%04x Value=0x%x Type=0x%04x",
                    Context, Name, Value, Type);
    gcmASSERT(sampler != gcvNULL);

    switch (Name)
    {
    case GL_TEXTURE_ENV_MODE:
        glfGetFromEnum(
            _TextureFunctionNames[sampler->function],
            Value,
            Type
            );
        break;

    case GL_COMBINE_RGB:
        glfGetFromEnum(
            _CombineColorTextureFunctionNames[sampler->combColor.function],
            Value,
            Type
            );
        break;

    case GL_COMBINE_ALPHA:
        glfGetFromEnum(
            _CombineAlphaTextureFunctionNames[sampler->combAlpha.function],
            Value,
            Type
            );
        break;

    case GL_SRC0_RGB:
        glfGetFromEnum(
            _CombineFunctionSourceNames[sampler->combColor.source[0]],
            Value,
            Type
            );
        break;

    case GL_SRC1_RGB:
        glfGetFromEnum(
            _CombineFunctionSourceNames[sampler->combColor.source[1]],
            Value,
            Type
            );
        break;

    case GL_SRC2_RGB:
        glfGetFromEnum(
            _CombineFunctionSourceNames[sampler->combColor.source[2]],
            Value,
            Type
            );
        break;

    case GL_SRC0_ALPHA:
        glfGetFromEnum(
            _CombineFunctionSourceNames[sampler->combAlpha.source[0]],
            Value,
            Type
            );
        break;

    case GL_SRC1_ALPHA:
        glfGetFromEnum(
            _CombineFunctionSourceNames[sampler->combAlpha.source[1]],
            Value,
            Type
            );
        break;

    case GL_SRC2_ALPHA:
        glfGetFromEnum(
            _CombineFunctionSourceNames[sampler->combAlpha.source[2]],
            Value,
            Type
            );
        break;

    case GL_OPERAND0_RGB:
        glfGetFromEnum(
            _CombineFunctionColorOperandNames[sampler->combColor.operand[0]],
            Value,
            Type
            );
        break;

    case GL_OPERAND1_RGB:
        glfGetFromEnum(
            _CombineFunctionColorOperandNames[sampler->combColor.operand[1]],
            Value,
            Type
            );
        break;

    case GL_OPERAND2_RGB:
        glfGetFromEnum(
            _CombineFunctionColorOperandNames[sampler->combColor.operand[2]],
            Value,
            Type
            );
        break;

    case GL_OPERAND0_ALPHA:
        glfGetFromEnum(
            _CombineFunctionAlphaOperandNames[sampler->combAlpha.operand[0]],
            Value,
            Type
            );
        break;

    case GL_OPERAND1_ALPHA:
        glfGetFromEnum(
            _CombineFunctionAlphaOperandNames[sampler->combAlpha.operand[1]],
            Value,
            Type
            );
        break;

    case GL_OPERAND2_ALPHA:
        glfGetFromEnum(
            _CombineFunctionAlphaOperandNames[sampler->combAlpha.operand[2]],
            Value,
            Type
            );
        break;

    case GL_TEXTURE_ENV_COLOR:
        glfGetFromVector4(
            &sampler->constColor,
            Value,
            Type
            );
        break;

    case GL_RGB_SCALE:
        glfGetFromFloat(
            sampler->combColor.scale,
            Value,
            Type
            );
        break;

    case GL_ALPHA_SCALE:
        glfGetFromFloat(
            sampler->combAlpha.scale,
            Value,
            Type
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
**  _SetTextureState/_GetTextureState
**
**  Texture state access main entry points.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      Target
**          Specifies a texture environment.
**
**      Name
**          Specifies the symbolic name of the state to set/get.
**
**      Value
**          Points to the data.
**
**      Type
**          Data format.
**
**  OUTPUT:
**
**      Nothing.
*/

static GLboolean _SetTextureState(
    glsCONTEXT_PTR Context,
    GLenum Target,
    GLenum Name,
    const GLfloat* Value
    )
{
    GLboolean result;
    gcmHEADER_ARG("Context=0x%x Target=0x%04x Name=0x%04x Value=0x%x",
                    Context, Target, Name, Value);
    switch (Target)
    {
    case GL_TEXTURE_ENV:
        result = _SetTextureEnvironment(
            Context,
            Name,
            Value
            );
        gcmFOOTER_ARG("result=%d", result);
        return result;

    case GL_POINT_SPRITE_OES:
        {
            GLuint value;
            if ((Name == GL_COORD_REPLACE_OES) &&
                glfConvertGLboolean(Value, glvFLOAT, &value))
            {
                /* Invalidate point sprite state. */
                Context->pointStates.spriteDirty = GL_TRUE;

                /* Set coordinate replacement mode for the unit. */
                Context->texture.activeSampler->coordReplace = value != 0;
                gcmFOOTER_ARG("result=%d", GL_TRUE);
                return GL_TRUE;
            }
        }
        break;
    case GL_TEXTURE_FILTER_CONTROL_EXT:
        if (Name == GL_TEXTURE_LOD_BIAS_EXT)
        {
            Context->texture.activeSampler->lodBias = * (GLfloat *) Value;
            gcmFOOTER_ARG("result=%d", GL_TRUE);
            return GL_TRUE;
        }
        break;
    }

    gcmFOOTER_ARG("result=%d", GL_FALSE);
    return GL_FALSE;
}

static GLboolean _GetTextureState(
    glsCONTEXT_PTR Context,
    GLenum Target,
    GLenum Name,
    GLvoid* Value,
    gleTYPE Type
    )
{
    GLboolean result;
    gcmHEADER_ARG("Context=0x%x Target=0x%04x Name=0x%04x Value=0x%x Type=0x%04x",
                    Context, Target, Name, Value, Type);
    switch (Target)
    {
    case GL_TEXTURE_ENV:
        result = _GetTextureEnvironment(
            Context,
            Name,
            Value,
            Type
            );
        gcmFOOTER_ARG("result=%d", result);
        return result;

    case GL_POINT_SPRITE_OES:
        if (Name == GL_COORD_REPLACE_OES)
        {
            glfGetFromInt(
                Context->texture.activeSampler->coordReplace,
                Value,
                Type
                );

            gcmFOOTER_ARG("result=%d", GL_TRUE);
            return GL_TRUE;
        }
        break;
    case GL_TEXTURE_FILTER_CONTROL_EXT:
        if (Name == GL_TEXTURE_LOD_BIAS_EXT)
        {
            glfGetFloatFromFloatArray(
                &(Context->texture.activeSampler->lodBias),
                1,
                Value
                );
            gcmFOOTER_ARG("result=%d", GL_TRUE);
            return GL_TRUE;
        }
        break;
    }

    gcmFOOTER_ARG("result=%d", GL_FALSE);
    return GL_FALSE;
}


/*******************************************************************************
**
**  _SetTexGen/_GetTexGen (GL_OES_texture_cube_map)
**
**  Texture coordinate generation mode.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      Coord
**          Specifies the type of coordinate.
**
**      Name
**          Specifies the symbolic name of the state to set/get.
**
**      Value
**          Points to the data.
**
**      Type
**          Data format.
**
**  OUTPUT:
**
**      Nothing.
*/

static GLboolean _SetTexGen(
    glsCONTEXT_PTR Context,
    GLenum Coord,
    GLenum Name,
    const GLfloat* Value
    )
{
    GLboolean result;
    gcmHEADER_ARG("Context=0x%x Coord=0x%04x Name=0x%04x Value=0x%x",
                    Context, Coord, Name, Value);

    do
    {
        glsTEXTURESAMPLER_PTR sampler;

        /* Verify the target coordinate. */
        if (Coord != GL_TEXTURE_GEN_STR_OES)
        {
            result = GL_FALSE;
            break;
        }

        /* Verify the coordinate mode. */
        if (Name != GL_TEXTURE_GEN_MODE_OES)
        {
            result = GL_FALSE;
            break;
        }

        /* Get a shortcut to the active sampler. */
        sampler = Context->texture.activeSampler;
        gcmASSERT(sampler != gcvNULL);

        /* Set texture coordinate generation mode. */
        result = _SetTexCoordGenMode(
            Context, sampler, Value
            );
    }
    while (gcvFALSE);

    gcmFOOTER_ARG("result=%d", result);
    /* Return result. */
    return result;
}

static GLboolean _GetTexGen(
    glsCONTEXT_PTR Context,
    GLenum Coord,
    GLenum Name,
    GLvoid* Value,
    gleTYPE Type
    )
{
    GLboolean result = GL_TRUE;
    gcmHEADER_ARG("Context=0x%x Coord=0x%04x Name=0x%04x Value=0x%x Type=0x%04x",
                    Context, Coord, Name, Value, Type);

    do
    {
        glsTEXTURESAMPLER_PTR sampler;

        /* Verify the target coordinate. */
        if (Coord != GL_TEXTURE_GEN_STR_OES)
        {
            result = GL_FALSE;
            break;
        }

        /* Verify the coordinate mode. */
        if (Name != GL_TEXTURE_GEN_MODE_OES)
        {
            result = GL_FALSE;
            break;
        }

        /* Get a shortcut to the active sampler. */
        sampler = Context->texture.activeSampler;
        gcmASSERT(sampler != gcvNULL);

        /* Convert the value. */
        glfGetFromEnum(
            _TextureGenModes[sampler->genMode],
            Value,
            Type
            );
    }
    while (gcvFALSE);

    gcmFOOTER_ARG("result=%d", result);
    /* Return result. */
    return result;
}


/*******************************************************************************
**
**  glfEnableTexturing
**
**  Enable texturing for the active sampler.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      Enable
**          Enable flag.
**
**  OUTPUT:
**
**      Nothing.
*/
GLenum glfEnableTexturing(
    glsCONTEXT_PTR Context,
    GLboolean Enable
    )
{
    /* Make a shortcut to the sampler. */
    glsTEXTURESAMPLER_PTR sampler = Context->texture.activeSampler;
    gcmHEADER_ARG("Context=0x%x Enable=%d", Context, Enable);

    /* Set texturing enable state. */
    sampler->enableTexturing = Enable;
    if (Enable)
    {
        sampler->binding = sampler->bindings[glvTEXTURE2D];
    }

    gcmFOOTER_ARG("result=0x%04x", GL_NO_ERROR);
    /* Success. */
    return GL_NO_ERROR;
}


/*******************************************************************************
**
**  glfEnableCubeTexturing
**
**  Enable cubemap texturing for the active sampler.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      Enable
**          Enable flag.
**
**  OUTPUT:
**
**      Nothing.
*/

GLenum glfEnableCubeTexturing(
    glsCONTEXT_PTR Context,
    GLboolean Enable
    )
{
    /* Make a shortcut to the sampler. */
    glsTEXTURESAMPLER_PTR sampler = Context->texture.activeSampler;

    gcmHEADER_ARG("Context=0x%x Enable=%d", Context, Enable);

    /* Set texturing enable state. */
    sampler->enableCubeTexturing = Enable;

    /* Update texture binding. */
    gcmASSERT((Enable == 0) || (Enable == 1));
    if (Enable)
    {
        sampler->binding = sampler->bindings[glvCUBEMAP];
    }

    gcmFOOTER_ARG("result=0x%04x", GL_NO_ERROR);
    /* Success. */
    return GL_NO_ERROR;
}

/*******************************************************************************
**
**  glfEnableExternalTexturing
**
**  Enable Image external texturing for the active sampler.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      Enable
**          Enable flag.
**
**  OUTPUT:
**
**      Nothing.
*/

GLenum glfEnableExternalTexturing(
    glsCONTEXT_PTR Context,
    GLboolean Enable
    )
{
    /* Make a shortcut to the sampler. */
    glsTEXTURESAMPLER_PTR sampler = Context->texture.activeSampler;

    /* Set texturing enable state. */
    sampler->enableExternalTexturing = Enable;
    if (Enable)
    {
        sampler->binding = sampler->bindings[glvTEXTUREEXTERNAL];
    }

    /* Success. */
    return GL_NO_ERROR;
}


/*******************************************************************************
**
**  glfEnableCoordGen
**
**  Enable texture coordinate generation for the active sampler.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      Enable
**          Enable flag.
**
**  OUTPUT:
**
**      Nothing.
*/

GLenum glfEnableCoordGen(
    glsCONTEXT_PTR Context,
    GLboolean Enable
    )
{
    /* Make a shortcut to the sampler. */
    glsTEXTURESAMPLER_PTR sampler = Context->texture.activeSampler;

    gcmHEADER_ARG("Context=0x%x Enable=%d", Context, Enable);

    /* Update the state. */
    sampler->genEnable = Enable;

    /* Update the hash key. */
    glmSETHASH_1BIT(hashTexCubeCoordGenEnable, Enable, sampler->index);

    gcmFOOTER_ARG("result=0x%04x", GL_NO_ERROR);
    /* Success. */
    return GL_NO_ERROR;
}


/*******************************************************************************
**
**  glfUpdateTextureStates
**
**  Update texture states.
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

gceSTATUS glfUpdateTextureStates(
    glsCONTEXT_PTR Context
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    GLboolean coordReplace = GL_FALSE;
    GLint hashComponents;
    GLint i;

    gcmHEADER_ARG("Context=0x%x", Context);

    for (i = 0; i < Context->texture.pixelSamplers; i++)
    {
        glsTEXTURESAMPLER_PTR sampler = &Context->texture.sampler[i];

        if ((sampler->enableTexturing
          || sampler->enableExternalTexturing
          || sampler->enableCubeTexturing)
        &&  (sampler->binding->image.source  != gcvNULL
          || sampler->binding->direct.source != gcvNULL)
        )
        {
            /* Dynamically allocate mipmap level 0 for EGLImage case. */
            /* Get shortcuts. */
            glsTEXTUREWRAPPER_PTR texture = sampler->binding;
            gcoSURF mipmap = gcvNULL;

            gceSURF_FORMAT textureFormat;
            gctBOOL directSample;
            gcoSURF source;
            gctBOOL dirty;

            if (texture->image.source  != gcvNULL)
            {
                /* GL_OES_egl_image[_external] */
                textureFormat = texture->image.textureFormat;
                directSample  = texture->image.directSample;
                source        = texture->image.source;

                /* Get dirty state. */
                dirty         = texture->image.dirty;

                texture->image.dirty = gcvFALSE;
            }
            else
            {
                /* GL_VIV_direct_texture */
                textureFormat = texture->direct.textureFormat;
                directSample  = texture->direct.directSample;
                source        = texture->direct.source;

                /* Get dirty state. */
                dirty         = texture->direct.dirty;

                texture->direct.dirty = gcvFALSE;
            }

            if (texture->object == gcvNULL)
            {
                /* Dynamically allocate texture object. */
                status = gcoTEXTURE_ConstructEx(Context->hal, _HALtexType[texture->targetType], &texture->object);

                /* Verify creation. */
                if (gcmIS_ERROR(status))
                {
                    glmERROR(GL_OUT_OF_MEMORY);
                    break;
                }
            }

            if (texture->image.image != gcvNULL)
            {
                khrEGL_IMAGE_PTR image = (khrEGL_IMAGE_PTR) texture->image.image;

                if (image->update)
                {
                    /* Update source sibling pixels. */
                    if (image->update(image))
                    {
                        /* Force dirty if source updated. */
                        dirty = gcvTRUE;
                    }
                }
                else
                {
                    /* Assume dirty for other EGL image types. */
                    dirty = gcvTRUE;
                }
            }

            if (!directSample)
            {
                /* Try allocate mipmap if not allocated. */
                status = gcoTEXTURE_GetMipMap(
                    texture->object,
                    0,
                    &mipmap
                    );

                if (gcmIS_ERROR(status))
                {
                    /* Allocate mipmap for later uploading. */
                    gctUINT width;
                    gctUINT height;

                    /* Clear status. */
                    status = gcvSTATUS_OK;

                    /* Get source size. */
                    if (gcmIS_ERROR(gcoSURF_GetSize(source,
                                                    &width,
                                                    &height,
                                                    gcvNULL)))
                    {
                        glmERROR(GL_INVALID_VALUE);
                        break;
                    }

                    /* Create mipmap level 0. */
                    status = gcoTEXTURE_AddMipMap(
                        texture->object,
                        0,
                        gcvUNKNOWN_MIPMAP_IMAGE_FORMAT,
                        textureFormat,
                        width,
                        height,
                        0,
                        gcvFACE_NONE,
                        gcvPOOL_DEFAULT,
                        &mipmap
                        );

                    if (gcmIS_ERROR(status))
                    {
                        gcmFATAL("%s: add mipmap fail", __FUNCTION__);
                        glmERROR(GL_INVALID_VALUE);
                        break;
                    }

                    gcmVERIFY_OK(gcoSURF_SetSharedLock(mipmap,
                        Context->texture.textureList->sharedLock));

                    /* Force dirty flag. */
                    dirty = gcvTRUE;
                }
            }

            if (dirty)
            {
                if (directSample)
                {
                    /* Directly add surface to mipmap. */
                    status = gcoTEXTURE_AddMipMapFromClient(
                        texture->object,
                        0,
                        source
                        );

                    gcmVERIFY_OK(gcoSURF_SetSharedLock(source,
                        Context->texture.textureList->sharedLock));

                    if (gcmIS_ERROR(status))
                    {
                        gcmFATAL("%s: add mipmap fail", __FUNCTION__);
                        glmERROR(GL_INVALID_VALUE);
                        break;
                    }
                }
                else
                {
                    gceSURF_FORMAT srcFormat;

                    /* Get source format. */
                    if (gcmIS_ERROR(gcoSURF_GetFormat(source,
                                                      gcvNULL,
                                                      &srcFormat)))
                    {
                        glmERROR(GL_INVALID_VALUE);
                        break;
                    }

                    /*
                     * Android has following formats, but hw does not support
                     * Need software upload for such formats.
                     */
                    if ((srcFormat == gcvSURF_NV16)
                    ||  (srcFormat == gcvSURF_NV61)
                    ||  (srcFormat == gcvSURF_R4G4B4A4)
                    ||  (srcFormat == gcvSURF_R5G5B5A1)
                    )
                    {
                        gctUINT width;
                        gctUINT height;
                        gctPOINTER memory[3] = {gcvNULL};
                        gctINT stride[3];

                        gcmVERIFY_OK(gcoSURF_GetSize(
                            source,
                            &width,
                            &height,
                            gcvNULL
                            ));

                        gcmVERIFY_OK(gcoSURF_GetAlignedSize(
                            source,
                            gcvNULL,
                            gcvNULL,
                            stride
                            ));

                        /* Lock source surface for read. */
                        gcmERR_BREAK(gcoSURF_Lock(
                            source,
                            gcvNULL,
                            memory
                            ));

                        if ((srcFormat == gcvSURF_NV16)
                        ||  (srcFormat == gcvSURF_NV61)
                        )
                        {
                            /* UV stride should be same as Y stride. */
                            stride[1] = stride[0];

                            /* Upload NV16/NV61 to YUY2 by software. */
                            status = gcoTEXTURE_UploadYUV(
                                texture->object,
                                gcvFACE_NONE,
                                width,
                                height,
                                0,
                                memory,
                                stride,
                                srcFormat
                                );
                        }
                        else
                        {
                            /* Upload by software. */
                            status = gcoTEXTURE_Upload(
                                texture->object,
                                0,
                                gcvFACE_NONE,
                                width,
                                height,
                                0,
                                memory[0],
                                stride[0],
                                srcFormat,
                                gcvSURF_COLOR_SPACE_LINEAR
                                );
                        }

                        /* Unlock. */
                        gcmVERIFY_OK(gcoSURF_Unlock(
                            source,
                            memory[0]
                            ));

                        if (gcmIS_ERROR(status))
                        {
                            gcmFATAL("%s: upload texture fail", __FUNCTION__);
                            glmERROR(GL_INVALID_VALUE);
                            break;
                        }
                    }
                    else
                    {
                        gcsSURF_VIEW srcView = {source, 0, 1};
                        gcsSURF_VIEW mipView = {mipmap, 0, 1};

#if defined(ANDROID) && gcdANDROID_IMPLICIT_NATIVE_BUFFER_SYNC
                        android_native_buffer_t * nativeBuffer;
                        struct private_handle_t * hnd = gcvNULL;
                        struct gc_native_handle_t * handle = gcvNULL;

                        /* Cast to android native buffer. */
                        nativeBuffer = (android_native_buffer_t *) texture->image.nativeBuffer;

                        if (nativeBuffer != gcvNULL)
                        {
                            /* Get private handle. */
                            hnd = (struct private_handle_t *) nativeBuffer->handle;
                            handle = gc_native_handle_get(nativeBuffer->handle);
                        }

                        /* Check composition signal. */
                        if (handle != gcvNULL && handle->hwDoneSignal != 0)
                        {
                            gcmVERIFY_OK(gcoOS_Signal(
                                gcvNULL,
                                (gctSIGNAL) (gctUINTPTR_T) handle->hwDoneSignal,
                                gcvFALSE
                                ));
                        }
#endif

                        /* Use resolve to upload texture. */
                        status = gcoSURF_ResolveRect(&srcView, &mipView,  gcvNULL);

                        if (gcmIS_ERROR(status))
                        {
                            gcmFATAL("%s: Failed to upload texture.", __FUNCTION__);
                            glmERROR(GL_INVALID_VALUE);
                            break;
                        }

#if defined(ANDROID) && gcdANDROID_IMPLICIT_NATIVE_BUFFER_SYNC
                        if (handle != gcvNULL && handle->hwDoneSignal != 0)
                        {
                            /* Signal the signal, so CPU apps
                             * can lock again once resolve is done. */
                            gcsHAL_INTERFACE iface;

                            iface.command            = gcvHAL_SIGNAL;
                            iface.u.Signal.signal    = handle->hwDoneSignal;
                            iface.u.Signal.auxSignal = 0;
                            /* Stuff the client's PID. */
                            iface.u.Signal.process   = handle->clientPID;
                            iface.u.Signal.fromWhere = gcvKERNEL_PIXEL;

                            /* Schedule the event. */
                            gcmVERIFY_OK(gcoHAL_ScheduleEvent(gcvNULL, &iface));
                        }
#endif

                        /* Wait all the pixels done. */
                        gcmVERIFY_OK(gco3D_Semaphore(
                            Context->hw,
                            gcvWHERE_RASTER,
                            gcvWHERE_PIXEL,
                            gcvHOW_SEMAPHORE_STALL
                            ));
                    }

                }

                /* Set dirty flag (for later flush). */
                texture->dirty = gcvTRUE;
            }
        }

        /* Update stage enable sgtate. */
        _UpdateStageEnable(Context, sampler);

        /* Optimization. */
        if (!sampler->stageEnabled)
        {
            glmCLEARHASH_2BITS(
                hashTexCoordComponentCount,
                i
                );
            continue;
        }

        /* Our hardware currently has only one global coordinate replacement
           state (not per unit as specified in OpenGL ES spec). Up until now
           it has not been a problem. Here we determine whether the state
           has been turned on for any of the enabled samplers and OR them
           into one state for later analysis. */
        if (Context->pointStates.spriteDirty && sampler->stageEnabled)
        {
            coordReplace |= sampler->coordReplace;
        }

        /* Determine the number of components in streamed texture coordinate. */
        if (Context->drawTexOESEnabled)
        {
            /* Always 2 components for DrawTex extension. */
            sampler->coordType    = gcSHADER_FLOAT_X2;
            sampler->coordSwizzle = gcSL_SWIZZLE_XYYY;

            /* Set hash key component count. */
            hashComponents = 2;
        }
        else if (sampler->aTexCoordInfo.streamEnabled)
        {
            /* Copy stream component count. */
            sampler->coordType    = sampler->aTexCoordInfo.varyingType;
            sampler->coordSwizzle = sampler->aTexCoordInfo.varyingSwizzle;

            /* Set hash key component count. */
            hashComponents = sampler->aTexCoordInfo.components;
        }
        else
        {
            /* Constant texture coordinate allways has 4 components. */
            sampler->coordType    = gcSHADER_FLOAT_X4;
            sampler->coordSwizzle = gcSL_SWIZZLE_XYZW;

            /* Set hash key component count. */
            hashComponents = 4;
        }

        /* Update the hash key; only values in 2..4 range are possible,
           use the count reduced by 2 to save (1 * 4) = 4 hash bits. */
        glmSETHASH_2BITS(
            hashTexCoordComponentCount,
            (hashComponents - 2),
            i
            );
    }

    /* Update point sprite state. */
    if (Context->pointStates.spriteDirty)
    {
        Context->pointStates.spriteActive
            =  coordReplace
            && Context->pointStates.pointPrimitive
            && Context->pointStates.spriteEnable;

        if (Context->hwPointSprite != Context->pointStates.spriteActive)
        {
            status = gco3D_SetPointSprite(
                Context->hw,
                Context->hwPointSprite = Context->pointStates.spriteActive
                );
        }

        Context->pointStates.spriteDirty = GL_FALSE;
    }

    gcmFOOTER();
    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  glfQueryTextureState
**
**  Queries texture state values.
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
*/

GLboolean glfQueryTextureState(
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
    case GL_ACTIVE_TEXTURE:
        glfGetFromEnum(
            GL_TEXTURE0 + Context->texture.activeSamplerIndex,
            Value,
            Type
            );
        break;

    case GL_CLIENT_ACTIVE_TEXTURE:
        glfGetFromEnum(
            GL_TEXTURE0 + Context->texture.activeClientSamplerIndex,
            Value,
            Type
            );
        break;

    case GL_TEXTURE_2D:
        glfGetFromInt(
            Context->texture.activeSampler->enableTexturing,
            Value,
            Type
            );
        break;

    case GL_TEXTURE_CUBE_MAP_OES:
        glfGetFromInt(
            Context->texture.activeSampler->enableCubeTexturing,
            Value,
            Type
            );
        break;

    case GL_TEXTURE_EXTERNAL_OES:
        glfGetFromInt(
            Context->texture.activeSampler->enableExternalTexturing,
            Value,
            Type
            );
        break;

    case GL_TEXTURE_GEN_STR_OES:
        glfGetFromInt(
            Context->texture.activeSampler->genEnable,
            Value,
            Type
            );
        break;

    case GL_TEXTURE_BINDING_2D:
        glfGetFromInt(
            Context->texture.activeSampler->bindings[glvTEXTURE2D]->name,
            Value,
            Type
            );
        break;

    case GL_TEXTURE_BINDING_CUBE_MAP_OES:
        glfGetFromInt(
            Context->texture.activeSampler->bindings[glvCUBEMAP]->name,
            Value,
            Type
            );
        break;

    case GL_TEXTURE_BINDING_EXTERNAL_OES:
        glfGetFromInt(
            Context->texture.activeSampler->bindings[glvTEXTUREEXTERNAL]->name,
            Value,
            Type
            );
        break;

    case GL_NUM_COMPRESSED_TEXTURE_FORMATS:
        glfGetFromInt(
            gcmCOUNTOF(_compressedTextures),
            Value,
            Type
            );
        break;

    case GL_COMPRESSED_TEXTURE_FORMATS:
        glfGetFromEnumArray(
            _compressedTextures,
            gcmCOUNTOF(_compressedTextures),
            Value,
            Type
            );
        break;

    case GL_MAX_TEXTURE_SIZE:
    case GL_MAX_CUBE_MAP_TEXTURE_SIZE_OES:
        {
            glfGetFromInt(
                gcmMAX(Context->maxTextureWidth,
                       Context->maxTextureHeight),
                Value,
                Type
                );
        }
        break;

    case GL_MAX_TEXTURE_UNITS:
        glfGetFromInt(
            glvMAX_TEXTURES,
            Value,
            Type
            );
        break;

    case GL_ALPHA_SCALE:
        glfGetFromFloat(
            Context->texture.activeSampler->combAlpha.scale,
            Value,
            Type
            );
        break;

    case GL_RGB_SCALE:
        glfGetFromFloat(
            Context->texture.activeSampler->combColor.scale,
            Value,
            Type
            );
        break;

    case GL_GENERATE_MIPMAP_HINT:
        glfGetFromEnum(
            Context->texture.generateMipmapHint,
            Value,
            Type
            );
        break;

    case GL_MAX_TEXTURE_LOD_BIAS_EXT:
        glfGetFloatFromFloatArray(
            &(Context->texture.activeSampler->lodBias),
            1,
            Value
            );
        break;

    case GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT:
        glfGetFromInt(
            (GLint)Context->maxAniso,
            Value,
            Type
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
**  _validateFormat
**
**  Verifies whether the format specified is one of the valid constants.
**
**  INPUT:
**
**      Format
**          Specifies the number of color components in the texture. Must be
**          GL_ALPHA, GL_RGB, GL_RGBA, GL_LUMINANCE, or GL_LUMINANCE_ALPHA.
**
**  OUTPUT:
**
**      Returns GL_FALSE for unsupported/invalid formats.
*/

static GLboolean _validateFormat(
    GLenum Format
    )
{
    gcmHEADER_ARG("Format=0x%04x", Format);
    switch (Format)
    {
    case GL_ALPHA:
    case GL_RGB:
    case GL_RGBA:
    case GL_LUMINANCE:
    case GL_LUMINANCE_ALPHA:
    case GL_BGRA_EXT:
    case GL_DEPTH_STENCIL_OES:
        gcmFOOTER_ARG("result=%d", GL_TRUE);
        return GL_TRUE;
        break;
    default:
        gcmFOOTER_ARG("result=%d", GL_FALSE);
        return GL_FALSE;
    }
}


/*******************************************************************************
**
**  _validateType
**
**  Verifies whether the type specified is one of the valid constants.
**
**  INPUT:
**
**      Type
**          Specifies the data type of the pixel data. The following symbolic
**          values are accepted: GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT_5_6_5,
**          GL_UNSIGNED_SHORT_4_4_4_4, and GL_UNSIGNED_SHORT_5_5_5_1.
**
**  OUTPUT:
**
**      Returns GL_FALSE for unsupported/invalid types.
*/

static GLboolean _validateType(
    GLenum Type
    )
{
    gcmHEADER_ARG("Type=0x%04x", Type);
    switch (Type)
    {
    case GL_UNSIGNED_BYTE:
    case GL_UNSIGNED_SHORT_5_6_5:
    case GL_UNSIGNED_SHORT_4_4_4_4:
    case GL_UNSIGNED_SHORT_5_5_5_1:
    case GL_UNSIGNED_INT_24_8_OES:

    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
    case GL_ETC1_RGB8_OES:
        gcmFOOTER_ARG("result=%d", GL_TRUE);
        return GL_TRUE;

    default:
        gcmFOOTER_ARG("result=%d", GL_FALSE);
        return GL_FALSE;
    }
}


/*******************************************************************************
**
**  _getFormat
**
**  Convert OpenGL texture format into HAL format.
**  The function fails if type is GL_UNSIGNED_SHORT_5_6_5 and format is not
**  GL_RGB. The function also fails if type is one of GL_UNSIGNED_SHORT_4_4_4_4,
**  or GL_UNSIGNED_SHORT_5_5_5_1 and formatis not GL_RGBA.
**
**  INPUT:
**
**      Format
**          Specifies the format of the pixel data. Must be same as
**          internalformat. The following symbolic values are accepted:
**          GL_ALPHA, GL_RGB, GL_RGBA, GL_LUMINANCE, and GL_LUMINANCE_ALPHA.
**
**      Type
**          Specifies the data type of the pixel data. The following symbolic
**          values are accepted: GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT_5_6_5,
**          GL_UNSIGNED_SHORT_4_4_4_4, and GL_UNSIGNED_SHORT_5_5_5_1.
**
**      HalFormat
**          HAL format.
**
**  OUTPUT:
**
**      Returns GL_FALSE for unsupported formats.
*/

static GLboolean _getFormat(
    GLenum Format,
    GLenum Type,
    gceSURF_FORMAT* HalFormat
    )
{
    GLboolean result;

    gcmHEADER_ARG("Format=0x%04x Type=0x%04x HalFormat=0x%x",
                    Format, Type, HalFormat);

    /* Assume there is no equivalent. */
    *HalFormat = gcvSURF_UNKNOWN;

    /* Dispatch on the type. */
    switch (Type)
    {
    case GL_UNSIGNED_BYTE:

        /* Dispatch on format. */
        switch (Format)
        {
        case GL_ALPHA:
            *HalFormat = gcvSURF_A8;
            break;

        case GL_RGB:
            *HalFormat = gcvSURF_B8G8R8;
            break;

        case GL_RGBA:
            *HalFormat = gcvSURF_A8B8G8R8;
            break;

        case GL_LUMINANCE:
            *HalFormat = gcvSURF_L8;
            break;

        case GL_LUMINANCE_ALPHA:
            *HalFormat = gcvSURF_A8L8;
            break;

        case GL_BGRA_EXT:
            *HalFormat = gcvSURF_A8R8G8B8;
            break;
        }

        break;

    case GL_UNSIGNED_SHORT_5_6_5:
        if (Format == GL_RGB)
        {
            *HalFormat = gcvSURF_R5G6B5;
        }
        break;

    case GL_UNSIGNED_SHORT_4_4_4_4:
        if (Format == GL_RGBA)
        {
            *HalFormat = gcvSURF_R4G4B4A4;
        }
        break;

    case GL_UNSIGNED_SHORT_5_5_5_1:
        if (Format == GL_RGBA)
        {
            *HalFormat = gcvSURF_R5G5B5A1;
        }
        break;

    case GL_PALETTE4_RGBA4_OES:
    case GL_PALETTE8_RGBA4_OES:
        *HalFormat = gcvSURF_A4R4G4B4;
        break;

    case GL_PALETTE4_RGB5_A1_OES:
    case GL_PALETTE8_RGB5_A1_OES:
        *HalFormat = gcvSURF_A1R5G5B5;
        break;

    case GL_PALETTE4_R5_G6_B5_OES:
    case GL_PALETTE8_R5_G6_B5_OES:
        *HalFormat = gcvSURF_R5G6B5;
        break;

    case GL_PALETTE4_RGB8_OES:
    case GL_PALETTE8_RGB8_OES:
        *HalFormat = gcvSURF_X8R8G8B8;
        break;

    case GL_PALETTE4_RGBA8_OES:
    case GL_PALETTE8_RGBA8_OES:
        *HalFormat = gcvSURF_A8R8G8B8;
        break;

    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        *HalFormat = gcvSURF_DXT1;
        break;

    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        *HalFormat = gcvSURF_DXT3;
        break;

    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        *HalFormat = gcvSURF_DXT5;
        break;

    case GL_ETC1_RGB8_OES:
        *HalFormat = gcvSURF_ETC1;
        break;

    case GL_UNSIGNED_INT_24_8_OES:
        if (Format == GL_DEPTH_STENCIL_OES)
        {
            *HalFormat = gcvSURF_D24S8;
        }
        break;
    }

    result = (*HalFormat == gcvSURF_UNKNOWN) ? GL_FALSE : GL_TRUE;
    gcmFOOTER_ARG("result=%d", result);
    /* Return result. */
    return result;
}

static gceENDIAN_HINT _getEndianHint(
    GLenum Format,
    GLenum Type
    )
{
    gcmHEADER_ARG("Format=0x%04x Type=0x%04x", Format, Type);
    /* Dispatch on the type. */
    switch (Type)
    {
    case GL_UNSIGNED_BYTE:
        gcmFOOTER_ARG("result=0x%04x", gcvENDIAN_NO_SWAP);
        return gcvENDIAN_NO_SWAP;

    case GL_UNSIGNED_SHORT_5_6_5:
    case GL_UNSIGNED_SHORT_4_4_4_4:
    case GL_UNSIGNED_SHORT_5_5_5_1:
        gcmFOOTER_ARG("result=0x%04x", gcvENDIAN_SWAP_WORD);
        return gcvENDIAN_SWAP_WORD;

    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
    case GL_ETC1_RGB8_OES:
        gcmFOOTER_ARG("result=0x%04x", gcvENDIAN_NO_SWAP);
        return gcvENDIAN_NO_SWAP;

    default:
        gcmASSERT(0);
        gcmFOOTER_ARG("result=0x%04x", gcvENDIAN_NO_SWAP);
        return gcvENDIAN_NO_SWAP;
    }
}

/*******************************************************************************
**
**  _QueryImageBaseFormat
**
**  Get the texture's format of an EGL image.
**
**  INPUT:
**
**      Image
**          Specifies an eglImage.
**
**  OUTPUT
**
**      Format
**          Return format of the texture.
**
*/

static gceSTATUS _QueryImageBaseFormat(
    khrEGL_IMAGE_PTR Image,
    GLenum * Format
   )
{
    gcmHEADER_ARG("Image=0x%x Format=0x%x", Image, Format);
    switch (Image->type)
    {
    case KHR_IMAGE_TEXTURE_2D:
    case KHR_IMAGE_TEXTURE_CUBE:
        *Format = (GLenum) Image->u.texture.format;
        break;

    case KHR_IMAGE_ANDROID_NATIVE_BUFFER:
        switch (Image->u.ANativeBuffer.format)
        {
        case gcvSURF_R5G6B5:
            *Format = GL_RGB;
            break;

        case gcvSURF_A8B8G8R8:
        case gcvSURF_X8B8G8R8:
        case gcvSURF_R4G4B4A4:
        case gcvSURF_R5G5B5A1:
            *Format = GL_RGBA;
            break;

        case gcvSURF_A8R8G8B8:
            *Format = GL_BGRA_EXT;
            break;

        default:
            *Format = GL_RGBA;
            gcmFOOTER_ARG("result=0x%04x", gcvFALSE);
            return gcvFALSE;
        }
        break;

    default:
        *Format = GL_RGBA;
        break;
    }

    gcmFOOTER_ARG("result=%s", "gcvSTATUS_OK");
    return gcvSTATUS_OK;
}



/*******************************************************************************
**
**  _GetSourceStride
**
**  Compute the source texture's stride, when doing TexImage2D and TexSubImage2D.
**  The function fails if type is GL_UNSIGNED_SHORT_5_6_5 and format is not
**  GL_RGB. The function also fails if type is one of GL_UNSIGNED_SHORT_4_4_4_4,
**  or GL_UNSIGNED_SHORT_5_5_5_1 and formatis not GL_RGBA.
**
**  INPUT:
**
**      Format
**          Specifies the format of the pixel data. Must be same as
**          internalformat. The following symbolic values are accepted:
**          GL_ALPHA, GL_RGB, GL_RGBA, GL_LUMINANCE, and GL_LUMINANCE_ALPHA.
**

**      Type
**          Specifies the data type of the pixel data. The following symbolic
**          values are accepted: GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT_5_6_5,
**          GL_UNSIGNED_SHORT_4_4_4_4, and GL_UNSIGNED_SHORT_5_5_5_1.
**
**
**      Width
**          Specifies the width of texture in user buffer, in pixels.
**
**      UnpackAlignment
**          Specifies the unpack aligment of texture in the user buffer, in GLbytes
**
**      Stride
**          The stride of texture in the user buffer, in GLbytes
**
**  OUTPUT:
**
**      Returns GL_FALSE for unsupported formats.
*/

static GLboolean _GetSourceStride(
    GLenum  Format,
    GLenum  Type,
    GLsizei Width,
    GLint   UnpackAlignment,
    gctINT* Stride
    )
{
    GLint pixelSize = -1;
    gcmHEADER_ARG("Format=0x%04x Type=0x%04x Width=%d UnpackAlignment=%d Stride=0x%x",
                    Format, Type, Width, UnpackAlignment, Stride);

    switch (Type)
    {
    case GL_UNSIGNED_BYTE:

        switch (Format)
        {
        case GL_ALPHA:
        case GL_LUMINANCE:
            pixelSize = 1;
            break;

        case GL_LUMINANCE_ALPHA:
            pixelSize = 2;
            break;

        case GL_RGB:
            pixelSize = 3;
            break;

        case GL_RGBA:
        case GL_BGRA_EXT:
            pixelSize = 4;
            break;

        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        case GL_ETC1_RGB8_OES:
            pixelSize = 0;
            break;
        }

        break;

    case GL_UNSIGNED_SHORT_5_6_5:
    case GL_UNSIGNED_SHORT_4_4_4_4:
    case GL_UNSIGNED_SHORT_5_5_5_1:
        pixelSize = 2;
        break;
    case GL_UNSIGNED_INT_24_8_OES:
        pixelSize = 4;
        break;
    }

    if (pixelSize != -1)
    {
        *Stride
            = (Width * pixelSize + UnpackAlignment - 1)
            & (~(UnpackAlignment-1));

        gcmFOOTER_ARG("result=%d", GL_TRUE);
        return GL_TRUE;
    }

    gcmFOOTER_ARG("result=%d", GL_FALSE);
    return GL_FALSE;
}

/*******************************************************************************
**
**  _DrawTexOES
**
**  Main entry function for GL_OES_draw_texture extension support.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      Xs, Ys, Zs
**          Position of the affected screen rectangle.
**
**      Ws, Hs
**          The width and height of the affected screen rectangle in pixels.
**
**  OUTPUT:
**
**      Nothing.
*/

static GLboolean _DrawTexOES(
    glsCONTEXT_PTR Context,
    GLfloat Xs,
    GLfloat Ys,
    GLfloat Zs,
    GLfloat Ws,
    GLfloat Hs
    )
{
    /* Define result. */
    GLboolean result = GL_TRUE;

    do
    {
        GLint i;

        /* Viewport bounding box size. */
        GLfloat widthViewport, heightViewport;

        /* Normalized coordinates. */
        GLfloat normLeft, normTop, normWidth, normHeight;

        /* Rectangle coordinates. */
        GLfloat left, top, right, bottom, width, height;

        /* Define depth. */
        GLfloat depth;

        /* 4 vertices for two triangles with three coordinates each. */
        GLfloat vertexBuffer[4 * 3];

        /* Validate the render area size. */
        if ((Ws <= glvFLOATZERO) || (Hs <= glvFLOATZERO))
        {
            result = GL_FALSE;
            break;
        }

        /* Determine the viewport bounding box size. */
        widthViewport  = glmINT2FLOAT(Context->viewportStates.viewportBox[2]);
        heightViewport = glmINT2FLOAT(Context->viewportStates.viewportBox[3]);

        /* Normalize coordinates. */
        normLeft   = Xs / widthViewport;
        normTop    = Ys / heightViewport;
        normWidth  = Ws / widthViewport;
        normHeight = Hs / heightViewport;

        /* Transform coordinates. */
        left   = (normLeft   * glvFLOATTWO) - glvFLOATONE;
        top    = (normTop    * glvFLOATTWO) - glvFLOATONE;
        right  = (normWidth  * glvFLOATTWO) + left;
        bottom = (normHeight * glvFLOATTWO) + top;

        /* Compute the depth value. */
        if (Zs <= glvFLOATZERO)
        {
            /* Get the near value. */
            depth = Context->depthStates.depthRange[0];
        }

        else if (Zs >= glvFLOATONE)
        {
            /* Get the far value. */
            depth = Context->depthStates.depthRange[1];
        }

        else
        {
            GLfloat zNear = Context->depthStates.depthRange[0];

            GLfloat zFar  = Context->depthStates.depthRange[1];

            depth = zNear + (Zs * (zFar - zNear));
        }

        /* Convert to Vivante space. */
        if (Context->chipModel < gcv1000 && Context->chipModel != gcv880)
        {
            depth = (depth + glvFLOATONE) * glvFLOATHALF;
        }

        /* Create two triangles. */
        vertexBuffer[ 0] = left;
        vertexBuffer[ 1] = top;
        vertexBuffer[ 2] = depth;

        vertexBuffer[ 3] = right;
        vertexBuffer[ 4] = top;
        vertexBuffer[ 5] = depth;

        vertexBuffer[ 6] = right;
        vertexBuffer[ 7] = bottom;
        vertexBuffer[ 8] = depth;

        vertexBuffer[ 9] = left;
        vertexBuffer[10] = bottom;
        vertexBuffer[11] = depth;

        /* Update the extension flags. */
        Context->hashKey.hashDrawTextureOES = 1;
        Context->drawTexOESEnabled = GL_TRUE;

        /* Set stream parameters. */
        glfSetStreamParameters(
            Context,
            &Context->aPositionDrawTexInfo,
            GL_FLOAT,
            3,
            3 * sizeof(GLfloat),
            gcvFALSE,
            &vertexBuffer,
            gcvNULL, glvTOTALBINDINGS       /* No buffer support here. */
            );

        /* Create texture coordinates as necessary. */
        for (i = 0; i < Context->texture.pixelSamplers; i++)
        {
            glsTEXTURESAMPLER_PTR sampler = &Context->texture.sampler[i];

            /* Update stage enable state. */
            _UpdateStageEnable(Context, sampler);

            if (sampler->stageEnabled)
            {
                /* Make a shortcut to the bound texture. */
                glsTEXTUREWRAPPER_PTR texture = sampler->binding;

                if (texture->dirtyCropRect)
                {
                    gcoSURF surface = gcvNULL;
                    gceORIENTATION orientation = gcvORIENTATION_TOP_BOTTOM;

                    /* Convert texture size to fractional values. */
                    GLfloat textureWidth  = glmINT2FLOAT(texture->width);
                    GLfloat textureHeight = glmINT2FLOAT(texture->height);

                    /* Get texture orientation. */
                    gcmVERIFY_OK(gcoTEXTURE_GetMipMap(texture->object, 0, &surface));
                    gcmVERIFY_OK(gcoSURF_QueryOrientation(surface, &orientation));

                    /* Convert crop rectangle to fractional values. */
                    left   = glmINT2FLOAT(texture->cropRect[0]);
                    top    = glmINT2FLOAT(texture->cropRect[1]);
                    width  = glmINT2FLOAT(texture->cropRect[2]);
                    height = glmINT2FLOAT(texture->cropRect[3]);

                    /* Flip bottom-top texture. */
                    if (orientation == gcvORIENTATION_BOTTOM_TOP)
                    {
                        gcmTRACE(gcvLEVEL_VERBOSE, "texture %d is bottom-up", i);

                        top    = textureHeight - top;
                        height = -height;
                    }

                    /* Normalize crop rectangle. */
                    normLeft   = left   / textureWidth;
                    normTop    = top    / textureHeight;
                    normWidth  = width  / textureWidth;
                    normHeight = height / textureHeight;

                    /* Construct two triangles. */
                    texture->texCoordBuffer[0] = normLeft;
                    texture->texCoordBuffer[1] = normTop;

                    texture->texCoordBuffer[2] = normLeft + normWidth;
                    texture->texCoordBuffer[3] = normTop;

                    texture->texCoordBuffer[4] = normLeft + normWidth;
                    texture->texCoordBuffer[5] = normTop  + normHeight;

                    texture->texCoordBuffer[6] = normLeft;
                    texture->texCoordBuffer[7] = normTop  + normHeight;

                    texture->dirtyCropRect = GL_FALSE;
                }

                /* Set stream parameters. */
                glfSetStreamParameters(
                    Context,
                    &sampler->aTexCoordDrawTexInfo,
                    GL_FLOAT,
                    2,
                    2 * sizeof(GLfloat),
                    gcvFALSE,
                    &texture->texCoordBuffer,
                    gcvNULL,                       /* No buffer support here. */
                    glvTEX0COORDBUFFER_AUX + i
                    );
            }
        }

        /* Draw the texture. */
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        /* Restore the hash key. */
        Context->hashKey.hashDrawTextureOES = 0;
        Context->drawTexOESEnabled = GL_FALSE;
    }
    while (gcvFALSE);

    /* Return result. */
    return result;
}

static GLboolean _DetachTexture(
    glsCONTEXT_PTR Context,
    glsTEXTUREWRAPPER_PTR Texture
    )
{
    /* Define result. */
    GLboolean result = GL_TRUE;

    /* detach shadow surface of tex.*/
    if(Context->frameBuffer != gcvNULL)
    {
        if(Context->frameBuffer->color.object == Texture)
        {
            /* detach shadow of tex.*/
            if(Context->frameBuffer->color.target != gcvNULL)
            {
                if(gcmIS_ERROR(gco3D_UnsetTarget(Context->hw, 0, Context->frameBuffer->color.target)))
                {
                    return GL_FALSE;
                }
            }
            else
            {
                /* detach surface of tex.*/
                if(Context->frameBuffer->color.surface != gcvNULL)
                {
                    if(gcmIS_ERROR(gco3D_UnsetTarget(Context->hw, 0, Context->frameBuffer->color.surface)))
                    {
                        return GL_FALSE;
                    }
                }
            }
        }

        if(Context->frameBuffer->depth.object == Texture)
        {
            if(Context->frameBuffer->depth.target != gcvNULL)
            {
                if(gcmIS_ERROR(gco3D_UnsetDepth(Context->hw, Context->frameBuffer->depth.target)))
                {
                    return GL_FALSE;
                }
            }
            else
            {
                if(Context->frameBuffer->depth.surface != gcvNULL)
                {
                    if(gcmIS_ERROR(gco3D_UnsetDepth(Context->hw, Context->frameBuffer->depth.surface)))
                    {
                        return GL_FALSE;
                    }
                }
            }
        }
    }

    /* Return result. */
    return result;
}

/******************************************************************************\
**************************** Texture Management Code ***************************
\******************************************************************************/

/*******************************************************************************
**
**  _ConstructWrapper
**
**  Constructs a new texture wrapper object.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      InsertAfter
**          Pointer to the wrapper that will preceed the new one.
**
**      Name
**          Name of the new texture wrapper object.
**
**  OUTPUT:
**
**      Pointer to the new texture wrapper object.
*/
static glsTEXTUREWRAPPER_PTR _ConstructWrapper(
    glsCONTEXT_PTR Context,
    GLuint Name
    )
{
    glsTEXTUREWRAPPER_PTR texture;
    gctPOINTER pointer = gcvNULL;

    gcmHEADER_ARG("Context=0x%x Name=%u", Context, Name);

    /* Allocate new texture descriptor. */
    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL,
                                   gcmSIZEOF(glsTEXTUREWRAPPER),
                                   &pointer)))
    {
        gcmFOOTER_NO();
        return gcvNULL;
    }

    texture = pointer;

    /* Init texture defaults. */
    _InitTextureWrapper(Context, texture);

    /* Set the name. */
    texture->name = Name;

    gcmFOOTER_ARG("texture=0x%x", texture);

    /* Return result. */
    return texture;
}

/*******************************************************************************
**
**  glGenTextures
**
**  glGenTextures returns 'Count' texture names in 'Textures'. There is no
**  guarantee that the names form a contiguous set of integers. However, it is
**  guaranteed that none of the returned names was in use immediately before the
**  call to glGenTextures.
**
**  The generated textures have no dimensionality; they assume the
**  dimensionality of the texture target to which they are first bound
**  (see glBindTexture).
**
**  Texture names returned by a call to glGenTextures are not returned by
**  subsequent calls, unless they are first deleted with glDeleteTextures.
**
**  INPUT:
**
**      Count
**          Specifies the number of texture names to be generated.
**
**      Textures
**          Specifies an array in which the generated texture names are stored.
**
**  OUTPUT:
**
**      Textures
**          An array filled with generated texture names.
*/
GL_API void GL_APIENTRY glGenTextures(
    GLsizei Count,
    GLuint* Textures
    )
{
    glsTEXTUREWRAPPER_PTR texture;
    GLsizei i;
    glsCONTEXT_PTR shared;
    GLuint name;

    glmENTER2(glmARGUINT, Count, glmARGPTR, Textures)
    {
        glmPROFILE(context, GLES1_GENTEXTURES, 0);

        /* Verify the arguments. */
        if (Count < 0)
        {
            /* The texture count is wrong. */
            glmERROR(GL_INVALID_VALUE);
            break;
        }

        if (Textures == gcvNULL)
        {
            break;
        }

        /* Map shared context. */
#if gcdRENDER_THREADS
        shared = (context->shared != gcvNULL) ? context->shared : context;
#else
        shared = context;
#endif

        /* Generate textures. */
        for (i = 0; i < Count; i++)
        {
            /* Set to default texture name in case of failure. */
            Textures[i] = 0;

            /* get next available name */
            name = _glffGetNextAvailableName(shared->texture.textureList);

            /* Construct new object. */
            texture = _ConstructWrapper(context, name);

            /* Make sure the allocation succeeded. */
            if (texture == gcvNULL)
            {
                continue;
            }
            else
            {
               /* Insert it to the list */
               _glffInsertTexture(shared->texture.textureList, texture);
            }

            /* Set the name. */
            Textures[i] = texture->name;
        }

        gcmDUMP_API("${ES11 glGenTextures 0x%08X (0x%08X)", Count, Textures);
        gcmDUMP_API_ARRAY(Textures, Count);
        gcmDUMP_API("$}");
    }
    glmLEAVE();
}

/*******************************************************************************
**
**  glDeleteTextures
**
**  glDeleteTextures deletes 'Count' textures named by the elements of the array
**  'Textures'. After a texture is deleted, it has no contents or dimensionality,
**  and its name is free for reuse (for example by glGenTextures). If a texture
**  that is currently bound is deleted, the binding reverts to 0 (the default
**  texture).
**
**  glDeleteTextures silently ignores 0's and names that do not correspond to
**  existing textures.
**
**  INPUT:
**
**      Count
**          Specifies the number of textures to be deleted.
**
**      Textures
**          Specifies an array of textures to be deleted.
**
**  OUTPUT:
**
**      Nothing.
*/

GL_API void GL_APIENTRY glDeleteTextures(
    GLsizei Count,
    const GLuint* Textures
    )
{
    glsTEXTUREWRAPPER * texture;
    glsTEXTURESAMPLER * sampler;
    GLsizei i;
    glsCONTEXT_PTR shared;

    glmENTER2(glmARGUINT, Count, glmARGPTR, Textures)
    {
        gcmDUMP_API("${ES11 glDeleteTextures 0x%08X (0x%08X)", Count, Textures);
        gcmDUMP_API_ARRAY(Textures, Count);
        gcmDUMP_API("$}");

        glmPROFILE(context, GLES1_DELETETEXTURES, 0);
        /* Verify the arguments. */
        if (Count < 0)
        {
            /* The texture count is wrong. */
            glmERROR(GL_INVALID_VALUE);
            break;
        }

        if (Textures == gcvNULL)
        {
            break;
        }

        /* Map shared context. */
#if gcdRENDER_THREADS
        shared = (context->shared != gcvNULL) ? context->shared : context;
#else
        shared = context;
#endif

        gcmLOCK_SHARE_OBJ(shared->texture.textureList);
        /* Iterate through the texture names. */
        for (i = 0; i < Count; i++)
        {
            GLsizei j;

            /* Skip default textures. */
            if (Textures[i] == 0)
            {
                continue;
            }

            /* Try to find the texture. */
            texture = glfFindTexture(context, Textures[i]);

            /* Skip if not found. */
            if (texture == gcvNULL)
            {
                continue;
            }

            /* detach the texture from RT.*/
            if(!_DetachTexture(context, texture))
            {
                glmERROR(GL_INVALID_OPERATION);
                break;
            }

            if (gcvNULL != context->frameBuffer)
            {
                if (context->frameBuffer->color.object == texture)
                {
                    glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, 0, 0);
                }

                if (context->frameBuffer->depth.object == texture)
                {
                    glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_TEXTURE_2D, 0, 0);
                }

                if (context->frameBuffer->stencil.object == texture)
                {
                    glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_STENCIL_ATTACHMENT_OES, GL_TEXTURE_2D, 0, 0);
                }
            }

            /* Unbind if bound. */
            for (j = 0; j < context->texture.pixelSamplers; j++)
            {
                /* Get a shortcut to the bound sampler. */
                sampler = &context->texture.sampler[j];

                /* Check binding. */
                if (sampler->bindings[texture->targetType] != texture)
                {
                    continue;
                }

                /* Bind to the default texture. */
                sampler->bindings[texture->targetType]
                    = &context->texture.defaultTexture[texture->targetType];

                texture->bindCount--;
                /* Is the texture currently selected? */
                if (sampler->binding == texture)
                {
                    /* Select the default. */
                    sampler->binding = sampler->bindings[texture->targetType];
                }
            }

            /* Do not delete the texObj if there are other texture units or contexts bound to it. */
            if (texture->bindCount != 0)
            {
                /* Set the flag to indicate the object is marked for delete */
                texture->flag |= __GL_OBJECT_IS_DELETED;
                break;
            }

            /* Destroy the texture. */
            if (gcmIS_ERROR(_ResetTextureWrapper(context, texture)))
            {
                glmERROR(GL_INVALID_OPERATION);
                break;
            }

            /* Map shared context. */
           _glffRemoveTexture(shared->texture.textureList, texture);

            /* Destroy the texture descriptor. */
            if (gcmIS_ERROR(gcoOS_Free(gcvNULL, texture)))
            {
                glmERROR(GL_INVALID_OPERATION);
                break;
            }
            else
            {
                texture = gcvNULL;
            }
        }
        gcmUNLOCK_SHARE_OBJ(shared->texture.textureList);
    }
    glmLEAVE();
}


/*******************************************************************************
**
**  glIsTexture
**
**  glIsTexture returns GL_TRUE if texture is currently the name of a texture.
**  If texture is zero, or is a non-zero value that is not currently the name
**  of a texture, or if an error occurs, glIsTexture returns GL_FALSE.
**
**  INPUT:
**
**      Texture
**          Specifies a value that may be the name of a texture.
**
**  OUTPUT:
**
**      GL_TRUE or GL_FALSE (see above description.)
*/

GL_API GLboolean GL_APIENTRY glIsTexture(
    GLuint Texture
    )
{
    glsTEXTUREWRAPPER_PTR texture;
    GLboolean result = GL_FALSE;

    glmENTER1(glmARGHEX, Texture)
    {
        glmPROFILE(context, GLES1_ISTEXTURE, 0);
        /* Try to find the texture. */
        texture = glfFindTexture(context, Texture);

        /* Set the result. */
        result = (texture != gcvNULL)
               ? (texture->boundAtLeastOnce != gcvFALSE)
               : GL_FALSE;

        gcmDUMP_API("${ES11 glIsTexture 0x%08X := 0x%08X}", Texture, result);
    }
    glmLEAVE();

    /* Return result. */
    return result;
}


/*******************************************************************************
**
**  glActiveTexture
**
**  glActiveTexture selects which texture unit subsequent texture state calls
**  will affect. The number of texture units an implementation supports is
**  implementation dependent, it must be at least 1.
**
**  INPUT:
**
**      Texture
**          Specifies which texture unit to make active. The number of texture
**          units is implementation dependent, but must be at least one.
**          'Texture' must be one of GL_TEXTUREi, where
**          0 <= i < GL_MAX_TEXTURE_UNITS, which is an implementation-dependent
**          value. The intial value is GL_TEXTURE0.
**
**  OUTPUT:
**
**      Nothing
*/

GL_API void GL_APIENTRY glActiveTexture(
    GLenum Texture
    )
{
    glmENTER1(glmARGHEX, Texture)
    {
        /* Convert to 0..glvMAX_TEXTURES range. */
        GLint index = Texture - GL_TEXTURE0;
        GLuint matrixID;

        gcmDUMP_API("${ES11 glActiveTexture 0x%08X}", Texture);

        glmPROFILE(context, GLES1_ACTIVETEXTURE, 0);
        /* Validate. */
        if ( (index < 0) || (index >= context->texture.pixelSamplers) )
        {
            /* Invalid texture unit. */
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        /* Set the texture unit. */
        context->texture.activeSampler = &context->texture.sampler[index];
        context->texture.activeSamplerIndex = index;

        /* Update matrix mode if necessary. */
        if ((context->matrixMode >= glvTEXTURE_MATRIX_0) &&
            (context->matrixMode <= glvTEXTURE_MATRIX_LAST))
        {
            glfSetMatrixMode(context, GL_TEXTURE);
        }

        /* Update current texture matrix. */
        matrixID = context->matrixStackArray[glvTEXTURE_MATRIX_0 + index].matrixID;
        (*context->matrixStackArray[glvTEXTURE_MATRIX_0 + index].currChanged) (
            context, matrixID
            );
    }
    glmLEAVE();
}


/*******************************************************************************
**
**  glClientActiveTexture
**
**  glClientActiveTexture selects the vertex array client state parameters to
**  be modified by glTexCoordPointer, and enabled or disabled with
**  glEnableClientState or glDisableClientState, respectively, when called with
**  a parameter of GL_TEXTURE_COORD_ARRAY.
**
**  INPUT:
**
**      Texture
**          Specifies which texture unit to make active. The number of texture
**          units is implementation dependent, but must be at least one.
**          'Texture' must be one of GL_TEXTUREi, where
**          0 <= i < GL_MAX_TEXTURE_UNITS, which is an implementation-dependent
**          value. The intial value is GL_TEXTURE0.
**
**  OUTPUT:
**
**      Nothing
*/

GL_API void GL_APIENTRY glClientActiveTexture(
    GLenum Texture
    )
{
    GLint index;

    glmENTER1(glmARGHEX, Texture)
    {
        gcmDUMP_API("${ES11 glClientActiveTexture 0x%08X}", Texture);

        /* Convert to 0..glvMAX_TEXTURES range. */
        index = Texture - GL_TEXTURE0;

        glmPROFILE(context, GLES1_CLIENTACTIVETEXTURE, 0);
        /* Validate. */
        if ( (index < 0) || (index >= context->texture.pixelSamplers) )
        {
            /* Invalid texture unit. */
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        /* Set the texture unit. */
        context->texture.activeClientSampler = &context->texture.sampler[index];
        context->texture.activeClientSamplerIndex = index;
    }
    glmLEAVE();
}


/*******************************************************************************
**
**  glBindTexture
**
**  glBindTexture lets you create or use a named texture. Calling glBindTexture
**  with target set to GL_TEXTURE_2D, and texture set to the name of the new
**  texture binds the texture name to the target. When a texture is bound to a
**  target, the previous binding for that target is automatically broken.
**  Texture names are unsigned integers. The value 0 is reserved to represent
**  the default texture for each texture target. Texture names and the
**  corresponding texture contents are local to the shared texture-object space
**  (see eglCreateContext) of the current GL rendering context.
**
**  You may use glGenTextures to generate a set of new texture names.
**
**  While a texture is bound, GL operations on the target to which it is bound
**  affect the bound texture. If texture mapping of the dimensionality of the
**  target to which a texture is bound is active, the bound texture is used.
**  In effect, the texture targets become aliases for the textures currently
**  bound to them, and the texture name 0 refers to the default textures that
**  were bound to them at initialization.
**
**  A texture binding created with glBindTexture remains active until a
**  different texture is bound to the same target, or until the bound texture
**  is deleted with glDeleteTextures.
**
**  Once created, a named texture may be re-bound to the target of the matching
**  dimensionality as often as needed. It is usually much faster to use
**  glBindTexture to bind an existing named texture to one of the texture
**  targets than it is to reload the texture image using glTexImage2D.
**
**  INPUT:
**
**      Target
**          Specifies the target to which the texture is bound.
**          Must be GL_TEXTURE_2D or GL_TEXTURE_CUBE_MAP_OES per
**          GL_OES_texture_cube_map extension.
**
**      Texture
**          Specifies the name of a texture.
**
**  OUTPUT:
**
**      Nothing.
*/

GL_API void GL_APIENTRY glBindTexture(
    GLenum Target,
    GLuint Texture
    )
{
    glsTEXTURESAMPLER_PTR sampler;
    glsTEXTUREWRAPPER_PTR texture;
    glsTEXTUREWRAPPER_PTR preTexture = gcvNULL;
    glmENTER2(glmARGUINT, Target, glmARGHEX, Texture)
    {
        gleTARGETTYPE target;
        glsCONTEXT_PTR shared;

        gcmDUMP_API("${ES11 glBindTexture 0x%08X 0x%08X}", Target, Texture);

        glmPROFILE(context, GLES1_BINDTEXTURE, 0);

        /* Map shared context. */
#if gcdRENDER_THREADS
        shared = (context->shared != gcvNULL) ? context->shared : context;
#else
        shared = context;
#endif

        /* Determine the target. */
        if (Target == GL_TEXTURE_2D)
        {
            /* 2D target. */
            target = glvTEXTURE2D;
        }
        else if (Target == GL_TEXTURE_CUBE_MAP_OES)
        {
            /* Cubemap target. */
            target = glvCUBEMAP;
        }
        else if (Target == GL_TEXTURE_EXTERNAL_OES)
        {
            /*External texture target*/
            target = glvTEXTUREEXTERNAL;
        }
        else
        {
            /* Invalid target. */
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        /* Get a shortcut to the active sampler. */
        sampler = context->texture.activeSampler;

        gcmLOCK_SHARE_OBJ(shared->texture.textureList);
        /* Bind to default? */
        if (Texture == 0)
        {
            texture = &context->texture.defaultTexture[target];
        }

        /* Try to find the texture; create it if does not exist. */
        else
        {
            /* try to get the required texture */
            texture = _glffFindTexture(shared->texture.textureList, Texture);
            if (texture == gcvNULL)
            {
                /* if we cannot find one, construct a new texture wrapper */
                texture = _ConstructWrapper(context, Texture);

                if (texture != gcvNULL)
                {
                    /* Insert it to the list */
                    _glffInsertTexture(shared->texture.textureList, texture);
                }
            }
        }

        /* Make sure we have a texture. */
        if (texture == gcvNULL)
        {
            gcmUNLOCK_SHARE_OBJ(shared->texture.textureList);
            break;
        }

        /* Set default values for external texture. */
        if (Target == GL_TEXTURE_EXTERNAL_OES)
        {
            texture->minFilter        = glvLINEAR;
            texture->magFilter        = glvLINEAR;
            texture->wrapS            = glvCLAMP;
            texture->wrapT            = glvCLAMP;
        }

        /* Don't rebind the same thing again. */
        if (sampler->bindings[target] == texture)
        {
            gcmUNLOCK_SHARE_OBJ(shared->texture.textureList);
            break;
        }

        /* Been bound before? */
        if (texture->boundAtLeastOnce)
        {
            /* Types have to match. */
            if (texture->targetType != target)
            {
                glmERROR(GL_INVALID_OPERATION);
                gcmUNLOCK_SHARE_OBJ(shared->texture.textureList);
                break;
            }
        }
        else
        {
            /* Assign the type. */
            texture->targetType = target;
        }

        /* Unbind currently bound texture. */
        sampler->bindings[target]->binding = gcvNULL;

        /* Is this the current system target? */
        if (sampler->binding == sampler->bindings[target])
        {
            /* Update the current system target as well. */
            sampler->binding = texture;
        }

        preTexture = sampler->bindings[target];
        sampler->bindings[target] = gcvNULL;
        /* Delete boundTexObj if there is nothing bound to the object */
        if (preTexture->name != 0)
        {
            if ((--preTexture->bindCount) == 0 && (preTexture->flag & __GL_OBJECT_IS_DELETED))
            {
                /* Destroy the texture. */
                if (gcmIS_ERROR(_ResetTextureWrapper(context, preTexture)))
                {
                    glmERROR(GL_INVALID_OPERATION);
                    gcmUNLOCK_SHARE_OBJ(shared->texture.textureList);
                    break;
                }

                _glffRemoveTexture(shared->texture.textureList, preTexture);

                /* Destroy the texture descriptor. */
                if (gcmIS_ERROR(gcoOS_Free(gcvNULL, preTexture)))
                {
                    glmERROR(GL_INVALID_OPERATION);
                    gcmUNLOCK_SHARE_OBJ(shared->texture.textureList);
                    break;
                }
                else
                {
                    preTexture = gcvNULL;
                }
            }
        }
        /* Update the sampler binding for the selected target. */
        sampler->bindings[target] = texture;
        texture->bindCount++;

        /* Bind the texture to the sampler. */
        texture->binding          = sampler;
        texture->boundAtLeastOnce = gcvTRUE;
        gcmUNLOCK_SHARE_OBJ(shared->texture.textureList);
    }
    glmLEAVE();
}


/*******************************************************************************
**
**  glTexParameter
**
**  glTexParameter assigns the value in 'Value' to the texture parameter
**  specified in 'Name'. 'Target' defines the target texture.
**
**  The following 'Name'/'Value' combinations are accepted:
**
**      GL_TEXTURE_MIN_FILTER
**          GL_NEAREST
**          GL_LINEAR
**          GL_NEAREST_MIPMAP_NEAREST
**          GL_LINEAR_MIPMAP_NEAREST
**          GL_NEAREST_MIPMAP_LINEAR (default)
**          GL_LINEAR_MIPMAP_LINEAR
**
**      GL_TEXTURE_MAG_FILTER
**          GL_NEAREST
**          GL_LINEAR (default)
**
**      GL_TEXTURE_WRAP_S
**          GL_CLAMP_TO_EDGE
**          GL_REPEAT (default)
**
**      GL_TEXTURE_WRAP_T
**          GL_CLAMP_TO_EDGE
**          GL_REPEAT (default)
**
**      GL_GENERATE_MIPMAP
**          GL_TRUE
**          GL_FALSE (default)
**
**  INPUT:
**
**      Target
**          Specifies the target texture. Must be GL_TEXTURE_2D or
**          GL_TEXTURE_CUBE_MAP_OES per GL_OES_texture_cube_map extension.
**
**      Name
**          Specifies the symbolic name of a single-valued texture parameter.
**          'Name' can be one of the following: GL_TEXTURE_MIN_FILTER,
**          GL_TEXTURE_MAG_FILTER, GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T
**          or GL_GENERATE_MIPMAP.
**
**      Value
**          Specifies the value of 'Name'.
**
**  OUTPUT:
**
**      Nothing.
*/

GL_API void GL_APIENTRY glTexParameterf(
    GLenum Target,
    GLenum Name,
    GLfloat Value
    )
{
    glmENTER3(glmARGENUM, Target, glmARGENUM, Name, glmARGFLOAT, Value)
    {
        glmPROFILE(context, GLES1_TEXPARAMETERF, 0);
        if (Name == GL_TEXTURE_CROP_RECT_OES)
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        if (!_SetTextureParameter(context, Target, Name, &Value))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        gcmDUMP_API("${ES11 glTexParameterf 0x%08X 0x%08X 0x%08X}", Target, Name, *(GLuint*)&Value);
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glTexParameterfv(
    GLenum Target,
    GLenum Name,
    const GLfloat* Value
    )
{
    glmENTER3(glmARGENUM, Target, glmARGENUM, Name, glmARGPTR, Value)
    {
        gcmDUMP_API("${ES11 glTexParameterfv 0x%08X 0x%08X (0x%08X)", Target, Name, Value);
        gcmDUMP_API_ARRAY(Value, 1);
        gcmDUMP_API("$}");

        glmPROFILE(context, GLES1_TEXPARAMETERFV, 0);

        if (!_SetTextureParameter(context, Target, Name, Value))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glTexParameterx(
    GLenum Target,
    GLenum Name,
    GLfixed Value
    )
{
    glmENTER3(glmARGENUM, Target, glmARGENUM, Name, glmARGFIXED, Value)
    {
        GLfloat value;

        gcmDUMP_API("${ES11 glTexParameterx 0x%08X 0x%08X 0x%08X}", Target, Name, Value);
        glmPROFILE(context, GLES1_TEXPARAMETERX, 0);
        if (Name == GL_TEXTURE_CROP_RECT_OES)
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        /* Convert to float */
        value = (GLfloat) Value;

        if (!_SetTextureParameter(context, Target, Name, &value))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glTexParameterxOES(
    GLenum Target,
    GLenum Name,
    GLfixed Value
    )
{
    glmENTER3(glmARGENUM, Target, glmARGENUM, Name, glmARGFIXED, Value)
    {
        GLfloat value;

        gcmDUMP_API("${ES11 glTexParameterxOES 0x%08X 0x%08X 0x%08X}", Target, Name, Value);

        if (Name == GL_TEXTURE_CROP_RECT_OES)
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        /* Convert to float */
        value = (GLfloat) Value;

        if (!_SetTextureParameter(context, Target, Name, &value))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glTexParameterxv(
    GLenum Target,
    GLenum Name,
    const GLfixed* Value
    )
{
    glmENTER3(glmARGENUM, Target, glmARGENUM, Name, glmARGPTR, Value)
    {
        GLfloat value[4];

        gcmDUMP_API("${ES11 glTexParameterxv 0x%08X 0x%08X (0x%08X)", Target, Name, Value);
        gcmDUMP_API_ARRAY(Value, 1);
        gcmDUMP_API("$}");

        if (Name == GL_TEXTURE_CROP_RECT_OES)
        {
            value[0] = glmFIXED2FLOAT(Value[0]);
            value[1] = glmFIXED2FLOAT(Value[1]);
            value[2] = glmFIXED2FLOAT(Value[2]);
            value[3] = glmFIXED2FLOAT(Value[3]);
        }
        else
        {
            value[0] = (GLfloat)Value[0];
        }

        if (!_SetTextureParameter(context, Target, Name, value))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glTexParameterxvOES(
    GLenum Target,
    GLenum Name,
    const GLfixed* Value
    )
{
    glmENTER3(glmARGENUM, Target, glmARGENUM, Name, glmARGPTR, Value)
    {
        GLfloat value[4];

        gcmDUMP_API("${ES11 glTexParameterxvOES 0x%08X 0x%08X (0x%08X)", Target, Name, Value);
        gcmDUMP_API_ARRAY(Value, 1);
        gcmDUMP_API("$}");

        /* Covnert to Float */
        if (Name == GL_TEXTURE_CROP_RECT_OES)
        {
            value[0] = glmFIXED2FLOAT(Value[0]);
            value[1] = glmFIXED2FLOAT(Value[1]);
            value[2] = glmFIXED2FLOAT(Value[2]);
            value[3] = glmFIXED2FLOAT(Value[3]);
        }
        else
        {
            value[0] = (GLfloat)Value[0];
        }

        if (!_SetTextureParameter(context, Target, Name, value))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glTexParameteri(
    GLenum Target,
    GLenum Name,
    GLint Value
    )
{
    glmENTER3(glmARGENUM, Target, glmARGENUM, Name, glmARGINT, Value)
    {
        GLfloat value;

        gcmDUMP_API("${ES11 glTexParameteri 0x%08X 0x%08X 0x%08X}", Target, Name, Value);
        glmPROFILE(context, GLES1_TEXPARAMETERI, 0);

        if (Name == GL_TEXTURE_CROP_RECT_OES)
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        /* Convert to float */
        value = glmINT2FLOAT(Value);

        if (!_SetTextureParameter(context, Target, Name, &value))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glTexParameteriv(
    GLenum Target,
    GLenum Name,
    const GLint* Value
    )
{
    glmENTER3(glmARGENUM, Target, glmARGENUM, Name, glmARGPTR, Value)
    {
        GLfloat value[4];

        gcmDUMP_API("${ES11 glTexParameteriv 0x%08X 0x%08X (0x%08X)", Target, Name, Value);
        gcmDUMP_API_ARRAY(Value, (Name == GL_TEXTURE_CROP_RECT_OES) ? 4 : 1);
        gcmDUMP_API("$}");

        glmPROFILE(context, GLES1_TEXPARAMETERIV, 0);

        /* Covnert to Float */
        value[0] = glmINT2FLOAT(Value[0]);
        if (Name == GL_TEXTURE_CROP_RECT_OES)
        {
            value[1] = glmINT2FLOAT(Value[1]);
            value[2] = glmINT2FLOAT(Value[2]);
            value[3] = glmINT2FLOAT(Value[3]);
        }

        if (!_SetTextureParameter(context, Target, Name, value))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }
    }
    glmLEAVE();
}


/*******************************************************************************
**
**  glGetTexParameter
**
**  glGetTexParameter returns in 'Value' the value of the texture parameter
**  specified in 'Name'. 'Target' defines the target texture. 'Name' accepts
**  the same symbols as glTexParameter, with the same interpretations.
**
**  INPUT:
**
**      Target
**          Specifies the target texture. Must be GL_TEXTURE_2D or
**          GL_TEXTURE_CUBE_MAP_OES per GL_OES_texture_cube_map extension.
**
**      Name
**          Specifies the symbolic name of a single-valued texture parameter.
**          'Name' can be one of the following: GL_TEXTURE_MIN_FILTER,
**          GL_TEXTURE_MAG_FILTER, GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T
**          or GL_GENERATE_MIPMAP.
**
**      Value
**          Returns the texture parameters.
**
**  OUTPUT:
**
**      See above.
*/

GL_API void GL_APIENTRY glGetTexParameterfv(
    GLenum Target,
    GLenum Name,
    GLfloat* Value
    )
{
    glmENTER3(glmARGENUM, Target, glmARGENUM, Name, glmARGPTR, Value)
    {
        glmPROFILE(context, GLES1_GETTEXPARAMETERFV, 0);
        if (!_GetTextureParameter(context, Target, Name, Value, glvFLOAT))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        /* TODO: Is the size fixed to 1. */
        gcmDUMP_API("${ES11 glGetTexParameterfv 0x%08X 0x%08X (0x%08X)", Target, Name, Value);
        gcmDUMP_API_ARRAY(Value, 1);
        gcmDUMP_API("$}");
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glGetTexParameterxv(
    GLenum Target,
    GLenum Name,
    GLfixed* Value
    )
{
    glmENTER3(glmARGENUM, Target, glmARGENUM, Name, glmARGPTR, Value)
    {
        glmPROFILE(context, GLES1_GETTEXPARAMETERXV, 0);
        if (!_GetTextureParameter(context, Target, Name,
                                  Value, glvFIXED))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        gcmDUMP_API("${ES11 glGetTexParameterxv 0x%08X 0x%08X (0x%08X)", Target, Name, Value);
        gcmDUMP_API_ARRAY(Value, 1);
        gcmDUMP_API("$}");
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glGetTexParameterxvOES(
    GLenum Target,
    GLenum Name,
    GLfixed* Value
    )
{
    glmENTER3(glmARGENUM, Target, glmARGENUM, Name, glmARGPTR, Value)
    {
        if (!_GetTextureParameter(context, Target, Name,
                                  Value, glvFIXED))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        gcmDUMP_API("${ES11 glGetTexParameterxvOES 0x%08X 0x%08X (0x%08X)", Target, Name, Value);
        gcmDUMP_API_ARRAY(Value, 1);
        gcmDUMP_API("$}");
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glGetTexParameteriv(
    GLenum Target,
    GLenum Name,
    GLint* Value
    )
{
    glmENTER3(glmARGENUM, Target, glmARGENUM, Name, glmARGPTR, Value)
    {
        glmPROFILE(context, GLES1_GETTEXPARAMETERIV, 0);
        if (!_GetTextureParameter(context, Target, Name,
                                  Value, glvINT))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        gcmDUMP_API("${ES11 glGetTexParameteriv 0x%08X 0x%08X (0x%08X)", Target, Name, Value);
        gcmDUMP_API_ARRAY(Value, 1);
        gcmDUMP_API("$}");
    }
    glmLEAVE();
}


/*******************************************************************************
**
**  glTexEnv
**
**  A texture environment specifies how texture values are interpreted when a
**  fragment is textured. 'Target' must be GL_TEXTURE_ENV.
**
**  If 'Name' is GL_TEXTURE_ENV_MODE, then 'Param' or 'Params' is
**  (or points to) the symbolic name of a texture function.
**  Four texture functions may be specified:
**  GL_MODULATE, GL_DECAL, GL_BLEND, and GL_REPLACE.
**
**  A texture function acts on the fragment to be textured using the texture
**  image value that applies to the fragment (see glTexParameter) and produces
**  an RGBA color for that fragment. The following table shows how the RGBA
**  color is produced for each of the three texture functions that can be
**  chosen. C is a triple of color values (RGB) and A is the associated alpha
**  value. RGBA values extracted from a texture image are in the range [0, 1].
**  The subscript f refers to the incoming fragment, the subscript t to the
**  texture image, the subscript c to the texture environment color, and
**  subscript v indicates a value produced by the texture function.
**
**  INPUT:
**
**      Target
**          Specifies a texture environment.
**
**      Name
**          Specifies the symbolic name of a texture environment parameter.
**
**      Param
**          Specifies a parameter or a pointer to a parameter array that
**          contains either a single symbolic constant or an RGBA color.
**
**  OUTPUT:
**
**      Nothing.
*/

GL_API void GL_APIENTRY glTexEnvf(
    GLenum Target,
    GLenum Name,
    GLfloat Param
    )
{
    glmENTER3(glmARGENUM, Target, glmARGENUM, Name, glmARGFLOAT, Param)
    {
        gcmDUMP_API("${ES11 glTexEnvf 0x%08X 0x%08X 0x%08X}", Target, Name, *(GLuint*)&Param);

        glmPROFILE(context, GLES1_TEXENVF, 0);
        if (Name == GL_TEXTURE_ENV_COLOR)
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        if (!_SetTextureState(context, Target, Name, &Param))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glTexEnvfv(
    GLenum Target,
    GLenum Name,
    const GLfloat* Params
    )
{
    glmENTER3(glmARGENUM, Target, glmARGENUM, Name, glmARGPTR, Params)
    {
        gcmDUMP_API("${ES11 glTexEnvfv 0x%08X 0x%08X (0x%08X)", Target, Name, Params);
        gcmDUMP_API_ARRAY(Params, 1);
        gcmDUMP_API("$}");

        glmPROFILE(context, GLES1_TEXENVFV, 0);
        if (!_SetTextureState(context, Target, Name, Params))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glTexEnvx(
    GLenum Target,
    GLenum Name,
    GLfixed Param
    )
{
    glmENTER3(glmARGENUM, Target, glmARGENUM, Name, glmARGFIXED, Param)
    {
        GLfloat param;

        gcmDUMP_API("${ES11 glTexEnvx 0x%08X 0x%08X 0x%08X}", Target, Name, Param);
        glmPROFILE(context, GLES1_TEXENVX, 0);
        if (Name == GL_TEXTURE_ENV_COLOR)
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        /* Convert to float */
        if (Name == GL_RGB_SCALE || Name == GL_ALPHA_SCALE)
        {
            param = glmFIXED2FLOAT(Param);
        }
        else
        {
            param = (GLfloat) Param;
        }

        if (!_SetTextureState(context, Target, Name, &param))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glTexEnvxOES(
    GLenum Target,
    GLenum Name,
    GLfixed Param
    )
{
    glmENTER3(glmARGENUM, Target, glmARGENUM, Name, glmARGFIXED, Param)
    {
        GLfloat param;

        gcmDUMP_API("${ES11 glTexEnvxOES 0x%08X 0x%08X 0x%08X}", Target, Name, Param);

        if (Name == GL_TEXTURE_ENV_COLOR)
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        /* Convert to float */
        if (Name == GL_RGB_SCALE || Name == GL_ALPHA_SCALE)
        {
            param = glmFIXED2FLOAT(Param);
        }
        else
        {
            param = (GLfloat) Param;
        }

        if (!_SetTextureState(context, Target, Name, &param))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glTexEnvxv(
    GLenum Target,
    GLenum Name,
    const GLfixed* Params
    )
{
    glmENTER3(glmARGENUM, Target, glmARGENUM, Name, glmARGPTR, Params)
    {
        GLfloat params[4];

        gcmDUMP_API("${ES11 glTexEnvxv 0x%08X 0x%08X (0x%08X)", Target, Name, Params);
        gcmDUMP_API_ARRAY(Params, 1);
        gcmDUMP_API("$}");

        glmPROFILE(context, GLES1_TEXENVXV, 0);

        /* Convert to float */
        if (Name == GL_TEXTURE_ENV_COLOR)
        {
            params[0] = glmFIXED2FLOAT(Params[0]);
            params[1] = glmFIXED2FLOAT(Params[1]);
            params[2] = glmFIXED2FLOAT(Params[2]);
            params[3] = glmFIXED2FLOAT(Params[3]);
        }
        else if (Name == GL_RGB_SCALE || Name == GL_ALPHA_SCALE)
        {
            params[0] = glmFIXED2FLOAT(Params[0]);
        }
        else
        {
            params[0] = (GLfloat) Params[0];
        }

        if (!_SetTextureState(context, Target, Name, params))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glTexEnvxvOES(
    GLenum Target,
    GLenum Name,
    const GLfixed* Params
    )
{
    glmENTER3(glmARGENUM, Target, glmARGENUM, Name, glmARGPTR, Params)
    {
        GLfloat params[4];

        gcmDUMP_API("${ES11 glTexEnvxvOES 0x%08X 0x%08X (0x%08X)", Target, Name, Params);
        gcmDUMP_API_ARRAY(Params, 1);
        gcmDUMP_API("$}");

        /* Convert to float */
        if (Name == GL_TEXTURE_ENV_COLOR)
        {
            params[0] = glmFIXED2FLOAT(Params[0]);
            params[1] = glmFIXED2FLOAT(Params[1]);
            params[2] = glmFIXED2FLOAT(Params[2]);
            params[3] = glmFIXED2FLOAT(Params[3]);
        }
        else if (Name == GL_RGB_SCALE || Name == GL_ALPHA_SCALE)
        {
            params[0] = glmFIXED2FLOAT(Params[0]);
        }
        else
        {
            params[0] = (GLfloat) Params[0];
        }

        if (!_SetTextureState(context, Target, Name, params))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glTexEnvi(
    GLenum Target,
    GLenum Name,
    GLint Param
    )
{
    glmENTER3(glmARGENUM, Target, glmARGENUM, Name, glmARGUINT, Param)
    {
        GLfloat param;

        gcmDUMP_API("${ES11 glTexEnvi 0x%08X 0x%08X 0x%08X}", Target, Name, Param);
        glmPROFILE(context, GLES1_TEXENVI, 0);
        if (Name == GL_TEXTURE_ENV_COLOR)
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        /* Convert to float */
        param = glmINT2FLOAT(Param);

        if (!_SetTextureState(context, Target, Name, &param))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glTexEnviv(
    GLenum Target,
    GLenum Name,
    const GLint* Params
    )
{
    glmENTER3(glmARGENUM, Target, glmARGENUM, Name, glmARGPTR, Params)
    {
        GLfloat params[4];

        gcmDUMP_API("${ES11 glTexEnviv 0x%08X 0x%08X (0x%08X)", Target, Name, Params);
        gcmDUMP_API_ARRAY(Params, 1);
        gcmDUMP_API("$}");
        glmPROFILE(context, GLES1_TEXENVIV, 0);

        /* Convert to float */
        params[0] = glmINT2FLOAT(Params[0]);
        if (Name == GL_TEXTURE_ENV_COLOR)
        {
            params[1] = glmINT2FLOAT(Params[1]);
            params[2] = glmINT2FLOAT(Params[2]);
            params[3] = glmINT2FLOAT(Params[3]);
        }

        if (!_SetTextureState(context, Target, Name, params))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }
    }
    glmLEAVE();
}


/*******************************************************************************
**
**  glGetTexEnv
**
**  Returns a specified value from the texture environment.
**
**  INPUT:
**
**      Target
**          Specifies a texture environment. Must be GL_TEXTURE_ENV.
**
**      Name
**          Specifies the symbolic name of a texture environment parameter.
**
**      Param
**          Specifies a parameter or a pointer to a parameter array that
**          contains either a single symbolic constant or an RGBA color.
**
**  OUTPUT:
**
**      Nothing.
*/

GL_API void GL_APIENTRY glGetTexEnviv(
    GLenum Target,
    GLenum Name,
    GLint* Params
    )
{
    glmENTER3(glmARGENUM, Target, glmARGENUM, Name, glmARGPTR, Params)
    {
        glmPROFILE(context, GLES1_GETTEXENVIV, 0);
        if (!_GetTextureState(context, Target, Name, Params, glvINT))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        gcmDUMP_API("${ES11 glGetTexEnviv 0x%08X 0x%08X (0x%08X)", Target, Name, Params);
        gcmDUMP_API_ARRAY(Params, 1);
        gcmDUMP_API("$}");
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glGetTexEnvfv(
    GLenum Target,
    GLenum Name,
    GLfloat* Params
    )
{
    glmENTER3(glmARGENUM, Target, glmARGENUM, Name, glmARGPTR, Params)
    {
        glmPROFILE(context, GLES1_GETTEXENVFV, 0);
        if (!_GetTextureState(context, Target, Name, Params, glvFLOAT))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        gcmDUMP_API("${ES11 glGetTexEnvfv 0x%08X 0x%08X (0x%08X)", Target, Name, Params);
        gcmDUMP_API_ARRAY(Params, 1);
        gcmDUMP_API("$}");
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glGetTexEnvxv(
    GLenum Target,
    GLenum Name,
    GLfixed* Params
    )
{
    glmENTER3(glmARGENUM, Target, glmARGENUM, Name, glmARGPTR, Params)
    {
        glmPROFILE(context, GLES1_GETTEXENVXV, 0);
        if (!_GetTextureState(context, Target, Name, Params, glvFIXED))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        gcmDUMP_API("${ES11 glGetTexEnvxv 0x%08X 0x%08X (0x%08X)", Target, Name, Params);
        gcmDUMP_API_ARRAY(Params, 1);
        gcmDUMP_API("$}");
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glGetTexEnvxvOES(
    GLenum Target,
    GLenum Name,
    GLfixed* Params
    )
{
    glmENTER3(glmARGENUM, Target, glmARGENUM, Name, glmARGPTR, Params)
    {
        if (!_GetTextureState(context, Target, Name, Params, glvFIXED))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        gcmDUMP_API("${ES11 glGetTexEnvxvOES 0x%08X 0x%08X (0x%08X)", Target, Name, Params);
        gcmDUMP_API_ARRAY(Params, 1);
        gcmDUMP_API("$}");
    }
    glmLEAVE();
}


/*******************************************************************************
**
**  glTexImage2D
**
**  Texturing maps a portion of a specified texture image onto each graphical
**  primitive for which texturing is enabled. To enable and disable
**  two-dimensional texturing, call glEnable and glDisable with argument
**  GL_TEXTURE_2D. Two-dimensional texturing is initially disabled.
**
**  To define texture images, call glTexImage2D. The arguments describe the
**  parameters of the texture image, such as height, width, width of the border,
**  level-of-detail number (see glTexParameter), and number of color components
**  provided. The last three arguments describe how the image is represented in
**  memory.
**
**  Data is read from pixels as a sequence of unsigned bytes or shorts,
**  depending on type. These values are grouped into sets of one, two, three,
**  or four values, depending on format, to form elements.
**
**  When type is GL_UNSIGNED_BYTE, each of these bytes is interpreted as one
**  color component, depending on format. When type is one of
**  GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_SHORT_4_4_4_4,
**  GL_UNSIGNED_SHORT_5_5_5_1, each unsigned value is interpreted as containing
**  all the components for a single pixel, with the color components arranged
**  according to format.
**
**  The first element corresponds to the lower left corner of the texture image.
**  Subsequent elements progress left-to-right through the remaining texels in
**  the lowest row of the texture image, and then in successively higher rows of
**  the texture image. The final element corresponds to the upper right corner
**  of the texture image.
**
**  By default, adjacent pixels are taken from adjacent memory locations, except
**  that after all width pixels are read, the read pointer is advanced to the
**  next four-byte boundary. The four-byte row alignment is specified by
**  glPixelStore with argument GL_UNPACK_ALIGNMENT, and it can be set to one,
**  two, four, or eight bytes.
**
**  INPUT:
**
**      Target
**          Specifies the target texture. Must be GL_TEXTURE_2D or
**          GL_TEXTURE_CUBE_MAP_xxx_OES per GL_OES_texture_cube_map extension.
**
**      Level
**          Specifies the level-of-detail number. Level 0 is the base image
**          level. Level n is the nth mipmap reduction image. Must be greater
**          or equal 0.
**
**      InternalFormat
**          Specifies the number of color components in the texture. Must be
**          GL_ALPHA, GL_RGB, GL_RGBA, GL_LUMINANCE, or GL_LUMINANCE_ALPHA.
**
**      Width
**          Specifies the width of the texture image. Must be 2**n for some
**          integer n. All implementations support texture images that are
**          at least 64 texels wide.
**
**      Height
**          Specifies the height of the texture image. Must be 2**m for some
**          integer m. All implementations support texture images that are
**          at least 64 texels high.
**
**      Border
**          Specifies the width of the border. Must be 0.
**
**      Format
**          Specifies the format of the pixel data. Must be same as
**          internalformat. The following symbolic values are accepted:
**          GL_ALPHA, GL_RGB, GL_RGBA, GL_LUMINANCE, and GL_LUMINANCE_ALPHA.
**
**      Type
**          Specifies the data type of the pixel data. The following symbolic
**          values are accepted: GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT_5_6_5,
**          GL_UNSIGNED_SHORT_4_4_4_4, and GL_UNSIGNED_SHORT_5_5_5_1.
**
**      Pixels
**          Specifies a pointer to the image data in memory.
**
**  OUTPUT:
**
**      Nothing.
*/

GL_API void GL_APIENTRY glTexImage2D(
    GLenum Target,
    GLint Level,
    GLint InternalFormat,
    GLsizei Width,
    GLsizei Height,
    GLint Border,
    GLenum Format,
    GLenum Type,
    const GLvoid* Pixels
    )
{
    gceSTATUS status;

    glmENTER8(glmARGENUM, Target, glmARGINT, Level, glmARGENUM, InternalFormat,
              glmARGINT, Width, glmARGINT, Height, glmARGENUM, Format,
              glmARGENUM, Type, glmARGPTR, Pixels)
    {
        gctINT faces;
        gceTEXTURE_FACE face;
        gceSURF_FORMAT srcFormat;
        gceSURF_FORMAT dstFormat;
        glsTEXTURESAMPLER_PTR sampler;
        glsTEXTUREWRAPPER_PTR texture;
        gctINT stride;
        gcoSURF shareObj = gcvNULL;

        glmPROFILE(context, GLES1_TEXIMAGE2D, 0);

        /* Get a shortcut to the active sampler. */
        sampler = context->texture.activeSampler;

        /* Determine the target and the texture. */
        switch (Target)
        {
        case GL_TEXTURE_2D:
            texture = sampler->bindings[glvTEXTURE2D];
            face    = gcvFACE_NONE;
            faces   = 0;
            break;

        case GL_TEXTURE_CUBE_MAP_POSITIVE_X_OES:
            if (Width != Height)
            {
                glmERROR(GL_INVALID_VALUE);
                goto OnError;
            }
            texture = sampler->bindings[glvCUBEMAP];
            face    = gcvFACE_POSITIVE_X;
            faces   = 6;
            break;

        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X_OES:
            if (Width != Height)
            {
                glmERROR(GL_INVALID_VALUE);
                goto OnError;
            }
            texture = sampler->bindings[glvCUBEMAP];
            face    = gcvFACE_NEGATIVE_X;
            faces   = 6;
            break;

        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y_OES:
            if (Width != Height)
            {
                glmERROR(GL_INVALID_VALUE);
                goto OnError;
            }
            texture = sampler->bindings[glvCUBEMAP];
            face    = gcvFACE_POSITIVE_Y;
            faces   = 6;
            break;

        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_OES:
            if (Width != Height)
            {
                glmERROR(GL_INVALID_VALUE);
                goto OnError;
            }
            texture = sampler->bindings[glvCUBEMAP];
            face    = gcvFACE_NEGATIVE_Y;
            faces   = 6;
            break;

        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z_OES:
            if (Width != Height)
            {
                glmERROR(GL_INVALID_VALUE);
                goto OnError;
            }
            texture = sampler->bindings[glvCUBEMAP];
            face    = gcvFACE_POSITIVE_Z;
            faces   = 6;
            break;

        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_OES:
            if (Width != Height)
            {
                glmERROR(GL_INVALID_VALUE);
                goto OnError;
            }
            texture = sampler->bindings[glvCUBEMAP];
            face    = gcvFACE_NEGATIVE_Z;
            faces   = 6;
            break;

        default:
            glmERROR(GL_INVALID_ENUM);
            goto OnError;
        }

        /* Texture has to be something. */
        gcmASSERT(texture != gcvNULL);

        /* Validate arguments. */
        if (!_validateFormat(Format) || !_validateType(Type))

        {
            glmERROR(GL_INVALID_ENUM);
            goto OnError;
        }

        if ((Level < 0) || (Border != 0) ||
            /* Valid Width and Height. */
            (Width < 0) || (Height < 0)  ||
            (Width  > (GLsizei)context->maxTextureWidth)  ||
            (Height > (GLsizei)context->maxTextureHeight) ||
            /* Valid level. */
            (Level  > (GLint)gcoMATH_Ceiling(
                                 gcoMATH_Log2(
                                    (gctFLOAT)context->maxTextureWidth))) ||
            !_validateFormat(InternalFormat))
        {
            glmERROR(GL_INVALID_VALUE);
            goto OnError;
        }

        /* Check internal format and format conformance. */
        if (((GLenum) InternalFormat != Format) &&
            !((InternalFormat == GL_RGBA) && (Format == GL_BGRA_EXT))
           )
        {
            glmERROR(GL_INVALID_OPERATION);
            goto OnError;
        }

        if (!_getFormat(Format, Type, &srcFormat))
        {
            glmERROR(GL_INVALID_OPERATION);
            goto OnError;
        }

        /* Get closest supported destination. */
        status = gcoTEXTURE_GetClosestFormat(
            context->hal, srcFormat, &dstFormat
            );

        if (gcmIS_ERROR(status))
        {
            glmERROR(GL_INVALID_VALUE);
            goto OnError;
        }

        /* Null texture? */
        if ((Width == 0) || (Height == 0))
        {
            /* Destroy the texture. */
            gcmVERIFY_OK(_ResetTextureWrapper(context, texture));
            break;
        }

        /* Reset the texture if planar object is defined. */
        if ((texture->direct.source != gcvNULL)
        ||  (texture->image.source != gcvNULL)
        )
        {
            /* Destroy the texture. */
            gcmVERIFY_OK(_ResetTextureWrapper(context, texture));
        }

        /* Construct texture object. */
        if (texture->object == gcvNULL)
        {
            /* Construct the gcoTEXTURE object. */
            status = gcoTEXTURE_ConstructEx(context->hal, _HALtexType[texture->targetType], &texture->object);

            if (gcmIS_ERROR(status))
            {
                /* Error. */
                glmERROR(GL_OUT_OF_MEMORY);
                goto OnError;
            }
        }

        /* Add the mipmap. If it already exists, the call will be ignored. */
        status = gcoTEXTURE_AddMipMap(
            texture->object,
            Level,
            InternalFormat,
            dstFormat,
            Width, Height, 0,
            faces,
            gcvPOOL_DEFAULT,
            &shareObj
            );

        if (gcmIS_ERROR(status))
        {
            /* Destroy the texture. */
            gcmVERIFY_OK(_ResetTextureWrapper(context, texture));

            glmERROR(GL_OUT_OF_MEMORY);
            goto OnError;
        }

        /* Set shared lock.*/
        gcmVERIFY_OK(gcoSURF_SetSharedLock(shareObj, context->texture.textureList->sharedLock));
        /* Get Stride of source texture */
        stride = 0;
        _GetSourceStride(
            InternalFormat, Type, Width, context->unpackAlignment, &stride
            );

        /* Upload the texture. */
        if (Pixels != gcvNULL)
        {
            do
            {
                if (srcFormat == gcvSURF_ETC1)
                {
                    status = gcoTEXTURE_UploadCompressed(texture->object,
                                                         Level,
                                                         face,
                                                         Width, Height, 0,
                                                         Pixels,
                                                         ((Width + 3) / 4) * ((Height + 3) / 4) * 8);
                }
                else if(srcFormat == gcvSURF_DXT3 || srcFormat == gcvSURF_DXT5)
                {
                    /* size must be 4x4 block aligned. */
                    gctSIZE_T size = ((Width + 3) & (~0x3))
                                   * ((Height + 3) & (~0x3));

                    status = gcoTEXTURE_UploadCompressed(texture->object,
                                                         Level,
                                                         face,
                                                         Width, Height, 0,
                                                         Pixels,
                                                         size
                                                         );
                }
                else if (srcFormat == gcvSURF_DXT1)
                {
                    /* size must be 4x4 block aligned. */
                    gctSIZE_T size = ((Width + 3) & (~0x3))
                                   * ((Height + 3) & (~0x3));

                    status = gcoTEXTURE_UploadCompressed(texture->object,
                                                         Level,
                                                         face,
                                                         Width, Height, 0,
                                                         Pixels,
                                                         size / 2
                                                         );
                }
                else
                {
                    status = gcoTEXTURE_Upload(texture->object,
                                               Level,
                                               face,
                                               Width, Height, 0,
                                               Pixels,
                                               stride,
                                               srcFormat,
                                               gcvSURF_COLOR_SPACE_LINEAR);
                }
            }
            while (gcvFALSE);

            if (status == gcvSTATUS_NOT_SUPPORTED)
            {
                glmERROR(GL_INVALID_OPERATION);
                goto OnError;
            }

            /* Mark texture as dirty. */
            texture->dirty = gcvTRUE;
        }
        else
        {
            if (context->clearTexturePatch)
            {
                gcoSURF  surface = gcvNULL;
                gctPOINTER memory = gcvNULL;
                gctUINT  aWidth, aHeight;
                gctINT   stride;

                gcmVERIFY_OK(gcoTEXTURE_GetMipMap(texture->object, Level, &surface));

                gcmVERIFY_OK(gcoSURF_GetAlignedSize(surface, &aWidth, &aHeight, &stride));

                gcmVERIFY_OK(gcoSURF_Lock(surface, gcvNULL, &memory));

                gcoOS_ZeroMemory(memory,stride * aHeight);

                gcmVERIFY_OK(gcoSURF_Unlock(surface, memory));

            }
        }

        /* Verify the result. */
        gcmASSERT(!gcmIS_ERROR(status));

        /* Level 0 is special. */
        if (Level == 0)
        {
            GLint tValue;
            /* Set endian hint */
            gcmVERIFY_OK(gcoTEXTURE_SetEndianHint(
                texture->object, _getEndianHint(Format, Type)
                ));

            /* Invalidate normalized crop rectangle. */
            texture->dirtyCropRect = GL_TRUE;

            /* Set texture parameters. */
            texture->width  = Width;
            texture->height = Height;

            /* Determine the number of mipmaps. */
            tValue = glfGetMaxLOD(texture->width, texture->height);
            texture->maxLevel = gcmMIN(tValue, texture->maxLevel);

            /* Update texture format. */
            _SetTextureWrapperFormat(context, texture, Format);

            /* Auto generate mipmaps. */
            if (texture->genMipmap)
            {
                gcmERR_BREAK(glfGenerateMipMaps(
                    context, texture, dstFormat, 0, Width, Height, faces
                    ));
            }
        }


            gcmDUMP_API("${ES11 glTexImage2D 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X (0x%08X)",
                Target, Level, InternalFormat, Width, Height, Border, Format, Type, Pixels);
            gcmDUMP_API_DATA(Pixels, stride * Height);
            gcmDUMP_API("$}");

        /* Easy return on error. */
OnError:;
    }
    glmLEAVE();
}


/*******************************************************************************
**
**  glTexSubImage2D
**
**  Texturing maps a portion of a specified texture image onto each graphical
**  primitive for which texturing is enabled. To enable and disable
**  two-dimensional texturing, call glEnable and glDisable with argument
**  GL_TEXTURE_2D. Two-dimensional texturing is initially disabled.
**
**  glTexSubImage2D redefines a contiguous subregion of an existing
**  two-dimensional texture image. The texels referenced by pixels replace the
**  portion of the existing texture array with x indices xoffset and
**  xoffset + width - 1, inclusive, and y indices yoffset and
**  yoffset + height - 1, inclusive. This region may not include any texels
**  outside the range of the texture array as it was originally specified.
**  It is not an error to specify a subtexture with zero width or height,
**  but such a specification has no effect.
**
**  INPUT:
**
**      Target
**          Specifies the target texture. Must be GL_TEXTURE_2D or
**          GL_TEXTURE_CUBE_MAP_xxx_OES per GL_OES_texture_cube_map extension.
**
**      Level
**          Specifies the level-of-detail number. Level 0 is the base image
**          level. Level n is the nth mipmap reduction image. Must be greater
**          or equal 0.
**
**      XOffset
**      YOffset
**          Specifies a texel offset within the texture array.
**
**      Width
**      Height
**          Specifies the size of the texture subimage.
**
**      Format
**          Specifies the format of the pixel data. Must be same as
**          internalformat. The following symbolic values are accepted:
**          GL_ALPHA, GL_RGB, GL_RGBA, GL_LUMINANCE, and GL_LUMINANCE_ALPHA.
**
**      Type
**          Specifies the data type of the pixel data. The following symbolic
**          values are accepted: GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT_5_6_5,
**          GL_UNSIGNED_SHORT_4_4_4_4, and GL_UNSIGNED_SHORT_5_5_5_1.
**
**      Pixels
**          Specifies a pointer to the image data in memory.
**
**  OUTPUT:
**
**      Nothing.
*/

GL_API void GL_APIENTRY glTexSubImage2D(
    GLenum Target,
    GLint Level,
    GLint XOffset,
    GLint YOffset,
    GLsizei Width,
    GLsizei Height,
    GLenum Format,
    GLenum Type,
    const GLvoid* Pixels
    )
{
    glmENTER9(glmARGENUM, Target, glmARGINT, Level, glmARGINT, XOffset,
              glmARGINT, YOffset, glmARGINT, Width, glmARGINT, Height,
              glmARGENUM, Format, glmARGENUM, Type, glmARGPTR, Pixels)
    {
        gceTEXTURE_FACE face;
        glsTEXTURESAMPLER_PTR sampler;
        glsTEXTUREWRAPPER_PTR texture;
        gceSURF_FORMAT srcFormat, dstFormat;
        gceSTATUS status;
        gcoSURF map;
        gctUINT height, width;
        gctINT stride;

        glmPROFILE(context, GLES1_TEXSUBIMAGE2D, 0);
        /* Get a shortcut to the active sampler. */
        sampler = context->texture.activeSampler;

        /* Determine the target and the texture. */
        switch (Target)
        {
        case GL_TEXTURE_2D:
            texture = sampler->bindings[glvTEXTURE2D];
            face    = gcvFACE_NONE;
            break;

        case GL_TEXTURE_CUBE_MAP_POSITIVE_X_OES:
            if (Width != Height)
            {
                glmERROR(GL_INVALID_VALUE);
                goto OnError;
            }
            texture = sampler->bindings[glvCUBEMAP];
            face    = gcvFACE_POSITIVE_X;
            break;

        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X_OES:
            if (Width != Height)
            {
                glmERROR(GL_INVALID_VALUE);
                goto OnError;
            }
            texture = sampler->bindings[glvCUBEMAP];
            face    = gcvFACE_NEGATIVE_X;
            break;

        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y_OES:
            if (Width != Height)
            {
                glmERROR(GL_INVALID_VALUE);
                goto OnError;
            }
            texture = sampler->bindings[glvCUBEMAP];
            face    = gcvFACE_POSITIVE_Y;
            break;

        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_OES:
            if (Width != Height)
            {
                glmERROR(GL_INVALID_VALUE);
                goto OnError;
            }
            texture = sampler->bindings[glvCUBEMAP];
            face    = gcvFACE_NEGATIVE_Y;
            break;

        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z_OES:
            if (Width != Height)
            {
                glmERROR(GL_INVALID_VALUE);
                goto OnError;
            }
            texture = sampler->bindings[glvCUBEMAP];
            face    = gcvFACE_POSITIVE_Z;
            break;

        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_OES:
            if (Width != Height)
            {
                glmERROR(GL_INVALID_VALUE);
                goto OnError;
            }
            texture = sampler->bindings[glvCUBEMAP];
            face    = gcvFACE_NEGATIVE_Z;
            break;

        default:
            glmERROR(GL_INVALID_ENUM);
            goto OnError;
        }

        /* Texture has to be something. */
        gcmASSERT(texture != gcvNULL);

        /* Validate arguments. */
        if (!_validateFormat(Format) || !_validateType(Type))
        {
            glmERROR(GL_INVALID_ENUM);
            goto OnError;
        }

        if ((Pixels == gcvNULL) ||
            (Level < 0) ||
            (XOffset < 0) || (YOffset < 0) ||
            /* Valid Width and Height. */
            (Width < 0) || (Height < 0)  ||
            (Width  > (GLsizei)context->maxTextureWidth)  ||
            (Height > (GLsizei)context->maxTextureHeight) ||
            /* Valid level. */
            (Level  > (GLint)gcoMATH_Ceiling(
                                gcoMATH_Log2(
                                    (gctFLOAT)context->maxTextureWidth)))
            )
        {
            glmERROR(GL_INVALID_VALUE);
            goto OnError;
        }

        if (!_getFormat(Format, Type, &srcFormat))
        {
            glmERROR(GL_INVALID_OPERATION);
            goto OnError;
        }

        /* Get closest supported destination. */
        status = gcoTEXTURE_GetClosestFormat(
            context->hal, srcFormat, &dstFormat
            );

        if (gcmIS_ERROR(status))
        {
            glmERROR(GL_INVALID_VALUE);
            goto OnError;
        }

        /* Make sure the texture is not empty. */
        if (texture->object == gcvNULL)
        {
            glmERROR(GL_INVALID_OPERATION);
            goto OnError;
        }

        /* Get mip map for specified level. */
        status = gcoTEXTURE_GetMipMap(texture->object, Level, &map);

        if (gcmIS_ERROR(status))
        {
            glmERROR(GL_INVALID_OPERATION);
            goto OnError;
        }

        /* Get mip map height. */
        status = gcoSURF_GetSize(map, &width, &height, gcvNULL);

        if (gcmIS_ERROR(status))
        {
            glmERROR(GL_INVALID_OPERATION);
            goto OnError;
        }

        /* Check the size validation. */
        if ((XOffset + Width  > (GLint) width) ||
            (YOffset + Height > (GLint) height))
        {
            glmERROR(GL_INVALID_VALUE);
            goto OnError;
        }

        /* Get Stride of source texture */
        stride = 0;
        _GetSourceStride(
            Format, Type, Width, context->unpackAlignment, &stride
            );

        /* Upload the texture. */
        status = gcoTEXTURE_UploadSub(
            texture->object,
            Level, face,
            XOffset, YOffset,
            Width, Height, 0,
            Pixels, stride,
            srcFormat,
            gcvSURF_COLOR_SPACE_LINEAR,
            gcvINVALID_ADDRESS);

        if (gcmIS_ERROR(status))
        {
            glmERROR(GL_INVALID_OPERATION);
            goto OnError;
        }

        /* Regenerate mipmap if necessary. */
        if ((texture->genMipmap) && (Level == 0))
        {
            gcmERR_BREAK(glfGenerateMipMaps(
                context, texture, dstFormat, 0, width, height, face
                ));
        }

        /* Mark texture as dirty. */
        texture->dirty = gcvTRUE;

        if ((texture->image.source != gcvNULL) &&
            (texture->image.image != gcvNULL))
        {
            gcoSURF surface;
            gctSIZE_T offset = 0;

            if (gcmIS_SUCCESS(gcoTEXTURE_GetMipMapSlice(texture->object,
                                                        Level,
                                                        face,
                                                        &surface,
                                                        &offset)))
            {
                /* Update latest EGLImage sibling. */
                glfSetEGLImageSrc(texture->image.image, surface);
            }
        }

        gcmDUMP_API("${ES11 glTexSubImage2D 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X (0x%08X)",
            Target, Level, XOffset, YOffset, Width, Height, Format, Type, Pixels);
        gcmDUMP_API_DATA(Pixels, stride * Height);
        gcmDUMP_API("$}");

        /* Easy return on error. */
OnError:;
    }
    glmLEAVE();
}

gceSTATUS glfBlitCPU(
                     IN glsCONTEXT_PTR Context,
                     IN gcoTEXTURE     Texture,
                     IN GLint          Target,
                     IN GLint          Level,
                     IN GLint          Face,
                     IN gctUINT        Slice,
                     IN GLint          XOffset,
                     IN GLint          YOffset,
                     IN GLint          X,
                     IN GLint          Y,
                     IN GLsizei        Width,
                     IN GLsizei        Height
                     )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsSURF_BLIT_ARGS  blitArgs;
    gcoSURF texSurf;
    gctUINT index;


    gcmONERROR(gcoTEXTURE_GetMipMap(Texture, Level, &texSurf));
    gcoOS_ZeroMemory(&blitArgs, sizeof(blitArgs));

    gcmPRINT("glfBlitCPU is called");
    /* Convert face into index. */
    switch (Face)
    {
    case gcvFACE_NONE:
        /* Use slice for volume maps or 2D arrays. */
        index = Slice;
        break;
    case gcvFACE_POSITIVE_X:
    case gcvFACE_NEGATIVE_X:
    case gcvFACE_POSITIVE_Y:
    case gcvFACE_NEGATIVE_Y:
    case gcvFACE_POSITIVE_Z:
    case gcvFACE_NEGATIVE_Z:
        /* Get index from Face. */
        index = Face - gcvFACE_POSITIVE_X;
        break;
    default:
        index = 0;
        break;
    }

    blitArgs.srcX               = X;
    blitArgs.srcY               = Y;
    blitArgs.srcWidth           = Width;
    blitArgs.srcHeight          = Height;
    blitArgs.srcDepth           = 1;
    blitArgs.srcSurface         = Context->draw;
    blitArgs.dstX               = XOffset;
    blitArgs.dstY               = YOffset;
    blitArgs.dstZ               = (gctINT)index;
    blitArgs.dstWidth           = Width;
    blitArgs.dstHeight          = Height;
    blitArgs.dstDepth           = 1;
    blitArgs.dstSurface         = texSurf;
    blitArgs.xReverse           = gcvFALSE;
    blitArgs.yReverse           = gcvTRUE;
    blitArgs.scissorTest        = gcvFALSE;

    status = gcoSURF_BlitCPU(&blitArgs);

OnError:

    return status;
}

/*******************************************************************************
**
**  glCopyTexImage2D
**
**  glCopyTexImage2D defines a two-dimensional texture image with pixels from
**  the color buffer.
**
**  The screen-aligned pixel rectangle with lower left corner at (x, y) and with
**  a width of width + 2border and a height of height + 2border defines the
**  texture array at the mipmap level specified by level. InternalFormat
**  specifies the color components of the texture.
**
**  The red, green, blue, and alpha components of each pixel that is read are
**  converted to an internal fixed-point or floating-point format with
**  unspecified precision. The conversion maps the largest representable
**  component value to 1.0, and component value 0 to 0.0. The values are then
**  converted to the texture's internal format for storage in the texel array.
**
**  InternalFormat must be chosen such that color buffer components can be
**  dropped during conversion to the internal format, but new components cannot
**  be added. For example, an RGB color buffer can be used to create LUMINANCE
**  or RGB textures, but not ALPHA, LUMINANCE_ALPHA or RGBA textures.
**
**  Pixel ordering is such that lower x and y screen coordinates correspond to
**  lower s and t texture coordinates.
**
**  If any of the pixels within the specified rectangle of the color buffer are
**  outside the window associated with the current rendering context, then the
**  values obtained for those pixels are undefined.
**
**  INPUT:
**
**      Target
**          Specifies the target texture. Must be GL_TEXTURE_2D or
**          GL_TEXTURE_CUBE_MAP_xxx_OES per GL_OES_texture_cube_map extension.
**
**      Level
**          Specifies the level-of-detail number. Level 0 is the base image
**          level. Level n is the nth mipmap reduction image. Must be greater
**          or equal 0.
**
**      InternalFormat
**          Specifies the color components of the texture. The following
**          symbolic values are accepted: GL_ALPHA, GL_RGB, GL_RGBA,
**          GL_LUMINANCE, and GL_LUMINANCE_ALPHA.
**
**      X
**      Y
**          Specify the window coordinates of the lower left corner of the
**          rectangular region of pixels to be copied.
**
**      Width
**      Height
**          Specifies the size of the texture image. Must be 0 or
**          2**n + 2*border for some integer n.
**
**      Border
**          Specifies the width of the border. Must be 0.
**
**  OUTPUT:
**
**      Nothing.
*/

GL_API void GL_APIENTRY glCopyTexImage2D(
    GLenum Target,
    GLint Level,
    GLenum InternalFormat,
    GLint X,
    GLint Y,
    GLsizei Width,
    GLsizei Height,
    GLint Border
    )
{
    glmENTER7(glmARGENUM, Target, glmARGINT, Level, glmARGENUM, InternalFormat,
              glmARGINT, X, glmARGINT, Y, glmARGINT, Width, glmARGINT, Height)
    {
        gceSTATUS status;
        gctINT faces;
        gceTEXTURE_FACE face;
        glsTEXTURESAMPLER_PTR sampler;
        glsTEXTUREWRAPPER_PTR texture;
        gceSURF_FORMAT format, textureFormat;
        gcsSURF_VIEW texView = {gcvNULL, 0, 1};

        gctINT sy = Y;
        gctINT sx = X;

        gcoSURF shareObj;

        gcmDUMP_API("${ES11 glCopyTexImage2D 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X}",
            Target, Level, InternalFormat, X, Y, Width, Height, Border);

        glmPROFILE(context, GLES1_COPYTEXIMAGE2D, 0);
        /* Get a shortcut to the active sampler. */
        sampler = context->texture.activeSampler;

        /* Determine the target and the texture. */
        switch (Target)
        {
        case GL_TEXTURE_2D:
            texture = sampler->bindings[glvTEXTURE2D];
            faces   = 0;
            face    = gcvFACE_NONE;
            texView.firstSlice = 0;
            break;

        case GL_TEXTURE_CUBE_MAP_POSITIVE_X_OES:
            if (Width != Height)
            {
                glmERROR(GL_INVALID_VALUE);
                goto OnError;
            }
            texture = sampler->bindings[glvCUBEMAP];
            faces   = 6;
            face    = gcvFACE_POSITIVE_X;
            texView.firstSlice = 0;
            break;

        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X_OES:
            if (Width != Height)
            {
                glmERROR(GL_INVALID_VALUE);
                goto OnError;
            }
            texture = sampler->bindings[glvCUBEMAP];
            faces   = 6;
            face    = gcvFACE_NEGATIVE_X;
            texView.firstSlice = 1;
            break;

        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y_OES:
            if (Width != Height)
            {
                glmERROR(GL_INVALID_VALUE);
                goto OnError;
            }
            texture = sampler->bindings[glvCUBEMAP];
            faces   = 6;
            face    = gcvFACE_POSITIVE_Y;
            texView.firstSlice = 2;
            break;

        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_OES:
            if (Width != Height)
            {
                glmERROR(GL_INVALID_VALUE);
                goto OnError;
            }
            texture = sampler->bindings[glvCUBEMAP];
            faces   = 6;
            face    = gcvFACE_NEGATIVE_Y;
            texView.firstSlice = 3;
            break;

        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z_OES:
            if (Width != Height)
            {
                glmERROR(GL_INVALID_VALUE);
                goto OnError;
            }
            texture = sampler->bindings[glvCUBEMAP];
            faces   = 6;
            face    = gcvFACE_POSITIVE_Z;
            texView.firstSlice = 4;
            break;

        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_OES:
            if (Width != Height)
            {
                glmERROR(GL_INVALID_VALUE);
                goto OnError;
            }
            texture = sampler->bindings[glvCUBEMAP];
            faces   = 6;
            face    = gcvFACE_NEGATIVE_Z;
            texView.firstSlice = 5;
            break;

        default:
            glmERROR(GL_INVALID_ENUM);
            goto OnError;
        }

        /* Texture has to be something. */
        gcmASSERT(texture != gcvNULL);

        if ((Level < 0) || (Border != 0) ||
            /* Valid Width and Height. */
            (Width < 0) || (Height < 0)  ||
            (Width  > (GLsizei)context->maxTextureWidth)  ||
            (Height > (GLsizei)context->maxTextureHeight) ||
            /* Valid level. */
            (Level  > (GLint)gcoMATH_Ceiling(
                                gcoMATH_Log2
                                    ((gctFLOAT)context->maxTextureWidth)))
            )
        {
            glmERROR(GL_INVALID_VALUE);
            goto OnError;
        }

        /* Get frame buffer format. */
        gcmVERIFY_OK(gcoSURF_GetFormat(context->draw, gcvNULL, &format));

        switch (InternalFormat)
        {
        case GL_ALPHA:
            textureFormat = gcvSURF_A8;
            break;

        case GL_RGB:
            textureFormat = gcvSURF_X8R8G8B8;
            break;

        case GL_RGBA:
            textureFormat = gcvSURF_A8R8G8B8;
            break;

        case GL_LUMINANCE:
            textureFormat = gcvSURF_L8;
            break;

        case GL_LUMINANCE_ALPHA:
            textureFormat = gcvSURF_A8L8;
            break;

        default:
            /* Unknown format. */
            glmERROR(GL_INVALID_ENUM);
            goto OnError;
        }

        /* Get closest texture format hardware supports. */
        status = gcoTEXTURE_GetClosestFormat(
            context->hal, textureFormat, &textureFormat
            );

        if (gcmIS_ERROR(status))
        {
            /* Invalid format. */
            glmERROR(GL_INVALID_OPERATION);
            goto OnError;
        }

        /* Reset the texture if planar object is defined. */
        if ((texture->direct.source != gcvNULL)
        ||  (texture->image.source != gcvNULL)
        )
        {
            /* Destroy the texture. */
            gcmVERIFY_OK(_ResetTextureWrapper(context, texture));
        }

        /* Test if we need to create a new gcoTEXTURE object. */
        if (texture->object == gcvNULL)
        {
            /* Construct the gcoTEXTURE object. */
            status = gcoTEXTURE_ConstructEx(context->hal, _HALtexType[texture->targetType], &texture->object);

            if (gcmIS_ERROR(status))
            {
                /* Error. */
                glmERROR(GL_OUT_OF_MEMORY);
                goto OnError;
            }
        }

        /* Add the mipmap. If it already exists, the call will be ignored. */
        status = gcoTEXTURE_AddMipMap(
            texture->object,
            Level,
            InternalFormat,
            textureFormat,
            Width, Height, 0,
            faces,
            gcvPOOL_DEFAULT,
            &shareObj
            );

        if (gcmIS_ERROR(status))
        {
            /* Destroy the texture. */
            gcmVERIFY_OK(_ResetTextureWrapper(context, texture));

            glmERROR(GL_OUT_OF_MEMORY);
            goto OnError;
        }

        /* Set shared lock.*/
        gcmVERIFY_OK(gcoSURF_SetSharedLock(shareObj, context->texture.textureList->sharedLock));

        /* Update frame buffer. */
        gcmERR_BREAK(glfUpdateFrameBuffer(context));

        /* Check buffer preserve. */
        gcmERR_BREAK(glfUpdateBufferPreserve(context));

        /* Make sure the texture is renderable. */
        if (gcmIS_SUCCESS(gcoTEXTURE_GetMipMapSlice(texture->object, Level, face, &texView.surf, gcvNULL)) &&
            gcoSURF_IsRenderable(texView.surf) == gcvSTATUS_OK
            )
        {
            gcsSURF_VIEW srcView = {gcvNULL, 0, 1};
            gscSURF_BLITDRAW_BLIT blitArgs = {0};

            srcView.surf = context->frameBuffer
                         ? glfGetFramebufferSurface(&context->frameBuffer->color)
                         : context->draw;

            gcoOS_ZeroMemory(&blitArgs, gcmSIZEOF(blitArgs));
            blitArgs.srcRect.left = X;
            blitArgs.srcRect.top = Y;
            blitArgs.srcRect.right = X + Width;
            blitArgs.srcRect.bottom = Y + Height;
            blitArgs.dstRect.left = 0;
            blitArgs.dstRect.top = 0;
            blitArgs.dstRect.right = Width;
            blitArgs.dstRect.bottom  = Height;
            blitArgs.filterMode = gcvTEXTURE_POINT;
            blitArgs.yReverse = context->drawYInverted;
            if (context->drawYInverted)
            {
                blitArgs.srcRect.top = (context->drawHeight - (Y + Height));
                blitArgs.srcRect.bottom = (context->drawHeight - Y);
            }
            status = gcoSURF_DrawBlit(&srcView, &texView, &blitArgs);
        }
        else
        {
            /* run other path.*/
            status = gcvSTATUS_NOT_SUPPORTED;
        }

        if (gcmIS_ERROR(status))
        {

            /* Resolve the rectangle to the temporary surface. */
            status = glfResolveDrawToTempBitmap(context, X, Y, Width, Height);

            /* if draw to tempBitMap failed, go to BlitCPU. */

            if (gcmIS_ERROR(status))
            {
                if(context->drawYInverted)
                {
                    sy = context->drawHeight - Y - Height;
                }

                status = glfBlitCPU(
                    context,
                    texture->object,
                    Target,
                    Level,
                    face,
                    0,
                    0, 0,
                    sx, sy,
                    Width, Height);
            }
            else
            {
                /* Invalidate the cache. */
                gcmVERIFY_OK(
                    gcoSURF_CPUCacheOperation(context->tempBitmap,
                    gcvCACHE_INVALIDATE));

                /* Upload the texture. */
                status = gcoTEXTURE_Upload(
                    texture->object,
                    Level,
                    face,
                    Width, Height, 0,
                    context->tempLastLine,
                    context->tempStride,
                    context->tempFormat,
                    gcvSURF_COLOR_SPACE_LINEAR);
            }
        }

        if (gcmIS_ERROR(status))
        {
            glmERROR(GL_INVALID_OPERATION);
            goto OnError;
        }

        /* Mark texture as dirty. */
        texture->dirty = gcvTRUE;

        if ((texture->image.source != gcvNULL) &&
            (texture->image.image != gcvNULL))
        {
            gcoSURF surface;
            gctSIZE_T offset = 0;

            if (gcmIS_SUCCESS(gcoTEXTURE_GetMipMapSlice(texture->object,
                                                        Level,
                                                        face,
                                                        &surface,
                                                        &offset)))
            {
                /* Update latest EGLImage sibling. */
                glfSetEGLImageSrc(texture->image.image, surface);
            }
        }

        /* Level 0 is special. */
        if (Level == 0)
        {
            GLint tValue;
            /* Invalidate normalized crop rectangle. */
            texture->dirtyCropRect = GL_TRUE;

            /* Set texture parameters. */
            texture->width  = Width;
            texture->height = Height;

            /* Determine the number of mipmaps. */
            tValue = glfGetMaxLOD(texture->width, texture->height);
            texture->maxLevel = gcmMIN(tValue, texture->maxLevel);

            /* Update texture format. */
            _SetTextureWrapperFormat(context, texture, InternalFormat);
        }

        /* Easy return on error. */
OnError:;
    }
    glmLEAVE();
}


/*******************************************************************************
**
**  glCopyTexSubImage2D
**
**  glCopyTexSubImage2D replaces a rectangular portion of a two-dimensional
**  texture image with pixels from the color buffer.
**
**  The screen-aligned pixel rectangle with lower left corner at ( x, y) and
**  with width width and height height replaces the portion of the texture array
**  with x indices xoffset through xoffset + width - 1, inclusive, and y indices
**  yoffset through yoffset + height - 1, inclusive, at the mipmap level
**  specified by level.
**
**  The pixels in the rectangle are processed the same way as with
**  glCopyTexImage2D.
**
**  glCopyTexSubImage2D requires that the internal format of the currently
**  bound texture is such that color buffer components can be dropped during
**  conversion to the internal format, but new components cannot be added.
**  For example, an RGB color buffer can be used to create LUMINANCE or RGB
**  textures, but not ALPHA, LUMINANCE_ALPHA or RGBA textures.
**
**  The destination rectangle in the texture array may not include any texels
**  outside the texture array as it was originally specified. It is not an error
**  to specify a subtexture with zero width or height, but such a specification
**  has no effect.
**
**  If any of the pixels within the specified rectangle of the current color
**  buffer are outside the read window associated with the current rendering
**  context, then the values obtained for those pixels are undefined.
**
**  No change is made to the internalformat, width, height, or border
**  parameters of the specified texture array or to texel values outside
**  the specified subregion.
**
**  INPUT:
**
**      Target
**          Specifies the target texture. Must be GL_TEXTURE_2D or
**          GL_TEXTURE_CUBE_MAP_xxx_OES per GL_OES_texture_cube_map extension.
**
**      Level
**          Specifies the level-of-detail number. Level 0 is the base image
**          level. Level n is the nth mipmap reduction image. Must be greater
**          or equal 0.
**
**      XOffset
**      YOffset
**          Specifies a texel offset within the texture array.
**
**      X
**      Y
**          Specify the window coordinates of the lower left corner of the
**          rectangular region of pixels to be copied.
**
**      Width
**      Height
**          Specifies the size of the texture subimage.
**
**  OUTPUT:
**
**      Nothing.
*/

GL_API void GL_APIENTRY glCopyTexSubImage2D(
    GLenum Target,
    GLint Level,
    GLint XOffset,
    GLint YOffset,
    GLint X,
    GLint Y,
    GLsizei Width,
    GLsizei Height
    )
{
    glmENTER8(glmARGENUM, Target, glmARGINT, Level, glmARGINT, XOffset,
              glmARGINT, YOffset, glmARGINT, X, glmARGINT, Y, glmARGINT, Width,
              glmARGINT, Height)
    {
        gceSTATUS status;
        gceTEXTURE_FACE face;
        gctUINT mapHeight, mapWidth;
        glsTEXTURESAMPLER_PTR sampler;
        glsTEXTUREWRAPPER_PTR texture;
        gcoSURF map;
        gcsSURF_VIEW texView = {gcvNULL, 0, 1};

        gctINT sx = X;
        gctINT sy = Y;

        gcmDUMP_API("${ES11 glCopyTexSubImage2D 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X}",
            Target, Level, XOffset, YOffset, X, Y, Width, Height);

        glmPROFILE(context, GLES1_COPYTEXSUBIMAGE2D, 0);
        /* Get a shortcut to the active sampler. */
        sampler = context->texture.activeSampler;

        /* Determine the target and the texture. */
        switch (Target)
        {
        case GL_TEXTURE_2D:
            texture = sampler->bindings[glvTEXTURE2D];
            face    = gcvFACE_NONE;
            texView.firstSlice = 0;
            break;

        case GL_TEXTURE_CUBE_MAP_POSITIVE_X_OES:
            if (Width != Height)
            {
                glmERROR(GL_INVALID_VALUE);
                goto OnError;
            }
            texture = sampler->bindings[glvCUBEMAP];
            face    = gcvFACE_POSITIVE_X;
            texView.firstSlice = 0;
            break;

        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X_OES:
            if (Width != Height)
            {
                glmERROR(GL_INVALID_VALUE);
                goto OnError;
            }
            texture = sampler->bindings[glvCUBEMAP];
            face    = gcvFACE_NEGATIVE_X;
            texView.firstSlice = 1;
            break;

        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y_OES:
            if (Width != Height)
            {
                glmERROR(GL_INVALID_VALUE);
                goto OnError;
            }
            texture = sampler->bindings[glvCUBEMAP];
            face    = gcvFACE_POSITIVE_Y;
            texView.firstSlice = 2;
            break;

        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_OES:
            if (Width != Height)
            {
                glmERROR(GL_INVALID_VALUE);
                goto OnError;
            }
            texture = sampler->bindings[glvCUBEMAP];
            face    = gcvFACE_NEGATIVE_Y;
            texView.firstSlice = 3;
            break;

        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z_OES:
            if (Width != Height)
            {
                glmERROR(GL_INVALID_VALUE);
                goto OnError;
            }
            texture = sampler->bindings[glvCUBEMAP];
            face    = gcvFACE_POSITIVE_Z;
            texView.firstSlice = 4;
            break;

        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_OES:
            if (Width != Height)
            {
                glmERROR(GL_INVALID_VALUE);
                goto OnError;
            }
            texture = sampler->bindings[glvCUBEMAP];
            face    = gcvFACE_NEGATIVE_Z;
            texView.firstSlice = 5;
            break;

        default:
            glmERROR(GL_INVALID_ENUM);
            goto OnError;
        }

        /* Texture has to be something. */
        gcmASSERT(texture != gcvNULL);

        /* Verify arguments. */
        if (/* Valid Width and Height. */
            (Width < 0) || (Height < 0) ||
            (Width  > (GLsizei)context->maxTextureWidth)  ||
            (Height > (GLsizei)context->maxTextureHeight) ||
            (XOffset < 0) || (YOffset < 0) ||
            /* Valid level. */
            (Level < 0) ||
            (Level  > (GLint)gcoMATH_Ceiling(
                                gcoMATH_Log2
                                    ((gctFLOAT)context->maxTextureWidth)))
            )
        {
            glmERROR(GL_INVALID_VALUE);
            goto OnError;
        }

        /* Texture object has to exist. */
        if (texture->object == gcvNULL)
        {
            /* No bound texture object. */
            glmERROR(GL_INVALID_OPERATION);
            goto OnError;
        }

        /* Get requested mip map surface. */
        status = gcoTEXTURE_GetMipMap(texture->object, Level, &map);

        if (gcmIS_ERROR(status))
        {
            /* Mip map has to exist. */
            glmERROR(GL_INVALID_OPERATION);
            goto OnError;
        }

        /* Get size of mip map. */
        status = gcoSURF_GetSize(map, &mapWidth, &mapHeight, gcvNULL);
        if (gcmIS_ERROR(status))
        {
            glmERROR(GL_INVALID_OPERATION);
            goto OnError;
        }

        /* Check for value's validation. */
        if (((XOffset + Width)  > (GLint) mapWidth) ||
            ((YOffset + Height) > (GLint) mapHeight))
        {
            glmERROR(GL_INVALID_VALUE);
            goto OnError;
        }

        /* Update frame buffer. */
        gcmERR_BREAK(glfUpdateFrameBuffer(context));

        /* Check buffer preserve. */
        gcmERR_BREAK(glfUpdateBufferPreserve(context));

        /* Make sure the texture is renderable. */
        if (gcmIS_SUCCESS(gcoTEXTURE_GetMipMapSlice(texture->object, Level, face, &texView.surf, gcvNULL)) &&
            gcoSURF_IsRenderable(texView.surf) == gcvSTATUS_OK
            )
        {
            gcsSURF_VIEW srcView = {gcvNULL, 0, 1};
            gscSURF_BLITDRAW_BLIT blitArgs = {0};

            srcView.surf = context->frameBuffer
                         ? glfGetFramebufferSurface(&context->frameBuffer->color)
                         : context->draw;

            gcoOS_ZeroMemory(&blitArgs, sizeof(blitArgs));
            blitArgs.srcRect.left = X;
            blitArgs.srcRect.top = Y;
            blitArgs.srcRect.right = X + Width;
            blitArgs.srcRect.bottom = Y + Height;
            blitArgs.dstRect.left = XOffset;
            blitArgs.dstRect.top = YOffset;
            blitArgs.dstRect.right = XOffset + Width;
            blitArgs.dstRect.bottom  = YOffset + Height;
            blitArgs.filterMode = gcvTEXTURE_POINT;
            blitArgs.yReverse = context->drawYInverted;
            if (context->drawYInverted)
            {
                blitArgs.srcRect.top = (context->drawHeight - (Y + Height));
                blitArgs.srcRect.bottom = (context->drawHeight - Y);
            }

            status = gcoSURF_DrawBlit(&srcView, &texView, &blitArgs);
        }
        else
        {
            /* run other path.*/
            status = gcvSTATUS_NOT_SUPPORTED;
        }

        if (gcmIS_ERROR(status))
        {

            /* Resolve the rectangle to the temporary surface. */
            status = glfResolveDrawToTempBitmap(context, X, Y, Width, Height);

            /* if draw to tempBitMap failed, go to BlitCPU. */
            if (gcmIS_ERROR(status))
            {
                if(context->drawYInverted)
                {
                    sy = context->drawHeight - Y - Height;
                }
                status = glfBlitCPU(
                    context,
                    texture->object,
                    Target,
                    Level,
                    face,
                    0,
                    XOffset, YOffset,
                    sx, sy,
                    Width, Height);
            }
            else
            {
                /* Invalidate the cache. */
                gcmVERIFY_OK(
                    gcoSURF_CPUCacheOperation(context->tempBitmap,
                    gcvCACHE_INVALIDATE));

                /* Upload the texture. */
                status = gcoTEXTURE_UploadSub(
                    texture->object,
                    Level,
                    face,
                    XOffset, YOffset,
                    Width, Height, 0,
                    context->tempLastLine,
                    context->tempStride,
                    context->tempFormat,
                    gcvSURF_COLOR_SPACE_LINEAR,
                    gcvINVALID_ADDRESS);
            }
        }

        if (gcmIS_ERROR(status))
        {
            glmERROR(GL_INVALID_OPERATION);
            goto OnError;
        }

        /* Mark texture as dirty. */
        texture->dirty = gcvTRUE;

        if ((texture->image.source != gcvNULL) &&
            (texture->image.image != gcvNULL))
        {
            gcoSURF surface;
            gctSIZE_T offset = 0;

            if (gcmIS_SUCCESS(gcoTEXTURE_GetMipMapSlice(texture->object,
                                                        Level,
                                                        face,
                                                        &surface,
                                                        &offset)))
            {
                /* Update latest EGLImage sibling. */
                glfSetEGLImageSrc(texture->image.image, surface);
            }
        }

        /* Easy return on error. */
OnError:;
    }
    glmLEAVE();
}

/*******************************************************************************
**
**  glCompressedTexImage2D
**
**  glCompressedTexImage2D defines a two-dimensional texture image in compressed
**  format.
**
**  The supported compressed formats are paletted textures. The layout of the
**  compressed image is a palette followed by multiple mip-levels of texture
**  indices used for lookup into the palette. The palette format can be one of
**  R5_G6_B5, RGBA4, RGB5_A1, RGB8, or RGBA8. The texture indices can have a
**  resolution of 4 or 8 bits. As a result, the number of palette entries is
**  either 16 or 256.
**
**  INPUT:
**
**      Target
**          Specifies the target texture. Must be GL_TEXTURE_2D.
**
**      Level
**          Specifies the level-of-detail number. Level 0 is the base image
**          level. Level n is the nth mipmap reduction image. Must be greater
**          or equal 0.
**
**      InternalFormat
**          Specifies the color components in the texture. The following
**          symbolic constants are accepted:
**              GL_PALETTE4_RGB8_OES
**              GL_PALETTE4_RGBA8_OES
**              GL_PALETTE4_R5_G6_B5_OES
**              GL_PALETTE4_RGBA4_OES
**              GL_PALETTE4_RGB5_A1_OES
**              GL_PALETTE8_RGB8_OES
**              GL_PALETTE8_RGBA8_OES
**              GL_PALETTE8_R5_G6_B5_OES
**              GL_PALETTE8_RGBA4_OES
**              GL_PALETTE8_RGB5_A1_OES
**          also:
**              GL_COMPRESSED_RGB_S3TC_DXT1_EXT
**              GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
**              GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
**              GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
**              GL_ETC1_RGB8_OES
**
**      Width
**          Specifies the width of the texture image. Must be 2**n for some
**          integer n. All implementations support texture images that are
**          at least 64 texels wide.
**
**      Height
**          Specifies the height of the texture image. Must be 2**m for some
**          integer m. All implementations support texture images that are
**          at least 64 texels high.
**
**      Border
**          Specifies the width of the border. Must be 0.
**
**      ImageSize
**          Specifies the size of the compressed image data in bytes.
**
**      Data
**          Specifies a pointer to the compressed image data in memory.
**
**  OUTPUT:
**
**      Nothing.
*/

GL_API void GL_APIENTRY glCompressedTexImage2D(
    GLenum Target,
    GLint Level,
    GLenum InternalFormat,
    GLsizei Width,
    GLsizei Height,
    GLint Border,
    GLsizei ImageSize,
    const GLvoid* Data
    )
{
    glmENTER7(glmARGENUM, Target, glmARGINT, Level, glmARGENUM, InternalFormat,
              glmARGINT, Width, glmARGINT, Height, glmARGUINT, ImageSize,
              glmARGPTR, Data)
    {
        gceSTATUS status;
        GLubyte * pixels, p;
        GLvoid * buffer;
        GLubyte * b;
        GLint i, shift;
        GLint pixelCount;
        GLint paletteSize;
        GLint compressedSize;
        GLint uncompressedSize;
        glsCOMPRESSEDTEXTURE_PTR formatInfo;
        gceSURF_FORMAT srcFormat, dstFormat;

        glmPROFILE(context, GLES1_COMPRESSEDTEXIMAGE2D, 0);

        /* Validate the arguments. */
        if (Target != GL_TEXTURE_2D)
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        if (!_getFormat(GL_RGB, InternalFormat, &srcFormat))
        {
            glmERROR(GL_INVALID_ENUM);
            goto OnError;
        }

        /* Get closest supported destination. */
        status = gcoTEXTURE_GetClosestFormat(
            context->hal, srcFormat, &dstFormat
            );

        if (gcmIS_ERROR(status))
        {
            glmERROR(GL_INVALID_VALUE);
            goto OnError;
        }

        if ((InternalFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT)
        ||  (InternalFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
        ||  (InternalFormat == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT)
        ||  (InternalFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
        )
        {
            if ((dstFormat < gcvSURF_DXT1) || (dstFormat > gcvSURF_DXT5))
            {
                gceSURF_FORMAT imageFormat = gcvSURF_UNKNOWN;
                gctUINT32 rgbFormat = (InternalFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT) ?
                    GL_RGB : GL_RGBA;

                /* Decompress DXT texture since hardware doesn't support
                ** it. */
                pixels = _DecompressDXT(Width, Height,
                                         ImageSize, Data,
                                         InternalFormat,
                                         imageFormat = dstFormat);

                if (pixels == gcvNULL)
                {
                    /* Could not decompress the DXT texture. */
                    gcmFOOTER_NO();
                    return;
                }

                glTexImage2D(
                    Target, Level, rgbFormat, Width, Height,
                    Border, rgbFormat, imageFormat, pixels
                    );
            }
            else
            {
                glTexImage2D(
                    Target, Level, GL_RGBA,  Width, Height,
                    Border, GL_RGBA, InternalFormat, Data
                    );
            }
            break;
        }

        if (InternalFormat == GL_ETC1_RGB8_OES)
        {
            if (dstFormat != gcvSURF_ETC1)
            {
                gctUINT32 imageType = 0;

                /* Decompress ETC1 texture since hardware doesn't support
                ** it. */
                pixels = _DecompressETC1(Width, Height,
                                         ImageSize, Data,
                                         &imageType);

                if (pixels == gcvNULL)
                {
                    /* Could not decompress the ETC1 texture. */
                    gcmFOOTER_NO();
                    return;
                }

                glTexImage2D(
                    Target, Level, GL_RGB, Width, Height,
                    Border, GL_RGB, imageType, pixels
                    );
            }
            else
            {
                glTexImage2D(
                    Target, Level, GL_RGB, Width, Height,
                    Border, GL_RGB, InternalFormat, Data
                    );
            }
            break;
        }


        /* Decode the paletted texture format. */
        formatInfo = _GetCompressedPalettedTextureDetails(InternalFormat);

        if (formatInfo == gcvNULL)
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        if ((Level < 0) || (Border != 0) ||
            /* Valid Width and Height. */
            (Width < 0) || (Height < 0)  ||
            (Width  > (GLsizei)context->maxTextureWidth)  ||
            (Height > (GLsizei)context->maxTextureHeight) ||
            /* Valid level. */
            (Level  > (GLint)gcoMATH_Ceiling(
                                 gcoMATH_Log2(
                                     (gctFLOAT)context->maxTextureWidth))) ||
            (Data   == gcvNULL)
            )
        {
            glmERROR(GL_INVALID_VALUE);
            break;
        }

        /* Calcuate the input texture sizes. */
        pixelCount     = Width * Height;
        paletteSize    = (1 << formatInfo->bits) * formatInfo->bytes;
        compressedSize = (pixelCount * formatInfo->bits + 7) / 8;

        /* Verify the size of the compressed texture. */
        if (ImageSize != (paletteSize + compressedSize))
        {
            glmERROR(GL_INVALID_VALUE);
            break;
        }

        /* Calculate the uncompressed texture size. */
        uncompressedSize = pixelCount * formatInfo->bytes;

        /* Allocate uncompressed pixel buffer. */
        status = gcoOS_Allocate(
            gcvNULL,
            uncompressedSize,
            (gctPOINTER *) &buffer
            );

        if (gcmIS_ERROR(status))
        {
            /* Out of memory. */
            glmERROR(GL_OUT_OF_MEMORY);
            break;
        }

        /* Point to pixel data. */
        shift  = (formatInfo->bits == 4) ? 4 : 0;
        pixels = (GLubyte *) Data + paletteSize;

        /* Upload each level of mipmap */
        b = buffer;

        for (i = 0; i < pixelCount; i++)
        {
            /* Fetch compressed pixel. */
            p = (formatInfo->bits == 4)
                ? (*pixels >> shift) & 0xF
                :  *pixels;

            /* Move to next compressed pixel. */
            if (shift == 4)
            {
                shift = 0;
            }
            else
            {
                pixels += 1;
                shift = (formatInfo->bits == 4) ? 4 : 0;
            }

            /* Copy data from palette to uncompressed pixel buffer. */
            gcoOS_MemCopy(
                b, (GLubyte *) Data + p * formatInfo->bytes, formatInfo->bytes
                );

            b += formatInfo->bytes;
        }

        /* Perform a texture load on the uncompressed data. */
        glTexImage2D(
            Target,
            Level,
            formatInfo->format,
            Width, Height,
            Border,
            formatInfo->format, formatInfo->type,
            buffer
            );

        /* Free the uncompressed pixel buffer. */
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, buffer));

        gcmDUMP_API("${ES11 glCompressedTexImage2D 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X (0x%08X)",
            Target, Level, InternalFormat, Width, Height, Border, ImageSize, Data);
        gcmDUMP_API_DATA(Data, compressedSize);
        gcmDUMP_API("$}");

OnError:;
    }
    glmLEAVE();
}


/*******************************************************************************
**
**  glCompressedTexSubImage2D
**
**  glCompressedTexSubImage2D redefines a contiguous subregion of an existing
**  two-dimensional compressed texture image. The texels referenced by pixels
**  replace the portion of the existing texture array with x indices xoffset
**  and xoffset + width - 1, inclusive, and y indices yoffset and
**  yoffset + height - 1, inclusive. This region may not include any texels
**  outside the range of the texture array as it was originally specified.
**  It is not an error to specify a subtexture with zero width or height,
**  but such a specification has no effect.
**
**  Currently, there is no supported compressed format for this function.
**
**  INPUT:
**
**      Target
**          Specifies the target texture. Must be GL_TEXTURE_2D.
**
**      Level
**          Specifies the level-of-detail number.
**
**      XOffset
**      YOffset
**          Specifies a texel offset within the texture array.
**
**      Width
**      Height
**          Specifies the size of the texture subimage.
**
**      Format
**          Specifies the format of the pixel data. Currently, there is no
**          supported format.
**
**      ImageSize
**          Specifies the size of the compressed image data in bytes.
**
**      Data
**          Specifies a pointer to the compressed image data in memory.
**
**  OUTPUT:
**
**      Nothing.
*/

GL_API void GL_APIENTRY glCompressedTexSubImage2D(
    GLenum Target,
    GLint Level,
    GLint XOffset,
    GLint YOffset,
    GLsizei Width,
    GLsizei Height,
    GLenum Format,
    GLsizei ImageSize,
    const GLvoid* Data
    )
{
    glmENTER9(glmARGENUM, Target, glmARGINT, Level,
              glmARGINT, XOffset, glmARGINT, YOffset,
              glmARGINT, Width, glmARGINT, Height,
              glmARGENUM, Format, glmARGUINT, ImageSize,
              glmARGPTR, Data)
    {
        glsCOMPRESSEDTEXTURE_PTR formatInfo;

        gcmDUMP_API("${ES11 glCompressedTexImage2D 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X}",
            Target, Level, XOffset, YOffset, Width, Height, Format, ImageSize, Data);

        glmPROFILE(context, GLES1_COMPRESSEDTEXSUBIMAGE2D, 0);
        /* Validate the arguments. */
        if (Target != GL_TEXTURE_2D)
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        /* Decode the compressed texture format. */
        formatInfo = _GetCompressedPalettedTextureDetails(Format);

        if (formatInfo == gcvNULL)
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        if ((Level < 0) ||
            /* Valid Width and Height. */
            (Width < 0) || (Height < 0)  ||
            (Width  > (GLsizei)context->maxTextureWidth)  ||
            (Height > (GLsizei)context->maxTextureHeight) ||
            (XOffset < 0) || (YOffset < 0) ||
            /* Valid level. */
            (Level  > (GLint)gcoMATH_Ceiling(
                                 gcoMATH_Log2(
                                     (gctFLOAT)context->maxTextureWidth))) ||
            (Data   == gcvNULL)
            )
        {
            glmERROR(GL_INVALID_VALUE);
            break;
        }

        /* Per openGL ES11 spec, there is not supported compress format for this
           function. Return GL_INVALID_OPERATION if no other errors occured. */
        glmERROR(GL_INVALID_OPERATION);
    }
    glmLEAVE();
}


/*******************************************************************************
**
**  glDrawTex (GL_OES_draw_texture)
**
**  OpenGL supports drawing sub-regions of a texture to rectangular regions of
**  the screen using the texturing pipeline. Source region size and content are
**  determined by the texture crop rectangle(s) of the enabled texture(s).
**
**  The functions
**
**      void DrawTex{sifx}OES(T Xs, T Ys, T Zs, T Ws, T Hs);
**      void DrawTex{sifx}vOES(T *coords);
**
**  draw a texture rectangle to the screen.  Xs, Ys, and Zs specify the
**  position of the affected screen rectangle. Xs and Ys are given directly in
**  window (viewport) coordinates.  Zs is mapped to window depth Zw as follows:
**
**                 { n,                 if z <= 0
**            Zw = { f,                 if z >= 1
**                 { n + z * (f - n),   otherwise
**
**  where <n> and <f> are the near and far values of DEPTH_RANGE.  Ws and Hs
**  specify the width and height of the affected screen rectangle in pixels.
**  These values may be positive or negative; however, if either (Ws <= 0) or
**  (Hs <= 0), the INVALID_VALUE error is generated.
**
**  Calling one of the DrawTex functions generates a fragment for each pixel
**  that overlaps the screen rectangle bounded by (Xs, Ys) and (Xs + Ws),
**  (Ys + Hs). For each generated fragment, the depth is given by Zw as defined
**  above, and the color by the current color.
**
**  Texture coordinates for each texture unit are computed as follows:
**
**  Let X and Y be the screen x and y coordinates of each sample point
**  associated with the fragment.  Let Wt and Ht be the width and height in
**  texels of the texture currently bound to the texture unit. Let Ucr, Vcr,
**  Wcr and Hcr be (respectively) the four integers that make up the texture
**  crop rectangle parameter for the currently bound texture. The fragment
**  texture coordinates (s, t, r, q) are given by
**
**      s = (Ucr + (X - Xs)*(Wcr/Ws)) / Wt
**      t = (Vcr + (Y - Ys)*(Hcr/Hs)) / Ht
**      r = 0
**      q = 1
**
**  In the specific case where X, Y, Xs and Ys are all integers, Wcr/Ws and
**  Hcr/Hs are both equal to one, the base level is used for the texture read,
**  and fragments are sampled at pixel centers, implementations are required
**  to ensure that the resulting u, v texture indices are also integers.
**  This results in a one-to-one mapping of texels to fragments.
**
**  Note that Wcr and/or Hcr can be negative.  The formulas given above for
**  s and t still apply in this case. The result is that if Wcr is negative,
**  the source rectangle for DrawTex operations lies to the left of the
**  reference point (Ucr, Vcr) rather than to the right of it, and appears
**  right-to-left reversed on the screen after a call to DrawTex.  Similarly,
**  if Hcr is negative, the source rectangle lies below the reference point
**  (Ucr, Vcr) rather than above it, and appears upside-down on the screen.
**
**  Note also that s, t, r, and q are computed for each fragment as part of
**  DrawTex rendering. This implies that the texture matrix is ignored and
**  has no effect on the rendered result.
**
**  INPUT:
**
**      Xs, Ys, Zs
**          Position of the affected screen rectangle.
**
**      Ws, Hs
**          The width and height of the affected screen rectangle in pixels.
**
**  OUTPUT:
**
**      Nothing.
*/

#undef  _GC_OBJ_ZONE
#define _GC_OBJ_ZONE    glvZONE_EXTENTION

GL_API void GL_APIENTRY glDrawTexsOES(
    GLshort Xs,
    GLshort Ys,
    GLshort Zs,
    GLshort Ws,
    GLshort Hs
    )
{
    glmENTER5(glmARGINT, Xs, glmARGINT, Ys, glmARGINT, Zs, glmARGINT, Ws,
              glmARGINT, Hs)
    {
        gcmDUMP_API("${ES11 glDrawTexsOES 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X}", Xs, Ys, Zs, Ws, Hs);

        if (!_DrawTexOES(
            context,
            glmINT2FLOAT(Xs),
            glmINT2FLOAT(Ys),
            glmINT2FLOAT(Zs),
            glmINT2FLOAT(Ws),
            glmINT2FLOAT(Hs)
            ))
        {
            glmERROR(GL_INVALID_VALUE);
            break;
        }
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glDrawTexiOES(
    GLint Xs,
    GLint Ys,
    GLint Zs,
    GLint Ws,
    GLint Hs
    )
{
    glmENTER5(glmARGINT, Xs, glmARGINT, Ys, glmARGINT, Zs, glmARGINT, Ws,
              glmARGINT, Hs)
    {
        gcmDUMP_API("${ES11 glDrawTexiOES 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X}", Xs, Ys, Zs, Ws, Hs);

        if (!_DrawTexOES(
            context,
            glmINT2FLOAT(Xs),
            glmINT2FLOAT(Ys),
            glmINT2FLOAT(Zs),
            glmINT2FLOAT(Ws),
            glmINT2FLOAT(Hs)
            ))
        {
            glmERROR(GL_INVALID_VALUE);
            break;
        }
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glDrawTexfOES(
    GLfloat Xs,
    GLfloat Ys,
    GLfloat Zs,
    GLfloat Ws,
    GLfloat Hs
    )
{
    glmENTER5(glmARGFLOAT, Xs, glmARGFLOAT, Ys, glmARGFLOAT, Zs,
              glmARGFLOAT, Ws, glmARGFLOAT, Hs)
    {
        gcmDUMP_API("${ES11 glDrawTexiOES 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X}",
            *(GLuint*)&Xs, *(GLuint*)&Ys, *(GLuint*)&Zs, *(GLuint*)&Ws, *(GLuint*)&Hs);

        if (!_DrawTexOES(
            context,
            Xs,
            Ys,
            Zs,
            Ws,
            Hs
            ))
        {
            glmERROR(GL_INVALID_VALUE);
            break;
        }
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glDrawTexxOES(
    GLfixed Xs,
    GLfixed Ys,
    GLfixed Zs,
    GLfixed Ws,
    GLfixed Hs
    )
{
    glmENTER5(glmARGFIXED, Xs, glmARGFIXED, Ys, glmARGFIXED, Zs,
              glmARGFIXED, Ws, glmARGFIXED, Hs)
    {
        gcmDUMP_API("${ES11 glDrawTexxOES 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X}", Xs, Ys, Zs, Ws, Hs);

        if (!_DrawTexOES(
            context,
            glmFIXED2FLOAT(Xs),
            glmFIXED2FLOAT(Ys),
            glmFIXED2FLOAT(Zs),
            glmFIXED2FLOAT(Ws),
            glmFIXED2FLOAT(Hs)
            ))
        {
            glmERROR(GL_INVALID_VALUE);
            break;
        }
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glDrawTexsvOES(
    const GLshort* Coords
    )
{
    glmENTER1(glmARGPTR, Coords)
    {
        gcmDUMP_API("${ES11 glDrawTexsvOES (0x%08X)", Coords);
        gcmDUMP_API_DATA(Coords, sizeof(GLshort) * 5);
        gcmDUMP_API("$}");

        if (Coords != gcvNULL)
        {
            if (!_DrawTexOES(
                context,
                glmINT2FLOAT(Coords[0]),
                glmINT2FLOAT(Coords[1]),
                glmINT2FLOAT(Coords[2]),
                glmINT2FLOAT(Coords[3]),
                glmINT2FLOAT(Coords[4])
                ))
            {
                glmERROR(GL_INVALID_VALUE);
                break;
            }
        }
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glDrawTexivOES(
    const GLint* Coords
    )
{
    glmENTER1(glmARGPTR, Coords)
    {
        gcmDUMP_API("${ES11 glDrawTexivOES (0x%08X)", Coords);
        gcmDUMP_API_ARRAY(Coords, 5);
        gcmDUMP_API("$}");

        if (Coords != gcvNULL)
        {
            if (!_DrawTexOES(
                context,
                glmINT2FLOAT(Coords[0]),
                glmINT2FLOAT(Coords[1]),
                glmINT2FLOAT(Coords[2]),
                glmINT2FLOAT(Coords[3]),
                glmINT2FLOAT(Coords[4])
                ))
            {
                glmERROR(GL_INVALID_VALUE);
                break;
            }
        }
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glDrawTexfvOES(
    const GLfloat* Coords
    )
{
    glmENTER1(glmARGPTR, Coords)
    {
        gcmDUMP_API("${ES11 glDrawTexfvOES (0x%08X)", Coords);
        gcmDUMP_API_ARRAY(Coords, 5);
        gcmDUMP_API("$}");

        if (Coords != gcvNULL)
        {
            if (!_DrawTexOES(
                context,
                Coords[0],
                Coords[1],
                Coords[2],
                Coords[3],
                Coords[4]
                ))
            {
                glmERROR(GL_INVALID_VALUE);
                break;
            }
        }
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glDrawTexxvOES(
    const GLfixed* Coords
    )
{
    glmENTER1(glmARGPTR, Coords)
    {
        gcmDUMP_API("${ES11 glDrawTexxvOES (0x%08X)", Coords);
        gcmDUMP_API_ARRAY(Coords, 5);
        gcmDUMP_API("$}");

        if (Coords != gcvNULL)
        {
            if (!_DrawTexOES(
                context,
                glmFIXED2FLOAT(Coords[0]),
                glmFIXED2FLOAT(Coords[1]),
                glmFIXED2FLOAT(Coords[2]),
                glmFIXED2FLOAT(Coords[3]),
                glmFIXED2FLOAT(Coords[4])
                ))
            {
                glmERROR(GL_INVALID_VALUE);
                break;
            }
        }
    }
    glmLEAVE();
}

/******************************************************************************\
*************************** eglBindTexImage support ****************************
\******************************************************************************/

#undef  _GC_OBJ_ZONE
#define _GC_OBJ_ZONE    glvZONE_TEXTURE

EGLenum
glfBindTexImage(
    void * Surface,
    EGLenum Format,
    EGLBoolean Mipmap,
    EGLint Level,
    EGLint Width,
    EGLint Height,
    void ** Binder
    )
{
    EGLenum error = EGL_BAD_ACCESS;

    glmENTER2(glmARGPTR, Surface,
              glmARGENUM, Format)
    {
        gceSTATUS status;
        gctUINT width                 = 0;
        gctUINT height                = 0;
        glsTEXTURESAMPLER_PTR sampler = gcvNULL;
        glsTEXTUREWRAPPER_PTR texture = gcvNULL;
        gleTARGETTYPE target          = glvTEXTURE2D;
        GLenum format                 = 0;
        gceSURF_FORMAT srcFormat      = gcvSURF_UNKNOWN;
        gceSURF_FORMAT dstFormat      = gcvSURF_UNKNOWN;
        gcsSURF_VIEW surfView          = {(gcoSURF)Surface, 0, 1};
        gcsSURF_VIEW mipView           = {gcvNULL, 0, 1};

        /* Translate EGL format to GL format and HAL format.*/
        switch (Format)
        {
        case EGL_TEXTURE_RGB:
            format    = GL_RGB;
            srcFormat = gcvSURF_B8G8R8;
            break;

        case EGL_TEXTURE_RGBA:
            format    = GL_RGBA;
            srcFormat = gcvSURF_A8B8G8R8;
            break;

        default:
            srcFormat = gcvSURF_UNKNOWN;
            break;
        }

        if (srcFormat == gcvSURF_UNKNOWN)
        {
            error  = EGL_BAD_PARAMETER;
            break;
        }

        /* Translate to closest texture format. */
        if (gcmIS_ERROR(gcoTEXTURE_GetClosestFormat(context->hal,
                                                    srcFormat,
                                                    &dstFormat)))
        {
            error  = EGL_BAD_PARAMETER;
            break;
        }

        /* Get the current sampler. */
        sampler = context->texture.activeSampler;

        /* Get the current texture object. */
        texture = sampler->bindings[target]->name > 0 ? &sampler->binding[target]
                : &context->texture.defaultTexture[target];

        /* Destroy the texture. */
        gcmVERIFY_OK(_ResetTextureWrapper(context, texture));

        /* Done when we are relaseing the texture. */
        if (Surface == gcvNULL)
        {
            if (Binder != gcvNULL)
            {
                *Binder = gcvNULL;
            }

            /* Success. */
            error = EGL_SUCCESS;
            break;
        }

        /* Query surface dimensions. */
        gcmERR_BREAK(gcoSURF_GetSize((gcoSURF)Surface, &width, &height, gcvNULL));

        /* Create a new texture object. */
        gcmERR_BREAK(gcoTEXTURE_ConstructEx(context->hal, _HALtexType[texture->targetType], &texture->object));

        /*
         * Create the mipmap that the render target will resolve to.
         * We should bind the incoming surface directly to the texture for the
         * first time and and do a resolve when necessary to sync.
         */
        gcmERR_BREAK(gcoTEXTURE_AddMipMap(
            texture->object,
            Level,
            format,
            dstFormat,
            width, height, 0,
            0,
            gcvPOOL_DEFAULT,
            &mipView.surf
            ));

        /* Set shared lock.*/
        gcmVERIFY_OK(gcoSURF_SetSharedLock(mipView.surf,
            context->texture.textureList->sharedLock));


        /* Copy render target to texture without flip. */
        gcmERR_BREAK(gcoSURF_ResolveRect(&surfView, &mipView, gcvNULL));

        /* Reset texture orientation. */
        gcmVERIFY_OK(gcoSURF_SetOrientation(
            mipView.surf,
            gcvORIENTATION_TOP_BOTTOM
            ));

        if (Binder != gcvNULL)
        {
            *Binder = (void*)mipView.surf;
        }

        /* See if we need to do any binding. */
        if (sampler->bindings[target] != texture)
        {
            /* Unbind currently bound texture. */
            sampler->bindings[target]->binding = gcvNULL;

            /* Is this the current system target? */
            if (sampler->binding == sampler->bindings[target])
            {
                /* Update the current system target as well. */
                sampler->binding = texture;
            }

            /* Update the sampler binding for the selected target. */
            sampler->bindings[target] = texture;

            /* Bind the texture to the sampler. */
            texture->binding          = sampler;
            texture->boundAtLeastOnce = gcvTRUE;
        }

        /* Level 0 is special. */
        if (Level == 0)
        {
            GLint tValue;
            /* Invalidate normalized crop rectangle. */
            texture->dirtyCropRect = GL_TRUE;

            /* Set texture parameters. */
            texture->width  = width;
            texture->height = height;

            /* Determine the number of mipmaps. */
            tValue = glfGetMaxLOD(texture->width, texture->height);
            texture->maxLevel = gcmMIN(tValue, texture->maxLevel);

            /* Update texture format. */
            _SetTextureWrapperFormat(context, texture, format);

            /* Auto generate mipmaps. */
            if (Mipmap)
            {
                gcmERR_BREAK(glfGenerateMipMaps(
                    context,
                    texture,
                    dstFormat,
                    0,
                    width, height,
                    0
                    ));
            }
        }

        /* Mark texture as dirty. */
        texture->dirty = gcvTRUE;

        /* Success. */
        error = EGL_SUCCESS;
    }
    glmLEAVE();

    return error;
}


/*******************************************************************************
**
**  glTexDirectVIV
**
**  glTexDirectVIV function creates a texture with direct access support. This
**  is useful when the application desires to use the same texture over and over
**  while frequently updating its content. This could be used for mapping live
**  video to a texture. Video decoder could write its result directly to the
**  texture and then the texture could be directly rendered onto a 3D shape.
**
**  If the function succeedes, it returns a pointer, or, for some YUV formats,
**  a set of pointers that directly point to the texture. The pointer(s) will
**  be returned in the user-allocated array pointed to by Pixels parameter.
**
**  The width and height of LOD 0 of the texture is specified by the Width
**  and Height parameters. The driver may auto-generate the rest of LODs if
**  the hardware supports high quality scalling (for non-power of 2 textures)
**  and LOD generation. If the hardware does not support high quality scaing
**  and LOD generation, the texture will remain as a single-LOD texture.
**  The optional mimmap generation may be enabled or disabled through the
**  glTexParameter call with GL_GENERATE_MIPMAP parameter.
**
**  Format parameter supports the following formats: GL_VIV_YV12, GL_VIV_I420,
**  GL_VIV_NV12, GL_VIV_NV21, GL_VIV_YUY2 and GL_VIV_UYVY.
**
**  If Format is GL_VIV_YV12, glTexDirectVIV creates a planar YV12 4:2:0 texture
**  and the format of Pixels array is as follows: (Yplane, Vplane, Uplane).
**
**  If Format is GL_VIV_I420, glTexDirectVIV creates a planar I420 4:2:0 texture
**  and the format of Pixels array is as follows: (Yplane, Uplane, Vplane).
**
**  If Format is GL_VIV_NV12, glTexDirectVIV creates a planar NV12 4:2:0 texture
**  and the format of Pixels array is as follows: (Yplane, UVplane).
**
**  If Format is GL_VIV_NV21, glTexDirectVIV creates a planar NV21 4:2:0 texture
**  and the format of Pixels array is as follows: (Yplane, VUplane).
**
**  If Format is GL_VIV_YUY2 or GL_VIV_UYVY or, glTexDirectVIV creates a packed
**  4:2:2 texture and the Pixels array contains only one pointer to the packed
**  YUV texture.
**
**  ERRORS:
**
**      GL_INVALID_ENUM
**          - if Target is not GL_TEXTURE_2D;
**          - if Format is not a valid format.
**
**      GL_INVALID_VALUE
**          - if Width or Height parameters are less than 1.
**
**      GL_OUT_OF_MEMORY
**          - if a memory allocation error occures at any point.
**
**      GL_INVALID_OPERATION
**          - if the specified format is not supported by the hardware;
**          - if no texture is bound to the active texture unit;
**          - if any other error occures during the call.
**
**  INPUT:
**
**      Target
**          Specifies the target texture. Must be GL_TEXTURE_2D.
**
**      Width
**      Height
**          Specifies the size of LOD 0.
**
**      Format
**          One of the following: GL_VIV_YV12, GL_VIV_I420,
**          GL_VIV_NV12, GL_VIV_NV21, GL_VIV_YUY2, GL_VIV_UYVY.
**
**      Pixels
**          Pointer to the application-defined array of pointers to the
**          texture created by glTexDirectVIV. There should be enough space
**          allocated to hold as many pointers as needed for the specified
**          format.
**
**  OUTPUT:
**
**      Pixels array initialized with texture pointers.
*/

GL_API void GL_APIENTRY glTexDirectVIV(
    GLenum Target,
    GLsizei Width,
    GLsizei Height,
    GLenum Format,
    GLvoid ** Pixels
    )
{
    glmENTER5(glmARGENUM, Target, glmARGINT, Width,
              glmARGINT, Height, glmARGENUM, Format, glmARGPTR, Pixels)
    {
        gceSTATUS status = gcvSTATUS_OK;
        gctBOOL tilerAvailable;
        gctBOOL sourceYuv;
        gctBOOL planarYuv;
        gceSURF_FORMAT sourceFormat   = gcvSURF_UNKNOWN;
        gceSURF_FORMAT textureFormat  = gcvSURF_UNKNOWN;
        glsTEXTURESAMPLER_PTR sampler = gcvNULL;
        glsTEXTUREWRAPPER_PTR texture = gcvNULL;
        gctPOINTER pixels[3] = {gcvNULL};

        /***********************************************************************
        ** Validate parameters.
        */

        /* Validate the target. */
        if (Target != GL_TEXTURE_2D)
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        /* Simple parameter validation. */
        if ((Width < 1) || (Height < 1) || (Pixels == gcvNULL))
        {
            glmERROR(GL_INVALID_VALUE);
            break;
        }

        /***********************************************************************
        ** Get the bound texture.
        */

        /* Get a shortcut to the active sampler. */
        sampler = context->texture.activeSampler;

        /* Get a shortcut to the bound texture. */
        texture = sampler->binding;

        /* A texture has to be bound. */
        if (texture == gcvNULL)
        {
            glmERROR(GL_INVALID_OPERATION);
            break;
        }

        /***********************************************************************
        ** Query hardware support.
        */

        tilerAvailable = gcoHAL_IsFeatureAvailable(
            context->hal, gcvFEATURE_YUV420_TILER
            ) == gcvSTATUS_TRUE;

        /***********************************************************************
        ** Validate the format.
        */

        switch (Format)
        {
        case GL_VIV_YV12:
            sourceFormat = gcvSURF_YV12;
            textureFormat = gcvSURF_YUY2;
            sourceYuv = gcvTRUE;
            planarYuv = gcvTRUE;
            break;

        case GL_VIV_I420:
            sourceFormat = gcvSURF_I420;
            textureFormat = gcvSURF_YUY2;
            sourceYuv = gcvTRUE;
            planarYuv = gcvTRUE;
            break;

        case GL_VIV_NV12:
            sourceFormat = gcvSURF_NV12;
            textureFormat = gcvSURF_YUY2;
            sourceYuv = gcvTRUE;
            planarYuv = gcvTRUE;
            break;

        case GL_VIV_NV21:
            sourceFormat = gcvSURF_NV21;
            textureFormat = gcvSURF_YUY2;
            sourceYuv = gcvTRUE;
            planarYuv = gcvTRUE;
            break;

        case GL_VIV_YUY2:
            sourceFormat = gcvSURF_YUY2;
            textureFormat = gcvSURF_YUY2;
            sourceYuv = gcvTRUE;
            planarYuv = gcvFALSE;
            break;

        case GL_VIV_UYVY:
            sourceFormat = gcvSURF_UYVY;
            textureFormat = gcvSURF_UYVY;
            sourceYuv = gcvTRUE;
            planarYuv = gcvFALSE;
            break;

        case GL_VIV_YUV420_10_ST:
            sourceFormat  = gcvSURF_YUV420_10_ST;
            textureFormat = gcvSURF_YUV420_10_ST;
            sourceYuv = gcvTRUE;
            planarYuv = gcvTRUE;
            break;

        case GL_VIV_YUV420_TILE_ST:
            sourceFormat  = gcvSURF_YUV420_TILE_ST;
            textureFormat = gcvSURF_YUV420_TILE_ST;
            sourceYuv = gcvTRUE;
            planarYuv = gcvTRUE;
            break;

        case GL_VIV_YUV420_TILE_10_ST:
            sourceFormat  = gcvSURF_YUV420_TILE_10_ST;
            textureFormat = gcvSURF_YUV420_TILE_10_ST;
            sourceYuv = gcvTRUE;
            planarYuv = gcvTRUE;
            break;

        case GL_RGBA:
            sourceFormat = gcvSURF_A8B8G8R8;
            gcoTEXTURE_GetClosestFormat(gcvNULL,
                                        sourceFormat,
                                        &textureFormat);
            sourceYuv = gcvFALSE;
            planarYuv = gcvFALSE;
            break;

        case GL_RGB:
            sourceFormat = gcvSURF_X8R8G8B8;
            gcoTEXTURE_GetClosestFormat(gcvNULL,
                                        sourceFormat,
                                        &textureFormat);
            sourceYuv = gcvFALSE;
            planarYuv = gcvFALSE;
            break;

        case GL_BGRA_EXT:
            sourceFormat = gcvSURF_A8R8G8B8;
            gcoTEXTURE_GetClosestFormat(gcvNULL,
                                        sourceFormat,
                                        &textureFormat);
            sourceYuv = gcvFALSE;
            planarYuv = gcvFALSE;
            break;

        case GL_RGB565_OES:
            sourceFormat = gcvSURF_R5G6B5;
            gcoTEXTURE_GetClosestFormat(gcvNULL,
                                        sourceFormat,
                                        &textureFormat);
            sourceYuv = gcvFALSE;
            planarYuv = gcvFALSE;
            break;

        default:
            status = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

        if (gcmIS_ERROR(status))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        /***********************************************************************
        ** Check whether the source can be handled.
        */

        if (sourceYuv)
        {
            /*
             * Planar YUV requires 420tiler (422 not supported)
             * or yuv-assembler.
             */
            if (planarYuv && !context->hasYuvAssembler && !tilerAvailable)
            {
                glmERROR(GL_INVALID_OPERATION);
                break;
            }

            if ((Format == GL_VIV_YUV420_10_ST
                || Format == GL_VIV_YUV420_TILE_ST
                || Format == GL_VIV_YUV420_TILE_10_ST)
            && (!context->hasYuvAssembler10bit))
            {
                glmERROR(GL_INVALID_OPERATION);
                break;
            }
        }

        /***********************************************************************
        ** Reset the bound texture.
        */

        /* Remove the existing texture. */
        status = _ResetTextureWrapper(context, texture);

        if (gcmIS_ERROR(status))
        {
            glmERROR(GL_INVALID_OPERATION);
            break;
        }

        /* Invalidate normalized crop rectangle. */
        texture->dirtyCropRect = GL_TRUE;

        /* Set texture parameters. */
        texture->width  = Width;
        texture->height = Height;

        /* Set texture format. */
        texture->direct.textureFormat = textureFormat;

        /* Reset the dirty flag. */
        texture->direct.dirty = gcvFALSE;

        /* YUV texture reaches the shader in the RGB form. */
        _SetTextureWrapperFormat(context, texture, GL_RGB);

        /* Construct the source surface. */
        status = gcoSURF_Construct(
            context->hal,
            Width, Height, 1,
            gcvSURF_BITMAP,
            sourceFormat,
            gcvPOOL_DEFAULT,
            &texture->direct.source
            );

        if (gcmIS_ERROR(status))
        {
            glmERROR(GL_OUT_OF_MEMORY);
            break;
        }

        /* Lock the source surface. */
        status = gcoSURF_Lock(
            texture->direct.source,
            gcvNULL,
            pixels
            );

        /* Test if can sample texture source directly. */
        texture->direct.directSample = gcvFALSE;

        if (sourceYuv)
        {
            /* YUV class. */
            if (
                /* YUV assembler for planar yuv. */
                (planarYuv && context->hasYuvAssembler)
                /* linear texture for interleaved yuv. */
            ||  (!planarYuv && context->hasLinearTx)
            )
            {
                texture->direct.directSample = gcvTRUE;
            }
        }
        else
        {
            /* RGB class. */
            if (
                /* Need linear texture feature. */
                (context->hasLinearTx)
                /* and when format supported. */
            &&  (context->hasTxSwizzle || sourceFormat == textureFormat)
            )
            {
                texture->direct.directSample = gcvTRUE;
            }
        }

        /* Return back virtual address pointers. */
        switch (Format)
        {
        case GL_VIV_YV12:
            /* Y plane, V plane, U plane. */
            Pixels[0] = pixels[0];
            Pixels[1] = pixels[2];
            Pixels[2] = pixels[1];
            break;

        case GL_VIV_I420:
            /* Y plane, U plane, V plane. */
            Pixels[0] = pixels[0];
            Pixels[1] = pixels[1];
            Pixels[2] = pixels[2];
            break;

        case GL_VIV_NV12:
        case GL_VIV_NV21:
        case GL_VIV_YUV420_10_ST:
        case GL_VIV_YUV420_TILE_ST:
        case GL_VIV_YUV420_TILE_10_ST:
            /* Y plane, UV (VU) plane. */
            Pixels[0] = pixels[0];
            Pixels[1] = pixels[1];
            break;

        default:
            /* Single plane. */
            Pixels[0] = pixels[0];
            break;
        }

        gcmDUMP_API("${ES11 glTexDirectVIV 0x%08X 0x%08X 0x%08X 0x%08X (0x%08X)", Target, Width, Height, Format, Pixels);
        gcmDUMP_API_ARRAY(*Pixels, 1);
        gcmDUMP_API("$}");
    }
    glmLEAVE();
}

/*******************************************************************************
**
**  glTexDirectVIVMap
**
**  glTexDirectVIVMap function creates a texture with direct access support.
**  This function is similar to glTexDirectVIV, the only difference is that,
**  glTexDirectVIVMap has two input "Logical" and "Physical", which support mapping
**  a user space momory or a physical address into the texture surface.
**
**  If the function succeedes, it returns a pointer, or, for some YUV formats,
**  a set of pointers that directly point to the texture. The pointer(s) will
**  be returned in the user-allocated array pointed to by Pixels parameter.
**
**  The width and height of LOD 0 of the texture is specified by the Width
**  and Height parameters. The driver may auto-generate the rest of LODs if
**  the hardware supports high quality scalling (for non-power of 2 textures)
**  and LOD generation. If the hardware does not support high quality scaing
**  and LOD generation, the texture will remain as a single-LOD texture.
**  The optional mimmap generation may be enabled or disabled through the
**  glTexParameter call with GL_GENERATE_MIPMAP parameter.
**
**  Format parameter supports the following formats: GL_VIV_YV12, GL_VIV_I420,
**  GL_VIV_NV12, GL_VIV_NV21, GL_VIV_YUY2 and GL_VIV_UYVY.
**
**  If Format is GL_VIV_YV12, glTexDirectVIVMap creates a planar YV12 4:2:0 texture
**  and the format of Pixels array is as follows: (Yplane, Vplane, Uplane).
**
**  If Format is GL_VIV_I420, glTexDirectVIVMap creates a planar I420 4:2:0 texture
**  and the format of Pixels array is as follows: (Yplane, Uplane, Vplane).
**
**  If Format is GL_VIV_NV12, glTexDirectVIVMap creates a planar NV12 4:2:0 texture
**  and the format of Pixels array is as follows: (Yplane, UVplane).
**
**  If Format is GL_VIV_NV21, glTexDirectVIVMap creates a planar NV21 4:2:0 texture
**  and the format of Pixels array is as follows: (Yplane, VUplane).
**
**  If Format is GL_VIV_YUY2 or GL_VIV_UYVY, glTexDirectVIVMap creates a packed
**  4:2:2 texture and the Pixels array contains only one pointer to the packed
**  YUV texture.
**
**  ERRORS:
**
**      GL_INVALID_ENUM
**          - if Target is not GL_TEXTURE_2D;
**          - if Format is not a valid format.
**
**      GL_INVALID_VALUE
**          - if Width or Height parameters are less than 1.
            - if Width or Height are not aligned.
**
**      GL_OUT_OF_MEMORY
**          - if a memory allocation error occures at any point.
**
**      GL_INVALID_OPERATION
**          - if the specified format is not supported by the hardware;
**          - if no texture is bound to the active texture unit;
**          - if any other error occures during the call.
**
**  INPUT:
**
**      Target
**          Specifies the target texture. Must be GL_TEXTURE_2D.
**
**      Width
**      Height
**          Specifies the size of LOD 0.
**
**      Format
**          One of the following: GL_VIV_YV12, GL_VIV_I420,
**          GL_VIV_NV12, GL_VIV_NV21, GL_VIV_YUY2, GL_VIV_UYVY.
**
**      Logical
**          Pointer to the logic address of the application-defined buffer
**          to the texture. or NULL if no logical pointer has been provided.
**
**      Physical
**          Physical address of the application-defined buffer
**          to the texture. or ~0 if no physical pointer has been provided.
**
**
*/

GL_API void GL_APIENTRY glTexDirectVIVMap(
    GLenum Target,
    GLsizei Width,
    GLsizei Height,
    GLenum Format,
    GLvoid ** Logical,
    const GLuint * Physical
    )
{
    glmENTER6(glmARGENUM, Target, glmARGINT, Width,
              glmARGINT, Height, glmARGENUM, Format, glmARGPTR, Logical, glmARGPTR, Physical)
    {
        gceSTATUS status;
        gctBOOL tilerAvailable;
        gctBOOL sourceYuv;
        gctBOOL planarYuv;
        gceSURF_FORMAT sourceFormat;
        gceSURF_FORMAT textureFormat;
        glsTEXTURESAMPLER_PTR sampler;
        glsTEXTUREWRAPPER_PTR texture;
        gctINT32 tileWidth, tileHeight;

        /***********************************************************************
        ** Validate parameters.
        */

        /* Validate the target. */
        if (Target != GL_TEXTURE_2D)
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        /* Simple parameter validation. */
        if ((Width < 1) || (Height < 1) || (Logical == gcvNULL))
        {
            glmERROR(GL_INVALID_VALUE);
            break;
        }

        if ( (*Logical) == gcvNULL || ((gctUINTPTR_T)(*Logical) & 0x3F) != 0 )
        {
            glmERROR(GL_INVALID_VALUE);
            break;
        }

        gcoHAL_QueryTiled(context->hal,
                          gcvNULL, gcvNULL,
                          &tileWidth, &tileHeight);

        /* Currently hardware only supprot aligned Width and Height */
        if (Width & (tileWidth * 4 - 1))
        {
            glmERROR(GL_INVALID_VALUE);
            break;
        }

        if (Height & (tileHeight - 1))
        {
            glmERROR(GL_INVALID_VALUE);
            break;
        }

        /***********************************************************************
        ** Get the bound texture.
        */

        /* Get a shortcut to the active sampler. */
        sampler = context->texture.activeSampler;

        /* Get a shortcut to the bound texture. */
        texture = sampler->binding;

        /* A texture has to be bound. */
        if (texture == gcvNULL)
        {
            glmERROR(GL_INVALID_OPERATION);
            break;
        }

        /***********************************************************************
        ** Query hardware support.
        */

        tilerAvailable = gcoHAL_IsFeatureAvailable(
            context->hal, gcvFEATURE_YUV420_TILER
            ) == gcvSTATUS_TRUE;

        /***********************************************************************
        ** Validate the format.
        */

        if (Format == GL_VIV_YV12)
        {
            sourceFormat = gcvSURF_YV12;
            textureFormat = gcvSURF_YUY2;
            sourceYuv = gcvTRUE;
            planarYuv = gcvTRUE;
        }

        else if (Format == GL_VIV_I420)
        {
            sourceFormat = gcvSURF_I420;
            textureFormat = gcvSURF_YUY2;
            sourceYuv = gcvTRUE;
            planarYuv = gcvTRUE;
        }

        else if (Format == GL_VIV_NV12)
        {
            sourceFormat = gcvSURF_NV12;
            textureFormat = gcvSURF_YUY2;
            sourceYuv = gcvTRUE;
            planarYuv = gcvTRUE;
        }

        else if (Format == GL_VIV_NV21)
        {
            sourceFormat = gcvSURF_NV21;
            textureFormat = gcvSURF_YUY2;
            sourceYuv = gcvTRUE;
            planarYuv = gcvTRUE;
        }

        else if (Format == GL_VIV_YUY2)
        {
            sourceFormat = gcvSURF_YUY2;
            textureFormat = gcvSURF_YUY2;
            sourceYuv = gcvTRUE;
            planarYuv = gcvFALSE;
        }

        else if (Format == GL_VIV_UYVY)
        {
            sourceFormat = gcvSURF_UYVY;
            textureFormat = gcvSURF_UYVY;
            sourceYuv = gcvTRUE;
            planarYuv = gcvFALSE;
        }

        else if (Format == GL_RGB565_OES)
        {
            sourceFormat = gcvSURF_R5G6B5;
            gcoTEXTURE_GetClosestFormat(gcvNULL, sourceFormat, &textureFormat);

            sourceYuv = gcvFALSE;
            planarYuv = gcvFALSE;
        }

        else if (Format == GL_RGBA)
        {
            sourceFormat = gcvSURF_A8B8G8R8;
            gcoTEXTURE_GetClosestFormat(gcvNULL, sourceFormat, &textureFormat);

            sourceYuv = gcvFALSE;
            planarYuv = gcvFALSE;
        }

        else if (Format == GL_RGB)
        {
            sourceFormat = gcvSURF_X8R8G8B8;
            gcoTEXTURE_GetClosestFormat(gcvNULL,
                                        sourceFormat,
                                        &textureFormat);
            sourceYuv = gcvFALSE;
            planarYuv = gcvFALSE;
        }

        else if (Format == GL_BGRA_EXT)
        {
            sourceFormat = gcvSURF_A8R8G8B8;
            gcoTEXTURE_GetClosestFormat(gcvNULL, sourceFormat, &textureFormat);

            sourceYuv = gcvFALSE;
            planarYuv = gcvFALSE;
        }

        else
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        /***********************************************************************
        ** Check whether the source can be handled.
        */

        if (sourceYuv && planarYuv && !tilerAvailable)
        {
            glmERROR(GL_INVALID_OPERATION);
            break;
        }

        /***********************************************************************
        ** Reset the bound texture.
        */

        /* Remove the existing texture. */
        status = _ResetTextureWrapper(context, texture);

        if (gcmIS_ERROR(status))
        {
            glmERROR(GL_INVALID_OPERATION);
            break;
        }

        /* Invalidate normalized crop rectangle. */
        texture->dirtyCropRect = GL_TRUE;

        /* Set texture parameters. */
        texture->width  = Width;
        texture->height = Height;

        /* Set texture format. */
        texture->direct.textureFormat = textureFormat;

        /* Reset the dirty flag. */
        texture->direct.dirty = gcvFALSE;

        /* YUV texture reaches the shader in the RGB form. */
        _SetTextureWrapperFormat(context, texture, GL_RGB);

        /* Construct the source surface. */
        status = gcoSURF_Construct(
            context->hal,
            Width, Height, 1,
            gcvSURF_BITMAP,
            sourceFormat,
            gcvPOOL_USER,
            &texture->direct.source
            );

        if (gcmIS_ERROR(status))
        {
            glmERROR(GL_OUT_OF_MEMORY);
            break;
        }

        /* Set the user buffer to the surface. */
        gcmERR_BREAK(gcoSURF_MapUserSurface(
            texture->direct.source,
            0,
            (GLvoid*)(*Logical),
            (gctUINT32)(*Physical)
            ));

        gcmERR_BREAK(gcoSURF_Lock(
            texture->direct.source,
            gcvNULL, gcvNULL));

        /* Test if can sample texture source directly. */
        texture->direct.directSample = gcvFALSE;

        if (sourceYuv)
        {
            /* YUV class. */
            if (
                /* YUV assembler for planar yuv. */
                (planarYuv && context->hasYuvAssembler)
                /* linear texture for interleaved yuv. */
            ||  (!planarYuv && context->hasLinearTx)
            )
            {
                texture->direct.directSample = gcvTRUE;
            }
        }
        else
        {
            /* RGB class. */
            if (
                /* Need linear texture feature. */
                (context->hasLinearTx)
                /* and when format supported. */
            &&  (context->hasTxSwizzle || sourceFormat == textureFormat)
            )
            {
                texture->direct.directSample = gcvTRUE;
            }
        }
    }
    glmLEAVE();
}


/* Correct API name. */
GL_API void GL_APIENTRY glTexDirectMapVIV(
    GLenum Target,
    GLsizei Width,
    GLsizei Height,
    GLenum Format,
    GLvoid ** Logical,
    const GLuint * Physical
    )
{
    glTexDirectVIVMap(Target, Width, Height, Format, Logical, Physical);
}


/*******************************************************************************
**
**  glTexDirectTiledMapVIV
**
**  glTexDirectTiledMapVIV function creates a texture with direct access support.
**  This function is similar to glTexDirectVIV, the only difference is that,
**  glTexDirectTiledMapVIV has two input "Logical" and "Physical", which support mapping
**  a user space momory or a physical address into the texture surface.
**
**  If the function succeedes, it returns a pointer, or, for some YUV formats,
**  a set of pointers that directly point to the texture. The pointer(s) will
**  be returned in the user-allocated array pointed to by Pixels parameter.
**
**  The width and height of LOD 0 of the texture is specified by the Width
**  and Height parameters. The driver may auto-generate the rest of LODs if
**  the hardware supports high quality scalling (for non-power of 2 textures)
**  and LOD generation. If the hardware does not support high quality scaing
**  and LOD generation, the texture will remain as a single-LOD texture.
**  The optional mimmap generation may be enabled or disabled through the
**  glTexParameter call with GL_GENERATE_MIPMAP parameter.
**
**  Format parameter supports the following formats: GL_VIV_YUY2.
**
**  If Format is GL_VIV_YUY2,  glTexDirectTiledMapVIV creates a packed
**  4:2:2 texture and the Pixels array contains only one pointer to the packed
**  YUV texture.
**
**  ERRORS:
**
**      GL_INVALID_ENUM
**          - if Target is not GL_TEXTURE_2D;
**          - if Format is not a valid format.
**
**      GL_INVALID_VALUE
**          - if Width or Height parameters are less than 1.
**          - if Width or Height are not aligned.
**
**      GL_OUT_OF_MEMORY
**          - if a memory allocation error occures at any point.
**
**      GL_INVALID_OPERATION
**          - if the specified format is not supported by the hardware;
**          - if no texture is bound to the active texture unit;
**          - if any other error occures during the call.
**
**  INPUT:
**
**      Target
**          Specifies the target texture. Must be GL_TEXTURE_2D.
**
**      Width
**      Height
**          Specifies the size of LOD 0.
**
**      Format
**          One of the following: GL_VIV_YUY2
**
**      Logical
**          Pointer to the logic address of the application-defined buffer
**          to the texture. or NULL if no logical pointer has been provided.
**
**      Physical
**          Physical address of the application-defined buffer
**          to the texture. or ~0 if no physical pointer has been provided.
**
**
*/
GL_API void GL_APIENTRY glTexDirectTiledMapVIV(
    GLenum Target,
    GLsizei Width,
    GLsizei Height,
    GLenum Format,
    GLvoid ** Logical,
    const GLuint * Physical
    )
{
    glmENTER6(glmARGENUM, Target, glmARGINT, Width,
              glmARGINT, Height, glmARGENUM, Format, glmARGPTR, Logical, glmARGPTR, Physical)
    {
        gceSTATUS status;
        gctBOOL tilerAvailable;
        gctBOOL sourceYuv;
        gctBOOL planarYuv;
        gceSURF_FORMAT sourceFormat;
        gceSURF_FORMAT textureFormat;
        glsTEXTURESAMPLER_PTR sampler;
        glsTEXTUREWRAPPER_PTR texture;
        gctINT32 tileWidth, tileHeight;

        /***********************************************************************
        ** Validate parameters.
        */

        /* Validate the target. */
        if (Target != GL_TEXTURE_2D)
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        /* Simple parameter validation. */
        if ((Width < 1) || (Height < 1) || (Logical == gcvNULL))
        {
            glmERROR(GL_INVALID_VALUE);
            break;
        }

        if ( (*Logical) == gcvNULL || ((gctUINTPTR_T)(*Logical) & 0x3F) != 0 )
        {
            glmERROR(GL_INVALID_VALUE);
            break;
        }

        gcoHAL_QueryTiled(context->hal,
                          gcvNULL, gcvNULL,
                          &tileWidth, &tileHeight);

        /* Currently hardware only supprot aligned Width and Height */
        if (Width & (tileWidth * 4 - 1))
        {
            glmERROR(GL_INVALID_VALUE);
            break;
        }

        if (Height & (tileHeight - 1))
        {
            glmERROR(GL_INVALID_VALUE);
            break;
        }

        /***********************************************************************
        ** Get the bound texture.
        */

        /* Get a shortcut to the active sampler. */
        sampler = context->texture.activeSampler;

        /* Get a shortcut to the bound texture. */
        texture = sampler->binding;

        /* A texture has to be bound. */
        if (texture == gcvNULL)
        {
            glmERROR(GL_INVALID_OPERATION);
            break;
        }

        /***********************************************************************
        ** Query hardware support.
        */

        tilerAvailable = gcoHAL_IsFeatureAvailable(
            context->hal, gcvFEATURE_YUV420_TILER
            ) == gcvSTATUS_TRUE;


        /***********************************************************************
        ** Validate the format.
        */

        if (Format == GL_VIV_YUY2)
        {
            sourceFormat = gcvSURF_YUY2;
            textureFormat = gcvSURF_YUY2;
            sourceYuv = gcvTRUE;
            planarYuv = gcvFALSE;
        }

        else if (Format == GL_RGBA)
        {
            sourceFormat = gcvSURF_A8B8G8R8;
            gcoTEXTURE_GetClosestFormat(gcvNULL, sourceFormat, &textureFormat);

            sourceYuv = gcvFALSE;
            planarYuv = gcvFALSE;
        }

        else if (Format == GL_BGRA_EXT)
        {
            sourceFormat = gcvSURF_A8R8G8B8;
            gcoTEXTURE_GetClosestFormat(gcvNULL, sourceFormat, &textureFormat);

            sourceYuv = gcvFALSE;
            planarYuv = gcvFALSE;
        }

        else
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        /***********************************************************************
        ** Check whether the source can be handled.
        */

        if (sourceYuv && planarYuv && !tilerAvailable)
        {
            glmERROR(GL_INVALID_OPERATION);
            break;
        }

        /***********************************************************************
        ** Reset the bound texture.
        */

        /* Remove the existing texture. */
        status = _ResetTextureWrapper(context, texture);

        if (gcmIS_ERROR(status))
        {
            glmERROR(GL_INVALID_OPERATION);
            break;
        }

        /* Invalidate normalized crop rectangle. */
        texture->dirtyCropRect = GL_TRUE;

        /* Set texture parameters. */
        texture->width  = Width;
        texture->height = Height;

        /* Set texture format. */
        texture->direct.textureFormat = textureFormat;

        /* Reset the dirty flag. */
        texture->direct.dirty = gcvFALSE;

        /* YUV texture reaches the shader in the RGB form. */
        _SetTextureWrapperFormat(context, texture, GL_RGB);

        /* Construct the source surface. */
        status = gcoSURF_Construct(
            context->hal,
            Width, Height, 1,
            gcvSURF_TEXTURE,
            sourceFormat,
            gcvPOOL_USER,
            &texture->direct.source
            );

        if (gcmIS_ERROR(status))
        {
            glmERROR(GL_OUT_OF_MEMORY);
            break;
        }

        /* Set the user buffer to the surface. */
        gcmERR_BREAK(gcoSURF_MapUserSurface(
            texture->direct.source,
            0,
            (GLvoid*)(*Logical),
            (gctUINT32)(*Physical)
            ));

        if (sourceYuv)
        {
            /* Tiled YUY2, UYVY. */
            texture->direct.directSample = gcvTRUE;
        }
        else if ((context->hasTxSwizzle || sourceFormat == textureFormat))
        {
            texture->direct.directSample = gcvTRUE;
        }
        else
        {
            texture->direct.directSample = gcvFALSE;
        }
    }
    glmLEAVE();
}


/*******************************************************************************
**
**  glTexDirectInvalidateVIV
**
**  glTexDirectInvalidateVIV function should be used by the applcations to
**  signal that the direct texture data has changed to allow the OpenGL driver
**  and hardware to perform any transformations on the data if necessary.
**
**  ERRORS:
**
**      GL_INVALID_ENUM
**          - if Target is not GL_TEXTURE_2D.
**
**      GL_INVALID_OPERATION
**          - if no texture is bound to the active texture unit;
**          - if the bound texture is not a direct texture.
**
**  INPUT:
**
**      Target
**          Specifies the target texture. Must be GL_TEXTURE_2D.
**
**  OUTPUT:
**
**      Nothing.
*/

GL_API void GL_APIENTRY glTexDirectInvalidateVIV(
    GLenum Target
    )
{
    glmENTER1(glmARGENUM, Target)
    {
        glsTEXTURESAMPLER_PTR sampler;
        glsTEXTUREWRAPPER_PTR texture;

        /* Validate arguments. */
        if (Target != GL_TEXTURE_2D)
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        /* Get a shortcut to the active sampler. */
        sampler = context->texture.activeSampler;

        /* Get a shortcut to the bound texture. */
        texture = sampler->binding;

        /* A texture has to be bound. */
        if (texture == gcvNULL)
        {
            glmERROR(GL_INVALID_OPERATION);
            break;
        }

        /* Has to be a planar-sourced texture. */
        if (texture->direct.source == gcvNULL)
        {
            glmERROR(GL_INVALID_OPERATION);
            break;
        }

        /* Mark texture as dirty to be flushed later. */
        texture->dirty = gcvTRUE;

        /* Set the quick-reference dirty flag. */
        texture->direct.dirty = gcvTRUE;
    }
    glmLEAVE();
}


/*******************************************************************************
**
**  veglCreateImageTexture
**  (EGL_KHR_gl_texture_2D_image, KHR_gl_texture_cubemap_image)
**
**  veglCreateImageTexture function create an EGLimage from given args.
**
**
**  ERRORS:
**
**      GL_INVALID_VALUE
**          -if cann't find a texture object or sub-object from the given args
**          -if Level isn't 0 and the texture isn't complete
**
**
**  INPUT:
**
**      Target
**          Specifies the target texture.
**
**      Texture
**          Specifies the name of a texture ojbect.
**
**      Level
**          Specifies the texture level of a mipmap.
**
**      Face
**          Specifies the face of a cube map.
**
**      Depth
**          Specifies the depeth of a 3D map.
**
**  OUTPUT:
**
**      Image
**          return an EGLImage object.
*/

EGLenum
glfCreateImageTexture(
    void* Context,
    EGLenum Target,
    gctINT Texture,
    gctINT Level,
    gctINT Depth,
    gctPOINTER Image
    )
{
    EGLenum status = EGL_BAD_PARAMETER;

    glmENTER5(glmARGENUM, Target, glmARGHEX, Texture,
              glmARGINT, Level, glmARGINT, Depth,
              glmARGPTR, Image)
    {
        gceTEXTURE_FACE face;
        glsTEXTUREWRAPPER_PTR texture;
        khrEGL_IMAGE_PTR image = gcvNULL;
        khrIMAGE_TYPE imageType;
        gctINT32 referenceCount = 0;
        gcoSURF surface;

        /* Determine the target and the texture. */
        switch (Target)
        {
        case EGL_GL_TEXTURE_2D_KHR:
            face      = gcvFACE_NONE;
            imageType = KHR_IMAGE_TEXTURE_2D;
            break;

        case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR:
            face      = gcvFACE_POSITIVE_X;
            imageType = KHR_IMAGE_TEXTURE_CUBE;
            break;

        case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_X_KHR:
            face      = gcvFACE_NEGATIVE_X;
            imageType = KHR_IMAGE_TEXTURE_CUBE;
            break;

        case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Y_KHR:
            face      = gcvFACE_POSITIVE_Y;
            imageType = KHR_IMAGE_TEXTURE_CUBE;
            break;

        case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_KHR:
            face      = gcvFACE_NEGATIVE_Y;
            imageType = KHR_IMAGE_TEXTURE_CUBE;
            break;

        case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Z_KHR:
            face      = gcvFACE_POSITIVE_Z;
            imageType = KHR_IMAGE_TEXTURE_CUBE;
            break;

        case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_KHR:
            face      = gcvFACE_NEGATIVE_Z;
            imageType = KHR_IMAGE_TEXTURE_CUBE;
            break;

        default:
            status = EGL_BAD_PARAMETER;
            goto OnError;
        }

        /* Create eglImage from default texture object isn't permitted. */
        if (Texture == 0)
        {
            status = EGL_BAD_PARAMETER;
            goto OnError;
        }

        /* Find the texture object by name. */
        texture = glfFindTexture(context, Texture);

        /* Test texture object. */
        if ((texture == gcvNULL) || (texture->object == gcvNULL))
        {
            status = EGL_BAD_PARAMETER;
            goto OnError;
        }

        /* Get the surface of a level */
        if (gcmIS_ERROR(gcoTEXTURE_GetMipMap(texture->object, Level, &surface)))
        {
            status = EGL_BAD_PARAMETER;
            goto OnError;
        }

        if (surface == gcvNULL)
        {
            status = EGL_BAD_PARAMETER;
            goto OnError;
        }

        /* Test if the texture is from EGL image. */
        if (texture->image.source != gcvNULL)
        {
            status = EGL_BAD_ACCESS;
            goto OnError;
        }

        /* Test if texture is yuv format. */
        if (texture->direct.source != gcvNULL)
        {
            status = EGL_BAD_ACCESS;
            goto OnError;
        }

        /* Get source surface reference count. */
        gcmVERIFY_OK(gcoSURF_QueryReferenceCount(surface, &referenceCount));

        /* Test if surface is a sibling of any eglImage. */
        if (referenceCount > 1)
        {
            status = EGL_BAD_ACCESS;
            goto OnError;
        }

        image = (khrEGL_IMAGE_PTR) Image;

        /* Set EGL Image info. */
        image->magic    = KHR_EGL_IMAGE_MAGIC_NUM;
        image->type     = imageType;
        image->surface  = surface;

        image->u.texture.format  = (gctUINT) texture->format;
        image->u.texture.level   = Level;
        image->u.texture.face    = face;
        image->u.texture.depth   = Depth;
        image->u.texture.texture = Texture;
        image->u.texture.object  = texture->object;

        /* Success. */
        status = EGL_SUCCESS;

        /* Easy return on error. */
OnError:;
    }
    glmLEAVE();

    return status;
}


/*******************************************************************************
**
**  glEGLImageTargetTexture2DOES (GL_OES_EGL_image)
**
**  Defines an entire two-dimensional texture array.  All properties of the
**  texture images (including width, height, format, border, mipmap levels of
**  detail, and image data) are taken from the specified eglImageOES <image>,
**  rather than from the client or the framebuffer. Any existing image arrays
**  associated with any mipmap levels in the texture object are freed (as if
**  TexImage was called for each, with an image of zero size).  As a result of
**  this referencing operation, all of the pixel data in the <buffer> used as
**  the EGLImage source resource (i.e., the <buffer> parameter passed to the
**  CreateImageOES command that returned <image>) will become undefined.
**
**  Currently, <target> must be TEXTURE_2D. <image> must be the handle of a
**  valid EGLImage resource, cast into the type eglImageOES. Assuming no errors
**  are generated in EGLImageTargetTexture2DOES, the newly specified texture
**  object will be an EGLImage target of the specified eglImageOES. If an
**  application later respecifies any image array in the texture object
**  (through mechanisms such as calls to TexImage2D and/or GenerateMipmapOES,
**  or setting the SGIS_GENERATE_MIPMAP parameter to TRUE), implementations
**  should allocate additional space for all specified (and respecified) image
**  arrays, and copy any existing image data to the newly (re)specified texture
**  object (as if TexImage was called for every level-of-detail in the texture
**  object). The respecified texture object will not be an EGLImage target.
**
**  If the GL is unable to specify a texture object using the supplied
**  eglImageOES <image> (if, for example, <image> refers to a multisampled
**  eglImageOES), the error INVALID_OPERATION is generated.
**
**  If <target> is not TEXTURE_2D, the error INVALID_ENUM is generated.
**
**  ERRORS:
**
**      GL_INVALID_ENUM
**          - if Target is not GL_TEXTURE_2D.
**
**      GL_INVALID_OPERATION
**          - if the specified image cannot be operated on;
**
**  INPUT:
**
**      Target
**          Specifies the target texture. Must be GL_TEXTURE_2D.
**
**      Image
**          Specifies the source eglImage handle, cast to GLeglImageOES.
**
**  OUTPUT:
**
**      Nothing.
*/

GL_API void GL_APIENTRY glEGLImageTargetTexture2DOES(
    GLenum Target,
    GLeglImageOES Image
    )
{
    glmENTER2(glmARGENUM, Target, glmARGPTR, Image)
    {
        gceSTATUS status;
        khrEGL_IMAGE_PTR image;
        glsEGL_IMAGE_ATTRIBUTES attributes;
        glsTEXTURESAMPLER_PTR sampler;
        glsTEXTUREWRAPPER_PTR texture = gcvNULL;
        gceSURF_FORMAT dstFormat;
        gctBOOL sourceYuv = gcvFALSE;
        gctBOOL planarYuv = gcvFALSE;
        gceSURF_TYPE srcType = gcvSURF_TYPE_UNKNOWN;
        gctBOOL resetTexture = gcvTRUE;
        gctBOOL dirty = gcvFALSE;

        gcmDUMP_API("${ES11 glEGLImageTargetTexture2DOES 0x%08X 0x%08X}", Target, Image);

        /* Validate arguments. */
        if ((Target != GL_TEXTURE_2D) && (Target != GL_TEXTURE_EXTERNAL_OES))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        /* Get the eglImage. */
        image = (khrEGL_IMAGE_PTR) Image;

        /* Get texture attributes from eglImage. */
        status = glfGetEGLImageAttributes(image, &attributes);

        if (gcmIS_ERROR(status))
        {
            glmERROR(GL_INVALID_OPERATION);
            break;
        }

        /* Check the texture size. */
        if ((attributes.width  == 0) ||
            (attributes.height == 0) ||
            (attributes.width  > context->maxTextureWidth) ||
            (attributes.height > context->maxTextureHeight))
        {
            glmERROR(GL_INVALID_VALUE);
            break;
        }

        /* Check if width and height is power of 2. */
        /* Here not fully follow GLES spec for gnash, which may use npot texture. */
        if (((attributes.width  & (attributes.width  - 1))  ||
             (attributes.height & (attributes.height - 1))) &&
             (attributes.level != 0))
        {
            glmERROR(GL_INVALID_VALUE);
            break;
        }

        if (image->update)
        {
            /* Update source sibling pixels. */
            dirty = image->update(image);
        }

        /* Get a shortcut to the active sampler. */
        sampler = context->texture.activeSampler;

        /* Get a shortcut to the bound texture. */
        switch (Target)
        {
        case GL_TEXTURE_2D:
            texture = sampler->bindings[glvTEXTURE2D];
            break;

        case GL_TEXTURE_EXTERNAL_OES:
            texture = sampler->bindings[glvTEXTUREEXTERNAL];
            break;

        default:
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        if (texture == gcvNULL)
        {
            break;
        }

        /**********************************************************************
        ** Validate the format.
        */
        switch (attributes.format)
        {
        case gcvSURF_YV12:
        case gcvSURF_I420:
        case gcvSURF_NV12:
        case gcvSURF_NV21:
        case gcvSURF_NV16:
        case gcvSURF_NV61:
            dstFormat = gcvSURF_YUY2;
            sourceYuv = gcvTRUE;
            planarYuv = gcvTRUE;
            break;

        case gcvSURF_YUY2:
        case gcvSURF_UYVY:
        case gcvSURF_YVYU:
        case gcvSURF_VYUY:
            dstFormat = attributes.format;
            sourceYuv = gcvTRUE;
            break;

        default:
            /* Get closest supported destination. */
            status = gcoTEXTURE_GetClosestFormat(context->hal,
                                                 attributes.format,
                                                 &dstFormat);
            break;
        }

        if (gcmIS_ERROR(status))
        {
            glmERROR(GL_INVALID_VALUE);
            break;
        }

        /* Make the compiler happy, disable warnings. */
        sourceYuv = sourceYuv;
        planarYuv = planarYuv;

        /* Save closest texture format. */
        texture->image.textureFormat = dstFormat;

        /* Get source surface type. */
        gcmVERIFY_OK(
            gcoSURF_GetFormat(attributes.surface, &srcType, gcvNULL));

        /* Test if can sample texture source directly. */
        texture->image.directSample = gcvFALSE;

        if (sourceYuv)
        {
            /* YUV class, should be linear. */
            gcmASSERT(srcType == gcvSURF_BITMAP);

            if (
                /* YUV assembler for planar yuv. */
                (planarYuv && context->hasYuvAssembler)
                /* linear texture for interleaved yuv. */
            ||  (!planarYuv && context->hasLinearTx)
            )
            {
                texture->image.directSample = gcvTRUE;
            }
        }
        else
        {
            /* RGB class. */
            if (
                /* linear texture. */
                (srcType == gcvSURF_BITMAP && context->hasLinearTx)
                /* tiled texture. */
            ||  (srcType == gcvSURF_TEXTURE)
                /* supertiled texture. */
            ||  (srcType == gcvSURF_RENDER_TARGET && context->hasSupertiledTx)
            )
            {
                /* Direct if format supported. */
                if (attributes.format == dstFormat)
                {
                    texture->image.directSample = gcvTRUE;
                }
                else if (context->hasTxSwizzle)
                {
                    switch (attributes.format)
                    {
                    case gcvSURF_A8B8G8R8:
                    case gcvSURF_X8B8G8R8:
                    case gcvSURF_A1B5G5R5:
                    case gcvSURF_X1B5G5R5:
                    case gcvSURF_A4B4G4R4:
                    case gcvSURF_X4B4G4R4:
                    case gcvSURF_B5G6R5:
                        /* Tx swizzle to support directly. */
                        texture->image.directSample = gcvTRUE;
                        break;

                    default:
                        break;
                    }
                }
            }

            /* More checks. */
            if (texture->image.directSample)
            {
                /* Check tile status. */
                if (!context->hasTxTileStatus &&
                    gcoSURF_IsTileStatusEnabled(attributes.surface))
                {
                    texture->image.directSample = gcvFALSE;
                }
                /* Check compression. */
                else if (!context->hasTxDecompressor &&
                    gcoSURF_IsCompressed(attributes.surface))
                {
                    texture->image.directSample = gcvFALSE;
                }
                /* Check multi-sample. */
                else
                {
                    gctUINT samples = 0;

                    gcmVERIFY_OK(
                        gcoSURF_GetSamples(attributes.surface, &samples));

                    if (samples > 1)
                    {
                        texture->image.directSample = gcvFALSE;
                    }
                }
            }
        }

        do
        {
            /* Test if need to reset texture wrapper. */
            gcoSURF mipmap = gcvNULL;

            gctUINT width;
            gctUINT height;
            gctUINT depth;
            gceSURF_FORMAT format;
            gceSURF_TYPE type;

            if (texture->object == gcvNULL)
            {
                /* No texture object, do nothing here. */
                resetTexture = gcvFALSE;

                break;
            }

            if (texture->direct.source != gcvNULL)
            {
                /* Formerly bound a GL_VIV_direct_texture. */
                break;
            }

            /* Mipmap level 1 is available, can not skip reset. */
            if (gcmIS_SUCCESS(
                    gcoTEXTURE_GetMipMap(texture->object, 1, &mipmap)))
            {
                /* This texture object has more than 1 levels, need destroy
                 * and re-create again. */
                break;
            }

            /* Get mipmap level 0. */
            if (gcmIS_ERROR(
                    gcoTEXTURE_GetMipMap(texture->object, 0, &mipmap)))
            {
                /* Can not get mipmap level 0, which means this texture
                 * object is clean. Destroy it. */
                break;
            }

            /* Query size and format. */
            gcmERR_BREAK(gcoSURF_GetSize(mipmap, &width, &height, &depth));

            gcmERR_BREAK(gcoSURF_GetFormat(mipmap, &type, &format));

            if ((width  != attributes.width)
            ||  (height != attributes.height)
            ||  (depth  != 1)
            )
            {
                /* Has different size. */
                break;
            }

            if (texture->image.directSample)
            {
                break;
            }
            else
            {
                if ((format != dstFormat) || (type != gcvSURF_TEXTURE))
                {
                    /* Has different dst parameters. */
                    break;
                }
            }

            /* All parameters are matched, we do not need re-create
             * another texture object. */
            resetTexture = gcvFALSE;
        }
        while (gcvFALSE);

        if (resetTexture)
        {
            /* Destroy the texture. */
            gcmVERIFY_OK(_ResetTextureWrapper(context, texture));

            /* Commit. */
            gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));

            /* Mark texture as as dirty because it will be re-constructed. */
            dirty = gcvTRUE;
        }

        /* Reference linear source surface. */
        if (texture->image.source != attributes.surface)
        {
            if (texture->image.source != gcvNULL)
            {
                /* Decrease reference count of old surface. */
                gcmVERIFY_OK(gcoSURF_Destroy(texture->image.source));

                /* Commit the destroy. */
                gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));
            }

            /* Reference new source surface. */
            texture->image.source = attributes.surface;

            if (texture->image.source != gcvNULL)
            {
                /* Increase reference count of new source. */
                gcmVERIFY_OK(gcoSURF_ReferenceSurface(texture->image.source));
            }

            /* Mark texture as dirty because source changed. */
            dirty = gcvTRUE;
        }

        /* Reference EGLImageKHR. */
        if (texture->image.image != image)
        {
            if (texture->image.image != gcvNULL)
            {
                /* Dereference old EGLImageKHR. */
                context->imports.dereferenceImage(texture->image.image);
            }

            /* Reference new EGLImageKHR. */
            texture->image.image = image;
            context->imports.referenceImage(image);
        }

#if defined(ANDROID) && (ANDROID_SDK_VERSION >= 7)
        if (image->type == KHR_IMAGE_ANDROID_NATIVE_BUFFER)
        {
            /* Reference android native buffer. */
            if (texture->image.nativeBuffer != attributes.nativeBuffer)
            {
                android_native_buffer_t * nativeBuffer;

                if (texture->image.nativeBuffer != gcvNULL)
                {
                    /* Cast to android native buffer. */
                    nativeBuffer = (android_native_buffer_t *) texture->image.nativeBuffer;

                    /* Decrease native buffer reference count. */
                    nativeBuffer->common.decRef(&nativeBuffer->common);
                }

                /* Reference new android native buffer. */
                texture->image.nativeBuffer = attributes.nativeBuffer;

                if (texture->image.nativeBuffer != gcvNULL)
                {
                    /* Cast to android native buffer. */
                    nativeBuffer = (android_native_buffer_t *) texture->image.nativeBuffer;

                    /* Increase reference count. */
                    nativeBuffer->common.incRef(&nativeBuffer->common);
                }
            }
        }
#endif

        if (dirty)
        {
            /* Store dirty flag. */
            texture->image.dirty = gcvTRUE;
        }

        /* TODO: No mipmap for EGLImage target texture for now. */
        texture->maxLevel = 0;

        /* Level 0 is special. */
        if (attributes.level == 0)
        {
            GLenum baseFormat;
            GLint tValue;

            /* Invalidate normalized crop rectangle. */
            texture->dirtyCropRect = GL_TRUE;

            /* Set texture parameters. */
            texture->width  = attributes.width;
            texture->height = attributes.height;

            /* Determine the number of mipmaps. */
            tValue=glfGetMaxLOD(texture->width, texture->height);
            texture->maxLevel = gcmMIN(tValue, texture->maxLevel);

            /* Update texture format. */
            _QueryImageBaseFormat(image, &baseFormat);
            _SetTextureWrapperFormat(context, texture, baseFormat);

        }
    }
    glmLEAVE();
}


/*******************************************************************************
**
**  glTexGenxOES (GL_OES_texture_cube_map)
**
**  Set of functions to configure the texture generation modes.
**
**  INPUT:
**
**      Coord
**          Specifies the type of coordinates for which the mode is to be
**          modified. Must be GL_TEXTURE_GEN_STR_OES (S, T, R coordinates).
**
**      Name
**          The name of the parameter that is being modified.
**          Must be GL_TEXTURE_GEN_MODE_OES.
**
**      Param, Params
**          The new value for the parameter. Can be either one of the following:
**              GL_NORMAL_MAP_OES
**              GL_REFLECTION_MAP_OES
**
**  OUTPUT:
**
**      Nothing.
*/

GL_API void GL_APIENTRY glTexGenfOES(
    GLenum Coord,
    GLenum Name,
    GLfloat Param
    )
{
    glmENTER3(glmARGENUM, Coord, glmARGENUM, Name,
              glmARGFLOAT, Param)
    {
        if (!_SetTexGen(context, Coord, Name, &Param))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glTexGenfvOES(
    GLenum Coord,
    GLenum Name,
    const GLfloat * Params
    )
{
    glmENTER3(glmARGENUM, Coord, glmARGENUM, Name,
              glmARGPTR, Params)
    {
        if (!_SetTexGen(context, Coord, Name, Params))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glTexGeniOES(
    GLenum Coord,
    GLenum Name,
    GLint Param
    )
{
    glmENTER3(glmARGENUM, Coord, glmARGENUM, Name,
              glmARGFLOAT, Param)
    {
        /* Convert to float */
        GLfloat param = glmINT2FLOAT(Param);

        if (!_SetTexGen(context, Coord, Name, &param))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glTexGenivOES(
    GLenum Coord,
    GLenum Name,
    const GLint * Params
    )
{
    glmENTER3(glmARGENUM, Coord, glmARGENUM, Name, glmARGPTR, Params)
    {
        /* Convert to float */
        GLfloat params[1];
        params[0] = glmINT2FLOAT(Params[0]);

        if (!_SetTexGen(context, Coord, Name, params))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glTexGenxOES(
    GLenum Coord,
    GLenum Name,
    GLfixed Param
    )
{
    glmENTER3(glmARGENUM, Coord, glmARGENUM, Name, glmARGFLOAT, Param)
    {
        /* Convert to float */
        GLfloat param = (GLfloat) Param;

        if (!_SetTexGen(context, Coord, Name, &param))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glTexGenxvOES(
    GLenum Coord,
    GLenum Name,
    const GLfixed * Params
    )
{
    glmENTER3(glmARGENUM, Coord, glmARGENUM, Name, glmARGPTR, Params)
    {
        /* Convert to float */
        GLfloat params[1];
        params[0] = (GLfloat) Params[0];

        if (!_SetTexGen(context, Coord, Name, params))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }
    }
    glmLEAVE();
}


/*******************************************************************************
**
**  glGetTexGenxOES (GL_OES_texture_cube_map)
**
**  Set of functions to retrieve the texture generation modes.
**
**  INPUT:
**
**      Coord
**          Specifies the type of coordinates for which the mode is to be
**          modified. Must be GL_TEXTURE_GEN_STR_OES (S, T, R coordinates).
**
**      Name
**          The name of the parameter that is being modified.
**          Must be GL_TEXTURE_GEN_MODE_OES.
**
**      Params
**          The pointer to the return value.
**
**  OUTPUT:
**
**      Params
**          The requested state value.
*/

GL_API void GL_APIENTRY glGetTexGenfvOES(
    GLenum Coord,
    GLenum Name,
    GLfloat * Params
    )
{
    glmENTER3(glmARGENUM, Coord, glmARGENUM, Name, glmARGPTR, Params)
    {
        if (!_GetTexGen(context, Coord, Name, Params, glvFLOAT))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glGetTexGenivOES(
    GLenum Coord,
    GLenum Name,
    GLint * Params
    )
{
    glmENTER3(glmARGENUM, Coord, glmARGENUM, Name, glmARGPTR, Params)
    {
        if (!_GetTexGen(context, Coord, Name, Params, glvINT))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }
    }
    glmLEAVE();
}

GL_API void GL_APIENTRY glGetTexGenxvOES(
    GLenum Coord,
    GLenum Name,
    GLfixed * Params
    )
{
    glmENTER3(glmARGENUM, Coord, glmARGENUM, Name, glmARGPTR, Params)
    {
        if (!_GetTexGen(context, Coord, Name, Params, glvFIXED))
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }
    }
    glmLEAVE();
}


/*******************************************************************************
**
**  glGenerateMipmapOES (OES_framebuffer_object)
**
**  Generate mipmap for spcific texture.
**
**  Arguments:
**
**      GLenum Target
**          Specifies the target texture.
**
**  OUTPUT:
**
**      Nothing.
**
*/

GL_API void GL_APIENTRY glGenerateMipmapOES(
    GLenum Target
    )
{
    gceSTATUS status;

    glmENTER1(glmARGENUM, Target)
    {
        gctINT faces;
        gceSURF_FORMAT format;
        glsTEXTURESAMPLER_PTR sampler;
        glsTEXTUREWRAPPER_PTR texture;
        gcoSURF surface;
        gctUINT width, height;

        gcmDUMP_API("${ES11 glGenerateMipmapOES 0x%08X}", Target);

        /* Get a shortcut to the active sampler. */
        sampler = context->texture.activeSampler;

        switch (Target)
        {
        case GL_TEXTURE_2D:
            texture = sampler->bindings[glvTEXTURE2D];
            faces   = 0;
            break;

        case GL_TEXTURE_CUBE_MAP_OES:
            texture = sampler->bindings[glvCUBEMAP];
            faces   = 6;
            break;

        default:
        gcmFATAL("glGenerateMipmap: Invalid target: %04X", Target);
        glmERROR(GL_INVALID_ENUM);
        goto OnError;
        }

        gcmASSERT(texture != gcvNULL);

        /* Make sure the texture is not empty. */
        if (texture->object == gcvNULL)
        {
        gcmTRACE(gcvLEVEL_WARNING,
                    "glGenerateMipMap: No texture object created for target %04X",
                    Target);

            glmERROR(GL_INVALID_OPERATION);
            goto OnError;
        }

        gcmERR_BREAK(gcoTEXTURE_GetMipMap(texture->object,
                                        0,
                                        &surface));

        gcmERR_BREAK(gcoSURF_GetFormat(surface, gcvNULL, &format));

        gcmERR_BREAK(gcoSURF_GetSize(surface, &width, &height, gcvNULL));

        gcmERR_BREAK(glfGenerateMipMaps(
                context, texture, format, 0, width, height, faces
                ));

        /* Easy return on error. */
OnError:;
    }
    glmLEAVE();
}
