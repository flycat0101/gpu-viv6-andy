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


#include "gc_es_context.h"

#define _GC_OBJ_ZONE gcdZONE_GL40_CORE


extern  const __GLprocInfo __glProcInfoTable[];


/* The extension APIs' startIndex, endIndex can be found in __GLprocID enum.
*/

__GLextension __glExtension[] =
{
#ifdef OPENGL40
    {__GL_EXTID_ARB_ES2_compatibility, "GL_ARB_ES2_compatibility", GL_TRUE, GL_TRUE},
    {__GL_EXTID_ARB_ES3_compatibility, "GL_ARB_ES3_compatibility", GL_TRUE, GL_TRUE},
    {__GL_EXTID_ARB_ES3_1_compatibility, "GL_ARB_ES3_1_compatibility", GL_TRUE, GL_TRUE},
    {__GL_EXTID_ARB_ES3_2_compatibility, "GL_ARB_ES3_2_compatibility", GL_TRUE, GL_TRUE},
    {__GL_EXTID_ARB_color_buffer_float, "GL_ARB_color_buffer_float", GL_FALSE, GL_FALSE},
    {__GL_EXTID_ARB_depth_texture, "GL_ARB_depth_texture", GL_TRUE, GL_FALSE},
    {__GL_EXTID_ARB_framebuffer_object, "GL_ARB_framebuffer_object", GL_TRUE, GL_FALSE},
    {__GL_EXTID_ARB_half_float_pixel, "GL_ARB_half_float_pixel", GL_TRUE, GL_TRUE},
    {__GL_EXTID_ARB_shader_objects, "GL_ARB_shader_objects", GL_TRUE, GL_FALSE},
    {__GL_EXTID_ARB_shading_language_100, "GL_ARB_shading_language_100", GL_TRUE, GL_FALSE},
    {__GL_EXTID_ARB_texture_array, "GL_EXT_texture_array", GL_FALSE, GL_FALSE},
    {__GL_EXTID_ARB_texture_float, "GL_ARB_texture_float", GL_TRUE, GL_FALSE},
    {__GL_EXTID_ARB_vertex_buffer_object, "GL_ARB_vertex_buffer_object", GL_TRUE, GL_FALSE},
    {__GL_EXTID_ARB_vertex_program, "GL_ARB_vertex_program", GL_FALSE, GL_FALSE},
    {__GL_EXTID_ARB_window_pos, "GL_ARB_window_pos", GL_TRUE, GL_FALSE},
    {__GL_EXTID_ARB_texture_compression, "GL_ARB_texture_compression", GL_TRUE, GL_FALSE},
    {__GL_EXTID_ARB_multitexture, "GL_ARB_multitexture", GL_TRUE, GL_FALSE},

    /* Fix: KHR-GL**.gpu_shader5_gl.*(GL30/31/32/33/40), add glTexStorage2D */
    {__GL_EXTID_ARB_texture_storage, "GL_ARB_texture_storage", GL_TRUE, GL_FALSE},

    /* Fix: GTF-GL40.gtf40.GLCoverage.CoverageGL40, add glProgramParameteri support in OpenGL 4.0*/
    {__GL_EXTID_ARB_get_program_binary, "GL_ARB_get_program_binary", GL_TRUE, GL_FALSE},

    /* Fix: GTF-GL40.gtf30.GL3Tests.framebuffer_blit.framebuffer_blit_error_blitframebuffer_multisampled_framebuffers_different_sample_count, add glGetInternalformativ support in OpenGL 4.0*/
    {__GL_EXTID_ARB_internalformat_query, "GL_ARB_internalformat_query", GL_TRUE, GL_FALSE},

    /*Fix: GTF-GL30.gtf21.GL3Tests.proxy_textures.proxy_textures_valid, add glTexImage2DMultisample support in OpenGL 4.0*/
    {__GL_EXTID_ARB_texture_multisample, "GL_ARB_texture_multisample", GL_TRUE, GL_FALSE},

    /*Fix: GTF-GL40.gtf33.GL3Tests.explicit_attrib_location.explicit_attrib_location_* */
    {__GL_EXTID_ARB_explicit_attrib_location, "GL_ARB_explicit_attrib_location", GL_FALSE, GL_FALSE},

    {__GL_EXTID_EXT_framebuffer_blit, "GL_EXT_framebuffer_blit", GL_TRUE, GL_FALSE},
    {__GL_EXTID_EXT_framebuffer_object, "GL_EXT_framebuffer_object", GL_TRUE, GL_FALSE},
    {__GL_EXTID_EXT_packed_float, "GL_EXT_packed_float", GL_TRUE, GL_FALSE},
    {__GL_EXTID_EXT_texture_compression_latc, "GL_EXT_texture_compression_latc", GL_FALSE, GL_FALSE},
    {__GL_EXTID_EXT_texture_compression_rgtc, "GL_EXT_texture_compression_rgtc", GL_FALSE, GL_FALSE},
    {__GL_EXTID_EXT_texture_integer, "GL_EXT_texture_integer", GL_TRUE, GL_FALSE},
    {__GL_EXTID_EXT_texture_shared_exponent, "GL_EXT_texture_shared_exponent", GL_TRUE, GL_FALSE},
    {__GL_EXTID_EXT_texture_sRGB, "GL_EXT_texture_sRGB", GL_FALSE, GL_FALSE},
    {__GL_EXTID_EXT_timer_query, "GL_EXT_timer_query", GL_TRUE, GL_FALSE},
#endif

    {__GL_EXTID_OES_vertex_type_10_10_10_2, "GL_OES_vertex_type_10_10_10_2", GL_FALSE, GL_FALSE},
    {__GL_EXTID_OES_vertex_half_float, "GL_OES_vertex_half_float", GL_FALSE, GL_FALSE},
    {__GL_EXTID_OES_element_index_uint, "GL_OES_element_index_uint", GL_FALSE, GL_FALSE},
    {__GL_EXTID_OES_mapbuffer, "GL_OES_mapbuffer", GL_FALSE, GL_FALSE},
    {__GL_EXTID_OES_texture_3D, "GL_OES_texture_3D", GL_FALSE, GL_TRUE},
    {__GL_EXTID_OES_vertex_array_object, "GL_OES_vertex_array_object", GL_FALSE, GL_FALSE},
    {__GL_EXTID_OES_compressed_ETC1_RGB8_texture, "GL_OES_compressed_ETC1_RGB8_texture", GL_FALSE, GL_FALSE},
    {__GL_EXTID_OES_compressed_paletted_texture, "GL_OES_compressed_paletted_texture", GL_FALSE, GL_FALSE},
    {__GL_EXTID_OES_texture_npot, "GL_OES_texture_npot", GL_FALSE, GL_FALSE},
    {__GL_EXTID_OES_read_format, "GL_OES_read_format", GL_FALSE, GL_FALSE},
    {__GL_EXTID_OES_rgb8_rgba8, "GL_OES_rgb8_rgba8", GL_FALSE, GL_FALSE},
    {__GL_EXTID_OES_depth_texture, "GL_OES_depth_texture", GL_FALSE, GL_FALSE},
    {__GL_EXTID_OES_depth_texture_cube_map, "GL_OES_depth_texture_cube_map", GL_FALSE, GL_FALSE},
    {__GL_EXTID_OES_depth24, "GL_OES_depth24", GL_FALSE, GL_FALSE},
    {__GL_EXTID_OES_depth32, "GL_OES_depth32", GL_FALSE, GL_FALSE},
    {__GL_EXTID_OES_packed_depth_stencil, "GL_OES_packed_depth_stencil", GL_FALSE, GL_FALSE},
    {__GL_EXTID_OES_stencil1, "GL_OES_stencil1", GL_FALSE, GL_FALSE},
    {__GL_EXTID_OES_stencil4, "GL_OES_stencil4", GL_FALSE, GL_FALSE},
    {__GL_EXTID_OES_fbo_render_mipmap, "GL_OES_fbo_render_mipmap", GL_FALSE, GL_FALSE},
    {__GL_EXTID_OES_get_program_binary, "GL_OES_get_program_binary", GL_FALSE, GL_FALSE},
    {__GL_EXTID_OES_fragment_precision_high, "GL_OES_fragment_precision_high", GL_FALSE, GL_FALSE},
    {__GL_EXTID_OES_standard_derivatives, "GL_OES_standard_derivatives", GL_FALSE, GL_TRUE},
    {__GL_EXTID_OES_EGL_image, "GL_OES_EGL_image", GL_FALSE, GL_FALSE},
    {__GL_EXTID_OES_EGL_image_external, "GL_OES_EGL_image_external", GL_FALSE, GL_TRUE},
    {__GL_EXTID_OES_EGL_sync, "GL_OES_EGL_sync", GL_FALSE, GL_FALSE},
    {__GL_EXTID_OES_texture_stencil8, "GL_OES_texture_stencil8", GL_FALSE, GL_FALSE},
    {__GL_EXTID_OES_shader_image_atomic, "GL_OES_shader_image_atomic", GL_FALSE, GL_TRUE},
    {__GL_EXTID_OES_sample_variables, "GL_OES_sample_variables", GL_FALSE, GL_TRUE},
    {__GL_EXTID_OES_sample_shading, "GL_OES_sample_shading", GL_FALSE, GL_FALSE},
    {__GL_EXTID_OES_texture_storage_multisample_2d_array, "GL_OES_texture_storage_multisample_2d_array", GL_FALSE, GL_TRUE},
    {__GL_EXTID_OES_shader_multisample_interpolation, "GL_OES_shader_multisample_interpolation", GL_FALSE, GL_TRUE},
    {__GL_EXTID_OES_texture_compression_astc, "GL_OES_texture_compression_astc", GL_FALSE, GL_FALSE},
    {__GL_EXTID_OES_required_internalformat, "GL_OES_required_internalformat", GL_FALSE, GL_FALSE},
    {__GL_EXTID_OES_surfaceless_context, "GL_OES_surfaceless_context", GL_FALSE, GL_FALSE},
    {__GL_EXTID_OES_copy_image, "GL_OES_copy_image", GL_FALSE, GL_FALSE},
    {__GL_EXTID_OES_draw_buffers_indexed, "GL_OES_draw_buffers_indexed", GL_FALSE, GL_FALSE},
    {__GL_EXTID_OES_geometry_shader, "GL_OES_geometry_shader", GL_FALSE, GL_TRUE},
    {__GL_EXTID_OES_geometry_point_size, "GL_OES_geometry_point_size", GL_FALSE, GL_TRUE},
    {__GL_EXTID_OES_gpu_shader5, "GL_OES_gpu_shader5", GL_FALSE, GL_TRUE},
    {__GL_EXTID_OES_shader_io_blocks, "GL_OES_shader_io_blocks", GL_FALSE, GL_TRUE},
    {__GL_EXTID_OES_texture_border_clamp, "GL_OES_texture_border_clamp", GL_FALSE, GL_FALSE},
    {__GL_EXTID_OES_texture_buffer, "GL_OES_texture_buffer", GL_FALSE, GL_TRUE},
    {__GL_EXTID_OES_tessellation_shader, "GL_OES_tessellation_shader", GL_FALSE, GL_TRUE},
    {__GL_EXTID_OES_tessellation_point_size, "GL_OES_tessellation_point_size", GL_FALSE, GL_TRUE},
    {__GL_EXTID_OES_texture_cube_map_array, "GL_OES_texture_cube_map_array", GL_FALSE, GL_TRUE},
    {__GL_EXTID_OES_draw_elements_base_vertex, "GL_OES_draw_elements_base_vertex", GL_FALSE, GL_FALSE},
    {__GL_EXTID_OES_texture_half_float, "GL_OES_texture_half_float", GL_FALSE, GL_FALSE},
    {__GL_EXTID_OES_texture_float, "GL_OES_texture_float", GL_FALSE, GL_FALSE},
    {__GL_EXTID_OES_primitive_bounding_box, "GL_OES_primitive_bounding_box", GL_FALSE, GL_TRUE},

    {__GL_EXTID_KHR_texture_compression_astc_hdr, "GL_KHR_texture_compression_astc_hdr", GL_FALSE, GL_FALSE},
    {__GL_EXTID_KHR_texture_compression_astc_ldr, "GL_KHR_texture_compression_astc_ldr", GL_FALSE, GL_FALSE},
    {__GL_EXTID_KHR_blend_equation_advanced, "GL_KHR_blend_equation_advanced", GL_FALSE, GL_TRUE},
    {__GL_EXTID_KHR_debug, "GL_KHR_debug", GL_FALSE, GL_FALSE},
    {__GL_EXTID_KHR_robustness, "GL_KHR_robustness", GL_FALSE, GL_FALSE},
    {__GL_EXTID_KHR_robust_buffer_access_behavior, "GL_KHR_robust_buffer_access_behavior", GL_FALSE, GL_FALSE},

    {__GL_EXTID_EXT_texture_type_2_10_10_10_REV, "GL_EXT_texture_type_2_10_10_10_REV", GL_FALSE, GL_FALSE},
    {__GL_EXTID_EXT_texture_filter_anisotropic, "GL_EXT_texture_filter_anisotropic", GL_FALSE, GL_FALSE},
    {__GL_EXTID_EXT_texture_compression_dxt1, "GL_EXT_texture_compression_dxt1", GL_FALSE, GL_FALSE},
    {__GL_EXTID_EXT_texture_format_BGRA8888, "GL_EXT_texture_format_BGRA8888", GL_FALSE, GL_FALSE},
    {__GL_EXTID_EXT_texture_compression_s3tc, "GL_EXT_texture_compression_s3tc", GL_FALSE, GL_FALSE},
    {__GL_EXTID_EXT_read_format_bgra, "GL_EXT_read_format_bgra", GL_FALSE, GL_FALSE},
    {__GL_EXTID_EXT_multi_draw_arrays, "GL_EXT_multi_draw_arrays", GL_FALSE, GL_FALSE},
    {__GL_EXTID_EXT_frag_depth, "GL_EXT_frag_depth", GL_FALSE, GL_TRUE},
    {__GL_EXTID_EXT_discard_framebuffer, "GL_EXT_discard_framebuffer", GL_FALSE, GL_FALSE},
    {__GL_EXTID_EXT_blend_minmax, "GL_EXT_blend_minmax", GL_FALSE, GL_FALSE},
    {__GL_EXTID_EXT_multisampled_render_to_texture, "GL_EXT_multisampled_render_to_texture", GL_FALSE, GL_FALSE},
    {__GL_EXTID_EXT_color_buffer_half_float, "GL_EXT_color_buffer_half_float", GL_FALSE, GL_FALSE},
    {__GL_EXTID_EXT_color_buffer_float, "GL_EXT_color_buffer_float", GL_FALSE, GL_FALSE},
    {__GL_EXTID_EXT_robustness, "GL_EXT_robustness", GL_FALSE, GL_TRUE},
    {__GL_EXTID_EXT_texture_sRGB_decode, "GL_EXT_texture_sRGB_decode", GL_FALSE, GL_FALSE},
    {__GL_EXTID_EXT_draw_buffers_indexed, "GL_EXT_draw_buffers_indexed", GL_FALSE, GL_FALSE},
    {__GL_EXTID_EXT_texture_border_clamp, "GL_EXT_texture_border_clamp", GL_FALSE, GL_FALSE},
    {__GL_EXTID_EXT_texture_buffer, "GL_EXT_texture_buffer", GL_FALSE, GL_TRUE},
    {__GL_EXTID_EXT_tessellation_shader, "GL_EXT_tessellation_shader", GL_FALSE, GL_TRUE},
    {__GL_EXTID_EXT_tessellation_point_size, "GL_EXT_tessellation_point_size", GL_FALSE, GL_TRUE},
    {__GL_EXTID_EXT_geometry_shader, "GL_EXT_geometry_shader", GL_FALSE, GL_TRUE},
    {__GL_EXTID_EXT_geometry_point_size, "GL_EXT_geometry_point_size", GL_FALSE, GL_TRUE},
    {__GL_EXTID_EXT_copy_image, "GL_EXT_copy_image", GL_FALSE, GL_FALSE},
    {__GL_EXTID_EXT_texture_cube_map_array, "GL_EXT_texture_cube_map_array", GL_FALSE, GL_TRUE},
    {__GL_EXTID_EXT_gpu_shader5, "GL_EXT_gpu_shader5", GL_FALSE, GL_TRUE},
    {__GL_EXTID_EXT_shader_io_blocks, "GL_EXT_shader_io_blocks", GL_FALSE, GL_TRUE},
    {__GL_EXTID_EXT_shader_implicit_conversions, "GL_EXT_shader_implicit_conversions", GL_FALSE, GL_TRUE},
    {__GL_EXTID_EXT_multi_draw_indirect, "GL_EXT_multi_draw_indirect", GL_FALSE, GL_FALSE},
    {__GL_EXTID_EXT_draw_elements_base_vertex, "GL_EXT_draw_elements_base_vertex", GL_FALSE, GL_FALSE},
    {__GL_EXTID_EXT_texture_rg, "GL_EXT_texture_rg", GL_FALSE, GL_FALSE},
    {__GL_EXTID_EXT_primitive_bounding_box, "GL_EXT_primitive_bounding_box", GL_FALSE, GL_TRUE},
    {__GL_EXTID_EXT_shader_framebuffer_fetch, "GL_EXT_shader_framebuffer_fetch", GL_FALSE, GL_TRUE },
    {__GL_EXTID_EXT_protected_textures, "GL_EXT_protected_textures", GL_FALSE, GL_FALSE},
    {__GL_EXTID_EXT_sRGB, "GL_EXT_sRGB", GL_FALSE, GL_FALSE},

    {__GL_EXTID_ANDROID_extension_pack_es31a, "GL_ANDROID_extension_pack_es31a", GL_FALSE, GL_TRUE },

    {__GL_EXTID_VIV_tex_direct, "GL_VIV_direct_texture", GL_FALSE, GL_FALSE},


    {__GL_EXTID_EXT_LAST, gcvNULL, GL_FALSE, GL_FALSE}
};

