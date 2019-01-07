//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#include "BufferManager.h"
#include "../Helpers/FileManager.h"
#include "../Renderers/OpenGL/Buffer_OpenGL.h"

BufferManager* g_pBufferManager = nullptr;

BufferDefinition::BufferDefinition()
{
    m_pBuffer = MyNew Buffer_OpenGL();

    m_pData = nullptr;
    m_DataSize = 0;

    m_Dirty = true;
}

BufferDefinition::~BufferDefinition()
{
    MyAssert( GetRefCount() == 0 ); // Did you call ->Release()?  Don't delete BufferDefinition objects.

    Invalidate( true );
    SAFE_DELETE_ARRAY( m_pData );

    delete m_pBuffer;

    this->Remove();
}

void* BufferDefinition::GetData(bool markDirty = false)
{
    if( markDirty )
        m_Dirty = true;

    return m_pData;
}

VertexFormats BufferDefinition::GetVertexFormat()
{
    Buffer_OpenGL* pGLBuffer = (Buffer_OpenGL*)m_pBuffer;

    MyAssert( pGLBuffer->m_BufferType == MyRE::BufferType_Vertex );

    return pGLBuffer->m_VertexFormat;
}

VertexFormat_Dynamic_Desc* BufferDefinition::GetFormatDesc()
{
    Buffer_OpenGL* pGLBuffer = (Buffer_OpenGL*)m_pBuffer;

    return pGLBuffer->m_pFormatDesc;
}

uint32 BufferDefinition::GetStride()
{
    Buffer_OpenGL* pGLBuffer = (Buffer_OpenGL*)m_pBuffer;

    MyAssert( pGLBuffer->m_BufferType == MyRE::BufferType_Vertex );

    if( pGLBuffer->m_VertexFormat == VertexFormat_Dynamic )
        return pGLBuffer->m_pFormatDesc->stride;

    return g_VertexFormatSizes[pGLBuffer->m_VertexFormat];
}

uint32 BufferDefinition::GetBytesPerIndex()
{
    Buffer_OpenGL* pGLBuffer = (Buffer_OpenGL*)m_pBuffer;

    MyAssert( pGLBuffer->m_BufferType == MyRE::BufferType_Index );

    return pGLBuffer->m_BytesPerIndex;
}

uint32 BufferDefinition::GetMemoryUsage()
{
    Buffer_OpenGL* pGLBuffer = (Buffer_OpenGL*)m_pBuffer;

    return m_DataSize * (pGLBuffer->m_NumBuffersToUse + 1);
}

MyRE::IndexTypes BufferDefinition::GetIBOType()
{
    return m_pBuffer->GetIBOType();
}

// Copy data into the GL buffer, but don't store the pointer or size so it can't be rebuilt.
void BufferDefinition::TempBufferData(unsigned int sizeInBytes, void* pData)
{
    m_pBuffer->TempBufferData( sizeInBytes, pData );

    m_Dirty = false;
}

void BufferDefinition::Rebuild()
{
    // Rebuffer all data.
    Rebuild( 0, UINT_MAX, false );
}

void BufferDefinition::Rebuild(unsigned int offset, unsigned int sizeInBytes, bool forceRebuild)
{
    if( g_pGameCore->IsGLSurfaceIsValid() == false )
        return;

    if( m_Dirty == false && forceRebuild == false )
        return;

    if( m_DataSize == 0 )
        return;

    if( sizeInBytes == UINT_MAX )
    {
        // Rebuffer all data.
        m_pBuffer->Rebuild( 0, m_DataSize, forceRebuild, m_DataSize, m_pData );
    }
    else
    {
        // Rebuffer as much as was requested.
        m_pBuffer->Rebuild( offset, sizeInBytes, forceRebuild, m_DataSize, m_pData );

        if( sizeInBytes > m_DataSize )
        {
            m_DataSize = sizeInBytes;
        }
    }

    m_Dirty = false;
}

void BufferDefinition::Invalidate(bool cleanGLAllocs)
{
    m_pBuffer->Invalidate( cleanGLAllocs );

    m_Dirty = true;
}

void BufferDefinition::CreateAndBindVAO()
{
    m_pBuffer->CreateAndBindVAO();
}

void BufferDefinition::ResetVAOs()
{
    m_pBuffer->ResetVAOs();
}

void BufferDefinition::FreeBufferedData()
{
    m_pBuffer->FreeBufferedData();

    SAFE_DELETE_ARRAY( m_pData );
    m_DataSize = 0;
}

void BufferDefinition::InitializeBuffer(void* pData, unsigned int dataSize, MyRE::BufferTypes bufferType, MyRE::BufferUsages bufferUsage, bool bufferData, unsigned int numBuffersToAllocate, int bytesPerIndex, const char* category, const char* desc)
{
    InitializeBuffer( pData, dataSize, bufferType, bufferUsage, bufferData, numBuffersToAllocate, (VertexFormats)bytesPerIndex, nullptr, category, desc );
}

