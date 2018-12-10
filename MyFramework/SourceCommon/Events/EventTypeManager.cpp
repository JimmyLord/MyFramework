//
// Copyright (c) 2016-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

// must be in same order as enum EventTypes
EventTypeInfo g_EngineEventTypeInfo[Event_NumEventTypes] = // ADDING_NEW_EventType
{
    { Event_Undefined,                  "Undefined",                  },  //Event_Undefined,
    { Event_MaterialFinishedLoading,    "MaterialLoaded",             },  //Event_MaterialFinishedLoading,
    { Event_ShaderFinishedLoading,      "ShaderLoaded",               },  //Event_ShaderFinishedLoading,
    { Event_IAP,                        "IAP",                        },  //Event_IAP,
};

EventTypeManager* g_pEventTypeManager = 0;

EventTypeManager::EventTypeManager()
{
    g_pEventTypeManager = this;
}

EventTypeManager::~EventTypeManager()
{
}

uint32 EventTypeManager::GetNumberOfEventTypes()
{
    return 0;
}

const char* EventTypeManager::GetTypeCategory(uint32 type)
{
    return 0;
}

const char* EventTypeManager::GetTypeName(uint32 type)
{
    return 0;
}

uint32 EventTypeManager::GetTypeByName(const char* name)
{
    return 0;
}

void EventTypeManager::RegisterEventType(const char* name, bool assertIfDuplicate)
{
#if MYFW_EDITOR
    // Create a new event type.
    EventTypeInfo newInfo;
    newInfo.type = hash_djb2( name );
    newInfo.name = name;

    // Check for hash collisions.
    bool eventTypeHashCollision = false;

    if( newInfo.type < Event_NumEventTypes )
    {
        eventTypeHashCollision = true;
        LOGError( LOGTag, "EventType Hash Collision: %s produces a duplicate hash (%d)\n", newInfo.name, newInfo.type );
        LOGError( LOGTag, "EventType Hash Collision: old string is %s\n", g_EngineEventTypeInfo[newInfo.type].name );
        MyAssert( assertIfDuplicate == false );
        return;
    }

    for( unsigned int i=0; i<m_RegisteredEvents.size(); i++ )
    {
        if( m_RegisteredEvents[i].type == newInfo.type )
        {
            eventTypeHashCollision = true;
            LOGError( LOGTag, "EventType Hash Collision: %s produces a duplicate hash (%d)\n", newInfo.name, newInfo.type );
            LOGError( LOGTag, "EventType Hash Collision: old string is %s\n", m_RegisteredEvents[i].name );
            MyAssert( assertIfDuplicate == false );
            return;
        }
    }

    // If no hash collision was found, push this new event type into the vector.
    m_RegisteredEvents.push_back( newInfo );
#endif
}