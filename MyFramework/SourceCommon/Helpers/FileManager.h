//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __FileManager_H__
#define __FileManager_H__

class MySaveFileObject;
class FileManager;
class MyFileObject;
class TextureDefinition;

extern FileManager* g_pFileManager;
MySaveFileObject* CreatePlatformSpecificSaveFile();

#if MYFW_WINDOWS || MYFW_BLACKBERRY || MYFW_EMSCRIPTEN || MYFW_IOS
MyFileObject* RequestFile(const char* filename);
#endif
GLuint LoadTextureFromMemory(TextureDefinition* texturedef);

class FileManager
{
protected:
    CPPListHead m_FilesLoaded;
    CPPListHead m_FilesStillLoading;

public:
    FileManager();
    virtual ~FileManager();

    void FreeFile(MyFileObject* pFile);
    //void FreeAllFiles();

    unsigned int CalculateTotalMemoryUsedByFiles();

    MyFileObject* RequestFile(const char* filename); // will add a ref.
    MyFileObject* FindFileByName(const char* filename); // doesn't add a ref to the file.
    void ReloadFile(MyFileObject* pFile);
    void Tick();

    MyFileObject* GetFirstFileLoaded() { return (MyFileObject*)m_FilesLoaded.GetHead(); }
    MyFileObject* GetFirstFileStillLoading() { return (MyFileObject*)m_FilesStillLoading.GetHead(); }
};

enum SaveFileOperation
{
    SFO_None,
    SFO_Read,
    SFO_Write,
};

class MySaveFileObject
{
public:
    SaveFileOperation m_SaveFileOp;
    bool m_OpComplete;
    char* m_pReadStringBuffer; // allocated by this object(or subclasses), so needs to be cleaned up.

public:
    MySaveFileObject()
    {
        m_SaveFileOp = SFO_None;
        m_OpComplete = false;
        m_pReadStringBuffer = 0;
    }
    virtual ~MySaveFileObject() {}

    virtual void Reset()
    {
        m_SaveFileOp = SFO_None;
        m_OpComplete = false;
        SAFE_DELETE_ARRAY( m_pReadStringBuffer );
    }

    virtual void WriteString(const char* path, const char* filename, const char* string)
    {
        assert( m_SaveFileOp == SFO_None );
        m_SaveFileOp = SFO_Write;
        m_OpComplete = false;
    }

    virtual void ReadString(const char* path, const char* filename)
    {
        assert( m_SaveFileOp == SFO_None );
        m_SaveFileOp = SFO_Read;
        m_OpComplete = false;
    }

    virtual void Tick()
    {
        m_OpComplete = true;
    }

    virtual bool QueryIsDone()
    {
        return m_OpComplete || m_SaveFileOp == SFO_None;
    }

    virtual char* QueryString()
    {
        return m_pReadStringBuffer;
    }
};

class MySaveFileObject_FILE : public MySaveFileObject
{
public:
    FILE* m_pFile;
    const char* m_pObjectToWriteBuffer; // managed by caller to this function... i.e. will not be deleted when write is complete.

public:
    MySaveFileObject_FILE();
    ~MySaveFileObject_FILE();

    virtual void Reset();

    virtual void WriteString(const char* path, const char* filename, const char* string);
    virtual void ReadString(const char* path, const char* filename);
    virtual void Tick();

    //virtual bool QueryIsDone();
    //virtual char* QueryString();
};

#endif //__FileManager_H__
