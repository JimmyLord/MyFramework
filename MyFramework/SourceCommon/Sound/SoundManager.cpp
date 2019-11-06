//
// Copyright (c) 2016-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "SoundManager.h"
#include "../Core/GameCore.h"
#include "../Helpers/FileManager.h"
#include "../Helpers/MyFileObject.h"
#include "../JSON/cJSONHelpers.h"

#include "../../SourceWindows/SoundPlayerXAudio.h" // TODO: Fix this dependency.

#if MYFW_EDITOR
#include "../../SourceEditor/EditorCommands.h"
#include "../../SourceEditor/CommandStack.h"
#endif

SoundCue::SoundCue()
{
    m_FullyLoaded = false;
    m_UnsavedChanges = false;

    m_Name[0] = '\0';
    m_pFile = nullptr;
    m_pSourcePool = nullptr;
}

SoundCue::~SoundCue()
{
}

void SoundCue::Release()
{
    RefCount::Release();

    // If removing the second to last ref, return it to the pool.
    if( m_RefCount == 1 )
    {
        m_pSourcePool->ReturnObjectToPool( this );

        m_FullyLoaded = false;
        m_UnsavedChanges = false;
        m_Name[0] = '\0';
        SAFE_RELEASE( m_pFile );
        m_pSourcePool = nullptr;

        for( unsigned int i=0; i<m_pSoundObjects.size(); i++ )
        {
            m_pSoundObjects[i]->Release();
        }

#if MYFW_EDITOR
        m_pSoundObjects.clear();
#else
        m_pSoundObjects.FreeAllInList();
#endif
    }
}

