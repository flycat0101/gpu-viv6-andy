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


#include "gc_gl_context.h"
#include "gc_gl_extensions.h"
#include "glapioffsets.h"


/* The extension APIs' startIndex, endIndex can be found in glapioffset.h.
*/
__GLextension __glExtension[] =
{
    {INDEX_ARB_extensions_string, "WGL_ARB_extensions_string", GL_TRUE, -1, -1},
    {INDEX_EXT_extensions_string, "WGL_EXT_extensions_string", GL_TRUE, -1, -1},
    {INDEX_GL_EXT_swap_control, "WGL_EXT_swap_control", GL_TRUE, -1, -1},
    {INDEX_S3_s3tc, "GL_S3_s3tc", GL_FALSE, -1, -1},
    {INDEX_EXT_texture_env_add, "GL_EXT_texture_env_add", GL_FALSE, -1, -1},
    {INDEX_ARB_multitexture, "GL_ARB_multitexture", GL_FALSE, -1, -1},
    {INDEX_ARB_get_proc_address, "GLX_ARB_get_proc_address", GL_FALSE, -1, -1},
    {INDEX_ARB_transpose_matrix, "GL_ARB_transpose_matrix", GL_FALSE, -1, -1},
    {INDEX_ARB_multisample, "GL_ARB_multisample", GL_FALSE, -1, -1},
    {INDEX_ARB_texture_env_add, "GL_ARB_texture_env_add", GL_FALSE, -1, -1},
    {INDEX_ARB_texture_cube_map, "GL_ARB_texture_cube_map", GL_FALSE, -1, -1},
    {INDEX_ARB_texture_compression, "GL_ARB_texture_compression", GL_FALSE, -1, -1},
    {INDEX_ARB_texture_border_clamp, "GL_ARB_texture_border_clamp", GL_FALSE, -1, -1},
    {INDEX_ARB_point_parameters, "GL_ARB_point_parameters", GL_FALSE, -1, -1},
    {INDEX_ARB_vertex_blend, "ARB_vertex_blend", GL_FALSE, -1, -1},
    {INDEX_ARB_matrix_palette, "GL_ARB_matrix_palette", GL_FALSE, -1, -1},
    {INDEX_ARB_texture_env_combine, "GL_ARB_texture_env_combine", GL_FALSE, -1, -1},
    {INDEX_ARB_texture_env_crossbar, "GL_ARB_texture_env_crossbar", GL_FALSE, -1, -1},
    {INDEX_ARB_texture_env_dot3, "GL_ARB_texture_env_dot3", GL_FALSE, -1, -1},
    {INDEX_ARB_texture_mirrored_repeat, "GL_ARB_texture_mirrored_repeat", GL_FALSE, -1, -1},
    {INDEX_ARB_depth_texture, "GL_ARB_depth_texture", GL_FALSE, -1, -1},
    {INDEX_ARB_shadow, "GL_ARB_shadow", GL_FALSE, -1, -1},
    {INDEX_ARB_shadow_ambient, "GL_ARB_shadow_ambient", GL_FALSE, -1, -1},
    {INDEX_ARB_window_pos, "GL_ARB_window_pos", GL_FALSE, -1, -1},
    {INDEX_ARB_vertex_program, "GL_ARB_vertex_program", GL_FALSE, 583, 601},
    {INDEX_ARB_fragment_program, "GL_ARB_fragment_program", GL_FALSE, -1, -1},
    {INDEX_ARB_vertex_buffer_object, "GL_ARB_vertex_buffer_object", GL_FALSE, -1, -1},
    {INDEX_ARB_occlusion_query, "GL_ARB_occlusion_query", GL_FALSE, -1, -1},
    {INDEX_ARB_shader_objects, "GL_ARB_shader_objects", GL_FALSE, 602, 606},
    {INDEX_ARB_vertex_shader, "GL_ARB_vertex_shader", GL_FALSE, -1, -1},
    {INDEX_ARB_fragment_shader, "GL_ARB_fragment_shader", GL_FALSE, -1, -1},
    {INDEX_ARB_shading_language_100, "GL_ARB_shading_language_100", GL_FALSE, -1, -1},
    {INDEX_ARB_texture_non_power_of_two, "GL_ARB_texture_non_power_of_two", GL_FALSE, -1, -1},
    {INDEX_ARB_point_sprite, "GL_ARB_point_sprite", GL_FALSE, -1, -1},
    {INDEX_ARB_fragment_program_shadow, "GL_ARB_fragment_program_shadow", GL_FALSE, -1, -1},
    {INDEX_ARB_draw_buffers, "GL_ARB_draw_buffers", GL_FALSE, -1, -1},
    {INDEX_ARB_texture_rectangle, "GL_EXT_texture_rectangle GL_ARB_texture_rectangle", GL_FALSE, -1, -1},
    {INDEX_ARB_half_float_pixel, "GL_ARB_half_float_pixel", GL_FALSE, -1, -1},
    {INDEX_ARB_texture_float, "GL_ARB_texture_float", GL_FALSE, -1, -1},
    {INDEX_ARB_pixel_buffer_object, "GL_ARB_pixel_buffer_object", GL_FALSE, -1, -1},
    {INDEX_EXT_abgr, "GL_EXT_abgr", GL_FALSE, -1, -1},
    {INDEX_EXT_blend_color, "GL_EXT_blend_color", GL_FALSE, -1, -1},
    {INDEX_EXT_polygon_offset, "GL_EXT_polygon_offset", GL_FALSE, -1, -1},
    {INDEX_EXT_texture, "GL_EXT_texture", GL_FALSE, -1, -1},
    {INDEX_EXT_texture3D, "GL_EXT_texture3D", GL_FALSE, -1, -1},
    {INDEX_SGIS_texture_filter4, "GL_SGIS_texture_filter4", GL_FALSE, -1, -1},
    {INDEX_EXT_subtexture, "GL_EXT_subtexture", GL_FALSE, -1, -1},
    {INDEX_EXT_copy_texture, "GL_EXT_copy_texture", GL_FALSE, -1, -1},
    {INDEX_EXT_histogram, "GL_EXT_histogram", GL_FALSE, -1, -1},
    {INDEX_EXT_convolution, "GL_EXT_convolution", GL_FALSE, -1, -1},
    {INDEX_SGI_color_matrix, "GL_SGI_color_matrix", GL_FALSE, -1, -1},
    {INDEX_SGI_color_table, "GL_SGI_color_table", GL_FALSE, -1, -1},
    {INDEX_SGIS_pixel_texture, "GL_SGIS_pixel_texture", GL_FALSE, -1, -1},
    {INDEX_SGI_texture_color_table, "GL_SGI_texture_color_table", GL_FALSE, -1, -1},
    {INDEX_EXT_cmyka, "GL_EXT_cmyka", GL_FALSE, -1, -1},
    {INDEX_EXT_texture_object, "GL_EXT_texture_object", GL_FALSE, -1, -1},
    {INDEX_SGIS_detail_texture, "GL_SGIS_detail_texture", GL_FALSE, -1, -1},
    {INDEX_SGIS_sharpen_texture, "GL_SGIS_sharpen_texture", GL_FALSE, -1, -1},
    {INDEX_EXT_packed_pixels, "GL_EXT_packed_pixels", GL_FALSE, -1, -1},
    {INDEX_SGIS_texture_lod, "GL_SGIS_texture_lod", GL_FALSE, -1, -1},
    {INDEX_SGIS_multisample, "GL_SGIS_multisample", GL_FALSE, -1, -1},
    {INDEX_EXT_rescale_normal, "GL_EXT_rescale_normal", GL_FALSE, -1, -1},
    {INDEX_EXT_visual_info, "GLX_EXT_visual_info", GL_FALSE, -1, -1},
    {INDEX_EXT_vertex_array, "GL_EXT_vertex_array", GL_FALSE, -1, -1},
    {INDEX_EXT_misc_attribute, "GL_EXT_misc_attribute", GL_FALSE, -1, -1},
    {INDEX_SGIS_generate_mipmap, "GL_SGIS_generate_mipmap", GL_FALSE, -1, -1},
    {INDEX_SGIX_clipmap, "GL_SGIX_clipmap", GL_FALSE, -1, -1},
    {INDEX_SGIX_shadow, "GL_SGIX_shadow", GL_FALSE, -1, -1},
    {INDEX_SGIS_texture_edge_clamp, "GL_SGIS_texture_edge_clamp", GL_FALSE, -1, -1},
    {INDEX_SGIS_texture_border_clamp, "GL_SGIS_texture_border_clamp", GL_FALSE, -1, -1},
    {INDEX_EXT_blend_minmax, "GL_EXT_blend_minmax", GL_FALSE, -1, -1},
    {INDEX_EXT_blend_subtract, "GL_EXT_blend_subtract", GL_FALSE, -1, -1},
    {INDEX_EXT_blend_logic_op, "GL_EXT_blend_logic_op", GL_FALSE, -1, -1},
    {INDEX_SGI_swap_control, "GLX_SGI_swap_control", GL_FALSE, -1, -1},
    {INDEX_SGI_video_sync, "GLX_SGI_video_sync", GL_FALSE, -1, -1},
    {INDEX_SGI_make_current_read, "GLX_SGI_make_current_read", GL_FALSE, -1, -1},
    {INDEX_SGIX_video_source, "GLX_SGIX_video_source", GL_FALSE, -1, -1},
    {INDEX_EXT_visual_rating, "GLX_EXT_visual_rating", GL_FALSE, -1, -1},
    {INDEX_SGIX_interlace, "GL_SGIX_interlace", GL_FALSE, -1, -1},
    {INDEX_EXT_import_context, "GLX_EXT_import_context", GL_FALSE, -1, -1},
    {INDEX_SGIX_fbconfig, "GLX_SGIX_fbconfig", GL_FALSE, -1, -1},
    {INDEX_SGIX_pbuffer, "GLX_SGIX_pbuffer", GL_FALSE, -1, -1},
    {INDEX_SGIS_texture_select, "GL_SGIS_texture_select", GL_FALSE, -1, -1},
    {INDEX_SGIX_sprite, "GL_SGIX_sprite", GL_FALSE, -1, -1},
    {INDEX_SGIX_texture_multi_buffer, "GL_SGIX_texture_multi_buffer", GL_FALSE, -1, -1},
    {INDEX_EXT_point_parameters, "GL_EXT_point_parameters", GL_FALSE, -1, -1},
    {INDEX_SGIX_instruments, "GL_SGIX_instruments", GL_FALSE, -1, -1},
    {INDEX_SGIX_texture_scale_bias, "GL_SGIX_texture_scale_bias ", GL_FALSE, -1, -1},
    {INDEX_SGIX_framezoom, "GL_SGIX_framezoom", GL_FALSE, -1, -1},
    {INDEX_SGIX_tag_sample_buffer, "GL_SGIX_tag_sample_buffer", GL_FALSE, -1, -1},
    {INDEX_SGIX_reference_plane, "GL_SGIX_reference_plane ", GL_FALSE, -1, -1},
    {INDEX_SGIX_flush_raster, "GL_SGIX_flush_raster", GL_FALSE, -1, -1},
    {INDEX_SGI_cushion, "GLX_SGI_cushion", GL_FALSE, -1, -1},
    {INDEX_SGIX_depth_texture, "GL_SGIX_depth_texture", GL_FALSE, -1, -1},
    {INDEX_SGIS_fog_function, "GL_SGIS_fog_function", GL_FALSE, -1, -1},
    {INDEX_SGIX_fog_offset, "GL_SGIX_fog_of", GL_FALSE, -1, -1},
    {INDEX_HP_image_transform, "GL_HP_image_transform", GL_FALSE, -1, -1},
    {INDEX_HP_convolution_border_modes, "GL_HP_convolution_border_modes", GL_FALSE, -1, -1},
    {INDEX_SGIX_texture_add_env, "GL_SGIX_texture_add_env ", GL_FALSE, -1, -1},
    {INDEX_EXT_color_subtable, "GL_EXT_color_subtable ", GL_FALSE, -1, -1},
    {INDEX_EXT_object_space_tess, "GLU_EXT_object_space_tess", GL_FALSE, -1, -1},
    {INDEX_PGI_vertex_hints, "GL_PGI_vertex_hints", GL_FALSE, -1, -1},
    {INDEX_PGI_misc_hints, "GL_PGI_misc_hints", GL_FALSE, -1, -1},
    {INDEX_EXT_paletted_texture, "GL_EXT_paletted_texture", GL_FALSE, -1, -1},
    {INDEX_EXT_clip_volume_hint, "GL_EXT_clip_volume_hint", GL_FALSE, -1, -1},
    {INDEX_SGIX_list_priority, "GL_SGIX_list_priority", GL_FALSE, -1, -1},
    {INDEX_SGIX_ir_instrument1, "GL_SGIX_ir_instrument1", GL_FALSE, -1, -1},
    {INDEX_SGIX_video_resize, "GLX_SGIX_video_resize", GL_FALSE, -1, -1},
    {INDEX_SGIX_texture_lod_bias, "GL_SGIX_texture_lod_bias", GL_FALSE, -1, -1},
    {INDEX_SGI_filter4_parameters, "GLU_SGI_filter4_parameters", GL_FALSE, -1, -1},
    {INDEX_SGIX_dm_buffer, "GLX_SGIX_dm_buffer", GL_FALSE, -1, -1},
    {INDEX_SGIX_shadow_ambient, "GL_SGIX_shadow_ambient", GL_FALSE, -1, -1},
    {INDEX_SGIX_swap_group, "GLX_SGIX_swap_group", GL_FALSE, -1, -1},
    {INDEX_SGIX_swap_barrier, "GLX_SGIX_swap_barrier", GL_FALSE, -1, -1},
    {INDEX_EXT_index_texture, "GL_EXT_index_texture", GL_FALSE, -1, -1},
    {INDEX_EXT_index_material, "GL_EXT_index_material", GL_FALSE, -1, -1},
    {INDEX_EXT_index_func, "GL_EXT_index_func", GL_FALSE, -1, -1},
    {INDEX_EXT_index_array_formats, "GL_EXT_index_array_formats", GL_FALSE, -1, -1},
    {INDEX_EXT_cull_vertex, "GL_EXT_cull_vertex ", GL_FALSE, -1, -1},
    {INDEX_EXT_nurbs_tessellator, "GLU_EXT_nurbs_tessellator", GL_FALSE, -1, -1},
    {INDEX_SGIX_ycrcb, "GL_SGIX_ycrcb", GL_FALSE, -1, -1},
    {INDEX_EXT_fragment_lighting, "GL_EXT_fragment_lighting", GL_FALSE, -1, -1},
    {INDEX_IBM_rasterpos_clip, "GL_IBM_rasterpos_clip", GL_FALSE, -1, -1},
    {INDEX_HP_texture_lighting, "GL_HP_texture_lighting", GL_FALSE, -1, -1},
    {INDEX_EXT_draw_range_elements, "GL_EXT_draw_range_elements", GL_FALSE, -1, -1},
    {INDEX_WIN_phong_shading, "GL_WIN_phong_shading", GL_FALSE, -1, -1},
    {INDEX_WIN_specular_fog, "GL_WIN_specular_fog", GL_FALSE, -1, -1},
    {INDEX_SGIS_color_range, "GLX_SGIS_color_range", GL_FALSE, -1, -1},
    {INDEX_EXT_light_texture, "GL_EXT_light_texture", GL_FALSE, -1, -1},
    {INDEX_SGIX_blend_alpha_minmax, "GL_SGIX_blend_alpha_minmax", GL_FALSE, -1, -1},
    {INDEX_EXT_scene_marker, "GL_EXT_scene_marker", GL_FALSE, -1, -1},
    {INDEX_SGIX_pixel_texture_bits, "GL_SGIX_pixel_texture_bits", GL_FALSE, -1, -1},
    {INDEX_EXT_bgra, "GL_EXT_bgra", GL_FALSE, -1, -1},
    {INDEX_SGIX_async, "GL_SGIX_async", GL_FALSE, -1, -1},
    {INDEX_SGIX_async_pixel, "GL_SGIX_async_pixel", GL_FALSE, -1, -1},
    {INDEX_SGIX_async_histogram, "GL_SGIX_async_histogram", GL_FALSE, -1, -1},
    {INDEX_INTEL_texture_scissor, "GL_INTEL_texture_scissor", GL_FALSE, -1, -1},
    {INDEX_INTEL_parallel_arrays, "GL_INTEL_parallel_arrays", GL_FALSE, -1, -1},
    {INDEX_HP_occlusion_test, "GL_HP_occlusion_test", GL_FALSE, -1, -1},
    {INDEX_EXT_pixel_transform, "GL_EXT_pixel_transform", GL_FALSE, -1, -1},
    {INDEX_EXT_pixel_transform_color_table, "GL_EXT_pixel_transform_color_table", GL_FALSE, -1, -1},
    {INDEX_EXT_shared_texture_palette, "GL_EXT_shared_texture_palette", GL_FALSE, -1, -1},
    {INDEX_SGIS_blended_overlay, "GLX_SGIS_blended_overlay", GL_FALSE, -1, -1},
    {INDEX_EXT_separate_specular_color, "GL_EXT_separate_specular_color", GL_FALSE, -1, -1},
    {INDEX_EXT_secondary_color, "GL_EXT_secondary_color", GL_FALSE, -1, -1},
    {INDEX_EXT_texture_env, "GL_EXT_texture_env", GL_FALSE, -1, -1},
    {INDEX_EXT_texture_perturb_normal, "GL_EXT_texture_perturb_normal", GL_FALSE, -1, -1},
    {INDEX_EXT_multi_draw_arrays, "GL_EXT_multi_draw_arrays", GL_FALSE, -1, -1},
    {INDEX_EXT_fog_coord, "GL_EXT_fog_coord", GL_FALSE, -1, -1},
    {INDEX_REND_screen_coordinates, "GL_REND_screen_coordinates", GL_FALSE, -1, -1},
    {INDEX_EXT_coordinate_frame, "GL_EXT_coordinate_frame", GL_FALSE, -1, -1},
    {INDEX_EXT_texture_env_combine, "GL_EXT_texture_env_combine", GL_FALSE, -1, -1},
    {INDEX_APPLE_specular_vector, "GL_APPLE_specular_vector", GL_FALSE, -1, -1},
    {INDEX_SGIX_pixel_texture, "GL_SGIX_pixel_texture", GL_FALSE, -1, -1},
    {INDEX_SGIS_texture4D, "GL_SGIS_texture4D", GL_FALSE, -1, -1},
    {INDEX_APPLE_transform_hint, "GL_APPLE_transform_hint ", GL_FALSE, -1, -1},
    {INDEX_SUNX_constant_data, "GL_SUNX_constant_data", GL_FALSE, -1, -1},
    {INDEX_SUN_global_alpha, "GL_SUN_global_alpha", GL_FALSE, -1, -1},
    {INDEX_SUN_triangle_list, "GL_SUN_triangle_list", GL_FALSE, -1, -1},
    {INDEX_SUN_vertex, "GL_SUN_vertex", GL_FALSE, -1, -1},
    {INDEX_EXT_blend_func_separate, "GL_EXT_blend_func_separate", GL_FALSE, -1, -1},
    {INDEX_INGR_color_clamp, "GL_INGR_color_clamp", GL_FALSE, -1, -1},
    {INDEX_INGR_interlace_read, "GL_INGR_interlace_read", GL_FALSE, -1, -1},
    {INDEX_EXT_stencil_wrap, "GL_EXT_stencil_wrap", GL_FALSE, -1, -1},
    {INDEX_EXT_422_pixels, "GL_EXT_422_pixels", GL_FALSE, -1, -1},
    {INDEX_NV_texgen_reflection, "GL_NV_texgen_reflection", GL_FALSE, -1, -1},
    {INDEX_SGIX_texture_range, "GL_SGIX_texture_range", GL_FALSE, -1, -1},
    {INDEX_SUN_convolution_border_modes, "GL_SUN_convolution_border_modes", GL_FALSE, -1, -1},
    {INDEX_SUN_get_transparent_index, "GLX_SUN_get_transparent_index", GL_FALSE, -1, -1},
    {INDEX_EXT_texture_lod_bias, "GL_EXT_texture_lod_bias", GL_FALSE, -1, -1},
    {INDEX_EXT_texture_filter_anisotropic, "GL_EXT_texture_filter_anisotropic", GL_FALSE, -1, -1},
    {INDEX_EXT_vertex_weighting, "GL_EXT_vertex_weighting", GL_FALSE, -1, -1},
    {INDEX_NV_light_max_exponent, "GL_NV_light_max_exponent", GL_FALSE, -1, -1},
    {INDEX_NV_vertex_array_range, "GL_NV_vertex_array_range", GL_FALSE, -1, -1},
    {INDEX_NV_register_combiners, "GL_NV_register_combiners", GL_FALSE, -1, -1},
    {INDEX_NV_fog_distance, "GL_NV_fog_distance", GL_FALSE, -1, -1},
    {INDEX_NV_texgen_emboss, "GL_NV_texgen_emboss ", GL_FALSE, -1, -1},
    {INDEX_NV_blend_square, "GL_NV_blend_square", GL_FALSE, -1, -1},
    {INDEX_NV_texture_env_combine4, "GL_NV_texture_env_combine4", GL_FALSE, -1, -1},
    {INDEX_MESA_resize_buffers, "GL_MESA_resize_buffers", GL_FALSE, -1, -1},
    {INDEX_MESA_window_pos, "GL_MESA_window_pos", GL_FALSE, -1, -1},
    {INDEX_EXT_texture_compression_s3tc, "GL_EXT_texture_compression_s3tc", GL_FALSE, -1, -1},
    {INDEX_IBM_cull_vertex, "GL_IBM_cull_vertex", GL_FALSE, -1, -1},
    {INDEX_IBM_multimode_draw_arrays, "GL_IBM_multimode_draw_arrays", GL_FALSE, -1, -1},
    {INDEX_IBM_vertex_array_lists, "GL_IBM_vertex_array_lists", GL_FALSE, -1, -1},
    {INDEX_3DFX_texture_compression_FXT1, "GL_3DFX_texture_compression_FXT1", GL_FALSE, -1, -1},
    {INDEX_3DFX_multisample, "GL_3DFX_multisample", GL_FALSE, -1, -1},
    {INDEX_3DFX_tbuffer, "GL_3DFX_tbuffer", GL_FALSE, -1, -1},
    {INDEX_SGIX_vertex_preclip, "GL_SGIX_vertex_preclip", GL_FALSE, -1, -1},
    {INDEX_SGIX_resample, "GL_SGIX_resample", GL_FALSE, -1, -1},
    {INDEX_SGIS_texture_color_mask, "GL_SGIS_texture_color_mask", GL_FALSE, -1, -1},
    {INDEX_MESA_copy_sub_buffer, "GLX_MESA_copy_sub_buffer", GL_FALSE, -1, -1},
    {INDEX_MESA_pixmap_colormap, "GLX_MESA_pixmap_colormap", GL_FALSE, -1, -1},
    {INDEX_MESA_release_buffers, "GLX_MESA_release_buffers", GL_FALSE, -1, -1},
    {INDEX_MESA_set_3dfx_mode, "GLX_MESA_set_3dfx_mode", GL_FALSE, -1, -1},
    {INDEX_EXT_texture_env_dot3, "GL_EXT_texture_env_dot3", GL_FALSE, -1, -1},
    {INDEX_ATI_texture_mirror_once, "GL_ATI_texture_mirror_once", GL_FALSE, -1, -1},
    {INDEX_NV_fence, "GL_NV_fence", GL_FALSE, -1, -1},
    {INDEX_IBM_static_data, "GL_IBM_static_data", GL_FALSE, -1, -1},
    {INDEX_IBM_texture_mirrored_repeat, "GL_IBM_texture_mirrored_repeat", GL_FALSE, -1, -1},
    {INDEX_NV_evaluators, "GL_NV_evaluators", GL_FALSE, -1, -1},
    {INDEX_NV_packed_depth_stencil, "GL_NV_packed_depth_stencil", GL_FALSE, -1, -1},
    {INDEX_NV_register_combiners2, "GL_NV_register_combiners2", GL_FALSE, -1, -1},
    {INDEX_NV_texture_compression_vtc, "GL_NV_texture_compression_vtc", GL_FALSE, -1, -1},
    {INDEX_NV_texture_rectangle, "GL_NV_texture_rectangle", GL_FALSE, -1, -1},
    {INDEX_NV_texture_shader, "GL_NV_texture_shader", GL_FALSE, -1, -1},
    {INDEX_NV_texture_shader2, "GL_NV_texture_shader2", GL_FALSE, -1, -1},
    {INDEX_NV_vertex_array_range2, "GL_NV_vertex_array_range2", GL_FALSE, -1, -1},
    {INDEX_NV_vertex_program, "GL_NV_vertex_program", GL_FALSE, -1, -1},
    {INDEX_SGIX_visual_select_group, "GLX_SGIX_visual_select_group", GL_FALSE, -1, -1},
    {INDEX_SGIX_texture_coordinate_clamp, "GL_SGIX_texture_coordinate_clamp", GL_FALSE, -1, -1},
    {INDEX_OML_swap_method, "GLX_OML_swap_method", GL_FALSE, -1, -1},
    {INDEX_OML_sync_control, "GLX_OML_sync_control", GL_FALSE, -1, -1},
    {INDEX_OML_interlace, "GLX_OML_interlace", GL_FALSE, -1, -1},
    {INDEX_OML_subsample, "GL_OML_subsample", GL_FALSE, -1, -1},
    {INDEX_OML_resample, "GL_OML_resample", GL_FALSE, -1, -1},
    {INDEX_NV_copy_depth_to_color, "GL_NV_copy_depth_to_color", GL_FALSE, -1, -1},
    {INDEX_ATI_envmap_bumpmap, "GL_ATI_envmap_bumpmap", GL_FALSE, -1, -1},
    {INDEX_ATI_pn_triangles, "GL_ATI_pn_triangles", GL_FALSE, -1, -1},
    {INDEX_ATI_vertex_array_object, "GL_ATI_vertex_array_object", GL_FALSE, 607, 618},
    {INDEX_ATI_vertex_streams, "GL_ATI_vertex_streams", GL_FALSE, -1, -1},
    {INDEX_ATI_element_array, "GL_ATI_element_array", GL_FALSE, 622, 624},
    {INDEX_SUN_mesh_array, "GL_SUN_mesh_array", GL_FALSE, -1, -1},
    {INDEX_SUN_slice_accum, "GL_SUN_slice_accum", GL_FALSE, -1, -1},
    {INDEX_NV_multisample_filter_hint, "GL_NV_multisample_filter_hint", GL_FALSE, -1, -1},
    {INDEX_NV_depth_clamp, "GL_NV_depth_clamp", GL_FALSE, -1, -1},
    {INDEX_NV_occlusion_query, "GL_NV_occlusion_query", GL_FALSE, 710, 711},
    {INDEX_NV_point_sprite, "GL_NV_point_sprite", GL_FALSE, -1, -1},
    {INDEX_NV_texture_shader3, "GL_NV_texture_shader3", GL_FALSE, -1, -1},
    {INDEX_NV_vertex_program1_1, "GL_NV_vertex_program1_1", GL_FALSE, -1, -1},
    {INDEX_EXT_shadow_funcs, "GL_EXT_shadow_funcs", GL_FALSE, -1, -1},
    {INDEX_EXT_stencil_two_side, "GL_EXT_stencil_two_side", GL_FALSE, 625, 625},
    {INDEX_ATI_text_fragment_shader, "GL_ATI_text_fragment_shader", GL_FALSE, -1, -1},
    {INDEX_APPLE_client_storage, "GL_APPLE_client_storage", GL_FALSE, -1, -1},
    {INDEX_APPLE_element_array, "GL_APPLE_element_array", GL_FALSE, -1, -1},
    {INDEX_APPLE_fence, "GL_APPLE_fence", GL_FALSE, -1, -1},
    {INDEX_APPLE_vertex_array_object, "GL_APPLE_vertex_array_object", GL_FALSE, -1, -1},
    {INDEX_APPLE_vertex_array_range, "GL_APPLE_vertex_array_range", GL_FALSE, -1, -1},
    {INDEX_APPLE_ycbcr_422, "GL_APPLE_ycbcr_422", GL_FALSE, -1, -1},
    {INDEX_ATI_draw_buffers, "GL_ATI_draw_buffers", GL_FALSE, -1, -1},
    {INDEX_ATI_texture_env_combine3, "GL_ATI_texture_env_combine3", GL_FALSE, -1, -1},
    {INDEX_ATI_texture_float, "GL_ATI_texture_float", GL_FALSE, -1, -1},
    {INDEX_NV_float_buffer, "GL_NV_float_buffer", GL_FALSE, -1, -1},
    {INDEX_NV_fragment_program, "GL_NV_fragment_program", GL_FALSE, -1, -1},
    {INDEX_NV_half_float, "GL_NV_half_float", GL_FALSE, -1, -1},
    {INDEX_NV_pixel_data_range, "GL_NV_pixel_data_range", GL_FALSE, -1, -1},
    {INDEX_NV_primitive_restart, "GL_NV_primitive_restart", GL_FALSE, -1, -1},
    {INDEX_NV_texture_expand_normal, "GL_NV_texture_expand_normal", GL_FALSE, -1, -1},
    {INDEX_NV_vertex_program2, "GL_NV_vertex_program2", GL_FALSE, -1, -1},
    {INDEX_ATI_map_object_buffer, "GL_ATI_map_object_buffer", GL_FALSE, -1, -1},
    {INDEX_ATI_separate_stencil, "GL_ATI_separate_stencil", GL_FALSE, 780, 780},
    {INDEX_ATI_vertex_attrib_array_object, "GL_ATI_vertex_attrib_array_object", GL_FALSE, 619, 621},
    {INDEX_OES_byte_coordinates, "GL_OES_byte_coordinates", GL_FALSE, -1, -1},
    {INDEX_OES_fixed_point, "GL_OES_fixed_point", GL_FALSE, -1, -1},
    {INDEX_OES_single_precision, "GL_OES_single_precision", GL_FALSE, -1, -1},
    {INDEX_OES_compressed_paletted_texture, "GL_OES_compressed_paletted_texture", GL_FALSE, -1, -1},
    {INDEX_OES_read_format, "GL_OES_read_format", GL_FALSE, -1, -1},
    {INDEX_OES_query_matrix, "GL_OES_query_matrix", GL_FALSE, -1, -1},
    {INDEX_EXT_depth_bounds_test, "GL_EXT_depth_bounds_test", GL_FALSE, 627, 627},
    {INDEX_EXT_texture_mirror_clamp, "GL_EXT_texture_mirror_clamp", GL_FALSE, -1, -1},
    {INDEX_EXT_blend_equation_separate, "GL_EXT_blend_equation_separate", GL_FALSE, -1, -1},
    {INDEX_MESA_pack_invert, "GL_MESA_pack_invert", GL_FALSE, -1, -1},
    {INDEX_MESA_ycbcr_texture, "GL_MESA_ycbcr_texture", GL_FALSE, -1, -1},
    {INDEX_EXT_pixel_buffer_object, "GL_EXT_pixel_buffer_object", GL_FALSE, -1, -1},
    {INDEX_NV_fragment_program_option, "GL_NV_fragment_program_option", GL_FALSE, -1, -1},
    {INDEX_NV_fragment_program2, "GL_NV_fragment_program2", GL_FALSE, -1, -1},
    {INDEX_NV_vertex_program2_option, "GL_NV_vertex_program2_option", GL_FALSE, -1, -1},
    {INDEX_NV_vertex_program3, "GL_NV_vertex_program3", GL_FALSE, -1, -1},
    {INDEX_SGIX_hyperpipe, "GLX_SGIX_hyperpipe", GL_FALSE, -1, -1},
    {INDEX_MESA_agp_offset, "GLX_MESA_agp_offset", GL_FALSE, -1, -1},
    {INDEX_EXT_texture_compression_dxt1, "GL_EXT_texture_compression_dxt1", GL_FALSE, -1, -1},
    {INDEX_EXT_static_vertex_array, "GL_EXT_static_vertex_array", GL_FALSE, -1, -1},
    {INDEX_EXT_vertex_array_set, "GL_EXT_vertex_array_set", GL_FALSE, -1, -1},
    {INDEX_EXT_vertex_array_setXXX, "GL_EXT_vertex_array_setXXX", GL_FALSE, -1, -1},
    {INDEX_SGIX_fog_texture, "GL_SGIX_fog_texture", GL_FALSE, -1, -1},
    {INDEX_SGIX_fragment_specular_lighting, "GL_SGIX_fragment_specular_lighting", GL_FALSE, -1, -1},
    {INDEX_WIN_swap_hint, "GL_WIN_swap_hint", GL_FALSE, -1, -1},
    {INDEX_EXT_color_table, "GL_EXT_color_table", GL_FALSE, -1, -1},
    {INDEX_EXT_color_matrix, "GL_SGI_color_matrix", GL_FALSE, -1, -1},
    {INDEX_EXT_texture_cube_map, "GL_EXT_texture_cube_map", GL_FALSE, -1, -1},
    {INDEX_EXT_texture_edge_clamp, "GL_EXT_texture_edge_clamp", GL_FALSE, -1, -1},
    {INDEX_ARB_imaging, "GL_ARB_imaging", GL_FALSE, -1, -1},
    {INDEX_EXT_framebuffer_object, "GL_EXT_framebuffer_object", GL_FALSE, 628, 646},
    {INDEX_EXT_framebuffer_blit, "GL_EXT_framebuffer_blit", GL_FALSE, -1, -1},
    {INDEX_EXT_bindable_uniform, "GL_EXT_bindable_uniform", GL_FALSE, 649, 651},
    {INDEX_EXT_framebuffer_multisample, "GL_EXT_framebuffer_multisample", GL_FALSE, -1, -1},
    {INDEX_EXT_texture_compression_latc, "GL_EXT_texture_compression_latc", GL_FALSE, -1, -1},
    {INDEX_EXT_texture_compression_rgtc, "GL_EXT_texture_compression_rgtc", GL_FALSE, -1, -1},
    {INDEX_EXT_texture_integer, "GL_EXT_texture_integer", GL_FALSE, 652, 657},
    {INDEX_EXT_gpu_shader4, "GL_EXT_gpu_shader4", GL_FALSE, 658, 691},
    {INDEX_EXT_texture_array, "GL_EXT_texture_array", GL_FALSE, 694, 694},
    {INDEX_EXT_geometry_shader4, "GL_EXT_geometry_shader4", GL_FALSE, 692, 695},
    {INDEX_EXT_draw_buffers2, "GL_EXT_draw_buffers2", GL_FALSE, 696, 701},
    {INDEX_EXT_texture_buffer_object, "GL_EXT_texture_buffer_object", GL_FALSE, 702, 702},
    {INDEX_EXT_texture_sRGB, "GL_EXT_texture_sRGB", GL_FALSE, -1, -1},
    {INDEX_EXT_texture_shared_exponent, "GL_EXT_texture_shared_exponent", GL_FALSE, -1, -1},
    {INDEX_EXT_gpu_program_parameters, "GL_EXT_gpu_program_parameters", GL_FALSE, 703, 704},
    {INDEX_EXT_packed_float, "GL_EXT_packed_float",GL_FALSE,-1,-1},
    {INDEX_EXT_draw_instanced, "GL_EXT_draw_instanced", GL_FALSE, 705, 706},
    {INDEX_ARB_color_buffer_float, "GL_ARB_color_buffer_float", GL_FALSE, 707, 707},
    {INDEX_EXT_timer_query, "GL_EXT_timer_query", GL_FALSE,708,709},
    {INDEX_EXT_packed_depth_stencil, "GL_EXT_packed_depth_stencil", GL_TRUE,-1,-1},

    {INDEX_EXT_LAST, NULL, GL_FALSE, -1, -1} /*the last tag*/
};

