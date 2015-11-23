//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __GLExtensions_H__
#define __GLExtensions_H__

#include "glext.h"

void OpenGL_InitExtensions();

extern PFNGLTEXIMAGE3DPROC                  glTexImage3D;
extern PFNGLFRAMEBUFFERTEXTURELAYERPROC     glFramebufferTextureLayer;

extern PFNGLUNIFORM1FPROC                   glUniform1f;
extern PFNGLUNIFORM2FPROC                   glUniform2f;
extern PFNGLUNIFORM3FPROC                   glUniform3f;
extern PFNGLUNIFORM4FPROC                   glUniform4f;
extern PFNGLUNIFORM1IPROC                   glUniform1i;
extern PFNGLUNIFORM2IPROC                   glUniform2i;
extern PFNGLUNIFORM3IPROC                   glUniform3i;
extern PFNGLUNIFORM4IPROC                   glUniform4i;
extern PFNGLUNIFORM1FVPROC                  glUniform1fv;
extern PFNGLUNIFORM2FVPROC                  glUniform2fv;
extern PFNGLUNIFORM3FVPROC                  glUniform3fv;
extern PFNGLUNIFORM4FVPROC                  glUniform4fv;
extern PFNGLUNIFORM1IVPROC                  glUniform1iv;
extern PFNGLUNIFORM2IVPROC                  glUniform2iv;
extern PFNGLUNIFORM3IVPROC                  glUniform3iv;
extern PFNGLUNIFORM4IVPROC                  glUniform4iv;

extern PFNGLUNIFORMMATRIX4FVPROC            glUniformMatrix4fv;
extern PFNGLVERTEXATTRIB1FPROC              glVertexAttrib1f;
extern PFNGLVERTEXATTRIB2FPROC              glVertexAttrib2f;
extern PFNGLVERTEXATTRIB3FPROC              glVertexAttrib3f;
extern PFNGLVERTEXATTRIB4FPROC              glVertexAttrib4f;
extern PFNGLVERTEXATTRIB1FVPROC             glVertexAttrib1fv;
extern PFNGLVERTEXATTRIB2FVPROC             glVertexAttrib2fv;
extern PFNGLVERTEXATTRIB3FVPROC             glVertexAttrib3fv;
extern PFNGLVERTEXATTRIB4FVPROC             glVertexAttrib4fv;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC     glEnableVertexAttribArray;
extern PFNGLBINDATTRIBLOCATIONPROC          glBindAttribLocation;

extern PFNGLDISABLEVERTEXATTRIBARRAYPROC    glDisableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC         glVertexAttribPointer;
extern PFNGLVERTEXATTRIBIPOINTERPROC        glVertexAttribIPointer;

extern PFNGLCREATESHADERPROC                glCreateShader;
extern PFNGLSHADERSOURCEPROC                glShaderSource;
extern PFNGLCOMPILESHADERPROC               glCompileShader;
extern PFNGLCREATEPROGRAMPROC               glCreateProgram;
extern PFNGLATTACHSHADERPROC                glAttachShader;
extern PFNGLLINKPROGRAMPROC                 glLinkProgram;
extern PFNGLDETACHSHADERPROC                glDetachShader;
extern PFNGLDELETEPROGRAMPROC               glDeleteProgram;
extern PFNGLDELETESHADERPROC                glDeleteShader;
extern PFNGLUSEPROGRAMPROC                  glUseProgram;
extern PFNGLGETSHADERINFOLOGPROC            glGetShaderInfoLog;

extern PFNGLGETATTRIBLOCATIONPROC           glGetAttribLocation;
extern PFNGLGETPROGRAMINFOLOGPROC           glGetProgramInfoLog;
extern PFNGLGETPROGRAMIVPROC                glGetProgramiv;
extern PFNGLGETSHADERINFOLOGPROC            glGetShaderInfoLog;
extern PFNGLGETSHADERSOURCEPROC             glGetShaderSource;
extern PFNGLGETSHADERIVPROC                 glGetShaderiv;
extern PFNGLGETUNIFORMLOCATIONPROC          glGetUniformLocation;

extern PFNGLACTIVETEXTUREPROC               glActiveTexture;

extern PFNGLGENBUFFERSPROC                  glGenBuffers;
extern PFNGLBINDBUFFERPROC                  glBindBuffer;
extern PFNGLBUFFERDATAPROC                  glBufferData;
extern PFNGLBUFFERSUBDATAPROC               glBufferSubData;
extern PFNGLDELETEBUFFERSPROC               glDeleteBuffers;

extern PFNGLBLENDFUNCSEPARATEPROC           glBlendFuncSeparate;
extern PFNGLBLENDCOLORPROC                  glBlendColor;

extern PFNGLDELETEFRAMEBUFFERSPROC          glDeleteFramebuffers;
extern PFNGLGENFRAMEBUFFERSPROC             glGenFramebuffers;
extern PFNGLBINDFRAMEBUFFERPROC             glBindFramebuffer;
extern PFNGLFRAMEBUFFERTEXTURE2DPROC        glFramebufferTexture2D;
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC      glCheckFramebufferStatus;

extern PFNGLGENRENDERBUFFERSPROC            glGenRenderbuffers;
extern PFNGLBINDRENDERBUFFERPROC            glBindRenderbuffer;
extern PFNGLFRAMEBUFFERRENDERBUFFERPROC     glFramebufferRenderbuffer;
extern PFNGLRENDERBUFFERSTORAGEPROC         glRenderbufferStorage;
extern PFNGLDELETERENDERBUFFERSPROC         glDeleteRenderbuffers;

extern PFNGLGENVERTEXARRAYSPROC             glGenVertexArrays;
extern PFNGLDELETEVERTEXARRAYSPROC          glDeleteVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC             glBindVertexArray;

extern PFNGLGENERATEMIPMAPPROC              glGenerateMipmap;

extern PFNGLDRAWARRAYSINSTANCEDPROC         glDrawArraysInstanced;      //(GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
extern PFNGLDRAWELEMENTSINSTANCEDPROC       glDrawElementsInstanced;    //(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount);
extern PFNGLVERTEXATTRIBDIVISORPROC         glVertexAttribDivisor;      //(GLuint index, GLuint divisor);

//extern PFNGLVERTEXBINDINGDIVISORPROC        glVertexBindingDivisor;     //(GLuint bindingindex, GLuint divisor)

#endif //__GLExtensions_H__
