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


#include "gltypes.h"
#include "g_imfncs.h"
#include "g_lcfncs.h"
#include "g_disp.h"
#include "g_lefncs.h"
#include "gc_gl_context.h"
#include "gc_gl_debug.h"


GLvoid APIENTRY __glapi_Nop(GLvoid) {}

const GLubyte *__glle_Noop(__GLcontext *gc, const GLubyte *pc)
{
#ifdef DEBUG
    dbgError("******************  __glle_Noop() should not be called!!!\n");
#endif
    GL_ASSERT(0);
    return 0;
}

/*
** The following "#define"s match GL APIs that are NOT compiled into display list.
** They are defined as Noop function so that __glListCompileFuncTable can pass compilation.
** Their entries in __glListCompileFuncTable are overwriten by corresponding __glim_FuncName
** so that these API entries are executed immediately during display list compile time.
*/
#define __gllc_NewList                          ((__T_NewList                   )__glapi_Nop)
#define __gllc_EndList                          ((__T_EndList                   )__glapi_Nop)
#define __gllc_GenLists                         ((__T_GenLists                  )__glapi_Nop)
#define __gllc_DeleteLists                      ((__T_DeleteLists               )__glapi_Nop)
#define __gllc_FeedbackBuffer                   ((__T_FeedbackBuffer            )__glapi_Nop)
#define __gllc_SelectBuffer                     ((__T_SelectBuffer              )__glapi_Nop)
#define __gllc_RenderMode                       ((__T_RenderMode                )__glapi_Nop)
#define __gllc_ClientActiveTexture              ((__T_ClientActiveTexture       )__glapi_Nop)
#define __gllc_ColorPointer                     ((__T_ColorPointer              )__glapi_Nop)
#define __gllc_EdgeFlagPointer                  ((__T_EdgeFlagPointer           )__glapi_Nop)
#define __gllc_FogCoordPointer                  ((__T_FogCoordPointer           )__glapi_Nop)
#define __gllc_IndexPointer                     ((__T_IndexPointer              )__glapi_Nop)
#define __gllc_InterleavedArrays                ((__T_InterleavedArrays         )__glapi_Nop)
#define __gllc_NormalPointer                    ((__T_NormalPointer             )__glapi_Nop)
#define __gllc_SecondaryColorPointer            ((__T_SecondaryColorPointer     )__glapi_Nop)
#define __gllc_TexCoordPointer                  ((__T_TexCoordPointer           )__glapi_Nop)
#define __gllc_VertexAttribPointer              ((__T_VertexAttribPointer       )__glapi_Nop)
#define __gllc_VertexPointer                    ((__T_VertexPointer             )__glapi_Nop)
#define __gllc_EnableClientState                ((__T_EnableClientState         )__glapi_Nop)
#define __gllc_DisableClientState               ((__T_DisableClientState        )__glapi_Nop)
#define __gllc_EnableVertexAttribArray          ((__T_EnableVertexAttribArray   )__glapi_Nop)
#define __gllc_DisableVertexAttribArray         ((__T_DisableVertexAttribArray  )__glapi_Nop)
#define __gllc_PushClientAttrib                 ((__T_PushClientAttrib          )__glapi_Nop)
#define __gllc_PopClientAttrib                  ((__T_PopClientAttrib           )__glapi_Nop)
#define __gllc_PixelStorei                      ((__T_PixelStorei               )__glapi_Nop)
#define __gllc_PixelStoref                      ((__T_PixelStoref               )__glapi_Nop)
#define __gllc_ReadPixels                       ((__T_ReadPixels                )__glapi_Nop)
#define __gllc_GenTextures                      ((__T_GenTextures               )__glapi_Nop)
#define __gllc_DeleteTextures                   ((__T_DeleteTextures            )__glapi_Nop)
#define __gllc_AreTexturesResident              ((__T_AreTexturesResident       )__glapi_Nop)
#define __gllc_GenQueries                       ((__T_GenQueries                )__glapi_Nop)
#define __gllc_DeleteQueries                    ((__T_DeleteQueries             )__glapi_Nop)
#define __gllc_GenBuffers                       ((__T_GenBuffers                )__glapi_Nop)
#define __gllc_DeleteBuffers                    ((__T_DeleteBuffers             )__glapi_Nop)
#define __gllc_BindBuffer                       ((__T_BindBuffer                )__glapi_Nop)
#define __gllc_BufferData                       ((__T_BufferData                )__glapi_Nop)
#define __gllc_BufferSubData                    ((__T_BufferSubData             )__glapi_Nop)
#define __gllc_MapBuffer                        ((__T_MapBuffer                 )__glapi_Nop)
#define __gllc_UnmapBuffer                      ((__T_UnmapBuffer               )__glapi_Nop)
#define __gllc_CreateProgram                    ((__T_CreateProgram             )__glapi_Nop)
#define __gllc_CreateShader                     ((__T_CreateShader              )__glapi_Nop)
#define __gllc_DeleteProgram                    ((__T_DeleteProgram             )__glapi_Nop)
#define __gllc_DeleteShader                     ((__T_DeleteShader              )__glapi_Nop)
#define __gllc_AttachShader                     ((__T_AttachShader              )__glapi_Nop)
#define __gllc_DetachShader                     ((__T_DetachShader              )__glapi_Nop)
#define __gllc_BindAttribLocation               ((__T_BindAttribLocation        )__glapi_Nop)
#define __gllc_CompileShader                    ((__T_CompileShader             )__glapi_Nop)
#define __gllc_ShaderSource                     ((__T_ShaderSource              )__glapi_Nop)
#define __gllc_LinkProgram                      ((__T_LinkProgram               )__glapi_Nop)
#define __gllc_ValidateProgram                  ((__T_ValidateProgram           )__glapi_Nop)
#define __gllc_Flush                            ((__T_Flush                     )__glapi_Nop)
#define __gllc_Finish                           ((__T_Finish                    )__glapi_Nop)
#define __gllc_IsBuffer                         ((__T_IsBuffer                  )__glapi_Nop)
#define __gllc_IsShader                         ((__T_IsShader                  )__glapi_Nop)
#define __gllc_IsProgram                        ((__T_IsProgram                 )__glapi_Nop)
#define __gllc_IsTexture                        ((__T_IsTexture                 )__glapi_Nop)
#define __gllc_IsList                           ((__T_IsList                    )__glapi_Nop)
#define __gllc_IsEnabled                        ((__T_IsEnabled                 )__glapi_Nop)
#define __gllc_IsQuery                          ((__T_IsQuery                   )__glapi_Nop)
#define __gllc_GetBufferSubData                 ((__T_GetBufferSubData          )__glapi_Nop)
#define __gllc_GetPointerv                      ((__T_GetPointerv               )__glapi_Nop)
#define __gllc_GetVertexAttribPointerv          ((__T_GetVertexAttribPointerv   )__glapi_Nop)
#define __gllc_GetVertexAttribiv                ((__T_GetVertexAttribiv         )__glapi_Nop)
#define __gllc_GetVertexAttribfv                ((__T_GetVertexAttribfv         )__glapi_Nop)
#define __gllc_GetVertexAttribdv                ((__T_GetVertexAttribdv         )__glapi_Nop)
#define __gllc_GetUniformiv                     ((__T_GetUniformiv              )__glapi_Nop)
#define __gllc_GetUniformfv                     ((__T_GetUniformfv              )__glapi_Nop)
#define __gllc_GetUniformLocation               ((__T_GetUniformLocation        )__glapi_Nop)
#define __gllc_GetShaderSource                  ((__T_GetShaderSource           )__glapi_Nop)
#define __gllc_GetShaderInfoLog                 ((__T_GetShaderInfoLog          )__glapi_Nop)
#define __gllc_GetShaderiv                      ((__T_GetShaderiv               )__glapi_Nop)
#define __gllc_GetProgramInfoLog                ((__T_GetProgramInfoLog         )__glapi_Nop)
#define __gllc_GetProgramiv                     ((__T_GetProgramiv              )__glapi_Nop)
#define __gllc_GetAttribLocation                ((__T_GetAttribLocation         )__glapi_Nop)
#define __gllc_GetAttachedShaders               ((__T_GetAttachedShaders        )__glapi_Nop)
#define __gllc_GetActiveUniform                 ((__T_GetActiveUniform          )__glapi_Nop)
#define __gllc_GetActiveAttrib                  ((__T_GetActiveAttrib           )__glapi_Nop)
#define __gllc_GetBufferPointerv                ((__T_GetBufferPointerv         )__glapi_Nop)
#define __gllc_GetBufferParameteriv             ((__T_GetBufferParameteriv      )__glapi_Nop)
#define __gllc_GetQueryObjectuiv                ((__T_GetQueryObjectuiv         )__glapi_Nop)
#define __gllc_GetQueryObjectiv                 ((__T_GetQueryObjectiv          )__glapi_Nop)
#define __gllc_GetQueryiv                       ((__T_GetQueryiv                )__glapi_Nop)
#define __gllc_GetTexLevelParameteriv           ((__T_GetTexLevelParameteriv    )__glapi_Nop)
#define __gllc_GetTexLevelParameterfv           ((__T_GetTexLevelParameterfv    )__glapi_Nop)
#define __gllc_GetTexParameteriv                ((__T_GetTexParameteriv         )__glapi_Nop)
#define __gllc_GetTexParameterfv                ((__T_GetTexParameterfv         )__glapi_Nop)
#define __gllc_GetTexImage                      ((__T_GetTexImage               )__glapi_Nop)
#define __gllc_GetCompressedTexImage            ((__T_GetCompressedTexImage     )__glapi_Nop)
#define __gllc_GetTexGeniv                      ((__T_GetTexGeniv               )__glapi_Nop)
#define __gllc_GetTexGenfv                      ((__T_GetTexGenfv               )__glapi_Nop)
#define __gllc_GetTexGendv                      ((__T_GetTexGendv               )__glapi_Nop)
#define __gllc_GetTexEnviv                      ((__T_GetTexEnviv               )__glapi_Nop)
#define __gllc_GetTexEnvfv                      ((__T_GetTexEnvfv               )__glapi_Nop)
#define __gllc_GetString                        ((__T_GetString                 )__glapi_Nop)
#define __gllc_GetPolygonStipple                ((__T_GetPolygonStipple         )__glapi_Nop)
#define __gllc_GetPixelMapusv                   ((__T_GetPixelMapusv            )__glapi_Nop)
#define __gllc_GetPixelMapuiv                   ((__T_GetPixelMapuiv            )__glapi_Nop)
#define __gllc_GetPixelMapfv                    ((__T_GetPixelMapfv             )__glapi_Nop)
#define __gllc_GetMaterialiv                    ((__T_GetMaterialiv             )__glapi_Nop)
#define __gllc_GetMaterialfv                    ((__T_GetMaterialfv             )__glapi_Nop)
#define __gllc_GetMapiv                         ((__T_GetMapiv                  )__glapi_Nop)
#define __gllc_GetMapfv                         ((__T_GetMapfv                  )__glapi_Nop)
#define __gllc_GetMapdv                         ((__T_GetMapdv                  )__glapi_Nop)
#define __gllc_GetLightiv                       ((__T_GetLightiv                )__glapi_Nop)
#define __gllc_GetLightfv                       ((__T_GetLightfv                )__glapi_Nop)
#define __gllc_GetIntegerv                      ((__T_GetIntegerv               )__glapi_Nop)
#define __gllc_GetFloatv                        ((__T_GetFloatv                 )__glapi_Nop)
#define __gllc_GetError                         ((__T_GetError                  )__glapi_Nop)
#define __gllc_GetDoublev                       ((__T_GetDoublev                )__glapi_Nop)
#define __gllc_GetClipPlane                     ((__T_GetClipPlane              )__glapi_Nop)
#define __gllc_GetBooleanv                      ((__T_GetBooleanv               )__glapi_Nop)
#define __gllc_GetColorTable                    ((__T_GetColorTable             )__glapi_Nop)
#define __gllc_GetMinmaxParameteriv             ((__T_GetMinmaxParameteriv      )__glapi_Nop)
#define __gllc_GetColorTableParameteriv         ((__T_GetColorTableParameteriv  )__glapi_Nop)
#define __gllc_GetColorTableParameterfv         ((__T_GetColorTableParameterfv  )__glapi_Nop)
#define __gllc_GetMinmaxParameterfv             ((__T_GetMinmaxParameterfv      )__glapi_Nop)
#define __gllc_GetMinmax                        ((__T_GetMinmax                 )__glapi_Nop)
#define __gllc_GetHistogramParameteriv          ((__T_GetHistogramParameteriv   )__glapi_Nop)
#define __gllc_GetHistogramParameterfv          ((__T_GetHistogramParameterfv   )__glapi_Nop)
#define __gllc_GetHistogram                     ((__T_GetHistogram              )__glapi_Nop)
#define __gllc_GetSeparableFilter               ((__T_GetSeparableFilter        )__glapi_Nop)
#define __gllc_GetConvolutionParameteriv        ((__T_GetConvolutionParameteriv )__glapi_Nop)
#define __gllc_GetConvolutionParameterfv        ((__T_GetConvolutionParameterfv )__glapi_Nop)
#define __gllc_GetConvolutionFilter             ((__T_GetConvolutionFilter      )__glapi_Nop)

