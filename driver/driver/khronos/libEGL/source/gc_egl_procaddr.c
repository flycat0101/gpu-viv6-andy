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


#include "gc_egl_precomp.h"
#include <GLES3/gl32.h>
#include <GLES2/gl2ext.h>

#define VIV_EGL_BUILD
#include "../../libGLESv11/gc_glff_functions.h"
#include "../../libGLESv3/include/glcore/gc_es_dispatch.h"
#include "../../libGL4/include/glcore/gc_es_dispatch.h"
#include "../../libOpenVG_3D/vg11/driver/gc_vgsh_dump.h"
#undef VIV_EGL_BUILD

/* Zone used for header/footer. */
#define _GC_OBJ_ZONE    gcdZONE_EGL_API


#define COMMON_ES_INDEX(_api) _api##_index
#define COMMON_ES_API(_api) forward_##_api

#define __GLES_COMMON_API(esApiMacro, _init) \
    esApiMacro(ActiveTexture)_init, \
    esApiMacro(BindBuffer), \
    esApiMacro(BindTexture), \
    esApiMacro(BlendFunc), \
    esApiMacro(BufferData), \
    esApiMacro(BufferSubData), \
    esApiMacro(Clear), \
    esApiMacro(ClearColor), \
    esApiMacro(ClearDepthf), \
    esApiMacro(ClearStencil), \
    esApiMacro(ColorMask), \
    esApiMacro(CompressedTexImage2D), \
    esApiMacro(CompressedTexSubImage2D), \
    esApiMacro(CopyTexImage2D), \
    esApiMacro(CopyTexSubImage2D), \
    esApiMacro(CullFace), \
    esApiMacro(DeleteBuffers), \
    esApiMacro(DeleteTextures), \
    esApiMacro(DepthFunc), \
    esApiMacro(DepthMask), \
    esApiMacro(DepthRangef), \
    esApiMacro(Disable), \
    esApiMacro(DrawArrays), \
    esApiMacro(DrawElements), \
    esApiMacro(Enable), \
    esApiMacro(Finish), \
    esApiMacro(Flush), \
    esApiMacro(FrontFace), \
    esApiMacro(GenBuffers), \
    esApiMacro(GenTextures), \
    esApiMacro(GetBooleanv), \
    esApiMacro(GetBufferParameteriv), \
    esApiMacro(GetError), \
    esApiMacro(GetFloatv), \
    esApiMacro(GetIntegerv), \
    esApiMacro(GetPointerv), \
    esApiMacro(GetString), \
    esApiMacro(GetTexParameterfv), \
    esApiMacro(GetTexParameteriv), \
    esApiMacro(Hint), \
    esApiMacro(IsBuffer), \
    esApiMacro(IsEnabled), \
    esApiMacro(IsTexture), \
    esApiMacro(LineWidth), \
    esApiMacro(PixelStorei), \
    esApiMacro(PolygonOffset), \
    esApiMacro(ReadPixels), \
    esApiMacro(SampleCoverage), \
    esApiMacro(Scissor), \
    esApiMacro(StencilFunc), \
    esApiMacro(StencilMask), \
    esApiMacro(StencilOp), \
    esApiMacro(TexImage2D), \
    esApiMacro(TexParameterf), \
    esApiMacro(TexParameterfv), \
    esApiMacro(TexParameteri), \
    esApiMacro(TexParameteriv), \
    esApiMacro(TexSubImage2D), \
    esApiMacro(Viewport), \
    esApiMacro(MapBufferOES), \
    esApiMacro(UnmapBufferOES), \
    esApiMacro(EGLImageTargetTexture2DOES), \
    esApiMacro(EGLImageTargetRenderbufferStorageOES), \
    esApiMacro(MultiDrawArraysEXT), \
    esApiMacro(MultiDrawElementsEXT),

enum {
    __GLES_COMMON_API(COMMON_ES_INDEX,=0)
};

static EGL_PROC _GetCommonGlesApiProc(EGLint index);

