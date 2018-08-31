//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

FBODefinition::FBODefinition()
{
    m_HasValidResources = false;
    m_FullyLoaded = false;
    m_FailedToInit = false;
    m_OnlyFreeOnShutdown = false;

    m_NumColorTextures = 0;
    for( int i=0; i<MAX_COLOR_TEXTURES; i++ )
    {
        m_pColorTextures[i] = 0;
    }
    m_pDepthTexture = 0;
    m_FrameBufferID = 0;

    m_Width = 0;
    m_Height = 0;

    m_TextureWidth = 0;
    m_TextureHeight = 0;

    m_MinFilter = GL_LINEAR;
    m_MagFilter = GL_LINEAR;

    for( int i=0; i<MAX_COLOR_TEXTURES; i++ )
        m_ColorFormats[i] = FBOColorFormat_None;
    m_DepthBits = 32;
    m_DepthIsTexture = false;

    m_LastFrameBufferID = -1;
}

FBODefinition::~FBODefinition()
{
    this->Remove();

    Invalidate( true );
}

// Returns true if a new textures need to be created.
bool FBODefinition::Setup(unsigned int width, unsigned int height, int minFilter, int magFilter, FBOColorFormat colorFormat, int depthBits, bool depthReadable)
{
    return Setup( width, height, minFilter, magFilter, &colorFormat, 1, depthBits, depthReadable );
}

