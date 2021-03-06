//
// Copyright (c) 2016-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __EventTypeManager_H__
#define __EventTypeManager_H__

class EventTypeManager;
class MyEvent;

enum EventTypes // ADDING_NEW_EventType
{
    Event_Undefined,
    Event_MaterialFinishedLoading,
    Event_ShaderFinishedLoading,
    Event_IAP,
    Event_NumEventTypes,
};

struct EventTypeInfo
{
    uint32 type;
    const char* name;
};

class EventTypeManager
{
protected:
#if MYFW_EDITOR
    std::vector<EventTypeInfo> m_RegisteredEvents;
#endif

    bool CheckForHashCollision(const char* name, bool assertIfDuplicate);

public:
    EventTypeManager();
    virtual ~EventTypeManager();

    uint32 GetNumberOfEventTypes();

    const char* GetTypeCategory(uint32 type);
    const char* GetTypeName(uint32 type);

    bool IsTypeRegistered(const char* name, bool assertIfDuplicate);

    uint32 RegisterEventType(const char* name, bool assertIfDuplicate);
};

#endif //__EventTypeManager_H__
