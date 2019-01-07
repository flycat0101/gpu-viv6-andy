/****************************************************************************
*
*    Copyright 2012 - 2018 Vivante Corporation, Santa Clara, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/


#ifndef __gl3rename_h_
#define __gl3rename_h_

#if defined(_GL_3_APPENDIX)

#define _GL_3_RENAME_2(api, appendix)        api ## appendix
#define _GL_3_RENAME_1(api, appendix)        _GL_3_RENAME_2(api, appendix)
#define gcmGLES3(api)                        _GL_3_RENAME_1(api, _GL_3_APPENDIX)

#define glActiveTexture                      gcmGLES3(glActiveTexture)
#define glAttachShader                       gcmGLES3(glAttachShader)
#define glBindAttribLocation                 gcmGLES3(glBindAttribLocation)
#define glBindBuffer                         gcmGLES3(glBindBuffer)
#define glBindFramebuffer                    gcmGLES3(glBindFramebuffer)
#define glBindRenderbuffer                   gcmGLES3(glBindRenderbuffer)
#define glBindTexture                        gcmGLES3(glBindTexture)
#define glBlendColor                         gcmGLES3(glBlendColor)
#define glBlendEquation                      gcmGLES3(glBlendEquation)
#define glBlendEquationSeparate              gcmGLES3(glBlendEquationSeparate)
#define glBlendFunc                          gcmGLES3(glBlendFunc)
#define glBlendFuncSeparate                  gcmGLES3(glBlendFuncSeparate)
#define glBufferData                         gcmGLES3(glBufferData)
#define glBufferSubData                      gcmGLES3(glBufferSubData)
#define glCheckFramebufferStatus             gcmGLES3(glCheckFramebufferStatus)
#define glClear                              gcmGLES3(glClear)
#define glClearColor                         gcmGLES3(glClearColor)
#define glClearDepthf                        gcmGLES3(glClearDepthf)
#define glClearStencil                       gcmGLES3(glClearStencil)
#define glColorMask                          gcmGLES3(glColorMask)
#define glCompileShader                      gcmGLES3(glCompileShader)
#define glCompressedTexImage2D               gcmGLES3(glCompressedTexImage2D)
#define glCompressedTexSubImage2D            gcmGLES3(glCompressedTexSubImage2D)
#define glCopyTexImage2D                     gcmGLES3(glCopyTexImage2D)
#define glCopyTexSubImage2D                  gcmGLES3(glCopyTexSubImage2D)
#define glCreateProgram                      gcmGLES3(glCreateProgram)
#define glCreateShader                       gcmGLES3(glCreateShader)
#define glCullFace                           gcmGLES3(glCullFace)
#define glDeleteBuffers                      gcmGLES3(glDeleteBuffers)
#define glDeleteFramebuffers                 gcmGLES3(glDeleteFramebuffers)
#define glDeleteProgram                      gcmGLES3(glDeleteProgram)
#define glDeleteRenderbuffers                gcmGLES3(glDeleteRenderbuffers)
#define glDeleteShader                       gcmGLES3(glDeleteShader)
#define glDeleteTextures                     gcmGLES3(glDeleteTextures)
#define glDepthFunc                          gcmGLES3(glDepthFunc)
#define glDepthMask                          gcmGLES3(glDepthMask)
#define glDepthRangef                        gcmGLES3(glDepthRangef)
#define glDetachShader                       gcmGLES3(glDetachShader)
#define glDisable                            gcmGLES3(glDisable)
#define glDisableVertexAttribArray           gcmGLES3(glDisableVertexAttribArray)
#define glDrawArrays                         gcmGLES3(glDrawArrays)
#define glDrawElements                       gcmGLES3(glDrawElements)
#define glEnable                             gcmGLES3(glEnable)
#define glEnableVertexAttribArray            gcmGLES3(glEnableVertexAttribArray)
#define glFinish                             gcmGLES3(glFinish)
#define glFlush                              gcmGLES3(glFlush)
#define glFramebufferRenderbuffer            gcmGLES3(glFramebufferRenderbuffer)
#define glFramebufferTexture2D               gcmGLES3(glFramebufferTexture2D)
#define glFrontFace                          gcmGLES3(glFrontFace)
#define glGenBuffers                         gcmGLES3(glGenBuffers)
#define glGenerateMipmap                     gcmGLES3(glGenerateMipmap)
#define glGenFramebuffers                    gcmGLES3(glGenFramebuffers)
#define glGenRenderbuffers                   gcmGLES3(glGenRenderbuffers)
#define glGenTextures                        gcmGLES3(glGenTextures)
#define glGetActiveAttrib                    gcmGLES3(glGetActiveAttrib)
#define glGetActiveUniform                   gcmGLES3(glGetActiveUniform)
#define glGetAttachedShaders                 gcmGLES3(glGetAttachedShaders)
#define glGetAttribLocation                  gcmGLES3(glGetAttribLocation)
#define glGetBooleanv                        gcmGLES3(glGetBooleanv)
#define glGetBufferParameteriv               gcmGLES3(glGetBufferParameteriv)
#define glGetError                           gcmGLES3(glGetError)
#define glGetFloatv                          gcmGLES3(glGetFloatv)
#define glGetFramebufferAttachmentParameteriv \
            gcmGLES3(glGetFramebufferAttachmentParameteriv)
#define glGetIntegerv                        gcmGLES3(glGetIntegerv)
#define glGetProgramInfoLog                  gcmGLES3(glGetProgramInfoLog)
#define glGetProgramiv                       gcmGLES3(glGetProgramiv)
#define glGetRenderbufferParameteriv \
            gcmGLES3(glGetRenderbufferParameteriv)
#define glGetShaderiv                        gcmGLES3(glGetShaderiv)
#define glGetShaderInfoLog                   gcmGLES3(glGetShaderInfoLog)
#define glGetShaderPrecisionFormat           gcmGLES3(glGetShaderPrecisionFormat)
#define glGetShaderSource                    gcmGLES3(glGetShaderSource)
#define glGetString                          gcmGLES3(glGetString)
#define glGetTexParameterfv                  gcmGLES3(glGetTexParameterfv)
#define glGetTexParameteriv                  gcmGLES3(glGetTexParameteriv)
#define glGetUniformfv                       gcmGLES3(glGetUniformfv)
#define glGetUniformiv                       gcmGLES3(glGetUniformiv)
#define glGetUniformLocation                 gcmGLES3(glGetUniformLocation)
#define glGetVertexAttribfv                  gcmGLES3(glGetVertexAttribfv)
#define glGetVertexAttribiv                  gcmGLES3(glGetVertexAttribiv)
#define glGetVertexAttribPointerv            gcmGLES3(glGetVertexAttribPointerv)
#define glHint                               gcmGLES3(glHint)
#define glIsBuffer                           gcmGLES3(glIsBuffer)
#define glIsEnabled                          gcmGLES3(glIsEnabled)
#define glIsFramebuffer                      gcmGLES3(glIsFramebuffer)
#define glIsProgram                          gcmGLES3(glIsProgram)
#define glIsRenderbuffer                     gcmGLES3(glIsRenderbuffer)
#define glIsShader                           gcmGLES3(glIsShader)
#define glIsTexture                          gcmGLES3(glIsTexture)
#define glLineWidth                          gcmGLES3(glLineWidth)
#define glLinkProgram                        gcmGLES3(glLinkProgram)
#define glPixelStorei                        gcmGLES3(glPixelStorei)
#define glPolygonOffset                      gcmGLES3(glPolygonOffset)
#define glReadPixels                         gcmGLES3(glReadPixels)
#define glReleaseShaderCompiler              gcmGLES3(glReleaseShaderCompiler)
#define glRenderbufferStorage                gcmGLES3(glRenderbufferStorage)
#define glSampleCoverage                     gcmGLES3(glSampleCoverage)
#define glScissor                            gcmGLES3(glScissor)
#define glShaderBinary                       gcmGLES3(glShaderBinary)
#define glShaderSource                       gcmGLES3(glShaderSource)
#define glStencilFunc                        gcmGLES3(glStencilFunc)
#define glStencilFuncSeparate                gcmGLES3(glStencilFuncSeparate)
#define glStencilMask                        gcmGLES3(glStencilMask)
#define glStencilMaskSeparate                gcmGLES3(glStencilMaskSeparate)
#define glStencilOp                          gcmGLES3(glStencilOp)
#define glStencilOpSeparate                  gcmGLES3(glStencilOpSeparate)
#define glTexImage2D                         gcmGLES3(glTexImage2D)
#define glTexParameterf                      gcmGLES3(glTexParameterf)
#define glTexParameterfv                     gcmGLES3(glTexParameterfv)
#define glTexParameteri                      gcmGLES3(glTexParameteri)
#define glTexParameteriv                     gcmGLES3(glTexParameteriv)
#define glTexSubImage2D                      gcmGLES3(glTexSubImage2D)
#define glTexSubImage3DOES                   gcmGLES3(glTexSubImage3DOES)
#define glUniform1f                          gcmGLES3(glUniform1f)
#define glUniform1fv                         gcmGLES3(glUniform1fv)
#define glUniform1i                          gcmGLES3(glUniform1i)
#define glUniform1iv                         gcmGLES3(glUniform1iv)
#define glUniform2f                          gcmGLES3(glUniform2f)
#define glUniform2fv                         gcmGLES3(glUniform2fv)
#define glUniform2i                          gcmGLES3(glUniform2i)
#define glUniform2iv                         gcmGLES3(glUniform2iv)
#define glUniform3f                          gcmGLES3(glUniform3f)
#define glUniform3fv                         gcmGLES3(glUniform3fv)
#define glUniform3i                          gcmGLES3(glUniform3i)
#define glUniform3iv                         gcmGLES3(glUniform3iv)
#define glUniform4f                          gcmGLES3(glUniform4f)
#define glUniform4fv                         gcmGLES3(glUniform4fv)
#define glUniform4i                          gcmGLES3(glUniform4i)
#define glUniform4iv                         gcmGLES3(glUniform4iv)
#define glUniformMatrix2fv                   gcmGLES3(glUniformMatrix2fv)
#define glUniformMatrix3fv                   gcmGLES3(glUniformMatrix3fv)
#define glUniformMatrix4fv                   gcmGLES3(glUniformMatrix4fv)
#define glUseProgram                         gcmGLES3(glUseProgram)
#define glValidateProgram                    gcmGLES3(glValidateProgram)
#define glVertexAttrib1f                     gcmGLES3(glVertexAttrib1f)
#define glVertexAttrib1fv                    gcmGLES3(glVertexAttrib1fv)
#define glVertexAttrib2f                     gcmGLES3(glVertexAttrib2f)
#define glVertexAttrib2fv                    gcmGLES3(glVertexAttrib2fv)
#define glVertexAttrib3f                     gcmGLES3(glVertexAttrib3f)
#define glVertexAttrib3fv                    gcmGLES3(glVertexAttrib3fv)
#define glVertexAttrib4f                     gcmGLES3(glVertexAttrib4f)
#define glVertexAttrib4fv                    gcmGLES3(glVertexAttrib4fv)
#define glVertexAttribPointer                gcmGLES3(glVertexAttribPointer)
#define glViewport                           gcmGLES3(glViewport)

/*OpenGL ES 3.0 */
#define glReadBuffer                         gcmGLES3(glReadBuffer)
#define glDrawRangeElements                  gcmGLES3(glDrawRangeElements)
#define glTexImage3D                         gcmGLES3(glTexImage3D)
#define glTexSubImage3D                      gcmGLES3(glTexSubImage3D)
#define glCopyTexSubImage3D                  gcmGLES3(glCopyTexSubImage3D)
#define glCompressedTexImage3D               gcmGLES3(glCompressedTexImage3D)
#define glCompressedTexSubImage3D            gcmGLES3(glCompressedTexSubImage3D)
#define glGenQueries                         gcmGLES3(glGenQueries)
#define glDeleteQueries                      gcmGLES3(glDeleteQueries)
#define glIsQuery                            gcmGLES3(glIsQuery)
#define glBeginQuery                         gcmGLES3(glBeginQuery)
#define glEndQuery                           gcmGLES3(glEndQuery)
#define glGetQueryiv                         gcmGLES3(glGetQueryiv)
#define glGetQueryObjectuiv                  gcmGLES3(glGetQueryObjectuiv)
#define glUnmapBuffer                        gcmGLES3(glUnmapBuffer)
#define glGetBufferPointerv                  gcmGLES3(glGetBufferPointerv)
#define glDrawBuffers                        gcmGLES3(glDrawBuffers)
#define glUniformMatrix2x3fv                 gcmGLES3(glUniformMatrix2x3fv)
#define glUniformMatrix3x2fv                 gcmGLES3(glUniformMatrix3x2fv)
#define glUniformMatrix2x4fv                 gcmGLES3(glUniformMatrix2x4fv)
#define glUniformMatrix4x2fv                 gcmGLES3(glUniformMatrix4x2fv)
#define glUniformMatrix3x4fv                 gcmGLES3(glUniformMatrix3x4fv)
#define glUniformMatrix4x3fv                 gcmGLES3(glUniformMatrix4x3fv)
#define glBlitFramebuffer                    gcmGLES3(glBlitFramebuffer)
#define glRenderbufferStorageMultisample     gcmGLES3(glRenderbufferStorageMultisample)
#define glFramebufferTextureLayer            gcmGLES3(glFramebufferTextureLayer)
#define glMapBufferRange                     gcmGLES3(glMapBufferRange)
#define glFlushMappedBufferRange             gcmGLES3(glFlushMappedBufferRange)
#define glBindVertexArray                    gcmGLES3(glBindVertexArray)
#define glDeleteVertexArrays                 gcmGLES3(glDeleteVertexArrays)
#define glGenVertexArrays                    gcmGLES3(glGenVertexArrays)
#define glIsVertexArray                      gcmGLES3(glIsVertexArray)
#define glGetIntegeri_v                      gcmGLES3(glGetIntegeri_v)
#define glBeginTransformFeedback             gcmGLES3(glBeginTransformFeedback)
#define glEndTransformFeedback               gcmGLES3(glEndTransformFeedback)
#define glBindBufferRange                    gcmGLES3(glBindBufferRange)
#define glBindBufferBase                     gcmGLES3(glBindBufferBase)
#define glTransformFeedbackVaryings          gcmGLES3(glTransformFeedbackVaryings)
#define glGetTransformFeedbackVarying        gcmGLES3(glGetTransformFeedbackVarying)
#define glVertexAttribIPointer               gcmGLES3(glVertexAttribIPointer)
#define glGetVertexAttribIiv                 gcmGLES3(glGetVertexAttribIiv)
#define glGetVertexAttribIuiv                gcmGLES3(glGetVertexAttribIuiv)
#define glVertexAttribI4i                    gcmGLES3(glVertexAttribI4i)
#define glVertexAttribI4ui                   gcmGLES3(glVertexAttribI4ui)
#define glVertexAttribI4iv                   gcmGLES3(glVertexAttribI4iv)
#define glVertexAttribI4uiv                  gcmGLES3(glVertexAttribI4uiv)
#define glGetUniformuiv                      gcmGLES3(glGetUniformuiv)
#define glGetFragDataLocation                gcmGLES3(glGetFragDataLocation)
#define glUniform1ui                         gcmGLES3(glUniform1ui)
#define glUniform2ui                         gcmGLES3(glUniform2ui)
#define glUniform3ui                         gcmGLES3(glUniform3ui)
#define glUniform4ui                         gcmGLES3(glUniform4ui)
#define glUniform1uiv                        gcmGLES3(glUniform1uiv)
#define glUniform2uiv                        gcmGLES3(glUniform2uiv)
#define glUniform3uiv                        gcmGLES3(glUniform3uiv)
#define glUniform4uiv                        gcmGLES3(glUniform4uiv)
#define glClearBufferiv                      gcmGLES3(glClearBufferiv)
#define glClearBufferuiv                     gcmGLES3(glClearBufferuiv)
#define glClearBufferfv                      gcmGLES3(glClearBufferfv)
#define glClearBufferfi                      gcmGLES3(glClearBufferfi)
#define glGetStringi                         gcmGLES3(glGetStringi)
#define glCopyBufferSubData                  gcmGLES3(glCopyBufferSubData)
#define glGetUniformIndices                  gcmGLES3(glGetUniformIndices)
#define glGetActiveUniformsiv                gcmGLES3(glGetActiveUniformsiv)
#define glGetUniformBlockIndex               gcmGLES3(glGetUniformBlockIndex)
#define glGetActiveUniformBlockiv            gcmGLES3(glGetActiveUniformBlockiv)
#define glGetActiveUniformBlockName          gcmGLES3(glGetActiveUniformBlockName)
#define glUniformBlockBinding                gcmGLES3(glUniformBlockBinding)
#define glDrawArraysInstanced                gcmGLES3(glDrawArraysInstanced)
#define glDrawElementsInstanced              gcmGLES3(glDrawElementsInstanced)
#define glFenceSync                          gcmGLES3(glFenceSync)
#define glIsSync                             gcmGLES3(glIsSync)
#define glDeleteSync                         gcmGLES3(glDeleteSync)
#define glClientWaitSync                     gcmGLES3(glClientWaitSync)
#define glWaitSync                           gcmGLES3(glWaitSync)
#define glGetInteger64v                      gcmGLES3(glGetInteger64v)
#define glGetSynciv                          gcmGLES3(glGetSynciv)
#define glGetInteger64i_v                    gcmGLES3(glGetInteger64i_v)
#define glGetBufferParameteri64v             gcmGLES3(glGetBufferParameteri64v)
#define glGenSamplers                        gcmGLES3(glGenSamplers)
#define glDeleteSamplers                     gcmGLES3(glDeleteSamplers)
#define glIsSampler                          gcmGLES3(glIsSampler)
#define glBindSampler                        gcmGLES3(glBindSampler)
#define glSamplerParameteri                  gcmGLES3(glSamplerParameteri)
#define glSamplerParameteriv                 gcmGLES3(glSamplerParameteriv)
#define glSamplerParameterf                  gcmGLES3(glSamplerParameterf)
#define glSamplerParameterfv                 gcmGLES3(glSamplerParameterfv)
#define glGetSamplerParameteriv              gcmGLES3(glGetSamplerParameteriv)
#define glGetSamplerParameterfv              gcmGLES3(glGetSamplerParameterfv)
#define glVertexAttribDivisor                gcmGLES3(glVertexAttribDivisor)
#define glBindTransformFeedback              gcmGLES3(glBindTransformFeedback)
#define glDeleteTransformFeedbacks           gcmGLES3(glDeleteTransformFeedbacks)
#define glGenTransformFeedbacks              gcmGLES3(glGenTransformFeedbacks)
#define glIsTransformFeedback                gcmGLES3(glIsTransformFeedback)
#define glPauseTransformFeedback             gcmGLES3(glPauseTransformFeedback)
#define glResumeTransformFeedback            gcmGLES3(glResumeTransformFeedback)
#define glGetProgramBinary                   gcmGLES3(glGetProgramBinary)
#define glProgramBinary                      gcmGLES3(glProgramBinary)
#define glProgramParameteri                  gcmGLES3(glProgramParameteri)
#define glInvalidateFramebuffer              gcmGLES3(glInvalidateFramebuffer)
#define glInvalidateSubFramebuffer           gcmGLES3(glInvalidateSubFramebuffer)
#define glTexStorage2D                       gcmGLES3(glTexStorage2D)
#define glTexStorage3D                       gcmGLES3(glTexStorage3D)
#define glGetInternalformativ                gcmGLES3(glGetInternalformativ)

#endif /* _GL_3_APPENDIX */
#endif /* __gl3rename_h_ */