#if GL_ARB_vertex_program
#define __gllc_GenProgramsARB                   ((__T_GenProgramsARB                  )__glapi_Nop)
#define __gllc_DeleteProgramsARB                ((__T_DeleteProgramsARB               )__glapi_Nop)
#define __gllc_ProgramStringARB                 ((__T_ProgramStringARB                )__glapi_Nop)
#define __gllc_IsProgramARB                     ((__T_IsProgramARB                    )__glapi_Nop)
#define __gllc_GetProgramStringARB              ((__T_GetProgramStringARB             )__glapi_Nop)
#define __gllc_GetProgramivARB                  ((__T_GetProgramivARB                 )__glapi_Nop)
#define __gllc_GetProgramLocalParameterfvARB    ((__T_GetProgramLocalParameterfvARB   )__glapi_Nop)
#define __gllc_GetProgramLocalParameterdvARB    ((__T_GetProgramLocalParameterdvARB   )__glapi_Nop)
#define __gllc_GetProgramEnvParameterfvARB      ((__T_GetProgramEnvParameterfvARB     )__glapi_Nop)
#define __gllc_GetProgramEnvParameterdvARB      ((__T_GetProgramEnvParameterdvARB     )__glapi_Nop)
#endif

#if GL_ARB_shader_objects
#define __gllc_DeleteObjectARB                  ((__T_DeleteObjectARB          )__glapi_Nop)
#define __gllc_GetHandleARB                     ((__T_GetHandleARB             )__glapi_Nop)
#define __gllc_GetInfoLogARB                    ((__T_GetInfoLogARB            )__glapi_Nop)
#define __gllc_GetObjectParameterfvARB          ((__T_GetObjectParameterfvARB  )__glapi_Nop)
#define __gllc_GetObjectParameterivARB          ((__T_GetObjectParameterivARB  )__glapi_Nop)
#endif

