//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#include "MyEvent.h"

MyEvent::MyEvent()
{
    m_Type = Event_Undefined;
    m_FirstArgument = 0;
}

MyEvent::~MyEvent()
{
}

void MyEvent::CheckIfArgumentIsAlreadyAttached(char* name) // Protected
{
    MyEventArgument* pArg = m_FirstArgument;
    while( pArg )
    {
        if( *(uint64*)name == pArg->m_NameInt )
        {
            MyAssert( false );
            LOGError( LOGTag, "Duplicate argument attached to event (%s)\n", name );
        }

        pArg = pArg->m_NextArgument;
    }
}

void MyEvent::ClearArguments()
{
    MyEventArgument* pArg = m_FirstArgument;
    while( pArg )
    {
        g_pEventManager->m_pEventArgumentPool.ReturnObjectToPool( pArg );

        pArg = pArg->m_NextArgument;
    }
}

void MyEvent::AttachArgument(MyEventArgument* pArg) // Protected
{
    // debug check for duplicate arguments, asserts(in debug) and puts up an error in log.
    CheckIfArgumentIsAlreadyAttached( pArg->m_NameStr );

    // attach this argument to this event as the first in the list
    pArg->m_NextArgument = this->m_FirstArgument;
    this->m_FirstArgument = pArg;
}

//void MyEvent::AttachPointer(char* name, void* value)

void MyEvent::AttachBool(char* name, bool value)
{
    // setup the new argument.
    MyEventArgument* pArg = g_pEventManager->m_pEventArgumentPool.GetObjectFromPool();
    pArg->m_Type = MyEventArgument::Type_Bool;
    pArg->m_Bool = value;

    AttachArgument( pArg );
}

//void MyEvent::AttachInt(char* name, int32 value)
//void MyEvent::AttachUnsignedInt(char* name, uint32 value)
//void MyEvent::AttachFloat(char* name, float value)
//void MyEvent::AttachDouble(char* name, double value)

MyEventArgument* MyEvent::GetArgument(char* name)
{
    MyEventArgument* pArg = m_FirstArgument;
    while( pArg )
    {
        if( *(uint64*)name == pArg->m_NameInt )
        {
            return pArg;
        }

        pArg = pArg->m_NextArgument;
    }

    LOGError( LOGTag, "Event Argument not found (%s)\n", name );

    return 0;
}

//void* MyEvent::GetPointer(char* name)
//bool MyEvent::GetBool(char* name)
//int32 MyEvent::GetInt(char* name)
//uint32 MyEvent::GetUnsignedInt(char* name)
//float MyEvent::GetFloat(char* name)
//double MyEvent::GetDouble(char* name)
