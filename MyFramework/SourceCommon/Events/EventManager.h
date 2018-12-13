//
// Copyright (c) 2016-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __EventManager_H__
#define __EventManager_H__

class EventManager;

extern EventManager* g_pEventManager;

typedef bool (*EventCallbackFunc)(void* pObjectPtr, MyEvent* pEvent);

class MyEventHandler
{
public:
    EventHashType m_EventTypeHash; // Either from EventTypes enum or calculated via hash (hash_djb).
    void* m_pObject;
    EventCallbackFunc m_pOnEventFunction;

public:
    MyEventHandler()
    {
        m_EventTypeHash = 0;

        m_pObject = 0;
        m_pOnEventFunction = 0;
    }
};

class EventManager
{
    static const unsigned int MAX_EVENTS = 10000;
    static const unsigned int MAX_EVENT_ARGUMENTS = 40000;
    static const unsigned int MAX_EVENT_HANDLERS = 1000;

    friend class MyEvent; // allow access to m_pEventArgumentPool to clear arguments.

protected:
    MySimplePool<MyEvent> m_pEventPool;
    MySimplePool<MyEventArgument> m_pEventArgumentPool;
    MySimplePool<MyEventHandler> m_pEventHandlerPool;

    TCPPListHead<MyEvent*> m_pEventQueue; // list of all active events, TODO: make linked list

    MyEventHandler* m_pEventHandlers[MAX_EVENT_HANDLERS]; // list of all active event handlers, TODO: make linked list, one per type?
    unsigned int m_NumEventHandlers;

protected:
    void ReleaseEvent(MyEvent* pEvent);

public:
    EventManager();
    ~EventManager();

    void Tick(float deltaTime);

    MyEvent* CreateNewEvent(const char* name);
    void RegisterForEvents(const char* name, void* pObject, EventCallbackFunc pOnEventFunction);
    void UnregisterForEvents(const char* name, void* pObject, EventCallbackFunc pOnEventFunction);

    MyEvent* CreateNewEvent(EventHashType hash);
    void RegisterForEvents(EventHashType hash, void* pObject, EventCallbackFunc pOnEventFunction);
    void UnregisterForEvents(EventHashType hash, void* pObject, EventCallbackFunc pOnEventFunction);

    void QueueEvent(MyEvent* pEvent);
    void SendEventNow(MyEvent* pEvent);
};

#endif //__EventManager_H__
