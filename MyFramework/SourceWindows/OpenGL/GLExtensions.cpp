//
// Copyright (c) 2012-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "GLExtensions.h"

#pragma warning( push )
#pragma warning( disable : 4191 ) // Unsafe conversion from 'type of expression' to 'type required'.

PFNGLTEXIMAGE3DPROC                 glTexImage3D = nullptr;
PFNGLFRAMEBUFFERTEXTURELAYERPROC    glFramebufferTextureLayer = nullptr;

PFNGLUNIFORM1FPROC                  glUniform1f = nullptr;
PFNGLUNIFORM2FPROC                  glUniform2f = nullptr;
PFNGLUNIFORM3FPROC                  glUniform3f = nullptr;
PFNGLUNIFORM4FPROC                  glUniform4f = nullptr;
PFNGLUNIFORM1IPROC                  glUniform1i = nullptr;
PFNGLUNIFORM2IPROC                  glUniform2i = nullptr;
PFNGLUNIFORM3IPROC                  glUniform3i = nullptr;
PFNGLUNIFORM4IPROC                  glUniform4i = nullptr;
PFNGLUNIFORM1FVPROC                 glUniform1fv = nullptr;
PFNGLUNIFORM2FVPROC                 glUniform2fv = nullptr;
PFNGLUNIFORM3FVPROC                 glUniform3fv = nullptr;
PFNGLUNIFORM4FVPROC                 glUniform4fv = nullptr;
PFNGLUNIFORM1IVPROC                 glUniform1iv = nullptr;
PFNGLUNIFORM2IVPROC                 glUniform2iv = nullptr;
PFNGLUNIFORM3IVPROC                 glUniform3iv = nullptr;
PFNGLUNIFORM4IVPROC                 glUniform4iv = nullptr;

PFNGLUNIFORMMATRIX3FVPROC           glUniformMatrix3fv = nullptr;
PFNGLUNIFORMMATRIX4FVPROC           glUniformMatrix4fv = nullptr;
PFNGLVERTEXATTRIB1FPROC             glVertexAttrib1f = nullptr;
PFNGLVERTEXATTRIB2FPROC             glVertexAttrib2f = nullptr;
PFNGLVERTEXATTRIB3FPROC             glVertexAttrib3f = nullptr;
PFNGLVERTEXATTRIB4FPROC             glVertexAttrib4f = nullptr;
PFNGLVERTEXATTRIB1FVPROC            glVertexAttrib1fv = nullptr;
PFNGLVERTEXATTRIB2FVPROC            glVertexAttrib2fv = nullptr;
PFNGLVERTEXATTRIB3FVPROC            glVertexAttrib3fv = nullptr;
PFNGLVERTEXATTRIB4FVPROC            glVertexAttrib4fv = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC    glEnableVertexAttribArray = nullptr;
PFNGLBINDATTRIBLOCATIONPROC         glBindAttribLocation = nullptr;

PFNGLDISABLEVERTEXATTRIBARRAYPROC   glDisableVertexAttribArray = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC        glVertexAttribPointer = nullptr;
PFNGLVERTEXATTRIBIPOINTERPROC       glVertexAttribIPointer = nullptr;

PFNGLCREATESHADERPROC               glCreateShader = nullptr;
PFNGLSHADERSOURCEPROC               glShaderSource = nullptr;
PFNGLCOMPILESHADERPROC              glCompileShader = nullptr;
PFNGLCREATEPROGRAMPROC              glCreateProgram = nullptr;
PFNGLATTACHSHADERPROC               glAttachShader = nullptr;
PFNGLLINKPROGRAMPROC                glLinkProgram = nullptr;
PFNGLDETACHSHADERPROC               glDetachShader = nullptr;
PFNGLDELETEPROGRAMPROC              glDeleteProgram = nullptr;
PFNGLDELETESHADERPROC               glDeleteShader = nullptr;
PFNGLUSEPROGRAMPROC                 glUseProgram = nullptr;

