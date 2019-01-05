//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __BufferManager_H__
#define __BufferManager_H__

class BufferManager;
class RefCount;

extern BufferManager* g_pBufferManager;

// ATM this whole buffer system creates a cpu memory block that mirrors what should be copied into OpenGL memory.

class BufferDefinition : public TCPPListNode<BufferDefinition*>, public RefCount
{
    friend class BufferManager;
    friend class PanelMemory;
    friend class Shader_Base;
    friend class Shader_OpenGL;

protected:
    GLuint m_BufferIDs[3]; // Up to 3 buffers created for double/triple buffering data.
    GLuint m_VAOHandles[3]; // Used only for vbo's ATM.
    bool m_VAOInitialized[3];

    unsigned int m_NumBuffersToUse;
    unsigned int m_CurrentBufferIndex;
    unsigned int m_NextBufferIndex;

public:
#if _DEBUG && MYFW_WINDOWS
    int m_DEBUG_CurrentVAOIndex;
    GLuint m_DEBUG_VBOUsedOnCreation[3];
    GLuint m_DEBUG_IBOUsedOnCreation[3];
    int m_DEBUG_LastFrameUpdated;
#endif

    GLuint m_CurrentBufferID;
    GLuint m_CurrentVAOHandle;

    char* m_pData;
    unsigned int m_DataSize;
    union
    {
        VertexFormats m_VertexFormat; // sanity check for GL_ARRAY_BUFFER'S, if this is a vertex buffer.
        int m_BytesPerIndex; // if this is an index buffer.
    };
    VertexFormat_Dynamic_Desc* m_pFormatDesc;
    MyRE::BufferTypes m_BufferType;
    MyRE::BufferUsages m_BufferUsage;
    bool m_Dirty;

public:
    BufferDefinition();
    virtual ~BufferDefinition();

    // Getters.
    MyRE::IndexTypes GetIBOType();
    void* GetData() { return m_pData; }

    // Other.
    void TempBufferData(unsigned int sizeinbytes, void* pData);
    void Rebuild(unsigned int offset, unsigned int sizeinbytes, bool forcerebuild = false);
    void Invalidate(bool cleanglallocs);

    void CreateAndBindVAO();
    void ResetVAOs();

    void FreeBufferedData();
    void InitializeBuffer(void* pData, unsigned int dataSize, MyRE::BufferTypes bufferType, MyRE::BufferUsages bufferUsage, bool bufferData, unsigned int numBuffersToAllocate, int bytesPerIndex, const char* category, const char* desc);
    void InitializeBuffer(void* pData, unsigned int dataSize, MyRE::BufferTypes bufferType, MyRE::BufferUsages bufferUsage, bool bufferData, unsigned int numBuffersToAllocate, VertexFormats format, VertexFormat_Dynamic_Desc* pVertexFormatDesc, const char* category, const char* desc);
};

class BufferManager
{
    friend class PanelMemory;

protected:
    TCPPListHead<BufferDefinition*> m_Buffers;

public:
    BufferManager();
    virtual ~BufferManager();

    // pData pointer passed in will be deleted by the BufferDefinition.
    BufferDefinition* CreateBuffer();
    BufferDefinition* CreateBuffer(void* pData, unsigned int dataSize, MyRE::BufferTypes bufferType, MyRE::BufferUsages bufferUsage, bool bufferData, unsigned int numBuffersToAllocate, int bytesPerIndex, const char* category, const char* desc);
    BufferDefinition* CreateBuffer(void* pData, unsigned int dataSize, MyRE::BufferTypes bufferType, MyRE::BufferUsages bufferUsage, bool bufferData, unsigned int numBuffersToAllocate, VertexFormats format, VertexFormat_Dynamic_Desc* pVertexFormatDesc, const char* category, const char* desc);

    void Tick();

    void InvalidateAllBuffers(bool cleanglallocs);

    unsigned int CalculateTotalMemoryUsedByBuffers();

private:
    void FreeAllBuffers();
};

#endif //__BufferManager_H__
