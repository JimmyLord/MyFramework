//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "MyStackAllocator.h"

MyStackAllocator::MyStackAllocator()
{
    m_pMemory = 0;
    m_pCurrentPointer = 0;

    m_SizeInBytes = 0;
}

MyStackAllocator::~MyStackAllocator()
{
}

void MyStackAllocator::Initialize(unsigned int sizeinbytes)
{
    m_pMemory = MyNew char[sizeinbytes];
    m_pCurrentPointer = m_pMemory;

    m_SizeInBytes = sizeinbytes;
}

void MyStackAllocator::Cleanup()
{
    delete[] m_pMemory;
    m_pMemory = nullptr;
}

MyStackAllocator::MyStackPointer MyStackAllocator::GetCurrentLocation()
{
    return m_pCurrentPointer;
}

void MyStackAllocator::RewindStack(MyStackAllocator::MyStackPointer ptr)
{
    m_pCurrentPointer = ptr;
}

void MyStackAllocator::Clear()
{
    m_pCurrentPointer = m_pMemory;
}

void* MyStackAllocator::AllocateBlock(unsigned int byteswanted, MyStackPointer* pStartLocation)
{
    MyAssert( m_pCurrentPointer + byteswanted < m_pMemory + m_SizeInBytes );

    if( m_pCurrentPointer + byteswanted >= m_pMemory + m_SizeInBytes )
        return 0;

    if( pStartLocation )
        *pStartLocation = m_pCurrentPointer;

    void* pPointer = m_pCurrentPointer;

    // always place the current pointer on the next 8 byte boundary
    m_pCurrentPointer += byteswanted + (8 - byteswanted%8);

    return pPointer;
}

unsigned int MyStackAllocator::GetBytesUsed()
{
    return (int)(m_pCurrentPointer - m_pMemory);
}
