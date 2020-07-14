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


#include "gc_egl_precomp.h"

#define _GC_OBJ_ZONE    gcdZONE_EGL_CONFIG

static EGLBoolean
veglParseAttributes(
    VEGLDisplay Display,
    const EGLint * AttributeList,
    VEGLConfig Configuration
    )
{
    VEGLThreadData thread;
    EGLenum attribute;
    EGLint value = 0;

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

    /* Assume success. */
    veglSetEGLerror(thread,  EGL_SUCCESS);

    /* Fill in default attributes. */
    Configuration->bufferSize        = 0;
    Configuration->configBufferSize  = 0;
    Configuration->alphaSize         = 0;
    Configuration->blueSize          = 0;
    Configuration->greenSize         = 0;
    Configuration->redSize           = 0;
    Configuration->depthSize         = 0;
    Configuration->stencilSize       = 0;
    Configuration->configCaveat      = (EGLenum) EGL_DONT_CARE;
    Configuration->configId          = EGL_DONT_CARE;
    Configuration->nativeRenderable  = (EGLBoolean) EGL_DONT_CARE;
    Configuration->nativeVisualType  = EGL_DONT_CARE;
    Configuration->samples           = 0;
    Configuration->sampleBuffers     = 0;
    Configuration->surfaceType       = EGL_WINDOW_BIT;
    Configuration->bindToTetxureRGB  = (EGLBoolean) EGL_DONT_CARE;
    Configuration->bindToTetxureRGBA = (EGLBoolean) EGL_DONT_CARE;
    Configuration->luminanceSize     = 0;
    Configuration->alphaMaskSize     = 0;
    Configuration->colorBufferType   = EGL_RGB_BUFFER;
    Configuration->renderableType    = EGL_OPENGL_ES_BIT;
    Configuration->conformant        = 0;
    Configuration->matchFormat       = (EGLenum) EGL_DONT_CARE;
    Configuration->matchNativePixmap = EGL_NONE;
    Configuration->recordableConfig  = (EGLBoolean) EGL_DONT_CARE;
    Configuration->level             = 0;
    Configuration->minSwapInterval   = EGL_DONT_CARE;
    Configuration->maxSwapInterval   = EGL_DONT_CARE;
    Configuration->transparentType   = EGL_NONE;
    Configuration->transparentRedValue    = EGL_DONT_CARE;
    Configuration->transparentGreenValue  = EGL_DONT_CARE;
    Configuration->transparentBlueValue   = EGL_DONT_CARE;
#if defined(ANDROID)
    Configuration->supportFBTarget   = EGL_DONT_CARE;
#endif

    /* Parse the attribute list. */
    do
    {
        if (AttributeList != gcvNULL)
        {
            attribute      = AttributeList[0];
            value          = AttributeList[1];
            AttributeList += 2;
        }
        else
        {
            attribute = EGL_NONE;
        }

        switch (attribute)
        {
        case EGL_BUFFER_SIZE:
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                         "%s: EGL_BUFFER_SIZE=%d",
                         __FUNCTION__, value);
            Configuration->configBufferSize = value;
            break;

        case EGL_ALPHA_SIZE:
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                         "%s: EGL_ALPHA_SIZE=%d",
                         __FUNCTION__, value);
            Configuration->alphaSize = value;
            break;

        case EGL_BLUE_SIZE:
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_BLUE_SIZE=%d",
                          __FUNCTION__, value);
            Configuration->blueSize = value;
            break;

        case EGL_GREEN_SIZE:
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_GREEN_SIZE=%d",
                          __FUNCTION__, value);
            Configuration->greenSize = value;
            break;

        case EGL_RED_SIZE:
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_RED_SIZE=%d",
                          __FUNCTION__, value);
            Configuration->redSize = value;
            break;

        case EGL_DEPTH_SIZE:
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_DEPTH_SIZE=%d",
                          __FUNCTION__, value);
            Configuration->depthSize = value;
            break;

        case EGL_STENCIL_SIZE:
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_STENCIL_SIZE=%d",
                          __FUNCTION__, value);
            Configuration->stencilSize = value;
            break;

        case EGL_CONFIG_CAVEAT:
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_CONFIG_CAVEAT=%d",
                          __FUNCTION__, value);
            Configuration->configCaveat = value;
            break;

        case EGL_CONFIG_ID:
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_CONFIG_ID=%d",
                          __FUNCTION__, value);

            if ((value != EGL_DONT_CARE) &&
                ((value >= Display->configCount + 1) || (value <= 0)))
            {
                /* Bad attribute. */
                veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                return EGL_FALSE;
            }
            Configuration->configId = value;
            break;

        case EGL_LEVEL:
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_LEVEL=%d",
                          __FUNCTION__, value);
            Configuration->level = value;
            break;

        case EGL_MAX_PBUFFER_WIDTH:
            /*
             * EGL SPEC 1.4, Section 3.4.1:
             * If EGL_MAX_PBUFFER_WIDTH, EGL_MAX_PBUFFER_HEIGHT, EGL_MAX_-
             * PBUFFER_PIXELS, or EGL_NATIVE_VISUAL_ID are specified in
             * attrib list, then they are ignored.
             */
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_MAX_PBUFFER_WIDTH=%d",
                          __FUNCTION__, value);
            break;

        case EGL_MAX_PBUFFER_HEIGHT:
            /* Should be ignored. See EGL_MAX_PBUFFER_WIDTH. */
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_MAX_PBUFFER_HEIGHT=%d",
                          __FUNCTION__, value);
            break;

        case EGL_MAX_PBUFFER_PIXELS:
            /* Should be ignored. See EGL_MAX_PBUFFER_WIDTH. */
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_MAX_PBUFFER_PIXELS=%d",
                          __FUNCTION__, value);
            break;

        case EGL_NATIVE_RENDERABLE:
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_NATIVE_RENDERABLE=%d",
                          __FUNCTION__, value);
            if ((value != EGL_DONT_CARE) &&
                (value != EGL_TRUE) && (value != EGL_FALSE))
            {
                /* Bad attribute. */
                veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                return EGL_FALSE;
            }
            Configuration->nativeRenderable = value;
            break;

        case EGL_NATIVE_VISUAL_ID:
            /* Should be ignored. See EGL_MAX_PBUFFER_WIDTH. */
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_NATIVE_VISUAL_ID=%d",
                          __FUNCTION__, value);
            break;

        case EGL_NATIVE_VISUAL_TYPE:
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_NATIVE_VISUAL_TYPE=%d",
                          __FUNCTION__, value);
            Configuration->nativeVisualType = value;
            break;

        case EGL_SAMPLES:
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_SAMPLES=%d",
                          __FUNCTION__, value);
            Configuration->samples = value;
            break;

        case EGL_SAMPLE_BUFFERS:
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_SAMPLE_BUFFERS=%d",
                          __FUNCTION__, value);
            Configuration->sampleBuffers = value;
            break;

        case EGL_SURFACE_TYPE:
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_SURFACE_TYPE=%d",
                          __FUNCTION__, value);
            Configuration->surfaceType = value;
            break;

        case EGL_TRANSPARENT_TYPE:
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_TRANSPARENT_TYPE=%d",
                          __FUNCTION__, value);
            if ((value != EGL_DONT_CARE) && (value != EGL_NONE)
                && (value != EGL_TRANSPARENT_RGB))
            {
                /* Bad attribute. */
                veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                return EGL_FALSE;
            }
            Configuration->transparentType = value;
            break;

        case EGL_TRANSPARENT_BLUE_VALUE:
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_TRANSPARENT_BLUE_VALUE=%d",
                          __FUNCTION__, value);
            Configuration->transparentBlueValue = value;
            break;

        case EGL_TRANSPARENT_GREEN_VALUE:
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_TRANSPARENT_GREEN_VALUE=%d",
                          __FUNCTION__, value);
            Configuration->transparentGreenValue = value;
            break;

        case EGL_TRANSPARENT_RED_VALUE:
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_TRANSPARENT_RED_VALUE=%d",
                          __FUNCTION__, value);
            Configuration->transparentRedValue = value;
            break;

        case EGL_BIND_TO_TEXTURE_RGB:
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_BIND_TO_TEXTURE_RGB=%d",
                          __FUNCTION__, value);
            if ((value != EGL_DONT_CARE) &&
                (value != EGL_TRUE) && (value != EGL_FALSE))
            {
                /* Bad attribute. */
                veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                return EGL_FALSE;
            }
            Configuration->bindToTetxureRGB = value;
            break;

        case EGL_BIND_TO_TEXTURE_RGBA:
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_BIND_TO_TEXTURE_RGBA=%d",
                          __FUNCTION__, value);
            if ((value != EGL_DONT_CARE) &&
                (value != EGL_TRUE) && (value != EGL_FALSE))
            {
                /* Bad attribute. */
                veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                return EGL_FALSE;
            }
            Configuration->bindToTetxureRGBA = value;
            break;

        case EGL_MIN_SWAP_INTERVAL:
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_MIN_SWAP_INTERVAL=%d",
                          __FUNCTION__, value);
            if ((value != EGL_DONT_CARE) && (value < 0))
            {
                /* Bad attribute. */
                veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                return EGL_FALSE;
            }
            Configuration->minSwapInterval = value;
            break;

        case EGL_MAX_SWAP_INTERVAL:
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_MAX_SWAP_INTERVAL=%d",
                          __FUNCTION__, value);
            if ((value != EGL_DONT_CARE) && (value < 0))
            {
                /* Bad attribute. */
                veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                return EGL_FALSE;
            }
            Configuration->maxSwapInterval = value;
            break;

