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


#include "gc_egl_precomp.h"

/* Zone used for header/footer. */
#define _GC_OBJ_ZONE    gcdZONE_EGL_IMAGE

#if defined(ANDROID)
#if ANDROID_SDK_VERSION >= 16
#    include <ui/ANativeObjectBase.h>
#  else
#    include <private/ui/android_natives_priv.h>
#  endif
#endif

#ifdef LINUX
#  include <drm_fourcc.h>
#endif

static VEGLImage
_InitializeImage(
    VEGLThreadData Thread,
    VEGLDisplay Dpy,
    VEGLContext ctx
    )
{
    VEGLImage image;
    gceSTATUS status;
    gcsATOM_PTR atom = gcvNULL;
    gctPOINTER mutex = gcvNULL;
    gctPOINTER pointer = gcvNULL;

    /* Allocate the surface structure. */
    status = gcoOS_Allocate(gcvNULL,
                            sizeof(struct eglImage),
                            &pointer);

    if (!gcmIS_SUCCESS(status))
    {
        /* Out of memory. */
        veglSetEGLerror(Thread, EGL_BAD_ALLOC);
        return gcvNULL;
    }

    /* Allocate access lock. */
    status = gcoOS_CreateMutex(gcvNULL, &mutex);

    if (gcmIS_ERROR(status))
    {
        gcmOS_SAFE_FREE(gcvNULL, pointer);

        /* Out of memory. */
        veglSetEGLerror(Thread, EGL_BAD_ALLOC);
        return gcvNULL;
    }

    /* Allocate reference atom. */
    status = gcoOS_AtomConstruct(gcvNULL, &atom);

    if (gcmIS_ERROR(status))
    {
        gcoOS_DeleteMutex(gcvNULL, mutex);
        gcmOS_SAFE_FREE(gcvNULL, pointer);

        /* Out of memory. */
        veglSetEGLerror(Thread, EGL_BAD_ALLOC);
        return gcvNULL;
    }

    image = pointer;

    /* Initialize the image object. */
    image->signature        = EGL_IMAGE_SIGNATURE;
    image->display          = Dpy;
    image->destroyed        = EGL_FALSE;
    image->reference        = atom;
    image->protectedContent = EGL_FALSE;
    image->next             = gcvNULL;

    /* Initialize reference count. */
    gcoOS_AtomSet(gcvNULL, image->reference, 1);

    gcoOS_MemFill(&(image->image), 0, sizeof(image->image));
    image->image.mutex      = mutex;

    return image;
}

static void
_FinalizeImage(
    VEGLThreadData Thread,
    VEGLDisplay Dpy,
    VEGLImage Image
    )
{
    /* Destroy reference atom. */
    gcmVERIFY_OK(gcoOS_AtomDestroy(gcvNULL, Image->reference));
    Image->reference = gcvNULL;

    /* Delete the mutex. */
    gcoOS_DeleteMutex(gcvNULL, Image->image.mutex);
    Image->image.mutex = gcvNULL;

    /* Free the eglImage structure. */
    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, Image));
}

static void
_DestroyImage(
    VEGLThreadData Thread,
    VEGLDisplay Display,
    VEGLImage Image,
    EGLBoolean FromTerminate
    )
{
    if (Image->image.surface != gcvNULL)
    {
        if (Image->image.type == KHR_IMAGE_PIXMAP)
        {
            VEGLImageRef ref = gcvNULL, previous = gcvNULL;

            /* Find the surface in the reference stack. */
            for (ref = Display->imageRefStack;
                 ref != gcvNULL;
                 ref = ref->next)
            {
                /* See if the surface matches. */
                if (ref->surface == Image->image.surface)
                {
                    break;
                }

                /* Save current pointer of the linked list. */
                previous = ref;
            }

            /* If we have a valid reference and the reference count has
            ** reached 1, we can remove the surface from the reference
            ** stack. */
            if (ref != gcvNULL)
            {
                /* Unlink the reference from the linked list. */
                if (previous == gcvNULL)
                {
                    Display->imageRefStack = ref->next;
                }
                else
                {
                    if (previous->next != ref->next)
                    {
                        previous->next = ref->next;
                    }
                }

                /* Free the structure. */
                gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, ref));
            }
        }

        if (Image->image.type == KHR_IMAGE_PIXMAP &&
            Image->image.u.pixmap.nativePixmap)
        {
            void * pixmap = Image->image.u.pixmap.nativePixmap;
            VEGLPixmapInfo info =
                    (VEGLPixmapInfo) Image->image.u.pixmap.pixInfo;

            Display->platform->disconnectPixmap(Display, (void *) pixmap, info);
            Image->image.u.pixmap.nativePixmap = gcvNULL;
        }

        /* Destroy the surface. */
        gcoSURF_Destroy(Image->image.surface);
        Image->image.surface = gcvNULL;

        if (Image->image.srcSurface)
        {
            gcoSURF_Destroy(Image->image.srcSurface);
            Image->image.srcSurface = gcvNULL;
        }

        if ((Image->image.type == KHR_IMAGE_TEXTURE_CUBE) &&
            (Image->image.u.texture.shadowSurface))
        {
            gcoSURF_Destroy(Image->image.u.texture.shadowSurface);
            Image->image.u.texture.shadowSurface = gcvNULL;
        }
    }

#if defined(ANDROID)
    /* Clean up android native eglImage. */
    if ((Image->image.type == KHR_IMAGE_ANDROID_NATIVE_BUFFER) &&
        (Image->image.u.ANativeBuffer.nativeBuffer != gcvNULL))
    {
        gctPOINTER buffer;
        android_native_buffer_t * nativeBuffer;

        /* Cast to android native buffer. */
        buffer = Image->image.u.ANativeBuffer.nativeBuffer;
        nativeBuffer = (android_native_buffer_t *) buffer;

        /* Decrease native buffer reference count. */
        nativeBuffer->common.decRef(&nativeBuffer->common);
        Image->image.u.ANativeBuffer.nativeBuffer = gcvNULL;
    }
#endif

    /* Commit accumulated commands. */
    gcoHAL_Commit(gcvNULL, gcvFALSE);

    /* Finalize. */
    _FinalizeImage(Thread, Display, Image);
}

void
veglDestroyImage(
    IN VEGLThreadData Thread,
    IN VEGLDisplay Display,
    IN VEGLImage Image
    )
{
    if (Image != EGL_NO_IMAGE)
    {
        gctINT32 oldValue = 0;

        /* Decrement reference. */
        gcmVERIFY_OK(gcoOS_AtomDecrement(gcvNULL, Image->reference, &oldValue));

        if (oldValue == 1)
        {
            /* Destroy image. */
            _DestroyImage(Thread, Display, Image, EGL_TRUE);
        }
    }
}

void
veglReferenceImage(
    IN VEGLThreadData Thread,
    IN VEGLDisplay Display,
    IN VEGLImage Image
    )
{
    /* Can not reference destroyed EGLImage. */
    if (!Image->destroyed)
    {
        /* Increment reference. */
        gcmVERIFY_OK(gcoOS_AtomIncrement(gcvNULL, Image->reference, gcvNULL));
    }
}

void
veglDereferenceImage(
    IN VEGLThreadData Thread,
    IN VEGLDisplay Display,
    IN VEGLImage Image
    )
{
    gctINT32 oldValue = 0;

    /* Decrement reference. */
    gcmVERIFY_OK(gcoOS_AtomDecrement(gcvNULL, Image->reference, &oldValue));

    if (oldValue == 1)
    {
        /* Destroy image. */
        _DestroyImage(Thread, Display, Image, EGL_FALSE);
    }
}

static gcmINLINE EGLAttrib
_AttribValue(
    const void *attrib_list,
    EGLBoolean intAttrib,
    EGLint index
    )
{
    const EGLint * intList       = (const EGLint *) attrib_list;
    const EGLAttrib * attribList = (const EGLAttrib *) attrib_list;

    return intAttrib ? (EGLAttrib) intList[index]
                     : attribList[index];
}

