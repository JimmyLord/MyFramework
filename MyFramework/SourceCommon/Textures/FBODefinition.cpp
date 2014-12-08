//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

FBODefinition::FBODefinition()
{
    m_FullyLoaded = false;
    m_FailedToInit = false;

    m_ColorTextureID = 0;
    m_DepthBufferID = 0;
    m_FrameBufferID = 0;

    m_Width = 0;
    m_Height = 0;

    m_MinFilter = GL_LINEAR;
    m_MagFilter = GL_LINEAR;

    m_NeedColorTexture = true;
    m_NeedDepthTexture = true;
    m_DepthIsTexture = false;
}

FBODefinition::~FBODefinition()
{
    this->Remove();

    Invalidate( true );
}

void FBODefinition::Setup(unsigned int width, unsigned int height, int minfilter, int magfilter, bool needcolor, bool needdepth, bool depthreadable)
{
    m_Width = width;
    m_Height = height;
    m_MinFilter = minfilter;
    m_MagFilter = magfilter;

    m_NeedColorTexture = needcolor;
    m_NeedDepthTexture = needdepth;
    m_DepthIsTexture = depthreadable;
}

bool FBODefinition::Create()
{
#if MYFW_WINDOWS
    if( glGenFramebuffers == 0 )
    {
        return false;
    }
#endif
#if MYFW_IOS || MYFW_ANDROID
    //return false;
#endif

#if !USE_D3D
    GLint maxsize;

    glGetIntegerv( GL_MAX_RENDERBUFFER_SIZE, &maxsize );
    LOGInfo( LOGTag, "CreateFBO - maxsize: %d\n", maxsize );

#if MYFW_ANDROID
    int range[2], precision;
    glGetShaderPrecisionFormat( GL_FRAGMENT_SHADER, GL_HIGH_FLOAT, range, &precision );
    LOGInfo( LOGTag, "CreateFBO - High float precision: %d\n", precision );
    LOGInfo( LOGTag, "CreateFBO - High float range min: %d\n", range[0] );
    LOGInfo( LOGTag, "CreateFBO - High float range max: %d\n", range[1] );
    glGetShaderPrecisionFormat( GL_FRAGMENT_SHADER, GL_MEDIUM_FLOAT, range, &precision );
    LOGInfo( LOGTag, "CreateFBO - Medium float precision: %d\n", precision );
    LOGInfo( LOGTag, "CreateFBO - Medium float range min: %d\n", range[0] );
    LOGInfo( LOGTag, "CreateFBO - Medium float range max: %d\n", range[1] );
    glGetShaderPrecisionFormat( GL_FRAGMENT_SHADER, GL_LOW_FLOAT, range, &precision );
    LOGInfo( LOGTag, "CreateFBO - Low float precision: %d\n", precision );
    LOGInfo( LOGTag, "CreateFBO - Low float range min: %d\n", range[0] );
    LOGInfo( LOGTag, "CreateFBO - Low float range max: %d\n", range[1] );
#endif

    if( m_Width > (unsigned int)maxsize || m_Height > (unsigned int)maxsize )
    {
        // requested size is too big.
        return false;
    }

    // get a framebuffer, render buffer and a texture from opengl.
    glGenFramebuffers( 1, &m_FrameBufferID );
    checkGlError( "glGenFramebuffers" );

    if( m_NeedColorTexture )
    {
        glGenTextures( 1, &m_ColorTextureID );
    }
    checkGlError( "glGenTextures" );

    if( m_NeedDepthTexture )
    {
        if( m_DepthIsTexture )
        {
            glGenTextures( 1, &m_DepthBufferID );
            checkGlError( "glGenTextures" );
        }
        else
        {
            glGenRenderbuffers( 1, &m_DepthBufferID );
            checkGlError( "glGenRenderbuffers" );
        }
    }

    // create the texture
    if( m_ColorTextureID != 0 )
    {
        if( m_ColorTextureID != 0 )
        {
            glBindTexture( GL_TEXTURE_2D, m_ColorTextureID );
            //glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, m_Width, m_Height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL );
            glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_MinFilter );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_MagFilter );
            glBindTexture( GL_TEXTURE_2D, 0 );
            checkGlError( "glBindTexture" );
        }
    }

    // create a depth renderbuffer.
    if( m_DepthBufferID != 0 )
    {
        if( m_DepthIsTexture )
        {
            glBindTexture( GL_TEXTURE_2D, m_DepthBufferID );
            checkGlError( "glBindTexture" );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
            checkGlError( "glTexParameteri" );
            glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, m_Width, m_Height, 0,
                          GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0 );
                          //GL_DEPTH_COMPONENT, GL_FLOAT, 0 );
            checkGlError( "glTexImage2D" );
            glBindTexture( GL_TEXTURE_2D, 0 );
            checkGlError( "glBindTexture" );
        }
        else
        {
            glBindRenderbuffer( GL_RENDERBUFFER, m_DepthBufferID );
            glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_Width, m_Height );
            checkGlError( "glRenderbufferStorageEXT" );
        }
    }

    // attach everything to the FBO
    {
        glBindFramebuffer( GL_FRAMEBUFFER, m_FrameBufferID );

        // attach color texture
        if( m_ColorTextureID != 0 )
            glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorTextureID, 0 );

        // attach depth renderbuffer
        if( m_DepthBufferID != 0 )
        {
            if( m_DepthIsTexture )
            {
                glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthBufferID, 0 );
            }
            else
            {
               glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthBufferID );
            }
        }

        glBindFramebuffer( GL_FRAMEBUFFER, 0 );
        checkGlError( "glBindFramebufferEXT" );
    }

    // any problems?
    GLint status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
    checkGlError( "glCheckFramebufferStatus" );
    if( status != GL_FRAMEBUFFER_COMPLETE )
    {
        LOGInfo( LOGTag, "CreateFBO - error\n" );
        //assert( false );
        Invalidate( true );
        return false;
    }

    LOGInfo( LOGTag, "CreateFBO - complete\n" );
#else
    return false;
#endif

    return true;
}

void FBODefinition::Bind()
{
#if !USE_D3D
    glBindFramebuffer( GL_FRAMEBUFFER, m_FrameBufferID );
    checkGlError( "glBindFramebuffer" );
#endif
}

void FBODefinition::Unbind()
{
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void FBODefinition::Invalidate(bool cleanglallocs)
{
#if !USE_D3D
    if( cleanglallocs )
    {
        if( m_ColorTextureID != 0 )
            glDeleteTextures( 1, &m_ColorTextureID );
        if( m_DepthBufferID != 0 )
        {
            if( m_DepthIsTexture )
                glDeleteTextures( 1, &m_DepthBufferID );
            else
                glDeleteRenderbuffers( 1, &m_DepthBufferID );
        }
        if( m_FrameBufferID != 0 )
            glDeleteFramebuffers( 1, &m_FrameBufferID );
    }

    m_ColorTextureID = 0;
    m_DepthBufferID = 0;
    m_FrameBufferID = 0;
#endif
}