GLvoid __glInitExtensions(__GLcontext * gc)
{
    size_t stringLen = 0;
    __GLextension *curExt;
    GLint index;

#if defined (_DEBUG) || defined (DEBUG)
    for (index = 0; index < INDEX_EXT_LAST; index++)
    {
        GL_ASSERT(__glExtension[index].index == index);
    }
#endif

    /* go through the extension table to get the extension string length */
    curExt = __glExtension;
    while (curExt->index < INDEX_EXT_LAST)
    {
        if (curExt->bEnabled)
        {
            /* add one more space to separate extension strings*/
            stringLen += strlen(curExt->name) + 1;
        }
        curExt++;
    }
    stringLen++;/*one more for the null character */

    /*allocate buffer to hold the extension string*/
    gc->constants.extensions = (GLbyte *)(*gc->imports.malloc)(gc, stringLen );
    GL_ASSERT(gc->constants.extensions != NULL);
    gc->constants.extensions[0] = '\0';

    /*go through the extension table again to construct the extension string */
    curExt = __glExtension;
    index = 0;
    while (curExt->index < INDEX_EXT_LAST)
    {
        if (curExt->bEnabled)
        {
            strcat((char *)gc->constants.extensions, curExt->name);
            /* add one more space to separate extension strings */
            strcat((char *)gc->constants.extensions, " ");
        }
        curExt++;
        index++;
    }
    GL_ASSERT(stringLen== strlen(gc->constants.extensions)+1);
}