#if defined(ANDROID)
        case EGL_ANDROID_framebuffer_target:
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_ANDROID_framebuffer_target=%d",
                          __FUNCTION__, value);
            Configuration->supportFBTarget = value;
            break;
#endif

        case EGL_LUMINANCE_SIZE:
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_LUMINANCE_SIZE=%d",
                          __FUNCTION__, value);
            Configuration->luminanceSize = value;
            break;

        case EGL_ALPHA_MASK_SIZE:
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_ALPHA_MASK_SIZE=%d",
                          __FUNCTION__, value);
            Configuration->alphaMaskSize = value;
            break;

        case EGL_COLOR_BUFFER_TYPE:
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_COLOR_BUFFER_TYPE=%d",
                          __FUNCTION__, value);
            if ((value != EGL_DONT_CARE) &&
                (value != EGL_RGB_BUFFER) && (value != EGL_LUMINANCE_BUFFER))
            {
                /* Bad attribute. */
                veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                return EGL_FALSE;
            }
            Configuration->colorBufferType = value;
            break;

        case EGL_RENDERABLE_TYPE:
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_RENDERABLE_TYPE=%d",
                          __FUNCTION__, value);
            Configuration->renderableType = value;
            break;

        case EGL_CONFORMANT:
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_RENDERABLE_TYPE=%d",
                          __FUNCTION__, value);
            Configuration->conformant = value;
            break;

        case EGL_MATCH_FORMAT_KHR:
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_MATCH_FORMAT_KHR=%d",
                          __FUNCTION__, value);
            Configuration->matchFormat = value;
            break;

        case EGL_MATCH_NATIVE_PIXMAP:
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_MATCH_NATIVE_PIXMAP=%d",
                          __FUNCTION__, value);
            if (value == EGL_DONT_CARE)
            {
                /* Bad attribute. */
                veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                return EGL_FALSE;
            }
            Configuration->matchNativePixmap = value;
            break;

        case EGL_RECORDABLE_ANDROID:
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "%s: EGL_RECORDABLE_ANDROID=%d",
                          __FUNCTION__, value);
            Configuration->recordableConfig = value;
            break;

        case EGL_NONE:
            break;

        default:
            /* Bad attribute. */
            veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
            return EGL_FALSE;
        }
    }
    while (attribute != EGL_NONE);

    /* Success. */
    return EGL_TRUE;
}

