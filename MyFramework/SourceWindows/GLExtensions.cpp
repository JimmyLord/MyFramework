//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#pragma warning( push )
#pragma warning( disable : 4191 ) // unsafe conversion from 'type of expression' to 'type required'

PFNGLTEXIMAGE3DPROC                 glTexImage3D = 0;
PFNGLFRAMEBUFFERTEXTURELAYERPROC    glFramebufferTextureLayer = 0;

PFNGLUNIFORM1FPROC                  glUniform1f = 0;
PFNGLUNIFORM2FPROC                  glUniform2f = 0;
PFNGLUNIFORM3FPROC                  glUniform3f = 0;
PFNGLUNIFORM4FPROC                  glUniform4f = 0;
PFNGLUNIFORM1IPROC                  glUniform1i = 0;
PFNGLUNIFORM2IPROC                  glUniform2i = 0;
PFNGLUNIFORM3IPROC                  glUniform3i = 0;
PFNGLUNIFORM4IPROC                  glUniform4i = 0;
PFNGLUNIFORM1FVPROC                 glUniform1fv = 0;
PFNGLUNIFORM2FVPROC                 glUniform2fv = 0;
PFNGLUNIFORM3FVPROC                 glUniform3fv = 0;
PFNGLUNIFORM4FVPROC                 glUniform4fv = 0;
PFNGLUNIFORM1IVPROC                 glUniform1iv = 0;
PFNGLUNIFORM2IVPROC                 glUniform2iv = 0;
PFNGLUNIFORM3IVPROC                 glUniform3iv = 0;
PFNGLUNIFORM4IVPROC                 glUniform4iv = 0;

PFNGLUNIFORMMATRIX4FVPROC           glUniformMatrix4fv = 0;
PFNGLVERTEXATTRIB1FPROC             glVertexAttrib1f = 0;
PFNGLVERTEXATTRIB2FPROC             glVertexAttrib2f = 0;
PFNGLVERTEXATTRIB3FPROC             glVertexAttrib3f = 0;
PFNGLVERTEXATTRIB4FPROC             glVertexAttrib4f = 0;
PFNGLVERTEXATTRIB1FVPROC            glVertexAttrib1fv = 0;
PFNGLVERTEXATTRIB2FVPROC            glVertexAttrib2fv = 0;
PFNGLVERTEXATTRIB3FVPROC            glVertexAttrib3fv = 0;
PFNGLVERTEXATTRIB4FVPROC            glVertexAttrib4fv = 0;
PFNGLENABLEVERTEXATTRIBARRAYPROC    glEnableVertexAttribArray = 0;
PFNGLBINDATTRIBLOCATIONPROC         glBindAttribLocation = 0;

PFNGLDISABLEVERTEXATTRIBARRAYPROC   glDisableVertexAttribArray = 0;
PFNGLVERTEXATTRIBPOINTERPROC        glVertexAttribPointer = 0;
PFNGLVERTEXATTRIBIPOINTERPROC       glVertexAttribIPointer = 0;

PFNGLCREATESHADERPROC               glCreateShader = 0;
PFNGLSHADERSOURCEPROC               glShaderSource = 0;
PFNGLCOMPILESHADERPROC              glCompileShader = 0;
PFNGLCREATEPROGRAMPROC              glCreateProgram = 0;
PFNGLATTACHSHADERPROC               glAttachShader = 0;
PFNGLLINKPROGRAMPROC                glLinkProgram = 0;
PFNGLDETACHSHADERPROC               glDetachShader = 0;
PFNGLDELETEPROGRAMPROC              glDeleteProgram = 0;
PFNGLDELETESHADERPROC               glDeleteShader = 0;
PFNGLUSEPROGRAMPROC                 glUseProgram = 0;

PFNGLGETATTRIBLOCATIONPROC          glGetAttribLocation = 0;
PFNGLGETPROGRAMINFOLOGPROC          glGetProgramInfoLog = 0;
PFNGLGETPROGRAMIVPROC               glGetProgramiv = 0;
PFNGLGETSHADERINFOLOGPROC           glGetShaderInfoLog = 0;
PFNGLGETSHADERSOURCEPROC            glGetShaderSource = 0;
PFNGLGETSHADERIVPROC                glGetShaderiv = 0;
PFNGLGETUNIFORMLOCATIONPROC         glGetUniformLocation = 0;

PFNGLACTIVETEXTUREPROC              glActiveTexture = 0;

PFNGLGENBUFFERSPROC                 glGenBuffers = 0;
PFNGLBINDBUFFERPROC                 glBindBuffer = 0;
PFNGLBUFFERDATAPROC                 glBufferData = 0;
PFNGLBUFFERSUBDATAPROC              glBufferSubData = 0;
PFNGLDELETEBUFFERSPROC              glDeleteBuffers = 0;

PFNGLBLENDFUNCSEPARATEPROC          glBlendFuncSeparate = 0;
PFNGLBLENDCOLORPROC                 glBlendColor = 0;

PFNGLDELETEFRAMEBUFFERSPROC         glDeleteFramebuffers = 0;
PFNGLGENFRAMEBUFFERSPROC            glGenFramebuffers = 0;
PFNGLBINDFRAMEBUFFERPROC            glBindFramebuffer = 0;
PFNGLFRAMEBUFFERTEXTURE2DPROC       glFramebufferTexture2D = 0;
PFNGLCHECKFRAMEBUFFERSTATUSPROC     glCheckFramebufferStatus = 0;

