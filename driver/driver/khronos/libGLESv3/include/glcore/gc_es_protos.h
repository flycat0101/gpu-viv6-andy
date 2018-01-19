/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_es_protos_h__
#define __gc_es_protos_h__

/* OpenGL ES 2.0 */

extern GLvoid GL_APIENTRY __gles_ActiveTexture(__GLcontext *gc, GLenum texture);
extern GLvoid GL_APIENTRY __gles_AttachShader(__GLcontext *gc, GLuint program, GLuint shader);
extern GLvoid GL_APIENTRY __gles_BindAttribLocation(__GLcontext *gc, GLuint program, GLuint index, const GLchar* name);
extern GLvoid GL_APIENTRY __gles_BindBuffer(__GLcontext *gc, GLenum target, GLuint buffer);
extern GLvoid GL_APIENTRY __gles_BindFramebuffer(__GLcontext *gc, GLenum target, GLuint framebuffer);
extern GLvoid GL_APIENTRY __gles_BindRenderbuffer(__GLcontext *gc, GLenum target, GLuint renderbuffer);
extern GLvoid GL_APIENTRY __gles_BindTexture(__GLcontext *gc, GLenum target, GLuint texture);
extern GLvoid GL_APIENTRY __gles_BlendColor(__GLcontext *gc, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern GLvoid GL_APIENTRY __gles_BlendEquation(__GLcontext *gc, GLenum mode);
extern GLvoid GL_APIENTRY __gles_BlendEquationSeparate(__GLcontext *gc, GLenum modeRGB, GLenum modeAlpha);
extern GLvoid GL_APIENTRY __gles_BlendFunc(__GLcontext *gc, GLenum sfactor, GLenum dfactor);
extern GLvoid GL_APIENTRY __gles_BlendFuncSeparate(__GLcontext *gc, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
extern GLvoid GL_APIENTRY __gles_BufferData(__GLcontext *gc, GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
extern GLvoid GL_APIENTRY __gles_BufferSubData(__GLcontext *gc, GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data);
extern GLenum GL_APIENTRY __gles_CheckFramebufferStatus(__GLcontext *gc, GLenum target);
extern GLvoid GL_APIENTRY __gles_Clear(__GLcontext *gc, GLbitfield mask);
extern GLvoid GL_APIENTRY __gles_ClearColor(__GLcontext *gc, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern GLvoid GL_APIENTRY __gles_ClearDepthf(__GLcontext *gc, GLfloat depth);
extern GLvoid GL_APIENTRY __gles_ClearStencil(__GLcontext *gc, GLint s);
extern GLvoid GL_APIENTRY __gles_ColorMask(__GLcontext *gc, GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
extern GLvoid GL_APIENTRY __gles_CompileShader(__GLcontext *gc, GLuint shader);
extern GLvoid GL_APIENTRY __gles_CompressedTexImage2D(__GLcontext *gc, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data);
extern GLvoid GL_APIENTRY __gles_CompressedTexSubImage2D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data);
extern GLvoid GL_APIENTRY __gles_CopyTexImage2D(__GLcontext *gc, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
extern GLvoid GL_APIENTRY __gles_CopyTexSubImage2D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
extern GLuint GL_APIENTRY __gles_CreateProgram(__GLcontext *gc);
extern GLuint GL_APIENTRY __gles_CreateShader(__GLcontext *gc, GLenum type);
extern GLvoid GL_APIENTRY __gles_CullFace(__GLcontext *gc, GLenum mode);
extern GLvoid GL_APIENTRY __gles_DeleteBuffers(__GLcontext *gc, GLsizei n, const GLuint* buffers);
extern GLvoid GL_APIENTRY __gles_DeleteFramebuffers(__GLcontext *gc, GLsizei n, const GLuint* framebuffers);
extern GLvoid GL_APIENTRY __gles_DeleteProgram(__GLcontext *gc, GLuint program);
extern GLvoid GL_APIENTRY __gles_DeleteRenderbuffers(__GLcontext *gc, GLsizei n, const GLuint* renderbuffers);
extern GLvoid GL_APIENTRY __gles_DeleteShader(__GLcontext *gc, GLuint shader);
extern GLvoid GL_APIENTRY __gles_DeleteTextures(__GLcontext *gc, GLsizei n, const GLuint* textures);
extern GLvoid GL_APIENTRY __gles_DepthFunc(__GLcontext *gc, GLenum func);
extern GLvoid GL_APIENTRY __gles_DepthMask(__GLcontext *gc, GLboolean flag);
extern GLvoid GL_APIENTRY __gles_DepthRangef(__GLcontext *gc, GLfloat n, GLfloat f);
extern GLvoid GL_APIENTRY __gles_DetachShader(__GLcontext *gc, GLuint program, GLuint shader);
extern GLvoid GL_APIENTRY __gles_Disable(__GLcontext *gc, GLenum cap);
extern GLvoid GL_APIENTRY __gles_DisableVertexAttribArray(__GLcontext *gc, GLuint index);
extern GLvoid GL_APIENTRY __gles_DrawArrays(__GLcontext *gc, GLenum mode, GLint first, GLsizei count);
extern GLvoid GL_APIENTRY __gles_DrawElements(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid* indices);
extern GLvoid GL_APIENTRY __gles_Enable(__GLcontext *gc, GLenum cap);
extern GLvoid GL_APIENTRY __gles_EnableVertexAttribArray(__GLcontext *gc, GLuint index);
extern GLvoid GL_APIENTRY __gles_Finish(__GLcontext *gc);
extern GLvoid GL_APIENTRY __gles_Flush(__GLcontext *gc);
extern GLvoid GL_APIENTRY __gles_FramebufferRenderbuffer(__GLcontext *gc, GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
extern GLvoid GL_APIENTRY __gles_FramebufferTexture2D(__GLcontext *gc, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
extern GLvoid GL_APIENTRY __gles_FrontFace(__GLcontext *gc, GLenum mode);
extern GLvoid GL_APIENTRY __gles_GenBuffers(__GLcontext *gc, GLsizei n, GLuint* buffers);
extern GLvoid GL_APIENTRY __gles_GenerateMipmap(__GLcontext *gc, GLenum target);
extern GLvoid GL_APIENTRY __gles_GenFramebuffers(__GLcontext *gc, GLsizei n, GLuint* framebuffers);
extern GLvoid GL_APIENTRY __gles_GenRenderbuffers(__GLcontext *gc, GLsizei n, GLuint* renderbuffers);
extern GLvoid GL_APIENTRY __gles_GenTextures(__GLcontext *gc, GLsizei n, GLuint* textures);
extern GLvoid GL_APIENTRY __gles_GetActiveAttrib(__GLcontext *gc, GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
extern GLvoid GL_APIENTRY __gles_GetActiveUniform(__GLcontext *gc, GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
extern GLvoid GL_APIENTRY __gles_GetAttachedShaders(__GLcontext *gc, GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders);
extern GLint  GL_APIENTRY __gles_GetAttribLocation(__GLcontext *gc, GLuint program, const GLchar* name);
extern GLvoid GL_APIENTRY __gles_GetBooleanv(__GLcontext *gc, GLenum pname, GLboolean* params);
extern GLvoid GL_APIENTRY __gles_GetBufferParameteriv(__GLcontext *gc, GLenum target, GLenum pname, GLint* params);
extern GLenum GL_APIENTRY __gles_GetError(__GLcontext *gc);
extern GLvoid GL_APIENTRY __gles_GetFloatv(__GLcontext *gc, GLenum pname, GLfloat* params);
extern GLvoid GL_APIENTRY __gles_GetFramebufferAttachmentParameteriv(__GLcontext *gc, GLenum target, GLenum attachment, GLenum pname, GLint* params);
extern GLvoid GL_APIENTRY __gles_GetIntegerv(__GLcontext *gc, GLenum pname, GLint* params);
extern GLvoid GL_APIENTRY __gles_GetProgramiv(__GLcontext *gc, GLuint program, GLenum pname, GLint* params);
extern GLvoid GL_APIENTRY __gles_GetProgramInfoLog(__GLcontext *gc, GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog);
extern GLvoid GL_APIENTRY __gles_GetRenderbufferParameteriv(__GLcontext *gc, GLenum target, GLenum pname, GLint* params);
extern GLvoid GL_APIENTRY __gles_GetShaderiv(__GLcontext *gc, GLuint shader, GLenum pname, GLint* params);
extern GLvoid GL_APIENTRY __gles_GetShaderInfoLog(__GLcontext *gc, GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog);
extern GLvoid GL_APIENTRY __gles_GetShaderPrecisionFormat(__GLcontext *gc, GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision);
extern GLvoid GL_APIENTRY __gles_GetShaderSource(__GLcontext *gc, GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source);
extern const GLubyte* GL_APIENTRY __gles_GetString(__GLcontext *gc, GLenum name);
extern GLvoid GL_APIENTRY __gles_GetTexParameterfv(__GLcontext *gc, GLenum target, GLenum pname, GLfloat* params);
extern GLvoid GL_APIENTRY __gles_GetTexParameteriv(__GLcontext *gc, GLenum target, GLenum pname, GLint* params);
extern GLvoid GL_APIENTRY __gles_GetUniformfv(__GLcontext *gc, GLuint program, GLint location, GLfloat* params);
extern GLvoid GL_APIENTRY __gles_GetUniformiv(__GLcontext *gc, GLuint program, GLint location, GLint* params);
extern GLint  GL_APIENTRY __gles_GetUniformLocation(__GLcontext *gc, GLuint program, const GLchar* name);
extern GLvoid GL_APIENTRY __gles_GetVertexAttribfv(__GLcontext *gc, GLuint index, GLenum pname, GLfloat* params);
extern GLvoid GL_APIENTRY __gles_GetVertexAttribiv(__GLcontext *gc, GLuint index, GLenum pname, GLint* params);
extern GLvoid GL_APIENTRY __gles_GetVertexAttribPointerv(__GLcontext *gc, GLuint index, GLenum pname, GLvoid** pointer);
extern GLvoid GL_APIENTRY __gles_Hint(__GLcontext *gc, GLenum target, GLenum mode);
extern GLboolean GL_APIENTRY __gles_IsBuffer(__GLcontext *gc, GLuint buffer);
extern GLboolean GL_APIENTRY __gles_IsEnabled(__GLcontext *gc, GLenum cap);
extern GLboolean GL_APIENTRY __gles_IsFramebuffer(__GLcontext *gc, GLuint framebuffer);
extern GLboolean GL_APIENTRY __gles_IsProgram(__GLcontext *gc, GLuint program);
extern GLboolean GL_APIENTRY __gles_IsRenderbuffer(__GLcontext *gc, GLuint renderbuffer);
extern GLboolean GL_APIENTRY __gles_IsShader(__GLcontext *gc, GLuint shader);
extern GLboolean GL_APIENTRY __gles_IsTexture(__GLcontext *gc, GLuint texture);
extern GLvoid GL_APIENTRY __gles_LineWidth(__GLcontext *gc, GLfloat width);
extern GLvoid GL_APIENTRY __gles_LinkProgram(__GLcontext *gc, GLuint program);
extern GLvoid GL_APIENTRY __gles_PixelStorei(__GLcontext *gc, GLenum pname, GLint param);
extern GLvoid GL_APIENTRY __gles_PolygonOffset(__GLcontext *gc, GLfloat factor, GLfloat units);
extern GLvoid GL_APIENTRY __gles_ReadPixels(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels);
extern GLvoid GL_APIENTRY __gles_ReleaseShaderCompiler(__GLcontext *gc);
extern GLvoid GL_APIENTRY __gles_RenderbufferStorage(__GLcontext *gc, GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
extern GLvoid GL_APIENTRY __gles_SampleCoverage(__GLcontext *gc, GLfloat value, GLboolean invert);
extern GLvoid GL_APIENTRY __gles_Scissor(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height);
extern GLvoid GL_APIENTRY __gles_ShaderBinary(__GLcontext *gc, GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length);
extern GLvoid GL_APIENTRY __gles_ShaderSource(__GLcontext *gc, GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
extern GLvoid GL_APIENTRY __gles_StencilFunc(__GLcontext *gc, GLenum func, GLint ref, GLuint mask);
extern GLvoid GL_APIENTRY __gles_StencilFuncSeparate(__GLcontext *gc, GLenum face, GLenum func, GLint ref, GLuint mask);
extern GLvoid GL_APIENTRY __gles_StencilMask(__GLcontext *gc, GLuint mask);
extern GLvoid GL_APIENTRY __gles_StencilMaskSeparate(__GLcontext *gc, GLenum face, GLuint mask);
extern GLvoid GL_APIENTRY __gles_StencilOp(__GLcontext *gc, GLenum fail, GLenum zfail, GLenum zpass);
extern GLvoid GL_APIENTRY __gles_StencilOpSeparate(__GLcontext *gc, GLenum face, GLenum fail, GLenum zfail, GLenum zpass);
extern GLvoid GL_APIENTRY __gles_TexImage2D(__GLcontext *gc, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
extern GLvoid GL_APIENTRY __gles_TexParameterf(__GLcontext *gc, GLenum target, GLenum pname, GLfloat param);
extern GLvoid GL_APIENTRY __gles_TexParameterfv(__GLcontext *gc, GLenum target, GLenum pname, const GLfloat* params);
extern GLvoid GL_APIENTRY __gles_TexParameteri(__GLcontext *gc, GLenum target, GLenum pname, GLint param);
extern GLvoid GL_APIENTRY __gles_TexParameteriv(__GLcontext *gc, GLenum target, GLenum pname, const GLint* params);
extern GLvoid GL_APIENTRY __gles_TexSubImage2D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels);
extern GLvoid GL_APIENTRY __gles_Uniform1f(__GLcontext *gc, GLint location, GLfloat x);
extern GLvoid GL_APIENTRY __gles_Uniform1fv(__GLcontext *gc, GLint location, GLsizei count, const GLfloat* v);
extern GLvoid GL_APIENTRY __gles_Uniform1i(__GLcontext *gc, GLint location, GLint x);
extern GLvoid GL_APIENTRY __gles_Uniform1iv(__GLcontext *gc, GLint location, GLsizei count, const GLint* v);
extern GLvoid GL_APIENTRY __gles_Uniform2f(__GLcontext *gc, GLint location, GLfloat x, GLfloat y);
extern GLvoid GL_APIENTRY __gles_Uniform2fv(__GLcontext *gc, GLint location, GLsizei count, const GLfloat* v);
extern GLvoid GL_APIENTRY __gles_Uniform2i(__GLcontext *gc, GLint location, GLint x, GLint y);
extern GLvoid GL_APIENTRY __gles_Uniform2iv(__GLcontext *gc, GLint location, GLsizei count, const GLint* v);
extern GLvoid GL_APIENTRY __gles_Uniform3f(__GLcontext *gc, GLint location, GLfloat x, GLfloat y, GLfloat z);
extern GLvoid GL_APIENTRY __gles_Uniform3fv(__GLcontext *gc, GLint location, GLsizei count, const GLfloat* v);
extern GLvoid GL_APIENTRY __gles_Uniform3i(__GLcontext *gc, GLint location, GLint x, GLint y, GLint z);
extern GLvoid GL_APIENTRY __gles_Uniform3iv(__GLcontext *gc, GLint location, GLsizei count, const GLint* v);
extern GLvoid GL_APIENTRY __gles_Uniform4f(__GLcontext *gc, GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern GLvoid GL_APIENTRY __gles_Uniform4fv(__GLcontext *gc, GLint location, GLsizei count, const GLfloat* v);
extern GLvoid GL_APIENTRY __gles_Uniform4i(__GLcontext *gc, GLint location, GLint x, GLint y, GLint z, GLint w);
extern GLvoid GL_APIENTRY __gles_Uniform4iv(__GLcontext *gc, GLint location, GLsizei count, const GLint* v);
extern GLvoid GL_APIENTRY __gles_UniformMatrix2fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
extern GLvoid GL_APIENTRY __gles_UniformMatrix3fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
extern GLvoid GL_APIENTRY __gles_UniformMatrix4fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
extern GLvoid GL_APIENTRY __gles_UseProgram(__GLcontext *gc, GLuint program);
extern GLvoid GL_APIENTRY __gles_ValidateProgram(__GLcontext *gc, GLuint program);
extern GLvoid GL_APIENTRY __gles_VertexAttrib1f(__GLcontext *gc, GLuint indx, GLfloat x);
extern GLvoid GL_APIENTRY __gles_VertexAttrib1fv(__GLcontext *gc, GLuint indx, const GLfloat* values);
extern GLvoid GL_APIENTRY __gles_VertexAttrib2f(__GLcontext *gc, GLuint indx, GLfloat x, GLfloat y);
extern GLvoid GL_APIENTRY __gles_VertexAttrib2fv(__GLcontext *gc, GLuint indx, const GLfloat* values);
extern GLvoid GL_APIENTRY __gles_VertexAttrib3f(__GLcontext *gc, GLuint indx, GLfloat x, GLfloat y, GLfloat z);
extern GLvoid GL_APIENTRY __gles_VertexAttrib3fv(__GLcontext *gc, GLuint indx, const GLfloat* values);
extern GLvoid GL_APIENTRY __gles_VertexAttrib4f(__GLcontext *gc, GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern GLvoid GL_APIENTRY __gles_VertexAttrib4fv(__GLcontext *gc, GLuint indx, const GLfloat* values);
extern GLvoid GL_APIENTRY __gles_VertexAttribPointer(__GLcontext *gc, GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr);
extern GLvoid GL_APIENTRY __gles_Viewport(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height);

/* OpenGL ES 3.0 */

extern GLvoid GL_APIENTRY __gles_ReadBuffer(__GLcontext *gc, GLenum mode);
extern GLvoid GL_APIENTRY __gles_DrawRangeElements(__GLcontext *gc, GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices);
extern GLvoid GL_APIENTRY __gles_TexImage3D(__GLcontext *gc, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
extern GLvoid GL_APIENTRY __gles_TexSubImage3D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels);
extern GLvoid GL_APIENTRY __gles_CopyTexSubImage3D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
extern GLvoid GL_APIENTRY __gles_CompressedTexImage3D(__GLcontext *gc, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data);
extern GLvoid GL_APIENTRY __gles_CompressedTexSubImage3D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data);
extern GLvoid GL_APIENTRY __gles_GenQueries(__GLcontext *gc, GLsizei n, GLuint* ids);
extern GLvoid GL_APIENTRY __gles_DeleteQueries(__GLcontext *gc, GLsizei n, const GLuint* ids);
extern GLboolean GL_APIENTRY __gles_IsQuery(__GLcontext *gc, GLuint id);
extern GLvoid GL_APIENTRY __gles_BeginQuery(__GLcontext *gc, GLenum target, GLuint id);
extern GLvoid GL_APIENTRY __gles_EndQuery(__GLcontext *gc, GLenum target);
extern GLvoid GL_APIENTRY __gles_GetQueryiv(__GLcontext *gc, GLenum target, GLenum pname, GLint* params);
extern GLvoid GL_APIENTRY __gles_GetQueryObjectuiv(__GLcontext *gc, GLuint id, GLenum pname, GLuint* params);
extern GLboolean GL_APIENTRY __gles_UnmapBuffer(__GLcontext *gc, GLenum target);
extern GLvoid GL_APIENTRY __gles_GetBufferPointerv(__GLcontext *gc, GLenum target, GLenum pname, GLvoid** params);
extern GLvoid GL_APIENTRY __gles_DrawBuffers(__GLcontext *gc, GLsizei n, const GLenum* bufs);
extern GLvoid GL_APIENTRY __gles_UniformMatrix2x3fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
extern GLvoid GL_APIENTRY __gles_UniformMatrix3x2fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
extern GLvoid GL_APIENTRY __gles_UniformMatrix2x4fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
extern GLvoid GL_APIENTRY __gles_UniformMatrix4x2fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
extern GLvoid GL_APIENTRY __gles_UniformMatrix3x4fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
extern GLvoid GL_APIENTRY __gles_UniformMatrix4x3fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
extern GLvoid GL_APIENTRY __gles_BlitFramebuffer(__GLcontext *gc, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
extern GLvoid GL_APIENTRY __gles_RenderbufferStorageMultisample(__GLcontext *gc, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
extern GLvoid GL_APIENTRY __gles_FramebufferTextureLayer(__GLcontext *gc, GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
extern GLvoid* GL_APIENTRY __gles_MapBufferRange(__GLcontext *gc, GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
extern GLvoid GL_APIENTRY __gles_FlushMappedBufferRange(__GLcontext *gc, GLenum target, GLintptr offset, GLsizeiptr length);
extern GLvoid GL_APIENTRY __gles_BindVertexArray(__GLcontext *gc, GLuint array);
extern GLvoid GL_APIENTRY __gles_DeleteVertexArrays(__GLcontext *gc, GLsizei n, const GLuint* arrays);
extern GLvoid GL_APIENTRY __gles_GenVertexArrays(__GLcontext *gc, GLsizei n, GLuint* arrays);
extern GLboolean GL_APIENTRY __gles_IsVertexArray(__GLcontext *gc, GLuint array);
extern GLvoid GL_APIENTRY __gles_GetIntegeri_v(__GLcontext *gc, GLenum target, GLuint index, GLint* data);
extern GLvoid GL_APIENTRY __gles_BeginTransformFeedback(__GLcontext *gc, GLenum primitiveMode);
extern GLvoid GL_APIENTRY __gles_EndTransformFeedback(__GLcontext *gc);
extern GLvoid GL_APIENTRY __gles_BindBufferRange(__GLcontext *gc, GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
extern GLvoid GL_APIENTRY __gles_BindBufferBase(__GLcontext *gc, GLenum target, GLuint index, GLuint buffer);
extern GLvoid GL_APIENTRY __gles_TransformFeedbackVaryings(__GLcontext *gc, GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode);
extern GLvoid GL_APIENTRY __gles_GetTransformFeedbackVarying(__GLcontext *gc, GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name);
extern GLvoid GL_APIENTRY __gles_VertexAttribIPointer(__GLcontext *gc, GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
extern GLvoid GL_APIENTRY __gles_GetVertexAttribIiv(__GLcontext *gc, GLuint index, GLenum pname, GLint* params);
extern GLvoid GL_APIENTRY __gles_GetVertexAttribIuiv(__GLcontext *gc, GLuint index, GLenum pname, GLuint* params);
extern GLvoid GL_APIENTRY __gles_VertexAttribI4i(__GLcontext *gc, GLuint index, GLint x, GLint y, GLint z, GLint w);
extern GLvoid GL_APIENTRY __gles_VertexAttribI4ui(__GLcontext *gc, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
extern GLvoid GL_APIENTRY __gles_VertexAttribI4iv(__GLcontext *gc, GLuint index, const GLint* v);
extern GLvoid GL_APIENTRY __gles_VertexAttribI4uiv(__GLcontext *gc, GLuint index, const GLuint* v);
extern GLvoid GL_APIENTRY __gles_GetUniformuiv(__GLcontext *gc, GLuint program, GLint location, GLuint* params);
extern GLint  GL_APIENTRY __gles_GetFragDataLocation(__GLcontext *gc, GLuint program, const GLchar *name);
extern GLvoid GL_APIENTRY __gles_Uniform1ui(__GLcontext *gc, GLint location, GLuint v0);
extern GLvoid GL_APIENTRY __gles_Uniform2ui(__GLcontext *gc, GLint location, GLuint v0, GLuint v1);
extern GLvoid GL_APIENTRY __gles_Uniform3ui(__GLcontext *gc, GLint location, GLuint v0, GLuint v1, GLuint v2);
extern GLvoid GL_APIENTRY __gles_Uniform4ui(__GLcontext *gc, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
extern GLvoid GL_APIENTRY __gles_Uniform1uiv(__GLcontext *gc, GLint location, GLsizei count, const GLuint* value);
extern GLvoid GL_APIENTRY __gles_Uniform2uiv(__GLcontext *gc, GLint location, GLsizei count, const GLuint* value);
extern GLvoid GL_APIENTRY __gles_Uniform3uiv(__GLcontext *gc, GLint location, GLsizei count, const GLuint* value);
extern GLvoid GL_APIENTRY __gles_Uniform4uiv(__GLcontext *gc, GLint location, GLsizei count, const GLuint* value);
extern GLvoid GL_APIENTRY __gles_ClearBufferiv(__GLcontext *gc, GLenum buffer, GLint drawbuffer, const GLint* value);
extern GLvoid GL_APIENTRY __gles_ClearBufferuiv(__GLcontext *gc, GLenum buffer, GLint drawbuffer, const GLuint* value);
extern GLvoid GL_APIENTRY __gles_ClearBufferfv(__GLcontext *gc, GLenum buffer, GLint drawbuffer, const GLfloat* value);
extern GLvoid GL_APIENTRY __gles_ClearBufferfi(__GLcontext *gc, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
extern const GLubyte* GL_APIENTRY __gles_GetStringi(__GLcontext *gc, GLenum name, GLuint index);
extern GLvoid GL_APIENTRY __gles_CopyBufferSubData(__GLcontext *gc, GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
extern GLvoid GL_APIENTRY __gles_GetUniformIndices(__GLcontext *gc, GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices);
extern GLvoid GL_APIENTRY __gles_GetActiveUniformsiv(__GLcontext *gc, GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params);
extern GLuint GL_APIENTRY __gles_GetUniformBlockIndex(__GLcontext *gc, GLuint program, const GLchar* uniformBlockName);
extern GLvoid GL_APIENTRY __gles_GetActiveUniformBlockiv(__GLcontext *gc, GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params);
extern GLvoid GL_APIENTRY __gles_GetActiveUniformBlockName(__GLcontext *gc, GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName);
extern GLvoid GL_APIENTRY __gles_UniformBlockBinding(__GLcontext *gc, GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
extern GLvoid GL_APIENTRY __gles_DrawArraysInstanced(__GLcontext *gc, GLenum mode, GLint first, GLsizei count, GLsizei instanceCount);
extern GLvoid GL_APIENTRY __gles_DrawElementsInstanced(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei instanceCount);
extern GLsync GL_APIENTRY __gles_FenceSync(__GLcontext *gc, GLenum condition, GLbitfield flags);
extern GLboolean GL_APIENTRY __gles_IsSync(__GLcontext *gc, GLsync sync);
extern GLvoid GL_APIENTRY __gles_DeleteSync(__GLcontext *gc, GLsync sync);
extern GLenum GL_APIENTRY __gles_ClientWaitSync(__GLcontext *gc, GLsync sync, GLbitfield flags, GLuint64 timeout);
extern GLvoid GL_APIENTRY __gles_WaitSync(__GLcontext *gc, GLsync sync, GLbitfield flags, GLuint64 timeout);
extern GLvoid GL_APIENTRY __gles_GetInteger64v(__GLcontext *gc, GLenum pname, GLint64* params);
extern GLvoid GL_APIENTRY __gles_GetSynciv(__GLcontext *gc, GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values);
extern GLvoid GL_APIENTRY __gles_GetInteger64i_v(__GLcontext *gc, GLenum target, GLuint index, GLint64* data);
extern GLvoid GL_APIENTRY __gles_GetBufferParameteri64v(__GLcontext *gc, GLenum target, GLenum pname, GLint64* params);
extern GLvoid GL_APIENTRY __gles_GenSamplers(__GLcontext *gc, GLsizei count, GLuint* samplers);
extern GLvoid GL_APIENTRY __gles_DeleteSamplers(__GLcontext *gc, GLsizei count, const GLuint* samplers);
extern GLboolean GL_APIENTRY __gles_IsSampler(__GLcontext *gc, GLuint sampler);
extern GLvoid GL_APIENTRY __gles_BindSampler(__GLcontext *gc, GLuint unit, GLuint sampler);
extern GLvoid GL_APIENTRY __gles_SamplerParameteri(__GLcontext *gc, GLuint sampler, GLenum pname, GLint param);
extern GLvoid GL_APIENTRY __gles_SamplerParameteriv(__GLcontext *gc, GLuint sampler, GLenum pname, const GLint* param);
extern GLvoid GL_APIENTRY __gles_SamplerParameterf(__GLcontext *gc, GLuint sampler, GLenum pname, GLfloat param);
extern GLvoid GL_APIENTRY __gles_SamplerParameterfv(__GLcontext *gc, GLuint sampler, GLenum pname, const GLfloat* param);
extern GLvoid GL_APIENTRY __gles_GetSamplerParameteriv(__GLcontext *gc, GLuint sampler, GLenum pname, GLint* params);
extern GLvoid GL_APIENTRY __gles_GetSamplerParameterfv(__GLcontext *gc, GLuint sampler, GLenum pname, GLfloat* params);
extern GLvoid GL_APIENTRY __gles_VertexAttribDivisor(__GLcontext *gc, GLuint index, GLuint divisor);
extern GLvoid GL_APIENTRY __gles_BindTransformFeedback(__GLcontext *gc, GLenum target, GLuint id);
extern GLvoid GL_APIENTRY __gles_DeleteTransformFeedbacks(__GLcontext *gc, GLsizei n, const GLuint* ids);
extern GLvoid GL_APIENTRY __gles_GenTransformFeedbacks(__GLcontext *gc, GLsizei n, GLuint* ids);
extern GLboolean GL_APIENTRY __gles_IsTransformFeedback(__GLcontext *gc, GLuint id);
extern GLvoid GL_APIENTRY __gles_PauseTransformFeedback(__GLcontext *gc);
extern GLvoid GL_APIENTRY __gles_ResumeTransformFeedback(__GLcontext *gc);
extern GLvoid GL_APIENTRY __gles_GetProgramBinary(__GLcontext *gc, GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary);
extern GLvoid GL_APIENTRY __gles_ProgramBinary(__GLcontext *gc, GLuint program, GLenum binaryFormat, const GLvoid* binary, GLsizei length);
extern GLvoid GL_APIENTRY __gles_ProgramParameteri(__GLcontext *gc, GLuint program, GLenum pname, GLint value);
extern GLvoid GL_APIENTRY __gles_InvalidateFramebuffer(__GLcontext *gc, GLenum target, GLsizei numAttachments, const GLenum* attachments);
extern GLvoid GL_APIENTRY __gles_InvalidateSubFramebuffer(__GLcontext *gc, GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height);
extern GLvoid GL_APIENTRY __gles_TexStorage2D(__GLcontext *gc, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
extern GLvoid GL_APIENTRY __gles_TexStorage3D(__GLcontext *gc, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
extern GLvoid GL_APIENTRY __gles_GetInternalformativ(__GLcontext *gc, GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params);

/* OpenGL ES 3.1 */

extern GLvoid GL_APIENTRY __gles_DispatchCompute(__GLcontext *gc, GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
extern GLvoid GL_APIENTRY __gles_DispatchComputeIndirect(__GLcontext *gc, GLintptr indirect);
extern GLvoid GL_APIENTRY __gles_DrawArraysIndirect(__GLcontext *gc, GLenum mode, const void *indirect);
extern GLvoid GL_APIENTRY __gles_DrawElementsIndirect(__GLcontext *gc, GLenum mode, GLenum type, const void *indirect);
extern GLvoid GL_APIENTRY __gles_FramebufferParameteri(__GLcontext *gc, GLenum target, GLenum pname, GLint param);
extern GLvoid GL_APIENTRY __gles_GetFramebufferParameteriv(__GLcontext *gc, GLenum target, GLenum pname, GLint *params);
extern GLvoid GL_APIENTRY __gles_GetProgramInterfaceiv(__GLcontext *gc, GLuint program, GLenum programInterface, GLenum pname, GLint *params);
extern GLuint GL_APIENTRY __gles_GetProgramResourceIndex(__GLcontext *gc, GLuint program, GLenum programInterface, const GLchar *name);
extern GLvoid GL_APIENTRY __gles_GetProgramResourceName(__GLcontext *gc, GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name);
extern GLvoid GL_APIENTRY __gles_GetProgramResourceiv(__GLcontext *gc, GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params);
extern GLint GL_APIENTRY __gles_GetProgramResourceLocation(__GLcontext *gc, GLuint program, GLenum programInterface, const GLchar *name);
extern GLvoid GL_APIENTRY __gles_UseProgramStages(__GLcontext *gc, GLuint pipeline, GLbitfield stages, GLuint program);
extern GLvoid GL_APIENTRY __gles_ActiveShaderProgram(__GLcontext *gc, GLuint pipeline, GLuint program);
extern GLuint GL_APIENTRY __gles_CreateShaderProgramv(__GLcontext *gc, GLenum type, GLsizei count, const GLchar *const*strings);
extern GLvoid GL_APIENTRY __gles_BindProgramPipeline(__GLcontext *gc, GLuint pipeline);
extern GLvoid GL_APIENTRY __gles_DeleteProgramPipelines(__GLcontext *gc, GLsizei n, const GLuint *pipelines);
extern GLvoid GL_APIENTRY __gles_GenProgramPipelines(__GLcontext *gc, GLsizei n, GLuint *pipelines);
extern GLboolean GL_APIENTRY __gles_IsProgramPipeline(__GLcontext *gc, GLuint pipeline);
extern GLvoid GL_APIENTRY __gles_GetProgramPipelineiv(__GLcontext *gc, GLuint pipeline, GLenum pname, GLint *params);
extern GLvoid GL_APIENTRY __gles_ProgramUniform1i(__GLcontext *gc, GLuint program, GLint location, GLint v0);
extern GLvoid GL_APIENTRY __gles_ProgramUniform2i(__GLcontext *gc, GLuint program, GLint location, GLint v0, GLint v1);
extern GLvoid GL_APIENTRY __gles_ProgramUniform3i(__GLcontext *gc, GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
extern GLvoid GL_APIENTRY __gles_ProgramUniform4i(__GLcontext *gc, GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
extern GLvoid GL_APIENTRY __gles_ProgramUniform1ui(__GLcontext *gc, GLuint program, GLint location, GLuint v0);
extern GLvoid GL_APIENTRY __gles_ProgramUniform2ui(__GLcontext *gc, GLuint program, GLint location, GLuint v0, GLuint v1);
extern GLvoid GL_APIENTRY __gles_ProgramUniform3ui(__GLcontext *gc, GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
extern GLvoid GL_APIENTRY __gles_ProgramUniform4ui(__GLcontext *gc, GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
extern GLvoid GL_APIENTRY __gles_ProgramUniform1f(__GLcontext *gc, GLuint program, GLint location, GLfloat v0);
extern GLvoid GL_APIENTRY __gles_ProgramUniform2f(__GLcontext *gc, GLuint program, GLint location, GLfloat v0, GLfloat v1);
extern GLvoid GL_APIENTRY __gles_ProgramUniform3f(__GLcontext *gc, GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
extern GLvoid GL_APIENTRY __gles_ProgramUniform4f(__GLcontext *gc, GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
extern GLvoid GL_APIENTRY __gles_ProgramUniform1iv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLint *value);
extern GLvoid GL_APIENTRY __gles_ProgramUniform2iv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLint *value);
extern GLvoid GL_APIENTRY __gles_ProgramUniform3iv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLint *value);
extern GLvoid GL_APIENTRY __gles_ProgramUniform4iv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLint *value);
extern GLvoid GL_APIENTRY __gles_ProgramUniform1uiv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLuint *value);
extern GLvoid GL_APIENTRY __gles_ProgramUniform2uiv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLuint *value);
extern GLvoid GL_APIENTRY __gles_ProgramUniform3uiv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLuint *value);
extern GLvoid GL_APIENTRY __gles_ProgramUniform4uiv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLuint *value);
extern GLvoid GL_APIENTRY __gles_ProgramUniform1fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLfloat *value);
extern GLvoid GL_APIENTRY __gles_ProgramUniform2fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLfloat *value);
extern GLvoid GL_APIENTRY __gles_ProgramUniform3fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLfloat *value);
extern GLvoid GL_APIENTRY __gles_ProgramUniform4fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLfloat *value);
extern GLvoid GL_APIENTRY __gles_ProgramUniformMatrix2fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern GLvoid GL_APIENTRY __gles_ProgramUniformMatrix3fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern GLvoid GL_APIENTRY __gles_ProgramUniformMatrix4fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern GLvoid GL_APIENTRY __gles_ProgramUniformMatrix2x3fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern GLvoid GL_APIENTRY __gles_ProgramUniformMatrix3x2fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern GLvoid GL_APIENTRY __gles_ProgramUniformMatrix2x4fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern GLvoid GL_APIENTRY __gles_ProgramUniformMatrix4x2fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern GLvoid GL_APIENTRY __gles_ProgramUniformMatrix3x4fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern GLvoid GL_APIENTRY __gles_ProgramUniformMatrix4x3fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern GLvoid GL_APIENTRY __gles_ValidateProgramPipeline(__GLcontext *gc, GLuint pipeline);
extern GLvoid GL_APIENTRY __gles_GetProgramPipelineInfoLog(__GLcontext *gc, GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
extern GLvoid GL_APIENTRY __gles_BindImageTexture(__GLcontext *gc, GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
extern GLvoid GL_APIENTRY __gles_GetBooleani_v(__GLcontext *gc, GLenum target, GLuint index, GLboolean *data);
extern GLvoid GL_APIENTRY __gles_MemoryBarrier(__GLcontext *gc, GLbitfield barriers);
extern GLvoid GL_APIENTRY __gles_MemoryBarrierByRegion(__GLcontext *gc, GLbitfield barriers);
extern GLvoid GL_APIENTRY __gles_TexStorage2DMultisample(__GLcontext *gc, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
extern GLvoid GL_APIENTRY __gles_GetMultisamplefv(__GLcontext *gc, GLenum pname, GLuint index, GLfloat *val);
extern GLvoid GL_APIENTRY __gles_SampleMaski(__GLcontext *gc, GLuint maskNumber, GLbitfield mask);
extern GLvoid GL_APIENTRY __gles_GetTexLevelParameteriv(__GLcontext *gc, GLenum target, GLint level, GLenum pname, GLint *params);
extern GLvoid GL_APIENTRY __gles_GetTexLevelParameterfv(__GLcontext *gc, GLenum target, GLint level, GLenum pname, GLfloat *params);
extern GLvoid GL_APIENTRY __gles_BindVertexBuffer(__GLcontext *gc, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
extern GLvoid GL_APIENTRY __gles_VertexAttribFormat(__GLcontext *gc, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
extern GLvoid GL_APIENTRY __gles_VertexAttribIFormat(__GLcontext *gc, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
extern GLvoid GL_APIENTRY __gles_VertexAttribBinding(__GLcontext *gc, GLuint attribindex, GLuint bindingindex);
extern GLvoid GL_APIENTRY __gles_VertexBindingDivisor(__GLcontext *gc, GLuint bindingindex, GLuint divisor);

/* OpenGL ES 3.2 */
extern GLvoid GL_APIENTRY __gles_TexStorage3DMultisample(__GLcontext *gc, GLenum target, GLsizei samples, GLenum sizedinternalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);

extern GLvoid GL_APIENTRY __gles_BlendBarrier(__GLcontext *gc);

extern GLvoid GL_APIENTRY __gles_DebugMessageControl(__GLcontext *gc, GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled);
extern GLvoid GL_APIENTRY __gles_DebugMessageInsert(__GLcontext *gc, GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf);
extern GLvoid GL_APIENTRY __gles_DebugMessageCallback(__GLcontext *gc, GLDEBUGPROCKHR callback, const GLvoid* userParam);
extern GLuint GL_APIENTRY __gles_GetDebugMessageLog(__GLcontext *gc, GLuint count, GLsizei bufSize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog);
extern GLvoid GL_APIENTRY __gles_GetPointerv(__GLcontext *gc, GLenum pname, GLvoid** params);
extern GLvoid GL_APIENTRY __gles_PushDebugGroup(__GLcontext *gc, GLenum source, GLuint id, GLsizei length, const GLchar * message);
extern GLvoid GL_APIENTRY __gles_PopDebugGroup(__GLcontext *gc);
extern GLvoid GL_APIENTRY __gles_ObjectLabel(__GLcontext *gc, GLenum identifier, GLuint name, GLsizei length, const GLchar *label);
extern GLvoid GL_APIENTRY __gles_GetObjectLabel(__GLcontext *gc, GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label);
extern GLvoid GL_APIENTRY __gles_ObjectPtrLabel(__GLcontext *gc, const GLvoid* ptr, GLsizei length, const GLchar *label);
extern GLvoid GL_APIENTRY __gles_GetObjectPtrLabel(__GLcontext *gc, const GLvoid* ptr, GLsizei bufSize, GLsizei *length, GLchar *label);


extern GLenum GL_APIENTRY __gles_GetGraphicsResetStatus(__GLcontext *gc);
extern GLvoid GL_APIENTRY __gles_ReadnPixels(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLvoid *data);
extern GLvoid GL_APIENTRY __gles_GetnUniformfv(__GLcontext *gc, GLuint program, GLint location, GLsizei bufSize, GLfloat *params);
extern GLvoid GL_APIENTRY __gles_GetnUniformiv(__GLcontext *gc, GLuint program, GLint location, GLsizei bufSize, GLint *params);
extern GLvoid GL_APIENTRY __gles_GetnUniformuiv(__GLcontext *gc, GLuint program, GLint location, GLsizei bufSize, GLuint *params);

extern GLvoid GL_APIENTRY __gles_BlendEquationi(__GLcontext * gc, GLuint buf, GLenum mode);
extern GLvoid GL_APIENTRY __gles_BlendEquationSeparatei(__GLcontext * gc, GLuint buf, GLenum modeRGB, GLenum modeAlpha);
extern GLvoid GL_APIENTRY __gles_BlendFunci(__GLcontext * gc, GLuint buf, GLenum sfactor, GLenum dfactor);
extern GLvoid GL_APIENTRY __gles_BlendFuncSeparatei(__GLcontext * gc, GLuint buf, GLenum sfactorRGB,GLenum dfactorRGB,GLenum sfactorAlpha,GLenum dfactorAlpha);
extern GLvoid GL_APIENTRY __gles_ColorMaski(__GLcontext * gc,GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
extern GLvoid GL_APIENTRY __gles_Enablei(__GLcontext *gc, GLenum target, GLuint index);
extern GLvoid GL_APIENTRY __gles_Disablei(__GLcontext *gc, GLenum target, GLuint index);
extern GLboolean GL_APIENTRY __gles_IsEnabledi(__GLcontext * gc, GLenum target, GLuint index);

extern GLvoid GL_APIENTRY __gles_TexParameterIiv(__GLcontext *gc, GLenum target, GLenum pname, const GLint *params);
extern GLvoid GL_APIENTRY __gles_TexParameterIuiv(__GLcontext *gc, GLenum target, GLenum pname, const GLuint *params);
extern GLvoid GL_APIENTRY __gles_GetTexParameterIiv(__GLcontext *gc, GLenum target, GLenum pname, GLint *params);
extern GLvoid GL_APIENTRY __gles_GetTexParameterIuiv(__GLcontext *gc, GLenum target, GLenum pname, GLuint *params);
extern GLvoid GL_APIENTRY __gles_SamplerParameterIiv(__GLcontext *gc, GLuint sampler, GLenum pname, const GLint *param);
extern GLvoid GL_APIENTRY __gles_SamplerParameterIuiv(__GLcontext *gc, GLuint sampler, GLenum pname, const GLuint *param);
extern GLvoid GL_APIENTRY __gles_GetSamplerParameterIiv(__GLcontext *gc, GLuint sampler, GLenum pname, GLint *params);
extern GLvoid GL_APIENTRY __gles_GetSamplerParameterIuiv(__GLcontext *gc, GLuint sampler, GLenum pname, GLuint *params);

extern GLvoid GL_APIENTRY __gles_TexBuffer(__GLcontext *gc, GLenum target, GLenum internalformat, GLuint buffer);
extern GLvoid GL_APIENTRY __gles_TexBufferRange(__GLcontext *gc, GLenum target, GLenum internalformat,
                                                   GLuint buffer, GLintptr offset, GLsizeiptr size);
GLvoid GL_APIENTRY __gles_PatchParameteri(__GLcontext *gc, GLenum pname, GLint value);

GLvoid GL_APIENTRY __gles_FramebufferTexture(__GLcontext *gc, GLenum target, GLenum attachment, GLuint texture, GLint level);

GLvoid GL_APIENTRY __gles_MinSampleShading(__GLcontext *gc, GLfloat value);

GLvoid GL_APIENTRY __gles_CopyImageSubData(__GLcontext *gc,
                                           GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ,
                                           GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ,
                                           GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth);

extern GLvoid GL_APIENTRY __gles_DrawElementsBaseVertex(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex);
extern GLvoid GL_APIENTRY __gles_DrawRangeElementsBaseVertex(__GLcontext *gc, GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex);
extern GLvoid GL_APIENTRY __gles_DrawElementsInstancedBaseVertex(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex);
extern GLvoid GL_APIENTRY __gles_PrimitiveBoundingBox(__GLcontext *gc, GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW);

/* OpenGL ES extension */
#if GL_OES_EGL_image
extern GLvoid GL_APIENTRY __gles_EGLImageTargetTexture2DOES(__GLcontext *gc, GLenum target, GLeglImageOES image);
extern GLvoid GL_APIENTRY __gles_EGLImageTargetRenderbufferStorageOES(__GLcontext *gc, GLenum target, GLeglImageOES image);
#endif

#if GL_EXT_multi_draw_arrays
extern GLvoid GL_APIENTRY __gles_MultiDrawArraysEXT(__GLcontext *gc, GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount);
extern GLvoid GL_APIENTRY __gles_MultiDrawElementsEXT(__GLcontext *gc, GLenum mode, const GLsizei *count, GLenum type, const GLvoid*const*indices, GLsizei primcount);
#if GL_EXT_draw_elements_base_vertex
extern GLvoid GL_APIENTRY __gles_MultiDrawElementsBaseVertexEXT(__GLcontext *gc, GLenum mode, const GLsizei * count, GLenum type, const void *const*indices, GLsizei drawcount, const GLint * basevertex);
#endif
#endif

#if GL_OES_mapbuffer
extern GLvoid GL_APIENTRY __gles_GetBufferPointervOES(__GLcontext *gc, GLenum target, GLenum pname, GLvoid** params);
extern GLvoid* GL_APIENTRY __gles_MapBufferOES(__GLcontext *gc, GLenum target, GLenum access);
extern GLboolean GL_APIENTRY __gles_UnmapBufferOES(__GLcontext *gc, GLenum target);
#endif

#if GL_EXT_discard_framebuffer
extern GLvoid GL_APIENTRY __gles_DiscardFramebufferEXT(__GLcontext *gc, GLenum target, GLsizei numAttachments, const GLenum *attachments);
#endif

#if GL_EXT_multisampled_render_to_texture
extern GLvoid GL_APIENTRY __gles_RenderbufferStorageMultisampleEXT(__GLcontext *gc, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
extern GLvoid GL_APIENTRY __gles_FramebufferTexture2DMultisampleEXT(__GLcontext *gc, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples);
#endif

#if GL_VIV_direct_texture
extern GLvoid GL_APIENTRY __gles_TexDirectVIV(__GLcontext *gc, GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** pixels);
extern GLvoid GL_APIENTRY __gles_TexDirectInvalidateVIV(__GLcontext *gc, GLenum target);
extern GLvoid GL_APIENTRY __gles_TexDirectVIVMap(__GLcontext *gc, GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** logical, const GLuint * physical);
extern GLvoid GL_APIENTRY __gles_TexDirectTiledMapVIV(__GLcontext *gc, GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** logical, const GLuint * physical);
#endif

#if GL_EXT_multi_draw_indirect
extern GLvoid GL_APIENTRY __gles_MultiDrawArraysIndirectEXT(__GLcontext *gc, GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride);
extern GLvoid GL_APIENTRY __gles_MultiDrawElementsIndirectEXT(__GLcontext *gc, GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride);
#endif

extern GLvoid GL_APIENTRY __gles_GetTexImage(__GLcontext *gc, GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
#endif /* __gc_es_protos_h__ */
