//
// Copyright (c) 2012-2017 Jimmy Lord http://www.flatheadgames.com
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

TextureDefinition::TextureDefinition(bool freeonceloaded)
: m_FreeFileFromRamWhenTextureCreated(freeonceloaded)
{
    m_ManagedByTextureManager = false;

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
    if( m_ManagedByTextureManager )
    {
        this->Remove();
    }

    if( m_pFile )
    {
        g_pFileManager->FreeFile( m_pFile );
    }

    Invalidate( true );

#if MYFW_USING_WX
    if( m_ManagedByTextureManager )
    {
        if( g_pPanelMemory )
            g_pPanelMemory->RemoveTexture( this );
    }
#endif
}

#if MYFW_USING_WX
void TextureDefinition::OnRightClick() // StaticOnRightClick
{
 	wxMenu menu;
    menu.SetClientData( this );
    
    MyFileObject* pFile = this->m_pFile;
    if( pFile )
    {
        menu.Append( RightClick_UnloadFile, "Unload File" );
        menu.Append( RightClick_FindAllReferences, "Find References (" + std::to_string( (long long)this->GetRefCount() ) + ")" );
    }

    menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&TextureDefinition::OnPopupClick );

    // blocking call.
    g_pPanelWatch->PopupMenu( &menu ); // there's no reason this is using g_pPanelWatch other than convenience.
}

void TextureDefinition::OnPopupClick(wxEvent &evt)
{
    TextureDefinition* pTexture = (TextureDefinition*)static_cast<wxMenu*>(evt.GetEventObject())->GetClientData();
    MyFileObject* pTextureFile = pTexture->m_pFile;

    int id = evt.GetId();
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

void TextureDefinition::OnDrag()
{
    g_DragAndDropStruct.Add( DragAndDropType_TextureDefinitionPointer, this );
}
#endif //MYFW_USING_WX

void TextureDefinition::Invalidate(bool cleanglallocs)
{
    if( cleanglallocs && m_TextureID != 0 )
    {
        glDeleteTextures( 1, &m_TextureID );
    }

    m_TextureID = 0;
}