#if GL_ATI_vertex_array_object
#define __gllc_NewObjectBufferATI               ((__T_NewObjectBufferATI           )__glapi_Nop)
#define __gllc_IsObjectBufferATI                ((__T_IsObjectBufferATI            )__glapi_Nop)
#define __gllc_UpdateObjectBufferATI            ((__T_UpdateObjectBufferATI        )__glapi_Nop)
#define __gllc_GetObjectBufferfvATI             ((__T_GetObjectBufferfvATI         )__glapi_Nop)
#define __gllc_GetObjectBufferivATI             ((__T_GetObjectBufferivATI         )__glapi_Nop)
#define __gllc_FreeObjectBufferATI              ((__T_FreeObjectBufferATI          )__glapi_Nop)
#define __gllc_ArrayObjectATI                   ((__T_ArrayObjectATI               )__glapi_Nop)
#define __gllc_GetArrayObjectfvATI              ((__T_GetArrayObjectfvATI          )__glapi_Nop)
#define __gllc_GetArrayObjectivATI              ((__T_GetArrayObjectivATI          )__glapi_Nop)
#define __gllc_VariantArrayObjectATI            ((__T_VariantArrayObjectATI        )__glapi_Nop)
#define __gllc_GetVariantArrayObjectfvATI       ((__T_GetVariantArrayObjectfvATI   )__glapi_Nop)
#define __gllc_GetVariantArrayObjectivATI       ((__T_GetVariantArrayObjectivATI   )__glapi_Nop)
#endif

#if GL_ATI_vertex_attrib_array_object
#define __gllc_VertexAttribArrayObjectATI       ((__T_VertexAttribArrayObjectATI       )__glapi_Nop)
#define __gllc_GetVertexAttribArrayObjectfvATI  ((__T_GetVertexAttribArrayObjectfvATI  )__glapi_Nop)
#define __gllc_GetVertexAttribArrayObjectivATI  ((__T_GetVertexAttribArrayObjectivATI  )__glapi_Nop)
#endif

#if GL_ATI_element_array
#define __gllc_ElementPointerATI                ((__T_ElementPointerATI )__glapi_Nop)
#endif

#define __gllc_AddSwapHintRectWIN               ((__T_AddSwapHintRectWIN )__glapi_Nop)

