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

TextureManager* g_pTextureManager = 0;

TextureManager::TextureManager()
{
    m_MaxTexturesToLoadInOneTick = -1;
}

TextureManager::~TextureManager()
{
    FreeAllTextures( true );
}

TextureDefinition* TextureManager::CreateTexture(const char* texturefilename, int minfilter, int magfilter, int wraps, int wrapt)
{
    MyAssert( texturefilename );
    //LOGInfo( LOGTag, "CreateTexture - %s\n", texturefilename );

    TextureDefinition* pTextureDef = FindTexture( texturefilename );
    if( pTextureDef != 0 )
    {
        pTextureDef->AddRef();
        return pTextureDef;
    }

    MyFileObject* pFile = g_pFileManager->RequestFile( texturefilename );
    pTextureDef = CreateTexture( pFile, minfilter, magfilter, wraps, wrapt );
    pFile->Release(); // CreateTexture() will add a ref.

    return pTextureDef;
}

TextureDefinition* TextureManager::CreateTexture(MyFileObject* pFile, int minfilter, int magfilter, int wraps, int wrapt)
{
    MyAssert( pFile );
    //LOGInfo( LOGTag, "CreateTexture - %s\n", pFile->GetFullPath() );

    // find the texture if it already exists:
    TextureDefinition* pTextureDef = FindTexture( pFile );
    if( pTextureDef != 0 )
    {
        pTextureDef->AddRef();
        return pTextureDef;
    }

    // Create a new texture and add it to m_TexturesStillLoading
    pTextureDef = MyNew TextureDefinition();
    pTextureDef->m_ManagedByTextureManager = true;
    strcpy_s( pTextureDef->m_Filename, MAX_PATH, pFile->GetFullPath() );
    pTextureDef->m_MinFilter = minfilter;
    pTextureDef->m_MagFilter = magfilter;
    pTextureDef->m_WrapS = wraps;
    pTextureDef->m_WrapT = wrapt;

    m_TexturesStillLoading.AddTail( pTextureDef );

    // assign the file to the texture def.  Add a ref to the file.
    MyAssert( pTextureDef->m_pFile == 0 );
    if( pTextureDef->m_pFile != 0 )
        pTextureDef->m_pFile->Release();

    //LOGInfo( LOGTag, "Loading Texture: RequestFile\n" );
    pTextureDef->m_pFile = pFile;
    pFile->AddRef();
    //LOGInfo( LOGTag, "Loading Texture: ~RequestFile\n" );

    return pTextureDef;
}

FBODefinition* TextureManager::CreateFBO(int width, int height, int minfilter, int magfilter, bool needcolor, int depthbits, bool depthreadable, bool onlyfreeonshutdown)
{
    //LOGInfo( LOGTag, "CreateFBO - %dx%d\n", width, height );

    FBODefinition* pFBO = MyNew FBODefinition();
    bool newtexneeded = pFBO->Setup( width, height, minfilter, magfilter, needcolor, depthbits, depthreadable );
    pFBO->m_OnlyFreeOnShutdown = onlyfreeonshutdown;

    if( newtexneeded )
    {
        m_UninitializedFBOs.AddTail( pFBO );
    }

    return pFBO;
}

// return true if new texture was needed.
bool TextureManager::ReSetupFBO(FBODefinition* pFBO, int width, int height, int minfilter, int magfilter, bool needcolor, int depthbits, bool depthreadable)
{
    //MyAssert( width > 0 && height > 0 );
    if( width <= 0 || height <= 0 )
        return false;
    //LOGInfo( LOGTag, "ReSetupFBO - %dx%d\n", width, height );

    bool newtexneeded = pFBO->Setup( width, height, minfilter, magfilter, needcolor, depthbits, depthreadable );

    if( newtexneeded )
    {
        LOGInfo( LOGTag, "ReSetupFBO - Creating new FBO textures %dx%d\n", width, height );
        InvalidateFBO( pFBO );
    }

    return newtexneeded;
}

