//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "Buffer_OpenGL.h"
#include "GLHelpers.h"
#include "../BaseClasses/Renderer_Base.h"
#include "../Old/OpenGLWrapper.h"
#include "../../GLExtensions.h"

Buffer_OpenGL::Buffer_OpenGL()
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

    m_VertexFormat = VertexFormat_Invalid;
    m_pFormatDesc = nullptr;
    m_BufferType = MyRE::BufferType_Vertex;
    m_BufferUsage = MyRE::BufferUsage_StaticDraw;
}

Buffer_OpenGL::~Buffer_OpenGL()
{
}

MyRE::IndexTypes Buffer_OpenGL::GetIBOType()
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
void Buffer_OpenGL::TempBufferData(unsigned int bytesToBuffer, void* pData)
{
    MyAssert( pData != nullptr && bytesToBuffer != 0 );

    if( m_BufferIDs[m_NextBufferIndex] == 0 )
    {
        glGenBuffers( 1, &m_BufferIDs[m_NextBufferIndex] );
        checkGlError( "glGenBuffers" );
    }

    g_pRenderer->BufferData( this, m_BufferIDs[m_NextBufferIndex], bytesToBuffer, pData );

    m_CurrentBufferID = m_BufferIDs[m_NextBufferIndex];
    m_CurrentVAOHandle = m_VAOHandles[m_NextBufferIndex];
#if _DEBUG && MYFW_WINDOWS
    m_DEBUG_CurrentVAOIndex = m_NextBufferIndex;
#endif

    m_CurrentBufferIndex = m_NextBufferIndex;
    m_NextBufferIndex++;
    if( m_NextBufferIndex >= m_NumBuffersToUse )
        m_NextBufferIndex = 0;

    checkGlError( "Buffer_OpenGL::TempBufferData" );
}

void Buffer_OpenGL::Rebuild(unsigned int offset, unsigned int bytesToBuffer, bool forceRebuild, unsigned int bufferSize, void* pData)
{
    checkGlError( "Buffer_OpenGL::Rebuild" );

    if( m_BufferIDs[m_NextBufferIndex] == 0 )
    {
        glGenBuffers( 1, &m_BufferIDs[m_NextBufferIndex] );
        checkGlError( "glGenBuffers" );

        g_pRenderer->BufferData( this, m_BufferIDs[m_NextBufferIndex], bytesToBuffer, pData );

        //LOGInfo( LOGTag, "Creating buffer - %d - Target:%d\n", m_BufferIDs[m_NextBufferIndex], m_Target );
    }
    else
    {
        //LOGInfo( LOGTag, "Buffer_OpenGL::Rebuild() rebuilding bytesToBuffer(%d) m_DataSize(%d)\n", bytesToBuffer, m_DataSize );
        
        if( bytesToBuffer > bufferSize )
        {
            // TODO: Test this code, it makes little sense, offset is ignored.
            MyAssert( offset == 0 );

            g_pRenderer->BufferData( this, m_BufferIDs[m_NextBufferIndex], bytesToBuffer, pData );
        }
        else
        {
            // TODO: Test this code, it makes little sense, offset is ignored on iOS and glBufferSubData might overrun the old buffer size.
            MyAssert( offset == 0 );
            MyAssert( offset + bytesToBuffer <= bufferSize );

#if MYFW_IOS
            g_pRenderer->BufferData( this, m_BufferIDs[m_NextBufferIndex], bytesToBuffer, m_pData );
#else
            g_pRenderer->BufferSubData( this, m_BufferIDs[m_NextBufferIndex], offset, bytesToBuffer, pData );
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

        //if( bytesToBuffer > m_DataSize )
        //    LOGError( LOGTag, "glBufferSubData bytesToBuffer(%d) > m_DataSize(%d)\n", bytesToBuffer, m_DataSize );
        //if( checkGlError( "Buffer_OpenGL::Rebuild glBufferSubData" ) == true )
        //    LOGError( LOGTag, "glBufferSubData - offset(%d), size(%d), m_DataSize(%d)\n", offset, bytesToBuffer, m_DataSize );
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
}

void Buffer_OpenGL::Invalidate(bool cleanGLAllocs)
{
    if( cleanGLAllocs )
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

    checkGlError( "Invalidate" );
}

void Buffer_OpenGL::CreateAndBindVAO()
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

    checkGlError( "CreateAndBindVAO" );
}

void Buffer_OpenGL::ResetVAOs()
{
    for( int i=0; i<3; i++ )
    {
        m_VAOInitialized[i] = false;
    }
}

void Buffer_OpenGL::FreeBufferedData()
{
}

void Buffer_OpenGL::InitializeBuffer(void* pData, unsigned int dataSize, MyRE::BufferTypes bufferType, MyRE::BufferUsages bufferUsage, bool bufferData, unsigned int numBuffersToAllocate, int bytesPerIndex, const char* category, const char* desc)
{
    InitializeBuffer( false, pData, dataSize, bufferType, bufferUsage, bufferData, numBuffersToAllocate, (VertexFormats)bytesPerIndex, nullptr, category, desc );
}

void Buffer_OpenGL::InitializeBuffer(bool dataSizeChanged, void* pData, unsigned int dataSize, MyRE::BufferTypes bufferType, MyRE::BufferUsages bufferUsage, bool bufferData, unsigned int numBuffersToAllocate, VertexFormats format, VertexFormat_Dynamic_Desc* pVertexFormatDesc, const char* category, const char* desc)
{
    MyAssert( numBuffersToAllocate >= 1 && numBuffersToAllocate <= 3 );

    if( dataSizeChanged )
    {
        for( int i=0; i<3; i++ )
        {
            if( m_BufferIDs[i] != 0 )
            {
                MyDeleteBuffers( 1, &m_BufferIDs[i] );
                m_BufferIDs[i] = 0;
            }
        }
    }

    m_NumBuffersToUse = numBuffersToAllocate;
    m_BufferType = bufferType;
    m_BufferUsage = bufferUsage;
    m_VertexFormat = format;
    m_pFormatDesc = pVertexFormatDesc;
}
