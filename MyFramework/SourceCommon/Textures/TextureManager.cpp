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

#include "TextureManager.h"
#include "../Helpers/FileManager.h"

TextureManager* g_pTextureManager = 0;

TextureDefinition::TextureDefinition(bool freeonceloaded)
: m_FreeFileFromRamWhenTextureCreated(freeonceloaded)
{
    m_FullyLoaded = false;

    m_Filename[0] = 0;
    m_pFile = 0;
    m_TextureID = 0;

    m_MemoryUsed = 0;

    m_Width = 0;
    m_Height = 0;

    m_MinFilter = GL_LINEAR;
    m_MagFilter = GL_LINEAR;

    m_WrapS = GL_CLAMP_TO_EDGE;
    m_WrapT = GL_CLAMP_TO_EDGE;
}

TextureDefinition::~TextureDefinition()
{
    this->Remove();

    g_pFileManager->FreeFile( m_pFile );
    Invalidate( true );

#if MYFW_USING_WX
    if( g_pPanelMemory )
        g_pPanelMemory->RemoveTexture( this );
#endif
}

#if MYFW_USING_WX
void TextureDefinition::OnDrag()
{
    g_DragAndDropStruct.m_Type = DragAndDropType_TextureDefinitionPointer;
    g_DragAndDropStruct.m_Value = this;
}
#endif //MYFW_USING_WX

void TextureDefinition::Invalidate(bool cleanglallocs)
{
    if( cleanglallocs )
    {
        glDeleteTextures( 1, &m_TextureID );
    }

    m_TextureID = 0;
}

TextureManager::TextureManager()
{
    m_MaxTexturesToLoadInOneTick = -1;
}

TextureManager::~TextureManager()
{
    FreeAllTextures();
}

TextureDefinition* TextureManager::CreateTexture(const char* texturefilename, int minfilter, int magfilter, int wraps, int wrapt)
{
    assert( texturefilename );

    LOGInfo( LOGTag, "CreateTexture - %s\n", texturefilename );

    // find the texture if it already exists:
    for( CPPListNode* pNode = m_TexturesStillLoading.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        TextureDefinition* pTextureDef = (TextureDefinition*)pNode;

        if( strcmp( pTextureDef->m_Filename, texturefilename ) == 0 )
        {
            pTextureDef->AddRef();
            return pTextureDef;
        }
    }

    for( CPPListNode* pNode = m_LoadedTextures.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        TextureDefinition* pTextureDef = (TextureDefinition*)pNode;

        if( strcmp( pTextureDef->m_Filename, texturefilename ) == 0 )
        {
            pTextureDef->AddRef();
            return pTextureDef;
        }
    }

    TextureDefinition* pTextureDef = MyNew TextureDefinition();
    strcpy_s( pTextureDef->m_Filename, MAX_PATH, texturefilename );
    pTextureDef->m_MinFilter = minfilter;
    pTextureDef->m_MagFilter = magfilter;
    pTextureDef->m_WrapS = wraps;
    pTextureDef->m_WrapT = wrapt;

    m_TexturesStillLoading.AddTail( pTextureDef );

    // if the file load hasn't started... start the file load.
    assert( pTextureDef->m_pFile == 0 );
#if MYFW_ANDROID
    LOGInfo( LOGTag, "Loading Texture: pTextureDef->m_pFile %d\n", pTextureDef->m_pFile );
    if( pTextureDef->m_pFile == 0 )
    {
        pTextureDef->m_pFile = RequestTexture( pTextureDef->m_Filename, pTextureDef );
        textureloaded = true;
    }
#else
    if( pTextureDef->m_pFile == 0 )
    {
        LOGInfo( LOGTag, "Loading Texture: RequestFile\n" );
        pTextureDef->m_pFile = RequestFile( pTextureDef->m_Filename );
        LOGInfo( LOGTag, "Loading Texture: ~RequestFile\n" );
    }
#endif

    return pTextureDef;
}

FBODefinition* TextureManager::CreateFBO(int width, int height, int minfilter, int magfilter, bool needcolor, bool needdepth, bool depthreadable)
{
    LOGInfo( LOGTag, "CreateFBO - %dx%d\n", width, height );

    FBODefinition* pFBODef = MyNew FBODefinition();
    pFBODef->Setup( width, height, minfilter, magfilter, needcolor, needdepth, depthreadable );

    m_UninitializedFBOs.AddTail( pFBODef );

    return pFBODef;
}

