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


#ifndef __gc_es_extensions_h__
#define __gc_es_extensions_h__
#ifdef OPENGL40
#include "gc_hal_types.h"
#endif

typedef enum
{
    /* OES extensions */
    __GL_EXTID_OES_vertex_type_10_10_10_2,
    __GL_EXTID_OES_vertex_half_float,
    __GL_EXTID_OES_element_index_uint,
    __GL_EXTID_OES_mapbuffer,
    __GL_EXTID_OES_texture_3D,
    __GL_EXTID_OES_vertex_array_object,
    __GL_EXTID_OES_compressed_ETC1_RGB8_texture,
    __GL_EXTID_OES_compressed_paletted_texture,
    __GL_EXTID_OES_texture_npot,
    __GL_EXTID_OES_read_format,
    __GL_EXTID_OES_rgb8_rgba8,
    __GL_EXTID_OES_depth_texture,
    __GL_EXTID_OES_depth_texture_cube_map,
    __GL_EXTID_OES_depth24,
    __GL_EXTID_OES_depth32,
    __GL_EXTID_OES_packed_depth_stencil,
    __GL_EXTID_OES_stencil1,
    __GL_EXTID_OES_stencil4,
    __GL_EXTID_OES_fbo_render_mipmap,
    __GL_EXTID_OES_get_program_binary,
    __GL_EXTID_OES_fragment_precision_high,
    __GL_EXTID_OES_standard_derivatives,
    __GL_EXTID_OES_EGL_image,
    __GL_EXTID_OES_EGL_image_external,
    __GL_EXTID_OES_EGL_sync,
    __GL_EXTID_OES_texture_stencil8,
    __GL_EXTID_OES_shader_image_atomic,
    __GL_EXTID_OES_sample_variables,
    __GL_EXTID_OES_sample_shading,
    __GL_EXTID_OES_texture_storage_multisample_2d_array,
    __GL_EXTID_OES_shader_multisample_interpolation,
    __GL_EXTID_OES_texture_compression_astc,
    __GL_EXTID_OES_required_internalformat,
    __GL_EXTID_OES_surfaceless_context,
    __GL_EXTID_OES_copy_image,
    __GL_EXTID_OES_draw_buffers_indexed,
    __GL_EXTID_OES_geometry_shader,
    __GL_EXTID_OES_geometry_point_size,
    __GL_EXTID_OES_gpu_shader5,
    __GL_EXTID_OES_shader_io_blocks,
    __GL_EXTID_OES_texture_border_clamp,
    __GL_EXTID_OES_texture_buffer,
    __GL_EXTID_OES_tessellation_shader,
    __GL_EXTID_OES_tessellation_point_size,
    __GL_EXTID_OES_texture_cube_map_array,
    __GL_EXTID_OES_draw_elements_base_vertex,
    __GL_EXTID_OES_texture_half_float,
    __GL_EXTID_OES_texture_float,
    __GL_EXTID_OES_primitive_bounding_box,

    /* KHR extensions */
    __GL_EXTID_KHR_texture_compression_astc_hdr,
    __GL_EXTID_KHR_texture_compression_astc_ldr,
    __GL_EXTID_KHR_blend_equation_advanced,
    __GL_EXTID_KHR_debug,
    __GL_EXTID_KHR_robustness,
    __GL_EXTID_KHR_robust_buffer_access_behavior,

    /* EXT extensions */
    __GL_EXTID_EXT_texture_type_2_10_10_10_REV,
    __GL_EXTID_EXT_texture_filter_anisotropic,
    __GL_EXTID_EXT_texture_compression_dxt1,
    __GL_EXTID_EXT_texture_format_BGRA8888,
    __GL_EXTID_EXT_texture_compression_s3tc,
    __GL_EXTID_EXT_read_format_bgra,
    __GL_EXTID_EXT_multi_draw_arrays,
    __GL_EXTID_EXT_frag_depth,
    __GL_EXTID_EXT_discard_framebuffer,
    __GL_EXTID_EXT_blend_minmax,
    __GL_EXTID_EXT_multisampled_render_to_texture,
    __GL_EXTID_EXT_color_buffer_half_float,
    __GL_EXTID_EXT_color_buffer_float,
    __GL_EXTID_EXT_robustness,
    __GL_EXTID_EXT_texture_sRGB_decode,
    __GL_EXTID_EXT_draw_buffers_indexed,
    __GL_EXTID_EXT_texture_border_clamp,
    __GL_EXTID_EXT_texture_buffer,
    __GL_EXTID_EXT_tessellation_shader,
    __GL_EXTID_EXT_tessellation_point_size,
    __GL_EXTID_EXT_geometry_shader,
    __GL_EXTID_EXT_geometry_point_size,
    __GL_EXTID_EXT_copy_image,
    __GL_EXTID_EXT_texture_cube_map_array,
    __GL_EXTID_EXT_gpu_shader5,
    __GL_EXTID_EXT_shader_io_blocks,
    __GL_EXTID_EXT_shader_implicit_conversions,
    __GL_EXTID_EXT_multi_draw_indirect,
    __GL_EXTID_EXT_draw_elements_base_vertex,
    __GL_EXTID_EXT_texture_rg,
    __GL_EXTID_EXT_primitive_bounding_box,
    __GL_EXTID_EXT_shader_framebuffer_fetch,
    __GL_EXTID_EXT_protected_textures,
    __GL_EXTID_ANDROID_extension_pack_es31a,

    /* Vendor extensions */
    __GL_EXTID_VIV_tex_direct,


#ifdef OPENGL40
    __GL_EXTID_EXT_framebuffer_blit,
    __GL_EXTID_EXT_framebuffer_object,
    __GL_EXTID_EXT_packed_float,
    __GL_EXTID_EXT_texture_compression_latc,
    __GL_EXTID_EXT_texture_compression_rgtc,
    __GL_EXTID_EXT_texture_integer,
    __GL_EXTID_EXT_texture_shared_exponent,
    __GL_EXTID_EXT_texture_sRGB,
    __GL_EXTID_EXT_timer_query,

    __GL_EXTID_ARB_color_buffer_float,
    __GL_EXTID_ARB_depth_texture,
    __GL_EXTID_ARB_framebuffer_object,
    __GL_EXTID_ARB_half_float_pixel,
    __GL_EXTID_ARB_shader_objects,
    __GL_EXTID_ARB_shading_language_100,
    __GL_EXTID_ARB_texture_array,
    __GL_EXTID_ARB_texture_float,
    __GL_EXTID_ARB_vertex_buffer_object,
    __GL_EXTID_ARB_vertex_program,
    __GL_EXTID_ARB_window_pos,
    __GL_EXTID_ARB_texture_compression,
    __GL_EXTID_ARB_multitexture,
#endif

    __GL_EXTID_EXT_LAST,

} __GLextID;