static EGLBoolean
veglSortAfter(
    VEGLConfig Config1,
    VEGLConfig Config2,
    VEGLConfig Attributes
    )
{
    EGLint bits1, bits2;

    /*
     * Priority 1:
     * Special: by EGL_CONFIG_CAVEAT where the precedence is EGL_NONE,
     * EGL_SLOW_CONFIG, EGL_NON_CONFORMANT_CONFIG.
     */
    if (Config1->configCaveat < Config2->configCaveat)
    {
        return EGL_FALSE;
    }
    else if (Config1->configCaveat > Config2->configCaveat)
    {
        return EGL_TRUE;
    }

    /*
     * Priority 2:
     * Special: by EGL_COLOR_BUFFER_TYPE where the precedence is EGL_-
     * RGB_BUFFER, EGL_LUMINANCE_BUFFER.
     */
    if (Config1->colorBufferType < Config2->colorBufferType)
    {
        return EGL_FALSE;
    }
    else if (Config1->colorBufferType > Config2->colorBufferType)
    {
        return EGL_TRUE;
    }

    /*
     * Priority 3:
     * Special: by larger total number of color bits (for an RGB color buffer,
     * this is the sum of EGL_RED_SIZE, EGL_GREEN_SIZE, EGL_BLUE_SIZE,
     * and EGL_ALPHA_SIZE; for a luminance color buffer, the sum of EGL_-
     * LUMINANCE_SIZE and EGL_ALPHA_SIZE) If the requested number of bits
     * in attrib list for a particular color component is 0 or EGL_DONT_CARE, then
     * the number of bits for that component is not considered.
     */
    if ((Attributes->redSize  > 0) || (Attributes->greenSize > 0) ||
        (Attributes->blueSize > 0) || (Attributes->alphaSize > 0))
    {
        /* Compute number of color bits based on attributes. */
        if (Config1->colorBufferType == EGL_RGB_BUFFER)
        {
            bits1 = ((Attributes->redSize   > 0) ? Config1->redSize   : 0) +
                    ((Attributes->greenSize > 0) ? Config1->greenSize : 0) +
                    ((Attributes->blueSize  > 0) ? Config1->blueSize  : 0) +
                    ((Attributes->alphaSize > 0) ? Config1->alphaSize : 0);

            bits2 = ((Attributes->redSize   > 0) ? Config2->redSize   : 0) +
                    ((Attributes->greenSize > 0) ? Config2->greenSize : 0) +
                    ((Attributes->blueSize  > 0) ? Config2->blueSize  : 0) +
                    ((Attributes->alphaSize > 0) ? Config2->alphaSize : 0);
        }
        else
        {
            bits1 = ((Attributes->luminanceSize > 0) ? Config1->luminanceSize : 0) +
                    ((Attributes->alphaSize     > 0) ? Config1->alphaSize     : 0);

            bits2 = ((Attributes->luminanceSize > 0) ? Config2->luminanceSize : 0) +
                    ((Attributes->alphaSize     > 0) ? Config2->alphaSize     : 0);
        }

        if (bits1 > bits2)
        {
            return EGL_FALSE;
        }
        else if (bits1 < bits2)
        {
            return EGL_TRUE;
        }
    }

    /*
     * Priority 4.
     * Smaller EGL_BUFFER_SIZE
     */
    if (Config1->configBufferSize < Config2->configBufferSize)
    {
        return EGL_FALSE;
    }
    else if (Config1->configBufferSize > Config2->configBufferSize)
    {
        return EGL_TRUE;
    }

    /*
     * Priority 5.
     * Smaller EGL_SAMPLE_BUFFERS.
     */
    if (Config1->sampleBuffers < Config2->sampleBuffers)
    {
        return EGL_FALSE;
    }
    else if (Config1->sampleBuffers > Config2->sampleBuffers)
    {
        return EGL_TRUE;
    }

    /*
     * Priority 6.
     * Smaller EGL_SAMPLES.
     */
    if (Config1->samples < Config2->samples)
    {
        return EGL_FALSE;
    }
    else if (Config1->samples > Config2->samples)
    {
        return EGL_TRUE;
    }

    /*
     * Priority 7.
     * Smaller EGL_DEPTH_SIZE.
     */
    if (Config1->depthSize < Config2->depthSize)
    {
        return EGL_FALSE;
    }
    else if (Config1->depthSize > Config2->depthSize)
    {
        return EGL_TRUE;
    }

    /*
     * Priority 8.
     * Smaller EGL_STENCIL_SIZE.
     */
    if (Config1->stencilSize < Config2->stencilSize)
    {
        return EGL_FALSE;
    }
    else if (Config1->stencilSize > Config2->stencilSize)
    {
        return EGL_TRUE;
    }

    /*
     * Priority 9.
     * Smaller EGL_ALPHA_MASK_SIZE.
     */
    if (Config1->alphaMaskSize < Config2->alphaMaskSize)
    {
        return EGL_FALSE;
    }
    else if (Config1->alphaMaskSize > Config2->alphaMaskSize)
    {
        return EGL_TRUE;
    }

    /* Compute native visual type sorting. */
    bits1 = (Config1->nativeVisualType == EGL_NONE)
        ? 0
        : Config1->nativeVisualType;
    bits2 = (Config2->nativeVisualType == EGL_NONE)
        ? 0
        : Config2->nativeVisualType;

    /*
     * Priority 10.
     * Special: by EGL_NATIVE_VISUAL_TYPE (the actual sort order is
     * implementation-defined, depending on the meaning of native visual types).
     */
    if (bits1 < bits2)
    {
        return EGL_FALSE;
    }
    else if (bits1 > bits2)
    {
        return EGL_TRUE;
    }

    /*
     * Priority 11.
     * Smaller EGL_CONFIG_ID (this is always the last sorting rule, and guarantees
     * a unique ordering).
     */
    if (Config1->configId < Config2->configId)
    {
        return EGL_FALSE;
    }
    else if (Config1->configId > Config2->configId)
    {
        return EGL_TRUE;
    }

    /* Nothing to sort. */
    return EGL_FALSE;
}