bool FBODefinition::Setup(unsigned int width, unsigned int height, int minFilter, int magFilter, FBOColorFormat* colorFormats, int numColorFormats, int depthBits, bool depthReadable)
{
    MyAssert( numColorFormats < MAX_COLOR_TEXTURES );
    MyAssert( width <= 4096 );
    MyAssert( height <= 4096 );

    unsigned int NewTextureWidth = 0;
    unsigned int NewTextureHeight = 0;

    // Loop from 64 to 4096 and find appropriate size.
    for( unsigned int pow=6; pow<12; pow++ )
    {
        unsigned int powsize = (unsigned int)(1 << pow);

        if( powsize >= width && NewTextureWidth == 0 )
            NewTextureWidth = powsize;
        if( powsize >= height && NewTextureHeight == 0 )
            NewTextureHeight = powsize;
    }

    // Don't allow FBO textures to shrink, just use a smaller piece of them.
    if( NewTextureWidth < m_TextureWidth )
        NewTextureWidth = m_TextureWidth;
    if( NewTextureHeight < m_TextureHeight )
        NewTextureHeight = m_TextureHeight;

    bool newtextureneeded = false;
    bool newfilteroptions = false;

    if( m_TextureWidth != NewTextureWidth || m_TextureHeight != NewTextureHeight )
        newtextureneeded = true;

    m_NumColorTextures = numColorFormats;
    for( int i=0; i<numColorFormats; i++ )
    {
        if( m_ColorFormats[i] != colorFormats[i] )
            newtextureneeded = true;
    }
    
    if( m_DepthBits != depthBits || m_DepthIsTexture != depthReadable )
        newtextureneeded = true;

    if( newtextureneeded == false && (m_MinFilter != minFilter || m_MagFilter != magFilter) )
        newfilteroptions = true;

    m_TextureWidth = NewTextureWidth;
    m_TextureHeight = NewTextureHeight;

    m_Width = width;
    m_Height = height;
    m_MinFilter = minFilter;
    m_MagFilter = magFilter;

    for( int i=0; i<MAX_COLOR_TEXTURES; i++ )
    {
        if( m_pColorTextures[i] )
        {
            m_pColorTextures[i]->m_Width = m_TextureWidth;
            m_pColorTextures[i]->m_Height = m_TextureHeight;
        }
    }
    if( m_pDepthTexture )
    {
        m_pDepthTexture->m_Width = m_TextureWidth;
        m_pDepthTexture->m_Height = m_TextureHeight;
    }

    for( int i=0; i<MAX_COLOR_TEXTURES; i++ )
    {
        m_ColorFormats[i] = FBOColorFormat_None;
    }

    for( int i=0; i<numColorFormats; i++ )
    {
        m_ColorFormats[i] = colorFormats[i];
    }
    m_DepthBits = depthBits;
    m_DepthIsTexture = depthReadable;

    // If filter options changed, reset them on the texture
    if( newfilteroptions == true )
    {
        for( int i=0; i<MAX_COLOR_TEXTURES; i++ )
        {
            glBindTexture( GL_TEXTURE_2D, m_pColorTextures[i]->m_TextureID );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_MinFilter );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_MagFilter );
            glBindTexture( GL_TEXTURE_2D, 0 );
        }
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
    //LOGInfo( LOGTag, "CreateFBO - maxsize: %d\n", maxsize );

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

    MyAssert( m_FrameBufferID == 0 );

    // get a framebuffer, render buffer and a texture from opengl.
    glGenFramebuffers( 1, &m_FrameBufferID );
    m_HasValidResources = true;
    checkGlError( "glGenFramebuffers" );

    // Create the color textures if they don't already exist.
    for( int i=0; i<MAX_COLOR_TEXTURES; i++ )
    {
        if( m_ColorFormats[i] != FBOColorFormat_None )
        {
            MyAssert( m_pColorTextures[i] == 0 );
            m_pColorTextures[i] = MyNew TextureDefinition();

            glGenTextures( 1, &m_pColorTextures[i]->m_TextureID );
            m_HasValidResources = true;

            m_pColorTextures[i]->m_MinFilter = m_MinFilter;
            m_pColorTextures[i]->m_MagFilter = m_MagFilter;
            m_pColorTextures[i]->m_WrapS = GL_CLAMP_TO_EDGE;
            m_pColorTextures[i]->m_WrapT = GL_CLAMP_TO_EDGE;
            m_pColorTextures[i]->m_Width = m_Width;
            m_pColorTextures[i]->m_Height = m_Height;
        }
        else
        {
            // FBO had more color textures then was later resetup with less.
            // TODO: handle this case:
            //   - delete old TextureDefinitions
            MyAssert( m_pColorTextures[i] == 0 );
        }
    }
    checkGlError( "glGenTextures" );

    if( m_DepthBits != 0 )
    {
        MyAssert( m_pDepthTexture == 0 );
        m_pDepthTexture = MyNew TextureDefinition();

        MyAssert( m_DepthBits == 16 || m_DepthBits == 24 || m_DepthBits == 32 );

        if( m_DepthIsTexture )
        {
            glGenTextures( 1, &m_pDepthTexture->m_TextureID );
            m_HasValidResources = true;
            checkGlError( "glGenTextures" );
        }
        else
        {
            glGenRenderbuffers( 1, &m_pDepthTexture->m_TextureID );
            m_HasValidResources = true;
            checkGlError( "glGenRenderbuffers" );
        }

        m_pDepthTexture->m_Width = m_Width;
        m_pDepthTexture->m_Height = m_Height;
    }

    // Create the color textures.
    for( int i=0; i<MAX_COLOR_TEXTURES; i++ )
    {
        if( m_pColorTextures[i] && m_pColorTextures[i]->m_TextureID != 0 )
        {
            MyAssert( m_ColorFormats[i] != FBOColorFormat_None );

            GLint internalformat = 0;
            GLenum format = 0;
            GLenum type = 0;

            switch( m_ColorFormats[i] )
            {
            case FBOColorFormat_RGBA_UByte:
                internalformat = GL_RGBA;
                format = GL_RGBA;
                type = GL_UNSIGNED_BYTE;
                break;

#if !MYFW_OPENGLES2
            case FBOColorFormat_RGBA_Float16:
                internalformat = GL_RGBA16F;
                format = GL_RGBA;
                type = GL_FLOAT;
                break;

            case FBOColorFormat_RGB_Float16:
                internalformat = GL_RGB16F;
                format = GL_RGB;
                type = GL_FLOAT;
                break;
#endif

            case FBOColorFormat_None:
                assert( false );
            }

            MyAssert( format != 0 );

            glBindTexture( GL_TEXTURE_2D, m_pColorTextures[i]->m_TextureID );
            glTexImage2D( GL_TEXTURE_2D, 0, internalformat, m_TextureWidth, m_TextureHeight, 0, format, type, 0 );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_MinFilter );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_MagFilter );
            glBindTexture( GL_TEXTURE_2D, 0 );
            checkGlError( "glBindTexture" );
        }
    }

    // Create a depth renderbuffer.
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

    // Attach everything to the FBO.
    {
        MyBindFramebuffer( GL_FRAMEBUFFER, m_FrameBufferID, 0, 0 );

        // Attach color texture.
        for( unsigned int i=0; i<m_NumColorTextures; i++ )
        {
            if( m_pColorTextures[i] && m_pColorTextures[i]->m_TextureID != 0 )
            {
                //LOGInfo( LOGTag, "FBO: Attaching color texture %d\n", m_pColorTextures[i]->m_TextureID );
                glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_pColorTextures[i]->m_TextureID, 0 );
            }
        }

        // Attach depth renderbuffer.
        if( m_pDepthTexture && m_pDepthTexture->m_TextureID != 0 )
        {
            if( m_DepthIsTexture )
            {
                //LOGInfo( LOGTag, "FBO: Attaching depth texture %d\n", m_pDepthTexture->m_TextureID );
                glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_pDepthTexture->m_TextureID, 0 );
            }
            else
            {
                //LOGInfo( LOGTag, "FBO: Attaching depth renderbuffer %d\n", m_pDepthTexture->m_TextureID );
               glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_pDepthTexture->m_TextureID );
            }
        }

        // Any problems?
        GLint status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
        checkGlError( "glCheckFramebufferStatus" );
        if( status != GL_FRAMEBUFFER_COMPLETE )
        {
            //LOGInfo( LOGTag, "CreateFBO - error glCheckFramebufferStatus( GL_FRAMEBUFFER )\n" );
            //MyAssert( false );
            Invalidate( true );
            return false;
        }

