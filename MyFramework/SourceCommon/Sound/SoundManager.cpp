//
// Copyright (c) 2016-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

SoundCue::SoundCue()
{
    m_FullyLoaded = false;

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
        m_pSourcePool->ReturnObjectToPool( this );
        SAFE_RELEASE( m_pFile );

        m_FullyLoaded = false;
        m_Name[0] = 0;
        m_pSourcePool = 0;
    }
}

void SoundCue::ImportFromFile()
{
    MyAssert( m_pFile && m_pFile->GetFileLoadStatus() == FileLoadStatus_Success );
    if( m_pFile == 0 || m_pFile->GetFileLoadStatus() != FileLoadStatus_Success )
        return;

    cJSON* jRoot = cJSON_Parse( m_pFile->GetBuffer() );

    cJSON* jCue = cJSON_GetObjectItem( jRoot, "Cue" );
    if( jCue )
    {
        cJSONExt_GetString( jCue, "Name", m_Name, MAX_SOUND_CUE_NAME_LEN );

        cJSON* jSoundArray = cJSON_GetObjectItem( jCue, "Sounds" );
        if( jSoundArray )
        {
            int numwavs = cJSON_GetArraySize( jSoundArray );

            for( int i=0; i<numwavs; i++ )
            {
                cJSON* jSound = cJSON_GetArrayItem( jSoundArray, i );
                MyAssert( jSound != 0 );
                if( jSound )
                {
                    cJSON* jPath = cJSON_GetObjectItem( jSound, "Path" );
                    MyAssert( jPath->valuestring[0] != 0 );
                    if( jPath )
                    {
                        g_pGameCore->m_pSoundManager->AddSoundToCue( this, jPath->valuestring );
                    }
                }
            }
        }

        m_FullyLoaded = true;
    }

    cJSON_Delete( jRoot );
}

#if MYFW_USING_WX
void SoundCue::OnDrag()
{
    g_DragAndDropStruct.m_Type = DragAndDropType_SoundCuePointer;
    g_DragAndDropStruct.m_Value = this;
}

void SoundCue::SaveSoundCue(const char* relativefolder)
{
    if( m_Name[0] == 0 )
        return;

    m_FullyLoaded = true;

    char filename[MAX_PATH];

    if( m_pFile != 0 )
    {
        // if a file exists, use the existing file's fullpath
        strcpy_s( filename, MAX_PATH, m_pFile->GetFullPath() );
    }
    else
    {
        // if a file doesn't exist, create the filename out of parts.
        // TODO: move most of this block into generic system code.
        //MyAssert( relativepath != 0 );
        if( relativefolder == 0 )
            relativefolder = "Data/Audio";

        char workingdir[MAX_PATH];
#if MYFW_WINDOWS
        _getcwd( workingdir, MAX_PATH * sizeof(char) );
#else
        getcwd( workingdir, MAX_PATH * sizeof(char) );
#endif
        sprintf_s( filename, MAX_PATH, "%s/%s/", workingdir, relativefolder );
#if MYFW_WINDOWS
        CreateDirectoryA( filename, 0 );
#else
        MyAssert( false );
#endif
        sprintf_s( filename, MAX_PATH, "%s/%s/%s.mycue", workingdir, relativefolder, m_Name );

        // this is a new file, check for filename conflict
        {
            unsigned int count = 0;
            char newname[MAX_SOUND_CUE_NAME_LEN];
            strcpy_s( newname, MAX_SOUND_CUE_NAME_LEN, m_Name );
            while( g_pFileManager->DoesFileExist( filename ) == true )
            {
                count++;

                sprintf_s( newname, "%s(%d)", m_Name, count );
                sprintf_s( filename, MAX_PATH, "%s/%s/%s.mycue", workingdir, relativefolder, newname );
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
            sprintf_s( filename, MAX_PATH, "%s/%s.mycue", relativefolder, m_Name );
            m_pFile = g_pFileManager->RequestFile( filename );
        }
    }
}
#endif //MYFW_USING_WX

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
    CPPListNode* pNextNode;

    for( CPPListNode* pNode = m_CuesStillLoading.GetHead(); pNode; pNode = pNextNode )
    {
        pNextNode = pNode->GetNext();
        ((SoundCue*)pNode)->Release();
    }

    for( CPPListNode* pNode = m_Cues.GetHead(); pNode; pNode = pNextNode )
    {
        pNextNode = pNode->GetNext();
        ((SoundCue*)pNode)->Release();
    }

    m_pSoundCueCreatedCallbackList.FreeAllInList();
}

void SoundManager::Tick()
{
    for( CPPListNode* pNode = m_CuesStillLoading.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        SoundCue* pCue = (SoundCue*)pNode;

        if( pCue->m_pFile && pCue->m_pFile->GetFileLoadStatus() == FileLoadStatus_Success )
        {
            pCue->ImportFromFile();

#if MYFW_USING_WX
            //const char* foldername = "Unknown";
            //if( pCue->m_pFile )
            //    foldername = pCue->m_pFile->GetNameOfDeepestFolderPath();

            g_pPanelMemory->RemoveSoundCue( pCue );
            g_pPanelMemory->AddSoundCue( pCue, "Default", pCue->m_Name, SoundCue::StaticOnDrag );

            // Add all the sounds to the tree.
            for( CPPListNode* pSoundNode = pCue->m_SoundObjects.GetHead(); pSoundNode; pSoundNode = pSoundNode->GetNext() )
            {
                SoundObject* pSoundObject = (SoundObject*)pSoundNode;
                g_pPanelMemory->AddSoundObject( pSoundObject, pCue, pSoundObject->m_FullPath, 0 );
            }

            //g_pPanelMemory->SetLabelEditFunction( g_pPanelMemory->m_pTree_SoundCues, pCue, SoundCue::StaticOnLabelEdit );

            // Add right-click options to each cue "folder".
            //wxTreeItemId treeid = g_pPanelMemory->FindSoundCueCategory( foldername );
            //g_pPanelMemory->SetSoundCuePanelCallbacks( treeid, this, SoundManager::StaticOnLeftClick, SoundManager::StaticOnRightClick, SoundManager::StaticOnDrag );
#endif
        }

        if( pCue->m_FullyLoaded )
        {
            m_Cues.MoveTail( pCue );
        }
    }
}