static void
veglSort(
    VEGLConfig Configs,
    EGLint *IndexTable,
    EGLint ConfigCount,
    VEGLConfig Attributes
    )
{
    EGLBoolean swapped;
    EGLint i;

    do
    {
        /* Assume no sorting has happened. */
        swapped = EGL_FALSE;

        /* Loop through all configurations. */
        for (i = 0; i < ConfigCount - 1; i++)
        {
            /* Do we need to swap the current and next configuration? */
            if (veglSortAfter(&Configs[IndexTable[i]], &Configs[IndexTable[i + 1]], Attributes))
            {
                /* Swap configurations. */
                EGLint temp = IndexTable[i];
                IndexTable[i] = IndexTable[i + 1];
                IndexTable[i + 1] = temp;

                /* We need another pass. */
                swapped = EGL_TRUE;
            }
        }
    }
    while (swapped);
}

EGLAPI EGLBoolean EGLAPIENTRY
eglGetConfigs(
    EGLDisplay Dpy,
    EGLConfig *configs,
    EGLint config_size,
    EGLint *num_config
    )
{
    VEGLThreadData thread;
    VEGLDisplay dpy;
    gceSTATUS status;

    gcmHEADER_ARG("Dpy=0x%x configs=0x%x config_size=%d",
                  Dpy, configs, config_size);

    VEGL_TRACE_API_PRE(GetConfigs)(Dpy, configs, config_size, num_config);

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
        return EGL_FALSE;
    }

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Dpy);
    if (dpy == gcvNULL)
    {
        /* Bad display. */
        veglSetEGLerror(thread,  EGL_BAD_DISPLAY);

        gcmFOOTER_ARG("return=%d", EGL_FALSE);
        return EGL_FALSE;
    }

    VEGL_LOCK_DISPLAY(dpy);

    /* Test for initialized or not. */
    if (!dpy->initialized)
    {
        veglSetEGLerror(thread,  EGL_NOT_INITIALIZED);;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Hardware relevant thread data initialization. */
    veglInitDeviceThreadData(thread);

    if (num_config == gcvNULL)
    {
        /* Bad parameter. */
        veglSetEGLerror(thread,  EGL_BAD_PARAMETER);;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (configs == gcvNULL)
    {
        /* Return number of configurations. */
        *num_config = dpy->configCount;
    }
    else
    {
        EGLint index;

        /* Copy pointers to configurations into supplied buffer. */
        for (index = 0; index < dpy->configCount; index++)
        {
            /* Bail out if the supplied buffer is too small. */
            if (index >= config_size)
            {
                break;
            }

            configs[index] = (EGLConfig)(intptr_t)(index + 1);
        }

        *num_config = index;

        /*  any remaining configurations in the supplied buffer. */
        while (index < config_size)
        {
            configs[index++] = (EGLConfig)__EGL_INVALID_CONFIG__;
        }
    }

    VEGL_UNLOCK_DISPLAY(dpy);
    /* Success. */
    veglSetEGLerror(thread,  EGL_SUCCESS);

    VEGL_TRACE_API_POST(GetConfigs)(Dpy, configs, config_size, num_config);
    gcmDUMP_API("${EGL eglGetConfigs 0x%08X (0x%08X) 0x%08X (0x%08X)",
                Dpy, configs, config_size, num_config);
    gcmDUMP_API_ARRAY(configs, *num_config);
    gcmDUMP_API_ARRAY(num_config, 1);
    gcmDUMP_API("$}");

    gcmFOOTER_ARG("*num_config=%d", *num_config);
    return EGL_TRUE;

OnError:
    VEGL_UNLOCK_DISPLAY(dpy);

    gcmFOOTER_ARG("return=%d", EGL_FALSE);
    return EGL_FALSE;
}


EGLAPI EGLBoolean EGLAPIENTRY
eglChooseConfig(
    EGLDisplay Dpy,
    const EGLint *attrib_list,
    EGLConfig *configs,
    EGLint config_size,
    EGLint *num_config
    )
{
    VEGLThreadData thread;
    VEGLDisplay dpy;
    struct eglConfig criteria = { 0 };
    EGLint config;
    EGLint candidates[128];
    EGLint matchCount;
    gceSURF_FORMAT configFormat = gcvSURF_A8R8G8B8;
    gceSTATUS status;
    gcePATCH_ID patchId = gcvPATCH_INVALID;

    gcmHEADER_ARG("Dpy=0x%x attrib_list=0x%x configs=0x%x config_size=%d",
                  Dpy, attrib_list, configs, config_size);

    VEGL_TRACE_API_PRE(ChooseConfig)(Dpy, attrib_list, configs, config_size, num_config);

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
        return EGL_FALSE;
    }

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Dpy);
    if (dpy == gcvNULL)
    {
        /* Bad display. */
        veglSetEGLerror(thread,  EGL_BAD_DISPLAY);

        gcmFOOTER_ARG("return=%d", EGL_FALSE);
        return EGL_FALSE;
    }

    VEGL_LOCK_DISPLAY(dpy);

    /* Test for initialized or not. */
    if (!dpy->initialized)
    {
        veglSetEGLerror(thread,  EGL_NOT_INITIALIZED);;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (num_config == gcvNULL)
    {
        /* Bad parameter. */
        veglSetEGLerror(thread,  EGL_BAD_PARAMETER);;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Parse attributes. */
    if (!veglParseAttributes(dpy, attrib_list, &criteria))
    {
        /* Bail out on invalid or non-matching attributes. */
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Reset number of configurations. */
    matchCount = 0;
    gcoHAL_GetPatchID(gcvNULL, &patchId);

    /* Walk through all configurations. */
    for (config = 0; config < dpy->configCount; config++)
    {
        /* Get pointer to configuration. */
        VEGLConfig configuration = &dpy->config[config];

        gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                      "%s: examining config index=%d",
                      __FUNCTION__, config);

        if (criteria.configId != EGL_DONT_CARE)
        {
            if (!((criteria.configId == configuration->configId) ||
                ((criteria.configId == 0) && configuration->defaultConfig)))
            {
                /* Criterium doesn't match. */
                gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                    "  rejected on config ID.");
                continue;
            }
            else
            {
                /* Copy configuration into specified buffer. */
                candidates[matchCount++] = config;

                break;
            }
        }

        /* Check configuration against criteria. */
        if (criteria.configBufferSize > configuration->configBufferSize)
        {
            /* Criterium doesn't match. */
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "  rejected on buffer size.");
            continue;
        }

        if (criteria.alphaSize > configuration->alphaSize)
        {
            /* Criterium doesn't match. */
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "  rejected on alpha size.");
            continue;
        }

        if (criteria.blueSize > configuration->blueSize)
        {
            /* Criterium doesn't match. */
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "  rejected on blue size.");
            continue;
        }

        if (criteria.greenSize > configuration->greenSize)
        {
            /* Criterium doesn't match. */
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "  rejected on green size.");
            continue;
        }

        if (criteria.redSize > configuration->redSize)
        {
            /* Criterium doesn't match. */
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "  rejected on red size.");
            continue;
        }

        if (criteria.depthSize > configuration->depthSize)
        {
            /* Criterium doesn't match. */
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "  rejected on depth size.");
            continue;
        }

        if (criteria.stencilSize > configuration->stencilSize)
        {
            /* Criterium doesn't match. */
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "  rejected on stencil size.");
            continue;
        }

        if ((criteria.configCaveat != (EGLenum) EGL_DONT_CARE)
        &&  (criteria.configCaveat != configuration->configCaveat)
        )
        {
            /* Criterium doesn't match. */
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "  rejected on config caveat.");
            continue;
        }

        if ((criteria.nativeRenderable != (EGLBoolean) EGL_DONT_CARE)
        &&  (patchId == gcvPATCH_DEQP || patchId == gcvPATCH_GTFES30 || criteria.nativeRenderable) /* Do patch to follow spec for dEQP test */
        &&  (criteria.nativeRenderable != configuration->nativeRenderable)
        )
        {
            /* Criterium doesn't match. */
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "  rejected on native renderable.");
            continue;
        }

        if ((criteria.nativeVisualType != EGL_DONT_CARE) &&
            (criteria.nativeVisualType != configuration->nativeVisualType))
        {
            /* Criterium doesn't match. */
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "  rejected on native visual type.");
            continue;
        }

        if (criteria.samples > configuration->samples)
        {
            /* Do patch to follow spec for dEQP test */
            if (!(patchId == gcvPATCH_DEQP || patchId == gcvPATCH_GTFES30) && ((criteria.samples == 1) && (configuration->samples == 0)))
            {
                /* Criterium still matches. */
            }
            else
            {
                /* Criterium doesn't match. */
                gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                              "  rejected on samples.");
                continue;
            }
        }

        if (criteria.sampleBuffers > configuration->sampleBuffers)
        {
            /* Criterium doesn't match. */
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "  rejected on sample buffers.");
            continue;
        }

        if ((criteria.surfaceType != (EGLenum) EGL_DONT_CARE)
        && ((criteria.surfaceType & configuration->surfaceType) != criteria.surfaceType)
        )
        {
            /* Criterium doesn't match. */
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "  rejected on surface type.");
            continue;
        }

        if ((criteria.bindToTetxureRGB != (EGLBoolean) EGL_DONT_CARE)
        &&  (criteria.bindToTetxureRGB != configuration->bindToTetxureRGB)
        )
        {
            /* Criterium doesn't match. */
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "  rejected on bind to tetxure RGB.");
            continue;
        }

        if ((criteria.bindToTetxureRGBA != (EGLBoolean) EGL_DONT_CARE)
        &&  (criteria.bindToTetxureRGBA != configuration->bindToTetxureRGBA)
        )
        {
            /* Criterium doesn't match. */
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "  rejected on bind to tetxure RGBA.");
            continue;
        }

        if (criteria.luminanceSize > configuration->luminanceSize)
        {
            /* Criterium doesn't match. */
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "  rejected on luminance size.");
            continue;
        }

        if (criteria.alphaMaskSize > configuration->alphaMaskSize)
        {
            /* Criterium doesn't match. */
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "  rejected on alpha mask size.");
            continue;
        }

        if ((criteria.colorBufferType != (EGLenum) EGL_DONT_CARE)
        &&  (criteria.colorBufferType != configuration->colorBufferType)
        )
        {
            /* Criterium doesn't match. */
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "  rejected on color buffer type.");
            continue;
        }

        if ((criteria.renderableType != (EGLenum) EGL_DONT_CARE)
        &&  ((criteria.renderableType & configuration->renderableType)
             != criteria.renderableType)
        )
        {
            /* Criterium doesn't match. */
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "  rejected on renderable type.");
            continue;
        }

        if ((criteria.conformant != (EGLenum) EGL_DONT_CARE)
        &&  (criteria.conformant != 0)
        &&  !(criteria.conformant & configuration->conformant)
        )
        {
            /* Criterium doesn't match. */
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "  rejected on conformant.");
            continue;
        }

        if ((criteria.matchNativePixmap != EGL_DONT_CARE)
        &&  (criteria.matchNativePixmap != EGL_NONE)
        )
        {
            NativePixmapType pixmap;

            if (!(configuration->surfaceType & EGL_PIXMAP_BIT))
            {
                /* surface type of config is not EGL_PIXMAP_BIT or bad pixmap. */
                gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                              "  rejected on match native pixmap.");
                continue;
            }

            pixmap = (NativePixmapType) gcmINT2PTR(criteria.matchNativePixmap);

            if (!dpy->platform->matchPixmap(dpy, (void *) pixmap, configuration))
            {
                /* Criterium doesn't match. */
                gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                              "  rejected on match native pixmap.");
                continue;
            }
        }

        if ((criteria.matchFormat != (EGLenum) EGL_DONT_CARE)
        &&  (criteria.matchFormat != (EGLenum) EGL_NONE)
        )
        {
            EGLBoolean match = EGL_TRUE;

            switch(criteria.matchFormat)
            {
            case EGL_FORMAT_RGB_565_EXACT_KHR:
                if (configuration->matchFormat != EGL_FORMAT_RGB_565_EXACT_KHR)
                {
                    match = EGL_FALSE;
                }
                break;

            case EGL_FORMAT_RGB_565_KHR:
                if ((configuration->matchFormat != EGL_FORMAT_RGB_565_EXACT_KHR)
                &&  (configuration->matchFormat != EGL_FORMAT_RGB_565_KHR)
                )
                {
                    match = EGL_FALSE;
                }
                break;

            case EGL_FORMAT_RGBA_8888_EXACT_KHR:
                if (configuration->matchFormat != EGL_FORMAT_RGBA_8888_EXACT_KHR)
                {
                    match = EGL_FALSE;
                }
                break;

            case EGL_FORMAT_RGBA_8888_KHR:
                if ((configuration->matchFormat != EGL_FORMAT_RGBA_8888_EXACT_KHR)
                &&  (configuration->matchFormat != EGL_FORMAT_RGBA_8888_KHR)
                )
                {
                    match = EGL_FALSE;
                }
                break;

            default:
                break;
            }

            if (!match)
            {
                /* Criterium doesn't match. */
                gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                              "  rejected on conformant.");
                continue;
            }
        }

        if ((criteria.recordableConfig != (EGLBoolean) EGL_DONT_CARE)
        &&  (criteria.recordableConfig != configuration->recordableConfig)
        )
        {
            /* Criterium doesn't match. */
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "  rejected on recordable config.");
            continue;
        }

        if ((criteria.level != (EGLint) EGL_DONT_CARE)
        &&  (criteria.level != configuration->level)
        )
        {
            /* Criterium doesn't match. */
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "  rejected on eglLevel config.");
            continue;
        }

        if ((criteria.minSwapInterval != (EGLint) EGL_DONT_CARE)
        &&  (criteria.minSwapInterval != configuration->minSwapInterval)
        )
        {
            /* Criterium doesn't match. */
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "  rejected on minSwapInterval config.");
            continue;
        }

        if ((criteria.maxSwapInterval != (EGLint) EGL_DONT_CARE)
        &&  (criteria.maxSwapInterval != configuration->maxSwapInterval)
        )
        {
            /* Criterium doesn't match. */
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "  rejected on maxSwapInterval config.");
            continue;
        }

