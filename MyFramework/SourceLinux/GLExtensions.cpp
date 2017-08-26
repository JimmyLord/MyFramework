//
// Copyright (c) 2012-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include <GL/glx.h>

#pragma warning( push )
#pragma warning( disable : 4191 ) // unsafe conversion from 'type of expression' to 'type required'

//PFNGLTEXIMAGE3DPROC                 glTexImage3D = 0;
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

PFNGLUNIFORMMATRIX3FVPROC           glUniformMatrix3fv = 0;
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

//PFNGLACTIVETEXTUREPROC              glActiveTexture = 0;

PFNGLGENBUFFERSPROC                 glGenBuffers = 0;
PFNGLBINDBUFFERPROC                 glBindBuffer = 0;
PFNGLBUFFERDATAPROC                 glBufferData = 0;
PFNGLBUFFERSUBDATAPROC              glBufferSubData = 0;
PFNGLDELETEBUFFERSPROC              glDeleteBuffers = 0;

PFNGLBLENDFUNCSEPARATEPROC          glBlendFuncSeparate = 0;
//PFNGLBLENDCOLORPROC                 glBlendColor = 0;

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

PFNGLGENERATEMIPMAPPROC             glGenerateMipmap = 0;

PFNGLDRAWARRAYSINSTANCEDPROC        glDrawArraysInstanced = 0;      //(GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
PFNGLDRAWELEMENTSINSTANCEDPROC      glDrawElementsInstanced = 0;    //(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount);
PFNGLVERTEXATTRIBDIVISORPROC        glVertexAttribDivisor = 0;      //(GLuint index, GLuint divisor);

//PFNGLVERTEXBINDINGDIVISORPROC       glVertexBindingDivisor = 0;     //(GLuint bindingindex, GLuint divisor);

