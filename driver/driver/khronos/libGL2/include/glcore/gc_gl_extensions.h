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


#ifndef __gc_gl_extensions_h_
#define __gc_gl_extensions_h_

typedef enum{

    /*
    ** These are actually wgl extensions, but are put here because their names
    ** should be exported in the gl extension strings
    */
    INDEX_ARB_extensions_string = 0,
    INDEX_EXT_extensions_string,
    INDEX_GL_EXT_swap_control,

    INDEX_S3_s3tc,
    INDEX_EXT_texture_env_add,
    INDEX_ARB_multitexture,
    INDEX_ARB_get_proc_address,
    INDEX_ARB_transpose_matrix,
    INDEX_ARB_multisample,
    INDEX_ARB_texture_env_add,
    INDEX_ARB_texture_cube_map,
    INDEX_ARB_texture_compression,
    INDEX_ARB_texture_border_clamp,
    INDEX_ARB_point_parameters,
    INDEX_ARB_vertex_blend,
    INDEX_ARB_matrix_palette,
    INDEX_ARB_texture_env_combine,
    INDEX_ARB_texture_env_crossbar,
    INDEX_ARB_texture_env_dot3,
    INDEX_ARB_texture_mirrored_repeat,
    INDEX_ARB_depth_texture,
    INDEX_ARB_shadow,
    INDEX_ARB_shadow_ambient,
    INDEX_ARB_window_pos,
    INDEX_ARB_vertex_program,
    INDEX_ARB_fragment_program,
    INDEX_ARB_vertex_buffer_object,
    INDEX_ARB_occlusion_query,
    INDEX_ARB_shader_objects,
    INDEX_ARB_vertex_shader,
    INDEX_ARB_fragment_shader,
    INDEX_ARB_shading_language_100,
    INDEX_ARB_texture_non_power_of_two,
    INDEX_ARB_point_sprite,
    INDEX_ARB_fragment_program_shadow,
    INDEX_ARB_draw_buffers,
    INDEX_ARB_texture_rectangle,
    INDEX_ARB_half_float_pixel,
    INDEX_ARB_texture_float,
    INDEX_ARB_pixel_buffer_object,
    INDEX_EXT_abgr,
    INDEX_EXT_blend_color,
    INDEX_EXT_polygon_offset,
    INDEX_EXT_texture,
    INDEX_EXT_texture3D,
    INDEX_SGIS_texture_filter4,
    INDEX_EXT_subtexture,
    INDEX_EXT_copy_texture,
    INDEX_EXT_histogram,
    INDEX_EXT_convolution,
    INDEX_SGI_color_matrix,
    INDEX_SGI_color_table,
    INDEX_SGIS_pixel_texture,
    INDEX_SGI_texture_color_table,
    INDEX_EXT_cmyka,
    INDEX_EXT_texture_object,
    INDEX_SGIS_detail_texture,
    INDEX_SGIS_sharpen_texture,
    INDEX_EXT_packed_pixels,
    INDEX_SGIS_texture_lod,
    INDEX_SGIS_multisample,
    INDEX_EXT_rescale_normal,
    INDEX_EXT_visual_info,
    INDEX_EXT_vertex_array,
    INDEX_EXT_misc_attribute,
    INDEX_SGIS_generate_mipmap,
    INDEX_SGIX_clipmap,
    INDEX_SGIX_shadow,
    INDEX_SGIS_texture_edge_clamp,
    INDEX_SGIS_texture_border_clamp,
    INDEX_EXT_blend_minmax,
    INDEX_EXT_blend_subtract,
    INDEX_EXT_blend_logic_op,
    INDEX_SGI_swap_control,
    INDEX_SGI_video_sync,
    INDEX_SGI_make_current_read,
    INDEX_SGIX_video_source,
    INDEX_EXT_visual_rating,
    INDEX_SGIX_interlace,
    INDEX_EXT_import_context,
    INDEX_SGIX_fbconfig,
    INDEX_SGIX_pbuffer,
    INDEX_SGIS_texture_select,
    INDEX_SGIX_sprite,
    INDEX_SGIX_texture_multi_buffer,
    INDEX_EXT_point_parameters,
    INDEX_SGIX_instruments,
    INDEX_SGIX_texture_scale_bias,
    INDEX_SGIX_framezoom,
    INDEX_SGIX_tag_sample_buffer,
    INDEX_SGIX_reference_plane,
    INDEX_SGIX_flush_raster,
    INDEX_SGI_cushion,
    INDEX_SGIX_depth_texture,
    INDEX_SGIS_fog_function,
    INDEX_SGIX_fog_offset,
    INDEX_HP_image_transform,
    INDEX_HP_convolution_border_modes,
    INDEX_SGIX_texture_add_env,
    INDEX_EXT_color_subtable,
    INDEX_EXT_object_space_tess,
    INDEX_PGI_vertex_hints,
    INDEX_PGI_misc_hints,
    INDEX_EXT_paletted_texture,
    INDEX_EXT_clip_volume_hint,
    INDEX_SGIX_list_priority,
    INDEX_SGIX_ir_instrument1,
    INDEX_SGIX_video_resize,
    INDEX_SGIX_texture_lod_bias,
    INDEX_SGI_filter4_parameters,
    INDEX_SGIX_dm_buffer,
    INDEX_SGIX_shadow_ambient,
    INDEX_SGIX_swap_group,
    INDEX_SGIX_swap_barrier,
    INDEX_EXT_index_texture,
    INDEX_EXT_index_material,
    INDEX_EXT_index_func,
    INDEX_EXT_index_array_formats,
    INDEX_EXT_cull_vertex,
    INDEX_EXT_nurbs_tessellator,
    INDEX_SGIX_ycrcb,
    INDEX_EXT_fragment_lighting,
    INDEX_IBM_rasterpos_clip,
    INDEX_HP_texture_lighting,
    INDEX_EXT_draw_range_elements,
    INDEX_WIN_phong_shading,
    INDEX_WIN_specular_fog,
    INDEX_SGIS_color_range,
    INDEX_EXT_light_texture,
    INDEX_SGIX_blend_alpha_minmax,
    INDEX_EXT_scene_marker,
    INDEX_SGIX_pixel_texture_bits,
    INDEX_EXT_bgra,
    INDEX_SGIX_async,
    INDEX_SGIX_async_pixel,
    INDEX_SGIX_async_histogram,
    INDEX_INTEL_texture_scissor,
    INDEX_INTEL_parallel_arrays,
    INDEX_HP_occlusion_test,
    INDEX_EXT_pixel_transform,
    INDEX_EXT_pixel_transform_color_table,
    INDEX_EXT_shared_texture_palette,
    INDEX_SGIS_blended_overlay,
    INDEX_EXT_separate_specular_color,
    INDEX_EXT_secondary_color,
    INDEX_EXT_texture_env,
    INDEX_EXT_texture_perturb_normal,
    INDEX_EXT_multi_draw_arrays,
    INDEX_EXT_fog_coord,
    INDEX_REND_screen_coordinates,
    INDEX_EXT_coordinate_frame,
    INDEX_EXT_texture_env_combine,
    INDEX_APPLE_specular_vector,
    INDEX_SGIX_pixel_texture,
    INDEX_SGIS_texture4D,
    INDEX_APPLE_transform_hint,
    INDEX_SUNX_constant_data,
    INDEX_SUN_global_alpha,
    INDEX_SUN_triangle_list,
    INDEX_SUN_vertex,
    INDEX_EXT_blend_func_separate,
    INDEX_INGR_color_clamp,
    INDEX_INGR_interlace_read,
    INDEX_EXT_stencil_wrap,
    INDEX_EXT_422_pixels,
    INDEX_NV_texgen_reflection,
    INDEX_SGIX_texture_range,
    INDEX_SUN_convolution_border_modes,
    INDEX_SUN_get_transparent_index,
    INDEX_EXT_texture_lod_bias,
    INDEX_EXT_texture_filter_anisotropic,
    INDEX_EXT_vertex_weighting,
    INDEX_NV_light_max_exponent,
    INDEX_NV_vertex_array_range,
    INDEX_NV_register_combiners,
    INDEX_NV_fog_distance,
    INDEX_NV_texgen_emboss,
    INDEX_NV_blend_square,
    INDEX_NV_texture_env_combine4,
    INDEX_MESA_resize_buffers,
    INDEX_MESA_window_pos,
    INDEX_EXT_texture_compression_s3tc,
    INDEX_IBM_cull_vertex,
    INDEX_IBM_multimode_draw_arrays,
    INDEX_IBM_vertex_array_lists,
    INDEX_3DFX_texture_compression_FXT1,
    INDEX_3DFX_multisample,
    INDEX_3DFX_tbuffer,
    INDEX_SGIX_vertex_preclip,
    INDEX_SGIX_resample,
    INDEX_SGIS_texture_color_mask,
    INDEX_MESA_copy_sub_buffer,
    INDEX_MESA_pixmap_colormap,
    INDEX_MESA_release_buffers,
    INDEX_MESA_set_3dfx_mode,
    INDEX_EXT_texture_env_dot3,
    INDEX_ATI_texture_mirror_once,
    INDEX_NV_fence,
    INDEX_IBM_static_data,
    INDEX_IBM_texture_mirrored_repeat,
    INDEX_NV_evaluators,
    INDEX_NV_packed_depth_stencil,
    INDEX_NV_register_combiners2,
    INDEX_NV_texture_compression_vtc,
    INDEX_NV_texture_rectangle,
    INDEX_NV_texture_shader,
    INDEX_NV_texture_shader2,
    INDEX_NV_vertex_array_range2,
    INDEX_NV_vertex_program,
    INDEX_SGIX_visual_select_group,
    INDEX_SGIX_texture_coordinate_clamp,
    INDEX_OML_swap_method,
    INDEX_OML_sync_control,
    INDEX_OML_interlace,
    INDEX_OML_subsample,
    INDEX_OML_resample,
    INDEX_NV_copy_depth_to_color,
    INDEX_ATI_envmap_bumpmap,
    INDEX_ATI_pn_triangles,
    INDEX_ATI_vertex_array_object,
    INDEX_ATI_vertex_streams,
    INDEX_ATI_element_array,
    INDEX_SUN_mesh_array,
    INDEX_SUN_slice_accum,
    INDEX_NV_multisample_filter_hint,
    INDEX_NV_depth_clamp,
    INDEX_NV_occlusion_query,
    INDEX_NV_point_sprite,
    INDEX_NV_texture_shader3,
    INDEX_NV_vertex_program1_1,
    INDEX_EXT_shadow_funcs,
    INDEX_EXT_stencil_two_side,
    INDEX_ATI_text_fragment_shader,
    INDEX_APPLE_client_storage,
    INDEX_APPLE_element_array,
    INDEX_APPLE_fence,
    INDEX_APPLE_vertex_array_object,
    INDEX_APPLE_vertex_array_range,
    INDEX_APPLE_ycbcr_422,
    INDEX_ATI_draw_buffers,
    INDEX_ATI_texture_env_combine3,
    INDEX_ATI_texture_float,
    INDEX_NV_float_buffer,
    INDEX_NV_fragment_program,
    INDEX_NV_half_float,
    INDEX_NV_pixel_data_range,
    INDEX_NV_primitive_restart,
    INDEX_NV_texture_expand_normal,
    INDEX_NV_vertex_program2,
    INDEX_ATI_map_object_buffer,
    INDEX_ATI_separate_stencil,
    INDEX_ATI_vertex_attrib_array_object,
    INDEX_OES_byte_coordinates,
    INDEX_OES_fixed_point,
    INDEX_OES_single_precision,
    INDEX_OES_compressed_paletted_texture,
    INDEX_OES_read_format,
    INDEX_OES_query_matrix,
    INDEX_EXT_depth_bounds_test,
    INDEX_EXT_texture_mirror_clamp,
    INDEX_EXT_blend_equation_separate,
    INDEX_MESA_pack_invert,
    INDEX_MESA_ycbcr_texture,
    INDEX_EXT_pixel_buffer_object,
    INDEX_NV_fragment_program_option,
    INDEX_NV_fragment_program2,
    INDEX_NV_vertex_program2_option,
    INDEX_NV_vertex_program3,
    INDEX_SGIX_hyperpipe,
    INDEX_MESA_agp_offset,
    INDEX_EXT_texture_compression_dxt1,
    INDEX_EXT_static_vertex_array,
    INDEX_EXT_vertex_array_set,
    INDEX_EXT_vertex_array_setXXX,
    INDEX_SGIX_fog_texture,
    INDEX_SGIX_fragment_specular_lighting,
    INDEX_WIN_swap_hint,
    INDEX_EXT_color_table,
    INDEX_EXT_color_matrix,
    INDEX_EXT_texture_cube_map,
    INDEX_EXT_texture_edge_clamp,
    INDEX_ARB_imaging,
    INDEX_EXT_framebuffer_object,
    INDEX_EXT_framebuffer_blit,
    INDEX_EXT_bindable_uniform,
    INDEX_EXT_framebuffer_multisample,
    INDEX_EXT_texture_compression_latc,
    INDEX_EXT_texture_compression_rgtc,
    INDEX_EXT_texture_integer,
    INDEX_EXT_gpu_shader4,
    INDEX_EXT_texture_array,
    INDEX_EXT_geometry_shader4,
    INDEX_EXT_draw_buffers2,
    INDEX_EXT_texture_buffer_object,
    INDEX_EXT_texture_sRGB,
    INDEX_EXT_texture_shared_exponent,
    INDEX_EXT_gpu_program_parameters,
    INDEX_EXT_packed_float,
    INDEX_EXT_draw_instanced,
    INDEX_ARB_color_buffer_float,
    INDEX_EXT_timer_query,
    INDEX_EXT_packed_depth_stencil,
    INDEX_EXT_LAST,
} GL_EXT_INDEX;