PFNGLGETATTRIBLOCATIONPROC          glGetAttribLocation = nullptr;
PFNGLGETPROGRAMINFOLOGPROC          glGetProgramInfoLog = nullptr;
PFNGLGETPROGRAMIVPROC               glGetProgramiv = nullptr;
PFNGLGETSHADERINFOLOGPROC           glGetShaderInfoLog = nullptr;
PFNGLGETSHADERSOURCEPROC            glGetShaderSource = nullptr;
PFNGLGETSHADERIVPROC                glGetShaderiv = nullptr;
PFNGLGETUNIFORMLOCATIONPROC         glGetUniformLocation = nullptr;

PFNGLACTIVETEXTUREPROC              glActiveTexture = nullptr;

PFNGLGENBUFFERSPROC                 glGenBuffers = nullptr;
PFNGLBINDBUFFERPROC                 glBindBuffer = nullptr;
PFNGLBUFFERDATAPROC                 glBufferData = nullptr;
PFNGLBUFFERSUBDATAPROC              glBufferSubData = nullptr;
PFNGLGETBUFFERSUBDATAPROC           glGetBufferSubData = nullptr;
PFNGLDELETEBUFFERSPROC              glDeleteBuffers = nullptr;

PFNGLBLENDEQUATIONPROC              glBlendEquation = nullptr;
PFNGLBLENDEQUATIONSEPARATEPROC      glBlendEquationSeparate = nullptr;
PFNGLBLENDFUNCSEPARATEPROC          glBlendFuncSeparate = nullptr;
PFNGLBLENDCOLORPROC                 glBlendColor = nullptr;

PFNGLDELETEFRAMEBUFFERSPROC         glDeleteFramebuffers = nullptr;
PFNGLGENFRAMEBUFFERSPROC            glGenFramebuffers = nullptr;
PFNGLBINDFRAMEBUFFERPROC            glBindFramebuffer = nullptr;
PFNGLFRAMEBUFFERTEXTURE2DPROC       glFramebufferTexture2D = nullptr;
PFNGLCHECKFRAMEBUFFERSTATUSPROC     glCheckFramebufferStatus = nullptr;
PFNGLDRAWBUFFERSPROC                glDrawBuffers = nullptr;

PFNGLGENRENDERBUFFERSPROC           glGenRenderbuffers = nullptr;
PFNGLBINDRENDERBUFFERPROC           glBindRenderbuffer = nullptr;
PFNGLFRAMEBUFFERRENDERBUFFERPROC    glFramebufferRenderbuffer = nullptr;
PFNGLRENDERBUFFERSTORAGEPROC        glRenderbufferStorage = nullptr;
PFNGLDELETERENDERBUFFERSPROC        glDeleteRenderbuffers = nullptr;

PFNGLGENVERTEXARRAYSPROC            glGenVertexArrays = nullptr;
PFNGLDELETEVERTEXARRAYSPROC         glDeleteVertexArrays = nullptr;
PFNGLBINDVERTEXARRAYPROC            glBindVertexArray = nullptr;

PFNGLGENERATEMIPMAPPROC             glGenerateMipmap = nullptr;

PFNGLDRAWARRAYSINSTANCEDPROC        glDrawArraysInstanced = nullptr;      //(GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
PFNGLDRAWELEMENTSINSTANCEDPROC      glDrawElementsInstanced = nullptr;    //(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount);
PFNGLVERTEXATTRIBDIVISORPROC        glVertexAttribDivisor = nullptr;      //(GLuint index, GLuint divisor);

//PFNGLVERTEXBINDINGDIVISORPROC       glVertexBindingDivisor = nullptr;     //(GLuint bindingindex, GLuint divisor);