static VEGLImage
_CreateImageTex2D(
    VEGLThreadData Thread,
    VEGLDisplay Dpy,
    VEGLContext Ctx,
    EGLClientBuffer Buffer,
    const void *attrib_list,
    EGLBoolean intAttrib
    )
{
    VEGLImage   image;

    gctINT      texture;
    gctINT      level  = 0;
    gctINT      depth  = 0;

    EGLenum     status;
    EGLBoolean  protectedContent = EGL_FALSE;

    /* Test if ctx is null. */
    if (Ctx == gcvNULL)
    {
        veglSetEGLerror(Thread,  EGL_BAD_CONTEXT);
        return EGL_NO_IMAGE;
    }

    /* Test if ctx is valid */
    if ( (Ctx->api      != Thread->api) ||
         (Ctx->api      != EGL_OPENGL_ES_API) ||
         (Ctx->display  != Dpy))
    {
        veglSetEGLerror(Thread, EGL_BAD_MATCH);
        return EGL_NO_IMAGE;
    }

    /* Cast to texture ojbect. */
    texture = gcmPTR2INT32(Buffer);

    /* Texture must be nonzero. */
    if (texture == 0)
    {
        veglSetEGLerror(Thread,  EGL_BAD_PARAMETER);;
        return EGL_NO_IMAGE;
    }

    /* Parse the attribute list. */
    if (attrib_list != gcvNULL)
    {
        EGLint i;

        for (i = 0; _AttribValue(attrib_list, intAttrib, i) != EGL_NONE; i += 2)
        {
            switch (_AttribValue(attrib_list, intAttrib, i))
            {
            case EGL_GL_TEXTURE_LEVEL_KHR:
                level = (gctINT) _AttribValue(attrib_list, intAttrib, i + 1);
                break;

            case EGL_IMAGE_PRESERVED_KHR:
                break;

            case EGL_PROTECTED_CONTENT_EXT:
                if(_AttribValue(attrib_list, intAttrib, i + 1) == EGL_TRUE)
                {
                    protectedContent = EGL_TRUE;
                }
                break;

            default:
                /* Ignore unacceptable attributes. */
                gcmTRACE_ZONE(
                    gcvLEVEL_ERROR, gcdZONE_EGL_IMAGE,
                    "%s(%d): Unknown attribute 0x%04X = 0x%04X.",
                    __FUNCTION__, __LINE__,
                    _AttribValue(attrib_list, intAttrib, i),
                    _AttribValue(attrib_list, intAttrib, i + 1)
                    );
                veglSetEGLerror(Thread,  EGL_BAD_PARAMETER);;
                return EGL_NO_IMAGE;
            }
        }
    }

    if (level < 0)
    {
        veglSetEGLerror(Thread, EGL_BAD_MATCH);
        return EGL_NO_IMAGE;
    }

    /* Initialize an image struct. */
    image = _InitializeImage(Thread, Dpy, Ctx);

    if (!image)
    {
        veglSetEGLerror(Thread,  EGL_BAD_ALLOC);
        return EGL_NO_IMAGE;
    }

    image->protectedContent = protectedContent;

    /* Create eglImage from a texture object. */
    status  = _CreateImageTexture(Thread, Ctx, EGL_GL_TEXTURE_2D_KHR, texture,
                                    level, depth, image);

    /* Clean up if error happen. */
    if (status != EGL_SUCCESS)
    {
        _FinalizeImage(Thread, Dpy, image);

        veglSetEGLerror(Thread, status);
        return EGL_NO_IMAGE;
    }

    if (image->image.surface)
    {
        /* Reference the surface. */
        gcoSURF_ReferenceSurface(image->image.surface);
    }

    return image;
}

static VEGLImage
_CreateImageTexCube(
    VEGLThreadData Thread,
    VEGLDisplay Dpy,
    VEGLContext Ctx,
    EGLenum Target,
    EGLClientBuffer Buffer,
    const void *attrib_list,
    EGLBoolean intAttrib
    )
{
    VEGLImage   image;

    gctINT      texture;
    gctINT      level  = 0;
    gctINT      depth  = 0;

    EGLenum     status;
    EGLBoolean  protectedContent = EGL_FALSE;

    /* Test if ctx is null. */
    if (Ctx == gcvNULL)
    {
        veglSetEGLerror(Thread,  EGL_BAD_CONTEXT);
        return EGL_NO_IMAGE;
    }

    /* Test if ctx is valid */
    if ( (Ctx->api      != Thread->api) ||
         (Ctx->api      != EGL_OPENGL_ES_API) ||
         (Ctx->display  != Dpy))
    {
        veglSetEGLerror(Thread, EGL_BAD_MATCH);
        return EGL_NO_IMAGE;
    }

    /* Cast to texture ojbect. */
    texture = gcmPTR2INT32(Buffer);

    /* Texture must be nonzero. */
    if (texture == 0)
    {
        veglSetEGLerror(Thread,  EGL_BAD_PARAMETER);;
        return EGL_NO_IMAGE;
    }

    /* Parse the attribute list. */
    if (attrib_list != gcvNULL)
    {
        EGLint i;

        for (i = 0; _AttribValue(attrib_list, intAttrib, i) != EGL_NONE; i += 2)
        {
            EGLint attribute = (EGLint) _AttribValue(attrib_list, intAttrib, i);
            EGLint value     = (EGLint) _AttribValue(attrib_list, intAttrib, i + 1);

            switch(attribute)
            {
            case EGL_GL_TEXTURE_LEVEL_KHR:
                level = value;
                break;

            case EGL_IMAGE_PRESERVED_KHR:
                break;

            case EGL_PROTECTED_CONTENT_EXT:
                if(value == EGL_TRUE)
                {
                    protectedContent = EGL_TRUE;
                }
                break;

            default:
                /* Ignore unacceptable attributes. */
                gcmTRACE_ZONE(
                    gcvLEVEL_ERROR, gcdZONE_EGL_IMAGE,
                    "%s(%d): Unknown attribute 0x%04X = 0x%04X.",
                    __FUNCTION__, __LINE__,
                    attribute, value
                    );
                veglSetEGLerror(Thread,  EGL_BAD_PARAMETER);;
                return EGL_NO_IMAGE;
            }
        }
    }

    if (level < 0)
    {
        veglSetEGLerror(Thread, EGL_BAD_MATCH);
        return EGL_NO_IMAGE;
    }

    /* Initialize an image struct. */
    image = _InitializeImage(Thread, Dpy, Ctx);

    if (!image)
    {
        veglSetEGLerror(Thread,  EGL_BAD_ALLOC);
        return EGL_NO_IMAGE;
    }

    image->protectedContent = protectedContent;

    /* Create eglImage from a texture object. */
    status  = _CreateImageTexture(Thread, Ctx, Target, texture,
                                    level, depth, image);

    /* Clean up if error happen. */
    if (status != EGL_SUCCESS)
    {
        _FinalizeImage(Thread, Dpy, image);

        veglSetEGLerror(Thread, status);
        return EGL_NO_IMAGE;
    }

    if (image->image.surface)
    {
        /* Reference the surface. */
        gcoSURF_ReferenceSurface(image->image.surface);
    }

    return image;
}

#if gcdENABLE_3D
static gctBOOL
_UpdatePixmap(
    khrEGL_IMAGE * Image
    )
{
    gcmASSERT(Image->type == KHR_IMAGE_PIXMAP);

    if (Image->surface && Image->u.pixmap.nativePixmap)
    {
        VEGLImage image = (VEGLImage) Image;
        void * pixmap =  Image->u.pixmap.nativePixmap;
        VEGLPixmapInfo info = (VEGLPixmapInfo) Image->u.pixmap.pixInfo;

        image->display->platform->syncFromPixmap(pixmap, info);
    }

    /* Always changed. */
    return gcvTRUE;
}
#endif

