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

// TODO: Fix GL Includes.
#include <gl/GL.h>
#include "../../GLExtensions.h"
#include "../Renderers/OpenGL/GLHelpers.h"

BufferManager* g_pBufferManager = nullptr;

BufferDefinition::BufferDefinition()
{
    for( int i=0; i<3; i++ )
    {
        m_BufferIDs[i] = 0; // Up to 3 buffers created for double/triple buffering data.
        m_VAOHandles[i] = 0;
    }
    m_NumBuffersToUse = 0;
    m_CurrentBufferIndex = 0;
    m_NextBufferIndex = 0;

#if _DEBUG && MYFW_WINDOWS
    m_DEBUG_CurrentVAOIndex = 0;
    for( int i=0; i<3; i++ )
    {
        m_DEBUG_VBOUsedOnCreation[i] = 0;
        m_DEBUG_IBOUsedOnCreation[i] = 0;
    }
    m_DEBUG_LastFrameUpdated = -1;
#endif

    m_CurrentBufferID = 0;
    m_CurrentVAOHandle = 0;
    for( int i=0; i<3; i++ )
    {
        m_VAOInitialized[i] = false;
    }

    m_pData = nullptr;
    m_DataSize = 0;
    m_VertexFormat = VertexFormat_Invalid;
    m_pFormatDesc = nullptr;
    m_BufferType = MyRE::BufferType_Vertex;
    m_BufferUsage = MyRE::BufferUsage_StaticDraw;
    m_Dirty = true;
}

BufferDefinition::~BufferDefinition()
{
    MyAssert( GetRefCount() == 0 ); // Did you call ->Release()?  Don't delete BufferDefinition objects.

    Invalidate( true );
    SAFE_DELETE_ARRAY( m_pData );

    this->Remove();
}

MyRE::IndexTypes BufferDefinition::GetIBOType()
{
    if( m_BytesPerIndex == 1 )
        return MyRE::IndexType_U8;
    else if( m_BytesPerIndex == 2 )
        return MyRE::IndexType_U16;
    else if( m_BytesPerIndex == 4 )
        return MyRE::IndexType_U32;

    MyAssert( false );

    return MyRE::IndexType_Undefined;
}

// Copy data into the gl buffer, but don't store the pointer or size so it can't be rebuilt.
void BufferDefinition::TempBufferData(unsigned int sizeinbytes, void* pData)
{
    MyAssert( pData != nullptr && sizeinbytes != 0 );

    if( m_BufferIDs[m_NextBufferIndex] == 0 )
    {
        glGenBuffers( 1, &m_BufferIDs[m_NextBufferIndex] );
        checkGlError( "glGenBuffers" );
    }

    g_pRenderer->BufferData( this, m_BufferIDs[m_NextBufferIndex], sizeinbytes, pData );

    m_CurrentBufferID = m_BufferIDs[m_NextBufferIndex];
    m_CurrentVAOHandle = m_VAOHandles[m_NextBufferIndex];
#if _DEBUG && MYFW_WINDOWS
    m_DEBUG_CurrentVAOIndex = m_NextBufferIndex;
#endif

    m_CurrentBufferIndex = m_NextBufferIndex;
    m_NextBufferIndex++;
    if( m_NextBufferIndex >= m_NumBuffersToUse )
        m_NextBufferIndex = 0;

    m_Dirty = false;

    checkGlError( "BufferDefinition::TempBufferData" );
}

void BufferDefinition::Rebuild(unsigned int offset, unsigned int sizeinbytes, bool forcerebuild)
{
    checkGlError( "BufferDefinition::Rebuild" );

    if( g_pGameCore->IsGLSurfaceIsValid() == false )
        return;

    if( m_Dirty == false && forcerebuild == false )
        return;

    if( m_DataSize == 0 )
        return;

    if( m_BufferIDs[m_NextBufferIndex] == 0 )
    {
        glGenBuffers( 1, &m_BufferIDs[m_NextBufferIndex] );
        checkGlError( "glGenBuffers" );

        g_pRenderer->BufferData( this, m_BufferIDs[m_NextBufferIndex], m_DataSize, m_pData );

        //LOGInfo( LOGTag, "creating buffer - %d - target:%d\n", m_BufferIDs[m_NextBufferIndex], m_Target );
    }
    else
    {
        //LOGInfo( LOGTag, "BufferDefinition::Rebuild() rebuilding sizeinbytes(%d) m_DataSize(%d)\n", sizeinbytes, m_DataSize );

        if( sizeinbytes > m_DataSize )
        {
            m_DataSize = sizeinbytes;

            g_pRenderer->BufferData( this, m_BufferIDs[m_NextBufferIndex], m_DataSize, m_pData );
        }
        else
        {
#if MYFW_IOS
            g_pRenderer->BufferData( this, m_BufferIDs[m_NextBufferIndex], sizeinbytes, m_pData );
#else
            g_pRenderer->BufferSubData( this, m_BufferIDs[m_NextBufferIndex], offset, sizeinbytes, m_pData );
#endif
        }

#if _DEBUG && MYFW_WINDOWS
        // Buffer shouldn't be updated twice in one frame, might cause stall.
        //MyAssert( m_DEBUG_LastFrameUpdated != g_GLStats.m_NumFramesDrawn );
        if( m_DEBUG_LastFrameUpdated == g_GLStats.m_NumFramesDrawn )
        {
            int bp = 1;
            //LOGInfo( LOGTag, "Buffer updated twice in one frame!\n" );
        }
        m_DEBUG_LastFrameUpdated = g_GLStats.m_NumFramesDrawn;
#endif

        //if( sizeinbytes > m_DataSize )
        //    LOGError( LOGTag, "glBufferSubData sizeinbytes(%d) > m_DataSize(%d)\n", sizeinbytes, m_DataSize );
        //if( checkGlError( "BufferDefinition::Rebuild glBufferSubData" ) == true )
        //    LOGError( LOGTag, "glBufferSubData - offset(%d), size(%d), m_DataSize(%d)\n", offset, sizeinbytes, m_DataSize );
    }

    m_CurrentBufferID = m_BufferIDs[m_NextBufferIndex];
    m_CurrentVAOHandle = m_VAOHandles[m_NextBufferIndex];
#if _DEBUG && MYFW_WINDOWS
    m_DEBUG_CurrentVAOIndex = m_NextBufferIndex;
#endif

    m_CurrentBufferIndex = m_NextBufferIndex;
    m_NextBufferIndex++;
    if( m_NextBufferIndex >= m_NumBuffersToUse )
        m_NextBufferIndex = 0;

    m_Dirty = false;
}

