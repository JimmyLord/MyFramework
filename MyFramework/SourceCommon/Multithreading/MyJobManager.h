//
// Copyright (c) 2016-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

// tutorial: https://vichargrave.github.io/articles/2013-01/multithreaded-work-queue-in-cpp

class MyJobManager;
class MyThread;

class MyJob : public TCPPListNode<MyJob*>
{
    friend class MyJobManager;
    friend class MyJobThread;

protected:
    bool m_IsStarted  : 1; // True when removed from MyJobManager::m_JobList and given to a thread.
    bool m_IsActive   : 1; // True when running as the active job in one of MyJobManager::m_pThreads.
    bool m_IsFinished : 1; // True when job is complete, but might still be sitting in a queue to send notifications (not implemented).

    virtual void DoWork() = 0;

    void MarkAsStarted() { m_IsStarted = true; m_IsActive = true; }
    void MarkAsFinished() { m_IsActive = false; m_IsFinished = true; }

public:
    MyJob()
    {
        m_IsStarted = false;
        m_IsActive = false;
        m_IsFinished = false;
    }

    virtual ~MyJob()
    {
    }

    bool IsQueued() { return this->Prev != nullptr; } // If we're in a cpplist, we're queued.
    bool IsStarted() { return m_IsStarted; }
    bool IsActive() { return m_IsActive; }
    bool IsFinished() { return m_IsFinished; }
    void Reset() { MyAssert( m_IsFinished == true ); m_IsStarted = m_IsActive = m_IsFinished = false; }
};

class JobWithCallbackFunction : public MyJob
{
    typedef void JobCallbackFunction(void* pObject);

protected:
    void* m_pObject;
    JobCallbackFunction* m_pFunction;

public:
    JobWithCallbackFunction(void* pObject, JobCallbackFunction* pFunction)
    {
        m_pObject = pObject;
        m_pFunction = pFunction;
    }
    virtual ~JobWithCallbackFunction() {}

    virtual void DoWork()
    {
        m_pFunction( m_pObject );
    }
};

class MyJobManager
{
    friend class MyJobThread;
    static const int MAX_THREADS = 3;

protected:
#if USE_PTHREAD
    MyJobThread* m_pThreads[MAX_THREADS];

    pthread_mutex_t m_JobListMutex;
    pthread_cond_t m_JobAvailableConditional;
#endif

    TCPPListHead<MyJob*> m_JobList;

    bool m_ShuttingDown;

protected:
#if USE_PTHREAD
    MyJob* RemoveJob(pthread_t threadID);
#endif

public:
    MyJobManager();
    virtual ~MyJobManager();

    void GetJobListMutexLock();
    void ReleaseJobListMutexLock();

    void AddJob(MyJob* pJob, bool lockMutex = true);
    void WaitForJobToComplete(MyJob* pJob);
};