static VEGLImage
_CreateImagePixmap(
    VEGLThreadData Thread,
    VEGLDisplay Dpy,
    VEGLContext Ctx,
    EGLClientBuffer Buffer,
    const void *attrib_list,
    EGLBoolean intAttrib
    )
{
#if gcdENABLE_3D
    VEGLImage        image;
    void *           pixmap;
    VEGLPixmapInfo   info = gcvNULL;
    EGLint           width = 0, height = 0;
    gcoSURF          surface = gcvNULL;
    gceSTATUS        status;
    VEGLImageRef     ref;
    EGLBoolean       protectedContent = EGL_FALSE;

    /* Pixmap require context is EGL_NO_CONTEXT. */
    if (Ctx != EGL_NO_CONTEXT)
    {
        veglSetEGLerror(Thread,  EGL_BAD_PARAMETER);;
        return EGL_NO_IMAGE;
    }

    /* Parse the attribute list. */
    if (attrib_list != gcvNULL)
    {
        EGLint i;

        for (i = 0; _AttribValue(attrib_list, intAttrib, i) != EGL_NONE; i += 2)
        {
            EGLint attribute = (EGLint) _AttribValue(attrib_list, intAttrib, i);
            EGLint value     = (EGLint) _AttribValue(attrib_list, intAttrib, i + 1);

            switch(attribute)
            {
            case EGL_IMAGE_PRESERVED_KHR:
                break;

            case EGL_PROTECTED_CONTENT_EXT:
                if(value == EGL_TRUE)
                {
                    protectedContent = EGL_TRUE;
                }
                break;

            default:
                /* Ignore unacceptable attributes. */
                gcmTRACE_ZONE(
                    gcvLEVEL_ERROR, gcdZONE_EGL_IMAGE,
                    "%s(%d): Unknown attribute 0x%04X = 0x%04X.",
                    __FUNCTION__, __LINE__,
                    attribute, value
                    );
                veglSetEGLerror(Thread,  EGL_BAD_PARAMETER);;
                return EGL_NO_IMAGE;
            }
        }
    }

    /* Cast buffer to native pixmap type. */
    pixmap = (void *) Buffer;

    /* Check if has eglImage created from this pixmap. */
    for (ref = Dpy->imageRefStack; ref != gcvNULL; ref = ref->next)
    {
        if ((ref->pixmap == pixmap)
        &&  (ref->surface != gcvNULL)
        )
        {
            gctINT32 refCount = 0;

            status = gcoSURF_QueryReferenceCount(ref->surface, &refCount);
            if (gcmIS_SUCCESS(status) && (refCount > 1))
            {
                veglSetEGLerror(Thread,  EGL_BAD_ACCESS);
                return EGL_NO_IMAGE;
            }

            break;
        }
    }

    /* Initialize an image struct. */
    image = _InitializeImage(Thread, Dpy, Ctx);

    if (!image)
    {
        veglSetEGLerror(Thread,  EGL_BAD_ALLOC);
        return EGL_NO_IMAGE;
    }

    image->protectedContent = protectedContent;

    if (ref == gcvNULL)
    {
        gctPOINTER pointer;

        /* According to extension EGL_KHR_image_pixmap:
        ** If <target> is EGL_NATIVE_PIXMAP_KHR and <buffer> is not a
        ** valid native pixmap handle, or if <buffer> is a native pixmap
        ** whose color buffer format is incompatible with the system's
        ** EGLImage implementation, the error EGL_BAD_PARAMETER is generated.
        */
        if (!Dpy->platform->connectPixmap(Dpy, pixmap, &info, &surface))
        {
            veglSetEGLerror(Thread,  EGL_BAD_PARAMETER);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        if (!Dpy->platform->getPixmapSize(Dpy, pixmap, info, &width, &height))
        {
            veglSetEGLerror(Thread,  EGL_BAD_ACCESS);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        /* Sync pixels. */
        Dpy->platform->syncFromPixmap(pixmap, info);

        gcmONERROR(gcoOS_Allocate(gcvNULL,
                                  gcmSIZEOF(struct eglImageRef),
                                  &pointer));

        ref = pointer;

        /* Initialize the reference. */
        ref->pixmap       = pixmap;
        ref->pixInfo      = info;
        ref->surface      = surface;

        /* Push it to the stack. */
        ref->next = Dpy->imageRefStack;
        Dpy->imageRefStack = ref;
    }
    else
    {
        surface = ref->surface;
        pixmap  = ref->pixmap;
        info    = ref->pixInfo;
    }

    image->image.magic   = KHR_EGL_IMAGE_MAGIC_NUM;
    image->image.type    = KHR_IMAGE_PIXMAP;

    image->image.surface = surface;
    image->image.update  = _UpdatePixmap;

    image->image.u.pixmap.nativePixmap = pixmap;
    image->image.u.pixmap.pixInfo      = info;
    image->image.u.pixmap.width        = (gctUINT) width;
    image->image.u.pixmap.height       = (gctUINT) height;

    if (image->image.surface)
    {
        /* Reference the surface. */
        gcoSURF_ReferenceSurface(image->image.surface);
    }

#ifdef EGL_API_XXX
    /* Reset the sequence NO. */
    image->image.u.pixmap.seqNo = 0;
#endif
    return image;

OnError:
    if (info)
    {
        Dpy->platform->disconnectPixmap(Dpy, pixmap, info);
        info = gcvNULL;
    }

    _FinalizeImage(Thread, Dpy, image);

    return EGL_NO_IMAGE;
#else
    return EGL_NO_IMAGE;
#endif
}

static VEGLImage
_CreateImageRenderBuffer(
    VEGLThreadData Thread,
    VEGLDisplay Dpy,
    VEGLContext Ctx,
    EGLClientBuffer Buffer,
    const void *attrib_list,
    EGLBoolean intAttrib
    )
{
    VEGLImage   image;
    gctINT      renderbuffer;
    EGLenum     status;
    EGLBoolean  protectedContent = EGL_FALSE;

    /* Test if ctx is null. */
    if (Ctx == gcvNULL)
    {
        veglSetEGLerror(Thread,  EGL_BAD_CONTEXT);
        return EGL_NO_IMAGE;
    }

    /* Test if ctx is valid */
    if ( (Ctx->api      != Thread->api) ||
         (Ctx->api      != EGL_OPENGL_ES_API) ||
         (Ctx->display  != Dpy))
    {
        veglSetEGLerror(Thread, EGL_BAD_MATCH);
        return EGL_NO_IMAGE;
    }

    /* Cast to framebuffer name. */
    renderbuffer = gcmPTR2INT32(Buffer);

    if (renderbuffer == 0)
    {
        veglSetEGLerror(Thread, EGL_BAD_PARAMETER);
        return EGL_NO_IMAGE;
    }

    /* Parse the attribute list. */
    if (attrib_list != gcvNULL)
    {
        EGLint i;

        for (i = 0; _AttribValue(attrib_list, intAttrib, i) != EGL_NONE; i += 2)
        {
            EGLint attribute = (EGLint) _AttribValue(attrib_list, intAttrib, i);
            EGLint value     = (EGLint) _AttribValue(attrib_list, intAttrib, i + 1);

            switch(attribute)
            {
            case EGL_IMAGE_PRESERVED_KHR:
                break;

            case EGL_PROTECTED_CONTENT_EXT:
                if (value == EGL_TRUE)
                {
                    protectedContent = EGL_TRUE;
                }
                break;

            default:
                /* Ignore unacceptable attributes. */
                gcmTRACE_ZONE(
                    gcvLEVEL_ERROR, gcdZONE_EGL_IMAGE,
                    "%s(%d): Unknown attribute 0x%04X = 0x%04X.",
                    __FUNCTION__, __LINE__,
                    attribute, value
                    );
                veglSetEGLerror(Thread,  EGL_BAD_PARAMETER);;
                return EGL_NO_IMAGE;
            }
        }
    }

    /* Initialize an image struct. */
    image = _InitializeImage(Thread, Dpy, Ctx);

    if (!image)
    {
        veglSetEGLerror(Thread,  EGL_BAD_ALLOC);
        return EGL_NO_IMAGE;
    }

    image->protectedContent = protectedContent;

    /* Create from a framebuffer object. */
    status = _CreateImageFromRenderBuffer(Thread, Ctx, renderbuffer, image);

    /* Clean up if error happen. */
    if (status != EGL_SUCCESS)
    {
        _FinalizeImage(Thread, Dpy, image);

        veglSetEGLerror(Thread, status);
        return EGL_NO_IMAGE;
    }

    if (image->image.surface)
    {
        /* Reference the surface. */
        gcoSURF_ReferenceSurface(image->image.surface);
    }

    return image;
}

#if defined(ANDROID)
#if gcdDRM_GRALLOC
static EGLBoolean
_GetANativeBufferSurface(
    android_native_buffer_t * Buffer,
    gcoSURF * Surface
    )
{
    return EGL_FALSE;
}

#  else
#    include <gc_gralloc_priv.h>
static EGLBoolean
_GetANativeBufferSurface(
    android_native_buffer_t * Buffer,
    gcoSURF * Surface
    )
{
    gcoSURF surface;

    if (!Buffer->handle)
    {
        return EGL_FALSE;
    }

    surface = (gcoSURF)gcmUINT64_TO_PTR(
                    gc_native_handle_get(Buffer->handle)->surface);

    if (!surface)
    {
        return EGL_FALSE;
    }

    gcoSURF_ReferenceSurface(surface);

    *Surface = surface;
    return EGL_TRUE;
}
#  endif

static VEGLImage
_CreateImageANativeBuffer(
    VEGLThreadData Thread,
    VEGLDisplay Dpy,
    VEGLContext Ctx,
    EGLClientBuffer Buffer,
    const void *attrib_list,
    EGLBoolean intAttrib
    )
{
    VEGLImage image;
    EGLenum status;

    /* Context must be null. */
    if (Ctx != gcvNULL)
    {
        veglSetEGLerror(Thread,  EGL_BAD_CONTEXT);
        return EGL_NO_IMAGE;
    }

    /* Initialize an image struct. */
    image = _InitializeImage(Thread, Dpy, Ctx);

    if (image == gcvNULL)
    {
        veglSetEGLerror(Thread,  EGL_BAD_ALLOC);;
        return EGL_NO_IMAGE;
    }

    status = _CreateImageFromANativeBuffer(Thread, Buffer, image);

    /* Clean up if error happen. */
    if (status != EGL_SUCCESS)
    {
        _FinalizeImage(Thread, Dpy, image);

        veglSetEGLerror(Thread, status);
        return EGL_NO_IMAGE;
    }

    return image;
}
#endif

static VEGLImage
_CreateImageVGParent(
    VEGLThreadData Thread,
    VEGLDisplay Dpy,
    VEGLContext Ctx,
    EGLClientBuffer Buffer,
    const void *attrib_list,
    EGLBoolean intAttrib
    )
{
    VEGLImage       image;
    gctUINT    vgimage_obj;
    EGLenum         status;

    /* Test if ctx is null. */
    if (Ctx == gcvNULL)
    {
        veglSetEGLerror(Thread,  EGL_BAD_CONTEXT);
        return EGL_NO_IMAGE;
    }

    /* Test if ctx is a valid EGL context. */
    if ( (Ctx->api      != Thread->api) ||
         (Ctx->display  != Dpy))
    {
        veglSetEGLerror(Thread,  EGL_BAD_CONTEXT);
        return EGL_NO_IMAGE;
    }

    /* Test if ctx is a valid OpenVG context. */
    if ((Ctx->api != EGL_OPENVG_API))
    {
        veglSetEGLerror(Thread, EGL_BAD_MATCH);
        return EGL_NO_IMAGE;
    }

    /* Cast to a vgImage object handle. */
    vgimage_obj = gcmPTR2INT32(Buffer);

    if (vgimage_obj == 0)
    {
        veglSetEGLerror(Thread,  EGL_BAD_PARAMETER);;
        return EGL_NO_IMAGE;
    }

    /* Parse the attribute list. */
    if (attrib_list != gcvNULL)
    {
        EGLint i;

        for (i = 0; _AttribValue(attrib_list, intAttrib, i) != EGL_NONE; i += 2)
        {
            EGLint attribute = (EGLint) _AttribValue(attrib_list, intAttrib, i);
            EGLint value     = (EGLint) _AttribValue(attrib_list, intAttrib, i + 1);

            switch(attribute)
            {
            case EGL_IMAGE_PRESERVED_KHR:
                (void) value;
                break;

            default:
                /* Ignore unacceptable attributes. */
                gcmTRACE_ZONE(
                    gcvLEVEL_ERROR, gcdZONE_EGL_IMAGE,
                    "%s(%d): Unknown attribute 0x%04X = 0x%04X.",
                    __FUNCTION__, __LINE__,
                    attribute, value
                    );

                veglSetEGLerror(Thread,  EGL_BAD_PARAMETER);;
                return EGL_NO_IMAGE;
            }
        }
    }

    /* Initialize an image struct. */
    image = _InitializeImage(Thread, Dpy, Ctx);

    if (!image)
    {
        veglSetEGLerror(Thread,  EGL_BAD_ALLOC);
        return EGL_NO_IMAGE;
    }

    /* Create from a openvg image. */
    status = _CreateImageFromVGParentImage(Thread, Ctx, vgimage_obj, image);

    /* Clean up if error happen. */
    if (status != EGL_SUCCESS)
    {
        _FinalizeImage(Thread, Dpy, image);

        veglSetEGLerror(Thread, status);
        return EGL_NO_IMAGE;
    }

    if (image->image.surface)
    {
        /* Reference the surface. */
        gcoSURF_ReferenceSurface(image->image.surface);
    }

    return image;
}

#ifdef EGL_WAYLAND_BUFFER_WL
static VEGLImage
_CreateImageWL(
    VEGLThreadData Thread,
    VEGLDisplay Dpy,
    VEGLContext Ctx,
    EGLClientBuffer Buffer,
    const void *attrib_list,
    EGLBoolean intAttrib
    )
{
    VEGLImage image = gcvNULL;

#if defined(WL_EGL_PLATFORM)
    gctUINT width  = 0;
    gctUINT height = 0;
    gcoSURF surface = gcvNULL;

    /* Context must be null. */
    if (Ctx != gcvNULL)
    {
        veglSetEGLerror(Thread,  EGL_BAD_CONTEXT);
        return EGL_NO_IMAGE_KHR;
    }

    if (veglQueryWaylandBuffer(Dpy,
                               Buffer,
                               (EGLint *) &width,
                               (EGLint *) &height,
                               &surface) != EGL_TRUE)
    {
        veglSetEGLerror(Thread,  EGL_BAD_PARAMETER);
        return EGL_NO_IMAGE_KHR;
    }

    /* Initialize an image struct. */
    image = _InitializeImage(Thread, Dpy, Ctx);

    if (image == gcvNULL)
    {
        veglSetEGLerror(Thread, EGL_BAD_ALLOC);;
        return EGL_NO_IMAGE;
    }

    image->image.magic   = KHR_EGL_IMAGE_MAGIC_NUM;
    image->image.type    = KHR_IMAGE_WAYLAND_BUFFER;

    image->image.surface           = surface;
    image->image.u.wlbuffer.width  = width;
    image->image.u.wlbuffer.height = height;

    if (image->image.surface)
    {
        /* Reference the surface. */
        gcoSURF_ReferenceSurface(image->image.surface);
    }
#endif

    return image;
}
#endif

#ifdef LINUX
const EGLuint64KHR _ModiferTable[] =
{
    DRM_FORMAT_MOD_VIVANTE_TILED,
    DRM_FORMAT_MOD_VIVANTE_SUPER_TILED,
    DRM_FORMAT_MOD_VIVANTE_SPLIT_TILED,
    DRM_FORMAT_MOD_VIVANTE_SPLIT_SUPER_TILED
};

#define __DRM_FORMAT_MOD_VIVANTE_UNKNOWN_BIT           0
#define __DRM_FORMAT_MOD_VIVANTE_TILED_BIT             (1 << 0)
#define __DRM_FORMAT_MOD_VIVANTE_SUPER_TILED_BIT       (1 << 1)
#define __DRM_FORMAT_MOD_VIVANTE_SPLIT_TILED_BIT       (1 << 2)
#define __DRM_FORMAT_MOD_VIVANTE_SPLIT_SUPER_TILED_BIT (1 << 3)

#define __DRM_FORMAT_MOD_VIVANTE_ALL_BIT (__DRM_FORMAT_MOD_VIVANTE_TILED_BIT | \
                                          __DRM_FORMAT_MOD_VIVANTE_SUPER_TILED_BIT | \
                                          __DRM_FORMAT_MOD_VIVANTE_SPLIT_TILED_BIT | \
                                          __DRM_FORMAT_MOD_VIVANTE_SPLIT_SUPER_TILED_BIT )
static struct
{
    int fourcc;
    gceSURF_FORMAT format;
    gctUINT32 modifer;  /* modifier.*/
}
_FormatTable[] =
{
    /* 8 bpp R */
    {DRM_FORMAT_R8,       gcvSURF_R8,       __DRM_FORMAT_MOD_VIVANTE_SUPER_TILED_BIT},

    /* 16 bpp RG */
    {DRM_FORMAT_GR88,     gcvSURF_G8R8,     __DRM_FORMAT_MOD_VIVANTE_SUPER_TILED_BIT},

    /* 16 bpp RGB */
    {DRM_FORMAT_XRGB4444, gcvSURF_X4R4G4B4, __DRM_FORMAT_MOD_VIVANTE_SUPER_TILED_BIT},
    {DRM_FORMAT_XBGR4444, gcvSURF_X4B4G4R4, __DRM_FORMAT_MOD_VIVANTE_SUPER_TILED_BIT},

/*
    {DRM_FORMAT_RGBX4444, gcvSURF_R4G4B4X4},
    {DRM_FORMAT_BGRX4444, gcvSURF_B4G4R4X4},
 */

    {DRM_FORMAT_ARGB4444, gcvSURF_A4R4G4B4, __DRM_FORMAT_MOD_VIVANTE_SUPER_TILED_BIT},
    {DRM_FORMAT_ABGR4444, gcvSURF_A4B4G4R4, __DRM_FORMAT_MOD_VIVANTE_SUPER_TILED_BIT},

/*
    {DRM_FORMAT_RGBA4444, gcvSURF_R4G4B4A4},
    {DRM_FORMAT_BGRA4444, gcvSURF_B4G4R4A4},
 */

    {DRM_FORMAT_XRGB1555, gcvSURF_X1R5G5B5, __DRM_FORMAT_MOD_VIVANTE_SUPER_TILED_BIT},
    {DRM_FORMAT_XBGR1555, gcvSURF_X1B5G5R5, __DRM_FORMAT_MOD_VIVANTE_SUPER_TILED_BIT},

/*
    {DRM_FORMAT_RGBX5551, gcvSURF_R5G5B5X1},
    {DRM_FORMAT_BGRX5551, gcvSURF_B5G5R5X1},
 */

    {DRM_FORMAT_ARGB1555, gcvSURF_A1R5G5B5, __DRM_FORMAT_MOD_VIVANTE_SUPER_TILED_BIT},
    {DRM_FORMAT_ABGR1555, gcvSURF_A1B5G5R5, __DRM_FORMAT_MOD_VIVANTE_SUPER_TILED_BIT},

/*
    {DRM_FORMAT_RGBA5551, gcvSURF_R5G5B5A1},
    {DRM_FORMAT_BGRA5551, gcvSURF_B5G5R5A1},
 */

    {DRM_FORMAT_RGB565,   gcvSURF_R5G6B5,   __DRM_FORMAT_MOD_VIVANTE_SUPER_TILED_BIT},
    {DRM_FORMAT_BGR565,   gcvSURF_B5G6R5,   __DRM_FORMAT_MOD_VIVANTE_SUPER_TILED_BIT},

    /* 24 bpp RGB not supported */

    /* 32 bpp RGB */
    {DRM_FORMAT_XRGB8888, gcvSURF_X8R8G8B8, __DRM_FORMAT_MOD_VIVANTE_SUPER_TILED_BIT},
    {DRM_FORMAT_XBGR8888, gcvSURF_X8B8G8R8, __DRM_FORMAT_MOD_VIVANTE_SUPER_TILED_BIT},

    {DRM_FORMAT_RGBX8888, gcvSURF_R8G8B8X8, __DRM_FORMAT_MOD_VIVANTE_SUPER_TILED_BIT},
    {DRM_FORMAT_BGRA8888, gcvSURF_B8G8R8A8, __DRM_FORMAT_MOD_VIVANTE_SUPER_TILED_BIT},

    {DRM_FORMAT_ARGB8888, gcvSURF_A8R8G8B8, __DRM_FORMAT_MOD_VIVANTE_SUPER_TILED_BIT},
    {DRM_FORMAT_ABGR8888, gcvSURF_A8B8G8R8, __DRM_FORMAT_MOD_VIVANTE_SUPER_TILED_BIT},

    {DRM_FORMAT_RGBA8888, gcvSURF_R8G8B8A8, __DRM_FORMAT_MOD_VIVANTE_SUPER_TILED_BIT},
    {DRM_FORMAT_BGRA8888, gcvSURF_B8G8R8A8, __DRM_FORMAT_MOD_VIVANTE_SUPER_TILED_BIT},

    /* 32 bpp 2-10-10-10 format not supported */

    /* packed YCbCr */
    {DRM_FORMAT_YUYV, gcvSURF_YUY2,         __DRM_FORMAT_MOD_VIVANTE_UNKNOWN_BIT},
    {DRM_FORMAT_YVYU, gcvSURF_YVYU,         __DRM_FORMAT_MOD_VIVANTE_UNKNOWN_BIT},
    {DRM_FORMAT_UYVY, gcvSURF_UYVY,         __DRM_FORMAT_MOD_VIVANTE_UNKNOWN_BIT},
    {DRM_FORMAT_VYUY, gcvSURF_VYUY,         __DRM_FORMAT_MOD_VIVANTE_UNKNOWN_BIT},

    {DRM_FORMAT_AYUV, gcvSURF_AYUV,         __DRM_FORMAT_MOD_VIVANTE_UNKNOWN_BIT},

    /* 2 plane YCbCr */
    {DRM_FORMAT_NV12, gcvSURF_NV12,         __DRM_FORMAT_MOD_VIVANTE_UNKNOWN_BIT},
    {DRM_FORMAT_NV21, gcvSURF_NV21,         __DRM_FORMAT_MOD_VIVANTE_UNKNOWN_BIT},
    /* supported by SW convertor. */
    {DRM_FORMAT_NV16, gcvSURF_NV16,         __DRM_FORMAT_MOD_VIVANTE_UNKNOWN_BIT},
    {DRM_FORMAT_NV61, gcvSURF_NV61,         __DRM_FORMAT_MOD_VIVANTE_UNKNOWN_BIT},

    /* 3 plane YCbCr */
    {DRM_FORMAT_YUV420, gcvSURF_I420,       __DRM_FORMAT_MOD_VIVANTE_UNKNOWN_BIT},
    {DRM_FORMAT_YVU420, gcvSURF_YV12,       __DRM_FORMAT_MOD_VIVANTE_UNKNOWN_BIT},
};


static VEGLImage
_CreateImageDMABuf(
    VEGLThreadData Thread,
    VEGLDisplay Dpy,
    VEGLContext Ctx,
    EGLClientBuffer Buffer,
    const void *attrib_list,
    EGLBoolean intAttrib
    )
{
    gctUINT i;
    VEGLImage image;
    gcoSURF surface;
    gceSURF_FORMAT format = gcvSURF_UNKNOWN;
    gcsSURF_FORMAT_INFO_PTR formatInfo;

    EGLint width  = 0;
    EGLint height = 0;
    EGLint fd[4]     = {-1, -1, -1, -1};
    EGLint fourcc    = 0;
    EGLint offset[4] = {-1, -1, -1, -1};
    EGLint pitch[4]  = {-1, -1, -1, -1};
    EGLint modifierLO[4] = {0, 0, 0, 0};
    EGLint modifierHI[4] = {0, 0, 0, 0};
    EGLuint64KHR modifier0 = 0;
    EGLenum colorSpace  = EGL_ITU_REC601_EXT;
    EGLenum sampleRange = EGL_YUV_NARROW_RANGE_EXT;
    EGLenum siting[2]   = {
        EGL_YUV_CHROMA_SITING_0_EXT,
        EGL_YUV_CHROMA_SITING_0_EXT
    };
    gctUINT stride[4];
    gctUINT32 handle[4];
    gctUINT bufferOffset[4];
    gceSURF_TYPE surfType = gcvSURF_BITMAP;

    /* Context must be null, Buffer must be null. */
    if (Ctx != gcvNULL || Buffer != (EGLClientBuffer) gcvNULL)
    {
        veglSetEGLerror(Thread, EGL_BAD_CONTEXT);
        return EGL_NO_IMAGE;
    }

    /* Attributes must exist, otherwise the list is incomplete. */
    if (attrib_list == gcvNULL)
    {
        veglSetEGLerror(Thread, EGL_BAD_PARAMETER);
        return EGL_NO_IMAGE;
    }

    for (i = 0; _AttribValue(attrib_list, intAttrib, i) != EGL_NONE; i += 2)
    {
        EGLint attrib = _AttribValue(attrib_list, intAttrib, i);
        EGLint value  = _AttribValue(attrib_list, intAttrib, i + 1);

        switch (attrib)
        {
        case EGL_WIDTH:
            width = value;
            break;
        case EGL_HEIGHT:
            height = value;
            break;

        case EGL_LINUX_DRM_FOURCC_EXT:
            fourcc = value;
            break;

        case EGL_DMA_BUF_PLANE0_FD_EXT:
            fd[0] = value;
            break;
        case EGL_DMA_BUF_PLANE0_OFFSET_EXT:
            offset[0] = value;
            break;
        case EGL_DMA_BUF_PLANE0_PITCH_EXT:
            pitch[0] = value;
            break;
        case EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT:
            modifierLO[0] = value;
            break;
        case EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT:
            modifierHI[0] = value;
            break;

        case EGL_DMA_BUF_PLANE1_FD_EXT:
            fd[1] = value;
            break;
        case EGL_DMA_BUF_PLANE1_OFFSET_EXT:
            offset[1] = value;
            break;
        case EGL_DMA_BUF_PLANE1_PITCH_EXT:
            pitch[1] = value;
            break;
        case EGL_DMA_BUF_PLANE1_MODIFIER_LO_EXT:
            modifierLO[1] = value;
            break;
        case EGL_DMA_BUF_PLANE1_MODIFIER_HI_EXT:
            modifierHI[1] = value;
            break;

        case EGL_DMA_BUF_PLANE2_FD_EXT:
            fd[2] = value;
            break;
        case EGL_DMA_BUF_PLANE2_OFFSET_EXT:
            offset[2] = value;
            break;
        case EGL_DMA_BUF_PLANE2_PITCH_EXT:
            pitch[2] = value;
            break;
        case EGL_DMA_BUF_PLANE2_MODIFIER_LO_EXT:
            modifierLO[2] = value;
            break;
        case EGL_DMA_BUF_PLANE2_MODIFIER_HI_EXT:
            modifierHI[2] = value;
            break;

        case EGL_DMA_BUF_PLANE3_FD_EXT:
            fd[3] = value;
            break;
        case EGL_DMA_BUF_PLANE3_OFFSET_EXT:
            offset[3] = value;
            break;
        case EGL_DMA_BUF_PLANE3_PITCH_EXT:
            pitch[3] = value;
            break;
        case EGL_DMA_BUF_PLANE3_MODIFIER_LO_EXT:
            modifierLO[3] = value;
            break;
        case EGL_DMA_BUF_PLANE3_MODIFIER_HI_EXT:
            modifierHI[3] = value;
            break;

        case EGL_YUV_COLOR_SPACE_HINT_EXT:
            if ((value != EGL_ITU_REC601_EXT) &&
                (value != EGL_ITU_REC709_EXT) &&
                (value != EGL_ITU_REC2020_EXT))
            {
                veglSetEGLerror(Thread, EGL_BAD_ATTRIBUTE);
                return EGL_NO_IMAGE;
            }
            colorSpace = value;
            break;

        case EGL_SAMPLE_RANGE_HINT_EXT:
            if ((value != EGL_YUV_FULL_RANGE_EXT) &&
                (value != EGL_YUV_NARROW_RANGE_EXT))
            {
                veglSetEGLerror(Thread, EGL_BAD_ATTRIBUTE);
                return EGL_NO_IMAGE;
            }
            sampleRange = value;
            break;

        case EGL_YUV_CHROMA_HORIZONTAL_SITING_HINT_EXT:
        case EGL_YUV_CHROMA_VERTICAL_SITING_HINT_EXT:
            if ((value != EGL_YUV_CHROMA_SITING_0_EXT) &&
                (value != EGL_YUV_CHROMA_SITING_0_5_EXT))
            {
                veglSetEGLerror(Thread, EGL_BAD_ATTRIBUTE);
                return EGL_NO_IMAGE;
            }
            siting[attrib - EGL_YUV_CHROMA_HORIZONTAL_SITING_HINT_EXT] = value;
            break;

        default:
            /* Unacceptable attributes. */
            veglSetEGLerror(Thread, EGL_BAD_PARAMETER);
            return EGL_NO_IMAGE;
        }
    }

    /* Seems like we can't support different plane with different modifiers.*/
    modifier0 = modifierLO[0] | (modifierHI[0] < 32);

    if (!modifier0)
    {
        /* Check Modifier.*/
        for (i = 0; i < 4; ++i)
        {
            EGLuint64KHR modifier = modifierLO[i] | (modifierHI[i] < 32);

            if (modifier0 != modifier ||
                !(modifierLO[i] != 0 && modifierHI[i] != 0))
            {
                veglSetEGLerror(Thread, EGL_BAD_PARAMETER);
                return EGL_NO_IMAGE;
            }
        }
    }

    /* Check width, height */
    if (width <= 0 || height <= 0)
    {
        /* incomplete. */
        veglSetEGLerror(Thread, EGL_BAD_PARAMETER);
        return EGL_NO_IMAGE;
    }

    /* Translate to HAL format. */
    for (i = 0; i < gcmCOUNTOF(_FormatTable); i++)
    {
        if (fourcc == _FormatTable[i].fourcc)
        {
            format = _FormatTable[i].format;
            break;
        }
    }

    if (format == gcvSURF_UNKNOWN)
    {
        /* Format not supported. */
        veglSetEGLerror(Thread, EGL_BAD_MATCH);
        return EGL_NO_IMAGE;
    }

    /* Get surface type.*/
    switch (modifier0)
    {
        /* Currently, we only support super tile */
    case DRM_FORMAT_MOD_VIVANTE_SUPER_TILED:
        surfType = gcvSURF_TEXTURE;
        break;
    case DRM_FORMAT_MOD_VIVANTE_TILED:
    case DRM_FORMAT_MOD_VIVANTE_SPLIT_TILED:
    case DRM_FORMAT_MOD_VIVANTE_SPLIT_SUPER_TILED:
        /* Need add code to support other modifier in future.*/
        gcmASSERT(gcvFALSE);
        veglSetEGLerror(Thread, EGL_BAD_PARAMETER);
        return EGL_NO_IMAGE;
    default:
        surfType = gcvSURF_BITMAP;
        break;
    }

    /* Check format*/
    switch (format)
    {
    case gcvSURF_YV12:
    case gcvSURF_I420:
        /* Check completeness. */
        if ((fd[2] < 0) || (offset[2] < 0) || (pitch[2] < 0) ||
            (fd[1] < 0) || (offset[1] < 0) || (pitch[1] < 0) ||
            (fd[0] < 0) || (offset[0] < 0) || (pitch[0] < 0))
        {
            veglSetEGLerror(Thread, EGL_BAD_PARAMETER);
            return EGL_NO_IMAGE;
        }

        /* Check address and stride alignment. */
        if ((offset[0] & 0x3F) || (offset[1] & 0x3F) || (offset[2] & 0x3F) ||
            (pitch[0]  & 0x0F) || (pitch[1]  & 0x07) || (pitch[2]  & 0x07))
        {
            veglSetEGLerror(Thread, EGL_BAD_ACCESS);
            return EGL_NO_IMAGE;
        }
        break;

    case gcvSURF_NV12:
    case gcvSURF_NV21:
    case gcvSURF_NV16:
    case gcvSURF_NV61:
        /* Check completeness. */
        if ((fd[1] < 0) || (offset[1] < 0) || (pitch[1] < 0) ||
            (fd[0] < 0) || (offset[0] < 0) || (pitch[0] < 0))
        {
            veglSetEGLerror(Thread, EGL_BAD_PARAMETER);
            return EGL_NO_IMAGE;
        }

        /* Check address and stride alignment. */
        if ((offset[0] & 0x3F) || (offset[1] & 0x3F) ||
            (pitch[0]  & 0x0F) || (pitch[1]  & 0x07))
        {
            veglSetEGLerror(Thread, EGL_BAD_ACCESS);
            return EGL_NO_IMAGE;
        }
        break;

    default:
        /* Check completeness. */
        if ((fd[0] < 0) || (offset[0] < 0) || (pitch[0] < 0))
        {
            veglSetEGLerror(Thread, EGL_BAD_PARAMETER);
            return EGL_NO_IMAGE;
        }

        /* Check for useless attributes. */
        if ((fd[1] > 0) || (offset[1] > 0) || (pitch[1] > 0) ||
            (fd[2] > 0) || (offset[2] > 0) || (pitch[2] > 0))
        {
            veglSetEGLerror(Thread, EGL_BAD_ATTRIBUTE);
            return EGL_NO_IMAGE;
        }

        /* Check address and stride alignment. */
        if (gcmIS_ERROR(gcoSURF_QueryFormat(format, &formatInfo)))
        {
            /* Unknown format? */
            veglSetEGLerror(Thread, EGL_BAD_MATCH);
            return EGL_NO_IMAGE;
        }

        if ((offset[0] & 0x3F) ||
            /* 4 pixel alignment in stride. */
            (pitch[0] & (4 * formatInfo->bitsPerPixel / 8 - 1)))
        {
            veglSetEGLerror(Thread, EGL_BAD_ACCESS);
            return EGL_NO_IMAGE;
        }
        break;
    }

    for (i = 0; i < 4; i++)
    {
        stride[i] = (gctUINT) pitch[i];
        handle[i] = (gctUINT32) fd[i];
        bufferOffset[i] = (gctUINT) offset[i];
    }

    /* Wrap as a surface object. */
    if (gcmIS_ERROR(gcoSURF_WrapUserMultiBuffer(gcvNULL,
                                                width, height,
                                                surfType,
                                                format,
                                                stride,
                                                handle,
                                                bufferOffset,
                                                gcvALLOC_FLAG_DMABUF,
                                                &surface)))
    {
        /* Failed. */
        veglSetEGLerror(Thread, EGL_BAD_ACCESS);
        return EGL_NO_IMAGE;
    }

    /* Initialize an image struct. */
    image = _InitializeImage(Thread, Dpy, Ctx);

    if (image == gcvNULL)
    {
        /* Error, very unlikely. */
        gcmVERIFY_OK(gcoSURF_Destroy(surface));
        gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));

        veglSetEGLerror(Thread, EGL_BAD_ALLOC);;
        return EGL_NO_IMAGE;
    }

    image->image.magic   = KHR_EGL_IMAGE_MAGIC_NUM;
    image->image.type    = KHR_IMAGE_LINUX_DMA_BUF;

    /* Stores image buffer */
    image->image.surface = surface;
    image->image.update  = gcvNULL;

    image->image.u.dmaBuf.width  = width;
    image->image.u.dmaBuf.height = height;
    image->image.u.dmaBuf.format = format;

    for (i = 0; i < 3; i++)
    {
        image->image.u.dmaBuf.fd[i]     = fd[i];
        image->image.u.dmaBuf.offset[i] = offset[i];
        image->image.u.dmaBuf.pitch[i]  = pitch[i];
    }

    /* Map into Vivante HAL enums. */
    switch (colorSpace)
    {
    case EGL_ITU_REC601_EXT:
    default:
        image->image.u.dmaBuf.colorSpace = gcvSURF_ITU_REC601;
        break;
    case EGL_ITU_REC709_EXT:
        image->image.u.dmaBuf.colorSpace = gcvSURF_ITU_REC709;
        break;
    case EGL_ITU_REC2020_EXT:
        image->image.u.dmaBuf.colorSpace = gcvSURF_ITU_REC2020;
        break;
    }

    switch (sampleRange)
    {
    case EGL_YUV_NARROW_RANGE_EXT:
        image->image.u.dmaBuf.sampleRange = gcvSURF_YUV_NARROW_RANGE;
        break;
    case EGL_YUV_FULL_RANGE_EXT:
    default:
        image->image.u.dmaBuf.sampleRange = gcvSURF_YUV_FULL_RANGE;
        break;
    }

    for (i = 0; i < 2; i++)
    {
        switch (siting[i])
        {
        case EGL_YUV_CHROMA_SITING_0_EXT:
            image->image.u.dmaBuf.siting[i] = gcvSURF_YUV_CHROMA_SITING_0;
            break;
        case EGL_YUV_CHROMA_SITING_0_5_EXT:
        default:
            image->image.u.dmaBuf.siting[i] = gcvSURF_YUV_CHROMA_SITING_0_5;
            break;
        }
    }

    return image;
}
#endif