#define __GL_EXT_ALIAS_TABLE \
    /* extension functions for GL_ARB_multitexture */ \
    {INDEX_ARB_multitexture, "ActiveTextureARB", "ActiveTexture"}, \
    {INDEX_ARB_multitexture, "ClientActiveTextureARB", "ClientActiveTexture"}, \
    {INDEX_ARB_multitexture, "MultiTexCoord1dARB", "MultiTexCoord1d"}, \
    {INDEX_ARB_multitexture, "MultiTexCoord1dvARB", "MultiTexCoord1dv"}, \
    {INDEX_ARB_multitexture, "MultiTexCoord1sARB", "MultiTexCoord1s"}, \
    {INDEX_ARB_multitexture, "MultiTexCoord1svARB", "MultiTexCoord1sv"}, \
    {INDEX_ARB_multitexture, "MultiTexCoord1iARB", "MultiTexCoord1i"}, \
    {INDEX_ARB_multitexture, "MultiTexCoord1ivARB", "MultiTexCoord1iv"}, \
    {INDEX_ARB_multitexture, "MultiTexCoord1fARB", "MultiTexCoord1f"}, \
    {INDEX_ARB_multitexture, "MultiTexCoord1fvARB", "MultiTexCoord1fv"}, \
    {INDEX_ARB_multitexture, "MultiTexCoord2dARB", "MultiTexCoord2d"}, \
    {INDEX_ARB_multitexture, "MultiTexCoord2dvARB", "MultiTexCoord2dv"}, \
    {INDEX_ARB_multitexture, "MultiTexCoord2sARB", "MultiTexCoord2s"}, \
    {INDEX_ARB_multitexture, "MultiTexCoord2svARB", "MultiTexCoord2sv"}, \
    {INDEX_ARB_multitexture, "MultiTexCoord2iARB", "MultiTexCoord2i"}, \
    {INDEX_ARB_multitexture, "MultiTexCoord2ivARB", "MultiTexCoord2iv"}, \
    {INDEX_ARB_multitexture, "MultiTexCoord2fARB", "MultiTexCoord2f"}, \
    {INDEX_ARB_multitexture, "MultiTexCoord2fvARB", "MultiTexCoord2fv"}, \
    {INDEX_ARB_multitexture, "MultiTexCoord3dARB", "MultiTexCoord3d"}, \
    {INDEX_ARB_multitexture, "MultiTexCoord3dvARB", "MultiTexCoord3dv"}, \
    {INDEX_ARB_multitexture, "MultiTexCoord3sARB", "MultiTexCoord3s"}, \
    {INDEX_ARB_multitexture, "MultiTexCoord3svARB", "MultiTexCoord3sv"}, \
    {INDEX_ARB_multitexture, "MultiTexCoord3iARB", "MultiTexCoord3i"}, \
    {INDEX_ARB_multitexture, "MultiTexCoord3ivARB", "MultiTexCoord3iv"}, \
    {INDEX_ARB_multitexture, "MultiTexCoord3fARB", "MultiTexCoord3f"}, \
    {INDEX_ARB_multitexture, "MultiTexCoord3fvARB", "MultiTexCoord3fv"}, \
    {INDEX_ARB_multitexture, "MultiTexCoord4dARB", "MultiTexCoord4d"}, \
    {INDEX_ARB_multitexture, "MultiTexCoord4dvARB", "MultiTexCoord4dv"}, \
    {INDEX_ARB_multitexture, "MultiTexCoord4sARB", "MultiTexCoord4s"}, \
    {INDEX_ARB_multitexture, "MultiTexCoord4svARB", "MultiTexCoord4sv"}, \
    {INDEX_ARB_multitexture, "MultiTexCoord4iARB", "MultiTexCoord4i"}, \
    {INDEX_ARB_multitexture, "MultiTexCoord4ivARB", "MultiTexCoord4iv"}, \
    {INDEX_ARB_multitexture, "MultiTexCoord4fARB", "MultiTexCoord4f"}, \
    {INDEX_ARB_multitexture, "MultiTexCoord4fvARB", "MultiTexCoord4fv"}, \
    \
    /* extension functions for  GL_ARB_transpose_matrix */ \
    {INDEX_ARB_transpose_matrix, "LoadTransposeMatrixdARB", "LoadTransposeMatrixd"}, \
    {INDEX_ARB_transpose_matrix, "LoadTransposeMatrixfARB", "LoadTransposeMatrixf"}, \
    {INDEX_ARB_transpose_matrix, "MultTransposeMatrixdARB", "MultTransposeMatrixd"}, \
    {INDEX_ARB_transpose_matrix, "MultTransposeMatrixfARB", "MultTransposeMatrixf"}, \
    \
    /* extension functions for  GL_ARB_multisample */ \
    {INDEX_ARB_multisample, "SampleCoverageARB", "SampleCoverage"}, \
    \
    /* extension functions for  GL_ARB_texture_compression */ \
    {INDEX_ARB_texture_compression, "CompressedTexImage3DARB", "CompressedTexImage3D"}, \
    {INDEX_ARB_texture_compression, "CompressedTexImage2DARB", "CompressedTexImage2D"}, \
    {INDEX_ARB_texture_compression, "CompressedTexImage1DARB", "CompressedTexImage1D"}, \
    {INDEX_ARB_texture_compression, "CompressedTexSubImage3DARB", "CompressedTexSubImage3D"}, \
    {INDEX_ARB_texture_compression, "CompressedTexSubImage2DARB", "CompressedTexSubImage2D"}, \
    {INDEX_ARB_texture_compression, "CompressedTexSubImage1DARB", "CompressedTexSubImage1D"}, \
    {INDEX_ARB_texture_compression, "GetCompressedTexImageARB", "GetCompressedTexImage"}, \
    \
    /* extension functions for  GL_ARB_point_parameters */ \
    {INDEX_ARB_point_parameters, "PointParameterfARB", "PointParameterf"}, \
    {INDEX_ARB_point_parameters, "PointParameterfvARB", "PointParameterfv"}, \
    \
    /* extension functions for  GL_ARB_window_pos */ \
    {INDEX_ARB_window_pos, "WindowPos2dARB", "WindowPos2d"}, \
    {INDEX_ARB_window_pos, "WindowPos2dvARB", "WindowPos2dv"}, \
    {INDEX_ARB_window_pos, "WindowPos2fARB", "WindowPos2f"}, \
    {INDEX_ARB_window_pos, "WindowPos2fvARB", "WindowPos2fv"}, \
    {INDEX_ARB_window_pos, "WindowPos2iARB", "WindowPos2i"}, \
    {INDEX_ARB_window_pos, "WindowPos2ivARB", "WindowPos2iv"}, \
    {INDEX_ARB_window_pos, "WindowPos2sARB", "WindowPos2s"}, \
    {INDEX_ARB_window_pos, "WindowPos2svARB", "WindowPos2sv"}, \
    {INDEX_ARB_window_pos, "WindowPos3dARB", "WindowPos3d"}, \
    {INDEX_ARB_window_pos, "WindowPos3dvARB", "WindowPos3dv"}, \
    {INDEX_ARB_window_pos, "WindowPos3fARB", "WindowPos3f"}, \
    {INDEX_ARB_window_pos, "WindowPos3fvARB", "WindowPos3fv"}, \
    {INDEX_ARB_window_pos, "WindowPos3iARB", "WindowPos3i"}, \
    {INDEX_ARB_window_pos, "WindowPos3ivARB", "WindowPos3iv"}, \
    {INDEX_ARB_window_pos, "WindowPos3sARB", "WindowPos3s"}, \
    {INDEX_ARB_window_pos, "WindowPos3svARB", "WindowPos3sv"}, \
    \
    /* extension functions for GL_ARB_vertex_program */ \
    {INDEX_ARB_vertex_program, "VertexAttrib1sARB", "VertexAttrib1s"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib1fARB", "VertexAttrib1f"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib1dARB", "VertexAttrib1d"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib2sARB", "VertexAttrib2s"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib2fARB", "VertexAttrib2f"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib2dARB", "VertexAttrib2d"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib3sARB", "VertexAttrib3s"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib3fARB", "VertexAttrib3f"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib3dARB", "VertexAttrib3d"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib4sARB", "VertexAttrib4s"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib4fARB", "VertexAttrib4f"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib4dARB", "VertexAttrib4d"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib4NubARB", "VertexAttrib4Nub"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib1svARB", "VertexAttrib1sv"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib1fvARB", "VertexAttrib1fv"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib1dvARB", "VertexAttrib1dv"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib2svARB", "VertexAttrib2sv"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib2fvARB", "VertexAttrib2fv"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib2dvARB", "VertexAttrib2dv"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib3svARB", "VertexAttrib3sv"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib3fvARB", "VertexAttrib3fv"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib3dvARB", "VertexAttrib3dv"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib4bvARB", "VertexAttrib4bv"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib4svARB", "VertexAttrib4sv"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib4ivARB", "VertexAttrib4iv"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib4ubvARB", "VertexAttrib4ubv"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib4usvARB", "VertexAttrib4usv"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib4uivARB", "VertexAttrib4uiv"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib4fvARB", "VertexAttrib4fv"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib4dvARB", "VertexAttrib4dv"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib4NbvARB", "VertexAttrib4Nbv"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib4NsvARB", "VertexAttrib4Nsv"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib4NivARB", "VertexAttrib4Niv"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib4NubvARB", "VertexAttrib4Nubv"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib4NusvARB", "VertexAttrib4Nusv"}, \
    {INDEX_ARB_vertex_program, "VertexAttrib4NuivARB", "VertexAttrib4Nuiv"}, \
    {INDEX_ARB_vertex_program, "VertexAttribPointerARB", "VertexAttribPointer"}, \
    {INDEX_ARB_vertex_program, "EnableVertexAttribArrayARB", "EnableVertexAttribArray"}, \
    {INDEX_ARB_vertex_program, "DisableVertexAttribArrayARB", "DisableVertexAttribArray"}, \
    {INDEX_ARB_vertex_program, "ProgramStringARB", "ProgramStringARB"}, \
    {INDEX_ARB_vertex_program, "BindProgramARB", "BindProgramARB"}, \
    {INDEX_ARB_vertex_program, "DeleteProgramsARB", "DeleteProgramsARB"}, \
    {INDEX_ARB_vertex_program, "GenProgramsARB", "GenProgramsARB"}, \
    {INDEX_ARB_vertex_program, "ProgramEnvParameter4dARB", "ProgramEnvParameter4dARB"}, \
    {INDEX_ARB_vertex_program, "ProgramEnvParameter4dvARB", "ProgramEnvParameter4dvARB"}, \
    {INDEX_ARB_vertex_program, "ProgramEnvParameter4fARB", "ProgramEnvParameter4fARB"}, \
    {INDEX_ARB_vertex_program, "ProgramEnvParameter4fvARB", "ProgramEnvParameter4fvARB"}, \
    {INDEX_ARB_vertex_program, "ProgramLocalParameter4dARB", "ProgramLocalParameter4dARB"}, \
    {INDEX_ARB_vertex_program, "ProgramLocalParameter4dvARB", "ProgramLocalParameter4dvARB"}, \
    {INDEX_ARB_vertex_program, "ProgramLocalParameter4fARB", "ProgramLocalParameter4fARB"}, \
    {INDEX_ARB_vertex_program, "ProgramLocalParameter4fvARB", "ProgramLocalParameter4fvARB"}, \
    {INDEX_ARB_vertex_program, "GetProgramEnvParameterdvARB", "GetProgramEnvParameterdvARB"}, \
    {INDEX_ARB_vertex_program, "GetProgramEnvParameterfvARB", "GetProgramEnvParameterfvARB"}, \
    {INDEX_ARB_vertex_program, "GetProgramLocalParameterdvARB", "GetProgramLocalParameterdvARB"}, \
    {INDEX_ARB_vertex_program, "GetProgramLocalParameterfvARB", "GetProgramLocalParameterfvARB"}, \
    {INDEX_ARB_vertex_program, "GetProgramivARB", "GetProgramivARB"}, \
    {INDEX_ARB_vertex_program, "GetProgramStringARB", "GetProgramStringARB"}, \
    {INDEX_ARB_vertex_program, "GetVertexAttribdvARB", "GetVertexAttribdv"}, \
    {INDEX_ARB_vertex_program, "GetVertexAttribfvARB", "GetVertexAttribfv"}, \
    {INDEX_ARB_vertex_program, "GetVertexAttribivARB", "GetVertexAttribiv"}, \
    {INDEX_ARB_vertex_program, "GetVertexAttribPointervARB", "GetVertexAttribPointerv"}, \
    {INDEX_ARB_vertex_program, "IsProgramARB", "IsProgramARB"}, \
    \
    /* extension functions for GL_ARB_vertex_buffer_object */ \
    {INDEX_ARB_vertex_buffer_object, "BindBufferARB", "BindBuffer"}, \
    {INDEX_ARB_vertex_buffer_object, "DeleteBuffersARB", "DeleteBuffers"}, \
    {INDEX_ARB_vertex_buffer_object, "GenBuffersARB", "GenBuffers"}, \
    {INDEX_ARB_vertex_buffer_object, "IsBufferARB", "IsBuffer"}, \
    {INDEX_ARB_vertex_buffer_object, "BufferDataARB", "BufferData"}, \
    {INDEX_ARB_vertex_buffer_object, "BufferSubDataARB", "BufferSubData"}, \
    {INDEX_ARB_vertex_buffer_object, "GetBufferSubDataARB", "GetBufferSubData"}, \
    {INDEX_ARB_vertex_buffer_object, "MapBufferARB", "MapBuffer"}, \
    {INDEX_ARB_vertex_buffer_object, "UnmapBufferARB", "UnmapBuffer"}, \
    {INDEX_ARB_vertex_buffer_object, "GetBufferParameterivARB", "GetBufferParameteriv"}, \
    {INDEX_ARB_vertex_buffer_object, "GetBufferPointervARB", "GetBufferPointerv"}, \
    \
    /* extension functions for GL_ARB_occlusion_query */ \
    {INDEX_ARB_occlusion_query, "GenQueriesARB", "GenQueries"}, \
    {INDEX_ARB_occlusion_query, "DeleteQueriesARB", "DeleteQueries"}, \
    {INDEX_ARB_occlusion_query, "IsQueryARB", "IsQuery"}, \
    {INDEX_ARB_occlusion_query, "BeginQueryARB", "BeginQuery"}, \
    {INDEX_ARB_occlusion_query, "EndQueryARB", "EndQuery"}, \
    {INDEX_ARB_occlusion_query, "GetQueryivARB", "GetQueryiv"}, \
    {INDEX_ARB_occlusion_query, "GetQueryObjectivARB", "GetQueryObjectiv"}, \
    {INDEX_ARB_occlusion_query, "GetQueryObjectuivARB", "GetQueryObjectuiv"}, \
    \
    /* extension functions for GL_ARB_shader_objects */ \
    {INDEX_ARB_shader_objects, "DeleteObjectARB", "DeleteObjectARB"}, \
    {INDEX_ARB_shader_objects, "GetHandleARB", "GetHandleARB"}, \
    {INDEX_ARB_shader_objects, "DetachObjectARB", "DetachShader"}, \
    {INDEX_ARB_shader_objects, "CreateShaderObjectARB", "CreateShader"}, \
    {INDEX_ARB_shader_objects, "ShaderSourceARB", "ShaderSource"}, \
    {INDEX_ARB_shader_objects, "CompileShaderARB", "CompileShader"}, \
    {INDEX_ARB_shader_objects, "CreateProgramObjectARB", "CreateProgram"}, \
    {INDEX_ARB_shader_objects, "AttachObjectARB", "AttachShader"}, \
    {INDEX_ARB_shader_objects, "LinkProgramARB", "LinkProgram"}, \
    {INDEX_ARB_shader_objects, "UseProgramObjectARB", "UseProgram"}, \
    {INDEX_ARB_shader_objects, "ValidateProgramARB", "ValidateProgram"}, \
    {INDEX_ARB_shader_objects, "Uniform1fARB", "Uniform1f"}, \
    {INDEX_ARB_shader_objects, "Uniform2fARB", "Uniform2f"}, \
    {INDEX_ARB_shader_objects, "Uniform3fARB", "Uniform3f"}, \
    {INDEX_ARB_shader_objects, "Uniform4fARB", "Uniform4f"}, \
    {INDEX_ARB_shader_objects, "Uniform1iARB", "Uniform1i"}, \
    {INDEX_ARB_shader_objects, "Uniform2iARB", "Uniform2i"}, \
    {INDEX_ARB_shader_objects, "Uniform3iARB", "Uniform3i"}, \
    {INDEX_ARB_shader_objects, "Uniform4iARB", "Uniform4i"}, \
    {INDEX_ARB_shader_objects, "Uniform1fvARB", "Uniform1fv"}, \
    {INDEX_ARB_shader_objects, "Uniform2fvARB", "Uniform2fv"}, \
    {INDEX_ARB_shader_objects, "Uniform3fvARB", "Uniform3fv"}, \
    {INDEX_ARB_shader_objects, "Uniform4fvARB", "Uniform4fv"}, \
    {INDEX_ARB_shader_objects, "Uniform1ivARB", "Uniform1iv"}, \
    {INDEX_ARB_shader_objects, "Uniform2ivARB", "Uniform2iv"}, \
    {INDEX_ARB_shader_objects, "Uniform3ivARB", "Uniform3iv"}, \
    {INDEX_ARB_shader_objects, "Uniform4ivARB", "Uniform4iv"}, \
    {INDEX_ARB_shader_objects, "UniformMatrix2fvARB", "UniformMatrix2fv"}, \
    {INDEX_ARB_shader_objects, "UniformMatrix3fvARB", "UniformMatrix3fv"}, \
    {INDEX_ARB_shader_objects, "UniformMatrix4fvARB", "UniformMatrix4fv"}, \
    {INDEX_ARB_shader_objects, "GetObjectParameterfvARB", "GetObjectParameterfvARB"}, \
    {INDEX_ARB_shader_objects, "GetObjectParameterivARB", "GetObjectParameterivARB"}, \
    {INDEX_ARB_shader_objects, "GetInfoLogARB", "GetInfoLogARB"}, \
    {INDEX_ARB_shader_objects, "GetAttachedObjectsARB", "GetAttachedShaders"}, \
    {INDEX_ARB_shader_objects, "GetUniformLocationARB", "GetUniformLocation"}, \
    {INDEX_ARB_shader_objects, "GetActiveUniformARB", "GetActiveUniform"}, \
    {INDEX_ARB_shader_objects, "GetUniformfvARB", "GetUniformfv"}, \
    {INDEX_ARB_shader_objects, "GetUniformivARB", "GetUniformiv"}, \
    {INDEX_ARB_shader_objects, "GetShaderSourceARB", "GetShaderSource"}, \
    \
    /* extension functions for GL_ARB_vertex_shader */ \
    {INDEX_ARB_vertex_shader, "VertexAttrib1sARB", "VertexAttrib1s"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib1fARB", "VertexAttrib1f"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib1dARB", "VertexAttrib1d"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib2sARB", "VertexAttrib2s"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib2fARB", "VertexAttrib2f"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib2dARB", "VertexAttrib2d"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib3sARB", "VertexAttrib3s"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib3fARB", "VertexAttrib3f"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib3dARB", "VertexAttrib3d"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib4sARB", "VertexAttrib4s"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib4fARB", "VertexAttrib4f"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib4dARB", "VertexAttrib4d"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib4NubARB", "VertexAttrib4Nub"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib1svARB", "VertexAttrib1sv"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib1fvARB", "VertexAttrib1fv"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib1dvARB", "VertexAttrib1dv"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib2svARB", "VertexAttrib2sv"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib2fvARB", "VertexAttrib2fv"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib2dvARB", "VertexAttrib2dv"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib3svARB", "VertexAttrib3sv"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib3fvARB", "VertexAttrib3fv"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib3dvARB", "VertexAttrib3dv"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib4bvARB", "VertexAttrib4bv"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib4svARB", "VertexAttrib4sv"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib4ivARB", "VertexAttrib4iv"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib4ubvARB", "VertexAttrib4ubv"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib4usvARB", "VertexAttrib4usv"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib4uivARB", "VertexAttrib4uiv"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib4fvARB", "VertexAttrib4fv"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib4dvARB", "VertexAttrib4dv"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib4NbvARB", "VertexAttrib4Nbv"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib4NsvARB", "VertexAttrib4Nsv"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib4NivARB", "VertexAttrib4Niv"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib4NubvARB", "VertexAttrib4Nubv"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib4NusvARB", "VertexAttrib4Nusv"}, \
    {INDEX_ARB_vertex_shader, "VertexAttrib4NuivARB", "VertexAttrib4Nuiv"}, \
    {INDEX_ARB_vertex_shader, "VertexAttribPointerARB", "VertexAttribPointer"}, \
    {INDEX_ARB_vertex_shader, "EnableVertexAttribArrayARB", "EnableVertexAttribArray"}, \
    {INDEX_ARB_vertex_shader, "DisableVertexAttribArrayARB", "DisableVertexAttribArray"}, \
    \
    {INDEX_ARB_vertex_shader, "BindAttribLocationARB", "BindAttribLocation"}, \
    {INDEX_ARB_vertex_shader, "GetActiveAttribARB", "GetActiveAttrib"}, \
    {INDEX_ARB_vertex_shader, "GetAttribLocationARB", "GetAttribLocation"}, \
    \
    {INDEX_ARB_vertex_shader, "GetVertexAttribdvARB", "GetVertexAttribdv"}, \
    {INDEX_ARB_vertex_shader, "GetVertexAttribfvARB", "GetVertexAttribfv"}, \
    {INDEX_ARB_vertex_shader, "GetVertexAttribivARB", "GetVertexAttribiv"}, \
    {INDEX_ARB_vertex_shader, "GetVertexAttribPointervARB", "GetVertexAttribPointerv"}, \
    \
    /* extension functions for GL_EXT_blend_color */ \
    {INDEX_EXT_blend_color, "BlendColorEXT", "BlendColor"}, \
    \
    /* extension functions for GL_EXT_texture3d */ \
    {INDEX_EXT_texture3D, "TexImage3DEXT", "TexImage3D"}, \
    {INDEX_EXT_texture3D, "TexSubImage3DEXT", "TexSubImage3D"}, \
    {INDEX_EXT_texture3D, "CopyTexSubImage3DEXT", "CopyTexSubImage3D"}, \
    \
    /* extension functions for GL_EXT_subtexture */ \
    {INDEX_EXT_subtexture, "TexSubImage1DEXT", "TexSubImage1D"}, \
    {INDEX_EXT_subtexture, "TexSubImage2DEXT", "TexSubImage2D"}, \
    {INDEX_EXT_subtexture, "TexSubImage3DEXT", "TexSubImage3D"}, \
    \
    /* extension functions for GL_EXT_histogram */ \
    {INDEX_EXT_histogram, "HistogramEXT", "Histogram"}, \
    {INDEX_EXT_histogram, "ResetHistogramEXT", "ResetHistogram"}, \
    {INDEX_EXT_histogram, "GetHistogramEXT", "GetHistogram"}, \
    {INDEX_EXT_histogram, "GetHistogramParameterivEXT", "GetHistogramParameteriv"}, \
    {INDEX_EXT_histogram, "GetHistogramParameterfvEXT", "GetHistogramParameterfv"}, \
    {INDEX_EXT_histogram, "MinmaxEXT", "Minmax"}, \
    {INDEX_EXT_histogram, "ResetMinmaxEXT", "ResetMinmax"}, \
    {INDEX_EXT_histogram, "GetMinmaxEXT", "GetMinmax"}, \
    {INDEX_EXT_histogram, "GetMinmaxParameterivEXT", "GetMinmaxParameteriv"}, \
    {INDEX_EXT_histogram, "GetMinmaxParameterfvEXT", "GetMinmaxParameterfv"}, \
    \
    /* extension functions for GL_EXT_convolution */ \
    {INDEX_EXT_convolution, "ConvolutionFilter1DEXT", "ConvolutionFilter1D"}, \
    {INDEX_EXT_convolution, "ConvolutionFilter2DEXT", "ConvolutionFilter2D"}, \
    {INDEX_EXT_convolution, "SeparableFilter2DEXT", "SeparableFilter2D"}, \
    {INDEX_EXT_convolution, "CopyConvolutionFilter1DEXT", "CopyConvolutionFilter1D"}, \
    {INDEX_EXT_convolution, "CopyConvolutionFilter2DEXT", "CopyConvolutionFilter2D"}, \
    {INDEX_EXT_convolution, "GetConvolutionFilterEXT", "GetConvolutionFilter"}, \
    {INDEX_EXT_convolution, "GetSeparableFilterEXT", "GetSeparableFilter"}, \
    {INDEX_EXT_convolution, "ConvolutionParameteriEXT", "ConvolutionParameteri"}, \
    {INDEX_EXT_convolution, "ConvolutionParameterivEXT", "ConvolutionParameteriv"}, \
    {INDEX_EXT_convolution, "ConvolutionParameterfEXT", "ConvolutionParameterf"}, \
    {INDEX_EXT_convolution, "ConvolutionParameterfvEXT", "ConvolutionParameterfv"}, \
    {INDEX_EXT_convolution, "GetConvolutionParameterivEXT", "GetConvolutionParameteriv"}, \
    {INDEX_EXT_convolution, "GetConvolutionParameterfvEXT", "GetConvolutionParameterfv"}, \
    \
    /* extension functions for GL_EXT_multi_draw_arrays */ \
    {INDEX_EXT_multi_draw_arrays, "MultiDrawArraysEXT", "MultiDrawArrays"}, \
    {INDEX_EXT_multi_draw_arrays, "MultiDrawElementsEXT", "MultiDrawElements"}, \
    \
    /* extension functions for GL_EXT_blend_equation_separate */ \
    {INDEX_EXT_blend_equation_separate, "BlendEquationSeparateEXT", "BlendEquationSeparate"}, \
    \
    /* extension functions for GL_EXT_blend_func_separate */ \
    {INDEX_EXT_blend_func_separate, "BlendFuncSeparateEXT", "BlendFuncSeparate"}, \
    \
    /* extension functions for GL_EXT_vertex_array */ \
    {INDEX_EXT_vertex_array, "ArrayElementEXT", "ArrayElement"}, \
    {INDEX_EXT_vertex_array, "DrawArraysEXT", "DrawArrays"}, \
    {INDEX_EXT_vertex_array, "VertexPointerEXT", "VertexPointer"}, \
    {INDEX_EXT_vertex_array, "NormalPointerEXT", "NormalPointer"}, \
    {INDEX_EXT_vertex_array, "ColorPointerEXT", "ColorPointer"}, \
    {INDEX_EXT_vertex_array, "IndexPointerEXT", "IndexPointer"}, \
    {INDEX_EXT_vertex_array, "TexCoordPointerEXT", "TexCoordPointer"}, \
    {INDEX_EXT_vertex_array, "EdgeFlagPointerEXT", "EdgeFlagPointer"}, \
    {INDEX_EXT_vertex_array, "GetPointervEXT", "GetPointerv"}, \
    \
    /* extension function string for GL_EXT_texture_object */ \
    {INDEX_EXT_texture_object, "AreTexturesResidentEXT", "AreTexturesResident"}, \
    {INDEX_EXT_texture_object, "BindTextureEXT", "BindTexture"}, \
    {INDEX_EXT_texture_object, "DeleteTexturesEXT", "DeleteTextures"}, \
    {INDEX_EXT_texture_object, "GenTexturesEXT", "GenTextures"}, \
    {INDEX_EXT_texture_object, "IsTextureEXT", "IsTexture"}, \
    {INDEX_EXT_texture_object, "PrioritizeTexturesEXT", "PrioritizeTextures"}, \
    \
    /* extension functions for GL_EXT_secondary_color */ \
    {INDEX_EXT_secondary_color, "SecondaryColor3bEXT", "SecondaryColor3b"}, \
    {INDEX_EXT_secondary_color, "SecondaryColor3bvEXT", "SecondaryColor3bv"}, \
    {INDEX_EXT_secondary_color, "SecondaryColor3dEXT", "SecondaryColor3d"}, \
    {INDEX_EXT_secondary_color, "SecondaryColor3dvEXT", "SecondaryColor3dv"}, \
    {INDEX_EXT_secondary_color, "SecondaryColor3fEXT", "SecondaryColor3f"}, \
    {INDEX_EXT_secondary_color, "SecondaryColor3fvEXT", "SecondaryColor3fv"}, \
    {INDEX_EXT_secondary_color, "SecondaryColor3iEXT", "SecondaryColor3i"}, \
    {INDEX_EXT_secondary_color, "SecondaryColor3ivEXT", "SecondaryColor3iv"}, \
    {INDEX_EXT_secondary_color, "SecondaryColor3sEXT", "SecondaryColor3s"}, \
    {INDEX_EXT_secondary_color, "SecondaryColor3svEXT", "SecondaryColor3sv"}, \
    {INDEX_EXT_secondary_color, "SecondaryColor3ubEXT", "SecondaryColor3ub"}, \
    {INDEX_EXT_secondary_color, "SecondaryColor3ubvEXT", "SecondaryColor3ubv"}, \
    {INDEX_EXT_secondary_color, "SecondaryColor3uiEXT", "SecondaryColor3ui"}, \
    {INDEX_EXT_secondary_color, "SecondaryColor3uivEXT", "SecondaryColor3uiv"}, \
    {INDEX_EXT_secondary_color, "SecondaryColor3usEXT", "SecondaryColor3us"}, \
    {INDEX_EXT_secondary_color, "SecondaryColor3usvEXT", "SecondaryColor3usv"}, \
    {INDEX_EXT_secondary_color, "SecondaryColorPointerEXT", "SecondaryColorPointer"}, \
    \
    /* extension functions for GL_EXT_fog_coord */ \
    {INDEX_EXT_fog_coord, "FogCoorddEXT", "FogCoordd"}, \
    {INDEX_EXT_fog_coord, "FogCoorddvEXT", "FogCoorddv"}, \
    {INDEX_EXT_fog_coord, "FogCoordfEXT", "FogCoordf"}, \
    {INDEX_EXT_fog_coord, "FogCoordfvEXT", "FogCoordfv"}, \
    {INDEX_EXT_fog_coord, "FogCoordPointerEXT", "FogCoordPointer"}, \
    \
    /* extension functions for GL_EXT_point_parameters */ \
    {INDEX_EXT_point_parameters, "PointParameterfEXT", "PointParameterf"}, \
    {INDEX_EXT_point_parameters, "PointParameterfvEXT", "PointParameterfv"}, \
    \
    /* extension functions for GL_EXT_blend_subtract */ \
    {INDEX_EXT_blend_subtract, "BlendEquationEXT", "BlendEquation"}, \
    \
    /* extension functions for GL_EXT_blend_minmax */ \
    {INDEX_EXT_blend_minmax, "BlendEquationEXT", "BlendEquation"}, \
    \
    /* extension functions for GL_EXT_draw_range_elements */ \
    {INDEX_EXT_draw_range_elements, "DrawRangeElementsEXT", "DrawRangeElements"}, \
    \
    /* extension functions for GL_ATI_separate_stencil */ \
    {INDEX_ATI_separate_stencil, "StencilOpSeparateATI", "StencilOpSeparate"}, \
    \
    /* extension functions for GL_NV_occlusion_query */ \
    {INDEX_NV_occlusion_query, "GenOcclusionQueriesNV", "GenQueries"}, \
    {INDEX_NV_occlusion_query, "DeleteOcclusionQueriesNV", "DeleteQueries"}, \
    {INDEX_NV_occlusion_query, "IsOcclusionQueryNV", "IsQuery"}, \
    {INDEX_NV_occlusion_query, "BeginOcclusionQueryNV", "BeginQueryNV"}, \
    {INDEX_NV_occlusion_query, "EndOcclusionQueryNV", "EndQueryNV"}, \
    {INDEX_NV_occlusion_query, "GetOcclusionQueryivNV", "GetQueryObjectiv"}, \
    {INDEX_NV_occlusion_query, "GetOcclusionQueryuivNV", "GetQueryObjectuiv"}, \
    \
    /* extension function for GL_ARB_draw_buffers */ \
    {INDEX_ARB_draw_buffers, "DrawBuffersARB", "DrawBuffers"}, \
    \
    /* extension function for GL_EXT_polygon_offset */ \
    {INDEX_EXT_polygon_offset, "PolygonOffsetEXT", "PolygonOffset"},\
    \
    {INDEX_EXT_LAST, NULL, NULL} \


typedef struct __GLextensionRec {
    GL_EXT_INDEX index;         /* the extension index */
    const char *name;           /* the extension name */
    GLboolean bEnabled;         /* is this extension enabled ? */
    GLint startIndex;           /* start index of the extension APIs in dispatch table */
    GLint endIndex;             /* end index of the extension APIs in dispatch table */
} __GLextension;

typedef struct __GLextFuncAliasRec {
    GL_EXT_INDEX index;         /* the extension index to which the function belongs to */
    const char *procName;       /* extension function name */
    const char *aliasName;      /* extension alias function name in dispatch table */
} __GLextFuncAlias;


#ifdef __cplusplus
extern "C" {
#endif

extern __GLextension __glExtension[];

#ifdef __cplusplus
}
#endif

#endif /* __gc_gl_extensions_h_ */
