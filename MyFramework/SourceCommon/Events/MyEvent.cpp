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
#include "MyEvent.h"

EventTypeHashFunction* g_pEventTypeHashFunc = &hash_djb2;

MyEvent::MyEvent()
{
    m_TypeHash = 0;
    m_FirstArgument = 0;
}

MyEvent::~MyEvent()
{
}

void MyEvent::CheckIfArgumentIsAlreadyAttached(MyEventArgument* pNewArg) // Protected
{
    MyEventArgument* pArg = m_FirstArgument;
    while( pArg )
    {
        if( pNewArg->m_NameHash == pArg->m_NameHash )
        {
            MyAssert( false );
            LOGError( LOGTag, "Duplicate argument attached to event\n" );
        }

        pArg = pArg->m_NextArgument;
    }
}

void MyEvent::AttachArgument(MyEventArgument* pArg) // Protected
{
    // Debug check for duplicate arguments, asserts(in debug) and puts up an error in log.
    CheckIfArgumentIsAlreadyAttached( pArg );

    // Attach this argument to this event as the first in the list.
    pArg->m_NextArgument = this->m_FirstArgument;
    this->m_FirstArgument = pArg;
}

bool MyEvent::IsType(const char* name)
{
    EventHashType type = g_pEventTypeHashFunc( name );
    return IsType( type );
}

bool MyEvent::IsType(EventHashType hash)
{
    if( m_TypeHash == hash )
        return true;

    return false;
}

void MyEvent::ClearArguments(EventManager* pEventManager)
{
    MyEventArgument* pArg = m_FirstArgument;
    while( pArg )
    {
        MyEventArgument* pNextArg = pArg->m_NextArgument;

        pArg->m_NextArgument = 0;
        pEventManager->m_pEventArgumentPool.ReturnObjectToPool( pArg );

        pArg = pNextArg;
    }
    m_FirstArgument = 0;
}

//====================================================================================================
// Create a function for each type of argument in ArgumentTypes enum.
//====================================================================================================
#define CREATE_ATTACH_ARGUMENT_FUNC(ArgumentName, ArgumentType) \
void MyEvent::Attach##ArgumentName(EventManager* pEventManager, const char* name, ArgumentType value) \
{ \
    MyEventArgument* pArg = pEventManager->m_pEventArgumentPool.GetObjectFromPool(); \
    pArg->m_NameHash = g_pEventTypeHashFunc( name ); \
    pArg->m_Type = MyEventArgument::Type_##ArgumentName; \
    pArg->m_##ArgumentName = value; \
    AttachArgument( pArg ); \
}

CREATE_ATTACH_ARGUMENT_FUNC( Pointer, void* );
CREATE_ATTACH_ARGUMENT_FUNC( Bool, bool );
CREATE_ATTACH_ARGUMENT_FUNC( Int, int32 );
CREATE_ATTACH_ARGUMENT_FUNC( UnsignedInt, uint32 );
CREATE_ATTACH_ARGUMENT_FUNC( Float, float );
CREATE_ATTACH_ARGUMENT_FUNC( Double, double );
//====================================================================================================

bool MyEvent::IsArgumentAttached(const char* name)
{
    EventHashType nameHash = g_pEventTypeHashFunc( name );

    MyEventArgument* pArg = m_FirstArgument;
    while( pArg )
    {
        if( nameHash == pArg->m_NameHash )
            return true;

        pArg = pArg->m_NextArgument;
    }

    return false;
}

MyEventArgument* MyEvent::GetArgument(const char* name)
{
    EventHashType nameHash = g_pEventTypeHashFunc( name );

    MyEventArgument* pArg = m_FirstArgument;
    while( pArg )
    {
        if( nameHash == pArg->m_NameHash )
            return pArg;

        pArg = pArg->m_NextArgument;
    }

    LOGError( LOGTag, "Event Argument not found (%s)\n", name );

    return 0;
}

//====================================================================================================
// Create a function for each type of argument in ArgumentTypes enum
//====================================================================================================
#define CREATE_GET_ARGUMENT_FUNC(ArgumentName, ArgumentType, DefaultValue) \
ArgumentType MyEvent::Get##ArgumentName(const char* name) \
{ \
    MyEventArgument* pArg = GetArgument( name ); \
    if( pArg ) \
        return pArg->m_##ArgumentName; \
    return DefaultValue; \
}

CREATE_GET_ARGUMENT_FUNC( Pointer, void*, 0 );
CREATE_GET_ARGUMENT_FUNC( Bool, bool, false );
CREATE_GET_ARGUMENT_FUNC( Int, int32, 0 );
CREATE_GET_ARGUMENT_FUNC( UnsignedInt, uint32, 0 );
CREATE_GET_ARGUMENT_FUNC( Float, float, 0 );
CREATE_GET_ARGUMENT_FUNC( Double, double, 0 );
//====================================================================================================
