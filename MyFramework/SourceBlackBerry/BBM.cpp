//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#if MYFW_BLACKBERRY10

#include <stdlib.h>
#include <bps/event.h>
#include <bps/bps.h>

#include "BBM.h"

BlackBerryMessenger* g_pBBM = 0;

BlackBerryMessenger::BlackBerryMessenger(const char* UUID)
{
    m_State = BBMState_None;
    m_UUID = UUID;

    Init( UUID );
}

BlackBerryMessenger::~BlackBerryMessenger()
{
    if( m_State != BBMState_None && m_State != BBMState_Failed )
    {
        // Shutdown bps, which triggers the bbmsp library to shut down too
        //bps_shutdown(); // this will happen in main, if we kill it other messages won't fire.
    }
}

bool BlackBerryMessenger::IsReady()
{
    if( m_State != BBMState_Ready )
        return false;

    return true;
}

void BlackBerryMessenger::Init(const char* UUID)
{
    assert( m_State == BBMState_None || m_State == BBMState_Failed );
    if( m_State != BBMState_None && m_State != BBMState_Failed )
        return;

    m_State = BBMState_Initializing;

    // Start the BPS library and initialize the event queue
    //bps_initialize(); // done in main.cpp

    // Request that events flow into event queue
    bbmsp_result_t rc = bbmsp_request_events( 0 ); // 0 = request all events
    if( rc != BBMSP_SUCCESS )
    {
        m_State = BBMState_Failed;
        return;
    }

    // we'll get a "BBMSP_SP_EVENT_ACCESS_CHANGED" event when we're ready to try to register
}

void BlackBerryMessenger::Register()
{
    if( m_State != BBMState_Initializing )
        return;

    m_State = BBMState_Registering;

    bbmsp_result_t rc = bbmsp_register( m_UUID );
    if( rc == BBMSP_SUCCESS )
    {
        // not sure, shouldn't happen.
        return;
    }
    else if( rc == BBMSP_FAILURE )
    {
        m_State = BBMState_Failed;
        return;
    }
    else if( rc == BBMSP_ASYNC )
    {
        // registration is looking good
        // we'll get a "BBMSP_SP_EVENT_ACCESS_CHANGED" event when access is ready(or not).
        int bp = 1;
    }
}

void BlackBerryMessenger::HandleEvent(bps_event_t* event)
{
    int event_category = 0;
    int event_type = 0;
    bbmsp_event_t* bbmsp_event = 0;

    // Retrieve the bbmsp event category, type, and actual event
    bbmsp_event_get_category( event, &event_category );
    bbmsp_event_get_type( event, &event_type );
    bbmsp_event_get( event, &bbmsp_event );

    // Determine the bbmsp event type & handle accordingly
    // In this case only BBMSP_SP_EVENT_ACCESS_CHANGED
    // events are handled, which are triggered when
    // the init is complete and we're ready to register and when
    // the registration status of the app changes

    if( event_type == BBMSP_SP_EVENT_ACCESS_CHANGED )
    {
        // Determine if the app has successfully
        // registered and handle each case
        int event_is_access_allowed = bbmsp_event_access_changed_get_access_allowed( bbmsp_event );
        if( event_is_access_allowed == 1 )
        {
            // Registration successful, The bbmsp APIs are ready to use
            m_State = BBMState_Ready;
        }
        else
        {
            bbmsp_access_error_codes_t event_access_error_code;// = 0;
            event_access_error_code = bbmsp_event_access_changed_get_access_error_code( bbmsp_event );

            if( event_access_error_code == BBMSP_ACCESS_UNREGISTERED )
            {
                Register();
            }
            else if( event_access_error_code != BBMSP_ACCESS_PENDING &&
                     event_access_error_code != BBMSP_ACCESS_ALLOWED )
            {
                m_State = BBMState_Failed;
            }
        }
    }
    else
    {
        if( m_State == BBMState_Ready )
        {
            // Handle other events now that the
            // bbmsp APIs are ready to use
        }
    }
}

void BlackBerryMessenger::SendString(const char* cookie, const char* string, ...)
{
    if( m_State != BBMState_Ready )
        return;

#define MAX_MESSAGE 1024
    char tempstring[MAX_MESSAGE];
    va_list arg;
    va_start( arg, string );
    vsnprintf( tempstring, MAX_MESSAGE, string, arg );
    va_end(arg);

    //bbmsp_profile_t* user_profile = 0;
    //bbmsp_result_t rc = BBMSP_SUCCESS;
    //rc = bbmsp_get_user_profile( user_profile );
}

void BlackBerryMessenger::AddStringToBBMProfile(const char* cookie, const char* string, ...)
{
    if( m_State != BBMState_Ready )
        return;

    if( bbmsp_can_show_profile_box() == false )
        return;

#define MAX_BBM_MESSAGE 100
    char tempstring[MAX_BBM_MESSAGE];
    va_list arg;
    va_start( arg, string );
    vsnprintf( tempstring, MAX_BBM_MESSAGE, string, arg );
    va_end(arg);

    // if we specified a cookie, then remove the old box item that matches.
    if( cookie )
    {
        // TODO: figure out what an item id is... it's not the cookie...
//        bbmsp_result_t rc = bbmsp_user_profile_box_remove_item( cookie );
//
//        if( rc == BBMSP_FAILURE )
//        {
//            int bp = 1;
//        }

// GARBAGE: these funcs are for a local list.
//        for( int i=0; i<bbmsp_user_profile_box_items_size(); i++ )
//        {
//            bbmsp_user_profile_box_item_list_t* pItemList = 0; // TOdeadDO: get the item list
//
//            if( pItemList )
//            {
//                const bbmsp_user_profile_box_item_t* pItem = bbmsp_user_profile_box_itemlist_get_at(pItemList, i);
//                char oldcookie[100];
//                bbmsp_result_t result = bbmsp_user_profile_box_item_get_cookie( pItem, oldcookie, 100 );
//
//                if( result == BBMSP_SUCCESS )
//                {
//                    if( strcmp_s( oldcookie, cookie ) == 0 )
//                    {
//                        bbmsp_result_t result = bbmsp_user_profile_box_itemlist_remove_at( pItemList, i );
//                        //BBMSP_SUCCESS
//
//                        break;
//                    }
//                }
//            }
//        }
    }

    bbmsp_result_t rc = bbmsp_user_profile_box_add_item_no_icon( tempstring, cookie );

    if( rc == BBMSP_FAILURE )
    {
        int bp = 1;
    }
    else
    {
        int bp = 1;
    }
}

void BlackBerryMessenger::SendInviteToDownload()
{
    if( m_State != BBMState_Ready )
        return;

    int ret = bbmsp_send_download_invitation();

    if( ret == BBMSP_SUCCESS )
    {
        int bp = 1;
    }
    else if( ret == BBMSP_SUCCESS )
    {
        int bp = 1;
    }
}

#endif //MYFW_BLACKBERRY10
