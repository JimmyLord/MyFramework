//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

SoundCue::SoundCue()
{
    m_Name[0] = 0;
    m_pFile = 0;
    m_pSourcePool = 0;
}

SoundCue::~SoundCue()
{
}

void SoundCue::Release()
{
    RefCount::Release();

    // if removing the second to last ref, return it to the pool.
    if( m_RefCount == 1 )
    {
        m_pSourcePool->ReturnObject( this );

        m_Name[0] = 0;
        m_pFile = 0;
        m_pSourcePool = 0;
    }
}

void SoundCue::OnDrag()
{
    g_DragAndDropStruct.m_Type = DragAndDropType_SoundCuePointer;
    g_DragAndDropStruct.m_Value = this;
}

void SoundCue::SaveSoundCue(const char* relativepath)
{
    if( m_Name[0] == 0 )
        return;

    char filename[MAX_PATH];

    if( m_pFile != 0 )
    {
        // if a file exists, use the existing file's fullpath
        strcpy_s( filename, MAX_PATH, m_pFile->m_FullPath );
    }
    else
    {
        // if a file doesn't exist, create the filename out of parts.
        // TODO: move most of this block into generic system code.
        //MyAssert( relativepath != 0 );
        if( relativepath == 0 )
            relativepath = "Data/Audio";

        char workingdir[MAX_PATH];
#if MYFW_WINDOWS
        _getcwd( workingdir, MAX_PATH * sizeof(char) );
#else
        getcwd( workingdir, MAX_PATH * sizeof(char) );
#endif
        sprintf_s( filename, MAX_PATH, "%s/%s/", workingdir, relativepath );
#if MYFW_WINDOWS
        CreateDirectoryA( filename, 0 );
#else
        MyAssert( false );
#endif
        sprintf_s( filename, MAX_PATH, "%s/%s/%s.mycue", workingdir, relativepath, m_Name );

        // this is a new file, check for filename conflict
        {
            unsigned int count = 0;
            char newname[MAX_SOUND_CUE_NAME_LEN];
            strcpy_s( newname, MAX_SOUND_CUE_NAME_LEN, m_Name );
            while( g_pFileManager->DoesFileExist( filename ) == true )
            {
                count++;

                sprintf_s( newname, "%s(%d)", m_Name, count );
                sprintf_s( filename, MAX_PATH, "%s/%s/%s.mycue", workingdir, relativepath, newname );
            }
            strcpy_s( m_Name, MAX_SOUND_CUE_NAME_LEN, newname );
        }
    }

    // Create the json string to save into the sound cue file
    char* jsonstr = 0;
    {
        cJSON* root = cJSON_CreateObject();

        cJSON* jCue = cJSON_CreateObject();
        cJSON_AddItemToObject( root, "Cue", jCue );

        cJSON_AddStringToObject( jCue, "Name", m_Name );

        cJSON* jSoundArray = cJSON_CreateArray();
        for( CPPListNode* pNode = m_SoundObjects.GetHead(); pNode; pNode = pNode->GetNext() )
        {
            SoundObject* pSound = (SoundObject*)pNode;
            cJSON_AddItemToArray( jSoundArray, pSound->ExportAsJSONObject() );
        }

        cJSON_AddItemToObject( jCue, "Sounds", jSoundArray );

        // dump sound cue json structure to disk
        jsonstr = cJSON_Print( root );
        cJSON_Delete( root );
    }

    if( jsonstr != 0 )
    {
        FILE* pFile = 0;
#if MYFW_WINDOWS
        fopen_s( &pFile, filename, "wb" );
#else
        pFile = fopen( filename, "wb" );
#endif
        if( pFile )
        {
            fprintf( pFile, "%s", jsonstr );
            fclose( pFile );
        }

        cJSONExt_free( jsonstr );

        // if the file managed to save, request it.
        if( m_pFile == 0 )
        {
            sprintf_s( filename, MAX_PATH, "%s/%s.mycue", relativepath, m_Name );
            m_pFile = g_pFileManager->RequestFile( filename );
        }
    }
}