void OpenGL_InitExtensions()
{
    //glTexImage3D                    = (PFNGLTEXIMAGE3DPROC)                 glXGetProcAddress( (const GLubyte*)"glTexImage3D" );
    glFramebufferTextureLayer       = (PFNGLFRAMEBUFFERTEXTURELAYERPROC)    glXGetProcAddressARB( (const GLubyte*)"glFramebufferTextureLayer" );

    glUniform1i                     = (PFNGLUNIFORM1IPROC)                  glXGetProcAddress( (const GLubyte*)"glUniform1i" );
    glUniform2i                     = (PFNGLUNIFORM2IPROC)                  glXGetProcAddress( (const GLubyte*)"glUniform2i" );
    glUniform3i                     = (PFNGLUNIFORM3IPROC)                  glXGetProcAddress( (const GLubyte*)"glUniform3i" );
    glUniform4i                     = (PFNGLUNIFORM4IPROC)                  glXGetProcAddress( (const GLubyte*)"glUniform4i" );
    glUniform1iv                    = (PFNGLUNIFORM1IVPROC)                 glXGetProcAddress( (const GLubyte*)"glUniform1iv" );
    glUniform2iv                    = (PFNGLUNIFORM2IVPROC)                 glXGetProcAddress( (const GLubyte*)"glUniform2iv" );
    glUniform3iv                    = (PFNGLUNIFORM3IVPROC)                 glXGetProcAddress( (const GLubyte*)"glUniform3iv" );
    glUniform4iv                    = (PFNGLUNIFORM4IVPROC)                 glXGetProcAddress( (const GLubyte*)"glUniform4iv" );
    glUniform1f                     = (PFNGLUNIFORM1FPROC)                  glXGetProcAddress( (const GLubyte*)"glUniform1f" );
    glUniform2f                     = (PFNGLUNIFORM2FPROC)                  glXGetProcAddress( (const GLubyte*)"glUniform2f" );
    glUniform3f                     = (PFNGLUNIFORM3FPROC)                  glXGetProcAddress( (const GLubyte*)"glUniform3f" );
    glUniform4f                     = (PFNGLUNIFORM4FPROC)                  glXGetProcAddress( (const GLubyte*)"glUniform4f" );
    glUniform1fv                    = (PFNGLUNIFORM1FVPROC)                 glXGetProcAddress( (const GLubyte*)"glUniform1fv" );
    glUniform2fv                    = (PFNGLUNIFORM2FVPROC)                 glXGetProcAddress( (const GLubyte*)"glUniform2fv" );
    glUniform3fv                    = (PFNGLUNIFORM3FVPROC)                 glXGetProcAddress( (const GLubyte*)"glUniform3fv" );
    glUniform4fv                    = (PFNGLUNIFORM4FVPROC)                 glXGetProcAddress( (const GLubyte*)"glUniform4fv" );
    glUniformMatrix3fv              = (PFNGLUNIFORMMATRIX3FVPROC)           glXGetProcAddress( (const GLubyte*)"glUniformMatrix3fv" );
    glUniformMatrix4fv              = (PFNGLUNIFORMMATRIX4FVPROC)           glXGetProcAddress( (const GLubyte*)"glUniformMatrix4fv" );
    glVertexAttrib1f                = (PFNGLVERTEXATTRIB1FPROC)             glXGetProcAddress( (const GLubyte*)"glVertexAttrib1f" );
    glVertexAttrib2f                = (PFNGLVERTEXATTRIB2FPROC)             glXGetProcAddress( (const GLubyte*)"glVertexAttrib2f" );
    glVertexAttrib3f                = (PFNGLVERTEXATTRIB3FPROC)             glXGetProcAddress( (const GLubyte*)"glVertexAttrib3f" );
    glVertexAttrib4f                = (PFNGLVERTEXATTRIB4FPROC)             glXGetProcAddress( (const GLubyte*)"glVertexAttrib4f" );
    glVertexAttrib1fv               = (PFNGLVERTEXATTRIB1FVPROC)            glXGetProcAddress( (const GLubyte*)"glVertexAttrib1fv" );
    glVertexAttrib2fv               = (PFNGLVERTEXATTRIB2FVPROC)            glXGetProcAddress( (const GLubyte*)"glVertexAttrib2fv" );
    glVertexAttrib3fv               = (PFNGLVERTEXATTRIB3FVPROC)            glXGetProcAddress( (const GLubyte*)"glVertexAttrib3fv" );
    glVertexAttrib4fv               = (PFNGLVERTEXATTRIB4FVPROC)            glXGetProcAddress( (const GLubyte*)"glVertexAttrib4fv" );
    glEnableVertexAttribArray       = (PFNGLENABLEVERTEXATTRIBARRAYPROC)    glXGetProcAddress( (const GLubyte*)"glEnableVertexAttribArray" );
    glBindAttribLocation            = (PFNGLBINDATTRIBLOCATIONPROC)         glXGetProcAddress( (const GLubyte*)"glBindAttribLocation" );

    glDisableVertexAttribArray      = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)   glXGetProcAddress( (const GLubyte*)"glDisableVertexAttribArray" );
    glVertexAttribPointer           = (PFNGLVERTEXATTRIBPOINTERPROC)        glXGetProcAddress( (const GLubyte*)"glVertexAttribPointer" );
    glVertexAttribIPointer          = (PFNGLVERTEXATTRIBIPOINTERPROC)       glXGetProcAddress( (const GLubyte*)"glVertexAttribIPointer" );

    glCreateShader                  = (PFNGLCREATESHADERPROC)               glXGetProcAddress( (const GLubyte*)"glCreateShader" );
    glShaderSource                  = (PFNGLSHADERSOURCEPROC)               glXGetProcAddress( (const GLubyte*)"glShaderSource" );
    glCompileShader                 = (PFNGLCOMPILESHADERPROC)              glXGetProcAddress( (const GLubyte*)"glCompileShader" );
    glCreateProgram                 = (PFNGLCREATEPROGRAMPROC)              glXGetProcAddress( (const GLubyte*)"glCreateProgram" );
    glAttachShader                  = (PFNGLATTACHSHADERPROC)               glXGetProcAddress( (const GLubyte*)"glAttachShader" );
    glLinkProgram                   = (PFNGLLINKPROGRAMPROC)                glXGetProcAddress( (const GLubyte*)"glLinkProgram" );
    glDetachShader                  = (PFNGLDETACHSHADERPROC)               glXGetProcAddress( (const GLubyte*)"glDetachShader" );
    glDeleteShader                  = (PFNGLDELETESHADERPROC)               glXGetProcAddress( (const GLubyte*)"glDeleteShader" );
    glDeleteProgram                 = (PFNGLDELETEPROGRAMPROC)              glXGetProcAddress( (const GLubyte*)"glDeleteProgram" );
    glUseProgram                    = (PFNGLUSEPROGRAMPROC)                 glXGetProcAddress( (const GLubyte*)"glUseProgram" );

    glGetAttribLocation             = (PFNGLGETATTRIBLOCATIONPROC)          glXGetProcAddress( (const GLubyte*)"glGetAttribLocation" );
    glGetProgramInfoLog             = (PFNGLGETPROGRAMINFOLOGPROC)          glXGetProcAddress( (const GLubyte*)"glGetProgramInfoLog" );
    glGetProgramiv                  = (PFNGLGETPROGRAMIVPROC)               glXGetProcAddress( (const GLubyte*)"glGetProgramiv" );
    glGetShaderInfoLog              = (PFNGLGETSHADERINFOLOGPROC)           glXGetProcAddress( (const GLubyte*)"glGetShaderInfoLog" );
    glGetShaderSource               = (PFNGLGETSHADERSOURCEPROC)            glXGetProcAddress( (const GLubyte*)"glGetShaderSource" );
    glGetShaderiv                   = (PFNGLGETSHADERIVPROC)                glXGetProcAddress( (const GLubyte*)"glGetShaderiv" );
    glGetUniformLocation            = (PFNGLGETUNIFORMLOCATIONPROC)         glXGetProcAddress( (const GLubyte*)"glGetUniformLocation" );

    //glActiveTexture                 = (PFNGLACTIVETEXTUREPROC)              glXGetProcAddress( (const GLubyte*)"glActiveTexture" );

    glGenBuffers                    = (PFNGLGENBUFFERSPROC)                 glXGetProcAddress( (const GLubyte*)"glGenBuffers" );
    glBindBuffer                    = (PFNGLBINDBUFFERPROC)                 glXGetProcAddress( (const GLubyte*)"glBindBuffer" );
    glBufferData                    = (PFNGLBUFFERDATAPROC)                 glXGetProcAddress( (const GLubyte*)"glBufferData" );
    glBufferSubData                 = (PFNGLBUFFERSUBDATAPROC)              glXGetProcAddress( (const GLubyte*)"glBufferSubData" );
    glDeleteBuffers                 = (PFNGLDELETEBUFFERSPROC)              glXGetProcAddress( (const GLubyte*)"glDeleteBuffers" );

    glBlendFuncSeparate             = (PFNGLBLENDFUNCSEPARATEPROC)          glXGetProcAddress( (const GLubyte*)"glBlendFuncSeparate" );
    //glBlendColor                    = (PFNGLBLENDCOLORPROC)                 glXGetProcAddress( (const GLubyte*)"glBlendColor" );

    glDeleteFramebuffers            = (PFNGLDELETEFRAMEBUFFERSPROC)         glXGetProcAddress( (const GLubyte*)"glDeleteFramebuffers" );
    if( glDeleteFramebuffers == 0 )
        glDeleteFramebuffers        = (PFNGLDELETEFRAMEBUFFERSPROC)         glXGetProcAddress( (const GLubyte*)"glDeleteFramebuffersEXT" );

    glGenFramebuffers               = (PFNGLGENFRAMEBUFFERSPROC)            glXGetProcAddress( (const GLubyte*)"glGenFramebuffers" );
    if( glGenFramebuffers == 0 )
        glGenFramebuffers           = (PFNGLGENFRAMEBUFFERSPROC)            glXGetProcAddress( (const GLubyte*)"glGenFramebuffersEXT" );

    glBindFramebuffer               = (PFNGLBINDFRAMEBUFFERPROC)            glXGetProcAddress( (const GLubyte*)"glBindFramebuffer" );
    if( glBindFramebuffer == 0 )
        glBindFramebuffer           = (PFNGLBINDFRAMEBUFFERPROC)            glXGetProcAddress( (const GLubyte*)"glBindFramebufferEXT" );

    glFramebufferTexture2D          = (PFNGLFRAMEBUFFERTEXTURE2DPROC)       glXGetProcAddress( (const GLubyte*)"glFramebufferTexture2D" );
    if( glFramebufferTexture2D == 0 )
        glFramebufferTexture2D      = (PFNGLFRAMEBUFFERTEXTURE2DPROC)       glXGetProcAddress( (const GLubyte*)"glFramebufferTexture2DEXT" );

    glCheckFramebufferStatus        = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)     glXGetProcAddress( (const GLubyte*)"glCheckFramebufferStatus" );
    if( glCheckFramebufferStatus == 0 )
        glCheckFramebufferStatus    = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)     glXGetProcAddress( (const GLubyte*)"glCheckFramebufferStatusEXT" );

    glGenRenderbuffers              = (PFNGLGENRENDERBUFFERSPROC)           glXGetProcAddress( (const GLubyte*)"glGenRenderbuffers" );
    if( glGenRenderbuffers == 0 )
        glGenRenderbuffers          = (PFNGLGENRENDERBUFFERSPROC)           glXGetProcAddress( (const GLubyte*)"glGenRenderbuffersEXT" );

    glBindRenderbuffer              = (PFNGLBINDRENDERBUFFERPROC)           glXGetProcAddress( (const GLubyte*)"glBindRenderbuffer" );
    if( glBindRenderbuffer == 0 )
        glBindRenderbuffer          = (PFNGLBINDRENDERBUFFERPROC)           glXGetProcAddress( (const GLubyte*)"glBindRenderbufferEXT" );

    glFramebufferRenderbuffer       = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)    glXGetProcAddress( (const GLubyte*)"glFramebufferRenderbuffer" );
    if( glFramebufferRenderbuffer == 0 )
        glFramebufferRenderbuffer   = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)    glXGetProcAddress( (const GLubyte*)"glFramebufferRenderbufferEXT" );

    glRenderbufferStorage           = (PFNGLRENDERBUFFERSTORAGEPROC)        glXGetProcAddress( (const GLubyte*)"glRenderbufferStorage" );
    if( glRenderbufferStorage == 0 )
        glRenderbufferStorage       = (PFNGLRENDERBUFFERSTORAGEPROC)        glXGetProcAddress( (const GLubyte*)"glRenderbufferStorageEXT" );

    glDeleteRenderbuffers           = (PFNGLDELETERENDERBUFFERSPROC)        glXGetProcAddress( (const GLubyte*)"glDeleteRenderbuffers" );
    if( glDeleteRenderbuffers == 0 )
        glDeleteRenderbuffers       = (PFNGLDELETERENDERBUFFERSPROC)        glXGetProcAddress( (const GLubyte*)"glDeleteRenderbuffersEXT" );

    glGenVertexArrays               = (PFNGLGENVERTEXARRAYSPROC)            glXGetProcAddress( (const GLubyte*)"glGenVertexArrays" );
    glDeleteVertexArrays            = (PFNGLDELETEVERTEXARRAYSPROC)         glXGetProcAddress( (const GLubyte*)"glDeleteVertexArrays" );
    glBindVertexArray               = (PFNGLBINDVERTEXARRAYPROC)            glXGetProcAddress( (const GLubyte*)"glBindVertexArray" );

    glGenerateMipmap                = (PFNGLGENERATEMIPMAPPROC)             glXGetProcAddress( (const GLubyte*)"glGenerateMipmap" );

    glDrawArraysInstanced           = (PFNGLDRAWARRAYSINSTANCEDPROC)        glXGetProcAddress( (const GLubyte*)"glDrawArraysInstanced" );
    glDrawElementsInstanced         = (PFNGLDRAWELEMENTSINSTANCEDPROC)      glXGetProcAddress( (const GLubyte*)"glDrawElementsInstanced" );
    glVertexAttribDivisor           = (PFNGLVERTEXATTRIBDIVISORPROC)        glXGetProcAddress( (const GLubyte*)"glVertexAttribDivisor" );
    
    //glVertexBindingDivisor          = (PFNGLVERTEXBINDINGDIVISORPROC)       glXGetProcAddress( (const GLubyte*)"glVertexBindingDivisor" );
}

#pragma warning( pop )
