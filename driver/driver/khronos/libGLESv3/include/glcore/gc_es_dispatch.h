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


#ifndef __gc_es_dispatch_h__
#define __gc_es_dispatch_h__

#define __GLES_API_ENTRIES(esApiMacro) \
    esApiMacro(ActiveTexture), \
    esApiMacro(AttachShader), \
    esApiMacro(BindAttribLocation), \
    esApiMacro(BindBuffer), \
    esApiMacro(BindFramebuffer), \
    esApiMacro(BindRenderbuffer), \
    esApiMacro(BindTexture), \
    esApiMacro(BlendColor), \
    esApiMacro(BlendEquation), \
    esApiMacro(BlendEquationSeparate), \
    esApiMacro(BlendFunc), \
    esApiMacro(BlendFuncSeparate), \
    esApiMacro(BufferData), \
    esApiMacro(BufferSubData), \
    esApiMacro(CheckFramebufferStatus), \
    esApiMacro(Clear), \
    esApiMacro(ClearColor), \
    esApiMacro(ClearDepthf), \
    esApiMacro(ClearStencil), \
    esApiMacro(ColorMask), \
    esApiMacro(CompileShader), \
    esApiMacro(CompressedTexImage2D), \
    esApiMacro(CompressedTexSubImage2D), \
    esApiMacro(CopyTexImage2D), \
    esApiMacro(CopyTexSubImage2D), \
    esApiMacro(CreateProgram), \
    esApiMacro(CreateShader), \
    esApiMacro(CullFace), \
    esApiMacro(DeleteBuffers), \
    esApiMacro(DeleteFramebuffers), \
    esApiMacro(DeleteProgram), \
    esApiMacro(DeleteRenderbuffers), \
    esApiMacro(DeleteShader), \
    esApiMacro(DeleteTextures), \
    esApiMacro(DepthFunc), \
    esApiMacro(DepthMask), \
    esApiMacro(DepthRangef), \
    esApiMacro(DetachShader), \
    esApiMacro(Disable), \
    esApiMacro(DisableVertexAttribArray), \
    esApiMacro(DrawArrays), \
    esApiMacro(DrawElements), \
    esApiMacro(Enable), \
    esApiMacro(EnableVertexAttribArray), \
    esApiMacro(Finish), \
    esApiMacro(Flush), \
    esApiMacro(FramebufferRenderbuffer), \
    esApiMacro(FramebufferTexture2D), \
    esApiMacro(FrontFace), \
    esApiMacro(GenBuffers), \
    esApiMacro(GenerateMipmap), \
    esApiMacro(GenFramebuffers), \
    esApiMacro(GenRenderbuffers), \
    esApiMacro(GenTextures), \
    esApiMacro(GetActiveAttrib), \
    esApiMacro(GetActiveUniform), \
    esApiMacro(GetAttachedShaders), \
    esApiMacro(GetAttribLocation), \
    esApiMacro(GetBooleanv), \
    esApiMacro(GetBufferParameteriv), \
    esApiMacro(GetError), \
    esApiMacro(GetFloatv), \
    esApiMacro(GetFramebufferAttachmentParameteriv), \
    esApiMacro(GetIntegerv), \
    esApiMacro(GetProgramiv), \
    esApiMacro(GetProgramInfoLog), \
    esApiMacro(GetRenderbufferParameteriv), \
    esApiMacro(GetShaderiv), \
    esApiMacro(GetShaderInfoLog), \
    esApiMacro(GetShaderPrecisionFormat), \
    esApiMacro(GetShaderSource), \
    esApiMacro(GetString), \
    esApiMacro(GetTexParameterfv), \
    esApiMacro(GetTexParameteriv), \
    esApiMacro(GetUniformfv), \
    esApiMacro(GetUniformiv), \
    esApiMacro(GetUniformLocation), \
    esApiMacro(GetVertexAttribfv), \
    esApiMacro(GetVertexAttribiv), \
    esApiMacro(GetVertexAttribPointerv), \
    esApiMacro(Hint), \
    esApiMacro(IsBuffer), \
    esApiMacro(IsEnabled), \
    esApiMacro(IsFramebuffer), \
    esApiMacro(IsProgram), \
    esApiMacro(IsRenderbuffer), \
    esApiMacro(IsShader), \
    esApiMacro(IsTexture), \
    esApiMacro(LineWidth), \
    esApiMacro(LinkProgram), \
    esApiMacro(PixelStorei), \
    esApiMacro(PolygonOffset), \
    esApiMacro(ReadPixels), \
    esApiMacro(ReleaseShaderCompiler), \
    esApiMacro(RenderbufferStorage), \
    esApiMacro(SampleCoverage), \
    esApiMacro(Scissor), \
    esApiMacro(ShaderBinary), \
    esApiMacro(ShaderSource), \
    esApiMacro(StencilFunc), \
    esApiMacro(StencilFuncSeparate), \
    esApiMacro(StencilMask), \
    esApiMacro(StencilMaskSeparate), \
    esApiMacro(StencilOp), \
    esApiMacro(StencilOpSeparate), \
    esApiMacro(TexImage2D), \
    esApiMacro(TexParameterf), \
    esApiMacro(TexParameterfv), \
    esApiMacro(TexParameteri), \
    esApiMacro(TexParameteriv), \
    esApiMacro(TexSubImage2D), \
    esApiMacro(Uniform1f), \
    esApiMacro(Uniform1fv), \
    esApiMacro(Uniform1i), \
    esApiMacro(Uniform1iv), \
    esApiMacro(Uniform2f), \
    esApiMacro(Uniform2fv), \
    esApiMacro(Uniform2i), \
    esApiMacro(Uniform2iv), \
    esApiMacro(Uniform3f), \
    esApiMacro(Uniform3fv), \
    esApiMacro(Uniform3i), \
    esApiMacro(Uniform3iv), \
    esApiMacro(Uniform4f), \
    esApiMacro(Uniform4fv), \
    esApiMacro(Uniform4i), \
    esApiMacro(Uniform4iv), \
    esApiMacro(UniformMatrix2fv), \
    esApiMacro(UniformMatrix3fv), \
    esApiMacro(UniformMatrix4fv), \
    esApiMacro(UseProgram), \
    esApiMacro(ValidateProgram), \
    esApiMacro(VertexAttrib1f), \
    esApiMacro(VertexAttrib1fv), \
    esApiMacro(VertexAttrib2f), \
    esApiMacro(VertexAttrib2fv), \
    esApiMacro(VertexAttrib3f), \
    esApiMacro(VertexAttrib3fv), \
    esApiMacro(VertexAttrib4f), \
    esApiMacro(VertexAttrib4fv), \
    esApiMacro(VertexAttribPointer), \
    esApiMacro(Viewport), \
    /* OpenGL ES 3.0 */ \
    esApiMacro(ReadBuffer), \
    esApiMacro(DrawRangeElements), \
    esApiMacro(TexImage3D), \
    esApiMacro(TexSubImage3D), \
    esApiMacro(CopyTexSubImage3D), \
    esApiMacro(CompressedTexImage3D), \
    esApiMacro(CompressedTexSubImage3D), \
    esApiMacro(GenQueries), \
    esApiMacro(DeleteQueries), \
    esApiMacro(IsQuery), \
    esApiMacro(BeginQuery), \
    esApiMacro(EndQuery), \
    esApiMacro(GetQueryiv), \
    esApiMacro(GetQueryObjectuiv), \
    esApiMacro(UnmapBuffer), \
    esApiMacro(GetBufferPointerv), \
    esApiMacro(DrawBuffers), \
    esApiMacro(UniformMatrix2x3fv), \
    esApiMacro(UniformMatrix3x2fv), \
    esApiMacro(UniformMatrix2x4fv), \
    esApiMacro(UniformMatrix4x2fv), \
    esApiMacro(UniformMatrix3x4fv), \
    esApiMacro(UniformMatrix4x3fv), \
    esApiMacro(BlitFramebuffer), \
    esApiMacro(RenderbufferStorageMultisample), \
    esApiMacro(FramebufferTextureLayer), \
    esApiMacro(MapBufferRange), \
    esApiMacro(FlushMappedBufferRange), \
    esApiMacro(BindVertexArray), \
    esApiMacro(DeleteVertexArrays), \
    esApiMacro(GenVertexArrays), \
    esApiMacro(IsVertexArray), \
    esApiMacro(GetIntegeri_v), \
    esApiMacro(BeginTransformFeedback), \
    esApiMacro(EndTransformFeedback), \
    esApiMacro(BindBufferRange), \
    esApiMacro(BindBufferBase), \
    esApiMacro(TransformFeedbackVaryings), \
    esApiMacro(GetTransformFeedbackVarying), \
    esApiMacro(VertexAttribIPointer), \
    esApiMacro(GetVertexAttribIiv), \
    esApiMacro(GetVertexAttribIuiv), \
    esApiMacro(VertexAttribI4i), \
    esApiMacro(VertexAttribI4ui), \
    esApiMacro(VertexAttribI4iv), \
    esApiMacro(VertexAttribI4uiv), \
    esApiMacro(GetUniformuiv), \
    esApiMacro(GetFragDataLocation), \
    esApiMacro(Uniform1ui), \
    esApiMacro(Uniform2ui), \
    esApiMacro(Uniform3ui), \
    esApiMacro(Uniform4ui), \
    esApiMacro(Uniform1uiv), \
    esApiMacro(Uniform2uiv), \
    esApiMacro(Uniform3uiv), \
    esApiMacro(Uniform4uiv), \
    esApiMacro(ClearBufferiv), \
    esApiMacro(ClearBufferuiv), \
    esApiMacro(ClearBufferfv), \
    esApiMacro(ClearBufferfi), \
    esApiMacro(GetStringi), \
    esApiMacro(CopyBufferSubData), \
    esApiMacro(GetUniformIndices), \
    esApiMacro(GetActiveUniformsiv), \
    esApiMacro(GetUniformBlockIndex), \
    esApiMacro(GetActiveUniformBlockiv), \
    esApiMacro(GetActiveUniformBlockName), \
    esApiMacro(UniformBlockBinding), \
    esApiMacro(DrawArraysInstanced), \
    esApiMacro(DrawElementsInstanced), \
    esApiMacro(FenceSync), \
    esApiMacro(IsSync), \
    esApiMacro(DeleteSync), \
    esApiMacro(ClientWaitSync), \
    esApiMacro(WaitSync), \
    esApiMacro(GetInteger64v), \
    esApiMacro(GetSynciv), \
    esApiMacro(GetInteger64i_v), \
    esApiMacro(GetBufferParameteri64v), \
    esApiMacro(GenSamplers), \
    esApiMacro(DeleteSamplers), \
    esApiMacro(IsSampler), \
    esApiMacro(BindSampler), \
    esApiMacro(SamplerParameteri), \
    esApiMacro(SamplerParameteriv), \
    esApiMacro(SamplerParameterf), \
    esApiMacro(SamplerParameterfv), \
    esApiMacro(GetSamplerParameteriv), \
    esApiMacro(GetSamplerParameterfv), \
    esApiMacro(VertexAttribDivisor), \
    esApiMacro(BindTransformFeedback), \
    esApiMacro(DeleteTransformFeedbacks), \
    esApiMacro(GenTransformFeedbacks), \
    esApiMacro(IsTransformFeedback), \
    esApiMacro(PauseTransformFeedback), \
    esApiMacro(ResumeTransformFeedback), \
    esApiMacro(GetProgramBinary), \
    esApiMacro(ProgramBinary), \
    esApiMacro(ProgramParameteri), \
    esApiMacro(InvalidateFramebuffer), \
    esApiMacro(InvalidateSubFramebuffer), \
    esApiMacro(TexStorage2D), \
    esApiMacro(TexStorage3D), \
    esApiMacro(GetInternalformativ), \
    /* OpenGL ES 3.1 */ \
    esApiMacro(DispatchCompute), \
    esApiMacro(DispatchComputeIndirect), \
    esApiMacro(DrawArraysIndirect), \
    esApiMacro(DrawElementsIndirect), \
    esApiMacro(FramebufferParameteri), \
    esApiMacro(GetFramebufferParameteriv), \
    esApiMacro(GetProgramInterfaceiv), \
    esApiMacro(GetProgramResourceIndex), \
    esApiMacro(GetProgramResourceName), \
    esApiMacro(GetProgramResourceiv), \
    esApiMacro(GetProgramResourceLocation), \
    esApiMacro(UseProgramStages), \
    esApiMacro(ActiveShaderProgram), \
    esApiMacro(CreateShaderProgramv), \
    esApiMacro(BindProgramPipeline), \
    esApiMacro(DeleteProgramPipelines), \
    esApiMacro(GenProgramPipelines), \
    esApiMacro(IsProgramPipeline), \
    esApiMacro(GetProgramPipelineiv), \
    esApiMacro(ProgramUniform1i), \
    esApiMacro(ProgramUniform2i), \
    esApiMacro(ProgramUniform3i), \
    esApiMacro(ProgramUniform4i), \
    esApiMacro(ProgramUniform1ui), \
    esApiMacro(ProgramUniform2ui), \
    esApiMacro(ProgramUniform3ui), \
    esApiMacro(ProgramUniform4ui), \
    esApiMacro(ProgramUniform1f), \
    esApiMacro(ProgramUniform2f), \
    esApiMacro(ProgramUniform3f), \
    esApiMacro(ProgramUniform4f), \
    esApiMacro(ProgramUniform1iv), \
    esApiMacro(ProgramUniform2iv), \
    esApiMacro(ProgramUniform3iv), \
    esApiMacro(ProgramUniform4iv), \
    esApiMacro(ProgramUniform1uiv), \
    esApiMacro(ProgramUniform2uiv), \
    esApiMacro(ProgramUniform3uiv), \
    esApiMacro(ProgramUniform4uiv), \
    esApiMacro(ProgramUniform1fv), \
    esApiMacro(ProgramUniform2fv), \
    esApiMacro(ProgramUniform3fv), \
    esApiMacro(ProgramUniform4fv), \
    esApiMacro(ProgramUniformMatrix2fv), \
    esApiMacro(ProgramUniformMatrix3fv), \
    esApiMacro(ProgramUniformMatrix4fv), \
    esApiMacro(ProgramUniformMatrix2x3fv), \
    esApiMacro(ProgramUniformMatrix3x2fv), \
    esApiMacro(ProgramUniformMatrix2x4fv), \
    esApiMacro(ProgramUniformMatrix4x2fv), \
    esApiMacro(ProgramUniformMatrix3x4fv), \
    esApiMacro(ProgramUniformMatrix4x3fv), \
    esApiMacro(ValidateProgramPipeline), \
    esApiMacro(GetProgramPipelineInfoLog), \
    esApiMacro(BindImageTexture), \
    esApiMacro(GetBooleani_v), \
    esApiMacro(MemoryBarrier), \
    esApiMacro(MemoryBarrierByRegion), \
    esApiMacro(TexStorage2DMultisample), \
    esApiMacro(GetMultisamplefv), \
    esApiMacro(SampleMaski), \
    esApiMacro(GetTexLevelParameteriv), \
    esApiMacro(GetTexLevelParameterfv), \
    esApiMacro(BindVertexBuffer), \
    esApiMacro(VertexAttribFormat), \
    esApiMacro(VertexAttribIFormat), \
    esApiMacro(VertexAttribBinding), \
    esApiMacro(VertexBindingDivisor), \
    /* OpenGL ES 3.2 */ \
    esApiMacro(TexStorage3DMultisample), \
    esApiMacro(BlendBarrier), \
    esApiMacro(DebugMessageControl), \
    esApiMacro(DebugMessageInsert), \
    esApiMacro(DebugMessageCallback), \
    esApiMacro(GetDebugMessageLog), \
    esApiMacro(GetPointerv), \
    esApiMacro(PushDebugGroup), \
    esApiMacro(PopDebugGroup), \
    esApiMacro(ObjectLabel), \
    esApiMacro(GetObjectLabel), \
    esApiMacro(ObjectPtrLabel), \
    esApiMacro(GetObjectPtrLabel), \
    esApiMacro(GetGraphicsResetStatus), \
    esApiMacro(ReadnPixels), \
    esApiMacro(GetnUniformfv), \
    esApiMacro(GetnUniformiv), \
    esApiMacro(GetnUniformuiv), \
    esApiMacro(BlendEquationi), \
    esApiMacro(BlendEquationSeparatei), \
    esApiMacro(BlendFunci), \
    esApiMacro(BlendFuncSeparatei),\
    esApiMacro(ColorMaski), \
    esApiMacro(Enablei),  \
    esApiMacro(Disablei),\
    esApiMacro(IsEnabledi), \
    esApiMacro(TexParameterIiv),  \
    esApiMacro(TexParameterIuiv),  \
    esApiMacro(GetTexParameterIiv), \
    esApiMacro(GetTexParameterIuiv),  \
    esApiMacro(SamplerParameterIiv),  \
    esApiMacro(SamplerParameterIuiv),  \
    esApiMacro(GetSamplerParameterIiv),  \
    esApiMacro(GetSamplerParameterIuiv), \
    esApiMacro(TexBuffer), \
    esApiMacro(TexBufferRange), \
    esApiMacro(PatchParameteri), \
    esApiMacro(FramebufferTexture), \
    esApiMacro(MinSampleShading), \
    esApiMacro(CopyImageSubData), \
    esApiMacro(DrawElementsBaseVertex), \
    esApiMacro(DrawRangeElementsBaseVertex), \
    esApiMacro(DrawElementsInstancedBaseVertex), \
    esApiMacro(PrimitiveBoundingBox), \
    /* OpenGL ES extensions */ \
    /* GL_OES_EGL_image */ \
    esApiMacro(EGLImageTargetTexture2DOES), \
    esApiMacro(EGLImageTargetRenderbufferStorageOES), \
    /* GL_EXT_multi_draw_arrays */ \
    esApiMacro(MultiDrawArraysEXT), \
    esApiMacro(MultiDrawElementsEXT), \
    esApiMacro(MultiDrawElementsBaseVertexEXT), \
    /* GL_OES_mapbuffer */ \
    esApiMacro(GetBufferPointervOES), \
    esApiMacro(MapBufferOES), \
    esApiMacro(UnmapBufferOES), \
    /* GL_EXT_discard_framebuffer */ \
    esApiMacro(DiscardFramebufferEXT), \
    /* GL_EXT_multisampled_render_to_texture */ \
    esApiMacro(RenderbufferStorageMultisampleEXT), \
    esApiMacro(FramebufferTexture2DMultisampleEXT), \
    /* GL_VIV_direct_texture */ \
    esApiMacro(TexDirectVIV), \
    esApiMacro(TexDirectInvalidateVIV), \
    esApiMacro(TexDirectVIVMap), \
    esApiMacro(TexDirectTiledMapVIV), \
    /* GL_EXT_multi_draw_indirect */ \
    esApiMacro(MultiDrawArraysIndirectEXT), \
    esApiMacro(MultiDrawElementsIndirectEXT), \
    /* cl_khr_gl_sharing */ \
    esApiMacro(GetTexImage)



