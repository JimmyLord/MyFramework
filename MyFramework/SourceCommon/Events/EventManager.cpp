//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#include "EventManager.h"
#include "../Helpers/FileManager.h"

EventManager* g_pEventManager = 0;

EventManager::EventManager()
{
    m_pEventPool.AllocateObjects( MAX_EVENTS );
    m_pEventArgumentPool.AllocateObjects( MAX_EVENT_ARGUMENTS );
    m_pEventHandlerPool.AllocateObjects( MAX_EVENT_HANDLERS );

    m_NumEvents = 0;
    m_NumEventHandlers = 0;
}

EventManager::~EventManager()
{
    for( unsigned int i=0; i<m_NumEvents; i++ )
    {
        m_pEvents[i]->ClearArguments();
        m_pEventPool.ReturnObjectToPool( m_pEvents[i] );
    }

    for( unsigned int i=0; i<m_NumEventHandlers; i++ )
    {
        m_pEventHandlerPool.ReturnObjectToPool( m_pEventHandlers[i] );
    }
}

void EventManager::Tick()
{
}

MyEvent* EventManager::CreateNewEvent(EventTypes type)
{
    MyEvent* pEvent = m_pEventPool.GetObjectFromPool();
    MyAssert( pEvent->GetFirstArgument() == 0 );

    pEvent->SetType( type );

    return pEvent;
}

void EventManager::ReleaseEvent(MyEvent* pEvent)
{
    pEvent->ClearArguments();
    m_pEventPool.ReturnObjectToPool( pEvent );
}

void EventManager::RegisterForEvents(EventTypes type, void* pObject, EventCallbackFunc pOnEventFunction)
{
    MyEventHandler* pEventHandler = m_pEventHandlerPool.GetObjectFromPool();
    
    pEventHandler->m_EventType = type;
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

void EventManager::UnregisterForEvents(EventTypes type, void* pObject, EventCallbackFunc pOnEventFunction)
{
    for( unsigned int i=0; i<m_NumEventHandlers; i++ )
    {
        MyAssert( m_pEventHandlers[i] );

        if( m_pEventHandlers[i]->m_EventType == type &&
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

void EventManager::SendEventNow(MyEvent* pEvent)
{
    for( unsigned int i=0; i<m_NumEventHandlers; i++ )
    {
        MyAssert( m_pEventHandlers[i] );

        if( m_pEventHandlers[i]->m_EventType == pEvent->GetType() )
        {
            (m_pEventHandlers[i]->m_pOnEventFunction)( m_pEventHandlers[i]->m_pObject, pEvent );
        }
    }
    //g_pGameCore->OnEvent( pEvent );

    ReleaseEvent( pEvent );
}
