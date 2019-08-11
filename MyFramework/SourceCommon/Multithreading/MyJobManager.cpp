//
// Copyright (c) 2016-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "MyJobManager.h"
#include "MyThread.h"

#if USE_PTHREAD
class MyJobThread : public MyThread
{
protected:
    MyJobManager* m_pJobManager;

public:
    MyJobThread(MyJobManager* pJobManager) : m_pJobManager(pJobManager)
    {
    }

    virtual void* Run()
    {
        while( true )
        {
            // Ask for a job, block while waiting for one to come in.
            MyJob* pJob = m_pJobManager->RemoveJob( m_ThreadID );

            // If the job given by the manager is nullptr, then exit the thread.
            if( pJob == nullptr )
            {
                //LOGInfo( "Threaded Job Manager", "Thread %d exiting\n", m_ThreadID );
                return nullptr;
            }

            // Do the job.
            pJob->MarkAsStarted();
            pJob->DoWork();
            pJob->MarkAsFinished();

            // TODO: Move job to a completed list, then tick jobmanager each frame to call completed callbacks.

            //LOGInfo( "Threaded Job Manager", "Thread %d finished job\n", m_ThreadID );
        }
    }
};
#endif //USE_PTHREAD

#if USE_PTHREAD
MyJobManager::MyJobManager()
{
    pthread_mutex_init( &m_JobListMutex, nullptr );
    pthread_cond_init( &m_JobAvailableConditional, nullptr );

    for( int i=0; i<MAX_THREADS; i++ )
    {
        m_pThreads[i] = MyNew MyJobThread( this );
        m_pThreads[i]->Start();
    }

    m_ShuttingDown = false;
}

MyJobManager::~MyJobManager()
{
    pthread_mutex_lock( &m_JobListMutex );
    
    m_ShuttingDown = true;

    {
        // Destroy all outstanding jobs to allow threads to shutdown.
        while( m_JobList.GetHead() )
        {
            MyJob* pJob = m_JobList.RemHead();
            pJob->Prev = nullptr;
            pJob->Next = nullptr;
            // Jobs are not deleted, must be managed by other code.
            // TODO: Add job complete callback function, return "job failed".
        }

        // Send a signal to each thread that a job is ready, RemoveJob() will return 0, which will kill the thread.
        pthread_cond_broadcast( &m_JobAvailableConditional );
    }

    pthread_mutex_unlock( &m_JobListMutex );

    // Wait for all threads to exit, then delete them.
    for( int i=0; i<MAX_THREADS; i++ )
    {
        m_pThreads[i]->Join();
        SAFE_DELETE( m_pThreads[i] );
    }

    pthread_mutex_destroy( &m_JobListMutex );
    pthread_cond_destroy( &m_JobAvailableConditional );
}

void MyJobManager::GetJobListMutexLock()
{
    pthread_mutex_lock( &m_JobListMutex );
}

void MyJobManager::ReleaseJobListMutexLock()
{
    pthread_mutex_unlock( &m_JobListMutex );
}

void MyJobManager::AddJob(MyJob* pJob, bool lockMutex)
{
    MyAssert( pJob->Prev == nullptr );

    if( lockMutex )
        pthread_mutex_lock( &m_JobListMutex );
    
    m_JobList.AddTail( pJob );

    // Wake up a single thread so it can do this job.
    pthread_cond_signal( &m_JobAvailableConditional );

    if( lockMutex )
        pthread_mutex_unlock( &m_JobListMutex );
}

MyJob* MyJobManager::RemoveJob(pthread_t threadID)
{
    pthread_mutex_lock( &m_JobListMutex );

    while( m_JobList.GetHead() == nullptr && m_ShuttingDown == false )
    {
        //LOGInfo( "Threaded Job Manager", "Thread %d going to sleep waiting for job\n", threadID );
        pthread_cond_wait( &m_JobAvailableConditional, &m_JobListMutex );
    }

    // Grab item from list, will be 0 if list is empty (which will cause thread to die).
    // List is emptied in destructor when shutting down.
    //LOGInfo( "Threaded Job Manager", "Thread %d taking a job\n", threadID );
    MyJob* pJob = (MyJob*)m_JobList.RemHead();
    if( pJob )
    {
        pJob->Prev = nullptr;
        pJob->Next = nullptr;
    }

    pthread_mutex_unlock( &m_JobListMutex );

    return pJob;
}

void MyJobManager::WaitForJobToComplete(MyJob* pJob)
{
    // If the job isn't queued up, just return immediately.
    if( pJob->IsQueued() == false )
        return;

    // Wait for the job to complete.
    while( pJob->IsFinished() == false );
}
#else //USE_PTHREAD
MyJobManager::MyJobManager()
{
    m_ShuttingDown = false;
}

MyJobManager::~MyJobManager()
{
    m_ShuttingDown = true;
}

void MyJobManager::GetJobListMutexLock()
{
}

void MyJobManager::ReleaseJobListMutexLock()
{
}

void MyJobManager::AddJob(MyJob* pJob, bool lockMutex)
{
    pJob->MarkAsStarted();
    pJob->DoWork();
    pJob->MarkAsFinished();
}

void MyJobManager::WaitForJobToComplete(MyJob* pJob)
{
}
#endif //USE_PTHREAD