void TextureManager::InvalidateFBO(FBODefinition* pFBO)
{
    pFBO->Invalidate( true );
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

            bool success = pFBODef->Create();

            if( success )
            {
                LOGInfo( LOGTag, "pFBODef->Create() succeeded\n" );
                //g_pPanelMemory->AddTexture( pFBODef );

                m_InitializedFBOs.MoveTail( pFBODef );
                pFBODef->m_FullyLoaded = true;
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
        LOGInfo( LOGTag, "Loading Texture: %s\n", pTextureDef->m_Filename );

        // if we have an opengl texture, then nothing to do.  this shouldn't happen, loaded textures should be in "m_LoadedTextures".
        assert( pTextureDef->m_TextureID == 0 );
        if( pTextureDef->m_TextureID != 0 )
        {
            LOGInfo( LOGTag, "Loading Texture: Already had a texture id?!? pTextureDef->m_TextureID != 0\n" );
            continue;
        }

        bool textureloaded = false;

#if MYFW_ANDROID
        LOGInfo( LOGTag, "Loading Texture: pTextureDef->m_pFile %d\n", pTextureDef->m_pFile );
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
            LOGInfo( LOGTag, "Loading Texture: RequestFile\n" );
            pTextureDef->m_pFile = RequestFile( pTextureDef->m_Filename );
            LOGInfo( LOGTag, "Loading Texture: ~RequestFile\n" );
        }
        else
        {
            // if the file is ready, create an opengl texture from it.
            if( pTextureDef->m_pFile->m_FileReady )
            {
                LOGInfo( LOGTag, "Loading Texture: pTextureDef->m_pFile->m_FileReady\n" );
                pTextureDef->m_TextureID = CreateTextureFromBuffer( pTextureDef );
                LOGInfo( LOGTag, "Loading Texture: CreateTextureFromBuffer\n" );

                if( pTextureDef->m_TextureID != 0 )
                {
                    LOGInfo( LOGTag, "Loading Texture: textureloaded = true\n" );
                    textureloaded = true;
                }
            }

            if( pTextureDef->m_pFile->m_LoadFailed )
            {
                LOGError( LOGTag, "File load failed\n" );
                SAFE_DELETE( pTextureDef );
            }
        }
#endif

        if( textureloaded )
        {
            // by default, we don't free the texture from main ram, so if we free the opengl tex, we can "reload" quick.
            if( pTextureDef->QueryFreeWhenCreated() )
                g_pFileManager->FreeFile( pTextureDef->m_pFile );

            m_LoadedTextures.MoveTail( pTextureDef );

            pTextureDef->m_FullyLoaded = true;

#if MYFW_USING_WX
            g_pPanelMemory->AddTexture( pTextureDef, "Global", pTextureDef->m_Filename, TextureDefinition::StaticOnDrag );
#endif
        }
    }
}

GLuint TextureManager::CreateTextureFromBuffer(TextureDefinition* texturedef)
{
#if MYFW_ANDROID
    return 0;
#else
    return LoadTextureFromMemory( texturedef );
#endif
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

void TextureManager::FreeAllTextures()
{
    while( CPPListNode* pNode = m_LoadedTextures.GetHead() )
    {
        ((TextureDefinition*)pNode)->Release();
    }

    while( CPPListNode* pNode = m_TexturesStillLoading.GetHead() )
    {
        ((TextureDefinition*)pNode)->Release();
    }

    while( CPPListNode* pNode = m_InitializedFBOs.GetHead() )
    {
        ((TextureDefinition*)pNode)->Release();
    }

    while( CPPListNode* pNode = m_UninitializedFBOs.GetHead() )
    {
        ((TextureDefinition*)pNode)->Release();
    }
}

void TextureManager::InvalidateAllTextures(bool cleanglallocs)
{
    for( CPPListNode* pNode = m_LoadedTextures.GetHead(); pNode; )
    {
        TextureDefinition* pTextureDef = (TextureDefinition*)pNode;
        pNode = pNode->GetNext();
        
        pTextureDef->Invalidate(cleanglallocs);

        m_TexturesStillLoading.MoveTail( pTextureDef );

        pTextureDef->m_FullyLoaded = false;
    }

    for( CPPListNode* pNode = m_InitializedFBOs.GetHead(); pNode; )
    {
        FBODefinition* pFBODef = (FBODefinition*)pNode;
        pNode = pNode->GetNext();
        
        pFBODef->Invalidate(cleanglallocs);

        m_UninitializedFBOs.MoveTail( pFBODef );

        pFBODef->m_FullyLoaded = false;
    }
}
