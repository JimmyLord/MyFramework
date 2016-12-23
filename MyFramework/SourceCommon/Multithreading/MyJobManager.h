//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

// tutorial: https://vichargrave.github.io/articles/2013-01/multithreaded-work-queue-in-cpp

class MyJobManager;

extern MyJobManager* g_pMyJobManager;

class MyJob : public CPPListNode
{
protected:
    bool m_IsStarted;
    bool m_IsFinished;

public:
    MyJob()
    {
        m_IsStarted = false;
        m_IsFinished = false;
    }

    virtual ~MyJob()
    {
    }

    void MarkAsStarted()
    {
        m_IsStarted = true;
    }

    virtual void DoWork() = 0;

    void MarkAsFinished()
    {
        m_IsFinished = true;
    }
};

class MyJobManager
{
    friend class MyJobThread;
    static const int MAX_THREADS = 8;

protected:
    MyThread* m_pThreads[MAX_THREADS];

    CPPListHead m_JobList;
    pthread_mutex_t m_JobListMutex;
    pthread_cond_t m_JobAvailableConditional;

    bool m_ShuttingDown;

protected:
    MyJob* RemoveJob(pthread_t threadid);

public:
    MyJobManager();
    virtual ~MyJobManager();

    void AddJob(MyJob* pItem);
};