#define __GLES_API_DISPATCH_FUNCS \
    GLvoid         (GL_APIENTRY *ActiveTexture) (_gcArgComma_ GLenum texture); \
    GLvoid         (GL_APIENTRY *AttachShader) (_gcArgComma_ GLuint program, GLuint shader); \
    GLvoid         (GL_APIENTRY *BindAttribLocation) (_gcArgComma_ GLuint program, GLuint index, const GLchar* name); \
    GLvoid         (GL_APIENTRY *BindBuffer) (_gcArgComma_ GLenum target, GLuint buffer); \
    GLvoid         (GL_APIENTRY *BindFramebuffer) (_gcArgComma_ GLenum target, GLuint framebuffer); \
    GLvoid         (GL_APIENTRY *BindRenderbuffer) (_gcArgComma_ GLenum target, GLuint renderbuffer); \
    GLvoid         (GL_APIENTRY *BindTexture) (_gcArgComma_ GLenum target, GLuint texture); \
    GLvoid         (GL_APIENTRY *BlendColor) (_gcArgComma_ GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha); \
    GLvoid         (GL_APIENTRY *BlendEquation) (_gcArgComma_ GLenum mode); \
    GLvoid         (GL_APIENTRY *BlendEquationSeparate) (_gcArgComma_ GLenum modeRGB, GLenum modeAlpha); \
    GLvoid         (GL_APIENTRY *BlendFunc) (_gcArgComma_ GLenum sfactor, GLenum dfactor); \
    GLvoid         (GL_APIENTRY *BlendFuncSeparate) (_gcArgComma_ GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha); \
    GLvoid         (GL_APIENTRY *BufferData) (_gcArgComma_ GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage); \
    GLvoid         (GL_APIENTRY *BufferSubData) (_gcArgComma_ GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data); \
    GLenum         (GL_APIENTRY *CheckFramebufferStatus) (_gcArgComma_ GLenum target); \
    GLvoid         (GL_APIENTRY *Clear) (_gcArgComma_ GLbitfield mask); \
    GLvoid         (GL_APIENTRY *ClearColor) (_gcArgComma_ GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha); \
    GLvoid         (GL_APIENTRY *ClearDepthf) (_gcArgComma_ GLfloat depth); \
    GLvoid         (GL_APIENTRY *ClearStencil) (_gcArgComma_ GLint s); \
    GLvoid         (GL_APIENTRY *ColorMask) (_gcArgComma_ GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha); \
    GLvoid         (GL_APIENTRY *CompileShader) (_gcArgComma_ GLuint shader); \
    GLvoid         (GL_APIENTRY *CompressedTexImage2D) (_gcArgComma_ GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data); \
    GLvoid         (GL_APIENTRY *CompressedTexSubImage2D) (_gcArgComma_ GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data); \
    GLvoid         (GL_APIENTRY *CopyTexImage2D) (_gcArgComma_ GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border); \
    GLvoid         (GL_APIENTRY *CopyTexSubImage2D) (_gcArgComma_ GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height); \
    GLuint         (GL_APIENTRY *CreateProgram) (_gcArgOnly_ _retProgram_); \
    GLuint         (GL_APIENTRY *CreateShader) (_gcArgComma_ GLenum type _retShader_); \
    GLvoid         (GL_APIENTRY *CullFace) (_gcArgComma_ GLenum mode); \
    GLvoid         (GL_APIENTRY *DeleteBuffers) (_gcArgComma_ GLsizei n, const GLuint* buffers); \
    GLvoid         (GL_APIENTRY *DeleteFramebuffers) (_gcArgComma_ GLsizei n, const GLuint* framebuffers); \
    GLvoid         (GL_APIENTRY *DeleteProgram) (_gcArgComma_ GLuint program); \
    GLvoid         (GL_APIENTRY *DeleteRenderbuffers) (_gcArgComma_ GLsizei n, const GLuint* renderbuffers); \
    GLvoid         (GL_APIENTRY *DeleteShader) (_gcArgComma_ GLuint shader); \
    GLvoid         (GL_APIENTRY *DeleteTextures) (_gcArgComma_ GLsizei n, const GLuint* textures); \
    GLvoid         (GL_APIENTRY *DepthFunc) (_gcArgComma_ GLenum func); \
    GLvoid         (GL_APIENTRY *DepthMask) (_gcArgComma_ GLboolean flag); \
    GLvoid         (GL_APIENTRY *DepthRangef) (_gcArgComma_ GLfloat n, GLfloat f); \
    GLvoid         (GL_APIENTRY *DetachShader) (_gcArgComma_ GLuint program, GLuint shader); \
    GLvoid         (GL_APIENTRY *Disable) (_gcArgComma_ GLenum cap); \
    GLvoid         (GL_APIENTRY *DisableVertexAttribArray) (_gcArgComma_ GLuint index); \
    GLvoid         (GL_APIENTRY *DrawArrays) (_gcArgComma_ GLenum mode, GLint first, GLsizei count); \
    GLvoid         (GL_APIENTRY *DrawElements) (_gcArgComma_ GLenum mode, GLsizei count, GLenum type, const GLvoid* indices); \
    GLvoid         (GL_APIENTRY *Enable) (_gcArgComma_ GLenum cap); \
    GLvoid         (GL_APIENTRY *EnableVertexAttribArray) (_gcArgComma_ GLuint index); \
    GLvoid         (GL_APIENTRY *Finish) (_gcArgOnly_); \
    GLvoid         (GL_APIENTRY *Flush) (_gcArgOnly_); \
    GLvoid         (GL_APIENTRY *FramebufferRenderbuffer) (_gcArgComma_ GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer); \
    GLvoid         (GL_APIENTRY *FramebufferTexture2D) (_gcArgComma_ GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level); \
    GLvoid         (GL_APIENTRY *FrontFace) (_gcArgComma_ GLenum mode); \
    GLvoid         (GL_APIENTRY *GenBuffers) (_gcArgComma_ GLsizei n, GLuint* buffers); \
    GLvoid         (GL_APIENTRY *GenerateMipmap) (_gcArgComma_ GLenum target); \
    GLvoid         (GL_APIENTRY *GenFramebuffers) (_gcArgComma_ GLsizei n, GLuint* framebuffers); \
    GLvoid         (GL_APIENTRY *GenRenderbuffers) (_gcArgComma_ GLsizei n, GLuint* renderbuffers); \
    GLvoid         (GL_APIENTRY *GenTextures) (_gcArgComma_ GLsizei n, GLuint* textures); \
    GLvoid         (GL_APIENTRY *GetActiveAttrib) (_gcArgComma_ GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name); \
    GLvoid         (GL_APIENTRY *GetActiveUniform) (_gcArgComma_ GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name); \
    GLvoid         (GL_APIENTRY *GetAttachedShaders) (_gcArgComma_ GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders); \
    GLint          (GL_APIENTRY *GetAttribLocation) (_gcArgComma_ GLuint program, const GLchar* name _retLocation_); \
    GLvoid         (GL_APIENTRY *GetBooleanv) (_gcArgComma_ GLenum pname, GLboolean* params); \
    GLvoid         (GL_APIENTRY *GetBufferParameteriv) (_gcArgComma_ GLenum target, GLenum pname, GLint* params); \
    GLenum         (GL_APIENTRY *GetError) (_gcArgOnly_); \
    GLvoid         (GL_APIENTRY *GetFloatv) (_gcArgComma_ GLenum pname, GLfloat* params); \
    GLvoid         (GL_APIENTRY *GetFramebufferAttachmentParameteriv) (_gcArgComma_ GLenum target, GLenum attachment, GLenum pname, GLint* params); \
    GLvoid         (GL_APIENTRY *GetIntegerv) (_gcArgComma_ GLenum pname, GLint* params); \
    GLvoid         (GL_APIENTRY *GetProgramiv) (_gcArgComma_ GLuint program, GLenum pname, GLint* params); \
    GLvoid         (GL_APIENTRY *GetProgramInfoLog) (_gcArgComma_ GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog); \
    GLvoid         (GL_APIENTRY *GetRenderbufferParameteriv) (_gcArgComma_ GLenum target, GLenum pname, GLint* params); \
    GLvoid         (GL_APIENTRY *GetShaderiv) (_gcArgComma_ GLuint shader, GLenum pname, GLint* params); \
    GLvoid         (GL_APIENTRY *GetShaderInfoLog) (_gcArgComma_ GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog); \
    GLvoid         (GL_APIENTRY *GetShaderPrecisionFormat) (_gcArgComma_ GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision); \
    GLvoid         (GL_APIENTRY *GetShaderSource) (_gcArgComma_ GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source); \
    const GLubyte* (GL_APIENTRY *GetString) (_gcArgComma_ GLenum name); \
    GLvoid         (GL_APIENTRY *GetTexParameterfv) (_gcArgComma_ GLenum target, GLenum pname, GLfloat* params); \
    GLvoid         (GL_APIENTRY *GetTexParameteriv) (_gcArgComma_ GLenum target, GLenum pname, GLint* params); \
    GLvoid         (GL_APIENTRY *GetUniformfv) (_gcArgComma_ GLuint program, GLint location, GLfloat* params); \
    GLvoid         (GL_APIENTRY *GetUniformiv) (_gcArgComma_ GLuint program, GLint location, GLint* params); \
    GLint          (GL_APIENTRY *GetUniformLocation) (_gcArgComma_ GLuint program, const GLchar* name _retLocation_); \
    GLvoid         (GL_APIENTRY *GetVertexAttribfv) (_gcArgComma_ GLuint index, GLenum pname, GLfloat* params); \
    GLvoid         (GL_APIENTRY *GetVertexAttribiv) (_gcArgComma_ GLuint index, GLenum pname, GLint* params); \
    GLvoid         (GL_APIENTRY *GetVertexAttribPointerv) (_gcArgComma_ GLuint index, GLenum pname, GLvoid** pointer); \
    GLvoid         (GL_APIENTRY *Hint) (_gcArgComma_ GLenum target, GLenum mode); \
    GLboolean      (GL_APIENTRY *IsBuffer) (_gcArgComma_ GLuint buffer); \
    GLboolean      (GL_APIENTRY *IsEnabled) (_gcArgComma_ GLenum cap); \
    GLboolean      (GL_APIENTRY *IsFramebuffer) (_gcArgComma_ GLuint framebuffer); \
    GLboolean      (GL_APIENTRY *IsProgram) (_gcArgComma_ GLuint program); \
    GLboolean      (GL_APIENTRY *IsRenderbuffer) (_gcArgComma_ GLuint renderbuffer); \
    GLboolean      (GL_APIENTRY *IsShader) (_gcArgComma_ GLuint shader); \
    GLboolean      (GL_APIENTRY *IsTexture) (_gcArgComma_ GLuint texture); \
    GLvoid         (GL_APIENTRY *LineWidth) (_gcArgComma_ GLfloat width); \
    GLvoid         (GL_APIENTRY *LinkProgram) (_gcArgComma_ GLuint program); \
    GLvoid         (GL_APIENTRY *PixelStorei) (_gcArgComma_ GLenum pname, GLint param); \
    GLvoid         (GL_APIENTRY *PolygonOffset) (_gcArgComma_ GLfloat factor, GLfloat units); \
    GLvoid         (GL_APIENTRY *ReadPixels) (_gcArgComma_ GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels); \
    GLvoid         (GL_APIENTRY *ReleaseShaderCompiler) (_gcArgOnly_); \
    GLvoid         (GL_APIENTRY *RenderbufferStorage) (_gcArgComma_ GLenum target, GLenum internalformat, GLsizei width, GLsizei height); \
    GLvoid         (GL_APIENTRY *SampleCoverage) (_gcArgComma_ GLfloat value, GLboolean invert); \
    GLvoid         (GL_APIENTRY *Scissor) (_gcArgComma_ GLint x, GLint y, GLsizei width, GLsizei height); \
    GLvoid         (GL_APIENTRY *ShaderBinary) (_gcArgComma_ GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length); \
    GLvoid         (GL_APIENTRY *ShaderSource) (_gcArgComma_ GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length); \
    GLvoid         (GL_APIENTRY *StencilFunc) (_gcArgComma_ GLenum func, GLint ref, GLuint mask); \
    GLvoid         (GL_APIENTRY *StencilFuncSeparate) (_gcArgComma_ GLenum face, GLenum func, GLint ref, GLuint mask); \
    GLvoid         (GL_APIENTRY *StencilMask) (_gcArgComma_ GLuint mask); \
    GLvoid         (GL_APIENTRY *StencilMaskSeparate) (_gcArgComma_ GLenum face, GLuint mask); \
    GLvoid         (GL_APIENTRY *StencilOp) (_gcArgComma_ GLenum fail, GLenum zfail, GLenum zpass); \
    GLvoid         (GL_APIENTRY *StencilOpSeparate) (_gcArgComma_ GLenum face, GLenum fail, GLenum zfail, GLenum zpass); \
    GLvoid         (GL_APIENTRY *TexImage2D) (_gcArgComma_ GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels); \
    GLvoid         (GL_APIENTRY *TexParameterf) (_gcArgComma_ GLenum target, GLenum pname, GLfloat param); \
    GLvoid         (GL_APIENTRY *TexParameterfv) (_gcArgComma_ GLenum target, GLenum pname, const GLfloat* params); \
    GLvoid         (GL_APIENTRY *TexParameteri) (_gcArgComma_ GLenum target, GLenum pname, GLint param); \
    GLvoid         (GL_APIENTRY *TexParameteriv) (_gcArgComma_ GLenum target, GLenum pname, const GLint* params); \
    GLvoid         (GL_APIENTRY *TexSubImage2D) (_gcArgComma_ GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels); \
    GLvoid         (GL_APIENTRY *Uniform1f) (_gcArgComma_ GLint location, GLfloat x); \
    GLvoid         (GL_APIENTRY *Uniform1fv) (_gcArgComma_ GLint location, GLsizei count, const GLfloat* v); \
    GLvoid         (GL_APIENTRY *Uniform1i) (_gcArgComma_ GLint location, GLint x); \
    GLvoid         (GL_APIENTRY *Uniform1iv) (_gcArgComma_ GLint location, GLsizei count, const GLint* v); \
    GLvoid         (GL_APIENTRY *Uniform2f) (_gcArgComma_ GLint location, GLfloat x, GLfloat y); \
    GLvoid         (GL_APIENTRY *Uniform2fv) (_gcArgComma_ GLint location, GLsizei count, const GLfloat* v); \
    GLvoid         (GL_APIENTRY *Uniform2i) (_gcArgComma_ GLint location, GLint x, GLint y); \
    GLvoid         (GL_APIENTRY *Uniform2iv) (_gcArgComma_ GLint location, GLsizei count, const GLint* v); \
    GLvoid         (GL_APIENTRY *Uniform3f) (_gcArgComma_ GLint location, GLfloat x, GLfloat y, GLfloat z); \
    GLvoid         (GL_APIENTRY *Uniform3fv) (_gcArgComma_ GLint location, GLsizei count, const GLfloat* v); \
    GLvoid         (GL_APIENTRY *Uniform3i) (_gcArgComma_ GLint location, GLint x, GLint y, GLint z); \
    GLvoid         (GL_APIENTRY *Uniform3iv) (_gcArgComma_ GLint location, GLsizei count, const GLint* v); \
    GLvoid         (GL_APIENTRY *Uniform4f) (_gcArgComma_ GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w); \
    GLvoid         (GL_APIENTRY *Uniform4fv) (_gcArgComma_ GLint location, GLsizei count, const GLfloat* v); \
    GLvoid         (GL_APIENTRY *Uniform4i) (_gcArgComma_ GLint location, GLint x, GLint y, GLint z, GLint w); \
    GLvoid         (GL_APIENTRY *Uniform4iv) (_gcArgComma_ GLint location, GLsizei count, const GLint* v); \
    GLvoid         (GL_APIENTRY *UniformMatrix2fv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLfloat* value); \
    GLvoid         (GL_APIENTRY *UniformMatrix3fv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLfloat* value); \
    GLvoid         (GL_APIENTRY *UniformMatrix4fv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLfloat* value); \
    GLvoid         (GL_APIENTRY *UseProgram) (_gcArgComma_ GLuint program); \
    GLvoid         (GL_APIENTRY *ValidateProgram) (_gcArgComma_ GLuint program); \
    GLvoid         (GL_APIENTRY *VertexAttrib1f) (_gcArgComma_ GLuint indx, GLfloat x); \
    GLvoid         (GL_APIENTRY *VertexAttrib1fv) (_gcArgComma_ GLuint indx, const GLfloat* values); \
    GLvoid         (GL_APIENTRY *VertexAttrib2f) (_gcArgComma_ GLuint indx, GLfloat x, GLfloat y); \
    GLvoid         (GL_APIENTRY *VertexAttrib2fv) (_gcArgComma_ GLuint indx, const GLfloat* values); \
    GLvoid         (GL_APIENTRY *VertexAttrib3f) (_gcArgComma_ GLuint indx, GLfloat x, GLfloat y, GLfloat z); \
    GLvoid         (GL_APIENTRY *VertexAttrib3fv) (_gcArgComma_ GLuint indx, const GLfloat* values); \
    GLvoid         (GL_APIENTRY *VertexAttrib4f) (_gcArgComma_ GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w); \
    GLvoid         (GL_APIENTRY *VertexAttrib4fv) (_gcArgComma_ GLuint indx, const GLfloat* values); \
    GLvoid         (GL_APIENTRY *VertexAttribPointer) (_gcArgComma_ GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr); \
    GLvoid         (GL_APIENTRY *Viewport) (_gcArgComma_ GLint x, GLint y, GLsizei width, GLsizei height); \
    /* OpenGL ES 3.0 */ \
    GLvoid         (GL_APIENTRY *ReadBuffer) (_gcArgComma_ GLenum mode); \
    GLvoid         (GL_APIENTRY *DrawRangeElements) (_gcArgComma_ GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices); \
    GLvoid         (GL_APIENTRY *TexImage3D) (_gcArgComma_ GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels); \
    GLvoid         (GL_APIENTRY *TexSubImage3D) (_gcArgComma_ GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels); \
    GLvoid         (GL_APIENTRY *CopyTexSubImage3D) (_gcArgComma_ GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height); \
    GLvoid         (GL_APIENTRY *CompressedTexImage3D) (_gcArgComma_ GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data); \
    GLvoid         (GL_APIENTRY *CompressedTexSubImage3D) (_gcArgComma_ GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data); \
    GLvoid         (GL_APIENTRY *GenQueries) (_gcArgComma_ GLsizei n, GLuint* ids); \
    GLvoid         (GL_APIENTRY *DeleteQueries) (_gcArgComma_ GLsizei n, const GLuint* ids); \
    GLboolean      (GL_APIENTRY *IsQuery) (_gcArgComma_ GLuint id); \
    GLvoid         (GL_APIENTRY *BeginQuery) (_gcArgComma_ GLenum target, GLuint id); \
    GLvoid         (GL_APIENTRY *EndQuery) (_gcArgComma_ GLenum target); \
    GLvoid         (GL_APIENTRY *GetQueryiv) (_gcArgComma_ GLenum target, GLenum pname, GLint* params); \
    GLvoid         (GL_APIENTRY *GetQueryObjectuiv) (_gcArgComma_ GLuint id, GLenum pname, GLuint* params); \
    GLboolean      (GL_APIENTRY *UnmapBuffer) (_gcArgComma_ GLenum target); \
    GLvoid         (GL_APIENTRY *GetBufferPointerv) (_gcArgComma_ GLenum target, GLenum pname, GLvoid** params); \
    GLvoid         (GL_APIENTRY *DrawBuffers) (_gcArgComma_ GLsizei n, const GLenum* bufs); \
    GLvoid         (GL_APIENTRY *UniformMatrix2x3fv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLfloat* value); \
    GLvoid         (GL_APIENTRY *UniformMatrix3x2fv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLfloat* value); \
    GLvoid         (GL_APIENTRY *UniformMatrix2x4fv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLfloat* value); \
    GLvoid         (GL_APIENTRY *UniformMatrix4x2fv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLfloat* value); \
    GLvoid         (GL_APIENTRY *UniformMatrix3x4fv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLfloat* value); \
    GLvoid         (GL_APIENTRY *UniformMatrix4x3fv) (_gcArgComma_ GLint location, GLsizei count, GLboolean transpose, const GLfloat* value); \
    GLvoid         (GL_APIENTRY *BlitFramebuffer) (_gcArgComma_ GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter); \
    GLvoid         (GL_APIENTRY *RenderbufferStorageMultisample) (_gcArgComma_ GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height); \
    GLvoid         (GL_APIENTRY *FramebufferTextureLayer) (_gcArgComma_ GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer); \
    GLvoid*        (GL_APIENTRY *MapBufferRange) (_gcArgComma_ GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access _retPointer); \
    GLvoid         (GL_APIENTRY *FlushMappedBufferRange) (_gcArgComma_ GLenum target, GLintptr offset, GLsizeiptr length); \
    GLvoid         (GL_APIENTRY *BindVertexArray) (_gcArgComma_ GLuint array); \
    GLvoid         (GL_APIENTRY *DeleteVertexArrays) (_gcArgComma_ GLsizei n, const GLuint* arrays); \
    GLvoid         (GL_APIENTRY *GenVertexArrays) (_gcArgComma_ GLsizei n, GLuint* arrays); \
    GLboolean      (GL_APIENTRY *IsVertexArray) (_gcArgComma_ GLuint array); \
    GLvoid         (GL_APIENTRY *GetIntegeri_v) (_gcArgComma_ GLenum target, GLuint index, GLint* data); \
    GLvoid         (GL_APIENTRY *BeginTransformFeedback) (_gcArgComma_ GLenum primitiveMode); \
    GLvoid         (GL_APIENTRY *EndTransformFeedback) (_gcArgOnly_); \
    GLvoid         (GL_APIENTRY *BindBufferRange) (_gcArgComma_ GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size); \
    GLvoid         (GL_APIENTRY *BindBufferBase) (_gcArgComma_ GLenum target, GLuint index, GLuint buffer); \
    GLvoid         (GL_APIENTRY *TransformFeedbackVaryings) (_gcArgComma_ GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode); \
    GLvoid         (GL_APIENTRY *GetTransformFeedbackVarying) (_gcArgComma_ GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name); \
    GLvoid         (GL_APIENTRY *VertexAttribIPointer) (_gcArgComma_ GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer); \
    GLvoid         (GL_APIENTRY *GetVertexAttribIiv) (_gcArgComma_ GLuint index, GLenum pname, GLint* params); \
    GLvoid         (GL_APIENTRY *GetVertexAttribIuiv) (_gcArgComma_ GLuint index, GLenum pname, GLuint* params); \
    GLvoid         (GL_APIENTRY *VertexAttribI4i) (_gcArgComma_ GLuint index, GLint x, GLint y, GLint z, GLint w); \
    GLvoid         (GL_APIENTRY *VertexAttribI4ui) (_gcArgComma_ GLuint index, GLuint x, GLuint y, GLuint z, GLuint w); \
    GLvoid         (GL_APIENTRY *VertexAttribI4iv) (_gcArgComma_ GLuint index, const GLint* v); \
    GLvoid         (GL_APIENTRY *VertexAttribI4uiv) (_gcArgComma_ GLuint index, const GLuint* v); \
    GLvoid         (GL_APIENTRY *GetUniformuiv) (_gcArgComma_ GLuint program, GLint location, GLuint* params); \
    GLint          (GL_APIENTRY *GetFragDataLocation) (_gcArgComma_ GLuint program, const GLchar *name _retLocation_); \
    GLvoid         (GL_APIENTRY *Uniform1ui) (_gcArgComma_ GLint location, GLuint v0); \
    GLvoid         (GL_APIENTRY *Uniform2ui) (_gcArgComma_ GLint location, GLuint v0, GLuint v1); \
    GLvoid         (GL_APIENTRY *Uniform3ui) (_gcArgComma_ GLint location, GLuint v0, GLuint v1, GLuint v2); \
    GLvoid         (GL_APIENTRY *Uniform4ui) (_gcArgComma_ GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3); \
    GLvoid         (GL_APIENTRY *Uniform1uiv) (_gcArgComma_ GLint location, GLsizei count, const GLuint* value); \
    GLvoid         (GL_APIENTRY *Uniform2uiv) (_gcArgComma_ GLint location, GLsizei count, const GLuint* value); \
    GLvoid         (GL_APIENTRY *Uniform3uiv) (_gcArgComma_ GLint location, GLsizei count, const GLuint* value); \
    GLvoid         (GL_APIENTRY *Uniform4uiv) (_gcArgComma_ GLint location, GLsizei count, const GLuint* value); \
    GLvoid         (GL_APIENTRY *ClearBufferiv) (_gcArgComma_ GLenum buffer, GLint drawbuffer, const GLint* value); \
    GLvoid         (GL_APIENTRY *ClearBufferuiv) (_gcArgComma_ GLenum buffer, GLint drawbuffer, const GLuint* value); \
    GLvoid         (GL_APIENTRY *ClearBufferfv) (_gcArgComma_ GLenum buffer, GLint drawbuffer, const GLfloat* value); \
    GLvoid         (GL_APIENTRY *ClearBufferfi) (_gcArgComma_ GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil); \
    const GLubyte* (GL_APIENTRY *GetStringi) (_gcArgComma_ GLenum name, GLuint index); \
    GLvoid         (GL_APIENTRY *CopyBufferSubData) (_gcArgComma_ GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size); \
    GLvoid         (GL_APIENTRY *GetUniformIndices) (_gcArgComma_ GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices); \
    GLvoid         (GL_APIENTRY *GetActiveUniformsiv) (_gcArgComma_ GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params); \
    GLuint         (GL_APIENTRY *GetUniformBlockIndex) (_gcArgComma_ GLuint program, const GLchar* uniformBlockName _retIndex_); \
    GLvoid         (GL_APIENTRY *GetActiveUniformBlockiv) (_gcArgComma_ GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params); \
    GLvoid         (GL_APIENTRY *GetActiveUniformBlockName) (_gcArgComma_ GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName); \
    GLvoid         (GL_APIENTRY *UniformBlockBinding) (_gcArgComma_ GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding); \
    GLvoid         (GL_APIENTRY *DrawArraysInstanced) (_gcArgComma_ GLenum mode, GLint first, GLsizei count, GLsizei instanceCount); \
    GLvoid         (GL_APIENTRY *DrawElementsInstanced) (_gcArgComma_ GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei instanceCount); \
    GLsync         (GL_APIENTRY *FenceSync) (_gcArgComma_ GLenum condition, GLbitfield flags _retSync_); \
    GLboolean      (GL_APIENTRY *IsSync) (_gcArgComma_ GLsync sync); \
    GLvoid         (GL_APIENTRY *DeleteSync) (_gcArgComma_ GLsync sync); \
    GLenum         (GL_APIENTRY *ClientWaitSync) (_gcArgComma_ GLsync sync, GLbitfield flags, GLuint64 timeout); \
    GLvoid         (GL_APIENTRY *WaitSync) (_gcArgComma_ GLsync sync, GLbitfield flags, GLuint64 timeout); \
    GLvoid         (GL_APIENTRY *GetInteger64v) (_gcArgComma_ GLenum pname, GLint64* params); \
    GLvoid         (GL_APIENTRY *GetSynciv) (_gcArgComma_ GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values); \
    GLvoid         (GL_APIENTRY *GetInteger64i_v) (_gcArgComma_ GLenum target, GLuint index, GLint64* data); \
    GLvoid         (GL_APIENTRY *GetBufferParameteri64v) (_gcArgComma_ GLenum target, GLenum pname, GLint64* params); \
    GLvoid         (GL_APIENTRY *GenSamplers) (_gcArgComma_ GLsizei count, GLuint* samplers); \
    GLvoid         (GL_APIENTRY *DeleteSamplers) (_gcArgComma_ GLsizei count, const GLuint* samplers); \
    GLboolean      (GL_APIENTRY *IsSampler) (_gcArgComma_ GLuint sampler); \
    GLvoid         (GL_APIENTRY *BindSampler) (_gcArgComma_ GLuint unit, GLuint sampler); \
    GLvoid         (GL_APIENTRY *SamplerParameteri) (_gcArgComma_ GLuint sampler, GLenum pname, GLint param); \
    GLvoid         (GL_APIENTRY *SamplerParameteriv) (_gcArgComma_ GLuint sampler, GLenum pname, const GLint* param); \
    GLvoid         (GL_APIENTRY *SamplerParameterf) (_gcArgComma_ GLuint sampler, GLenum pname, GLfloat param); \
    GLvoid         (GL_APIENTRY *SamplerParameterfv) (_gcArgComma_ GLuint sampler, GLenum pname, const GLfloat* param); \
    GLvoid         (GL_APIENTRY *GetSamplerParameteriv) (_gcArgComma_ GLuint sampler, GLenum pname, GLint* params); \
    GLvoid         (GL_APIENTRY *GetSamplerParameterfv) (_gcArgComma_ GLuint sampler, GLenum pname, GLfloat* params); \
    GLvoid         (GL_APIENTRY *VertexAttribDivisor) (_gcArgComma_ GLuint index, GLuint divisor); \
    GLvoid         (GL_APIENTRY *BindTransformFeedback) (_gcArgComma_ GLenum target, GLuint id); \
    GLvoid         (GL_APIENTRY *DeleteTransformFeedbacks) (_gcArgComma_ GLsizei n, const GLuint* ids); \
    GLvoid         (GL_APIENTRY *GenTransformFeedbacks) (_gcArgComma_ GLsizei n, GLuint* ids); \
    GLboolean      (GL_APIENTRY *IsTransformFeedback) (_gcArgComma_ GLuint id); \
    GLvoid         (GL_APIENTRY *PauseTransformFeedback) (_gcArgOnly_); \
    GLvoid         (GL_APIENTRY *ResumeTransformFeedback) (_gcArgOnly_); \
    GLvoid         (GL_APIENTRY *GetProgramBinary) (_gcArgComma_ GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary); \
    GLvoid         (GL_APIENTRY *ProgramBinary) (_gcArgComma_ GLuint program, GLenum binaryFormat, const GLvoid* binary, GLsizei length); \
    GLvoid         (GL_APIENTRY *ProgramParameteri) (_gcArgComma_ GLuint program, GLenum pname, GLint value); \
    GLvoid         (GL_APIENTRY *InvalidateFramebuffer) (_gcArgComma_ GLenum target, GLsizei numAttachments, const GLenum* attachments); \
    GLvoid         (GL_APIENTRY *InvalidateSubFramebuffer) (_gcArgComma_ GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height); \
    GLvoid         (GL_APIENTRY *TexStorage2D) (_gcArgComma_ GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height); \
    GLvoid         (GL_APIENTRY *TexStorage3D) (_gcArgComma_ GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth); \
    GLvoid         (GL_APIENTRY *GetInternalformativ) (_gcArgComma_ GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params); \
    /* OpenGL ES 3.1 */ \
    GLvoid         (GL_APIENTRY *DispatchCompute) (_gcArgComma_ GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z); \
    GLvoid         (GL_APIENTRY *DispatchComputeIndirect) (_gcArgComma_ GLintptr indirect); \
    GLvoid         (GL_APIENTRY *DrawArraysIndirect) (_gcArgComma_ GLenum mode, const void *indirect); \
    GLvoid         (GL_APIENTRY *DrawElementsIndirect) (_gcArgComma_ GLenum mode, GLenum type, const void *indirect); \
    GLvoid         (GL_APIENTRY *FramebufferParameteri) (_gcArgComma_ GLenum target, GLenum pname, GLint param); \
    GLvoid         (GL_APIENTRY *GetFramebufferParameteriv) (_gcArgComma_ GLenum target, GLenum pname, GLint *params); \
    GLvoid         (GL_APIENTRY *GetProgramInterfaceiv) (_gcArgComma_ GLuint program, GLenum programInterface, GLenum pname, GLint *params); \
    GLuint         (GL_APIENTRY *GetProgramResourceIndex) (_gcArgComma_ GLuint program, GLenum programInterface, const GLchar *name); \
    GLvoid         (GL_APIENTRY *GetProgramResourceName) (_gcArgComma_ GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name); \
    GLvoid         (GL_APIENTRY *GetProgramResourceiv) (_gcArgComma_ GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params); \
    GLint          (GL_APIENTRY *GetProgramResourceLocation) (_gcArgComma_ GLuint program, GLenum programInterface, const GLchar *name); \
    GLvoid         (GL_APIENTRY *UseProgramStages) (_gcArgComma_ GLuint pipeline, GLbitfield stages, GLuint program); \
    GLvoid         (GL_APIENTRY *ActiveShaderProgram) (_gcArgComma_ GLuint pipeline, GLuint program); \
    GLuint         (GL_APIENTRY *CreateShaderProgramv) (_gcArgComma_ GLenum type, GLsizei count, const GLchar *const*strings); \
    GLvoid         (GL_APIENTRY *BindProgramPipeline) (_gcArgComma_ GLuint pipeline); \
    GLvoid         (GL_APIENTRY *DeleteProgramPipelines) (_gcArgComma_ GLsizei n, const GLuint *pipelines); \
    GLvoid         (GL_APIENTRY *GenProgramPipelines) (_gcArgComma_ GLsizei n, GLuint *pipelines); \
    GLboolean      (GL_APIENTRY *IsProgramPipeline) (_gcArgComma_ GLuint pipeline); \
    GLvoid         (GL_APIENTRY *GetProgramPipelineiv) (_gcArgComma_ GLuint pipeline, GLenum pname, GLint *params); \
    GLvoid         (GL_APIENTRY *ProgramUniform1i) (_gcArgComma_ GLuint program, GLint location, GLint v0); \
    GLvoid         (GL_APIENTRY *ProgramUniform2i) (_gcArgComma_ GLuint program, GLint location, GLint v0, GLint v1); \
    GLvoid         (GL_APIENTRY *ProgramUniform3i) (_gcArgComma_ GLuint program, GLint location, GLint v0, GLint v1, GLint v2); \
    GLvoid         (GL_APIENTRY *ProgramUniform4i) (_gcArgComma_ GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3); \
    GLvoid         (GL_APIENTRY *ProgramUniform1ui) (_gcArgComma_ GLuint program, GLint location, GLuint v0); \
    GLvoid         (GL_APIENTRY *ProgramUniform2ui) (_gcArgComma_ GLuint program, GLint location, GLuint v0, GLuint v1); \
    GLvoid         (GL_APIENTRY *ProgramUniform3ui) (_gcArgComma_ GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2); \
    GLvoid         (GL_APIENTRY *ProgramUniform4ui) (_gcArgComma_ GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3); \
    GLvoid         (GL_APIENTRY *ProgramUniform1f) (_gcArgComma_ GLuint program, GLint location, GLfloat v0); \
    GLvoid         (GL_APIENTRY *ProgramUniform2f) (_gcArgComma_ GLuint program, GLint location, GLfloat v0, GLfloat v1); \
    GLvoid         (GL_APIENTRY *ProgramUniform3f) (_gcArgComma_ GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2); \
    GLvoid         (GL_APIENTRY *ProgramUniform4f) (_gcArgComma_ GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3); \
    GLvoid         (GL_APIENTRY *ProgramUniform1iv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLint *value); \
    GLvoid         (GL_APIENTRY *ProgramUniform2iv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLint *value); \
    GLvoid         (GL_APIENTRY *ProgramUniform3iv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLint *value); \
    GLvoid         (GL_APIENTRY *ProgramUniform4iv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLint *value); \
    GLvoid         (GL_APIENTRY *ProgramUniform1uiv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLuint *value); \
    GLvoid         (GL_APIENTRY *ProgramUniform2uiv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLuint *value); \
    GLvoid         (GL_APIENTRY *ProgramUniform3uiv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLuint *value); \
    GLvoid         (GL_APIENTRY *ProgramUniform4uiv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLuint *value); \
    GLvoid         (GL_APIENTRY *ProgramUniform1fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLfloat *value); \
    GLvoid         (GL_APIENTRY *ProgramUniform2fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLfloat *value); \
    GLvoid         (GL_APIENTRY *ProgramUniform3fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLfloat *value); \
    GLvoid         (GL_APIENTRY *ProgramUniform4fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, const GLfloat *value); \
    GLvoid         (GL_APIENTRY *ProgramUniformMatrix2fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value); \
    GLvoid         (GL_APIENTRY *ProgramUniformMatrix3fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value); \
    GLvoid         (GL_APIENTRY *ProgramUniformMatrix4fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value); \
    GLvoid         (GL_APIENTRY *ProgramUniformMatrix2x3fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value); \
    GLvoid         (GL_APIENTRY *ProgramUniformMatrix3x2fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value); \
    GLvoid         (GL_APIENTRY *ProgramUniformMatrix2x4fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value); \
    GLvoid         (GL_APIENTRY *ProgramUniformMatrix4x2fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value); \
    GLvoid         (GL_APIENTRY *ProgramUniformMatrix3x4fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value); \
    GLvoid         (GL_APIENTRY *ProgramUniformMatrix4x3fv) (_gcArgComma_ GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value); \
    GLvoid         (GL_APIENTRY *ValidateProgramPipeline) (_gcArgComma_ GLuint pipeline); \
    GLvoid         (GL_APIENTRY *GetProgramPipelineInfoLog) (_gcArgComma_ GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog); \
    GLvoid         (GL_APIENTRY *BindImageTexture) (_gcArgComma_ GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format); \
    GLvoid         (GL_APIENTRY *GetBooleani_v) (_gcArgComma_ GLenum target, GLuint index, GLboolean *data); \
    GLvoid         (GL_APIENTRY *MemoryBarrier) (_gcArgComma_ GLbitfield barriers); \
    GLvoid         (GL_APIENTRY *MemoryBarrierByRegion) (_gcArgComma_ GLbitfield barriers); \
    GLvoid         (GL_APIENTRY *TexStorage2DMultisample) (_gcArgComma_ GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations); \
    GLvoid         (GL_APIENTRY *GetMultisamplefv) (_gcArgComma_ GLenum pname, GLuint index, GLfloat *val); \
    GLvoid         (GL_APIENTRY *SampleMaski) (_gcArgComma_ GLuint maskNumber, GLbitfield mask); \
    GLvoid         (GL_APIENTRY *GetTexLevelParameteriv) (_gcArgComma_ GLenum target, GLint level, GLenum pname, GLint *params); \
    GLvoid         (GL_APIENTRY *GetTexLevelParameterfv) (_gcArgComma_ GLenum target, GLint level, GLenum pname, GLfloat *params); \
    GLvoid         (GL_APIENTRY *BindVertexBuffer) (_gcArgComma_ GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride); \
    GLvoid         (GL_APIENTRY *VertexAttribFormat) (_gcArgComma_ GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset); \
    GLvoid         (GL_APIENTRY *VertexAttribIFormat) (_gcArgComma_ GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset); \
    GLvoid         (GL_APIENTRY *VertexAttribBinding) (_gcArgComma_ GLuint attribindex, GLuint bindingindex); \
    GLvoid         (GL_APIENTRY *VertexBindingDivisor) (_gcArgComma_ GLuint bindingindex, GLuint divisor); \
    /* OpenGL ES 3.2 */ \
    GLvoid         (GL_APIENTRY *TexStorage3DMultisample) (_gcArgComma_ GLenum target, GLsizei samples, GLenum sizedinternalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations); \
    GLvoid         (GL_APIENTRY *BlendBarrier) (_gcArgOnly_); \
    GLvoid         (GL_APIENTRY *DebugMessageControl) (_gcArgComma_ GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled); \
    GLvoid         (GL_APIENTRY *DebugMessageInsert) (_gcArgComma_ GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf); \
    GLvoid         (GL_APIENTRY *DebugMessageCallback) (_gcArgComma_ GLDEBUGPROCKHR callback, const GLvoid* userParam); \
    GLuint         (GL_APIENTRY *GetDebugMessageLog) (_gcArgComma_ GLuint count, GLsizei bufSize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog); \
    GLvoid         (GL_APIENTRY *GetPointerv) (_gcArgComma_ GLenum pname, GLvoid** params); \
    GLvoid         (GL_APIENTRY *PushDebugGroup) (_gcArgComma_ GLenum source, GLuint id, GLsizei length, const GLchar * message); \
    GLvoid         (GL_APIENTRY *PopDebugGroup) (_gcArgOnly_); \
    GLvoid         (GL_APIENTRY *ObjectLabel) (_gcArgComma_ GLenum identifier, GLuint name, GLsizei length, const GLchar *label); \
    GLvoid         (GL_APIENTRY *GetObjectLabel) (_gcArgComma_ GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label); \
    GLvoid         (GL_APIENTRY *ObjectPtrLabel) (_gcArgComma_ const GLvoid* ptr, GLsizei length, const GLchar *label); \
    GLvoid         (GL_APIENTRY *GetObjectPtrLabel) (_gcArgComma_ const GLvoid* ptr, GLsizei bufSize, GLsizei *length, GLchar *label); \
    GLenum         (GL_APIENTRY *GetGraphicsResetStatus) (_gcArgOnly_); \
    GLvoid         (GL_APIENTRY *ReadnPixels) (_gcArgComma_ GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLvoid *data); \
    GLvoid         (GL_APIENTRY *GetnUniformfv) (_gcArgComma_ GLuint program, GLint location, GLsizei bufSize, GLfloat *params); \
    GLvoid         (GL_APIENTRY *GetnUniformiv) (_gcArgComma_ GLuint program, GLint location, GLsizei bufSize, GLint *params);   \
    GLvoid         (GL_APIENTRY *GetnUniformuiv) (_gcArgComma_ GLuint program, GLint location, GLsizei bufSize, GLuint *params);   \
    GLvoid         (GL_APIENTRY  *BlendEquationi) (_gcArgComma_ GLuint buf, GLenum mode); \
    GLvoid         (GL_APIENTRY  *BlendEquationSeparatei) (_gcArgComma_ GLuint buf, GLenum modeRGB, GLenum modeAlpha); \
    GLvoid         (GL_APIENTRY  *BlendFunci) (_gcArgComma_ GLuint buf, GLenum sfactor, GLenum dfactor); \
    GLvoid         (GL_APIENTRY  *BlendFuncSeparatei) (_gcArgComma_ GLuint buf, GLenum sfactorRGB,GLenum dfactorRGB,GLenum sfactorAlpha,GLenum dfactorAlpha) ; \
    GLvoid         (GL_APIENTRY  *ColorMaski) (_gcArgComma_ GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a); \
    GLvoid         (GL_APIENTRY  *Enablei) (_gcArgComma_ GLenum target, GLuint index);  \
    GLvoid         (GL_APIENTRY  *Disablei) (_gcArgComma_ GLenum target, GLuint index); \
    GLboolean      (GL_APIENTRY  *IsEnabledi) (_gcArgComma_ GLenum target, GLuint index); \
    GLvoid         (GL_APIENTRY *TexParameterIiv) (_gcArgComma_ GLenum target, GLenum pname, const GLint *params);   \
    GLvoid         (GL_APIENTRY *TexParameterIuiv) (_gcArgComma_ GLenum target, GLenum pname, const GLuint *params); \
    GLvoid         (GL_APIENTRY *GetTexParameterIiv) (_gcArgComma_ GLenum target, GLenum pname, GLint *params);      \
    GLvoid         (GL_APIENTRY *GetTexParameterIuiv) (_gcArgComma_ GLenum target, GLenum pname, GLuint *params);    \
    GLvoid         (GL_APIENTRY *SamplerParameterIiv) (_gcArgComma_ GLuint sampler, GLenum pname, const GLint *param); \
    GLvoid         (GL_APIENTRY *SamplerParameterIuiv) (_gcArgComma_ GLuint sampler, GLenum pname, const GLuint *param); \
    GLvoid         (GL_APIENTRY *GetSamplerParameterIiv) (_gcArgComma_ GLuint sampler, GLenum pname, GLint *params);  \
    GLvoid         (GL_APIENTRY *GetSamplerParameterIuiv) (_gcArgComma_ GLuint sampler, GLenum pname, GLuint *params);  \
    GLvoid         (GL_APIENTRY *TexBuffer) (_gcArgComma_ GLenum target, GLenum internalformat, GLuint buffer); \
    GLvoid         (GL_APIENTRY *TexBufferRange) (_gcArgComma_ GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size); \
    GLvoid         (GL_APIENTRY *PatchParameteri) (_gcArgComma_ GLenum pname, GLint value); \
    GLvoid         (GL_APIENTRY *FramebufferTexture) (_gcArgComma_ GLenum target, GLenum attachment, GLuint texture, GLint level); \
    GLvoid         (GL_APIENTRY *MinSampleShading) (_gcArgComma_ GLfloat value); \
    GLvoid         (GL_APIENTRY *CopyImageSubData) (_gcArgComma_ GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, \
                                                       GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ,  \
                                                       GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth); \
    GLvoid         (GL_APIENTRY *DrawElementsBaseVertex) (_gcArgComma_ GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex); \
    GLvoid         (GL_APIENTRY *DrawRangeElementsBaseVertex) (_gcArgComma_ GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex); \
    GLvoid         (GL_APIENTRY *DrawElementsInstancedBaseVertex) (_gcArgComma_ GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex); \
    GLvoid         (GL_APIENTRY *PrimitiveBoundingBox)(_gcArgComma_ GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW); \
    /* OpenGL ES extensions */ \
    /* GL_OES_EGL_image */ \
    GLvoid         (GL_APIENTRY *EGLImageTargetTexture2DOES) (_gcArgComma_ GLenum target, GLeglImageOES image); \
    GLvoid         (GL_APIENTRY *EGLImageTargetRenderbufferStorageOES) (_gcArgComma_ GLenum target, GLeglImageOES image); \
    /* GL_EXT_multi_draw_arrays */ \
    GLvoid         (GL_APIENTRY *MultiDrawArraysEXT) (_gcArgComma_ GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount); \
    GLvoid         (GL_APIENTRY *MultiDrawElementsEXT) (_gcArgComma_ GLenum mode, const GLsizei *count, GLenum type, const GLvoid*const*indices, GLsizei primcount); \
    /* GL_EXT_multi_draw_arrays && GL_EXT_draw_elements_base_vertex */ \
    GLvoid         (GL_APIENTRY *MultiDrawElementsBaseVertexEXT) (_gcArgComma_ GLenum mode, const GLsizei *count, GLenum type, const void *const *indices, GLsizei drawcount, const GLint * basevertex); \
    /* GL_OES_mapbuffer */ \
    GLvoid         (GL_APIENTRY *GetBufferPointervOES) (_gcArgComma_ GLenum target, GLenum pname, GLvoid** params); \
    GLvoid*        (GL_APIENTRY *MapBufferOES) (_gcArgComma_ GLenum target, GLenum access _retPointer); \
    GLboolean      (GL_APIENTRY *UnmapBufferOES) (_gcArgComma_ GLenum target); \
    /* GL_EXT_discard_framebuffer */ \
    GLvoid         (GL_APIENTRY *DiscardFramebufferEXT) (_gcArgComma_ GLenum target, GLsizei numAttachments, const GLenum *attachments); \
    /* GL_EXT_multisampled_render_to_texture */ \
    GLvoid         (GL_APIENTRY *RenderbufferStorageMultisampleEXT) (_gcArgComma_ GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height); \
    GLvoid         (GL_APIENTRY *FramebufferTexture2DMultisampleEXT) (_gcArgComma_ GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples); \
    /* GL_VIV_direct_texture */ \
    GLvoid         (GL_APIENTRY *TexDirectVIV) (_gcArgComma_ GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** pixels); \
    GLvoid         (GL_APIENTRY *TexDirectInvalidateVIV) (_gcArgComma_ GLenum target); \
    GLvoid         (GL_APIENTRY *TexDirectVIVMap) (_gcArgComma_ GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** logical, const GLuint * physical); \
    GLvoid         (GL_APIENTRY *TexDirectTiledMapVIV) (_gcArgComma_ GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** logical, const GLuint * physical); \
    /* GL_EXT_multi_draw_indirect */ \
    GLvoid         (GL_APIENTRY *MultiDrawArraysIndirectEXT) (_gcArgComma_ GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride); \
    GLvoid         (GL_APIENTRY *MultiDrawElementsIndirectEXT) (_gcArgComma_ GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride); \
    /* cl_khr_gl_sharing */ \
    GLvoid         (GL_APIENTRY *GetTexImage) (_gcArgComma_ GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);