static EGLImage
veglCreateImage (
    EGLDisplay Dpy,
    EGLContext Ctx,
    EGLenum Target,
    EGLClientBuffer Buffer,
    const void *attrib_list,
    EGLBoolean intAttrib
    )
{
    VEGLThreadData  thread;
    VEGLDisplay     dpy;
    VEGLContext     ctx = gcvNULL;
    VEGLImage       image;
    gceSTATUS status;

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        return EGL_NO_IMAGE;
    }

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Dpy);
    if (dpy == gcvNULL)
    {
        /* Bad display. */
        veglSetEGLerror(thread,  EGL_BAD_DISPLAY);
        return EGL_NO_IMAGE;
    }

    /* Test if EGLDisplay structure has been initialized. */
    if (!dpy->initialized)
    {
        /* Not initialized. */
        veglSetEGLerror(thread,  EGL_NOT_INITIALIZED);;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Hardware relevant thread data initialization. */
    veglInitDeviceThreadData(thread);

    /* Get context, context may be NULL. */
    if (Ctx == EGL_NO_CONTEXT)
    {
        ctx = EGL_NO_CONTEXT;
    }
    else
    {
        ctx = (VEGLContext)veglGetResObj(dpy,
                                         (VEGLResObj*)&dpy->contextStack,
                                         (EGLResObj)Ctx,
                                         EGL_CONTEXT_SIGNATURE);

        if (ctx == gcvNULL)
        {
            veglSetEGLerror(thread,  EGL_BAD_CONTEXT);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }

    /* Create eglImge by target. */
    switch (Target)
    {
    case EGL_GL_TEXTURE_2D_KHR:
        image = _CreateImageTex2D(thread,
                                  dpy, ctx,
                                  Buffer,
                                  attrib_list,
                                  intAttrib);
        break;

    case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR:
    case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_X_KHR:
    case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Y_KHR:
    case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_KHR:
    case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Z_KHR:
    case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_KHR:
        image = _CreateImageTexCube(thread,
                                    dpy, ctx,
                                    Target,
                                    Buffer,
                                    attrib_list,
                                    intAttrib);
        break;

    case EGL_NATIVE_PIXMAP_KHR:
        image = _CreateImagePixmap(thread,
                                   dpy,
                                   ctx,
                                   Buffer,
                                   attrib_list,
                                   intAttrib);
        break;

    case EGL_GL_RENDERBUFFER_KHR:
        image = _CreateImageRenderBuffer(thread,
                                         dpy, ctx,
                                         Buffer,
                                         attrib_list,
                                         intAttrib);
        break;

#if defined(ANDROID)
    case EGL_NATIVE_BUFFER_ANDROID:
        image = _CreateImageANativeBuffer(thread,
                                          dpy, ctx,
                                          Buffer,
                                          attrib_list,
                                          intAttrib);
        break;
#endif

    case EGL_VG_PARENT_IMAGE_KHR:
        image = _CreateImageVGParent(thread,
                                     dpy,
                                     ctx,
                                     Buffer,
                                     attrib_list,
                                     intAttrib);
        break;

#ifdef EGL_WAYLAND_BUFFER_WL
    case EGL_WAYLAND_BUFFER_WL:
        image = _CreateImageWL(thread,
                               dpy, ctx,
                               Buffer,
                               attrib_list,
                               intAttrib);
        break;
#endif

#ifdef LINUX
    case EGL_LINUX_DMA_BUF_EXT:
        image = _CreateImageDMABuf(thread,
                                   dpy, ctx,
                                   Buffer,
                                   attrib_list,
                                   intAttrib);
        break;
#endif

    default:
        image = EGL_NO_IMAGE;
        /* Not a valid target type. */
        veglSetEGLerror(thread,  EGL_BAD_PARAMETER);;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Test if create successful. */
    if (image == EGL_NO_IMAGE)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    VEGL_LOCK_DISPLAY_RESOURCE(dpy);

    /* Push image into stack. */
    /* Image is already referenced when creation. */
    {
        VEGLImage img = image;

        while (img->next != gcvNULL)
        {
            img = img->next;
        }

        img->next = dpy->imageStack;
    }
    dpy->imageStack = image;

    VEGL_UNLOCK_DISPLAY_RESOURCE(dpy);

    /* Success. */
    veglSetEGLerror(thread,  EGL_SUCCESS);

    return (EGLImage) image;

OnError:
    return EGL_NO_IMAGE;

}

static EGLBoolean
veglDestroyImageImpl(
    EGLDisplay Dpy,
    EGLImage Image
    )
{
    VEGLThreadData  thread;
    VEGLDisplay     dpy;
    VEGLImage       image;
    VEGLImage       stack;
    gceSTATUS status;

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        return EGL_FALSE;
    }

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Dpy);
    if (dpy == gcvNULL)
    {
        /* Bad display. */
        veglSetEGLerror(thread,  EGL_BAD_DISPLAY);
        return EGL_FALSE;
    }

    /* Test if EGLDisplay structure has been initialized. */
    if (!dpy->initialized)
    {
        /* Not initialized. */
        veglSetEGLerror(thread,  EGL_NOT_INITIALIZED);;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Hardware relevant thread data initialization. */
    veglInitDeviceThreadData(thread);

    /* Get shortcut of the eglImage. */
    image = VEGL_IMAGE(Image);

    /* Test if eglImage is valid. */
    if ((image == gcvNULL) ||
        (image->signature != EGL_IMAGE_SIGNATURE))
    {
        veglSetEGLerror(thread,  EGL_BAD_PARAMETER);;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    VEGL_LOCK_DISPLAY_RESOURCE(dpy);
    /* Pop EGLImage from the stack. */
    if (image == dpy->imageStack)
    {
        /* Simple - it is the top of the stack. */
        dpy->imageStack = image->next;
    }
    else
    {
        /* Walk the stack. */
        for (stack = dpy->imageStack;
             stack != gcvNULL;
             stack = stack->next)
        {
            /* Check if the next image on the stack is ours. */
            if (stack->next == image)
            {
                /* Pop the image from the stack. */
                stack->next = image->next;
                break;
            }
        }
    }
    VEGL_UNLOCK_DISPLAY_RESOURCE(dpy);

    /* Mark as destroyed. */
    image->destroyed = gcvTRUE;

    /* Dereference EGLImage. */
    veglDereferenceImage(thread, dpy, image);

    /* Success. */
    veglSetEGLerror(thread,  EGL_SUCCESS);
    return EGL_TRUE;

OnError:
    return EGL_FALSE;
}

static EGLBoolean
veglQueryDmaBufFormats(
    EGLDisplay Dpy,
    EGLint max_formats,
    EGLint *formats,
    EGLint *num_formats
    )
{
#ifdef LINUX
    VEGLThreadData  thread;
    VEGLDisplay     dpy;
    gceSTATUS       status;
    gctUINT         formatCount = 0;
    gctUINT         i = 0;

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        return EGL_FALSE;
    }

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Dpy);
    if (dpy == gcvNULL)
    {
        /* Bad display. */
        veglSetEGLerror(thread,  EGL_BAD_DISPLAY);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Test if EGLDisplay structure has been initialized. */
    if (!dpy->initialized)
    {
        /* Not initialized. */
        veglSetEGLerror(thread,  EGL_NOT_INITIALIZED);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Get format Count */
    for (i = 0; i < gcmCOUNTOF(_FormatTable); ++i)
    {
        /* Now, we only support super tiled format.*/
        if (_FormatTable[i].modifer != __DRM_FORMAT_MOD_VIVANTE_UNKNOWN_BIT)
        {
            formatCount++;
        }
    }

    if (max_formats < 0)
    {
        veglSetEGLerror(thread,  EGL_BAD_PARAMETER);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }
    else if (max_formats == 0)
    {
        gcmASSERT(num_formats);
        *num_formats = formatCount;
    }
    else
    {
        if (formats)
        {
            for (i = 0; i < gcmCOUNTOF(_FormatTable); ++i)
            {
                if (_FormatTable[i].modifer != __DRM_FORMAT_MOD_VIVANTE_UNKNOWN_BIT)
                {
                    formats[i] = _FormatTable[i].fourcc;
                }
            }
        }
        else
        {
            veglSetEGLerror(thread,  EGL_BAD_PARAMETER);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }

    /* Success */
    return EGL_TRUE;
OnError:
#endif
    return EGL_FALSE;
}

#ifdef LINUX
static EGLBoolean
_QueryDmaBufModifiers(
    gctUINT formatIndex,
    EGLint *num_modifiers,
    EGLuint64KHR *modifiers
    )
{
    EGLint modifierCount = 0;
    gctUINT32 modifier = _FormatTable[formatIndex].modifer;
    const gctUINT32 modifierBitTable[] = {
        __DRM_FORMAT_MOD_VIVANTE_TILED_BIT,
        __DRM_FORMAT_MOD_VIVANTE_SUPER_TILED_BIT,
        __DRM_FORMAT_MOD_VIVANTE_SPLIT_TILED_BIT,
        __DRM_FORMAT_MOD_VIVANTE_SPLIT_SUPER_TILED_BIT
        };
    gctUINT i = 0;

    /* Compute modifier num or fill modifier.*/
    for (i = 0; i < gcmCOUNTOF(modifierBitTable); ++i)
    {
        if (modifier & modifierBitTable[i])
        {
            if (num_modifiers)
            {
                modifierCount++;
            }
            else
            {
                gcmASSERT(modifiers);
                modifiers[i] = _ModiferTable[i];
            }
        }
    }

    if (num_modifiers)
    {
        *num_modifiers = modifierCount;
    }

    return EGL_TRUE;
}
#endif

static EGLBoolean
veglQueryDmaBufModifiers(
    EGLDisplay Dpy,
    EGLint format,
    EGLint max_modifiers,
    EGLuint64KHR *modifiers,
    EGLBoolean *external_only,
    EGLint *num_modifiers
    )
{
#ifdef LINUX
    VEGLThreadData     thread;
    VEGLDisplay        dpy;
    gceSTATUS          status;
    gctUINT            formatCount = 0;
    gctUINT            i = 0;

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        return EGL_FALSE;
    }

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Dpy);
    if (dpy == gcvNULL)
    {
        /* Bad display. */
        veglSetEGLerror(thread,  EGL_BAD_DISPLAY);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Test if EGLDisplay structure has been initialized. */
    if (!dpy->initialized)
    {
        /* Not initialized. */
        veglSetEGLerror(thread,  EGL_NOT_INITIALIZED);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Get format Count */
    formatCount = gcmCOUNTOF(_FormatTable);

    /* Check format */
    for (i = 0; i < formatCount; ++i)
    {
        if (_FormatTable[i].modifer != __DRM_FORMAT_MOD_VIVANTE_UNKNOWN_BIT &&
            format == _FormatTable[i].fourcc)
            break;
    }

    if (i >= formatCount)
    {
        veglSetEGLerror(thread,  EGL_BAD_PARAMETER);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (max_modifiers < 0)
    {
        veglSetEGLerror(thread,  EGL_BAD_PARAMETER);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }
    else if (max_modifiers == 0)
    {
        gcmASSERT(num_modifiers);
        _QueryDmaBufModifiers(i, num_modifiers, gcvNULL);
    }
    else
    {
        if (modifiers)
        {
            _QueryDmaBufModifiers(i, gcvNULL, modifiers);
        }
        else
        {
            veglSetEGLerror(thread,  EGL_BAD_PARAMETER);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }

    /* Success */
    return EGL_TRUE;
OnError:
#endif
    return EGL_FALSE;
}

/* EGL 1.5 */
EGLAPI EGLImage EGLAPIENTRY
eglCreateImage(
    EGLDisplay dpy,
    EGLContext ctx,
    EGLenum target,
    EGLClientBuffer buffer,
    const EGLAttrib *attrib_list
    )
{
    EGLImage image;

    gcmHEADER_ARG("dpy=0x%08x ctx=0x%08x target=0x%04x buffer=0x%08x attrib_list=0x%08x",
                    dpy, ctx, target, buffer, attrib_list);
    VEGL_TRACE_API_PRE(CreateImage)(dpy, ctx, target, buffer, attrib_list);

    /* Call internal function. */
    image = veglCreateImage(dpy, ctx, target, buffer, attrib_list, EGL_FALSE);

    VEGL_TRACE_API_POST(CreateImage)(dpy, ctx, target, buffer, attrib_list, image);
    gcmDUMP_API("${EGL eglCreateImage 0x%08X 0x%08X 0x%08X 0x%08X (0x%08X) "
                ":= 0x%08X",
                dpy, ctx, target, buffer, attrib_list, image);
    gcmDUMP_API_ARRAY_TOKEN(attrib_list, EGL_NONE);
    gcmDUMP_API("$}");

    gcmFOOTER_ARG("return=%d", image);
    return image;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglDestroyImage(
    EGLDisplay dpy,
    EGLImage image
    )
{
    EGLBoolean result;
    gcmHEADER_ARG("dpy=0x%x image=0x%x", dpy, image);

    /* Alias to eglDestroyimage. */
    result = veglDestroyImageImpl(dpy, image);

    gcmDUMP_API("${EGL eglDestroyImage 0x%08X 0x%08X}", dpy, image);
    VEGL_TRACE_API(DestroyImage)(dpy, image);

    gcmFOOTER_ARG("return=%d", result);
    return result;
}

/* EGL_KHR_image. */
EGLAPI EGLImageKHR EGLAPIENTRY
eglCreateImageKHR(
    EGLDisplay dpy,
    EGLContext ctx,
    EGLenum target,
    EGLClientBuffer buffer,
    const EGLint *attrib_list
    )
{
    EGLImageKHR image;

    gcmHEADER_ARG("dpy=0x%08x ctx=0x%08x target=0x%04x buffer=0x%08x attrib_list=0x%08x",
                    dpy, ctx, target, buffer, attrib_list);
    VEGL_TRACE_API_PRE(CreateImageKHR)(dpy, ctx, target, buffer, attrib_list);

    /* Alias to eglCreateImage. */
    image = (EGLImageKHR) veglCreateImage(
                dpy, ctx, target, buffer, attrib_list, EGL_TRUE);

    VEGL_TRACE_API_POST(CreateImageKHR)(dpy, ctx, target, buffer, attrib_list, image);
    gcmDUMP_API("${EGL eglCreateImageKHR 0x%08X 0x%08X 0x%08X 0x%08X (0x%08X) "
                ":= 0x%08X",
                dpy, ctx, target, buffer, attrib_list, image);
    gcmDUMP_API_ARRAY_TOKEN(attrib_list, EGL_NONE);
    gcmDUMP_API("$}");

    gcmFOOTER_ARG("return=%d", image);
    return image;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglDestroyImageKHR(
    EGLDisplay dpy,
    EGLImageKHR image
    )
{
    EGLBoolean result;
    gcmHEADER_ARG("dpy=0x%x image=0x%x", dpy, image);

    /* Alias to eglDestroyimage. */
    result = veglDestroyImageImpl(dpy, (EGLImage) image);

    gcmDUMP_API("${EGL eglDestroyImageKHR 0x%08X 0x%08X}", dpy, image);
    VEGL_TRACE_API(DestroyImageKHR)(dpy, image);

    gcmFOOTER_ARG("return=%d", result);
    return result;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglQueryDmaBufFormatsEXT(
    EGLDisplay dpy,
    EGLint max_formats,
    EGLint *formats,
    EGLint *num_formats
    )
{
    EGLBoolean result = EGL_FALSE;
    gcmHEADER_ARG("dpy=0x%x max_formats=0x%x, formats=0x%x, num_formats=0x%x", dpy, max_formats, formats, num_formats);

    result = veglQueryDmaBufFormats(dpy, max_formats, formats, num_formats);

    gcmDUMP_API("${EGL eglQueryDmaBufFormatsEXT 0x%08X 0x%08X 0x%08X 0x%08X}", dpy, max_formats, formats, num_formats);
    VEGL_TRACE_API(QueryDmaBufFormatsEXT)(dpy, max_formats, formats, num_formats);

    gcmFOOTER_ARG("return=%d", result);
    return result;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglQueryDmaBufModifiersEXT(
    EGLDisplay dpy,
    EGLint format,
    EGLint max_modifiers,
    EGLuint64KHR *modifiers,
    EGLBoolean *external_only,
    EGLint *num_modifiers
    )
{
    EGLBoolean result = EGL_FALSE;
    gcmHEADER_ARG("dpy=0x%x format=0x%x, max_modifiers=0x%x, modifiers=0x%x, external_only=0x%x, num_modifiers=0x%x", dpy, format, max_modifiers, modifiers, external_only, num_modifiers);

    result = veglQueryDmaBufModifiers(dpy, format, max_modifiers, modifiers, external_only, num_modifiers);

    gcmDUMP_API("${EGL eglQueryDmaBufModifiersEXT 0x%08X 0x%08X 0x%08X 0x%08X}", dpy, format, max_modifiers, modifiers, external_only, num_modifiers);
    VEGL_TRACE_API(QueryDmaBufModifiersEXT)(dpy, format, max_modifiers, modifiers, external_only, num_modifiers);

    gcmFOOTER_ARG("return=%d", result);
    return result;
}

#if defined EGL_WAYLAND_BUFFER_WL && defined EGL_API_WL
struct wl_buffer *eglCreateWaylandBufferFromImageWL(EGLDisplay Dpy, EGLImageKHR Image)
{
    VEGLThreadData  thread;
    VEGLDisplay     dpy;
    VEGLImage       image;
    gceSTATUS status;
    struct wl_buffer * wl_buf = gcvNULL;

    gcmHEADER_ARG("Dpy=0x%x Image=0x%x", Dpy, Image);

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        gcmFOOTER_ARG("return=%d", EGL_FALSE);
        return gcvNULL;
    }
    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Dpy);
    if (dpy == gcvNULL)
    {
        /* Bad display. */
        veglSetEGLerror(thread,  EGL_BAD_DISPLAY);

        gcmFOOTER_ARG("return=%d", EGL_FALSE);
        return gcvNULL;
    }
    /* Test if EGLDisplay structure has been initialized. */
    if (!dpy->initialized)
    {
        /* Not initialized. */
        veglSetEGLerror(thread,  EGL_NOT_INITIALIZED);;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Hardware relevant thread data initialization. */
    veglInitDeviceThreadData(thread);

    /* Get shortcut of the eglImage. */
    image = VEGL_IMAGE(Image);
    /* Test if eglImage is valid. */
    if ((image == gcvNULL) ||
        (image->signature != EGL_IMAGE_SIGNATURE))
    {
        veglSetEGLerror(thread,  EGL_BAD_PARAMETER);;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }
    /*Currently KHR_IMAGE_WAYLAND_BUFFER is needed for subsurface */
    if(image->image.type != KHR_IMAGE_WAYLAND_BUFFER)
    {
        veglSetEGLerror(thread,  EGL_BAD_MATCH);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Call underlying function. */
    wl_buf = veglCreateWaylandBufferFromImage(thread, dpy, image);

    if (!wl_buf)
    {
        veglSetEGLerror(thread,  EGL_BAD_PARAMETER);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Success. */
    veglSetEGLerror(thread,  EGL_SUCCESS);
    return wl_buf;

OnError:
    gcmFOOTER_ARG("return=%p", wl_buf);
    return wl_buf;
}
#endif

