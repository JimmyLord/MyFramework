//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __Buffer_Base_H__
#define __Buffer_Base_H__

#include "Renderer_Enums.h"
#include "../../Shaders/VertexFormats.h"

class Buffer_Base
{
protected:

public:
    Buffer_Base();
    virtual ~Buffer_Base();

    // Getters.
    virtual MyRE::IndexTypes GetIBOType() = 0;

    // Other.
    virtual void TempBufferData(unsigned int sizeInBytes, void* pData) = 0;
    virtual void Rebuild(unsigned int offset, unsigned int sizeInBytes, bool forceRebuild, unsigned int bufferSize, void* pData) = 0;
    virtual void Invalidate(bool cleanGLAllocs) = 0;

    virtual void CreateAndBindVAO() = 0;
    virtual void ResetVAOs() = 0;

    virtual void FreeBufferedData() = 0;
    virtual void InitializeBuffer(void* pData, unsigned int dataSize, MyRE::BufferTypes bufferType, MyRE::BufferUsages bufferUsage, bool bufferData, unsigned int numBuffersToAllocate, int bytesPerIndex, const char* category, const char* desc) = 0;
    virtual void InitializeBuffer(bool dataSizeChanged, void* pData, unsigned int dataSize, MyRE::BufferTypes bufferType, MyRE::BufferUsages bufferUsage, bool bufferData, unsigned int numBuffersToAllocate, VertexFormats format, VertexFormat_Dynamic_Desc* pVertexFormatDesc, const char* category, const char* desc) = 0;
};

#endif //__Buffer_Base_H__
