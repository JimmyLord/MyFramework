//
// Copyright (c) 2016-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "EventManager.h"
#include "EventTypeManager.h"
#include "../Helpers/FileManager.h"

EventManager* g_pEventManager = 0;

EventManager::EventManager()
{
    m_pEventPool.AllocateObjects( MAX_EVENTS );
    m_pEventArgumentPool.AllocateObjects( MAX_EVENT_ARGUMENTS );
    m_pEventHandlerPool.AllocateObjects( MAX_EVENT_HANDLERS );

    m_NumEventHandlers = 0;
}

EventManager::~EventManager()
{
    MyEvent* pNextEvent = 0;
    for( MyEvent* pEvent = m_pEventQueue.GetHead(); pEvent; pEvent = pNextEvent )
    {
        pNextEvent = pEvent->GetNext();

        ReleaseEvent( pEvent );
        pEvent->Remove();
    }

    for( unsigned int i=0; i<m_NumEventHandlers; i++ )
    {
        m_pEventHandlerPool.ReturnObjectToPool( m_pEventHandlers[i] );
    }
}

void EventManager::ReleaseEvent(MyEvent* pEvent)
{
    pEvent->ClearArguments();
    m_pEventPool.ReturnObjectToPool( pEvent );
}

void EventManager::Tick(float deltaTime)
{
    MyEvent* pNextEvent = 0;
    for( MyEvent* pEvent = m_pEventQueue.GetHead(); pEvent; pEvent = pNextEvent )
    {
        pNextEvent = pEvent->GetNext();

        SendEventNow( pEvent );
        pEvent->Remove();
    }
}

MyEvent* EventManager::CreateNewEvent(const char* name)
{
    EventHashType type = g_pEventTypeHashFunc( name );
    return CreateNewEvent( type );
}

void EventManager::RegisterForEvents(const char* name, void* pObject, EventCallbackFunc pOnEventFunction)
{
#if MYFW_EDITOR
    // Currently a safety check for duplicate hashes, so only done in editor mode.
    // Check if this event type is registered, if not, register it.
    if( g_pEventTypeManager->IsTypeRegistered( name, true ) == false )
    {
        g_pEventTypeManager->RegisterEventType( name, true );
    }
#endif

    EventHashType type = g_pEventTypeHashFunc( name );
    RegisterForEvents( type, pObject, pOnEventFunction );
}

void EventManager::UnregisterForEvents(const char* name, void* pObject, EventCallbackFunc pOnEventFunction)
{
    EventHashType type = g_pEventTypeHashFunc( name );
    return UnregisterForEvents( type, pObject, pOnEventFunction );
}

MyEvent* EventManager::CreateNewEvent(EventHashType hash)
{
    MyEvent* pEvent = m_pEventPool.GetObjectFromPool();
    MyAssert( pEvent->GetFirstArgument() == 0 );

    pEvent->SetType( hash );

    return pEvent;
}

void EventManager::RegisterForEvents(EventHashType hash, void* pObject, EventCallbackFunc pOnEventFunction)
{
    MyEventHandler* pEventHandler = m_pEventHandlerPool.GetObjectFromPool();
    
    pEventHandler->m_EventTypeHash = hash;
    pEventHandler->m_pObject = pObject;
    pEventHandler->m_pOnEventFunction = pOnEventFunction;

    if( m_NumEventHandlers < MAX_EVENT_HANDLERS )
    {
        m_pEventHandlers[m_NumEventHandlers] = pEventHandler;
        m_NumEventHandlers++;
    }
    else
    {
        LOGError( LOGTag, "Too many event handlers!\n" );
    }
}

void EventManager::UnregisterForEvents(EventHashType hash, void* pObject, EventCallbackFunc pOnEventFunction)
{
    for( unsigned int i=0; i<m_NumEventHandlers; i++ )
    {
        MyAssert( m_pEventHandlers[i] );

        if( m_pEventHandlers[i]->m_EventTypeHash == hash &&
            m_pEventHandlers[i]->m_pObject == pObject &&
            m_pEventHandlers[i]->m_pOnEventFunction == pOnEventFunction )
        {
            m_pEventHandlerPool.ReturnObjectToPool( m_pEventHandlers[i] );

            m_pEventHandlers[i] = m_pEventHandlers[m_NumEventHandlers-1];
            m_pEventHandlers[m_NumEventHandlers-1] = 0;
            m_NumEventHandlers--;
            return;
        }
    }
}

void EventManager::QueueEvent(MyEvent* pEvent)
{
    m_pEventQueue.AddTail( pEvent );
}

void EventManager::SendEventNow(MyEvent* pEvent)
{
    for( unsigned int i=0; i<m_NumEventHandlers; i++ )
    {
        MyAssert( m_pEventHandlers[i] );

        if( m_pEventHandlers[i]->m_EventTypeHash == pEvent->GetType() )
        {
            (m_pEventHandlers[i]->m_pOnEventFunction)( m_pEventHandlers[i]->m_pObject, pEvent );
        }
    }

    ReleaseEvent( pEvent );
}
