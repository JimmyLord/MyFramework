//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

MyJobManager* g_pMyJobManager = 0;

class MyJobThread : public MyThread
{
protected:
public:
    virtual void* Run()
    {
        while( 1 )
        {
            // ask for a job, block while waiting for one to come in
            MyJob* pJob = g_pMyJobManager->RemoveJob( m_ThreadID );

            // if the job given by the manager is 0, then exit the thread
            if( pJob == 0 )
            {
                //LOGInfo( "Threaded Job Manager", "Thread %d exiting\n", m_ThreadID );
                return 0;
            }

            // do the job
            pJob->MarkAsStarted();
            pJob->DoWork();
            pJob->MarkAsFinished();

            // TODO: move job to a completed list, then tick jobmanager each frame to call completed callbacks

            //LOGInfo( "Threaded Job Manager", "Thread %d finished job\n", m_ThreadID );
        }
    }
};

MyJobManager::MyJobManager()
{
    g_pMyJobManager = this;

    pthread_mutex_init( &m_JobListMutex, 0 );
    pthread_cond_init( &m_JobAvailableConditional, 0 );

    for( int i=0; i<MAX_THREADS; i++ )
    {
        m_pThreads[i] = MyNew MyJobThread;
        m_pThreads[i]->Start();
    }

    m_ShuttingDown = false;
}

MyJobManager::~MyJobManager()
{
    pthread_mutex_lock( &m_JobListMutex );
    
    m_ShuttingDown = true;

    {
        // destroy all outstanding jobs to allow threads to shutdown
        while( m_JobList.GetHead() )
        {
            CPPListNode* pJob = m_JobList.RemHead();
            // jobs are not deleted, must be managed by other code
            // TODO: add job complete callback function, return "job failed"
        }

        // send a signal to each thread that a job is ready, remove job will return 0, which will kill the thread
        pthread_cond_broadcast( &m_JobAvailableConditional );
    }

    pthread_mutex_unlock( &m_JobListMutex );

    // wait for all threads to exit, then delete them
    for( int i=0; i<MAX_THREADS; i++ )
    {
        m_pThreads[i]->Join();
        SAFE_DELETE( m_pThreads[i] );
    }

    pthread_mutex_destroy( &m_JobListMutex );
    pthread_cond_destroy( &m_JobAvailableConditional );
}

void MyJobManager::AddJob(MyJob* pItem)
{
    pthread_mutex_lock( &m_JobListMutex );
    
    m_JobList.AddTail( pItem );

    // Wake up a single thread so it can do this job.
    pthread_cond_signal( &m_JobAvailableConditional );

    pthread_mutex_unlock( &m_JobListMutex );
}

MyJob* MyJobManager::RemoveJob(pthread_t threadid)
{
    pthread_mutex_lock( &m_JobListMutex );

    while( m_JobList.GetHead() == 0 && m_ShuttingDown == false )
    {
        //LOGInfo( "Threaded Job Manager", "Thread %d going to sleep waiting for job\n", threadid );
        pthread_cond_wait( &m_JobAvailableConditional, &m_JobListMutex );
    }

    // Grab item from list, will be 0 if list is empty (which will cause thread to die)
    // list is emptied in destructor when shutting down.
    //LOGInfo( "Threaded Job Manager", "Thread %d taking a job\n", threadid );
    MyJob* pItem = (MyJob*)m_JobList.RemHead();

    pthread_mutex_unlock( &m_JobListMutex );

    return pItem;
}