SoundCue* SoundManager::GetCueFromPool()
{
    SoundCue* pCue = m_SoundCuePool.GetObjectFromPool();
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

#if MYFW_USING_WX
    g_pPanelMemory->AddSoundCue( pCue, "Default", name, SoundCue::StaticOnDrag );
#endif //MYFW_USING_WX

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
        pCue->AddRef(); // Automatically add a ref for the calling code.
        return pCue;
    }

    pCue = GetCueFromPool();
    if( pCue )
    {
        pCue->AddRef(); // Automatically add a ref for the calling code.
        pCue->m_pFile = g_pFileManager->RequestFile( fullpath );

        m_CuesStillLoading.AddTail( pCue );

#if MYFW_USING_WX
        g_pPanelMemory->AddSoundCue( pCue, "Loading", pCue->m_pFile->GetFilenameWithoutExtension(), SoundCue::StaticOnDrag );
        //g_pPanelMemory->SetLabelEditFunction( g_pPanelMemory->m_pTree_SoundCues, pCue, SoundCue::StaticOnLabelEdit );
#endif
    }

    return pCue;
}

void SoundManager::AddSoundToCue(SoundCue* pCue, const char* fullpath)
{
#if MYFW_NACL
    // TODO: fix
#else
    SoundObject* pSoundObject = g_pGameCore->m_pSoundPlayer->LoadSound( fullpath );
    pCue->m_SoundObjects.AddTail( pSoundObject );
#endif

#if MYFW_USING_WX
    g_pPanelMemory->AddSoundObject( pSoundObject, pCue, fullpath, 0 );
#endif //MYFW_USING_WX
}

SoundCue* SoundManager::FindCueByName(const char* name)
{
    // name shouldn't be set until file is loaded
    //for( CPPListNode* pNode = m_CuesStillLoading.GetHead(); pNode; pNode = pNode->GetNext() )
    //{
    //    SoundCue* pCue = (SoundCue*)pNode;

    //    if( strcmp( pCue->m_Name, name ) == 0 )
    //    {
    //        return pCue;
    //    }
    //}

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
    for( CPPListNode* pNode = m_CuesStillLoading.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        SoundCue* pCue = (SoundCue*)pNode;

        if( strcmp( pCue->m_pFile->GetFullPath(), fullpath ) == 0 )
        {
            return pCue;
        }
    }

    for( CPPListNode* pNode = m_Cues.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        SoundCue* pCue = (SoundCue*)pNode;

        if( strcmp( pCue->m_pFile->GetFullPath(), fullpath ) == 0 )
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
    MyAssert( m_pSoundCueCreatedCallbackList.Count() < (unsigned int)MAX_REGISTERED_CALLBACKS );

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
    SoundObject* m_pSoundObject = pEvtHandler->m_pSoundObject;

    int id = evt.GetId();
    if( id == RightClick_LoadSoundFile )
    {
        // multiple select file open dialog
        wxFileDialog FileDialog( g_pMainApp->m_pMainFrame, _("Open Datafile"), "./Data/Audio", "", "Waves and Cues(*.wav;*.mycue)|*.wav;*.mycue", wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_MULTIPLE );

        if( FileDialog.ShowModal() == wxID_CANCEL )
            return;

        // load the files chosen by the user
        // TODO: typecasting will likely cause issues with multibyte names
        wxArrayString patharray;
        FileDialog.GetPaths( patharray );

        SoundCue* pNewCue = 0;

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

            // Add all wav's selected to a new cue.
            if( strcmp( extension, ".wav" ) == 0 )
            {
                // Create a cue for the first wav selected.
                if( pNewCue == 0 )
                {
                    pNewCue = pSoundManager->CreateCue( filename );
                }

                // Add each wav.
                g_pGameCore->m_pSoundManager->AddSoundToCue( pNewCue, relativepath );
            }

            if( strcmp( extension, ".mycue" ) == 0 )
            {
                // TODO: load cue files
            }
        }

        // If we made a new cue, save it and inform all observers.
        if( pNewCue )
        {
            pNewCue->SaveSoundCue( 0 );

            for( unsigned int i=0; i<pSoundManager->m_pSoundCueCreatedCallbackList.Count(); i++ )
                pSoundManager->m_pSoundCueCreatedCallbackList[i].pFunc( pSoundManager->m_pSoundCueCreatedCallbackList[i].pObj, pNewCue );
        }
    }

    if( id == RightClick_CreateNewCue )
    {
        SoundCue* pSoundCue = pEvtHandler->m_pSoundCue;
        pSoundCue = pSoundManager->CreateCue( "new cue" );

        for( unsigned int i=0; i<pSoundManager->m_pSoundCueCreatedCallbackList.Count(); i++ )
        {
            pSoundManager->m_pSoundCueCreatedCallbackList[i].pFunc( pSoundManager->m_pSoundCueCreatedCallbackList[i].pObj, pSoundCue );
        }
    }
}
#endif
