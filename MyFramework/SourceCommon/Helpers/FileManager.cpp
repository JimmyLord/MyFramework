//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include "FileManager.h"

#pragma warning (disable : 4996)
#include "../../../LodePNG/lodepng.h"
#pragma warning (default : 4996)

char* PlatformSpecific_LoadFile(const char* filename, int* length = 0, const char* file = __FILE__, unsigned long line = __LINE__);

FileManager* g_pFileManager = 0;

FileManager::FileManager()
{
}

FileManager::~FileManager()
{
    assert( m_FilesLoaded.GetHead() == 0 );
    assert( m_FilesStillLoading.GetHead() == 0 );
    //FreeAllFiles();
}

void FileManager::FreeFile(MyFileObject* pFile)
{
    assert( pFile );
    pFile->Release(); // file's are refcounted, so release a reference to it.
}

//void FileManager::FreeAllFiles()
//{
//    // TODO: remove this function... why did I put this here..
//    assert( m_FilesLoaded.GetHead() == 0 );
//    assert( m_FilesStillLoading.GetHead() == 0 );
//}

unsigned int FileManager::CalculateTotalMemoryUsedByFiles()
{
    unsigned int totalsize = 0;

    CPPListNode* pNextNode;
    for( CPPListNode* pNode = m_FilesLoaded.GetHead(); pNode != 0; pNode = pNextNode )
    {
        pNextNode = pNode->GetNext();

        MyFileObject* pFile = (MyFileObject*)pNode;

        totalsize += pFile->m_FileLength;
    }

    for( CPPListNode* pNode = m_FilesStillLoading.GetHead(); pNode != 0; pNode = pNextNode )
    {
        pNextNode = pNode->GetNext();

        MyFileObject* pFile = (MyFileObject*)pNode;

        totalsize += pFile->m_FileLength;
    }

    return totalsize;
}

MyFileObject* FileManager::RequestFile(const char* filename)
{
    MyFileObject* pFile;
    
    // check if the file has already been requested... might still be loading.
    pFile = FindFileByName( filename );
    if( pFile )
    {
        pFile->AddRef();
        return pFile;
    }

    // if the file wasn't already loaded create a new one and load it up.
    pFile = MyNew MyFileObject;
    pFile->RequestFile( filename );

    m_FilesStillLoading.AddTail( pFile );

    return pFile;
}

void FileManager::ReloadFile(MyFileObject* pFile)
{
    assert( pFile );

    pFile->UnloadContents();
    m_FilesStillLoading.MoveTail( pFile );
}

MyFileObject* FileManager::FindFileByName(const char* filename)
{
    for( CPPListNode* pNode = m_FilesLoaded.GetHead(); pNode != 0; pNode = pNode->GetNext() )
    {
        MyFileObject* pFile = (MyFileObject*)pNode;

        if( strcmp( filename, pFile->m_FullPath ) == 0 )
            return pFile;
    }

    for( CPPListNode* pNode = m_FilesStillLoading.GetHead(); pNode != 0; pNode = pNode->GetNext() )
    {
        MyFileObject* pFile = (MyFileObject*)pNode;

        if( strcmp( filename, pFile->m_FullPath ) == 0 )
            return pFile;
    }

    return 0;
}

void FileManager::Tick()
{
    // continue to tick any files still in the "loading" queue.
    CPPListNode* pNextNode;
    for( CPPListNode* pNode = m_FilesStillLoading.GetHead(); pNode != 0; pNode = pNextNode )
    {
        pNextNode = pNode->GetNext();

        MyFileObject* pFile = (MyFileObject*)pNode;
        LOGInfo( LOGTag, "Loading File: %s\n", pFile->m_FullPath );

        // sanity check, make sure file isn't already loaded.
        assert( pFile->m_FileReady == false );

        pFile->Tick();

        // if we're done loading, move the file into the loaded list.
        if( pFile->m_FileReady )
        {
            m_FilesLoaded.MoveTail( pFile );

#if MYFW_USING_WX
            g_pPanelMemory->AddFile( pFile, "Global", pFile->m_FullPath, MyFileObject::StaticOnDrag );
#endif
        }
    }
}

MyFileObject::MyFileObject()
{
    m_FullPath = 0;
    m_FilenameWithoutExtension = 0;
    m_ExtensionWithDot = 0;
    m_LoadFailed = false;
    m_FileReady = false;
    m_FileLength = 0;
    m_pBuffer = 0;
    m_BytesRead = 0;

    m_Hack_TicksToWaitUntilWeActuallyLoadToSimulateAsyncLoading = 2;
}

MyFileObject::~MyFileObject()
{
    // make sure you call ->Release.  don't delete a file object, it's refcounted.
#if MYFW_WINDOWS
    assert( Prev );
#endif
    if( Prev ) // if it's in a list... it isn't on some? platforms ATM, need to update file loaders on each.
        Remove();

    SAFE_DELETE_ARRAY( m_FullPath );
    SAFE_DELETE_ARRAY( m_FilenameWithoutExtension );
    SAFE_DELETE_ARRAY( m_ExtensionWithDot );
    SAFE_DELETE_ARRAY( m_pBuffer );

#if MYFW_USING_WX
    if( g_pPanelMemory )
        g_pPanelMemory->RemoveFile( this );
#endif
}