#if GL_EXT_framebuffer_object
#define __gllc_IsRenderbufferEXT                ((__T_IsRenderbufferEXT              )__glapi_Nop)
#define __gllc_BindRenderbufferEXT              ((__T_BindRenderbufferEXT            )__glapi_Nop)
#define __gllc_DeleteRenderbuffersEXT           ((__T_DeleteRenderbuffersEXT         )__glapi_Nop)
#define __gllc_GenRenderbuffersEXT              ((__T_GenRenderbuffersEXT            )__glapi_Nop)
#define __gllc_RenderbufferStorageEXT           ((__T_RenderbufferStorageEXT         )__glapi_Nop)
#define __gllc_GetRenderbufferParameterivEXT    ((__T_GetRenderbufferParameterivEXT  )__glapi_Nop)
#define __gllc_IsFramebufferEXT                 ((__T_IsFramebufferEXT               )__glapi_Nop)
#define __gllc_BindFramebufferEXT               ((__T_BindFramebufferEXT             )__glapi_Nop)
#define __gllc_DeleteFramebuffersEXT            ((__T_DeleteFramebuffersEXT          )__glapi_Nop)
#define __gllc_GenFramebuffersEXT               ((__T_GenFramebuffersEXT             )__glapi_Nop)
#define __gllc_CheckFramebufferStatusEXT        ((__T_CheckFramebufferStatusEXT      )__glapi_Nop)
#define __gllc_FramebufferTexture1DEXT          ((__T_FramebufferTexture1DEXT        )__glapi_Nop)
#define __gllc_FramebufferTexture2DEXT          ((__T_FramebufferTexture2DEXT        )__glapi_Nop)
#define __gllc_FramebufferTexture3DEXT          ((__T_FramebufferTexture3DEXT        )__glapi_Nop)
#define __gllc_FramebufferRenderbufferEXT       ((__T_FramebufferRenderbufferEXT     )__glapi_Nop)
#define __gllc_GetFramebufferAttachmentParameterivEXT ((__T_GetFramebufferAttachmentParameterivEXT )__glapi_Nop)
#define __gllc_GenerateMipmapEXT                ((__T_GenerateMipmapEXT )__glapi_Nop)
#if GL_EXT_framebuffer_blit
#define __gllc_BlitFramebufferEXT               ((__T_BlitFramebufferEXT )__glapi_Nop)
#if GL_EXT_framebuffer_multisample
#define __gllc_RenderbufferStorageMultisampleEXT ((__T_RenderbufferStorageMultisampleEXT )__glapi_Nop)
#endif
#endif
#endif

#if GL_EXT_bindable_uniform
#define __gllc_UniformBufferEXT                 ((__T_UniformBufferEXT         )__glapi_Nop)
#define __gllc_GetUniformBufferSizeEXT          ((__T_GetUniformBufferSizeEXT  )__glapi_Nop)
#define __gllc_GetUniformOffsetEXT              ((__T_GetUniformOffsetEXT      )__glapi_Nop)
#endif

#if GL_EXT_texture_integer
#define __gllc_GetTexParameterIivEXT            ((__T_GetTexParameterIivEXT   )__glapi_Nop)
#define __gllc_GetTexParameterIuivEXT           ((__T_GetTexParameterIuivEXT  )__glapi_Nop)
#endif

#if GL_EXT_gpu_shader4
#define __gllc_VertexAttribI1iEXT               ((__T_VertexAttribI1iEXT       )__glapi_Nop)
#define __gllc_VertexAttribI2iEXT               ((__T_VertexAttribI2iEXT       )__glapi_Nop)
#define __gllc_VertexAttribI3iEXT               ((__T_VertexAttribI3iEXT       )__glapi_Nop)
#define __gllc_VertexAttribI4iEXT               ((__T_VertexAttribI4iEXT       )__glapi_Nop)
#define __gllc_VertexAttribI1uiEXT              ((__T_VertexAttribI1uiEXT      )__glapi_Nop)
#define __gllc_VertexAttribI2uiEXT              ((__T_VertexAttribI2uiEXT      )__glapi_Nop)
#define __gllc_VertexAttribI3uiEXT              ((__T_VertexAttribI3uiEXT      )__glapi_Nop)
#define __gllc_VertexAttribI4uiEXT              ((__T_VertexAttribI4uiEXT      )__glapi_Nop)
#define __gllc_VertexAttribI1ivEXT              ((__T_VertexAttribI1ivEXT      )__glapi_Nop)
#define __gllc_VertexAttribI2ivEXT              ((__T_VertexAttribI2ivEXT      )__glapi_Nop)
#define __gllc_VertexAttribI3ivEXT              ((__T_VertexAttribI3ivEXT      )__glapi_Nop)
#define __gllc_VertexAttribI4ivEXT              ((__T_VertexAttribI4ivEXT      )__glapi_Nop)
#define __gllc_VertexAttribI1uivEXT             ((__T_VertexAttribI1uivEXT     )__glapi_Nop)
#define __gllc_VertexAttribI2uivEXT             ((__T_VertexAttribI2uivEXT     )__glapi_Nop)
#define __gllc_VertexAttribI3uivEXT             ((__T_VertexAttribI3uivEXT     )__glapi_Nop)
#define __gllc_VertexAttribI4uivEXT             ((__T_VertexAttribI4uivEXT     )__glapi_Nop)
#define __gllc_VertexAttribI4bvEXT              ((__T_VertexAttribI4bvEXT      )__glapi_Nop)
#define __gllc_VertexAttribI4svEXT              ((__T_VertexAttribI4svEXT      )__glapi_Nop)
#define __gllc_VertexAttribI4ubvEXT             ((__T_VertexAttribI4ubvEXT     )__glapi_Nop)
#define __gllc_VertexAttribI4usvEXT             ((__T_VertexAttribI4usvEXT     )__glapi_Nop)
#define __gllc_VertexAttribIPointerEXT          ((__T_VertexAttribIPointerEXT  )__glapi_Nop)
#define __gllc_GetVertexAttribIivEXT            ((__T_GetVertexAttribIivEXT    )__glapi_Nop)
#define __gllc_GetVertexAttribIuivEXT           ((__T_GetVertexAttribIuivEXT   )__glapi_Nop)
#define __gllc_Uniform1uiEXT                    ((__T_Uniform1uiEXT            )__glapi_Nop)
#define __gllc_Uniform2uiEXT                    ((__T_Uniform2uiEXT            )__glapi_Nop)
#define __gllc_Uniform3uiEXT                    ((__T_Uniform3uiEXT            )__glapi_Nop)
#define __gllc_Uniform4uiEXT                    ((__T_Uniform4uiEXT            )__glapi_Nop)
#define __gllc_Uniform1uivEXT                   ((__T_Uniform1uivEXT           )__glapi_Nop)
#define __gllc_Uniform2uivEXT                   ((__T_Uniform2uivEXT           )__glapi_Nop)
#define __gllc_Uniform3uivEXT                   ((__T_Uniform3uivEXT           )__glapi_Nop)
#define __gllc_Uniform4uivEXT                   ((__T_Uniform4uivEXT           )__glapi_Nop)
#define __gllc_GetUniformuivEXT                 ((__T_GetUniformuivEXT         )__glapi_Nop)
#define __gllc_BindFragDataLocationEXT          ((__T_BindFragDataLocationEXT  )__glapi_Nop)
#define __gllc_GetFragDataLocationEXT           ((__T_GetFragDataLocationEXT   )__glapi_Nop)
#endif

