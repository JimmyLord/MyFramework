//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

My2DAnimationFrame::My2DAnimationFrame()
{
    m_pMaterial = 0;
}

My2DAnimationFrame::~My2DAnimationFrame()
{
    SAFE_RELEASE( m_pMaterial );
}

void My2DAnimationFrame::SetMaterial(MaterialDefinition* pMaterial)
{
    if( m_pMaterial == pMaterial )
        return;

    SAFE_RELEASE( m_pMaterial );
    m_pMaterial = pMaterial;
    m_pMaterial->AddRef();
}

void My2DAnimation::SetName(const char* name)
{
    if( strlen(name) > (unsigned int)MAX_ANIMATION_NAME_LEN )
        LOGInfo( LOGTag, "Warning: name longer than 32 characters - %s - truncating\n", name );
    strncpy_s( m_Name, MAX_ANIMATION_NAME_LEN+1, name, MAX_ANIMATION_NAME_LEN );
}

uint32 My2DAnimation::GetFrameCount()
{
    return m_Frames.Count();
}

My2DAnimationFrame* My2DAnimation::GetFrameByIndex(uint32 frameindex)
{
    MyAssert( frameindex < m_Frames.Count() );

    return m_Frames[frameindex];
}

My2DAnimationFrame* My2DAnimation::GetFrameByIndexClamped(uint32 frameindex)
{
    MyClamp( frameindex, (uint32)0, m_Frames.Count()-1 );

    return m_Frames[frameindex];
}

My2DAnimInfo::My2DAnimInfo()
{
    m_pSourceFile = 0;
}

My2DAnimInfo::~My2DAnimInfo()
{
    for( unsigned int i=0; i<m_Animations.Length(); i++ )
    {
        for( unsigned int j=0; j<m_Animations[i]->m_Frames.Length(); j++ )
        {
            delete m_Animations[i]->m_Frames[j];
        }

        delete m_Animations[i];
    }
    SAFE_RELEASE( m_pSourceFile );
}

uint32 My2DAnimInfo::GetNumberOfAnimations()
{
    return m_Animations.Count();
}

My2DAnimation* My2DAnimInfo::GetAnimationByIndex(uint32 animindex)
{
    MyAssert( animindex < m_Animations.Count() );

    return m_Animations[animindex];
}

My2DAnimation* My2DAnimInfo::GetAnimationByIndexClamped(uint32 animindex)
{
    MyClamp( animindex, (uint32)0, GetNumberOfAnimations()-1 );

    return m_Animations[animindex];
}

void My2DAnimInfo::SetSourceFile(MyFileObject* pSourceFile)
{
    if( m_pSourceFile == pSourceFile )
        return;

    SAFE_RELEASE( m_pSourceFile );
    m_pSourceFile = pSourceFile;
    m_pSourceFile->AddRef();

    wxTreeItemId treeid = g_pPanelMemory->FindFile( m_pSourceFile );
    if( treeid.IsOk() )
        g_pPanelMemory->SetFilePanelCallbacks( treeid, this, MyFileObject::StaticOnLeftClick, My2DAnimInfo::StaticOnRightClick, MyFileObject::StaticOnDrag );
}

#if MYFW_USING_WX
void My2DAnimInfo::RefreshWatchWindow()
{
    FillPropertiesWindow( true );
}

void My2DAnimInfo::FillPropertiesWindow(bool clear)
{
    g_pPanelWatch->SetRefreshCallback( this, My2DAnimInfo::StaticRefreshWatchWindow );

    g_pPanelWatch->Freeze();

    g_pPanelWatch->ClearAllVariables();

    g_pPanelWatch->AddButton( "Save Animations", this, My2DAnimInfo::StaticOnSaveAnimationsPressed );

    for( unsigned int animindex=0; animindex<m_Animations.Count(); animindex++ )
    {
        My2DAnimation* pAnim = m_Animations[animindex];
        g_pPanelWatch->AddSpace( pAnim->m_Name );
        g_pPanelWatch->AddString( "Animation Name", pAnim->m_Name, My2DAnimation::MAX_ANIMATION_NAME_LEN );

        unsigned int numframes = m_Animations[animindex]->GetFrameCount();

        for( unsigned int frameindex=0; frameindex<numframes; frameindex++ )
        {
            My2DAnimationFrame* pFrame = m_Animations[animindex]->GetFrameByIndex( frameindex );

            g_pPanelWatch->AddFloat( "Frame", &pFrame->m_Duration, 0, 1 );

            const char* desc = "no material";
            if( pFrame->m_pMaterial != 0 )
                desc = pFrame->m_pMaterial->GetName();
            //pFrame->m_pMaterial->m_ControlID_Shader = 
            g_pPanelWatch->AddPointerWithDescription( "Material", 0, desc, pFrame->m_pMaterial );//MaterialDefinition::StaticOnDropShader );
        }

        if( m_Animations[animindex]->m_Frames.Count() < MAX_FRAMES_IN_ANIMATION )
            g_pPanelWatch->AddButton( "Add Frame", this, My2DAnimInfo::StaticOnAddFramePressed );
    }

    if( m_Animations.Count() < MAX_ANIMATIONS )
        g_pPanelWatch->AddButton( "Add Animation", this, My2DAnimInfo::StaticOnAddAnimationPressed );

    g_pPanelWatch->Thaw();
}