#if MYFW_USING_WX
void MyFileObject::OnDrag()
{
    g_DragAndDropStruct.m_Type = DragAndDropType_FileObjectPointer;
    g_DragAndDropStruct.m_Value = this;
}
#endif //MYFW_USING_WX

void MyFileObject::RequestFile(const char* filename)
{
    assert( filename != 0 );
    if( filename == 0 )
        return;

    LOGInfo( LOGTag, "RequestFile %s\n", filename );

    int len = (int)strlen( filename );
    assert( len > 0 );
    if( len <= 0 )
        return;

    m_FullPath = MyNew char[len+1];
    strcpy_s( m_FullPath, len+1, filename );

    int extensionstartlocation = len;
    {
        while( extensionstartlocation > 0 )
        {
            if( filename[extensionstartlocation] == '.' )
            {
                int extensionlen = len-extensionstartlocation;
                m_ExtensionWithDot = MyNew char[extensionlen+1];
                strncpy_s( m_ExtensionWithDot, extensionlen+1, &filename[extensionstartlocation], extensionlen );
                m_ExtensionWithDot[extensionlen] = 0;
                break;
            }
            extensionstartlocation--;
        }

        if( m_ExtensionWithDot == 0 )
        {
            m_ExtensionWithDot = MyNew char[2];
            m_ExtensionWithDot[0] = '.';
            m_ExtensionWithDot[1] = 0;

            extensionstartlocation = len;
        }
    }

    {
        int i = extensionstartlocation;
        while( i >= 0 )
        {
            if( i == 0 || filename[i] == '/' || filename[i] == '\\' )
            {
                i++;
                int namelen = extensionstartlocation-i;
                m_FilenameWithoutExtension = MyNew char[namelen+1];
                strncpy_s( m_FilenameWithoutExtension, namelen+1, &filename[i], namelen );
                m_FilenameWithoutExtension[namelen] = 0;
                break;
            }
            i--;
        }
    }
}

void MyFileObject::Tick()
{
    if( m_FileReady == false && m_Hack_TicksToWaitUntilWeActuallyLoadToSimulateAsyncLoading == 0 )
    {
        int length = 0;

        char* buffer = PlatformSpecific_LoadFile( m_FullPath, &length, m_FullPath, __LINE__ );
        if( length > 0 && buffer != 0 )
            FakeFileLoad( buffer, length );
    }

    if( m_Hack_TicksToWaitUntilWeActuallyLoadToSimulateAsyncLoading > 0 )
        m_Hack_TicksToWaitUntilWeActuallyLoadToSimulateAsyncLoading--;
}

void MyFileObject::FakeFileLoad(char* buffer, int length)
{
    assert( buffer != 0 && length > 0 );
    if( buffer == 0 || length <= 0 )
        return;

    m_pBuffer = buffer;
    m_FileLength = length;
    m_BytesRead = length;
    m_FileReady = true;
}

void MyFileObject::UnloadContents()
{
    SAFE_DELETE_ARRAY( m_pBuffer );
    m_FileLength = 0;
    m_BytesRead = 0;
    m_LoadFailed = false;
    m_FileReady = false;

    m_Hack_TicksToWaitUntilWeActuallyLoadToSimulateAsyncLoading = 2;

#if MYFW_USING_WX
    if( g_pPanelMemory )
        g_pPanelMemory->RemoveFile( this );
#endif
}

MySaveFileObject* CreatePlatformSpecificSaveFile()
{
    MySaveFileObject* pSaveFile = 0;

#if MYFW_WINDOWS
    pSaveFile = MyNew MySaveFileObject_FILE();
#elif MYFW_WP8
    pSaveFile = MyNew MySaveFileObject_FILE();
#elif MYFW_ANDROID
    pSaveFile = MyNew MySaveFileObject_FILE();
#elif MYFW_NACL
    pSaveFile = MyNew MySaveFileObject();
#elif MYFW_BLACKBERRY
    pSaveFile = MyNew MySaveFileObject_FILE();
#elif MYFW_BADA
    pSaveFile = MyNew MySaveFileObject_FILE();
#elif MYFW_IOS
    pSaveFile = MyNew MySaveFileObject_FILE();
#elif MYFW_EMSCRIPTEN
    LOGError( LOGTag, "Save file not implemented\n" );
#endif

    return pSaveFile;
}

MySaveFileObject_FILE::MySaveFileObject_FILE()
{
    m_pFile = 0;
    m_pObjectToWriteBuffer = 0;
}

MySaveFileObject_FILE::~MySaveFileObject_FILE()
{
}

void MySaveFileObject_FILE::Reset()
{
    MySaveFileObject::Reset();

    if( m_pFile )
        fclose( m_pFile );
    m_pFile = 0;

    m_pObjectToWriteBuffer = 0;
}

