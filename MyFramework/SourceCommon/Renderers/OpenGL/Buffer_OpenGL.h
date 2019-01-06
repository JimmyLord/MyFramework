//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __Buffer_OpenGL_H__
#define __Buffer_OpenGL_H__

#include "../BaseClasses/Buffer_Base.h"

class Buffer_OpenGL : public Buffer_Base
{
    friend class Shader_OpenGL;
    friend class Renderer_OpenGL;
    friend class BufferDefinition; // TODO: Remove this friend and add accessors.

private:
#if _DEBUG && MYFW_WINDOWS
    int m_DEBUG_CurrentVAOIndex;
    GLuint m_DEBUG_VBOUsedOnCreation[3];
    GLuint m_DEBUG_IBOUsedOnCreation[3];
    int m_DEBUG_LastFrameUpdated;
#endif

protected:
    GLuint m_BufferIDs[3]; // Up to 3 buffers created for double/triple buffering data.
    GLuint m_VAOHandles[3]; // Used only for vbo's ATM.
    bool m_VAOInitialized[3];

    unsigned int m_NumBuffersToUse;
    unsigned int m_CurrentBufferIndex;
    unsigned int m_NextBufferIndex;

    GLuint m_CurrentBufferID;
    GLuint m_CurrentVAOHandle;

    MyRE::BufferTypes m_BufferType;
    union
    {
        VertexFormats m_VertexFormat; // Sanity check for GL_ARRAY_BUFFER'S, if this is a vertex buffer.
        int m_BytesPerIndex; // If this is an index buffer.
    };
    VertexFormat_Dynamic_Desc* m_pFormatDesc;
    MyRE::BufferUsages m_BufferUsage;

public:
    Buffer_OpenGL();
    virtual ~Buffer_OpenGL();

    // Getters.
    virtual MyRE::IndexTypes GetIBOType() override;

    // Other.
    virtual void TempBufferData(unsigned int sizeInBytes, void* pData) override;
    virtual void Rebuild(unsigned int offset, unsigned int sizeInBytes, bool forceRebuild, unsigned int bufferSize, void* pData) override;
    virtual void Invalidate(bool cleanGLAllocs) override;

    virtual void CreateAndBindVAO() override;
    virtual void ResetVAOs() override;

    virtual void FreeBufferedData() override;
    virtual void InitializeBuffer(void* pData, unsigned int dataSize, MyRE::BufferTypes bufferType, MyRE::BufferUsages bufferUsage, bool bufferData, unsigned int numBuffersToAllocate, int bytesPerIndex, const char* category, const char* desc) override;
    virtual void InitializeBuffer(bool dataSizeChanged, void* pData, unsigned int dataSize, MyRE::BufferTypes bufferType, MyRE::BufferUsages bufferUsage, bool bufferData, unsigned int numBuffersToAllocate, VertexFormats format, VertexFormat_Dynamic_Desc* pVertexFormatDesc, const char* category, const char* desc) override;
};

#endif //__Buffer_OpenGL_H__