#if defined(ANDROID)
        if ((criteria.supportFBTarget != (EGLint) EGL_DONT_CARE)
        &&  (criteria.supportFBTarget != configuration->supportFBTarget)
        )
        {
            /* Criterium doesn't match. */
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                          "  rejected on EGL_ANDROID_framebuffer_target config.");
            continue;
        }
#endif

        if (criteria.transparentType != (EGLenum) EGL_DONT_CARE)
        {
            if (criteria.transparentType != configuration->transparentType)
            {
                /* Criterium doesn't match. */
                gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                              "  rejected on transparentType config.");
                continue;
            }

            if (criteria.transparentType != (EGLenum) EGL_NONE)
            {
                if ((criteria.transparentRedValue != (EGLint) EGL_DONT_CARE)
                && (criteria.transparentRedValue != configuration->transparentRedValue)
                )
                {
                    /* Criterium doesn't match. */
                    gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                                  "  rejected on transparent Red value config.");
                    continue;
                }

                if ((criteria.transparentGreenValue != (EGLint) EGL_DONT_CARE)
                &&  (criteria.transparentGreenValue != configuration->transparentGreenValue)
                )
                {
                    /* Criterium doesn't match. */
                    gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                                  "  rejected on transparent Green value config.");
                    continue;
                }

                if ((criteria.transparentBlueValue != (EGLint) EGL_DONT_CARE)
                &&  (criteria.transparentBlueValue != configuration->transparentBlueValue)
                )
                {
                    /* Criterium doesn't match. */
                    gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG,
                                  "  rejected on transparent Blue value config.");
                    continue;
                }
            }
        }

        gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_EGL_CONFIG, "  accepted.");

        if (!(criteria.surfaceType & EGL_LOCK_SURFACE_BIT_KHR))
        {
            configuration->surfaceType &= ~EGL_LOCK_SURFACE_BIT_KHR;
        }

        /* Copy configuration into specified buffer. */
        candidates[matchCount++] = config;
    }

    if ((matchCount >= 1) && (config_size >= 1) && (configs != gcvNULL))
    {
        EGLint hiCandidate = 0;
        /* Sort the matching configurations. */
        veglSort(dpy->config, candidates, matchCount, &criteria);


        /* Copy configuration out. */
        for (config = 0; (config < config_size) && (config < matchCount); config++)
        {
            configs[config] = (EGLConfig) gcmINT2PTR(candidates[config] + 1);
        }

        hiCandidate = candidates[0];
        veglGetFormat(thread, &dpy->config[hiCandidate], &configFormat, gcvNULL);
        /* Obtain num_config. */
        *num_config = config;
    }
    else
    {
        /* Return number of configs. */
        *num_config = matchCount;
    }

    /* Set it to PLS to fetch it later. */
    gcoOS_SetPLSValue(gcePLS_VALUE_EGL_CONFIG_FORMAT_INFO, gcmINT2PTR(configFormat));

    VEGL_UNLOCK_DISPLAY(dpy);
    /* Success. */
    veglSetEGLerror(thread,  EGL_SUCCESS);

    VEGL_TRACE_API_POST(ChooseConfig)(Dpy, attrib_list, configs, config_size, num_config);
    gcmDUMP_API("${EGL eglChooseConfig 0x%08X (0x%08X) (0x%08X) 0x%08X "
                "(0x%08X)",
                Dpy, attrib_list, configs, config_size, num_config);
    gcmDUMP_API_ARRAY_TOKEN(attrib_list, EGL_NONE);
    gcmDUMP_API_ARRAY(configs, *num_config);
    gcmDUMP_API_ARRAY(num_config, 1);
    gcmDUMP_API("$}");
    gcmFOOTER_ARG("*num_config=%d", *num_config);

    return EGL_TRUE;

