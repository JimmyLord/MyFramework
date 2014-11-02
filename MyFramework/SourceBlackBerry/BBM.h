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

#ifndef __BBM_H__
#define __BBM_H__

#if MYFW_BLACKBERRY10

#include <bbmsp/bbmsp.h>
#include <bbmsp/bbmsp_events.h>
#include <bbmsp/bbmsp_context.h>
#include <bbmsp/bbmsp_messaging.h>
#include <bbmsp/bbmsp_userprofile.h>
#include <bbmsp/bbmsp_user_profile_box.h>

class BlackBerryMessenger;

extern BlackBerryMessenger* g_pBBM;

enum BlackBerryMessangerState
{
    BBMState_None,
    BBMState_Initializing,
    BBMState_Registering,
    BBMState_Ready,
    BBMState_Failed,
};

class BlackBerryMessenger
{
public:
    BlackBerryMessangerState m_State;

    const char* m_UUID; // should be statically allocated somewhere in code.

protected:
    bool IsReady();

public:
    BlackBerryMessenger(const char* UUID);
    ~BlackBerryMessenger();

    void Init(const char* UUID);
    void Register();
    void HandleEvent(bps_event_t* event);

    void SendString(const char* cookie, const char* string, ...);
    void AddStringToBBMProfile(const char* cookie, const char* string, ...);
    void SendInviteToDownload();
};

#endif //MYFW_BLACKBERRY10

#endif //__BBM_H__