void BufferDefinition::Invalidate(bool cleanglallocs)
{
    if( cleanglallocs )
    {
        for( int i=0; i<3; i++ )
        {
            if( m_BufferIDs[i] != 0 )
            {
                MyDeleteBuffers( 1, &m_BufferIDs[i] );
                m_BufferIDs[i] = 0;
            }

#if !MYFW_IOS
            if( glDeleteVertexArrays )
#endif
            {
                if( m_VAOHandles[i] != 0 )
                {
                    glDeleteVertexArrays( 1, &m_VAOHandles[i] );
                    m_VAOHandles[i] = 0;
                }
            }
        }
    }

    for( int i=0; i<3; i++ )
    {
        m_BufferIDs[i] = 0;
        m_VAOHandles[i] = 0;
        m_VAOInitialized[i] = 0;
    }

#if _DEBUG && MYFW_WINDOWS
    m_DEBUG_CurrentVAOIndex = 0;
    for( int i=0; i<3; i++ )
    {
        m_DEBUG_VBOUsedOnCreation[i] = 0;
        m_DEBUG_IBOUsedOnCreation[i] = 0;
    }
#endif

    m_CurrentBufferID = 0;
    m_Dirty = true;
}

void BufferDefinition::CreateAndBindVAO()
{
    MyAssert( glBindVertexArray != 0 );

    if( m_VAOHandles[m_CurrentBufferIndex] == 0 )
	{
        glGenVertexArrays( 1, &m_VAOHandles[m_CurrentBufferIndex] );
	}
    MyAssert( m_VAOHandles[m_CurrentBufferIndex] != 0 );

    m_CurrentVAOHandle = m_VAOHandles[m_CurrentBufferIndex];

    glBindVertexArray( m_CurrentVAOHandle );

    // HACK: Since MyBindBuffer doesn't rebind the buffer if it's the same as the currently bound buffer...
    //       bind the 2 buffers to zero, so the InitializeAttributeArrays will bind the correct ones to the VAO.
    //   Not sure why this is necessary since the VertexAttribPointer calls should work the same either way.
    MyBindBuffer( GL_ARRAY_BUFFER, 0 );
    MyBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    // ~HACK
}

void BufferDefinition::ResetVAOs()
{
    for( int i=0; i<3; i++ )
    {
        m_VAOInitialized[i] = false;
    }
}

void BufferDefinition::FreeBufferedData()
{
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

    if( dataSize == 0 || dataSize != m_DataSize )
    {
        // Delete old data block if necessary.
        SAFE_DELETE_ARRAY( m_pData );

        // If no data block was passed in, allocate one if dataSize isn't 0.
        if( pData == nullptr && dataSize != 0 )
            pData = MyNew char[dataSize];

        for( int i=0; i<3; i++ )
        {
            if( m_BufferIDs[i] != 0 )
            {
                MyDeleteBuffers( 1, &m_BufferIDs[i] );
                m_BufferIDs[i] = 0;
            }
        }
    }

    ResetVAOs();

    m_NumBuffersToUse = numBuffersToAllocate;
    m_pData = (char*)pData;
    m_DataSize = dataSize;
    m_BufferType = bufferType;
    m_BufferUsage = bufferUsage;
    m_VertexFormat = format;
    m_pFormatDesc = pVertexFormatDesc;

    if( bufferData )
    {
        m_pData = (char*)pData;
        for( unsigned int i=0; i<m_NumBuffersToUse; i++ )
            Rebuild( 0, m_DataSize, true );
    }
    else
    {
        m_pData = nullptr;
        for( unsigned int i=0; i<m_NumBuffersToUse; i++ )
            Rebuild( 0, m_DataSize, true );
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
        totalsize += pBufferDef->m_DataSize * (pBufferDef->m_NumBuffersToUse + 1);
    }

    return totalsize;
}
