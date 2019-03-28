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

#include "../Renderers/BaseClasses/Renderer_Enums.h"
#include "../Shaders/VertexFormats.h"

class BufferManager;
class RefCount;
class Buffer_Base;

// ATM this whole buffer system creates a cpu memory block that mirrors what should be copied into OpenGL managed memory.

class BufferDefinition : public TCPPListNode<BufferDefinition*>, public RefCount
{
    friend class BufferManager;
    friend class PanelMemory;
    friend class Shader_Base;
    friend class Shader_OpenGL;

protected:
    Buffer_Base* m_pBuffer;
    char* m_pData;
    unsigned int m_DataSize;
    bool m_Dirty;

public:
    BufferDefinition();
    virtual ~BufferDefinition();

    // Getters.
    bool IsDirty() { return m_Dirty; }
    void* GetData(bool markDirty);
    uint32 GetDataSize() { return m_DataSize; }
    VertexFormats GetVertexFormat();
    VertexFormat_Dynamic_Desc* GetFormatDesc();
    uint32 GetStride();
    uint32 GetBytesPerIndex();
    uint32 GetMemoryUsage();

    // Stubs, clean up.
    // Getters.
    MyRE::IndexTypes GetIBOType();

    // Other.
    void TempBufferData(unsigned int sizeInBytes, void* pData);
    void Rebuild();
    void Rebuild(unsigned int offset, unsigned int sizeInBytes, bool forceRebuild = false);
    void Invalidate(bool cleanGLAllocs);

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
