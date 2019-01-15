//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "MyThread.h"

#if USE_PTHREAD

static void* GlobalStartThread(void* arg)
{
    return ((MyThread*)arg)->Run();
}

MyThread::MyThread()
{
    //m_ThreadID = 0; // can't be initialized, is valid only if m_IsRunning is true

    m_IsRunning = false;
    m_IsDetached = false;
}

MyThread::~MyThread()
{
    if( m_IsRunning == true && m_IsDetached == false )
    {
        pthread_detach( m_ThreadID );
    }

    if( m_IsRunning == true )
    {
#if !MYFW_NACL // TODO: is there an alternative for NACL?
        pthread_cancel( m_ThreadID );
#endif
    }
}

int MyThread::Start()
{
    MyAssert( m_IsRunning == false );

    int result = pthread_create( &m_ThreadID, NULL, GlobalStartThread, this );
    MyAssert( result == 0 ); // assert pthread_create succeeded
    
    if( result == 0 )
        m_IsRunning = true;

    return result;
}

int MyThread::Join()
{
    if( m_IsRunning == false )
        return -1;

    int result = pthread_join( m_ThreadID, 0 );
    if( result == 0 )
        m_IsDetached = true;

    return result;
}

int MyThread::Detach()
{
    if( m_IsRunning == false || m_IsDetached == true )
        return -1;

    int result = pthread_detach( m_ThreadID );
    if( result == 0 )
        m_IsDetached = true;

    return result;
}

#endif //USE_PTHREAD
