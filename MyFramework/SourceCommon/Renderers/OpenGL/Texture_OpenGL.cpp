//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#include <gl/GL.h>
#include "../../GLExtensions.h"
#include "GLHelpers.h"
#include "Texture_OpenGL.h"
#include "Buffer_OpenGL.h"

Texture_OpenGL::Texture_OpenGL()
{
    m_TextureID = 0;
}

Texture_OpenGL::~Texture_OpenGL()
{
    // Explicit call to virtual Invalidate() in this class.
    Texture_OpenGL::Invalidate( true );
}

bool Texture_OpenGL::IsFullyLoaded()
{
    // If m_FullyLoaded is set and we don't have a valid texture id, something is wrong.
    MyAssert( m_FullyLoaded == false || m_TextureID != 0 );

    return TextureDefinition::IsFullyLoaded();
}

GLuint Texture_OpenGL::GetTextureID()
{
    return m_TextureID;
}

void Texture_OpenGL::GenerateTexture(unsigned char* pImageBuffer, uint32 width, uint32 height)
{
    GLuint textureHandle = 0;
    glGenTextures( 1, &textureHandle );
    MyAssert( textureHandle != 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, textureHandle );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImageBuffer );
    checkGlError( "glTexImage2D" );

    g_pRenderer->SetTextureMinMagFilters( this, m_MinFilter, m_MagFilter );
    g_pRenderer->SetTextureWrapModes( this, m_WrapS, m_WrapT );

    m_Width = width;
    m_Height = height;

    m_TextureID = textureHandle;
}

void Texture_OpenGL::GenerateErrorTexture()
{
    m_ManagedByTextureManager = true;
    m_MinFilter = MyRE::MinFilter_Nearest;
    m_MagFilter = MyRE::MagFilter_Nearest;
    m_WrapS = MyRE::WrapMode_Repeat;
    m_WrapT = MyRE::WrapMode_Repeat;

    unsigned int width = 64;
    unsigned int height = 64;
    unsigned char* pixelbuffer = MyNew unsigned char[width*height*4];

    for( unsigned int y=0; y<height; y++ )
    {
        for( unsigned int x=0; x<width; x++ )
        {
            if( (x+y)%2 == 0 )
            {
                pixelbuffer[(y*width+x)*4 + 0] = 0;
                pixelbuffer[(y*width+x)*4 + 1] = 0;
                pixelbuffer[(y*width+x)*4 + 2] = 255;
                pixelbuffer[(y*width+x)*4 + 3] = 255;
            }
            else
            {
                pixelbuffer[(y*width+x)*4 + 0] = 0;
                pixelbuffer[(y*width+x)*4 + 1] = 0;
                pixelbuffer[(y*width+x)*4 + 2] = 0;
                pixelbuffer[(y*width+x)*4 + 3] = 255;
            }
        }
    }

    GLuint textureHandle = 0;
    glGenTextures( 1, &textureHandle );
    MyAssert( textureHandle != 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, textureHandle );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelbuffer );
    checkGlError( "glTexImage2D" );
    glBindTexture( GL_TEXTURE_2D, 0 );

    delete[] pixelbuffer;

    g_pRenderer->SetTextureMinMagFilters( this, MyRE::MinFilter_Nearest, MyRE::MagFilter_Nearest );
    g_pRenderer->SetTextureWrapModes( this, MyRE::WrapMode_Clamp, MyRE::WrapMode_Clamp );

    m_Width = width;
    m_Height = height;

    m_TextureID = textureHandle;
}

void Texture_OpenGL::Invalidate(bool cleanGLAllocs)
{
    if( cleanGLAllocs && m_TextureID != 0 )
    {
        glDeleteTextures( 1, &m_TextureID );
    }

    m_TextureID = 0;
}
