//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
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

extern EventTypeManager* g_pEventTypeManager;

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
    const char* name;
};

class EventTypeManager
{
protected:

public:
    EventTypeManager();
    virtual ~EventTypeManager();

    virtual unsigned int GetNumberOfEventTypes();

    virtual const char* GetTypeCategory(int type);
    virtual const char* GetTypeName(int type);

    virtual int GetTypeByName(const char* name);
};

#endif //__EventTypeManager_H__