void My2DAnimInfo::OnRightClick()
{
 	wxMenu menu;
    menu.SetClientData( this );
    
    menu.Append( RightClick_ViewInWatchWindow, "View in watch window" );
 	menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&My2DAnimInfo::OnPopupClick );

    // blocking call.
    g_pPanelWatch->PopupMenu( &menu ); // there's no reason this is using g_pPanelWatch other than convenience.
}

void My2DAnimInfo::OnPopupClick(wxEvent &evt)
{
    My2DAnimInfo* pAnimInfo = (My2DAnimInfo*)static_cast<wxMenu*>(evt.GetEventObject())->GetClientData();

    int id = evt.GetId();
    switch( id )
    {
    case RightClick_ViewInWatchWindow:
        {
            pAnimInfo->FillPropertiesWindow( false );
        }
        break;
    }
}

void My2DAnimInfo::OnAddAnimationPressed()
{
    if( m_Animations.Count() >= MAX_ANIMATIONS )
        return;

    My2DAnimation* pAnim = MyNew My2DAnimation;

    pAnim->SetName( "New" );

    m_Animations.Add( pAnim );

    g_pPanelWatch->SetNeedsRefresh();
}

void My2DAnimInfo::OnAddFramePressed()
{
    
}

void My2DAnimInfo::OnSaveAnimationsPressed()
{
    SaveAnimationControlFile();
}

void My2DAnimInfo::SaveAnimationControlFile()
{
    char filename[MAX_PATH];
    m_pSourceFile->GenerateNewFullPathExtensionWithSameNameInSameFolder( ".my2daniminfo", filename, MAX_PATH );

    cJSON* jRoot = cJSON_CreateObject();

    cJSON* jAnimArray = cJSON_CreateArray();
    cJSON_AddItemToObject( jRoot, "Anims", jAnimArray );

    for( unsigned int i=0; i<m_Animations.Count(); i++ )
    {
        cJSON* jAnim = cJSON_CreateObject();

        cJSON_AddItemToArray( jAnimArray, jAnim );

        cJSON_AddStringToObject( jAnim, "Name", m_Animations[i]->m_Name );

        // TODO: write out frame info
        MyAssert( false );
    }

    // dump animarray to disk
    char* jsonstr = cJSON_Print( jRoot );
    cJSON_Delete( jRoot );

    FILE* pFile;
#if MYFW_WINDOWS
    fopen_s( &pFile, filename, "wb" );
#else
    pFile = fopen( filename, "wb" );
#endif
    fprintf( pFile, "%s", jsonstr );
    fclose( pFile );

    cJSONExt_free( jsonstr );
}
#endif

void My2DAnimInfo::LoadAnimationControlFile(char* buffer)
{
    MyAssert( buffer != 0 );
    MyAssert( m_Animations.Count() == 0 );

    // if the file doesn't exist, do nothing for now.
    if( buffer == 0 )
    {
        MyAssert( m_Animations.Count() == 0 );
    }
    else
    {
        cJSON* jRoot = cJSON_Parse( buffer );

        if( jRoot )
        {
            cJSON* jAnimArray = cJSON_GetObjectItem( jRoot, "Anims" );

            int numanims = cJSON_GetArraySize( jAnimArray );
#if MYFW_USING_WX
            m_Animations.AllocateObjects( MAX_ANIMATIONS );
#else
            m_Animations.AllocateObjects( numanims );
#endif
            for( int i=0; i<numanims; i++ )
            {
                cJSON* jAnim = cJSON_GetArrayItem( jAnimArray, i );

                My2DAnimation* pAnim = MyNew My2DAnimation;
                m_Animations.Add( pAnim );

                cJSON* jName = cJSON_GetObjectItem( jAnim, "Name" );
                if( jName )
                {
                    pAnim->SetName( jName->valuestring );
                }

                cJSON* jFrameArray = cJSON_GetObjectItem( jAnim, "Frames" );

                int numframes = cJSON_GetArraySize( jFrameArray );
                pAnim->m_Frames.AllocateObjects( numframes );
                for( int i=0; i<numframes; i++ )
                {
                    cJSON* jFrame = cJSON_GetArrayItem( jFrameArray, i );

                    My2DAnimationFrame* pFrame = MyNew My2DAnimationFrame();
                    pAnim->m_Frames.Add( pFrame );

                    cJSON* jMatName = cJSON_GetObjectItem( jFrame, "Material" );
                    MaterialDefinition* pMaterial = g_pMaterialManager->LoadMaterial( jMatName->valuestring );
                    if( pMaterial )
                    {
                        pFrame->SetMaterial( pMaterial );
                        pMaterial->Release();
                    }

                    cJSONExt_GetFloat( jFrame, "Duration", &pFrame->m_Duration );
                }
            }
        }

        cJSON_Delete( jRoot );
    }
}
