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

TextureManager* g_pTextureManager = nullptr;

TextureManager::TextureManager()
{
    m_MaxTexturesToLoadInOneTick = -1;

    m_pErrorTexture = nullptr;
}

TextureManager::~TextureManager()
{
    FreeAllTextures( true );
}

TextureDefinition* TextureManager::CreateTexture(const char* textureFilename, MyRE::MinFilters minFilter, MyRE::MagFilters magFilter, MyRE::WrapModes wrapS, MyRE::WrapModes wrapT)
{
    MyAssert( textureFilename );
    //LOGInfo( LOGTag, "CreateTexture - %s\n", textureFilename );

    TextureDefinition* pTextureDef = FindTexture( textureFilename );
    if( pTextureDef != nullptr )
    {
        pTextureDef->AddRef();
        return pTextureDef;
    }

    MyFileObject* pFile = g_pFileManager->RequestFile( textureFilename );
    pTextureDef = CreateTexture( pFile, minFilter, magFilter, wrapS, wrapT );
    pFile->Release(); // CreateTexture() will add a ref.

    return pTextureDef;
}

TextureDefinition* TextureManager::CreateTexture(MyFileObject* pFile, MyRE::MinFilters minFilter, MyRE::MagFilters magFilter, MyRE::WrapModes wrapS, MyRE::WrapModes wrapT)
{
    MyAssert( pFile );
    //LOGInfo( LOGTag, "CreateTexture - %s\n", pFile->GetFullPath() );

    // Find the texture if it already exists.
    TextureDefinition* pTextureDef = FindTexture( pFile );
    if( pTextureDef != nullptr )
    {
        pTextureDef->AddRef();
        return pTextureDef;
    }

    // Create a new texture and add it to m_TexturesStillLoading.
    pTextureDef = MyNew TextureDefinition();
    pTextureDef->m_ManagedByTextureManager = true;
    strcpy_s( pTextureDef->m_Filename, MAX_PATH, pFile->GetFullPath() );
    pTextureDef->m_MinFilter = minFilter;
    pTextureDef->m_MagFilter = magFilter;
    pTextureDef->m_WrapS = wrapS;
    pTextureDef->m_WrapT = wrapT;

    m_TexturesStillLoading.AddTail( pTextureDef );

    // Assign the file to the texture def.  Add a ref to the file.
    MyAssert( pTextureDef->m_pFile == nullptr );
    if( pTextureDef->m_pFile != nullptr )
        pTextureDef->m_pFile->Release();

    //LOGInfo( LOGTag, "Loading Texture: RequestFile\n" );
    pTextureDef->m_pFile = pFile;
    pFile->AddRef();
    //LOGInfo( LOGTag, "Loading Texture: ~RequestFile\n" );

    return pTextureDef;
}

FBODefinition* TextureManager::CreateFBO(int width, int height, MyRE::MinFilters minFilter, MyRE::MagFilters magFilter, FBODefinition::FBOColorFormat colorFormat, int depthBits, bool depthReadable, bool onlyFreeOnShutdown)
{
    //LOGInfo( LOGTag, "CreateFBO - %dx%d\n", width, height );

    FBODefinition* pFBO = MyNew FBODefinition();
    bool newtexneeded = pFBO->Setup( width, height, minFilter, magFilter, colorFormat, depthBits, depthReadable );
    pFBO->m_OnlyFreeOnShutdown = onlyFreeOnShutdown;

    if( newtexneeded )
    {
        m_UninitializedFBOs.AddTail( pFBO );
    }

    return pFBO;
}

FBODefinition* TextureManager::CreateFBO(int width, int height, MyRE::MinFilters minFilter, MyRE::MagFilters magFilter, FBODefinition::FBOColorFormat* colorFormats, int numColorFormats, int depthBits, bool depthReadable, bool onlyFreeOnShutdown)
{
    //LOGInfo( LOGTag, "CreateFBO - %dx%d\n", width, height );

    FBODefinition* pFBO = MyNew FBODefinition();
    bool newTexNeeded = pFBO->Setup( width, height, minFilter, magFilter, colorFormats, numColorFormats, depthBits, depthReadable );
    pFBO->m_OnlyFreeOnShutdown = onlyFreeOnShutdown;

    if( newTexNeeded )
    {
        m_UninitializedFBOs.AddTail( pFBO );
    }

    return pFBO;
}

