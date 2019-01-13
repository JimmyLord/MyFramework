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
        m_pColorTextures[i] = nullptr;
    }
    m_pDepthTexture = nullptr;

    m_Width = 0;
    m_Height = 0;

    m_TextureWidth = 0;
    m_TextureHeight = 0;

    m_MinFilter = MyRE::MinFilter_Linear;
    m_MagFilter = MyRE::MagFilter_Linear;

    for( int i=0; i<MAX_COLOR_TEXTURES; i++ )
        m_ColorFormats[i] = FBOColorFormat_None;
    m_DepthBits = 32;
    m_DepthIsTexture = false;

    m_LastFrameBufferID = -1;
}

FBODefinition::~FBODefinition()
{
    this->Remove();
}

// Returns true if a new textures need to be created.
bool FBODefinition::Setup(unsigned int width, unsigned int height, MyRE::MinFilters minFilter, MyRE::MagFilters magFilter, FBOColorFormat colorFormat, int depthBits, bool depthReadable)
{
    return Setup( width, height, minFilter, magFilter, &colorFormat, 1, depthBits, depthReadable );
}

bool FBODefinition::Setup(unsigned int width, unsigned int height, MyRE::MinFilters minFilter, MyRE::MagFilters magFilter, FBOColorFormat* colorFormats, int numColorFormats, int depthBits, bool depthReadable)
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

    bool newTextureNeeded = false;
    bool newFilterOptions = false;

    if( m_TextureWidth != NewTextureWidth || m_TextureHeight != NewTextureHeight )
        newTextureNeeded = true;

    m_NumColorTextures = numColorFormats;
    for( int i=0; i<numColorFormats; i++ )
    {
        if( m_ColorFormats[i] != colorFormats[i] )
            newTextureNeeded = true;
    }
    
    if( m_DepthBits != depthBits || m_DepthIsTexture != depthReadable )
        newTextureNeeded = true;

    if( newTextureNeeded == false && (m_MinFilter != minFilter || m_MagFilter != magFilter) )
        newFilterOptions = true;

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

    // If filter options changed, reset them on the texture.
    if( newFilterOptions == true )
    {
        for( int i=0; i<MAX_COLOR_TEXTURES; i++ )
        {
            g_pRenderer->SetTextureMinMagFilters( m_pColorTextures[i], m_MinFilter, m_MagFilter );
        }
    }

    return newTextureNeeded;
}

bool FBODefinition::Create()
{
    if( GenerateFrameBuffer() == false )
        return false;

    m_FullyLoaded = true;

    return true;
}
