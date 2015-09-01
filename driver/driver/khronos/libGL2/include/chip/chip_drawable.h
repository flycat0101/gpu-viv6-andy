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


#ifndef __chip_drawable_h_
#define __chip_drawable_h_
#ifdef __cplusplus
extern "C" {
#endif

/* Index for Drawbuffers*/
#define __GL_FRONT_BUFFER_INDEX          __GL_DRAWBUFFER_FRONTLEFT_INDEX
#define __GL_BACK_BUFFER0_INDEX          __GL_DRAWBUFFER_BACKLEFT_INDEX
#define __GL_BACK_BUFFER1_INDEX          __GL_DRAWBUFFER_BACKRIGHT_INDEX

/* Set current RT view for specific RT */
GLvoid setCurrentSurf(__GLpBufferTexture* pbufferTex, glsCHIPRENDERTEXTURE * chipRenderTexture);

#define __GL_SET_RENDERTARGET_FROM_DRAWBUFFER(index)\
        if(*(chipDrawable->drawBuffers[index]))\
        {\
            if ((drawable->type == __GL_PBUFFER) && pbufferTex->renderTexture)\
            {\
                chipRenderTexture = (glsCHIPRENDERTEXTURE*)*chipDrawable->drawBuffers[index];\
                setCurrentSurf(pbufferTex, chipRenderTexture);\
            }\
            rtSurfs[NumRtSurf++] = (*(chipDrawable->drawBuffers[index]))->renderTarget;\
        }

#define __GL_SET_RENDERTARGET_FROM_READBUFFER(index)\
        if(*(chipReadable->drawBuffers[index]))\
        {\
            if ((readable->type == __GL_PBUFFER) && pbufferTex->renderTexture)\
            {\
                chipRenderTexture = (glsCHIPRENDERTEXTURE*)*chipReadable->drawBuffers[index];\
                setCurrentSurf(pbufferTex, chipRenderTexture);\
            }\
            rtSurf = (*(chipReadable->drawBuffers[index]))->renderTarget;\
        }

typedef struct _glsSwapInfo {
    /* Swap surface. */
    gcoSURF swapSurface;
    GLuint  swapBitsPerPixel;
    GLuint  bitsAlignedWidth;
    GLuint  bitsAlignedHeight;
    GLvoid * swapBits;
} glsSwapInfo;

typedef struct _glsCHIPDRAWABLE
{
    __GLcontext * gc;
    /* Set if VG pipe is present and this surface was created when the current
       API was set to OpenVG. */
    gctBOOL openVG;
    /* Direct frame buffer access. */
    gcoSURF fbWrapper;
    gctBOOL fbDirect;

    GLint width;
    GLint height;
    GLenum internalFormatColorBuffer;

    /* Render target array. */
    gctBOOL   renderListEnable;
    gctUINT   renderListCount;

    GLboolean haveRenderBuffer;
    GLboolean haveDepthBuffer;
    GLboolean haveStencilBuffer;
    GLboolean haveAccumBuffer;

    glsCHIPRENDERBUFFER * originalFrontBuffer;
    glsCHIPRENDERBUFFER* originalFrontBuffer1;

    glsCHIPRENDERBUFFER** drawBuffers[__GL_MAX_DRAW_BUFFERS];
    glsCHIPDEPTHBUFFER * depthBuffer;
    glsCHIPSTENCILBUFFER * stencilBuffer;
    glsCHIPACCUMBUFFER * accumBuffer;

    /* Resolve surface. */
    glsCHIPRENDERBUFFER*  resolveBuffer;

    glsSwapInfo swapInfo;

#if DIRECT_TO_FB
    glsCHIPRENDERBUFFER   displayBuffer;
#endif

    /* Image swap workers. */
    struct glsWorkerInfo       workers[WORKER_COUNT];
    glsWORKINFO_PTR            availableWorkers;
    glsWORKINFO_PTR            lastSubmittedWorker;
    gctPOINTER                 workerMutex;

} glsCHIPDRAWABLE;

typedef glsCHIPDRAWABLE glsCHIPREADABLE;
typedef glsCHIPDRAWABLE * glsCHIPDRAWABLE_PTR;
typedef glsCHIPREADABLE * glsCHIPREADABLE_PTR;

extern GLvoid __glChipNotifyChangeBufferSize(__GLcontext * gc);
extern GLvoid __glChipNotifyDestroyBuffers(__GLcontext *gc);
extern GLvoid __glChipNotifyDrawableSwitch(__GLcontext *gc);

extern GLvoid __glChipDrawBuffers(__GLcontext * gc);
extern GLvoid __glChipReadBuffer(__GLcontext * gc);

#ifdef __cplusplus
}
#endif
#endif /* __chip_drawable_h_ */