// return true if new texture was needed.
bool TextureManager::ReSetupFBO(FBODefinition* pFBO, int width, int height, MyRE::MinFilters minFilter, MyRE::MagFilters magFilter, FBODefinition::FBOColorFormat colorFormat, int depthBits, bool depthReadable)
{
    //MyAssert( width > 0 && height > 0 );
    if( width <= 0 || height <= 0 )
        return false;
    //LOGInfo( LOGTag, "ReSetupFBO - %dx%d\n", width, height );

    bool newTexNeeded = pFBO->Setup( width, height, minFilter, magFilter, colorFormat, depthBits, depthReadable );

    if( newTexNeeded )
    {
        LOGInfo( LOGTag, "ReSetupFBO - Creating new FBO textures %dx%d\n", width, height );
        InvalidateFBO( pFBO );
    }

    return newTexNeeded;
}

// return true if new texture was needed.
bool TextureManager::ReSetupFBO(FBODefinition* pFBO, int width, int height, MyRE::MinFilters minFilter, MyRE::MagFilters magFilter, FBODefinition::FBOColorFormat* colorFormats, int numColorFormats, int depthBits, bool depthReadable)
{
    //MyAssert( width > 0 && height > 0 );
    if( width <= 0 || height <= 0 )
        return false;
    //LOGInfo( LOGTag, "ReSetupFBO - %dx%d\n", width, height );

    bool newTexNeeded = pFBO->Setup( width, height, minFilter, magFilter, colorFormats, numColorFormats, depthBits, depthReadable );

    if( newTexNeeded )
    {
        LOGInfo( LOGTag, "ReSetupFBO - Creating new FBO textures %dx%d\n", width, height );
        InvalidateFBO( pFBO );
    }

    return newTexNeeded;
}

void TextureManager::InvalidateFBO(FBODefinition* pFBO)
{
    pFBO->Invalidate( true );
    if( pFBO->Prev )
        m_UninitializedFBOs.MoveTail( pFBO );
}

