//
// Copyright (c) 2012-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#include "BufferManager.h"
#include "../Helpers/FileManager.h"

BufferManager* g_pBufferManager = 0;

BufferDefinition::BufferDefinition()
{
    for( int i=0; i<3; i++ )
    {
        m_BufferIDs[i] = 0; // up to 3 buffers created for double/triple buffering data.
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

    m_pData = 0;
    m_DataSize = 0;
    m_VertexFormat = VertexFormat_Invalid;
    m_pFormatDesc = 0;
    m_Target = GL_ARRAY_BUFFER; //GL_ELEMENT_ARRAY_BUFFER
    m_Usage = GL_STATIC_DRAW; //GL_DYNAMIC_DRAW //GL_STREAM_DRAW
    m_Dirty = true;
}

BufferDefinition::~BufferDefinition()
{
    MyAssert( GetRefCount() == 0 ); // Did you call ->Release()?  don't delete BufferDefinition objects.

    Invalidate( true );
    SAFE_DELETE_ARRAY( m_pData );

#if MYFW_USING_WX
    if( g_pPanelMemory )
        g_pPanelMemory->RemoveBuffer( this );
#endif

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

// copy data into the gl buffer, but don't store the pointer or size so it can't be rebuilt.
void BufferDefinition::TempBufferData(unsigned int sizeinbytes, void* pData)
{
    MyAssert( pData != 0 && sizeinbytes != 0 );

    if( m_BufferIDs[m_NextBufferIndex] == 0 )
    {
        glGenBuffers( 1, &m_BufferIDs[m_NextBufferIndex] );
        checkGlError( "glGenBuffers" );
    }

    MyBindBuffer( m_Target, m_BufferIDs[m_NextBufferIndex] );
    glBufferData( m_Target, sizeinbytes, pData, m_Usage );

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

    //MyAssert( g_pGameCore->m_GLSurfaceIsValid );
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

        MyBindBuffer( m_Target, m_BufferIDs[m_NextBufferIndex] );
        checkGlError( "MyBindBuffer" );

        glBufferData( m_Target, m_DataSize, m_pData, m_Usage );
        checkGlError( "glBufferData" );

        //LOGInfo( LOGTag, "creating buffer - %d - target:%d\n", m_BufferIDs[m_NextBufferIndex], m_Target );
    }
    else
    {
        MyBindBuffer( m_Target, m_BufferIDs[m_NextBufferIndex] );
        checkGlError( "BufferDefinition::Rebuild MyBindBuffer" );

        //LOGInfo( LOGTag, "BufferDefinition::Rebuild() rebuilding sizeinbytes(%d) m_DataSize(%d)\n", sizeinbytes, m_DataSize );

        //glBufferData( m_Target, sizeinbytes, 0, m_Usage );
        //glBufferData( m_Target, sizeinbytes, m_pData, m_Usage );
        if( sizeinbytes > m_DataSize )
        {
            m_DataSize = sizeinbytes;

            glBufferData( m_Target, m_DataSize, m_pData, m_Usage );
        }
        else
        {
#if MYFW_IOS
            glBufferData( m_Target, sizeinbytes, m_pData, m_Usage );
#else
            glBufferSubData( m_Target, offset, sizeinbytes, m_pData );
#endif
        }
        checkGlError( "glBufferData or glBufferSubData" );

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

void BufferDefinition::InitializeBuffer(void* pData, unsigned int datasize, GLenum target, GLenum usage, bool bufferdata, unsigned int numbufferstoallocate, int bytesperindex, const char* category, const char* desc)
{
    InitializeBuffer( pData, datasize, target, usage, bufferdata, numbufferstoallocate, (VertexFormats)bytesperindex, 0, category, desc );
}

void BufferDefinition::InitializeBuffer(void* pData, unsigned int datasize, GLenum target, GLenum usage, bool bufferdata, unsigned int numbufferstoallocate, VertexFormats format, VertexFormat_Dynamic_Desc* pVertexFormatDesc, const char* category, const char* desc)
{
    MyAssert( numbufferstoallocate >= 1 && numbufferstoallocate <= 3 );

    if( datasize == 0 || datasize != m_DataSize )
    {
        // delete old data block if necessary
        SAFE_DELETE_ARRAY( m_pData );

        // if no data block was passed in allocate one if datasize isn't 0.
        if( pData == 0 && datasize != 0 )
            pData = MyNew char[datasize];

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

    m_NumBuffersToUse = numbufferstoallocate;
    m_pData = (char*)pData;
    m_DataSize = datasize;
    m_Target = target;
    m_Usage = usage;
    m_VertexFormat = format;
    m_pFormatDesc = pVertexFormatDesc;

    if( bufferdata )
    {
        m_pData = (char*)pData;
        for( unsigned int i=0; i<m_NumBuffersToUse; i++ )
            Rebuild( 0, m_DataSize, true );
    }
    else
    {
        m_pData = 0;
        for( unsigned int i=0; i<m_NumBuffersToUse; i++ )
            Rebuild( 0, m_DataSize, true );
        m_pData = (char*)pData;
        m_Dirty = true;
    }

#if MYFW_USING_WX
    // TODO: temp disabled this, never used it, but can be slow with 100s of buffers.
    //g_pPanelMemory->RemoveBuffer( this );
    //g_pPanelMemory->AddBuffer( this, category, desc );
#endif
}

//====================================================
//====================================================

//VAODefinition::VAODefinition()
//{
//    m_Handle = 0;
//    m_Initialized = false;
//
//#if _DEBUG && MYFW_WINDOWS
//    m_DEBUG_VBOUsedOnCreation = 0;
//    m_DEBUG_IBOUsedOnCreation = 0;
//#endif
//}
//
//VAODefinition::~VAODefinition()
//{
//    MyAssert( GetRefCount() == 0 ); // Did you call ->Release()?  don't delete VAODefinition objects.
//
//    Invalidate( true );
//
//    this->Remove();
//}
//
//void VAODefinition::Invalidate(bool cleanglallocs)
//{
//    if( cleanglallocs && m_Handle != 0 )
//    {
//        glDeleteVertexArrays( 1, &m_Handle );
//    }
//
//    m_Handle = 0;
//    m_Initialized = false;
//}
//
//void VAODefinition::Create()
//{
//    MyAssert( m_Handle == 0 );
//    MyAssert( glGenVertexArrays != 0 );
//
//    //if( glGenVertexArrays != 0 )
//    {
//        glGenVertexArrays( 1, &m_Handle );
//        MyAssert( m_Handle != 0 );
//        m_Initialized = true;
//    }
//}

//====================================================
//====================================================


BufferManager::BufferManager()
{
}

BufferManager::~BufferManager()
{
    FreeAllBuffers();
}

BufferDefinition* BufferManager::CreateBuffer(void* pData, unsigned int datasize, GLenum target, GLenum usage, bool bufferdata, unsigned int numbufferstoallocate, int bytesperindex, const char* category, const char* desc)
{
    return CreateBuffer(pData, datasize, target, usage, bufferdata, numbufferstoallocate, (VertexFormats)bytesperindex, 0, category, desc);
}

BufferDefinition* BufferManager::CreateBuffer()
{
    //LOGInfo( LOGTag, "CreateBuffer\n" );

    BufferDefinition* pBufferDef = MyNew BufferDefinition();
    m_Buffers.AddTail( pBufferDef );

    return pBufferDef;
}

BufferDefinition* BufferManager::CreateBuffer(void* pData, unsigned int datasize, GLenum target, GLenum usage, bool bufferdata, unsigned int numbufferstoallocate, VertexFormats format, VertexFormat_Dynamic_Desc* pVertexFormatDesc, const char* category, const char* desc)
{
    //LOGInfo( LOGTag, "CreateBuffer\n" );

    BufferDefinition* pBufferDef = CreateBuffer();

    pBufferDef->InitializeBuffer( pData, datasize, target, usage, bufferdata, numbufferstoallocate, format, pVertexFormatDesc, category, desc );

    return pBufferDef;
}

//VAODefinition* BufferManager::CreateVAO()
//{
//    //LOGInfo( LOGTag, "CreateVAO\n" );
//
//    VAODefinition* pVAODef = MyNew VAODefinition();
//
//    m_VAOs.AddTail( pVAODef );
//
//#if MYFW_USING_WX
//    //g_pPanelMemory->AddBuffer( pBufferDef, category, desc );
//#endif
//
//    return pVAODef;
//}

void BufferManager::Tick()
{
}

void BufferManager::FreeAllBuffers()
{
    while( CPPListNode* pNode = m_Buffers.GetHead() )
    {
        LOGInfo( LOGTag, "Buffers weren't cleaned by their owners\n" );
        MyAssert( false );

        BufferDefinition* pBuffer = (BufferDefinition*)pNode;
        while( pBuffer->GetRefCount() > 1 )
            pBuffer->Release();

        pBuffer->Release();
    }

    //while( CPPListNode* pNode = m_VAOs.GetHead() )
    //{
    //    LOGInfo( LOGTag, "VAOs weren't cleaned by their owners\n" );
    //    MyAssert( false );

    //    VAODefinition* pVAO = (VAODefinition*)pNode;
    //    while( pVAO->GetRefCount() > 1 )
    //        pVAO->Release();

    //    pVAO->Release();
    //}
}

void BufferManager::InvalidateAllBuffers(bool cleanglallocs)
{
    for( CPPListNode* pNode = m_Buffers.GetHead(); pNode; )
    {
        BufferDefinition* pBufferDef = (BufferDefinition*)pNode;
        pNode = pNode->GetNext();

        //LOGInfo( LOGTag, "Invalidated buffer\n" );

        pBufferDef->Invalidate( cleanglallocs );
    }

    //for( CPPListNode* pNode = m_VAOs.GetHead(); pNode; )
    //{
    //    VAODefinition* pVAODef = (VAODefinition*)pNode;
    //    pNode = pNode->GetNext();

    //    //LOGInfo( LOGTag, "Invalidated VAO\n" );

    //    pVAODef->Invalidate( cleanglallocs );
    //}
}

unsigned int BufferManager::CalculateTotalMemoryUsedByBuffers()
{
    unsigned int totalsize = 0;

    for( CPPListNode* pNode = m_Buffers.GetHead(); pNode; )
    {
        BufferDefinition* pBufferDef = (BufferDefinition*)pNode;
        pNode = pNode->GetNext();

        totalsize += pBufferDef->m_DataSize * (pBufferDef->m_NumBuffersToUse + 1);
    }

    return totalsize;
}