PFNGLGENRENDERBUFFERSPROC           glGenRenderbuffers = 0;
PFNGLBINDRENDERBUFFERPROC           glBindRenderbuffer = 0;
PFNGLFRAMEBUFFERRENDERBUFFERPROC    glFramebufferRenderbuffer = 0;
PFNGLRENDERBUFFERSTORAGEPROC        glRenderbufferStorage = 0;
PFNGLDELETERENDERBUFFERSPROC        glDeleteRenderbuffers = 0;

PFNGLGENVERTEXARRAYSPROC            glGenVertexArrays = 0;
PFNGLDELETEVERTEXARRAYSPROC         glDeleteVertexArrays = 0;
PFNGLBINDVERTEXARRAYPROC            glBindVertexArray = 0;

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
    glDeleteBuffers                 = (PFNGLDELETEBUFFERSPROC)              wglGetProcAddress( "glDeleteBuffers" );

    glBlendFuncSeparate             = (PFNGLBLENDFUNCSEPARATEPROC)          wglGetProcAddress( "glBlendFuncSeparate" );
    glBlendColor                    = (PFNGLBLENDCOLORPROC)                 wglGetProcAddress( "glBlendColor" );

    glDeleteFramebuffers            = (PFNGLDELETEFRAMEBUFFERSPROC)         wglGetProcAddress( "glDeleteFramebuffers" );
    if( glDeleteFramebuffers == 0 )
        glDeleteFramebuffers        = (PFNGLDELETEFRAMEBUFFERSPROC)         wglGetProcAddress( "glDeleteFramebuffersEXT" );

    glGenFramebuffers               = (PFNGLGENFRAMEBUFFERSPROC)            wglGetProcAddress( "glGenFramebuffers" );
    if( glGenFramebuffers == 0 )
        glGenFramebuffers           = (PFNGLGENFRAMEBUFFERSPROC)            wglGetProcAddress( "glGenFramebuffersEXT" );

    glBindFramebuffer               = (PFNGLBINDFRAMEBUFFERPROC)            wglGetProcAddress( "glBindFramebuffer" );
    if( glBindFramebuffer == 0 )
        glBindFramebuffer           = (PFNGLBINDFRAMEBUFFERPROC)            wglGetProcAddress( "glBindFramebufferEXT" );

    glFramebufferTexture2D          = (PFNGLFRAMEBUFFERTEXTURE2DPROC)       wglGetProcAddress( "glFramebufferTexture2D" );
    if( glFramebufferTexture2D == 0 )
        glFramebufferTexture2D      = (PFNGLFRAMEBUFFERTEXTURE2DPROC)       wglGetProcAddress( "glFramebufferTexture2DEXT" );

    glCheckFramebufferStatus        = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)     wglGetProcAddress( "glCheckFramebufferStatus" );
    if( glCheckFramebufferStatus == 0 )
        glCheckFramebufferStatus    = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)     wglGetProcAddress( "glCheckFramebufferStatusEXT" );

    glGenRenderbuffers              = (PFNGLGENRENDERBUFFERSPROC)           wglGetProcAddress( "glGenRenderbuffers" );
    if( glGenRenderbuffers == 0 )
        glGenRenderbuffers          = (PFNGLGENRENDERBUFFERSPROC)           wglGetProcAddress( "glGenRenderbuffersEXT" );

    glBindRenderbuffer              = (PFNGLBINDRENDERBUFFERPROC)           wglGetProcAddress( "glBindRenderbuffer" );
    if( glBindRenderbuffer == 0 )
        glBindRenderbuffer          = (PFNGLBINDRENDERBUFFERPROC)           wglGetProcAddress( "glBindRenderbufferEXT" );

    glFramebufferRenderbuffer       = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)    wglGetProcAddress( "glFramebufferRenderbuffer" );
    if( glFramebufferRenderbuffer == 0 )
        glFramebufferRenderbuffer   = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)    wglGetProcAddress( "glFramebufferRenderbufferEXT" );

    glRenderbufferStorage           = (PFNGLRENDERBUFFERSTORAGEPROC)        wglGetProcAddress( "glRenderbufferStorage" );
    if( glRenderbufferStorage == 0 )
        glRenderbufferStorage       = (PFNGLRENDERBUFFERSTORAGEPROC)        wglGetProcAddress( "glRenderbufferStorageEXT" );

    glDeleteRenderbuffers           = (PFNGLDELETERENDERBUFFERSPROC)        wglGetProcAddress( "glDeleteRenderbuffers" );
    if( glDeleteRenderbuffers == 0 )
        glDeleteRenderbuffers       = (PFNGLDELETERENDERBUFFERSPROC)        wglGetProcAddress( "glDeleteRenderbuffersEXT" );

    glGenVertexArrays               = (PFNGLGENVERTEXARRAYSPROC)            wglGetProcAddress( "glGenVertexArrays" );
    glDeleteVertexArrays            = (PFNGLDELETEVERTEXARRAYSPROC)         wglGetProcAddress( "glDeleteVertexArrays" );
    glBindVertexArray               = (PFNGLBINDVERTEXARRAYPROC)            wglGetProcAddress( "glBindVertexArray" );
}

#pragma warning( pop )