void TextureManager::Tick()
{
    // Initialize all FBOs.
    {
        FBODefinition* pNextFBODef;
        for( FBODefinition* pFBODef = m_UninitializedFBOs.GetHead(); pFBODef; pFBODef = pNextFBODef )
        {
            pNextFBODef = pFBODef->GetNext();

            if( pFBODef->m_FailedToInit )
                continue;

            if( pFBODef->m_ColorFormats[0] == FBODefinition::FBOColorFormat_None && pFBODef->m_DepthBits == 0 )
                continue;

            bool success = pFBODef->Create();

            if( success )
            {
                //LOGInfo( LOGTag, "pFBODef->Create() succeeded\n" );
                //g_pPanelMemory->AddTexture( pFBODef );

                m_InitializedFBOs.MoveTail( pFBODef );
            }
            else
            {
                pFBODef->m_FailedToInit = true;

                LOGError( LOGTag, "========================\n" );
                LOGError( LOGTag, "pFBODef->Create() failed\n" );
                LOGError( LOGTag, "========================\n" );
            }
        }
    }

    //// Debug: List all textures that need loading.
    //for( TextureDefinition* pTextureDef = m_TexturesStillLoading.GetHead(); pTextureDef; pTextureDef = pTextureDef->GetNext() )
    //{
    //    LOGInfo( LOGTag, "Still need to load: %s\n", pTextureDef->m_Filename );
    //}

    int texturesLoadedThisTick = 0;

    CPPListNode* pNextNode;
    for( CPPListNode* pNode = m_TexturesStillLoading.GetHead(); pNode; pNode = pNextNode )
    {
        pNextNode = pNode->GetNext();

        if( m_MaxTexturesToLoadInOneTick != -1 && texturesLoadedThisTick >= m_MaxTexturesToLoadInOneTick )
            break;

        texturesLoadedThisTick++;

        TextureDefinition* pTextureDef = (TextureDefinition*)pNode;
        //LOGInfo( LOGTag, "Loading Texture: %s\n", pTextureDef->m_Filename );

        // If we have an opengl texture, then nothing to do.  this shouldn't happen, loaded textures should be in "m_LoadedTextures".
        MyAssert( pTextureDef->m_TextureID == 0 );
        if( pTextureDef->m_TextureID != 0 )
        {
            LOGInfo( LOGTag, "Loading Texture: Already had a texture id?!? pTextureDef->m_TextureID != 0\n" );
            continue;
        }

        bool textureLoaded = false;

#if 0 //MYFW_ANDROID
        //LOGInfo( LOGTag, "Loading Texture: pTextureDef->m_pFile %d\n", pTextureDef->m_pFile );
        if( pTextureDef->m_pFile == nullptr )
        {
            pTextureDef->m_pFile = RequestTexture( pTextureDef->m_Filename, pTextureDef );
            textureLoaded = true;
        }
        else
        {
            LOGInfo( LOGTag, "Loading Texture: calling Android_LoadTextureFromMemory\n" );
            pTextureDef->m_TextureID = Android_LoadTextureFromMemory( pTextureDef );
            textureLoaded = true;
        }
#else
        // If the file load hasn't started... start the file load.
        if( pTextureDef->m_pFile == nullptr )
        {
            MyAssert( pTextureDef->m_Filename[0] != '\0' );
            if( pTextureDef->m_Filename[0] != '\0' )
            {
                //LOGInfo( LOGTag, "Loading Texture: RequestFile\n" );
                pTextureDef->m_pFile = RequestFile( pTextureDef->m_Filename );
                //LOGInfo( LOGTag, "Loading Texture: ~RequestFile\n" );
            }
        }
        else
        {
            // if the file is ready, create an opengl texture from it.
            if( pTextureDef->m_pFile->GetFileLoadStatus() == FileLoadStatus_Success )
            {
                //LOGInfo( LOGTag, "Loading Texture: pTextureDef->m_pFile->m_FileReady\n" );
                pTextureDef->FinishLoadingFileAndGenerateTexture();
                //LOGInfo( LOGTag, "Loading Texture: FinishLoadingFileAndGenerateTexture\n" );

                if( pTextureDef->m_TextureID != 0 )
                {
                    //LOGInfo( LOGTag, "Loading Texture: textureLoaded = true\n" );
                    textureLoaded = true;
                }
            }

            if( pTextureDef->m_pFile->GetFileLoadStatus() > FileLoadStatus_Success )
            {
                LOGError( LOGTag, "File load failed %s\n", pTextureDef->m_Filename );

                // By default, we don't free the texture from main ram, so if we free the opengl tex, we can "reload" quick.
                if( pTextureDef->QueryFreeWhenCreated() )
                    g_pFileManager->FreeFile( pTextureDef->m_pFile );

                // The texture failed to load, but add it to the loaded texture list anyway.
                m_LoadedTextures.MoveTail( pTextureDef );
            }
        }
#endif

        if( textureLoaded )
        {
            //LOGInfo( LOGTag, "textureloaded %s\n", pTextureDef->m_Filename );

            // By default, we don't free the texture from main ram, so if we free the opengl tex, we can "reload" quick.
            if( pTextureDef->QueryFreeWhenCreated() )
                g_pFileManager->FreeFile( pTextureDef->m_pFile );

            m_LoadedTextures.MoveTail( pTextureDef );

            pTextureDef->m_FullyLoaded = true;

            //LOGInfo( LOGTag, "pTextureDef->m_FullyLoaded = true %s\n", pTextureDef->m_Filename );
        }
    }
}

TextureDefinition* TextureManager::FindTexture(const char* textureFilename)
{
    for( TextureDefinition* pTextureDef = m_LoadedTextures.GetHead(); pTextureDef; pTextureDef = pTextureDef->GetNext() )
    {
        if( strcmp( pTextureDef->m_Filename, textureFilename ) == 0 )
            return pTextureDef;
    }

    for( TextureDefinition* pTextureDef = m_TexturesStillLoading.GetHead(); pTextureDef; pTextureDef = pTextureDef->GetNext() )
    {
        if( strcmp( pTextureDef->m_Filename, textureFilename ) == 0 )
            return pTextureDef;
    }

    return nullptr;
}