void MySaveFileObject_FILE::WriteString(const char* path, const char* filename, const char* string)
{
    assert( path != 0 );
    assert( filename != 0 );
    assert( string != 0 );
    assert( m_pFile == 0 );

    if( m_pFile != 0 )
        return;

    m_pFile = OpenSavedDataFile( path, filename, "wb" );
    m_pObjectToWriteBuffer = string;

    if( m_pFile )
    {
        MySaveFileObject::WriteString( path, filename, string );
    }
}

void MySaveFileObject_FILE::ReadString(const char* path, const char* filename)
{
    assert( m_SaveFileOp == SFO_None );
    assert( path != 0 );
    assert( filename != 0 );
    assert( m_pFile == 0 );

    m_pFile = OpenSavedDataFile( path, filename, "rb" );

    if( m_pFile )
    {
        MySaveFileObject::ReadString( path, filename );
    }
    else
    {
        assert( m_SaveFileOp == SFO_None );
        m_SaveFileOp = SFO_Read;
        m_OpComplete = true;
    }
}

void MySaveFileObject_FILE::Tick()
{
    if( m_SaveFileOp == SFO_None )
        return;
    if( m_OpComplete == true )
        return;

    assert( m_pFile != 0 );

    if( m_SaveFileOp == SFO_Read )
    {
        fseek( m_pFile, 0, SEEK_END );
        long length = ftell( m_pFile );
        if( length > 0 )
        {
            fseek( m_pFile, 0, SEEK_SET );
            m_pReadStringBuffer = MyNew char[length+1];

            fread( m_pReadStringBuffer, length, 1, m_pFile );
            m_pReadStringBuffer[length] = 0;
        }

        fclose( m_pFile );
        m_pFile = 0;
        m_OpComplete = true;
    }

    if( m_SaveFileOp == SFO_Write )
    {
        assert( m_pObjectToWriteBuffer != 0 );

        int length = (int)strlen( m_pObjectToWriteBuffer );
        fwrite( m_pObjectToWriteBuffer, length, 1, m_pFile );
        fclose( m_pFile );
        m_pFile = 0;
        m_OpComplete = true;
    }
}

#if MYFW_WINDOWS || MYFW_BLACKBERRY || MYFW_EMSCRIPTEN || MYFW_IOS
MyFileObject* RequestFile(const char* filename)
{
    return g_pFileManager->RequestFile( filename );
}
#endif

char* PlatformSpecific_LoadFile(const char* filename, int* length, const char* file, unsigned long line)
{
    FILE* filehandle;

#if MYFW_WINDOWS
    const char* fullpath = filename;

    errno_t error = fopen_s( &filehandle, fullpath, "rb" );
#elif MYFW_BLACKBERRY
    char fullpath[MAX_PATH];
    sprintf_s( fullpath, MAX_PATH, "app/native/%s", filename );

    filehandle = fopen( fullpath, "rb" );
#elif MYFW_IOS
    const char* fullpath = filename;
    
    filehandle = IOS_fopen( fullpath );
#else
    char* fullpath = filename;

    filehandle = fopen( fullpath, "rb" );
#endif

    char* filecontents = 0;

    if( filehandle )
    {
        fseek( filehandle, 0, SEEK_END );
        long size = ftell( filehandle );
        rewind( filehandle );

        filecontents = MyNew char[size+1];
        //filecontents = new(__FILE__, __LINE__) char[size];
        fread( filecontents, size, 1, filehandle );
        filecontents[size] = 0;

        if( length )
            *length = (int)size+1;
        
        fclose( filehandle );
    }
    else
    {
        LOGError( LOGTag, "File not found: %s\n", fullpath );
    }

    return filecontents;
}

GLuint LoadTextureFromMemory(TextureDefinition* texturedef)
{
    //LOGInfo( LOGTag, "LoadTextureFromMemory texturedef(%d)", texturedef );
    //LOGInfo( LOGTag, "LoadTextureFromMemory texturedef->m_pFile(%d)", texturedef->m_pFile );
    //LOGInfo( LOGTag, "LoadTextureFromMemory texturedef->m_pFile->m_pBuffer(%d)", texturedef->m_pFile->m_pBuffer );

    unsigned char* buffer = (unsigned char*)texturedef->m_pFile->m_pBuffer;
    int length = texturedef->m_pFile->m_FileLength;

    unsigned char* pngbuffer;
    unsigned int width, height;

    unsigned int error = lodepng_decode32( &pngbuffer, &width, &height, buffer, length );
    assert( error == 0 );

    GLuint texhandle = 0;
    glGenTextures( 1, &texhandle );
    assert( texhandle != 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, texhandle );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pngbuffer );
    checkGlError( "glTexImage2D" );
    free( pngbuffer );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texturedef->m_MinFilter ); //LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texturedef->m_MagFilter ); //GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texturedef->m_WrapS );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texturedef->m_WrapT );

    return texhandle;
}
