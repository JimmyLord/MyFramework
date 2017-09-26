//
// Copyright (c) 2016-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#if MYFW_USING_WX
#include "../../SourceWidgets/EditorCommands.h"
#include "../../SourceWidgets/CommandStack.h"
#endif

SoundCue::SoundCue()
{
    m_FullyLoaded = false;
    m_UnsavedChanges = false;

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

        m_FullyLoaded = false;
        m_UnsavedChanges = false;
        m_Name[0] = 0;
        SAFE_RELEASE( m_pFile );
        m_pSourcePool = 0;

        for( unsigned int i=0; i<m_pSoundObjects.size(); i++ )
        {
            m_pSoundObjects[i]->Release();
        }
#if MYFW_USING_WX
        m_pSoundObjects.clear();
#else
        m_pSoundObjects.FreeAllInList();
#endif
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

#if !MYFW_USING_WX
            this->m_pSoundObjects.AllocateObjects( numwavs );
#endif            

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
        m_UnsavedChanges = false;
    }

    cJSON_Delete( jRoot );
}

void SoundCue::SetName(const char* name)
{
    MyAssert( name );

    if( strcmp( m_Name, name ) == 0 ) // name hasn't changed.
        return;

    const char* newfilename = 0;
    if( m_pFile )
    {
        // if file rename fails, we'll keep the original name
        newfilename = m_pFile->Rename( name );
    }

    strcpy_s( m_Name, MAX_SOUND_CUE_NAME_LEN, newfilename );

#if MYFW_USING_WX
    if( g_pPanelMemory )
    {
        g_pPanelMemory->RenameObject( g_pPanelMemory->m_pTree_SoundCues, this, m_Name );
    }
#endif //MYFW_USING_WX
}

#if MYFW_USING_WX
void SoundCue::OnDrag()
{
    g_DragAndDropStruct.Add( DragAndDropType_SoundCuePointer, this );
}