#if GL_EXT_bindable_uniform
#define __gllc_ProgramParameteriEXT             ((__T_ProgramParameteriEXT        )__glapi_Nop)
#define __gllc_FramebufferTextureEXT            ((__T_FramebufferTextureEXT       )__glapi_Nop)
#define __gllc_FramebufferTextureLayerEXT       ((__T_FramebufferTextureLayerEXT  )__glapi_Nop)
#define __gllc_FramebufferTextureFaceEXT        ((__T_FramebufferTextureFaceEXT   )__glapi_Nop)
#endif

#if GL_EXT_draw_buffers2
#define __gllc_IsEnabledIndexedEXT              ((__T_IsEnabledIndexedEXT    )__glapi_Nop)
#define __gllc_GetIntegerIndexedvEXT            ((__T_GetIntegerIndexedvEXT  )__glapi_Nop)
#define __gllc_GetBooleanIndexedvEXT            ((__T_GetBooleanIndexedvEXT  )__glapi_Nop)
#endif

#if GL_EXT_texture_buffer_object
#define __gllc_TexBufferEXT                     ((__T_TexBufferEXT )__glapi_Nop)
#endif

#if GL_EXT_draw_instanced
#define __gllc_DrawArraysInstancedEXT           ((__T_DrawArraysInstancedEXT    )__glapi_Nop)
#define __gllc_DrawElementsInstancedEXT         ((__T_DrawElementsInstancedEXT  )__glapi_Nop)
#endif

#if GL_EXT_timer_query
#define __gllc_GetQueryObjecti64vEXT            ((__T_GetQueryObjecti64vEXT    )__glapi_Nop)
#define __gllc_GetQueryObjectui64vEXT           ((__T_GetQueryObjectui64vEXT   )__glapi_Nop)
#endif


__GLdispatchState __glImmediateFuncTable = {
    OPENGL_VERSION_110_ENTRIES,
    {
        __GL_API_ENTRIES(glim)
    }
};

#if defined(_WIN32)
#pragma warning(disable: 4026)
#pragma warning(disable: 4716)
#pragma warning(disable: 4113)
#pragma warning(disable: 4133)
#pragma warning(disable: 4047)
#pragma warning(disable: 4028)
#endif


__GLdispatchState __glListCompileFuncTable = {
    OPENGL_VERSION_110_ENTRIES,
    {
        __GL_API_ENTRIES(gllc)
    }
};

#if defined(_WIN32)
#pragma warning(default: 4026)
#pragma warning(default: 4716)
#pragma warning(default: 4113)
#pragma warning(default: 4133)
#pragma warning(default: 4047)
#pragma warning(default: 4028)
#endif


const GLubyte * (*__glListExecFuncTable[])(const GLubyte *) = {
        __GL_LISTEXEC_ENTRIES(glle,)
};


