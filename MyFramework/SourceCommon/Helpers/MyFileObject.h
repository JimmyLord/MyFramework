//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __MyFileObject_H__
#define __MyFileObject_H__

class MyFileObject : public CPPListNode, public RefCount
{
    friend class FileManager;

public:
    char* m_FullPath;
    char* m_FilenameWithoutExtension;
    char* m_ExtensionWithDot; // will be "." if no extension
    bool m_LoadFailed;
    bool m_FileReady;
    unsigned int m_FileLength;
    char* m_pBuffer;
    int m_BytesRead;

#if MYFW_WINDOWS
    FILETIME m_FileLastWriteTime;
#endif

    int m_Hack_TicksToWaitUntilWeActuallyLoadToSimulateAsyncLoading;

public:
    MyFileObject();
    ~MyFileObject();

protected:
    void RequestFile(const char* filename);

    void Tick();
    void FakeFileLoad(char* buffer, int length);
    void UnloadContents();

    bool IsNewVersionAvailable();

public:
#if MYFW_USING_WX
    static void StaticOnDrag(void* pObjectPtr) { ((MyFileObject*)pObjectPtr)->OnDrag(); }
    void OnDrag();
#endif //MYFW_USING_WX
};

#endif //__MyFileObject_H__
