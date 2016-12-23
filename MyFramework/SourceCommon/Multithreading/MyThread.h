//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

// tutorial: https://vichargrave.github.io/articles/2012-12/java-style-thread-class-in-cpp

#if USE_PTHREAD

class MyThread
{
protected:
    pthread_t m_ThreadID;
    bool m_IsRunning;
    bool m_IsDetached;

public:
    MyThread();
    virtual ~MyThread();
 
    int Start();
    int Join();
    int Detach();
    pthread_t GetThreadID() { return m_ThreadID; }
 
    virtual void* Run() = 0; 
};

#endif //USE_PTHREAD