typedef struct __GLextensionRec
{
    __GLextID       index;          /* the extension index */
    const GLchar *  name;           /* the extension name */
    GLboolean       bEnabled;       /* is this extension enabled ? */
    GLboolean       bGLSL;          /* the extension need support from glsl? */

} __GLextension;

typedef void (*__GLprocAddr)(void);

typedef struct __GLextProcAliasRec
{
    __GLextID       index;          /* the extension index to which the function belongs to */
    const GLchar *  procName;       /* extension function name */
    __GLprocAddr    func;

} __GLextProcAlias;

typedef struct
{
    const GLchar* name;
    __GLprocAddr  func;
} __GLprocInfo;

#if defined(OPENGL40)&&defined(DRI_PIXMAPRENDER_GL)
typedef struct __GLextFuncAliasRec {
    __GLextID  index;         /* the extension index to which the function belongs to */
    const char *procName;       /* extension function name */
    const char *aliasName;      /* extension alias function name in dispatch table */
} __GLextFuncAlias;
static __GLextFuncAlias __glExtFuncAlias[] =
{
    /* Extension API alias for GL_OES_texture_3D */
    {__GL_EXTID_OES_texture_3D, "TexImage3DOES", "TexImage3D"},
    {__GL_EXTID_OES_texture_3D, "TexSubImage3DOES", "TexSubImage3D"},
    {__GL_EXTID_OES_texture_3D, "CopyTexSubImage3DOES", "CopyTexSubImage3D"},
    {__GL_EXTID_OES_texture_3D, "CompressedTexImage3DOES", "CompressedTexImage3D"},
    {__GL_EXTID_OES_texture_3D, "CompressedTexSubImage3DOES", "CompressedTexSubImage3D"},

    /* Extension API alias for GL_OES_get_program_binary */
    {__GL_EXTID_OES_get_program_binary, "GetProgramBinaryOES", "GetProgramBinary"},
    {__GL_EXTID_OES_get_program_binary, "ProgramBinaryOES", "ProgramBinary"},

    /* Extension API alias for GL_OES_vertex_array_object */
    {__GL_EXTID_OES_vertex_array_object, "BindVertexArrayOES", "BindVertexArray"},
    {__GL_EXTID_OES_vertex_array_object, "DeleteVertexArraysOES", "DeleteVertexArrays"},
    {__GL_EXTID_OES_vertex_array_object, "GenVertexArraysOES", "GenVertexArrays"},
    {__GL_EXTID_OES_vertex_array_object, "IsVertexArrayOES", "IsVertexArray"},

     /* Extension API alias for GL_EXT_blend_minmax */
     {__GL_EXTID_EXT_blend_minmax, "BlendEquationEXT", "BlendEquation"},

    /* Extension API alias for GL_OES_copy_image */
    {__GL_EXTID_OES_copy_image, "CopyImageSubDataOES", "CopyImageSubData"},

    /* Extension API alias for GL_EXT_copy_image */
    {__GL_EXTID_EXT_copy_image, "CopyImageSubDataEXT", "CopyImageSubData"},

    /* Extension API alias for GL_OES_draw_buffers_indexed */
    {__GL_EXTID_OES_draw_buffers_indexed, "EnableiOES", "Enablei"},
    {__GL_EXTID_OES_draw_buffers_indexed, "DisableiOES", "Disablei"},
    {__GL_EXTID_OES_draw_buffers_indexed, "BlendEquationiOES", "BlendEquationi"},
    {__GL_EXTID_OES_draw_buffers_indexed, "BlendEquationSeparateiOES", "BlendEquationSeparatei"},
    {__GL_EXTID_OES_draw_buffers_indexed, "BlendFunciOES", "BlendFunci"},
    {__GL_EXTID_OES_draw_buffers_indexed, "BlendFuncSeparateiOES", "BlendFuncSeparatei"},
    {__GL_EXTID_OES_draw_buffers_indexed, "ColorMaskiOES", "ColorMaski"},
    {__GL_EXTID_OES_draw_buffers_indexed, "IsEnablediOES", "IsEnabledi"},

    /* Extension API alias for GL_EXT_draw_buffers_indexed */
    {__GL_EXTID_EXT_draw_buffers_indexed, "EnableiEXT", "Enablei"},
    {__GL_EXTID_EXT_draw_buffers_indexed, "DisableiEXT", "Disablei"},
    {__GL_EXTID_EXT_draw_buffers_indexed, "BlendEquationiEXT", "BlendEquationi"},
    {__GL_EXTID_EXT_draw_buffers_indexed, "BlendEquationSeparateiEXT", "BlendEquationSeparatei"},
    {__GL_EXTID_EXT_draw_buffers_indexed, "BlendFunciEXT", "BlendFunci"},
    {__GL_EXTID_EXT_draw_buffers_indexed, "BlendFuncSeparateiEXT", "BlendFuncSeparatei"},
    {__GL_EXTID_EXT_draw_buffers_indexed, "ColorMaskiEXT", "ColorMaski"},
    {__GL_EXTID_EXT_draw_buffers_indexed, "IsEnablediEXT", "IsEnabledi"},

    /* Extension API alias for GL_OES_geometry_shader */
    {__GL_EXTID_OES_geometry_shader, "FramebufferTextureOES", "FramebufferTexture"},

    /* Extension API alias for GL_EXT_geometry_shader */
    {__GL_EXTID_EXT_geometry_shader, "FramebufferTextureEXT", "FramebufferTexture"},

    /* Extension API alias for GL_OES_tessellation_shader */
    {__GL_EXTID_OES_tessellation_shader, "PatchParameteriOES", "PatchParameteri"},

    /* Extension API alias for GL_EXT_tessellation_shader */
    {__GL_EXTID_EXT_tessellation_shader, "PatchParameteriEXT", "PatchParameteri"},

    /* Extension API alias for GL_OES_texture_border_clamp */
    {__GL_EXTID_OES_texture_border_clamp, "TexParameterIivOES", "TexParameterIiv"},
    {__GL_EXTID_OES_texture_border_clamp, "TexParameterIuivOES", "TexParameterIuiv"},
    {__GL_EXTID_OES_texture_border_clamp, "GetTexParameterIivOES", "GetTexParameterIiv"},
    {__GL_EXTID_OES_texture_border_clamp, "GetTexParameterIuivOES", "GetTexParameterIuiv"},
    {__GL_EXTID_OES_texture_border_clamp, "SamplerParameterIivOES", "SamplerParameterIiv"},
    {__GL_EXTID_OES_texture_border_clamp, "SamplerParameterIuivOES", "SamplerParameterIuiv"},
    {__GL_EXTID_OES_texture_border_clamp, "GetSamplerParameterIivOES", "GetSamplerParameterIiv"},
    {__GL_EXTID_OES_texture_border_clamp, "GetSamplerParameterIuivOES", "GetSamplerParameterIuiv"},

    /* Extension API alias for GL_EXT_texture_border_clamp */
    {__GL_EXTID_EXT_texture_border_clamp, "TexParameterIivEXT", "TexParameterIiv"},
    {__GL_EXTID_EXT_texture_border_clamp, "TexParameterIuivEXT", "TexParameterIuiv"},
    {__GL_EXTID_EXT_texture_border_clamp, "GetTexParameterIivEXT", "GetTexParameterIiv"},
    {__GL_EXTID_EXT_texture_border_clamp, "GetTexParameterIuivEXT", "GetTexParameterIuiv"},
    {__GL_EXTID_EXT_texture_border_clamp, "SamplerParameterIivEXT", "SamplerParameterIiv"},
    {__GL_EXTID_EXT_texture_border_clamp, "SamplerParameterIuivEXT", "SamplerParameterIuiv"},
    {__GL_EXTID_EXT_texture_border_clamp, "GetSamplerParameterIivEXT", "GetSamplerParameterIiv"},
    {__GL_EXTID_EXT_texture_border_clamp, "GetSamplerParameterIuivEXT", "GetSamplerParameterIuiv"},

    /* Extension API alias for GL_OES_texture_border_clamp */
    {__GL_EXTID_OES_texture_buffer, "TexBufferOES", "TexBuffer"},
    {__GL_EXTID_OES_texture_buffer, "TexBufferRangeOES", "TexBufferRange"},

    /* Extension API alias for GL_EXT_texture_border_clamp */
    {__GL_EXTID_EXT_texture_buffer, "TexBufferEXT", "TexBuffer"},
    {__GL_EXTID_EXT_texture_buffer, "TexBufferRangeEXT", "TexBufferRange"},

    /* Extension API alias for GL_OES_draw_elements_base_vertex */
    {__GL_EXTID_OES_draw_elements_base_vertex, "DrawElementsBaseVertexOES", "DrawElementsBaseVertex"},
    {__GL_EXTID_OES_draw_elements_base_vertex, "DrawRangeElementsBaseVertexOES", "DrawRangeElementsBaseVertex"},
    {__GL_EXTID_OES_draw_elements_base_vertex, "DrawElementsInstancedBaseVertexOES", "DrawElementsInstancedBaseVertex"},
    {__GL_EXTID_OES_draw_elements_base_vertex, "MultiDrawElementsBaseVertexOES", "MultiDrawElementsBaseVertexEXT"},

    /* Extension API alias for GL_EXT_draw_elements_base_vertex */
    {__GL_EXTID_EXT_draw_elements_base_vertex, "DrawElementsBaseVertexEXT", "DrawElementsBaseVertex"},
    {__GL_EXTID_EXT_draw_elements_base_vertex, "DrawRangeElementsBaseVertexEXT", "DrawRangeElementsBaseVertex"},
    {__GL_EXTID_EXT_draw_elements_base_vertex, "DrawElementsInstancedBaseVertexEXT", "DrawElementsInstancedBaseVertex"},

    /* Extension API alias for GL_EXT_framebuffer_object */
    {__GL_EXTID_EXT_framebuffer_object, "IsRenderbufferEXT", "IsRenderbuffer"},
    {__GL_EXTID_EXT_framebuffer_object, "BindRenderbufferEXT", "BindRenderbuffer"},
    {__GL_EXTID_EXT_framebuffer_object, "DeleteRenderbuffersEXT", "DeleteRenderbuffers"},
    {__GL_EXTID_EXT_framebuffer_object, "GenRenderbuffersEXT", "GenRenderbuffers"},
    {__GL_EXTID_EXT_framebuffer_object, "RenderbufferStorageEXT", "RenderbufferStorage"},
    {__GL_EXTID_EXT_framebuffer_object, "GetRenderbufferParameterivEXT", "GetRenderbufferParameteriv"},
    {__GL_EXTID_EXT_framebuffer_object, "IsFramebufferEXT", "IsFramebuffer"},
    {__GL_EXTID_EXT_framebuffer_object, "BindFramebufferEXT", "BindFramebuffer"},
    {__GL_EXTID_EXT_framebuffer_object, "DeleteFramebuffersEXT", "DeleteFramebuffers"},
    {__GL_EXTID_EXT_framebuffer_object, "GenFramebuffersEXT", "GenFramebuffers"},
    {__GL_EXTID_EXT_framebuffer_object, "CheckFramebufferStatusEXT", "CheckFramebufferStatus"},
    {__GL_EXTID_EXT_framebuffer_object, "FramebufferTexture1DEXT", "FramebufferTexture1D"},
    {__GL_EXTID_EXT_framebuffer_object, "FramebufferTexture2DEXT", "FramebufferTexture2D"},
    {__GL_EXTID_EXT_framebuffer_object, "FramebufferTexture3DEXT", "FramebufferTexture3D"},
    {__GL_EXTID_EXT_framebuffer_object, "FramebufferRenderbufferEXT", "FramebufferRenderbuffer"},
    {__GL_EXTID_EXT_framebuffer_object, "GetFramebufferAttachmentParameterivEXT", "GetFramebufferAttachmentParameteriv"},
    {__GL_EXTID_EXT_framebuffer_object, "GenerateMipmapEXT", "GenerateMipmap"},

    /* Extension API alias for GL_OES_primitive_bounding_box */
    {__GL_EXTID_OES_primitive_bounding_box, "PrimitiveBoundingBoxOES", "PrimitiveBoundingBox"},

    /* Extension API alias for GL_EXT_primitive_bounding_box */
    {__GL_EXTID_EXT_primitive_bounding_box, "PrimitiveBoundingBoxEXT", "PrimitiveBoundingBox"},

    /* Extension API alias for GL_OES_sample_shading */
    {__GL_EXTID_OES_sample_shading, "MinSampleShadingOES", "MinSampleShading"},

    /* Extension API alias for GL_OES_texture_storage_multisample_2d_array */
    {__GL_EXTID_OES_texture_storage_multisample_2d_array, "TexStorage3DMultisampleOES", "TexStorage3DMultisample"},

    /* Extension API alias for GL_KHR_debug */
    {__GL_EXTID_KHR_debug, "DebugMessageControlKHR", "DebugMessageControl"},
    {__GL_EXTID_KHR_debug, "DebugMessageInsertKHR", "DebugMessageInsert"},
    {__GL_EXTID_KHR_debug, "DebugMessageCallbackKHR", "DebugMessageCallback"},
    {__GL_EXTID_KHR_debug, "GetDebugMessageLogKHR", "GetDebugMessageLog"},
    {__GL_EXTID_KHR_debug, "GetPointervKHR", "GetPointerv"},
    {__GL_EXTID_KHR_debug, "PushDebugGroupKHR", "PushDebugGroup"},
    {__GL_EXTID_KHR_debug, "PopDebugGroupKHR", "PopDebugGroup"},
    {__GL_EXTID_KHR_debug, "ObjectLabelKHR", "ObjectLabel"},
    {__GL_EXTID_KHR_debug, "GetObjectLabelKHR", "GetObjectLabel"},
    {__GL_EXTID_KHR_debug, "ObjectPtrLabelKHR", "ObjectPtrLabel"},
    {__GL_EXTID_KHR_debug, "GetObjectPtrLabelKHR", "GetObjectPtrLabel"},

    /* Extension API alias for GL_KHR_blend_equation_advanced */
    {__GL_EXTID_KHR_blend_equation_advanced, "BlendBarrierKHR", "BlendBarrier"},

    /* Extension API alias for GL_KHR_robustness */
    {__GL_EXTID_KHR_robustness, "GetGraphicsResetStatusKHR", "GetGraphicsResetStatus"},
    {__GL_EXTID_KHR_robustness, "ReadnPixelsKHR", "ReadnPixels"},
    {__GL_EXTID_KHR_robustness, "GetnUniformfvKHR", "GetnUniformfv"},
    {__GL_EXTID_KHR_robustness, "GetnUniformivKHR", "GetnUniformiv"},
    {__GL_EXTID_KHR_robustness, "GetnUniformuivKHR", "GetnUniformuiv"},

    /* Extension API alias for GL_EXT_robustness */
    {__GL_EXTID_KHR_robustness, "GetGraphicsResetStatusEXT", "GetGraphicsResetStatus"},
    {__GL_EXTID_KHR_robustness, "ReadnPixelsEXT", "ReadnPixels"},
    {__GL_EXTID_KHR_robustness, "GetnUniformfvEXT", "GetnUniformfv"},
    {__GL_EXTID_KHR_robustness, "GetnUniformivEXT", "GetnUniformiv"},

    {__GL_EXTID_ARB_vertex_buffer_object, "GenBuffersARB", "GenBuffers"},
    {__GL_EXTID_ARB_vertex_buffer_object, "BindBufferARB", "BindBuffer"},
    {__GL_EXTID_ARB_vertex_buffer_object, "BufferDataARB", "BufferData"},
    {__GL_EXTID_ARB_vertex_buffer_object, "BufferSubDataARB", "BufferSubData"},
    {__GL_EXTID_ARB_vertex_buffer_object, "DeleteBuffersARB", "DeleteBuffers"},
    {__GL_EXTID_ARB_vertex_buffer_object, "IsBufferARB", "IsBuffer"},

    {__GL_EXTID_ARB_vertex_program, "EnableVertexAttribArrayARB", "EnableVertexAttribArray"},
    {__GL_EXTID_ARB_vertex_program, "VertexAttribPointerARB", "VertexAttribPointer"},

    {__GL_EXTID_ARB_shader_objects, "CreateProgramObjectARB", "CreateProgram"},
    {__GL_EXTID_ARB_shader_objects, "DeleteObjectARB", "DeleteObjectARB"},
    {__GL_EXTID_ARB_shader_objects, "UseProgramObjectARB", "UseProgram"},
    {__GL_EXTID_ARB_shader_objects, "CreateShaderObjectARB", "CreateShader"},
    {__GL_EXTID_ARB_shader_objects, "ShaderSourceARB", "ShaderSource"},
    {__GL_EXTID_ARB_shader_objects, "CompileShaderARB", "CompileShader"},
    {__GL_EXTID_ARB_shader_objects, "GetObjectParameterivARB", "GetObjectParameterivARB"},
    {__GL_EXTID_ARB_shader_objects, "AttachObjectARB", "AttachShader"},
    {__GL_EXTID_ARB_shader_objects, "GetInfoLogARB", "GetInfoLogARB"},
    {__GL_EXTID_ARB_shader_objects, "LinkProgramARB", "LinkProgram"},
    {__GL_EXTID_ARB_shader_objects, "GetUniformLocationARB", "GetUniformLocation"},
    {__GL_EXTID_ARB_shader_objects, "Uniform4fARB", "Uniform4f"},
    {__GL_EXTID_ARB_shader_objects, "Uniform1iARB", "Uniform1i"},

    /* Extension API alias for GL_ARB_window_pos */
    {__GL_EXTID_ARB_window_pos, "WindowPos2dARB", "WindowPos2d"},
    {__GL_EXTID_ARB_window_pos, "WindowPos2dvARB", "WindowPos2dv"},
    {__GL_EXTID_ARB_window_pos, "WindowPos2fARB", "WindowPos2f"},
    {__GL_EXTID_ARB_window_pos, "WindowPos2fvARB", "WindowPos2fv"},
    {__GL_EXTID_ARB_window_pos, "WindowPos2iARB", "WindowPos2d"},
    {__GL_EXTID_ARB_window_pos, "WindowPos2ivARB", "WindowPos2dv"},
    {__GL_EXTID_ARB_window_pos, "WindowPos2sARB", "WindowPos2s"},
    {__GL_EXTID_ARB_window_pos, "WindowPos2svARB", "WindowPos2sv"},
    {__GL_EXTID_ARB_window_pos, "WindowPos3dARB", "WindowPos3d"},
    {__GL_EXTID_ARB_window_pos, "WindowPos3dvARB", "WindowPos3dv"},
    {__GL_EXTID_ARB_window_pos, "WindowPos3fARB", "WindowPos3f"},
    {__GL_EXTID_ARB_window_pos, "WindowPos3fvARB", "WindowPos3fv"},
    {__GL_EXTID_ARB_window_pos, "WindowPos3iARB", "WindowPos3d"},
    {__GL_EXTID_ARB_window_pos, "WindowPos3ivARB", "WindowPos3dv"},
    {__GL_EXTID_ARB_window_pos, "WindowPos3sARB", "WindowPos3s"},
    {__GL_EXTID_ARB_window_pos, "WindowPos3svARB", "WindowPos3sv"},

    {__GL_EXTID_ARB_texture_compression, "GetCompressedTexImageARB", "GetCompressedTexImage"},

    {__GL_EXTID_ARB_multitexture, "ActiveTextureARB", "ActiveTexture"},
    {__GL_EXTID_ARB_multitexture, "ClientActiveTextureARB", "ClientActiveTexture"},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord1dARB", "MultiTexCoord1d"},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord1dvARB", "MultiTexCoord1dv"},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord1fARB", "MultiTexCoord1f"},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord1fvARB", "MultiTexCoord1fv"},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord1iARB", "MultiTexCoord1i"},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord1ivARB", "MultiTexCoord1iv"},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord1sARB", "MultiTexCoord1s"},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord1svARB", "MultiTexCoord1sv"},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord2dARB", "MultiTexCoord2d"},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord2dvARB", "MultiTexCoord2dv"},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord2fARB", "MultiTexCoord2f"},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord2fvARB", "MultiTexCoord2fv"},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord2iARB", "MultiTexCoord2i"},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord2ivARB", "MultiTexCoord2iv"},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord2sARB", "MultiTexCoord2s"},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord2svARB", "MultiTexCoord2sv"},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord3dARB", "MultiTexCoord3d"},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord3dvARB", "MultiTexCoord3dv"},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord3fARB", "MultiTexCoord3f"},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord3fvARB", "MultiTexCoord3fv"},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord3iARB", "MultiTexCoord3i"},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord3ivARB", "MultiTexCoord3iv"},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord3sARB", "MultiTexCoord3s"},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord3svARB", "MultiTexCoord3sv"},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord4dARB", "MultiTexCoord4d"},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord4dvARB", "MultiTexCoord4dv"},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord4fARB", "MultiTexCoord4f"},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord4fvARB", "MultiTexCoord4fv"},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord4iARB", "MultiTexCoord4i"},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord4ivARB", "MultiTexCoord4iv"},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord4sARB", "MultiTexCoord4s"},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord4svARB", "MultiTexCoord4sv"},

    {__GL_EXTID_EXT_LAST, gcvNULL, gcvNULL}
};
#endif


#ifdef __cplusplus
extern "C" {
#endif

extern __GLextension __glExtension[];

#ifdef __cplusplus
}
#endif

#endif /* __gc_es_extensions_h__ */