#define CALL_ES_API(_fptype, _api, ...) \
{ \
    _fptype apiProc = (_fptype)_GetCommonGlesApiProc(_api##_index); \
    if (apiProc) (apiProc)(__VA_ARGS__); \
}

#define CALL_ES_API_RETURN(_fptype, _api, ...) \
{ \
    _fptype apiProc = (_fptype)_GetCommonGlesApiProc(_api##_index); \
    if (apiProc) return (apiProc)(__VA_ARGS__); \
    return 0; \
}

void GL_APIENTRY COMMON_ES_API(glActiveTexture)(GLenum texture)
{
    CALL_ES_API(PFNGLACTIVETEXTUREPROC, ActiveTexture, texture)
}

void GL_APIENTRY COMMON_ES_API(glBindBuffer)(GLenum target, GLuint buffer)
{
    CALL_ES_API(PFNGLBINDBUFFERPROC, BindBuffer, target, buffer);
}

void GL_APIENTRY COMMON_ES_API(glBindTexture)(GLenum target, GLuint texture)
{
    CALL_ES_API(PFNGLBINDTEXTUREPROC, BindTexture, target, texture);
}

void GL_APIENTRY COMMON_ES_API(glBlendFunc)(GLenum sfactor, GLenum dfactor)
{
    CALL_ES_API(PFNGLBLENDFUNCPROC, BlendFunc, sfactor, dfactor);
}

void GL_APIENTRY COMMON_ES_API(glBufferData)(GLenum target, GLsizeiptr size, const void * data, GLenum usage)
{
    CALL_ES_API(PFNGLBUFFERDATAPROC, BufferData, target, size, data, usage);
}

void GL_APIENTRY COMMON_ES_API(glBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, const void * data)
{
    CALL_ES_API(PFNGLBUFFERSUBDATAPROC, BufferSubData, target, offset, size, data);
}

void GL_APIENTRY COMMON_ES_API(glClear)(GLbitfield mask)
{
    CALL_ES_API(PFNGLCLEARPROC, Clear, mask);
}

void GL_APIENTRY COMMON_ES_API(glClearColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    CALL_ES_API(PFNGLCLEARCOLORPROC, ClearColor, red, green, blue, alpha);
}

void GL_APIENTRY COMMON_ES_API(glClearDepthf)(GLfloat depth)
{
    CALL_ES_API(PFNGLCLEARDEPTHFPROC, ClearDepthf, depth);
}

void GL_APIENTRY COMMON_ES_API(glClearStencil)(GLint s)
{
    CALL_ES_API(PFNGLCLEARSTENCILPROC, ClearStencil, s);
}

void GL_APIENTRY COMMON_ES_API(glColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    CALL_ES_API(PFNGLCOLORMASKPROC, ColorMask, red, green, blue, alpha);
}

void GL_APIENTRY COMMON_ES_API(glCompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void * data)
{
    CALL_ES_API(PFNGLCOMPRESSEDTEXIMAGE2DPROC, CompressedTexImage2D, target, level, internalformat, width, height, border, imageSize, data);
}

void GL_APIENTRY COMMON_ES_API(glCompressedTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void * data)
{
    CALL_ES_API(PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC, CompressedTexSubImage2D, target, level, xoffset, yoffset, width, height, format, imageSize, data);
}

void GL_APIENTRY COMMON_ES_API(glCopyTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    CALL_ES_API(PFNGLCOPYTEXIMAGE2DPROC, CopyTexImage2D, target, level, internalformat, x, y, width, height, border);
}

void GL_APIENTRY COMMON_ES_API(glCopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    CALL_ES_API(PFNGLCOPYTEXSUBIMAGE2DPROC, CopyTexSubImage2D, target, level, xoffset, yoffset, x, y, width, height);
}

void GL_APIENTRY COMMON_ES_API(glCullFace)(GLenum mode)
{
    CALL_ES_API(PFNGLCULLFACEPROC, CullFace, mode);
}

void GL_APIENTRY COMMON_ES_API(glDeleteBuffers)(GLsizei n, const GLuint * buffers)
{
    CALL_ES_API(PFNGLDELETEBUFFERSPROC, DeleteBuffers, n, buffers);
}

void GL_APIENTRY COMMON_ES_API(glDeleteTextures)(GLsizei n, const GLuint * textures)
{
    CALL_ES_API(PFNGLDELETETEXTURESPROC, DeleteTextures, n, textures);
}

void GL_APIENTRY COMMON_ES_API(glDepthFunc)(GLenum func)
{
    CALL_ES_API(PFNGLDEPTHFUNCPROC, DepthFunc, func);
}

void GL_APIENTRY COMMON_ES_API(glDepthMask)(GLboolean flag)
{
    CALL_ES_API(PFNGLDEPTHMASKPROC, DepthMask, flag);
}

void GL_APIENTRY COMMON_ES_API(glDepthRangef)(GLfloat n, GLfloat f)
{
    CALL_ES_API(PFNGLDEPTHRANGEFPROC, DepthRangef, n, f);
}

void GL_APIENTRY COMMON_ES_API(glDisable)(GLenum cap)
{
    CALL_ES_API(PFNGLDISABLEPROC, Disable, cap);
}

void GL_APIENTRY COMMON_ES_API(glDrawArrays)(GLenum mode, GLint first, GLsizei count)
{
    CALL_ES_API(PFNGLDRAWARRAYSPROC, DrawArrays, mode, first, count);
}

void GL_APIENTRY COMMON_ES_API(glDrawElements)(GLenum mode, GLsizei count, GLenum type, const void * indices)
{
    CALL_ES_API(PFNGLDRAWELEMENTSPROC, DrawElements, mode, count, type, indices);
}

void GL_APIENTRY COMMON_ES_API(glEnable)(GLenum cap)
{
    CALL_ES_API(PFNGLENABLEPROC, Enable, cap);
}

void GL_APIENTRY COMMON_ES_API(glFinish)(void)
{
    CALL_ES_API(PFNGLFINISHPROC, Finish);
}

void GL_APIENTRY COMMON_ES_API(glFlush)(void)
{
    CALL_ES_API(PFNGLFLUSHPROC, Flush);
}

void GL_APIENTRY COMMON_ES_API(glFrontFace)(GLenum mode)
{
    CALL_ES_API(PFNGLFRONTFACEPROC, FrontFace, mode);
}

void GL_APIENTRY COMMON_ES_API(glGenBuffers)(GLsizei n, GLuint * buffers)
{
    CALL_ES_API(PFNGLGENBUFFERSPROC, GenBuffers, n, buffers);
}

void GL_APIENTRY COMMON_ES_API(glGenTextures)(GLsizei n, GLuint * textures)
{
    CALL_ES_API(PFNGLGENTEXTURESPROC, GenTextures, n, textures);
}

void GL_APIENTRY COMMON_ES_API(glGetBooleanv)(GLenum pname, GLboolean * data)
{
    CALL_ES_API(PFNGLGETBOOLEANVPROC, GetBooleanv, pname, data);
}

void GL_APIENTRY COMMON_ES_API(glGetBufferParameteriv)(GLenum target, GLenum pname, GLint * params)
{
    CALL_ES_API(PFNGLGETBUFFERPARAMETERIVPROC, GetBufferParameteriv, target, pname, params);
}

GLenum GL_APIENTRY COMMON_ES_API(glGetError)(void)
{
    CALL_ES_API_RETURN(PFNGLGETERRORPROC, GetError)
}

void GL_APIENTRY COMMON_ES_API(glGetFloatv)(GLenum pname, GLfloat * data)
{
    CALL_ES_API(PFNGLGETFLOATVPROC, GetFloatv, pname, data);
}

void GL_APIENTRY COMMON_ES_API(glGetIntegerv)(GLenum pname, GLint * data)
{
    CALL_ES_API(PFNGLGETINTEGERVPROC, GetIntegerv, pname, data);
}

void GL_APIENTRY COMMON_ES_API(glGetPointerv)(GLenum pname, void **params)
{
    CALL_ES_API(PFNGLGETPOINTERVPROC, GetPointerv, pname, params);
}

const GLubyte * GL_APIENTRY COMMON_ES_API(glGetString)(GLenum name)
{
    CALL_ES_API_RETURN(PFNGLGETSTRINGPROC, GetString, name)
}

void GL_APIENTRY COMMON_ES_API(glGetTexParameterfv)(GLenum target, GLenum pname, GLfloat * params)
{
    CALL_ES_API(PFNGLGETTEXPARAMETERFVPROC, GetTexParameterfv, target, pname, params);
}

void GL_APIENTRY COMMON_ES_API(glGetTexParameteriv)(GLenum target, GLenum pname, GLint * params)
{
    CALL_ES_API(PFNGLGETTEXPARAMETERIVPROC, GetTexParameteriv, target, pname, params);
}

void GL_APIENTRY COMMON_ES_API(glHint)(GLenum target, GLenum mode)
{
    CALL_ES_API(PFNGLHINTPROC, Hint, target, mode);
}

GLboolean GL_APIENTRY COMMON_ES_API(glIsBuffer)(GLuint buffer)
{
    CALL_ES_API_RETURN(PFNGLISBUFFERPROC, IsBuffer, buffer)
}

GLboolean GL_APIENTRY COMMON_ES_API(glIsEnabled)(GLenum cap)
{
    CALL_ES_API_RETURN(PFNGLISENABLEDPROC, IsEnabled, cap)
}

GLboolean GL_APIENTRY COMMON_ES_API(glIsTexture)(GLuint texture)
{
    CALL_ES_API_RETURN(PFNGLISTEXTUREPROC, IsTexture, texture)
}

void GL_APIENTRY COMMON_ES_API(glLineWidth)(GLfloat width)
{
    CALL_ES_API(PFNGLLINEWIDTHPROC, LineWidth, width);
}

void GL_APIENTRY COMMON_ES_API(glPixelStorei)(GLenum pname, GLint param)
{
    CALL_ES_API(PFNGLPIXELSTOREIPROC, PixelStorei, pname, param);
}

void GL_APIENTRY COMMON_ES_API(glPolygonOffset)(GLfloat factor, GLfloat units)
{
    CALL_ES_API(PFNGLPOLYGONOFFSETPROC, PolygonOffset, factor, units);
}

void GL_APIENTRY COMMON_ES_API(glReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void * pixels)
{
    CALL_ES_API(PFNGLREADPIXELSPROC, ReadPixels, x, y, width, height, format, type, pixels);
}

void GL_APIENTRY COMMON_ES_API(glSampleCoverage)(GLfloat value, GLboolean invert)
{
    CALL_ES_API(PFNGLSAMPLECOVERAGEPROC, SampleCoverage, value, invert);
}

void GL_APIENTRY COMMON_ES_API(glScissor)(GLint x, GLint y, GLsizei width, GLsizei height)
{
    CALL_ES_API(PFNGLSCISSORPROC, Scissor, x, y, width, height);
}

void GL_APIENTRY COMMON_ES_API(glStencilFunc)(GLenum func, GLint ref, GLuint mask)
{
    CALL_ES_API(PFNGLSTENCILFUNCPROC, StencilFunc, func, ref, mask);
}

void GL_APIENTRY COMMON_ES_API(glStencilMask)(GLuint mask)
{
    CALL_ES_API(PFNGLSTENCILMASKPROC, StencilMask, mask);
}

void GL_APIENTRY COMMON_ES_API(glStencilOp)(GLenum fail, GLenum zfail, GLenum zpass)
{
    CALL_ES_API(PFNGLSTENCILOPPROC, StencilOp, fail, zfail, zpass);
}

void GL_APIENTRY COMMON_ES_API(glTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void * pixels)
{
    CALL_ES_API(PFNGLTEXIMAGE2DPROC, TexImage2D, target, level, internalformat, width, height, border, format, type, pixels);
}

void GL_APIENTRY COMMON_ES_API(glTexParameterf)(GLenum target, GLenum pname, GLfloat param)
{
    CALL_ES_API(PFNGLTEXPARAMETERFPROC, TexParameterf, target, pname, param);
}

void GL_APIENTRY COMMON_ES_API(glTexParameterfv)(GLenum target, GLenum pname, const GLfloat * params)
{
    CALL_ES_API(PFNGLTEXPARAMETERFVPROC, TexParameterfv, target, pname, params);
}

void GL_APIENTRY COMMON_ES_API(glTexParameteri)(GLenum target, GLenum pname, GLint param)
{
    CALL_ES_API(PFNGLTEXPARAMETERIPROC, TexParameteri, target, pname, param);
}

void GL_APIENTRY COMMON_ES_API(glTexParameteriv)(GLenum target, GLenum pname, const GLint * params)
{
    CALL_ES_API(PFNGLTEXPARAMETERIVPROC, TexParameteriv, target, pname, params);
}

void GL_APIENTRY COMMON_ES_API(glTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void * pixels)
{
    CALL_ES_API(PFNGLTEXSUBIMAGE2DPROC, TexSubImage2D, target, level, xoffset, yoffset, width, height, format, type, pixels);
}

void GL_APIENTRY COMMON_ES_API(glViewport)(GLint x, GLint y, GLsizei width, GLsizei height)
{
    CALL_ES_API(PFNGLVIEWPORTPROC, Viewport, x, y, width, height);
}

void GL_APIENTRY COMMON_ES_API(glMapBufferOES)(GLenum target, GLenum access)
{
    CALL_ES_API(PFNGLMAPBUFFEROESPROC, MapBufferOES, target, access);
}

void GL_APIENTRY COMMON_ES_API(glUnmapBufferOES)(GLenum target, GLenum pname, void **params)
{
    CALL_ES_API(PFNGLUNMAPBUFFEROESPROC, UnmapBufferOES, target, pname, params);
}

void GL_APIENTRY COMMON_ES_API(glEGLImageTargetTexture2DOES)(GLenum target, GLeglImageOES image)
{
    CALL_ES_API(PFNGLEGLIMAGETARGETTEXTURE2DOESPROC, EGLImageTargetTexture2DOES, target, image);
}

void GL_APIENTRY COMMON_ES_API(glEGLImageTargetRenderbufferStorageOES)(GLenum target, GLeglImageOES image)
{
    CALL_ES_API(PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC, EGLImageTargetRenderbufferStorageOES, target, image);
}

void GL_APIENTRY COMMON_ES_API(glMultiDrawArraysEXT)(GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount)
{
    CALL_ES_API(PFNGLMULTIDRAWARRAYSEXTPROC, MultiDrawArraysEXT, mode, first, count, primcount);
}

void GL_APIENTRY COMMON_ES_API(glMultiDrawElementsEXT)(GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei primcount)
{
    CALL_ES_API(PFNGLMULTIDRAWELEMENTSEXTPROC, MultiDrawElementsEXT, mode, count, type, indices, primcount);
}

EGLAPI EGLBoolean EGLAPIENTRY eglPatchID(EGLenum *PatchID, EGLBoolean Set)
{
#if gcdENABLE_3D
    gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D);

    if (Set)
    {
        gcoHAL_SetGlobalPatchID(gcvNULL, (gcePATCH_ID)*PatchID);
    }
    else
    {
        gcoHAL_GetPatchID(gcvNULL, (gcePATCH_ID*)PatchID);
    }
#endif
    return EGL_TRUE;
}

#define eglApiEntry(function) \
    { #function, gcvNULL }
#define esCommonApiDispatch(function) \
    { #function, gcvNULL, gcvNULL }

veglClientApiEntry eglApiEntryTbl[] =
{
    eglApiEntry(eglGetError),
    eglApiEntry(eglGetDisplay),
    eglApiEntry(eglInitialize),
    eglApiEntry(eglTerminate),
    eglApiEntry(eglQueryString),
    eglApiEntry(eglGetConfigs),
    eglApiEntry(eglChooseConfig),
    eglApiEntry(eglGetConfigAttrib),
    eglApiEntry(eglCreateWindowSurface),
    eglApiEntry(eglCreatePbufferSurface),
    eglApiEntry(eglCreatePixmapSurface),
    eglApiEntry(eglDestroySurface),
    eglApiEntry(eglQuerySurface),
    eglApiEntry(eglBindAPI),
    eglApiEntry(eglQueryAPI),
    eglApiEntry(eglWaitClient),
    eglApiEntry(eglReleaseThread),
    eglApiEntry(eglCreatePbufferFromClientBuffer),
    eglApiEntry(eglSurfaceAttrib),
    eglApiEntry(eglBindTexImage),
    eglApiEntry(eglReleaseTexImage),
    eglApiEntry(eglSwapInterval),
    eglApiEntry(eglCreateContext),
    eglApiEntry(eglDestroyContext),
    eglApiEntry(eglMakeCurrent),
    eglApiEntry(eglGetCurrentContext),
    eglApiEntry(eglGetCurrentSurface),
    eglApiEntry(eglGetCurrentDisplay),
    eglApiEntry(eglQueryContext),
    eglApiEntry(eglWaitGL),
    eglApiEntry(eglWaitNative),
    eglApiEntry(eglSwapBuffers),
    eglApiEntry(eglCopyBuffers),
    eglApiEntry(eglGetProcAddress),
    /* EGL 1.5 */
    eglApiEntry(eglCreateSync),
    eglApiEntry(eglDestroySync),
    eglApiEntry(eglClientWaitSync),
    eglApiEntry(eglGetSyncAttrib),
    eglApiEntry(eglCreateImage),
    eglApiEntry(eglDestroyImage),
    eglApiEntry(eglGetPlatformDisplay),
    eglApiEntry(eglCreatePlatformWindowSurface),
    eglApiEntry(eglCreatePlatformPixmapSurface),
    eglApiEntry(eglWaitSync),
    /* EGL_KHR_lock_surface. */
    eglApiEntry(eglLockSurfaceKHR),
    eglApiEntry(eglUnlockSurfaceKHR),
    /* EGL_KHR_image. */
    eglApiEntry(eglCreateImageKHR),
    eglApiEntry(eglDestroyImageKHR),
    /* EGL_KHR_fence_sync. */
    eglApiEntry(eglCreateSyncKHR),
    eglApiEntry(eglDestroySyncKHR),
    eglApiEntry(eglClientWaitSyncKHR),
    eglApiEntry(eglGetSyncAttribKHR),
    /* EGL_KHR_reusable_sync. */
    eglApiEntry(eglSignalSyncKHR),
    /* EGL_KHR_wait_sync. */
    eglApiEntry(eglWaitSyncKHR),
    /* EGL_EXT_platform_base. */
    eglApiEntry(eglGetPlatformDisplayEXT),
    eglApiEntry(eglCreatePlatformWindowSurfaceEXT),
    eglApiEntry(eglCreatePlatformPixmapSurfaceEXT),
#if defined(WL_EGL_PLATFORM)
    /* EGL_WL_bind_wayland_display. */
    eglApiEntry(eglBindWaylandDisplayWL),
    eglApiEntry(eglUnbindWaylandDisplayWL),
    eglApiEntry(eglQueryWaylandBufferWL),
#endif
#if defined(__linux__)
    /* EGL_ANDROID_native_fence_sync. */
    eglApiEntry(eglDupNativeFenceFDANDROID),
#endif
    eglApiEntry(eglSetDamageRegionKHR),
    eglApiEntry(eglSwapBuffersWithDamageKHR),
    eglApiEntry(eglSwapBuffersWithDamageEXT),
    eglApiEntry(eglQueryDmaBufFormatsEXT),
    eglApiEntry(eglQueryDmaBufModifiersEXT),
    eglApiEntry(eglPatchID),
    { gcvNULL, gcvNULL }
};

veglClientApiEntry glExtApiAliasTbl[] =
{
    /* GL_KHR_blend_equation_advanced */
    eglApiEntry(glBlendBarrierKHR),
    /* GL_OES_texture_3D */
    eglApiEntry(glTexImage3DOES),
    eglApiEntry(glTexSubImage3DOES),
    eglApiEntry(glCopyTexSubImage3DOES),
    eglApiEntry(glCompressedTexImage3DOES),
    eglApiEntry(glCompressedTexSubImage3DOES),
    /* GL_OES_get_program_binary */
    eglApiEntry(glGetProgramBinaryOES),
    eglApiEntry(glProgramBinaryOES),
    /* GL_OES_vertex_array_object */
    eglApiEntry(glBindVertexArrayOES),
    eglApiEntry(glDeleteVertexArraysOES),
    eglApiEntry(glGenVertexArraysOES),
    eglApiEntry(glIsVertexArrayOES),
     /* GL_EXT_blend_minmax */
    eglApiEntry(glBlendEquationEXT),
    /* GL_OES_copy_image */
    eglApiEntry(glCopyImageSubDataOES),
    /* GL_EXT_copy_image */
    eglApiEntry(glCopyImageSubDataEXT),
    /* GL_OES_draw_buffers_indexed */
    eglApiEntry(glEnableiOES),
    eglApiEntry(glDisableiOES),
    eglApiEntry(glBlendEquationiOES),
    eglApiEntry(glBlendEquationSeparateiOES),
    eglApiEntry(glBlendFunciOES),
    eglApiEntry(glBlendFuncSeparateiOES),
    eglApiEntry(glColorMaskiOES),
    eglApiEntry(glIsEnablediOES),
    /* GL_EXT_draw_buffers_indexed */
    eglApiEntry(glEnableiEXT),
    eglApiEntry(glDisableiEXT),
    eglApiEntry(glBlendEquationiEXT),
    eglApiEntry(glBlendEquationSeparateiEXT),
    eglApiEntry(glBlendFunciEXT),
    eglApiEntry(glBlendFuncSeparateiEXT),
    eglApiEntry(glColorMaskiEXT),
    eglApiEntry(glIsEnablediEXT),
    /* GL_OES_geometry_shader */
    eglApiEntry(glFramebufferTextureOES),
    /* GL_EXT_geometry_shader */
    eglApiEntry(glFramebufferTextureEXT),
    /* GL_OES_tessellation_shader */
    eglApiEntry(glPatchParameteriOES),
    /* GL_EXT_tessellation_shader */
    eglApiEntry(glPatchParameteriEXT),
    /* GL_OES_texture_border_clamp */
    eglApiEntry(glTexParameterIivOES),
    eglApiEntry(glTexParameterIuivOES),
    eglApiEntry(glGetTexParameterIivOES),
    eglApiEntry(glGetTexParameterIuivOES),
    eglApiEntry(glSamplerParameterIivOES),
    eglApiEntry(glSamplerParameterIuivOES),
    eglApiEntry(glGetSamplerParameterIivOES),
    eglApiEntry(glGetSamplerParameterIuivOES),
    /* GL_EXT_texture_border_clamp */
    eglApiEntry(glTexParameterIivEXT),
    eglApiEntry(glTexParameterIuivEXT),
    eglApiEntry(glGetTexParameterIivEXT),
    eglApiEntry(glGetTexParameterIuivEXT),
    eglApiEntry(glSamplerParameterIivEXT),
    eglApiEntry(glSamplerParameterIuivEXT),
    eglApiEntry(glGetSamplerParameterIivEXT),
    eglApiEntry(glGetSamplerParameterIuivEXT),
    /* GL_OES_texture_border_clamp */
    eglApiEntry(glTexBufferOES),
    eglApiEntry(glTexBufferRangeOES),
    /* GL_EXT_texture_border_clamp */
    eglApiEntry(glTexBufferEXT),
    eglApiEntry(glTexBufferRangeEXT),
    /* GL_OES_draw_elements_base_vertex */
    eglApiEntry(glDrawElementsBaseVertexOES),
    eglApiEntry(glDrawRangeElementsBaseVertexOES),
    eglApiEntry(glDrawElementsInstancedBaseVertexOES),
    eglApiEntry(glMultiDrawElementsBaseVertexOES),
    /* GL_EXT_draw_elements_base_vertex */
    eglApiEntry(glDrawElementsBaseVertexEXT),
    eglApiEntry(glDrawRangeElementsBaseVertexEXT),
    eglApiEntry(glDrawElementsInstancedBaseVertexEXT),
    /* GL_OES_primitive_bounding_box */
    eglApiEntry(glPrimitiveBoundingBoxOES),
    /* GL_EXT_primitive_bounding_box */
    eglApiEntry(glPrimitiveBoundingBoxEXT),
    /* GL_OES_sample_shading */
    eglApiEntry(glMinSampleShadingOES),
    /* GL_OES_texture_storage_multisample_2d_array */
    eglApiEntry(glTexStorage3DMultisampleOES),
    /* GL_KHR_debug */
    eglApiEntry(glDebugMessageControlKHR),
    eglApiEntry(glDebugMessageInsertKHR),
    eglApiEntry(glDebugMessageCallbackKHR),
    eglApiEntry(glGetDebugMessageLogKHR),
    eglApiEntry(glGetPointervKHR),
    eglApiEntry(glPushDebugGroupKHR),
    eglApiEntry(glPopDebugGroupKHR),
    eglApiEntry(glObjectLabelKHR),
    eglApiEntry(glGetObjectLabelKHR),
    eglApiEntry(glObjectPtrLabelKHR),
    eglApiEntry(glGetObjectPtrLabelKHR),
    /* GL_KHR_robustness */
    eglApiEntry(glGetGraphicsResetStatusKHR),
    eglApiEntry(glReadnPixelsKHR),
    eglApiEntry(glGetnUniformfvKHR),
    eglApiEntry(glGetnUniformivKHR),
    eglApiEntry(glGetnUniformuivKHR),
    /* GL_EXT_robustness */
    eglApiEntry(glGetGraphicsResetStatusEXT),
    eglApiEntry(glReadnPixelsEXT),
    eglApiEntry(glGetnUniformfvEXT),
    eglApiEntry(glGetnUniformivEXT),
    eglApiEntry(glGetnUniformuivEXT),
    /* GL_EXT_framebuffer_object */
    eglApiEntry(glIsRenderbufferEXT),
    eglApiEntry(glBindRenderbufferEXT),
    eglApiEntry(glDeleteRenderbuffersEXT),
    eglApiEntry(glGenRenderbuffersEXT),
    eglApiEntry(glRenderbufferStorageEXT),
    eglApiEntry(glGetRenderbufferParameterivEXT),
    eglApiEntry(glIsFramebufferEXT),
    eglApiEntry(glBindFramebufferEXT),
    eglApiEntry(glDeleteFramebuffersEXT),
    eglApiEntry(glGenFramebuffersEXT),
    eglApiEntry(glCheckFramebufferStatusEXT),
    eglApiEntry(glFramebufferTexture1DEXT),
    eglApiEntry(glFramebufferTexture2DEXT),
    eglApiEntry(glFramebufferTexture3DEXT),
    eglApiEntry(glFramebufferRenderbufferEXT),
    eglApiEntry(glGetFramebufferAttachmentParameterivEXT),
    eglApiEntry(glGenerateMipmapEXT),
    /* GL_EXT_framebuffer_blit */
    eglApiEntry(glBlitFramebufferEXT),
    /* GL_ARB_vertex_buffer_object */
    eglApiEntry(glGenBuffersARB),
    eglApiEntry(glBindBufferARB),
    eglApiEntry(glBufferDataARB),
    eglApiEntry(glBufferSubDataARB),
    eglApiEntry(glDeleteBuffersARB),
    eglApiEntry(glIsBufferARB),
    /* GL_ARB_vertex_program */
    eglApiEntry(glEnableVertexAttribArrayARB),
    eglApiEntry(glVertexAttribPointerARB),
    /* GL_ARB_shader_objects */
    eglApiEntry(glCreateProgramObjectARB),
    eglApiEntry(glDeleteObjectARB),
    eglApiEntry(glUseProgramObjectARB),
    eglApiEntry(glCreateShaderObjectARB),
    eglApiEntry(glShaderSourceARB),
    eglApiEntry(glCompileShaderARB),
    eglApiEntry(glGetObjectParameterivARB),
    eglApiEntry(glAttachObjectARB),
    eglApiEntry(glGetInfoLogARB),
    eglApiEntry(glLinkProgramARB),
    eglApiEntry(glGetUniformLocationARB),
    eglApiEntry(glUniform4fARB),
    eglApiEntry(glUniform1iARB),
    /* GL_ARB_color_buffer_float */
    eglApiEntry(glClampColorARB),
    /* GL_ARB_window_pos */
    eglApiEntry(glWindowPos2dARB),
    eglApiEntry(glWindowPos2dvARB),
    eglApiEntry(glWindowPos2fARB),
    eglApiEntry(glWindowPos2fvARB),
    eglApiEntry(glWindowPos2iARB),
    eglApiEntry(glWindowPos2ivARB),
    eglApiEntry(glWindowPos2sARB),
    eglApiEntry(glWindowPos2svARB),
    eglApiEntry(glWindowPos3dARB),
    eglApiEntry(glWindowPos3dvARB),
    eglApiEntry(glWindowPos3fARB),
    eglApiEntry(glWindowPos3fvARB),
    eglApiEntry(glWindowPos3iARB),
    eglApiEntry(glWindowPos3ivARB),
    eglApiEntry(glWindowPos3sARB),
    eglApiEntry(glWindowPos3svARB),
    /* GL_ARB_texture_compression */
    eglApiEntry(glGetCompressedTexImageARB),
    /* GL_ARB_ARB_multitexture */
    eglApiEntry(glActiveTextureARB),
    eglApiEntry(glClientActiveTextureARB),
    eglApiEntry(glMultiTexCoord1dARB),
    eglApiEntry(glMultiTexCoord1dvARB),
    eglApiEntry(glMultiTexCoord1fARB),
    eglApiEntry(glMultiTexCoord1fvARB),
    eglApiEntry(glMultiTexCoord1iARB),
    eglApiEntry(glMultiTexCoord1ivARB),
    eglApiEntry(glMultiTexCoord1sARB),
    eglApiEntry(glMultiTexCoord1svARB),
    eglApiEntry(glMultiTexCoord2dARB),
    eglApiEntry(glMultiTexCoord2dvARB),
    eglApiEntry(glMultiTexCoord2fARB),
    eglApiEntry(glMultiTexCoord2fvARB),
    eglApiEntry(glMultiTexCoord2iARB),
    eglApiEntry(glMultiTexCoord2ivARB),
    eglApiEntry(glMultiTexCoord2sARB),
    eglApiEntry(glMultiTexCoord2svARB),
    eglApiEntry(glMultiTexCoord3dARB),
    eglApiEntry(glMultiTexCoord3dvARB),
    eglApiEntry(glMultiTexCoord3fARB),
    eglApiEntry(glMultiTexCoord3fvARB),
    eglApiEntry(glMultiTexCoord3iARB),
    eglApiEntry(glMultiTexCoord3ivARB),
    eglApiEntry(glMultiTexCoord3sARB),
    eglApiEntry(glMultiTexCoord3svARB),
    eglApiEntry(glMultiTexCoord4dARB),
    eglApiEntry(glMultiTexCoord4dvARB),
    eglApiEntry(glMultiTexCoord4fARB),
    eglApiEntry(glMultiTexCoord4fvARB),
    eglApiEntry(glMultiTexCoord4iARB),
    eglApiEntry(glMultiTexCoord4ivARB),
    eglApiEntry(glMultiTexCoord4sARB),
    eglApiEntry(glMultiTexCoord4svARB),
    { gcvNULL, gcvNULL }
};

veglClientApiEntry glesCommonApiEntryTbl[] =
{
    __GLES_COMMON_API(eglApiEntry,)
    { gcvNULL, gcvNULL }
};

veglCommonEsApiDispatch glesCommonApiDispatchTbl[] =
{
    __GLES_COMMON_API(esCommonApiDispatch,)
    { gcvNULL, gcvNULL, gcvNULL }
};

veglClientApiEntry gles11ApiEntryTbl[] =
{
    __GLES11_API_ENTRIES(eglApiEntry)
    { gcvNULL, gcvNULL }
};

veglClientApiEntry gles32ApiEntryTbl[] =
{
    __GLES_API_ENTRIES(eglApiEntry)
    { gcvNULL, gcvNULL }
};

veglClientApiEntry gl4xApiEntryTbl[] =
{
    __GL_API_ENTRIES(eglApiEntry)
    { gcvNULL, gcvNULL }
};

veglClientApiEntry vgApiEntryTbl[] =
{
    __VG_API_ENTRIES(eglApiEntry)
    { gcvNULL, gcvNULL }
};

#undef eglApiEntry

void veglInitClientApiProcTbl(gctHANDLE library, veglClientApiEntry *lookupTbl, const char *prefix, const char *info)
{
    char apiName[80];

    if (library == gcvNULL) return;

    while (lookupTbl->name != gcvNULL)
    {
        apiName[0] = '\0';
        gcoOS_StrCatSafe(apiName, 80, prefix);
        gcoOS_StrCatSafe(apiName, 80, lookupTbl->name);

        /* Initialize the lookupTbl->function */
        if (!gcmIS_SUCCESS(gcoOS_GetProcAddress(gcvNULL, library, apiName, (gctPOINTER *)&lookupTbl->function)))
        {
            /* Something wrong */;
            gcoOS_Print("Failed %s API GetProcAddress: %s !\n", info, apiName);
        }

        /* Next lookup entry. */
        ++lookupTbl;
    }
}

void veglInitEsCommonApiDispatchTbl(gctHANDLE es11lib, gctHANDLE es2xlib, veglCommonEsApiDispatch *lookupTbl, const char *prefix)
{
    char apiName[80];

    while (lookupTbl->name != gcvNULL)
    {
        apiName[0] = '\0';
        gcoOS_StrCatSafe(apiName, 80, prefix);
        gcoOS_StrCatSafe(apiName, 80, lookupTbl->name);

        /* Initialize the lookupTbl->es11func */
        if (es11lib)
        {
            if (!gcmIS_SUCCESS(gcoOS_GetProcAddress(gcvNULL, es11lib, apiName, (gctPOINTER *)&lookupTbl->es11func)))
            {
                /* Something wrong */;
                gcoOS_Print("Failed ES Common GLES11 API GetProcAddress: %s !\n", apiName);
                return;
            }
        }

        /* Initialize the lookupTbl->es2xfunc */
        if (es2xlib)
        {
            if (!gcmIS_SUCCESS(gcoOS_GetProcAddress(gcvNULL, es2xlib, apiName, (gctPOINTER *)&lookupTbl->es2xfunc)))
            {
                /* Something wrong */;
                gcoOS_Print("Failed ES Common GLES2X API GetProcAddress: %s !\n", apiName);
                return;
            }
        }
        /* Next lookup entry. */
        ++lookupTbl;
    }
}

static EGL_PROC _LookupProc(veglClientApiEntry *lookupTbl, const char *apiName, int offset)
{
    /* Loop while there are entries in the lookup table. */
    while (lookupTbl->name != gcvNULL)
    {
        /* See if we have a match. */
        if (gcmIS_SUCCESS(gcoOS_StrCmp((apiName + offset), lookupTbl->name)))
        {
            if (lookupTbl->function)
            {
                /* Return the function pointer. */
                return lookupTbl->function;
            }
            return gcvNULL;
        }

        /* Next lookup entry. */
        ++lookupTbl;
    }

    /* No match found. */
    return gcvNULL;
}

static gctBOOL LookupGLExtAliasApiProc(veglClientApiEntry *lookupTbl, char *apiName)
{
    char *suffix;

    /* Loop while there are entries in the lookup table. */
    while (lookupTbl->name != gcvNULL)
    {
        /* See if we have a match. */
        if (gcmIS_SUCCESS(gcoOS_StrCmp(apiName, lookupTbl->name)))
        {
            /* Remove extension API suffix to look up the core API name */
            suffix = apiName + (gcoOS_StrLen(apiName, gcvNULL) - 3);
            suffix[0] = '\0';
            suffix[1] = '\0';
            suffix[2] = '\0';
            return gcvTRUE;
        }

        /* Next lookup entry. */
        ++lookupTbl;
    }

    /* No match found. */
    return gcvFALSE;
}

static EGL_PROC _GetCommonGlesApiProc(EGLint index)
{
    EGL_PROC func = gcvNULL;
    VEGLThreadData thread = veglGetThreadData();

    if (thread == gcvNULL)
    {
        gcmTRACE(gcvLEVEL_ERROR, "%s(%d): veglGetThreadData failed.", __FUNCTION__, __LINE__);
        return func;
    }

    if (thread->context)
    {
        if (MAJOR_API_VER(thread->context->client) == 1)
        {
            func = glesCommonApiDispatchTbl[index].es11func;
        }
        else if (MAJOR_API_VER(thread->context->client) == 2 || MAJOR_API_VER(thread->context->client) == 3)
        {
            func = glesCommonApiDispatchTbl[index].es2xfunc;
        }
    }

    return func;
}

EGLAPI __eglMustCastToProperFunctionPointerType EGLAPIENTRY
eglGetProcAddress(const char *procname)
{
    __eglMustCastToProperFunctionPointerType func = gcvNULL;
    VEGLThreadData thread;
    char apiName[80];
    char fwApiName[80];

    gcmHEADER_ARG("procname=%s", procname);
    VEGL_TRACE_API_PRE(GetProcAddress)(procname);

    do
    {
        thread = veglGetThreadData();
        if (thread == gcvNULL)
        {
            gcmTRACE(gcvLEVEL_ERROR, "%s(%d): veglGetThreadData failed.", __FUNCTION__, __LINE__);
            break;
        }

        /* Copy the procname to local string apiName[] */
        gcoOS_StrCopySafe(apiName, 80, procname);

        /* Look for EGL function from eglApiEntryTbl[] */
        if (gcmIS_SUCCESS(gcoOS_StrNCmp(apiName, "egl", 3)))
        {
            /* Look up EGL apiName in eglApiEntryTbl only */
            func = _LookupProc(eglApiEntryTbl, apiName, 0);
            break;
        }

        /* Look for GLES and GL function from OpenGL API tables */
        if (gcmIS_SUCCESS(gcoOS_StrNCmp(apiName, "gl", 2)))
        {
            if (thread->api == EGL_OPENGL_ES_API)
            {
                /* Look up common GLES API in glesCommonApiEntryTbl first */
                fwApiName[0] = '\0';
                gcoOS_StrCatSafe(fwApiName, 80, "forward_");
                gcoOS_StrCatSafe(fwApiName, 80, apiName);
                func = _LookupProc(glesCommonApiEntryTbl, fwApiName, 10);
                if (func) break;

                /* Change some GL extension API name to alias core API name */
                LookupGLExtAliasApiProc(glExtApiAliasTbl, apiName);

                /* Look up GLES32 apiName in gles32ApiEntryTbl */
                func = _LookupProc(gles32ApiEntryTbl, apiName, 2);
                if (func) break;

                /* Look up GLES11 apiName in gles11ApiEntryTbl */
                func = _LookupProc(gles11ApiEntryTbl, apiName, 2);
                break;
            }

            if (thread->api == EGL_OPENGL_API)
            {
                /* Change some GL extension API name to alias core API name */
                LookupGLExtAliasApiProc(glExtApiAliasTbl, apiName);

                /* Look up GL4x apiName in gl4xApiEntryTbl only */
                func = _LookupProc(gl4xApiEntryTbl, apiName, 2);
                break;
            }
        }

        /* Look for OpenVG function from vgApiEntryTbl[] */
        if (gcmIS_SUCCESS(gcoOS_StrNCmp(apiName, "vg", 2)))
        {
            func = _LookupProc(vgApiEntryTbl, apiName, 2);
            break;
        }
    }
    while (gcvFALSE);

    VEGL_TRACE_API_POST(GetProcAddress)(procname, func);
    gcmDUMP_API("${EGL eglGetProcAddress (0x%08X) := 0x%08X", procname, func);
    gcmDUMP_API_DATA(procname, 0);
    gcmDUMP_API("$}");
    gcmFOOTER_ARG("0x%x", func);
    return func;
}