static __GLextProcAlias __glExtProcAlias[] =
{
    /* Extension API alias for GL_OES_mapbuffer */
    {__GL_EXTID_OES_mapbuffer, "MapBufferOES", (__GLprocAddr)glMapBuffer},
    {__GL_EXTID_OES_mapbuffer, "UnmapBufferOES", (__GLprocAddr)glUnmapBuffer},
    {__GL_EXTID_OES_mapbuffer, "GetBufferPointervOES", (__GLprocAddr)glGetBufferPointerv},

    /* Extension API alias for GL_OES_texture_3D */
    {__GL_EXTID_OES_texture_3D, "TexImage3DOES", (__GLprocAddr)glTexImage3D},
    {__GL_EXTID_OES_texture_3D, "TexSubImage3DOES", (__GLprocAddr)glTexSubImage3D},
    {__GL_EXTID_OES_texture_3D, "CopyTexSubImage3DOES", (__GLprocAddr)glCopyTexSubImage3D},
    {__GL_EXTID_OES_texture_3D, "CompressedTexImage3DOES", (__GLprocAddr)glCompressedTexImage3D},
    {__GL_EXTID_OES_texture_3D, "CompressedTexSubImage3DOES", (__GLprocAddr)glCompressedTexSubImage3D},

    /* Extension API alias for GL_OES_get_program_binary */
    {__GL_EXTID_OES_get_program_binary, "GetProgramBinaryOES", (__GLprocAddr)glGetProgramBinary},
    {__GL_EXTID_OES_get_program_binary, "ProgramBinaryOES", (__GLprocAddr)glProgramBinary},

    /* Extension API alias for GL_OES_vertex_array_object */
    {__GL_EXTID_OES_vertex_array_object, "BindVertexArrayOES", (__GLprocAddr)glBindVertexArray},
    {__GL_EXTID_OES_vertex_array_object, "DeleteVertexArraysOES", (__GLprocAddr)glDeleteVertexArrays},
    {__GL_EXTID_OES_vertex_array_object, "GenVertexArraysOES", (__GLprocAddr)glGenVertexArrays},
    {__GL_EXTID_OES_vertex_array_object, "IsVertexArrayOES", (__GLprocAddr)glIsVertexArray},

     /* Extension API alias for GL_EXT_blend_minmax */
     {__GL_EXTID_EXT_blend_minmax, "BlendEquationEXT", (__GLprocAddr)glBlendEquation},

    /* Extension API alias for GL_OES_copy_image */
    {__GL_EXTID_OES_copy_image, "CopyImageSubDataOES", (__GLprocAddr)glCopyImageSubData},

    /* Extension API alias for GL_EXT_copy_image */
    {__GL_EXTID_EXT_copy_image, "CopyImageSubDataEXT", (__GLprocAddr)glCopyImageSubData},

    /* Extension API alias for GL_OES_draw_buffers_indexed */
    {__GL_EXTID_OES_draw_buffers_indexed, "EnableiOES", (__GLprocAddr)glEnablei},
    {__GL_EXTID_OES_draw_buffers_indexed, "DisableiOES", (__GLprocAddr)glDisablei},
    {__GL_EXTID_OES_draw_buffers_indexed, "BlendEquationiOES", (__GLprocAddr)glBlendEquationi},
    {__GL_EXTID_OES_draw_buffers_indexed, "BlendEquationSeparateiOES", (__GLprocAddr)glBlendEquationSeparatei},
    {__GL_EXTID_OES_draw_buffers_indexed, "BlendFunciOES", (__GLprocAddr)glBlendFunci},
    {__GL_EXTID_OES_draw_buffers_indexed, "BlendFuncSeparateiOES", (__GLprocAddr)glBlendFuncSeparatei},
    {__GL_EXTID_OES_draw_buffers_indexed, "ColorMaskiOES", (__GLprocAddr)glColorMaski},
    {__GL_EXTID_OES_draw_buffers_indexed, "IsEnablediOES", (__GLprocAddr)glIsEnabledi},

    /* Extension API alias for GL_EXT_draw_buffers_indexed */
    {__GL_EXTID_EXT_draw_buffers_indexed, "EnableiEXT", (__GLprocAddr)glEnablei},
    {__GL_EXTID_EXT_draw_buffers_indexed, "DisableiEXT", (__GLprocAddr)glDisablei},
    {__GL_EXTID_EXT_draw_buffers_indexed, "BlendEquationiEXT", (__GLprocAddr)glBlendEquationi},
    {__GL_EXTID_EXT_draw_buffers_indexed, "BlendEquationSeparateiEXT", (__GLprocAddr)glBlendEquationSeparatei},
    {__GL_EXTID_EXT_draw_buffers_indexed, "BlendFunciEXT", (__GLprocAddr)glBlendFunci},
    {__GL_EXTID_EXT_draw_buffers_indexed, "BlendFuncSeparateiEXT", (__GLprocAddr)glBlendFuncSeparatei},
    {__GL_EXTID_EXT_draw_buffers_indexed, "ColorMaskiEXT", (__GLprocAddr)glColorMaski},
    {__GL_EXTID_EXT_draw_buffers_indexed, "IsEnablediEXT", (__GLprocAddr)glIsEnabledi},


    /* Extension API alias for GL_OES_geometry_shader */
    {__GL_EXTID_OES_geometry_shader, "FramebufferTextureOES", (__GLprocAddr)glFramebufferTexture},

    /* Extension API alias for GL_EXT_geometry_shader */
    {__GL_EXTID_EXT_geometry_shader, "FramebufferTextureEXT", (__GLprocAddr)glFramebufferTexture},

    /* Extension API alias for GL_OES_tessellation_shader */
    {__GL_EXTID_OES_tessellation_shader, "PatchParameteriOES", (__GLprocAddr)glPatchParameteri},

    /* Extension API alias for GL_EXT_tessellation_shader */
    {__GL_EXTID_EXT_tessellation_shader, "PatchParameteriEXT", (__GLprocAddr)glPatchParameteri},

    /* Extension API alias for GL_OES_texture_border_clamp */
    {__GL_EXTID_OES_texture_border_clamp, "TexParameterIivOES", (__GLprocAddr)glTexParameterIiv},
    {__GL_EXTID_OES_texture_border_clamp, "TexParameterIuivOES", (__GLprocAddr)glTexParameterIuiv},
    {__GL_EXTID_OES_texture_border_clamp, "GetTexParameterIivOES", (__GLprocAddr)glGetTexParameterIiv},
    {__GL_EXTID_OES_texture_border_clamp, "GetTexParameterIuivOES", (__GLprocAddr)glGetTexParameterIuiv},
    {__GL_EXTID_OES_texture_border_clamp, "SamplerParameterIivOES", (__GLprocAddr)glSamplerParameterIiv},
    {__GL_EXTID_OES_texture_border_clamp, "SamplerParameterIuivOES", (__GLprocAddr)glSamplerParameterIuiv},
    {__GL_EXTID_OES_texture_border_clamp, "GetSamplerParameterIivOES", (__GLprocAddr)glGetSamplerParameterIiv},
    {__GL_EXTID_OES_texture_border_clamp, "GetSamplerParameterIuivOES", (__GLprocAddr)glGetSamplerParameterIuiv},

    /* Extension API alias for GL_EXT_texture_border_clamp */
    {__GL_EXTID_EXT_texture_border_clamp, "TexParameterIivEXT", (__GLprocAddr)glTexParameterIiv},
    {__GL_EXTID_EXT_texture_border_clamp, "TexParameterIuivEXT", (__GLprocAddr)glTexParameterIuiv},
    {__GL_EXTID_EXT_texture_border_clamp, "GetTexParameterIivEXT", (__GLprocAddr)glGetTexParameterIiv},
    {__GL_EXTID_EXT_texture_border_clamp, "GetTexParameterIuivEXT", (__GLprocAddr)glGetTexParameterIuiv},
    {__GL_EXTID_EXT_texture_border_clamp, "SamplerParameterIivEXT", (__GLprocAddr)glSamplerParameterIiv},
    {__GL_EXTID_EXT_texture_border_clamp, "SamplerParameterIuivEXT", (__GLprocAddr)glSamplerParameterIuiv},
    {__GL_EXTID_EXT_texture_border_clamp, "GetSamplerParameterIivEXT", (__GLprocAddr)glGetSamplerParameterIiv},
    {__GL_EXTID_EXT_texture_border_clamp, "GetSamplerParameterIuivEXT", (__GLprocAddr)glGetSamplerParameterIuiv},

    /* Extension API alias for GL_OES_texture_border_clamp */
    {__GL_EXTID_OES_texture_buffer, "TexBufferOES", (__GLprocAddr)glTexBuffer},
    {__GL_EXTID_OES_texture_buffer, "TexBufferRangeOES", (__GLprocAddr)glTexBufferRange},

    /* Extension API alias for GL_EXT_texture_border_clamp */
    {__GL_EXTID_EXT_texture_buffer, "TexBufferEXT", (__GLprocAddr)glTexBuffer},
    {__GL_EXTID_EXT_texture_buffer, "TexBufferRangeEXT", (__GLprocAddr)glTexBufferRange},

    /* Extension API alias for GL_OES_draw_elements_base_vertex */
    {__GL_EXTID_OES_draw_elements_base_vertex, "DrawElementsBaseVertexOES", (__GLprocAddr)glDrawElementsBaseVertex},
    {__GL_EXTID_OES_draw_elements_base_vertex, "DrawRangeElementsBaseVertexOES", (__GLprocAddr)glDrawRangeElementsBaseVertex},
    {__GL_EXTID_OES_draw_elements_base_vertex, "DrawElementsInstancedBaseVertexOES", (__GLprocAddr)glDrawElementsInstancedBaseVertex},
    {__GL_EXTID_OES_draw_elements_base_vertex, "MultiDrawElementsBaseVertexOES", (__GLprocAddr)glMultiDrawElementsBaseVertex},

    /* Extension API alias for GL_EXT_draw_elements_base_vertex */
    {__GL_EXTID_EXT_draw_elements_base_vertex, "DrawElementsBaseVertexEXT", (__GLprocAddr)glDrawElementsBaseVertex},
    {__GL_EXTID_EXT_draw_elements_base_vertex, "DrawRangeElementsBaseVertexEXT", (__GLprocAddr)glDrawRangeElementsBaseVertex},
    {__GL_EXTID_EXT_draw_elements_base_vertex, "DrawElementsInstancedBaseVertexEXT", (__GLprocAddr)glDrawElementsInstancedBaseVertex},

    /* Extension API alias for GL_OES_primitive_bounding_box */
    {__GL_EXTID_OES_primitive_bounding_box, "PrimitiveBoundingBoxOES", (__GLprocAddr)glPrimitiveBoundingBox},

    /* Extension API alias for GL_EXT_primitive_bounding_box */
    {__GL_EXTID_EXT_primitive_bounding_box, "PrimitiveBoundingBoxEXT", (__GLprocAddr)glPrimitiveBoundingBox},

    /* Extension API alias for GL_OES_sample_shading */
    {__GL_EXTID_OES_sample_shading, "MinSampleShadingOES", (__GLprocAddr)glMinSampleShading},

    /* Extension API alias for GL_OES_texture_storage_multisample_2d_array */
    {__GL_EXTID_OES_texture_storage_multisample_2d_array, "TexStorage3DMultisampleOES", (__GLprocAddr)glTexStorage3DMultisample},

    /* Extension API alias for GL_KHR_debug */
    {__GL_EXTID_KHR_debug, "DebugMessageControlKHR", (__GLprocAddr)glDebugMessageControl},
    {__GL_EXTID_KHR_debug, "DebugMessageInsertKHR", (__GLprocAddr)glDebugMessageInsert},
    {__GL_EXTID_KHR_debug, "DebugMessageCallbackKHR", (__GLprocAddr)glDebugMessageCallback},
    {__GL_EXTID_KHR_debug, "GetDebugMessageLogKHR", (__GLprocAddr)glGetDebugMessageLog},
    {__GL_EXTID_KHR_debug, "GetPointervKHR", (__GLprocAddr)glGetPointerv},
    {__GL_EXTID_KHR_debug, "PushDebugGroupKHR", (__GLprocAddr)glPushDebugGroup},
    {__GL_EXTID_KHR_debug, "PopDebugGroupKHR", (__GLprocAddr)glPopDebugGroup},
    {__GL_EXTID_KHR_debug, "ObjectLabelKHR", (__GLprocAddr)glObjectLabel},
    {__GL_EXTID_KHR_debug, "GetObjectLabelKHR", (__GLprocAddr)glGetObjectLabel},
    {__GL_EXTID_KHR_debug, "ObjectPtrLabelKHR", (__GLprocAddr)glObjectPtrLabel},
    {__GL_EXTID_KHR_debug, "GetObjectPtrLabelKHR", (__GLprocAddr)glGetObjectPtrLabel},

    /* Extension API alias for GL_KHR_blend_equation_advanced */
    {__GL_EXTID_KHR_blend_equation_advanced, "BlendBarrierKHR", (__GLprocAddr)glBlendBarrier},

    /* Extension API alias for GL_KHR_robustness */
    {__GL_EXTID_KHR_robustness, "GetGraphicsResetStatusKHR", (__GLprocAddr)glGetGraphicsResetStatus},
    {__GL_EXTID_KHR_robustness, "ReadnPixelsKHR", (__GLprocAddr)glReadnPixels},
    {__GL_EXTID_KHR_robustness, "GetnUniformfvKHR", (__GLprocAddr)glGetnUniformfv},
    {__GL_EXTID_KHR_robustness, "GetnUniformivKHR", (__GLprocAddr)glGetnUniformiv},
    {__GL_EXTID_KHR_robustness, "GetnUniformuivKHR", (__GLprocAddr)glGetnUniformuiv},

    /* Extension API alias for GL_EXT_multi_draw_arrays */
    {__GL_EXTID_EXT_multi_draw_arrays, "MultiDrawArraysEXT", (__GLprocAddr)glMultiDrawArrays},
    {__GL_EXTID_EXT_multi_draw_arrays, "MultiDrawElementsEXT", (__GLprocAddr)glMultiDrawElements},

    /* Extension API alias for GL_EXT_robustness */
    {__GL_EXTID_EXT_robustness, "GetGraphicsResetStatusEXT", (__GLprocAddr)glGetGraphicsResetStatus},
    {__GL_EXTID_EXT_robustness, "ReadnPixelsEXT", (__GLprocAddr)glReadnPixels},
    {__GL_EXTID_EXT_robustness, "GetnUniformfvEXT", (__GLprocAddr)glGetnUniformfv},
    {__GL_EXTID_EXT_robustness, "GetnUniformivEXT", (__GLprocAddr)glGetnUniformiv},

    /* Extension API alias for GL_EXT_framebuffer_object */
    {__GL_EXTID_EXT_framebuffer_object, "IsRenderbufferEXT", (__GLprocAddr)glIsRenderbuffer},
    {__GL_EXTID_EXT_framebuffer_object, "BindRenderbufferEXT", (__GLprocAddr)glBindRenderbuffer},
    {__GL_EXTID_EXT_framebuffer_object, "DeleteRenderbuffersEXT", (__GLprocAddr)glDeleteRenderbuffers},
    {__GL_EXTID_EXT_framebuffer_object, "GenRenderbuffersEXT", (__GLprocAddr)glGenRenderbuffers},
    {__GL_EXTID_EXT_framebuffer_object, "RenderbufferStorageEXT", (__GLprocAddr)glRenderbufferStorage},
    {__GL_EXTID_EXT_framebuffer_object, "GetRenderbufferParameterivEXT", (__GLprocAddr)glGetRenderbufferParameteriv},
    {__GL_EXTID_EXT_framebuffer_object, "IsFramebufferEXT", (__GLprocAddr)glIsFramebuffer},
    {__GL_EXTID_EXT_framebuffer_object, "BindFramebufferEXT", (__GLprocAddr)glBindFramebuffer},
    {__GL_EXTID_EXT_framebuffer_object, "DeleteFramebuffersEXT", (__GLprocAddr)glDeleteFramebuffers},
    {__GL_EXTID_EXT_framebuffer_object, "GenFramebuffersEXT", (__GLprocAddr)glGenFramebuffers},
    {__GL_EXTID_EXT_framebuffer_object, "CheckFramebufferStatusEXT", (__GLprocAddr)glCheckFramebufferStatus},
    {__GL_EXTID_EXT_framebuffer_object, "FramebufferTexture1DEXT", (__GLprocAddr)glFramebufferTexture1D},
    {__GL_EXTID_EXT_framebuffer_object, "FramebufferTexture2DEXT", (__GLprocAddr)glFramebufferTexture2D},
    {__GL_EXTID_EXT_framebuffer_object, "FramebufferTexture3DEXT", (__GLprocAddr)glFramebufferTexture3D},
    {__GL_EXTID_EXT_framebuffer_object, "FramebufferRenderbufferEXT", (__GLprocAddr)glFramebufferRenderbuffer},
    {__GL_EXTID_EXT_framebuffer_object, "GetFramebufferAttachmentParameterivEXT", (__GLprocAddr)glGetFramebufferAttachmentParameteriv},
    {__GL_EXTID_EXT_framebuffer_object, "GenerateMipmapEXT", (__GLprocAddr)glGenerateMipmap},

    /* Extension API alias for GL_EXT_framebuffer_blit */
    {__GL_EXTID_EXT_framebuffer_object, "BlitFramebufferEXT", (__GLprocAddr)glBlitFramebuffer},

#ifdef OPENGL40
    /* Extension API alias for GL_ARB_vertex_buffer_object */
    {__GL_EXTID_ARB_vertex_buffer_object, "GenBuffers", (__GLprocAddr)glGenBuffers},
    {__GL_EXTID_ARB_vertex_buffer_object, "BindBuffer", (__GLprocAddr)glBindBuffer},
    {__GL_EXTID_ARB_vertex_buffer_object, "BufferData", (__GLprocAddr)glBufferData},
    {__GL_EXTID_ARB_vertex_buffer_object, "BufferSubData", (__GLprocAddr)glBufferSubData},
    {__GL_EXTID_ARB_vertex_buffer_object, "DeleteBuffers", (__GLprocAddr)glDeleteBuffers},
    {__GL_EXTID_ARB_vertex_buffer_object, "IsBuffer", (__GLprocAddr)glIsBuffer},

    /* Extension API alias for GL_ARB_vertex_program */
    {__GL_EXTID_ARB_vertex_program, "EnableVertexAttribArray", (__GLprocAddr)glEnableVertexAttribArray},
    {__GL_EXTID_ARB_vertex_program, "VertexAttribPointer", (__GLprocAddr)glVertexAttribPointer},

    /* Extension API alias for GL_ARB_shader_objects */
    {__GL_EXTID_ARB_shader_objects, "CreateProgramObjectARB", (__GLprocAddr)glCreateProgram},
    {__GL_EXTID_ARB_shader_objects, "DeleteObjectARB", (__GLprocAddr)glDeleteObjectARB},
    {__GL_EXTID_ARB_shader_objects, "UseProgramObjectARB", (__GLprocAddr)glUseProgram},
    {__GL_EXTID_ARB_shader_objects, "CreateShaderObjectARB", (__GLprocAddr)glCreateShader},
    {__GL_EXTID_ARB_shader_objects, "ShaderSourceARB", (__GLprocAddr)glShaderSource},
    {__GL_EXTID_ARB_shader_objects, "CompileShaderARB", (__GLprocAddr)glCompileShader},
    {__GL_EXTID_ARB_shader_objects, "GetObjectParameterivARB", (__GLprocAddr)glGetObjectParameterivARB},
    {__GL_EXTID_ARB_shader_objects, "AttachObjectARB", (__GLprocAddr)glAttachShader},
    {__GL_EXTID_ARB_shader_objects, "GetInfoLogARB", (__GLprocAddr)glGetInfoLogARB},
    {__GL_EXTID_ARB_shader_objects, "LinkProgramARB", (__GLprocAddr)glLinkProgram},
    {__GL_EXTID_ARB_shader_objects, "GetUniformLocationARB", (__GLprocAddr)glGetUniformLocation},
    {__GL_EXTID_ARB_shader_objects, "Uniform4fARB", (__GLprocAddr)glUniform4f},
    {__GL_EXTID_ARB_shader_objects, "Uniform1iARB", (__GLprocAddr)glUniform1i},

    /* Extension API alias for GL_ARB_color_buffer_float */
    {__GL_EXTID_ARB_color_buffer_float, "ClampColorARB", (__GLprocAddr)glClampColor},

    /* Extension API alias for GL_ARB_window_pos */
    {__GL_EXTID_ARB_window_pos, "WindowPos2dARB", (__GLprocAddr)glWindowPos2d},
    {__GL_EXTID_ARB_window_pos, "WindowPos2dvARB", (__GLprocAddr)glWindowPos2dv},
    {__GL_EXTID_ARB_window_pos, "WindowPos2fARB", (__GLprocAddr)glWindowPos2f},
    {__GL_EXTID_ARB_window_pos, "WindowPos2fvARB", (__GLprocAddr)glWindowPos2fv},
    {__GL_EXTID_ARB_window_pos, "WindowPos2iARB", (__GLprocAddr)glWindowPos2d},
    {__GL_EXTID_ARB_window_pos, "WindowPos2ivARB", (__GLprocAddr)glWindowPos2dv},
    {__GL_EXTID_ARB_window_pos, "WindowPos2sARB", (__GLprocAddr)glWindowPos2s},
    {__GL_EXTID_ARB_window_pos, "WindowPos2svARB", (__GLprocAddr)glWindowPos2sv},
    {__GL_EXTID_ARB_window_pos, "WindowPos3dARB", (__GLprocAddr)glWindowPos3d},
    {__GL_EXTID_ARB_window_pos, "WindowPos3dvARB", (__GLprocAddr)glWindowPos3dv},
    {__GL_EXTID_ARB_window_pos, "WindowPos3fARB", (__GLprocAddr)glWindowPos3f},
    {__GL_EXTID_ARB_window_pos, "WindowPos3fvARB", (__GLprocAddr)glWindowPos3fv},
    {__GL_EXTID_ARB_window_pos, "WindowPos3iARB", (__GLprocAddr)glWindowPos3d},
    {__GL_EXTID_ARB_window_pos, "WindowPos3ivARB", (__GLprocAddr)glWindowPos3dv},
    {__GL_EXTID_ARB_window_pos, "WindowPos3sARB", (__GLprocAddr)glWindowPos3s},
    {__GL_EXTID_ARB_window_pos, "WindowPos3svARB", (__GLprocAddr)glWindowPos3sv},

    {__GL_EXTID_ARB_texture_compression, "GetCompressedTexImageARB", (__GLprocAddr)glGetCompressedTexImage},

    {__GL_EXTID_ARB_multitexture, "ActiveTextureARB", (__GLprocAddr)glActiveTexture},
    {__GL_EXTID_ARB_multitexture, "ClientActiveTextureARB", (__GLprocAddr)glClientActiveTexture},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord1dARB", (__GLprocAddr)glMultiTexCoord1d},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord1dvARB", (__GLprocAddr)glMultiTexCoord1dv},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord1fARB", (__GLprocAddr)glMultiTexCoord1f},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord1fvARB", (__GLprocAddr)glMultiTexCoord1fv},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord1iARB", (__GLprocAddr)glMultiTexCoord1i},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord1ivARB", (__GLprocAddr)glMultiTexCoord1iv},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord1sARB", (__GLprocAddr)glMultiTexCoord1s},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord1svARB", (__GLprocAddr)glMultiTexCoord1sv},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord2dARB", (__GLprocAddr)glMultiTexCoord2d},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord2dvARB", (__GLprocAddr)glMultiTexCoord2dv},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord2fARB", (__GLprocAddr)glMultiTexCoord2f},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord2fvARB", (__GLprocAddr)glMultiTexCoord2fv},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord2iARB", (__GLprocAddr)glMultiTexCoord2i},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord2ivARB", (__GLprocAddr)glMultiTexCoord2iv},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord2sARB", (__GLprocAddr)glMultiTexCoord2s},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord2svARB", (__GLprocAddr)glMultiTexCoord2sv},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord3dARB", (__GLprocAddr)glMultiTexCoord3d},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord3dvARB", (__GLprocAddr)glMultiTexCoord3dv},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord3fARB", (__GLprocAddr)glMultiTexCoord3f},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord3fvARB", (__GLprocAddr)glMultiTexCoord3fv},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord3iARB", (__GLprocAddr)glMultiTexCoord3i},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord3ivARB", (__GLprocAddr)glMultiTexCoord3iv},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord3sARB", (__GLprocAddr)glMultiTexCoord3s},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord3svARB", (__GLprocAddr)glMultiTexCoord3sv},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord4dARB", (__GLprocAddr)glMultiTexCoord4d},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord4dvARB", (__GLprocAddr)glMultiTexCoord4dv},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord4fARB", (__GLprocAddr)glMultiTexCoord4f},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord4fvARB", (__GLprocAddr)glMultiTexCoord4fv},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord4iARB", (__GLprocAddr)glMultiTexCoord4i},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord4ivARB", (__GLprocAddr)glMultiTexCoord4iv},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord4sARB", (__GLprocAddr)glMultiTexCoord4s},
    {__GL_EXTID_ARB_multitexture, "MultiTexCoord4svARB", (__GLprocAddr)glMultiTexCoord4sv},
#endif

    {__GL_EXTID_EXT_LAST, gcvNULL, gcvNULL}
};

extern const GLuint __glProcTabSize;

__GLprocAddr __glGetProcAddr(const GLchar *procName)
{
    const __GLprocInfo *procInfo = gcvNULL;
    __GLextProcAlias *curAlias;
    const GLchar *apiName;
    GLuint i;

    /* Skip invalid names first */
    if (!procName || procName[0] != 'g' || procName[1] != 'l' || procName[2] == '\0')
    {
        return gcvNULL;
    }

    /* Skip the first two characters "gl" of procName */
    apiName = procName + 2;

    /* Find in __glExtFuncAlias[] table first */
    for (curAlias = __glExtProcAlias; curAlias->index < __GL_EXTID_EXT_LAST; ++curAlias)
    {
        if (strcmp(curAlias->procName, apiName) == 0)
        {
            return curAlias->func;
        }
    }

    /* Find API function's offset in __glProcInfoTable[] table */
    for (i = 0; i < __glProcTabSize; ++i)
    {
        procInfo = &__glProcInfoTable[i];
        if (strcmp(procInfo->name, apiName) == 0)
        {
            return procInfo->func;
        }
    }

    return gcvNULL;
}