void OpenGL_InitExtensions()
{
    glTexImage3D                    = (PFNGLTEXIMAGE3DPROC)                 wglGetProcAddress( "glTexImage3D" );
    glFramebufferTextureLayer       = (PFNGLFRAMEBUFFERTEXTURELAYERPROC)    wglGetProcAddress( "glFramebufferTextureLayer" );

    glUniform1i                     = (PFNGLUNIFORM1IPROC)                  wglGetProcAddress( "glUniform1i" );
    glUniform2i                     = (PFNGLUNIFORM2IPROC)                  wglGetProcAddress( "glUniform2i" );
    glUniform3i                     = (PFNGLUNIFORM3IPROC)                  wglGetProcAddress( "glUniform3i" );
    glUniform4i                     = (PFNGLUNIFORM4IPROC)                  wglGetProcAddress( "glUniform4i" );
    glUniform1iv                    = (PFNGLUNIFORM1IVPROC)                 wglGetProcAddress( "glUniform1iv" );
    glUniform2iv                    = (PFNGLUNIFORM2IVPROC)                 wglGetProcAddress( "glUniform2iv" );
    glUniform3iv                    = (PFNGLUNIFORM3IVPROC)                 wglGetProcAddress( "glUniform3iv" );
    glUniform4iv                    = (PFNGLUNIFORM4IVPROC)                 wglGetProcAddress( "glUniform4iv" );
    glUniform1f                     = (PFNGLUNIFORM1FPROC)                  wglGetProcAddress( "glUniform1f" );
    glUniform2f                     = (PFNGLUNIFORM2FPROC)                  wglGetProcAddress( "glUniform2f" );
    glUniform3f                     = (PFNGLUNIFORM3FPROC)                  wglGetProcAddress( "glUniform3f" );
    glUniform4f                     = (PFNGLUNIFORM4FPROC)                  wglGetProcAddress( "glUniform4f" );
    glUniform1fv                    = (PFNGLUNIFORM1FVPROC)                 wglGetProcAddress( "glUniform1fv" );
    glUniform2fv                    = (PFNGLUNIFORM2FVPROC)                 wglGetProcAddress( "glUniform2fv" );
    glUniform3fv                    = (PFNGLUNIFORM3FVPROC)                 wglGetProcAddress( "glUniform3fv" );
    glUniform4fv                    = (PFNGLUNIFORM4FVPROC)                 wglGetProcAddress( "glUniform4fv" );
    glUniformMatrix3fv              = (PFNGLUNIFORMMATRIX3FVPROC)           wglGetProcAddress( "glUniformMatrix3fv" );
    glUniformMatrix4fv              = (PFNGLUNIFORMMATRIX4FVPROC)           wglGetProcAddress( "glUniformMatrix4fv" );
    glVertexAttrib1f                = (PFNGLVERTEXATTRIB1FPROC)             wglGetProcAddress( "glVertexAttrib1f" );
    glVertexAttrib2f                = (PFNGLVERTEXATTRIB2FPROC)             wglGetProcAddress( "glVertexAttrib2f" );
    glVertexAttrib3f                = (PFNGLVERTEXATTRIB3FPROC)             wglGetProcAddress( "glVertexAttrib3f" );
    glVertexAttrib4f                = (PFNGLVERTEXATTRIB4FPROC)             wglGetProcAddress( "glVertexAttrib4f" );
    glVertexAttrib1fv               = (PFNGLVERTEXATTRIB1FVPROC)            wglGetProcAddress( "glVertexAttrib1fv" );
    glVertexAttrib2fv               = (PFNGLVERTEXATTRIB2FVPROC)            wglGetProcAddress( "glVertexAttrib2fv" );
    glVertexAttrib3fv               = (PFNGLVERTEXATTRIB3FVPROC)            wglGetProcAddress( "glVertexAttrib3fv" );
    glVertexAttrib4fv               = (PFNGLVERTEXATTRIB4FVPROC)            wglGetProcAddress( "glVertexAttrib4fv" );
    glEnableVertexAttribArray       = (PFNGLENABLEVERTEXATTRIBARRAYPROC)    wglGetProcAddress( "glEnableVertexAttribArray" );
    glBindAttribLocation            = (PFNGLBINDATTRIBLOCATIONPROC)         wglGetProcAddress( "glBindAttribLocation" );

    glDisableVertexAttribArray      = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)   wglGetProcAddress( "glDisableVertexAttribArray" );
    glVertexAttribPointer           = (PFNGLVERTEXATTRIBPOINTERPROC)        wglGetProcAddress( "glVertexAttribPointer" );
    glVertexAttribIPointer          = (PFNGLVERTEXATTRIBIPOINTERPROC)       wglGetProcAddress( "glVertexAttribIPointer" );

    glCreateShader                  = (PFNGLCREATESHADERPROC)               wglGetProcAddress( "glCreateShader" );
    glShaderSource                  = (PFNGLSHADERSOURCEPROC)               wglGetProcAddress( "glShaderSource" );
    glCompileShader                 = (PFNGLCOMPILESHADERPROC)              wglGetProcAddress( "glCompileShader" );
    glCreateProgram                 = (PFNGLCREATEPROGRAMPROC)              wglGetProcAddress( "glCreateProgram" );
    glAttachShader                  = (PFNGLATTACHSHADERPROC)               wglGetProcAddress( "glAttachShader" );
    glLinkProgram                   = (PFNGLLINKPROGRAMPROC)                wglGetProcAddress( "glLinkProgram" );
    glDetachShader                  = (PFNGLDETACHSHADERPROC)               wglGetProcAddress( "glDetachShader" );
    glDeleteShader                  = (PFNGLDELETESHADERPROC)               wglGetProcAddress( "glDeleteShader" );
    glDeleteProgram                 = (PFNGLDELETEPROGRAMPROC)              wglGetProcAddress( "glDeleteProgram" );
    glUseProgram                    = (PFNGLUSEPROGRAMPROC)                 wglGetProcAddress( "glUseProgram" );

    glGetAttribLocation             = (PFNGLGETATTRIBLOCATIONPROC)          wglGetProcAddress( "glGetAttribLocation" );
    glGetProgramInfoLog             = (PFNGLGETPROGRAMINFOLOGPROC)          wglGetProcAddress( "glGetProgramInfoLog" );
    glGetProgramiv                  = (PFNGLGETPROGRAMIVPROC)               wglGetProcAddress( "glGetProgramiv" );
    glGetShaderInfoLog              = (PFNGLGETSHADERINFOLOGPROC)           wglGetProcAddress( "glGetShaderInfoLog" );
    glGetShaderSource               = (PFNGLGETSHADERSOURCEPROC)            wglGetProcAddress( "glGetShaderSource" );
    glGetShaderiv                   = (PFNGLGETSHADERIVPROC)                wglGetProcAddress( "glGetShaderiv" );
    glGetUniformLocation            = (PFNGLGETUNIFORMLOCATIONPROC)         wglGetProcAddress( "glGetUniformLocation" );

    glActiveTexture                 = (PFNGLACTIVETEXTUREPROC)              wglGetProcAddress( "glActiveTexture" );

    glGenBuffers                    = (PFNGLGENBUFFERSPROC)                 wglGetProcAddress( "glGenBuffers" );
    glBindBuffer                    = (PFNGLBINDBUFFERPROC)                 wglGetProcAddress( "glBindBuffer" );
    glBufferData                    = (PFNGLBUFFERDATAPROC)                 wglGetProcAddress( "glBufferData" );
    glBufferSubData                 = (PFNGLBUFFERSUBDATAPROC)              wglGetProcAddress( "glBufferSubData" );
    glGetBufferSubData              = (PFNGLGETBUFFERSUBDATAPROC)           wglGetProcAddress( "glGetBufferSubData" );
    glDeleteBuffers                 = (PFNGLDELETEBUFFERSPROC)              wglGetProcAddress( "glDeleteBuffers" );

    glBlendEquation                 = (PFNGLBLENDEQUATIONPROC)              wglGetProcAddress( "glBlendEquation" );
    glBlendEquationSeparate         = (PFNGLBLENDEQUATIONSEPARATEPROC)      wglGetProcAddress( "glBlendEquationSeparate" );
    glBlendFuncSeparate             = (PFNGLBLENDFUNCSEPARATEPROC)          wglGetProcAddress( "glBlendFuncSeparate" );
    glBlendColor                    = (PFNGLBLENDCOLORPROC)                 wglGetProcAddress( "glBlendColor" );

    glDeleteFramebuffers            = (PFNGLDELETEFRAMEBUFFERSPROC)         wglGetProcAddress( "glDeleteFramebuffers" );
    if( glDeleteFramebuffers == nullptr )
        glDeleteFramebuffers        = (PFNGLDELETEFRAMEBUFFERSPROC)         wglGetProcAddress( "glDeleteFramebuffersEXT" );

    glGenFramebuffers               = (PFNGLGENFRAMEBUFFERSPROC)            wglGetProcAddress( "glGenFramebuffers" );
    if( glGenFramebuffers == nullptr )
        glGenFramebuffers           = (PFNGLGENFRAMEBUFFERSPROC)            wglGetProcAddress( "glGenFramebuffersEXT" );

    glBindFramebuffer               = (PFNGLBINDFRAMEBUFFERPROC)            wglGetProcAddress( "glBindFramebuffer" );
    if( glBindFramebuffer == nullptr )
        glBindFramebuffer           = (PFNGLBINDFRAMEBUFFERPROC)            wglGetProcAddress( "glBindFramebufferEXT" );

    glFramebufferTexture2D          = (PFNGLFRAMEBUFFERTEXTURE2DPROC)       wglGetProcAddress( "glFramebufferTexture2D" );
    if( glFramebufferTexture2D == nullptr )
        glFramebufferTexture2D      = (PFNGLFRAMEBUFFERTEXTURE2DPROC)       wglGetProcAddress( "glFramebufferTexture2DEXT" );

    glCheckFramebufferStatus        = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)     wglGetProcAddress( "glCheckFramebufferStatus" );
    if( glCheckFramebufferStatus == nullptr )
        glCheckFramebufferStatus    = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)     wglGetProcAddress( "glCheckFramebufferStatusEXT" );

    glDrawBuffers                   = (PFNGLDRAWBUFFERSPROC)                wglGetProcAddress( "glDrawBuffers" );

    glGenRenderbuffers              = (PFNGLGENRENDERBUFFERSPROC)           wglGetProcAddress( "glGenRenderbuffers" );
    if( glGenRenderbuffers == nullptr )
        glGenRenderbuffers          = (PFNGLGENRENDERBUFFERSPROC)           wglGetProcAddress( "glGenRenderbuffersEXT" );

    glBindRenderbuffer              = (PFNGLBINDRENDERBUFFERPROC)           wglGetProcAddress( "glBindRenderbuffer" );
    if( glBindRenderbuffer == nullptr )
        glBindRenderbuffer          = (PFNGLBINDRENDERBUFFERPROC)           wglGetProcAddress( "glBindRenderbufferEXT" );

    glFramebufferRenderbuffer       = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)    wglGetProcAddress( "glFramebufferRenderbuffer" );
    if( glFramebufferRenderbuffer == nullptr )
        glFramebufferRenderbuffer   = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)    wglGetProcAddress( "glFramebufferRenderbufferEXT" );

    glRenderbufferStorage           = (PFNGLRENDERBUFFERSTORAGEPROC)        wglGetProcAddress( "glRenderbufferStorage" );
    if( glRenderbufferStorage == nullptr )
        glRenderbufferStorage       = (PFNGLRENDERBUFFERSTORAGEPROC)        wglGetProcAddress( "glRenderbufferStorageEXT" );

    glDeleteRenderbuffers           = (PFNGLDELETERENDERBUFFERSPROC)        wglGetProcAddress( "glDeleteRenderbuffers" );
    if( glDeleteRenderbuffers == nullptr )
        glDeleteRenderbuffers       = (PFNGLDELETERENDERBUFFERSPROC)        wglGetProcAddress( "glDeleteRenderbuffersEXT" );

    glGenVertexArrays               = (PFNGLGENVERTEXARRAYSPROC)            wglGetProcAddress( "glGenVertexArrays" );
    glDeleteVertexArrays            = (PFNGLDELETEVERTEXARRAYSPROC)         wglGetProcAddress( "glDeleteVertexArrays" );
    glBindVertexArray               = (PFNGLBINDVERTEXARRAYPROC)            wglGetProcAddress( "glBindVertexArray" );

    glGenerateMipmap                = (PFNGLGENERATEMIPMAPPROC)             wglGetProcAddress( "glGenerateMipmap" );

    glDrawArraysInstanced           = (PFNGLDRAWARRAYSINSTANCEDPROC)        wglGetProcAddress( "glDrawArraysInstanced" );
    glDrawElementsInstanced         = (PFNGLDRAWELEMENTSINSTANCEDPROC)      wglGetProcAddress( "glDrawElementsInstanced" );
    glVertexAttribDivisor           = (PFNGLVERTEXATTRIBDIVISORPROC)        wglGetProcAddress( "glVertexAttribDivisor" );
    
    //glVertexBindingDivisor          = (PFNGLVERTEXBINDINGDIVISORPROC)       wglGetProcAddress( "glVertexBindingDivisor" );
}

#pragma warning( pop )