void SoundCue::SaveSoundCue(const char* relativefolder)
{
    if( m_Name[0] == 0 )
        return;

    m_FullyLoaded = true;
    m_UnsavedChanges = false;

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

                sprintf_s( newname, MAX_SOUND_CUE_NAME_LEN, "%s(%d)", m_Name, count );
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
        for( unsigned int i=0; i<m_pSoundObjects.size(); i++ )
        {
            cJSON_AddItemToArray( jSoundArray, m_pSoundObjects[i]->ExportAsJSONObject() );
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

void SoundCue::OnLabelEdit(wxString newlabel)
{
    size_t len = newlabel.length();
    if( len > 0 )
    {
        SetName( newlabel );
    }
}

void SoundCue::OnLeftClick(unsigned int count)
{
}

void SoundCue::OnRightClick(wxTreeItemId treeid)
{
 	wxMenu menu;
    menu.SetClientData( &m_WxEventHandler );

    m_WxEventHandler.m_pSoundCue = this;

    m_TreeIDRightClicked = treeid;

    menu.Append( SoundCueWxEventHandler::RightClick_Rename, "Rename" );
    if( m_RefCount == g_pGameCore->m_pSoundManager->m_NumRefsPlacedOnSoundCueBySystem )
    {
        menu.Append( SoundCueWxEventHandler::RightClick_Unload, "Unload" );
    }
    else
    {
        wxMenuItem* pItem = menu.Append( SoundCueWxEventHandler::RightClick_Unload, "Can't unload, still in use" );
        pItem->Enable( false );
    }

    menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&SoundCueWxEventHandler::OnPopupClick );

    // blocking call.
    g_pPanelWatch->PopupMenu( &menu ); // there's no reason this is using g_pPanelWatch other than convenience.
}

void SoundCueWxEventHandler::OnPopupClick(wxEvent &evt)
{
    SoundCueWxEventHandler* pEvtHandler = (SoundCueWxEventHandler*)static_cast<wxMenu*>(evt.GetEventObject())->GetClientData();
    SoundCue* pSoundCue = pEvtHandler->m_pSoundCue;
    SoundObject* pSoundObject = pEvtHandler->m_pSoundObject;

    int id = evt.GetId();
    switch( id )
    {
    case RightClick_Rename:
        {
            wxTreeItemId treeid = g_pPanelMemory->FindSoundCue( pSoundCue );
            MyAssert( treeid == pSoundCue->m_TreeIDRightClicked );
            g_pPanelMemory->m_pTree_SoundCues->EditLabel( pSoundCue->m_TreeIDRightClicked );
        }
        break;

    case RightClick_Unload:
        {
            SoundManager* pSoundManager = g_pGameCore->m_pSoundManager;

            MyAssert( pSoundCue && pSoundCue->GetRefCount() == pSoundManager->m_NumRefsPlacedOnSoundCueBySystem );

#if MYFW_USING_WX
            // TODO: replace with call to editorcommand
            std::vector<SoundCue*> cues;
            cues.push_back( pSoundCue );
            g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_UnloadSoundCues( cues ) );
#else
            pSoundManager->UnloadCue( pSoundCue );
#endif
        }
        break;
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
    m_pSoundCueUnloadedCallbackList.AllocateObjects( MAX_REGISTERED_CALLBACKS );

#if MYFW_USING_WX
    // m_SoundCuePool is responsible for this object, holds 1st reference whether the SoundCue is in use or not.
    // SoundManager adds a reference when a SoundCue is taken from pool.
    // This number exists since MyEngine will add another ref when the soundcue is loaded
    //    and unloading can't happen if other objects are referencing it as well.
    m_NumRefsPlacedOnSoundCueBySystem = 2;

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
    m_pSoundCueUnloadedCallbackList.FreeAllInList();
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

            AddSoundCueToMemoryPanel( pCue );

            //g_pPanelMemory->SetLabelEditFunction( g_pPanelMemory->m_pTree_SoundCues, pCue, SoundCue::StaticOnLabelEdit );

            // Add right-click options to each cue "folder".
            //wxTreeItemId treeid = g_pPanelMemory->FindSoundCueCategory( foldername );
            //g_pPanelMemory->SetSoundCuePanelCallbacks( treeid, this, SoundManager::StaticOnLeftClick, SoundManager::StaticOnRightClick, SoundManager::StaticOnDrag );
#endif
        }

        if( pCue->IsFullyLoaded() )
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
    AddSoundCueToMemoryPanel( pCue );
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

    pCue = GetCueFromPool(); // Ref added
    if( pCue )
    {
        pCue->AddRef(); // Automatically add a ref for the calling code.
        pCue->m_pFile = g_pFileManager->RequestFile( fullpath );

        m_CuesStillLoading.AddTail( pCue );

#if MYFW_USING_WX
        wxTreeItemId idcue = g_pPanelMemory->AddSoundCue( pCue, "Loading", pCue->m_pFile->GetFilenameWithoutExtension(), SoundCue::StaticOnDrag );
        //g_pPanelMemory->SetLabelEditFunction( g_pPanelMemory->m_pTree_SoundCues, pCue, SoundCue::StaticOnLabelEdit );
#endif
    }

    return pCue;
}

SoundCue* SoundManager::LoadExistingCue(SoundCue* pCue)
{
    if( pCue )
    {
        pCue->AddRef();
        m_Cues.AddTail( pCue );

        for( unsigned int i=0; i<m_pSoundCueCreatedCallbackList.Count(); i++ )
        {
            m_pSoundCueCreatedCallbackList[i].pFunc( m_pSoundCueCreatedCallbackList[i].pObj, pCue );
        }

#if MYFW_USING_WX
        AddSoundCueToMemoryPanel( pCue );
#endif
    }

    return pCue;
}

void SoundManager::UnloadCue(SoundCue* pCue)
{
    // Remove the cue from the cue list.
    pCue->Remove();

    // Release our reference to the cue
    pCue->Release();

#if MYFW_USING_WX
    g_pPanelMemory->RemoveSoundCue( pCue );
#endif

    for( unsigned int i=0; i<m_pSoundCueUnloadedCallbackList.Count(); i++ )
    {
        m_pSoundCueUnloadedCallbackList[i].pFunc( m_pSoundCueUnloadedCallbackList[i].pObj, pCue );
    }
}

