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


#ifdef OPENGL40
#include <stdlib.h>
#include <search.h>
#include "gc_gl_debug.h"
#include "gc_es_core.h"

#ifdef _WIN32
#define C_DECL __cdecl
#else
#define C_DECL
#endif

#if (defined(_DEBUG) || defined(DEBUG))
static __GLenumName dbg_attribNames[] =
{
    /* AttribMask */
    {GL_CURRENT_BIT,                    "GL_CURRENT_BIT"              },
    {GL_POINT_BIT,                      "GL_POINT_BIT"                },
    {GL_LINE_BIT,                       "GL_LINE_BIT"                 },
    {GL_POLYGON_BIT ,                   "GL_POLYGON_BIT"              },
    {GL_POLYGON_STIPPLE_BIT,            "GL_POLYGON_STIPPLE_BIT"      },
    {GL_PIXEL_MODE_BIT,                 "GL_PIXEL_MODE_BIT"           },
    {GL_LIGHTING_BIT,                   "GL_LIGHTING_BIT"             },
    {GL_FOG_BIT   ,                     "GL_FOG_BIT"                  },
    {GL_DEPTH_BUFFER_BIT ,              "GL_DEPTH_BUFFER_BIT"         },
    {GL_ACCUM_BUFFER_BIT,               "GL_ACCUM_BUFFER_BIT"         },
    {GL_STENCIL_BUFFER_BIT,             "GL_STENCIL_BUFFER_BIT"       },
    {GL_VIEWPORT_BIT  ,                 "GL_VIEWPORT_BIT"             },
    {GL_TRANSFORM_BIT,                  "GL_TRANSFORM_BIT"            },
    {GL_ENABLE_BIT ,                    "GL_ENABLE_BIT"               },
    {GL_COLOR_BUFFER_BIT,               "GL_COLOR_BUFFER_BIT"         },
    {GL_HINT_BIT,                       "GL_HINT_BIT"                 },
    {GL_EVAL_BIT,                       "GL_EVAL_BIT"                 },
    {GL_LIST_BIT,                       "GL_LIST_BIT"                 },
    {GL_TEXTURE_BIT,                    "GL_TEXTURE_BIT"              },
    {GL_SCISSOR_BIT,                    "GL_SCISSOR_BIT"              },
    {GL_MULTISAMPLE_BIT,                "GL_MULTISAMPLE_BIT"          },
    {GL_ALL_ATTRIB_BITS,                "GL_ALL_ATTRIB_BITS"          },

    /*client attribute mask*/
    {GL_CLIENT_PIXEL_STORE_BIT,         "GL_CLIENT_PIXEL_STORE_BIT"   },
    {GL_CLIENT_VERTEX_ARRAY_BIT,        "GL_CLIENT_VERTEX_ARRAY_BIT"  },
    {GL_CLIENT_ALL_ATTRIB_BITS,         "GL_CLIENT_ALL_ATTRIB_BITS"   },

    {GL_MULTISAMPLE_BIT,                "GL_MULTISAMPLE_BIT"          },

};



static __GLenumName dbg_constNames[]=
{
    /*AccumOp*/                                            /*AccumOp*/
    {GL_ACCUM,                                             "GL_ACCUM"},
    {GL_LOAD,                                              "GL_LOAD"},
    {GL_RETURN,                                            "GL_RETURN"},
    {GL_MULT,                                              "GL_MULT"},
    {GL_ADD,                                               "GL_ADD"},

    /*AlphaFunction*/                                      /*AlphaFunction*/
    {GL_NEVER,                                             "GL_NEVER"},
    {GL_LESS,                                              "GL_LESS"},
    {GL_EQUAL,                                             "GL_EQUAL"},
    {GL_LEQUAL,                                            "GL_LEQUAL"},
    {GL_GREATER,                                           "GL_GREATER"},
    {GL_NOTEQUAL,                                          "GL_NOTEQUAL"},
    {GL_GEQUAL,                                            "GL_GEQUAL"},
    {GL_ALWAYS,                                            "GL_ALWAYS"},

    /*BeginMode*/                                          /*BeginMode*/
    {GL_POINTS,                                            "GL_POINTS"},
    {GL_LINES,                                             "GL_LINES"},
    {GL_LINE_LOOP,                                         "GL_LINE_LOOP"},
    {GL_LINE_STRIP,                                        "GL_LINE_STRIP"},
    {GL_TRIANGLES,                                         "GL_TRIANGLES"},
    {GL_TRIANGLE_STRIP,                                    "GL_TRIANGLE_STRIP"},
    {GL_TRIANGLE_FAN,                                      "GL_TRIANGLE_FAN"},
    {GL_QUADS,                                             "GL_QUADS"},
    {GL_QUAD_STRIP,                                        "GL_QUAD_STRIP"},
    {GL_POLYGON,                                           "GL_POLYGON"},



    /*BlendingFactorDest*/                                 /*BlendingFactorDest*/
    {GL_SRC_COLOR,                                         "GL_SRC_COLOR"},
    {GL_ONE_MINUS_SRC_COLOR,                               "GL_ONE_MINUS_SRC_COLOR"},
    {GL_SRC_ALPHA,                                         "GL_SRC_ALPHA"},
    {GL_ONE_MINUS_SRC_ALPHA,                               "GL_ONE_MINUS_SRC_ALPHA"},
    {GL_DST_ALPHA,                                         "GL_DST_ALPHA"},
    {GL_ONE_MINUS_DST_ALPHA,                               "GL_ONE_MINUS_DST_ALPHA"},

    /*BlendingFactorSrc*/                                  /*BlendingFactorSrc*/
    /*GL_ZERO*/                                            /*GL_ZERO*/
    /*GL_ONE*/                                             /*GL_ONE*/
    {GL_DST_COLOR,                                         "GL_DST_COLOR"},
    {GL_ONE_MINUS_DST_COLOR,                               "GL_ONE_MINUS_DST_COLOR"},
    {GL_SRC_ALPHA_SATURATE,                                "GL_SRC_ALPHA_SATURATE"},



    /*ClipPlaneName*/                                      /*ClipPlaneName*/
    {GL_CLIP_PLANE0,                                       "GL_CLIP_PLANE0"},
    {GL_CLIP_PLANE1,                                       "GL_CLIP_PLANE1"},
    {GL_CLIP_PLANE2,                                       "GL_CLIP_PLANE2"},
    {GL_CLIP_PLANE3,                                       "GL_CLIP_PLANE3"},
    {GL_CLIP_PLANE4,                                       "GL_CLIP_PLANE4"},
    {GL_CLIP_PLANE5,                                       "GL_CLIP_PLANE5"},


    /*DataType*/                                           /*DataType*/
    {GL_BYTE,                                              "GL_BYTE"},
    {GL_UNSIGNED_BYTE,                                     "GL_UNSIGNED_BYTE"},
    {GL_SHORT,                                             "GL_SHORT"},
    {GL_UNSIGNED_SHORT,                                    "GL_UNSIGNED_SHORT"},
    {GL_INT,                                               "GL_INT"},
    {GL_UNSIGNED_INT,                                      "GL_UNSIGNED_INT"},
    {GL_FLOAT,                                             "GL_FLOAT"},
    {GL_2_BYTES,                                           "GL_2_BYTES"},
    {GL_3_BYTES,                                           "GL_3_BYTES"},
    {GL_4_BYTES,                                           "GL_4_BYTES"},
    {GL_DOUBLE,                                            "GL_DOUBLE"},


    /*DrawBufferMode*/                                     /*DrawBufferMode*/
    /*GL_NONE*/                                            /*GL_NONE*/
    {GL_FRONT_LEFT,                                        "GL_FRONT_LEFT"},
    {GL_FRONT_RIGHT,                                       "GL_FRONT_RIGHT"},
    {GL_BACK_LEFT,                                         "GL_BACK_LEFT"},
    {GL_BACK_RIGHT,                                        "GL_BACK_RIGHT"},
    {GL_FRONT,                                             "GL_FRONT"},
    {GL_BACK,                                              "GL_BACK"},
    {GL_LEFT,                                              "GL_LEFT"},
    {GL_RIGHT,                                             "GL_RIGHT"},
    {GL_FRONT_AND_BACK,                                    "GL_FRONT_AND_BACK"},
    {GL_AUX0,                                              "GL_AUX0"},
    {GL_AUX1,                                              "GL_AUX1"},
    {GL_AUX2,                                              "GL_AUX2"},
    {GL_AUX3,                                              "GL_AUX3"},


    /*ErrorCode*/                                          /*ErrorCode*/
    /*GL_NO_ERROR*/                                        /*GL_NO_ERROR*/
    {GL_INVALID_ENUM,                                      "GL_INVALID_ENUM"},
    {GL_INVALID_VALUE,                                     "GL_INVALID_VALUE"},
    {GL_INVALID_OPERATION,                                 "GL_INVALID_OPERATION"},
    {GL_STACK_OVERFLOW,                                    "GL_STACK_OVERFLOW"},
    {GL_STACK_UNDERFLOW,                                   "GL_STACK_UNDERFLOW"},
    {GL_OUT_OF_MEMORY,                                     "GL_OUT_OF_MEMORY"},

    /*FeedBackMode*/                                       /*FeedBackMode*/
    {GL_2D,                                                "GL_2D"},
    {GL_3D,                                                "GL_3D"},
    {GL_3D_COLOR,                                          "GL_3D_COLOR"},
    {GL_3D_COLOR_TEXTURE,                                  "GL_3D_COLOR_TEXTURE"},
    {GL_4D_COLOR_TEXTURE,                                  "GL_4D_COLOR_TEXTURE"},

    /*FeedBackToken*/                                      /*FeedBackToken*/
    {GL_PASS_THROUGH_TOKEN,                                "GL_PASS_THROUGH_TOKEN"},
    {GL_POINT_TOKEN,                                       "GL_POINT_TOKEN"},
    {GL_LINE_TOKEN,                                        "GL_LINE_TOKEN"},
    {GL_POLYGON_TOKEN,                                     "GL_POLYGON_TOKEN"},
    {GL_BITMAP_TOKEN,                                      "GL_BITMAP_TOKEN"},
    {GL_DRAW_PIXEL_TOKEN,                                  "GL_DRAW_PIXEL_TOKEN"},
    {GL_COPY_PIXEL_TOKEN,                                  "GL_COPY_PIXEL_TOKEN"},
    {GL_LINE_RESET_TOKEN,                                  "GL_LINE_RESET_TOKEN"},

    /*FogMode*/                                            /*FogMode*/
    {GL_EXP,                                               "GL_EXP"},
    {GL_EXP2,                                              "GL_EXP2"},


    /*FrontFaceDirection*/                                 /*FrontFaceDirection*/
    {GL_CW,                                                "GL_CW"},
    {GL_CCW,                                               "GL_CCW"},


    /*GetMapTarget*/                                       /*GetMapTarget*/
    {GL_COEFF,                                             "GL_COEFF"},
    {GL_ORDER,                                             "GL_ORDER"},
    {GL_DOMAIN,                                            "GL_DOMAIN"},

    /*GetTarget*/                                          /*GetTarget*/
    {GL_CURRENT_COLOR,                                     "GL_CURRENT_COLOR"},
    {GL_CURRENT_INDEX,                                     "GL_CURRENT_INDEX"},
    {GL_CURRENT_NORMAL,                                    "GL_CURRENT_NORMAL"},
    {GL_CURRENT_TEXTURE_COORDS,                            "GL_CURRENT_TEXTURE_COORDS"},
    {GL_CURRENT_RASTER_COLOR,                              "GL_CURRENT_RASTER_COLOR"},
    {GL_CURRENT_RASTER_INDEX,                              "GL_CURRENT_RASTER_INDEX"},
    {GL_CURRENT_RASTER_TEXTURE_COORDS,                     "GL_CURRENT_RASTER_TEXTURE_COORDS"},
    {GL_CURRENT_RASTER_POSITION,                           "GL_CURRENT_RASTER_POSITION"},
    {GL_CURRENT_RASTER_POSITION_VALID,                     "GL_CURRENT_RASTER_POSITION_VALID"},
    {GL_CURRENT_RASTER_DISTANCE,                           "GL_CURRENT_RASTER_DISTANCE"},
    {GL_POINT_SMOOTH,                                      "GL_POINT_SMOOTH"},
    {GL_POINT_SIZE,                                        "GL_POINT_SIZE"},
    {GL_POINT_SIZE_RANGE,                                  "GL_POINT_SIZE_RANGE"},
    {GL_POINT_SIZE_GRANULARITY,                            "GL_POINT_SIZE_GRANULARITY"},
    {GL_LINE_SMOOTH,                                       "GL_LINE_SMOOTH"},
    {GL_LINE_WIDTH,                                        "GL_LINE_WIDTH"},
    {GL_LINE_WIDTH_RANGE,                                  "GL_LINE_WIDTH_RANGE"},
    {GL_LINE_WIDTH_GRANULARITY,                            "GL_LINE_WIDTH_GRANULARITY"},
    {GL_LINE_STIPPLE,                                      "GL_LINE_STIPPLE"},
    {GL_LINE_STIPPLE_PATTERN,                              "GL_LINE_STIPPLE_PATTERN"},
    {GL_LINE_STIPPLE_REPEAT,                               "GL_LINE_STIPPLE_REPEAT"},

    {GL_LIST_MODE,                                         "GL_LIST_MODE"},
    {GL_MAX_LIST_NESTING,                                  "GL_MAX_LIST_NESTING"},
    {GL_LIST_BASE,                                         "GL_LIST_BASE"},
    {GL_LIST_INDEX,                                        "GL_LIST_INDEX"},
    {GL_POLYGON_MODE,                                      "GL_POLYGON_MODE"},
    {GL_POLYGON_SMOOTH,                                    "GL_POLYGON_SMOOTH"},
    {GL_POLYGON_STIPPLE,                                   "GL_POLYGON_STIPPLE"},
    {GL_EDGE_FLAG,                                         "GL_EDGE_FLAG"},
    {GL_CULL_FACE,                                         "GL_CULL_FACE"},
    {GL_CULL_FACE_MODE,                                    "GL_CULL_FACE_MODE"},
    {GL_FRONT_FACE,                                        "GL_FRONT_FACE"},
    {GL_LIGHTING,                                          "GL_LIGHTING"},
    {GL_LIGHT_MODEL_LOCAL_VIEWER,                          "GL_LIGHT_MODEL_LOCAL_VIEWER"},
    {GL_LIGHT_MODEL_TWO_SIDE,                              "GL_LIGHT_MODEL_TWO_SIDE"},
    {GL_LIGHT_MODEL_AMBIENT,                               "GL_LIGHT_MODEL_AMBIENT"},
    {GL_SHADE_MODEL,                                       "GL_SHADE_MODEL"},
    {GL_COLOR_MATERIAL_FACE,                               "GL_COLOR_MATERIAL_FACE"},
    {GL_COLOR_MATERIAL_PARAMETER,                          "GL_COLOR_MATERIAL_PARAMETER"},
    {GL_COLOR_MATERIAL,                                    "GL_COLOR_MATERIAL"},
    {GL_FOG,                                               "GL_FOG"},
    {GL_FOG_INDEX,                                         "GL_FOG_INDEX"},
    {GL_FOG_DENSITY,                                       "GL_FOG_DENSITY"},
    {GL_FOG_START,                                         "GL_FOG_START"},
    {GL_FOG_END,                                           "GL_FOG_END"},
    {GL_FOG_MODE,                                          "GL_FOG_MODE"},
    {GL_FOG_COLOR,                                         "GL_FOG_COLOR"},
    {GL_DEPTH_RANGE,                                       "GL_DEPTH_RANGE"},
    {GL_DEPTH_TEST,                                        "GL_DEPTH_TEST"},
    {GL_DEPTH_WRITEMASK,                                   "GL_DEPTH_WRITEMASK"},
    {GL_DEPTH_CLEAR_VALUE,                                 "GL_DEPTH_CLEAR_VALUE"},
    {GL_DEPTH_FUNC,                                        "GL_DEPTH_FUNC"},
    {GL_ACCUM_CLEAR_VALUE,                                 "GL_ACCUM_CLEAR_VALUE"},
    {GL_STENCIL_TEST,                                      "GL_STENCIL_TEST"},
    {GL_STENCIL_CLEAR_VALUE,                               "GL_STENCIL_CLEAR_VALUE"},
    {GL_STENCIL_FUNC,                                      "GL_STENCIL_FUNC"},
    {GL_STENCIL_VALUE_MASK,                                "GL_STENCIL_VALUE_MASK"},
    {GL_STENCIL_FAIL,                                      "GL_STENCIL_FAIL"},
    {GL_STENCIL_PASS_DEPTH_FAIL,                           "GL_STENCIL_PASS_DEPTH_FAIL"},
    {GL_STENCIL_PASS_DEPTH_PASS,                           "GL_STENCIL_PASS_DEPTH_PASS"},
    {GL_STENCIL_REF,                                       "GL_STENCIL_REF"},
    {GL_STENCIL_WRITEMASK,                                 "GL_STENCIL_WRITEMASK"},
    {GL_MATRIX_MODE,                                       "GL_MATRIX_MODE"},
    {GL_NORMALIZE,                                         "GL_NORMALIZE"},
    {GL_VIEWPORT,                                          "GL_VIEWPORT"},
    {GL_MODELVIEW_STACK_DEPTH,                             "GL_MODELVIEW_STACK_DEPTH"},
    {GL_PROJECTION_STACK_DEPTH,                            "GL_PROJECTION_STACK_DEPTH"},
    {GL_TEXTURE_STACK_DEPTH,                               "GL_TEXTURE_STACK_DEPTH"},
    {GL_MODELVIEW_MATRIX,                                  "GL_MODELVIEW_MATRIX"},
    {GL_PROJECTION_MATRIX,                                 "GL_PROJECTION_MATRIX"},
    {GL_TEXTURE_MATRIX,                                    "GL_TEXTURE_MATRIX"},
    {GL_ATTRIB_STACK_DEPTH,                                "GL_ATTRIB_STACK_DEPTH"},
    {GL_CLIENT_ATTRIB_STACK_DEPTH,                         "GL_CLIENT_ATTRIB_STACK_DEPTH"},
    {GL_ALPHA_TEST,                                        "GL_ALPHA_TEST"},
    {GL_ALPHA_TEST_FUNC,                                   "GL_ALPHA_TEST_FUNC"},
    {GL_ALPHA_TEST_REF,                                    "GL_ALPHA_TEST_REF"},
    {GL_DITHER,                                            "GL_DITHER"},
    {GL_BLEND_DST,                                         "GL_BLEND_DST"},
    {GL_BLEND_SRC,                                         "GL_BLEND_SRC"},
    {GL_BLEND,                                             "GL_BLEND"},
    {GL_LOGIC_OP_MODE,                                     "GL_LOGIC_OP_MODE"},
    {GL_INDEX_LOGIC_OP,                                    "GL_INDEX_LOGIC_OP"},
    {GL_COLOR_LOGIC_OP,                                    "GL_COLOR_LOGIC_OP"},
    {GL_AUX_BUFFERS,                                       "GL_AUX_BUFFERS"},
    {GL_DRAW_BUFFER,                                       "GL_DRAW_BUFFER"},
    {GL_READ_BUFFER,                                       "GL_READ_BUFFER"},
    {GL_SCISSOR_BOX,                                       "GL_SCISSOR_BOX"},
    {GL_SCISSOR_TEST,                                      "GL_SCISSOR_TEST"},
    {GL_INDEX_CLEAR_VALUE,                                 "GL_INDEX_CLEAR_VALUE"},
    {GL_INDEX_WRITEMASK,                                   "GL_INDEX_WRITEMASK"},
    {GL_COLOR_CLEAR_VALUE,                                 "GL_COLOR_CLEAR_VALUE"},
    {GL_COLOR_WRITEMASK,                                   "GL_COLOR_WRITEMASK"},
    {GL_INDEX_MODE,                                        "GL_INDEX_MODE"},
    {GL_RGBA_MODE,                                         "GL_RGBA_MODE"},
    {GL_DOUBLEBUFFER,                                      "GL_DOUBLEBUFFER"},
    {GL_STEREO,                                            "GL_STEREO"},
    {GL_RENDER_MODE,                                       "GL_RENDER_MODE"},
    {GL_PERSPECTIVE_CORRECTION_HINT,                       "GL_PERSPECTIVE_CORRECTION_HINT"},
    {GL_POINT_SMOOTH_HINT,                                 "GL_POINT_SMOOTH_HINT"},
    {GL_LINE_SMOOTH_HINT,                                  "GL_LINE_SMOOTH_HINT"},
    {GL_POLYGON_SMOOTH_HINT,                               "GL_POLYGON_SMOOTH_HINT"},
    {GL_FOG_HINT,                                          "GL_FOG_HINT"},
    {GL_TEXTURE_GEN_S,                                     "GL_TEXTURE_GEN_S"},
    {GL_TEXTURE_GEN_T,                                     "GL_TEXTURE_GEN_T"},
    {GL_TEXTURE_GEN_R,                                     "GL_TEXTURE_GEN_R"},
    {GL_TEXTURE_GEN_Q,                                     "GL_TEXTURE_GEN_Q"},
    {GL_PIXEL_MAP_I_TO_I,                                  "GL_PIXEL_MAP_I_TO_I"},
    {GL_PIXEL_MAP_S_TO_S,                                  "GL_PIXEL_MAP_S_TO_S"},
    {GL_PIXEL_MAP_I_TO_R,                                  "GL_PIXEL_MAP_I_TO_R"},
    {GL_PIXEL_MAP_I_TO_G,                                  "GL_PIXEL_MAP_I_TO_G"},
    {GL_PIXEL_MAP_I_TO_B,                                  "GL_PIXEL_MAP_I_TO_B"},
    {GL_PIXEL_MAP_I_TO_A,                                  "GL_PIXEL_MAP_I_TO_A"},
    {GL_PIXEL_MAP_R_TO_R,                                  "GL_PIXEL_MAP_R_TO_R"},
    {GL_PIXEL_MAP_G_TO_G,                                  "GL_PIXEL_MAP_G_TO_G"},
    {GL_PIXEL_MAP_B_TO_B,                                  "GL_PIXEL_MAP_B_TO_B"},
    {GL_PIXEL_MAP_A_TO_A,                                  "GL_PIXEL_MAP_A_TO_A"},
    {GL_PIXEL_MAP_I_TO_I_SIZE,                             "GL_PIXEL_MAP_I_TO_I_SIZE"},
    {GL_PIXEL_MAP_S_TO_S_SIZE,                             "GL_PIXEL_MAP_S_TO_S_SIZE"},
    {GL_PIXEL_MAP_I_TO_R_SIZE,                             "GL_PIXEL_MAP_I_TO_R_SIZE"},
    {GL_PIXEL_MAP_I_TO_G_SIZE,                             "GL_PIXEL_MAP_I_TO_G_SIZE"},
    {GL_PIXEL_MAP_I_TO_B_SIZE,                             "GL_PIXEL_MAP_I_TO_B_SIZE"},
    {GL_PIXEL_MAP_I_TO_A_SIZE,                             "GL_PIXEL_MAP_I_TO_A_SIZE"},
    {GL_PIXEL_MAP_R_TO_R_SIZE,                             "GL_PIXEL_MAP_R_TO_R_SIZE"},
    {GL_PIXEL_MAP_G_TO_G_SIZE,                             "GL_PIXEL_MAP_G_TO_G_SIZE"},
    {GL_PIXEL_MAP_B_TO_B_SIZE,                             "GL_PIXEL_MAP_B_TO_B_SIZE"},
    {GL_PIXEL_MAP_A_TO_A_SIZE,                             "GL_PIXEL_MAP_A_TO_A_SIZE"},
    {GL_UNPACK_SWAP_BYTES,                                 "GL_UNPACK_SWAP_BYTES"},
    {GL_UNPACK_LSB_FIRST,                                  "GL_UNPACK_LSB_FIRST"},
    {GL_UNPACK_ROW_LENGTH,                                 "GL_UNPACK_ROW_LENGTH"},
    {GL_UNPACK_SKIP_ROWS,                                  "GL_UNPACK_SKIP_ROWS"},
    {GL_UNPACK_SKIP_PIXELS,                                "GL_UNPACK_SKIP_PIXELS"},
    {GL_UNPACK_ALIGNMENT,                                  "GL_UNPACK_ALIGNMENT"},
    {GL_PACK_SWAP_BYTES,                                   "GL_PACK_SWAP_BYTES"},
    {GL_PACK_LSB_FIRST,                                    "GL_PACK_LSB_FIRST"},
    {GL_PACK_ROW_LENGTH,                                   "GL_PACK_ROW_LENGTH"},
    {GL_PACK_SKIP_ROWS,                                    "GL_PACK_SKIP_ROWS"},
    {GL_PACK_SKIP_PIXELS,                                  "GL_PACK_SKIP_PIXELS"},
    {GL_PACK_ALIGNMENT,                                    "GL_PACK_ALIGNMENT"},
    {GL_MAP_COLOR,                                         "GL_MAP_COLOR"},
    {GL_MAP_STENCIL,                                       "GL_MAP_STENCIL"},
    {GL_INDEX_SHIFT,                                       "GL_INDEX_SHIFT"},
    {GL_INDEX_OFFSET,                                      "GL_INDEX_OFFSET"},
    {GL_RED_SCALE,                                         "GL_RED_SCALE"},
    {GL_RED_BIAS,                                          "GL_RED_BIAS"},
    {GL_ZOOM_X,                                            "GL_ZOOM_X"},
    {GL_ZOOM_Y,                                            "GL_ZOOM_Y"},
    {GL_GREEN_SCALE,                                       "GL_GREEN_SCALE"},
    {GL_GREEN_BIAS,                                        "GL_GREEN_BIAS"},
    {GL_BLUE_SCALE,                                        "GL_BLUE_SCALE"},
    {GL_BLUE_BIAS,                                         "GL_BLUE_BIAS"},
    {GL_ALPHA_SCALE,                                       "GL_ALPHA_SCALE"},
    {GL_ALPHA_BIAS,                                        "GL_ALPHA_BIAS"},
    {GL_DEPTH_SCALE,                                       "GL_DEPTH_SCALE"},
    {GL_DEPTH_BIAS,                                        "GL_DEPTH_BIAS"},
    {GL_MAX_EVAL_ORDER,                                    "GL_MAX_EVAL_ORDER"},
    {GL_MAX_LIGHTS,                                        "GL_MAX_LIGHTS"},
    {GL_MAX_CLIP_PLANES,                                   "GL_MAX_CLIP_PLANES"},
    {GL_MAX_TEXTURE_SIZE,                                  "GL_MAX_TEXTURE_SIZE"},
    {GL_MAX_PIXEL_MAP_TABLE,                               "GL_MAX_PIXEL_MAP_TABLE"},
    {GL_MAX_ATTRIB_STACK_DEPTH,                            "GL_MAX_ATTRIB_STACK_DEPTH"},
    {GL_MAX_MODELVIEW_STACK_DEPTH,                         "GL_MAX_MODELVIEW_STACK_DEPTH"},
    {GL_MAX_NAME_STACK_DEPTH,                              "GL_MAX_NAME_STACK_DEPTH"},
    {GL_MAX_PROJECTION_STACK_DEPTH,                        "GL_MAX_PROJECTION_STACK_DEPTH"},
    {GL_MAX_TEXTURE_STACK_DEPTH,                           "GL_MAX_TEXTURE_STACK_DEPTH"},
    {GL_MAX_VIEWPORT_DIMS,                                 "GL_MAX_VIEWPORT_DIMS"},
    {GL_MAX_CLIENT_ATTRIB_STACK_DEPTH,                     "GL_MAX_CLIENT_ATTRIB_STACK_DEPTH"},
    {GL_SUBPIXEL_BITS,                                     "GL_SUBPIXEL_BITS"},
    {GL_INDEX_BITS,                                        "GL_INDEX_BITS"},
    {GL_RED_BITS,                                          "GL_RED_BITS"},
    {GL_GREEN_BITS,                                        "GL_GREEN_BITS"},
    {GL_BLUE_BITS,                                         "GL_BLUE_BITS"},
    {GL_ALPHA_BITS,                                        "GL_ALPHA_BITS"},
    {GL_DEPTH_BITS,                                        "GL_DEPTH_BITS"},
    {GL_STENCIL_BITS,                                      "GL_STENCIL_BITS"},
    {GL_ACCUM_RED_BITS,                                    "GL_ACCUM_RED_BITS"},
    {GL_ACCUM_GREEN_BITS,                                  "GL_ACCUM_GREEN_BITS"},
    {GL_ACCUM_BLUE_BITS,                                   "GL_ACCUM_BLUE_BITS"},
    {GL_ACCUM_ALPHA_BITS,                                  "GL_ACCUM_ALPHA_BITS"},
    {GL_NAME_STACK_DEPTH,                                  "GL_NAME_STACK_DEPTH"},
    {GL_AUTO_NORMAL,                                       "GL_AUTO_NORMAL"},
    {GL_MAP1_COLOR_4,                                      "GL_MAP1_COLOR_4"},
    {GL_MAP1_INDEX,                                        "GL_MAP1_INDEX"},
    {GL_MAP1_NORMAL,                                       "GL_MAP1_NORMAL"},
    {GL_MAP1_TEXTURE_COORD_1,                              "GL_MAP1_TEXTURE_COORD_1"},
    {GL_MAP1_TEXTURE_COORD_2,                              "GL_MAP1_TEXTURE_COORD_2"},
    {GL_MAP1_TEXTURE_COORD_3,                              "GL_MAP1_TEXTURE_COORD_3"},
    {GL_MAP1_TEXTURE_COORD_4,                              "GL_MAP1_TEXTURE_COORD_4"},
    {GL_MAP1_VERTEX_3,                                     "GL_MAP1_VERTEX_3"},
    {GL_MAP1_VERTEX_4,                                     "GL_MAP1_VERTEX_4"},
    {GL_MAP2_COLOR_4,                                      "GL_MAP2_COLOR_4"},
    {GL_MAP2_INDEX,                                        "GL_MAP2_INDEX"},
    {GL_MAP2_NORMAL,                                       "GL_MAP2_NORMAL"},
    {GL_MAP2_TEXTURE_COORD_1,                              "GL_MAP2_TEXTURE_COORD_1"},
    {GL_MAP2_TEXTURE_COORD_2,                              "GL_MAP2_TEXTURE_COORD_2"},
    {GL_MAP2_TEXTURE_COORD_3,                              "GL_MAP2_TEXTURE_COORD_3"},
    {GL_MAP2_TEXTURE_COORD_4,                              "GL_MAP2_TEXTURE_COORD_4"},
    {GL_MAP2_VERTEX_3,                                     "GL_MAP2_VERTEX_3"},
    {GL_MAP2_VERTEX_4,                                     "GL_MAP2_VERTEX_4"},
    {GL_MAP1_GRID_DOMAIN,                                  "GL_MAP1_GRID_DOMAIN"},
    {GL_MAP1_GRID_SEGMENTS,                                "GL_MAP1_GRID_SEGMENTS"},
    {GL_MAP2_GRID_DOMAIN,                                  "GL_MAP2_GRID_DOMAIN"},
    {GL_MAP2_GRID_SEGMENTS,                                "GL_MAP2_GRID_SEGMENTS"},
    {GL_TEXTURE_1D,                                        "GL_TEXTURE_1D"},
    {GL_TEXTURE_2D,                                        "GL_TEXTURE_2D"},
    {GL_FEEDBACK_BUFFER_POINTER,                           "GL_FEEDBACK_BUFFER_POINTER"},
    {GL_FEEDBACK_BUFFER_SIZE,                              "GL_FEEDBACK_BUFFER_SIZE"},
    {GL_FEEDBACK_BUFFER_TYPE,                              "GL_FEEDBACK_BUFFER_TYPE"},
    {GL_SELECTION_BUFFER_POINTER,                          "GL_SELECTION_BUFFER_POINTER"},
    {GL_SELECTION_BUFFER_SIZE,                             "GL_SELECTION_BUFFER_SIZE"},

    /*GetTextureParameter*/                                /*GetTextureParameter*/
    {GL_TEXTURE_WIDTH,                                     "GL_TEXTURE_WIDTH"},
    {GL_TEXTURE_HEIGHT,                                    "GL_TEXTURE_HEIGHT"},
    {GL_TEXTURE_INTERNAL_FORMAT,                           "GL_TEXTURE_INTERNAL_FORMAT"},
    {GL_TEXTURE_BORDER_COLOR,                              "GL_TEXTURE_BORDER_COLOR"},
    {GL_TEXTURE_BORDER,                                    "GL_TEXTURE_BORDER"},

    /*HintMode*/                                           /*HintMode*/
    {GL_DONT_CARE,                                         "GL_DONT_CARE"},
    {GL_FASTEST,                                           "GL_FASTEST"},
    {GL_NICEST,                                            "GL_NICEST"},

    /*LightName*/                                          /*LightName*/
    {GL_LIGHT0,                                            "GL_LIGHT0"},
    {GL_LIGHT1,                                            "GL_LIGHT1"},
    {GL_LIGHT2,                                            "GL_LIGHT2"},
    {GL_LIGHT3,                                            "GL_LIGHT3"},
    {GL_LIGHT4,                                            "GL_LIGHT4"},
    {GL_LIGHT5,                                            "GL_LIGHT5"},
    {GL_LIGHT6,                                            "GL_LIGHT6"},
    {GL_LIGHT7,                                            "GL_LIGHT7"},

    /*LightParameter*/                                     /*LightParameter*/
    {GL_AMBIENT,                                           "GL_AMBIENT"},
    {GL_DIFFUSE,                                           "GL_DIFFUSE"},
    {GL_SPECULAR,                                          "GL_SPECULAR"},
    {GL_POSITION,                                          "GL_POSITION"},
    {GL_SPOT_DIRECTION,                                    "GL_SPOT_DIRECTION"},
    {GL_SPOT_EXPONENT,                                     "GL_SPOT_EXPONENT"},
    {GL_SPOT_CUTOFF,                                       "GL_SPOT_CUTOFF"},
    {GL_CONSTANT_ATTENUATION,                              "GL_CONSTANT_ATTENUATION"},
    {GL_LINEAR_ATTENUATION,                                "GL_LINEAR_ATTENUATION"},
    {GL_QUADRATIC_ATTENUATION,                             "GL_QUADRATIC_ATTENUATION"},

    /*InterleavedArrays*/                                  /*InterleavedArrays*/

    /*ListMode*/                                           /*ListMode*/
    {GL_COMPILE,                                           "GL_COMPILE"},
    {GL_COMPILE_AND_EXECUTE,                               "GL_COMPILE_AND_EXECUTE"},

    /*LogicOp*/                                            /*LogicOp*/
    {GL_CLEAR,                                             "GL_CLEAR"},
    {GL_AND,                                               "GL_AND"},
    {GL_AND_REVERSE,                                       "GL_AND_REVERSE"},
    {GL_COPY,                                              "GL_COPY"},
    {GL_AND_INVERTED,                                      "GL_AND_INVERTED"},
    {GL_NOOP,                                              "GL_NOOP"},
    {GL_XOR,                                               "GL_XOR"},
    {GL_OR,                                                "GL_OR"},
    {GL_NOR,                                               "GL_NOR"},
    {GL_EQUIV,                                             "GL_EQUIV"},
    {GL_INVERT,                                            "GL_INVERT"},
    {GL_OR_REVERSE,                                        "GL_OR_REVERSE"},
    {GL_COPY_INVERTED,                                     "GL_COPY_INVERTED"},
    {GL_OR_INVERTED,                                       "GL_OR_INVERTED"},
    {GL_NAND,                                              "GL_NAND"},
    {GL_SET,                                               "GL_SET"},


    /*MaterialFace*/                                       /*MaterialFace*/

    /*MaterialParameter*/                                  /*MaterialParameter*/
    {GL_EMISSION,                                          "GL_EMISSION"},
    {GL_SHININESS,                                         "GL_SHININESS"},
    {GL_AMBIENT_AND_DIFFUSE,                               "GL_AMBIENT_AND_DIFFUSE"},
    {GL_COLOR_INDEXES,                                     "GL_COLOR_INDEXES"},

    /*MatrixMode*/                                         /*MatrixMode*/
    {GL_MODELVIEW,                                         "GL_MODELVIEW"},
    {GL_PROJECTION,                                        "GL_PROJECTION"},
    {GL_TEXTURE,                                           "GL_TEXTURE"},

    /*PixelCopyType*/                                      /*PixelCopyType*/
    {GL_COLOR,                                             "GL_COLOR"},
    {GL_DEPTH,                                             "GL_DEPTH"},
    {GL_STENCIL,                                           "GL_STENCIL"},

    /*PixelFormat*/                                        /*PixelFormat*/
    {GL_COLOR_INDEX,                                       "GL_COLOR_INDEX"},
    {GL_STENCIL_INDEX,                                     "GL_STENCIL_INDEX"},
    {GL_DEPTH_COMPONENT,                                   "GL_DEPTH_COMPONENT"},
    {GL_RED,                                               "GL_RED"},
    {GL_GREEN,                                             "GL_GREEN"},
    {GL_BLUE,                                              "GL_BLUE"},
    {GL_ALPHA,                                             "GL_ALPHA"},
    {GL_RGB,                                               "GL_RGB"},
    {GL_RGBA,                                              "GL_RGBA"},
    {GL_LUMINANCE,                                         "GL_LUMINANCE"},
    {GL_LUMINANCE_ALPHA,                                   "GL_LUMINANCE_ALPHA"},


    /*PixelType*/                                          /*PixelType*/
    {GL_BITMAP,                                            "GL_BITMAP"},

    /*PolygonMode*/                                        /*PolygonMode*/
    {GL_POINT,                                             "GL_POINT"},
    {GL_LINE,                                              "GL_LINE"},
    {GL_FILL,                                              "GL_FILL"},


    /*RenderingMode*/                                      /*RenderingMode*/
    {GL_RENDER,                                            "GL_RENDER"},
    {GL_FEEDBACK,                                          "GL_FEEDBACK"},
    {GL_SELECT,                                            "GL_SELECT"},

    /*ShadingModel*/                                       /*ShadingModel*/
    {GL_FLAT,                                              "GL_FLAT"},
    {GL_SMOOTH,                                            "GL_SMOOTH"},


    /*StencilOp*/                                          /*StencilOp*/
    {GL_KEEP,                                              "GL_KEEP"},
    {GL_REPLACE,                                           "GL_REPLACE"},
    {GL_INCR,                                              "GL_INCR"},
    {GL_DECR,                                              "GL_DECR"},

    /*StringName*/                                         /*StringName*/
    {GL_VENDOR,                                            "GL_VENDOR"},
    {GL_RENDERER,                                          "GL_RENDERER"},
    {GL_VERSION,                                           "GL_VERSION"},
    {GL_EXTENSIONS,                                        "GL_EXTENSIONS"},

    /*TextureCoordName*/                                   /*TextureCoordName*/
    {GL_S,                                                 "GL_S"},
    {GL_T,                                                 "GL_T"},
    {GL_R,                                                 "GL_R"},
    {GL_Q,                                                 "GL_Q"},

    /*TextureEnvMode*/                                     /*TextureEnvMode*/
    {GL_MODULATE,                                          "GL_MODULATE"},
    {GL_DECAL,                                             "GL_DECAL"},

    /*TextureEnvParameter*/                                /*TextureEnvParameter*/
    {GL_TEXTURE_ENV_MODE,                                  "GL_TEXTURE_ENV_MODE"},
    {GL_TEXTURE_ENV_COLOR,                                 "GL_TEXTURE_ENV_COLOR"},

    /*TextureEnvTarget*/                                   /*TextureEnvTarget*/
    {GL_TEXTURE_ENV,                                       "GL_TEXTURE_ENV"},

    /*TextureGenMode*/                                     /*TextureGenMode*/
    {GL_EYE_LINEAR,                                        "GL_EYE_LINEAR"},
    {GL_OBJECT_LINEAR,                                     "GL_OBJECT_LINEAR"},
    {GL_SPHERE_MAP,                                        "GL_SPHERE_MAP"},

    /*TextureGenParameter*/                                /*TextureGenParameter*/
    {GL_TEXTURE_GEN_MODE,                                  "GL_TEXTURE_GEN_MODE"},
    {GL_OBJECT_PLANE,                                      "GL_OBJECT_PLANE"},
    {GL_EYE_PLANE,                                         "GL_EYE_PLANE"},

    /*TextureMagFilter*/                                   /*TextureMagFilter*/
    {GL_NEAREST,                                           "GL_NEAREST"},
    {GL_LINEAR,                                            "GL_LINEAR"},

    /*TextureMinFilter*/                                   /*TextureMinFilter*/
    {GL_NEAREST_MIPMAP_NEAREST,                            "GL_NEAREST_MIPMAP_NEAREST"},
    {GL_LINEAR_MIPMAP_NEAREST,                             "GL_LINEAR_MIPMAP_NEAREST"},
    {GL_NEAREST_MIPMAP_LINEAR,                             "GL_NEAREST_MIPMAP_LINEAR"},
    {GL_LINEAR_MIPMAP_LINEAR,                              "GL_LINEAR_MIPMAP_LINEAR"},

    /*TextureParameterName*/                               /*TextureParameterName*/
    {GL_TEXTURE_MAG_FILTER,                                "GL_TEXTURE_MAG_FILTER"},
    {GL_TEXTURE_MIN_FILTER,                                "GL_TEXTURE_MIN_FILTER"},
    {GL_TEXTURE_WRAP_S,                                    "GL_TEXTURE_WRAP_S"},
    {GL_TEXTURE_WRAP_T,                                    "GL_TEXTURE_WRAP_T"},

    /*TextureWrapMode*/                                    /*TextureWrapMode*/
    {GL_CLAMP,                                             "GL_CLAMP"},
    {GL_REPEAT,                                            "GL_REPEAT"},


    /*ClientAttribMask*/                                   /*ClientAttribMask*/




    /*polygon_offset*/                                     /*polygon_offset*/
    {GL_POLYGON_OFFSET_FACTOR,                             "GL_POLYGON_OFFSET_FACTOR"},
    {GL_POLYGON_OFFSET_UNITS,                              "GL_POLYGON_OFFSET_UNITS"},
    {GL_POLYGON_OFFSET_POINT,                              "GL_POLYGON_OFFSET_POINT"},
    {GL_POLYGON_OFFSET_LINE,                               "GL_POLYGON_OFFSET_LINE"},
    {GL_POLYGON_OFFSET_FILL,                               "GL_POLYGON_OFFSET_FILL"},

    /*texture*/                                            /*texture*/
    {GL_ALPHA4,                                            "GL_ALPHA4"},
    {GL_ALPHA8,                                            "GL_ALPHA8"},
    {GL_ALPHA12,                                           "GL_ALPHA12"},
    {GL_ALPHA16,                                           "GL_ALPHA16"},
    {GL_LUMINANCE4,                                        "GL_LUMINANCE4"},
    {GL_LUMINANCE8,                                        "GL_LUMINANCE8"},
    {GL_LUMINANCE12,                                       "GL_LUMINANCE12"},
    {GL_LUMINANCE16,                                       "GL_LUMINANCE16"},
    {GL_LUMINANCE4_ALPHA4,                                 "GL_LUMINANCE4_ALPHA4"},
    {GL_LUMINANCE6_ALPHA2,                                 "GL_LUMINANCE6_ALPHA2"},
    {GL_LUMINANCE8_ALPHA8,                                 "GL_LUMINANCE8_ALPHA8"},
    {GL_LUMINANCE12_ALPHA4,                                "GL_LUMINANCE12_ALPHA4"},
    {GL_LUMINANCE12_ALPHA12,                               "GL_LUMINANCE12_ALPHA12"},
    {GL_LUMINANCE16_ALPHA16,                               "GL_LUMINANCE16_ALPHA16"},
    {GL_INTENSITY,                                         "GL_INTENSITY"},
    {GL_INTENSITY4,                                        "GL_INTENSITY4"},
    {GL_INTENSITY8,                                        "GL_INTENSITY8"},
    {GL_INTENSITY12,                                       "GL_INTENSITY12"},
    {GL_INTENSITY16,                                       "GL_INTENSITY16"},
    {GL_R3_G3_B2,                                          "GL_R3_G3_B2"},
    {GL_RGB4,                                              "GL_RGB4"},
    {GL_RGB5,                                              "GL_RGB5"},
    {GL_RGB8,                                              "GL_RGB8"},
    {GL_RGB10,                                             "GL_RGB10"},
    {GL_RGB12,                                             "GL_RGB12"},
    {GL_RGB16,                                             "GL_RGB16"},
    {GL_RGBA2,                                             "GL_RGBA2"},
    {GL_RGBA4,                                             "GL_RGBA4"},
    {GL_RGB5_A1,                                           "GL_RGB5_A1"},
    {GL_RGBA8,                                             "GL_RGBA8"},
    {GL_RGB10_A2,                                          "GL_RGB10_A2"},
    {GL_RGBA12,                                            "GL_RGBA12"},
    {GL_RGBA16,                                            "GL_RGBA16"},
    {GL_TEXTURE_RED_SIZE,                                  "GL_TEXTURE_RED_SIZE"},
    {GL_TEXTURE_GREEN_SIZE,                                "GL_TEXTURE_GREEN_SIZE"},
    {GL_TEXTURE_BLUE_SIZE,                                 "GL_TEXTURE_BLUE_SIZE"},
    {GL_TEXTURE_ALPHA_SIZE,                                "GL_TEXTURE_ALPHA_SIZE"},
    {GL_TEXTURE_LUMINANCE_SIZE,                            "GL_TEXTURE_LUMINANCE_SIZE"},
    {GL_TEXTURE_INTENSITY_SIZE,                            "GL_TEXTURE_INTENSITY_SIZE"},
    {GL_PROXY_TEXTURE_1D,                                  "GL_PROXY_TEXTURE_1D"},
    {GL_PROXY_TEXTURE_2D,                                  "GL_PROXY_TEXTURE_2D"},

    /*texture_object*/                                     /*texture_object*/
    {GL_TEXTURE_PRIORITY,                                  "GL_TEXTURE_PRIORITY"},
    {GL_TEXTURE_RESIDENT,                                  "GL_TEXTURE_RESIDENT"},
    {GL_TEXTURE_BINDING_1D,                                "GL_TEXTURE_BINDING_1D"},
    {GL_TEXTURE_BINDING_2D,                                "GL_TEXTURE_BINDING_2D"},
    {GL_TEXTURE_BINDING_3D,                                "GL_TEXTURE_BINDING_3D"},

    /*vertex_array*/                                       /*vertex_array*/
    {GL_VERTEX_ARRAY,                                      "GL_VERTEX_ARRAY"},
    {GL_NORMAL_ARRAY,                                      "GL_NORMAL_ARRAY"},
    {GL_COLOR_ARRAY,                                       "GL_COLOR_ARRAY"},
    {GL_INDEX_ARRAY,                                       "GL_INDEX_ARRAY"},
    {GL_TEXTURE_COORD_ARRAY,                               "GL_TEXTURE_COORD_ARRAY"},
    {GL_EDGE_FLAG_ARRAY,                                   "GL_EDGE_FLAG_ARRAY"},
    {GL_VERTEX_ARRAY_SIZE,                                 "GL_VERTEX_ARRAY_SIZE"},
    {GL_VERTEX_ARRAY_TYPE,                                 "GL_VERTEX_ARRAY_TYPE"},
    {GL_VERTEX_ARRAY_STRIDE,                               "GL_VERTEX_ARRAY_STRIDE"},
    {GL_NORMAL_ARRAY_TYPE,                                 "GL_NORMAL_ARRAY_TYPE"},
    {GL_NORMAL_ARRAY_STRIDE,                               "GL_NORMAL_ARRAY_STRIDE"},
    {GL_COLOR_ARRAY_SIZE,                                  "GL_COLOR_ARRAY_SIZE"},
    {GL_COLOR_ARRAY_TYPE,                                  "GL_COLOR_ARRAY_TYPE"},
    {GL_COLOR_ARRAY_STRIDE,                                "GL_COLOR_ARRAY_STRIDE"},
    {GL_INDEX_ARRAY_TYPE,                                  "GL_INDEX_ARRAY_TYPE"},
    {GL_INDEX_ARRAY_STRIDE,                                "GL_INDEX_ARRAY_STRIDE"},
    {GL_TEXTURE_COORD_ARRAY_SIZE,                          "GL_TEXTURE_COORD_ARRAY_SIZE"},
    {GL_TEXTURE_COORD_ARRAY_TYPE,                          "GL_TEXTURE_COORD_ARRAY_TYPE"},
    {GL_TEXTURE_COORD_ARRAY_STRIDE,                        "GL_TEXTURE_COORD_ARRAY_STRIDE"},
    {GL_EDGE_FLAG_ARRAY_STRIDE,                            "GL_EDGE_FLAG_ARRAY_STRIDE"},
    {GL_VERTEX_ARRAY_POINTER,                              "GL_VERTEX_ARRAY_POINTER"},
    {GL_NORMAL_ARRAY_POINTER,                              "GL_NORMAL_ARRAY_POINTER"},
    {GL_COLOR_ARRAY_POINTER,                               "GL_COLOR_ARRAY_POINTER"},
    {GL_INDEX_ARRAY_POINTER,                               "GL_INDEX_ARRAY_POINTER"},
    {GL_TEXTURE_COORD_ARRAY_POINTER,                       "GL_TEXTURE_COORD_ARRAY_POINTER"},
    {GL_EDGE_FLAG_ARRAY_POINTER,                           "GL_EDGE_FLAG_ARRAY_POINTER"},
    {GL_V2F,                                               "GL_V2F"},
    {GL_V3F,                                               "GL_V3F"},
    {GL_C4UB_V2F,                                          "GL_C4UB_V2F"},
    {GL_C4UB_V3F,                                          "GL_C4UB_V3F"},
    {GL_C3F_V3F,                                           "GL_C3F_V3F"},
    {GL_N3F_V3F,                                           "GL_N3F_V3F"},
    {GL_C4F_N3F_V3F,                                       "GL_C4F_N3F_V3F"},
    {GL_T2F_V3F,                                           "GL_T2F_V3F"},
    {GL_T4F_V4F,                                           "GL_T4F_V4F"},
    {GL_T2F_C4UB_V3F,                                      "GL_T2F_C4UB_V3F"},
    {GL_T2F_C3F_V3F,                                       "GL_T2F_C3F_V3F"},
    {GL_T2F_N3F_V3F,                                       "GL_T2F_N3F_V3F"},
    {GL_T2F_C4F_N3F_V3F,                                   "GL_T2F_C4F_N3F_V3F"},
    {GL_T4F_C4F_N3F_V4F,                                   "GL_T4F_C4F_N3F_V4F"},

    /*bgra*/                                               /*bgra*/
    {GL_BGR,                                               "GL_BGR"},
    {GL_BGRA,                                              "GL_BGRA"},

    /*blend_color*/                                        /*blend_color*/
    {GL_CONSTANT_COLOR,                                    "GL_CONSTANT_COLOR"},
    {GL_ONE_MINUS_CONSTANT_COLOR,                          "GL_ONE_MINUS_CONSTANT_COLOR"},
    {GL_CONSTANT_ALPHA,                                    "GL_CONSTANT_ALPHA"},
    {GL_ONE_MINUS_CONSTANT_ALPHA,                          "GL_ONE_MINUS_CONSTANT_ALPHA"},
    {GL_BLEND_COLOR,                                       "GL_BLEND_COLOR"},

    /*blend_minmax*/                                       /*blend_minmax*/
    {GL_FUNC_ADD,                                          "GL_FUNC_ADD"},
    {GL_MIN,                                               "GL_MIN"},
    {GL_MAX,                                               "GL_MAX"},
    {GL_BLEND_EQUATION,                                    "GL_BLEND_EQUATION"},

    /*blend_subtract*/                                     /*blend_subtract*/
    {GL_FUNC_SUBTRACT,                                     "GL_FUNC_SUBTRACT"},
    {GL_FUNC_REVERSE_SUBTRACT,                             "GL_FUNC_REVERSE_SUBTRACT"},

    /*color_matrix*/                                       /*color_matrix*/
    {GL_COLOR_MATRIX,                                      "GL_COLOR_MATRIX"},
    {GL_COLOR_MATRIX_STACK_DEPTH,                          "GL_COLOR_MATRIX_STACK_DEPTH"},
    {GL_MAX_COLOR_MATRIX_STACK_DEPTH,                      "GL_MAX_COLOR_MATRIX_STACK_DEPTH"},
    {GL_POST_COLOR_MATRIX_RED_SCALE,                       "GL_POST_COLOR_MATRIX_RED_SCALE"},
    {GL_POST_COLOR_MATRIX_GREEN_SCALE,                     "GL_POST_COLOR_MATRIX_GREEN_SCALE"},
    {GL_POST_COLOR_MATRIX_BLUE_SCALE,                      "GL_POST_COLOR_MATRIX_BLUE_SCALE"},
    {GL_POST_COLOR_MATRIX_ALPHA_SCALE,                     "GL_POST_COLOR_MATRIX_ALPHA_SCALE"},
    {GL_POST_COLOR_MATRIX_RED_BIAS,                        "GL_POST_COLOR_MATRIX_RED_BIAS"},
    {GL_POST_COLOR_MATRIX_GREEN_BIAS,                      "GL_POST_COLOR_MATRIX_GREEN_BIAS"},
    {GL_POST_COLOR_MATRIX_BLUE_BIAS,                       "GL_POST_COLOR_MATRIX_BLUE_BIAS"},
    {GL_POST_COLOR_MATRIX_ALPHA_BIAS,                      "GL_POST_COLOR_MATRIX_ALPHA_BIAS"},

    /*color_table*/                                        /*color_table*/
    {GL_COLOR_TABLE,                                       "GL_COLOR_TABLE"},
    {GL_POST_CONVOLUTION_COLOR_TABLE,                      "GL_POST_CONVOLUTION_COLOR_TABLE"},
    {GL_POST_COLOR_MATRIX_COLOR_TABLE,                     "GL_POST_COLOR_MATRIX_COLOR_TABLE"},
    {GL_PROXY_COLOR_TABLE,                                 "GL_PROXY_COLOR_TABLE"},
    {GL_PROXY_POST_CONVOLUTION_COLOR_TABLE,                "GL_PROXY_POST_CONVOLUTION_COLOR_TABLE"},
    {GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE,               "GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE"},
    {GL_COLOR_TABLE_SCALE,                                 "GL_COLOR_TABLE_SCALE"},
    {GL_COLOR_TABLE_BIAS,                                  "GL_COLOR_TABLE_BIAS"},
    {GL_COLOR_TABLE_FORMAT,                                "GL_COLOR_TABLE_FORMAT"},
    {GL_COLOR_TABLE_WIDTH,                                 "GL_COLOR_TABLE_WIDTH"},
    {GL_COLOR_TABLE_RED_SIZE,                              "GL_COLOR_TABLE_RED_SIZE"},
    {GL_COLOR_TABLE_GREEN_SIZE,                            "GL_COLOR_TABLE_GREEN_SIZE"},
    {GL_COLOR_TABLE_BLUE_SIZE,                             "GL_COLOR_TABLE_BLUE_SIZE"},
    {GL_COLOR_TABLE_ALPHA_SIZE,                            "GL_COLOR_TABLE_ALPHA_SIZE"},
    {GL_COLOR_TABLE_LUMINANCE_SIZE,                        "GL_COLOR_TABLE_LUMINANCE_SIZE"},
    {GL_COLOR_TABLE_INTENSITY_SIZE,                        "GL_COLOR_TABLE_INTENSITY_SIZE"},

    /*convolution*/                                        /*convolution*/
    {GL_CONVOLUTION_1D,                                    "GL_CONVOLUTION_1D"},
    {GL_CONVOLUTION_2D,                                    "GL_CONVOLUTION_2D"},
    {GL_SEPARABLE_2D,                                      "GL_SEPARABLE_2D"},
    {GL_CONVOLUTION_BORDER_MODE,                           "GL_CONVOLUTION_BORDER_MODE"},
    {GL_CONVOLUTION_FILTER_SCALE,                          "GL_CONVOLUTION_FILTER_SCALE"},
    {GL_CONVOLUTION_FILTER_BIAS,                           "GL_CONVOLUTION_FILTER_BIAS"},
    {GL_REDUCE,                                            "GL_REDUCE"},
    {GL_CONVOLUTION_FORMAT,                                "GL_CONVOLUTION_FORMAT"},
    {GL_CONVOLUTION_WIDTH,                                 "GL_CONVOLUTION_WIDTH"},
    {GL_CONVOLUTION_HEIGHT,                                "GL_CONVOLUTION_HEIGHT"},
    {GL_MAX_CONVOLUTION_WIDTH,                             "GL_MAX_CONVOLUTION_WIDTH"},
    {GL_MAX_CONVOLUTION_HEIGHT,                            "GL_MAX_CONVOLUTION_HEIGHT"},
    {GL_POST_CONVOLUTION_RED_SCALE,                        "GL_POST_CONVOLUTION_RED_SCALE"},
    {GL_POST_CONVOLUTION_GREEN_SCALE,                      "GL_POST_CONVOLUTION_GREEN_SCALE"},
    {GL_POST_CONVOLUTION_BLUE_SCALE,                       "GL_POST_CONVOLUTION_BLUE_SCALE"},
    {GL_POST_CONVOLUTION_ALPHA_SCALE,                      "GL_POST_CONVOLUTION_ALPHA_SCALE"},
    {GL_POST_CONVOLUTION_RED_BIAS,                         "GL_POST_CONVOLUTION_RED_BIAS"},
    {GL_POST_CONVOLUTION_GREEN_BIAS,                       "GL_POST_CONVOLUTION_GREEN_BIAS"},
    {GL_POST_CONVOLUTION_BLUE_BIAS,                        "GL_POST_CONVOLUTION_BLUE_BIAS"},
    {GL_POST_CONVOLUTION_ALPHA_BIAS,                       "GL_POST_CONVOLUTION_ALPHA_BIAS"},
    {GL_CONSTANT_BORDER,                                   "GL_CONSTANT_BORDER"},
    {GL_REPLICATE_BORDER,                                  "GL_REPLICATE_BORDER"},
    {GL_CONVOLUTION_BORDER_COLOR,                          "GL_CONVOLUTION_BORDER_COLOR"},

    /*draw_range_elements*/                                /*draw_range_elements*/
    {GL_MAX_ELEMENTS_VERTICES,                             "GL_MAX_ELEMENTS_VERTICES"},
    {GL_MAX_ELEMENTS_INDICES,                              "GL_MAX_ELEMENTS_INDICES"},

    /*histogram*/                                          /*histogram*/
    {GL_HISTOGRAM,                                         "GL_HISTOGRAM"},
    {GL_PROXY_HISTOGRAM,                                   "GL_PROXY_HISTOGRAM"},
    {GL_HISTOGRAM_WIDTH,                                   "GL_HISTOGRAM_WIDTH"},
    {GL_HISTOGRAM_FORMAT,                                  "GL_HISTOGRAM_FORMAT"},
    {GL_HISTOGRAM_RED_SIZE,                                "GL_HISTOGRAM_RED_SIZE"},
    {GL_HISTOGRAM_GREEN_SIZE,                              "GL_HISTOGRAM_GREEN_SIZE"},
    {GL_HISTOGRAM_BLUE_SIZE,                               "GL_HISTOGRAM_BLUE_SIZE"},
    {GL_HISTOGRAM_ALPHA_SIZE,                              "GL_HISTOGRAM_ALPHA_SIZE"},
    {GL_HISTOGRAM_LUMINANCE_SIZE,                          "GL_HISTOGRAM_LUMINANCE_SIZE"},
    {GL_HISTOGRAM_SINK,                                    "GL_HISTOGRAM_SINK"},
    {GL_MINMAX,                                            "GL_MINMAX"},
    {GL_MINMAX_FORMAT,                                     "GL_MINMAX_FORMAT"},
    {GL_MINMAX_SINK,                                       "GL_MINMAX_SINK"},
    {GL_TABLE_TOO_LARGE,                                   "GL_TABLE_TOO_LARGE"},

    /*packed_pixels*/                                      /*packed_pixels*/
    {GL_UNSIGNED_BYTE_3_3_2,                               "GL_UNSIGNED_BYTE_3_3_2"},
    {GL_UNSIGNED_SHORT_4_4_4_4,                            "GL_UNSIGNED_SHORT_4_4_4_4"},
    {GL_UNSIGNED_SHORT_5_5_5_1,                            "GL_UNSIGNED_SHORT_5_5_5_1"},
    {GL_UNSIGNED_INT_8_8_8_8,                              "GL_UNSIGNED_INT_8_8_8_8"},
    {GL_UNSIGNED_INT_10_10_10_2,                           "GL_UNSIGNED_INT_10_10_10_2"},
    {GL_UNSIGNED_BYTE_2_3_3_REV,                           "GL_UNSIGNED_BYTE_2_3_3_REV"},
    {GL_UNSIGNED_SHORT_5_6_5,                              "GL_UNSIGNED_SHORT_5_6_5"},
    {GL_UNSIGNED_SHORT_5_6_5_REV,                          "GL_UNSIGNED_SHORT_5_6_5_REV"},
    {GL_UNSIGNED_SHORT_4_4_4_4_REV,                        "GL_UNSIGNED_SHORT_4_4_4_4_REV"},
    {GL_UNSIGNED_SHORT_1_5_5_5_REV,                        "GL_UNSIGNED_SHORT_1_5_5_5_REV"},
    {GL_UNSIGNED_INT_8_8_8_8_REV,                          "GL_UNSIGNED_INT_8_8_8_8_REV"},
    {GL_UNSIGNED_INT_2_10_10_10_REV,                       "GL_UNSIGNED_INT_2_10_10_10_REV"},

    /*rescale_normal*/                                     /*rescale_normal*/
    {GL_RESCALE_NORMAL,                                    "GL_RESCALE_NORMAL"},

    /*separate_specular_color*/                            /*separate_specular_color*/
    {GL_LIGHT_MODEL_COLOR_CONTROL,                         "GL_LIGHT_MODEL_COLOR_CONTROL"},
    {GL_SINGLE_COLOR,                                      "GL_SINGLE_COLOR"},
    {GL_SEPARATE_SPECULAR_COLOR,                           "GL_SEPARATE_SPECULAR_COLOR"},

    /*texture3D*/                                          /*texture3D*/
    {GL_PACK_SKIP_IMAGES,                                  "GL_PACK_SKIP_IMAGES"},
    {GL_PACK_IMAGE_HEIGHT,                                 "GL_PACK_IMAGE_HEIGHT"},
    {GL_UNPACK_SKIP_IMAGES,                                "GL_UNPACK_SKIP_IMAGES"},
    {GL_UNPACK_IMAGE_HEIGHT,                               "GL_UNPACK_IMAGE_HEIGHT"},
    {GL_TEXTURE_3D,                                        "GL_TEXTURE_3D"},
    {GL_PROXY_TEXTURE_3D,                                  "GL_PROXY_TEXTURE_3D"},
    {GL_TEXTURE_DEPTH,                                     "GL_TEXTURE_DEPTH"},
    {GL_TEXTURE_WRAP_R,                                    "GL_TEXTURE_WRAP_R"},
    {GL_MAX_3D_TEXTURE_SIZE,                               "GL_MAX_3D_TEXTURE_SIZE"},

    /*texture_edge_clamp*/                                 /*texture_edge_clamp*/
    {GL_CLAMP_TO_EDGE,                                     "GL_CLAMP_TO_EDGE"},

    /*texture_lod*/                                        /*texture_lod*/
    {GL_TEXTURE_MIN_LOD,                                   "GL_TEXTURE_MIN_LOD"},
    {GL_TEXTURE_MAX_LOD,                                   "GL_TEXTURE_MAX_LOD"},
    {GL_TEXTURE_BASE_LEVEL,                                "GL_TEXTURE_BASE_LEVEL"},
    {GL_TEXTURE_MAX_LEVEL,                                 "GL_TEXTURE_MAX_LEVEL"},

    /*GetTarget1_2*/                                       /*GetTarget1_2*/
    {GL_SMOOTH_POINT_SIZE_RANGE,                           "GL_SMOOTH_POINT_SIZE_RANGE"},
    {GL_SMOOTH_POINT_SIZE_GRANULARITY,                     "GL_SMOOTH_POINT_SIZE_GRANULARITY"},
    {GL_SMOOTH_LINE_WIDTH_RANGE,                           "GL_SMOOTH_LINE_WIDTH_RANGE"},
    {GL_SMOOTH_LINE_WIDTH_GRANULARITY,                     "GL_SMOOTH_LINE_WIDTH_GRANULARITY"},
    {GL_ALIASED_POINT_SIZE_RANGE,                          "GL_ALIASED_POINT_SIZE_RANGE"},
    {GL_ALIASED_LINE_WIDTH_RANGE,                          "GL_ALIASED_LINE_WIDTH_RANGE"},

    /*multitexture*/                                       /*multitexture*/
    {GL_TEXTURE0_ARB,                                      "GL_TEXTURE0_ARB"},
    {GL_TEXTURE1_ARB,                                      "GL_TEXTURE1_ARB"},
    {GL_TEXTURE2_ARB,                                      "GL_TEXTURE2_ARB"},
    {GL_TEXTURE3_ARB,                                      "GL_TEXTURE3_ARB"},
    {GL_TEXTURE4_ARB,                                      "GL_TEXTURE4_ARB"},
    {GL_TEXTURE5_ARB,                                      "GL_TEXTURE5_ARB"},
    {GL_TEXTURE6_ARB,                                      "GL_TEXTURE6_ARB"},
    {GL_TEXTURE7_ARB,                                      "GL_TEXTURE7_ARB"},
    {GL_TEXTURE8_ARB,                                      "GL_TEXTURE8_ARB"},
    {GL_TEXTURE9_ARB,                                      "GL_TEXTURE9_ARB"},
    {GL_TEXTURE10_ARB,                                     "GL_TEXTURE10_ARB"},
    {GL_TEXTURE11_ARB,                                     "GL_TEXTURE11_ARB"},
    {GL_TEXTURE12_ARB,                                     "GL_TEXTURE12_ARB"},
    {GL_TEXTURE13_ARB,                                     "GL_TEXTURE13_ARB"},
    {GL_TEXTURE14_ARB,                                     "GL_TEXTURE14_ARB"},
    {GL_TEXTURE15_ARB,                                     "GL_TEXTURE15_ARB"},
    {GL_TEXTURE16_ARB,                                     "GL_TEXTURE16_ARB"},
    {GL_TEXTURE17_ARB,                                     "GL_TEXTURE17_ARB"},
    {GL_TEXTURE18_ARB,                                     "GL_TEXTURE18_ARB"},
    {GL_TEXTURE19_ARB,                                     "GL_TEXTURE19_ARB"},
    {GL_TEXTURE20_ARB,                                     "GL_TEXTURE20_ARB"},
    {GL_TEXTURE21_ARB,                                     "GL_TEXTURE21_ARB"},
    {GL_TEXTURE22_ARB,                                     "GL_TEXTURE22_ARB"},
    {GL_TEXTURE23_ARB,                                     "GL_TEXTURE23_ARB"},
    {GL_TEXTURE24_ARB,                                     "GL_TEXTURE24_ARB"},
    {GL_TEXTURE25_ARB,                                     "GL_TEXTURE25_ARB"},
    {GL_TEXTURE26_ARB,                                     "GL_TEXTURE26_ARB"},
    {GL_TEXTURE27_ARB,                                     "GL_TEXTURE27_ARB"},
    {GL_TEXTURE28_ARB,                                     "GL_TEXTURE28_ARB"},
    {GL_TEXTURE29_ARB,                                     "GL_TEXTURE29_ARB"},
    {GL_TEXTURE30_ARB,                                     "GL_TEXTURE30_ARB"},
    {GL_TEXTURE31_ARB,                                     "GL_TEXTURE31_ARB"},
    {GL_ACTIVE_TEXTURE_ARB,                                "GL_ACTIVE_TEXTURE_ARB"},
    {GL_CLIENT_ACTIVE_TEXTURE_ARB,                         "GL_CLIENT_ACTIVE_TEXTURE_ARB"},
    {GL_MAX_TEXTURE_UNITS_ARB,                             "GL_MAX_TEXTURE_UNITS_ARB"},

    /*EXT_abgr*/                                           /*EXT_abgr*/
    {GL_ABGR_EXT,                                          "GL_ABGR_EXT"},
    {GL_BGR_EXT,                                           "GL_BGR_EXT"},
    {GL_BGRA_EXT,                                          "GL_BGRA_EXT"},

    /*EXT_blend_color*/                                    /*EXT_blend_color*/
    {GL_CONSTANT_COLOR_EXT,                                "GL_CONSTANT_COLOR_EXT"},
    {GL_ONE_MINUS_CONSTANT_COLOR_EXT,                      "GL_ONE_MINUS_CONSTANT_COLOR_EXT"},
    {GL_CONSTANT_ALPHA_EXT,                                "GL_CONSTANT_ALPHA_EXT"},
    {GL_ONE_MINUS_CONSTANT_ALPHA_EXT,                      "GL_ONE_MINUS_CONSTANT_ALPHA_EXT"},
    {GL_BLEND_COLOR_EXT,                                   "GL_BLEND_COLOR_EXT"},

    /*EXT_blend_minmax*/                                   /*EXT_blend_minmax*/
    {GL_FUNC_ADD_EXT,                                      "GL_FUNC_ADD_EXT"},
    {GL_MIN_EXT,                                           "GL_MIN_EXT"},
    {GL_MAX_EXT,                                           "GL_MAX_EXT"},
    {GL_BLEND_EQUATION_EXT,                                "GL_BLEND_EQUATION_EXT"},

    /*EXT_blend_subtract*/                                 /*EXT_blend_subtract*/
    {GL_FUNC_SUBTRACT_EXT,                                 "GL_FUNC_SUBTRACT_EXT"},
    {GL_FUNC_REVERSE_SUBTRACT_EXT,                         "GL_FUNC_REVERSE_SUBTRACT_EXT"},

    /*EXT_texture_env_combine*/                            /*EXT_texture_env_combine*/
    {GL_COMBINE_EXT,                                       "GL_COMBINE_EXT"},
    {GL_COMBINE_RGB_EXT,                                   "GL_COMBINE_RGB_EXT"},
    {GL_COMBINE_ALPHA_EXT,                                 "GL_COMBINE_ALPHA_EXT"},
    {GL_RGB_SCALE_EXT,                                     "GL_RGB_SCALE_EXT"},
    {GL_ADD_SIGNED_EXT,                                    "GL_ADD_SIGNED_EXT"},
    {GL_INTERPOLATE_EXT,                                   "GL_INTERPOLATE_EXT"},
    {GL_CONSTANT_EXT,                                      "GL_CONSTANT_EXT"},
    {GL_PRIMARY_COLOR_EXT,                                 "GL_PRIMARY_COLOR_EXT"},
    {GL_PREVIOUS_EXT,                                      "GL_PREVIOUS_EXT"},
    {GL_SOURCE0_RGB_EXT,                                   "GL_SOURCE0_RGB_EXT"},
    {GL_SOURCE1_RGB_EXT,                                   "GL_SOURCE1_RGB_EXT"},
    {GL_SOURCE2_RGB_EXT,                                   "GL_SOURCE2_RGB_EXT"},
    {GL_SOURCE0_ALPHA_EXT,                                 "GL_SOURCE0_ALPHA_EXT"},
    {GL_SOURCE1_ALPHA_EXT,                                 "GL_SOURCE1_ALPHA_EXT"},
    {GL_SOURCE2_ALPHA_EXT,                                 "GL_SOURCE2_ALPHA_EXT"},
    {GL_OPERAND0_RGB_EXT,                                  "GL_OPERAND0_RGB_EXT"},
    {GL_OPERAND1_RGB_EXT,                                  "GL_OPERAND1_RGB_EXT"},
    {GL_OPERAND2_RGB_EXT,                                  "GL_OPERAND2_RGB_EXT"},
    {GL_OPERAND0_ALPHA_EXT,                                "GL_OPERAND0_ALPHA_EXT"},
    {GL_OPERAND1_ALPHA_EXT,                                "GL_OPERAND1_ALPHA_EXT"},
    {GL_OPERAND2_ALPHA_EXT,                                "GL_OPERAND2_ALPHA_EXT"},

    {GL_UNSIGNED_BYTE_3_3_2,                               "GL_UNSIGNED_BYTE_3_3_2"},
    {GL_UNSIGNED_SHORT_4_4_4_4,                            "GL_UNSIGNED_SHORT_4_4_4_4"},
    {GL_UNSIGNED_SHORT_5_5_5_1,                            "GL_UNSIGNED_SHORT_5_5_5_1"},
    {GL_UNSIGNED_INT_8_8_8_8,                              "GL_UNSIGNED_INT_8_8_8_8"},
    {GL_UNSIGNED_INT_10_10_10_2,                           "GL_UNSIGNED_INT_10_10_10_2"},
    {GL_RESCALE_NORMAL,                                    "GL_RESCALE_NORMAL"},
    {GL_TEXTURE_BINDING_3D,                                "GL_TEXTURE_BINDING_3D"},
    {GL_PACK_SKIP_IMAGES,                                  "GL_PACK_SKIP_IMAGES"},
    {GL_PACK_IMAGE_HEIGHT,                                 "GL_PACK_IMAGE_HEIGHT"},
    {GL_UNPACK_SKIP_IMAGES,                                "GL_UNPACK_SKIP_IMAGES"},
    {GL_UNPACK_IMAGE_HEIGHT,                               "GL_UNPACK_IMAGE_HEIGHT"},
    {GL_TEXTURE_3D,                                        "GL_TEXTURE_3D"},
    {GL_PROXY_TEXTURE_3D,                                  "GL_PROXY_TEXTURE_3D"},
    {GL_TEXTURE_DEPTH,                                     "GL_TEXTURE_DEPTH"},
    {GL_TEXTURE_WRAP_R,                                    "GL_TEXTURE_WRAP_R"},
    {GL_MAX_3D_TEXTURE_SIZE,                               "GL_MAX_3D_TEXTURE_SIZE"},
    {GL_UNSIGNED_BYTE_2_3_3_REV,                           "GL_UNSIGNED_BYTE_2_3_3_REV"},
    {GL_UNSIGNED_SHORT_5_6_5,                              "GL_UNSIGNED_SHORT_5_6_5"},
    {GL_UNSIGNED_SHORT_5_6_5_REV,                          "GL_UNSIGNED_SHORT_5_6_5_REV"},
    {GL_UNSIGNED_SHORT_4_4_4_4_REV,                        "GL_UNSIGNED_SHORT_4_4_4_4_REV"},
    {GL_UNSIGNED_SHORT_1_5_5_5_REV,                        "GL_UNSIGNED_SHORT_1_5_5_5_REV"},
    {GL_UNSIGNED_INT_8_8_8_8_REV,                          "GL_UNSIGNED_INT_8_8_8_8_REV"},
    {GL_UNSIGNED_INT_2_10_10_10_REV,                       "GL_UNSIGNED_INT_2_10_10_10_REV"},
    {GL_BGR,                                               "GL_BGR"},
    {GL_BGRA,                                              "GL_BGRA"},
    {GL_MAX_ELEMENTS_VERTICES,                             "GL_MAX_ELEMENTS_VERTICES"},
    {GL_MAX_ELEMENTS_INDICES,                              "GL_MAX_ELEMENTS_INDICES"},
    {GL_CLAMP_TO_EDGE,                                     "GL_CLAMP_TO_EDGE"},
    {GL_TEXTURE_MIN_LOD,                                   "GL_TEXTURE_MIN_LOD"},
    {GL_TEXTURE_MAX_LOD,                                   "GL_TEXTURE_MAX_LOD"},
    {GL_TEXTURE_BASE_LEVEL,                                "GL_TEXTURE_BASE_LEVEL"},
    {GL_TEXTURE_MAX_LEVEL,                                 "GL_TEXTURE_MAX_LEVEL"},
    {GL_LIGHT_MODEL_COLOR_CONTROL,                         "GL_LIGHT_MODEL_COLOR_CONTROL"},
    {GL_SINGLE_COLOR,                                      "GL_SINGLE_COLOR"},
    {GL_SEPARATE_SPECULAR_COLOR,                           "GL_SEPARATE_SPECULAR_COLOR"},
    {GL_SMOOTH_POINT_SIZE_RANGE,                           "GL_SMOOTH_POINT_SIZE_RANGE"},
    {GL_SMOOTH_POINT_SIZE_GRANULARITY,                     "GL_SMOOTH_POINT_SIZE_GRANULARITY"},
    {GL_SMOOTH_LINE_WIDTH_RANGE,                           "GL_SMOOTH_LINE_WIDTH_RANGE"},
    {GL_SMOOTH_LINE_WIDTH_GRANULARITY,                     "GL_SMOOTH_LINE_WIDTH_GRANULARITY"},
    {GL_ALIASED_POINT_SIZE_RANGE,                          "GL_ALIASED_POINT_SIZE_RANGE"},
    {GL_ALIASED_LINE_WIDTH_RANGE,                          "GL_ALIASED_LINE_WIDTH_RANGE"},

    /*GL_ARB_imaging*/                                     /*GL_ARB_imaging*/
    {GL_CONSTANT_COLOR,                                    "GL_CONSTANT_COLOR"},
    {GL_ONE_MINUS_CONSTANT_COLOR,                          "GL_ONE_MINUS_CONSTANT_COLOR"},
    {GL_CONSTANT_ALPHA,                                    "GL_CONSTANT_ALPHA"},
    {GL_ONE_MINUS_CONSTANT_ALPHA,                          "GL_ONE_MINUS_CONSTANT_ALPHA"},
    {GL_BLEND_COLOR,                                       "GL_BLEND_COLOR"},
    {GL_FUNC_ADD,                                          "GL_FUNC_ADD"},
    {GL_MIN,                                               "GL_MIN"},
    {GL_MAX,                                               "GL_MAX"},
    {GL_BLEND_EQUATION,                                    "GL_BLEND_EQUATION"},
    {GL_FUNC_SUBTRACT,                                     "GL_FUNC_SUBTRACT"},
    {GL_FUNC_REVERSE_SUBTRACT,                             "GL_FUNC_REVERSE_SUBTRACT"},
    {GL_CONVOLUTION_1D,                                    "GL_CONVOLUTION_1D"},
    {GL_CONVOLUTION_2D,                                    "GL_CONVOLUTION_2D"},
    {GL_SEPARABLE_2D,                                      "GL_SEPARABLE_2D"},
    {GL_CONVOLUTION_BORDER_MODE,                           "GL_CONVOLUTION_BORDER_MODE"},
    {GL_CONVOLUTION_FILTER_SCALE,                          "GL_CONVOLUTION_FILTER_SCALE"},
    {GL_CONVOLUTION_FILTER_BIAS,                           "GL_CONVOLUTION_FILTER_BIAS"},
    {GL_REDUCE,                                            "GL_REDUCE"},
    {GL_CONVOLUTION_FORMAT,                                "GL_CONVOLUTION_FORMAT"},
    {GL_CONVOLUTION_WIDTH,                                 "GL_CONVOLUTION_WIDTH"},
    {GL_CONVOLUTION_HEIGHT,                                "GL_CONVOLUTION_HEIGHT"},
    {GL_MAX_CONVOLUTION_WIDTH,                             "GL_MAX_CONVOLUTION_WIDTH"},
    {GL_MAX_CONVOLUTION_HEIGHT,                            "GL_MAX_CONVOLUTION_HEIGHT"},
    {GL_POST_CONVOLUTION_RED_SCALE,                        "GL_POST_CONVOLUTION_RED_SCALE"},
    {GL_POST_CONVOLUTION_GREEN_SCALE,                      "GL_POST_CONVOLUTION_GREEN_SCALE"},
    {GL_POST_CONVOLUTION_BLUE_SCALE,                       "GL_POST_CONVOLUTION_BLUE_SCALE"},
    {GL_POST_CONVOLUTION_ALPHA_SCALE,                      "GL_POST_CONVOLUTION_ALPHA_SCALE"},
    {GL_POST_CONVOLUTION_RED_BIAS,                         "GL_POST_CONVOLUTION_RED_BIAS"},
    {GL_POST_CONVOLUTION_GREEN_BIAS,                       "GL_POST_CONVOLUTION_GREEN_BIAS"},
    {GL_POST_CONVOLUTION_BLUE_BIAS,                        "GL_POST_CONVOLUTION_BLUE_BIAS"},
    {GL_POST_CONVOLUTION_ALPHA_BIAS,                       "GL_POST_CONVOLUTION_ALPHA_BIAS"},
    {GL_HISTOGRAM,                                         "GL_HISTOGRAM"},
    {GL_PROXY_HISTOGRAM,                                   "GL_PROXY_HISTOGRAM"},
    {GL_HISTOGRAM_WIDTH,                                   "GL_HISTOGRAM_WIDTH"},
    {GL_HISTOGRAM_FORMAT,                                  "GL_HISTOGRAM_FORMAT"},
    {GL_HISTOGRAM_RED_SIZE,                                "GL_HISTOGRAM_RED_SIZE"},
    {GL_HISTOGRAM_GREEN_SIZE,                              "GL_HISTOGRAM_GREEN_SIZE"},
    {GL_HISTOGRAM_BLUE_SIZE,                               "GL_HISTOGRAM_BLUE_SIZE"},
    {GL_HISTOGRAM_ALPHA_SIZE,                              "GL_HISTOGRAM_ALPHA_SIZE"},
    {GL_HISTOGRAM_LUMINANCE_SIZE,                          "GL_HISTOGRAM_LUMINANCE_SIZE"},
    {GL_HISTOGRAM_SINK,                                    "GL_HISTOGRAM_SINK"},
    {GL_MINMAX,                                            "GL_MINMAX"},
    {GL_MINMAX_FORMAT,                                     "GL_MINMAX_FORMAT"},
    {GL_MINMAX_SINK,                                       "GL_MINMAX_SINK"},
    {GL_TABLE_TOO_LARGE,                                   "GL_TABLE_TOO_LARGE"},
    {GL_COLOR_MATRIX,                                      "GL_COLOR_MATRIX"},
    {GL_COLOR_MATRIX_STACK_DEPTH,                          "GL_COLOR_MATRIX_STACK_DEPTH"},
    {GL_MAX_COLOR_MATRIX_STACK_DEPTH,                      "GL_MAX_COLOR_MATRIX_STACK_DEPTH"},
    {GL_POST_COLOR_MATRIX_RED_SCALE,                       "GL_POST_COLOR_MATRIX_RED_SCALE"},
    {GL_POST_COLOR_MATRIX_GREEN_SCALE,                     "GL_POST_COLOR_MATRIX_GREEN_SCALE"},
    {GL_POST_COLOR_MATRIX_BLUE_SCALE,                      "GL_POST_COLOR_MATRIX_BLUE_SCALE"},
    {GL_POST_COLOR_MATRIX_ALPHA_SCALE,                     "GL_POST_COLOR_MATRIX_ALPHA_SCALE"},
    {GL_POST_COLOR_MATRIX_RED_BIAS,                        "GL_POST_COLOR_MATRIX_RED_BIAS"},
    {GL_POST_COLOR_MATRIX_GREEN_BIAS,                      "GL_POST_COLOR_MATRIX_GREEN_BIAS"},
    {GL_POST_COLOR_MATRIX_BLUE_BIAS,                       "GL_POST_COLOR_MATRIX_BLUE_BIAS"},
    {GL_POST_COLOR_MATRIX_ALPHA_BIAS,                      "GL_POST_COLOR_MATRIX_ALPHA_BIAS"},
    {GL_COLOR_TABLE,                                       "GL_COLOR_TABLE"},
    {GL_POST_CONVOLUTION_COLOR_TABLE,                      "GL_POST_CONVOLUTION_COLOR_TABLE"},
    {GL_POST_COLOR_MATRIX_COLOR_TABLE,                     "GL_POST_COLOR_MATRIX_COLOR_TABLE"},
    {GL_PROXY_COLOR_TABLE,                                 "GL_PROXY_COLOR_TABLE"},
    {GL_PROXY_POST_CONVOLUTION_COLOR_TABLE,                "GL_PROXY_POST_CONVOLUTION_COLOR_TABLE"},
    {GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE,               "GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE"},
    {GL_COLOR_TABLE_SCALE,                                 "GL_COLOR_TABLE_SCALE"},
    {GL_COLOR_TABLE_BIAS,                                  "GL_COLOR_TABLE_BIAS"},
    {GL_COLOR_TABLE_FORMAT,                                "GL_COLOR_TABLE_FORMAT"},
    {GL_COLOR_TABLE_WIDTH,                                 "GL_COLOR_TABLE_WIDTH"},
    {GL_COLOR_TABLE_RED_SIZE,                              "GL_COLOR_TABLE_RED_SIZE"},
    {GL_COLOR_TABLE_GREEN_SIZE,                            "GL_COLOR_TABLE_GREEN_SIZE"},
    {GL_COLOR_TABLE_BLUE_SIZE,                             "GL_COLOR_TABLE_BLUE_SIZE"},
    {GL_COLOR_TABLE_ALPHA_SIZE,                            "GL_COLOR_TABLE_ALPHA_SIZE"},
    {GL_COLOR_TABLE_LUMINANCE_SIZE,                        "GL_COLOR_TABLE_LUMINANCE_SIZE"},
    {GL_COLOR_TABLE_INTENSITY_SIZE,                        "GL_COLOR_TABLE_INTENSITY_SIZE"},
    {GL_CONSTANT_BORDER,                                   "GL_CONSTANT_BORDER"},
    {GL_REPLICATE_BORDER,                                  "GL_REPLICATE_BORDER"},
    {GL_CONVOLUTION_BORDER_COLOR,                          "GL_CONVOLUTION_BORDER_COLOR"},

    /*GL_VERSION_1_3*/                                     /*GL_VERSION_1_3*/
    {GL_TEXTURE0,                                          "GL_TEXTURE0"},
    {GL_TEXTURE1,                                          "GL_TEXTURE1"},
    {GL_TEXTURE2,                                          "GL_TEXTURE2"},
    {GL_TEXTURE3,                                          "GL_TEXTURE3"},
    {GL_TEXTURE4,                                          "GL_TEXTURE4"},
    {GL_TEXTURE5,                                          "GL_TEXTURE5"},
    {GL_TEXTURE6,                                          "GL_TEXTURE6"},
    {GL_TEXTURE7,                                          "GL_TEXTURE7"},
    {GL_TEXTURE8,                                          "GL_TEXTURE8"},
    {GL_TEXTURE9,                                          "GL_TEXTURE9"},
    {GL_TEXTURE10,                                         "GL_TEXTURE10"},
    {GL_TEXTURE11,                                         "GL_TEXTURE11"},
    {GL_TEXTURE12,                                         "GL_TEXTURE12"},
    {GL_TEXTURE13,                                         "GL_TEXTURE13"},
    {GL_TEXTURE14,                                         "GL_TEXTURE14"},
    {GL_TEXTURE15,                                         "GL_TEXTURE15"},
    {GL_TEXTURE16,                                         "GL_TEXTURE16"},
    {GL_TEXTURE17,                                         "GL_TEXTURE17"},
    {GL_TEXTURE18,                                         "GL_TEXTURE18"},
    {GL_TEXTURE19,                                         "GL_TEXTURE19"},
    {GL_TEXTURE20,                                         "GL_TEXTURE20"},
    {GL_TEXTURE21,                                         "GL_TEXTURE21"},
    {GL_TEXTURE22,                                         "GL_TEXTURE22"},
    {GL_TEXTURE23,                                         "GL_TEXTURE23"},
    {GL_TEXTURE24,                                         "GL_TEXTURE24"},
    {GL_TEXTURE25,                                         "GL_TEXTURE25"},
    {GL_TEXTURE26,                                         "GL_TEXTURE26"},
    {GL_TEXTURE27,                                         "GL_TEXTURE27"},
    {GL_TEXTURE28,                                         "GL_TEXTURE28"},
    {GL_TEXTURE29,                                         "GL_TEXTURE29"},
    {GL_TEXTURE30,                                         "GL_TEXTURE30"},
    {GL_TEXTURE31,                                         "GL_TEXTURE31"},
    {GL_ACTIVE_TEXTURE,                                    "GL_ACTIVE_TEXTURE"},
    {GL_CLIENT_ACTIVE_TEXTURE,                             "GL_CLIENT_ACTIVE_TEXTURE"},
    {GL_MAX_TEXTURE_UNITS,                                 "GL_MAX_TEXTURE_UNITS"},
    {GL_TRANSPOSE_MODELVIEW_MATRIX,                        "GL_TRANSPOSE_MODELVIEW_MATRIX"},
    {GL_TRANSPOSE_PROJECTION_MATRIX,                       "GL_TRANSPOSE_PROJECTION_MATRIX"},
    {GL_TRANSPOSE_TEXTURE_MATRIX,                          "GL_TRANSPOSE_TEXTURE_MATRIX"},
    {GL_TRANSPOSE_COLOR_MATRIX,                            "GL_TRANSPOSE_COLOR_MATRIX"},
    {GL_MULTISAMPLE,                                       "GL_MULTISAMPLE"},
    {GL_SAMPLE_ALPHA_TO_COVERAGE,                          "GL_SAMPLE_ALPHA_TO_COVERAGE"},
    {GL_SAMPLE_ALPHA_TO_ONE,                               "GL_SAMPLE_ALPHA_TO_ONE"},
    {GL_SAMPLE_COVERAGE,                                   "GL_SAMPLE_COVERAGE"},
    {GL_SAMPLE_BUFFERS,                                    "GL_SAMPLE_BUFFERS"},
    {GL_SAMPLES,                                           "GL_SAMPLES"},
    {GL_SAMPLE_COVERAGE_VALUE,                             "GL_SAMPLE_COVERAGE_VALUE"},
    {GL_SAMPLE_COVERAGE_INVERT,                            "GL_SAMPLE_COVERAGE_INVERT"},

    {GL_NORMAL_MAP,                                        "GL_NORMAL_MAP"},
    {GL_REFLECTION_MAP,                                    "GL_REFLECTION_MAP"},
    {GL_TEXTURE_CUBE_MAP,                                  "GL_TEXTURE_CUBE_MAP"},
    {GL_TEXTURE_BINDING_CUBE_MAP,                          "GL_TEXTURE_BINDING_CUBE_MAP"},
    {GL_TEXTURE_CUBE_MAP_POSITIVE_X,                       "GL_TEXTURE_CUBE_MAP_POSITIVE_X"},
    {GL_TEXTURE_CUBE_MAP_NEGATIVE_X,                       "GL_TEXTURE_CUBE_MAP_NEGATIVE_X"},
    {GL_TEXTURE_CUBE_MAP_POSITIVE_Y,                       "GL_TEXTURE_CUBE_MAP_POSITIVE_Y"},
    {GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,                       "GL_TEXTURE_CUBE_MAP_NEGATIVE_Y"},
    {GL_TEXTURE_CUBE_MAP_POSITIVE_Z,                       "GL_TEXTURE_CUBE_MAP_POSITIVE_Z"},
    {GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,                       "GL_TEXTURE_CUBE_MAP_NEGATIVE_Z"},
    {GL_PROXY_TEXTURE_CUBE_MAP,                            "GL_PROXY_TEXTURE_CUBE_MAP"},
    {GL_MAX_CUBE_MAP_TEXTURE_SIZE,                         "GL_MAX_CUBE_MAP_TEXTURE_SIZE"},
    {GL_COMPRESSED_ALPHA,                                  "GL_COMPRESSED_ALPHA"},
    {GL_COMPRESSED_LUMINANCE,                              "GL_COMPRESSED_LUMINANCE"},
    {GL_COMPRESSED_LUMINANCE_ALPHA,                        "GL_COMPRESSED_LUMINANCE_ALPHA"},
    {GL_COMPRESSED_INTENSITY,                              "GL_COMPRESSED_INTENSITY"},
    {GL_COMPRESSED_RGB,                                    "GL_COMPRESSED_RGB"},
    {GL_COMPRESSED_RGBA,                                   "GL_COMPRESSED_RGBA"},
    {GL_TEXTURE_COMPRESSION_HINT,                          "GL_TEXTURE_COMPRESSION_HINT"},
    {GL_TEXTURE_COMPRESSED_IMAGE_SIZE,                     "GL_TEXTURE_COMPRESSED_IMAGE_SIZE"},
    {GL_TEXTURE_COMPRESSED,                                "GL_TEXTURE_COMPRESSED"},
    {GL_NUM_COMPRESSED_TEXTURE_FORMATS,                    "GL_NUM_COMPRESSED_TEXTURE_FORMATS"},
    {GL_COMPRESSED_TEXTURE_FORMATS,                        "GL_COMPRESSED_TEXTURE_FORMATS"},
    {GL_CLAMP_TO_BORDER,                                   "GL_CLAMP_TO_BORDER"},
    {GL_COMBINE,                                           "GL_COMBINE"},
    {GL_COMBINE_RGB,                                       "GL_COMBINE_RGB"},
    {GL_COMBINE_ALPHA,                                     "GL_COMBINE_ALPHA"},
    {GL_SOURCE0_RGB,                                       "GL_SOURCE0_RGB"},
    {GL_SOURCE1_RGB,                                       "GL_SOURCE1_RGB"},
    {GL_SOURCE2_RGB,                                       "GL_SOURCE2_RGB"},
    {GL_SOURCE0_ALPHA,                                     "GL_SOURCE0_ALPHA"},
    {GL_SOURCE1_ALPHA,                                     "GL_SOURCE1_ALPHA"},
    {GL_SOURCE2_ALPHA,                                     "GL_SOURCE2_ALPHA"},
    {GL_OPERAND0_RGB,                                      "GL_OPERAND0_RGB"},
    {GL_OPERAND1_RGB,                                      "GL_OPERAND1_RGB"},
    {GL_OPERAND2_RGB,                                      "GL_OPERAND2_RGB"},
    {GL_OPERAND0_ALPHA,                                    "GL_OPERAND0_ALPHA"},
    {GL_OPERAND1_ALPHA,                                    "GL_OPERAND1_ALPHA"},
    {GL_OPERAND2_ALPHA,                                    "GL_OPERAND2_ALPHA"},
    {GL_RGB_SCALE,                                         "GL_RGB_SCALE"},
    {GL_ADD_SIGNED,                                        "GL_ADD_SIGNED"},
    {GL_INTERPOLATE,                                       "GL_INTERPOLATE"},
    {GL_SUBTRACT,                                          "GL_SUBTRACT"},
    {GL_CONSTANT,                                          "GL_CONSTANT"},
    {GL_PRIMARY_COLOR,                                     "GL_PRIMARY_COLOR"},
    {GL_PREVIOUS,                                          "GL_PREVIOUS"},
    {GL_DOT3_RGB,                                          "GL_DOT3_RGB"},
    {GL_DOT3_RGBA,                                         "GL_DOT3_RGBA"},

    /*GL_VERSION_1_4*/                                     /*GL_VERSION_1_4*/
    {GL_BLEND_DST_RGB,                                     "GL_BLEND_DST_RGB"},
    {GL_BLEND_SRC_RGB,                                     "GL_BLEND_SRC_RGB"},
    {GL_BLEND_DST_ALPHA,                                   "GL_BLEND_DST_ALPHA"},
    {GL_BLEND_SRC_ALPHA,                                   "GL_BLEND_SRC_ALPHA"},
    {GL_POINT_SIZE_MIN,                                    "GL_POINT_SIZE_MIN"},
    {GL_POINT_SIZE_MAX,                                    "GL_POINT_SIZE_MAX"},
    {GL_POINT_FADE_THRESHOLD_SIZE,                         "GL_POINT_FADE_THRESHOLD_SIZE"},
    {GL_POINT_DISTANCE_ATTENUATION,                        "GL_POINT_DISTANCE_ATTENUATION"},
    {GL_GENERATE_MIPMAP,                                   "GL_GENERATE_MIPMAP"},
    {GL_GENERATE_MIPMAP_HINT,                              "GL_GENERATE_MIPMAP_HINT"},
    {GL_DEPTH_COMPONENT16,                                 "GL_DEPTH_COMPONENT16"},
    {GL_DEPTH_COMPONENT24,                                 "GL_DEPTH_COMPONENT24"},
    {GL_DEPTH_COMPONENT32,                                 "GL_DEPTH_COMPONENT32"},
    {GL_MIRRORED_REPEAT,                                   "GL_MIRRORED_REPEAT"},
    {GL_FOG_COORDINATE_SOURCE,                             "GL_FOG_COORDINATE_SOURCE"},
    {GL_FOG_COORDINATE,                                    "GL_FOG_COORDINATE"},
    {GL_FRAGMENT_DEPTH,                                    "GL_FRAGMENT_DEPTH"},
    {GL_CURRENT_FOG_COORDINATE,                            "GL_CURRENT_FOG_COORDINATE"},
    {GL_FOG_COORDINATE_ARRAY_TYPE,                         "GL_FOG_COORDINATE_ARRAY_TYPE"},
    {GL_FOG_COORDINATE_ARRAY_STRIDE,                       "GL_FOG_COORDINATE_ARRAY_STRIDE"},
    {GL_FOG_COORDINATE_ARRAY_POINTER,                      "GL_FOG_COORDINATE_ARRAY_POINTER"},
    {GL_FOG_COORDINATE_ARRAY,                              "GL_FOG_COORDINATE_ARRAY"},
    {GL_COLOR_SUM,                                         "GL_COLOR_SUM"},
    {GL_CURRENT_SECONDARY_COLOR,                           "GL_CURRENT_SECONDARY_COLOR"},
    {GL_SECONDARY_COLOR_ARRAY_SIZE,                        "GL_SECONDARY_COLOR_ARRAY_SIZE"},
    {GL_SECONDARY_COLOR_ARRAY_TYPE,                        "GL_SECONDARY_COLOR_ARRAY_TYPE"},
    {GL_SECONDARY_COLOR_ARRAY_STRIDE,                      "GL_SECONDARY_COLOR_ARRAY_STRIDE"},
    {GL_SECONDARY_COLOR_ARRAY_POINTER,                     "GL_SECONDARY_COLOR_ARRAY_POINTER"},
    {GL_SECONDARY_COLOR_ARRAY,                             "GL_SECONDARY_COLOR_ARRAY"},
    {GL_MAX_TEXTURE_LOD_BIAS,                              "GL_MAX_TEXTURE_LOD_BIAS"},
    {GL_TEXTURE_FILTER_CONTROL,                            "GL_TEXTURE_FILTER_CONTROL"},
    {GL_TEXTURE_LOD_BIAS,                                  "GL_TEXTURE_LOD_BIAS"},
    {GL_INCR_WRAP,                                         "GL_INCR_WRAP"},
    {GL_DECR_WRAP,                                         "GL_DECR_WRAP"},
    {GL_TEXTURE_DEPTH_SIZE,                                "GL_TEXTURE_DEPTH_SIZE"},
    {GL_DEPTH_TEXTURE_MODE,                                "GL_DEPTH_TEXTURE_MODE"},
    {GL_TEXTURE_COMPARE_MODE,                              "GL_TEXTURE_COMPARE_MODE"},
    {GL_TEXTURE_COMPARE_FUNC,                              "GL_TEXTURE_COMPARE_FUNC"},
    {GL_COMPARE_R_TO_TEXTURE,                              "GL_COMPARE_R_TO_TEXTURE"},


    /*GL_VERSION_1_5*/                                     /*GL_VERSION_1_5*/
    {GL_BUFFER_SIZE,                                       "GL_BUFFER_SIZE"},
    {GL_BUFFER_USAGE,                                      "GL_BUFFER_USAGE"},
    {GL_QUERY_COUNTER_BITS,                                "GL_QUERY_COUNTER_BITS"},
    {GL_CURRENT_QUERY,                                     "GL_CURRENT_QUERY"},
    {GL_QUERY_RESULT,                                      "GL_QUERY_RESULT"},
    {GL_QUERY_RESULT_AVAILABLE,                            "GL_QUERY_RESULT_AVAILABLE"},
    {GL_ARRAY_BUFFER,                                      "GL_ARRAY_BUFFER"},
    {GL_ELEMENT_ARRAY_BUFFER,                              "GL_ELEMENT_ARRAY_BUFFER"},
    {GL_ARRAY_BUFFER_BINDING,                              "GL_ARRAY_BUFFER_BINDING"},
    {GL_ELEMENT_ARRAY_BUFFER_BINDING,                      "GL_ELEMENT_ARRAY_BUFFER_BINDING"},
    {GL_VERTEX_ARRAY_BUFFER_BINDING,                       "GL_VERTEX_ARRAY_BUFFER_BINDING"},
    {GL_NORMAL_ARRAY_BUFFER_BINDING,                       "GL_NORMAL_ARRAY_BUFFER_BINDING"},
    {GL_COLOR_ARRAY_BUFFER_BINDING,                        "GL_COLOR_ARRAY_BUFFER_BINDING"},
    {GL_INDEX_ARRAY_BUFFER_BINDING,                        "GL_INDEX_ARRAY_BUFFER_BINDING"},
    {GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING,                "GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING"},
    {GL_EDGE_FLAG_ARRAY_BUFFER_BINDING,                    "GL_EDGE_FLAG_ARRAY_BUFFER_BINDING"},
    {GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING,              "GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING"},
    {GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING,               "GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING"},
    {GL_WEIGHT_ARRAY_BUFFER_BINDING,                       "GL_WEIGHT_ARRAY_BUFFER_BINDING"},
    {GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING,                "GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING"},
    {GL_READ_ONLY,                                         "GL_READ_ONLY"},
    {GL_WRITE_ONLY,                                        "GL_WRITE_ONLY"},
    {GL_READ_WRITE,                                        "GL_READ_WRITE"},
    {GL_BUFFER_ACCESS,                                     "GL_BUFFER_ACCESS"},
    {GL_BUFFER_MAPPED,                                     "GL_BUFFER_MAPPED"},
    {GL_BUFFER_MAP_POINTER,                                "GL_BUFFER_MAP_POINTER"},
    {GL_STREAM_DRAW,                                       "GL_STREAM_DRAW"},
    {GL_STREAM_READ,                                       "GL_STREAM_READ"},
    {GL_STREAM_COPY,                                       "GL_STREAM_COPY"},
    {GL_STATIC_DRAW,                                       "GL_STATIC_DRAW"},
    {GL_STATIC_READ,                                       "GL_STATIC_READ"},
    {GL_STATIC_COPY,                                       "GL_STATIC_COPY"},
    {GL_DYNAMIC_DRAW,                                      "GL_DYNAMIC_DRAW"},
    {GL_DYNAMIC_READ,                                      "GL_DYNAMIC_READ"},
    {GL_DYNAMIC_COPY,                                      "GL_DYNAMIC_COPY"},
    {GL_SAMPLES_PASSED,                                    "GL_SAMPLES_PASSED"},

    /*GL_VERSION_2_0*/                                     /*GL_VERSION_2_0*/
    {GL_VERTEX_ATTRIB_ARRAY_ENABLED,                       "GL_VERTEX_ATTRIB_ARRAY_ENABLED"},
    {GL_VERTEX_ATTRIB_ARRAY_SIZE,                          "GL_VERTEX_ATTRIB_ARRAY_SIZE"},
    {GL_VERTEX_ATTRIB_ARRAY_STRIDE,                        "GL_VERTEX_ATTRIB_ARRAY_STRIDE"},
    {GL_VERTEX_ATTRIB_ARRAY_TYPE,                          "GL_VERTEX_ATTRIB_ARRAY_TYPE"},
    {GL_CURRENT_VERTEX_ATTRIB,                             "GL_CURRENT_VERTEX_ATTRIB"},
    {GL_VERTEX_PROGRAM_POINT_SIZE,                         "GL_VERTEX_PROGRAM_POINT_SIZE"},
    {GL_VERTEX_PROGRAM_TWO_SIDE,                           "GL_VERTEX_PROGRAM_TWO_SIDE"},
    {GL_VERTEX_ATTRIB_ARRAY_POINTER,                       "GL_VERTEX_ATTRIB_ARRAY_POINTER"},
    {GL_STENCIL_BACK_FUNC,                                 "GL_STENCIL_BACK_FUNC"},
    {GL_STENCIL_BACK_FAIL,                                 "GL_STENCIL_BACK_FAIL"},
    {GL_STENCIL_BACK_PASS_DEPTH_FAIL,                      "GL_STENCIL_BACK_PASS_DEPTH_FAIL"},
    {GL_STENCIL_BACK_PASS_DEPTH_PASS,                      "GL_STENCIL_BACK_PASS_DEPTH_PASS"},
    {GL_MAX_DRAW_BUFFERS,                                  "GL_MAX_DRAW_BUFFERS"},
    {GL_DRAW_BUFFER0,                                      "GL_DRAW_BUFFER0"},
    {GL_DRAW_BUFFER1,                                      "GL_DRAW_BUFFER1"},
    {GL_DRAW_BUFFER2,                                      "GL_DRAW_BUFFER2"},
    {GL_DRAW_BUFFER3,                                      "GL_DRAW_BUFFER3"},
    {GL_DRAW_BUFFER4,                                      "GL_DRAW_BUFFER4"},
    {GL_DRAW_BUFFER5,                                      "GL_DRAW_BUFFER5"},
    {GL_DRAW_BUFFER6,                                      "GL_DRAW_BUFFER6"},
    {GL_DRAW_BUFFER7,                                      "GL_DRAW_BUFFER7"},
    {GL_DRAW_BUFFER8,                                      "GL_DRAW_BUFFER8"},
    {GL_DRAW_BUFFER9,                                      "GL_DRAW_BUFFER9"},
    {GL_DRAW_BUFFER10,                                     "GL_DRAW_BUFFER10"},
    {GL_DRAW_BUFFER11,                                     "GL_DRAW_BUFFER11"},
    {GL_DRAW_BUFFER12,                                     "GL_DRAW_BUFFER12"},
    {GL_DRAW_BUFFER13,                                     "GL_DRAW_BUFFER13"},
    {GL_DRAW_BUFFER14,                                     "GL_DRAW_BUFFER14"},
    {GL_DRAW_BUFFER15,                                     "GL_DRAW_BUFFER15"},
    {GL_BLEND_EQUATION_ALPHA,                              "GL_BLEND_EQUATION_ALPHA"},
    {GL_POINT_SPRITE,                                      "GL_POINT_SPRITE"},
    {GL_COORD_REPLACE,                                     "GL_COORD_REPLACE"},
    {GL_MAX_VERTEX_ATTRIBS,                                "GL_MAX_VERTEX_ATTRIBS"},
    {GL_VERTEX_ATTRIB_ARRAY_NORMALIZED,                    "GL_VERTEX_ATTRIB_ARRAY_NORMALIZED"},
    {GL_MAX_TEXTURE_COORDS,                                "GL_MAX_TEXTURE_COORDS"},
    {GL_MAX_TEXTURE_IMAGE_UNITS,                           "GL_MAX_TEXTURE_IMAGE_UNITS"},
    {GL_FRAGMENT_SHADER,                                   "GL_FRAGMENT_SHADER"},
    {GL_VERTEX_SHADER,                                     "GL_VERTEX_SHADER"},
    {GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,                   "GL_MAX_FRAGMENT_UNIFORM_COMPONENTS"},
    {GL_MAX_VERTEX_UNIFORM_COMPONENTS,                     "GL_MAX_VERTEX_UNIFORM_COMPONENTS"},
    {GL_MAX_VARYING_FLOATS,                                "GL_MAX_VARYING_FLOATS"},
    {GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,                    "GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS"},
    {GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,                  "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS"},
    {GL_SHADER_TYPE,                                       "GL_SHADER_TYPE"},
    {GL_FLOAT_VEC2,                                        "GL_FLOAT_VEC2"},
    {GL_FLOAT_VEC3,                                        "GL_FLOAT_VEC3"},
    {GL_FLOAT_VEC4,                                        "GL_FLOAT_VEC4"},
    {GL_INT_VEC2,                                          "GL_INT_VEC2"},
    {GL_INT_VEC3,                                          "GL_INT_VEC3"},
    {GL_INT_VEC4,                                          "GL_INT_VEC4"},
    {GL_BOOL,                                              "GL_BOOL"},
    {GL_BOOL_VEC2,                                         "GL_BOOL_VEC2"},
    {GL_BOOL_VEC3,                                         "GL_BOOL_VEC3"},
    {GL_BOOL_VEC4,                                         "GL_BOOL_VEC4"},
    {GL_FLOAT_MAT2,                                        "GL_FLOAT_MAT2"},
    {GL_FLOAT_MAT3,                                        "GL_FLOAT_MAT3"},
    {GL_FLOAT_MAT4,                                        "GL_FLOAT_MAT4"},
    {GL_SAMPLER_1D,                                        "GL_SAMPLER_1D"},
    {GL_SAMPLER_2D,                                        "GL_SAMPLER_2D"},
    {GL_SAMPLER_3D,                                        "GL_SAMPLER_3D"},
    {GL_SAMPLER_CUBE,                                      "GL_SAMPLER_CUBE"},
    {GL_SAMPLER_1D_SHADOW,                                 "GL_SAMPLER_1D_SHADOW"},
    {GL_SAMPLER_2D_SHADOW,                                 "GL_SAMPLER_2D_SHADOW"},
    {GL_DELETE_STATUS,                                     "GL_DELETE_STATUS"},
    {GL_COMPILE_STATUS,                                    "GL_COMPILE_STATUS"},
    {GL_LINK_STATUS,                                       "GL_LINK_STATUS"},
    {GL_VALIDATE_STATUS,                                   "GL_VALIDATE_STATUS"},
    {GL_INFO_LOG_LENGTH,                                   "GL_INFO_LOG_LENGTH"},
    {GL_ATTACHED_SHADERS,                                  "GL_ATTACHED_SHADERS"},
    {GL_ACTIVE_UNIFORMS,                                   "GL_ACTIVE_UNIFORMS"},
    {GL_ACTIVE_UNIFORM_MAX_LENGTH,                         "GL_ACTIVE_UNIFORM_MAX_LENGTH"},
    {GL_SHADER_SOURCE_LENGTH,                              "GL_SHADER_SOURCE_LENGTH"},
    {GL_ACTIVE_ATTRIBUTES,                                 "GL_ACTIVE_ATTRIBUTES"},
    {GL_ACTIVE_ATTRIBUTE_MAX_LENGTH,                       "GL_ACTIVE_ATTRIBUTE_MAX_LENGTH"},
    {GL_FRAGMENT_SHADER_DERIVATIVE_HINT,                   "GL_FRAGMENT_SHADER_DERIVATIVE_HINT"},
    {GL_SHADING_LANGUAGE_VERSION,                          "GL_SHADING_LANGUAGE_VERSION"},
    {GL_CURRENT_PROGRAM,                                   "GL_CURRENT_PROGRAM"},
    {GL_POINT_SPRITE_COORD_ORIGIN,                         "GL_POINT_SPRITE_COORD_ORIGIN"},
    {GL_LOWER_LEFT,                                        "GL_LOWER_LEFT"},
    {GL_UPPER_LEFT,                                        "GL_UPPER_LEFT"},
    {GL_STENCIL_BACK_REF,                                  "GL_STENCIL_BACK_REF"},
    {GL_STENCIL_BACK_VALUE_MASK,                           "GL_STENCIL_BACK_VALUE_MASK"},
    {GL_STENCIL_BACK_WRITEMASK,                            "GL_STENCIL_BACK_WRITEMASK"},

    /*GL_ARB_multitexture*/                                /*GL_ARB_multitexture*/
    {GL_TEXTURE0_ARB,                                      "GL_TEXTURE0_ARB"},
    {GL_TEXTURE1_ARB,                                      "GL_TEXTURE1_ARB"},
    {GL_TEXTURE2_ARB,                                      "GL_TEXTURE2_ARB"},
    {GL_TEXTURE3_ARB,                                      "GL_TEXTURE3_ARB"},
    {GL_TEXTURE4_ARB,                                      "GL_TEXTURE4_ARB"},
    {GL_TEXTURE5_ARB,                                      "GL_TEXTURE5_ARB"},
    {GL_TEXTURE6_ARB,                                      "GL_TEXTURE6_ARB"},
    {GL_TEXTURE7_ARB,                                      "GL_TEXTURE7_ARB"},
    {GL_TEXTURE8_ARB,                                      "GL_TEXTURE8_ARB"},
    {GL_TEXTURE9_ARB,                                      "GL_TEXTURE9_ARB"},
    {GL_TEXTURE10_ARB,                                     "GL_TEXTURE10_ARB"},
    {GL_TEXTURE11_ARB,                                     "GL_TEXTURE11_ARB"},
    {GL_TEXTURE12_ARB,                                     "GL_TEXTURE12_ARB"},
    {GL_TEXTURE13_ARB,                                     "GL_TEXTURE13_ARB"},
    {GL_TEXTURE14_ARB,                                     "GL_TEXTURE14_ARB"},
    {GL_TEXTURE15_ARB,                                     "GL_TEXTURE15_ARB"},
    {GL_TEXTURE16_ARB,                                     "GL_TEXTURE16_ARB"},
    {GL_TEXTURE17_ARB,                                     "GL_TEXTURE17_ARB"},
    {GL_TEXTURE18_ARB,                                     "GL_TEXTURE18_ARB"},
    {GL_TEXTURE19_ARB,                                     "GL_TEXTURE19_ARB"},
    {GL_TEXTURE20_ARB,                                     "GL_TEXTURE20_ARB"},
    {GL_TEXTURE21_ARB,                                     "GL_TEXTURE21_ARB"},
    {GL_TEXTURE22_ARB,                                     "GL_TEXTURE22_ARB"},
    {GL_TEXTURE23_ARB,                                     "GL_TEXTURE23_ARB"},
    {GL_TEXTURE24_ARB,                                     "GL_TEXTURE24_ARB"},
    {GL_TEXTURE25_ARB,                                     "GL_TEXTURE25_ARB"},
    {GL_TEXTURE26_ARB,                                     "GL_TEXTURE26_ARB"},
    {GL_TEXTURE27_ARB,                                     "GL_TEXTURE27_ARB"},
    {GL_TEXTURE28_ARB,                                     "GL_TEXTURE28_ARB"},
    {GL_TEXTURE29_ARB,                                     "GL_TEXTURE29_ARB"},
    {GL_TEXTURE30_ARB,                                     "GL_TEXTURE30_ARB"},
    {GL_TEXTURE31_ARB,                                     "GL_TEXTURE31_ARB"},
    {GL_ACTIVE_TEXTURE_ARB,                                "GL_ACTIVE_TEXTURE_ARB"},
    {GL_CLIENT_ACTIVE_TEXTURE_ARB,                         "GL_CLIENT_ACTIVE_TEXTURE_ARB"},
    {GL_MAX_TEXTURE_UNITS_ARB,                             "GL_MAX_TEXTURE_UNITS_ARB"},


    /*GL_ARB_transpose_matrix*/                            /*GL_ARB_transpose_matrix*/
    {GL_TRANSPOSE_MODELVIEW_MATRIX_ARB,                    "GL_TRANSPOSE_MODELVIEW_MATRIX_ARB"},
    {GL_TRANSPOSE_PROJECTION_MATRIX_ARB,                   "GL_TRANSPOSE_PROJECTION_MATRIX_ARB"},
    {GL_TRANSPOSE_TEXTURE_MATRIX_ARB,                      "GL_TRANSPOSE_TEXTURE_MATRIX_ARB"},
    {GL_TRANSPOSE_COLOR_MATRIX_ARB,                        "GL_TRANSPOSE_COLOR_MATRIX_ARB"},

    /*GL_ARB_multisample*/                                 /*GL_ARB_multisample*/
    {GL_MULTISAMPLE_ARB,                                   "GL_MULTISAMPLE_ARB"},
    {GL_SAMPLE_ALPHA_TO_COVERAGE_ARB,                      "GL_SAMPLE_ALPHA_TO_COVERAGE_ARB"},
    {GL_SAMPLE_ALPHA_TO_ONE_ARB,                           "GL_SAMPLE_ALPHA_TO_ONE_ARB"},
    {GL_SAMPLE_COVERAGE_ARB,                               "GL_SAMPLE_COVERAGE_ARB"},
    {GL_SAMPLE_BUFFERS_ARB,                                "GL_SAMPLE_BUFFERS_ARB"},
    {GL_SAMPLES_ARB,                                       "GL_SAMPLES_ARB"},
    {GL_SAMPLE_COVERAGE_VALUE_ARB,                         "GL_SAMPLE_COVERAGE_VALUE_ARB"},
    {GL_SAMPLE_COVERAGE_INVERT_ARB,                        "GL_SAMPLE_COVERAGE_INVERT_ARB"},


    /*GL_ARB_texture_cube_map*/                            /*GL_ARB_texture_cube_map*/
    {GL_NORMAL_MAP_ARB,                                    "GL_NORMAL_MAP_ARB"},
    {GL_REFLECTION_MAP_ARB,                                "GL_REFLECTION_MAP_ARB"},
    {GL_TEXTURE_CUBE_MAP_ARB,                              "GL_TEXTURE_CUBE_MAP_ARB"},
    {GL_TEXTURE_BINDING_CUBE_MAP_ARB,                      "GL_TEXTURE_BINDING_CUBE_MAP_ARB"},
    {GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB,                   "GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB"},
    {GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB,                   "GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB"},
    {GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB,                   "GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB"},
    {GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB,                   "GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB"},
    {GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB,                   "GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB"},
    {GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB,                   "GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB"},
    {GL_PROXY_TEXTURE_CUBE_MAP_ARB,                        "GL_PROXY_TEXTURE_CUBE_MAP_ARB"},
    {GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB,                     "GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB"},

    /*GL_ARB_texture_compression*/                         /*GL_ARB_texture_compression*/
    {GL_COMPRESSED_ALPHA_ARB,                              "GL_COMPRESSED_ALPHA_ARB"},
    {GL_COMPRESSED_LUMINANCE_ARB,                          "GL_COMPRESSED_LUMINANCE_ARB"},
    {GL_COMPRESSED_LUMINANCE_ALPHA_ARB,                    "GL_COMPRESSED_LUMINANCE_ALPHA_ARB"},
    {GL_COMPRESSED_INTENSITY_ARB,                          "GL_COMPRESSED_INTENSITY_ARB"},
    {GL_COMPRESSED_RGB_ARB,                                "GL_COMPRESSED_RGB_ARB"},
    {GL_COMPRESSED_RGBA_ARB,                               "GL_COMPRESSED_RGBA_ARB"},
    {GL_TEXTURE_COMPRESSION_HINT_ARB,                      "GL_TEXTURE_COMPRESSION_HINT_ARB"},
    {GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB,                 "GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB"},
    {GL_TEXTURE_COMPRESSED_ARB,                            "GL_TEXTURE_COMPRESSED_ARB"},
    {GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB,                "GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB"},
    {GL_COMPRESSED_TEXTURE_FORMATS_ARB,                    "GL_COMPRESSED_TEXTURE_FORMATS_ARB"},

    /*GL_ARB_texture_border_clamp*/                        /*GL_ARB_texture_border_clamp*/
    {GL_CLAMP_TO_BORDER_ARB,                               "GL_CLAMP_TO_BORDER_ARB"},

    /*GL_ARB_point_parameters*/                            /*GL_ARB_point_parameters*/
    {GL_POINT_SIZE_MIN_ARB,                                "GL_POINT_SIZE_MIN_ARB"},
    {GL_POINT_SIZE_MAX_ARB,                                "GL_POINT_SIZE_MAX_ARB"},
    {GL_POINT_FADE_THRESHOLD_SIZE_ARB,                     "GL_POINT_FADE_THRESHOLD_SIZE_ARB"},
    {GL_POINT_DISTANCE_ATTENUATION_ARB,                    "GL_POINT_DISTANCE_ATTENUATION_ARB"},

    /*GL_ARB_vertex_blend*/                                /*GL_ARB_vertex_blend*/
    {GL_MAX_VERTEX_UNITS_ARB,                              "GL_MAX_VERTEX_UNITS_ARB"},
    {GL_ACTIVE_VERTEX_UNITS_ARB,                           "GL_ACTIVE_VERTEX_UNITS_ARB"},
    {GL_WEIGHT_SUM_UNITY_ARB,                              "GL_WEIGHT_SUM_UNITY_ARB"},
    {GL_VERTEX_BLEND_ARB,                                  "GL_VERTEX_BLEND_ARB"},
    {GL_CURRENT_WEIGHT_ARB,                                "GL_CURRENT_WEIGHT_ARB"},
    {GL_WEIGHT_ARRAY_TYPE_ARB,                             "GL_WEIGHT_ARRAY_TYPE_ARB"},
    {GL_WEIGHT_ARRAY_STRIDE_ARB,                           "GL_WEIGHT_ARRAY_STRIDE_ARB"},
    {GL_WEIGHT_ARRAY_SIZE_ARB,                             "GL_WEIGHT_ARRAY_SIZE_ARB"},
    {GL_WEIGHT_ARRAY_POINTER_ARB,                          "GL_WEIGHT_ARRAY_POINTER_ARB"},
    {GL_WEIGHT_ARRAY_ARB,                                  "GL_WEIGHT_ARRAY_ARB"},
    {GL_MODELVIEW0_ARB,                                    "GL_MODELVIEW0_ARB"},
    {GL_MODELVIEW1_ARB,                                    "GL_MODELVIEW1_ARB"},
    {GL_MODELVIEW2_ARB,                                    "GL_MODELVIEW2_ARB"},
    {GL_MODELVIEW3_ARB,                                    "GL_MODELVIEW3_ARB"},
    {GL_MODELVIEW4_ARB,                                    "GL_MODELVIEW4_ARB"},
    {GL_MODELVIEW5_ARB,                                    "GL_MODELVIEW5_ARB"},
    {GL_MODELVIEW6_ARB,                                    "GL_MODELVIEW6_ARB"},
    {GL_MODELVIEW7_ARB,                                    "GL_MODELVIEW7_ARB"},
    {GL_MODELVIEW8_ARB,                                    "GL_MODELVIEW8_ARB"},
    {GL_MODELVIEW9_ARB,                                    "GL_MODELVIEW9_ARB"},
    {GL_MODELVIEW10_ARB,                                   "GL_MODELVIEW10_ARB"},
    {GL_MODELVIEW11_ARB,                                   "GL_MODELVIEW11_ARB"},
    {GL_MODELVIEW12_ARB,                                   "GL_MODELVIEW12_ARB"},
    {GL_MODELVIEW13_ARB,                                   "GL_MODELVIEW13_ARB"},
    {GL_MODELVIEW14_ARB,                                   "GL_MODELVIEW14_ARB"},
    {GL_MODELVIEW15_ARB,                                   "GL_MODELVIEW15_ARB"},
    {GL_MODELVIEW16_ARB,                                   "GL_MODELVIEW16_ARB"},
    {GL_MODELVIEW17_ARB,                                   "GL_MODELVIEW17_ARB"},
    {GL_MODELVIEW18_ARB,                                   "GL_MODELVIEW18_ARB"},
    {GL_MODELVIEW19_ARB,                                   "GL_MODELVIEW19_ARB"},
    {GL_MODELVIEW20_ARB,                                   "GL_MODELVIEW20_ARB"},
    {GL_MODELVIEW21_ARB,                                   "GL_MODELVIEW21_ARB"},
    {GL_MODELVIEW22_ARB,                                   "GL_MODELVIEW22_ARB"},
    {GL_MODELVIEW23_ARB,                                   "GL_MODELVIEW23_ARB"},
    {GL_MODELVIEW24_ARB,                                   "GL_MODELVIEW24_ARB"},
    {GL_MODELVIEW25_ARB,                                   "GL_MODELVIEW25_ARB"},
    {GL_MODELVIEW26_ARB,                                   "GL_MODELVIEW26_ARB"},
    {GL_MODELVIEW27_ARB,                                   "GL_MODELVIEW27_ARB"},
    {GL_MODELVIEW28_ARB,                                   "GL_MODELVIEW28_ARB"},
    {GL_MODELVIEW29_ARB,                                   "GL_MODELVIEW29_ARB"},
    {GL_MODELVIEW30_ARB,                                   "GL_MODELVIEW30_ARB"},
    {GL_MODELVIEW31_ARB,                                   "GL_MODELVIEW31_ARB"},

    /*GL_ARB_matrix_palette*/                              /*GL_ARB_matrix_palette*/
    {GL_MATRIX_PALETTE_ARB,                                "GL_MATRIX_PALETTE_ARB"},
    {GL_MAX_MATRIX_PALETTE_STACK_DEPTH_ARB,                "GL_MAX_MATRIX_PALETTE_STACK_DEPTH_ARB"},
    {GL_MAX_PALETTE_MATRICES_ARB,                          "GL_MAX_PALETTE_MATRICES_ARB"},
    {GL_CURRENT_PALETTE_MATRIX_ARB,                        "GL_CURRENT_PALETTE_MATRIX_ARB"},
    {GL_MATRIX_INDEX_ARRAY_ARB,                            "GL_MATRIX_INDEX_ARRAY_ARB"},
    {GL_CURRENT_MATRIX_INDEX_ARB,                          "GL_CURRENT_MATRIX_INDEX_ARB"},
    {GL_MATRIX_INDEX_ARRAY_SIZE_ARB,                       "GL_MATRIX_INDEX_ARRAY_SIZE_ARB"},
    {GL_MATRIX_INDEX_ARRAY_TYPE_ARB,                       "GL_MATRIX_INDEX_ARRAY_TYPE_ARB"},
    {GL_MATRIX_INDEX_ARRAY_STRIDE_ARB,                     "GL_MATRIX_INDEX_ARRAY_STRIDE_ARB"},
    {GL_MATRIX_INDEX_ARRAY_POINTER_ARB,                    "GL_MATRIX_INDEX_ARRAY_POINTER_ARB"},

    /*GL_ARB_texture_env_combine*/                         /*GL_ARB_texture_env_combine*/
    {GL_COMBINE_ARB,                                       "GL_COMBINE_ARB"},
    {GL_COMBINE_RGB_ARB,                                   "GL_COMBINE_RGB_ARB"},
    {GL_COMBINE_ALPHA_ARB,                                 "GL_COMBINE_ALPHA_ARB"},
    {GL_SOURCE0_RGB_ARB,                                   "GL_SOURCE0_RGB_ARB"},
    {GL_SOURCE1_RGB_ARB,                                   "GL_SOURCE1_RGB_ARB"},
    {GL_SOURCE2_RGB_ARB,                                   "GL_SOURCE2_RGB_ARB"},
    {GL_SOURCE0_ALPHA_ARB,                                 "GL_SOURCE0_ALPHA_ARB"},
    {GL_SOURCE1_ALPHA_ARB,                                 "GL_SOURCE1_ALPHA_ARB"},
    {GL_SOURCE2_ALPHA_ARB,                                 "GL_SOURCE2_ALPHA_ARB"},
    {GL_OPERAND0_RGB_ARB,                                  "GL_OPERAND0_RGB_ARB"},
    {GL_OPERAND1_RGB_ARB,                                  "GL_OPERAND1_RGB_ARB"},
    {GL_OPERAND2_RGB_ARB,                                  "GL_OPERAND2_RGB_ARB"},
    {GL_OPERAND0_ALPHA_ARB,                                "GL_OPERAND0_ALPHA_ARB"},
    {GL_OPERAND1_ALPHA_ARB,                                "GL_OPERAND1_ALPHA_ARB"},
    {GL_OPERAND2_ALPHA_ARB,                                "GL_OPERAND2_ALPHA_ARB"},
    {GL_RGB_SCALE_ARB,                                     "GL_RGB_SCALE_ARB"},
    {GL_ADD_SIGNED_ARB,                                    "GL_ADD_SIGNED_ARB"},
    {GL_INTERPOLATE_ARB,                                   "GL_INTERPOLATE_ARB"},
    {GL_SUBTRACT_ARB,                                      "GL_SUBTRACT_ARB"},
    {GL_CONSTANT_ARB,                                      "GL_CONSTANT_ARB"},
    {GL_PRIMARY_COLOR_ARB,                                 "GL_PRIMARY_COLOR_ARB"},
    {GL_PREVIOUS_ARB,                                      "GL_PREVIOUS_ARB"},


    /*GL_ARB_texture_env_dot3*/                            /*GL_ARB_texture_env_dot3*/
    {GL_DOT3_RGB_ARB,                                      "GL_DOT3_RGB_ARB"},
    {GL_DOT3_RGBA_ARB,                                     "GL_DOT3_RGBA_ARB"},

    /*GL_ARB_texture_mirrored_repeat*/                     /*GL_ARB_texture_mirrored_repeat*/
    {GL_MIRRORED_REPEAT_ARB,                               "GL_MIRRORED_REPEAT_ARB"},

    /*GL_ARB_depth_texture*/                               /*GL_ARB_depth_texture*/
    {GL_DEPTH_COMPONENT16_ARB,                             "GL_DEPTH_COMPONENT16_ARB"},
    {GL_DEPTH_COMPONENT24_ARB,                             "GL_DEPTH_COMPONENT24_ARB"},
    {GL_DEPTH_COMPONENT32_ARB,                             "GL_DEPTH_COMPONENT32_ARB"},
    {GL_TEXTURE_DEPTH_SIZE_ARB,                            "GL_TEXTURE_DEPTH_SIZE_ARB"},
    {GL_DEPTH_TEXTURE_MODE_ARB,                            "GL_DEPTH_TEXTURE_MODE_ARB"},

    /*GL_ARB_shadow*/                                      /*GL_ARB_shadow*/
    {GL_TEXTURE_COMPARE_MODE_ARB,                          "GL_TEXTURE_COMPARE_MODE_ARB"},
    {GL_TEXTURE_COMPARE_FUNC_ARB,                          "GL_TEXTURE_COMPARE_FUNC_ARB"},
    {GL_COMPARE_R_TO_TEXTURE_ARB,                          "GL_COMPARE_R_TO_TEXTURE_ARB"},

    /*GL_ARB_shadow_ambient*/                              /*GL_ARB_shadow_ambient*/
    {GL_TEXTURE_COMPARE_FAIL_VALUE_ARB,                    "GL_TEXTURE_COMPARE_FAIL_VALUE_ARB"},

    /*GL_ARB_vertex_program*/                              /*GL_ARB_vertex_program*/
    {GL_COLOR_SUM_ARB,                                     "GL_COLOR_SUM_ARB"},
    {GL_VERTEX_PROGRAM_ARB,                                "GL_VERTEX_PROGRAM_ARB"},
    {GL_VERTEX_ATTRIB_ARRAY_ENABLED_ARB,                   "GL_VERTEX_ATTRIB_ARRAY_ENABLED_ARB2"},
    {GL_VERTEX_ATTRIB_ARRAY_SIZE_ARB,                      "GL_VERTEX_ATTRIB_ARRAY_SIZE_ARB"},
    {GL_VERTEX_ATTRIB_ARRAY_STRIDE_ARB,                    "GL_VERTEX_ATTRIB_ARRAY_STRIDE_ARB"},
    {GL_VERTEX_ATTRIB_ARRAY_TYPE_ARB,                      "GL_VERTEX_ATTRIB_ARRAY_TYPE_ARB"},
    {GL_CURRENT_VERTEX_ATTRIB_ARB,                         "GL_CURRENT_VERTEX_ATTRIB_ARB"},
    {GL_PROGRAM_LENGTH_ARB,                                "GL_PROGRAM_LENGTH_ARB"},
    {GL_PROGRAM_STRING_ARB,                                "GL_PROGRAM_STRING_ARB"},
    {GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB,                "GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB"},
    {GL_MAX_PROGRAM_MATRICES_ARB,                          "GL_MAX_PROGRAM_MATRICES_ARB"},
    {GL_CURRENT_MATRIX_STACK_DEPTH_ARB,                    "GL_CURRENT_MATRIX_STACK_DEPTH_ARB"},
    {GL_CURRENT_MATRIX_ARB,                                "GL_CURRENT_MATRIX_ARB"},
    {GL_VERTEX_PROGRAM_POINT_SIZE_ARB,                     "GL_VERTEX_PROGRAM_POINT_SIZE_ARB"},
    {GL_VERTEX_PROGRAM_TWO_SIDE_ARB,                       "GL_VERTEX_PROGRAM_TWO_SIDE_ARB"},
    {GL_VERTEX_ATTRIB_ARRAY_POINTER_ARB,                   "GL_VERTEX_ATTRIB_ARRAY_POINTER_ARB"},
    {GL_PROGRAM_ERROR_POSITION_ARB,                        "GL_PROGRAM_ERROR_POSITION_ARB"},
    {GL_PROGRAM_BINDING_ARB,                               "GL_PROGRAM_BINDING_ARB"},
    {GL_MAX_VERTEX_ATTRIBS_ARB,                            "GL_MAX_VERTEX_ATTRIBS_ARB"},
    {GL_VERTEX_ATTRIB_ARRAY_NORMALIZED_ARB,                "GL_VERTEX_ATTRIB_ARRAY_NORMALIZED_ARB"},
    {GL_PROGRAM_ERROR_STRING_ARB,                          "GL_PROGRAM_ERROR_STRING_ARB"},
    {GL_PROGRAM_FORMAT_ASCII_ARB,                          "GL_PROGRAM_FORMAT_ASCII_ARB"},
    {GL_PROGRAM_FORMAT_ARB,                                "GL_PROGRAM_FORMAT_ARB"},
    {GL_PROGRAM_INSTRUCTIONS_ARB,                          "GL_PROGRAM_INSTRUCTIONS_ARB"},
    {GL_MAX_PROGRAM_INSTRUCTIONS_ARB,                      "GL_MAX_PROGRAM_INSTRUCTIONS_ARB"},
    {GL_PROGRAM_NATIVE_INSTRUCTIONS_ARB,                   "GL_PROGRAM_NATIVE_INSTRUCTIONS_ARB"},
    {GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB,               "GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB"},
    {GL_PROGRAM_TEMPORARIES_ARB,                           "GL_PROGRAM_TEMPORARIES_ARB"},
    {GL_MAX_PROGRAM_TEMPORARIES_ARB,                       "GL_MAX_PROGRAM_TEMPORARIES_ARB"},
    {GL_PROGRAM_NATIVE_TEMPORARIES_ARB,                    "GL_PROGRAM_NATIVE_TEMPORARIES_ARB"},
    {GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB,                "GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB"},
    {GL_PROGRAM_PARAMETERS_ARB,                            "GL_PROGRAM_PARAMETERS_ARB"},
    {GL_MAX_PROGRAM_PARAMETERS_ARB,                        "GL_MAX_PROGRAM_PARAMETERS_ARB"},
    {GL_PROGRAM_NATIVE_PARAMETERS_ARB,                     "GL_PROGRAM_NATIVE_PARAMETERS_ARB"},
    {GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB,                 "GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB"},
    {GL_PROGRAM_ATTRIBS_ARB,                               "GL_PROGRAM_ATTRIBS_ARB"},
    {GL_MAX_PROGRAM_ATTRIBS_ARB,                           "GL_MAX_PROGRAM_ATTRIBS_ARB"},
    {GL_PROGRAM_NATIVE_ATTRIBS_ARB,                        "GL_PROGRAM_NATIVE_ATTRIBS_ARB"},
    {GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB,                    "GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB"},
    {GL_PROGRAM_ADDRESS_REGISTERS_ARB,                     "GL_PROGRAM_ADDRESS_REGISTERS_ARB"},
    {GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB,                 "GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB"},
    {GL_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB,              "GL_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB"},
    {GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB,          "GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB"},
    {GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB,                  "GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB"},
    {GL_MAX_PROGRAM_ENV_PARAMETERS_ARB,                    "GL_MAX_PROGRAM_ENV_PARAMETERS_ARB"},
    {GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB,                   "GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB"},
    {GL_TRANSPOSE_CURRENT_MATRIX_ARB,                      "GL_TRANSPOSE_CURRENT_MATRIX_ARB"},
    {GL_MATRIX0_ARB,                                       "GL_MATRIX0_ARB"},
    {GL_MATRIX1_ARB,                                       "GL_MATRIX1_ARB"},
    {GL_MATRIX2_ARB,                                       "GL_MATRIX2_ARB"},
    {GL_MATRIX3_ARB,                                       "GL_MATRIX3_ARB"},
    {GL_MATRIX4_ARB,                                       "GL_MATRIX4_ARB"},
    {GL_MATRIX5_ARB,                                       "GL_MATRIX5_ARB"},
    {GL_MATRIX6_ARB,                                       "GL_MATRIX6_ARB"},
    {GL_MATRIX7_ARB,                                       "GL_MATRIX7_ARB"},
    {GL_MATRIX8_ARB,                                       "GL_MATRIX8_ARB"},
    {GL_MATRIX9_ARB,                                       "GL_MATRIX9_ARB"},
    {GL_MATRIX10_ARB,                                      "GL_MATRIX10_ARB"},
    {GL_MATRIX11_ARB,                                      "GL_MATRIX11_ARB"},
    {GL_MATRIX12_ARB,                                      "GL_MATRIX12_ARB"},
    {GL_MATRIX13_ARB,                                      "GL_MATRIX13_ARB"},
    {GL_MATRIX14_ARB,                                      "GL_MATRIX14_ARB"},
    {GL_MATRIX15_ARB,                                      "GL_MATRIX15_ARB"},
    {GL_MATRIX16_ARB,                                      "GL_MATRIX16_ARB"},
    {GL_MATRIX17_ARB,                                      "GL_MATRIX17_ARB"},
    {GL_MATRIX18_ARB,                                      "GL_MATRIX18_ARB"},
    {GL_MATRIX19_ARB,                                      "GL_MATRIX19_ARB"},
    {GL_MATRIX20_ARB,                                      "GL_MATRIX20_ARB"},
    {GL_MATRIX21_ARB,                                      "GL_MATRIX21_ARB"},
    {GL_MATRIX22_ARB,                                      "GL_MATRIX22_ARB"},
    {GL_MATRIX23_ARB,                                      "GL_MATRIX23_ARB"},
    {GL_MATRIX24_ARB,                                      "GL_MATRIX24_ARB"},
    {GL_MATRIX25_ARB,                                      "GL_MATRIX25_ARB"},
    {GL_MATRIX26_ARB,                                      "GL_MATRIX26_ARB"},
    {GL_MATRIX27_ARB,                                      "GL_MATRIX27_ARB"},
    {GL_MATRIX28_ARB,                                      "GL_MATRIX28_ARB"},
    {GL_MATRIX29_ARB,                                      "GL_MATRIX29_ARB"},
    {GL_MATRIX30_ARB,                                      "GL_MATRIX30_ARB"},
    {GL_MATRIX31_ARB,                                      "GL_MATRIX31_ARB"},

    /*GL_ARB_fragment_program*/                            /*GL_ARB_fragment_program*/
    {GL_FRAGMENT_PROGRAM_ARB,                              "GL_FRAGMENT_PROGRAM_ARB"},
    {GL_PROGRAM_ALU_INSTRUCTIONS_ARB,                      "GL_PROGRAM_ALU_INSTRUCTIONS_ARB"},
    {GL_PROGRAM_TEX_INSTRUCTIONS_ARB,                      "GL_PROGRAM_TEX_INSTRUCTIONS_ARB"},
    {GL_PROGRAM_TEX_INDIRECTIONS_ARB,                      "GL_PROGRAM_TEX_INDIRECTIONS_ARB"},
    {GL_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB,               "GL_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB"},
    {GL_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB,               "GL_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB"},
    {GL_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB,               "GL_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB"},
    {GL_MAX_PROGRAM_ALU_INSTRUCTIONS_ARB,                  "GL_MAX_PROGRAM_ALU_INSTRUCTIONS_ARB"},
    {GL_MAX_PROGRAM_TEX_INSTRUCTIONS_ARB,                  "GL_MAX_PROGRAM_TEX_INSTRUCTIONS_ARB"},
    {GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB,                  "GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB"},
    {GL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB,           "GL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB"},
    {GL_MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB,           "GL_MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB"},
    {GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB,           "GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB"},
    {GL_MAX_TEXTURE_COORDS_ARB,                            "GL_MAX_TEXTURE_COORDS_ARB"},
    {GL_MAX_TEXTURE_IMAGE_UNITS_ARB,                       "GL_MAX_TEXTURE_IMAGE_UNITS_ARB"},

    /*GL_ARB_vertex_buffer_object*/                        /*GL_ARB_vertex_buffer_object*/
    {GL_BUFFER_SIZE_ARB,                                   "GL_BUFFER_SIZE_ARB"},
    {GL_BUFFER_USAGE_ARB,                                  "GL_BUFFER_USAGE_ARB"},
    {GL_ARRAY_BUFFER_ARB,                                  "GL_ARRAY_BUFFER_ARB"},
    {GL_ELEMENT_ARRAY_BUFFER_ARB,                          "GL_ELEMENT_ARRAY_BUFFER_ARB"},
    {GL_ARRAY_BUFFER_BINDING_ARB,                          "GL_ARRAY_BUFFER_BINDING_ARB"},
    {GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB,                  "GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB"},
    {GL_VERTEX_ARRAY_BUFFER_BINDING_ARB,                   "GL_VERTEX_ARRAY_BUFFER_BINDING_ARB"},
    {GL_NORMAL_ARRAY_BUFFER_BINDING_ARB,                   "GL_NORMAL_ARRAY_BUFFER_BINDING_ARB"},
    {GL_COLOR_ARRAY_BUFFER_BINDING_ARB,                    "GL_COLOR_ARRAY_BUFFER_BINDING_ARB"},
    {GL_INDEX_ARRAY_BUFFER_BINDING_ARB,                    "GL_INDEX_ARRAY_BUFFER_BINDING_ARB"},
    {GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING_ARB,            "GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING_ARB"},
    {GL_EDGE_FLAG_ARRAY_BUFFER_BINDING_ARB,                "GL_EDGE_FLAG_ARRAY_BUFFER_BINDING_ARB"},
    {GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING_ARB,          "GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING_ARB"},
    {GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING_ARB,           "GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING_ARB"},
    {GL_WEIGHT_ARRAY_BUFFER_BINDING_ARB,                   "GL_WEIGHT_ARRAY_BUFFER_BINDING_ARB"},
    {GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB,            "GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB"},
    {GL_READ_ONLY_ARB,                                     "GL_READ_ONLY_ARB"},
    {GL_WRITE_ONLY_ARB,                                    "GL_WRITE_ONLY_ARB"},
    {GL_READ_WRITE_ARB,                                    "GL_READ_WRITE_ARB"},
    {GL_BUFFER_ACCESS_ARB,                                 "GL_BUFFER_ACCESS_ARB"},
    {GL_BUFFER_MAPPED_ARB,                                 "GL_BUFFER_MAPPED_ARB"},
    {GL_BUFFER_MAP_POINTER_ARB,                            "GL_BUFFER_MAP_POINTER_ARB"},
    {GL_STREAM_DRAW_ARB,                                   "GL_STREAM_DRAW_ARB"},
    {GL_STREAM_READ_ARB,                                   "GL_STREAM_READ_ARB"},
    {GL_STREAM_COPY_ARB,                                   "GL_STREAM_COPY_ARB"},
    {GL_STATIC_DRAW_ARB,                                   "GL_STATIC_DRAW_ARB"},
    {GL_STATIC_READ_ARB,                                   "GL_STATIC_READ_ARB"},
    {GL_STATIC_COPY_ARB,                                   "GL_STATIC_COPY_ARB"},
    {GL_DYNAMIC_DRAW_ARB,                                  "GL_DYNAMIC_DRAW_ARB"},
    {GL_DYNAMIC_READ_ARB,                                  "GL_DYNAMIC_READ_ARB"},
    {GL_DYNAMIC_COPY_ARB,                                  "GL_DYNAMIC_COPY_ARB"},


    /*GL_ARB_occlusion_query*/                             /*GL_ARB_occlusion_query*/
    {GL_QUERY_COUNTER_BITS_ARB,                            "GL_QUERY_COUNTER_BITS_ARB"},
    {GL_CURRENT_QUERY_ARB,                                 "GL_CURRENT_QUERY_ARB"},
    {GL_QUERY_RESULT_ARB,                                  "GL_QUERY_RESULT_ARB"},
    {GL_QUERY_RESULT_AVAILABLE_ARB,                        "GL_QUERY_RESULT_AVAILABLE_ARB"},
    {GL_SAMPLES_PASSED_ARB,                                "GL_SAMPLES_PASSED_ARB"},

    /*GL_ARB_shader_objects*/                              /*GL_ARB_shader_objects*/
    {GL_PROGRAM_OBJECT_ARB,                                "GL_PROGRAM_OBJECT_ARB"},
    {GL_SHADER_OBJECT_ARB,                                 "GL_SHADER_OBJECT_ARB"},
    {GL_OBJECT_TYPE_ARB,                                   "GL_OBJECT_TYPE_ARB"},
    {GL_OBJECT_SUBTYPE_ARB,                                "GL_OBJECT_SUBTYPE_ARB"},
    {GL_FLOAT_VEC2_ARB,                                    "GL_FLOAT_VEC2_ARB"},
    {GL_FLOAT_VEC3_ARB,                                    "GL_FLOAT_VEC3_ARB"},
    {GL_FLOAT_VEC4_ARB,                                    "GL_FLOAT_VEC4_ARB"},
    {GL_INT_VEC2_ARB,                                      "GL_INT_VEC2_ARB"},
    {GL_INT_VEC3_ARB,                                      "GL_INT_VEC3_ARB"},
    {GL_INT_VEC4_ARB,                                      "GL_INT_VEC4_ARB"},
    {GL_BOOL_ARB,                                          "GL_BOOL_ARB"},
    {GL_BOOL_VEC2_ARB,                                     "GL_BOOL_VEC2_ARB"},
    {GL_BOOL_VEC3_ARB,                                     "GL_BOOL_VEC3_ARB"},
    {GL_BOOL_VEC4_ARB,                                     "GL_BOOL_VEC4_ARB"},
    {GL_FLOAT_MAT2_ARB,                                    "GL_FLOAT_MAT2_ARB"},
    {GL_FLOAT_MAT3_ARB,                                    "GL_FLOAT_MAT3_ARB"},
    {GL_FLOAT_MAT4_ARB,                                    "GL_FLOAT_MAT4_ARB"},
    {GL_SAMPLER_1D_ARB,                                    "GL_SAMPLER_1D_ARB"},
    {GL_SAMPLER_2D_ARB,                                    "GL_SAMPLER_2D_ARB"},
    {GL_SAMPLER_3D_ARB,                                    "GL_SAMPLER_3D_ARB"},
    {GL_SAMPLER_CUBE_ARB,                                  "GL_SAMPLER_CUBE_ARB"},
    {GL_SAMPLER_1D_SHADOW_ARB,                             "GL_SAMPLER_1D_SHADOW_ARB"},
    {GL_SAMPLER_2D_SHADOW_ARB,                             "GL_SAMPLER_2D_SHADOW_ARB"},
    {GL_SAMPLER_2D_RECT_ARB,                               "GL_SAMPLER_2D_RECT_ARB"},
    {GL_SAMPLER_2D_RECT_SHADOW_ARB,                        "GL_SAMPLER_2D_RECT_SHADOW_ARB"},
    {GL_OBJECT_DELETE_STATUS_ARB,                          "GL_OBJECT_DELETE_STATUS_ARB"},
    {GL_OBJECT_COMPILE_STATUS_ARB,                         "GL_OBJECT_COMPILE_STATUS_ARB"},
    {GL_OBJECT_LINK_STATUS_ARB,                            "GL_OBJECT_LINK_STATUS_ARB"},
    {GL_OBJECT_VALIDATE_STATUS_ARB,                        "GL_OBJECT_VALIDATE_STATUS_ARB"},
    {GL_OBJECT_INFO_LOG_LENGTH_ARB,                        "GL_OBJECT_INFO_LOG_LENGTH_ARB"},
    {GL_OBJECT_ATTACHED_OBJECTS_ARB,                       "GL_OBJECT_ATTACHED_OBJECTS_ARB"},
    {GL_OBJECT_ACTIVE_UNIFORMS_ARB,                        "GL_OBJECT_ACTIVE_UNIFORMS_ARB"},
    {GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB,              "GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB"},
    {GL_OBJECT_SHADER_SOURCE_LENGTH_ARB,                   "GL_OBJECT_SHADER_SOURCE_LENGTH_ARB"},

    /*GL_ARB_vertex_shader*/                               /*GL_ARB_vertex_shader*/
    {GL_VERTEX_SHADER_ARB,                                 "GL_VERTEX_SHADER_ARB"},
    {GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB,                 "GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB"},
    {GL_MAX_VARYING_FLOATS_ARB,                            "GL_MAX_VARYING_FLOATS_ARB"},
    {GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB,                "GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB"},
    {GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB,              "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB"},
    {GL_OBJECT_ACTIVE_ATTRIBUTES_ARB,                      "GL_OBJECT_ACTIVE_ATTRIBUTES_ARB"},
    {GL_OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH_ARB,            "GL_OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH_ARB"},

    /*GL_ARB_fragment_shader*/                             /*GL_ARB_fragment_shader*/
    {GL_FRAGMENT_SHADER_ARB,                               "GL_FRAGMENT_SHADER_ARB"},
    {GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB,               "GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB"},
    {GL_FRAGMENT_SHADER_DERIVATIVE_HINT_ARB,               "GL_FRAGMENT_SHADER_DERIVATIVE_HINT_ARB"},

    /*GL_ARB_shading_language_100*/                        /*GL_ARB_shading_language_100*/
    {GL_SHADING_LANGUAGE_VERSION_ARB,                      "GL_SHADING_LANGUAGE_VERSION_ARB"},

    /*GL_ARB_texture_non_power_of_two*/                    /*GL_ARB_texture_non_power_of_two*/


    /*GL_ARB_point_sprite*/                                /*GL_ARB_point_sprite*/
    {GL_POINT_SPRITE_ARB,                                  "GL_POINT_SPRITE_ARB"},
    {GL_COORD_REPLACE_ARB,                                 "GL_COORD_REPLACE_ARB"},


    /*GL_ARB_fragment_program_shadow*/                     /*GL_ARB_fragment_program_shadow*/

    /*GL_ARB_draw_buffers*/                                /*GL_ARB_draw_buffers*/
    {GL_MAX_DRAW_BUFFERS_ARB,                              "GL_MAX_DRAW_BUFFERS_ARB"},
    {GL_DRAW_BUFFER0_ARB,                                  "GL_DRAW_BUFFER0_ARB"},
    {GL_DRAW_BUFFER1_ARB,                                  "GL_DRAW_BUFFER1_ARB"},
    {GL_DRAW_BUFFER2_ARB,                                  "GL_DRAW_BUFFER2_ARB"},
    {GL_DRAW_BUFFER3_ARB,                                  "GL_DRAW_BUFFER3_ARB"},
    {GL_DRAW_BUFFER4_ARB,                                  "GL_DRAW_BUFFER4_ARB"},
    {GL_DRAW_BUFFER5_ARB,                                  "GL_DRAW_BUFFER5_ARB"},
    {GL_DRAW_BUFFER6_ARB,                                  "GL_DRAW_BUFFER6_ARB"},
    {GL_DRAW_BUFFER7_ARB,                                  "GL_DRAW_BUFFER7_ARB"},
    {GL_DRAW_BUFFER8_ARB,                                  "GL_DRAW_BUFFER8_ARB"},
    {GL_DRAW_BUFFER9_ARB,                                  "GL_DRAW_BUFFER9_ARB"},
    {GL_DRAW_BUFFER10_ARB,                                 "GL_DRAW_BUFFER10_ARB"},
    {GL_DRAW_BUFFER11_ARB,                                 "GL_DRAW_BUFFER11_ARB"},
    {GL_DRAW_BUFFER12_ARB,                                 "GL_DRAW_BUFFER12_ARB"},
    {GL_DRAW_BUFFER13_ARB,                                 "GL_DRAW_BUFFER13_ARB"},
    {GL_DRAW_BUFFER14_ARB,                                 "GL_DRAW_BUFFER14_ARB"},
    {GL_DRAW_BUFFER15_ARB,                                 "GL_DRAW_BUFFER15_ARB"},


    /*GL_ARB_texture_rectangle*/                           /*GL_ARB_texture_rectangle*/
    {GL_TEXTURE_RECTANGLE_ARB,                             "GL_TEXTURE_RECTANGLE_ARB"},
    {GL_TEXTURE_BINDING_RECTANGLE_ARB,                     "GL_TEXTURE_BINDING_RECTANGLE_ARB"},
    {GL_PROXY_TEXTURE_RECTANGLE_ARB,                       "GL_PROXY_TEXTURE_RECTANGLE_ARB"},
    {GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB,                    "GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB"},


    /*GL_ARB_color_buffer_float*/                          /*GL_ARB_color_buffer_float*/
    {GL_RGBA_FLOAT_MODE_ARB,                               "GL_RGBA_FLOAT_MODE_ARB"},
    {GL_CLAMP_VERTEX_COLOR_ARB,                            "GL_CLAMP_VERTEX_COLOR_ARB"},
    {GL_CLAMP_FRAGMENT_COLOR_ARB,                          "GL_CLAMP_FRAGMENT_COLOR_ARB"},
    {GL_CLAMP_READ_COLOR_ARB,                              "GL_CLAMP_READ_COLOR_ARB"},
    {GL_FIXED_ONLY_ARB,                                    "GL_FIXED_ONLY_ARB"},


    /*GL_ARB_half_float_pixel*/                            /*GL_ARB_half_float_pixel*/
    {GL_HALF_FLOAT_ARB,                                    "GL_HALF_FLOAT_ARB"},


    /*GL_ARB_texture_float*/                               /*GL_ARB_texture_float*/
    {GL_TEXTURE_RED_TYPE_ARB,                              "GL_TEXTURE_RED_TYPE_ARB"},
    {GL_TEXTURE_GREEN_TYPE_ARB,                            "GL_TEXTURE_GREEN_TYPE_ARB"},
    {GL_TEXTURE_BLUE_TYPE_ARB,                             "GL_TEXTURE_BLUE_TYPE_ARB"},
    {GL_TEXTURE_ALPHA_TYPE_ARB,                            "GL_TEXTURE_ALPHA_TYPE_ARB"},
    {GL_TEXTURE_LUMINANCE_TYPE_ARB,                        "GL_TEXTURE_LUMINANCE_TYPE_ARB"},
    {GL_TEXTURE_INTENSITY_TYPE_ARB,                        "GL_TEXTURE_INTENSITY_TYPE_ARB"},
    {GL_TEXTURE_DEPTH_TYPE_ARB,                            "GL_TEXTURE_DEPTH_TYPE_ARB"},
    {GL_UNSIGNED_NORMALIZED_ARB,                           "GL_UNSIGNED_NORMALIZED_ARB"},
    {GL_RGBA32F_ARB,                                       "GL_RGBA32F_ARB"},
    {GL_RGB32F_ARB,                                        "GL_RGB32F_ARB"},
    {GL_ALPHA32F_ARB,                                      "GL_ALPHA32F_ARB"},
    {GL_INTENSITY32F_ARB,                                  "GL_INTENSITY32F_ARB"},
    {GL_LUMINANCE32F_ARB,                                  "GL_LUMINANCE32F_ARB"},
    {GL_LUMINANCE_ALPHA32F_ARB,                            "GL_LUMINANCE_ALPHA32F_ARB"},
    {GL_RGBA16F_ARB,                                       "GL_RGBA16F_ARB"},
    {GL_RGB16F_ARB,                                        "GL_RGB16F_ARB"},
    {GL_ALPHA16F_ARB,                                      "GL_ALPHA16F_ARB"},
    {GL_INTENSITY16F_ARB,                                  "GL_INTENSITY16F_ARB"},
    {GL_LUMINANCE16F_ARB,                                  "GL_LUMINANCE16F_ARB"},
    {GL_LUMINANCE_ALPHA16F_ARB,                            "GL_LUMINANCE_ALPHA16F_ARB"},


    /*GL_ARB_pixel_buffer_object*/                         /*GL_ARB_pixel_buffer_object*/
    {GL_PIXEL_PACK_BUFFER_ARB,                             "GL_PIXEL_PACK_BUFFER_ARB"},
    {GL_PIXEL_UNPACK_BUFFER_ARB,                           "GL_PIXEL_UNPACK_BUFFER_ARB"},
    {GL_PIXEL_PACK_BUFFER_BINDING_ARB,                     "GL_PIXEL_PACK_BUFFER_BINDING_ARB"},
    {GL_PIXEL_UNPACK_BUFFER_BINDING_ARB,                   "GL_PIXEL_UNPACK_BUFFER_BINDING_ARB"},


    /*GL_EXT_abgr*/                                        /*GL_EXT_abgr*/
    {GL_ABGR_EXT,                                          "GL_ABGR_EXT"},


    /*GL_EXT_blend_color*/                                 /*GL_EXT_blend_color*/
    {GL_CONSTANT_COLOR_EXT,                                "GL_CONSTANT_COLOR_EXT"},
    {GL_ONE_MINUS_CONSTANT_COLOR_EXT,                      "GL_ONE_MINUS_CONSTANT_COLOR_EXT"},
    {GL_CONSTANT_ALPHA_EXT,                                "GL_CONSTANT_ALPHA_EXT"},
    {GL_ONE_MINUS_CONSTANT_ALPHA_EXT,                      "GL_ONE_MINUS_CONSTANT_ALPHA_EXT"},
    {GL_BLEND_COLOR_EXT,                                   "GL_BLEND_COLOR_EXT"},


    /*GL_EXT_polygon_offset*/                              /*GL_EXT_polygon_offset*/
    {GL_POLYGON_OFFSET_EXT,                                "GL_POLYGON_OFFSET_EXT"},
    {GL_POLYGON_OFFSET_FACTOR_EXT,                         "GL_POLYGON_OFFSET_FACTOR_EXT"},
    {GL_POLYGON_OFFSET_BIAS_EXT,                           "GL_POLYGON_OFFSET_BIAS_EXT"},


    /*GL_EXT_texture*/                                     /*GL_EXT_texture*/
    {GL_ALPHA4_EXT,                                        "GL_ALPHA4_EXT"},
    {GL_ALPHA8_EXT,                                        "GL_ALPHA8_EXT"},
    {GL_ALPHA12_EXT,                                       "GL_ALPHA12_EXT"},
    {GL_ALPHA16_EXT,                                       "GL_ALPHA16_EXT"},
    {GL_LUMINANCE4_EXT,                                    "GL_LUMINANCE4_EXT"},
    {GL_LUMINANCE8_EXT,                                    "GL_LUMINANCE8_EXT"},
    {GL_LUMINANCE12_EXT,                                   "GL_LUMINANCE12_EXT"},
    {GL_LUMINANCE16_EXT,                                   "GL_LUMINANCE16_EXT"},
    {GL_LUMINANCE4_ALPHA4_EXT,                             "GL_LUMINANCE4_ALPHA4_EXT"},
    {GL_LUMINANCE6_ALPHA2_EXT,                             "GL_LUMINANCE6_ALPHA2_EXT"},
    {GL_LUMINANCE8_ALPHA8_EXT,                             "GL_LUMINANCE8_ALPHA8_EXT"},
    {GL_LUMINANCE12_ALPHA4_EXT,                            "GL_LUMINANCE12_ALPHA4_EXT"},
    {GL_LUMINANCE12_ALPHA12_EXT,                           "GL_LUMINANCE12_ALPHA12_EXT"},
    {GL_LUMINANCE16_ALPHA16_EXT,                           "GL_LUMINANCE16_ALPHA16_EXT"},
    {GL_INTENSITY_EXT,                                     "GL_INTENSITY_EXT"},
    {GL_INTENSITY4_EXT,                                    "GL_INTENSITY4_EXT"},
    {GL_INTENSITY8_EXT,                                    "GL_INTENSITY8_EXT"},
    {GL_INTENSITY12_EXT,                                   "GL_INTENSITY12_EXT"},
    {GL_INTENSITY16_EXT,                                   "GL_INTENSITY16_EXT"},
    {GL_RGB2_EXT,                                          "GL_RGB2_EXT"},
    {GL_RGB4_EXT,                                          "GL_RGB4_EXT"},
    {GL_RGB5_EXT,                                          "GL_RGB5_EXT"},
    {GL_RGB8_EXT,                                          "GL_RGB8_EXT"},
    {GL_RGB10_EXT,                                         "GL_RGB10_EXT"},
    {GL_RGB12_EXT,                                         "GL_RGB12_EXT"},
    {GL_RGB16_EXT,                                         "GL_RGB16_EXT"},
    {GL_RGBA2_EXT,                                         "GL_RGBA2_EXT"},
    {GL_RGBA4_EXT,                                         "GL_RGBA4_EXT"},
    {GL_RGB5_A1_EXT,                                       "GL_RGB5_A1_EXT"},
    {GL_RGBA8_EXT,                                         "GL_RGBA8_EXT"},
    {GL_RGB10_A2_EXT,                                      "GL_RGB10_A2_EXT"},
    {GL_RGBA12_EXT,                                        "GL_RGBA12_EXT"},
    {GL_RGBA16_EXT,                                        "GL_RGBA16_EXT"},
    {GL_TEXTURE_RED_SIZE_EXT,                              "GL_TEXTURE_RED_SIZE_EXT"},
    {GL_TEXTURE_GREEN_SIZE_EXT,                            "GL_TEXTURE_GREEN_SIZE_EXT"},
    {GL_TEXTURE_BLUE_SIZE_EXT,                             "GL_TEXTURE_BLUE_SIZE_EXT"},
    {GL_TEXTURE_ALPHA_SIZE_EXT,                            "GL_TEXTURE_ALPHA_SIZE_EXT"},
    {GL_TEXTURE_LUMINANCE_SIZE_EXT,                        "GL_TEXTURE_LUMINANCE_SIZE_EXT"},
    {GL_TEXTURE_INTENSITY_SIZE_EXT,                        "GL_TEXTURE_INTENSITY_SIZE_EXT"},
    {GL_REPLACE_EXT,                                       "GL_REPLACE_EXT"},
    {GL_PROXY_TEXTURE_1D_EXT,                              "GL_PROXY_TEXTURE_1D_EXT"},
    {GL_PROXY_TEXTURE_2D_EXT,                              "GL_PROXY_TEXTURE_2D_EXT"},
    {GL_TEXTURE_TOO_LARGE_EXT,                             "GL_TEXTURE_TOO_LARGE_EXT"},


    /*GL_EXT_texture3D*/                                   /*GL_EXT_texture3D*/
    {GL_PACK_SKIP_IMAGES_EXT,                              "GL_PACK_SKIP_IMAGES_EXT"},
    {GL_PACK_IMAGE_HEIGHT_EXT,                             "GL_PACK_IMAGE_HEIGHT_EXT"},
    {GL_UNPACK_SKIP_IMAGES_EXT,                            "GL_UNPACK_SKIP_IMAGES_EXT"},
    {GL_UNPACK_IMAGE_HEIGHT_EXT,                           "GL_UNPACK_IMAGE_HEIGHT_EXT"},
    {GL_TEXTURE_3D_EXT,                                    "GL_TEXTURE_3D_EXT"},
    {GL_PROXY_TEXTURE_3D_EXT,                              "GL_PROXY_TEXTURE_3D_EXT"},
    {GL_TEXTURE_DEPTH_EXT,                                 "GL_TEXTURE_DEPTH_EXT"},
    {GL_TEXTURE_WRAP_R_EXT,                                "GL_TEXTURE_WRAP_R_EXT"},
    {GL_MAX_3D_TEXTURE_SIZE_EXT,                           "GL_MAX_3D_TEXTURE_SIZE_EXT"},


    /*GL_SGIS_texture_filter4*/                            /*GL_SGIS_texture_filter4*/
    {GL_FILTER4_SGIS,                                      "GL_FILTER4_SGIS"},
    {GL_TEXTURE_FILTER4_SIZE_SGIS,                         "GL_TEXTURE_FILTER4_SIZE_SGIS"},


    /*GL_EXT_subtexture*/                                  /*GL_EXT_subtexture*/


    /*GL_EXT_copy_texture*/                                /*GL_EXT_copy_texture*/


    /*GL_EXT_histogram*/                                   /*GL_EXT_histogram*/
    {GL_HISTOGRAM_EXT,                                     "GL_HISTOGRAM_EXT"},
    {GL_PROXY_HISTOGRAM_EXT,                               "GL_PROXY_HISTOGRAM_EXT"},
    {GL_HISTOGRAM_WIDTH_EXT,                               "GL_HISTOGRAM_WIDTH_EXT"},
    {GL_HISTOGRAM_FORMAT_EXT,                              "GL_HISTOGRAM_FORMAT_EXT"},
    {GL_HISTOGRAM_RED_SIZE_EXT,                            "GL_HISTOGRAM_RED_SIZE_EXT"},
    {GL_HISTOGRAM_GREEN_SIZE_EXT,                          "GL_HISTOGRAM_GREEN_SIZE_EXT"},
    {GL_HISTOGRAM_BLUE_SIZE_EXT,                           "GL_HISTOGRAM_BLUE_SIZE_EXT"},
    {GL_HISTOGRAM_ALPHA_SIZE_EXT,                          "GL_HISTOGRAM_ALPHA_SIZE_EXT"},
    {GL_HISTOGRAM_LUMINANCE_SIZE_EXT,                      "GL_HISTOGRAM_LUMINANCE_SIZE_EXT"},
    {GL_HISTOGRAM_SINK_EXT,                                "GL_HISTOGRAM_SINK_EXT"},
    {GL_MINMAX_EXT,                                        "GL_MINMAX_EXT"},
    {GL_MINMAX_FORMAT_EXT,                                 "GL_MINMAX_FORMAT_EXT"},
    {GL_MINMAX_SINK_EXT,                                   "GL_MINMAX_SINK_EXT"},
    {GL_TABLE_TOO_LARGE_EXT,                               "GL_TABLE_TOO_LARGE_EXT"},


    /*GL_EXT_convolution*/                                 /*GL_EXT_convolution*/
    {GL_CONVOLUTION_1D_EXT,                                "GL_CONVOLUTION_1D_EXT"},
    {GL_CONVOLUTION_2D_EXT,                                "GL_CONVOLUTION_2D_EXT"},
    {GL_SEPARABLE_2D_EXT,                                  "GL_SEPARABLE_2D_EXT"},
    {GL_CONVOLUTION_BORDER_MODE_EXT,                       "GL_CONVOLUTION_BORDER_MODE_EXT"},
    {GL_CONVOLUTION_FILTER_SCALE_EXT,                      "GL_CONVOLUTION_FILTER_SCALE_EXT"},
    {GL_CONVOLUTION_FILTER_BIAS_EXT,                       "GL_CONVOLUTION_FILTER_BIAS_EXT"},
    {GL_REDUCE_EXT,                                        "GL_REDUCE_EXT"},
    {GL_CONVOLUTION_FORMAT_EXT,                            "GL_CONVOLUTION_FORMAT_EXT"},
    {GL_CONVOLUTION_WIDTH_EXT,                             "GL_CONVOLUTION_WIDTH_EXT"},
    {GL_CONVOLUTION_HEIGHT_EXT,                            "GL_CONVOLUTION_HEIGHT_EXT"},
    {GL_MAX_CONVOLUTION_WIDTH_EXT,                         "GL_MAX_CONVOLUTION_WIDTH_EXT"},
    {GL_MAX_CONVOLUTION_HEIGHT_EXT,                        "GL_MAX_CONVOLUTION_HEIGHT_EXT"},
    {GL_POST_CONVOLUTION_RED_SCALE_EXT,                    "GL_POST_CONVOLUTION_RED_SCALE_EXT"},
    {GL_POST_CONVOLUTION_GREEN_SCALE_EXT,                  "GL_POST_CONVOLUTION_GREEN_SCALE_EXT"},
    {GL_POST_CONVOLUTION_BLUE_SCALE_EXT,                   "GL_POST_CONVOLUTION_BLUE_SCALE_EXT"},
    {GL_POST_CONVOLUTION_ALPHA_SCALE_EXT,                  "GL_POST_CONVOLUTION_ALPHA_SCALE_EXT"},
    {GL_POST_CONVOLUTION_RED_BIAS_EXT,                     "GL_POST_CONVOLUTION_RED_BIAS_EXT"},
    {GL_POST_CONVOLUTION_GREEN_BIAS_EXT,                   "GL_POST_CONVOLUTION_GREEN_BIAS_EXT"},
    {GL_POST_CONVOLUTION_BLUE_BIAS_EXT,                    "GL_POST_CONVOLUTION_BLUE_BIAS_EXT"},
    {GL_POST_CONVOLUTION_ALPHA_BIAS_EXT,                   "GL_POST_CONVOLUTION_ALPHA_BIAS_EXT"},


    /*GL_SGI_color_matrix*/                                /*GL_SGI_color_matrix*/
    {GL_COLOR_MATRIX_SGI,                                  "GL_COLOR_MATRIX_SGI"},
    {GL_COLOR_MATRIX_STACK_DEPTH_SGI,                      "GL_COLOR_MATRIX_STACK_DEPTH_SGI"},
    {GL_MAX_COLOR_MATRIX_STACK_DEPTH_SGI,                  "GL_MAX_COLOR_MATRIX_STACK_DEPTH_SGI"},
    {GL_POST_COLOR_MATRIX_RED_SCALE_SGI,                   "GL_POST_COLOR_MATRIX_RED_SCALE_SGI"},
    {GL_POST_COLOR_MATRIX_GREEN_SCALE_SGI,                 "GL_POST_COLOR_MATRIX_GREEN_SCALE_SGI"},
    {GL_POST_COLOR_MATRIX_BLUE_SCALE_SGI,                  "GL_POST_COLOR_MATRIX_BLUE_SCALE_SGI"},
    {GL_POST_COLOR_MATRIX_ALPHA_SCALE_SGI,                 "GL_POST_COLOR_MATRIX_ALPHA_SCALE_SGI"},
    {GL_POST_COLOR_MATRIX_RED_BIAS_SGI,                    "GL_POST_COLOR_MATRIX_RED_BIAS_SGI"},
    {GL_POST_COLOR_MATRIX_GREEN_BIAS_SGI,                  "GL_POST_COLOR_MATRIX_GREEN_BIAS_SGI"},
    {GL_POST_COLOR_MATRIX_BLUE_BIAS_SGI,                   "GL_POST_COLOR_MATRIX_BLUE_BIAS_SGI"},
    {GL_POST_COLOR_MATRIX_ALPHA_BIAS_SGI,                  "GL_POST_COLOR_MATRIX_ALPHA_BIAS_SGI"},


    /*GL_SGI_color_table*/                                 /*GL_SGI_color_table*/
    {GL_COLOR_TABLE_SGI,                                   "GL_COLOR_TABLE_SGI"},
    {GL_POST_CONVOLUTION_COLOR_TABLE_SGI,                  "GL_POST_CONVOLUTION_COLOR_TABLE_SGI"},
    {GL_POST_COLOR_MATRIX_COLOR_TABLE_SGI,                 "GL_POST_COLOR_MATRIX_COLOR_TABLE_SGI"},
    {GL_PROXY_COLOR_TABLE_SGI,                             "GL_PROXY_COLOR_TABLE_SGI"},
    {GL_PROXY_POST_CONVOLUTION_COLOR_TABLE_SGI,            "GL_PROXY_POST_CONVOLUTION_COLOR_TABLE_SGI"},
    {GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE_SGI,           "GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE_SGI"},
    {GL_COLOR_TABLE_SCALE_SGI,                             "GL_COLOR_TABLE_SCALE_SGI"},
    {GL_COLOR_TABLE_BIAS_SGI,                              "GL_COLOR_TABLE_BIAS_SGI"},
    {GL_COLOR_TABLE_FORMAT_SGI,                            "GL_COLOR_TABLE_FORMAT_SGI"},
    {GL_COLOR_TABLE_WIDTH_SGI,                             "GL_COLOR_TABLE_WIDTH_SGI"},
    {GL_COLOR_TABLE_RED_SIZE_SGI,                          "GL_COLOR_TABLE_RED_SIZE_SGI"},
    {GL_COLOR_TABLE_GREEN_SIZE_SGI,                        "GL_COLOR_TABLE_GREEN_SIZE_SGI"},
    {GL_COLOR_TABLE_BLUE_SIZE_SGI,                         "GL_COLOR_TABLE_BLUE_SIZE_SGI"},
    {GL_COLOR_TABLE_ALPHA_SIZE_SGI,                        "GL_COLOR_TABLE_ALPHA_SIZE_SGI"},
    {GL_COLOR_TABLE_LUMINANCE_SIZE_SGI,                    "GL_COLOR_TABLE_LUMINANCE_SIZE_SGI"},
    {GL_COLOR_TABLE_INTENSITY_SIZE_SGI,                    "GL_COLOR_TABLE_INTENSITY_SIZE_SGI"},


    /*GL_SGIS_pixel_texture*/                              /*GL_SGIS_pixel_texture*/
    {GL_PIXEL_TEXTURE_SGIS,                                "GL_PIXEL_TEXTURE_SGIS"},
    {GL_PIXEL_FRAGMENT_RGB_SOURCE_SGIS,                    "GL_PIXEL_FRAGMENT_RGB_SOURCE_SGIS"},
    {GL_PIXEL_FRAGMENT_ALPHA_SOURCE_SGIS,                  "GL_PIXEL_FRAGMENT_ALPHA_SOURCE_SGIS"},
    {GL_PIXEL_GROUP_COLOR_SGIS,                            "GL_PIXEL_GROUP_COLOR_SGIS"},


    /*GL_SGIX_pixel_texture*/                              /*GL_SGIX_pixel_texture*/
    {GL_PIXEL_TEX_GEN_SGIX,                                "GL_PIXEL_TEX_GEN_SGIX"},
    {GL_PIXEL_TEX_GEN_MODE_SGIX,                           "GL_PIXEL_TEX_GEN_MODE_SGIX"},


    /*GL_SGIS_texture4D*/                                  /*GL_SGIS_texture4D*/
    {GL_PACK_SKIP_VOLUMES_SGIS,                            "GL_PACK_SKIP_VOLUMES_SGIS"},
    {GL_PACK_IMAGE_DEPTH_SGIS,                             "GL_PACK_IMAGE_DEPTH_SGIS"},
    {GL_UNPACK_SKIP_VOLUMES_SGIS,                          "GL_UNPACK_SKIP_VOLUMES_SGIS"},
    {GL_UNPACK_IMAGE_DEPTH_SGIS,                           "GL_UNPACK_IMAGE_DEPTH_SGIS"},
    {GL_TEXTURE_4D_SGIS,                                   "GL_TEXTURE_4D_SGIS"},
    {GL_PROXY_TEXTURE_4D_SGIS,                             "GL_PROXY_TEXTURE_4D_SGIS"},
    {GL_TEXTURE_4DSIZE_SGIS,                               "GL_TEXTURE_4DSIZE_SGIS"},
    {GL_TEXTURE_WRAP_Q_SGIS,                               "GL_TEXTURE_WRAP_Q_SGIS"},
    {GL_MAX_4D_TEXTURE_SIZE_SGIS,                          "GL_MAX_4D_TEXTURE_SIZE_SGIS"},
    {GL_TEXTURE_4D_BINDING_SGIS,                           "GL_TEXTURE_4D_BINDING_SGIS"},


    /*GL_SGI_texture_color_table*/                         /*GL_SGI_texture_color_table*/
    {GL_TEXTURE_COLOR_TABLE_SGI,                           "GL_TEXTURE_COLOR_TABLE_SGI"},
    {GL_PROXY_TEXTURE_COLOR_TABLE_SGI,                     "GL_PROXY_TEXTURE_COLOR_TABLE_SGI"},


    /*GL_EXT_cmyka*/                                       /*GL_EXT_cmyka*/
    {GL_CMYK_EXT,                                          "GL_CMYK_EXT"},
    {GL_CMYKA_EXT,                                         "GL_CMYKA_EXT"},
    {GL_PACK_CMYK_HINT_EXT,                                "GL_PACK_CMYK_HINT_EXT"},
    {GL_UNPACK_CMYK_HINT_EXT,                              "GL_UNPACK_CMYK_HINT_EXT"},


    /*GL_EXT_texture_object*/                              /*GL_EXT_texture_object*/
    {GL_TEXTURE_PRIORITY_EXT,                              "GL_TEXTURE_PRIORITY_EXT"},
    {GL_TEXTURE_RESIDENT_EXT,                              "GL_TEXTURE_RESIDENT_EXT"},
    {GL_TEXTURE_1D_BINDING_EXT,                            "GL_TEXTURE_1D_BINDING_EXT"},
    {GL_TEXTURE_2D_BINDING_EXT,                            "GL_TEXTURE_2D_BINDING_EXT"},
    {GL_TEXTURE_3D_BINDING_EXT,                            "GL_TEXTURE_3D_BINDING_EXT"},


    /*GL_SGIS_detail_texture*/                             /*GL_SGIS_detail_texture*/
    {GL_DETAIL_TEXTURE_2D_SGIS,                            "GL_DETAIL_TEXTURE_2D_SGIS"},
    {GL_DETAIL_TEXTURE_2D_BINDING_SGIS,                    "GL_DETAIL_TEXTURE_2D_BINDING_SGIS"},
    {GL_LINEAR_DETAIL_SGIS,                                "GL_LINEAR_DETAIL_SGIS"},
    {GL_LINEAR_DETAIL_ALPHA_SGIS,                          "GL_LINEAR_DETAIL_ALPHA_SGIS"},
    {GL_LINEAR_DETAIL_COLOR_SGIS,                          "GL_LINEAR_DETAIL_COLOR_SGIS"},
    {GL_DETAIL_TEXTURE_LEVEL_SGIS,                         "GL_DETAIL_TEXTURE_LEVEL_SGIS"},
    {GL_DETAIL_TEXTURE_MODE_SGIS,                          "GL_DETAIL_TEXTURE_MODE_SGIS"},
    {GL_DETAIL_TEXTURE_FUNC_POINTS_SGIS,                   "GL_DETAIL_TEXTURE_FUNC_POINTS_SGIS"},


    /*GL_SGIS_sharpen_texture*/                            /*GL_SGIS_sharpen_texture*/
    {GL_LINEAR_SHARPEN_SGIS,                               "GL_LINEAR_SHARPEN_SGIS"},
    {GL_LINEAR_SHARPEN_ALPHA_SGIS,                         "GL_LINEAR_SHARPEN_ALPHA_SGIS"},
    {GL_LINEAR_SHARPEN_COLOR_SGIS,                         "GL_LINEAR_SHARPEN_COLOR_SGIS"},
    {GL_SHARPEN_TEXTURE_FUNC_POINTS_SGIS,                  "GL_SHARPEN_TEXTURE_FUNC_POINTS_SGIS"},


    /*GL_EXT_packed_pixels*/                               /*GL_EXT_packed_pixels*/
    {GL_UNSIGNED_BYTE_3_3_2_EXT,                           "GL_UNSIGNED_BYTE_3_3_2_EXT"},
    {GL_UNSIGNED_SHORT_4_4_4_4_EXT,                        "GL_UNSIGNED_SHORT_4_4_4_4_EXT"},
    {GL_UNSIGNED_SHORT_5_5_5_1_EXT,                        "GL_UNSIGNED_SHORT_5_5_5_1_EXT"},
    {GL_UNSIGNED_INT_8_8_8_8_EXT,                          "GL_UNSIGNED_INT_8_8_8_8_EXT"},
    {GL_UNSIGNED_INT_10_10_10_2_EXT,                       "GL_UNSIGNED_INT_10_10_10_2_EXT"},


    /*GL_SGIS_texture_lod*/                                /*GL_SGIS_texture_lod*/
    {GL_TEXTURE_MIN_LOD_SGIS,                              "GL_TEXTURE_MIN_LOD_SGIS"},
    {GL_TEXTURE_MAX_LOD_SGIS,                              "GL_TEXTURE_MAX_LOD_SGIS"},
    {GL_TEXTURE_BASE_LEVEL_SGIS,                           "GL_TEXTURE_BASE_LEVEL_SGIS"},
    {GL_TEXTURE_MAX_LEVEL_SGIS,                            "GL_TEXTURE_MAX_LEVEL_SGIS"},


    /*GL_SGIS_multisample*/                                /*GL_SGIS_multisample*/
    {GL_MULTISAMPLE_SGIS,                                  "GL_MULTISAMPLE_SGIS"},
    {GL_SAMPLE_ALPHA_TO_MASK_SGIS,                         "GL_SAMPLE_ALPHA_TO_MASK_SGIS"},
    {GL_SAMPLE_ALPHA_TO_ONE_SGIS,                          "GL_SAMPLE_ALPHA_TO_ONE_SGIS"},
    {GL_SAMPLE_MASK_SGIS,                                  "GL_SAMPLE_MASK_SGIS"},
    {GL_1PASS_SGIS,                                        "GL_1PASS_SGIS"},
    {GL_2PASS_0_SGIS,                                      "GL_2PASS_0_SGIS"},
    {GL_2PASS_1_SGIS,                                      "GL_2PASS_1_SGIS"},
    {GL_4PASS_0_SGIS,                                      "GL_4PASS_0_SGIS"},
    {GL_4PASS_1_SGIS,                                      "GL_4PASS_1_SGIS"},
    {GL_4PASS_2_SGIS,                                      "GL_4PASS_2_SGIS"},
    {GL_4PASS_3_SGIS,                                      "GL_4PASS_3_SGIS"},
    {GL_SAMPLE_BUFFERS_SGIS,                               "GL_SAMPLE_BUFFERS_SGIS"},
    {GL_SAMPLES_SGIS,                                      "GL_SAMPLES_SGIS"},
    {GL_SAMPLE_MASK_VALUE_SGIS,                            "GL_SAMPLE_MASK_VALUE_SGIS"},
    {GL_SAMPLE_MASK_INVERT_SGIS,                           "GL_SAMPLE_MASK_INVERT_SGIS"},
    {GL_SAMPLE_PATTERN_SGIS,                               "GL_SAMPLE_PATTERN_SGIS"},


    /*GL_EXT_rescale_normal*/                              /*GL_EXT_rescale_normal*/
    {GL_RESCALE_NORMAL_EXT,                                "GL_RESCALE_NORMAL_EXT"},


    /*GL_EXT_vertex_array*/                                /*GL_EXT_vertex_array*/
    {GL_VERTEX_ARRAY_EXT,                                  "GL_VERTEX_ARRAY_EXT"},
    {GL_NORMAL_ARRAY_EXT,                                  "GL_NORMAL_ARRAY_EXT"},
    {GL_COLOR_ARRAY_EXT,                                   "GL_COLOR_ARRAY_EXT"},
    {GL_INDEX_ARRAY_EXT,                                   "GL_INDEX_ARRAY_EXT"},
    {GL_TEXTURE_COORD_ARRAY_EXT,                           "GL_TEXTURE_COORD_ARRAY_EXT"},
    {GL_EDGE_FLAG_ARRAY_EXT,                               "GL_EDGE_FLAG_ARRAY_EXT"},
    {GL_VERTEX_ARRAY_SIZE_EXT,                             "GL_VERTEX_ARRAY_SIZE_EXT"},
    {GL_VERTEX_ARRAY_TYPE_EXT,                             "GL_VERTEX_ARRAY_TYPE_EXT"},
    {GL_VERTEX_ARRAY_STRIDE_EXT,                           "GL_VERTEX_ARRAY_STRIDE_EXT"},
    {GL_VERTEX_ARRAY_COUNT_EXT,                            "GL_VERTEX_ARRAY_COUNT_EXT"},
    {GL_NORMAL_ARRAY_TYPE_EXT,                             "GL_NORMAL_ARRAY_TYPE_EXT"},
    {GL_NORMAL_ARRAY_STRIDE_EXT,                           "GL_NORMAL_ARRAY_STRIDE_EXT"},
    {GL_NORMAL_ARRAY_COUNT_EXT,                            "GL_NORMAL_ARRAY_COUNT_EXT"},
    {GL_COLOR_ARRAY_SIZE_EXT,                              "GL_COLOR_ARRAY_SIZE_EXT"},
    {GL_COLOR_ARRAY_TYPE_EXT,                              "GL_COLOR_ARRAY_TYPE_EXT"},
    {GL_COLOR_ARRAY_STRIDE_EXT,                            "GL_COLOR_ARRAY_STRIDE_EXT"},
    {GL_COLOR_ARRAY_COUNT_EXT,                             "GL_COLOR_ARRAY_COUNT_EXT"},
    {GL_INDEX_ARRAY_TYPE_EXT,                              "GL_INDEX_ARRAY_TYPE_EXT"},
    {GL_INDEX_ARRAY_STRIDE_EXT,                            "GL_INDEX_ARRAY_STRIDE_EXT"},
    {GL_INDEX_ARRAY_COUNT_EXT,                             "GL_INDEX_ARRAY_COUNT_EXT"},
    {GL_TEXTURE_COORD_ARRAY_SIZE_EXT,                      "GL_TEXTURE_COORD_ARRAY_SIZE_EXT"},
    {GL_TEXTURE_COORD_ARRAY_TYPE_EXT,                      "GL_TEXTURE_COORD_ARRAY_TYPE_EXT"},
    {GL_TEXTURE_COORD_ARRAY_STRIDE_EXT,                    "GL_TEXTURE_COORD_ARRAY_STRIDE_EXT"},
    {GL_TEXTURE_COORD_ARRAY_COUNT_EXT,                     "GL_TEXTURE_COORD_ARRAY_COUNT_EXT"},
    {GL_EDGE_FLAG_ARRAY_STRIDE_EXT,                        "GL_EDGE_FLAG_ARRAY_STRIDE_EXT"},
    {GL_EDGE_FLAG_ARRAY_COUNT_EXT,                         "GL_EDGE_FLAG_ARRAY_COUNT_EXT"},
    {GL_VERTEX_ARRAY_POINTER_EXT,                          "GL_VERTEX_ARRAY_POINTER_EXT"},
    {GL_NORMAL_ARRAY_POINTER_EXT,                          "GL_NORMAL_ARRAY_POINTER_EXT"},
    {GL_COLOR_ARRAY_POINTER_EXT,                           "GL_COLOR_ARRAY_POINTER_EXT"},
    {GL_INDEX_ARRAY_POINTER_EXT,                           "GL_INDEX_ARRAY_POINTER_EXT"},
    {GL_TEXTURE_COORD_ARRAY_POINTER_EXT,                   "GL_TEXTURE_COORD_ARRAY_POINTER_EXT"},
    {GL_EDGE_FLAG_ARRAY_POINTER_EXT,                       "GL_EDGE_FLAG_ARRAY_POINTER_EXT"},


    /*GL_EXT_misc_attribute*/                              /*GL_EXT_misc_attribute*/


    /*GL_SGIS_generate_mipmap*/                            /*GL_SGIS_generate_mipmap*/
    {GL_GENERATE_MIPMAP_SGIS,                              "GL_GENERATE_MIPMAP_SGIS"},
    {GL_GENERATE_MIPMAP_HINT_SGIS,                         "GL_GENERATE_MIPMAP_HINT_SGIS"},


    /*GL_SGIX_clipmap*/                                    /*GL_SGIX_clipmap*/
    {GL_LINEAR_CLIPMAP_LINEAR_SGIX,                        "GL_LINEAR_CLIPMAP_LINEAR_SGIX"},
    {GL_TEXTURE_CLIPMAP_CENTER_SGIX,                       "GL_TEXTURE_CLIPMAP_CENTER_SGIX"},
    {GL_TEXTURE_CLIPMAP_FRAME_SGIX,                        "GL_TEXTURE_CLIPMAP_FRAME_SGIX"},
    {GL_TEXTURE_CLIPMAP_OFFSET_SGIX,                       "GL_TEXTURE_CLIPMAP_OFFSET_SGIX"},
    {GL_TEXTURE_CLIPMAP_VIRTUAL_DEPTH_SGIX,                "GL_TEXTURE_CLIPMAP_VIRTUAL_DEPTH_SGIX"},
    {GL_TEXTURE_CLIPMAP_LOD_OFFSET_SGIX,                   "GL_TEXTURE_CLIPMAP_LOD_OFFSET_SGIX"},
    {GL_TEXTURE_CLIPMAP_DEPTH_SGIX,                        "GL_TEXTURE_CLIPMAP_DEPTH_SGIX"},
    {GL_MAX_CLIPMAP_DEPTH_SGIX,                            "GL_MAX_CLIPMAP_DEPTH_SGIX"},
    {GL_MAX_CLIPMAP_VIRTUAL_DEPTH_SGIX,                    "GL_MAX_CLIPMAP_VIRTUAL_DEPTH_SGIX"},
    {GL_NEAREST_CLIPMAP_NEAREST_SGIX,                      "GL_NEAREST_CLIPMAP_NEAREST_SGIX"},
    {GL_NEAREST_CLIPMAP_LINEAR_SGIX,                       "GL_NEAREST_CLIPMAP_LINEAR_SGIX"},
    {GL_LINEAR_CLIPMAP_NEAREST_SGIX,                       "GL_LINEAR_CLIPMAP_NEAREST_SGIX"},


    /*GL_SGIX_shadow*/                                     /*GL_SGIX_shadow*/
    {GL_TEXTURE_COMPARE_SGIX,                              "GL_TEXTURE_COMPARE_SGIX"},
    {GL_TEXTURE_COMPARE_OPERATOR_SGIX,                     "GL_TEXTURE_COMPARE_OPERATOR_SGIX"},
    {GL_TEXTURE_LEQUAL_R_SGIX,                             "GL_TEXTURE_LEQUAL_R_SGIX"},
    {GL_TEXTURE_GEQUAL_R_SGIX,                             "GL_TEXTURE_GEQUAL_R_SGIX"},


    /*GL_SGIS_texture_edge_clamp*/                         /*GL_SGIS_texture_edge_clamp*/
    {GL_CLAMP_TO_EDGE_SGIS,                                "GL_CLAMP_TO_EDGE_SGIS"},


    /*GL_SGIS_texture_border_clamp*/                       /*GL_SGIS_texture_border_clamp*/
    {GL_CLAMP_TO_BORDER_SGIS,                              "GL_CLAMP_TO_BORDER_SGIS"},


    /*GL_EXT_blend_minmax*/                                /*GL_EXT_blend_minmax*/
    {GL_FUNC_ADD_EXT,                                      "GL_FUNC_ADD_EXT"},
    {GL_MIN_EXT,                                           "GL_MIN_EXT"},
    {GL_MAX_EXT,                                           "GL_MAX_EXT"},
    {GL_BLEND_EQUATION_EXT,                                "GL_BLEND_EQUATION_EXT"},


    /*GL_EXT_blend_subtract*/                              /*GL_EXT_blend_subtract*/
    {GL_FUNC_SUBTRACT_EXT,                                 "GL_FUNC_SUBTRACT_EXT"},
    {GL_FUNC_REVERSE_SUBTRACT_EXT,                         "GL_FUNC_REVERSE_SUBTRACT_EXT"},


    /*GL_EXT_blend_logic_op*/                              /*GL_EXT_blend_logic_op*/


    /*GL_SGIX_interlace*/                                  /*GL_SGIX_interlace*/
    {GL_INTERLACE_SGIX,                                    "GL_INTERLACE_SGIX"},


    /*GL_SGIX_pixel_tiles*/                                /*GL_SGIX_pixel_tiles*/
    {GL_PIXEL_TILE_BEST_ALIGNMENT_SGIX,                    "GL_PIXEL_TILE_BEST_ALIGNMENT_SGIX"},
    {GL_PIXEL_TILE_CACHE_INCREMENT_SGIX,                   "GL_PIXEL_TILE_CACHE_INCREMENT_SGIX"},
    {GL_PIXEL_TILE_WIDTH_SGIX,                             "GL_PIXEL_TILE_WIDTH_SGIX"},
    {GL_PIXEL_TILE_HEIGHT_SGIX,                            "GL_PIXEL_TILE_HEIGHT_SGIX"},
    {GL_PIXEL_TILE_GRID_WIDTH_SGIX,                        "GL_PIXEL_TILE_GRID_WIDTH_SGIX"},
    {GL_PIXEL_TILE_GRID_HEIGHT_SGIX,                       "GL_PIXEL_TILE_GRID_HEIGHT_SGIX"},
    {GL_PIXEL_TILE_GRID_DEPTH_SGIX,                        "GL_PIXEL_TILE_GRID_DEPTH_SGIX"},
    {GL_PIXEL_TILE_CACHE_SIZE_SGIX,                        "GL_PIXEL_TILE_CACHE_SIZE_SGIX"},


    /*GL_SGIS_texture_select*/                             /*GL_SGIS_texture_select*/
    {GL_DUAL_ALPHA4_SGIS,                                  "GL_DUAL_ALPHA4_SGIS"},
    {GL_DUAL_ALPHA8_SGIS,                                  "GL_DUAL_ALPHA8_SGIS"},
    {GL_DUAL_ALPHA12_SGIS,                                 "GL_DUAL_ALPHA12_SGIS"},
    {GL_DUAL_ALPHA16_SGIS,                                 "GL_DUAL_ALPHA16_SGIS"},
    {GL_DUAL_LUMINANCE4_SGIS,                              "GL_DUAL_LUMINANCE4_SGIS"},
    {GL_DUAL_LUMINANCE8_SGIS,                              "GL_DUAL_LUMINANCE8_SGIS"},
    {GL_DUAL_LUMINANCE12_SGIS,                             "GL_DUAL_LUMINANCE12_SGIS"},
    {GL_DUAL_LUMINANCE16_SGIS,                             "GL_DUAL_LUMINANCE16_SGIS"},
    {GL_DUAL_INTENSITY4_SGIS,                              "GL_DUAL_INTENSITY4_SGIS"},
    {GL_DUAL_INTENSITY8_SGIS,                              "GL_DUAL_INTENSITY8_SGIS"},
    {GL_DUAL_INTENSITY12_SGIS,                             "GL_DUAL_INTENSITY12_SGIS"},
    {GL_DUAL_INTENSITY16_SGIS,                             "GL_DUAL_INTENSITY16_SGIS"},
    {GL_DUAL_LUMINANCE_ALPHA4_SGIS,                        "GL_DUAL_LUMINANCE_ALPHA4_SGIS"},
    {GL_DUAL_LUMINANCE_ALPHA8_SGIS,                        "GL_DUAL_LUMINANCE_ALPHA8_SGIS"},
    {GL_QUAD_ALPHA4_SGIS,                                  "GL_QUAD_ALPHA4_SGIS"},
    {GL_QUAD_ALPHA8_SGIS,                                  "GL_QUAD_ALPHA8_SGIS"},
    {GL_QUAD_LUMINANCE4_SGIS,                              "GL_QUAD_LUMINANCE4_SGIS"},
    {GL_QUAD_LUMINANCE8_SGIS,                              "GL_QUAD_LUMINANCE8_SGIS"},
    {GL_QUAD_INTENSITY4_SGIS,                              "GL_QUAD_INTENSITY4_SGIS"},
    {GL_QUAD_INTENSITY8_SGIS,                              "GL_QUAD_INTENSITY8_SGIS"},
    {GL_DUAL_TEXTURE_SELECT_SGIS,                          "GL_DUAL_TEXTURE_SELECT_SGIS"},
    {GL_QUAD_TEXTURE_SELECT_SGIS,                          "GL_QUAD_TEXTURE_SELECT_SGIS"},


    /*GL_SGIX_sprite*/                                     /*GL_SGIX_sprite*/
    {GL_SPRITE_SGIX,                                       "GL_SPRITE_SGIX"},
    {GL_SPRITE_MODE_SGIX,                                  "GL_SPRITE_MODE_SGIX"},
    {GL_SPRITE_AXIS_SGIX,                                  "GL_SPRITE_AXIS_SGIX"},
    {GL_SPRITE_TRANSLATION_SGIX,                           "GL_SPRITE_TRANSLATION_SGIX"},
    {GL_SPRITE_AXIAL_SGIX,                                 "GL_SPRITE_AXIAL_SGIX"},
    {GL_SPRITE_OBJECT_ALIGNED_SGIX,                        "GL_SPRITE_OBJECT_ALIGNED_SGIX"},
    {GL_SPRITE_EYE_ALIGNED_SGIX,                           "GL_SPRITE_EYE_ALIGNED_SGIX"},


    /*GL_SGIX_texture_multi_buffer*/                       /*GL_SGIX_texture_multi_buffer*/
    {GL_TEXTURE_MULTI_BUFFER_HINT_SGIX,                    "GL_TEXTURE_MULTI_BUFFER_HINT_SGIX"},


    /*GL_EXT_point_parameters*/                            /*GL_EXT_point_parameters*/
    {GL_POINT_SIZE_MIN_EXT,                                "GL_POINT_SIZE_MIN_EXT"},
    {GL_POINT_SIZE_MAX_EXT,                                "GL_POINT_SIZE_MAX_EXT"},
    {GL_POINT_FADE_THRESHOLD_SIZE_EXT,                     "GL_POINT_FADE_THRESHOLD_SIZE_EXT"},
    {GL_DISTANCE_ATTENUATION_EXT,                          "GL_DISTANCE_ATTENUATION_EXT"},


    /*GL_SGIS_point_parameters*/                           /*GL_SGIS_point_parameters*/
    {GL_POINT_SIZE_MIN_SGIS,                               "GL_POINT_SIZE_MIN_SGIS"},
    {GL_POINT_SIZE_MAX_SGIS,                               "GL_POINT_SIZE_MAX_SGIS"},
    {GL_POINT_FADE_THRESHOLD_SIZE_SGIS,                    "GL_POINT_FADE_THRESHOLD_SIZE_SGIS"},
    {GL_DISTANCE_ATTENUATION_SGIS,                         "GL_DISTANCE_ATTENUATION_SGIS"},


    /*GL_SGIX_instruments*/                                /*GL_SGIX_instruments*/
    {GL_INSTRUMENT_BUFFER_POINTER_SGIX,                    "GL_INSTRUMENT_BUFFER_POINTER_SGIX"},
    {GL_INSTRUMENT_MEASUREMENTS_SGIX,                      "GL_INSTRUMENT_MEASUREMENTS_SGIX"},


    /*GL_SGIX_texture_scale_bias*/                         /*GL_SGIX_texture_scale_bias*/
    {GL_POST_TEXTURE_FILTER_BIAS_SGIX,                     "GL_POST_TEXTURE_FILTER_BIAS_SGIX"},
    {GL_POST_TEXTURE_FILTER_SCALE_SGIX,                    "GL_POST_TEXTURE_FILTER_SCALE_SGIX"},
    {GL_POST_TEXTURE_FILTER_BIAS_RANGE_SGIX,               "GL_POST_TEXTURE_FILTER_BIAS_RANGE_SGIX"},
    {GL_POST_TEXTURE_FILTER_SCALE_RANGE_SGIX,              "GL_POST_TEXTURE_FILTER_SCALE_RANGE_SGIX"},


    /*GL_SGIX_framezoom*/                                  /*GL_SGIX_framezoom*/
    {GL_FRAMEZOOM_SGIX,                                    "GL_FRAMEZOOM_SGIX"},
    {GL_FRAMEZOOM_FACTOR_SGIX,                             "GL_FRAMEZOOM_FACTOR_SGIX"},
    {GL_MAX_FRAMEZOOM_FACTOR_SGIX,                         "GL_MAX_FRAMEZOOM_FACTOR_SGIX"},


    /*GL_SGIX_tag_sample_buffer*/                          /*GL_SGIX_tag_sample_buffer*/


    /*GL_SGIX_polynomial_ffd*/                             /*GL_SGIX_polynomial_ffd*/
    {GL_GEOMETRY_DEFORMATION_SGIX,                         "GL_GEOMETRY_DEFORMATION_SGIX"},
    {GL_TEXTURE_DEFORMATION_SGIX,                          "GL_TEXTURE_DEFORMATION_SGIX"},
    {GL_DEFORMATIONS_MASK_SGIX,                            "GL_DEFORMATIONS_MASK_SGIX"},
    {GL_MAX_DEFORMATION_ORDER_SGIX,                        "GL_MAX_DEFORMATION_ORDER_SGIX"},


    /*GL_SGIX_reference_plane*/                            /*GL_SGIX_reference_plane*/
    {GL_REFERENCE_PLANE_SGIX,                              "GL_REFERENCE_PLANE_SGIX"},
    {GL_REFERENCE_PLANE_EQUATION_SGIX,                     "GL_REFERENCE_PLANE_EQUATION_SGIX"},


    /*GL_SGIX_flush_raster*/                               /*GL_SGIX_flush_raster*/


    /*GL_SGIX_depth_texture*/                              /*GL_SGIX_depth_texture*/
    {GL_DEPTH_COMPONENT16_SGIX,                            "GL_DEPTH_COMPONENT16_SGIX"},
    {GL_DEPTH_COMPONENT24_SGIX,                            "GL_DEPTH_COMPONENT24_SGIX"},
    {GL_DEPTH_COMPONENT32_SGIX,                            "GL_DEPTH_COMPONENT32_SGIX"},


    /*GL_SGIS_fog_function*/                               /*GL_SGIS_fog_function*/
    {GL_FOG_FUNC_SGIS,                                     "GL_FOG_FUNC_SGIS"},
    {GL_FOG_FUNC_POINTS_SGIS,                              "GL_FOG_FUNC_POINTS_SGIS"},
    {GL_MAX_FOG_FUNC_POINTS_SGIS,                          "GL_MAX_FOG_FUNC_POINTS_SGIS"},


    /*GL_SGIX_fog_offset*/                                 /*GL_SGIX_fog_offset*/
    {GL_FOG_OFFSET_SGIX,                                   "GL_FOG_OFFSET_SGIX"},
    {GL_FOG_OFFSET_VALUE_SGIX,                             "GL_FOG_OFFSET_VALUE_SGIX"},


    /*GL_HP_image_transform*/                              /*GL_HP_image_transform*/
    {GL_IMAGE_SCALE_X_HP,                                  "GL_IMAGE_SCALE_X_HP"},
    {GL_IMAGE_SCALE_Y_HP,                                  "GL_IMAGE_SCALE_Y_HP"},
    {GL_IMAGE_TRANSLATE_X_HP,                              "GL_IMAGE_TRANSLATE_X_HP"},
    {GL_IMAGE_TRANSLATE_Y_HP,                              "GL_IMAGE_TRANSLATE_Y_HP"},
    {GL_IMAGE_ROTATE_ANGLE_HP,                             "GL_IMAGE_ROTATE_ANGLE_HP"},
    {GL_IMAGE_ROTATE_ORIGIN_X_HP,                          "GL_IMAGE_ROTATE_ORIGIN_X_HP"},
    {GL_IMAGE_ROTATE_ORIGIN_Y_HP,                          "GL_IMAGE_ROTATE_ORIGIN_Y_HP"},
    {GL_IMAGE_MAG_FILTER_HP,                               "GL_IMAGE_MAG_FILTER_HP"},
    {GL_IMAGE_MIN_FILTER_HP,                               "GL_IMAGE_MIN_FILTER_HP"},
    {GL_IMAGE_CUBIC_WEIGHT_HP,                             "GL_IMAGE_CUBIC_WEIGHT_HP"},
    {GL_CUBIC_HP,                                          "GL_CUBIC_HP"},
    {GL_AVERAGE_HP,                                        "GL_AVERAGE_HP"},
    {GL_IMAGE_TRANSFORM_2D_HP,                             "GL_IMAGE_TRANSFORM_2D_HP"},
    {GL_POST_IMAGE_TRANSFORM_COLOR_TABLE_HP,               "GL_POST_IMAGE_TRANSFORM_COLOR_TABLE_HP"},
    {GL_PROXY_POST_IMAGE_TRANSFORM_COLOR_TABLE_HP,         "GL_PROXY_POST_IMAGE_TRANSFORM_COLOR_TABLE_HP"},


    /*GL_HP_convolution_border_modes*/                     /*GL_HP_convolution_border_modes*/
    {GL_IGNORE_BORDER_HP,                                  "GL_IGNORE_BORDER_HP"},
    {GL_CONSTANT_BORDER_HP,                                "GL_CONSTANT_BORDER_HP"},
    {GL_REPLICATE_BORDER_HP,                               "GL_REPLICATE_BORDER_HP"},
    {GL_CONVOLUTION_BORDER_COLOR_HP,                       "GL_CONVOLUTION_BORDER_COLOR_HP"},


    /*GL_INGR_palette_buffer*/                             /*GL_INGR_palette_buffer*/


    /*GL_SGIX_texture_add_env*/                            /*GL_SGIX_texture_add_env*/
    {GL_TEXTURE_ENV_BIAS_SGIX,                             "GL_TEXTURE_ENV_BIAS_SGIX"},


    /*GL_EXT_color_subtable*/                              /*GL_EXT_color_subtable*/


    /*GL_PGI_vertex_hints*/                                /*GL_PGI_vertex_hints*/
    {GL_VERTEX_DATA_HINT_PGI,                              "GL_VERTEX_DATA_HINT_PGI"},
    {GL_VERTEX_CONSISTENT_HINT_PGI,                        "GL_VERTEX_CONSISTENT_HINT_PGI"},
    {GL_MATERIAL_SIDE_HINT_PGI,                            "GL_MATERIAL_SIDE_HINT_PGI"},
    {GL_MAX_VERTEX_HINT_PGI,                               "GL_MAX_VERTEX_HINT_PGI"},

    /*GL_PGI_misc_hints*/                                  /*GL_PGI_misc_hints*/
    {GL_PREFER_DOUBLEBUFFER_HINT_PGI,                      "GL_PREFER_DOUBLEBUFFER_HINT_PGI"},
    {GL_CONSERVE_MEMORY_HINT_PGI,                          "GL_CONSERVE_MEMORY_HINT_PGI"},
    {GL_RECLAIM_MEMORY_HINT_PGI,                           "GL_RECLAIM_MEMORY_HINT_PGI"},
    {GL_NATIVE_GRAPHICS_HANDLE_PGI,                        "GL_NATIVE_GRAPHICS_HANDLE_PGI"},
    {GL_NATIVE_GRAPHICS_BEGIN_HINT_PGI,                    "GL_NATIVE_GRAPHICS_BEGIN_HINT_PGI"},
    {GL_NATIVE_GRAPHICS_END_HINT_PGI,                      "GL_NATIVE_GRAPHICS_END_HINT_PGI"},
    {GL_ALWAYS_FAST_HINT_PGI,                              "GL_ALWAYS_FAST_HINT_PGI"},
    {GL_ALWAYS_SOFT_HINT_PGI,                              "GL_ALWAYS_SOFT_HINT_PGI"},
    {GL_ALLOW_DRAW_OBJ_HINT_PGI,                           "GL_ALLOW_DRAW_OBJ_HINT_PGI"},
    {GL_ALLOW_DRAW_WIN_HINT_PGI,                           "GL_ALLOW_DRAW_WIN_HINT_PGI"},
    {GL_ALLOW_DRAW_FRG_HINT_PGI,                           "GL_ALLOW_DRAW_FRG_HINT_PGI"},
    {GL_ALLOW_DRAW_MEM_HINT_PGI,                           "GL_ALLOW_DRAW_MEM_HINT_PGI"},
    {GL_STRICT_DEPTHFUNC_HINT_PGI,                         "GL_STRICT_DEPTHFUNC_HINT_PGI"},
    {GL_STRICT_LIGHTING_HINT_PGI,                          "GL_STRICT_LIGHTING_HINT_PGI"},
    {GL_STRICT_SCISSOR_HINT_PGI,                           "GL_STRICT_SCISSOR_HINT_PGI"},
    {GL_FULL_STIPPLE_HINT_PGI,                             "GL_FULL_STIPPLE_HINT_PGI"},
    {GL_CLIP_NEAR_HINT_PGI,                                "GL_CLIP_NEAR_HINT_PGI"},
    {GL_CLIP_FAR_HINT_PGI,                                 "GL_CLIP_FAR_HINT_PGI"},
    {GL_WIDE_LINE_HINT_PGI,                                "GL_WIDE_LINE_HINT_PGI"},
    {GL_BACK_NORMALS_HINT_PGI,                             "GL_BACK_NORMALS_HINT_PGI"},


    /*GL_EXT_paletted_texture*/                            /*GL_EXT_paletted_texture*/
    {GL_COLOR_INDEX1_EXT,                                  "GL_COLOR_INDEX1_EXT"},
    {GL_COLOR_INDEX2_EXT,                                  "GL_COLOR_INDEX2_EXT"},
    {GL_COLOR_INDEX4_EXT,                                  "GL_COLOR_INDEX4_EXT"},
    {GL_COLOR_INDEX8_EXT,                                  "GL_COLOR_INDEX8_EXT"},
    {GL_COLOR_INDEX12_EXT,                                 "GL_COLOR_INDEX12_EXT"},
    {GL_COLOR_INDEX16_EXT,                                 "GL_COLOR_INDEX16_EXT"},
    {GL_TEXTURE_INDEX_SIZE_EXT,                            "GL_TEXTURE_INDEX_SIZE_EXT"},


    /*GL_EXT_clip_volume_hint*/                            /*GL_EXT_clip_volume_hint*/
    {GL_CLIP_VOLUME_CLIPPING_HINT_EXT,                     "GL_CLIP_VOLUME_CLIPPING_HINT_EXT"},


    /*GL_SGIX_list_priority*/                              /*GL_SGIX_list_priority*/
    {GL_LIST_PRIORITY_SGIX,                                "GL_LIST_PRIORITY_SGIX"},


    /*GL_SGIX_ir_instrument1*/                             /*GL_SGIX_ir_instrument1*/
    {GL_IR_INSTRUMENT1_SGIX,                               "GL_IR_INSTRUMENT1_SGIX"},


    /*GL_SGIX_calligraphic_fragment*/                      /*GL_SGIX_calligraphic_fragment*/
    {GL_CALLIGRAPHIC_FRAGMENT_SGIX,                        "GL_CALLIGRAPHIC_FRAGMENT_SGIX"},


    /*GL_SGIX_texture_lod_bias*/                           /*GL_SGIX_texture_lod_bias*/
    {GL_TEXTURE_LOD_BIAS_S_SGIX,                           "GL_TEXTURE_LOD_BIAS_S_SGIX"},
    {GL_TEXTURE_LOD_BIAS_T_SGIX,                           "GL_TEXTURE_LOD_BIAS_T_SGIX"},
    {GL_TEXTURE_LOD_BIAS_R_SGIX,                           "GL_TEXTURE_LOD_BIAS_R_SGIX"},


    /*GL_SGIX_shadow_ambient*/                             /*GL_SGIX_shadow_ambient*/
    {GL_SHADOW_AMBIENT_SGIX,                               "GL_SHADOW_AMBIENT_SGIX"},


    /*GL_EXT_index_texture*/                               /*GL_EXT_index_texture*/


    /*GL_EXT_index_material*/                              /*GL_EXT_index_material*/
    {GL_INDEX_MATERIAL_EXT,                                "GL_INDEX_MATERIAL_EXT"},
    {GL_INDEX_MATERIAL_PARAMETER_EXT,                      "GL_INDEX_MATERIAL_PARAMETER_EXT"},
    {GL_INDEX_MATERIAL_FACE_EXT,                           "GL_INDEX_MATERIAL_FACE_EXT"},


    /*GL_EXT_index_func*/                                  /*GL_EXT_index_func*/
    {GL_INDEX_TEST_EXT,                                    "GL_INDEX_TEST_EXT"},
    {GL_INDEX_TEST_FUNC_EXT,                               "GL_INDEX_TEST_FUNC_EXT"},
    {GL_INDEX_TEST_REF_EXT,                                "GL_INDEX_TEST_REF_EXT"},


    /*GL_EXT_index_array_formats*/                         /*GL_EXT_index_array_formats*/
    {GL_IUI_V2F_EXT,                                       "GL_IUI_V2F_EXT"},
    {GL_IUI_V3F_EXT,                                       "GL_IUI_V3F_EXT"},
    {GL_IUI_N3F_V2F_EXT,                                   "GL_IUI_N3F_V2F_EXT"},
    {GL_IUI_N3F_V3F_EXT,                                   "GL_IUI_N3F_V3F_EXT"},
    {GL_T2F_IUI_V2F_EXT,                                   "GL_T2F_IUI_V2F_EXT"},
    {GL_T2F_IUI_V3F_EXT,                                   "GL_T2F_IUI_V3F_EXT"},
    {GL_T2F_IUI_N3F_V2F_EXT,                               "GL_T2F_IUI_N3F_V2F_EXT"},
    {GL_T2F_IUI_N3F_V3F_EXT,                               "GL_T2F_IUI_N3F_V3F_EXT"},

    /*GL_EXT_cull_vertex*/                                 /*GL_EXT_cull_vertex*/
    {GL_CULL_VERTEX_EXT,                                   "GL_CULL_VERTEX_EXT"},
    {GL_CULL_VERTEX_EYE_POSITION_EXT,                      "GL_CULL_VERTEX_EYE_POSITION_EXT"},
    {GL_CULL_VERTEX_OBJECT_POSITION_EXT,                   "GL_CULL_VERTEX_OBJECT_POSITION_EXT"},


    /*GL_SGIX_ycrcb*/                                      /*GL_SGIX_ycrcb*/
    {GL_YCRCB_422_SGIX,                                    "GL_YCRCB_422_SGIX"},
    {GL_YCRCB_444_SGIX,                                    "GL_YCRCB_444_SGIX"},


    /*GL_SGIX_fragment_lighting*/                          /*GL_SGIX_fragment_lighting*/
    {GL_FRAGMENT_LIGHTING_SGIX,                            "GL_FRAGMENT_LIGHTING_SGIX"},
    {GL_FRAGMENT_COLOR_MATERIAL_SGIX,                      "GL_FRAGMENT_COLOR_MATERIAL_SGIX"},
    {GL_FRAGMENT_COLOR_MATERIAL_FACE_SGIX,                 "GL_FRAGMENT_COLOR_MATERIAL_FACE_SGIX"},
    {GL_FRAGMENT_COLOR_MATERIAL_PARAMETER_SGIX,            "GL_FRAGMENT_COLOR_MATERIAL_PARAMETER_SGIX"},
    {GL_MAX_FRAGMENT_LIGHTS_SGIX,                          "GL_MAX_FRAGMENT_LIGHTS_SGIX"},
    {GL_MAX_ACTIVE_LIGHTS_SGIX,                            "GL_MAX_ACTIVE_LIGHTS_SGIX"},
    {GL_CURRENT_RASTER_NORMAL_SGIX,                        "GL_CURRENT_RASTER_NORMAL_SGIX"},
    {GL_LIGHT_ENV_MODE_SGIX,                               "GL_LIGHT_ENV_MODE_SGIX"},
    {GL_FRAGMENT_LIGHT_MODEL_LOCAL_VIEWER_SGIX,            "GL_FRAGMENT_LIGHT_MODEL_LOCAL_VIEWER_SGIX"},
    {GL_FRAGMENT_LIGHT_MODEL_TWO_SIDE_SGIX,                "GL_FRAGMENT_LIGHT_MODEL_TWO_SIDE_SGIX"},
    {GL_FRAGMENT_LIGHT_MODEL_AMBIENT_SGIX,                 "GL_FRAGMENT_LIGHT_MODEL_AMBIENT_SGIX"},
    {GL_FRAGMENT_LIGHT_MODEL_NORMAL_INTERPOLATION_SGIX,    "GL_FRAGMENT_LIGHT_MODEL_NORMAL_INTERPOLATION_SGIX"},
    {GL_FRAGMENT_LIGHT0_SGIX,                              "GL_FRAGMENT_LIGHT0_SGIX"},
    {GL_FRAGMENT_LIGHT1_SGIX,                              "GL_FRAGMENT_LIGHT1_SGIX"},
    {GL_FRAGMENT_LIGHT2_SGIX,                              "GL_FRAGMENT_LIGHT2_SGIX"},
    {GL_FRAGMENT_LIGHT3_SGIX,                              "GL_FRAGMENT_LIGHT3_SGIX"},
    {GL_FRAGMENT_LIGHT4_SGIX,                              "GL_FRAGMENT_LIGHT4_SGIX"},
    {GL_FRAGMENT_LIGHT5_SGIX,                              "GL_FRAGMENT_LIGHT5_SGIX"},
    {GL_FRAGMENT_LIGHT6_SGIX,                              "GL_FRAGMENT_LIGHT6_SGIX"},
    {GL_FRAGMENT_LIGHT7_SGIX,                              "GL_FRAGMENT_LIGHT7_SGIX"},


    /*GL_IBM_rasterpos_clip*/                              /*GL_IBM_rasterpos_clip*/
    {GL_RASTER_POSITION_UNCLIPPED_IBM,                     "GL_RASTER_POSITION_UNCLIPPED_IBM"},


    /*GL_HP_texture_lighting*/                             /*GL_HP_texture_lighting*/
    {GL_TEXTURE_LIGHTING_MODE_HP,                          "GL_TEXTURE_LIGHTING_MODE_HP"},
    {GL_TEXTURE_POST_SPECULAR_HP,                          "GL_TEXTURE_POST_SPECULAR_HP"},
    {GL_TEXTURE_PRE_SPECULAR_HP,                           "GL_TEXTURE_PRE_SPECULAR_HP"},


    /*GL_EXT_draw_range_elements*/                         /*GL_EXT_draw_range_elements*/
    {GL_MAX_ELEMENTS_VERTICES_EXT,                         "GL_MAX_ELEMENTS_VERTICES_EXT"},
    {GL_MAX_ELEMENTS_INDICES_EXT,                          "GL_MAX_ELEMENTS_INDICES_EXT"},


    /*GL_WIN_phong_shading*/                               /*GL_WIN_phong_shading*/
    {GL_PHONG_WIN,                                         "GL_PHONG_WIN"},
    {GL_PHONG_HINT_WIN,                                    "GL_PHONG_HINT_WIN"},


    /*GL_WIN_specular_fog*/                                /*GL_WIN_specular_fog*/
    {GL_FOG_SPECULAR_TEXTURE_WIN,                          "GL_FOG_SPECULAR_TEXTURE_WIN"},


    /*GL_EXT_light_texture*/                               /*GL_EXT_light_texture*/
    {GL_FRAGMENT_MATERIAL_EXT,                             "GL_FRAGMENT_MATERIAL_EXT"},
    {GL_FRAGMENT_NORMAL_EXT,                               "GL_FRAGMENT_NORMAL_EXT"},
    {GL_FRAGMENT_COLOR_EXT,                                "GL_FRAGMENT_COLOR_EXT"},
    {GL_ATTENUATION_EXT,                                   "GL_ATTENUATION_EXT"},
    {GL_SHADOW_ATTENUATION_EXT,                            "GL_SHADOW_ATTENUATION_EXT"},
    {GL_TEXTURE_APPLICATION_MODE_EXT,                      "GL_TEXTURE_APPLICATION_MODE_EXT"},
    {GL_TEXTURE_LIGHT_EXT,                                 "GL_TEXTURE_LIGHT_EXT"},
    {GL_TEXTURE_MATERIAL_FACE_EXT,                         "GL_TEXTURE_MATERIAL_FACE_EXT"},
    {GL_TEXTURE_MATERIAL_PARAMETER_EXT,                    "GL_TEXTURE_MATERIAL_PARAMETER_EXT"},
    /*reuseGL_FRAGMENT_DEPTH_EXT*/                         /*reuseGL_FRAGMENT_DEPTH_EXT*/


    /*GL_SGIX_blend_alpha_minmax*/                         /*GL_SGIX_blend_alpha_minmax*/
    {GL_ALPHA_MIN_SGIX,                                    "GL_ALPHA_MIN_SGIX"},
    {GL_ALPHA_MAX_SGIX,                                    "GL_ALPHA_MAX_SGIX"},


    /*GL_SGIX_impact_pixel_texture*/                       /*GL_SGIX_impact_pixel_texture*/
#ifdef GL_SGIX_impact_pixel_texture
    {GL_PIXEL_TEX_GEN_Q_CEILING_SGIX,                      "GL_PIXEL_TEX_GEN_Q_CEILING_SGIX"},
    {GL_PIXEL_TEX_GEN_Q_ROUND_SGIX,                        "GL_PIXEL_TEX_GEN_Q_ROUND_SGIX"},
    {GL_PIXEL_TEX_GEN_Q_FLOOR_SGIX,                        "GL_PIXEL_TEX_GEN_Q_FLOOR_SGIX"},
    {GL_PIXEL_TEX_GEN_ALPHA_REPLACE_SGIX,                  "GL_PIXEL_TEX_GEN_ALPHA_REPLACE_SGIX"},
    {GL_PIXEL_TEX_GEN_ALPHA_NO_REPLACE_SGIX,               "GL_PIXEL_TEX_GEN_ALPHA_NO_REPLACE_SGIX"},
    {GL_PIXEL_TEX_GEN_ALPHA_LS_SGIX,                       "GL_PIXEL_TEX_GEN_ALPHA_LS_SGIX"},
    {GL_PIXEL_TEX_GEN_ALPHA_MS_SGIX,                       "GL_PIXEL_TEX_GEN_ALPHA_MS_SGIX"},
#endif


    /*GL_EXT_bgra*/                                        /*GL_EXT_bgra*/
    {GL_BGR_EXT,                                           "GL_BGR_EXT"},
    {GL_BGRA_EXT,                                          "GL_BGRA_EXT"},


    /*GL_SGIX_async*/                                      /*GL_SGIX_async*/
    {GL_ASYNC_MARKER_SGIX,                                 "GL_ASYNC_MARKER_SGIX"},


    /*GL_SGIX_async_pixel*/                                /*GL_SGIX_async_pixel*/
    {GL_ASYNC_TEX_IMAGE_SGIX,                              "GL_ASYNC_TEX_IMAGE_SGIX"},
    {GL_ASYNC_DRAW_PIXELS_SGIX,                            "GL_ASYNC_DRAW_PIXELS_SGIX"},
    {GL_ASYNC_READ_PIXELS_SGIX,                            "GL_ASYNC_READ_PIXELS_SGIX"},
    {GL_MAX_ASYNC_TEX_IMAGE_SGIX,                          "GL_MAX_ASYNC_TEX_IMAGE_SGIX"},
    {GL_MAX_ASYNC_DRAW_PIXELS_SGIX,                        "GL_MAX_ASYNC_DRAW_PIXELS_SGIX"},
    {GL_MAX_ASYNC_READ_PIXELS_SGIX,                        "GL_MAX_ASYNC_READ_PIXELS_SGIX"},


    /*GL_SGIX_async_histogram*/                            /*GL_SGIX_async_histogram*/
    {GL_ASYNC_HISTOGRAM_SGIX,                              "GL_ASYNC_HISTOGRAM_SGIX"},
    {GL_MAX_ASYNC_HISTOGRAM_SGIX,                          "GL_MAX_ASYNC_HISTOGRAM_SGIX"},


    /*GL_INTEL_texture_scissor*/                           /*GL_INTEL_texture_scissor*/


    /*GL_INTEL_parallel_arrays*/                           /*GL_INTEL_parallel_arrays*/
    {GL_PARALLEL_ARRAYS_INTEL,                             "GL_PARALLEL_ARRAYS_INTEL"},
    {GL_VERTEX_ARRAY_PARALLEL_POINTERS_INTEL,              "GL_VERTEX_ARRAY_PARALLEL_POINTERS_INTEL"},
    {GL_NORMAL_ARRAY_PARALLEL_POINTERS_INTEL,              "GL_NORMAL_ARRAY_PARALLEL_POINTERS_INTEL"},
    {GL_COLOR_ARRAY_PARALLEL_POINTERS_INTEL,               "GL_COLOR_ARRAY_PARALLEL_POINTERS_INTEL"},
    {GL_TEXTURE_COORD_ARRAY_PARALLEL_POINTERS_INTEL,       "GL_TEXTURE_COORD_ARRAY_PARALLEL_POINTERS_INTEL"},


    /*GL_HP_occlusion_test*/                               /*GL_HP_occlusion_test*/
    {GL_OCCLUSION_TEST_HP,                                 "GL_OCCLUSION_TEST_HP"},
    {GL_OCCLUSION_TEST_RESULT_HP,                          "GL_OCCLUSION_TEST_RESULT_HP"},


    /*GL_EXT_pixel_transform*/                             /*GL_EXT_pixel_transform*/
    {GL_PIXEL_TRANSFORM_2D_EXT,                            "GL_PIXEL_TRANSFORM_2D_EXT"},
    {GL_PIXEL_MAG_FILTER_EXT,                              "GL_PIXEL_MAG_FILTER_EXT"},
    {GL_PIXEL_MIN_FILTER_EXT,                              "GL_PIXEL_MIN_FILTER_EXT"},
    {GL_PIXEL_CUBIC_WEIGHT_EXT,                            "GL_PIXEL_CUBIC_WEIGHT_EXT"},
    {GL_CUBIC_EXT,                                         "GL_CUBIC_EXT"},
    {GL_AVERAGE_EXT,                                       "GL_AVERAGE_EXT"},
    {GL_PIXEL_TRANSFORM_2D_STACK_DEPTH_EXT,                "GL_PIXEL_TRANSFORM_2D_STACK_DEPTH_EXT"},
    {GL_MAX_PIXEL_TRANSFORM_2D_STACK_DEPTH_EXT,            "GL_MAX_PIXEL_TRANSFORM_2D_STACK_DEPTH_EXT"},
    {GL_PIXEL_TRANSFORM_2D_MATRIX_EXT,                     "GL_PIXEL_TRANSFORM_2D_MATRIX_EXT"},


    /*GL_EXT_pixel_transform_color_table*/                 /*GL_EXT_pixel_transform_color_table*/


    /*GL_EXT_shared_texture_palette*/                      /*GL_EXT_shared_texture_palette*/
    {GL_SHARED_TEXTURE_PALETTE_EXT,                        "GL_SHARED_TEXTURE_PALETTE_EXT"},


    /*GL_EXT_separate_specular_color*/                     /*GL_EXT_separate_specular_color*/
    {GL_LIGHT_MODEL_COLOR_CONTROL_EXT,                     "GL_LIGHT_MODEL_COLOR_CONTROL_EXT"},
    {GL_SINGLE_COLOR_EXT,                                  "GL_SINGLE_COLOR_EXT"},
    {GL_SEPARATE_SPECULAR_COLOR_EXT,                       "GL_SEPARATE_SPECULAR_COLOR_EXT"},


    /*GL_EXT_secondary_color*/                             /*GL_EXT_secondary_color*/
    {GL_COLOR_SUM_EXT,                                     "GL_COLOR_SUM_EXT"},
    {GL_CURRENT_SECONDARY_COLOR_EXT,                       "GL_CURRENT_SECONDARY_COLOR_EXT"},
    {GL_SECONDARY_COLOR_ARRAY_SIZE_EXT,                    "GL_SECONDARY_COLOR_ARRAY_SIZE_EXT"},
    {GL_SECONDARY_COLOR_ARRAY_TYPE_EXT,                    "GL_SECONDARY_COLOR_ARRAY_TYPE_EXT"},
    {GL_SECONDARY_COLOR_ARRAY_STRIDE_EXT,                  "GL_SECONDARY_COLOR_ARRAY_STRIDE_EXT"},
    {GL_SECONDARY_COLOR_ARRAY_POINTER_EXT,                 "GL_SECONDARY_COLOR_ARRAY_POINTER_EXT"},
    {GL_SECONDARY_COLOR_ARRAY_EXT,                         "GL_SECONDARY_COLOR_ARRAY_EXT"},


    /*GL_EXT_texture_perturb_normal*/                      /*GL_EXT_texture_perturb_normal*/
    {GL_PERTURB_EXT,                                       "GL_PERTURB_EXT"},
    {GL_TEXTURE_NORMAL_EXT,                                "GL_TEXTURE_NORMAL_EXT"},


    /*GL_EXT_multi_draw_arrays*/                           /*GL_EXT_multi_draw_arrays*/


    /*GL_EXT_fog_coord*/                                   /*GL_EXT_fog_coord*/
    {GL_FOG_COORDINATE_SOURCE_EXT,                         "GL_FOG_COORDINATE_SOURCE_EXT"},
    {GL_FOG_COORDINATE_EXT,                                "GL_FOG_COORDINATE_EXT"},
    {GL_FRAGMENT_DEPTH_EXT,                                "GL_FRAGMENT_DEPTH_EXT"},
    {GL_CURRENT_FOG_COORDINATE_EXT,                        "GL_CURRENT_FOG_COORDINATE_EXT"},
    {GL_FOG_COORDINATE_ARRAY_TYPE_EXT,                     "GL_FOG_COORDINATE_ARRAY_TYPE_EXT"},
    {GL_FOG_COORDINATE_ARRAY_STRIDE_EXT,                   "GL_FOG_COORDINATE_ARRAY_STRIDE_EXT"},
    {GL_FOG_COORDINATE_ARRAY_POINTER_EXT,                  "GL_FOG_COORDINATE_ARRAY_POINTER_EXT"},
    {GL_FOG_COORDINATE_ARRAY_EXT,                          "GL_FOG_COORDINATE_ARRAY_EXT"},


    /*GL_REND_screen_coordinates*/                         /*GL_REND_screen_coordinates*/
    {GL_SCREEN_COORDINATES_REND,                           "GL_SCREEN_COORDINATES_REND"},
    {GL_INVERTED_SCREEN_W_REND,                            "GL_INVERTED_SCREEN_W_REND"},


    /*GL_EXT_coordinate_frame*/                            /*GL_EXT_coordinate_frame*/
    {GL_TANGENT_ARRAY_EXT,                                 "GL_TANGENT_ARRAY_EXT"},
    {GL_BINORMAL_ARRAY_EXT,                                "GL_BINORMAL_ARRAY_EXT"},
    {GL_CURRENT_TANGENT_EXT,                               "GL_CURRENT_TANGENT_EXT"},
    {GL_CURRENT_BINORMAL_EXT,                              "GL_CURRENT_BINORMAL_EXT"},
    {GL_TANGENT_ARRAY_TYPE_EXT,                            "GL_TANGENT_ARRAY_TYPE_EXT"},
    {GL_TANGENT_ARRAY_STRIDE_EXT,                          "GL_TANGENT_ARRAY_STRIDE_EXT"},
    {GL_BINORMAL_ARRAY_TYPE_EXT,                           "GL_BINORMAL_ARRAY_TYPE_EXT"},
    {GL_BINORMAL_ARRAY_STRIDE_EXT,                         "GL_BINORMAL_ARRAY_STRIDE_EXT"},
    {GL_TANGENT_ARRAY_POINTER_EXT,                         "GL_TANGENT_ARRAY_POINTER_EXT"},
    {GL_BINORMAL_ARRAY_POINTER_EXT,                        "GL_BINORMAL_ARRAY_POINTER_EXT"},
    {GL_MAP1_TANGENT_EXT,                                  "GL_MAP1_TANGENT_EXT"},
    {GL_MAP2_TANGENT_EXT,                                  "GL_MAP2_TANGENT_EXT"},
    {GL_MAP1_BINORMAL_EXT,                                 "GL_MAP1_BINORMAL_EXT"},
    {GL_MAP2_BINORMAL_EXT,                                 "GL_MAP2_BINORMAL_EXT"},


    /*GL_EXT_texture_env_combine*/                         /*GL_EXT_texture_env_combine*/
    {GL_COMBINE_EXT,                                       "GL_COMBINE_EXT"},
    {GL_COMBINE_RGB_EXT,                                   "GL_COMBINE_RGB_EXT"},
    {GL_COMBINE_ALPHA_EXT,                                 "GL_COMBINE_ALPHA_EXT"},
    {GL_RGB_SCALE_EXT,                                     "GL_RGB_SCALE_EXT"},
    {GL_ADD_SIGNED_EXT,                                    "GL_ADD_SIGNED_EXT"},
    {GL_INTERPOLATE_EXT,                                   "GL_INTERPOLATE_EXT"},
    {GL_CONSTANT_EXT,                                      "GL_CONSTANT_EXT"},
    {GL_PRIMARY_COLOR_EXT,                                 "GL_PRIMARY_COLOR_EXT"},
    {GL_PREVIOUS_EXT,                                      "GL_PREVIOUS_EXT"},
    {GL_SOURCE0_RGB_EXT,                                   "GL_SOURCE0_RGB_EXT"},
    {GL_SOURCE1_RGB_EXT,                                   "GL_SOURCE1_RGB_EXT"},
    {GL_SOURCE2_RGB_EXT,                                   "GL_SOURCE2_RGB_EXT"},
    {GL_SOURCE0_ALPHA_EXT,                                 "GL_SOURCE0_ALPHA_EXT"},
    {GL_SOURCE1_ALPHA_EXT,                                 "GL_SOURCE1_ALPHA_EXT"},
    {GL_SOURCE2_ALPHA_EXT,                                 "GL_SOURCE2_ALPHA_EXT"},
    {GL_OPERAND0_RGB_EXT,                                  "GL_OPERAND0_RGB_EXT"},
    {GL_OPERAND1_RGB_EXT,                                  "GL_OPERAND1_RGB_EXT"},
    {GL_OPERAND2_RGB_EXT,                                  "GL_OPERAND2_RGB_EXT"},
    {GL_OPERAND0_ALPHA_EXT,                                "GL_OPERAND0_ALPHA_EXT"},
    {GL_OPERAND1_ALPHA_EXT,                                "GL_OPERAND1_ALPHA_EXT"},
    {GL_OPERAND2_ALPHA_EXT,                                "GL_OPERAND2_ALPHA_EXT"},


    /*GL_APPLE_specular_vector*/                           /*GL_APPLE_specular_vector*/
    {GL_LIGHT_MODEL_SPECULAR_VECTOR_APPLE,                 "GL_LIGHT_MODEL_SPECULAR_VECTOR_APPLE"},


    /*GL_APPLE_transform_hint*/                            /*GL_APPLE_transform_hint*/
    {GL_TRANSFORM_HINT_APPLE,                              "GL_TRANSFORM_HINT_APPLE"},


    /*GL_SGIX_fog_scale*/                                  /*GL_SGIX_fog_scale*/
#ifdef GL_SGIX_fog_scale
    {GL_FOG_SCALE_SGIX,                                    "GL_FOG_SCALE_SGIX"},
    {GL_FOG_SCALE_VALUE_SGIX,                              "GL_FOG_SCALE_VALUE_SGIX"},
#endif


    /*GL_SUNX_constant_data*/                              /*GL_SUNX_constant_data*/
    {GL_UNPACK_CONSTANT_DATA_SUNX,                         "GL_UNPACK_CONSTANT_DATA_SUNX"},
    {GL_TEXTURE_CONSTANT_DATA_SUNX,                        "GL_TEXTURE_CONSTANT_DATA_SUNX"},


    /*GL_SUN_global_alpha*/                                /*GL_SUN_global_alpha*/
    {GL_GLOBAL_ALPHA_SUN,                                  "GL_GLOBAL_ALPHA_SUN"},
    {GL_GLOBAL_ALPHA_FACTOR_SUN,                           "GL_GLOBAL_ALPHA_FACTOR_SUN"},


    /*GL_SUN_triangle_list*/                               /*GL_SUN_triangle_list*/
    {GL_RESTART_SUN,                                       "GL_RESTART_SUN"},
    {GL_REPLACE_MIDDLE_SUN,                                "GL_REPLACE_MIDDLE_SUN"},
    {GL_REPLACE_OLDEST_SUN,                                "GL_REPLACE_OLDEST_SUN"},
    {GL_TRIANGLE_LIST_SUN,                                 "GL_TRIANGLE_LIST_SUN"},
    {GL_REPLACEMENT_CODE_SUN,                              "GL_REPLACEMENT_CODE_SUN"},
    {GL_REPLACEMENT_CODE_ARRAY_SUN,                        "GL_REPLACEMENT_CODE_ARRAY_SUN"},
    {GL_REPLACEMENT_CODE_ARRAY_TYPE_SUN,                   "GL_REPLACEMENT_CODE_ARRAY_TYPE_SUN"},
    {GL_REPLACEMENT_CODE_ARRAY_STRIDE_SUN,                 "GL_REPLACEMENT_CODE_ARRAY_STRIDE_SUN"},
    {GL_REPLACEMENT_CODE_ARRAY_POINTER_SUN,                "GL_REPLACEMENT_CODE_ARRAY_POINTER_SUN"},
    {GL_R1UI_V3F_SUN,                                      "GL_R1UI_V3F_SUN"},
    {GL_R1UI_C4UB_V3F_SUN,                                 "GL_R1UI_C4UB_V3F_SUN"},
    {GL_R1UI_C3F_V3F_SUN,                                  "GL_R1UI_C3F_V3F_SUN"},
    {GL_R1UI_N3F_V3F_SUN,                                  "GL_R1UI_N3F_V3F_SUN"},
    {GL_R1UI_C4F_N3F_V3F_SUN,                              "GL_R1UI_C4F_N3F_V3F_SUN"},
    {GL_R1UI_T2F_V3F_SUN,                                  "GL_R1UI_T2F_V3F_SUN"},
    {GL_R1UI_T2F_N3F_V3F_SUN,                              "GL_R1UI_T2F_N3F_V3F_SUN"},
    {GL_R1UI_T2F_C4F_N3F_V3F_SUN,                          "GL_R1UI_T2F_C4F_N3F_V3F_SUN"},


    /*GL_SUN_vertex*/                                      /*GL_SUN_vertex*/


    /*GL_EXT_blend_func_separate*/                         /*GL_EXT_blend_func_separate*/
    {GL_BLEND_DST_RGB_EXT,                                 "GL_BLEND_DST_RGB_EXT"},
    {GL_BLEND_SRC_RGB_EXT,                                 "GL_BLEND_SRC_RGB_EXT"},
    {GL_BLEND_DST_ALPHA_EXT,                               "GL_BLEND_DST_ALPHA_EXT"},
    {GL_BLEND_SRC_ALPHA_EXT,                               "GL_BLEND_SRC_ALPHA_EXT"},


    /*GL_INGR_color_clamp*/                                /*GL_INGR_color_clamp*/
    {GL_RED_MIN_CLAMP_INGR,                                "GL_RED_MIN_CLAMP_INGR"},
    {GL_GREEN_MIN_CLAMP_INGR,                              "GL_GREEN_MIN_CLAMP_INGR"},
    {GL_BLUE_MIN_CLAMP_INGR,                               "GL_BLUE_MIN_CLAMP_INGR"},
    {GL_ALPHA_MIN_CLAMP_INGR,                              "GL_ALPHA_MIN_CLAMP_INGR"},
    {GL_RED_MAX_CLAMP_INGR,                                "GL_RED_MAX_CLAMP_INGR"},
    {GL_GREEN_MAX_CLAMP_INGR,                              "GL_GREEN_MAX_CLAMP_INGR"},
    {GL_BLUE_MAX_CLAMP_INGR,                               "GL_BLUE_MAX_CLAMP_INGR"},
    {GL_ALPHA_MAX_CLAMP_INGR,                              "GL_ALPHA_MAX_CLAMP_INGR"},


    /*GL_INGR_interlace_read*/                             /*GL_INGR_interlace_read*/
    {GL_INTERLACE_READ_INGR,                               "GL_INTERLACE_READ_INGR"},


    /*GL_EXT_stencil_wrap*/                                /*GL_EXT_stencil_wrap*/
    {GL_INCR_WRAP_EXT,                                     "GL_INCR_WRAP_EXT"},
    {GL_DECR_WRAP_EXT,                                     "GL_DECR_WRAP_EXT"},


    /*GL_EXT_422_pixels*/                                  /*GL_EXT_422_pixels*/
    {GL_422_EXT,                                           "GL_422_EXT"},
    {GL_422_REV_EXT,                                       "GL_422_REV_EXT"},
    {GL_422_AVERAGE_EXT,                                   "GL_422_AVERAGE_EXT"},
    {GL_422_REV_AVERAGE_EXT,                               "GL_422_REV_AVERAGE_EXT"},


    /*GL_NV_texgen_reflection*/                            /*GL_NV_texgen_reflection*/
    {GL_NORMAL_MAP_NV,                                     "GL_NORMAL_MAP_NV"},
    {GL_REFLECTION_MAP_NV,                                 "GL_REFLECTION_MAP_NV"},


    /*GL_EXT_texture_cube_map*/                            /*GL_EXT_texture_cube_map*/
    {GL_NORMAL_MAP_EXT,                                    "GL_NORMAL_MAP_EXT"},
    {GL_REFLECTION_MAP_EXT,                                "GL_REFLECTION_MAP_EXT"},
    {GL_TEXTURE_CUBE_MAP_EXT,                              "GL_TEXTURE_CUBE_MAP_EXT"},
    {GL_TEXTURE_BINDING_CUBE_MAP_EXT,                      "GL_TEXTURE_BINDING_CUBE_MAP_EXT"},
    {GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT,                   "GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT"},
    {GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT,                   "GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT"},
    {GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT,                   "GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT"},
    {GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT,                   "GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT"},
    {GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT,                   "GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT"},
    {GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT,                   "GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT"},
    {GL_PROXY_TEXTURE_CUBE_MAP_EXT,                        "GL_PROXY_TEXTURE_CUBE_MAP_EXT"},
    {GL_MAX_CUBE_MAP_TEXTURE_SIZE_EXT,                     "GL_MAX_CUBE_MAP_TEXTURE_SIZE_EXT"},


    /*GL_SUN_convolution_border_modes*/                    /*GL_SUN_convolution_border_modes*/
    {GL_WRAP_BORDER_SUN,                                   "GL_WRAP_BORDER_SUN"},


    /*GL_EXT_texture_env_add*/                             /*GL_EXT_texture_env_add*/


    /*GL_EXT_texture_lod_bias*/                            /*GL_EXT_texture_lod_bias*/
    {GL_MAX_TEXTURE_LOD_BIAS_EXT,                          "GL_MAX_TEXTURE_LOD_BIAS_EXT"},
    {GL_TEXTURE_FILTER_CONTROL_EXT,                        "GL_TEXTURE_FILTER_CONTROL_EXT"},
    {GL_TEXTURE_LOD_BIAS_EXT,                              "GL_TEXTURE_LOD_BIAS_EXT"},


    /*GL_EXT_texture_filter_anisotropic*/                  /*GL_EXT_texture_filter_anisotropic*/
    {GL_TEXTURE_MAX_ANISOTROPY_EXT,                        "GL_TEXTURE_MAX_ANISOTROPY_EXT"},
    {GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT,                    "GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT"},


    /*GL_EXT_vertex_weighting*/                            /*GL_EXT_vertex_weighting*/
    {GL_MODELVIEW1_STACK_DEPTH_EXT,                        "GL_MODELVIEW1_STACK_DEPTH_EXT"},
    {GL_MODELVIEW1_MATRIX_EXT,                             "GL_MODELVIEW1_MATRIX_EXT"},
    {GL_VERTEX_WEIGHTING_EXT,                              "GL_VERTEX_WEIGHTING_EXT"},
    {GL_MODELVIEW1_EXT,                                    "GL_MODELVIEW1_EXT"},
    {GL_CURRENT_VERTEX_WEIGHT_EXT,                         "GL_CURRENT_VERTEX_WEIGHT_EXT"},
    {GL_VERTEX_WEIGHT_ARRAY_EXT,                           "GL_VERTEX_WEIGHT_ARRAY_EXT"},
    {GL_VERTEX_WEIGHT_ARRAY_SIZE_EXT,                      "GL_VERTEX_WEIGHT_ARRAY_SIZE_EXT"},
    {GL_VERTEX_WEIGHT_ARRAY_TYPE_EXT,                      "GL_VERTEX_WEIGHT_ARRAY_TYPE_EXT"},
    {GL_VERTEX_WEIGHT_ARRAY_STRIDE_EXT,                    "GL_VERTEX_WEIGHT_ARRAY_STRIDE_EXT"},
    {GL_VERTEX_WEIGHT_ARRAY_POINTER_EXT,                   "GL_VERTEX_WEIGHT_ARRAY_POINTER_EXT"},


    /*GL_NV_light_max_exponent*/                           /*GL_NV_light_max_exponent*/
    {GL_MAX_SHININESS_NV,                                  "GL_MAX_SHININESS_NV"},
    {GL_MAX_SPOT_EXPONENT_NV,                              "GL_MAX_SPOT_EXPONENT_NV"},


    /*GL_NV_vertex_array_range*/                           /*GL_NV_vertex_array_range*/
    {GL_VERTEX_ARRAY_RANGE_NV,                             "GL_VERTEX_ARRAY_RANGE_NV"},
    {GL_VERTEX_ARRAY_RANGE_LENGTH_NV,                      "GL_VERTEX_ARRAY_RANGE_LENGTH_NV"},
    {GL_VERTEX_ARRAY_RANGE_VALID_NV,                       "GL_VERTEX_ARRAY_RANGE_VALID_NV"},
    {GL_MAX_VERTEX_ARRAY_RANGE_ELEMENT_NV,                 "GL_MAX_VERTEX_ARRAY_RANGE_ELEMENT_NV"},
    {GL_VERTEX_ARRAY_RANGE_POINTER_NV,                     "GL_VERTEX_ARRAY_RANGE_POINTER_NV"},


    /*GL_NV_register_combiners*/                           /*GL_NV_register_combiners*/
    {GL_REGISTER_COMBINERS_NV,                             "GL_REGISTER_COMBINERS_NV"},
    {GL_VARIABLE_A_NV,                                     "GL_VARIABLE_A_NV"},
    {GL_VARIABLE_B_NV,                                     "GL_VARIABLE_B_NV"},
    {GL_VARIABLE_C_NV,                                     "GL_VARIABLE_C_NV"},
    {GL_VARIABLE_D_NV,                                     "GL_VARIABLE_D_NV"},
    {GL_VARIABLE_E_NV,                                     "GL_VARIABLE_E_NV"},
    {GL_VARIABLE_F_NV,                                     "GL_VARIABLE_F_NV"},
    {GL_VARIABLE_G_NV,                                     "GL_VARIABLE_G_NV"},
    {GL_CONSTANT_COLOR0_NV,                                "GL_CONSTANT_COLOR0_NV"},
    {GL_CONSTANT_COLOR1_NV,                                "GL_CONSTANT_COLOR1_NV"},
    {GL_PRIMARY_COLOR_NV,                                  "GL_PRIMARY_COLOR_NV"},
    {GL_SECONDARY_COLOR_NV,                                "GL_SECONDARY_COLOR_NV"},
    {GL_SPARE0_NV,                                         "GL_SPARE0_NV"},
    {GL_SPARE1_NV,                                         "GL_SPARE1_NV"},
    {GL_DISCARD_NV,                                        "GL_DISCARD_NV"},
    {GL_E_TIMES_F_NV,                                      "GL_E_TIMES_F_NV"},
    {GL_SPARE0_PLUS_SECONDARY_COLOR_NV,                    "GL_SPARE0_PLUS_SECONDARY_COLOR_NV"},
    {GL_UNSIGNED_IDENTITY_NV,                              "GL_UNSIGNED_IDENTITY_NV"},
    {GL_UNSIGNED_INVERT_NV,                                "GL_UNSIGNED_INVERT_NV"},
    {GL_EXPAND_NORMAL_NV,                                  "GL_EXPAND_NORMAL_NV"},
    {GL_EXPAND_NEGATE_NV,                                  "GL_EXPAND_NEGATE_NV"},
    {GL_HALF_BIAS_NORMAL_NV,                               "GL_HALF_BIAS_NORMAL_NV"},
    {GL_HALF_BIAS_NEGATE_NV,                               "GL_HALF_BIAS_NEGATE_NV"},
    {GL_SIGNED_IDENTITY_NV,                                "GL_SIGNED_IDENTITY_NV"},
    {GL_SIGNED_NEGATE_NV,                                  "GL_SIGNED_NEGATE_NV"},
    {GL_SCALE_BY_TWO_NV,                                   "GL_SCALE_BY_TWO_NV"},
    {GL_SCALE_BY_FOUR_NV,                                  "GL_SCALE_BY_FOUR_NV"},
    {GL_SCALE_BY_ONE_HALF_NV,                              "GL_SCALE_BY_ONE_HALF_NV"},
    {GL_BIAS_BY_NEGATIVE_ONE_HALF_NV,                      "GL_BIAS_BY_NEGATIVE_ONE_HALF_NV"},
    {GL_COMBINER_INPUT_NV,                                 "GL_COMBINER_INPUT_NV"},
    {GL_COMBINER_MAPPING_NV,                               "GL_COMBINER_MAPPING_NV"},
    {GL_COMBINER_COMPONENT_USAGE_NV,                       "GL_COMBINER_COMPONENT_USAGE_NV"},
    {GL_COMBINER_AB_DOT_PRODUCT_NV,                        "GL_COMBINER_AB_DOT_PRODUCT_NV"},
    {GL_COMBINER_CD_DOT_PRODUCT_NV,                        "GL_COMBINER_CD_DOT_PRODUCT_NV"},
    {GL_COMBINER_MUX_SUM_NV,                               "GL_COMBINER_MUX_SUM_NV"},
    {GL_COMBINER_SCALE_NV,                                 "GL_COMBINER_SCALE_NV"},
    {GL_COMBINER_BIAS_NV,                                  "GL_COMBINER_BIAS_NV"},
    {GL_COMBINER_AB_OUTPUT_NV,                             "GL_COMBINER_AB_OUTPUT_NV"},
    {GL_COMBINER_CD_OUTPUT_NV,                             "GL_COMBINER_CD_OUTPUT_NV"},
    {GL_COMBINER_SUM_OUTPUT_NV,                            "GL_COMBINER_SUM_OUTPUT_NV"},
    {GL_MAX_GENERAL_COMBINERS_NV,                          "GL_MAX_GENERAL_COMBINERS_NV"},
    {GL_NUM_GENERAL_COMBINERS_NV,                          "GL_NUM_GENERAL_COMBINERS_NV"},
    {GL_COLOR_SUM_CLAMP_NV,                                "GL_COLOR_SUM_CLAMP_NV"},
    {GL_COMBINER0_NV,                                      "GL_COMBINER0_NV"},
    {GL_COMBINER1_NV,                                      "GL_COMBINER1_NV"},
    {GL_COMBINER2_NV,                                      "GL_COMBINER2_NV"},
    {GL_COMBINER3_NV,                                      "GL_COMBINER3_NV"},
    {GL_COMBINER4_NV,                                      "GL_COMBINER4_NV"},
    {GL_COMBINER5_NV,                                      "GL_COMBINER5_NV"},
    {GL_COMBINER6_NV,                                      "GL_COMBINER6_NV"},
    {GL_COMBINER7_NV,                                      "GL_COMBINER7_NV"},
    /*reuseGL_TEXTURE0_ARB*/                               /*reuseGL_TEXTURE0_ARB*/
    /*reuseGL_TEXTURE1_ARB*/                               /*reuseGL_TEXTURE1_ARB*/
    /*reuseGL_ZERO*/                                       /*reuseGL_ZERO*/
    /*reuseGL_NONE*/                                       /*reuseGL_NONE*/
    /*reuseGL_FOG*/                                        /*reuseGL_FOG*/


    /*GL_NV_fog_distance*/                                 /*GL_NV_fog_distance*/
    {GL_FOG_DISTANCE_MODE_NV,                              "GL_FOG_DISTANCE_MODE_NV"},
    {GL_EYE_RADIAL_NV,                                     "GL_EYE_RADIAL_NV"},
    {GL_EYE_PLANE_ABSOLUTE_NV,                             "GL_EYE_PLANE_ABSOLUTE_NV"},
    /*reuseGL_EYE_PLANE*/                                  /*reuseGL_EYE_PLANE*/


    /*GL_NV_texgen_emboss*/                                /*GL_NV_texgen_emboss*/
    {GL_EMBOSS_LIGHT_NV,                                   "GL_EMBOSS_LIGHT_NV"},
    {GL_EMBOSS_CONSTANT_NV,                                "GL_EMBOSS_CONSTANT_NV"},
    {GL_EMBOSS_MAP_NV,                                     "GL_EMBOSS_MAP_NV"},


    /*GL_NV_blend_square*/                                 /*GL_NV_blend_square*/


    /*GL_NV_texture_env_combine4*/                         /*GL_NV_texture_env_combine4*/
    {GL_COMBINE4_NV,                                       "GL_COMBINE4_NV"},
    {GL_SOURCE3_RGB_NV,                                    "GL_SOURCE3_RGB_NV"},
    {GL_SOURCE3_ALPHA_NV,                                  "GL_SOURCE3_ALPHA_NV"},
    {GL_OPERAND3_RGB_NV,                                   "GL_OPERAND3_RGB_NV"},
    {GL_OPERAND3_ALPHA_NV,                                 "GL_OPERAND3_ALPHA_NV"},


    /*GL_MESA_resize_buffers*/                             /*GL_MESA_resize_buffers*/


    /*GL_MESA_window_pos*/                                 /*GL_MESA_window_pos*/


    /*GL_EXT_texture_compression_s3tc*/                    /*GL_EXT_texture_compression_s3tc*/
    {GL_COMPRESSED_RGB_S3TC_DXT1_EXT,                      "GL_COMPRESSED_RGB_S3TC_DXT1_EXT"},
    {GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,                     "GL_COMPRESSED_RGBA_S3TC_DXT1_EXT"},
    {GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,                     "GL_COMPRESSED_RGBA_S3TC_DXT3_EXT"},
    {GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,                     "GL_COMPRESSED_RGBA_S3TC_DXT5_EXT"},


    /*GL_IBM_cull_vertex*/                                 /*GL_IBM_cull_vertex*/
    {GL_CULL_VERTEX_IBM,                                   "GL_CULL_VERTEX_IBM"},


    /*GL_IBM_multimode_draw_arrays*/                       /*GL_IBM_multimode_draw_arrays*/


    /*GL_IBM_vertex_array_lists*/                          /*GL_IBM_vertex_array_lists*/
    {GL_VERTEX_ARRAY_LIST_IBM,                             "GL_VERTEX_ARRAY_LIST_IBM"},
    {GL_NORMAL_ARRAY_LIST_IBM,                             "GL_NORMAL_ARRAY_LIST_IBM"},
    {GL_COLOR_ARRAY_LIST_IBM,                              "GL_COLOR_ARRAY_LIST_IBM"},
    {GL_INDEX_ARRAY_LIST_IBM,                              "GL_INDEX_ARRAY_LIST_IBM"},
    {GL_TEXTURE_COORD_ARRAY_LIST_IBM,                      "GL_TEXTURE_COORD_ARRAY_LIST_IBM"},
    {GL_EDGE_FLAG_ARRAY_LIST_IBM,                          "GL_EDGE_FLAG_ARRAY_LIST_IBM"},
    {GL_FOG_COORDINATE_ARRAY_LIST_IBM,                     "GL_FOG_COORDINATE_ARRAY_LIST_IBM"},
    {GL_SECONDARY_COLOR_ARRAY_LIST_IBM,                    "GL_SECONDARY_COLOR_ARRAY_LIST_IBM"},
    {GL_VERTEX_ARRAY_LIST_STRIDE_IBM,                      "GL_VERTEX_ARRAY_LIST_STRIDE_IBM"},
    {GL_NORMAL_ARRAY_LIST_STRIDE_IBM,                      "GL_NORMAL_ARRAY_LIST_STRIDE_IBM"},
    {GL_COLOR_ARRAY_LIST_STRIDE_IBM,                       "GL_COLOR_ARRAY_LIST_STRIDE_IBM"},
    {GL_INDEX_ARRAY_LIST_STRIDE_IBM,                       "GL_INDEX_ARRAY_LIST_STRIDE_IBM"},
    {GL_TEXTURE_COORD_ARRAY_LIST_STRIDE_IBM,               "GL_TEXTURE_COORD_ARRAY_LIST_STRIDE_IBM"},
    {GL_EDGE_FLAG_ARRAY_LIST_STRIDE_IBM,                   "GL_EDGE_FLAG_ARRAY_LIST_STRIDE_IBM"},
    {GL_FOG_COORDINATE_ARRAY_LIST_STRIDE_IBM,              "GL_FOG_COORDINATE_ARRAY_LIST_STRIDE_IBM"},
    {GL_SECONDARY_COLOR_ARRAY_LIST_STRIDE_IBM,             "GL_SECONDARY_COLOR_ARRAY_LIST_STRIDE_IBM"},


    /*GL_SGIX_subsample*/                                  /*GL_SGIX_subsample*/
    {GL_PACK_SUBSAMPLE_RATE_SGIX,                          "GL_PACK_SUBSAMPLE_RATE_SGIX"},
    {GL_UNPACK_SUBSAMPLE_RATE_SGIX,                        "GL_UNPACK_SUBSAMPLE_RATE_SGIX"},
    {GL_PIXEL_SUBSAMPLE_4444_SGIX,                         "GL_PIXEL_SUBSAMPLE_4444_SGIX"},
    {GL_PIXEL_SUBSAMPLE_2424_SGIX,                         "GL_PIXEL_SUBSAMPLE_2424_SGIX"},
    {GL_PIXEL_SUBSAMPLE_4242_SGIX,                         "GL_PIXEL_SUBSAMPLE_4242_SGIX"},


    /*GL_SGIX_ycrcb_subsample*/                            /*GL_SGIX_ycrcb_subsample*/


    /*GL_SGIX_ycrcba*/                                     /*GL_SGIX_ycrcba*/
    {GL_YCRCB_SGIX,                                        "GL_YCRCB_SGIX"},
    {GL_YCRCBA_SGIX,                                       "GL_YCRCBA_SGIX"},


    /*GL_SGI_depth_pass_instrument*/                       /*GL_SGI_depth_pass_instrument*/
#ifdef GL_SGI_depth_pass_instrument
    {GL_DEPTH_PASS_INSTRUMENT_SGIX,                        "GL_DEPTH_PASS_INSTRUMENT_SGIX"},
    {GL_DEPTH_PASS_INSTRUMENT_COUNTERS_SGIX,               "GL_DEPTH_PASS_INSTRUMENT_COUNTERS_SGIX"},
    {GL_DEPTH_PASS_INSTRUMENT_MAX_SGIX,                    "GL_DEPTH_PASS_INSTRUMENT_MAX_SGIX"},
#endif


    /*GL_3DFX_texture_compression_FXT1*/                   /*GL_3DFX_texture_compression_FXT1*/
    {GL_COMPRESSED_RGB_FXT1_3DFX,                          "GL_COMPRESSED_RGB_FXT1_3DFX"},
    {GL_COMPRESSED_RGBA_FXT1_3DFX,                         "GL_COMPRESSED_RGBA_FXT1_3DFX"},


    /*GL_3DFX_multisample*/                                /*GL_3DFX_multisample*/
    {GL_MULTISAMPLE_3DFX,                                  "GL_MULTISAMPLE_3DFX"},
    {GL_SAMPLE_BUFFERS_3DFX,                               "GL_SAMPLE_BUFFERS_3DFX"},
    {GL_SAMPLES_3DFX,                                      "GL_SAMPLES_3DFX"},


    /*GL_3DFX_tbuffer*/                                    /*GL_3DFX_tbuffer*/


    /*GL_EXT_multisample*/                                 /*GL_EXT_multisample*/
    {GL_MULTISAMPLE_EXT,                                   "GL_MULTISAMPLE_EXT"},
    {GL_SAMPLE_ALPHA_TO_MASK_EXT,                          "GL_SAMPLE_ALPHA_TO_MASK_EXT"},
    {GL_SAMPLE_ALPHA_TO_ONE_EXT,                           "GL_SAMPLE_ALPHA_TO_ONE_EXT"},
    {GL_SAMPLE_MASK_EXT,                                   "GL_SAMPLE_MASK_EXT"},
    {GL_1PASS_EXT,                                         "GL_1PASS_EXT"},
    {GL_2PASS_0_EXT,                                       "GL_2PASS_0_EXT"},
    {GL_2PASS_1_EXT,                                       "GL_2PASS_1_EXT"},
    {GL_4PASS_0_EXT,                                       "GL_4PASS_0_EXT"},
    {GL_4PASS_1_EXT,                                       "GL_4PASS_1_EXT"},
    {GL_4PASS_2_EXT,                                       "GL_4PASS_2_EXT"},
    {GL_4PASS_3_EXT,                                       "GL_4PASS_3_EXT"},
    {GL_SAMPLE_BUFFERS_EXT,                                "GL_SAMPLE_BUFFERS_EXT"},
    {GL_SAMPLES_EXT,                                       "GL_SAMPLES_EXT"},
    {GL_SAMPLE_MASK_VALUE_EXT,                             "GL_SAMPLE_MASK_VALUE_EXT"},
    {GL_SAMPLE_MASK_INVERT_EXT,                            "GL_SAMPLE_MASK_INVERT_EXT"},
    {GL_SAMPLE_PATTERN_EXT,                                "GL_SAMPLE_PATTERN_EXT"},



    /*GL_SGIX_vertex_preclip*/                             /*GL_SGIX_vertex_preclip*/
    {GL_VERTEX_PRECLIP_SGIX,                               "GL_VERTEX_PRECLIP_SGIX"},
    {GL_VERTEX_PRECLIP_HINT_SGIX,                          "GL_VERTEX_PRECLIP_HINT_SGIX"},


    /*GL_SGIX_convolution_accuracy*/                       /*GL_SGIX_convolution_accuracy*/
    {GL_CONVOLUTION_HINT_SGIX,                             "GL_CONVOLUTION_HINT_SGIX"},


    /*GL_SGIX_resample*/                                   /*GL_SGIX_resample*/
    {GL_PACK_RESAMPLE_SGIX,                                "GL_PACK_RESAMPLE_SGIX"},
    {GL_UNPACK_RESAMPLE_SGIX,                              "GL_UNPACK_RESAMPLE_SGIX"},
    {GL_RESAMPLE_REPLICATE_SGIX,                           "GL_RESAMPLE_REPLICATE_SGIX"},
    {GL_RESAMPLE_ZERO_FILL_SGIX,                           "GL_RESAMPLE_ZERO_FILL_SGIX"},
    {GL_RESAMPLE_DECIMATE_SGIX,                            "GL_RESAMPLE_DECIMATE_SGIX"},


    /*GL_SGIS_point_line_texgen*/                          /*GL_SGIS_point_line_texgen*/
    {GL_EYE_DISTANCE_TO_POINT_SGIS,                        "GL_EYE_DISTANCE_TO_POINT_SGIS"},
    {GL_OBJECT_DISTANCE_TO_POINT_SGIS,                     "GL_OBJECT_DISTANCE_TO_POINT_SGIS"},
    {GL_EYE_DISTANCE_TO_LINE_SGIS,                         "GL_EYE_DISTANCE_TO_LINE_SGIS"},
    {GL_OBJECT_DISTANCE_TO_LINE_SGIS,                      "GL_OBJECT_DISTANCE_TO_LINE_SGIS"},
    {GL_EYE_POINT_SGIS,                                    "GL_EYE_POINT_SGIS"},
    {GL_OBJECT_POINT_SGIS,                                 "GL_OBJECT_POINT_SGIS"},
    {GL_EYE_LINE_SGIS,                                     "GL_EYE_LINE_SGIS"},
    {GL_OBJECT_LINE_SGIS,                                  "GL_OBJECT_LINE_SGIS"},


    /*GL_SGIS_texture_color_mask*/                         /*GL_SGIS_texture_color_mask*/
    {GL_TEXTURE_COLOR_WRITEMASK_SGIS,                      "GL_TEXTURE_COLOR_WRITEMASK_SGIS"},


    /*GL_EXT_texture_env_dot3*/                            /*GL_EXT_texture_env_dot3*/
    {GL_DOT3_RGB_EXT,                                      "GL_DOT3_RGB_EXT"},
    {GL_DOT3_RGBA_EXT,                                     "GL_DOT3_RGBA_EXT"},


    /*GL_ATI_texture_mirror_once*/                         /*GL_ATI_texture_mirror_once*/
    {GL_MIRROR_CLAMP_ATI,                                  "GL_MIRROR_CLAMP_ATI"},
    {GL_MIRROR_CLAMP_TO_EDGE_ATI,                          "GL_MIRROR_CLAMP_TO_EDGE_ATI"},


    /*GL_NV_fence*/                                        /*GL_NV_fence*/
    {GL_ALL_COMPLETED_NV,                                  "GL_ALL_COMPLETED_NV"},
    {GL_FENCE_STATUS_NV,                                   "GL_FENCE_STATUS_NV"},
    {GL_FENCE_CONDITION_NV,                                "GL_FENCE_CONDITION_NV"},


    /*GL_IBM_texture_mirrored_repeat*/                     /*GL_IBM_texture_mirrored_repeat*/
    {GL_MIRRORED_REPEAT_IBM,                               "GL_MIRRORED_REPEAT_IBM"},


    /*GL_NV_evaluators*/                                   /*GL_NV_evaluators*/
    {GL_EVAL_2D_NV,                                        "GL_EVAL_2D_NV"},
    {GL_EVAL_TRIANGULAR_2D_NV,                             "GL_EVAL_TRIANGULAR_2D_NV"},
    {GL_MAP_TESSELLATION_NV,                               "GL_MAP_TESSELLATION_NV"},
    {GL_MAP_ATTRIB_U_ORDER_NV,                             "GL_MAP_ATTRIB_U_ORDER_NV"},
    {GL_MAP_ATTRIB_V_ORDER_NV,                             "GL_MAP_ATTRIB_V_ORDER_NV"},
    {GL_EVAL_FRACTIONAL_TESSELLATION_NV,                   "GL_EVAL_FRACTIONAL_TESSELLATION_NV"},
    {GL_EVAL_VERTEX_ATTRIB0_NV,                            "GL_EVAL_VERTEX_ATTRIB0_NV"},
    {GL_EVAL_VERTEX_ATTRIB1_NV,                            "GL_EVAL_VERTEX_ATTRIB1_NV"},
    {GL_EVAL_VERTEX_ATTRIB2_NV,                            "GL_EVAL_VERTEX_ATTRIB2_NV"},
    {GL_EVAL_VERTEX_ATTRIB3_NV,                            "GL_EVAL_VERTEX_ATTRIB3_NV"},
    {GL_EVAL_VERTEX_ATTRIB4_NV,                            "GL_EVAL_VERTEX_ATTRIB4_NV"},
    {GL_EVAL_VERTEX_ATTRIB5_NV,                            "GL_EVAL_VERTEX_ATTRIB5_NV"},
    {GL_EVAL_VERTEX_ATTRIB6_NV,                            "GL_EVAL_VERTEX_ATTRIB6_NV"},
    {GL_EVAL_VERTEX_ATTRIB7_NV,                            "GL_EVAL_VERTEX_ATTRIB7_NV"},
    {GL_EVAL_VERTEX_ATTRIB8_NV,                            "GL_EVAL_VERTEX_ATTRIB8_NV"},
    {GL_EVAL_VERTEX_ATTRIB9_NV,                            "GL_EVAL_VERTEX_ATTRIB9_NV"},
    {GL_EVAL_VERTEX_ATTRIB10_NV,                           "GL_EVAL_VERTEX_ATTRIB10_NV"},
    {GL_EVAL_VERTEX_ATTRIB11_NV,                           "GL_EVAL_VERTEX_ATTRIB11_NV"},
    {GL_EVAL_VERTEX_ATTRIB12_NV,                           "GL_EVAL_VERTEX_ATTRIB12_NV"},
    {GL_EVAL_VERTEX_ATTRIB13_NV,                           "GL_EVAL_VERTEX_ATTRIB13_NV"},
    {GL_EVAL_VERTEX_ATTRIB14_NV,                           "GL_EVAL_VERTEX_ATTRIB14_NV"},
    {GL_EVAL_VERTEX_ATTRIB15_NV,                           "GL_EVAL_VERTEX_ATTRIB15_NV"},
    {GL_MAX_MAP_TESSELLATION_NV,                           "GL_MAX_MAP_TESSELLATION_NV"},
    {GL_MAX_RATIONAL_EVAL_ORDER_NV,                        "GL_MAX_RATIONAL_EVAL_ORDER_NV"},


    /*GL_NV_packed_depth_stencil*/                         /*GL_NV_packed_depth_stencil*/
    {GL_DEPTH_STENCIL_NV,                                  "GL_DEPTH_STENCIL_NV"},
    {GL_UNSIGNED_INT_24_8_NV,                              "GL_UNSIGNED_INT_24_8_NV"},


    /*GL_NV_register_combiners2*/                          /*GL_NV_register_combiners2*/
    {GL_PER_STAGE_CONSTANTS_NV,                            "GL_PER_STAGE_CONSTANTS_NV"},


    /*GL_NV_texture_compression_vtc*/                      /*GL_NV_texture_compression_vtc*/


    /*GL_NV_texture_rectangle*/                            /*GL_NV_texture_rectangle*/
    {GL_TEXTURE_RECTANGLE_NV,                              "GL_TEXTURE_RECTANGLE_NV"},
    {GL_TEXTURE_BINDING_RECTANGLE_NV,                      "GL_TEXTURE_BINDING_RECTANGLE_NV"},
    {GL_PROXY_TEXTURE_RECTANGLE_NV,                        "GL_PROXY_TEXTURE_RECTANGLE_NV"},
    {GL_MAX_RECTANGLE_TEXTURE_SIZE_NV,                     "GL_MAX_RECTANGLE_TEXTURE_SIZE_NV"},


    /*GL_NV_texture_shader*/                               /*GL_NV_texture_shader*/
    {GL_OFFSET_TEXTURE_RECTANGLE_NV,                       "GL_OFFSET_TEXTURE_RECTANGLE_NV"},
    {GL_OFFSET_TEXTURE_RECTANGLE_SCALE_NV,                 "GL_OFFSET_TEXTURE_RECTANGLE_SCALE_NV"},
    {GL_DOT_PRODUCT_TEXTURE_RECTANGLE_NV,                  "GL_DOT_PRODUCT_TEXTURE_RECTANGLE_NV"},
    {GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV,              "GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV"},
    {GL_UNSIGNED_INT_S8_S8_8_8_NV,                         "GL_UNSIGNED_INT_S8_S8_8_8_NV"},
    {GL_UNSIGNED_INT_8_8_S8_S8_REV_NV,                     "GL_UNSIGNED_INT_8_8_S8_S8_REV_NV"},
    {GL_DSDT_MAG_INTENSITY_NV,                             "GL_DSDT_MAG_INTENSITY_NV"},
    {GL_SHADER_CONSISTENT_NV,                              "GL_SHADER_CONSISTENT_NV"},
    {GL_TEXTURE_SHADER_NV,                                 "GL_TEXTURE_SHADER_NV"},
    {GL_SHADER_OPERATION_NV,                               "GL_SHADER_OPERATION_NV"},
    {GL_CULL_MODES_NV,                                     "GL_CULL_MODES_NV"},
    {GL_OFFSET_TEXTURE_MATRIX_NV,                          "GL_OFFSET_TEXTURE_MATRIX_NV"},
    {GL_OFFSET_TEXTURE_SCALE_NV,                           "GL_OFFSET_TEXTURE_SCALE_NV"},
    {GL_OFFSET_TEXTURE_BIAS_NV,                            "GL_OFFSET_TEXTURE_BIAS_NV"},
    {GL_PREVIOUS_TEXTURE_INPUT_NV,                         "GL_PREVIOUS_TEXTURE_INPUT_NV"},
    {GL_CONST_EYE_NV,                                      "GL_CONST_EYE_NV"},
    {GL_PASS_THROUGH_NV,                                   "GL_PASS_THROUGH_NV"},
    {GL_CULL_FRAGMENT_NV,                                  "GL_CULL_FRAGMENT_NV"},
    {GL_OFFSET_TEXTURE_2D_NV,                              "GL_OFFSET_TEXTURE_2D_NV"},
    {GL_DEPENDENT_AR_TEXTURE_2D_NV,                        "GL_DEPENDENT_AR_TEXTURE_2D_NV"},
    {GL_DEPENDENT_GB_TEXTURE_2D_NV,                        "GL_DEPENDENT_GB_TEXTURE_2D_NV"},
    {GL_DOT_PRODUCT_NV,                                    "GL_DOT_PRODUCT_NV"},
    {GL_DOT_PRODUCT_DEPTH_REPLACE_NV,                      "GL_DOT_PRODUCT_DEPTH_REPLACE_NV"},
    {GL_DOT_PRODUCT_TEXTURE_2D_NV,                         "GL_DOT_PRODUCT_TEXTURE_2D_NV"},
    {GL_DOT_PRODUCT_TEXTURE_CUBE_MAP_NV,                   "GL_DOT_PRODUCT_TEXTURE_CUBE_MAP_NV"},
    {GL_DOT_PRODUCT_DIFFUSE_CUBE_MAP_NV,                   "GL_DOT_PRODUCT_DIFFUSE_CUBE_MAP_NV"},
    {GL_DOT_PRODUCT_REFLECT_CUBE_MAP_NV,                   "GL_DOT_PRODUCT_REFLECT_CUBE_MAP_NV"},
    {GL_DOT_PRODUCT_CONST_EYE_REFLECT_CUBE_MAP_NV,         "GL_DOT_PRODUCT_CONST_EYE_REFLECT_CUBE_MAP_NV"},
    {GL_HILO_NV,                                           "GL_HILO_NV"},
    {GL_DSDT_NV,                                           "GL_DSDT_NV"},
    {GL_DSDT_MAG_NV,                                       "GL_DSDT_MAG_NV"},
    {GL_DSDT_MAG_VIB_NV,                                   "GL_DSDT_MAG_VIB_NV"},
    {GL_HILO16_NV,                                         "GL_HILO16_NV"},
    {GL_SIGNED_HILO_NV,                                    "GL_SIGNED_HILO_NV"},
    {GL_SIGNED_HILO16_NV,                                  "GL_SIGNED_HILO16_NV"},
    {GL_SIGNED_RGBA_NV,                                    "GL_SIGNED_RGBA_NV"},
    {GL_SIGNED_RGBA8_NV,                                   "GL_SIGNED_RGBA8_NV"},
    {GL_SIGNED_RGB_NV,                                     "GL_SIGNED_RGB_NV"},
    {GL_SIGNED_RGB8_NV,                                    "GL_SIGNED_RGB8_NV"},
    {GL_SIGNED_LUMINANCE_NV,                               "GL_SIGNED_LUMINANCE_NV"},
    {GL_SIGNED_LUMINANCE8_NV,                              "GL_SIGNED_LUMINANCE8_NV"},
    {GL_SIGNED_LUMINANCE_ALPHA_NV,                         "GL_SIGNED_LUMINANCE_ALPHA_NV"},
    {GL_SIGNED_LUMINANCE8_ALPHA8_NV,                       "GL_SIGNED_LUMINANCE8_ALPHA8_NV"},
    {GL_SIGNED_ALPHA_NV,                                   "GL_SIGNED_ALPHA_NV"},
    {GL_SIGNED_ALPHA8_NV,                                  "GL_SIGNED_ALPHA8_NV"},
    {GL_SIGNED_INTENSITY_NV,                               "GL_SIGNED_INTENSITY_NV"},
    {GL_SIGNED_INTENSITY8_NV,                              "GL_SIGNED_INTENSITY8_NV"},
    {GL_DSDT8_NV,                                          "GL_DSDT8_NV"},
    {GL_DSDT8_MAG8_NV,                                     "GL_DSDT8_MAG8_NV"},
    {GL_DSDT8_MAG8_INTENSITY8_NV,                          "GL_DSDT8_MAG8_INTENSITY8_NV"},
    {GL_SIGNED_RGB_UNSIGNED_ALPHA_NV,                      "GL_SIGNED_RGB_UNSIGNED_ALPHA_NV"},
    {GL_SIGNED_RGB8_UNSIGNED_ALPHA8_NV,                    "GL_SIGNED_RGB8_UNSIGNED_ALPHA8_NV"},
    {GL_HI_SCALE_NV,                                       "GL_HI_SCALE_NV"},
    {GL_LO_SCALE_NV,                                       "GL_LO_SCALE_NV"},
    {GL_DS_SCALE_NV,                                       "GL_DS_SCALE_NV"},
    {GL_DT_SCALE_NV,                                       "GL_DT_SCALE_NV"},
    {GL_MAGNITUDE_SCALE_NV,                                "GL_MAGNITUDE_SCALE_NV"},
    {GL_VIBRANCE_SCALE_NV,                                 "GL_VIBRANCE_SCALE_NV"},
    {GL_HI_BIAS_NV,                                        "GL_HI_BIAS_NV"},
    {GL_LO_BIAS_NV,                                        "GL_LO_BIAS_NV"},
    {GL_DS_BIAS_NV,                                        "GL_DS_BIAS_NV"},
    {GL_DT_BIAS_NV,                                        "GL_DT_BIAS_NV"},
    {GL_MAGNITUDE_BIAS_NV,                                 "GL_MAGNITUDE_BIAS_NV"},
    {GL_VIBRANCE_BIAS_NV,                                  "GL_VIBRANCE_BIAS_NV"},
    {GL_TEXTURE_BORDER_VALUES_NV,                          "GL_TEXTURE_BORDER_VALUES_NV"},
    {GL_TEXTURE_HI_SIZE_NV,                                "GL_TEXTURE_HI_SIZE_NV"},
    {GL_TEXTURE_LO_SIZE_NV,                                "GL_TEXTURE_LO_SIZE_NV"},
    {GL_TEXTURE_DS_SIZE_NV,                                "GL_TEXTURE_DS_SIZE_NV"},
    {GL_TEXTURE_DT_SIZE_NV,                                "GL_TEXTURE_DT_SIZE_NV"},
    {GL_TEXTURE_MAG_SIZE_NV,                               "GL_TEXTURE_MAG_SIZE_NV"},


    /*GL_NV_texture_shader2*/                              /*GL_NV_texture_shader2*/
    {GL_DOT_PRODUCT_TEXTURE_3D_NV,                         "GL_DOT_PRODUCT_TEXTURE_3D_NV"},


    /*GL_NV_vertex_array_range2*/                          /*GL_NV_vertex_array_range2*/
    {GL_VERTEX_ARRAY_RANGE_WITHOUT_FLUSH_NV,               "GL_VERTEX_ARRAY_RANGE_WITHOUT_FLUSH_NV"},


    /*GL_NV_vertex_program*/                               /*GL_NV_vertex_program*/
    {GL_VERTEX_PROGRAM_NV,                                 "GL_VERTEX_PROGRAM_NV"},
    {GL_VERTEX_STATE_PROGRAM_NV,                           "GL_VERTEX_STATE_PROGRAM_NV"},
    {GL_ATTRIB_ARRAY_SIZE_NV,                              "GL_ATTRIB_ARRAY_SIZE_NV"},
    {GL_ATTRIB_ARRAY_STRIDE_NV,                            "GL_ATTRIB_ARRAY_STRIDE_NV"},
    {GL_ATTRIB_ARRAY_TYPE_NV,                              "GL_ATTRIB_ARRAY_TYPE_NV"},
    {GL_CURRENT_ATTRIB_NV,                                 "GL_CURRENT_ATTRIB_NV"},
    {GL_PROGRAM_LENGTH_NV,                                 "GL_PROGRAM_LENGTH_NV"},
    {GL_PROGRAM_STRING_NV,                                 "GL_PROGRAM_STRING_NV"},
    {GL_MODELVIEW_PROJECTION_NV,                           "GL_MODELVIEW_PROJECTION_NV"},
    {GL_IDENTITY_NV,                                       "GL_IDENTITY_NV"},
    {GL_INVERSE_NV,                                        "GL_INVERSE_NV"},
    {GL_TRANSPOSE_NV,                                      "GL_TRANSPOSE_NV"},
    {GL_INVERSE_TRANSPOSE_NV,                              "GL_INVERSE_TRANSPOSE_NV"},
    {GL_MAX_TRACK_MATRIX_STACK_DEPTH_NV,                   "GL_MAX_TRACK_MATRIX_STACK_DEPTH_NV"},
    {GL_MAX_TRACK_MATRICES_NV,                             "GL_MAX_TRACK_MATRICES_NV"},
    {GL_MATRIX0_NV,                                        "GL_MATRIX0_NV"},
    {GL_MATRIX1_NV,                                        "GL_MATRIX1_NV"},
    {GL_MATRIX2_NV,                                        "GL_MATRIX2_NV"},
    {GL_MATRIX3_NV,                                        "GL_MATRIX3_NV"},
    {GL_MATRIX4_NV,                                        "GL_MATRIX4_NV"},
    {GL_MATRIX5_NV,                                        "GL_MATRIX5_NV"},
    {GL_MATRIX6_NV,                                        "GL_MATRIX6_NV"},
    {GL_MATRIX7_NV,                                        "GL_MATRIX7_NV"},
    {GL_CURRENT_MATRIX_STACK_DEPTH_NV,                     "GL_CURRENT_MATRIX_STACK_DEPTH_NV"},
    {GL_CURRENT_MATRIX_NV,                                 "GL_CURRENT_MATRIX_NV"},
    {GL_VERTEX_PROGRAM_POINT_SIZE_NV,                      "GL_VERTEX_PROGRAM_POINT_SIZE_NV"},
    {GL_VERTEX_PROGRAM_TWO_SIDE_NV,                        "GL_VERTEX_PROGRAM_TWO_SIDE_NV"},
    {GL_PROGRAM_PARAMETER_NV,                              "GL_PROGRAM_PARAMETER_NV"},
    {GL_ATTRIB_ARRAY_POINTER_NV,                           "GL_ATTRIB_ARRAY_POINTER_NV"},
    {GL_PROGRAM_TARGET_NV,                                 "GL_PROGRAM_TARGET_NV"},
    {GL_PROGRAM_RESIDENT_NV,                               "GL_PROGRAM_RESIDENT_NV"},
    {GL_TRACK_MATRIX_NV,                                   "GL_TRACK_MATRIX_NV"},
    {GL_TRACK_MATRIX_TRANSFORM_NV,                         "GL_TRACK_MATRIX_TRANSFORM_NV"},
    {GL_VERTEX_PROGRAM_BINDING_NV,                         "GL_VERTEX_PROGRAM_BINDING_NV"},
    {GL_PROGRAM_ERROR_POSITION_NV,                         "GL_PROGRAM_ERROR_POSITION_NV"},
    {GL_VERTEX_ATTRIB_ARRAY0_NV,                           "GL_VERTEX_ATTRIB_ARRAY0_NV"},
    {GL_VERTEX_ATTRIB_ARRAY1_NV,                           "GL_VERTEX_ATTRIB_ARRAY1_NV"},
    {GL_VERTEX_ATTRIB_ARRAY2_NV,                           "GL_VERTEX_ATTRIB_ARRAY2_NV"},
    {GL_VERTEX_ATTRIB_ARRAY3_NV,                           "GL_VERTEX_ATTRIB_ARRAY3_NV"},
    {GL_VERTEX_ATTRIB_ARRAY4_NV,                           "GL_VERTEX_ATTRIB_ARRAY4_NV"},
    {GL_VERTEX_ATTRIB_ARRAY5_NV,                           "GL_VERTEX_ATTRIB_ARRAY5_NV"},
    {GL_VERTEX_ATTRIB_ARRAY6_NV,                           "GL_VERTEX_ATTRIB_ARRAY6_NV"},
    {GL_VERTEX_ATTRIB_ARRAY7_NV,                           "GL_VERTEX_ATTRIB_ARRAY7_NV"},
    {GL_VERTEX_ATTRIB_ARRAY8_NV,                           "GL_VERTEX_ATTRIB_ARRAY8_NV"},
    {GL_VERTEX_ATTRIB_ARRAY9_NV,                           "GL_VERTEX_ATTRIB_ARRAY9_NV"},
    {GL_VERTEX_ATTRIB_ARRAY10_NV,                          "GL_VERTEX_ATTRIB_ARRAY10_NV"},
    {GL_VERTEX_ATTRIB_ARRAY11_NV,                          "GL_VERTEX_ATTRIB_ARRAY11_NV"},
    {GL_VERTEX_ATTRIB_ARRAY12_NV,                          "GL_VERTEX_ATTRIB_ARRAY12_NV"},
    {GL_VERTEX_ATTRIB_ARRAY13_NV,                          "GL_VERTEX_ATTRIB_ARRAY13_NV"},
    {GL_VERTEX_ATTRIB_ARRAY14_NV,                          "GL_VERTEX_ATTRIB_ARRAY14_NV"},
    {GL_VERTEX_ATTRIB_ARRAY15_NV,                          "GL_VERTEX_ATTRIB_ARRAY15_NV"},
    {GL_MAP1_VERTEX_ATTRIB0_4_NV,                          "GL_MAP1_VERTEX_ATTRIB0_4_NV"},
    {GL_MAP1_VERTEX_ATTRIB1_4_NV,                          "GL_MAP1_VERTEX_ATTRIB1_4_NV"},
    {GL_MAP1_VERTEX_ATTRIB2_4_NV,                          "GL_MAP1_VERTEX_ATTRIB2_4_NV"},
    {GL_MAP1_VERTEX_ATTRIB3_4_NV,                          "GL_MAP1_VERTEX_ATTRIB3_4_NV"},
    {GL_MAP1_VERTEX_ATTRIB4_4_NV,                          "GL_MAP1_VERTEX_ATTRIB4_4_NV"},
    {GL_MAP1_VERTEX_ATTRIB5_4_NV,                          "GL_MAP1_VERTEX_ATTRIB5_4_NV"},
    {GL_MAP1_VERTEX_ATTRIB6_4_NV,                          "GL_MAP1_VERTEX_ATTRIB6_4_NV"},
    {GL_MAP1_VERTEX_ATTRIB7_4_NV,                          "GL_MAP1_VERTEX_ATTRIB7_4_NV"},
    {GL_MAP1_VERTEX_ATTRIB8_4_NV,                          "GL_MAP1_VERTEX_ATTRIB8_4_NV"},
    {GL_MAP1_VERTEX_ATTRIB9_4_NV,                          "GL_MAP1_VERTEX_ATTRIB9_4_NV"},
    {GL_MAP1_VERTEX_ATTRIB10_4_NV,                         "GL_MAP1_VERTEX_ATTRIB10_4_NV"},
    {GL_MAP1_VERTEX_ATTRIB11_4_NV,                         "GL_MAP1_VERTEX_ATTRIB11_4_NV"},
    {GL_MAP1_VERTEX_ATTRIB12_4_NV,                         "GL_MAP1_VERTEX_ATTRIB12_4_NV"},
    {GL_MAP1_VERTEX_ATTRIB13_4_NV,                         "GL_MAP1_VERTEX_ATTRIB13_4_NV"},
    {GL_MAP1_VERTEX_ATTRIB14_4_NV,                         "GL_MAP1_VERTEX_ATTRIB14_4_NV"},
    {GL_MAP1_VERTEX_ATTRIB15_4_NV,                         "GL_MAP1_VERTEX_ATTRIB15_4_NV"},
    {GL_MAP2_VERTEX_ATTRIB0_4_NV,                          "GL_MAP2_VERTEX_ATTRIB0_4_NV"},
    {GL_MAP2_VERTEX_ATTRIB1_4_NV,                          "GL_MAP2_VERTEX_ATTRIB1_4_NV"},
    {GL_MAP2_VERTEX_ATTRIB2_4_NV,                          "GL_MAP2_VERTEX_ATTRIB2_4_NV"},
    {GL_MAP2_VERTEX_ATTRIB3_4_NV,                          "GL_MAP2_VERTEX_ATTRIB3_4_NV"},
    {GL_MAP2_VERTEX_ATTRIB4_4_NV,                          "GL_MAP2_VERTEX_ATTRIB4_4_NV"},
    {GL_MAP2_VERTEX_ATTRIB5_4_NV,                          "GL_MAP2_VERTEX_ATTRIB5_4_NV"},
    {GL_MAP2_VERTEX_ATTRIB6_4_NV,                          "GL_MAP2_VERTEX_ATTRIB6_4_NV"},
    {GL_MAP2_VERTEX_ATTRIB7_4_NV,                          "GL_MAP2_VERTEX_ATTRIB7_4_NV"},
    {GL_MAP2_VERTEX_ATTRIB8_4_NV,                          "GL_MAP2_VERTEX_ATTRIB8_4_NV"},
    {GL_MAP2_VERTEX_ATTRIB9_4_NV,                          "GL_MAP2_VERTEX_ATTRIB9_4_NV"},
    {GL_MAP2_VERTEX_ATTRIB10_4_NV,                         "GL_MAP2_VERTEX_ATTRIB10_4_NV"},
    {GL_MAP2_VERTEX_ATTRIB11_4_NV,                         "GL_MAP2_VERTEX_ATTRIB11_4_NV"},
    {GL_MAP2_VERTEX_ATTRIB12_4_NV,                         "GL_MAP2_VERTEX_ATTRIB12_4_NV"},
    {GL_MAP2_VERTEX_ATTRIB13_4_NV,                         "GL_MAP2_VERTEX_ATTRIB13_4_NV"},
    {GL_MAP2_VERTEX_ATTRIB14_4_NV,                         "GL_MAP2_VERTEX_ATTRIB14_4_NV"},
    {GL_MAP2_VERTEX_ATTRIB15_4_NV,                         "GL_MAP2_VERTEX_ATTRIB15_4_NV"},


    /*GL_SGIX_texture_coordinate_clamp*/                   /*GL_SGIX_texture_coordinate_clamp*/
    {GL_TEXTURE_MAX_CLAMP_S_SGIX,                          "GL_TEXTURE_MAX_CLAMP_S_SGIX"},
    {GL_TEXTURE_MAX_CLAMP_T_SGIX,                          "GL_TEXTURE_MAX_CLAMP_T_SGIX"},
    {GL_TEXTURE_MAX_CLAMP_R_SGIX,                          "GL_TEXTURE_MAX_CLAMP_R_SGIX"},


    /*GL_SGIX_scalebias_hint*/                             /*GL_SGIX_scalebias_hint*/
    {GL_SCALEBIAS_HINT_SGIX,                               "GL_SCALEBIAS_HINT_SGIX"},


    /*GL_OML_interlace*/                                   /*GL_OML_interlace*/
    {GL_INTERLACE_OML,                                     "GL_INTERLACE_OML"},
    {GL_INTERLACE_READ_OML,                                "GL_INTERLACE_READ_OML"},


    /*GL_OML_subsample*/                                   /*GL_OML_subsample*/
    {GL_FORMAT_SUBSAMPLE_24_24_OML,                        "GL_FORMAT_SUBSAMPLE_24_24_OML"},
    {GL_FORMAT_SUBSAMPLE_244_244_OML,                      "GL_FORMAT_SUBSAMPLE_244_244_OML"},


    /*GL_OML_resample*/                                    /*GL_OML_resample*/
    {GL_PACK_RESAMPLE_OML,                                 "GL_PACK_RESAMPLE_OML"},
    {GL_UNPACK_RESAMPLE_OML,                               "GL_UNPACK_RESAMPLE_OML"},
    {GL_RESAMPLE_REPLICATE_OML,                            "GL_RESAMPLE_REPLICATE_OML"},
    {GL_RESAMPLE_ZERO_FILL_OML,                            "GL_RESAMPLE_ZERO_FILL_OML"},
    {GL_RESAMPLE_AVERAGE_OML,                              "GL_RESAMPLE_AVERAGE_OML"},
    {GL_RESAMPLE_DECIMATE_OML,                             "GL_RESAMPLE_DECIMATE_OML"},


    /*GL_NV_copy_depth_to_color*/                          /*GL_NV_copy_depth_to_color*/
    {GL_DEPTH_STENCIL_TO_RGBA_NV,                          "GL_DEPTH_STENCIL_TO_RGBA_NV"},
    {GL_DEPTH_STENCIL_TO_BGRA_NV,                          "GL_DEPTH_STENCIL_TO_BGRA_NV"},


    /*GL_ATI_envmap_bumpmap*/                              /*GL_ATI_envmap_bumpmap*/
    {GL_BUMP_ROT_MATRIX_ATI,                               "GL_BUMP_ROT_MATRIX_ATI"},
    {GL_BUMP_ROT_MATRIX_SIZE_ATI,                          "GL_BUMP_ROT_MATRIX_SIZE_ATI"},
    {GL_BUMP_NUM_TEX_UNITS_ATI,                            "GL_BUMP_NUM_TEX_UNITS_ATI"},
    {GL_BUMP_TEX_UNITS_ATI,                                "GL_BUMP_TEX_UNITS_ATI"},
    {GL_DUDV_ATI,                                          "GL_DUDV_ATI"},
    {GL_DU8DV8_ATI,                                        "GL_DU8DV8_ATI"},
    {GL_BUMP_ENVMAP_ATI,                                   "GL_BUMP_ENVMAP_ATI"},
    {GL_BUMP_TARGET_ATI,                                   "GL_BUMP_TARGET_ATI"},

    /*GL_ATI_pn_triangles*/                                /*GL_ATI_pn_triangles*/
    {GL_PN_TRIANGLES_ATI,                                  "GL_PN_TRIANGLES_ATI"},
    {GL_MAX_PN_TRIANGLES_TESSELATION_LEVEL_ATI,            "GL_MAX_PN_TRIANGLES_TESSELATION_LEVEL_ATI"},
    {GL_PN_TRIANGLES_POINT_MODE_ATI,                       "GL_PN_TRIANGLES_POINT_MODE_ATI"},
    {GL_PN_TRIANGLES_NORMAL_MODE_ATI,                      "GL_PN_TRIANGLES_NORMAL_MODE_ATI"},
    {GL_PN_TRIANGLES_TESSELATION_LEVEL_ATI,                "GL_PN_TRIANGLES_TESSELATION_LEVEL_ATI"},
    {GL_PN_TRIANGLES_POINT_MODE_LINEAR_ATI,                "GL_PN_TRIANGLES_POINT_MODE_LINEAR_ATI"},
    {GL_PN_TRIANGLES_POINT_MODE_CUBIC_ATI,                 "GL_PN_TRIANGLES_POINT_MODE_CUBIC_ATI"},
    {GL_PN_TRIANGLES_NORMAL_MODE_LINEAR_ATI,               "GL_PN_TRIANGLES_NORMAL_MODE_LINEAR_ATI"},
    {GL_PN_TRIANGLES_NORMAL_MODE_QUADRATIC_ATI,            "GL_PN_TRIANGLES_NORMAL_MODE_QUADRATIC_ATI"},


    /*GL_ATI_vertex_array_object*/                         /*GL_ATI_vertex_array_object*/
    {GL_STATIC_ATI,                                        "GL_STATIC_ATI"},
    {GL_DYNAMIC_ATI,                                       "GL_DYNAMIC_ATI"},
    {GL_PRESERVE_ATI,                                      "GL_PRESERVE_ATI"},
    {GL_DISCARD_ATI,                                       "GL_DISCARD_ATI"},
    {GL_OBJECT_BUFFER_SIZE_ATI,                            "GL_OBJECT_BUFFER_SIZE_ATI"},
    {GL_OBJECT_BUFFER_USAGE_ATI,                           "GL_OBJECT_BUFFER_USAGE_ATI"},
    {GL_ARRAY_OBJECT_BUFFER_ATI,                           "GL_ARRAY_OBJECT_BUFFER_ATI"},
    {GL_ARRAY_OBJECT_OFFSET_ATI,                           "GL_ARRAY_OBJECT_OFFSET_ATI"},


    /*GL_ATI_vertex_streams*/                              /*GL_ATI_vertex_streams*/
    {GL_MAX_VERTEX_STREAMS_ATI,                            "GL_MAX_VERTEX_STREAMS_ATI"},
    {GL_VERTEX_STREAM0_ATI,                                "GL_VERTEX_STREAM0_ATI"},
    {GL_VERTEX_STREAM1_ATI,                                "GL_VERTEX_STREAM1_ATI"},
    {GL_VERTEX_STREAM2_ATI,                                "GL_VERTEX_STREAM2_ATI"},
    {GL_VERTEX_STREAM3_ATI,                                "GL_VERTEX_STREAM3_ATI"},
    {GL_VERTEX_STREAM4_ATI,                                "GL_VERTEX_STREAM4_ATI"},
    {GL_VERTEX_STREAM5_ATI,                                "GL_VERTEX_STREAM5_ATI"},
    {GL_VERTEX_STREAM6_ATI,                                "GL_VERTEX_STREAM6_ATI"},
    {GL_VERTEX_STREAM7_ATI,                                "GL_VERTEX_STREAM7_ATI"},
    {GL_VERTEX_SOURCE_ATI,                                 "GL_VERTEX_SOURCE_ATI"},


    /*GL_ATI_element_array*/                               /*GL_ATI_element_array*/
    {GL_ELEMENT_ARRAY_ATI,                                 "GL_ELEMENT_ARRAY_ATI"},
    {GL_ELEMENT_ARRAY_TYPE_ATI,                            "GL_ELEMENT_ARRAY_TYPE_ATI"},
    {GL_ELEMENT_ARRAY_POINTER_ATI,                         "GL_ELEMENT_ARRAY_POINTER_ATI"},


    /*GL_SUN_mesh_array*/                                  /*GL_SUN_mesh_array*/
    {GL_QUAD_MESH_SUN,                                     "GL_QUAD_MESH_SUN"},
    {GL_TRIANGLE_MESH_SUN,                                 "GL_TRIANGLE_MESH_SUN"},


    /*GL_SUN_slice_accum*/                                 /*GL_SUN_slice_accum*/
    {GL_SLICE_ACCUM_SUN,                                   "GL_SLICE_ACCUM_SUN"},


    /*GL_NV_multisample_filter_hint*/                      /*GL_NV_multisample_filter_hint*/
    {GL_MULTISAMPLE_FILTER_HINT_NV,                        "GL_MULTISAMPLE_FILTER_HINT_NV"},


    /*GL_NV_depth_clamp*/                                  /*GL_NV_depth_clamp*/
    {GL_DEPTH_CLAMP_NV,                                    "GL_DEPTH_CLAMP_NV"},


    /*GL_NV_occlusion_query*/                              /*GL_NV_occlusion_query*/
    {GL_PIXEL_COUNTER_BITS_NV,                             "GL_PIXEL_COUNTER_BITS_NV"},
    {GL_CURRENT_OCCLUSION_QUERY_ID_NV,                     "GL_CURRENT_OCCLUSION_QUERY_ID_NV"},
    {GL_PIXEL_COUNT_NV,                                    "GL_PIXEL_COUNT_NV"},
    {GL_PIXEL_COUNT_AVAILABLE_NV,                          "GL_PIXEL_COUNT_AVAILABLE_NV"},


    /*GL_NV_point_sprite*/                                 /*GL_NV_point_sprite*/
    {GL_POINT_SPRITE_NV,                                   "GL_POINT_SPRITE_NV"},
    {GL_COORD_REPLACE_NV,                                  "GL_COORD_REPLACE_NV"},
    {GL_POINT_SPRITE_R_MODE_NV,                            "GL_POINT_SPRITE_R_MODE_NV"},


    /*GL_NV_texture_shader3*/                              /*GL_NV_texture_shader3*/
    {GL_OFFSET_PROJECTIVE_TEXTURE_2D_NV,                   "GL_OFFSET_PROJECTIVE_TEXTURE_2D_NV"},
    {GL_OFFSET_PROJECTIVE_TEXTURE_2D_SCALE_NV,             "GL_OFFSET_PROJECTIVE_TEXTURE_2D_SCALE_NV"},
    {GL_OFFSET_PROJECTIVE_TEXTURE_RECTANGLE_NV,            "GL_OFFSET_PROJECTIVE_TEXTURE_RECTANGLE_NV"},
    {GL_OFFSET_PROJECTIVE_TEXTURE_RECTANGLE_SCALE_NV,      "GL_OFFSET_PROJECTIVE_TEXTURE_RECTANGLE_SCALE_NV"},
    {GL_OFFSET_HILO_TEXTURE_2D_NV,                         "GL_OFFSET_HILO_TEXTURE_2D_NV"},
    {GL_OFFSET_HILO_TEXTURE_RECTANGLE_NV,                  "GL_OFFSET_HILO_TEXTURE_RECTANGLE_NV"},
    {GL_OFFSET_HILO_PROJECTIVE_TEXTURE_2D_NV,              "GL_OFFSET_HILO_PROJECTIVE_TEXTURE_2D_NV"},
    {GL_OFFSET_HILO_PROJECTIVE_TEXTURE_RECTANGLE_NV,       "GL_OFFSET_HILO_PROJECTIVE_TEXTURE_RECTANGLE_NV"},
    {GL_DEPENDENT_HILO_TEXTURE_2D_NV,                      "GL_DEPENDENT_HILO_TEXTURE_2D_NV"},
    {GL_DEPENDENT_RGB_TEXTURE_3D_NV,                       "GL_DEPENDENT_RGB_TEXTURE_3D_NV"},
    {GL_DEPENDENT_RGB_TEXTURE_CUBE_MAP_NV,                 "GL_DEPENDENT_RGB_TEXTURE_CUBE_MAP_NV"},
    {GL_DOT_PRODUCT_PASS_THROUGH_NV,                       "GL_DOT_PRODUCT_PASS_THROUGH_NV"},
    {GL_DOT_PRODUCT_TEXTURE_1D_NV,                         "GL_DOT_PRODUCT_TEXTURE_1D_NV"},
    {GL_DOT_PRODUCT_AFFINE_DEPTH_REPLACE_NV,               "GL_DOT_PRODUCT_AFFINE_DEPTH_REPLACE_NV"},
    {GL_HILO8_NV,                                          "GL_HILO8_NV"},
    {GL_SIGNED_HILO8_NV,                                   "GL_SIGNED_HILO8_NV"},
    {GL_FORCE_BLUE_TO_ONE_NV,                              "GL_FORCE_BLUE_TO_ONE_NV"},


    /*GL_NV_vertex_program1_1*/                            /*GL_NV_vertex_program1_1*/


    /*GL_EXT_shadow_funcs*/                                /*GL_EXT_shadow_funcs*/


    /*GL_EXT_stencil_two_side*/                            /*GL_EXT_stencil_two_side*/
    {GL_STENCIL_TEST_TWO_SIDE_EXT,                         "GL_STENCIL_TEST_TWO_SIDE_EXT"},
    {GL_ACTIVE_STENCIL_FACE_EXT,                           "GL_ACTIVE_STENCIL_FACE_EXT"},


    /*GL_ATI_text_fragment_shader*/                        /*GL_ATI_text_fragment_shader*/
    {GL_TEXT_FRAGMENT_SHADER_ATI,                          "GL_TEXT_FRAGMENT_SHADER_ATI"},


    /*GL_APPLE_client_storage*/                            /*GL_APPLE_client_storage*/
    {GL_UNPACK_CLIENT_STORAGE_APPLE,                       "GL_UNPACK_CLIENT_STORAGE_APPLE"},


    /*GL_APPLE_element_array*/                             /*GL_APPLE_element_array*/
    {GL_ELEMENT_ARRAY_APPLE,                               "GL_ELEMENT_ARRAY_APPLE"},
    {GL_ELEMENT_ARRAY_TYPE_APPLE,                          "GL_ELEMENT_ARRAY_TYPE_APPLE"},
    {GL_ELEMENT_ARRAY_POINTER_APPLE,                       "GL_ELEMENT_ARRAY_POINTER_APPLE"},


    /*GL_APPLE_fence*/                                     /*GL_APPLE_fence*/
    {GL_DRAW_PIXELS_APPLE,                                 "GL_DRAW_PIXELS_APPLE"},
    {GL_FENCE_APPLE,                                       "GL_FENCE_APPLE"},


    /*GL_APPLE_vertex_array_object*/                       /*GL_APPLE_vertex_array_object*/
    {GL_VERTEX_ARRAY_BINDING_APPLE,                        "GL_VERTEX_ARRAY_BINDING_APPLE"},


    /*GL_APPLE_vertex_array_range*/                        /*GL_APPLE_vertex_array_range*/
    {GL_VERTEX_ARRAY_RANGE_APPLE,                          "GL_VERTEX_ARRAY_RANGE_APPLE"},
    {GL_VERTEX_ARRAY_RANGE_LENGTH_APPLE,                   "GL_VERTEX_ARRAY_RANGE_LENGTH_APPLE"},
    {GL_VERTEX_ARRAY_STORAGE_HINT_APPLE,                   "GL_VERTEX_ARRAY_STORAGE_HINT_APPLE"},
    {GL_VERTEX_ARRAY_RANGE_POINTER_APPLE,                  "GL_VERTEX_ARRAY_RANGE_POINTER_APPLE"},
    {GL_STORAGE_CACHED_APPLE,                              "GL_STORAGE_CACHED_APPLE"},
    {GL_STORAGE_SHARED_APPLE,                              "GL_STORAGE_SHARED_APPLE"},


    /*GL_APPLE_ycbcr_422*/                                 /*GL_APPLE_ycbcr_422*/
    {GL_YCBCR_422_APPLE,                                   "GL_YCBCR_422_APPLE"},
    {GL_UNSIGNED_SHORT_8_8_APPLE,                          "GL_UNSIGNED_SHORT_8_8_APPLE"},
    {GL_UNSIGNED_SHORT_8_8_REV_APPLE,                      "GL_UNSIGNED_SHORT_8_8_REV_APPLE"},


    /*GL_S3_s3tc*/                                         /*GL_S3_s3tc*/
    {GL_RGB_S3TC,                                          "GL_RGB_S3TC"},
    {GL_RGB4_S3TC,                                         "GL_RGB4_S3TC"},
    {GL_RGBA_S3TC,                                         "GL_RGBA_S3TC"},
    {GL_RGBA4_S3TC,                                        "GL_RGBA4_S3TC"},
#ifdef GL_S3_S3TC_EX
    {GL_COMPRESSED_LUMINANCE_S3TC,                         "GL_COMPRESSED_LUMINANCE_S3TC"},
    {GL_COMPRESSED_LUMINANCE_ALPHA_S3TC,                   "GL_COMPRESSED_LUMINANCE_ALPHA_S3TC"},
    {GL_COMPRESSED_RGB4_COMPRESSED_ALPHA4_S3TC,            "GL_COMPRESSED_RGB4_COMPRESSED_ALPHA4_S3TC"},
    {GL_COMPRESSED_RGB4_ALPHA4_S3TC,                       "GL_COMPRESSED_RGB4_ALPHA4_S3TC"},
    {GL_COMPRESSED_LUMINANCE4_S3TC,                        "GL_COMPRESSED_LUMINANCE4_S3TC"},
    {GL_COMPRESSED_LUMINANCE4_ALPHA_S3TC,                  "GL_COMPRESSED_LUMINANCE4_ALPHA_S3TC"},
#endif


    /*GL_ATI_draw_buffers*/                                /*GL_ATI_draw_buffers*/
    {GL_MAX_DRAW_BUFFERS_ATI,                              "GL_MAX_DRAW_BUFFERS_ATI"},
    {GL_DRAW_BUFFER0_ATI,                                  "GL_DRAW_BUFFER0_ATI"},
    {GL_DRAW_BUFFER1_ATI,                                  "GL_DRAW_BUFFER1_ATI"},
    {GL_DRAW_BUFFER2_ATI,                                  "GL_DRAW_BUFFER2_ATI"},
    {GL_DRAW_BUFFER3_ATI,                                  "GL_DRAW_BUFFER3_ATI"},
    {GL_DRAW_BUFFER4_ATI,                                  "GL_DRAW_BUFFER4_ATI"},
    {GL_DRAW_BUFFER5_ATI,                                  "GL_DRAW_BUFFER5_ATI"},
    {GL_DRAW_BUFFER6_ATI,                                  "GL_DRAW_BUFFER6_ATI"},
    {GL_DRAW_BUFFER7_ATI,                                  "GL_DRAW_BUFFER7_ATI"},
    {GL_DRAW_BUFFER8_ATI,                                  "GL_DRAW_BUFFER8_ATI"},
    {GL_DRAW_BUFFER9_ATI,                                  "GL_DRAW_BUFFER9_ATI"},
    {GL_DRAW_BUFFER10_ATI,                                 "GL_DRAW_BUFFER10_ATI"},
    {GL_DRAW_BUFFER11_ATI,                                 "GL_DRAW_BUFFER11_ATI"},
    {GL_DRAW_BUFFER12_ATI,                                 "GL_DRAW_BUFFER12_ATI"},
    {GL_DRAW_BUFFER13_ATI,                                 "GL_DRAW_BUFFER13_ATI"},
    {GL_DRAW_BUFFER14_ATI,                                 "GL_DRAW_BUFFER14_ATI"},
    {GL_DRAW_BUFFER15_ATI,                                 "GL_DRAW_BUFFER15_ATI"},


    /*GL_ATI_pixel_format_float*/                          /*GL_ATI_pixel_format_float*/
    {GL_RGBA_FLOAT_MODE_ATI,                               "GL_RGBA_FLOAT_MODE_ATI"},
    {GL_COLOR_CLEAR_UNCLAMPED_VALUE_ATI,                   "GL_COLOR_CLEAR_UNCLAMPED_VALUE_ATI"},


    /*GL_ATI_texture_env_combine3*/                        /*GL_ATI_texture_env_combine3*/
    {GL_MODULATE_ADD_ATI,                                  "GL_MODULATE_ADD_ATI"},
    {GL_MODULATE_SIGNED_ADD_ATI,                           "GL_MODULATE_SIGNED_ADD_ATI"},
    {GL_MODULATE_SUBTRACT_ATI,                             "GL_MODULATE_SUBTRACT_ATI"},


    /*GL_ATI_texture_float*/                               /*GL_ATI_texture_float*/
    {GL_RGBA_FLOAT32_ATI,                                  "GL_RGBA_FLOAT32_ATI"},
    {GL_RGB_FLOAT32_ATI,                                   "GL_RGB_FLOAT32_ATI"},
    {GL_ALPHA_FLOAT32_ATI,                                 "GL_ALPHA_FLOAT32_ATI"},
    {GL_INTENSITY_FLOAT32_ATI,                             "GL_INTENSITY_FLOAT32_ATI"},
    {GL_LUMINANCE_FLOAT32_ATI,                             "GL_LUMINANCE_FLOAT32_ATI"},
    {GL_LUMINANCE_ALPHA_FLOAT32_ATI,                       "GL_LUMINANCE_ALPHA_FLOAT32_ATI"},
    {GL_RGBA_FLOAT16_ATI,                                  "GL_RGBA_FLOAT16_ATI"},
    {GL_RGB_FLOAT16_ATI,                                   "GL_RGB_FLOAT16_ATI"},
    {GL_ALPHA_FLOAT16_ATI,                                 "GL_ALPHA_FLOAT16_ATI"},
    {GL_INTENSITY_FLOAT16_ATI,                             "GL_INTENSITY_FLOAT16_ATI"},
    {GL_LUMINANCE_FLOAT16_ATI,                             "GL_LUMINANCE_FLOAT16_ATI"},
    {GL_LUMINANCE_ALPHA_FLOAT16_ATI,                       "GL_LUMINANCE_ALPHA_FLOAT16_ATI"},


    /*GL_NV_float_buffer*/                                 /*GL_NV_float_buffer*/
    {GL_FLOAT_R_NV,                                        "GL_FLOAT_R_NV"},
    {GL_FLOAT_RG_NV,                                       "GL_FLOAT_RG_NV"},
    {GL_FLOAT_RGB_NV,                                      "GL_FLOAT_RGB_NV"},
    {GL_FLOAT_RGBA_NV,                                     "GL_FLOAT_RGBA_NV"},
    {GL_FLOAT_R16_NV,                                      "GL_FLOAT_R16_NV"},
    {GL_FLOAT_R32_NV,                                      "GL_FLOAT_R32_NV"},
    {GL_FLOAT_RG16_NV,                                     "GL_FLOAT_RG16_NV"},
    {GL_FLOAT_RG32_NV,                                     "GL_FLOAT_RG32_NV"},
    {GL_FLOAT_RGB16_NV,                                    "GL_FLOAT_RGB16_NV"},
    {GL_FLOAT_RGB32_NV,                                    "GL_FLOAT_RGB32_NV"},
    {GL_FLOAT_RGBA16_NV,                                   "GL_FLOAT_RGBA16_NV"},
    {GL_FLOAT_RGBA32_NV,                                   "GL_FLOAT_RGBA32_NV"},
    {GL_TEXTURE_FLOAT_COMPONENTS_NV,                       "GL_TEXTURE_FLOAT_COMPONENTS_NV"},
    {GL_FLOAT_CLEAR_COLOR_VALUE_NV,                        "GL_FLOAT_CLEAR_COLOR_VALUE_NV"},
    {GL_FLOAT_RGBA_MODE_NV,                                "GL_FLOAT_RGBA_MODE_NV"},


    /*GL_NV_fragment_program*/                             /*GL_NV_fragment_program*/
    {GL_MAX_FRAGMENT_PROGRAM_LOCAL_PARAMETERS_NV,          "GL_MAX_FRAGMENT_PROGRAM_LOCAL_PARAMETERS_NV"},
    {GL_FRAGMENT_PROGRAM_NV,                               "GL_FRAGMENT_PROGRAM_NV"},
    {GL_MAX_TEXTURE_COORDS_NV,                             "GL_MAX_TEXTURE_COORDS_NV"},
    {GL_MAX_TEXTURE_IMAGE_UNITS_NV,                        "GL_MAX_TEXTURE_IMAGE_UNITS_NV"},
    {GL_FRAGMENT_PROGRAM_BINDING_NV,                       "GL_FRAGMENT_PROGRAM_BINDING_NV"},
    {GL_PROGRAM_ERROR_STRING_NV,                           "GL_PROGRAM_ERROR_STRING_NV"},


    /*GL_NV_half_float*/                                   /*GL_NV_half_float*/
    {GL_HALF_FLOAT_NV,                                     "GL_HALF_FLOAT_NV"},


    /*GL_NV_pixel_data_range*/                             /*GL_NV_pixel_data_range*/
    {GL_WRITE_PIXEL_DATA_RANGE_NV,                         "GL_WRITE_PIXEL_DATA_RANGE_NV"},
    {GL_READ_PIXEL_DATA_RANGE_NV,                          "GL_READ_PIXEL_DATA_RANGE_NV"},
    {GL_WRITE_PIXEL_DATA_RANGE_LENGTH_NV,                  "GL_WRITE_PIXEL_DATA_RANGE_LENGTH_NV"},
    {GL_READ_PIXEL_DATA_RANGE_LENGTH_NV,                   "GL_READ_PIXEL_DATA_RANGE_LENGTH_NV"},
    {GL_WRITE_PIXEL_DATA_RANGE_POINTER_NV,                 "GL_WRITE_PIXEL_DATA_RANGE_POINTER_NV"},
    {GL_READ_PIXEL_DATA_RANGE_POINTER_NV,                  "GL_READ_PIXEL_DATA_RANGE_POINTER_NV"},


    /*GL_NV_primitive_restart*/                            /*GL_NV_primitive_restart*/
    {GL_PRIMITIVE_RESTART_NV,                              "GL_PRIMITIVE_RESTART_NV"},
    {GL_PRIMITIVE_RESTART_INDEX_NV,                        "GL_PRIMITIVE_RESTART_INDEX_NV"},


    /*GL_NV_texture_expand_normal*/                        /*GL_NV_texture_expand_normal*/
    {GL_TEXTURE_UNSIGNED_REMAP_MODE_NV,                    "GL_TEXTURE_UNSIGNED_REMAP_MODE_NV"},


    /*GL_NV_vertex_program2*/                              /*GL_NV_vertex_program2*/


    /*GL_ATI_map_object_buffer*/                           /*GL_ATI_map_object_buffer*/


    /*GL_ATI_separate_stencil*/                            /*GL_ATI_separate_stencil*/
    {GL_STENCIL_BACK_FUNC_ATI,                             "GL_STENCIL_BACK_FUNC_ATI"},
    {GL_STENCIL_BACK_FAIL_ATI,                             "GL_STENCIL_BACK_FAIL_ATI"},
    {GL_STENCIL_BACK_PASS_DEPTH_FAIL_ATI,                  "GL_STENCIL_BACK_PASS_DEPTH_FAIL_ATI"},
    {GL_STENCIL_BACK_PASS_DEPTH_PASS_ATI,                  "GL_STENCIL_BACK_PASS_DEPTH_PASS_ATI"},


    /*GL_ATI_vertex_attrib_array_object*/                  /*GL_ATI_vertex_attrib_array_object*/


    /*GL_EXT_depth_bounds_test*/                           /*GL_EXT_depth_bounds_test*/
    {GL_DEPTH_BOUNDS_TEST_EXT,                             "GL_DEPTH_BOUNDS_TEST_EXT"},
    {GL_DEPTH_BOUNDS_EXT,                                  "GL_DEPTH_BOUNDS_EXT"},


    /*GL_EXT_texture_mirror_clamp*/                        /*GL_EXT_texture_mirror_clamp*/
    {GL_MIRROR_CLAMP_EXT,                                  "GL_MIRROR_CLAMP_EXT"},
    {GL_MIRROR_CLAMP_TO_EDGE_EXT,                          "GL_MIRROR_CLAMP_TO_EDGE_EXT"},
    {GL_MIRROR_CLAMP_TO_BORDER_EXT,                        "GL_MIRROR_CLAMP_TO_BORDER_EXT"},


    /*GL_EXT_blend_equation_separate*/                     /*GL_EXT_blend_equation_separate*/
    {GL_BLEND_EQUATION_ALPHA_EXT,                          "GL_BLEND_EQUATION_ALPHA_EXT"},


    /*GL_MESA_pack_invert*/                                /*GL_MESA_pack_invert*/
    {GL_PACK_INVERT_MESA,                                  "GL_PACK_INVERT_MESA"},


    /*GL_MESA_ycbcr_texture*/                              /*GL_MESA_ycbcr_texture*/
    {GL_UNSIGNED_SHORT_8_8_MESA,                           "GL_UNSIGNED_SHORT_8_8_MESA"},
    {GL_UNSIGNED_SHORT_8_8_REV_MESA,                       "GL_UNSIGNED_SHORT_8_8_REV_MESA"},
    {GL_YCBCR_MESA,                                        "GL_YCBCR_MESA"},


    /*GL_EXT_pixel_buffer_object*/                         /*GL_EXT_pixel_buffer_object*/
    {GL_PIXEL_PACK_BUFFER_EXT,                             "GL_PIXEL_PACK_BUFFER_EXT"},
    {GL_PIXEL_UNPACK_BUFFER_EXT,                           "GL_PIXEL_UNPACK_BUFFER_EXT"},
    {GL_PIXEL_PACK_BUFFER_BINDING_EXT,                     "GL_PIXEL_PACK_BUFFER_BINDING_EXT"},
    {GL_PIXEL_UNPACK_BUFFER_BINDING_EXT,                   "GL_PIXEL_UNPACK_BUFFER_BINDING_EXT"},


    /*GL_NV_fragment_program_option*/                      /*GL_NV_fragment_program_option*/


    /*GL_NV_fragment_program2*/                            /*GL_NV_fragment_program2*/
    {GL_MAX_PROGRAM_EXEC_INSTRUCTIONS_NV,                  "GL_MAX_PROGRAM_EXEC_INSTRUCTIONS_NV"},
    {GL_MAX_PROGRAM_CALL_DEPTH_NV,                         "GL_MAX_PROGRAM_CALL_DEPTH_NV"},
    {GL_MAX_PROGRAM_IF_DEPTH_NV,                           "GL_MAX_PROGRAM_IF_DEPTH_NV"},
    {GL_MAX_PROGRAM_LOOP_DEPTH_NV,                         "GL_MAX_PROGRAM_LOOP_DEPTH_NV"},
    {GL_MAX_PROGRAM_LOOP_COUNT_NV,                         "GL_MAX_PROGRAM_LOOP_COUNT_NV"},


    /*GL_EXT_framebuffer_object*/                          /*GL_EXT_framebuffer_object*/
    {GL_INVALID_FRAMEBUFFER_OPERATION_EXT,                 "GL_INVALID_FRAMEBUFFER_OPERATION_EXT"},
    {GL_FRAMEBUFFER_EXT,                                   "GL_FRAMEBUFFER_EXT"},
    {GL_RENDERBUFFER_EXT,                                  "GL_RENDERBUFFER_EXT"},
    {GL_STENCIL_INDEX1_EXT,                                "GL_STENCIL_INDEX1_EXT"},
    {GL_STENCIL_INDEX4_EXT,                                "GL_STENCIL_INDEX4_EXT"},
    {GL_STENCIL_INDEX8_EXT,                                "GL_STENCIL_INDEX8_EXT"},
    {GL_STENCIL_INDEX16_EXT,                               "GL_STENCIL_INDEX16_EXT"},
    {GL_RENDERBUFFER_WIDTH_EXT,                            "GL_RENDERBUFFER_WIDTH_EXT"},
    {GL_RENDERBUFFER_HEIGHT_EXT,                           "GL_RENDERBUFFER_HEIGHT_EXT"},
    {GL_RENDERBUFFER_INTERNAL_FORMAT_EXT,                  "GL_RENDERBUFFER_INTERNAL_FORMAT_EXT"},
    {GL_RENDERBUFFER_RED_SIZE_EXT,                         "GL_RENDERBUFFER_RED_SIZE_EXT"},
    {GL_RENDERBUFFER_GREEN_SIZE_EXT,                       "GL_RENDERBUFFER_GREEN_SIZE_EXT"},
    {GL_RENDERBUFFER_BLUE_SIZE_EXT,                        "GL_RENDERBUFFER_BLUE_SIZE_EXT"},
    {GL_RENDERBUFFER_ALPHA_SIZE_EXT,                       "GL_RENDERBUFFER_ALPHA_SIZE_EXT"},
    {GL_RENDERBUFFER_DEPTH_SIZE_EXT,                       "GL_RENDERBUFFER_DEPTH_SIZE_EXT"},
    {GL_RENDERBUFFER_STENCIL_SIZE_EXT,                     "GL_RENDERBUFFER_STENCIL_SIZE_EXT"},
    {GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT,            "GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT"},
    {GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT,            "GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT"},
    {GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT,          "GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT"},
    {GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT,   "GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT"},
    {GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT,     "GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT"},
    {GL_COLOR_ATTACHMENT0_EXT,                             "GL_COLOR_ATTACHMENT0_EXT"},
    {GL_COLOR_ATTACHMENT1_EXT,                             "GL_COLOR_ATTACHMENT1_EXT"},
    {GL_COLOR_ATTACHMENT2_EXT,                             "GL_COLOR_ATTACHMENT2_EXT"},
    {GL_COLOR_ATTACHMENT3_EXT,                             "GL_COLOR_ATTACHMENT3_EXT"},
    {GL_COLOR_ATTACHMENT4_EXT,                             "GL_COLOR_ATTACHMENT4_EXT"},
    {GL_COLOR_ATTACHMENT5_EXT,                             "GL_COLOR_ATTACHMENT5_EXT"},
    {GL_COLOR_ATTACHMENT6_EXT,                             "GL_COLOR_ATTACHMENT6_EXT"},
    {GL_COLOR_ATTACHMENT7_EXT,                             "GL_COLOR_ATTACHMENT7_EXT"},
    {GL_COLOR_ATTACHMENT8_EXT,                             "GL_COLOR_ATTACHMENT8_EXT"},
    {GL_COLOR_ATTACHMENT9_EXT,                             "GL_COLOR_ATTACHMENT9_EXT"},
    {GL_COLOR_ATTACHMENT10_EXT,                            "GL_COLOR_ATTACHMENT10_EXT"},
    {GL_COLOR_ATTACHMENT11_EXT,                            "GL_COLOR_ATTACHMENT11_EXT"},
    {GL_COLOR_ATTACHMENT12_EXT,                            "GL_COLOR_ATTACHMENT12_EXT"},
    {GL_COLOR_ATTACHMENT13_EXT,                            "GL_COLOR_ATTACHMENT13_EXT"},
    {GL_COLOR_ATTACHMENT14_EXT,                            "GL_COLOR_ATTACHMENT14_EXT"},
    {GL_COLOR_ATTACHMENT15_EXT,                            "GL_COLOR_ATTACHMENT15_EXT"},
    {GL_DEPTH_ATTACHMENT_EXT,                              "GL_DEPTH_ATTACHMENT_EXT"},
    {GL_STENCIL_ATTACHMENT_EXT,                            "GL_STENCIL_ATTACHMENT_EXT"},
    {GL_FRAMEBUFFER_COMPLETE_EXT,                          "GL_FRAMEBUFFER_COMPLETE_EXT"},
    {GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT,             "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT"},
    {GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT,     "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT"},
    {GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT,             "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT"},
    {GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT,                "GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT"},
    {GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT,            "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT"},
    {GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT,            "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT"},
    {GL_FRAMEBUFFER_UNSUPPORTED_EXT,                       "GL_FRAMEBUFFER_UNSUPPORTED_EXT"},
    {GL_FRAMEBUFFER_BINDING_EXT,                           "GL_FRAMEBUFFER_BINDING_EXT"},
    {GL_RENDERBUFFER_BINDING_EXT,                          "GL_RENDERBUFFER_BINDING_EXT"},
    {GL_MAX_COLOR_ATTACHMENTS_EXT,                         "GL_MAX_COLOR_ATTACHMENTS_EXT"},
    {GL_MAX_RENDERBUFFER_SIZE_EXT,                         "GL_MAX_RENDERBUFFER_SIZE_EXT"},

    /*GL_EXT_blindable_uniform*/
    {GL_MAX_VERTEX_BINDABLE_UNIFORMS_EXT,                  "GL_MAX_VERTEX_BINDABLE_UNIFORMS_EXT"},
    {GL_MAX_FRAGMENT_BINDABLE_UNIFORMS_EXT,                "GL_MAX_FRAGMENT_BINDABLE_UNIFORMS_EXT"},
    {GL_MAX_GEOMETRY_BINDABLE_UNIFORMS_EXT,                "GL_MAX_GEOMETRY_BINDABLE_UNIFORMS_EXT"},
    {GL_MAX_BINDABLE_UNIFORM_SIZE_EXT,                     "GL_MAX_BINDABLE_UNIFORM_SIZE_EXT"},
    {GL_UNIFORM_BUFFER_EXT,                                "GL_UNIFORM_BUFFER_EXT"},
    {GL_UNIFORM_BUFFER_BINDING_EXT,                        "GL_UNIFORM_BUFFER_BINDING_EXT"},

    /* GL_EXT_texture_array */
    {GL_TEXTURE_1D_ARRAY_EXT,                              "GL_TEXTURE_1D_ARRAY_EXT"},
    {GL_TEXTURE_2D_ARRAY_EXT,                              "GL_TEXTURE_2D_ARRAY_EXT"},
    {GL_PROXY_TEXTURE_1D_ARRAY_EXT,                        "GL_PROXY_TEXTURE_1D_ARRAY_EXT"},
    {GL_PROXY_TEXTURE_2D_ARRAY_EXT,                        "GL_PROXY_TEXTURE_1D_ARRAY_EXT"},
    {GL_TEXTURE_BINDING_1D_ARRAY_EXT,                      "GL_TEXTURE_BINDING_1D_ARRAY_EXT"},
    {GL_TEXTURE_BINDING_2D_ARRAY_EXT,                      "GL_TEXTURE_BINDING_2D_ARRAY_EXT"},
    {GL_MAX_ARRAY_TEXTURE_LAYERS_EXT,                      "GL_MAX_ARRAY_TEXTURE_LAYERS_EXT"},
    {GL_SAMPLER_1D_ARRAY_EXT,                              "GL_SAMPLER_1D_ARRAY_EXT"},
    {GL_SAMPLER_2D_ARRAY_EXT,                              "GL_SAMPLER_2D_ARRAY_EXT"},
    {GL_SAMPLER_1D_ARRAY_SHADOW_EXT,                       "GL_SAMPLER_1D_ARRAY_SHADOW_EXT"},
    {GL_SAMPLER_2D_ARRAY_SHADOW_EXT,                       "GL_SAMPLER_2D_ARRAY_SHADOW_EXT"},

    /* GL_EXT_texture_buffer_object */
    {GL_TEXTURE_BUFFER_EXT,                                "GL_TEXTURE_BUFFER_EXT"},
    {GL_MAX_TEXTURE_BUFFER_SIZE_EXT,                       "GL_MAX_TEXTURE_BUFFER_SIZE_EXT"},
    {GL_TEXTURE_BINDING_BUFFER_EXT,                        "GL_TEXTURE_BINDING_BUFFER_EXT"},
    {GL_TEXTURE_BUFFER_DATA_STORE_BINDING_EXT,             "GL_TEXTURE_BUFFER_DATA_STORE_BINDING_EXT"},
    {GL_TEXTURE_BUFFER_FORMAT_EXT,                         "GL_TEXTURE_BUFFER_FORMAT_EXT"},

    /* GL_EXT_timer_query */
    {GL_TIME_ELAPSED_EXT,                                   "GL_TIME_ELAPSED_EXT"},
};

static __GLenumName dbg_wglConstantNames[]=
{
    {0x2000,    "WGL_NUMBER_PIXEL_FORMATS_ARB"},
    {0x2001,    "WGL_DRAW_TO_WINDOW_ARB"},
    {0x2002,    "WGL_DRAW_TO_BITMAP_ARB"},
    {0x2003,    "WGL_ACCELERATION_ARB"},
    {0x2004,    "WGL_NEED_PALETTE_ARB"},
    {0x2005,    "WGL_NEED_SYSTEM_PALETTE_ARB"},
    {0x2006,    "WGL_SWAP_LAYER_BUFFERS_ARB"},
    {0x2007,    "WGL_SWAP_METHOD_ARB"},
    {0x2008,    "WGL_NUMBER_OVERLAYS_ARB"},
    {0x2009,    "WGL_NUMBER_UNDERLAYS_ARB"},
    {0x200A,    "WGL_TRANSPARENT_ARB"},
    {0x2037,    "WGL_TRANSPARENT_RED_VALUE_ARB"},
    {0x2038,    "WGL_TRANSPARENT_GREEN_VALUE_ARB"},
    {0x2039,    "WGL_TRANSPARENT_BLUE_VALUE_ARB"},
    {0x203A,    "WGL_TRANSPARENT_ALPHA_VALUE_ARB"},
    {0x203B,    "WGL_TRANSPARENT_INDEX_VALUE_ARB"},
    {0x200C,    "WGL_SHARE_DEPTH_ARB"},
    {0x200D,    "WGL_SHARE_STENCIL_ARB"},
    {0x200E,    "WGL_SHARE_ACCUM_ARB"},
    {0x200F,    "WGL_SUPPORT_GDI_ARB"},
    {0x2010,    "WGL_SUPPORT_OPENGL_ARB"},
    {0x2011,    "WGL_DOUBLE_BUFFER_ARB"},
    {0x2012,    "WGL_STEREO_ARB"},
    {0x2013,    "WGL_PIXEL_TYPE_ARB"},
    {0x2014,    "WGL_COLOR_BITS_ARB"},
    {0x2015,    "WGL_RED_BITS_ARB"},
    {0x2016,    "WGL_RED_SHIFT_ARB"},
    {0x2017,    "WGL_GREEN_BITS_ARB"},
    {0x2018,    "WGL_GREEN_SHIFT_ARB"},
    {0x2019,    "WGL_BLUE_BITS_ARB"},
    {0x201A,    "WGL_BLUE_SHIFT_ARB"},
    {0x201B,    "WGL_ALPHA_BITS_ARB"},
    {0x201C,    "WGL_ALPHA_SHIFT_ARB"},
    {0x201D,    "WGL_ACCUM_BITS_ARB"},
    {0x201E,    "WGL_ACCUM_RED_BITS_ARB"},
    {0x201F,    "WGL_ACCUM_GREEN_BITS_ARB"},
    {0x2020,    "WGL_ACCUM_BLUE_BITS_ARB"},
    {0x2021,    "WGL_ACCUM_ALPHA_BITS_ARB"},
    {0x2022,    "WGL_DEPTH_BITS_ARB"},
    {0x2023,    "WGL_STENCIL_BITS_ARB"},
    {0x2024,    "WGL_AUX_BUFFERS_ARB"},
    {0x2025,    "WGL_NO_ACCELERATION_ARB"},
    {0x2026,    "WGL_GENERIC_ACCELERATION_ARB"},
    {0x2027,    "WGL_FULL_ACCELERATION_ARB"},
    {0x2028,    "WGL_SWAP_EXCHANGE_ARB"},
    {0x2029,    "WGL_SWAP_COPY_ARB"},
    {0x202A,    "WGL_SWAP_UNDEFINED_ARB"},
    {0x202B,    "WGL_TYPE_RGBA_ARB"},
    {0x202C,    "WGL_TYPE_COLORINDEX_ARB"},

    /*WGL_ARB_pbuffer*/
    {0x202D,    "WGL_DRAW_TO_PBUFFER_ARB"},
    {0x202E,    "WGL_MAX_PBUFFER_PIXELS_ARB"},
    {0x202F,    "WGL_MAX_PBUFFER_WIDTH_ARB"},
    {0x2030,    "WGL_MAX_PBUFFER_HEIGHT_ARB"},
    {0x2033,    "WGL_PBUFFER_LARGEST_ARB"},
    {0x2034,    "WGL_PBUFFER_WIDTH_ARB"},
    {0x2035,    "WGL_PBUFFER_HEIGHT_ARB"},
    {0x2036,    "WGL_PBUFFER_LOST_ARB"},

    /*WGL_EXT_depth_float*/
    {0x2040,    "WGL_DEPTH_FLOAT_EXT"},

    /*WGL_EXT_multisample*/
    {0x2041,    "WGL_SAMPLE_BUFFERS_EXT"},
    {0x2042,    "WGL_SAMPLES_EXT"},

    /*WGL_ARB_make_current_read*/
    {0x2043,        "ERROR_INVALID_PIXEL_TYPE_ARB"},

    /*WGL_I3D_genlock*/
    {0x2044,    "WGL_GENLOCK_SOURCE_MULTIVIEW_I3D"},
    {0x2045,    "WGL_GENLOCK_SOURCE_EXTENAL_SYNC_I3D"},
    {0x2046,    "WGL_GENLOCK_SOURCE_EXTENAL_FIELD_I3D"},
    {0x2047,    "WGL_GENLOCK_SOURCE_EXTENAL_TTL_I3D"},
    {0x2048,    "WGL_GENLOCK_SOURCE_DIGITAL_SYNC_I3D"},
    {0x2049,    "WGL_GENLOCK_SOURCE_DIGITAL_FIELD_I3D"},
    {0x204A,    "WGL_GENLOCK_SOURCE_EDGE_FALLING_I3D"},
    {0x204B,    "WGL_GENLOCK_SOURCE_EDGE_RISING_I3D"},
    {0x204C,    "WGL_GENLOCK_SOURCE_EDGE_BOTH_I3D"},

    /*WGL_I3D_gamma*/
    {0x204E,    "WGL_GAMMA_TABLE_SIZE_I3D"},
    {0x204F,    "WGL_GAMMA_EXCLUDE_DESKTOP_I3D"},

    /*WGL_I3D_digital_video_control*/
    {0x2050,    "WGL_DIGITAL_VIDEO_CURSOR_ALPHA_FRAMEBUFFER_I3D"},
    {0x2051,    "WGL_DIGITAL_VIDEO_CURSOR_ALPHA_VALUE_I3D"},
    {0x2052,    "WGL_DIGITAL_VIDEO_CURSOR_INCLUDED_I3D"},
    {0x2053,    "WGL_DIGITAL_VIDEO_GAMMA_CORRECTED_I3D"},

    {0x2054,    "ERROR_INCOMPATIBLE_DEVICE_CONTEXTS_ARB"},


    /*WGL_3DFX_multisample*/
    {0x2060,    "WGL_SAMPLE_BUFFERS_3DFX"},
    {0x2061,    "WGL_SAMPLES_3DFX"},

    /*WGL_ARB_render_texture*/
    {0x2070,    "WGL_BIND_TO_TEXTURE_RGB_ARB"},
    {0x2071,    "WGL_BIND_TO_TEXTURE_RGBA_ARB"},
    {0x2072,    "WGL_TEXTURE_FORMAT_ARB"},
    {0x2073,    "WGL_TEXTURE_TARGET_ARB"},
    {0x2074,    "WGL_MIPMAP_TEXTURE_ARB"},
    {0x2075,    "WGL_TEXTURE_RGB_ARB"},
    {0x2076,    "WGL_TEXTURE_RGBA_ARB"},
    {0x2077,    "WGL_NO_TEXTURE_ARB"},
    {0x2078,    "WGL_TEXTURE_CUBE_MAP_ARB"},
    {0x2079,    "WGL_TEXTURE_1D_ARB"},
    {0x207A,    "WGL_TEXTURE_2D_ARB"},
    {0x207B,    "WGL_MIPMAP_LEVEL_ARB"},
    {0x207C,    "WGL_CUBE_MAP_FACE_ARB"},
    {0x207D,    "WGL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB"},
    {0x207E,    "WGL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB"},
    {0x207F,    "WGL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB"},
    {0x2080,    "WGL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB"},
    {0x2081,    "WGL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB"},
    {0x2082,    "WGL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB"},
    {0x2083,    "WGL_FRONT_LEFT_ARB"},
    {0x2084,    "WGL_FRONT_RIGHT_ARB"},
    {0x2085,    "WGL_BACK_LEFT_ARB"},
    {0x2086,    "WGL_BACK_RIGHT_ARB"},
    {0x2087,    "WGL_AUX0_ARB"},
    {0x2088,    "WGL_AUX1_ARB"},
    {0x2089,    "WGL_AUX2_ARB"},
    {0x208A,    "WGL_AUX3_ARB"},
    {0x208B,    "WGL_AUX4_ARB"},
    {0x208C,    "WGL_AUX5_ARB"},
    {0x208D,    "WGL_AUX6_ARB"},
    {0x208E,    "WGL_AUX7_ARB"},
    {0x208F,    "WGL_AUX8_ARB"},
    {0x2090,    "WGL_AUX9_ARB"},

    /*WGL_NV_render_texture_rectangle*/
    {0x20A0,    "WGL_BIND_TO_TEXTURE_RECTANGLE_RGB_NV"},
    {0x20A1,    "WGL_BIND_TO_TEXTURE_RECTANGLE_RGBA_NV"},
    {0x20A2,    "WGL_TEXTURE_RECTANGLE_NV"},


    /*WGL_NV_render_depth_texture*/
    {0x20A3,    "WGL_BIND_TO_TEXTURE_DEPTH_NV"},
    {0x20A4,    "WGL_BIND_TO_TEXTURE_RECTANGLE_DEPTH_NV"},
    {0x20A5,    "WGL_DEPTH_TEXTURE_FORMAT_NV"},
    {0x20A6,    "WGL_TEXTURE_DEPTH_COMPONENT_NV"},
    {0x20A7,    "WGL_DEPTH_COMPONENT_NV"},

    /*WGL_NV_float_buffer*/
    {0x20B0,    "WGL_FLOAT_COMPONENTS_NV"},
    {0x20B1,    "WGL_BIND_TO_TEXTURE_RECTANGLE_FLOAT_R_NV"},
    {0x20B2,    "WGL_BIND_TO_TEXTURE_RECTANGLE_FLOAT_RG_NV"},
    {0x20B3,    "WGL_BIND_TO_TEXTURE_RECTANGLE_FLOAT_RGB_NV"},
    {0x20B4,    "WGL_BIND_TO_TEXTURE_RECTANGLE_FLOAT_RGBA_NV"},
    {0x20B5,    "WGL_TEXTURE_FLOAT_R_NV"},
    {0x20B6,    "WGL_TEXTURE_FLOAT_RG_NV"},
    {0x20B7,    "WGL_TEXTURE_FLOAT_RGB_NV"},
    {0x20B8,    "WGL_TEXTURE_FLOAT_RGBA_NV"},

    /*WGL_ATI_pixel_format_float*/
    {0x21A0,    "WGL_TYPE_RGBA_FLOAT_ATI"},
};

static __GLenumName dbg_texInternalFmtConstNames[] =
{
    {__GL_FMT_A8,                                         "__GL_DEVFMT_ALPHA8"},
    {__GL_FMT_A16,                                        "__GL_DEVFMT_ALPHA16"},
    {__GL_FMT_A24,                                        "__GL_DEVFMT_ALPHA24"},
    {__GL_FMT_A32F,                                       "__GL_DEVFMT_ALPHA32F"},
    {__GL_FMT_L8,                                         "__GL_DEVFMT_LUMINANCE8"},
    {__GL_FMT_L16,                                        "__GL_DEVFMT_LUMINANCE16"},
    {__GL_FMT_L24,                                        "__GL_DEVFMT_LUMINANCE24"},
    {__GL_FMT_L32F,                                       "__GL_DEVFMT_LUMINANCE32F"},
    {__GL_FMT_I8,                                         "__GL_DEVFMT_INTENSITY8"},
    {__GL_FMT_I16,                                        "__GL_DEVFMT_INTENSITY16"},
    {__GL_FMT_I24,                                        "__GL_DEVFMT_INTENSITY24"},
    {__GL_FMT_I32F,                                       "__GL_DEVFMT_INTENSITY32F"},
    {__GL_FMT_LA4,                                        "__GL_DEVFMT_LUMINANCE_ALPHA4"},
    {__GL_FMT_LA8,                                        "__GL_DEVFMT_LUMINANCE_ALPHA8"},
    {__GL_FMT_LA16,                                       "__GL_DEVFMT_LUMINANCE_ALPHA16"},
    {__GL_FMT_BGR565,                                     "__GL_DEVFMT_BGR565"},
    {__GL_FMT_BGRA4444,                                   "__GL_DEVFMT_BGRA4444"},
    {__GL_FMT_BGRA5551,                                   "__GL_DEVFMT_BGRA5551"},
    {__GL_FMT_BGRA,                                       "__GL_DEVFMT_BGRA8888"},
    {__GL_FMT_RGBA16,                                     "__GL_DEVFMT_RGBA16"},
    {__GL_FMT_BGRX8,                                      "__GL_DEVFMT_BGRX8888"},
    {__GL_FMT_BGRA1010102,                                "__GL_DEVFMT_BGRA1010102"},
    {__GL_FMT_Z24,                                        "__GL_DEVFMT_Z24"},
    {__GL_FMT_Z24S8,                                      "__GL_DEVFMT_Z24_STENCIL"},
    {__GL_FMT_Z32F,                                       "__GL_DEVFMT_Z32F"},
    {__GL_FMT_COMPRESSED_RGB_S3TC_DXT1_EXT,               "__GL_DEVFMT_COMPRESSED_RGB_DXT1"},
    {__GL_FMT_COMPRESSED_RGBA_S3TC_DXT1_EXT,              "__GL_DEVFMT_COMPRESSED_RGBA_DXT1"},
    {__GL_FMT_COMPRESSED_RGBA_S3TC_DXT3_EXT,              "__GL_DEVFMT_COMPRESSED_RGBA_DXT3"},
    {__GL_FMT_COMPRESSED_RGBA_S3TC_DXT5_EXT,              "__GL_DEVFMT_COMPRESSED_RGBA_DXT5"},
    {__GL_FMT_ZHIGH24,                                    "__GL_DEVFMT_ZHIGH24"},
    {__GL_FMT_ZHIGH24S8,                                  "__GL_DEVFMT_ZHIGH24_STENCIL"},
    {__GL_FMT_COMPRESSED_LUMINANCE_LATC1,                     "__GL_DEVFMT_COMPRESSED_LUMINANCE_LATC1"},
    {__GL_FMT_COMPRESSED_SIGNED_LUMINANCE_LATC1,              "__GL_DEVFMT_COMPRESSED_SIGNED_LUMINANCE_LATC1"},
    {__GL_FMT_COMPRESSED_LUMINANCE_ALPHA_LATC2,               "__GL_DEVFMT_COMPRESSED_LUMINANCE_ALPHA_LATC2"},
    {__GL_FMT_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2,        "__GL_DEVFMT_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2"},
    {__GL_FMT_COMPRESSED_RED_RGTC1,                           "__GL_DEVFMT_COMPRESSED_RED_RGTC1"},
    {__GL_FMT_COMPRESSED_SIGNED_RED_RGTC1,                    "__GL_DEVFMT_COMPRESSED_SIGNED_RED_RGTC1"},
    {__GL_FMT_COMPRESSED_RED_GREEN_RGTC2,                     "__GL_DEVFMT_COMPRESSED_RED_GREEN_RGTC2"},
    {__GL_FMT_COMPRESSED_SIGNED_RED_GREEN_RGTC2,              "__GL_DEVFMT_COMPRESSED_SIGNED_RED_GREEN_RGTC2"},
    {__GL_FMT_RGBA8,                                          "__GL_DEVFMT_RGBA8888"},
    {__GL_FMT_RGBA8888_SIGNED,                                "__GL_DEVFMT_RGBA8888_SIGNED"},
    {__GL_FMT_RGBA32UI,                                       "__GL_DEVFMT_RGBA32UI"},
    {__GL_FMT_RGBA32I,                                        "__GL_DEVFMT_RGBA32I"},
    {__GL_FMT_RGBA16UI,                                       "__GL_DEVFMT_RGBA16UI"},
    {__GL_FMT_RGBA16I,                                        "__GL_DEVFMT_RGBA16I"},
    {__GL_FMT_RGBA8UI,                                        "__GL_DEVFMT_RGBA8UI"},
    {__GL_FMT_RGBA8I,                                         "__GL_DEVFMT_RGBA8I"},
    {__GL_FMT_RGB32UI,                                        "__GL_DEVFMT_RGB32UI"},
    {__GL_FMT_RGB32I,                                         "__GL_DEVFMT_RGB32I"},
    {__GL_FMT_RGBA16F,                                        "__GL_DEVFMT_RGBA16F"},
    {__GL_FMT_RGBA32F,                                        "__GL_DEVFMT_RGBA32F"},
    {__GL_FMT_SRGB_ALPHA,                                     "__GL_DEVFMT_SRGB_ALPHA"},
    {__GL_FMT_COMPRESSED_SRGB_S3TC_DXT1,                      "__GL_DEVFMT_COMPRESSED_SRGB_S3TC_DXT1"},
    {__GL_FMT_COMPRESSED_SRGB_ALPHA_S3TC_DXT1,                "__GL_DEVFMT_COMPRESSED_SRGB_ALPHA_S3TC_DXT1"},
    {__GL_FMT_COMPRESSED_SRGB_ALPHA_S3TC_DXT3,                "__GL_DEVFMT_COMPRESSED_SRGB_ALPHA_S3TC_DXT3"},
    {__GL_FMT_COMPRESSED_SRGB_ALPHA_S3TC_DXT5,                "__GL_DEVFMT_COMPRESSED_SRGB_ALPHA_S3TC_DXT5"},
    {__GL_FMT_RGB9_E5,                                        "__GL_DEVFMT_RGB9_E5"},
    {__GL_FMT_MAX,                                            "__GL_FMT_MAX"},
};


static GLint C_DECL dbg_GLenumComp(const GLvoid *arg1, const GLvoid *arg2 )
{
    return (GLint)((__GLenumName*)arg1)->value - (GLint)((__GLenumName*)arg2)->value;
}

static const GLbyte* dbg_GetGLenumName(GLenum val)
{
    __GLenumName* result = NULL;
    __GLenumName  key;

    static GLuint sorted = 0;
    GLint num = sizeof(dbg_constNames) / sizeof(__GLenumName);

    if(0 == sorted )
    {
        qsort(dbg_constNames, num, sizeof(__GLenumName), dbg_GLenumComp);
        sorted = 1;
    }

    /*binary search*/
    key.value = val;

    result = (__GLenumName*)bsearch(&key, dbg_constNames, num, sizeof(__GLenumName), dbg_GLenumComp);

    if(result)
        return result->name;
    else
        return NULL;
}


static const GLbyte* dbg_WGLGetGLenumName(GLenum val)
{
    __GLenumName* result = NULL;
    __GLenumName  key;

    GLint num = sizeof(dbg_wglConstantNames) / sizeof(__GLenumName);

    /*binary search*/
    key.value = val;

    result = (__GLenumName*)bsearch(&key, dbg_wglConstantNames, num, sizeof(__GLenumName), dbg_GLenumComp);

    if(result)
        return result->name;
    else
        return NULL;
}
#endif
#endif