void TextureManager::InvalidateFBO(FBODefinition* pFBO)
{
    pFBO->Invalidate( true );
    if( pFBO->Prev )
        m_UninitializedFBOs.MoveTail( pFBO );
}

void TextureManager::Tick()
{
    // Initialize all FBOs
    {
        CPPListNode* pNextNode;
        for( CPPListNode* pNode = m_UninitializedFBOs.GetHead(); pNode != 0; pNode = pNextNode )
        {
            pNextNode = pNode->GetNext();

            FBODefinition* pFBODef = (FBODefinition*)pNode;

            if( pFBODef->m_FailedToInit )
                continue;

            if( pFBODef->m_NeedColorTexture == false && pFBODef->m_DepthBits == 0 )
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

    //// debug: list all textures that need loading.
    //for( CPPListNode* pNode = m_TexturesStillLoading.GetHead(); pNode; pNode = pNode->GetNext() )
    //{
    //    TextureDefinition* pTextureDef = (TextureDefinition*)pNode;
    //    LOGInfo( LOGTag, "Still need to load: %s\n", pTextureDef->m_Filename );
    //}

    int texturesloadedthistick = 0;

    CPPListNode* pNextNode;
    for( CPPListNode* pNode = m_TexturesStillLoading.GetHead(); pNode != 0; pNode = pNextNode )
    {
        pNextNode = pNode->GetNext();

        if( m_MaxTexturesToLoadInOneTick != -1 && texturesloadedthistick >= m_MaxTexturesToLoadInOneTick )
            break;

        texturesloadedthistick++;

        TextureDefinition* pTextureDef = (TextureDefinition*)pNode;
        //LOGInfo( LOGTag, "Loading Texture: %s\n", pTextureDef->m_Filename );

        // if we have an opengl texture, then nothing to do.  this shouldn't happen, loaded textures should be in "m_LoadedTextures".
        MyAssert( pTextureDef->m_TextureID == 0 );
        if( pTextureDef->m_TextureID != 0 )
        {
            LOGInfo( LOGTag, "Loading Texture: Already had a texture id?!? pTextureDef->m_TextureID != 0\n" );
            continue;
        }

        bool textureloaded = false;

#if 0 //MYFW_ANDROID
        //LOGInfo( LOGTag, "Loading Texture: pTextureDef->m_pFile %d\n", pTextureDef->m_pFile );
        if( pTextureDef->m_pFile == 0 )
        {
            pTextureDef->m_pFile = RequestTexture( pTextureDef->m_Filename, pTextureDef );
            textureloaded = true;
        }
        else
        {
            LOGInfo( LOGTag, "Loading Texture: calling Android_LoadTextureFromMemory\n" );
            pTextureDef->m_TextureID = Android_LoadTextureFromMemory( pTextureDef );
            textureloaded = true;
        }
#else
        // if the file load hasn't started... start the file load.
        if( pTextureDef->m_pFile == 0 )
        {
            MyAssert( pTextureDef->m_Filename[0] != 0 );
            if( pTextureDef->m_Filename[0] != 0 )
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
                    //LOGInfo( LOGTag, "Loading Texture: textureloaded = true\n" );
                    textureloaded = true;
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
#if MYFW_USING_WX
                g_pPanelMemory->AddTexture( pTextureDef, "Failed to load", pTextureDef->m_Filename, TextureDefinition::StaticOnRightClick, TextureDefinition::StaticOnDrag );
#endif
            }
        }
#endif

        if( textureloaded )
        {
            //LOGInfo( LOGTag, "textureloaded %s\n", pTextureDef->m_Filename );

            // By default, we don't free the texture from main ram, so if we free the opengl tex, we can "reload" quick.
            if( pTextureDef->QueryFreeWhenCreated() )
                g_pFileManager->FreeFile( pTextureDef->m_pFile );

            m_LoadedTextures.MoveTail( pTextureDef );

            pTextureDef->m_FullyLoaded = true;

#if MYFW_USING_WX
            if( pTextureDef->m_ShowInMemoryPanel )
            {
                g_pPanelMemory->AddTexture( pTextureDef, "Global", pTextureDef->m_Filename, TextureDefinition::StaticOnRightClick, TextureDefinition::StaticOnDrag );
            }
#endif

            //LOGInfo( LOGTag, "pTextureDef->m_FullyLoaded = true %s\n", pTextureDef->m_Filename );
        }
    }
}

TextureDefinition* TextureManager::FindTexture(const char* texturefilename)
{
    for( CPPListNode* pNode = m_LoadedTextures.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        if( strcmp( ((TextureDefinition*)pNode)->m_Filename, texturefilename ) == 0 )
            return (TextureDefinition*)pNode;
    }

    for( CPPListNode* pNode = m_TexturesStillLoading.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        if( strcmp( ((TextureDefinition*)pNode)->m_Filename, texturefilename ) == 0 )
            return (TextureDefinition*)pNode;
    }

    return 0;
}

TextureDefinition* TextureManager::FindTexture(const MyFileObject* pFile)
{
    for( CPPListNode* pNode = m_LoadedTextures.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        if( ((TextureDefinition*)pNode)->m_pFile == pFile )
            return (TextureDefinition*)pNode;
    }

    for( CPPListNode* pNode = m_TexturesStillLoading.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        if( ((TextureDefinition*)pNode)->m_pFile == pFile )
            return (TextureDefinition*)pNode;
    }

    return 0;
}

void TextureManager::FreeAllTextures(bool shuttingdown)
{
    for( CPPListNode* pNode = m_LoadedTextures.GetHead(); pNode; )
    {
        TextureDefinition* pTextureDef = (TextureDefinition*)pNode;
        pNode = pNode->GetNext();

        MyAssert( pTextureDef->GetRefCount() == 1 );
        pTextureDef->Release();
    }

    for( CPPListNode* pNode = m_TexturesStillLoading.GetHead(); pNode; )
    {
        TextureDefinition* pTextureDef = (TextureDefinition*)pNode;
        pNode = pNode->GetNext();

        MyAssert( pTextureDef->GetRefCount() == 1 );
        pTextureDef->Release();
    }

    for( CPPListNode* pNode = m_InitializedFBOs.GetHead(); pNode; )
    {
        FBODefinition* pFBODef = (FBODefinition*)pNode;
        pNode = pNode->GetNext();

        if( pFBODef->m_OnlyFreeOnShutdown == false || shuttingdown )
        {
            MyAssert( pFBODef->GetRefCount() == 1 );
            pFBODef->Release();
        }
    }

    for( CPPListNode* pNode = m_UninitializedFBOs.GetHead(); pNode; )
    {
        FBODefinition* pFBODef = (FBODefinition*)pNode;
        pNode = pNode->GetNext();

        if( pFBODef->m_OnlyFreeOnShutdown == false || shuttingdown )
        {
            MyAssert( pFBODef->GetRefCount() == 1 );
            pFBODef->Release();
        }
    }
}

void TextureManager::InvalidateAllTextures(bool cleanglallocs)
{
    for( CPPListNode* pNode = m_LoadedTextures.GetHead(); pNode; )
    {
        TextureDefinition* pTextureDef = (TextureDefinition*)pNode;
        pNode = pNode->GetNext();
        
        pTextureDef->Invalidate( cleanglallocs );

        m_TexturesStillLoading.MoveTail( pTextureDef );

        pTextureDef->m_FullyLoaded = false;
    }

    for( CPPListNode* pNode = m_InitializedFBOs.GetHead(); pNode; )
    {
        FBODefinition* pFBODef = (FBODefinition*)pNode;
        pNode = pNode->GetNext();
        
        pFBODef->Invalidate( cleanglallocs );

        m_UninitializedFBOs.MoveTail( pFBODef );
    }
}
