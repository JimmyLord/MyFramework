//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

FBODefinition::FBODefinition()
{
    m_FullyLoaded = false;
    m_FailedToInit = false;
    m_OnlyFreeOnShutdown = false;

    m_pColorTexture = 0;
    m_pDepthTexture = 0;
    m_FrameBufferID = 0;

    m_Width = 0;
    m_Height = 0;

    m_TextureWidth = 0;
    m_TextureHeight = 0;

    m_MinFilter = GL_LINEAR;
    m_MagFilter = GL_LINEAR;

    m_NeedColorTexture = true;
    m_DepthBits = 32;
    m_DepthIsTexture = false;
}

FBODefinition::~FBODefinition()
{
    this->Remove();

    Invalidate( true );
}

// returns true if a new texture needs to be created.
bool FBODefinition::Setup(unsigned int width, unsigned int height, int minfilter, int magfilter, bool needcolor, int depthbits, bool depthreadable)
{
    assert( width <= 4096 );
    assert( height <= 4096 );

    unsigned int NewTextureWidth = 0;
    unsigned int NewTextureHeight = 0;

    // loop from 64 to 4096 and find appropriate size.
    for( unsigned int pow=6; pow<12; pow++ )
    {
        unsigned int powsize = (unsigned int)(1 << pow);

        if( powsize >= width && NewTextureWidth == 0 )
            NewTextureWidth = powsize;
        if( powsize >= height && NewTextureHeight == 0 )
            NewTextureHeight = powsize;
    }

    bool newtextureneeded = false;
    bool newfilteroptions = false;

    if( m_TextureWidth != NewTextureWidth || m_TextureHeight != NewTextureHeight )
        newtextureneeded = true;

    if( m_NeedColorTexture != needcolor || m_DepthBits != depthbits || m_DepthIsTexture != depthreadable )
        newtextureneeded = true;

    if( newtextureneeded == false && (m_MinFilter != minfilter || m_MagFilter != magfilter) )
        newfilteroptions = true;

    m_TextureWidth = NewTextureWidth;
    m_TextureHeight = NewTextureHeight;

    m_Width = width;
    m_Height = height;
    m_MinFilter = minfilter;
    m_MagFilter = magfilter;

    m_NeedColorTexture = needcolor;
    m_DepthBits = depthbits;
    m_DepthIsTexture = depthreadable;

    // if filter options changed, reset them on the texture
    if( newfilteroptions == true )
    {
        glBindTexture( GL_TEXTURE_2D, m_pColorTexture->m_TextureID );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_MinFilter );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_MagFilter );
        glBindTexture( GL_TEXTURE_2D, 0 );
    }

    return newtextureneeded;
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

    if( m_TextureWidth > (unsigned int)maxsize || m_TextureHeight > (unsigned int)maxsize )
    {
        // requested size is too big.
        return false;
    }

    assert( m_FrameBufferID == 0 );

    // get a framebuffer, render buffer and a texture from opengl.
    glGenFramebuffers( 1, &m_FrameBufferID );
    checkGlError( "glGenFramebuffers" );

    if( m_NeedColorTexture )
    {
        m_pColorTexture = MyNew TextureDefinition();
        glGenTextures( 1, &m_pColorTexture->m_TextureID );
        m_pColorTexture->m_MinFilter = m_MinFilter;
        m_pColorTexture->m_MagFilter = m_MagFilter;
        m_pColorTexture->m_WrapS = GL_CLAMP_TO_EDGE;
        m_pColorTexture->m_WrapT = GL_CLAMP_TO_EDGE;
        m_pColorTexture->m_Width = m_Width;
        m_pColorTexture->m_Height = m_Height;
    }
    checkGlError( "glGenTextures" );

    if( m_DepthBits != 0 )
    {
        m_pDepthTexture = MyNew TextureDefinition();

        assert( m_DepthBits == 16 || m_DepthBits == 24 || m_DepthBits == 32 );

        if( m_DepthIsTexture )
        {
            glGenTextures( 1, &m_pDepthTexture->m_TextureID );
            checkGlError( "glGenTextures" );
        }
        else
        {
            glGenRenderbuffers( 1, &m_pDepthTexture->m_TextureID );
            checkGlError( "glGenRenderbuffers" );
        }

        m_pDepthTexture->m_Width = m_Width;
        m_pDepthTexture->m_Height = m_Height;
    }

    // create the texture
    if( m_pColorTexture && m_pColorTexture->m_TextureID != 0 )
    {
        glBindTexture( GL_TEXTURE_2D, m_pColorTexture->m_TextureID );
        //glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, m_TextureWidth, m_TextureHeight, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, m_TextureWidth, m_TextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_MinFilter );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_MagFilter );
        glBindTexture( GL_TEXTURE_2D, 0 );
        checkGlError( "glBindTexture" );
    }

    // create a depth renderbuffer.
    if( m_pDepthTexture && m_pDepthTexture->m_TextureID != 0 )
    {
#if !MYFW_OPENGLES2
        GLint depthformat = GL_DEPTH_COMPONENT32;
        if( m_DepthBits == 24 )
            depthformat = GL_DEPTH_COMPONENT24;
        else if( m_DepthBits == 16 )
            depthformat = GL_DEPTH_COMPONENT16;
#else
        GLint depthformat = GL_DEPTH_COMPONENT16;
#endif

        if( m_DepthIsTexture )
        {
            glBindTexture( GL_TEXTURE_2D, m_pDepthTexture->m_TextureID );
            checkGlError( "glBindTexture" );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
            checkGlError( "glTexParameteri" );
            glTexImage2D( GL_TEXTURE_2D, 0, depthformat, m_TextureWidth, m_TextureHeight, 0,
                          GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0 );
                          //GL_DEPTH_COMPONENT, GL_FLOAT, 0 );
            checkGlError( "glTexImage2D" );
            glBindTexture( GL_TEXTURE_2D, 0 );
            checkGlError( "glBindTexture" );
        }
        else
        {
            glBindRenderbuffer( GL_RENDERBUFFER, m_pDepthTexture->m_TextureID );
            glRenderbufferStorage( GL_RENDERBUFFER, depthformat, m_TextureWidth, m_TextureHeight );
            checkGlError( "glRenderbufferStorageEXT" );
        }
    }

    // attach everything to the FBO
    {
        glBindFramebuffer( GL_FRAMEBUFFER, m_FrameBufferID );

        // attach color texture
        if( m_pColorTexture && m_pColorTexture->m_TextureID != 0 )
            glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_pColorTexture->m_TextureID, 0 );

        // attach depth renderbuffer
        if( m_pDepthTexture && m_pDepthTexture->m_TextureID != 0 )
        {
            if( m_DepthIsTexture )
            {
                glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_pDepthTexture->m_TextureID, 0 );
            }
            else
            {
               glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_pDepthTexture->m_TextureID );
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

    LOGInfo( LOGTag, "CreateFBO - complete (%d, %d)\n", m_TextureWidth, m_TextureHeight );
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
        if( m_pColorTexture && m_pColorTexture->m_TextureID != 0 )
            glDeleteTextures( 1, &m_pColorTexture->m_TextureID );
        if( m_pDepthTexture && m_pDepthTexture->m_TextureID != 0 )
        {
            if( m_DepthIsTexture )
                glDeleteTextures( 1, &m_pDepthTexture->m_TextureID );
            else
                glDeleteRenderbuffers( 1, &m_pDepthTexture->m_TextureID );
        }
        if( m_FrameBufferID != 0 )
            glDeleteFramebuffers( 1, &m_FrameBufferID );

        SAFE_RELEASE( m_pColorTexture );
        SAFE_RELEASE( m_pDepthTexture );
    }

    m_pColorTexture = 0;
    m_pDepthTexture = 0;
    m_FrameBufferID = 0;
#endif
}