/* Define GLES 3.0 API Dispatch Table */
#define _gcArgComma_  __GLcontext* gc,
#define _gcArgOnly_  __GLcontext* gc
#define _retProgram_
#define _retShader_
#define _retIndex_
#define _retLocation_
#define _retSync_
#define _retPointer

struct __GLesDispatchTableRec
{
    __GLES_API_DISPATCH_FUNCS
};

#undef _gcArgComma_
#undef _gcArgOnly_
#undef _retProgram_
#undef _retShader_
#undef _retIndex_
#undef _retLocation_
#undef _retSync_
#undef _retPointer


/* Define GLES 3.0 API Tracer Dispatch Table */
#define _gcArgComma_
#define _gcArgOnly_
#define _retProgram_     GLuint retval
#define _retShader_     ,GLuint retval
#define _retIndex_      ,GLuint retidx
#define _retLocation_   ,GLint retloc
#define _retSync_       ,GLsync retsync
#define _retPointer     ,GLvoid* retptr

typedef struct __GLtraceDispatchTableRec{

    __GLES_API_DISPATCH_FUNCS

} __GLesTracerDispatchTableStruct;

#undef _gcArgComma_
#undef _gcArgOnly_
#undef _retProgram_
#undef _retShader_
#undef _retIndex_
#undef _retLocation_
#undef _retSync_
#undef _retPointer

extern __GLesDispatchTable __glesApiFuncDispatchTable;


#endif /* __gc_es_dispatch_h__ */