void SoundCue::ImportFromFile()
{
    MyAssert( m_pFile && m_pFile->GetFileLoadStatus() == FileLoadStatus_Success );
    if( m_pFile == nullptr || m_pFile->GetFileLoadStatus() != FileLoadStatus_Success )
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

#if !MYFW_EDITOR
            this->m_pSoundObjects.AllocateObjects( numwavs );
#endif            

            for( int i=0; i<numwavs; i++ )
            {
                cJSON* jSound = cJSON_GetArrayItem( jSoundArray, i );
                MyAssert( jSound != nullptr );
                if( jSound )
                {
                    cJSON* jPath = cJSON_GetObjectItem( jSound, "Path" );
                    MyAssert( jPath->valuestring[0] != '\0' );
                    if( jPath )
                    {
                        g_pGameCore->GetSoundManager()->AddSoundToCue( this, jPath->valuestring );
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

    if( strcmp( m_Name, name ) == 0 ) // Name hasn't changed.
        return;

    const char* newfilename = nullptr;
    if( m_pFile )
    {
        // If file rename fails, we'll keep the original name.
        newfilename = m_pFile->Rename( name );
    }

    strcpy_s( m_Name, MAX_SOUND_CUE_NAME_LEN, newfilename );
}

#if MYFW_EDITOR
void SoundCue::SaveSoundCue(const char* relativefolder)
{
    if( m_Name[0] == '\0' )
        return;

    m_FullyLoaded = true;
    m_UnsavedChanges = false;

    char filename[MAX_PATH];

    if( m_pFile != nullptr )
    {
        // If a file exists, use the existing file's fullpath.
        strcpy_s( filename, MAX_PATH, m_pFile->GetFullPath() );
    }
    else
    {
        // If a file doesn't exist, create the filename out of parts.
        // TODO: Move most of this block into generic system code.
        //MyAssert( relativepath != nullptr );
        if( relativefolder == nullptr )
            relativefolder = "Data/Audio";

        char workingdir[MAX_PATH];
#if MYFW_WINDOWS
        _getcwd( workingdir, MAX_PATH * sizeof(char) );
#else
        getcwd( workingdir, MAX_PATH * sizeof(char) );
#endif
        sprintf_s( filename, MAX_PATH, "%s/%s/", workingdir, relativefolder );
#if MYFW_WINDOWS
        CreateDirectoryA( filename, nullptr );
#else
        MyAssert( false );
#endif
        sprintf_s( filename, MAX_PATH, "%s/%s/%s.mycue", workingdir, relativefolder, m_Name );

        // This is a new file, check for filename conflict.
        {
            unsigned int count = 0;
            char newname[MAX_SOUND_CUE_NAME_LEN];
            strcpy_s( newname, MAX_SOUND_CUE_NAME_LEN, m_Name );
            while( FileManager::DoesFileExist( filename ) == true )
            {
                count++;

                sprintf_s( newname, MAX_SOUND_CUE_NAME_LEN, "%s(%d)", m_Name, count );
                sprintf_s( filename, MAX_PATH, "%s/%s/%s.mycue", workingdir, relativefolder, newname );
            }
            strcpy_s( m_Name, MAX_SOUND_CUE_NAME_LEN, newname );
        }
    }

    // Create the json string to save into the sound cue file.
    char* jsonstr = nullptr;
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

        // Dump sound cue json structure to disk.
        jsonstr = cJSON_Print( root );
        cJSON_Delete( root );
    }

    if( jsonstr != nullptr )
    {
        FILE* pFile = nullptr;
#if MYFW_WINDOWS
        fopen_s( &pFile, filename, "wb" );
#else
        pFile = fopen( filename, "wb" );
#endif
        if( pFile != nullptr )
        {
            fprintf( pFile, "%s", jsonstr );
            fclose( pFile );
        }
        else
        {
            LOGError( "File failed to open: %s\n", filename );
        }

        cJSONExt_free( jsonstr );

        // If the file managed to save, request it.
        if( m_pFile == nullptr )
        {
            sprintf_s( filename, MAX_PATH, "%s/%s.mycue", relativefolder, m_Name );
            m_pFile = g_pGameCore->GetManagers()->GetFileManager()->RequestFile( filename );
        }
    }
}

//    case RightClick_Unload:
//        {
//            SoundManager* pSoundManager = g_pGameCore->GetSoundManager();
//
//            MyAssert( pSoundCue && pSoundCue->GetRefCount() == pSoundManager->Editor_GetNumRefsPlacedOnSoundCueBySystem() );
//
//#if MYFW_EDITOR
//            std::vector<SoundCue*> cues;
//            cues.push_back( pSoundCue );
//            g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_UnloadSoundCues( cues ) );
//#else
//            pSoundManager->UnloadCue( pSoundCue );
//#endif
//        }
//        break;
//    }
#endif //MYFW_EDITOR

SoundManager::SoundManager(GameCore* pGameCore)
{
    m_pGameCore = pGameCore;

    m_SoundCuePool.AllocateObjects( NUM_SOUND_CUES_TO_POOL );
#if _DEBUG
    for( unsigned int i=0; i<m_SoundCuePool.Debug_GetLength(); i++ )
    {
        m_SoundCuePool[i].Debug_SetBaseCount( 1 ); // Assert refcount is 1 when returned to pool.
    }
#endif //_DEBUG

    m_pSoundCueCreatedCallbackList.AllocateObjects( MAX_REGISTERED_CALLBACKS );
    m_pSoundCueUnloadedCallbackList.AllocateObjects( MAX_REGISTERED_CALLBACKS );

#if MYFW_EDITOR
    // m_SoundCuePool is responsible for this object, holds 1st reference whether the SoundCue is in use or not.
    // SoundManager adds a reference when a SoundCue is taken from pool.
    // This number exists since MyEngine will add another ref when the soundcue is loaded
    //    and unloading can't happen if other objects are referencing it as well.
    m_NumRefsPlacedOnSoundCueBySystem = 2;
#endif //MYFW_EDITOR
}

SoundManager::~SoundManager()
{
    SoundCue* pNextCue;

    for( SoundCue* pCue = m_CuesStillLoading.GetHead(); pCue; pCue = pNextCue )
    {
        pNextCue = pCue->GetNext();
        pCue->Release();
    }

    for( SoundCue* pCue = m_Cues.GetHead(); pCue; pCue = pNextCue )
    {
        pNextCue = pCue->GetNext();
        pCue->Release();
    }

    m_pSoundCueCreatedCallbackList.FreeAllInList();
    m_pSoundCueUnloadedCallbackList.FreeAllInList();
}

void SoundManager::Tick()
{
    for( SoundCue* pCue = m_CuesStillLoading.GetHead(); pCue; pCue = pCue->GetNext() )
    {
        if( pCue->m_pFile && pCue->m_pFile->GetFileLoadStatus() == FileLoadStatus_Success )
        {
            pCue->ImportFromFile();
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
    if( pCue == nullptr )
    {
        LOGError( LOGTag, "SoundManager::GetCueFromPool(): Sound cue pool ran out of cues\n" );
        return nullptr;
    }

    pCue->AddRef(); // Add a ref if pulled from pool, assert refcount is 1 when returned to pool.
    pCue->m_pSourcePool = &m_SoundCuePool;

    return pCue;
}

SoundCue* SoundManager::CreateCue(const char* name)
{
    SoundCue* pCue = GetCueFromPool();
    if( pCue == nullptr )
        return nullptr;

    strcpy_s( pCue->m_Name, MAX_SOUND_CUE_NAME_LEN, name );
    m_Cues.AddTail( pCue );

    return pCue;
}

SoundCue* SoundManager::LoadCue(const char* fullpath)
{
    MyAssert( fullpath );

    SoundCue* pCue;

    // Check if this file was already loaded.
    pCue = FindCueByFilename( fullpath );
    if( pCue )
    {
        pCue->AddRef(); // Automatically add a ref for the calling code.
        return pCue;
    }

    pCue = GetCueFromPool(); // Ref added.
    if( pCue )
    {
        pCue->AddRef(); // Automatically add a ref for the calling code.
        pCue->m_pFile = m_pGameCore->GetManagers()->GetFileManager()->RequestFile( fullpath );

        m_CuesStillLoading.AddTail( pCue );
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
    }

    return pCue;
}

void SoundManager::UnloadCue(SoundCue* pCue)
{
    // Remove the cue from the cue list.
    pCue->Remove();

    // Release our reference to the cue.
    pCue->Release();

    for( unsigned int i=0; i<m_pSoundCueUnloadedCallbackList.Count(); i++ )
    {
        m_pSoundCueUnloadedCallbackList[i].pFunc( m_pSoundCueUnloadedCallbackList[i].pObj, pCue );
    }
}

void SoundManager::AddSoundToCue(SoundCue* pCue, const char* fullpath)
{
    // TODO: Check if the file was already loaded.
#if MYFW_NACL
    // TODO: fix
    SoundObject* pSoundObject = nullptr;
#else
    SoundObject* pSoundObject = m_pGameCore->GetSoundPlayer()->LoadSound( m_pGameCore->GetManagers()->GetFileManager(), fullpath );
#endif

    if( pSoundObject )
    {
        pSoundObject->AddRef();

#if MYFW_EDITOR
        pCue->m_pSoundObjects.push_back( pSoundObject );
#else
        pCue->m_pSoundObjects.Add( pSoundObject );
#endif //MYFW_EDITOR

        pCue->m_UnsavedChanges = true;
    }
}

void SoundManager::RemoveSoundFromCue(SoundCue* pCue, SoundObject* pSoundObject)
{
#if MYFW_EDITOR
    // Remove the sound object, but maintain the order of the list.
    pCue->m_pSoundObjects.erase( std::find( pCue->m_pSoundObjects.begin(), pCue->m_pSoundObjects.end(), pSoundObject ) );

    // Assert that there weren't two or more of the same sound object in the list.
    MyAssert( std::find( pCue->m_pSoundObjects.begin(), pCue->m_pSoundObjects.end(), pSoundObject ) == pCue->m_pSoundObjects.end() );
#else
    pCue->m_pSoundObjects.Remove_MaintainOrder( pSoundObject );
#endif

    pCue->m_UnsavedChanges = true;
}

SoundCue* SoundManager::FindCueByName(const char* name)
{
    // Name shouldn't be set until file is loaded.
    //for( SoundCue* pCue = m_CuesStillLoading.GetHead(); pCue; pCue = pCue->GetNext() )
    //{
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

    return nullptr;
}

SoundCue* SoundManager::FindCueByFilename(const char* fullpath)
{
    for( SoundCue* pCue = m_CuesStillLoading.GetHead(); pCue; pCue = pCue->GetNext() )
    {
        if( strcmp( pCue->m_pFile->GetFullPath(), fullpath ) == 0 )
        {
            return pCue;
        }
    }

    for( SoundCue* pCue = m_Cues.GetHead(); pCue; pCue = pCue->GetNext() )
    {
        if( strcmp( pCue->m_pFile->GetFullPath(), fullpath ) == 0 )
        {
            return pCue;
        }
    }

    return nullptr;
}

// Exposed to Lua, change elsewhere if function signature changes.
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
    return g_pGameCore->GetSoundPlayer()->PlaySound( pSoundObject );
}

void SoundManager::RegisterSoundCueCreatedCallback(void* pObj, SoundCueCallbackFunc* pCallback)
{
    MyAssert( pCallback != nullptr );
    MyAssert( m_pSoundCueCreatedCallbackList.Count() < (unsigned int)MAX_REGISTERED_CALLBACKS );

    SoundCueCallbackStruct callbackstruct;
    callbackstruct.pObj = pObj;
    callbackstruct.pFunc = pCallback;

    m_pSoundCueCreatedCallbackList.Add( callbackstruct );
}

void SoundManager::RegisterSoundCueUnloadedCallback(void* pObj, SoundCueCallbackFunc* pCallback)
{
    MyAssert( pCallback != nullptr );
    MyAssert( m_pSoundCueUnloadedCallbackList.Count() < (unsigned int)MAX_REGISTERED_CALLBACKS );

    SoundCueCallbackStruct callbackstruct;
    callbackstruct.pObj = pObj;
    callbackstruct.pFunc = pCallback;

    m_pSoundCueUnloadedCallbackList.Add( callbackstruct );
}

#if MYFW_EDITOR
void SoundManager::SaveAllCues(bool saveunchanged)
{
    for( SoundCue* pCue = m_Cues.GetHead(); pCue; pCue = pCue->GetNext() )
    {
        if( pCue->m_UnsavedChanges || saveunchanged )
        {
            pCue->SaveSoundCue( nullptr );
        }
    }
}

    //int id = evt.GetId();
    //if( id == RightClick_LoadSoundFile )
    //{
    //    // multiple select file open dialog
    //    wxFileDialog FileDialog( g_pMainApp->m_pMainFrame, _("Open Datafile"), "./Data/Audio", "", "Waves and Cues(*.wav;*.mycue)|*.wav;*.mycue", wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_MULTIPLE );

    //    if( FileDialog.ShowModal() == wxID_CANCEL )
    //        return;

    //    // load the files chosen by the user
    //    // TODO: typecasting will likely cause issues with multibyte names
    //    wxArrayString patharray;
    //    FileDialog.GetPaths( patharray );

    //    SoundCue* pNewCue = nullptr;

    //    char fullpath[MAX_PATH];
    //    for( unsigned int filenum=0; filenum<patharray.Count(); filenum++ )
    //    {
    //        sprintf_s( fullpath, MAX_PATH, "%s", (const char*)patharray[filenum] );

    //        // if the datafile is in our working directory, then load it... otherwise TODO: copy it in?
    //        const char* relativepath = GetRelativePath( fullpath );
    //        if( relativepath == nullptr )
    //        {
    //            // File is not in our working directory.
    //            // TODO: copy the file into our data folder?
    //            LOGError( LOGTag, "file must be in working directory\n" );
    //            //MyAssert( false );
    //            return;
    //        }

    //        char filename[32];
    //        char extension[10];
    //        ParseFilename( fullpath, filename, 32, extension, 10 );

    //        // Add all wav's selected to a new cue.
    //        if( strcmp( extension, ".wav" ) == 0 )
    //        {
    //            // Create a cue for the first wav selected.
    //            if( pNewCue == nullptr )
    //            {
    //                pNewCue = pSoundManager->CreateCue( filename );
    //            }

    //            // Add each wav.
    //            g_pGameCore->GetSoundManager()->AddSoundToCue( pNewCue, relativepath );
    //        }

    //        if( strcmp( extension, ".mycue" ) == 0 )
    //        {
    //            // TODO: load cue files.
    //        }
    //    }

    //    // If we made a new cue, save it and inform all observers.
    //    if( pNewCue )
    //    {
    //        pNewCue->SaveSoundCue( nullptr );

    //        for( unsigned int i=0; i<pSoundManager->m_pSoundCueCreatedCallbackList.Count(); i++ )
    //        {
    //            pSoundManager->m_pSoundCueCreatedCallbackList[i].pFunc( pSoundManager->m_pSoundCueCreatedCallbackList[i].pObj, pNewCue );
    //        }
    //    }
    //}

    //if( id == RightClick_CreateNewCue )
    //{
    //    SoundCue* pSoundCue = pSoundManager->CreateCue( "new cue" );

    //    for( unsigned int i=0; i<pSoundManager->m_pSoundCueCreatedCallbackList.Count(); i++ )
    //    {
    //        pSoundManager->m_pSoundCueCreatedCallbackList[i].pFunc( pSoundManager->m_pSoundCueCreatedCallbackList[i].pObj, pSoundCue );
    //    }
    //}

    //if( id == RightClick_RemoveSoundObjectFromCue )
    //{
    //    pSoundManager->RemoveSoundFromCue( pSoundCue, pSoundObject );
    //}
#endif //MYFW_EDITOR