#ifndef _LINUX_

static __GLextFuncAlias __glExtFuncAlias[] =
{
    __GL_EXT_ALIAS_TABLE
};

/*
** The sequence of the __glApiNameString must match the API offsets in glapioffsets.h
*/
static const GLbyte *__glApiNameString[] = {
    __GL_API_ENTRIES(glStr)
};

GLint __glGetDispatchOffset(const GLbyte *procName)
{
    __GLextFuncAlias *extAlias;
    __GLextension *curExt;
    const GLbyte *apiName;
    GLboolean aliasFound;
    GLint offset, index;

    /* Skip the first two characters "gl" of procName */
    apiName = procName + 2;

    /* Find extension function alias name in __glExtFuncAlias[] table */
    aliasFound = GL_FALSE;
    for (extAlias = __glExtFuncAlias; extAlias->index < INDEX_EXT_LAST; extAlias++)
    {
        if ( strcmp(extAlias->procName, apiName) == 0 )
        {
            if (__glExtension[extAlias->index].bEnabled)
            {
                /* Get the aliasName if the extension is enabled */
                apiName = extAlias->aliasName;
                aliasFound = GL_TRUE;
                break;
            }
            else
            {
                return -1;
            }
        }
    }

    /* Find extension function's offset in __glApiNameString[] table */
    offset = -1;
    for (index = 0; index < _gloffset_LAST; index++)
    {
        if (strcmp(__glApiNameString[index], apiName) == 0)
        {
            offset = index;
            break;
        }
    }


    /* Make sure the extension function is enabled in __glExtension[] table */
    if (aliasFound == GL_FALSE)
    {
        for (curExt = __glExtension; curExt->index < INDEX_EXT_LAST; curExt++)
        {
            if ((curExt->endIndex >= offset) && (curExt->startIndex <= offset))
            {
                if (curExt->bEnabled == GL_FALSE)
                {
                    offset = -1;
                    break;
                }
            }
        }
    }

    return offset;
}

#endif