GLvoid __glOverWriteListCompileTable(GLvoid)
{
    __GLdispatchTable *__gllc_Table = &__glListCompileFuncTable.dispatch;

    __gllc_Table->NewList = __glim_NewList;
    __gllc_Table->EndList = __glim_EndList;
    __gllc_Table->GenLists = __glim_GenLists;
    __gllc_Table->DeleteLists = __glim_DeleteLists;
    __gllc_Table->FeedbackBuffer = __glim_FeedbackBuffer;
    __gllc_Table->SelectBuffer = __glim_SelectBuffer;
    __gllc_Table->RenderMode = __glim_RenderMode;
    __gllc_Table->ClientActiveTexture = __glim_ClientActiveTexture;
    __gllc_Table->ColorPointer = __glim_ColorPointer;
    __gllc_Table->EdgeFlagPointer = __glim_EdgeFlagPointer;
    __gllc_Table->FogCoordPointer = __glim_FogCoordPointer;
    __gllc_Table->IndexPointer = __glim_IndexPointer;
    __gllc_Table->InterleavedArrays = __glim_InterleavedArrays;
    __gllc_Table->NormalPointer = __glim_NormalPointer;
    __gllc_Table->SecondaryColorPointer = __glim_SecondaryColorPointer;
    __gllc_Table->TexCoordPointer = __glim_TexCoordPointer;
    __gllc_Table->VertexAttribPointer = __glim_VertexAttribPointer;
    __gllc_Table->VertexPointer = __glim_VertexPointer;
    __gllc_Table->EnableClientState = __glim_EnableClientState;
    __gllc_Table->DisableClientState = __glim_DisableClientState;
    __gllc_Table->EnableVertexAttribArray = __glim_EnableVertexAttribArray;
    __gllc_Table->DisableVertexAttribArray = __glim_DisableVertexAttribArray;
    __gllc_Table->PushClientAttrib = __glim_PushClientAttrib;
    __gllc_Table->PopClientAttrib = __glim_PopClientAttrib;
    __gllc_Table->PixelStorei = __glim_PixelStorei;
    __gllc_Table->PixelStoref = __glim_PixelStoref;
    __gllc_Table->ReadPixels = __glim_ReadPixels;
    __gllc_Table->GenTextures = __glim_GenTextures;
    __gllc_Table->DeleteTextures = __glim_DeleteTextures;
    __gllc_Table->AreTexturesResident = __glim_AreTexturesResident;
    __gllc_Table->GenQueries = __glim_GenQueries;
    __gllc_Table->DeleteQueries = __glim_DeleteQueries;
    __gllc_Table->GenBuffers = __glim_GenBuffers;
    __gllc_Table->DeleteBuffers = __glim_DeleteBuffers;
    __gllc_Table->BindBuffer = __glim_BindBuffer;
    __gllc_Table->BufferData = __glim_BufferData;
    __gllc_Table->BufferSubData = __glim_BufferSubData;
    __gllc_Table->MapBuffer = __glim_MapBuffer;
    __gllc_Table->UnmapBuffer = __glim_UnmapBuffer;
    __gllc_Table->CreateProgram = __glim_CreateProgram;
    __gllc_Table->CreateShader = __glim_CreateShader;
    __gllc_Table->VertexPointer = __glim_VertexPointer;
    __gllc_Table->DeleteProgram = __glim_DeleteProgram;
    __gllc_Table->DeleteShader = __glim_DeleteShader;
    __gllc_Table->AttachShader = __glim_AttachShader;
    __gllc_Table->DetachShader = __glim_DetachShader;
    __gllc_Table->BindAttribLocation = __glim_BindAttribLocation;
    __gllc_Table->CompileShader = __glim_CompileShader;
    __gllc_Table->ShaderSource = __glim_ShaderSource;
    __gllc_Table->LinkProgram = __glim_LinkProgram;
    __gllc_Table->ValidateProgram = __glim_ValidateProgram;
    __gllc_Table->Flush = __glim_Flush;
    __gllc_Table->Finish = __glim_Finish;
    __gllc_Table->IsBuffer = __glim_IsBuffer;
    __gllc_Table->IsShader = __glim_IsShader;
    __gllc_Table->IsProgram = __glim_IsProgram;
    __gllc_Table->IsTexture = __glim_IsTexture;
    __gllc_Table->IsList = __glim_IsList;
    __gllc_Table->IsEnabled = __glim_IsEnabled;
    __gllc_Table->IsQuery = __glim_IsQuery;
    __gllc_Table->GetBufferSubData = __glim_GetBufferSubData;
    __gllc_Table->GetPointerv = __glim_GetPointerv;
    __gllc_Table->GetVertexAttribPointerv = __glim_GetVertexAttribPointerv;
    __gllc_Table->GetVertexAttribiv = __glim_GetVertexAttribiv;
    __gllc_Table->GetVertexAttribfv = __glim_GetVertexAttribfv;
    __gllc_Table->GetVertexAttribdv = __glim_GetVertexAttribdv;
    __gllc_Table->GetUniformiv = __glim_GetUniformiv;
    __gllc_Table->GetUniformfv = __glim_GetUniformfv;
    __gllc_Table->GetUniformLocation = __glim_GetUniformLocation;
    __gllc_Table->GetShaderSource = __glim_GetShaderSource;
    __gllc_Table->GetShaderInfoLog = __glim_GetShaderInfoLog;
    __gllc_Table->GetShaderiv = __glim_GetShaderiv;
    __gllc_Table->GetProgramInfoLog = __glim_GetProgramInfoLog;
    __gllc_Table->GetProgramiv = __glim_GetProgramiv;
    __gllc_Table->GetAttribLocation = __glim_GetAttribLocation;
    __gllc_Table->GetAttachedShaders = __glim_GetAttachedShaders;
    __gllc_Table->GetAttachedShaders = __glim_GetAttachedShaders;
    __gllc_Table->GetActiveUniform = __glim_GetActiveUniform;
    __gllc_Table->GetActiveUniform = __glim_GetActiveUniform;
    __gllc_Table->GetActiveUniform = __glim_GetActiveUniform;
    __gllc_Table->GetActiveAttrib = __glim_GetActiveAttrib;
    __gllc_Table->GetBufferPointerv = __glim_GetBufferPointerv;
    __gllc_Table->GetBufferParameteriv = __glim_GetBufferParameteriv;
    __gllc_Table->GetQueryObjectuiv = __glim_GetQueryObjectuiv;
    __gllc_Table->GetQueryObjectiv = __glim_GetQueryObjectiv;
    __gllc_Table->GetQueryiv = __glim_GetQueryiv;
    __gllc_Table->GetTexLevelParameteriv = __glim_GetTexLevelParameteriv;
    __gllc_Table->GetTexLevelParameterfv = __glim_GetTexLevelParameterfv;
    __gllc_Table->GetTexParameteriv = __glim_GetTexParameteriv;
    __gllc_Table->GetTexParameterfv = __glim_GetTexParameterfv;
    __gllc_Table->GetTexImage = __glim_GetTexImage;
    __gllc_Table->GetCompressedTexImage = __glim_GetCompressedTexImage;
    __gllc_Table->GetTexGeniv = __glim_GetTexGeniv;
    __gllc_Table->GetTexGenfv = __glim_GetTexGenfv;
    __gllc_Table->GetTexGendv = __glim_GetTexGendv;
    __gllc_Table->GetTexEnviv = __glim_GetTexEnviv;
    __gllc_Table->GetTexEnvfv = __glim_GetTexEnvfv;
    __gllc_Table->GetString = __glim_GetString;
    __gllc_Table->GetPolygonStipple = __glim_GetPolygonStipple;
    __gllc_Table->GetPixelMapusv = __glim_GetPixelMapusv;
    __gllc_Table->GetPixelMapuiv = __glim_GetPixelMapuiv;
    __gllc_Table->GetPixelMapfv = __glim_GetPixelMapfv;
    __gllc_Table->GetMaterialiv = __glim_GetMaterialiv;
    __gllc_Table->GetMaterialfv = __glim_GetMaterialfv;
    __gllc_Table->GetMapiv = __glim_GetMapiv;
    __gllc_Table->GetMapfv = __glim_GetMapfv;
    __gllc_Table->GetMapdv = __glim_GetMapdv;
    __gllc_Table->GetLightiv = __glim_GetLightiv;
    __gllc_Table->GetLightfv = __glim_GetLightfv;
    __gllc_Table->GetIntegerv = __glim_GetIntegerv;
    __gllc_Table->GetFloatv = __glim_GetFloatv;
    __gllc_Table->GetError = __glim_GetError;
    __gllc_Table->GetDoublev = __glim_GetDoublev;
    __gllc_Table->GetClipPlane = __glim_GetClipPlane;
    __gllc_Table->GetBooleanv = __glim_GetBooleanv;
    __gllc_Table->GetColorTable = __glim_GetColorTable;
    __gllc_Table->GetMinmaxParameteriv = __glim_GetMinmaxParameteriv;
    __gllc_Table->GetColorTableParameteriv = __glim_GetColorTableParameteriv;
    __gllc_Table->GetColorTableParameterfv = __glim_GetColorTableParameterfv;
    __gllc_Table->GetMinmaxParameterfv = __glim_GetMinmaxParameterfv;
    __gllc_Table->GetMinmax = __glim_GetMinmax;
    __gllc_Table->GetHistogramParameteriv = __glim_GetHistogramParameteriv;
    __gllc_Table->GetHistogramParameterfv = __glim_GetHistogramParameterfv;
    __gllc_Table->GetHistogram = __glim_GetHistogram;
    __gllc_Table->GetSeparableFilter = __glim_GetSeparableFilter;
    __gllc_Table->GetConvolutionParameteriv = __glim_GetConvolutionParameteriv;
    __gllc_Table->GetConvolutionParameterfv = __glim_GetConvolutionParameterfv;
    __gllc_Table->GetConvolutionFilter = __glim_GetConvolutionFilter;

#if GL_ARB_vertex_program
    __gllc_Table->GenProgramsARB = __glim_GenProgramsARB;
    __gllc_Table->DeleteProgramsARB = __glim_DeleteProgramsARB;
    __gllc_Table->ProgramStringARB = __glim_ProgramStringARB;
    __gllc_Table->IsProgramARB = __glim_IsProgramARB;
    __gllc_Table->GetProgramStringARB = __glim_GetProgramStringARB;
    __gllc_Table->GetProgramivARB = __glim_GetProgramivARB;
    __gllc_Table->GetProgramLocalParameterfvARB = __glim_GetProgramLocalParameterfvARB;
    __gllc_Table->GetProgramLocalParameterdvARB = __glim_GetProgramLocalParameterdvARB;
    __gllc_Table->GetProgramEnvParameterfvARB = __glim_GetProgramEnvParameterfvARB;
    __gllc_Table->GetProgramEnvParameterdvARB = __glim_GetProgramEnvParameterdvARB;
#endif

#if GL_ARB_shader_objects
    __gllc_Table->DeleteObjectARB = __glim_DeleteObjectARB;
    __gllc_Table->GetHandleARB = __glim_GetHandleARB;
    __gllc_Table->GetInfoLogARB = __glim_GetInfoLogARB;
    __gllc_Table->GetObjectParameterfvARB = __glim_GetObjectParameterfvARB;
    __gllc_Table->GetObjectParameterivARB = __glim_GetObjectParameterivARB;
#endif

#if GL_ATI_vertex_array_object
    __gllc_Table->NewObjectBufferATI  = __glim_NewObjectBufferATI;
    __gllc_Table->IsObjectBufferATI   = __glim_IsObjectBufferATI;
    __gllc_Table->UpdateObjectBufferATI = __glim_UpdateObjectBufferATI;
    __gllc_Table->GetObjectBufferfvATI = __glim_GetObjectBufferfvATI;
    __gllc_Table->GetObjectBufferivATI = __glim_GetObjectBufferivATI;
    __gllc_Table->FreeObjectBufferATI =  __glim_FreeObjectBufferATI;
    __gllc_Table->ArrayObjectATI = __glim_ArrayObjectATI;
    __gllc_Table->GetArrayObjectfvATI =  __glim_GetArrayObjectfvATI;
    __gllc_Table->GetArrayObjectivATI =  __glim_GetArrayObjectivATI;
    __gllc_Table->VariantArrayObjectATI =  __glim_VariantArrayObjectATI;
    __gllc_Table->GetVariantArrayObjectfvATI =  __glim_GetVariantArrayObjectfvATI;
    __gllc_Table->GetVariantArrayObjectivATI =  __glim_GetVariantArrayObjectivATI;
#endif

#if GL_ATI_element_array
    __gllc_Table->ElementPointerATI = __glim_ElementPointerATI;
#endif

    __gllc_Table->AddSwapHintRectWIN = __glim_AddSwapHintRectWIN;

 #if GL_EXT_framebuffer_object
    __gllc_Table->IsRenderbufferEXT = __glim_IsRenderbufferEXT;
    __gllc_Table->BindRenderbufferEXT = __glim_BindRenderbufferEXT;
    __gllc_Table->DeleteRenderbuffersEXT = __glim_DeleteRenderbuffersEXT;
    __gllc_Table->GenRenderbuffersEXT = __glim_GenRenderbuffersEXT;
    __gllc_Table->RenderbufferStorageEXT = __glim_RenderbufferStorageEXT;
    __gllc_Table->GetRenderbufferParameterivEXT = __glim_GetRenderbufferParameterivEXT;
    __gllc_Table->IsFramebufferEXT = __glim_IsFramebufferEXT;
    __gllc_Table->BindFramebufferEXT = __glim_BindFramebufferEXT;
    __gllc_Table->DeleteFramebuffersEXT = __glim_DeleteFramebuffersEXT;
    __gllc_Table->GenFramebuffersEXT = __glim_GenFramebuffersEXT;
    __gllc_Table->CheckFramebufferStatusEXT = __glim_CheckFramebufferStatusEXT;
    __gllc_Table->FramebufferTexture1DEXT = __glim_FramebufferTexture1DEXT;
    __gllc_Table->FramebufferTexture2DEXT = __glim_FramebufferTexture2DEXT;
    __gllc_Table->FramebufferTexture3DEXT = __glim_FramebufferTexture3DEXT;
    __gllc_Table->FramebufferRenderbufferEXT = __glim_FramebufferRenderbufferEXT;
    __gllc_Table->GetFramebufferAttachmentParameterivEXT = __glim_GetFramebufferAttachmentParameterivEXT;
    __gllc_Table->GenerateMipmapEXT = __glim_GenerateMipmapEXT;
    __gllc_Table->BlitFramebufferEXT = __glim_BlitFramebufferEXT;
    __gllc_Table->RenderbufferStorageMultisampleEXT = __glim_RenderbufferStorageMultisampleEXT;
#endif

#if GL_EXT_bindable_uniform
    __gllc_Table->UniformBufferEXT = __glim_UniformBufferEXT;
    __gllc_Table->GetUniformBufferSizeEXT = __glim_GetUniformBufferSizeEXT;
    __gllc_Table->GetUniformOffsetEXT = __glim_GetUniformOffsetEXT;
#endif

#if GL_EXT_texture_integer
    __gllc_Table->GetTexParameterIivEXT = __glim_GetTexParameterIivEXT;
    __gllc_Table->GetTexParameterIuivEXT = __glim_GetTexParameterIuivEXT;
#endif

#if GL_EXT_gpu_shader4
    __gllc_Table->VertexAttribI1iEXT = __glim_VertexAttribI1iEXT;
    __gllc_Table->VertexAttribI2iEXT = __glim_VertexAttribI2iEXT;
    __gllc_Table->VertexAttribI3iEXT = __glim_VertexAttribI3iEXT;
    __gllc_Table->VertexAttribI4iEXT = __glim_VertexAttribI4iEXT;
    __gllc_Table->VertexAttribI1uiEXT = __glim_VertexAttribI1uiEXT;
    __gllc_Table->VertexAttribI2uiEXT = __glim_VertexAttribI2uiEXT;
    __gllc_Table->VertexAttribI3uiEXT = __glim_VertexAttribI3uiEXT;
    __gllc_Table->VertexAttribI4uiEXT = __glim_VertexAttribI4uiEXT;
    __gllc_Table->VertexAttribI1ivEXT = __glim_VertexAttribI1ivEXT;
    __gllc_Table->VertexAttribI2ivEXT = __glim_VertexAttribI2ivEXT;
    __gllc_Table->VertexAttribI3ivEXT = __glim_VertexAttribI3ivEXT;
    __gllc_Table->VertexAttribI4ivEXT = __glim_VertexAttribI4ivEXT;
    __gllc_Table->VertexAttribI1uivEXT = __glim_VertexAttribI1uivEXT;
    __gllc_Table->VertexAttribI2uivEXT = __glim_VertexAttribI2uivEXT;
    __gllc_Table->VertexAttribI3uivEXT = __glim_VertexAttribI3uivEXT;
    __gllc_Table->VertexAttribI4uivEXT = __glim_VertexAttribI4uivEXT;
    __gllc_Table->VertexAttribI4bvEXT = __glim_VertexAttribI4bvEXT;
    __gllc_Table->VertexAttribI4svEXT = __glim_VertexAttribI4svEXT;
    __gllc_Table->VertexAttribI4ubvEXT = __glim_VertexAttribI4ubvEXT;
    __gllc_Table->VertexAttribI4usvEXT = __glim_VertexAttribI4usvEXT;
    __gllc_Table->VertexAttribIPointerEXT = __glim_VertexAttribIPointerEXT;
    __gllc_Table->GetVertexAttribIivEXT = __glim_GetVertexAttribIivEXT;
    __gllc_Table->GetVertexAttribIuivEXT = __glim_GetVertexAttribIuivEXT;
    __gllc_Table->Uniform1uiEXT = __glim_Uniform1uiEXT;
    __gllc_Table->Uniform2uiEXT = __glim_Uniform2uiEXT;
    __gllc_Table->Uniform3uiEXT = __glim_Uniform3uiEXT;
    __gllc_Table->Uniform4uiEXT = __glim_Uniform4uiEXT;
    __gllc_Table->Uniform1uivEXT = __glim_Uniform1uivEXT;
    __gllc_Table->Uniform2uivEXT = __glim_Uniform2uivEXT;
    __gllc_Table->Uniform3uivEXT = __glim_Uniform3uivEXT;
    __gllc_Table->Uniform4uivEXT = __glim_Uniform4uivEXT;
    __gllc_Table->GetUniformuivEXT = __glim_GetUniformuivEXT;
    __gllc_Table->BindFragDataLocationEXT = __glim_BindFragDataLocationEXT;
    __gllc_Table->GetFragDataLocationEXT = __glim_GetFragDataLocationEXT;
#endif

#if GL_EXT_geometry_shader4
    __gllc_Table->ProgramParameteriEXT = __glim_ProgramParameteriEXT;
    __gllc_Table->FramebufferTextureEXT = __glim_FramebufferTextureEXT;
    __gllc_Table->FramebufferTextureLayerEXT = __glim_FramebufferTextureLayerEXT;
    __gllc_Table->FramebufferTextureFaceEXT = __glim_FramebufferTextureFaceEXT;
#endif

#if GL_EXT_texture_buffer_object
    __gllc_Table->TexBufferEXT = __glim_TexBufferEXT;
#endif

#if GL_EXT_draw_instanced
    __gllc_Table->DrawArraysInstancedEXT = __glim_DrawArraysInstancedEXT;
    __gllc_Table->DrawElementsInstancedEXT = __glim_DrawElementsInstancedEXT;
#endif

#if GL_EXT_timer_query
    __gllc_Table->GetQueryObjecti64vEXT = __glim_GetQueryObjecti64vEXT;
    __gllc_Table->GetQueryObjectui64vEXT = __glim_GetQueryObjectui64vEXT;
#endif

}



