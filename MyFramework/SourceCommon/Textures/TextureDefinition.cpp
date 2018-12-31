//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#include "TextureDefinition.h"
#include "TextureManager.h"
#include "../Helpers/FileManager.h"

// TODO: Fix GL Includes.
#include <gl/GL.h>
#include "../../GLExtensions.h"
#include "../Shaders/GLHelpers.h"

#pragma warning( push )
#pragma warning( disable : 4996 )
#include "../../Libraries/LodePNG/lodepng.h"
#pragma warning( pop )

TextureDefinition::TextureDefinition(bool freeOnceLoaded)
: m_FreeFileFromRamWhenTextureCreated(freeOnceLoaded)
{
    m_ManagedByTextureManager = false;

    m_FullyLoaded = false;

    m_Filename[0] = '\0';
    m_pFile = nullptr;
    m_TextureID = 0;

    m_MemoryUsed = 0;

    m_Width = 0;
    m_Height = 0;

    m_MinFilter = MyRE::MinFilter_Linear;
    m_MagFilter = MyRE::MagFilter_Linear;

    m_WrapS = MyRE::WrapMode_Clamp;
    m_WrapT = MyRE::WrapMode_Clamp;

#if MYFW_EDITOR
    m_ShowInMemoryPanel = true;
#endif
}

TextureDefinition::~TextureDefinition()
{
    if( m_ManagedByTextureManager )
    {
        this->Remove();
    }

    if( m_pFile )
    {
        g_pFileManager->FreeFile( m_pFile );
    }

    Invalidate( true );
}

#if MYFW_EDITOR
void TextureDefinition::OnPopupClick(TextureDefinition* pTexture, int id)
{
    MyFileObject* pTextureFile = pTexture->m_pFile;

    switch( id )
    {
    case RightClick_UnloadFile:
        {
            if( pTextureFile )
                g_pFileManager->Editor_UnloadFile( pTextureFile );
        }
        break;

    case RightClick_FindAllReferences:
        {
            if( pTextureFile )
                g_pFileManager->Editor_FindAllReferences( pTextureFile );
        }
        break;
    }
}
#endif //MYFW_EDITOR

void TextureDefinition::Invalidate(bool cleanGLAllocs)
{
    if( cleanGLAllocs && m_TextureID != 0 )
    {
        glDeleteTextures( 1, &m_TextureID );
    }

    m_TextureID = 0;
}

void TextureDefinition::FinishLoadingFileAndGenerateTexture()
{
    //LOGInfo( LOGTag, "FinishLoadingFileAndGenerateTexture texturedef(%d)", this );
    //LOGInfo( LOGTag, "FinishLoadingFileAndGenerateTexture texturedef->m_pFile(%d)", this->m_pFile );
    //LOGInfo( LOGTag, "FinishLoadingFileAndGenerateTexture texturedef->m_pFile->GetBuffer()(%d)", this->m_pFile->m_pBuffer );

    MyFileObject* pFile = GetFile();

    MyAssert( pFile );
    MyAssert( pFile->IsFinishedLoading() == true );
    MyAssert( pFile->GetBuffer() != nullptr );

    unsigned char* buffer = (unsigned char*)pFile->GetBuffer();
    int length = pFile->GetFileLength();

    unsigned char* pngbuffer;
    unsigned int width, height;

    unsigned int error = lodepng_decode32( &pngbuffer, &width, &height, buffer, length );
    MyAssert( error == 0 );

    GLuint textureHandle = 0;
    glGenTextures( 1, &textureHandle );
    MyAssert( textureHandle != 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, textureHandle );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pngbuffer );
    checkGlError( "glTexImage2D" );
    free( pngbuffer );

    g_pRenderer->SetTextureMinMagFilters( textureHandle, m_MinFilter, m_MagFilter );
    g_pRenderer->SetTextureWrapModes( textureHandle, m_WrapS, m_WrapT );

    m_Width = width;
    m_Height = height;

    m_TextureID = textureHandle;
}
