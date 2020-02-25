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
#ifdef OPENGL40
    __GL_EXTID_ARB_ES2_compatibility,
    __GL_EXTID_ARB_ES3_compatibility,
    __GL_EXTID_ARB_ES3_1_compatibility,
    __GL_EXTID_ARB_ES3_2_compatibility,
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
    __GL_EXTID_ARB_texture_storage,
    __GL_EXTID_ARB_get_program_binary,
    __GL_EXTID_ARB_internalformat_query,
    __GL_EXTID_ARB_texture_multisample,
    __GL_EXTID_ARB_explicit_attrib_location,
    __GL_EXTID_ARB_uniform_buffer_object,
    __GL_EXTID_ARB_gpu_shader5,

    __GL_EXTID_EXT_framebuffer_blit,
    __GL_EXTID_EXT_framebuffer_object,
    __GL_EXTID_EXT_packed_float,
    __GL_EXTID_EXT_texture_compression_latc,
    __GL_EXTID_EXT_texture_compression_rgtc,
    __GL_EXTID_EXT_texture_integer,
    __GL_EXTID_EXT_texture_shared_exponent,
    __GL_EXTID_EXT_texture_sRGB,
    __GL_EXTID_EXT_timer_query,
#endif

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
    __GL_EXTID_OES_EGL_image_external_essl3,
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
    __GL_EXTID_OES_texture_half_float_linear,
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
    __GL_EXTID_EXT_sRGB,
    __GL_EXTID_ANDROID_extension_pack_es31a,

    /* Vendor extensions */
    __GL_EXTID_VIV_tex_direct,


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


#ifdef __cplusplus
extern "C" {
#endif

extern __GLextension __glExtension[];

#ifdef __cplusplus
}
#endif

#endif /* __gc_es_extensions_h__ */
