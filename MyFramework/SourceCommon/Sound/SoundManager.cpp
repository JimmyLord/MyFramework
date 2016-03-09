//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

void SoundCue::OnDrag()
{
    g_DragAndDropStruct.m_Type = DragAndDropType_SoundCuePointer;
    g_DragAndDropStruct.m_Value = this;
}

SoundManager::SoundManager()
{
    m_SoundCuePool.AllocateObjects( NUM_SOUND_CUES_TO_POOL );

#if MYFW_USING_WX
    wxTreeItemId idroot = g_pPanelMemory->m_pTree_SoundCues->GetRootItem();
    g_pPanelMemory->SetSoundPanelCallbacks( idroot, this, SoundManager::StaticOnLeftClick, SoundManager::StaticOnRightClick, 0 );
#endif
}

SoundManager::~SoundManager()
{
}

SoundCue* SoundManager::CreateCue(const char* name)
{
    SoundCue* pCue = m_SoundCuePool.GetObject();

    strcpy_s( pCue->m_Name, MAX_SOUND_CUE_NAME_LEN, name );
    m_Cues.AddTail( pCue );

    g_pPanelMemory->AddSoundCue( pCue, "Default", name, SoundCue::StaticOnDrag );

    return pCue;
}

void SoundManager::AddSoundToCue(SoundCue* pCue, const char* fullpath)
{
    SoundObject* pSoundObject = g_pGameCore->m_pSoundPlayer->LoadSound( fullpath );
    pCue->m_SoundObjects.AddTail( pSoundObject );

    g_pPanelMemory->AddSoundObject( pSoundObject, pCue, fullpath, 0 );
}

SoundCue* SoundManager::FindCueByName(const char* name)
{
    for( CPPListNode* pNode = m_Cues.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        SoundCue* pCue = (SoundCue*)pNode;

        if( strcmp( pCue->m_Name, name ) == 0 )
        {
            return pCue;
        }
    }

    return 0;
}

int SoundManager::PlayCueByName(const char* name)
{
    SoundCue* pCue = FindCueByName( name );
    if( pCue )
    {
        return PlayCue( pCue );
    }

    return -1;
}

int SoundManager::PlayCue(SoundCue* pCue)
{
    MyAssert( pCue );

    SoundObject* pSoundObject = (SoundObject*)pCue->m_SoundObjects.GetHead();
    return g_pGameCore->m_pSoundPlayer->PlaySound( pSoundObject );
}

#if MYFW_USING_WX
void SoundManager::OnLeftClick(unsigned int count)
{
}

void SoundManager::OnRightClick(wxTreeItemId treeid)
{
 	wxMenu menu;
    menu.SetClientData( &m_WxEventHandler );

    m_WxEventHandler.m_pSoundManager = this;

    m_TreeIDRightClicked = treeid;

    menu.Append( SoundManagerWxEventHandler::RightClick_LoadSoundFile, "Load new sound" );
    menu.Append( SoundManagerWxEventHandler::RightClick_CreateNewCue, "Create new cue" );

    menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&SoundManagerWxEventHandler::OnPopupClick );

    // blocking call.
    g_pPanelWatch->PopupMenu( &menu ); // there's no reason this is using g_pPanelWatch other than convenience.
}

void SoundManagerWxEventHandler::OnPopupClick(wxEvent &evt)
{
    SoundManagerWxEventHandler* pEvtHandler = (SoundManagerWxEventHandler*)static_cast<wxMenu*>(evt.GetEventObject())->GetClientData();
    SoundManager* pSoundManager = pEvtHandler->m_pSoundManager;
    SoundCue* pSoundCue = pEvtHandler->m_pSoundCue;
    SoundObject* m_pSoundObject = pEvtHandler->m_pSoundObject;

    int id = evt.GetId();
    if( id == RightClick_LoadSoundFile )
    {
        // multiple select file open dialog
        wxFileDialog FileDialog( g_pMainApp->m_pMainFrame, _("Open Datafile"), "./Data", "", "Sound files(*.wav)|*.wav", wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_MULTIPLE );
    
        if( FileDialog.ShowModal() == wxID_CANCEL )
            return;
    
        // load the files chosen by the user
        // TODO: typecasting will likely cause issues with multibyte names
        wxArrayString patharray;
        FileDialog.GetPaths( patharray );

        char fullpath[MAX_PATH];
        for( unsigned int filenum=0; filenum<patharray.Count(); filenum++ )
        {
            sprintf_s( fullpath, MAX_PATH, "%s", (const char*)patharray[filenum] );

            // if the datafile is in our working directory, then load it... otherwise TODO: copy it in?
            const char* relativepath = GetRelativePath( fullpath );
            if( relativepath == 0 )
            {
                // File is not in our working directory.
                // TODO: copy the file into our data folder?
                LOGError( LOGTag, "file must be in working directory\n" );
                //MyAssert( false );
                return;
            }

            char filename[32];
            char extension[10];
            ParseFilename( fullpath, filename, 32, extension, 10 );
    
            SoundCue* pCue = pSoundManager->CreateCue( filename );
            g_pGameCore->m_pSoundManager->AddSoundToCue( pCue, relativepath );
        }
    }

    if( id == RightClick_CreateNewCue )
    {
        pSoundManager->CreateCue( "new cue" );
    }
}
#endif