SoundManager::SoundManager()
{
    m_SoundCuePool.AllocateObjects( NUM_SOUND_CUES_TO_POOL );
#if _DEBUG
    for( unsigned int i=0; i<m_SoundCuePool.Debug_GetLength(); i++ )
    {
        m_SoundCuePool[i].Debug_SetBaseCount( 1 ); // assert refcount is 1 when returned to pool
    }
#endif //_DEBUG

    m_pSoundCueCreatedCallbackList.AllocateObjects( MAX_REGISTERED_CALLBACKS );

#if MYFW_USING_WX
    wxTreeItemId idroot = g_pPanelMemory->m_pTree_SoundCues->GetRootItem();
    g_pPanelMemory->SetSoundPanelCallbacks( idroot, this, SoundManager::StaticOnLeftClick, SoundManager::StaticOnRightClick, 0 );
#endif
}

SoundManager::~SoundManager()
{
    m_pSoundCueCreatedCallbackList.FreeAllInList();
}

SoundCue* SoundManager::GetCueFromPool()
{
    SoundCue* pCue = m_SoundCuePool.GetObject();
    if( pCue == 0 )
    {
        LOGError( LOGTag, "SoundManager::GetCueFromPool(): Sound cue pool ran out of cues\n" );
        return 0;
    }

    pCue->AddRef(); // add a ref if pulled from pool, assert refcount is 1 when returned to pool
    pCue->m_pSourcePool = &m_SoundCuePool;

    return pCue;
}

SoundCue* SoundManager::CreateCue(const char* name)
{
    SoundCue* pCue = GetCueFromPool();
    if( pCue == 0 )
        return 0;

    strcpy_s( pCue->m_Name, MAX_SOUND_CUE_NAME_LEN, name );
    m_Cues.AddTail( pCue );

    g_pPanelMemory->AddSoundCue( pCue, "Default", name, SoundCue::StaticOnDrag );

    return pCue;
}

SoundCue* SoundManager::LoadCue(const char* fullpath)
{
    MyAssert( fullpath );

    SoundCue* pCue;

    // check if this file was already loaded.
    pCue = FindCueByFilename( fullpath );
    if( pCue )
    {
        pCue->AddRef();
        return pCue;
    }

    pCue = GetCueFromPool();
    if( pCue )
    {
        pCue->m_pFile = g_pFileManager->RequestFile( fullpath );

#if MYFW_USING_WX
        //g_pPanelMemory->AddSoundCue( pSoundCue, "Loading", pSoundCue->m_pFile->m_FilenameWithoutExtension, SoundCue::StaticOnLeftClick, SoundCue::StaticOnRightClick, SoundCue::StaticOnDrag );
        //g_pPanelMemory->SetLabelEditFunction( g_pPanelMemory->m_pTree_SoundCues, pSoundCue, SoundCue::StaticOnLabelEdit );
#endif
    }

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

SoundCue* SoundManager::FindCueByFilename(const char* fullpath)
{
    for( CPPListNode* pNode = m_Cues.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        SoundCue* pCue = (SoundCue*)pNode;

        if( strcmp( pCue->m_pFile->m_FullPath, fullpath ) == 0 )
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

void SoundManager::RegisterSoundCueCreatedCallback(void* pObj, SoundCueCreatedCallbackFunc pCallback)
{
    MyAssert( pCallback != 0 );
    MyAssert( m_pSoundCueCreatedCallbackList.Count() < MAX_REGISTERED_CALLBACKS );

    SoundCueCreatedCallbackStruct callbackstruct;
    callbackstruct.pObj = pObj;
    callbackstruct.pFunc = pCallback;

    m_pSoundCueCreatedCallbackList.Add( callbackstruct );
}

#if MYFW_USING_WX
void SoundManager::SaveAllCues(bool saveunchanged)
{
    for( CPPListNode* pNode = m_Cues.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        SoundCue* pCue = (SoundCue*)pNode;

        //if( pCue->m_UnsavedChanges || saveunchanged )
        {
            pCue->SaveSoundCue( 0 );
        }
    }
}

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

            for( unsigned int i=0; i<pSoundManager->m_pSoundCueCreatedCallbackList.Count(); i++ )
                pSoundManager->m_pSoundCueCreatedCallbackList[i].pFunc( pSoundManager->m_pSoundCueCreatedCallbackList[i].pObj, pSoundCue );
        }
    }

    if( id == RightClick_CreateNewCue )
    {
        pSoundManager->CreateCue( "new cue" );

        for( unsigned int i=0; i<pSoundManager->m_pSoundCueCreatedCallbackList.Count(); i++ )
            pSoundManager->m_pSoundCueCreatedCallbackList[i].pFunc( pSoundManager->m_pSoundCueCreatedCallbackList[i].pObj, pSoundCue );
    }
}
#endif
