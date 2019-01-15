//
// Copyright (c) 2016-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "EventTypeManager.h"

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

bool EventTypeManager::CheckForHashCollision(const char* name, bool assertIfDuplicate)
{
#if MYFW_EDITOR
    uint32 type = hash_djb2( name );

    if( type < Event_NumEventTypes )
    {
        if( strcmp( g_EngineEventTypeInfo[type].name, name ) != 0 )
        {
            LOGError( LOGTag, "EventType Hash Collision: %s produces a same hash as %s (%d)\n", name, g_EngineEventTypeInfo[type].name, type );
            MyAssert( assertIfDuplicate == false );
            return true;
        }
    }

    for( unsigned int i=0; i<m_RegisteredEvents.size(); i++ )
    {
        if( m_RegisteredEvents[i].type == type )
        {
            if( strcmp( m_RegisteredEvents[i].name, name ) != 0 )
            {
                LOGError( LOGTag, "EventType Hash Collision: %s produces a same hash as %s (%d)\n", name, m_RegisteredEvents[i].name, type );
                MyAssert( assertIfDuplicate == false );
                return true;
            }
        }
    }
#endif

    return false;
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

bool EventTypeManager::IsTypeRegistered(const char* name, bool assertIfDuplicate)
{
#if MYFW_EDITOR
    uint32 type = hash_djb2( name );

    bool found = false;

    for( unsigned int i=0; i<m_RegisteredEvents.size(); i++ )
    {
        if( m_RegisteredEvents[i].type == type )
        {
            if( strcmp( m_RegisteredEvents[i].name, name ) == 0 )
            {
                found = true;
            }
            else
            {
                LOGError( LOGTag, "EventType Hash Collision: %s produces a same hash as (%d)\n", name, m_RegisteredEvents[i].name, type );
                MyAssert( assertIfDuplicate == false );
            }
        }
    }

    return found;

#endif //MYFW_EDITOR

    return false;
}

uint32 EventTypeManager::RegisterEventType(const char* name, bool assertIfDuplicate)
{
#if MYFW_EDITOR
    // Create a new event type.
    EventTypeInfo newInfo;
    newInfo.type = hash_djb2( name );
    newInfo.name = name;

    if( CheckForHashCollision( name, assertIfDuplicate ) == 0 )
    {
        // If no hash collision was found, push this new event type into the vector.
        m_RegisteredEvents.push_back( newInfo );
    }

    return newInfo.type;
#endif

    return 0;
}