OnError:

    VEGL_UNLOCK_DISPLAY(dpy);

    gcmFOOTER_ARG("return=%d", EGL_FALSE);
    return EGL_FALSE;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglGetConfigAttrib(
    EGLDisplay Dpy,
    EGLConfig Config,
    EGLint attribute,
    EGLint *value
    )
{
    VEGLThreadData thread;
    VEGLDisplay dpy;
    VEGLConfig eglConfig;
    gceSTATUS status;

    gcmHEADER_ARG("Dpy=0x%x Config=0x%x attribute=%d", Dpy, Config, attribute);

    VEGL_TRACE_API_PRE(GetConfigAttrib)(Dpy, Config, attribute, value);

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
        return EGL_FALSE;
    }

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Dpy);
    if (dpy == gcvNULL)
    {
        /* Bad display. */
        veglSetEGLerror(thread,  EGL_BAD_DISPLAY);

        gcmFOOTER_ARG("return=%d", EGL_FALSE);
        return EGL_FALSE;
    }

    VEGL_LOCK_DISPLAY(dpy);

    /* Test for initialized or not. */
    if (!dpy->initialized)
    {
        veglSetEGLerror(thread,  EGL_NOT_INITIALIZED);;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Hardware relevant thread data initialization. */
    veglInitDeviceThreadData(thread);

    /* Test for valid config. */
    if (((EGLint)(intptr_t)Config <= __EGL_INVALID_CONFIG__)
    ||  ((EGLint)(intptr_t)Config > dpy->configCount)
    )
    {
        veglSetEGLerror(thread,  EGL_BAD_CONFIG);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    eglConfig = VEGL_CONFIG(&dpy->config[(EGLint)(intptr_t)Config - 1]);

    if (value == gcvNULL)
    {
        /* Bad parameter. */
        veglSetEGLerror(thread,  EGL_BAD_PARAMETER);;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    switch (attribute)
    {
    case EGL_BUFFER_SIZE:
        *value = eglConfig->configBufferSize;
        break;

    case EGL_ALPHA_SIZE:
        *value = eglConfig->alphaSize;
        break;

    case EGL_BLUE_SIZE:
        *value = eglConfig->blueSize;
        break;

    case EGL_GREEN_SIZE:
        *value = eglConfig->greenSize;
        break;

    case EGL_RED_SIZE:
        *value = eglConfig->redSize;
        break;

    case EGL_DEPTH_SIZE:
        *value = eglConfig->depthSize;
        break;

    case EGL_STENCIL_SIZE:
        *value = eglConfig->stencilSize;
        break;

    case EGL_CONFIG_CAVEAT:
        *value = eglConfig->configCaveat;
        break;

    case EGL_CONFIG_ID:
        *value = eglConfig->configId;
        break;

    case EGL_LEVEL:
        *value = 0;
        break;

    case EGL_MAX_PBUFFER_WIDTH:
        *value = thread->maxWidth;
        break;

    case EGL_MAX_PBUFFER_HEIGHT:
        *value = thread->maxHeight;
        break;

    case EGL_MAX_PBUFFER_PIXELS:
        *value = thread->maxWidth * thread->maxHeight;
        break;

    case EGL_NATIVE_RENDERABLE:
        *value = eglConfig->nativeRenderable;
        break;

    case EGL_NATIVE_VISUAL_ID:
        *value = dpy->platform->getNativeVisualId(dpy, eglConfig);
        break;

    case EGL_NATIVE_VISUAL_TYPE:
        *value = eglConfig->nativeVisualType;
        break;

    case EGL_SAMPLES:
        *value = eglConfig->samples;
        break;

    case EGL_SAMPLE_BUFFERS:
        if ((eglConfig->samples == 16) && (thread->api == EGL_OPENVG_API))
        {
            *value = 0;
        }
        else
        {
            *value = eglConfig->sampleBuffers;
        }
        break;

    case EGL_SURFACE_TYPE:
        *value = eglConfig->surfaceType;
        break;

    case EGL_TRANSPARENT_TYPE:
        *value = EGL_NONE;
        break;

    case EGL_TRANSPARENT_BLUE_VALUE:
    case EGL_TRANSPARENT_GREEN_VALUE:
    case EGL_TRANSPARENT_RED_VALUE:
        *value = EGL_DONT_CARE;
        break;

    case EGL_BIND_TO_TEXTURE_RGB:
        *value = eglConfig->bindToTetxureRGB;
        break;

    case EGL_BIND_TO_TEXTURE_RGBA:
        *value = eglConfig->bindToTetxureRGBA;
        break;

    case EGL_MIN_SWAP_INTERVAL:
        *value = eglConfig->minSwapInterval;
        break;

    case EGL_MAX_SWAP_INTERVAL:
        *value = eglConfig->maxSwapInterval;
        break;

#if defined(ANDROID)
    case EGL_ANDROID_framebuffer_target:
        *value = eglConfig->supportFBTarget;
        break;
#endif

    case EGL_LUMINANCE_SIZE:
        *value = eglConfig->luminanceSize;
        break;

    case EGL_ALPHA_MASK_SIZE:
        *value = eglConfig->alphaMaskSize;
        break;

    case EGL_COLOR_BUFFER_TYPE:
        *value = eglConfig->colorBufferType;
        break;

    case EGL_RENDERABLE_TYPE:
        *value = eglConfig->renderableType;
        break;

    case EGL_CONFORMANT:
        *value = eglConfig->conformant;
        break;

    case EGL_MATCH_NATIVE_PIXMAP:
        *value = eglConfig->matchNativePixmap;
        break;

    case EGL_RECORDABLE_ANDROID:
        *value = eglConfig->recordableConfig;
        break;

    default:
        /* Bad attribute. */
        veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    VEGL_UNLOCK_DISPLAY(dpy);
    /* Success. */
    veglSetEGLerror(thread,  EGL_SUCCESS);

    VEGL_TRACE_API_POST(GetConfigAttrib)(Dpy, Config, attribute, value);

    gcmDUMP_API("${EGL eglGetConfigAttrib 0x%08X 0x%08X 0x%08X := 0x%08X}",
                Dpy, Config, attribute, *value);
    gcmFOOTER_ARG("*value=%d", *value);

    return EGL_TRUE;

OnError:
    VEGL_UNLOCK_DISPLAY(dpy);

    gcmFOOTER_ARG("return=%d", EGL_FALSE);
    return EGL_FALSE;
}
