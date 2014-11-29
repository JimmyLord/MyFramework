//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __BufferManager_H__
#define __BufferManager_H__

class BufferManager;
class RefCount;

extern BufferManager* g_pBufferManager;

// ATM this whole buffer system creates a cpu memory block that mirrors what should be copied into OpenGL memory.

class BufferDefinition : public CPPListNode, public RefCount
{
    friend class BufferManager;
    friend class PanelMemory;

protected:
    GLuint m_BufferIDs[3]; // up to 3 buffers created for double/triple buffering data.
    GLuint m_VAOHandles[ShaderPass_NumTypes][3]; // used only for vbo's ATM.
    unsigned int m_NumBuffersToUse;
    unsigned int m_CurrentBufferIndex;
    unsigned int m_NextBufferIndex;

public:
#if _DEBUG && MYFW_WINDOWS
    Shader_Base* m_DEBUG_ShaderUsedOnCreation[ShaderPass_NumTypes];
    int m_DEBUG_CurrentVAOIndex[ShaderPass_NumTypes];
    GLuint m_DEBUG_VBOUsedOnCreation[3];
    GLuint m_DEBUG_IBOUsedOnCreation[3];
    int m_DEBUG_LastFrameUpdated;
#endif

    GLuint m_CurrentBufferID;
    bool m_CurrentVAOInitialized[ShaderPass_NumTypes];
    GLuint m_CurrentVAOHandle[ShaderPass_NumTypes];

    char* m_pData; // only using char* because android compiler doesn't like deleting void*(warning : deleting 'void*' is undefined)
    unsigned int m_DataSize;
    union
    {
        VertexFormats m_VertexFormat; // sanity check for GL_ARRAY_BUFFER'S, if this is a vertex buffer.
        int m_BytesPerIndex; // if this is an index buffer.
    };
    GLenum m_Target;
    GLenum m_Usage;
    bool m_Dirty;

public:
    BufferDefinition();
    virtual ~BufferDefinition();
    void Rebuild(unsigned int offset, unsigned int sizeinbytes, bool forcerebuild = false);
    void Invalidate(bool cleanglallocs);

    void CreateAndBindVAO();

    void FreeBufferedData();
    void InitializeBuffer(void* pData, unsigned int datasize, GLenum target, GLenum usage, bool bufferdata, unsigned int numbufferstoallocate, int bytesperindex);
    void InitializeBuffer(void* pData, unsigned int datasize, GLenum target, GLenum usage, bool bufferdata, unsigned int numbufferstoallocate, VertexFormats format);
};

//class VAODefinition : public CPPListNode, public RefCount
//{
//    friend class BufferManager;
//    friend class PanelMemory;
//
//public:
//#if _DEBUG && MYFW_WINDOWS
//    GLuint m_DEBUG_VBOUsedOnCreation;
//    GLuint m_DEBUG_IBOUsedOnCreation;
//#endif
//
//public:
//    GLuint m_Handle;
//    bool m_Initialized;
//
//public:
//    VAODefinition();
//    virtual ~VAODefinition();
//    void Invalidate(bool cleanglallocs);
//
//    void Create();
//};

class BufferManager
{
    friend class PanelMemory;

protected:
    CPPListHead m_Buffers;
    //CPPListHead m_VAOs;

public:
    BufferManager();
    virtual ~BufferManager();

    // pData pointer passed in will be deleted by the BufferDefinition.
    BufferDefinition* CreateBuffer(const char* category, const char* desc);
    BufferDefinition* CreateBuffer(void* pData, unsigned int datasize, GLenum target, GLenum usage, bool bufferdata, unsigned int numbufferstoallocate, int bytesperindex, const char* category, const char* desc);
    BufferDefinition* CreateBuffer(void* pData, unsigned int datasize, GLenum target, GLenum usage, bool bufferdata, unsigned int numbufferstoallocate, VertexFormats format, const char* category, const char* desc);
    //VAODefinition* CreateVAO();
    void Tick();

    void InvalidateAllBuffers(bool cleanglallocs);

    unsigned int CalculateTotalMemoryUsedByBuffers();

private:
    void FreeAllBuffers();
};

#endif //__BufferManager_H__