void BufferDefinition::InitializeBuffer(void* pData, unsigned int dataSize, MyRE::BufferTypes bufferType, MyRE::BufferUsages bufferUsage, bool bufferData, unsigned int numBuffersToAllocate, VertexFormats format, VertexFormat_Dynamic_Desc* pVertexFormatDesc, const char* category, const char* desc)
{
    MyAssert( numBuffersToAllocate >= 1 && numBuffersToAllocate <= 3 );

    bool dataSizeChanged = false;

    if( dataSize == 0 || dataSize != m_DataSize )
    {
        dataSizeChanged = true;

        // Delete old data block if necessary.
        SAFE_DELETE_ARRAY( m_pData );

        // If no data block was passed in, allocate one if dataSize isn't 0.
        if( pData == nullptr && dataSize != 0 )
            pData = MyNew char[dataSize];
    }

    m_pBuffer->InitializeBuffer( dataSizeChanged, pData, dataSize, bufferType, bufferUsage, bufferData, numBuffersToAllocate, format, pVertexFormatDesc, category, desc );

    ResetVAOs();

    m_pData = (char*)pData;
    m_DataSize = dataSize;

    Buffer_OpenGL* pGLBuffer = (Buffer_OpenGL*)m_pBuffer;

    if( bufferData )
    {
        m_pData = (char*)pData;
        for( unsigned int i=0; i<pGLBuffer->m_NumBuffersToUse; i++ )
        {
            Rebuild( 0, m_DataSize, true );
        }
    }
    else
    {
        m_pData = nullptr;
        for( unsigned int i=0; i<pGLBuffer->m_NumBuffersToUse; i++ )
        {
            Rebuild( 0, m_DataSize, true );
        }
        m_pData = (char*)pData;
        m_Dirty = true;
    }
}

//====================================================
//====================================================

BufferManager::BufferManager()
{
}

BufferManager::~BufferManager()
{
    FreeAllBuffers();
}

BufferDefinition* BufferManager::CreateBuffer(void* pData, unsigned int dataSize, MyRE::BufferTypes bufferType, MyRE::BufferUsages bufferUsage, bool bufferData, unsigned int numBuffersToAllocate, int bytesPerIndex, const char* category, const char* desc)
{
    return CreateBuffer( pData, dataSize, bufferType, bufferUsage, bufferData, numBuffersToAllocate, (VertexFormats)bytesPerIndex, nullptr, category, desc );
}

BufferDefinition* BufferManager::CreateBuffer()
{
    //LOGInfo( LOGTag, "CreateBuffer\n" );

    BufferDefinition* pBufferDef = MyNew BufferDefinition();
    m_Buffers.AddTail( pBufferDef );

    return pBufferDef;
}

BufferDefinition* BufferManager::CreateBuffer(void* pData, unsigned int dataSize, MyRE::BufferTypes bufferType, MyRE::BufferUsages bufferUsage, bool bufferData, unsigned int numBuffersToAllocate, VertexFormats format, VertexFormat_Dynamic_Desc* pVertexFormatDesc, const char* category, const char* desc)
{
    //LOGInfo( LOGTag, "CreateBuffer\n" );

    BufferDefinition* pBufferDef = CreateBuffer();

    pBufferDef->InitializeBuffer( pData, dataSize, bufferType, bufferUsage, bufferData, numBuffersToAllocate, format, pVertexFormatDesc, category, desc );

    return pBufferDef;
}

void BufferManager::Tick()
{
}

void BufferManager::FreeAllBuffers()
{
    while( BufferDefinition* pBuffer = m_Buffers.GetHead() )
    {
        LOGInfo( LOGTag, "Buffers weren't cleaned by their owners\n" );
        MyAssert( false );

        while( pBuffer->GetRefCount() > 1 )
            pBuffer->Release();

        pBuffer->Release();
    }
}

void BufferManager::InvalidateAllBuffers(bool cleanglallocs)
{
    for( BufferDefinition* pBufferDef = m_Buffers.GetHead(); pBufferDef; )
    {
        BufferDefinition* pNextBufferDef = pBufferDef->GetNext();

        //LOGInfo( LOGTag, "Invalidated buffer\n" );
        pBufferDef->Invalidate( cleanglallocs );

        pBufferDef = pNextBufferDef;
    }
}

unsigned int BufferManager::CalculateTotalMemoryUsedByBuffers()
{
    unsigned int totalsize = 0;

    for( BufferDefinition* pBufferDef = m_Buffers.GetHead(); pBufferDef; pBufferDef = pBufferDef->GetNext() )
    {
        totalsize += pBufferDef->GetMemoryUsage();
    }

    return totalsize;
}