void SoundManager::AddSoundToCue(SoundCue* pCue, const char* fullpath)
{
    // TODO: check if the file was already loaded
#if MYFW_NACL
    // TODO: fix
    SoundObject* pSoundObject = 0;
#else
    SoundObject* pSoundObject = g_pGameCore->m_pSoundPlayer->LoadSound( fullpath );
#endif

    if( pSoundObject )
    {
        pSoundObject->AddRef();

#if MYFW_USING_WX
        pCue->m_pSoundObjects.push_back( pSoundObject );

        wxTreeItemId idsoundobject = g_pPanelMemory->AddSoundObject( pSoundObject, pCue, fullpath, 0 );
        g_pPanelMemory->SetSoundPanelCallbacks( idsoundobject, pSoundObject, SoundManager::StaticOnLeftClickSoundObject, SoundManager::StaticOnRightClickSoundObject, 0 );
        //g_pPanelMemory->SetLabelEditFunction( g_pPanelMemory->m_pTree_SoundCues, pSoundObject, SoundManager::StaticOnLabelEditSoundObject );
#else
        pCue->m_pSoundObjects.Add( pSoundObject );
#endif //MYFW_USING_WX

        pCue->m_UnsavedChanges = true;
    }
}

void SoundManager::RemoveSoundFromCue(SoundCue* pCue, SoundObject* pSoundObject)
{
#if MYFW_USING_WX
    g_pPanelMemory->RemoveSoundObject( pSoundObject );
#endif //MYFW_USING_WX

#if MYFW_USING_WX
    // remove the sound object, but maintain the order of the list
    for( unsigned int i=0; i<pCue->m_pSoundObjects.size(); i++ )
    {
        if( pCue->m_pSoundObjects[i] == pSoundObject )
        {
            for( ; i<pCue->m_pSoundObjects.size()-1; i++ )
            {
                pCue->m_pSoundObjects[i] = pCue->m_pSoundObjects[i+1];
            }
            pCue->m_pSoundObjects.pop_back();
            break;
        }
    }
#else
    pCue->m_pSoundObjects.Remove_MaintainOrder( pSoundObject );
#endif

    pCue->m_UnsavedChanges = true;
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
    MyAssert( pCue->m_pSoundObjects.size() != 0 );

    int randindex = rand()%pCue->m_pSoundObjects.size();

    SoundObject* pSoundObject = (SoundObject*)pCue->m_pSoundObjects[randindex];
    return g_pGameCore->m_pSoundPlayer->PlaySound( pSoundObject );
}

void SoundManager::RegisterSoundCueCreatedCallback(void* pObj, SoundCueCallbackFunc pCallback)
{
    MyAssert( pCallback != 0 );
    MyAssert( m_pSoundCueCreatedCallbackList.Count() < (unsigned int)MAX_REGISTERED_CALLBACKS );

    SoundCueCallbackStruct callbackstruct;
    callbackstruct.pObj = pObj;
    callbackstruct.pFunc = pCallback;

    m_pSoundCueCreatedCallbackList.Add( callbackstruct );
}

void SoundManager::RegisterSoundCueUnloadedCallback(void* pObj, SoundCueCallbackFunc pCallback)
{
    MyAssert( pCallback != 0 );
    MyAssert( m_pSoundCueUnloadedCallbackList.Count() < (unsigned int)MAX_REGISTERED_CALLBACKS );

    SoundCueCallbackStruct callbackstruct;
    callbackstruct.pObj = pObj;
    callbackstruct.pFunc = pCallback;

    m_pSoundCueUnloadedCallbackList.Add( callbackstruct );
}

#if MYFW_USING_WX
void SoundManager::SaveAllCues(bool saveunchanged)
{
    for( CPPListNode* pNode = m_Cues.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        SoundCue* pCue = (SoundCue*)pNode;

        if( pCue->m_UnsavedChanges || saveunchanged )
        {
            pCue->SaveSoundCue( 0 );
        }
    }
}

