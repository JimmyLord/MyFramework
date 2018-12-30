//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#include "FontManager.h"
#include "../Helpers/FileManager.h"

FontManager* g_pFontManager = 0;

FontDefinition::FontDefinition()
{
    m_FullyLoaded = false;
    m_FriendlyName[0] = 0;
    //m_Filename[0] = 0;
    m_pFile = 0;
    m_pBMFont = 0;

    m_pTextureDef = 0;
}

FontDefinition::~FontDefinition()
{
    this->Remove();
    SAFE_RELEASE( m_pFile );
    SAFE_RELEASE( m_pTextureDef );
    SAFE_DELETE( m_pBMFont );
}

//========================
//========================

FontDefinition* FontManager::CreateFont(const char* fontfilename)
{
    FontDefinition* pFontDef = FindFontByFilename( fontfilename );
    if( pFontDef )
    {
        pFontDef->AddRef();
        return pFontDef;
    }

    pFontDef = MyNew FontDefinition();
    pFontDef->m_pFile = g_pFileManager->RequestFile( fontfilename );

    m_FontsStillLoading.AddTail( pFontDef );

    return pFontDef;
}

FontDefinition* FontManager::CreateFont(MyFileObject* pFile)
{
    MyAssert( pFile );

    FontDefinition* pFontDef = FindFont( pFile );
    if( pFontDef )
    {
        pFontDef->AddRef();
        return pFontDef;
    }

    pFontDef = MyNew FontDefinition();
    pFontDef->m_pFile = pFile;
    pFile->AddRef();

    m_FontsStillLoading.AddTail( pFontDef );

    return pFontDef;
}

void FontManager::Tick()
{
    // finish loading any fonts.
    for( FontDefinition* pFontDef = m_FontsStillLoading.GetHead(); pFontDef; pFontDef = pFontDef->GetNext() )
    {
        // check if the actual font description file(*.fnt) file is done loading.
        if( pFontDef->m_pFile && pFontDef->m_pBMFont == 0 && pFontDef->m_pFile->GetFileLoadStatus() == FileLoadStatus_Success )
        {
            // create the font description object.
            pFontDef->m_pBMFont = MyNew BMFont( pFontDef->m_pFile->GetBuffer(), pFontDef->m_pFile->GetFileLength() );

            // load the texture the font is stored on.
            char tempname[MAX_PATH];
            strcpy_s( tempname, MAX_PATH, pFontDef->m_pFile->GetFullPath() );
            for( int i=(int)strlen(pFontDef->m_pFile->GetFullPath())-1; i>=0; i-- )
            {
                if( tempname[i] == '/' || tempname[i] == '\\' )
                    break;

                tempname[i] = 0;
            }
            strcat_s( tempname, MAX_PATH, pFontDef->m_pBMFont->QueryImageName() );
            pFontDef->m_pTextureDef = g_pTextureManager->CreateTexture( tempname, MyRE::MinFilter_Linear, MyRE::MagFilter_Linear, MyRE::WrapMode_Clamp, MyRE::WrapMode_Clamp );
#if MYFW_EDITOR
            if( pFontDef->m_pFile->MemoryPanel_IsVisible() == false )
            {
                pFontDef->m_pTextureDef->GetFile()->MemoryPanel_Hide();
                pFontDef->m_pTextureDef->MemoryPanel_Hide();
            }
#endif
        }

        // when the font texture is loaded, move the font to the other list and mark it fully loaded.
        if( pFontDef->m_pTextureDef && pFontDef->m_pTextureDef->IsFullyLoaded() )
        {
            MyAssert( pFontDef->m_pTextureDef->GetTextureID() != 0 );
            pFontDef->m_FullyLoaded = true;
            m_FontsLoaded.MoveTail( pFontDef );
        }
    }
}

FontDefinition* FontManager::GetFirstFont()
{
    if( m_FontsLoaded.GetHead() )
        return m_FontsLoaded.GetHead();

    if( m_FontsStillLoading.GetHead() )
        return m_FontsStillLoading.GetHead();

    return 0;
}

FontDefinition* FontManager::FindFont(const char* friendlyname)
{
    for( FontDefinition* pFontDef = m_FontsLoaded.GetHead(); pFontDef; pFontDef = pFontDef->GetNext() )
    {
        if( strcmp( pFontDef->m_FriendlyName, friendlyname ) == 0 )
            return pFontDef;
    }

    for( FontDefinition* pFontDef = m_FontsStillLoading.GetHead(); pFontDef; pFontDef = pFontDef->GetNext() )
    {
        if( strcmp( pFontDef->m_FriendlyName, friendlyname ) == 0 )
            return pFontDef;
    }

    return 0;
}

FontDefinition* FontManager::FindFont(MyFileObject* pFile)
{
    for( FontDefinition* pFontDef = m_FontsLoaded.GetHead(); pFontDef; pFontDef = pFontDef->GetNext() )
    {
        if( pFontDef->m_pFile == pFile )
            return pFontDef;
    }

    for( FontDefinition* pFontDef = m_FontsStillLoading.GetHead(); pFontDef; pFontDef = pFontDef->GetNext() )
    {
        if( pFontDef->m_pFile == pFile )
            return pFontDef;
    }

    return 0;
}

FontDefinition* FontManager::FindFontByFilename(const char* fullpath)
{
    for( FontDefinition* pFontDef = m_FontsLoaded.GetHead(); pFontDef; pFontDef = pFontDef->GetNext() )
    {
        if( strcmp( pFontDef->m_pFile->GetFullPath(), fullpath ) == 0 )
            return pFontDef;
    }

    for( FontDefinition* pFontDef = m_FontsStillLoading.GetHead(); pFontDef; pFontDef = pFontDef->GetNext() )
    {
        if( strcmp( pFontDef->m_pFile->GetFullPath(), fullpath ) == 0 )
            return pFontDef;
    }

    return 0;
}

void FontManager::FreeAllFonts()
{
    MyAssert( m_FontsLoaded.GetHead() == 0 );
    MyAssert( m_FontsStillLoading.GetHead() == 0 );

    while( FontDefinition* pFontDef = m_FontsLoaded.GetHead() )
    {
        pFontDef->Release();
    }

    while( FontDefinition* pFontDef = m_FontsStillLoading.GetHead() )
    {
        pFontDef->Release();
    }
}