#if !MYFW_OPENGLES2
        // Set up the textures for GL to write to.
        GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
        glDrawBuffers( m_NumColorTextures, buffers );
#endif

        //LOGInfo( LOGTag, "FBO: created successfully\n" );

        MyBindFramebuffer( GL_FRAMEBUFFER, 0, 0, 0 );
        checkGlError( "glBindFramebufferEXT" );
    }

    //LOGInfo( LOGTag, "CreateFBO - complete (%d, %d)\n", m_TextureWidth, m_TextureHeight );
#else
    return false;
#endif

    m_FullyLoaded = true;

    return true;
}

void FBODefinition::Invalidate(bool cleanGLAllocs)
{
    m_FailedToInit = false;

    if( m_HasValidResources == false )
        return;

#if !USE_D3D
    if( cleanGLAllocs )
    {
        checkGlError( "start of FBODefinition::Invalidate" );

        if( g_GLStats.m_CurrentFramebuffer == m_FrameBufferID )
        {
            Unbind( true );
        }

        if( g_GLStats.m_PreviousFramebuffer == m_FrameBufferID )
        {
            g_GLStats.m_PreviousFramebuffer = 0;
            g_GLStats.m_PreviousFramebufferWidth = 0;
            g_GLStats.m_PreviousFramebufferHeight = 0;
        }

        for( int i=0; i<MAX_COLOR_TEXTURES; i++ )
        {
            if( m_pColorTextures[i] && m_pColorTextures[i]->m_TextureID != 0 )
            {
                glDeleteTextures( 1, &m_pColorTextures[i]->m_TextureID );
                m_pColorTextures[i]->m_TextureID = 0;
            }
        }

        if( m_pDepthTexture && m_pDepthTexture->m_TextureID != 0 )
        {
            if( m_DepthIsTexture )
                glDeleteTextures( 1, &m_pDepthTexture->m_TextureID );
            else
                glDeleteRenderbuffers( 1, &m_pDepthTexture->m_TextureID );

            m_pDepthTexture->m_TextureID = 0;
        }

        if( m_FrameBufferID != 0 )
        {
            glDeleteFramebuffers( 1, &m_FrameBufferID );
            m_FrameBufferID = 0;
        }

        checkGlError( "end of FBODefinition::Invalidate" );
    }
    else
    {
        for( int i=0; i<MAX_COLOR_TEXTURES; i++ )
        {
            if( m_pColorTextures[i] )
                m_pColorTextures[i]->Invalidate( false );
        }
        if( m_pDepthTexture )
            m_pDepthTexture->Invalidate( false );

        m_FrameBufferID = 0;
    }

    for( int i=0; i<MAX_COLOR_TEXTURES; i++ )
    {
        SAFE_RELEASE( m_pColorTextures[i] );
    }
    SAFE_RELEASE( m_pDepthTexture );

    m_FullyLoaded = false;
    m_HasValidResources = false;
#endif
}

void FBODefinition::Bind(bool storeFramebufferID)
{
#if !USE_D3D
    //if( storeFramebufferID )
    //    glGetIntegerv( GL_FRAMEBUFFER_BINDING, &m_LastFrameBufferID );

    MyBindFramebuffer( GL_FRAMEBUFFER, m_FrameBufferID, m_Width, m_Height );
    checkGlError( "glBindFramebuffer" );
#endif
}

void FBODefinition::Unbind(bool restoreLastFramebufferID)
{
    //if( restoreLastFramebufferID )
    //{
    //    MyAssert( m_LastFrameBufferID != -1 );
    //    MyBindFramebuffer( GL_FRAMEBUFFER, m_LastFrameBufferID );
        MyBindFramebuffer( GL_FRAMEBUFFER, g_GLStats.m_PreviousFramebuffer, g_GLStats.m_PreviousFramebufferWidth, g_GLStats.m_PreviousFramebufferHeight );
    //}
    //else
    //    glBindFramebuffer( GL_FRAMEBUFFER, 0 );

    //m_LastFrameBufferID = -1;
}