void SoundManager::AddSoundCueToMemoryPanel(SoundCue* pCue)
{
    // Add the sound cue to the memory panel.
    g_pPanelMemory->RemoveSoundCue( pCue );
    wxTreeItemId idcue = g_pPanelMemory->AddSoundCue( pCue, "Default", pCue->m_Name, SoundCue::StaticOnDrag );
    g_pPanelMemory->SetSoundPanelCallbacks( idcue, pCue, SoundCue::StaticOnLeftClick, SoundCue::StaticOnRightClick, SoundCue::StaticOnDrag );
    g_pPanelMemory->SetLabelEditFunction( g_pPanelMemory->m_pTree_SoundCues, pCue, SoundCue::StaticOnLabelEdit );

    // Add all of it's sound objects to the memory panel.
    for( unsigned int i=0; i<pCue->m_pSoundObjects.size(); i++ )
    {
        SoundObject* pSoundObject = pCue->m_pSoundObjects[i];
        wxTreeItemId idsoundobject = g_pPanelMemory->AddSoundObject( pSoundObject, pCue, pSoundObject->GetFullPath(), 0 );
        g_pPanelMemory->SetSoundPanelCallbacks( idsoundobject, pSoundObject, SoundManager::StaticOnLeftClickSoundObject, SoundManager::StaticOnRightClickSoundObject, 0 );
        //g_pPanelMemory->SetLabelEditFunction( g_pPanelMemory->m_pTree_SoundCues, pSoundObject, SoundManager::StaticOnLabelEditSoundObject );
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

void SoundManager::OnLeftClickSoundObject(unsigned int count)
{
}

void SoundManager::OnRightClickSoundObject(wxTreeItemId treeid)
{
    TreeItemDataGenericObjectInfo* pData = 0;

    wxMenu menu;
    menu.SetClientData( &m_WxEventHandler );

    m_WxEventHandler.m_pSoundManager = this;

    // Get the SoundObject pointer from the tree
    pData = (TreeItemDataGenericObjectInfo*)g_pPanelMemory->m_pTree_SoundCues->GetItemData( treeid );
    m_WxEventHandler.m_pSoundObject = (SoundObject*)pData->m_pObject;

    // Get the SoundCue pointer from the tree, should be the parent of treeid
    wxTreeItemId parentid = g_pPanelMemory->m_pTree_SoundCues->GetItemParent( treeid );
    pData = (TreeItemDataGenericObjectInfo*)g_pPanelMemory->m_pTree_SoundCues->GetItemData( parentid );
    m_WxEventHandler.m_pSoundCue = (SoundCue*)pData->m_pObject;

    m_TreeIDRightClicked = treeid;

    menu.Append( SoundManagerWxEventHandler::RightClick_RemoveSoundObjectFromCue, "Remove from cue" );

    menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&SoundManagerWxEventHandler::OnPopupClick );

    // blocking call.
    g_pPanelWatch->PopupMenu( &menu ); // there's no reason this is using g_pPanelWatch other than convenience.
}

void SoundManagerWxEventHandler::OnPopupClick(wxEvent &evt)
{
    SoundManagerWxEventHandler* pEvtHandler = (SoundManagerWxEventHandler*)static_cast<wxMenu*>(evt.GetEventObject())->GetClientData();
    SoundManager* pSoundManager = pEvtHandler->m_pSoundManager;
    SoundCue* pSoundCue = pEvtHandler->m_pSoundCue;
    SoundObject* pSoundObject = pEvtHandler->m_pSoundObject;

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
            {
                pSoundManager->m_pSoundCueCreatedCallbackList[i].pFunc( pSoundManager->m_pSoundCueCreatedCallbackList[i].pObj, pNewCue );
            }
        }
    }

    if( id == RightClick_CreateNewCue )
    {
        SoundCue* pSoundCue = pSoundManager->CreateCue( "new cue" );

        for( unsigned int i=0; i<pSoundManager->m_pSoundCueCreatedCallbackList.Count(); i++ )
        {
            pSoundManager->m_pSoundCueCreatedCallbackList[i].pFunc( pSoundManager->m_pSoundCueCreatedCallbackList[i].pObj, pSoundCue );
        }
    }

    if( id == RightClick_RemoveSoundObjectFromCue )
    {
        pSoundManager->RemoveSoundFromCue( pSoundCue, pSoundObject );
    }
}
#endif