TextureDefinition* TextureManager::FindTexture(const MyFileObject* pFile)
{
    for( TextureDefinition* pTextureDef = m_LoadedTextures.GetHead(); pTextureDef; pTextureDef = pTextureDef->GetNext() )
    {
        if( pTextureDef->m_pFile == pFile )
            return pTextureDef;
    }

    for( TextureDefinition* pTextureDef = m_TexturesStillLoading.GetHead(); pTextureDef; pTextureDef = pTextureDef->GetNext() )
    {
        if( pTextureDef->m_pFile == pFile )
            return pTextureDef;
    }

    return nullptr;
}

void TextureManager::FreeAllTextures(bool shuttingDown)
{
    TextureDefinition* pNextTextureDef;
    for( TextureDefinition* pTextureDef = m_LoadedTextures.GetHead(); pTextureDef; pTextureDef = pNextTextureDef )
    {
        pNextTextureDef = pTextureDef->GetNext();

        MyAssert( pTextureDef->GetRefCount() == 1 );
        pTextureDef->Release();
    }

    for( TextureDefinition* pTextureDef = m_TexturesStillLoading.GetHead(); pTextureDef; pTextureDef = pNextTextureDef )
    {
        pNextTextureDef = pTextureDef->GetNext();

        MyAssert( pTextureDef->GetRefCount() == 1 );
        pTextureDef->Release();
    }

    FBODefinition* pNextFBODef;
    for( FBODefinition* pFBODef = m_InitializedFBOs.GetHead(); pFBODef; pFBODef = pNextFBODef )
    {
        pNextFBODef = pFBODef->GetNext();

        if( pFBODef->m_OnlyFreeOnShutdown == false || shuttingDown )
        {
            MyAssert( pFBODef->GetRefCount() == 1 );
            pFBODef->Release();
        }
    }

    for( FBODefinition* pFBODef = m_UninitializedFBOs.GetHead(); pFBODef; pFBODef = pNextFBODef )
    {
        pNextFBODef = pFBODef->GetNext();

        if( pFBODef->m_OnlyFreeOnShutdown == false || shuttingDown )
        {
            MyAssert( pFBODef->GetRefCount() == 1 );
            pFBODef->Release();
        }
    }
}

void TextureManager::InvalidateAllTextures(bool cleanGLAllocs)
{
    TextureDefinition* pNextTextureDef;
    for( TextureDefinition* pTextureDef = m_LoadedTextures.GetHead(); pTextureDef; pTextureDef = pNextTextureDef )
    {
        pNextTextureDef = pTextureDef->GetNext();
        
        pTextureDef->Invalidate( cleanGLAllocs );

        m_TexturesStillLoading.MoveTail( pTextureDef );

        pTextureDef->m_FullyLoaded = false;
    }

    FBODefinition* pNextFBODef;
    for( FBODefinition* pFBODef = m_InitializedFBOs.GetHead(); pFBODef; pFBODef = pNextFBODef )
    {
        pNextFBODef = pFBODef->GetNext();
        
        pFBODef->Invalidate( cleanGLAllocs );

        m_UninitializedFBOs.MoveTail( pFBODef );
    }
}

TextureDefinition* TextureManager::GetErrorTexture()
{
    // If the error texture isn't created, create it.
    if( m_pErrorTexture == nullptr )
    {
        m_pErrorTexture = MyNew TextureDefinition();

        m_pErrorTexture->m_ManagedByTextureManager = true;
        m_pErrorTexture->m_MinFilter = MyRE::MinFilter_Nearest;
        m_pErrorTexture->m_MagFilter = MyRE::MagFilter_Nearest;
        m_pErrorTexture->m_WrapS = MyRE::WrapMode_Repeat;
        m_pErrorTexture->m_WrapT = MyRE::WrapMode_Repeat;

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

        g_pRenderer->SetTextureMinMagFilters( textureHandle, MyRE::MinFilter_Nearest, MyRE::MagFilter_Nearest );
        g_pRenderer->SetTextureWrapModes( textureHandle, MyRE::WrapMode_Clamp, MyRE::WrapMode_Clamp );

        m_pErrorTexture->m_Width = width;
        m_pErrorTexture->m_Height = height;

        m_pErrorTexture->m_TextureID = textureHandle;

        m_LoadedTextures.AddTail( m_pErrorTexture );
    }

    return m_pErrorTexture;
}
